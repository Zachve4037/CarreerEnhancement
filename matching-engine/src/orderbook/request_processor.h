#ifndef CPPFIRST_REQUEST_PROCESSOR_H
#define CPPFIRST_REQUEST_PROCESSOR_H

#include "order_book.h"

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

struct ReplaceOrder {
    OrderId old_id;
    NewOrder replacement;

    bool operator==(const ReplaceOrder&) const = default;
};

// Define all variant alternatives before creating the variant.
using RequestId = std::uint64_t;
using ClientRequest = std::variant<NewOrder, CancelOrder, ReplaceOrder>;

enum class RequestError {
    None,
    InvalidRequestId,
    DuplicateRequestIdWithDifferentPayload,
    OutOfOrderCommand,
    InvalidOrderId,
    InvalidSide,
    InvalidPrice,
    InvalidQuantity,
    DuplicateOrderId,
    UnknownOrder,
    ReplacementFailed
};

struct RequestResult {
    bool accepted{false};
    std::vector<ExecutionEvent> executions;
    RequestError error{RequestError::None};
};

// One entry in the deterministic command history.
struct CommandRecord {
    RequestId request_id;
    Sequence command_sequence;
    ClientRequest request;
};

class RequestProcessor {
public:
    [[nodiscard]] RequestResult process(
        RequestId request_id,
        const ClientRequest& request) {

        const auto existing = processed_requests_.find(request_id);

        // A retry must return the original result, not consume a new sequence.
        if (existing != processed_requests_.end()) {
            if (existing->second.request == request) {
                return existing->second.result;
            }

            return rejected(
                RequestError::DuplicateRequestIdWithDifferentPayload);
        }

        return process(request_id, next_command_sequence_, request);
    }

    // Explicit sequence interface: used by replay and out-of-order checks.
    [[nodiscard]] RequestResult process(
        RequestId request_id,
        Sequence command_sequence,
        const ClientRequest& request) {

        if (request_id == 0) {
            return rejected(RequestError::InvalidRequestId);
        }

        const auto existing = processed_requests_.find(request_id);

        if (existing != processed_requests_.end()) {
            if (existing->second.request == request &&
                existing->second.command_sequence == command_sequence) {
                return existing->second.result;
            }

            return rejected(
                RequestError::DuplicateRequestIdWithDifferentPayload);
        }

        // Only the next expected command may modify the book.
        // Out-of-order commands are not cached, so the caller can retry them
        // when their turn arrives.
        if (command_sequence != next_command_sequence_) {
            return rejected(RequestError::OutOfOrderCommand);
        }

        RequestResult result = execute(request);

        // Record both accepted and rejected in-order commands.
        processed_requests_.emplace(
            request_id,
            CachedRequest{command_sequence, request, result});

        history_.push_back(
            CommandRecord{request_id, command_sequence, request});

        // Every in-order command consumes a command sequence, even if its
        // business operation was rejected.
        ++next_command_sequence_;

        return result;
    }

    // Reapply a command history to this processor.
    // Call this on a fresh RequestProcessor to reconstruct its state.
    [[nodiscard]] std::vector<RequestResult> replay(
        const std::vector<CommandRecord>& records) {

        std::vector<RequestResult> results;
        results.reserve(records.size());

        for (const auto& record : records) {
            results.push_back(process(
                record.request_id,
                record.command_sequence,
                record.request));
        }

        return results;
    }

    [[nodiscard]] const OrderBook& order_book() const noexcept {
        return order_book_;
    }

    [[nodiscard]] const std::vector<CommandRecord>& history() const noexcept {
        return history_;
    }

    [[nodiscard]] Sequence next_command_sequence() const noexcept {
        return next_command_sequence_;
    }

    [[nodiscard]] Sequence next_order_sequence() const noexcept {
        return next_order_sequence_;
    }

private:
    struct CachedRequest {
        Sequence command_sequence;
        ClientRequest request;
        RequestResult result;
    };

    OrderBook order_book_;

    std::unordered_map<RequestId, CachedRequest> processed_requests_;
    std::vector<CommandRecord> history_;

    // These sequences have different responsibilities.
    Sequence next_command_sequence_{1};
    Sequence next_order_sequence_{1};

    [[nodiscard]] static RequestResult rejected(RequestError error) {
        return RequestResult{false, {}, error};
    }

    [[nodiscard]] static RequestError map_validation_error(
        ValidationError error) {

        switch (error) {
            case ValidationError::InvalidOrderId:
                return RequestError::InvalidOrderId;
            case ValidationError::InvalidSide:
                return RequestError::InvalidSide;
            case ValidationError::InvalidPrice:
                return RequestError::InvalidPrice;
            case ValidationError::InvalidQuantity:
                return RequestError::InvalidQuantity;
        }

        return RequestError::InvalidQuantity;
    }

    [[nodiscard]] RequestResult execute(const ClientRequest& request) {
        if (const auto* command = std::get_if<NewOrder>(&request)) {
            return execute_new(*command);
        }

        if (const auto* command = std::get_if<CancelOrder>(&request)) {
            return execute_cancel(*command);
        }

        if (const auto* command = std::get_if<ReplaceOrder>(&request)) {
            return execute_replace(*command);
        }

        return rejected(RequestError::ReplacementFailed);
    }

    [[nodiscard]] RequestResult execute_new(const NewOrder& command) {
        if (const auto error = validate(command)) {
            return rejected(map_validation_error(*error));
        }

        if (order_book_.contains_order(command.id)) {
            return rejected(RequestError::DuplicateOrderId);
        }

        Order order(
            command.id,
            command.side,
            command.price,
            command.quantity,
            next_order_sequence_);

        auto executions = order_book_.process_order(order);

        ++next_order_sequence_;

        return RequestResult{true, std::move(executions), RequestError::None};
    }

    [[nodiscard]] RequestResult execute_cancel(const CancelOrder& command) {
        if (const auto error = validate(command)) {
            return rejected(map_validation_error(*error));
        }

        if (!order_book_.contains_order(command.id)) {
            return rejected(RequestError::UnknownOrder);
        }

        if (!order_book_.cancel_order(command.id)) {
            return rejected(RequestError::UnknownOrder);
        }

        return RequestResult{true, {}, RequestError::None};
    }

    [[nodiscard]] RequestResult execute_replace(const ReplaceOrder& command) {
        if (const auto error = validate(command.replacement)) {
            return rejected(map_validation_error(*error));
        }

        if (!order_book_.contains_order(command.old_id)) {
            return rejected(RequestError::UnknownOrder);
        }

        if (order_book_.contains_order(command.replacement.id)) {
            return rejected(RequestError::DuplicateOrderId);
        }

        Order replacement(
            command.replacement.id,
            command.replacement.side,
            command.replacement.price,
            command.replacement.quantity,
            next_order_sequence_);

        auto executions = order_book_.replace_order(
            command.old_id,
            replacement);

        if (!executions.has_value()) {
            return rejected(RequestError::ReplacementFailed);
        }

        ++next_order_sequence_;

        return RequestResult{
            true,
            std::move(*executions),
            RequestError::None};
    }
};

#endif // CPPFIRST_REQUEST_PROCESSOR_H
