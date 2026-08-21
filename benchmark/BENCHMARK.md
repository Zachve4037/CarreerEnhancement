### byte_buffer benchmark:
2026-08-21T16:59:39+02:00\
Running ./build-release/benchmark/byte_buffer_benchmark\
Run on (20 X 3494.4 MHz CPU s)\
CPU Caches:\
L1 Data 48 KiB (x10)\
L1 Instruction 32 KiB (x10)\
L2 Unified 2048 KiB (x10)\
L3 Unified 24576 KiB (x1)\
Load Average: 0.26, 0.18, 0.07

| Benchmark/ByteCount        |Time             |CPU   | Iterations |
|----------------------------|-----------------|------|------------|
| BM_ByteBufferCopy/64       |   12.1 ns      |   12.1 ns   |  59732063|
| BM_ByteBufferCopy/1024     |   15.4 ns      |   15.5 ns   |  44520343|
| BM_ByteBufferCopy/1048576  |  12750 ns      |  12795 ns   |     54919|
| BM_ByteBufferCopy/10485760 | 186967 ns      | 187669 ns   |      3682|
| BM_ByteBufferMove/64       |   1.75 ns      |   1.76 ns   | 397241295|
| BM_ByteBufferMove/1024     |   1.75 ns      |   1.76 ns   | 400050212|
| BM_ByteBufferMove/1048576  |   1.76 ns      |   1.76 ns   | 398305848|
| BM_ByteBufferMove/10485760 |   1.78 ns      |   1.76 ns   | 398494476|