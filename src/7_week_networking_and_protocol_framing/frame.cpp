#include "frame.h"
#include "socket_utils.h"

#include <arpa/inet.h>
#include <cstdint>
#include <stdexcept>

namespace frame {

void send(int socket_fd, std::string_view payload) {
  // Reject oversized frames before converting the length to uint32_t.
  if (payload.size() > MAX_FRAME_SIZE) {
    throw std::invalid_argument("frame is too large");
  }

  // Our protocol uses a fixed-width 4-byte length field.
  const std::uint32_t length =
      static_cast<std::uint32_t>(payload.size());

  // Convert from host byte order to network byte order (big-endian).
  const std::uint32_t network_length = htonl(length);

  // Send the header first, followed by the payload.
  // send_all handles partial writes.
  socket_utils::send_all(
      socket_fd,
      &network_length,
      sizeof(network_length)
  );

  socket_utils::send_all(
      socket_fd,
      payload.data(),
      payload.size()
  );
}

std::string receive(int socket_fd) {
  // First read exactly the 4-byte header.
  std::uint32_t network_length{};

  socket_utils::receive_all(
      socket_fd,
      &network_length,
      sizeof(network_length)
  );

  // Convert the received length to host byte order.
  const std::uint32_t length = ntohl(network_length);

  // Validate before allocating memory.
  if (length > MAX_FRAME_SIZE) {
    throw std::runtime_error("frame is too large");
  }

  // Construct a string with exactly length bytes.
  // This also supports binary payloads and embedded null characters.
  std::string payload(length, '\0');

  // Read exactly the payload length specified by the header.
  socket_utils::receive_all(
      socket_fd,
      payload.data(),
      payload.size()
  );

  return payload;
}

}