# Order-book representation benchmark

## Goal

Compare two price-level representations with the same basic workload:

- `OrderBook`: `std::map` for bids and asks.
- `FlatPriceBook`: sorted `std::vector` for bids and asks.

The benchmark builds both sides from deterministic shuffled, unique prices and then queries the best bid and ask. Sizes are 64, 512, and 4096 input prices. Construction and queries are included in the timed region; price generation and shuffling happen before timing.

## Run

From the `CppFirst` repository root:

```bash
cmake -S matching-engine -B matching-engine/build-release -DCMAKE_BUILD_TYPE=Release
cmake --build matching-engine/build-release --target order_book_benchmark
./matching-engine/build-release/benchmark/order_book_benchmark
```

When building the full parent project, the root `benchmark` directory already fetches Google Benchmark, which is reused by this target. A standalone configure fetches Google Benchmark from its pinned `v1.9.4` tag.

## Interpretation

This is a synthetic microbenchmark, not a claim about production performance. The flat vector has O(N) insertion for a new price level; the map has O(log N) insertion. The flat vector may benefit from locality at small sizes, while the map should scale more favorably for shuffled insertions at larger sizes. Measure on the target machine and retain the emitted raw output with CPU, compiler, build type, and OS details. Do not compare Debug results or results from different machines as if they were controlled experiments.

The benchmark does not yet model same-price aggregation-heavy traffic, cancels, matching, allocations for individual orders, latency percentiles, or multi-threaded ingestion.
