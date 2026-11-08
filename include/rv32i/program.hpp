#ifndef PROGRAM_H
#define PROGRAM_H

#include <cstdint>
#include <vector>

namespace rv32i {

struct Program {
    std::vector<std::uint32_t> instructions;
};

}

#endif

