//
// Created by zachvem on 24-Sep-26.
//

#ifndef CPPFIRST_SPSC_QUEUE_H
#define CPPFIRST_SPSC_QUEUE_H
#include <atomic>
#include <cstddef>
#include <memory>

template <typename T>
class SPSCQueue {
public:
  explicit SPSCQueue(std::size_t capacity);

  //returns false if queue is full
  bool try_push(const T& item);

  //returns false if the queue is empty
  bool try_pop(T& item);

  std::size_t capacity() const noexcept;

private:
  const std::size_t capacity_;
  std::unique_ptr<T[]> buffer_;

  //producer writes to the head
  std::atomic<std::size_t> head_{0};
  //consumer writes to the tail
  std::atomic<std::size_t> tail{0};
};
#endif // CPPFIRST_SPSC_QUEUE_H
