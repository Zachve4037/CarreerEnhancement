#include "gtest/gtest.h"
#include "../src/2_weekScopedObjects/scoped_timer.h"

TEST(ScopedTimerTest, CreatesTimer) {
    scoped_timer timer;

    EXPECT_NE(timer.get(), -1);
}

TEST(ScopedTimerTest, MoveTransfersOwnership) {
    scoped_timer first;

    int fd = first.get();

    scoped_timer second(std::move(first));

    EXPECT_EQ(first.get(), -1);
    EXPECT_EQ(second.get(), fd);
}