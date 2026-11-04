#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <cstdint>
#include <expected>

#include "instruction.h"
#include "memory.h"
#include "register_file.h"

namespace rv32i {

struct ExecutionContext {
    RegisterFile& registers;
    Memory& memory;
    std::uint32_t& pc;
};

enum class ExecutionError {
    MEMORY_OUT_OF_BOUNDS,
    INSTRUCTION_ADDRESS_MISALIGNED,
    INSTRUCTION_ADDRESS_OUT_OF_BOUNDS
};

[[nodiscard]]
std::expected<void, ExecutionError>
execute(const DecodedInstruction& instruction, ExecutionContext& context);

}

#endif

