#ifndef TESTS_SUPPORT_TEST_UTILS_HPP
#define TESTS_SUPPORT_TEST_UTILS_HPP

#include "rv32i/cpu.hpp"
#include "rv32i/execution_error.hpp"
#include "rv32i/loader.hpp"
#include "rv32i/memory.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <utility>

namespace test {

[[nodiscard]]
rv32i::CPU load_and_run_program(const std::filesystem::path& path)
{
    auto load_result = rv32i::load_program(path);

    if (!load_result) {
        switch (load_result.error()) {
            case rv32i::LoaderError::FILE_NOT_FOUND:
                std::cerr << "Error: cannot open input file: "
                          << path << '\n';
                break;

            case rv32i::LoaderError::FILE_READ_ERROR:
                std::cerr << "Error: failed while reading input file: "
                          << path << '\n';
                break;

            case rv32i::LoaderError::INVALID_WORD_WIDTH:
                std::cerr << "Error: invalid instruction word width. "
                          << "Expected 8 hexadecimal digits per line.\n";
                break;

            case rv32i::LoaderError::INVALID_HEX_CHARACTER:
                std::cerr << "Error: input contains an invalid "
                          << "hexadecimal character.\n";
                break;

            case rv32i::LoaderError::PARSING_ERROR:
                std::cerr << "Error: failed to parse an instruction word.\n";
                break;
        }

        std::exit(EXIT_FAILURE);
    }

    auto cpu_result = rv32i::CPU::create(std::move(load_result).value());

    if (!cpu_result) {
        switch (cpu_result.error()) {
            case rv32i::CPUError::PROGRAM_TOO_LARGE:
                std::cerr << "Error: program exceeds available memory.\n";
                break;

            case rv32i::CPUError::MEMORY_INITIALIZATION_FAILED:
                std::cerr << "Error: failed to initialize program memory.\n";
                break;
        }

        std::exit(EXIT_FAILURE);
    }

    auto cpu = std::move(cpu_result).value();

    const auto run_result = cpu.run();

    if (!run_result) {
        switch (run_result.error()) {
            case rv32i::ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS:
                std::cerr << "Execution error: instruction address "
                          << "outside available memory.\n";
                break;

            case rv32i::ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED:
                std::cerr << "Execution error: misaligned instruction address.\n";
                break;

            case rv32i::ExecutionError::MEMORY_ACCESS_OUT_OF_BOUNDS:
                std::cerr << "Execution error: data memory access "
                          << "outside available memory.\n";
                break;

            case rv32i::ExecutionError::MEMORY_ACCESS_MISALIGNED:
                std::cerr << "Execution error: misaligned data memory access.\n";
                break;

            case rv32i::ExecutionError::UNSUPPORTED_INSTRUCTION:
                std::cerr << "Execution error: unsupported instruction.\n";
                break;

            case rv32i::ExecutionError::INVALID_INSTRUCTION:
                std::cerr << "Execution error: invalid instruction encoding.\n";
                break;

            case rv32i::ExecutionError::EXECUTION_OUTSIDE_PROGRAM:
                std::cerr << "Execution error: control flow left the loaded "
                          << "program region.\n";
                break;
        }

        std::exit(EXIT_FAILURE);
    }

    return cpu;
}

void check_register(const rv32i::CPU& cpu, std::size_t index,
                    std::uint32_t expected)
{
    const auto actual = cpu.read_register(index);

    if (actual != expected) {
        std::cerr << "Register x" << static_cast<unsigned>(index)
                  << ": expected 0x" << std::hex << expected
                  << ", got 0x" << actual << '\n';

        std::exit(EXIT_FAILURE);
    }
}

void check_memory(const rv32i::CPU& cpu, std::uint32_t address,
                  std::uint8_t expected)
{
    const auto actual = cpu.read_memory(address);

    if (!actual || actual.value() != expected) {
        std::cerr << "Memory byte at 0x" << std::hex << address
                  << ": expected 0x" << static_cast<unsigned>(expected);

        if (actual) {
            std::cerr << ", got 0x"
                      << static_cast<unsigned>(actual.value());
        } else {
            std::cerr << ", memory read failed";
        }

        std::cerr << '\n';
        std::exit(EXIT_FAILURE);
    }
}

}

#endif

