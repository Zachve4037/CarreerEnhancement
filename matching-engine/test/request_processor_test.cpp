#include "../src/orderbook/request_processor.h"
#include <gtest/gtest.h>

// ------------------------------------------------------------
// Existing idempotency tests
// ------------------------------------------------------------

TEST(RequestProcessor, Repeated_new_request_is_not_processed_twice) {
    RequestProcessor processor;
    ClientRequest request = NewOrder{101, Side::Buy, 100, 5};

    auto first = processor.process(5001, request);
    auto second = processor.process(5001, request);

    EXPECT_TRUE(first.accepted);
    EXPECT_TRUE(second.accepted);
    EXPECT_TRUE(processor.order_book().contains_order(101));
    ASSERT_NE(processor.order_book().bid_level(100), nullptr);
    EXPECT_EQ(processor.order_book().bid_level(100)->total_quantity, 5);
    EXPECT_EQ(first.executions.size(), second.executions.size());
}

TEST(RequestProcessor, Repeated_cancel_request_returns_cached_result) {
    RequestProcessor processor;

    ASSERT_TRUE(processor.process(
        5001, NewOrder{101, Side::Buy, 100, 5}).accepted);

    ClientRequest request = CancelOrder{101};

    auto first = processor.process(5002, request);
    auto second = processor.process(5002, request);

    EXPECT_TRUE(first.accepted);
    EXPECT_TRUE(second.accepted);
    EXPECT_FALSE(processor.order_book().contains_order(101));
    EXPECT_EQ(processor.order_book().bid_level_count(), 0);
}

TEST(RequestProcessor, Repeated_replace_request_is_not_processed_twice) {
    RequestProcessor processor;

    ASSERT_TRUE(processor.process(
        5001, NewOrder{101, Side::Buy, 100, 10}).accepted);

    ClientRequest request = ReplaceOrder{
        101, NewOrder{102, Side::Buy, 99, 15}
    };

    auto first = processor.process(5002, request);
    auto second = processor.process(5002, request);

    EXPECT_TRUE(first.accepted);
    EXPECT_TRUE(second.accepted);
    EXPECT_FALSE(processor.order_book().contains_order(101));
    EXPECT_TRUE(processor.order_book().contains_order(102));

    ASSERT_NE(processor.order_book().bid_level(99), nullptr);
    EXPECT_EQ(processor.order_book().bid_level(99)->total_quantity, 15);
}

TEST(RequestProcessor, Reusing_request_id_with_different_payload_is_rejected) {
    RequestProcessor processor;

    auto first = processor.process(
        5001, NewOrder{101, Side::Buy, 100, 5});

    auto second = processor.process(
        5001, NewOrder{102, Side::Buy, 105, 10});

    EXPECT_TRUE(first.accepted);
    EXPECT_FALSE(second.accepted);
    EXPECT_EQ(
        second.error,
        RequestError::DuplicateRequestIdWithDifferentPayload);

    EXPECT_TRUE(processor.order_book().contains_order(101));
    EXPECT_FALSE(processor.order_book().contains_order(102));
}

TEST(RequestProcessor, Repeated_rejected_request_returns_cached_rejection) {
    RequestProcessor processor;
    ClientRequest request = NewOrder{101, Side::Buy, 0, 5};

    auto first = processor.process(5001, request);
    auto second = processor.process(5001, request);

    EXPECT_FALSE(first.accepted);
    EXPECT_FALSE(second.accepted);
    EXPECT_EQ(first.error, RequestError::InvalidPrice);
    EXPECT_EQ(second.error, RequestError::InvalidPrice);
    EXPECT_FALSE(processor.order_book().contains_order(101));
}

// ------------------------------------------------------------
// Command ordering tests
// ------------------------------------------------------------

TEST(RequestProcessor, Rejects_command_that_arrives_out_of_order) {
    RequestProcessor processor;

    // Sequence 2 arrives before sequence 1.
    auto result = processor.process(
        5002, 2, NewOrder{102, Side::Buy, 100, 5});

    EXPECT_FALSE(result.accepted);
    EXPECT_EQ(result.error, RequestError::OutOfOrderCommand);
    EXPECT_FALSE(processor.order_book().contains_order(102));

    // Sequence 1 can still be processed.
    auto first = processor.process(
        5001, 1, NewOrder{101, Side::Buy, 100, 5});

    EXPECT_TRUE(first.accepted);

    // Sequence 2 can now be retried.
    auto second = processor.process(
        5002, 2, NewOrder{102, Side::Buy, 100, 10});

    EXPECT_TRUE(second.accepted);
    EXPECT_TRUE(processor.order_book().contains_order(102));
}

TEST(RequestProcessor, Rejected_domain_command_still_consumes_sequence) {
    RequestProcessor processor;

    auto cancel = processor.process(
        5001, 1, CancelOrder{999});

    EXPECT_FALSE(cancel.accepted);
    EXPECT_EQ(cancel.error, RequestError::UnknownOrder);

    auto new_order = processor.process(
        5002, 2, NewOrder{101, Side::Buy, 100, 5});

    EXPECT_TRUE(new_order.accepted);
}

// ------------------------------------------------------------
// Replay tests
// ------------------------------------------------------------

TEST(RequestProcessor, Replaying_history_reproduces_final_book_state) {
    RequestProcessor original;

    ASSERT_TRUE(original.process(
        5001, 1, NewOrder{101, Side::Buy, 100, 10}).accepted);

    ASSERT_TRUE(original.process(
        5002, 2, NewOrder{102, Side::Sell, 110, 5}).accepted);

    ASSERT_TRUE(original.process(
        5003, 3, CancelOrder{101}).accepted);

    ASSERT_TRUE(original.process(
        5004, 4, NewOrder{103, Side::Buy, 105, 7}).accepted);

    RequestProcessor replayed;
    const auto replay_results = replayed.replay(original.history());

    ASSERT_EQ(replay_results.size(), original.history().size());

    EXPECT_EQ(
        replayed.order_book().best_bid(),
        original.order_book().best_bid());

    EXPECT_EQ(
        replayed.order_book().best_ask(),
        original.order_book().best_ask());

    EXPECT_EQ(
        replayed.order_book().bid_level_count(),
        original.order_book().bid_level_count());

    EXPECT_EQ(
        replayed.order_book().ask_level_count(),
        original.order_book().ask_level_count());

    ASSERT_NE(replayed.order_book().bid_level(105), nullptr);
    ASSERT_NE(original.order_book().bid_level(105), nullptr);

    EXPECT_EQ(
        replayed.order_book().bid_level(105)->total_quantity,
        original.order_book().bid_level(105)->total_quantity);
}

TEST(RequestProcessor, Replay_reproduces_execution_events) {
    RequestProcessor original;

    ASSERT_TRUE(original.process(
        5001, 1, NewOrder{101, Side::Sell, 100, 10}).accepted);

    auto result = original.process(
        5002, 2, NewOrder{102, Side::Buy, 100, 5});

    ASSERT_TRUE(result.accepted);
    ASSERT_EQ(result.executions.size(), 1);

    RequestProcessor replayed;
    const auto replay_results = replayed.replay(original.history());

    ASSERT_EQ(replay_results.size(), 2);
    ASSERT_EQ(replay_results[1].executions.size(), 1);

    const auto& original_event = result.executions[0];
    const auto& replayed_event = replay_results[1].executions[0];

    EXPECT_EQ(replayed_event.buy_order_id_, original_event.buy_order_id_);
    EXPECT_EQ(replayed_event.sell_order_id_, original_event.sell_order_id_);
    EXPECT_EQ(replayed_event.execution_price_, original_event.execution_price_);
    EXPECT_EQ(
        replayed_event.executed_quantity_,
        original_event.executed_quantity_);
}

TEST(RequestProcessor, Replay_preserves_rejected_commands) {
    RequestProcessor original;

    auto rejected = original.process(
        5001, 1, CancelOrder{999});

    ASSERT_FALSE(rejected.accepted);
    ASSERT_EQ(rejected.error, RequestError::UnknownOrder);

    ASSERT_TRUE(original.process(
        5002, 2, NewOrder{101, Side::Buy, 100, 5}).accepted);

    RequestProcessor replayed;
    const auto results = replayed.replay(original.history());

    ASSERT_EQ(results.size(), 2);
    EXPECT_FALSE(results[0].accepted);
    EXPECT_EQ(results[0].error, RequestError::UnknownOrder);
    EXPECT_TRUE(results[1].accepted);
}

// ------------------------------------------------------------
// Determinism and error taxonomy
// ------------------------------------------------------------

TEST(RequestProcessor, Cancel_and_replace_produce_deterministic_results) {
    RequestProcessor first;
    RequestProcessor second;

    const std::vector<ClientRequest> commands{
        NewOrder{101, Side::Buy, 100, 10},
        ReplaceOrder{101, NewOrder{102, Side::Buy, 99, 15}},
        CancelOrder{102}
    };

    RequestId request_id = 1;

    for (const auto& command : commands) {
        const auto first_result = first.process(request_id, command);
        const auto second_result = second.process(request_id, command);

        EXPECT_EQ(first_result.accepted, second_result.accepted);
        EXPECT_EQ(first_result.error, second_result.error);
        EXPECT_EQ(
            first_result.executions.size(),
            second_result.executions.size());

        ++request_id;
    }

    EXPECT_EQ(first.order_book().best_bid(),
              second.order_book().best_bid());

    EXPECT_EQ(first.order_book().best_ask(),
              second.order_book().best_ask());

    EXPECT_EQ(first.order_book().bid_level_count(),
              second.order_book().bid_level_count());

    EXPECT_EQ(first.order_book().ask_level_count(),
              second.order_book().ask_level_count());
}

TEST(RequestProcessor, Reports_unknown_order_error) {
    RequestProcessor processor;

    const auto result = processor.process(
        5001, CancelOrder{999});

    EXPECT_FALSE(result.accepted);
    EXPECT_EQ(result.error, RequestError::UnknownOrder);
}

TEST(RequestProcessor, Reports_invalid_quantity_error) {
    RequestProcessor processor;

    const auto result = processor.process(
        5001, NewOrder{101, Side::Buy, 100, 0});

    EXPECT_FALSE(result.accepted);
    EXPECT_EQ(result.error, RequestError::InvalidQuantity);
}

TEST(RequestProcessor, Rejects_zero_request_id) {
    RequestProcessor processor;

    const auto result = processor.process(
        0, NewOrder{101, Side::Buy, 100, 5});

    EXPECT_FALSE(result.accepted);
    EXPECT_EQ(result.error, RequestError::InvalidRequestId);
}
