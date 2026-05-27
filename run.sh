#!/bin/bash
set -e

VERBOSE=""
while getopts "v" opt; do
    case $opt in
        v) VERBOSE="-v" ;;
        *) echo "Usage: $0 [-v] [input.c]"; exit 1 ;;
    esac
done
shift $((OPTIND - 1))

INPUT="${1:-examples/ret2.c}"
OUT_S="/tmp/prism_out.S"
OUT_BIN="/tmp/prism_out"

./build/prism $VERBOSE "$INPUT" -o "$OUT_S"
riscv64-unknown-elf-gcc "$OUT_S" -o "$OUT_BIN"

set +e
spike pk "$OUT_BIN"
echo "exit=$?"
