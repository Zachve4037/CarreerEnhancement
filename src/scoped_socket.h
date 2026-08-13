// //
// // Created by zachvem on 12-Aug-26.
// //
// ///for the record I am currently on windows so that's why it is commented out
// ///i have installed wsl and compiled and tested it there
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
