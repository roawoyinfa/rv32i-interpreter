#ifndef CPU_H
#define CPU_H

#include "execution_error.h"
#include "executor.h"
#include "program.h"
#include "register_file.h"
#include "memory.h"

#include <cstdint>
#include <expected>

namespace rv32i {

enum class CPUError {
    PROGRAM_TOO_LARGE,
    MEMORY_INITIALIZATION_FAILED
};

class CPU {
public:
    [[nodiscard]]
    static std::expected<CPU, CPUError> create(Program program) noexcept;

    CPU() = default;

    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;

    CPU(CPU&&) noexcept = default;
    CPU& operator=(CPU&&) = delete;

    [[nodiscard]] std::expected<void, ExecutionError> run();
    [[nodiscard]] std::expected<ExecutionOutcome, ExecutionError> step();

private:
    RegisterFile registers;
    Memory memory;
    std::uint32_t pc{};
};

}

#endif

