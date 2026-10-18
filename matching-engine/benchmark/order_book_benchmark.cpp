#include "../src/orderbook/order_book.h"
#include <benchmark/benchmark.h>
#include <algorithm>
#include <cstddef>
#include <map>
#include <optional>
#include <random>
#include <utility>
#include <vector>

// Comparison representation: sorted contiguous vector of price levels.
class FlatPriceBook {
public:
    void add_order(Side side, Price price, Quantity quantity) {
        auto& levels = side == Side::Buy ? bids_ : asks_;
        const auto it = std::lower_bound(levels.begin(), levels.end(), price,
            [](const auto& level, Price value) { return level.first < value; });
        if (it != levels.end() && it->first == price) {
            it->second.total_quantity += quantity;
            ++it->second.order_count;
        } else {
            levels.insert(it, {price, PriceLevel{quantity, 1}});
        }
    }
    std::optional<Price> best_bid() const {
        return bids_.empty() ? std::nullopt : std::optional<Price>{bids_.back().first};
    }
    std::optional<Price> best_ask() const {
        return asks_.empty() ? std::nullopt : std::optional<Price>{asks_.front().first};
    }
private:
    // Ascending order: highest bid at back; lowest ask at front.
    std::vector<std::pair<Price, PriceLevel>> bids_;
    std::vector<std::pair<Price, PriceLevel>> asks_;
};

static std::vector<Price> make_prices(std::size_t count) {
    std::vector<Price> prices;
    prices.reserve(count);
    for (std::size_t i = 0; i < count; ++i) prices.push_back(static_cast<Price>(10000 + i * 2));
    std::mt19937 rng(0xC0FFEE);
    std::shuffle(prices.begin(), prices.end(), rng);
    return prices;
}

template <typename Book>
static void BM_BookBuildAndBestPrice(benchmark::State& state) {
    const auto prices = make_prices(static_cast<std::size_t>(state.range(0)));
    for (auto _ : state) {
        Book book;
        for (const Price price : prices) {
            book.add_order(Side::Buy, price, 1);
            book.add_order(Side::Sell, price + 1, 1);
        }
        benchmark::DoNotOptimize(book.best_bid());
        benchmark::DoNotOptimize(book.best_ask());
    }
    state.SetItemsProcessed(state.iterations() * state.range(0) * 2);
}

BENCHMARK_TEMPLATE(BM_BookBuildAndBestPrice, OrderBook)->Arg(64)->Arg(512)->Arg(4096);
BENCHMARK_TEMPLATE(BM_BookBuildAndBestPrice, FlatPriceBook)->Arg(64)->Arg(512)->Arg(4096);
BENCHMARK_MAIN();
