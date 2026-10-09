# Component Diagram

```mermaid
flowchart TB

    subgraph Producers["Event Producers"]
        P1["Producer A"]
        P2["Producer B"]
        P3["Producer C"]
    end

    subgraph Infrastructure["Generic Infrastructure"]
        Q["Bounded Event Queue"]
        EL["Event Log"]
        R["Replay Engine"]
    end

    subgraph Domain["Deterministic Domain Core"]
        ME["Matching Engine"]
        OB["Order Book"]
        O["Order State"]
    end

    subgraph Output["Outputs"]
        OE["Output Events"]
        S["State Snapshot"]
    end

    P1 --> Q
    P2 --> Q
    P3 --> Q

    Q --> ME

    ME --> OB
    OB --> O

    ME --> OE
    ME --> EL

    EL --> R
    R --> ME

    ME --> S
```

## Responsibilities

### Event Producers

Generate input events. Producers may execute concurrently.

### Bounded Event Queue

Provides the concurrency boundary between producers and the deterministic processing core.

The queue transports events but has no knowledge of order-book rules.

### Matching Engine

The single logical owner of domain processing.

It consumes events, updates the order book, and produces output events.

### Order Book

Maintains active orders and implements price-time priority.

### Order State

Represents the lifecycle of individual orders.

### Event Log

Records the ordered input event stream for reproducibility and replay.

### Replay Engine

Feeds previously recorded events back into the matching engine.

### State Snapshot

Represents deterministic engine state at a particular point in processing.

## Dependency rule

The important dependency direction is:

```text
Application
    |
    v
Infrastructure
    |
    v
Matching Engine
    |
    v
Order Book
```

The order book must not depend on the queue, event log, filesystem, or network.

The queue must not depend on order-book concepts.

The replay engine feeds events into the same matching engine used for normal processing.

