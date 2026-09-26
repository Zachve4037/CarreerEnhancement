//
// Created by zachvem on 24-Sep-26.
//

#ifndef CPPFIRST_SPSC_QUEUE_H
#define CPPFIRST_SPSC_QUEUE_H
#include <atomic>
#include <cstddef>
#include <memory>
#include <stdexcept>

template <typename T> class SPSCQueue {
public:
  explicit SPSCQueue(std::size_t capacity)
      : capacity_(capacity == 0
                      ? throw std::invalid_argument("capacity cannot be 0")
                      : capacity),
        buffer_(std::make_unique<T[]>(capacity_)){}

  // returns false if queue is full
  bool try_push(const T &item) {
    const std::size_t tail = tail_.load(std::memory_order_acquire);
    const std::size_t head = head_.load(std::memory_order_relaxed);
    if (head - tail == capacity_) {
      return false;
    }
    buffer_[head % capacity_] = item;
    head_.store(head + 1, std::memory_order_release);
    return true;
  }

  // returns false if the queue is empty
  bool try_pop(T &item) {
    const std::size_t tail = tail_.load(std::memory_order_relaxed);
    const std::size_t head = head_.load(std::memory_order_acquire);
    if (head == tail) {
      return false;
    }
    item = std::move(buffer_[tail % capacity_]);
    tail_.store(tail + 1, std::memory_order_release);
    return true;
  }

  std::size_t capacity() const noexcept { return capacity_; }

private:
  const std::size_t capacity_;
  std::unique_ptr<T[]> buffer_;

  // producer writes to the head
  std::atomic<std::size_t> head_{0};
  // consumer writes to the tail
  std::atomic<std::size_t> tail_{0};
};
#endif // CPPFIRST_SPSC_QUEUE_H
