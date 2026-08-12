add_library(core
        file_stats.cpp
        file_stats.h
        scoped_file.cpp
        scoped_file.h
        scoped_timer.cpp
        scoped_timer.h
        scoped_socket.cpp
        scoped_socket.h
)

add_executable(cppfirst
        main.cpp
)

target_link_libraries(cppfirst
        PRIVATE
        core
)