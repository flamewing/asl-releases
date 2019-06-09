/* codest6.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator ST6-Familie                                                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "codevars.h"
#include "errmsg.h"

#include "codest6.h"

#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModInd 2
#define MModInd (1 << ModInd)


static Byte AdrMode;
static ShortInt AdrType;
static Byte AdrVal;

static LongInt WinAssume;

static CPUVar CPUST6210, CPUST6215, CPUST6220, CPUST6225;

#define ASSUME62Count 1
static ASSUMERec ASSUME62s[ASSUME62Count] =
{
  {"ROMBASE", &WinAssume, 0, 0x3f, 0x40, NULL}
};

/*---------------------------------------------------------------------------------*/
/* Helper Functions */

static void ResetAdr(void)
{
  AdrType = ModNone; AdrCnt = 0;
}

static void DecodeAdr(const tStrComp *pArg, Byte Mask)
{
#define RegCnt 5
  static char *RegNames[RegCnt + 1] = {"A", "V", "W", "X", "Y"};
  static Byte RegCodes[RegCnt + 1] = {0xff, 0x82, 0x83, 0x80, 0x81};

  Boolean OK;
  int z;
  Integer AdrInt;

  ResetAdr();

  if ((!strcasecmp(pArg->Str, "A")) && (Mask & MModAcc))
  {
    AdrType = ModAcc;
    goto chk;
  }

  for (z = 0; z < RegCnt; z++)
    if (!strcasecmp(pArg->Str, RegNames[z]))
    {
      AdrType = ModDir;
      AdrCnt = 1;
      AdrVal = RegCodes[z];
      goto chk;
    }

  if (!strcasecmp(pArg->Str, "(X)"))
  {
    AdrType = ModInd;
    AdrMode = 0;
    goto chk;
  }

  if (!strcasecmp(pArg->Str, "(Y)"))
  {
    AdrType = ModInd;
    AdrMode = 1;
    goto chk;
  }

  AdrInt = EvalStrIntExpression(pArg, UInt16, &OK);
  if (OK)
  {
    if (TypeFlag & (1 << SegCode))
    {
      AdrType = ModDir;
      AdrVal = (AdrInt & 0x3f) + 0x40;
      AdrCnt=1;
      if (!FirstPassUnknown)
        if (WinAssume != (AdrInt >> 6)) WrError(ErrNum_InAccPage);
    }
    else
    {
      if (FirstPassUnknown) AdrInt = Lo(AdrInt);
      if (AdrInt>0xff) WrError(ErrNum_OverRange);
      else
      {
        AdrType = ModDir;
        AdrVal = AdrInt;
        goto chk;
      }
    }
  }

chk:
  if ((AdrType != ModNone) && (!(Mask & (1 << AdrType))))
  {
    ResetAdr(); WrError(ErrNum_InvAddrMode);
  }
}

static Boolean IsReg(Byte Adr)
{
  return ((Adr & 0xfc) == 0x80);
}

static Byte MirrBit(Byte inp)
{
  return (((inp & 1) << 2) + (inp & 2) + ((inp & 4) >> 2));
}

/*---------------------------------------------------------------------------------*/
/* Instruction Decoders */

static void DecodeFixed(Word Code)
{
  if (ChkArgCnt(0, 0))
  {
    CodeLen = 1;
    BAsmCode[0] = Code;
  }
}

static void DecodeLD(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModAcc | MModDir | MModInd);
    switch (AdrType)
    {
      case ModAcc:
        DecodeAdr(&ArgStr[2], MModDir | MModInd);
        switch (AdrType)
        {
          case ModDir:
            if (IsReg(AdrVal))
            {
              CodeLen = 1;
              BAsmCode[0] = 0x35 + ((AdrVal & 3) << 6);
            }
            else
            {
              CodeLen = 2;
              BAsmCode[0] = 0x1f;
              BAsmCode[1] = AdrVal;
            }
            break;
          case ModInd:
            CodeLen = 1;
            BAsmCode[0] = 0x07 + (AdrMode << 3);
            break;
        }
        break;
      case ModDir:
        DecodeAdr(&ArgStr[2], MModAcc);
        if (AdrType != ModNone)
        {
          if (IsReg(AdrVal))
          {
            CodeLen = 1;
            BAsmCode[0] = 0x3d + ((AdrVal & 3) << 6);
          }
          else
          {
            CodeLen = 2;
            BAsmCode[0] = 0x9f;
            BAsmCode[1] = AdrVal;
          }
        }
        break;
      case ModInd:
        DecodeAdr(&ArgStr[2], MModAcc);
        if (AdrType != ModNone)
        {
          CodeLen = 1;
          BAsmCode[0] = 0x87 + (AdrMode << 3);
        }
        break;
    }
  }
}

static void DecodeLDI(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(2, 2))
  {
    Boolean OK;

    Integer AdrInt = EvalStrIntExpression(&ArgStr[2], Int8, &OK);
    if (OK)
    {
      DecodeAdr(&ArgStr[1], MModAcc | MModDir);
      switch (AdrType)
      {
        case ModAcc:
          CodeLen = 2;
          BAsmCode[0] = 0x17;
          BAsmCode[1] = Lo(AdrInt);
          break;
        case ModDir:
          CodeLen = 3;
          BAsmCode[0] = 0x0d;
          BAsmCode[1] = AdrVal;
          BAsmCode[2] = Lo(AdrInt);
          break;
      }
    }
  }
}

static void DecodeRel(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    Integer AdrInt = EvalStrIntExpression(&ArgStr[1], UInt16, &OK) - (EProgCounter() + 1);
    if (OK)
    {
      if ((!SymbolQuestionable) && ((AdrInt < -16) || (AdrInt > 15))) WrError(ErrNum_JmpDistTooBig);
      else
      {
        CodeLen = 1;
        BAsmCode[0] = Code + ((AdrInt << 3) & 0xf8);
      }
    }
  }
}

static void DecodeJP_CALL(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    Boolean OK;
    Integer AdrInt = EvalStrIntExpression(&ArgStr[1], Int16, &OK);
    if (OK)
    {
      if ((AdrInt < 0) || (AdrInt > 0xfff)) WrError(ErrNum_AdrOverflow);
      else
      {
        CodeLen = 2;
        BAsmCode[0] = Code + ((AdrInt & 0x00f) << 4);
        BAsmCode[1] = AdrInt >> 4;
      }
    }
  }
}

static void DecodeALU(Word Code)
{
  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModAcc);
    if (AdrType != ModNone)
    {
      DecodeAdr(&ArgStr[2], MModDir | MModInd);
      switch (AdrType)
      {
        case ModDir:
          CodeLen = 2;
          BAsmCode[0] = Code + 0x18;
          BAsmCode[1] = AdrVal;
          break;
        case ModInd:
          CodeLen = 1;
          BAsmCode[0] = Code + (AdrMode << 3);
          break;
      }
    }
  }
}

static void DecodeALUImm(Word Code)
{
  if (ChkArgCnt(2, 2))
  {
    DecodeAdr(&ArgStr[1], MModAcc);
    if (AdrType != ModNone)
    {
      Boolean OK;
      BAsmCode[1] = EvalStrIntExpression(&ArgStr[2], Int8, &OK);
      if (OK)
      {
        CodeLen = 2;
        BAsmCode[0] = Code + 0x10;
      }
    }
  }
}

static void DecodeCLR(Word Code)
{
  UNUSED(Code);

  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModAcc | MModDir);
    switch (AdrType)
    {
      case ModAcc:
        CodeLen = 2;
        BAsmCode[0] = 0xdf;
        BAsmCode[1] = 0xff;
        break;
      case ModDir:
        CodeLen = 3;
        BAsmCode[0] = 0x0d;
        BAsmCode[1] = AdrVal;
        BAsmCode[2] = 0;
        break;
    }
  }
}

static void DecodeAcc(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModAcc);
    if (AdrType != ModNone)
    {
      BAsmCode[CodeLen++] = Lo(Code);
      if (Hi(Code))
        BAsmCode[CodeLen++] = Hi(Code);
    }
  }
}

static void DecodeINC_DEC(Word Code)
{
  if (ChkArgCnt(1, 1))
  {
    DecodeAdr(&ArgStr[1], MModDir | MModInd);
    switch (AdrType)
    {
      case ModDir:
        if (IsReg(AdrVal))
        {
          CodeLen = 1;
          BAsmCode[0] = Code + 0x15 + ((AdrVal & 3) << 6);
        }
        else
        {
          CodeLen = 2;
          BAsmCode[0] = 0x7f + (Code << 4);
          BAsmCode[1] = AdrVal;
        }
        break;
      case ModInd:
        CodeLen = 1;
        BAsmCode[0] = 0x67 + (AdrMode << 3) + (Code << 4);
        break;
    }
  }
}

static void DecodeSET_RES(Word Code)
{
  if (ChkArgCnt(2, 2))
  {
    Boolean OK;

    BAsmCode[0] = MirrBit(EvalStrIntExpression(&ArgStr[1], UInt3, &OK));
    if (OK)
    {
      DecodeAdr(&ArgStr[2], MModDir);
      if (AdrType != ModNone)
      {
        CodeLen = 2;
        BAsmCode[0] = (BAsmCode[0] << 5)+ Code;
        BAsmCode[1] = AdrVal;
      }
    }
  }
}

static void DecodeJRR_JRS(Word Code)
{
  if (ChkArgCnt(3, 3))
  {
    Boolean OK;

    BAsmCode[0] = MirrBit(EvalStrIntExpression(&ArgStr[1], UInt3, &OK));
    if (OK)
    {
      BAsmCode[0] = (BAsmCode[0] << 5) + Code;
      DecodeAdr(&ArgStr[2], MModDir);
      if (AdrType != ModNone)
      {
        Integer AdrInt;

        BAsmCode[1] = AdrVal;
        AdrInt = EvalStrIntExpression(&ArgStr[3], UInt16, &OK) - (EProgCounter() + 3);
        if (OK)
        {
          if ((!SymbolQuestionable) && ((AdrInt > 127) || (AdrInt < -128))) WrError(ErrNum_JmpDistTooBig);
          else
          {
            CodeLen = 3;
            BAsmCode[2] = Lo(AdrInt);
          }
        }
      }
    }
  }
}

static void DecodeSFR(Word Code)
{
  UNUSED(Code);
  CodeEquate(SegData, 0, 0xff);
}

static void DecodeASCII_ASCIZ(Word IsZ)
{
  int z, l;
  Boolean OK;
  String s;

  if (ChkArgCnt(0, 0))
  {
    z = 1;
    do
    {
      EvalStrStringExpression(&ArgStr[z], &OK, s);
      if (OK)
      {
        l = strlen(s);
        TranslateString(s, -1);
        if (SetMaxCodeLen(CodeLen + l + IsZ))
        {
          WrError(ErrNum_CodeOverflow); OK = False;
        }
        else
        {
          memcpy(BAsmCode + CodeLen, s, l);
          CodeLen += l;
          if (IsZ)
            BAsmCode[CodeLen++] = 0;
        }
      }
      z++;
    }
    while ((OK) && (z <= ArgCnt));
    if (!OK)
      CodeLen = 0;
  }
}

static void DecodeBYTE(Word Code)
{
  UNUSED(Code);
  strmaxcpy(OpPart.Str, "BYT", STRINGSIZE);
  DecodeMotoPseudo(False);
}

static void DecodeWORD(Word Code)
{
  UNUSED(Code);
  strmaxcpy(OpPart.Str, "ADR", STRINGSIZE);
  DecodeMotoPseudo(False);
}

static void DecodeBLOCK(Word Code)
{
  UNUSED(Code);
  strmaxcpy(OpPart.Str, "DFS", STRINGSIZE);
  DecodeMotoPseudo(False);
}

/*---------------------------------------------------------------------------------*/
/* code table handling */

static void AddFixed(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddRel(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeRel);
}

static void AddALU(char *NName, char *NNameImm, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeALU);
  AddInstTable(InstTable, NNameImm, NCode, DecodeALUImm);
}

static void AddAcc(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeAcc);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(201);
  AddInstTable(InstTable, "LD", 0, DecodeLD);
  AddInstTable(InstTable, "LDI", 0, DecodeLDI);
  AddInstTable(InstTable, "JP", 0x09, DecodeJP_CALL);
  AddInstTable(InstTable, "CALL", 0x01, DecodeJP_CALL);
  AddInstTable(InstTable, "CLR", 0, DecodeCLR);
  AddInstTable(InstTable, "INC", 0, DecodeINC_DEC);
  AddInstTable(InstTable, "DEC", 8, DecodeINC_DEC);
  AddInstTable(InstTable, "SET", 0x1b, DecodeSET_RES);
  AddInstTable(InstTable, "RES", 0x0b, DecodeSET_RES);
  AddInstTable(InstTable, "JRR", 0x03, DecodeJRR_JRS);
  AddInstTable(InstTable, "JRS", 0x13, DecodeJRR_JRS);
  AddInstTable(InstTable, "SFR", 0, DecodeSFR);
  AddInstTable(InstTable, "ASCII", 0, DecodeASCII_ASCIZ);
  AddInstTable(InstTable, "ASCIZ", 1, DecodeASCII_ASCIZ);
  AddInstTable(InstTable, "BYTE", 0, DecodeBYTE);
  AddInstTable(InstTable, "WORD", 0, DecodeWORD);
  AddInstTable(InstTable, "BLOCK", 0, DecodeBLOCK);

  AddFixed("NOP" , 0x04);
  AddFixed("RET" , 0xcd);
  AddFixed("RETI", 0x4d);
  AddFixed("STOP", 0x6d);
  AddFixed("WAIT", 0xed);

  AddRel("JRZ" , 0x04);
  AddRel("JRNZ", 0x00);
  AddRel("JRC" , 0x06);
  AddRel("JRNC", 0x02);

  AddALU("ADD" , "ADDI" , 0x47);
  AddALU("AND" , "ANDI" , 0xa7);
  AddALU("CP"  , "CPI"  , 0x27);
  AddALU("SUB" , "SUBI" , 0xc7);

  AddAcc("COM", 0x002d);
  AddAcc("RLC", 0x00ad);
  AddAcc("SLA", 0xff5f);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------------------*/

static void MakeCode_ST62(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
}

static void InitCode_ST62(void)
{
  WinAssume = 0x40;
}

static Boolean IsDef_ST62(void)
{
  return (Memo("SFR"));
}

static void SwitchFrom_ST62(void)
{
  DeinitFields();
}

static Boolean ChkMoreOneArg(void)
{
  return (ArgCnt > 1);
}

static void SwitchTo_ST62(void)
{
  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupiedFnc = ChkMoreOneArg;

  PCSymbol = "PC"; HeaderID = 0x78; NOPCode = 0x04;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = (MomCPU < CPUST6220) ? 0xfff : 0x7ff;
  Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 0;
  SegLimits[SegData] = 0xff;

  pASSUMERecs = ASSUME62s;
  ASSUMERecCnt = ASSUME62Count;

  MakeCode = MakeCode_ST62; IsDef = IsDef_ST62;
  SwitchFrom = SwitchFrom_ST62; InitFields();
}

void codest6_init(void)
{
  CPUST6210 = AddCPU("ST6210", SwitchTo_ST62);
  CPUST6215 = AddCPU("ST6215", SwitchTo_ST62);
  CPUST6220 = AddCPU("ST6220", SwitchTo_ST62);
  CPUST6225 = AddCPU("ST6225", SwitchTo_ST62);

  AddInitPassProc(InitCode_ST62);
}
