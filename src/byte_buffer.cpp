//
// Created by zachvem on 20-Aug-26.
//

#include "byte_buffer.h"

byte_buffer::byte_buffer(std::size_t size)
    : data_(size == 0 ? nullptr : new std::byte[size]), size_(size) {}

byte_buffer::~byte_buffer() {
  if (this->size_ != 0 || this->data_ != nullptr) {
    delete[] this->data_;
  }
}

byte_buffer::byte_buffer(const byte_buffer &other)
    : size_(other.size_), data_(new std::byte[other.size_]) {
  std::copy(other.data_, other.data_ + other.size_, data_);
}

byte_buffer &byte_buffer::operator=(const byte_buffer &other) {
  if (this != &other) {
    delete[] this->data_;
    this->size_ = other.size_;
    this->data_ = new std::byte[other.size_];
    std::copy(other.data_, other.data_ + other.size_, data_);
  }
  return *this;
}

byte_buffer::byte_buffer(byte_buffer &&other) noexcept
    : size_(other.size_), data_(other.data_) {
  other.size_ = 0;
  other.data_ = nullptr;
}

byte_buffer &byte_buffer::operator=(byte_buffer &&other) noexcept {
  if (this != &other) {
    delete[] this->data_;
    this->size_ = other.size_;
    this->data_ = other.data_;
    other.size_ = 0;
    other.data_ = nullptr;
  }
  return *this;
}

std::size_t byte_buffer::size() const noexcept { return this->size_; }

std::byte *byte_buffer::data() noexcept { return this->data_; }

const std::byte *byte_buffer::data() const noexcept { return this->data_; }