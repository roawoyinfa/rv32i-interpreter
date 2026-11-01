#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <expected>

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
};

struct DecodedInstruction {
    InstructionOperation operation;

    std::uint8_t opcode;

    uint8_t rs1;
    uint8_t rs2;
    uint8_t rd;

    uint8_t funct3;
    uint8_t funct7;

    int32_t immediate;
};

enum class DecoderError {
    UNSUPPORTED_INSTRUCTION,
    INVALID_ENCODING
};

class Decoder {
public:
    Decoder() = default;

    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;

    Decoder(Decoder&&) = delete;
    Decoder& operator=(Decoder&&) = delete;

    [[nodiscard]]
    std::expected<DecodedInstruction, DecoderError>
    decode(std::uint32_t instruction);
};

}

#endif

