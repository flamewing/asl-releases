/* as_endian.inl */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Little/Big-Endian-Routinen                                                */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#ifndef AS_ENDIAN_H
# ifdef __BYTE_ORDER__
#  ifndef BYTE_ORDER
#   define BYTE_ORDER __BYTE_ORDER__
#  endif
#  ifndef BIG_ENDIAN
#   define BIG_ENDIAN __ORDER_BIG_ENDIAN__
#  endif
#  ifndef LITTLE_ENDIAN
#   define LITTLE_ENDIAN	__ORDER_LITTLE_ENDIAN__
#  endif
# elif defined (_MSC_VER)
#  define BIG_ENDIAN	4321
#  define LITTLE_ENDIAN	1234
#  define PDP_ENDIAN	3412
#  define BYTE_ORDER	LITTLE_ENDIAN
# elif defined(HAVE_ENDIAN_H)
#  include <endian.h>
# elif defined(HAVE_SYS_PARAM_H)
#  include <sys/param.h>
# else
#  error "Cannot determine endianness of target platform."
# endif
# define HostBigEndian (BYTE_ORDER == BIG_ENDIAN)
#endif

#ifdef _MSC_VER
# include <stdlib.h>
# pragma intrinsic(_byteswap_ushort)
# pragma intrinsic(_byteswap_ulong)
# pragma intrinsic(_byteswap_uint64)
# define bswap16(x) _byteswap_ushort(x)
# define bswap32(x) _byteswap_ulong(x)
# define bswap64(x) _byteswap_uint64(x)
#elif defined(__INTEL_COMPILER)
#    include <byteswap.h>
# define bswap16(x) __builtin_bswap16(x)
# define bswap32(x) __builtin_bswap32(x)
# define bswap64(x) __builtin_bswap64(x)
#elif defined(__GNUC__)
# define bswap16(x) __builtin_bswap16(x)
# define bswap32(x) __builtin_bswap32(x)
# define bswap64(x) __builtin_bswap64(x)
#else
# define bswap16(x) ((((x) & 0xffU) << 8) | ((x) >> 8))
# define bswap32(x)                                                            \
  ((((x) & 0xffUL) << 24) | (((x) & 0xff00UL) << 8) |                          \
   (((x) & 0xff0000UL) >> 8) | ((x) >> 24))
# define bswap64(x)                                                            \
  ((((x) & 0xffULL) << 56) | (((x) & 0xff00ULL) << 40) |                       \
   (((x) & 0xff0000ULL) << 24) | (((x) & 0xff000000ULL) << 8) |                \
   (((x) & 0xff00000000ULL) >> 8) | (((x) & 0xff0000000000ULL) >> 24) |        \
   (((x) & 0xff000000000000ULL) >> 40) | ((x) >> 56))
#endif

#ifdef _MSC_VER
# pragma intrinsic(_rotl)
#define rot32(x, y) _rotl(x, y)
#else
# define rot32(x, y) ((x << y) | (x >> (sizeof(x) * 8 - y)))
#endif

#ifdef __cplusplus
# define INLINE inline
#elif defined(__TINYC__)
# define INLINE
#else
# define INLINE static inline
#endif

INLINE void write16_as_little_endian(void *Buffer, uint16_t val) {
#if HostBigEndian
  val = bswap16(val);
#endif
  memcpy(Buffer, &val, sizeof(val));
}

INLINE void write32_as_little_endian(void *Buffer, uint32_t val) {
#if HostBigEndian
  val = bswap32(val);
#endif
  memcpy(Buffer, &val, sizeof(val));
}

INLINE void write64_as_little_endian(void *Buffer, uint64_t val) {
#if HostBigEndian
  val = bswap64(val);
#endif
  memcpy(Buffer, &val, sizeof(val));
}

INLINE uint16_t read16_as_little_endian(const void *Buffer) {
  uint16_t val;
  memcpy(&val, Buffer, sizeof(val));
#if HostBigEndian
  val = bswap16(val);
#endif
  return val;
}

INLINE uint32_t read32_as_little_endian(const void *Buffer) {
  uint32_t val;
  memcpy(&val, Buffer, sizeof(val));
#if HostBigEndian
  val = bswap32(val);
#endif
  return val;
}

INLINE uint64_t read64_as_little_endian(const void *Buffer) {
  uint64_t val;
  memcpy(&val, Buffer, sizeof(val));
#if HostBigEndian
  val = bswap64(val);
#endif
  return val;
}

INLINE void write16_as_big_endian(void *Buffer, uint16_t val) {
#if HostBigEndian == 0
  val = bswap16(val);
#endif
  memcpy(Buffer, &val, sizeof(val));
}

INLINE void write32_as_big_endian(void *Buffer, uint32_t val) {
#if HostBigEndian == 0
  val = bswap32(val);
#endif
  memcpy(Buffer, &val, sizeof(val));
}

INLINE void write64_as_big_endian(void *Buffer, uint64_t val) {
#if HostBigEndian == 0
  val = bswap64(val);
#endif
  memcpy(Buffer, &val, sizeof(val));
}

INLINE uint16_t read16_as_big_endian(const void *Buffer) {
  uint16_t val;
  memcpy(&val, Buffer, sizeof(val));
#if HostBigEndian == 0
  val = bswap16(val);
#endif
  return val;
}

INLINE uint32_t read32_as_big_endian(const void *Buffer) {
  uint32_t val;
  memcpy(&val, Buffer, sizeof(val));
#if HostBigEndian == 0
  val = bswap32(val);
#endif
  return val;
}

INLINE uint64_t read64_as_big_endian(const void *Buffer) {
  uint64_t val;
  memcpy(&val, Buffer, sizeof(val));
#if HostBigEndian == 0
  val = bswap64(val);
#endif
  return val;
}

INLINE void bswap16_ptr(unsigned char *Buffer) {
  uint16_t val;
  memcpy(&val, Buffer, sizeof(val));
  val = bswap16(val);
  memcpy(Buffer, &val, sizeof(val));
}

INLINE void bswap32_ptr(unsigned char *Buffer) {
  uint32_t val;
  memcpy(&val, Buffer, sizeof(val));
  val = bswap32(val);
  memcpy(Buffer, &val, sizeof(val));
}

INLINE void bswap64_ptr(unsigned char *Buffer) {
  uint64_t val;
  memcpy(&val, Buffer, sizeof(val));
  val = bswap64(val);
  memcpy(Buffer, &val, sizeof(val));
}

INLINE uint32_t read32_and_swap_words(const unsigned char *Buffer) {
  uint32_t val;
  memcpy(&val, Buffer, sizeof(val));
  val = rot32(val, 16);
  return val;
}

INLINE void write32(unsigned char *Buffer, uint32_t val) {
  memcpy(Buffer, &val, sizeof(val));
}

/*****************************************************************************/

INLINE void WSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field;
  register const unsigned char *End = Run + Cnt;

  for (; Run != End; Run += sizeof(uint16_t))
  {
    bswap16_ptr(Run);
  }
}

INLINE void DSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field;
  register const unsigned char *End = Run + Cnt;

  for (; Run != End; Run += sizeof(uint32_t))
  {
    bswap32_ptr(Run);
  }
}

INLINE void QSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field;
  register const unsigned char *End = Run + Cnt;

  for (; Run != End; Run += sizeof(uint64_t))
  {
    bswap64_ptr(Run);
  }
}

INLINE void TSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field;
  register const unsigned char *End = Run + Cnt;

  for (; Run != End; Run += sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t))
  {
    unsigned char * const ptr_first  = Run;
    unsigned char * const ptr_middle = Run + sizeof(uint32_t);
    unsigned char * const ptr_last   = Run + sizeof(uint32_t) + sizeof(uint16_t);
    uint32_t first  = read32_as_little_endian(ptr_first );
    uint16_t middle = read16_as_little_endian(ptr_middle);
    uint32_t last   = read32_as_little_endian(ptr_last  );
    write32_as_big_endian(ptr_first , last  );
    write16_as_big_endian(ptr_middle, middle);
    write32_as_big_endian(ptr_last  , first );
  }
}

INLINE void DWSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field;
  register const unsigned char *End = Run + Cnt;

  for (; Run != End; Run += sizeof(uint32_t))
  {
    write32(Run, read32_and_swap_words(Run));
  }
}

INLINE void QWSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field;
  register const unsigned char *End = Run + Cnt;

  for (; Run != End; Run += sizeof(uint64_t))
  {
    unsigned char * const ptr_first  = Run;
    unsigned char * const ptr_last   = Run + sizeof(uint32_t);
    uint32_t first = read32_and_swap_words(ptr_first);
    uint32_t last  = read32_and_swap_words(ptr_last);
    write32(ptr_first, last);
    write32(ptr_last, first);
  }
}

INLINE void TWSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field;
  register const unsigned char *End = Run + Cnt;

  for (; Run != End; Run += sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uint16_t))
  {
    /* center word needs not be swapped with itself */
    unsigned char * const ptr_first  = Run;
    unsigned char * const ptr_last   = Run + sizeof(uint32_t) + sizeof(uint16_t);
    uint32_t first = read32_and_swap_words(ptr_first);
    uint32_t last  = read32_and_swap_words(ptr_last);
    write32(ptr_first, last);
    write32(ptr_last, first);
  }
}

INLINE Boolean Read2(FILE *file, void *Ptr)
{
  uint16_t val = 0;
  Boolean OK = (fread(&val, 1, sizeof(val), file) == sizeof(val));
  write16_as_little_endian(Ptr, val);
  return OK;
}

INLINE Boolean Read4(FILE *file, void *Ptr)
{
  uint32_t val = 0;
  Boolean OK = (fread(&val, 1, sizeof(val), file) == sizeof(val));
  write32_as_little_endian(Ptr, val);
  return OK;
}

INLINE Boolean Read8(FILE *file, void *Ptr)
{
  uint64_t val = 0;
  Boolean OK = (fread(&val, 1, sizeof(val), file) == sizeof(val));
  write64_as_little_endian(Ptr, val);
  return OK;
}

INLINE Boolean Write2(FILE *file, const void *Ptr)
{
  uint16_t val = read16_as_little_endian(Ptr);
  return fwrite(&val, 1, sizeof(val), file) == sizeof(val);
}

INLINE Boolean Write4(FILE *file, const void *Ptr)
{
  uint32_t val = read32_as_little_endian(Ptr);
  return fwrite(&val, 1, sizeof(val), file) == sizeof(val);
}

INLINE Boolean Write8(FILE *file, const void *Ptr)
{
  uint64_t val = read64_as_little_endian(Ptr);
  return fwrite(&val, 1, sizeof(val), file) == sizeof(val);
}

INLINE Word MRead2L(const Byte *Buffer)
{
  return read16_as_little_endian(Buffer);
}

INLINE Word MRead2B(const Byte *Buffer)
{
  return read16_as_big_endian(Buffer);
}

INLINE void MWrite2L(Byte *Buffer, Word Value)
{
  write16_as_little_endian(Buffer, Value);
}

INLINE void MWrite2B(Byte *Buffer, Word Value)
{
  write16_as_big_endian(Buffer, Value);
}

INLINE LongWord MRead4L(const Byte *Buffer)
{
  return read32_as_little_endian(Buffer);
}

INLINE LongWord MRead4B(const Byte *Buffer)
{
  return read32_as_big_endian(Buffer);
}

INLINE void MWrite4L(Byte *Buffer, LongWord Value)
{
  write32_as_little_endian(Buffer, Value);
}

INLINE void MWrite4B(Byte *Buffer, LongWord Value)
{
  write32_as_big_endian(Buffer, Value);
}

#ifdef HAS64
INLINE QuadWord MRead8L(const Byte *Buffer)
{
  return read64_as_little_endian(Buffer);
}

INLINE QuadWord MRead8B(const Byte *Buffer)
{
  return read64_as_big_endian(Buffer);
}

INLINE void MWrite8L(Byte *Buffer, QuadWord Value)
{
  write64_as_little_endian(Buffer, Value);
}

INLINE void MWrite8B(Byte *Buffer, QuadWord Value)
{
  write64_as_big_endian(Buffer, Value);
}
#endif
