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
        6_week_process_supervisor/process_supervisor.cpp
        6_week_process_supervisor/process_supervisor.h
        7_week_networking_and_protocol_framing/frame.cpp
        7_week_networking_and_protocol_framing/frame.h
        7_week_networking_and_protocol_framing/tcp_server.cpp
        7_week_networking_and_protocol_framing/tcp_server.h
        7_week_networking_and_protocol_framing/tcp_client.cpp
        7_week_networking_and_protocol_framing/tcp_client.h
        7_week_networking_and_protocol_framing/socket_utils.cpp
        7_week_networking_and_protocol_framing/socket_utils.h
        8_week_threads_atomics_and_bounded_queues/bounded_blocking_queue.h
        8_week_threads_atomics_and_bounded_queues/spsc_queue.h
        8_week_threads_atomics_and_bounded_queues/queue_demo.cpp
)

add_executable(cppfirst
        main.cpp
)

target_link_libraries(cppfirst
        PRIVATE
        core
)

add_executable(process_supervisor_demo
        6_week_process_supervisor/process_supervisor_demo.cpp
)

target_link_libraries(process_supervisor_demo
        PRIVATE
        core
)