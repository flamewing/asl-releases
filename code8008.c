/* code8008.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Intel 8008                                                  */
/*                                                                           */
/* Historie:  3.12.1998 Grundsteinlegung                                     */
/*            4.12.1998 FixedOrders begonnen                                 */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "headids.h"
#include "codevars.h"
#include "intpseudo.h"

/*---------------------------------------------------------------------------*/
/* Variablen */

static CPUVar CPU8008, CPU8008New;

static char *RegNames = "ABCDEHLM";

/*---------------------------------------------------------------------------*/
/* Parser */

static Boolean DecodeReg(char *Asc, Byte *pErg)
{
  char *p;

  if (strlen(Asc) != 1) return False;
 
  p = strchr(RegNames, mytoupper(*Asc));
  if (p != NULL) *pErg = p - RegNames;
  return (p != NULL);
}

/*---------------------------------------------------------------------------*/
/* Hilfsdekoder */

static void DecodeFixed(Word Index)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    BAsmCode[0] = Index;
    CodeLen = 1;
  }
}

static void DecodeImm(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;

    BAsmCode[1] = EvalIntExpression(ArgStr[1], Int8, &OK);
    if (OK)
    {
      BAsmCode[0] = Index;
      CodeLen = 2;
    }
  }
}

static void DecodeJmp(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord;

    AdrWord = EvalIntExpression(ArgStr[1], UInt14, &OK);
    if (OK)
    {
      BAsmCode[0] = Index;
      BAsmCode[1] = Lo(AdrWord);
      BAsmCode[2] = Hi(AdrWord) & 0x3f;
      CodeLen = 3;
      ChkSpace(SegCode);
    }
  }
}

static void DecodeRST(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;
    Word AdrWord;
    UNUSED(Index);

    FirstPassUnknown = False;
    AdrWord = EvalIntExpression(ArgStr[1], UInt14, &OK);
    if (FirstPassUnknown) AdrWord &= 0x38;
    if (OK)
    {
      if (ChkRange(AdrWord, 0, 0x38))
      {
        if ((AdrWord & 7) != 0) WrError(1325);
        else
        {
          BAsmCode[0] = AdrWord + 0x05;
          CodeLen = 1;
          ChkSpace(SegCode);
        }
      }
    }
  }
}

static void DecodeINP(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Boolean OK;

    BAsmCode[0] = 0x41 | (EvalIntExpression(ArgStr[1], UInt3, &OK) << 1);
    if (OK)
    {
      CodeLen = 1;
      ChkSpace(SegIO);
    }
  }
}

static void DecodeOUT(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    Byte Addr;
    Boolean OK;

    FirstPassUnknown = FALSE;
    Addr = EvalIntExpression(ArgStr[1], UInt5, &OK);
    if (FirstPassUnknown)
      Addr |= 0x08;
    if (OK)
    {
      if (Addr < 8) WrError(1315);
      else
      {
        BAsmCode[0] = 0x41 | Addr;
        CodeLen = 1;
        ChkSpace(SegIO);
      }
    }
  }
}

static void DecodeMOV(Word Index)
{
  Byte SReg, DReg;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &DReg)) WrXError(1445, ArgStr[1]);
  else if (!DecodeReg(ArgStr[2], &SReg)) WrXError(1445, ArgStr[2]);
  else
  {
    BAsmCode[0] = 0xc0 | (DReg << 3) | SReg;
    CodeLen = 1;
  }
}

static void DecodeMVI(Word Index)
{
  Byte DReg;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &DReg)) WrXError(1445, ArgStr[1]);
  else
  {
    Boolean OK;

    BAsmCode[1] = EvalIntExpression(ArgStr[2], Int8, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x06 | (DReg << 3);
      CodeLen = 2;
    }
  }
}

static void DecodeLXI(Word Index)
{
  Byte DReg;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &DReg)) WrXError(1445, ArgStr[1]);
  else if ((DReg != 1) && (DReg != 3) && (DReg != 5)) WrXError(1445, ArgStr[1]);
  else
  {
    Boolean OK;
    Word Val;

    Val = EvalIntExpression(ArgStr[2], Int16, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x06 | (DReg << 3);
      BAsmCode[1] = Hi(Val);
      BAsmCode[2] = 0x06 | ((DReg + 1) << 3);
      BAsmCode[3] = Lo(Val);
      CodeLen = 4;
    }
  }
}

static void DecodeSingleReg(Word Index)
{
  Byte Reg;

  if (ArgCnt != 1) WrError(1110);
  else if (!DecodeReg(ArgStr[1], &Reg)) WrXError(1445, ArgStr[1]);
  else
  {
    BAsmCode[0] = Lo(Index) | (Reg << Hi(Index));
    CodeLen = 1;
  }
}

/*---------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static char *FlagNames = "CZSP";

static void AddFixed(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddFixeds(char *NName, Byte NCode, int Pos)
{
  char Memo[10], *p;
  int z;

  strcpy(Memo, NName); p = strchr(Memo, '*');
  for (z = 0; z < 8; z++)
  {
    *p = RegNames[z];
    AddFixed(Memo, NCode + (z << Pos));
  }
}

static void AddImm(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeImm);
}

static void AddImms(char *NName, Byte NCode, int Pos)
{
  char Memo[10], *p;
  int z;

  strcpy(Memo, NName); p = strchr(Memo, '*');
  for (z = 0; z < 8; z++)
  {
    *p = RegNames[z];
    AddImm(Memo, NCode + (z << Pos));
  }
}

static void AddJmp(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeJmp);
}

static void AddJmps(char *NName, Byte NCode, int Pos)
{
  char Memo[10], *p;
  int z;
   
  strcpy(Memo, NName); p = strchr(Memo, '*');
  for (z = 0; z < 4; z++) 
  {
    *p = FlagNames[z];  
    AddJmp(Memo, NCode + (z << Pos));
  }
}

static void InitFields(void)
{
  Boolean New = (MomCPU == CPU8008New);

  SetDynamicInstTable(InstTable = CreateInstTable(503));

  AddFixed("HLT", 0x00);
  AddFixed("NOP", 0xc0); /* = MOV A,A */

  AddInstTable(InstTable, New ? "IN" : "INP", 0, DecodeINP);
  AddInstTable(InstTable, "OUT", 0, DecodeOUT);

  AddJmp ("JMP", 0x44);
  if (New)
  {
    AddJmp ("JNC", 0x40);
    AddJmp ("JNZ", 0x48);
    AddJmp ("JP" , 0x50);
    AddJmp ("JPO", 0x58);
    AddJmp ("JM" , 0x70);
    AddJmp ("JPE", 0x78);
  }
  else
  {
    AddJmp ("JS" , 0x70);
    AddJmp ("JP" , 0x78);
    AddJmps("JF*", 0x40, 3);
    AddJmps("JT*", 0x60, 3);
  }
  AddJmp ("JC" , 0x60);
  AddJmp ("JZ" , 0x68);

  if (New)
  {
    AddJmp ("CALL", 0x46);
    AddJmp ("CNC", 0x42);
    AddJmp ("CNZ", 0x4a);
    AddJmp ("CP" , 0x52);
    AddJmp ("CPO", 0x5a);
    AddJmp ("CM" , 0x72);
    AddJmp ("CPE", 0x7a);
  }
  else
  {
    AddJmp ("CAL", 0x46);   
    AddJmp ("CS" , 0x72);
    AddJmp ("CP" , 0x7a);
    AddJmps("CF*", 0x42, 3);
    AddJmps("CT*", 0x62, 3);
  }
  AddJmp ("CC" , 0x62);
  AddJmp ("CZ" , 0x6a);

  AddFixed("RET", 0x07);
  if (New)
  {
    AddFixed("RNC", 0x03);
    AddFixed("RNZ", 0x0b);
    AddFixed("RP" , 0x13);
    AddFixed("RPO", 0x1b);
    AddFixed("RM" , 0x33);
    AddFixed("RPE", 0x3b);
  }
  else
  {
    AddFixed("RFC", 0x03);
    AddFixed("RFZ", 0x0b);
    AddFixed("RFS", 0x13);
    AddFixed("RFP", 0x1b);
    AddFixed("RS" , 0x33);
    AddFixed("RP" , 0x3b);
    AddFixed("RTC", 0x23); AddFixed("RTZ", 0x2b);
    AddFixed("RTS", 0x33); AddFixed("RTP", 0x3b);
  }
  AddFixed("RC" , 0x23);
  AddFixed("RZ" , 0x2b);

  AddInstTable(InstTable, "RST", 0, DecodeRST);

  if (New)
    AddInstTable(InstTable, "MOV", 0, DecodeMOV);
  else
  {
    AddFixeds("L*A", 0xc0, 3); AddFixeds("L*B", 0xc1, 3);
    AddFixeds("L*C", 0xc2, 3); AddFixeds("L*D", 0xc3, 3);
    AddFixeds("L*E", 0xc4, 3); AddFixeds("L*H", 0xc5, 3);
    AddFixeds("L*L", 0xc6, 3); AddFixeds("L*M", 0xc7, 3);
  }

  if (New)
  {
    AddInstTable(InstTable, "MVI", 0, DecodeMVI);
    AddInstTable(InstTable, "LXI", 0, DecodeLXI);
  }
  else
    AddImms("L*I", 0x06, 3);

  if (New)
  {
    AddInstTable(InstTable, "ADD", 0x80, DecodeSingleReg);
    AddInstTable(InstTable, "ADC", 0x88, DecodeSingleReg);
    AddInstTable(InstTable, "SUB", 0x90, DecodeSingleReg);
    AddInstTable(InstTable, "SBB", 0x98, DecodeSingleReg);
    AddInstTable(InstTable, "ANA", 0xa0, DecodeSingleReg);
    AddInstTable(InstTable, "XRA", 0xa8, DecodeSingleReg);
    AddInstTable(InstTable, "ORA", 0xb0, DecodeSingleReg);
    AddInstTable(InstTable, "CMP", 0xb8, DecodeSingleReg);
    AddInstTable(InstTable, "INR", 0x0300, DecodeSingleReg);
    AddInstTable(InstTable, "DCR", 0x0301, DecodeSingleReg);
  }
  else
  {
    AddFixeds("AD*", 0x80, 0);
    AddFixeds("AC*", 0x88, 0);
    AddFixeds("SU*", 0x90, 0);
    AddFixeds("SB*", 0x98, 0);
    AddFixeds("NR*", 0xa0, 0);
    AddFixeds("ND*", 0xa0, 0);
    AddFixeds("XR*", 0xa8, 0);
    AddFixeds("OR*", 0xb0, 0);
    AddFixeds("CP*", 0xb8, 0);
    AddFixeds("IN*", 0x00, 3);
    AddFixeds("DC*", 0x01, 3);
  }

  AddImm ("ADI", 0x04);
  AddImm ("ACI", 0x0c);
  AddImm ("SUI", 0x14);
  AddImm ("SBI", 0x1c);
  AddImm (New ? "ANI" : "NDI", 0x24);
  AddImm ("XRI", 0x2c);
  AddImm ("ORI", 0x34);
  AddImm ("CPI", 0x3c);

  AddFixed ("RLC", 0x02);
  AddFixed ("RRC", 0x0a);
  AddFixed ("RAL", 0x12);
  AddFixed ("RAR", 0x1a);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------------*/
/* Callbacks */

static void MakeCode_8008(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False)) return;

  /* der Rest */

  if (!LookupInstTable(InstTable, OpPart)) WrXError(1200, OpPart);
}

static Boolean IsDef_8008(void)
{
  return False;
}

static void SwitchFrom_8008(void)
{
  DeinitFields();
}

static void SwitchTo_8008(void)
{
  PFamilyDescr FoundDescr;

  FoundDescr = FindFamilyByName("8008");

  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

  PCSymbol = "$"; HeaderID = FoundDescr->Id; NOPCode = 0xc0;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegIO);
  Grans[SegCode ] = 1; ListGrans[SegCode ] = 1; SegInits[SegCode ] = 0;
  SegLimits[SegCode] = 0x3fff;
  Grans[SegIO   ] = 1; ListGrans[SegIO   ] = 1; SegInits[SegIO   ] = 0;
  SegLimits[SegIO] = 7;

  MakeCode = MakeCode_8008; IsDef = IsDef_8008;
  SwitchFrom = SwitchFrom_8008;

  InitFields();
}

/*---------------------------------------------------------------------------*/
/* Initialisierung */

void code8008_init(void)
{
  CPU8008    = AddCPU("8008"   , SwitchTo_8008);
  CPU8008New = AddCPU("8008NEW", SwitchTo_8008);
}
