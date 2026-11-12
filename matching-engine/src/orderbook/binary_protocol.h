#ifndef CPPFIRST_BINARY_PROTOCOL_H
#define CPPFIRST_BINARY_PROTOCOL_H

#include "request_processor.h"

#include <cstddef>
#include <cstdint>
#include <span>
#include <variant>
#include <vector>

namespace binary_protocol {

inline constexpr std::uint8_t version = 1;
inline constexpr std::size_t header_size = 6;

enum class ProtocolError {
    TruncatedFrame,
    UnsupportedVersion,
    UnknownMessageType,
    InvalidLength,
    InvalidValue,
    FrameTooLarge
};

struct ResultEvent {
    RequestId request_id;
    Sequence command_sequence;
    RequestResult result;
};

using Bytes = std::vector<std::uint8_t>;
using EncodeResult = std::variant<Bytes, ProtocolError>;
using CommandDecodeResult = std::variant<CommandRecord, ProtocolError>;
using EventDecodeResult = std::variant<ResultEvent, ProtocolError>;

[[nodiscard]] EncodeResult encode_command(const CommandRecord& command);
[[nodiscard]] CommandDecodeResult decode_command(std::span<const std::uint8_t> frame);
[[nodiscard]] EncodeResult encode_event(const ResultEvent& event);
[[nodiscard]] EventDecodeResult decode_event(std::span<const std::uint8_t> frame);

}

#endif
