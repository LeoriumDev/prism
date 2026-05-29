#!/usr/bin/env bash

PRISM=./build/prism
CC=riscv64-unknown-elf-gcc
RUN="spike pk"

TMP_S=$(mktemp /tmp/prism_test.XXXXXX.S)
TMP_BIN=$(mktemp /tmp/prism_test.XXXXXX)

GREEN='\033[32m'
RED='\033[31m'
RESET='\033[0m'

passed=0
failed=0

extract_expectation() {
    local file="$1"
    local key="$2"
    grep "// ${key}:" "$file" | head -1 | sed "s|.*${key}:[[:space:]]*||"
}

run_good() {
    local file="$1"
    local expected
    expected=$(extract_expectation "$file" "expect-exit")

    if ! "$PRISM" "$file" -o "$TMP_S" 2>/dev/null; then
        printf "${RED}FAIL${RESET} %-30s (prism failed to compile)\n" "$file"
        failed=$((failed + 1))
        return
    fi

    if ! "$CC" "$TMP_S" -o "$TMP_BIN" 2>/dev/null; then
        printf "${RED}FAIL${RESET} %-30s (assembler/linker failed)\n" "$file"
        failed=$((failed + 1))
        return
    fi

    $RUN "$TMP_BIN" >/dev/null 2>&1
    local actual=$?

    if [ "$actual" = "$expected" ]; then
        printf "${GREEN}PASS${RESET} %-30s (exit %s)\n" "$file" "$actual"
        passed=$((passed + 1))
    else
        printf "${RED}FAIL${RESET} %-30s (expected exit %s, got %s)\n" "$file" "$expected" "$actual"
        failed=$((failed + 1))
    fi
}

run_bad() {
    local file="$1"
    local expected
    expected=$(extract_expectation "$file" "expect-error")

    local output
    output=$("$PRISM" "$file" -o "$TMP_S" 2>&1)
    local rc=$?

    if [ "$rc" -ne 0 ] && printf '%s' "$output" | grep -q -- "$expected"; then
        printf "${GREEN}PASS${RESET} %-30s (errored as expected)\n" "$file"
        passed=$((passed + 1))
    else
        printf "${RED}FAIL${RESET} %-30s (rc=%s, expected error: %s)\n" "$file" "$rc" "$expected"
        failed=$((failed + 1))
    fi
}

echo "Running good tests..."
for file in tests/good/*.c; do
    run_good "$file"
done

echo
echo "Running bad tests..."
for file in tests/bad/*.c; do
    run_bad "$file"
done

rm -f "$TMP_S" "$TMP_BIN"

echo
if [ "$failed" -eq 0 ]; then
    printf "${GREEN}%d passed, %d failed${RESET}\n" "$passed" "$failed"
    exit 0
else
    printf "${RED}%d passed, %d failed${RESET}\n" "$passed" "$failed"
    exit 1
fi
