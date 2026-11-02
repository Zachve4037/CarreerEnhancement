#include "../src/orderbook/request_processor.h"
#include <gtest/gtest.h>

TEST(RequestProcessor, Repeated_new_request_is_not_processed_twice) {
  RequestProcessor processor;

  ClientRequest request = NewOrder{101, Side::Buy, 100, 5};

  auto first = processor.process(5001, request);
  auto second = processor.process(5001, request);

  EXPECT_TRUE(first.accepted);
  EXPECT_TRUE(second.accepted);

  EXPECT_TRUE(processor.order_book().contains_order(101));
  EXPECT_EQ(processor.order_book().bid_level_count(), 1);
  EXPECT_EQ(processor.order_book().bid_level(100)->total_quantity, 5);

  EXPECT_EQ(first.executions.size(), second.executions.size());
}

TEST(RequestProcessor, Repeated_cancel_request_returns_cached_result) {
  RequestProcessor processor;

  processor.process(
      5001, NewOrder{101, Side::Buy, 100, 5});

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

  processor.process(
      5001, NewOrder{101, Side::Buy, 100, 10});

  ClientRequest request = ReplaceOrder{
    101,
    NewOrder{102, Side::Buy, 99, 15}
  };

  auto first = processor.process(5002, request);
  auto second = processor.process(5002, request);

  EXPECT_TRUE(first.accepted);
  EXPECT_TRUE(second.accepted);

  EXPECT_FALSE(processor.order_book().contains_order(101));
  EXPECT_TRUE(processor.order_book().contains_order(102));

  EXPECT_EQ(processor.order_book().bid_level_count(), 1);
  EXPECT_EQ(processor.order_book().bid_level(99)->total_quantity, 15);
}

TEST(RequestProcessor, Reusing_request_id_with_different_payload_is_rejected) {
  RequestProcessor processor;

  ClientRequest first_request =
      NewOrder{101, Side::Buy, 100, 5};

  ClientRequest second_request =
      NewOrder{102, Side::Buy, 105, 10};

  auto first = processor.process(5001, first_request);
  auto second = processor.process(5001, second_request);

  EXPECT_TRUE(first.accepted);
  EXPECT_FALSE(second.accepted);

  EXPECT_TRUE(processor.order_book().contains_order(101));
  EXPECT_FALSE(processor.order_book().contains_order(102));

  EXPECT_EQ(processor.order_book().bid_level_count(), 1);
}

TEST(RequestProcessor, Repeated_rejected_request_returns_cached_rejection) {
  RequestProcessor processor;

  ClientRequest request =
      NewOrder{101, Side::Buy, 0, 5};

  auto first = processor.process(5001, request);
  auto second = processor.process(5001, request);

  EXPECT_FALSE(first.accepted);
  EXPECT_FALSE(second.accepted);

  EXPECT_FALSE(processor.order_book().contains_order(101));
  EXPECT_EQ(processor.order_book().bid_level_count(), 0);
}