// //
// // Created by zachvem on 12-Aug-26.
// //
//
// #ifndef CPPFIRST_SCOPEDSOCKET_H
// #define CPPFIRST_SCOPEDSOCKET_H
// #include <psdk_inc/_socket_types.h>
//
// class scoped_socket {
// public:
//   explicit scoped_socket(int protocol);
//   ~scoped_socket();
//
//   scoped_socket(const scoped_socket&) = delete;
//   scoped_socket& operator=(const scoped_socket&) = delete;
//
//   scoped_socket(scoped_socket&& other) noexcept;
//   scoped_socket& operator=(scoped_socket&& other) noexcept;
//
//   int get() const noexcept;
//
// private:
//   SOCKET fd_{INVALID_SOCKET};
// };
//
// #endif // CPPFIRST_SCOPEDSOCKET_H
