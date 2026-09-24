//
// Created by zachvem on 24-Sep-26.
//

#ifndef CPPFIRST_BOUNDED_BLOCKING_QUEUE_H
#define CPPFIRST_BOUNDED_BLOCKING_QUEUE_H
#include <cstddef>
#include <mutex>
#include <queue>
#include <condition_variable>
template <typename T>
class BoundedBlockingQueue {
public:
  explicit BoundedBlockingQueue(std::size_t capacity)
    : capacity_(capacity) {
    if (capacity == 0) {
      throw std::invalid_argument("queue capacity must be greater than zero");
    }
  };
  void push(T item) {
    std::unique_lock<std::mutex> lock(mutex_);

    not_full_.wait(lock, [this] {
      return queue_.size() < capacity_;
    });
    queue_.push(item);
    lock.release();
    //notify waiting consumer
  }
  T pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    not_empty_.wait(lock, [this] {
      return queue_.size() == capacity_;
    });
    T return_item = queue_.pop();
    lock.release();
    //notify waiting producer
    return return_item;
  }
  std::size_t size() const {
    return queue_.size();
  }
  std::size_t capacity() const {
    return capacity_;
  }

private:
  const std::size_t capacity_;
  mutable std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  std::queue<T> queue_;
};

#endif // CPPFIRST_BOUNDED_BLOCKING_QUEUE_H
