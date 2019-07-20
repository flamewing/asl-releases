/* code78k2.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 78K2-Familie                                                */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"
#include "headids.h"
#include "errmsg.h"

#include "code78k2.h"

/*-------------------------------------------------------------------------*/

enum
{
  ModNone = -1,
  ModImm = 0,
  ModReg8 = 1,
  ModReg16 = 2,
  ModMem = 3,
  ModAbs = 4,
  ModShort = 5,
  ModSFR = 6,
  ModPSW = 7,
  ModSP = 8,
  ModSTBC = 9
};

#define MModImm (1 << ModImm)
#define MModReg8 (1 << ModReg8)
#define MModReg16 (1 << ModReg16)
#define MModMem (1 << ModMem)
#define MModAbs (1 << ModAbs)
#define MModShort (1 << ModShort)
#define MModSFR (1 << ModSFR)
#define MModPSW (1 << ModPSW)
#define MModSP (1 << ModSP)
#define MModSTBC (1 << ModSTBC)

#define AccReg8 1
#define AccReg16 0

#define SFR_SP 0xfc
#define SFR_PSW 0xfe

/*-------------------------------------------------------------------------*/

static CPUVar CPU78214;

static ShortInt AdrMode, AdrVal;
static Byte AdrVals[3];
static ShortInt OpSize;
static Boolean AltBank;
static Byte *pCode;
static LongInt Reg_P6, Reg_PM6;

static ASSUMERec ASSUME78K2s[] =
{
  {"P6"  , &Reg_P6  , 0,  0xf,  0x10, NULL},
  {"PM6" , &Reg_PM6 , 0,  0xf,  0x10, NULL}
};

/*-------------------------------------------------------------------------*/
/* address decoders */

static Boolean SetOpSize(ShortInt NewSize)
{
  if (OpSize < 0)
  {
    OpSize = NewSize; 
    return True;
  }
  else if (OpSize != NewSize)
  {
    WrError(ErrNum_ConfOpSizes);
    return False;
  }
  else
    return True;
}

static ShortInt DecodeReg8(char *pAsc)
{
  ShortInt Result = -1;
  int l = strlen(pAsc);

  if (l == 1) 
  {
    static char Reg8Names[9] = "XACBEDLH";
    char *pPos = strchr(Reg8Names, mytoupper(*pAsc));

    if (pPos)
      Result = pPos - Reg8Names;
  }

  else if ((l == 2) && (toupper(pAsc[0]) == 'R'))
  {
    if ((pAsc[1] >= '0') && (pAsc[1] <= '7'))
      Result = pAsc[1] - '0';
  }

  return Result;
}

static ShortInt DecodeReg16(char *pAsc)
{
  ShortInt Result = -1;
  int l = strlen(pAsc);

  if (l == 2)
  {
    static char *Reg16Names[4] = {"AX", "BC", "DE", "HL"};
    int z;

    for (z = 0; z < 4; z++)
      if (!strcasecmp(Reg16Names[z], pAsc))
      {
        Result = z;
        break;
      }
  }
   
  else if ((l == 3) && (toupper(pAsc[0]) == 'R') && (toupper(pAsc[1]) == 'P'))
  {
    if ((pAsc[2] >= '0') && (pAsc[2] <= '3'))
      Result = pAsc[2] - '0';
  }
   
  return Result;
}
 
static void DecodeAdr(const tStrComp *pArg, Word Mask)
{
  Word WordOp;
  LongWord LongOp;
  Boolean OK;
  tStrComp Arg;
  unsigned ForceLong;
  int ArgLen;

  AdrMode = ModNone; AdrCnt = 0; AltBank = False;

  /* immediate ? */

  if (*pArg->Str == '#')
  {
    switch (OpSize)
    {
      case 0:
        AdrVals[0] = EvalStrIntExpressionOffs(pArg, 1, Int8, &OK);
        if (OK)
        {
          AdrCnt = 1;
          AdrMode = ModImm;
        }
        break;
      case 1:
        WordOp = EvalStrIntExpressionOffs(pArg, 1, Int16, &OK);
        if (OK)
        {
          AdrVals[0] = Lo(WordOp);
          AdrVals[1] = Hi(WordOp);
          AdrCnt = 2;
          AdrMode = ModImm;
        }
        break;
      default:
        WrError(ErrNum_UndefOpSizes);
    }
    goto AdrFound;
  }

  /* 8 bit registers? */

  if ((AdrVal = DecodeReg8(pArg->Str)) >= 0)
  {
    AdrMode = ModReg8;
    SetOpSize(0);
    goto AdrFound;
  }

  if (!strcasecmp(pArg->Str, "PSW"))
  {
    AdrMode = ModPSW;
    SetOpSize(0);
    goto AdrFound;
  }

  if (!strcasecmp(pArg->Str, "STBC"))
  {
    AdrMode = ModSTBC;
    SetOpSize(0);
    goto AdrFound;
  }

  /* 16 bit registers? */

  if ((AdrVal = DecodeReg16(pArg->Str)) >= 0)
  {
    AdrMode = ModReg16;
    SetOpSize(1);
    goto AdrFound;
  }

  if (!strcasecmp(pArg->Str, "SP"))
  {
    AdrMode = ModSP;
    SetOpSize(1); 
    goto AdrFound;
  }

  /* OK, everything that follows is memory: alternate bank ? */

  Arg = *pArg;
  if (*Arg.Str == '&')
  {
    AltBank = True;
    StrCompIncRefLeft(&Arg, 1);
  }

  /* memory-indirect addressing? */

  ArgLen = strlen(Arg.Str);
  if ((ArgLen >= 2) && (Arg.Str[ArgLen - 1] == ']'))
  {
    tStrComp Base, Remainder;
    char *pStart;

    /* remove ']' */

    StrCompShorten(&Arg, 1);

    pStart = RQuotPos(Arg.Str, '[');
    if (!pStart)
    {
      WrError(ErrNum_BrackErr);
      goto AdrFound;
    }

    /* purely indirect? */

    if (pStart == Arg.Str)
    {
      static char *Modes[] = { "DE+",  "HL+",  "DE-",  "HL-",  "DE",  "HL",
                               "RP2+", "RP3+", "RP2-", "RP3-", "RP2", "RP3" };
      unsigned z;
      char *pSep, Save;

      /* skip '[' */

      StrCompIncRefLeft(&Arg, 1);

      /* simple expression without displacement? */

      for (z = 0; z < sizeof(Modes) / sizeof(*Modes); z++)
        if (!strcasecmp(Arg.Str, Modes[z]))
        {
          AdrMode = ModMem; AdrVal = 0x16;
          AdrVals[0] = z % (sizeof(Modes) / sizeof(*Modes) / 2); 
          AdrCnt = 1;
          goto AdrFound;
        }

      /* no -> extract base register. Its name ends with the first non-letter,
         which either means +/- or a blank */

      for (pSep = Arg.Str; *pSep; pSep++)
        if (!myisalpha(*pSep))
          break;
      
      /* decode base register.  SP is not otherwise handled. */

      Save = StrCompSplitRef(&Base, &Remainder, &Arg, pSep);
      if (!strcasecmp(Base.Str, "SP"))
        AdrVals[0] = 1;
      else 
      {
        int tmp;

        tmp = DecodeReg16(Base.Str);
        if (tmp == 2) /* DE */
          AdrVals[0] = 0;
        else if (tmp == 3) /* HL */ 
          AdrVals[0] = 2;
        else
        {
          WrStrErrorPos(ErrNum_InvReg, &Base);
          goto AdrFound;
        }     
      }  

      /* now that we have the base, prepare displacement. */

      *pSep = Save; 
      if (pSep > Arg.Str)
        pSep--;
      *pSep = '0';
      AdrVals[1] = EvalStrIntExpressionOffs(&Arg, pSep - Arg.Str, Int8, &OK);
      if (OK)
      {
        AdrMode = ModMem; AdrVal = 0x06;
        AdrCnt = 2;
        goto AdrFound;
      } 
    }

    /* no -> with outer displacement */

    else
    {
      tStrComp Disp, Reg;
      int tmp;

      /* split displacement + register */

      StrCompSplitRef(&Disp, &Reg, &Arg, pStart);

       /* handle base register */

      tmp = DecodeReg8(Reg.Str);
      switch (tmp)
      {
        case 1: /* A/B */
        case 3:
          AdrVals[0] = tmp;
          break;
        case -1:
          tmp = DecodeReg16(Reg.Str);
          if (tmp >= 2) /* DE/HL */
          {
            AdrVals[0] = (tmp - 2) << 1;
            break; 
          }    
        default:
          WrStrErrorPos(ErrNum_InvReg, &Reg);
          goto AdrFound;
      }

      /* compute displacement */

      WordOp = EvalStrIntExpression(&Disp, Int16, &OK);
      if (OK)
      {
        AdrMode = ModMem; AdrVal = 0x0a;
        AdrVals[1] = Lo(WordOp); AdrVals[2] = Hi(WordOp);
        AdrCnt = 3;
        goto AdrFound; 
      }
    }

  }

  /* OK, nothing but absolute left...exclamation mark enforces 16-bit addressing */

  ForceLong = !!(*Arg.Str == '!');

  FirstPassUnknown = False;
  LongOp = EvalStrIntExpressionOffs(&Arg, ForceLong, UInt20, &OK);
  if (OK)
  {
    if (!FirstPassUnknown)
    {
      LongWord CompBank = AltBank ? Reg_P6 : Reg_PM6;
      if (CompBank != (LongOp >> 16)) WrError(ErrNum_InAccPage);
    }

    WordOp = LongOp & 0xffff;

    if ((Mask & MModShort) && (!ForceLong) && ((WordOp >= 0xfe20) && (WordOp <= 0xff1f)))
    {
      AdrMode = ModShort; AdrCnt = 1;
      AdrVals[0] = Lo(WordOp);
    }
    else if ((Mask & MModSFR) && (!ForceLong) && (Hi(WordOp) == 0xff))
    {                                                                                    
      AdrMode = ModSFR; AdrCnt = 1;
      AdrVals[0] = Lo(WordOp);                        
    }
    else
    {
      AdrMode = ModAbs; AdrCnt = 2;
      AdrVals[0] = Lo(WordOp); AdrVals[1] = Hi(WordOp);
    }
  }

AdrFound:

  if ((AdrMode != ModNone) && (!(Mask & (1 << AdrMode))))
  {
    WrError(ErrNum_InvAddrMode);
    AdrMode = ModNone; AdrCnt = 0; AltBank = False;
  }
}

static Boolean ChkAcc(const tStrComp *pArg)
{
  DecodeAdr(pArg, OpSize ? MModReg16 : MModReg8);
  if (AdrMode == ModNone)
    return False;

  if (((OpSize) && (AdrVal != AccReg16))
   || ((!OpSize) && (AdrVal != AccReg8)))
  {
    WrError(ErrNum_InvAddrMode);
    return False;
  }

  return True;
}

static Boolean ChkMem1(void)
{
  return (AdrVal == 0x16) && (*AdrVals >= 4);
}

static Boolean DecodeBitAdr(const tStrComp *pArg, LongWord *pResult)
{
  char *pSplit;
  Boolean OK;

  pSplit = RQuotPos(pArg->Str, '.');

  if (pSplit)
  {
    tStrComp Reg, Bit;

    StrCompSplitRef(&Reg, &Bit, pArg, pSplit);

    *pResult = EvalStrIntExpression(&Bit, UInt3, &OK) << 8;
    if (OK)
    {
      DecodeAdr(&Reg, MModReg8 | MModPSW | MModSFR | MModShort);
      switch (AdrMode)
      {
        case ModReg8:
          if (AdrVal >= 2)
          {
            WrStrErrorPos(ErrNum_InvReg, &Reg);
            OK = FALSE;
          }
          else
            *pResult |= (((LongWord)AdrVal) << 11) | 0x00030000;
          break;
        case ModPSW:
          *pResult |= 0x00020000;
          break;
        case ModSFR:
          *pResult |= 0x01080800 | *AdrVals;
          break;
        case ModShort:
          *pResult |= 0x01080000 | *AdrVals;
          break;
        default:
          OK = FALSE;
      }
    }
  }
  else
    *pResult = EvalStrIntExpression(pArg, UInt32, &OK);

  return OK;
}

/*-------------------------------------------------------------------------*/
/* instruction decoders */

static void DecodeFixed(Word Index)
{
  if (ChkArgCnt(0, 0))
    *pCode++ = Index; 
}

static void DecodeMOV(Word Index)
{
  Byte HReg;

  UNUSED(Index);

  SetOpSize(0);
  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg8 | MModShort | MModSFR | MModMem | MModAbs | MModPSW | MModSTBC);
    switch (AdrMode)
    {
      case ModReg8:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModImm | MModReg8 | (HReg == AccReg8 ? (MModShort | MModSFR | MModAbs | MModMem | MModPSW) : 0));
        switch (AdrMode)
        {
          case ModImm:
            *pCode++ = 0xb8 | HReg;
            *pCode++ = *AdrVals;
            break;
          case ModReg8:
            if (HReg == AccReg8)
              *pCode++ = 0xd0 | AdrVal;
            else
            {
              *pCode++ = 0x24;
              *pCode++ = (HReg << 4) | AdrVal;
            }
            break;
          case ModSFR:
            *pCode++ = 0x10;
            *pCode++ = *AdrVals;
            break;
          case ModShort:
            *pCode++ = 0x20;
            *pCode++ = *AdrVals;   
            break;
          case ModAbs:
            if (AltBank)
              *pCode++ = 0x01;
            *pCode++ = 0x09;
            *pCode++ = 0xf0;
            *pCode++ = *AdrVals;
            *pCode++ = 1[AdrVals]; 
            break;
          case ModMem:
            if (AltBank)
              *pCode++ = 0x01;
            if (AdrVal == 0x16)
              *pCode++ = 0x58 | *AdrVals;
            else
            {
              *pCode++ = 0x00 | AdrVal;
              *pCode++ = *AdrVals << 4;
              memcpy(pCode, AdrVals + 1, AdrCnt - 1);
              pCode += AdrCnt - 1;
            }
            break;
          case ModPSW:
            *pCode++ = 0x10;
            *pCode++ = SFR_PSW;
            break;
        }
        break;

      case ModShort:
        HReg = *AdrVals;
        DecodeAdr(&ArgStr[2], MModImm | MModReg8 | MModShort);
        switch (AdrMode)
        {
          case ModImm:
            *pCode++ = 0x3a;
            *pCode++ = HReg;
            *pCode++ = *AdrVals;   
            break;
          case ModReg8:
            if (AdrVal != AccReg8) WrError(ErrNum_InvAddrMode);
            else
            {
              *pCode++ = 0x22;
              *pCode++ = HReg;
            }
            break;
          case ModShort:
            *pCode++ = 0x38;
            *pCode++ = HReg;
            *pCode++ = *AdrVals;
            break;
        }
        break;

      case ModSFR:
        HReg = *AdrVals;
        DecodeAdr(&ArgStr[2], MModImm | MModReg8);
        switch (AdrMode)
        {
          case ModImm:
            *pCode++ = 0x2b;
            *pCode++ = HReg;
            *pCode++ = *AdrVals;   
            break;
          case ModReg8:
            if (AdrVal != AccReg8) WrError(ErrNum_InvAddrMode);
            else
            {
              *pCode++ = 0x12;
              *pCode++ = HReg;
            }
            break;
        }
        break;

      case ModPSW:
        DecodeAdr(&ArgStr[2], MModImm | MModReg8);
        switch (AdrMode)
        {
          case ModImm:
            *pCode++ = 0x2b;
            *pCode++ = SFR_PSW;
            *pCode++ = *AdrVals;
            break;
          case ModReg8:
            if (AdrVal != AccReg8) WrError(ErrNum_InvAddrMode);
            else
            {   
              *pCode++ = 0x12;
              *pCode++ = SFR_PSW;
            }
            break;
        }
        break;

      case ModSTBC:
        DecodeAdr(&ArgStr[2], MModImm);
        switch (AdrMode)
        {
          case ModImm:
            *pCode++ = 0x09;
            *pCode++ = 0xc0;
            *pCode++ = *AdrVals;
            *pCode++ = ~(*AdrVals);
            break;
        }
        break;

      /* only works against ACC - dump result first, since DecodeAdr 
         destroys values */

      case ModMem:
        if (AltBank)
          *pCode++ = 0x01;
        if (AdrVal == 0x16)
          *pCode++ = 0x50 | *AdrVals;
        else
        {
          *pCode++ = 0x00 | AdrVal;
          *pCode++ = 0x80 | (*AdrVals << 4);
          memcpy(pCode, AdrVals + 1, AdrCnt - 1);
          pCode += AdrCnt - 1;
        }
        if (!ChkAcc(&ArgStr[2]))
          pCode = BAsmCode;
        break;

      case ModAbs:
        if (AltBank)
          *pCode++ = 0x01;
        *pCode++ = 0x09;
        *pCode++ = 0xf1;
        *pCode++ = *AdrVals;
        *pCode++ = 1[AdrVals];
        if (!ChkAcc(&ArgStr[2]))
          pCode = BAsmCode;
        break;
    }
  }
}

static void DecodeXCH(Word Index)
{
  Byte HReg;

  UNUSED(Index);

  SetOpSize(0);
  if (ChkArgCnt(2, 2))
  {   
    DecodeAdr(&ArgStr[1], MModReg8 | MModShort | MModSFR | MModMem);

    switch (AdrMode)
    {
      case ModReg8:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg8 | ((HReg == AccReg8) ? (MModShort | MModSFR | MModMem) : 0));
        switch (AdrMode)
        {
          case ModReg8:
            if (HReg == AccReg8)
              *pCode++ = 0xd8 | AdrVal;
            else if (AdrVal == AccReg8)
              *pCode++ = 0xd8 | HReg;
            else
            {
              *pCode++ = 0x25;
              *pCode++ = (HReg << 4) | AdrVal;
            }
            break;
          case ModShort:
            *pCode++ = 0x21;
            *pCode++ = *AdrVals;
            break;
          case ModSFR:
            *pCode++ = 0x01;
            *pCode++ = 0x21;
            *pCode++ = *AdrVals;
            break;
          case ModMem:
            if (AltBank)
              *pCode++ = 0x01;
            *pCode++ = AdrVal;
            *pCode++ = (*AdrVals << 4) | 0x04;
            memcpy(pCode, AdrVals + 1, AdrCnt - 1);
            pCode += AdrCnt - 1;
            break;
        }
        break;

      case ModShort:
        HReg = *AdrVals;
        DecodeAdr(&ArgStr[2], MModReg8 | MModShort);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8) WrError(ErrNum_InvAddrMode);
            else
            {
              *pCode++ = 0x21;
              *pCode++ = HReg;
            }
            break;
          case ModShort:
            *pCode++ = 0x39;
            *pCode++ = HReg;
            *pCode++ = *AdrVals;
            break;
        }
        break;

      case ModSFR:
        HReg = *AdrVals;
        if (ChkAcc(&ArgStr[2]))
        {
          *pCode++ = 0x01;
          *pCode++ = 0x21;
          *pCode++ = *AdrVals;
        }
        break;

      case ModMem:
        if (AltBank)
          *pCode++ = 0x01;
        *pCode++ = AdrVal;
        *pCode++ = (*AdrVals << 4) | 0x04;
        memcpy(pCode, AdrVals + 1, AdrCnt - 1);
        pCode += AdrCnt - 1;
        if (!ChkAcc(&ArgStr[2]))
          pCode = BAsmCode;
        break;
    }
  }
}

static void DecodeMOVW(Word Index)
{
  Byte HReg;

  UNUSED(Index);

  SetOpSize(1);
  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg16 | MModSP | MModShort | MModSFR | MModMem);
    switch (AdrMode)
    {
      case ModReg16:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg16 | MModImm | ((HReg == AccReg16) ? (MModSP | MModShort | MModSFR | MModMem) : 0));
        switch (AdrMode)
        {
          case ModReg16:
            *pCode++ = 0x24;
            *pCode++ = 0x08 | (HReg << 5) | (AdrVal << 1);
            break;
          case ModImm:
            *pCode++ = 0x60 | (HReg << 1);
            *pCode++ = *AdrVals;
            *pCode++ = 1[AdrVals];
            break;
          case ModShort:
            *pCode++ = 0x1c;
            *pCode++ = *AdrVals;
            break;
          case ModSFR:
            *pCode++ = 0x11;
            *pCode++ = *AdrVals;
            break;
          case ModSP:
            *pCode++ = 0x11;
            *pCode++ = SFR_SP;
            break;
          case ModMem:
            if (ChkMem1())
            {
              if (AltBank)
                *pCode++ = 0x01;
              *pCode++ = 0x05;
              *pCode++ = 0xe2 | (*AdrVals & 0x01);
            }
            break;
        }
        break;

      case ModSP:
        DecodeAdr(&ArgStr[2], MModReg16 | MModImm);
        switch (AdrMode)
        {
          case ModReg16:
            if (AdrVal != AccReg16) WrError(ErrNum_InvAddrMode);
            else
            {
              *pCode++ = 0x13;
              *pCode++ = SFR_SP;
            }
            break;
          case ModImm:
            *pCode++ = 0x0b;
            *pCode++ = SFR_SP;
            *pCode++ = *AdrVals;
            *pCode++ = 1[AdrVals];
            break;
        }
        break;

      case ModShort:
        HReg = *AdrVals;
        DecodeAdr(&ArgStr[2], MModReg16 | MModImm);
        switch (AdrMode)
        {
          case ModReg16:
            if (AdrVal != AccReg16) WrError(ErrNum_InvAddrMode);
            else
            {
              *pCode++ = 0x1a;
              *pCode++ = HReg;
            }
            break;
          case ModImm:
            *pCode++ = 0x0c;
            *pCode++ = HReg;
            *pCode++ = *AdrVals;
            *pCode++ = 1[AdrVals];
            break;
        }
        break;

      case ModSFR:
        HReg = *AdrVals;
        DecodeAdr(&ArgStr[2], MModReg16 | MModImm);
        switch (AdrMode)
        {
          case ModReg16:
            if (AdrVal != AccReg16) WrError(ErrNum_InvAddrMode);
            else
            {
              *pCode++ = 0x13;
              *pCode++ = HReg;
            }
            break;
          case ModImm:
            *pCode++ = 0x0b;
            *pCode++ = HReg;
            *pCode++ = *AdrVals;
            *pCode++ = 1[AdrVals];
            break;
        }
        break;

      case ModMem:
        if (ChkMem1())
        {
          if (AltBank)
            *pCode++ = 0x01;
          *pCode++ = 0x05;
          *pCode++ = 0xe6 | (*AdrVals & 0x01);
          if (!ChkAcc(&ArgStr[2]))
            pCode = BAsmCode;
        }
        break;
    }
  }
}

static void DecodeALU(Word Index)
{
  Byte HReg;

  SetOpSize(0);
  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg8 | MModShort | MModSFR);
    switch (AdrMode)
    {
      case ModReg8:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg8 | ((HReg == AccReg8) ? (MModImm | MModShort | MModSFR | MModMem): 0));
        switch (AdrMode)
        {
          case ModReg8:
            *pCode++ = 0x88 | Index;
            *pCode++ = (HReg << 4) | AdrVal;
            break;
          case ModImm:
            *pCode++ = 0xa8 | Index;
            *pCode++ = *AdrVals;
            break;
          case ModShort:
            *pCode++ = 0x98 | Index;
            *pCode++ = *AdrVals;
            break;
          case ModSFR:
            *pCode++ = 0x01;
            *pCode++ = 0x98 | Index;
            *pCode++ = *AdrVals;
            break;
          case ModMem:
            if (AltBank)
              *pCode++ = 0x01;
            *pCode++ = 0x00 | AdrVal;
            *pCode++ = 0x08 | (*AdrVals << 4) | Index;
            memcpy(pCode, AdrVals + 1, AdrCnt - 1);
            pCode += AdrCnt - 1;
            break;
        }
        break;

      case ModShort:
        HReg = *AdrVals;
        DecodeAdr(&ArgStr[2], MModImm | MModShort);
        switch (AdrMode)
        {
          case ModImm:
            *pCode++ = 0x68 | Index;
            *pCode++ = HReg;
            *pCode++ = *AdrVals;
            break;
          case ModShort:
            *pCode++ = 0x78 | Index;
            *pCode++ = *AdrVals;
            *pCode++ = HReg;
            break;
        }
        break;

      case ModSFR:
        HReg = *AdrVals;
        DecodeAdr(&ArgStr[2], MModImm);
        switch (AdrMode)
        {
          case ModImm:
            *pCode++ = 0x01;
            *pCode++ = 0x68 | Index;
            *pCode++ = HReg;
            *pCode++ = *AdrVals;
            break;
        }
        break;
    }
  }
}

static void DecodeALU16(Word Index)
{
  static Byte Vals[3] = { 0, 2, 7 };

  if (ChkArgCnt(2, 2))
  {
    SetOpSize(1);
    if (ChkAcc(&ArgStr[1]))
    {
      DecodeAdr(&ArgStr[2], MModImm | MModReg16 | MModShort | MModSFR);
      switch (AdrMode)
      {
        case ModImm:
          *pCode++ = 0x2c | Index;
          *pCode++ = *AdrVals;
          *pCode++ = 1[AdrVals];
          break;
        case ModReg16:
          *pCode++ = 0x88 | Vals[Index - 1];
          *pCode++ = 0x08 | (AdrVal << 1);
          break;
        case ModShort:
          *pCode++ = 0x1c | Index;
          *pCode++ = *AdrVals;
          break;
        case ModSFR:
          *pCode++ = 0x01;
          *pCode++ = 0x1c | Index;
          *pCode++ = *AdrVals;
          break;
      }
    }
  }
}

static void DecodeMULDIV(Word Index)
{
  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModReg8);
    switch (AdrMode)
    {
      case ModReg8:
        *pCode++ = 0x05;
        *pCode++ = Index | AdrVal;
        break;
    }
  }
}

static void DecodeINCDEC(Word Index)
{
  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModReg8 | MModShort);
    switch (AdrMode)
    {
      case ModReg8:
        *pCode++ = 0xc0 | (Index << 3) | AdrVal;
        break;
      case ModShort:
        *pCode++ = 0x26 | Index;
        *pCode++ = *AdrVals;
        break;
    }
  }
}

static void DecodeINCDECW(Word Index)
{
  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModReg16 | MModSP);
    switch (AdrMode)
    {
      case ModReg16:
        *pCode++ = 0x44 | (Index << 3) | AdrVal;
        break;
      case ModSP:
        *pCode++ = 0x05;
        *pCode++ = 0xc0 | Index;
        break;
    }
  }
}

static void DecodeShift8(Word Index)
{
  Boolean OK;
  Byte Shift;

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg8);
    switch (AdrMode)
    {
      case ModReg8:
        Shift = EvalStrIntExpression(&ArgStr[2], UInt3, &OK);
        if (OK)
        {
          *pCode++ = 0x30 | Hi(Index);
          *pCode++ = Lo(Index) | (Shift << 3) | AdrVal;
        }
        break;
    }
  }
}

static void DecodeShift16(Word Index)
{
  Boolean OK;
  Byte Shift;

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg16);
    switch (AdrMode)
    {
      case ModReg16:
        Shift = EvalStrIntExpression(&ArgStr[2], UInt3, &OK);
        if (OK)
        {
          *pCode++ = 0x30 | Hi(Index);
          *pCode++ = Lo(Index) | (Shift << 3) | (AdrVal << 1);
        }
        break;
    }
  }
}

static void DecodeShift4(Word Index)
{
  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModMem);
    switch (AdrMode)
    {
      case ModMem:
        if (ChkMem1())
        {
          if (Index)
            *pCode++ = 0x01;
          *pCode++ = 0x05;
          *pCode++ = 0x8c | ((*AdrVals & 1) << 1);
        }
        break;
    }
  }
}

static void DecodePUSHPOP(Word Index)
{
  if (ChkArgCnt(1, 1))
  {                                                  
    DecodeAdr(&ArgStr[1], MModReg16 | MModPSW | MModSFR);
    switch (AdrMode)
    {
      case ModReg16:
        *pCode++ = 0x34 | (Index << 3) | AdrVal;
        break;
      case ModPSW:
        *pCode++ = 0x48 | Index;
        break;
      case ModSFR:
        *pCode++ = Index ? 0x29 : 0x43;
        *pCode++ = *AdrVals;
        break;
    }
  }
}

static void DecodeCALL(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModAbs | MModReg16);
    switch (AdrMode)
    {
      case ModAbs:
        *pCode++ = 0x28;
        *pCode++ = *AdrVals;
        *pCode++ = 1[AdrVals];
        break;
      case ModReg16:
        *pCode++ = 0x05;
        *pCode++ = 0x58 | (AdrVal << 1);
        break;
    }
  }
}

static void DecodeCALLF(Word Index)
{
  Word AdrWord;
  Boolean OK;

  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    FirstPassUnknown = FALSE;
    AdrWord = EvalStrIntExpressionOffs(&ArgStr[1], !!(*ArgStr[1].Str == '!'), UInt12, &OK);
    if (OK)
    {
      if (FirstPassUnknown)
        AdrWord |= 0x800;
      if (AdrWord < 0x800) WrError(ErrNum_UnderRange);
      else
      {
        *pCode++ = 0x90 | (Hi(AdrWord) & 7);
        *pCode++ = Lo(AdrWord);
      }
    }
  }
}

static void DecodeCALLT(Word Index)
{
  Word AdrWord;
  Boolean OK;
  int ArgLen;
  tStrComp Arg;

  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    Arg = ArgStr[1]; ArgLen = strlen(Arg.Str);
    if ((*Arg.Str != '[') || (Arg.Str[ArgLen - 1] != ']')) WrError(ErrNum_InvAddrMode);
    else
    {
      StrCompIncRefLeft(&Arg, 1);
      StrCompShorten(&Arg, 1);
      FirstPassUnknown = FALSE;
      AdrWord = EvalStrIntExpression(&Arg, UInt7, &OK);
      if (OK)
      {
        if (FirstPassUnknown)
        AdrWord = 0x40;
        if (ChkRange(AdrWord, 0x40, 0x7e))
        {
          if (AdrWord & 1) WrError(ErrNum_MustBeEven);
          else
          {
            *pCode++ = 0xe0 | ((AdrWord - 0x40) >> 1);
          }
        }
      }
    }
  }
}

static void DecodeBR(Word Index)
{
  Boolean Rel;

  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    tStrComp Arg = ArgStr[1];

    Rel = (*Arg.Str == '$');
    if (Rel)
      StrCompIncRefLeft(&Arg, 1);
    FirstPassUnknown = FALSE;
    DecodeAdr(&Arg, MModAbs | MModReg16);
    switch (AdrMode)
    {
      case ModAbs:
        if (Rel)
        {
          LongInt Addr = (((Word)1[AdrVals]) << 8) | (*AdrVals);

          Addr -= EProgCounter() + 2;
          if ((!FirstPassUnknown) && ((Addr < -128) || (Addr > 127))) WrError(ErrNum_JmpDistTooBig);
          else
          {
            *pCode++ = 0x14;
            *pCode++ = Addr & 0xff;
          }
        }
        else
        {
          *pCode++ = 0x2c;
          *pCode++ = *AdrVals;
          *pCode++ = 1[AdrVals];
        }
        break;
      case ModReg16:
        *pCode++ = 0x05;
        *pCode++ = 0x48 | (AdrVal << 1);
        break;
    }
  }
}

static void DecodeBranch(Word Index)
{
  LongInt Addr;
  Boolean OK;

  if (ChkArgCnt(1, 1))
  {   
    FirstPassUnknown = FALSE;
    Addr = EvalStrIntExpressionOffs(&ArgStr[1], !!(*ArgStr[1].Str == '$'), UInt16, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if ((!FirstPassUnknown) && ((Addr < -128) || (Addr > 127))) WrError(ErrNum_JmpDistTooBig);
      else
      {   
        *pCode++ = Index;
        *pCode++ = Addr & 0xff;
      }
    }
  }
}

static void DecodeDBNZ(Word Index)
{
  LongInt Addr;
  Boolean OK;

  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {   
    DecodeAdr(&ArgStr[1], MModShort | MModReg8);
    switch (AdrMode)
    {
      case ModShort:
        *pCode++ = 0x3b;
        *pCode++ = *AdrVals;
        break;
      case ModReg8:
        if ((AdrVal < 2) || (AdrVal > 3))
        {
          WrError(ErrNum_InvAddrMode);
          AdrMode = ModNone;
        }
        else
          *pCode++ = 0x30 | AdrVal;
        break;
    }
    if (AdrMode != ModNone)
    {
      FirstPassUnknown = FALSE;
      Addr = EvalStrIntExpressionOffs(&ArgStr[2], !!(*ArgStr[2].Str == '$'), UInt16, &OK) - (EProgCounter() + (pCode - BAsmCode) + 1);
      if ((!FirstPassUnknown) && ((Addr < -128) || (Addr > 127)))
      {
        WrError(ErrNum_JmpDistTooBig);
        pCode = BAsmCode;
      }
      else
        *pCode++ = Addr & 0xff;
    }
  }
}

static void DecodeSEL(Word Index)
{
  Boolean OK;
  Byte Bank;

  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (strncasecmp(ArgStr[1].Str, "RB", 2)) WrError(ErrNum_InvAddrMode);
  else
  {
    Bank = EvalStrIntExpressionOffs(&ArgStr[1], 2, UInt2, &OK);
    if (OK)
    {
      *pCode++ = 0x05;
      *pCode++ = 0xa8 | Bank;
    }
  }
}

static void DecodeBIT(Word Index)
{
  LongWord Result;

  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (DecodeBitAdr(&ArgStr[1], &Result))
    EnterIntSymbol(&LabPart, Result, SegNone, False);
}

static void DecodeMOV1(Word Index)
{
  LongWord Bit;
  int ArgPos;

  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    if (!strcasecmp(ArgStr[1].Str, "CY"))
      ArgPos = 2;
    else if (!strcasecmp(ArgStr[2].Str, "CY"))
      ArgPos = 1;
    else
    {
      WrError(ErrNum_InvAddrMode);
      return;
    }
    if (DecodeBitAdr(&ArgStr[ArgPos], &Bit))
    {
      *pCode++ = 0x00 | ((Bit >> 16) & 0xff);
      *pCode++ = ((2 - ArgPos) << 4) | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        *pCode++ = Bit & 0xff;
    }
  }
}

static void DecodeANDOR1(Word Index)
{
  LongWord Bit;

  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "CY")) WrError(ErrNum_InvAddrMode);
  else
  {
    tStrComp *pArg = &ArgStr[2], BitArg;

    if (*pArg->Str == '/')
    {
      StrCompRefRight(&BitArg, &ArgStr[2], 1);
      pArg = &BitArg;
      Index |= 0x10;
    }
    if (DecodeBitAdr(pArg, &Bit))
    {
      *pCode++ = 0x00 | ((Bit >> 16) & 0xff);
      *pCode++ = Index  | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        *pCode++ = Bit & 0xff;
    }
  }
}

static void DecodeXOR1(Word Index)
{
  LongWord Bit;

  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (strcasecmp(ArgStr[1].Str, "CY")) WrError(ErrNum_InvAddrMode);
  else
  {
    if (DecodeBitAdr(&ArgStr[2], &Bit))
    {
      *pCode++ = 0x00 | ((Bit >> 16) & 0xff);
      *pCode++ = 0x60 | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        *pCode++ = Bit & 0xff;
    }
  }
}

static void DecodeBit1(Word Index)
{
  LongWord Bit;

  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (!strcasecmp(ArgStr[1].Str, "CY"))
  {
    *pCode++ = 0x40 | (9 - (Index >> 4));
  }
  else if (DecodeBitAdr(&ArgStr[1], &Bit))
  {
    if ((Index >= 0x80) && ((Bit & 0xfffff800) == 0x01080000))
    {
      *pCode++ = (0x130 - Index) | ((Bit >> 8) & 7);
      *pCode++ = Bit & 0xff;
    }
    else
    {
      *pCode++ = 0x00 | ((Bit >> 16) & 0xff);
      *pCode++ = Index | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        *pCode++ = Bit & 0xff;
    }
  }
}

static void DecodeBrBit(Word Index)
{
  LongWord Bit;
  
  LongInt Addr;
  Boolean OK;

  UNUSED(Index);

  if (ChkArgCnt(2, 2)
   && DecodeBitAdr(&ArgStr[1], &Bit))
  {
    if ((Bit & 0xfffff800) == 0x01080000)
    {
      if (Index == 0x80)
      {
        *pCode++ = 0x70 | ((Bit >> 8) & 7);
	*pCode++ = Bit & 0xff;
      }
      else
      {
        *pCode++ = 0x00 | ((Bit >> 16) & 0xff);
	*pCode++ = (0x130 - Index) | ((Bit >> 8) & 0xff);
	if (Bit & 0x1000000)
          *pCode++ = Bit & 0xff;
      }
    }
    else
    {
      *pCode++ = 0x00 | ((Bit >> 16) & 0xff);
      *pCode++ = (0x130 - Index) | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        *pCode++ = Bit & 0xff;
    }
    if (AdrMode != ModNone)
    {
      FirstPassUnknown = FALSE;
      Addr = EvalStrIntExpressionOffs(&ArgStr[2], !!(*ArgStr[2].Str == '$'), UInt16, &OK) - (EProgCounter() + (pCode - BAsmCode) + 1);
      if ((!FirstPassUnknown) && ((Addr < -128) || (Addr > 127)))
      {
        WrError(ErrNum_JmpDistTooBig);
        pCode = BAsmCode;
      }
      else
        *pCode++ = Addr & 0xff;
    }
  }
}

/*-------------------------------------------------------------------------*/
/* dynamic code table handling */

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(201);

  AddInstTable(InstTable, "MOV", 0, DecodeMOV);
  AddInstTable(InstTable, "XCH", 0, DecodeXCH);
  AddInstTable(InstTable, "MOVW", 0, DecodeMOVW);

  AddInstTable(InstTable, "ADD"  , 0, DecodeALU);
  AddInstTable(InstTable, "ADDC" , 1, DecodeALU);
  AddInstTable(InstTable, "SUB"  , 2, DecodeALU);
  AddInstTable(InstTable, "SUBC" , 3, DecodeALU);
  AddInstTable(InstTable, "AND"  , 4, DecodeALU);
  AddInstTable(InstTable, "OR"   , 6, DecodeALU);
  AddInstTable(InstTable, "XOR"  , 5, DecodeALU);
  AddInstTable(InstTable, "CMP"  , 7, DecodeALU);

  AddInstTable(InstTable, "ADDW", 1, DecodeALU16);
  AddInstTable(InstTable, "SUBW", 2, DecodeALU16);
  AddInstTable(InstTable, "CMPW", 3, DecodeALU16);

  AddInstTable(InstTable, "MULU" , 0x08, DecodeMULDIV);
  AddInstTable(InstTable, "DIVUW", 0x18, DecodeMULDIV);

  AddInstTable(InstTable, "INC", 0, DecodeINCDEC);
  AddInstTable(InstTable, "DEC", 1, DecodeINCDEC);

  AddInstTable(InstTable, "INCW", 0, DecodeINCDECW);
  AddInstTable(InstTable, "DECW", 1, DecodeINCDECW);

  AddInstTable(InstTable, "ROR"  , 0x040, DecodeShift8);
  AddInstTable(InstTable, "ROL"  , 0x140, DecodeShift8);
  AddInstTable(InstTable, "RORC" , 0x000, DecodeShift8);
  AddInstTable(InstTable, "ROLC" , 0x100, DecodeShift8);
  AddInstTable(InstTable, "SHR"  , 0x080, DecodeShift8);
  AddInstTable(InstTable, "SHL"  , 0x180, DecodeShift8);

  AddInstTable(InstTable, "SHRW" , 0x0c0, DecodeShift16);
  AddInstTable(InstTable, "SHLW" , 0x1c0, DecodeShift16);

  AddInstTable(InstTable, "ROR4" , 0, DecodeShift4);
  AddInstTable(InstTable, "ROL4" , 1, DecodeShift4);

  AddInstTable(InstTable, "POP"  , 0, DecodePUSHPOP);
  AddInstTable(InstTable, "PUSH" , 1, DecodePUSHPOP);

  AddInstTable(InstTable, "CALL" , 0, DecodeCALL);
  AddInstTable(InstTable, "CALLF", 0, DecodeCALLF);
  AddInstTable(InstTable, "CALLT", 0, DecodeCALLT);

  AddInstTable(InstTable, "BR"   , 0, DecodeBR);

  AddInstTable(InstTable, "BC"   , 0x83, DecodeBranch);
  AddInstTable(InstTable, "BL"   , 0x83, DecodeBranch);
  AddInstTable(InstTable, "BNC"  , 0x82, DecodeBranch);
  AddInstTable(InstTable, "BNL"  , 0x82, DecodeBranch);
  AddInstTable(InstTable, "BZ"   , 0x81, DecodeBranch);
  AddInstTable(InstTable, "BE"   , 0x81, DecodeBranch);
  AddInstTable(InstTable, "BNZ"  , 0x80, DecodeBranch);
  AddInstTable(InstTable, "BNE"  , 0x80, DecodeBranch);

  AddInstTable(InstTable, "DBNZ" , 0, DecodeDBNZ);

  AddInstTable(InstTable, "SEL", 0, DecodeSEL);

  AddInstTable(InstTable, "MOV1", 0, DecodeMOV1);
  AddInstTable(InstTable, "AND1", 0x20, DecodeANDOR1);
  AddInstTable(InstTable, "OR1" , 0x40, DecodeANDOR1);
  AddInstTable(InstTable, "XOR1" , 0, DecodeXOR1);

  AddInstTable(InstTable, "SET1", 0x80, DecodeBit1);
  AddInstTable(InstTable, "CLR1", 0x90, DecodeBit1);
  AddInstTable(InstTable, "NOT1", 0x70, DecodeBit1);

  AddInstTable(InstTable, "BT"    , 0x80, DecodeBrBit);
  AddInstTable(InstTable, "BF"    , 0x90, DecodeBrBit);
  AddInstTable(InstTable, "BTCLR" , 0x60, DecodeBrBit);

  AddFixed("NOP",   0x00);
  AddFixed("DI",    0x4a);
  AddFixed("EI",    0x4b);
  AddFixed("BRK",   0x5e);
  AddFixed("RET",   0x56);
  AddFixed("RETI",  0x57);
  AddFixed("RETB",  0x5f);
  AddFixed("ADJBA", 0x0e);
  AddFixed("ADJBS", 0x0f);

  AddInstTable(InstTable, "BIT", 0, DecodeBIT);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/
/* interface to common layer */

static void MakeCode_78K2(void)
{
  CodeLen = 0; DontPrint = False; OpSize = -1;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False)) return;

  pCode = BAsmCode;
  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
  else
    CodeLen = pCode - BAsmCode;
}

static void InitCode_78K2(void)
{
  Reg_PM6 = 0;
  Reg_P6  = 0;
}

static Boolean IsDef_78K2(void)
{
  return Memo("BIT");
}

static void SwitchFrom_78K2(void)
{
  DeinitFields();
}

static void SwitchTo_78K2(void)
{
  PFamilyDescr pDescr;

  pDescr = FindFamilyByName("78K2");

  TurnWords = False;
  ConstMode = ConstModeIntel;

  PCSymbol = "PC";
  HeaderID = pDescr->Id;
  NOPCode = 0x00;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xfffff;

  pASSUMERecs = ASSUME78K2s;
  ASSUMERecCnt = sizeof(ASSUME78K2s) / sizeof(ASSUME78K2s[0]);

  MakeCode = MakeCode_78K2; IsDef = IsDef_78K2;
  SwitchFrom = SwitchFrom_78K2; InitFields();
}

void code78k2_init(void)
{
  CPU78214 = AddCPU("78214", SwitchTo_78K2);

  AddInitPassProc(InitCode_78K2);
}
