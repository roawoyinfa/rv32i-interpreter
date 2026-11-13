# Architecture Notes

## Introduction

This project contains a functional interpreter for a defined subset of RV32I.
The primary spine for this project is *Patterson &amp; Hennessy, Computer
Organization and Design: RISC-V Edition (2nd ed.)*, Chapter 2 (Instructions).

The interpreter loads raw 32-bit machine-code words, decodes them, and executes
them against a model of the architectural state. It is deliberately limited to a
defined instruction subset rather than implementing the complete RV32I
specification.

This document records the interpreter's scope, instruction encodings, machine
model, implementation decisions, and the architectural semantics that require
particular care. It also provides evidence of correctness through manual
instruction decoding and executable tests.

The document is organised around four concerns: what the interpreter supports
(sections 1–2), how it is structured (section 3), the decisions and
architectural details governing its behaviour (sections 4–5), and the evidence
used to verify it (sections 6–7). Manual analysis precedes tool-based
verification.

## 1. Scope and ISA Subset

### 1.1 Scope statement

The interpreter executes 21 selected RV32I instructions, operating on a program
counter, a 32-entry general-purpose register file, and byte-addressable memory.
Instructions are fetched as raw 32-bit machine-code words, decoded, and executed
sequentially until a defined halt condition is reached.

The model is functional rather than cycle-accurate: it describes the
architectural effects of instructions without modelling pipelines, caches,
execution timing, privilege levels, or a complete exception-handling system.

No assembler or linker is included. Test programs are assembled externally or
encoded manually before being loaded into the interpreter. Behaviour outside the
supported subset is governed by the scope limitations and design decisions
recorded in sections 1.3 and 4.

### 1.2 Subset table

| Instruction | Format | Opcode | funct3 | funct7 | Semantics                                             |
| ----------- | ------ | ------ | ------ | ------ | ------------------------------------------------------|
| ADD         | R      | 0x33   | 0x0    | 0x00   | rd = rs1 + rs2                                        |
| SUB         | R      | 0x33   | 0x0    | 0x20   | rd = rs1 - rs2                                        |
| ADDI        | I      | 0x13   | 0x0    | —      | rd = rs1 + sign_extend(imm)                           |
| AND         | R      | 0x33   | 0x7    | 0x00   | rd = rs1 & rs2                                        |
| OR          | R      | 0x33   | 0x6    | 0x00   | rd = rs1 \| rs2                                       |
| XOR         | R      | 0x33   | 0x4    | 0x00   | rd = rs1 ^ rs2                                        |
| SLT         | R      | 0x33   | 0x2    | 0x00   | rd = signed(rs1) < signed(rs2) ? 1 : 0                |
| SLL         | R      | 0x33   | 0x1    | 0x00   | rd = rs1 << rs2[4:0]                                  |
| SRL         | R      | 0x33   | 0x5    | 0x00   | rd = rs1 >> rs2[4:0] (logical)                        |
| SRA         | R      | 0x33   | 0x5    | 0x20   | rd = rs1 >> rs2[4:0] (arithmetic)                     |
| LUI         | U      | 0x37   | —      | —      | rd = imm[31:12] << 12                                 |
| LW          | I      | 0x03   | 0x2    | —      | rd = Memory[rs1 + sign_extend(imm)]                   |
| SW          | S      | 0x23   | 0x2    | —      | Memory[rs1 + sign_extend(imm)] = rs2                  |
| LB          | I      | 0x03   | 0x0    | —      | rd = sign_extend(Memory[rs1 + sign_extend(imm)][7:0]) |
| SB          | S      | 0x23   | 0x0    | —      | Memory[rs1 + sign_extend(imm)] = rs2[7:0]             |
| BEQ         | B      | 0x63   | 0x0    | —      | if (rs1 == rs2) PC += sign_extend(imm)                |
| BNE         | B      | 0x63   | 0x1    | —      | if (rs1 != rs2) PC += sign_extend(imm)                |
| BLT         | B      | 0x63   | 0x4    | —      | if (rs1 < rs2) PC += sign_extend(imm)                 |
| BGE         | B      | 0x63   | 0x5    | —      | if (rs1 >= rs2) PC += sign_extend(imm)                |
| JAL         | J      | 0x6F   | —      | —      | rd = PC + 4; PC += sign_extend(imm)                   |
| JALR        | I      | 0x67   | 0x0    | —      | rd = PC + 4; PC = (rs1 + sign_extend(imm)) & ~1       |

### 1.3 Explicitly out of scope

The interpreter does not implement the following:

- M extension: multiplication and division instructions are excluded.
- Unsigned loads: `lbu` and `lhu` are not supported.
- Unsigned comparisons: `sltu`, `bltu`, and `bgeu` are excluded.
- PC-relative upper immediate: `auipc` is not implemented.
- Additional integer instructions: instructions outside the 21-instruction
  subset are unsupported.
- System instructions: `ecall` is recognised as a project-specific halt
  instruction. Its conventional RISC-V environment-call behaviour is not
  implemented. ebreak is unsupported.
- Floating-point extensions: floating-point instructions and associated
  architectural state are not modelled.
- Compressed extension: 16-bit RISC-V instructions are not supported.

## 2. Encoding Layouts

All RISC-V RV32I instructions are 32 bits wide, and their encoding format
determines the binary layout. The format share several field positions, but
differ in the arrangement of function fields, registers, and immediates.

### 2.1 Instruction fields

| Field    | Purpose                                                                                                        |
| -------- | -------------------------------------------------------------------------------------------------------------- |
| `opcode` | Identifies the instruction class and determines the general operation category.                                |
| `rd`     | Specifies the destination register.                                                                            |
| `rs1`    | Specifies the first source register.                                                                           |
| `rs2`    | Specifies the second source register.                                                                          |
| `funct3` | Further distinguishes instructions that share an opcode.                                                       |
| `funct7` | Further distinguishes instructions that share the same opcode and funct3                                       |
| `imm`    | Encodes a constant or offset used by the instruction. Its size and placement depend on the instruction format. |

### 2.2 R-type

```text
   31              25 24     20 19      15 14    12 11         7 6             0
   ┌──────────────────┬──────────┬──────────┬────────┬───────────┬─────────────┐
   │      funct7      │   rs2    │   rs1    │ funct3 │    rd     │   opcode    │
   └──────────────────┴──────────┴──────────┴────────┴───────────┴─────────────┘
```

**Supported instructions:** `add`, `sub`, `and`, `or`, `xor`, `slt`, `sll`, `srl`,
and `sra`.

For register-register shift instructions, the shift amount is obtained from the
lower 5 bits of `rs2`. 

### 2.3 I-type

```text
   31                      20 19      15 14    12 11         7 6             0
   ┌───────────────────────────┬──────────┬────────┬───────────┬─────────────┐
   │         imm[11:0]         │   rs1    │ funct3 │    rd     │   opcode    │
   └───────────────────────────┴──────────┴────────┴───────────┴─────────────┘
```

**Supported instructions:** `addi`, `lw`, `lb`, and `jalr`.

The I-type contains a 12-bit immediate, interpreted as a two's complement signed
integer and signed extended to 32 bits before use.

### 2.4 S-type

```text
   31        25 24      20 19      15 14    12 11          7 6             0
   ┌─────────────┬──────────┬──────────┬────────┬────────────┬─────────────┐
   │  imm[11:5]  │   rs2    │   rs1    │ funct3 │  imm[4:0]  │   opcode    │
   └─────────────┴──────────┴──────────┴────────┴────────────┴─────────────┘
```

**Supported instructions:** `sw` and `sb`.

S-type instructions encode a 12-bit immediate offset. The immediate is
reconstructed from the instruction fields, interpreted as a two's complement
signed integer, sign-extended, and added to `rs1` to calculate the effective
memory address.

### 2.5 B-type

```text
   31      30          25 24       20 19      15 14    12 11         8 7 6             0
   ┌────────┬─────────────┬───────────┬───────────┬────────┬───────────┬─┬─────────────┐
   │ imm[12]│  imm[10:5]  │    rs2    │    rs1    │ funct3 │ imm[4:1]  │ │   opcode    │
   └────────┴─────────────┴───────────┴───────────┴────────┴───────────┴─┴─────────────┘
                                                                      imm[11]
```

**Supported Instruction:** `beq`, `bne`, `blt`, and `bge`.

B-type instructions encode a 13-bit signed branch offset, with bit 0 implicitly
set to zero because branch targets are 2-byte aligned. The immediate is
reconstructed from the instruction fields, interpreted as a two's complement
signed integer, sign-extended to 32 bits, and added to the current PC when the
branch condition is satisfied.

### 2.6 U-type

```text
   31                         12 11       7 6            0
   ┌───────────────────────────┬───────────┬─────────────┐
   │         imm[31:12]        │    rd     │   opcode    │
   └───────────────────────────┴───────────┴─────────────┘
```

**Supported Instruction:** `lui`.

U-type instructions encode a 20-bit immediate value. The immediate is occupies
the upper 20 bits of `rd`, while the lower 12 bits are set to zero.

### 2.7 J-type

```text
   31     30           21 20  19             12 11         7 6             0
   ┌───────┬───────────────┬───┬─────────────────┬───────────┬─────────────┐
   │imm[20]│  imm[10:1]    │   │   imm[19:12]    │    rd     │   opcode    │
   └───────┴───────────────┴───┴─────────────────┴───────────┴─────────────┘
                          imm[11]
```

**Supported instruction:** `jal`.

J-type instructions encode a 21-bit signed jump offset, with bit 0 implicitly
set to zero. The immediate is reconstructed from the instruction fields,
interpreted as a two's complement signed integer, sign-extended to 32 bits, and
added to the current PC.

