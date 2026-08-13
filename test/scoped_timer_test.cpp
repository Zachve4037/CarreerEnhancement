#include "gtest/gtest.h"
#include "../src/scoped_timer.h"

// I am running windows so for this case i am not making tests

TEST(ScopedTimerTest, CreatesTimer) {
    ScodpedTimer timer;

    EXPECT_NE(timer.get(), -1);
}

TEST(ScopedTimerTest, MoveTransfersOwnership) {
    ScopedTimer first;

    int fd = first.get();

    ScopedTimer second(std::move(first));

    EXPECT_EQ(first.get(), -1);
    EXPECT_EQ(second.get(), fd);
}