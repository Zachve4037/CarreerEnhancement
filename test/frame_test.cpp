#include "../src/7_week_networking_and_protocol_framing/frame.h"

#include <gtest/gtest.h>

#include <sys/socket.h>
#include <unistd.h>

#include <string>
#include <thread>

class FrameTest : public ::testing::Test {
protected:
    int sockets_[2]{-1, -1};

    void SetUp() override {
        // AF_UNIX socketpair is a convenient local test connection.
        ASSERT_EQ(
            ::socketpair(AF_UNIX, SOCK_STREAM, 0, sockets_),
            0
        );
    }

    void TearDown() override {
        if (sockets_[0] != -1) {
            ::close(sockets_[0]);
        }

        if (sockets_[1] != -1) {
            ::close(sockets_[1]);
        }
    }
};

TEST_F(FrameTest, SendsAndReceivesNormalMessage) {
    frame::send(sockets_[0], "hello");

    const std::string result = frame::receive(sockets_[1]);

    EXPECT_EQ(result, "hello");
}

TEST_F(FrameTest, SupportsEmptyMessage) {
    frame::send(sockets_[0], "");

    const std::string result = frame::receive(sockets_[1]);

    EXPECT_TRUE(result.empty());
}

TEST_F(FrameTest, SupportsLargeMessage) {
    const std::string message(10000, 'a');

    // A separate thread avoids blocking if the socket buffer fills.
    std::thread sender([&]() {
        frame::send(sockets_[0], message);
    });

    const std::string result = frame::receive(sockets_[1]);

    sender.join();

    EXPECT_EQ(result, message);
}

TEST_F(FrameTest, SupportsMultipleFrames) {
    frame::send(sockets_[0], "hello");
    frame::send(sockets_[0], "world");

    EXPECT_EQ(frame::receive(sockets_[1]), "hello");
    EXPECT_EQ(frame::receive(sockets_[1]), "world");
}

TEST_F(FrameTest, SupportsBinaryPayload) {
    // A string can contain null bytes; the protocol is length-based,
    // so it must not rely on null termination.
    const std::string message("A\0B\0C", 5);

    frame::send(sockets_[0], message);

    EXPECT_EQ(frame::receive(sockets_[1]), message);
}

TEST_F(FrameTest, RejectsOversizedOutgoingFrame) {
    const std::string message(frame::MAX_FRAME_SIZE + 1, 'x');

    EXPECT_THROW(
        frame::send(sockets_[0], message),
        std::invalid_argument
    );
}