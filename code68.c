/* code68.c */ 
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator fuer 68xx Prozessoren                                       */
/*                                                                           */
/* Historie:  13. 8.1996 Grundsteinlegung                                    */
/*             2. 1.1998 ChkPC ersetzt                                       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "codepseudo.h"
#include "codevars.h"
#include "asmitree.h"

/*---------------------------------------------------------------------------*/

typedef struct
         {
          char *Name;
	  CPUVar MinCPU,MaxCPU;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } BaseOrder;

typedef struct
         {
          char *Name;
          Boolean MayImm;
	  Byte Code;
	 } ALU8Order;

typedef struct 
         {
          Boolean MayImm;
	  CPUVar MinCPU;    /* Shift  andere   ,Y   */
          Byte PageShift;   /* 0 :     nix    Pg 2  */
	  Byte Code;        /* 1 :     Pg 3   Pg 4  */
	 } ALU16Order;      /* 2 :     nix    Pg 4  */
			    /* 3 :     Pg 2   Pg 3  */


#define ModNone (-1)
#define ModAcc  0
#define MModAcc (1<<ModAcc)
#define ModDir  1
#define MModDir (1<<ModDir)
#define ModExt  2
#define MModExt (1<<ModExt)
#define ModInd  3
#define MModInd (1<<ModInd)
#define ModImm  4
#define MModImm (1<<ModImm)
 
#define Page2Prefix 0x18
#define Page3Prefix 0x1a
#define Page4Prefix 0xcd

#define FixedOrderCnt 45
#define RelOrderCnt 19
#define ALU8OrderCnt 11
#define ALU16OrderCnt 13
#define Sing8OrderCnt 12
#define Bit63OrderCnt 4


static ShortInt OpSize;
static Byte PrefCnt;	       /* Anzahl Befehlspraefixe */
static ShortInt AdrMode;       /* Ergebnisadressmodus */
static Byte AdrPart;           /* Adressierungsmodusbits im Opcode */
static Byte AdrVals[4];        /* Adressargument */

static FixedOrder *FixedOrders;
static BaseOrder *RelOrders;
static ALU8Order *ALU8Orders;
static ALU16Order *ALU16Orders;
static BaseOrder *Bit63Orders;
static BaseOrder *Sing8Orders;
static PInstTable InstTable;

static CPUVar CPU6800,CPU6301,CPU6811;

/*---------------------------------------------------------------------------*/

	static void DecodeAdr(int StartInd, int StopInd, Byte Erl)
BEGIN
   String Asc;
   Boolean OK,ErrOcc;
   Word AdrWord;
   Byte Bit8;

   AdrMode=ModNone; AdrPart=0; strmaxcpy(Asc,ArgStr[StartInd],255); ErrOcc=False;

   /* eine Komponente ? */

   if (StartInd==StopInd)
    BEGIN

     /* Akkumulatoren ? */

     if (strcasecmp(Asc,"A")==0)
      BEGIN
       if ((MModAcc & Erl)!=0) AdrMode=ModAcc;
      END
     else if (strcasecmp(Asc,"B")==0)
      BEGIN
       if ((MModAcc & Erl)!=0)
        BEGIN
         AdrMode=ModAcc; AdrPart=1;
        END
      END

     /* immediate ? */

     else if ((strlen(Asc)>1) AND (*Asc=='#'))
      BEGIN
       if ((MModImm & Erl)!=0)
        BEGIN
         if (OpSize==1)
          BEGIN
           AdrWord=EvalIntExpression(Asc+1,Int16,&OK);
           if (OK)
            BEGIN
             AdrMode=ModImm;
  	   AdrVals[AdrCnt++]=Hi(AdrWord); AdrVals[AdrCnt++]=Lo(AdrWord);
            END
           else ErrOcc=True;
          END
         else
          BEGIN
           AdrVals[AdrCnt]=EvalIntExpression(Asc+1,Int8,&OK);
           if (OK)
            BEGIN
             AdrMode=ModImm; AdrCnt++;
            END
           else ErrOcc=True;
          END
        END
      END

     /* absolut ? */

     else
      BEGIN
       Bit8=0;
       if (*Asc=='<')
        BEGIN
         Bit8=2; strcpy(Asc,Asc+1);
        END
       else if (*Asc=='>')
        BEGIN
         Bit8=1; strcpy(Asc,Asc+1);
        END
       if ((Bit8==2) OR ((MModExt & Erl)==0))
        AdrWord=EvalIntExpression(Asc,Int8,&OK);
       else
        AdrWord=EvalIntExpression(Asc,Int16,&OK);
       if (OK)
        BEGIN
         if (((MModDir & Erl)!=0) AND (Bit8!=1) AND ((Bit8==2) OR ((MModExt & Erl)==0) OR (Hi(AdrWord)==0)))
  	  BEGIN
  	   if (Hi(AdrWord)!=0)
  	    BEGIN
  	     WrError(1340); ErrOcc=True;
  	    END
  	   else
  	    BEGIN
  	     AdrMode=ModDir; AdrPart=1;
  	     AdrVals[AdrCnt++]=Lo(AdrWord);
            END
          END
         else if ((MModExt & Erl)!=0)
          BEGIN
           AdrMode=ModExt; AdrPart=3;
           AdrVals[AdrCnt++]=Hi(AdrWord); AdrVals[AdrCnt++]=Lo(AdrWord);
          END
        END
       else ErrOcc=True;
      END
    END

   /* zwei Komponenten ? */

   else if (StartInd+1==StopInd)
    BEGIN

     /* indiziert ? */

     if (((strcasecmp(ArgStr[StopInd],"X")==0) OR (strcasecmp(ArgStr[StopInd],"Y")==0)))
      BEGIN
       if ((MModInd & Erl)!=0)
        BEGIN
         AdrWord=EvalIntExpression(Asc,Int8,&OK);
         if (OK)
          if ((MomCPU<CPU6811) AND (strcasecmp(ArgStr[StartInd+1],"Y")==0))
  	 BEGIN
  	  WrError(1505); ErrOcc=True;
           END
          else
           BEGIN
            AdrVals[AdrCnt++]=Lo(AdrWord);
            AdrMode=ModInd; AdrPart=2;
            if (strcasecmp(ArgStr[StartInd+1],"Y")==0)
             BEGIN
              BAsmCode[PrefCnt++]=0x18;
             END
           END
          else ErrOcc=True;
        END
      END
     else
      BEGIN
       WrXError(1445,ArgStr[StopInd]); ErrOcc=True;
      END

    END
   else
    BEGIN
     WrError(1110); ErrOcc=True;
    END

   if ((NOT ErrOcc) AND (AdrMode==ModNone)) WrError(1350);
END

	static void AddPrefix(Byte Prefix)
BEGIN
   BAsmCode[PrefCnt++]=Prefix;
END

	static void Try2Split(int Src)
BEGIN
   Integer z;
   char *p;

   KillPrefBlanks(ArgStr[Src]); KillPostBlanks(ArgStr[Src]);
   p=ArgStr[Src]+strlen(ArgStr[Src])-1;
   while ((p>ArgStr[Src]) AND (NOT isspace((unsigned int) *p))) p--;
   if (p>ArgStr[Src])
    BEGIN
     for (z=ArgCnt; z>=Src; z--) strcpy(ArgStr[z+1],ArgStr[z]); ArgCnt++;
     strcpy(ArgStr[Src+1],p+1); *p='\0';
     KillPostBlanks(ArgStr[Src]); KillPrefBlanks(ArgStr[Src+1]);
    END
END

/*---------------------------------------------------------------------------*/

	static void DecodeFixed(Word Index)
BEGIN
   FixedOrder *forder=FixedOrders+Index;

   if (ArgCnt!=0) WrError(1110);
   else if ((MomCPU<forder->MinCPU) OR (MomCPU>forder->MaxCPU)) WrError(1500);
   else if (Hi(forder->Code)!=0)
    BEGIN
     CodeLen=2;
     BAsmCode[0]=Hi(forder->Code);
     BAsmCode[1]=Lo(forder->Code);
    END
   else
    BEGIN
     CodeLen=1; 
     BAsmCode[0]=Lo(forder->Code);
    END
END

	static void DecodeRel(Word Index)
BEGIN
   BaseOrder *forder=RelOrders+Index;
   Integer AdrInt;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
     if (OK)
      BEGIN
       AdrInt-=EProgCounter()+2;
       if (((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable)) WrError(1370);
       else
        BEGIN
         CodeLen=2; BAsmCode[0]=forder->Code; BAsmCode[1]=Lo(AdrInt);
        END
      END
    END
END

	static void DecodeALU16(Word Index)
BEGIN
    ALU16Order *forder=ALU16Orders+Index;

    OpSize=1;
    if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
    else if (MomCPU<forder->MinCPU) WrError(1500);
    else
     BEGIN
      DecodeAdr(1,ArgCnt,(forder->MayImm?MModImm:0)+MModInd+MModExt+MModDir);
      if (AdrMode!=ModNone)
       BEGIN
        switch (forder->PageShift)
         BEGIN
          case 1: 
           if (PrefCnt==1) BAsmCode[PrefCnt-1]=Page4Prefix;
           else AddPrefix(Page3Prefix);
           break;
          case 2:
           if (PrefCnt==1) BAsmCode[PrefCnt-1]=Page4Prefix;
           break;
          case 3:
           if (PrefCnt==0) AddPrefix((AdrMode==ModInd)?Page3Prefix:Page2Prefix);
           break;
         END
        BAsmCode[PrefCnt]=forder->Code+(AdrPart << 4);
        CodeLen=PrefCnt+1+AdrCnt;
        memcpy(BAsmCode+1+PrefCnt,AdrVals,AdrCnt);
       END
     END
END

	static void DecodeBit63(Word Index)
BEGIN
   BaseOrder *forder=Bit63Orders+Index;

   if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
   else if (MomCPU!=CPU6301) WrError(1500);
   else
    BEGIN
     DecodeAdr(1,1,MModImm);
     if (AdrMode!=ModNone)
      BEGIN
       DecodeAdr(2,ArgCnt,MModDir+MModInd);
       if (AdrMode!=ModNone)
        BEGIN
         BAsmCode[PrefCnt]=forder->Code;
         if (AdrMode==ModDir) BAsmCode[PrefCnt]+=0x10;
         CodeLen=PrefCnt+1+AdrCnt;
         memcpy(BAsmCode+1+PrefCnt,AdrVals,AdrCnt);
         END
       END
     END
END

	static void DecodeJMP(Word Index)
BEGIN
   if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
   else
    BEGIN
     DecodeAdr(1,ArgCnt,MModExt+MModInd);
     if (AdrMode!=ModImm)
      BEGIN
       CodeLen=PrefCnt+1+AdrCnt;
       BAsmCode[PrefCnt]=0x4e + (AdrPart << 4);
       memcpy(BAsmCode+1+PrefCnt,AdrVals,AdrCnt);
      END
    END
END

	static void DecodeJSR(Word Index)
BEGIN
   if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
   else
    BEGIN
     DecodeAdr(1,ArgCnt,MModDir+MModExt+MModInd);
     if (AdrMode!=ModImm)
      BEGIN
       CodeLen=PrefCnt+1+AdrCnt;
       BAsmCode[PrefCnt]=0x8d+(AdrPart << 4);
       memcpy(BAsmCode+1+PrefCnt,AdrVals,AdrCnt);
      END
    END
END

	static void DecodeBRxx(Word Index)
BEGIN
   Boolean OK;
   Byte Mask;
   Integer AdrInt;

   if (ArgCnt==1)
    BEGIN
     Try2Split(1); Try2Split(1);
    END
   else if (ArgCnt==2)
    BEGIN
     Try2Split(ArgCnt); Try2Split(2);
    END
   if ((ArgCnt<3) OR (ArgCnt>4)) WrError(1110);
   else if (MomCPU<CPU6811) WrError(1500);
   else
    BEGIN
     if (*ArgStr[ArgCnt-1]=='#') strcpy(ArgStr[ArgCnt-1],ArgStr[ArgCnt-1]+1);
     Mask=EvalIntExpression(ArgStr[ArgCnt-1],Int8,&OK);
     if (OK)
      BEGIN
       DecodeAdr(1,ArgCnt-2,MModDir+MModInd);
       if (AdrMode!=ModNone)
        BEGIN
         AdrInt=EvalIntExpression(ArgStr[ArgCnt],Int16,&OK);
         if (OK)
          BEGIN
           AdrInt-=EProgCounter()+3+PrefCnt+AdrCnt;
           if ((AdrInt<-128) OR (AdrInt>127)) WrError(1370);
           else
            BEGIN
             CodeLen=PrefCnt+3+AdrCnt;
             BAsmCode[PrefCnt]=0x12+Index;
             if (AdrMode==ModInd) BAsmCode[PrefCnt]+=12;
             memcpy(BAsmCode+PrefCnt+1,AdrVals,AdrCnt);
             BAsmCode[PrefCnt+1+AdrCnt]=Mask;
             BAsmCode[PrefCnt+2+AdrCnt]=Lo(AdrInt);
            END
          END
        END
      END
    END
END

        static void DecodeBxx(Word Index)
BEGIN
    Byte Mask;
    Boolean OK;
    int z;

    if (MomCPU==CPU6301)
     BEGIN
      strcpy(ArgStr[ArgCnt+1],ArgStr[1]);
      for (z=1; z<=ArgCnt-1; z++) strcpy(ArgStr[z],ArgStr[z+1]);
      strcpy(ArgStr[ArgCnt],ArgStr[ArgCnt+1]);
     END
    if ((ArgCnt>=1) AND (ArgCnt<=2)) Try2Split(ArgCnt);
    if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
    else if (MomCPU<CPU6301) WrError(1500);
    else
     BEGIN
      if (*ArgStr[ArgCnt]=='#') strcpy(ArgStr[ArgCnt],ArgStr[ArgCnt]+1);
      Mask=EvalIntExpression(ArgStr[ArgCnt],Int8,&OK);
      if ((OK) AND (MomCPU==CPU6301))
       if (Mask>7)
        BEGIN
         WrError(1320); OK=False;
        END
       else
        BEGIN
         Mask=1 << Mask;
         if (Index==1) Mask=0xff-Mask;
        END
      if (OK)
       BEGIN
        DecodeAdr(1,ArgCnt-1,MModDir+MModInd);
        if (AdrMode!=ModNone)
         BEGIN
          CodeLen=PrefCnt+2+AdrCnt;
          if (MomCPU==CPU6301)
           BEGIN
            BAsmCode[PrefCnt]=0x62-Index;
            if (AdrMode==ModDir) BAsmCode[PrefCnt]+=0x10;
            BAsmCode[1+PrefCnt]=Mask;
            memcpy(BAsmCode+2+PrefCnt,AdrVals,AdrCnt);
           END
          else
           BEGIN
            BAsmCode[PrefCnt]=0x14+Index;
            if (AdrMode==ModInd) BAsmCode[PrefCnt]+=8;
            memcpy(BAsmCode+1+PrefCnt,AdrVals,AdrCnt);
            BAsmCode[1+PrefCnt+AdrCnt]=Mask;
           END
         END
       END
     END
END

	static void DecodeBTxx(Word Index)
BEGIN
   Boolean OK;
   Byte AdrByte;

   if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
   else if (MomCPU!=CPU6301) WrError(1500);
   else
    BEGIN
     AdrByte=EvalIntExpression(ArgStr[1],Int8,&OK);
     if (OK)
      if (AdrByte>7) WrError(1320);
      else
       BEGIN
        DecodeAdr(2,ArgCnt,MModDir+MModInd);
        if (AdrMode!=ModNone)
         BEGIN
          CodeLen=PrefCnt+2+AdrCnt;
          BAsmCode[1+PrefCnt]=1 << AdrByte;
          memcpy(BAsmCode+2+PrefCnt,AdrVals,AdrCnt);
          BAsmCode[PrefCnt]=0x65;
          BAsmCode[PrefCnt]+=Index;
          if (AdrMode==ModDir) BAsmCode[PrefCnt]+=0x10;
         END
       END
    END
END

/*---------------------------------------------------------------------------*/

	static void AddFixed(char *NName, CPUVar NMin, CPUVar NMax, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);

   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ].MaxCPU=NMax;
   FixedOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeFixed);
END

	static void AddRel(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);

   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeRel);
END

	static void AddALU8(char *NName, Boolean NMay, Byte NCode)
BEGIN
   if (InstrZ>=ALU8OrderCnt) exit(255);

   ALU8Orders[InstrZ].Name=NName;
   ALU8Orders[InstrZ].MayImm=NMay;
   ALU8Orders[InstrZ++].Code=NCode;
END

	static void AddALU16(char *NName, Boolean NMay, CPUVar NMin, Byte NShift, Byte NCode)
BEGIN
   if (InstrZ>=ALU16OrderCnt) exit(255);

   ALU16Orders[InstrZ].MayImm=NMay;
   ALU16Orders[InstrZ].MinCPU=NMin;
   ALU16Orders[InstrZ].PageShift=NShift;
   ALU16Orders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeALU16);
END

        static void AddSing8(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=Sing8OrderCnt) exit(255);

   Sing8Orders[InstrZ].Name=NName;
   Sing8Orders[InstrZ++].Code=NCode;
END

	static void AddBit63(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=Bit63OrderCnt) exit(255);

   Bit63Orders[InstrZ].Name=NName;
   Bit63Orders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeBit63);
END

	static void InitFields(void)
BEGIN
   InstTable=CreateInstTable(201);
   AddInstTable(InstTable,"JMP",0,DecodeJMP);
   AddInstTable(InstTable,"JSR",0,DecodeJSR);
   AddInstTable(InstTable,"BRCLR",1,DecodeBRxx);
   AddInstTable(InstTable,"BRSET",0,DecodeBRxx);
   AddInstTable(InstTable,"BCLR",1,DecodeBxx);
   AddInstTable(InstTable,"BSET",0,DecodeBxx);
   AddInstTable(InstTable,"BTST",6,DecodeBTxx);
   AddInstTable(InstTable,"BTGL",0,DecodeBTxx);   

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("ABA"  ,CPU6800, CPU6811, 0x001b); AddFixed("ABX"  ,CPU6301, CPU6811, 0x003a);
   AddFixed("ABY"  ,CPU6811, CPU6811, 0x183a); AddFixed("ASLD" ,CPU6301, CPU6811, 0x0005);
   AddFixed("CBA"  ,CPU6800, CPU6811, 0x0011); AddFixed("CLC"  ,CPU6800, CPU6811, 0x000c);
   AddFixed("CLI"  ,CPU6800, CPU6811, 0x000e); AddFixed("CLV"  ,CPU6800, CPU6811, 0x000a);
   AddFixed("DAA"  ,CPU6800, CPU6811, 0x0019); AddFixed("DES"  ,CPU6800, CPU6811, 0x0034);
   AddFixed("DEX"  ,CPU6800, CPU6811, 0x0009); AddFixed("DEY"  ,CPU6811, CPU6811, 0x1809);
   AddFixed("FDIV" ,CPU6811, CPU6811, 0x0003); AddFixed("IDIV" ,CPU6811, CPU6811, 0x0002);
   AddFixed("INS"  ,CPU6800, CPU6811, 0x0031); AddFixed("INX"  ,CPU6800, CPU6811, 0x0008);
   AddFixed("INY"  ,CPU6811, CPU6811, 0x1808); AddFixed("LSLD" ,CPU6301, CPU6811, 0x0005);
   AddFixed("LSRD" ,CPU6301, CPU6811, 0x0004); AddFixed("MUL"  ,CPU6301, CPU6811, 0x003d);
   AddFixed("NOP"  ,CPU6800, CPU6811, 0x0001); AddFixed("PSHX" ,CPU6301, CPU6811, 0x003c);
   AddFixed("PSHY" ,CPU6811, CPU6811, 0x183c); AddFixed("PULX" ,CPU6301, CPU6811, 0x0038);
   AddFixed("PULY" ,CPU6811, CPU6811, 0x1838); AddFixed("RTI"  ,CPU6800, CPU6811, 0x003b);
   AddFixed("RTS"  ,CPU6800, CPU6811, 0x0039); AddFixed("SBA"  ,CPU6800, CPU6811, 0x0010);
   AddFixed("SEC"  ,CPU6800, CPU6811, 0x000d); AddFixed("SEI"  ,CPU6800, CPU6811, 0x000f);
   AddFixed("SEV"  ,CPU6800, CPU6811, 0x000b); AddFixed("SLP"  ,CPU6301, CPU6301, 0x001a);
   AddFixed("STOP" ,CPU6811, CPU6811, 0x00cf); AddFixed("SWI"  ,CPU6800, CPU6811, 0x003f);
   AddFixed("TAB"  ,CPU6800, CPU6811, 0x0016); AddFixed("TAP"  ,CPU6800, CPU6811, 0x0006);
   AddFixed("TBA"  ,CPU6800, CPU6811, 0x0017); AddFixed("TPA"  ,CPU6800, CPU6811, 0x0007);
   AddFixed("TSX"  ,CPU6800, CPU6811, 0x0030); AddFixed("TSY"  ,CPU6811, CPU6811, 0x1830);
   AddFixed("TXS"  ,CPU6800, CPU6811, 0x0035); AddFixed("TYS"  ,CPU6811, CPU6811, 0x1835);
   AddFixed("WAI"  ,CPU6800, CPU6811, 0x003e); AddFixed("XGDX" ,CPU6811, CPU6811, 0x008f);
   AddFixed("XGDY" ,CPU6811, CPU6811, 0x188f);

   RelOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*RelOrderCnt); InstrZ=0;
   AddRel("BCC", 0x24); AddRel("BCS", 0x25);
   AddRel("BEQ", 0x27); AddRel("BGE", 0x2c);
   AddRel("BGT", 0x2e); AddRel("BHI", 0x22);
   AddRel("BHS", 0x24); AddRel("BLE", 0x2f);
   AddRel("BLO", 0x25); AddRel("BLS", 0x23);
   AddRel("BLT", 0x2d); AddRel("BMI", 0x2b);
   AddRel("BNE", 0x26); AddRel("BPL", 0x2a);
   AddRel("BRA", 0x20); AddRel("BRN", 0x21);
   AddRel("BSR", 0x8d); AddRel("BVC", 0x28);
   AddRel("BVS", 0x29);

   ALU8Orders=(ALU8Order *) malloc(sizeof(ALU8Order)*ALU8OrderCnt); InstrZ=0;
   AddALU8("ADC",True , 0x89);
   AddALU8("ADD",True , 0x8b);
   AddALU8("AND",True , 0x84);
   AddALU8("BIT",True , 0x85);
   AddALU8("CMP",True , 0x81);
   AddALU8("EOR",True , 0x88);
   AddALU8("LDA",True , 0x86);
   AddALU8("ORA",True , 0x8a);
   AddALU8("SBC",True , 0x82);
   AddALU8("STA",False, 0x87);
   AddALU8("SUB",True , 0x80);

   ALU16Orders=(ALU16Order *) malloc(sizeof(ALU16Order)*ALU16OrderCnt); InstrZ=0;
   AddALU16("ADDD", True , CPU6301, 0, 0xc3);
   AddALU16("CPD" , True , CPU6811, 1, 0x83);
   AddALU16("CPX" , True , CPU6800, 2, 0x8c);
   AddALU16("CPY" , True , CPU6811, 3, 0x8c);
   AddALU16("LDD" , True , CPU6301, 0, 0xcc);
   AddALU16("LDS" , True , CPU6800, 0, 0x8e);
   AddALU16("LDX" , True , CPU6800, 2, 0xce);
   AddALU16("LDY" , True , CPU6811, 3, 0xce);
   AddALU16("STD" , False, CPU6301, 0, 0xcd);
   AddALU16("STS" , False, CPU6800, 0, 0x8f);
   AddALU16("STX" , False, CPU6800, 2, 0xcf);
   AddALU16("STY" , False, CPU6811, 3, 0xcf);
   AddALU16("SUBD", True , CPU6301, 0, 0x83);

   Sing8Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*Sing8OrderCnt); InstrZ=0;
   AddSing8("ASL", 0x48);
   AddSing8("ASR", 0x47);
   AddSing8("CLR", 0x4f);
   AddSing8("COM", 0x43);
   AddSing8("DEC", 0x4a);
   AddSing8("INC", 0x4c);
   AddSing8("LSL", 0x48);
   AddSing8("LSR", 0x44);
   AddSing8("NEG", 0x40);
   AddSing8("ROL", 0x49);
   AddSing8("ROR", 0x46);
   AddSing8("TST", 0x4d);

   Bit63Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*Bit63OrderCnt); InstrZ=0;
   AddBit63("AIM", 0x61); AddBit63("EIM", 0x65); 
   AddBit63("OIM", 0x62); AddBit63("TIM", 0x6b);
END

	static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(FixedOrders);
   free(RelOrders);
   free(ALU8Orders);
   free(ALU16Orders);
   free(Sing8Orders);
   free(Bit63Orders);
END

        static Boolean SplitAcc(char *Op)
BEGIN
   char Ch;
   Integer z;
   int OpLen=strlen(Op),OpPartLen=strlen(OpPart);

   Ch=OpPart[OpPartLen-1];
   if ((OpLen+1==OpPartLen) AND
       (strncmp(OpPart,Op,OpLen)==0) AND
       ((Ch=='A') OR (Ch=='B')))
    BEGIN
     for (z=ArgCnt; z>=1; z--) strcpy(ArgStr[z+1],ArgStr[z]);
     ArgStr[1][0]=Ch; ArgStr[1][1]='\0';
     OpPart[OpPartLen-1]='\0'; ArgCnt++;
    END
   return (Memo(Op));
END

	static Boolean DecodePseudo(void)
BEGIN
   return False;
END

	static void MakeCode_68(void)
BEGIN
   int z;

   CodeLen=0; DontPrint=False; PrefCnt=0; AdrCnt=0; OpSize=0;

   /* Operandengroesse festlegen */

   if (*AttrPart!='\0')
    switch (toupper(*AttrPart))
     BEGIN
      case 'B':OpSize=0; break;
      case 'W':OpSize=1; break;
      case 'L':OpSize=2; break;
      case 'Q':OpSize=3; break;
      case 'S':OpSize=4; break;
      case 'D':OpSize=5; break;
      case 'X':OpSize=6; break;
      case 'P':OpSize=7; break;
      default:
       WrError(1107); return;
     END

   /* zu ignorierendes */

   if (*OpPart=='\0') return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(True)) return;
   if (DecodeMoto16Pseudo(OpSize,True)) return;

   /* gehashtes */

   if (LookupInstTable(InstTable,OpPart)) return;

   /* Anweisungen ohne Argument */

   /* Sonderfall : XGDX hat anderen Code bei 6301 !!!! */

   if ((MomCPU==CPU6301) AND (Memo("XGDX")))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x18;
      END
     return;
    END

   for (z=0; z<ALU8OrderCnt; z++)
    if (SplitAcc(ALU8Orders[z].Name))
     BEGIN
      if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
      else
       BEGIN
        DecodeAdr(2,ArgCnt,((ALU8Orders[z].MayImm)?MModImm:0)+MModInd+MModExt+MModDir);
        if (AdrMode!=ModNone)
         BEGIN
          BAsmCode[PrefCnt]=
          ALU8Orders[z].Code+(AdrPart << 4);
          DecodeAdr(1,1,1);
          if (AdrMode!=ModNone)
           BEGIN
            BAsmCode[PrefCnt]+=AdrPart << 6;
            CodeLen=PrefCnt+1+AdrCnt;
            memcpy(BAsmCode+1+PrefCnt,AdrVals,AdrCnt);
           END
         END
       END
      return;
     END

   for (z=0; z<Sing8OrderCnt; z++)
    if (SplitAcc(Sing8Orders[z].Name))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else
       BEGIN
        DecodeAdr(1,ArgCnt,MModAcc+MModExt+MModInd);
        if (AdrMode!=ModNone)
         BEGIN
          CodeLen=PrefCnt+1+AdrCnt;
          BAsmCode[PrefCnt]=Sing8Orders[z].Code+(AdrPart << 4);
          memcpy(BAsmCode+1+PrefCnt,AdrVals,AdrCnt);
         END
       END
      return;
     END

   if ((SplitAcc("PSH")) OR (SplitAcc("PUL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(1,1,MModAcc);
       if (AdrMode!=ModNone)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x32+AdrPart;
         if (Memo("PSH")) BAsmCode[0]+=4;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static Boolean IsDef_68(void)
BEGIN
   return False;
END

        static void SwitchFrom_68()
BEGIN
   DeinitFields();
END

	static void SwitchTo_68(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x61; NOPCode=0x01;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   MakeCode=MakeCode_68; IsDef=IsDef_68;
   SwitchFrom=SwitchFrom_68; InitFields();
   AddMoto16PseudoONOFF();

   SetFlag(&DoPadding,DoPaddingName,False);
END

	void code68_init(void)
BEGIN
   CPU6800=AddCPU("6800",SwitchTo_68);
   CPU6301=AddCPU("6301",SwitchTo_68);
   CPU6811=AddCPU("6811",SwitchTo_68);
END
