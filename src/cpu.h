#ifndef CPU_H
#define CPU_H

#include "execution_error.h"
#include "register_file.h"
#include "memory.h"

#include <cstdint>
#include <expected>

namespace rv32i {

class CPU {
public:
    explicit CPU() noexcept;

    CPU(const CPU&) = delete;
    CPU& operator=(const CPU&) = delete;

    CPU(CPU&&) = delete;
    CPU& operator=(CPU&&) = delete;

    [[nodiscard]] std::expected<void, ExecutionError> run();

    [[nodiscard]] std::expected<void, ExecutionError> step();

private:
    RegisterFile registers;
    Memory memory;
    std::uint32_t pc;
};

}

#endif

