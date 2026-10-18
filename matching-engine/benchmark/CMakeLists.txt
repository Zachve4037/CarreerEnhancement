if(NOT TARGET benchmark::benchmark)
    include(FetchContent)
    FetchContent_Declare(
        google_benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.9.4
    )
    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(google_benchmark)
endif()

add_executable(order_book_benchmark order_book_benchmark.cpp)
target_link_libraries(order_book_benchmark PRIVATE benchmark::benchmark)
