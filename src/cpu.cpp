#include "cpu.h"
#include "executor.h"
#include "execution_error.h"
#include "decoder.h"
#include "memory.h"

#include <utility>

namespace rv32i {

std::expected<void, ExecutionError> CPU::step()
{
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

    pc = context.next_pc;

    return {};
}

std::expected<void, ExecutionError> CPU::run()
{
    while (true) {
        const auto result = step();

        if (!result) {
            return std::unexpected(result.error());
        }
    }

    return {};
}

}

