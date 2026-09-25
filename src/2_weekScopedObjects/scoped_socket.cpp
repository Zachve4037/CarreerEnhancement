#include "scoped_socket.h"

#include <cerrno>
#include <system_error>

#include <sys/socket.h>
#include <unistd.h>

scoped_socket::scoped_socket(int protocol) {
  fd_ = ::socket(AF_INET, SOCK_STREAM, protocol);

  if (fd_ == -1) {
    throw std::system_error(
        errno,
        std::generic_category(),
        "socket creation failed"
    );
  }
}

scoped_socket::~scoped_socket() {
  if (fd_ != -1) {
    ::close(fd_);
  }
}

scoped_socket::scoped_socket(scoped_socket&& other) noexcept
    : fd_(other.fd_) {
  other.fd_ = -1;
}

scoped_socket& scoped_socket::operator=(
    scoped_socket&& other
) noexcept {
  if (this != &other) {
    if (fd_ != -1) {
      ::close(fd_);
    }

    fd_ = other.fd_;
    other.fd_ = -1;
  }

  return *this;
}

int scoped_socket::get() const noexcept {
  return fd_;
}