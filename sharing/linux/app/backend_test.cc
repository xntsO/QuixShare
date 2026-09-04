#include "sharing/linux/app/backend.h"

#include <cstdlib>
#include <memory>

#include <QCoreApplication>
#include <QEventLoop>
#include <QTemporaryFile>
#include "gtest/gtest.h"
#include "internal/platform/implementation/linux/network_safety.h"
#include "sharing/attachment_container.h"
#include "sharing/transfer_metadata_builder.h"

class BackendTestPeer {
 public:
  static std::unique_ptr<Backend> Create(
      nearby::sharing::NearbySharingService& service,
      nearby::api::FastInitiationManager& fast_init_manager) {
    return std::unique_ptr<Backend>(
        new Backend(service, fast_init_manager,
                    /*receive_timeout_milliseconds=*/5000, nullptr));
  }

  static bool IsIdle(const Backend& backend) {
    return backend.active_mode_ == Backend::Mode::kNone;
  }

  static bool IsReceiving(const Backend& backend) {
    return backend.active_mode_ == Backend::Mode::kReceive;
  }

  static bool IsDiscovering(const Backend& backend) {
    return backend.active_mode_ == Backend::Mode::kDiscovery;
  }

  static bool IsReceiveTimerActive(const Backend& backend) {
    return backend.receive_timeout_timer_.isActive();
  }

  static bool IsMonitoringRequested(const Backend& backend) {
    return backend.monitoring_requested_;
  }

  static int TargetCount(const Backend& backend) {
    return backend.targets_.rowCount();
  }

  static int TransferCount(const Backend& backend) {
    return backend.transfers_.rowCount();
  }

  static bool TransferIsFinal(const Backend& backend, int row) {
    return backend.transfers_
        .data(backend.transfers_.index(row),
              ShareTransferModel::IsFinalStatusRole)
        .toBool();
  }

  static bool HasIncomingOfferTimer(const Backend& backend,
                                    int64_t share_target_id) {
    return backend.incoming_offer_timers_.contains(share_target_id);
  }

  static void DiscoverTarget(Backend& backend,
                             const nearby::sharing::ShareTarget& target) {
    backend.OnShareTargetDiscovered(target);
  }

  static void ExpireReceiveWindow(Backend& backend) {
    backend.OnReceiveTimeout();
  }

  static void ExpireIncomingOffer(Backend& backend, int64_t share_target_id) {
    backend.OnIncomingOfferTimeout(share_target_id);
  }
};

namespace {

class TestSharingService final : public nearby::sharing::NearbySharingService {
 public:
  void AddObserver(Observer*) override {}
  void RemoveObserver(Observer*) override {}
  void Shutdown(std::function<void(StatusCodes)> callback) override {
    callback(StatusCodes::kOk);
  }
  void RegisterSendSurface(
      nearby::sharing::TransferUpdateCallback*,
      nearby::sharing::ShareTargetDiscoveredCallback*, SendSurfaceState,
      nearby::sharing::Advertisement::BlockedVendorId,
      bool disable_wifi_hotspot,
      absl::AnyInvocable<void(StatusCodes)> callback) override {
    send_registered_ = true;
    disable_wifi_hotspot_ = disable_wifi_hotspot;
    std::move(callback)(StatusCodes::kOk);
  }
  void UnregisterSendSurface(
      nearby::sharing::TransferUpdateCallback*,
      absl::AnyInvocable<void(StatusCodes)> callback) override {
    send_registered_ = false;
    std::move(callback)(StatusCodes::kOk);
  }
  void RegisterReceiveSurface(
      nearby::sharing::TransferUpdateCallback* callback, ReceiveSurfaceState,
      nearby::sharing::Advertisement::BlockedVendorId,
      absl::AnyInvocable<void(StatusCodes)> status_callback) override {
    receive_callback_ = callback;
    std::move(status_callback)(StatusCodes::kOk);
  }
  void UnregisterReceiveSurface(
      nearby::sharing::TransferUpdateCallback*,
      absl::AnyInvocable<void(StatusCodes)> callback) override {
    receive_callback_ = nullptr;
    std::move(callback)(StatusCodes::kOk);
  }
  void ClearForegroundReceiveSurfaces(
      absl::AnyInvocable<void(StatusCodes)> callback) override {
    receive_callback_ = nullptr;
    std::move(callback)(StatusCodes::kOk);
  }
  bool IsTransferring() const override { return false; }
  bool IsScanning() const override { return send_registered_; }
  bool IsBluetoothPresent() const override { return true; }
  bool IsBluetoothPowered() const override { return true; }
  bool IsExtendedAdvertisingSupported() const override { return true; }
  bool IsLanConnected() const override { return true; }
  std::string GetQrCodeUrl() const override { return {}; }
  void SendAttachments(
      int64_t share_target_id,
      std::unique_ptr<nearby::sharing::AttachmentContainer> attachments,
      std::function<void(StatusCodes)> callback) override {
    last_sent_target_ = share_target_id;
    last_sent_attachment_count_ = attachments->GetAttachmentCount();
    callback(next_send_status_);
  }
  void Accept(int64_t share_target_id,
              std::function<void(StatusCodes)> callback) override {
    ++accept_count_;
    last_accepted_target_ = share_target_id;
    callback(StatusCodes::kOk);
  }
  void Reject(int64_t share_target_id,
              std::function<void(StatusCodes)> callback) override {
    ++reject_count_;
    last_rejected_target_ = share_target_id;
    callback(StatusCodes::kOk);
  }
  void Cancel(int64_t, std::function<void(StatusCodes)> callback) override {
    callback(StatusCodes::kOk);
  }
  void InitiatePairing(
      int64_t, nearby::sharing::service::proto::BindingRequest::Type,
      absl::AnyInvocable<void(StatusCodes) &&> callback) override {
    std::move(callback)(StatusCodes::kOk);
  }
  void SetVisibility(
      nearby::sharing::proto::DeviceVisibility visibility, absl::Duration,
      absl::AnyInvocable<void(StatusCodes) &&> callback) override {
    last_visibility_ = visibility;
    ++visibility_request_count_;
    if (delay_visibility_callback_) {
      pending_visibility_callback_ = std::move(callback);
      return;
    }
    std::move(callback)(StatusCodes::kOk);
  }
  std::string Dump() const override { return {}; }
  void UpdateFilePathsInProgress(bool) override {}
  nearby::sharing::NearbyShareSettings* GetSettings() override {
    return nullptr;
  }
  nearby::sharing::NearbyShareCertificateManager* GetCertificateManager()
      override {
    return nullptr;
  }
  nearby::sharing::AccountManager* GetAccountManager() override {
    return nullptr;
  }
  nearby::Clock& GetClock() override { std::abort(); }
  void SetAlternateServiceUuidForDiscovery(uint16_t) override {}
  nearby::sharing::SyncManager& sync_manager() override { std::abort(); }
  nearby::sharing::OutgoingTargetsManager& outgoing_targets_manager() override {
    std::abort();
  }
  void UpdateBackupSavePath(
      absl::string_view, absl::string_view,
      absl::AnyInvocable<void(StatusCodes)> callback) override {
    std::move(callback)(StatusCodes::kOk);
  }

  void FireReceiveTransferUpdate(
      const nearby::sharing::ShareTarget& target,
      const nearby::sharing::AttachmentContainer& attachments,
      const nearby::sharing::TransferMetadata& metadata) {
    ASSERT_NE(receive_callback_, nullptr);
    receive_callback_->OnTransferUpdate(target, attachments, metadata);
  }

  int accept_count() const { return accept_count_; }
  int reject_count() const { return reject_count_; }
  int64_t last_accepted_target() const { return last_accepted_target_; }
  int64_t last_rejected_target() const { return last_rejected_target_; }
  int64_t last_sent_target() const { return last_sent_target_; }
  int last_sent_attachment_count() const { return last_sent_attachment_count_; }
  bool disable_wifi_hotspot() const { return disable_wifi_hotspot_; }
  void set_next_send_status(StatusCodes status) { next_send_status_ = status; }
  void set_delay_visibility_callback(bool delay) {
    delay_visibility_callback_ = delay;
  }
  void CompleteVisibility(StatusCodes status = StatusCodes::kOk) {
    ASSERT_TRUE(pending_visibility_callback_.has_value());
    auto callback = std::move(*pending_visibility_callback_);
    pending_visibility_callback_.reset();
    std::move(callback)(status);
  }
  int visibility_request_count() const { return visibility_request_count_; }
  nearby::sharing::proto::DeviceVisibility last_visibility() const {
    return last_visibility_;
  }

 private:
  bool send_registered_ = false;
  bool disable_wifi_hotspot_ = false;
  nearby::sharing::TransferUpdateCallback* receive_callback_ = nullptr;
  int accept_count_ = 0;
  int reject_count_ = 0;
  int64_t last_accepted_target_ = 0;
  int64_t last_rejected_target_ = 0;
  int64_t last_sent_target_ = 0;
  int last_sent_attachment_count_ = 0;
  StatusCodes next_send_status_ = StatusCodes::kOk;
  bool delay_visibility_callback_ = false;
  int visibility_request_count_ = 0;
  nearby::sharing::proto::DeviceVisibility last_visibility_ =
      nearby::sharing::proto::DEVICE_VISIBILITY_UNSPECIFIED;
  std::optional<absl::AnyInvocable<void(StatusCodes) &&>>
      pending_visibility_callback_;
};

class TestFastInitiationManager final
    : public nearby::api::FastInitiationManager {
 public:
  void StartAdvertising(nearby::api::FastInitBleBeacon::FastInitType,
                        std::function<void()> callback,
                        std::function<void(Error)>) override {
    if (callback) callback();
  }
  void StopAdvertising(std::function<void()> callback) override {
    if (callback) callback();
  }
  void StartScanning(std::function<void()> discovered_callback,
                     std::function<void()> not_discovered_callback,
                     std::function<void(Error)> error_callback) override {
    discovered_callback_ = std::move(discovered_callback);
    not_discovered_callback_ = std::move(not_discovered_callback);
    error_callback_ = std::move(error_callback);
    is_scanning_ = true;
  }
  void StopScanning(std::function<void()> callback) override {
    stopped_callback_ = std::move(callback);
    is_scanning_ = false;
  }
  bool IsAdvertising() override { return false; }
  bool IsScanning() override { return is_scanning_; }

  void FireDiscovered() { discovered_callback_(); }
  void FireNotDiscovered() { not_discovered_callback_(); }
  void FireError(Error error) { error_callback_(error); }
  void CompleteStop() {
    if (stopped_callback_) std::move(stopped_callback_)();
  }

 private:
  bool is_scanning_ = false;
  std::function<void()> discovered_callback_;
  std::function<void()> not_discovered_callback_;
  std::function<void()> stopped_callback_;
  std::function<void(Error)> error_callback_;
};

QCoreApplication& TestApplication() {
  static int argc = 1;
  static char application_name[] = "backend_test";
  static char* argv[] = {application_name, nullptr};
  static QCoreApplication application(argc, argv);
  return application;
}

void DrainEvents() {
  static_cast<void>(TestApplication());
  for (int i = 0; i < 4; ++i) {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
  }
}

class BackendFastInitiationTest : public testing::Test {
 protected:
  void SetUp() override {
    static_cast<void>(TestApplication());
    const char* opt_in =
        std::getenv(nearby::linux::kAllowWifiReconfigurationEnv);
    if (opt_in != nullptr) {
      original_wifi_reconfiguration_opt_in_ = opt_in;
    }
    unsetenv(nearby::linux::kAllowWifiReconfigurationEnv);
    backend_ = BackendTestPeer::Create(service_, fast_init_manager_);
  }

  void TearDown() override {
    backend_.reset();
    DrainEvents();
    if (original_wifi_reconfiguration_opt_in_.has_value()) {
      setenv(nearby::linux::kAllowWifiReconfigurationEnv,
             original_wifi_reconfiguration_opt_in_->c_str(), 1);
    } else {
      unsetenv(nearby::linux::kAllowWifiReconfigurationEnv);
    }
  }

  TestSharingService service_;
  TestFastInitiationManager fast_init_manager_;
  std::unique_ptr<Backend> backend_;
  std::optional<std::string> original_wifi_reconfiguration_opt_in_;
};

TEST_F(BackendFastInitiationTest, StartupScansWithoutReceiving) {
  EXPECT_TRUE(fast_init_manager_.IsScanning());
  EXPECT_TRUE(BackendTestPeer::IsIdle(*backend_));
  EXPECT_FALSE(BackendTestPeer::IsReceiveTimerActive(*backend_));
}

TEST_F(BackendFastInitiationTest, PresenceOpensTimedReceiveWindow) {
  fast_init_manager_.FireDiscovered();
  DrainEvents();

  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
  EXPECT_TRUE(BackendTestPeer::IsReceiveTimerActive(*backend_));
  EXPECT_TRUE(fast_init_manager_.IsScanning());
}

TEST_F(BackendFastInitiationTest,
       TimeoutRequiresSenderAbsenceBeforeAnotherWindow) {
  fast_init_manager_.FireDiscovered();
  DrainEvents();
  BackendTestPeer::ExpireReceiveWindow(*backend_);
  DrainEvents();
  ASSERT_TRUE(BackendTestPeer::IsIdle(*backend_));

  fast_init_manager_.FireDiscovered();
  DrainEvents();
  EXPECT_TRUE(BackendTestPeer::IsIdle(*backend_));

  fast_init_manager_.FireNotDiscovered();
  fast_init_manager_.FireDiscovered();
  DrainEvents();
  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
}

TEST_F(BackendFastInitiationTest, SenderLossDoesNotCloseCurrentWindow) {
  fast_init_manager_.FireDiscovered();
  DrainEvents();
  fast_init_manager_.FireNotDiscovered();
  DrainEvents();

  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
  EXPECT_TRUE(BackendTestPeer::IsReceiveTimerActive(*backend_));
}

TEST_F(BackendFastInitiationTest, IncomingOfferCancelsReceiveTimeout) {
  fast_init_manager_.FireDiscovered();
  DrainEvents();

  nearby::sharing::ShareTarget target;
  target.id = 1;
  auto attachments = nearby::sharing::AttachmentContainer::Builder().Build();
  service_.FireReceiveTransferUpdate(
      target, *attachments,
      nearby::sharing::TransferMetadataBuilder()
          .set_status(nearby::sharing::TransferMetadata::Status::
                          kAwaitingLocalConfirmation)
          .build());
  DrainEvents();

  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
  EXPECT_FALSE(BackendTestPeer::IsReceiveTimerActive(*backend_));
  EXPECT_TRUE(BackendTestPeer::HasIncomingOfferTimer(*backend_, target.id));
  EXPECT_EQ(BackendTestPeer::TargetCount(*backend_), 0);
}

TEST_F(BackendFastInitiationTest, AcceptCancelsIncomingOfferTimeout) {
  fast_init_manager_.FireDiscovered();
  DrainEvents();

  nearby::sharing::ShareTarget target;
  target.id = 11;
  auto attachments = nearby::sharing::AttachmentContainer::Builder().Build();
  service_.FireReceiveTransferUpdate(
      target, *attachments,
      nearby::sharing::TransferMetadataBuilder()
          .set_status(nearby::sharing::TransferMetadata::Status::
                          kAwaitingLocalConfirmation)
          .build());
  DrainEvents();

  backend_->accept(target.id);

  EXPECT_FALSE(BackendTestPeer::HasIncomingOfferTimer(*backend_, target.id));
  EXPECT_EQ(service_.accept_count(), 1);
  EXPECT_EQ(service_.last_accepted_target(), target.id);
  BackendTestPeer::ExpireIncomingOffer(*backend_, target.id);
  EXPECT_EQ(service_.reject_count(), 0);
}

TEST_F(BackendFastInitiationTest, IncomingOfferTimeoutRejectsExactlyOnce) {
  fast_init_manager_.FireDiscovered();
  DrainEvents();

  nearby::sharing::ShareTarget target;
  target.id = 12;
  auto attachments = nearby::sharing::AttachmentContainer::Builder().Build();
  service_.FireReceiveTransferUpdate(
      target, *attachments,
      nearby::sharing::TransferMetadataBuilder()
          .set_status(nearby::sharing::TransferMetadata::Status::
                          kAwaitingLocalConfirmation)
          .build());
  DrainEvents();

  BackendTestPeer::ExpireIncomingOffer(*backend_, target.id);
  BackendTestPeer::ExpireIncomingOffer(*backend_, target.id);

  EXPECT_FALSE(BackendTestPeer::HasIncomingOfferTimer(*backend_, target.id));
  EXPECT_EQ(service_.reject_count(), 1);
  EXPECT_EQ(service_.last_rejected_target(), target.id);
}

TEST_F(BackendFastInitiationTest, FinalReceiveRestartsPersistentReceive) {
  fast_init_manager_.FireDiscovered();
  DrainEvents();

  nearby::sharing::ShareTarget target;
  target.id = 13;
  auto attachments = nearby::sharing::AttachmentContainer::Builder().Build();
  service_.FireReceiveTransferUpdate(
      target, *attachments,
      nearby::sharing::TransferMetadataBuilder()
          .set_status(nearby::sharing::TransferMetadata::Status::kComplete)
          .build());
  DrainEvents();

  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
  EXPECT_FALSE(BackendTestPeer::IsMonitoringRequested(*backend_));
  EXPECT_FALSE(BackendTestPeer::IsReceiveTimerActive(*backend_));
  EXPECT_FALSE(fast_init_manager_.IsScanning());
}

TEST_F(BackendFastInitiationTest, DiscoveryStopsFastInitiationScanning) {
  backend_->startDiscovery();
  EXPECT_FALSE(fast_init_manager_.IsScanning());
  EXPECT_TRUE(BackendTestPeer::IsIdle(*backend_));

  fast_init_manager_.CompleteStop();
  DrainEvents();
  EXPECT_TRUE(BackendTestPeer::IsDiscovering(*backend_));
  EXPECT_TRUE(service_.disable_wifi_hotspot());

  backend_->startReceive();
  DrainEvents();
  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
  EXPECT_FALSE(BackendTestPeer::IsReceiveTimerActive(*backend_));
  EXPECT_FALSE(fast_init_manager_.IsScanning());
}

TEST_F(BackendFastInitiationTest,
       ReturningHomeDuringScanStopDoesNotRestartDiscovery) {
  backend_->startDiscovery();
  EXPECT_FALSE(fast_init_manager_.IsScanning());
  EXPECT_TRUE(BackendTestPeer::IsIdle(*backend_));

  backend_->startReceive();
  DrainEvents();
  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));

  fast_init_manager_.CompleteStop();
  DrainEvents();
  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
}

TEST_F(BackendFastInitiationTest, SendsMultipleSelectedFilesTogether) {
  backend_->startDiscovery();
  fast_init_manager_.CompleteStop();
  DrainEvents();

  nearby::sharing::ShareTarget target;
  target.id = 42;
  target.device_name = "Phone";
  BackendTestPeer::DiscoverTarget(*backend_, target);
  DrainEvents();

  QTemporaryFile first;
  QTemporaryFile second;
  ASSERT_TRUE(first.open());
  ASSERT_TRUE(second.open());
  first.close();
  second.close();

  EXPECT_TRUE(backend_->sendFiles(
      target.id, QStringList{first.fileName(), second.fileName()}));
  EXPECT_EQ(service_.last_sent_target(), target.id);
  EXPECT_EQ(service_.last_sent_attachment_count(), 2);
}

TEST_F(BackendFastInitiationTest, RejectsInvalidMultiFileSelectionAtomically) {
  backend_->startDiscovery();
  fast_init_manager_.CompleteStop();
  DrainEvents();

  nearby::sharing::ShareTarget target;
  target.id = 43;
  target.device_name = "Phone";
  BackendTestPeer::DiscoverTarget(*backend_, target);
  DrainEvents();

  QTemporaryFile valid_file;
  ASSERT_TRUE(valid_file.open());
  valid_file.close();

  EXPECT_FALSE(backend_->sendFiles(
      target.id,
      QStringList{valid_file.fileName(), QStringLiteral("/missing/file")}));
  EXPECT_EQ(service_.last_sent_attachment_count(), 0);
  EXPECT_EQ(BackendTestPeer::TransferCount(*backend_), 0);
}

TEST_F(BackendFastInitiationTest, FailedSendCanBeClearedAndRetried) {
  backend_->startDiscovery();
  fast_init_manager_.CompleteStop();
  DrainEvents();

  nearby::sharing::ShareTarget target;
  target.id = 44;
  target.device_name = "Phone";
  BackendTestPeer::DiscoverTarget(*backend_, target);
  DrainEvents();

  QTemporaryFile file;
  ASSERT_TRUE(file.open());
  file.close();
  service_.set_next_send_status(
      nearby::sharing::NearbySharingService::StatusCodes::kError);

  EXPECT_TRUE(backend_->sendFile(target.id, file.fileName()));
  DrainEvents();
  ASSERT_EQ(BackendTestPeer::TransferCount(*backend_), 1);
  EXPECT_TRUE(BackendTestPeer::TransferIsFinal(*backend_, 0));

  backend_->clearTransfer(target.id);
  EXPECT_EQ(BackendTestPeer::TransferCount(*backend_), 0);
}

TEST_F(BackendFastInitiationTest, SerializesRapidVisibilityChanges) {
  service_.set_delay_visibility_callback(true);

  backend_->setVisibleToEveryone(false);
  backend_->setVisibleToEveryone(true);
  EXPECT_EQ(service_.visibility_request_count(), 1);
  EXPECT_EQ(service_.last_visibility(),
            nearby::sharing::proto::DEVICE_VISIBILITY_HIDDEN);

  service_.CompleteVisibility();
  DrainEvents();
  EXPECT_EQ(service_.visibility_request_count(), 2);
  EXPECT_EQ(service_.last_visibility(),
            nearby::sharing::proto::DEVICE_VISIBILITY_EVERYONE);

  service_.CompleteVisibility();
  DrainEvents();
  EXPECT_TRUE(backend_->visibleToEveryone());
  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
}

TEST_F(BackendFastInitiationTest, FailedHideRestoresReceiving) {
  service_.set_delay_visibility_callback(true);

  backend_->setVisibleToEveryone(false);
  service_.CompleteVisibility(
      nearby::sharing::NearbySharingService::StatusCodes::kError);
  DrainEvents();

  EXPECT_TRUE(backend_->visibleToEveryone());
  EXPECT_TRUE(BackendTestPeer::IsReceiving(*backend_));
}

TEST_F(BackendFastInitiationTest, ScanErrorUsesOneFallbackWindowThenIdles) {
  fast_init_manager_.StopScanning(nullptr);
  fast_init_manager_.FireError(
      nearby::api::FastInitiationManager::Error::kHardwareNotSupported);
  DrainEvents();

  ASSERT_TRUE(BackendTestPeer::IsReceiving(*backend_));
  EXPECT_TRUE(BackendTestPeer::IsReceiveTimerActive(*backend_));
  BackendTestPeer::ExpireReceiveWindow(*backend_);
  DrainEvents();

  EXPECT_TRUE(BackendTestPeer::IsIdle(*backend_));
  EXPECT_FALSE(BackendTestPeer::IsMonitoringRequested(*backend_));
  EXPECT_FALSE(fast_init_manager_.IsScanning());
}

}  // namespace
