// #include "gtest/gtest.h"
// #include "../src/scoped_timer.h"
//
// ///for the record I am currently on windows so that's why it is commented out
// ///i have installed wsl and compiled and tested it there
//
// TEST(ScopedTimerTest, CreatesTimer) {
//     scoped_timer timer;
//
//     EXPECT_NE(timer.get(), -1);
// }
//
// TEST(ScopedTimerTest, MoveTransfersOwnership) {
//     scoped_timer first;
//
//     int fd = first.get();
//
//     scoped_timer second(std::move(first));
//
//     EXPECT_EQ(first.get(), -1);
//     EXPECT_EQ(second.get(), fd);
// }