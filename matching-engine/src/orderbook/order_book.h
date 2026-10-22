#ifndef CPPFIRST_ORDER_BOOK_H
#define CPPFIRST_ORDER_BOOK_H

#include "../../../cmake-build-debug/_deps/google_benchmark-src/src/arraysize.h"
#include "order.h"

#include <cstddef>
#include <functional>
#include <map>
#include <optional>

struct PriceLevel {
  Quantity total_quantity{0};
  std::size_t order_count{0};
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

  bool prices_cross(Side side_, Price price_, Price best_from_opposite_) {
    if ((side_ == Side::Buy && price_ >= best_from_opposite_) ||
        (side_ == Side::Sell && price_ <= best_from_opposite_)) {
      return true;
    }
    return false;
  }

private:
  std::map<Price, PriceLevel, std::greater<Price>> bids_;
  std::map<Price, PriceLevel, std::less<Price>> asks_;
};

#endif // CPPFIRST_ORDER_BOOK_H
