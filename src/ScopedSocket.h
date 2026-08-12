//
// Created by zachvem on 12-Aug-26.
//

#ifndef CPPFIRST_SCOPEDSOCKET_H
#define CPPFIRST_SCOPEDSOCKET_H

class ScopedSocket {
public:
  explicit ScopedSocket(int protocol);
  ~ScopedSocket();

  ScopedSocket(const ScopedSocket&) = delete;
  ScopedSocket& operator=(const ScopedSocket&) = delete;

  ScopedSocket(ScopedSocket&& other) noexcept;
  ScopedSocket& operator=(ScopedSocket&& other) noexcept;

  int get() const noexcept;

private:
  long long fd_{-1};
};

#endif // CPPFIRST_SCOPEDSOCKET_H
