add_library(core
        file_stats.cpp
        file_stats.h
        ScopedFile.cpp
        ScopedFile.h
        ScopedTimer.cpp
        ScopedTimer.h
        ScopedSocket.cpp
        ScopedSocket.h
)

add_executable(cppfirst
        main.cpp
)

target_link_libraries(cppfirst
        PRIVATE
        core
)