#include "../src/scoped_socket.h"
#include "gtest/gtest.h"

TEST(ScopedSocket, SuccessfulConstruction) {
  int fd;
  {
    scoped_socket socket();
    
    EXPECT_NE(fd, -1);
  }
}

TEST(ScopedSocketTest, MoveConstructorTransfersOwnership)
{
  scoped_socket socket(0);

  int fd = socket.get();
  scoped_socket second(std::move(socket.get()));

  EXPECT_EQ(socket.get(), -1);
  EXPECT_EQ(second.get(), fd);
}

TEST(ScopedSocketTest, MoveAssignmentTransfersOwnership)
{
  scoped_socket first(0);
  scoped_socket second(1);

  int fd = first.get();

  second = std::move(first);

  EXPECT_EQ(first.get(), -1);
  EXPECT_EQ(second.get(), fd);
}