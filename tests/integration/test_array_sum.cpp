#include "support/test_utils.hpp"

#include <iostream>

int main()
{
    std::cout << "[TEST] Running array sum integration test...\n";

    auto cpu = test::load_and_run_program("tests/programs/array_sum.hex");

    // Verify the array base address.
    test::check_register(cpu, 10, 0x00000100);

    // Verify the last value loaded and stored in the source register.
    test::check_register(cpu, 1, 0x00000032);

    // Verify the pointer advanced past the final array element.
    test::check_register(cpu, 11, 0x00000114);

    // Verify the loop counter reached zero.
    test::check_register(cpu, 12, 0x00000000);

    // Verify the accumulated sum: 10 + 20 + 30 + 40 + 50 = 150.
    test::check_register(cpu, 13, 0x00000096);

    // Verify the final value loaded from the array.
    test::check_register(cpu, 14, 0x00000032);

    // Verify the first array element (10).
    test::check_memory(cpu, 0x100, 0x0A);
    test::check_memory(cpu, 0x101, 0x00);
    test::check_memory(cpu, 0x102, 0x00);
    test::check_memory(cpu, 0x103, 0x00);

    // Verify the second array element (20).
    test::check_memory(cpu, 0x104, 0x14);
    test::check_memory(cpu, 0x105, 0x00);
    test::check_memory(cpu, 0x106, 0x00);
    test::check_memory(cpu, 0x107, 0x00);

    // Verify the third array element (30).
    test::check_memory(cpu, 0x108, 0x1E);
    test::check_memory(cpu, 0x109, 0x00);
    test::check_memory(cpu, 0x10A, 0x00);
    test::check_memory(cpu, 0x10B, 0x00);

    // Verify the fourth array element (40).
    test::check_memory(cpu, 0x10C, 0x28);
    test::check_memory(cpu, 0x10D, 0x00);
    test::check_memory(cpu, 0x10E, 0x00);
    test::check_memory(cpu, 0x10F, 0x00);

    // Verify the fifth array element (50).
    test::check_memory(cpu, 0x110, 0x32);
    test::check_memory(cpu, 0x111, 0x00);
    test::check_memory(cpu, 0x112, 0x00);
    test::check_memory(cpu, 0x113, 0x00);

    std::cout << "[PASS] Array sum integration test passed.\n";
}

