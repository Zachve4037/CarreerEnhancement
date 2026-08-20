cmake_minimum_required(VERSION 3.20)

project(CppFirst)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

option(ENABLE_SANITIZERS "Enable AddressSanitizer and UBSan" OFF)

if(ENABLE_SANITIZERS)
    add_compile_options(
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
    )

    add_link_options(
            -fsanitize=address,undefined
    )
endif()

add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(benchmark)