#!/bin/bash

OUT_S="/tmp/prism_out.S"
OUT_BIN="/tmp/prism_out"

run_one() {
    local input="$1"

    if ! ./build/prism "$input" -o "$OUT_S"; then
        printf "%-32s prism failed to compile\n" "$input:"
        return
    fi

    if ! riscv64-unknown-elf-gcc "$OUT_S" -o "$OUT_BIN" 2>/dev/null; then
        printf "%-32s assembler/linker failed\n" "$input:"
        return
    fi

    spike pk "$OUT_BIN" >/dev/null 2>&1
    printf "%-32s exit=%s\n" "$input:" "$?"
}

if [ $# -ge 1 ]; then
    run_one "$1"
else
    for f in examples/*.c; do
        run_one "$f"
    done
fi

rm -f "$OUT_S" "$OUT_BIN"