#include "tcp_client.h"

#include "frame.h"

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>

#include <unistd.h>

TcpClient::TcpClient(
    std::string host,
    std::uint16_t port,
    int timeout_ms,
    int max_reconnect_attempts
)
    : host_(std::move(host)),
      port_(port),
      timeout_ms_(timeout_ms),
      max_reconnect_attempts_(max_reconnect_attempts) {

    if (timeout_ms_ <= 0) {
        throw std::invalid_argument(
            "timeout must be positive"
        );
    }

    if (max_reconnect_attempts_ < 0) {
        throw std::invalid_argument(
            "reconnect attempts cannot be negative"
        );
    }
}

TcpClient::~TcpClient() {
    disconnect();
}

bool TcpClient::connected() const noexcept {
    return socket_fd_ != -1;
}

void TcpClient::connect() {
    // Don't create another connection if one is already open.
    if (connected()) {
        return;
    }

    socket_fd_ = socket_utils::connect_to_server(
        host_,
        port_
    );

    try {
        socket_utils::set_receive_timeout(
            socket_fd_,
            timeout_ms_
        );

        socket_utils::set_send_timeout(
            socket_fd_,
            timeout_ms_
        );
    } catch (...) {
        disconnect();
        throw;
    }

    std::cout << "Connected to "
              << host_ << ':' << port_ << '\n';
}

void TcpClient::disconnect() noexcept {
    if (socket_fd_ != -1) {
        ::close(socket_fd_);
        socket_fd_ = -1;
    }
}

void TcpClient::reconnect() {
    disconnect();

    // Fixed 100 ms delay between attempts.
    // This avoids an immediate busy retry loop.
    for (int attempt = 0;
         attempt < max_reconnect_attempts_;
         ++attempt) {

        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );

        try {
            connect();
            return;
        } catch (const std::exception& e) {
            std::cerr << "Reconnect attempt "
                      << (attempt + 1)
                      << " failed: "
                      << e.what() << '\n';
        }
    }

    throw std::runtime_error(
        "all reconnect attempts failed"
    );
}

std::string TcpClient::echo(std::string_view message) {
    // A client can be constructed without connecting immediately.
    if (!connected()) {
        connect();
    }

    try {
        frame::send(socket_fd_, message);
        return frame::receive(socket_fd_);
    } catch (const std::exception&) {
        // The current connection may no longer be usable.
        disconnect();

        // Try to establish a new connection.
        reconnect();

        // Retry the echo operation once on the new connection.
        // This is safe for this echo protocol because the server
        // has no side effects beyond returning the request.
        try {
            frame::send(socket_fd_, message);
            return frame::receive(socket_fd_);
        } catch (...) {
            disconnect();
            throw;
        }
    }
}