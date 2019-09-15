/* endian.c */
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

#include "endian.h"

/*****************************************************************************/

Boolean BigEndian;

char *Integ16Format, *Integ32Format, *Integ64Format;
char *IntegerFormat, *LongIntFormat, *QuadIntFormat;
char *LargeIntFormat, *LargeHIntFormat;

/*****************************************************************************/

void WSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field, Swap;
  register int z;

  for (z = 0; z < Cnt / 2; z++, Run += 2)
  {
    Swap = Run[0];
    Run[0] = Run[1];
    Run[1] = Swap;
  }
}

void DSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field, Swap;
  register int z;

  for (z = 0; z < Cnt / 4; z++, Run += 4)
  {
    Swap = Run[0]; Run[0] = Run[3]; Run[3] = Swap;
    Swap = Run[1]; Run[1] = Run[2]; Run[2] = Swap;
  }
}

void QSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field, Swap;
  register int z;

  for (z = 0; z < Cnt / 8; z++, Run += 8)
  {
    Swap = Run[0]; Run[0] = Run[7]; Run[7] = Swap;
    Swap = Run[1]; Run[1] = Run[6]; Run[6] = Swap;
    Swap = Run[2]; Run[2] = Run[5]; Run[5] = Swap;
    Swap = Run[3]; Run[3] = Run[4]; Run[4] = Swap;
  }
}

void TSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field, Swap;
  register int z;

  for (z = 0; z < Cnt / 10; z++, Run += 10)
  {
    Swap = Run[0]; Run[0] = Run[9]; Run[9] = Swap;
    Swap = Run[1]; Run[1] = Run[8]; Run[8] = Swap;
    Swap = Run[2]; Run[2] = Run[7]; Run[7] = Swap;
    Swap = Run[3]; Run[3] = Run[6]; Run[6] = Swap;
    Swap = Run[4]; Run[4] = Run[5]; Run[5] = Swap;
  }
}

void DWSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field, Swap;
  register int z;

  for (z = 0; z < Cnt / 4; z++, Run += 4)
  {
    Swap = Run[0]; Run[0] = Run[2]; Run[2] = Swap;
    Swap = Run[1]; Run[1] = Run[3]; Run[3] = Swap;
  }
}

void QWSwap(void *Field, int Cnt)
{
  register unsigned char *Run = (unsigned char *) Field, Swap;
  register int z;

  for (z = 0; z < Cnt / 8; z++, Run += 8)
  {
    Swap = Run[0]; Run[0] = Run[6]; Run[6] = Swap;
    Swap = Run[1]; Run[1] = Run[7]; Run[7] = Swap;
    Swap = Run[2]; Run[2] = Run[4]; Run[4] = Swap;
    Swap = Run[3]; Run[3] = Run[5]; Run[5] = Swap;
  }
}

void Double_2_ieee4(Double inp, Byte *dest, Boolean NeedsBig)
{
#ifdef IEEEFLOAT
  Single tmp = inp;
  memcpy(dest, &tmp, 4);
  if (BigEndian != NeedsBig)
    DSwap(dest, 4);
#endif
#ifdef VAXFLOAT
  Single tmp = inp / 4;
  memcpy(dest, &tmp, 4);
  WSwap(dest, 4);
  if (!NeedsBig)
    DSwap(dest, 4);
#endif
}

void Double_2_ieee8(Double inp, Byte *dest, Boolean NeedsBig)
{
#ifdef IEEEFLOAT
  memcpy(dest, &inp, 8);
  if (BigEndian != NeedsBig)
    QSwap(dest, 8);
#endif
#ifdef VAXFLOAT
  Byte tmp[8];
  Word Exp;
  int z;
  Boolean cont;

  memcpy(tmp, &inp, 8);
  WSwap(tmp, 8);
  Exp = ((tmp[0] << 1) & 0xfe) + (tmp[1] >> 7);
  Exp += 894; /* =1023-192 */
  tmp[1] &= 0x7f;
  if ((tmp[7] & 7) > 4)
  {
    for (tmp[7] += 8, cont = tmp[7] < 8, z = 0; cont && z > 1; z--)
    {
      tmp[z]++;
      cont = (tmp[z] == 0);
    }
    if (cont)
    {
      tmp[1]++;
      if (tmp[1] > 127)
        Exp++;
    }
  }
  dest[7] = (tmp[0] & 0x80) + ((Exp >> 4) & 0x7f);
  dest[6] = ((Exp & 0x0f) << 4) + ((tmp[1] >> 3) & 0x0f);
  for (z = 5; z >= 0; z--)
    dest[z] = ((tmp[6 - z] & 7) << 5) | ((tmp[7 - z] >> 3) & 0x1f);
  if (NeedsBig)
    QSwap(dest, 8);
#endif
}

void Double_2_ieee10(Double inp, Byte *dest, Boolean NeedsBig)
{
  Byte Buffer[8];
  Byte Sign;
  Word Exponent;
  int z;

#ifdef IEEEFLOAT
  Boolean Denormal;

  memcpy(Buffer, &inp, 8);
  if (BigEndian)
    QSwap(Buffer, 8);
  Sign = (Buffer[7] & 0x80);
  Exponent = (Buffer[6] >> 4) + (((Word) Buffer[7] & 0x7f) << 4);
  Denormal = (Exponent == 0);
  if (Exponent == 2047)
    Exponent = 32767;
  else Exponent += (16383 - 1023);
  Buffer[6] &= 0x0f;
  if (!Denormal)
    Buffer[6] |= 0x10;
  for (z = 7; z >= 2; z--)
    dest[z] = ((Buffer[z - 1] & 0x1f) << 3) | ((Buffer[z - 2] & 0xe0) >> 5);
  dest[1] = (Buffer[0] & 0x1f) << 3;
  dest[0] = 0;
#endif
#ifdef VAXFLOAT
  memcpy(Buffer, &inp, 8);
  WSwap(Buffer, 8);
  Sign = (*Buffer) & 0x80;
  Exponent = ((*Buffer) << 1) + ((Buffer[1] & 0x80) >> 7);
  Exponent += (16383 - 129);
  Buffer[1] |= 0x80;
  for (z = 1; z < 8; z++)
    dest[z] = Buffer[8 - z];
  dest[0] = 0;
#endif
  dest[9] = Sign | ((Exponent >> 8) & 0x7f);
  dest[8] = Exponent & 0xff;
  if (NeedsBig)
    TSwap(dest, 10);
}


Boolean Read2(FILE *file, void *Ptr)
{
  if (fread(Ptr, 1, 2, file) != 2)
    return False;
  if (BigEndian)
    WSwap(Ptr, 2);
  return True;
}

Boolean Read4(FILE *file, void *Ptr)
{
  if (fread(Ptr, 1, 4, file) != 4)
    return False;
  if (BigEndian)
    DSwap(Ptr, 4);
  return True;
}

Boolean Read8(FILE *file, void *Ptr)
{
  if (fread(Ptr, 1, 8, file) != 8)
    return False;
  if (BigEndian)
    QSwap(Ptr, 8);
  return True;
}


Boolean Write2(FILE *file, void *Ptr)
{
  Boolean OK;

  if (BigEndian)
    WSwap(Ptr, 2);
  OK = (fwrite(Ptr, 1, 2, file) == 2);
  if (BigEndian)
    WSwap(Ptr, 2);
  return OK;
}

Boolean Write4(FILE *file, void *Ptr)
{
  Boolean OK;

  if (BigEndian)
    DSwap(Ptr, 4);
  OK = (fwrite(Ptr, 1, 4, file) == 4);
  if (BigEndian)
    DSwap(Ptr, 4);
  return OK;
}

Boolean Write8(FILE *file, void *Ptr)
{
  Boolean OK;

  if (BigEndian)
    QSwap(Ptr, 8);
  OK = (fwrite(Ptr, 1, 8, file) == 8);
  if (BigEndian)
    QSwap(Ptr, 8);
  return OK;
}


Word MRead2L(Byte *Buffer)
{
  return (((Word) Buffer[1]) << 8) | Buffer[0];
}

Word MRead2B(Byte *Buffer)
{
  return (((Word) Buffer[0]) << 8) | Buffer[1];
}

void MWrite2L(Byte *Buffer, Word Value)
{
  Buffer[0] = Value & 0xff;
  Buffer[1] = (Value >> 8) & 0xff;
}

void MWrite2B(Byte *Buffer, Word Value)
{
  Buffer[1] = Value & 0xff;
  Buffer[0] = (Value >> 8) & 0xff;
}

LongWord MRead4L(Byte *Buffer)
{
  return (((LongWord) Buffer[3]) << 24) |
         (((LongWord) Buffer[2]) << 16) |
         (((LongWord) Buffer[1]) << 8)  | Buffer[0];
}

LongWord MRead4B(Byte *Buffer)
{
  return (((LongWord) Buffer[0]) << 24) |
         (((LongWord) Buffer[1]) << 16) |
         (((LongWord) Buffer[2]) << 8) | Buffer[3];
}

void MWrite4L(Byte *Buffer, LongWord Value)
{
  Buffer[0] = Value & 0xff;
  Buffer[1] = (Value >> 8) & 0xff;
  Buffer[2] = (Value >> 16) & 0xff;
  Buffer[3] = (Value >> 24) & 0xff;
}

void MWrite4B(Byte *Buffer, LongWord Value)
{
  Buffer[3] = Value & 0xff;
  Buffer[2] = (Value >> 8) & 0xff;
  Buffer[1] = (Value >> 16) & 0xff;
  Buffer[0] = (Value >> 24) & 0xff;
}

#ifdef HAS64
QuadWord MRead8L(Byte *Buffer)
{
  return (((LargeWord) Buffer[7]) << 56) |
         (((LargeWord) Buffer[6]) << 48) |
         (((LargeWord) Buffer[5]) << 40) |
         (((LargeWord) Buffer[4]) << 32) |
         (((LargeWord) Buffer[3]) << 24) |
         (((LargeWord) Buffer[2]) << 16) |
         (((LargeWord) Buffer[1]) << 8)  |
                       Buffer[0];
}

QuadWord MRead8B(Byte *Buffer)
{
  return (((LargeWord) Buffer[0]) << 56) |
         (((LargeWord) Buffer[1]) << 48) |
         (((LargeWord) Buffer[2]) << 40) |
         (((LargeWord) Buffer[3]) << 32) |
         (((LargeWord) Buffer[4]) << 24) |
         (((LargeWord) Buffer[7]) << 16) |
         (((LargeWord) Buffer[6]) << 8)  |
                       Buffer[7];
}

void MWrite8L(Byte *Buffer, QuadWord Value)
{
  Buffer[0] = Value & 0xff;
  Buffer[1] = (Value >> 8) & 0xff;
  Buffer[2] = (Value >> 16) & 0xff;
  Buffer[3] = (Value >> 24) & 0xff;
  Buffer[4] = (Value >> 32) & 0xff;
  Buffer[5] = (Value >> 40) & 0xff;
  Buffer[6] = (Value >> 48) & 0xff;
  Buffer[7] = (Value >> 56) & 0xff;
}

void MWrite8B(Byte *Buffer, QuadWord Value)
{
  Buffer[7] = Value & 0xff;
  Buffer[6] = (Value >> 8) & 0xff;
  Buffer[5] = (Value >> 16) & 0xff;
  Buffer[4] = (Value >> 24) & 0xff;
  Buffer[3] = (Value >> 32) & 0xff;
  Buffer[2] = (Value >> 40) & 0xff;
  Buffer[1] = (Value >> 48) & 0xff;
  Buffer[0] = (Value >> 56) & 0xff;
}
#endif


static void CheckSingle(int Is, int Should, char *Name)
{
  if (Is != Should)
  {
    fprintf(stderr, "Configuration error: Sizeof(%s) is %d, should be %d\n",
            Name, Is, Should);
    exit(255);
  }
}

static void CheckDataTypes(void)
{
  int intsize = sizeof(int);

  if (intsize < 2)
  {
    fprintf(stderr, "Configuration error: Sizeof(int) is %d, should be >=2\n",
            (int) sizeof(int));
    exit(255);
  }
  CheckSingle(sizeof(Byte),    1, "Byte");
  CheckSingle(sizeof(ShortInt), 1, "ShortInt");
#ifdef HAS16
  CheckSingle(sizeof(Word),    2, "Word");
  CheckSingle(sizeof(Integer), 2, "Integer");
#endif
  CheckSingle(sizeof(LongInt), 4, "LongInt");
  CheckSingle(sizeof(LongWord), 4, "LongWord");
#ifdef HAS64
  CheckSingle(sizeof(QuadInt), 8, "QuadInt");
  CheckSingle(sizeof(QuadWord), 8, "QuadWord");
#endif
  CheckSingle(sizeof(Single),  4, "Single");
  CheckSingle(sizeof(Double),  8, "Double");
}


static char *AssignSingle(int size)
{
  if (size == sizeof(short))
    return "%d";
  else if (size == sizeof(int))
    return "%d";
  else if (size == sizeof(long))
    return "%ld";
#ifndef NOLONGLONG
  else if (size == sizeof(long long))
    return "%lld";
#endif
  else
  {
    fprintf(stderr,
            "Configuration error: cannot assign format string for integer of size %d\n", size);
    exit(255);
    return "";
  }
}

static char *AssignHSingle(int size)
{
  if (size == sizeof(short))
    return "%x";
  else if (size == sizeof(int))
    return "%x";
  else if (size == sizeof(long))
    return "%lx";
#ifndef NOLONGLONG
  else if (size == sizeof(long long))
    return "%llx";
#endif
  else
  {
    fprintf(stderr,
            "Configuration error: cannot assign format string for integer of size %d\n", size);
    exit(255);
    return "";
  }
}

static void AssignFormats(void)
{
#ifdef HAS16
  IntegerFormat = Integ16Format = AssignSingle(2);
#endif
  LongIntFormat = Integ32Format = AssignSingle(4);
#ifdef HAS64
  QuadIntFormat = Integ64Format = AssignSingle(8);
#endif
  LargeIntFormat = AssignSingle(sizeof(LargeInt));
  LargeHIntFormat = AssignHSingle(sizeof(LargeInt));
}

void endian_init(void)
{
  union
  {
    unsigned char field[sizeof(int)];
    int test;
  } TwoFace;

  CheckDataTypes();
  AssignFormats();

  memset(TwoFace.field, 0, sizeof(int));
  TwoFace.field[0] = 1;
  BigEndian = ((TwoFace.test) != 1);
}
