#include "../src/orderbook/request_processor.h"
#include "random_order_stream.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <variant>

namespace {
    using QuantityByOrder =
    std::unordered_map<OrderId, Quantity>;

    [[nodiscard]]
    std::unordered_map<OrderId, OrderSnapshot>
    active_orders_by_id(
        const OrderBookSnapshot &snapshot) {
        std::unordered_map<OrderId, OrderSnapshot> result;

        const auto collect =
                [&result](const auto &levels) {
            for (const auto &level: levels) {
                for (const auto &order: level.orders) {
                    result.emplace(order.id, order);
                }
            }
        };

        collect(snapshot.bids);
        collect(snapshot.asks);

        return result;
    }

    void expect_side_is_well_formed(
        const std::vector<PriceLevelSnapshot> &levels,
        Side expected_side,
        bool descending_prices,
        const OrderBook &book,
        std::unordered_set<OrderId> &all_active_ids) {
        for (std::size_t level_index = 0;
             level_index < levels.size();
             ++level_index) {
            const auto &level = levels[level_index];

            EXPECT_EQ(level.side, expected_side);
            ASSERT_FALSE(level.orders.empty());

            // Price-priority invariant.
            if (level_index > 0) {
                if (descending_prices) {
                    EXPECT_GT(
                        levels[level_index - 1].price,
                        level.price);
                } else {
                    EXPECT_LT(
                        levels[level_index - 1].price,
                        level.price);
                }
            }

            Quantity recomputed_total = 0;
            Sequence previous_sequence = 0;

            for (const auto &order: level.orders) {
                EXPECT_EQ(order.side, expected_side);
                EXPECT_EQ(order.price, level.price);

                EXPECT_GT(order.quantity, 0U);
                EXPECT_LT(
                    order.filled_quantity,
                    order.quantity);

                EXPECT_GT(
                    order.remaining_quantity(),
                    0U);

                EXPECT_TRUE(
                    book.contains_order(order.id));

                EXPECT_TRUE(
                            all_active_ids.insert(order.id).second)
                        << "duplicate active order id "
                        << order.id;

                // FIFO/time-priority invariant.
                if (previous_sequence != 0) {
                    EXPECT_LT(
                                previous_sequence,
                                order.sequence)
                            << "FIFO order was lost at price "
                            << level.price;
                }

                previous_sequence = order.sequence;

                if (order.filled_quantity == 0) {
                    EXPECT_EQ(
                        order.status,
                        OrderStatus::Open);
                } else {
                    EXPECT_EQ(
                        order.status,
                        OrderStatus::PartiallyFilled);
                }

                recomputed_total +=
                        order.remaining_quantity();
            }

            // Price-level conservation invariant.
            EXPECT_EQ(
                        recomputed_total,
                        level.total_quantity)
                    << "price-level quantity is not conserved "
                    << "at price "
                    << level.price;
        }
    }

    void expect_book_ordering_and_level_conservation(
        const OrderBook &book) {
        const auto snapshot =
                book.snapshot();

        std::unordered_set<OrderId> active_ids;

        expect_side_is_well_formed(
            snapshot.bids,
            Side::Buy,
            true,
            book,
            active_ids);

        expect_side_is_well_formed(
            snapshot.asks,
            Side::Sell,
            false,
            book,
            active_ids);

        if (!snapshot.bids.empty()) {
            EXPECT_EQ(
                book.best_bid(),
                snapshot.bids.front().price);
        } else {
            EXPECT_FALSE(
                book.best_bid().has_value());
        }

        if (!snapshot.asks.empty()) {
            EXPECT_EQ(
                book.best_ask(),
                snapshot.asks.front().price);
        } else {
            EXPECT_FALSE(
                book.best_ask().has_value());
        }

        // After matching is complete, the resting book must not
        // contain an immediately matchable bid/ask pair.
        if (!snapshot.bids.empty() &&
            !snapshot.asks.empty()) {
            EXPECT_LT(
                snapshot.bids.front().price,
                snapshot.asks.front().price);
        }
    }

    void expect_order_lifecycle_conservation(
        const QuantityByOrder &original_quantity,
        const QuantityByOrder &executed_quantity,
        const QuantityByOrder &cancelled_quantity,
        const OrderBookSnapshot &snapshot) {
        QuantityByOrder active_remaining;

        const auto collect_remaining =
                [&active_remaining](const auto &levels) {
            for (const auto &level: levels) {
                for (const auto &order: level.orders) {
                    active_remaining[order.id] +=
                            order.remaining_quantity();
                }
            }
        };

        collect_remaining(snapshot.bids);
        collect_remaining(snapshot.asks);

        for (const auto &[id, original]
             : original_quantity) {
            const Quantity executed =
                    executed_quantity.contains(id)
                        ? executed_quantity.at(id)
                        : 0;

            const Quantity cancelled =
                    cancelled_quantity.contains(id)
                        ? cancelled_quantity.at(id)
                        : 0;

            const Quantity active =
                    active_remaining.contains(id)
                        ? active_remaining.at(id)
                        : 0;

            EXPECT_EQ(
                        original,
                        executed + cancelled + active)
                    << "quantity conservation failed for order "
                    << id;
        }
    }

    [[nodiscard]]
    Quantity remaining_before_operation(
        const OrderBookSnapshot &snapshot,
        OrderId id) {
        const auto active =
                active_orders_by_id(snapshot);

        const auto it =
                active.find(id);

        EXPECT_NE(it, active.end())
                << "generated command references inactive order";

        return it == active.end()
                   ? 0
                   : it->second.remaining_quantity();
    }
} // namespace

TEST(
    RandomizedOrderStream,
    PreservesQuantityAndPriceTimeOrdering) {
    constexpr std::uint64_t seed =
            0x5eed'2026ULL;

    constexpr std::size_t operation_count =
            5'000;

    RequestProcessor processor;
    RandomOrderStream stream(seed);

    QuantityByOrder original_quantity;
    QuantityByOrder executed_quantity;
    QuantityByOrder cancelled_quantity;

    for (std::size_t step = 0;
         step < operation_count;
         ++step) {
        const auto before =
                processor.order_book().snapshot();

        const ClientRequest request =
                stream.next(
                    processor.order_book());

        std::visit(
            [&](const auto &command) {
                using T =
                        std::decay_t<decltype(command)>;

                if constexpr (
                    std::is_same_v<T, NewOrder>) {
                    ASSERT_FALSE(
                        original_quantity.contains(
                            command.id));

                    original_quantity.emplace(
                        command.id,
                        command.quantity);
                } else if constexpr (
                    std::is_same_v<T, CancelOrder>) {
                    cancelled_quantity[command.id] +=
                            remaining_before_operation(
                                before,
                                command.id);
                } else if constexpr (
                    std::is_same_v<T, ReplaceOrder>) {
                    // Replacing an order is equivalent to cancelling
                    // the remaining quantity of the old order and
                    // creating a completely new order.
                    cancelled_quantity[command.old_id] +=
                            remaining_before_operation(
                                before,
                                command.old_id);

                    ASSERT_FALSE(
                        original_quantity.contains(
                            command.replacement.id));

                    original_quantity.emplace(
                        command.replacement.id,
                        command.replacement.quantity);
                }
            },
            request);

        const Sequence sequence =
                static_cast<Sequence>(step + 1);

        const RequestId request_id =
                10'000 + sequence;

        const RequestResult result =
                processor.process(
                    request_id,
                    sequence,
                    request);

        ASSERT_TRUE(result.accepted)
                << "seed=" << seed
                << " step=" << step
                << " error="
                << static_cast<int>(result.error);

        for (const auto &execution
             : result.executions) {
            ASSERT_GT(
                execution.executed_quantity_,
                0U);

            executed_quantity[
                        execution.buy_order_id_] +=
                    execution.executed_quantity_;

            executed_quantity[
                        execution.sell_order_id_] +=
                    execution.executed_quantity_;
        }

        expect_book_ordering_and_level_conservation(
            processor.order_book());

        expect_order_lifecycle_conservation(
            original_quantity,
            executed_quantity,
            cancelled_quantity,
            processor.order_book().snapshot());
    }
}
