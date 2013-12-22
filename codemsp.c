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
/* $Id: codemsp.c,v 1.11 2013/12/21 19:46:51 alfred Exp $                     */
/***************************************************************************** 
 * $Log: codemsp.c,v $
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

/*  float exp (8bit bias 128) sign mant (impl. norm.)
   double exp (8bit bias 128) sign mant (impl. norm.) */

static CPUVar CPUMSP430;

static OneOpOrder *OneOpOrders;

static Word AdrMode, AdrPart;
static Word AdrVal;
static Byte OpSize;
static Word PCDist;

/*-------------------------------------------------------------------------*/

static void ResetAdr(void)
{
  AdrMode = 0xff; AdrCnt = 0;
}

static void ChkAdr(Byte Mask)
{
  if ((AdrMode != 0xff) && ((Mask & (1 << AdrMode)) == 0))
  {
    ResetAdr(); WrError(1350);
  }
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
  else if (!strcasecmp(Asc,"SR"))
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

static void DecodeAdr(char *Asc, Byte Mask, Boolean MayImm)
{
  Word AdrWord;
  Boolean OK;
  char *p;

  ResetAdr();

  /* immediate */

  if (*Asc == '#')
  {
    if (!MayImm) WrError(1350);
    else
    {
      AdrWord = EvalIntExpression(Asc + 1, (OpSize == 1) ? Int8 : Int16, &OK);
      if (OK)
      {
        switch (AdrWord)
        {
          case 0:
            AdrPart = 3; AdrMode = 0;
            break;
          case 1:
            AdrPart = 3; AdrMode = 1;
            break;
          case 2:
            AdrPart = 3; AdrMode = 2;
            break;
          case 4:
            AdrPart = 2; AdrMode = 2;
            break;
          case 8:
            AdrPart = 2; AdrMode = 3;
            break;
          case 0xffff:
            AdrPart = 3; AdrMode = 3;
            break;
          default:
            AdrVal = AdrWord; AdrCnt = 1;
            AdrPart = 0; AdrMode = 3;
            break;
        }
      }
    }
    ChkAdr(Mask); return;
  }

  /* absolut */

  if (*Asc == '&')
  {
    AdrVal = EvalIntExpression(Asc + 1, UInt16, &OK);
    if (OK)
    {
      AdrMode = 1; AdrPart = 2; AdrCnt = 1;
    }
    ChkAdr(Mask); return;
  }

  /* Register */

  if (DecodeReg(Asc, &AdrPart))
  {
    if (AdrPart == 3) WrXError(1445,Asc);
    else AdrMode = 0;
    ChkAdr(Mask); return;
  }

  /* Displacement */

  if ((*Asc) && (Asc[strlen(Asc)-1] == ')'))
  {
    Asc[strlen(Asc) - 1] = '\0';
    p = RQuotPos(Asc, '(');
    if (p)
    {
      if (DecodeReg(p + 1, &AdrPart))
      {
        *p = '\0';
        AdrVal = EvalIntExpression(Asc, Int16, &OK);
        if (OK)
        {
          if ((AdrPart == 2) || (AdrPart == 3)) WrXError(1445, p + 1);
          else if ((AdrVal == 0) && ((Mask & 4) != 0)) AdrMode = 2;
          else
          {
            AdrCnt = 1; AdrMode = 1;
          }
        }
        *p = '(';
        ChkAdr(Mask); return;
      }
    }
    Asc[strlen(Asc)] = ')';
  }

  /* indirekt mit/ohne Autoinkrement */

  if ((*Asc == '@') || (*Asc=='*'))
  {
    if (Asc[strlen(Asc) - 1] == '+')
    {
      AdrWord = 1; Asc[strlen(Asc) - 1] = '\0';
    }
    else AdrWord = 0;
    if (!DecodeReg(Asc + 1, &AdrPart)) WrXError(1445,Asc);
    else if ((AdrPart == 2) || (AdrPart == 3)) WrXError(1445,Asc);
    else if ((AdrWord == 0) && ((Mask & 4) == 0))
    {
      AdrVal = 0; AdrCnt = 1; AdrMode = 1;
    }
    else AdrMode = 2 + AdrWord;
    ChkAdr(Mask); return;
  }

  /* bleibt PC-relativ */

  AdrWord = EvalIntExpression(Asc, UInt16, &OK) - EProgCounter() - PCDist;
  if (OK)
  {
    AdrPart = 0; AdrMode = 1; AdrCnt = 1; AdrVal = AdrWord;
  }

  ChkAdr(Mask);
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

static void DecodeFixed(Word Code)
{
  if (ArgCnt!=0) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (OpSize != 0) WrError(1130);
  else
  {
    if (Odd(EProgCounter())) WrError(180);
    WAsmCode[0] = Code; CodeLen = 2;
  }
}

static void DecodeTwoOp(Word Code)
{
  Word AdrMode2, AdrPart2;
  Byte AdrCnt2;
  Word AdrVal2;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    PCDist = 2; DecodeAdr(ArgStr[1], 15, True);
    if (AdrMode != 0xff)
    {
      AdrMode2 = AdrMode; AdrPart2 = AdrPart; AdrCnt2 = AdrCnt; AdrVal2 = AdrVal;
      PCDist += AdrCnt2 << 1; DecodeAdr(ArgStr[2], 3, False);
      if (AdrMode != 0xff)
      {
        if (Odd(EProgCounter())) WrError(180);
        WAsmCode[0] = Code | (AdrPart2 << 8) | (AdrMode << 7)
                    | (OpSize << 6) | (AdrMode2 << 4) | AdrPart;
        memcpy(WAsmCode + 1, &AdrVal2, AdrCnt2 << 1);
        memcpy(WAsmCode + 1 + AdrCnt2, &AdrVal, AdrCnt << 1);
        CodeLen = (1 + AdrCnt + AdrCnt2) << 1;
      }
    }
  }
}

static void DecodeOneOp(Word Index)
{
  const OneOpOrder *pOrder = OneOpOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else if ((OpSize == 1) && (!pOrder->MayByte)) WrError(1130);
  else
  {
    PCDist = 2; DecodeAdr(ArgStr[1], 15, True);
    if (AdrMode != 0xff)
    {
      if (Odd(EProgCounter())) WrError(180);
      WAsmCode[0] = pOrder->Code | (OpSize << 6) | (AdrMode << 4) | AdrPart;
      memcpy(WAsmCode + 1, &AdrVal, AdrCnt << 1);
      CodeLen = (1 + AdrCnt) << 1;
    }
  }
}

static void DecodeJmp(Word Code)
{
  Integer AdrInt; 
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else if (OpSize != 0) WrError(1130);
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
    while ((z<=ArgCnt) AND (OK));
    if (!OK) CodeLen = 0;
    else if ((Odd(CodeLen)) && (DoPadding)) PutByte(0);
  }
}

static void DecodeWORD(Word Index)
{
  int z;
  Word HVal16;
  Boolean OK;

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

/*-------------------------------------------------------------------------*/

#define AddFixed(NName, NCode) \
        AddInstTable(InstTable, NName, NCode, DecodeFixed)

#define AddTwoOp(NName, NCode) \
        AddInstTable(InstTable, NName, NCode, DecodeTwoOp)

static void AddOneOp(char *NName, Boolean NMay, Word NCode)
{
  if (InstrZ >= OneOpCount) exit(255);
  OneOpOrders[InstrZ].MayByte = NMay;
  OneOpOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeOneOp);
}

#define AddJmp(NName, NCode) \
        AddInstTable(InstTable, NName, NCode, DecodeJmp)

static void InitFields(void)
{
  InstTable = CreateInstTable(103);

  AddFixed("RETI", 0x1300);

  AddTwoOp("MOV" , 0x4000); AddTwoOp("ADD" , 0x5000);
  AddTwoOp("ADDC", 0x6000); AddTwoOp("SUBC", 0x7000);
  AddTwoOp("SUB" , 0x8000); AddTwoOp("CMP" , 0x9000);
  AddTwoOp("DADD", 0xa000); AddTwoOp("BIT" , 0xb000);
  AddTwoOp("BIC" , 0xc000); AddTwoOp("BIS" , 0xd000);
  AddTwoOp("XOR" , 0xe000); AddTwoOp("AND" , 0xf000);

  OneOpOrders = (OneOpOrder *) malloc(sizeof(OneOpOrder)*OneOpCount); InstrZ = 0;
  AddOneOp("RRC" , True , 0x1000); AddOneOp("RRA" , True , 0x1100);
  AddOneOp("PUSH", True , 0x1200); AddOneOp("SWPB", False, 0x1080);
  AddOneOp("CALL", False, 0x1280); AddOneOp("SXT" , False, 0x1180);

  AddJmp("JNE" , 0x2000); AddJmp("JNZ" , 0x2000);
  AddJmp("JE"  , 0x2400); AddJmp("JZ"  , 0x2400);
  AddJmp("JNC" , 0x2800); AddJmp("JC"  , 0x2c00);
  AddJmp("JN"  , 0x3000); AddJmp("JGE" , 0x3400);
  AddJmp("JL"  , 0x3800); AddJmp("JMP" , 0x3C00);

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
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Attribut bearbeiten */

  if (*AttrPart == '\0') OpSize = 0;
  else if (strlen(AttrPart) > 1) WrXError(1107, AttrPart);
  else switch (mytoupper(*AttrPart))
  {
    case 'B': OpSize = 1; break;
    case 'W': OpSize = 0; break;
    default:  WrXError(1107, AttrPart); return;
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
  DeinitFields(); ClearONOFF();
}

static void SwitchTo_MSP(void)
{
  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

  PCSymbol = "$"; HeaderID = 0x4a; NOPCode = 0x4303; /* = MOV #0,#0 */
  DivideChars = ","; HasAttrs = True; AttrChars = ".";

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;

  AddONOFF("PADDING", &DoPadding, DoPaddingName, False);

  MakeCode = MakeCode_MSP; IsDef = IsDef_MSP;
  SwitchFrom = SwitchFrom_MSP; InitFields();
}

void codemsp_init(void)
{
  CPUMSP430 = AddCPU("MSP430", SwitchTo_MSP);
}
