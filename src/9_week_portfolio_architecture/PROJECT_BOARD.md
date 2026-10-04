# Deterministic Limit Order Matching Engine

### MILESTONE 1 — Domain Model
* Order representation
* Order IDs and sequence numbers
* Order lifecycle
* Lifecycle tests

### MILESTONE 2 — Price-Time Order Book
* Bid-side priority
* Ask-side priority
* Same-price FIFO
* Insert/remove
* Cancellation
* Order-book tests

### MILESTONE 3 — Matching Engine
* Incoming buy matching
* Incoming sell matching
* Partial fills
* Full fills
* Multi-level matching
* Trade generation
* Matching tests

### MILESTONE 4 — Event Architecture
* Event types
* Event dispatcher
* Input events
* Output events
* Event serialization

### MILESTONE 5 — Deterministic Replay
* Event log
* Event loader
* Replay command
* State snapshot
* Replay determinism tests

### MILESTONE 6 — Concurrent Ingestion
* Producer
* Bounded queue
* Single-owner matching engine
* Thread-safety tests
* ThreadSanitizer

### MILESTONE 7 — Performance & Documentation
* Throughput benchmark
* Latency measurement
* CPU profiling
* Hotspot analysis
* Architecture documentation
* Design decisions
* Final README

## ARCHITECTURE
                    +----------------+
                    | Event Producers|
                    +-------+--------+
                            |
                            v
                   +------------------+
                   | Bounded Event    |
                   | Queue            |
                   +--------+---------+
                            |
                            v
                 +---------------------+
                 | Matching Engine     |
                 |                     |
                 | deterministic       |
                 | single owner        |
                 +----------+----------+
                            |
              +-------------+-------------+
              |                           |
              v                           v
       +-------------+             +-------------+
       | Order Book  |             | Trade/Event |
       |             |             | Output      |
       +-------------+             +-------------+
              |
              v
       Price-Time Priority

And alongside it: 
* Event Log
* Replay
* Matching Engine
* Same deterministic state