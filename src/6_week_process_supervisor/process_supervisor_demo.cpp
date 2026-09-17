// #include "process_supervisor.h"
//
// #include <csignal>
// #include <iostream>
// #include <string>
// #include <vector>
//
// int main(int argc, char* argv[]) {
//   if (argc < 2) {
//     std::cerr << "Usage: process_supervisor_demo <program> [args...]\n";
//     return 2;
//   }
//
//   std::vector<std::string> command;
//   command.reserve(static_cast<std::size_t>(argc - 1));
//   for (int i = 1; i < argc; ++i) {
//     command.emplace_back(argv[i]);
//   }
//
//   try {
//     ProcessSupervisor supervisor;
//     const ProcessResult result = supervisor.run(command);
//
//     std::cout << "--- captured output ---\n";
//     std::cout << result.output;
//     if (!result.output.empty() && result.output.back() != '\n') {
//       std::cout << '\n';
//     }
//     std::cout << "--- result ---\n";
//
//     switch (result.status) {
//     case ProcessStatus::exited:
//       std::cout << "status: exited\n";
//       std::cout << "exit_code: " << result.exit_code << '\n';
//       return result.exit_code;
//     case ProcessStatus::signaled:
//       std::cout << "status: signaled\n";
//       std::cout << "signal: " << result.signal << '\n';
//       return 128 + result.signal;
//     case ProcessStatus::failed_to_start:
//       std::cout << "status: failed_to_start\n";
//       std::cout << "exit_code: " << result.exit_code << '\n';
//       return 127;
//     }
//   } catch (const std::exception& ex) {
//     std::cerr << "supervisor error: " << ex.what() << '\n';
//     return 1;
//   }
//
//   return 1;
// }
