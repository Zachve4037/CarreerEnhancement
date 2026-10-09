//
// Created by zachvem on 09-Oct-26.
//

#include <gtest/gtest.h>

#include "../matching-engine/src/orderbook/order.h"

TEST(OrderTest, CalculatesRemainingQuantity) {
  const Order order{
    .id = 42,
    .side = Side::Buy,
    .price = 10500,
    .quantity = 100,
    .filled_quantity = 30,
    .sequence = 17,
    .status = OrderStatus::PartiallyFilled
};

  EXPECT_EQ(order.remaining_quantity(), 70);
}

TEST(OrderTest, NewOrderStartsWithNoFilledQuantity) {
  const Order order{
    .id = 42,
    .side = Side::Buy,
    .price = 10500,
    .quantity = 100,
    .filled_quantity = 0,
    .sequence = 17,
    .status = OrderStatus::Open
};

  EXPECT_EQ(order.remaining_quantity(), 100);
}