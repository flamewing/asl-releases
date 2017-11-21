/* code96c141.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TLCS-900(L)                                                 */
/*                                                                           */
/* Historie: 27. 6.1996 Grundsteinlegung                                     */
/*            9. 1.1999 ChkPC jetzt mit Adresse als Parameter                */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           22. 5.2000 fixed decoding xhl0...xhl3                           */
/*           14. 6.2000 fixed coding of minc/mdec                            */
/*           18. 6.2000 fixed coding of bs1b                                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           19. 8.2001 fixed errors for lower halves of XIX...XSP           */
/*                                                                           */
/*****************************************************************************/
/* $Id: code96c141.c,v 1.13 2017/06/07 19:49:28 alfred Exp $                          */
/*****************************************************************************
 * $Log: code96c141.c,v $
 * Revision 1.13  2017/06/07 19:49:28  alfred
 * - resolve CPL conflict
 *
 * Revision 1.12  2015/05/25 08:38:35  alfred
 * - silence come warnings on old GCC versions
 *
 * Revision 1.11  2014/12/07 19:14:00  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.10  2014/08/11 21:10:27  alfred
 * - adapt to current style
 *
 * Revision 1.9  2010/08/27 14:52:42  alfred
 * - some more overlapping strcpy() cleanups
 *
 * Revision 1.8  2007/11/24 22:48:06  alfred
 * - some NetBSD changes
 *
 * Revision 1.7  2006/08/05 18:06:05  alfred
 * - remove debug printf
 *
 * Revision 1.6  2006/08/05 12:20:03  alfred
 * - regard spaces in indexed expressions for TLCS-900
 *
 * Revision 1.5  2005/09/08 16:53:42  alfred
 * - use common PInstTable
 *
 * Revision 1.4  2005/09/08 16:10:03  alfred
 * - fix Qxxn register decoding
 *
 * Revision 1.3  2004/11/16 17:43:09  alfred
 * - add missing hex mark on short mask
 *
 * Revision 1.2  2004/05/29 11:33:02  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:22  alfred
 * - recreated
 *
 * Revision 1.2  2002/10/20 09:22:25  alfred
 * - work around the parser problem related to the ' character
 *
 * Revision 1.7  2002/10/07 20:25:01  alfred
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "errmsg.h"
#include "codepseudo.h"
#include "intpseudo.h"
#include "asmitree.h"
#include "codevars.h"
#include "errmsg.h"

#include "code96c141.h"

/*-------------------------------------------------------------------------*/
/* Daten */

typedef struct 
{
  char *Name;
  Word Code;
  Byte CPUFlag;
  Boolean InSup;
} FixedOrder;

typedef struct 
{
  char *Name;
  Word Code;
  Boolean InSup;
  Byte MinMax,MaxMax;
  ShortInt Default;
} ImmOrder;

typedef struct
{
  char *Name;
  Word Code;
  Byte OpMask;
} RegOrder;

typedef struct
{
  char *Name;
  Byte Code;
} Condition;

#define FixedOrderCnt 13
#define ImmOrderCnt 3 
#define RegOrderCnt 8
#define ConditionCnt 24

#define ModNone (-1)
#define ModReg 0
#define MModReg (1  << ModReg)
#define ModXReg 1
#define MModXReg (1 << ModXReg)
#define ModMem 2
#define MModMem (1  << ModMem)
#define ModImm 3
#define MModImm (1  << ModImm)
#define ModCReg 4
#define MModCReg (1 << ModCReg)

static FixedOrder *FixedOrders;
static RegOrder *RegOrders;
static ImmOrder *ImmOrders;
static Condition *Conditions;
static LongInt DefaultCondition;

static ShortInt AdrType;
static ShortInt OpSize;        /* -1/0/1/2 = nix/Byte/Word/Long */
static Byte AdrMode;
static Byte AdrVals[10];
static Boolean MinOneIs0;

static CPUVar CPU96C141,CPU93C141;

/*---------------------------------------------------------------------------*/
/* Adressparser */

static Boolean IsRegBase(Byte No, Byte Size)
{
  return ((Size == 2)
       || ((Size == 1) && (No < 0xf0) && (!Maximum) && ((No & 3) == 0)));
}

static void ChkMaximum(Boolean MustMax, Byte *Result)
{
  if (Maximum != MustMax)
  {
    *Result = 1;
    WrError((MustMax) ? 1997 : 1996);
  }
}

static Boolean IsQuot(char Ch)
{
  return ((Ch == '\'') || (Ch == '`'));
}

static Byte CodeEReg(char *Asc, Byte *ErgNo, Byte *ErgSize)
{
#define RegCnt 8
  static char Reg8Names[RegCnt+1] = "AWCBEDLH";
  static char *Reg16Names[RegCnt] =
  {
    "WA" ,"BC" ,"DE" ,"HL" ,"IX" ,"IY" ,"IZ" ,"SP"
  };
  static char *Reg32Names[RegCnt] =
  {
    "XWA","XBC","XDE","XHL","XIX","XIY","XIZ","XSP"
  };

  int z, l = strlen(Asc);
  char *pos;
  String HAsc, Asc_N;
  Byte Result;

  strmaxcpy(Asc_N, Asc, 255);
  NLS_UpString(Asc_N);
  Asc = Asc_N;

  Result = 2;

  /* mom. Bank ? */

  if (l == 1)
  {
    pos = strchr(Reg8Names, *Asc);
    if (pos)
    {
     z = pos - Reg8Names;
     *ErgNo = 0xe0 + ((z & 6) << 1) + (z & 1);
     *ErgSize = 0;
     return Result;
    }
  }
  for (z = 0; z < RegCnt; z++)
  {
    if (!strcmp(Asc, Reg16Names[z]))
    {
      *ErgNo = 0xe0 + (z << 2);
      *ErgSize = 1;
      return Result;
    }
    if (!strcmp(Asc, Reg32Names[z]))
    {
      *ErgNo = 0xe0 + (z << 2);
      *ErgSize = 2;
      if (z < 4)
        ChkMaximum(True, &Result);
      return Result;
    }
  }

  /* Bankregister, 8 Bit ? */

  if ((l == 3) && ((*Asc == 'Q') || (*Asc == 'R')) && ((Asc[2] >= '0') && (Asc[2] <= '7')))
  {
    for (z = 0; z < RegCnt; z++)
      if (Asc[1] == Reg8Names[z])
      {
        *ErgNo = ((Asc[2] - '0') << 4) + ((z & 6) << 1) + (z & 1);
        if (*Asc == 'Q')
        {
          *ErgNo |= 2;
          ChkMaximum(True, &Result);
        }
        if (((*Asc == 'Q') || (Maximum)) && (Asc[2] > '3'))
        {
          WrError(1320);
          Result = 1;
        }
        *ErgSize = 0;
        return Result;
      }
  }

  /* Bankregister, 16 Bit ? */

  if ((l == 4) && ((*Asc == 'Q') || (*Asc == 'R')) && ((Asc[3] >= '0') && (Asc[3] <= '7')))
  {
    strcpy(HAsc, Asc + 1);
    HAsc[2] = '\0';
    for (z = 0; z < RegCnt >> 1; z++)
      if (!strcmp(HAsc, Reg16Names[z]))
      {
        *ErgNo = ((Asc[3] - '0') << 4) + (z << 2);
        if (*Asc == 'Q')
        {
          *ErgNo |= 2;
          ChkMaximum(True, &Result);
        }
        if (((*Asc == 'Q') || (Maximum)) && (Asc[3] > '3'))
        {
          WrError(1320);
          Result = 1;
        }
        *ErgSize = 1;
         return Result;
      }
  }

  /* Bankregister, 32 Bit ? */

  if ((l == 4) && ((Asc[3] >= '0') && (Asc[3] <= '7')))
  {
    for (z = 0; z < RegCnt >> 1; z++)
     if (strncmp(Asc, Reg32Names[z], 3) == 0)
     {
       *ErgNo = ((Asc[3] - '0') << 4) + (z << 2);
       ChkMaximum(True, &Result);
       if (Asc[3] > '3')
       {
         WrError(1320); Result = 1;
       }
       *ErgSize = 2;
       return Result;
     }
  }

  /* obere 8-Bit-Haelften momentaner Bank ? */

  if ((l == 2) && (*Asc == 'Q'))
   for (z = 0; z < RegCnt; z++)
    if (Asc[1] == Reg8Names[z])
    {
      *ErgNo = 0xe2 + ((z & 6) << 1) + (z & 1);
      ChkMaximum(True, &Result);
      *ErgSize = 0;
      return Result;
    }

  /* obere 16-Bit-Haelften momentaner Bank und von XIX..XSP ? */

  if ((l == 3) && (*Asc == 'Q'))
  {
    for (z = 0; z < RegCnt; z++)
      if (!strcmp(Asc + 1, Reg16Names[z]))
      {
        *ErgNo = 0xe2 + (z << 2);
        if (z < 4) ChkMaximum(True, &Result);
        *ErgSize = 1;
        return Result;
      }
  }

  /* 8-Bit-Teile von XIX..XSP ? */

  if (((l == 3) || ((l == 4) && (*Asc == 'Q')))
  && ((Asc[l - 1] == 'L') || (Asc[l - 1] == 'H')))
  {
    strcpy(HAsc, Asc + (l - 3)); HAsc[2] = '\0';
    for (z = 0; z < RegCnt >> 1; z++)
      if (!strcmp(HAsc, Reg16Names[z + 4]))
      {
        *ErgNo = 0xf0 + (z << 2) + ((l - 3) << 1) + (Ord(Asc[l - 1] == 'H'));
        *ErgSize = 0;
        return Result;
      }
  }

  /* 8-Bit-Teile vorheriger Bank ? */

  if (((l == 2) || ((l == 3) && (*Asc == 'Q'))) && (IsQuot(Asc[l - 1])))
   for (z = 0; z < RegCnt; z++)
     if (Asc[l - 2] == Reg8Names[z])
     {
       *ErgNo = 0xd0 + ((z & 6) << 1) + ((strlen(Asc) - 2) << 1) + (z & 1);
       if (l == 3) ChkMaximum(True, &Result);
       *ErgSize = 0;
       return Result;
     }

  /* 16-Bit-Teile vorheriger Bank ? */

  if (((l == 3) || ((l == 4) && (*Asc == 'Q'))) && (IsQuot(Asc[l - 1])))
  {
    strcpy(HAsc, Asc + 1);
    HAsc[l - 2] = '\0';
    for (z = 0; z < RegCnt >> 1; z++)
      if (!strcmp(HAsc, Reg16Names[z]))
      {
        *ErgNo = 0xd0 + (z << 2) + ((strlen(Asc) - 3) << 1);
        if (l == 4) ChkMaximum(True, &Result);
        *ErgSize = 1;
        return Result;
      }
  }

  /* 32-Bit-Register vorheriger Bank ? */

  if ((l == 4) && (IsQuot(Asc[3])))
  {
    strcpy(HAsc, Asc); HAsc[3] = '\0';
    for (z = 0; z < RegCnt >> 1; z++)
      if (!strcmp(HAsc, Reg32Names[z]))
      {
        *ErgNo = 0xd0 + (z << 2);
        ChkMaximum(True, &Result);
        *ErgSize = 2;
        return Result;
      }
  }

  return (Result = 0);
}

static void ChkL(CPUVar Must, Byte *Result)
{
  if (MomCPU != Must)
  {
    WrError(1440);
    *Result = 0;
  }
}

static Byte CodeCReg(char *Asc, Byte *ErgNo, Byte *ErgSize)
{
  Byte Result = 2;

  if (!strcasecmp(Asc, "NSP"))
  {
    *ErgNo = 0x3c;
    *ErgSize = 1;
    ChkL(CPU96C141, &Result);
    return Result;
  }
  if (!strcasecmp(Asc, "XNSP"))
  {
    *ErgNo = 0x3c;
    *ErgSize = 2;
    ChkL(CPU96C141, &Result);
    return Result;
  }
  if (!strcasecmp(Asc,"INTNEST"))
  {
    *ErgNo = 0x3c;
    *ErgSize = 1;
    ChkL(CPU93C141, &Result);
    return Result;
  }
  if ((strlen(Asc) == 5)
   && (!strncasecmp(Asc, "DMA", 3))
   && (Asc[4] >= '0') && (Asc[4] <= '3'))
  {
    switch (mytoupper(Asc[3]))
    {
      case 'S':
        *ErgNo = (Asc[4] - '0') * 4;
        *ErgSize = 2;
        return Result;
      case 'D':
        *ErgNo = (Asc[4] - '0') * 4 + 0x10;
        *ErgSize = 2;
        return Result;
      case 'M':
        *ErgNo = (Asc[4] - '0') * 4 + 0x22;
        *ErgSize = 0;
        return Result;
      case 'C':
        *ErgNo = (Asc[4] - '0') * 4 + 0x20;
        *ErgSize = 1;
        return Result;
    }
  }

  return (Result = 0);
}


typedef struct
{
  char *Name;
  Byte Num;
  Boolean InMax, InMin;
} RegDesc;


static void SetOpSize(ShortInt NewSize)
{
  if (OpSize == -1)
    OpSize = NewSize;
  else if (OpSize != NewSize)
  {
    WrError(1131);
    AdrType = ModNone;
  }
}

static Boolean IsRegCurrent(Byte No, Byte Size, Byte *Erg)
{
  switch (Size)
  {
    case 0:
      if ((No & 0xf2) == 0xe0)
      {
        *Erg = ((No & 0x0c) >> 1) + ((No & 1) ^ 1);
        return True;
      }
      else
        return False;
    case 1:
    case 2:
      if ((No & 0xe3) == 0xe0)
      {
        *Erg = ((No & 0x1c) >> 2);
        return True;
      }
      else
        return False;
    default:
      return False;
  }
}

static void ChkAdr(Byte Erl)
{
  if (AdrType != ModNone)
   if (!(Erl & (1 << AdrType)))
   {
     WrError(1350);
     AdrType = ModNone;
   }
}

static void DecodeAdr(char *Asc, Byte Erl)
{
  String HAsc;
  Byte HNum, HSize;
  Boolean OK, NegFlag, NNegFlag, MustInd, FirstFlag;
  Byte BaseReg, BaseSize;
  Byte IndReg, IndSize;
  Byte PartMask;
  LongInt DispPart, DispAcc;
  char *MPos, *PPos, *EPos;

  AdrType = ModNone;

  /* Register ? */

  switch (CodeEReg(Asc, &HNum, &HSize))
  {
    case 1:
      ChkAdr(Erl);
      return;
    case 2:
      if (IsRegCurrent(HNum, HSize, &AdrMode))
        AdrType = ModReg;
      else
      {
       AdrType = ModXReg;
       AdrMode = HNum;
      }
      SetOpSize(HSize);
      ChkAdr(Erl);
      return;
  }

  /* Steuerregister ? */

  if (CodeCReg(Asc, &HNum, &HSize) == 2)
  {
    AdrType = ModCReg;
    AdrMode = HNum;
    SetOpSize(HSize);
    ChkAdr(Erl);
    return;
  }

  /* Predekrement ? */

  if ((strlen(Asc) > 4)
   && (Asc[strlen(Asc) - 1] == ')')
   && (!strncmp(Asc, "(-", 2)))
  {
    strcpy(HAsc, Asc + 2);
    HAsc[strlen(HAsc) - 1] = '\0';
    if (CodeEReg(HAsc, &HNum, &HSize) != 2) WrError(1350);
    else if (!IsRegBase(HNum, HSize)) WrError(1350);
    else
    {
      AdrType = ModMem;
      AdrMode = 0x44;
      AdrCnt = 1;
      AdrVals[0] = HNum;
      if (OpSize != -1)
        AdrVals[0] += OpSize;
    }
    ChkAdr(Erl);
    return;
  }

  /* Postinkrement ? */

  if ((strlen(Asc) > 4)
   && (Asc[0] == '(')
   && (!strncmp(Asc + strlen(Asc) - 2, "+)", 2)))
  {
    strcpy(HAsc, Asc + 1);
    HAsc[strlen(HAsc) - 2] = '\0';
    if (CodeEReg(HAsc, &HNum, &HSize) != 2) WrError(1350);
    else if (!IsRegBase(HNum, HSize)) WrError(1350);
    else
    {
      AdrType = ModMem;
      AdrMode = 0x45;
      AdrCnt = 1;
      AdrVals[0] = HNum;
      if (OpSize != -1)
        AdrVals[0] += OpSize;
    }
    ChkAdr(Erl);
    return;
  }

  /* Speicheroperand ? */

  if (IsIndirect(Asc))
  {
    char *Rest, *HAsc;

    NegFlag = False;
    NNegFlag = False;
    FirstFlag = False;
    PartMask = 0;
    DispAcc = 0;
    BaseReg = IndReg = BaseSize = IndSize = 0xff;
    Rest = Asc + 1;
    Rest[strlen(Rest) - 1] = '\0';

    do
    {
      MPos = QuotPos(Rest, '-');
      PPos = QuotPos(Rest, '+');
      if ((PPos) && ((!MPos) || (PPos<MPos)))
      {
        EPos = PPos;
        NNegFlag = False;
      }
      else if ((MPos) && ((!PPos) || (MPos<PPos)))
      {
        EPos = MPos;
        NNegFlag = True;
      }
      else
        EPos = NULL;
      if ((EPos == Rest) || (EPos == Rest + strlen(Rest) - 1))
      {
        WrError(1350);
        return;
      }
      HAsc = Rest;
      if (EPos)
      {
        *EPos = '\0';
        Rest = EPos + 1;
      }
      else
        Rest = NULL;
      KillPrefBlanks(HAsc);
      KillPostBlanks(HAsc);

      switch (CodeEReg(HAsc, &HNum, &HSize))
      {
        case 0:
          FirstPassUnknown = False;
          DispPart = EvalIntExpression(HAsc, Int32, &OK);
          if (FirstPassUnknown) FirstFlag = True;
          if (!OK) return;
          if (NegFlag)
            DispAcc -= DispPart;
          else
            DispAcc += DispPart;
          PartMask |= 1;
          break;
        case 1:
          break;
        case 2:
          if (NegFlag)
          {
            WrError(1350); return;
          }
          else
          {
            if (HSize == 0)
              MustInd = True;
            else if (HSize == 2)
              MustInd = False;
            else if (!IsRegBase(HNum, HSize))
              MustInd = True;
            else if (PartMask & 4)
              MustInd = True;
            else
              MustInd = False;
            if (MustInd)
            {
              if (PartMask & 2)
              {
                WrError(1350); return;
              }
              else
              {
                IndReg = HNum;
                PartMask |= 2;
                IndSize = HSize;
              }
            }
            else
            {
              if (PartMask & 4)
              {
                WrError(1350); return;
              }
              else
              {
                BaseReg = HNum;
                PartMask |= 4;
                BaseSize = HSize;
              }
            }
          }
          break;
      }

      NegFlag = NNegFlag;
      NNegFlag = False;
    }
    while (Rest);

    if ((DispAcc == 0) && (PartMask != 1))
      PartMask &= 6;
    if ((PartMask == 5) && (FirstFlag))
      DispAcc &= 0x7fff;

    switch (PartMask)
    {
      case 0:
      case 2:
      case 3:
      case 7:
        WrError(1350);
        break;
      case 1:
        if (DispAcc<=0xff)
        {
          AdrType = ModMem;
          AdrMode = 0x40;
          AdrCnt = 1;
          AdrVals[0] = DispAcc;
        }
        else if (DispAcc < 0xffff)
        {
          AdrType = ModMem;
          AdrMode = 0x41;
          AdrCnt = 2;
          AdrVals[0] = Lo(DispAcc);
          AdrVals[1] = Hi(DispAcc);
        }
        else if (DispAcc<0xffffff)
        {
          AdrType = ModMem;
          AdrMode = 0x42;
          AdrCnt = 3;
          AdrVals[0] = DispAcc         & 0xff;
          AdrVals[1] = (DispAcc >>  8) & 0xff;
          AdrVals[2] = (DispAcc >> 16) & 0xff;
        }
        else WrError(1925);
        break;
      case 4:
        if (IsRegCurrent(BaseReg, BaseSize, &AdrMode))
        {
          AdrType = ModMem;
          AdrCnt = 0;
        }
        else
        {
          AdrType = ModMem;
          AdrMode = 0x43;
          AdrCnt = 1;
          AdrVals[0] = BaseReg;
        }
        break;
      case 5:
        if ((DispAcc <= 127) && (DispAcc >= -128) && (IsRegCurrent(BaseReg, BaseSize, &AdrMode)))
        {
          AdrType = ModMem;
          AdrMode += 8;
          AdrCnt = 1;
          AdrVals[0] = DispAcc & 0xff;
        }
        else if ((DispAcc <= 32767) && (DispAcc >= -32768))
        {
          AdrType = ModMem;
          AdrMode = 0x43;
          AdrCnt = 3;
          AdrVals[0] = BaseReg + 1;
          AdrVals[1] = DispAcc & 0xff;
          AdrVals[2] = (DispAcc >> 8) & 0xff;
        }
        else WrError(1320);
        break;
      case 6:
        AdrType = ModMem;
        AdrMode = 0x43;
        AdrCnt = 3;
        AdrVals[0] = 3 + (IndSize << 2);
        AdrVals[1] = BaseReg;
        AdrVals[2] = IndReg;
        break;
    }
    ChkAdr(Erl); return;
  }

  /* bleibt nur noch immediate... */

  if ((MinOneIs0) && (OpSize == -1))
    OpSize = 0;
  switch (OpSize)
  {
    case -1:
      WrError(1132); 
      break;
    case 0:
      AdrVals[0] = EvalIntExpression(Asc, Int8, &OK);
      if (OK)
      {
        AdrType = ModImm;
        AdrCnt = 1;
      }
      break;
    case 1:
      DispAcc = EvalIntExpression(Asc, Int16, &OK);
      if (OK)
      {
        AdrType = ModImm;
        AdrCnt = 2;
        AdrVals[0] = Lo(DispAcc);
        AdrVals[1] = Hi(DispAcc);
      }
      break;
    case 2:
      DispAcc = EvalIntExpression(Asc, Int32, &OK);
      if (OK)
      {
        AdrType = ModImm;
        AdrCnt = 4;
        AdrVals[0] = Lo(DispAcc);
        AdrVals[1] = Hi(DispAcc);
        AdrVals[2] = Lo(DispAcc >> 16);
        AdrVals[3] = Hi(DispAcc >> 16);
      }
      break;
  }
}

/*---------------------------------------------------------------------------*/

static void CorrMode(Byte Ref, Byte Adr)
{
  if ((BAsmCode[Ref] & 0x4e) == 0x44)
    BAsmCode[Adr] = (BAsmCode[Adr] & 0xfc) | OpSize;
}

static Boolean ArgPair(const char *Val1, const char *Val2)
{
  return  (((!strcasecmp(ArgStr[1], Val1)) && (!strcasecmp(ArgStr[2], Val2)))
        || ((!strcasecmp(ArgStr[1], Val2)) && (!strcasecmp(ArgStr[2], Val1))));
}

static LongInt ImmVal(void)
{
  LongInt tmp;

  tmp = AdrVals[0];
  if (OpSize >= 1)
    tmp += ((LongInt)AdrVals[1]) << 8;
  if (OpSize == 2)
  {
    tmp += ((LongInt)AdrVals[2]) << 16;
    tmp += ((LongInt)AdrVals[3]) << 24;
  }
  return tmp;
}

static Boolean IsPwr2(LongInt Inp, Byte *Erg)
{
  LongInt Shift;

  Shift = 1;
  *Erg = 0;
  do
  {
    if (Inp == Shift)
      return True;
    Shift += Shift;
    (*Erg)++;
  }
  while (Shift != 0);
  return False;
}

static Boolean IsShort(Byte Code)
{
  return ((Code & 0x4e) == 0x40);
}

static void CheckSup(void)
{
  if ((MomCPU == CPU96C141)
   && (!SupAllowed))
    WrError(50);
}

static int DecodeCondition(const char *pAsc)
{
  int z;

  for (z = 0; (z < ConditionCnt) && (strcasecmp(pAsc, Conditions[z].Name)); z++);
  return z;
}

static void SetInstrOpSize(Byte Size)
{
  if (Size != 255)
    OpSize = Size;
}


/*---------------------------------------------------------------------------*/

static void DecodeMULA(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], MModReg | MModXReg);
    if ((AdrType != ModNone) && (OpSize != 2)) WrError(1130);
    else switch (AdrType)
    {
      case ModReg:
        CodeLen = 2;
        BAsmCode[0] = 0xd8 + AdrMode;
        BAsmCode[1] = 0x19;
        break;
      case ModXReg:
        CodeLen = 3;
        BAsmCode[0] = 0xd7;
        BAsmCode[1] = AdrMode;
        BAsmCode[2] = 0x19;
        break;
    }
  }
}

static void DecodeJPCALL(Word Index)
{
  int z;

  if (ChkArgCnt(1, 2))
  {
    z = (ArgCnt == 1) ? DefaultCondition : DecodeCondition(ArgStr[1]);
    if (z >= ConditionCnt) WrXError(1360, ArgStr[1]);
    else
    {
      OpSize = 2;
      DecodeAdr(ArgStr[ArgCnt], MModMem | MModImm);
      if (AdrType == ModImm) 
      {
        if (AdrVals[3] != 0) 
        {
          WrError(1320);
          AdrType=ModNone;
        }
        else if (AdrVals[2] != 0) 
        {
          AdrType = ModMem;
          AdrMode = 0x42;
          AdrCnt = 3;
        }
        else
        {
          AdrType = ModMem;
          AdrMode = 0x41;
          AdrCnt = 2;
        }
      }
      if (AdrType == ModMem) 
      {
        if ((z == DefaultCondition) && ((AdrMode == 0x41) || (AdrMode == 0x42))) 
        {
          CodeLen = 1 + AdrCnt;
          BAsmCode[0] = 0x1a + 2 * Index + (AdrCnt - 2);
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        }
        else
        {
          CodeLen = 2 + AdrCnt;
          BAsmCode[0] = 0xb0 + AdrMode;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          BAsmCode[1 + AdrCnt] = 0xd0 + (Index << 4) + (Conditions[z].Code);
        }
      }
    }
  }
}

static void DecodeJR(Word Index)
{
  Boolean OK;
  int z;
  LongInt AdrLong;

  if (ChkArgCnt(1, 2))
  {
    z = (ArgCnt==1) ? DefaultCondition : DecodeCondition(ArgStr[1]);
    if (z>=ConditionCnt) WrXError(1360, ArgStr[1]);
    else
    {
      AdrLong = EvalIntExpression(ArgStr[ArgCnt], Int32, &OK);
      if (OK) 
      {
        if (Index==1) 
        {
          AdrLong -= EProgCounter() + 3;
          if (((AdrLong > 0x7fffl) || (AdrLong < -0x8000l)) && (!SymbolQuestionable)) WrError(1330);
          else
          {
            CodeLen = 3;
            BAsmCode[0] = 0x70 + Conditions[z].Code;
            BAsmCode[1] = Lo(AdrLong);
            BAsmCode[2] = Hi(AdrLong);
            if (!FirstPassUnknown) 
            {
              AdrLong++;
              if ((AdrLong >= -128) && (AdrLong <= 127)) WrError(20);
            }
          }
        }
        else
        {
          AdrLong -= EProgCounter() + 2;
          if (((AdrLong > 127) || (AdrLong < -128)) && (!SymbolQuestionable)) WrError(1330);
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0x60 + Conditions[z].Code;
            BAsmCode[1] = Lo(AdrLong);
          }
        }
      }
    }
  }
}

static void DecodeCALR(Word Index)
{
  LongInt AdrLong;
  Boolean OK;
  UNUSED(Index);
  
  if (ChkArgCnt(1, 1))
  {
    AdrLong = EvalIntExpression(ArgStr[1], Int32, &OK) - (EProgCounter() + 3);
    if (OK) 
    {
      if (((AdrLong < -32768) || (AdrLong > 32767)) && (!SymbolQuestionable)) WrError(1330);
      else
      {
        CodeLen = 3;
        BAsmCode[0] = 0x1e;
        BAsmCode[1] = Lo(AdrLong);
        BAsmCode[2] = Hi(AdrLong);
      }
    }
  }
}

static void DecodeRET(Word Index)
{
  int z;
  UNUSED(Index);

  if (ChkArgCnt(0, 1))
  {
    z = (ArgCnt == 0) ? DefaultCondition : DecodeCondition(ArgStr[1]);
    if (z >= ConditionCnt) WrXError(1360, ArgStr[1]);
    else if (z == DefaultCondition) 
    {
      CodeLen = 1;
      BAsmCode[0] = 0x0e;
    }
    else
    {
      CodeLen = 2;
      BAsmCode[0] = 0xb0;
      BAsmCode[1] = 0xf0 + Conditions[z].Code;
    }
  }
}

static void DecodeRETD(Word Index)
{
  Word AdrWord;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (OK) 
    {
      CodeLen = 3;
      BAsmCode[0] = 0x0f;
      BAsmCode[1] = Lo(AdrWord);
      BAsmCode[2] = Hi(AdrWord);
    }
  }
}

static void DecodeDJNZ(Word Index)
{
  LongInt AdrLong;
  Boolean OK; 
  UNUSED(Index);

  if (ChkArgCnt(1, 2))
  {
    if (ArgCnt == 1) 
    {
      AdrType = ModReg;
      AdrMode = 2;
      OpSize = 0;
    }
    else
      DecodeAdr(ArgStr[1], MModReg | MModXReg);
    if (AdrType != ModNone) 
    {
      if (OpSize == 2) WrError(1130);
      else
      {
        AdrLong = EvalIntExpression(ArgStr[ArgCnt], Int32, &OK) - (EProgCounter() + 3 + Ord(AdrType == ModXReg));
        if (OK) 
        {
          if (((AdrLong < -128) || (AdrLong > 127)) && (!SymbolQuestionable)) WrError(1370);
          else 
           switch (AdrType)
           {
             case ModReg:
               CodeLen = 3;
               BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
               BAsmCode[1] = 0x1c;
               BAsmCode[2] = AdrLong & 0xff;
               break;
             case ModXReg:
               CodeLen = 4;
               BAsmCode[0] = 0xc7 + (OpSize << 4);
               BAsmCode[1] = AdrMode;
               BAsmCode[2] = 0x1c;
               BAsmCode[3] = AdrLong & 0xff;
               break;
           }
        }
      }
    }
  }
}

static void DecodeEX(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  /* work around the parser problem related to the ' character */

  if (!strncasecmp(ArgStr[2], "F\'", 2))
    ArgStr[2][2] = '\0';

  if (!ChkArgCnt(2, 2));
  else if ((ArgPair("F", "F\'")) || (ArgPair("F`", "F"))) 
  {
    CodeLen = 1;
    BAsmCode[0] = 0x16;
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg | MModXReg | MModMem);
    if (OpSize == 2) WrError(1130);
    else 
    {
      switch (AdrType)
      {
        case ModReg:
          HReg = AdrMode;
          DecodeAdr(ArgStr[2], MModReg | MModXReg | MModMem);
          switch (AdrType)
          {
            case ModReg:
              CodeLen = 2;
              BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
              BAsmCode[1] = 0xb8 + HReg;
              break;
            case ModXReg:
              CodeLen = 3;
              BAsmCode[0] = 0xc7 + (OpSize << 4);
              BAsmCode[1] = AdrMode;
              BAsmCode[2] = 0xb8 + HReg;
              break;
            case ModMem:
              CodeLen = 2 + AdrCnt;
              BAsmCode[0] = 0x80 + (OpSize << 4) + AdrMode;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              BAsmCode[1 + AdrCnt] = 0x30 + HReg;
              break;
          }
          break;
        case ModXReg:
          HReg = AdrMode;
          DecodeAdr(ArgStr[2], MModReg);
          if (AdrType == ModReg) 
          {
            CodeLen = 3;
            BAsmCode[0] = 0xc7 + (OpSize << 4);
            BAsmCode[1] = HReg;
            BAsmCode[2] = 0xb8 + AdrMode;
          }
          break;
        case ModMem:
          MinOneIs0 = True;
          HReg = AdrCnt;
          BAsmCode[0] = AdrMode;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          DecodeAdr(ArgStr[2], MModReg);
          if (AdrType == ModReg) 
          {
            CodeLen = 2 + HReg;
            CorrMode(0, 1);
            BAsmCode[0] += 0x80 + (OpSize << 4);
            BAsmCode[1 + HReg] = 0x30 + AdrMode;
          }
          break;
      }
    }
  }
}

static void DecodeBS1x(Word Index)
{
  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1], "A")) WrError(1135);
  else
  {
    DecodeAdr(ArgStr[2], MModReg | MModXReg);
    if (OpSize != 1) WrError(1130);
    else switch (AdrType)
    {
      case ModReg:
        CodeLen = 2;
        BAsmCode[0] = 0xd8 + AdrMode;
        BAsmCode[1] = 0x0e + Index; /* ANSI */
        break;
      case ModXReg:
        CodeLen = 3;
        BAsmCode[0] = 0xd7;
        BAsmCode[1] = AdrMode;
        BAsmCode[2] = 0x0e +Index; /* ANSI */
        break;
    }
  }
}

static void DecodeLDA(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrType != ModNone) 
    {
      if (OpSize < 1) WrError(1130);
      else
      {
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModMem);
        if (AdrType != ModNone) 
        {
          CodeLen = 2 + AdrCnt;
          BAsmCode[0] = 0xb0 + AdrMode;
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          BAsmCode[1 + AdrCnt] = 0x20 + ((OpSize - 1) << 4) + HReg;
        }
      }
    }
  }
}

static void DecodeLDAR(Word Index)
{
  LongInt AdrLong;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    AdrLong = EvalIntExpression(ArgStr[2], Int32, &OK) - (EProgCounter() + 4);
    if (OK) 
    {
      if (((AdrLong < -32768) || (AdrLong > 32767)) && (!SymbolQuestionable)) WrError(1330);
      else
      {
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrType != ModNone) 
        {
          if (OpSize < 1) WrError(1130);
          else
          {
            CodeLen = 5;
            BAsmCode[0] = 0xf3;
            BAsmCode[1] = 0x13;
            BAsmCode[2] = Lo(AdrLong);
            BAsmCode[3] = Hi(AdrLong);
            BAsmCode[4] = 0x20 + ((OpSize - 1) << 4) + AdrMode;
          }
        }
      }
    }
  }
}

static void DecodeLDC(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1], MModReg | MModXReg| MModCReg);
    HReg = AdrMode;
    switch (AdrType)
    {
      case ModReg:
        DecodeAdr(ArgStr[2], MModCReg);
        if (AdrType != ModNone) 
        {
          CodeLen = 3;
          BAsmCode[0] = 0xc8 + (OpSize << 4) + HReg;
          BAsmCode[1] = 0x2f;
          BAsmCode[2] = AdrMode;
        }
        break;
      case ModXReg:
        DecodeAdr(ArgStr[2], MModCReg);
        if (AdrType != ModNone) 
        {
          CodeLen = 4;
          BAsmCode[0] = 0xc7 + (OpSize << 4);
          BAsmCode[1] = HReg;
          BAsmCode[2] = 0x2f;
          BAsmCode[3] = AdrMode;
        };
        break;
      case ModCReg:
        DecodeAdr(ArgStr[2], MModReg | MModXReg);
        switch (AdrType)
        {
          case ModReg:
            CodeLen = 3;
            BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
            BAsmCode[1] = 0x2e;
            BAsmCode[2] = HReg;
            break;
          case ModXReg:
            CodeLen = 4;
            BAsmCode[0] = 0xc7 + (OpSize << 4);
            BAsmCode[1] = AdrMode;
            BAsmCode[2] = 0x2e;
            BAsmCode[3] = HReg;
            break;
        }
        break;
    }
  }
}

static void DecodeLDX(Word Index)
{
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1], MModMem);
    if (AdrType != ModNone) 
    {
      if (AdrMode != 0x40) WrError(1350);
      else
      {
        BAsmCode[4] = EvalIntExpression(ArgStr[2], Int8, &OK);
        if (OK) 
        {
          CodeLen = 6;
          BAsmCode[0] = 0xf7;
          BAsmCode[1] = 0;
          BAsmCode[2] = AdrVals[0];
          BAsmCode[3] = 0;
          BAsmCode[5] = 0;
        }
      }
    }
  }
}

static void DecodeLINK(Word Index)
{
  Word AdrWord;
  Boolean OK;
  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    AdrWord = EvalIntExpression(ArgStr[2], Int16, &OK);
    if (OK) 
    {
      DecodeAdr(ArgStr[1], MModReg | MModXReg);
      if ((AdrType != ModNone) && (OpSize != 2)) WrError(1130);
      else 
       switch (AdrType)
       {
         case ModReg:
           CodeLen = 4;
           BAsmCode[0] = 0xe8 + AdrMode;
           BAsmCode[1] = 0x0c;
           BAsmCode[2] = Lo(AdrWord);
           BAsmCode[3] = Hi(AdrWord);
           break;
         case ModXReg:
           CodeLen = 5;
           BAsmCode[0] = 0xe7;
           BAsmCode[1] = AdrMode;
           BAsmCode[2] = 0x0c;
           BAsmCode[3] = Lo(AdrWord);
           BAsmCode[4] = Hi(AdrWord);
           break;
       }
    }
  }
}

static void DecodeLD(Word Code)
{
  Byte HReg;
  Boolean ShDest, ShSrc, OK;

  SetInstrOpSize(Hi(Code));

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1], MModReg | MModXReg | MModMem);
    switch (AdrType)
    {
      case ModReg:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModReg | MModXReg| MModMem| MModImm);
        switch (AdrType)
        {
          case ModReg:
            CodeLen = 2;
            BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
            BAsmCode[1] = 0x88 + HReg;
            break;
          case ModXReg:
            CodeLen = 3;
            BAsmCode[0] = 0xc7 + (OpSize << 4);
            BAsmCode[1] = AdrMode;
            BAsmCode[2] = 0x88 + HReg;
            break;
          case ModMem:
            CodeLen = 2 + AdrCnt;
            BAsmCode[0] = 0x80 + (OpSize << 4) + AdrMode;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            BAsmCode[1 + AdrCnt] = 0x20 + HReg;
            break;
          case ModImm:
          {
            LongInt ImmValue = ImmVal();

            if ((ImmValue <= 7) && (ImmValue >= 0))
            {
              CodeLen = 2;
              BAsmCode[0] = 0xc8 + (OpSize << 4) + HReg;
              BAsmCode[1] = 0xa8 + AdrVals[0];
            }
            else
            {
              CodeLen = 1 + AdrCnt;
              BAsmCode[0] = ((OpSize + 2) << 4) + HReg;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            }
            break;
          }
        }
        break;
      case ModXReg:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModReg + MModImm);
        switch (AdrType)
        {
          case ModReg:
            CodeLen = 3;
            BAsmCode[0] = 0xc7 + (OpSize << 4);
            BAsmCode[1] = HReg;
            BAsmCode[2] = 0x98 + AdrMode;
            break;
          case ModImm:
          {
            LongInt ImmValue = ImmVal();

            if ((ImmValue <= 7) && (ImmValue >= 0))
            {
              CodeLen = 3;
              BAsmCode[0] = 0xc7 + (OpSize << 4);
              BAsmCode[1] = HReg;
              BAsmCode[2] = 0xa8 + AdrVals[0];
            }
            else
            {
              CodeLen = 3 + AdrCnt;
              BAsmCode[0] = 0xc7 + (OpSize << 4);
              BAsmCode[1] = HReg;
              BAsmCode[2] = 3;
              memcpy(BAsmCode + 3, AdrVals, AdrCnt);
            }
            break; 
          }
        }
        break;
      case ModMem:
        BAsmCode[0] = AdrMode;
        HReg = AdrCnt;
        MinOneIs0 = True;
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        DecodeAdr(ArgStr[2], MModReg | MModMem | MModImm);
        switch (AdrType)
        {
         case ModReg:
           CodeLen = 2 + HReg;
           BAsmCode[0] += 0xb0;
           CorrMode(0, 1);
           BAsmCode[1 + HReg] = 0x40 + (OpSize << 4) + AdrMode;
           break;
         case ModMem:
           if (OpSize == -1) OpSize = 0;
           ShDest = IsShort(BAsmCode[0]);
           ShSrc = IsShort(AdrMode);
           if (!(ShDest || ShSrc)) WrError(1350);
           else
           {
             if ((ShDest && (!ShSrc))) OK = True;
             else if (ShSrc && (!ShDest)) OK = False;
             else if (AdrMode == 0x40) OK = True;
             else OK = False;
             if (OK)   /* dest=(dir8/16) */
             {
               CodeLen = 4 + AdrCnt;
               HReg = BAsmCode[0];
               BAsmCode[3 + AdrCnt] = (BAsmCode[0] == 0x40) ? 0 : BAsmCode[2];
               BAsmCode[2 + AdrCnt] = BAsmCode[1];
               BAsmCode[0] = 0x80 + (OpSize << 4) + AdrMode;
               AdrMode = HReg;
               CorrMode(0, 1);
               memcpy(BAsmCode + 1, AdrVals, AdrCnt);
               BAsmCode[1 + AdrCnt] = 0x19;
             }
             else
             {
               CodeLen = 4 + HReg;
               BAsmCode[2 + HReg] = AdrVals[0];
               BAsmCode[3 + HReg] = (AdrMode == 0x40) ? 0 : AdrVals[1];
               BAsmCode[0] += 0xb0;
               CorrMode(0, 1);
               BAsmCode[1 + HReg] = 0x14 + (OpSize << 1);
             }
           }
           break;
         case ModImm:
          if (BAsmCode[0] == 0x40)
          {
            CodeLen = 2 + AdrCnt;
            BAsmCode[0] = 0x08 + (OpSize << 1);
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
          }
          else
          {
            CodeLen = 2 + HReg + AdrCnt;
            BAsmCode[0] += 0xb0;
            BAsmCode[1 + HReg] = OpSize << 1;
            memcpy(BAsmCode + 2 + HReg, AdrVals, AdrCnt);
          }
          break;
        }
        break;
    }
  }
}

static void DecodeFixed(Word Index)
{
  FixedOrder *FixedZ = FixedOrders + Index;

  if (ChkArgCnt(0, 0)
   && (ChkExactCPUMask(FixedZ->CPUFlag, CPU96C141) >= 0))
  {
    if (Hi(FixedZ->Code) == 0)
    {
      CodeLen = 1;
      BAsmCode[0] = Lo(FixedZ->Code);
    }
    else
    {
      CodeLen = 2; 
      BAsmCode[0] = Hi(FixedZ->Code);
      BAsmCode[1] = Lo(FixedZ->Code);
    }
    if (FixedZ->InSup)
      CheckSup();
  }
}

static void DecodeImm(Word Index)
{
  ImmOrder *ImmZ = ImmOrders + Index;
  Word AdrWord;
  Boolean OK;

  if (ChkArgCnt((ImmZ->Default == -1) ? 1 : 0, 1))
  {
    if (ArgCnt == 0) 
    {
      AdrWord = ImmZ->Default;
      OK = True;
    }
    else
      AdrWord = EvalIntExpression(ArgStr[1], Int8, &OK);
    if (OK) 
    {
      if (((Maximum) && (AdrWord > ImmZ->MaxMax)) || ((!Maximum) && (AdrWord > ImmZ->MinMax))) WrError(1320);
      else if (Hi(ImmZ->Code) == 0) 
      {
        CodeLen = 1;
        BAsmCode[0] = Lo(ImmZ->Code) + AdrWord;
      }
      else
      {
        CodeLen = 2;
        BAsmCode[0] = Hi(ImmZ->Code);
        BAsmCode[1] = Lo(ImmZ->Code) + AdrWord;
      }
    }
    if (ImmZ->InSup)
      CheckSup();
  }
}

static void DecodeALU2(Word Code);

static void DecodeReg(Word Index)
{
  RegOrder *RegZ = RegOrders + Index;

  /* dispatch to CPL as compare-long with two args: */
 
  if ((Memo("CPL")) && (ArgCnt >= 2))
  {
    DecodeALU2(0x0207);
    return;
  }

  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], MModReg | MModXReg);
    if (AdrType != ModNone) 
    {
      if (((1 << OpSize) & RegZ->OpMask) == 0) WrError(1130);
      else if (AdrType == ModReg) 
      {
        BAsmCode[0] = Hi(RegZ->Code) + 8 + (OpSize << 4) + AdrMode;
        BAsmCode[1] = Lo(RegZ->Code);
        CodeLen = 2;
      }
      else
      {
        BAsmCode[0] = Hi(RegZ->Code) + 7 + (OpSize << 4);
        BAsmCode[1] = AdrMode;
        BAsmCode[2] = Lo(RegZ->Code);
        CodeLen = 3;
      }
    }
  }
}

static void DecodeALU2(Word Code)
{
  Byte HReg;

  SetInstrOpSize(Hi(Code));
  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1], MModReg | MModXReg | MModMem);
    switch (AdrType)
    {
      case ModReg:
        HReg=AdrMode;
        DecodeAdr(ArgStr[2],MModReg | MModXReg | MModMem | MModImm);
        switch (AdrType)
        {
          case ModReg:
            CodeLen = 2;
            BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
            BAsmCode[1] = 0x80 + (Lo(Code) << 4) + HReg;
            break;
          case ModXReg:
            CodeLen = 3;
            BAsmCode[0] = 0xc7 + (OpSize << 4);
            BAsmCode[1] = AdrMode;
            BAsmCode[2] = 0x80 + (Lo(Code) << 4) + HReg;
            break;
          case ModMem:
            CodeLen = 2 + AdrCnt;
            BAsmCode[0] = 0x80 + AdrMode + (OpSize << 4);
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            BAsmCode[1 + AdrCnt] = 0x80 + HReg + (Lo(Code) << 4);
            break;
          case ModImm:
            if ((Lo(Code) == 7) && (OpSize != 2) && (ImmVal() <= 7) && (ImmVal() >= 0))
            {
              CodeLen = 2;
              BAsmCode[0] = 0xc8 + (OpSize << 4) + HReg;
              BAsmCode[1] = 0xd8 + AdrVals[0];
            }
            else
            {
              CodeLen = 2 + AdrCnt;
              BAsmCode[0] = 0xc8 + (OpSize << 4) + HReg;
              BAsmCode[1] = 0xc8 + Lo(Code);
              memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            }
            break;
        }
        break;
      case ModXReg:
        HReg = AdrMode;
        DecodeAdr(ArgStr[2], MModImm);
        switch (AdrType)
        {
          case ModImm:
            if ((Lo(Code) == 7) && (OpSize != 2) && (ImmVal() <= 7) && (ImmVal() >= 0))
            {
              CodeLen = 3;
              BAsmCode[0] = 0xc7 + (OpSize << 4);
              BAsmCode[1] = HReg;
              BAsmCode[2] = 0xd8 + AdrVals[0];
            }
            else
            {
              CodeLen = 3 + AdrCnt;
              BAsmCode[0] = 0xc7 + (OpSize << 4);
              BAsmCode[1] = HReg;
              BAsmCode[2] = 0xc8 + Lo(Code);
              memcpy(BAsmCode + 3, AdrVals, AdrCnt);
            }
            break;
        }
        break;
      case ModMem:
        MinOneIs0 = True;
        HReg = AdrCnt;
        BAsmCode[0] = AdrMode;
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        DecodeAdr(ArgStr[2], MModReg | MModImm);
        switch (AdrType)
        {
          case ModReg:
            CodeLen = 2 + HReg;
            CorrMode(0, 1);
            BAsmCode[0] += 0x80 + (OpSize << 4);
            BAsmCode[1 + HReg] = 0x88 + (Lo(Code) << 4) + AdrMode;
            break;
          case ModImm:
            CodeLen = 2 + HReg + AdrCnt;
            BAsmCode[0] += 0x80 + (OpSize << 4);
            BAsmCode[1 + HReg] = 0x38 + Lo(Code);
            memcpy(BAsmCode + 2 + HReg, AdrVals, AdrCnt);
            break;
        };
        break;
    }
  }
}

static void DecodePUSH_POP(Word Code)
{
  SetInstrOpSize(Hi(Code));

  if (!ChkArgCnt(1, 1));
  else if (!strcasecmp(ArgStr[1], "F"))
  {
    CodeLen = 1;
    BAsmCode[0] = 0x18 + Lo(Code);
  }
  else if (!strcasecmp(ArgStr[1], "A"))
  {
    CodeLen = 1;
    BAsmCode[0] = 0x14 + Lo(Code);
  }
  else if (!strcasecmp(ArgStr[1], "SR"))
  {
    CodeLen = 1;
    BAsmCode[0] = 0x02 + Lo(Code);
    CheckSup();
  }
  else
  {
    MinOneIs0 = True;
    DecodeAdr(ArgStr[1], MModReg | MModXReg | MModMem | (Lo(Code) ? 0 : MModImm));
    switch (AdrType)
    {
      case ModReg:
        if (OpSize == 0)
        {
          CodeLen = 2;
          BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
          BAsmCode[1] = 0x04 + Lo(Code);
        }
        else
        {
          CodeLen = 1;
          BAsmCode[0] = 0x28 + (Lo(Code) << 5) + ((OpSize - 1) << 4) + AdrMode;
        }
        break;
      case ModXReg:
        CodeLen = 3;
        BAsmCode[0] = 0xc7 + (OpSize << 4);
        BAsmCode[1] = AdrMode;
        BAsmCode[2] = 0x04 + Lo(Code);
        break;
      case ModMem:
        if (OpSize == -1)
          OpSize=0;
        CodeLen = 2 + AdrCnt;
        if (Lo(Code))
          BAsmCode[0] = 0xb0 + AdrMode;
        else
          BAsmCode[0] = 0x80 + (OpSize << 4) + AdrMode;
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        if (Lo(Code))
          BAsmCode[1 + AdrCnt] = 0x04 + (OpSize << 1);
        else
          BAsmCode[1 + AdrCnt] = 0x04;
        break;
      case ModImm:
        if (OpSize == -1)
          OpSize = 0;
        BAsmCode[0] = 9 + (OpSize << 1);
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        CodeLen = 1 + AdrCnt;
        break;
    }
  }
}

static void DecodeShift(Word Code)
{
  Boolean OK;
  Byte HReg;

  SetInstrOpSize(Hi(Code));
   
  if (ChkArgCnt(1, 2))
  {
    OK = True;
    if (ArgCnt == 1) 
      HReg = 1;
    else if (!strcasecmp(ArgStr[1], "A"))
      HReg = 0xff;
    else
    {
      FirstPassUnknown = False;
      HReg = EvalIntExpression(ArgStr[1], Int8, &OK);
      if (OK) 
      {
        if (FirstPassUnknown)
          HReg &= 0x0f;
        else
        {
          if ((HReg == 0) || (HReg > 16)) 
          {
            WrError(1320);
            OK = False;
          }
          else
            HReg &= 0x0f;
        }
      }
    }
    if (OK) 
    {
      DecodeAdr(ArgStr[ArgCnt],MModReg | MModXReg | ((HReg == 0xff) ? 0 : MModMem));
      switch (AdrType)
      {
        case ModReg:
          CodeLen = 2 + Ord(HReg != 0xff);
          BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
          BAsmCode[1] = 0xe8 + Lo(Code);
          CodeLen = 2;
          if (HReg == 0xff)
            BAsmCode[1] += 0x10;
          else
            BAsmCode[CodeLen++] = HReg;
          break;
        case ModXReg:
          BAsmCode[0] = 0xc7+(OpSize << 4);
          BAsmCode[1] = AdrMode;
          BAsmCode[2] = 0xe8 + Lo(Code);
          CodeLen = 3;
          if (HReg == 0xff)
            BAsmCode[2] += 0x10;
          else
            BAsmCode[CodeLen++] = HReg;
          break;
        case ModMem:
          if (HReg != 1) WrError(1350);
          else
          {
            if (OpSize == -1)
              OpSize = 0;
            CodeLen = 2 + AdrCnt;
            BAsmCode[0] = 0x80 + (OpSize << 4) + AdrMode;
            memcpy(BAsmCode + 1 , AdrVals, AdrCnt);
            BAsmCode[1 + AdrCnt] = 0x78 + Lo(Code);
          }
          break;
      }
    }
  }
}

static void DecodeMulDiv(Word Code)
{
  Byte HReg;

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1], MModReg | MModXReg);
    if (OpSize == 0) WrError(1130);
    else
    {
      if ((AdrType == ModReg) && (OpSize == 1))
      {
        if (AdrMode > 3)
        {
          AdrType = ModXReg;
          AdrMode = 0xe0 + (AdrMode << 2);
        }
        else
          AdrMode += 1 + AdrMode;
      }
      OpSize--;
      HReg = AdrMode;
      switch (AdrType)
      {
        case ModReg:
          DecodeAdr(ArgStr[2], MModReg | MModXReg | MModMem | MModImm);
          switch (AdrType)
          {
            case ModReg:
              CodeLen = 2;
              BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
              BAsmCode[1] = 0x40 + (Code << 3) + HReg;
              break;
            case ModXReg:
              CodeLen = 3;
              BAsmCode[0] = 0xc7 + (OpSize << 4);
              BAsmCode[1] = AdrMode;
              BAsmCode[2] = 0x40 + (Code << 3) + HReg;
              break;
            case ModMem:
              CodeLen = 2 + AdrCnt;
              BAsmCode[0] = 0x80 + (OpSize << 4) + AdrMode;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              BAsmCode[1 + AdrCnt] = 0x40 + (Code << 3) + HReg;
              break;
            case ModImm:
              CodeLen = 2 + AdrCnt;
              BAsmCode[0] = 0xc8 + (OpSize << 4) + HReg;
              BAsmCode[1] = 0x08 + Code;
              memcpy(BAsmCode + 2, AdrVals, AdrCnt);
              break;
          }
          break;
        case ModXReg:
          DecodeAdr(ArgStr[2], MModImm);
          if (AdrType == ModImm) 
          {
            CodeLen = 3 + AdrCnt;
            BAsmCode[0] = 0xc7 + (OpSize << 4);
            BAsmCode[1] = HReg;
            BAsmCode[2] = 0x08 + Code;
            memcpy(BAsmCode + 3, AdrVals, AdrCnt);
          }
          break;
      }
    }
  }
}

static void DecodeBitCF(Word Code)
{
  Boolean OK;
  Byte BitPos;

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[2],MModReg | MModXReg | MModMem);
    if (AdrType!=ModNone) 
    {
      if (OpSize == 2) WrError(1130);
      else
      {
        if (AdrType == ModMem)
          OpSize = 0;
        if (!strcasecmp(ArgStr[1], "A"))
        {
          BitPos = 0xff;
          OK = True;
        }
        else
        {
          FirstPassUnknown = False;
          BitPos = EvalIntExpression(ArgStr[1], (OpSize == 0) ? UInt3 : Int4, &OK);
        }
        if (OK) 
         switch (AdrType)
         {
           case ModReg:
             BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
             BAsmCode[1] = 0x20 + Code;
             CodeLen = 2;
             if (BitPos == 0xff)
               BAsmCode[1] |= 0x08;
             else
               BAsmCode[CodeLen++] = BitPos;
             break;
           case ModXReg:
             BAsmCode[0] = 0xc7 + (OpSize << 4);
             BAsmCode[1] = AdrMode;
             BAsmCode[2] = 0x20 + Code;
             CodeLen = 3;
             if (BitPos == 0xff)
               BAsmCode[2] |= 0x08;
             else
               BAsmCode[CodeLen++] = BitPos;
             break;
           case ModMem:
             CodeLen = 2 + AdrCnt;
             BAsmCode[0] = 0xb0 + AdrMode;
             memcpy(BAsmCode + 1, AdrVals, AdrCnt);
             BAsmCode[1 + AdrCnt] = (BitPos == 0xff)
                                  ? 0x28 + Code
                                  : 0x80 + (Code << 3) + BitPos;
             break;
         }
      }
    }
  }
}

static void DecodeBit(Word Code)
{
  Boolean OK;
  Byte BitPos;

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[2], MModReg | MModXReg | MModMem);
    if (AdrType == ModMem)
      OpSize = 0;
    if (AdrType != ModNone) 
    {
      if (OpSize == 2) WrError(1130);
      else
      {
        BitPos = EvalIntExpression(ArgStr[1], (OpSize == 0) ? UInt3 : Int4, &OK);
        if (OK)
        { 
          switch (AdrType)
          {
            case ModReg:
              CodeLen = 3;
              BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
              BAsmCode[1] = 0x30 + Code;
              BAsmCode[2] = BitPos;
              break;
            case ModXReg:
              CodeLen = 4;
              BAsmCode[0] = 0xc7 + (OpSize << 4);
              BAsmCode[1] = AdrMode;
              BAsmCode[2] = 0x30 + Code;
              BAsmCode[3] = BitPos;
              break;
            case ModMem:
              CodeLen = 2 + AdrCnt;
              Code = (Code == 4) ? 0 : Code + 1;
              BAsmCode[0] = 0xb0 + AdrMode;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              BAsmCode[1 + AdrCnt] = 0xa8 + (Code << 3) + BitPos;
              break;
          }
        }
      }
    }
  }
}

static void DecodeINC_DEC(Word Code)
{
  Boolean OK;
  Byte Incr;

  SetInstrOpSize(Hi(Code));

  if (ChkArgCnt(1, 2))
  {
    FirstPassUnknown = False;
    if (ArgCnt == 1) 
    {
      Incr = 1;
      OK = True;
    }
    else
      Incr = EvalIntExpression(ArgStr[1], Int4, &OK);
    if (OK) 
    {
      if (FirstPassUnknown)
        Incr &= 7;
      else if ((Incr < 1) || (Incr > 8)) 
      {
        WrError(1320);
        OK = False;
      }
    }
    if (OK) 
    {
      Incr &= 7;    /* 8-->0 */
      DecodeAdr(ArgStr[ArgCnt], MModReg | MModXReg | MModMem);
      switch (AdrType)
      {
        case ModReg:
          CodeLen = 2;
          BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
          BAsmCode[1] = 0x60 + (Lo(Code) << 3) + Incr;
          break;
        case ModXReg:
          CodeLen = 3;
          BAsmCode[0] = 0xc7 + (OpSize << 4);
          BAsmCode[1] = AdrMode;
          BAsmCode[2] = 0x60 + (Lo(Code) << 3) + Incr;
          break;
        case ModMem:
          if (OpSize == -1)
            OpSize = 0;
          CodeLen = 2 + AdrCnt;
          BAsmCode[0] = 0x80 + AdrMode + (OpSize << 4);
          memcpy(BAsmCode + 1, AdrVals, AdrCnt);
          BAsmCode[1 + AdrCnt] = 0x60 + (Lo(Code) << 3) + Incr;
          break;
      }
    }
  }
}

static void DecodeCPxx(Word Code)
{
  Boolean OK;

  if (ChkArgCntExtEitherOr(ArgCnt, 0, 2))
  {
    OK = True;
    if (ArgCnt == 0) 
    {
      OpSize = 0;
      AdrMode = 3;
    }
    else
    {
      Byte HReg;
      int l = strlen(ArgStr[2]);
      const char *CmpStr;

      if (!strcasecmp(ArgStr[1], "A"))
        OpSize = 0;
      else if (!strcasecmp(ArgStr[1], "WA"))
        OpSize = 1;
      CmpStr = (Code & 0x02) ? "-)" : "+)";
      if (OpSize == -1) OK = False;
      else if ((l < 2) || (*ArgStr[2] != '(') || (strcasecmp(ArgStr[2] + l - 2, CmpStr))) OK = False;
      else
      {
        ArgStr[2][l - 2] = '\0';
        if (CodeEReg(ArgStr[2] + 1, &AdrMode, &HReg) != 2) OK = False;
        else if (!IsRegBase(AdrMode, HReg)) OK = False;
        else if (!IsRegCurrent(AdrMode, HReg, &AdrMode)) OK = False;
      }
      if (!OK)
        WrError(1135);
    }
    if (OK) 
    {
      CodeLen = 2;
      BAsmCode[0] = 0x80 + (OpSize << 4) + AdrMode;
      BAsmCode[1] = Code;
    }
  }
}

static void DecodeLDxx(Word Code)
{
  SetInstrOpSize(Hi(Code));

  if (OpSize == -1)
    OpSize = 0;
  if (OpSize == 2) WrError(1130);
  else if (ChkArgCntExtEitherOr(ArgCnt, 0, 2))
  {
    Boolean OK;
    Byte HReg = 0;

    if (ArgCnt == 0) 
    {
      OK = True;
    }
    else
    {
      char *CmpStr;
      int l1 = strlen(ArgStr[1]),
          l2 = strlen(ArgStr[2]);

      OK = True;
      CmpStr = (Code & 0x02) ? "-)" : "+)";
      if ((*ArgStr[1] != '(') || (*ArgStr[2] != '(')
       || (l1 < 3) || (l2 < 3)
       || (strcasecmp(ArgStr[1] + l1 - 2, CmpStr))
       || (strcasecmp(ArgStr[2] + l2 - 2, CmpStr)))
        OK = False;
      else
      {
        ArgStr[1][l1 - 2] = '\0';
        ArgStr[2][l2 - 2] = '\0';
        if ((!strcasecmp(ArgStr[1] + 1,"XIX")) && (!strcasecmp(ArgStr[2] + 1, "XIY")))
          HReg = 2;
        else if ((Maximum) && (!strcasecmp(ArgStr[1] + 1, "XDE")) && (!strcasecmp(ArgStr[2] + 1 , "XHL")));
        else if ((!Maximum) && (!strcasecmp(ArgStr[1] + 1, "DE")) && (!strcasecmp(ArgStr[2] + 1 , "HL")));
        else
          OK = False;
      }
    }
    if (!OK) WrError(1350);
    else
    {
      CodeLen = 2;
      BAsmCode[0] = 0x83 + (OpSize << 4) + HReg;
      BAsmCode[1] = Lo(Code);
    }
  }
}

static void DecodeMINC_MDEC(Word Code)
{
  if (ChkArgCnt(2, 2))
  {
    Word AdrWord;
    Boolean OK;

    AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (OK) 
    {
      Byte Pwr;
      Byte ByteSize = 1 << (Code & 3);

      if (!IsPwr2(AdrWord, &Pwr)) WrError(1135);
      else if ((Pwr == 0) || ((ByteSize == 2) && (Pwr < 2)) || ((ByteSize == 4) && (Pwr < 3))) WrError(1135);
      else
      {
        AdrWord -= ByteSize;
        DecodeAdr(ArgStr[2], MModReg | MModXReg);
        if ((AdrType != ModNone) && (OpSize != 1)) WrError(1130);
        else 
         switch (AdrType)
         {
           case ModReg:
             CodeLen = 4;
             BAsmCode[0] = 0xd8 + AdrMode;
             BAsmCode[1] = Code;
             BAsmCode[2] = Lo(AdrWord);
             BAsmCode[3] = Hi(AdrWord);
             break;
           case ModXReg:
             CodeLen = 5;
             BAsmCode[0] = 0xd7;
             BAsmCode[1] = AdrMode;
             BAsmCode[2] = Code;
             BAsmCode[3] = Lo(AdrWord);
             BAsmCode[4] = Hi(AdrWord);
             break;
         }
      }
    }
  }
}

static void DecodeRLD_RRD(Word Code) 
{
  if (!ChkArgCnt(1, 2));
  else if ((ArgCnt == 2) && (strcasecmp(ArgStr[1], "A"))) WrError(1350);
  else
  {
    DecodeAdr(ArgStr[ArgCnt], MModMem);
    if (AdrType != ModNone)
    {
      CodeLen = 2 + AdrCnt;
      BAsmCode[0] = 0x80 + AdrMode;
      memcpy(BAsmCode + 1, AdrVals, AdrCnt);
      BAsmCode[1 + AdrCnt] = Code;
    }
  }
}

static void DecodeSCC(Word Code) 
{
  UNUSED(Code);

  if (ChkArgCnt(2, 2))
  {
    int Cond = DecodeCondition(ArgStr[1]);
    if (Cond >= ConditionCnt) WrXError(1360, ArgStr[1]);
    else
    {
      DecodeAdr(ArgStr[2], MModReg | MModXReg);
      if (OpSize > 1) WrError(1132);
      else 
      {
         switch (AdrType)
         {
           case ModReg:
             CodeLen = 2;
             BAsmCode[0] = 0xc8 + (OpSize << 4) + AdrMode;
             BAsmCode[1] = 0x70 + Conditions[Cond].Code;
             break;
           case ModXReg:
             CodeLen = 3;
             BAsmCode[0] = 0xc7 + (OpSize << 4);
             BAsmCode[1] = AdrMode;
             BAsmCode[2] = 0x70 + Conditions[Cond].Code;
             break;
         }
      }
    }
  }
  return;
}

/*---------------------------------------------------------------------------*/

static void AddSize(char *NName, Byte NCode, InstProc Proc, Word SizeMask)
{
  int l;
  char SizeName[20];

  AddInstTable(InstTable, NName, 0xff00 | NCode, Proc);
  l = sprintf(SizeName, "%sB", NName);
  if (SizeMask & 1)
    AddInstTable(InstTable, SizeName, 0x0000 | NCode, Proc);
  if (SizeMask & 2)
  {
    SizeName[l - 1] = 'W';
    AddInstTable(InstTable, SizeName, 0x0100 | NCode, Proc);
  }

  /* CP(L) would generate conflict with CPL instruction - dispatch
     it from CPL single-op instruction if ArgCnt >= 2! */

  if ((SizeMask & 4) && (strcmp(NName, "CP")))
  {
    SizeName[l - 1] = 'L';
    AddInstTable(InstTable, SizeName, 0x0200 | NCode, Proc);
  }
}

static void AddMod(char *NName, Byte NCode)
{
  int l;
  char SizeName[20];

  l = sprintf(SizeName, "%s1", NName);
  AddInstTable(InstTable, SizeName, NCode, DecodeMINC_MDEC);
  SizeName[l - 1] = '2';
  AddInstTable(InstTable, SizeName, NCode | 1, DecodeMINC_MDEC);
  SizeName[l - 1] = '4';
  AddInstTable(InstTable, SizeName, NCode | 2, DecodeMINC_MDEC);
}

static void AddFixed(char *NName, Word NCode, Byte NFlag, Boolean NSup)
{
  if (InstrZ >= FixedOrderCnt) exit(255);
  FixedOrders[InstrZ].Name = NName;
  FixedOrders[InstrZ].Code = NCode;
  FixedOrders[InstrZ].CPUFlag = NFlag;
  FixedOrders[InstrZ].InSup = NSup;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddReg(char *NName, Word NCode, Byte NMask)
{
  if (InstrZ >= RegOrderCnt) exit(255);
  RegOrders[InstrZ].Name = NName;
  RegOrders[InstrZ].Code = NCode;
  RegOrders[InstrZ].OpMask = NMask;
  AddInstTable(InstTable, NName, InstrZ++, DecodeReg);
}

static void AddImm(char *NName, Word NCode, Boolean NInSup,
                   Byte NMinMax, Byte NMaxMax, ShortInt NDefault)
{
  if (InstrZ >= ImmOrderCnt) exit(255);
  ImmOrders[InstrZ].Name = NName;
  ImmOrders[InstrZ].Code = NCode;
  ImmOrders[InstrZ].InSup = NInSup;
  ImmOrders[InstrZ].MinMax = NMinMax;
  ImmOrders[InstrZ].MaxMax = NMaxMax;
  ImmOrders[InstrZ].Default = NDefault;
  AddInstTable(InstTable, NName, InstrZ++, DecodeImm);
}

static void AddALU2(char *NName, Byte NCode)
{
  AddSize(NName, NCode, DecodeALU2, 7);
}

static void AddShift(char *NName)
{
  AddSize(NName, InstrZ++, DecodeShift, 7);
}

static void AddMulDiv(char *NName)
{
  AddInstTable(InstTable, NName, InstrZ++, DecodeMulDiv);
}

static void AddBitCF(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBitCF);
}

static void AddBit(char *NName)
{
  AddInstTable(InstTable, NName, InstrZ++, DecodeBit);
}

static void AddCondition(char *NName, Byte NCode)
{
  if (InstrZ >= ConditionCnt) exit(255);
  Conditions[InstrZ].Name = NName;
  Conditions[InstrZ++].Code = NCode;
}

static void InitFields(void)
{
  InstTable = CreateInstTable(301);
  SetDynamicInstTable(InstTable);

  AddInstTable(InstTable, "MULA"  , 0, DecodeMULA);
  AddInstTable(InstTable, "JP"    , 0, DecodeJPCALL);
  AddInstTable(InstTable, "CALL"  , 1, DecodeJPCALL);
  AddInstTable(InstTable, "JR"    , 0, DecodeJR);
  AddInstTable(InstTable, "JRL"   , 1, DecodeJR);
  AddInstTable(InstTable, "CALR"  , 0, DecodeCALR);
  AddInstTable(InstTable, "RET"   , 0, DecodeRET);
  AddInstTable(InstTable, "RETD"  , 0, DecodeRETD);
  AddInstTable(InstTable, "DJNZ"  , 0, DecodeDJNZ);
  AddInstTable(InstTable, "EX"    , 0, DecodeEX);
  AddInstTable(InstTable, "BS1F"  , 0, DecodeBS1x);
  AddInstTable(InstTable, "BS1B"  , 1, DecodeBS1x);
  AddInstTable(InstTable, "LDA"   , 0, DecodeLDA);
  AddInstTable(InstTable, "LDAR"  , 0, DecodeLDAR);
  AddInstTable(InstTable, "LDC"   , 0, DecodeLDC);
  AddInstTable(InstTable, "LDX"   , 0, DecodeLDX);
  AddInstTable(InstTable, "LINK"  , 0, DecodeLINK);
  AddSize("LD", 0, DecodeLD, 7);
  AddSize("PUSH", 0, DecodePUSH_POP, 7);
  AddSize("POP" , 1, DecodePUSH_POP, 7);
  AddSize("INC" , 0, DecodeINC_DEC, 7);
  AddSize("DEC" , 1, DecodeINC_DEC, 7);
  AddInstTable(InstTable, "CPI"  , 0x14, DecodeCPxx);
  AddInstTable(InstTable, "CPIR" , 0x15, DecodeCPxx);
  AddInstTable(InstTable, "CPD"  , 0x16, DecodeCPxx);
  AddInstTable(InstTable, "CPDR" , 0x17, DecodeCPxx);
  AddSize("LDI", 0x10 , DecodeLDxx, 3);
  AddSize("LDIR", 0x11, DecodeLDxx, 3);
  AddSize("LDD", 0x12 , DecodeLDxx, 3);
  AddSize("LDDR", 0x13, DecodeLDxx, 3); 
  AddMod("MINC", 0x38);
  AddMod("MDEC", 0x3c);
  AddInstTable(InstTable, "RLD", 0x06, DecodeRLD_RRD);
  AddInstTable(InstTable, "RRD", 0x07, DecodeRLD_RRD);
  AddInstTable(InstTable, "SCC", 0, DecodeSCC);

  FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt); InstrZ = 0;
  AddFixed("CCF"   , 0x0012, 3, False);
  AddFixed("DECF"  , 0x000d, 3, False);
  AddFixed("DI"    , 0x0607, 3, True );
  AddFixed("HALT"  , 0x0005, 3, True );
  AddFixed("INCF"  , 0x000c, 3, False);
  AddFixed("MAX"   , 0x0004, 1, True );
  AddFixed("MIN"   , 0x0004, 2, True );
  AddFixed("NOP"   , 0x0000, 3, False);
  AddFixed("NORMAL", 0x0001, 1, True );
  AddFixed("RCF"   , 0x0010, 3, False);
  AddFixed("RETI"  , 0x0007, 3, True );
  AddFixed("SCF"   , 0x0011, 3, False);
  AddFixed("ZCF"   , 0x0013, 3, False);

  RegOrders = (RegOrder *) malloc(sizeof(RegOrder) * RegOrderCnt); InstrZ = 0;
  AddReg("CPL" , 0xc006, 3);
  AddReg("DAA" , 0xc010, 1);
  AddReg("EXTS", 0xc013, 6);
  AddReg("EXTZ", 0xc012, 6);
  AddReg("MIRR", 0xc016, 2);
  AddReg("NEG" , 0xc007, 3);
  AddReg("PAA" , 0xc014, 6);
  AddReg("UNLK", 0xc00d, 4);

  ImmOrders = (ImmOrder *) malloc(sizeof(ImmOrder) * ImmOrderCnt); InstrZ = 0;
  AddImm("EI"  , 0x0600, True,  7, 7,  0);
  AddImm("LDF" , 0x1700, False, 7, 3, -1);
  AddImm("SWI" , 0x00f8, False, 7, 7,  7);

  AddALU2("ADC", 1);
  AddALU2("ADD", 0);
  AddALU2("AND", 4);
  AddALU2("OR" , 6);
  AddALU2("SBC", 3);
  AddALU2("SUB", 2);
  AddALU2("XOR", 5);
  AddALU2("CP" , 7);

  InstrZ = 0;
  AddShift("RLC");
  AddShift("RRC");
  AddShift("RL");
  AddShift("RR");
  AddShift("SLA");
  AddShift("SRA");
  AddShift("SLL");
  AddShift("SRL");

  InstrZ = 0;
  AddMulDiv("MUL");
  AddMulDiv("MULS");
  AddMulDiv("DIV");
  AddMulDiv("DIVS");

  AddBitCF("ANDCF" , 0);
  AddBitCF("LDCF"  , 3);
  AddBitCF("ORCF"  , 1);
  AddBitCF("STCF"  , 4);
  AddBitCF("XORCF" , 2);

  InstrZ = 0;
  AddBit("RES");
  AddBit("SET");
  AddBit("CHG");
  AddBit("BIT");
  AddBit("TSET");

  Conditions = (Condition *) malloc(sizeof(Condition) * ConditionCnt); InstrZ = 0;
  AddCondition("F"   ,  0); 
  DefaultCondition = InstrZ;  AddCondition("T"   ,  8);
  AddCondition("Z"   ,  6); AddCondition("NZ"  , 14);
  AddCondition("C"   ,  7); AddCondition("NC"  , 15);
  AddCondition("PL"  , 13); AddCondition("MI"  ,  5);
  AddCondition("P"   , 13); AddCondition("M"   ,  5);
  AddCondition("NE"  , 14); AddCondition("EQ"  ,  6);
  AddCondition("OV"  ,  4); AddCondition("NOV" , 12);
  AddCondition("PE"  ,  4); AddCondition("PO"  , 12);
  AddCondition("GE"  ,  9); AddCondition("LT"  ,  1);
  AddCondition("GT"  , 10); AddCondition("LE"  ,  2);
  AddCondition("UGE" , 15); AddCondition("ULT" ,  7);
  AddCondition("UGT" , 11); AddCondition("ULE" ,  3);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(RegOrders);
  free(ImmOrders);
  free(Conditions);
}

static void MakeCode_96C141(void)
{
  CodeLen = 0;
  DontPrint = False;
  OpSize = -1;
  MinOneIs0 = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False)) return;

  /* vermischt */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200,OpPart);
}

static Boolean ChkPC_96C141(LargeWord Addr)
{
  Boolean ok;

  switch (ActPC)
  {
    case SegCode:
      if (Maximum) ok = (Addr <= 0xffffff);
              else ok = (Addr <= 0xffff);
      break;
    default: 
      ok = False;
  }
  return (ok);
}


static Boolean IsDef_96C141(void)
{
  return False;
}

static void SwitchFrom_96C141(void)
{
  DeinitFields();
  ClearONOFF();
}

static void SwitchTo_96C141(void)
{
  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = True;

  PCSymbol = "$";
  HeaderID = 0x52;
  NOPCode = 0x00;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode);
  Grans[SegCode] = 1;
  ListGrans[SegCode] = 1;
  SegInits[SegCode] = 0;

  MakeCode = MakeCode_96C141;
  ChkPC = ChkPC_96C141;
  IsDef = IsDef_96C141;
  SwitchFrom = SwitchFrom_96C141;
  AddONOFF("MAXMODE", &Maximum   , MaximumName   , False);
  AddONOFF("SUPMODE", &SupAllowed, SupAllowedName, False);

  InitFields();
}

void code96c141_init(void)
{
  CPU96C141 = AddCPU("96C141", SwitchTo_96C141);
  CPU93C141 = AddCPU("93C141", SwitchTo_96C141);
}
