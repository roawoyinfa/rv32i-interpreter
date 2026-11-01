#include "decoder.h"

#include <cstdint>
#include <expected>

namespace rv32i {

constexpr std::int32_t Decoder::sign_extend(std::uint32_t value, unsigned bits)
{
    return static_cast<std::int32_t>(value << (32 - bits)) >> (32 - bits);
}

constexpr std::int32_t Decoder::decode_s_imm(std::uint32_t instruction)
{
    const std::uint32_t imm =
        ((instruction >> 7) & 0x1F) |
        (((instruction >> 25) & 0x7F) << 5);

    return sign_extend(imm, 12);
}

constexpr std::int32_t Decoder::decode_b_imm(std::uint32_t instruction)
{
    const std::uint32_t imm =
        (((instruction >> 8) & 0x0F) << 1)  |
        (((instruction >> 25) & 0x3F) << 5) |
        (((instruction >> 7) & 0x01) << 11) |
        (((instruction >> 31) & 0x01) << 12);

    return sign_extend(imm, 13);
}

constexpr std::int32_t Decoder::decode_j_imm(std::uint32_t instruction)
{
    const std::uint32_t imm =
        (((instruction >> 21) & 0x3FF) << 1)  |
        (((instruction >> 20) & 0x001) << 11) |
        (((instruction >> 12) & 0x0FF) << 12) |
        (((instruction >> 31) & 0x001) << 20);

    return sign_extend(imm, 21);
}

std::expected<DecodedInstruction, DecoderError>
Decoder::decode(std::uint32_t instruction)
{
    const std::uint8_t opcode = instruction & 0x7F;

    switch (opcode) {
        case 0x33:  {
            const std::uint8_t funct3 = (instruction >> 12) & 0x7;
            const std::uint8_t funct7 = (instruction >> 25) & 0x7F;

            const std::uint8_t rd = (instruction >> 7) & 0x1F;
            const std::uint8_t rs1 = (instruction >> 15) & 0x1F;
            const std::uint8_t rs2 = (instruction >> 20) & 0x1F;

            auto make_instruction = [&](InstructionOperation operation) {
                return DecodedInstruction{
                    .operation = operation,
                    .opcode = opcode,
                    .rs1 = rs1,
                    .rs2 = rs2,
                    .rd = rd,
                    .funct3 = funct3,
                    .funct7 = funct7
                };
            };

            switch (funct3) {
                case 0x00: {
                    if (funct7 == 0x0) {
                        return make_instruction(InstructionOperation::ADD);
                    }

                    if (funct7 == 0x20) {
                        return make_instruction(InstructionOperation::SUB);
                    }

                    break;
                }

                case 0x07:
                    if (funct7 == 0x00) {
                        return make_instruction(InstructionOperation::AND);
                    }
                    break;

                case 0x06:
                    if (funct7 == 0x00) {
                        return make_instruction(InstructionOperation::OR);
                    }
                    break;

                case 0x04:
                    if (funct7 == 0x00) {
                        return make_instruction(InstructionOperation::XOR);
                    }
                    break;

                case 0x02:
                    if (funct7 == 0x00) {
                        return make_instruction(InstructionOperation::SLT);
                    }
                    break;

                case 0x01:
                    if (funct7 == 0x00) {
                        return make_instruction(InstructionOperation::SLL);
                    }
                    break;

                case 0x05: {
                    if (funct7 == 0x0) {
                        return make_instruction(InstructionOperation::SRL);
                    }

                    if (funct7 == 0x20) {
                        return make_instruction(InstructionOperation::SRA);
                    }

                    break;
                }
            }

            return std::unexpected(DecoderError::INVALID_ENCODING);
        }

        case 0x13: {
            const std::uint8_t funct3 = (instruction >> 12) & 0x7;
            const std::uint8_t rs1 = (instruction >> 15) & 0x1F;
            const std::uint8_t rd = (instruction >> 7) & 0x1F;

            const std::uint32_t imm12 = (instruction >> 20) & 0xFFF;
            const std::int32_t immediate = sign_extend(imm12, 12);

            if (funct3 == 0x00) {
                return DecodedInstruction{
                    .operation = InstructionOperation::ADDI,
                    .opcode = opcode,
                    .rs1 = rs1,
                    .rd = rd,
                    .funct3 = funct3,
                    .immediate = immediate
                };
            }

            if (funct3 == 0x01) {
                // SLLI: imm[11:5] must be 0000000
                if ((imm12 >> 5) == 0x00) {
                    return std::unexpected(DecoderError::UNSUPPORTED_INSTRUCTION);
                }
            }

            if (funct3 == 0x05) {
                // SRLI: imm[11:5] must be 0000000
                // SRAI: imm[11:5] must be 0100000
                const std::uint32_t upper = imm12 >> 5;
                if (upper == 0x00 || upper == 0x20) {
                    return std::unexpected(DecoderError::UNSUPPORTED_INSTRUCTION);
                }
            }

            if (funct3 == 0x02 ||
                funct3 == 0x03 ||
                funct3 == 0x04 ||
                funct3 == 0x06 ||
                funct3 == 0x07) {
                return std::unexpected(DecoderError::UNSUPPORTED_INSTRUCTION);
            }

            return std::unexpected(DecoderError::INVALID_ENCODING);
        }

        case 0x37: {
            const std::uint8_t rd = (instruction >> 7) & 0x1F;

            const std::int32_t immediate =
                static_cast<std::int32_t>(instruction & 0xFFFFF000);

            return DecodedInstruction{
                .operation = InstructionOperation::LUI,
                .opcode = opcode,
                .rd = rd,
                .immediate = immediate
            };
        }

        case 0x03: {
            const std::uint8_t funct3 = (instruction >> 12) & 0x7;

            const std::uint8_t rs1 = (instruction >> 15) & 0x1F;
            const std::uint8_t rd = (instruction >> 7) & 0x1F;

            const std::uint32_t imm12 = (instruction >> 20) & 0xFFF;
            const std::int32_t immediate = sign_extend(imm12, 12);

            auto make_instruction = [&](InstructionOperation operation) {
                return DecodedInstruction{
                    .operation = operation,
                    .opcode = opcode,
                    .rs1 = rs1,
                    .rd = rd,
                    .funct3 = funct3,
                    .immediate = immediate
                };
            };

            switch (funct3) {
                case 0x00:
                    return make_instruction(InstructionOperation::LB);

                case 0x02:
                    return make_instruction(InstructionOperation::LW);

                case 0x01:
                case 0x04:
                case 0x05:
                    return std::unexpected(
                        DecoderError::UNSUPPORTED_INSTRUCTION);

                default:
                    return std::unexpected(
                        DecoderError::INVALID_ENCODING);
            }
        }

        case 0x23: {
            const std::uint8_t funct3 = (instruction >> 12) & 0x07;

            const std::uint8_t rs1 = (instruction >> 15) & 0x1F;
            const std::uint8_t rs2 = (instruction >> 20) & 0x1F;

            const std::int32_t immediate = decode_s_imm(instruction);

            auto make_instruction = [&](InstructionOperation operation) {
                return DecodedInstruction{
                    .operation = operation,
                    .opcode = opcode,
                    .rs1 = rs1,
                    .rs2 = rs2,
                    .funct3 = funct3,
                    .immediate = immediate
                };
            };

            switch(funct3) {
                case 0x00:
                    return make_instruction(InstructionOperation::SB);

                case 0x02:
                    return make_instruction(InstructionOperation::SW);

                case 0x01:
                    return std::unexpected(
                        DecoderError::UNSUPPORTED_INSTRUCTION);

                default:
                    return std::unexpected(DecoderError::INVALID_ENCODING);
            }

        }

        case 0x63: {
            std::uint8_t funct3 = (instruction >> 12) & 0x07;

            std::uint8_t rs1 = (instruction >> 15) & 0x1F;
            std::uint8_t rs2 = (instruction >> 20) & 0x1F;

            std::int32_t immediate = decode_b_imm(instruction);

            auto make_instruction = [&](InstructionOperation operation) {
                return DecodedInstruction{
                    .operation = operation,
                    .opcode = opcode,
                    .rs1 = rs1,
                    .rs2 = rs2,
                    .funct3 = funct3,
                    .immediate = immediate
                };
            };

            switch (funct3) {
                case 0x00:
                    return make_instruction(InstructionOperation::BEQ);

                case 0x01:
                    return make_instruction(InstructionOperation::BNE);

                case 0x04:
                    return make_instruction(InstructionOperation::BLT);

                case 0x05:
                    return make_instruction(InstructionOperation::BGE);

                case 0x06:
                case 0x07:
                    return std::unexpected(
                        DecoderError::UNSUPPORTED_INSTRUCTION);

                default:
                    return std::unexpected(DecoderError::INVALID_ENCODING);
            }
        }

        case 0x6F: {
            std::uint8_t rd = (instruction >> 7) & 0x1F;

            std::int32_t immediate = decode_j_imm(instruction);

            return DecodedInstruction{
                .operation = InstructionOperation::JAL,
                .opcode = opcode,
                .rd = rd,
                .immediate = immediate
            };
        }

        case 0x67: {
            std::uint8_t funct3 = (instruction >> 12) & 0x7;

            if (funct3 != 0x00) {
                return std::unexpected(DecoderError::INVALID_ENCODING);
            }

            std::uint8_t rs1 = (instruction >> 15) & 0x1F;
            std::uint8_t rd = (instruction >> 7) & 0x1F;

            const std::uint32_t imm12 = (instruction >> 20) & 0xFFF;
            const std::int32_t immediate = sign_extend(imm12, 12);

            return DecodedInstruction{
                .operation = InstructionOperation::JALR,
                .opcode = opcode,
                .rs1 = rs1,
                .rd = rd,
                .funct3 = funct3,
                .immediate = immediate
            };
        }

        default:
            return std::unexpected(
                DecoderError::UNSUPPORTED_INSTRUCTION);
    }
}

}

