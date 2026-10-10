#ifndef CPPFIRST_COMMAND_PARSER_H
#define CPPFIRST_COMMAND_PARSER_H

#include "order.h"

#include <string_view>
#include <variant>

enum class CommandError {
  EmptyInput,
  UnknownCommand,
  MissingArgument,
  UnexpectedArgument,
  InvalidInteger,
  InvalidOrderId,
  InvalidSide,
  InvalidPrice,
  InvalidQuantity
};

using Command = std::variant<NewOrder, CancelOrder>;
using CommandResult = std::variant<Command, CommandError>;

[[nodiscard]]
CommandResult parse_command(std::string_view input);

[[nodiscard]]
const char* error_message(CommandError error) noexcept;

#endif // CPPFIRST_COMMAND_PARSER_H