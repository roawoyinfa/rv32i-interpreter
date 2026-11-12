#include "support/test_utils.hpp"

#include <iostream>

int main()
{
    std::cout << "[TEST] Running shift integration test...\n";

    auto cpu = test::load_and_run_program("tests/programs/shifts.hex");

    // Initial value: 0x80000000.
    test::check_register(cpu, 1, 0x80000000);

    // SLL: 1 << 1 = 2.
    test::check_register(cpu, 3, 0x00000002);

    // SRL: logical right shift fills vacated bits with zeros.
    test::check_register(cpu, 4, 0x40000000);

    // SRA: arithmetic right shift preserves the sign bit.
    test::check_register(cpu, 5, 0xC0000000);

    // Shift amount: 33.
    test::check_register(cpu, 6, 0x00000021);

    // SLL: only the lower five bits of the shift amount are used.
    // 33 & 0x1F = 1, so 1 << 1 = 2.
    test::check_register(cpu, 7, 0x00000002);

    std::cout << "[PASS] Shift integration test passed.\n";
}

