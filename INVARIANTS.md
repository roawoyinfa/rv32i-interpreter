# Invariants

This document defines the correctness properties that the RV32I interpreter must
preserve across instruction execution and memory operations.

## Register File

### Invariants

- **INV-RF-01:** The register file contains exactly 32 architectural registers,
  each 32 bits wide.
- **INV-RF-02:** Reads from `x0` always return zero.
- **INV-RF-03:** Writes to `x0` are ignored.
- **INV-RF-04:** Writes to `x1`–`x31` modify only the corresponding register.

### Rationale

These invariants the architectural semantics of the RV32I integer register file.

## Memory

### Invariants

- **INV-M-01:** Memory is byte-addressable.
- **INV-M-02:** Multi-byte values are stored and accessed using the
  little-endian byte order.
- **INV-M-03:** All memory accesses must fall entirely within the bounds
  of the simulated memory.

### Rationale

INV-M-01 and INV-M-02 preserve the memory semantics of the RISC-V RV32I
architecture, while INV-M-03 prevents undefined behavior in the host language.

