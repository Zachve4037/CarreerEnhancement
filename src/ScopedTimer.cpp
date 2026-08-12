//
// Created by zachvem on 12-Aug-26.
//

#include "ScopedTimer.h"

#include <pthread_time.h>
#include <system_error>
// #include <timerfd.h>
// ///for the record I am currently on windows so I don't really know if this would be good implementation
// ScopedTimer::ScopedTimer(const int *ptr)
// {
//   this->fd_ = timerfd_create(CLOCK_REALTIME, 0);
//   if (this->fd_ == -1) {
//     throw std::system_error(
//       errno,
//       std::generic_category(),
//       "Failed to create timer!");
//   }
// }
//
// ScopedTimer::~ScopedTimer() {
//   if (this->fd_ != -1) {
//     this->fd_.close();
//   }
// }
//
// ScopedTimer::ScopedTimer(ScopedTimer &&other) noexcept : fd_(other.fd_) {
//   other.fd_ = -1;
// }
//
// ScopedTimer &ScopedTimer::operator=(ScopedTimer &&other) noexcept {
//   if (this != &other) {
//     if (this->fd_ != -1) {
//       this->fd_close();
//     }
//     this->fd_ = other.fd_;
//     other.fd_ = -1;
//   }
//   return *this;
// }
//
// int ScopedTimer::get() const noexcept {
//   return this->fd_;
// }