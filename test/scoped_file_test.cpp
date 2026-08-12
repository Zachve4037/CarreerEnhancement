#include "gtest/gtest.h"
#include "../src/ScopedFile.h"

TEST(ScopedFile, FailedConstruction) {
  EXPECT_THROW(ScopedFile file("this-is-non-existent"), std::system_error);
}

TEST(ScopedFile, SuccessfulConstruction) {
  int fd;
  {
    ScopedFile file("../../test/data/empty.txt");
    fd = file.get();
    
    EXPECT_NE(fd, -1);
  }
}

TEST(ScopedFileTest, MoveConstructorTransfersOwnership)
{
  ScopedFile first("../../test/data/empty.txt");

  int fd = first.get();

  ScopedFile second(std::move(first));

  EXPECT_EQ(first.get(), -1);
  EXPECT_EQ(second.get(), fd);
}

TEST(ScopedFileTest, MoveAssignmentTransfersOwnership)
{
  ScopedFile first("../../test/data/empty.txt");
  ScopedFile second("../../test/data/empty.txt");

  int fd = first.get();

  second = std::move(first);

  EXPECT_EQ(first.get(), -1);
  EXPECT_EQ(second.get(), fd);
}