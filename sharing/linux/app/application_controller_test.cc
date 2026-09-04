#include "sharing/linux/app/application_controller.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QQuickWindow>
#include <QTemporaryDir>
#include <QUrl>

#include "gtest/gtest.h"

class ApplicationControllerTestPeer {
 public:
  static void AddOfferNotification(ApplicationController& controller,
                                   uint notification_id,
                                   qint64 share_target_id) {
    controller.notification_to_transfer_.insert(notification_id,
                                                share_target_id);
    controller.transfer_to_notification_.insert(share_target_id,
                                                notification_id);
  }

  static void InvokeAction(ApplicationController& controller,
                           uint notification_id, const QString& action) {
    controller.OnNotificationAction(notification_id, action);
  }

  static void Dismiss(ApplicationController& controller, uint notification_id) {
    controller.OnNotificationClosed(notification_id, /*reason=*/2);
  }

  static void DeliverOfferWithoutActions(ApplicationController& controller,
                                         qint64 share_target_id) {
    controller.actions_supported_ = false;
    controller.OnIncomingOffer(share_target_id, QStringLiteral("Peer"), 1, 100);
  }

  static bool HasNotification(const ApplicationController& controller,
                              uint notification_id) {
    return controller.notification_to_transfer_.contains(notification_id);
  }

  static std::optional<QString> ValidateSelectedFolder(
      const QByteArray& output) {
    return ApplicationController::ValidateSelectedFolder(output);
  }

  static QString InitialFolder(ApplicationController& controller,
                               const QString& requested_path) {
    return controller.InitialFolder(requested_path);
  }

  static QStringList ValidateSelectedFiles(const QByteArray& output) {
    return ApplicationController::ValidateSelectedFiles(output);
  }
};

namespace {

QApplication& TestApplication() {
  static int argc = 1;
  static char application_name[] = "application_controller_test";
  static char* argv[] = {application_name, nullptr};
  static QApplication application(argc, argv);
  return application;
}

TEST(ApplicationControllerTest, NativeActionsRouteToTransfer) {
  ApplicationController controller(TestApplication());
  qint64 accepted_target = 0;
  qint64 rejected_target = 0;
  QObject::connect(&controller, &ApplicationController::acceptRequested,
                   [&accepted_target](qint64 id) { accepted_target = id; });
  QObject::connect(&controller, &ApplicationController::rejectRequested,
                   [&rejected_target](qint64 id) { rejected_target = id; });

  ApplicationControllerTestPeer::AddOfferNotification(controller, 10, 101);
  ApplicationControllerTestPeer::InvokeAction(controller, 10,
                                              QStringLiteral("accept"));
  ApplicationControllerTestPeer::AddOfferNotification(controller, 11, 102);
  ApplicationControllerTestPeer::InvokeAction(controller, 11,
                                              QStringLiteral("decline"));

  EXPECT_EQ(accepted_target, 101);
  EXPECT_EQ(rejected_target, 102);
  EXPECT_FALSE(ApplicationControllerTestPeer::HasNotification(controller, 10));
  EXPECT_FALSE(ApplicationControllerTestPeer::HasNotification(controller, 11));
}

TEST(ApplicationControllerTest, DismissalDoesNotResolveTransfer) {
  ApplicationController controller(TestApplication());
  int responses = 0;
  QObject::connect(&controller, &ApplicationController::acceptRequested,
                   [&responses](qint64) { ++responses; });
  QObject::connect(&controller, &ApplicationController::rejectRequested,
                   [&responses](qint64) { ++responses; });
  ApplicationControllerTestPeer::AddOfferNotification(controller, 20, 201);

  ApplicationControllerTestPeer::Dismiss(controller, 20);

  EXPECT_EQ(responses, 0);
  EXPECT_FALSE(ApplicationControllerTestPeer::HasNotification(controller, 20));
}

TEST(ApplicationControllerTest, DefaultActionShowsWindow) {
  ApplicationController controller(TestApplication());
  QQuickWindow window;
  window.hide();
  controller.AttachWindow(&window);
  ApplicationControllerTestPeer::AddOfferNotification(controller, 30, 301);

  ApplicationControllerTestPeer::InvokeAction(controller, 30,
                                              QStringLiteral("default"));

  EXPECT_TRUE(window.isVisible());
}

TEST(ApplicationControllerTest, MissingNotificationActionsShowsWindow) {
  ApplicationController controller(TestApplication());
  QQuickWindow window;
  window.hide();
  controller.AttachWindow(&window);

  ApplicationControllerTestPeer::DeliverOfferWithoutActions(controller, 401);

  EXPECT_TRUE(window.isVisible());
}

TEST(ApplicationControllerTest, NativeFolderSelectionAcceptsLocalPathsAndUrls) {
  QTemporaryDir directory;
  ASSERT_TRUE(directory.isValid());
  const QString expected = QFileInfo(directory.path()).absoluteFilePath();

  const std::optional<QString> local =
      ApplicationControllerTestPeer::ValidateSelectedFolder(
          directory.path().toLocal8Bit() + '\n');
  ASSERT_TRUE(local.has_value());
  EXPECT_EQ(*local, expected);

  const std::optional<QString> url =
      ApplicationControllerTestPeer::ValidateSelectedFolder(
          QUrl::fromLocalFile(directory.path()).toString().toLocal8Bit() +
          '\n');
  ASSERT_TRUE(url.has_value());
  EXPECT_EQ(*url, expected);
}

TEST(ApplicationControllerTest, NativeFolderSelectionRejectsInvalidOutput) {
  QTemporaryDir directory;
  ASSERT_TRUE(directory.isValid());
  QFile file(directory.filePath(QStringLiteral("not-a-folder")));
  ASSERT_TRUE(file.open(QIODevice::WriteOnly));
  file.close();

  EXPECT_FALSE(ApplicationControllerTestPeer::ValidateSelectedFolder({})
                   .has_value());
  EXPECT_FALSE(ApplicationControllerTestPeer::ValidateSelectedFolder(
                   file.fileName().toLocal8Bit())
                   .has_value());
  EXPECT_FALSE(ApplicationControllerTestPeer::ValidateSelectedFolder(
                   directory.filePath(QStringLiteral("missing")).toLocal8Bit())
                   .has_value());
}

TEST(ApplicationControllerTest, NativeFileSelectionAcceptsMultipleFiles) {
  QTemporaryDir directory;
  ASSERT_TRUE(directory.isValid());
  const QString first_path = directory.filePath(QStringLiteral("first.txt"));
  const QString second_path = directory.filePath(QStringLiteral("second.txt"));
  QFile first(first_path);
  QFile second(second_path);
  ASSERT_TRUE(first.open(QIODevice::WriteOnly));
  ASSERT_TRUE(second.open(QIODevice::WriteOnly));
  first.close();
  second.close();

  const QByteArray output = first_path.toLocal8Bit() + '\n' +
                            QUrl::fromLocalFile(second_path)
                                .toString()
                                .toLocal8Bit() +
                            '\n';
  EXPECT_EQ(ApplicationControllerTestPeer::ValidateSelectedFiles(output),
            QStringList({QFileInfo(first_path).absoluteFilePath(),
                         QFileInfo(second_path).absoluteFilePath()}));
}

TEST(ApplicationControllerTest, NativeFileSelectionRejectsInvalidEntries) {
  QTemporaryDir directory;
  ASSERT_TRUE(directory.isValid());
  const QString valid_path = directory.filePath(QStringLiteral("valid.txt"));
  QFile valid(valid_path);
  ASSERT_TRUE(valid.open(QIODevice::WriteOnly));
  valid.close();

  EXPECT_TRUE(
      ApplicationControllerTestPeer::ValidateSelectedFiles({}).isEmpty());
  EXPECT_TRUE(ApplicationControllerTestPeer::ValidateSelectedFiles(
                  directory.path().toLocal8Bit())
                  .isEmpty());
  EXPECT_TRUE(ApplicationControllerTestPeer::ValidateSelectedFiles(
                  valid_path.toLocal8Bit() + '\n' +
                  directory.filePath(QStringLiteral("missing.txt"))
                      .toLocal8Bit())
                  .isEmpty());
}

TEST(ApplicationControllerTest, NativeFolderPickerUsesValidInitialDirectory) {
  QTemporaryDir directory;
  ASSERT_TRUE(directory.isValid());
  ApplicationController controller(TestApplication());

  EXPECT_EQ(ApplicationControllerTestPeer::InitialFolder(controller,
                                                         directory.path()),
            QFileInfo(directory.path()).absoluteFilePath());
  EXPECT_EQ(ApplicationControllerTestPeer::InitialFolder(
                controller, QUrl::fromLocalFile(directory.path()).toString()),
            QFileInfo(directory.path()).absoluteFilePath());
}

}  // namespace
