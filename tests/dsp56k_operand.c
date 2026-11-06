/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
/* Exercise the private decoder directly: output alone does not reveal that
   the shared operand was left truncated. Keep its registration symbol local
   to this test's copy of the backend. */
#define code56k_init test_code56k_init
#include "../code56k.c"

int main(void) {
    static tStrComp arguments[2];
    static LongWord code[1];
    static char     operand[16];
    char            original[16];

    strutil_init();
    ArgStr   = arguments;
    DAsmCode = code;
    CPU56300 = MomCPU      = 2;
    arguments[1].str.p_str = operand;
    for (unsigned condition = 0; condition < CondCount; condition++) {
        for (unsigned unextended = 0; unextended < 2; unextended++) {
            Word encoded_condition;
            char name[3];
            strcpy(name, CondNames[condition]);
            if (!DecodeCondition(name, &encoded_condition)) {
                return 1;
            }
            memset(operand, 0xa5, sizeof(operand));
            as_snprintf(operand, sizeof(operand), "IF%s%s", name, unextended ? ".U" : "");
            memcpy(original, operand, sizeof(operand));
            CodeLen = 0;
            code[0] = 0;
            if (!DecodeMOVE_1(1) || CodeLen != 1
                || code[0]
                           != (0x202000u + (encoded_condition << 8)
                               + (unextended ? 0x1000u : 0))
                || memcmp(operand, original, sizeof(operand))) {
                fprintf(stderr, "IF operand or encoding changed for %s\n", original);
                return 1;
            }
        }
    }
    return 0;
}
