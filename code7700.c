/* code7700.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegeneratormodul MELPS-7700                                          */
/*                                                                           */
/* Historie:  5.11.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#include "code7700.h"

typedef struct
         {
          char *Name;
          Word Code;
          Byte Allowed;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Code;
          ShortInt Disp8,Disp16;
         } RelOrder;

typedef struct
         {   
          char *Name;
          Byte Code;
         } AccOrder;

typedef struct
         {
          char *Name;
          Byte ACode,MCode;
         } RMWOrder;

typedef struct
         {
          char *Name;
          Byte CodeImm,CodeAbs8,CodeAbs16,CodeIdxX8,CodeIdxX16,
               CodeIdxY8,CodeIdxY16;
         } XYOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Byte Allowed;
         } MulDivOrder;

#define ModNone    (-1)
#define ModImm      0
#define MModImm      (1l << ModImm)
#define ModAbs8     1
#define MModAbs8     (1l << ModAbs8)
#define ModAbs16    2
#define MModAbs16    (1l << ModAbs16)
#define ModAbs24    3
#define MModAbs24    (1l << ModAbs24)
#define ModIdxX8    4
#define MModIdxX8    (1l << ModIdxX8)
#define ModIdxX16   5
#define MModIdxX16   (1l << ModIdxX16)
#define ModIdxX24   6
#define MModIdxX24   (1l << ModIdxX24)
#define ModIdxY8    7
#define MModIdxY8    (1l << ModIdxY8)
#define ModIdxY16   8
#define MModIdxY16   (1l << ModIdxY16)
#define ModIdxY24   9
#define MModIdxY24   (1l << ModIdxY24)
#define ModInd8    10
#define MModInd8     (1l << ModInd8)
#define ModInd16   11
#define MModInd16    (1l << ModInd16)
#define ModInd24   12
#define MModInd24    (1l << ModInd24)
#define ModIndX8   13
#define MModIndX8    (1l << ModIndX8)
#define ModIndX16  14
#define MModIndX16   (1l << ModIndX16)
#define ModIndX24  15
#define MModIndX24   (1l << ModIndX24)
#define ModIndY8   16
#define MModIndY8    (1l << ModIndY8)
#define ModIndY16  17
#define MModIndY16   (1l << ModIndY16)
#define ModIndY24  18
#define MModIndY24   (1l << ModIndY24)
#define ModIdxS8   19
#define MModIdxS8    (1l << ModIdxS8)
#define ModIndS8   20
#define MModIndS8    (1l << ModIndS8)

#define FixedOrderCnt 64

#define RelOrderCnt 13

#define AccOrderCnt 9

#define RMWOrderCnt 6

#define Imm8OrderCnt 5

#define XYOrderCnt 6

#define MulDivOrderCnt 4

#define PushRegCnt 8
static char *PushRegs[PushRegCnt]={"A","B","X","Y","DPR","DT","PG","PS"};

#define PrefAccB 0x42

static LongInt Reg_PG,Reg_DT,Reg_X,Reg_M,Reg_DPR,BankReg;

static Boolean WordSize;
static Byte AdrVals[3];
static ShortInt AdrType;
static Boolean LFlag;

static FixedOrder *FixedOrders;
static RelOrder *RelOrders;
static AccOrder *AccOrders;
static RMWOrder *RMWOrders;
static FixedOrder *Imm8Orders;
static XYOrder *XYOrders;
static MulDivOrder *MulDivOrders;

static SimpProc SaveInitProc;

static CPUVar CPU65816,CPUM7700,CPUM7750,CPUM7751;

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode, Byte NAllowed)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ++].Allowed=NAllowed;
END

        static void AddRel(char *NName, Word NCode, ShortInt NDisp8, ShortInt NDisp16)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ].Code=NCode;
   RelOrders[InstrZ].Disp8=NDisp8;
   RelOrders[InstrZ++].Disp16=NDisp16;
END

        static void AddAcc(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=NName;
   AccOrders[InstrZ++].Code=NCode;
END

        static void AddRMW(char *NName, Byte NACode, Byte NMCode)
BEGIN
   if (InstrZ>=RMWOrderCnt) exit(255);
   RMWOrders[InstrZ].Name=NName;
   RMWOrders[InstrZ].MCode=NMCode;
   RMWOrders[InstrZ++].ACode=NACode;
END

        static void AddImm8(char *NName, Word NCode, Byte NAllowed)
BEGIN
   if (InstrZ>=Imm8OrderCnt) exit(255);
   Imm8Orders[InstrZ].Name=NName;
   Imm8Orders[InstrZ].Code=NCode;
   Imm8Orders[InstrZ++].Allowed=NAllowed;
END

        static void AddXY(char *NName, Byte NCodeImm, Byte NCodeAbs8, Byte NCodeAbs16,
                          Byte NCodeIdxX8, Byte NCodeIdxX16, Byte NCodeIdxY8,
                          Byte NCodeIdxY16)
BEGIN
   if (InstrZ>=XYOrderCnt) exit(255);
   XYOrders[InstrZ].Name=NName;
   XYOrders[InstrZ].CodeImm=NCodeImm;
   XYOrders[InstrZ].CodeAbs8=NCodeAbs8;
   XYOrders[InstrZ].CodeAbs16=NCodeAbs16;
   XYOrders[InstrZ].CodeIdxX8=NCodeIdxX8;
   XYOrders[InstrZ].CodeIdxX16=NCodeIdxX16;
   XYOrders[InstrZ].CodeIdxY8=NCodeIdxY8;
   XYOrders[InstrZ++].CodeIdxY16=NCodeIdxY16;
END

        static void AddMulDiv(char *NName, Word NCode, Byte NAllowed)
BEGIN
   if (InstrZ>=MulDivOrderCnt) exit(255);
   MulDivOrders[InstrZ].Name=NName;
   MulDivOrders[InstrZ].Code=NCode;
   MulDivOrders[InstrZ++].Allowed=NAllowed;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CLC",0x0018,15); AddFixed("CLI",0x0058,15);
   AddFixed("CLM",0x00d8,14); AddFixed("CLV",0x00b8,15);
   AddFixed("DEX",0x00ca,15); AddFixed("DEY",0x0088,15);
   AddFixed("INX",0x00e8,15); AddFixed("INY",0x00c8,15);
   AddFixed("NOP",0x00ea,15); AddFixed("PHA",0x0048,15);
   AddFixed("PHD",0x000b,15); AddFixed("PHG",0x004b,14);
   AddFixed("PHP",0x0008,15); AddFixed("PHT",0x008b,14);
   AddFixed("PHX",0x00da,15); AddFixed("PHY",0x005a,15);
   AddFixed("PLA",0x0068,15); AddFixed("PLD",0x002b,15);
   AddFixed("PLP",0x0028,15); AddFixed("PLT",0x00ab,14);
   AddFixed("PLX",0x00fa,15); AddFixed("PLY",0x007a,15);
   AddFixed("RTI",0x0040,15); AddFixed("RTL",0x006b,15);
   AddFixed("RTS",0x0060,15); AddFixed("SEC",0x0038,15);
   AddFixed("SEI",0x0078,15); AddFixed("SEM",0x00f8,14);
   AddFixed("STP",0x00db,15); AddFixed("TAD",0x005b,15);
   AddFixed("TAS",0x001b,15); AddFixed("TAX",0x00aa,15);
   AddFixed("TAY",0x00a8,15); AddFixed("TBD",0x425b,14);
   AddFixed("TBS",0x421b,14); AddFixed("TBX",0x42aa,14);
   AddFixed("TBY",0x42a8,14); AddFixed("TDA",0x007b,15);
   AddFixed("TDB",0x427b,14); AddFixed("TSA",0x003b,15);
   AddFixed("TSX",0x00ba,15); AddFixed("TXA",0x008a,15);
   AddFixed("TXB",0x428a,14); AddFixed("TXS",0x009a,15);
   AddFixed("TXY",0x009b,15); AddFixed("TYA",0x0098,15);
   AddFixed("TYB",0x4298,15); AddFixed("TYX",0x00bb,15);
   AddFixed("WIT",0x00cb,14); AddFixed("XAB",0x8928,14);
   AddFixed("COP",0x0002, 1); AddFixed("CLD",0x00d8, 1);
   AddFixed("SED",0x00f8, 1); AddFixed("TCS",0x001b,15);
   AddFixed("TSC",0x003b,15); AddFixed("TCD",0x005b,15);
   AddFixed("TDC",0x007b,15); AddFixed("PHK",0x004b, 1);
   AddFixed("WAI",0x00cb, 1); AddFixed("XBA",0x00eb, 1);
   AddFixed("SWA",0x00eb, 1); AddFixed("XCE",0x00fb, 1);
   AddFixed("DEA",(MomCPU>=CPUM7700) ? 0x001a : 0x003a,15);
   AddFixed("INA",(MomCPU>=CPUM7700) ? 0x003a : 0x001a,15);

   RelOrders=(RelOrder *) malloc(sizeof(RelOrder)*RelOrderCnt); InstrZ=0;
   AddRel("BCC" ,0x0090, 2,-1);
   AddRel("BLT" ,0x0090, 2,-1);
   AddRel("BCS" ,0x00b0, 2,-1);
   AddRel("BGE" ,0x00b0, 2,-1);
   AddRel("BEQ" ,0x00f0, 2,-1);
   AddRel("BMI" ,0x0030, 2,-1);
   AddRel("BNE" ,0x00d0, 2,-1);
   AddRel("BPL" ,0x0010, 2,-1);
   AddRel("BRA" ,0x8280, 2, 3);
   AddRel("BVC" ,0x0050, 2,-1);
   AddRel("BVS" ,0x0070, 2,-1);
   AddRel("BRL" ,0x8200,-1, 3);
   AddRel("BRAL",0x8200,-1, 3);

   AccOrders=(AccOrder *) malloc(sizeof(AccOrder)*AccOrderCnt); InstrZ=0;
   AddAcc("ADC",0x60);
   AddAcc("AND",0x20);
   AddAcc("CMP",0xc0);
   AddAcc("CPA",0xc0);
   AddAcc("EOR",0x40);
   AddAcc("LDA",0xa0);
   AddAcc("ORA",0x00);
   AddAcc("SBC",0xe0);
   AddAcc("STA",0x80);

   RMWOrders=(RMWOrder *) malloc(sizeof(RMWOrder)*RMWOrderCnt); InstrZ=0;
   AddRMW("ASL",0x0a,0x06);
   AddRMW("DEC",(MomCPU>=CPUM7700)?0x1a:0x3a,0xc6);
   AddRMW("ROL",0x2a,0x26);
   AddRMW("INC",(MomCPU>=CPUM7700)?0x3a:0x1a,0xe6);
   AddRMW("LSR",0x4a,0x46);
   AddRMW("ROR",0x6a,0x66);

   Imm8Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Imm8OrderCnt); InstrZ=0;
   AddImm8("CLP",0x00c2,15);
   AddImm8("REP",0x00c2,15);
   AddImm8("LDT",0x89c2,14);
   AddImm8("SEP",0x00e2,15);
   AddImm8("RMPA",0x89e2,8);

   XYOrders=(XYOrder *) malloc(sizeof(XYOrder)*XYOrderCnt); InstrZ=0;
   AddXY("CPX",0xe0,0xe4,0xec,0xff,0xff,0xff,0xff);
   AddXY("CPY",0xc0,0xc4,0xcc,0xff,0xff,0xff,0xff);
   AddXY("LDX",0xa2,0xa6,0xae,0xff,0xff,0xb6,0xbe);
   AddXY("LDY",0xa0,0xa4,0xac,0xb4,0xbc,0xff,0xff);
   AddXY("STX",0xff,0x86,0x8e,0xff,0xff,0x96,0xff);
   AddXY("STY",0xff,0x84,0x8c,0x94,0xff,0xff,0xff);

   MulDivOrders=(MulDivOrder *) malloc(sizeof(MulDivOrder)*MulDivOrderCnt); InstrZ=0;
   AddMulDiv("MPY",0x0000,14); AddMulDiv("MPYS",0x0080,12);
   AddMulDiv("DIV",0x0020,14); AddMulDiv("DIVS",0x00a0,12); /*???*/
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(RelOrders);
   free(AccOrders);
   free(RMWOrders);
   free(Imm8Orders);
   free(XYOrders);
   free(MulDivOrders);
END

/*---------------------------------------------------------------------------*/

        static void ChkAdr(LongWord Mask)
BEGIN
   if (AdrType!=ModNone)
    if ((Mask & (1l << ((LongWord)AdrType)))==0)
     BEGIN
      AdrType=ModNone; AdrCnt=0; WrError(1350);
     END
END


        static void CodeDisp(char *Asc, LongInt Start, LongWord Mask)
BEGIN
   Boolean OK;
   LongInt Adr;
   ShortInt DType;
   int l=strlen(Asc);

   if ((l>1) AND (*Asc=='<'))
    BEGIN
     Asc++; DType=0;
    END
   else if ((l>1) AND (*Asc=='>'))
    if ((l>2) AND (Asc[1]=='>'))
     BEGIN
      Asc+=2; DType=2;
     END
    else
     BEGIN
      Asc++; DType=1;
     END
   else DType=(-1);

   Adr=EvalIntExpression(Asc,UInt24,&OK);

   if (NOT OK) return;

   if (DType==-1)
    BEGIN
     if ((((Mask & (1l << Start)))!=0) AND (Adr>=Reg_DPR) AND (Adr<Reg_DPR+0x100)) DType=0;
     else if ((((Mask & (2l << Start)))!=0) AND ((Adr >> 16)==BankReg)) DType=1;
     else DType=2;
    END

   if ((Mask & (1l << (Start+DType)))==0) WrError(1350);
   else switch (DType)
    BEGIN
     case 0:
      if ((FirstPassUnknown) OR (ChkRange(Adr,Reg_DPR,Reg_DPR+0xff)))
       BEGIN
        AdrCnt=1; AdrType=Start;
        AdrVals[0]=Lo(Adr-Reg_DPR);
       END;
      break;
     case 1:
      if ((NOT FirstPassUnknown) AND ((Adr >> 16)!=BankReg)) WrError(1320);
      else
       BEGIN
        AdrCnt=2; AdrType=Start+1;
        AdrVals[0]=Lo(Adr); AdrVals[1]=Hi(Adr);
       END
      break;
     case 2:
      AdrCnt=3; AdrType=Start+2;
      AdrVals[0]=Lo(Adr); AdrVals[1]=Hi(Adr); AdrVals[2]=Adr >> 16;
      break;
    END
END

        static void SplitArg(char *Src, String *HStr, Integer *HCnt)
BEGIN
   char *p;

   strcpy(Src,Src+1); Src[strlen(Src)-1]='\0';
   p=QuotPos(Src,',');
   if (p==Nil)
    BEGIN
     strmaxcpy(HStr[0],Src,255); *HCnt=1;
    END
   else
    BEGIN
     *p='\0';
     strmaxcpy(HStr[0],Src,255);
     strmaxcpy(HStr[1],p+1,255);
     *p=',';
     *HCnt=2;
    END
END

        static void DecodeAdr(Integer Start, LongWord Mask)
BEGIN
   Word AdrWord;
   Boolean OK;
   Integer HCnt;
   String HStr[2];

   AdrType=ModNone; AdrCnt=0; BankReg=Reg_DT;

   /* I. 1 Parameter */

   if (Start==ArgCnt)
    BEGIN
     /* I.1. immediate */

     if (*ArgStr[Start]=='#')
      BEGIN
       if (WordSize)
        BEGIN
         AdrWord=EvalIntExpression(ArgStr[Start]+1,Int16,&OK);
         AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
        END
       else AdrVals[0]=EvalIntExpression(ArgStr[Start]+1,Int8,&OK);
       if (OK)
        BEGIN
         AdrCnt=1+Ord(WordSize); AdrType=ModImm;
        END
       ChkAdr(Mask); return;
      END

     /* I.2. indirekt */

     if (IsIndirect(ArgStr[Start]))
      BEGIN
       SplitArg(ArgStr[Start],HStr,&HCnt);

       /* I.2.i. einfach indirekt */

       if (HCnt==1)
        BEGIN
         CodeDisp(HStr[0],ModInd8,Mask); ChkAdr(Mask); return;
        END

       /* I.2.ii indirekt mit Vorindizierung */

       else if (strcasecmp(HStr[1],"X")==0)
        BEGIN
         CodeDisp(HStr[0],ModIndX8,Mask); ChkAdr(Mask); return;
        END

       else
        BEGIN
         WrError(1350); ChkAdr(Mask); return;
        END
      END

     /* I.3. absolut */

     else
      BEGIN
       CodeDisp(ArgStr[Start],ModAbs8,Mask); ChkAdr(Mask); return;
      END
    END

   /* II. 2 Parameter */

   else if (Start+1==ArgCnt)
    BEGIN
     /* II.1 indirekt mit Nachindizierung */

     if (IsIndirect(ArgStr[Start]))
      BEGIN
       if (strcasecmp(ArgStr[Start+1],"Y")!=0) WrError(1350);
       else
        BEGIN
         SplitArg(ArgStr[Start],HStr,&HCnt);

         /* II.1.i. (d),Y */

         if (HCnt==1)
          BEGIN
           CodeDisp(HStr[0],ModIndY8,Mask); ChkAdr(Mask); return;
          END

         /* II.1.ii. (d,S),Y */

         else if (strcasecmp(HStr[1],"S")==0)
          BEGIN
           AdrVals[0]=EvalIntExpression(HStr[0],Int8,&OK);
           if (OK)
            BEGIN
             AdrType=ModIndS8; AdrCnt=1;
            END
           ChkAdr(Mask); return;
          END

         else WrError(1350);
        END
       ChkAdr(Mask); return;
      END

     /* II.2. einfach indiziert */

     else
      BEGIN
       /* II.2.i. d,X */

       if (strcasecmp(ArgStr[Start+1],"X")==0)
        BEGIN
         CodeDisp(ArgStr[Start],ModIdxX8,Mask); ChkAdr(Mask); return;
        END

       /* II.2.ii. d,Y */

       else if (strcasecmp(ArgStr[Start+1],"Y")==0)
        BEGIN
         CodeDisp(ArgStr[Start],ModIdxY8,Mask); ChkAdr(Mask); return;
        END

       /* II.2.iii. d,S */

       else if (strcasecmp(ArgStr[Start+1],"S")==0)
        BEGIN
         AdrVals[0]=EvalIntExpression(ArgStr[Start],Int8,&OK);
         if (OK)
          BEGIN
           AdrType=ModIdxS8; AdrCnt=1;
          END
         ChkAdr(Mask); return;
        END

       else WrError(1350);
      END
    END

   else WrError(1110);
END

        static Boolean DecodePseudo(void)
BEGIN
#define ASSUME7700Count 5
static ASSUMERec ASSUME7700s[ASSUME7700Count]=
               {{"PG" , &Reg_PG , 0,  0xff,  0x100},
                {"DT" , &Reg_DT , 0,  0xff,  0x100},
                {"X"  , &Reg_X  , 0,     1,     -1},
                {"M"  , &Reg_M  , 0,     1,     -1},
                {"DPR", &Reg_DPR, 0,0xffff,0x10000}};

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME7700s,ASSUME7700Count);
     return True;
    END

   return False;
END

        static Boolean LMemo(char *s)
BEGIN
   String tmp;

   if (Memo(s)) 
    BEGIN
     LFlag=False; return True;
    END
   else
    BEGIN
     strmaxcpy(tmp,s,255); strmaxcat(tmp,"L",255);
     if (Memo(tmp))
      BEGIN
       LFlag=True; return True;
      END
     else return False;
    END
END

        static void MakeCode_7700(void)
BEGIN
   int z;
   Integer Start;
   LongInt AdrLong,Mask;
   Boolean OK,Rel;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(False)) return;
   if (DecodeIntelPseudo(False)) return;

   /* ohne Argument */

   if (Memo("BRK"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; BAsmCode[0]=0x00; BAsmCode[1]=NOPCode;
      END
     return;
    END

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (((FixedOrders[z].Allowed >> (MomCPU-CPU65816))&1)==0) WrError(1500);
      else
       BEGIN
        CodeLen=1+Ord(Hi(FixedOrders[z].Code)!=0);
        if (CodeLen==2) BAsmCode[0]=Hi(FixedOrders[z].Code);
        BAsmCode[CodeLen-1]=Lo(FixedOrders[z].Code);
       END
      return;
     END

   if ((Memo("PHB")) OR (Memo("PLB")))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       if (MomCPU>=CPUM7700)
        BEGIN
         CodeLen=2; BAsmCode[0]=PrefAccB; BAsmCode[1]=0x48;
        END
       else
        BEGIN
         CodeLen=1; BAsmCode[0]=0x8b;
        END;
       if (Memo("PLB")) BAsmCode[CodeLen-1]+=0x20;
      END
     return;
    END

   /* relative Adressierung */

   for (z=0; z<RelOrderCnt; z++)
    if (Memo(RelOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
        AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK);
        if (OK)
         BEGIN
          OK=RelOrders[z].Disp8==-1;
          if (OK) AdrLong-=EProgCounter()+RelOrders[z].Disp16;
          else
           BEGIN
            AdrLong-=EProgCounter()+RelOrders[z].Disp8;
            if (((AdrLong>127) OR (AdrLong<-128)) AND (NOT SymbolQuestionable) AND (RelOrders[z].Disp16!=-1))
             BEGIN
              OK=True; AdrLong-=RelOrders[z].Disp16-RelOrders[z].Disp8;
             END
           END
          if (OK)            /* d16 */
           if (((AdrLong<-32768) OR (AdrLong>32767)) AND (NOT SymbolQuestionable)) WrError(1330);
           else
            BEGIN
             CodeLen=3; BAsmCode[0]=Hi(RelOrders[z].Code);
             BAsmCode[1]=Lo(AdrLong); BAsmCode[2]=Hi(AdrLong);
            END
          else               /* d8 */
           if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT SymbolQuestionable)) WrError(1370);
           else
            BEGIN
             CodeLen=2; BAsmCode[0]=Lo(RelOrders[z].Code);
             BAsmCode[1]=Lo(AdrLong);
            END
         END
       END
      return;
     END

   /* mit Akku */

   for (z=0; z<AccOrderCnt; z++)
    if (LMemo(AccOrders[z].Name))
     BEGIN
      if ((ArgCnt==0) OR (ArgCnt>3)) WrError(1110);
      else
       BEGIN
        WordSize=(Reg_M==0);
        if (strcasecmp(ArgStr[1],"A")==0) Start=2;
        else if (strcasecmp(ArgStr[1],"B")==0)
         BEGIN
          Start=2; BAsmCode[0]=PrefAccB; CodeLen++;
          if (MomCPU==CPU65816)
           BEGIN
            WrError(1505); return;
           END
         END
        else Start=1;
        Mask=MModAbs8+MModAbs16+MModAbs24+
             MModIdxX8+MModIdxX16+MModIdxX24+
             MModIdxY16+
             MModInd8+MModIndX8+MModIndY8+
             MModIdxS8+MModIndS8;
        if (NOT LMemo("STA")) Mask+=MModImm;
        DecodeAdr(Start,Mask);
        if (AdrType!=ModNone)
         BEGIN
          if ((LFlag) AND (AdrType!=ModInd8) AND (AdrType!=ModIndY8)) WrError(1350);
          else
           BEGIN
            switch (AdrType)
             BEGIN
              case ModImm    : BAsmCode[CodeLen]=AccOrders[z].Code+0x09; break;
              case ModAbs8   : BAsmCode[CodeLen]=AccOrders[z].Code+0x05; break;
              case ModAbs16  : BAsmCode[CodeLen]=AccOrders[z].Code+0x0d; break;
              case ModAbs24  : BAsmCode[CodeLen]=AccOrders[z].Code+0x0f; break;
              case ModIdxX8  : BAsmCode[CodeLen]=AccOrders[z].Code+0x15; break;
              case ModIdxX16 : BAsmCode[CodeLen]=AccOrders[z].Code+0x1d; break;
              case ModIdxX24 : BAsmCode[CodeLen]=AccOrders[z].Code+0x1f; break;
              case ModIdxY16 : BAsmCode[CodeLen]=AccOrders[z].Code+0x19; break;
              case ModInd8   : if (LFlag) BAsmCode[CodeLen]=AccOrders[z].Code+0x07;
                               else BAsmCode[CodeLen]=AccOrders[z].Code+0x12; break;
              case ModIndX8  : BAsmCode[CodeLen]=AccOrders[z].Code+0x01; break;
              case ModIndY8  : if (LFlag) BAsmCode[CodeLen]=AccOrders[z].Code+0x17;
                               else BAsmCode[CodeLen]=AccOrders[z].Code+0x11; break;
              case ModIdxS8  : BAsmCode[CodeLen]=AccOrders[z].Code+0x03; break;
              case ModIndS8  : BAsmCode[CodeLen]=AccOrders[z].Code+0x13; break;
             END
            memcpy(BAsmCode+CodeLen+1,AdrVals,AdrCnt); CodeLen+=1+AdrCnt;
           END
         END
       END
     return;
    END

   if ((Memo("EXTS")) OR (Memo("EXTZ")))
    BEGIN
     if (ArgCnt==0)
      BEGIN
       strmaxcpy(ArgStr[1],"A",255); ArgCnt=1;
      END
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPUM7750) WrError(1500);
     else
      BEGIN
       BAsmCode[1]=0x8b+(Ord(Memo("EXTZ")) << 5);
       BAsmCode[0]=0;
       if (strcasecmp(ArgStr[1],"A")==0) BAsmCode[0]=0x89;
       else if (strcasecmp(ArgStr[1],"B")==0) BAsmCode[0]=0x42;
       else WrError(1350);
       if (BAsmCode[0]!=0) CodeLen=2;
      END
     return;
    END

   for (z=0; z<RMWOrderCnt; z++)
    if (Memo(RMWOrders[z].Name))
     BEGIN
      if ((ArgCnt==0) OR ((ArgCnt==1) AND (strcasecmp(ArgStr[1],"A")==0)))
       BEGIN
        CodeLen=1; BAsmCode[0]=RMWOrders[z].ACode;
       END
      else if ((ArgCnt==1) AND (strcasecmp(ArgStr[1],"B")==0))
       BEGIN
        CodeLen=2; BAsmCode[0]=PrefAccB; BAsmCode[1]=RMWOrders[z].ACode;
        if (MomCPU==CPU65816)
         BEGIN
          WrError(1505); return;
         END
       END
      else if (ArgCnt>2) WrError(1110);
      else
       BEGIN
        DecodeAdr(1,MModAbs8+MModAbs16+MModIdxX8+MModIdxX16);
        if (AdrType!=ModNone)
         BEGIN
          switch (AdrType)
           BEGIN
            case ModAbs8   : BAsmCode[0]=RMWOrders[z].MCode; break;
            case ModAbs16  : BAsmCode[0]=RMWOrders[z].MCode+8; break;
            case ModIdxX8  : BAsmCode[0]=RMWOrders[z].MCode+16; break;
            case ModIdxX16 : BAsmCode[0]=RMWOrders[z].MCode+24; break;
           END
          memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
         END
       END
      return;
     END

   if (Memo("ASR"))
    BEGIN
     if (MomCPU<CPUM7750) WrError(1500);
     else if ((ArgCnt==0) OR ((ArgCnt==1) AND (strcasecmp(ArgStr[1],"A")==0)))
      BEGIN
       BAsmCode[0]=0x89; BAsmCode[1]=0x08; CodeLen=2;
      END
     else if ((ArgCnt==1) AND (strcasecmp(ArgStr[1],"B")==0))
      BEGIN
       BAsmCode[0]=0x42; BAsmCode[1]=0x08; CodeLen=2;
      END
     else if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
     else
      BEGIN
       DecodeAdr(1,MModAbs8+MModIdxX8+MModAbs16+MModIdxX16);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[0]=0x89;
         switch (AdrType)
          BEGIN
           case ModAbs8:BAsmCode[1]=0x06; break;
           case ModIdxX8:BAsmCode[1]=0x16; break;
           case ModAbs16:BAsmCode[1]=0x0e; break;
           case ModIdxX16:BAsmCode[1]=0x1e; break;
          END
         memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
        END
      END
     return;
    END

   if ((Memo("BBC")) OR (Memo("BBS")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (MomCPU<CPUM7700) WrError(1500);
     else
      BEGIN
       WordSize=(Reg_M==0);
       ArgCnt=2; DecodeAdr(2,MModAbs8+MModAbs16);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[0]=0x24;
         if (Memo("BBC")) BAsmCode[0]+=0x10;
         if (AdrType==ModAbs16) BAsmCode[0]+=8;
         memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
         ArgCnt=1; DecodeAdr(1,MModImm);
         if (AdrType==ModNone) CodeLen=0;
         else
          BEGIN
           memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt); CodeLen+=AdrCnt;
           AdrLong=EvalIntExpression(ArgStr[3],UInt24,&OK)-(EProgCounter()+CodeLen+1);
           if (NOT OK) CodeLen=0;
           else if ((NOT SymbolQuestionable) AND ((AdrLong<-128) OR (AdrLong>127)))
            BEGIN
             WrError(1370); CodeLen=0;
            END
           else
            BEGIN
             BAsmCode[CodeLen]=Lo(AdrLong); CodeLen++;
            END
          END
        END
      END
     return;
    END

   if (Memo("BIT"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else if (MomCPU!=CPU65816) WrError(1500);
     else
      BEGIN
       WordSize=False;
       DecodeAdr(1,MModAbs8+MModAbs16+MModIdxX8+MModIdxX16+MModImm);
       if (AdrType!=ModNone)
        BEGIN
         switch (AdrType)
          BEGIN
           case ModAbs8:BAsmCode[0]=0x24; break;
           case ModAbs16:BAsmCode[0]=0x2c; break;
           case ModIdxX8:BAsmCode[0]=0x34; break;
           case ModIdxX16:BAsmCode[0]=0x3c; break;
           case ModImm:BAsmCode[0]=0x89; break;
          END
         memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
        END
      END
     return;
    END

   if ((Memo("CLB")) OR (Memo("SEB")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPUM7700) WrError(1500);
     else
      BEGIN
       WordSize=(Reg_M==0);
       DecodeAdr(2,MModAbs8+MModAbs16);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[0]=0x04;
         if (Memo("CLB")) BAsmCode[0]+=0x10;
         if (AdrType==ModAbs16) BAsmCode[0]+=8;
         memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
         ArgCnt=1; DecodeAdr(1,MModImm);
         if (AdrType==ModNone) CodeLen=0;
         else
          BEGIN
           memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt); CodeLen+=AdrCnt;
          END
        END
      END
     return;
    END

   if ((Memo("TSB")) OR (Memo("TRB")))
    BEGIN
     if (MomCPU==CPU65816)
      BEGIN
       if (ArgCnt!=1) WrError(1110);
       else
        BEGIN
         DecodeAdr(1,MModAbs8+MModAbs16);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[0]=0x04;
           if (Memo("TRB")) BAsmCode[0]+=0x10;
           if (AdrType==ModAbs16) BAsmCode[0]+=8;
           memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
          END
        END
      END
     else if (Memo("TRB")) WrError(1500);
     else if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; BAsmCode[0]=0x42; BAsmCode[1]=0x3b;
      END
     return;
    END

   for (z=0; z<Imm8OrderCnt; z++)
    if (Memo(Imm8Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (((Imm8Orders[z].Allowed >> (MomCPU-CPU65816))&1)==0) WrError(1500);
      else
       BEGIN
        WordSize=False;
        DecodeAdr(1,MModImm);
        if (AdrType==ModImm)
         BEGIN
          CodeLen=1+Ord(Hi(Imm8Orders[z].Code)!=0);
          if (CodeLen==2) BAsmCode[0]=Hi(Imm8Orders[z].Code);
          BAsmCode[CodeLen-1]=Lo(Imm8Orders[z].Code);
          memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt); CodeLen+=AdrCnt;
         END
       END
      return;
     END

   if (Memo("RLA"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       WordSize=(Reg_M==0);
       DecodeAdr(1,MModImm);
       if (AdrType!=ModNone)
        BEGIN
         CodeLen=2+AdrCnt; BAsmCode[0]=0x89; BAsmCode[1]=0x49;
         memcpy(BAsmCode+2,AdrVals,AdrCnt);
        END
      END
     return;
    END

   for (z=0; z<XYOrderCnt; z++)
    if (Memo(XYOrders[z].Name))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else
       BEGIN
        WordSize=(Reg_X==0); Mask=0;
        if (XYOrders[z].CodeImm   !=0xff) Mask+=MModImm;
        if (XYOrders[z].CodeAbs8  !=0xff) Mask+=MModAbs8;
        if (XYOrders[z].CodeAbs16 !=0xff) Mask+=MModAbs16;
        if (XYOrders[z].CodeIdxX8 !=0xff) Mask+=MModIdxX8;
        if (XYOrders[z].CodeIdxX16!=0xff) Mask+=MModIdxX16;
        if (XYOrders[z].CodeIdxY8 !=0xff) Mask+=MModIdxY8;
        if (XYOrders[z].CodeIdxY16!=0xff) Mask+=MModIdxY16;
        DecodeAdr(1,Mask);
        if (AdrType!=ModNone)
         BEGIN
          switch (AdrType)
           BEGIN
            case ModImm   :BAsmCode[0]=XYOrders[z].CodeImm; break;
            case ModAbs8  :BAsmCode[0]=XYOrders[z].CodeAbs8; break;
            case ModAbs16 :BAsmCode[0]=XYOrders[z].CodeAbs16; break;
            case ModIdxX8 :BAsmCode[0]=XYOrders[z].CodeIdxX8; break;
            case ModIdxY8 :BAsmCode[0]=XYOrders[z].CodeIdxY8; break;
            case ModIdxX16:BAsmCode[0]=XYOrders[z].CodeIdxX16; break;
            case ModIdxY16:BAsmCode[0]=XYOrders[z].CodeIdxY16; break;
           END
          memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
         END
       END
      return;
     END

    for (z=0; z<MulDivOrderCnt; z++)
     if (LMemo(MulDivOrders[z].Name))
      BEGIN
       if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
       else if (((MulDivOrders[z].Allowed >> (MomCPU-CPU65816))&1)==0) WrError(1500);
       else
        BEGIN
         WordSize=(Reg_M==0);
         DecodeAdr(1,MModImm+MModAbs8+MModAbs16+MModAbs24+MModIdxX8+MModIdxX16+
                     MModIdxX24+MModIdxY16+MModInd8+MModIndX8+MModIndY8+
                     MModIdxS8+MModIndS8);
         if (AdrType!=ModNone)
          BEGIN
           if ((LFlag) AND (AdrType!=ModInd8) AND (AdrType!=ModIndY8)) WrError(1350);
           else
            BEGIN
             BAsmCode[0]=0x89;
             switch (AdrType)
              BEGIN
               case ModImm    : BAsmCode[1]=0x09; break;
               case ModAbs8   : BAsmCode[1]=0x05; break;
               case ModAbs16  : BAsmCode[1]=0x0d; break;
               case ModAbs24  : BAsmCode[1]=0x0f; break;
               case ModIdxX8  : BAsmCode[1]=0x15; break;
               case ModIdxX16 : BAsmCode[1]=0x1d; break;
               case ModIdxX24 : BAsmCode[1]=0x1f; break;
               case ModIdxY16 : BAsmCode[1]=0x19; break;
               case ModInd8   : BAsmCode[1]=(LFlag) ? 0x07 : 0x12; break;
               case ModIndX8  : BAsmCode[1]=0x01; break;
               case ModIndY8  : BAsmCode[1]=(LFlag) ? 0x17 : 0x11; break;
               case ModIdxS8  : BAsmCode[1]=0x03; break;
               case ModIndS8  : BAsmCode[1]=0x13; break;
              END
             BAsmCode[1]+=MulDivOrders[z].Code;
             memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
            END
          END 
        END
       return;
      END

   if ((Memo("JML")) OR (Memo("JSL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (OK)
        BEGIN
         CodeLen=4;
         BAsmCode[0]=(Memo("JSL")) ? 0x22 : 0x5c;
         BAsmCode[1]=AdrLong >> 16;
         BAsmCode[2]=Hi(AdrLong);
         BAsmCode[3]=Lo(AdrLong);
        END
      END
     return;
    END

   if ((LMemo("JMP")) OR (LMemo("JSR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       BankReg=Reg_PG;
       Mask=MModAbs24+MModIndX16;
       if (NOT LFlag) Mask+=MModAbs16;
       DecodeAdr(1,Mask+((LMemo("JSR"))?0:MModInd16));
       if (AdrType!=ModNone)
        BEGIN
         switch (AdrType)
          BEGIN
           case ModAbs16:
            BAsmCode[0]=(LMemo("JSR")) ? 0x20 : 0x4c; break;
           case ModAbs24:
            BAsmCode[0]=(LMemo("JSR")) ? 0x22 : 0x5c; break;
           case ModIndX16:
            BAsmCode[0]=(LMemo("JSR")) ? 0xfc : 0x7c; break;
           case ModInd16:
            BAsmCode[0]=(LFlag) ? 0xdc : 0x6c; break;
          END
         memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
        END
      END
     return;
    END

   if (Memo("LDM"))
    BEGIN
     if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
     else if (MomCPU<CPUM7700) WrError(1500);
     else
      BEGIN
       DecodeAdr(2,MModAbs8+MModAbs16+MModIdxX8+MModIdxX16);
       if (AdrType!=ModNone)
        BEGIN
         switch (AdrType)
          BEGIN
           case ModAbs8  : BAsmCode[0]=0x64; break;
           case ModAbs16 : BAsmCode[0]=0x9c; break;
           case ModIdxX8 : BAsmCode[0]=0x74; break;
           case ModIdxX16: BAsmCode[0]=0x9e; break;
          END
         memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
         WordSize=(Reg_M==0);
         ArgCnt=1; DecodeAdr(1,MModImm);
         if (AdrType==ModNone) CodeLen=0;
         else
          BEGIN
           memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt); CodeLen+=AdrCnt;
          END
        END
      END
     return;
    END

   if (Memo("STZ"))
    BEGIN
     if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
     else if (MomCPU!=CPU65816) WrError(1500);
     else
      BEGIN
       DecodeAdr(1,MModAbs8+MModAbs16+MModIdxX8+MModIdxX16);
       if (AdrType!=ModNone)
        BEGIN
         switch (AdrType)
          BEGIN
           case ModAbs8  : BAsmCode[0]=0x64; break;
           case ModAbs16 : BAsmCode[0]=0x9c; break;
           case ModIdxX8 : BAsmCode[0]=0x74; break;
           case ModIdxX16: BAsmCode[0]=0x9e; break;
          END
         memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
        END
      END
     return;
    END

   if ((Memo("MVN")) OR (Memo("MVP")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK);
       if (OK)
        BEGIN
         Mask=EvalIntExpression(ArgStr[2],Int32,&OK);
         if (OK)
          BEGIN
           if (((Mask & 0xff000000)!=0) OR ((AdrLong & 0xff000000)!=0)) WrError(1320);
           else
            BEGIN
             BAsmCode[0]=(Memo("MVN")) ? 0x54 : 0x44;
             BAsmCode[1]=AdrLong >> 16;
             BAsmCode[2]=Mask >> 16;
             CodeLen=3;
            END
          END
        END
      END
     return;
    END

   if ((Memo("PSH")) OR (Memo("PUL")))
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else if (MomCPU<CPUM7700) WrError(1500);
     else
      BEGIN
       BAsmCode[0]=0xeb+(Ord(Memo("PUL")) << 4);
       BAsmCode[1]=0; OK=True;
       z=1;
       while ((z<=ArgCnt) AND (OK))
        BEGIN
         if (*ArgStr[z]=='#')
          BAsmCode[1]|=EvalIntExpression(ArgStr[z]+1,Int8,&OK);
         else
          BEGIN
           Start=0;
           while ((Start<PushRegCnt) AND (strcasecmp(PushRegs[Start],ArgStr[z])!=0)) Start++;
           OK=(Start<PushRegCnt);
           if (OK) BAsmCode[1]|=1l << Start;
           else WrXError(1980,ArgStr[z]);
          END
         z++;
        END
       if (OK) CodeLen=2;
      END
     return;
    END

   if (Memo("PEA"))
    BEGIN
     WordSize=True;
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(1,MModImm);
       if (AdrType!=ModNone)
        BEGIN
         CodeLen=1+AdrCnt; BAsmCode[0]=0xf4;
         memcpy(BAsmCode+1,AdrVals,AdrCnt);
        END
      END
     return;
    END

   if (Memo("PEI"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
       DecodeAdr(1,MModAbs8);
       if (AdrType!=ModNone)
        BEGIN
         CodeLen=1+AdrCnt; BAsmCode[0]=0xd4;
         memcpy(BAsmCode+1,AdrVals,AdrCnt);
        END
      END
     return;
    END

   if (Memo("PER"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       Rel=True;
       if (*ArgStr[1]=='#')
        BEGIN
         strcpy(ArgStr[1],ArgStr[1]+1); Rel=False;
        END
       BAsmCode[0]=0x62;
       if (Rel)
        BEGIN
         AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK)-(EProgCounter()+2);
         if (OK)
          BEGIN
           if ((AdrLong<-32768) OR (AdrLong>32767)) WrError(1370);
           else
            BEGIN
             CodeLen=3; BAsmCode[1]=AdrLong & 0xff;
             BAsmCode[2]=(AdrLong >> 8) & 0xff;
            END
          END
        END
       else
        BEGIN
         z=EvalIntExpression(ArgStr[1],Int16,&OK);
         if (OK)
          BEGIN
           CodeLen=3; BAsmCode[1]=Lo(z); BAsmCode[2]=Hi(z);
          END
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static void InitCode_7700(void)
BEGIN
   SaveInitProc();
   Reg_PG=0;
   Reg_DT=0;
   Reg_X=0;
   Reg_M=0;
   Reg_DPR=0;
END

        static Boolean IsDef_7700(void)
BEGIN
   return False;
END

        static void SwitchFrom_7700(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_7700(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x19; NOPCode=0xea;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1 << SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffffffl;

   MakeCode=MakeCode_7700; IsDef=IsDef_7700;
   SwitchFrom=SwitchFrom_7700; InitFields();
END


        void code7700_init(void)
BEGIN
   CPU65816=AddCPU("65816"    ,SwitchTo_7700);
   CPUM7700=AddCPU("MELPS7700",SwitchTo_7700);
   CPUM7750=AddCPU("MELPS7750",SwitchTo_7700);
   CPUM7751=AddCPU("MELPS7751",SwitchTo_7700);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_7700;
END
