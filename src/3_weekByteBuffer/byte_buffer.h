//
// Created by zachvem on 20-Aug-26.
//

#ifndef CPPFIRST_BYTE_BUFFER_H
#define CPPFIRST_BYTE_BUFFER_H

#include <cstddef>
#include <cstdint>
#include <algorithm>

class byte_buffer {
public:
  explicit byte_buffer(std::size_t size);
  ~byte_buffer();

  byte_buffer(const byte_buffer& other);
  byte_buffer& operator=(const byte_buffer& other);

  byte_buffer(byte_buffer&& other) noexcept;
  byte_buffer& operator=(byte_buffer&& other) noexcept;

  std::size_t size() const noexcept;
  std::byte* data() noexcept;
  const std::byte* data() const noexcept;

private:
  std::byte* data_ = nullptr;
  std::size_t size_ = 0;
};

#endif // CPPFIRST_BYTE_BUFFER_H
