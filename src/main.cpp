#include "file_stats.h"

#include <iostream>

int main(int argc, char* argv[]) {
  std::string str = "../../test/data/sample.txt";
  if (argc > 1)
    str = argv[1];
  auto [lines, words, bytes] = analyzeFile(str);
  std::cout << "File stats:" << std::endl;
  std::cout << "lines: " << lines << std::endl;
  std::cout << "words: " << words << std::endl;
  std::cout << "bytes: " << bytes << std::endl;
  std::cout << "End of stats!" << std::endl;
  return 0;
}