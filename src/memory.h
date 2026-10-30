#ifndef MEMORY_H
#define MEMORY_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>

#include "memory_error.h"

namespace rv32i {

class Memory {
public:
    static constexpr std::size_t MEMORY_CAPACITY = 1024;

    Memory() = default;

    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

    Memory(Memory&&) = delete;
    Memory& operator=(Memory&&) = delete;

    [[nodiscard]]
    std::expected<std::uint8_t, MemoryError>
    read_byte(std::uint32_t address) const noexcept;

    [[nodiscard]]
    std::expected<std::uint32_t, MemoryError>
    read_word(std::uint32_t address) const noexcept;

    [[nodiscard]]
    std::expected<void, MemoryError>
    write_byte(std::uint32_t address, std::uint8_t value) noexcept;

    [[nodiscard]]
    std::expected<void, MemoryError>
    write_word(std::uint32_t address, std::uint32_t value) noexcept;

private:
    std::array<std::uint8_t, MEMORY_CAPACITY> bytes_{};
};

}

#endif

