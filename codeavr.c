/* codeavr.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Atmel AVR                                                   */             
/*                                                                           */
/* Historie: 26.12.1996 Grundsteinlegung                                     */             
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "nls.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Word Code;
         } ArchOrder;


#define FixedOrderCnt 24
#define Reg1OrderCnt 10
#define Reg2OrderCnt 12
#define Reg3OrderCnt 4
#define ImmOrderCnt 7
#define RelOrderCnt 18
#define BitOrderCnt 4
#define PBitOrderCnt 4


static CPUVar CPU90S1200,CPU90S2313,CPU90S4414,CPU90S8515;

static ArchOrder *FixedOrders;
static ArchOrder *Reg1Orders;
static ArchOrder *Reg2Orders;
static FixedOrder *Reg3Orders;
static FixedOrder *ImmOrders;
static FixedOrder *RelOrders;
static FixedOrder *BitOrders;
static FixedOrder *PBitOrders;

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddReg1(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=Reg1OrderCnt) exit(255);
   Reg1Orders[InstrZ].Name=NName;
   Reg1Orders[InstrZ].MinCPU=NMin;
   Reg1Orders[InstrZ++].Code=NCode;
END
   
        static void AddReg2(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=Reg2OrderCnt) exit(255);
   Reg2Orders[InstrZ].Name=NName;
   Reg2Orders[InstrZ].MinCPU=NMin;
   Reg2Orders[InstrZ++].Code=NCode;
END

        static void AddReg3(char *NName, Word NCode)
BEGIN
   if (InstrZ>=Reg3OrderCnt) exit(255);
   Reg3Orders[InstrZ].Name=NName;
   Reg3Orders[InstrZ++].Code=NCode;
END

        static void AddImm(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name=NName;
   ImmOrders[InstrZ++].Code=NCode;
END

        static void AddRel(char *NName, Word NCode)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
END

        static void AddBit(char *NName, Word NCode)
BEGIN
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ].Name=NName;
   BitOrders[InstrZ++].Code=NCode;
END

        static void AddPBit(char *NName, Word NCode)
BEGIN
   if (InstrZ>=PBitOrderCnt) exit(255);
   PBitOrders[InstrZ].Name=NName;
   PBitOrders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(ArchOrder *) malloc(sizeof(ArchOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("IJMP" ,CPU90S2313,0x9409); AddFixed("ICALL",CPU90S2313,0x9509);
   AddFixed("RET"  ,CPU90S1200,0x9508); AddFixed("RETI" ,CPU90S1200,0x9518);
   AddFixed("LPM"  ,CPU90S2313,0x95c8); AddFixed("SEC"  ,CPU90S1200,0x9408);
   AddFixed("CLC"  ,CPU90S1200,0x9488); AddFixed("SEN"  ,CPU90S1200,0x9428);
   AddFixed("CLN"  ,CPU90S1200,0x94a8); AddFixed("SEZ"  ,CPU90S1200,0x9418);
   AddFixed("CLZ"  ,CPU90S1200,0x9498); AddFixed("SEI"  ,CPU90S1200,0x9478);
   AddFixed("CLI"  ,CPU90S1200,0x94f8); AddFixed("SES"  ,CPU90S1200,0x9448);
   AddFixed("CLS"  ,CPU90S1200,0x94c8); AddFixed("SEV"  ,CPU90S1200,0x9438);
   AddFixed("CLV"  ,CPU90S1200,0x94b8); AddFixed("SET"  ,CPU90S1200,0x9468);
   AddFixed("CLT"  ,CPU90S1200,0x94e8); AddFixed("SEH"  ,CPU90S1200,0x9458);
   AddFixed("CLH"  ,CPU90S1200,0x94d8); AddFixed("NOP"  ,CPU90S1200,0x0000);
   AddFixed("SLEEP",CPU90S1200,0x9588); AddFixed("WDR"  ,CPU90S1200,0x95a8);

   Reg1Orders=(ArchOrder *) malloc(sizeof(ArchOrder)*Reg1OrderCnt); InstrZ=0;
   AddReg1("COM"  ,CPU90S1200,0x9400); AddReg1("NEG"  ,CPU90S1200,0x9401);
   AddReg1("INC"  ,CPU90S1200,0x9403); AddReg1("DEC"  ,CPU90S1200,0x940a);
   AddReg1("PUSH" ,CPU90S2313,0x920f); AddReg1("POP"  ,CPU90S2313,0x900f);
   AddReg1("LSR"  ,CPU90S1200,0x9406); AddReg1("ROR"  ,CPU90S1200,0x9407);
   AddReg1("ASR"  ,CPU90S1200,0x9405); AddReg1("SWAP" ,CPU90S1200,0x9402);

   Reg2Orders=(ArchOrder *) malloc(sizeof(ArchOrder)*Reg2OrderCnt); InstrZ=0;
   AddReg2("ADD"  ,CPU90S1200,0x0c00); AddReg2("ADC"  ,CPU90S1200,0x1c00);
   AddReg2("SUB"  ,CPU90S1200,0x1800); AddReg2("SBC"  ,CPU90S1200,0x0800);
   AddReg2("AND"  ,CPU90S1200,0x2000); AddReg2("OR"   ,CPU90S1200,0x2800);
   AddReg2("EOR"  ,CPU90S1200,0x2400); AddReg2("CPSE" ,CPU90S1200,0x1000);
   AddReg2("CP"   ,CPU90S1200,0x1400); AddReg2("CPC"  ,CPU90S1200,0x0400);
   AddReg2("MOV"  ,CPU90S1200,0x2c00); AddReg2("MUL"  ,CPU90S8515+1,0x9c00);

   Reg3Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Reg3OrderCnt); InstrZ=0;
   AddReg3("CLR"  ,0x2400); AddReg3("TST"  ,0x2000); AddReg3("LSL"  ,0x0c00);
   AddReg3("ROL"  ,0x1c00);

   ImmOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*ImmOrderCnt); InstrZ=0;
   AddImm("SUBI" ,0x5000); AddImm("SBCI" ,0x4000); AddImm("ANDI" ,0x7000);
   AddImm("ORI"  ,0x6000); AddImm("SBR"  ,0x6000); AddImm("CPI"  ,0x3000);
   AddImm("LDI"  ,0xe000);

   RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCnt); InstrZ=0;
   AddRel("BRCC" ,0xf400); AddRel("BRCS" ,0xf000); AddRel("BREQ" ,0xf001);
   AddRel("BRGE" ,0xf404); AddRel("BRSH" ,0xf400); AddRel("BRID" ,0xf407);
   AddRel("BRIE" ,0xf007); AddRel("BRLO" ,0xf000); AddRel("BRLT" ,0xf004);
   AddRel("BRMI" ,0xf002); AddRel("BRNE" ,0xf401); AddRel("BRHC" ,0xf405);
   AddRel("BRHS" ,0xf005); AddRel("BRPL" ,0xf402); AddRel("BRTC" ,0xf406);
   AddRel("BRTS" ,0xf006); AddRel("BRVC" ,0xf403); AddRel("BRVS" ,0xf003);

   BitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BitOrderCnt); InstrZ=0;
   AddBit("BLD"  ,0xf800); AddBit("BST"  ,0xfa00);
   AddBit("SBRC" ,0xfc00); AddBit("SBRS" ,0xfe00);

   PBitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*PBitOrderCnt); InstrZ=0;
   AddPBit("CBI" ,0x9800); AddPBit("SBI" ,0x9a00);
   AddPBit("SBIC",0x9900); AddPBit("SBIS",0x9b00);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(Reg1Orders);
   free(Reg2Orders);
   free(Reg3Orders);
   free(ImmOrders);
   free(RelOrders);
   free(BitOrders);
   free(PBitOrders);
END

/*---------------------------------------------------------------------------*/

        static Boolean DecodeReg(char *Asc, Word *Erg)
BEGIN
   Boolean io;
   char *s;

   if (FindRegDef(Asc,&s)) Asc=s;

   if ((strlen(Asc)<2) OR (strlen(Asc)>3) OR (toupper(*Asc)!='R')) return False;
   else
    BEGIN
     *Erg=ConstLongInt(Asc+1,&io);
     return ((io) AND (*Erg<32));
    END
END

	static Boolean DecodeMem(char * Asc, Word *Erg)
BEGIN
   if (strcasecmp(Asc,"X")==0) *Erg=0x1c;
   else if (strcasecmp(Asc,"X+")==0) *Erg=0x1d;
   else if (strcasecmp(Asc,"-X")==0) *Erg=0x1e;
   else if (strcasecmp(Asc,"Y")==0) *Erg=0x08;
   else if (strcasecmp(Asc,"Y+")==0) *Erg=0x19;
   else if (strcasecmp(Asc,"-Y")==0) *Erg=0x1a;
   else if (strcasecmp(Asc,"Z")==0) *Erg=0x00;
   else if (strcasecmp(Asc,"Z+")==0) *Erg=0x11;
   else if (strcasecmp(Asc,"-Z")==0) *Erg=0x12;
   else return False;
   return True;
END

/*---------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   Integer Size;
   int z,z2;
   Boolean ValOK;
   TempResult t;
   LongInt MinV,MaxV;

   if (Memo("PORT"))
    BEGIN
     CodeEquate(SegIO,0,0x3f);
     return True;
    END

   if (Memo("RES"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],Int16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) AND (NOT FirstPassUnknown))
	BEGIN
	 DontPrint=True;
	 CodeLen=Size;
	 if (MakeUseList)
	  if (AddChunk(SegChunks+ActPC,ProgCounter(),CodeLen,ActPC==SegCode)) WrError(90);
	END
      END
     return True;
    END

   if (Memo("DATA"))
    BEGIN
     MaxV=(ActPC==SegCode)?65535:255; MinV=(-((MaxV+1) >> 1));
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       ValOK=True;
       for (z=1; z<=ArgCnt; z++)
	if (ValOK)
	 BEGIN
	  EvalExpression(ArgStr[z],&t);
          if ((FirstPassUnknown) AND (t.Typ==TempInt)) t.Contents.Int&=MaxV;
	  switch (t.Typ)
           BEGIN
	    case TempInt:
             if (ChkRange(t.Contents.Int,MinV,MaxV))
              if (ActPC==SegCode) WAsmCode[CodeLen++]=t.Contents.Int;
              else BAsmCode[CodeLen++]=t.Contents.Int;
             break;
	    case TempFloat:
             WrError(1135); ValOK=False;
	     break;
	    case TempString:
             for (z2=0; z2<strlen(t.Contents.Ascii); z2++)
	      BEGIN
               Size=CharTransTable[(Byte) t.Contents.Ascii[z2]];
               if (ActPC!=SegCode) BAsmCode[CodeLen++]=Size;
               else if ((z2&1)==0) WAsmCode[CodeLen++]=Size;
               else WAsmCode[CodeLen-1]+=Size<<8;
	      END
             break;
	    default:
             ValOK=False;
	   END
	 END
       if (NOT ValOK) CodeLen=0;
      END
     return True;
    END

   if (Memo("REG"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else AddRegDef(LabPart,ArgStr[1]);
     return True;
    END

   return False;
END

        static void MakeCode_AVR(void)
BEGIN
   int z;
   LongInt AdrInt;
   Word Reg1,Reg2;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* kein Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<FixedOrders[z].MinCPU) WrXError(1500,OpPart);
      else
       BEGIN
        WAsmCode[0]=FixedOrders[z].Code; CodeLen=1;
       END
      return;
     END

   /* nur Register */

   for (z=0; z<Reg1OrderCnt; z++)
    if (Memo(Reg1Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<Reg1Orders[z].MinCPU) WrXError(1500,OpPart);
      else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
      else
       BEGIN
        WAsmCode[0]=Reg1Orders[z].Code+(Reg1 << 4); CodeLen=1;
       END
      return;
     END

   for (z=0; z<Reg2OrderCnt; z++)
    if (Memo(Reg2Orders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (MomCPU<Reg2Orders[z].MinCPU) WrXError(1500,OpPart);
      else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
      else if (NOT DecodeReg(ArgStr[2],&Reg2)) WrXError(1445,ArgStr[2]);
      else
       BEGIN
        WAsmCode[0]=Reg2Orders[z].Code+(Reg2 & 15)+(Reg1 << 4)+
                     ((Reg2 & 16) << 5);
        CodeLen=1;
       END
      return;
     END

   for (z=0; z<Reg3OrderCnt; z++)
    if (Memo(Reg3Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
      else
       BEGIN
        WAsmCode[0]=Reg3Orders[z].Code+(Reg1 & 15)+(Reg1 << 4)+
                    ((Reg1 & 16) << 5);
        CodeLen=1;
       END
      return;
     END

   /* immediate mit Register */

   for (z=0; z<ImmOrderCnt; z++)
    if (Memo(ImmOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
      else if (Reg1<16) WrXError(1445,ArgStr[1]);
      else
       BEGIN
        Reg2=EvalIntExpression(ArgStr[2],Int8,&OK);
        if (OK)
         BEGIN
          WAsmCode[0]=ImmOrders[z].Code+((Reg2 & 0xf0) << 4)+(Reg2 & 0x0f)+
                      ((Reg1 & 0x0f) << 4);
          CodeLen=1;
         END
       END
      return;
     END

   if ((Memo("ADIW")) OR (Memo("SBIW")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU90S2313) WrError(1500);
     else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
     else if ((Reg1<24) OR (Odd(Reg1))) WrXError(1445,ArgStr[1]);
     else
      BEGIN
       Reg2=EvalIntExpression(ArgStr[2],UInt6,&OK);
       if (OK)
	BEGIN
	 WAsmCode[0]=0x9600+(Ord(Memo("SBIW")) << 8)+((Reg1 & 6) << 3)+
		     (Reg2 & 15)+((Reg2 & 0x30) << 2);
	 CodeLen=1;
	END
      END
     return;
    END

   /* Transfer */

   if ((Memo("LD")) OR (Memo("ST")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (Memo("ST"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         z=0x200;
        END
       else z=0;
       if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
       else if (NOT DecodeMem(ArgStr[2],&Reg2)) WrError(1350);
       else if ((MomCPU<CPU90S2313) AND (Reg2!=0)) WrError(1351);
       else
        BEGIN
         WAsmCode[0]=0x8000+z+(Reg1 << 4)+(Reg2 & 0x0f)+((Reg2 & 0x10) << 8);
         CodeLen=1;
        END
      END
     return;
    END

   if ((Memo("LDD")) OR (Memo("STD")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU90S2313) WrXError(1500,OpPart);
     else
      BEGIN
       if (Memo("STD"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         z=0x200;
        END
       else z=0;
       OK=True;
       if (toupper(*ArgStr[2])=='Y') z+=8;
       else if (toupper(*ArgStr[2])=='Z');
       else OK=False;
       if (NOT OK) WrError(1350);
       else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
       else
        BEGIN
         *ArgStr[2]='0';
         Reg2=EvalIntExpression(ArgStr[2]+1,UInt6,&OK);
         if (OK)
          BEGIN
           WAsmCode[0]=0x8000+z+(Reg1 << 4)+(Reg2 & 7)+((Reg2 & 0x18) << 7)+((Reg2 & 0x20) << 8);
	   CodeLen=1;
          END
        END
      END
     return;
    END

   if ((Memo("IN")) OR (Memo("OUT")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if ((Memo("OUT")))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         z=0x800;
        END
       else z=0;
       if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
       else
        BEGIN
         Reg2=EvalIntExpression(ArgStr[2],UInt6,&OK);
         if (OK)
          BEGIN
           ChkSpace(SegIO);
           WAsmCode[0]=0xb000+z+(Reg1 << 4)+(Reg2 & 0x0f)+((Reg2 & 0xf0) << 5);
           CodeLen=1;
          END
        END
      END
     return;
    END

   if ((Memo("LDS")) OR (Memo("STS")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU90S2313) WrError(1500);
     else
      BEGIN
       if (Memo("STS"))
	BEGIN
	 strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
	 z=0x200;
	END
       else z=0;
       if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
       else
	BEGIN
	 WAsmCode[1]=EvalIntExpression(ArgStr[2],UInt16,&OK);
	 if (OK)
	  BEGIN
           ChkSpace(SegData);
	   WAsmCode[0]=0x9000+z+(Reg1 << 4); 
	   CodeLen=2;
	  END
	END
      END
     return;
    END

   /* Bitoperationen */

   if ((Memo("BCLR")) OR (Memo("BSET")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       Reg1=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
        BEGIN
         WAsmCode[0]=0x9408+(Reg1 << 4)+(Ord(Memo("BCLR")) << 7);
         CodeLen=1;
        END
      END
     return;
    END

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
      else
       BEGIN
        Reg2=EvalIntExpression(ArgStr[2],UInt3,&OK);
        if (OK)
         BEGIN
          WAsmCode[0]=BitOrders[z].Code+(Reg1 << 4)+Reg2;
          CodeLen=1;
         END
       END
      return;
     END

   if (Memo("CBR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
     else if (Reg1<16) WrXError(1445,ArgStr[1]);
     else
      BEGIN
       Reg2=EvalIntExpression(ArgStr[2],Int8,&OK) ^ 0xff;
       if (OK)
        BEGIN
         WAsmCode[0]=0x7000+((Reg2 & 0xf0) << 4)+(Reg2 & 0x0f)+
                      ((Reg1 & 0x0f) << 4);
         CodeLen=1;
        END
      END
     return;
    END

   if (Memo("SER"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Reg1)) WrXError(1445,ArgStr[1]);
     else if (Reg1<16) WrXError(1445,ArgStr[1]);
     else
      BEGIN
       WAsmCode[0]=0xef0f+((Reg1 & 0x0f) << 4);
       CodeLen=1;
      END
     return;
    END

   for (z=0; z<PBitOrderCnt; z++)
    if (Memo(PBitOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	Reg1=EvalIntExpression(ArgStr[1],UInt5,&OK);
	if (OK)
	 BEGIN
	  ChkSpace(SegIO);
	  Reg2=EvalIntExpression(ArgStr[2],UInt3,&OK);
	  if (OK)
	   BEGIN
	    WAsmCode[0]=PBitOrders[z].Code+Reg2+(Reg1 << 3);
	    CodeLen=1;
           END
	 END
       END
      return;
     END

   /* Spruenge */

   for (z=0; z<RelOrderCnt; z++)
    if (Memo(RelOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+1);
        if (OK)
         if ((NOT SymbolQuestionable) AND ((AdrInt<-64) OR (AdrInt>63))) WrError(1370);
         else
          BEGIN
           ChkSpace(SegCode);
           WAsmCode[0]=RelOrders[z].Code+((AdrInt & 0x7f) << 3);
           CodeLen=1;
          END
       END
      return;
     END

   if ((Memo("BRBC")) OR (Memo("BRBS")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       Reg1=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
        BEGIN
         AdrInt=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+1);
         if (OK)
          if ((NOT SymbolQuestionable) AND ((AdrInt<-64) OR (AdrInt>63))) WrError(1370);
          else
           BEGIN
            ChkSpace(SegCode);
            WAsmCode[0]=0xf000+(Ord(Memo("BRBC")) << 10)+((AdrInt & 0x7f) << 3)+Reg1;
            CodeLen=1;
           END
        END
      END
     return;
    END

   if ((Memo("JMP")) OR (Memo("CALL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU90S2313) WrError(1500);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],UInt22,&OK);
       if (OK)
        BEGIN
         ChkSpace(SegCode);
         WAsmCode[0]=0x940c+(Ord(Memo("CALL")) << 1)+((AdrInt & 0x3e0000) >> 13)+((AdrInt & 0x10000) >> 16);
         WAsmCode[1]=AdrInt & 0xffff;
         CodeLen=2;
        END
      END
     return;
    END

   if ((Memo("RJMP")) OR (Memo("RCALL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],UInt22,&OK)-(EProgCounter()+1);
       if (OK)
        if ((NOT SymbolQuestionable) AND ((AdrInt<-2048) OR (AdrInt>2047))) WrError(1370);
        else
         BEGIN
          ChkSpace(SegCode);
          WAsmCode[0]=0xc000+(Ord(Memo("RCALL")) << 12)+(AdrInt & 0xfff);
          CodeLen=1;
         END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean ChkPC_AVR(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode:
      if (MomCPU==CPU90S1200) return (ProgCounter()<=0x03ff);
      else if (MomCPU==CPU90S2313) return (ProgCounter()<=0x07ff);
      else if (MomCPU==CPU90S4414) return (ProgCounter()<=0x0fff);
      else return (ProgCounter()<=0x1fff);
     case SegData:
      if (MomCPU==CPU90S1200) return (ProgCounter()<=0x5f);
      else if (MomCPU==CPU90S2313) return (ProgCounter()<=0xdf);
      else return (ProgCounter()<0xffff);
     case SegIO:
      return (ProgCounter()<0x3f);
     default:
      return False;
    END
END

        static Boolean IsDef_AVR(void)
BEGIN
   return (Memo("PORT") OR Memo("REG"));
END

        static void SwitchFrom_AVR(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_AVR(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=True;

   PCSymbol="*"; HeaderID=0x3b; NOPCode=0x0000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData)|(1<<SegIO);
   Grans[SegCode]=2; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=32;
   Grans[SegIO  ]=1; ListGrans[SegIO  ]=1; SegInits[SegIO  ]=0;

   MakeCode=MakeCode_AVR; ChkPC=ChkPC_AVR; IsDef=IsDef_AVR;
   SwitchFrom=SwitchFrom_AVR; InitFields();
END

	void codeavr_init(void)
BEGIN
   CPU90S1200=AddCPU("AT90S1200",SwitchTo_AVR);
   CPU90S2313=AddCPU("AT90S2313",SwitchTo_AVR);
   CPU90S4414=AddCPU("AT90S4414",SwitchTo_AVR);
   CPU90S8515=AddCPU("AT90S8515",SwitchTo_AVR);
END

