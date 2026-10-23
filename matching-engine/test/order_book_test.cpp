#include "../src/orderbook/order_book.h"
#include <gtest/gtest.h>

TEST(OrderBook, EmptyBookHasNoBestPrices) {
  const OrderBook book;
  EXPECT_FALSE(book.best_bid().has_value());
  EXPECT_FALSE(book.best_ask().has_value());
}

TEST(OrderBook, BestBidIsHighestBidPrice) {
  OrderBook book;
  ASSERT_TRUE(book.add_order(Side::Buy, 100, 2));
  ASSERT_TRUE(book.add_order(Side::Buy, 105, 3));
  ASSERT_TRUE(book.add_order(Side::Buy, 102, 4));
  EXPECT_EQ(book.best_bid(), 105);
}

TEST(OrderBook, BestAskIsLowestAskPrice) {
  OrderBook book;
  ASSERT_TRUE(book.add_order(Side::Sell, 110, 2));
  ASSERT_TRUE(book.add_order(Side::Sell, 103, 3));
  ASSERT_TRUE(book.add_order(Side::Sell, 107, 4));
  EXPECT_EQ(book.best_ask(), 103);
}

TEST(OrderBook, BidAndAskSidesAreIndependent) {
  OrderBook book;
  ASSERT_TRUE(book.add_order(Side::Buy, 105, 2));
  ASSERT_TRUE(book.add_order(Side::Sell, 106, 3));
  EXPECT_EQ(book.best_bid(), 105);
  EXPECT_EQ(book.best_ask(), 106);
}

TEST(OrderBook, AggregatesOrdersAtSamePrice) {
  OrderBook book;
  ASSERT_TRUE(book.add_order(Side::Sell, 105, 2));
  ASSERT_TRUE(book.add_order(Side::Sell, 105, 7));
  const PriceLevel *level = book.ask_level(105);
  ASSERT_NE(level, nullptr);
  EXPECT_EQ(level->total_quantity, 9);
  EXPECT_EQ(level->order_count, 2);
  EXPECT_EQ(book.ask_level_count(), 1);
}

TEST(OrderBook, RejectsInvalidPriceAndQuantity) {
  OrderBook book;
  EXPECT_FALSE(book.add_order(Side::Buy, 0, 1));
  EXPECT_FALSE(book.add_order(Side::Sell, -1, 1));
  EXPECT_FALSE(book.add_order(Side::Buy, 100, 0));
  EXPECT_FALSE(book.add_order(static_cast<Side>(99), 100, 1));
  EXPECT_EQ(book.bid_level_count(), 0);
  EXPECT_EQ(book.ask_level_count(), 0);
}

TEST(OrderBook, MissingPriceLevelReturnsNullptr) {
  OrderBook book;
  EXPECT_EQ(book.bid_level(100), nullptr);
  EXPECT_EQ(book.ask_level(100), nullptr);
}

TEST(OrderBook, PriceCrossing1) {
  OrderBook book;
  EXPECT_TRUE(book.prices_cross(Side::Buy, 10600, 10500));
}

TEST(OrderBook, PriceCrossing2) {
  OrderBook book;
  EXPECT_FALSE(book.prices_cross(Side::Buy, 10400, 10500));
}

TEST(OrderBook, PriceCrossing3) {
  OrderBook book;
  EXPECT_TRUE(book.prices_cross(Side::Sell, 10400, 10500));
}

TEST(OrderBook, PriceCrossing4) {
  OrderBook book;
  EXPECT_FALSE(book.prices_cross(Side::Sell, 10600, 10500));
}

TEST(OrderBook, Aplly_fill_partially_filled) {
  OrderBook book;
  auto *order = new Order(1, Side::Buy, 1, 10, 1) ;
  static_cast<void>(order->apply_fill(4));
  EXPECT_EQ(order->status, OrderStatus::PartiallyFilled);
}

TEST(OrderBook, Aplly_fill_filled) {
  OrderBook book;
  auto *order = new Order(1, Side::Buy, 1, 10, 1) ;
  static_cast<void>(order->apply_fill(10));
  EXPECT_EQ(order->status, OrderStatus::Filled);
}

TEST(OrderBook, Aplly_fill_zero_quantity) {
  OrderBook book;
  auto *order = new Order(1, Side::Buy, 1, 10, 1) ;
  EXPECT_FALSE(order->apply_fill(0));
}

TEST(OrderBook, Aplly_fill_overflow) {
  OrderBook book;
  auto *order = new Order(1, Side::Buy, 1, 10, 1) ;
  EXPECT_FALSE(order->apply_fill(11));
}
