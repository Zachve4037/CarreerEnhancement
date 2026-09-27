# Queue Performance & Concurrency Report

## Environment

* CPU: Intel Core i5 14600k
* OS / kernel: Windows 11 professional
* Compiler and version: cmake 20
* Build type: Release
* Queue capacity: 1024
* Total items per run: 1,000,000
* Number of measured runs: 5 (plus warm-up)

## Results

| Queue                | Producers | Consumers | Throughput (items/s) | Median time (s) |
| -------------------- | --------: | --------: | -------------------: | --------------: |
| BoundedBlockingQueue |         1 |         1 |              13092604|         0.076379|
| SPSCQueue            |         1 |         1 |               6385121|         0.156614|
| BoundedBlockingQueue |         2 |         1 |               4980028|         0.200802|
| BoundedBlockingQueue |         1 |         2 |               4492177|         0.222609|
| BoundedBlockingQueue |         4 |         4 |               3051836|         0.327672|

## Correctness

* Unit tests: Implemented tests for zero capacity, full /empty queue, FIFO push and pop,
* Concurrent stress test: Implemented a producer-consumer test transferring 100,000 sequential integers trhough the SPSC
queue, verifying FIFO order and detecting missing or reordered value.