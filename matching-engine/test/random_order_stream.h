#ifndef MATCHING_ENGINE_RANDOM_ORDER_STREAM_H
#define MATCHING_ENGINE_RANDOM_ORDER_STREAM_H

#include "../src/orderbook/request_processor.h"

#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

class RandomOrderStream {
public:
  explicit RandomOrderStream(std::uint64_t seed)
      : rng_(seed) {
  }

  [[nodiscard]]
  ClientRequest next(const OrderBook &book) {
    const auto active_orders =
        collect_active_order_ids(book.snapshot());

    const std::uint64_t operation =
        next_u64() % 100U;

    // When the book is empty we obviously need to generate NEW.
    //
    // Otherwise:
    //
    // 60% NEW
    // 22% CANCEL
    // 18% REPLACE
    if (active_orders.empty() || operation < 60U) {
      return make_new_order();
    }

    const OrderId selected =
        active_orders[
            static_cast<std::size_t>(
                next_u64() %
                active_orders.size())];

    if (operation < 82U) {
      return CancelOrder{selected};
    }

    return ReplaceOrder{
        selected,
        make_new_order()};
  }

private:
  std::mt19937_64 rng_;

  OrderId next_order_id_{1'000};

  [[nodiscard]]
  std::uint64_t next_u64() {
    return rng_();
  }

  [[nodiscard]]
  NewOrder make_new_order() {
    const Side side =
        (next_u64() & 1U) == 0U
            ? Side::Buy
            : Side::Sell;

    // Narrow price range is intentional:
    // it generates plenty of crossing orders and partial fills.
    const Price price =
        95 +
        static_cast<Price>(
            next_u64() % 11U);

    const Quantity quantity =
        1 +
        static_cast<Quantity>(
            next_u64() % 50U);

    return NewOrder{
        next_order_id_++,
        side,
        price,
        quantity};
  }

  [[nodiscard]]
  static std::vector<OrderId>
  collect_active_order_ids(
      const OrderBookSnapshot &snapshot) {

    std::vector<OrderId> result;

    const auto collect =
        [&result](const auto &levels) {

          for (const auto &level : levels) {
            for (const auto &order : level.orders) {
              result.push_back(order.id);
            }
          }
        };

    collect(snapshot.bids);
    collect(snapshot.asks);

    return result;
  }
};

#endif