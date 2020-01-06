/* code68k.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 680x0-Familie                                               */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "endian.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "motpseudo.h"
#include "asmitree.h"
#include "codevars.h"
#include "errmsg.h"

#include "code68k.h"

typedef enum
{
  e68KGen1a, /* 68008/68000 */
  e68KGen1b, /* 68010/68012 */
  eColdfire,
  eCPU32,
  e68KGen2,  /* 68020/68030 */
  e68KGen3   /* 68040 */
} tFamily;

typedef enum
{
  eCfISA_None,
  eCfISA_A,
  eCfISA_APlus,
  eCfISA_B,
  eCfISA_C
} tCfISA;

typedef enum
{
  eFlagLogCCR = 1 << 0,
  eFlagIdxScaling = 1 << 1,
  eFlagCALLM_RTM = 1 << 2,
  eFlagIntFPU = 1 << 3,
  eFlagExtFPU = 1 << 4,
  eFlagIntPMMU = 1 << 5,
  eFlagBranch32 = 1 << 6,
  eFlagMAC = 1 << 7,
  eFlagEMAC = 1 << 8
} tSuppFlags;

enum
{
  Std_Variant = 0,
  I_Variant = 4,
  A_Variant = 8,
  VariantMask = 12
};

typedef struct
{
  char *Name;
  Word Code;
} tCtReg;

#define MAX_CTREGS_GROUPS 4

typedef struct
{
  const char *pName;
  LongWord AddrSpaceMask;
  tFamily Family;
  tCfISA CfISA;
  tSuppFlags SuppFlags;
  const tCtReg *pCtRegs[MAX_CTREGS_GROUPS];
} tCPUProps;

typedef struct
{
  Word Code;
  Boolean MustSup;
  Word FamilyMask;
} FixedOrder;

typedef struct
{
  Byte Code;
  Boolean Dya;
  tSuppFlags NeedsSuppFlags;
} FPUOp;

typedef struct
{
  const char *pName;
  Byte Size;
  Word Code;
} PMMUReg;

#define FixedOrderCnt 10
#define CtRegCnt 29
#define FPUOpCnt 47
#define PMMURegCnt 13

#define EMACAvailName  "HASEMAC"
#define PMMUAvailName  "HASPMMU"     /* PMMU-Befehle erlaubt */
#define FullPMMUName   "FULLPMMU"    /* voller PMMU-Befehlssatz */

enum
{
  Mdata = 1,                     /* Adressierungsmasken */
  Madr = 2,
  Madri = 4,
  Mpost = 8,
  Mpre = 16,
  Mdadri = 32,
  Maix = 64,
  Mpc = 128,
  Mpcidx = 256,
  Mabs = 512,
  Mimm = 1024,
  Mfpn = 2048,
  Mfpcr = 4096
};

static ShortInt OpSize;
static ShortInt RelPos;
static Boolean PMMUAvail;               /* PMMU-Befehle erlaubt? */
static Boolean FullPMMU;                /* voller PMMU-Befehlssatz? */
static Byte AdrNum;                     /* Adressierungsnummer */
static Word AdrMode;                    /* Adressierungsmodus */
static Word AdrVals[10];                /* die Worte selber */

static FixedOrder *FixedOrders;
static FPUOp *FPUOps;
static PMMUReg *PMMURegs;

static const tCPUProps *pCurrCPUProps;
static tSymbolSize NativeFloatSize;

static const Byte FSizeCodes[10] =
{
  6, 4, 0, 7, 0, 1, 5, 2, 0, 3
};

/*-------------------------------------------------------------------------*/
/* Unterroutinen */

#define CopyAdrVals(Dest) memcpy(Dest, AdrVals, AdrCnt)

static Boolean CheckFamilyCore(unsigned FamilyMask)
{
  return !!((FamilyMask >> pCurrCPUProps->Family) & 1);
}

static Boolean CheckFamily(unsigned FamilyMask)
{
  if (CheckFamilyCore(FamilyMask))
    return True;
  WrStrErrorPos(ErrNum_InstructionNotSupported, &OpPart);
  CodeLen = 0;
  return False;
}

static Boolean CheckISA(unsigned ISAMask)
{
  if ((ISAMask >> pCurrCPUProps->CfISA) & 1)
    return True;
  WrStrErrorPos(ErrNum_InstructionNotSupported, &OpPart);
  CodeLen = 0;
  return False;
}

static Boolean CheckNoFamily(unsigned FamilyMask)
{
  if (!CheckFamilyCore(FamilyMask))
    return True;
  WrStrErrorPos(ErrNum_InstructionNotSupported, &OpPart);
  CodeLen = 0;
  return False;
}

static void CheckSup(void)
{
  if (!SupAllowed)
    WrStrErrorPos(ErrNum_PrivOrder, &OpPart);
}

static Boolean CheckColdSize(void)
{
  if ((OpSize > eSymbolSize32Bit) || ((pCurrCPUProps->Family == eColdfire) && (OpSize < eSymbolSize32Bit)))
  {
    WrError(ErrNum_InvOpSize);
    return False;
  }
  else
    return True;
}

static Boolean CheckFloatSize(void)
{
  if (!*AttrPart.Str)
    OpSize = NativeFloatSize;

  switch (OpSize)
  {
    case eSymbolSize8Bit:
    case eSymbolSize16Bit:
    case eSymbolSize32Bit:
    case eSymbolSizeFloat32Bit:
    case eSymbolSizeFloat64Bit:
      return True;
    case eSymbolSizeFloat96Bit:
    case eSymbolSizeFloatDec96Bit:
      if (pCurrCPUProps->Family != eColdfire)
        return True;
      /* else fall-through */
    default:
      WrError(ErrNum_InvOpSize);
      return False;
  }
}

static Boolean FloatOpSizeFitsDataReg(tSymbolSize OpSize)
{
  return (OpSize <= eSymbolSize32Bit) || (OpSize == eSymbolSizeFloat32Bit);
}

/*-------------------------------------------------------------------------*/
/* Adressparser */

typedef enum
{
  PC, AReg, Index, indir, Disp, None
} CompType;

/* static const char *CompNames[] = { "PC", "AReg", "Index", "indir", "Disp", "None" }; */

typedef struct
{
  tStrComp Comp;
  CompType Art;
  Word ANummer, INummer;
  Boolean Long;
  Word Scale;
  ShortInt Size;
  LongInt Wert;
} AdrComp;

static void ClrAdrVals(void)
{
  AdrNum = 0;
  AdrCnt = 0;
}

static Boolean ACheckFamily(unsigned FamilyMask, const tStrComp *pAdrComp)
{
  if (CheckFamilyCore(FamilyMask))
    return True;
  WrStrErrorPos(ErrNum_AddrModeNotSupported, pAdrComp);
  ClrAdrVals();
  return False;
}

static Boolean ValReg(char Ch)
{
  return ((Ch >= '0') && (Ch <= '7'));
}

static Boolean CodeReg(const char *s, Word *pErg)
{
  Boolean Result = True;

  if (strlen(s) != 2)
  {
    *pErg = 16;
    Result = False;
  }
  else if (!as_strcasecmp(s, "SP"))
    *pErg = 15;
  else if (ValReg(s[1]))
  {
    if (mytoupper(*s) == 'D')
      *pErg = s[1] - '0';
    else if (mytoupper(*s) == 'A')
      *pErg = s[1] - '0' + 8;
    else
      Result = False;
  }
  else
    Result = False;

  return Result;
}

static Boolean CodeRegPair(char *Asc, Word *Erg1, Word *Erg2)
{
  if ((strlen(Asc) != 5)
   || (mytoupper(*Asc) != 'D')
   || (Asc[2] != ':')
   || (mytoupper(Asc[3]) != 'D')
   || (!(ValReg(Asc[1]) && ValReg(Asc[4]))))
    return False;

  *Erg1 = Asc[1] - '0';
  *Erg2 = Asc[4] - '0';

  return True;
}

static Boolean CodeIndRegPair(char *Asc, Word *Erg1, Word *Erg2)
{
  if ((strlen(Asc) != 9)
   || (*Asc != '(')
   || ((mytoupper(Asc[1]) != 'D') && (mytoupper(Asc[1]) != 'A'))
   || (Asc[3] != ')')
   || (Asc[4] != ':')
   || (Asc[5] != '(')
   || ((mytoupper(Asc[6]) != 'D') && (mytoupper(Asc[6]) != 'A'))
   || (Asc[8] != ')')
   || (!(ValReg(Asc[2]) && ValReg(Asc[7]))))
    return False;

  *Erg1 = Asc[2] - '0' + ((mytoupper(Asc[1]) == 'A') ? 8 : 0);
  *Erg2 = Asc[7] - '0' + ((mytoupper(Asc[6]) == 'A') ? 8 : 0);

  return True;
}

static Boolean CodeCache(char *Asc, Word *Erg)
{
   if (!as_strcasecmp(Asc, "IC"))
     *Erg = 2;
   else if (!as_strcasecmp(Asc, "DC"))
     *Erg = 1;
   else if (!as_strcasecmp(Asc, "IC/DC"))
     *Erg = 3;
   else if (!as_strcasecmp(Asc, "DC/IC"))
     *Erg = 3;
   else
     return False;
   return True;
}

static Boolean DecodeCtrlReg(char *Asc, Word *Erg)
{
  int Grp;
  String Asc_N;
  const tCtReg *pReg;

  strmaxcpy(Asc_N, Asc, STRINGSIZE);
  NLS_UpString(Asc_N);
  Asc = Asc_N;

  for (Grp = 0; Grp < MAX_CTREGS_GROUPS; Grp++)
  {
    pReg = pCurrCPUProps->pCtRegs[Grp];
    if (!pReg)
      return False;
    for (; pReg->Name; pReg++)
      if (!strcmp(pReg->Name, Asc))
      {
        *Erg = pReg->Code;
        return True;
      }
  }
  return False;
}

static Boolean OneField(const tStrComp *pArg, Word *Erg, Boolean Ab1)
{
  Boolean ValOK;

  if ((strlen(pArg->Str) == 2) && (mytoupper(*pArg->Str) == 'D') && (ValReg(pArg->Str[1])))
  {
    *Erg = 0x20 + (pArg->Str[1] - '0');
    return True;
  }
  else
  {
    *Erg = EvalStrIntExpression(pArg, Int8, &ValOK);
    if ((Ab1) && (*Erg == 32))
      *Erg = 0;
    return ((ValOK) && (*Erg < 32));
  }
}

static Boolean SplitBitField(tStrComp *pArg, Word *Erg)
{
  char *p;
  Word OfsVal;
  tStrComp FieldArg, OffsArg, WidthArg;

  p = strchr(pArg->Str, '{');
  if (!p)
    return False;
  StrCompSplitRef(pArg, &FieldArg, pArg, p);
  if ((!*FieldArg.Str) || (FieldArg.Str[strlen(FieldArg.Str) - 1] != '}'))
    return False;
  StrCompShorten(&FieldArg, 1);

  p = strchr(FieldArg.Str, ':');
  if (!p)
    return False;
  StrCompSplitRef(&OffsArg, &WidthArg, &FieldArg, p);
  if (!OneField(&OffsArg, &OfsVal, False))
    return False;
  if (!OneField(&WidthArg, Erg, True))
    return False;
  *Erg += OfsVal << 6;
  return True;
}

static Boolean SplitSize(tStrComp *pArg, ShortInt *DispLen, unsigned OpSizeMask)
{
  ShortInt NewLen = -1;
  int ArgLen = strlen(pArg->Str);

  if ((ArgLen > 2) && (pArg->Str[ArgLen - 2] == '.'))
  {
    switch (mytoupper(pArg->Str[ArgLen - 1]))
    {
      case 'B':
        if (OpSizeMask & 1)
          NewLen = 0;
        else
          goto wrong;
        break;
      case 'W':
        if (OpSizeMask & 2)
          NewLen = 1;
        else
          goto wrong;
        break;
      case 'L':
        if (OpSizeMask & 2)
          NewLen = 2;
        else
          goto wrong;
        break;
      default:
      wrong:
        WrError(ErrNum_InvOpSize);
        return False;
    }
    if ((*DispLen != -1) && (*DispLen != NewLen))
    {
      WrError(ErrNum_ConfOpSizes);
      return False;
    }
    *DispLen = NewLen;
    StrCompShorten(pArg, 2);
  }

  return True;
}

static Boolean ClassComp(AdrComp *C)
{
  int CompLen = strlen(C->Comp.Str);
  char sh[10];

  C->Art = None;
  C->ANummer = C->INummer = 0;
  C->Long = False;
  C->Scale = 0;
  C->Size = -1;
  C->Wert = 0;

  if ((*C->Comp.Str == '[') && (C->Comp.Str[CompLen - 1] == ']'))
  {
    C->Art = indir;
    return True;
  }

  if (!as_strcasecmp(C->Comp.Str, "PC"))
  {
    C->Art = PC;
    return True;
  }

  memcpy(sh, C->Comp.Str, 2);
  sh[2] = '\0';
  if (CodeReg(sh, &C->ANummer))
  {
    int ScaleOffs = 2;

    if ((C->ANummer > 7) && (CompLen == 2))
    {
      C->Art = AReg;
      C->ANummer -= 8;
      return True;
    }
    else
    {
      if ((CompLen > 3) && (C->Comp.Str[2] == '.'))
      {
        switch (mytoupper(C->Comp.Str[3]))
        {
          case 'L':
            C->Long = True;
            break;
          case 'W':
            C->Long = False;
            break;
          default:
            return False;
        }
        ScaleOffs = 4;
      }
      else
        C->Long = (pCurrCPUProps->Family == eColdfire);
      if ((CompLen > ScaleOffs + 1) && (C->Comp.Str[ScaleOffs] == '*'))
      {
        switch (C->Comp.Str[ScaleOffs + 1])
        {
          case '1':
            C->Scale = 0;
            break;
          case '2':
            C->Scale = 1;
            break;
          case '4':
            C->Scale = 2;
            break;
          case '8':
            if (pCurrCPUProps->Family == eColdfire)
              return False;
            C->Scale = 3;
            break;
          default:
            return False;
        }
        ScaleOffs += 2;
      }
      else
        C->Scale = 0;
      C->INummer = C->ANummer;
      C->Art = Index;
      return True;
    }
  }

  C->Art = Disp;
  if ((CompLen >= 2) && (C->Comp.Str[CompLen - 2] == '.'))
  {
    switch (mytoupper(C->Comp.Str[CompLen - 1]))
    {
      case 'L':
        C->Size = 2;
        break;
      case 'W':
        C->Size = 1;
        break;
      default:
        return False;
    }
    StrCompShorten(&C->Comp, 2);
  }
  else
    C->Size = -1;
  C->Art = Disp;
  return True;
}

static void SwapAdrComps(AdrComp *pComp1, AdrComp *pComp2)
{
  AdrComp Tmp;

  Tmp = *pComp1;
  *pComp1 = *pComp2;
  *pComp2 = Tmp;
}

static void AdrCompToIndex(AdrComp *pComp)
{
  pComp->Art = Index;
  pComp->INummer = pComp->ANummer + 8;
  pComp->Long = False;
  pComp->Scale = 0;
}

static Boolean IsShortAdr(LongInt Addr)
{
  LongWord OrigAddr = (LongWord)Addr, ExtAddr;

  /* Assuming we would code this address as short address... */

  ExtAddr = OrigAddr & 0xffff;
  if (ExtAddr & 0x8000)
    ExtAddr |= 0xffff0000ul;

  /* ...would this result in the same address on the bus? */

  return (ExtAddr & pCurrCPUProps->AddrSpaceMask) == (OrigAddr & pCurrCPUProps->AddrSpaceMask);
}

static Boolean IsDisp8(LongInt Disp)
{
  return ((Disp >= -128) && (Disp <= 127));
}

static Boolean IsDisp16(LongInt Disp)
{
  return ((Disp >= -32768) && (Disp <= 32767));
}

ShortInt GetDispLen(LongInt Disp)
{
  if (IsDisp8(Disp))
    return 0;
  else if (IsDisp16(Disp))
    return 1;
  else
    return 2;
}

static void ChkEven(LongInt Adr)
{
  switch (pCurrCPUProps->Family)
  {
    case e68KGen1a:
    case e68KGen1b:
    case eColdfire:
      if (Odd(Adr))
        WrError(ErrNum_AddrNotAligned);
      break;
    default:
      break;
  }
}

static void DecodeAbs(const tStrComp *pArg, ShortInt Size)
{
  Boolean ValOK;
  LongInt HVal;
  Integer HVal16;

  AdrCnt = 0;

  FirstPassUnknown = False;
  HVal = EvalStrIntExpression(pArg, Int32, &ValOK);

  if (ValOK)
  {
    if ((!FirstPassUnknown) && (OpSize > eSymbolSize8Bit))
      ChkEven(HVal);
    HVal16 = HVal;

    if (Size == -1)
      Size = (IsShortAdr(HVal)) ? 1 : 2;
    AdrNum = 10;

    if (Size == 1)
    {
      if (!IsShortAdr(HVal))
      {
        WrError(ErrNum_NoShortAddr);
        AdrNum = 0;
      }
      else
      {
        AdrMode = 0x38;
        AdrVals[0] = HVal16;
        AdrCnt = 2;
      }
    }
    else
    {
      AdrMode = 0x39;
      AdrVals[0] = HVal >> 16;
      AdrVals[1] = HVal & 0xffff;
      AdrCnt = 4;
    }
  }
}

static void DecodeAdr(const tStrComp *pArg, Word Erl)
{
  Byte i;
  int ArgLen;
  char *p;
  Word rerg;
  Byte lklamm, rklamm, lastrklamm;
  Boolean doklamm;

  AdrComp AdrComps[3], OneComp;
  Byte CompCnt;
  ShortInt OutDispLen = -1;
  Boolean PreInd;

#ifdef HAS64
  QuadInt QVal;
#endif
  LongInt HVal;
  Integer HVal16;
  ShortInt HVal8;
  Double DVal;
  Boolean ValOK;
  Word SwapField[6];
  String ArgStr;
  tStrComp Arg;
  char CReg[10];
  const unsigned ExtAddrFamilyMask = (1 << e68KGen3) | (1 << e68KGen2) | (1 << eCPU32);
  IntType DispIntType;

  /* some insns decode the same arg twice, so we must keep the original string intact. */

  StrCompMkTemp(&Arg, ArgStr);
  StrCompCopy(&Arg, pArg);
  KillPrefBlanksStrComp(&Arg);
  KillPostBlanksStrComp(&Arg);
  ArgLen = strlen(Arg.Str);
  ClrAdrVals();

  /* immediate : */

  if (*Arg.Str == '#')
  {
    tStrComp ImmArg;

    StrCompRefRight(&ImmArg, &Arg, 1);
    KillPrefBlanksStrComp(&ImmArg);

    AdrNum = 11;
    AdrMode = 0x3c;
    switch (OpSize)
    {
      case eSymbolSize8Bit:
        AdrCnt = 2;
        HVal8 = EvalStrIntExpression(&ImmArg, Int8, &ValOK);
        if (ValOK)
          AdrVals[0] = (Word)((Byte) HVal8);
        break;
      case eSymbolSize16Bit:
        AdrCnt = 2;
        HVal16 = EvalStrIntExpression(&ImmArg, Int16, &ValOK);
        if (ValOK)
          AdrVals[0] = (Word) HVal16;
        break;
      case eSymbolSize32Bit:
        AdrCnt = 4;
        HVal = EvalStrIntExpression(&ImmArg, Int32, &ValOK);
        if (ValOK)
        {
          AdrVals[0] = HVal >> 16;
          AdrVals[1] = HVal & 0xffff;
        }
        break;
#ifdef HAS64
      case eSymbolSize64Bit:
        AdrCnt = 8;
        QVal = EvalStrIntExpression(&ImmArg, Int64, &ValOK);
        if (ValOK)
        {
          AdrVals[0] = (QVal >> 48) & 0xffff;
          AdrVals[1] = (QVal >> 32) & 0xffff;
          AdrVals[2] = (QVal >> 16) & 0xffff;
          AdrVals[3] = (QVal      ) & 0xffff;
        }
        break;
#endif
      case eSymbolSizeFloat32Bit:
        AdrCnt = 4;
        DVal = EvalStrFloatExpression(&ImmArg, Float32, &ValOK);
        if (ValOK)
        {
          Double_2_ieee4(DVal, (Byte *) SwapField, BigEndian);
          if (BigEndian)
            DWSwap((Byte *) SwapField, 4);
          AdrVals[0] = SwapField[1];
          AdrVals[1] = SwapField[0];
        }
        break;
      case eSymbolSizeFloat64Bit:
        AdrCnt = 8;
        DVal = EvalStrFloatExpression(&ImmArg, Float64, &ValOK);
        if (ValOK)
        {
          Double_2_ieee8(DVal, (Byte *) SwapField, BigEndian);
          if (BigEndian)
            QWSwap((Byte *) SwapField, 8);
          AdrVals[0] = SwapField[3];
          AdrVals[1] = SwapField[2];
          AdrVals[2] = SwapField[1];
          AdrVals[3] = SwapField[0];
        }
        break;
      case eSymbolSizeFloat96Bit:
        AdrCnt = 12;
        DVal = EvalStrFloatExpression(&ImmArg, Float64, &ValOK);
        if (ValOK)
        {
          Double_2_ieee10(DVal, (Byte *) SwapField, False);
          if (BigEndian)
            WSwap((Byte *) SwapField, 10);
          AdrVals[0] = SwapField[4];
          AdrVals[1] = 0;
          AdrVals[2] = SwapField[3];
          AdrVals[3] = SwapField[2];
          AdrVals[4] = SwapField[1];
          AdrVals[5] = SwapField[0];
        }
        break;
      case eSymbolSizeFloatDec96Bit:
        AdrCnt = 12;
        DVal = EvalStrFloatExpression(&ImmArg, Float64, &ValOK);
        if (ValOK)
        {
          ConvertMotoFloatDec(DVal, (Byte *) SwapField, False);
          AdrVals[0] = SwapField[5];
          AdrVals[1] = SwapField[4];
          AdrVals[2] = SwapField[3];
          AdrVals[3] = SwapField[2];
          AdrVals[4] = SwapField[1];
          AdrVals[5] = SwapField[0];
        }
        break;
      case 8: /* special arg 1..8 */
        AdrCnt = 2;
        FirstPassUnknown = False;
        HVal8 = EvalStrIntExpression(&ImmArg, UInt4, &ValOK);
        if (ValOK)
        {
          if (FirstPassUnknown)
           HVal8 = 1;
          ValOK = ChkRange(HVal8, 1, 8);
        }
        if (ValOK)
          AdrVals[0] = (Word)((Byte) HVal8);
        break;
    }
    goto chk;
  }

  /* CPU-Register direkt: */

  if (CodeReg(Arg.Str, &AdrMode))
  {
    AdrCnt = 0;
    AdrNum = (AdrMode >> 3) + 1;
    goto chk;
  }

  /* Gleitkommaregister direkt: */

  if (!as_strncasecmp(Arg.Str, "FP", 2))
  {
    if ((ArgLen == 3) && (ValReg(Arg.Str[2])))
    {
      AdrMode = Arg.Str[2] - '0';
      AdrCnt = 0;
      AdrNum = 12;
      goto chk;
    }
    if (!as_strcasecmp(Arg.Str, "FPCR"))
    {
      AdrMode = 4;
      AdrNum = 13;
      goto chk;
    }
    if (!as_strcasecmp(Arg.Str, "FPSR"))
    {
      AdrMode = 2;
      AdrNum = 13;
      goto chk;
    }
    if (!as_strcasecmp(Arg.Str, "FPIAR"))
    {
      AdrMode = 1;
      AdrNum = 13;
      goto chk;
    }
  }

  /* Adressregister indirekt mit Predekrement: */

  if ((ArgLen == 5) && (*Arg.Str == '-') && (Arg.Str[1] == '(') && (Arg.Str[4] == ')'))
  {
    strcpy(CReg, Arg.Str + 2);
    CReg[2] = '\0';
    if ((CodeReg(CReg, &rerg)) && (rerg > 7) )
    {
      AdrMode = rerg + 24;
      AdrCnt = 0;
      AdrNum = 5;
      goto chk;
    }
  }

  /* Adressregister indirekt mit Postinkrement */

  if ((ArgLen == 5) && (*Arg.Str == '(') && (Arg.Str[3] == ')') && (Arg.Str[4] == '+'))
  {
    strcpy(CReg, Arg.Str + 1);
    CReg[2] = '\0';
    if ((CodeReg(CReg, &rerg)) && (rerg > 7))
    {
      AdrMode = rerg + 16;
      AdrCnt = 0;
      AdrNum = 4;
      goto chk;
    }
  }

  /* Unterscheidung direkt<->indirekt: */

  lklamm = 0;
  rklamm = 0;
  lastrklamm = 0;
  doklamm = True;
  for (p = Arg.Str; *p; p++)
  {
    if (*p == '[')
      doklamm = False;
    if (*p == ']')
      doklamm = True;
    if (doklamm)
    {
      if (*p == '(')
        lklamm++;
      else if (*p == ')')
      {
        rklamm++;
        lastrklamm = p - Arg.Str;
      }
    }
  }

  if ((lklamm == 1) && (rklamm == 1) && (lastrklamm == ArgLen - 1))
  {
    tStrComp OutDisp, IndirComps, Remainder;
    char *pCompSplit;

    /* aeusseres Displacement abspalten, Klammern loeschen: */

    p = strchr(Arg.Str, '(');
    *p = '\0';
    StrCompSplitRef(&OutDisp, &IndirComps, &Arg, p);
    OutDispLen = -1;
    if (!SplitSize(&OutDisp, &OutDispLen, 7))
      return;
    StrCompShorten(&IndirComps, 1);

    /* in Komponenten zerteilen: */

    CompCnt = 0;
    do
    {
      doklamm = True;
      pCompSplit = IndirComps.Str;
      do
      {
        if (*pCompSplit == '[')
          doklamm = False;
        else if (*pCompSplit == ']')
          doklamm = True;
        pCompSplit++;
      }
      while (((!doklamm) || (*pCompSplit != ',')) && (*pCompSplit != '\0'));

      if (*pCompSplit == '\0')
      {
        AdrComps[CompCnt].Comp = IndirComps;
        pCompSplit = NULL;
      }
      else
      {
        StrCompSplitRef(&AdrComps[CompCnt].Comp, &Remainder, &IndirComps, pCompSplit);
        IndirComps = Remainder;
      }

      /* ignore empty component */

      if (!AdrComps[CompCnt].Comp.Str[0])
        continue;
      if (!ClassComp(&AdrComps[CompCnt]))
      {
        WrStrErrorPos(ErrNum_InvAddrMode, &AdrComps[CompCnt].Comp);
        return;
      }

      /* Base register position is already occupied and we get another one: */

      if ((CompCnt == 1) && ((AdrComps[CompCnt].Art == AReg) || (AdrComps[CompCnt].Art == PC)))
      {
        /* Index register at "base position": just swap comp 0 & 1, so we get (An,Xi) or (PC,Xi): */

        if (AdrComps[0].Art == Index)
          SwapAdrComps(&AdrComps[CompCnt], &AdrComps[0]);

        /* Address register at "base position" and we add PC: also swap and convert it to index so we get again (PC,Xi): */

        else if ((AdrComps[0].Art == AReg) && (AdrComps[CompCnt].Art == PC))
        {
          SwapAdrComps(&AdrComps[CompCnt], &AdrComps[0]);
          AdrCompToIndex(&AdrComps[CompCnt]);
        }

        /* Otherwise, convert address to general index register.  Result may require 68020++ modes: */

        else
          AdrCompToIndex(&AdrComps[CompCnt]);

        CompCnt++;
      }

      /* a displacement found inside (...), but outside [...].  Explicit
         sizes must be consistent, implicitly checked by SplitSize(). */

      else if (AdrComps[CompCnt].Art == Disp)
      {
        if (*OutDisp.Str)
        {
          WrError(ErrNum_InvAddrMode);
          return;
        }
        OutDisp = AdrComps[CompCnt].Comp;
        OutDispLen = AdrComps[CompCnt].Size;
      }

      /* no second index */

      else if ((AdrComps[CompCnt].Art != Index) && (CompCnt != 0))
      {
        WrError(ErrNum_InvAddrMode);
        return;
      }

      else
        CompCnt++;
    }
    while (pCompSplit);

    if ((CompCnt > 2) || ((CompCnt > 1) && (AdrComps[0].Art == Index)))
    {
      WrError(ErrNum_InvAddrMode);
      return;
    }

    /* 0. Absolut in Klammern (d) */

    if (CompCnt == 0)
    {
      DecodeAbs(&OutDisp, OutDispLen);
    }

    /* 1. Variante (An....), d(An....) */

    else if (AdrComps[0].Art == AReg)
    {

      /* 1.1. Variante (An), d(An) */

      if (CompCnt == 1)
      {
        /* 1.1.1. Variante (An) */

        if ((*OutDisp.Str == '\0') && ((Madri & Erl) != 0))
        {
          AdrMode = 0x10 + AdrComps[0].ANummer;
          AdrNum = 3;
          AdrCnt = 0;
          goto chk;
        }

        /* 1.1.2. Variante d(An) */

        else
        {
          /* only try 32-bit displacement if explicitly requested, or 68020++ and no size given */

          if (OutDispLen < 0)
            DispIntType = CheckFamilyCore(ExtAddrFamilyMask) ? SInt32 : SInt16;
          else
            DispIntType = (OutDispLen >= 2) ? SInt32 : SInt16;
          HVal = EvalStrIntExpression(&OutDisp, DispIntType, &ValOK);
          if (!ValOK)
            return;
          if (ValOK && (HVal == 0) && ((Madri & Erl) != 0) && (OutDispLen == -1))
          {
            AdrMode = 0x10 + AdrComps[0].ANummer;
            AdrNum = 3;
            AdrCnt = 0;
            goto chk;
          }
          if (OutDispLen == -1)
            OutDispLen = (IsDisp16(HVal)) ? 1 : 2;
          switch (OutDispLen)
          {
            case 1:                   /* d16(An) */
              AdrMode = 0x28 + AdrComps[0].ANummer;
              AdrNum = 6;
              AdrCnt = 2;
              AdrVals[0] = HVal & 0xffff;
              goto chk;
            case 2:                   /* d32(An) */
              AdrMode = 0x30 + AdrComps[0].ANummer;
              AdrNum = 7;
              AdrCnt = 6;
              AdrVals[0] = 0x0170;
              AdrVals[1] = (HVal >> 16) & 0xffff;
              AdrVals[2] = HVal & 0xffff;
              ACheckFamily(ExtAddrFamilyMask, pArg);
              goto chk;
          }
        }
      }

      /* 1.2. Variante d(An,Xi) */

      else
      {
        AdrVals[0] = (AdrComps[1].INummer << 12) + (Ord(AdrComps[1].Long) << 11) + (AdrComps[1].Scale << 9);
        AdrMode = 0x30 + AdrComps[0].ANummer;

        /* only try 32-bit displacement if explicitly requested, or 68020++ and no size given */

        if (OutDispLen < 0)
          DispIntType = CheckFamilyCore(ExtAddrFamilyMask) ? SInt32 : SInt8;
        else
          DispIntType = (OutDispLen >= 2) ? SInt32 : (OutDispLen >= 1 ? SInt16 : SInt8);
        HVal = EvalStrIntExpression(&OutDisp, DispIntType, &ValOK);
        if (ValOK)
          switch (OutDispLen)
          {
            case 0:
              if (!IsDisp8(HVal))
              {
                WrError(ErrNum_OverRange);
                ValOK = FALSE;
              }
              break;
            case 1:
              if (!IsDisp16(HVal))
              {
                WrError(ErrNum_OverRange);
                ValOK = FALSE;
              }
              break;
          }
        if (ValOK)
        {
          if (OutDispLen == -1)
            OutDispLen = GetDispLen(HVal);
          switch (OutDispLen)
          {
            case 0:
              AdrNum = 7;
              AdrCnt = 2;
              AdrVals[0] += (HVal & 0xff);
              if ((AdrComps[1].Scale != 0) && (!(pCurrCPUProps->SuppFlags & eFlagIdxScaling)))
              {
                WrStrErrorPos(ErrNum_AddrModeNotSupported, &AdrComps[1].Comp);
                ClrAdrVals();
              }
              goto chk;
            case 1:
              AdrNum = 7;
              AdrCnt = 4;
              AdrVals[0] += 0x120;
              AdrVals[1] = HVal & 0xffff;
              ACheckFamily(ExtAddrFamilyMask, pArg);
              goto chk;
            case 2:
              AdrNum = 7;
              AdrCnt = 6;
              AdrVals[0] += 0x130;
              AdrVals[1] = HVal >> 16;
              AdrVals[2] = HVal & 0xffff;
              ACheckFamily(ExtAddrFamilyMask, pArg);
              goto chk;
          }
        }
      }
    }

    /* 2. Variante d(PC....) */

    else if (AdrComps[0].Art == PC)
    {
      /* 2.1. Variante d(PC) */

      if (CompCnt == 1)
      {
        SymbolQuestionable = False;
        HVal = EvalStrIntExpression(&OutDisp, Int32, &ValOK) - (EProgCounter() + RelPos);
        if (!ValOK)
          return;
        if (OutDispLen < 0)
        {
          if (SymbolQuestionable && !CheckFamilyCore(ExtAddrFamilyMask))
            HVal &= 0x7fff;
          OutDispLen = (IsDisp16(HVal)) ? 1 : 2;
        }
        switch (OutDispLen)
        {
          case 1:
            AdrMode = 0x3a;
            if (!SymbolQuestionable && !IsDisp16(HVal))
            {
              WrError(ErrNum_DistTooBig);
              return;
            }
            AdrNum = 8;
            AdrCnt = 2;
            AdrVals[0] = HVal & 0xffff;
            goto chk;
          case 2:
            AdrMode = 0x3b;
            AdrNum = 9;
            AdrCnt = 6;
            AdrVals[0] = 0x170;
            AdrVals[1] = HVal >> 16;
            AdrVals[2] = HVal & 0xffff;
            ACheckFamily(ExtAddrFamilyMask, pArg);
            goto chk;
        }
      }

      /* 2.2. Variante d(PC,Xi) */

      else
      {
        AdrVals[0] = (AdrComps[1].INummer << 12) + (Ord(AdrComps[1].Long) << 11) + (AdrComps[1].Scale << 9);
        SymbolQuestionable = False;
        HVal = EvalStrIntExpression(&OutDisp, Int32, &ValOK) - (EProgCounter() + RelPos);
        if (!ValOK)
          return;
        if (OutDispLen < 0)
        {
          if (SymbolQuestionable && !CheckFamilyCore(ExtAddrFamilyMask))
            HVal &= 0x7f;
          OutDispLen = GetDispLen(HVal);
        }
        AdrMode = 0x3b;
        switch (OutDispLen)
        {
          case 0:
            if (!SymbolQuestionable && !IsDisp8(HVal))
            {
              WrError(ErrNum_DistTooBig);
              return;
            }
            AdrVals[0] += (HVal & 0xff);
            AdrCnt = 2;
            AdrNum = 9;
            if ((AdrComps[1].Scale != 0) && (!(pCurrCPUProps->SuppFlags & eFlagIdxScaling)))
            {
              WrStrErrorPos(ErrNum_AddrModeNotSupported, &AdrComps[1].Comp);
              ClrAdrVals();
            }
            goto chk;
          case 1:
            if (!SymbolQuestionable && !IsDisp16(HVal))
            {
              WrError(ErrNum_DistTooBig);
              return;
            }
            AdrVals[0] += 0x120;
            AdrCnt = 4;
            AdrNum = 9;
            AdrVals[1] = HVal & 0xffff;
            ACheckFamily(ExtAddrFamilyMask, pArg);
            goto chk;
          case 2:
            AdrVals[0] += 0x130;
            AdrCnt = 6;
            AdrNum = 9;
            AdrVals[1] = HVal >> 16;
            AdrVals[2] = HVal & 0xffff;
            ACheckFamily(ExtAddrFamilyMask, pArg);
            goto chk;
        }
      }
    }

    /* 3. Variante (Xi), d(Xi) */

    else if (AdrComps[0].Art == Index)
    {
      AdrVals[0] = (AdrComps[0].INummer << 12) + (Ord(AdrComps[0].Long) << 11) + (AdrComps[0].Scale << 9) + 0x180;
      AdrMode = 0x30;
      if (*OutDisp.Str == '\0')
      {
        AdrVals[0] = AdrVals[0] + 0x0010;
        AdrCnt = 2;
        AdrNum = 7;
        ACheckFamily(ExtAddrFamilyMask, pArg);
        goto chk;
      }
      else
      {
        HVal = EvalStrIntExpression(&OutDisp, (OutDispLen != 1) ? SInt32 : SInt16, &ValOK);
        if (ValOK)
        {
          if (OutDispLen == -1)
            OutDispLen = IsDisp16(HVal) ? 1 : 2;
          switch (OutDispLen)
          {
            case 0:
            case 1:
              AdrVals[0] = AdrVals[0] + 0x0020;
              AdrVals[1] = HVal & 0xffff;
              AdrNum = 7;
              AdrCnt = 4;
              ACheckFamily(ExtAddrFamilyMask, pArg);
              goto chk;
            case 2:
              AdrVals[0] = AdrVals[0] + 0x0030;
              AdrNum = 7;
              AdrCnt = 6;
              AdrVals[1] = HVal >> 16;
              AdrVals[2] = HVal & 0xffff;
              ACheckFamily(ExtAddrFamilyMask, pArg);
              goto chk;
          }
        }
      }
    }

    /* 4. Variante indirekt: */

    else if (AdrComps[0].Art == indir)
    {
      /* erst ab 68020 erlaubt */

      if (!ACheckFamily((1 << e68KGen3) | (1 << e68KGen2), pArg))
        return;

      /* Unterscheidung Vor- <---> Nachindizierung: */

      if (CompCnt == 2)
      {
        PreInd = False;
        AdrComps[2] = AdrComps[1];
      }
      else
      {
        PreInd = True;
        AdrComps[2].Art = None;
      }

      /* indirektes Argument herauskopieren: */

      StrCompRefRight(&IndirComps, &AdrComps[0].Comp, 1);
      StrCompShorten(&IndirComps, 1);

      /* Felder loeschen: */

      for (i = 0; i < 2; AdrComps[i++].Art = None);

      /* indirekten Ausdruck auseinanderfieseln: */

      do
      {
        /* abschneiden & klassifizieren: */

        pCompSplit = strchr(IndirComps.Str, ',');
        if (!pCompSplit)
          OneComp.Comp = IndirComps;
        else
        {
          StrCompSplitRef(&OneComp.Comp, &Remainder, &IndirComps, pCompSplit);
          IndirComps = Remainder;
        }
        if (!ClassComp(&OneComp))
        {
          WrError(ErrNum_InvAddrMode);
          return;
        }

        /* passend einsortieren: */

        if ((AdrComps[1].Art != None) && (OneComp.Art == AReg))
        {
          OneComp.Art = Index;
          OneComp.INummer = OneComp.ANummer + 8;
          OneComp.Long = False;
          OneComp.Scale = 0;
        }
        switch (OneComp.Art)
        {
          case Disp:
            i = 0;
            break;
          case AReg:
          case PC:
            i = 1;
            break;
          case Index:
            i = 2;
            break;
          default:
            i = -1;
        }
        if (AdrComps[i].Art != None)
        {
          WrError(ErrNum_InvAddrMode);
          return;
        }
        else
          AdrComps[i] = OneComp;
      }
      while (pCompSplit);

      /* extension word: 68020 format */

      AdrVals[0] = 0x100;

      /* bit 2 = post-indexed. */

      if (!PreInd)
        AdrVals[0] |= 0x0004;

      /* Set post-indexed also for no index register for compatibility with older versions. */

      if (AdrComps[2].Art == None)
        AdrVals[0] |= 0x0040 | 0x0004;
      else
        AdrVals[0] |= (AdrComps[2].INummer << 12) + (Ord(AdrComps[2].Long) << 11) + (AdrComps[2].Scale << 9);

      /* 4.1 Variante d([...PC...]...) */

      if (AdrComps[1].Art == PC)
      {
        if (AdrComps[0].Art == None)
        {
          AdrMode = 0x3b;
          AdrVals[0] |= 0x10;
          AdrNum = 7;
          AdrCnt = 2;
        }
        else
        {
          HVal = EvalStrIntExpression(&AdrComps[0].Comp, Int32, &ValOK);
          HVal -= EProgCounter() + RelPos;
          if (!ValOK)
            return;
          switch (AdrComps[0].Size)
          {
            case -1:
             if (IsDisp16(HVal))
               goto PCIs16;
             else
               goto PCIs32;
            case 1:
              if (!IsDisp16(HVal))
              {
                WrError(ErrNum_DistTooBig);
                return;
              }
            PCIs16:
              AdrVals[1] = HVal & 0xffff;
              AdrMode = 0x3b;
              AdrVals[0] += 0x20;
              AdrNum = 7;
              AdrCnt = 4;
              break;
            case 2:
            PCIs32:
              AdrVals[1] = HVal >> 16;
              AdrVals[2] = HVal & 0xffff;
              AdrMode = 0x3b;
              AdrVals[0] += 0x30;
              AdrNum = 7;
              AdrCnt = 6;
              break;
          }
        }
      }

      /* 4.2 Variante d([...An...]...) */

      else
      {
        if (AdrComps[1].Art == None)
        {
          AdrMode = 0x30;
          AdrVals[0] += 0x80;
        }
        else
          AdrMode = 0x30 + AdrComps[1].ANummer;

        if (AdrComps[0].Art == None)
        {
          AdrNum = 7;
          AdrCnt = 2;
          AdrVals[0] += 0x10;
        }
        else
        {
          HVal = EvalStrIntExpression(&AdrComps[0].Comp, Int32, &ValOK);
          if (!ValOK)
            return;
          switch (AdrComps[0].Size)
          {
            case -1:
              if (IsDisp16(HVal))
                goto AnIs16;
              else
                goto AnIs32;
            case 1:
              if (!IsDisp16(HVal))
              {
                WrError(ErrNum_DistTooBig);
                return;
              }
            AnIs16:
              AdrVals[0] += 0x20;
              AdrVals[1] = HVal & 0xffff;
              AdrNum = 7;
              AdrCnt = 4;
              break;
            case 2:
            AnIs32:
              AdrVals[0] += 0x30;
              AdrVals[1] = HVal >> 16;
              AdrVals[2] = HVal & 0xffff;
              AdrNum = 7;
              AdrCnt = 6;
              break;
          }
        }
      }

      /* aeusseres Displacement: */

      HVal = EvalStrIntExpression(&OutDisp, (OutDispLen == 1) ? SInt16 : SInt32, &ValOK);
      if (!ValOK)
      {
        AdrNum = 0;
        AdrCnt = 0;
        return;
      }
      if (OutDispLen == -1)
        OutDispLen = IsDisp16(HVal) ? 1 : 2;
      if (*OutDisp.Str == '\0')
      {
        AdrVals[0]++;
        goto chk;
      }
      else
        switch (OutDispLen)
        {
          case 0:
          case 1:
            AdrVals[AdrCnt >> 1] = HVal & 0xffff;
            AdrCnt += 2;
            AdrVals[0] += 2;
            break;
          case 2:
            AdrVals[(AdrCnt >> 1)    ] = HVal >> 16;
            AdrVals[(AdrCnt >> 1) + 1] = HVal & 0xffff;
            AdrCnt += 4;
            AdrVals[0] += 3;
            break;
        }

      goto chk;
    }

  }

  /* absolut: */

  else
  {
    if (!SplitSize(&Arg, &OutDispLen, 6))
      return;
    DecodeAbs(&Arg, OutDispLen);
  }

chk:
  if ((AdrNum > 0) && (!(Erl & (1 << (AdrNum - 1)))))
  {
    WrStrErrorPos(ErrNum_InvAddrMode, pArg);
    AdrNum = 0;
  }
}

static Byte OneReg(char *Asc)
{
  if (strlen(Asc) != 2)
    return 16;
  if ((mytoupper(*Asc) != 'A') && (mytoupper(*Asc) != 'D'))
    return 16;
  if (!ValReg(Asc[1]))
    return 16;
  return Asc[1] - '0' + ((mytoupper(*Asc) == 'D') ? 0 : 8);
}

static Boolean DecodeMACACC(const char *pArg, Word *pResult)
{
  /* interprete ACC like ACC0, independent of MAC or EMAC: */

  if (!as_strcasecmp(pArg, "ACC"))
    *pResult = 0;
  else if (!as_strncasecmp(pArg, "ACC", 3) && (strlen(pArg) == 4) && (pArg[3] >= '0') && (pArg[3] <= '3'))
    *pResult = pArg[3] - '0';
  else
    return False;

  /* allow ACC1..3 only on EMAC: */

  if ((!(pCurrCPUProps->SuppFlags & eFlagEMAC)) && *pResult)
    return False;
  return True;
}

static Boolean DecodeMACReg(const char *pArg, Word *pResult)
{
  if (!as_strcasecmp(pArg, "MACSR"))
  {
    *pResult = 4;
    return True;
  }
  if (!as_strcasecmp(pArg, "MASK"))
  {
    *pResult = 6;
    return True;
  }

  /* ACCEXT01/23 only on EMAC: */

  if (pCurrCPUProps->SuppFlags & eFlagEMAC)
  {
    if (!as_strcasecmp(pArg, "ACCEXT01"))
    {
      *pResult = 5;
      return True;
    }
    if (!as_strcasecmp(pArg, "ACCEXT23"))
    {
      *pResult = 7;
      return True;
    }
  }
  return DecodeMACACC(pArg, pResult);
}

static Boolean DecodeRegList(const char *Asc_o, Word *Erg)
{
  Byte h, h2, z;
  char *p, *p2, *pAsc;
  String Asc;

  *Erg = 0;
  strmaxcpy(Asc, Asc_o, STRINGSIZE);
  pAsc = Asc;
  do
  {
    p = strchr(pAsc, '/');
    if (p)
      *p++ = '\0';
    p2 = strchr(pAsc, '-');
    if (!p2)
    {
      if ((h = OneReg(pAsc)) == 16)
        return False;
      *Erg |= 1 << h;
    }
    else
    {
      *p2 = '\0';
      if ((h = OneReg(pAsc)) == 16)
        return False;
      if ((h2 = OneReg(p2 + 1)) == 16)
        return False;
      if (h <= h2)
      {
        for (z = h; z <= h2; z++)
          *Erg |= 1 << z;
      }
      else
      {
        for (z = h; z <= 15; z++)
          *Erg |= 1 << z;
        for (z = 0; z <= h2; z++)
          *Erg |= 1 << z;
      }
    }
    pAsc = p;
  }
  while (p);
  return True;
}

static Boolean DecodeMACScale(const tStrComp *pArg, Word *pResult)
{
  int l = strlen(pArg->Str);
  tStrComp ShiftArg;
  Boolean Left = False, OK;
  Word ShiftCnt;

  /* allow empty argument */

  if (!l)
  {
    *pResult = 0;
    return True;
  }
  /* left or right? */

  if (l < 2)
    return False;
  if (!strncmp(pArg->Str, "<<", 2))
    Left = True;
  else if (!strncmp(pArg->Str, ">>", 2))
    Left = False;
  else
    return False;

  /* evaluate shift cnt - empty count counts as one */

  StrCompRefRight(&ShiftArg, pArg, 2);
  KillPrefBlanksStrCompRef(&ShiftArg);
  if (!*ShiftArg.Str)
  {
    ShiftCnt = 1;
    OK = True;
  }
  else
    ShiftCnt = EvalStrIntExpression(&ShiftArg, UInt1, &OK);
  if (!OK)
    return False;

  /* codify */

  if (ShiftCnt)
    *pResult = Left ? 1 : 3;
  else
    *pResult = 0;
  return True;
}

static Boolean SplitMACUpperLower(Word *pResult, tStrComp *pArg)
{
  char *pSplit;
  tStrComp HalfComp;

  *pResult = 0;
  pSplit = strrchr(pArg->Str, '.');
  if (!pSplit)
  {
    WrStrErrorPos(ErrNum_InvReg, pArg);
    return False;
  }

  StrCompSplitRef(pArg, &HalfComp, pArg, pSplit);
  KillPostBlanksStrComp(pArg);
  if (!as_strcasecmp(HalfComp.Str, "L"))
    *pResult = 0;
  else if (!as_strcasecmp(HalfComp.Str, "U"))
    *pResult = 1;
  else
  {
    WrStrErrorPos(ErrNum_InvReg, &HalfComp);
    return False;
  }
  return True;
}

static Boolean SplitMACANDMASK(Word *pResult, tStrComp *pArg)
{
  char *pSplit, Save;
  tStrComp MaskComp, AddrComp;

  *pResult = 0;
  pSplit = strrchr(pArg->Str, '&');
  if (!pSplit)
    return True;

  Save = StrCompSplitRef(&AddrComp, &MaskComp, pArg, pSplit);
  KillPrefBlanksStrCompRef(&MaskComp);

  /* if no MASK argument, be sure to revert pArg to original state: */

  if (!strcmp(MaskComp.Str, "") || !as_strcasecmp(MaskComp.Str, "MASK"))
  {
    KillPostBlanksStrComp(&AddrComp);
    *pArg = AddrComp;
    *pResult = 1;
  }
  else
    *pSplit = Save;
  return True;
}

/*-------------------------------------------------------------------------*/
/* Dekodierroutinen: Integer-Einheit */

/* 0=MOVE 1=MOVEA */

static void DecodeMOVE(Word Index)
{
  Word MACReg;
  unsigned Variant = Index & VariantMask;

  if (!ChkArgCnt(2, 2));
  else if (!as_strcasecmp(ArgStr[1].Str, "USP"))
  {
    if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
    else if ((pCurrCPUProps->Family != eColdfire) || CheckISA((1 << eCfISA_APlus) | (1 << eCfISA_B) | (1 << eCfISA_C)))
    {
      DecodeAdr(&ArgStr[2], Madr);
      if (AdrNum != 0)
      {
        CodeLen = 2;
        WAsmCode[0] = 0x4e68 | (AdrMode & 7);
        CheckSup();
      }
    }
  }
  else if (!as_strcasecmp(ArgStr[2].Str, "USP"))
  {
    if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
    else if ((pCurrCPUProps->Family != eColdfire) || CheckISA((1 << eCfISA_APlus) | (1 << eCfISA_B) | (1 << eCfISA_C)))
    {
      DecodeAdr(&ArgStr[1], Madr);
      if (AdrNum != 0)
      {
        CodeLen = 2;
        WAsmCode[0] = 0x4e60 | (AdrMode & 7);
        CheckSup();
      }
    }
  }
  else if (!as_strcasecmp(ArgStr[1].Str, "SR"))
  {
    if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[2], Mdata | ((pCurrCPUProps->Family == eColdfire) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mabs));
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        WAsmCode[0] = 0x40c0 | AdrMode;
        CopyAdrVals(WAsmCode + 1);
        if (pCurrCPUProps->Family != e68KGen1a)
          CheckSup();
      }
    }
  }
  else if (!as_strcasecmp(ArgStr[1].Str, "CCR"))
  {
    if (*AttrPart.Str && (OpSize > eSymbolSize16Bit)) WrError(ErrNum_InvOpSize);
    else if (!CheckNoFamily(1 << e68KGen1a));
    else
    {
      OpSize = eSymbolSize8Bit;
      DecodeAdr(&ArgStr[2], Mdata | ((pCurrCPUProps->Family == eColdfire) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mabs));
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        WAsmCode[0] = 0x42c0 | AdrMode;
        CopyAdrVals(WAsmCode + 1);
      }
    }
  }
  else if ((pCurrCPUProps->SuppFlags & eFlagMAC) && (DecodeMACReg(ArgStr[1].Str, &MACReg)))
  {
    Word DestMACReg;

    if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
    else if ((MACReg == 4) && (!as_strcasecmp(ArgStr[2].Str, "CCR")))
    {
      WAsmCode[0] = 0xa9c0;
      CodeLen = 2;
    }
    else if ((MACReg < 4) && DecodeMACReg(ArgStr[2].Str, &DestMACReg) && (DestMACReg < 4) && (pCurrCPUProps->SuppFlags & eFlagEMAC))
    {
      WAsmCode[0] = 0xa110 | (DestMACReg << 9) | (MACReg << 0);
      CodeLen = 2;
    }
    else
    {
      DecodeAdr(&ArgStr[2], Mdata | Madr);
      if (AdrNum != 0)
      {
        CodeLen = 2;
        WAsmCode[0] = 0xa180 | (AdrMode & 15) | (MACReg << 9);
      }
    }
  }
  else if ((pCurrCPUProps->SuppFlags & eFlagMAC) && (DecodeMACReg(ArgStr[2].Str, &MACReg)))
  {
    if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[1], Mdata | Madr | Mimm);
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        WAsmCode[0] = 0xa100 | (AdrMode) | (MACReg << 9);
        CopyAdrVals(WAsmCode + 1);
      }
    }
  }
  else if (!as_strcasecmp(ArgStr[2].Str, "SR"))
  {
    if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[1], Mdata | Mimm | ((pCurrCPUProps->Family == eColdfire) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs));
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        WAsmCode[0] = 0x46c0 | AdrMode;
        CopyAdrVals(WAsmCode + 1);
        CheckSup();
      }
    }
  }
  else if (!as_strcasecmp(ArgStr[2].Str, "CCR"))
  {
    if (*AttrPart.Str && (OpSize > eSymbolSize16Bit)) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[1], Mdata | Mimm | ((pCurrCPUProps->Family == eColdfire) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs));
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        WAsmCode[0] = 0x44c0 | AdrMode;
        CopyAdrVals(WAsmCode + 1);
      }
    }
  }
  else
  {
    if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
    else
    {
      DecodeAdr(&ArgStr[1], ((Variant == I_Variant) ? 0 : Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs) | Mimm);
      if (AdrNum != 0)
      {
        unsigned SrcAdrNum = AdrNum;

        CodeLen = 2 + AdrCnt;
        CopyAdrVals(WAsmCode + 1);
        if (OpSize == eSymbolSize8Bit)
          WAsmCode[0] = 0x1000;
        else if (OpSize == eSymbolSize16Bit)
          WAsmCode[0] = 0x3000;
        else
          WAsmCode[0] = 0x2000;
        WAsmCode[0] |= AdrMode;
        DecodeAdr(&ArgStr[2], ((Variant == A_Variant) ? 0 : Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs) | Madr);
        if (AdrMode != 0)
        {
          Boolean CombinationOK;

          /* ColdFire does not allow all combinations of src+dest: */

          if (pCurrCPUProps->Family == eColdfire)
            switch (SrcAdrNum)
            {
              case 1: /* Dn */
              case 2: /* An */
              case 3: /* (An) */
              case 4: /* (An)+ */
              case 5: /* -(An) */
                CombinationOK = True;
                break;
              case 6: /* (d16,An) */
              case 8: /* (d16,PC) */
                CombinationOK = (AdrNum != 7)   /* no (d8,An,Xi) */
                             && (AdrNum != 10); /* no (xxx).W/L */
                break;
              case 7: /* (d8,An,Xi) */
              case 9: /* (d8,PC,Xi) */
              case 10: /* (xxx).W/L */
                CombinationOK = (AdrNum != 6)   /* no (d16,An) */
                             && (AdrNum != 7)   /* no (d8,An,Xi) */
                             && (AdrNum != 10); /* no (xxx).W/L */
                break;
              case 11: /* #xxx */
                if (AdrNum == 6) /* (d16,An) OK for 8/16 bit starting with ISA B */
                  CombinationOK = (pCurrCPUProps->CfISA >= eCfISA_B) && (OpSize <= eSymbolSize16Bit);
                else
                  CombinationOK = (AdrNum != 7)   /* no (d8,An,Xi) */
                               && (AdrNum != 10); /* no (xxx).W/L */
                break;
              default: /* should not occur */
                CombinationOK = False;
            }
          else
            CombinationOK = True;
          if (!CombinationOK)
          {
            WrError(ErrNum_InvAddrMode);
            CodeLen = 0;
          }
          else
          {
            AdrMode = ((AdrMode & 7) << 3) | (AdrMode >> 3);
            WAsmCode[0] |= AdrMode << 6;
            CopyAdrVals(WAsmCode + (CodeLen >> 1));
            CodeLen += AdrCnt;
          }
        }
      }
    }
  }
}

static void DecodeLEA(Word Index)
{
  UNUSED(Index);

  if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (!ChkArgCnt(2, 2));
  else
  {
    DecodeAdr(&ArgStr[2], Madr);
    if (AdrNum != 0)
    {
      OpSize = eSymbolSize8Bit;
      WAsmCode[0] = 0x41c0 | ((AdrMode & 7) << 9);
      DecodeAdr(&ArgStr[1], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] |= AdrMode;
        CodeLen = 2 + AdrCnt;
        CopyAdrVals(WAsmCode + 1);
      }
    }
  }
}

/* 0=ASR 1=ASL 2=LSR 3=LSL 4=ROXR 5=ROXL 6=ROR 7=ROL */

static void DecodeShift(Word Index)
{
  Boolean ValOK;
  Byte HVal8;
  Word LFlag = (Index >> 2), Op = Index & 3;

  if (!ChkArgCnt(1, 2));
  else if ((*OpPart.Str == 'R') && (!CheckNoFamily(1 << eColdfire)));
  else
  {
    DecodeAdr(&ArgStr[ArgCnt], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum == 1)
    {
      if (CheckColdSize())
      {
        WAsmCode[0] = 0xe000 | AdrMode | (Op << 3) | (OpSize << 6) | (LFlag << 8);
        OpSize = 8;
        if (ArgCnt == 2)
          DecodeAdr(&ArgStr[1], Mdata | Mimm);
        else
        {
          AdrNum = 11;
          AdrVals[0] = 1;
        }
        if ((AdrNum == 1) || ((AdrNum == 11) && (Lo(AdrVals[0]) >= 1) && (Lo(AdrVals[0]) <= 8)))
        {
          CodeLen = 2;
          WAsmCode[0] |= (AdrNum == 1) ? 0x20 | (AdrMode << 9) : ((AdrVals[0] & 7) << 9);
        }
        else
          WrStrErrorPos(ErrNum_InvShiftArg, &ArgStr[1]);
      }
    }
    else if (AdrNum != 0)
    {
      if (pCurrCPUProps->Family == eColdfire) WrError(ErrNum_InvAddrMode);
      else
      {
        if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
        else
        {

          WAsmCode[0] = 0xe0c0 | AdrMode | (Op << 9) | (LFlag << 8);
          CopyAdrVals(WAsmCode + 1);
          if (2 == ArgCnt)
          {
            HVal8 = EvalStrIntExpressionOffs(&ArgStr[1], !!(*ArgStr[1].Str == '#'), Int8, &ValOK);
          }
          else
          {
            HVal8 = 1;
            ValOK = True;
          }
          if ((ValOK) && (HVal8 == 1))
            CodeLen = 2 + AdrCnt;
          else
            WrStrErrorPos(ErrNum_Only1, &ArgStr[1]);
        }
      }
    }
  }
}

/* ADDQ=0 SUBQ=1 */

static void DecodeADDQSUBQ(Word Index)
{
  Byte HVal8;
  Boolean ValOK;

  if (!CheckColdSize())
    return;

  if (!ChkArgCnt(2, 2))
    return;

  DecodeAdr(&ArgStr[2], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
  if (!AdrNum)
    return;

  if ((2 == AdrNum) && (eSymbolSize8Bit == OpSize))
  {
    WrError(ErrNum_InvOpSize);
    return;
  }

  WAsmCode[0] = 0x5000 | AdrMode | (OpSize << 6) | (Index << 8);
  CopyAdrVals(WAsmCode + 1);
  FirstPassUnknown = False;
  HVal8 = EvalStrIntExpressionOffs(&ArgStr[1], !!(*ArgStr[1].Str == '#'), UInt4, &ValOK);
  if (FirstPassUnknown) HVal8 = 1;
  if ((!ValOK) || (HVal8 < 1) | (HVal8 > 8))
  {
    WrError(ErrNum_Range18);
    return;
  }

  CodeLen = 2 + AdrCnt;
  WAsmCode[0] |= (((Word) HVal8 & 7) << 9);
}

/* 0=SUBX 1=ADDX */

static void DecodeADDXSUBX(Word Index)
{
  if (CheckColdSize())
  {
    if (ChkArgCnt(2, 2))
    {
      DecodeAdr(&ArgStr[1], Mdata | Mpre);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x9100 | (OpSize << 6) | (AdrMode & 7) | (Index << 14);
        if (AdrNum == 5)
          WAsmCode[0] |= 8;
        DecodeAdr(&ArgStr[2], 1 << (AdrNum - 1));
        if (AdrNum != 0)
        {
          CodeLen = 2;
          WAsmCode[0] |= (AdrMode & 7) << 9;
        }
      }
    }
  }
}

static void DecodeCMPM(Word Index)
{
  UNUSED(Index);

  if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(2, 2)
        && CheckNoFamily(1 << eColdfire))
  {
    DecodeAdr(&ArgStr[1], Mpost);
    if (AdrNum == 4)
    {
      WAsmCode[0] = 0xb108 | (OpSize << 6) | (AdrMode & 7);
      DecodeAdr(&ArgStr[2], Mpost);
      if (AdrNum == 4)
      {
        WAsmCode[0] |= (AdrMode & 7) << 9;
        CodeLen = 2;
      }
    }
  }
}

/* 0=SUB 1=CMP 2=ADD +4=..I +8=..A */

static void DecodeADDSUBCMP(Word Index)
{
  Word Op = Index & 3, Reg;
  unsigned Variant = Index & VariantMask;
  Word DestMask, SrcMask;
  Boolean OpSizeOK;

  if (I_Variant == Variant)
    SrcMask = Mimm;
  else
    SrcMask = Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm;

  if (A_Variant == Variant)
    DestMask = Madr;
  else
  {
    DestMask = Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs;

    /* since CMP only reads operands, PC-relative addressing is also
       allowed for the second operand */

    if (mytoupper(*OpPart.Str) == 'C')
      DestMask |= Mpc | Mpcidx;
  }

  /* ColdFire ISA B ff. allows 8/16 bit operand size of CMP: */

  if (OpSize > eSymbolSize32Bit)
    OpSizeOK = False;
  else if (OpSize == eSymbolSize32Bit)
    OpSizeOK = True;
  else
    OpSizeOK = (pCurrCPUProps->Family != eColdfire)
            || ((pCurrCPUProps->CfISA >= eCfISA_B) && (Op == 1));

  if (!OpSizeOK) WrError(ErrNum_InvOpSize);
  else
  {
    if (ChkArgCnt(2, 2))
    {
      DecodeAdr(&ArgStr[2], DestMask);
      switch (AdrNum)
      {
        case 2: /* ADDA/SUBA/CMPA ? */
          if (OpSize == eSymbolSize8Bit) WrError(ErrNum_InvOpSize);
          else
          {
            WAsmCode[0] = 0x90c0 | ((AdrMode & 7) << 9) | (Op << 13);
            if (OpSize == eSymbolSize32Bit) WAsmCode[0] |= 0x100;
            DecodeAdr(&ArgStr[1], SrcMask);
            if (AdrNum != 0)
            {
              WAsmCode[0] |= AdrMode;
              CodeLen = 2 + AdrCnt;
              CopyAdrVals(WAsmCode + 1);
            }
          }
          break;

        case 1: /* ADD/SUB/CMP <ea>,Dn ? */
          WAsmCode[0] = 0x9000 | (OpSize << 6) | ((Reg = AdrMode) << 9) | (Op << 13);
          DecodeAdr(&ArgStr[1], SrcMask);
          if (AdrNum != 0)
          {
            if ((AdrNum == 11) && (Variant == I_Variant))
            {
              if (Op == 1) Op = 8;
              WAsmCode[0] = 0x400 | (OpSize << 6) | (Op << 8) | Reg;
            }
            else
              WAsmCode[0] |= AdrMode;
            CopyAdrVals(WAsmCode + 1);
            CodeLen = 2 + AdrCnt;
          }
          break;

        case 0:
          break;

        default: /* CMP/ADD/SUB <ea>, Dn */
          DecodeAdr(&ArgStr[1], Mdata | Mimm);
          if (AdrNum == 11)        /* ADDI/SUBI/CMPI ? */
          {
            /* we have to set the PC offset before we decode the destination operand.  Luckily,
               this is only needed afterwards for an immediate source operand, so we know the
               # of words ahead: */

            if (*ArgStr[1].Str == '#')
              RelPos += (OpSize == eSymbolSize32Bit) ? 4 : 2;

            if (Op == 1) Op = 8;
            WAsmCode[0] = 0x400 | (OpSize << 6) | (Op << 8);
            CodeLen = 2 + AdrCnt;
            CopyAdrVals(WAsmCode + 1);
            if (pCurrCPUProps->Family == eColdfire) DecodeAdr(&ArgStr[2], Mdata);
            else DecodeAdr(&ArgStr[2], DestMask);
            if (AdrNum != 0)
            {
              WAsmCode[0] |= AdrMode;
              CopyAdrVals(WAsmCode + (CodeLen >> 1));
              CodeLen += AdrCnt;
            }
            else
              CodeLen = 0;
          }
          else if (AdrNum != 0)    /* ADD Dn,<EA> ? */
          {
            if (Op == 1) WrError(ErrNum_InvCmpMode);
            else
            {
              WAsmCode[0] = 0x9100 | (OpSize << 6) | (AdrMode << 9) | (Op << 13);
              DecodeAdr(&ArgStr[2], DestMask);
              if (AdrNum != 0)
              {
                CodeLen = 2 + AdrCnt; CopyAdrVals(WAsmCode + 1);
                WAsmCode[0] |= AdrMode;
              }
            }
          }
      }
    }
  }
}

/* 0=OR 1=AND +4=..I */

static void DecodeANDOR(Word Index)
{
  Word Op = Index & 3, Reg;
  char Variant = Index & VariantMask;

  if (!ChkArgCnt(2, 2));
  else if (CheckColdSize())
  {
    if ((as_strcasecmp(ArgStr[2].Str, "CCR")) && (as_strcasecmp(ArgStr[2].Str, "SR")))
      DecodeAdr(&ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (!as_strcasecmp(ArgStr[2].Str, "CCR"))     /* AND #...,CCR */
    {
      if (*AttrPart.Str && (OpSize != eSymbolSize8Bit)) WrError(ErrNum_InvOpSize);
      else if (!(pCurrCPUProps->SuppFlags & eFlagLogCCR)) WrError(ErrNum_InstructionNotSupported);
      {
        WAsmCode[0] = 0x003c | (Op << 9);
        OpSize = eSymbolSize8Bit;
        DecodeAdr(&ArgStr[1], Mimm);
        if (AdrNum != 0)
        {
          CodeLen = 4;
          WAsmCode[1] = AdrVals[0];
        }
      }
    }
    else if (!as_strcasecmp(ArgStr[2].Str, "SR")) /* AND #...,SR */
    {
      if (*AttrPart.Str && (OpSize != eSymbolSize16Bit)) WrError(ErrNum_InvOpSize);
      else if (CheckNoFamily(1 << eColdfire))
      {
        WAsmCode[0] = 0x007c | (Op << 9);
        OpSize = eSymbolSize16Bit;
        DecodeAdr(&ArgStr[1], Mimm);
        if (AdrNum != 0)
        {
          CodeLen = 4;
          WAsmCode[1] = AdrVals[0];
          CheckSup();
        }
      }
    }
    else if (AdrNum == 1)                 /* AND <EA>,Dn */
    {
      WAsmCode[0] = 0x8000 | (OpSize << 6) | ((Reg = AdrMode) << 9) | (Op << 14);
      DecodeAdr(&ArgStr[1], ((Variant == I_Variant) ? 0 : Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs) | Mimm);
      if (AdrNum != 0)
      {
        if ((AdrNum == 11) && (Variant == I_Variant))
          WAsmCode[0] = (OpSize << 6) | (Op << 9) | Reg;
        else
          WAsmCode[0] |= AdrMode;
        CodeLen = 2 + AdrCnt;
        CopyAdrVals(WAsmCode + 1);
      }
    }
    else if (AdrNum != 0)                 /* AND ...,<EA> */
    {
      DecodeAdr(&ArgStr[1], Mdata | Mimm);
      if (AdrNum == 11)                   /* AND #..,<EA> */
      {
        WAsmCode[0] = (OpSize << 6) | (Op << 9);
        CodeLen = 2 + AdrCnt;
        CopyAdrVals(WAsmCode + 1);
        DecodeAdr(&ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
        if (AdrNum != 0)
        {
          WAsmCode[0] |= AdrMode;
          CopyAdrVals(WAsmCode + (CodeLen >> 1));
          CodeLen += AdrCnt;
        }
        else
          CodeLen = 0;
      }
      else if (AdrNum != 0)               /* AND Dn,<EA> ? */
      {
        WAsmCode[0] = 0x8100 | (OpSize << 6) | (AdrMode << 9) | (Op << 14);
        DecodeAdr(&ArgStr[2], Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
        if (AdrNum != 0)
        {
          CodeLen = 2 + AdrCnt;
          CopyAdrVals(WAsmCode + 1);
          WAsmCode[0] |= AdrMode;
        }
      }
    }
  }
}

/* 0=EOR 4=EORI */

static void DecodeEOR(Word Index)
{
  unsigned Variant = Index | VariantMask;

  if (!ChkArgCnt(2, 2));
  else if (!as_strcasecmp(ArgStr[2].Str, "CCR"))
  {
    if (*AttrPart.Str && (OpSize != eSymbolSize8Bit)) WrError(ErrNum_InvOpSize);
    else if (CheckNoFamily(1 << eColdfire))
    {
      WAsmCode[0] = 0xa3c;
      OpSize = eSymbolSize8Bit;
      DecodeAdr(&ArgStr[1], Mimm);
      if (AdrNum != 0)
      {
        CodeLen = 4;
        WAsmCode[1] = AdrVals[0];
      }
    }
  }
  else if (!as_strcasecmp(ArgStr[2].Str, "SR"))
  {
    if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
    else if (CheckNoFamily(1 << eColdfire))
    {
      WAsmCode[0] = 0xa7c;
      DecodeAdr(&ArgStr[1], Mimm);
      if (AdrNum != 0)
      {
        CodeLen = 4;
        WAsmCode[1] = AdrVals[0];
        CheckSup();
      }
    }
  }
  else if (CheckColdSize())
  {
    DecodeAdr(&ArgStr[1], ((Variant == I_Variant) ? 0 : Mdata) | Mimm);
    if (AdrNum == 1)
    {
      WAsmCode[0] = 0xb100 | (AdrMode << 9) | (OpSize << 6);
      DecodeAdr(&ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        CopyAdrVals(WAsmCode + 1);
        WAsmCode[0] |= AdrMode;
      }
    }
    else if (AdrNum == 11)
    {
      WAsmCode[0] = 0x0a00 | (OpSize << 6);
      CopyAdrVals(WAsmCode + 1);
      CodeLen = 2 + AdrCnt;
      DecodeAdr(&ArgStr[2], Mdata | ((pCurrCPUProps->Family == eColdfire) ? 0 : Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs));
      if (AdrNum != 0)
      {
        CopyAdrVals(WAsmCode + (CodeLen >> 1));
        CodeLen += AdrCnt;
        WAsmCode[0] |= AdrMode;
      }
      else CodeLen = 0;
    }
  }
}

static void DecodePEA(Word Index)
{
  UNUSED(Index);

  if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_UseLessAttr);
  else if (ChkArgCnt(1, 1))
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(&ArgStr[1], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = 0x4840 | AdrMode;
      CopyAdrVals(WAsmCode + 1);
    }
  }
}

/* 0=CLR 1=TST */

static void DecodeCLRTST(Word Index)
{
  if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(1, 1))
  {
    Word w1 = Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs;

    switch (pCurrCPUProps->Family)
    {
      case eCPU32:
      case e68KGen2:
      case e68KGen3:
        w1 |= Mpc | Mpcidx | Mimm;
        if (OpSize != eSymbolSize8Bit)
          w1 |= Madr;
      default:
        break;
    }
    DecodeAdr(&ArgStr[1], w1);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = 0x4200 | (Index << 11) | (OpSize << 6) | AdrMode;
      CopyAdrVals(WAsmCode + 1);
    }
  }
}

/* 0=JSR 1=JMP */

static void DecodeJSRJMP(Word Index)
{
  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = 0x4e80 | (Index << 6) | AdrMode;
      CopyAdrVals(WAsmCode + 1);
    }
  }
}

/* 0=TAS 1=NBCD */

static void DecodeNBCDTAS(Word Index)
{
  Boolean Allowed;

  /* TAS is allowed on ColdFire ISA B ff. ... */

  if (pCurrCPUProps->Family != eColdfire)
    Allowed = True;
  else
    Allowed = Index ? False : (pCurrCPUProps->CfISA >= eCfISA_B);

  if (*AttrPart.Str && (OpSize != eSymbolSize8Bit)) WrError(ErrNum_InvOpSize);
  else if (!Allowed) WrError(ErrNum_InstructionNotSupported);
  else if (ChkArgCnt(1, 1))
  {
    OpSize = eSymbolSize8Bit;

    /* ...but not on data register: */

    DecodeAdr(&ArgStr[1], ((pCurrCPUProps->Family == eColdfire) ? 0 : Mdata) | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = (Index == 1) ? 0x4800 : 0x4ac0;
      WAsmCode[0] |= AdrMode;
      CopyAdrVals(WAsmCode + 1);
    }
  }
}

/* 0=NEGX 2=NEG 3=NOT */

static void DecodeNEGNOT(Word Index)
{
  if (ChkArgCnt(1, 1)
   && CheckColdSize())
  {
    if (pCurrCPUProps->Family == eColdfire) DecodeAdr(&ArgStr[1], Mdata);
    else DecodeAdr(&ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = 0x4000 | (Index << 9) | (OpSize << 6) | AdrMode;
      CopyAdrVals(WAsmCode + 1);
    }
  }
}

static void DecodeSWAP(Word Index)
{
  UNUSED(Index);

  if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], Mdata);
    if (AdrNum != 0)
    {
      CodeLen = 2;
      WAsmCode[0] = 0x4840 | AdrMode;
    }
  }
}

static void DecodeUNLK(Word Index)
{
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], Madr);
    if (AdrNum != 0)
    {
      CodeLen = 2;
      WAsmCode[0] = 0x4e58 | AdrMode;
    }
  }
}

static void DecodeEXT(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if ((OpSize == eSymbolSize8Bit) || (OpSize > eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else
  {
    DecodeAdr(&ArgStr[1], Mdata);
    if (AdrNum == 1)
    {
      WAsmCode[0] = 0x4880 | AdrMode | (((Word)OpSize - 1) << 6);
      CodeLen = 2;
    }
  }
}

static void DecodeWDDATA(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
  else if (CheckFamily(1 << eColdfire))
  {
    DecodeAdr(&ArgStr[1], Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf400 + (OpSize << 6) + AdrMode;
      CopyAdrVals(WAsmCode + 1);
      CodeLen = 2 + AdrCnt;
      CheckSup();
    }
  }
}

static void DecodeWDEBUG(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(1, 1)
   && CheckFamily(1 << eColdfire)
   && CheckColdSize())
  {
    DecodeAdr(&ArgStr[1], Madri | Mdadri);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xfbc0 + AdrMode;
      WAsmCode[1] = 0x0003;
      CopyAdrVals(WAsmCode + 2);
      CodeLen = 4 + AdrCnt;
      CheckSup();
    }
  }
}

static void DecodeFixed(Word Index)
{
  FixedOrder *FixedZ = FixedOrders + Index;

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (ChkArgCnt(0, 0)
        && CheckFamily(FixedZ->FamilyMask))
  {
    CodeLen = 2;
    WAsmCode[0] = FixedZ->Code;
    if (FixedZ->MustSup)
      CheckSup();
  }
}

static void DecodeMOVEM(Word Index)
{
  int z;
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if ((OpSize < eSymbolSize16Bit) || (OpSize > eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if ((pCurrCPUProps->Family == eColdfire) && (OpSize == 1)) WrError(ErrNum_InvOpSize);
  else
  {
    RelPos = 4;
    if (DecodeRegList(ArgStr[2].Str, WAsmCode + 1))
    {
      DecodeAdr(&ArgStr[1], Madri | Mdadri | ((pCurrCPUProps->Family == eColdfire) ? 0 : Mpost | Maix | Mpc | Mpcidx | Mabs));
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x4c80 | AdrMode | ((OpSize - 1) << 6);
        CodeLen = 4 + AdrCnt; CopyAdrVals(WAsmCode + 2);
      }
    }
    else if (DecodeRegList(ArgStr[1].Str, WAsmCode + 1))
    {
      DecodeAdr(&ArgStr[2], Madri | Mdadri  | ((pCurrCPUProps->Family == eColdfire) ? 0 : Mpre | Maix | Mabs));
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x4880 | AdrMode | ((OpSize - 1) << 6);
        CodeLen = 4 + AdrCnt; CopyAdrVals(WAsmCode + 2);
        if (AdrNum == 5)
        {
          Word Tmp = WAsmCode[1];

          WAsmCode[1] = 0;
          for (z = 0; z < 16; z++)
          {
            WAsmCode[1] = (WAsmCode[1] << 1) + (Tmp & 1);
            Tmp >>= 1;
          }
        }
      }
    }
    else WrError(ErrNum_InvRegList);
  }
}

static void DecodeMOVEQ(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if ((*AttrPart.Str) && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (*ArgStr[1].Str != '#') WrStrErrorPos(ErrNum_OnlyImmAddr, &ArgStr[1]);
  else
  {
    DecodeAdr(&ArgStr[2], Mdata);
    if (AdrNum != 0)
    {
      Boolean OK;
      LongWord Value;

      WAsmCode[0] = 0x7000 | (AdrMode << 9);
      FirstPassUnknown = False;
      Value = EvalStrIntExpressionOffs(&ArgStr[1], 1, Int32, &OK);
      if (FirstPassUnknown)
        Value &= 0x7f;
      if ((Value > 0x7f) && (Value < 0xffffff80ul))
        WrStrErrorPos((Value & 0x80000000ul) ? ErrNum_UnderRange : ErrNum_OverRange, &ArgStr[1]);
      else
      {
        CodeLen = 2;
        WAsmCode[0] |= Value & 0xff;
      }
    }
  }
}

static void DecodeSTOP(Word Index)
{
  Word HVal;
  Boolean ValOK;
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!ChkArgCnt(1, 1));
  else if (*ArgStr[1].Str != '#') WrError(ErrNum_OnlyImmAddr);
  else
  {
    HVal = EvalStrIntExpressionOffs(&ArgStr[1], 1, Int16, &ValOK);
    if (ValOK)
    {
      CodeLen = 4;
      WAsmCode[0] = 0x4e72;
      WAsmCode[1] = HVal;
      CheckSup();
    }
  }
}

static void DecodeLPSTOP(Word Index)
{
  Word HVal;
  Boolean ValOK;
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!ChkArgCnt(1, 1));
  else if (!CheckFamily(1 << eCPU32));
  else if (*ArgStr[1].Str != '#') WrError(ErrNum_OnlyImmAddr);
  else
  {
    HVal = EvalStrIntExpressionOffs(&ArgStr[1], 1, Int16, &ValOK);
    if (ValOK)
    {
      CodeLen = 6;
      WAsmCode[0] = 0xf800;
      WAsmCode[1] = 0x01c0;
      WAsmCode[2] = HVal;
      CheckSup();
    }
  }
}

static void DecodeTRAP(Word Index)
{
  Byte HVal8;
  Boolean ValOK;
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!ChkArgCnt(1, 1));
  else if (*ArgStr[1].Str != '#') WrError(ErrNum_OnlyImmAddr);
  else
  {
    HVal8 = EvalStrIntExpressionOffs(&ArgStr[1], 1, Int4, &ValOK);
    if (ValOK)
    {
      CodeLen = 2;
      WAsmCode[0] = 0x4e40 + (HVal8 & 15);
    }
  }
}

static void DecodeBKPT(Word Index)
{
  Byte HVal8;
  Boolean ValOK;
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!ChkArgCnt(1, 1));
  else if (!CheckNoFamily((1 << e68KGen1a) | (1 << eColdfire)));
  else if (*ArgStr[1].Str != '#') WrError(ErrNum_OnlyImmAddr);
  else
  {
    HVal8 = EvalStrIntExpressionOffs(&ArgStr[1], 1, UInt3, &ValOK);
    if (ValOK)
    {
      CodeLen = 2;
      WAsmCode[0] = 0x4848 + (HVal8 & 7);
    }
  }
  UNUSED(Index);
}

static void DecodeRTD(Word Index)
{
  Word HVal;
  Boolean ValOK;
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!ChkArgCnt(1, 1));
  else if (!CheckNoFamily((1 << e68KGen1a) | (1 << eColdfire)));
  else if (*ArgStr[1].Str != '#') WrError(ErrNum_OnlyImmAddr);
  else
  {
    HVal = EvalStrIntExpressionOffs(&ArgStr[1], 1, Int16, &ValOK);
    if (ValOK)
    {
      CodeLen = 4;
      WAsmCode[0] = 0x4e74;
      WAsmCode[1] = HVal;
    }
  }
}

static void DecodeEXG(Word Index)
{
  Word HReg;
  UNUSED(Index);

  if ((*AttrPart.Str) && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(2, 2)
        && CheckNoFamily(1 << eColdfire))
  {
    DecodeAdr(&ArgStr[1], Mdata | Madr);
    if (AdrNum == 1)
    {
      WAsmCode[0] = 0xc100 | (AdrMode << 9);
      DecodeAdr(&ArgStr[2], Mdata | Madr);
      if (AdrNum == 1)
      {
        WAsmCode[0] |= 0x40 | AdrMode;
        CodeLen = 2;
      }
      else if (AdrNum == 2)
      {
        WAsmCode[0] |= 0x88 | (AdrMode & 7);
        CodeLen = 2;
      }
    }
    else if (AdrNum == 2)
    {
      WAsmCode[0] = 0xc100;
      HReg = AdrMode & 7;
      DecodeAdr(&ArgStr[2], Mdata | Madr);
      if (AdrNum == 1)
      {
        WAsmCode[0] |= 0x88 | (AdrMode << 9) | HReg;
        CodeLen = 2;
      }
      else
      {
        WAsmCode[0] |= 0x48 | (HReg << 9) | (AdrMode & 7);
        CodeLen = 2;
      }
    }
  }
}

static void DecodeMOVE16(Word Index)
{
  Word z, z2, w1, w2;
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (ChkArgCnt(2, 2)
        && CheckFamily(1 << e68KGen3))
  {
    DecodeAdr(&ArgStr[1], Mpost | Madri | Mabs);
    if (AdrNum != 0)
    {
      w1 = AdrNum;
      z = AdrMode & 7;
      if ((w1 == 10) && (AdrCnt == 2))
      {
        AdrVals[1] = AdrVals[0];
        AdrVals[0] = 0 - (AdrVals[1] >> 15);
      }
      DecodeAdr(&ArgStr[2], Mpost | Madri | Mabs);
      if (AdrNum != 0)
      {
        w2 = AdrNum;
        z2 = AdrMode & 7;
        if ((w2 == 10) && (AdrCnt == 2))
        {
          AdrVals[1] = AdrVals[0];
          AdrVals[0] = 0 - (AdrVals[1] >> 15);
        }
        if ((w1 == 4) && (w2 == 4))
        {
          WAsmCode[0] = 0xf620 + z;
          WAsmCode[1] = 0x8000 + (z2 << 12);
          CodeLen = 4;
        }
        else
        {
          WAsmCode[1] = AdrVals[0];
          WAsmCode[2] = AdrVals[1];
          CodeLen = 6;
          if ((w1 == 4) && (w2 == 10))
            WAsmCode[0] = 0xf600 + z;
          else if ((w1 == 10) && (w2 == 4))
            WAsmCode[0] = 0xf608 + z2;
          else if ((w1 == 3) && (w2 == 10))
            WAsmCode[0] = 0xf610 + z;
          else if ((w1 == 10) && (w2 == 3))
            WAsmCode[0] = 0xf618 + z2;
          else
          {
            WrError(ErrNum_InvAddrMode);
            CodeLen = 0;
          }
        }
      }
    }
  }
}

static void DecodeCacheAll(Word Index)
{
  Word w1;

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!ChkArgCnt(1, 1));
  else if (!CheckFamily(1 << e68KGen3));
  else if (!CodeCache(ArgStr[1].Str, &w1)) WrStrErrorPos(ErrNum_InvCtrlReg, &ArgStr[1]);
  else
  {
    WAsmCode[0] = 0xf418 + (w1 << 6) + (Index << 5);
    CodeLen = 2;
    CheckSup();
  }
}

static void DecodeCache(Word Index)
{
  Word w1;

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!ChkArgCnt(2, 2));
  else if (!CheckFamily(1 << e68KGen3));
  else if (!CodeCache(ArgStr[1].Str, &w1)) WrStrErrorPos(ErrNum_InvCtrlReg, &ArgStr[1]);
  else
  {
    DecodeAdr(&ArgStr[2], Madri);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf400 + (w1 << 6) + (Index << 3) + (AdrMode & 7);
      CodeLen = 2;
      CheckSup();
    }
  }
}

static void DecodeMUL_DIV(Word Code)
{

  if (!ChkArgCnt(2, 2));
  else if ((*OpPart.Str == 'D') && !CheckNoFamily(1 << eColdfire));
  else if (OpSize == eSymbolSize16Bit)
  {
    DecodeAdr(&ArgStr[2], Mdata);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x80c0 | (AdrMode << 9) | (Code & 0x0100);
      if (!(Code & 1))
        WAsmCode[0] |= 0x4000;
      DecodeAdr(&ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
      if (AdrNum != 0)
      {
        WAsmCode[0] |= AdrMode;
        CodeLen = 2 + AdrCnt; CopyAdrVals(WAsmCode + 1);
      }
    }
  }
  else if (OpSize == eSymbolSize32Bit)
  {
    Word w1, w2;
    Boolean OK;

    if (strchr(ArgStr[2].Str, ':'))
      OK = CodeRegPair(ArgStr[2].Str, &w1, &w2);
    else
    {
      OK = CodeReg(ArgStr[2].Str, &w1) && (w1 < 8);
      w2 = w1;
    }
    if (!OK) WrStrErrorPos(ErrNum_InvRegPair, &ArgStr[2]);
    else
    {
      WAsmCode[1] = w1 | (w2 << 12) | ((Code & 0x0100) << 3);
      RelPos = 4;
      if (w1 != w2)
        WAsmCode[1] |= 0x400;
      DecodeAdr(&ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x4c00 + AdrMode + (Lo(Code) << 6);
        CopyAdrVals(WAsmCode + 2);
        CodeLen = 4 + AdrCnt;
        CheckFamily((1 << e68KGen3) | (1 << e68KGen2) | (1 << eCPU32) | ((w1 == w2) ? (1 << eColdfire) : 0));
      }
    }
  }
  else
    WrError(ErrNum_InvOpSize);
}

static void DecodeDIVL(Word Index)
{
  Word w1, w2;

  if (!*AttrPart.Str)
    OpSize = eSymbolSize32Bit;
  if (!ChkArgCnt(2, 2));
  else if (!CheckFamily((1 << e68KGen3) | (1 << e68KGen2) | (1 << eCPU32)));
  else if (OpSize != eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
  else if (!CodeRegPair(ArgStr[2].Str, &w1, &w2)) WrStrErrorPos(ErrNum_InvRegPair, &ArgStr[2]);
  else
  {
    RelPos = 4;
    WAsmCode[1] = w1 | (w2 << 12) | (Index << 11);
    DecodeAdr(&ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x4c40 + AdrMode;
      CopyAdrVals(WAsmCode + 2);
      CodeLen = 4 + AdrCnt;
    }
  }
}

static void DecodeASBCD(Word Index)
{
  if ((OpSize != eSymbolSize8Bit) && *AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(2, 2)
        && CheckNoFamily(1 << eColdfire))
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(&ArgStr[1], Mdata | Mpre);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x8100 | (AdrMode & 7) | (Index << 14) | ((AdrNum == 5) ? 8 : 0);
      DecodeAdr(&ArgStr[2], 1 << (AdrNum - 1));
      if (AdrNum != 0)
      {
        CodeLen = 2;
        WAsmCode[0] |= (AdrMode & 7) << 9;
      }
    }
  }
}

static void DecodeCHK(Word Index)
{
  UNUSED(Index);

  if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(2, 2)
        && CheckNoFamily(1 << eColdfire))
  {
    DecodeAdr(&ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x4000 | AdrMode | ((4 - OpSize) << 7);
      CodeLen = 2 + AdrCnt;
      CopyAdrVals(WAsmCode + 1);
      DecodeAdr(&ArgStr[2], Mdata);
      if (AdrNum == 1)
        WAsmCode[0] |= WAsmCode[0] | (AdrMode << 9);
      else
        CodeLen = 0;
    }
  }
}

static void DecodeLINK(Word Index)
{
  UNUSED(Index);

  if (!*AttrPart.Str && (pCurrCPUProps->Family == eColdfire)) OpSize = eSymbolSize16Bit;
  if ((OpSize < 1) || (OpSize > 2)) WrError(ErrNum_InvOpSize);
  else if ((OpSize == eSymbolSize32Bit) && !CheckFamily((1 << eCPU32) | (1 << e68KGen2) | (1 << e68KGen3)));
  else if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], Madr);
    if (AdrNum != 0)
    {
      WAsmCode[0] = (OpSize == eSymbolSize16Bit) ? 0x4e50 : 0x4808;
      WAsmCode[0] += AdrMode & 7;
      DecodeAdr(&ArgStr[2], Mimm);
      if (AdrNum == 11)
      {
        CodeLen = 2 + AdrCnt;
        memcpy(WAsmCode + 1, AdrVals, AdrCnt);
      }
    }
  }
}

static void DecodeMOVEP(Word Index)
{
  UNUSED(Index);

  if ((OpSize == eSymbolSize8Bit) || (OpSize > eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(2, 2)
        && CheckNoFamily(1 << eColdfire))
  {
    DecodeAdr(&ArgStr[1], Mdata | Mdadri);
    if (AdrNum == 1)
    {
      WAsmCode[0] = 0x188 | ((OpSize - 1) << 6) | (AdrMode << 9);
      DecodeAdr(&ArgStr[2], Mdadri);
      if (AdrNum == 6)
      {
        WAsmCode[0] |= AdrMode & 7;
        CodeLen = 4;
        WAsmCode[1] = AdrVals[0];
      }
    }
    else if (AdrNum == 6)
    {
      WAsmCode[0] = 0x108 | ((OpSize - 1) << 6) | (AdrMode & 7);
      WAsmCode[1] = AdrVals[0];
      DecodeAdr(&ArgStr[2], Mdata);
      if (AdrNum == 1)
      {
        WAsmCode[0] |= (AdrMode & 7) << 9;
        CodeLen = 4;
      }
    }
  }
}

static void DecodeMOVEC(Word Index)
{
  UNUSED(Index);

  if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(2, 2))
  {
    if (DecodeCtrlReg(ArgStr[1].Str, WAsmCode + 1))
    {
      DecodeAdr(&ArgStr[2], Mdata | Madr);
      if (AdrNum != 0)
      {
        CodeLen = 4;
        WAsmCode[0] = 0x4e7a;
        WAsmCode[1] |= AdrMode << 12;
        CheckSup();
      }
    }
    else if (DecodeCtrlReg(ArgStr[2].Str, WAsmCode + 1))
    {
      DecodeAdr(&ArgStr[1], Mdata | Madr);
      if (AdrNum != 0)
      {
        CodeLen = 4;
        WAsmCode[0] = 0x4e7b;
        WAsmCode[1] |= AdrMode << 12; CheckSup();
      }
    }
    else
      WrError(ErrNum_InvCtrlReg);
  }
}

static void DecodeMOVES(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
  else if (CheckNoFamily((1 << e68KGen1a) | (1 << eColdfire)))
  {
    DecodeAdr(&ArgStr[1], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if ((AdrNum == 1) || (AdrNum == 2))
    {
      WAsmCode[1] = 0x800 | (AdrMode << 12);
      DecodeAdr(&ArgStr[2], Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0xe00 | AdrMode | (OpSize << 6);
        CodeLen = 4 + AdrCnt;
        CopyAdrVals(WAsmCode + 2);
        CheckSup();
      }
    }
    else if (AdrNum != 0)
    {
      WAsmCode[0] = 0xe00 | AdrMode | (OpSize << 6);
      CodeLen = 4 + AdrCnt;
      CopyAdrVals(WAsmCode + 2);
      DecodeAdr(&ArgStr[2], Mdata | Madr);
      if (AdrNum != 0)
      {
        WAsmCode[1] = AdrMode << 12;
        CheckSup();
      }
      else
        CodeLen = 0;
    }
  }
}

static void DecodeCALLM(Word Index)
{
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!(pCurrCPUProps->SuppFlags & eFlagCALLM_RTM)) WrError(ErrNum_InstructionNotSupported);
  else if (ChkArgCnt(2, 2))
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(&ArgStr[1], Mimm);
    if (AdrNum != 0)
    {
      WAsmCode[1] = AdrVals[0];
      RelPos = 4;
      DecodeAdr(&ArgStr[2], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x06c0 + AdrMode;
        CopyAdrVals(WAsmCode + 2);
        CodeLen = 4 + AdrCnt;
      }
    }
  }
}

static void DecodeCAS(Word Index)
{
  UNUSED(Index);

  if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(3, 3)
        && CheckFamily((1 << e68KGen3) | (1 << e68KGen2)))
  {
    DecodeAdr(&ArgStr[1], Mdata);
    if (AdrNum != 0)
    {
      WAsmCode[1] = AdrMode;
      DecodeAdr(&ArgStr[2], Mdata);
      if (AdrNum != 0)
      {
        RelPos = 4;
        WAsmCode[1] += (((Word)AdrMode) << 6);
        DecodeAdr(&ArgStr[3], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
        if (AdrNum != 0)
        {
          WAsmCode[0] = 0x08c0 + AdrMode + (((Word)OpSize + 1) << 9);
          CopyAdrVals(WAsmCode + 2);
          CodeLen = 4 + AdrCnt;
        }
      }
    }
  }
}

static void DecodeCAS2(Word Index)
{
  Word w1, w2;
  UNUSED(Index);

  if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (!ChkArgCnt(3, 3));
  else if (!CheckFamily((1 << e68KGen3) | (1 << e68KGen2)));
  else if (!CodeRegPair(ArgStr[1].Str, WAsmCode + 1, WAsmCode + 2)) WrStrErrorPos(ErrNum_InvRegPair, &ArgStr[1]);
  else if (!CodeRegPair(ArgStr[2].Str, &w1, &w2)) WrStrErrorPos(ErrNum_InvRegPair, &ArgStr[2]);
  else
  {
    WAsmCode[1] += (w1 << 6);
    WAsmCode[2] += (w2 << 6);
    if (!CodeIndRegPair(ArgStr[3].Str, &w1, &w2)) WrStrErrorPos(ErrNum_InvRegPair, &ArgStr[3]);
    else
    {
      WAsmCode[1] += (w1 << 12);
      WAsmCode[2] += (w2 << 12);
      WAsmCode[0] = 0x0cfc + (((Word)OpSize - 1) << 9);
      CodeLen = 6;
    }
  }
}

static void DecodeCMPCHK2(Word Index)
{
  if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
  else if (!CheckFamily((1 << e68KGen3) | (1 << e68KGen2) | (1 << eCPU32)));
  else if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[2], Mdata | Madr);
    if (AdrNum != 0)
    {
      RelPos = 4;
      WAsmCode[1] = (((Word)AdrMode) << 12) | (Index << 11);
      DecodeAdr(&ArgStr[1], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x00c0 + (((Word)OpSize) << 9) + AdrMode;
        CopyAdrVals(WAsmCode + 2);
        CodeLen = 4 + AdrCnt;
      }
    }
  }
}

static void DecodeEXTB(Word Index)
{
  UNUSED(Index);

  if ((OpSize != eSymbolSize32Bit) && *AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!CheckFamily((1 << e68KGen3) | (1 << e68KGen2) | (1 << eCPU32)));
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], Mdata);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x49c0 + AdrMode;
      CodeLen = 2;
    }
  }
}

static void DecodePACK(Word Index)
{
  if (!ChkArgCnt(3, 3));
  else if (!CheckFamily((1 << e68KGen3) | (1 << e68KGen2)));
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else
  {
    DecodeAdr(&ArgStr[1], Mdata | Mpre);
    if (AdrNum != 0)
    {
      WAsmCode[0] = (0x8140 + (Index << 6)) | (AdrMode & 7);
      if (AdrNum == 5)
        WAsmCode[0] += 8;
      DecodeAdr(&ArgStr[2], 1 << (AdrNum - 1));
      if (AdrNum != 0)
      {
        WAsmCode[0] |= ((AdrMode & 7) << 9);
        DecodeAdr(&ArgStr[3], Mimm);
        if (AdrNum != 0)
        {
          WAsmCode[1] = AdrVals[0];
          CodeLen = 4;
        }
      }
    }
  }
}

static void DecodeRTM(Word Index)
{
  UNUSED(Index);

  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!(pCurrCPUProps->SuppFlags & eFlagCALLM_RTM)) WrError(ErrNum_InstructionNotSupported);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], Mdata | Madr);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x06c0 + AdrMode;
      CodeLen = 2;
    }
  }
}

static void DecodeTBL(Word Index)
{
  char *p;
  Word w2, Mode;

  if (!ChkArgCnt(2, 2));
  else if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
  else if (CheckFamily(1 << eCPU32))
  {
    DecodeAdr(&ArgStr[2], Mdata);
    if (AdrNum != 0)
    {
      Mode = AdrMode;
      p = strchr(ArgStr[1].Str, ':');
      if (!p)
      {
        RelPos = 4;
        DecodeAdr(&ArgStr[1], Madri | Mdadri | Maix| Mabs | Mpc | Mpcidx);
        if (AdrNum != 0)
        {
          WAsmCode[0] = 0xf800 | AdrMode;
          WAsmCode[1] = 0x0100 | (OpSize << 6) | (Mode << 12) | (Index << 10);
          memcpy(WAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 4 + AdrCnt;
        }
      }
      else
      {
        strcpy(ArgStr[3].Str, p + 1);
        *p = '\0';
        DecodeAdr(&ArgStr[1], Mdata);
        if (AdrNum != 0)
        {
          w2 = AdrMode;
          DecodeAdr(&ArgStr[3], Mdata);
          if (AdrNum != 0)
          {
            WAsmCode[0] = 0xf800 | w2;
            WAsmCode[1] = 0x0000 | (OpSize << 6) | (Mode << 12) | AdrMode;
            if (OpPart.Str[3] == 'S')
              WAsmCode[1] |= 0x0800;
            if (OpPart.Str[strlen(OpPart.Str) - 1] == 'N')
              WAsmCode[1] |= 0x0400;
            CodeLen = 4;
          }
        }
      }
    }
  }
}

/* 0=BTST 1=BCHG 2=BCLR 3=BSET */

static void DecodeBits(Word Index)
{
  Word Mask, BitNum, BitMax;
  ShortInt SaveOpSize;
  unsigned ResCodeLen;
  Boolean BitNumUnknown = False;

  if (!ChkArgCnt(2, 2))
    return;

  WAsmCode[0] = (Index << 6);
  ResCodeLen = 1;

  SaveOpSize = OpSize;
  OpSize = eSymbolSize8Bit;
  DecodeAdr(&ArgStr[1], Mdata | Mimm);
  switch (AdrNum)
  {
    case 1:
      WAsmCode[0] |= 0x100 | (AdrMode << 9);
      BitNum = 0; /* implicitly suppresses bit pos check */
      break;
    case 11:
      WAsmCode[0] |= 0x800;
      WAsmCode[ResCodeLen++] = BitNum = AdrVals[0];
      BitNumUnknown = FirstPassUnknown;
      break;
    default:
      return;
  }

  OpSize = SaveOpSize;
  if (!*AttrPart.Str)
    OpSize = eSymbolSize8Bit;

  Mask = Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs;
  if (!Index)
    Mask |= Mpc | Mpcidx | Mimm;
  RelPos = ResCodeLen << 1;
  DecodeAdr(&ArgStr[2], Mask);

  if (!*AttrPart.Str)
    OpSize = (AdrNum == 1) ? eSymbolSize32Bit : eSymbolSize8Bit;
  if (!AdrNum)
    return;
  if (((AdrNum == 1) && (OpSize != eSymbolSize32Bit)) || ((AdrNum != 1) && (OpSize != eSymbolSize8Bit)))
  {
    WrError(ErrNum_InvOpSize);
    return;
  }

  BitMax = (AdrNum == 1) ? 31 : 7;
  WAsmCode[0] |= AdrMode;
  CopyAdrVals(WAsmCode + ResCodeLen);
  CodeLen = (ResCodeLen << 1) + AdrCnt;
  if (!BitNumUnknown && (BitNum > BitMax))
    WrError(ErrNum_BitNumberTruncated);
}

/* 0=BFTST 1=BFCHG 2=BFCLR 3=BFSET */

static void DecodeFBits(Word Index)
{
  if (!ChkArgCnt(1, 1));
  else if (!CheckFamily((1 << e68KGen3) | (1 << e68KGen2)));
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!SplitBitField(&ArgStr[1], WAsmCode + 1)) WrError(ErrNum_InvBitMask);
  else
  {
    RelPos = 4;
    OpSize = eSymbolSize8Bit;
    if (Memo("BFTST")) DecodeAdr(&ArgStr[1], Mdata | Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
    else DecodeAdr(&ArgStr[1], Mdata | Madri | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xe8c0 | AdrMode | (Index << 10);
      CopyAdrVals(WAsmCode + 2);
      CodeLen = 4 + AdrCnt;
    }
  }
}

/* 0=BFEXTU 1=BFEXTS 2=BFFFO */

static void DecodeEBits(Word Index)
{
  if (!ChkArgCnt(2, 2));
  else if (!CheckFamily((1 << e68KGen3) | (1 << e68KGen2)));
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!SplitBitField(&ArgStr[1], WAsmCode + 1)) WrError(ErrNum_InvBitMask);
  else
  {
    RelPos = 4;
    OpSize = eSymbolSize8Bit;
    DecodeAdr(&ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs);
    if (AdrNum != 0)
    {
      LongInt ThisCodeLen = 4 + AdrCnt;

      WAsmCode[0] = 0xe9c0 + AdrMode + (Index << 9); CopyAdrVals(WAsmCode + 2);
      DecodeAdr(&ArgStr[2], Mdata);
      if (AdrNum != 0)
      {
        WAsmCode[1] |= AdrMode << 12;
        CodeLen = ThisCodeLen;
      }
    }
  }
}

static void DecodeBFINS(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (!CheckFamily((1 << e68KGen3) | (1 << e68KGen2)));
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!SplitBitField(&ArgStr[2], WAsmCode + 1)) WrError(ErrNum_InvBitMask);
  else
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(&ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      LongInt ThisCodeLen = 4 + AdrCnt;

      WAsmCode[0] = 0xefc0 + AdrMode;
      CopyAdrVals(WAsmCode + 2);
      DecodeAdr(&ArgStr[1], Mdata);
      if (AdrNum != 0)
      {
        WAsmCode[1] |= AdrMode << 12;
        CodeLen = ThisCodeLen;
      }
    }
  }
}

/* bedingte Befehle */

static void DecodeBcc(Word CondCode)
{
  /* .W, .S, .L, .X erlaubt */

  if ((OpSize > eSymbolSize32Bit) && (OpSize != eSymbolSizeFloat32Bit) && (OpSize != eSymbolSizeFloat96Bit)) WrError(ErrNum_InvOpSize);

  /* nur ein Operand erlaubt */

  else if (ChkArgCnt(1, 1))
  {
    LongInt HVal;
    Integer HVal16;
    ShortInt HVal8;
    Boolean ValOK, IsBSR = (1 == CondCode);
    tSymbolFlags Flags;

    /* Zieladresse ermitteln, zum Programmzaehler relativieren */

    HVal = EvalStrIntExpressionWithFlags(&ArgStr[1], Int32, &ValOK, &Flags);
    HVal = HVal - (EProgCounter() + 2);

    /* Bei Automatik Groesse festlegen */

    if (!*AttrPart.Str)
    {
      if (IsDisp8(HVal))
      {
        /* BSR with zero displacement cannot be converted to NOP.  Generate a
           16 bit displacement instead. */

        if (!HVal && IsBSR)
          OpSize = eSymbolSize32Bit;

        /* if the jump target is the address right behind the BSR, keep
           16 bit displacement to avoid oscillating back and forth between
           8 and 16 bits: */

        else if ((Flags & eSymbolFlag_NextLabelAfterBSR) && (HVal == 2) && IsBSR)
          OpSize = eSymbolSize32Bit;
        else
          OpSize = eSymbolSizeFloat32Bit;
      }
      else if (IsDisp16(HVal))
        OpSize = eSymbolSize32Bit;
      else
        OpSize = eSymbolSizeFloat96Bit;
    }

    if (ValOK)
    {
      /* 16 Bit ? */

      if ((OpSize == eSymbolSize32Bit) || (OpSize == eSymbolSize16Bit))
      {
        /* zu weit ? */

        HVal16 = HVal;
        if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(ErrNum_JmpDistTooBig);
        else
        {
          /* Code erzeugen */

          CodeLen = 4;
          WAsmCode[0] = 0x6000 | (CondCode << 8);
          WAsmCode[1] = HVal16;
        }
      }

      /* 8 Bit ? */

      else if ((OpSize == eSymbolSizeFloat32Bit) || (OpSize == eSymbolSize8Bit))
      {
        /* zu weit ? */

        HVal8 = HVal;
        if (!IsDisp8(HVal) && !SymbolQuestionable) WrError(ErrNum_JmpDistTooBig);

        /* cannot generate short BSR with zero displacement, and BSR cannot
           be replaced with NOP -> error */

        else if ((HVal == 0) && IsBSR && !SymbolQuestionable) WrError(ErrNum_JmpDistTooBig);

        /* Code erzeugen */

        else
        {
          CodeLen = 2;
          if ((HVal8 != 0) || IsBSR)
          {
            WAsmCode[0] = 0x6000 | (CondCode << 8) | ((Byte)HVal8);
          }
          else
          {
            WAsmCode[0] = NOPCode;
            if ((!Repass) && *AttrPart.Str)
              WrError(ErrNum_DistNull);
          }
        }
      }

      /* 32 Bit ? */

      else if (!(pCurrCPUProps->SuppFlags & eFlagBranch32)) WrError(ErrNum_InstructionNotSupported);
      else
      {
        CodeLen = 6;
        WAsmCode[0] = 0x60ff | (CondCode << 8);
        WAsmCode[1] = HVal >> 16;
        WAsmCode[2] = HVal & 0xffff;
      }
    }

    if ((CodeLen > 0) && IsBSR)
      AfterBSRAddr = EProgCounter() + CodeLen;
  }
}

static void DecodeScc(Word CondCode)
{
  if (*AttrPart.Str && (OpSize != eSymbolSize8Bit)) WrError(ErrNum_InvOpSize);
  else if (ArgCnt != 1) WrError(ErrNum_InvOpSize);
  else
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(&ArgStr[1], Mdata | ((pCurrCPUProps->Family == eColdfire) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mabs));
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x50c0 | (CondCode << 8) | AdrMode;
      CodeLen = 2 + AdrCnt;
      CopyAdrVals(WAsmCode + 1);
    }
  }
}

static void DecodeDBcc(Word CondCode)
{
  if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(2, 2)
        && CheckNoFamily(1 << eColdfire))
  {
    Boolean ValOK;
    LongInt HVal = EvalStrIntExpression(&ArgStr[2], Int32, &ValOK);
    Integer HVal16;

    if (ValOK)
    {
      HVal -= (EProgCounter() + 2);
      HVal16 = HVal;
      if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(ErrNum_JmpDistTooBig);
      else
      {
        CodeLen = 4;
        WAsmCode[0] = 0x50c8 | (CondCode << 8);
        WAsmCode[1] = HVal16;
        DecodeAdr(&ArgStr[1], Mdata);
        if (AdrNum == 1)
          WAsmCode[0] |= AdrMode;
        else
          CodeLen = 0;
      }
    }
  }
}

static void DecodeTRAPcc(Word CondCode)
{
  int ExpectArgCnt;

  if (!*AttrPart.Str)
    OpSize = eSymbolSize8Bit;
  ExpectArgCnt = (OpSize == eSymbolSize8Bit) ? 0 : 1;
  if (OpSize > 2) WrError(ErrNum_InvOpSize);
  else if (!ChkArgCnt(ExpectArgCnt, ExpectArgCnt));
  else if ((CondCode != 1) && !CheckNoFamily(1 << eColdfire));
  else
  {
    WAsmCode[0] = 0x50f8 + (CondCode << 8);
    if (OpSize == eSymbolSize8Bit)
    {
      WAsmCode[0] += 4;
      CodeLen = 2;
    }
    else
    {
      DecodeAdr(&ArgStr[1], Mimm);
      if (AdrNum != 0)
      {
        WAsmCode[0] += OpSize + 1;
        CopyAdrVals(WAsmCode + 1);
        CodeLen = 2 + AdrCnt;
      }
    }
    CheckFamily((1 << eColdfire) | (1 << eCPU32) | (1 << e68KGen2) | (1 << e68KGen3));
  }
}

/*-------------------------------------------------------------------------*/
/* Dekodierroutinen Gleitkommaeinheit */

static Boolean DecodeOneFPReg(char *Asc, Byte * h)
{
  if ((strlen(Asc) == 3) && (!as_strncasecmp(Asc, "FP", 2)) && ValReg(Asc[2]))
  {
    *h = Asc[2] - '0';
    return True;
  }
  else
    return False;
}

static void DecodeFRegList(char *Asc_o, Byte *Typ, Byte *Erg)
{
  String s, Asc;
  Word hw;
  Byte h2, h3, z;
  char *h1;

  strmaxcpy(Asc, Asc_o, STRINGSIZE);
  *Typ = 0;
  if (*Asc == '\0')
    return;

  if ((strlen(Asc) == 2) && (mytoupper(*Asc) == 'D') && ValReg(Asc[1]))
  {
    *Typ = 1;
    *Erg = (Asc[1] - '0') << 4;
    return;
  }

  hw = 0;
  do
  {
    h1 = strchr(Asc, '/');
    if (!h1)
    {
      strcpy(s, Asc);
      *Asc = '\0';
    }
    else
    {
      *h1 = '\0';
      strcpy(s, Asc);
      strmov(Asc, h1 + 1);
    }
    if (!as_strcasecmp(s, "FPCR"))
      hw |= 0x400;
    else if (!as_strcasecmp(s, "FPSR"))
      hw |= 0x200;
    else if (!as_strcasecmp(s, "FPIAR"))
      hw |= 0x100;
    else
    {
      h1 = strchr(s, '-');
      if (!h1)
      {
        if (!DecodeOneFPReg(s, &h2))
          return;
        hw |= (1 << (7 - h2));
      }
      else
      {
        *h1 = '\0';
        if (!DecodeOneFPReg(s, &h2))
          return;
        if (!DecodeOneFPReg(h1 + 1, &h3))
          return;
        for (z = h2; z <= h3; z++) hw |= (1 << (7 - z));
      }
    }
  }
  while (*Asc != '\0');
  if (Hi(hw) == 0)
  {
    *Typ = 2;
    *Erg = Lo(hw);
  }
  else if (Lo(hw) == 0)
  {
    *Typ = 3;
    *Erg = Hi(hw);
  }
}

static void GenerateMovem(Byte z1, Byte z2)
{
  Byte hz2, z;

  if (AdrNum == 0)
    return;
  CodeLen = 4 + AdrCnt;
  CopyAdrVals(WAsmCode + 2);
  WAsmCode[0] = 0xf200 | AdrMode;
  switch (z1)
  {
    case 1:
    case 2:
      WAsmCode[1] |= 0xc000;
      if (z1 == 1)
        WAsmCode[1] |= 0x800;
      if (AdrNum != 5)
        WAsmCode[1] |= 0x1000;
      if ((AdrNum == 5) && (z1 == 2))
      {
        hz2 = z2; z2 = 0;
        for (z = 0; z < 8; z++)
        {
          z2 = z2 << 1;
          if (hz2 & 1)
            z2 |= 1;
          hz2 = hz2 >> 1;
        }
      }
      WAsmCode[1] |= z2;
      break;
    case 3:
      WAsmCode[1] |= 0x8000 | (((Word)z2) << 10);
      break;
  }
}

/*-------------------------------------------------------------------------*/

static void DecodeFPUOp(Word Index)
{
  FPUOp *Op = FPUOps + Index;
  tStrComp *pArg2 = &ArgStr[2];

  if ((ArgCnt == 1) && (!Op->Dya))
  {
    pArg2 = &ArgStr[1];
    ArgCnt = 2;
  }

  if (!CheckFloatSize());
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if ((pCurrCPUProps->SuppFlags & Op->NeedsSuppFlags) != Op->NeedsSuppFlags) WrError(ErrNum_InstructionNotSupported);
  else if (ChkArgCnt(2, 2))
  {
    DecodeAdr(pArg2, Mfpn);
    if (AdrNum == 12)
    {
      Word SrcMask;

      WAsmCode[0] = 0xf200;
      WAsmCode[1] = Op->Code | (AdrMode << 7);
      RelPos = 4;

      SrcMask = Madri | Mdadri | Mpost | Mpre | Mpc | Mfpn;
      if (FloatOpSizeFitsDataReg(OpSize))
        SrcMask |= Mdata;
      if (pCurrCPUProps->Family != eColdfire)
        SrcMask |= Maix | Mabs | Mpcidx | Mimm;
      DecodeAdr(&ArgStr[1], SrcMask);
      if (AdrNum == 12)
      {
        WAsmCode[1] |= AdrMode << 10;
        if (OpSize == NativeFloatSize)
          CodeLen = 4;
        else
          WrError(ErrNum_InvOpSize);
      }
      else if (AdrNum != 0)
      {
        CodeLen = 4 + AdrCnt;
        CopyAdrVals(WAsmCode + 2);
        WAsmCode[0] |= AdrMode;
        WAsmCode[1] |= 0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
      }
    }
  }
}

static void DecodeFSAVE(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else
  {
    DecodeAdr(&ArgStr[1], Madri | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = 0xf300 | AdrMode;
      CopyAdrVals(WAsmCode + 1);
      CheckSup();
    }
  }
}

static void DecodeFRESTORE(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else
  {
    DecodeAdr(&ArgStr[1], Madri | Mpost | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = 0xf340 | AdrMode;
      CopyAdrVals(WAsmCode + 1);
      CheckSup();
    }
  }
}

static void DecodeFNOP(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(0, 0));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else
  {
    CodeLen = 4;
    WAsmCode[0] = 0xf280;
    WAsmCode[1] = 0;
  }
}

static void DecodeFMOVE(Word Code)
{
  char *pKSep;
  tStrComp KArg;

  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (!CheckFloatSize());
  else
  {
    Word DestMask, SrcMask;

    /* k-Faktor abspalten */

    pKSep = strchr(AttrPart.Str, '{');
    if (pKSep)
    {
      StrCompSplitRef(&AttrPart, &KArg, &AttrPart, pKSep);
      StrCompShorten(&KArg, 1);
    }

    DestMask = Madri | Mpost | Mpre | Mdadri | Mfpcr | Mfpn;
    if (pCurrCPUProps->Family != eColdfire)
      DestMask |= Maix | Mabs | Mimm;
    if (FloatOpSizeFitsDataReg(OpSize))
      DestMask |= Mdata;
    DecodeAdr(&ArgStr[2], DestMask);
    if (AdrNum == 12)                         /* FMOVE.x <ea>/FPm,FPn ? */
    {
      WAsmCode[0] = 0xf200;
      WAsmCode[1] = AdrMode << 7;
      RelPos = 4;
      SrcMask = Madri | Mpost | Mpre | Mdadri | Mpc | Mfpn;
      if (pCurrCPUProps->Family != eColdfire)
        SrcMask |= Maix | Mabs | Mimm | Mpcidx;
      if (FloatOpSizeFitsDataReg(OpSize))
        SrcMask |= Mdata;
      DecodeAdr(&ArgStr[1], SrcMask);
      if (AdrNum == 12)                       /* FMOVE.X FPm,FPn ? */
      {
        WAsmCode[1] |= AdrMode << 10;
        if (OpSize == NativeFloatSize)
          CodeLen = 4;
        else
          WrError(ErrNum_InvOpSize);
      }
      else if (AdrNum != 0)                   /* FMOVE.x <ea>,FPn ? */
      {
        CodeLen = 4 + AdrCnt;
        CopyAdrVals(WAsmCode + 2);
        WAsmCode[0] |= AdrMode;
        WAsmCode[1] |= 0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
      }
    }
    else if (AdrNum == 13)                    /* FMOVE.L <ea>,FPcr ? */
    {
      if ((OpSize != eSymbolSize32Bit) && *AttrPart.Str) WrError(ErrNum_InvOpSize);
      else
      {
        RelPos = 4;
        WAsmCode[0] = 0xf200;
        WAsmCode[1] = 0x8000 | (AdrMode << 10);
        SrcMask = Mdata | Madri | Mpost | Mpre | Mdadri | Mpc;
        if (pCurrCPUProps->Family != eColdfire)
          SrcMask |= Maix | Mabs | Mimm | Mpcidx;
        if (AdrNum != 1) /* only for FPIAR */
          SrcMask |= Madr;
        DecodeAdr(&ArgStr[1], SrcMask);
        if (AdrNum != 0)
        {
          WAsmCode[0] |= AdrMode;
          CodeLen = 4 + AdrCnt;
          CopyAdrVals(WAsmCode + 2);
        }
      }
    }
    else if (AdrNum != 0)                     /* FMOVE.x ????,<ea> ? */
    {
      WAsmCode[0] = 0xf200 | AdrMode;
      CodeLen = 4 + AdrCnt;
      CopyAdrVals(WAsmCode + 2);
      DecodeAdr(&ArgStr[1], (AdrNum == 2) ? Mfpcr : Mfpn | Mfpcr);
      if (AdrNum == 12)                       /* FMOVE.x FPn,<ea> ? */
      {
        WAsmCode[1] = 0x6000 | (((Word)FSizeCodes[OpSize]) << 10) | (AdrMode << 7);
        if (OpSize == eSymbolSizeFloatDec96Bit)
        {
          if (pKSep && (strlen(KArg.Str) > 0))
          {
            OpSize = eSymbolSize8Bit;
            DecodeAdr(&KArg, Mdata | Mimm);
            if (AdrNum == 1)
              WAsmCode[1] |= (AdrMode << 4) | 0x1000;
            else if (AdrNum == 11)
              WAsmCode[1] |= (AdrVals[0] & 127);
            else
              CodeLen = 0;
          }
          else
            WAsmCode[1] |= 17;
        }
      }
      else if (AdrNum == 13)                  /* FMOVE.L FPcr,<ea> ? */
      {
        if (*AttrPart.Str && (OpSize != eSymbolSize32Bit))
        {
          WrError(ErrNum_InvOpSize);
          CodeLen = 0;
        }
        else
        {
          WAsmCode[1] = 0xa000 | (AdrMode << 10);
          if ((AdrMode != 1) && ((WAsmCode[0] & 0x38) == 8))
          {
            WrError(ErrNum_InvAddrMode);
            CodeLen = 0;
          }
        }
      }
      else
        CodeLen = 0;
    }
  }
}

static void DecodeFMOVECR(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (!CheckNoFamily(1 << eColdfire));
  else if (*AttrPart.Str && (OpSize != eSymbolSizeFloat96Bit)) WrError(ErrNum_InvOpSize);
  else
  {
    DecodeAdr(&ArgStr[2], Mfpn);
    if (AdrNum == 12)
    {
      WAsmCode[0] = 0xf200;
      WAsmCode[1] = 0x5c00 | (AdrMode << 7);
      OpSize = eSymbolSize8Bit;
      DecodeAdr(&ArgStr[1], Mimm);
      if (AdrNum == 11)
      {
        if (AdrVals[0] > 63) WrError(ErrNum_RomOffs063);
        else
        {
          CodeLen = 4;
          WAsmCode[1] |= AdrVals[0];
        }
      }
    }
  }
}

static void DecodeFTST(Word Code)
{
  UNUSED(Code);

  if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (!CheckFloatSize());
  else if (ChkArgCnt(1, 1))
  {
    Word Mask;

    RelPos = 4;
    Mask = Madri | Mpost | Mpre | Mdadri | Mpc | Mfpn;
    if (pCurrCPUProps->Family != eColdfire)
      Mask |= Maix | Mpcidx | Mabs | Mimm;
    if (FloatOpSizeFitsDataReg(OpSize))
      Mask |= Mdata;
    DecodeAdr(&ArgStr[1], Mask);
    if (AdrNum == 12)
    {
      WAsmCode[0] = 0xf200;
      WAsmCode[1] = 0x3a | (AdrMode << 10);
      CodeLen = 4;
    }
    else if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf200 | AdrMode;
      WAsmCode[1] = 0x403a | (((Word)FSizeCodes[OpSize]) << 10);
      CodeLen = 4 + AdrCnt;
      CopyAdrVals(WAsmCode + 2);
    }
  }
}

static void DecodeFSINCOS(Word Code)
{
  UNUSED(Code);

  if (!*AttrPart.Str)
    OpSize = NativeFloatSize;
  if (OpSize == 3) WrError(ErrNum_InvOpSize);
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (!CheckNoFamily(1 << eColdfire));
  else if (ChkArgCnt(2, 3))
  {
    tStrComp *pArg2, *pArg3, Arg2, Arg3;

    if (3 == ArgCnt)
    {
      pArg2 = &ArgStr[2];
      pArg3 = &ArgStr[3];
    }
    else
    {
      char *pKSep = strrchr(ArgStr[2].Str, ':');

      if (!pKSep)
      {
        WrError(ErrNum_WrongArgCnt);
        return;
      }
      StrCompSplitRef(&Arg2, &Arg3, &ArgStr[2], pKSep);
      pArg2 = &Arg2;
      pArg3 = &Arg3;
    }
    DecodeAdr(pArg2, Mfpn);
    if (AdrNum == 12)
    {
      WAsmCode[1] = AdrMode | 0x30;
      DecodeAdr(pArg3, Mfpn);
      if (AdrNum == 12)
      {
        WAsmCode[1] |= (AdrMode << 7);
        RelPos = 4;
        DecodeAdr(&ArgStr[1], ((OpSize <= eSymbolSize32Bit) || (OpSize == eSymbolSizeFloat32Bit))
                           ? Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm | Mfpn
                           : Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm | Mfpn);
        if (AdrNum == 12)
        {
          WAsmCode[0] = 0xf200;
          WAsmCode[1] |= (AdrMode << 10);
          CodeLen = 4;
        }
        else if (AdrNum != 0)
        {
          WAsmCode[0] = 0xf200 | AdrMode;
          WAsmCode[1] |= 0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
          CodeLen = 4 + AdrCnt;
          CopyAdrVals(WAsmCode + 2);
        }
      }
    }
  }
}

static void DecodeFDMOVE_FSMOVE(Word Code)
{
  if (!ChkArgCnt(2, 2));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (CheckFamily((1 << e68KGen3) | (1 << eColdfire)))
  {
    DecodeAdr(&ArgStr[2], Mfpn);
    if (AdrNum == 12)
    {
      unsigned Mask;

      WAsmCode[0] = 0xf200;
      WAsmCode[1] = Code | AdrMode << 7;
      RelPos = 4;
      if (!*AttrPart.Str)
        OpSize = NativeFloatSize;
      Mask = Mfpn | Madri | Mpost | Mpre | Mdadri | Mpc;
      if (pCurrCPUProps->Family != eColdfire)
        Mask |= Maix | Mabs | Mpcidx | Mimm;
      if (FloatOpSizeFitsDataReg(OpSize))
        Mask |= Mdata;
      DecodeAdr(&ArgStr[1], Mask);
      if (AdrNum == 12)
      {
        CodeLen = 4;
        WAsmCode[1] |= (AdrMode << 10);
      }
      else if (AdrNum != 0)
      {
        CodeLen = 4 + AdrCnt;
        CopyAdrVals(WAsmCode + 2);
        WAsmCode[0] |= AdrMode;
        WAsmCode[1] |= 0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
      }
    }
  }
}

static void DecodeFMOVEM(Word Code)
{
  Byte Typ, List;
  Word Mask;

  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else
  {
    DecodeFRegList(ArgStr[2].Str, &Typ, &List);
    if (Typ != 0)
    {
      if (*AttrPart.Str
      && (((Typ < 3) && (OpSize != NativeFloatSize))
        || ((Typ == 3) && (OpSize != eSymbolSize32Bit))))
        WrError(ErrNum_InvOpSize);
      else if ((Typ != 2) && (pCurrCPUProps->Family == eColdfire))
        WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[2]);
      else
      {
        RelPos = 4;
        Mask = Madri | Mdadri | Mpc;
        if (pCurrCPUProps->Family != eColdfire)
          Mask |= Mpost | Maix | Mpcidx | Mabs;
        if (Typ == 3)   /* Steuerregister auch Predekrement */
        {
          Mask |= Mpre;
          if ((List == 4) | (List == 2) | (List == 1)) /* nur ein Register */
            Mask |= Mdata | Mimm;
          if (List == 1) /* nur FPIAR */
            Mask |= Madr;
        }
        DecodeAdr(&ArgStr[1], Mask);
        WAsmCode[1] = 0x0000;
        GenerateMovem(Typ, List);
      }
    }
    else
    {
      DecodeFRegList(ArgStr[1].Str, &Typ, &List);
      if (Typ != 0)
      {
        if (*AttrPart.Str && (((Typ < 3) && (OpSize != NativeFloatSize)) || ((Typ == 3) && (OpSize != eSymbolSize32Bit)))) WrError(ErrNum_InvOpSize);
        else if ((Typ != 2) && (pCurrCPUProps->Family == eColdfire)) WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
        else
        {
          Mask = Madri | Mdadri;
          if (pCurrCPUProps->Family != eColdfire)
            Mask |= Mpre | Maix | Mabs;
          if (Typ == 3)   /* Steuerregister auch Postinkrement */
          {
            Mask |= Mpre;
            if ((List == 4) | (List == 2) | (List == 1)) /* nur ein Register */
              Mask |= Mdata;
            if (List == 1) /* nur FPIAR */
              Mask |= Madr;
          }
          DecodeAdr(&ArgStr[2], Mask);
          WAsmCode[1] = 0x2000;
          GenerateMovem(Typ, List);
        }
      }
      else
        WrError(ErrNum_InvRegList);
    }
  }
}

static void DecodeFBcc(Word CondCode)
{
  if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else
  {
    if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit) && (OpSize != eSymbolSizeFloat96Bit)) WrError(ErrNum_InvOpSize);
    else if (ChkArgCnt(1, 1))
    {
      LongInt HVal;
      Integer HVal16;
      Boolean ValOK;

      HVal = EvalStrIntExpression(&ArgStr[1], Int32, &ValOK) - (EProgCounter() + 2);
      HVal16 = HVal;

      if (!*AttrPart.Str)
      {
        OpSize = (IsDisp16(HVal)) ? eSymbolSize32Bit : eSymbolSizeFloat96Bit;
      }

      if ((OpSize == eSymbolSize32Bit) || (OpSize == eSymbolSize16Bit))
      {
        if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(ErrNum_JmpDistTooBig);
        else
        {
          CodeLen = 4;
          WAsmCode[0] = 0xf280 | CondCode;
          WAsmCode[1] = HVal16;
        }
      }
      else
      {
        CodeLen = 6;
        WAsmCode[0] = 0xf2c0 | CondCode;
        WAsmCode[2] = HVal & 0xffff;
        WAsmCode[1] = HVal >> 16;
        if (IsDisp16(HVal) && (PassNo > 1) && !*AttrPart.Str)
        {
          WrError(ErrNum_ShortJumpPossible);
          WAsmCode[0] ^= 0x40;
          CodeLen -= 2;
          WAsmCode[1] = WAsmCode[2];
          StopfZahl++;
        }
      }
    }
  }
}

static void DecodeFDBcc(Word CondCode)
{
  if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (CheckNoFamily(1 << eColdfire))
  {
    if ((OpSize != eSymbolSize16Bit) && *AttrPart.Str) WrError(ErrNum_InvOpSize);
    else if (ChkArgCnt(2, 2))
    {
      DecodeAdr(&ArgStr[1], Mdata);
      if (AdrNum != 0)
      {
        LongInt HVal;
        Integer HVal16;
        Boolean ValOK;

        WAsmCode[0] = 0xf248 | AdrMode;
        WAsmCode[1] = CondCode;
        HVal = EvalStrIntExpression(&ArgStr[2], Int32, &ValOK) - (EProgCounter() + 4);
        if (ValOK)
        {
          HVal16 = HVal;
          WAsmCode[2] = HVal16;
          if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(ErrNum_JmpDistTooBig);
            else CodeLen = 6;
        }
      }
    }
  }
}

static void DecodeFScc(Word CondCode)
{
  if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (!CheckNoFamily(1 << eColdfire));
  else if ((OpSize != eSymbolSize8Bit) && *AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 4 + AdrCnt;
      WAsmCode[0] = 0xf240 | AdrMode;
      WAsmCode[1] = CondCode;
      CopyAdrVals(WAsmCode + 2);
    }
  }
}

static void DecodeFTRAPcc(Word CondCode)
{
  if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (!CheckNoFamily(1 << eColdfire));
  else
  {
    if (!*AttrPart.Str)
      OpSize = eSymbolSize8Bit;
    if (OpSize > eSymbolSize32Bit) WrError(ErrNum_InvOpSize);
    else if (ChkArgCnt(OpSize ? 1 : 0, OpSize ? 1 : 0))
    {
      WAsmCode[0] = 0xf278;
      WAsmCode[1] = CondCode;
      if (OpSize == eSymbolSize8Bit)
      {
        WAsmCode[0] |= 4;
        CodeLen = 4;
      }
      else
      {
        DecodeAdr(&ArgStr[1], Mimm);
        if (AdrNum != 0)
        {
          WAsmCode[0] |= (OpSize + 1);
          CopyAdrVals(WAsmCode + 2);
          CodeLen = 4 + AdrCnt;
        }
      }
    }
  }
}

/*-------------------------------------------------------------------------*/
/* Hilfroutinen MMU: */

static Boolean DecodeFC(const tStrComp *pArg, Word *erg)
{
  Boolean OK;
  Word Val;

  if (!as_strcasecmp(pArg->Str, "SFC"))
  {
    *erg = 0;
    return True;
  }

  if (!as_strcasecmp(pArg->Str, "DFC"))
  {
    *erg = 1;
    return True;
  }

  if ((strlen(pArg->Str) == 2) && (toupper(*pArg->Str) == 'D') && ValReg(pArg->Str[1]))
  {
    *erg = pArg->Str[1] - '0' + 8;
    return True;
  }

  if (*pArg->Str == '#')
  {
    Val = EvalStrIntExpressionOffs(pArg, 1, Int4, &OK);
    if (OK)
      *erg = Val + 16;
    return OK;
  }

  return False;
}

static Boolean DecodePMMUReg(char *Asc, Word *erg, tSymbolSize *pSize)
{
  Byte z;

  if ((strlen(Asc) == 4) && (!as_strncasecmp(Asc, "BAD", 3)) && ValReg(Asc[3]))
  {
    *pSize = eSymbolSize16Bit;
    *erg = 0x7000 + ((Asc[3] - '0') << 2);
    return True;
  }
  if ((strlen(Asc) == 4) && (!as_strncasecmp(Asc, "BAC", 3)) && ValReg(Asc[3]))
  {
    *pSize = eSymbolSize16Bit;
    *erg = 0x7400 + ((Asc[3] - '0') << 2);
    return True;
  }

  for (z = 0; z < PMMURegCnt; z++)
    if (!as_strcasecmp(Asc, PMMURegs[z].pName))
      break;
  if (z < PMMURegCnt)
  {
    *pSize = PMMURegs[z].Size;
    *erg = PMMURegs[z].Code << 10;
  }
  return (z < PMMURegCnt);
}

/*-------------------------------------------------------------------------*/

static void DecodePSAVE(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
  else
  {
    DecodeAdr(&ArgStr[1], Madri | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = 0xf100 | AdrMode;
      CopyAdrVals(WAsmCode + 1);
      CheckSup();
    }
  }
}

static void DecodePRESTORE(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
  else
  {
    DecodeAdr(&ArgStr[1], Madri | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      CodeLen = 2 + AdrCnt;
      WAsmCode[0] = 0xf140 | AdrMode;
      CopyAdrVals(WAsmCode + 1);
      CheckSup();
    }
  }
}

static void DecodePFLUSHA(Word Code)
{
  UNUSED(Code);

  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (ChkArgCnt(0, 0))
  {
    switch (pCurrCPUProps->Family)
    {
      case e68KGen3:
        CodeLen = 2;
        WAsmCode[0] = 0xf518;
        break;
      default:
        CodeLen = 4;
        WAsmCode[0] = 0xf000;
        WAsmCode[1] = 0x2400;
        break;
    }
    CheckSup();
  }
}

static void DecodePFLUSHAN(Word Code)
{
  UNUSED(Code);

  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (ChkArgCnt(0, 0)
        && CheckFamily(1 << e68KGen3))
  {
    CodeLen = 2;
    WAsmCode[0] = 0xf510;
    CheckSup();
  }
}

static void DecodePFLUSH_PFLUSHS(Word Code)
{
  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (pCurrCPUProps->Family == e68KGen3)
  {
    if (Code) WrError(ErrNum_FullPMMUNotEnabled);
    else if (ChkArgCnt(1, 1))
    {
      DecodeAdr(&ArgStr[1], Madri);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0xf508 + (AdrMode & 7);
        CodeLen = 2;
        CheckSup();
      }
    }
  }
  else if (!ChkArgCnt(2, 3));
  else if ((Code) && (!FullPMMU)) WrError(ErrNum_FullPMMUNotEnabled);
  else if (!DecodeFC(&ArgStr[1], WAsmCode + 1)) WrError(ErrNum_InvFCode);
  else
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(&ArgStr[2], Mimm);
    if (AdrNum != 0)
    {
      if (AdrVals[0] > 15) WrError(ErrNum_InvFMask);
      else
      {
        WAsmCode[1] |= (AdrVals[0] << 5) | 0x3000 | Code;
        WAsmCode[0] = 0xf000;
        CodeLen = 4;
        CheckSup();
        if (ArgCnt == 3)
        {
          WAsmCode[1] |= 0x800;
          DecodeAdr(&ArgStr[3], Madri | Mdadri | Maix | Mabs);
          if (AdrNum == 0)
            CodeLen = 0;
          else
          {
            WAsmCode[0] |= AdrMode;
            CodeLen += AdrCnt;
            CopyAdrVals(WAsmCode + 2);
          }
        }
      }
    }
  }
}

static void DecodePFLUSHN(Word Code)
{
  UNUSED(Code);

  if (*AttrPart.Str) WrError(ErrNum_UseLessAttr);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (ChkArgCnt(1, 1)
        && CheckFamily(1 << e68KGen3))
  {
    DecodeAdr(&ArgStr[1], Madri);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf500 + (AdrMode & 7);
      CodeLen = 2;
      CheckSup();
    }
  }
}

static void DecodePFLUSHR(Word Code)
{
  UNUSED(Code);

  if (*AttrPart.Str)
    OpSize = eSymbolSize64Bit;
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  if (OpSize != eSymbolSize64Bit) WrError(ErrNum_InvOpSize);
  else if (!ChkArgCnt(1, 1));
  else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
  else
  {
    RelPos = 4;
    DecodeAdr(&ArgStr[1], Madri | Mpre | Mpost | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf000 | AdrMode;
      WAsmCode[1] = 0xa000;
      CopyAdrVals(WAsmCode + 2);
      CodeLen = 4 + AdrCnt; CheckSup();
    }
  }
}

static void DecodePLOADR_PLOADW(Word Code)
{
  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (!ChkArgCnt(2, 2));
  else if (!DecodeFC(&ArgStr[1], WAsmCode + 1)) WrError(ErrNum_InvFCode);
  else
  {
    DecodeAdr(&ArgStr[2], Madri | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf000 | AdrMode;
      WAsmCode[1] |= Code;
      CodeLen = 4 + AdrCnt;
      CopyAdrVals(WAsmCode + 2);
      CheckSup();
    }
  }
}

static void DecodePMOVE_PMOVEFD(Word Code)
{
  tSymbolSize RegSize;
  unsigned Mask;

  if (!ChkArgCnt(2, 2));
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else
  {
    if (DecodePMMUReg(ArgStr[1].Str, WAsmCode + 1, &RegSize))
    {
      WAsmCode[1] |= 0x200;
      if (!*AttrPart.Str)
        OpSize = RegSize;
      if (OpSize != RegSize) WrError(ErrNum_InvOpSize);
      else
      {
        Mask = Madri | Mdadri | Maix | Mabs;
        if (FullPMMU)
        {
          Mask *= Mpost | Mpre;
          if (RegSize != eSymbolSize64Bit)
            Mask += Mdata | Madr;
        }
        DecodeAdr(&ArgStr[2], Mask);
        if (AdrNum != 0)
        {
          WAsmCode[0] = 0xf000 | AdrMode;
          CodeLen = 4 + AdrCnt;
          CopyAdrVals(WAsmCode + 2);
          CheckSup();
        }
      }
    }
    else if (DecodePMMUReg(ArgStr[2].Str, WAsmCode + 1, &RegSize))
    {
      if (!*AttrPart.Str)
        OpSize = RegSize;
      if (OpSize != RegSize) WrError(ErrNum_InvOpSize);
      else
      {
        RelPos = 4;
        Mask = Madri | Mdadri | Maix | Mabs;
        if (FullPMMU)
        {
          Mask += Mpost | Mpre | Mpc | Mpcidx | Mimm;
          if (RegSize != eSymbolSize64Bit)
            Mask += Mdata | Madr;
        }
        DecodeAdr(&ArgStr[1], Mask);
        if (AdrNum != 0)
        {
          WAsmCode[0] = 0xf000 | AdrMode;
          CodeLen = 4 + AdrCnt;
          CopyAdrVals(WAsmCode + 2);
          WAsmCode[1] += Code;
          CheckSup();
        }
      }
    }
    else
      WrError(ErrNum_InvMMUReg);
  }
}

static void DecodePTESTR_PTESTW(Word Code)
{
  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (pCurrCPUProps->Family == e68KGen3)
  {
    if (ChkArgCnt(1, 1))
    {
      DecodeAdr(&ArgStr[1], Madri);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0xf548 + (AdrMode & 7) + (Code << 5);
        CodeLen = 2;
        CheckSup();
      }
    }
  }
  else if (ChkArgCnt(3, 4))
  {
    if (!DecodeFC(&ArgStr[1], WAsmCode + 1)) WrError(ErrNum_InvFCode);
    else
    {
      DecodeAdr(&ArgStr[2], Madri | Mdadri | Maix | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0xf000 | AdrMode;
        CodeLen = 4 + AdrCnt;
        WAsmCode[1] |= 0x8000 | (Code << 9);
        CopyAdrVals(WAsmCode + 2);
        DecodeAdr(&ArgStr[3], Mimm);
        if (AdrNum != 0)
        {
          if (AdrVals[0] > 7)
          {
            WrError(ErrNum_Level07);
            CodeLen = 0;
          }
          else
          {
            WAsmCode[1] |= AdrVals[0] << 10;
            if (ArgCnt == 4)
            {
              DecodeAdr(&ArgStr[4], Madr);
              if (AdrNum == 0)
                CodeLen = 0;
              else
                WAsmCode[1] |= AdrMode << 5;
              CheckSup();
            }
          }
        }
        else
          CodeLen = 0;
      }
    }
  }
}

static void DecodePVALID(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
  else if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else
  {
    DecodeAdr(&ArgStr[2], Madri | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf000 | AdrMode;
      WAsmCode[1] = 0x2800;
      CodeLen = 4 + AdrCnt;
      CopyAdrVals(WAsmCode + 1);
      if (!as_strcasecmp(ArgStr[1].Str, "VAL"));
      else
      {
        DecodeAdr(&ArgStr[1], Madr);
        if (AdrNum != 0)
          WAsmCode[1] |= 0x400 | (AdrMode & 7);
        else
          CodeLen = 0;
      }
    }
  }
}

static void DecodePBcc(Word CondCode)
{
  if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else
  {
    if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit) && (OpSize != eSymbolSizeFloat96Bit)) WrError(ErrNum_InvOpSize);
    else if (!ChkArgCnt(1, 1));
    else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
    else
    {
      LongInt HVal;
      Integer HVal16;
      Boolean ValOK;

      HVal = EvalStrIntExpression(&ArgStr[1], Int32, &ValOK) - (EProgCounter() + 2);
      HVal16 = HVal;

      if (!*AttrPart.Str)
        OpSize = (IsDisp16(HVal)) ? eSymbolSize32Bit : eSymbolSizeFloat96Bit;

      if ((OpSize == eSymbolSize32Bit) || (OpSize == eSymbolSize16Bit))
      {
        if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(ErrNum_JmpDistTooBig);
        else
        {
          CodeLen = 4;
          WAsmCode[0] = 0xf080 | CondCode;
          WAsmCode[1] = HVal16;
          CheckSup();
        }
      }
      else
      {
        CodeLen = 6;
        WAsmCode[0] = 0xf0c0 | CondCode;
        WAsmCode[2] = HVal & 0xffff;
        WAsmCode[1] = HVal >> 16;
        CheckSup();
      }
    }
  }
}

static void DecodePDBcc(Word CondCode)
{
  if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else
  {
    if ((OpSize != eSymbolSize16Bit) && *AttrPart.Str) WrError(ErrNum_InvOpSize);
    else if (!ChkArgCnt(2, 2));
    else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
    else
    {
      DecodeAdr(&ArgStr[1], Mdata);
      if (AdrNum != 0)
      {
        LongInt HVal;
        Integer HVal16;
        Boolean ValOK;

        WAsmCode[0] = 0xf048 | AdrMode;
        WAsmCode[1] = CondCode;
        HVal = EvalStrIntExpression(&ArgStr[2], Int32, &ValOK) - (EProgCounter() + 4);
        if (ValOK)
        {
          HVal16 = HVal;
          WAsmCode[2] = HVal16;
          if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(ErrNum_JmpDistTooBig);
          else
            CodeLen = 6;
          CheckSup();
        }
      }
    }
  }
}

static void DecodePScc(Word CondCode)
{
  if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else
  {
    if ((OpSize != eSymbolSize8Bit) && *AttrPart.Str) WrError(ErrNum_InvOpSize);
    else if (!ChkArgCnt(1, 1));
    else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
    else
    {
      DecodeAdr(&ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
      if (AdrNum != 0)
      {
        CodeLen = 4 + AdrCnt;
        WAsmCode[0] = 0xf040 | AdrMode;
        WAsmCode[1] = CondCode;
        CopyAdrVals(WAsmCode + 2);
        CheckSup();
      }
    }
  }
}

static void DecodePTRAPcc(Word CondCode)
{
  if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else
  {
    if (!*AttrPart.Str)
      OpSize = eSymbolSize8Bit;
    if (OpSize > 2) WrError(ErrNum_InvOpSize);
    else if (!ChkArgCnt(OpSize ? 1 : 0, OpSize ? 1 : 0));
    else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
    else
    {
      WAsmCode[0] = 0xf078;
      WAsmCode[1] = CondCode;
      if (OpSize == eSymbolSize8Bit)
      {
        WAsmCode[0] |= 4;
        CodeLen = 4;
        CheckSup();
      }
      else
      {
        DecodeAdr(&ArgStr[1], Mimm);
        if (AdrNum != 0)
        {
          WAsmCode[0] |= (OpSize + 1);
          CopyAdrVals(WAsmCode + 2);
          CodeLen = 4 + AdrCnt;
          CheckSup();
        }
      }
    }
  }
}

static void DecodeColdBit(Word Code)
{
  if (!*AttrPart.Str)
    OpSize = eSymbolSize32Bit;
  if (ChkArgCnt(1, 1)
   && CheckColdSize()
   && CheckFamily(1 << eColdfire)
   && CheckISA((1 << eCfISA_APlus) | (1 << eCfISA_C)))
  {
    DecodeAdr(&ArgStr[1], Mdata);
    if (AdrNum != 0)
    {
      CodeLen = 2;
      WAsmCode[0] = Code | (AdrMode & 7);
    }
  }
}

static void DecodeSTLDSR(Word Code)
{
  UNUSED(Code);

  if (!*AttrPart.Str)
    OpSize = eSymbolSize16Bit;
  if (OpSize != eSymbolSize16Bit) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(1, 1)
        && CheckFamily(1 << eColdfire)
        && CheckISA((1 << eCfISA_APlus) | (1 << eCfISA_C)))
  {
    DecodeAdr(&ArgStr[1], Mimm);
    if (AdrNum != 0)
    {
      CodeLen = 6;
      WAsmCode[0] = 0x40e7;
      WAsmCode[1] = 0x46fc;
      WAsmCode[2] = AdrVals[0];
    }
  }
}

static void DecodeINTOUCH(Word Code)
{
  UNUSED(Code);

  if (*AttrPart.Str) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(1, 1)
        && CheckFamily(1 << eColdfire)
        && (pCurrCPUProps->CfISA >= eCfISA_B))
  {
    DecodeAdr(&ArgStr[1], Madri);
    if (AdrNum != 0)
    {
      CodeLen = 2;
      WAsmCode[0] = 0xf428 | (AdrMode & 7);
      CheckSup();
    }
  }
}

static void DecodeMOV3Q(Word Code)
{
  Boolean OK;
  ShortInt Val;

  UNUSED(Code);

  if (!ChkArgCnt(2, 2)
   || !CheckFamily(1 << eColdfire)
   || (pCurrCPUProps->CfISA < eCfISA_B)
   || !CheckColdSize())
    return;

  DecodeAdr(&ArgStr[2], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
  if (AdrNum == 0)
    return;

  if (*ArgStr[1].Str != '#')
  {
    WrStrErrorPos(ErrNum_OnlyImmAddr, &ArgStr[1]);
    return;
  }

  FirstPassUnknown = False;
  Val = EvalStrIntExpressionOffs(&ArgStr[1], 1, SInt4, &OK);
  if (!OK)
    return;
  if (FirstPassUnknown)
    Val = 1;

  if (Val == -1)
    Val = 0;
  else if (!ChkRange(Val, 1, 7))
    return;

  WAsmCode[0] = 0xa140 | ((Val & 7) << 9) | AdrMode;
  CopyAdrVals(WAsmCode + 1);
  CodeLen = 2 + AdrCnt;
}

static void DecodeMVS_MVZ(Word Code)
{
  Word DestReg;

  if (!ChkArgCnt(2, 2)
   || !CheckFamily(1 << eColdfire)
   || (pCurrCPUProps->CfISA < eCfISA_B))
    return;

  if (!*AttrPart.Str)
    OpSize = eSymbolSize16Bit;
  if (OpSize > eSymbolSize16Bit)
  {
    WrError(ErrNum_InvOpSize);
    return;
  }

  DecodeAdr(&ArgStr[2], Mdata);
  if (!AdrNum)
    return;
  DestReg = AdrMode & 7;

  DecodeAdr(&ArgStr[1], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs | Mimm | Mpc | Mpcidx);
  if (AdrNum)
  {
    WAsmCode[0] = Code | (DestReg << 9) | (OpSize << 6) | AdrMode;
    CopyAdrVals(WAsmCode + 1);
    CodeLen = 2 + AdrCnt;
  }
}

static void DecodeSATS(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1)
   || !CheckFamily(1 << eColdfire)
   || (pCurrCPUProps->CfISA < eCfISA_B)
   || !CheckColdSize())
    return;

  DecodeAdr(&ArgStr[1], Mdata);
  if (AdrNum)
  {
    WAsmCode[0] = 0x4c80 | (AdrMode & 7);
    CodeLen = 2;
  }
}

static void DecodeMAC_MSAC(Word Code)
{
  Word Rx, Ry, Rw, Ux = 0, Uy = 0, Scale = 0, Mask, AccNum = 0;
  int CurrArg, RemArgCnt;
  Boolean ExplicitLoad = !!(Code & 0x8000);

  Code &= 0x7fff;

  if (!(pCurrCPUProps->SuppFlags & eFlagMAC))
  {
    WrError(ErrNum_InstructionNotSupported);
    return;
  }

  if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit))
  {
    WrError(ErrNum_InvOpSize);
    return;
  }

  /* 2 args is the absolute minimum.  6 is the maximum (Ry, Rx, scale, <ea>, Rw, ACC) */

  if (!ChkArgCnt(2, 6))
    return;

  /* Ry and Rx are always present, and are always the first arguments: */

  if (OpSize == eSymbolSize16Bit)
  {
    if (!SplitMACUpperLower(&Uy, &ArgStr[1])
     || !SplitMACUpperLower(&Ux, &ArgStr[2]))
      return;
  }

  DecodeAdr(&ArgStr[1], Mdata | Madr);
  if (!AdrNum)
    return;
  Ry = AdrMode & 15;
  DecodeAdr(&ArgStr[2], Mdata | Madr);
  if (!AdrNum)
    return;
  Rx = AdrMode & 15;
  CurrArg = 3;

  /* Is a scale given as next argument? */

  if ((ArgCnt >= CurrArg) && DecodeMACScale(&ArgStr[CurrArg], &Scale))
    CurrArg++;

  /* We now have between 0 and 3 args left:
     0 -> no load, ACC0
     1 -> ACCn
     2 -> load, ACC0
     3 -> load, ACCn
     If the 'L' variant (MACL, MSACL) was given, a parallel
     load was specified explicitly and there MUST be the <ea> and Rw arguments: */

  RemArgCnt = ArgCnt - CurrArg + 1;
  if ((RemArgCnt > 3)
   || (ExplicitLoad && (RemArgCnt < 2)))
  {
    WrError(ErrNum_WrongArgCnt);
    return;
  }

  /* assumed ACC(0) if no accumulator given */

  if (Odd(RemArgCnt))
  {
    if (!DecodeMACACC(ArgStr[ArgCnt].Str, &AccNum))
    {
      WrStrErrorPos(ErrNum_InvReg, &ArgStr[ArgCnt]);
      return;
    }
  }

  /* If parallel load, bit 7 of first word is set for MAC.  This bit is
     used on EMAC to store accumulator # LSB.  To keep things upward-compatible,
     accumulator # LSB is stored inverted on EMAC if a parallel load is present.
     Since MAC only uses accumulator #0, this works for either target: */

  if (RemArgCnt >= 2)
    AccNum ^= 1;

  /* Common things for variant with and without parallel load: */

  WAsmCode[0] = 0xa000 | ((AccNum & 1) << 7);
  WAsmCode[1] = ((OpSize - 1) << 11) | (Scale << 9) | Code | (Ux << 7) | (Uy << 6) | ((AccNum & 2) << 3);

  /* With parallel load? */

  if (RemArgCnt >= 2)
  {
    DecodeAdr(&ArgStr[CurrArg + 1], Mdata | Madr);
    if (!AdrNum)
      return;
    Rw = AdrMode & 15;

    if (!SplitMACANDMASK(&Mask, &ArgStr[CurrArg]))
      return;
    DecodeAdr(&ArgStr[CurrArg], Madri | Mpre | Mpost | Mdadri);
    if (!AdrNum)
      return;

    WAsmCode[0] |= ((Rw & 7) << 9) | ((Rw & 8) << 3) | AdrMode;
    WAsmCode[1] |= (Mask << 5) | (Rx << 12) | (Ry << 0);
    CodeLen = 4 + AdrCnt;
    CopyAdrVals(WAsmCode + 2);
  }

  /* multiply/accumulate only */

  else
  {
    WAsmCode[0] |= Ry | ((Rx & 7) << 9) | ((Rx & 8) << 3);
    CodeLen = 4;
  }
}

static void DecodeMOVCLR(Word Code)
{
  Word ACCReg;

  UNUSED(Code);

  if (!ChkArgCnt(2,2));
  else if (*AttrPart.Str && (OpSize != eSymbolSize32Bit)) WrError(ErrNum_InvOpSize);
  else if (!(pCurrCPUProps->SuppFlags & eFlagEMAC)) WrError(ErrNum_InstructionNotSupported);
  else if (!DecodeMACACC(ArgStr[1].Str, &ACCReg)) WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
  else
  {
    DecodeAdr(&ArgStr[2], Mdata | Madr);
    if (AdrNum)
    {
      WAsmCode[0] = 0xa1c0 | AdrMode | (ACCReg << 9);
      CodeLen = 2;
    }
  }
}

static void DecodeMxxAC(Word Code)
{
  Word Rx, Ry, Ux, Uy, Scale = 0, ACCx, ACCw;

  if (!(pCurrCPUProps->SuppFlags & eFlagEMAC)
    || (pCurrCPUProps->CfISA < eCfISA_B))
  {
    WrError(ErrNum_InstructionNotSupported);
    return;
  }

  if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit))
  {
    WrError(ErrNum_InvOpSize);
    return;
  }

  if (!ChkArgCnt(4, 5))
    return;

  if (!DecodeMACACC(ArgStr[ArgCnt - 1].Str, &ACCx))
  {
    WrStrErrorPos(ErrNum_InvReg, &ArgStr[ArgCnt - 1]);
    return;
  }
  if (!DecodeMACACC(ArgStr[ArgCnt].Str, &ACCw))
  {
    WrStrErrorPos(ErrNum_InvReg, &ArgStr[ArgCnt]);
    return;
  }

  if (5 == ArgCnt)
  {
    if (!DecodeMACScale(&ArgStr[3], &Scale))
    {
      WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[3]);
      return;
    }
  }

  if (OpSize == eSymbolSize16Bit)
  {
    if (!SplitMACUpperLower(&Uy, &ArgStr[1])
     || !SplitMACUpperLower(&Ux, &ArgStr[2]))
      return;
  }
  else
    Ux = Uy = 0;

  DecodeAdr(&ArgStr[1], Mdata | Madr);
  if (!AdrNum)
    return;
  Ry = AdrMode & 15;
  DecodeAdr(&ArgStr[2], Mdata | Madr);
  if (!AdrNum)
    return;
  Rx = AdrMode & 15;

  WAsmCode[0] = 0xa000 | ((Rx & 7) << 9) | ((Rx & 8) << 3) | Ry | ((ACCx & 1) << 7);
  WAsmCode[1] = Code | ((OpSize - 1) << 11) | (Scale << 9) | (Ux << 7) | (Uy << 6) | ((ACCx & 2) << 3) | (ACCw << 2);
  CodeLen = 4;
}

static void DecodeCPBCBUSY(Word Code)
{
  if (pCurrCPUProps->CfISA == eCfISA_None) WrError(ErrNum_InstructionNotSupported);
  else if (*AttrPart.Str && (OpSize != eSymbolSize16Bit)) WrError(ErrNum_InvOpSize);
  else if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    LongInt Dist;

    Dist = EvalStrIntExpression(&ArgStr[1], UInt32, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (!SymbolQuestionable && !IsDisp16(Dist)) WrError(ErrNum_JmpDistTooBig);
      else
      {
        WAsmCode[0] = Code;
        WAsmCode[1] = Dist & 0xffff;
        CodeLen = 4;
      }
    }
  }
}

static void DecodeCPLDST(Word Code)
{
  if (pCurrCPUProps->CfISA == eCfISA_None) WrError(ErrNum_InstructionNotSupported);
  else if (ChkArgCnt(1, 4))
  {
    Boolean OK;
    Word Reg;
    const tStrComp *pEAArg = NULL, *pRnArg = NULL, *pETArg = NULL;

    WAsmCode[0] = Code | (OpSize << 6);

    /* CMD is always present and i bits 0..8 - immediate marker is optional
       since it is always a constant. */

    WAsmCode[1] = EvalStrIntExpressionOffs(&ArgStr[ArgCnt], !!(*ArgStr[ArgCnt].Str == '#'), UInt16, &OK);
    if (!OK)
      return;

    if (ArgCnt >= 2)
      pEAArg = &ArgStr[1];
    switch (ArgCnt)
    {
      case 4:
        pRnArg = &ArgStr[2];
        pETArg = &ArgStr[3];
        break;
      case 3:
        if (CodeReg(ArgStr[2].Str, &Reg))
          pRnArg = &ArgStr[2];
        else
          pETArg = &ArgStr[2];
        break;
     }

    if (pRnArg)
    {
      if (!CodeReg(pRnArg->Str, &Reg))
      {
        WrStrErrorPos(ErrNum_InvReg, pRnArg);
        return;
      }
      WAsmCode[1] |= Reg << 12;
    }
    if (pETArg)
    {
      Word ET;

      ET = EvalStrIntExpression(pETArg, UInt3, &OK);
      if (!OK)
        return;
      WAsmCode[1] |= ET << 9;
    }

    if (pEAArg)
    {
      DecodeAdr(pEAArg, Mdata | Madr | Madri | Mpost | Mpre | Mdadri);
      if (!AdrNum)
        return;
      WAsmCode[0] |= AdrMode;
      CopyAdrVals(WAsmCode + 2);
      CodeLen = 4 + AdrCnt;
    }
    else
      CodeLen = 4;
  }
}

static void DecodeCPNOP(Word Code)
{
  if (pCurrCPUProps->CfISA == eCfISA_None) WrError(ErrNum_InstructionNotSupported);
  else if (ChkArgCnt(0, 1))
  {
    WAsmCode[0] = Code | (OpSize << 6);

    /* CMD is always present and i bits 0..8 - immediate marker is optional
       since it is always a constant. */

    if (ArgCnt > 0)
    {
      Word ET;
      Boolean OK;

      ET = EvalStrIntExpression(&ArgStr[1], UInt3, &OK);
      if (!OK)
        return;
      WAsmCode[1] |= ET << 9;
    }

    CodeLen = 4;
  }
}

/*-------------------------------------------------------------------------*/
/* Dekodierroutinen Pseudoinstruktionen: */

static void PutByte(Byte b)
{
  if ((CodeLen & 1) && (!BigEndian))
  {
    BAsmCode[CodeLen] = BAsmCode[CodeLen - 1];
    BAsmCode[CodeLen - 1] = b;
  }
  else
  {
    BAsmCode[CodeLen] = b;
  }
  CodeLen++;
}

static void DecodeSTR(Word Index)
{
  int l, z;
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (((l = strlen(ArgStr[1].Str)) < 2)
        || (*ArgStr[1].Str != '\'')
        || (ArgStr[1].Str[l - 1] != '\'')) WrStrErrorPos(ErrNum_ExpectString, &ArgStr[1]);
  else
  {
    PutByte(l - 2);
    for (z = 1; z < l - 1; z++)
      PutByte(CharTransTable[((usint) ArgStr[1].Str[z]) & 0xff]);
    if ((Odd(CodeLen)) && (DoPadding))
      PutByte(0);
  }
}

/*-------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static void AddFixed(char *NName, Word NCode, Boolean NSup, unsigned NMask)
{
  if (InstrZ >= FixedOrderCnt) exit(255);
  FixedOrders[InstrZ].Code = NCode;
  FixedOrders[InstrZ].MustSup = NSup;
  FixedOrders[InstrZ].FamilyMask = NMask;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddCond(char *NName, Byte NCode)
{
  char TmpName[30];

  if (NCode >= 2) /* BT is BRA and BF is BSR */
  {
    as_snprintf(TmpName, sizeof(TmpName), "B%s", NName);
    AddInstTable(InstTable, TmpName, NCode, DecodeBcc);
  }
  as_snprintf(TmpName, sizeof(TmpName), "S%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeScc);
  as_snprintf(TmpName, sizeof(TmpName), "DB%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeDBcc);
  as_snprintf(TmpName, sizeof(TmpName), "TRAP%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeTRAPcc);
}

static void AddFPUOp(char *NName, Byte NCode, Boolean NDya, tSuppFlags NeedFlags)
{
  if (InstrZ >= FPUOpCnt) exit(255);
  FPUOps[InstrZ].Code = NCode;
  FPUOps[InstrZ].Dya = NDya;
  FPUOps[InstrZ].NeedsSuppFlags = NeedFlags;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFPUOp);
}

static void AddFPUCond(char *NName, Byte NCode)
{
  char TmpName[30];

  as_snprintf(TmpName, sizeof(TmpName), "FB%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeFBcc);
  as_snprintf(TmpName, sizeof(TmpName), "FDB%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeFDBcc);
  as_snprintf(TmpName, sizeof(TmpName), "FS%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeFScc);
  as_snprintf(TmpName, sizeof(TmpName), "FTRAP%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeFTRAPcc);
}

static void AddPMMUCond(char *NName)
{
  char TmpName[30];

  as_snprintf(TmpName, sizeof(TmpName), "PB%s", NName);
  AddInstTable(InstTable, TmpName, InstrZ, DecodePBcc);
  as_snprintf(TmpName, sizeof(TmpName), "PDB%s", NName);
  AddInstTable(InstTable, TmpName, InstrZ, DecodePDBcc);
  as_snprintf(TmpName, sizeof(TmpName), "PS%s", NName);
  AddInstTable(InstTable, TmpName, InstrZ, DecodePScc);
  as_snprintf(TmpName, sizeof(TmpName), "PTRAP%s", NName);
  AddInstTable(InstTable, TmpName, InstrZ, DecodePTRAPcc);
  InstrZ++;
}

static void AddPMMUReg(char *Name, Byte Size, Word Code)
{
  if (InstrZ >= PMMURegCnt) exit(255);
  PMMURegs[InstrZ].pName = Name;
  PMMURegs[InstrZ].Size = Size;
  PMMURegs[InstrZ++].Code = Code;
}

static void InitFields(void)
{
  InstTable = CreateInstTable(607);
  SetDynamicInstTable(InstTable);

  AddInstTable(InstTable, "MOVE"   , Std_Variant, DecodeMOVE);
  AddInstTable(InstTable, "MOVEA"  , A_Variant, DecodeMOVE);
  AddInstTable(InstTable, "MOVEI"  , I_Variant, DecodeMOVE);
  AddInstTable(InstTable, "LEA"    , 0, DecodeLEA);
  AddInstTable(InstTable, "ASR"    , 0, DecodeShift);
  AddInstTable(InstTable, "ASL"    , 4, DecodeShift);
  AddInstTable(InstTable, "LSR"    , 1, DecodeShift);
  AddInstTable(InstTable, "LSL"    , 5, DecodeShift);
  AddInstTable(InstTable, "ROXR"   , 2, DecodeShift);
  AddInstTable(InstTable, "ROXL"   , 6, DecodeShift);
  AddInstTable(InstTable, "ROR"    , 3, DecodeShift);
  AddInstTable(InstTable, "ROL"    , 7, DecodeShift);
  AddInstTable(InstTable, "ADDQ"   , 0, DecodeADDQSUBQ);
  AddInstTable(InstTable, "SUBQ"   , 1, DecodeADDQSUBQ);
  AddInstTable(InstTable, "ADDX"   , 1, DecodeADDXSUBX);
  AddInstTable(InstTable, "SUBX"   , 0, DecodeADDXSUBX);
  AddInstTable(InstTable, "CMPM"   , 0, DecodeCMPM);
  AddInstTable(InstTable, "SUB"    , Std_Variant + 0, DecodeADDSUBCMP);
  AddInstTable(InstTable, "CMP"    , Std_Variant + 1, DecodeADDSUBCMP);
  AddInstTable(InstTable, "ADD"    , Std_Variant + 2, DecodeADDSUBCMP);
  AddInstTable(InstTable, "SUBI"   , I_Variant + 0, DecodeADDSUBCMP);
  AddInstTable(InstTable, "CMPI"   , I_Variant + 1, DecodeADDSUBCMP);
  AddInstTable(InstTable, "ADDI"   , I_Variant + 2, DecodeADDSUBCMP);
  AddInstTable(InstTable, "SUBA"   , A_Variant + 0, DecodeADDSUBCMP);
  AddInstTable(InstTable, "CMPA"   , A_Variant + 1, DecodeADDSUBCMP);
  AddInstTable(InstTable, "ADDA"   , A_Variant + 2, DecodeADDSUBCMP);
  AddInstTable(InstTable, "AND"    , Std_Variant + 1, DecodeANDOR);
  AddInstTable(InstTable, "OR"     , Std_Variant + 0, DecodeANDOR);
  AddInstTable(InstTable, "ANDI"   , I_Variant + 1, DecodeANDOR);
  AddInstTable(InstTable, "ORI"    , I_Variant + 0, DecodeANDOR);
  AddInstTable(InstTable, "EOR"    , Std_Variant, DecodeEOR);
  AddInstTable(InstTable, "EORI"   , I_Variant, DecodeEOR);
  AddInstTable(InstTable, "PEA"    , 0, DecodePEA);
  AddInstTable(InstTable, "CLR"    , 0, DecodeCLRTST);
  AddInstTable(InstTable, "TST"    , 1, DecodeCLRTST);
  AddInstTable(InstTable, "JSR"    , 0, DecodeJSRJMP);
  AddInstTable(InstTable, "JMP"    , 1, DecodeJSRJMP);
  AddInstTable(InstTable, "TAS"    , 0, DecodeNBCDTAS);
  AddInstTable(InstTable, "NBCD"   , 1, DecodeNBCDTAS);
  AddInstTable(InstTable, "NEGX"   , 0, DecodeNEGNOT);
  AddInstTable(InstTable, "NEG"    , 2, DecodeNEGNOT);
  AddInstTable(InstTable, "NOT"    , 3, DecodeNEGNOT);
  AddInstTable(InstTable, "SWAP"   , 0, DecodeSWAP);
  AddInstTable(InstTable, "UNLK"   , 0, DecodeUNLK);
  AddInstTable(InstTable, "EXT"    , 0, DecodeEXT);
  AddInstTable(InstTable, "WDDATA" , 0, DecodeWDDATA);
  AddInstTable(InstTable, "WDEBUG" , 0, DecodeWDEBUG);
  AddInstTable(InstTable, "MOVEM"  , 0, DecodeMOVEM);
  AddInstTable(InstTable, "MOVEQ"  , 0, DecodeMOVEQ);
  AddInstTable(InstTable, "STOP"   , 0, DecodeSTOP);
  AddInstTable(InstTable, "LPSTOP" , 0, DecodeLPSTOP);
  AddInstTable(InstTable, "TRAP"   , 0, DecodeTRAP);
  AddInstTable(InstTable, "BKPT"   , 0, DecodeBKPT);
  AddInstTable(InstTable, "RTD"    , 0, DecodeRTD);
  AddInstTable(InstTable, "EXG"    , 0, DecodeEXG);
  AddInstTable(InstTable, "MOVE16" , 0, DecodeMOVE16);
  AddInstTable(InstTable, "MULU"   , 0x0000, DecodeMUL_DIV);
  AddInstTable(InstTable, "MULS"   , 0x0100, DecodeMUL_DIV);
  AddInstTable(InstTable, "DIVU"   , 0x0001, DecodeMUL_DIV);
  AddInstTable(InstTable, "DIVS"   , 0x0101, DecodeMUL_DIV);
  AddInstTable(InstTable, "DIVUL"  , 0, DecodeDIVL);
  AddInstTable(InstTable, "DIVSL"  , 1, DecodeDIVL);
  AddInstTable(InstTable, "ABCD"   , 1, DecodeASBCD);
  AddInstTable(InstTable, "SBCD"   , 0, DecodeASBCD);
  AddInstTable(InstTable, "CHK"    , 0, DecodeCHK);
  AddInstTable(InstTable, "LINK"   , 0, DecodeLINK);
  AddInstTable(InstTable, "MOVEP"  , 0, DecodeMOVEP);
  AddInstTable(InstTable, "MOVEC"  , 0, DecodeMOVEC);
  AddInstTable(InstTable, "MOVES"  , 0, DecodeMOVES);
  AddInstTable(InstTable, "CALLM"  , 0, DecodeCALLM);
  AddInstTable(InstTable, "CAS"    , 0, DecodeCAS);
  AddInstTable(InstTable, "CAS2"   , 0, DecodeCAS2);
  AddInstTable(InstTable, "CMP2"   , 0, DecodeCMPCHK2);
  AddInstTable(InstTable, "CHK2"   , 1, DecodeCMPCHK2);
  AddInstTable(InstTable, "EXTB"   , 0, DecodeEXTB);
  AddInstTable(InstTable, "PACK"   , 0, DecodePACK);
  AddInstTable(InstTable, "UNPK"   , 1, DecodePACK);
  AddInstTable(InstTable, "RTM"    , 0, DecodeRTM);
  AddInstTable(InstTable, "TBLU"   , 0, DecodeTBL);
  AddInstTable(InstTable, "TBLUN"  , 1, DecodeTBL);
  AddInstTable(InstTable, "TBLS"   , 2, DecodeTBL);
  AddInstTable(InstTable, "TBLSN"  , 3, DecodeTBL);
  AddInstTable(InstTable, "BTST"   , 0, DecodeBits);
  AddInstTable(InstTable, "BSET"   , 3, DecodeBits);
  AddInstTable(InstTable, "BCLR"   , 2, DecodeBits);
  AddInstTable(InstTable, "BCHG"   , 1, DecodeBits);
  AddInstTable(InstTable, "BFTST"  , 0, DecodeFBits);
  AddInstTable(InstTable, "BFSET"  , 3, DecodeFBits);
  AddInstTable(InstTable, "BFCLR"  , 2, DecodeFBits);
  AddInstTable(InstTable, "BFCHG"  , 1, DecodeFBits);
  AddInstTable(InstTable, "BFEXTU" , 0, DecodeEBits);
  AddInstTable(InstTable, "BFEXTS" , 1, DecodeEBits);
  AddInstTable(InstTable, "BFFFO"  , 2, DecodeEBits);
  AddInstTable(InstTable, "BFINS"  , 0, DecodeBFINS);
  AddInstTable(InstTable, "CINVA"  , 0, DecodeCacheAll);
  AddInstTable(InstTable, "CPUSHA" , 1, DecodeCacheAll);
  AddInstTable(InstTable, "CINVL"  , 1, DecodeCache);
  AddInstTable(InstTable, "CPUSHL" , 5, DecodeCache);
  AddInstTable(InstTable, "CINVP"  , 2, DecodeCache);
  AddInstTable(InstTable, "CPUSHP" , 6, DecodeCache);
  AddInstTable(InstTable, "STR"    , 0, DecodeSTR);

  FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt); InstrZ = 0;
  AddFixed("NOP"    , 0x4e71, False, (1 << e68KGen1a) | (1 << e68KGen1b) | (1 << e68KGen2) | (1 << e68KGen3) | (1 << eCPU32) | (1 << eColdfire));
  AddFixed("RESET"  , 0x4e70, False, (1 << e68KGen1a) | (1 << e68KGen1b) | (1 << e68KGen2) | (1 << e68KGen3) | (1 << eCPU32));
  AddFixed("ILLEGAL", 0x4afc, False, (1 << e68KGen1a) | (1 << e68KGen1b) | (1 << e68KGen2) | (1 << e68KGen3) | (1 << eCPU32) | (1 << eColdfire));
  AddFixed("TRAPV"  , 0x4e76, False, (1 << e68KGen1a) | (1 << e68KGen1b) | (1 << e68KGen2) | (1 << e68KGen3) | (1 << eCPU32));
  AddFixed("RTE"    , 0x4e73, True , (1 << e68KGen1a) | (1 << e68KGen1b) | (1 << e68KGen2) | (1 << e68KGen3) | (1 << eCPU32) | (1 << eColdfire));
  AddFixed("RTR"    , 0x4e77, False, (1 << e68KGen1a) | (1 << e68KGen1b) | (1 << e68KGen2) | (1 << e68KGen3) | (1 << eCPU32));
  AddFixed("RTS"    , 0x4e75, False, (1 << e68KGen1a) | (1 << e68KGen1b) | (1 << e68KGen2) | (1 << e68KGen3) | (1 << eCPU32) | (1 << eColdfire));
  AddFixed("BGND"   , 0x4afa, False, (1 << eCPU32));
  AddFixed("HALT"   , 0x4ac8, True , (1 << eColdfire));
  AddFixed("PULSE"  , 0x4acc, True , (1 << eColdfire));

  AddCond("T" , 0);  AddCond("F" , 1);  AddCond("HI", 2);  AddCond("LS", 3);
  AddCond("CC", 4);  AddCond("CS", 5);  AddCond("NE", 6);  AddCond("EQ", 7);
  AddCond("VC", 8);  AddCond("VS", 9);  AddCond("PL",10);  AddCond("MI",11);
  AddCond("GE",12);  AddCond("LT",13);  AddCond("GT",14);  AddCond("LE",15);
  AddCond("HS", 4);  AddCond("LO", 5);
  AddInstTable(InstTable, "BRA", 0, DecodeBcc);
  AddInstTable(InstTable, "BSR", 1, DecodeBcc);
  AddInstTable(InstTable, "DBRA", 1, DecodeDBcc);

  FPUOps = (FPUOp *) malloc(sizeof(FPUOp) * FPUOpCnt); InstrZ = 0;
  AddFPUOp("FINT"   , 0x01, False, 0          );  AddFPUOp("FSINH"  , 0x02, False, eFlagExtFPU);
  AddFPUOp("FINTRZ" , 0x03, False, 0          );  AddFPUOp("FSQRT"  , 0x04, False, 0          );
  AddFPUOp("FSSQRT" , 0x41, False, eFlagIntFPU);  AddFPUOp("FDSQRT" , 0x45, False, eFlagIntFPU);
  AddFPUOp("FLOGNP1", 0x06, False, eFlagExtFPU);  AddFPUOp("FETOXM1", 0x08, False, eFlagExtFPU);
  AddFPUOp("FTANH"  , 0x09, False, eFlagExtFPU);  AddFPUOp("FATAN"  , 0x0a, False, eFlagExtFPU);
  AddFPUOp("FASIN"  , 0x0c, False, eFlagExtFPU);  AddFPUOp("FATANH" , 0x0d, False, eFlagExtFPU);
  AddFPUOp("FSIN"   , 0x0e, False, eFlagExtFPU);  AddFPUOp("FTAN"   , 0x0f, False, eFlagExtFPU);
  AddFPUOp("FETOX"  , 0x10, False, eFlagExtFPU);  AddFPUOp("FTWOTOX", 0x11, False, eFlagExtFPU);
  AddFPUOp("FTENTOX", 0x12, False, eFlagExtFPU);  AddFPUOp("FLOGN"  , 0x14, False, eFlagExtFPU);
  AddFPUOp("FLOG10" , 0x15, False, eFlagExtFPU);  AddFPUOp("FLOG2"  , 0x16, False, eFlagExtFPU);
  AddFPUOp("FABS"   , 0x18, False, 0          );  AddFPUOp("FSABS"  , 0x58, False, eFlagIntFPU);
  AddFPUOp("FDABS"  , 0x5c, False, eFlagIntFPU);  AddFPUOp("FCOSH"  , 0x19, False, eFlagExtFPU);
  AddFPUOp("FNEG"   , 0x1a, False, 0          );  AddFPUOp("FACOS"  , 0x1c, False, eFlagExtFPU);
  AddFPUOp("FCOS"   , 0x1d, False, eFlagExtFPU);  AddFPUOp("FGETEXP", 0x1e, False, eFlagExtFPU);
  AddFPUOp("FGETMAN", 0x1f, False, eFlagExtFPU);  AddFPUOp("FDIV"   , 0x20, True , 0          );
  AddFPUOp("FSDIV"  , 0x60, False, eFlagIntFPU);  AddFPUOp("FDDIV"  , 0x64, True , eFlagIntFPU);
  AddFPUOp("FMOD"   , 0x21, True , eFlagExtFPU);  AddFPUOp("FADD"   , 0x22, True , 0          );
  AddFPUOp("FSADD"  , 0x62, True , eFlagIntFPU);  AddFPUOp("FDADD"  , 0x66, True , eFlagIntFPU);
  AddFPUOp("FMUL"   , 0x23, True , 0          );  AddFPUOp("FSMUL"  , 0x63, True , eFlagIntFPU);
  AddFPUOp("FDMUL"  , 0x67, True , eFlagIntFPU);  AddFPUOp("FSGLDIV", 0x24, True , eFlagExtFPU);
  AddFPUOp("FREM"   , 0x25, True , eFlagExtFPU);  AddFPUOp("FSCALE" , 0x26, True , eFlagExtFPU);
  AddFPUOp("FSGLMUL", 0x27, True , eFlagExtFPU);  AddFPUOp("FSUB"   , 0x28, True , 0          );
  AddFPUOp("FSSUB"  , 0x68, True , eFlagIntFPU);  AddFPUOp("FDSUB"  , 0x6c, True , eFlagIntFPU);
  AddFPUOp("FCMP"   , 0x38, True , 0          );

  AddInstTable(InstTable, "FSAVE", 0, DecodeFSAVE);
  AddInstTable(InstTable, "FRESTORE", 0, DecodeFRESTORE);
  AddInstTable(InstTable, "FNOP", 0, DecodeFNOP);
  AddInstTable(InstTable, "FMOVE", 0, DecodeFMOVE);
  AddInstTable(InstTable, "FMOVECR", 0, DecodeFMOVECR);
  AddInstTable(InstTable, "FTST", 0, DecodeFTST);
  AddInstTable(InstTable, "FSINCOS", 0, DecodeFSINCOS);
  AddInstTable(InstTable, "FDMOVE", 0x0044, DecodeFDMOVE_FSMOVE);
  AddInstTable(InstTable, "FSMOVE", 0x0040, DecodeFDMOVE_FSMOVE);
  AddInstTable(InstTable, "FMOVEM", 0, DecodeFMOVEM);

  AddFPUCond("EQ"  , 0x01); AddFPUCond("NE"  , 0x0e);
  AddFPUCond("GT"  , 0x12); AddFPUCond("NGT" , 0x1d);
  AddFPUCond("GE"  , 0x13); AddFPUCond("NGE" , 0x1c);
  AddFPUCond("LT"  , 0x14); AddFPUCond("NLT" , 0x1b);
  AddFPUCond("LE"  , 0x15); AddFPUCond("NLE" , 0x1a);
  AddFPUCond("GL"  , 0x16); AddFPUCond("NGL" , 0x19);
  AddFPUCond("GLE" , 0x17); AddFPUCond("NGLE", 0x18);
  AddFPUCond("OGT" , 0x02); AddFPUCond("ULE" , 0x0d);
  AddFPUCond("OGE" , 0x03); AddFPUCond("ULT" , 0x0c);
  AddFPUCond("OLT" , 0x04); AddFPUCond("UGE" , 0x0b);
  AddFPUCond("OLE" , 0x05); AddFPUCond("UGT" , 0x0a);
  AddFPUCond("OGL" , 0x06); AddFPUCond("UEQ" , 0x09);
  AddFPUCond("OR"  , 0x07); AddFPUCond("UN"  , 0x08);
  AddFPUCond("F"   , 0x00); AddFPUCond("T"   , 0x0f);
  AddFPUCond("SF"  , 0x10); AddFPUCond("ST"  , 0x1f);
  AddFPUCond("SEQ" , 0x11); AddFPUCond("SNE" , 0x1e);

  AddPMMUCond("BS"); AddPMMUCond("BC"); AddPMMUCond("LS"); AddPMMUCond("LC");
  AddPMMUCond("SS"); AddPMMUCond("SC"); AddPMMUCond("AS"); AddPMMUCond("AC");
  AddPMMUCond("WS"); AddPMMUCond("WC"); AddPMMUCond("IS"); AddPMMUCond("IC");
  AddPMMUCond("GS"); AddPMMUCond("GC"); AddPMMUCond("CS"); AddPMMUCond("CC");

  AddInstTable(InstTable, "PSAVE", 0, DecodePSAVE);
  AddInstTable(InstTable, "PRESTORE", 0, DecodePRESTORE);
  AddInstTable(InstTable, "PFLUSHA", 0, DecodePFLUSHA);
  AddInstTable(InstTable, "PFLUSHAN", 0, DecodePFLUSHAN);
  AddInstTable(InstTable, "PFLUSH", 0x0000, DecodePFLUSH_PFLUSHS);
  AddInstTable(InstTable, "PFLUSHS", 0x0400, DecodePFLUSH_PFLUSHS);
  AddInstTable(InstTable, "PFLUSHN", 0, DecodePFLUSHN);
  AddInstTable(InstTable, "PFLUSHR", 0, DecodePFLUSHR);
  AddInstTable(InstTable, "PLOADR", 0x2200, DecodePLOADR_PLOADW);
  AddInstTable(InstTable, "PLOADW", 0x2000, DecodePLOADR_PLOADW);
  AddInstTable(InstTable, "PMOVE", 0x0000, DecodePMOVE_PMOVEFD);
  AddInstTable(InstTable, "PMOVEFD", 0x0100, DecodePMOVE_PMOVEFD);
  AddInstTable(InstTable, "PTESTR", 1, DecodePTESTR_PTESTW);
  AddInstTable(InstTable, "PTESTW", 0, DecodePTESTR_PTESTW);
  AddInstTable(InstTable, "PVALID", 0, DecodePVALID);

  AddInstTable(InstTable, "BITREV", 0x00c0, DecodeColdBit);
  AddInstTable(InstTable, "BYTEREV", 0x02c0, DecodeColdBit);
  AddInstTable(InstTable, "FF1", 0x04c0, DecodeColdBit);
  AddInstTable(InstTable, "STLDSR", 0x0000, DecodeSTLDSR);
  AddInstTable(InstTable, "INTOUCH", 0x0000, DecodeINTOUCH);
  AddInstTable(InstTable, "MOV3Q", 0x0000, DecodeMOV3Q);
  /* MOVEI? */
  AddInstTable(InstTable, "MVS", 0x7100, DecodeMVS_MVZ);
  AddInstTable(InstTable, "MVZ", 0x7180, DecodeMVS_MVZ);
  AddInstTable(InstTable, "SATS", 0x0000, DecodeSATS);
  AddInstTable(InstTable, "MAC" , 0x0000, DecodeMAC_MSAC);
  AddInstTable(InstTable, "MSAC", 0x0100, DecodeMAC_MSAC);
  AddInstTable(InstTable, "MACL" , 0x8000, DecodeMAC_MSAC);
  AddInstTable(InstTable, "MSACL", 0x8100, DecodeMAC_MSAC);
  AddInstTable(InstTable, "MOVCLR" , 0x0000, DecodeMOVCLR);
  AddInstTable(InstTable, "MAAAC" , 0x0001, DecodeMxxAC);
  AddInstTable(InstTable, "MASAC" , 0x0003, DecodeMxxAC);
  AddInstTable(InstTable, "MSAAC" , 0x0101, DecodeMxxAC);
  AddInstTable(InstTable, "MSSAC" , 0x0103, DecodeMxxAC);

  AddInstTable(InstTable, "CP0BCBUSY", 0xfcc0, DecodeCPBCBUSY);
  AddInstTable(InstTable, "CP1BCBUSY", 0xfec0, DecodeCPBCBUSY);
  AddInstTable(InstTable, "CP0LD", 0xfc00, DecodeCPLDST);
  AddInstTable(InstTable, "CP1LD", 0xfe00, DecodeCPLDST);
  AddInstTable(InstTable, "CP0ST", 0xfd00, DecodeCPLDST);
  AddInstTable(InstTable, "CP1ST", 0xff00, DecodeCPLDST);
  AddInstTable(InstTable, "CP0NOP", 0xfc00, DecodeCPNOP);
  AddInstTable(InstTable, "CP1NOP", 0xfe00, DecodeCPNOP);

  PMMURegs = (PMMUReg*) malloc(sizeof(PMMUReg) * PMMURegCnt);
  InstrZ = 0;
  AddPMMUReg("TC"   , eSymbolSize32Bit, 16); AddPMMUReg("DRP"  , eSymbolSize64Bit, 17);
  AddPMMUReg("SRP"  , eSymbolSize64Bit, 18); AddPMMUReg("CRP"  , eSymbolSize64Bit, 19);
  AddPMMUReg("CAL"  , eSymbolSize8Bit, 20);  AddPMMUReg("VAL"  , eSymbolSize8Bit, 21);
  AddPMMUReg("SCC"  , eSymbolSize8Bit, 22);  AddPMMUReg("AC"   , eSymbolSize16Bit, 23);
  AddPMMUReg("PSR"  , eSymbolSize16Bit, 24); AddPMMUReg("PCSR" , eSymbolSize16Bit, 25);
  AddPMMUReg("TT0"  , eSymbolSize32Bit,  2); AddPMMUReg("TT1"  , eSymbolSize32Bit,  3);
  AddPMMUReg("MMUSR", eSymbolSize16Bit, 24);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(FPUOps);
  free(PMMURegs);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_68K(void)
{
  CodeLen = 0;
  OpSize = (pCurrCPUProps->Family == eColdfire) ? eSymbolSize32Bit : eSymbolSize16Bit;
  DontPrint = False; RelPos = 2;

  if (!DecodeMoto16AttrSize(*AttrPart.Str, &OpSize, False))
    return;

  /* Nullanweisung */

  if ((*OpPart.Str == '\0') && !*AttrPart.Str && (ArgCnt == 0))
    return;

  /* Pseudoanweisungen */

  if (DecodeMoto16Pseudo(OpSize, True))
    return;

  /* Befehlszaehler ungerade ? */

  if (Odd(EProgCounter())) WrError(ErrNum_AddrNotAligned);

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static void InitCode_68K(void)
{
  SetFlag(&PMMUAvail, PMMUAvailName, False);
  SetFlag(&FullPMMU, FullPMMUName, True);
}

static Boolean IsDef_68K(void)
{
  return False;
}

static void SwitchFrom_68K(void)
{
  DeinitFields();
  ClearONOFF();
}

static void SwitchTo_68K(void *pUser)
{
  TurnWords = True;
  ConstMode = ConstModeMoto;

  PCSymbol = "*";
  HeaderID = 0x01;
  NOPCode = 0x4e71;
  DivideChars = ",";
  HasAttrs = True;
  AttrChars = ".";

  ValidSegs = (1 << SegCode);
  Grans[SegCode] = 1;
  ListGrans[SegCode] = 2;
  SegInits[SegCode] = 0;
  SegLimits[SegCode] = (LargeWord)IntTypeDefs[UInt32].Max;

  pCurrCPUProps = (const tCPUProps*)pUser;

  MakeCode = MakeCode_68K;
  IsDef = IsDef_68K;

  SwitchFrom = SwitchFrom_68K;
  InitFields();
  AddONOFF("PMMU"    , &PMMUAvail , PMMUAvailName , False);
  AddONOFF("FULLPMMU", &FullPMMU  , FullPMMUName  , False);
  AddONOFF("FPU"     , &FPUAvail  , FPUAvailName  , False);
  AddONOFF("SUPMODE" , &SupAllowed, SupAllowedName, False);
  AddMoto16PseudoONOFF();

  SetFlag(&FullPMMU, FullPMMUName, !(pCurrCPUProps->SuppFlags & eFlagIntPMMU));
  SetFlag(&DoPadding, DoPaddingName, True);
  NativeFloatSize = (pCurrCPUProps->Family == eColdfire) ? eSymbolSizeFloat64Bit : eSymbolSizeFloat96Bit;
}

static const tCtReg CtRegs_40[] =
{
  { "TC"   , 0x003 },
  { "ITT0" , 0x004 },
  { "ITT1" , 0x005 },
  { "DTT0" , 0x006 },
  { "DTT1" , 0x007 },
  { "MMUSR", 0x805 },
  { "URP"  , 0x806 },
  { "SRP"  , 0x807 },
  { "IACR0", 0x004 },
  { "IACR1", 0x005 },
  { "DACR0", 0x006 },
  { "DACR1", 0x007 },
  { NULL   , 0x000 },
},
CtRegs_2030[] =
{
  { "CAAR" , 0x802 },
  { NULL   , 0x000 },
},
CtRegs_2040[] =
{
  { "CACR" , 0x002 },
  { "MSP"  , 0x803 },
  { "ISP"  , 0x804 },
  { NULL   , 0x000 },
},
CtRegs_1040[] =
{
  { "SFC"  , 0x000 },
  { "DFC"  , 0x001 },
  { "USP"  , 0x800 },
  { "VBR"  , 0x801 },
  { NULL   , 0x000 },
};

static const tCtReg CtRegs_5202[] =
{
  { "CACR"   , 0x002 },
  { "ACR0"   , 0x004 },
  { "ACR1"   , 0x005 },
  { "VBR"    , 0x801 },
  { "SR"     , 0x80e },
  { "PC"     , 0x80f },
  { NULL     , 0x000 },
};

static const tCtReg CtRegs_5202_5204[] =
{
  { "RAMBAR" , 0xc04 },
  { "MBAR"   , 0xc0f },
  { NULL     , 0x000 },
};

static const tCtReg CtRegs_5202_5208[] =
{
  { "RGPIOBAR", 0x009},
  { "RAMBAR" , 0xc05 },
  { NULL     , 0x000 },
};

static const tCtReg CtRegs_5202_5307[] =
{
  { "ACR2"   , 0x006 },
  { "ACR3"   , 0x007 },
  { "RAMBAR0", 0xc04 },
  { "RAMBAR1", 0xc05 },
  { NULL     , 0x000 },
};

static const tCtReg CtRegs_5202_5329[] =
{
  { "RAMBAR" , 0xc05 },
  { NULL     , 0x000 },
};

static const tCtReg CtRegs_5202_5407[] =
{
  { "ACR2"   , 0x006 },
  { "ACR3"   , 0x007 },
  { "RAMBAR0", 0xc04 },
  { "RAMBAR1", 0xc05 },
  { "MBAR"   , 0xc0f },
  { NULL     , 0x000 },
};

static const tCtReg CtRegs_Cf_CPU[] =
{
  { "D0_LOAD"  , 0x080 },
  { "D1_LOAD"  , 0x081 },
  { "D2_LOAD"  , 0x082 },
  { "D3_LOAD"  , 0x083 },
  { "D4_LOAD"  , 0x084 },
  { "D5_LOAD"  , 0x085 },
  { "D6_LOAD"  , 0x086 },
  { "D7_LOAD"  , 0x087 },
  { "A0_LOAD"  , 0x088 },
  { "A1_LOAD"  , 0x089 },
  { "A2_LOAD"  , 0x08a },
  { "A3_LOAD"  , 0x08b },
  { "A4_LOAD"  , 0x08c },
  { "A5_LOAD"  , 0x08d },
  { "A6_LOAD"  , 0x08e },
  { "A7_LOAD"  , 0x08f },
  { "D0_STORE" , 0x180 },
  { "D1_STORE" , 0x181 },
  { "D2_STORE" , 0x182 },
  { "D3_STORE" , 0x183 },
  { "D4_STORE" , 0x184 },
  { "D5_STORE" , 0x185 },
  { "D6_STORE" , 0x186 },
  { "D7_STORE" , 0x187 },
  { "A0_STORE" , 0x188 },
  { "A1_STORE" , 0x189 },
  { "A2_STORE" , 0x18a },
  { "A3_STORE" , 0x18b },
  { "A4_STORE" , 0x18c },
  { "A5_STORE" , 0x18d },
  { "A6_STORE" , 0x18e },
  { "A7_STORE" , 0x18f },
  { "OTHER_A7" , 0x800 },
  { NULL       , 0x000 },
};

static const tCtReg CtRegs_Cf_EMAC[] =
{
  { "MACSR"    , 0x804 },
  { "MASK"     , 0x805 },
  { "ACC0"     , 0x806 },
  { "ACCEXT01" , 0x807 },
  { "ACCEXT23" , 0x808 },
  { "ACC1"     , 0x809 },
  { "ACC2"     , 0x80a },
  { "ACC3"     , 0x80b },
  { NULL       , 0x000 },
};

static const tCtReg CtRegs_MCF51[] =
{
  { "VBR"      , 0x801 },
  { "CPUCR"    , 0x802 },
  { NULL       , 0x000 },
};

static const tCPUProps CPUProps[] =
{
  /* 68881/68882 may be attached memory-mapped and emulated on pre-68020 devices */
  { "68008",    0x000ffffful, e68KGen1a, eCfISA_None  , eFlagExtFPU, { NULL } },
  { "68000",    0x00fffffful, e68KGen1a, eCfISA_None  , eFlagExtFPU | eFlagLogCCR, { NULL } },
  { "68010",    0x00fffffful, e68KGen1b, eCfISA_None  , eFlagExtFPU | eFlagLogCCR, { CtRegs_1040 } },
  { "68012",    0x7ffffffful, e68KGen1b, eCfISA_None  , eFlagExtFPU | eFlagLogCCR, { CtRegs_1040 } },
  { "MCF5202",  0xfffffffful, eColdfire, eCfISA_A     , eFlagIntFPU | eFlagIdxScaling, { CtRegs_5202 } },
  { "MCF5204",  0xfffffffful, eColdfire, eCfISA_A     , eFlagIntFPU | eFlagIdxScaling, { CtRegs_5202, CtRegs_5202_5204 } },
  { "MCF5206",  0xfffffffful, eColdfire, eCfISA_A     , eFlagIntFPU | eFlagIdxScaling, { CtRegs_5202, CtRegs_5202_5204 } },
  { "MCF5208",  0xfffffffful, eColdfire, eCfISA_APlus , eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5208, CtRegs_Cf_CPU, CtRegs_Cf_EMAC } }, /* V2 */
  { "MCF52274", 0xfffffffful, eColdfire, eCfISA_APlus , eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5208, CtRegs_Cf_CPU, CtRegs_Cf_EMAC } }, /* V2 */
  { "MCF52277", 0xfffffffful, eColdfire, eCfISA_APlus , eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5208, CtRegs_Cf_CPU, CtRegs_Cf_EMAC } }, /* V2 */
  { "MCF5307",  0xfffffffful, eColdfire, eCfISA_A     , eFlagIntFPU | eFlagIdxScaling | eFlagMAC, { CtRegs_5202, CtRegs_5202_5307 } }, /* V3 */
  { "MCF5329",  0xfffffffful, eColdfire, eCfISA_APlus , eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5329 } }, /* V3 */
  { "MCF5373",  0xfffffffful, eColdfire, eCfISA_APlus , eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5329 } }, /* V3 */
  { "MCF5407",  0xfffffffful, eColdfire, eCfISA_B     , eFlagBranch32 | eFlagIntFPU | eFlagIdxScaling | eFlagMAC, { CtRegs_5202, CtRegs_5202_5407 } }, /* V4 */
  { "MCF5470",  0xfffffffful, eColdfire, eCfISA_B     , eFlagBranch32 | eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5407 } }, /* V4e */
  { "MCF5471",  0xfffffffful, eColdfire, eCfISA_B     , eFlagBranch32 | eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5407 } }, /* V4e */
  { "MCF5472",  0xfffffffful, eColdfire, eCfISA_B     , eFlagBranch32 | eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5407 } }, /* V4e */
  { "MCF5473",  0xfffffffful, eColdfire, eCfISA_B     , eFlagBranch32 | eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5407 } }, /* V4e */
  { "MCF5474",  0xfffffffful, eColdfire, eCfISA_B     , eFlagBranch32 | eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5407 } }, /* V4e */
  { "MCF5475",  0xfffffffful, eColdfire, eCfISA_B     , eFlagBranch32 | eFlagIntFPU | eFlagIdxScaling | eFlagMAC | eFlagEMAC, { CtRegs_5202, CtRegs_5202_5407 } }, /* V4e */
  { "MCF51QM",  0xfffffffful, eColdfire, eCfISA_C     , eFlagBranch32 | eFlagMAC | eFlagIdxScaling | eFlagEMAC, { CtRegs_MCF51 } }, /* V1 */
  { "68332",    0xfffffffful, eCPU32   , eCfISA_None  , eFlagBranch32 | eFlagLogCCR | eFlagIdxScaling, { CtRegs_1040 } },
  { "68340",    0xfffffffful, eCPU32   , eCfISA_None  , eFlagBranch32 | eFlagLogCCR | eFlagIdxScaling, { CtRegs_1040 } },
  { "68360",    0xfffffffful, eCPU32   , eCfISA_None  , eFlagBranch32 | eFlagLogCCR | eFlagIdxScaling, { CtRegs_1040 } },
  { "68020",    0xfffffffful, e68KGen2 , eCfISA_None  , eFlagBranch32 | eFlagLogCCR | eFlagIdxScaling | eFlagExtFPU | eFlagCALLM_RTM, { CtRegs_1040, CtRegs_2040, CtRegs_2030 } },
  { "68030",    0xfffffffful, e68KGen2 , eCfISA_None  , eFlagBranch32 | eFlagLogCCR | eFlagIdxScaling | eFlagExtFPU | eFlagIntPMMU, { CtRegs_1040, CtRegs_2040, CtRegs_2030 } },
  /* setting eFlagExtFPU assumes instructions of external FPU are emulated/provided by M68040FPSP! */
  { "68040",    0xfffffffful, e68KGen3 , eCfISA_None  , eFlagBranch32 | eFlagLogCCR | eFlagIdxScaling | eFlagIntPMMU | eFlagExtFPU | eFlagIntFPU, { CtRegs_1040, CtRegs_2040, CtRegs_40 } },
  { NULL   ,    0           , e68KGen1a, eCfISA_None  , 0, { NULL } },
};

void code68k_init(void)
{
  const tCPUProps *pProp;
  for (pProp = CPUProps; pProp->pName; pProp++)
    (void)AddCPUUser(pProp->pName, SwitchTo_68K, (void*)pProp, NULL);

  AddInitPassProc(InitCode_68K);
}
