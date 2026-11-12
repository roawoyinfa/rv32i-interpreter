#include "rv32i/cpu.hpp"
#include "rv32i/executor.hpp"
#include "rv32i/execution_error.hpp"
#include "rv32i/decoder.hpp"
#include "rv32i/program.hpp"
#include "rv32i/memory.hpp"

#include <array>
#include <cstddef>
#include <expected>
#include <iomanip>
#include <iostream>
#include <utility>

namespace rv32i {

namespace
{
    const char* instruction_name(InstructionOperation operation) noexcept
    {
        switch (operation) {
            case InstructionOperation::ADD:   return "ADD";
            case InstructionOperation::SUB:   return "SUB";
            case InstructionOperation::ADDI:  return "ADDI";
            case InstructionOperation::AND:   return "AND";
            case InstructionOperation::OR:    return "OR";
            case InstructionOperation::XOR:   return "XOR";
            case InstructionOperation::SLT:   return "SLT";
            case InstructionOperation::SLL:   return "SLL";
            case InstructionOperation::SRL:   return "SRL";
            case InstructionOperation::SRA:   return "SRA";
            case InstructionOperation::LUI:   return "LUI";
            case InstructionOperation::LW:    return "LW";
            case InstructionOperation::SW:    return "SW";
            case InstructionOperation::LB:    return "LB";
            case InstructionOperation::SB:    return "SB";
            case InstructionOperation::BEQ:   return "BEQ";
            case InstructionOperation::BNE:   return "BNE";
            case InstructionOperation::BLT:   return "BLT";
            case InstructionOperation::BGE:   return "BGE";
            case InstructionOperation::JAL:   return "JAL";
            case InstructionOperation::JALR:  return "JALR";
            case InstructionOperation::ECALL: return "ECALL";
        }

        return "UNKNOWN";
    }

    bool writes_register(InstructionOperation operation) noexcept
    {
        switch (operation) {
            case InstructionOperation::ADD:
            case InstructionOperation::SUB:
            case InstructionOperation::ADDI:
            case InstructionOperation::AND:
            case InstructionOperation::OR:
            case InstructionOperation::XOR:
            case InstructionOperation::SLT:
            case InstructionOperation::SLL:
            case InstructionOperation::SRL:
            case InstructionOperation::SRA:
            case InstructionOperation::LUI:
            case InstructionOperation::LW:
            case InstructionOperation::LB:
            case InstructionOperation::JAL:
            case InstructionOperation::JALR:
                return true;

            default:
                return false;
        }
    }
}

std::expected<CPU, CPUError> CPU::create(Program program) noexcept
{
    CPU cpu{};

    cpu.program_end = static_cast<std::uint32_t>(program.instructions.size() *
                                                 sizeof(std::uint32_t));

    if (program.instructions.size() >
        Memory::MEMORY_CAPACITY / sizeof(std::uint32_t)) {
        return std::unexpected(CPUError::PROGRAM_TOO_LARGE);
    }

    for (std::size_t i = 0; i < program.instructions.size(); ++i) {
        const std::uint32_t address = i * sizeof(std::uint32_t);

        const auto result = cpu.memory.write_word(address,
                                                  program.instructions[i]);

        if (!result) {
            return std::unexpected(CPUError::MEMORY_INITIALIZATION_FAILED);
        }
    }

    return cpu;
}

std::expected<ExecutionOutcome, ExecutionError> CPU::step() noexcept
{
    if (pc >= program_end)
    {
        return std::unexpected(ExecutionError::EXECUTION_OUTSIDE_PROGRAM);
    }

    const std::uint32_t current_pc = pc;

    const auto fetch_result = memory.read_word(current_pc);

    if (!fetch_result) {
        switch (fetch_result.error()) {
            case MemoryError::OUT_OF_BOUNDS:
                return std::unexpected(
                    ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS);

            case MemoryError::MISALIGNED:
                return std::unexpected(
                    ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED);
        }

        std::unreachable();
    }

    const std::uint32_t instruction_word = fetch_result.value();

    const auto decoder_result = decode(instruction_word);

    if (!decoder_result) {
        switch (decoder_result.error()) {
            case DecoderError::UNSUPPORTED_INSTRUCTION:
                return std::unexpected(ExecutionError::UNSUPPORTED_INSTRUCTION);

            case DecoderError::INVALID_ENCODING:
                return std::unexpected(ExecutionError::INVALID_INSTRUCTION);
        }

        std::unreachable();
    }

    const auto& instruction = decoder_result.value();

    const std::uint32_t next_pc = current_pc + 4;

    ExecutionContext context{
        .registers = registers,
        .memory = memory,
        .pc = current_pc,
        .next_pc = next_pc
    };

    const bool trace_register =
        writes_register(instruction.operation) && instruction.rd != 0;

    const std::uint32_t old_register_value =
        trace_register ? registers.read(instruction.rd) : 0;

    std::array<std::uint8_t, 4> old_memory_bytes{};
    std::uint32_t memory_address{};

    std::size_t memory_width{};
    bool trace_memory = false;

    if (instruction.operation == InstructionOperation::SB ||
        instruction.operation == InstructionOperation::SW)
    {
        memory_address =
            registers.read(instruction.rs1) +
            static_cast<std::uint32_t>(instruction.immediate);

        memory_width =
            instruction.operation == InstructionOperation::SW ? 4 : 1;

        trace_memory = true;

        for (std::size_t i = 0; i < memory_width; ++i) {
            const auto result = read_memory(memory_address + i);

            if (!result) {
                trace_memory = false;
                break;
            }

            old_memory_bytes[i] = result.value();
        }
    }

    const auto executor_result = execute(instruction, context);

    if (!executor_result) {
        return std::unexpected(executor_result.error());
    }

    const auto execution_outcome = executor_result.value();

    std::cout << "[TRACE] PC=0x"
              << std::hex << std::uppercase << std::setfill('0')
              << std::setw(8) << current_pc
              << " WORD=0x" << std::setw(8) << instruction_word
              << " MNEMONIC=" << instruction_name(instruction.operation)
              << '\n';

    if (trace_register) {
        const std::uint32_t new_value = registers.read(instruction.rd);

        if (old_register_value != new_value) {
            std::cout << "        REG x"
                      << std::dec << static_cast<unsigned>(instruction.rd)
                      << ": 0x"
                      << std::hex << std::uppercase << std::setfill('0')
                      << std::setw(8) << old_register_value
                      << " -> 0x" << std::setw(8) << new_value
                      << '\n';
        }
    }

    if (trace_memory) {
        for (std::size_t i = 0; i < memory_width; ++i) {
            const auto result = read_memory(memory_address + i);

            if (result && old_memory_bytes[i] != result.value()) {
                std::cout << "        MEM [0x"
                          << std::hex << std::uppercase << std::setfill('0')
                          << std::setw(8) << memory_address + i
                          << "]: 0x" << std::setw(2)
                          << static_cast<unsigned>(old_memory_bytes[i])
                          << " -> 0x" << std::setw(2)
                          << static_cast<unsigned>(result.value())
                          << '\n';
            }
        }
    }

    if (execution_outcome == ExecutionOutcome::HALT) {
        std::cout << "        HALT: Simulator halt\n\n";
        return ExecutionOutcome::HALT;
    }

    switch (instruction.operation) {
        case InstructionOperation::BEQ:
        case InstructionOperation::BNE:
        case InstructionOperation::BLT:
        case InstructionOperation::BGE:
        case InstructionOperation::JAL:
        case InstructionOperation::JALR:
            std::cout << "        NEXT_PC: 0x"
                      << std::hex << std::uppercase << std::setfill('0')
                      << std::setw(8) << context.next_pc
                      << '\n';
            break;

        default:
            break;
    }

    std::cout << '\n';

    pc = context.next_pc;

    return ExecutionOutcome::CONTINUE;
}

std::expected<void, ExecutionError> CPU::run() noexcept
{
    while (true) {
        const auto result = step();

        if (!result) {
            return std::unexpected(result.error());
        }

        const auto execution_outcome = result.value();
        if (execution_outcome == ExecutionOutcome::HALT) {
            return {};
        }

    }

    return {};
}

std::uint32_t CPU::read_register(std::size_t register_index) const noexcept
{
    return registers.read(register_index);
}

std::expected<std::uint8_t, MemoryError>
CPU::read_memory(std::uint32_t address) const noexcept
{
    return memory.read_byte(address);
}

}

