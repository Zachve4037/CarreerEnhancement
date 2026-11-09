#include "../src/orderbook/replay_log.h"
#include "random_order_stream.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>

namespace {

[[nodiscard]]
replay_log::Recording make_recording(
    std::uint64_t seed,
    std::size_t operation_count) {

  replay_log::Recorder recorder(seed);
  RandomOrderStream stream(seed);

  for (std::size_t step = 0;
       step < operation_count;
       ++step) {

    const Sequence sequence =
        static_cast<Sequence>(step + 1);

    const RequestId request_id =
        50'000 + sequence;

    const ClientRequest request =
        stream.next(
            recorder
                .processor()
                .order_book());

    const auto result =
        recorder.record(
            request_id,
            sequence,
            request);

    EXPECT_TRUE(result.accepted)
        << "seed=" << seed
        << " step=" << step;
  }

  return recorder.finish();
}

} // namespace

TEST(
    ReplayLog,
    SameSeedProducesSameInputAndOutcomeChecksums) {

  constexpr std::uint64_t seed =
      0xc0ffee'2026ULL;

  const auto first =
      make_recording(seed, 1'500);

  const auto second =
      make_recording(seed, 1'500);

  EXPECT_EQ(
      first.input_checksum,
      second.input_checksum);

  EXPECT_EQ(
      first.outcome_checksum,
      second.outcome_checksum);

  EXPECT_EQ(
      first.records.size(),
      second.records.size());
}

TEST(
    ReplayLog,
    RecorderReplayerValidatesDeterministicReplay) {

  constexpr std::uint64_t seed =
      0x1234'5678'9abc'def0ULL;

  const auto recording =
      make_recording(seed, 2'000);

  const auto report =
      replay_log::Replayer::
          replay_and_validate(recording);

  EXPECT_TRUE(report.valid());

  EXPECT_EQ(
      report.error,
      replay_log::ReplayError::None);

  EXPECT_EQ(
      report.actual_input_checksum,
      recording.input_checksum);

  EXPECT_EQ(
      report.actual_outcome_checksum,
      recording.outcome_checksum);

  EXPECT_EQ(
      report.results.size(),
      recording.records.size());
}

TEST(
    ReplayLog,
    ChangedSeedIsDetectedBeforeReplay) {

  auto recording =
      make_recording(12345, 200);

  ++recording.seed;

  const auto report =
      replay_log::Replayer::
          replay_and_validate(recording);

  EXPECT_FALSE(report.valid());

  EXPECT_EQ(
      report.error,
      replay_log::ReplayError::
          InputChecksumMismatch);

  EXPECT_TRUE(
      report.results.empty());
}

TEST(
    ReplayLog,
    ChangedCommandIsDetectedBeforeReplay) {

  auto recording =
      make_recording(67890, 200);

  ASSERT_FALSE(
      recording.records.empty());

  // First generated operation must be NEW because
  // the initial order book is empty.
  auto &first_order =
      std::get<NewOrder>(
          recording
              .records
              .front()
              .request);

  ++first_order.quantity;

  const auto report =
      replay_log::Replayer::
          replay_and_validate(recording);

  EXPECT_FALSE(report.valid());

  EXPECT_EQ(
      report.error,
      replay_log::ReplayError::
          InputChecksumMismatch);
}

TEST(
    ReplayLog,
    ChangedExpectedOutcomeChecksumIsDetectedAfterReplay) {

  auto recording =
      make_recording(24680, 200);

  // Simulate corrupted expected output metadata.
  recording.outcome_checksum ^= 1U;

  const auto report =
      replay_log::Replayer::
          replay_and_validate(recording);

  EXPECT_FALSE(report.valid());

  EXPECT_EQ(
      report.error,
      replay_log::ReplayError::
          OutcomeChecksumMismatch);

  EXPECT_EQ(
      report.results.size(),
      recording.records.size());
}