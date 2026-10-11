/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#define main test_driver_program_main
#include "../test_driver.c"
#undef main

int main(int argc, char** argv) {
    if (argc != 2) {
        return 1;
    }
    return !compare_files(argv[1], argv[1], "empty")
           || !compare_files(argv[1], argv[1], "equal")
           || compare_files(argv[1], argv[1], "different")
           || compare_files(argv[1], argv[1], "missing");
}
