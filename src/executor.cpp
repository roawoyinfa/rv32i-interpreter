#include "rv32i/executor.hpp"
#include "rv32i/execution_error.hpp"
#include "rv32i/instruction.hpp"
#include "rv32i/memory.hpp"

#include <cassert>
#include <cstdint>
#include <expected>

namespace rv32i {
namespace {

[[nodiscard]]
ExecutionOutcome add(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                     ExecutionContext& context)
{
    const std::uint32_t result = context.registers.read(rs1) +
        context.registers.read(rs2);

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome sub(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                     ExecutionContext& context)
{
    const std::uint32_t result = context.registers.read(rs1) -
        context.registers.read(rs2);

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome addi(std::uint8_t rd, std::uint8_t rs1, std::int32_t immediate,
                      ExecutionContext& context)
{
    const std::uint32_t result = context.registers.read(rs1) +
        static_cast<std::uint32_t>(immediate);

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome and_(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                      ExecutionContext& context)
{
    const std::uint32_t result = context.registers.read(rs1) &
        context.registers.read(rs2);

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome or_(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                     ExecutionContext& context)
{
    const std::uint32_t result = context.registers.read(rs1) |
        context.registers.read(rs2);

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome xor_(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                      ExecutionContext& context)
{
    const std::uint32_t result = context.registers.read(rs1) ^
        context.registers.read(rs2);

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome slt(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                     ExecutionContext& context)
{
    const auto lhs = static_cast<std::int32_t>(context.registers.read(rs1));
    const auto rhs = static_cast<std::int32_t>(context.registers.read(rs2));

    context.registers.write(rd, lhs < rhs ? 1 : 0);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome sll(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                     ExecutionContext& context)
{
    const std::uint8_t low_5_bits = context.registers.read(rs2) & 0x0000001F;

    std::uint32_t result = context.registers.read(rs1) << low_5_bits;

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome srl(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                     ExecutionContext& context)
{
    const std::uint8_t shift_amount = context.registers.read(rs2) & 0x1F;

    const std::uint32_t value = context.registers.read(rs1);

    const std::uint32_t result = value >> shift_amount;

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome sra(std::uint8_t rd, std::uint8_t rs1, std::uint8_t rs2,
                     ExecutionContext& context)
{
    const std::uint8_t shift_amount = context.registers.read(rs2) & 0x1F;

    const std::int32_t value = static_cast<std::int32_t>(
        context.registers.read(rs1));

    const std::uint32_t result =  value >> shift_amount;

    context.registers.write(rd, result);

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
ExecutionOutcome lui(std::uint8_t rd, std::int32_t imm,
                     ExecutionContext& context)
{
    context.registers.write(rd, static_cast<std::uint32_t>(imm));

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
lw(std::uint8_t rd, std::int32_t offset, std::uint8_t rs1,
   ExecutionContext& context)
{
    const std::uint32_t base_address = context.registers.read(rs1);

    const std::uint32_t word_address = base_address + offset;

    const auto result{context.memory.read_word(word_address)};

    if (!result) {
        switch (result.error()) {
            case MemoryError::MISALIGNED:
                return std::unexpected(
                    ExecutionError::MEMORY_ACCESS_MISALIGNED);

            case MemoryError::OUT_OF_BOUNDS:
                return std::unexpected(
                    ExecutionError::MEMORY_ACCESS_OUT_OF_BOUNDS);
        }
    }

    context.registers.write(rd, result.value());

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
sw(std::uint8_t rs2, std::int32_t offset, std::uint8_t rs1,
   ExecutionContext& context)
{
    const std::uint32_t value = context.registers.read(rs2);

    const std::uint32_t base_address = context.registers.read(rs1);

    const std::uint32_t word_address = base_address + offset;

    const auto result{context.memory.write_word(word_address, value)};

    if (!result) {
        switch (result.error()) {
            case MemoryError::MISALIGNED:
                return std::unexpected(
                    ExecutionError::MEMORY_ACCESS_MISALIGNED);

            case MemoryError::OUT_OF_BOUNDS:
                return std::unexpected(
                    ExecutionError::MEMORY_ACCESS_OUT_OF_BOUNDS);
        }
    }

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
lb(std::uint8_t rd, std::int32_t offset, std::uint8_t rs1,
   ExecutionContext& context)
{
    const std::uint32_t base_address = context.registers.read(rs1);

    const std::uint32_t word_address = base_address + offset;

    const auto result{context.memory.read_byte(word_address)};

    if (!result) {
        return std::unexpected(ExecutionError::MEMORY_ACCESS_OUT_OF_BOUNDS);
    }

    const std::int32_t value = static_cast<std::int8_t>(result.value());

    context.registers.write(rd, static_cast<std::uint32_t>(value));

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
sb(std::uint8_t rs2, std::int32_t offset, std::uint8_t rs1,
   ExecutionContext& context)
{
    const std::uint32_t value = context.registers.read(rs2) & 0xFF;

    const std::uint32_t base_address = context.registers.read(rs1);

    const std::uint32_t word_address = base_address + offset;

    const auto result{context.memory.write_byte(word_address, value)};

    if (!result) {
        return std::unexpected(ExecutionError::MEMORY_ACCESS_OUT_OF_BOUNDS);
    }

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
beq(std::uint8_t rs1, std::uint8_t rs2, std::int32_t offset,
    ExecutionContext& context)
{
    const std::uint32_t lhs = context.registers.read(rs1);
    const std::uint32_t rhs = context.registers.read(rs2);

    if (lhs != rhs) {
        return {};
    }

    const std::uint32_t target_address = context.pc + offset;

    if (target_address >= Memory::MEMORY_CAPACITY) {
        return std::unexpected(
            ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS);
    }

    if (target_address % 4 != 0) {
        return std::unexpected(ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED);
    }

    context.next_pc = target_address;

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
bne(std::uint8_t rs1, std::uint8_t rs2, std::int32_t offset,
    ExecutionContext& context)
{
    const std::uint32_t lhs = context.registers.read(rs1);
    const std::uint32_t rhs = context.registers.read(rs2);

    if (lhs == rhs) {
        return {};
    }

    const std::uint32_t target_address = context.pc + offset;

    if (target_address >= Memory::MEMORY_CAPACITY) {
        return std::unexpected(
            ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS);
    }

    if (target_address % 4 != 0) {
        return std::unexpected(ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED);
    }

    context.next_pc = target_address;

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
blt(std::uint8_t rs1, std::uint8_t rs2, std::int32_t offset,
    ExecutionContext& context)
{
    const std::uint32_t lhs = context.registers.read(rs1);
    const std::uint32_t rhs = context.registers.read(rs2);

    if (lhs < rhs) {
        return {};
    }

    const std::uint32_t target_address = context.pc + offset;

    if (target_address >= Memory::MEMORY_CAPACITY) {
        return std::unexpected(
            ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS);
    }

    if (target_address % 4 != 0) {
        return std::unexpected(ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED);
    }

    context.next_pc = target_address;

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
bge(std::uint8_t rs1, std::uint8_t rs2, std::int32_t offset,
    ExecutionContext& context)
{
    const std::uint32_t lhs = context.registers.read(rs1);
    const std::uint32_t rhs = context.registers.read(rs2);

    if (lhs >= rhs) {
        return {};
    }

    const std::uint32_t target_address = context.pc + offset;

    if (target_address >= Memory::MEMORY_CAPACITY) {
        return std::unexpected(
            ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS);
    }

    if (target_address % 4 != 0) {
        return std::unexpected(ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED);
    }

    context.next_pc = target_address;

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
jal(std::uint8_t rd, std::int32_t offset, ExecutionContext& context)
{
    const std::uint32_t target_address = context.pc + offset;

    if (target_address >= Memory::MEMORY_CAPACITY) {
        return std::unexpected(
            ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS);
    }

    if (target_address % 4 != 0) {
        return std::unexpected(ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED);
    }

    const std::uint32_t return_address = context.pc + 4;

    context.registers.write(rd, return_address);
    context.next_pc = target_address;

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
jalr(std::uint8_t rd, std::int32_t offset, std::uint8_t rs1,
     ExecutionContext& context)
{
    const std::uint32_t base_address = context.registers.read(rs1);

    const std::uint32_t target_address = (base_address + offset) &
        ~std::uint32_t{1};

    if (target_address >= Memory::MEMORY_CAPACITY) {
        return std::unexpected(
            ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS);
    }

    if (target_address % 4 != 0) {
        return std::unexpected(ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED);
    }

    const std::uint32_t return_address = context.pc + 4;

    context.registers.write(rd, return_address);
    context.next_pc = target_address;

    return ExecutionOutcome::CONTINUE;
}

[[nodiscard]] ExecutionOutcome ecall()
{
    return ExecutionOutcome::HALT;
}

}

std::expected<ExecutionOutcome, ExecutionError>
execute(const DecodedInstruction& instruction, ExecutionContext& context)
{
    switch (instruction.operation) {
        case InstructionOperation::ADD:
            return add(instruction.rd, instruction.rs1, instruction.rs2,
                       context);

        case InstructionOperation::SUB:
            return sub(instruction.rd, instruction.rs1, instruction.rs2,
                       context);

        case InstructionOperation::ADDI:
            return addi(instruction.rd, instruction.rs1, instruction.immediate,
                        context);

        case InstructionOperation::AND:
            return and_(instruction.rd, instruction.rs1, instruction.rs2,
                        context);

        case InstructionOperation::OR:
            return or_(instruction.rd, instruction.rs1, instruction.rs2,
                       context);

        case InstructionOperation::XOR:
            return xor_(instruction.rd, instruction.rs1, instruction.rs2,
                        context);

        case InstructionOperation::SLT:
            return slt(instruction.rd, instruction.rs1, instruction.rs2,
                       context);

        case InstructionOperation::SLL:
            return sll(instruction.rd, instruction.rs1, instruction.rs2,
                       context);

        case InstructionOperation::SRL:
            return srl(instruction.rd, instruction.rs1, instruction.rs2,
                       context);

        case InstructionOperation::SRA:
            return sra(instruction.rd, instruction.rs1, instruction.rs2,
                       context);

        case InstructionOperation::LUI:
            return lui(instruction.rd, instruction.immediate, context);

        case InstructionOperation::LW:
            return lw(instruction.rd, instruction.immediate, instruction.rs1,
                      context);

        case InstructionOperation::SW:
            return sw(instruction.rs2, instruction.immediate, instruction.rs1,
                      context);

        case InstructionOperation::LB:
            return lb(instruction.rd, instruction.immediate, instruction.rs1,
                      context);

        case InstructionOperation::SB:
            return sb(instruction.rs2, instruction.immediate, instruction.rs1,
                      context);

        case InstructionOperation::BEQ:
            return beq(instruction.rs1, instruction.rs2, instruction.immediate,
                       context);

        case InstructionOperation::BNE:
            return bne(instruction.rs1, instruction.rs2, instruction.immediate,
                       context);

        case InstructionOperation::BLT:
            return blt(instruction.rs1, instruction.rs2, instruction.immediate,
                       context);

        case InstructionOperation::BGE:
            return bge(instruction.rs1, instruction.rs2, instruction.immediate,
                       context);

        case InstructionOperation::JAL:
            return jal(instruction.rd, instruction.immediate, context);

        case InstructionOperation::JALR:
            return jalr(instruction.rd, instruction.immediate, instruction.rs1,
                        context);

        case InstructionOperation::ECALL:
            return ecall();
    }
}

}

