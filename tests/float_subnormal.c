/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#include "stdinc.h"

#include "as_float.h"
#include "bfloat.h"
#include "ieeefloat.h"

static int check16(
        int (*convert)(as_float_t, Byte*, Boolean), as_float_t value, unsigned expected) {
    unsigned sign, big;
    for (sign = 0; sign < 2; sign++) {
        for (big = 0; big < 2; big++) {
            Byte     bytes[4] = {0xa5, 0, 0, 0xa5};
            unsigned actual;
            if (convert(sign ? -value : value, bytes + 1, big) != 2) {
                return 1;
            }
            actual = ((unsigned)bytes[big ? 1 : 2] << 8) | bytes[big ? 2 : 1];
            if (actual != (expected | (sign << 15)) || bytes[0] != 0xa5
                || bytes[3] != 0xa5) {
                fprintf(stderr, "16-bit rounding: got %04x, expected %04x\n", actual,
                        expected | (sign << 15));
                return 1;
            }
        }
    }
    return 0;
}

static int check_subnormals(
        int (*convert)(as_float_t, Byte*, Boolean), unsigned normal, int exponent) {
    unsigned quarter;
    /* Exact binary quarter steps provide an independent nearest-even oracle. */
    for (quarter = 0; quarter <= 4 * normal + 3; quarter++) {
        unsigned expected = quarter / 4, fraction = quarter % 4;
        if (fraction > 2 || (fraction == 2 && (expected & 1))) {
            expected++;
        }
        if (check16(convert, as_ldexp((as_float_t)quarter, exponent - 2), expected)) {
            return 1;
        }
    }
    return check16(convert, as_ldexp((as_float_t)1, exponent - 20), 0);
}

static int check_precision(unsigned bits, Integer minimum) {
    as_float_dissect_t num;
    unsigned           i;
    /* 1.5 units of the smallest subnormal rounds to two units. */
    as_float_zero(&num);
    num.exponent = minimum - (Integer)(bits - 1);
    as_float_append_mantissa_bits(&num, 3, 2);
    as_float_round_ieee(&num, bits, minimum);
    if (num.exponent != minimum - 1 || as_float_mantissa_extract(&num, bits - 2, 2) != 2
        || !as_float_mantissa_is_zero_from(&num, bits)) {
        return 1;
    }
    /* Largest subnormal plus 0.75 unit carries into the smallest normal. */
    as_float_zero(&num);
    num.exponent = minimum;
    as_float_append_mantissa_bits(&num, 0, 1);
    for (i = 1; i < bits + 2; i++) {
        as_float_append_mantissa_bits(&num, 1, 1);
    }
    as_float_round_ieee(&num, bits, minimum);
    return num.exponent != minimum
           || !as_float_get_mantissa_bit(num.mantissa, num.mantissa_bits, 0)
           || !as_float_mantissa_is_zero_from(&num, 1);
}

int main(void) {
    if (check_subnormals(as_float_2_bfloat16, 128, -133)
        || check_subnormals(as_float_2_ieee2, 1024, -24)
        || check16(as_float_2_bfloat16, (as_float_t)1, 0x3f80)
        || check16(as_float_2_ieee2, (as_float_t)1, 0x3c00) || check_precision(8, -126)
        || check_precision(11, -14) || check_precision(24, -126)
        || check_precision(53, -1022) || check_precision(64, -16382)
        || check_precision(113, -16382)) {
        return 1;
    }
    return 0;
}
