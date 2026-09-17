// #include "process_supervisor.h"
//
// #include <cerrno>
// #include <cstring>
// #include <stdexcept>
// #include <system_error>
//
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <unistd.h>
//
// ProcessResult ProcessSupervisor::run(const std::vector<std::string>& command) const {
//     if (command.empty()) {
//         throw std::invalid_argument("command must not be empty");
//     }
//
//     int pipe_fds[2];
//     if (::pipe(pipe_fds) == -1) {
//         throw std::system_error(errno, std::generic_category(), "pipe failed");
//     }
//
//     const pid_t pid = ::fork();
//
//     if (pid == -1) {
//         const int saved_errno = errno;
//         ::close(pipe_fds[0]);
//         ::close(pipe_fds[1]);
//         throw std::system_error(saved_errno, std::generic_category(), "fork failed");
//     }
//
//     if (pid == 0) {
//         // The child only writes to the pipe.
//         ::close(pipe_fds[0]);
//
//         // Send both stdout and stderr to the same pipe so the parent can
//         // capture all output from the child.
//         if (::dup2(pipe_fds[1], STDOUT_FILENO) == -1 ||
//             ::dup2(pipe_fds[1], STDERR_FILENO) == -1) {
//             _exit(127);
//         }
//
//         // stdout/stderr now refer to the pipe, so this extra descriptor is
//         // no longer needed.
//         ::close(pipe_fds[1]);
//
//         std::vector<char*> argv;
//         argv.reserve(command.size() + 1);
//         for (const std::string& argument : command) {
//             argv.push_back(const_cast<char*>(argument.c_str()));
//         }
//         argv.push_back(nullptr);
//
//         // execvp searches PATH and replaces the child process image.
//         ::execvp(argv[0], argv.data());
//
//         // Only reached when execvp fails.  stderr is already redirected,
//         // so the parent receives this diagnostic through the pipe.
//         const char message[] = "execvp failed: ";
//         (void)::write(STDERR_FILENO, message, sizeof(message) - 1);
//         (void)::write(STDERR_FILENO, std::strerror(errno), std::strlen(std::strerror(errno)));
//         (void)::write(STDERR_FILENO, "\n", 1);
//         _exit(127);
//     }
//
//     // The parent only reads from the pipe.
//     ::close(pipe_fds[1]);
//
//     ProcessResult result{};
//     char buffer[4096];
//
//     while (true) {
//         const ssize_t bytes_read = ::read(pipe_fds[0], buffer, sizeof(buffer));
//
//         if (bytes_read > 0) {
//             result.output.append(buffer, static_cast<std::size_t>(bytes_read));
//             continue;
//         }
//
//         if (bytes_read == 0) {
//             break; // Child closed its output descriptors: EOF.
//         }
//
//         if (errno == EINTR) {
//             continue;
//         }
//
//         const int saved_errno = errno;
//         ::close(pipe_fds[0]);
//
//         int status = 0;
//         while (::waitpid(pid, &status, 0) == -1 && errno == EINTR) {
//         }
//
//         throw std::system_error(saved_errno, std::generic_category(), "read failed");
//     }
//
//     ::close(pipe_fds[0]);
//
//     int status = 0;
//     while (::waitpid(pid, &status, 0) == -1) {
//         if (errno != EINTR) {
//             throw std::system_error(errno, std::generic_category(), "waitpid failed");
//         }
//     }
//
//     if (WIFEXITED(status)) {
//         result.status = (WEXITSTATUS(status) == 127 && result.output.rfind("execvp failed: ", 0) == 0)
//             ? ProcessStatus::failed_to_start
//             : ProcessStatus::exited;
//         result.exit_code = WEXITSTATUS(status);
//     } else if (WIFSIGNALED(status)) {
//         result.status = ProcessStatus::signaled;
//         result.signal = WTERMSIG(status);
//     }
//
//     return result;
// }
