#include "../src/orderbook/command_parser.h"

#include <gtest/gtest.h>
#include <variant>

TEST(CommandParser, ParsesBuyOrder) {
    const auto result = parse_command("NEW 1 BUY 10500 10");

    ASSERT_TRUE(std::holds_alternative<Command>(result));

    const auto& command = std::get<Command>(result);
    ASSERT_TRUE(std::holds_alternative<NewOrder>(command));

    const auto& order = std::get<NewOrder>(command);
    EXPECT_EQ(order.id, 1);
    EXPECT_EQ(order.side, Side::Buy);
    EXPECT_EQ(order.price, 10500);
    EXPECT_EQ(order.quantity, 10);
}

TEST(CommandParser, ParsesCancelOrder) {
    const auto result = parse_command("CANCEL 42");

    ASSERT_TRUE(std::holds_alternative<Command>(result));

    const auto& command = std::get<Command>(result);
    ASSERT_TRUE(std::holds_alternative<CancelOrder>(command));

    EXPECT_EQ(std::get<CancelOrder>(command).id, 42);
}

TEST(CommandParser, RejectsEmptyInput) {
    const auto result = parse_command("   ");

    ASSERT_TRUE(std::holds_alternative<CommandError>(result));
    EXPECT_EQ(std::get<CommandError>(result), CommandError::EmptyInput);
}

TEST(CommandParser, RejectsUnknownCommand) {
    const auto result = parse_command("DELETE 1");

    ASSERT_TRUE(std::holds_alternative<CommandError>(result));
    EXPECT_EQ(std::get<CommandError>(result),
              CommandError::UnknownCommand);
}

TEST(CommandParser, RejectsInvalidPrice) {
    const auto result = parse_command("NEW 1 BUY 0 10");

    ASSERT_TRUE(std::holds_alternative<CommandError>(result));
    EXPECT_EQ(std::get<CommandError>(result),
              CommandError::InvalidPrice);
}

TEST(CommandParser, RejectsMissingArgument) {
    const auto result = parse_command("NEW 1 BUY 100");

    ASSERT_TRUE(std::holds_alternative<CommandError>(result));
    EXPECT_EQ(std::get<CommandError>(result),
              CommandError::MissingArgument);
}

TEST(CommandParser, RejectsExtraArgument) {
    const auto result = parse_command("CANCEL 1 extra");

    ASSERT_TRUE(std::holds_alternative<CommandError>(result));
    EXPECT_EQ(std::get<CommandError>(result),
              CommandError::UnexpectedArgument);
}

TEST(CommandParser, RejectsMalformedNumber) {
    const auto result = parse_command("NEW abc BUY 100 10");

    ASSERT_TRUE(std::holds_alternative<CommandError>(result));
    EXPECT_EQ(std::get<CommandError>(result),
              CommandError::InvalidInteger);
}