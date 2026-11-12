#ifndef RV32I_REGISTER_FILE_HPP
#define RV32I_REGISTER_FILE_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace rv32i {

class RegisterFile {
public:
    static constexpr std::size_t REGISTER_COUNT = 32;

    RegisterFile() = default;

    RegisterFile(const RegisterFile&) = delete;
    RegisterFile& operator=(const RegisterFile&) = delete;

    RegisterFile(RegisterFile&&) noexcept = default;
    RegisterFile& operator=(RegisterFile&&) = delete;

    [[nodiscard]] std::uint32_t read(std::size_t register_index) const noexcept;

    void write(std::size_t register_index, std::uint32_t value) noexcept;

private:
    std::array<uint32_t, REGISTER_COUNT> registers_{};
};

}

#endif

