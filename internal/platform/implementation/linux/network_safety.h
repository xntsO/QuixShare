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

#ifndef PLATFORM_IMPL_LINUX_NETWORK_SAFETY_H_
#define PLATFORM_IMPL_LINUX_NETWORK_SAFETY_H_

#include <cstdlib>
#include <string_view>

namespace nearby::linux {

// Wi-Fi LAN only observes and uses the active network. Hotspot and Wi-Fi
// Direct can change NetworkManager state, so both require an explicit opt-in.
inline constexpr char kAllowWifiReconfigurationEnv[] =
    "QUIXSHARE_ALLOW_WIFI_RECONFIGURATION";

struct WifiMediumPolicy {
  bool wifi_lan;
  bool wifi_hotspot;
  bool wifi_direct;

  constexpr bool CanReconfigureManagedWifi() const {
    return wifi_hotspot || wifi_direct;
  }
};

constexpr WifiMediumPolicy WifiMediumPolicyForOptInValue(
    std::string_view opt_in_value) {
  const bool allow_reconfiguration = opt_in_value == "1";
  return {
      .wifi_lan = true,
      .wifi_hotspot = allow_reconfiguration,
      .wifi_direct = allow_reconfiguration,
  };
}

inline WifiMediumPolicy GetWifiMediumPolicy() {
  const char* opt_in_value = std::getenv(kAllowWifiReconfigurationEnv);
  return WifiMediumPolicyForOptInValue(opt_in_value == nullptr
                                           ? std::string_view()
                                           : std::string_view(opt_in_value));
}

}  // namespace nearby::linux

#endif  // PLATFORM_IMPL_LINUX_NETWORK_SAFETY_H_
