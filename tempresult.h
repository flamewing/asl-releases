#ifndef TEMPRESULT_H
#define TEMPRESULT_H
/* tempresult.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* internal holder for int/float/string                                      */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "nonzstring.h"
#include "symbolsize.h"
#include "symflags.h"

#include <stddef.h>

typedef enum {
    TempNone   = 0,
    TempInt    = 1,
    TempFloat  = 2,
    TempString = 4,
    TempReg    = 8,
    TempAll    = 15
} TempType;

struct sRelocEntry;
struct as_dynstr;

typedef unsigned tRegInt;

typedef void (*DissectRegProc)(
        char* pDest, size_t DestSize, tRegInt Value, tSymbolSize InpSize);

typedef DissectRegProc dissect_reg_fnc_t;

typedef int (*compare_reg_fnc_t)(
        tRegInt value1, tSymbolSize reg_size1, tRegInt value2, tSymbolSize reg_size2);

/*
 * Used in register's number to signify a built in alternate name
 * for the same register, e.g. SP=A7 on 68K.  Registers will compare
 * equal, but will be printed with individual name in the symbol table:
 */

#define REGSYM_FLAG_ALIAS 0x80

#define INTSYM_NUM_BITS_DEFAULT 0

typedef struct sRegDescr {
    dissect_reg_fnc_t Dissect;
    compare_reg_fnc_t compare;
    tRegInt           Reg;
} tRegDescr;

struct sTempResult {
    TempType            Typ;
    tSymbolFlags        Flags;
    unsigned            AddrSpaceMask;
    tSymbolSize         DataSize;
    struct sRelocEntry* Relocs;

    union {
        struct {
            LargeInt Value;
            Byte     NumBits;
        } Int;

        as_float_t       Float;
        as_nonz_dynstr_t str;
        tRegDescr        RegDescr;
    } Contents;
};
typedef struct sTempResult TempResult;

extern void        as_tempres_ini(TempResult* p_res);
extern TempResult* as_tempres_dyn_ini(void);

extern void as_tempres_free(TempResult* p_res);
extern void as_tempres_dyn_free(TempResult* p_res);

extern void as_tempres_set_none(TempResult* p_res);

extern void as_tempres_set_int(TempResult* p_res, LargeInt value);

extern void as_tempres_set_float(TempResult* p_res, as_float_t value);

extern void as_tempres_set_str(TempResult* p_res, as_nonz_dynstr_t const* p_value);

extern void as_tempres_set_str_raw(TempResult* p_res, char const* p_value, size_t len);

extern void as_tempres_set_c_str(TempResult* p_res, char const* p_value);

extern void as_tempres_set_reg(TempResult* p_res, tRegDescr const* p_value);

void as_tempres_copy_value(TempResult* p_dest, TempResult const* p_src);

extern void as_tempres_copy(TempResult* p_dest, TempResult const* p_src);

extern int as_tempres_cmp(TempResult const* p_res1, TempResult const* p_res2);

extern int TempResultToFloat(TempResult* pResult);

extern int as_tempres_append_dynstr(
        struct as_dynstr* p_dest, TempResult const* pResult, int int_radix);

#endif /* TEMPRESULT_H */
