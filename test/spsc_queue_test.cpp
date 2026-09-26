//
// Created by zachvem on 24-Sep-26.
//
#include "../src/8_week_threads_atomics_and_bounded_queues/spsc_queue.h"
#include "gtest/gtest.h"

TEST(spsc_queue_test, RejectsZeroCapacity) {
  EXPECT_THROW(SPSCQueue<int>(0), std::invalid_argument);
}

TEST(spsc_queue_test, StorecCapacity) {
  SPSCQueue<int> queue(8);
  EXPECT_EQ(queue.capacity(), 8);
}

TEST(spsc_queue_test, EmptyQueue) {
  SPSCQueue<int> queue(1);
  int item;
  EXPECT_EQ(queue.try_pop(item), false);
}

TEST(spsc_queue_test, PushAndPop) {
  SPSCQueue<int> queue(1);
  queue.try_push(1);
  int poped;
  EXPECT_EQ(queue.try_pop(poped), true);
  EXPECT_EQ(poped, 1);
}

TEST(spsc_queue_test, FullQueue) {
  SPSCQueue<int> queue(1);
  queue.try_push(1);
  EXPECT_EQ(queue.try_push(2), false);
}
//wraparound - pop some items push more verify FIFO order

TEST(spsc_queue_test, FIFO) {
  SPSCQueue<int> queue(3);
  queue.try_push(1);
  queue.try_push(2);
  queue.try_push(3);
  int poped;
  queue.try_pop(poped);
  EXPECT_EQ(poped, 1);
  queue.try_pop(poped);
  EXPECT_EQ(poped, 2);
  queue.try_pop(poped);
  EXPECT_EQ(poped, 3);
}

TEST(spsc_queue_test, ConcurrentProducerConsumer)
{
  constexpr std::size_t num_items = 100'000;
  constexpr std::size_t capacity = 128;

  SPSCQueue<int> queue(capacity);

  std::atomic<bool> failed{false};

  std::thread producer([&]() {
      for (std::size_t i = 0; i < num_items; ++i) {
          while (!queue.try_push(static_cast<int>(i))) {
              if (failed.load(std::memory_order_relaxed)) {
                  return;
              }

              std::this_thread::yield();
          }
      }
  });

  std::thread consumer([&]() {
      for (std::size_t i = 0; i < num_items; ++i) {
          int value = -1;

          while (!queue.try_pop(value)) {
              if (failed.load(std::memory_order_relaxed)) {
                  return;
              }

              std::this_thread::yield();
          }

          if (value != static_cast<int>(i)) {
              failed.store(true, std::memory_order_relaxed);
              ADD_FAILURE() << "Expected " << i
                            << ", got " << value;
              return;
          }
      }
  });

  producer.join();
  consumer.join();

  EXPECT_FALSE(failed.load(std::memory_order_relaxed));
}