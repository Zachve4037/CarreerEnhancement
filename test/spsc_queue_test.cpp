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

//empty queue
//basic push/pop
//full queue
//wraparound - pop some items push more verify FIFO order
//spsc concurrency - one producer and one consumer transfer
//  many sequential integers without loss or reordering