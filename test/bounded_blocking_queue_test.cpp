//
// Created by zachvem on 24-Sep-26.
//

#include "../src/8_week_threads_atomics_and_bounded_queues/bounded_blocking_queue.h"
#include "gtest/gtest.h"
#include <chrono>
#include <future>

using namespace std::chrono_literals;

TEST(bounded_blocking_queue_test, ConstructorRejectsCapacity_0) {
  EXPECT_THROW(BoundedBlockingQueue<int> queue(0), std::invalid_argument);
}

TEST(bounded_blocking_queue_test, PushAndPopOneInteger) {
  BoundedBlockingQueue<int> queue(1);
  queue.push(1);
  EXPECT_EQ(queue.pop(), 1);
}

TEST(bounded_blocking_queue_test, FifoOrdering) {
  BoundedBlockingQueue<int> queue(3);
  queue.push(1);
  queue.push(2);
  queue.push(3);
  EXPECT_EQ(queue.pop(), 1);
  EXPECT_EQ(queue.pop(), 2);
  EXPECT_EQ(queue.pop(), 3);
}

TEST(bounded_blocking_queue_test, FullCapactiy) {
  BoundedBlockingQueue<int> queue(1);
  queue.push(1);
  EXPECT_EQ(queue.size(), 1);
  EXPECT_EQ(queue.pop(), 1);
}

TEST(bounded_blocking_queue_test, ConsumerBlockedAndAwakened) {
  BoundedBlockingQueue<int> queue(1);

  auto consumer =
      std::async(std::launch::async, [&queue] { return queue.pop(); });

  EXPECT_EQ(consumer.wait_for(100ms), std::future_status::timeout);

  queue.push(42);

  EXPECT_EQ(consumer.get(), 42);
}

TEST(bounded_blocking_queue_test, ProducerBlockedAndAwakened) {
  BoundedBlockingQueue<int> queue(1);

  queue.push(1);

  auto producer = std::async(std::launch::async, [&queue] { queue.push(2); });

  EXPECT_EQ(producer.wait_for(100ms), std::future_status::timeout);

  EXPECT_EQ(queue.pop(), 1);
  producer.get();
  EXPECT_EQ(queue.pop(), 2);
}