/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "asl_library.h"

#include <stdio.h>
#include <string.h>

static int check_call(int expected, size_t count, char const* const args[]) {
    int result = asl_assemble((int)count, args);
    if (result != expected) {
        fprintf(stderr, "Expected %d, got %d for", expected, result);
        for (size_t index = 0; index < count; index++) {
            fprintf(stderr, " %s", args[index]);
        }
        fputc('\n', stderr);
        return 1;
    }
    return 0;
}

#define RUN(expected, ...)                                                  \
    do {                                                                    \
        char const* const args[] = {"asl-library-test", "-q", __VA_ARGS__}; \
        if (check_call(expected, sizeof(args) / sizeof(*args), args)) {     \
            return 1;                                                       \
        }                                                                   \
    } while (0)

int main(void) {
    if (asl_assemble(0, NULL) != 4) {
        return 1;
    }
    RUN(1, "-q");
    RUN(0, "-help");
    RUN(0, "-version");
    RUN(0, "-cpu", "list");
    RUN(4, "-codepage", "nonexistent-codepage");

    /* NLS may reorder its private argument copy; caller storage is immutable. */
    {
        char const* const args[] = {"asl-library-test", "-codepage", "utf-8", "-version"};
        char const*       original = args[1];
        if (check_call(0, sizeof(args) / sizeof(*args), args) || args[1] != original
            || strcmp(args[2], "utf-8")) {
            return 1;
        }
    }

    for (int iteration = 0; iteration < 4; iteration++) {
        RUN(0, "m68k.asm", "-o", "m68k.p");
        RUN(0, "z80.asm", "-o", "z80.p");
        RUN(0, "default.asm", "-cpu", "68000", "-o", "default.p");
        RUN(2, "default.asm", "-o", "no-default.p");
    }

    RUN(0, "definitions.asm", "-D", "FLAG=1,TEXT=\"hello\"", "-o", "defined.p");
    RUN(0, "definitions.asm", "-o", "definitions.p");
    RUN(0, "default.asm", "-alias", "MYCPU=68000", "-cpu", "MYCPU", "-o", "alias.p");
    RUN(4, "default.asm", "-cpu", "MYCPU");
    RUN(4, "-o", "stale.p", "-werror", "-radix", "8", "-compmode", "-packing",
        "-listline-prefix", "PREFIX", "-listing", "off", "-unknown-option");
    RUN(0, "m68k.asm", "-o", "after-options.p");
    RUN(2, "warning.asm", "-werror", "-o", "warning-error.p");
    RUN(0, "warning.asm", "-o", "warning.p");
    RUN(0, "utf8.asm", "-codepage", "utf-8", "-o", "utf8.p");
    RUN(0, "cp1252.asm", "-codepage", "1252", "-o", "cp1252.p");
    RUN(0, "utf8.asm", "-codepage", "utf-8", "-o", "utf8.p");

    /* Fatal failures return, release files/CPU state, and permit another call. */
    RUN(3, "m68k.asm", "-o", "missing-directory/output.p");
    RUN(0, "m68k.asm", "-o", "after-open-error.p");
    RUN(3, "include.asm", "-o", "include.p");
    RUN(0, "z80.asm", "-o", "after-include-error.p");
    RUN(3, "macro.asm", "-maxerrors", "1", "-o", "macro.p");
    RUN(0, "m68k.asm", "-o", "after-macro-error.p");
    RUN(3, "frames.asm", "-maxerrors", "1", "-o", "frames.p");
    RUN(0, "m68k.asm", "-o", "after-frames-error.p");
    RUN(3, "unfinished.asm", "-maxerrors", "1", "-o", "unfinished.p");
    RUN(0, "m68k.asm", "-o", "after-unfinished-error.p");
    RUN(3, "structure.asm", "-maxerrors", "1", "-o", "structure.p");
    RUN(0, "m68k.asm", "-o", "after-structure-error.p");
    RUN(3, "bracket.asm", "-maxerrors", "1", "-o", "bracket.p");
    RUN(0, "m68k.asm", "-o", "after-bracket-error.p");
    RUN(0, "-cpu", "list");
    return 0;
}
