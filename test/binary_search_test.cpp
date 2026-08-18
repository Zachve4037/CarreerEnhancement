//
// Created by zachvem on 18-Aug-26.
//
#include <gtest/gtest.h>
#include "../src/binary_search.h"

TEST(BinarySearchTest, Empty) {
  std::span<int> s = std::span<int>{};
  std::optional<int> res = binary_search(s, 10);
  EXPECT_FALSE(res.has_value());
}

TEST(BinarySearchTest, Success) {
  std::vector<int> v = {1, 2, 3, 4, 5};
  std::span<int> s(v);
  std::optional<int> res = binary_search(s, 3);
  EXPECT_EQ(res, 2);
}

TEST(BinarySearchTest, Duplicates) {
  std::vector<int> v = {1, 2, 3, 3, 4, 5};
  std::span<int> s(v);
  std::optional<int> res = binary_search(s, 3);
  EXPECT_EQ(res, 2);
}

TEST(BinarySearchTest, OneElement) {
  std::vector<int> v {1};
  std::span<int> s = std::span(v);
  std::optional<int> res = binary_search(s, 1);
  EXPECT_EQ(res, 0);
}

TEST(BinarySearchTest, TwoElements) {
  std::vector<int> v {1, 2};
  std::span<int> s = std::span(v);
  std::optional<int> res = binary_search(s, 2);
  EXPECT_EQ(res, 1);
}

TEST(BinarySearchTest, First) {
  std::vector<int> v = {1, 2, 3, 3, 4, 5};
  std::span<int> s(v);
  std::optional<int> res = binary_search(s, 1);
  EXPECT_EQ(res, 0);
}

TEST(BinarySearchTest, Last) {
  std::vector<int> v = {1, 2, 3, 3, 4, 5};
  std::span<int> s(v);
  std::optional<int> res = binary_search(s, 5);
  EXPECT_EQ(res, 5);
}

TEST(BinarySearchTest, SmallerThanEverything) {
  std::vector<int> v = {2, 3, 3, 4, 5};
  std::span<int> s(v);
  std::optional<int> res = binary_search(s, 1);
  EXPECT_FALSE(res.has_value());
}

TEST(BinarySearchTest, BiggerThanEverything) {
  std::vector<int> v = {1, 2, 3, 3, 4, 5};
  std::span<int> s(v);
  std::optional<int> res = binary_search(s, 6);
  EXPECT_FALSE(res.has_value());
}