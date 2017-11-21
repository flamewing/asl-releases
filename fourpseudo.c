/* intpseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Pseudo Instructions used by some 4 bit devices                            */
/*                                                                           */
/*****************************************************************************/
/* $Id: fourpseudo.c,v 1.1 2016/10/07 19:40:42 alfred Exp $                 */
/***************************************************************************** 
 * $Log: fourpseudo.c,v $
 * Revision 1.1  2016/10/07 19:40:42  alfred
 * - add missing files
 *
 *****************************************************************************/

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "bpemu.h"
#include "endian.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "errmsg.h"

#include "fourpseudo.h"

void DecodeRES(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    Boolean ValOK;
    Word Size;

    FirstPassUnknown = False;
    Size = EvalIntExpression(ArgStr[1], Int16, &ValOK);
    if (FirstPassUnknown) WrError(1820);
    if ((ValOK) && (!FirstPassUnknown))
    {
      DontPrint = True;
      if (!Size) WrError(290);
      CodeLen = Size;
      BookKeeping();
    }
  }
}

void DecodeDATA(IntType CodeIntType, IntType DataIntType)
{
  Boolean ValOK;
  TempResult t;
  int z;
  IntType ValIntType = (ActPC == SegData) ? DataIntType : CodeIntType;
  LargeWord ValMask = IntTypeDefs[ValIntType].Mask;
  LargeWord UnknownMask = ValMask / 2;

  if (ChkArgCnt(1, ArgCntMax))
  {
    ValOK = True;
    for (z = 1; ValOK && (z <= ArgCnt); z++)
    {
      FirstPassUnknown = False;
      EvalExpression(ArgStr[z], &t);
      if ((t.Typ == TempInt) && (FirstPassUnknown))
        t.Contents.Int &= UnknownMask;

      switch (t.Typ)
      {
        case TempInt:
          if (!RangeCheck(t.Contents.Int, ValIntType))
          {
            WrError(1320);
            ValOK = False;
          }
          else if (ValMask <= 0xff)
            BAsmCode[CodeLen++] = t.Contents.Int & ValMask;
          else if (ValMask <= 0xfffful)
            WAsmCode[CodeLen++] = t.Contents.Int & ValMask;
          else
            DAsmCode[CodeLen++] = t.Contents.Int & ValMask;
          break;
        case TempFloat:
          WrError(1135);
          ValOK = False;
          break;
        case TempString:
        {
          unsigned char *cp;
          int z2, bpos;
          LongWord TransCh;

          for (z2 = 0, cp = (unsigned char *)t.Contents.Ascii.Contents, bpos = 0;
               z2 < t.Contents.Ascii.Length;
               z2++, cp++)
          {
            TransCh = CharTransTable[((usint)*cp) & 0xff];

            /* word width 24..31 bits: pack three characters into one dword */

            if (ValMask >= 0xfffffful) 
            {
              if (!bpos)
                DAsmCode[CodeLen++] = TransCh;
              else if (1 == bpos)
                DAsmCode[CodeLen - 1] |= TransCh << 8;
              else
                DAsmCode[CodeLen - 1] |= TransCh << 16;
              if (++bpos >= 3)
                bpos = 0;
            }

            /* word width 17..23 bits: pack two characters into one dword */

            else if (ValMask > 0xfffful)
            {
              if (!bpos)
                DAsmCode[CodeLen++] = TransCh;
              else
                DAsmCode[CodeLen - 1] |= TransCh << 8;
              if (++bpos >= 2)
                bpos = 0;
            }

            /* word width 16 bits: pack two characters into one word */
 
            else if (ValMask == 0xfffful)
            {
              if (!bpos)
                WAsmCode[CodeLen++] = TransCh;
              else
                WAsmCode[CodeLen - 1] |= TransCh << 8;
              if (++bpos >= 2)
                bpos = 0;
            }

            /* word width 9..15 bits: pack one character into one word */

            else if (ValMask > 0xff)
              WAsmCode[CodeLen++] = TransCh;

            /* word width 8 bits: pack one character into one byte */

            else if (ValMask == 0xff)
              BAsmCode[CodeLen++] = TransCh;

            /* word width 4...7 bits: pack one character into two nibbles */

            else
            {
              BAsmCode[CodeLen++] = TransCh >> 4;
              BAsmCode[CodeLen++] = TransCh & 15;
            }
          }
          break;
        }
        default:
          ValOK = False;
      }
    }
    if (!ValOK)
      CodeLen = 0;
  }
}
