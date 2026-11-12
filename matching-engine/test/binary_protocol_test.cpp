#include "../src/orderbook/binary_protocol.h"
#include "../src/orderbook/replay_log.h"
#include "random_order_stream.h"

#include <gtest/gtest.h>

#include <array>
#include <limits>

namespace {
using namespace binary_protocol;

Bytes command_bytes(const CommandRecord& record) {
    return std::get<Bytes>(encode_command(record));
}

Bytes event_bytes(const ResultEvent& event) {
    return std::get<Bytes>(encode_event(event));
}

void expect_same_result(const RequestResult& actual, const RequestResult& expected) {
    EXPECT_EQ(actual.accepted, expected.accepted);
    EXPECT_EQ(actual.error, expected.error);
    ASSERT_EQ(actual.executions.size(), expected.executions.size());
    for (std::size_t index = 0; index < actual.executions.size(); ++index) {
        EXPECT_EQ(actual.executions[index].buy_order_id_, expected.executions[index].buy_order_id_);
        EXPECT_EQ(actual.executions[index].sell_order_id_, expected.executions[index].sell_order_id_);
        EXPECT_EQ(actual.executions[index].execution_price_, expected.executions[index].execution_price_);
        EXPECT_EQ(actual.executions[index].executed_quantity_, expected.executions[index].executed_quantity_);
    }
}

TEST(BinaryProtocol, CommandWireFixtures) {
    const Bytes expected_new{
        1, 1, 41, 0, 0, 0,
        8, 7, 6, 5, 4, 3, 2, 1,
        2, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0, 1,
        4, 0, 0, 0, 0, 0, 0, 0,
        5, 0, 0, 0, 0, 0, 0, 0};
    const CommandRecord order{0x0102030405060708ULL, 2, NewOrder{3, Side::Sell, 4, 5}};
    EXPECT_EQ(command_bytes(order), expected_new);
    EXPECT_EQ(std::get<CommandRecord>(decode_command(expected_new)).request, order.request);

    const Bytes expected_cancel{
        1, 2, 24, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0,
        2, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(command_bytes({1, 2, CancelOrder{3}}), expected_cancel);
    const Bytes expected_replace{
        1, 3, 49, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0,
        2, 0, 0, 0, 0, 0, 0, 0,
        3, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0, 0,
        5, 0, 0, 0, 0, 0, 0, 0,
        6, 0, 0, 0, 0, 0, 0, 0};
    EXPECT_EQ(command_bytes({1, 2, ReplaceOrder{3, {4, Side::Buy, 5, 6}}}), expected_replace);
}

TEST(BinaryProtocol, EventWireFixtures) {
    const Bytes expected{
        1, 4, 50, 0, 0, 0,
        1, 0, 0, 0, 0, 0, 0, 0,
        2, 0, 0, 0, 0, 0, 0, 0, 1, 0,
        3, 0, 0, 0, 0, 0, 0, 0,
        4, 0, 0, 0, 0, 0, 0, 0,
        5, 0, 0, 0, 0, 0, 0, 0,
        6, 0, 0, 0, 0, 0, 0, 0};
    const ResultEvent event{1, 2, {true, {{3, 4, 5, 6}}, RequestError::None}};
    EXPECT_EQ(event_bytes(event), expected);
    expect_same_result(std::get<ResultEvent>(decode_event(expected)).result, event.result);
    auto rejected = Bytes{1, 4, 18, 0, 0, 0,
                          1, 0, 0, 0, 0, 0, 0, 0,
                          2, 0, 0, 0, 0, 0, 0, 0, 0, 9};
    EXPECT_EQ(event_bytes({1, 2, {false, {}, RequestError::UnknownOrder}}), rejected);
}

TEST(BinaryProtocol, AllCommandTypesPreserveBoundaryValues) {
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    for (const auto price : {std::numeric_limits<Price>::min(), Price{-1}, Price{0},
                             std::numeric_limits<Price>::max()}) {
        const NewOrder order{maximum, Side::Buy, price, maximum};
        for (const ClientRequest& request : {ClientRequest{order}, ClientRequest{CancelOrder{0}},
                                             ClientRequest{ReplaceOrder{maximum, order}}}) {
            const CommandRecord original{maximum, maximum, request};
            const auto decoded = std::get<CommandRecord>(decode_command(command_bytes(original)));
            EXPECT_EQ(decoded.request_id, original.request_id);
            EXPECT_EQ(decoded.command_sequence, original.command_sequence);
            EXPECT_EQ(decoded.request, original.request);
        }
    }
    auto negative = command_bytes({0, 0, NewOrder{0, Side::Sell, -1, 0}});
    for (std::size_t index = 31; index < 39; ++index) {
        EXPECT_EQ(negative[index], 255);
    }
}

TEST(BinaryProtocol, EveryRequestErrorRoundTripsWithStableCode) {
    const std::array errors{
        RequestError::None, RequestError::InvalidRequestId,
        RequestError::DuplicateRequestIdWithDifferentPayload, RequestError::OutOfOrderCommand,
        RequestError::InvalidOrderId, RequestError::InvalidSide, RequestError::InvalidPrice,
        RequestError::InvalidQuantity, RequestError::DuplicateOrderId,
        RequestError::UnknownOrder, RequestError::ReplacementFailed};
    for (std::size_t code = 0; code < errors.size(); ++code) {
        const RequestResult result{code == 0, {}, errors[code]};
        const auto bytes = event_bytes({9, 7, result});
        EXPECT_EQ(bytes[23], code);
        const auto decoded = std::get<ResultEvent>(decode_event(bytes));
        EXPECT_EQ(decoded.request_id, 9);
        EXPECT_EQ(decoded.command_sequence, 7);
        expect_same_result(decoded.result, result);
    }
}

TEST(BinaryProtocol, EventPreservesFullWidthFields) {
    const auto maximum = std::numeric_limits<std::uint64_t>::max();
    const ResultEvent original{maximum, maximum,
        {true, {{maximum, maximum, std::numeric_limits<Price>::max(), maximum},
                {0, 0, std::numeric_limits<Price>::min(), 0}}, RequestError::None}};
    const auto decoded = std::get<ResultEvent>(decode_event(event_bytes(original)));
    EXPECT_EQ(decoded.request_id, maximum);
    EXPECT_EQ(decoded.command_sequence, maximum);
    expect_same_result(decoded.result, original.result);
}

TEST(BinaryProtocol, SingleByteMutationsEitherFailOrRemainCanonical) {
    const std::vector<Bytes> frames{
        command_bytes({1, 1, NewOrder{1, Side::Buy, 100, 10}}),
        command_bytes({2, 2, CancelOrder{1}}),
        command_bytes({3, 3, ReplaceOrder{1, {2, Side::Sell, 101, 10}}}),
        event_bytes({1, 1, {true, {{1, 2, 100, 10}}, RequestError::None}})};
    for (const auto& original : frames) {
        for (std::size_t offset = 0; offset < original.size(); ++offset) {
            for (unsigned value = 0; value <= 255; ++value) {
                auto mutated = original;
                mutated[offset] = static_cast<std::uint8_t>(value);
                if (original[1] == 4) {
                    const auto decoded = decode_event(mutated);
                    if (const auto* event = std::get_if<ResultEvent>(&decoded)) {
                        EXPECT_EQ(event_bytes(*event), mutated);
                    }
                } else {
                    const auto decoded = decode_command(mutated);
                    if (const auto* command = std::get_if<CommandRecord>(&decoded)) {
                        EXPECT_EQ(command_bytes(*command), mutated);
                    }
                }
            }
        }
    }
}

TEST(BinaryProtocol, RejectsEveryTruncatedPrefixAndTrailingBytes) {
    const std::vector<Bytes> frames{
        command_bytes({1, 1, NewOrder{1, Side::Buy, 100, 10}}),
        command_bytes({2, 2, CancelOrder{1}}),
        command_bytes({3, 3, ReplaceOrder{1, {2, Side::Sell, 101, 10}}}),
        event_bytes({1, 1, {true, {}, RequestError::None}}),
        event_bytes({1, 1, {true, {{1, 2, 100, 10}, {1, 3, 101, 5}}, RequestError::None}})};
    for (auto frame : frames) {
        const bool event = frame[1] == 4;
        for (std::size_t length = 0; length < frame.size(); ++length) {
            const auto prefix = std::span<const std::uint8_t>(frame).first(length);
            if (event) {
                EXPECT_EQ(std::get<ProtocolError>(decode_event(prefix)), ProtocolError::TruncatedFrame);
            } else {
                EXPECT_EQ(std::get<ProtocolError>(decode_command(prefix)), ProtocolError::TruncatedFrame);
            }
        }
        frame.push_back(0);
        if (event) {
            EXPECT_EQ(std::get<ProtocolError>(decode_event(frame)), ProtocolError::InvalidLength);
        } else {
            EXPECT_EQ(std::get<ProtocolError>(decode_command(frame)), ProtocolError::InvalidLength);
        }
    }
}

TEST(BinaryProtocol, RejectsInvalidHeadersLengthsAndSides) {
    const auto original = command_bytes({1, 1, NewOrder{1, Side::Buy, 100, 10}});
    auto frame = original;
    frame[0] = 2;
    EXPECT_EQ(std::get<ProtocolError>(decode_command(frame)), ProtocolError::UnsupportedVersion);
    frame = original;
    frame[1] = 255;
    EXPECT_EQ(std::get<ProtocolError>(decode_command(frame)), ProtocolError::UnknownMessageType);
    EXPECT_EQ(std::get<ProtocolError>(decode_event(original)), ProtocolError::UnknownMessageType);
    EXPECT_EQ(std::get<ProtocolError>(decode_command(event_bytes({1, 1, {true, {}, RequestError::None}}))),
              ProtocolError::UnknownMessageType);
    frame = original;
    frame[2] = 40;
    frame.pop_back();
    EXPECT_EQ(std::get<ProtocolError>(decode_command(frame)), ProtocolError::InvalidLength);
    frame = original;
    for (std::size_t index = 2; index < 6; ++index) {
        frame[index] = 255;
    }
    EXPECT_EQ(std::get<ProtocolError>(decode_command(frame)), ProtocolError::TruncatedFrame);
    for (const auto& record : {CommandRecord{1, 1, NewOrder{1, Side::Buy, 1, 1}},
                               CommandRecord{1, 1, ReplaceOrder{1, {2, Side::Sell, 1, 1}}}}) {
        frame = command_bytes(record);
        frame[record.request.index() == 0 ? 30 : 38] = 2;
        EXPECT_EQ(std::get<ProtocolError>(decode_command(frame)), ProtocolError::InvalidValue);
    }
    EXPECT_EQ(std::get<ProtocolError>(encode_command({1, 1, NewOrder{1, static_cast<Side>(2), 1, 1}})),
              ProtocolError::InvalidValue);
}

TEST(BinaryProtocol, RejectsMalformedResultPayloads) {
    const auto original = event_bytes({1, 1, {true, {}, RequestError::None}});
    for (const auto [offset, value] : {std::pair{22, 2}, std::pair{23, 255}, std::pair{23, 9}, std::pair{22, 0}}) {
        auto frame = original;
        frame[offset] = static_cast<std::uint8_t>(value);
        EXPECT_EQ(std::get<ProtocolError>(decode_event(frame)), ProtocolError::InvalidValue);
    }
    for (const auto size : {17U, 19U, 49U}) {
        auto frame = original;
        frame.resize(header_size + size);
        frame[2] = static_cast<std::uint8_t>(size);
        EXPECT_EQ(std::get<ProtocolError>(decode_event(frame)), ProtocolError::InvalidLength);
    }
    auto rejected_with_fill = event_bytes({1, 1, {true, {{1, 2, 100, 1}}, RequestError::None}});
    rejected_with_fill[22] = 0;
    rejected_with_fill[23] = 9;
    EXPECT_EQ(std::get<ProtocolError>(decode_event(rejected_with_fill)), ProtocolError::InvalidValue);
    for (const auto& result : {RequestResult{true, {}, RequestError::UnknownOrder},
                               RequestResult{false, {}, RequestError::None},
                               RequestResult{false, {}, static_cast<RequestError>(255)},
                               RequestResult{false, {{1, 2, 100, 1}}, RequestError::UnknownOrder}}) {
        EXPECT_EQ(std::get<ProtocolError>(encode_event({1, 1, result})), ProtocolError::InvalidValue);
    }
}

TEST(BinaryProtocol, ProcessingRetriesRejectionsAndReplayMatchDirectCalls) {
    RequestProcessor direct;
    RequestProcessor binary;
    const std::vector<CommandRecord> commands{
        {1, 1, NewOrder{1, Side::Sell, 100, 5}},
        {2, 2, NewOrder{2, Side::Sell, 101, 7}},
        {3, 3, NewOrder{3, Side::Buy, 101, 8}},
        {3, 3, NewOrder{3, Side::Buy, 101, 8}},
        {3, 3, CancelOrder{2}},
        {4, 5, CancelOrder{2}},
        {4, 4, ReplaceOrder{2, {4, Side::Sell, 99, 4}}},
        {5, 5, CancelOrder{4}},
        {6, 6, CancelOrder{4}},
        {7, 7, NewOrder{0, Side::Buy, 100, 1}},
        {8, 8, NewOrder{5, Side::Buy, -1, 1}},
        {9, 9, NewOrder{5, Side::Buy, 100, 0}},
        {0, 10, CancelOrder{1}}};
    for (const auto& command : commands) {
        const auto decoded = std::get<CommandRecord>(decode_command(command_bytes(command)));
        const auto result = binary.process(decoded.request_id, decoded.command_sequence, decoded.request);
        const auto event = std::get<ResultEvent>(decode_event(event_bytes({decoded.request_id, decoded.command_sequence, result})));
        expect_same_result(event.result, direct.process(command.request_id, command.command_sequence, command.request));
        EXPECT_EQ(binary.order_book().snapshot(), direct.order_book().snapshot());
        EXPECT_EQ(binary.next_command_sequence(), direct.next_command_sequence());
    }
    RequestProcessor replayed;
    const auto results = replayed.replay(binary.history());
    EXPECT_EQ(results.size(), binary.history().size());
    EXPECT_EQ(replayed.order_book().snapshot(), binary.order_book().snapshot());
}

TEST(BinaryProtocol, RandomStreamPreservesHistoryAndExecutionChecksums) {
    RequestProcessor direct;
    RequestProcessor binary;
    RandomOrderStream stream(0x12345678);
    for (Sequence sequence = 1; sequence <= 2000; ++sequence) {
        const CommandRecord record{sequence, sequence, stream.next(direct.order_book())};
        const auto decoded = std::get<CommandRecord>(decode_command(command_bytes(record)));
        const auto expected = direct.process(record.request_id, record.command_sequence, record.request);
        const auto actual = binary.process(decoded.request_id, decoded.command_sequence, decoded.request);
        expect_same_result(std::get<ResultEvent>(decode_event(event_bytes({sequence, sequence, actual}))).result, expected);
    }
    EXPECT_EQ(binary.order_book().snapshot(), direct.order_book().snapshot());
    EXPECT_EQ(replay_log::command_stream_checksum(0x12345678, binary.history()),
              replay_log::command_stream_checksum(0x12345678, direct.history()));
}

}
