cmake_minimum_required(VERSION 3.20)

project(CppFirst)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

add_compile_options(
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
)

add_link_options(
        -fsanitize=address,undefined
)

add_subdirectory(src)
add_subdirectory(test)