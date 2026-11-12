#include "support/test_utils.hpp"

#include <iostream>

int main()
{
    std::cout << "[TEST] Running memory integration test...\n";

    auto cpu = test::load_and_run_program("tests/programs/memory.hex");

    // Verify the value constructed before storing it.
    test::check_register(cpu, 1, 0x12345678);

    // LW: load the complete 32-bit word from memory.
    test::check_register(cpu, 2, 0x12345678);

    // LB: load the first byte (0x78).
    test::check_register(cpu, 3, 0x00000078);

    // LB: load the fourth byte (0x12).
    // Since bit 7 is zero, sign extension leaves the value unchanged.
    test::check_register(cpu, 4, 0x00000012);

    // Value written to memory using SB.
    test::check_register(cpu, 5, 0x00000080);

    // LB: loading 0x80 sign-extends it to 0xFFFFFF80.
    test::check_register(cpu, 6, 0xFFFFFF80);

    // Verify that the neighbouring byte remains unchanged after SB.
    test::check_register(cpu, 7, 0x00000056);

    // Verify the little-endian representation of 0x12345678.
    test::check_memory(cpu, 0x100, 0x78);
    test::check_memory(cpu, 0x101, 0x56);
    test::check_memory(cpu, 0x102, 0x34);
    test::check_memory(cpu, 0x103, 0x12);

    // Verify the byte written by SB at address 0x104.
    test::check_memory(cpu, 0x104, 0x80);

    std::cout << "[PASS] Memory integration test passed.\n";
}

