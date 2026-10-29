# Invariants

This document defines the correctness properties that the RV32I interpreter must
preserve across instruction execution and memory operations.

## Register File

### Invariant

- **INV-RF-01:** The register file contains 32 architectural registers, each 32
  bits wide.
- **INV-RF-02:** Reads from `x0` always return zero.
- **INV-RF-03:** Writes to `x0` are discarded.
- **INV-RF-04:** Writes to `x1`–`x31` update only the corresponding register.

### Rationale

These properties preserve the architectural behavior of the RV32I integer register file.

