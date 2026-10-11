/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"
#include "tempresult.h"
#include <math.h>
#include <string.h>

/* The assembler-only string formatter is outside this conversion test. */
void KillBlanks(char* str) {
    UNUSED(str);
    as_abort("Unexpected call to the assembler string formatter\n");
}

static int check_int(LargeInt value) {
    TempResult result;
    as_tempres_ini(&result);
    as_tempres_set_int(&result, value);
    result.Flags = (tSymbolFlags)0x55;
    result.AddrSpaceMask = 3;
    result.DataSize = eSymbolSize16Bit;
    result.Relocs = (struct sRelocEntry*)&result;
    as_float_t expected =
#ifdef AS_HAS128
            (QuadInt)
#endif
                    value;
    return TempResultToFloat(&result) != 0 || result.Typ != TempFloat
           || result.Contents.Float != expected
           || result.Flags != (tSymbolFlags)0x55 || result.AddrSpaceMask != 3
           || result.DataSize != eSymbolSize16Bit
           || result.Relocs != (struct sRelocEntry*)&result;
}

int main(void) {
    LargeInt const integers[] = {0, 1, -1, 42, -42, MaxLongInt, -MaxLongInt - 1,
#ifdef AS_HAS64
        ((LargeInt)1 << 53) - 1, ((LargeInt)1 << 53) + 1,
        (LargeInt)INT64_MAX, (LargeInt)INT64_MIN,
#endif
#ifdef AS_HAS128
        (LargeInt)1 << 80, -((LargeInt)1 << 80),
#endif
    };
    for (size_t i = 0; i < sizeof(integers) / sizeof(*integers); i++) {
        if (check_int(integers[i])) {
            return 1;
        }
    }
    as_float_t const floats[] = {1.5, -0.0, INFINITY, -INFINITY, NAN};
    for (size_t i = 0; i < sizeof(floats) / sizeof(*floats); i++) {
        TempResult result, before;
        as_tempres_ini(&result);
        as_tempres_set_float(&result, floats[i]);
        memcpy(&before, &result, sizeof(before));
        if (TempResultToFloat(&result) != 0
            || memcmp(&before, &result, sizeof(before))) {
            return 1;
        }
    }
    TempResult result;
    as_tempres_ini(&result);
    if (TempResultToFloat(&result) != -1 || result.Typ != TempNone) {
        return 1;
    }
    as_tempres_set_c_str(&result, "allocated string");
    if (!result.Contents.str.p_str || TempResultToFloat(&result) != -1
        || result.Typ != TempNone || result.Contents.str.p_str) {
        return 1;
    }
    tRegDescr reg = {0};
    as_tempres_set_reg(&result, &reg);
    return TempResultToFloat(&result) != -1 || result.Typ != TempNone;
}
