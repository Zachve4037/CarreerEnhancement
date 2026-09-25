#include "tcp_server.h"

#include "frame.h"
#include "socket_utils.h"

#include <cerrno>
#include <iostream>
#include <stdexcept>
#include <system_error>

#include <sys/socket.h>
#include <unistd.h>

TcpServer::TcpServer(std::uint16_t port)
    : port_(port) {
}

void TcpServer::run() {
    server_fd_ = socket_utils::create_server_socket(port_);
    running_ = true;

    std::cout << "Server listening on port "
              << port_ << '\n';

    while (running_) {
        // accept() blocks until a client connects.
        const int client_fd = ::accept(
            server_fd_,
            nullptr,
            nullptr
        );

        if (client_fd == -1) {
            if (errno == EINTR) {
                continue;
            }

            // stop() closes the listening socket to interrupt accept().
            if (!running_) {
                break;
            }

            throw std::system_error(
                errno,
                std::generic_category(),
                "accept failed"
            );
        }

        std::cout << "Client connected\n";

        // The server handles this client before accepting another one.
        handle_client(client_fd);

        ::close(client_fd);

        std::cout << "Client disconnected\n";
    }

    if (server_fd_ != -1) {
        ::close(server_fd_);
        server_fd_ = -1;
    }
}

void TcpServer::handle_client(int client_fd) {
    try {
        while (running_) {
            // receive() blocks until one complete frame arrives.
            const std::string message = frame::receive(client_fd);

            std::cout << "Received " << message.size()
                      << " bytes\n";

            // Echo the exact payload back to the client.
            frame::send(client_fd, message);
        }
    } catch (const std::exception& e) {
        // A disconnect, incomplete frame, or socket error ends
        // this client's session, but should not terminate the server.
        std::cerr << "Client session ended: "
                  << e.what() << '\n';
    }
}

void TcpServer::stop() noexcept {
    running_ = false;

    // Closing the listening socket can interrupt a blocking accept().
    if (server_fd_ != -1) {
        ::close(server_fd_);
        server_fd_ = -1;
    }
}