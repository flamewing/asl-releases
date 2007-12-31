/* code1802.c */
/****************************************************************************/
/* AS                                                                       */
/* Code Generator for Intersil 1802                                         */
/*                                                                          */
/* History: 27. 1.2001 begun                                                */
/*           3. 2.2001 added 1805 instructions                              */
/*                                                                          */
/****************************************************************************/
/* $Id: code1802.c,v 1.4 2007/11/27 11:24:39 alfred Exp $                   */
/****************************************************************************
 * $Log: code1802.c,v $
 * Revision 1.4  2007/11/27 11:24:39  alfred
 * - some instruction fixes
 *
 * Revision 1.3  2005/09/08 16:53:39  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2004/05/29 11:33:00  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 ****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"
#include "intpseudo.h"
#include "headids.h"

/*-------------------------------------------------------------------------*/
/* Variables */

static CPUVar CPU1802, CPU1805;

/*-------------------------------------------------------------------------*/
/* Subroutines */

	static Boolean PutCode(Word Code)
BEGIN
   if (Hi(Code) == 0)
    BEGIN
     BAsmCode[0] = Lo(Code); CodeLen = 1;
     return True;
    END
   else if (MomCPU <= CPU1802)
    BEGIN
     WrError(1500);
     return False;
    END
   else
    BEGIN
     BAsmCode[0] = Hi(Code); BAsmCode[1] = Lo(Code); CodeLen = 2;
     return True;
    END
END

/*-------------------------------------------------------------------------*/
/* Coders */

	static Boolean DecodePseudo(void)
BEGIN
   if (Memo("PORT"))
    BEGIN
     CodeEquate(SegIO, 1, 0x7);
     return True;
    END

   return False;
END

	static void DecodeFixed(Word Opcode)
BEGIN
   if (ArgCnt != 0) WrError(1110);
   else PutCode(Opcode);
END

	static void DecodeReg(Word Opcode)
BEGIN
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else if (PutCode(Opcode))
    BEGIN
     BAsmCode[CodeLen - 1] |= EvalIntExpression(ArgStr[1], UInt4, &OK);
     if (NOT OK) CodeLen = 0;
    END
END

static void DecodeRegNoZero(Word Opcode)
{
  if (ArgCnt != 1) WrError(1110);
  else if (PutCode(Opcode))
  {
    Boolean OK;
    Byte Reg;

    FirstPassUnknown = FALSE;
    Reg = EvalIntExpression(ArgStr[1], UInt4, &OK);
    if (!OK) CodeLen = 0;
    else if ((!FirstPassUnknown) && (0 == Reg))
    {
      WrXError(1445, ArgStr[1]);
      CodeLen = 0;
    }
    else
      BAsmCode[CodeLen - 1] |= Reg;
  }
}

static void DecodeRegImm16(Word Opcode)
{
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (PutCode(Opcode))
  {
    BAsmCode[CodeLen - 1] |= EvalIntExpression(ArgStr[1], UInt4, &OK);
    if (!OK) CodeLen = 0;
    else
    {
      Word Arg = EvalIntExpression(ArgStr[2], Int16, &OK);

      if (!OK) CodeLen = 0;
      else
      {
        BAsmCode[CodeLen++] = Hi(Arg);
        BAsmCode[CodeLen++] = Lo(Arg);
      }
    }
  }
}

	static void DecodeRegLBranch(Word Opcode)
BEGIN
   Boolean OK;
   Word Addr;

   if (ArgCnt != 2) WrError(1110);
   else if (PutCode(Opcode))
    BEGIN
     BAsmCode[CodeLen - 1] |= EvalIntExpression(ArgStr[1], UInt4, &OK);
     if (NOT OK) CodeLen = 0;
     else
      BEGIN
       Addr = EvalIntExpression(ArgStr[2], UInt16, &OK);
       if (NOT OK) CodeLen = 0;
       else
        BEGIN
         ChkSpace(SegCode);
         BAsmCode[CodeLen++] = Hi(Addr);
         BAsmCode[CodeLen++] = Lo(Addr);
        END
      END
    END
END

	static void DecodeImm(Word Opcode)
BEGIN
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else if (PutCode(Opcode))
    BEGIN
     BAsmCode[CodeLen++] = EvalIntExpression(ArgStr[1], Int8, &OK);
     if (NOT OK) CodeLen = 0;
    END
END

	static void DecodeSBranch(Word Opcode)
BEGIN
   Word Addr;
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else if (PutCode(Opcode))
    BEGIN
     Addr = EvalIntExpression(ArgStr[1], UInt16, &OK);
     if (NOT OK) CodeLen = 0;
     else
      BEGIN
       if ((Hi(EProgCounter() + 1) != Hi(Addr)) AND (NOT SymbolQuestionable))
        BEGIN
         WrError(1910);
         CodeLen = 0;
        END
       else
        BEGIN
         ChkSpace(SegCode);
         BAsmCode[CodeLen++] = Lo(Addr);
        END
      END
    END
END

	static void DecodeLBranch(Word Opcode)
BEGIN
   Word Addr;
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else if (PutCode(Opcode))
    BEGIN
     Addr = EvalIntExpression(ArgStr[1], UInt16, &OK);
     if (NOT OK) CodeLen = 0;
      BEGIN
       ChkSpace(SegCode);
       BAsmCode[CodeLen++] = Hi(Addr);
       BAsmCode[CodeLen++] = Lo(Addr);
      END
    END
END

	static void DecodeIO(Word Opcode)
BEGIN
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else  
    BEGIN
     FirstPassUnknown = False;
     BAsmCode[0] = EvalIntExpression(ArgStr[1], UInt3, &OK);
     if (OK)
      BEGIN
       if (FirstPassUnknown) BAsmCode[0] = 1;
       if (BAsmCode[0] == 0) WrError(1315);
       else
        BEGIN
         BAsmCode[0] |= Opcode;
         CodeLen = 1;
        END
      END
    END
END

/*-------------------------------------------------------------------------*/
/* Instruction Hash Table Handling */

	static void InitFields(void)
BEGIN
   InstTable = CreateInstTable(203);

   AddInstTable(InstTable, "LDN"  , 0x0000, DecodeRegNoZero);
   AddInstTable(InstTable, "LDA"  , 0x0040, DecodeReg);
   AddInstTable(InstTable, "LDX"  , 0x00f0, DecodeFixed);
   AddInstTable(InstTable, "LDXA" , 0x0072, DecodeFixed);
   AddInstTable(InstTable, "LDI"  , 0x00f8, DecodeImm);
   AddInstTable(InstTable, "STR"  , 0x0050, DecodeReg);
   AddInstTable(InstTable, "STXD" , 0x0073, DecodeFixed);
   AddInstTable(InstTable, "RLDI" , 0x68c0, DecodeRegImm16);
   AddInstTable(InstTable, "RLXA" , 0x6860, DecodeReg);
   AddInstTable(InstTable, "RSXD" , 0x68a0, DecodeReg);

   AddInstTable(InstTable, "INC"  , 0x0010, DecodeReg);
   AddInstTable(InstTable, "DEC"  , 0x0020, DecodeReg);
   AddInstTable(InstTable, "IRX"  , 0x0060, DecodeFixed);
   AddInstTable(InstTable, "GLO"  , 0x0080, DecodeReg);
   AddInstTable(InstTable, "PLO"  , 0x00a0, DecodeReg);
   AddInstTable(InstTable, "GHI"  , 0x0090, DecodeReg);
   AddInstTable(InstTable, "PHI"  , 0x00b0, DecodeReg);
   AddInstTable(InstTable, "DBNZ" , 0x6820, DecodeRegLBranch);
   AddInstTable(InstTable, "RNX"  , 0x68b0, DecodeReg);

   AddInstTable(InstTable, "OR"   , 0x00f1, DecodeFixed);
   AddInstTable(InstTable, "ORI"  , 0x00f9, DecodeImm);
   AddInstTable(InstTable, "XOR"  , 0x00f3, DecodeFixed);
   AddInstTable(InstTable, "XRI"  , 0x00fb, DecodeImm);
   AddInstTable(InstTable, "AND"  , 0x00f2, DecodeFixed);
   AddInstTable(InstTable, "ANI"  , 0x00fa, DecodeImm);
   AddInstTable(InstTable, "SHR"  , 0x00f6, DecodeFixed);
   AddInstTable(InstTable, "SHRC" , 0x0076, DecodeFixed);
   AddInstTable(InstTable, "RSHR" , 0x0076, DecodeFixed);
   AddInstTable(InstTable, "SHL"  , 0x00fe, DecodeFixed);
   AddInstTable(InstTable, "SHLC" , 0x007e, DecodeFixed);
   AddInstTable(InstTable, "RSHL" , 0x007e, DecodeFixed);

   AddInstTable(InstTable, "ADD"  , 0x00f4, DecodeFixed);
   AddInstTable(InstTable, "ADI"  , 0x00fc, DecodeImm);  
   AddInstTable(InstTable, "ADC"  , 0x0074, DecodeFixed);
   AddInstTable(InstTable, "ADCI" , 0x007c, DecodeImm);
   AddInstTable(InstTable, "SD"   , 0x00f5, DecodeFixed);
   AddInstTable(InstTable, "SDI"  , 0x00fd, DecodeImm);
   AddInstTable(InstTable, "SDB"  , 0x0075, DecodeFixed);
   AddInstTable(InstTable, "SDBI" , 0x007d, DecodeImm);  
   AddInstTable(InstTable, "SM"   , 0x00f7, DecodeFixed);
   AddInstTable(InstTable, "SMI"  , 0x00ff, DecodeImm);  
   AddInstTable(InstTable, "SMB"  , 0x0077, DecodeFixed);
   AddInstTable(InstTable, "SMBI" , 0x007f, DecodeImm);
   AddInstTable(InstTable, "DADD" , 0x68f4, DecodeFixed);
   AddInstTable(InstTable, "DADI" , 0x68fc, DecodeImm);
   AddInstTable(InstTable, "DADC" , 0x6874, DecodeFixed);
   AddInstTable(InstTable, "DACI" , 0x687c, DecodeImm);
   AddInstTable(InstTable, "DSM"  , 0x68f7, DecodeFixed);
   AddInstTable(InstTable, "DSMI" , 0x68ff, DecodeImm);
   AddInstTable(InstTable, "DSMB" , 0x6877, DecodeFixed);
   AddInstTable(InstTable, "DSBI" , 0x687f, DecodeImm);

   AddInstTable(InstTable, "BR"   , 0x0030, DecodeSBranch);
   AddInstTable(InstTable, "NBR"  , 0x0038, DecodeSBranch);
   AddInstTable(InstTable, "BZ"   , 0x0032, DecodeSBranch);
   AddInstTable(InstTable, "BNZ"  , 0x003a, DecodeSBranch);
   AddInstTable(InstTable, "BDF"  , 0x0033, DecodeSBranch);
   AddInstTable(InstTable, "BPZ"  , 0x0033, DecodeSBranch);
   AddInstTable(InstTable, "BGE"  , 0x0033, DecodeSBranch);
   AddInstTable(InstTable, "BNF"  , 0x003b, DecodeSBranch);
   AddInstTable(InstTable, "BM"   , 0x003b, DecodeSBranch);
   AddInstTable(InstTable, "BL"   , 0x003b, DecodeSBranch);
   AddInstTable(InstTable, "BQ"   , 0x0031, DecodeSBranch);
   AddInstTable(InstTable, "BNQ"  , 0x0039, DecodeSBranch);
   AddInstTable(InstTable, "B1"   , 0x0034, DecodeSBranch);
   AddInstTable(InstTable, "BN1"  , 0x003c, DecodeSBranch);
   AddInstTable(InstTable, "B2"   , 0x0035, DecodeSBranch);
   AddInstTable(InstTable, "BN2"  , 0x003d, DecodeSBranch);
   AddInstTable(InstTable, "B3"   , 0x0036, DecodeSBranch);
   AddInstTable(InstTable, "BN3"  , 0x003e, DecodeSBranch);
   AddInstTable(InstTable, "B4"   , 0x0037, DecodeSBranch);
   AddInstTable(InstTable, "BN4"  , 0x003f, DecodeSBranch);
   AddInstTable(InstTable, "BCI"  , 0x683e, DecodeSBranch);
   AddInstTable(InstTable, "BXI"  , 0x683f, DecodeSBranch);

   AddInstTable(InstTable, "LBR"  , 0x00c0, DecodeLBranch);
   AddInstTable(InstTable, "NLBR" , 0x00c8, DecodeLBranch);
   AddInstTable(InstTable, "LBZ"  , 0x00c2, DecodeLBranch);
   AddInstTable(InstTable, "LBNZ" , 0x00ca, DecodeLBranch);
   AddInstTable(InstTable, "LBDF" , 0x00c3, DecodeLBranch);
   AddInstTable(InstTable, "LBNF" , 0x00cb, DecodeLBranch);
   AddInstTable(InstTable, "LBQ"  , 0x00c1, DecodeLBranch);
   AddInstTable(InstTable, "LBNQ" , 0x00c9, DecodeLBranch);

   AddInstTable(InstTable, "SKP"  , 0x0038, DecodeFixed);
   AddInstTable(InstTable, "LSKP" , 0x00c8, DecodeFixed);
   AddInstTable(InstTable, "LSZ"  , 0x00ce, DecodeFixed);
   AddInstTable(InstTable, "LSNZ" , 0x00c6, DecodeFixed);
   AddInstTable(InstTable, "LSDF" , 0x00cf, DecodeFixed);
   AddInstTable(InstTable, "LSNF" , 0x00c7, DecodeFixed);
   AddInstTable(InstTable, "LSQ"  , 0x00cd, DecodeFixed);
   AddInstTable(InstTable, "LSNQ" , 0x00c5, DecodeFixed);
   AddInstTable(InstTable, "LSIE" , 0x00cc, DecodeFixed);

   AddInstTable(InstTable, "IDL"  , 0x0000, DecodeFixed);
   AddInstTable(InstTable, "NOP"  , 0x00c4, DecodeFixed);
   AddInstTable(InstTable, "SEP"  , 0x00d0, DecodeReg);
   AddInstTable(InstTable, "SEX"  , 0x00e0, DecodeReg);
   AddInstTable(InstTable, "SEQ"  , 0x007b, DecodeFixed);
   AddInstTable(InstTable, "REQ"  , 0x007a, DecodeFixed);
   AddInstTable(InstTable, "SAV"  , 0x0078, DecodeFixed);
   AddInstTable(InstTable, "MARK" , 0x0079, DecodeFixed);
   AddInstTable(InstTable, "RET"  , 0x0070, DecodeFixed);
   AddInstTable(InstTable, "DIS"  , 0x0071, DecodeFixed);
   AddInstTable(InstTable, "LDC"  , 0x6806, DecodeFixed);
   AddInstTable(InstTable, "GEC"  , 0x6808, DecodeFixed);
   AddInstTable(InstTable, "STPC" , 0x6800, DecodeFixed);
   AddInstTable(InstTable, "DTC"  , 0x6801, DecodeFixed);
   AddInstTable(InstTable, "STM"  , 0x6807, DecodeFixed);
   AddInstTable(InstTable, "SCM1" , 0x6805, DecodeFixed);
   AddInstTable(InstTable, "SCM2" , 0x6803, DecodeFixed);
   AddInstTable(InstTable, "SPM1" , 0x6804, DecodeFixed);
   AddInstTable(InstTable, "SPM2" , 0x6802, DecodeFixed);
   AddInstTable(InstTable, "ETQ"  , 0x6809, DecodeFixed);

   AddInstTable(InstTable, "XIE"  , 0x680a, DecodeFixed);
   AddInstTable(InstTable, "XID"  , 0x680b, DecodeFixed);
   AddInstTable(InstTable, "CIE"  , 0x680c, DecodeFixed);
   AddInstTable(InstTable, "CID"  , 0x680d, DecodeFixed);
   AddInstTable(InstTable, "DSAV" , 0x6876, DecodeFixed);

   AddInstTable(InstTable, "OUT"  , 0x0060, DecodeIO);
   AddInstTable(InstTable, "INP"  , 0x0068, DecodeIO);

   AddInstTable(InstTable, "SCAL" , 0x6880, DecodeReg);
   AddInstTable(InstTable, "SRET" , 0x6890, DecodeReg);
END

	static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
END

/*-------------------------------------------------------------------------*/
/* Interface to Upper Level */

        static void MakeCode_1802(void)
BEGIN
   CodeLen = 0; DontPrint = False;

   /* to be ignored */

   if (*OpPart == '\0') return;

   /* Pseudo Instructions */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(True)) return;

   /* search */

   if (NOT LookupInstTable(InstTable, OpPart)) WrXError(1200, OpPart);
END

	static Boolean IsDef_1802(void)
BEGIN
   return Memo("PORT");
END

	static void SwitchFrom_1802(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_1802(void)
BEGIN
   PFamilyDescr FoundDescr;

   FoundDescr = FindFamilyByName("1802");

   TurnWords = FALSE;  ConstMode = ConstModeIntel; SetIsOccupied = False;

   PCSymbol = "$"; HeaderID = FoundDescr->Id; NOPCode = 0xc4;
   DivideChars = ","; HasAttrs = False;

   ValidSegs = (1 << SegCode) | (1 << SegIO);
   Grans[SegCode ] = 1; ListGrans[SegCode ] = 1; SegInits[SegCode ] = 0;
   SegLimits[SegCode ] = 0xffff;
   Grans[SegIO   ] = 1; ListGrans[SegIO   ] = 1; SegInits[SegIO   ] = 1;
   SegLimits[SegIO   ] = 0x7;

   MakeCode = MakeCode_1802; IsDef = IsDef_1802;

   InitFields(); SwitchFrom = SwitchFrom_1802;
END

/*-------------------------------------------------------------------------*/
/* Module Initialization */

	void code1802_init(void)
BEGIN
   CPU1802 = AddCPU("1802", SwitchTo_1802);
   CPU1805 = AddCPU("1805", SwitchTo_1802);
END
