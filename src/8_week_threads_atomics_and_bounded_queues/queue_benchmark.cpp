
#include "bounded_blocking_queue.h"
#include "spsc_queue.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <latch>
#include <numeric>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

using Clock = std::chrono::steady_clock;

constexpr std::size_t NUM_ITEMS = 1'000'000;
constexpr std::size_t CAPACITY = 1024;
constexpr int WARMUP_RUNS = 1;
constexpr int MEASURED_RUNS = 5;

struct Result {
    double seconds;
    double throughput;
};

// Distribute total items evenly among threads.
std::size_t items_for_thread(std::size_t total,
                             int thread_id,
                             int num_threads) {
    return total / num_threads +
           (static_cast<std::size_t>(thread_id) < total % num_threads);
}

double median(std::vector<double> values) {
    std::sort(values.begin(), values.end());
    return values[values.size() / 2];
}

// --------------------------------------------------
// SPSC benchmark: exactly one producer and consumer.
// --------------------------------------------------
Result benchmark_spsc() {
    SPSCQueue<int> queue(CAPACITY);

    std::latch ready(2);
    std::atomic<bool> start{false};

    std::thread producer([&]() {
        ready.count_down();

        while (!start.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        for (std::size_t i = 0; i < NUM_ITEMS; ++i) {
            while (!queue.try_push(static_cast<int>(i))) {
                std::this_thread::yield();
            }
        }
    });

    std::thread consumer([&]() {
        ready.count_down();

        while (!start.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        int value;
        for (std::size_t i = 0; i < NUM_ITEMS; ++i) {
            while (!queue.try_pop(value)) {
                std::this_thread::yield();
            }
        }
    });

    ready.wait();

    const auto begin = Clock::now();
    start.store(true, std::memory_order_release);

    producer.join();
    consumer.join();

    const auto end = Clock::now();

    const double seconds =
        std::chrono::duration<double>(end - begin).count();

    return {seconds, NUM_ITEMS / seconds};
}

// --------------------------------------------------
// Blocking queue benchmark: configurable P/C counts.
// --------------------------------------------------
Result benchmark_blocking(int num_producers,
                          int num_consumers) {
    BoundedBlockingQueue<int> queue(CAPACITY);

    std::latch ready(num_producers + num_consumers);
    std::atomic<bool> start{false};

    std::vector<std::thread> threads;
    threads.reserve(num_producers + num_consumers);

    for (int p = 0; p < num_producers; ++p) {
        threads.emplace_back([&, p]() {
            ready.count_down();

            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            const auto count =
                items_for_thread(NUM_ITEMS, p, num_producers);

            for (std::size_t i = 0; i < count; ++i) {
                queue.push(static_cast<int>(i));
            }
        });
    }

    for (int c = 0; c < num_consumers; ++c) {
        threads.emplace_back([&, c]() {
            ready.count_down();

            while (!start.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            const auto count =
                items_for_thread(NUM_ITEMS, c, num_consumers);

            for (std::size_t i = 0; i < count; ++i) {
                (void)queue.pop();
            }
        });
    }

    ready.wait();

    const auto begin = Clock::now();
    start.store(true, std::memory_order_release);

    for (auto& thread : threads) {
        thread.join();
    }

    const auto end = Clock::now();

    const double seconds =
        std::chrono::duration<double>(end - begin).count();

    return {seconds, NUM_ITEMS / seconds};
}

// --------------------------------------------------
// Run warm-ups and measured iterations.
// --------------------------------------------------
template <typename Benchmark>
void run_benchmark(const std::string& name,
                   Benchmark benchmark) {
    for (int i = 0; i < WARMUP_RUNS; ++i) {
        benchmark();
    }

    std::vector<double> times;
    std::vector<double> throughputs;

    for (int i = 0; i < MEASURED_RUNS; ++i) {
        const Result result = benchmark();

        times.push_back(result.seconds);
        throughputs.push_back(result.throughput);
    }

    std::cout << name << ','
              << std::fixed << std::setprecision(6)
              << median(times) << ','
              << std::setprecision(0)
              << median(throughputs) << '\n';
}

int main() {
    std::cout << "Queue,Median seconds,Median items/sec\n";

    run_benchmark("SPSCQueue 1P/1C", [] {
        return benchmark_spsc();
    });

    run_benchmark("BlockingQueue 1P/1C", [] {
        return benchmark_blocking(1, 1);
    });

    run_benchmark("BlockingQueue 2P/1C", [] {
        return benchmark_blocking(2, 1);
    });

    run_benchmark("BlockingQueue 1P/2C", [] {
        return benchmark_blocking(1, 2);
    });

    run_benchmark("BlockingQueue 4P/4C", [] {
        return benchmark_blocking(4, 4);
    });

    return 0;
}