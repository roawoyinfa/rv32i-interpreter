#include "register_file.h"

#include <cassert>
#include <cstddef>

namespace rv32i {

std::uint32_t RegisterFile::read(std::size_t register_index) const noexcept
{
    assert(register_index < REGISTER_COUNT);

    if (register_index == 0) {
        return 0;
    }

    return registers_[register_index];
}

void RegisterFile::write(std::size_t register_index,
                         std::uint32_t value) noexcept
{
    assert(register_index < REGISTER_COUNT);

    if (register_index == 0) {
        return;
    }

    registers_[register_index] = value;
}

}

