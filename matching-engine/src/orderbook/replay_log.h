#ifndef CPPFIRST_REPLAY_LOG_H
#define CPPFIRST_REPLAY_LOG_H

#include "request_processor.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace replay_log {

// Stable checksum implementation.
//
// std::hash is intentionally not used because its exact value is not
// intended to be a persistent replay format.
//
// FNV-1a is NOT a cryptographic hash. It is used here to detect accidental
// corruption and nondeterministic replay differences.
class Fnv1a64 {
public:
  void add_byte(std::uint8_t value) noexcept {
    state_ ^= value;
    state_ *= kPrime;
  }

  void add_u64(std::uint64_t value) noexcept {
    // Explicit byte order means checksum generation does not depend
    // on native host endianness.
    for (unsigned shift = 0; shift < 64; shift += 8) {
      add_byte(
          static_cast<std::uint8_t>((value >> shift) & 0xffU));
    }
  }

  void add_bool(bool value) noexcept {
    add_byte(value ? 1U : 0U);
  }

  [[nodiscard]] std::uint64_t value() const noexcept {
    return state_;
  }

private:
  static constexpr std::uint64_t kOffsetBasis =
      14695981039346656037ULL;

  static constexpr std::uint64_t kPrime =
      1099511628211ULL;

  std::uint64_t state_{kOffsetBasis};
};

inline void add_new_order(
    Fnv1a64 &hash,
    const NewOrder &order) noexcept {

  hash.add_u64(order.id);
  hash.add_byte(static_cast<std::uint8_t>(order.side));
  hash.add_u64(static_cast<std::uint64_t>(order.price));
  hash.add_u64(order.quantity);
}

inline void add_request(
    Fnv1a64 &hash,
    const ClientRequest &request) noexcept {

  std::visit(
      [&hash](const auto &command) {
        using T = std::decay_t<decltype(command)>;

        if constexpr (std::is_same_v<T, NewOrder>) {
          hash.add_byte(1U);
          add_new_order(hash, command);

        } else if constexpr (std::is_same_v<T, CancelOrder>) {
          hash.add_byte(2U);
          hash.add_u64(command.id);

        } else if constexpr (std::is_same_v<T, ReplaceOrder>) {
          hash.add_byte(3U);
          hash.add_u64(command.old_id);
          add_new_order(hash, command.replacement);
        }
      },
      request);
}

[[nodiscard]]
inline std::uint64_t command_stream_checksum(
    std::uint64_t seed,
    const std::vector<CommandRecord> &records) noexcept {

  Fnv1a64 hash;

  // Version/format marker.
  hash.add_u64(0x4d454e47494e4531ULL);

  hash.add_u64(seed);
  hash.add_u64(records.size());

  for (const auto &record : records) {
    hash.add_u64(record.request_id);
    hash.add_u64(record.command_sequence);
    add_request(hash, record.request);
  }

  return hash.value();
}

inline void add_book_snapshot(
    Fnv1a64 &hash,
    const OrderBookSnapshot &snapshot) noexcept {

  const auto add_side = [&hash](const auto &levels) {
    hash.add_u64(levels.size());

    for (const auto &level : levels) {
      hash.add_byte(
          static_cast<std::uint8_t>(level.side));

      hash.add_u64(
          static_cast<std::uint64_t>(level.price));

      hash.add_u64(level.total_quantity);
      hash.add_u64(level.orders.size());

      for (const auto &order : level.orders) {
        hash.add_u64(order.id);

        hash.add_byte(
            static_cast<std::uint8_t>(order.side));

        hash.add_u64(
            static_cast<std::uint64_t>(order.price));

        hash.add_u64(order.quantity);
        hash.add_u64(order.filled_quantity);
        hash.add_u64(order.sequence);

        hash.add_byte(
            static_cast<std::uint8_t>(order.status));
      }
    }
  };

  add_side(snapshot.bids);
  add_side(snapshot.asks);
}

[[nodiscard]]
inline std::uint64_t outcome_checksum(
    const std::vector<RequestResult> &results,
    const RequestProcessor &processor) noexcept {

  Fnv1a64 hash;

  // Outcome format/version marker.
  hash.add_u64(0x4f5554434f4d4531ULL);

  hash.add_u64(results.size());

  for (const auto &result : results) {
    hash.add_bool(result.accepted);

    hash.add_byte(
        static_cast<std::uint8_t>(result.error));

    hash.add_u64(result.executions.size());

    for (const auto &execution : result.executions) {
      hash.add_u64(execution.buy_order_id_);
      hash.add_u64(execution.sell_order_id_);

      hash.add_u64(
          static_cast<std::uint64_t>(
              execution.execution_price_));

      hash.add_u64(execution.executed_quantity_);
    }
  }

  hash.add_u64(processor.next_command_sequence());
  hash.add_u64(processor.next_order_sequence());

  add_book_snapshot(
      hash,
      processor.order_book().snapshot());

  return hash.value();
}

struct Recording {
  std::uint64_t seed{0};

  std::vector<CommandRecord> records;

  std::uint64_t input_checksum{0};
  std::uint64_t outcome_checksum{0};
};

class Recorder {
public:
  explicit Recorder(std::uint64_t seed)
      : seed_(seed) {
  }

  [[nodiscard]]
  RequestResult record(
      const CommandRecord &record) {

    RequestResult result =
        processor_.process(
            record.request_id,
            record.command_sequence,
            record.request);

    records_.push_back(record);
    results_.push_back(result);

    return result;
  }

  [[nodiscard]]
  RequestResult record(
      RequestId request_id,
      Sequence command_sequence,
      const ClientRequest &request) {

    return record(
        CommandRecord{
            request_id,
            command_sequence,
            request});
  }

  [[nodiscard]]
  Recording finish() const {
    return Recording{
        seed_,
        records_,
        command_stream_checksum(seed_, records_),
        outcome_checksum(results_, processor_)};
  }

  [[nodiscard]]
  const RequestProcessor &processor() const noexcept {
    return processor_;
  }

  [[nodiscard]]
  const std::vector<RequestResult> &
  results() const noexcept {
    return results_;
  }

private:
  std::uint64_t seed_;

  RequestProcessor processor_;

  std::vector<CommandRecord> records_;
  std::vector<RequestResult> results_;
};

enum class ReplayError {
  None,
  InputChecksumMismatch,
  OutcomeChecksumMismatch
};

struct ReplayReport {
  ReplayError error{ReplayError::None};

  std::uint64_t actual_input_checksum{0};
  std::uint64_t actual_outcome_checksum{0};

  std::vector<RequestResult> results;

  [[nodiscard]]
  bool valid() const noexcept {
    return error == ReplayError::None;
  }
};

class Replayer {
public:
  [[nodiscard]]
  static ReplayReport replay_and_validate(
      const Recording &recording) {

    ReplayReport report;

    // First verify that neither the seed nor recorded commands changed.
    report.actual_input_checksum =
        command_stream_checksum(
            recording.seed,
            recording.records);

    if (report.actual_input_checksum !=
        recording.input_checksum) {

      report.error =
          ReplayError::InputChecksumMismatch;

      return report;
    }

    RequestProcessor replayed;

    report.results.reserve(
        recording.records.size());

    for (const auto &record : recording.records) {
      report.results.push_back(
          replayed.process(
              record.request_id,
              record.command_sequence,
              record.request));
    }

    // Then verify that replay generated exactly the same results,
    // executions, sequence state and final order book.
    report.actual_outcome_checksum =
        outcome_checksum(
            report.results,
            replayed);

    if (report.actual_outcome_checksum !=
        recording.outcome_checksum) {

      report.error =
          ReplayError::OutcomeChecksumMismatch;

      return report;
    }

    return report;
  }
};

} // namespace replay_log

#endif // CPPFIRST_REPLAY_LOG_H