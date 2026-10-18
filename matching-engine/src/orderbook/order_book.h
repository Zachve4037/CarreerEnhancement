#ifndef CPPFIRST_ORDER_BOOK_H
#define CPPFIRST_ORDER_BOOK_H

#include "order.h"

#include <cstddef>
#include <functional>
#include <map>
#include <optional>

// Order-book v0.1 stores aggregate quantity per price level. It does not yet
// store individual orders or perform matching/cancellation by OrderId.
struct PriceLevel {
    Quantity total_quantity{0};
    std::size_t order_count{0};
};

class OrderBook {
public:
    // Add one order to the appropriate side, aggregating by price.
    // Returns false for non-positive prices or zero quantities.
    [[nodiscard]] bool add_order(Side side, Price price, Quantity quantity) {
        if (price <= 0 || quantity == 0 ||
            (side != Side::Buy && side != Side::Sell)) {
            return false;
        }

        if (side == Side::Buy) {
            auto& level = bids_[price];
            level.total_quantity += quantity;
            ++level.order_count;
        } else {
            auto& level = asks_[price];
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

    [[nodiscard]] const PriceLevel* bid_level(Price price) const noexcept {
        const auto it = bids_.find(price);
        return it == bids_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] const PriceLevel* ask_level(Price price) const noexcept {
        const auto it = asks_.find(price);
        return it == asks_.end() ? nullptr : &it->second;
    }

    [[nodiscard]] std::size_t bid_level_count() const noexcept { return bids_.size(); }
    [[nodiscard]] std::size_t ask_level_count() const noexcept { return asks_.size(); }

private:
    // begin() is the best price on each side: highest bid and lowest ask.
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel, std::less<Price>> asks_;
};

#endif // CPPFIRST_ORDER_BOOK_H
