//
// Created by zachvem on 02-Nov-26.
//

#ifndef CPPFIRST_REQUEST_PROCESSOR_H
#define CPPFIRST_REQUEST_PROCESSOR_H
#include "order_book.h"
#include <cstdint>
#include <unordered_map>
#include <variant>
#include <vector>

struct ReplaceOrder;
using RequestId = std::uint64_t;
using ClientRequest = std::variant<NewOrder, CancelOrder, ReplaceOrder>;

struct ReplaceOrder {
  OrderId old_id;
  NewOrder replacement;

  bool operator==(const ReplaceOrder &) const = default;
};

struct RequestResult {
  bool accepted;
  std::vector<ExecutionEvent> executions;
};

class RequestProcessor {
public:
  [[nodiscard]] RequestResult process(
    RequestId request_id,
    const ClientRequest& request) {

    // Request ID zero is reserved as invalid.
    if (request_id == 0) {
        return {false, {}};
    }

    // Check whether this request ID has already been processed.
    const auto existing = processed_requests_.find(request_id);

    if (existing != processed_requests_.end()) {

        // Identical request: return the previously stored result.
        if (existing->second.request == request) {
            return existing->second.result;
        }

        // Same ID, different payload: reject it.
        return {false, {}};
    }

    RequestResult result{false, {}};

    // Handle a new order.
    if (std::holds_alternative<NewOrder>(request)) {

        const auto& command = std::get<NewOrder>(request);

        if (!validate(command).has_value() &&
            !order_book_.contains_order(command.id)) {

            Order order(
                command.id,
                command.side,
                command.price,
                command.quantity,
                next_sequence_
            );

            result = {true, order_book_.process_order(order)};
            ++next_sequence_;
        }
    }

    // Handle cancellation.
    else if (std::holds_alternative<CancelOrder>(request)) {

        const auto& command = std::get<CancelOrder>(request);

        if (!validate(command).has_value()) {
            result = {
                order_book_.cancel_order(command.id),
                {}
            };
        }
    }

    // Handle replacement.
    else if (std::holds_alternative<ReplaceOrder>(request)) {

        const auto& command = std::get<ReplaceOrder>(request);

        if (!validate(command.replacement).has_value()) {

            Order replacement(
                command.replacement.id,
                command.replacement.side,
                command.replacement.price,
                command.replacement.quantity,
                next_sequence_
            );

            auto executions = order_book_.replace_order(
                command.old_id,
                replacement
            );

            if (executions.has_value()) {
                result = {true, std::move(*executions)};
                ++next_sequence_;
            }
        }
    }

    // Cache both successful and rejected requests.
    processed_requests_.emplace(
        request_id,
        CachedRequest{request, result}
    );

    return result;
}

  [[nodiscard]] const OrderBook& order_book() const noexcept {
    return order_book_;
  }

private:
  struct CachedRequest {
    ClientRequest request;
    RequestResult result;
  };

  OrderBook order_book_;

  std::unordered_map<RequestId, CachedRequest> processed_requests_;

  Sequence next_sequence_{1};
};

#endif // CPPFIRST_REQUEST_PROCESSOR_H
