#ifndef RV32I_EXECUTOR_HPP
#define RV32I_EXECUTOR_HPP

#include <cstdint>
#include <expected>

#include "rv32i/execution_error.hpp"
#include "rv32i/execution_outcome.hpp"
#include "rv32i/instruction.hpp"
#include "rv32i/memory.hpp"
#include "rv32i/register_file.hpp"

namespace rv32i {

struct ExecutionContext {
    RegisterFile& registers;
    Memory& memory;
    std::uint32_t pc;
    std::uint32_t next_pc;
};

[[nodiscard]]
std::expected<ExecutionOutcome, ExecutionError>
execute(const DecodedInstruction& instruction, ExecutionContext& context);

}

#endif

