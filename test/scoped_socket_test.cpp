#include "gtest/gtest.h"
#include "../src/ScopedSocket.h"

TEST(ScopedSocket, SuccessfulConstruction) {
  int fd;
  {
    ScopedSocket socket();
    
    EXPECT_NE(fd, -1);
  }
}

TEST(ScopedSocketTest, MoveConstructorTransfersOwnership)
{
  ScopedSocket socket(0);

  int fd = socket.get();
  ScopedSocket second(std::move(socket.get()));

  EXPECT_EQ(socket.get(), -1);
  EXPECT_EQ(second.get(), fd);
}

TEST(ScopedSocketTest, MoveAssignmentTransfersOwnership)
{
  ScopedSocket first(0);
  ScopedSocket second(1);

  int fd = first.get();

  second = std::move(first);

  EXPECT_EQ(first.get(), -1);
  EXPECT_EQ(second.get(), fd);
}