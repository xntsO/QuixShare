// Copyright 2026 Google LLC
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

#include "internal/platform/implementation/linux/network_safety.h"

#include <cstdlib>
#include <optional>
#include <string>

#include "gtest/gtest.h"
#include "internal/platform/implementation/platform.h"

namespace nearby::linux {
namespace {

class ScopedWifiReconfigurationOptInUnset {
 public:
  ScopedWifiReconfigurationOptInUnset() {
    const char* current = std::getenv(kAllowWifiReconfigurationEnv);
    if (current != nullptr) {
      original_value_ = current;
    }
    unsetenv(kAllowWifiReconfigurationEnv);
  }

  ~ScopedWifiReconfigurationOptInUnset() {
    if (original_value_.has_value()) {
      setenv(kAllowWifiReconfigurationEnv, original_value_->c_str(), 1);
    } else {
      unsetenv(kAllowWifiReconfigurationEnv);
    }
  }

 private:
  std::optional<std::string> original_value_;
};

TEST(NetworkSafetyTest, DefaultPolicyAllowsLanOnly) {
  const WifiMediumPolicy policy = WifiMediumPolicyForOptInValue("");

  EXPECT_TRUE(policy.wifi_lan);
  EXPECT_FALSE(policy.wifi_hotspot);
  EXPECT_FALSE(policy.wifi_direct);
  EXPECT_FALSE(policy.CanReconfigureManagedWifi());
}

TEST(NetworkSafetyTest, OnlyExactExplicitOptInAllowsReconfiguration) {
  for (std::string_view value : {"true", "TRUE", "0", "01", " 1"}) {
    const WifiMediumPolicy policy = WifiMediumPolicyForOptInValue(value);
    EXPECT_TRUE(policy.wifi_lan) << value;
    EXPECT_FALSE(policy.CanReconfigureManagedWifi()) << value;
  }

  const WifiMediumPolicy opted_in = WifiMediumPolicyForOptInValue("1");
  EXPECT_TRUE(opted_in.wifi_lan);
  EXPECT_TRUE(opted_in.wifi_hotspot);
  EXPECT_TRUE(opted_in.wifi_direct);
  EXPECT_TRUE(opted_in.CanReconfigureManagedWifi());
}

TEST(NetworkSafetyTest,
     DefaultFactoriesCannotConstructManagedWifiReconfigurationMediums) {
  ScopedWifiReconfigurationOptInUnset scoped_opt_in;

  // These return before creating a system bus or NetworkManager object. This
  // makes mutating wlan0 (or any other managed Wi-Fi interface) unreachable.
  EXPECT_EQ(api::ImplementationPlatform::CreateWifiHotspotMedium(), nullptr);
  EXPECT_EQ(api::ImplementationPlatform::CreateWifiDirectMedium(), nullptr);
  EXPECT_TRUE(GetWifiMediumPolicy().wifi_lan);
}

}  // namespace
}  // namespace nearby::linux
