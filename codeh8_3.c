/* codeh8_3.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator H8/300(L/H)                                                 */
/*                                                                           */
/* Historie: 22.11.1996 Grundsteinlegung                                     */
/*           15.10.1998 TRAPA nachgetragen                                   */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "codepseudo.h"
#include "codevars.h"

#define FixedOrderCount 4
#define ConditionCount 20
#define ShiftOrderCount 8
#define LogicOrderCount 3
#define MulOrderCount 4
#define Bit1OrderCount 10
#define Bit2OrderCount 4

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModImm 1
#define MModImm (1 << ModImm)
#define ModAbs8 2
#define MModAbs8 (1 << ModAbs8)
#define ModAbs16 3
#define MModAbs16 (1 << ModAbs16)
#define ModAbs24 4
#define MModAbs24 (1 << ModAbs24)
#define MModAbs (MModAbs8+MModAbs16+MModAbs24)
#define ModIReg 5
#define MModIReg (1 << ModIReg)
#define ModPreDec 6
#define MModPreDec (1 << ModPreDec)
#define ModPostInc 7
#define MModPostInc (1 << ModPostInc)
#define ModInd16 8
#define MModInd16 (1 << ModInd16)
#define ModInd24 9
#define MModInd24 (1 << ModInd24)
#define ModIIAbs 10
#define MModIIAbs (1 << ModIIAbs)
#define MModInd (MModInd16+MModInd24)

typedef struct
         { 
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         { 
          char *Name;
          Byte Code;
         } Condition;


static ShortInt OpSize;     /* Groesse=8*(2^OpSize) */
static ShortInt AdrMode;    /* Ergebnisadressmodus */
static Byte AdrPart;        /* Adressierungsmodusbits im Opcode */
static Word AdrVals[6];     /* Adressargument */

static CPUVar CPUH8_300L;
static CPUVar CPU6413308,CPUH8_300;
static CPUVar CPU6413309,CPUH8_300H;
static Boolean CPU16;       /* keine 32-Bit-Register */

static Condition  *Conditions;
static FixedOrder *FixedOrders;
static FixedOrder *ShiftOrders;
static FixedOrder *LogicOrders;
static FixedOrder *MulOrders;
static FixedOrder *Bit1Orders;
static FixedOrder *Bit2Orders;

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddCond(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ConditionCount) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END

        static void AddShift(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ShiftOrderCount) exit(255);
   ShiftOrders[InstrZ].Name=NName;
   ShiftOrders[InstrZ++].Code=NCode;
END

        static void AddLogic(char *NName, Word NCode)
BEGIN
   if (InstrZ>=LogicOrderCount) exit(255);
   LogicOrders[InstrZ].Name=NName;
   LogicOrders[InstrZ++].Code=NCode;
END

        static void AddMul(char *NName, Word NCode)
BEGIN
   if (InstrZ>=MulOrderCount) exit(255);
   MulOrders[InstrZ].Name=NName;
   MulOrders[InstrZ++].Code=NCode;
END

        static void AddBit1(char *NName, Word NCode)
BEGIN
   if (InstrZ>=Bit1OrderCount) exit(255);
   Bit1Orders[InstrZ].Name=NName;
   Bit1Orders[InstrZ++].Code=NCode;
END

        static void AddBit2(char *NName, Word NCode)
BEGIN
   if (InstrZ>=Bit2OrderCount) exit(255);
   Bit2Orders[InstrZ].Name=NName;
   Bit2Orders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("NOP",0x0000); AddFixed("RTE",0x5670);
   AddFixed("RTS",0x5470); AddFixed("SLEEP",0x0180);

   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCount); InstrZ=0;
   AddCond("BRA",0x0); AddCond("BT" ,0x0);
   AddCond("BRN",0x1); AddCond("BF" ,0x1);
   AddCond("BHI",0x2); AddCond("BLS",0x3);
   AddCond("BCC",0x4); AddCond("BHS",0x4);
   AddCond("BCS",0x5); AddCond("BLO",0x5);
   AddCond("BNE",0x6); AddCond("BEQ",0x7);
   AddCond("BVC",0x8); AddCond("BVS",0x9);
   AddCond("BPL",0xa); AddCond("BMI",0xb);
   AddCond("BGE",0xc); AddCond("BLT",0xd);
   AddCond("BGT",0xe); AddCond("BLE",0xf);

   ShiftOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*ShiftOrderCount); InstrZ=0;
   AddShift("ROTL" ,0x1280); AddShift("ROTR" ,0x1380);
   AddShift("ROTXL",0x1200); AddShift("ROTXR",0x1300);
   AddShift("SHAL" ,0x1080); AddShift("SHAR" ,0x1180);
   AddShift("SHLL" ,0x1000); AddShift("SHLR" ,0x1100);

   LogicOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LogicOrderCount); InstrZ=0;
   AddLogic("OR",0); AddLogic("XOR",1); AddLogic("AND",2);

   MulOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*MulOrderCount); InstrZ=0;
   AddMul("DIVXS",3); AddMul("DIVXU",1); AddMul("MULXS",2); AddMul("MULXU",0);

   Bit1Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Bit1OrderCount); InstrZ=0;
   AddBit1("BAND",0x16); AddBit1("BIAND",0x96);
   AddBit1("BOR" ,0x14); AddBit1("BIOR" ,0x94);
   AddBit1("BXOR",0x15); AddBit1("BIXOR",0x95);
   AddBit1("BLD" ,0x17); AddBit1("BILD" ,0x97);
   AddBit1("BST" ,0x07); AddBit1("BIST" ,0x87);

   Bit2Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Bit2OrderCount); InstrZ=0;
   AddBit2("BCLR",2); AddBit2("BNOT",1); AddBit2("BSET",0); AddBit2("BTST",3);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(Conditions);
   free(ShiftOrders);
   free(LogicOrders);
   free(MulOrders);
   free(Bit1Orders);
   free(Bit2Orders);
END

/*-------------------------------------------------------------------------*/
/* Adressparsing */

typedef enum {SizeNone,Size8,Size16,Size24} MomSize_t;
static MomSize_t MomSize;

        static void SetOpSize(ShortInt Size)
BEGIN
   if (OpSize==-1) OpSize=Size;
   else if (Size!=OpSize)
    BEGIN
     WrError(1131); AdrMode=ModNone; AdrCnt=0;
    END
END

        static Boolean IsNum(char Inp, Byte *Erg)
BEGIN
   if ((Inp<'0') OR (Inp>'7')) return False;
   else
    BEGIN
     *Erg=Inp-AscOfs; return True;
    END
END

        static Boolean DecodeReg(char *Asc, Byte *Erg, ShortInt *Size)
BEGIN
   if (strcasecmp(Asc,"SP")==0)
    BEGIN
     *Erg=7; *Size=(Maximum) ? 2 : 1; return True;
    END

   else if ((strlen(Asc)==3) AND (toupper(*Asc)=='R') AND (IsNum(Asc[1],Erg)))
    if (toupper(Asc[2])=='L')
     BEGIN
      *Erg+=8; *Size=0; return True;
     END
    else if (toupper(Asc[2])=='H')
     BEGIN
      *Size=0; return True;
     END
    else return False;

   else if ((strlen(Asc)==2) AND (IsNum(Asc[1],Erg)))
    if (toupper(*Asc)=='R')
     BEGIN
      *Size=1; return True;
     END
    else if (toupper(*Asc)=='E')
     BEGIN
      *Erg+=8; *Size=1; return True;
     END
    else return False;

   else if ((strlen(Asc)==3) AND (toupper(*Asc)=='E') AND (toupper(Asc[1])=='R') AND (IsNum(Asc[2],Erg)))
    BEGIN
     *Size=2; return True;
    END
   else return False;
END

        static void CutSize(char *Asc)
BEGIN
   int l=strlen(Asc);

   if ((l>=2) AND (Asc[l-2]==':') AND (Asc[l-1]=='8'))
    BEGIN
     Asc[l-2]='\0'; MomSize=Size8;
    END
   else if ((l>=3) AND (Asc[l-3]==':'))
    BEGIN
     if ((Asc[l-2]=='1') AND (Asc[l-1]=='6'))
      BEGIN
       Asc[l-3]='\0'; MomSize=Size16;
      END
     else if ((Asc[l-2]=='2') AND (Asc[l-1]=='4'))
      BEGIN
       Asc[l-3]='\0'; MomSize=Size24;
      END
    END
END

        static Byte DecodeBaseReg(char *Asc, Byte *Erg)
BEGIN
   ShortInt HSize;

   if (NOT DecodeReg(Asc,Erg,&HSize)) return 0;
   if ((HSize==0) OR ((HSize==1) AND (*Erg>7)))
    BEGIN
     WrError(1350); return 1;
    END;
   if ((CPU16) != (HSize==1))
    BEGIN
     WrError(1505); return 1;
    END
   return 2;
END

        static Boolean Is8(LongInt Address)
BEGIN
   if (CPU16) return (((Address >> 8)&0xff)==0xff);
   else return (((Address >> 8)&0xffff)==0xffff);
END

        static Boolean Is16(LongInt Address)
BEGIN
   return (CPU16) ? (True) : (((Address>=0) AND (Address<=0x7fff)) OR ((Address>=0xff8000) AND (Address<=0xffffff)));
END

        static void DecideVAbsolute(LongInt Address, Word Mask)
BEGIN
   /* bei Automatik Operandengroesse festlegen */

   if (MomSize==SizeNone)
    BEGIN
     if (Is8(Address)) MomSize=Size8;
     else if (Is16(Address)) MomSize=Size16;
     else MomSize=Size24;
    END

   /* wenn nicht vorhanden, eins rauf */

   if ((MomSize==Size8)  AND ((Mask & MModAbs8)==0)) MomSize=Size16;
   if ((MomSize==Size16) AND ((Mask & MModAbs16)==0)) MomSize=Size24;

   /* entsprechend Modus Bytes ablegen */

   switch (MomSize)
    BEGIN
     case Size8:
      if (NOT Is8(Address)) WrError(1925);
      else
       BEGIN
        AdrCnt=2; AdrVals[0]=Address & 0xff; AdrMode=ModAbs8;
       END
      break;
     case Size16:
      if (NOT Is16(Address)) WrError(1925);
      else
       BEGIN
        AdrCnt=2; AdrVals[0]=Address & 0xffff; AdrMode=ModAbs16;
       END
      break;
     case Size24:
      AdrCnt=4;
      AdrVals[1]=Address & 0xffff;
      AdrVals[0]=Lo(Address >> 16);
      AdrMode=ModAbs24;
      break;
     default:
      WrError(10000);
    END
END

        static void DecideAbsolute(char *Asc, Word Mask)
BEGIN
   LongInt Addr;
   Boolean OK;

   Addr=EvalIntExpression(Asc,Int32,&OK);
   if (OK) DecideVAbsolute(Addr,Mask);
END


        static void ChkAdr(Word Mask)
BEGIN
   if (CPU16)
    if (((AdrMode==ModReg) AND (OpSize==2))
    OR  ((AdrMode==ModReg) AND (OpSize==1) AND (AdrPart>7))
    OR  (AdrMode==ModAbs24)
    OR  (AdrMode==ModInd24))
     BEGIN
      WrError(1505); AdrMode=ModNone; AdrCnt=0;
     END
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     WrError(1350); AdrMode=ModNone; AdrCnt=0;
    END
END

        static void DecodeAdr(char *Asc, Word Mask)
BEGIN
   ShortInt HSize;
   Byte HReg;
   LongInt HLong;
   Boolean OK;
   char *p;
   LongInt DispAcc;
   String Part;
   int l;

   AdrMode=ModNone; AdrCnt=0; MomSize=SizeNone;

   /* immediate ? */

   if (*Asc=='#')
    BEGIN
     switch (OpSize)
      BEGIN
       case -1:
        WrError(1132);
        break;
       case 0:
        HReg=EvalIntExpression(Asc+1,Int8,&OK);
        if (OK)
         BEGIN
          AdrCnt=2; AdrVals[0]=HReg; AdrMode=ModImm;
         END
        break;
       case 1:
        AdrVals[0]=EvalIntExpression(Asc+1,Int16,&OK);
        if (OK)
         BEGIN
          AdrCnt=2; AdrMode=ModImm;
         END
        break;
       case 2:
        HLong=EvalIntExpression(Asc+1,Int32,&OK);
        if (OK)
         BEGIN
          AdrCnt=4;
          AdrVals[0]=HLong >> 16;
          AdrVals[1]=HLong & 0xffff;
          AdrMode=ModImm;
         END
        break;
       default:
        WrError(1130);
      END
     ChkAdr(Mask); return;
    END

   /* Register ? */

   if (DecodeReg(Asc,&HReg,&HSize))
    BEGIN
     AdrMode=ModReg; AdrPart=HReg; SetOpSize(HSize); ChkAdr(Mask); return;
    END

   /* indirekt ? */

   if (*Asc=='@')
    BEGIN
     strcpy(Asc,Asc+1);

     if (*Asc=='@')
      BEGIN
       AdrVals[0]=EvalIntExpression(Asc+1,UInt8,&OK) & 0xff;
       if (OK)
        BEGIN
         AdrCnt=1; AdrMode=ModIIAbs;
        END
       ChkAdr(Mask); return;
      END

     switch (DecodeBaseReg(Asc,&AdrPart))
      BEGIN
       case 1:
        ChkAdr(Mask); return;
       case 2:
        AdrMode=ModIReg; ChkAdr(Mask); return;
      END

     if (*Asc=='-')
      switch (DecodeBaseReg(Asc+1,&AdrPart))
       BEGIN
        case 1:
         ChkAdr(Mask); return;
        case 2:
         AdrMode=ModPreDec; ChkAdr(Mask); return;
       END

     if ((*Asc) && (Asc[l=strlen(Asc)-1]=='+'))
      BEGIN
       Asc[l]='\0';
       switch (DecodeBaseReg(Asc,&AdrPart))
        BEGIN
         case 1:
          ChkAdr(Mask); return;
         case 2:
          AdrMode=ModPostInc; ChkAdr(Mask); return;
        END
       Asc[l]='+';
      END

     if (IsIndirect(Asc))
      BEGIN
       strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
       AdrPart=0xff; DispAcc=0;
       do
        BEGIN
         p=QuotPos(Asc,',');
         if (p==Nil)
          BEGIN
           strmaxcpy(Part,Asc,255); *Asc='\0';
          END
         else
          BEGIN
           *p='\0'; strmaxcpy(Part,Asc,255); strmaxcpy(Asc,p+1,255);
          END 
         switch (DecodeBaseReg(Part,&HReg))
          BEGIN
           case 2:
            if (AdrPart!=0xff)
             BEGIN
              WrError(1350); ChkAdr(Mask); return;
             END
            else AdrPart=HReg;
            break;
           case 1:
            ChkAdr(Mask); return;
           case 0:
            CutSize(Part);
            DispAcc+=EvalIntExpression(Part,Int32,&OK);
            if (NOT OK) 
             BEGIN
              ChkAdr(Mask); return;
             END
            break;
          END
        END
       while (*Asc!='\0');
       if (AdrPart==0xff) DecideVAbsolute(DispAcc,Mask);
       else
        BEGIN
         if ((CPU16) AND ((DispAcc & 0xffff8000)==0x8000)) DispAcc+=0xffff0000;
         if (MomSize==SizeNone)
          MomSize=((DispAcc>=-32768) AND (DispAcc<=32767)) ? Size16 : Size24;
         switch (MomSize)
          BEGIN
           case Size8:
            WrError(1130); break;
           case Size16:
            if (DispAcc<-32768) WrError(1315);
            else if (DispAcc>32767) WrError(1320);
            else
             BEGIN
              AdrCnt=2; AdrVals[0]=DispAcc & 0xffff; AdrMode=ModInd16;
             END
            break;
           case Size24:
            AdrVals[1]=DispAcc & 0xffff; AdrVals[0]=Lo(DispAcc >> 16);
            AdrCnt=4; AdrMode=ModInd24;
            break;
           default:
            WrError(10000);
          END
        END
      END
     else
      BEGIN
       CutSize(Asc);
       DecideAbsolute(Asc,Mask);
      END
     ChkAdr(Mask); return;
    END

   CutSize(Asc);
   DecideAbsolute(Asc,Mask);
   ChkAdr(Mask);
END

        static LongInt ImmVal(void)
BEGIN
   switch (OpSize)
    BEGIN
     case 0: return Lo(AdrVals[0]);
     case 1: return AdrVals[0];
     case 2: return (((LongInt)AdrVals[0]) << 16)+AdrVals[1];
     default: WrError(10000); return 0;
    END
END

/*-------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        static void MakeCode_H8_3(void)
BEGIN
   int z;
   Word Mask;
   ShortInt HSize;
   LongInt AdrLong;
   Byte HReg,OpCode;
   Boolean OK;

   CodeLen=0; DontPrint=False; OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* Attribut verwursten */

   if (*AttrPart!='\0')
    BEGIN
     if (strlen(AttrPart)!=1)
      BEGIN
       WrError(1105); return;
      END
     switch (toupper(*AttrPart))
      BEGIN
       case 'B': SetOpSize(0); break;
       case 'W': SetOpSize(1); break;
       case 'L': SetOpSize(2); break;
       case 'Q': SetOpSize(3); break;
       case 'S': SetOpSize(4); break;
       case 'D': SetOpSize(5); break;
       case 'X': SetOpSize(6); break;
       case 'P': SetOpSize(7); break;
       default:
        WrError(1107); return;
      END
    END

   if (DecodeMoto16Pseudo(OpSize,True)) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        CodeLen=2; WAsmCode[0]=FixedOrders[z].Code;
       END;
      return;
     END

   if (Memo("EEPMOV"))
    BEGIN
     if (OpSize==-1) OpSize=Ord(NOT CPU16);
     if (OpSize>1) WrError(1130);
     else if (ArgCnt!=0) WrError(1110);
     else if ((OpSize==1) AND (CPU16)) WrError(1500);
     else
      BEGIN
       CodeLen=4;
       WAsmCode[0]=(OpSize==0) ? 0x7b5c : 0x7bd4;
       WAsmCode[1]=0x598f;
      END
     return;
    END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPreDec+MModInd+MModAbs);
       switch (AdrMode)
        BEGIN
         case ModReg:
          HReg=AdrPart;
          Mask=MModReg+MModIReg+MModPostInc+MModInd+MModAbs+MModImm;
          if (OpSize!=0) Mask-=MModAbs8;
          DecodeAdr(ArgStr[1],Mask);
          switch (AdrMode)
           BEGIN
            case ModReg:
             z=OpSize; if (z==2) z=3;
             CodeLen=2; WAsmCode[0]=0x0c00+(z << 8)+(AdrPart << 4)+HReg;
             if (OpSize==2) WAsmCode[0]+=0x80;
             break;
            case ModIReg:
             switch (OpSize)
              BEGIN
               case 0:
                CodeLen=2; WAsmCode[0]=0x6800+(AdrPart << 4)+HReg;
                break;
               case 1:
                CodeLen=2; WAsmCode[0]=0x6900+(AdrPart << 4)+HReg;
                break;
               case 2:
                CodeLen=4; WAsmCode[0]=0x0100;
                WAsmCode[1]=0x6900+(AdrPart << 4)+HReg;
                break;
              END
             break;
            case ModPostInc:
             switch (OpSize)
              BEGIN
               case 0:
                CodeLen=2; WAsmCode[0]=0x6c00+(AdrPart << 4)+HReg;
                break;
               case 1:
                CodeLen=2; WAsmCode[0]=0x6d00+(AdrPart << 4)+HReg;
                break;
               case 2:
                CodeLen=4; WAsmCode[0]=0x0100;
                WAsmCode[1]=0x6d00+(AdrPart << 4)+HReg;
                break;
              END
             break;
            case ModInd16:
             switch (OpSize)
              BEGIN
               case 0:
                CodeLen=4; WAsmCode[0]=0x6e00+(AdrPart << 4)+HReg;
                WAsmCode[1]=AdrVals[0];
                break;
               case 1:
                CodeLen=4; WAsmCode[0]=0x6f00+(AdrPart << 4)+HReg;
                WAsmCode[1]=AdrVals[0];
                break;
               case 2:
                CodeLen=6; WAsmCode[0]=0x0100;
                WAsmCode[1]=0x6f00+(AdrPart << 4)+HReg;
                WAsmCode[2]=AdrVals[0];
                break;
              END
             break;
            case ModInd24:
             switch (OpSize)
              BEGIN
               case 0:
                CodeLen=8;
                WAsmCode[0]=0x7800+(AdrPart << 4);
                WAsmCode[1]=0x6a20+HReg;
                memcpy(WAsmCode+2,AdrVals,AdrCnt);
                break;
               case 1:
                CodeLen=8;
                WAsmCode[0]=0x7800+(AdrPart << 4);
                WAsmCode[1]=0x6b20+HReg;
                memcpy(WAsmCode+2,AdrVals,AdrCnt);
                break;
               case 2:
                CodeLen=10; WAsmCode[0]=0x0100;
                WAsmCode[1]=0x7800+(AdrPart << 4);
                WAsmCode[2]=0x6b20+HReg;
                memcpy(WAsmCode+3,AdrVals,AdrCnt);
                break;
              END
             break;
            case ModAbs8:
             CodeLen=2; WAsmCode[0]=0x2000+(((Word)HReg) << 8)+Lo(AdrVals[0]);
             break;
            case ModAbs16:
             switch (OpSize)
              BEGIN
               case 0:
                CodeLen=4; WAsmCode[0]=0x6a00+HReg;
                WAsmCode[1]=AdrVals[0];
                break;
               case 1:
                CodeLen=4; WAsmCode[0]=0x6b00+HReg;
                WAsmCode[1]=AdrVals[0];
                break;
               case 2:
                CodeLen=6; WAsmCode[0]=0x0100;
                WAsmCode[1]=0x6b00+HReg;
                WAsmCode[2]=AdrVals[0];
                break;
              END
             break;
            case ModAbs24:
             switch (OpSize)
              BEGIN
               case 0:
                CodeLen=6; WAsmCode[0]=0x6a20+HReg;
                memcpy(WAsmCode+1,AdrVals,AdrCnt);
                break;
               case 1:
                CodeLen=6; WAsmCode[0]=0x6b20+HReg;
                memcpy(WAsmCode+1,AdrVals,AdrCnt);
                break;
               case 2:
                CodeLen=8; WAsmCode[0]=0x0100;
                WAsmCode[1]=0x6b20+HReg;
                memcpy(WAsmCode+2,AdrVals,AdrCnt);
                break;
              END
             break;
            case ModImm:
             switch (OpSize)
              BEGIN
               case 0:
                CodeLen=2; WAsmCode[0]=0xf000+(((Word)HReg) << 8)+Lo(AdrVals[0]);
                break;
               case 1:
                CodeLen=4; WAsmCode[0]=0x7900+HReg; WAsmCode[1]=AdrVals[0];
                break;
               case 2:
                CodeLen=6; WAsmCode[0]=0x7a00+HReg;
                memcpy(WAsmCode+1,AdrVals,AdrCnt);
                break;
              END
             break;
           END
          break;
         case ModIReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[1],MModReg);
          if (AdrMode!=ModNone)
           switch (OpSize)
            BEGIN
             case 0:
              CodeLen=2; WAsmCode[0]=0x6880+(HReg << 4)+AdrPart;
              break;
             case 1:
              CodeLen=2; WAsmCode[0]=0x6980+(HReg << 4)+AdrPart;
              break;
             case 2:
              CodeLen=4; WAsmCode[0]=0x0100;
              WAsmCode[1]=0x6980+(HReg << 4)+AdrPart;
              break;
            END
           break;
         case ModPreDec:
          HReg=AdrPart;
          DecodeAdr(ArgStr[1],MModReg);
          if (AdrMode!=ModNone)
           switch (OpSize)
            BEGIN
             case 0:
              CodeLen=2; WAsmCode[0]=0x6c80+(HReg << 4)+AdrPart;
              break;
             case 1:
              CodeLen=2; WAsmCode[0]=0x6d80+(HReg << 4)+AdrPart;
              break;
             case 2:
              CodeLen=4; WAsmCode[0]=0x0100;
              WAsmCode[1]=0x6d80+(HReg << 4)+AdrPart;
              break;
            END
          break;
         case ModInd16:
          HReg=AdrPart; WAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[1],MModReg);
          if (AdrMode!=ModNone)
           switch (OpSize)
            BEGIN
             case 0:
              CodeLen=4; WAsmCode[0]=0x6e80+(HReg << 4)+AdrPart;
              break;
             case 1:
              CodeLen=4; WAsmCode[0]=0x6f80+(HReg << 4)+AdrPart;
              break;
             case 2:
              CodeLen=6; WAsmCode[0]=0x0100; WAsmCode[2]=WAsmCode[1];
              WAsmCode[1]=0x6f80+(HReg << 4)+AdrPart;
              break;
            END
           break;
         case ModInd24:
          HReg=AdrPart; memcpy(WAsmCode+2,AdrVals,4);
          DecodeAdr(ArgStr[1],MModReg);
          if (AdrMode!=ModNone)
           switch (OpSize)
            BEGIN
             case 0:
              CodeLen=8; WAsmCode[0]=0x7800+(HReg << 4);
              WAsmCode[1]=0x6aa0+AdrPart;
              break;
             case 1:
              CodeLen=8; WAsmCode[0]=0x7800+(HReg << 4);
              WAsmCode[1]=0x6ba0+AdrPart;
              break;
             case 2:
              CodeLen=10; WAsmCode[0]=0x0100;
              WAsmCode[4]=WAsmCode[3]; WAsmCode[3]=WAsmCode[2];
              WAsmCode[1]=0x7800+(HReg << 4);
              WAsmCode[2]=0x6ba0+AdrPart;
              break;
            END
          break;
         case ModAbs8:
          HReg=Lo(AdrVals[0]);
          DecodeAdr(ArgStr[1],MModReg);
          if (AdrMode!=ModNone)
           switch (OpSize)
            BEGIN
             case 0:
              CodeLen=2; WAsmCode[0]=0x3000+(((Word)AdrPart) << 8)+HReg;
              break;
             case 1:
              CodeLen=4;
              WAsmCode[0]=0x6b80+AdrPart; WAsmCode[1]=0xff00+HReg;
              break;
             case 2:
              CodeLen=6; WAsmCode[0]=0x0100;
              WAsmCode[1]=0x6b80+AdrPart; WAsmCode[2]=0xff00+HReg;
              break;
            END
          break;
         case ModAbs16:
          WAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[1],MModReg);
          if (AdrMode!=ModNone)
           switch (OpSize)
            BEGIN
             case 0:
              CodeLen=4; WAsmCode[0]=0x6a80+AdrPart;
              break;
             case 1:
              CodeLen=4; WAsmCode[0]=0x6b80+AdrPart;
              break;
             case 2:
              CodeLen=6; WAsmCode[0]=0x0100; WAsmCode[2]=WAsmCode[1];
              WAsmCode[1]=0x6b80+AdrPart;
              break;
            END
          break;
         case ModAbs24:
          memcpy(WAsmCode+1,AdrVals,4);
          DecodeAdr(ArgStr[1],MModReg);
          if (AdrMode!=ModNone)
           switch (OpSize)
            BEGIN
             case 0:
              CodeLen=6; WAsmCode[0]=0x6aa0+AdrPart;
              break;
             case 1:
              CodeLen=6; WAsmCode[0]=0x6ba0+AdrPart;
              break;
             case 2:
              CodeLen=8; WAsmCode[0]=0x0100;
              WAsmCode[3]=WAsmCode[2]; WAsmCode[2]=WAsmCode[1];
              WAsmCode[1]=0x6ba0+AdrPart;
              break;
            END
          break;
        END
      END
     return;
    END

   if ((Memo("MOVTPE")) OR (Memo("MOVFPE")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<=CPUH8_300L) WrError(1500);
     else
      BEGIN
       if (Memo("MOVTPE"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]); 
         strcpy(ArgStr[1],ArgStr[3]);
        END
       DecodeAdr(ArgStr[2],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         if (OpSize!=0) WrError(1130);
         else
          BEGIN
           HReg=AdrPart; DecodeAdr(ArgStr[1],MModAbs16);
           if (AdrMode!=ModNone)
            BEGIN
             CodeLen=4; WAsmCode[0]=0x6a40+HReg; WAsmCode[1]=AdrVals[0];
             if (Memo("MOVTPE")) WAsmCode[0]+=0x80;
            END
          END
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       z=Ord(Memo("PUSH"));
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         if (OpSize==0) WrError(1130);
         else if ((CPU16) AND (OpSize==2)) WrError(1500);
         else
          BEGIN
           if (OpSize==2) WAsmCode[0]=0x0100;
           CodeLen=2*OpSize;
           WAsmCode[(CodeLen-2) >> 1]=0x6d70+(z << 7)+AdrPart;
          END
        END
      END
     return;
    END

   if ((Memo("LDC")) OR (Memo("STC")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (Memo("STC"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         z=0x80;
        END
       else z=0;
       if (strcasecmp(ArgStr[2],"CCR")!=0) WrError(1350);
       else
        BEGIN
         SetOpSize(0);
         Mask=MModReg+MModIReg+MModInd+MModAbs16+MModAbs24;
         if (Memo("LDC")) Mask+=MModImm+MModPostInc;
         else Mask+=MModPreDec;
         DecodeAdr(ArgStr[1],Mask);
         switch (AdrMode)
          BEGIN
           case ModReg:
            CodeLen=2;
            WAsmCode[0]=0x0300+AdrPart-(z << 1);
            break;
           case ModIReg:
            CodeLen=4; WAsmCode[0]=0x0140;
            WAsmCode[1]=0x6900+z+(AdrPart << 4);
            break;
           case ModPostInc:
           case ModPreDec:
            CodeLen=4; WAsmCode[0]=0x0140;
            WAsmCode[1]=0x6d00+z+(AdrPart << 4);
            break;
           case ModInd16:
            CodeLen=6; WAsmCode[0]=0x0140; WAsmCode[2]=AdrVals[0];
            WAsmCode[1]=0x6f00+z+(AdrPart << 4);
            break;
           case ModInd24:
            CodeLen=10; WAsmCode[0]=0x0140; WAsmCode[1]=0x7800+(AdrPart << 4);
            WAsmCode[2]=0x6b20+z; memcpy(WAsmCode+3,AdrVals,AdrCnt);
            break;
           case ModAbs16:
            CodeLen=6; WAsmCode[0]=0x0140; WAsmCode[2]=AdrVals[0];
            WAsmCode[1]=0x6b00+z;
            break;
           case ModAbs24:
            CodeLen=8; WAsmCode[0]=0x0140; 
            WAsmCode[1]=0x6b20+z; memcpy(WAsmCode+2,AdrVals,AdrCnt);
            break;
           case ModImm:
            CodeLen=2; WAsmCode[0]=0x0700+Lo(AdrVals[0]);
            break;
          END
        END
      END
     return;
    END

   /* Arithmetik mit 2 Operanden */

   if ((Memo("ADD")) OR (Memo("SUB")))
    BEGIN
     z=Ord(Memo("SUB"));
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         HReg=AdrPart;
         DecodeAdr(ArgStr[1],MModReg+MModImm);
         if (AdrMode!=ModNone)
          BEGIN
           if ((CPU16) AND ((OpSize>1) OR ((OpSize==1) AND (AdrMode==ModImm)))) WrError(1500);
           else switch (AdrMode)
            BEGIN
             case ModImm:
              switch (OpSize)
               BEGIN
                case 0:
                 if (z==1) WrError(1350);
                 else
                  BEGIN
                   CodeLen=2; WAsmCode[0]=0x8000+(((Word)HReg) << 8)+Lo(AdrVals[0]);
                  END
                 break;
                case 1:
                 CodeLen=4; WAsmCode[1]=AdrVals[0];
                 WAsmCode[0]=0x7910+(z << 5)+HReg;
                 break;
                case 2:
                 CodeLen=6; memcpy(WAsmCode+1,AdrVals,4);
                 WAsmCode[0]=0x7a10+(z << 5)+HReg;
                 break;
               END
              break;
             case ModReg:
              switch (OpSize)
               BEGIN
                case 0:
                 CodeLen=2; WAsmCode[0]=0x0800+(z << 12)+(AdrPart << 4)+HReg;
                 break;
                case 1:
                 CodeLen=2; WAsmCode[0]=0x0900+(z << 12)+(AdrPart << 4)+HReg;
                 break;
                case 2:
                 CodeLen=2; WAsmCode[0]=0x0a00+(z << 12)+0x80+(AdrPart << 4)+HReg;
                 break;
               END
              break;
            END
          END
        END
      END
     return;
    END

   if (Memo("CMP"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         HReg=AdrPart;
         DecodeAdr(ArgStr[1],MModReg+MModImm);
         if (AdrMode!=ModNone)
          BEGIN
           if ((CPU16) AND ((OpSize>1) OR ((OpSize==1) AND (AdrMode==ModImm)))) WrError(1500);
           else switch (AdrMode)
            BEGIN
             case ModImm:
              switch (OpSize)
               BEGIN
                case 0:
                 CodeLen=2; WAsmCode[0]=0xa000+(((Word)HReg) << 8)+Lo(AdrVals[0]);
                 break;
                case 1:
                 CodeLen=4; WAsmCode[1]=AdrVals[0];
                 WAsmCode[0]=0x7920+HReg;
                 break;
                case 2:
                 CodeLen=6; memcpy(WAsmCode+1,AdrVals,4);
                 WAsmCode[0]=0x7a20+HReg;
               END
              break;
             case ModReg:
              switch (OpSize)
               BEGIN
                case 0:
                 CodeLen=2; WAsmCode[0]=0x1c00+(AdrPart << 4)+HReg;
                 break;
                case 1:
                 CodeLen=2; WAsmCode[0]=0x1d00+(AdrPart << 4)+HReg;
                 break;
                case 2:
                 CodeLen=2; WAsmCode[0]=0x1f80+(AdrPart << 4)+HReg;
                 break;
               END
              break;
            END
          END
        END
      END
     return;
    END

   for (z=0; z<LogicOrderCount; z++)
    if (strncmp(OpPart,LogicOrders[z].Name,strlen(LogicOrders[z].Name))==0)
     switch (OpPart[strlen(LogicOrders[z].Name)])
      BEGIN
       case '\0':
        if (ArgCnt!=2) WrError(1110);
        else
         BEGIN
          DecodeAdr(ArgStr[2],MModReg);
          if (AdrMode!=ModNone)
           BEGIN
            if ((CPU16) AND (OpSize>0)) WrError(1500);
            else
             BEGIN
              HReg=AdrPart; DecodeAdr(ArgStr[1],MModImm+MModReg);
              switch (AdrMode)
               BEGIN
                case ModImm:
                 switch (OpSize)
                  BEGIN
                   case 0:
                    CodeLen=2;
                    WAsmCode[0]=0xc000+(((Word)LogicOrders[z].Code) << 12)+(((Word)HReg) << 8)+Lo(AdrVals[0]);
                    break;
                   case 1:
                    CodeLen=4; WAsmCode[1]=AdrVals[0];
                    WAsmCode[0]=0x7940+(((Word)LogicOrders[z].Code) << 4)+HReg;
                    break;
                   case 2:
                    CodeLen=6; memcpy(WAsmCode+1,AdrVals,AdrCnt);
                    WAsmCode[0]=0x7a40+(((Word)LogicOrders[z].Code) << 4)+HReg;
                    break;
                  END
                 break;
                case ModReg:
                 switch (OpSize)
                  BEGIN
                   case 0:
                    CodeLen=2; WAsmCode[0]=0x1400+(((Word)LogicOrders[z].Code) << 8)+(AdrPart << 4)+HReg;
                    break;
                   case 1:
                    CodeLen=2; WAsmCode[0]=0x6400+(((Word)LogicOrders[z].Code) << 8)+(AdrPart << 4)+HReg;
                    break;
                   case 2:
                    CodeLen=4; WAsmCode[0]=0x01f0;
                    WAsmCode[1]=0x6400+(((Word)LogicOrders[z].Code) << 8)+(AdrPart << 4)+HReg;
                    break;
                  END
                 break;
               END
             END
           END
         END
        return;
       case 'C':
        SetOpSize(0);
        if (ArgCnt!=2) WrError(1110);
        else if (strcasecmp(ArgStr[2],"CCR")!=0) WrError(1350);
        else
         BEGIN
          DecodeAdr(ArgStr[1],MModImm);
          if (AdrMode!=ModNone)
           BEGIN
            CodeLen=2;
            WAsmCode[0]=0x0400+(((Word)LogicOrders[z].Code) << 8)+Lo(AdrVals[0]);
           END
         END
        return;
      END

   if ((Memo("ADDX")) OR (Memo("SUBX")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         if (OpSize!=0) WrError(1130);
         else
          BEGIN
           HReg=AdrPart;
           DecodeAdr(ArgStr[1],MModImm+MModReg);
           switch (AdrMode)
            BEGIN
             case ModImm:
              CodeLen=2; WAsmCode[0]=0x9000+(((Word)HReg) << 8)+Lo(AdrVals[0]);
              if (Memo("SUBX")) WAsmCode[0]+=0x2000;
              break;
             case ModReg:
              CodeLen=2; WAsmCode[0]=0x0e00+(AdrPart << 4)+HReg;
              if (Memo("SUBX")) WAsmCode[0]+=0x1000;
              break;
            END
          END
        END
      END
     return;
    END

   if ((Memo("ADDS")) OR (Memo("SUBS")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         if (((CPU16) AND (OpSize!=1)) OR ((NOT CPU16) AND (OpSize!=2))) WrError(1130);
         else
          BEGIN
           HReg=AdrPart;
           DecodeAdr(ArgStr[1],MModImm);
           if (AdrMode!=ModNone)
            BEGIN
            AdrLong=ImmVal();
             if ((AdrLong!=1) AND (AdrLong!=2) AND (AdrLong!=4)) WrError(1320);
             else
              BEGIN
               switch (AdrLong)
                BEGIN
                 case 1: WAsmCode[0]=0x0b00; break;
                 case 2: WAsmCode[0]=0x0b80; break;
                 case 4: WAsmCode[0]=0x0b90; break;
                END
               CodeLen=2; WAsmCode[0]+=HReg;
               if (Memo("SUBS")) WAsmCode[0]+=0x1000;
              END
            END
          END
        END
      END
     return;
    END

   for (z=0; z<MulOrderCount; z++)
    if (Memo(MulOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        if (OpSize!=-1) OpSize++;
        DecodeAdr(ArgStr[2],MModReg);
        if (AdrMode!=ModNone)
         BEGIN
          if (OpSize==0) WrError(1130);
          else if ((CPU16) AND (OpSize==2)) WrError(1500);
          else
           BEGIN
            HReg=AdrPart; OpSize--;
            DecodeAdr(ArgStr[1],MModReg);
            if (AdrMode!=ModNone)
             BEGIN
              if ((MulOrders[z].Code & 2)==2)
               BEGIN
                CodeLen=4; WAsmCode[0]=0x01c0;
                if ((MulOrders[z].Code & 1)==1) WAsmCode[0]+=0x10;
               END
              else CodeLen=2;
              WAsmCode[CodeLen >> 2]=0x5000
                               +(((Word)OpSize) << 9)
                               +(((Word)MulOrders[z].Code & 1) << 8)
                               +(AdrPart << 4)+HReg;
             END
           END
         END
       END
      return;
     END

   /* Bitoperationen */

   for (z=0; z<Bit1OrderCount; z++)
    if (Memo(Bit1Orders[z].Name))
     BEGIN
      OpCode=0x60+(Bit1Orders[z].Code & 0x7f);
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        if (*ArgStr[1]!='#') WrError(1350);
        else
         BEGIN
          HReg=EvalIntExpression(ArgStr[1]+1,UInt3,&OK);
          if (OK)
           BEGIN
            DecodeAdr(ArgStr[2],MModReg+MModIReg+MModAbs8);
            if (AdrMode!=ModNone)
             BEGIN
              if (OpSize>0) WrError(1130);
              else switch (AdrMode)
               BEGIN
                case ModReg:
                 CodeLen=2;
                 WAsmCode[0]=(((Word)OpCode) << 8)+(Bit1Orders[z].Code & 0x80)+(HReg << 4)+AdrPart;
                 break;
                case ModIReg:
                 CodeLen=4;
                 WAsmCode[0]=0x7c00+(AdrPart << 4);
                 WAsmCode[1]=(((Word)OpCode) << 8)+(Bit1Orders[z].Code & 0x80)+(HReg << 4);
                 if (OpCode<0x70) WAsmCode[0]+=0x100;
                 break;
                case ModAbs8:
                 CodeLen=4;
                 WAsmCode[0]=0x7e00+Lo(AdrVals[0]);
                 WAsmCode[1]=(((Word)OpCode) << 8)+(Bit1Orders[z].Code & 0x80)+(HReg << 4);
                 if (OpCode<0x70) WAsmCode[0]+=0x100;
                 break;
               END
             END
           END
         END
       END
      return;
     END

   for (z=0; z<Bit2OrderCount; z++)
    if (Memo(Bit2Orders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        if (*ArgStr[1]=='#')
         BEGIN
          OpCode=Bit2Orders[z].Code+0x70;
          HReg=EvalIntExpression(ArgStr[1]+1,UInt3,&OK);
         END
        else
         BEGIN
          OpCode=Bit2Orders[z].Code+0x60;
          OK=DecodeReg(ArgStr[1],&HReg,&HSize);
          if (NOT OK) WrError(1350);
          if ((OK) AND (HSize!=0))
           BEGIN
            WrError(1130); OK=False;
           END
         END
        if (OK)
         BEGIN
          DecodeAdr(ArgStr[2],MModReg+MModIReg+MModAbs8);
          if (AdrMode!=ModNone)
           BEGIN
            if (OpSize>0) WrError(1130);
            else switch (AdrMode)
             BEGIN
              case ModReg:
               CodeLen=2;
               WAsmCode[0]=(((Word)OpCode) << 8)+(HReg << 4)+AdrPart;
               break;
              case ModIReg:
               CodeLen=4;
               WAsmCode[0]=0x7d00+(AdrPart << 4);
               WAsmCode[1]=(((Word)OpCode) << 8)+(HReg << 4);
               if (Bit2Orders[z].Code==3) WAsmCode[0]-=0x100;
               break;
              case ModAbs8:
               CodeLen=4;
               WAsmCode[0]=0x7f00+Lo(AdrVals[0]);
               WAsmCode[1]=(((Word)OpCode) << 8)+(HReg << 4);
               if (Bit2Orders[z].Code==3) WAsmCode[0]-=0x100;
               break;
             END
           END
         END
       END
      return;
     END

   /* Read/Modify/Write-Operationen */

   if ((Memo("INC")) OR (Memo("DEC")))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[ArgCnt],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         if ((OpSize>0) AND (CPU16)) WrError(1500);
         else
          BEGIN
           HReg=AdrPart;
           if (ArgCnt==1)
            BEGIN
             OK=True; z=1;
            END
           else
            BEGIN
             DecodeAdr(ArgStr[1],MModImm);
             OK=(AdrMode==ModImm);
             if (OK)
              BEGIN
               z=ImmVal();
               if (z<1)
                BEGIN
                 WrError(1315); OK=False;
                END
               else if (((OpSize==0) AND (z>1)) OR (z>2))
                BEGIN
                 WrError(1320); OK=False;
                END
              END
            END
           if (OK)
            BEGIN
             CodeLen=2; z--;
             switch (OpSize)
              BEGIN
               case 0:WAsmCode[0]=0x0a00+HReg; break;
               case 1:WAsmCode[0]=0x0b50+HReg+(z << 7); break;
               case 2:WAsmCode[0]=0x0b70+HReg+(z << 7);
              END
             if (Memo("DEC")) WAsmCode[0]+=0x1000;
            END
          END
        END
      END
     return;
    END

   for (z=0; z<ShiftOrderCount; z++)
    if (Memo(ShiftOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg);
        if (AdrMode!=ModNone)
         BEGIN
          if ((OpSize>0) AND (CPU16)) WrError(1500);
          else
           BEGIN
            CodeLen=2;
            switch (OpSize)
             BEGIN
              case 0: WAsmCode[0]=ShiftOrders[z].Code+AdrPart; break;
              case 1: WAsmCode[0]=ShiftOrders[z].Code+AdrPart+0x10; break;
              case 2: WAsmCode[0]=ShiftOrders[z].Code+AdrPart+0x30; break;
             END
           END
         END
       END
      return;
     END

   if ((Memo("NEG")) OR (Memo("NOT")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         if ((OpSize>0) AND (CPU16)) WrError(1500);
         else
          BEGIN
           CodeLen=2;
           switch (OpSize)
            BEGIN
             case 0:WAsmCode[0]=0x1700+AdrPart; break;
             case 1:WAsmCode[0]=0x1710+AdrPart; break;
             case 2:WAsmCode[0]=0x1730+AdrPart; break;
            END
           if (Memo("NEG")) WAsmCode[0]+=0x80;
          END
        END
      END
     return;
    END

   if ((Memo("EXTS")) OR (Memo("EXTU")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (CPU16) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
         else
          BEGIN
           CodeLen=2;
           switch (OpSize)
            BEGIN
             case 1: WAsmCode[0]=(Memo("EXTS")) ? 0x17d0 : 0x1750; break;
             case 2: WAsmCode[0]=(Memo("EXTS")) ? 0x17f0 : 0x1770; break;
            END
           WAsmCode[0]+=AdrPart;
          END
        END
      END
     return;
    END

   if ((Memo("DAA")) OR (Memo("DAS")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode!=ModNone)
        BEGIN
         if (OpSize!=0) WrError(1130);
         else
          BEGIN
           CodeLen=2;
           WAsmCode[0]=0x0f00+AdrPart;
           if (Memo("DAS")) WAsmCode[0]+=0x1000;
          END
        END
      END
     return;
    END

   /* Spruenge */

   for (z=0; z<ConditionCount; z++)
    if (Memo(Conditions[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if ((OpSize!=-1) AND (OpSize!=4) AND (OpSize!=2)) WrError(1130);
      else
       BEGIN
        AdrLong=EvalIntExpression(ArgStr[1],Int24,&OK)-(EProgCounter()+2);
        if (OK)
         BEGIN
          if (OpSize==-1)
          if ((AdrLong>=-128) AND (AdrLong<=127)) OpSize=4;
          else
           BEGIN
            OpSize=2; AdrLong-=2;
           END
          else if (OpSize==2) AdrLong-=2;
          if (OpSize==2)
           BEGIN
            if ((NOT SymbolQuestionable) AND ((AdrLong<-32768) OR (AdrLong>32767))) WrError(1370);
            else if (CPU16) WrError(1500);
            else
             BEGIN
              CodeLen=4;
              WAsmCode[0]=0x5800+(Conditions[z].Code << 4); WAsmCode[1]=AdrLong & 0xffff;
             END
           END
          else
           BEGIN
            if ((NOT SymbolQuestionable) AND ((AdrLong<-128) OR (AdrLong>127))) WrError(1370);
            else
             BEGIN
              CodeLen=2;
              WAsmCode[0]=0x4000+(((Word)Conditions[z].Code) << 8)+(AdrLong & 0xff);
             END
           END
         END
       END
      return;
     END

   if ((Memo("JMP")) OR (Memo("JSR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       z=Ord(Memo("JSR")) << 10;
       DecodeAdr(ArgStr[1],MModIReg+((CPU16)?MModAbs16:MModAbs24)+MModIIAbs);
       switch (AdrMode)
        BEGIN
         case ModIReg:
          CodeLen=2; WAsmCode[0]=0x5900+z+(AdrPart << 4);
          break;
         case ModAbs16:
          CodeLen=4; WAsmCode[0]=0x5a00+z; WAsmCode[1]=AdrVals[0];
          break;
         case ModAbs24:
          CodeLen=4; WAsmCode[0]=0x5a00+z+Lo(AdrVals[0]);
          WAsmCode[1]=AdrVals[1];
          break;
         case ModIIAbs:
          CodeLen=2; WAsmCode[0]=0x5b00+z+Lo(AdrVals[0]);
          break;
        END
      END
     return;
    END

   if (Memo("BSR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if ((OpSize!=-1) AND (OpSize!=4) AND (OpSize!=2)) WrError(1130);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],Int24,&OK)-(EProgCounter()+2);
       if (OK)
        BEGIN
         if (OpSize==-1)
         if ((AdrLong>=-128) AND (AdrLong<=127)) OpSize=4;
         else
          BEGIN
           OpSize=2; AdrLong-=2;
          END
         else if (OpSize==2) AdrLong-=2;
         if (OpSize==2)
          BEGIN
           if ((NOT SymbolQuestionable) AND ((AdrLong<-32768) OR (AdrLong>32767))) WrError(1370);
           else if (CPU16) WrError(1500);
           else
            BEGIN
             CodeLen=4;
             WAsmCode[0]=0x5c00; WAsmCode[1]=AdrLong & 0xffff;
            END
          END
         else
          BEGIN
           if ((AdrLong<-128) OR (AdrLong>127)) WrError(1370);
           else
            BEGIN
             CodeLen=2;
             WAsmCode[0]=0x5500+(AdrLong & 0xff);
            END
          END
        END
      END
     return;
    END

   if (Memo("TRAPA"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU6413309) WrError(1500);
     else
      BEGIN
       if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
       WAsmCode[0]=EvalIntExpression(ArgStr[1],UInt2,&OK)<<4;
       if (OK)
        BEGIN
         WAsmCode[0]+=0x5700; CodeLen=2;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_H8_3(void)
BEGIN
   return False;
END

        static void SwitchFrom_H8_3(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void SwitchTo_H8_3(void)
BEGIN
   TurnWords=True; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x68; NOPCode=0x0000;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   SegLimits[SegCode] = (MomCPU <= CPUH8_300) ? 0xffff : 0xffffffl;

   MakeCode=MakeCode_H8_3; IsDef=IsDef_H8_3;
   SwitchFrom=SwitchFrom_H8_3; InitFields();
   AddONOFF("MAXMODE", &Maximum   , MaximumName   ,False);
   AddONOFF("PADDING", &DoPadding , DoPaddingName ,False);
   AddMoto16PseudoONOFF();

   CPU16=(MomCPU<=CPUH8_300);

   SetFlag(&DoPadding,DoPaddingName,False);
END

        void codeh8_3_init(void)
BEGIN
   CPUH8_300L=AddCPU("H8/300L"   ,SwitchTo_H8_3);
   CPU6413308=AddCPU("HD6413308" ,SwitchTo_H8_3);
   CPUH8_300 =AddCPU("H8/300"    ,SwitchTo_H8_3);
   CPU6413309=AddCPU("HD6413309" ,SwitchTo_H8_3);
   CPUH8_300H=AddCPU("H8/300H"   ,SwitchTo_H8_3);
END
