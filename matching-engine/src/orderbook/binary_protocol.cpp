#include "binary_protocol.h"

#include <bit>
#include <limits>
#include <optional>
#include <utility>

namespace binary_protocol {
namespace {

enum class MessageType : std::uint8_t { New = 1, Cancel = 2, Replace = 3, Result = 4 };

void append_integer(Bytes& bytes, std::uint64_t value, std::size_t width) {
    for (std::size_t index = 0; index < width; ++index) {
        bytes.push_back(static_cast<std::uint8_t>(value >> (index * 8)));
    }
}

std::uint64_t read_integer(std::span<const std::uint8_t> bytes,
                           std::size_t& offset, std::size_t width) {
    std::uint64_t value = 0;
    for (std::size_t index = 0; index < width; ++index) {
        value |= static_cast<std::uint64_t>(bytes[offset++]) << (index * 8);
    }
    return value;
}

Bytes make_frame(MessageType type, std::size_t payload_size) {
    Bytes bytes;
    bytes.reserve(header_size + payload_size);
    bytes.push_back(version);
    bytes.push_back(static_cast<std::uint8_t>(type));
    append_integer(bytes, payload_size, 4);
    return bytes;
}

std::optional<ProtocolError> check_frame(std::span<const std::uint8_t> frame) {
    if (frame.size() < header_size) {
        return ProtocolError::TruncatedFrame;
    }
    if (frame[0] != version) {
        return ProtocolError::UnsupportedVersion;
    }
    std::size_t offset = 2;
    const auto payload_size = read_integer(frame, offset, 4);
    if (payload_size > frame.size() - header_size) {
        return ProtocolError::TruncatedFrame;
    }
    if (payload_size != frame.size() - header_size) {
        return ProtocolError::InvalidLength;
    }
    return std::nullopt;
}

bool valid_side(Side side) {
    return side == Side::Buy || side == Side::Sell;
}

void append_order(Bytes& bytes, const NewOrder& order) {
    append_integer(bytes, order.id, 8);
    bytes.push_back(order.side == Side::Buy ? 0 : 1);
    append_integer(bytes, std::bit_cast<std::uint64_t>(order.price), 8);
    append_integer(bytes, order.quantity, 8);
}

NewOrder read_order(std::span<const std::uint8_t> frame, std::size_t& offset) {
    const auto id = read_integer(frame, offset, 8);
    const auto side = frame[offset++] == 0 ? Side::Buy : Side::Sell;
    const auto price = std::bit_cast<Price>(read_integer(frame, offset, 8));
    const auto quantity = read_integer(frame, offset, 8);
    return NewOrder{id, side, price, quantity};
}

bool valid_result(bool accepted, RequestError error, std::size_t execution_count) {
    const auto code = static_cast<unsigned>(error);
    return code <= static_cast<unsigned>(RequestError::ReplacementFailed) &&
           accepted == (error == RequestError::None) &&
           (accepted || execution_count == 0);
}

}

EncodeResult encode_command(const CommandRecord& command) {
    MessageType type;
    std::size_t payload_size;
    const auto* order = std::get_if<NewOrder>(&command.request);
    const auto* cancel = std::get_if<CancelOrder>(&command.request);
    const auto* replace = std::get_if<ReplaceOrder>(&command.request);
    if (order) {
        type = MessageType::New;
        payload_size = 41;
    } else if (cancel) {
        type = MessageType::Cancel;
        payload_size = 24;
    } else if (replace) {
        type = MessageType::Replace;
        payload_size = 49;
        order = &replace->replacement;
    } else {
        return ProtocolError::InvalidValue;
    }
    if (order && !valid_side(order->side)) {
        return ProtocolError::InvalidValue;
    }
    auto bytes = make_frame(type, payload_size);
    append_integer(bytes, command.request_id, 8);
    append_integer(bytes, command.command_sequence, 8);
    if (cancel) {
        append_integer(bytes, cancel->id, 8);
    } else {
        if (replace) {
            append_integer(bytes, replace->old_id, 8);
        }
        append_order(bytes, *order);
    }
    return bytes;
}

CommandDecodeResult decode_command(std::span<const std::uint8_t> frame) {
    if (const auto error = check_frame(frame)) {
        return *error;
    }
    const auto type = static_cast<MessageType>(frame[1]);
    std::size_t expected_size;
    switch (type) {
    case MessageType::New: expected_size = 41; break;
    case MessageType::Cancel: expected_size = 24; break;
    case MessageType::Replace: expected_size = 49; break;
    default: return ProtocolError::UnknownMessageType;
    }
    if (frame.size() - header_size != expected_size) {
        return ProtocolError::InvalidLength;
    }
    std::size_t offset = header_size;
    const auto request_id = read_integer(frame, offset, 8);
    const auto sequence = read_integer(frame, offset, 8);
    if (type == MessageType::Cancel) {
        return CommandRecord{request_id, sequence, CancelOrder{read_integer(frame, offset, 8)}};
    }
    const auto old_id = type == MessageType::Replace ? read_integer(frame, offset, 8) : 0;
    if (frame[offset + 8] > 1) {
        return ProtocolError::InvalidValue;
    }
    const auto order = read_order(frame, offset);
    if (type == MessageType::Replace) {
        return CommandRecord{request_id, sequence, ReplaceOrder{old_id, order}};
    }
    return CommandRecord{request_id, sequence, order};
}

EncodeResult encode_event(const ResultEvent& event) {
    const auto& result = event.result;
    if (!valid_result(result.accepted, result.error, result.executions.size())) {
        return ProtocolError::InvalidValue;
    }
    if (result.executions.size() > (std::numeric_limits<std::uint32_t>::max() - 18) / 32) {
        return ProtocolError::FrameTooLarge;
    }
    auto bytes = make_frame(MessageType::Result, 18 + result.executions.size() * 32);
    append_integer(bytes, event.request_id, 8);
    append_integer(bytes, event.command_sequence, 8);
    bytes.push_back(result.accepted ? 1 : 0);
    bytes.push_back(static_cast<std::uint8_t>(result.error));
    for (const auto& execution : result.executions) {
        append_integer(bytes, execution.buy_order_id_, 8);
        append_integer(bytes, execution.sell_order_id_, 8);
        append_integer(bytes, std::bit_cast<std::uint64_t>(execution.execution_price_), 8);
        append_integer(bytes, execution.executed_quantity_, 8);
    }
    return bytes;
}

EventDecodeResult decode_event(std::span<const std::uint8_t> frame) {
    if (const auto error = check_frame(frame)) {
        return *error;
    }
    if (frame[1] != static_cast<std::uint8_t>(MessageType::Result)) {
        return ProtocolError::UnknownMessageType;
    }
    const auto payload_size = frame.size() - header_size;
    if (payload_size < 18 || (payload_size - 18) % 32 != 0) {
        return ProtocolError::InvalidLength;
    }
    std::size_t offset = header_size;
    const auto request_id = read_integer(frame, offset, 8);
    const auto sequence = read_integer(frame, offset, 8);
    const auto accepted = frame[offset++];
    const auto error = static_cast<RequestError>(frame[offset++]);
    const auto count = (payload_size - 18) / 32;
    if (accepted > 1 || !valid_result(accepted == 1, error, count)) {
        return ProtocolError::InvalidValue;
    }
    RequestResult result{accepted == 1, {}, error};
    result.executions.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        const auto buy_id = read_integer(frame, offset, 8);
        const auto sell_id = read_integer(frame, offset, 8);
        const auto price = std::bit_cast<Price>(read_integer(frame, offset, 8));
        const auto quantity = read_integer(frame, offset, 8);
        result.executions.push_back(ExecutionEvent{buy_id, sell_id, price, quantity});
    }
    return ResultEvent{request_id, sequence, std::move(result)};
}

}
