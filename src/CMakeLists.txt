add_library(core
        file_stats.cpp
        file_stats.h
)

add_executable(cppfirst
        main.cpp
)

target_link_libraries(cppfirst
        PRIVATE
        core
)