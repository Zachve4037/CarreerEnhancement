#ifndef CPPFIRST_ORDER_BOOK_H
#define CPPFIRST_ORDER_BOOK_H

#include "order.h"
#include <cstddef>
#include <functional>
#include <map>
#include <optional>
#include <algorithm>

struct PriceLevel {
  Quantity total_quantity{0};
  std::size_t order_count{0};
};

struct ExecutionEvent {
  OrderId buy_order_id_l;
  OrderId sell_order_id_;
  Price execution_price_;
  Quantity executed_quantity_;
};

class OrderBook {
public:
  [[nodiscard]] bool add_order(Side side, Price price, Quantity quantity) {
    if (price <= 0 || quantity == 0 ||
        (side != Side::Buy && side != Side::Sell)) {
      return false;
    }

    if (side == Side::Buy) {
      auto &level = bids_[price];
      level.total_quantity += quantity;
      ++level.order_count;
    } else {
      auto &level = asks_[price];
      level.total_quantity += quantity;
      ++level.order_count;
    }
    return true;
  }

  [[nodiscard]] std::optional<Price> best_bid() const noexcept {
    if (bids_.empty()) {
      return std::nullopt;
    }
    return bids_.begin()->first;
  }

  [[nodiscard]] std::optional<Price> best_ask() const noexcept {
    if (asks_.empty()) {
      return std::nullopt;
    }
    return asks_.begin()->first;
  }

  [[nodiscard]] const PriceLevel *bid_level(Price price) const noexcept {
    const auto it = bids_.find(price);
    return it == bids_.end() ? nullptr : &it->second;
  }

  [[nodiscard]] const PriceLevel *ask_level(Price price) const noexcept {
    const auto it = asks_.find(price);
    return it == asks_.end() ? nullptr : &it->second;
  }

  [[nodiscard]] std::size_t bid_level_count() const noexcept {
    return bids_.size();
  }
  [[nodiscard]] std::size_t ask_level_count() const noexcept {
    return asks_.size();
  }

  [[nodiscard]] static bool prices_cross(Side side_, Price price_, Price suggestion_) {
    return (side_ == Side::Buy && price_ >= suggestion_) ||
           (side_ == Side::Sell && price_ <= suggestion_);
  }

  [[nodiscard]] std::optional<Price> best_opposite_price(Side side_) {
    if (side_ == Side::Buy && !asks_.empty()) {
      return asks_.begin()->first;
    }
    if (side_ == Side::Sell && !bids_.empty()) {
      return bids_.begin()->first;
    }
    return std::nullopt;
  }

  [[nodiscard]] std::optional<ExecutionEvent> execute_event(Order& incoming, Order& resting) {
    if (incoming.side == resting.side) {
      return std::nullopt;
    }
    if (!prices_cross(incoming.side, incoming.price, resting.price)) {
      return std::nullopt;
    }
    const Quantity executed_quantity =
        std::min(resting.remaining_quantity(), incoming.remaining_quantity());
    if (executed_quantity != 0) {
      bool resting_applied = resting.apply_fill(executed_quantity);
      bool incoming_applied = incoming.apply_fill(executed_quantity);
      if (resting_applied && incoming_applied) {
        std::uint64_t buy_order_id_ = incoming.side == Side::Buy ? incoming.id : resting.id;
        std::uint64_t sell_order_id = incoming.side == Side::Sell ? resting.id : incoming.id;
        return ExecutionEvent(buy_order_id_, sell_order_id, resting.price, executed_quantity);
      }
    }
    return std::nullopt;
  }

private:
  std::map<Price, PriceLevel, std::greater<Price>> bids_;
  std::map<Price, PriceLevel, std::less<Price>> asks_;
};

#endif // CPPFIRST_ORDER_BOOK_H
