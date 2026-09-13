# Process supervisor: strace profile and failure handling

## Goal

Profile the Linux process supervisor with `strace` and document what happens on
both the normal and failure paths.

The supervisor creates a pipe, forks a child, redirects the child's stdout and
stderr into the pipe, executes the requested program with `execvp`, captures the
output with `read`, and finally reaps the child with `waitpid`.

## Build

Run this on Linux/WSL:

```bash
cmake -S . -B build
cmake --build build --target process_supervisor_demo
```

If `strace` is missing on Debian/Ubuntu:

```bash
sudo apt update
sudo apt install strace
```

## Basic profile

Run:

```bash
strace -f -tt -T ./build/src/process_supervisor_demo sh -c 'printf "hello\\n"; printf "warning\\n" >&2'
```

Important options:

- `-f`: follow the child created by `fork`.
- `-tt`: print timestamps with microsecond resolution.
- `-T`: print how long each system call took.

For a compact syscall summary:

```bash
strace -c -f ./build/src/process_supervisor_demo sh -c 'printf "hello\\n"'
```

For a focused trace of the supervisor lifecycle:

```bash
strace -f -tt -T \
  -e trace=process,read,write,close,dup2,execve,wait4,pipe,pipe2 \
  ./build/src/process_supervisor_demo \
  sh -c 'printf "hello\\n"'
```

## What to look for

The exact syscall names and details depend on the Linux version and libc, but
the important sequence is approximately:

```text
pipe/pipe2       create the communication channel
fork/clone       create the child
close            parent/child close unused pipe ends
dup2             child redirects stdout/stderr to the pipe
execve           child becomes the requested program
write            child writes output
read             parent receives captured output
exit_group       child terminates
wait4            parent reaps the child
```

`strace` is showing the kernel boundary: these are not C++ function calls but
Linux system calls (or libc operations that ultimately make system calls).

## Failure handling

| Failure | Where detected | Current behaviour |
|---|---|---|
| Empty command | Parent, before `pipe()` | Throws `std::invalid_argument` |
| `pipe()` failure | Parent | Closes nothing because no pipe was created; throws `std::system_error` |
| `fork()` failure | Parent | Closes both pipe descriptors and throws `std::system_error` |
| `dup2()` failure | Child | Calls `_exit(127)` |
| `execvp()` failure | Child | Writes a diagnostic to captured stderr and exits with `127` |
| `read()` interrupted by signal | Parent | Retries when `errno == EINTR` |
| Other `read()` failure | Parent | Reaps child and throws `std::system_error` |
| `waitpid()` interrupted by signal | Parent | Retries when `errno == EINTR` |
| Child exits non-zero | Parent | Returns `ProcessStatus::exited` with the child's exit code |
| Child killed by signal | Parent | Returns `ProcessStatus::signaled` with the signal number |

### Why exit code 127 is used for failed exec

The child must report that `execvp()` failed without accidentally continuing
through the supervisor code. `_exit(127)` is used after the failure. The parent
then recognises the `execvp failed:` diagnostic together with exit code 127 and
returns `ProcessStatus::failed_to_start`.

This is deliberately a small teaching implementation. A production supervisor
would normally use a dedicated error-reporting pipe so that an ordinary child
program that legitimately exits 127 cannot be confused with an `execvp`
failure.

## Reproducible failure profiles

The repository contains:

```bash
./scripts/profile_process_supervisor.sh
```

It runs four cases:

1. successful child with stdout and stderr;
2. child that exits with status 42;
3. nonexistent executable (`execvp` failure);
4. child terminated by `SIGTERM`.

The script stores raw `strace` output under `docs/strace-output/` when you run
it locally. These traces are intentionally generated locally rather than
committed, because syscall traces depend on the host kernel, libc, paths and
other environment details.

## Example observations

### Successful execution

The trace should show the supervisor creating the pipe and child, the child
redirecting file descriptors and calling `execve`, followed by writes from the
executed program and reads by the parent. The parent eventually observes EOF
and calls `wait4` to reap the child.

### Failed executable

The child attempts several `execve` calls when `execvp` searches `PATH`. The
attempts can end with `ENOENT`. The child then writes the diagnostic message to
stderr (which is the pipe) and exits with status 127. The parent captures the
message and classifies the result as `failed_to_start`.

### Signal termination

The executed child can terminate itself with `SIGTERM`. The parent does not
see this as a normal exit. `waitpid` reports a signaled child, and the
supervisor records `ProcessStatus::signaled` and the signal number.

## What this profile tells us

`strace` confirms the supervisor's process lifecycle rather than merely
confirming its C++ return values. In particular, it gives evidence that:

- the supervisor really creates a separate child process;
- stdout/stderr are redirected through file descriptors;
- `execvp` replaces the child process image;
- output crosses the pipe using `write`/`read`;
- the parent waits for and reaps the child;
- failure paths are visible as failed syscalls and error returns.

## Limitations / next step

The current supervisor reads from the pipe synchronously. A future version
with timeouts must avoid waiting forever on a child that stops producing output,
and it must also avoid pipe-buffer deadlocks when a child produces more output
than the pipe can hold. That is where non-blocking I/O plus `poll`/`select` or
`epoll`, together with `waitpid(..., WNOHANG)`, becomes relevant.
