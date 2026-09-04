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

#include "internal/platform/implementation/linux/bluetooth_bluez_profile.h"

#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <thread>

#include "absl/synchronization/mutex.h"
#include "absl/synchronization/notification.h"
#include "absl/time/time.h"
#include "internal/platform/cancellation_flag.h"
#include "internal/platform/cancellation_flag_listener.h"
#include "internal/platform/implementation/linux/bluetooth_classic_socket.h"
#include "gtest/gtest.h"

namespace nearby::linux {
namespace {

TEST(ProfileConnectionWaitTest, AcceptsEventDeliveredAfterWaitStarts) {
  absl::Mutex mutex;
  absl::Notification start_delivery;
  bool connection_delivered = false;

  std::thread deliver_connection;
  {
    absl::MutexLock lock(&mutex);
    deliver_connection = std::thread([&]() {
      start_delivery.WaitForNotification();
      absl::MutexLock lock(&mutex);
      connection_delivered = true;
    });
    auto connected = [&]() { return connection_delivered; };
    start_delivery.Notify();

    EXPECT_TRUE(profile_internal::AwaitProfileEvent(
        &mutex, absl::Condition(&connected), absl::Seconds(1)));
    EXPECT_TRUE(connection_delivered);
  }
  deliver_connection.join();
}

TEST(ProfileConnectionWaitTest, ReturnsWhenBoundedWaitExpires) {
  absl::Mutex mutex;
  bool connection_delivered = false;
  auto connected = [&]() { return connection_delivered; };

  absl::MutexLock lock(&mutex);
  EXPECT_FALSE(profile_internal::AwaitProfileEvent(
      &mutex, absl::Condition(&connected), absl::Milliseconds(1)));
}

TEST(ProfileConnectionWaitTest, CancellationWakesInfiniteWait) {
  absl::Mutex mutex;
  absl::Notification start_cancellation;
  CancellationFlag cancelled;
  CancellationFlagListener listener(&cancelled, [&mutex]() {
    mutex.Lock();
    mutex.Unlock();
  });

  std::thread cancel;
  {
    absl::MutexLock lock(&mutex);
    cancel = std::thread([&]() {
      start_cancellation.WaitForNotification();
      cancelled.Cancel();
    });
    auto was_cancelled = [&]() { return cancelled.Cancelled(); };
    start_cancellation.Notify();

    EXPECT_TRUE(profile_internal::AwaitProfileEvent(
        &mutex, absl::Condition(&was_cancelled), absl::InfiniteDuration()));
    EXPECT_TRUE(cancelled.Cancelled());
  }
  cancel.join();
}

TEST(BluetoothSocketLifecycleTest, DestructorClosesFdAndSignalsPeerHangup) {
  int fds[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);

  {
    BluetoothSocket socket(nullptr, sdbus::UnixFd(fds[0]));
  }

  pollfd peer{};
  peer.fd = fds[1];
  peer.events = POLLIN;
  ASSERT_GT(poll(&peer, 1, 1000), 0);
  EXPECT_NE(peer.revents & POLLHUP, 0);
  close(fds[1]);
}

}  // namespace
}  // namespace nearby::linux
