#ifndef RV32I_PROGRAM_HPP
#define RV32I_PROGRAM_HPP

#include <cstdint>
#include <vector>

namespace rv32i {

struct Program {
    std::vector<std::uint32_t> instructions;
};

}

#endif

