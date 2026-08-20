include(FetchContent)

FetchContent_Declare(
        google_benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.4
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(google_benchmark)

add_executable(byte_buffer_benchmark
        byte_buffer_benchmark.cpp
)

target_link_libraries(byte_buffer_benchmark
        PRIVATE
        core
        benchmark::benchmark
        benchmark::benchmark_main
)