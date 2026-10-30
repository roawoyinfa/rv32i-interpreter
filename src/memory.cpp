#include "memory.h"

#include <cstdint>
#include <expected>

namespace rv32i {

[[nodiscard]]
std::expected<std::uint8_t, MemoryError>
Memory::read_byte(std::uint32_t address) const noexcept
{
    if (address >= MEMORY_CAPACITY) {
        return std::unexpected(MemoryError::OutofBounds);
    }

    return bytes_[address];
}

[[nodiscard]]
std::expected<std::uint32_t, MemoryError>
Memory::read_word(std::uint32_t address) const noexcept
{
    constexpr std::size_t access_size = sizeof(std::uint32_t);

    if (address >= MEMORY_CAPACITY || access_size > MEMORY_CAPACITY - address) {
        return std::unexpected(MemoryError::OutofBounds);
    }

    std::uint32_t word = 0;

    for (std::size_t i = 0; i < access_size; ++i) {
        word |= static_cast<std::uint32_t>(bytes_[address]) << (i * 8);
    }

    return word;
}

[[nodiscard]]
std::expected<void, MemoryError>
Memory::write_byte(std::uint32_t address, std::uint8_t value) noexcept
{
    if (address >= MEMORY_CAPACITY) {
        return std::unexpected(MemoryError::OutofBounds);
    }

    bytes_[address] = value;
}

[[nodiscard]]
std::expected<void, MemoryError>
Memory::write_word(std::uint32_t address, std::uint32_t value) noexcept
{
    constexpr std::size_t access_size = sizeof(std::uint32_t);

    if (address >= MEMORY_CAPACITY || access_size > MEMORY_CAPACITY - address) {
        return std::unexpected(MemoryError::OutofBounds);
    }

    for (std::size_t i = 0; i < access_size; ++i) {
        bytes_[address] = static_cast<std::uint8_t>((value >> (i * 8)) & 0xFF);
    }
}

}

