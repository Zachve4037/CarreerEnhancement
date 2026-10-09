# Deterministic Event Processing Engine

## 1. Overview

This project implements a deterministic, event-driven processing engine with a limit-order matching engine as its first domain application.

The project is designed as reusable systems software rather than as a production trading system. The architecture separates generic event-processing infrastructure from domain-specific order-book and matching logic.

The main goals are:

* deterministic processing of an ordered event stream;
* explicit module boundaries and ownership of state;
* reproducible execution through event replay;
* support for bounded asynchronous input;
* testable and independently usable components;
* measurable performance and concurrency characteristics.

The order-matching domain is used because it provides useful requirements for ordering, state transitions, event processing, replay, and deterministic behavior.

---

## 2. Goals

### 2.1 Functional goals

The system should:

* accept new orders;
* cancel existing orders;
* maintain an order book;
* apply price-time priority;
* execute trades when compatible orders meet;
* maintain order lifecycle state;
* emit deterministic output events;
* record input events;
* replay an event stream and reproduce the same final state.

### 2.2 Systems goals

The system should also demonstrate:

* clear ownership of mutable state;
* bounded queues and backpressure;
* deterministic single-threaded processing;
* separation between input/concurrency and domain logic;
* unit and integration testing;
* ThreadSanitizer-clean concurrent infrastructure;
* benchmarking and profiling;
* reproducible debugging through event replay.

---

## 3. Non-goals

This project is not intended to be a production-grade financial exchange.

The following are explicitly outside the initial scope:

* real market connectivity;
* network protocols;
* authentication and authorization;
* persistence guarantees required for financial trading;
* distributed consensus;
* high-availability deployment;
* real-money settlement;
* exchange regulation and compliance;
* production-grade fault tolerance.

These features can be considered future extensions but should not complicate the initial architecture.

---

## 4. Architectural principles

### 4.1 Deterministic core

The matching engine processes events sequentially.

Given:

* the same initial state;
* the same ordered input events;

the engine must produce the same:

* trades;
* order states;
* output events;
* final order-book state.

Thread scheduling, wall-clock timing, and memory addresses must not affect the result.

---

### 4.2 Single owner of mutable domain state

The order book and matching state have one logical owner: the matching engine.

External components must not directly modify the order book.

Instead, they submit events to the engine.

This avoids unnecessary locking inside the core matching algorithm and makes the core easier to reason about and test.

---

### 4.3 Generic infrastructure versus domain logic

The architecture separates reusable infrastructure from the order-book domain.

Generic infrastructure includes:

* event representation;
* event dispatch;
* queues;
* event logging;
* replay;
* sequencing;
* serialization.

Domain-specific components include:

* orders;
* order states;
* bids and asks;
* price-time priority;
* matching;
* trade generation.

The generic infrastructure should not depend on trading-specific concepts.

---

### 4.4 Explicit boundaries

Each component owns a specific responsibility and exposes a small interface.

For example:

`OrderBook` owns order-book state.

`MatchingEngine` owns the processing of domain events.

`EventLog` owns event persistence.

`ReplayEngine` owns replaying events.

No component should bypass another component's public interface to manipulate its internal state.

---

## 5. Components

### 5.1 Event model

The event model represents actions and results flowing through the system.

Example input events:

* `NewOrder`
* `CancelOrder`

Example output events:

* `OrderAccepted`
* `OrderRejected`
* `TradeExecuted`
* `OrderPartiallyFilled`
* `OrderFilled`
* `OrderCancelled`

Events should contain all information required to process or reproduce the associated operation.

Events should use explicit sequence numbers rather than relying on wall-clock timestamps for ordering.

---

### 5.2 Event queue

The event queue decouples event producers from the deterministic processing core.

Responsibilities:

* accept events from producers;
* provide events to the matching engine;
* apply bounded capacity;
* provide backpressure when the queue is full;
* provide thread-safe synchronization where required.

The queue does not understand order-book rules.

The queue only transports events.

The existing bounded blocking queue implementation can be reused here.

---

### 5.3 Matching engine

The matching engine is the central domain component.

Responsibilities:

* consume input events;
* validate domain operations;
* update order state;
* invoke order-book operations;
* generate trades;
* generate output events.

The matching engine should expose a deterministic operation conceptually equivalent to:

```cpp
engine.apply(event);
```

The matching engine itself should not depend on:

* network APIs;
* threads;
* operating-system timing;
* file-system details.

This allows it to be tested independently.

---

### 5.4 Order book

The order book maintains currently active orders.

It contains:

* bid orders;
* ask orders;
* price levels;
* order priority;
* lookup by order ID.

The order book implements price-time priority.

For bids:

1. higher price has priority;
2. earlier sequence number wins at the same price.

For asks:

1. lower price has priority;
2. earlier sequence number wins at the same price.

The order book should not own event logging or replay functionality.

---

### 5.5 Order model

An order contains the state necessary to represent an order throughout its lifecycle.

Conceptually:

```cpp
struct Order {
    OrderId id;
    Side side;
    Price price;
    Quantity quantity;
    Quantity filled_quantity;
    Sequence sequence;
    OrderStatus status;
};
```

The lifecycle should allow only valid transitions.

Example:

```text
NEW
 |
 +--> REJECTED
 |
 v
OPEN
 |
 +--> PARTIALLY_FILLED
 |          |
 |          +--> FILLED
 |          |
 |          +--> CANCELLED
 |
 +--> FILLED
 |
 +--> CANCELLED
```

Invalid transitions must be rejected.

---

### 5.6 Event log

The event log records the input event stream.

Its primary purpose is reproducibility and debugging.

Conceptually:

```text
sequence 1 -> NewOrder
sequence 2 -> NewOrder
sequence 3 -> CancelOrder
sequence 4 -> NewOrder
...
```

The event log should preserve event ordering.

The first implementation may use a simple file-based representation.

The storage implementation should remain behind an interface so that another persistence mechanism can be introduced later.

---

### 5.7 Replay engine

The replay engine reconstructs state by applying previously recorded input events.

Conceptually:

```text
Event log
    |
    v
Replay engine
    |
    v
Matching engine
    |
    v
Final state
```

The replay engine should not implement matching rules itself.

It should simply:

1. read events in sequence order;
2. provide them to the matching engine;
3. optionally verify resulting state or output events.

---

### 5.8 Snapshot

A snapshot represents the relevant state of the matching engine at a specific point.

A snapshot may contain:

* active orders;
* order statuses;
* order-book contents;
* next sequence number;
* other deterministic engine state.

Snapshots are not required for the first implementation but provide a future optimization for large event logs.

Instead of:

```text
replay 10,000,000 events
```

the system could eventually:

```text
load snapshot at event 9,000,000
        +
replay events 9,000,001 ... 10,000,000
```

---

## 6. Component boundaries

The intended dependency direction is:

```text
Application
    |
    v
Event infrastructure
    |
    v
Matching engine
    |
    v
Order book / order domain
```

The application layer is responsible for assembling the components.

The event queue must not depend on the order book.

The replay system must depend on the event representation but not on the internal implementation of the order book.

The matching engine may depend on the order domain but should not depend on infrastructure such as files or network sockets.

---

## 7. Concurrency model

The core matching engine is intentionally single-threaded.

Multiple producers may generate events concurrently:

```text
Producer A ---\
Producer B ----+--> bounded queue --> Matching Engine
Producer C ---/
```

Only the matching-engine thread mutates the order book.

This provides a simple ownership model:

> Many producers, one queue, one deterministic state owner.

The queue is responsible for synchronization.

The matching engine is responsible for deterministic domain processing.

The order book does not need internal locks in the initial design.

---

## 8. Backpressure

The input queue is bounded.

If producers generate events faster than the matching engine can process them, the queue eventually reaches capacity.

The system must then apply an explicit backpressure policy.

The initial policy is blocking producers until space becomes available.

This prevents unbounded memory growth.

Future policies may include:

* rejection;
* dropping low-priority work;
* rate limiting;
* batching.

The policy should remain outside the matching algorithm.

---

## 9. Determinism

The following properties must hold:

```text
same initial state
+
same ordered events
=
same output
+
same final state
```

The implementation must avoid using nondeterministic properties as domain ordering rules.

In particular, order priority must not depend on:

* thread scheduling;
* hash-table iteration order;
* memory addresses;
* wall-clock timing;
* random values.

Sequence numbers provide an explicit ordering mechanism.

---

## 10. Error handling

Errors should be classified according to where they occur.

### Domain errors

Examples:

* invalid price;
* invalid quantity;
* unknown order ID;
* invalid lifecycle transition;
* invalid cancellation.

These should produce explicit rejection results/events.

### Infrastructure errors

Examples:

* event-log I/O failure;
* queue shutdown;
* serialization failure.

These should be propagated to the appropriate infrastructure or application layer.

The matching algorithm should not directly handle operating-system-specific errors unless necessary.

---

## 11. Testing strategy

Testing should be performed at several levels.

### Unit tests

Test individual components:

* order lifecycle;
* price-time priority;
* order insertion;
* cancellation;
* matching;
* partial fills.

### Integration tests

Test:

```text
events -> matching engine -> order book -> output events
```

### Replay tests

Verify:

```text
events
  -> engine
  -> snapshot A

same events
  -> engine
  -> snapshot B

A == B
```

### Concurrency tests

Test:

* multiple producers;
* queue full/empty behavior;
* shutdown;
* producer/consumer synchronization.

ThreadSanitizer should be used on the concurrent infrastructure.

### Performance tests

Measure:

* events/sec;
* matches/sec;
* end-to-end latency;
* queue throughput;
* CPU hotspots.

Correctness and performance tests should remain separate.

---

## 12. Performance considerations

The first implementation should prioritize correctness and deterministic behavior.

Potential optimization areas include:

* order-book data structures;
* price-level lookup;
* order-ID lookup;
* memory allocation;
* event serialization;
* queue contention.

Optimizations should be supported by measurements rather than assumptions.

The project should retain benchmark results and profiling output so that performance changes can be compared objectively.

---

## 13. Initial public API

The exact API may evolve, but the conceptual interface should remain small.

Example:

```cpp
class MatchingEngine {
public:
    void apply(const Event& event);

    Snapshot snapshot() const;
};
```

The event log may expose:

```cpp
class EventLog {
public:
    void append(const Event& event);

    std::vector<Event> read_all() const;
};
```

Replay may expose:

```cpp
class ReplayEngine {
public:
    void replay(
        const std::vector<Event>& events,
        MatchingEngine& engine);
};
```

The concrete implementation details should remain private.

---

## 14. Project structure

A possible initial structure is:

```text
matching-engine/
├── CMakeLists.txt
├── README.md
├── docs/
│   ├── architecture.md
│   ├── component-diagram.md
│   └── message-flow.md
│
├── src/
│   ├── core/
│   │   ├── event.h
│   │   └── sequence.h
│   │
│   ├── queue/
│   │   └── bounded_queue.h
│   │
│   ├── orderbook/
│   │   ├── order.h
│   │   ├── order_book.h
│   │   └── matching_engine.h
│   │
│   ├── replay/
│   │   ├── event_log.h
│   │   └── replay_engine.h
│   │
│   └── app/
│       └── main.cpp
│
└── test/
    ├── order_test.cpp
    ├── order_book_test.cpp
    ├── matching_engine_test.cpp
    └── replay_test.cpp
```

The structure is only a starting point. Dependency boundaries are more important than directory names.

---

## 15. Future extensions

Potential future extensions include:

* persistent snapshots;
* alternative queue implementations;
* asynchronous event-log writers;
* network input;
* multiple matching-engine instances;
* metrics;
* structured logging;
* configurable matching policies;
* alternative priority policies;
* recovery after process restart.

These should be added only when they provide a useful engineering learning objective.

---

## 16. Design summary

The central architectural decision is:

> Keep the domain-processing core deterministic and single-owner, while moving concurrency and infrastructure to the boundaries.

The resulting system is:

```text
Concurrent input
      |
      v
Bounded queue
      |
      v
Deterministic matching engine
      |
      +----> Order book
      |
      +----> Output events
      |
      v
Event log / replay
```

The order-book domain provides a concrete workload for demonstrating general systems concepts:

* event-driven processing;
* state machines;
* priority queues;
* bounded concurrency;
* deterministic execution;
* replay;
* modular design;
* testing;
* benchmarking.
