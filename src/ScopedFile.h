//
// Created by zachvem on 12-Aug-26.
//

#ifndef CPPFIRST_SCOPEDFILE_H
#define CPPFIRST_SCOPEDFILE_H

class ScopedFile {
public:
  explicit ScopedFile(const char* path); //disable implicit conversion
  ~ScopedFile();

  ScopedFile(const ScopedFile&) = delete; //delete coppying
  ScopedFile& operator=(const ScopedFile&) = delete;

  ScopedFile(ScopedFile&& other) noexcept; //noexception when copying
  ScopedFile& operator=(ScopedFile&& other) noexcept;

  int get() const noexcept;

private:
  int fd_{-1};
};

#endif // CPPFIRST_SCOPEDFILE_H
