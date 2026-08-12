//
// Created by zachvem on 12-Aug-26.
//

#include "ScopedSocket.h"

#include <system_error>
#include <winsock2.h>

ScopedSocket::ScopedSocket(int protocol) {
  this->fd_ = socket(AF_INET, SOCK_STREAM, protocol);
  if (this->fd_ == -1) {
    throw std::system_error(
      errno,
      std::generic_category(),
      "Failed to create socket!");
  }
}

ScopedSocket::~ScopedSocket() {
  if (this->fd_ != -1) {
    ::closesocket(this->fd_);
  }
}

ScopedSocket::ScopedSocket(ScopedSocket &&other) noexcept : fd_(other.fd_) {
  other.fd_ = -1;
}

ScopedSocket &ScopedSocket::operator=(ScopedSocket &&other) noexcept {
  if (this != &other) {
    if (this->fd_ != -1) {
      ::closesocket(this->fd_);
    }
    this->fd_ = other.fd_;
    other.fd_ = -1;
  }
  return *this;
}

int ScopedSocket::get() const noexcept {
  return this->fd_;
}