include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)

add_executable(runUnitTests
        gtest.cpp
        scoped_file_test.cpp
        scoped_timer_test.cpp
        scoped_socket_test.cpp
        binary_search_test.cpp
        byte_buffer_test.cpp
        process_supervisor_test.cpp
        frame_test.cpp
        tcp_server_test.cpp
        bounded_blocking_queue_test.cpp
        spsc_queue_test.cpp
)

target_link_libraries(runUnitTests
        PRIVATE
        core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(runUnitTests)