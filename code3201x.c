/* code3201x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS3201x-Familie                                            */
/*                                                                           */
/* Historie: 28.11.1996 Grundsteinlegung                                     */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1992 BookKeeping-Aufruf in RES                            */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code3201x.c,v 1.5 2014/12/07 19:13:59 alfred Exp $                          */
/*****************************************************************************
 * $Log: code3201x.c,v $
 * Revision 1.5  2014/12/07 19:13:59  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.4  2014/10/06 18:53:05  alfred
 * - rework to current style
 *
 * Revision 1.3  2008/11/23 10:39:16  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.2  2005/09/08 17:31:03  alfred
 * - add missing include
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.2  2002/08/14 18:43:48  alfred
 * - warn null allocation, remove some warnings
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct
{
  Word Code;
  Word AllowShifts;
} AdrShiftOrder;

typedef struct
{
  Word Code;
  Integer Min, Max;
  Word Mask;
} ImmOrder;


#define AdrShiftOrderCnt 5
#define ImmOrderCnt 3


static Word AdrMode;
static Boolean AdrOK;

static CPUVar CPU32010, CPU32015;

static AdrShiftOrder *AdrShiftOrders;
static ImmOrder *ImmOrders;

/*----------------------------------------------------------------------------*/

static Word EvalARExpression(char *Asc, Boolean *OK)
{
  *OK = True;
  if (!strcasecmp(Asc, "AR0"))
    return 0;
  if (!strcasecmp(Asc, "AR1"))
    return 1;
  return EvalIntExpression(Asc, UInt1, OK);
}

static void DecodeAdr(char *Arg, int Aux, Boolean Must1)
{
  Byte h;
  char *p;

  AdrOK = False;

  if ((!strcmp(Arg, "*")) || (!strcmp(Arg, "*-")) || (!strcmp(Arg, "*+")))
  {
    AdrMode = 0x88;
    if (strlen(Arg) == 2)
      AdrMode += (Arg[1] == '+') ? 0x20 : 0x10;
    if (Aux <= ArgCnt)
    {
      h = EvalARExpression(ArgStr[Aux], &AdrOK);
      if (AdrOK)
      {
        AdrMode &= 0xf7;
        AdrMode += h;
      }
    }
    else
      AdrOK = True;
  }
  else if (Aux <= ArgCnt) WrError(1110);
  else
  {
    h = 0;
    if ((strlen(Arg) > 3) && (!strncasecmp(Arg, "DAT", 3)))
    {
      AdrOK = True;
      for (p = Arg + 3; *p != '\0'; p++)
        if ((*p > '9') || (*p < '0'))
          AdrOK = False;
      if (AdrOK)
        h = EvalIntExpression(Arg + 3, UInt8, &AdrOK);
    }
    if (!AdrOK)
      h = EvalIntExpression(Arg, Int8, &AdrOK);
    if (AdrOK)
    {
      if ((Must1) && (h < 0x80) && (!FirstPassUnknown))
      {
        WrError(1315);
        AdrOK = False;
      }
      else
      {
        AdrMode = h & 0x7f;
        ChkSpace(SegData);
      }
    }
  }
}

/*----------------------------------------------------------------------------*/

/* kein Argument */

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    CodeLen = 1;
    WAsmCode[0] = Code;
  }
}

/* Spruenge */

static void DecodeJmp(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;

    WAsmCode[1] = EvalIntExpression(ArgStr[1], UInt12, &OK);
    if (OK)
    {
      CodeLen = 2;
      WAsmCode[0] = Code;
    }
  }
}

/* nur Adresse */

static void DecodeAdrInst(Word Code)
{
  if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], 2, Code & 1);
    if (AdrOK)
    {
      CodeLen = 1;
      WAsmCode[0] = (Code & 0xfffe) + AdrMode;
    }
  }
}

/* Adresse & schieben */

static void DecodeAdrShift(Word Index)
{
  Boolean HasSh;
  int Cnt;
  const AdrShiftOrder *pOrder = AdrShiftOrders + Index;

  if ((ArgCnt < 1) || (ArgCnt > 3)) WrError(1110);
  else
  {
    if (*ArgStr[1] == '*')
    {
      if (ArgCnt == 2)
      {
        if (!strncasecmp(ArgStr[2], "AR", 2))
        {
          HasSh = False;
          Cnt = 2;
        }
        else
        {
          HasSh = True;
          Cnt = 3;
        }
      }
      else 
      {
        HasSh = True;
        Cnt = 3;
      }
    }
    else 
    {
      Cnt = 3;
      HasSh = (ArgCnt == 2);
    }
    DecodeAdr(ArgStr[1], Cnt, False);
    if (AdrOK)
    {
      Boolean OK;
      Word AdrWord;

      if (!HasSh)
      {
        OK = True;
        AdrWord = 0;
      }
      else
      {
        AdrWord = EvalIntExpression(ArgStr[2], Int4, &OK);
        if ((OK) && (FirstPassUnknown))
          AdrWord = 0;
      }
      if (OK)
      {
        if ((pOrder->AllowShifts & (1 << AdrWord)) == 0) WrError(1380);
        else
        {
          CodeLen = 1;
          WAsmCode[0] = pOrder->Code + AdrMode + (AdrWord << 8);
        }
      }
    }
  }
}

/* Ein/Ausgabe */

static void DecodeIN_OUT(Word Code)
{
  if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], 3, False);
    if (AdrOK)
    {
      Boolean OK;
      Word AdrWord = EvalIntExpression(ArgStr[2], UInt3, &OK);
      if (OK)
      {
        ChkSpace(SegIO);
        CodeLen = 1;
        WAsmCode[0] = Code + AdrMode + (AdrWord << 8);
      }
    }
  }
}

/* konstantes Argument */

static void DecodeImm(Word Index)
{
  const ImmOrder *pOrder = ImmOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    LongInt AdrLong = EvalIntExpression(ArgStr[1], Int32, &OK);
    if (OK)
    {
      if (FirstPassUnknown)
        AdrLong &= pOrder->Mask;
      if (AdrLong < pOrder->Min) WrError(1315);
      else if (AdrLong > pOrder->Max) WrError(1320);
      else
      {
        CodeLen = 1;
        WAsmCode[0] = pOrder->Code + (AdrLong & pOrder->Mask);
      }
    }
  }
}

/* mit Hilfsregistern */

static void DecodeLARP(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord = EvalARExpression(ArgStr[1], &OK);
    if (OK)
    {
      CodeLen = 1;
      WAsmCode[0] = 0x6880 + AdrWord;
    }
  }
}

static void DecodeLAR_SAR(Word Code)
{
  if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord = EvalARExpression(ArgStr[1], &OK);
    if (OK)
    {
      DecodeAdr(ArgStr[2], 3, False);
      if (AdrOK)
      {
        CodeLen = 1;
        WAsmCode[0] = Code + AdrMode + (AdrWord << 8);
      }
    }
  }
}

static void DecodeLARK(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord = EvalARExpression(ArgStr[1], &OK);
    if (OK)
    {
      WAsmCode[0] = EvalIntExpression(ArgStr[2], Int8, &OK);
      if (OK)
      {
        CodeLen = 1;
        WAsmCode[0] = Lo(WAsmCode[0]) + 0x7000 + (AdrWord << 8);
      }
    }
  }
}

static void DecodePORT(Word Code)
{
  UNUSED(Code);

  CodeEquate(SegIO, 0, 7);
}

static void DecodeRES(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word Size;

    FirstPassUnknown = False;
    Size = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (FirstPassUnknown)
      WrError(1820);
    if ((OK) && (!FirstPassUnknown))
    {
      DontPrint = True;
      if (!Size)
        WrError(290);
      CodeLen = Size;
      BookKeeping();
    }
  }
}

static void DecodeDATA(Word Code)
{
  int z;
  TempResult t;
  Boolean OK;

  UNUSED(Code);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    OK = True;
    for (z = 1; z <= ArgCnt; z++)
     if (OK)
     {
       EvalExpression(ArgStr[z], &t);
       switch (t.Typ)
       {
         case TempInt:
           if ((t.Contents.Int < -32768) || (t.Contents.Int > 0xffff))
           {
             WrError(1320);
             OK = False;
           }
           else
             WAsmCode[CodeLen++] = t.Contents.Int;
           break;
         case TempFloat:
           WrError(1135);
           OK = False;
           break;
         case TempString:
         {
           Word Trans;
           unsigned z2;

           for (z2 = 0; z2 < t.Contents.Ascii.Length; z2++)
           {
             Trans = CharTransTable[((usint)t.Contents.Ascii.Contents[z2]) & 0xff];
             if (z2 & 1)
               WAsmCode[CodeLen++] |= Trans << 8;
             else
               WAsmCode[CodeLen] = Trans;
           }
           if (z2 & 1)
             CodeLen++;
           break;
         }
         default:
           OK = False;
       }
     }
    if (!OK)
      CodeLen = 0;
  }
}

/*----------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddJmp(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeJmp);
}

static void AddAdr(char *NName, Word NCode, Word NMust1)
{
  AddInstTable(InstTable, NName, NCode | NMust1, DecodeAdrInst);
}

static void AddAdrShift(char *NName, Word NCode, Word NAllow)
{
  if (InstrZ >= AdrShiftOrderCnt) exit(255);
  AdrShiftOrders[InstrZ].Code = NCode;
  AdrShiftOrders[InstrZ].AllowShifts = NAllow;
  AddInstTable(InstTable, NName, InstrZ++, DecodeAdrShift);
}

static void AddImm(char *NName, Word NCode, Integer NMin, Integer NMax, Word NMask)
{
  if (InstrZ >= ImmOrderCnt) exit(255);
  ImmOrders[InstrZ].Code = NCode;
  ImmOrders[InstrZ].Min = NMin;
  ImmOrders[InstrZ].Max = NMax;
  ImmOrders[InstrZ].Mask = NMask;
  AddInstTable(InstTable, NName, InstrZ++, DecodeImm);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);
  AddInstTable(InstTable, "IN", 0x4000, DecodeIN_OUT);
  AddInstTable(InstTable, "OUT", 0x4800, DecodeIN_OUT);
  AddInstTable(InstTable, "LARP", 0, DecodeLARP);
  AddInstTable(InstTable, "LAR", 0x3800, DecodeLAR_SAR);
  AddInstTable(InstTable, "SAR", 0x3000, DecodeLAR_SAR);
  AddInstTable(InstTable, "LARK", 0, DecodeLARK);
  AddInstTable(InstTable, "PORT", 0, DecodePORT);
  AddInstTable(InstTable, "RES", 0, DecodeRES);
  AddInstTable(InstTable, "DATA", 0, DecodeDATA);

  AddFixed("ABS"   , 0x7f88);  AddFixed("APAC"  , 0x7f8f);
  AddFixed("CALA"  , 0x7f8c);  AddFixed("DINT"  , 0x7f81);
  AddFixed("EINT"  , 0x7f82);  AddFixed("NOP"   , 0x7f80);
  AddFixed("PAC"   , 0x7f8e);  AddFixed("POP"   , 0x7f9d);
  AddFixed("PUSH"  , 0x7f9c);  AddFixed("RET"   , 0x7f8d);
  AddFixed("ROVM"  , 0x7f8a);  AddFixed("SOVM"  , 0x7f8b);
  AddFixed("SPAC"  , 0x7f90);  AddFixed("ZAC"   , 0x7f89);

  AddJmp("B"     , 0xf900);  AddJmp("BANZ"  , 0xf400);
  AddJmp("BGEZ"  , 0xfd00);  AddJmp("BGZ"   , 0xfc00);
  AddJmp("BIOZ"  , 0xf600);  AddJmp("BLEZ"  , 0xfb00);
  AddJmp("BLZ"   , 0xfa00);  AddJmp("BNZ"   , 0xfe00);
  AddJmp("BV"    , 0xf500);  AddJmp("BZ"    , 0xff00);
  AddJmp("CALL"  , 0xf800);

  AddAdr("ADDH"  , 0x6000, False);  AddAdr("ADDS"  , 0x6100, False);
  AddAdr("AND"   , 0x7900, False);  AddAdr("DMOV"  , 0x6900, False);
  AddAdr("LDP"   , 0x6f00, False);  AddAdr("LST"   , 0x7b00, False);
  AddAdr("LT"    , 0x6a00, False);  AddAdr("LTA"   , 0x6c00, False);
  AddAdr("LTD"   , 0x6b00, False);  AddAdr("MAR"   , 0x6800, False);
  AddAdr("MPY"   , 0x6d00, False);  AddAdr("OR"    , 0x7a00, False);
  AddAdr("SST"   , 0x7c00, True );  AddAdr("SUBC"  , 0x6400, False);
  AddAdr("SUBH"  , 0x6200, False);  AddAdr("SUBS"  , 0x6300, False);
  AddAdr("TBLR"  , 0x6700, False);  AddAdr("TBLW"  , 0x7d00, False);
  AddAdr("XOR"   , 0x7800, False);  AddAdr("ZALH"  , 0x6500, False);
  AddAdr("ZALS"  , 0x6600, False);

  AdrShiftOrders = (AdrShiftOrder *) malloc(sizeof(AdrShiftOrder) * AdrShiftOrderCnt); InstrZ = 0;
  AddAdrShift("ADD"   , 0x0000, 0xffff);
  AddAdrShift("LAC"   , 0x2000, 0xffff);
  AddAdrShift("SACH"  , 0x5800, 0x0013);
  AddAdrShift("SACL"  , 0x5000, 0x0001);
  AddAdrShift("SUB"   , 0x1000, 0xffff);

  ImmOrders = (ImmOrder *) malloc(sizeof(ImmOrder)*ImmOrderCnt); InstrZ = 0;
  AddImm("LACK", 0x7e00,     0,  255,   0xff);
  AddImm("LDPK", 0x6e00,     0,    1,    0x1);
  AddImm("MPYK", 0x8000, -4096, 4095, 0x1fff);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable); 

  free(AdrShiftOrders);
  free(ImmOrders);
}

/*----------------------------------------------------------------------------*/

static void MakeCode_3201X(void)
{
  CodeLen = 0;
  DontPrint = False;

  /* zu ignorierendes */

  if (Memo(""))
    return;

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static Boolean IsDef_3201X(void)
{
  return (Memo("PORT"));
}

static void SwitchFrom_3201X(void)
{
  DeinitFields();
}

static void SwitchTo_3201X(void)
{
  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = False;

  PCSymbol = "$";
  HeaderID = 0x74;
  NOPCode = 0x7f80;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode)|(1 << SegData)|(1 << SegIO);
  Grans[SegCode] = 2; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xfff;
  Grans[SegData] = 2; ListGrans[SegData] = 2; SegInits[SegData] = 0;
  SegLimits[SegData] = (MomCPU == CPU32010) ? 0x8f : 0xff;
  Grans[SegIO  ] = 2; ListGrans[SegIO  ] = 2; SegInits[SegIO  ] = 0;
  SegLimits[SegIO  ] = 7;

  MakeCode = MakeCode_3201X;
  IsDef = IsDef_3201X;
  SwitchFrom = SwitchFrom_3201X;
  InitFields();
}

void code3201x_init(void)
{
  CPU32010 = AddCPU("32010", SwitchTo_3201X);
  CPU32015 = AddCPU("32015", SwitchTo_3201X);
}
