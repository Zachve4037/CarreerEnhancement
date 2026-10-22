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
)

target_link_libraries(command_parser_test
        PRIVATE GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(command_parser_test)

