/* codefmc16.c */ 
/****************************************************************************/
/* AS, C-Version                                                            */
/*                                                                          */
/* Codegenerator fuer Fujitsu-F2MC16L-Prozessoren                           */
/*                                                                          */
/* Historie: 19.11.1999 Grundsteinlegung, einfache Befehle                  */
/*                      Adressparser, ADD                                   */
/*           20.11.1999 SUB AND OR XOR ADDC SUBC                            */
/*           24.11.1999 A... fertig                                         */
/*           27.11.1999 C... fertig                                         */
/*            1. 1.2000 Befehle durch                                       */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                */
/*           14. 1.2001 silenced warnings about unused parameters           */
/*                                                                          */
/* TODO: PC-relativ Displacements berechnen                                 */
/*       Registersymbole                                                    */
/*       explizite Displacement-Laengenangaben (Adressen, ADDSP)            */
/****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "asmallg.h"
#include "codepseudo.h"
#include "codevars.h"
#include "asmitree.h"
#include "headids.h"

/*--------------------------------------------------------------------------*/
/* Definitionen */

#define FixedOrderCnt 14
#define ALU8OrderCnt 2
#define Log8OrderCnt 3
#define AccOrderCnt 2
#define ShiftOrderCnt 11
#define BranchOrderCnt 21
#define IncDecOrderCnt 6
#define MulDivOrderCnt 8
#define SegOrderCnt 10
#define StringOrderCnt 6

typedef struct
        {
          Byte Code;
        } FixedOrder;

typedef struct
        {
          Byte Code;
          Boolean MayOne;
        } ShiftOrder;

typedef struct
        {
          Byte Code;
          Word AccCode;
          Boolean MayOne;
        } MulDivOrder;

#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModReg 1
#define MModReg (1 << ModReg)
#define ModMem 2
#define MModMem (1 << ModMem)
#define ModDir 3
#define MModDir (1 << ModDir)
#define ModImm 4
#define MModImm (1 << ModImm)
#define ModCCR 5
#define MModCCR (1 << ModCCR)
#define ModIO 6
#define MModIO (1 << ModIO)
#define ModSeg 7
#define MModSeg (1 << ModSeg)
#define ModIAcc 8
#define MModIAcc (1 << ModIAcc)
#define ModRDisp 9
#define MModRDisp (1 << ModRDisp)
#define ModSpec 10
#define MModSpec (1 << ModSpec)
#define ModRP 11
#define MModRP (1 << ModRP)
#define ModILM 12
#define MModILM (1 << ModILM)
#define ModSP 13
#define MModSP (1 << ModSP)

#define ABSMODE 0x1f

static CPUVar CPU90500;

static PInstTable InstTable;
static FixedOrder *FixedOrders;
static FixedOrder *ALU8Orders;
static FixedOrder *Log8Orders;
static FixedOrder *AccOrders;
static ShiftOrder *ShiftOrders;
static FixedOrder *BranchOrders;
static FixedOrder *IncDecOrders;
static MulDivOrder *MulDivOrders;
static FixedOrder *SegOrders;
static FixedOrder *StringOrders;

static char *BankNames[4] = {"PCB", "DTB", "ADB", "SPB"};

static Byte AdrVals[5], AdrPart, NextDataSeg;
static LongWord CurrBank;
static ShortInt AdrMode, OpSize;

static LongInt Reg_PCB, Reg_DTB, Reg_ADB, Reg_USB, Reg_SSB, Reg_DPR;

static SimpProc SaveInitProc;

/*--------------------------------------------------------------------------*/
/* Adressdekoder */

        static void SetOpSize(ShortInt NewSize)
BEGIN
   if (OpSize == -1)
    OpSize = NewSize;
   else if (OpSize != NewSize)
    BEGIN
     WrError(1131);
     AdrMode = ModNone; AdrCnt = 0;
    END 
END

        static Boolean DecodeAdr(char *Asc, int Mask)
BEGIN
   Integer AdrVal;
   LongWord ImmVal;
   Boolean OK;
   int z;
   static char *SpecNames[7] = {"DTB", "ADB", "SSB", "USB", "DPR", "\a", "PCB"};

   AdrMode = ModNone; AdrCnt = 0;

   /* 1. Sonderregister: */

   if (strcasecmp(Asc, "A") == 0)
    BEGIN
     AdrMode = ModAcc;
     goto found;
    END

   if (strcasecmp(Asc, "CCR") == 0)
    BEGIN
     AdrMode = ModCCR;
     goto found;
    END

   if (strcasecmp(Asc, "ILM") == 0)
    BEGIN
     AdrMode = ModILM;
     goto found;
    END

   if (strcasecmp(Asc, "RP") == 0)
    BEGIN
     AdrMode = ModRP;
     goto found;
    END

   if (strcasecmp(Asc, "SP") == 0)
    BEGIN
     AdrMode = ModSP;
     goto found;
    END

   if (Mask & MModSeg)
    for (z = 0; z < sizeof(BankNames) / sizeof(char *); z++)
     if (strcasecmp(Asc, BankNames[z]) == 0)
      BEGIN
       AdrMode = ModSeg;
       AdrPart = z;
       goto found;
      END

   if (Mask & MModSpec)
    for (z = 0; z < sizeof(SpecNames) / sizeof(char *); z++)
     if (strcasecmp(Asc, SpecNames[z]) == 0)
      BEGIN
       AdrMode = ModSpec;
       AdrPart = z;
       goto found;
      END

   /* 2. Register: */

   if (toupper(*Asc) == 'R')
    switch(toupper(Asc[1]))
     BEGIN
      case 'W':
       if ((Asc[3] == '\0') && (Asc[2] >= '0') && (Asc[2] <= '7'))
        BEGIN
         AdrPart = Asc[2] - '0';
         AdrMode = ModReg;
         SetOpSize(1);
         goto found;
        END
       break;
      case 'L':
       if ((Asc[3] == '\0') && (Asc[2] >= '0') && (Asc[2] <= '3'))
        BEGIN
         AdrPart = (Asc[2] - '0') << 1;
         AdrMode = ModReg;
         SetOpSize(2);
         goto found;
        END
       break;
      case '0': case '1': case '2': case '3': 
      case '4': case '5': case '6': case '7': 
       if (Asc[2] == '\0')
        BEGIN
         AdrPart = Asc[1] - '0';
         AdrMode = ModReg;
         SetOpSize(0);
         goto found;
        END
     END

   /* 3. 32-Bit-Register indirekt: */

   if ((*Asc == '(') AND (toupper(Asc[1]) == 'R') AND (toupper(Asc[2]) == 'L') AND
       (Asc[3] >= '0') AND (Asc[3] <= '3') AND (Asc[4] == ')') AND (Asc[5] == '\0'))
    BEGIN
     AdrPart = ((Asc[3] - '0') << 1) + 1;
     AdrMode = ModMem;
     goto found;
    END

   /* 4. immediate: */

   else if (*Asc == '#')
    BEGIN
     if (OpSize == -1) WrError(1132);
     else
      BEGIN
       ImmVal = EvalIntExpression(Asc + 1, (OpSize == 2) ? Int32 : ((OpSize == 1) ? Int16 : Int8), &OK);
       if (OK)
        BEGIN
         AdrMode = ModImm;
         AdrVals[AdrCnt++] = ImmVal & 0xff;
         if (OpSize >= 1)
          AdrVals[AdrCnt++] = (ImmVal >> 8) & 0xff;
         if (OpSize >= 2)
          BEGIN
           AdrVals[AdrCnt++] = (ImmVal >> 16) & 0xff;
           AdrVals[AdrCnt++] = (ImmVal >> 24) & 0xff; 
          END
        END
      END

     goto found;
    END

   /* 5. indirekt: */

   if (*Asc == '@')
    BEGIN
     Asc++;

     /* Akku-indirekt: */

     if (strcasecmp(Asc, "A") == 0)
      BEGIN
       AdrMode = ModIAcc;
      END

     /* PC-relativ: */

     else if (strncasecmp(Asc, "PC", 2) == 0)
      BEGIN
       AdrPart = 0x1e; Asc += 2;
       if ((*Asc == '+') OR (*Asc == '-') OR (isspace(*Asc)))
        BEGIN
         AdrVal = EvalIntExpression(Asc, SInt16, &OK);
         if (OK)
          BEGIN
           AdrVals[0] = AdrVal & 0xff;
           AdrVals[1] = (AdrVal >> 8) & 0xff;
           AdrCnt = 2;
           AdrMode = ModMem;
          END
        END
       else if (*Asc == '\0')
        BEGIN
         AdrVals[0] = AdrVals[1] = 0;
         AdrCnt = 2;
         AdrMode = ModMem;
        END
       else
        WrXError(1445, Asc - 2);
      END

     /* base register, 32 bit: */

     else if ((toupper(*Asc) == 'R') AND (toupper(Asc[1]) == 'L') AND
              (Asc[2] >= '0') AND (Asc[2] <= '3'))
      BEGIN
       AdrVal = EvalIntExpression(Asc + 3, SInt8, &OK);
       if (OK)
        BEGIN
         AdrVals[0] = AdrVal & 0xff;
         AdrCnt = 1;
         AdrPart = Asc[2] - '0';
         AdrMode = ModRDisp;
        END
      END

     /* base register, 16 bit: */

     else if ((toupper(*Asc) == 'R') AND (toupper(Asc[1]) == 'W') AND
              (Asc[2] >= '0') AND (Asc[2] <= '7'))
      BEGIN
       AdrPart = Asc[2] - '0';
       Asc += 3;
       switch (*Asc)
        BEGIN
         case '\0':                          /* no displacement             */
          if (AdrPart < 4)                   /* RW0..RW3 directly available */
           BEGIN
            AdrPart += 8;
            AdrMode = ModMem;
           END
          else                               /* dummy disp for RW4..RW7     */
           BEGIN
            AdrPart += 0x10;
            AdrVals[0] = 0; AdrCnt = 1;
            AdrMode = ModMem;
           END
          break;
         case '+':
          if (Asc[1] == '\0')                /* postincrement               */
           BEGIN
            if (AdrPart > 3) WrError(1445);  /* only allowed for RW0..RW3   */
            else
             BEGIN
              AdrPart += 0x0c;
              AdrMode = ModMem;
             END
            break;
           END                               /* run into disp part otherwise*/
         case ' ': case '\t': case '-':
          while (isspace(*Asc))              /* skip leading spaces         */
           Asc++;
          if (strcasecmp(Asc, "+RW7") == 0)  /* base + RW7 as index         */
           BEGIN
            if (AdrPart > 1) WrError(1445);
            else
             BEGIN
              AdrPart += 0x1c;
              AdrMode = ModMem;
             END
           END
          else                               /* numeric index               */
           BEGIN
            AdrVal =                         /* max length depends on base  */
             EvalIntExpression(Asc, (AdrPart > 3) ? SInt8 : SInt16, &OK);
            if (OK)
             BEGIN                           /* optimize length             */
              AdrVals[0] = AdrVal & 0xff;
              AdrCnt = 1;
              AdrMode = ModMem;
              AdrPart |= 0x10;
              if ((AdrVal < -0x80) OR (AdrVal > 0x7f))
               BEGIN
                AdrVals[AdrCnt++] = (AdrVal >> 8) & 0xff;
                AdrPart |= 0x08;
               END
             END
           END
          break;
         default:
          WrError(1350);
        END
      END

     else
      WrXError(1445, Asc);

     goto found;
    END

   /* 6. dann direkt: */

   ImmVal = EvalIntExpression(Asc, UInt24, &OK);
   if (OK)
    BEGIN
      AdrVals[AdrCnt++] = ImmVal & 0xff;
      if (((ImmVal >> 8) == 0) AND (Mask & MModIO))
       AdrMode = ModIO;
      else if ((Lo(ImmVal >> 8) == Reg_DPR) AND ((ImmVal & 0xffff0000) == CurrBank) AND (Mask & MModDir))
       AdrMode = ModDir;
      else
       BEGIN
        AdrVals[AdrCnt++] = (ImmVal >> 8) & 0xff;
        AdrPart = ABSMODE;
        AdrMode = ModMem;
        if ((ImmVal & 0xffff0000) != CurrBank) WrError(110);
       END
    END

found:
   if ((AdrMode != ModNone) AND ((Mask & (1 << AdrMode)) == 0))
    BEGIN
     WrError(1350);
     AdrMode = ModNone; AdrCnt = 0;
    END

   return (AdrMode != ModNone);
END

        static Boolean SplitBit(char *Asc, Byte *Result)
BEGIN
   char *pos;
   Boolean Res = FALSE;

   pos = RQuotPos(Asc, ':');
   if (pos == NULL) WrError(1510);
   else
    BEGIN
     *pos = '\0';
     *Result = EvalIntExpression(pos + 1, UInt3, &Res);
    END

   return Res;
END

        static void CopyVals(int Offset)
BEGIN
   memcpy(BAsmCode + Offset, AdrVals, AdrCnt);
   CodeLen = Offset + AdrCnt;
END

/*--------------------------------------------------------------------------*/
/* Dekoder fuer einzelne Instruktionen */

        static void DecodeFixed(Word Index)
BEGIN
   FixedOrder *POrder = FixedOrders + Index;

   if (ArgCnt != 0) WrError(1110);
   else
    BEGIN
     BAsmCode[0] = POrder->Code;
     CodeLen = 1;
    END
END

        static void DecodeALU8(Word Index)
BEGIN
   FixedOrder *POrder = ALU8Orders + Index;
   int HCnt;

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     SetOpSize(0);
     DecodeAdr(ArgStr[1], MModAcc | MModReg | MModMem);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModReg | MModMem | MModImm | MModDir);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x30 + POrder->Code;
           BAsmCode[1] = AdrVals[0];
           CodeLen = 2;
           break;
          case ModDir:
           BAsmCode[0] = 0x20 + POrder->Code;
           BAsmCode[1] = AdrVals[0];
           CodeLen = 2;
           break;
          case ModReg:
          case ModMem:
           BEGIN
            BAsmCode[0] = 0x74;
            BAsmCode[1] = (POrder->Code << 5) | AdrPart;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
           END
         END
        break;
       case ModReg:
       case ModMem:
        HCnt = AdrCnt;
        BAsmCode[1] = (POrder->Code << 5) | AdrPart;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        if (DecodeAdr(ArgStr[2], MModAcc))
         BEGIN
          BAsmCode[0] = 0x75;
          CodeLen = 2 + HCnt;
         END
        break;
      END
    END
END

        static void DecodeLog8(Word Index)
BEGIN
   FixedOrder *POrder = Log8Orders + Index;
   int HCnt, Mask;

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     SetOpSize(0);
     Mask = MModAcc | MModReg | MModMem;
     if (POrder->Code < 6) Mask |= MModCCR;
     DecodeAdr(ArgStr[1], Mask);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModReg | MModMem | MModImm);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x30 + POrder->Code;
           BAsmCode[1] = AdrVals[0];
           CodeLen = 2;
           break;
          case ModReg:
          case ModMem:
           BEGIN
            BAsmCode[0] = 0x74;
            BAsmCode[1] = (POrder->Code << 5) | AdrPart;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
           END
         END
        break;
       case ModReg:
       case ModMem:
        HCnt = AdrCnt;
        BAsmCode[1] = (POrder->Code << 5) | AdrPart;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        if (DecodeAdr(ArgStr[2], MModAcc))
         BEGIN
          BAsmCode[0] = 0x75;
          CodeLen = 2 + HCnt;
         END
        break;
       case ModCCR:
        if (DecodeAdr(ArgStr[2], MModImm))
         BEGIN
          BAsmCode[0] = 0x20 + POrder->Code;
          BAsmCode[1] = AdrVals[0];
          CodeLen = 2;
         END
      END
    END
END

        static void DecodeCarry8(Word Index)
BEGIN
   if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     if (ArgCnt == 1)
      BEGIN
       BAsmCode[0] = 0x22 + (Index << 4);
       CodeLen = 1;
      END
     else if (DecodeAdr(ArgStr[2], MModReg | MModMem))
      BEGIN
       BAsmCode[0] = 0x74 + Index;
       BAsmCode[1] = AdrPart | 0x40;
       memcpy(BAsmCode + 2, AdrVals, AdrCnt);
       CodeLen = 2 + AdrCnt;
      END
    END
END

        static void DecodeCarry16(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    if (DecodeAdr(ArgStr[2], MModReg | MModMem))
     BEGIN
      BAsmCode[0] = 0x76 + Index;
      BAsmCode[1] = AdrPart | 0x40;
      memcpy(BAsmCode + 2, AdrVals, AdrCnt);
      CodeLen = 2 + AdrCnt;
     END
END

        static void DecodeAcc(Word Index)
BEGIN
   FixedOrder *POrder = AccOrders + Index;

   if (ArgCnt != 1) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     BAsmCode[0] = POrder->Code;
     CodeLen = 1;
    END
END

        static void DecodeShift(Word Index)
BEGIN
   ShiftOrder *POrder = ShiftOrders + Index;

   if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     if (ArgCnt == 1)
      BEGIN
       if (NOT POrder->MayOne) WrError(1110);
       else
        BEGIN
         BAsmCode[0] = POrder->Code;
         CodeLen = 1;
        END
      END
     else
      BEGIN
       SetOpSize(0);
       if (DecodeAdr(ArgStr[2], MModReg))
        BEGIN
         if (AdrPart != 0) WrXError(1445, ArgStr[2]);
         else
          BEGIN
           BAsmCode[0] = 0x6f;
           BAsmCode[1] = POrder->Code;
           CodeLen = 2;
          END
        END
      END
    END
END

        static void DecodeAdd32(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     SetOpSize(2);
     if (DecodeAdr(ArgStr[2], MModImm | MModMem | MModReg))
      switch (AdrMode)
       BEGIN
        case ModImm:
         BAsmCode[0] = 0x18 + Index;
         memcpy(BAsmCode + 1, AdrVals, AdrCnt);
         CodeLen = 1 + AdrCnt;
         break;
        case ModReg:
        case ModMem:
         BAsmCode[0] = 0x70;
         BAsmCode[1] = AdrPart | (Index << 5);
         memcpy(BAsmCode + 2, AdrVals, AdrCnt);
         CodeLen = 2 + AdrCnt;
         break;
       END
    END
END

        static void DecodeLog32(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     if (DecodeAdr(ArgStr[2], MModMem | MModReg))
      BEGIN
       BAsmCode[0] = 0x70;
       BAsmCode[1] = AdrPart | (Index << 5);
       memcpy(BAsmCode + 2, AdrVals, AdrCnt);
       CodeLen = 2 + AdrCnt;
      END
    END
END

        static void DecodeADDSP(Word Index)
BEGIN
   Integer Val;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else if (*ArgStr[1] != '#') WrError(1120);
   else
    BEGIN
     Val = EvalIntExpression(ArgStr[1] + 1, Int16, &OK);
     if (OK)
      BEGIN
       BAsmCode[CodeLen++] = 0x17;
       BAsmCode[CodeLen++] = Val & 0xff;
       if ((Val > 127) OR (Val < -128))
        BEGIN
         BAsmCode[CodeLen++] = (Val >> 8) & 0xff;
         BAsmCode[0] |= 8;
        END
      END
    END
END

        static void DecodeAdd16(Word Index)
BEGIN
   int HCnt;

   if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
   else if (ArgCnt == 1)
    BEGIN
     if (DecodeAdr(ArgStr[1], MModAcc))
      BEGIN
       BAsmCode[0] = 0x28 | Index;
       CodeLen = 1;
      END
    END
   else
    BEGIN
     SetOpSize(1);
     DecodeAdr(ArgStr[1], MModAcc | ((Index != 3) ? (MModMem | MModReg) : 0));
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModImm | MModMem | MModReg);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x38 | Index;
           memcpy(BAsmCode + 1, AdrVals, AdrCnt);
           CodeLen = 1 + AdrCnt;
           break;
          case ModMem:
          case ModReg:
           BAsmCode[0] = 0x76;
           BAsmCode[1] = AdrPart | (Index << 5);
           memcpy(BAsmCode + 2, AdrVals, AdrCnt);
           CodeLen = 2 + AdrCnt;
           break;
         END
        break;
       case ModMem:
       case ModReg:
        BAsmCode[0] = 0x77;
        BAsmCode[1] = AdrPart | (Index << 5);
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        HCnt = 2 + AdrCnt;
        if (DecodeAdr(ArgStr[2], MModAcc))
         CodeLen = HCnt;
        break;
      END
    END
END

        static void DecodeBBcc(Word Index)
BEGIN
   Byte BitPos, HLen;
   LongInt Addr;
   Boolean OK;
   
   if (ArgCnt != 2) WrError(1110);
   else if (SplitBit(ArgStr[1], &BitPos))
    BEGIN
     HLen = 0;
     DecodeAdr(ArgStr[1], MModMem | MModDir | MModIO);
     if (AdrMode != ModNone)
      BEGIN
       BAsmCode[HLen++] = 0x6c;
       switch (AdrMode)
        BEGIN
         case ModDir:
          BAsmCode[HLen++] = Index + 8 + BitPos;
          BAsmCode[HLen++] = AdrVals[0];
          break;
         case ModIO:
          BAsmCode[HLen++] = Index + BitPos;
          BAsmCode[HLen++] = AdrVals[0];
          break;
         case ModMem:
          if (AdrPart != ABSMODE) WrError(1350);
          else
           BEGIN
            BAsmCode[HLen++] = Index + 16 + BitPos;
            memcpy(BAsmCode + HLen, AdrVals, AdrCnt);
            HLen += AdrCnt;
           END
          break;
        END
       if (HLen > 1)
        BEGIN
         Addr = EvalIntExpression(ArgStr[2], UInt24, &OK) - (EProgCounter() + HLen + 1);
         if (OK)
          BEGIN
           if ((NOT SymbolQuestionable) AND ((Addr < -128) OR (Addr > 127))) WrError(1370);
           else
            BEGIN
             BAsmCode[HLen++] = Addr & 0xff;
             CodeLen = HLen;
            END
          END
        END
      END
    END
END

        static void DecodeBranch(Word Index)
BEGIN
   FixedOrder *POrder = BranchOrders + Index;
   LongInt Addr;
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     Addr = EvalIntExpression(ArgStr[1], UInt24, &OK) - (EProgCounter() + 2);
     if (OK)
      BEGIN
       if ((NOT SymbolQuestionable) AND ((Addr < -128) OR (Addr > 127)))WrError(1370);
       else
        BEGIN
         BAsmCode[0] = POrder->Code;
         BAsmCode[1] = Addr & 0xff;
         CodeLen = 2;
        END
      END
    END
END

        static void DecodeJmp16(Word Index)
BEGIN
   LongWord Addr;
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else if (*ArgStr[1] == '@')
    BEGIN
     SetOpSize(1);
     DecodeAdr(ArgStr[1] + 1, MModReg | ((Index == 0) ? MModAcc : 0) | MModMem);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        BAsmCode[0] = 0x61;
        CodeLen = 1;
        break;
       case ModReg:
       case ModMem:
        BAsmCode[0] = 0x73;
        BAsmCode[1] = (Index << 4) | AdrPart;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
        break;
      END
    END
   else
    BEGIN
     Addr = EvalIntExpression(ArgStr[1], UInt24, &OK);
     if (OK)
      BEGIN
       BAsmCode[0] = 0x62 + Index;
       BAsmCode[1] = Addr & 0xff;
       BAsmCode[2] = (Addr >> 8) & 0xff;
       CodeLen = 3;
       if (((Addr >> 16) & 0xff) != Reg_PCB)
        WrError(75);
      END
    END
END

        static void DecodeJmp24(Word Index)
BEGIN
   LongWord Addr;
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else if (*ArgStr[1] == '@')
    BEGIN
     SetOpSize(2);
     if (DecodeAdr(ArgStr[1] + 1, MModReg | MModMem))
      BEGIN
       BAsmCode[0] = 0x71;
       BAsmCode[1] = (Index << 4) | AdrPart;
       memcpy(BAsmCode + 2, AdrVals, AdrCnt);
       CodeLen = 2 + AdrCnt;
      END
    END
   else
    BEGIN
     Addr = EvalIntExpression(ArgStr[1], UInt24, &OK);
     if (OK)
      BEGIN
       BAsmCode[0] = 0x63 + Index;
       BAsmCode[1] = Addr & 0xff;
       BAsmCode[2] = (Addr >> 8) & 0xff;
       BAsmCode[3] = (Addr >> 16) & 0xff;
       CodeLen = 4;
      END
    END
END

        static void DecodeCALLV(Word Index)
BEGIN
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else if (*ArgStr[1] != '#') WrError(1120);
   else
    BEGIN
     BAsmCode[0] = 0xe0 + EvalIntExpression(ArgStr[1] + 1, UInt4, &OK);
     if (OK) CodeLen = 1;
    END
END

        static void DecodeCmpBranch(Word Index)
BEGIN
   LongInt Addr;
   int HCnt;
   Boolean OK;

   if (ArgCnt != 3) WrError(1110);
   else
    BEGIN
     SetOpSize(Index); HCnt = 0;
     if (DecodeAdr(ArgStr[1], MModMem | MModReg | MModAcc))
      BEGIN
       OK = TRUE;
       switch (AdrMode)
        BEGIN
         case ModAcc:
          BAsmCode[HCnt++] = 0x2a + (Index << 4);
          break;
         case ModReg:
         case ModMem:
          if ((AdrPart >= 0x0c) AND (AdrPart <= 0x0f))
           BEGIN
            WrError(1350); OK = FALSE;
           END
          BAsmCode[HCnt++] = 0x70;
          BAsmCode[HCnt++] = 0xe0 - (Index * 0xa0) + AdrPart;
          memcpy(BAsmCode + HCnt, AdrVals, AdrCnt);
          HCnt += AdrCnt;
          break;
        END
       if ((OK) AND (DecodeAdr(ArgStr[2], MModImm)))
        BEGIN
         memcpy(BAsmCode + HCnt, AdrVals, AdrCnt);
         HCnt += AdrCnt;
         Addr = EvalIntExpression(ArgStr[3], UInt24, &OK) - (EProgCounter() + HCnt + 1);
         if (OK)
          BEGIN
           if ((NOT SymbolQuestionable) AND ((Addr > 127) OR (Addr < -128))) WrError(1370);
           else
            BEGIN
             BAsmCode[HCnt++] = Addr & 0xff;
             CodeLen = HCnt;
            END
          END
        END
      END
    END
END

        static void DecodeBit(Word Index)
BEGIN
   Byte BitPos;
   
   if (ArgCnt != 1) WrError(1110);
   else if (SplitBit(ArgStr[1], &BitPos))
    BEGIN
     DecodeAdr(ArgStr[1], MModMem | MModDir | MModIO);
     if (AdrMode != ModNone)
      BEGIN
       BAsmCode[CodeLen++] = 0x6c;
       switch (AdrMode)
        BEGIN
         case ModDir:
          BAsmCode[CodeLen++] = Index + 8 + BitPos;
          BAsmCode[CodeLen++] = AdrVals[0];
          break;
         case ModIO:
          BAsmCode[CodeLen++] = Index + BitPos;
          BAsmCode[CodeLen++] = AdrVals[0];
          break;
         case ModMem:
          if (AdrPart != ABSMODE) WrError(1350);
          else
           BEGIN
            BAsmCode[CodeLen++] = Index + 16 + BitPos;
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
           END
          break;
        END
      END
    END
END

        static void DecodeCMP(Word Index)
BEGIN
   UNUSED(Index);

   if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     if (ArgCnt == 1)
      BEGIN
       BAsmCode[0] = 0x23;
       CodeLen = 1;
      END
     else
      BEGIN
       SetOpSize(0);
       DecodeAdr(ArgStr[2], MModMem | MModReg | MModImm);
       switch (AdrMode)
        BEGIN
         case ModMem:
         case ModReg:
          BAsmCode[0] = 0x74;
          BAsmCode[1] = AdrPart | 0x40;
          memcpy(BAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 2 + AdrCnt;
          break;
         case ModImm:
          BAsmCode[0] = 0x33;
          BAsmCode[1] = AdrVals[0];
          CodeLen = 2;
          break;
        END
      END
    END
END

        static void DecodeDBNZ(Word Index)
BEGIN
   LongInt Addr;
   Boolean OK;

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     SetOpSize(Index);
     if (DecodeAdr(ArgStr[1], MModReg | MModMem))
      BEGIN
       Addr = EvalIntExpression(ArgStr[2], UInt16, &OK) - (EProgCounter() + 3 + AdrCnt);
       if (OK)
        BEGIN
         if ((NOT SymbolQuestionable) AND ((Addr < -128) OR (Addr > 127))) WrError(1370);
         else
          BEGIN
           BAsmCode[0] = 0x74 + (Index << 1);
           BAsmCode[1] = 0xe0 | AdrPart;
           memcpy(BAsmCode + 2, AdrVals, AdrCnt);
           BAsmCode[2 + AdrCnt] = Addr & 0xff;
           CodeLen = 3 + AdrCnt;
          END
        END
      END
    END
END

        static void DecodeIncDec(Word Index)
BEGIN
   static Byte Sizes[3] = {2, 0, 1};

   Index = IncDecOrders[Index].Code;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     SetOpSize(Sizes[(Index & 3) - 1]);
     if (DecodeAdr(ArgStr[1], MModMem | MModReg))
      BEGIN
       BAsmCode[0] = 0x70 | (Index & 15);
       BAsmCode[1] = (Index & 0xf0) | AdrPart;
       memcpy(BAsmCode + 2, AdrVals, AdrCnt);
       CodeLen = 2 + AdrCnt;
      END
    END
END

        static void DecodeMulDiv(Word Index)
BEGIN
   MulDivOrder *POrder = MulDivOrders + Index;

   if ((ArgCnt < 1) OR (ArgCnt > 2)) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     if (ArgCnt == 1)
      BEGIN
       if (POrder->AccCode == 0xfff) WrError(1350);
       else
        BEGIN
         BAsmCode[CodeLen++] = Lo(POrder->AccCode);
         if (Hi(POrder->AccCode) != 0)
          BAsmCode[CodeLen++] = Hi(POrder->AccCode);
        END
      END
     else
      BEGIN
       SetOpSize((POrder->Code >> 5) & 1);
       if (DecodeAdr(ArgStr[2], MModMem | MModReg))
        BEGIN
         BAsmCode[0] = 0x78;
         BAsmCode[1] = POrder->Code | AdrPart;
         memcpy(BAsmCode + 2, AdrVals, AdrCnt);
         CodeLen = 2 + AdrCnt;
        END
      END
    END
END

        static void DecodeSeg(Word Index)
BEGIN
   FixedOrder *POrder = SegOrders + Index;

   if (ArgCnt != 1) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModSeg))
    BEGIN
     BAsmCode[0] = 0x6e;
     BAsmCode[1] = POrder->Code + AdrPart;
     CodeLen = 2;
    END
END

        static void DecodeString(Word Index)
BEGIN
   FixedOrder *POrder = StringOrders + Index;

   if (ArgCnt != 2) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModSeg))
    BEGIN
     BAsmCode[1] = AdrPart << 2;
     if (DecodeAdr(ArgStr[2], MModSeg))
      BEGIN
       BAsmCode[1] += POrder->Code + AdrPart;
       BAsmCode[0] = 0x6e;
       CodeLen = 2; 
      END
    END
END

        static void DecodeINT(Word Index)
BEGIN
   Boolean OK;
   LongWord Addr;
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else if (*ArgStr[1] == '#')
    BEGIN
     BAsmCode[1] = EvalIntExpression(ArgStr[1] + 1, UInt8, &OK);
     if (OK)
      BEGIN
       BAsmCode[0] = 0x68;
       CodeLen = 2;
      END
    END
   else
    BEGIN
     Addr = EvalIntExpression(ArgStr[1], UInt24, &OK);
     if (OK)
      BEGIN
       if ((NOT SymbolQuestionable) AND ((Addr & 0xff0000) != 0xff0000))
         WrError(110);
       BAsmCode[0] = 0x69;
       BAsmCode[1] = Addr & 0xff;
       BAsmCode[2] = (Addr >> 8) & 0xff;
       CodeLen = 3;
      END
    END
END

        static void DecodeINTP(Word Index)
BEGIN
   Boolean OK;
   LongWord Addr;
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     Addr = EvalIntExpression(ArgStr[1], UInt24, &OK);
     if (OK)
      BEGIN
       BAsmCode[0] = 0x6a;
       BAsmCode[1] = Addr & 0xff;
       BAsmCode[2] = (Addr >> 8) & 0xff;
       BAsmCode[3] = (Addr >> 16) & 0xff;
       CodeLen = 4;
      END
    END
END

        static void DecodeJCTX(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else if (*ArgStr[1] != '@') WrError(1350);
   else if (DecodeAdr(ArgStr[1] + 1, MModAcc))
    BEGIN
     BAsmCode[0] = 0x13;
     CodeLen = 1;
    END
END

        static void DecodeLINK(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     SetOpSize(0);
     if (DecodeAdr(ArgStr[1], MModImm))
      BEGIN
       BAsmCode[0] = 0x08;
       BAsmCode[1] = AdrVals[0];
       CodeLen = 2;
      END
   END
END

        static void DecodeMOV(Word Index)
BEGIN
   Byte HPart, HCnt;
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else if (((strcasecmp(ArgStr[1], "@AL") == 0) OR (strcasecmp(ArgStr[1], "@A") == 0))
        AND ((strcasecmp(ArgStr[2], "AH" ) == 0) OR (strcasecmp(ArgStr[2], "T" ) == 0)))
    BEGIN
     BAsmCode[0] = 0x6f; BAsmCode[1] = 0x15;
     CodeLen = 2;
    END
   else
    BEGIN
     SetOpSize(0);
     DecodeAdr(ArgStr[1], MModRP | MModILM | MModAcc | MModDir | MModRDisp | MModIO | MModSpec | MModReg | MModMem);
     switch (AdrMode)
      BEGIN
       case ModRP:
        BEGIN
         if (DecodeAdr(ArgStr[2], MModImm))
          BEGIN
           BAsmCode[0] = 0x0a;
           BAsmCode[1] = AdrVals[0];
           CodeLen = 2;
          END
        END /* 1 = ModRP */
        break;
       case ModILM:
        BEGIN
         if (DecodeAdr(ArgStr[2], MModImm))
          BEGIN
           BAsmCode[0] = 0x1a;
           BAsmCode[1] = AdrVals[0];
           CodeLen = 2;
          END
        END /* 1 = ModILM */
        break;
       case ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2], MModImm | MModIAcc | MModRDisp | MModIO | MModMem | MModReg | MModDir | MModSpec);
         switch (AdrMode)
          BEGIN
           case ModImm:
            BAsmCode[0] = 0x42;
            BAsmCode[1] = AdrVals[0];
            CodeLen = 2;
            break;
           case ModIAcc:
            BAsmCode[0] = 0x6f;
            BAsmCode[1] = 0x05;
            CodeLen = 2;
            break;
           case ModRDisp:
            BAsmCode[0] = 0x6f;
            BAsmCode[1] = 0x40 | (AdrPart << 1);
            BAsmCode[2] = AdrVals[0];
            CodeLen = 3;
            break;
           case ModIO:
            BAsmCode[0] = 0x50;
            BAsmCode[1] = AdrVals[0];
            CodeLen = 2;
            break;
           case ModMem:
            if (AdrPart == ABSMODE) /* 16 bit absolute */
             BEGIN
              BAsmCode[0] = 0x52;
              CodeLen = 1;
             END
            else
             BEGIN
              BAsmCode[0] = 0x72;
              BAsmCode[1] = 0x80 + AdrPart;
              CodeLen = 2;
             END
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
           case ModReg:
            BAsmCode[0] = 0x80 | AdrPart;
            CodeLen = 1;
            break;
           case ModDir:
            BAsmCode[0] = 0x40;
            BAsmCode[1] = AdrVals[0];
            CodeLen = 2;
            break;
           case ModSpec:
            BAsmCode[0] = 0x6f;
            BAsmCode[1] = 0x00 + AdrPart;
            CodeLen = 2;
            break;
          END
        END /* 1 = ModAcc */
        break;
       case ModDir:
        BEGIN
         BAsmCode[1] = AdrVals[0];
         DecodeAdr(ArgStr[2], MModAcc | MModImm | MModReg);
         switch (AdrMode)
          BEGIN
           case ModAcc:
            BAsmCode[0] = 0x41;
            CodeLen = 2;
            break;
           case ModImm:
            BAsmCode[0] = 0x44;
            BAsmCode[2] = AdrVals[0];
            CodeLen = 3;
            break;
           case ModReg:           /* extend to 16 bits */
            BAsmCode[0] = 0x7c;
            BAsmCode[2] = BAsmCode[1];
            BAsmCode[1] = (AdrPart << 5) | ABSMODE;
            BAsmCode[3] = Reg_DPR;
            CodeLen = 4;
            break;
          END
        END /* 1 = ModDir */
        break;
       case ModRDisp:
        BEGIN
         BAsmCode[2] = AdrVals[0];
         DecodeAdr(ArgStr[2], MModAcc);
         switch (AdrMode)
          BEGIN
           case ModAcc:  
            BAsmCode[0] = 0x6f;
            BAsmCode[1] = 0x30 | (AdrPart << 1);
            CodeLen = 3;
            break;
          END
        END /* 1 = ModRDisp */
        break;
       case ModIO:
        BEGIN
         BAsmCode[1] = AdrVals[0];
         DecodeAdr(ArgStr[2], MModAcc | MModImm | MModReg);
         switch (AdrMode)
          BEGIN
           case ModAcc:
            BAsmCode[0] = 0x51;
            CodeLen = 2;
            break;
           case ModImm:
            BAsmCode[0] = 0x54;
            BAsmCode[2] = AdrVals[0];
            CodeLen = 3;
            break;
           case ModReg:           /* extend to 16 bits - will only work when in Bank 0 */
            BAsmCode[0] = 0x7c;
            BAsmCode[2] = BAsmCode[1];
            BAsmCode[1] = (AdrPart << 5) | ABSMODE;
            BAsmCode[3] = 0;
            if (CurrBank != 0) WrError(110);
            CodeLen = 4;
            break;
          END
        END /* 1 = ModIO */
        break;
       case ModSpec:
        if (AdrPart == 6) WrError(1350);
        else
         BEGIN
          BAsmCode[1] = 0x10 + AdrPart;
          DecodeAdr(ArgStr[2], MModAcc);
          switch (AdrMode)
           BEGIN
            case ModAcc:
             BAsmCode[0] = 0x6f;
             CodeLen = 2;
             break;
           END
         END /* 1 = ModSpec */
        break;
       case ModReg:
        BEGIN
         BAsmCode[0] = AdrPart;
         DecodeAdr(ArgStr[2], MModAcc | MModImm | MModReg | MModMem);
         switch (AdrMode)
          BEGIN
           case ModAcc:
            BAsmCode[0] += 0x90;
            CodeLen = 1;
            break;
           case ModImm:
            BAsmCode[0] += 0xa0;
            BAsmCode[1] = AdrVals[0];
            CodeLen = 2;
            break;
           case ModReg:
           case ModMem:
            BAsmCode[1] = (BAsmCode[0] << 5) | AdrPart;
            BAsmCode[0] = 0x7a;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
            break;
          END
        END /* 1 = ModReg */
        break;
       case ModMem:
        BEGIN
         HPart = AdrPart; HCnt = AdrCnt;
         memcpy(BAsmCode + 2, AdrVals, AdrCnt);
         DecodeAdr(ArgStr[2], MModAcc | MModImm | MModReg);
         switch (AdrMode)
          BEGIN
           case ModAcc:
            if (HPart == ABSMODE)
             BEGIN
              memmove(BAsmCode + 1, BAsmCode + 2, 2);
              BAsmCode[0] = 0x53;
              CodeLen = 3;
             END
            else
             BEGIN
              BAsmCode[0] = 0x72;
              BAsmCode[1] = 0xa0 | HPart;
              CodeLen = 2 + HCnt;
             END
            break;
           case ModImm:
            BAsmCode[0] = 0x71;
            BAsmCode[1] = 0xc0 | HPart;
            BAsmCode[2 + HCnt] = AdrVals[0];
            CodeLen = 3 + HCnt;
            break;
           case ModReg:
            BAsmCode[0] = 0x7c;
            BAsmCode[1] = (AdrPart << 5) | HPart;
            CodeLen = 2 + HCnt;
          END
        END /* 1 = ModMem */
        break;
      END
    END
END
        
        static void DecodeMOVB(Word Index)
BEGIN
   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     if (strcasecmp(ArgStr[1], "A") == 0)
      Index = 2;
     else if (strcasecmp(ArgStr[2], "A") == 0)
      Index = 1;
     else
      WrError(1350);
     if ((Index > 0) AND (SplitBit(ArgStr[Index], BAsmCode + 1)))
      if (DecodeAdr(ArgStr[Index], MModDir | MModIO | MModMem))
       BEGIN
        BAsmCode[0] = 0x6c;
        BAsmCode[1] += (2 - Index) << 5;
        switch (AdrMode)
         BEGIN
          case ModDir:
           BAsmCode[1] += 0x08;
           break;
          case ModMem:
           BAsmCode[1] += 0x18;
           if (AdrPart != ABSMODE)
            BEGIN
             WrError(1350);
             AdrCnt = 0;
            END
           break;
         END
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
       END
    END
END

        static void DecodeMOVEA(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     SetOpSize(1);
     DecodeAdr(ArgStr[1], MModAcc | MModReg);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        if (DecodeAdr(ArgStr[2], MModMem | MModReg))
         BEGIN
          BAsmCode[0] = 0x71;
          BAsmCode[1] = 0xe0 | AdrPart;
          memcpy(BAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 2 + AdrCnt;
         END
        break;
       case ModReg:
        BAsmCode[1] = AdrPart << 5;
        if (DecodeAdr(ArgStr[2], MModMem | MModReg))
         BEGIN
          BAsmCode[0] = 0x79;
          BAsmCode[1] += AdrPart;
          memcpy(BAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 2 + AdrCnt;
         END
        break;
      END
    END
END

        static void DecodeMOVL(Word Index)
BEGIN
   Byte HCnt;
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     SetOpSize(2);
     DecodeAdr(ArgStr[1], MModAcc | MModMem | MModReg);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModMem | MModReg | MModImm);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x4b;
           CopyVals(1);
           break;
          case ModReg:
          case ModMem:
           BAsmCode[0] = 0x71;
           BAsmCode[1] = 0x80 | AdrPart;
           CopyVals(2);
         END
        break;
       case ModReg:
       case ModMem:
        BAsmCode[1] = 0xa0 | AdrPart;
        HCnt = AdrCnt;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        if (DecodeAdr(ArgStr[2], MModAcc))
         BEGIN
          BAsmCode[0] = 0x71;
          CodeLen = 2 + HCnt;
         END
        break;
      END
    END
END

        static void DecodeMOVN(Word Index)
BEGIN
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     if (*ArgStr[2] != '#') WrError(1120);
     else
      BEGIN
       BAsmCode[0] = 0xd0 + EvalIntExpression(ArgStr[2] + 1, UInt4, &OK);
       if (OK)
        CodeLen = 1;
      END
    END
END

        static void DecodeMOVW(Word Index)
BEGIN
   Byte HPart, HCnt;
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else if (((strcasecmp(ArgStr[1], "@AL") == 0) OR (strcasecmp(ArgStr[1], "@A") == 0))
        AND ((strcasecmp(ArgStr[2], "AH" ) == 0) OR (strcasecmp(ArgStr[2], "T" ) == 0)))
    BEGIN
     BAsmCode[0] = 0x6f; BAsmCode[1] = 0x1d;
     CodeLen = 2;
    END
   else
    BEGIN
     SetOpSize(1);
     DecodeAdr(ArgStr[1], MModAcc | MModRDisp | MModSP | MModDir | MModIO | MModReg | MModMem);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        DecodeAdr(ArgStr[2], MModImm | MModIAcc | MModRDisp | MModSP | MModIO | MModMem | MModReg | MModDir);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0] = 0x4a;
           CopyVals(1);
           break;
          case ModIAcc:
           BAsmCode[0] = 0x6f;
           BAsmCode[1] = 0x0d;
           CodeLen = 2;
           break;
          case ModRDisp:
           BAsmCode[0] = 0x6f;
           BAsmCode[1] = 0x48 + (AdrPart << 1);
           CopyVals(2);
           break;
          case ModSP:
           BAsmCode[0] = 0x46;
           CodeLen = 1;
           break;
          case ModIO:
           BAsmCode[0] = 0x58;
           CopyVals(1);
           break;
          case ModDir:
           BAsmCode[0] = 0x48;
           CopyVals(1);
           break;
          case ModReg:
           BAsmCode[0] = 0x88 + AdrPart;
           CodeLen = 1;
           break;
          case ModMem:
           if (AdrPart == ABSMODE)
            BEGIN
             BAsmCode[0] = 0x5a;
             CopyVals(1);
            END
           else if ((AdrPart >= 0x10) AND (AdrPart <= 0x17))
            BEGIN
             BAsmCode[0] = 0xa8 + AdrPart;
             CopyVals(1);
            END
           else
            BEGIN
             BAsmCode[0] = 0x73;
             BAsmCode[1] = 0x80 + AdrPart;
             CopyVals(2);
            END
           break;
         END
        break; /* 1 = ModAcc */
       case ModRDisp:
        BAsmCode[1] = 0x38 + (AdrPart << 1);
        BAsmCode[2] = AdrVals[0];
        if (DecodeAdr(ArgStr[2], MModAcc))
         BEGIN
          BAsmCode[0] = 0x6f;
          CodeLen = 3;
         END
        break; /* 1 = ModRDisp */
       case ModSP:
        if (DecodeAdr(ArgStr[2], MModAcc))
         BEGIN
          BAsmCode[0] = 0x47;
          CodeLen = 1;
         END
        break; /* 1 = ModSP */
       case ModDir:
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModAcc | MModImm | MModReg);
        switch (AdrMode)
         BEGIN
          case ModAcc:
           BAsmCode[0] = 0x49;
           CodeLen = 2;
           break;
          case ModImm:           /* extend to 16 bits */
           BAsmCode[0] = 0x73;
           BAsmCode[2] = BAsmCode[1];
           BAsmCode[1] = 0xc0 | ABSMODE;
           BAsmCode[3] = Reg_DPR;
           CopyVals(4);
           break;
          case ModReg:           /* extend to 16 bits */
           BAsmCode[0] = 0x7d;
           BAsmCode[2] = BAsmCode[1];
           BAsmCode[1] = (AdrPart << 5) | ABSMODE;
           BAsmCode[3] = Reg_DPR;
           CodeLen = 4;
           break;
         END
        break; /* 1 = ModDir */
       case ModIO:
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModAcc | MModImm | MModReg);
        switch (AdrMode)
         BEGIN
          case ModAcc:
           BAsmCode[0] = 0x59;
           CodeLen = 2;
           break;
          case ModImm:
           BAsmCode[0] = 0x56;
           CopyVals(1);
           break;
          case ModReg:           /* extend to 16 bits - will only work when in Bank 0 */
           BAsmCode[0] = 0x7d;
           BAsmCode[2] = BAsmCode[1];
           BAsmCode[1] = (AdrPart << 5) | ABSMODE;
           BAsmCode[3] = 0;
           if (CurrBank != 0) WrError(110);
           CodeLen = 4;
           break;
         END
        break; /* 1 = ModIO */
       case ModReg:
        HPart = AdrPart;
        DecodeAdr(ArgStr[2], MModAcc | MModImm | MModReg | MModMem);
        switch (AdrMode)
         BEGIN
          case ModAcc:
           BAsmCode[0] = 0x98 | HPart;
           CodeLen = 1;
           break;
          case ModImm:
           BAsmCode[0] = 0x98 | HPart;
           CopyVals(1);
           break;
          case ModReg:
          case ModMem:
           BAsmCode[0] = 0x7b;
           BAsmCode[1] = (HPart << 5) | AdrPart;
           CopyVals(2);
           break;
         END
        break; /* 1 = ModReg */
       case ModMem:
        HPart = AdrPart; HCnt = AdrCnt;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        DecodeAdr(ArgStr[2], MModAcc | MModImm | MModReg);
        switch (AdrMode)
         BEGIN
          case ModAcc:
           if (HPart == ABSMODE)
            BEGIN
             BAsmCode[0] = 0x5b;
             memmove(BAsmCode + 1, BAsmCode + 2, HCnt);
             CodeLen = 1 + HCnt;
            END
           else if ((HPart >= 0x10) && (HPart <= 0x17))
            BEGIN
             BAsmCode[0] = 0xb8 + AdrPart;
             memmove(BAsmCode + 1, BAsmCode + 2, HCnt);
             CodeLen = 1 + HCnt;
            END
           else
            BEGIN
             BAsmCode[0] = 0x73;
             BAsmCode[1] = 0xa0 | AdrPart;
             CodeLen = 2 + HCnt;
            END
           break;
          case ModImm:
           BAsmCode[0] = 0x73;
           BAsmCode[1] = 0xc0 | HPart;
           CopyVals(2 + HCnt);
           break;
          case ModReg:
           BAsmCode[0] = 0x7d;
           BAsmCode[1] = (AdrPart << 5) | HPart;
           CodeLen = 2 + HCnt;
         END
        break; /* 1 = ModMem */
      END
    END
END

        static void DecodeMOVX(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     SetOpSize(0);
     DecodeAdr(ArgStr[2], MModImm | MModIAcc | MModRDisp | MModDir | MModIO | MModReg | MModMem);
     switch (AdrMode)
      BEGIN
       case ModImm:
        BAsmCode[0] = 0x43;
        CopyVals(1);
        break;
       case ModIAcc:
        BAsmCode[0] = 0x6f;
        BAsmCode[1] = 0x16;
        CodeLen = 2;
        break;
       case ModRDisp:
        BAsmCode[0] = 0x6f;
        BAsmCode[1] = 0x20 | (AdrPart << 1);
        CopyVals(2);
        break;
       case ModDir:
        BAsmCode[0] = 0x45;
        CopyVals(1);
        break;
       case ModIO:
        BAsmCode[0] = 0x55;
        CopyVals(1);
        break;
       case ModReg:
        BAsmCode[0] = 0xb0 + AdrPart;
        CodeLen = 1;
        break;
       case ModMem:
        if (AdrPart == ABSMODE)
         BEGIN
          BAsmCode[0] = 0x57;
          CopyVals(1);
         END
        else if ((AdrPart >= 0x10) AND (AdrPart <= 0x17))
         BEGIN
          BAsmCode[0] = 0xb0 + AdrPart;
          CopyVals(1);
         END
        else
         BEGIN
          BAsmCode[0] = 0x72;
          BAsmCode[1] = 0xc0 | AdrPart;
          CopyVals(2);
         END
        break;
      END
    END
END

        static void DecodeNEGNOT(Word Index)
BEGIN
   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     SetOpSize((OpPart[3] == 'W') ? 1 : 0);
     DecodeAdr(ArgStr[1], MModAcc | MModReg | MModMem);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        BAsmCode[0] = Lo(Index);
        CodeLen = 1;
        break;
       case ModReg:
       case ModMem:
        BAsmCode[0] = 0x75 + (OpSize << 1);
        BAsmCode[1] = Hi(Index) + AdrPart;
        CopyVals(2);
        break;
      END
    END
END

        static void DecodeNRML(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else if (DecodeAdr(ArgStr[1], MModAcc))
    BEGIN
     SetOpSize(0);
     if (DecodeAdr(ArgStr[2], MModReg))
      BEGIN
       if (AdrPart != 0) WrError(1350);
       else
        BEGIN
         BAsmCode[0] = 0x6f;
         BAsmCode[1] = 0x2d;
         CodeLen = 2;
        END
      END
    END
END

        static void DecodeStack(Word Index)
BEGIN
   int z, z2;
   Byte HReg;
   char *p;

   if (ArgCnt == 0) WrError(1110);
   else if ((ArgCnt == 1) AND (strcasecmp(ArgStr[1], "A") == 0))
    BEGIN
     BAsmCode[0] = Index;
     CodeLen = 1;
    END
   else if ((ArgCnt == 1) AND (strcasecmp(ArgStr[1], "AH") == 0))
    BEGIN
     BAsmCode[0] = Index + 1;
     CodeLen = 1;
    END
   else if ((ArgCnt == 1) AND (strcasecmp(ArgStr[1], "PS") == 0))
    BEGIN
     BAsmCode[0] = Index + 2;
     CodeLen = 1;
    END
   else
    BEGIN
     BAsmCode[1] = 0; SetOpSize(1);
     for (z = 1; z <= ArgCnt; z++)
      if ((p = strchr(ArgStr[z], '-')) != NULL)
       BEGIN
        *p = '\0';
        if (!DecodeAdr(ArgStr[z], MModReg)) break;
        HReg = AdrPart;
        if (!DecodeAdr(p + 1, MModReg)) break;
        if (AdrPart >= HReg)
         BEGIN
          for (z2 = HReg; z2 <= AdrPart; z2++)
           BAsmCode[1] |= (1 << z2);
         END
        else
         BEGIN
          for (z2 = HReg; z2 <= 7; z2++)
           BAsmCode[1] |= (1 << z2);
          for (z2 = 0; z2 <= AdrPart; z2++)
           BAsmCode[1] |= (1 << z2);
         END
       END
      else
       BEGIN
        if (!DecodeAdr(ArgStr[z], MModReg)) break;
        BAsmCode[1] |= (1 << AdrPart);
       END
     if (z > ArgCnt)
      BEGIN
       BAsmCode[0] = Index + 3;
       CodeLen = 2;
      END
    END
END

        static void DecodeRotate(Word Index)
BEGIN
   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1], MModAcc | MModReg | MModMem);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        BAsmCode[0] = 0x6f;
        BAsmCode[1] = 0x07 | (Index << 4);
        CodeLen = 2;
        break;
       case ModReg:
       case ModMem:
        BAsmCode[0] = 0x72;
        BAsmCode[1] = (Index << 5) | AdrPart;
        CopyVals(2);
        break;
      END
    END
END

        static void DecodeSBBS(Word Index)
BEGIN
   LongInt Adr;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else if (SplitBit(ArgStr[1], BAsmCode + 1))
    if (DecodeAdr(ArgStr[1], MModMem))
     BEGIN
      if (AdrPart != ABSMODE) WrError(1350);
      else
       BEGIN
        Adr = EvalIntExpression(ArgStr[2], UInt24, &OK) - (EProgCounter() + 5);
        if (OK)
         BEGIN
          if ((NOT SymbolQuestionable) AND ((Adr < -128) OR (Adr > 127))) WrError(1370);
          else
           BEGIN
            BAsmCode[0] = 0x6c;
            BAsmCode[1] += 0xf8;
            CopyVals(2);
            BAsmCode[CodeLen++] = Adr & 0xff;
           END
         END
       END
     END
END

        static void DecodeWBit(Word Index)
BEGIN
   if (ArgCnt != 1) WrError(1110);
   else if (SplitBit(ArgStr[1], BAsmCode + 1))
    if (DecodeAdr(ArgStr[1], MModIO))
     BEGIN
      BAsmCode[0] = 0x6c;
      BAsmCode[1] += Index;
      CopyVals(2);
     END
END

        static void DecodeExchange(Word Index)
BEGIN
   Byte HPart, HCnt;

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     SetOpSize(Index);
     DecodeAdr(ArgStr[1], MModAcc | MModReg | MModMem);
     switch (AdrMode)
      BEGIN
       case ModAcc:
        if (DecodeAdr(ArgStr[2], MModReg | MModMem))
         BEGIN
          BAsmCode[0] = 0x72 + Index;
          BAsmCode[1] = 0xe0 | AdrPart;
          CopyVals(2);
         END
        break;
       case ModReg:
        HPart = AdrPart;
        DecodeAdr(ArgStr[2], MModAcc | MModReg | MModMem);
        switch (AdrMode)
         BEGIN
          case ModAcc:
           BAsmCode[0] = 0x72 + Index;
           BAsmCode[1] = 0xe0 | HPart;
           CodeLen = 2;
           break;
          case ModReg:
          case ModMem:
           BAsmCode[0] = 0x7e + Index;
           BAsmCode[1] = (HPart << 5) | AdrPart;
           CopyVals(2);
           break;
         END
        break;
       case ModMem:
        HPart = AdrPart; HCnt = AdrCnt;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        DecodeAdr(ArgStr[2], MModAcc | MModReg);
        switch (AdrMode)
         BEGIN
          case ModAcc:
           BAsmCode[0] = 0x72 + Index;
           BAsmCode[1] = 0xe0 | HPart;
           CodeLen = 2 + HCnt;
           break;
          case ModReg:
           BAsmCode[0] = 0x7e + Index;
           BAsmCode[1] = (AdrPart << 5) | HPart;
           CodeLen = 2 + HCnt;
           break;
         END
        break;
      END
    END
END

        static void DecodeBank(Word Index)
BEGIN
   if (ArgCnt != 0) WrError(1110);
   else
    BEGIN
     BAsmCode[0] = Index + 4;
     CodeLen = 1;
     NextDataSeg = Index;
    END
END

        static Boolean DecodePseudo(void)
BEGIN
#define ASSUMEF2MC16Count 6
   static ASSUMERec ASSUMEF2MC16s[ASSUMEF2MC16Count] = 
                  {{"PCB"    , &Reg_PCB,     0x00, 0xff, 0x100},
                   {"DTB"    , &Reg_DTB,     0x00, 0xff, 0x100},
                   {"ADB"    , &Reg_ADB,     0x00, 0xff, 0x100},
                   {"USB"    , &Reg_USB,     0x00, 0xff, 0x100},
                   {"SSB"    , &Reg_SSB,     0x00, 0xff, 0x100},
                   {"DPR"    , &Reg_DPR,     0x00, 0xff, 0x100}};

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUMEF2MC16s,ASSUMEF2MC16Count);
     return True;
    END

   return False;
END

/*--------------------------------------------------------------------------*/
/* Codetabellen */

        static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= FixedOrderCnt)
    exit(255);

   FixedOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
END

        static void AddALU8(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= ALU8OrderCnt)
    exit(255);

   ALU8Orders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeALU8);
END

        static void AddLog8(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= Log8OrderCnt)
    exit(255);

   Log8Orders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeLog8);
END

        static void AddAcc(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= AccOrderCnt)
    exit(255);

   AccOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeAcc);
END

        static void AddShift(char *NName, Byte NCode, Byte NMay)
BEGIN
   if (InstrZ >= ShiftOrderCnt)
    exit(255);

   ShiftOrders[InstrZ].Code = NCode;
   ShiftOrders[InstrZ].MayOne = NMay;
   AddInstTable(InstTable, NName, InstrZ++, DecodeShift);
END

        static void AddBranch(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= BranchOrderCnt)
    exit(255);

   BranchOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeBranch);
END

        static void AddIncDec(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= IncDecOrderCnt)
    exit(255);

   IncDecOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeIncDec);
END

        static void AddMulDiv(char *NName, Byte NCode, Word NAccCode)
BEGIN
   if (InstrZ >= MulDivOrderCnt)
    exit(255);

   MulDivOrders[InstrZ].Code = NCode;
   MulDivOrders[InstrZ].AccCode = NAccCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeMulDiv);
END

        static void AddSeg(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= SegOrderCnt)
    exit(255);

   SegOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeSeg);
END

        static void AddString(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= StringOrderCnt)
    exit(255);

   StringOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeString);
END

        static void InitFields(void)
BEGIN
   int z;

   InstTable = CreateInstTable(201);

   FixedOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * FixedOrderCnt);
   InstrZ = 0;
   AddFixed("EXT"    , 0x14); AddFixed("EXTW"   , 0x1c);
   AddFixed("INT9"   , 0x01); AddFixed("NOP"    , 0x00);
   AddFixed("RET"    , 0x67); AddFixed("RETI"   , 0x6b);
   AddFixed("RETP"   , 0x66); AddFixed("SWAP"   , 0x16);
   AddFixed("SWAPW"  , 0x1e); AddFixed("UNLINK" , 0x09);
   AddFixed("ZEXT"   , 0x15); AddFixed("ZEXTW"  , 0x1d);
   AddFixed("CMR"    , 0x10); AddFixed("NCC"    , 0x11);

   ALU8Orders = (FixedOrder*) malloc(sizeof(FixedOrder) * ALU8OrderCnt);
   InstrZ = 0;
   AddALU8("ADD"  , 0); AddALU8("SUB"  , 1);

   Log8Orders = (FixedOrder*) malloc(sizeof(FixedOrder) * Log8OrderCnt);
   InstrZ = 0;
   AddLog8("AND"  , 4); AddLog8("OR"   , 5);
   AddLog8("XOR"  , 6);

   AccOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * AccOrderCnt);
   InstrZ = 0;
   AddAcc("ADDDC"  , 0x02);
   AddAcc("SUBDC"  , 0x12);

   ShiftOrders = (ShiftOrder*) malloc(sizeof(ShiftOrder) * ShiftOrderCnt);
   InstrZ = 0;
   AddShift("ASR"  , 0x2e, FALSE); AddShift("ASRL" , 0x1e, FALSE);
   AddShift("ASRW" , 0x0e, TRUE ); AddShift("LSLW" , 0x0c, TRUE );
   AddShift("LSRW" , 0x0f, TRUE ); AddShift("LSL"  , 0x2c, FALSE);
   AddShift("LSR"  , 0x2f, FALSE); AddShift("LSLL" , 0x1c, FALSE);
   AddShift("LSRL" , 0x1f, FALSE); AddShift("SHLW" , 0x0c, TRUE );
   AddShift("SHRW" , 0x0f, TRUE );

   BranchOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * BranchOrderCnt);
   InstrZ = 0;
   AddBranch("BZ"  , 0xf0); AddBranch("BEQ" , 0xf0); AddBranch("BNZ" , 0xf1); 
   AddBranch("BNE" , 0xf1); AddBranch("BC"  , 0xf2); AddBranch("BLO" , 0xf2); 
   AddBranch("BNC" , 0xf3); AddBranch("BHS" , 0xf3); AddBranch("BN"  , 0xf4); 
   AddBranch("BP"  , 0xf5); AddBranch("BV"  , 0xf6); AddBranch("BNV" , 0xf7); 
   AddBranch("BT"  , 0xf8); AddBranch("BNT" , 0xf9); AddBranch("BLT" , 0xfa); 
   AddBranch("BGE" , 0xfb); AddBranch("BLE" , 0xfc); AddBranch("BGT" , 0xfd); 
   AddBranch("BLS" , 0xfe); AddBranch("BHI" , 0xff); AddBranch("BRA" , 0x60);

   IncDecOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * IncDecOrderCnt);
   InstrZ = 0;
   AddIncDec("INC"  , 0x42); AddIncDec("INCW" , 0x43); AddIncDec("INCL" , 0x41);
   AddIncDec("DEC"  , 0x62); AddIncDec("DECW" , 0x63); AddIncDec("DECL" , 0x61);

   MulDivOrders = (MulDivOrder*) malloc(sizeof(MulDivOrder) * MulDivOrderCnt);
   InstrZ = 0;
   AddMulDiv("MULU", 0x00, 0x0027); AddMulDiv("MULUW", 0x20, 0x002f);
   AddMulDiv("MUL" , 0x40, 0x786f); AddMulDiv("MULW" , 0x60, 0x796f);
   AddMulDiv("DIVU", 0x80, 0x0026); AddMulDiv("DIVUW", 0xa0, 0xffff);
   AddMulDiv("DIV" , 0xc0, 0x7a6f); AddMulDiv("DIVW" , 0xe0, 0xffff);

   SegOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * SegOrderCnt);
   InstrZ = 0;
   AddSeg("SCEQI" , 0x80); AddSeg("SCEQD" , 0x90);
   AddSeg("SCWEQI", 0xa0); AddSeg("SCWEQD", 0xb0);
   AddSeg("FILSI" , 0xc0); AddSeg("FILS"  , 0xc0);
   AddSeg("FILSWI", 0xe0); AddSeg("FILSW" , 0xe0);
   AddSeg("SCEQ"  , 0x80); AddSeg("SCWEQ" , 0xa0);

   StringOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * StringOrderCnt);
   InstrZ = 0;
   AddString("MOVS" , 0x00); AddString("MOVSI" , 0x00); AddString("MOVSD" , 0x10);
   AddString("MOVSW", 0x20); AddString("MOVSWI", 0x20); AddString("MOVSWD", 0x30);

   for (z = 0; z < sizeof(BankNames) / sizeof(*BankNames); z++)
     AddInstTable(InstTable, BankNames[z], z, DecodeBank);

   AddInstTable(InstTable, "ADDC", 0, DecodeCarry8);
   AddInstTable(InstTable, "SUBC", 1, DecodeCarry8);
   AddInstTable(InstTable, "ADDCW",0, DecodeCarry16);
   AddInstTable(InstTable, "SUBCW",1, DecodeCarry16);
   AddInstTable(InstTable, "ADDL", 0, DecodeAdd32);
   AddInstTable(InstTable, "SUBL", 1, DecodeAdd32);
   AddInstTable(InstTable, "CMPL", 3, DecodeAdd32);
   AddInstTable(InstTable, "ADDSP",0, DecodeADDSP);
   AddInstTable(InstTable, "ADDW", 0, DecodeAdd16);
   AddInstTable(InstTable, "SUBW", 1, DecodeAdd16);
   AddInstTable(InstTable, "CMPW", 3, DecodeAdd16);
   AddInstTable(InstTable, "ANDW", 4, DecodeAdd16);
   AddInstTable(InstTable, "ORW" , 5, DecodeAdd16);
   AddInstTable(InstTable, "XORW", 6, DecodeAdd16);
   AddInstTable(InstTable, "ANDL", 4, DecodeLog32);
   AddInstTable(InstTable, "ORL",  5, DecodeLog32);
   AddInstTable(InstTable, "XORL", 6, DecodeLog32);
   AddInstTable(InstTable, "BBC", 0x80, DecodeBBcc);
   AddInstTable(InstTable, "BBS", 0xa0, DecodeBBcc);
   AddInstTable(InstTable, "CALL", 2, DecodeJmp16);
   AddInstTable(InstTable, "JMP", 0, DecodeJmp16);
   AddInstTable(InstTable, "CALLP", 2, DecodeJmp24);
   AddInstTable(InstTable, "JMPP", 0, DecodeJmp24);
   AddInstTable(InstTable, "CALLV", 0, DecodeCALLV);
   AddInstTable(InstTable, "CBNE", 0, DecodeCmpBranch);
   AddInstTable(InstTable, "CWBNE", 1, DecodeCmpBranch);
   AddInstTable(InstTable, "CLRB", 0x40, DecodeBit);
   AddInstTable(InstTable, "SETB", 0x60, DecodeBit);
   AddInstTable(InstTable, "CMP" , 0, DecodeCMP);
   AddInstTable(InstTable, "DBNZ" , 0, DecodeDBNZ);
   AddInstTable(InstTable, "DWBNZ" , 1, DecodeDBNZ);
   AddInstTable(InstTable, "INT", 0, DecodeINT);
   AddInstTable(InstTable, "INTP", 0, DecodeINTP);
   AddInstTable(InstTable, "JCTX", 0, DecodeJCTX);
   AddInstTable(InstTable, "LINK", 0, DecodeLINK);
   AddInstTable(InstTable, "MOV", 0, DecodeMOV);
   AddInstTable(InstTable, "MOVB", 0, DecodeMOVB);
   AddInstTable(InstTable, "MOVEA", 0, DecodeMOVEA);
   AddInstTable(InstTable, "MOVL", 0, DecodeMOVL);
   AddInstTable(InstTable, "MOVN", 0, DecodeMOVN);
   AddInstTable(InstTable, "MOVW", 0, DecodeMOVW);
   AddInstTable(InstTable, "MOVX", 0, DecodeMOVX);
   AddInstTable(InstTable, "NOT" , 0xe037, DecodeNEGNOT);
   AddInstTable(InstTable, "NEG" , 0x6003, DecodeNEGNOT);
   AddInstTable(InstTable, "NOTW", 0xe03f, DecodeNEGNOT);
   AddInstTable(InstTable, "NEGW", 0x600b, DecodeNEGNOT);
   AddInstTable(InstTable, "NRML", 0, DecodeNRML);
   AddInstTable(InstTable, "PUSHW", 0x40, DecodeStack);
   AddInstTable(InstTable, "POPW", 0x50, DecodeStack);
   AddInstTable(InstTable, "ROLC", 0, DecodeRotate);
   AddInstTable(InstTable, "RORC", 1, DecodeRotate);
   AddInstTable(InstTable, "SBBS", 1, DecodeSBBS);
   AddInstTable(InstTable, "WBTS", 0xc0, DecodeWBit);
   AddInstTable(InstTable, "WBTC", 0xe0, DecodeWBit);
   AddInstTable(InstTable, "XCH", 0, DecodeExchange);
   AddInstTable(InstTable, "XCHW", 1, DecodeExchange);
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(FixedOrders);
   free(ALU8Orders);
   free(Log8Orders);
   free(AccOrders);
   free(ShiftOrders);
   free(BranchOrders);
   free(IncDecOrders);
   free(MulDivOrders);
   free(SegOrders);
   free(StringOrders);
END

/*--------------------------------------------------------------------------*/
/* Interface zu AS */

        static void MakeCode_F2MC16(void)
BEGIN
   /* Leeranweisung ignorieren */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;
   if (DecodeIntelPseudo(False)) return;

   /* akt. Datensegment */

   switch (NextDataSeg)
    BEGIN
     case 0:
      CurrBank = Reg_PCB; break;
     case 1:
      CurrBank = Reg_DTB; break;
     case 2:
      CurrBank = Reg_ADB; break;
     case 3:
      CurrBank = SupAllowed ? Reg_SSB : Reg_USB; break;
    END
   CurrBank <<= 16;
   NextDataSeg = 1; /* = DTB */

   OpSize = -1;

   if (NOT LookupInstTable(InstTable, OpPart))
     WrXError(1200, OpPart);
END

        static Boolean IsDef_F2MC16(void)
BEGIN
   return FALSE;
END

        static void InitCode_F2MC16(void)
BEGIN
   SaveInitProc();
   Reg_PCB = 0xff;
   Reg_DTB = 0x00;
   Reg_USB = 0x00;
   Reg_SSB = 0x00;
   Reg_ADB = 0x00;
   Reg_DPR = 0x01;
END

        static void SwitchFrom_F2MC16(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void SwitchTo_F2MC16(void)
BEGIN
   PFamilyDescr FoundDescr;

   FoundDescr = FindFamilyByName("F2MC16");

   TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

   PCSymbol = "$"; HeaderID = FoundDescr->Id; NOPCode=0x00;
   DivideChars = ","; HasAttrs = False;

   ValidSegs = 1 << SegCode;
   Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
   SegLimits[SegCode] = 0xffffff;

   MakeCode = MakeCode_F2MC16; IsDef = IsDef_F2MC16;
   SwitchFrom = SwitchFrom_F2MC16; InitFields();

   AddONOFF("SUPMODE" , &SupAllowed, SupAllowedName,False);

   NextDataSeg = 1; /* DTB */
END

/*--------------------------------------------------------------------------*/
/* Initialisierung */

        void codef2mc16_init(void)
BEGIN
   CPU90500 = AddCPU("MB90500", SwitchTo_F2MC16);

   SaveInitProc = InitPassProc; InitPassProc = InitCode_F2MC16;
END
