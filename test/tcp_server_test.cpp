// #include "../src/7_week_networking_and_protocol_framing/tcp_server.h"
// #include "../src/7_week_networking_and_protocol_framing/tcp_client.h"
//
// #include <gtest/gtest.h>
//
// #include <chrono>
// #include <thread>
//
// // This test uses a dedicated local port.
// // Make sure another application is not already using it.
// constexpr std::uint16_t TEST_PORT = 19090;
//
// TEST(TcpServerTest, EchoesClientMessage) {
//   TcpServer server(TEST_PORT);
//
//   // Run the blocking server loop in a background thread.
//   std::thread server_thread([&]() {
//       server.run();
//   });
//
//   // Give the server time to bind and listen.
//   std::this_thread::sleep_for(
//       std::chrono::milliseconds(200)
//   );
//
//   try {
//     TcpClient client(
//         "127.0.0.1",
//         TEST_PORT,
//         2000,
//         3
//     );
//
//     const std::string response = client.echo("hello server");
//
//     EXPECT_EQ(response, "hello server");
//
//     client.disconnect();
//   } catch (...) {
//     server.stop();
//     server_thread.join();
//     throw;
//   }
//
//   server.stop();
//   server_thread.join();
// }