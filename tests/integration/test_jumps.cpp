#include "support/test_utils.hpp"

#include <iostream>

int main()
{
    std::cout << "[TEST] Running jump integration test...\n";

    auto cpu = test::load_and_run_program("tests/programs/jumps.hex");

    // Verify the initial value used as the jump target calculation input.
    test::check_register(cpu, 1, 0x0000000A);

    // Verify that the function body executed.
    test::check_register(cpu, 2, 0x00000014);

    // jal stores the return address (address of the instruction after jal) in x5.
    test::check_register(cpu, 5, 0x00000008);

    // Verify that jalr returned to the instruction following jal.
    test::check_register(cpu, 6, 0x00000001);

    std::cout << "[PASS] Jump integration test passed.\n";
}

