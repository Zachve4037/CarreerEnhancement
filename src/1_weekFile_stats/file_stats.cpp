//
// Created by zachvem on 09-Aug-26.
//

#include "../file_stats.h"

#include <fstream>
#include <sstream>

FileStats analyzeFile(const std::string& path) {
  std::ifstream input_stream(path);
  if (!input_stream)
    throw std::runtime_error("Unable to open file " + path);

  FileStats stats{};
  std::string line;
  while (std::getline(input_stream, line)) {
    ++stats.lines;
    stats.bytes += line.size();
    std::stringstream lineStream(line);
    while (std::getline(lineStream, line, ' ')) {
      ++stats.words;
    }
  }
  return stats;
}

