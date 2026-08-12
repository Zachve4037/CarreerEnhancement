//
// Created by zachvem on 12-Aug-26.
//

#include "scoped_socket.h"

#include <system_error>
#include <winsock2.h>

scoped_socket::scoped_socket(int protocol) {
  this->fd_ = socket(AF_INET, SOCK_STREAM, protocol);
  if (this->fd_ == -1) {
    throw std::system_error(
      errno,
      std::generic_category(),
      "Failed to create socket!");
  }
}

scoped_socket::~scoped_socket() {
  if (this->fd_ != -1) {
    ::closesocket(this->fd_);
  }
}

scoped_socket::scoped_socket(scoped_socket &&other) noexcept : fd_(other.fd_) {
  other.fd_ = -1;
}

scoped_socket &scoped_socket::operator=(scoped_socket &&other) noexcept {
  if (this != &other) {
    if (this->fd_ != -1) {
      ::closesocket(this->fd_);
    }
    this->fd_ = other.fd_;
    other.fd_ = -1;
  }
  return *this;
}

int scoped_socket::get() const noexcept {
  return this->fd_;
}