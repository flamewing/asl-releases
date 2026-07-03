/* bfloat.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* Brain Floating Point Handling                                             */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "bfloat.h"

#include "as_endian.h"
#include "as_float.h"

#include <errno.h>
#include <float.h>
#include <math.h>
#include <string.h>

/*!------------------------------------------------------------------------
 * \fn     as_float_2_bfloat16(as_float_t inp, Byte *pDest, Boolean NeedsBig)
 * \brief  convert floating point number to bfloat16 format
 * \param  inp floating point number to store
 * \param  pDest where to write result (2 bytes)
 * \param  NeedsBig req's big endian?
 * \return >=0 if conversion was successful, <0 for error
 * ------------------------------------------------------------------------ */

/* Format is similar to IEEE FP16, however with 8 exponent and 7 mantissa bits: */

int as_float_2_bfloat16(as_float_t inp, Byte* pDest, Boolean NeedsBig) {
    as_float_dissect_t dissect;
    as_float_round_t   round_type;

    /* (1) Dissect number */

    as_float_dissect(&dissect, inp);

    /* Infinity/NaN: */

    if ((dissect.fp_class == AS_FP_NAN) || (dissect.fp_class == AS_FP_INFINITE)) {
        pDest[1 ^ !!NeedsBig] = (dissect.negative << 7) | 0x7f;
        pDest[0 ^ !!NeedsBig] = 0x80;

        /* clone all-ones mantissa for NaN: */

        if (as_float_mantissa_is_allones(&dissect)) {
            pDest[0 ^ !!NeedsBig] |= 0x7f;
        }

        /* otherwise clone MSB+LSB of mantissa: */

        else {
            if (as_float_get_mantissa_bit(dissect.mantissa, dissect.mantissa_bits, 1)) {
                pDest[0 ^ !!NeedsBig] |= 0x40;
            }
            if (as_float_get_mantissa_bit(
                        dissect.mantissa, dissect.mantissa_bits,
                        dissect.mantissa_bits - 1)) {
                pDest[0 ^ !!NeedsBig] |= 0x01;
            }
        }
        return 2;
    }

    /* (2) Round to target precision: */

    round_type = as_float_round(&dissect, 8);

    /* (3a) Overrange? */

    if (dissect.exponent > 127) {
        return -E2BIG;
    } else if (
            (dissect.exponent == 127) && as_float_mantissa_is_allones(&dissect)
            && (round_type == e_round_down)) {
        return -E2BIG;
    }

    /* (3b) number that is too small may degenerate to 0: */

    while ((dissect.exponent < -127) && !as_float_mantissa_is_zero(&dissect)) {
        dissect.exponent++;
        as_float_mantissa_shift_right(dissect.mantissa, 0, dissect.mantissa_bits);
    }

    /* numbers too small to represent degenerate to 0 (mantissa was shifted out) */

    if (dissect.exponent < -127) {
        dissect.exponent = -127;
    }

    /* For denormal numbers, exponent is 2^(-1126) and not 2^(-127)!
       So if we end up with an exponent of 2^(-127), convert
       mantissa so it corresponds to 2^(-126): */

    else if (dissect.exponent == -127) {
        as_float_mantissa_shift_right(dissect.mantissa, 0, dissect.mantissa_bits);
    }

    /* (3c) add bias to exponent */

    dissect.exponent += 127;

    /* (3d) store result */

    pDest[1 ^ !!NeedsBig] = (dissect.negative << 7) | ((dissect.exponent >> 1) & 0x7f);
    pDest[0 ^ !!NeedsBig]
            = ((dissect.exponent & 1) << 7) | as_float_mantissa_extract(&dissect, 1, 7);
    return 2;
}
