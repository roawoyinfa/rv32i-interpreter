#include "rv32i/cpu.h"
#include "rv32i/loader.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <utility>

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << "<program.hex>\n";
        return EXIT_FAILURE;
    }

    const std::filesystem::path path{argv[1]};

    if (path.empty()) {
        std::cout << "Error: input file path cannot be empty.\n";
        return EXIT_FAILURE;
    }

    const auto load_result = rv32i::load_program(path);

    if (!load_result) {
        switch (load_result.error()) {
            case rv32i::LoaderError::FILE_NOT_FOUND:
                std::cerr << "Error: cannot open input file: "
                          << path << '\n';
                return EXIT_FAILURE;

            case rv32i::LoaderError::FILE_READ_ERROR:
                std::cerr << "Error: failed while reading input file: "
                          << path << '\n';
                return EXIT_FAILURE;

            case rv32i::LoaderError::INVALID_WORD_WIDTH:
                std::cerr << "Error: invalid instruction word width. "
                          << "Expected 8 hexadecimal digits per line.\n";
                return EXIT_FAILURE;

            case rv32i::LoaderError::INVALID_HEX_CHARACTER:
                std::cerr << "Error: input contains an invalid "
                          << "hexadecimal character.\n";
                return EXIT_FAILURE;

            case rv32i::LoaderError::PARSING_ERROR:
                std::cerr << "Error: failed to parse an instruction word.\n";
                return EXIT_FAILURE;
        }

        std::unreachable();
    }

    const auto cpu_result = CPU::create(std::move(load_result).value());

    if (!cpu_result) {
        switch (cpu_result.error()) {
            case rv32i::CPUError::PROGRAM_TOO_LARGE:
                std::cerr << "Error: program exceeds available memory.\n";
                return EXIT_FAILURE;

            case rv32i::CPUError::MEMORY_INITIALIZATION_FAILED:
                std::cerr << "Error: failed to initialize program memory.\n";
                return EXIT_FAILURE;
        }

        std::unreachable();
    }

    const auto cpu = std::move(cpu_result).value();

    const auto run_result = cpu.run();

    if (!run_result) {
        switch (run_result.error()) {
            case rv32i::ExecutionError::INSTRUCTION_ADDRESS_OUT_OF_BOUNDS:
                std::cerr << "Execution error: instruction address "
                          << "outside available memory.\n";
                return EXIT_FAILURE;

            case rv32i::ExecutionError::INSTRUCTION_ADDRESS_MISALIGNED:
                std::cerr << "Execution error: misaligned instruction address.\n";
                return EXIT_FAILURE;

            case rv32i::ExecutionError::MEMORY_ACCESS_OUT_OF_BOUNDS:
                std::cerr << "Execution error: data memory access "
                          << "outside available memory.\n";
                return EXIT_FAILURE;

            case rv32i::ExecutionError::MEMORY_ACCESS_MISALIGNED:
                std::cerr << "Execution error: misaligned data memory access.\n";
                return EXIT_FAILURE;

            case rv32i::ExecutionError::UNSUPPORTED_INSTRUCTION:
                std::cerr << "Execution error: unsupported instruction.\n";
                return EXIT_FAILURE;

            case rv32i::ExecutionError::INVALID_INSTRUCTION:
                std::cerr << "Execution error: invalid instruction encoding.\n";
                return EXIT_FAILURE;

            case rv32i::ExecutionError::EXECUTION_OUTSIDE_PROGRAM:
                std::cerr << "Execution error: control flow left the loaded "
                          << "program region.\n";
                return EXIT_FAILURE;
            }

            std::unreachable();
        }
    }

    std::cout << "Program terminated successfully\n";
    return EXIT_SUCCESS;
}

