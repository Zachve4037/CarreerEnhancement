// //
// // Created by zachvem on 12-Aug-26.
// //
//
// #include "scoped_timer.h"
//
// #include <cerrno>
// #include <pthread_time.h>
// #include <sys/timerfd.h>
// #include <system_error>
// #include <unistd.h>
// ///for the record I am currently on windows so that's why it is commented out
// ///i have installed wsl and compiled and tested it there
// scoped_timer::scoped_timer()
// {
//   this->fd_ = ::timerfd_create(CLOCK_MONOTONIC, 0);
//   if (this->fd_ == -1) {
//     throw std::system_error(
//       errno,
//       std::generic_category(),
//       "Failed to create timer!");
//   }
// }
//
// scoped_timer::~scoped_timer() {
//   if (this->fd_ != -1) {
//     ::close(fd_);
//   }
// }
//
// scoped_timer::scoped_timer(scoped_timer &&other) noexcept : fd_(other.fd_) {
//   other.fd_ = -1;
// }
//
// scoped_timer &scoped_timer::operator=(scoped_timer &&other) noexcept {
//   if (this != &other) {
//     if (this->fd_ != -1) {
//       ::close(fd_);
//     }
//     this->fd_ = other.fd_;
//     other.fd_ = -1;
//   }
//   return *this;
// }
//
// int scoped_timer::get() const noexcept {
//   return this->fd_;
// }