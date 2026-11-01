#include "decoder.h"

#include <cstdint>
#include <expected>

namespace rv32i {

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
            if (funct3 == 0x00) {
                const std::uint8_t rs1 = (instruction >> 15) & 0x1F;
                const std::uint8_t rd = (instruction >> 7) & 0x1F;

                const std::uint32_t imm12 = (instruction >> 20) & 0xFFF;

                const std::int32_t immediate =
                    static_cast<std::int32_t>(imm12 << 19) >> 19;

                return DecodedInstruction{
                    .operation = InstructionOperation::ADDI,
                    .opcode = opcode,
                    .rs1 = rs1,
                    .rd = rd,
                    .funct3 = funct3,
                    .immediate = immediate
                };
            }

            return std::unexpected(DecoderError::INVALID_ENCODING);
        }

        case 0x37: {
            const std::uint8_t rd = (instruction >> 7) & 0x1F;

            const std::uint32_t imm20 = (instruction >> 12) & 0xFFFFF;

            const std::int32_t immediate =
                static_cast<std::int32_t>(imm20 << 12);

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

            const std::int32_t immediate =
                static_cast<std::int32_t>(imm12 << 19) >> 19;

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

            if (funct3 == 0x02) {
                return make_instruction(InstructionOperation::LW);
            }

            if (funct3 == 0x00) {
                return make_instruction(InstructionOperation::LB);
            }

            return std::unexpected(DecoderError::INVALID_ENCODING);
        }

        case 0x23: {
            const std::uint8_t funct3 = (instruction >> 12) & 0x07;

            const std::uint8_t rs1 = (instruction >> 15) & 0x1F;
            const std::uint8_t rs2 = (instruction >> 20) & 0x1F;

            const std::uint32_t imm12 =
                ((instruction >> 7) & 0x1F) |
                (((instruction >> 25) & 0x7F) << 7);

            const std::int32_t immediate =
                static_cast<std::int32_t>(imm12 << 20) >> 20; // sign-extension

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


            if (funct3 == 0x02) {
                return make_instruction(InstructionOperation::SW);
            }

            if (funct3 == 0x00) {
                return make_instruction(InstructionOperation::SB);
            }

            return std::unexpected(DecoderError::INVALID_ENCODING);
        }

        case 0x63: {
            std::uint8_t funct3 = (instruction >> 12) & 0x07;

            std::uint8_t rs1 = (instruction >> 15) & 0x1F;
            std::uint8_t rs2 = (instruction >> 20) & 0x1F;

            std::uint32_t imm13 =
                (((instruction >> 8) & 0x0F) << 1) |
                (((instruction >> 25) & 0x3F) << 5) |
                (((instruction >> 7) & 0x01) << 11) |
                (((instruction >> 31) & 0x01) << 12);

            std::int32_t immediate =
                static_cast<std::int32_t>(imm13 << 19) >> 19; // sign-extension

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

            if (funct3 == 0x00) {
                return make_instruction(InstructionOperation::BEQ);
            }

            if (funct3 == 0x01) {
                return make_instruction(InstructionOperation::BNE);
            }

            if (funct3 == 0x04) {
                return make_instruction(InstructionOperation::BLT);
            }

            if (funct3 == 0x05) {
                return make_instruction(InstructionOperation::BGE);
            }

            return std::unexpected(DecoderError::INVALID_ENCODING);
        }

        case 0x6F: {
            std::uint8_t rd = (instruction >> 7) & 0x1F;

            const std::uint32_t imm21 =
                (((instruction >> 31) & 0x01) << 20) |
                (((instruction >> 12) & 0xFF) << 12) |
                (((instruction >> 20) & 0x01) << 11) |
                (((instruction >> 21) & 0x3FF) << 1);

            std::int32_t immediate =
                static_cast<std::int32_t>(imm21 << 10) >> 10;

            return DecodedInstruction{
                .operation = InstructionOperation::JAL,
                .opcode = opcode,
                .rd = rd,
                .immediate = immediate
            };
        }

        case 0x67: {
            std::uint8_t funct3 = (instruction >> 12) & 0x7;
            if (funct3 == 0x00) {
                std::uint8_t rs1 = (instruction >> 15) & 0x1F;
                std::uint8_t rd = (instruction >> 7) & 0x1F;

                const std::uint32_t imm12 = (instruction >> 20) & 0xFFF;

                const std::int32_t immediate =
                    static_cast<std::int32_t>(imm12 << 19) >> 19;

                return DecodedInstruction{
                    .operation = InstructionOperation::JALR,
                    .opcode = opcode,
                    .rs1 = rs1,
                    .rd = rd,
                    .funct3 = funct3,
                    .immediate = immediate
                };
            }

            return std::unexpected(DecoderError::INVALID_ENCODING);
        }

        default:
            return std::unexpected(
                DecoderError::UNSUPPORTED_INSTRUCTION
            );
    }
}

}

