#pragma once

#include <memory>
#include <optional>

#include <QByteArray>
#include <QHash>
#include <QObject>
#include <QPointer>
#include <QSet>
#include <QStringList>

class Backend;
class QApplication;
class QMenu;
class QProcess;
class QQuickWindow;
class QSystemTrayIcon;

class ApplicationController : public QObject {
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "io.github.xntso.quixshare")
  Q_PROPERTY(bool closeToTray READ closeToTray NOTIFY closeToTrayChanged)
  Q_PROPERTY(bool folderPickerBusy READ folderPickerBusy NOTIFY
                 folderPickerBusyChanged)
  Q_PROPERTY(bool filePickerBusy READ filePickerBusy NOTIFY
                 filePickerBusyChanged)

 public:
  explicit ApplicationController(QApplication& application,
                                 QObject* parent = nullptr);
  ~ApplicationController() override;

  // Returns false after successfully activating an existing instance.
  bool ClaimSingleInstance();
  void InitializeTray();
  void SetBackend(Backend* backend);
  void AttachWindow(QObject* root_object);

  bool closeToTray() const { return close_to_tray_; }
  bool folderPickerBusy() const;
  bool filePickerBusy() const;

 public slots:
  Q_SCRIPTABLE void ShowWindow();
  Q_INVOKABLE bool chooseDownloadFolder(const QString& initial_path);
  Q_INVOKABLE bool chooseFiles(const QString& initial_path);
  void Quit();

 signals:
  void closeToTrayChanged();
  void folderPickerBusyChanged();
  void filePickerBusyChanged();
  void downloadFolderSelected(const QString& path);
  void filesSelected(const QStringList& paths);
  void nativeFolderPickerUnavailable();
  void nativeFilePickerUnavailable();
  void acceptRequested(qint64 share_target_id);
  void rejectRequested(qint64 share_target_id);

 private slots:
  void OnIncomingOffer(qint64 share_target_id, const QString& device_name,
                       int attachment_count, qint64 total_bytes);
  void OnIncomingOfferResolved(qint64 share_target_id);
  void OnTransferFinished(qint64 share_target_id, bool receive_mode,
                          const QString& device_name, const QString& status);
  void OnNotificationAction(uint notification_id, const QString& action_key);
  void OnNotificationClosed(uint notification_id, uint reason);
  void UpdateTrayAvailability();

 private:
  friend class ApplicationControllerTestPeer;

  uint SendNotification(const QString& summary, const QString& body,
                        const QStringList& actions, int timeout_milliseconds);
  void CloseOfferNotification(qint64 share_target_id);
  void RemoveNotificationMapping(uint notification_id);
  QString IncomingOfferBody(const QString& device_name, int attachment_count,
                            qint64 total_bytes) const;
  static std::optional<QString> ValidateSelectedFolder(
      const QByteArray& picker_output);
  static QStringList ValidateSelectedFiles(const QByteArray& picker_output);
  QString InitialFolder(const QString& requested_path) const;
  void ClearFolderPicker(QProcess* process);
  void ClearFilePicker(QProcess* process);

  QApplication& application_;
  QPointer<QQuickWindow> window_;
  QSystemTrayIcon* tray_icon_ = nullptr;
  std::unique_ptr<QMenu> tray_menu_;
  QPointer<QProcess> folder_picker_;
  QPointer<QProcess> file_picker_;
  QHash<uint, qint64> notification_to_transfer_;
  QHash<qint64, uint> transfer_to_notification_;
  QSet<qint64> background_accepted_transfers_;
  bool close_to_tray_ = false;
  bool actions_supported_ = false;
  bool owns_dbus_service_ = false;
  bool pending_activation_ = false;
};
