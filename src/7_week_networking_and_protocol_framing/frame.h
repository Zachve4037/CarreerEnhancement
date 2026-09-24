#pragma once

#include <cstddef>
#include <string>
#include <string_view>

namespace frame {

// Maximum allowed payload: 1 MiB, excluding the 4-byte header.
constexpr std::size_t MAX_FRAME_SIZE = 1024 * 1024;

// Wire format: 4-byte unsigned length in network byte order,
// followed by exactly length bytes of payload.
void send(int socket_fd, std::string_view payload);

// Reads one complete frame and returns its payload.
std::string receive(int socket_fd);

}