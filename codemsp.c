/* codemsp.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MSP430                                                      */
/*                                                                           */
/* Historie:                                                                 */
/*             18. 8.1998 BookKeeping-Aufruf bei BSS                         */
/*              2. 1.1998 ChkPC umgestellt                                   */
/*              9. 3.2000 'ambiguous else'-Warnungen beseitigt               */
/*             2001-11-16 Endianness must be LSB first                       */
/*             2002-01-27 allow immediate addressing for one-op instrs(doj)  */
/*                                                                           */
/*****************************************************************************/
/* $Id: codemsp.c,v 1.26 2017/06/28 17:02:40 alfred Exp $                     */
/***************************************************************************** 
 * $Log: codemsp.c,v $
 * Revision 1.26  2017/06/28 17:02:40  alfred
 * - correct MSP430 source operand conversion 0(Rn) -> @Rn
 *
 * Revision 1.25  2017/06/07 19:12:43  alfred
 * - remove double JNZ instruction
 *
 * Revision 1.24  2017/05/31 19:13:14  alfred
 * - MSP430(X): forgot to set PCDist for some instructions
 * - MSP430(X): also set upper 4 bits of displacement for emulated instruction
 * - correct commented code in t_msp430x
 *
 * Revision 1.23  2017/05/15 19:18:36  alfred
 * - correct encoding of TSTA
 *
 * Revision 1.22  2017/05/01 12:39:41  alfred
 * - complete 430X target
 *
 * Revision 1.21  2017/05/01 09:50:32  alfred
 * - correct CALLA range checking
 *
 * Revision 1.20  2017/04/21 20:00:46  alfred
 * - handle rtpc/rptz prefixes
 * - allow > operator to force long immediate
 *
 * Revision 1.19  2017/04/15 13:18:22  alfred
 * - implement emulated MSP instructions no longer via macros
 *
 * Revision 1.18  2017/04/14 10:01:57  alfred
 * - better handling of constant -1 depending on operand size
 *
 * Revision 1.17  2017/04/14 09:56:45  alfred
 * - add the new MSP430X instructions without prefix
 *
 * Revision 1.16  2017/04/03 19:40:47  alfred
 * - extend addresses for MSP430X address decoding
 *
 * Revision 1.15  2017/04/03 18:51:49  alfred
 * - extend/fix MSP430(X) operand sizes
 *
 * Revision 1.14  2017/04/03 18:10:47  alfred
 * - first reworks and cleanups for MSP430X
 *
 * Revision 1.13  2014/12/07 19:14:01  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.12  2014/12/05 11:15:28  alfred
 * - eliminate AND/OR/NOT
 *
 * Revision 1.11  2013/12/21 19:46:51  alfred
 * - dynamically resize code buffer
 *
 * Revision 1.10  2012-05-26 13:49:19  alfred
 * - MSP additions, make implicit macro parameters always case-insensitive
 *
 * Revision 1.9  2010/12/05 23:18:57  alfred
 * - improve erro arguments
 *
 * Revision 1.8  2008/11/23 10:39:17  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.7  2007/12/31 12:56:27  alfred
 * - rework to hash table
 *
 * Revision 1.6  2007/11/24 22:48:07  alfred
 * - some NetBSD changes
 *
 * Revision 1.5  2005/10/30 13:23:28  alfred
 * - warn about odd program counters
 *
 * Revision 1.4  2005/10/02 10:00:46  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2005/09/30 08:31:48  alfred
 * - correct byte disposition for big-endian machines
 *
 * Revision 1.2  2005/09/08 17:31:05  alfred
 * - add missing include
 *
 * Revision 1.1  2003/11/06 02:49:23  alfred
 * - recreated
 *
 * Revision 1.5  2003/05/02 21:23:12  alfred
 * - strlen() updates
 *
 * Revision 1.4  2002/08/14 18:43:49  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.3  2002/07/27 17:44:50  alfred
 * - one more TempAll fix
 *
 * Revision 1.2  2002/07/14 18:39:59  alfred
 * - fixed TempAll-related warnings
 *
 *****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "endian.h"
#include "strutil.h"
#include "bpemu.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"  
#include "codepseudo.h"
#include "codevars.h"

#define OneOpCount 6

typedef struct
{
  Boolean MayByte;
  Word Code;
} OneOpOrder;

typedef enum
{
  eModeReg = 0,
  eModeRegDisp = 1,
  eModeIReg = 2,
  eModeIRegAutoInc = 3,
  eModeNone = 0xff,
} tMode;

#define MModeReg (1 << eModeReg)
#define MModeRegDisp (1 << eModeRegDisp)
#define MModeIReg (1 << eModeIReg)
#define MModeIRegAutoInc (1 << eModeIRegAutoInc)
#define MModeAs 15
#define MModeAd 3

typedef enum
{
  eExtModeNo = 0,
  eExtModeYes = 1,
} tExtMode;

typedef enum
{
  eOpSizeB = 0,
  eOpSizeW = 1,
  eOpSizeA = 2,
  eOpSizeCnt,
  eOpSizeDefault = eOpSizeW,
} tOpSize;

#define RegPC 0
#define RegSP 1
#define RegSR 2
#define RegCG1 2
#define RegCG2 3

typedef struct
{
  Word Mode, Part, Cnt;
  LongWord Val;
  Boolean WasImm, WasAbs;
} tAdrParts;

/*  float exp (8bit bias 128) sign mant (impl. norm.)
   double exp (8bit bias 128) sign mant (impl. norm.) */

static CPUVar CPUMSP430, CPUMSP430X;

static OneOpOrder *OneOpOrders;

static tOpSize OpSize;
static Word PCDist, MultPrefix;
static IntType AdrIntType, DispIntType;
static const IntType OpSizeIntTypes[eOpSizeCnt] = { Int8, Int16, Int20 };

/*-------------------------------------------------------------------------*/

static void ResetAdr(tAdrParts *pAdrParts)
{
  pAdrParts->Mode = eModeNone;
  pAdrParts->Part = 0;
  pAdrParts->Cnt = 0;
  pAdrParts->WasImm =
  pAdrParts->WasAbs = False;
}

static Boolean ChkAdr(Byte Mask, tAdrParts *pAdrParts)
{
  if ((pAdrParts->Mode != 0xff) && ((Mask & (1 << pAdrParts->Mode)) == 0))
  {
    ResetAdr(pAdrParts);
    WrError(1350);
    return False;
  }
  return True;
}

static Boolean DecodeReg(const char *Asc, Word *pErg)
{
  char *s;

  if (FindRegDef(Asc, &s)) Asc = s;

  if (!strcasecmp(Asc, "PC"))
  {
    *pErg = 0; return True;
  }
  else if (!strcasecmp(Asc,"SP"))
  {
    *pErg = 1; return True;
  }
  else if (!strcasecmp(Asc, "SR"))
  {
    *pErg = 2; return True;
  }
  if ((mytoupper(*Asc) == 'R') && (strlen(Asc) >= 2) && (strlen(Asc) <= 3))
  {
    Boolean OK;

    *pErg = ConstLongInt(Asc + 1, &OK, 10);
    return ((OK) && (*pErg < 16));
  }

  return False;
}

static void FillAdrPartsImm(tAdrParts *pAdrParts, LongWord Value, Boolean ForceLong)
{
  ResetAdr(pAdrParts); 
  pAdrParts->WasImm = True;
  pAdrParts->Val = Value;

  /* assume no usage of constant generators */

  pAdrParts->Part = RegPC;

  /* constant generators allowed at all? */

  if (!ForceLong)
  {
    /* special treatment for -1 since it depends on the operand size: */

    if ((Value == 0xffffffff)
     || ((OpSize == 0) && (Value == 0xff))
     || ((OpSize == 1) && (Value == 0xffff))
     || ((OpSize == 2) && (Value == 0xfffff)))
    {
      pAdrParts->Cnt = 0;
      pAdrParts->Part = RegCG2;
      pAdrParts->Mode = eModeIRegAutoInc;
    }
    else switch (Value)
    {
      case 0:
        pAdrParts->Part = RegCG2;
        pAdrParts->Mode = eModeReg;
        break;
      case 1:
        pAdrParts->Part = RegCG2;
        pAdrParts->Mode = eModeRegDisp;
        break;
      case 2:
        pAdrParts->Part = RegCG2;
        pAdrParts->Mode = eModeIReg;
        break;
      case 4:
        pAdrParts->Part = RegCG1;
        pAdrParts->Mode = eModeIReg;
        break;
      case 8:
        pAdrParts->Part = RegCG1;
        pAdrParts->Mode = eModeIRegAutoInc;
        break;
      default:
        break;
    }
  }

  /* constant generators not used, in one or the other way -> use
     @PC++ to dispose constant */

  if (pAdrParts->Part == RegPC)
  {
    pAdrParts->Cnt = 1;
    pAdrParts->Mode = eModeIRegAutoInc;
  }
}

static Boolean DecodeAdr(char *Asc, tExtMode ExtMode, Byte Mask, Boolean MayImm, tAdrParts *pAdrParts)
{
  LongWord AdrWord, CurrPC;
  Word Reg;
  Boolean OK;
  char *p;
  IntType ThisAdrIntType = (ExtMode == eExtModeYes) ? AdrIntType : UInt16;
  IntType ThisDispIntType = (ExtMode == eExtModeYes) ? DispIntType : Int16;

  ResetAdr(pAdrParts);

  /* immediate */

  if (*Asc == '#')
  {
    if (!MayImm) WrError(1350);
    else
    {
      int ForceLong = (Asc[1] == '>') ? 1 : 0;

      AdrWord = EvalIntExpression(Asc + 1 + ForceLong, OpSizeIntTypes[OpSize], &OK);
      if (OK)
      {
        FillAdrPartsImm(pAdrParts, AdrWord, ForceLong);
      }
    }
    return ChkAdr(Mask, pAdrParts);
  }

  /* absolut */

  if (*Asc == '&')
  {
    pAdrParts->Val = EvalIntExpression(Asc + 1, ThisAdrIntType, &OK);
    if (OK)
    {
      pAdrParts->WasAbs = True;
      pAdrParts->Mode = eModeRegDisp;
      pAdrParts->Part = RegCG1; /* == 0 with As/Ad=1 */
      pAdrParts->Cnt = 1;
    }
    return ChkAdr(Mask, pAdrParts);
  }

  /* Register */

  if (DecodeReg(Asc, &Reg))
  {
    if (Reg == RegCG2) WrXError(1445, Asc);
    else
    {
      pAdrParts->Mode = eModeReg;
      pAdrParts->Part = Reg;
    }
    return ChkAdr(Mask, pAdrParts);
  }

  /* Displacement */

  if ((*Asc) && (Asc[strlen(Asc)-1] == ')'))
  {
    Asc[strlen(Asc) - 1] = '\0';
    p = RQuotPos(Asc, '(');
    if (p)
    {
      if (DecodeReg(p + 1, &Reg))
      {
        *p = '\0';
        pAdrParts->Val = EvalIntExpression(Asc, ThisDispIntType, &OK);
        if (OK)
        {
          if ((Reg == 2) || (Reg == 3)) WrXError(1445, p + 1);
          else if ((pAdrParts->Val == 0) && ((Mask & 4) != 0))
          {
            pAdrParts->Part = Reg;
            pAdrParts->Mode = eModeIReg;
          }
          else
          {
            pAdrParts->Part = Reg;
            pAdrParts->Cnt = 1;
            pAdrParts->Mode = eModeRegDisp;
          }
        }
        *p = '(';
        return ChkAdr(Mask, pAdrParts);
      }
    }
    Asc[strlen(Asc)] = ')';
  }

  /* indirekt mit/ohne Autoinkrement */

  if ((*Asc == '@') || (*Asc=='*'))
  {
    Boolean AutoInc = False;

    if (Asc[strlen(Asc) - 1] == '+')
    {
      AutoInc = True;
      Asc[strlen(Asc) - 1] = '\0';
    }
    if (!DecodeReg(Asc + 1, &Reg)) WrXError(1445, Asc);
    else if ((Reg == 2) || (Reg == 3)) WrXError(1445, Asc);
    else if (!AutoInc && ((Mask & MModeIReg) == 0))
    {
      pAdrParts->Part = Reg;
      pAdrParts->Val = 0;
      pAdrParts->Cnt = 1;
      pAdrParts->Mode = eModeRegDisp;
    }
    else
    {
      pAdrParts->Part = Reg;
      pAdrParts->Mode = AutoInc ? eModeIRegAutoInc : eModeIReg;
    }
    return ChkAdr(Mask, pAdrParts);
  }

  /* bleibt PC-relativ aka 'symbolic mode': */

  if (!PCDist)
  {
    fprintf(stderr, "internal error: PCDist not set for '%s'\n", OpPart);
    exit(10);
  }
  CurrPC = EProgCounter() + PCDist;

  /* extended instruction (on 430X): use the full 20 bit displacement: */

  if (ExtMode == eExtModeYes)
  {
    AdrWord = (EvalIntExpression(Asc, UInt20, &OK) - CurrPC) & 0xfffff;
  }

  /* non-extended instruction on 430X: if the current PC is within the
     first 64K, bits 16..19 will be cleared after addition, i.e. the
     target address must also be within the first 64K: */

  else if (MomCPU >= CPUMSP430X)
  {
    if (CurrPC <= 0xffff)
    {
      AdrWord = (EvalIntExpression(Asc, UInt16, &OK) - CurrPC) & 0xffff;
    }
    else
    {
      AdrWord = (EvalIntExpression(Asc, UInt20, &OK) - CurrPC) & 0xfffff;
      if ((AdrWord > 0x7fff) && (AdrWord < 0xf8000))
      {
        WrError(1320);
        OK = False;
      }
    }
  }

  /* non-extended instruction on 430: all within 64K with wraparound */

  else
  {
    AdrWord = (EvalIntExpression(Asc, UInt16, &OK) - CurrPC) & 0xffff;
  }

  if (OK)
  {
    pAdrParts->Part = RegPC;
    pAdrParts->Mode = eModeRegDisp;
    pAdrParts->Cnt = 1;
    pAdrParts->Val = AdrWord;
  }

  return ChkAdr(Mask, pAdrParts);
}

static Word GetBW(void)
{
  return (OpSize == eOpSizeB) || (OpSize == eOpSizeA) ? 0x0040 : 0x0000;
}

static Word GetAL(void)
{
  return (OpSize == eOpSizeW) || (OpSize == eOpSizeB) ? 0x0040 : 0x0000;
}

static Word GetMult(const char *pArg, Boolean *pOK)
{
  Word Result = 0x0000;

  if (DecodeReg(pArg, &Result))
  {
    *pOK = True;
    return Result | 0x0080;
  }
  if (*pArg == '#')
  {
    FirstPassUnknown = False;
    Result = EvalIntExpression(pArg + 1, UInt5, pOK);
    if (*pOK)
    {
      if (FirstPassUnknown)
        Result = 1;
      if (!ChkRange(Result, 1, 16))
        *pOK = False;
      else
        Result--;
    }
  }
  else
    *pOK = False;
  return Result;
}

/*-------------------------------------------------------------------------*/

static void PutByte(Word Value)
{
  if (CodeLen & 1)
    WAsmCode[CodeLen >> 1] = (Value << 8) | BAsmCode[CodeLen - 1];
  else
    BAsmCode[CodeLen] = Value;
  CodeLen++;
}

static void ConstructTwoOp(Word Code, const tAdrParts *pSrcParts, const tAdrParts *pDestParts)
{
  WAsmCode[CodeLen >> 1] = Code | (pSrcParts->Part << 8) | (pDestParts->Mode << 7)
                         | GetBW() | (pSrcParts->Mode << 4) | pDestParts->Part;
  CodeLen += 2;
  memcpy(WAsmCode + (CodeLen >> 1), &pSrcParts->Val, pSrcParts->Cnt << 1); CodeLen += pSrcParts->Cnt << 1;
  memcpy(WAsmCode + (CodeLen >> 1), &pDestParts->Val, pDestParts->Cnt << 1); CodeLen += pDestParts->Cnt << 1;
}

static void ConstructTwoOpX(Word Code, const tAdrParts *pSrcParts, const tAdrParts *pDestParts)
{
  Word Prefix = 0x1800 | GetAL();

  if ((eModeReg != pSrcParts->Mode) || (eModeReg != pDestParts->Mode))
  {
    if (pSrcParts->Cnt)
      Prefix |= ((pSrcParts->Val >> 16) & 15) << 7;
    if (pDestParts->Cnt)
      Prefix |= ((pDestParts->Val >> 16) & 15);
  }

  /* take over multiply prefix for register<->register ops only */

  else
  {
    Prefix |= MultPrefix;
    MultPrefix = 0;
  }
  WAsmCode[CodeLen >> 1] = Prefix; CodeLen += 2;
  ConstructTwoOp(Code, pSrcParts, pDestParts);
}

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (OpSize != eOpSizeDefault) WrError(1130);
  else
  {
    if (Odd(EProgCounter())) WrError(180);
    WAsmCode[0] = Code; CodeLen = 2;
  }
}

static void DecodeTwoOp(Word Code)
{
  tAdrParts SrcParts, DestParts;

  if (ArgCnt != 2) WrError(1110);
  else if (OpSize > eOpSizeW) WrError(1130);
  else
  {
    PCDist = 2;
    if (DecodeAdr(ArgStr[1], eExtModeNo, 15, True, &SrcParts))
    {
      PCDist += SrcParts.Cnt << 1;
      if (DecodeAdr(ArgStr[2], eExtModeNo, 3, False, &DestParts))
      {
        if (Odd(EProgCounter())) WrError(180);
        ConstructTwoOp(Code, &SrcParts, &DestParts);
      }
    }
  }
}

static void DecodeTwoOpX(Word Code)
{
  tAdrParts SrcParts, DestParts;

  Code &= ~1;

  if (ArgCnt != 2)
  {
    WrError(1110);
    return;
  }

  PCDist = 4;
  if (DecodeAdr(ArgStr[1], eExtModeYes, MModeAs, True, &SrcParts))
  {
    PCDist += SrcParts.Cnt << 1;
    if (DecodeAdr(ArgStr[2], eExtModeYes, MModeAd, False, &DestParts))
    {
      if (Odd(EProgCounter())) WrError(180);
      ConstructTwoOpX(Code, &SrcParts, &DestParts);
    }
  }
}

static void DecodeEmulOneToTwo(Word Code)
{
  Byte SrcSpec;
  tAdrParts SrcParts, DestParts;

  /* separate src spec & opcode */

  SrcSpec = Lo(Code);
  Code &= 0xff00;

  if (ArgCnt != 1)
  {
    WrError(1110);
    return;
  }

  if (OpSize > eOpSizeW)
  {
    WrError(1130);
    return;
  }

  /* Decode operand:
      - Ad modes always allowed
      - for Src == Dest, also allow @Rn+: */

  PCDist = 2;
  if (!DecodeAdr(ArgStr[1], eExtModeNo, MModeAd | ((SrcSpec == 0xaa) ? MModeIRegAutoInc : 0), False, &DestParts))
    return;

  /* filter immediate out separately (we get it as d(PC): */

  if (DestParts.WasImm)
  {
    WrError(1350);
    return;
  }

  /* deduce src operand: 0xaa = special value for Src == Dest: */

  if (SrcSpec == 0xaa)
  {
    /* default assumption: */

    SrcParts = DestParts;

    /* @Rn+: is transformed to @Rn+,-opsize(Rn): */

    if (SrcParts.Mode == eModeIRegAutoInc)
    {
      static const Byte MemLen[3] = { 1, 2, 4 };

      DestParts.Mode = eModeRegDisp;
      DestParts.Val = (0 - MemLen[OpSize]) & 0xffff;
      DestParts.Cnt = 1;
    }

    /* for PC-relative addressing, fix up destination displacement and
       complain on displacement overflow: */

    else if ((DestParts.Mode == eModeRegDisp) && (DestParts.Part == RegPC))
    {
      LongWord NewDist = DestParts.Val - 2;

      if ((NewDist & 0x8000) != (DestParts.Val & 0x8000))
      {
        WrError(1330);
        return;
      }
      DestParts.Val = NewDist;
    }

    /* transform 0(Rn) as Dest back to @Rn as Src: */

    else if ((SrcParts.Mode == eModeRegDisp) && (DestParts.Val == 0))
    {
      SrcParts.Mode = eModeIReg;
      SrcParts.Cnt = 0;
    }
  }

  /* Src == other (constant) value: 0xff means -1: */

  else
    FillAdrPartsImm(&SrcParts, SrcSpec == 0xff ? 0xffffffff : SrcSpec, False);

  /* assemble like 2-op instruction: */

  ConstructTwoOp(Code, &SrcParts, &DestParts);
}

static void DecodeBR(Word Code)
{
  tAdrParts DstParts, SrcParts;

  PCDist = 2;
  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (DecodeAdr(ArgStr[1], eExtModeNo, MModeAs, True, &SrcParts))
  {
    if (Odd(EProgCounter())) WrError(180);
    ResetAdr(&DstParts);
    DstParts.Mode = eModeReg;
    DstParts.Part = RegPC;
    ConstructTwoOp(Code, &SrcParts, &DstParts);
  }
}

static void DecodeEmulOneToTwoX(Word Code)
{
  Byte SrcSpec;
  tAdrParts SrcParts, DestParts;

  /* separate src spec & opcode */

  SrcSpec = Lo(Code);
  Code &= 0xff00;

  if (ArgCnt != 1)
  {
    WrError(1110);
    return;
  }

  /* Decode operand:
      - Ad modes always allowed
      - for Src == Dest, also allow @Rn+: */

  PCDist = 4;
  if (!DecodeAdr(ArgStr[1], eExtModeYes, MModeAd | ((SrcSpec == 0xaa) ? MModeIRegAutoInc : 0), False, &DestParts))
    return;

  /* filter immediate out separately (we get it as d(PC): */

  if (DestParts.WasImm)
  {
    WrError(1350);
    return;
  }

  /* deduce src operand: 0xaa = special value for Src == Dest: */

  if (SrcSpec == 0xaa)
  {
    /* default assumption: */

    SrcParts = DestParts;

    /* @Rn+: is transformed to @Rn+,-opsize(Rn): */

    if (SrcParts.Mode == eModeIRegAutoInc)
    {
      static const Byte MemLen[3] = { 1, 2, 4 };

      DestParts.Mode = eModeRegDisp;
      DestParts.Val = (0 - MemLen[OpSize]) & 0xfffff;
      DestParts.Cnt = 1;
    }

    /* for PC-relative addressing, fix up destination displacement and
       complain on displacement overflow: */

    else if ((DestParts.Mode == eModeRegDisp) && (DestParts.Part == RegPC))
    {
      LongWord NewDist = DestParts.Val - 2;

      if ((NewDist & 0x8000) != (DestParts.Val & 0x8000))
      {
        WrError(1330);
        return;
      }
      DestParts.Val = NewDist;
    }

    /* transform 0(Rn) as Dest back to @Rn as Src: */

    else if ((SrcParts.Mode == eModeRegDisp) && (DestParts.Val == 0))
    {
      SrcParts.Mode = eModeIReg;
      SrcParts.Cnt = 0;
    }
  }

  /* Src == other (constant) value: 0xff means -1: */

  else
    FillAdrPartsImm(&SrcParts, SrcSpec == 0xff ? 0xffffffff : SrcSpec, False);

  /* assemble like 2-op instruction: */

  ConstructTwoOpX(Code, &SrcParts, &DestParts);
}

static void DecodePOP(Word Code)
{
  tAdrParts DstParts, SrcParts;

  PCDist = 2;
  if (ArgCnt != 1) WrError(1110);
  else if (DecodeAdr(ArgStr[1], eExtModeNo, MModeAd, True, &DstParts))
  {
    if (Odd(EProgCounter())) WrError(180);
    ResetAdr(&SrcParts);
    SrcParts.Mode = eModeIRegAutoInc;
    SrcParts.Part = RegSP;
    ConstructTwoOp(Code, &SrcParts, &DstParts);
  }
}

static void DecodePOPX(Word Code)
{
  tAdrParts DstParts, SrcParts;

  PCDist = 4;
  if (ArgCnt != 1) WrError(1110);
  else if (DecodeAdr(ArgStr[1], eExtModeYes, MModeAd, True, &DstParts))
  {
    if (Odd(EProgCounter())) WrError(180);
    ResetAdr(&SrcParts);
    SrcParts.Mode = eModeIRegAutoInc;
    SrcParts.Part = RegSP;
    ConstructTwoOpX(Code, &SrcParts, &DstParts);
  }
}

static void DecodeOneOp(Word Index)
{
  const OneOpOrder *pOrder = OneOpOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else if (OpSize > eOpSizeW) WrError(1130);
  else if ((OpSize == eOpSizeB) && (!pOrder->MayByte)) WrError(1130);
  else
  {
    tAdrParts AdrParts;

    PCDist = 2;
    if (DecodeAdr(ArgStr[1], eExtModeNo, 15, True, &AdrParts))
    {
      if (Odd(EProgCounter())) WrError(180);
      WAsmCode[0] = pOrder->Code | GetBW() | (AdrParts.Mode << 4) | AdrParts.Part;
      memcpy(WAsmCode + 1, &AdrParts.Val, AdrParts.Cnt << 1);
      CodeLen = (1 + AdrParts.Cnt) << 1;
    }
  }
}

static void DecodeOneOpX(Word Index)
{
  const OneOpOrder *pOrder = OneOpOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else if ((OpSize == eOpSizeB) && (!pOrder->MayByte)) WrError(1130);
  else
  {
    tAdrParts AdrParts;

    PCDist = 4;
    if (DecodeAdr(ArgStr[1], eExtModeYes, 15, True, &AdrParts))
    {
      /* B/W for 20 bit size is 0 instead of 1 for SXT/SWPB */

      Word ActBW = pOrder->MayByte ? GetBW() : 0;

      if (Odd(EProgCounter())) WrError(180);
      WAsmCode[0] = 0x1800 | GetAL();

      /* put bits 16:19 of operand into bits 0:3 or 7:10 of extension word? */

      if (AdrParts.Cnt)
        WAsmCode[0] |= (((AdrParts.Val >> 16) & 15) << 7);

      /* repeat only supported for register op */

      if (AdrParts.Mode == eModeReg)
      {
        WAsmCode[0] |= MultPrefix;
        MultPrefix = 0;
      }
      WAsmCode[1] = pOrder->Code | ActBW | (AdrParts.Mode << 4) | AdrParts.Part;
      memcpy(WAsmCode + 2, &AdrParts.Val, AdrParts.Cnt << 1);
      CodeLen = (2 + AdrParts.Cnt) << 1;
    }
  }
}

static void DecodeMOVA(Word Code)
{
  tAdrParts AdrParts;

  UNUSED(Code);

  OpSize = 2;
  if (ArgCnt != 2) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    PCDist = 2;
    DecodeAdr(ArgStr[2], eExtModeYes, 15, False, &AdrParts);
    if (AdrParts.WasAbs)
    {
      if (!DecodeReg(ArgStr[1], &WAsmCode[0])) WrXError(1445, ArgStr[1]);
      else
      {
        if (Odd(EProgCounter())) WrError(180);
        WAsmCode[0] = 0x0060 | (WAsmCode[0] << 8) | ((AdrParts.Val >> 16) & 0x0f);
        WAsmCode[1] = AdrParts.Val & 0xffff;
        CodeLen = 4;
      }
    }
    else switch (AdrParts.Mode)
    {
      case eModeReg:
        WAsmCode[0] = AdrParts.Part;
        DecodeAdr(ArgStr[1], eExtModeYes, 15, True, &AdrParts);
        if (AdrParts.WasImm)
        {
          if (Odd(EProgCounter())) WrError(180);
          WAsmCode[0] |= ((AdrParts.Val >> 8) & 0x0f00) | 0x0080;
          WAsmCode[1] = AdrParts.Val & 0xffff;
          CodeLen = 4;
        }
        else if (AdrParts.WasAbs)
        {
          if (Odd(EProgCounter())) WrError(180);
          WAsmCode[0] |= ((AdrParts.Val >> 8) & 0x0f00) | 0x0020;
          WAsmCode[1] = AdrParts.Val & 0xffff;
          CodeLen = 4;
        }
        else switch (AdrParts.Mode)
        {
          case eModeReg:
           if (Odd(EProgCounter())) WrError(180);
            WAsmCode[0] |= (AdrParts.Part << 8) | 0x00c0;
            CodeLen = 2;
            break;
          case eModeIReg:
            if (Odd(EProgCounter())) WrError(180);
            WAsmCode[0] |= (AdrParts.Part << 8) | 0x0000;
            CodeLen = 2;
            break;
          case eModeIRegAutoInc:
            if (Odd(EProgCounter())) WrError(180);
            WAsmCode[0] |= (AdrParts.Part << 8) | 0x0010;
            CodeLen = 2;
            break;
          case eModeRegDisp:
            if (ChkRange(AdrParts.Val, 0, 0xffff))
            {
              if (Odd(EProgCounter())) WrError(180);
              WAsmCode[0] |= (AdrParts.Part << 8) | 0x0030;
              WAsmCode[1] = AdrParts.Val & 0xffff;
              CodeLen = 4;  
            }
            break;
        }
        break;
      case eModeRegDisp:
        if (!ChkRange(AdrParts.Val, 0, 0xffff));
        else if (!DecodeReg(ArgStr[1], &WAsmCode[0])) WrXError(1445, ArgStr[1]);
        else
        {
          if (Odd(EProgCounter())) WrError(180);
          WAsmCode[0] = 0x0070 | (WAsmCode[0] << 8) | AdrParts.Part;
          WAsmCode[1] = AdrParts.Val & 0xffff;
          CodeLen = 4;
        }
        break;
    }
  }
}

static void DecodeBRA(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  strcpy(ArgStr[2], "PC");
  ArgCnt++;
  DecodeMOVA(Code);
}

static void DecodeCLRA(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (DecodeReg(ArgStr[1], &WAsmCode[0]))
  {
    WAsmCode[0] |= Code;
    CodeLen = 2;
  }
}

static void DecodeTSTA(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (DecodeReg(ArgStr[1], &WAsmCode[0]))
  {
    WAsmCode[0] |= Code;
    WAsmCode[1] = 0x0000;
    CodeLen = 4;
  }
}

static void DecodeDECDA_INCDA(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (DecodeReg(ArgStr[1], &WAsmCode[0]))
  {
    WAsmCode[0] |= Code;
    WAsmCode[1] = 2;
    CodeLen = 4;
  }
}

static void DecodeADDA_SUBA_CMPA(Word Code)
{
  OpSize = 2;

  if (ArgCnt != 2) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (!DecodeReg(ArgStr[2], &WAsmCode[0])) WrXError(1445, ArgStr[2]);
  else
  {
    tAdrParts AdrParts;

    DecodeAdr(ArgStr[1], eExtModeYes, 15, True, &AdrParts);
    if (AdrParts.WasImm)
    {
      if (Odd(EProgCounter())) WrError(180);
      WAsmCode[0] |= Code | ((AdrParts.Val >> 8) & 0xf00);
      WAsmCode[1] = AdrParts.Val & 0xffff;
      CodeLen = 4;
    }
    else if (eModeReg == AdrParts.Mode)
    {
      if (Odd(EProgCounter())) WrError(180);
      WAsmCode[0] |= Code | 0x0040 | (AdrParts.Part << 8);
      CodeLen = 2;
    }
    else
      WrError(1130);
  }
}

static void DecodeRxM(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else if (OpSize == eOpSizeB) WrError(1130);
  else if (!DecodeReg(ArgStr[2], &WAsmCode[0])) WrXError(1445, ArgStr[2]);
  else if (ArgStr[1][0] != '#') WrError(1120);
  else
  {
    Word Mult;
    Boolean OK;

    FirstPassUnknown = False;
    Mult = EvalIntExpression(ArgStr[1] + 1, UInt3, &OK);
    if (OK)
    {
      if (FirstPassUnknown)
        Mult = 1;
      if (ChkRange(Mult, 1, 4))
      {
        if (Odd(EProgCounter())) WrError(180);
        WAsmCode[0] |= Code | ((Mult - 1) << 10) | (GetAL() >> 2);
        CodeLen = 2;
      }
    }
  }
}

static void DecodeCALLA(Word Code)
{
  tAdrParts AdrParts;

  UNUSED(Code);

  OpSize = 2;
  PCDist = 2;
  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (DecodeAdr(ArgStr[1], eExtModeYes, 15, True, &AdrParts))
  {
    if (AdrParts.WasImm)
    {
      WAsmCode[0] = 0x13b0 | ((AdrParts.Val >> 16) & 15);
      WAsmCode[1] = AdrParts.Val & 0xffff;
      CodeLen = 4;
    }
    else if (AdrParts.WasAbs)
    {
      WAsmCode[0] = 0x1380 | ((AdrParts.Val >> 16) & 15);
      WAsmCode[1] = AdrParts.Val & 0xffff;
      CodeLen = 4;
    }
    else if ((AdrParts.Mode == eModeRegDisp) && (AdrParts.Part == RegPC))
    {
      WAsmCode[0] = 0x1390 | ((AdrParts.Val >> 16) & 15);
      WAsmCode[1] = AdrParts.Val & 0xffff;
      CodeLen = 4;
    }
    else if ((AdrParts.Mode == eModeRegDisp) && (((AdrParts.Val & 0xfffff) > 0x7fff) && ((AdrParts.Val & 0xfffff) < 0xf8000))) WrError(1320);
    else
    {
      WAsmCode[0] = 0x1340 | (AdrParts.Mode << 4) | (AdrParts.Part);
      memcpy(WAsmCode + 1, &AdrParts.Val, AdrParts.Cnt << 1);
      CodeLen = (1 + AdrParts.Cnt) << 1;
    }
  }
}

static void DecodePUSHM_POPM(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else if (OpSize == 0) WrError(1130);
  else if (!DecodeReg(ArgStr[2], &WAsmCode[0])) WrXError(1445, ArgStr[2]);
  else if (ArgStr[1][0] != '#') WrError(1120);
  else
  {
    Boolean OK;
    Word Cnt;

    FirstPassUnknown = False;
    Cnt = EvalIntExpression(ArgStr[1] + 1, UInt5, &OK);
    if (FirstPassUnknown)
      Cnt = 1;
    if (OK && ChkRange(Cnt, 1, 16))
    {
      Cnt--;
      if (Code & 0x0200)
        WAsmCode[0] = (WAsmCode[0] - Cnt) & 15;
      WAsmCode[0] |= Code | (Cnt << 4) | (GetAL() << 2);
      CodeLen = 2;
    }
  }
}

static void DecodeJmp(Word Code)
{
  Integer AdrInt; 
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else if (OpSize != eOpSizeDefault) WrError(1130);
  {
    AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (Odd(AdrInt)) WrError(1375);
      else if ((!SymbolQuestionable) && ((AdrInt<-1024) || (AdrInt>1022))) WrError(1370);
      else
      {
        if (Odd(EProgCounter())) WrError(180);
        WAsmCode[0] = Code | ((AdrInt >> 1) & 0x3ff);
        CodeLen = 2;
      }
    }
  }
}

static void DecodeBYTE(Word Index)
{
  Boolean OK;
  int z;
  TempResult t;

  UNUSED(Index);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    z = 1; OK = True;
    do
    {
      KillBlanks(ArgStr[z]);
      FirstPassUnknown = False;
      EvalExpression(ArgStr[z], &t);
      switch (t.Typ)
      {
        case TempInt:
          if (FirstPassUnknown) t.Contents.Int &= 0xff;
          if (!RangeCheck(t.Contents.Int, Int8)) WrError(1320);
          else if (SetMaxCodeLen(CodeLen + 1))
          {
            WrError(1920); OK = False;
          }
          else PutByte(t.Contents.Int);
          break;
        case TempFloat:
          WrError(1135); OK = False;
          break;
        case TempString:
        {
          unsigned l = t.Contents.Ascii.Length;

          if (SetMaxCodeLen(l + CodeLen))
          {
            WrError(1920); OK = False;
          }
          else
          {
            char *pEnd = t.Contents.Ascii.Contents + l, *p;

            TranslateString(t.Contents.Ascii.Contents, l);
            for (p = t.Contents.Ascii.Contents; p < pEnd; PutByte(*(p++)));
          }
          break;
        }
        default: 
          OK = False; break;
      }
      z++;
    }
    while ((z <= ArgCnt) && (OK));
    if (!OK) CodeLen = 0;
    else if ((Odd(CodeLen)) && (DoPadding)) PutByte(0);
  }
}

static void DecodeWORD(Word Index)
{
  int z;
  Word HVal16;
  Boolean OK;

  UNUSED(Index);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    z = 1; OK = True;
    do
    {
      HVal16 = EvalIntExpression(ArgStr[z], Int16, &OK);
      if (OK)
      {
        WAsmCode[CodeLen >> 1] = HVal16;
        CodeLen += 2;
      }
      z++;
    }
    while ((z <= ArgCnt) && (OK));
    if (!OK) CodeLen = 0;
  }
}

static void DecodeBSS(Word Index)
{
  Word HVal16;
  Boolean OK;

  UNUSED(Index);

  if (ArgCnt!=1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    HVal16 = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (FirstPassUnknown) WrError(1820);
    else if (OK)
    {
      if ((Odd(HVal16)) && (DoPadding)) HVal16++;
      if (!HVal16) WrError(290);
      DontPrint = True; CodeLen = HVal16;
      BookKeeping();
    }
  }
}

static void DecodeRegDef(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else AddRegDef(LabPart, ArgStr[1]);
}

static void DecodeRPT(Word Code)
{
  char *pOpPart, *pArgPart1, *pAttrPart;
  Boolean OK;

  /* fundamentals */

  if (ArgCnt < 1)
  {
    WrError(1110);
    return;
  }
  if (*AttrPart != '\0')
  {
    WrError(1100);
    return;
  }

  /* multiplier argument */

  pOpPart = FirstBlank(ArgStr[1]);
  if (!pOpPart)
  {
    WrError(1110);
    return;
  }
  *pOpPart++ = '\0';
  MultPrefix = Code | GetMult(ArgStr[1], &OK);
  if (!OK)
    return;

  /* new OpPart: */

  KillPrefBlanks(pOpPart);
  pArgPart1 = FirstBlank(pOpPart);
  if (!pArgPart1)
  {
    WrError(1110);
    return;
  }
  *pArgPart1++ = '\0';
  strcpy(OpPart, pOpPart);
  UpString(OpPart);
  KillPrefBlanks(pArgPart1);
  strmov(ArgStr[1], pArgPart1);

  /* split off new attribute part: */

  pAttrPart = strrchr(OpPart, '.');
  if (pAttrPart)
  {
    strcpy(AttrPart, pAttrPart + 1);
    *pAttrPart = '\0';
  }
  else
    *AttrPart = '\0';

  /* prefix 0x0000 is rptc #1 and effectively a NOP prefix: */

  MakeCode();
  if (MultPrefix)
  {
    WrError(1560);
    CodeLen = 0;
  }
}

/*-------------------------------------------------------------------------*/

#define AddFixed(NName, NCode) \
        AddInstTable(InstTable, NName, NCode, DecodeFixed)

static void AddTwoOp(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeTwoOp);
  if (MomCPU >= CPUMSP430X)
  {
    char XName[20];

    sprintf(XName, "%sX", NName);
    AddInstTable(InstTable, XName, NCode, DecodeTwoOpX);
  }
}

static void AddEmulOneToTwo(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeEmulOneToTwo);
}

static void AddEmulOneToTwoX(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeEmulOneToTwoX);
}

static void AddOneOp(char *NName, Boolean NMay, Boolean AllowX, Word NCode)
{
  if (InstrZ >= OneOpCount) exit(255);
  OneOpOrders[InstrZ].MayByte = NMay;
  OneOpOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ, DecodeOneOp);
  if ((MomCPU >= CPUMSP430X) && AllowX)
  {
    char XName[20];

    sprintf(XName, "%sX", NName);
    AddInstTable(InstTable, XName, InstrZ, DecodeOneOpX);
  }
  InstrZ++;
}

#define AddJmp(NName, NCode) \
        AddInstTable(InstTable, NName, NCode, DecodeJmp)

static void InitFields(void)
{
  InstTable = CreateInstTable(207);
  SetDynamicInstTable(InstTable);

  AddFixed("RETI", 0x1300);
  AddFixed("CLRC", 0xc312);
  AddFixed("CLRN", 0xc222);
  AddFixed("CLRZ", 0xc322);
  AddFixed("DINT", 0xc232);
  AddFixed("EINT", 0xd232);
  AddFixed("NOP" , NOPCode);
  AddFixed("RET" , 0x4130);
  AddFixed("SETC", 0xd312);
  AddFixed("SETN", 0xd222);
  AddFixed("SETZ", 0xd322);

  AddTwoOp("MOV" , 0x4000); AddTwoOp("ADD" , 0x5000);
  AddTwoOp("ADDC", 0x6000); AddTwoOp("SUBC", 0x7000);
  AddTwoOp("SUB" , 0x8000); AddTwoOp("CMP" , 0x9000);
  AddTwoOp("DADD", 0xa000); AddTwoOp("BIT" , 0xb000);
  AddTwoOp("BIC" , 0xc000); AddTwoOp("BIS" , 0xd000);
  AddTwoOp("XOR" , 0xe000); AddTwoOp("AND" , 0xf000);

  AddEmulOneToTwo("ADC" , 0x6000); /* ADDC #0, dst */
  AddInstTable(InstTable, "BR", 0x4000, DecodeBR); /* MOV dst, PC */
  AddEmulOneToTwo("CLR" , 0x4000); /* MOV #0, dst */
  AddEmulOneToTwo("DADC", 0xa000); /* DADD #0, dst */
  AddEmulOneToTwo("DEC" , 0x8001); /* SUB #1, dst */
  AddEmulOneToTwo("DECD", 0x8002); /* SUB #2, dst */
  AddEmulOneToTwo("INC" , 0x5001); /* ADD #1, dst */
  AddEmulOneToTwo("INCD", 0x5002); /* ADD #2, dst */
  AddEmulOneToTwo("INV" , 0xe0ff); /* XOR #-1, dst */
  AddInstTable(InstTable, "POP", 0x4000, DecodePOP); /* MOV @SP+,dst */
  AddEmulOneToTwo("RLA" , 0x50aa); /* ADD dst, dst */
  AddEmulOneToTwo("RLC" , 0x60aa); /* ADDC dst, dst */
  AddEmulOneToTwo("SBC" , 0x7000); /* SUBC #0, dst */
  AddEmulOneToTwo("TST" , 0x9000); /* CMP #0, dst */

  OneOpOrders = (OneOpOrder *) malloc(sizeof(OneOpOrder) * OneOpCount); InstrZ = 0;
  AddOneOp("RRC" , True , True , 0x1000); AddOneOp("RRA" , True , True , 0x1100);
  AddOneOp("PUSH", True , True , 0x1200); AddOneOp("SWPB", False, True , 0x1080);
  AddOneOp("CALL", False, False, 0x1280); AddOneOp("SXT" , False, True , 0x1180);

  if (MomCPU >= CPUMSP430X)
  {
    /* what about  RRUX? */

    AddInstTable(InstTable, "MOVA", 0x0000, DecodeMOVA);
    AddInstTable(InstTable, "ADDA", 0x00a0, DecodeADDA_SUBA_CMPA);
    AddInstTable(InstTable, "CMPA", 0x0090, DecodeADDA_SUBA_CMPA);
    AddInstTable(InstTable, "SUBA", 0x00b0, DecodeADDA_SUBA_CMPA);

    AddInstTable(InstTable, "RRCM", 0x0040, DecodeRxM);
    AddInstTable(InstTable, "RRAM", 0x0140, DecodeRxM);
    AddInstTable(InstTable, "RLAM", 0x0240, DecodeRxM);
    AddInstTable(InstTable, "RRUM", 0x0340, DecodeRxM);

    AddInstTable(InstTable, "CALLA", 0x0000, DecodeCALLA);

    AddInstTable(InstTable, "PUSHM", 0x1400, DecodePUSHM_POPM);
    AddInstTable(InstTable, "POPM",  0x1600, DecodePUSHM_POPM);

    AddEmulOneToTwoX("ADCX", 0x6000); /* ADDCX #0, dst */
    AddInstTable(InstTable, "BRA", 0x4000, DecodeBRA); /* MOVA dst, PC */
    AddFixed("RETA", 0x0110); /* MOVA @SP+,PC */
    AddInstTable(InstTable, "CLRA", 0x4300, DecodeCLRA); /* MOV #0,Rdst */
    AddEmulOneToTwoX("CLRX", 0x4000); /* MOVX #0, dest */
    AddEmulOneToTwoX("DADCX", 0xa000); /* DADDX #0, dst */
    AddEmulOneToTwoX("DECX" , 0x8001); /* SUBX #1, dst */
    AddInstTable(InstTable, "DECDA", 0x00b0, DecodeDECDA_INCDA); /* SUBA #2,Rdst */
    AddEmulOneToTwoX("DECDX", 0x8002); /* SUBX #2, dst */
    AddEmulOneToTwoX("INCX" , 0x5001); /* SUBX #1, dst */
    AddInstTable(InstTable, "INCDA", 0x00a0, DecodeDECDA_INCDA); /* SUBA #2,Rdst */
    AddEmulOneToTwoX("INCDX", 0x5002); /* SUBX #2, dst */
    AddEmulOneToTwoX("INVX" , 0xe0ff); /* XORX #-1, dst */
    AddEmulOneToTwoX("RLAX" , 0x50aa); /* ADDX dst, dst */
    AddEmulOneToTwoX("RLCX" , 0x60aa); /* ADDCX dst, dst */
    AddEmulOneToTwoX("SBCX" , 0x7000); /* SUBCX #0, dst */
    AddInstTable(InstTable, "TSTA" , 0x0090, DecodeTSTA); /* CMPA #0,Rdst */
    AddEmulOneToTwoX("TSTX" , 0x9000); /* CMPX #0, dst */
    AddInstTable(InstTable, "POPX", 0x4000, DecodePOPX); /* MOVX @SP+,dst */

    AddInstTable(InstTable, "RPTC", 0x0000, DecodeRPT);
    AddInstTable(InstTable, "RPTZ", 0x0100, DecodeRPT);
  }

  AddJmp("JNE" , 0x2000); AddJmp("JNZ" , 0x2000);
  AddJmp("JE"  , 0x2400); AddJmp("JZ"  , 0x2400);
  AddJmp("JNC" , 0x2800); AddJmp("JC"  , 0x2c00);
  AddJmp("JN"  , 0x3000); AddJmp("JGE" , 0x3400);
  AddJmp("JL"  , 0x3800); AddJmp("JMP" , 0x3C00);
  AddJmp("JEQ" , 0x2400); AddJmp("JLO" , 0x2800);
  AddJmp("JHS" , 0x2c00);

  AddInstTable(InstTable, "BYTE", 0, DecodeBYTE);
  AddInstTable(InstTable, "WORD", 0, DecodeWORD);
  AddInstTable(InstTable, "BSS" , 0, DecodeBSS);

  AddInstTable(InstTable, "REG", 0, DecodeRegDef);
}

static void DeinitFields(void)
{
  free(OneOpOrders);

  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_MSP(void)
{
  CodeLen = 0; DontPrint = False; PCDist = 0;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Attribut bearbeiten */

  if (*AttrPart == '\0') OpSize = eOpSizeDefault;
  else if (strlen(AttrPart) > 1) WrXError(1107, AttrPart);
  else switch (mytoupper(*AttrPart))
  {
    case 'B':
      OpSize = eOpSizeB;
      break;
    case 'W':
      OpSize = eOpSizeW;
      break;
    case 'A':
      if (MomCPU >= CPUMSP430X)
      {
        OpSize = eOpSizeA;
        break;
        /* conditional break */
      }
    default:
      WrXError(1107, AttrPart);
      return;
  }

  /* alles aus der Tabelle */
 
  if (!LookupInstTable(InstTable,OpPart))
    WrXError(1200,OpPart);
}

static Boolean IsDef_MSP(void)
{
  return Memo("REG");
}

static void SwitchFrom_MSP(void)
{
  DeinitFields();
  ClearONOFF();
}

static void SwitchTo_MSP(void)
{
  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

  PCSymbol = "$"; HeaderID = 0x4a; NOPCode = 0x4303; /* = MOV #0,#0 */
  DivideChars = ","; HasAttrs = True; AttrChars = ".";

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  AdrIntType = (MomCPU == CPUMSP430X) ? UInt20 : UInt16;
  DispIntType = (MomCPU == CPUMSP430X) ? Int20 : Int16;
  SegLimits[SegCode] = IntTypeDefs[AdrIntType].Max;

  AddONOFF("PADDING", &DoPadding, DoPaddingName, False);

  MakeCode = MakeCode_MSP; IsDef = IsDef_MSP;
  SwitchFrom = SwitchFrom_MSP; InitFields();

  MultPrefix = 0x0000;
}

void codemsp_init(void)
{
  CPUMSP430 = AddCPU("MSP430", SwitchTo_MSP);
  CPUMSP430X = AddCPU("MSP430X", SwitchTo_MSP);
}
