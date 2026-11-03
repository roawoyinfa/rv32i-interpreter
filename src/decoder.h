#ifndef DECODER_H
#define DECODER_H

#include <cstdint>
#include <expected>

#include "instruction.h"

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

