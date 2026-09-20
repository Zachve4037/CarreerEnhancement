//
// Created by zachvem on 16-Sep-26.
//

#include "socket_utils.h"
#include <system_error>

void socket_utils::send_all(int socket_fd, const void* data, std::size_t size) {
 const char* current = static_cast<const char*>(data);
 std::size_t remaining = size;

 while (remaining > 0) {
  const ssize_t n = ::send(socket_fd, current, remaining, 0);

  if (n < 0) {
   throw std::system_error(
    errno,
    std::generic_category(),
    "failed send"
    );
  }
  remaining -= static_cast<std::size_t>(n);
  current += n;
  if (remaining == 0) {
   break;
  }
 }
}

void socket_utils::receive_all(int socket_fd, void* data, std::size_t size) {
 char* current = static_cast<char*>(data);
 std::size_t remaining = size;

 while (remaining > 0) {
  const ssize_t n = ::recv(socket_fd, current, remaining, 0);

  if (n < 0) {
   throw std::system_error(
    errno,
    std::generic_category(),
    "failed receive"
    );
  }
  if (n == 0) {
   throw std::runtime_error("connection closed while receiving data");
  }
  remaining -= static_cast<std::size_t>(n);
  current += n;
  if (remaining == 0) {
   break;
  }
 }
}