/* codeavr.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Atmel AVR                                                   */             
/*                                                                           */
/* Historie: 26.12.1996 Grundsteinlegung                                     */             
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 BookKeeping-Aufruf bei RES                           */
/*           15.10.1998 LDD/STD mit <reg>+<symbol> ging nicht                */
/*            2. 5.1999 JMP/CALL momentan bei keinem Mitglied erlaubt        */
/*                      WRAPMODE eingebaut                                   */
/*           19.11.1999 Default-Hexmodus ist jetzt C                         */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*            7. 5.2000 Packing hinzugefuegt                                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codeavr.c,v 1.9 2014/06/22 08:13:30 alfred Exp $                     */
/*****************************************************************************
 * $Log: codeavr.c,v $
 * Revision 1.9  2014/06/22 08:13:30  alfred
 * - rework to current style
 *
 * Revision 1.8  2009/04/10 08:58:31  alfred
 * - correct address ranges for AVRs
 *
 * Revision 1.7  2008/11/23 10:39:17  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.6  2007/11/24 22:48:06  alfred
 * - some NetBSD changes
 *
 * Revision 1.5  2006/12/14 21:03:37  alfred
 * - correct target assignment
 *
 * Revision 1.4  2006/07/31 18:44:20  alfred
 * - add LPM variation with operands, devices up to ATmega256
 *
 * Revision 1.3  2006/03/05 18:07:42  alfred
 * - remove double InstTable variable
 *
 * Revision 1.2  2005/10/02 10:00:45  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.1  2003/11/06 02:49:22  alfred
 * - recreated
 *
 * Revision 1.9  2003/05/02 21:23:11  alfred
 * - strlen() updates
 *
 * Revision 1.8  2003/02/26 19:18:25  alfred
 * - add/use EvalIntDisplacement()
 *
 * Revision 1.7  2003/02/07 22:57:51  alfred
 * - fix empty left hand in offset
 *
 * Revision 1.6  2002/11/04 19:05:01  alfred
 * - silenced compiler warning
 *
 * Revision 1.5  2002/08/14 18:43:49  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.4  2002/05/11 20:12:46  alfred
 * - added MEGA instructions
 *
 * Revision 1.3  2002/04/20 19:26:30  alfred
 * - add MEGA8, use instruction hash table
 *
 * Revision 1.2  2002/04/06 20:24:10  alfred
 * - fixed alignment in DATA statement
 *
 *****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "nls.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"

static SimpProc SaveInitProc;

static CPUVar CPU90S1200, CPU90S2313, CPU90S4414, CPU90S8515,
              CPUATMEGA8, CPUATMEGA16, CPUATMEGA32, CPUATMEGA64, CPUATMEGA128, CPUATMEGA256;

static Boolean WrapFlag;
static LongInt ORMask, SignMask;

static IntType AdrIntType;

static char *WrapFlagName = "WRAPMODE";

/*---------------------------------------------------------------------------*/

static LongInt CutAdr(LongInt Adr)
{
  if ((Adr & SignMask) != 0)
    return (Adr | ORMask);
  else
    return (Adr & SegLimits[SegCode]);
}

/*---------------------------------------------------------------------------*/
/* argument decoders                                                         */

static Boolean DecodeReg(char *Asc, Word *Erg)
{
  Boolean io;
  char *s;
  int l;

  if (FindRegDef(Asc, &s))
    Asc = s;
  l = strlen(Asc);

  if ((l < 2) || (l > 3) || (mytoupper(*Asc) != 'R')) return False;
  else
  {
    *Erg = ConstLongInt(Asc + 1, &io, 10);
    return ((io) && (*Erg < 32));
  }
}

static Boolean DecodeMem(char * Asc, Word *Erg)
{
  if (strcasecmp(Asc, "X") == 0) *Erg=0x1c;
  else if (strcasecmp(Asc, "X+") == 0) *Erg = 0x1d;
  else if (strcasecmp(Asc, "-X") == 0) *Erg = 0x1e;
  else if (strcasecmp(Asc, "Y" ) == 0) *Erg = 0x08;
  else if (strcasecmp(Asc, "Y+") == 0) *Erg = 0x19;
  else if (strcasecmp(Asc, "-Y") == 0) *Erg = 0x1a;
  else if (strcasecmp(Asc, "Z" ) == 0) *Erg = 0x00;
  else if (strcasecmp(Asc, "Z+") == 0) *Erg = 0x11;
  else if (strcasecmp(Asc, "-Z") == 0) *Erg = 0x12;
  else return False;
  return True;
}

/*---------------------------------------------------------------------------*/
/* individual decoders                                                       */

/* pseudo instructions */

static void DecodePORT(Word Index)
{
  UNUSED(Index);

  CodeEquate(SegIO, 0, 0x3f);
}

/* no argument */

static void DecodeFixed(Word Code)
{
  if (ArgCnt!=0) WrError(1110);
  else
  {
    WAsmCode[0] = Code;
    CodeLen = 1;
  }
}

static void DecodeRES(Word Index)
{
  Boolean OK;
  Integer Size;

  UNUSED(Index);

  FirstPassUnknown = False;
  Size = EvalIntExpression(ArgStr[1], Int16, &OK);
  if (FirstPassUnknown) WrError(1820);
  if ((OK) && (!FirstPassUnknown))
  {
    DontPrint = True;
    if (!Size) WrError(290);
    CodeLen = Size;
    BookKeeping();
  }
}

static Boolean AccFull;

static void PlaceByte(Word Value, Boolean Pack)
{
  if (ActPC == SegCode)
  {
    if (Pack)
    {
      Value &= 0xff;
      if (AccFull)
        WAsmCode[CodeLen - 1] |= (Value << 8);
      else
       WAsmCode[CodeLen++] = Value;
      AccFull = !AccFull;
    }
    else
    {
      WAsmCode[CodeLen++] = Value;
      AccFull = FALSE;
    }
  }
  else
    BAsmCode[CodeLen++] = Value;
}

static void DecodeDATA(Word Index)
{
  Integer Trans;
  int z, z2;
  Boolean OK;
  TempResult t;
  LongInt MinV, MaxV;

  UNUSED(Index);

  MaxV = ((ActPC == SegCode) && (!Packing)) ? 65535 : 255;
  MinV = (-((MaxV + 1) >> 1));
  AccFull = FALSE;
  if (ArgCnt == 0) WrError(1110);
  else
  {
    OK = True;
    for (z = 1; z <= ArgCnt; z++)
     if (OK)
     {
       EvalExpression(ArgStr[z], &t);
       if ((FirstPassUnknown) && (t.Typ == TempInt)) t.Contents.Int &= MaxV;
       switch (t.Typ)
       {
         case TempInt:
           if (ChkRange(t.Contents.Int, MinV, MaxV))
             PlaceByte(t.Contents.Int, Packing);
           break;
         case TempFloat:
           WrError(1135); OK = False;
           break;
         case TempString:
           for (z2 = 0; z2 < (int)t.Contents.Ascii.Length; z2++)
           {
             Trans = CharTransTable[((usint) t.Contents.Ascii.Contents[z2]) & 0xff];
             PlaceByte(Trans, TRUE);
           }
           break;
         default:
           OK = False;
       }
     }
    if (!OK)
       CodeLen = 0;
  }
}

static void DecodeREG(Word Index)
{
  UNUSED(Index);

  if (ArgCnt!=1) WrError(1110);
  else AddRegDef(LabPart,ArgStr[1]);
}

/* one register 0..31 */

static void DecodeReg1(Word Code)
{
  Word Reg;

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else
  {
    WAsmCode[0] = Code | (Reg << 4);
    CodeLen = 1;
  }
}

/* two registers 0..31 */

static void DecodeReg2(Word Code)
{
  Word Reg1, Reg2;

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg1)) WrXError(1445, ArgStr[1]);
  else if (!DecodeReg(ArgStr[2], &Reg2)) WrXError(1445, ArgStr[2]);
  else
  {
    WAsmCode[0] = Code | (Reg2 & 15) | (Reg1 << 4) | ((Reg2 & 16) << 5);
    CodeLen = 1;
  }
}

/* one register 0..31 with itself */

static void DecodeReg3(Word Code)
{
  Word Reg;

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else
  {
    WAsmCode[0] = Code | (Reg & 15) | (Reg << 4) | ((Reg & 16) << 5);
    CodeLen = 1;
  }
}

/* immediate with register */

static void DecodeImm(Word Code)
{
  Word Reg, Const;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else if (Reg < 16) WrXError(1445, ArgStr[1]);
  else
  {
    Const = EvalIntExpression(ArgStr[2], Int8, &OK);
    if (OK)
    {
      WAsmCode[0] = Code | ((Const & 0xf0) << 4) | (Const & 0x0f) | ((Reg & 0x0f) << 4);
      CodeLen = 1;
    }
  }
}

static void DecodeADIW(Word Index)
{
  Word Reg, Const;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPU90S2313) WrError(1500);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else if ((Reg < 24) || (Odd(Reg))) WrXError(1445, ArgStr[1]);
  else
  {
    Const = EvalIntExpression(ArgStr[2], UInt6, &OK);
    if (OK)
    {
      WAsmCode[0] = 0x9600 | Index | ((Reg & 6) << 3) | (Const & 15) | ((Const & 0x30) << 2);
      CodeLen = 1;
    }
  }
}

/* transfer operations */

static void DecodeLDST(Word Index)
{
  int RegI, MemI;
  Word Reg, Mem;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    if (Index) /* ST */
    {
      MemI = 1; RegI = 2;
    }
    else
    {
      MemI = 2; RegI = 1;
    }
    if (!DecodeReg(ArgStr[RegI], &Reg)) WrXError(1445, ArgStr[RegI]);
    else if (!DecodeMem(ArgStr[MemI], &Mem)) WrError(1350);
    else if ((MomCPU < CPU90S2313) && (Mem != 0)) WrError(1351);
    else
    {
      WAsmCode[0] = 0x8000 | Index | (Reg << 4) | (Mem & 0x0f) | ((Mem & 0x10) << 8);
      CodeLen = 1;
      if (((Mem >= 0x1d) && (Mem <= 0x1e) && (Reg >= 26) && (Reg <= 27))  /* X+/-X with X */
       || ((Mem >= 0x19) && (Mem <= 0x1a) && (Reg >= 28) && (Reg <= 29))  /* Y+/-Y with Y */
       || ((Mem >= 0x11) && (Mem <= 0x12) && (Reg >= 30) && (Reg <= 31))) /* Z+/-Z with Z */
        WrError(140);
    }
  }
}

static void DecodeLDDSTD(Word Index)
{
  int RegI, MemI;
  Word Reg, Disp;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPU90S2313) WrXError(1500, OpPart);
  else
  {
    if (Index) /* STD */
    {
      MemI = 1; RegI = 2;
    }
    else
    {
      MemI = 2; RegI = 1;
    }
    OK = True;
    if (mytoupper(*ArgStr[MemI]) == 'Y') Index += 8;
    else if (mytoupper(*ArgStr[MemI]) == 'Z');
    else OK = False;
    if (!OK) WrError(1350);
    else if (!DecodeReg(ArgStr[RegI], &Reg)) WrXError(1445, ArgStr[RegI]);
    else
    {
      Disp = EvalIntDisplacement(ArgStr[MemI] + 1, UInt6, &OK);
      if (OK)
      {
        WAsmCode[0] = 0x8000 | Index | (Reg << 4) | (Disp & 7) | ((Disp & 0x18) << 7) | ((Disp & 0x20) << 8);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeINOUT(Word Index)
{
  int RegI, MemI;
  Word Reg, Mem;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    if (Index) /* OUT */
    {
      RegI = 2; MemI = 1;
    }
    else
    {
      RegI = 1; MemI = 2;
    }
    if (!DecodeReg(ArgStr[RegI], &Reg)) WrXError(1445, ArgStr[RegI]);
    else
    {
      Mem = EvalIntExpression(ArgStr[MemI], UInt6, &OK);
      if (OK)
      {
        ChkSpace(SegIO);
        WAsmCode[0] = 0xb000 | Index | (Reg << 4) | (Mem & 0x0f) | ((Mem & 0xf0) << 5);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeLDSSTS(Word Index)
{
  int RegI, MemI;
  Word Reg;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPU90S2313) WrError(1500);
  else
  {
    if (Index)
    {
      RegI = 2; MemI = 1;
    }
    else
    {
      RegI = 1; MemI = 2;
    }
    if (!DecodeReg(ArgStr[RegI], &Reg)) WrXError(1445, ArgStr[RegI]);
    else
    {
      WAsmCode[1] = EvalIntExpression(ArgStr[MemI], UInt16, &OK);
      if (OK)
      {
        ChkSpace(SegData);
        WAsmCode[0] = 0x9000 | Index | (Reg << 4); 
        CodeLen = 2;
      }
    }
  }
}

/* bit operations */

static void DecodeBCLRSET(Word Index)
{
  Word Bit;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Bit = EvalIntExpression(ArgStr[1], UInt3, &OK);
    if (OK)
    {
      WAsmCode[0] = 0x9408 | (Bit << 4) | Index;
      CodeLen = 1;
    }
  }
}

static void DecodeBit(Word Code)
{
  Word Reg, Bit;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else
  {
    Bit = EvalIntExpression(ArgStr[2], UInt3, &OK);
    if (OK)
    {
      WAsmCode[0] = Code | (Reg << 4) | Bit;
      CodeLen = 1;
    }
  }
}

static void DecodeCBR(Word Index)
{
  Word Reg, Mask;
  Boolean OK;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else if (Reg < 16) WrXError(1445, ArgStr[1]);
  else
  {
    Mask = EvalIntExpression(ArgStr[2], Int8, &OK) ^ 0xff;
    if (OK)
    {
      WAsmCode[0] = 0x7000 | ((Mask & 0xf0) << 4) | (Mask & 0x0f) | ((Reg & 0x0f) << 4);
      CodeLen = 1;
    }
  }
}

static void DecodeSER(Word Index)
{
  Word Reg;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else if (Reg < 16) WrXError(1445, ArgStr[1]);
  else
  {
    WAsmCode[0] = 0xef0f | ((Reg & 0x0f) << 4);
    CodeLen = 1;
  }
}

static void DecodePBit(Word Code)
{
  Word Adr, Bit;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    Adr = EvalIntExpression(ArgStr[1], UInt5, &OK);
    if (OK)
    {
      ChkSpace(SegIO);
      Bit = EvalIntExpression(ArgStr[2], UInt3, &OK);
      if (OK)
      {
        WAsmCode[0] = Code | Bit | (Adr << 3);
        CodeLen = 1;
      }
    }
  }
}

/* branches */

static void DecodeRel(Word Code)
{
  LongInt AdrInt;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    AdrInt = EvalIntExpression(ArgStr[1], AdrIntType, &OK) - (EProgCounter() + 1);
    if (OK)
    {
      if (WrapFlag) AdrInt = CutAdr(AdrInt);
      if ((!SymbolQuestionable) && ((AdrInt < -64) || (AdrInt > 63))) WrError(1370);
      else
      {
        ChkSpace(SegCode);
        WAsmCode[0] = Code | ((AdrInt & 0x7f) << 3);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeBRBSBC(Word Index)
{
  Word Bit;
  LongInt AdrInt;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    Bit = EvalIntExpression(ArgStr[1], UInt3, &OK);
    if (OK)
    {
      AdrInt = EvalIntExpression(ArgStr[2], AdrIntType, &OK) - (EProgCounter() + 1);
      if (OK)
      {
        if (WrapFlag) AdrInt = CutAdr(AdrInt);
        if ((!SymbolQuestionable) && ((AdrInt < -64) || (AdrInt > 63))) WrError(1370);
        else
        {
          ChkSpace(SegCode);
          WAsmCode[0] = 0xf000 | Index | ((AdrInt & 0x7f) << 3) | Bit;
          CodeLen = 1;
        }
      }
    }
  }
}

static void DecodeJMPCALL(Word Index)
{
  LongInt AdrInt;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < CPUATMEGA8) WrError(1500);
  else
  {
    AdrInt = EvalIntExpression(ArgStr[1], UInt22, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      WAsmCode[0] = 0x940c | Index | ((AdrInt & 0x3e0000) >> 13) | ((AdrInt & 0x10000) >> 16);
      WAsmCode[1] = AdrInt & 0xffff;
      CodeLen = 2;
    }
  }
}

static void DecodeRJMPCALL(Word Index)
{
  LongInt AdrInt;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    AdrInt = EvalIntExpression(ArgStr[1], UInt22, &OK) - (EProgCounter() + 1);
    if (OK)
    {
      if (WrapFlag) AdrInt = CutAdr(AdrInt);
      if ((!SymbolQuestionable) && ((AdrInt < -2048) || (AdrInt > 2047))) WrError(1370);
      else
      {
        ChkSpace(SegCode);
        WAsmCode[0] = 0xc000 | Index | (AdrInt & 0xfff);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeMULS(Word Index)
{
  Word Reg1, Reg2;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPUATMEGA8) WrError(1500);
  else if (!DecodeReg(ArgStr[1], &Reg1)) WrXError(1445, ArgStr[1]);
  else if (Reg1 < 16) WrXError(1445, ArgStr[1]);
  else if (!DecodeReg(ArgStr[2], &Reg2)) WrXError(1445, ArgStr[2]);
  else if (Reg2 < 16) WrXError(1445, ArgStr[2]);
  else
  {
    WAsmCode[0] = 0x0200 | ((Reg1 & 15) << 4) | (Reg2 & 15);
    CodeLen = 1;
  }
}

static void DecodeMegaMUL(Word Index)
{
  Word Reg1, Reg2;

  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPUATMEGA8) WrError(1500);
  else if (!DecodeReg(ArgStr[1], &Reg1)) WrXError(1445, ArgStr[1]);
  else if ((Reg1 < 16) || (Reg1 > 23)) WrXError(1445, ArgStr[1]);
  else if (!DecodeReg(ArgStr[2], &Reg2)) WrXError(1445, ArgStr[2]);
  else if ((Reg2 < 16) || (Reg2 > 23)) WrXError(1445, ArgStr[2]);
  else
  {
    WAsmCode[0] = Index | ((Reg1 & 7) << 4) | (Reg2 & 7);
    CodeLen = 1;
  }
}

static void DecodeMOVW(Word Index)
{
  Word Reg1, Reg2;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPUATMEGA8) WrError(1500);
  else if (!DecodeReg(ArgStr[1], &Reg1)) WrXError(1445, ArgStr[1]);
  else if (Reg1 & 1) WrXError(1445, ArgStr[1]);  
  else if (!DecodeReg(ArgStr[2], &Reg2)) WrXError(1445, ArgStr[2]);
  else if (Reg2 & 1) WrXError(1445, ArgStr[2]);  
  else
  {   
    WAsmCode[0] = 0x0100 | ((Reg1 >> 1) << 4) | (Reg2 >> 1);
    CodeLen = 1;
  }
}

static void DecodeLPM(Word Index)
{
  Word Reg, Adr;

  UNUSED(Index);

  if (!ArgCnt)
  {
    if (MomCPU < CPU90S2313) WrError(1500);
    else
    {
      WAsmCode[0] = 0x95c8;
      CodeLen = 1;
    }
  }
  else if (ArgCnt == 2)
  {
    if (MomCPU < CPUATMEGA8) WrError(1500);
    else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
    else if (!DecodeMem(ArgStr[2], &Adr)) WrError(1350);
    else if ((Adr != 0x00) && (Adr != 0x11)) WrError(1350);
    else
    {
      if (((Reg == 30) || (Reg == 31)) && (Adr == 0x11)) WrError(140);
      WAsmCode[0] = 0x9004 | (Reg << 4) | (Adr & 1);
      CodeLen = 1;
    }
  }
  else WrError(1110);
}

static void DecodeELPM(Word Index)
{
  Word Reg, Adr;

  UNUSED(Index);

  if (MomCPU < CPUATMEGA8) WrError(1500);
  else if (!ArgCnt)
  {
    WAsmCode[0] = 0x95d8;
    CodeLen = 1;
  }
  else if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else if (!DecodeMem(ArgStr[2], &Adr)) WrError(1350);
  else if ((Adr != 0x00) && (Adr != 0x11)) WrError(1350);
  else
  {
    if (((Reg == 30) || (Reg == 31)) && (Adr == 0x11)) WrError(140);
    WAsmCode[0] = 0x9006 | (Reg << 4) | (Adr & 1);
    CodeLen = 1;
  }
}

/*---------------------------------------------------------------------------*/
/* dynamic code table handling                                               */

static void AddFixed(char *NName, CPUVar NMin, Word NCode)
{
  if (MomCPU >= NMin)
    AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddReg1(char *NName, CPUVar NMin, Word NCode)
{
  if (MomCPU >= NMin)
    AddInstTable(InstTable, NName, NCode, DecodeReg1);
}
   
static void AddReg2(char *NName, CPUVar NMin, Word NCode)
{
  if (MomCPU >= NMin)
    AddInstTable(InstTable, NName, NCode, DecodeReg2);
}

static void AddReg3(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeReg3);
}

static void AddImm(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm);
}

static void AddRel(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeRel);
}

static void AddBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBit);
}

static void AddPBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodePBit);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);

  AddFixed("IJMP" , CPU90S2313, 0x9409); AddFixed("ICALL" , CPU90S2313, 0x9509);
  AddFixed("RET"  , CPU90S1200, 0x9508); AddFixed("RETI"  , CPU90S1200, 0x9518);
  AddFixed("SEC"  , CPU90S1200, 0x9408);
  AddFixed("CLC"  , CPU90S1200, 0x9488); AddFixed("SEN"   , CPU90S1200, 0x9428);
  AddFixed("CLN"  , CPU90S1200, 0x94a8); AddFixed("SEZ"   , CPU90S1200, 0x9418);
  AddFixed("CLZ"  , CPU90S1200, 0x9498); AddFixed("SEI"   , CPU90S1200, 0x9478);
  AddFixed("CLI"  , CPU90S1200, 0x94f8); AddFixed("SES"   , CPU90S1200, 0x9448);
  AddFixed("CLS"  , CPU90S1200, 0x94c8); AddFixed("SEV"   , CPU90S1200, 0x9438);
  AddFixed("CLV"  , CPU90S1200, 0x94b8); AddFixed("SET"   , CPU90S1200, 0x9468);
  AddFixed("CLT"  , CPU90S1200, 0x94e8); AddFixed("SEH"   , CPU90S1200, 0x9458);
  AddFixed("CLH"  , CPU90S1200, 0x94d8); AddFixed("NOP"   , CPU90S1200, 0x0000);
  AddFixed("SLEEP", CPU90S1200, 0x9588); AddFixed("WDR"   , CPU90S1200, 0x95a8);
  AddFixed("EIJMP", CPUATMEGA8, 0x9419); AddFixed("EICALL", CPUATMEGA8, 0x9519);
  AddFixed("SPM"  , CPUATMEGA8, 0x95e8); AddFixed("BREAK" , CPUATMEGA8, 0x9598);

  AddReg1("COM"  , CPU90S1200, 0x9400); AddReg1("NEG"  , CPU90S1200, 0x9401);
  AddReg1("INC"  , CPU90S1200, 0x9403); AddReg1("DEC"  , CPU90S1200, 0x940a);
  AddReg1("PUSH" , CPU90S2313, 0x920f); AddReg1("POP"  , CPU90S2313, 0x900f);
  AddReg1("LSR"  , CPU90S1200, 0x9406); AddReg1("ROR"  , CPU90S1200, 0x9407);
  AddReg1("ASR"  , CPU90S1200, 0x9405); AddReg1("SWAP" , CPU90S1200, 0x9402);

  AddReg2("ADD"  , CPU90S1200, 0x0c00); AddReg2("ADC"  , CPU90S1200, 0x1c00);
  AddReg2("SUB"  , CPU90S1200, 0x1800); AddReg2("SBC"  , CPU90S1200, 0x0800);
  AddReg2("AND"  , CPU90S1200, 0x2000); AddReg2("OR"   , CPU90S1200, 0x2800);
  AddReg2("EOR"  , CPU90S1200, 0x2400); AddReg2("CPSE" , CPU90S1200, 0x1000);
  AddReg2("CP"   , CPU90S1200, 0x1400); AddReg2("CPC"  , CPU90S1200, 0x0400);
  AddReg2("MOV"  , CPU90S1200, 0x2c00); AddReg2("MUL"  , CPUATMEGA8, 0x9c00);

  AddReg3("CLR"  , 0x2400); AddReg3("TST"  , 0x2000); AddReg3("LSL"  , 0x0c00);
  AddReg3("ROL"  , 0x1c00);

  AddImm("SUBI" , 0x5000); AddImm("SBCI" , 0x4000); AddImm("ANDI" , 0x7000);
  AddImm("ORI"  , 0x6000); AddImm("SBR"  , 0x6000); AddImm("CPI"  , 0x3000);
  AddImm("LDI"  , 0xe000);

  AddRel("BRCC" , 0xf400); AddRel("BRCS" , 0xf000); AddRel("BREQ" , 0xf001);
  AddRel("BRGE" , 0xf404); AddRel("BRSH" , 0xf400); AddRel("BRID" , 0xf407);
  AddRel("BRIE" , 0xf007); AddRel("BRLO" , 0xf000); AddRel("BRLT" , 0xf004);
  AddRel("BRMI" , 0xf002); AddRel("BRNE" , 0xf401); AddRel("BRHC" , 0xf405);
  AddRel("BRHS" , 0xf005); AddRel("BRPL" , 0xf402); AddRel("BRTC" , 0xf406);
  AddRel("BRTS" , 0xf006); AddRel("BRVC" , 0xf403); AddRel("BRVS" , 0xf003);

  AddBit("BLD"  , 0xf800); AddBit("BST"  , 0xfa00);
  AddBit("SBRC" , 0xfc00); AddBit("SBRS" , 0xfe00);

  AddPBit("CBI" , 0x9800); AddPBit("SBI" , 0x9a00);
  AddPBit("SBIC", 0x9900); AddPBit("SBIS", 0x9b00);

  AddInstTable(InstTable, "ADIW", 0x0000, DecodeADIW);
  AddInstTable(InstTable, "SBIW", 0x0100, DecodeADIW);

  AddInstTable(InstTable, "LD", 0x0000, DecodeLDST);
  AddInstTable(InstTable, "ST", 0x0200, DecodeLDST);

  AddInstTable(InstTable, "LDD", 0x0000, DecodeLDDSTD);
  AddInstTable(InstTable, "STD", 0x0200, DecodeLDDSTD);

  AddInstTable(InstTable, "IN" , 0x0000, DecodeINOUT);
  AddInstTable(InstTable, "OUT", 0x0800, DecodeINOUT);

  AddInstTable(InstTable, "LDS", 0x0000, DecodeLDSSTS);
  AddInstTable(InstTable, "STS", 0x0200, DecodeLDSSTS);

  AddInstTable(InstTable, "BCLR", 0x0080, DecodeBCLRSET);
  AddInstTable(InstTable, "BSET", 0x0000, DecodeBCLRSET);

  AddInstTable(InstTable, "CBR", 0, DecodeCBR);
  AddInstTable(InstTable, "SER", 0, DecodeSER);

  AddInstTable(InstTable, "BRBC", 0x0400, DecodeBRBSBC);
  AddInstTable(InstTable, "BRBS", 0x0000, DecodeBRBSBC);

  AddInstTable(InstTable, "JMP" , 0, DecodeJMPCALL);
  AddInstTable(InstTable, "CALL", 2, DecodeJMPCALL);

  AddInstTable(InstTable, "RJMP" , 0x0000, DecodeRJMPCALL);
  AddInstTable(InstTable, "RCALL", 0x1000, DecodeRJMPCALL);

  AddInstTable(InstTable, "PORT", 0, DecodePORT);
  AddInstTable(InstTable, "RES" , 0, DecodeRES);
  AddInstTable(InstTable, "DATA", 0, DecodeDATA);
  AddInstTable(InstTable, "REG" , 0, DecodeREG);

  AddInstTable(InstTable, "MULS", 0, DecodeMULS);

  AddInstTable(InstTable, "MULSU" , 0x0300, DecodeMegaMUL);
  AddInstTable(InstTable, "FMUL"  , 0x0308, DecodeMegaMUL);
  AddInstTable(InstTable, "FMULS" , 0x0380, DecodeMegaMUL);
  AddInstTable(InstTable, "FMULSU", 0x0388, DecodeMegaMUL);

  AddInstTable(InstTable, "MOVW", 0, DecodeMOVW);

  AddInstTable(InstTable, "LPM" , 0, DecodeLPM);
  AddInstTable(InstTable, "ELPM", 0, DecodeELPM);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------------*/

static void MakeCode_AVR(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* all done via table :-) */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static void InitCode_AVR(void)
{
  SaveInitProc();
  SetFlag(&Packing, PackingName, False);
}

static Boolean IsDef_AVR(void)
{
  return (Memo("PORT") || Memo("REG"));
}

static void SwitchFrom_AVR(void)
{
  DeinitFields();
  ClearONOFF();
}

static void SwitchTo_AVR(void)
{
  TurnWords = False;
  ConstMode = ConstModeC;
  SetIsOccupied = True;

  PCSymbol = "*";
  HeaderID = 0x3b;
  NOPCode = 0x0000;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
  Grans[SegCode] = 2; ListGrans[SegCode] = 2; SegInits[SegCode] = 0;
  Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 32;
  Grans[SegIO  ] = 1; ListGrans[SegIO  ] = 1; SegInits[SegIO  ] = 0;  SegLimits[SegIO] = 0x3f;

  if (MomCPU == CPU90S1200)
  {
    SegLimits[SegCode] = 0x01ff;
    SegLimits[SegData] = 0x5f;
    AdrIntType = UInt9;
  }
  else if (MomCPU == CPU90S2313)
  {
    SegLimits[SegCode] = 0x03ff;
    SegLimits[SegData] = 0xdf;
    AdrIntType = UInt10;
  }
  else if (MomCPU == CPU90S4414)
  {
    SegLimits[SegCode] = 0x07ff;
    AdrIntType = UInt11;
  }
  else if ((MomCPU == CPU90S8515) || (MomCPU == CPUATMEGA8))
  {
    SegLimits[SegCode] = 0xfff;
    SegLimits[SegData] = 0x3ff;
    AdrIntType = UInt12;
  }
  else if (MomCPU == CPUATMEGA16)
  {
    SegLimits[SegCode] = 0x1fff;
    SegLimits[SegData] = 0x3ff;
    AdrIntType = UInt13;
  }
  else if (MomCPU == CPUATMEGA32)
  {
    SegLimits[SegCode] = 0x3fff;
    SegLimits[SegData] = 0x7ff; 
    AdrIntType = UInt14;
  }
  else if (MomCPU == CPUATMEGA64)
  {
    SegLimits[SegCode] = 0x7fff;
    SegLimits[SegData] = 0xfff; 
    AdrIntType = UInt15;
  }
  else if (MomCPU == CPUATMEGA128)
  {
    SegLimits[SegCode] = 0xffff;
    SegLimits[SegData] = 0xfff; 
    AdrIntType = UInt16;
  }
  else if (MomCPU == CPUATMEGA256)
  {
    SegLimits[SegCode] = 0x1ffff;
    SegLimits[SegData] = 0x1fff; 
    AdrIntType = UInt17;
  }

  SignMask = (SegLimits[SegCode] + 1) >> 1;
  ORMask = ((LongInt) - 1) - SegLimits[SegCode];

  AddONOFF("WRAPMODE", &WrapFlag, WrapFlagName, False);
  AddONOFF("PACKING", &Packing, PackingName, False);
  SetFlag(&WrapFlag, WrapFlagName, False);

  MakeCode = MakeCode_AVR;
  IsDef = IsDef_AVR;
  SwitchFrom = SwitchFrom_AVR;
  InitFields();
}

        void codeavr_init(void)
{
   CPU90S1200  = AddCPU("AT90S1200" , SwitchTo_AVR);
   CPU90S2313  = AddCPU("AT90S2313" , SwitchTo_AVR);
   CPU90S4414  = AddCPU("AT90S4414" , SwitchTo_AVR);
   CPU90S8515  = AddCPU("AT90S8515" , SwitchTo_AVR);
   CPUATMEGA8  = AddCPU("ATMEGA8"   , SwitchTo_AVR); 
   CPUATMEGA16 = AddCPU("ATMEGA16"  , SwitchTo_AVR); 
   CPUATMEGA32 = AddCPU("ATMEGA32"  , SwitchTo_AVR); 
   CPUATMEGA64 = AddCPU("ATMEGA64"  , SwitchTo_AVR);
   CPUATMEGA128 = AddCPU("ATMEGA128" , SwitchTo_AVR);
   CPUATMEGA256 = AddCPU("ATMEGA256" , SwitchTo_AVR);

   SaveInitProc = InitPassProc; InitPassProc = InitCode_AVR;
}

