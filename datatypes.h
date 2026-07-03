#ifndef DATATYPES_H
#define DATATYPES_H
/* datatypes.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* define some handy types & constants                                       */
/*                                                                           */
/* History:  2001-10-13 /AArnold - created this comment                      */
/*                                                                           */
/*****************************************************************************/

#include "sysdefs.h"

/* Integer Types */

typedef as_uint8_t Byte;
typedef as_int8_t  ShortInt;

#ifdef AS_HAS16
typedef as_uint16_t Word;
typedef as_int16_t  Integer;
#endif

typedef as_uint32_t LongWord;
typedef as_int32_t  LongInt;
#define PRILongInt AS_PRId32
#define MaxLongInt 2147483647

#ifdef AS_HAS64
typedef as_uint64_t QuadWord;
typedef as_int64_t  QuadInt;
#endif

#ifdef AS_HAS128
typedef as_uint128_t OctaWord;
typedef as_int128_t  OctaInt;
#endif

/* On pure 32 bit platforms, avoid warnings about shift
   count being equal to operand size: */

#ifdef AS_HAS128
typedef OctaInt  LargeInt;
typedef OctaWord LargeWord;
#    define LARGEBITS         128
#    define largeint_shr32(l) (l >>= 32)
#else
#    ifdef AS_HAS64
typedef QuadInt  LargeInt;
typedef QuadWord LargeWord;
#        define LARGEBITS         64
#        define largeint_shr32(l) (l >>= 32)
#    else
typedef LongInt  LargeInt;
typedef LongWord LargeWord;
#        define LARGEBITS         32
#        define largeint_shr32(l) (l = 0)
#    endif
#endif

typedef signed int   sint;
typedef unsigned int usint;

typedef Byte Boolean;

#ifndef STRINGSIZE
#    define STRINGSIZE 1024
#endif
#define SHORTSTRINGSIZE 65

typedef char String[STRINGSIZE];
typedef char ShortString[SHORTSTRINGSIZE];

#ifndef TRUE
#    define TRUE 1
#endif
#ifndef True
#    define True 1
#endif

#ifndef FALSE
#    define FALSE 0
#endif
#ifndef False
#    define False 0
#endif

#endif /* DATATYPES_H */
