/* code4500.c */ 
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MELPS-4500                                                  */
/*                                                                           */
/* Historie: 31.12.1996 (23.44!!) Grundsteinlegung                           */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 BookKeeping-Aufruf bei RES                           */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code4500.c,v 1.4 2014/11/06 11:58:16 alfred Exp $                    */
/*****************************************************************************
 * $Log: code4500.c,v $
 * Revision 1.4  2014/11/06 11:58:16  alfred
 * - rework to current style
 *
 * Revision 1.3  2008/11/23 10:39:16  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.2  2005/09/08 17:31:03  alfred
 * - add missing include
 *
 * Revision 1.1  2003/11/06 02:49:20  alfred
 * - recreated
 *
 * Revision 1.2  2002/08/14 18:43:48  alfred
 * - warn null allocation, remove some warnings
 *
 *****************************************************************************/

#include "stdinc.h"

#include <string.h>

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
  IntType Max;
} ConstOrder;


#define ConstOrderCount 12

static CPUVar CPU4500;

static ConstOrder *ConstOrders;

/*-------------------------------------------------------------------------*/

static void DecodeSFR(Word Code)
{
  UNUSED(Code);

  CodeEquate(SegData, 0, 415);
}

static void DecodeRES(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean ValOK;
    Word Size;

    FirstPassUnknown = False;
    Size = EvalIntExpression(ArgStr[1], Int16, &ValOK);
    if (FirstPassUnknown) WrError(1820);
    if ((ValOK) && (!FirstPassUnknown))
    {
      DontPrint = True;
      if (!Size) WrError(290);
      CodeLen = Size;
      BookKeeping();
    }
  }
}

static void DecodeDATA(Word Code)
{
  Boolean ValOK;
  TempResult t;
  char Ch;
  int z, z2;

  UNUSED(Code);

  if (ArgCnt == 0) WrError(1110);
  else
  {
    ValOK = True;
    for (z = 1; ValOK && (z <= ArgCnt); z++)
    {
      FirstPassUnknown = False;
      EvalExpression(ArgStr[z], &t);
      if ((t.Typ == TempInt) && (FirstPassUnknown))
        t.Contents.Int &= (ActPC == SegData) ? 7 : 511;

      switch (t.Typ)
      {
        case TempInt:
          if (ActPC == SegCode)
          {
            if (!RangeCheck(t.Contents.Int, Int10))
            {
              WrError(1320);
              ValOK = False;
            }
            else
              WAsmCode[CodeLen++] = t.Contents.Int & 0x3ff;
          }
          else
          {
            if (!RangeCheck(t.Contents.Int, Int4))
            {
              WrError(1320);
              ValOK = False;
            }
            else
              BAsmCode[CodeLen++] = t.Contents.Int & 0x0f;
          }
          break;
        case TempFloat:
          WrError(1135);
          ValOK = False;
          break;
        case TempString:
          for (z2 = 0; z2 < t.Contents.Ascii.Length; z2++)
          {
            Ch = CharTransTable[((usint) t.Contents.Ascii.Contents[z2]) & 0xff];
            if (ActPC == SegCode)
              WAsmCode[CodeLen++] = Ch;
            else
            {
              BAsmCode[CodeLen++] = Ch >> 4;
              BAsmCode[CodeLen++] = Ch & 15;
            }
          }
          break;
        default:
          ValOK = False;
      }
    }
    if (!ValOK)
      CodeLen = 0;
  }
}

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    CodeLen = 1;
    WAsmCode[0] = Code;
  }
}

static void DecodeConst(Word Index)
{
  const ConstOrder *pOrder = ConstOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;

    WAsmCode[0] = EvalIntExpression(ArgStr[1], pOrder->Max, &OK);
    if (OK)
    {
      CodeLen = 1;
      WAsmCode[0] += pOrder->Code;
    }
  }
}

static void DecodeSZD(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 0) WrError(1110);
  else
  {
    CodeLen = 2;
    WAsmCode[0] = 0x024;
    WAsmCode[1] = 0x02b;
  }
}

static void DecodeSEA(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;

    WAsmCode[1] = EvalIntExpression(ArgStr[1], UInt4, &OK);
    if (OK)
    {
      CodeLen = 2;
      WAsmCode[1] += 0x070;
      WAsmCode[0] = 0x025;
    }
  }
}

static void DecodeB(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Word AdrWord;
    Boolean OK;

    AdrWord = EvalIntExpression(ArgStr[1], UInt13, &OK);
    if (OK)
    {
      if ((!SymbolQuestionable) && ((((int)EProgCounter()) >> 7) != (AdrWord >> 7))) WrError(1910);
      else
      {
        CodeLen = 1;
        WAsmCode[0] = 0x180 + (AdrWord & 0x7f);
      }
    }
  }
}

static void DecodeBL_BML(Word Code)
{
  if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord;

    if (ArgCnt == 1)
      AdrWord = EvalIntExpression(ArgStr[1], UInt13, &OK);
    else
    {
      AdrWord = EvalIntExpression(ArgStr[1], UInt6, &OK) << 7;
      if (OK)
        AdrWord += EvalIntExpression(ArgStr[2], UInt7, &OK);
    }
    if (OK)
    {
      CodeLen = 2;
      WAsmCode[1] = 0x200 + (AdrWord & 0x7f) + ((AdrWord >> 12) << 7);
      WAsmCode[0] = Code + ((AdrWord >> 7) & 0x1f);
    }
  }
}

static void DecodeBLA_BMLA(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord;

    AdrWord = EvalIntExpression(ArgStr[1], UInt6, &OK);
    if (OK)
    {
      CodeLen = 2;
      WAsmCode[1] = 0x200 + (AdrWord & 0x0f) + ((AdrWord & 0x30) << 2);
      WAsmCode[0] = Code;
    }
  }
}

static void DecodeBM(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord;

    AdrWord = EvalIntExpression(ArgStr[1], UInt13, &OK);
    if (OK)
    {
      if ((AdrWord >> 7) != 2) WrError(1905);
      else
      {
        CodeLen = 1;
        WAsmCode[0] = 0x100 + (AdrWord & 0x7f);
      }
    }
  }
}

static void DecodeLXY(Word Code)
{
  UNUSED(Code);

  if ((ArgCnt == 0) || (ArgCnt > 2)) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord;

    if (ArgCnt == 1)
      AdrWord = EvalIntExpression(ArgStr[1], Int8, &OK);
    else
    {
      AdrWord = EvalIntExpression(ArgStr[1], Int4, &OK) << 4;
      if (OK)
        AdrWord += EvalIntExpression(ArgStr[2], Int4, &OK);
    }
    if (OK)
    {
      CodeLen = 1;
      WAsmCode[0] = 0x300 + AdrWord;
    }
  }
}

/*---------------------------------------------------------------------------*/

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddConst(char *NName, Word NCode, IntType NMax)
{
  if (InstrZ >= ConstOrderCount) exit(255);
  ConstOrders[InstrZ].Code = NCode;
  ConstOrders[InstrZ].Max = NMax;
  AddInstTable(InstTable, NName, InstrZ++, DecodeConst);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(307);
  AddInstTable(InstTable, "SZD", 0, DecodeSZD);
  AddInstTable(InstTable, "SEA", 0, DecodeSEA);
  AddInstTable(InstTable, "B", 0, DecodeB);
  AddInstTable(InstTable, "BL", 0x0e0, DecodeBL_BML);
  AddInstTable(InstTable, "BML", 0x0c0, DecodeBL_BML);
  AddInstTable(InstTable, "BLA", 0x010, DecodeBLA_BMLA);
  AddInstTable(InstTable, "BMLA", 0x030, DecodeBLA_BMLA);
  AddInstTable(InstTable, "BM", 0, DecodeBM);
  AddInstTable(InstTable, "LXY", 0, DecodeLXY);

  AddInstTable(InstTable, "SFR", 0, DecodeSFR);
  AddInstTable(InstTable, "RES", 0, DecodeRES);
  AddInstTable(InstTable, "DATA", 0, DecodeDATA);

  AddFixed("AM"   , 0x00a);  AddFixed("AMC"  , 0x00b);  AddFixed("AND"  , 0x018);
  AddFixed("CLD"  , 0x011);  AddFixed("CMA"  , 0x01c);  AddFixed("DEY"  , 0x017);
  AddFixed("DI"   , 0x004);  AddFixed("EI"   , 0x005);  AddFixed("IAP0" , 0x260);
  AddFixed("IAP1" , 0x261);  AddFixed("IAP2" , 0x262);  AddFixed("IAP3" , 0x263);
  AddFixed("IAP4" , 0x264);  AddFixed("INY"  , 0x013);  AddFixed("NOP"  , 0x000);
  AddFixed("OR"   , 0x019);  AddFixed("OP0A" , 0x220);  AddFixed("OP1A" , 0x221);
  AddFixed("POF"  , 0x002);  AddFixed("POF2" , 0x008);  AddFixed("RAR"  , 0x01d);
  AddFixed("RC"   , 0x006);  AddFixed("RC3"  , 0x2ac);  AddFixed("RC4"  , 0x2ae);
  AddFixed("RD"   , 0x014);  AddFixed("RT"   , 0x044);  AddFixed("RTI"  , 0x046);
  AddFixed("RTS"  , 0x045);  AddFixed("SC"   , 0x007);  AddFixed("SC3"  , 0x2ad);
  AddFixed("SC4"  , 0x2af);  AddFixed("SD"   , 0x015);  AddFixed("SEAM" , 0x026);
  AddFixed("SNZ0" , 0x038);  AddFixed("SNZP" , 0x003);  AddFixed("SNZT1", 0x280);
  AddFixed("SNZT2", 0x281);  AddFixed("SNZT3", 0x282);  AddFixed("SPCR" , 0x299);
  AddFixed("STCR" , 0x298);  AddFixed("SZC"  , 0x02f);  AddFixed("T1R1" , 0x2ab);
  AddFixed("T3AB" , 0x232);  AddFixed("TAB"  , 0x01e);  AddFixed("TAB3" , 0x272);
  AddFixed("TABE" , 0x02a);  AddFixed("TAD"  , 0x051);  AddFixed("TAI1" , 0x253);
  AddFixed("TAL1" , 0x24a);  AddFixed("TAMR" , 0x252);  AddFixed("TASP" , 0x050);
  AddFixed("TAV1" , 0x054);  AddFixed("TAW1" , 0x24b);  AddFixed("TAW2" , 0x24c);
  AddFixed("TAW3" , 0x24d);  AddFixed("TAX"  , 0x052);  AddFixed("TAY"  , 0x01f);
  AddFixed("TAZ"  , 0x053);  AddFixed("TBA"  , 0x00e);  AddFixed("TC1A" , 0x2a8);
  AddFixed("TC2A" , 0x2a9);  AddFixed("TDA"  , 0x029);  AddFixed("TEAB" , 0x01a);
  AddFixed("TI1A" , 0x217);  AddFixed("TL1A" , 0x20a);  AddFixed("TL2A" , 0x20b);
  AddFixed("TL3A" , 0x20c);  AddFixed("TLCA" , 0x20d);  AddFixed("TMRA" , 0x216);
  AddFixed("TPTA" , 0x2a5);  AddFixed("TPAA" , 0x2aa);  AddFixed("TR1A" , 0x2a6);
  AddFixed("TR1AB", 0x23f);  AddFixed("TV1A" , 0x03f);  AddFixed("TW1A" , 0x20e);
  AddFixed("TW2A" , 0x20f);  AddFixed("TW3A" , 0x210);  AddFixed("TYA"  , 0x00c);
  AddFixed("WRST" , 0x2a0);

  ConstOrders = (ConstOrder *) malloc(sizeof(ConstOrder) * ConstOrderCount); InstrZ = 0;
  AddConst("A"   , 0x060, UInt4);  AddConst("LA"  , 0x070, UInt4);
  AddConst("LZ"  , 0x048, UInt2);  AddConst("RB"  , 0x04c, UInt2);
  AddConst("SB"  , 0x05c, UInt2);  AddConst("SZB" , 0x020, UInt2);
  AddConst("TABP", 0x080, UInt6);  AddConst("TAM" , 0x2c0, UInt4);
  AddConst("TMA" , 0x2b0, UInt4);  AddConst("XAM" , 0x2d0, UInt4);
  AddConst("XAMD", 0x2f0, UInt4);  AddConst("XAMI", 0x2e0, UInt4);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);

  free(ConstOrders);
}

/*---------------------------------------------------------------------------*/

static void MakeCode_4500(void)
{
  CodeLen = 0;
  DontPrint = False;

  /* zu ignorierendes */

  if (Memo(""))
    return;

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static Boolean IsDef_4500(void)
{
  return (Memo("SFR"));
}

static void SwitchFrom_4500(void)
{
  DeinitFields();
}

static void SwitchTo_4500(void)
{
  TurnWords = False;
  ConstMode = ConstModeMoto;
  SetIsOccupied = False;

  PCSymbol = "*";
  HeaderID = 0x12;
  NOPCode = 0x000;
  DivideChars = ",";
  HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData);
  Grans[SegCode ] = 2; ListGrans[SegCode ] = 2; SegInits[SegCode ] = 0;
  SegLimits[SegCode] = 0x1fff;
  Grans[SegData ] = 1; ListGrans[SegData ] = 1; SegInits[SegData ] = 0;
  SegLimits[SegData] = 415;

  MakeCode = MakeCode_4500;
  IsDef = IsDef_4500;
  SwitchFrom = SwitchFrom_4500;

  InitFields();
}

void code4500_init(void)
{
  CPU4500 = AddCPU("MELPS4500", SwitchTo_4500);
}
