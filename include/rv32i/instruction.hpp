#ifndef RV32I_INSTRUCTION_HPP
#define RV32I_INSTRUCTION_HPP

#include <cstdint>

namespace rv32i {

enum class InstructionOperation {
    ADD, SUB, ADDI,
    AND, OR, XOR,
    SLT,
    SLL, SRL, SRA,
    LUI,
    LW, SW, LB, SB,
    BEQ, BNE, BLT, BGE,
    JAL, JALR,
    ECALL
};

struct DecodedInstruction {
    InstructionOperation operation;

    std::uint8_t opcode{};

    uint8_t rs1{};
    uint8_t rs2{};
    uint8_t rd{};

    uint8_t funct3{};
    uint8_t funct7{};

    int32_t immediate{};
};

}

#endif

