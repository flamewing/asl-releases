/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "asl_library.h"

#include <stdlib.h>

int main(int argc, char** argv) {
    char const** args = (char const**)malloc((size_t)argc * sizeof(*args));
    int          result;
    if (!args) {
        return 3;
    }
    for (int index = 0; index < argc; index++) {
        args[index] = argv[index];
    }
    result = asl_assemble(argc, args);
    free(args);
    return result;
}
