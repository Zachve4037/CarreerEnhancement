// #include "../src/scoped_socket.h"
// #include "gtest/gtest.h"
// #include <windows.h>
//
// TEST(ScopedSocket, SuccessfulConstruction) {
//   int fd;
//   {
//     scoped_socket scoped_socket(1);
//
//     EXPECT_NE(scoped_socket.get(), -1);
//   }
// }
//
// TEST(ScopedSocketTest, MoveConstructorTransfersOwnership)
// {
//
//   scoped_socket socket(0);
//
//   int fd = socket.get();
//   scoped_socket second(std::move(socket));
//
//   EXPECT_EQ(socket.get(), -1);
//   EXPECT_EQ(second.get(), fd);
// }
//
// TEST(ScopedSocketTest, MoveAssignmentTransfersOwnership)
// {
//   scoped_socket first(0);
//   scoped_socket second(1);1
//
//   int fd = first.get();
//
//   second = std::move(first);
//
//   EXPECT_EQ(first.get(), -1);
//   EXPECT_EQ(second.get(), fd);
// }