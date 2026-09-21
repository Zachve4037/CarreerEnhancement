//
// Created by zachvem on 16-Sep-26.
//

#ifndef CPPFIRST_FRAME_H
#define CPPFIRST_FRAME_H
#include <string>

namespace frame {
  constexpr std::size_t MAX_FRAME_SIZE = 1024 * 1024;

  void send(int socket_fd, std::string_view payload);
  std::string receive(int socket_fd);
};
#endif // CPPFIRST_FRAME_H
