//
// Created by zachvem on 12-Aug-26.
//

#include "scoped_file.h"

#include <cerrno>
#include <fcntl.h>
#include <system_error>
#include <unistd.h>

scoped_file::scoped_file(const char *path) {
  this->fd_ = ::open(path, O_RDONLY);
  if (this->fd_ == -1) {
    throw std::system_error(
      errno,
      std::generic_category(),
      "Failed to open file!");
  }
}

scoped_file::~scoped_file() {
  if (this->fd_ != -1) {
    ::close(fd_);
  }
}

scoped_file::scoped_file(scoped_file&& other) noexcept : fd_(other.fd_) {
  other.fd_ = -1;
}

scoped_file& scoped_file::operator=(scoped_file&& other) noexcept {
  if (this != &other) {
    if (this->fd_ != -1) {
      ::close(this->fd_);
    }
    this->fd_ = other.fd_;
    other.fd_ = -1;
  }
  return *this;
};

int scoped_file::get() const noexcept {
  return this->fd_;
}