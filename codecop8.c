/* codecop8.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul COP8-Familie                                           */
/*                                                                           */
/* Historie:  7.10.1996 Grundsteinlegung                                     */
/*           18. 8.1998 BookKeeping-Aufruf bei DSx                           */
/*            2. 1.1998 ChkPC umgebaut                                       */
/*           14. 8.1999 Maskierung in ChkAdr falsch                          */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModBInd 1
#define MModBInd (1 << ModBInd)
#define ModBInc 2
#define MModBInc (1 << ModBInc)
#define ModBDec 3
#define MModBDec (1 << ModBDec)
#define ModXInd 4
#define MModXInd (1 << ModXInd)
#define ModXInc 5
#define MModXInc (1 << ModXInc)
#define ModXDec 6
#define MModXDec (1 << ModXDec)
#define ModDir 7
#define MModDir (1 << ModDir)
#define ModImm 8
#define MModImm (1 << ModImm)

#define DirPrefix 0xbd
#define BReg 0xfe

#define FixedOrderCnt 13
#define AccOrderCnt 9
#define AccMemOrderCnt 7
#define BitOrderCnt 3

   typedef struct
            {
             char *Name;
             Byte Code;
            } FixedOrder;

static CPUVar CPUCOP87L84;

static FixedOrder *FixedOrders;
static FixedOrder *AccOrders;
static FixedOrder *AccMemOrders;
static FixedOrder *BitOrders;

static ShortInt AdrMode;
static Byte AdrVal;
static Boolean BigFlag;

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddAcc(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=NName;
   AccOrders[InstrZ++].Code=NCode;
END

        static void AddAccMem(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=AccMemOrderCnt) exit(255);
   AccMemOrders[InstrZ].Name=NName;
   AccMemOrders[InstrZ++].Code=NCode;
END

        static void AddBit(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ].Name=NName;
   BitOrders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(FixedOrderCnt*sizeof(FixedOrder)); InstrZ=0;
   AddFixed("LAID" ,0xa4);  AddFixed("SC"   ,0xa1);  AddFixed("RC"   ,0xa0);
   AddFixed("IFC"  ,0x88);  AddFixed("IFNC" ,0x89);  AddFixed("VIS"  ,0xb4);
   AddFixed("JID"  ,0xa5);  AddFixed("RET"  ,0x8e);  AddFixed("RETSK",0x8d);
   AddFixed("RETI" ,0x8f);  AddFixed("INTR" ,0x00);  AddFixed("NOP"  ,0xb8);
   AddFixed("RPND" ,0xb5);

   AccOrders=(FixedOrder *) malloc(AccOrderCnt*sizeof(FixedOrder)); InstrZ=0;
   AddAcc("CLR"  ,0x64);  AddAcc("INC"  ,0x8a);  AddAcc("DEC"  ,0x8b);
   AddAcc("DCOR" ,0x66);  AddAcc("RRC"  ,0xb0);  AddAcc("RLC"  ,0xa8);
   AddAcc("SWAP" ,0x65);  AddAcc("POP"  ,0x8c);  AddAcc("PUSH" ,0x67);

   AccMemOrders=(FixedOrder *) malloc(AccMemOrderCnt*sizeof(FixedOrder)); InstrZ=0;
   AddAccMem("ADD"  ,0x84);  AddAccMem("ADC"  ,0x80);  AddAccMem("SUBC" ,0x81);
   AddAccMem("AND"  ,0x85);  AddAccMem("OR"   ,0x87);  AddAccMem("XOR"  ,0x86);
   AddAccMem("IFGT" ,0x83);

   BitOrders=(FixedOrder *) malloc(BitOrderCnt*sizeof(FixedOrder)); InstrZ=0;
   AddBit("IFBIT",0x70); AddBit("SBIT",0x78); AddBit("RBIT",0x68);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(AccOrders);
   free(AccMemOrders);
   free(BitOrders);
END

/*---------------------------------------------------------------------------*/

        static void ChkAdr(Word Mask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     AdrMode=ModNone; WrError(1350);
    END
END

        static void DecodeAdr(char *Asc, Word Mask)
BEGIN
   static char *ModStrings[ModXDec+1]=
              {"A","[B]","[B+]","[B-]","[X]","[X+]","[X-]"};

   int z;
   Boolean OK;

   AdrMode=ModNone;

   /* indirekt/Akku */

   for (z=ModAcc; z<=ModXDec; z++)
    if (strcasecmp(Asc,ModStrings[z])==0)
     BEGIN
      AdrMode=z; ChkAdr(Mask); return;
     END

   /* immediate */

   if (*Asc=='#')
    BEGIN
     AdrVal=EvalIntExpression(Asc+1,Int8,&OK);
     if (OK) AdrMode=ModImm;
     ChkAdr(Mask); return;
    END

   /* direkt */

   AdrVal=EvalIntExpression(Asc,Int8,&OK);
   if (OK)
    BEGIN
     AdrMode=ModDir; ChkSpace(SegData);
    END

   ChkAdr(Mask);
END

/*---------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
   Boolean ValOK;
   Word Size,Value,t,z;

   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,0xff);
     return True;
    END;

   if (Memo("ADDR"))
    BEGIN
     strcpy(OpPart,"DB"); BigFlag=True;
    END

   if (Memo("ADDRW"))
    BEGIN
     strcpy(OpPart,"DW"); BigFlag=True;
    END

   if (Memo("BYTE")) strcpy(OpPart,"DB");

   if (Memo("WORD")) strcpy(OpPart,"DW");

   if ((Memo("DSB")) OR (Memo("DSW")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],UInt16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) AND (NOT FirstPassUnknown))
        BEGIN
         DontPrint=True;
         if (Memo("DSW")) Size+=Size;
         CodeLen=Size;
         BookKeeping();
        END
      END
     return True;
    END

   if (Memo("FB"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],UInt16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) AND (NOT FirstPassUnknown))
        BEGIN
         if (Size>MaxCodeLen) WrError(1920);
         else
          BEGIN
           BAsmCode[0]=EvalIntExpression(ArgStr[2],Int8,&ValOK);
           if (ValOK)
            BEGIN
             CodeLen=Size;
             memset(BAsmCode+1,BAsmCode[0],Size-1);
            END
          END
        END
      END
     return True;
    END

   if (Memo("FW"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],UInt16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) AND (NOT FirstPassUnknown))
        BEGIN
         if ((Size << 1)>MaxCodeLen) WrError(1920);
         else
          BEGIN
           Value=EvalIntExpression(ArgStr[2],Int16,&ValOK);
           if (ValOK)
            BEGIN
             CodeLen=Size << 1; t=0;
             for (z=0; z<Size; z++)
              BEGIN
               BAsmCode[t++]=Lo(Value); BAsmCode[t++]=Hi(Value);
              END
            END
          END
        END
      END
     return True;
    END

   return False;
END

        static void MakeCode_COP8(void)
BEGIN
   Integer AdrInt;
   int z;
   Byte HReg;
   Boolean OK;
   Word AdrWord;

   CodeLen=0; DontPrint=False; BigFlag=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(BigFlag)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if Memo(FixedOrders[z].Name)
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        BAsmCode[0]=FixedOrders[z].Code; CodeLen=1;
       END
      return;
     END

   /* Datentransfer */

   if (Memo("LD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModDir+MModBInd+MModBInc+MModBDec);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          DecodeAdr(ArgStr[2],MModDir+MModImm+MModBInd+MModXInd+MModBInc+MModXInc+MModBDec+MModXDec);
          switch (AdrMode)
           BEGIN
            case ModDir:
             BAsmCode[0]=0x9d; BAsmCode[1]=AdrVal; CodeLen=2;
             break;
            case ModImm:
             BAsmCode[0]=0x98; BAsmCode[1]=AdrVal; CodeLen=2;
             break;
            case ModBInd:
             BAsmCode[0]=0xae; CodeLen=1;
             break;
            case ModXInd:
             BAsmCode[0]=0xbe; CodeLen=1;
             break;
            case ModBInc:
             BAsmCode[0]=0xaa; CodeLen=1;
             break;
            case ModXInc:
             BAsmCode[0]=0xba; CodeLen=1;
             break;
            case ModBDec:
             BAsmCode[0]=0xab; CodeLen=1;
             break;
            case ModXDec:
             BAsmCode[0]=0xbb; CodeLen=1;
             break;
           END
          break;
         case ModDir:
          HReg=AdrVal; DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode==ModImm)
           BEGIN
            if (HReg==BReg)
             BEGIN
              if (AdrVal<=15)
               BEGIN
                BAsmCode[0]=0x5f-AdrVal; CodeLen=1;
               END
              else
               BEGIN
                BAsmCode[0]=0x9f; BAsmCode[1]=AdrVal; CodeLen=2;
               END
             END
            else if (HReg>=0xf0)
             BEGIN
              BAsmCode[0]=HReg-0x20; BAsmCode[1]=AdrVal; CodeLen=2;
             END
            else
             BEGIN
              BAsmCode[0]=0xbc; BAsmCode[1]=HReg; BAsmCode[2]=AdrVal; CodeLen=3;
             END
           END
          break;
         case ModBInd:
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode!=ModNone)
           BEGIN
            BAsmCode[0]=0x9e; BAsmCode[1]=AdrVal; CodeLen=2;
           END
          break;
         case ModBInc:
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode!=ModNone)
           BEGIN
            BAsmCode[0]=0x9a; BAsmCode[1]=AdrVal; CodeLen=2;
           END
          break;
         case ModBDec:
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode!=ModNone)
           BEGIN
            BAsmCode[0]=0x9b; BAsmCode[1]=AdrVal; CodeLen=2;
           END
          break;
        END
      END
     return;
    END

   if (Memo("X"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (strcasecmp(ArgStr[1],"A")!=0)
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
        END
       DecodeAdr(ArgStr[1],MModAcc);
       if (AdrMode!=ModNone)
        BEGIN
         DecodeAdr(ArgStr[2],MModDir+MModBInd+MModXInd+MModBInc+MModXInc+MModBDec+MModXDec);
         switch (AdrMode)
          BEGIN
           case ModDir:
            BAsmCode[0]=0x9c; BAsmCode[1]=AdrVal; CodeLen=2;
            break;
           case ModBInd:
            BAsmCode[0]=0xa6; CodeLen=1;
            break;
           case ModBInc:
            BAsmCode[0]=0xa2; CodeLen=1;
            break;
           case ModBDec:
            BAsmCode[0]=0xa3; CodeLen=1;
            break;
           case ModXInd:
            BAsmCode[0]=0xb6; CodeLen=1;
            break;
           case ModXInc:
            BAsmCode[0]=0xb2; CodeLen=1;
            break;
           case ModXDec:
            BAsmCode[0]=0xb3; CodeLen=1;
            break;
          END
        END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<AccOrderCnt; z++)
    if Memo(AccOrders[z].Name)
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModAcc);
        if (AdrMode!=ModNone)
         BEGIN
          BAsmCode[0]=AccOrders[z].Code; CodeLen=1;
         END
       END
      return;
     END

   for (z=0; z<AccMemOrderCnt; z++)
    if Memo(AccMemOrders[z].Name)
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModAcc);
        if (AdrMode!=ModNone)
         BEGIN
          DecodeAdr(ArgStr[2],MModDir+MModImm+MModBInd);
          switch (AdrMode)
           BEGIN
            case ModBInd:
             BAsmCode[0]=AccMemOrders[z].Code; CodeLen=1;
             break;
            case ModImm:
             BAsmCode[0]=AccMemOrders[z].Code+0x10; BAsmCode[1]=AdrVal;
             CodeLen=2;
             break;
            case ModDir:
             BAsmCode[0]=DirPrefix; BAsmCode[1]=AdrVal; 
             BAsmCode[2]=AccMemOrders[z].Code;
             CodeLen=3;
             break;
           END
         END
       END
      return;
     END

   if (Memo("ANDSZ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc);
       if (AdrMode!=ModNone)
        BEGIN
         DecodeAdr(ArgStr[2],MModImm);
         if (AdrMode==ModImm)
          BEGIN
           BAsmCode[0]=0x60; BAsmCode[1]=AdrVal; CodeLen=2;
          END
        END
      END
     return;
    END

   /* Bedingungen */

   if (Memo("IFEQ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModDir);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          DecodeAdr(ArgStr[2],MModDir+MModBInd+MModImm);
          switch (AdrMode)
           BEGIN
            case ModDir:
             BAsmCode[0]=DirPrefix; BAsmCode[1]=AdrVal; BAsmCode[2]=0x82; 
             CodeLen=3;
             break;
            case ModBInd:
             BAsmCode[0]=0x82; CodeLen=1;
             break;
            case ModImm:
             BAsmCode[0]=0x92; BAsmCode[1]=AdrVal; CodeLen=2;
             break;
           END
          break;
         case ModDir:
          BAsmCode[1]=AdrVal;
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode==ModImm)
           BEGIN
            BAsmCode[0]=0xa9; BAsmCode[2]=AdrVal; CodeLen=3;
           END
          break;
        END
      END
     return;
    END

   if (Memo("IFNE"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          DecodeAdr(ArgStr[2],MModDir+MModBInd+MModImm);
          switch (AdrMode)
           BEGIN
            case ModDir:
             BAsmCode[0]=DirPrefix; BAsmCode[1]=AdrVal; BAsmCode[2]=0xb9; 
             CodeLen=3;
             break;
            case ModBInd:
             BAsmCode[0]=0xb9; CodeLen=1;
             break;
            case ModImm:
             BAsmCode[0]=0x99; BAsmCode[1]=AdrVal; CodeLen=2;
             break;
           END
          break;
        END
      END
     return;
    END

   if (Memo("IFBNE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      BEGIN
       BAsmCode[0]=EvalIntExpression(ArgStr[1]+1,UInt4,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]+=0x40; CodeLen=1;
        END
      END
     return;
    END

   /* Bitbefehle */

   for (z=0; z<BitOrderCnt; z++)
    if Memo(BitOrders[z].Name)
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        HReg=EvalIntExpression(ArgStr[1],UInt3,&OK);
        if (OK)
         BEGIN
          DecodeAdr(ArgStr[2],MModDir+MModBInd);
          switch (AdrMode)
           BEGIN
            case ModDir:
             BAsmCode[0]=DirPrefix; BAsmCode[1]=AdrVal; 
             BAsmCode[2]=BitOrders[z].Code+HReg; CodeLen=3;
             break;
            case ModBInd:
             BAsmCode[0]=BitOrders[z].Code+HReg; CodeLen=1;
             break;
           END
         END
       END
      return;
     END

   /* Spruenge */

   if ((Memo("JMP")) OR (Memo("JSR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        BEGIN
         if (((EProgCounter()+2) >> 12)!=(AdrWord >> 12)) WrError(1910);
         else
          BEGIN
           ChkSpace(SegCode);
           BAsmCode[0]=0x20+(Ord(Memo("JSR")) << 4)+((AdrWord >> 8) & 15);
           BAsmCode[1]=Lo(AdrWord);
           CodeLen=2;
          END
        END
      END
     return;
    END

   if ((Memo("JMPL")) OR (Memo("JSRL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        BEGIN
         ChkSpace(SegCode);
         BAsmCode[0]=0xac+Ord(Memo("JSRL"));
         BAsmCode[1]=Hi(AdrWord);
         BAsmCode[2]=Lo(AdrWord);
         CodeLen=3;
        END
      END
     return;
    END

   if (Memo("JP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+1);
       if (OK)
        BEGIN
         if (AdrInt==0)
          BEGIN
           BAsmCode[0]=NOPCode; CodeLen=1; WrError(60);
          END
         else if (((AdrInt>31) OR (AdrInt<-32)) AND (NOT SymbolQuestionable)) WrError(1370);
         else
          BEGIN
           BAsmCode[0]=AdrInt & 0xff; CodeLen=1;
          END
        END
      END
     return;
    END

   if (Memo("DRSZ"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       DecodeAdr(ArgStr[1],MModDir);
       if (FirstPassUnknown) AdrVal|=0xf0;
       if (AdrVal<0xf0) WrError(1315);
       else
        BEGIN
         BAsmCode[0]=AdrVal-0x30; CodeLen=1;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_COP8(void)
BEGIN
   return (Memo("SFR"));
END

        static void SwitchFrom_COP8(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_COP8(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeC; SetIsOccupied=False;

   PCSymbol="."; HeaderID=0x6f; NOPCode=0xb8;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0x1fff;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;
   SegLimits[SegData] = 0xff;

   MakeCode=MakeCode_COP8; IsDef=IsDef_COP8;
   SwitchFrom=SwitchFrom_COP8; InitFields();
END

        void codecop8_init(void)
BEGIN
   CPUCOP87L84=AddCPU("COP87L84",SwitchTo_COP8);
END


