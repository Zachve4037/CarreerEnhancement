#set(core_SRCS ADD ALL SOURCE FILES HERE)

add_library(core INTERFACE)
add_executable(exe main.cpp)
target_link_libraries(exe core)
