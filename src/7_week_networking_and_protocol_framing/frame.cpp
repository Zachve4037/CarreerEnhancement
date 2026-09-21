//
// Created by zachvem on 16-Sep-26.
//

#include "frame.h"

#include "socket_utils.h"
#include <winsock.h>
#include <cstdint>
#include <system_error>
void frame::send(int socket_fd, std::string_view payload) {
  if (payload.size() > MAX_FRAME_SIZE) {
    throw std::invalid_argument("frame is too large");
  }
  auto length = static_cast<std::uint32_t>(payload.size());
  std::uint32_t network_length = htonl(length);
  socket_utils::send_all(socket_fd, &network_length, sizeof(network_length));
  socket_utils::send_all(socket_fd, payload.data(), payload.size());
}

std::string frame::receive(int socket_fd) {
  std::uint32_t network_length;
  socket_utils::receive_all(socket_fd, &network_length, sizeof(network_length));
  const std::uint32_t length = ntohl(network_length);
  if (length > MAX_FRAME_SIZE) {
    throw std::runtime_error("frame is too large");
  }
  std::string payload(length, '\0');
  socket_utils::receive_all(socket_fd, payload.data(), payload.size());
  return payload;
}