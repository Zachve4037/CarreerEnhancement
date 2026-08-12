//
// Created by zachvem on 12-Aug-26.
//

#include "ScopedFile.h"

#include <fcntl.h>
#include <unistd.h>
#include <system_error>
#include <cerrno>

ScopedFile::ScopedFile(const char *path) {
  this->fd_ = ::open(path, O_RDONLY);
  if (this->fd_ == -1) {
    throw std::system_error(
      errno,
      std::generic_category(),
      "Failed to open file!");
  }
}

ScopedFile::~ScopedFile() {
  if (this->fd_ != -1) {
    ::close(fd_);
  }
}

ScopedFile::ScopedFile(ScopedFile&& other) noexcept : fd_(other.fd_) {
  other.fd_ = -1;
}

ScopedFile& ScopedFile::operator=(ScopedFile&& other) noexcept {
  if (this != &other) {
    if (this->fd_ != -1) {
      ::close(this->fd_);
    }
    this->fd_ = other.fd_;
    other.fd_ = -1;
  }
  return *this;
};

int ScopedFile::get() const noexcept {
  return this->fd_;
}