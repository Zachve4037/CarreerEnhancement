//
// Created by zachvem on 20-Aug-26.
//

#include <benchmark/benchmark.h>
#include "../src/byte_buffer.h"

static void BM_ByteBufferCopy(benchmark::State& state) {
  byte_buffer source(state.range(0));

  for (auto _ : state) {
    byte_buffer copy = source;

    benchmark::DoNotOptimize(copy.data());
  }
}

BENCHMARK(BM_ByteBufferCopy)
    ->Arg(1024)
    ->Arg(1024 * 1024)
    ->Arg(10 * 1024 * 1024);

static void BM_ByteBufferMove(benchmark::State& state)
{
  byte_buffer source(state.range(0));

  for (auto _ : state) {
    byte_buffer moved = std::move(source);

    benchmark::DoNotOptimize(moved.data());

    source = std::move(moved);
  }
}

BENCHMARK(BM_ByteBufferMove)
    ->Arg(1024)
    ->Arg(1024 * 1024)
    ->Arg(10 * 1024 * 1024);