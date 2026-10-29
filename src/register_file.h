#ifndef REGISTER_FILE_H
#define REGISTER_FILE_H

#include <array>
#include <cstdint>

namespace rv32i {

class RegisterFile {
public:
    static constexpr std::size_t register_count = 32;

    RegisterFile() = default;

    RegisterFile(const RegisterFile&) = delete;
    RegisterFile& operator=(const RegisterFile&) = delete;

    RegisterFile(RegisterFile&&) = delete;
    RegisterFile& operator=(RegisterFile&&) = delete;

    [[nodiscard]] std::uint32_t read(std::size_t register_index) const noexcept;

    void write(std::size_t register_index, std::uint32_t value) noexcept;

private:
    std::array<uint32_t, register_count> registers_{};
};

}

#endif

