#include "support/test_utils.hpp"

int main()
{
    std::cout << "[TEST] Running arithmetic integration test...\n";

    auto cpu = test::load_and_run_program("tests/programs/arithmetic.hex");

    // LUI: load 0x80000000 into x1.
    test::check_register(cpu, 1, 0x80000000);

    // ADDI: load -1 into x2.
    test::check_register(cpu, 2, 0xFFFFFFFF);

    // ADD: verify 32-bit wraparound (0x80000000 + 0xFFFFFFFF).
    test::check_register(cpu, 3, 0x7FFFFFFF);

    // SUB: verify subtraction (0 - (-1) = 1).
    test::check_register(cpu, 4, 0x00000001);

    // AND: 0xC & 0xA = 0x8.
    test::check_register(cpu, 7, 0x00000008);

    // OR: 0xC | 0xA = 0xE.
    test::check_register(cpu, 8, 0x0000000E);

    // XOR: 0xC ^ 0xA = 0x6.
    test::check_register(cpu, 9, 0x00000006);

    // SLT: signed comparison (-2147483648 < 0) evaluates to true.
    test::check_register(cpu, 10, 0x00000001);

    // Verify that an instruction producing zero writes zero correctly.
    test::check_register(cpu, 11, 0x00000000);

    std::cout << "[PASS] Arithmetic integration test passed.\n";
}

