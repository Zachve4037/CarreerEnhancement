//
// Created by zachvem on 16-Sep-26.
//

#include "socket_utils.h"

#include "frame.h"

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
    "byte overflow"
    );
  }
  remaining -= n;
  if (remaining == 0) {
   break;
  }
 }
}