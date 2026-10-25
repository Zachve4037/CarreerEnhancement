#ifndef CPPFIRST_ORDER_BOOK_H
#define CPPFIRST_ORDER_BOOK_H

#include "order.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <utility>
#include <vector>

struct PriceLevel {
    Quantity total_quantity{0};
    std::size_t order_count{0};
    std::list<Order> orders;
};

struct ExecutionEvent {
    OrderId buy_order_id_;
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

    [[nodiscard]] std::size_t bid_level_count() const noexcept {
        return bids_.size();
    }

    [[nodiscard]] std::size_t ask_level_count() const noexcept {
        return asks_.size();
    }

    [[nodiscard]] static bool prices_cross(
        Side side, Price incoming_price, Price resting_price) noexcept {
        return (side == Side::Buy && incoming_price >= resting_price) ||
               (side == Side::Sell && incoming_price <= resting_price);
    }

    [[nodiscard]] std::optional<Price> best_opposite_price(Side side) const noexcept {
        if (side == Side::Buy && !asks_.empty()) {
            return asks_.begin()->first;
        }

        if (side == Side::Sell && !bids_.empty()) {
            return bids_.begin()->first;
        }

        return std::nullopt;
    }

    [[nodiscard]] std::optional<ExecutionEvent> execute_event(
        Order& incoming, Order& resting) {
        if (incoming.side == resting.side ||
            !prices_cross(incoming.side, incoming.price, resting.price)) {
            return std::nullopt;
        }

        const Quantity incoming_remaining = incoming.remaining_quantity();
        const Quantity resting_remaining = resting.remaining_quantity();

        if (incoming_remaining == 0 || resting_remaining == 0) {
            return std::nullopt;
        }

        const Quantity executed_quantity =
            std::min(incoming_remaining, resting_remaining);

        if (!incoming.apply_fill(executed_quantity) ||
            !resting.apply_fill(executed_quantity)) {
            return std::nullopt;
        }

        const OrderId buy_id =
            incoming.side == Side::Buy ? incoming.id : resting.id;
        const OrderId sell_id =
            incoming.side == Side::Sell ? incoming.id : resting.id;

        return ExecutionEvent{
            buy_id, sell_id, resting.price, executed_quantity
        };
    }

    [[nodiscard]] std::vector<ExecutionEvent> match_order(Order& incoming) {
        std::vector<ExecutionEvent> executions;

        if (incoming.side == Side::Buy) {
            match_against(incoming, asks_, executions);
        } else {
            match_against(incoming, bids_, executions);
        }

        return executions;
    }

    [[nodiscard]] std::vector<ExecutionEvent> process_order(Order incoming) {
        auto executions = match_order(incoming);

        if (incoming.remaining_quantity() == 0) {
            return executions;
        }

        if (incoming.filled_quantity == 0) {
            incoming.status = OrderStatus::Open;
        }

        if (incoming.side == Side::Buy) {
            auto& level = bids_[incoming.price];
            level.total_quantity += incoming.remaining_quantity();
            ++level.order_count;
            level.orders.push_back(std::move(incoming));
        } else {
            auto& level = asks_[incoming.price];
            level.total_quantity += incoming.remaining_quantity();
            ++level.order_count;
            level.orders.push_back(std::move(incoming));
        }

        return executions;
    }

private:
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel, std::less<Price>> asks_;

    template <typename Book>
    void match_against(
        Order& incoming,
        Book& opposite_book,
        std::vector<ExecutionEvent>& executions) {

        while (incoming.remaining_quantity() > 0) {
            const auto best_price = best_opposite_price(incoming.side);

            if (!best_price ||
                !prices_cross(incoming.side, incoming.price, *best_price)) {
                break;
            }

            auto level_it = opposite_book.find(*best_price);

            if (level_it == opposite_book.end()) {
                break;
            }

            auto& level = level_it->second;
            auto& resting_orders = level.orders;

            if (resting_orders.empty()) {
                opposite_book.erase(level_it);
                continue;
            }

            Order& resting = resting_orders.front();
            const auto execution = execute_event(incoming, resting);

            if (!execution) {
                break;
            }

            executions.push_back(*execution);

            level.total_quantity -= execution->executed_quantity_;

            if (resting.status == OrderStatus::Filled) {
                resting_orders.pop_front();
                --level.order_count;
            }

            if (resting_orders.empty()) {
                opposite_book.erase(level_it);
            }
        }
    }
};

#endif