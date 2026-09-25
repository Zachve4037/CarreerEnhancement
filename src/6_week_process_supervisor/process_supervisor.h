#ifndef CPPFIRST_PROCESS_SUPERVISOR_H
#define CPPFIRST_PROCESS_SUPERVISOR_H

#include <string>
#include <vector>

enum class ProcessStatus {
  exited,
  signaled,
  failed_to_start
};

struct ProcessResult {
  ProcessStatus status;
  int exit_code{-1};
  int signal{-1};
  std::string output;
};

class ProcessSupervisor {
public:
  ProcessResult run(const std::vector<std::string>& command) const;
};

#endif
