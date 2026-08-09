#include <gtest/gtest.h>

#include "../src/file_stats.h"

TEST(FileStatsTest, EmptyFile) {
  auto stats = analyzeFile("../../test/data/empty.txt");

  EXPECT_EQ(stats.lines, 0);
  EXPECT_EQ(stats.words, 0);
  EXPECT_EQ(stats.bytes, 0);
}

TEST(FileStatsTest, OneLineFile) {
  auto stats = analyzeFile("../../test/data/one_line.txt");

  EXPECT_EQ(stats.lines, 1);
  EXPECT_EQ(stats.words, 3);
  EXPECT_EQ(stats.bytes, 15);
}

TEST(FileStatsTest, SampleFile) {
  auto stats = analyzeFile("../../test/data/sample.txt");

  EXPECT_EQ(stats.lines, 6);
  EXPECT_EQ(stats.words, 98);
  EXPECT_EQ(stats.bytes, 626);
}

TEST(FileStatsTest, NotFoundFile) {
  EXPECT_ANY_THROW(analyzeFile("not-existing-file.tst"));
}