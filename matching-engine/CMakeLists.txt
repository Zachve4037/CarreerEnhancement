cmake_minimum_required(VERSION 3.20)

project(MatchingEngine LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(matching_engine_protocol src/orderbook/binary_protocol.cpp)
target_include_directories(matching_engine_protocol PUBLIC src/orderbook)

add_executable(matching_engine_demo
        src/main.cpp
        src/orderbook/command_parser.cpp
        src/orderbook/replay_log.h
)

add_subdirectory(test)
