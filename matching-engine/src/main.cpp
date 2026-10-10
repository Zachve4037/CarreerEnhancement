//
// Created by zachvem on 10-Oct-26.
//
#include "orderbook/command_parser.h"
#include "orderbook/order.h"

#include <iostream>
#include <string>
#include <type_traits>
#include <variant>

int main() {
  std::string line;

  while (std::getline(std::cin, line)) {
    const CommandResult result = parse_command(line);

    if (const auto* error = std::get_if<CommandError>(&result)) {
      std::cout << "REJECTED: "
                << error_message(*error) << '\n';
      continue;
    }

    const auto& command = std::get<Command>(result);

    std::visit([](const auto& cmd) {
        using T = std::decay_t<decltype(cmd)>;

        if constexpr (std::is_same_v<T, NewOrder>) {
            std::cout << "VALID NEW ORDER: id=" << cmd.id
                      << ", side="
                      << (cmd.side == Side::Buy ? "BUY" : "SELL")
                      << ", price=" << cmd.price
                      << ", quantity=" << cmd.quantity << '\n';
        } else {
            std::cout << "VALID CANCEL COMMAND: id="
                      << cmd.id << '\n';
        }
    }, command);
  }

  return 0;
}