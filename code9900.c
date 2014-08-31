/* code9900.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS99xx                                                     */     
/*                                                                           */
/* Historie:  9. 3.1997 Grundsteinlegung                                     */     
/*           18. 8.1998 BookKeeping-Aufruf bei BSS                           */
/*            2. 1.1999 ChkPC angepasst                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code9900.c,v 1.7 2014/06/25 21:34:02 alfred Exp $                    */
/***************************************************************************** 
 * $Log: code9900.c,v $
 * Revision 1.7  2014/06/25 21:34:02  alfred
 * - rework to current style
 *
 * Revision 1.6  2013/12/21 19:46:51  alfred
 * - dynamically resize code buffer
 *
 * Revision 1.5  2008/11/23 10:39:17  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.4  2007/11/24 22:48:06  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/10/02 10:00:45  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.2  2005/09/08 17:31:05  alfred
 * - add missing include
 *
 * Revision 1.1  2003/11/06 02:49:22  alfred
 * - recreated
 *
 * Revision 1.4  2003/05/02 21:23:11  alfred
 * - strlen() updates
 *
 * Revision 1.3  2002/08/14 18:43:49  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.2  2002/07/14 18:39:59  alfred
 * - fixed TempAll-related warnings
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"
#include "endian.h"
#include "bpemu.h"
#include "nls.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"


static CPUVar CPU9900;

static Boolean IsWord;
static Word AdrVal,AdrPart;

/*-------------------------------------------------------------------------*/
/* Adressparser */

static Boolean DecodeReg(char *Asc, Word *Erg)
{
  Boolean OK;
  *Erg = EvalIntExpression(Asc, UInt4, &OK);
  return OK;
}

static char *HasDisp(char *Asc)
{
  char *p;
  int Lev, Len = strlen(Asc);

  if ((Len >= 2) && (Asc[Len - 1] == ')'))
  {
    p = Asc + Len - 2; Lev = 0;
    while ((p >= Asc) && (Lev != -1))
    {
      switch (*p)
      {
        case '(': Lev--; break;
        case ')': Lev++; break;
      }
      if (Lev != -1) p--;
    }
    if (Lev != -1)
    {
      WrError(1300);
      p = NULL;
    }
  }
  else
    p = NULL;

  return p;
}

static Boolean DecodeAdr(char *Asc)
{
  Boolean IncFlag;
  String Reg;
  Boolean OK;
  char *p;

  AdrCnt=0;

  if (*Asc == '*')
  {
    Asc++;
    if (Asc[strlen(Asc) - 1] == '+')
    {
      IncFlag = True;
      Asc[strlen(Asc) - 1] = '\0';
    }
    else
      IncFlag = False;
    if (DecodeReg(Asc, &AdrPart))
    {
      AdrPart += 0x10 + (Ord(IncFlag) << 5);
      return True;
    }
    return False;
  }

  if (*Asc == '@')
  {
    Asc++; p = HasDisp(Asc);
    if (!p)
    {
      FirstPassUnknown = False;
      AdrVal = EvalIntExpression(Asc, UInt16, &OK);
      if (OK)
      {
        AdrPart = 0x20;
        AdrCnt = 1;
        if ((!FirstPassUnknown) && (IsWord) && (Odd(AdrVal)))
          WrError(180);
        return True;
      }
    }
    else
    {
      strmaxcpy(Reg, p + 1, 255);
      Reg[strlen(Reg) - 1] = '\0';
      if (DecodeReg(Reg, &AdrPart))
      {
        if (AdrPart == 0) WrXError(1445, Reg);
        else
        {
          *p = '\0';
          AdrVal = EvalIntExpression(Asc, Int16, &OK);
          if (OK)
          {
            AdrPart += 0x20;
            AdrCnt = 1;
            return True;
          }
        }
      }
    }
    return False;
  }  

  if (DecodeReg(Asc, &AdrPart))
    return True;
  else
  {
    WrError(1350);
    return False;
  }
}

static void PutByte(Byte Value)
{
  if ((CodeLen & 1) && (!BigEndian))
  {
    BAsmCode[CodeLen] = BAsmCode[CodeLen - 1];
    BAsmCode[CodeLen - 1] = Value;
  }
  else
  {
    BAsmCode[CodeLen] = Value;
  }
  CodeLen++;
}

/*-------------------------------------------------------------------------*/
/* Code Generators */
 
static void DecodeTwo(Word Code)
{
  Word HPart;

  if (ArgCnt != 2) WrError(1110);
  else if (DecodeAdr(ArgStr[1]))
  {
    WAsmCode[0] = AdrPart;
    WAsmCode[1] = AdrVal;
    HPart = AdrCnt;
    if (DecodeAdr(ArgStr[2]))
    {
      WAsmCode[0] += AdrPart << 6;
      WAsmCode[1 + HPart] = AdrVal;
      CodeLen = (1 + HPart + AdrCnt) << 1;
      WAsmCode[0] += Code;
    }
  }
}

static void DecodeOne(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else if (DecodeAdr(ArgStr[1]))
  {
    Word HPart;

    WAsmCode[0] = AdrPart;
    WAsmCode[1] = AdrVal;
    if (DecodeReg(ArgStr[2], &HPart))
    {
      WAsmCode[0] += (HPart << 6) + (Code << 10);
      CodeLen = (1 + AdrCnt) << 1;
    }
  }
}

static void DecodeLDCR_STCR(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else if (DecodeAdr(ArgStr[1]))
  {
    Word HPart;
    Boolean OK;

    WAsmCode[0] = Code + AdrPart;
    WAsmCode[1] = AdrVal;
    FirstPassUnknown = False;
    HPart = EvalIntExpression(ArgStr[2], UInt5, &OK);
    if (FirstPassUnknown)
      HPart = 1;
    if (OK)
    {
      if (ChkRange(HPart, 1, 16))
      {
        WAsmCode[0] += (HPart & 15) << 6;
        CodeLen = (1 + AdrCnt) << 1;
      }
    }
  }
  return;
}

static void DecodeShift(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else if (DecodeReg(ArgStr[1], WAsmCode + 0))
  {
    Word HPart;

    if (DecodeReg(ArgStr[2], &HPart))
    {
      WAsmCode[0] += (HPart << 4) + (Code << 8);
      CodeLen = 2;
    }
  }
}

static void DecodeImm(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else if (DecodeReg(ArgStr[1], WAsmCode + 0))
  {
    Boolean OK;

    WAsmCode[1] = EvalIntExpression(ArgStr[2], Int16, &OK);
    if (OK)
    {
      WAsmCode[0] += Code;
      CodeLen = 4;
    }
  }
}

static void DecodeRegOrder(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (DecodeReg(ArgStr[1], WAsmCode + 0))
  {
    WAsmCode[0] += Code << 4;
    CodeLen = 2;
  }
};

static void DecodeLMF(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (DecodeReg(ArgStr[1], WAsmCode + 0))
  {
    Boolean OK;

    WAsmCode[0] += 0x320 + (EvalIntExpression(ArgStr[2], UInt1, &OK) << 4);
    if (OK)
      CodeLen = 2;
    if (!SupAllowed)
      WrError(50);
  }
}

static void DecodeMPYS_DIVS(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (DecodeAdr(ArgStr[1]))
  {
    WAsmCode[0] = Code + AdrPart;
    WAsmCode[1] = AdrVal;
    CodeLen = (1 + AdrCnt) << 1;
  }
}

static void DecodeSBit(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;

    WAsmCode[0] = EvalIntExpression(ArgStr[1], SInt8, &OK);
    if (OK)
    {
      WAsmCode[0] = (WAsmCode[0] & 0xff) | Code;
      CodeLen = 2;
    }
  }
}

static void DecodeJmp(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Integer AdrInt;

    AdrInt = EvalIntExpression(ArgStr[1], UInt16, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (Odd(AdrInt)) WrError(1375);
      else if ((!SymbolQuestionable) && ((AdrInt < -256) || (AdrInt > 254))) WrError(1370);
      else
      {
        WAsmCode[0] = ((AdrInt >> 1) & 0xff) | Code;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeLWPI_LIMI(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;

    WAsmCode[1] = EvalIntExpression(ArgStr[1], UInt16, &OK);
    if (OK)
    {
      WAsmCode[0] = Code;
      CodeLen = 4;
    }
  }
}

static void DecodeSing(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (DecodeAdr(ArgStr[1]))
  {
    WAsmCode[0] = (Code & 0x7fff) | AdrPart;
    WAsmCode[1] = AdrVal;
    CodeLen=(1 + AdrCnt) << 1;
    if ((Code & 0x8000) && (!SupAllowed))
      WrError(50);
  }
}

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    WAsmCode[0] = Code & 0x7fff;
    CodeLen = 2;
    if ((Code & 0x8000) && (!SupAllowed))
      WrError(50);
  }
}

static void DecodeBYTE(Word Code)
{
  int z;
  Boolean OK;
  TempResult t;

  UNUSED(Code);

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
          if (FirstPassUnknown)
            t.Contents.Int &= 0xff;
          if (!RangeCheck(t.Contents.Int, Int8)) WrError(1320);
          else if (SetMaxCodeLen(CodeLen + 1))
          {
            WrError(1920);
            OK = False;
          }
          else
            PutByte(t.Contents.Int);
          break;
        case TempFloat:
          WrError(1135);
          OK = False;
          break;
        case TempString:
          if (SetMaxCodeLen(t.Contents.Ascii.Length + CodeLen))
          {
            WrError(1920);
            OK = False;
          }
          else
          {
            char *p, *pEnd = t.Contents.Ascii.Contents + t.Contents.Ascii.Length;

            TranslateString(t.Contents.Ascii.Contents, t.Contents.Ascii.Length);
            for (p = t.Contents.Ascii.Contents; p < pEnd; PutByte(*(p++)));
          }
          break;
        default:
          OK = False;
          break;
      }
      z++;
    }
    while ((z <= ArgCnt) && (OK));
    if (!OK)
      CodeLen = 0;
    else if ((Odd(CodeLen)) && (DoPadding))
      PutByte(0);
  }
}

static void DecodeWORD(Word Code)
{
  int z;
  Boolean OK;
  Word HVal16;

  UNUSED(Code);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    z = 1;
    OK = True;
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
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeBSS(Word Code)
{
  Boolean OK;
  Word HVal16;  

  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    HVal16 = EvalIntExpression(ArgStr[1], Int16, &OK);
    if (FirstPassUnknown) WrError(1820);
    else if (OK)
    {
      if ((DoPadding) && (Odd(HVal16)))
        HVal16++;
      if (!HVal16) WrError(290);
      DontPrint = True;
      CodeLen = HVal16;
      BookKeeping();
    }
  }
}

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

static void AddTwo(char *NName16, char *NName8, Word NCode)
{
  AddInstTable(InstTable, NName16, (NCode << 13)         , DecodeTwo);
  AddInstTable(InstTable, NName8,  (NCode << 13) + 0x1000, DecodeTwo);
}

static void AddOne(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeOne);
}

static void AddSing(char *NName, Word NCode, Boolean NSup)
{
  AddInstTable(InstTable, NName, NCode | (NSup ? 0x8000 : 0x0000), DecodeSing);
}

static void AddSBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode << 8, DecodeSBit);
}

static void AddJmp(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode << 8, DecodeJmp);
}

static void AddShift(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeShift);
}

static void AddImm(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode << 5, DecodeImm);
}

static void AddReg(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeRegOrder);
}

static void AddFixed(char *NName, Word NCode, Boolean NSup)
{
  AddInstTable(InstTable, NName, NCode | (NSup ? 0x8000 : 0x0000), DecodeFixed);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(103);
  AddInstTable(InstTable, "LDCR", 0x3000, DecodeLDCR_STCR);
  AddInstTable(InstTable, "STCR", 0x3400, DecodeLDCR_STCR);
  AddInstTable(InstTable, "LMF", 0, DecodeLMF);
  AddInstTable(InstTable, "MPYS", 0x01c0, DecodeMPYS_DIVS);
  AddInstTable(InstTable, "DIVS", 0x0180, DecodeMPYS_DIVS);
  AddInstTable(InstTable, "LWPI", 0x02e0, DecodeLWPI_LIMI);
  AddInstTable(InstTable, "LIMI", 0x0300, DecodeLWPI_LIMI);
  AddInstTable(InstTable, "BYTE", 0, DecodeBYTE);
  AddInstTable(InstTable, "WORD", 0, DecodeWORD);
  AddInstTable(InstTable, "BSS", 0, DecodeBSS);

  AddTwo("A"   , "AB"   , 5); AddTwo("C"   , "CB"   , 4); AddTwo("S"   , "SB"   , 3);
  AddTwo("SOC" , "SOCB" , 7); AddTwo("SZC" , "SZCB" , 2); AddTwo("MOV" , "MOVB" , 6);

  AddOne("COC" , 0x08); AddOne("CZC" , 0x09); AddOne("XOR" , 0x0a);
  AddOne("MPY" , 0x0e); AddOne("DIV" , 0x0f); AddOne("XOP" , 0x0b);

  AddSing("B"   , 0x0440, False); AddSing("BL"  , 0x0680, False); AddSing("BLWP", 0x0400, False);
  AddSing("CLR" , 0x04c0, False); AddSing("SETO", 0x0700, False); AddSing("INV" , 0x0540, False);
  AddSing("NEG" , 0x0500, False); AddSing("ABS" , 0x0740, False); AddSing("SWPB", 0x06c0, False);
  AddSing("INC" , 0x0580, False); AddSing("INCT", 0x05c0, False); AddSing("DEC" , 0x0600, False);
  AddSing("DECT", 0x0640, True ); AddSing("X"   , 0x0480, False); AddSing("LDS" , 0x0780, True );
  AddSing("LDD" , 0x07c0, True );

  AddSBit("SBO" , 0x1d); AddSBit("SBZ", 0x1e); AddSBit("TB" , 0x1f);

  AddJmp("JEQ", 0x13); AddJmp("JGT", 0x15); AddJmp("JH" , 0x1b);
  AddJmp("JHE", 0x14); AddJmp("JL" , 0x1a); AddJmp("JLE", 0x12);
  AddJmp("JLT", 0x11); AddJmp("JMP", 0x10); AddJmp("JNC", 0x17);
  AddJmp("JNE", 0x16); AddJmp("JNO", 0x19); AddJmp("JOC", 0x18);
  AddJmp("JOP", 0x1c);

  AddShift("SLA", 0x0a); AddShift("SRA", 0x08);
  AddShift("SRC", 0x0b); AddShift("SRL", 0x09);

  AddImm("AI"  , 0x011); AddImm("ANDI", 0x012); AddImm("CI"  , 0x014);
  AddImm("LI"  , 0x010); AddImm("ORI" , 0x013);

  AddReg("STST", 0x02c); AddReg("LST", 0x008);
  AddReg("STWP", 0x02a); AddReg("LWP", 0x009);

  AddFixed("RTWP", 0x0380, False); AddFixed("IDLE", 0x0340, True );
  AddFixed("RSET", 0x0360, True ); AddFixed("CKOF", 0x03c0, True );
  AddFixed("CKON", 0x03a0, True ); AddFixed("LREX", 0x03e0, True );
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_9900(void)
{
  CodeLen = 0;
  DontPrint = False;
  IsWord = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static Boolean IsDef_9900(void)
{
  return False;
}

static void SwitchFrom_9900(void)
{
  DeinitFields();
  ClearONOFF();
}

static void InternSymbol_9900(char *Asc, TempResult*Erg)
{
  Boolean err;
  char *h = Asc;

  Erg->Typ = TempNone;
  if ((strlen(Asc) >= 2) && (mytoupper(*Asc) == 'R'))
    h = Asc + 1;
  else if ((strlen(Asc) >= 3) && (mytoupper(*Asc) == 'W') && (mytoupper(Asc[1]) == 'R'))
    h = Asc + 2;

  Erg->Contents.Int = ConstLongInt(h, &err, 10);
  if ((!err) || (Erg->Contents.Int < 0) || (Erg->Contents.Int > 15))
    return;

  Erg->Typ = TempInt;
}

static void SwitchTo_9900(void)
{
  TurnWords = True;
  ConstMode = ConstModeIntel;
  SetIsOccupied = False;

  PCSymbol = "$";
  HeaderID = 0x48;
  NOPCode = 0x0000;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;

  MakeCode = MakeCode_9900;
  IsDef = IsDef_9900;
  SwitchFrom = SwitchFrom_9900;
  InternSymbol = InternSymbol_9900;
  AddONOFF("PADDING", &DoPadding , DoPaddingName , False);
  AddONOFF("SUPMODE", &SupAllowed, SupAllowedName, False);

  InitFields();
}

void code9900_init(void)
{
  CPU9900 = AddCPU("TMS9900", SwitchTo_9900);
}
