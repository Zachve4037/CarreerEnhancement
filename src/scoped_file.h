//
// Created by zachvem on 12-Aug-26.
//

#ifndef CPPFIRST_SCOPEDFILE_H
#define CPPFIRST_SCOPEDFILE_H

class scoped_file {
public:
  explicit scoped_file(const char* path); //disable implicit conversion
  ~scoped_file();

  scoped_file(const scoped_file&) = delete; //delete coppying
  scoped_file& operator=(const scoped_file&) = delete;

  scoped_file(scoped_file&& other) noexcept; //noexception when copying
  scoped_file& operator=(scoped_file&& other) noexcept;

  int get() const noexcept;

private:
  int fd_{-1};
};

#endif // CPPFIRST_SCOPEDFILE_H
