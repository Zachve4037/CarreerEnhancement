// #include "../src/6_week_process_supervisor/process_supervisor.h"
//
// #include <csignal>
// #include <stdexcept>
// #include <string>
// #include <vector>
//
// #include <gtest/gtest.h>
//
// TEST(ProcessSupervisor, CapturesStdout) {
//     ProcessSupervisor supervisor;
//
//     const auto result = supervisor.run({"sh", "-c", "printf 'hello from child\\n'"});
//
//     EXPECT_EQ(result.status, ProcessStatus::exited);
//     EXPECT_EQ(result.exit_code, 0);
//     EXPECT_EQ(result.output, "hello from child\n");
// }
//
// TEST(ProcessSupervisor, CapturesStderr) {
//     ProcessSupervisor supervisor;
//
//     const auto result = supervisor.run({"sh", "-c", "printf 'error message\\n' >&2"});
//
//     EXPECT_EQ(result.status, ProcessStatus::exited);
//     EXPECT_EQ(result.exit_code, 0);
//     EXPECT_EQ(result.output, "error message\n");
// }
//
// TEST(ProcessSupervisor, CapturesStdoutAndStderr) {
//     ProcessSupervisor supervisor;
//
//     const auto result = supervisor.run({
//         "sh", "-c", "printf 'out\\n'; printf 'err\\n' >&2"
//     });
//
//     EXPECT_EQ(result.status, ProcessStatus::exited);
//     EXPECT_EQ(result.exit_code, 0);
//     EXPECT_EQ(result.output, "out\nerr\n");
// }
//
// TEST(ProcessSupervisor, ReturnsExitCode) {
//     ProcessSupervisor supervisor;
//
//     const auto result = supervisor.run({"sh", "-c", "exit 42"});
//
//     EXPECT_EQ(result.status, ProcessStatus::exited);
//     EXPECT_EQ(result.exit_code, 42);
// }
//
// TEST(ProcessSupervisor, DetectsSignalTermination) {
//     ProcessSupervisor supervisor;
//
//     const auto result = supervisor.run({"sh", "-c", "kill -TERM $$"});
//
//     EXPECT_EQ(result.status, ProcessStatus::signaled);
//     EXPECT_EQ(result.signal, SIGTERM);
// }
//
// TEST(ProcessSupervisor, ReportsFailedExec) {
//     ProcessSupervisor supervisor;
//
//     const auto result = supervisor.run({"definitely-not-a-real-program-xyz"});
//
//     EXPECT_EQ(result.status, ProcessStatus::failed_to_start);
//     EXPECT_EQ(result.exit_code, 127);
//     EXPECT_NE(result.output.find("execvp failed:"), std::string::npos);
// }
//
// TEST(ProcessSupervisor, RejectsEmptyCommand) {
//     ProcessSupervisor supervisor;
//
//     EXPECT_THROW(supervisor.run({}), std::invalid_argument);
// }
