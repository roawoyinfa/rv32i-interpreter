#include "rv32i/loader.hpp"
#include "rv32i/program.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

namespace rv32i {

namespace {

std::string trim(const std::string& str)
{
    const std::string whitespace = " \t\n\r\f\v";

    const std::size_t first = str.find_first_not_of(whitespace);
    if (first == std::string::npos) {
        return "";
    }

    const std::size_t last = str.find_last_not_of(whitespace);

    return str.substr(first, last - first + 1);
}

}

std::expected<Program, LoaderError>
load_program(const std::filesystem::path& path)
{
    std::ifstream file(path);

    if (!file.is_open()) {
        return std::unexpected(LoaderError::FILE_NOT_FOUND);
    }

    Program program;
    std::string line;

    constexpr std::size_t expected_word_width = 8;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty()) {
            continue;
        }

        if (line.length() != expected_word_width) {
            return std::unexpected(LoaderError::INVALID_WORD_WIDTH);
        }

        const bool is_valid_hex = std::all_of(
            line.begin(),
            line.end(),
            [](unsigned char c) {
                return std::isxdigit(c);
            });

        if (!is_valid_hex) {
            return std::unexpected(LoaderError::INVALID_HEX_CHARACTER);
        }

        std::uint32_t instruction;

        const auto [ptr, ec] = std::from_chars(line.data(),
                                               line.data() + line.size(),
                                               instruction, 16);

        if (ec != std::errc{} || ptr != line.data() + line.size()) {
            return std:unexpected(LoaderError::PARSING_ERROR);
        }

        program.instructions.push_back(instruction);
    }

    if (file.bad()) {
        return std::unexpected(LoaderError::FILE_READ_ERROR);
    }

    return program;
}

}

