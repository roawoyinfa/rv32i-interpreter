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

## 3. Machine Model and Structure

### 3.1 Architectural state

The interpreter models the architectural state of a RISC-V RV32I processor using
three components: a program counter (PC), a general-purpose register file, and
byte-addressable memory. Together, these components represent the machine state
that instructions operate on.

| Component            | Description                                                                                        |
| ---------------------| ---------------------------------------------------------------------------------------------------|
| Program Counter (PC) | A 32-bit value that holds the address of the current instruction being executed.                   |
| Register file        | Contains 32 general-purpose registers, x0 through x31, each 32 bits wide. x0 is hardwired to zero. |
| Memory               | Byte-addressable storage using little-endian byte ordering.                                        |

### 3.2 Memory model

Memory is represented as an array of bytes, with each element holding a 8-bit
value. Each byte occupies a distinct address, allowing instructions to access
memory at byte granularity.

The interpreter uses little-endian byte ordering, meaning that the least
significant byte of a multi-byte value is stored at the lowest memory address.
Consequently, a 32-bit word occupies four consecutive memory locations, with
its bytes arranged from least significant to most significant.

The load and store instructions operate on this byte-addressable memory:

- `lw` reads four consecutive bytes and combines them into a 32-bit value.
- `lb` reads a single byte and sign-extends it to 32 bits.
- `sw` writes the four bytes of a 32-bit register value into consecutive memory
  locations.
- `sb` writes only the least significant byte of a register into memory.

Memory accesses use an effective address calculated from the base register and
the instruction's sign-extended immediate offset. The alignment requirements
and handling of misaligned accesses are documented separately in section 4.2.

### 3.3 Program format and loading

The interpreter accepts programs as text files containing hexadecimal
machine-code words. Each line contains one 32-bit hexadecimal word representing
a single instruction.

Instructions are loaded sequentially into memory, beginning at the start of the
memory region. Each instruction occupies four bytes. The program counter is
initialised to the beginning of memory, allowing execution to start from the
first loaded instruction.

The interpreter therefore operates directly on machine-code words rather than
assembly source code.

### 3.4 Interpreter structure

The interpreter separates instruction decoding from instruction execution into
two distinct stages.

#### Decoder stage

The decode stage receives a raw 32-bit machine instruction and extracts its
constituent fields, including the opcode, register indices, function fields,
and immediate value. These fields are interpreted to identify the corresponding
instruction operation and are assembled into a structured representation.

#### Executor stage

The execute stage receives the decoded instruction and dispatches execution
according to its identified operation. It implements the semantics of the
supported instructions, performing the required arithmetic, logical, memory,
or control-flow operations and updating the architectural state accordingly.

The interface between the two stages is the `DecodedInstruction` structure. It
stores the identified operation alongside the extracted instruction fields and
the decoded immediate.

Separating decoding from execution keeps instruction interpretation independent
of instruction behaviour. This makes the decoder easier to test independently and
allows the execution stage to operate on a consistent instruction representation
rather than repeatedly extracting fields from the raw machine word.

### 3.5 Trace format

The interpreter produces a trace entry for each instruction executed. Each entry
begins with the instruction's program counter (PC), its raw 32-bit machine-code
word (WORD), and its decoded mnemonic (MNEMONIC).

Additional trace lines record the effects of the instruction on the
architectural state:

- Register updates: When an instruction modifies a register, the trace records
  the destination register and its value before and after execution.
- Memory updates: When an instruction writes to memory, the trace records each
  affected memory address and the byte value before and after the write.
- Halt message: When the ECALL instruction is encountered, the trace records a
  halt message indicating that the simulator has stopped execution.

This trace format makes it possible to follow instruction execution and observe
changes to the machine state.

The following excerpt shows the trace format produced by the memory integration
test:

```text
[TRACE] PC=0x0000000C WORD=0x00152023 MNEMONIC=SW
        MEM [0x00000100]: 0x00 -> 0x78
        MEM [0x00000101]: 0x00 -> 0x56
        MEM [0x00000102]: 0x00 -> 0x34
        MEM [0x00000103]: 0x00 -> 0x12

[TRACE] PC=0x00000010 WORD=0x00052103 MNEMONIC=LW
        REG x2: 0x00000000 -> 0x12345678

[TRACE] PC=0x00000024 WORD=0x00450303 MNEMONIC=LB
        REG x6: 0x00000000 -> 0xFFFFFF80

[TRACE] PC=0x0000002C WORD=0x00000073 MNEMONIC=ECALL
        HALT: Simulator halt
```

## 4. Declared Design Decisions

### 4.1 Halt convention

#### Context

The interpreter requires a defined mechanism for terminating program execution.
Since RISC-V does not define an explicit termination command, the processor
simulator would need to rely on an external condition.

#### Decision

The interpreter uses the `ECALL` instruction as a project-specific halt condition.

#### Alternatives considered

Tracking the program boundary and terminating execution when program counter
reaches the end of the loaded program.

#### Rationale

Using `ECALL` provides an explicit termination instruction within the program
itself. This makes program termination predictable.

### 4.2 Misaligned-access policy

#### Context

Memory instructions access data of different widths. A word access (`LW`, `SW`)
requires a four-byte aligned effective address to be naturaly aligned, whereas a
byte (`LB`, `SW`) has no additional alignment requirement.

The RISC-V specification leaves the behavior misaligned accesses to the
execution environment.

#### Decision

The interpreter reports an error when a memory instruction attempts a misaligned
access.

#### Alternatives considered

Allow misaligned accesses to complete successfully.

#### Rationale

Rejecting misaligned accesses provides an explicit memory-access policy.

### 4.3 Endianness

#### Context

Multi-byte values occupy a contiguous sequence of bytes. These bytes must
be ordered within the sequence. Endianness determines the order of inidividual
bytes within a multi-byte value.

#### Decision

The interpreter uses little-endian ordering.

#### Alternatives considered

Big-endian is another byte ordering convention.

#### Rationale

Preserves the byte-ordering semantics of the RISC-V RV32I architecture.

### 4.4 Unsupported or invalid instructions

#### Context

The interpreter only implements a defined subset of the RV32I instruction set.
During decoding, it may encounter instruction encoding that is invalid or that
is not implemented by the interpreter. 

#### Decision

The interpreter reports an appropriate error when it encounters an invalid
instruction encoding or an unsupported instruction operation.

#### Rationale

Reporting an error prevents invalid or unsupported instructions from being
executed as though they were valid.

## 5. Semantics I Had to Get Right

### 5.1 x0 behavior

#### Rule

The RISC-V register `x0` is hardwired to zero: reading `x0` always return zero,
and writes to it have no effect.

#### Why it is easy to get wrong

The register file contains 32 general-purpose registers, which may lead to the
assumption that all 32 registers behave identically.

#### Implementation

[`RegisterFile`](../src/register_file.cpp) enfoces this behavior in its read
and write operations

### 5.2 32-bit wraparound

#### rule

RV32I registers hold 32-bit values. Arithmetic results written to a register are
therefore limited to 32 bits. Overflow wraps around modulo $2^32$.

#### Why it is easy to get wrong

A host programming language may use integer types wider than 32 bits. If the
interpreter does not enforce the 32-bit result width, arithmetic operations may
produce values that cannot be represented in an RV32I register.

This can cause subsequent instructions to operate on incorrect values.

#### Implementation

The interpreter uses `std::uint32_t` to represent register values. Unsigned
32-bit arithmetic provides modulo-(2^{32}) behaviour, ensuring that arithmetic
results retain only the lower 32 bits when stored in a register.

### 5.3 Sign extension

#### rule

Immediate values encoded in fewer than 32 bits are interpreted as signed
two's-complement values and sign-extended to 32 bits before use.

#### Why it is easy to get wrong

Sign-extension must preserves the value represented by the original bit pattern.

#### Implementation

The decoder sign-extends immediate values while extracting instruction fields,
before constructing the `DecodedInstruction` representation.

### 5.4 Signed vs unsigned comparison

#### rule

The `SLT`, `BLT`, and `BGE` instructions interpret their register operands as
signed 32-bit integers when performing comparisons.

#### why it is easy to get wrong

Register values are stored as unsigned 32-bit integers. Performing comparisons
directly on these values when an instruction requires signed comparison can
produce incorrect results, particularly when an operand has its most significant
bit set.

#### Implementation

The interpreter converts register values to signed 32-bit integers before
performing comparisons in `SLT`, `BLT`, and `BGE`.

### 5.5 Shift-amount masking

#### Rule

For register-register shift instructions, RV32I uses only the lower five bits
of the second source register (rs2) to determine the shift amount.

#### Why it is easy to get wrong

Using the entire value of `rs2` as the shift amount can produce incorrect
results. It can also result in undefined behaviour in C++ when the shift amount
is greater than or equal to the width of the operand.

#### Implementation

The interpreter masks the shift amount with `0x1F`, ensuring that only the lower
five bits of rs2 determine the number of positions to shift.

### 5.6 Branch and jump targets, and `jalr` target handling

#### Rule

Conditional branches (`BEQ`, `BNE`, `BLT`, and `BGE`) and `JAL` use PC-relative
addressing. Their target addresses are calculated by adding a sign-extended
immediate offset to the current program counter.

`JALR` uses register-relative addressing. Its target is calculated by adding a
sign-extended immediate to the value in the base register (`rs1`), then clearing
bit `0` of the resulting address.

Under the base RV32I instruction alignment requirement, instruction addresses
must be four-byte aligned.

#### Why it is easy to get wrong

Branch and jump immediates are signed offsets, not absolute addresses. Treating
them as absolute addresses or adding them to the wrong reference address produces
incorrect control flow.

`JALR` also has a special target-address rule: bit `0` must be cleared after
adding the immediate to the base register. Clearing only bit 0 does not
necessarily guarantee four-byte alignment.

#### Implementation

After calculating the target address, the interpreter checks whether it is
four-byte aligned. If the target address is not divisible by four, execution
returns an `INSTRUCTION_ADDRESS_MISALIGNED` error.

### 5.7 Bugs the tests caught

#### Signed vs Unsigned comparisons

One bug identified during testing involved using unsigned comparisons for the
`SLT`, `BLT`, and `BGE` instructions instead of signed comparisons. Since
register values are stored as unsigned 32-bit integers, directly comparing them
as unsigned values produces incorrect results when operands represent negative
numbers.

## 6. Hand-Decode Excercises

### 6.1 R-type word

**Hex word:** 0x002081B3

**Binary representation:** 0000 0000 0010 0000 1000 0001 1011 0011

**Instruction format**: R-type

```text
   31              25 24     20 19      15 14    12 11         7 6             0
   ┌──────────────────┬──────────┬──────────┬────────┬───────────┬─────────────┐
   │      funct7      │   rs2    │   rs1    │ funct3 │    rd     │   opcode    │
   │     0000000      │  00010   │  00001   │  000   │  00011    │   0110011   │
   └──────────────────┴──────────┴──────────┴────────┴───────────┴─────────────┘
```

**Field interpretation:**

- `opcode = 0110011`: R-type integer register-register operation.
- `rd = 00011`: destination register `x3`
- `rs1 = 00001`: first source register `x1`
- `rs2 = 00010`: second source register `x2`
- `funct3 = 000` and funct7 = `0000000`: identifies `ADD`

**Decoded instruction:**

```
add x3, x1, x2
```
**Effect on state:**

The instruction performs:

$$
\text{x3} \leftarrow \text{x1}+\text{x2}
$$

For illustration, assume the initial register values are:

| Register | Initial value |
| -------- | ------------- |
| x1       | 0x00000005    |
| x2       | 0x00000003    |
| x3       | 0x00000000    |

The expected state change is:

| Register | Before     | After      |
| -------- | ---------- | -----------|
| x3       | 0x00000000 | 0x00000008 |
| x1       | 0x00000005 | unchanged  |
| x1       | 0x00000003 | unchanged  |

**Interpreter verification:**

```text
[TRACE] PC=0x00000008 WORD=0x002081B3 MNEMONIC=ADD
        REG x3: 0x00000000 -> 0x00000008
```

### 6.2 I-type word

**Hex word:** 0x00A00093

**Binary representation:** 0000 0000 1010 0000 0000 0000 1001 0011

**Instruction format**: I-type

```text
   31                      20 19      15 14    12 11         7 6             0
   ┌───────────────────────────┬──────────┬────────┬───────────┬─────────────┐
   │        imm[11:0]          │   rs1    │ funct3 │    rd     │   opcode    │
   │       000000001010        │  00000   │  000   │   00001   │   0010011   │
   └───────────────────────────┴──────────┴────────┴───────────┴─────────────┘
```

**Field interpretation:**

- `opcode = 0110011`: I-type integer operation.
- `rd = 00001`: destination register `x1`
- `rs1 = 00000`: source register `x0`
- `funct3 = 000`: identifies `ADDI`
- `immediate = 000000001010`: signed immediate value `10`

**Decoded instruction:**

```
addi x1, x0, 10
```

**Effect on state:**

The instruction performs:

$$
\text{x1} \leftarrow \text{x0}+\text{10}
$$

Since `x0` is always zero:

$$
\text{x1} \leftarrow \text{0}+\text{10}=10
$$

The expected state change is:

| Register | Before     | After      |
| -------- | ---------- | -----------|
| x0       | 0x00000000 | unchanged  |
| x1       | 0x00000000 | 0x0000000A |

**Interpreter verification:**

```text
[TRACE] PC=0x00000000 WORD=0x00A00093 MNEMONIC=ADDI
        REG x1: 0x00000000 -> 0x0000000A
```

### 6.3 S-type word

**Hex word:** 0x0020A423

**Binary representation:** 0000 0000 0010 0000 1010 0100 0010 0011

**Instruction format**: S-type

```text
   31        25 24      20 19      15 14    12 11          7 6             0
   ┌─────────────┬──────────┬──────────┬────────┬────────────┬─────────────┐
   │  imm[11:5]  │   rs2    │   rs1    │ funct3 │  imm[4:0]  │   opcode    │
   │   0000000   │  00010   │  00001   │  010   │   01000    │   0100011   │
   └─────────────┴──────────┴──────────┴────────┴────────────┴─────────────┘
```

**Field interpretation:**

- `opcode = 0100011`: S-type store instruction.
- `rs1 = 00001`: base address register `x1`.
- `rs2 = 00010`: second source register `x2`, containing the value to be stored.
- `funct3 = 010`: identifies `SW`.
- `immediate[11:5] = 0000000` and `immediate[4:0] = 01000`: the two immediate
   fields are combined to reconstruct the 12-bit immediate `000000001000`,
   representing the signed value `8`.

**Decoded instruction:**

```
sw x2, 8(x1)
```

**Effect on state:**

The instruction calculates the effective memory address by adding the
sign-extended immediate to the value in the base register:

$$
\text{Effective address}=x1+8
$$

For illustration, assume the initial register values are:

| Register | Initial value |
| -------- | ------------- |
| `x1`     | `0x00000100`  |
| `x2`     | `0x12345678`  |

The effective address is `0x00000108`. The instruction stores the 32-bit value
in `x2` across four consecutive memory locations using little-endian byte
ordering.

| Memory address | Before | After  |
| -------------- | ------ | ------ |
| `0x00000108`   | `0x00` | `0x78` |
| `0x00000109`   | `0x00` | `0x56` |
| `0x0000010A`   | `0x00` | `0x34` |
| `0x0000010B`   | `0x00` | `0x12` |

The source registers `x1` and `x2` remain unchanged.

**Interpreter verification:**

```text
[TRACE] PC=0x0000000C WORD=0x0020A423 MNEMONIC=SW
        MEM [0x00000108]: 0x00 -> 0x78
        MEM [0x00000109]: 0x00 -> 0x56
        MEM [0x0000010A]: 0x00 -> 0x34
        MEM [0x0000010B]: 0x00 -> 0x12
```

## 7. Test programs

### 7.1 Testing approach

The interpreter is validated using integration tests that execute complete
machine-code programs.

The tests are organised around groups of related instruction operations and
behaviours:

- Arithmetic: integer arithmetic, logical operations, and register updates.
- Shifts: logical and arithmetic shift operations, including shift-amount
  handling.
- Memory: load and store operations, byte ordering, sign extension, and memory
  access behaviour.
- Branches: conditional branch instructions and control-flow changes.
- Jumps: JAL, JALR, and return-address calculation.
- Loop sum: repeated instruction execution, conditional loop termination, and
  accumulated results.
- Array sum: Memory-backed iteration, pointer arithmetic, repeated loads, and
  accumulation within a loop.

The test programs are maintained in [`tests/`](../tests), with the test
implementation responsible for loading programs, executing them, and validating
their results.

### 7.2 Test coverage

The following table summarises the integration tests and the behaviours
they validate.

| Test program    | Instructions exercised                                 | Validation focus                               |
|-----------------|--------------------------------------------------------|------------------------------------------------|
| `arithmetic.hex`| `LUI`, `ADDI`, `ADD`, `SUB`, `AND`, `OR`, `XOR`, `SLT` | Arithmetic, logical operations, signed comparison, and register behaviour |
| `shifts.hex`    | `SLL`, `SRL`, `SRA`, `LUI`, `ADDI`                     | Shift operations and shift-amount handling     |
| `memory.hex`    | `LW`, `SW`, `LB`, `SB`, `LUI`, `ADDI`                  | Memory access, little-endian byte ordering, and byte sign extension |
| `branches.hex`  | `BEQ`, `BNE`, `BLT`, `BGE`, `ADDI`                     | Conditional branch evaluation and control flow |
| `jumps.hex`     | `JAL`, `JALR`, `ADDI`                                  | Jump targets and return-address calculation    |
| `loop_sum.hex`  | `ADD`, `ADDI`, `BLT`                                   | Repeated execution, loop termination, and accumulation |
| `array_sum.hex` | `LW`, `SW`, `ADD`, `ADDI`, `BNE`                       | Memory-backed iteration, pointer arithmetic, and accumulation |

The tests cover the supported instruction operations individually and in
combination. In particular, the loop-sum and array-sum programs validatea
interactions between arithmetic, memory access, and control-flow operations.

### 7.3 Test results

Test results are determined by comparing the interpreter's execution state
against the expected register and memory values defined by the tests.

The complete test programs and their corresponding test implementation are
available in the project repository under [`tests/`](../tests).

