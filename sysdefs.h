#ifndef SYSDEFS_H
#define SYSDEFS_H
/* sysdefs.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS Port                                                                   */
/*                                                                           */
/* system-specific definitions                                               */
/*                                                                           */
/* History:  2001-04-13 activated DRSEP for Win32 platform                   */
/*           2001-09-11 added MacOSX                                         */
/*           2001-10-13 added ARM/Linux                                      */
/*                                                                           */
/*****************************************************************************/

#include "config.h"    // IWYU pragma: export

#include <inttypes.h>
#include <stdint.h>

#ifdef _MSC_VER
#    include <stddef.h>
#    ifdef _USE_ATTRIBUTES_FOR_SAL
#        undef _USE_ATTRIBUTES_FOR_SAL
#    endif
/* nolint */
#    define _USE_ATTRIBUTES_FOR_SAL 1
#    include <sal.h>
#    define PRINTF_FORMAT _Printf_format_string_
#    define PRINTF_FORMAT_ATTR(format_param, dots_param)
#elif defined(__GNUC__)
#    define PRINTF_FORMAT
#    define PRINTF_FORMAT_ATTR(format_param, dots_param) \
        __attribute__((__format__(__printf__, format_param, dots_param)))
#else
#    define PRINTF_FORMAT
#    define PRINTF_FORMAT_ATTR(format_param, dots_param)
#endif

#if defined(_WIN32) || defined(__EMX__) || defined(__MSDOS__) || defined(__IBMC__)
#    define OPENRDMODE "rb"
#    define OPENWRMODE "wb"
#    define OPENUPMODE "rb+"
#    define SLASHARGS
#    define PATHSEP  '\\'
#    define SPATHSEP "\\"
#    define DIRSEP   ';'
#    define SDIRSEP  ";"
#    define DRSEP    ':'
#    define SDRSEP   ":"
#    define NULLDEV  "NUL"
#else
#    define OPENRDMODE "r"
#    define OPENWRMODE "w"
#    define OPENUPMODE "r+"
#    define PATHSEP    '/'
#    define SPATHSEP   "/"
#    define DIRSEP     ':'
#    define SDIRSEP    ":"
#    define NULLDEV    "/dev/null"
#endif

#define AS_HAS_LONGLONG   1
#define AS_HAS_LONGDOUBLE 1

#if defined(vax)
#    define HOST_DECFLOAT
#elif defined(AS_IEEE_EXTENDED)
#    if AS_LONG_DOUBLE_SIZE == 10
#        define IEEEFLOAT_10_10_LONG_DOUBLE
#    elif AS_LONG_DOUBLE_SIZE == 12
#        if defined(__m68k__)
#            define IEEEFLOAT_10_2P8_LONG_DOUBLE
#        else
#            define IEEEFLOAT_10_12_LONG_DOUBLE
#        endif
#    elif AS_LONG_DOUBLE_SIZE == 16
#        define IEEEFLOAT_10_16_LONG_DOUBLE
#    endif
#else
#    define IEEEFLOAT_8_DOUBLE
#endif

// Note: SunOS and derived incorrectly defines int8_t as 'char'.
typedef int8_t   as_int8_t;
typedef uint8_t  as_uint8_t;
typedef int16_t  as_int16_t;
typedef uint16_t as_uint16_t;
typedef int32_t  as_int32_t;
#define PRIas_int32_t PRId32
typedef uint32_t    as_uint32_t;
typedef int64_t     as_int64_t;
typedef uint64_t    as_uint64_t;
typedef as_int8_t   Integ8;
typedef as_uint8_t  Card8;
typedef as_int16_t  Integ16;
typedef as_uint16_t Card16;
typedef as_int32_t  Integ32;
typedef as_uint32_t Card32;
typedef as_int64_t  Integ64;
typedef as_uint64_t Card64;
#define PRIInteg32 PRId32
#define HAS16
#define HAS64

/*---------------------------------------------------------------------------*/
/* We require C11, so we surely can use prototypes */

/* Some VAX compilers internally seem to use D float
   and are unable to parse the G float DBL_MAX literal
   of 8.98...E+308 from float.h.
   So we put a hand-crafted constant in memory: */

#ifdef HOST_DECFLOAT
typedef double as_float_t;
#    ifdef __GFLOAT
extern double as_decfloat_get_max_gfloat(void);
#        define AS_FLOAT_MAX as_decfloat_get_max_gfloat()
#    else
#        define AS_FLOAT_MAX DBL_MAX
#    endif
#    define AS_FLOAT_DIG    DBL_DIG
#    define AS_HUGE_VAL     HUGE_VAL
#    define as_strtof(s, e) strtod(s, e)
#endif /* HOST_DECFLOAT */

#ifdef IEEEFLOAT_8_DOUBLE
typedef double as_float_t;
#    define AS_FLOAT_MAX    DBL_MAX
#    define AS_FLOAT_DIG    DBL_DIG
#    define AS_HUGE_VAL     HUGE_VAL
#    define as_strtof(s, e) strtod(s, e)
#endif /* IEEEFLOAT_8_DOUBLE */

#ifdef IEEEFLOAT_10_16_LONG_DOUBLE
#    define IEEEFLOAT_10_LONG_DOUBLE
#    define XPRIas_float_t "L"
#endif /* IEEEFLOAT_10_16_LONG_DOUBLE */

#ifdef IEEEFLOAT_10_12_LONG_DOUBLE
#    define IEEEFLOAT_10_LONG_DOUBLE
#    define XPRIas_float_t "L"
#endif /* IEEEFLOAT_10_12_LONG_DOUBLE */

#ifdef IEEEFLOAT_10_10_LONG_DOUBLE
#    define IEEEFLOAT_10_LONG_DOUBLE
#    define XPRIas_float_t "L"
#endif /* IEEEFLOAT_10_10_LONG_DOUBLE */

#ifdef IEEEFLOAT_10_2P8_LONG_DOUBLE
#    define IEEEFLOAT_10_LONG_DOUBLE
#    define XPRIas_float_t "L"
#endif /* IEEEFLOAT_10_2P8_LONG_DOUBLE */

#ifdef IEEEFLOAT_10_LONG_DOUBLE
typedef long double as_float_t;
#    define AS_FLOAT_MAX    LDBL_MAX
#    define AS_FLOAT_DIG    LDBL_DIG
#    define AS_HUGE_VAL     HUGE_VALL
#    define as_strtof(s, e) strtold(s, e)
#    define as_fabs(f)      fabsl(f)
#    define as_ldexp(f, e)  ldexpl(f, e)
#    define as_frexp(f, e)  frexpl(f, e)
#    define as_modf(f, e)   modfl(f, e)
#    define as_sqrt(f)      sqrtl(f)
#    define as_sin(f)       sinl(f)
#    define as_cos(f)       cosl(f)
#    define as_tan(f)       tanl(f)
#    define as_asin(f)      asinl(f)
#    define as_acos(f)      acosl(f)
#    define as_atan(f)      atanl(f)
#    define as_exp(f)       expl(f)
#    define as_log(f)       logl(f)
#    define as_log10(f)     log10l(f)
#    define as_sinh(f)      sinh(f)
#    define as_cosh(f)      coshl(f)
#    define as_tanh(f)      tanhl(f)
#endif /* IEEEFLOAT_10_LONG_DOUBLE */

#ifndef as_strtof
#    define as_strtof(s, e) strtod(s, e)
#endif
#ifndef XPRIas_float_t
#    define XPRIas_float_t ""
#endif
#ifndef as_fabs
#    define as_fabs(f) fabs(f)
#endif
#ifndef as_ldexp
#    define as_ldexp(f, e) ldexp(f, e)
#endif
#ifndef as_frexp
#    define as_frexp(f, e) frexp(f, e)
#endif
#ifndef as_modf
#    define as_modf(f, e) modf(f, e)
#endif
#ifndef as_sqrt
#    define as_sqrt(f) sqrt(f)
#endif
#ifndef as_sin
#    define as_sin(f) sin(f)
#endif
#ifndef as_cos
#    define as_cos(f) cos(f)
#endif
#ifndef as_tan
#    define as_tan(f) tan(f)
#endif
#ifndef as_asin
#    define as_asin(f) asin(f)
#endif
#ifndef as_acos
#    define as_acos(f) acos(f)
#endif
#ifndef as_atan
#    define as_atan(f) atan(f)
#endif
#ifndef as_sinh
#    define as_sinh(f) sinh(f)
#endif
#ifndef as_exp
#    define as_exp(f) exp(f)
#endif
#ifndef as_log
#    define as_log(f) log(f)
#endif
#ifndef as_log10
#    define as_log10(f) log10(f)
#endif
#ifndef as_cosh
#    define as_cosh(f) cosh(f)
#endif
#ifndef as_tanh
#    define as_tanh(f) tanh(f)
#endif

#define UNUSED(x) (void)x

#endif /* SYSDEFS_H */
