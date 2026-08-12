//
// Created by zachvem on 12-Aug-26.
//

#ifndef CPPFIRST_TIMER_H
#define CPPFIRST_TIMER_H

class ScopedTimer {
public:
  explicit ScopedTimer(const int* ptr);
  ~ScopedTimer();

  ScopedTimer(const ScopedTimer&) = delete;
  ScopedTimer& operator=(const ScopedTimer&) = delete;

  ScopedTimer(ScopedTimer&& other) noexcept;
  ScopedTimer& operator=(ScopedTimer&& other) noexcept;

  int get() const noexcept;

private:
  int fd_{-1};
};

#endif // CPPFIRST_TIMER_H
