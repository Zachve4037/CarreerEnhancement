#pragma once

#include "socket_utils.h"

#include <cstdint>
#include <string>
#include <string_view>

class TcpClient {
public:
  TcpClient(
      std::string host,
      std::uint16_t port,
      int timeout_ms = 5000,
      int max_reconnect_attempts = 3
  );

  ~TcpClient();

  // This class owns the socket and cannot be copied.
  TcpClient(const TcpClient&) = delete;
  TcpClient& operator=(const TcpClient&) = delete;

  void connect();
  void disconnect() noexcept;

  [[nodiscard]] bool connected() const noexcept;

  // Sends a message, receives the echo, and attempts reconnection
  // if the connection fails.
  std::string echo(std::string_view message);

private:
  void reconnect();

  std::string host_;
  std::uint16_t port_;
  int timeout_ms_;
  int max_reconnect_attempts_;

  int socket_fd_{-1};
};