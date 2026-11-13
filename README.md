# RV32I Interpreter

A minimal C++23 interpreter for a defined subset of the RISC-V RV32I instruction
set. The project implements instruction decoding, execution, register state, and
byte-addressable memory to demonstrate how a processor executes raw machine-code
instructions.

## Overview

This project explores the fundamental mechanisms behind instruction-set
execution by implementing a small RISC-V interpreter in software.

Rather than building a hardware processor or implementing the entire RISC-V ISA,
the project focuses on a deliberately limited instruction subset. The goal is to
establish a correct, testable execution model.

The primary goals of the project are to:

- Understand RISC-V instruction formats and machine-code encodings.
- Implement instruction decoding independently from instruction execution.
- Model the architectural state of an RV32I processor.
- Implement 32-bit arithmetic, logical operations, memory access, and control flow.
- Apply correct sign extension, signed comparisons, and PC-relative addressing.
- Model byte-addressable, little-endian memory.
- Validate instruction semantics using automated tests and small machine-code programs.
- Trace instruction execution to make register, memory, and control-flow changes observable.
- Document architectural decisions and implementation constraints.

## Instruction Subset

The interpreter supports the following 21 instruction mnemonics.

| Category   | Instructions               | Description                                           |
|------------|----------------------------|-------------------------------------------------------|
| Arithmetic | `add`, `sub`, `addi`       | Integer addition and subtraction                      |
| Logical    | `and`, `or`, `xor`         | Bitwise logical operations                            |
| Comparison | `slt`                      | Signed less-than comparison                           |
| Shifts     | `sll`, `srl`, `sra`        | Logical and arithmetic shifts                         |
| Immediate  | `lui`                      | Load upper immediate                                  |
| Memory     | `lw`, `sw`, `lb`, `sb`     | Word and byte loads and stores                        |
| Branches   | `beq`, `bne`, `blt`, `bge` | Conditional control flow                              |
| Jumps      | `jal`, `jalr`              | Unconditional control flow with link-register updates |

## Architecture

The interpreter is organized around seven primary responsibilities:

| Component    | Responsibility                                                         |
| ------------ | ---------------------------------------------------------------------- |
| CPU          | Coordinates the fetch-decode-execute cycle and manages execution state |
| Decoder      | Converts raw instruction words into structured representations         |
| Executor     | Dispatches decoded instructions and implements their semantics         |
| RegisterFile | Manages architectural registers                                        |
| Memory       | Provides checked access to byte-addressable memory                     |
| Loader       | Parses the program file and supplies machine-code words                |
| `main()`     | Initializes the application and handles user-facing errors             |

## Input Format

The interpreter loads raw 32-bit machine-code words rather than assembly-
language instructions. The decoder extracts the instruction fields and
produces a structured representation that the executor can consume.

## Testing and Validation

This project uses integration tests to validate instruction execution and
overall program behaviour. Each test loads and executes a machine-code
program, then checks the resulting CPU state.

The integration tests cover:

| Test       | Purpose                                                                 |
|------------|-------------------------------------------------------------------------|
| Arithmetic | Validate integer arithmetic and register updates.                       |
| Shifts     | Validate logical and arithmetic shift operations.                       |
| Memory     | Validate load/store operations and memory access behaviour.             |
| Branches   | Validate conditional branching and control flow.                        |
| Jumps      | Validate `jal`, `jalr`, and return-address calculation.                 |
| Loop sum   | Validate repeated execution, loop termination, and accumulated results. |

The tests verify expected architectural state, including register values and
memory contents where applicable.

## Requirements

- GCC with C++23 support.
- GNU Make.

## Build, Run, and Test

### Build

Compile the project using GNU Make:

```bash
make
```

### Run

Execute the interpreter with a machine-code program:

```bash
./build/rv32i-interpreter path/to/program.hex
```

Replace the path with the location of your `.hex` program.

### Test

Build and run the integration tests:

```bash
make test
```

The integration tests execute predefined machine-code programs and verify the
resulting CPU state, including register values and memory contents where
applicable.

## Design Constraints

The project deliberately excludes functionality beyond the specified instruction
subset. It does not attempt to implement:

- The complete RISC-V ISA.
- Floating-point instructions.
- Privileged architecture features.
- Operating-system services.
- Pipelining or hazard handling.
- Cache hierarchies.
- Hardware description or FPGA synthesis.
- A graphical debugging interface.

The interpreter models instruction behavior in software; it is not a hardware
implementation of a RISC-V processor.

## Documentation

| Document                           | Purpose                                                    |
|------------------------------------|------------------------------------------------------------|
| [`INVARIANTS.md`](./docs/INVARIANTS.md) | Correctness properties that the interpreter must preserve. |

## License

Licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.

