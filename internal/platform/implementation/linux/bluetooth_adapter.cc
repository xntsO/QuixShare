// Copyright 2023 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <sdbus-c++/ProxyInterfaces.h>
#include <sdbus-c++/Types.h>

#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <utility>

#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "internal/platform/implementation/bluetooth_adapter.h"
#include "internal/platform/implementation/linux/bluetooth_adapter.h"
#include "internal/platform/implementation/linux/dbus.h"
#include "internal/platform/implementation/linux/generated/dbus/bluez/adapter_client.h"
#include "internal/platform/logging.h"

namespace nearby {
namespace linux {
namespace {

// Nearby Connections creates more than one BluetoothAdapter wrapper for the
// same BlueZ adapter. Coordinate their discoverability requests so the last
// wrapper to stop restores the value that existed before the first started.
std::mutex discoverability_mutex;
struct DiscoverabilityState {
  std::optional<uint32_t> original_timeout;
  std::optional<bool> original_discoverable;
  int users = 0;
};
std::map<std::string, DiscoverabilityState> discoverability_by_adapter;

constexpr int kBluezPropertyUpdateAttempts = 10;
constexpr absl::Duration kBluezPropertyUpdateRetryDelay =
    absl::Milliseconds(50);

// BlueZ can keep returning org.bluez.Error.Failed for a short settling window
// after an advertisement or profile has been unregistered. Retrying here is
// intentionally bounded; permanent errors are returned immediately.
template <typename Setter>
void SetBluezPropertyWithRetry(absl::string_view property, Setter&& setter) {
  for (int attempt = 1; attempt <= kBluezPropertyUpdateAttempts; ++attempt) {
    try {
      std::forward<Setter>(setter)();
      return;
    } catch (const sdbus::Error& error) {
      if (error.getName() != "org.bluez.Error.Failed" ||
          attempt == kBluezPropertyUpdateAttempts) {
        throw;
      }
      LOG(WARNING) << "BlueZ is busy setting " << property << " (attempt "
                   << attempt << "/" << kBluezPropertyUpdateAttempts
                   << "); retrying: " << error.getMessage();
      absl::SleepFor(kBluezPropertyUpdateRetryDelay);
    }
  }
}

}  // namespace

BluetoothAdapter::~BluetoothAdapter() {
  if (owns_discoverability_) {
    if (!DisableDiscoverabilityAndRestoreTimeout()) {
      // The proxy may already be unavailable during process shutdown. Never
      // leave process-local lease accounting poisoned for surviving wrappers.
      AbandonDiscoverabilityOwnership();
    }
  }
}

bool BluetoothAdapter::SetStatus(Status status) {
  try {
    bool val = status == api::BluetoothAdapter::Status::kEnabled;
    bluez_adapter_->Powered(val);
    return true;
  } catch (const sdbus::Error& e) {
    DBUS_LOG_PROPERTY_SET_ERROR(bluez_adapter_, "Powered", e);
    return false;
  }
}

bool BluetoothAdapter::IsEnabled() const {
  try {
    return bluez_adapter_->Powered();
  } catch (const sdbus::Error& e) {
    DBUS_LOG_PROPERTY_GET_ERROR(bluez_adapter_, "Powered", e);
    return false;
  }
}

BluetoothAdapter::ScanMode BluetoothAdapter::GetScanMode() const {
  bool powered = IsEnabled();
  if (!powered) {
    return ScanMode::kNone;
  }

  try {
    bool discoverable = bluez_adapter_->Discoverable();
    return discoverable ? ScanMode::kConnectableDiscoverable
                        : ScanMode::kConnectable;
  } catch (const sdbus::Error& e) {
    DBUS_LOG_PROPERTY_GET_ERROR(bluez_adapter_, "Discoverable", e);
    return ScanMode::kUnknown;
  }
}

bool BluetoothAdapter::SetScanMode(ScanMode scan_mode) {
  switch (scan_mode) {
    case ScanMode::kConnectable: {
      if (!SetStatus(Status::kEnabled)) {
        return false;
      }
      return DisableDiscoverabilityAndRestoreTimeout();
    }
    case ScanMode::kConnectableDiscoverable: {
      if (!SetStatus(Status::kEnabled)) {
        return false;
      }

      try {
        std::lock_guard<std::mutex> lock(discoverability_mutex);
        auto key = std::string(bluez_adapter_->getProxy().getObjectPath());
        auto& state = discoverability_by_adapter[key];
        if (owns_discoverability_) {
          return true;
        }
        if (state.users > 0) {
          ++state.users;
          owns_discoverability_ = true;
          return true;
        }

        // A prior destructor may have lost D-Bus connectivity while restoring
        // the last lease. Repair that saved baseline before starting a new
        // session.
        if (state.original_timeout.has_value()) {
          // Disable discoverability while timeout is still zero. Setting a
          // non-zero timeout first makes BlueZ start a mode update and the
          // immediately-following Discoverable(false) can fail with MGMT Busy.
          SetBluezPropertyWithRetry("Discoverable", [&]() {
            bluez_adapter_->Discoverable(
                state.original_discoverable.value_or(false));
          });
          SetBluezPropertyWithRetry("DiscoverableTimeout", [&]() {
            bluez_adapter_->DiscoverableTimeout(*state.original_timeout);
          });
          state = DiscoverabilityState{};
        }

        uint32_t saved_timeout = bluez_adapter_->DiscoverableTimeout();
        bool saved_discoverable = bluez_adapter_->Discoverable();
        state.original_timeout = saved_timeout;
        state.original_discoverable = saved_discoverable;
        // BlueZ otherwise applies the system-wide timeout (commonly 180
        // seconds), even while Nearby Sharing is still advertising. Keep the
        // adapter discoverable for the lifetime of this advertising session.
        bluez_adapter_->DiscoverableTimeout(0);
        try {
          bluez_adapter_->Discoverable(true);
        } catch (...) {
          // Roll back a partially-created lease before reporting failure.
          try {
            SetBluezPropertyWithRetry("Discoverable rollback", [&]() {
              bluez_adapter_->Discoverable(saved_discoverable);
            });
            SetBluezPropertyWithRetry("DiscoverableTimeout rollback", [&]() {
              bluez_adapter_->DiscoverableTimeout(saved_timeout);
            });
            discoverability_by_adapter.erase(key);
          } catch (const sdbus::Error& rollback_error) {
            DBUS_LOG_PROPERTY_SET_ERROR(bluez_adapter_, "Discoverable rollback",
                                        rollback_error);
          }
          throw;
        }
        state.users = 1;
        owns_discoverability_ = true;
      } catch (const sdbus::Error& e) {
        DBUS_LOG_PROPERTY_SET_ERROR(bluez_adapter_, "Discoverable", e);
        return false;
      }

      return true;
    }
    case ScanMode::kNone: {
      if (!DisableDiscoverabilityAndRestoreTimeout()) {
        return false;
      }
      return SetStatus(Status::kDisabled);
    }
    default:
      return false;
  }
}

bool BluetoothAdapter::DisableDiscoverabilityAndRestoreTimeout() {
  try {
    std::lock_guard<std::mutex> lock(discoverability_mutex);
    auto key = std::string(bluez_adapter_->getProxy().getObjectPath());
    auto state_it = discoverability_by_adapter.find(key);
    if (!owns_discoverability_) {
      if (state_it != discoverability_by_adapter.end() &&
          state_it->second.users == 0 &&
          state_it->second.original_timeout.has_value()) {
        SetBluezPropertyWithRetry("Discoverable", [&]() {
          bluez_adapter_->Discoverable(
              state_it->second.original_discoverable.value_or(false));
        });
        SetBluezPropertyWithRetry("DiscoverableTimeout", [&]() {
          bluez_adapter_->DiscoverableTimeout(
              *state_it->second.original_timeout);
        });
        discoverability_by_adapter.erase(state_it);
      } else if (state_it == discoverability_by_adapter.end() ||
                 state_it->second.users == 0) {
        SetBluezPropertyWithRetry(
            "Discoverable", [&]() { bluez_adapter_->Discoverable(false); });
      }
      return true;
    }

    if (state_it == discoverability_by_adapter.end()) {
      owns_discoverability_ = false;
      return true;
    }
    auto& state = state_it->second;
    if (state.users > 1) {
      --state.users;
      owns_discoverability_ = false;
      return true;
    }

    // Keep ownership until both properties have been restored so a failed
    // D-Bus write can be retried by the caller or destructor.
    SetBluezPropertyWithRetry("Discoverable", [&]() {
      bluez_adapter_->Discoverable(state.original_discoverable.value_or(false));
    });
    if (state.original_timeout.has_value()) {
      SetBluezPropertyWithRetry("DiscoverableTimeout", [&]() {
        bluez_adapter_->DiscoverableTimeout(*state.original_timeout);
      });
    }
    --state.users;
    owns_discoverability_ = false;
    discoverability_by_adapter.erase(state_it);
    return true;
  } catch (const sdbus::Error& e) {
    DBUS_LOG_PROPERTY_SET_ERROR(bluez_adapter_, "Discoverable", e);
    return false;
  }
}

void BluetoothAdapter::AbandonDiscoverabilityOwnership() {
  std::lock_guard<std::mutex> lock(discoverability_mutex);
  if (!owns_discoverability_) {
    return;
  }
  owns_discoverability_ = false;
  auto key = std::string(bluez_adapter_->getProxy().getObjectPath());
  auto state_it = discoverability_by_adapter.find(key);
  if (state_it == discoverability_by_adapter.end()) {
    return;
  }
  if (state_it->second.users > 0) {
    --state_it->second.users;
  }
  if (state_it->second.users == 0 &&
      !state_it->second.original_timeout.has_value()) {
    discoverability_by_adapter.erase(state_it);
  }
}

std::string BluetoothAdapter::GetName() const {
  try {
    return bluez_adapter_->Alias();
  } catch (const sdbus::Error& e) {
    DBUS_LOG_PROPERTY_GET_ERROR(bluez_adapter_, "Alias", e);
    return {};
  }
}

bool BluetoothAdapter::SetName(absl::string_view name, bool /*persist*/) {
  return SetName(name);
}

bool BluetoothAdapter::SetName(absl::string_view name) {
  try {
    SetBluezPropertyWithRetry(
        "Alias", [&]() { bluez_adapter_->Alias(std::string(name)); });
    return true;
  } catch (const sdbus::Error& e) {
    DBUS_LOG_PROPERTY_SET_ERROR(bluez_adapter_, "Alias", e);
    return false;
  }
}

MacAddress BluetoothAdapter::GetMacAddress() const {
  try {
    MacAddress addr;
    MacAddress::FromString(bluez_adapter_->Address(), addr);
    return addr;
  } catch (const sdbus::Error& e) {
    DBUS_LOG_PROPERTY_GET_ERROR(bluez_adapter_, "Address", e);
    return {};
  }
}

}  // namespace linux
}  // namespace nearby
