# Binary command/event protocol v1

The text demo parses and validates `NEW` and `CANCEL`; it does not execute them.
`RequestProcessor` executes new, cancel, and replace requests against a price/FIFO
order book. It caches results by request ID, enforces command sequence order,
and records in-order requests, including business rejections. Replay reconstructs
the book from these records and checks deterministic outcomes.

The binary codec uses these existing `CommandRecord` and `RequestResult` types.
It is a library boundary for callers, independent of the text demo and transport.

## Representation

All multibyte integers are little-endian. IDs, sequences, and quantities are
unsigned 64-bit integers. Prices are signed 64-bit two's-complement integers in
the engine's existing price units. There is no native structure padding.

Every frame starts with this six-byte header:

| Offset | Width | Field |
| --- | --- | --- |
| 0 | 1 | Version: `1` |
| 1 | 1 | Message type |
| 2 | 4 | Payload length, excluding header |

Each decoder accepts exactly one complete frame. Callers must buffer partial
input and split concatenated frames using the header length. Unknown versions,
types, invalid enum values, trailing bytes, and inconsistent lengths are errors.
Decoders validate lengths before reading fields or allocating execution storage.
Transport callers should impose their own frame-size limit before buffering.
This format has no checksum, authentication, or persistent-file container.

## Commands

All command payloads start with `request_id:u64, command_sequence:u64`.
Sequence assignment belongs to the caller; the processor expects sequence 1
initially. A retry must retain both its original ID and sequence.

| Type | Command | Remaining payload fields | Payload bytes | Total bytes |
| --- | --- | --- | --- | --- |
| 1 | New | `id:u64, side:u8, price:i64, quantity:u64` | 41 | 47 |
| 2 | Cancel | `id:u64` | 24 | 30 |
| 3 | Replace | `old_id:u64, id:u64, side:u8, price:i64, quantity:u64` | 49 | 55 |

Side codes are `0=Buy`, `1=Sell`. Replacement fields describe the new order.
Zero IDs, zero sequences, nonpositive prices, and zero quantities remain
representable: business validation and sequencing belong to `RequestProcessor`,
which produces the same rejections as direct calls. Unknown side codes are
malformed protocol values and do not reach the processor.

## Result events

Type `4` contains one whole request result, including all fills in matching order:

`request_id:u64, command_sequence:u64, accepted:u8, error:u8, executions...`

Each execution is `buy_order_id:u64, sell_order_id:u64, price:i64, quantity:u64`
(32 bytes). The count is `(payload_length - 18) / 32`; the payload must be at least
18 bytes with an integral number of executions. An event without fills is 24
bytes including the header. `accepted` is exactly 0 or 1; acceptance requires
error 0, rejection requires a nonzero error and no executions. Retries return
the cached fills; consumers must deduplicate by request identity when necessary.
The event sequence identifies the submitted command, not a separate fill sequence.

Stable error codes:

| Code | RequestError |
| --- | --- |
| 0 | None |
| 1 | InvalidRequestId |
| 2 | DuplicateRequestIdWithDifferentPayload |
| 3 | OutOfOrderCommand |
| 4 | InvalidOrderId |
| 5 | InvalidSide |
| 6 | InvalidPrice |
| 7 | InvalidQuantity |
| 8 | DuplicateOrderId |
| 9 | UnknownOrder |
| 10 | ReplacementFailed |

Encoding rejects unknown sides, unknown error codes, inconsistent results, and
event payloads exceeding the 32-bit length field. Existing codes and layouts
must retain their meaning; incompatible changes require a new protocol version.

## Calling the codec

Link `matching_engine_protocol` and include `binary_protocol.h`.
`encode_command` and `encode_event` return `variant<Bytes, ProtocolError>`;
`decode_command` and `decode_event` return their decoded value or `ProtocolError`.
All byte storage is owned by the returned vector; decode accepts a borrowed span.
Malformed frames never modify engine state. After successful command decoding:

```cpp
const auto result = processor.process(
    command.request_id, command.command_sequence, command.request);
const auto output = binary_protocol::encode_event(
    {command.request_id, command.command_sequence, result});
```

Check each variant before using it. Protocol errors are separate from business
rejections. Standard allocation failures propagate as exceptions.
