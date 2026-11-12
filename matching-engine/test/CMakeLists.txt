include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

add_executable(command_parser_test
        command_parser_test.cpp
        order_book_test.cpp
        request_processor_test.cpp
        random_order_stream.h
        replay_log_test.cpp
        binary_protocol_test.cpp
        ../src/orderbook/command_parser.cpp
)

target_link_libraries(command_parser_test
        PRIVATE GTest::gtest_main matching_engine_protocol
)

include(GoogleTest)
gtest_discover_tests(command_parser_test)

