#include "command_parser.h"

#include <charconv>
#include <sstream>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>

namespace {

template <typename T>
bool parse_integer(const std::string& text, T& value) {
    static_assert(std::is_integral_v<T>);

    if (text.empty()) {
        return false;
    }

    const char* begin = text.data();
    const char* end = begin + text.size();

    const auto [ptr, ec] = std::from_chars(begin, end, value);

    return ec == std::errc{} && ptr == end;
}

}

CommandResult parse_command(std::string_view input) {
    std::istringstream stream{std::string(input)};
    std::vector<std::string> tokens;

    for (std::string token; stream >> token;) {
        tokens.push_back(token);
    }

    if (tokens.empty()) {
        return CommandError::EmptyInput;
    }

    if (tokens[0] == "NEW") {
        if (tokens.size() < 5) {
            return CommandError::MissingArgument;
        }

        if (tokens.size() > 5) {
            return CommandError::UnexpectedArgument;
        }

        OrderId id{};
        Price price{};
        Quantity quantity{};

        if (!parse_integer(tokens[1], id)) {
            return CommandError::InvalidInteger;
        }

        Side side;
        if (tokens[2] == "BUY") {
            side = Side::Buy;
        } else if (tokens[2] == "SELL") {
            side = Side::Sell;
        } else {
            return CommandError::InvalidSide;
        }

        if (!parse_integer(tokens[3], price)) {
            return CommandError::InvalidInteger;
        }

        if (!parse_integer(tokens[4], quantity)) {
            return CommandError::InvalidInteger;
        }

        const NewOrder command{id, side, price, quantity};

        if (const auto error = validate(command)) {
            switch (*error) {
            case ValidationError::InvalidOrderId:
                return CommandError::InvalidOrderId;
            case ValidationError::InvalidSide:
                return CommandError::InvalidSide;
            case ValidationError::InvalidPrice:
                return CommandError::InvalidPrice;
            case ValidationError::InvalidQuantity:
                return CommandError::InvalidQuantity;
            }
        }

        return Command{command};
    }

    if (tokens[0] == "CANCEL") {
        if (tokens.size() < 2) {
            return CommandError::MissingArgument;
        }

        if (tokens.size() > 2) {
            return CommandError::UnexpectedArgument;
        }

        OrderId id{};

        if (!parse_integer(tokens[1], id)) {
            return CommandError::InvalidInteger;
        }

        const CancelOrder command{id};

        if (const auto error = validate(command)) {
            switch (*error) {
            case ValidationError::InvalidOrderId:
                return CommandError::InvalidOrderId;
            default:
                break;
            }
        }

        return Command{command};
    }

    return CommandError::UnknownCommand;
}

const char* error_message(CommandError error) noexcept {
    switch (error) {
    case CommandError::EmptyInput:
        return "Input is empty";
    case CommandError::UnknownCommand:
        return "Unknown command; expected NEW or CANCEL";
    case CommandError::MissingArgument:
        return "Command is missing an argument";
    case CommandError::UnexpectedArgument:
        return "Command has too many arguments";
    case CommandError::InvalidInteger:
        return "An integer argument is invalid or out of range";
    case CommandError::InvalidOrderId:
        return "Order ID must be greater than zero";
    case CommandError::InvalidSide:
        return "Side must be BUY or SELL";
    case CommandError::InvalidPrice:
        return "Price must be greater than zero";
    case CommandError::InvalidQuantity:
        return "Quantity must be greater than zero";
    }

    return "Unknown error";
}