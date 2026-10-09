/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only */
#ifndef INTCONST_CORE_H
#define INTCONST_CORE_H

#include "stdinc.h"
#include "asmdef.h"
#include "asmerr.h"
#include "int_type.h"
#include "strref.h"

/* Internal parser leaf: no termination or ownership is required. Keep inline
   visibility for the evaluator and exercise the same implementation in tests. */
/*!------------------------------------------------------------------------
 * \fn     const_int_val_ref(as_strref_t expr, LargeInt *p_value, tSymbolFlags *p_flags, Byte *p_num_bits, int *p_outof_range)
 * \brief  evaluate integer constant
 * \param  expr bounded source argument
 * \param  p_value parsed integer value
 * \param  p_flags associated symbol flags (only negative ATM)
 * \param  p_num_bits returns bit length of integer
 * \param  p_outof_range returns -1/+1 if out of LargeInt range
 * \return True if parsing OK
 * ------------------------------------------------------------------------ */

static inline Boolean const_int_val_ref(
        as_strref_t expr, LargeInt* p_value, tSymbolFlags* p_flags, Byte* p_num_bits,
        int* p_outof_range) {
    Boolean               neg_flag = False, first_digit;
    LargeWord             acc, acc_max, mul_max;
    tIntCheckCtx          ctx;
    unsigned              num_digits;
    tIntFormatList const* p_int_format;

    char const* p_expr = expr.data;

    *p_outof_range = 0;

    /* sign: */

    acc_max = (LargeWord)-1;
    switch (expr.len ? *p_expr : 0) {
    case '-':
        neg_flag = True;
        acc_max  = (acc_max >> 1) + 1;
        /* else fall-through */
    case '+':
        p_expr++;
        expr.len--;
        break;
    }

    /* empty string is interpreted as 0 */

    if (!expr.len) {
        *p_value = 0;
        return True;
    }

    ctx.pExpr   = p_expr;
    ctx.ExprLen = expr.len;
    ctx.Base    = -1;

    for (p_int_format = IntFormatList; p_int_format->Check; p_int_format++) {
        if (p_int_format->Check(&ctx, p_int_format->Ch)) {
            ctx.Base = (p_int_format->Base > 0) ? p_int_format->Base : RadixBase;
            break;
        }
    }
    if (ctx.Base <= 0) {
        return False;
    }
    mul_max = acc_max / ctx.Base;

    /* we may have decremented ctx.ExprLen, so do not run until string end */

    acc         = 0;
    num_digits  = 0;
    first_digit = True;
    if (256 == ctx.Base) {
        TransTableCheckRead();
    }
    while (ctx.ExprLen > 0) {
        int digit;

        if (256 == ctx.Base) {
            digit = as_chartrans_xlate(
                    CurrTransTable->p_table, ((unsigned)*ctx.pExpr) & 0xff);
            if (digit < 0) {
                WrError(ErrNum_UnmappedChar);
                digit = 0;
            }
        } else {
            digit = DigitVal(as_toupper(*ctx.pExpr), ctx.Base);
        }
        if (digit == -1) {
            *p_value = -1;
            return False;
        }
        if (acc > mul_max) {
            *p_outof_range = neg_flag ? -1 : 1;
            *p_value       = neg_flag ? -acc_max : acc_max;
            return True;
        }
        acc = acc * ctx.Base;
        if (acc_max - acc < (unsigned)digit) {
            *p_outof_range = neg_flag ? -1 : 1;
            *p_value       = neg_flag ? -acc_max : acc_max;
            return True;
        }
        acc = acc + digit;
        /* Necessary leading 0 for Intel-style hex consts does not count: */
        if ((p_int_format->Id == eIntFormatIntHex) && first_digit && (*ctx.pExpr == '0')
            && (ctx.ExprLen > 1) && as_isalpha(ctx.pExpr[1])) {
        } else {
            if (num_digits < LARGEBITS) {
                num_digits++;
            }
        }
        first_digit = False;
        ctx.pExpr++;
        ctx.ExprLen--;
    }

    if (neg_flag) {
        *p_flags    = eSymbolFlag_Negative;
        *p_num_bits = -1;
        *p_value    = -acc;
    } else {
        *p_flags    = eSymbolFlag_None;
        *p_num_bits = force_restricted_extension_digits
                              ? min(num_digits * as_log2(ctx.Base), LARGEBITS)
                              : 0;
        *p_value    = acc;
    }
    return True;
}


#endif
