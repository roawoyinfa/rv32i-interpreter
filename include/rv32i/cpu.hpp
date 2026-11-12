#ifndef CPU_H
#define CPU_H

#include "rv32i/execution_error.hpp"
#include "rv32i/execution_outcome.hpp"
#include "rv32i/program.hpp"
#include "rv32i/register_file.hpp"
#include "rv32i/memory.hpp"

#include <cstddef>
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

    [[nodiscard]] std::expected<void, ExecutionError> run() noexcept;

    [[nodiscard]]
    std::expected<ExecutionOutcome, ExecutionError> step() noexcept;

    [[nodiscard]]
    std::uint32_t read_register(std::size_t register_index) const noexcept;

    [[nodiscard]]
    std::expected<std::uint8_t, MemoryError>
    read_memory(std::uint32_t address) const noexcept;

private:
    RegisterFile registers;
    Memory memory;
    std::uint32_t pc{};
    std::uint32_t program_end{};
};

}

#endif

