# Networking and protocol framing

### Requirement and Implementation
Message framing -> 4-byte big-endian lenght prefix
Partial I/O -> send_all() and receive_all()
Maximum frame size -> 1 MiB
TCP server -> Sequential client handling
Client reconnect -> Limited retries with 100ms delay
Timeouts -> Socket send and receive timeouts
Tests -> Frame tests and loopback echo integration test

### Limitations
* single-client server
* fixed reconnect delay
* no strict overall operation deadline
* no concurrent-client handling