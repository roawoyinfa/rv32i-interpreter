#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <cstdint>
#include <expected>

#include "execution_error.h"
#include "instruction.h"
#include "memory.h"
#include "register_file.h"

namespace rv32i {

struct ExecutionContext {
    RegisterFile& registers;
    Memory& memory;
    std::uint32_t pc;
    std::uint32_t next_pc;
};

[[nodiscard]]
std::expected<void, ExecutionError>
execute(const DecodedInstruction& instruction, ExecutionContext& context);

}

#endif

