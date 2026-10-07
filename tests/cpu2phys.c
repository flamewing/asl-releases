/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"

#include "cpu2phys.h"

#include "asmdef.h"
#include "asmsub.h"

/* State needed by the expression wrappers; mapping tests use the public API. */
as_addrspace_t ActPC;
LargeWord*     SegLimits;

/* The assembler-only string formatter is outside this mapping test. */
void KillBlanks(char* str) {
    UNUSED(str);
    as_abort("Unexpected call to the assembler string formatter\n");
}

static int check(Boolean reverse, LargeWord input, Boolean found, LargeWord expected) {
    LargeWord actual = input;
    Boolean   result = reverse ? def_phys_2_cpu(SegCode, &actual)
                               : def_cpu_2_phys(SegCode, &actual);
    if (result != found || actual != expected) {
        fprintf(stderr, "Incorrect %s translation at %lx\n",
                reverse ? "reverse" : "forward", (unsigned long)input);
        return 1;
    }
    return 0;
}

static int overlap(LargeWord start, LargeWord length) {
    LargeWord cpu;
    cpu_2_phys_area_clear(SegCode);
    cpu_2_phys_area_add(SegCode, 0x100, 0x1000, 0x100);
    cpu_2_phys_area_add(SegCode, start, 0x2000, length);
    for (cpu = 0; cpu < 0x400; cpu++) {
        Boolean   newer    = cpu >= start && cpu < start + length;
        Boolean   old      = cpu >= 0x100 && cpu < 0x200;
        LargeWord physical = newer ? cpu - start + 0x2000
                             : old ? cpu - 0x100 + 0x1000
                                   : cpu;
        if (check(False, cpu, newer || old, physical)) {
            return 1;
        }
        if ((newer || old) && check(True, physical, True, cpu)) {
            return 1;
        }
        if (old && newer
            && check(True, cpu - 0x100 + 0x1000, False, cpu - 0x100 + 0x1000)) {
            return 1;
        }
    }
    /* Zero-length insertion must leave the map untouched. */
    cpu_2_phys_area_add(SegCode, 0x100, 0x3000, 0);
    return check(False, start, True, 0x2000);
}

int main(void) {
    if (overlap(0x1ff, 0x100) /* shared end address */
        || overlap(1, 0x100)  /* shared start address */
        || overlap(0x100, 1) || overlap(0x1ff, 1)
        || overlap(0x180, 1)     /* split around one replaced address */
        || overlap(0x100, 0x100) /* complete replacement */
        || overlap(0x200, 1) /* adjacent ranges do not overlap */) {
        return 1;
    }
    cpu_2_phys_area_clear(SegCode);
    cpu_2_phys_area_add(SegCode, 0x180, 0x1000, 1);
    cpu_2_phys_area_add(SegCode, 0x100, 0x2000, 0x100);
    if (check(False, 0x180, True, 0x2080) || check(True, 0x1000, False, 0x1000)) {
        return 1;
    }
    cpu_2_phys_area_fill(SegCode, 0, 0x3ff);
    return check(False, 0xff, True, 0xff) || check(False, 0x180, True, 0x2080)
           || check(False, 0x200, True, 0x200);
}
