#pragma once

#include <cstdint>

class TcpServer {
public:
  explicit TcpServer(std::uint16_t port);

  // Blocks while the server is running.
  // Returns when stop() is called.
  void run();

  // Requests that the server stop.
  void stop() noexcept;

private:
  void handle_client(int client_fd);

  std::uint16_t port_;
  int server_fd_{-1};
  bool running_{false};
};