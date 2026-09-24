#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace socket_utils {

// Send exactly size bytes, handling partial writes.
void send_all(int socket_fd, const void* data, std::size_t size);

// Receive exactly size bytes, handling partial reads.
// Throws if the peer closes the connection before all bytes arrive.
void receive_all(int socket_fd, void* data, std::size_t size);

// Create, bind and listen on a TCP socket.
// Returns the listening socket's file descriptor.
int create_server_socket(std::uint16_t port);

// Connect to a TCP server using its hostname/IP and port.
// Returns a connected socket descriptor.
int connect_to_server(
    const std::string& host,
    std::uint16_t port
);

// Set socket send/receive timeouts in milliseconds.
void set_receive_timeout(int socket_fd, int timeout_ms);
void set_send_timeout(int socket_fd, int timeout_ms);

}