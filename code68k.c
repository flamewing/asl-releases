/* code68k.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 680x0-Familie                                               */
/*                                                                           */
/* Historie:  9. 9.1996 Grundsteinlegung                                     */
/*           14.11.1997 Coldfire-Erweiterungen                               */
/*           31. 5.1998 68040-Erweiterungen                                  */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*           17. 1.1999 automatische Laengenanpassung OutDisp                */
/*           23. 1.1999 Einen String an sich selber anzuhaengen, ist keine   */
/*                      gute Idee gewesen :-)                                */
/*           25. 1.1999 falscher Code fuer SBCD korrigiert                   */
/*            5. 7.1999 bei FMOVE FPreg, <ea> war die Modusmaske Humbug...   */
/*                      FSMOVE/FDMOVE fuer 68040 fehlten noch                */
/*            9. 7.1999 In der Bitfeld-Dekodierung war bei der Portierung    */
/*                      ein call-by-reference verlorengegangen               */
/*            3.11.1999 ...in SplitBitField auch!                            */
/*            4.11.1999 FSMOVE/DMOVE auch mit FPn als Quelle                 */
/*                      F(S/D)(ADD/SUB/MUL/DIV)                              */
/*                      FMOVEM statt FMOVE fpcr<->ea erlaubt                 */
/*           21. 1.2000 ADDX/SUBX vertauscht                                 */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                      EXG korrigiert                                       */
/*            1.10.2000 added missing chk.l                                  */
/*                      differ add(i) sub(i) cmp(i) #imm,dn                  */
/*            3.10.2000 fixed coding of register lists with start > stop     */
/*                      better auto-scaling of outer displacements           */
/*                      fix extension word for 32-bit PC-rel. displacements  */
/*                      allow PC-rel. addressing for CMP                     */
/*                      register names must be 2 chars long                  */
/*           15.10.2000 added handling of outer displacement in ()           */
/*           12.11.2000 RelPos must be 4 for MOVEM                           */
/*           2001-12-02 fixed problems with forward refs of shift arguments  */
/*                                                                           */
/*****************************************************************************/
/* $Id: code68k.c,v 1.27 2017/06/07 18:58:34 alfred Exp $                     */
/*****************************************************************************
 * $Log: code68k.c,v $
 * Revision 1.27  2017/06/07 18:58:34  alfred
 * - correct DBxx -> PDBxx (68K PMMU)
 *
 * Revision 1.26  2016/08/17 21:26:46  alfred
 * - fix some errors and warnings detected by clang
 *
 * Revision 1.25  2016/04/09 12:33:11  alfred
 * - allow automatic 16/32 bis deduction of inner displacement on 68K
 *
 * Revision 1.24  2015/08/28 17:22:27  alfred
 * - add special handling for labels following BSR
 *
 * Revision 1.23  2015/08/19 17:04:47  alfred
 * - correct handling of short BSR for 68K
 *
 * Revision 1.22  2015/08/19 16:32:32  alfred
 * - add missing FPU conditions
 *
 * Revision 1.21  2014/12/07 19:13:59  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.20  2014/12/05 11:58:15  alfred
 * - collapse STDC queries into one file
 *
 * Revision 1.19  2014/11/16 13:15:07  alfred
 * - remove some superfluous semicolons
 *
 * Revision 1.18  2014/11/16 13:05:29  alfred
 * - rework to current style
 *
 * Revision 1.17  2014/11/05 15:47:14  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.16  2012-07-19 20:30:19  alfred
 * - -
 *
 * Revision 1.15  2010/08/27 14:52:41  alfred
 * - some more overlapping strcpy() cleanups
 *
 * Revision 1.14  2010/06/13 17:48:57  alfred
 * - do not optimize BSR with zero displacement
 *
 * Revision 1.13  2010/04/17 13:14:21  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.12  2010/03/07 10:45:22  alfred
 * - generalization of Motorola disposal instructions
 *
 * Revision 1.11  2008/09/01 18:19:21  alfred
 * - allow immediate operand on BTST
 *
 * Revision 1.10  2008/08/29 19:18:49  alfred
 * - corrected a few PC-relative offsets
 *
 * Revision 1.9  2008/08/10 11:57:48  alfred
 * - handle truncated bit numbers for 68K
 *
 * Revision 1.8  2008/08/10 11:29:38  alfred
 * - fix some FPU coding errors
 *
 * Revision 1.7  2008/06/01 08:53:07  alfred
 * - forbid ADDQ/SUBQ with byte size on address registers
 *
 * Revision 1.6  2007/11/24 22:48:05  alfred
 * - some NetBSD changes
 *
 * Revision 1.5  2005/10/30 09:39:05  alfred
 * - honour .B as branch size
 *
 * Revision 1.4  2005/09/17 19:11:48  alfred
 * - allow .B/.W as branch length specifier
 *
 * Revision 1.3  2005/09/08 16:53:41  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2004/05/29 12:04:47  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/

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
#include "intconsts.h"
#include "errmsg.h"

#include "code68k.h"

typedef struct
{
  Word Code;
  Boolean MustSup;
  Word CPUMask;
} FixedOrder;

typedef struct
{
  char *Name;
  Word Code;
  CPUVar FirstCPU, LastCPU;
} CtReg;

typedef struct
{
  Byte Code;
  Boolean Dya;
  CPUVar MinCPU;
} FPUOp;

typedef struct
{
  const char *pName;
  Byte Size;
  Word Code;
} PMMUReg;

#define FixedOrderCnt 10
#define CtRegCnt 29
#define FPUOpCnt 43
#define PMMURegCnt 13

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
  Mfpcr = 4096,
};

static ShortInt OpSize;
static ShortInt RelPos;
static Boolean PMMUAvail;               /* PMMU-Befehle erlaubt? */
static Boolean FullPMMU;                /* voller PMMU-Befehlssatz? */
static Byte AdrNum;                     /* Adressierungsnummer */
static Word AdrMode;                    /* Adressierungsmodus */
static Word AdrVals[10];                /* die Worte selber */

static FixedOrder *FixedOrders;
static CtReg *CtRegs;
static FPUOp *FPUOps;
static PMMUReg *PMMURegs;

static CPUVar CPU68008, CPU68000, CPU68010, CPU68012,
              CPUCOLD,
              CPU68332, CPU68340, CPU68360,
              CPU68020, CPU68030, CPU68040;

static const Byte FSizeCodes[10] =
{
  6, 4, 0, 7, 0, 1, 5, 2, 0, 3
};

/*-------------------------------------------------------------------------*/
/* Unterroutinen */

#define CopyAdrVals(Dest) memcpy(Dest, AdrVals, AdrCnt)

static void ACheckCPU(CPUVar MinCPU)
{
  if (!ChkMinCPUExt(MinCPU, ErrNum_AddrModeNotSupported))
  {
    AdrNum = 0;
    AdrCnt = 0;
  }
}

static void CheckCPU(CPUVar MinCPU)
{
  if (!ChkMinCPU(MinCPU))
  {
    CodeLen = 0;
  }
}

static void Check020(void)
{
  if (!ChkExactCPU(CPU68020))
  {
    CodeLen = 0;
  }
}

static void Check32(void)
{
  if (ChkExactCPUList(0, CPU68332, CPU68340, CPU68360, CPUNone) < 0)
  {
    CodeLen = 0;
  }
}

static void CheckSup(void)
{
  if (!SupAllowed)
    WrError(50);
}

static Boolean CheckColdSize(void)
{
  if ((OpSize > eSymbolSize32Bit) || ((MomCPU == CPUCOLD) && (OpSize < eSymbolSize32Bit)))
  {
    WrError(1130);
    return False;
  }
  else
    return True;
}

/*-------------------------------------------------------------------------*/
/* Adressparser */

typedef enum
{
  PC, AReg, Index, indir, Disp, None
} CompType;
typedef struct
{
  String Name;
  CompType Art;
  Word ANummer, INummer;
  Boolean Long;
  Word Scale;
  ShortInt Size;
  LongInt Wert;
} AdrComp;

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
  else if (!strcasecmp(s, "SP"))
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
   if (!strcasecmp(Asc, "IC"))
     *Erg = 2;
   else if (!strcasecmp(Asc, "DC"))
     *Erg = 1;
   else if (!strcasecmp(Asc, "IC/DC"))
     *Erg = 3;
   else if (!strcasecmp(Asc, "DC/IC"))
     *Erg = 3;
   else
     return False;
   return True;
}

static Boolean DecodeCtrlReg(char *Asc, Word *Erg)
{
  Byte z;
  String Asc_N;
  CtReg *Reg;

  strmaxcpy(Asc_N, Asc, 255);
  NLS_UpString(Asc_N);
  Asc = Asc_N;

  for (z = 0, Reg = CtRegs; z < CtRegCnt; z++, Reg++)
    if (!strcmp(Reg->Name, Asc))
    {
      if ((MomCPU < Reg->FirstCPU) || (MomCPU > Reg->LastCPU))
        return False;
      *Erg = Reg->Code;
      return True;
    }
  return False;
}

static Boolean OneField(char *Asc, Word *Erg, Boolean Ab1)
{
  Boolean ValOK;

  if ((strlen(Asc) == 2) && (mytoupper(*Asc) == 'D') && (ValReg(Asc[1])))
  {
    *Erg = 0x20 + (Asc[1] - '0');
    return True;
  }
  else
  {
    *Erg = EvalIntExpression(Asc, Int8, &ValOK);
    if ((Ab1) && (*Erg == 32))
      *Erg = 0;
    return ((ValOK) && (*Erg < 32));
  }
}

static Boolean SplitBitField(char *Arg, Word *Erg)
{
  char *p;
  Word OfsVal;
  String Desc;

  p = strchr(Arg, '{');
  if (!p)
    return False;
  *p = '\0';
  strcpy(Desc, p + 1);
  if ((!*Desc) || (Desc[strlen(Desc) - 1] != '}'))
    return False;
  Desc[strlen(Desc) - 1] = '\0';

  p = strchr(Desc, ':');
  if (!p)
    return False;
  *p = '\0';
  if (!OneField(Desc, &OfsVal, False))
    return False;
  if (!OneField(p + 1, Erg, True))
    return False;
  *Erg += OfsVal << 6;
  return True;
}

static Boolean SplitSize(char *Asc, ShortInt *DispLen, unsigned OpSizeMask)
{
  ShortInt NewLen = -1;
  int l = strlen(Asc);

  if ((l > 2) && (Asc[l - 2] == '.'))
  {
    switch (mytoupper(Asc[l - 1]))
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
        WrError(1130);
        return False;
    }
    if ((*DispLen != -1) && (*DispLen != NewLen))
    {
      WrError(1131);
      return False;
    }
    *DispLen = NewLen;
    Asc[l - 2] = '\0';
  }

  return True;
}

static Boolean ClassComp(AdrComp *C)
{
  char sh[10];

  C->Art = None;
  C->ANummer = C->INummer = 0;
  C->Long = False;
  C->Scale = 0;
  C->Size = -1;
  C->Wert = 0;

  if ((*C->Name == '[') && (C->Name[strlen(C->Name) - 1] == ']'))
  {
    C->Art = indir;
    return True;
  }

  if (!strcasecmp(C->Name, "PC"))
  {
    C->Art = PC;
    return True;
  }

  sh[0] = C->Name[0];
  sh[1] = C->Name[1];
  sh[2] = '\0';
  if (CodeReg(sh, &C->ANummer))
  {
    if ((C->ANummer > 7) && (strlen(C->Name) == 2))
    {
      C->Art = AReg;
      C->ANummer -= 8;
      return True;
    }
    else
    {
      if ((strlen(C->Name)>3) && (C->Name[2] == '.'))
      {
        switch (mytoupper(C->Name[3]))
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
        strmov(C->Name + 2, C->Name + 4);
      }
      else
        C->Long = (MomCPU == CPUCOLD);
      if ((strlen(C->Name) > 3) && (C->Name[2] == '*'))
      {
        switch (C->Name[3])
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
            if (MomCPU == CPUCOLD)
              return False;
            C->Scale = 3;
            break;
          default:
            return False;
        }
        strmov(C->Name + 2, C->Name + 4);
      }
      else
        C->Scale = 0;
      C->INummer = C->ANummer;
      C->Art = Index;
      return True;
    }
  }

  C->Art = Disp;
  if (C->Name[strlen(C->Name) - 2] == '.')
  {
    switch (mytoupper(C->Name[strlen(C->Name) - 1]))
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
    C->Name[strlen(C->Name) - 2] = '\0';
  }
  else
    C->Size = -1;
  C->Art = Disp;
  return True;
}

static Boolean IsShortAdr(LongInt Adr)
{
  Word WHi = (Adr >> 16) & 0xffff,
       WLo =  Adr        & 0xffff;

  return ((WHi == 0     ) && (WLo <= 0x7fff))
      || ((WHi == 0xffff) && (WLo >= 0x8000));
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
  if ((MomCPU <= CPU68340) && (Odd(Adr)))
    WrError(180);
}

static void DecodeAbs(char *Asc, ShortInt Size)
{
  Boolean ValOK;
  LongInt HVal;
  Integer HVal16;

  AdrCnt = 0;

  FirstPassUnknown = False;
  HVal = EvalIntExpression(Asc, Int32, &ValOK);

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
        WrError(1340);
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

static void DecodeAdr(const char *Asc_O, Word Erl)
{
  Byte l, i;
  char *p;
  Word rerg;
  Byte lklamm, rklamm, lastrklamm;
  Boolean doklamm;

  AdrComp AdrComps[3], OneComp;
  Byte CompCnt;
  String OutDisp;
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
  String Asc;
  char CReg[10];

  strmaxcpy(Asc, Asc_O, 255);
  KillBlanks(Asc);
  l = strlen(Asc);
  AdrNum = 0;
  AdrCnt = 0;

  /* immediate : */

  if (*Asc == '#')
  {
    char *pAsc = Asc + 1;

    AdrNum = 11;
    AdrMode = 0x3c;
    switch (OpSize)
    {
      case eSymbolSize8Bit:
        AdrCnt = 2;
        HVal8 = EvalIntExpression(pAsc, Int8, &ValOK);
        if (ValOK)
          AdrVals[0] = (Word)((Byte) HVal8);
        break;
      case eSymbolSize16Bit:
        AdrCnt = 2;
        HVal16 = EvalIntExpression(pAsc, Int16, &ValOK);
        if (ValOK)
          AdrVals[0] = (Word) HVal16;
        break;
      case eSymbolSize32Bit:
        AdrCnt = 4;
        HVal = EvalIntExpression(pAsc, Int32, &ValOK);
        if (ValOK)
        {
          AdrVals[0] = HVal >> 16;
          AdrVals[1] = HVal & 0xffff;
        }
        break;
#ifdef HAS64
      case eSymbolSize64Bit:
        AdrCnt = 8;
        QVal = EvalIntExpression(pAsc, Int64, &ValOK);
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
        DVal = EvalFloatExpression(pAsc, Float32, &ValOK);
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
        DVal = EvalFloatExpression(pAsc, Float64, &ValOK);
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
        DVal = EvalFloatExpression(pAsc, Float64, &ValOK);
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
        DVal = EvalFloatExpression(pAsc, Float64, &ValOK);
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
        HVal8 = EvalIntExpression(pAsc, UInt4, &ValOK);
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

  if (CodeReg(Asc, &AdrMode))
  {
    AdrCnt = 0;
    AdrNum = (AdrMode >> 3) + 1;
    goto chk;
  }

  /* Gleitkommaregister direkt: */

  if (!strncasecmp(Asc, "FP", 2))
  {
    if ((strlen(Asc) == 3) && (ValReg(Asc[2])))
    {
      AdrMode = Asc[2] - '0';
      AdrCnt = 0;
      AdrNum = 12;
      goto chk;
    }
    if (!strcasecmp(Asc, "FPCR"))
    {
      AdrMode = 4;
      AdrNum = 13;
      goto chk;
    }
    if (!strcasecmp(Asc, "FPSR"))
    {
      AdrMode = 2;
      AdrNum = 13;
      goto chk;
    }
    if (!strcasecmp(Asc, "FPIAR"))
    {
      AdrMode = 1;
      AdrNum = 13;
      goto chk;
    }
  }

  /* Adressregister indirekt mit Predekrement: */

  if ((l == 5) && (*Asc == '-') && (Asc[1] == '(') && (Asc[4] == ')'))
  {
    strcpy(CReg, Asc + 2);
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

  if ((l == 5) && (*Asc == '(') && (Asc[3] == ')') && (Asc[4] == '+'))
  {
    strcpy(CReg, Asc + 1);
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
  for (p = Asc; *p != '\0'; p++)
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
        lastrklamm = p - Asc;
      }
    }
  }

  if ((lklamm == 1) && (rklamm == 1) && (lastrklamm == strlen(Asc) - 1))
  {
    /* aeusseres Displacement abspalten, Klammern loeschen: */

    p = strchr(Asc, '(');
    *p = '\0';
    strmaxcpy(OutDisp, Asc, 255);
    strmov(Asc, p + 1);
    OutDispLen = -1;
    if (!SplitSize(OutDisp, &OutDispLen, 7))
      return;
    Asc[strlen(Asc) - 1] = '\0';

    /* in Komponenten zerteilen: */

    CompCnt = 0;
    do
    {
      doklamm = True;
      p = Asc;
      do
      {
        if (*p == '[')
          doklamm = False;
        else if (*p == ']')
          doklamm = True;
        p++;
      }
      while (((!doklamm) || (*p != ',')) && (*p != '\0'));
      if (*p == '\0')
      {
        strcpy(AdrComps[CompCnt].Name, Asc);
        *Asc = '\0';
      }
      else
      {
        *p = '\0';
        strcpy(AdrComps[CompCnt].Name, Asc);
        strmov(Asc, p + 1);
      }

      /* ignore empty component */

      if (!AdrComps[CompCnt].Name[0])
        continue;
      if (!ClassComp(AdrComps + CompCnt))
      {
        WrError(1350);
        return;
      }

      /* when the base register is already occupied, we have to move a
         second address register to the index position */

      if ((CompCnt == 1) && (AdrComps[CompCnt].Art == AReg))
      {
        AdrComps[CompCnt].Art = Index;
        AdrComps[CompCnt].INummer = AdrComps[CompCnt].ANummer + 8;
        AdrComps[CompCnt].Long = False;
        AdrComps[CompCnt].Scale = 0;
        CompCnt++;
      }

      /* a displacement found inside (...), but outside [...].  Explicit
         sizes must be consistent, implicitly checked by SplitSize(). */

      else if (AdrComps[CompCnt].Art == Disp)
      {
        if (*OutDisp)
        {
          WrError(1350);
          return;
        }
        strcpy(OutDisp, AdrComps[CompCnt].Name);
        OutDispLen = AdrComps[CompCnt].Size;
      }

      /* no second index */

      else if ((AdrComps[CompCnt].Art != Index) && (CompCnt != 0))
      {
        WrError(1350);
        return;
      }

      else
       CompCnt++;
    }
    while (*Asc != '\0');
    if ((CompCnt > 2) || ((CompCnt > 1) && (AdrComps[0].Art == Index)))
    {
      WrError(1350);
      return;
    }

    /* 0. Absolut in Klammern (d) */

    if (CompCnt == 0)
    {
      DecodeAbs(OutDisp, OutDispLen);
    }

    /* 1. Variante (An....), d(An....) */

    else if (AdrComps[0].Art == AReg)
    {

      /* 1.1. Variante (An), d(An) */

      if (CompCnt == 1)
      {
        /* 1.1.1. Variante (An) */

        if ((*OutDisp == '\0') && ((Madri & Erl) != 0))
        {
          AdrMode = 0x10 + AdrComps[0].ANummer;
          AdrNum = 3;
          AdrCnt = 0;
          goto chk;
        }

        /* 1.1.2. Variante d(An) */

        else
        {
          HVal = EvalIntExpression(OutDisp, (OutDispLen < 0) || (OutDispLen >= 2) ? SInt32 : SInt16, &ValOK);
          if (!ValOK)
            return;
          if ((ValOK) && (HVal == 0) && ((Madri & Erl) != 0) && (OutDispLen == -1))
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
              ACheckCPU(CPU68332);
              goto chk;
          }
        }
      }

      /* 1.2. Variante d(An,Xi) */

      else
      {
        AdrVals[0] = (AdrComps[1].INummer << 12) + (Ord(AdrComps[1].Long) << 11) + (AdrComps[1].Scale << 9);
        AdrMode = 0x30 + AdrComps[0].ANummer;
        HVal = EvalIntExpression(OutDisp, Int32, &ValOK);
        if (ValOK)
          switch (OutDispLen)
          {
            case 0:
              if (!IsDisp8(HVal))
              {
                WrError(1320);
                ValOK = FALSE;
              }
              break;
            case 1:
              if (!IsDisp16(HVal))
              {
                WrError(1320);
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
              if (AdrComps[1].Scale != 0)
                ACheckCPU(CPUCOLD);
              goto chk;
            case 1:
              AdrNum = 7;
              AdrCnt = 4;
              AdrVals[0] += 0x120;
              AdrVals[1] = HVal & 0xffff;
              ACheckCPU(CPU68332);
              goto chk;
            case 2:
              AdrNum = 7;
              AdrCnt = 6;
              AdrVals[0] += 0x130;
              AdrVals[1] = HVal >> 16;
              AdrVals[2] = HVal & 0xffff;
              ACheckCPU(CPU68332);
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
        HVal = EvalIntExpression(OutDisp, Int32, &ValOK) - (EProgCounter() + RelPos);
        if (!ValOK)
        {
          WrError(1350);
          return;
        }
        if (OutDispLen < 0)
          OutDispLen = (IsDisp16(HVal)) ? 1 : 2;
        switch (OutDispLen)
        {
          case 1:
            AdrMode = 0x3a;
            if (!IsDisp16(HVal))
            {
              WrError(1330);
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
            ACheckCPU(CPU68332);
            goto chk;
        }
      }

      /* 2.2. Variante d(PC,Xi) */

      else
      {
        AdrVals[0] = (AdrComps[1].INummer << 12) + (Ord(AdrComps[1].Long) << 11) + (AdrComps[1].Scale << 9);
        HVal = EvalIntExpression(OutDisp, Int32, &ValOK) - (EProgCounter() + RelPos);
        if (!ValOK)
        {
          WrError(1350);
          return;
        }
        if (OutDispLen < 0)
          OutDispLen = GetDispLen(HVal);
        AdrMode = 0x3b;
        switch (OutDispLen)
        {
          case 0:
            if (!IsDisp8(HVal))
            {
              WrError(1330);
              return;
            }
            AdrVals[0] += (HVal & 0xff);
            AdrCnt = 2;
            AdrNum = 9;
            if (AdrComps[1].Scale != 0)
            ACheckCPU(CPUCOLD);
            goto chk;
          case 1:
            if (!IsDisp16(HVal))
            {
              WrError(1330);
              return;
            }
            AdrVals[0] += 0x120;
            AdrCnt = 4;
            AdrNum = 9;
            AdrVals[1] = HVal & 0xffff;
            ACheckCPU(CPU68332);
            goto chk;
          case 2:
            AdrVals[0] += 0x130;
            AdrCnt = 6;
            AdrNum = 9;
            AdrVals[1] = HVal >> 16;
            AdrVals[2] = HVal & 0xffff;
            ACheckCPU(CPU68332);
            goto chk;
        }
      }
    }

    /* 3. Variante (Xi), d(Xi) */

    else if (AdrComps[0].Art == Index)
    {
      AdrVals[0] = (AdrComps[0].INummer << 12) + (Ord(AdrComps[0].Long) << 11) + (AdrComps[0].Scale << 9) + 0x180;
      AdrMode = 0x30;
      if (*OutDisp == '\0')
      {
        AdrVals[0] = AdrVals[0] + 0x0010;
        AdrCnt = 2;
        AdrNum = 7;
        ACheckCPU(CPU68332);
        goto chk;
      }
      else
      {
        HVal = EvalIntExpression(OutDisp, (OutDispLen != 1) ? SInt32 : SInt16, &ValOK);
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
              ACheckCPU(CPU68332);
              goto chk;
            case 2:
              AdrVals[0] = AdrVals[0] + 0x0030;
              AdrNum = 7;
              AdrCnt = 6;
              AdrVals[1] = HVal >> 16;
              AdrVals[2] = HVal & 0xffff;
              ACheckCPU(CPU68332);
              goto chk;
          }
        }
      }
    }

    /* 4. Variante indirekt: */

    else if (AdrComps[0].Art == indir)
    {
      /* erst ab 68020 erlaubt */

      if (!ChkMinCPUExt(CPU68020, ErrNum_AddrModeNotSupported))
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

      strcpy(Asc, AdrComps[0].Name + 1);
      Asc[strlen(Asc) - 1] = '\0';

      /* Felder loeschen: */

      for (i = 0; i < 2; AdrComps[i++].Art = None);

      /* indirekten Ausdruck auseinanderfieseln: */

      do
      {
        /* abschneiden & klassifizieren: */

        p = strchr(Asc, ',');
        if (!p)
        {
          strcpy(OneComp.Name, Asc);
          *Asc = '\0';
        }
        else
        {
          *p = '\0';
          strcpy(OneComp.Name, Asc);
          strmov(Asc, p + 1);
        }
        if (!ClassComp(&OneComp))
        {
          WrError(1350);
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
          WrError(1350);
          return;
        }
        else
          AdrComps[i] = OneComp;
      }
      while (*Asc != '\0');

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
          HVal = EvalIntExpression(AdrComps[0].Name, Int32, &ValOK);
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
                WrError(1330);
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
          HVal = EvalIntExpression(AdrComps[0].Name, Int32, &ValOK);
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
                WrError(1330);
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

      HVal = EvalIntExpression(OutDisp, (OutDispLen == 1) ? SInt16 : SInt32, &ValOK);
      if (!ValOK)
      {
        AdrNum = 0;
        AdrCnt = 0;
        return;
      }
      if (OutDispLen == -1)
        OutDispLen = IsDisp16(HVal) ? 1 : 2;
      if (*OutDisp == '\0')
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
    if (!SplitSize(Asc, &OutDispLen, 6))
      return;
    DecodeAbs(Asc, OutDispLen);
  }

chk:
  if ((AdrNum > 0) && (!(Erl & (1 << (AdrNum - 1)))))
  {
    WrError(1350);
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

static Boolean DecodeRegList(const char *Asc_o, Word *Erg)
{
  Byte h, h2, z;
  char *p, *p2, *pAsc;
  String Asc;

  *Erg = 0;
  strmaxcpy(Asc, Asc_o, 255);
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

/*-------------------------------------------------------------------------*/
/* Dekodierroutinen: Integer-Einheit */

/* 0=MOVE 1=MOVEA */

static void DecodeMOVE(Word Index)
{
  int z;
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (!strcasecmp(ArgStr[1], "USP"))
  {
    if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1130);
    else if (ChkExcludeCPU(CPUCOLD))
    {
      DecodeAdr(ArgStr[2], Madr);
      if (AdrNum != 0)
      {
        CodeLen = 2;
        WAsmCode[0] = 0x4e68 | (AdrMode & 7);
        CheckSup();
      }
    }
  }
  else if (!strcasecmp(ArgStr[2], "USP"))
  {
    if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1130);
    else if (ChkExcludeCPU(CPUCOLD))
    {
      DecodeAdr(ArgStr[1], Madr);
      if (AdrNum != 0)
      {
        CodeLen = 2;
        WAsmCode[0] = 0x4e60 | (AdrMode & 7);
        CheckSup();
      }
    }
  }
  else if (!strcasecmp(ArgStr[1], "SR"))
  {
    if (OpSize != eSymbolSize16Bit) WrError(1130);
    else
    {
      DecodeAdr(ArgStr[2], Mdata | ((MomCPU == CPUCOLD) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mabs));
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        WAsmCode[0] = 0x40c0 | AdrMode;
        CopyAdrVals(WAsmCode + 1);
        if (MomCPU >= CPU68010)
          CheckSup();
      }
    }
  }
  else if (!strcasecmp(ArgStr[1], "CCR"))
  {
    if ((*AttrPart != '\0') && (OpSize > eSymbolSize16Bit)) WrError(1130);
    else
    {
      OpSize = eSymbolSize8Bit;
      DecodeAdr(ArgStr[2], Mdata | ((MomCPU == CPUCOLD) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mabs));
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        WAsmCode[0] = 0x42c0 | AdrMode;
        CopyAdrVals(WAsmCode + 1);
        CheckCPU(CPU68010);
      }
    }
  }
  else if (!strcasecmp(ArgStr[2], "SR"))
  {
    if (OpSize != eSymbolSize16Bit) WrError(1130);
    else
    {
      DecodeAdr(ArgStr[1], Mdata | Mimm | ((MomCPU == CPUCOLD) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs));
      if (AdrNum != 0)
      {
        CodeLen = 2 + AdrCnt;
        WAsmCode[0] = 0x46c0 | AdrMode;
        CopyAdrVals(WAsmCode + 1);
        CheckSup();
      }
    }
  }
  else if (!strcasecmp(ArgStr[2], "CCR"))
  {
    if ((*AttrPart != '\0') && (OpSize > eSymbolSize16Bit)) WrError(1130);
    else
    {
      OpSize = eSymbolSize8Bit;
      DecodeAdr(ArgStr[1], Mdata | Mimm | ((MomCPU == CPUCOLD) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs));
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
    if (OpSize > eSymbolSize32Bit) WrError(1130);
    else
    {
      DecodeAdr(ArgStr[1], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
      if (AdrNum != 0)
      {
        z = AdrCnt;
        CodeLen = 2 + z;
        CopyAdrVals(WAsmCode + 1);
        if (OpSize == eSymbolSize8Bit)
          WAsmCode[0] = 0x1000;
        else if (OpSize == eSymbolSize16Bit)
          WAsmCode[0] = 0x3000;
        else
          WAsmCode[0] = 0x2000;
        WAsmCode[0] |= AdrMode;
        DecodeAdr(ArgStr[2], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
        if (AdrMode != 0)
        {
          if ((MomCPU == CPUCOLD) && (z > 0) && (AdrCnt > 0)) WrError(1350);
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

  if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1130);
  else if (!ChkArgCnt(2, 2));
  else
  {
    DecodeAdr(ArgStr[2], Madr);
    if (AdrNum != 0)
    {
      OpSize = eSymbolSize8Bit;
      WAsmCode[0] = 0x41c0 | ((AdrMode & 7) << 9);
      DecodeAdr(ArgStr[1], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
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

  if (ArgCnt == 1)
  {
    strcpy(ArgStr[2], ArgStr[1]);
    strcpy(ArgStr[1], "#1");
    ArgCnt = 2;
  }
  if (!ChkArgCnt(2, 2));
  else if ((*OpPart.Str == 'R') && (!ChkExcludeCPU(CPUCOLD)));
  else
  {
    DecodeAdr(ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum == 1)
    {
      if (CheckColdSize())
      {
        WAsmCode[0] = 0xe000 | AdrMode | (Op << 3) | (OpSize << 6) | (LFlag << 8);
        OpSize = 8;
        DecodeAdr(ArgStr[1], Mdata | Mimm);
        if ((AdrNum == 1) || ((AdrNum == 11) && (Lo(AdrVals[0]) >= 1) && (Lo(AdrVals[0]) <= 8)))
        {
          CodeLen = 2;
          WAsmCode[0] |= (AdrNum == 1) ? 0x20 | (AdrMode << 9) : ((AdrVals[0] & 7) << 9);
        }
        else
          WrXErrorPos(ErrNum_InvShiftArg, ArgStr[1], &ArgStrPos[1]);
      }
    }
    else if (AdrNum != 0)
    {
      if (MomCPU == CPUCOLD) WrError(1350);
      else
      {
        if (OpSize != eSymbolSize16Bit) WrError(1130);
        else
        {
          char *pVal = ArgStr[1];

          WAsmCode[0] = 0xe0c0 | AdrMode | (Op << 9) | (LFlag << 8);
          CopyAdrVals(WAsmCode + 1);
          if (*pVal == '#')
            pVal++;
          HVal8 = EvalIntExpression(pVal, Int8, &ValOK);
          if ((ValOK) && (HVal8 == 1))
            CodeLen = 2 + AdrCnt;
          else
            WrXErrorPos(ErrNum_Range18, ArgStr[1], &ArgStrPos[1]);
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
  char *pArg1;

  if (!CheckColdSize())
    return;

  if (!ChkArgCnt(2, 2))
    return;

  DecodeAdr(ArgStr[2], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
  if (!AdrNum)
    return;

  if ((2 == AdrNum) && (eSymbolSize8Bit == OpSize))
  {
    WrError(1130);
    return;
  }

  WAsmCode[0] = 0x5000 | AdrMode | (OpSize << 6) | (Index << 8);
  CopyAdrVals(WAsmCode + 1);
  pArg1 = ArgStr[1];
  if ('#' == *pArg1)
    pArg1++;
  FirstPassUnknown = False;
  HVal8 = EvalIntExpression(pArg1, UInt4, &ValOK);
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
      DecodeAdr(ArgStr[1], Mdata | Mpre);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x9100 | (OpSize << 6) | (AdrMode & 7) | (Index << 14);
        if (AdrNum == 5)
          WAsmCode[0] |= 8;
        DecodeAdr(ArgStr[2], 1 << (AdrNum - 1));
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

  if (OpSize > eSymbolSize32Bit) WrError(1130);
  else if (!ChkArgCnt(2, 2));
  else if (ChkExcludeCPU(CPUCOLD))
  {
    DecodeAdr(ArgStr[1], Mpost);
    if (AdrNum == 4)
    {
      WAsmCode[0] = 0xb108 | (OpSize << 6) | (AdrMode & 7);
      DecodeAdr(ArgStr[2], Mpost);
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
  char Variant = mytoupper(OpPart.Str[strlen(OpPart.Str) - 1]);
  Word DestMask, SrcMask;

  if ('I' == Variant)
    SrcMask = Mimm;
  else
    SrcMask = Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm;

  if ('A' == Variant)
    DestMask = Madr;
  else
  {
    DestMask = Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs;

    /* since CMP only reads operands, PC-relative addressing is also
       allowed for the second operand */

    if (mytoupper(*OpPart.Str) == 'C')
      DestMask |= Mpc | Mpcidx;
  }

  if (CheckColdSize())
  {
    if (ChkArgCnt(2, 2))
    {
      DecodeAdr(ArgStr[2], DestMask);
      switch (AdrNum)
      {
        case 2: /* ADDA/SUBA/CMPA ? */
          if (OpSize == eSymbolSize8Bit) WrError(1130);
          else
          {
            WAsmCode[0] = 0x90c0 | ((AdrMode & 7) << 9) | (Op << 13);
            if (OpSize == eSymbolSize32Bit) WAsmCode[0] |= 0x100;
            DecodeAdr(ArgStr[1], SrcMask);
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
          DecodeAdr(ArgStr[1], SrcMask);
          if (AdrNum != 0)
          {
            if ((AdrNum == 11) && (Variant == 'I'))
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
          DecodeAdr(ArgStr[1], Mdata | Mimm);
          if (AdrNum == 11)        /* ADDI/SUBI/CMPI ? */
          {
            /* we have to set the PC offset before we decode the destination operand.  Luckily,
               this is only needed afterwards for an immediate source operand, so we know the
               # of words ahead: */

            if (*ArgStr[1] == '#')
              RelPos += (OpSize == eSymbolSize32Bit) ? 4 : 2;

            if (Op == 1) Op = 8;
            WAsmCode[0] = 0x400 | (OpSize << 6) | (Op << 8);
            CodeLen = 2 + AdrCnt;
            CopyAdrVals(WAsmCode + 1);
            if (MomCPU == CPUCOLD) DecodeAdr(ArgStr[2], Mdata);
            else DecodeAdr(ArgStr[2], DestMask);
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
            if (Op == 1) WrError(1420);
            else
            {
              WAsmCode[0] = 0x9100 | (OpSize << 6) | (AdrMode << 9) | (Op << 13);
              DecodeAdr(ArgStr[2], DestMask);
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
  char Variant = mytoupper(OpPart.Str[strlen(OpPart.Str) - 1]);

  if (!ChkArgCnt(2, 2));
  else if (CheckColdSize())
  {
    if ((strcasecmp(ArgStr[2], "CCR")) && (strcasecmp(ArgStr[2], "SR")))
      DecodeAdr(ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (!strcasecmp(ArgStr[2], "CCR"))     /* AND #...,CCR */
    {
      if ((*AttrPart != '\0') && (OpSize != eSymbolSize8Bit)) WrError(1130);
      else if (ChkExcludeCPU(CPU68008) && ChkExcludeCPU(CPUCOLD))
      {
        WAsmCode[0] = 0x003c | (Op << 9);
        OpSize = eSymbolSize8Bit;
        DecodeAdr(ArgStr[1], Mimm);
        if (AdrNum != 0)
        {
          CodeLen = 4;
          WAsmCode[1] = AdrVals[0];
        }
      }
    }
    else if (!strcasecmp(ArgStr[2], "SR")) /* AND #...,SR */
    {
      if ((*AttrPart != '\0') && (OpSize != eSymbolSize16Bit)) WrError(1130);
      else if (ChkExcludeCPU(CPUCOLD))
      {
        WAsmCode[0] = 0x007c | (Op << 9);
        OpSize = eSymbolSize16Bit;
        DecodeAdr(ArgStr[1], Mimm);
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
      DecodeAdr(ArgStr[1], ((Variant == 'I') ? 0 : Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm) | Mimm);
      if (AdrNum != 0)
      {
        if ((AdrNum == 11) && (Variant == 'I'))
          WAsmCode[0] = (OpSize << 6) | (Op << 9) | Reg;
        else
          WAsmCode[0] |= AdrMode;
        CodeLen = 2 + AdrCnt;
        CopyAdrVals(WAsmCode + 1);
      }
    }
    else if (AdrNum != 0)                 /* AND ...,<EA> */
    {
      DecodeAdr(ArgStr[1], Mdata | Mimm);
      if (AdrNum == 11)                   /* AND #..,<EA> */
      {
        WAsmCode[0] = (OpSize << 6) | (Op << 9);
        CodeLen = 2 + AdrCnt;
        CopyAdrVals(WAsmCode + 1);
        DecodeAdr(ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
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
        DecodeAdr(ArgStr[2], Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
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
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (!strcasecmp(ArgStr[2], "CCR"))
  {
    if ((*AttrPart != '\0') && (OpSize != eSymbolSize8Bit)) WrError(1130);
    else if (ChkExcludeCPU(CPUCOLD))
    {
      WAsmCode[0] = 0xa3c;
      OpSize = eSymbolSize8Bit;
      DecodeAdr(ArgStr[1], Mimm);
      if (AdrNum != 0)
      {
        CodeLen = 4;
        WAsmCode[1] = AdrVals[0];
      }
    }
  }
  else if (!strcasecmp(ArgStr[2], "SR"))
  {
    if (OpSize != eSymbolSize16Bit) WrError(1130);
    else if (ChkExcludeCPU(CPUCOLD))
    {
      WAsmCode[0] = 0xa7c;
      DecodeAdr(ArgStr[1], Mimm);
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
    DecodeAdr(ArgStr[1], Mdata | Mimm);
    if (AdrNum == 1)
    {
      WAsmCode[0] = 0xb100 | (AdrMode << 9) | (OpSize << 6);
      DecodeAdr(ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
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
      DecodeAdr(ArgStr[2], Mdata | ((MomCPU == CPUCOLD) ? 0 : Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs));
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

  if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1100);
  else if (ChkArgCnt(1, 1))
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(ArgStr[1], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
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
  Word w1;

  if (OpSize > eSymbolSize32Bit) WrError(1130);
  else if (ChkArgCnt(1, 1))
  {
    w1 = Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs;
    if ((Index == 1) && (MomCPU >= CPU68332))
    {
      w1 |= Mpc | Mpcidx | Mimm;
      if (OpSize != eSymbolSize8Bit)
        w1 |= Madr;
    }
    DecodeAdr(ArgStr[1], w1);
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
  if (*AttrPart != '\0') WrError(1130);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
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
  if ((*AttrPart != '\0') && (OpSize != eSymbolSize8Bit)) WrError(1130);
  else if (!ChkArgCnt(1, 1));
  else if (ChkExcludeCPU(CPUCOLD))
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
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
  if (ChkArgCnt(1, 1))
  if (CheckColdSize())
  {
    if (MomCPU == CPUCOLD) DecodeAdr(ArgStr[1], Mdata);
    else DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
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

  if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1130);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], Mdata);
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

  if (*AttrPart != '\0') WrError(1130);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], Madr);
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
  else if ((OpSize == eSymbolSize8Bit) || (OpSize > eSymbolSize32Bit)) WrError(1130);
  else
  {
    DecodeAdr(ArgStr[1], Mdata);
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
  else if (!ChkExcludeCPU(CPUCOLD));
  else if (OpSize > eSymbolSize32Bit) WrError(1130);
  else
  {
    DecodeAdr(ArgStr[1], Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
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
   && ChkExcludeCPU(CPUCOLD)
   && CheckColdSize())
  {
    DecodeAdr(ArgStr[1], Madri | Mdadri);
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

  if (*AttrPart != '\0') WrError(1100);
  else if (!ChkArgCnt(0, 0));
  else if (ChkExactCPUMask(FixedZ->CPUMask, CPU68008))
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
  else if ((OpSize < eSymbolSize16Bit) || (OpSize > eSymbolSize32Bit)) WrError(1130);
  else if ((MomCPU == CPUCOLD) && (OpSize == 1)) WrError(1130);
  else
  {
    RelPos = 4;
    if (DecodeRegList(ArgStr[2], WAsmCode + 1))
    {
      DecodeAdr(ArgStr[1], Madri | Mdadri | ((MomCPU == CPUCOLD) ? 0 : Mpost | Maix | Mpc | Mpcidx | Mabs));
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x4c80 | AdrMode | ((OpSize - 1) << 6);
        CodeLen = 4 + AdrCnt; CopyAdrVals(WAsmCode + 2);
      }
    }
    else if (DecodeRegList(ArgStr[1], WAsmCode + 1))
    {
      DecodeAdr(ArgStr[2], Madri | Mdadri  | ((MomCPU == CPUCOLD) ? 0 : Mpre | Maix | Mabs));
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
    else WrError(1410);
  }
}

static void DecodeMOVEQ(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1130);
  else
  {
    DecodeAdr(ArgStr[2], Mdata);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x7000 | (AdrMode << 9);
      OpSize = eSymbolSize8Bit;
      DecodeAdr(ArgStr[1], Mimm);
      if (AdrNum != 0)
      {
        CodeLen = 2;
        WAsmCode[0] |= AdrVals[0];
      }
    }
  }
}

static void DecodeSTOP(Word Index)
{
  Word HVal;
  Boolean ValOK;
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1100);
  else if (!ChkArgCnt(1, 1));
  else if (*ArgStr[1] != '#') WrError(1120);
  else
  {
    HVal = EvalIntExpression(ArgStr[1] + 1, Int16, &ValOK);
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

  if (*AttrPart != '\0') WrError(1100);
  else if (!ChkArgCnt(1, 1));
  else if (*ArgStr[1] != '#') WrError(1120);
  else
  {
    HVal = EvalIntExpression(ArgStr[1] + 1, Int16, &ValOK);
    if (ValOK)
    {
      CodeLen = 6;
      WAsmCode[0] = 0xf800;
      WAsmCode[1] = 0x01c0;
      WAsmCode[2] = HVal;
      CheckSup(); Check32();
    }
  }
}

static void DecodeTRAP(Word Index)
{
  Byte HVal8;
  Boolean ValOK;
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1100);
  else if (!ChkArgCnt(1, 1));
  else if (*ArgStr[1] != '#') WrError(1120);
  else
  {
    HVal8 = EvalIntExpression(ArgStr[1] + 1, Int4, &ValOK);
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

  if (*AttrPart != '\0') WrError(1100);
  else if (!ChkArgCnt(1, 1));
  else if (!ChkExcludeCPU(CPUCOLD));
  else if (*ArgStr[1] != '#') WrError(1120);
  else
  {
    HVal8 = EvalIntExpression(ArgStr[1] + 1, UInt3, &ValOK);
    if (ValOK)
    {
      CodeLen = 2;
      WAsmCode[0] = 0x4848 + (HVal8 & 7);
      CheckCPU(CPU68010);
    }
  }
  UNUSED(Index);
}

static void DecodeRTD(Word Index)
{
  Word HVal;
  Boolean ValOK;
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1100);
  else if (!ChkArgCnt(1, 1));
  else if (!ChkExcludeCPU(CPUCOLD));
  else if (*ArgStr[1] != '#') WrError(1120);
  else
  {
    HVal = EvalIntExpression(ArgStr[1] + 1, Int16, &ValOK);
    if (ValOK)
    {
      CodeLen = 4;
      WAsmCode[0] = 0x4e74;
      WAsmCode[1] = HVal;
      CheckCPU(CPU68010);
    }
  }
}

static void DecodeEXG(Word Index)
{
  Word HReg;
  UNUSED(Index);

  if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1130);
  else if (ChkArgCnt(2, 2)
        && ChkExcludeCPU(CPUCOLD))
  {
    DecodeAdr(ArgStr[1], Mdata | Madr);
    if (AdrNum == 1)
    {
      WAsmCode[0] = 0xc100 | (AdrMode << 9);
      DecodeAdr(ArgStr[2], Mdata | Madr);
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
      DecodeAdr(ArgStr[2], Mdata | Madr);
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

  if (*AttrPart != '\0') WrError(1100);
  else if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1], Mpost | Madri | Mabs);
    if (AdrNum != 0)
    {
      w1 = AdrNum;
      z = AdrMode & 7;
      if ((w1 == 10) && (AdrCnt == 2))
      {
        AdrVals[1] = AdrVals[0];
        AdrVals[0] = 0 - (AdrVals[1] >> 15);
      }
      DecodeAdr(ArgStr[2], Mpost | Madri | Mabs);
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
            WrError(1350);
            CodeLen = 0;
          }
        }
        if (CodeLen > 0)
          CheckCPU(CPU68040);
      }
    }
  }
}

static void DecodeCacheAll(Word Index)
{
  Word w1;

  if (*AttrPart != '\0') WrError(1100);
  else if (!ChkArgCnt(1, 1));
  else if (!CodeCache(ArgStr[1], &w1)) WrXErrorPos(ErrNum_InvCtrlReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    WAsmCode[0] = 0xf418 + (w1 << 6) + (Index << 5);
    CodeLen = 2;
    CheckCPU(CPU68040);
    CheckSup();
  }
}

static void DecodeCache(Word Index)
{
  Word w1;

  if (*AttrPart != '\0') WrError(1100);
  else if (!ChkArgCnt(2, 2));
  else if (!CodeCache(ArgStr[1], &w1)) WrXErrorPos(ErrNum_InvCtrlReg, ArgStr[1], &ArgStrPos[1]);
  else
  {
    DecodeAdr(ArgStr[2], Madri);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf400 + (w1 << 6) + (Index << 3) + (AdrMode & 7);
      CodeLen = 2;
      CheckCPU(CPU68040);
      CheckSup();
    }
  }
}

static void DecodeMUL_DIV(Word Code)
{

  if (!ChkArgCnt(2, 2));
  else if ((*OpPart.Str == 'D') && !ChkExcludeCPU(CPUCOLD));
  else if (OpSize == eSymbolSize16Bit)
  {
    DecodeAdr(ArgStr[2], Mdata);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x80c0 | (AdrMode << 9) | (Code & 0x0100);
      if (!(Code & 1))
        WAsmCode[0] |= 0x4000;
      DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
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

    if (strchr(ArgStr[2], ':'))
      OK = CodeRegPair(ArgStr[2], &w1, &w2);
    else
    {
      OK = CodeReg(ArgStr[2], &w1) && (w1 < 8);
      w2 = w1;
    }
    if (!OK) WrXErrorPos(ErrNum_InvRegPair, ArgStr[2], &ArgStrPos[2]);
    else
    {
      WAsmCode[1] = w1 | (w2 << 12) | ((Code & 0x0100) << 3);
      RelPos = 4;
      if (w1 != w2)
        WAsmCode[1] |= 0x400;
      DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x4c00 + AdrMode + (Lo(Code) << 6);
        CopyAdrVals(WAsmCode + 2);
        CodeLen = 4 + AdrCnt;
        CheckCPU((w1 != w2) ? CPU68332 : CPUCOLD);
      }
    }
  }
  else
    WrError(1130);
}

static void DecodeDIVL(Word Index)
{
  Word w1, w2;

  if (*AttrPart == '\0')
    OpSize = eSymbolSize32Bit;
  if (!ChkArgCnt(2, 2));
  else if (OpSize != eSymbolSize32Bit) WrError(1130);
  else if (!CodeRegPair(ArgStr[2], &w1, &w2)) WrXErrorPos(ErrNum_InvRegPair, ArgStr[2], &ArgStrPos[2]);
  else
  {
    RelPos = 4;
    WAsmCode[1] = w1 | (w2 << 12) | (Index << 11);
    DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x4c40 + AdrMode;
      CopyAdrVals(WAsmCode + 2); CodeLen = 4 + AdrCnt;
      CheckCPU(CPU68332);
    }
  }
}

static void DecodeASBCD(Word Index)
{
  if ((OpSize != eSymbolSize8Bit) && (*AttrPart != '\0')) WrError(1130);
  else if (ChkArgCnt(2, 2)
        && ChkExcludeCPU(CPUCOLD))
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(ArgStr[1], Mdata | Mpre);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x8100 | (AdrMode & 7) | (Index << 14) | ((AdrNum == 5) ? 8 : 0);
      DecodeAdr(ArgStr[2], 1 << (AdrNum - 1));
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

  if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit)) WrError(1130);
  else if (ChkArgCnt(2, 2)
        && ChkExcludeCPU(CPUCOLD))
  {
    DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x4000 | AdrMode | ((4 - OpSize) << 7);
      CodeLen = 2 + AdrCnt;
      CopyAdrVals(WAsmCode + 1);
      DecodeAdr(ArgStr[2], Mdata);
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

  if ((*AttrPart == '\0') && (MomCPU == CPUCOLD)) OpSize = eSymbolSize16Bit;
  if ((OpSize < 1) || (OpSize > 2)) WrError(1130);
  else if ((OpSize == eSymbolSize32Bit) && !ChkMinCPU(CPU68332));
  else if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1], Madr);
    if (AdrNum != 0)
    {
      WAsmCode[0] = (OpSize == eSymbolSize16Bit) ? 0x4e50 : 0x4808;
      WAsmCode[0] += AdrMode & 7;
      DecodeAdr(ArgStr[2], Mimm);
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

  if ((OpSize == eSymbolSize8Bit) || (OpSize > eSymbolSize32Bit)) WrError(1130);
  else if (ChkArgCnt(2, 2)
        && ChkExcludeCPU(CPUCOLD))
  {
    DecodeAdr(ArgStr[1], Mdata | Mdadri);
    if (AdrNum == 1)
    {
      WAsmCode[0] = 0x188 | ((OpSize - 1) << 6) | (AdrMode << 9);
      DecodeAdr(ArgStr[2], Mdadri);
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
      DecodeAdr(ArgStr[2], Mdata);
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

  if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1130);
  else if (ChkArgCnt(2, 2))
  {
    if (DecodeCtrlReg(ArgStr[1], WAsmCode + 1))
    {
      DecodeAdr(ArgStr[2], Mdata | Madr);
      if (AdrNum != 0)
      {
        CodeLen = 4;
        WAsmCode[0] = 0x4e7a;
        WAsmCode[1] |= AdrMode << 12;
        CheckSup();
      }
    }
    else if (DecodeCtrlReg(ArgStr[2], WAsmCode + 1))
    {
      DecodeAdr(ArgStr[1], Mdata | Madr);
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
  else if (OpSize > eSymbolSize32Bit) WrError(1130);
  else if (ChkExcludeCPU(CPUCOLD))
  {
    DecodeAdr(ArgStr[1], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if ((AdrNum == 1) || (AdrNum == 2))
    {
      WAsmCode[1] = 0x800 | (AdrMode << 12);
      DecodeAdr(ArgStr[2], Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0xe00 | AdrMode | (OpSize << 6);
        CodeLen = 4 + AdrCnt;
        CopyAdrVals(WAsmCode + 2);
        CheckSup();
        CheckCPU(CPU68010);
      }
    }
    else if (AdrNum != 0)
    {
      WAsmCode[0] = 0xe00 | AdrMode | (OpSize << 6);
      CodeLen = 4 + AdrCnt;
      CopyAdrVals(WAsmCode + 2);
      DecodeAdr(ArgStr[2], Mdata | Madr);
      if (AdrNum != 0)
      {
        WAsmCode[1] = AdrMode << 12;
        CheckSup();
        CheckCPU(CPU68010);
      }
      else
        CodeLen = 0;
    }
  }
}

static void DecodeCALLM(Word Index)
{
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1130);
  else if (ChkArgCnt(2, 2))
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(ArgStr[1], Mimm);
    if (AdrNum != 0)
    {
      WAsmCode[1] = AdrVals[0];
      RelPos = 4;
      DecodeAdr(ArgStr[2], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x06c0 + AdrMode;
        CopyAdrVals(WAsmCode + 2);
        CodeLen = 4 + AdrCnt;
        CheckCPU(CPU68020);
        Check020();
      }
    }
  }
}

static void DecodeCAS(Word Index)
{
  UNUSED(Index);

  if (OpSize > eSymbolSize32Bit) WrError(1130);
  else if (ChkArgCnt(3, 3))
  {
    DecodeAdr(ArgStr[1], Mdata);
    if (AdrNum != 0)
    {
      WAsmCode[1] = AdrMode;
      DecodeAdr(ArgStr[2], Mdata);
      if (AdrNum != 0)
      {
        RelPos = 4;
        WAsmCode[1] += (((Word)AdrMode) << 6);
        DecodeAdr(ArgStr[3], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
        if (AdrNum != 0)
        {
          WAsmCode[0] = 0x08c0 + AdrMode + (((Word)OpSize + 1) << 9);
          CopyAdrVals(WAsmCode + 2);
          CodeLen = 4 + AdrCnt;
          CheckCPU(CPU68020);
        }
      }
    }
  }
}

static void DecodeCAS2(Word Index)
{
  Word w1, w2;
  UNUSED(Index);

  if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit)) WrError(1130);
  else if (!ChkArgCnt(3, 3));
  else if (!CodeRegPair(ArgStr[1], WAsmCode + 1, WAsmCode + 2)) WrXErrorPos(ErrNum_InvRegPair, ArgStr[1], &ArgStrPos[1]);
  else if (!CodeRegPair(ArgStr[2], &w1, &w2)) WrXErrorPos(ErrNum_InvRegPair, ArgStr[2], &ArgStrPos[2]);
  else
  {
    WAsmCode[1] += (w1 << 6);
    WAsmCode[2] += (w2 << 6);
    if (!CodeIndRegPair(ArgStr[3], &w1, &w2)) WrXErrorPos(ErrNum_InvRegPair, ArgStr[3], &ArgStrPos[3]);
    else
    {
      WAsmCode[1] += (w1 << 12);
      WAsmCode[2] += (w2 << 12);
      WAsmCode[0] = 0x0cfc + (((Word)OpSize - 1) << 9);
      CodeLen = 6;
      CheckCPU(CPU68020);
    }
  }
}

static void DecodeCMPCHK2(Word Index)
{
  if (OpSize > eSymbolSize32Bit) WrError(1130);
  else if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[2], Mdata | Madr);
    if (AdrNum != 0)
    {
      RelPos = 4;
      WAsmCode[1] = (((Word)AdrMode) << 12) | (Index << 11);
      DecodeAdr(ArgStr[1], Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0x00c0 + (((Word)OpSize) << 9) + AdrMode;
        CopyAdrVals(WAsmCode + 2);
        CodeLen = 4 + AdrCnt;
        CheckCPU(CPU68332);
      }
    }
  }
}

static void DecodeEXTB(Word Index)
{
  UNUSED(Index);

  if ((OpSize != eSymbolSize32Bit) && (*AttrPart != '\0')) WrError(1130);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], Mdata);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x49c0 + AdrMode;
      CodeLen = 2;
      CheckCPU(CPU68332);
    }
  }
}

static void DecodePACK(Word Index)
{
  if (!ChkArgCnt(3, 3));
  else if (*AttrPart != '\0') WrError(1130);
  else
  {
    DecodeAdr(ArgStr[1], Mdata | Mpre);
    if (AdrNum != 0)
    {
      WAsmCode[0] = (0x8140 + (Index << 6)) | (AdrMode & 7);
      if (AdrNum == 5)
        WAsmCode[0] += 8;
      DecodeAdr(ArgStr[2], 1 << (AdrNum - 1));
      if (AdrNum != 0)
      {
        WAsmCode[0] |= ((AdrMode & 7) << 9);
        DecodeAdr(ArgStr[3], Mimm);
        if (AdrNum != 0)
        {
          WAsmCode[1] = AdrVals[0];
          CodeLen = 4;
          CheckCPU(CPU68020);
        }
      }
    }
  }
}

static void DecodeRTM(Word Index)
{
  UNUSED(Index);

  if (*AttrPart != '\0') WrError(1130);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], Mdata | Madr);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0x06c0 + AdrMode;
      CodeLen = 2;
      CheckCPU(CPU68020);
      Check020();
    }
  }
}

static void DecodeTBL(Word Index)
{
  char *p;
  Word w2, Mode;

  if (!ChkArgCnt(2, 2));
  else if (OpSize > eSymbolSize32Bit) WrError(1130);
  else if (ChkMinCPU(CPU68332))
  {
    DecodeAdr(ArgStr[2], Mdata);
    if (AdrNum != 0)
    {
      Mode = AdrMode;
      p = strchr(ArgStr[1], ':');
      if (!p)
      {
        RelPos = 4;
        DecodeAdr(ArgStr[1], Madri | Mdadri | Maix| Mabs | Mpc | Mpcidx);
        if (AdrNum != 0)
        {
          WAsmCode[0] = 0xf800 | AdrMode;
          WAsmCode[1] = 0x0100 | (OpSize << 6) | (Mode << 12) | (Index << 10);
          memcpy(WAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 4 + AdrCnt;
          Check32();
        }
      }
      else
      {
        strcpy(ArgStr[3], p + 1);
        *p = '\0';
        DecodeAdr(ArgStr[1], Mdata);
        if (AdrNum != 0)
        {
          w2 = AdrMode;
          DecodeAdr(ArgStr[3], Mdata);
          if (AdrNum != 0)
          {
            WAsmCode[0] = 0xf800 | w2;
            WAsmCode[1] = 0x0000 | (OpSize << 6) | (Mode << 12) | AdrMode;
            if (OpPart.Str[3] == 'S')
              WAsmCode[1] |= 0x0800;
            if (OpPart.Str[strlen(OpPart.Str) - 1] == 'N')
              WAsmCode[1] |= 0x0400;
            CodeLen = 4;
            Check32();
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
  DecodeAdr(ArgStr[1], Mdata | Mimm);
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
  if (*AttrPart == '\0')
    OpSize = eSymbolSize8Bit;

  Mask = Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs;
  if (!Index)
    Mask |= Mpc | Mpcidx | Mimm;
  RelPos = ResCodeLen << 1;
  DecodeAdr(ArgStr[2], Mask);

  if (*AttrPart == '\0')
    OpSize = (AdrNum == 1) ? eSymbolSize32Bit : eSymbolSize8Bit;
  if (!AdrNum)
    return;
  if (((AdrNum == 1) && (OpSize != eSymbolSize32Bit)) || ((AdrNum != 1) && (OpSize != eSymbolSize8Bit)))
  {
    WrError(1130);
    return;
  }

  BitMax = (AdrNum == 1) ? 31 : 7;
  WAsmCode[0] |= AdrMode;
  CopyAdrVals(WAsmCode + ResCodeLen);
  CodeLen = (ResCodeLen << 1) + AdrCnt;
  if (!BitNumUnknown && (BitNum > BitMax))
    WrError(300);
}

/* 0=BFTST 1=BFCHG 2=BFCLR 3=BFSET */

static void DecodeFBits(Word Index)
{
  if (!ChkArgCnt(1, 1));
  else if (*AttrPart != '\0') WrError(1130);
  else if (!SplitBitField(ArgStr[1], WAsmCode + 1)) WrError(1750);
  else
  {
    RelPos = 4;
    OpSize = eSymbolSize8Bit;
    if (Memo("BFTST")) DecodeAdr(ArgStr[1], Mdata | Madri | Mdadri | Maix | Mpc | Mpcidx | Mabs);
    else DecodeAdr(ArgStr[1], Mdata | Madri | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xe8c0 | AdrMode | (Index << 10);
      CopyAdrVals(WAsmCode + 2); CodeLen = 4 + AdrCnt;
      CheckCPU(CPU68020);
    }
  }
}

/* 0=BFEXTU 1=BFEXTS 2=BFFFO */

static void DecodeEBits(Word Index)
{
  if (!ChkArgCnt(2, 2));
  else if (*AttrPart != '\0') WrError(1130);
  else if (!SplitBitField(ArgStr[1], WAsmCode + 1)) WrError(1750);
  else
  {
    RelPos = 4;
    OpSize = eSymbolSize8Bit;
    DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xe9c0 + AdrMode + (Index << 9); CopyAdrVals(WAsmCode + 2);
      CodeLen = 4 + AdrCnt;
      DecodeAdr(ArgStr[2], Mdata);
      if (AdrNum != 0)
      {
        WAsmCode[1] |= AdrMode << 12;
        CheckCPU(CPU68020);
      }
      else
        CodeLen = 0;
    }
  }
}

static void DecodeBFINS(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else if (*AttrPart != '\0') WrError(1130);
  else if (!SplitBitField(ArgStr[2], WAsmCode + 1)) WrError(1750);
  else
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(ArgStr[2], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xefc0 + AdrMode;
      CopyAdrVals(WAsmCode + 2);
      CodeLen = 4 + AdrCnt;
      DecodeAdr(ArgStr[1], Mdata);
      if (AdrNum != 0)
      {
        WAsmCode[1] |= AdrMode << 12;
        CheckCPU(CPU68020);
      }
      else
        CodeLen = 0;
    }
  }
}

/* bedingte Befehle */

static void DecodeBcc(Word CondCode)
{
  /* .W, .S, .L, .X erlaubt */

  if ((OpSize > eSymbolSize32Bit) && (OpSize != eSymbolSizeFloat32Bit) && (OpSize != eSymbolSizeFloat96Bit)) WrError(1130);

  /* nur ein Operand erlaubt */

  else if (ChkArgCnt(1, 1))
  {
    LongInt HVal;
    Integer HVal16;
    ShortInt HVal8;
    Boolean ValOK, IsBSR = (1 == CondCode);
    tSymbolFlags Flags;

    /* Zieladresse ermitteln, zum Programmzaehler relativieren */

    HVal = EvalIntExpressionWithFlags(ArgStr[1], Int32, &ValOK, &Flags);
    HVal = HVal - (EProgCounter() + 2);

    /* Bei Automatik Groesse festlegen */

    if (*AttrPart == '\0')
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

        else if ((Flags & NextLabelFlag_AfterBSR) && (HVal == 2) && IsBSR)
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
        if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
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
        if ((!IsDisp8(HVal)) && (!SymbolQuestionable)) WrError(1370);

        /* cannot generate short BSR with zero displacement, and BSR cannot
           be replaced with NOP -> error */

        else if ((HVal == 0) && IsBSR) WrError(1370);

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
            if ((!Repass) && (*AttrPart != '\0'))
              WrError(60);
          }
        }
      }

      /* 32 Bit ? */

      else
      {
        CodeLen = 6;
        WAsmCode[0] = 0x60ff | (CondCode << 8);
        WAsmCode[1] = HVal >> 16;
        WAsmCode[2] = HVal & 0xffff;
        CheckCPU(CPU68332);
      }
    }

    if ((CodeLen > 0) && IsBSR)
      AfterBSRAddr = EProgCounter() + CodeLen;
  }
}

static void DecodeScc(Word CondCode)
{
  if ((*AttrPart != '\0') && (OpSize != eSymbolSize8Bit)) WrError(1130);
  else if (ArgCnt != 1) WrError(1130);
  else
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(ArgStr[1], Mdata | ((MomCPU == CPUCOLD) ? 0 : Madri | Mpost | Mpre | Mdadri | Maix | Mabs));
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
  if (OpSize != eSymbolSize16Bit) WrError(1130);
  else if (ChkArgCnt(2, 2)
        && ChkExcludeCPU(CPUCOLD))
  {
    Boolean ValOK;
    LongInt HVal = EvalIntExpression(ArgStr[2], Int32, &ValOK);
    Integer HVal16;

    if (ValOK)
    {
      HVal -= (EProgCounter() + 2);
      HVal16 = HVal;
      if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
      else
      {
        CodeLen = 4;
        WAsmCode[0] = 0x50c8 | (CondCode << 8);
        WAsmCode[1] = HVal16;
        DecodeAdr(ArgStr[1], Mdata);
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

  if (*AttrPart == '\0')
    OpSize = eSymbolSize8Bit;
  ExpectArgCnt = (OpSize == eSymbolSize8Bit) ? 0 : 1;
  if (OpSize > 2) WrError(1130);
  else if (!ChkArgCnt(ExpectArgCnt, ExpectArgCnt));
  else if ((CondCode != 1) && !ChkExcludeCPU(CPUCOLD));
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
      DecodeAdr(ArgStr[1], Mimm);
      if (AdrNum != 0)
      {
        WAsmCode[0] += OpSize + 1;
        CopyAdrVals(WAsmCode + 1);
        CodeLen = 2 + AdrCnt;
      }
    }
    CheckCPU(CPUCOLD);
  }
}

/*-------------------------------------------------------------------------*/
/* Dekodierroutinen Gleitkommaeinheit */

static Boolean DecodeOneFPReg(char *Asc, Byte * h)
{
  if ((strlen(Asc) == 3) && (!strncasecmp(Asc, "FP", 2)) && ValReg(Asc[2]))
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

  strmaxcpy(Asc, Asc_o, 255);
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
    if (!strcasecmp(s, "FPCR"))
      hw |= 0x400;
    else if (!strcasecmp(s, "FPSR"))
      hw |= 0x200;
    else if (!strcasecmp(s, "FPIAR"))
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
  const char *pArg2 = ArgStr[2];

  if ((ArgCnt == 1) && (!Op->Dya))
  {
    pArg2 = ArgStr[1];
    ArgCnt = 2;
  }
  if (*AttrPart == '\0')
    OpSize = eSymbolSizeFloat96Bit;
  if (OpSize == eSymbolSize64Bit) WrError(1130);
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (ChkArgCnt(2, 2))
  {
    DecodeAdr(pArg2, Mfpn);
    if (AdrNum == 12)
    {
      WAsmCode[0] = 0xf200;
      WAsmCode[1] = Op->Code | (AdrMode << 7);
      RelPos = 4;
      DecodeAdr(ArgStr[1], ((OpSize <= eSymbolSize32Bit) || (OpSize == eSymbolSizeFloat32Bit))
                         ? Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm | Mfpn
                         : Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm | Mfpn);
      if (AdrNum == 12)
      {
        WAsmCode[1] |= AdrMode << 10;
        if (OpSize == eSymbolSizeFloat96Bit)
          CodeLen = 4;
        else
          WrError(1130);
        CheckCPU(Op->MinCPU);
      }
      else if (AdrNum != 0)
      {
        CodeLen = 4 + AdrCnt;
        CopyAdrVals(WAsmCode + 2);
        WAsmCode[0] |= AdrMode;
        WAsmCode[1] |= 0x4000 | (((Word)FSizeCodes[OpSize]) << 10);
        CheckCPU(Op->MinCPU);
      }
    }
  }
}

static void DecodeFSAVE(Word Code)
{
  UNUSED(Code);

  if (!ChkArgCnt(1, 1));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (*AttrPart != '\0') WrError(1130);
  else
  {
    DecodeAdr(ArgStr[1], Madri | Mpre | Mdadri | Maix | Mabs);
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
  else if (*AttrPart != '\0') WrError(1130);
  else
  {
    DecodeAdr(ArgStr[1], Madri | Mpost | Mdadri | Maix | Mabs);
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
  else if (*AttrPart != '\0') WrError(1130);
  else
  {
    CodeLen = 4;
    WAsmCode[0] = 0xf280;
    WAsmCode[1] = 0;
  }
}

static void DecodeFMOVE(Word Code)
{
  char *p;
  String sk;

  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (OpSize == 3) WrError(1130);
  else
  {
    p = strchr(AttrPart, '{');
    if (p)                               /* k-Faktor abspalten */
    {
      strcpy(sk, p);
      *p = '\0';
    }
    else
      *sk = '\0';
    DecodeAdr(ArgStr[2], Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Maix | Mabs | Mfpn | Mfpcr);
    if (AdrNum == 12)                         /* FMOVE.x <ea>/FPm,FPn ? */
    {
      WAsmCode[0] = 0xf200;
      WAsmCode[1] = AdrMode << 7;
      RelPos = 4;
      if (*AttrPart == '\0')
        OpSize = eSymbolSizeFloat96Bit;
      DecodeAdr(ArgStr[1], ((OpSize <= eSymbolSize32Bit) || (OpSize == eSymbolSizeFloat32Bit))
                          ? Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm | Mfpn
                          : Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm | Mfpn);
      if (AdrNum == 12)                       /* FMOVE.X FPm,FPn ? */
      {
        WAsmCode[1] |= AdrMode << 10;
        if (OpSize == eSymbolSizeFloat96Bit)
          CodeLen = 4;
        else
          WrError(1130);
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
      if ((OpSize != eSymbolSize32Bit) && (*AttrPart != '\0')) WrError(1130);
      else
      {
        RelPos = 4;
        WAsmCode[0] = 0xf200;
        WAsmCode[1] = 0x8000 | (AdrMode << 10);
        DecodeAdr(ArgStr[1], (AdrMode == 1)
               ?  Mdata | Madr | Madri | Mpost | Mpre | Mdadri | Mpc | Mpcidx | Mabs | Mimm
               :  Mdata | Madri | Mpost | Mpre | Mdadri | Mpc | Mpcidx | Mabs | Mimm);
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
      DecodeAdr(ArgStr[1], (AdrNum == 2) ? Mfpcr : Mfpn | Mfpcr);
      if (AdrNum == 12)                       /* FMOVE.x FPn,<ea> ? */
      {
        if (*AttrPart == '\0')
          OpSize = eSymbolSizeFloat96Bit;
        WAsmCode[1] = 0x6000 | (((Word)FSizeCodes[OpSize]) << 10) | (AdrMode << 7);
        if (OpSize == eSymbolSizeFloatDec96Bit)
        {
          if (strlen(sk) > 2)
          {
            OpSize = eSymbolSize8Bit;
            strmov(sk, sk + 1);
            sk[strlen(sk) - 1] = '\0';
            DecodeAdr(sk, Mdata | Mimm);
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
        if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit))
        {
          WrError(1130);
          CodeLen = 0;
        }
        else
        {
          WAsmCode[1] = 0xa000 | (AdrMode << 10);
          if ((AdrMode != 1) && ((WAsmCode[0] & 0x38) == 8))
          {
            WrError(1350);
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
  else if ((*AttrPart != '\0') && (OpSize != eSymbolSizeFloat96Bit)) WrError(1130);
  else
  {
    DecodeAdr(ArgStr[2], Mfpn);
    if (AdrNum == 12)
    {
      WAsmCode[0] = 0xf200;
      WAsmCode[1] = 0x5c00 | (AdrMode << 7);
      OpSize = eSymbolSize8Bit;
      DecodeAdr(ArgStr[1], Mimm);
      if (AdrNum == 11)
      {
        if (AdrVals[0] > 63) WrError(1700);
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

  if (*AttrPart == '\0') OpSize = eSymbolSizeFloat96Bit;
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (OpSize == eSymbolSize64Bit) WrError(1130);
  else if (ChkArgCnt(1, 1))
  {
    RelPos = 4;
    DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm | Mfpn);
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
  char *p;
  String sk;

  UNUSED(Code);

  if (*AttrPart == '\0')
    OpSize = eSymbolSizeFloat96Bit;
  if (OpSize == 3) WrError(1130);
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else if (ChkArgCnt(2, 2))
  {
    p = strrchr(ArgStr[2], ':');
    if (p)
    {
      *p = '\0';
      strcpy(sk, ArgStr[2]);
      strmov(ArgStr[2], p + 1);
    }
    else
      *sk = '\0';
    DecodeAdr(sk, Mfpn);
    if (AdrNum == 12)
    {
      WAsmCode[1] = AdrMode | 0x30;
      DecodeAdr(ArgStr[2], Mfpn);
      if (AdrNum == 12)
      {
        WAsmCode[1] |= (AdrMode << 7);
        RelPos = 4;
        DecodeAdr(ArgStr[1], ((OpSize <= eSymbolSize32Bit) || (OpSize == eSymbolSizeFloat32Bit))
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
  else if (ChkMinCPU(CPU68040))
  {
    DecodeAdr(ArgStr[2], Mfpn);
    if (AdrNum == 12)
    {
      unsigned Mask;

      WAsmCode[0] = 0xf200;
      WAsmCode[1] = Code | AdrMode << 7;
      RelPos = 4;
      if (*AttrPart == '\0')
        OpSize = eSymbolSizeFloat96Bit;
      Mask = Mfpn | Madri | Mpost | Mpre | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm;
      if ((OpSize <= eSymbolSize32Bit) || (OpSize == eSymbolSizeFloat32Bit))
        Mask |= Mdata;
      DecodeAdr(ArgStr[1], Mask);
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
  Byte z1, z2;
  Word Mask;

  UNUSED(Code);

  if (!ChkArgCnt(2, 2));
  else if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else
  {
    DecodeFRegList(ArgStr[2], &z1, &z2);
    if (z1 != 0)
    {
      if ((*AttrPart != '\0')
      && (((z1 < 3) && (OpSize != eSymbolSizeFloat96Bit))
        || ((z1 == 3) && (OpSize != eSymbolSize32Bit))))
       WrError(1130);
      else
      {
        RelPos = 4;
        Mask = Madri | Mpost | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm;
        if (z1 == 3)   /* Steuerregister auch Predekrement */
        {
          Mask |= Mpre;
          if ((z2 == 4) | (z2 == 2) | (z2 == 1)) /* nur ein Register */
            Mask |= Mdata;
          if (z2 == 1) /* nur FPIAR */
            Mask |= Madr;
        }
        DecodeAdr(ArgStr[1], Mask);
        WAsmCode[1] = 0;
        GenerateMovem(z1, z2);
      }
    }
    else
    {
      DecodeFRegList(ArgStr[1], &z1, &z2);
      if (z1 != 0)
      {
        if ((*AttrPart != '\0') && (((z1 < 3) && (OpSize != eSymbolSizeFloat96Bit)) || ((z1 == 3) && (OpSize != eSymbolSize32Bit)))) WrError(1130);
        else
        {
          Mask = Madri | Mpre | Mdadri | Maix | Mabs;
          if (z1 == 3)   /* Steuerregister auch Postinkrement */
          {
            Mask |= Mpre;
            if ((z2 == 4) | (z2 == 2) | (z2 == 1)) /* nur ein Register */
              Mask |= Mdata;
            if (z2 == 1) /* nur FPIAR */
              Mask |= Madr;
          }
          DecodeAdr(ArgStr[2], Mask);
          WAsmCode[1] = 0x2000;
          GenerateMovem(z1, z2);
        }
      }
      else
        WrError(1410);
    }
  }
}

static void DecodeFBcc(Word CondCode)
{
  if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  else
  {
    if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit) && (OpSize != eSymbolSizeFloat96Bit)) WrError(1130);
    else if (ChkArgCnt(1, 1))
    {
      LongInt HVal;
      Integer HVal16;
      Boolean ValOK;

      HVal = EvalIntExpression(ArgStr[1], Int32, &ValOK) - (EProgCounter() + 2);
      HVal16 = HVal;

      if (*AttrPart == 0)
      {
        OpSize = (IsDisp16(HVal)) ? eSymbolSize32Bit : eSymbolSizeFloat96Bit;
      }

      if ((OpSize == eSymbolSize32Bit) || (OpSize == eSymbolSize16Bit))
      {
        if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
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
        if ((IsDisp16(HVal)) && (PassNo > 1) && (*AttrPart == '\0'))
        {
          WrError(20);
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
  else
  {
    if ((OpSize != eSymbolSize16Bit) && (*AttrPart != '\0')) WrError(1130);
    else if (ChkArgCnt(2, 2))
    {
      DecodeAdr(ArgStr[1], Mdata);
      if (AdrNum != 0)
      {
        LongInt HVal;
        Integer HVal16;
        Boolean ValOK;

        WAsmCode[0] = 0xf248 | AdrMode;
        WAsmCode[1] = CondCode;
        HVal = EvalIntExpression(ArgStr[2], Int32, &ValOK) - (EProgCounter() + 4);
        if (ValOK)
        {
          HVal16 = HVal;
          WAsmCode[2] = HVal16;
          if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
            else CodeLen = 6;
        }
      }
    }
  }
}

static void DecodeFScc(Word CondCode)
{
  if (!FPUAvail) WrError(ErrNum_FPUNotEnabled);
  if ((OpSize != eSymbolSize8Bit) && (*AttrPart != '\0')) WrError(1130);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
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
  else
  {
    if (*AttrPart == '\0')
      OpSize = eSymbolSize8Bit;
    if (OpSize > eSymbolSize32Bit) WrError(1130);
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
        DecodeAdr(ArgStr[1], Mimm);
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

static Boolean DecodeFC(char *Asc, Word *erg)
{
  Boolean OK;
  Word Val;
  String Asc_N;

  strmaxcpy(Asc_N, Asc, 255);
  NLS_UpString(Asc_N);
  Asc = Asc_N;

  if (!strcmp(Asc, "SFC"))
  {
    *erg = 0;
    return True;
  }

  if (!strcmp(Asc, "DFC"))
  {
    *erg = 1;
    return True;
  }

  if ((strlen(Asc) == 2) && (*Asc == 'D') && ValReg(Asc[1]))
  {
    *erg = Asc[2] - '0' + 8;
    return True;
  }

  if (*Asc == '#')
  {
    Val = EvalIntExpression(Asc + 1, Int4, &OK);
    if (OK)
      *erg = Val + 16;
    return OK;
  }

  return False;
}

static Boolean DecodePMMUReg(char *Asc, Word *erg, tSymbolSize *pSize)
{
  Byte z;

  if ((strlen(Asc) == 4) && (!strncasecmp(Asc, "BAD", 3)) && ValReg(Asc[3]))
  {
    *pSize = eSymbolSize16Bit;
    *erg = 0x7000 + ((Asc[3] - '0') << 2);
    return True;
  }
  if ((strlen(Asc) == 4) && (!strncasecmp(Asc, "BAC", 3)) && ValReg(Asc[3]))
  {
    *pSize = eSymbolSize16Bit;
    *erg = 0x7400 + ((Asc[3] - '0') << 2);
    return True;
  }

  for (z = 0; z < PMMURegCnt; z++)
    if (!strcasecmp(Asc, PMMURegs[z].pName))
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
  else if (*AttrPart != '\0') WrError(1130);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
  else
  {
    DecodeAdr(ArgStr[1], Madri | Mpre | Mdadri | Maix | Mabs);
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
  else if (*AttrPart != '\0') WrError(1130);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
  else
  {
    DecodeAdr(ArgStr[1], Madri | Mpre | Mdadri | Maix | Mabs);
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

  if (*AttrPart != '\0') WrError(1130);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (ChkArgCnt(0, 0))
  {
    if (MomCPU >= CPU68040)
    {
      CodeLen = 2;
      WAsmCode[0] = 0xf518;
    }
    else
    {
      CodeLen = 4;
      WAsmCode[0] = 0xf000;
      WAsmCode[1] = 0x2400;
    }
    CheckSup();
  }
}

static void DecodePFLUSHAN(Word Code)
{
  UNUSED(Code);

  if (*AttrPart != '\0') WrError(1130);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (ChkArgCnt(0, 0))
  {
    CodeLen = 2;
    WAsmCode[0] = 0xf510;
    CheckCPU(CPU68040);
    CheckSup();
  }
}

static void DecodePFLUSH_PFLUSHS(Word Code)
{
  if (*AttrPart != '\0') WrError(1130);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (MomCPU >= CPU68040)
  {
    if (Code) WrError(ErrNum_FullPMMUNotEnabled);
    else if (ChkArgCnt(1, 1))
    {
      DecodeAdr(ArgStr[1], Madri);
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
  else if (!DecodeFC(ArgStr[1], WAsmCode + 1)) WrError(1710);
  else
  {
    OpSize = eSymbolSize8Bit;
    DecodeAdr(ArgStr[2], Mimm);
    if (AdrNum != 0)
    {
      if (AdrVals[0] > 15) WrError(1720);
      else
      {
        WAsmCode[1] |= (AdrVals[0] << 5) | 0x3000 | Code;
        WAsmCode[0] = 0xf000;
        CodeLen = 4;
        CheckSup();
        if (ArgCnt == 3)
        {
          WAsmCode[1] |= 0x800;
          DecodeAdr(ArgStr[3], Madri | Mdadri | Maix | Mabs);
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

  if (*AttrPart != '\0') WrError(1100);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1], Madri);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf500 + (AdrMode & 7);
      CodeLen = 2;
      CheckCPU(CPU68040);
      CheckSup();
    }
  }
}

static void DecodePFLUSHR(Word Code)
{
  UNUSED(Code);

  if (*AttrPart == '\0')
    OpSize = eSymbolSize64Bit;
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  if (OpSize != eSymbolSize64Bit) WrError(1130);
  else if (!ChkArgCnt(1, 1));
  else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
  else
  {
    RelPos = 4;
    DecodeAdr(ArgStr[1], Madri | Mpre | Mpost | Mdadri | Maix | Mpc | Mpcidx | Mabs | Mimm);
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
  if (*AttrPart != '\0') WrError(1130);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (!ChkArgCnt(2, 2));
  else if (!DecodeFC(ArgStr[1], WAsmCode + 1)) WrError(1710);
  else
  {
    DecodeAdr(ArgStr[2], Madri | Mdadri | Maix | Mabs);
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
    if (DecodePMMUReg(ArgStr[1], WAsmCode + 1, &RegSize))
    {
      WAsmCode[1] |= 0x200;
      if (*AttrPart == '\0')
        OpSize = RegSize;
      if (OpSize != RegSize) WrError(1130);
      else
      {
        Mask = Madri | Mdadri | Maix | Mabs;
        if (FullPMMU)
        {
          Mask *= Mpost | Mpre;
          if (RegSize != eSymbolSize64Bit)
            Mask += Mdata | Madr;
        }
        DecodeAdr(ArgStr[2], Mask);
        if (AdrNum != 0)
        {
          WAsmCode[0] = 0xf000 | AdrMode;
          CodeLen = 4 + AdrCnt;
          CopyAdrVals(WAsmCode + 2);
          CheckSup();
        }
      }
    }
    else if (DecodePMMUReg(ArgStr[2], WAsmCode + 1, &RegSize))
    {
      if (*AttrPart == '\0')
        OpSize = RegSize;
      if (OpSize != RegSize) WrError(1130);
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
        DecodeAdr(ArgStr[1], Mask);
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
      WrError(1730);
  }
}

static void DecodePTESTR_PTESTW(Word Code)
{
  if (*AttrPart != '\0') WrError(1130);
  else if (!PMMUAvail) WrError(ErrNum_PMMUNotEnabled);
  else if (MomCPU >= CPU68040)
  {
    if (ChkArgCnt(1, 1))
    {
      DecodeAdr(ArgStr[1], Madri);
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
    if (!DecodeFC(ArgStr[1], WAsmCode + 1)) WrError(1710);
    else
    {
      DecodeAdr(ArgStr[2], Madri | Mdadri | Maix | Mabs);
      if (AdrNum != 0)
      {
        WAsmCode[0] = 0xf000 | AdrMode;
        CodeLen = 4 + AdrCnt;
        WAsmCode[1] |= 0x8000 | (Code << 9);
        CopyAdrVals(WAsmCode + 2);
        DecodeAdr(ArgStr[3], Mimm);
        if (AdrNum != 0)
        {
          if (AdrVals[0] > 7)
          {
            WrError(1740);
            CodeLen = 0;
          }
          else
          {
            WAsmCode[1] |= AdrVals[0] << 10;
            if (ArgCnt == 4)
            {
              DecodeAdr(ArgStr[4], Madr);
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
  else if ((*AttrPart != '\0') && (OpSize != eSymbolSize32Bit)) WrError(1130);
  else
  {
    DecodeAdr(ArgStr[2], Madri | Mdadri | Maix | Mabs);
    if (AdrNum != 0)
    {
      WAsmCode[0] = 0xf000 | AdrMode;
      WAsmCode[1] = 0x2800;
      CodeLen = 4 + AdrCnt;
      CopyAdrVals(WAsmCode + 1);
      if (!strcasecmp(ArgStr[1], "VAL"));
      else
      {
        DecodeAdr(ArgStr[1], Madr);
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
    if ((OpSize != eSymbolSize16Bit) && (OpSize != eSymbolSize32Bit) && (OpSize != eSymbolSizeFloat96Bit)) WrError(1130);
    else if (!ChkArgCnt(1, 1));
    else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
    else
    {
      LongInt HVal;
      Integer HVal16;
      Boolean ValOK;

      HVal = EvalIntExpression(ArgStr[1], Int32, &ValOK) - (EProgCounter() + 2);
      HVal16 = HVal;

      if (*AttrPart == 0)
        OpSize = (IsDisp16(HVal)) ? eSymbolSize32Bit : eSymbolSizeFloat96Bit;

      if ((OpSize == eSymbolSize32Bit) || (OpSize == eSymbolSize16Bit))
      {
        if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
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
    if ((OpSize != eSymbolSize16Bit) && (*AttrPart != '\0')) WrError(1130);
    else if (!ChkArgCnt(2, 2));
    else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
    else
    {
      DecodeAdr(ArgStr[1], Mdata);
      if (AdrNum != 0)
      {
        LongInt HVal;
        Integer HVal16;
        Boolean ValOK;

        WAsmCode[0] = 0xf048 | AdrMode;
        WAsmCode[1] = CondCode;
        HVal = EvalIntExpression(ArgStr[2], Int32, &ValOK) - (EProgCounter() + 4);
        if (ValOK)
        {
          HVal16 = HVal;
          WAsmCode[2] = HVal16;
          if ((!IsDisp16(HVal)) && (!SymbolQuestionable)) WrError(1370);
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
    if ((OpSize != eSymbolSize8Bit) && (*AttrPart != '\0')) WrError(1130);
    else if (!ChkArgCnt(1, 1));
    else if (!FullPMMU) WrError(ErrNum_FullPMMUNotEnabled);
    else
    {
      DecodeAdr(ArgStr[1], Mdata | Madri | Mpost | Mpre | Mdadri | Maix | Mabs);
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
    if (*AttrPart == '\0')
      OpSize = eSymbolSize8Bit;
    if (OpSize > 2) WrError(1130);
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
        DecodeAdr(ArgStr[1], Mimm);
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
  else if ((l = strlen(ArgStr[1])) < 2) WrError(1135);
  else if (*ArgStr[1] != '\'') WrError(1135);
  else if (ArgStr[1][l - 1] != '\'') WrError(1135);
  else
  {
    PutByte(l - 2);
    for (z = 1; z < l - 1; z++)
      PutByte(CharTransTable[((usint) ArgStr[1][z]) & 0xff]);
    if ((Odd(CodeLen)) && (DoPadding))
      PutByte(0);
  }
}

/*-------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static void AddFixed(char *NName, Word NCode, Boolean NSup, Word NMask)
{
  if (InstrZ >= FixedOrderCnt) exit(255);
  FixedOrders[InstrZ].Code = NCode;
  FixedOrders[InstrZ].MustSup = NSup;
  FixedOrders[InstrZ].CPUMask = NMask;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddCtReg(char *NName, Word NCode, CPUVar NFirst, CPUVar NLast)
{
  if (InstrZ >= CtRegCnt) exit(255);
  CtRegs[InstrZ].Name = NName;
  CtRegs[InstrZ].Code = NCode;
  CtRegs[InstrZ].FirstCPU = NFirst;
  CtRegs[InstrZ++].LastCPU = NLast;
}

static void AddCond(char *NName, Byte NCode)
{
  char TmpName[30];

  if (NCode >= 2) /* BT is BRA and BF is BSR */
  {
    sprintf(TmpName, "B%s", NName);
    AddInstTable(InstTable, TmpName, NCode, DecodeBcc);
  }
  sprintf(TmpName, "S%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeScc);
  sprintf(TmpName, "DB%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeDBcc);
  sprintf(TmpName, "TRAP%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeTRAPcc);
}

static void AddFPUOp(char *NName, Byte NCode, Boolean NDya, CPUVar NMin)
{
  if (InstrZ >= FPUOpCnt) exit(255);
  FPUOps[InstrZ].Code = NCode;
  FPUOps[InstrZ].Dya = NDya;
  FPUOps[InstrZ].MinCPU = NMin;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFPUOp);
}

static void AddFPUCond(char *NName, Byte NCode)
{
  char TmpName[30];

  sprintf(TmpName, "FB%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeFBcc);
  sprintf(TmpName, "FDB%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeFDBcc);
  sprintf(TmpName, "FS%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeFScc);
  sprintf(TmpName, "FTRAP%s", NName);
  AddInstTable(InstTable, TmpName, NCode, DecodeFTRAPcc);
}

static void AddPMMUCond(char *NName)
{
  char TmpName[30];

  sprintf(TmpName, "PB%s", NName);
  AddInstTable(InstTable, TmpName, InstrZ, DecodePBcc);
  sprintf(TmpName, "PDB%s", NName);
  AddInstTable(InstTable, TmpName, InstrZ, DecodePDBcc);
  sprintf(TmpName, "PS%s", NName);
  AddInstTable(InstTable, TmpName, InstrZ, DecodePScc);
  sprintf(TmpName, "PTRAP%s", NName);
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

  AddInstTable(InstTable, "MOVE"   , 0, DecodeMOVE);
  AddInstTable(InstTable, "MOVEA"  , 1, DecodeMOVE);
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
  AddInstTable(InstTable, "SUB"    , 0, DecodeADDSUBCMP);
  AddInstTable(InstTable, "CMP"    , 1, DecodeADDSUBCMP);
  AddInstTable(InstTable, "ADD"    , 2, DecodeADDSUBCMP);
  AddInstTable(InstTable, "SUBI"   , 4, DecodeADDSUBCMP);
  AddInstTable(InstTable, "CMPI"   , 5, DecodeADDSUBCMP);
  AddInstTable(InstTable, "ADDI"   , 6, DecodeADDSUBCMP);
  AddInstTable(InstTable, "SUBA"   , 8, DecodeADDSUBCMP);
  AddInstTable(InstTable, "CMPA"   , 9, DecodeADDSUBCMP);
  AddInstTable(InstTable, "ADDA"   , 10, DecodeADDSUBCMP);
  AddInstTable(InstTable, "AND"    , 1, DecodeANDOR);
  AddInstTable(InstTable, "OR"     , 0, DecodeANDOR);
  AddInstTable(InstTable, "ANDI"   , 5, DecodeANDOR);
  AddInstTable(InstTable, "ORI"    , 4, DecodeANDOR);
  AddInstTable(InstTable, "EOR"    , 0, DecodeEOR);
  AddInstTable(InstTable, "EORI"   , 4, DecodeEOR);
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
  AddFixed("NOP"    , 0x4e71, False, 0x07ff);
  AddFixed("RESET"  , 0x4e70, False, 0x07ef);
  AddFixed("ILLEGAL", 0x4afc, False, 0x07ff);
  AddFixed("TRAPV"  , 0x4e76, False, 0x07ef);
  AddFixed("RTE"    , 0x4e73, True , 0x07ff);
  AddFixed("RTR"    , 0x4e77, False, 0x07ef);
  AddFixed("RTS"    , 0x4e75, False, 0x07ff);
  AddFixed("BGND"   , 0x4afa, False, 0x00e0);
  AddFixed("HALT"   , 0x4ac8, True , 0x0010);
  AddFixed("PULSE"  , 0x4acc, True , 0x0010);

  CtRegs = (CtReg *) malloc(sizeof(CtReg) * CtRegCnt); InstrZ = 0;
  AddCtReg("SFC"  , 0x000, CPU68010, CPU68040);
  AddCtReg("DFC"  , 0x001, CPU68010, CPU68040);
  AddCtReg("CACR" , 0x002, CPU68020, CPU68040);
  AddCtReg("TC"   , 0x003, CPU68040, CPU68040);
  AddCtReg("ITT0" , 0x004, CPU68040, CPU68040);
  AddCtReg("ITT1" , 0x005, CPU68040, CPU68040);
  AddCtReg("DTT0" , 0x006, CPU68040, CPU68040);
  AddCtReg("DTT1" , 0x007, CPU68040, CPU68040);
  AddCtReg("USP"  , 0x800, CPU68010, CPU68040);
  AddCtReg("VBR"  , 0x801, CPU68010, CPU68040);
  AddCtReg("CAAR" , 0x802, CPU68020, CPU68030);
  AddCtReg("MSP"  , 0x803, CPU68020, CPU68040);
  AddCtReg("ISP"  , 0x804, CPU68020, CPU68040);
  AddCtReg("MMUSR", 0x805, CPU68040, CPU68040);
  AddCtReg("URP"  , 0x806,  CPU68040, CPU68040);
  AddCtReg("SRP"  , 0x807, CPU68040, CPU68040);
  AddCtReg("IACR0", 0x004, CPU68040, CPU68040);
  AddCtReg("IACR1", 0x005, CPU68040, CPU68040);
  AddCtReg("DACR0", 0x006, CPU68040, CPU68040);
  AddCtReg("DACR1", 0x007, CPU68040, CPU68040);
  AddCtReg("TCR"  , 0x003, CPUCOLD , CPUCOLD );
  AddCtReg("ACR2" , 0x004, CPUCOLD , CPUCOLD );
  AddCtReg("ACR3" , 0x005, CPUCOLD , CPUCOLD );
  AddCtReg("ACR0" , 0x006, CPUCOLD , CPUCOLD );
  AddCtReg("ACR1" , 0x007,  CPUCOLD , CPUCOLD );
  AddCtReg("ROMBAR", 0xc00, CPUCOLD , CPUCOLD );
  AddCtReg("RAMBAR0", 0xc04, CPUCOLD, CPUCOLD );
  AddCtReg("RAMBAR1", 0xc05, CPUCOLD, CPUCOLD );
  AddCtReg("MBAR" , 0xc0f, CPUCOLD , CPUCOLD );

  AddCond("T" , 0);  AddCond("F" , 1);  AddCond("HI", 2);  AddCond("LS", 3);
  AddCond("CC", 4);  AddCond("CS", 5);  AddCond("NE", 6);  AddCond("EQ", 7);
  AddCond("VC", 8);  AddCond("VS", 9);  AddCond("PL",10);  AddCond("MI",11);
  AddCond("GE",12);  AddCond("LT",13);  AddCond("GT",14);  AddCond("LE",15);
  AddCond("HS", 4);  AddCond("LO", 5);
  AddInstTable(InstTable, "BRA", 0, DecodeBcc);
  AddInstTable(InstTable, "BSR", 1, DecodeBcc);
  AddInstTable(InstTable, "DBRA", 1, DecodeDBcc);

  FPUOps = (FPUOp *) malloc(sizeof(FPUOp) * FPUOpCnt); InstrZ = 0;
  AddFPUOp("FINT"   , 0x01, False, CPU68000);  AddFPUOp("FSINH"  , 0x02, False, CPU68000);
  AddFPUOp("FINTRZ" , 0x03, False, CPU68000);  AddFPUOp("FSQRT"  , 0x04, False, CPU68000);
  AddFPUOp("FLOGNP1", 0x06, False, CPU68000);  AddFPUOp("FETOXM1", 0x08, False, CPU68000);
  AddFPUOp("FTANH"  , 0x09, False, CPU68000);  AddFPUOp("FATAN"  , 0x0a, False, CPU68000);
  AddFPUOp("FASIN"  , 0x0c, False, CPU68000);  AddFPUOp("FATANH" , 0x0d, False, CPU68000);
  AddFPUOp("FSIN"   , 0x0e, False, CPU68000);  AddFPUOp("FTAN"   , 0x0f, False, CPU68000);
  AddFPUOp("FETOX"  , 0x10, False, CPU68000);  AddFPUOp("FTWOTOX", 0x11, False, CPU68000);
  AddFPUOp("FTENTOX", 0x12, False, CPU68000);  AddFPUOp("FLOGN"  , 0x14, False, CPU68000);
  AddFPUOp("FLOG10" , 0x15, False, CPU68000);  AddFPUOp("FLOG2"  , 0x16, False, CPU68000);
  AddFPUOp("FABS"   , 0x18, False, CPU68000);  AddFPUOp("FCOSH"  , 0x19, False, CPU68000);
  AddFPUOp("FNEG"   , 0x1a, False, CPU68000);  AddFPUOp("FACOS"  , 0x1c, False, CPU68000);
  AddFPUOp("FCOS"   , 0x1d, False, CPU68000);  AddFPUOp("FGETEXP", 0x1e, False, CPU68000);
  AddFPUOp("FGETMAN", 0x1f, False, CPU68000);  AddFPUOp("FDIV"   , 0x20, True , CPU68000);
  AddFPUOp("FSDIV"  , 0x60, False, CPU68040);  AddFPUOp("FDDIV"  , 0x64, True , CPU68040);
  AddFPUOp("FMOD"   , 0x21, True , CPU68000);  AddFPUOp("FADD"   , 0x22, True , CPU68000);
  AddFPUOp("FSADD"  , 0x62, True , CPU68040);  AddFPUOp("FDADD"  , 0x66, True , CPU68040);
  AddFPUOp("FMUL"   , 0x23, True , CPU68000);  AddFPUOp("FSMUL"  , 0x63, True , CPU68040);
  AddFPUOp("FDMUL"  , 0x67, True , CPU68040);  AddFPUOp("FSGLDIV", 0x24, True , CPU68000);
  AddFPUOp("FREM"   , 0x25, True , CPU68000);  AddFPUOp("FSCALE" , 0x26, True , CPU68000);
  AddFPUOp("FSGLMUL", 0x27, True , CPU68000);  AddFPUOp("FSUB"   , 0x28, True , CPU68000);
  AddFPUOp("FSSUB"  , 0x68, True , CPU68040);  AddFPUOp("FDSUB"  , 0x6c, True , CPU68040);
  AddFPUOp("FCMP"   , 0x38, True , CPU68000);

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
  free(CtRegs);
  free(FPUOps);
  free(PMMURegs);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_68K(void)
{
  CodeLen = 0;
  OpSize = (MomCPU == CPUCOLD) ? eSymbolSize32Bit : eSymbolSize16Bit;
  DontPrint = False; RelPos = 2;

  if (!DecodeMoto16AttrSize(*AttrPart, &OpSize, False))
    return;

  /* Nullanweisung */

  if ((*OpPart.Str == '\0') && (*AttrPart == '\0') && (ArgCnt == 0))
    return;

  /* Pseudoanweisungen */

  if (DecodeMoto16Pseudo(OpSize, True))
    return;

  /* Befehlszaehler ungerade ? */

  if (Odd(EProgCounter())) WrError(180);

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
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

static void SwitchTo_68K(void)
{
  TurnWords = True;
  ConstMode = ConstModeMoto;
  SetIsOccupied = False;

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
  SegLimits[SegCode] = INTCONST_ffffffff;

  MakeCode = MakeCode_68K;
  IsDef = IsDef_68K;

  SwitchFrom = SwitchFrom_68K;
  InitFields();
  AddONOFF("PMMU"    , &PMMUAvail , PMMUAvailName , False);
  AddONOFF("FULLPMMU", &FullPMMU  , FullPMMUName  , False);
  AddONOFF("FPU"     , &FPUAvail  , FPUAvailName  , False);
  AddONOFF("SUPMODE" , &SupAllowed, SupAllowedName, False);
  AddMoto16PseudoONOFF();

  SetFlag(&FullPMMU, FullPMMUName, MomCPU <= CPU68020);
  SetFlag(&DoPadding, DoPaddingName, True);
}

void code68k_init(void)
{
  CPU68008 = AddCPU("68008",   SwitchTo_68K);
  CPU68000 = AddCPU("68000",   SwitchTo_68K);
  CPU68010 = AddCPU("68010",   SwitchTo_68K);
  CPU68012 = AddCPU("68012",   SwitchTo_68K);
  CPUCOLD  = AddCPU("MCF5200", SwitchTo_68K);
  CPU68332 = AddCPU("68332",   SwitchTo_68K);
  CPU68340 = AddCPU("68340",   SwitchTo_68K);
  CPU68360 = AddCPU("68360",   SwitchTo_68K);
  CPU68020 = AddCPU("68020",   SwitchTo_68K);
  CPU68030 = AddCPU("68030",   SwitchTo_68K);
  CPU68040 = AddCPU("68040",   SwitchTo_68K);

  AddInitPassProc(InitCode_68K);
}
