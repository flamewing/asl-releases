/* codeace.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul ACE-Familie                                            */
/*                                                                           */
/* Historie: 14. 8.1999 Grundsteinlegung                                     */
/*           15. 8.1999 Datensegment immer 256 Byte                          */
/*                      angekuendigte Typen                                  */
/*                      nur noch Intel-Pseudos                               */
/*           16. 8.1999 Fehler beseitigt                                     */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "headids.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"

#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModX 1
#define MModX (1 << ModX)
#define ModXInd 2
#define MModXInd (1 << ModXInd)
#define ModXDisp 3
#define MModXDisp (1 << ModXDisp)
#define ModDir 4
#define MModDir (1 << ModDir)
#define ModImm 5
#define MModImm (1 << ModImm)

#define FixedOrderCnt 9
#define AriOrderCnt 7
#define SingOrderCnt 3
#define BitOrderCnt 6

typedef struct
        {
         Byte Code;
        } FixedOrder;

typedef struct
        {
         Byte ImmCode, DirCode, IndCode, DispCode;
        } AriOrder;

typedef struct
        {
         Byte AccCode, XCode, DirCode;
        } SingOrder;

typedef struct
        {
         Byte AccCode, XIndCode, DirCode;
        } BitOrder;

enum {D_CPUACE1001, D_CPUACE1101, D_CPUACE2101,
      D_CPUACE1202, D_CPUACE2202, D_CPUACE2404};

static CPUVar CPUACE1001, CPUACE1101, CPUACE2101,
              CPUACE1202, CPUACE2202, CPUACE2404;

static PInstTable InstTable;
static FixedOrder *FixedOrders;
static AriOrder *AriOrders;
static SingOrder *SingOrders;
static BitOrder *BitOrders;

static ShortInt AdrMode;
static Byte AdrVal;
static Word WAdrVal;
static Boolean BigFlag, OpSize;
static IntType XType, CodeType;

/*---------------------------------------------------------------------------*/

        static void ChkAdr(Word Mask)
BEGIN
   if ((AdrMode == ModXDisp) AND (MomCPU < CPUACE1202))
    BEGIN
     AdrMode = ModNone; WrError(1505);
    END
   else if ((AdrMode != ModNone) AND ((Mask & (1 << AdrMode)) == 0))
    BEGIN
     AdrMode = ModNone; WrError(1350);
    END
END

        static void DecodeAdr(char *Asc, Word Mask)
BEGIN
   Boolean OK, DispOcc, XOcc;
   int l;
   String Part;
   char *p;

   AdrMode = ModNone;

   /* Register ? */

   if (strcasecmp(Asc, "A") == 0)
    AdrMode = ModAcc;

   else if (strcasecmp(Asc, "X") == 0)
    AdrMode = ModX;

   /* immediate ? */

   else if (*Asc == '#')
    BEGIN
     if (OpSize)
      WAdrVal = EvalIntExpression(Asc + 1, XType, &OK);
     else
      AdrVal = EvalIntExpression(Asc + 1, Int8, &OK);
     if (OK) AdrMode = ModImm;
    END

   /* indirekt ? */

   else if (*Asc == '[')
    BEGIN
     l = strlen(Asc) - 1;
     if (Asc[l] != ']') WrError(1350);
     else
      BEGIN
       Asc[l] = '\0'; Asc++;
       DispOcc = XOcc = False;
       while (*Asc != '\0')
        BEGIN
         p = QuotPos(Asc, ',');
         if (p != Nil) *p = '\0';
         strmaxcpy(Part, Asc, 255);
         KillPrefBlanks(Part); KillPostBlanks(Part);
         if (strcasecmp(Part, "X") == 0)
          if (XOcc)
           BEGIN
            WrError(1350); break;
           END
          else XOcc = True;
         else if (DispOcc)
          BEGIN
           WrError(1350); break;
          END
         else
          BEGIN
           if (*Part == '#') strcpy(Part, Part +1);
           AdrVal = EvalIntExpression(Part, SInt8, &OK);
           if (NOT OK) break;
           DispOcc = True;
          END
         if (p == Nil) Asc = "";
         else Asc = p + 1;
        END
       if (*Asc == '\0')
        AdrMode = (DispOcc && (AdrVal != 0)) ? ModXDisp : ModXInd;
      END
    END
    
   /* direkt */

   else
    BEGIN
     if (OpSize)
      WAdrVal = EvalIntExpression(Asc, CodeType, &OK);
     else
      AdrVal = EvalIntExpression(Asc, UInt6, &OK);
     if (OK) AdrMode = ModDir;
    END

   ChkAdr(Mask);
END

/*---------------------------------------------------------------------------*/

        static void DecodeFixed(Word Index)
BEGIN
   FixedOrder *porder = FixedOrders + Index;

   if (ArgCnt != 0) WrError(1110);
   else
    BEGIN
     BAsmCode[0] = porder->Code;
     CodeLen = 1;
    END
END

        static void DecodeAri(Word Index)
BEGIN
   AriOrder *porder = AriOrders + Index;

   if (ArgCnt != 2) WrError(1110);
   else  
    BEGIN
     DecodeAdr(ArgStr[1], MModAcc);
     if (AdrMode != ModNone)
      BEGIN
       DecodeAdr(ArgStr[2], MModImm | MModDir | MModXInd | MModXDisp);
       switch (AdrMode)
        BEGIN
         case ModImm:
          BAsmCode[0] = porder->ImmCode; BAsmCode[1] = AdrVal;
          CodeLen = 2;
          break;
         case ModDir:
          BAsmCode[0] = porder->DirCode; BAsmCode[1] = AdrVal;
          CodeLen = 2;
          break;
         case ModXInd:
          BAsmCode[0] = porder->IndCode;
          CodeLen = 1;
          break;
         case ModXDisp:
          BAsmCode[0] = porder->DispCode; BAsmCode[1] = AdrVal;
          CodeLen = 2;
          break;
        END
      END
    END
END

        static void DecodeSing(Word Index)
BEGIN
   SingOrder *porder = SingOrders + Index;

   if (ArgCnt != 1) WrError(1110);
   else  
    BEGIN
     DecodeAdr(ArgStr[1], MModDir | MModX | MModAcc);
     switch (AdrMode)
      BEGIN
       case ModDir:
        BAsmCode[0] = porder->DirCode; BAsmCode[1] = AdrVal;
        CodeLen = 2;
        break;
       case ModAcc:
        BAsmCode[0] = porder->AccCode;
        CodeLen = 1;
        break;
       case ModX:
        BAsmCode[0] = porder->XCode;
        CodeLen = 1;
        break;
      END
    END
END

        static void DecodeBit(Word Index)
BEGIN
   Byte Bit, Mask;
   BitOrder *porder = BitOrders + Index;
   Boolean OK;

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     Bit = EvalIntExpression(ArgStr[1], UInt8, &OK);
     if (OK)
      BEGIN
       Mask = 0;
       if (porder->AccCode != 0xff) Mask |= MModAcc;
       if (porder->XIndCode != 0xff) Mask |= MModXInd;
       if (porder->DirCode != 0xff) Mask |= MModDir;
       DecodeAdr(ArgStr[2], Mask);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          BAsmCode[0] = porder->AccCode;
          if (porder->AccCode & 7)
           BEGIN
            BAsmCode[1] = 1 << Bit;
            if (porder->AccCode & 1) BAsmCode[1] = 255 - BAsmCode[1];
            CodeLen = 2;
           END
          else
           BEGIN
            BAsmCode[0] |= Bit; CodeLen = 1;
           END
          break;
         case ModXInd:
          BAsmCode[0] = porder->XIndCode + Bit;
          CodeLen = 1;
          break;
         case ModDir:
          BAsmCode[0] = porder->DirCode + Bit; BAsmCode[1] = AdrVal;
          CodeLen = 2;
          break;
        END
      END
    END
END

        static void DecodeIFEQ(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1], MModAcc | MModX | MModDir);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModImm | MModDir | MModXInd | MModXDisp);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x65; BAsmCode[1] = AdrVal;
           CodeLen = 2;
           break;
          case ModDir:
           BAsmCode[0] = 0x56; BAsmCode[1] = AdrVal;
           CodeLen = 2; 
           break;
          case ModXInd:
           BAsmCode[0] = 0x09;
           CodeLen = 1;
           break;
          case ModXDisp:
           BAsmCode[0] = 0x76; BAsmCode[1] = AdrVal;
           CodeLen = 2; 
           break;
         END
        break;
       case ModX:
        OpSize = True;
        DecodeAdr(ArgStr[2], MModImm);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x26;
           BAsmCode[1] = Lo(WAdrVal); BAsmCode[2] = Hi(WAdrVal);
           CodeLen = 3;
           break;
         END
        break;
       case ModDir:
        BAsmCode[1] = AdrVal;
        DecodeAdr(ArgStr[2], MModImm);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x20;
           BAsmCode[2] = AdrVal;
           CodeLen = 3;
           break;
         END
        break;
      END
    END
END

        static void DecodeIFGT(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1], MModAcc | MModX);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModImm | MModDir | MModXInd | MModXDisp);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x67; BAsmCode[1] = AdrVal;
           CodeLen = 2;
           break;
          case ModDir:
           BAsmCode[0] = 0x55; BAsmCode[1] = AdrVal;
           CodeLen = 2; 
           break;
          case ModXInd:
           BAsmCode[0] = 0x0a;
           CodeLen = 1;
           break;
          case ModXDisp:
           BAsmCode[0] = 0x77; BAsmCode[1] = AdrVal;
           CodeLen = 2; 
           break;
         END
        break;
       case ModX:
        OpSize = True;
        DecodeAdr(ArgStr[2], MModImm);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x27;
           BAsmCode[1] = Lo(WAdrVal); BAsmCode[2] = Hi(WAdrVal);
           CodeLen = 3;
           break;
         END
        break;
      END
    END
END

        static void DecodeIFLT(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1], MModX);
     switch (AdrMode)
      BEGIN
       case ModX:
        OpSize = True;
        DecodeAdr(ArgStr[2], MModImm);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x28;
           BAsmCode[1] = Lo(WAdrVal); BAsmCode[2] = Hi(WAdrVal);
           CodeLen = 3;
           break;
         END
        break;
      END
    END
END

        static void DecodeJMPJSR(Word Index)
BEGIN
   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     OpSize = True;
     DecodeAdr(ArgStr[1], MModDir | MModXDisp);
     switch (AdrMode)
      BEGIN
       case ModDir:
        BAsmCode[0] = 0x24 - Index;
        BAsmCode[1] = Lo(WAdrVal); BAsmCode[2] = Hi(WAdrVal);
        CodeLen = 3;
        break;
       case ModXDisp:
        BAsmCode[0] = 0x7e + Index; BAsmCode[1] = AdrVal;
        CodeLen = 2;
        break;
      END
    END
END

        static void DecodeJP(Word Index)
BEGIN
   LongInt Dist;
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     Dist = EvalIntExpression(ArgStr[1], CodeType, &OK) - (EProgCounter() + 1);
     if (OK)
      BEGIN
       if ((NOT SymbolQuestionable) AND ((Dist > 31) OR (Dist < -31))) WrError(1370);
       else
        BEGIN
         if (Dist >= 0) BAsmCode[0] = 0xe0 + Dist;
         else BAsmCode[0] = 0xc0 - Dist;
         CodeLen = 1;
        END 
      END
    END
END

        static void DecodeLD(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1], MModAcc | MModX | MModDir);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModImm | MModDir | MModXInd | MModXDisp);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x51; BAsmCode[1] = AdrVal;
           CodeLen = 2;
           break;
          case ModDir:
           BAsmCode[0] = 0x46; BAsmCode[1] = AdrVal;
           CodeLen = 2; 
           break;
          case ModXInd:
           BAsmCode[0] = 0x0e;
           CodeLen = 1;
           break;
          case ModXDisp:
           BAsmCode[0] = 0x52; BAsmCode[1] = AdrVal;
           CodeLen = 2; 
           break;
         END
        break;
       case ModX:
        OpSize = True;
        DecodeAdr(ArgStr[2], MModImm);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x25;
           BAsmCode[1] = Lo(WAdrVal); BAsmCode[2] = Hi(WAdrVal);
           CodeLen = 3;
           break;
         END
        break;
       case ModDir:
        BAsmCode[1] = AdrVal;
        DecodeAdr(ArgStr[2], MModImm | MModDir);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x21;
           BAsmCode[2] = AdrVal;
           CodeLen = 3;
           break;
          case ModDir:
           BAsmCode[0] = 0x22;
           BAsmCode[2] = BAsmCode[1];
           BAsmCode[1] = AdrVal;
           CodeLen = 3;
           break;
         END
        break;
      END
    END
END

        static void DecodeRotate(Word Index)
BEGIN
   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1], MModAcc | MModDir);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        BAsmCode[0] = 0x15 - Index - Index;
        CodeLen = 1;
        break;
       case ModDir:
        BAsmCode[0] = 0x79 + Index; BAsmCode[1] = AdrVal;
        CodeLen = 2;
        break;
      END
    END
END

        static void DecodeST(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1], MModAcc);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModDir | MModXInd | MModXDisp);
        switch (AdrMode)
         BEGIN
          case ModDir:
           BAsmCode[0] = 0x47; BAsmCode[1] = AdrVal;
           CodeLen = 2; 
           break;
          case ModXInd:
           BAsmCode[0] = 0x11;
           CodeLen = 1;
           break;
          case ModXDisp:
           BAsmCode[0] = 0x40; BAsmCode[1] = AdrVal;
           CodeLen = 2; 
           break;
         END
        break;
      END
    END
END

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
END

        static void AddAri(char *NName, Byte NImm, Byte NDir, Byte NInd, Byte NDisp)
BEGIN
   if (InstrZ >= AriOrderCnt) exit(255);
   AriOrders[InstrZ].ImmCode = NImm;
   AriOrders[InstrZ].DirCode = NDir;
   AriOrders[InstrZ].IndCode = NInd;
   AriOrders[InstrZ].DispCode = NDisp;
   AddInstTable(InstTable, NName, InstrZ++, DecodeAri);
END

        static void AddSing(char *NName, Byte NAcc, Byte NX, Byte NDir)
BEGIN
   if (InstrZ >= SingOrderCnt) exit(255);
   SingOrders[InstrZ].AccCode = NAcc;
   SingOrders[InstrZ].XCode = NX;
   SingOrders[InstrZ].DirCode = NDir;
   AddInstTable(InstTable, NName, InstrZ++, DecodeSing);
END

        static void AddBit(char *NName, Byte NAcc, Byte NIndX, Byte NDir)
BEGIN
   if (InstrZ >= BitOrderCnt) exit(255);
   BitOrders[InstrZ].AccCode = NAcc;
   BitOrders[InstrZ].XIndCode = NIndX;
   BitOrders[InstrZ].DirCode = NDir;
   AddInstTable(InstTable, NName, InstrZ++, DecodeBit);
END

        static void InitFields(void)
BEGIN
   InstTable = CreateInstTable(101);

   FixedOrders = (FixedOrder *) malloc(FixedOrderCnt * sizeof(FixedOrder));
   InstrZ = 0;
   AddFixed("IFC"  ,0x19);  AddFixed("IFNC" ,0x1f);  AddFixed("INTR" ,0x00);
   AddFixed("INVC" ,0x12);  AddFixed("NOP"  ,0x1c);  AddFixed("RC"   ,0x1e);
   AddFixed("RET"  ,0x17);  AddFixed("RETI" ,0x18);  AddFixed("SC"   ,0x1d);

   AriOrders = (AriOrder *) malloc(AriOrderCnt * sizeof(AriOrder));
   InstrZ = 0;
   AddAri("ADC" , 0x60, 0x42, 0x02, 0x70);
   AddAri("ADD" , 0x66, 0x43, 0x03, 0x71);
   AddAri("AND" , 0x61, 0x50, 0x04, 0x72);
   AddAri("IFNE", 0x57, 0x54, 0x0b, 0x78);
   AddAri("OR"  , 0x62, 0x44, 0x05, 0x73);
   AddAri("SUBC", 0x63, 0x53, 0x06, 0x74);
   AddAri("XOR" , 0x64, 0x45, 0x07, 0x75);

   SingOrders = (SingOrder *) malloc(SingOrderCnt * sizeof(SingOrder));
   InstrZ = 0;
   AddSing("CLR" , 0x16, 0x0f, 0x7d);
   AddSing("DEC" , 0x1a, 0x0c, 0x7b);
   AddSing("INC" , 0x1b, 0x0d, 0x7c);

   BitOrders = (BitOrder *) malloc(BitOrderCnt * sizeof(BitOrder));
   InstrZ = 0;
   AddBit("IFBIT", 0xa0, 0xa8, 0x58);
   AddBit("LDC"  , 0xff, 0xff, 0x80);
   AddBit("RBIT" , 0x61, 0xb8, 0x68);
   AddBit("SBIT" , 0x62, 0xb0, 0x48);
   AddBit("STC"  , 0xff, 0xff, 0x88);

   AddInstTable(InstTable, "IFEQ", 0, DecodeIFEQ);
   AddInstTable(InstTable, "IFGT", 0, DecodeIFGT);
   AddInstTable(InstTable, "IFLT", 0, DecodeIFLT);
   AddInstTable(InstTable, "JMP" , 0, DecodeJMPJSR);
   AddInstTable(InstTable, "JSR" , 1, DecodeJMPJSR);
   AddInstTable(InstTable, "JP"  , 0, DecodeJP);
   AddInstTable(InstTable, "LD"  , 0, DecodeLD);
   AddInstTable(InstTable, "RLC" , 0, DecodeRotate);
   AddInstTable(InstTable, "RRC" , 1, DecodeRotate);
   AddInstTable(InstTable, "ST"  , 0, DecodeST);
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(FixedOrders);
   free(AriOrders);
   free(SingOrders);
   free(BitOrders);
END

/*---------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN

   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,0xff);
     return True;
    END;

   return False;
END

        static void MakeCode_ACE(void)
BEGIN
   CodeLen = 0; DontPrint = False; BigFlag = False; OpSize = False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(BigFlag)) return;

   if (NOT LookupInstTable(InstTable, OpPart))
    WrXError(1200,OpPart);
END

        static Boolean IsDef_ACE(void)
BEGIN
   return (Memo("SFR"));
END

        static void SwitchFrom_ACE(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_ACE(void)
BEGIN
   PFamilyDescr Descr;

   TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

   Descr = FindFamilyByName("ACE");
   PCSymbol = "$"; HeaderID = Descr->Id; NOPCode = 0x1c;
   DivideChars = ","; HasAttrs = False;

   ValidSegs = (1 << SegCode) | (1 << SegData);
   Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
   Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 0;
   SegLimits[SegData] = 0xff;

   switch (MomCPU - CPUACE1001)
    BEGIN
     case D_CPUACE1001: case D_CPUACE1101: case D_CPUACE2101:
      SegLimits[SegCode] = 0x3ff;
      CodeType = UInt10;
      XType = UInt11;
      break;
     case D_CPUACE1202: case D_CPUACE2202:
      SegLimits[SegCode] = 0x7ff;
      CodeType = UInt11;
      XType = UInt12;
      break;
     case D_CPUACE2404:
      SegLimits[SegCode] = 0xfff;
      CodeType = UInt12;
      XType = UInt12;
      break;
    END

   MakeCode = MakeCode_ACE; IsDef = IsDef_ACE;
   SwitchFrom = SwitchFrom_ACE; InitFields();
END

        void codeace_init(void)
BEGIN
   CPUACE1001 = AddCPU("ACE1001", SwitchTo_ACE);
   CPUACE1101 = AddCPU("ACE1101", SwitchTo_ACE);
   CPUACE2101 = AddCPU("ACE2101", SwitchTo_ACE);
   CPUACE1202 = AddCPU("ACE1202", SwitchTo_ACE);
   CPUACE2202 = AddCPU("ACE2202", SwitchTo_ACE);
   CPUACE2404 = AddCPU("ACE2404", SwitchTo_ACE);
END
