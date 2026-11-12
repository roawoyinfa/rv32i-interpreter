#include "support/test_utils.hpp"

#include <iostream>

int main()
{
    std::cout << "[TEST] Running branch integration test...\n";

    auto cpu = test::load_and_run_program("tests/programs/branches.hex");

    // Verify the initial register values used by the branch tests.
    test::check_register(cpu, 1, 0x00000005);
    test::check_register(cpu, 2, 0x00000005);

    // Verify the result of the branch sequence.
    // The final value confirms that the executed paths updated x3 as expected.
    test::check_register(cpu, 3, 0x00000006);

    // Verify that the negative immediate loaded into x4 is sign-extended.
    test::check_register(cpu, 4, 0xFFFFFFFF);

    std::cout << "[PASS] Branch integration test passed.\n";
}

