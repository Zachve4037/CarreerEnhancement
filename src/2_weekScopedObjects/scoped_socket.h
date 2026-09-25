#ifndef CPPFIRST_SCOPED_SOCKET_H
#define CPPFIRST_SCOPED_SOCKET_H

class scoped_socket {
public:
  explicit scoped_socket(int protocol);

  ~scoped_socket();

  scoped_socket(const scoped_socket&) = delete;
  scoped_socket& operator=(const scoped_socket&) = delete;

  scoped_socket(scoped_socket&& other) noexcept;
  scoped_socket& operator=(scoped_socket&& other) noexcept;

  int get() const noexcept;

private:
  int fd_{-1};
};

#endif // CPPFIRST_SCOPED_SOCKET_H