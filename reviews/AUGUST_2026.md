# August review
## What I built
| Stage |             What it demonstrates |
|----------|:-------------------------------:|
| C++ & CMake | Build-system fundamentals|
| GoogleTests + CI        |      Automated verification |
| RAII wrappers        |      Resource/lifetime management|
|  Copy/move       |      Modern C++ ownership semantics |
|  Benchmarks       |      Measuring performance rather than guessing |
|  Linux APIs       |      Systems programming |
|  Process supervisor       |      Processes, signals, timeouts, failure handling|
| strace / proc        |      Practical Linux debugging|

## What I learned and improved
Despite I had previous experience with C++, I definitely made big progress 
in learning basics and deepened my knowledge in this particular language.
Same thing goes for testing, I wrote some Unit tests or similar 
tests, but never Google-Tests. Also, I wrote my second CI/CD pipeline
in my life. In terms of systems programming, I also had done something
with threads, mutexes and other things, but I also learned much more. 
The biggest "black holes" for me were the memory optimizations, 
strace / proc, process supervisor and TCP server. Once again, I had
some base knowledge, but that was definitely just the tip of the iceberg.

## Short project narrative
Building a small C++ systems project from scratch and progressively
making it production-like:
* reporoducible builds
* automated tests/CI
* RAII/resources management
* Linux system programming
* performance profiling and failure analysis

The goal is to have good enough experiences in system engineering.