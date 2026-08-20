//
// Created by zachvem on 20-Aug-26.
//

#include "../src/byte_buffer.h"

#include "gtest/gtest.h"

TEST(byte_buffer, Construction) {
  byte_buffer buffer(10);
  EXPECT_EQ(buffer.size(), 10);
  EXPECT_TRUE(buffer.data() != nullptr);
}

TEST(byte_buffer, EmptyBuffer) {
  byte_buffer buffer(0);
  EXPECT_TRUE(buffer.data() == nullptr);
}

TEST(byte_buffer, CopyConstructor) {
  byte_buffer a(10);
  {
    byte_buffer b(a);
    EXPECT_EQ(a.size(), b.size());
  }
  EXPECT_TRUE(a.data() != nullptr);
}

TEST(byte_buffer, CopyAssignment) {
  byte_buffer a(10);
  {
    byte_buffer b(5);
    EXPECT_EQ(b.size(), 5);
    b = a;
    EXPECT_EQ(a.size(), b.size());
  }
  EXPECT_EQ(a.size(), 10);;
}

TEST(byte_buffer, MoveConstructor) {
  byte_buffer original(1024);
  byte_buffer moved(std::move(original));

  EXPECT_EQ(moved.size(), 1024);

  EXPECT_EQ(original.data(), nullptr);
  EXPECT_EQ(original.size(), 0);
}

TEST(byte_buffer, MoveAssignment) {
  byte_buffer original(1024);
  byte_buffer moved(10);

  moved = std::move(original);

  EXPECT_EQ(moved.size(), 1024);

  EXPECT_EQ(original.data(), nullptr);
  EXPECT_EQ(original.size(), 0);
}

