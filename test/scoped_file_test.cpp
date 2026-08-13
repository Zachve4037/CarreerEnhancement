#include "gtest/gtest.h"
#include "../src/scoped_file.h"

TEST(ScopedFile, FailedConstruction) {
  EXPECT_THROW(scoped_file file("this-is-non-existent"), std::system_error);
}

TEST(ScopedFile, SuccessfulConstruction) {
  int fd;
  {
    scoped_file file("../../test/data/empty.txt");
    fd = file.get();
    
    EXPECT_NE(fd, -1);
  }
}

TEST(ScopedFileTest, MoveConstructorTransfersOwnership)
{
  scoped_file first("../../test/data/empty.txt");

  int fd = first.get();

  scoped_file second(std::move(first));

  EXPECT_EQ(first.get(), -1);
  EXPECT_EQ(second.get(), fd);
}

TEST(ScopedFileTest, MoveAssignmentTransfersOwnership)
{
  scoped_file first("../../test/data/empty.txt");
  scoped_file second("../../test/data/empty.txt");

  int fd = first.get();

  second = std::move(first);

  EXPECT_EQ(first.get(), -1);
  EXPECT_EQ(second.get(), fd);
}