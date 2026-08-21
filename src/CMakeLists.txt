add_library(core
        1_weekFile_stats/file_stats.cpp
        1_weekFile_stats/file_stats.h
        2_weekScopedObjects/scoped_file.cpp
        2_weekScopedObjects/scoped_file.h
        2_weekScopedObjects/scoped_timer.cpp
        2_weekScopedObjects/scoped_timer.h
        2_weekScopedObjects/scoped_socket.cpp
        2_weekScopedObjects/scoped_socket.h
        3_weekBinarySearch/binary_search.cpp
        3_weekBinarySearch/binary_search.h
        3_weekByteBuffer/byte_buffer.cpp
        3_weekByteBuffer/byte_buffer.h
)

add_executable(cppfirst
        main.cpp
)

target_link_libraries(cppfirst
        PRIVATE
        core
)