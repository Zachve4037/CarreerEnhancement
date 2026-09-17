//
// Created by zachvem on 16-Sep-26.
//

#ifndef CPPFIRST_SOCKET_UTILS_H
#define CPPFIRST_SOCKET_UTILS_H
#include <cstddef>
#include <cstdint>
#include <string>

namespace socket_utils {
  void send_all(int socket_fd, const void* data, std::size_t size);
  void receive_all(int socket_fd, void* data, std::size_t size);
  int create_server_socket(std::uint16_t port);
  int connect_to_server(const std::string& host, std::uint16_t port);
  void set_receive_timeout(int socket_fd, int timeout_ms);
};

#endif // CPPFIRST_SOCKET_UTILS_H
