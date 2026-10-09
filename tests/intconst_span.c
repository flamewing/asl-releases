/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "intconst_core.h"

#include <stdio.h>

Boolean force_restricted_extension_digits = False;
PTransTable CurrTransTable;
static unsigned translation_reads, unmapped_errors;

void TransTableCheckRead(void) { translation_reads++; }
void WrError(tErrorNum number) {
    if (number == ErrNum_UnmappedChar) unmapped_errors++;
}
/* Controlled translation dependency: the parser must forward each bounded byte. */
int as_chartrans_xlate(as_chartrans_table_t const* table, unsigned byte) {
    (void)table;
    return byte == '!' ? -1 : (int)byte;
}

static int check(char const* data, size_t len, Boolean valid, LargeInt value,
                 int range, unsigned bits, tSymbolFlags flags) {
    LargeInt parsed = 999;
    Byte parsed_bits = 77;
    tSymbolFlags parsed_flags = eSymbolFlag_None;
    int parsed_range = 99;
    Boolean result = const_int_val_ref((as_strref_t){data, len}, &parsed,
                                      &parsed_flags, &parsed_bits, &parsed_range);
    if (result != valid || parsed != value || parsed_range != range
        || (valid && len && !range
            && (parsed_bits != bits || parsed_flags != flags))) {
        fprintf(stderr, "Integer span mismatch, length %zu\n", len);
        return 1;
    }
    return 0;
}

int main(void) {
    /* None of these arrays needs a terminator at the end of the view. */
    static char const decimal[] = {'-', '1', '2', '3', '9'};
    static char const hexadecimal[] = {'0', 'x', 'a', 'f', 'z'};
    static char const intel[] = {'0', 'a', 'f', 'h', 'z'};
    static char const motorola[] = {'$', 'a', 'f', 'z'};
    static char const binary[] = {'%', '1', '0', '1', '2'};
    static char const ibm[] = {'a', '\'', 'A', '!', '\'', 'z'};
    static char const plus[] = {'+', '9'};
    TTransTable table;
    char overflow[LARGEBITS / 4 + 3];
    int failed = 0;
    memset(&table, 0, sizeof(table));
    CurrTransTable = &table;
    strutil_init();
    intformat_init();
    RadixBase = 10;
    SetIntConstMode(eIntConstModeC);
    failed |= check(NULL, 0, True, 0, 0, 0, eSymbolFlag_None);
    failed |= check(plus, 1, True, 0, 0, 77, eSymbolFlag_None);
    failed |= check(decimal, 4, True, -123, 0, (Byte)-1, eSymbolFlag_Negative);
    failed |= check(hexadecimal, 4, True, 175, 0, 0, eSymbolFlag_None);
    failed |= check(hexadecimal, 5, False, -1, 0, 0, eSymbolFlag_None);
    force_restricted_extension_digits = True;
    SetIntConstMode(eIntConstModeIntel);
    failed |= check(intel, 4, True, 175, 0, 8, eSymbolFlag_None);
    memset(overflow, 'f', sizeof(overflow));
    overflow[0] = '0';
    overflow[sizeof(overflow) - 1] = 'h';
    failed |= check(overflow, sizeof(overflow), True, -1, 1, 0, eSymbolFlag_None);
    SetIntConstMode(eIntConstModeMoto);
    failed |= check(motorola, 3, True, 175, 0, 8, eSymbolFlag_None);
    failed |= check(binary, 4, True, 5, 0, 3, eSymbolFlag_None);
    SetIntConstMode(eIntConstModeIBM);
    ModifyIntConstModeByMask(0, 1ul << eIntFormatIBMAsc);
    failed |= check(ibm, 5, True, 0x4100, 0, 16, eSymbolFlag_None);
    failed |= translation_reads != 1 || unmapped_errors != 1;
    return failed;
}
