#ifndef RV32I_DECODER_HPP
#define RV32I_DECODER_HPP

#include <cstdint>
#include <expected>

#include "rv32i/instruction.hpp"

namespace rv32i {

enum class DecoderError {
    UNSUPPORTED_INSTRUCTION,
    INVALID_ENCODING
};

[[nodiscard]]
std::expected<DecodedInstruction, DecoderError>
decode(std::uint32_t instruction);

}

#endif

