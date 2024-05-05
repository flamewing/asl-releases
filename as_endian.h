#ifndef AS_ENDIAN_H
#define AS_ENDIAN_H
/* as_endian.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Little/Big-Endian-Routinen                                                */
/*                                                                           */
/* Historie: 30. 5.1996 Grundsteinlegung                                     */
/*            6. 7.1997 Dec32BlankString dazu                                */
/*            1. 6.2000 added LargeHIntFormat                                */
/*            7. 7.2000 added memory read/write functions                    */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

#include <stdio.h>

_Static_assert(sizeof(Byte) == 1, "sizeof(Byte) is not 1");
_Static_assert(sizeof(ShortInt) == 1, "sizeof(ShortInt) is not 1");
#ifdef HAS16
_Static_assert(sizeof(Word) == 2, "sizeof(Word) is not 2");
_Static_assert(sizeof(Integer) == 2, "sizeof(Integer) is not 2");
#endif
_Static_assert(sizeof(LongInt) == 4, "sizeof(LongInt) is not 4");
_Static_assert(sizeof(LongWord) == 4, "sizeof(LongWord) is not 4");
#ifdef HAS64
_Static_assert(sizeof(QuadInt) == 8, "sizeof(QuadInt) is not 8");
_Static_assert(sizeof(QuadWord) == 8, "sizeof(QuadWord) is not 8");
#endif
_Static_assert(sizeof(Single) == 4, "sizeof(Single) is not 4");
_Static_assert(sizeof(Double) == 8, "sizeof(Double) is not 8");

#ifdef __BYTE_ORDER__
#    ifndef BYTE_ORDER
#        define BYTE_ORDER __BYTE_ORDER__
#    endif
#    ifndef BIG_ENDIAN
#        define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#    endif
#    ifndef LITTLE_ENDIAN
#        define LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#    endif
#elif defined(_MSC_VER)
#    define BIG_ENDIAN    4321
#    define LITTLE_ENDIAN 1234
#    define PDP_ENDIAN    3412
#    define BYTE_ORDER    LITTLE_ENDIAN
#elif defined(HAVE_ENDIAN_H)
#    include <endian.h>
#elif defined(HAVE_SYS_PARAM_H)
#    include <sys/param.h>
#else
#    error "Cannot determine endianness of target platform."
#endif

#define HostBigEndian (BYTE_ORDER == BIG_ENDIAN)

#ifdef __TINYC__
#    define INLINE extern
#else
#    define INLINE static inline
#endif

INLINE void WSwap(void* Field, int Cnt);
INLINE void DSwap(void* Field, int Cnt);
INLINE void QSwap(void* Field, int Cnt);
INLINE void TSwap(void* Field, int Cnt);
INLINE void DWSwap(void* Field, int Cnt);
INLINE void QWSwap(void* Field, int Cnt);
INLINE void TWSwap(void* Field, int Cnt);

#if HostBigEndian
#    define WSwapBigEndianToHost(Field, Cnt)
#    define DSwapBigEndianToHost(Field, Cnt)
#    define QSwapBigEndianToHost(Field, Cnt)
#    define TSwapBigEndianToHost(Field, Cnt)
#    define DWSwapBigEndianToHost(Field, Cnt)
#    define QWSwapBigEndianToHost(Field, Cnt)
#    define TWSwapBigEndianToHost(Field, Cnt)
#    define WSwapLittleEndianToHost(Field, Cnt)  WSwap(Field, Cnt)
#    define DSwapLittleEndianToHost(Field, Cnt)  DSwap(Field, Cnt)
#    define QSwapLittleEndianToHost(Field, Cnt)  QSwap(Field, Cnt)
#    define TSwapLittleEndianToHost(Field, Cnt)  TSwap(Field, Cnt)
#    define DWSwapLittleEndianToHost(Field, Cnt) DWSwap(Field, Cnt)
#    define QWSwapLittleEndianToHost(Field, Cnt) QWSwap(Field, Cnt)
#    define TWSwapLittleEndianToHost(Field, Cnt) TWSwap(Field, Cnt)
#else
#    define WSwapBigEndianToHost(Field, Cnt)  WSwap(Field, Cnt)
#    define DSwapBigEndianToHost(Field, Cnt)  DSwap(Field, Cnt)
#    define QSwapBigEndianToHost(Field, Cnt)  QSwap(Field, Cnt)
#    define TSwapBigEndianToHost(Field, Cnt)  TSwap(Field, Cnt)
#    define DWSwapBigEndianToHost(Field, Cnt) DWSwap(Field, Cnt)
#    define QWSwapBigEndianToHost(Field, Cnt) QWSwap(Field, Cnt)
#    define TWSwapBigEndianToHost(Field, Cnt) TWSwap(Field, Cnt)
#    define WSwapLittleEndianToHost(Field, Cnt)
#    define DSwapLittleEndianToHost(Field, Cnt)
#    define QSwapLittleEndianToHost(Field, Cnt)
#    define TSwapLittleEndianToHost(Field, Cnt)
#    define DWSwapLittleEndianToHost(Field, Cnt)
#    define QWSwapLittleEndianToHost(Field, Cnt)
#    define TWSwapLittleEndianToHost(Field, Cnt)
#endif

INLINE Boolean Read2(FILE* file, void* Ptr);
INLINE Boolean Read4(FILE* file, void* Ptr);
INLINE Boolean Read8(FILE* file, void* Ptr);

INLINE Boolean Write2(FILE* file, void const* Ptr);
INLINE Boolean Write4(FILE* file, void const* Ptr);
INLINE Boolean Write8(FILE* file, void const* Ptr);

#define MRead1L(Buffer) (*((Byte*)(Buffer)))
#define MRead1B(Buffer) (*((Byte*)(Buffer)))

INLINE Word MRead2L(Byte const* Buffer);
INLINE Word MRead2B(Byte const* Buffer);

#define MWrite1L(Buffer, Value) (*((Byte*)(Buffer))) = Value
#define MWrite1B(Buffer, Value) (*((Byte*)(Buffer))) = Value

INLINE void     MWrite2L(Byte* Buffer, Word Value);
INLINE void     MWrite2B(Byte* Buffer, Word Value);
INLINE LongWord MRead4L(Byte const* Buffer);
INLINE LongWord MRead4B(Byte const* Buffer);
INLINE void     MWrite4L(Byte* Buffer, LongWord Value);
INLINE void     MWrite4B(Byte* Buffer, LongWord Value);

#ifdef HAS64
INLINE QuadWord MRead8L(Byte const* Buffer);
INLINE QuadWord MRead8B(Byte const* Buffer);
INLINE void     MWrite8L(Byte* Buffer, QuadWord Value);
INLINE void     MWrite8B(Byte* Buffer, QuadWord Value);
#endif

#undef INLINE

#ifndef __TINYC__
#    include "as_endian.inl.h"
#endif

#endif /* AS_ENDIAN_H */
