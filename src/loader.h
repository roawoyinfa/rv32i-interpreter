#ifndef LOADER_H
#define LOADER_H

#include <expected>
#include <filesystem>

#include "program.h"

namespace rv32i {

enum class LoaderError {
    FILE_NOT_FOUND,
    FILE_READ_ERROR,
    INVALID_WORD_WIDTH,
    INVALID_HEX_CHARACTER,
    PARSING_ERROR
};

[[nodiscard]]
std::expected<Program, LoaderError>
load_program(const std::filesystem::path& path);

}


#endif

