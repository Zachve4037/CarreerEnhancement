#include "socket_utils.h"

#include <cerrno>
#include <stdexcept>
#include <system_error>

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace socket_utils {

void send_all(
    int socket_fd,
    const void* data,
    std::size_t size
) {
    const char* current = static_cast<const char*>(data);
    std::size_t remaining = size;

    while (remaining > 0) {
        // MSG_NOSIGNAL prevents the process from being terminated by
        // SIGPIPE if the peer has already closed the connection.
        const ssize_t n = ::send(
            socket_fd,
            current,
            remaining,
            MSG_NOSIGNAL
        );

        if (n < 0) {
            // EINTR means the system call was interrupted by a signal.
            // Nothing was necessarily sent, so retry the operation.
            if (errno == EINTR) {
                continue;
            }

            throw std::system_error(
                errno,
                std::generic_category(),
                "send failed"
            );
        }

        if (n == 0) {
            // No progress was made. Avoid an infinite loop.
            throw std::runtime_error("send returned zero");
        }

        // send() may have sent only part of the buffer.
        current += n;
        remaining -= static_cast<std::size_t>(n);
    }
}

void receive_all(
    int socket_fd,
    void* data,
    std::size_t size
) {
    char* current = static_cast<char*>(data);
    std::size_t remaining = size;

    while (remaining > 0) {
        const ssize_t n = ::recv(
            socket_fd,
            current,
            remaining,
            0
        );

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }

            throw std::system_error(
                errno,
                std::generic_category(),
                "recv failed"
            );
        }

        if (n == 0) {
            // The peer closed its sending direction before the complete
            // requested buffer arrived. This is an incomplete message.
            throw std::runtime_error(
                "connection closed while receiving data"
            );
        }

        current += n;
        remaining -= static_cast<std::size_t>(n);
    }
}

void set_receive_timeout(int socket_fd, int timeout_ms) {
    if (timeout_ms <= 0) {
        throw std::invalid_argument(
            "receive timeout must be positive"
        );
    }

    // SO_RCVTIMEO expects a timeval structure.
    timeval timeout{};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    if (::setsockopt(
            socket_fd,
            SOL_SOCKET,
            SO_RCVTIMEO,
            &timeout,
            sizeof(timeout)
        ) == -1) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "setsockopt SO_RCVTIMEO failed"
        );
    }
}

void set_send_timeout(int socket_fd, int timeout_ms) {
    if (timeout_ms <= 0) {
        throw std::invalid_argument(
            "send timeout must be positive"
        );
    }

    timeval timeout{};
    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_usec = (timeout_ms % 1000) * 1000;

    if (::setsockopt(
            socket_fd,
            SOL_SOCKET,
            SO_SNDTIMEO,
            &timeout,
            sizeof(timeout)
        ) == -1) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "setsockopt SO_SNDTIMEO failed"
        );
    }
}

int create_server_socket(std::uint16_t port) {
    // AF_INET means IPv4; SOCK_STREAM means TCP.
    const int fd = ::socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (fd == -1) {
        throw std::system_error(
            errno,
            std::generic_category(),
            "socket failed"
        );
    }

    // Allow quick restart after the previous server has closed.
    int reuse = 1;

    if (::setsockopt(
            fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)
        ) == -1) {
        const int saved_errno = errno;
        ::close(fd);

        throw std::system_error(
            saved_errno,
            std::generic_category(),
            "setsockopt SO_REUSEADDR failed"
        );
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;

    // INADDR_ANY means listen on all local IPv4 interfaces.
    address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Ports in sockaddr_in must be in network byte order.
    address.sin_port = htons(port);

    if (::bind(
            fd,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) == -1) {
        const int saved_errno = errno;
        ::close(fd);

        throw std::system_error(
            saved_errno,
            std::generic_category(),
            "bind failed"
        );
    }

    // The operating system can queue up to 10 pending connections.
    if (::listen(fd, 10) == -1) {
        const int saved_errno = errno;
        ::close(fd);

        throw std::system_error(
            saved_errno,
            std::generic_category(),
            "listen failed"
        );
    }

    return fd;
}

int connect_to_server(
    const std::string& host,
    std::uint16_t port
) {
    // getaddrinfo supports both hostnames and numeric IP addresses.
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;      // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP

    addrinfo* addresses = nullptr;

    const std::string service = std::to_string(port);

    const int result = ::getaddrinfo(
        host.c_str(),
        service.c_str(),
        &hints,
        &addresses
    );

    if (result != 0) {
        throw std::runtime_error(
            std::string("getaddrinfo failed: ") +
            gai_strerror(result)
        );
    }

    int connected_fd = -1;
    int last_error = ECONNREFUSED;

    // Try each resolved address until a connection succeeds.
    for (addrinfo* address = addresses;
         address != nullptr;
         address = address->ai_next) {

        const int fd = ::socket(
            address->ai_family,
            address->ai_socktype,
            address->ai_protocol
        );

        if (fd == -1) {
            last_error = errno;
            continue;
        }

        if (::connect(
                fd,
                address->ai_addr,
                address->ai_addrlen
            ) == 0) {
            connected_fd = fd;
            break;
        }

        last_error = errno;
        ::close(fd);
    }

    ::freeaddrinfo(addresses);

    if (connected_fd == -1) {
        throw std::system_error(
            last_error,
            std::generic_category(),
            "connect failed"
        );
    }

    return connected_fd;
}

}