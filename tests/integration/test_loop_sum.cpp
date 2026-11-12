#include "support/test_utils.hpp"

int main()
{
    std::cout << "[TEST] Running loop sum integration test...\n";

    auto cpu = test::load_and_run_program("tests/programs/loop_sum.hex");

    // x1 is incremented once per iteration, ending at 11 after ten iterations.
    // test::check_register(cpu, 1, 0x0000000B);

    // Verify the accumulated sum: 1 + 2 + ... + 10 = 55.
    test::check_register(cpu, 2, 0x00000037);

    // Verify that the loop counter reached its termination value.
    test::check_register(cpu, 3, 0x0000000B);

    std::cout << "[PASS] Loop sum integration test passed.\n";
}

