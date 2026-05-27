# Prism

A hand-written C compiler targeting RISC-V, built from scratch without parser generators or LLVM. The name comes from how it splits one language into multiple target backends, just like light through a prism.

Currently supports the minimal C subset (N = integer literal):
```c
int main(void) {
    return N;
}
```

## Quick start

Prerequisites:
- `clang` (C23 support)
- `riscv64-unknown-elf-gcc` (for assembling/linking RISC-V output)
- `spike` + `pk` (for running RISC-V binaries)

On macOS:

```sh
brew install llvm
brew tap riscv-software-src/riscv
brew install riscv-tools
```

Build and run:

```sh
make
./run.sh examples/ret2.c
# → exit=2
./run.sh -v examples/ret2.c # verbose: print AST
# → Program
# → └─ Function name="main"
# →    └─ Return
# →       └─ IntLit value=2
# → exit=2
```

## Pipeline

Prism takes a C source file and emits RISC-V assembly:

C source → lex → parse → AST → codegen → RISC-V (.S)

For example, `examples/ret2.c`:

```c
int main(void) {
    return 2;
}
```

compiles to:

```asm
    .text
    .globl main
main:
    li a0, 2
    ret
```

which, when assembled and run on spike, exits with code 2.

## Usage

```sh
./build/prism <input.c> -o <output.S>
./build/prism -v <input.c> -o <output.S>   # verbose: print AST
```

## Build

```sh
make              # build prism
make clean        # remove build artifacts
make sanitizer    # build with AddressSanitizer + UBSan
```

## Project structure

```
src/
  lexer.{c,h}    — tokenizer
  parser.{c,h}   — recursive descent parser
  ast.{c,h}      — AST node definitions, print, free
  codegen.{c,h}  — RISC-V assembly emitter
  main.c         — driver
examples/        — sample C programs
```

## Milestones

- 2026-05-28 — End-to-end pipeline works: `int main(void) { return N; }` compiles, assembles, runs on spike, returns N as exit code.
