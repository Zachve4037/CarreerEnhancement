#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
DEMO="${DEMO:-$BUILD_DIR/src/process_supervisor_demo}"

if [[ ! -x "$DEMO" ]]; then
    echo "Demo executable not found: $DEMO" >&2
    echo "Build it first with: cmake -S . -B $BUILD_DIR && cmake --build $BUILD_DIR --target process_supervisor_demo" >&2
    exit 1
fi

if ! command -v strace >/dev/null 2>&1; then
    echo "strace is not installed. On Debian/Ubuntu: sudo apt install strace" >&2
    exit 1
fi

OUT_DIR="${OUT_DIR:-docs/strace-output}"
mkdir -p "$OUT_DIR"

run_trace() {
    local name="$1"
    shift
    echo "[profile] $name"
    strace -f -tt -T -o "$OUT_DIR/$name.trace" "$DEMO" "$@" >"$OUT_DIR/$name.stdout" 2>"$OUT_DIR/$name.stderr" || true
}

run_trace "success" sh -c "printf 'hello from child\\n'; printf 'child warning\\n' >&2; exit 0"
run_trace "nonzero_exit" sh -c "printf 'child failed\\n' >&2; exit 42"
run_trace "exec_failure" definitely-not-a-real-program-xyz
run_trace "signal" sh -c "kill -TERM \$\$"

echo
printf '%s\n' "Created traces in $OUT_DIR/"
printf '%s\n' "Use: strace -c -f $DEMO sh -c 'printf hello'"
printf '%s\n' "Use: strace -f -tt -T -e trace=process,read,write,close,dup2,execve,wait4,pipe,pipe2 $DEMO sh -c 'printf hello'"
