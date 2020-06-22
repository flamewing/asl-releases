/* code78k3.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 78K3-Familie                                                */
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

#include "code78k3.h"

/*-------------------------------------------------------------------------*/

enum
{
  ModNone = -1,
  ModImm = 0,
  ModReg8 = 1,
  ModReg16 = 2,
  ModMem = 3,
  ModShort = 4,
  ModSFR = 5,
  ModAbs = 6,
  ModShortIndir = 7,
  ModSP = 8,
  ModSTBC = 9,
  ModWDM = 10
};

#define MModImm (1 << ModImm)
#define MModReg8 (1 << ModReg8)
#define MModReg16 (1 << ModReg16)
#define MModMem (1 << ModMem)
#define MModShort (1 << ModShort)
#define MModSFR (1 << ModSFR)
#define MModAbs (1 << ModAbs)
#define MModShortIndir (1 << ModShortIndir)
#define MModSP (1 << ModSP)
#define MModSTBC (1 << ModSTBC)
#define MModWDM (1 << ModWDM)

#define PSWLAddr 0xfffe
#define PSWHAddr 0xffff

/*-------------------------------------------------------------------------*/

static CPUVar CPU78310;

static ShortInt AdrMode, AdrVal;
static Byte AdrVals[3];
static ShortInt OpSize;
static Boolean AssumeByte, ForceLong, ForceRel;
static LongInt Reg_RSS;

static ASSUMERec ASSUME78K3s[] =
{ 
  {"RSS" , &Reg_RSS , 0,  0x1,  0x0, NULL},
};

/*-------------------------------------------------------------------------*/
/* address decoders */

static Byte AccReg8(void)
{
  return Reg_RSS ? 5 : 1;
}

static Byte BReg8(void)
{
  return Reg_RSS ? 7 : 3;
}

static Byte CReg8(void)
{
  return Reg_RSS ? 6 : 2;
}

static Byte AccReg16(void)
{
  return Reg_RSS ? 2 : 0;
}

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

static ShortInt DecodeReg8(const char *pAsc)
{
  ShortInt Result = -1;

  switch (strlen(pAsc))
  {
    case 1:
    {
      static const char Reg8Names[9] = "XACBEDLH";
      char *pPos = strchr(Reg8Names, mytoupper(*pAsc));

      if (pPos)
      {
        Result = pPos - Reg8Names;
        /* E/D/L/H maps to R12..R15 */
        if (Result >= 4)
          Result += 8;
        /* X/A/C/B maps to R4..7 if RSS=1 */
        else if (Reg_RSS)
          Result += 4;
      }

      break;
    }

    case 2:
    {
      if ((toupper(pAsc[0]) == 'R') && isdigit(pAsc[1]))
        Result = pAsc[1] - '0';

      break;
    }

    case 3:
    {
      static const char *Reg8Names[] = { "VPL", "VPH", "UPL", "UPH", "R10", "R11", "R12", "R13", "R14", "R15", NULL };
      int z;

      for (z = 0; Reg8Names[z]; z++)
        if (!as_strcasecmp(pAsc, Reg8Names[z]))
        {
          /* map to 8..11 resp. 10..15 */
          Result = (z > 4) ? z + 6 : z + 8;
          break;
        }
    }
  }

  return Result;
}

static ShortInt DecodeReg16(const char *pAsc)
{
  ShortInt Result = -1;

  switch (strlen(pAsc))
  {
    case  2:
    {
      static char *Reg16Names[] = {"AX", "BC", "VP", "UP", "DE", "HL", NULL};
      int z;

      for (z = 0; Reg16Names[z]; z++)
        if (!as_strcasecmp(Reg16Names[z], pAsc))
        {
          Result = ((z >= 2) || Reg_RSS) ? z + 2 : z;
          break;
        }
      break;
    }

    case 3:
    {
      if ((toupper(pAsc[0]) == 'R') && (toupper(pAsc[1]) == 'P')
       && (pAsc[2] >= '0') && (pAsc[2] <= '7'))
        Result = pAsc[2] - '0';

      break;
    }
  }

  return Result;
}

static ShortInt DecodeIndReg16(const char *pAsc)
{
  int l = strlen(pAsc);
  char Copy[10];

  if ((l < 2) || (l > 5) || (*pAsc != '[') || (pAsc[l -1] != ']'))
    return -1;
  memcpy(Copy, pAsc + 1, l - 2); Copy[l - 2] = '\0';
  l =  DecodeReg16(Copy);
  return l;
}

static ShortInt DecodeRegBank(const char *pAsc)
{
  if ((strlen(pAsc) == 3)
   && (toupper(pAsc[0]) == 'R') && (toupper(pAsc[1]) == 'B')
   && (pAsc[2] >= '0') && (pAsc[2] <= '7'))
    return pAsc[2] - '0';
  else
    return -1;
}

static void ExecAssumeByte(void)
{
  if ((OpSize == -1) && AssumeByte)
  {
    SetOpSize(0);
    AssumeByte = False;
  }
}

static void DecodeAdr(const tStrComp *pArg, Word Mask)
{
  Boolean OK;
  Word WordOp;
  int ArgLen;
  unsigned Offset;

  AdrMode = ModNone; AdrCnt = 0;
  ForceLong = ForceRel = False;

  /* immediate ? */

  if (*pArg->Str == '#')
  {
    ExecAssumeByte();
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

  if (!as_strcasecmp(pArg->Str, "STBC"))
  {
    AdrMode = ModSTBC;
    SetOpSize(0);
    goto AdrFound;
  }

  if (!as_strcasecmp(pArg->Str, "WDM"))
  {
    AdrMode = ModWDM;
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

  if (!as_strcasecmp(pArg->Str, "SP"))
  {
    AdrMode = ModSP;
    SetOpSize(1);
    goto AdrFound;
  }

  /* memory-indirect addressing? */

  ArgLen = strlen(pArg->Str);
  if ((ArgLen >= 2) && (pArg->Str[ArgLen - 1] == ']'))
  {
    tStrComp Arg;
    char *pStart;

    StrCompRefRight(&Arg, pArg, 0);

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
      static char *Modes[] = { "DE+",  "HL+",  "DE-",  "HL-",  "DE",  "HL",  "VP",  "UP",
                               "RP6+", "RP7+", "RP6-", "RP7-", "RP6", "RP7", "RP4", "RP5" };
      unsigned z;
      char *pSep, Save;
      tStrComp Base, Remainder;

      /* skip '[' */

      StrCompIncRefLeft(&Arg, 1);

      /* simple expression without displacement? */

      for (z = 0; z < sizeof(Modes) / sizeof(*Modes); z++)
        if (!as_strcasecmp(Arg.Str, Modes[z]))
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
      if (!as_strcasecmp(Base.Str, "SP"))
        AdrVals[0] = 1;
      else
      {
        int tmp;

        tmp = DecodeReg16(Base.Str);
        switch (tmp)
        {
          case -1: AdrVals[0] = 0xff; break; /* no register */
          case 4: AdrVals[0] = 4; break; /* VP */
          case 5: AdrVals[0] = 3; break; /* UP */
          case 6: AdrVals[0] = 0; break; /* DE */
          case 7: AdrVals[0] = 2; break; /* HL */
          default:
            WrStrErrorPos(ErrNum_InvReg, &Base);
            goto AdrFound;
        }
      }
      *pSep = Save;

      /* no base register detected: purely indirect */

      if (0xff == AdrVals[0])
      {
        FirstPassUnknown = False;
        WordOp = EvalStrIntExpression(&Arg, UInt16, &OK);
        if (OK)
        {
          if (FirstPassUnknown)
            WordOp = 0xfe20;
          if (ChkRange(WordOp, 0xfe20, 0xff1f))
          {
            AdrMode = ModShortIndir;
            AdrVals[0] = Lo(WordOp);
          }
        }
        goto AdrFound;
      }

      /* Now that we have the base, prepare displacement.  May
         be an 8/16-bit register in certain combinations, or a number: */

      if (*pSep == '+')
      {
        int tmp;

        tmp = DecodeReg8(pSep + 1);
        if (tmp == -1); /* no reg at all */
        else if ((tmp == AccReg8())  /* A */
              || (tmp == BReg8())) /* B */
        {
          if (AdrVals[0] == 0) /* DE+A/B */
          {
            AdrMode = ModMem; AdrVal = 0x17;
            AdrCnt = 1; AdrVals[0] = tmp & 2;
            goto AdrFound;
          }
          else if (AdrVals[0] == 2) /* HL+A/B */
          {
            AdrMode = ModMem; AdrVal = 0x17;
            AdrCnt = 1; AdrVals[0] = tmp;
            goto AdrFound;
          }
          else
            WrError(ErrNum_InvAddrMode);
          goto AdrFound;
        }
        else
        {
          WrError(ErrNum_InvAddrMode);
          goto AdrFound;
        }
        tmp = DecodeReg16(pSep + 1);
        switch (tmp)
        {
          case -1: /* no reg at all */
            break;
          case 6: /* DE */
          case 7: /* HL */
            if (AdrVals[0] == 4) /* VP+DE/HL */
            {
              AdrMode = ModMem; AdrVal = 0x17;
              AdrCnt = 1; AdrVals[0] = tmp - 2;
              goto AdrFound;
            }
            /* fall-through */
          default:
            WrError(ErrNum_InvAddrMode);
            goto AdrFound;
        }
      }

      /* it's a number: put a fake 0 in front so displacement expression evaluates correctly! */

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
      int tmp;
      tStrComp Disp, Reg;

      /* split displacement + register */

      StrCompSplitRef(&Disp, &Reg, &Arg, pStart);  

       /* handle base register */

      tmp = DecodeReg8(Reg.Str);
      if ((tmp == AccReg8()) /* A */
       || (tmp == BReg8())) /* B */
      {
        AdrVals[0] = tmp & 3;
      }
      else if (tmp == -1)
      {
        tmp = DecodeReg16(Reg.Str);
        if (tmp >= 6) /* DE/HL */
        {
          AdrVals[0] = (tmp - 6) << 1;
        }
        else
        {
          WrStrErrorPos(ErrNum_InvReg, &Reg);
          goto AdrFound;
        }
      }
      else
      {
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

  /* OK, nothing but absolute left...exclamation mark enforces 16-bit addressing,
     dollar sign relative addressing.  Relative flag is only a hint to the individual
     instruction decoders and not used in this context. */

  Offset = 0;
  if (pArg->Str[Offset] == '!')
  {
    ForceLong = True;
    Offset++;
  }
  else if (pArg->Str[Offset] == '$')
  {
    ForceRel = True;
    Offset++;
  }

  FirstPassUnknown = False;
  WordOp = EvalStrIntExpressionOffs(pArg, Offset, UInt16, &OK);
  if (OK)
  {
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
      AdrVals[0] = Lo(WordOp);
      AdrVals[1] = Hi(WordOp);
    }
  }

AdrFound:

  if ((AdrMode != ModNone) && (!(Mask & (1 << AdrMode))))
  {
    WrError(ErrNum_InvAddrMode);
    AdrMode = ModNone; AdrCnt = 0;
  }
}

static void AppendDisp(const tStrComp *pArg)
{
  Boolean OK;
  LongInt Dist;

  FirstPassUnknown = True;
  Dist = EvalStrIntExpressionOffs(pArg, !!(*pArg->Str == '$'), UInt16, &OK) - (EProgCounter() + CodeLen + 1);
  if ((!FirstPassUnknown) && ((Dist < -128) || (Dist > 127)))
  {
    WrError(ErrNum_JmpDistTooBig);
    CodeLen = 0;
  }
  else
    BAsmCode[CodeLen++] = Dist & 0xff;
}

static Boolean ChkAcc(tStrComp *pArg)
{
  DecodeAdr(pArg, OpSize ? MModReg16 : MModReg8);
  if (AdrMode == ModNone)
    return False;

  if (((OpSize) && (AdrVal != AccReg16()))
   || ((!OpSize) && (AdrVal != AccReg8())))
  {
    WrError(ErrNum_InvAddrMode);
    return False;
  }

  return True;
}

static Boolean DecodeBitAdr(const tStrComp *pArg, LongWord *pResult)
{
  char *pSplit;
  Boolean OK;

  pSplit = RQuotPos(pArg->Str, '.');

  if (pSplit)
  {
    tStrComp RegArg, BitArg;

    StrCompSplitRef(&RegArg, &BitArg, pArg, pSplit);

    *pResult = EvalStrIntExpression(&BitArg, UInt3, &OK) << 8;
    if (OK)
    {
      DecodeAdr(&RegArg, MModReg8 | MModSFR | MModShort);
      switch (AdrMode)
      {
        case ModReg8:
          if ((AdrVal != AccReg8()) && (AdrVal != AccReg8() - 1))
          {
            WrStrErrorPos(ErrNum_InvReg, &RegArg);
            OK = FALSE;
          }
          else
            *pResult |= (((LongWord)(AdrVal & 1)) << 11) | 0x00030000;
          break;
        case ModSFR:
          switch (AdrVals[0])
          {
            case PSWLAddr & 0xff:
              *pResult |= 0x00020000;
              break;
            case PSWHAddr & 0xff:
              *pResult |= 0x00020800;
              break;
            default:
              *pResult |= 0x01080800 | *AdrVals;
              break;
          }
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
/* Instruction Decoders */

static void DecodeFixed(Word Index)
{
  if (ChkArgCnt(0, 0))
    BAsmCode[CodeLen++] = Lo(Index);
}

static void DecodeMOV(Word Is16)
{
  Byte HReg, HCnt;

  if (Is16)
    SetOpSize(1);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg16 | MModSP | MModShort | MModSFR
            | (Is16 ? 0 : (MModReg8 | MModSTBC | MModWDM | MModAbs | MModShortIndir | MModMem)));
    switch (AdrMode)
    {
      case ModReg8:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModImm | MModReg8 | ((HReg == AccReg8()) ? (MModMem | MModAbs | MModShort | MModSFR | MModShortIndir) : 0));
        switch (AdrMode)
        {
          case ModImm:
            if (HReg >= 8) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0xb8 | HReg;
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            break;
          case ModReg8:
            if (AdrVal >= 8) WrError(ErrNum_InvAddrMode);
            else if (HReg == AccReg8())
              BAsmCode[CodeLen++] = 0xd0 | AdrVal;
            else
            {
              BAsmCode[CodeLen++] = 0x24;
              BAsmCode[CodeLen++] = (HReg << 4) | AdrVal;
            }
            break;
          case ModMem:
            if ((AdrVal == 0x16) && (AdrVals[0] <= 5))
              BAsmCode[CodeLen++] = 0x58 | AdrVals[0];
            else
            {
              BAsmCode[CodeLen++] = 0x00 | AdrVal;
              BAsmCode[CodeLen++] = 0x00 | (AdrVals[0] << 4);
              memcpy(BAsmCode + CodeLen, AdrVals + 1, AdrCnt - 1);
              CodeLen += AdrCnt - 1;
            }
            break;
          case ModAbs:
            BAsmCode[CodeLen++] = 0x09;
            BAsmCode[CodeLen++] = 0xf0;
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = AdrVals[1];
            break;
          case ModShort:
            BAsmCode[CodeLen++] = 0x20;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModSFR:
            BAsmCode[CodeLen++] = 0x10;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModShortIndir:
            BAsmCode[CodeLen++] = 0x18;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModSTBC:
        DecodeAdr(&ArgStr[2], MModImm);
        switch (AdrMode)
        {
          case ModImm:
            BAsmCode[CodeLen++] = 0x09;
            BAsmCode[CodeLen++] = 0x44;
            BAsmCode[CodeLen++] = 0xff - AdrVals[0];
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModWDM:
        DecodeAdr(&ArgStr[2], MModImm);
        switch (AdrMode)
        {
          case ModImm:
            BAsmCode[CodeLen++] = 0x09;
            BAsmCode[CodeLen++] = 0x42;
            BAsmCode[CodeLen++] = 0xff - AdrVals[0];
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModReg16:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg16 | MModImm | ((HReg == AccReg16()) ? (MModShort | MModSFR | MModSP) : 0));
        switch (AdrMode)
        {
          case ModReg16:
            BAsmCode[CodeLen++] = 0x24;
            BAsmCode[CodeLen++] = 0x08 | (HReg << 5) | AdrVal;
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0x60 | HReg;
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = AdrVals[1];
            break;
          case ModShort:
            BAsmCode[CodeLen++] = 0x1c;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModSFR:
            BAsmCode[CodeLen++] = 0x11;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModSP:
            BAsmCode[CodeLen++] = 0x11;
            BAsmCode[CodeLen++] = 0xfc;
            break;
        }
        break;
      case ModSP:
        DecodeAdr(&ArgStr[2], MModReg16 | MModImm);
        switch (AdrMode)
        {
          case ModReg16:
            if (AdrVal != AccReg16()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x13;
              BAsmCode[CodeLen++] = 0xfc;
            }
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0x0b;
            BAsmCode[CodeLen++] = 0xfc;
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = AdrVals[1];
            break;
        }
        break;
      case ModShort:
        AssumeByte = (OpSize == -1);
        HReg = AdrVals[0];
        DecodeAdr(&ArgStr[2], MModImm | MModShort | MModReg8 | MModReg16);
        switch (AdrMode)
        {
          case ModImm:
            BAsmCode[CodeLen++] = OpSize ? 0x0c : 0x3a;
            BAsmCode[CodeLen++] = HReg;
            BAsmCode[CodeLen++] = AdrVals[0];
            if (OpSize)
              BAsmCode[CodeLen++] = AdrVals[1];
            break;
          case ModShort:
            ExecAssumeByte();
            BAsmCode[CodeLen++] = (OpSize == 1) ? 0x3c : 0x38;
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = HReg;
            break;
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x22;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
          case ModReg16:
            if (AdrVal != AccReg16()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x1a;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
        }
        break;
      case ModSFR:
        AssumeByte = (OpSize == -1);
        HReg = AdrVals[0];
        DecodeAdr(&ArgStr[2], MModImm | MModReg8 | MModReg16);
        switch (AdrMode)
        {
          case ModImm:
            BAsmCode[CodeLen++] = OpSize ? 0x0b : 0x2b;
            BAsmCode[CodeLen++] = HReg;
            BAsmCode[CodeLen++] = AdrVals[0];
            if (OpSize)
              BAsmCode[CodeLen++] = AdrVals[1];
            break;
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x12;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
          case ModReg16:
            if (AdrVal != AccReg16()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x13;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
        }
        break;
      case ModMem:
        HReg = AdrVal;
        HCnt = AdrCnt;
        SetOpSize(0);
        DecodeAdr(&ArgStr[2], MModImm | MModReg8);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else if ((HReg == 0x16) && (AdrVals[0] <= 5))
              BAsmCode[CodeLen++] = 0x50 | AdrVals[0];
            else
            {
              BAsmCode[CodeLen++] = 0x00 | HReg;
              BAsmCode[CodeLen++] = 0x80 | (AdrVals[0] << 4);
              memcpy(BAsmCode + CodeLen, AdrVals + 1, HCnt - 1);
              CodeLen += HCnt - 1;
            }
            break;
        }
        break;
      case ModAbs:
        DecodeAdr(&ArgStr[2], MModReg8);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x09;
              BAsmCode[CodeLen++] = 0xf1;
              BAsmCode[CodeLen++] = AdrVals[0];
              BAsmCode[CodeLen++] = AdrVals[1];
            }
            break;
        }
        break;
      case ModShortIndir:
        DecodeAdr(&ArgStr[2], MModReg8);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x19;
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            break;
        }
        break;
    }
  }
}

static void DecodeXCH(Word Is16)
{
  Byte HReg, HCnt;

  if (Is16)
    SetOpSize(1);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg16 | MModShort | MModSFR
            | (Is16 ? 0 : (MModReg8 | MModShortIndir | MModMem)));
    switch (AdrMode)
    {
      case ModReg8:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg8 | ((HReg == AccReg8()) ? (MModMem | MModShort | MModShortIndir | MModSFR) : 0));
        switch (AdrMode)
        {
          case ModReg8:
            if ((HReg == AccReg8()) && (AdrVal < 8))
              BAsmCode[CodeLen++] = 0xd8 | AdrVal;
            else if ((AdrVal == AccReg8()) && (HReg < 8))
              BAsmCode[CodeLen++] = 0xd8 | HReg;
            else if (HReg < 8)
            {
              BAsmCode[CodeLen++] = 0x25;
              BAsmCode[CodeLen++] = 0x00 | (AdrVal << 4) | HReg;
            }
            else if (AdrVal < 8)
            {
              BAsmCode[CodeLen++] = 0x25;
              BAsmCode[CodeLen++] = 0x00 | (HReg << 4) | AdrVal;
            }
            else
              WrError(ErrNum_InvRegPair);
            break;
          case ModMem:
            BAsmCode[CodeLen++] = 0x00 | AdrVal;
            BAsmCode[CodeLen++] = 0x04 | (AdrVals[0] << 4);
            memcpy(BAsmCode + CodeLen, AdrVals + 1, AdrCnt - 1);
            CodeLen += AdrCnt - 1;
            break;
          case ModShort:
            BAsmCode[CodeLen++] = 0x21;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModSFR:
            BAsmCode[CodeLen++] = 0x01;
            BAsmCode[CodeLen++] = 0x21;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModShortIndir:
            BAsmCode[CodeLen++] = 0x23;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModReg16:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg16 | ((HReg == AccReg16()) ? (MModShort | MModSFR) : 0));
        switch (AdrMode)
        {
          case ModReg16:
            BAsmCode[CodeLen++] = 0x25;
            BAsmCode[CodeLen++] = 0x08 | (HReg << 5) | AdrVal;
            break;
          case ModShort:
            BAsmCode[CodeLen++] = 0x1b;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModSFR:
            BAsmCode[CodeLen++] = 0x01;
            BAsmCode[CodeLen++] = 0x1b;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModShort:
        HReg = AdrVals[0];
        DecodeAdr(&ArgStr[2], MModReg8 | MModReg16 | MModShort);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x21;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
          case ModReg16:
            if (AdrVal != AccReg16()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x1b;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
          case ModShort:
            BAsmCode[CodeLen++] = (OpSize == 1) ? 0x2a : 0x39;
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = HReg;
            break;
        }
        break;
      case ModSFR:
        HReg = AdrVals[0];
        DecodeAdr(&ArgStr[2], MModReg8 | MModReg16);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x01;
              BAsmCode[CodeLen++] = 0x21;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
          case ModReg16:
            if (AdrVal != AccReg16()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x01;
              BAsmCode[CodeLen++] = 0x1b;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
        }
        break;
      case ModMem:
        HReg = AdrVal;
        HCnt = AdrCnt;
        DecodeAdr(&ArgStr[2], MModReg8);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x00 | HReg;
              BAsmCode[CodeLen++] = 0x04 | (AdrVals[0] << 4);
              memcpy(BAsmCode + CodeLen, AdrVals + 1, HCnt - 1);
              CodeLen += HCnt - 1;
            }
            break;
        }
        break;
      case ModShortIndir:
        HReg = AdrVals[0];
        DecodeAdr(&ArgStr[2], MModReg8);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x23;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
        }
        break;
    }
  }
}

static void DecodeALU(Word Props)
{
  Byte HReg, HCnt;
  const Byte Code8 = (Props >> 0) & 15,
             Code16 = (Props >> 4) & 15,
             Code16Reg = (Props >> 8) & 15;
  const Boolean Is16 = (Props & 0x8000) || False,
                May16 = (Props & 0x4000) || False;

  if (Is16)
    SetOpSize(1);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1],
              MModShort | MModSFR
            | (May16 ? MModReg16 : 0)
            | (Is16 ? 0 : (MModReg8 | MModMem)));
    switch (AdrMode)
    {
      case ModReg8:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg8 | ((HReg == AccReg8()) ? (MModShort | MModSFR | MModMem | MModImm) : 0));
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal >= 8) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x88 | Code8;
              BAsmCode[CodeLen++] = (HReg << 4) | AdrVal;
            }
            break;
          case ModShort:
            BAsmCode[CodeLen++] = 0x98 | Code8;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModSFR:
            BAsmCode[CodeLen++] = 0x01;
            BAsmCode[CodeLen++] = 0x98 | Code8;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModMem:
            BAsmCode[CodeLen++] = 0x00 | AdrVal;
            BAsmCode[CodeLen++] = 0x08 | Code8 | (AdrVals[0] << 4);
            memcpy(BAsmCode + CodeLen, AdrVals + 1, AdrCnt - 1);
            CodeLen += AdrCnt - 1;
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0xa8 | Code8;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModReg16:
        HReg = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg16 | ((HReg == AccReg16()) ? (MModShort | MModImm | MModSFR) : 0));
        switch (AdrMode)
        {
          case ModReg16:
            BAsmCode[CodeLen++] = 0x88 | Code16Reg;
            BAsmCode[CodeLen++] = 0x08 | (HReg << 5) | AdrVal;
            break;
          case ModShort:
            BAsmCode[CodeLen++] = 0x10 | Code16;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModSFR:
            BAsmCode[CodeLen++] = 0x01;
            BAsmCode[CodeLen++] = 0x1d | Code16;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0x20 | Code16;
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModShort:
        HReg = AdrVals[0];
        AssumeByte = (OpSize == -1);
        DecodeAdr(&ArgStr[2], MModShort | MModImm);
        switch (AdrMode)
        {
          case ModShort:
            BAsmCode[CodeLen++] = OpSize ? (0x30 | Code16) : (0x78 | Code8);
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = HReg;
            break;
          case ModImm:
            BAsmCode[CodeLen++] = OpSize ? (0x00 | Code16) : (0x68 | Code8);
            BAsmCode[CodeLen++] = HReg;
            BAsmCode[CodeLen++] = AdrVals[0];
            if (OpSize)
              BAsmCode[CodeLen++] = AdrVals[1];
            break;
        }
        break;
      case ModSFR:
        HReg = AdrVals[0];
        AssumeByte = (OpSize == -1);
        DecodeAdr(&ArgStr[2], MModImm);
        switch (AdrMode)
        {
          case ModImm:
            BAsmCode[CodeLen++] = 0x01;
            BAsmCode[CodeLen++] = OpSize ? (0x00 | Code16) : (0x68 | Code8);
            BAsmCode[CodeLen++] = HReg;
            BAsmCode[CodeLen++] = AdrVals[0];
            if (OpSize)
              BAsmCode[CodeLen++] = AdrVals[1];
            break;
        }
        break;
      case ModMem:
        HReg = AdrVal;
        HCnt = AdrCnt;
        DecodeAdr(&ArgStr[2], MModReg8);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrVal != AccReg8()) WrError(ErrNum_InvAddrMode);
            else
            {
              BAsmCode[CodeLen++] = 0x00 | HReg;
              BAsmCode[CodeLen++] = 0x88 | Code8 | (AdrVals[0] << 4);
              memcpy(BAsmCode + CodeLen, AdrVals + 1, HCnt - 1);
              CodeLen += HCnt - 1;
            }
            break;
        }
        break;
    }
  }
}

static void DecodeMULDIV(Word Props)
{
  if (ChkArgCnt(1, 1))
  {
    if (Props & 0x8000)
      SetOpSize(1);
    DecodeAdr(&ArgStr[1], MModReg16 | ((OpSize == 1) ? 0 : MModReg8));
    switch (AdrMode)
    {
      case ModReg8:
        if (AdrVal > 7) WrError(ErrNum_InvAddrMode);
        else
        {
          BAsmCode[CodeLen++] = 0x05;
          BAsmCode[CodeLen++] = 0x08 | ((Props << 4) & 0xf0) | AdrVal;
        }
        break;
      case ModReg16:
        BAsmCode[CodeLen++] = 0x05;
        BAsmCode[CodeLen++] = 0x08 | (Props & 0xf0) | AdrVal;
        break;
    }
  }
}

static void DecodeINCDEC(Word Props)
{
  const Byte Code = Props & 1;

  if (ChkArgCnt(1, 1))
  {
    if (Props & 2)
      SetOpSize(1);
    DecodeAdr(&ArgStr[1], MModShort | MModReg16 | MModSP | ((OpSize == 1) ? 0 : MModReg8));
    switch (AdrMode)
    {
      case ModReg8:
        if (AdrVal > 7) WrError(ErrNum_InvAddrMode);
        else
          BAsmCode[CodeLen++] = 0xc0 | AdrVal | (Code << 3);
        break;
      case ModReg16:
        if (AdrVal < 4) WrError(ErrNum_InvAddrMode);
        else
          BAsmCode[CodeLen++] = 0x40 | AdrVal | (Code << 3);
        break;
      case ModSP:
        BAsmCode[CodeLen++] = 0x05;
        BAsmCode[CodeLen++] = 0xc8 | Code;
        break;
      case ModShort:
        if (OpSize == 1)
        {
          BAsmCode[CodeLen++] = 0x07;
          BAsmCode[CodeLen++] = 0xe8 | Code;
        }
        else
          BAsmCode[CodeLen++] = 0x26 | Code;
        BAsmCode[CodeLen++] = AdrVals[0];
        break;
    }
  }
}

static void DecodeShift(Word Props)
{
  Boolean OK;
  Byte Shift;

  if (!ChkArgCnt(2, 2))
    return;

  Shift = EvalStrIntExpression(&ArgStr[2], UInt3, &OK);
  if (!OK)
    return;

  if (Props & 0x8000)
    SetOpSize(1);
  DecodeAdr(&ArgStr[1], ((OpSize != 1) ? MModReg8 : 0) | ((Props & 0x4000) ? MModReg16 : 0));
  switch (AdrMode)
  {
    case ModReg8:
      if (AdrVal > 7) WrError(ErrNum_InvAddrMode);
      else
      {
        BAsmCode[CodeLen++] = 0x30 | (Props & 1);
        BAsmCode[CodeLen++] = (Props & 0xf0) | AdrVal | (Shift << 3);
      }
      break;
    case ModReg16:
      BAsmCode[CodeLen++] = 0x30 | (Props & 1);
      BAsmCode[CodeLen++] = ((Props & 0xf00) >> 4) | AdrVal | (Shift << 3);
      break;
  }
}

static void DecodeROLROR4(Word Code)
{
  ShortInt Reg;

  if (!ChkArgCnt(1, 1));
  else if ((Reg = DecodeIndReg16(ArgStr[1].Str)) < 0) WrError(ErrNum_InvAddrMode);
  else
  {
    BAsmCode[CodeLen++] = 0x05;
    BAsmCode[CodeLen++] = Code | Reg;
  }
}

static void DecodeBIT(Word Index)
{
  LongWord Result;

  UNUSED(Index);

  if (ChkArgCnt(1, 1)
   && DecodeBitAdr(&ArgStr[1], &Result))
    EnterIntSymbol(&LabPart, Result, SegNone, False);
}

static void DecodeMOV1(Word Index)
{
  LongWord Bit;
  int ArgPos;

  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    if (!as_strcasecmp(ArgStr[1].Str, "CY"))
      ArgPos = 2;
    else if (!as_strcasecmp(ArgStr[2].Str, "CY"))
      ArgPos = 1;
    else
    {
      WrError(ErrNum_InvAddrMode);
      return;
    }
    if (DecodeBitAdr(&ArgStr[ArgPos], &Bit))
    {
      BAsmCode[CodeLen++] = 0x00 | ((Bit >> 16) & 0xff);
      BAsmCode[CodeLen++] = ((2 - ArgPos) << 4) | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        BAsmCode[CodeLen++] = Bit & 0xff;
    }
  }
}

static void DecodeANDOR1(Word Index)
{
  LongWord Bit;

  if (!ChkArgCnt(2, 2));
  else if (as_strcasecmp(ArgStr[1].Str, "CY")) WrError(ErrNum_InvAddrMode);
  else
  {
    tStrComp *pArg, BitArg;

    pArg = &ArgStr[2];
    if (*pArg->Str == '/')
    {
      StrCompRefRight(&BitArg, pArg, 1);
      pArg = &BitArg;
      Index |= 0x10;
    }
    if (DecodeBitAdr(pArg, &Bit))
    {
      BAsmCode[CodeLen++] = 0x00 | ((Bit >> 16) & 0xff);
      BAsmCode[CodeLen++] = Index  | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        BAsmCode[CodeLen++] = Bit & 0xff;
    }
  }
}

static void DecodeXOR1(Word Index)
{
  LongWord Bit;

  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (as_strcasecmp(ArgStr[1].Str, "CY")) WrError(ErrNum_InvAddrMode);
  else
  {
    if (DecodeBitAdr(&ArgStr[2], &Bit))
    {
      BAsmCode[CodeLen++] = 0x00 | ((Bit >> 16) & 0xff);
      BAsmCode[CodeLen++] = 0x60 | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        BAsmCode[CodeLen++] = Bit & 0xff;
    }
  }
}

static void DecodeBit1(Word Index)
{
  LongWord Bit;

  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (!as_strcasecmp(ArgStr[1].Str, "CY"))
  {
    BAsmCode[CodeLen++] = 0x40 | (9 - (Index >> 4));
  }
  else if (DecodeBitAdr(&ArgStr[1], &Bit))
  {
    if ((Index >= 0x80) && ((Bit & 0xfffff800) == 0x01080000))
    {
      BAsmCode[CodeLen++] = (0x130 - Index) | ((Bit >> 8) & 7);
      BAsmCode[CodeLen++] = Bit & 0xff;
    }
    else
    {
      BAsmCode[CodeLen++] = 0x00 | ((Bit >> 16) & 0xff);
      BAsmCode[CodeLen++] = Index | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        BAsmCode[CodeLen++] = Bit & 0xff;
    }
  }
}

static void DecodeCALL(Word Code)
{
  ShortInt Reg;

  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if ((Reg = DecodeIndReg16(ArgStr[1].Str)) >= 0)
  {
    BAsmCode[CodeLen++] = 0x05;
    BAsmCode[CodeLen++] = 0x78 | Reg;
  }
  else
  {
    DecodeAdr(&ArgStr[1], MModAbs | MModReg16);
    switch (AdrMode)
    {
      case ModAbs:
        BAsmCode[CodeLen++] = 0x28;
        BAsmCode[CodeLen++] = AdrVals[0];
        BAsmCode[CodeLen++] = AdrVals[1];
        break;
      case ModReg16:
        BAsmCode[CodeLen++] = 0x05;
        BAsmCode[CodeLen++] = 0x58 | AdrVal;
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
        BAsmCode[CodeLen++] = 0x90 | (Hi(AdrWord) & 7);
        BAsmCode[CodeLen++] = Lo(AdrWord);
      }
    }
  }
}

static void DecodeCALLT(Word Index)
{
  Word AdrWord;
  Boolean OK;
  int l;

  UNUSED(Index);

  if (ChkArgCnt(1, 1))
  {
    l = strlen(ArgStr[1].Str);
    if ((*ArgStr[1].Str != '[') || (ArgStr[1].Str[l - 1] != ']')) WrError(ErrNum_InvAddrMode);
    else
    {
      tStrComp Arg;

      StrCompRefRight(&Arg, &ArgStr[1], 1);
      StrCompShorten(&Arg, 1);
      FirstPassUnknown = FALSE;
      AdrWord = EvalStrIntExpression(&Arg, UInt7, &OK);
      if (OK)
      {
        if (FirstPassUnknown)
        AdrWord = 0x40;
        if (ChkRange(AdrWord, 0x40, 0x7e))
        {
          if (AdrWord & 1) WrError(ErrNum_AddrMustBeEven);
          else
          {
            BAsmCode[CodeLen++] = 0xe0 | ((AdrWord - 0x40) >> 1);
          }
        }
      }
    }
  }
}

static void DecodePUSHPOP(Word Code)
{
  Boolean IsU = (Code & 2) || False;
  ShortInt Reg;

  if (ChkArgCnt(1, ArgCntMax))
  {
    Word Mask = 0, ThisMask;
    int z;

    for (z = 1; z <= ArgCnt; z++)
    {
      /* special case, either bit 5 or separate instruction */

      if (!as_strcasecmp(ArgStr[z].Str, "PSW"))
        ThisMask = IsU ? 0x20 : 0x100;

      /* user stack ptr itself cannot be pushed onto user stack */

      else if (((Reg = DecodeReg16(ArgStr[z].Str)) < 0)
            || (IsU && (Reg == 5)))
      {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[z]);
        return;
      }
      else
        ThisMask = 1 << Reg;

      /* register already named? */

      if (Mask & ThisMask)
      {
        WrStrErrorPos(ErrNum_InvRegList, &ArgStr[z]);
        return;
      }
      else
        Mask |= ThisMask;
    }

    /* cannot mix separate PSW and other registers in one instruction */

    if (Lo(Mask) && Hi(Mask)) WrError(ErrNum_InvRegList);
    else if (Hi(Mask))
      BAsmCode[CodeLen++] = Hi(Code);
    else
    {
      BAsmCode[CodeLen++] = Lo(Code);
      BAsmCode[CodeLen++] = Lo(Mask);
    }
  }
}

static void DecodeBR(Word Code)
{
  ShortInt Reg;

  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if ((Reg = DecodeIndReg16(ArgStr[1].Str)) >= 0)
  {
    BAsmCode[CodeLen++] = 0x05;
    BAsmCode[CodeLen++] = 0x68 | Reg;
  }
  else
  {
    DecodeAdr(&ArgStr[1], MModAbs | MModReg16);
    switch (AdrMode)
    {
      case ModReg16:
        BAsmCode[CodeLen++] = 0x05;
        BAsmCode[CodeLen++] = 0x48 | AdrVal;
        break;
      case ModAbs:
      {
        Word AbsAddr = (((Word)AdrVals[1]) << 8) | AdrVals[0];
        Integer Dist = AbsAddr - (EProgCounter() + 2);
        Boolean DistOK = (Dist >= -128) && (Dist < 127);

        if (ForceRel && !DistOK) WrError(ErrNum_JmpDistTooBig);
        else if (ForceLong || !DistOK)
        {
          BAsmCode[CodeLen++] = 0x14;
          BAsmCode[CodeLen++] = AdrVals[0];
          BAsmCode[CodeLen++] = AdrVals[1];
        }
        else
        {
          BAsmCode[CodeLen++] = 0x14;
          BAsmCode[CodeLen++] = Dist & 0xff;
        }
        break;
      }
    }
  }
}

static void DecodeBranch(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    if (Hi(Code))
      BAsmCode[CodeLen++] = Hi(Code);
    BAsmCode[CodeLen++] = Lo(Code);

    AppendDisp(&ArgStr[1]);
  }
}

static void DecodeBrBit(Word Index)
{
  LongWord Bit;

  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (DecodeBitAdr(&ArgStr[1], &Bit))
  {
    if ((Bit & 0xfffff800) == 0x01080000)
    {
      if (Index == 0x80)
      {
        BAsmCode[CodeLen++] = 0x70 | ((Bit >> 8) & 7);
	BAsmCode[CodeLen++] = Bit & 0xff;
      }
      else
      {
        BAsmCode[CodeLen++] = 0x00 | ((Bit >> 16) & 0xff);
	BAsmCode[CodeLen++] = (0x130 - Index) | ((Bit >> 8) & 0xff);
	if (Bit & 0x1000000)
          BAsmCode[CodeLen++] = Bit & 0xff;
      }
    }
    else
    {
      BAsmCode[CodeLen++] = 0x00 | ((Bit >> 16) & 0xff);
      BAsmCode[CodeLen++] = (0x130 - Index) | ((Bit >> 8) & 0xff);
      if (Bit & 0x1000000)
        BAsmCode[CodeLen++] = Bit & 0xff;
    }

    if (AdrMode != ModNone)
      AppendDisp(&ArgStr[2]);
  }
}

static void DecodeDBNZ(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModReg8 | MModShort);
    switch (AdrMode)
    {
      case ModReg8:
        if ((AdrVal != CReg8()) && (AdrVal != BReg8())) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
        else
          BAsmCode[CodeLen++] = 0x30 | (AdrVal & 3);
        break;
      case ModShort:
        BAsmCode[CodeLen++] = 0x3b;
        BAsmCode[CodeLen++] = AdrVals[0];
        break;
      default:
        return;
    }

    AppendDisp(&ArgStr[2]);
  }
}

static void DecodeBRKCS(Word Code)
{
  ShortInt Bank;

  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if ((Bank = DecodeRegBank(ArgStr[1].Str)) < 0) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    BAsmCode[CodeLen++] = 0x05;
    BAsmCode[CodeLen++] = 0xd8 | Bank;
  }
}

static void DecodeRETCS(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModAbs);
    switch (AdrMode)
    {
      case ModAbs:
        BAsmCode[CodeLen++] = 0x29;
        BAsmCode[CodeLen++] = AdrVals[0];
        BAsmCode[CodeLen++] = AdrVals[1];
        break;
    }
  }
}

static void DecodeString1(Word Code)
{
  UNUSED(Code);

  SetOpSize(0);
  if (!ChkArgCnt(2, 2));
  else if (ChkAcc(&ArgStr[2]))
  {
    DecodeAdr(&ArgStr[1], MModMem);
    switch (AdrMode)
    {
      case ModMem:
        if ((AdrVal != 0x16) || (AdrVals[0] & 13)) WrError(ErrNum_InvAddrMode);
        else
        {
          BAsmCode[CodeLen++] = 0x15;
          BAsmCode[CodeLen++] = Code | ((AdrVals[0] & 2) << 3);
        }
        break;
    }
  }
}

static void DecodeString2(Word Code)
{
  Byte HReg;

  UNUSED(Code);

  if (!ChkArgCnt(2, 2))
    return;

  DecodeAdr(&ArgStr[1], MModMem);
  if (AdrMode != ModMem)
    return;
  if ((AdrVal != 0x16) || (AdrVals[0] & 5)) /* [DE-] or [DE+] */
  {
    WrError(ErrNum_InvAddrMode);
    return;
  }
  HReg = AdrVals[0];

  DecodeAdr(&ArgStr[2], MModMem);
  if (AdrMode != ModMem)
    return;
  if ((AdrVal != 0x16)
   || ((AdrVals[0] & 5) != 1) /* [HL-] or [HL+] */
   || ((HReg ^ AdrVals[0]) & 2)) /* match [DE+] with [HL+] and [DE-] with [HL-] */
  {
    WrError(ErrNum_InvAddrMode);
    return;
  }

  BAsmCode[CodeLen++] = 0x15;
  BAsmCode[CodeLen++] = Code | ((AdrVals[0] & 2) << 3);
}

static void DecodeSEL(Word Code)
{
  ShortInt Bank = 0;

  UNUSED(Code);

  if (!ChkArgCnt(1, 2));
  else if ((Bank = DecodeRegBank(ArgStr[1].Str)) < 0) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else if ((ArgCnt == 2) && (as_strcasecmp(ArgStr[2].Str, "ALT"))) WrStrErrorPos(ErrNum_InvReg, &ArgStr[2]);
  {
    BAsmCode[CodeLen++] = 0x05;
    BAsmCode[CodeLen++] = 0xa8 | Bank | ((ArgCnt - 1) << 4);
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

  AddInstTable(InstTable, "MOV",  0, DecodeMOV);
  AddInstTable(InstTable, "MOVW", 1, DecodeMOV);
  AddInstTable(InstTable, "XCH",  0, DecodeXCH);
  AddInstTable(InstTable, "XCHW", 1, DecodeXCH);

  AddInstTable(InstTable, "ADD",  0x40d0, DecodeALU);
  AddInstTable(InstTable, "ADDW", 0xc0d0, DecodeALU);
  AddInstTable(InstTable, "ADDC", 0x0001, DecodeALU);
  AddInstTable(InstTable, "SUB",  0x42e2, DecodeALU);
  AddInstTable(InstTable, "SUBW", 0xc2e2, DecodeALU);
  AddInstTable(InstTable, "SUBC", 0x0003, DecodeALU);
  AddInstTable(InstTable, "AND",  0x0004, DecodeALU);
  AddInstTable(InstTable, "OR",   0x0006, DecodeALU);
  AddInstTable(InstTable, "XOR",  0x0005, DecodeALU);
  AddInstTable(InstTable, "CMP",  0x47f7, DecodeALU);
  AddInstTable(InstTable, "CMPW", 0xc7f7, DecodeALU);

  AddInstTable(InstTable, "MULU",  0x0020, DecodeMULDIV);
  AddInstTable(InstTable, "DIVU",  0x00e1, DecodeMULDIV);
  AddInstTable(InstTable, "MULUW", 0x8020, DecodeMULDIV);
  AddInstTable(InstTable, "DIVUX", 0x80e1, DecodeMULDIV);

  AddInstTable(InstTable, "INC",   0, DecodeINCDEC);
  AddInstTable(InstTable, "INCW",  2, DecodeINCDEC);
  AddInstTable(InstTable, "DEC",   1, DecodeINCDEC);
  AddInstTable(InstTable, "DECW",  3, DecodeINCDEC);

  AddInstTable(InstTable, "ROR",   0x0040, DecodeShift);
  AddInstTable(InstTable, "ROL",   0x0041, DecodeShift);
  AddInstTable(InstTable, "RORC",  0x0000, DecodeShift);
  AddInstTable(InstTable, "ROLC",  0x0001, DecodeShift);
  AddInstTable(InstTable, "SHR",   0x4c80, DecodeShift);
  AddInstTable(InstTable, "SHL",   0x4c81, DecodeShift);
  AddInstTable(InstTable, "SHRW",  0xcc80, DecodeShift);
  AddInstTable(InstTable, "SHLW",  0xcc81, DecodeShift);

  AddInstTable(InstTable, "ROL4",  0x88, DecodeROLROR4);
  AddInstTable(InstTable, "ROR4",  0x98, DecodeROLROR4);
  AddFixed("ADJ4", 0x0004);

  AddInstTable(InstTable, "MOV1", 0, DecodeMOV1);
  AddInstTable(InstTable, "AND1", 0x20, DecodeANDOR1);
  AddInstTable(InstTable, "OR1" , 0x40, DecodeANDOR1);
  AddInstTable(InstTable, "XOR1" , 0, DecodeXOR1);

  AddInstTable(InstTable, "SET1", 0x80, DecodeBit1);
  AddInstTable(InstTable, "CLR1", 0x90, DecodeBit1);
  AddInstTable(InstTable, "NOT1", 0x70, DecodeBit1);

  AddInstTable(InstTable, "CALL", 0, DecodeCALL);
  AddInstTable(InstTable, "CALLF", 0, DecodeCALLF);
  AddInstTable(InstTable, "CALLT", 0, DecodeCALLT);
  AddFixed("BRK", 0x005e);
  AddFixed("RET", 0x0056);
  AddFixed("RETI", 0x0057);

  AddInstTable(InstTable, "PUSH",  0x4935, DecodePUSHPOP);
  AddInstTable(InstTable, "PUSHU", 0x0037, DecodePUSHPOP);
  AddInstTable(InstTable, "POP",   0x4834, DecodePUSHPOP);
  AddInstTable(InstTable, "POPU",  0x0036, DecodePUSHPOP);

  AddInstTable(InstTable, "BR", 0, DecodeBR);

  AddInstTable(InstTable, "BC",   0x83, DecodeBranch);
  AddInstTable(InstTable, "BL",   0x83, DecodeBranch);
  AddInstTable(InstTable, "BNC",  0x82, DecodeBranch);
  AddInstTable(InstTable, "BNL",  0x82, DecodeBranch);
  AddInstTable(InstTable, "BZ",   0x81, DecodeBranch);
  AddInstTable(InstTable, "BE",   0x81, DecodeBranch);
  AddInstTable(InstTable, "BNZ",  0x80, DecodeBranch);
  AddInstTable(InstTable, "BNE",  0x80, DecodeBranch);
  AddInstTable(InstTable, "BV",   0x85, DecodeBranch);
  AddInstTable(InstTable, "BPE",  0x85, DecodeBranch);
  AddInstTable(InstTable, "BNV",  0x84, DecodeBranch);
  AddInstTable(InstTable, "BPO",  0x84, DecodeBranch);
  AddInstTable(InstTable, "BN",   0x87, DecodeBranch);
  AddInstTable(InstTable, "BP",   0x86, DecodeBranch);
  AddInstTable(InstTable, "BGT",0x07fb, DecodeBranch);
  AddInstTable(InstTable, "BGE",0x07f9, DecodeBranch);
  AddInstTable(InstTable, "BLT",0x07f8, DecodeBranch);
  AddInstTable(InstTable, "BLE",0x07fa, DecodeBranch);
  AddInstTable(InstTable, "BH", 0x07fd, DecodeBranch);
  AddInstTable(InstTable, "BNH",0x07fc, DecodeBranch);

  AddInstTable(InstTable, "BT"    , 0x80, DecodeBrBit);
  AddInstTable(InstTable, "BF"    , 0x90, DecodeBrBit);
  AddInstTable(InstTable, "BTCLR" , 0x60, DecodeBrBit);
  AddInstTable(InstTable, "BFSET" , 0x70, DecodeBrBit);

  AddInstTable(InstTable, "DBNZ", 0, DecodeDBNZ);

  AddInstTable(InstTable, "BRKCS", 0, DecodeBRKCS);
  AddInstTable(InstTable, "RETCS", 0, DecodeRETCS);

  AddInstTable(InstTable, "MOVM",   0x00, DecodeString1);
  AddInstTable(InstTable, "XCHM",   0x01, DecodeString1);
  AddInstTable(InstTable, "CMPME",  0x04, DecodeString1);
  AddInstTable(InstTable, "CMPMNE", 0x05, DecodeString1);
  AddInstTable(InstTable, "CMPMC",  0x07, DecodeString1);
  AddInstTable(InstTable, "CMPMNC", 0x06, DecodeString1);

  AddInstTable(InstTable, "MOVBK",   0x20, DecodeString2);
  AddInstTable(InstTable, "XCHBK",   0x21, DecodeString2);
  AddInstTable(InstTable, "CMPBKE",  0x24, DecodeString2);
  AddInstTable(InstTable, "CMPBKNE", 0x25, DecodeString2);
  AddInstTable(InstTable, "CMPBKC",  0x27, DecodeString2);
  AddInstTable(InstTable, "CMPBKNC", 0x26, DecodeString2);

  AddFixed("SWRS", 0x43);
  AddInstTable(InstTable, "SEL", 0, DecodeSEL);
  AddFixed("NOP", 0x00);
  AddFixed("EI", 0x4b);
  AddFixed("DI", 0x4a);

  AddInstTable(InstTable, "BIT", 0, DecodeBIT);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/
/* interface to common layer */

static void MakeCode_78K3(void)
{
  CodeLen = 0; DontPrint = False; OpSize = -1;
  AssumeByte = False;

  /* zu ignorierendes */

  if (Memo(""))
    return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False))
    return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static Boolean IsDef_78K3(void)
{
  return Memo("BIT");
}

static void InternSymbol_78K3(char *pAsc, TempResult *pErg)
{
  if (!as_strcasecmp(pAsc, "PSWL"))
  {
    pErg->Typ = TempInt;
    pErg->Contents.Int = PSWLAddr;
  }
  else if (!as_strcasecmp(pAsc, "PSWH"))
  {
    pErg->Typ = TempInt;
    pErg->Contents.Int = PSWHAddr;
  }
}

static void SwitchFrom_78K3(void)
{
  DeinitFields();
}

static void SwitchTo_78K3(void)
{
  PFamilyDescr pDescr;

  pDescr = FindFamilyByName("78K3");

  TurnWords = False;
  ConstMode = ConstModeIntel;

  PCSymbol = "PC";
  HeaderID = pDescr->Id;
  NOPCode = 0x00;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;

  pASSUMERecs = ASSUME78K3s;
  ASSUMERecCnt = sizeof(ASSUME78K3s) / sizeof(ASSUME78K3s[0]);

  MakeCode = MakeCode_78K3;
  IsDef = IsDef_78K3;
  InternSymbol = InternSymbol_78K3;
  SwitchFrom = SwitchFrom_78K3; InitFields();
}

void code78k3_init(void)
{
  CPU78310 = AddCPU("78310", SwitchTo_78K3);
}
