/* codez80.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Zilog Z80/180/380                                           */
/*                                                                           */
/* Historie: 26. 8.1996 Grundsteinlegung                                     */
/*            1. 2.1998 ChkPC ersetzt                                        */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           2001-12-11 begun with Rabbit2000                                */
/*                                                                           */
/*****************************************************************************/
/* $Id: codez80.c,v 1.15 2017/06/04 19:00:44 alfred Exp $                     */
/*****************************************************************************
 * $Log: codez80.c,v $
 * Revision 1.15  2017/06/04 19:00:44  alfred
 * - add some aliases for Z80UNDOC
 *
 * Revision 1.14  2014/12/07 19:14:02  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.13  2014/12/05 11:58:16  alfred
 * - collapse STDC queries into one file
 *
 * Revision 1.12  2014/12/05 08:53:45  alfred
 * - eliminate remaining BEGIN/END
 *
 * Revision 1.11  2014/11/05 15:47:16  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.10  2014/06/20 17:39:22  alfred
 * - correctly report invalid conditions
 *
 * Revision 1.9  2014/06/19 10:12:17  alfred
 * - RRA is not equal to RR A regarding flags, so not warn about short addressing
 *
 * Revision 1.8  2014/06/15 09:17:49  alfred
 * - eliminate some Memo calls
 *
 * Revision 1.7  2014/06/02 21:10:43  alfred
 * - rework to current style
 *
 * Revision 1.6  2007/11/24 22:48:08  alfred
 * - some NetBSD changes
 *
 * Revision 1.5  2007/06/28 20:27:31  alfred
 * - silence some warnings on recent GNU C versions
 *
 * Revision 1.4  2006/08/05 12:01:39  alfred
 * - correct parsing of indexed expressions
 *
 * Revision 1.3  2005/09/08 16:53:43  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2004/05/29 11:33:04  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:24  alfred
 * - recreated
 *
 * Revision 1.3  2003/05/02 21:23:12  alfred
 * - strlen() updates
 *
 * Revision 1.2  2002/10/20 09:22:25  alfred
 * - work around the parser problem related to the ' character
 *
 * Revision 1.7  2002/10/07 20:25:01  alfred
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmcode.h"
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h" 
#include "intpseudo.h"
#include "codevars.h"
#include "intconsts.h"
#include "errmsg.h"

#include "codez80.h"

/*-------------------------------------------------------------------------*/
/* Instruktionsgruppendefinitionen */

typedef struct
{
  CPUVar MinCPU;
  Byte Len;
  Word Code;
} BaseOrder;

typedef struct
{
  char *Name;
  Byte Code;
} Condition; 

/*-------------------------------------------------------------------------*/
/* Praefixtyp */

typedef enum
{
  Pref_IN_N,Pref_IN_W ,Pref_IB_W ,Pref_IW_W ,Pref_IB_N ,
  Pref_IN_LW,Pref_IB_LW,Pref_IW_LW,Pref_IW_N
} PrefType;


#define ExtFlagName    "INEXTMODE"       /* Flag-Symbolnamen */
#define LWordFlagName  "INLWORDMODE"

#define ModNone (-1)
#define ModReg8 1
#define ModReg16 2
#define ModIndReg16 3
#define ModImm 4
#define ModAbs 5
#define ModRef 6
#define ModInt 7
#define ModSPRel 8

#define FixedOrderCnt 53
#define AccOrderCnt 3
#define HLOrderCnt 3
#define ALUOrderCnt 5
#define ConditionCnt 12

#define IXPrefix 0xdd
#define IYPrefix 0xfd

#define U32Mask INTCONST_ffffffff
#define U16Mask INTCONST_ffff0000
#define U8Mask  INTCONST_ff000000

/*-------------------------------------------------------------------------*/

static Byte PrefixCnt;
static Byte AdrPart,OpSize;
static Byte AdrVals[4];
static ShortInt AdrMode;

static BaseOrder *FixedOrders;
static BaseOrder *AccOrders;
static BaseOrder *HLOrders;
static Condition *Conditions;

static CPUVar CPUZ80, CPUZ80U, CPUZ180, CPUR2000, CPUZ380;

static Boolean MayLW,             /* Instruktion erlaubt 32 Bit */
               ExtFlag,           /* Prozessor im 4GByte-Modus ? */
               LWordFlag;         /* 32-Bit-Verarbeitung ? */

static PrefType CurrPrefix,       /* mom. explizit erzeugter Praefix */
                LastPrefix;       /* von der letzten Anweisung generierter Praefix */


/*==========================================================================*/
/* aux functions */

/*--------------------------------------------------------------------------*/
/* Praefix dazuaddieren */

static Boolean ExtendPrefix(PrefType *Dest, char *AddArg)
{
  Byte SPart,IPart;

  switch (*Dest)
  {
    case Pref_IB_N:
    case Pref_IB_W:
    case Pref_IB_LW:
      IPart = 1;
      break;
    case Pref_IW_N:
    case Pref_IW_W:
    case Pref_IW_LW:
      IPart = 2;
      break;
    default:
      IPart = 0;
  }

  switch (*Dest)
  {
    case Pref_IN_W:
    case Pref_IB_W:
    case Pref_IW_W:
      SPart = 1;
      break;
    case Pref_IN_LW:
    case Pref_IB_LW: 
    case Pref_IW_LW:
      SPart = 2;
      break;
    default:
      SPart = 0;
  }

  if (!strcmp(AddArg, "W"))         /* Wortverarbeitung */
    SPart = 1;
  else if (!strcmp(AddArg, "LW"))   /* Langwortverarbeitung */
    SPart = 2;
  else if (!strcmp(AddArg, "IB"))   /* ein Byte im Argument mehr */
    IPart = 1;
  else if (!strcmp(AddArg, "IW"))   /* ein Wort im Argument mehr */
    IPart = 2;
  else
    return False;

  switch ((IPart << 4) | SPart)
  {
    case 0x00:
      *Dest = Pref_IN_N;
      break;
    case 0x01:
      *Dest = Pref_IN_W;
      break;
    case 0x02:
      *Dest = Pref_IN_LW;
      break;
    case 0x10:
      *Dest = Pref_IB_N;
      break;
    case 0x11:
      *Dest = Pref_IB_W;
      break;
    case 0x12:
      *Dest = Pref_IB_LW;
      break;
    case 0x20:
      *Dest = Pref_IW_N;
      break;
    case 0x21:
      *Dest = Pref_IW_W;
      break;
    case 0x22:
      *Dest = Pref_IW_LW;
      break;
  }

  return True;
}

/*--------------------------------------------------------------------------*/
/* Code fuer Praefix bilden */

static void GetPrefixCode(PrefType inp, Byte *b1 ,Byte *b2)
{
  int z;

  z = ((int)inp) - 1;
  *b1 = 0xdd + ((z & 4) << 3);
  *b2 = 0xc0 + (z & 3);
}

/*--------------------------------------------------------------------------*/
/* DD-Praefix addieren, nur EINMAL pro Instruktion benutzen! */

static void ChangeDDPrefix(char *Add)
{
  PrefType ActPrefix;
  int z;

  ActPrefix = LastPrefix;
  if (ExtendPrefix(&ActPrefix, Add))
    if (LastPrefix != ActPrefix)
    {
      if (LastPrefix != Pref_IN_N) RetractWords(2);
      for (z = PrefixCnt - 1; z >= 0; z--) BAsmCode[2 + z] = BAsmCode[z];
      PrefixCnt += 2;
      GetPrefixCode(ActPrefix, BAsmCode + 0, BAsmCode + 1);
    }
}

/*--------------------------------------------------------------------------*/
/* IX/IY used ? */

static Boolean IndexPrefix(void)
{
  return (BAsmCode[PrefixCnt - 1] == IXPrefix)
      || (BAsmCode[PrefixCnt - 1] == IYPrefix);
}

/*--------------------------------------------------------------------------*/
/* Wortgroesse ? */

static Boolean InLongMode(void)
{
  switch (LastPrefix)
  {
    case Pref_IN_W:
    case Pref_IB_W:
    case Pref_IW_W:
      return False;
    case Pref_IN_LW:
    case Pref_IB_LW:
    case Pref_IW_LW:
      return MayLW;
    default:
      return LWordFlag && MayLW;
  }
}

/*--------------------------------------------------------------------------*/
/* absolute Adresse */

static LongWord EvalAbsAdrExpression(char *inp, Boolean *OK)
{
  return EvalIntExpression(inp, ExtFlag ? Int32 : UInt16, OK);
}

/*==========================================================================*/
/* Adressparser */

static Boolean DecodeReg8(char *Asc, Byte *Erg)
{
#define Reg8Cnt 7
  static char *Reg8Names[Reg8Cnt] = { "B", "C", "D", "E", "H", "L", "A" };
  int z;

  for (z = 0; z < Reg8Cnt; z++)
    if (!strcasecmp(Asc, Reg8Names[z]))
    {
      *Erg = z;
      if (z == 6)
        (*Erg)++;
      return True;
    }

  return False;
}

static Boolean IsSym(char ch)
{
  return ((ch == '_')
       || ((ch >= '0') && (ch <= '9'))
       || ((ch >= 'A') && (ch <= 'Z'))
       || ((ch >= 'a') && (ch <= 'z')));
}

static void DecodeAdr(char *Asc_O)
{
#define Reg16Cnt 6
  static char *Reg16Names[Reg16Cnt]={"BC","DE","HL","SP","IX","IY"};

  int z, l;
  Integer AdrInt;
  LongInt AdrLong;
  Boolean OK;

  AdrMode = ModNone;
  AdrCnt = 0;
  AdrPart = 0;

  /* 0. Sonderregister */

  if (!strcasecmp(Asc_O, "R"))
  {
    AdrMode = ModRef;
    return;
  }

  if (!strcasecmp(Asc_O, "I"))
  {
    AdrMode = ModInt;
    return;
  }

  /* 1. 8-Bit-Register ? */

  if (DecodeReg8(Asc_O, &AdrPart))
  {
    AdrMode = ModReg8;
    return;
  }

  /* 1a. 8-Bit-Haelften von IX/IY ? (nur Z380/Z80UNDOC, sonst als Symbole zulassen) */

  if (((MomCPU >= CPUZ380) || (MomCPU == CPUZ80U))
   && ((strlen(Asc_O) == 3) && (toupper(Asc_O[0]) == 'I')))
  {
    char ix = toupper(Asc_O[1]);

    if ((ix == 'X') || (ix == 'Y'))
      switch (toupper(Asc_O[2]))
      {
        case 'L':
          AdrMode = ModReg8;
          BAsmCode[PrefixCnt++] = (ix == 'X') ? IXPrefix : IYPrefix;
          AdrPart = 5;
          return;
        case 'H':
          if (MomCPU != CPUZ80U)
            break; /* conditional break, do not allow IXH/IYH on Z380 */
        case 'U':
          AdrMode = ModReg8;
          BAsmCode[PrefixCnt++] = (ix == 'X') ? IXPrefix : IYPrefix;
          AdrPart = 4;
          return;
      }
  }

  /* 2. 16-Bit-Register ? */

  for (z = 0; z < Reg16Cnt; z++)
    if (!strcasecmp(Asc_O, Reg16Names[z]))
    {
      AdrMode = ModReg16;
      if (z <= 3)
        AdrPart = z;
      else
      {
        BAsmCode[PrefixCnt++] = (z == 4) ? IXPrefix : IYPrefix;
        AdrPart = 2; /* = HL */
      }
      return;
    }

  /* 3. 16-Bit-Register indirekt ? */

  if ((strlen(Asc_O) >= 4) && (*Asc_O == '(') && (Asc_O[l = strlen(Asc_O) - 1] == ')'))
  {
    for (z = 0; z < Reg16Cnt; z++)
      if ((!strncasecmp(Asc_O+1,Reg16Names[z],2)) && (!IsSym(Asc_O[3])))
      {
        if (z < 3)
        {
          if (strlen(Asc_O) != 4)
          {
            WrError(1350);
            return;
          }
          switch (z)
          {
            case 0:
            case 1:   /* BC,DE */
              AdrMode = ModIndReg16;
              AdrPart = z;
              break;
            case 2:   /* HL=M-Register */
              AdrMode = ModReg8;
              AdrPart = 6;
              break;
          }
        }
        else
        {        /* SP,IX,IY */
          /* we replace the register name with '0 ', making it a valid
             expression for the parser.  Removing the outer parentheses saves 
             a level of recursion in the parser: */

          Asc_O[1] = '0';
          Asc_O[2] = ' ';
          Asc_O[l] = '\0';
          AdrLong = EvalIntExpression(Asc_O + 1, (MomCPU >= CPUZ380) ? SInt24 : SInt8, &OK);
          if (OK)
          {
            if (z == 3)
              AdrMode = ModSPRel;
            else
            {
              AdrMode = ModReg8;
              AdrPart=6;
              BAsmCode[PrefixCnt++] = (z == 4) ? IXPrefix : IYPrefix;
            }
            AdrVals[AdrCnt++] = AdrLong & 0xff;
            if ((AdrLong >= -0x80l) && (AdrLong <= 0x7fl));
            else
            {
              AdrVals[AdrCnt++] = (AdrLong >> 8) & 0xff;
              if ((AdrLong >= -0x8000l) && (AdrLong <= 0x7fffl))
                ChangeDDPrefix("IB");
              else
              {
                AdrVals[AdrCnt++] = (AdrLong >> 16) & 0xff;
                ChangeDDPrefix("IW");
              }
            }
          }
        }
        return;
      }
  }

  /* absolut ? */

  if (IsIndirect(Asc_O))
  {
    AdrLong = EvalAbsAdrExpression(Asc_O, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      AdrMode = ModAbs;
      AdrVals[0] = AdrLong & 0xff;
      AdrVals[1] = (AdrLong >> 8) & 0xff;
      AdrCnt = 2;
      if ((AdrLong & U16Mask) == 0);
      else
      {
        AdrVals[AdrCnt++] = ((AdrLong >> 16) & 0xff);
        if ((AdrLong & U8Mask) == 0)
          ChangeDDPrefix("IB");
        else
        {
          AdrVals[AdrCnt++] = ((AdrLong >> 24) & 0xff);
          ChangeDDPrefix("IW");
        }
      }
    }
    return;
  }

  /* ...immediate */

  switch (OpSize)
  {
    case 0xff:
      WrError(1132);
      break;
    case 0:
      AdrVals[0] = EvalIntExpression(Asc_O, Int8, &OK);
      if (OK)
      {
        AdrMode = ModImm;
        AdrCnt = 1;
      }
      break;
    case 1:
      if (InLongMode())
      {
        AdrLong = EvalIntExpression(Asc_O, Int32, &OK);
        if (OK)
        {
          AdrVals[0] = Lo(AdrLong);
          AdrVals[1] = Hi(AdrLong);
          AdrMode = ModImm;
          AdrCnt = 2;
          if ((AdrLong & U16Mask) == 0);
          else
          {
            AdrVals[AdrCnt++] = (AdrLong >> 16) & 0xff;
            if ((AdrLong & U8Mask) == 0)
              ChangeDDPrefix("IB");
            else
            {
              AdrVals[AdrCnt++] = (AdrLong >> 24) & 0xff;
              ChangeDDPrefix("IW");
            }
          }
        }
      }
     else
     {
       AdrInt = EvalIntExpression(Asc_O, Int16, &OK);
       if (OK)
       {
         AdrVals[0] = Lo(AdrInt);
         AdrVals[1] = Hi(AdrInt);
         AdrMode = ModImm;
         AdrCnt=2;
       }
     }
     break;
  }
}

static Boolean ImmIs8(void)
{
  Word tmp;

  if (AdrCnt < 2)
    return True;

  tmp = (Word) AdrVals[AdrCnt - 2];

  return ((tmp <= 255) || (tmp >= 0xff80));
}

static void AppendVals(const Byte *pVals, unsigned ValLen)
{
  memcpy(BAsmCode + CodeLen, pVals, ValLen);
  CodeLen += ValLen;
}

static void AppendAdrVals(void)
{
  AppendVals(AdrVals, AdrCnt);
}

static Boolean ParPair(char *Name1, char *Name2)
{
  return (((!strcasecmp(ArgStr[1], Name1)) && (!strcasecmp(ArgStr[2], Name2))) ||
          ((!strcasecmp(ArgStr[1], Name2)) && (!strcasecmp(ArgStr[2], Name1))));
}

/*-------------------------------------------------------------------------*/
/* Bedingung entschluesseln */

static Boolean DecodeCondition(const char *Name, int *Erg)
{
  int z;

  z = 0;
  while ((z < ConditionCnt) && (strcasecmp(Conditions[z].Name, Name))) z++;
  if (z >= ConditionCnt)
  {
    *Erg = 0;
    return False;
  }
  else
  {
    *Erg = Conditions[z].Code;
    return True;
  }
}

/*-------------------------------------------------------------------------*/
/* Sonderregister dekodieren */

static Boolean DecodeSFR(char *Inp, Byte *Erg)
{
  if (!strcasecmp(Inp, "SR"))
    *Erg = 1;
  else if (!strcasecmp(Inp, "XSR"))
    *Erg = 5;
  else if (!strcasecmp(Inp, "DSR"))
    *Erg = 6;
  else if (!strcasecmp(Inp, "YSR"))
    *Erg = 7;
  else
    return False;
  return True;
}

/*==========================================================================*/
/* Adressbereiche */

static LargeWord CodeEnd(void)
{
  if (ExtFlag) return U32Mask;
  else if (MomCPU == CPUZ180) return 0x7ffffl;
  else return 0xffff;
}

static LargeWord PortEnd(void)
{
  if (ExtFlag) return U32Mask;
  else return 0xff;
}

/*==========================================================================*/
/* instruction decoders */

static void DecodeFixed(Word Index)
{
  BaseOrder *POrder = FixedOrders + Index;

  if (ChkArgCnt(0, 0)
   && ChkMinCPU(POrder->MinCPU))
  {
    if (POrder->Len == 2)
    {
      BAsmCode[PrefixCnt++] = Hi(POrder->Code);
      BAsmCode[PrefixCnt++] = Lo(POrder->Code);
    }
    else
      BAsmCode[PrefixCnt++] = Lo(POrder->Code);
    CodeLen = PrefixCnt;
  }
}

static void DecodeAcc(Word Index)
{
  BaseOrder *POrder = AccOrders + Index;

  if (!ChkArgCnt(0, 1));
  else if (!ChkMinCPU(POrder->MinCPU));
  else if ((ArgCnt) && (strcasecmp(ArgStr[1], "A"))) WrError(1350);
  else
  {
    if (POrder->Len == 2)
    {
      BAsmCode[PrefixCnt++] = Hi(POrder->Code);
      BAsmCode[PrefixCnt++] = Lo(POrder->Code);
    }
    else
      BAsmCode[PrefixCnt++] = Lo(POrder->Code);
    CodeLen = PrefixCnt;
  }
}

static void DecodeHL(Word Index)
{
  BaseOrder *POrder = HLOrders + Index;

  if (!ChkArgCnt(0, 1));
  else if (!ChkMinCPU(POrder->MinCPU));
  else if ((ArgCnt) && (strcasecmp(ArgStr[1], "HL"))) WrError(1350);
  else
  {
    if (POrder->Len == 2)
    {
      BAsmCode[PrefixCnt++] = Hi(POrder->Code);
      BAsmCode[PrefixCnt++] = Lo(POrder->Code);
    }
    else
      BAsmCode[PrefixCnt++] = Lo(POrder->Code);
    CodeLen = PrefixCnt;
  }
}

static void DecodeLD(Word IsLDW)
{
  Byte AdrByte,HLen;
  int z;
  Byte HVals[5];

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg8:
        if (AdrPart == 7) /* LD A, ... */
        {
          OpSize = 0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg8: /* LD A, R8/RX8/(HL)/(XY+D) */
              BAsmCode[PrefixCnt] = 0x78 + AdrPart;
              memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 1 + AdrCnt;
              break;
            case ModIndReg16: /* LD A, (BC)/(DE) */
              BAsmCode[PrefixCnt++] = 0x0a + (AdrPart << 4);
              CodeLen = PrefixCnt;
              break;
            case ModImm: /* LD A, imm8 */
              BAsmCode[PrefixCnt++] = 0x3e;
              BAsmCode[PrefixCnt++] = AdrVals[0];
              CodeLen = PrefixCnt;
              break;
            case ModAbs: /* LD a, (adr) */
              BAsmCode[PrefixCnt] = 0x3a;
              memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 1 + AdrCnt;
              break;
            case ModRef: /* LD A, R */
              BAsmCode[PrefixCnt++] = 0xed;
              BAsmCode[PrefixCnt++] = 0x5f;
              CodeLen = PrefixCnt;
              break;
            case ModInt: /* LD A, I */
              BAsmCode[PrefixCnt++] = 0xed;
              BAsmCode[PrefixCnt++] = 0x57;
              CodeLen = PrefixCnt;
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        else if ((AdrPart != 6) && (PrefixCnt == 0)) /* LD R8, ... */
        {
          AdrByte = AdrPart; OpSize = 0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg8: /* LD R8, R8/RX8/(HL)/(XY+D) */
              /* if (I(XY)+d) as target, cannot use H/L as source ! */
              if (((AdrByte == 4) || (AdrByte == 5)) && (IndexPrefix()) && (AdrCnt == 0)) WrError(1350);
              else
              {
                BAsmCode[PrefixCnt] = 0x40 + (AdrByte << 3) + AdrPart;
                memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                CodeLen = PrefixCnt + 1 + AdrCnt;
              }
              break;
            case ModImm: /* LD R8, imm8 */
              BAsmCode[0] = 0x06 + (AdrByte << 3); BAsmCode[1] = AdrVals[0];
              CodeLen = 2;
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        else if ((AdrPart == 4) || (AdrPart == 5)) /* LD RX8, ... */
        {
          AdrByte = AdrPart; OpSize = 0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg8: /* LD RX8, R8/RX8 */
              if (AdrPart == 6) WrError(1350);        /* stopped here */
              else if ((AdrPart >= 4) && (AdrPart <= 5) && (PrefixCnt != 2)) WrError(1350);
              else if ((AdrPart >= 4) && (AdrPart <= 5) && (BAsmCode[0] != BAsmCode[1])) WrError(1350);
              else
              {
                if (PrefixCnt == 2) PrefixCnt--;
                BAsmCode[PrefixCnt] = 0x40 + (AdrByte << 3) + AdrPart;
                CodeLen = PrefixCnt + 1;
              }
              break;
            case ModImm: /* LD RX8,imm8 */
              BAsmCode[PrefixCnt]=0x06+(AdrByte << 3);
              BAsmCode[PrefixCnt+1]=AdrVals[0];
              CodeLen=PrefixCnt+2;
              break;
            default:
              if (AdrMode!=ModNone) WrError(1350);
          }
        }
        else /* LD (HL)/(XY+d),... */
        {
          HLen = AdrCnt;
          memcpy(HVals, AdrVals, AdrCnt);
          z = PrefixCnt;
          if ((z == 0) && (IsLDW))
          {
            OpSize = 1;
            MayLW = True;
          }
          else
            OpSize = 0;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg8: /* LD (HL)/(XY+D),R8 */
              if ((PrefixCnt != z) || (AdrPart == 6)) WrError(1350);
              else
              {
                BAsmCode[PrefixCnt] = 0x70 + AdrPart;
                memcpy(BAsmCode + PrefixCnt + 1, HVals, HLen);
                CodeLen=PrefixCnt + 1 + HLen;
              }
              break;
            case ModImm: /* LD (HL)/(XY+D),imm8:16:32 */
              if ((z == 0) && (IsLDW))
              {
                if (ChkMinCPU(CPUZ380))
                {
                  BAsmCode[PrefixCnt] = 0xed;
                  BAsmCode[PrefixCnt + 1] = 0x36;
                  memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                  CodeLen = PrefixCnt + 2 + AdrCnt;
                }
              }
              else
              {
                BAsmCode[PrefixCnt] = 0x36;
                memcpy(BAsmCode + 1 + PrefixCnt, HVals, HLen);
                BAsmCode[PrefixCnt + 1 + HLen] = AdrVals[0];
                CodeLen = PrefixCnt + 1 + HLen + AdrCnt;
              }
              break;
            case ModReg16: /* LD (HL)/(XY+D),R16/XY */
              if (!ChkMinCPU(CPUZ380));
              else if (AdrPart == 3) WrError(1350);
              else if (HLen == 0)
              {
                if (PrefixCnt == z) /* LD (HL),R16 */
                {
                  if (AdrPart == 2)
                    AdrPart = 3;
                  BAsmCode[0] = 0xfd;
                  BAsmCode[1] = 0x0f + (AdrPart << 4);
                  CodeLen = 2;
                }
                else /* LD (HL),XY */
                {
                  CodeLen = PrefixCnt + 1;
                  BAsmCode[PrefixCnt] = 0x31;
                  CodeLen = 1 + PrefixCnt;
                }
              }
              else
              {
                if (PrefixCnt == z) /* LD (XY+D),R16 */
                {
                  if (AdrPart == 2)
                    AdrPart = 3;
                  BAsmCode[PrefixCnt] = 0xcb;
                  memcpy(BAsmCode + PrefixCnt + 1, HVals, HLen);
                  BAsmCode[PrefixCnt + 1 + HLen] = 0x0b + (AdrPart << 4);
                  CodeLen = PrefixCnt + 1 + HLen + 1;
                }
                else if (BAsmCode[0] == BAsmCode[1]) WrError(1350);
                else
                {
                  PrefixCnt--;
                  BAsmCode[PrefixCnt] = 0xcb;
                  memcpy(BAsmCode+PrefixCnt + 1, HVals, HLen);
                  BAsmCode[PrefixCnt + 1 + HLen] = 0x2b;
                  CodeLen = PrefixCnt + 1 + HLen + 1;
                }
              }
              break;
            default:
              if (AdrMode!=ModNone) WrError(1350);
          }
        }
        break;
      case ModReg16:
        if (AdrPart == 3) /* LD SP,... */
        {
          OpSize = 1;
          MayLW = True;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg16: /* LD SP,HL/XY */
              if (AdrPart != 2) WrError(1350);
              else
              {
                BAsmCode[PrefixCnt] = 0xf9;
                CodeLen = PrefixCnt + 1;
              }
              break;
            case ModImm: /* LD SP,imm16:32 */
              BAsmCode[PrefixCnt] = 0x31;
              memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 1 + AdrCnt;
              break;
            case ModAbs: /* LD SP,(adr) */
              BAsmCode[PrefixCnt] = 0xed;
              BAsmCode[PrefixCnt + 1] = 0x7b;
              memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 2 + AdrCnt;
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        else if (PrefixCnt == 0) /* LD R16,... */
        {
          AdrByte = (AdrPart == 2) ? 3 : AdrPart;
          OpSize = 1;
          MayLW = True;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModInt: /* LD HL,I */
              if (!ChkMinCPU(CPUZ380));
              else if (AdrByte != 3) WrError(1350);
              else
              {
                BAsmCode[0] = 0xdd;
                BAsmCode[1] = 0x57;
                CodeLen = 2;
              }
              break;
            case ModReg8:
              if (AdrPart != 6) WrError(1350);
              else if (!ChkMinCPU(CPUZ380));
              else if (PrefixCnt == 0) /* LD R16,(HL) */
              {
                BAsmCode[0] = 0xdd;
                BAsmCode[1] = 0x0f + (AdrByte << 4);
                CodeLen = 2;
              }
              else /* LD R16,(XY+d) */
              {
                BAsmCode[PrefixCnt] = 0xcb;
                memcpy(BAsmCode+PrefixCnt + 1, AdrVals, AdrCnt);
                BAsmCode[PrefixCnt + 1 + AdrCnt] = 0x03 + (AdrByte << 4);
                CodeLen = PrefixCnt + 1 + AdrCnt + 1;
              }
              break;
            case ModReg16:
              if (AdrPart == 3) WrError(1350);
              else if (!ChkMinCPU(CPUZ380));
              else if (PrefixCnt == 0) /* LD R16,R16 */
              {
                if (AdrPart == 2)
                  AdrPart = 3;
                else if (AdrPart == 0)
                  AdrPart = 2;
                BAsmCode[0] = 0xcd + (AdrPart << 4);
                BAsmCode[1] = 0x02 + (AdrByte << 4);
                CodeLen = 2;
              }
              else /* LD R16,XY */
              {
                BAsmCode[PrefixCnt] = 0x0b + (AdrByte << 4);
                CodeLen=PrefixCnt + 1;
              }
              break;
            case ModIndReg16: /* LD R16,(R16) */
              if (ChkMinCPU(CPUZ380))
              {
                CodeLen = 2;
                BAsmCode[0] = 0xdd;
                BAsmCode[1] = 0x0c + (AdrByte << 4) + AdrPart;
              }
              break;
            case ModImm: /* LD R16,imm */
              if (AdrByte == 3)
                AdrByte = 2;
              CodeLen=PrefixCnt + 1 + AdrCnt;
              BAsmCode[PrefixCnt] = 0x01 + (AdrByte << 4);
              memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
              break;
            case ModAbs: /* LD R16,(adr) */
              if (AdrByte == 3)
              {
                BAsmCode[PrefixCnt] = 0x2a;
                memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                CodeLen = 1 + PrefixCnt + AdrCnt;
              }
              else
              {
                BAsmCode[PrefixCnt] = 0xed;
                BAsmCode[PrefixCnt+1] = 0x4b + (AdrByte << 4);
                memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                CodeLen = PrefixCnt + 2 + AdrCnt;
              }
              break;
            case ModSPRel: /* LD R16,(SP+D) */
              if (ChkMinCPU(CPUZ380))
              {
                BAsmCode[PrefixCnt] = 0xdd;
                BAsmCode[PrefixCnt + 1] = 0xcb;
                memcpy(BAsmCode + PrefixCnt + 2, AdrVals, AdrCnt);
                BAsmCode[PrefixCnt + 2 + AdrCnt] = 0x01 + (AdrByte << 4);
                CodeLen=PrefixCnt + 3 + AdrCnt;
              }
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        else /* LD XY,... */
        {
          OpSize = 1;
          MayLW = True;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg8:
              if (AdrPart != 6) WrError(1350);
              else if (!ChkMinCPU(CPUZ380));
              else if (AdrCnt == 0) /* LD XY,(HL) */
              {
                BAsmCode[PrefixCnt] = 0x33;
                CodeLen = PrefixCnt + 1;
              }
              else if (BAsmCode[0] == BAsmCode[1]) WrError(1350);
              else /* LD XY,(XY+D) */
              {
                BAsmCode[0] = BAsmCode[1];
                PrefixCnt--;
                BAsmCode[PrefixCnt] = 0xcb;
                memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                BAsmCode[PrefixCnt + 1 + AdrCnt] = 0x23;
                CodeLen = PrefixCnt + 1 + AdrCnt + 1;
              }
              break;
            case ModReg16:
              if (!ChkMinCPU(CPUZ380));
              else if (AdrPart == 3) WrError(1350);
              else if (PrefixCnt == 1) /* LD XY,R16 */
              {
                if (AdrPart == 2) AdrPart = 3;
                CodeLen = 1 + PrefixCnt;
                BAsmCode[PrefixCnt] = 0x07 + (AdrPart << 4);
              }
              else if (BAsmCode[0] == BAsmCode[1]) WrError(1350);
              else /* LD XY,XY */
              {
                BAsmCode[--PrefixCnt] = 0x27;
                CodeLen = 1 + PrefixCnt;
              }
              break;
            case ModIndReg16:
              if (ChkMinCPU(CPUZ380)) /* LD XY,(R16) */
              {
                BAsmCode[PrefixCnt] = 0x03 + (AdrPart << 4);
                CodeLen = PrefixCnt + 1;
              }
              break;
            case ModImm: /* LD XY,imm16:32 */
              BAsmCode[PrefixCnt] = 0x21;
              memcpy(BAsmCode+PrefixCnt + 1, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 1 + AdrCnt;
              break;
            case ModAbs: /* LD XY,(adr) */
              BAsmCode[PrefixCnt] = 0x2a;
              memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
              CodeLen = PrefixCnt + 1 + AdrCnt;
              break;
            case ModSPRel: /* LD XY,(SP+D) */
              if (ChkMinCPU(CPUZ380))
              {
                BAsmCode[PrefixCnt] = 0xcb;
                memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
                BAsmCode[PrefixCnt + 1 + AdrCnt] = 0x21;
                CodeLen = PrefixCnt + 1 + AdrCnt + 1;
              }
              break;
            default:
              if (AdrMode!=ModNone) WrError(1350);
          }
        }
        break;
      case ModIndReg16:
        AdrByte = AdrPart;
        if (IsLDW)
        {
          OpSize = 1;
          MayLW = True;
        }
        else
          OpSize = 0;
        DecodeAdr(ArgStr[2]);
        switch (AdrMode)
        {
          case ModReg8: /* LD (R16),A */
            if (AdrPart != 7) WrError(1350);
            else
            {
              CodeLen = 1;
              BAsmCode[0] = 0x02 + (AdrByte << 4);
            }
            break;
          case ModReg16:
            if (AdrPart == 3) WrError(1350);
            else if (!ChkMinCPU(CPUZ380));
            else if (PrefixCnt == 0) /* LD (R16),R16 */
            {
              if (AdrPart == 2)
                AdrPart = 3;
              BAsmCode[0] = 0xfd;
              BAsmCode[1] = 0x0c + AdrByte + (AdrPart << 4);
              CodeLen = 2;
            }
            else /* LD (R16),XY */
            {
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0x01 + (AdrByte << 4);
            }
            break;
          case ModImm:
            if (!IsLDW) WrError(1350);
            else if (ChkMinCPU(CPUZ380))
            {
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0xed;
              BAsmCode[CodeLen++] = 0x06 + (AdrByte << 4);
              AppendAdrVals();
            }
            break;
          default:
            if (AdrMode != ModNone) WrError(1350);
        }
        break;
      case ModAbs:
        HLen = AdrCnt;
        memcpy(HVals, AdrVals, AdrCnt);
        OpSize = 0;
        DecodeAdr(ArgStr[2]);
        switch (AdrMode)
        {
          case ModReg8: /* LD (adr),A */
            if (AdrPart != 7) WrError(1350);
            else
            {
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0x32;
              AppendVals(HVals, HLen);
            }
            break;
          case ModReg16:
            if (AdrPart == 2) /* LD (adr),HL/XY */
            {
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0x22;
              AppendVals(HVals, HLen);
            }
            else /* LD (adr),R16 */
            {
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0xed;
              BAsmCode[CodeLen++] = 0x43 + (AdrPart << 4);
              AppendVals(HVals, HLen);
            }
            break;
          default:
            if (AdrMode!=ModNone) WrError(1350);
        }
        break;
      case ModInt:
        if (!strcasecmp(ArgStr[2], "A")) /* LD I,A */
        {
          CodeLen = 2;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0x47;
        }
        else if (!strcasecmp(ArgStr[2], "HL")) /* LD I,HL */
        {
         if (ChkMinCPU(CPUZ380))
         {
           CodeLen = 2;
           BAsmCode[0] = 0xdd;
           BAsmCode[1] = 0x47;
         }
        }
        else WrError(1350);
        break;
      case ModRef:
        if (!strcasecmp(ArgStr[2], "A")) /* LD R,A */
        {
          CodeLen = 2;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0x4f;
        }
        else WrError(1350);
        break;
      case ModSPRel:
        if (ChkMinCPU(CPUZ380))
        {
          HLen = AdrCnt;
          memcpy(HVals, AdrVals, AdrCnt);
          OpSize = 0;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg16:
              if (AdrPart == 3) WrError(1350);
              else if (PrefixCnt == 0) /* LD (SP+D),R16 */
              {
                if (AdrPart == 2)
                  AdrPart = 3;
                CodeLen = PrefixCnt;
                BAsmCode[CodeLen++] = 0xdd;
                BAsmCode[CodeLen++] = 0xcb;
                AppendVals(HVals, HLen);
                BAsmCode[CodeLen++] = 0x09 + (AdrPart << 4);
              }
              else /* LD (SP+D),XY */
              {
                CodeLen = PrefixCnt;
                BAsmCode[CodeLen++] = 0xcb;
                AppendVals(HVals, HLen);
                BAsmCode[CodeLen++] = 0x29;
              }
              break;
            default:
              if (AdrMode!=ModNone) WrError(1350);
          }
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }  /* outer switch */
  }
}

static void DecodeALU8(Word Code)
{
  char *pSrcArg, *pDestArg;

  switch (ArgCnt)
  {
    case 1:
      strmaxcpy(ArgStr[2], "A", 255);
      pDestArg = ArgStr[2];
      pSrcArg = ArgStr[1];
      break;
    case 2:
      pDestArg = ArgStr[1];
      pSrcArg = ArgStr[2];
      break;
    default:
      (void)ChkArgCnt(1, 2);
      return;
  }

  if (!strcasecmp(pDestArg, "HL"))
  {
    if (Code != 2) WrError(1350);
    else
    {
      OpSize = 1; DecodeAdr(pSrcArg);
      switch (AdrMode)
      {
        case ModAbs:
          CodeLen = PrefixCnt;
          BAsmCode[CodeLen++] = 0xed;
          BAsmCode[CodeLen++] = 0xd6;
          AppendAdrVals();
          break;
        default:
          if (AdrMode != ModNone) WrError(1350);
      }
    }
  }
  else if (!strcasecmp(pDestArg, "SP"))
  {
    if (Code != 2) WrError(1350);
    else
    {
      OpSize = 1; DecodeAdr(pSrcArg);
      switch (AdrMode)
      {
        case ModImm:
          CodeLen = 0;
          BAsmCode[CodeLen++] = 0xed;
          BAsmCode[CodeLen++] = 0x92;
          AppendAdrVals();
          break;
        default:
          if (AdrMode != ModNone) WrError(1350);
      }
    }
  }
  else if (strcasecmp(pDestArg, "A")) WrError(1350);
  else
  {
    OpSize = 0; DecodeAdr(pSrcArg);
    switch (AdrMode)
    {
      case ModReg8:
        CodeLen = PrefixCnt + 1 + AdrCnt;
        BAsmCode[PrefixCnt] = 0x80 + (Code << 3) + AdrPart;
        memcpy(BAsmCode + PrefixCnt + 1, AdrVals, AdrCnt);
        break;
      case ModImm:
        if (!ImmIs8()) WrError(1320);
        else
        {
          CodeLen = 2;
          BAsmCode[0] = 0xc6 + (Code << 3);
          BAsmCode[1] = AdrVals[0];
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeALU16(Word Code)
{
  if (!ChkArgCnt(1, 2));
  else if (!ChkMinCPU(CPUZ380));
  else if ((ArgCnt == 2) && (strcasecmp(ArgStr[1], "HL"))) WrError(1350);
  else
  {
    OpSize = 1; DecodeAdr(ArgStr[ArgCnt]);
    switch (AdrMode)
    {
      case ModReg16:
        if (PrefixCnt > 0)      /* wenn Register, dann nie DDIR! */
        {
          BAsmCode[PrefixCnt] = 0x87 + (Code << 3);
          CodeLen = 1 + PrefixCnt;
        }
        else if (AdrPart == 3) WrError(1350);
        else
        {
          if (AdrPart == 2)
            AdrPart = 3;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0x84 + (Code << 3) + AdrPart;
          CodeLen = 2;
        }
        break;
      case ModReg8:
        if ((AdrPart != 6) || (AdrCnt == 0)) WrError(1350);
        else
        {
          CodeLen = PrefixCnt;
          BAsmCode[CodeLen++] = 0xc6 + (Code << 3);
          AppendAdrVals();
        }
        break;
      case ModImm:
        CodeLen = 0;
        BAsmCode[CodeLen++] = 0xed;
        BAsmCode[CodeLen++] = 0x86 + (Code << 3);
        AppendAdrVals();
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeADD(Word Index)
{
  UNUSED(Index);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg8:
        if (AdrPart != 7) WrError(1350);
        else
        {
          OpSize = 0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg8:
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0x80 + AdrPart;
              AppendAdrVals();
              break;
            case ModImm:
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0xc6;
              AppendAdrVals();
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        break;
      case ModReg16:
        if (AdrPart == 3) /* SP */
        {
          OpSize = (MomCPU == CPUZ380) ? 1 : 0;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModImm:
              switch (ChkExactCPUList(0, CPUZ380, CPUR2000, CPUNone))
              {
                case 0:
                  BAsmCode[0] = 0xed; BAsmCode[1] = 0x82;
                  memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                  CodeLen = 2 + AdrCnt;
                  break;
                case 1:
                  BAsmCode[0] = 0x27; BAsmCode[1] = 0[AdrVals];
                  CodeLen = 2;
                  break;
              }
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        else if (AdrPart != 2) WrError(1350);
        else
        {
          Boolean HasPrefixes = (PrefixCnt> 0);

          OpSize = 1; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg16:
              if ((AdrPart == 2) && (PrefixCnt != 0) && ((PrefixCnt != 2) || (BAsmCode[0] != BAsmCode[1]))) WrError(1350);
              else
              {
                if (PrefixCnt == 2)
                  PrefixCnt--;
                CodeLen = PrefixCnt;
                BAsmCode[CodeLen++] = 0x09 + (AdrPart << 4);
              }
              break;
            case ModAbs:
              if (HasPrefixes) WrError(1350);
              else if (ChkMinCPU(CPUZ380))
              {
                CodeLen = PrefixCnt;
                BAsmCode[CodeLen++] = 0xed;
                BAsmCode[CodeLen++] = 0xc2;
                AppendAdrVals();
              }
              break;
            default:
              if (AdrMode!=ModNone) WrError(1350);
          }
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeADDW(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 2));
  else if (!ChkMinCPU(CPUZ380));
  else if ((ArgCnt == 2) && (strcasecmp(ArgStr[1], "HL"))) WrError(1350);
  else
  {
    OpSize = 1; DecodeAdr(ArgStr[ArgCnt]);
    switch (AdrMode)
    {
      case ModReg16:
        if (PrefixCnt > 0)      /* wenn Register, dann nie DDIR! */
        {
          BAsmCode[PrefixCnt] = 0x87;
          CodeLen = 1 + PrefixCnt;
        }
        else if (AdrPart == 3) WrError(1350);
        else
        {
          if (AdrPart == 2)
            AdrPart = 3;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0x84 + AdrPart;
          CodeLen = 2;
        }
        break;
      case ModReg8:
        if ((AdrPart != 6) || (AdrCnt == 0)) WrError(1350);
        else
        {
          CodeLen = PrefixCnt;
          BAsmCode[CodeLen++] = 0xc6;
          AppendAdrVals();
        }
        break;
      case ModImm:
        BAsmCode[0] = 0xed;
        BAsmCode[1] = 0x86;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeADC_SBC(Word IsSBC)
{
  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg8:
        if (AdrPart != 7) WrError(1350);
        else
        {
          OpSize = 0; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg8:
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0x88 + AdrPart;
              AppendAdrVals();
              break;
            case ModImm:
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0xce;
              AppendAdrVals();
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
          if ((IsSBC) && (CodeLen != 0))
            BAsmCode[PrefixCnt] += 0x10;
        }
        break; 
      case ModReg16:
        if ((AdrPart != 2) || (PrefixCnt != 0)) WrError(1350);
        else
        {
          OpSize = 1; DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg16:
              if (PrefixCnt != 0) WrError(1350);
              else
              {
                CodeLen = 2;
                BAsmCode[0] = 0xed;
                BAsmCode[1] = 0x42 + (AdrPart << 4);
                if (!IsSBC)
                  BAsmCode[1] += 8;
              }
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeADCW_SBCW(Word Code)
{
  if (!ChkArgCnt(1, 2));
  else if (!ChkMinCPU(CPUZ380));
  else if ((ArgCnt == 2) && (strcasecmp(ArgStr[1], "HL"))) WrError(1350);
  else
  {
    OpSize = 1; DecodeAdr(ArgStr[ArgCnt]);
    switch (AdrMode)
    {
      case ModReg16:
        if (PrefixCnt > 0)      /* wenn Register, dann nie DDIR! */
        {
          BAsmCode[PrefixCnt] = 0x8f + Code;
          CodeLen = 1 + PrefixCnt;
        }
        else if (AdrPart == 3) WrError(1350);
        else
        {
          if (AdrPart == 2)
            AdrPart = 3;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0x8c + Code + AdrPart;
          CodeLen = 2;
        }
        break;
      case ModReg8:
        if ((AdrPart != 6) || (AdrCnt == 0)) WrError(1350);
        else
        {
          CodeLen = PrefixCnt;
          BAsmCode[CodeLen++] = 0xce + Code; /* ANSI :-0 */
          AppendAdrVals();
        }
        break;
      case ModImm:
        CodeLen = 0;
        BAsmCode[CodeLen++] = 0xed;
        BAsmCode[CodeLen++] = 0x8e + Code;
        AppendAdrVals();
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeINC_DEC(Word Index)
{
  Word IsDEC = (Index & 1), IsWord = (Index & 2);

  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg8:
        if (IsWord) WrError(1350);
        else
        {
          CodeLen = PrefixCnt;
          BAsmCode[CodeLen++] = 0x04 + (AdrPart << 3) + IsDEC;
          AppendAdrVals();
        }
        break;
      case ModReg16:
        CodeLen = PrefixCnt;
        BAsmCode[CodeLen++] = 0x03 + (AdrPart << 4) + (IsDEC << 3);
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeShift8(Word Code)
{
  Boolean OK;

  if (!ChkArgCnt(1, 2));
  else if ((Code == 6) && (!ChkExactCPU(CPUZ80U))); /* SLIA/SLS undok. Z80 */
  else
  {
    OpSize = 0;
    DecodeAdr(ArgStr[ArgCnt]);
    switch (AdrMode)
    {
      case ModReg8:
        if ((PrefixCnt > 0) && (AdrPart != 6)) WrError(1350); /* IXL..IYU verbieten */
        else
        {
          if (ArgCnt == 1)
            OK = True;
          else if (!ChkExactCPU(CPUZ80U))
            OK = False;
          else if ((AdrPart != 6) || (PrefixCnt != 1) || (!DecodeReg8(ArgStr[1], &AdrPart)))
          {
            WrError(1350);
            OK = False;
          }
          else
            OK = True;
          if (OK)
          {
            CodeLen = PrefixCnt;
            BAsmCode[CodeLen++]=0xcb;
            AppendAdrVals();
            BAsmCode[CodeLen++] = AdrPart + (Code << 3);
          }
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeShift16(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (ChkMinCPU(CPUZ380))
  {
    OpSize = 1; DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg16:
        if (PrefixCnt > 0)
        {
          BAsmCode[2] = 0x04 + (Code << 3) + ((BAsmCode[0] >> 5) & 1);
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0xcb;
          CodeLen = 3;
        }
        else if (AdrPart == 3) WrError(1350);
        else
        {
          if (AdrPart == 2)
            AdrPart = 3;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0xcb;
          BAsmCode[2] = (Code << 3) + AdrPart;
          CodeLen = 3;
        }
        break;
      case ModReg8:
        if (AdrPart != 6) WrError(1350);
        else
        {
          if (AdrCnt == 0)
          {
            BAsmCode[0] = 0xed;
            PrefixCnt = 1;
          }
          CodeLen = PrefixCnt;
          BAsmCode[CodeLen++] = 0xcb;
          AppendAdrVals();
          BAsmCode[CodeLen++] = 0x02 + (Code << 3);
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeBit(Word Code)
{
  if (!ChkArgCnt(2, 3));
  else
  {
    DecodeAdr(ArgStr[ArgCnt]);
    switch (AdrMode)
    {
      case ModReg8:
        if ((AdrPart != 6) && (PrefixCnt != 0)) WrError(1350);
        else
        {
          Boolean OK;
          Byte BitPos;

          BitPos = EvalIntExpression(ArgStr[ArgCnt - 1], UInt3, &OK);
          if (OK)
          {
            if (ArgCnt == 2)
              OK = True;
            else if (!ChkExactCPU(CPUZ80U))
              OK = False;
            else if ((AdrPart != 6) || (PrefixCnt != 1) || (Code == 0) || (!DecodeReg8(ArgStr[1], &AdrPart)))
            {
              WrError(1350);
              OK = False;
            }
            else
              OK = True;
            if (OK)
            {
              CodeLen = PrefixCnt;
              BAsmCode[CodeLen++] = 0xcb;
              AppendAdrVals();
              BAsmCode[CodeLen++] = AdrPart + (BitPos << 3) + ((Code + 1) << 6);
            }
          }
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350); 
    }
  }
}

static void DecodeMLT(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (ChkMinCPU(CPUZ180))
  {
    DecodeAdr(ArgStr[1]);
    if ((AdrMode != ModReg16) || (PrefixCnt != 0)) WrError(1350);
    else
    {
      BAsmCode[CodeLen] = 0xed;
      BAsmCode[CodeLen + 1] = 0x4c + (AdrPart << 4);
      CodeLen = 2;
    }
  }
}

static void DecodeMULT_DIV(Word Code)
{
  char *pSrcArg, *pDestArg;

  switch (ArgCnt)
  {
    case 1:
      strmaxcpy(ArgStr[2], "HL", 255);
      pDestArg = ArgStr[2];
      pSrcArg = ArgStr[1];
      break;
    case 2:
      pDestArg = ArgStr[1];
      pSrcArg = ArgStr[2];
      break;
    default:
      (void)ChkArgCnt(1, 2);
      return;
  }

  if (!ChkMinCPU(CPUZ380));
  else if (strcasecmp(pDestArg, "HL")) WrError(1350);
  else
  {
    OpSize = 1; DecodeAdr(pSrcArg);
    switch (AdrMode)
    {
      case ModReg8:
        if ((AdrPart != 6) || (PrefixCnt == 0)) WrError(1350);
        else
        {
          CodeLen = PrefixCnt;
          BAsmCode[CodeLen++] = 0xcb;
          AppendAdrVals();
          BAsmCode[CodeLen++] = 0x92 | Code;
        }
        break;
      case ModReg16:
        if (AdrPart == 3) WrError(1350);
        else if (PrefixCnt==0)
        {
          if (AdrPart == 2)
            AdrPart = 3;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0xcb;
          BAsmCode[2] = 0x90 + AdrPart + Code;
          CodeLen = 3;
        }
        else
        {
          BAsmCode[2] = 0x94 + ((BAsmCode[0] >> 5) & 1) + Code;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0xcb;
          CodeLen = 3;
        }
        break;
      case ModImm:
        CodeLen = 0;
        BAsmCode[CodeLen++] = 0xed;
        BAsmCode[CodeLen++] = 0xcb;
        BAsmCode[CodeLen++] = 0x97 + Code;
        AppendAdrVals();
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeTST(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (ChkMinCPU(CPUZ180))
  {
    OpSize = 0; DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg8:
        if (PrefixCnt != 0) WrError(1350);
        else
        {
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 4 + (AdrPart << 3);
          CodeLen = 2;
        }
        break;
      case ModImm:
        BAsmCode[0] = 0xed;
        BAsmCode[1] = 0x64;
        BAsmCode[2] = AdrVals[0];
        CodeLen = 3;
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeSWAP(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else if (ChkMinCPU(CPUZ380))
  {
    DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg16:
        if (AdrPart == 3) WrError(1350);
        else if (PrefixCnt == 0)
        {
          if (AdrPart == 2)
            AdrPart = 3;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0x0e + (AdrPart << 4); /*?*/
          CodeLen = 2;
        }
        else
        {
          BAsmCode[PrefixCnt] = 0x3e;
          CodeLen = PrefixCnt + 1;
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodePUSH_POP(Word Code)
{
  if (!ChkArgCnt(1, 1));
  else if (!strcasecmp(ArgStr[1], "SR"))
  {
    if (ChkMinCPU(CPUZ380))
    {
      CodeLen = 2;
      BAsmCode[0] = 0xed;
      BAsmCode[1] = 0xc1 + Code;
    }
  }
  else
  {
    if (!strcasecmp(ArgStr[1], "SP"))
      strmaxcpy(ArgStr[1], "A", 255);
    if (!strcasecmp(ArgStr[1], "AF"))
      strmaxcpy(ArgStr[1], "SP", 255);
    OpSize = 1; MayLW = True;
    DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg16:
        CodeLen = 1 + PrefixCnt;
        BAsmCode[PrefixCnt] = 0xc1 + (AdrPart << 4) + Code;
        break;
      case ModImm:
        if (ChkMinCPU(CPUZ380))
        {
          CodeLen = PrefixCnt;
          BAsmCode[CodeLen++] = 0xfd;
          BAsmCode[CodeLen++] = 0xf5;
          AppendAdrVals();
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeEX(Word Index)
{
  Boolean OK;
  Byte AdrByte;

  UNUSED(Index);

  /* work around the parser problem related to the ' character */ 

  if (!strncasecmp(ArgStr[2], "AF\'", 3))
    ArgStr[2][3] = '\0';

  if (!ChkArgCnt(2, 2));
  else if (ParPair("DE", "HL"))
  {
    BAsmCode[0] = 0xeb;
    CodeLen = 1;
  }
  else if (ParPair("AF", "AF\'"))
  {
    BAsmCode[0] = 0x08;
    CodeLen = 1;
  }
  else if (ParPair("AF", "AF`"))
  {
    BAsmCode[0] = 0x08;
    CodeLen = 1;
  }
  else if (ParPair("(SP)", "HL"))
  {
    BAsmCode[0] = 0xe3;
    CodeLen = 1;
  }
  else if (ParPair("(SP)", "IX"))
  {
    BAsmCode[0] = IXPrefix;
    BAsmCode[1] = 0xe3;
    CodeLen = 2;
  }
  else if (ParPair("(SP)", "IY"))
  {
    BAsmCode[0] = IYPrefix;
    BAsmCode[1] = 0xe3;
    CodeLen = 2;
  }
  else if (ParPair("(HL)", "A"))
  {
    if (ChkMinCPU(CPUZ380))
    {
      BAsmCode[0] = 0xed;
      BAsmCode[1] = 0x37;
      CodeLen = 2;
    }
  }
  else
  {
    if ((ArgStr[2][0]) && (ArgStr[2][strlen(ArgStr[2]) - 1] == '\''))
    {
      OK = True;
      ArgStr[2][strlen(ArgStr[2]) - 1] = '\0';
    }
    else
      OK = False;

    DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg8:
        if ((AdrPart == 6) || (PrefixCnt != 0)) WrError(1350);
        else
        {
          AdrByte = AdrPart;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg8:
              if ((AdrPart == 6) || (PrefixCnt != 0)) WrError(1350);
              else if (!ChkMinCPU(CPUZ380));
              else if ((AdrByte == 7) && (!OK))
              {
                BAsmCode[0] = 0xed;
                BAsmCode[1] = 0x07 + (AdrPart << 3);
                CodeLen = 2;
              }
              else if ((AdrPart == 7) && (!OK))
              {
                BAsmCode[0] = 0xed;
                BAsmCode[1] = 0x07 + (AdrByte << 3);
                CodeLen = 2;
              }
              else if ((OK) && (AdrPart == AdrByte))
              {
                BAsmCode[0] = 0xcb;
                BAsmCode[1] = 0x30 + AdrPart;
                CodeLen = 2;
              }
              else WrError(1350);
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        break;
      case ModReg16:
        if (AdrPart == 3) WrError(1350);
        else if (PrefixCnt == 0) /* EX R16,... */
        {
          AdrByte = (AdrPart == 2) ? 3 : AdrPart;
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg16:
              if (AdrPart == 3) WrError(1350);
              else if (!ChkMinCPU(CPUZ380));
              else if (OK)
              {
                if (AdrPart == 2)
                  AdrPart = 3;
                if ((PrefixCnt != 0) || (AdrPart != AdrByte)) WrError(1350);
                else
                {
                  BAsmCode[0]=0xed;
                  BAsmCode[1]=0xcb;
                  BAsmCode[2]=0x30+AdrByte;
                  CodeLen = 3;
                }
              }
              else if (PrefixCnt == 0)
              {
                if (AdrByte == 0)
                {
                  if (AdrPart == 2)
                    AdrPart = 3;
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0x01 + (AdrPart << 2);
                  CodeLen = 2;
                }
                else if (AdrPart == 0)
                {
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0x01 + (AdrByte << 2);
                  CodeLen = 2;
                }
              }
              else
              {
                if (AdrPart == 2)
                  AdrPart = 3;
                BAsmCode[1] = 0x03 + ((BAsmCode[0] >> 2) & 8) + (AdrByte << 4);
                BAsmCode[0] = 0xed;
                CodeLen = 2;
              }
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        else /* EX XY,... */
        {
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
          {
            case ModReg16:
              if (AdrPart == 3) WrError(1350);
              else if (!ChkMinCPU(CPUZ380));
              else if (OK)
              {
                if ((PrefixCnt != 2) || (BAsmCode[0] != BAsmCode[1])) WrError(1350);
                else
                {
                  BAsmCode[2] = ((BAsmCode[0] >> 5) & 1)+0x34;
                  BAsmCode[0] = 0xed;
                  BAsmCode[1] = 0xcb;
                  CodeLen = 3;
                }
              }
              else if (PrefixCnt == 1)
              {
                if (AdrPart == 2)
                  AdrPart = 3;
                BAsmCode[1] = ((BAsmCode[0] >> 2) & 8) + 3 + (AdrPart << 4);
                BAsmCode[0] = 0xed;
                CodeLen = 2;
              }
              else if (BAsmCode[0] == BAsmCode[1]) WrError(1350);
              else
              {
                BAsmCode[0] = 0xed;
                BAsmCode[1] = 0x2b;
                CodeLen = 2;
              }
              break;
            default:
              if (AdrMode != ModNone) WrError(1350);
          }
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
}

static void DecodeTSTI(Word Code)
{
  UNUSED(Code);

  if (ChkExactCPU(CPUZ80U)
   && ChkArgCnt(0, 0))
  {
    BAsmCode[0] = 0xed;
    BAsmCode[1] = 0x70;
    CodeLen = 2;
  }
}

static void DecodeIN_OUT(Word IsOUT)
{
  if ((ArgCnt == 1) && (!IsOUT))
  {
    if (!ChkExactCPU(CPUZ80U));
    else if (strcasecmp(ArgStr[1], "(C)")) WrError(1350);
    else
    {
      BAsmCode[0] = 0xed;
      BAsmCode[1] = 0x70;
      CodeLen = 2;
    }
  }
  else if (!ChkArgCnt(2, 2));
  else
  {
    char *pPortArg = IsOUT ? ArgStr[1] : ArgStr[2],
         *pRegArg = IsOUT ? ArgStr[2] : ArgStr[1];

    if (!strcasecmp(pPortArg, "(C)"))
    {
      OpSize = 0; DecodeAdr(pRegArg);
      switch (AdrMode)
      {
        case ModReg8:
          if ((AdrPart == 6) || (PrefixCnt != 0)) WrError(1350);
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0xed;
            BAsmCode[1] = 0x40 + (AdrPart << 3);
            if (IsOUT)
              BAsmCode[1]++;
          }
          break;
        case ModImm:
          if (!IsOUT) WrError(1350);
          else if ((MomCPU == CPUZ80U) && (AdrVals[0] == 0))
          {
            BAsmCode[0] = 0xed;
            BAsmCode[1] = 0x71;
            CodeLen = 2;
          }
          else if (ChkMinCPU(CPUZ380))
          {
            BAsmCode[0] = 0xed;
            BAsmCode[1] = 0x71;
            BAsmCode[2] = AdrVals[0];
            CodeLen = 3;
          }
          break;
        default:
          if (AdrMode != ModNone) WrError(1350);
      }
    }
    else if (strcasecmp(pRegArg, "A")) WrError(1350);
    else
    {
      Boolean OK;

      BAsmCode[1] = EvalIntExpression(pPortArg, UInt8, &OK);
      if (OK)
      {
        ChkSpace(SegIO);
        CodeLen = 2;
        BAsmCode[0] = IsOUT ? 0xd3 : 0xdb;
      }
    }
  }
}

static void DecodeINW_OUTW(Word IsOUTW)
{
  if (ChkArgCnt(2, 2)
   && ChkMinCPU(CPUZ380))
  {
    char *pPortArg = IsOUTW ? ArgStr[1] : ArgStr[2],
         *pRegArg  = IsOUTW ? ArgStr[2] : ArgStr[1];

    if (strcasecmp(pPortArg, "(C)")) WrError(1350);
    else
    {
      OpSize = 1; DecodeAdr(pRegArg);
      switch (AdrMode)
      {
        case ModReg16:
          if ((AdrPart == 3) || (PrefixCnt > 0)) WrError(1350);
          else
          {
            switch (AdrPart)
            {
              case 1: AdrPart = 2; break;
              case 2: AdrPart = 7; break;
            }
            BAsmCode[0] = 0xdd;
            BAsmCode[1] = 0x40 + (AdrPart << 3);
            if (IsOUTW)
              BAsmCode[1]++;
            CodeLen = 2;
          }
          break;
        case ModImm:
          if (!IsOUTW) WrError(1350);
          else
          {
            CodeLen = 0;
            BAsmCode[CodeLen++] = 0xfd;
            BAsmCode[CodeLen++] = 0x79;
            AppendAdrVals();
          }
          break;
        default:
          if (AdrMode != ModNone) WrError(1350);
      }
    }
  }
}

static void DecodeIN0_OUT0(Word IsOUT0)
{
  if (ChkArgCnt(1, 2)
   && ChkMinCPU(CPUZ180))
  {
    Boolean OK;
    char *pRegArg, *pPortArg;

    if (IsOUT0)
    {
      pRegArg = (ArgCnt == 2) ? ArgStr[2] : NULL;
      pPortArg = ArgStr[1];
    }
    else
    {
      pRegArg = (ArgCnt == 2) ? ArgStr[1] : NULL;
      pPortArg = ArgStr[ArgCnt];
    }
    OpSize = 0;
    if (!pRegArg)
    {
      AdrPart = 6;
      OK = True;
    }
    else
    {
      DecodeAdr(pRegArg);
      if ((AdrMode == ModReg8) && (AdrPart != 6) && (PrefixCnt == 0)) OK = True;
      else
      {
        OK = False;
        if (AdrMode != ModNone) WrError(1350);
      }
    }
    if (OK)
    {
      BAsmCode[2] = EvalIntExpression(pPortArg, UInt8, &OK);
      if (OK)
      {
        BAsmCode[0] = 0xed;
        BAsmCode[1] = AdrPart << 3;
        if (IsOUT0)
          BAsmCode[1]++;
        CodeLen = 3;
      }
    }
  }
}

static void DecodeINA_INAW_OUTA_OUTAW(Word Code)
{
  Word IsIn = Code & 8;
  LongWord AdrLong;

  if (ChkArgCnt(2, 2)
   && ChkMinCPU(CPUZ380))
  {
    char *pRegArg = IsIn ? ArgStr[1] : ArgStr[2],
         *pPortArg = IsIn ? ArgStr[2] : ArgStr[1];

    OpSize = Code & 1;
    if (((OpSize == 0) && (strcasecmp(pRegArg, "A")))
     || ((OpSize == 1) && (strcasecmp(pRegArg, "HL")))) WrError(1350);
    else
    {
      Boolean OK;

      AdrLong = EvalIntExpression(pPortArg, ExtFlag ? Int32 : UInt8, &OK);
      if (OK)
      {
        ChkSpace(SegIO);
        if (AdrLong > 0xffffff)
          ChangeDDPrefix("IW");
        else if (AdrLong > 0xffff)
          ChangeDDPrefix("IB");
        CodeLen = PrefixCnt;
        BAsmCode[CodeLen++] = 0xed + (OpSize << 4);
        BAsmCode[CodeLen++] = 0xd3 + IsIn;
        BAsmCode[CodeLen++] = AdrLong & 0xff;
        BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
        if (AdrLong > 0xffff)
          BAsmCode[CodeLen++] = (AdrLong >> 16) & 0xff;
        if (AdrLong > 0xffffff)
          BAsmCode[CodeLen++] = (AdrLong >> 24) & 0xff;
      }
    }
  }
}

static void DecodeTSTIO(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1)
   && ChkMinCPU(CPUZ180))
  {
    Boolean OK;

    BAsmCode[2] = EvalIntExpression(ArgStr[1], Int8, &OK);
    if (OK)
    {
      BAsmCode[0] = 0xed;
      BAsmCode[1] = 0x74;
      CodeLen = 3;
    }
  }
}

static void DecodeRET(Word Code)
{
  int Cond;

  UNUSED(Code);

  if (ArgCnt == 0)
  {
    CodeLen = 1;
    BAsmCode[0] = 0xc9;
  }
  else if (!ChkArgCnt(0, 1));
  else if (!DecodeCondition(ArgStr[1], &Cond)) WrXErrorPos(ErrNum_UndefCond, ArgStr[1], &ArgStrPos[1]);
  else
  {
    CodeLen = 1;
    BAsmCode[0] = 0xc0 + (Cond << 3);
  }
}

static void DecodeJP(Word Code)
{
  int Cond;
  Boolean OK;

  UNUSED(Code);

  if (ArgCnt == 1)
  {
    if (!strcasecmp(ArgStr[1], "(HL)"))
    {
      CodeLen = 1;
      BAsmCode[0] = 0xe9;
      return;
    }
    else if (!strcasecmp(ArgStr[1], "(IX)"))
    {
      CodeLen = 2;
      BAsmCode[0] = IXPrefix;
      BAsmCode[1] = 0xe9;
      return;
    }
    else if (!strcasecmp(ArgStr[1],"(IY)"))
    {
      CodeLen = 2;
      BAsmCode[0] = IYPrefix;
      BAsmCode[1] = 0xe9;
      return;
    }
    else
    {
      Cond = 1;
      OK = True;
    }
  }
  else if (ArgCnt == 2)
  {
    OK = DecodeCondition(ArgStr[1], &Cond);
    if (OK)
      Cond <<= 3;
    else
      WrXErrorPos(ErrNum_UndefCond, ArgStr[1], &ArgStrPos[1]);
  }
  else
  {
    (void)ChkArgCnt(1, 2);
    OK = False;
  }
  if (OK)
  {
    LongWord AdrLong;

    AdrLong = EvalAbsAdrExpression(ArgStr[ArgCnt], &OK);
    if (OK)
    {
      if ((AdrLong & U16Mask) == 0)
      {
        BAsmCode[0] = 0xc2 + Cond;
        BAsmCode[1] = Lo(AdrLong);
        BAsmCode[2] = Hi(AdrLong);
        CodeLen = 3;
      }
      else if ((AdrLong & U8Mask)==0)
      {
        ChangeDDPrefix("IB");
        CodeLen = 4 + PrefixCnt;
        BAsmCode[PrefixCnt] = 0xc2 + Cond;
        BAsmCode[PrefixCnt + 1] = Lo(AdrLong);
        BAsmCode[PrefixCnt + 2] = Hi(AdrLong);
        BAsmCode[PrefixCnt + 3] = Hi(AdrLong >> 8);
      }
      else
      {
        ChangeDDPrefix("IW");
        CodeLen = 5 + PrefixCnt;
        BAsmCode[PrefixCnt] = 0xc2 + Cond;
        BAsmCode[PrefixCnt + 1] = Lo(AdrLong);
        BAsmCode[PrefixCnt + 2] = Hi(AdrLong);
        BAsmCode[PrefixCnt + 3] = Hi(AdrLong >> 8);
        BAsmCode[PrefixCnt + 4] = Hi(AdrLong >> 16);
      }
    }
  }
}

static void DecodeCALL(Word Code)
{
  Boolean OK;
  int Condition;

  UNUSED(Code);

  switch (ArgCnt)
  {
    case 1:
      Condition = 9;
      OK = True;
      break;
    case 2:
      OK = DecodeCondition(ArgStr[1], &Condition);
      if (OK)
        Condition <<= 3;
      else
        WrXErrorPos(ErrNum_UndefCond, ArgStr[1], &ArgStrPos[1]);
      break;
    default:
      (void)ChkArgCnt(1, 2);
      OK = False;
  }

  if (OK)
  {
    LongWord AdrLong;

    AdrLong = EvalAbsAdrExpression(ArgStr[ArgCnt], &OK);
    if (OK)
    {
      if ((AdrLong & U16Mask) == 0)
      {
        CodeLen = 3;
        BAsmCode[0] = 0xc4 + Condition;
        BAsmCode[1] = Lo(AdrLong);
        BAsmCode[2] = Hi(AdrLong);
      }
      else if ((AdrLong & U8Mask) == 0)
      {
        ChangeDDPrefix("IB");
        CodeLen = PrefixCnt;
        BAsmCode[CodeLen++] = 0xc4 + Condition;
        BAsmCode[CodeLen++] = Lo(AdrLong);
        BAsmCode[CodeLen++] = Hi(AdrLong);
        BAsmCode[CodeLen++] = Hi(AdrLong >> 8);
      }
      else
      {
        ChangeDDPrefix("IW");
        CodeLen = PrefixCnt;
        BAsmCode[CodeLen++] = 0xc4 + Condition;
        BAsmCode[CodeLen++] = Lo(AdrLong);
        BAsmCode[CodeLen++] = Hi(AdrLong);
        BAsmCode[CodeLen++] = Hi(AdrLong >> 8);
        BAsmCode[CodeLen++] = Hi(AdrLong >> 16);
      }
    }
  }
}

static void DecodeJR(Word Code)
{
  Boolean OK;
  int Condition;

  UNUSED(Code);

  switch (ArgCnt)
  {
    case 1:
      Condition = 3;
      OK = True;
      break;
    case 2:
      OK = DecodeCondition(ArgStr[1], &Condition);
      if ((OK) && (Condition > 3))
        OK = False;
      if (OK)
        Condition += 4;
      else
        WrXErrorPos(ErrNum_UndefCond, ArgStr[1], &ArgStrPos[1]);
      break;
    default:
      (void)ChkArgCnt(1, 2);
      OK = False;
  }

  if (OK)
  {
    LongInt AdrLInt;

    AdrLInt = EvalAbsAdrExpression(ArgStr[ArgCnt], &OK);
    if (OK)
    {
      AdrLInt -= EProgCounter() + 2;
      if ((AdrLInt <= 0x7fl) && (AdrLInt >= -0x80l))
      {
        CodeLen = 2;
        BAsmCode[0] = Condition << 3;
        BAsmCode[1] = AdrLInt & 0xff;
      }
      else
      {
        if (MomCPU<CPUZ380) WrError(1370);
        else
        {
          AdrLInt -= 2;
          if ((AdrLInt <= 0x7fffl) && (AdrLInt >= -0x8000l))
          {
            CodeLen = 4;
            BAsmCode[0] = 0xdd;
            BAsmCode[1] = Condition << 3;
            BAsmCode[2] = AdrLInt & 0xff;
            BAsmCode[3] = (AdrLInt >> 8) & 0xff;
          }
          else
          {
            AdrLInt--;
            if ((AdrLInt <= 0x7fffffl) && (AdrLInt >= -0x800000l))
            {
              CodeLen = 5;
              BAsmCode[0] = 0xfd;
              BAsmCode[1] = Condition << 3;
              BAsmCode[2] = AdrLInt & 0xff;
              BAsmCode[3] = (AdrLInt >> 8) & 0xff;
              BAsmCode[4] = (AdrLInt >> 16) & 0xff;
            }
            else WrError(1370);
          }
        }
      }
    }
  }
}

static void DecodeCALR(Word Code)
{
  Boolean OK;
  int Condition;

  UNUSED(Code);

  switch (ArgCnt)
  {
    case 1:
      Condition = 9;
      OK = True;
      break;
    case 2:
      OK = DecodeCondition(ArgStr[1], &Condition);
      if (OK)
        Condition <<= 3;
      else
        WrXErrorPos(ErrNum_UndefCond, ArgStr[1], &ArgStrPos[1]);
      break;
    default:
      (void)ChkArgCnt(1, 2);
      OK = False;
  }

  if (OK)
  {
    if (ChkMinCPU(CPUZ380))
    {
      LongInt AdrLInt;

      AdrLInt = EvalAbsAdrExpression(ArgStr[ArgCnt], &OK);
      if (OK)
      {
        AdrLInt -= EProgCounter() + 3;
        if ((AdrLInt <= 0x7fl) && (AdrLInt >= -0x80l))
        {
          CodeLen = 3;
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0xc4 | Condition;
          BAsmCode[2] = AdrLInt & 0xff;
        }
        else
        {
          AdrLInt--;
          if ((AdrLInt <= 0x7fffl) && (AdrLInt >= -0x8000l))
          {
            CodeLen = 4;
            BAsmCode[0] = 0xdd;
            BAsmCode[1] = 0xc4 + Condition;
            BAsmCode[2] = AdrLInt & 0xff;
            BAsmCode[3] = (AdrLInt >> 8) & 0xff;
          }
          else
          {
            AdrLInt--;
            if ((AdrLInt <= 0x7fffffl) && (AdrLInt >= -0x800000l))
            {
              CodeLen = 5;
              BAsmCode[0] = 0xfd;
              BAsmCode[1] = 0xc4 + Condition;
              BAsmCode[2] = AdrLInt & 0xff;
              BAsmCode[3] = (AdrLInt >> 8) & 0xff;
              BAsmCode[4] = (AdrLInt >> 16) & 0xff;
            }
            else WrError(1370);
          }
        }
      }
    }
  }
}

static void DecodeDJNZ(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    LongInt AdrLInt;

    AdrLInt = EvalAbsAdrExpression(ArgStr[1], &OK);
    if (OK)
    {
      AdrLInt -= EProgCounter() + 2;
      if ((AdrLInt <= 0x7fl) & (AdrLInt >= -0x80l))
      {
        CodeLen = 2;
        BAsmCode[0] = 0x10;
        BAsmCode[1] = Lo(AdrLInt);
      }
      else if (MomCPU<CPUZ380) WrError(1370);
      else
      {
        AdrLInt -= 2;
        if ((AdrLInt <= 0x7fffl) && (AdrLInt >= -0x8000l))
        {
          CodeLen = 4;
          BAsmCode[0] = 0xdd;
          BAsmCode[1] = 0x10;
          BAsmCode[2] = AdrLInt & 0xff;
          BAsmCode[3] = (AdrLInt >> 8) & 0xff;
        }
        else
        {
          AdrLInt--;
          if ((AdrLInt <= 0x7fffffl) && (AdrLInt >= -0x800000l))
          {
            CodeLen = 5;
            BAsmCode[0] = 0xfd;
            BAsmCode[1] = 0x10;
            BAsmCode[2] = AdrLInt & 0xff;
            BAsmCode[3] = (AdrLInt >> 8) & 0xff;
            BAsmCode[4] = (AdrLInt >> 16) & 0xff;
          }
          else WrError(1370);
        }
      }
    }
  }
}

static void DecodeRST(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    Byte AdrByte;
    LargeWord SaveRadixBase = RadixBase;

#if 0
    /* some people like to regard the RST argument as a literal
       and leave away the 'h' to mark 38 as a hex number... */
    RadixBase = 16;
#endif

    FirstPassUnknown = False;
    AdrByte = EvalIntExpression(ArgStr[1], Int8, &OK);
    RadixBase = SaveRadixBase;

    if (FirstPassUnknown)
      AdrByte = AdrByte & 0x38;  
    if (OK)
    {
      if ((AdrByte > 0x38) || (AdrByte & 7)) WrError(1905);
      else
      {
        CodeLen = 1;
        BAsmCode[0] = 0xc7 + AdrByte;
      }
    }
  }
}

static void DecodeEI_DI(Word Code)
{
  if (ArgCnt == 0)
  {
    BAsmCode[0] = 0xf3 + Code;
    CodeLen = 1;
  }
  else if (ChkArgCnt(1, 1)
        && ChkMinCPU(CPUZ380))
  {
    Boolean OK;

    BAsmCode[2] = EvalIntExpression(ArgStr[1], UInt8, &OK);
    if (OK)
    {
      BAsmCode[0] = 0xdd;
      BAsmCode[1] = 0xf3 + Code;
      CodeLen = 3;
    }
  }
}

static void DecodeIM(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    Byte AdrByte;
    Boolean OK;

    AdrByte = EvalIntExpression(ArgStr[1], UInt2, &OK);
    if (OK)
    {
      if (AdrByte > 3) WrError(1320);
      else if ((AdrByte == 3) && (!ChkMinCPU(CPUZ380)));
      else
      {
        if (AdrByte == 3)
          AdrByte = 1;
        else if (AdrByte >= 1)
          AdrByte++;
        CodeLen = 2;
        BAsmCode[0] = 0xed;
        BAsmCode[1] = 0x46 + (AdrByte << 3);
      }
    }
  }
}

static void DecodeLDCTL(Word Code)
{
  Byte AdrByte;

  UNUSED(Code);

  OpSize = 0;
  if (!ChkArgCnt(2, 2));
  else if (!ChkMinCPU(CPUZ380));
  else if (DecodeSFR(ArgStr[1], &AdrByte))
  {
    DecodeAdr(ArgStr[2]);
    switch (AdrMode)
    {
      case ModReg8:
        if (AdrPart != 7) WrError(1350);
        else
        {
          BAsmCode[0] = 0xcd + ((AdrByte & 3) << 4);
          BAsmCode[1] = 0xc8 + ((AdrByte & 4) << 2);
          CodeLen = 2;
        }
        break;
      case ModReg16:
        if ((AdrByte != 1) || (AdrPart != 2) || (PrefixCnt != 0)) WrError(1350);
        else
        {
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0xc8;
          CodeLen = 2;
        }
        break;
      case ModImm:
        BAsmCode[0] = 0xcd +((AdrByte & 3) << 4);
        BAsmCode[1] = 0xca +((AdrByte & 4) << 2);
        BAsmCode[2] = AdrVals[0];
        CodeLen = 3;
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
  else if (DecodeSFR(ArgStr[2], &AdrByte))
  {
    DecodeAdr(ArgStr[1]);
    switch (AdrMode)
    {
      case ModReg8:
        if ((AdrPart != 7) || (AdrByte == 1)) WrError(1350);
        else
        {
          BAsmCode[0] = 0xcd + ((AdrByte & 3) << 4);
          BAsmCode[1] = 0xd0;
          CodeLen = 2;
        }
        break;
      case ModReg16:
        if ((AdrByte != 1) || (AdrPart != 2) || (PrefixCnt != 0)) WrError(1350);
        else
        {
          BAsmCode[0] = 0xed;
          BAsmCode[1] = 0xc0;
          CodeLen = 2;
        }
        break;
      default:
        if (AdrMode != ModNone) WrError(1350);
    }
  }
  else
    WrError(1350);
}

static void DecodeRESC_SETC(Word Code)
{
  if (ChkArgCnt(1, 1)
   && ChkMinCPU(CPUZ380))
  {
    Byte AdrByte = 0xff;

    NLS_UpString(ArgStr[1]);
    if (!strcmp(ArgStr[1], "LW")) AdrByte = 1;
    else if (!strcmp(ArgStr[1], "LCK")) AdrByte = 2;
    else if (!strcmp(ArgStr[1], "XM")) AdrByte = 3;
    else WrError(1440);
    if (AdrByte != 0xff)
    {
      CodeLen = 2;
      BAsmCode[0] = 0xcd + (AdrByte << 4);
      BAsmCode[1] = 0xf7 + Code;
    }
  }
}

static void DecodeDDIR(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 2)
   && ChkMinCPU(CPUZ380))
  {
    Boolean OK;
    int z;

    OK = True;
    for (z = 1; z <= ArgCnt; z++)
    {
      if (OK)
      {
        NLS_UpString(ArgStr[z]);
        OK = ExtendPrefix(&CurrPrefix, ArgStr[z]);
        if (!OK) WrError(1135);
      }
    }
    if (OK)
    {
      GetPrefixCode(CurrPrefix, BAsmCode + 0, BAsmCode + 1);
      CodeLen = 2;
    }
  }
}

static void DecodePORT(Word Code)
{
  UNUSED(Code);

  CodeEquate(SegIO, 0, PortEnd());
}

static void ModIntel(Word Code)
{
  UNUSED(Code);

  /* M80 compatibility: DEFB->DB, DEFW->DW */

  strmov(OpPart.Str + 1, OpPart.Str + 3);
  DecodeIntelPseudo(False);
}

/*==========================================================================*/
/* Codetabellenerzeugung */

static void AddFixed(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode)
{
  if (InstrZ >= FixedOrderCnt) exit(255);
  FixedOrders[InstrZ].MinCPU = NewMin;
  FixedOrders[InstrZ].Len = NewLen;
  FixedOrders[InstrZ].Code = NewCode;
  AddInstTable(InstTable, NewName, InstrZ++, DecodeFixed);
}

static void AddAcc(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode)
{
  if (InstrZ >= AccOrderCnt) exit(255);
  AccOrders[InstrZ].MinCPU = NewMin;
  AccOrders[InstrZ].Len = NewLen;
  AccOrders[InstrZ].Code = NewCode;
  AddInstTable(InstTable, NewName, InstrZ++, DecodeAcc);
}

static void AddHL(char *NewName, CPUVar NewMin, Byte NewLen, Word NewCode)
{
  if (InstrZ >= HLOrderCnt) exit(255);
  HLOrders[InstrZ].MinCPU = NewMin;
  HLOrders[InstrZ].Len = NewLen;
  HLOrders[InstrZ].Code = NewCode;
  AddInstTable(InstTable, NewName, InstrZ++, DecodeHL);
}

static void AddALU(char *Name8, char *Name16, Byte Code)
{
  AddInstTable(InstTable, Name8 , Code, DecodeALU8);
  AddInstTable(InstTable, Name16, Code, DecodeALU16);
}

static void AddShift(char *Name8, char *Name16, Byte Code)
{
  AddInstTable(InstTable, Name8 , Code, DecodeShift8);
  if (Name16)
    AddInstTable(InstTable, Name16, Code, DecodeShift16);
}

static void AddBit(char *NName, Word Code)
{
  AddInstTable(InstTable, NName, Code, DecodeBit);
}

static void AddCondition(char *NewName, Byte NewCode)
{
  if (InstrZ >= ConditionCnt) exit(255);
  Conditions[InstrZ].Name = NewName;
  Conditions[InstrZ++].Code = NewCode;
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);

  AddInstTable(InstTable, "LD" , 0, DecodeLD);
  AddInstTable(InstTable, "LDW", 1, DecodeLD);
  AddInstTable(InstTable, "ADD", 0, DecodeADD);
  AddInstTable(InstTable, "ADDW", 0, DecodeADDW);
  AddInstTable(InstTable, "ADC" , 0, DecodeADC_SBC);
  AddInstTable(InstTable, "SBC" , 1, DecodeADC_SBC);
  AddInstTable(InstTable, "ADCW", 0, DecodeADCW_SBCW);
  AddInstTable(InstTable, "SBCW",16, DecodeADCW_SBCW);
  AddInstTable(InstTable, "INC" , 0, DecodeINC_DEC);
  AddInstTable(InstTable, "DEC" , 1, DecodeINC_DEC);
  AddInstTable(InstTable, "INCW", 2, DecodeINC_DEC);
  AddInstTable(InstTable, "DECW", 3, DecodeINC_DEC);
  AddInstTable(InstTable, "MLT" , 0, DecodeMLT);
  AddInstTable(InstTable, "DIVUW" , 0x28, DecodeMULT_DIV);
  AddInstTable(InstTable, "MULTW" , 0x00, DecodeMULT_DIV);
  AddInstTable(InstTable, "MULTUW", 0x08, DecodeMULT_DIV);
  AddInstTable(InstTable, "TST", 0, DecodeTST);
  AddInstTable(InstTable, "SWAP", 0, DecodeSWAP);
  AddInstTable(InstTable, "PUSH", 4, DecodePUSH_POP);
  AddInstTable(InstTable, "POP" , 0, DecodePUSH_POP);
  AddInstTable(InstTable, "EX"  , 0, DecodeEX);
  AddInstTable(InstTable, "TSTI", 0, DecodeTSTI);
  AddInstTable(InstTable, "IN"  , 0, DecodeIN_OUT);
  AddInstTable(InstTable, "OUT" , 1, DecodeIN_OUT);
  AddInstTable(InstTable, "INW"  , 0, DecodeINW_OUTW);
  AddInstTable(InstTable, "OUTW" , 1, DecodeINW_OUTW);
  AddInstTable(InstTable, "IN0"  , 0, DecodeIN0_OUT0);
  AddInstTable(InstTable, "OUT0" , 1, DecodeIN0_OUT0);
  AddInstTable(InstTable, "INA"  , 8, DecodeINA_INAW_OUTA_OUTAW);
  AddInstTable(InstTable, "INAW" , 9, DecodeINA_INAW_OUTA_OUTAW);
  AddInstTable(InstTable, "OUTA" , 0, DecodeINA_INAW_OUTA_OUTAW);
  AddInstTable(InstTable, "OUTAW", 1, DecodeINA_INAW_OUTA_OUTAW);
  AddInstTable(InstTable, "TSTIO", 0, DecodeTSTIO);
  AddInstTable(InstTable, "RET" , 0, DecodeRET);
  AddInstTable(InstTable, "JP" , 0, DecodeJP);
  AddInstTable(InstTable, "CALL", 0, DecodeCALL);
  AddInstTable(InstTable, "JR" , 0, DecodeJR);
  AddInstTable(InstTable, "CALR", 0, DecodeCALR);
  AddInstTable(InstTable, "DJNZ", 0, DecodeDJNZ);
  AddInstTable(InstTable, "RST", 0, DecodeRST);
  AddInstTable(InstTable, "DI", 0, DecodeEI_DI);
  AddInstTable(InstTable, "EI", 8, DecodeEI_DI);
  AddInstTable(InstTable, "IM", 0, DecodeIM);
  AddInstTable(InstTable, "LDCTL", 0, DecodeLDCTL);
  AddInstTable(InstTable, "RESC", 8, DecodeRESC_SETC);
  AddInstTable(InstTable, "SETC", 0, DecodeRESC_SETC);
  AddInstTable(InstTable, "DDIR", 0, DecodeDDIR);
  AddInstTable(InstTable, "PORT", 0, DecodePORT);
  AddInstTable(InstTable, "DEFB", 0, ModIntel);
  AddInstTable(InstTable, "DEFW", 0, ModIntel);

  InstrZ = 0; Conditions = (Condition *) malloc(sizeof(Condition) * ConditionCnt);
  AddCondition("NZ", 0); AddCondition("Z" , 1);
  AddCondition("NC", 2); AddCondition("C" , 3);
  AddCondition("PO", 4); AddCondition("NV", 4);
  AddCondition("PE", 5); AddCondition("V" , 5);
  AddCondition("P" , 6); AddCondition("NS", 6);
  AddCondition("M" , 7); AddCondition("S" , 7);

  InstrZ = 0; FixedOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * FixedOrderCnt);
  AddFixed("EXX"  ,CPUZ80 , 1, 0x00d9); AddFixed("LDI"  , CPUZ80 , 2, 0xeda0);
  AddFixed("LDIR" ,CPUZ80 , 2, 0xedb0); AddFixed("LDD"  , CPUZ80 , 2, 0xeda8);
  AddFixed("LDDR" ,CPUZ80 , 2, 0xedb8); AddFixed("CPI"  , CPUZ80 , 2, 0xeda1);
  AddFixed("CPIR" ,CPUZ80 , 2, 0xedb1); AddFixed("CPD"  , CPUZ80 , 2, 0xeda9);
  AddFixed("CPDR" ,CPUZ80 , 2, 0xedb9); AddFixed("RLCA" , CPUZ80 , 1, 0x0007);
  AddFixed("RRCA" ,CPUZ80 , 1, 0x000f); AddFixed("RLA"  , CPUZ80 , 1, 0x0017);
  AddFixed("RRA"  ,CPUZ80 , 1, 0x001f); AddFixed("RLD"  , CPUZ80 , 2, 0xed6f);
  AddFixed("RRD"  ,CPUZ80 , 2, 0xed67); AddFixed("DAA"  , CPUZ80 , 1, 0x0027);
  AddFixed("CCF"  ,CPUZ80 , 1, 0x003f); AddFixed("SCF"  , CPUZ80 , 1, 0x0037);
  AddFixed("NOP"  ,CPUZ80 , 1, 0x0000); AddFixed("HALT" , CPUZ80 , 1, 0x0076);
  AddFixed("RETI" ,CPUZ80 , 2, 0xed4d); AddFixed("RETN" , CPUZ80 , 2, 0xed45);
  AddFixed("INI"  ,CPUZ80 , 2, 0xeda2); AddFixed("INIR" , CPUZ80 , 2, 0xedb2);
  AddFixed("IND"  ,CPUZ80 , 2, 0xedaa); AddFixed("INDR" , CPUZ80 , 2, 0xedba);
  AddFixed("OUTI" ,CPUZ80 , 2, 0xeda3); AddFixed("OTIR" , CPUZ80 , 2, 0xedb3);
  AddFixed("OUTD" ,CPUZ80 , 2, 0xedab); AddFixed("OTDR" , CPUZ80 , 2, 0xedbb);
  AddFixed("SLP"  ,CPUZ180, 2, 0xed76); AddFixed("OTIM" , CPUZ180, 2, 0xed83);
  AddFixed("OTIMR",CPUZ180, 2, 0xed93); AddFixed("OTDM" , CPUZ180, 2, 0xed8b);
  AddFixed("OTDMR",CPUZ180, 2, 0xed9b); AddFixed("BTEST", CPUZ380, 2, 0xedcf);
  AddFixed("EXALL",CPUZ380, 2, 0xedd9); AddFixed("EXXX" , CPUZ380, 2, 0xddd9);
  AddFixed("EXXY" ,CPUZ380, 2, 0xfdd9); AddFixed("INDW" , CPUZ380, 2, 0xedea);
  AddFixed("INDRW",CPUZ380, 2, 0xedfa); AddFixed("INIW" , CPUZ380, 2, 0xede2);
  AddFixed("INIRW",CPUZ380, 2, 0xedf2); AddFixed("LDDW" , CPUZ380, 2, 0xede8);
  AddFixed("LDDRW",CPUZ380, 2, 0xedf8); AddFixed("LDIW" , CPUZ380, 2, 0xede0);
  AddFixed("LDIRW",CPUZ380, 2, 0xedf0); AddFixed("MTEST", CPUZ380, 2, 0xddcf);
  AddFixed("OTDRW",CPUZ380, 2, 0xedfb); AddFixed("OTIRW", CPUZ380, 2, 0xedf3);
  AddFixed("OUTDW",CPUZ380, 2, 0xedeb); AddFixed("OUTIW", CPUZ380, 2, 0xede3);
  AddFixed("RETB" ,CPUZ380, 2, 0xed55);

  InstrZ = 0; AccOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * AccOrderCnt);
  AddAcc("CPL"  , CPUZ80 , 1, 0x002f); AddAcc("NEG"  , CPUZ80 , 2, 0xed44);
  AddAcc("EXTS" , CPUZ380, 2, 0xed65);

  InstrZ = 0; HLOrders = (BaseOrder *) malloc(sizeof(BaseOrder) * HLOrderCnt);
  AddHL("CPLW" , CPUZ380, 2, 0xdd2f);  AddHL("NEGW" , CPUZ380, 2, 0xed54);
  AddHL("EXTSW", CPUZ380, 2, 0xed75);
 
  AddALU("SUB", "SUBW", 2); AddALU("AND", "ANDW", 4);
  AddALU("OR" , "ORW" , 6); AddALU("XOR", "XORW", 5);
  AddALU("CP" , "CPW" , 7);

  AddShift("RLC" , "RLCW" , 0); AddShift("RRC", "RRCW", 1);
  AddShift("RL"  , "RLW"  , 2); AddShift("RR" , "RRW" , 3); 
  AddShift("SLA" , "SLAW" , 4); AddShift("SRA", "SRAW", 5);
  AddShift("SLIA", NULL   , 6); AddShift("SRL", "SRLW", 7);
  AddShift("SLS" , NULL   , 6);

  AddBit("BIT", 0); AddBit("RES", 1); AddBit("SET", 2);
}

static void DeinitFields(void)
{
  free(Conditions);
  free(FixedOrders);
  free(AccOrders);
  free(HLOrders);

  DestroyInstTable(InstTable);
}

/*=========================================================================*/

static void StripPref(const char *Arg, Byte Opcode)
{
  char *ptr, *ptr2;
  int z;

  /* do we have a prefix ? */

  if (!strcmp(OpPart.Str, Arg))
  {
    /* add to code */

    BAsmCode[PrefixCnt++] = Opcode;
    *OpPart.Str = '\0';

    /* cut true opcode out of next argument */

    if (ArgCnt)
    {
      /* look for end of string */

      for (ptr = ArgStr[1]; *ptr; ptr++)
        if (myisspace(*ptr))
          break;

      /* look for beginning of next string */

      for (ptr2 = ptr; *ptr2; ptr2++)
        if (!myisspace(*ptr2))
          break;

      /* copy out new opcode */
 
      *ptr = '\0';
      strcpy(OpPart.Str, ArgStr[1]);
      NLS_UpString(OpPart.Str);

      /* cut down arg or eliminate it completely */

      if (*ptr2)
        strcpy(ArgStr[1], ptr2);
      else
      {
        for (z = 1; z < ArgCnt; z++)
          strcpy(ArgStr[z], ArgStr[z + 1]);
        ArgCnt--;
      }
    }

    /* if no further argument, that's all folks */

    else
      CodeLen = PrefixCnt;
  }
}

static void MakeCode_Z80(void)
{
  CodeLen = 0;
  DontPrint = False;
  PrefixCnt = 0;
  OpSize = 0xff;
  MayLW = False;

/*--------------------------------------------------------------------------*/
/* Rabbit 2000 prefixes */

  if (MomCPU == CPUR2000)
  {
    StripPref("ALTD", 0x76);
  }

  /* zu ignorierendes */

  if (Memo("")) return;

  /* letzten Praefix umkopieren */

  LastPrefix = CurrPrefix;
  CurrPrefix = Pref_IN_N;

  /* evtl. Datenablage */

  if (DecodeIntelPseudo(False)) return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
}

static void InitCode_Z80(void)
{
  SetFlag(&ExtFlag, ExtFlagName, False);
  SetFlag(&LWordFlag, LWordFlagName, False);
}

static Boolean IsDef_Z80(void)
{
  return Memo("PORT");
}

static void SwitchFrom_Z80(void)
{
  DeinitFields();
  ClearONOFF();
}

static void SwitchTo_Z80(void)
{
  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = True;

  PCSymbol = "$"; HeaderID = 0x51; NOPCode = 0x00;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode) + (1 << SegIO);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = CodeEnd();
  Grans[SegIO  ] = 1; ListGrans[SegIO  ] = 1; SegInits[SegIO  ] = 0;
  SegLimits[SegIO  ] = PortEnd();

  MakeCode = MakeCode_Z80; IsDef = IsDef_Z80;
  SwitchFrom = SwitchFrom_Z80; InitFields();

  /* erweiterte Modi nur bei Z380 */

  if (MomCPU>=CPUZ380)
  {
    AddONOFF("EXTMODE",   &ExtFlag   , ExtFlagName   , False);
    AddONOFF("LWORDMODE", &LWordFlag , LWordFlagName , False);
  }
  SetFlag(&ExtFlag, ExtFlagName, False);
  SetFlag(&LWordFlag, LWordFlagName, False);
}

void codez80_init(void)
{
  CPUZ80   = AddCPU("Z80"       , SwitchTo_Z80);
  CPUZ80U  = AddCPU("Z80UNDOC"  , SwitchTo_Z80);
  CPUZ180  = AddCPU("Z180"      , SwitchTo_Z80);
  CPUR2000 = AddCPU("RABBIT2000", SwitchTo_Z80);
  CPUZ380  = AddCPU("Z380"      , SwitchTo_Z80);

  AddInitPassProc(InitCode_Z80);
}
