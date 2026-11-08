#include "rv32i/cpu.hpp"
#include "rv32i/executor.hpp"
#include "rv32i/execution_error.hpp"
#include "rv32i/decoder.hpp"
#include "rv32i/program.hpp"
#include "rv32i/memory.hpp"

#include <cstddef>
#include <expected>
#include <utility>

namespace rv32i {

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

std::expected<ExecutionOutcome, ExecutionError> CPU::step()
{
    if (pc >= program_end)
    {
        return std::unexpected(ExecutionError::EXECUTION_OUTSIDE_PROGRAM);
    }

    const auto fetch_result = memory.read_word(pc);

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

    const auto decoder_result = decode(fetch_result.value());

    if (!decoder_result) {
        switch (decoder_result.error()) {
            case DecoderError::UNSUPPORTED_INSTRUCTION:
                return std::unexpected(ExecutionError::UNSUPPORTED_INSTRUCTION);

            case DecoderError::INVALID_ENCODING:
                return std::unexpected(ExecutionError::INVALID_INSTRUCTION);
        }

        std::unreachable();
    }

    const std::uint32_t next_pc = pc + 4;

    ExecutionContext context{
        .registers = registers,
        .memory = memory,
        .pc = pc,
        .next_pc = next_pc
    };

    const auto executor_result = execute(decoder_result.value(), context);

    if (!executor_result) {
        return std::unexpected(executor_result.error());
    }

    const auto execution_outcome = executor_result.value();
    if (execution_outcome == ExecutionOutcome::HALT) {
        return ExecutionOutcome::HALT;
    }

    pc = context.next_pc;

    return ExecutionOutcome::CONTINUE;
}

std::expected<void, ExecutionError> CPU::run()
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

}

