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

#include "internal/platform/implementation/linux/stream.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string>

#include "internal/platform/byte_array.h"
#include "internal/platform/exception.h"
#include "internal/platform/implementation/linux/tcp_server_socket.h"
#include "internal/platform/implementation/linux/wifi_lan.h"
#include "gtest/gtest.h"

namespace nearby::linux {
namespace {

TEST(InputStreamTest, ReturnsBufferedBytesBeforePeerReset) {
  int listener = socket(AF_INET, SOCK_STREAM, 0);
  ASSERT_GE(listener, 0);

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  address.sin_port = 0;
  ASSERT_EQ(
      bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)),
      0);
  ASSERT_EQ(listen(listener, 1), 0);

  socklen_t address_size = sizeof(address);
  ASSERT_EQ(getsockname(listener, reinterpret_cast<sockaddr*>(&address),
                        &address_size),
            0);

  int peer = socket(AF_INET, SOCK_STREAM, 0);
  ASSERT_GE(peer, 0);
  ASSERT_EQ(
      connect(peer, reinterpret_cast<sockaddr*>(&address), sizeof(address)), 0);
  int stream_fd = accept(listener, nullptr, nullptr);
  ASSERT_GE(stream_fd, 0);
  close(listener);

  constexpr char kMessage[] = "buffered-before-reset";
  ASSERT_EQ(send(peer, kMessage, std::strlen(kMessage), MSG_NOSIGNAL),
            std::strlen(kMessage));

  linger reset_on_close{.l_onoff = 1, .l_linger = 0};
  ASSERT_EQ(setsockopt(peer, SOL_SOCKET, SO_LINGER, &reset_on_close,
                       sizeof(reset_on_close)),
            0);
  close(peer);

  InputStream stream(stream_fd);
  ExceptionOr<ByteArray> result = stream.Read(sizeof(kMessage));
  EXPECT_EQ(result.exception(), Exception::kSuccess);
  EXPECT_EQ(result.result().AsStringView(), kMessage);
  EXPECT_EQ(stream.Read(1).exception(), Exception::kIo);

  close(stream_fd);
}

TEST(InputStreamTest, OrderlyPeerCloseReturnsEndOfStream) {
  int sockets[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
  close(sockets[1]);

  InputStream stream(sockets[0]);
  ExceptionOr<ByteArray> result = stream.Read(1);
  EXPECT_EQ(result.exception(), Exception::kSuccess);
  EXPECT_TRUE(result.result().Empty());

  close(sockets[0]);
}

TEST(InputStreamTest, InvalidDescriptorReturnsIoError) {
  int sockets[2];
  ASSERT_EQ(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets), 0);
  InputStream stream(sockets[0]);
  close(sockets[0]);
  close(sockets[1]);

  EXPECT_EQ(stream.Read(1).exception(), Exception::kIo);
}

TEST(TCPServerSocketTest, WifiLanUsesFirewallPermittedPort) {
  EXPECT_EQ(GetWifiLanDynamicPortRange(),
            std::make_pair(kQuixShareWifiLanPort, kQuixShareWifiLanPort));
  EXPECT_EQ(kQuixShareWifiLanPort, 53317);
}

TEST(TCPServerSocketTest, ReusesPortAfterConnectedListenerCloses) {
  std::string loopback = "127.0.0.1";
  auto first = TCPServerSocket::Listen(std::ref(loopback), /*port=*/0);
  ASSERT_TRUE(first.has_value());
  const int port = first->GetPort();
  ASSERT_GT(port, 0);

  auto client = TCPSocket::Connect(loopback, port);
  ASSERT_TRUE(client.has_value());
  auto accepted = first->Accept();
  ASSERT_TRUE(accepted.has_value());

  EXPECT_TRUE(accepted->Close().Ok());
  EXPECT_TRUE(client->Close().Ok());
  EXPECT_TRUE(first->Close().Ok());

  auto second = TCPServerSocket::Listen(std::ref(loopback), port);
  ASSERT_TRUE(second.has_value());
  EXPECT_EQ(second->GetPort(), port);
  EXPECT_TRUE(second->Close().Ok());
}

}  // namespace
}  // namespace nearby::linux
