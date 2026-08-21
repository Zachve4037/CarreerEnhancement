
#ifndef CPPFIRST_FILE_STATS_H
#define CPPFIRST_FILE_STATS_H
#include <cstddef>
#include <string>

struct FileStats {
  std::size_t lines;
  std::size_t words;
  std::size_t bytes;
};

FileStats analyzeFile(const std::string& path);

#endif // CPPFIRST_FILE_STATS_H
