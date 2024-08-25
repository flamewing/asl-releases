/* aplfloat.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* APPLE<->IEEE Floating Point Conversion on host                            */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "aplfloat.h"

#include "as_float.h"
#include "asmerr.h"
#include "errmsg.h"
#include "strcomp.h"

#include <errno.h>
#include <string.h>

/*!------------------------------------------------------------------------
 * \fn     as_float_2_apl4(as_float_t inp, Word *p_dest)
 * \brief  convert from host to Apple II 4 byte float format
 * \param  inp value to dispose
 * \param  p_dest where to dispose
 * \return 0 or error code (<0)
 * ------------------------------------------------------------------------ */

int as_float_2_apl4(as_float_t inp, Word* p_dest) {
    Boolean            round_up;
    as_float_dissect_t dissect;

    /* Dissect number: */

    as_float_dissect(&dissect, inp);

    /* NaN and Infinity cannot be represented: */

    if ((dissect.fp_class == AS_FP_NAN) || (dissect.fp_class == AS_FP_INFINITE)) {
        return -EINVAL;
    }

    /* (3) Denormalize small numbers: */

    while ((dissect.exponent < -128) && !as_float_mantissa_is_zero(&dissect)) {
        dissect.exponent++;
        as_float_mantissa_shift_right(dissect.mantissa, 0, dissect.mantissa_bits);
    }

    /* Build Two's complement.  Note the sign becomes part of the
       mantissa, which is afterwards one bit longer: */

    as_float_mantissa_twos_complement(&dissect);

    /* Normalize, so that topmost bits of mantissa are unequal.  This happens
       for powers of two, after negating: */

    switch (as_float_mantissa_extract(&dissect, 0, 2)) {
    case 0:
    case 3:
        if (dissect.exponent > -128) {
            dissect.exponent--;
            as_float_mantissa_shift_left(dissect.mantissa, 0, dissect.mantissa_bits);
        }
        break;
    }

    /* (4) Round mantissa.  We will use 24 of them.  So the "half LSB" bit
           to look at is bit 24, seen from left: */

    if (as_float_get_mantissa_bit(dissect.mantissa, dissect.mantissa_bits, 24)) {
        if (!as_float_mantissa_is_zero_from(&dissect, 25)) { /* > 0.5 */
            round_up = True;
        } else { /* == 0.5 */
            round_up = as_float_get_mantissa_bit(
                    dissect.mantissa, dissect.mantissa_bits, 23);
        }
    } else { /* < 0.5 */
        round_up = False;
    }

    if (round_up) {
        as_float_mant_t round_sum;

        (void)as_float_mantissa_add_bit(
                round_sum, dissect.mantissa, 24, dissect.mantissa_bits);

        /* overflow during round-up? */

        if ((round_sum[0] ^ dissect.mantissa[0]) & 0x80000000ul) {
            dissect.exponent++;
            /* Arithmetic right shift of signed number to preserve sign: */
            as_float_mantissa_shift_right(
                    round_sum,
                    as_float_get_mantissa_bit(dissect.mantissa, dissect.mantissa_bits, 0),
                    dissect.mantissa_bits);
        }

        memcpy(dissect.mantissa, round_sum, sizeof(dissect.mantissa));
    }

    /* After knowing final exponent, check for overflow: */

    if (dissect.exponent > 127) {
        return -E2BIG;
    }

    /* (5) mantissa zero means exponent is also zero */

    if (as_float_mantissa_is_zero(&dissect)) {
        dissect.exponent = 0;
    }

    /* (7) Assemble: */

    p_dest[0] = (((dissect.exponent + 128) << 8) & 0xff00ul)
                | as_float_mantissa_extract(&dissect, 0, 8);
    p_dest[1] = as_float_mantissa_extract(&dissect, 8, 16);

    return 0;
}
