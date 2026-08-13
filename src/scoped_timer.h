// //
// // Created by zachvem on 12-Aug-26.
// //
//
// #ifndef CPPFIRST_TIMER_H
// #define CPPFIRST_TIMER_H
//
// class scoped_timer {
// public:
//   explicit scoped_timer();
//   ~scoped_timer();
//
//   scoped_timer(const scoped_timer&) = delete;
//   scoped_timer& operator=(const scoped_timer&) = delete;
//
//   scoped_timer(scoped_timer&& other) noexcept;
//   scoped_timer& operator=(scoped_timer&& other) noexcept;
//
//   int get() const noexcept;
//
// private:
//   int fd_{-1};
// };
//
// #endif // CPPFIRST_TIMER_H
