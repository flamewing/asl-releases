/* code96.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MCS/96-Familie                                              */
/*                                                                           */
/* Historie: 10.11.1996                                                      */
/*           16. 3.1997 80196N/80296                                         */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct 
         {
          char *Name;
          Byte Code;
         } FixedOrder;

typedef struct 
         {
          char *Name;
          Byte Code;
          CPUVar MinCPU,MaxCPU;
         } BaseOrder;

typedef struct 
         {
          char *Name; 
          Byte Code;
          Boolean Reloc;
         } MacOrder;

#define FixedOrderCnt 16

#define ALU3OrderCnt 5

#define ALU2OrderCnt 9

#define ALU1OrderCnt 6

#define ShiftOrderCnt 3

#define RelOrderCnt 16

#define MacOrderCnt 8

#define RptOrderCnt 34


static char *ShiftOrders[ShiftOrderCnt]={"SHR","SHL","SHRA"};

#define ModNone (-1)
#define ModDir 0
#define MModDir (1 << ModDir)
#define ModMem 1
#define MModMem (1 << ModMem)
#define ModImm 2
#define MModImm (1 << ModImm)

#define SFRStart 2
#define SFRStop 0x17

static BaseOrder *FixedOrders;
static FixedOrder *ALU3Orders;
static FixedOrder *ALU2Orders;
static FixedOrder *ALU1Orders;
static FixedOrder *RelOrders;
static MacOrder *MacOrders;
static FixedOrder *RptOrders;

static CPUVar CPU8096,CPU80196,CPU80196N,CPU80296;
static SimpProc SaveInitProc;

static Byte AdrMode;
static ShortInt AdrType;
static Byte AdrVals[4];
static ShortInt OpSize;

static LongInt WSRVal,WSR1Val;
static Word WinStart,WinStop,WinEnd,WinBegin;
static Word Win1Start,Win1Stop,Win1Begin,Win1End;

IntType MemInt;

/*---------------------------------------------------------------------------*/


   	static void AddFixed(char *NName, Byte NCode, CPUVar NMin, CPUVar NMax)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ++].MaxCPU=NMax;
END

        static void AddALU3(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ALU3OrderCnt) exit(255);
   ALU3Orders[InstrZ].Name=NName;
   ALU3Orders[InstrZ++].Code=NCode;
END

        static void AddALU2(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Name=NName;
   ALU2Orders[InstrZ++].Code=NCode;
END

        static void AddALU1(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ALU1OrderCnt) exit(255);
   ALU1Orders[InstrZ].Name=NName;
   ALU1Orders[InstrZ++].Code=NCode;
END

        static void AddRel(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
END

        static void AddMac(char *NName, Byte NCode, Boolean NRel)
BEGIN
   if (InstrZ>=MacOrderCnt) exit(255);
   MacOrders[InstrZ].Name=NName;
   MacOrders[InstrZ].Code=NCode;
   MacOrders[InstrZ++].Reloc=NRel;
END

        static void AddRpt(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=RptOrderCnt) exit(255);
   RptOrders[InstrZ].Name=NName;
   RptOrders[InstrZ++].Code=NCode;
END

   	static void InitFields(void)
BEGIN
   FixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CLRC" ,0xf8,CPU8096  ,CPU80296 );
   AddFixed("CLRVT",0xfc,CPU8096  ,CPU80296 );
   AddFixed("DI"   ,0xfa,CPU8096  ,CPU80296 );
   AddFixed("DPTS" ,0xea,CPU80196 ,CPU80196N);
   AddFixed("EI"   ,0xfb,CPU8096  ,CPU80296 );
   AddFixed("EPTS" ,0xeb,CPU80196 ,CPU80196N);
   AddFixed("NOP"  ,0xfd,CPU8096  ,CPU80296 );
   AddFixed("POPA" ,0xf5,CPU80196 ,CPU80296 );
   AddFixed("POPF" ,0xf3,CPU8096  ,CPU80296 );
   AddFixed("PUSHA",0xf4,CPU80196 ,CPU80296 );
   AddFixed("PUSHF",0xf2,CPU8096  ,CPU80296 );
   AddFixed("RET"  ,0xf0,CPU8096  ,CPU80296 );
   AddFixed("RSC"  ,0xff,CPU8096  ,CPU80296 );
   AddFixed("SETC" ,0xf9,CPU8096  ,CPU80296 );
   AddFixed("TRAP" ,0xf7,CPU8096  ,CPU80296 );
   AddFixed("RETI" ,0xe5,CPU80196N,CPU80296 );

   ALU3Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU3OrderCnt); InstrZ=0;
   AddALU3("ADD" , 0x01);
   AddALU3("AND" , 0x00);
   AddALU3("MUL" , 0x83);   /* ** */
   AddALU3("MULU", 0x03);
   AddALU3("SUB" , 0x02);

   ALU2Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU2OrderCnt); InstrZ=0;
   AddALU2("ADDC", 0xa4);
   AddALU2("CMP" , 0x88);
   AddALU2("DIV" , 0x8c);   /* ** */
   AddALU2("DIVU", 0x8c);
   AddALU2("LD"  , 0xa0);
   AddALU2("OR"  , 0x80);
   AddALU2("ST"  , 0xc0);
   AddALU2("SUBC", 0xa8);
   AddALU2("XOR" , 0x84);

   ALU1Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU1OrderCnt); InstrZ=0;
   AddALU1("CLR", 0x01);
   AddALU1("DEC", 0x05);
   AddALU1("EXT", 0x06);
   AddALU1("INC", 0x07);
   AddALU1("NEG", 0x03);
   AddALU1("NOT", 0x02);

   RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCnt); InstrZ=0;
   AddRel("JC"   , 0xdb);
   AddRel("JE"   , 0xdf);
   AddRel("JGE"  , 0xd6);
   AddRel("JGT"  , 0xd2);
   AddRel("JH"   , 0xd9);
   AddRel("JLE"  , 0xda);
   AddRel("JLT"  , 0xde);
   AddRel("JNC"  , 0xd3);
   AddRel("JNE"  , 0xd7);
   AddRel("JNH"  , 0xd1);
   AddRel("JNST" , 0xd0);
   AddRel("JNV"  , 0xd5);
   AddRel("JNVT" , 0xd4);
   AddRel("JST"  , 0xd8);
   AddRel("JV"   , 0xdd);
   AddRel("JVT"  , 0xdc);

   MacOrders=(MacOrder *) malloc(sizeof(MacOrder)*MacOrderCnt); InstrZ=0;
   AddMac("MAC"   ,0x00,False); AddMac("SMAC"  ,0x01,False);
   AddMac("MACR"  ,0x04,True ); AddMac("SMACR" ,0x05,True ); 
   AddMac("MACZ"  ,0x08,False); AddMac("SMACZ" ,0x09,False);
   AddMac("MACRZ" ,0x0c,True ); AddMac("SMACRZ",0x0d,True );

   RptOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RptOrderCnt); InstrZ=0;
   AddRpt("RPT"    ,0x00); AddRpt("RPTNST" ,0x10); AddRpt("RPTNH"  ,0x11);
   AddRpt("RPTGT"  ,0x12); AddRpt("RPTNC"  ,0x13); AddRpt("RPTNVT" ,0x14);
   AddRpt("RPTNV"  ,0x15); AddRpt("RPTGE"  ,0x16); AddRpt("RPTNE"  ,0x17);
   AddRpt("RPTST"  ,0x18); AddRpt("RPTH"   ,0x19); AddRpt("RPTLE"  ,0x1a);
   AddRpt("RPTC"   ,0x1b); AddRpt("RPTVT"  ,0x1c); AddRpt("RPTV"   ,0x1d);
   AddRpt("RPTLT"  ,0x1e); AddRpt("RPTE"   ,0x1f); AddRpt("RPTI"   ,0x20);
   AddRpt("RPTINST",0x30); AddRpt("RPTINH" ,0x31); AddRpt("RPTIGT" ,0x32);
   AddRpt("RPTINC" ,0x33); AddRpt("RPTINVT",0x34); AddRpt("RPTINV" ,0x35);
   AddRpt("RPTIGE" ,0x36); AddRpt("RPTINE" ,0x37); AddRpt("RPTIST" ,0x38);
   AddRpt("RPTIH"  ,0x39); AddRpt("RPTILE" ,0x3a); AddRpt("RPTIC"  ,0x3b);
   AddRpt("RPTIVT" ,0x3c); AddRpt("RPTIV"  ,0x3d); AddRpt("RPTILT" ,0x3e);
   AddRpt("RPTIE"  ,0x3f);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(ALU3Orders);
   free(ALU2Orders);
   free(ALU1Orders);
   free(RelOrders);
   free(MacOrders);
   free(RptOrders);
END

/*-------------------------------------------------------------------------*/

	static void ChkSFR(Word Adr)
BEGIN
   if ((Adr>=SFRStart) & (Adr<=SFRStop)) WrError(190);
END

        static void Chk296(Word Adr)
BEGIN
   if ((MomCPU==CPU80296) AND (Adr<=1)) WrError(190);
END

	static Boolean ChkWork(Word *Adr)
BEGIN
   /* Registeradresse, die von Fenstern ueberdeckt wird ? */

   if ((*Adr>=WinBegin) AND (*Adr<=WinEnd)) return False;

   else if ((*Adr>=Win1Begin) AND (*Adr<=Win1End)) return False;

   /* Speicheradresse in Fenster ? */

   else if ((*Adr>=WinStart) AND (*Adr<=WinStop))
    BEGIN
     *Adr=(*Adr)-WinStart+WinBegin; return True;
    END

   else if ((*Adr>=Win1Start) AND (*Adr<=Win1Stop))
    BEGIN
     *Adr=(*Adr)-Win1Start+Win1Begin; return True;
    END

   /* Default */

   else return (*Adr<=0xff);
END

	static void ChkAlign(Byte Adr)
BEGIN
   if (((OpSize==0) AND ((Adr & 1)!=0))
   OR  ((OpSize==1) AND ((Adr & 3)!=0))) WrError(180);
END

	static void ChkAdr(Byte Mask)
BEGIN
   if ((AdrType==ModDir) AND ((Mask & MModDir)==0))
    BEGIN
     AdrType=ModMem; AdrMode=0;
    END

   if ((AdrType!=ModNone) AND ((Mask & (1 << AdrType))==0))
    BEGIN
     WrError(1350); AdrType=ModNone; AdrCnt=0;
    END
END

	static void DecodeAdr(char *Asc, Byte Mask, Boolean AddrWide)
BEGIN
   LongInt AdrInt;
   LongWord AdrWord;
   Word BReg;
   Boolean OK;
   char *p,*p2;
   int l;
   Byte Reg;
   LongWord OMask;

   AdrType=ModNone; AdrCnt=0;
   OMask=(1 << OpSize)-1;

   if (*Asc=='#')
    BEGIN
     switch (OpSize)
      BEGIN
       case -1:
        WrError(1132); break;
       case 0:
	AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
	if (OK)
	 BEGIN
	  AdrType=ModImm; AdrCnt=1; AdrMode=1;
	 END
        break;
       case 1:
	AdrWord=EvalIntExpression(Asc+1,Int16,&OK);
	if (OK)
	 BEGIN
	  AdrType=ModImm; AdrCnt=2; AdrMode=1;
	  AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
	 END
        break;
      END
     ChkAdr(Mask); return;
    END

   p=QuotPos(Asc,'[');
   if (p!=Nil)
    BEGIN
     p2=RQuotPos(Asc,']'); l=strlen(Asc);
     if ((p2>Asc+l-1) OR (p2<Asc+l-2)) WrError(1350);
     else
      BEGIN
       FirstPassUnknown=False; *p2='\0';
       BReg=EvalIntExpression(p+1,Int16,&OK);
       if (FirstPassUnknown) BReg=0;
       if (OK)
        if (NOT ChkWork(&BReg)) WrError(1320);
        else
	 BEGIN
	  Reg=Lo(BReg); ChkSFR(Reg);
	  if ((Reg&1)==1) WrError(1351);
	  else if ((p==Asc) AND (p2==Asc+l-2) AND (Asc[l-1]=='+'))
	   BEGIN
	    AdrType=ModMem; AdrMode=2; AdrCnt=1; AdrVals[0]=Reg+1;
	   END
	  else if (p2!=Asc+l-1) WrError(1350);
	  else if (p==Asc)
	   BEGIN
	    AdrType=ModMem; AdrMode=2; AdrCnt=1; AdrVals[0]=Reg;
	   END
	  else
	   BEGIN
            *p='\0';
	    if (NOT AddrWide) AdrInt=EvalIntExpression(Asc,Int16,&OK); 
            else AdrInt=EvalIntExpression(Asc,Int24,&OK);
	    if (OK)
	     if (AdrInt==0)
	      BEGIN
	       AdrType=ModMem; AdrMode=2; AdrCnt=1; AdrVals[0]=Reg;
	      END
             else if (AddrWide)
              BEGIN
               AdrType=ModMem; AdrMode=3; AdrCnt=4;
               AdrVals[0]=Reg; AdrVals[1]=AdrInt & 0xff;
               AdrVals[2]=(AdrInt >> 8) & 0xff;
               AdrVals[3]=(AdrInt >> 16) & 0xff;
              END
	     else if ((AdrInt>=-128) AND (AdrInt<127))
	      BEGIN
	       AdrType=ModMem; AdrMode=3; AdrCnt=2;
	       AdrVals[0]=Reg; AdrVals[1]=Lo(AdrInt);
	      END
	     else
	      BEGIN
	       AdrType=ModMem; AdrMode=3; AdrCnt=3;
	       AdrVals[0]=Reg+1; AdrVals[1]=Lo(AdrInt); AdrVals[2]=Hi(AdrInt);
	      END
	   END
	 END
      END
    END
   else
    BEGIN
     FirstPassUnknown=False;
     AdrWord=EvalIntExpression(Asc,MemInt,&OK);
     if (FirstPassUnknown) AdrWord&=(0xffffffff-OMask);
     if (OK)
      if ((AdrWord & OMask)!=0) WrError(1325);
      else
       BEGIN
        BReg=AdrWord & 0xffff;
        if (((BReg & 0xffff0000)==0) AND (ChkWork(&BReg)))
 	 BEGIN
 	  AdrType=ModDir; AdrCnt=1; AdrVals[0]=Lo(BReg);
 	 END
        else if (AddrWide)
         BEGIN
          AdrType=ModMem; AdrMode=3; AdrCnt=4; AdrVals[0]=0;
          AdrVals[1]=AdrWord & 0xff; 
          AdrVals[2]=(AdrWord >> 8) & 0xff;
          AdrVals[3]=(AdrWord >> 16) & 0xff;
         END
        else if (AdrWord>=0xff80)
 	 BEGIN
 	  AdrType=ModMem; AdrMode=3; AdrCnt=2; AdrVals[0]=0;
 	  AdrVals[1]=Lo(AdrWord);
 	 END
        else
 	 BEGIN
 	  AdrType=ModMem; AdrMode=3; AdrCnt=3; AdrVals[0]=1;
 	  AdrVals[1]=Lo(AdrWord); AdrVals[2]=Hi(AdrWord);
 	 END
       END
    END

   ChkAdr(Mask);
END

	static void CalcWSRWindow(void)
BEGIN
   if (WSRVal<=0x0f)
    BEGIN
     WinStart=0xffff; WinStop=0; WinBegin=0xff; WinEnd=0;
    END
   else if (WSRVal<=0x1f)
    BEGIN
     WinBegin=0x80; WinEnd=0xff;
     if (WSRVal<0x18) WinStart=(WSRVal-0x10) << 7;
     else WinStart=(WSRVal+0x20) << 7;
     WinStop=WinStart+0x7f;
    END
   else if (WSRVal<=0x3f)
    BEGIN
     WinBegin=0xc0; WinEnd=0xff;
     if (WSRVal<0x30) WinStart=(WSRVal-0x20) << 6;
     else WinStart=(WSRVal+0x40) << 6;
     WinStop=WinStart+0x3f;
    END
   else if (WSRVal<=0x7f)
    BEGIN
     WinBegin=0xe0; WinEnd=0xff;
     if (WSRVal<0x60) WinStart=(WSRVal-0x40) << 5;
     else WinStart=(WSRVal+0x80) << 5;
     WinStop=WinStart+0x1f;
    END
   if ((WinStop>0x1fdf) AND (MomCPU<CPU80296)) WinStop=0x1fdf;
END

        static void CalcWSR1Window(void)
BEGIN
   if (WSR1Val<=0x1f)
    BEGIN
     Win1Start=0xffff; Win1Stop=0; Win1Begin=0xff; Win1End=0;
    END
   else if (WSR1Val<=0x3f)
    BEGIN
     Win1Begin=0x40; Win1End=0x7f;
     if (WSR1Val<0x30) Win1Start=(WSR1Val-0x20) << 6;
     else Win1Start=(WSR1Val+0x40) << 6;
     Win1Stop=Win1Start+0x3f;
    END
   else if (WSR1Val<=0x7f)
    BEGIN
     Win1Begin=0x60; Win1End=0x7f;
     if (WSR1Val<0x60) Win1Start=(WSR1Val-0x40) << 5;
     else Win1Start=(WSR1Val+0x80) << 5;
     Win1Stop=Win1Start+0x1f;
    END
   else
    BEGIN
     Win1Begin=0x40; Win1End=0x7f;
     Win1Start=(WSR1Val+0x340) << 6;
     Win1Stop=Win1Start+0x3f;
    END
END

	static Boolean DecodePseudo(void)
BEGIN
#define ASSUME96Count 2
   static ASSUMERec ASSUME96s[ASSUME96Count]=
	     {{"WSR", &WSRVal, 0, 0xff, 0x00},
              {"WSR1", &WSR1Val, 0, 0xbf, 0x00}};

   if (Memo("ASSUME"))
    BEGIN
     if (MomCPU<CPU80196) WrError(1500);
     else CodeASSUME(ASSUME96s,(MomCPU>=CPU80296)?ASSUME96Count:1);
     WSRVal&=0x7f;
     CalcWSRWindow(); CalcWSR1Window();
     return True;
    END

   return False;
END

	static Boolean BMemo(char *Name)
BEGIN
   int l;

   if (strncmp(OpPart,Name,l=strlen(Name))!=0) return False;
   switch (OpPart[l])
    BEGIN
     case '\0':
      OpSize=1; return True;
     case 'B':
      if (OpPart[l+1]=='\0')
       BEGIN
        OpSize=0; return True;
       END
      else return False;
     default:
      return False;
    END
END

	static Boolean LMemo(char *Name)
BEGIN
   int l;

   if (strncmp(OpPart,Name,l=strlen(Name))!=0) return False;
   switch (OpPart[l])
    BEGIN
     case '\0':
      OpSize=1; return True;
     case 'B':
      if (OpPart[l+1]=='\0')
       BEGIN
        OpSize=0; return True;
       END
      else return False;
     case 'L':
      if (OpPart[l+1]=='\0')
       BEGIN
        OpSize=2; return True;
       END
      else return False;
     default:
      return False;
    END
END

	static void MakeCode_96(void)
BEGIN
   Boolean OK,Special,IsShort;
   Word AdrWord;
   int z;
   LongInt AdrInt;
   Byte Start,HReg,Mask;

   CodeLen=0; DontPrint=False; OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<FixedOrders[z].MinCPU) WrError(1500);
      else BAsmCode[(CodeLen=1)-1]=FixedOrders[z].Code;
      return;
     END

   /* Arithmetik */

   for (z=0; z<ALU3OrderCnt; z++)
    if (BMemo(ALU3Orders[z].Name))
     BEGIN
      if ((ArgCnt!=2) AND (ArgCnt!=3)) WrError(1110);
      else
       BEGIN
        Start=0; Special=(strncmp(ALU3Orders[z].Name,"MUL",3)==0);
        if ((ALU3Orders[z].Code & 0x80)!=0) BAsmCode[Start++]=0xfe;
        BAsmCode[Start++]=0x40+(Ord(ArgCnt==2) << 5)
       		          +((1-OpSize) << 4)
       		          +((ALU3Orders[z].Code & 0x7f) << 2);
        DecodeAdr(ArgStr[ArgCnt],MModImm+MModMem,False);
        if (AdrType!=ModNone)
         BEGIN
          BAsmCode[Start-1]+=AdrMode;
          memcpy(BAsmCode+Start,AdrVals,AdrCnt); Start+=AdrCnt;
          if ((Special) AND (AdrMode==0)) ChkSFR(AdrVals[0]);
          if (ArgCnt==3)
           BEGIN
            DecodeAdr(ArgStr[2],MModDir,False);
            OK=(AdrType!=ModNone);
            if (OK)
             BEGIN
              BAsmCode[Start++]=AdrVals[0];
              if (Special) ChkSFR(AdrVals[0]);
             END
           END
          else OK=True;
          if (OK)
           BEGIN
            DecodeAdr(ArgStr[1],MModDir,False);
            if (AdrType!=ModNone)
             BEGIN
              BAsmCode[Start]=AdrVals[0]; CodeLen=Start+1;
              if (Special)
       	       BEGIN
       	        ChkSFR(AdrVals[0]);
                Chk296(AdrVals[0]);
                ChkAlign(AdrVals[0]);
       	       END
             END
           END
         END
       END
      return;
     END

   for (z=0; z<ALU2OrderCnt; z++)
    if (BMemo(ALU2Orders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        Start=0; Special=(strncmp(OpPart,"DIV",3)==0);
        if (strcmp(ALU2Orders[z].Name,"DIV")==0) BAsmCode[Start++]=0xfe;
        HReg=(1+Ord(strcmp(ALU2Orders[z].Name,"ST")!=0)) << 1;
        BAsmCode[Start++]=ALU2Orders[z].Code+((1-OpSize) << HReg);
        Mask=MModMem; if (NOT BMemo("ST")) Mask+=MModImm;
        DecodeAdr(ArgStr[2],Mask,False);
        if (AdrType!=ModNone)
         BEGIN
          BAsmCode[Start-1]+=AdrMode;
          memcpy(BAsmCode+Start,AdrVals,AdrCnt); Start+=AdrCnt;
          if ((Special) AND (AdrMode==0)) ChkSFR(AdrVals[0]);
          DecodeAdr(ArgStr[1],MModDir,False);
          if (AdrType!=ModNone)
           BEGIN
            BAsmCode[Start]=AdrVals[0]; CodeLen=1+Start;
            if (Special)
             BEGIN
              ChkSFR(AdrVals[0]); ChkAlign(AdrVals[0]);
             END
           END
         END
       END
      return;
     END

   if (Memo("CMPL"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      BEGIN
       OpSize=2;
       DecodeAdr(ArgStr[1],MModDir,False);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[2]=AdrVals[0];
         DecodeAdr(ArgStr[2],MModDir,False);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[1]=AdrVals[0]; BAsmCode[0]=0xc5; CodeLen=3;
          END
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     OpSize=1;
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       Mask=MModMem; if (Memo("PUSH")) Mask+=MModImm;
       DecodeAdr(ArgStr[1],Mask,False);
       if (AdrType!=ModNone)
	BEGIN
	 CodeLen=1+AdrCnt;
	 BAsmCode[0]=0xc8+AdrMode+(Ord(Memo("POP")) << 2);
	 memcpy(BAsmCode+1,AdrVals,AdrCnt);
	END
      END
     return;
    END

   if ((Memo("BMOV")) OR (Memo("BMOVI")) OR (Memo("EBMOVI")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else if ((MomCPU<CPU80196N) AND (Memo("EBMOVI"))) WrError(1500);
     else
      BEGIN
       OpSize=2; DecodeAdr(ArgStr[1],MModDir,False);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[2]=AdrVals[0];
         OpSize=1; DecodeAdr(ArgStr[2],MModDir,False);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[1]=AdrVals[0];
           if (Memo("BMOVI")) BAsmCode[0]=0xad;
           else if (Memo("BMOV")) BAsmCode[0]=0xc1;
           else BAsmCode[0]=0xe4;          
           CodeLen=3;
          END
        END
      END
     return;
    END

   for (z=0; z<ALU1OrderCnt; z++)
    if (BMemo(ALU1Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModDir,False);
        if (AdrType!=ModNone)
         BEGIN
          CodeLen=1+AdrCnt;
          BAsmCode[0]=ALU1Orders[z].Code+((1-OpSize) << 4);
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          if (BMemo("EXT")) ChkAlign(AdrVals[0]);
         END
       END
      return;
     END

   if (BMemo("XCH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModMem+MModDir,False);
       switch (AdrType)
        BEGIN
         case ModMem:
          if (AdrMode==1) WrError(1350);
          else
           BEGIN
            memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=AdrCnt;
            BAsmCode[0]=0x04+((1-OpSize) << 4)+AdrMode;
            DecodeAdr(ArgStr[2],MModDir,False);
            if (AdrType!=ModNone)
             BEGIN
              BAsmCode[1+HReg]=AdrVals[0]; CodeLen=2+HReg;
             END
           END
          break;
         case ModDir:
          HReg=AdrVals[0];
          DecodeAdr(ArgStr[2],MModMem,False);
          if (AdrType!=ModNone)
           if (AdrMode==1) WrError(1350);
           else
            BEGIN
             BAsmCode[0]=0x04+((1-OpSize) << 4)+AdrMode;
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
             BAsmCode[1+AdrCnt]=HReg; CodeLen=2+AdrCnt;
            END
          break;
        END
      END
     return;
    END

   if ((Memo("LDBZE")) OR (Memo("LDBSE")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[2],MModMem+MModImm,False);
       if (AdrType!=ModNone)
	BEGIN
	 BAsmCode[0]=0xac+(Ord(Memo("LDBSE")) << 4)+AdrMode;
	 memcpy(BAsmCode+1,AdrVals,AdrCnt); Start=1+AdrCnt;
	 OpSize=1; DecodeAdr(ArgStr[1],MModDir,False);
	 if (AdrType!=ModNone)
	  BEGIN
	   BAsmCode[Start]=AdrVals[0]; CodeLen=1+Start;
	  END
	END
      END
     return;
    END

   if (Memo("NORML"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OpSize=0; DecodeAdr(ArgStr[2],MModDir,False);
       if (AdrType!=ModNone)
	BEGIN
	 BAsmCode[1]=AdrVals[0];
	 OpSize=1; DecodeAdr(ArgStr[1],MModDir,False);
	 if (AdrType!=ModNone)
	  BEGIN
	   CodeLen=3; BAsmCode[0]=0x0f; BAsmCode[2]=AdrVals[0];
	   ChkAlign(AdrVals[0]);
	  END
	END
      END
     return;
    END

   if (Memo("IDLPD"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      BEGIN
       OpSize=0; DecodeAdr(ArgStr[1],MModImm,False);
       if (AdrType!=ModNone)
        BEGIN
         CodeLen=2; BAsmCode[0]=0xf6; BAsmCode[1]=AdrVals[0];
        END
      END
     return;
    END

   for (z=0; z<ShiftOrderCnt; z++)
    if (LMemo(ShiftOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	DecodeAdr(ArgStr[1],MModDir,False);
	if (AdrType!=ModNone)
	 BEGIN
	  BAsmCode[0]=0x08+z+(Ord(OpSize==0) << 4)+(Ord(OpSize==2) << 2);
	  BAsmCode[2]=AdrVals[0];
	  OpSize=0; DecodeAdr(ArgStr[2],MModDir+MModImm,False);
	  if (AdrType!=ModNone)
	   if ((AdrType==ModImm) AND (AdrVals[0]>15)) WrError(1320);
	   else if ((AdrType==ModDir) AND (AdrVals[0]<16)) WrError(1315);
	   else
	    BEGIN
	     BAsmCode[1]=AdrVals[0]; CodeLen=3;
	    END
	 END
       END
      return;
     END

   if (Memo("SKIP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       OpSize=0; DecodeAdr(ArgStr[1],MModDir,False);
       if (AdrType!=ModNone)
	BEGIN
	 CodeLen=2; BAsmCode[0]=0; BAsmCode[1]=AdrVals[0];
	END
      END
     return;
    END

   if ((BMemo("ELD")) OR (BMemo("EST")))
    BEGIN 
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196N) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModMem,True);
       if (AdrType==ModMem)
        if ((AdrMode==2) AND (Odd(AdrVals[0]))) WrError(1350); /* kein Autoincrement */
        else
         BEGIN
          BAsmCode[0]=(AdrMode & 1)+((1-OpSize) << 1);
          if (OpPart[1]=='L') BAsmCode[0]+=0xe8;
                         else BAsmCode[0]+=0x1c;
          memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=1+AdrCnt;
          DecodeAdr(ArgStr[1],MModDir,False);
          if (AdrType==ModDir)  
           BEGIN
            BAsmCode[HReg]=AdrVals[0]; CodeLen=HReg+1;
           END; 
         END;
      END;
     return;
    END

   for (z=0; z<MacOrderCnt; z++)
    if (Memo(MacOrders[z].Name))
     BEGIN
      if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
      else if (MomCPU<CPU80296) WrError(1500);
      else
       BEGIN
        OpSize=1; BAsmCode[0]=0x4c+(Ord(ArgCnt==1) << 5);
        if (MacOrders[z].Reloc) DecodeAdr(ArgStr[ArgCnt],MModMem,False);
        else DecodeAdr(ArgStr[ArgCnt],MModMem+MModImm,False);
        if (AdrType!=ModNone)
         BEGIN
          BAsmCode[0]+=AdrMode;
          memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=1+AdrCnt;
          if (ArgCnt==2)
           BEGIN
            DecodeAdr(ArgStr[1],MModDir,False);
            if (AdrType==ModDir)
             BEGIN
              BAsmCode[HReg]=AdrVals[0]; HReg++;
             END
           END
          if (AdrType!=ModNone)
           BEGIN
            BAsmCode[HReg]=MacOrders[z].Code; CodeLen=1+HReg;
           END
         END  
       END
      return;
     END

   if ((Memo("MVAC")) OR (Memo("MSAC")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80296) WrError(1500);
     else
      BEGIN
       OpSize=2; DecodeAdr(ArgStr[1],MModDir,False);
       if (AdrType==ModDir)
        BEGIN
         BAsmCode[0]=0x0d; BAsmCode[2]=AdrVals[0]+1+(Ord(Memo("MSAC")) << 1);
         OpSize=0; DecodeAdr(ArgStr[2],MModImm+MModDir,False);
         BAsmCode[1]=AdrVals[0];
         switch (AdrType)
          BEGIN
           case ModImm:
            if (AdrVals[0]>31) WrError(1320); else CodeLen=3;
            break;
           case ModDir:
            if (AdrVals[0]<32) WrError(1315); else CodeLen=3;
          END
        END 
      END      
     return;  
    END        

   for (z=0; z<RptOrderCnt; z++)
    if (Memo(RptOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<CPU80296) WrError(1500);
      else
       BEGIN
        OpSize=1; DecodeAdr(ArgStr[1],MModImm+MModMem,False);
        if (AdrType!=ModNone)
         if (AdrMode==3) WrError(1350);
         else   
          BEGIN
           BAsmCode[0]=0x40+AdrMode;
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
           BAsmCode[1+AdrCnt]=RptOrders[z].Code;
           BAsmCode[2+AdrCnt]=4;
           CodeLen=3+AdrCnt;
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
        AdrInt=EvalIntExpression(ArgStr[1],MemInt,&OK)-(EProgCounter()+2);
        if (OK)
         if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
         else
          BEGIN
           CodeLen=2; BAsmCode[0]=RelOrders[z].Code; BAsmCode[1]=AdrInt & 0xff;
          END
       END
      return;
     END

   if ((Memo("SCALL")) OR (Memo("LCALL")) OR (Memo("CALL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],MemInt,&OK);
       if (OK)
	BEGIN
	 AdrInt=AdrWord-(EProgCounter()+2);
	 if (Memo("SCALL")) IsShort=True;
	 else if (Memo("LCALL")) IsShort=False;
	 else IsShort=((AdrInt>=-1024) AND (AdrInt<1023));
	 if (IsShort)
	  BEGIN
	   if ((NOT SymbolQuestionable) AND ((AdrInt<-1024) OR (AdrInt>1023))) WrError(1370);
	   else
	    BEGIN
	     CodeLen=2; BAsmCode[1]=AdrInt & 0xff;
	     BAsmCode[0]=0x28+((AdrInt & 0x700) >> 8);
	    END
	  END
	 else
	  BEGIN
           CodeLen=3; BAsmCode[0]=0xef; AdrInt--;
           BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	   if ((NOT SymbolQuestionable) AND (AdrInt>=-1024) AND (AdrInt<=1023)) WrError(20);
	  END
	END
      END
     return;
    END

   if ((Memo("BR")) OR (Memo("LJMP")) OR (Memo("SJMP")))
    BEGIN
     OpSize=1;
     if (ArgCnt!=1) WrError(1110);
     else if ((Memo("BR")) AND (QuotPos(ArgStr[1],'[')!=Nil))
      BEGIN
       DecodeAdr(ArgStr[1],MModMem,False);
       if (AdrType!=ModNone)
	if ((AdrMode!=2) OR ((AdrVals[0]&1)==1)) WrError(1350);
	else
	 BEGIN
	  CodeLen=2; BAsmCode[0]=0xe3; BAsmCode[1]=AdrVals[0];
	 END
      END
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],MemInt,&OK);
       if (OK)
	BEGIN
	 AdrInt=AdrWord-(EProgCounter()+2);
	 if (Memo("SJMP")) IsShort=True;
	 else if (Memo("LJMP")) IsShort=False;
	 else IsShort=((AdrInt>=-1024) AND (AdrInt<1023));
	 if (IsShort)
	  BEGIN
	   if ((NOT SymbolQuestionable) AND ((AdrInt<-1024) OR (AdrInt>1023))) WrError(1370);
	   else
	    BEGIN
	     CodeLen=2; BAsmCode[1]=AdrInt & 0xff;
	     BAsmCode[0]=0x20+((AdrInt & 0x700) >> 8);
	    END
	  END
	 else
	  BEGIN
           CodeLen=3; BAsmCode[0]=0xe7; AdrInt--;
           BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	   if ((NOT SymbolQuestionable) AND (AdrInt>=-1024) AND (AdrInt<=1023)) WrError(20);
	  END
	END
      END
     return;
    END

   if (Memo("TIJMP"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      BEGIN
       OpSize=1; DecodeAdr(ArgStr[1],MModDir,False);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[3]=AdrVals[0];
         DecodeAdr(ArgStr[2],MModDir,False);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[1]=AdrVals[0];
           OpSize=0; DecodeAdr(ArgStr[3],MModImm,False);
           if (AdrType!=ModNone)
            BEGIN
             BAsmCode[2]=AdrVals[0]; BAsmCode[0]=0xe2; CodeLen=4;
            END
          END
        END
      END
     return;
    END

   if ((Memo("DJNZ")) OR (Memo("DJNZW")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((Memo("DJNZW")) AND (MomCPU<CPU80196)) WrError(1500);
     else
      BEGIN
       OpSize=Ord(Memo("DJNZW"));
       DecodeAdr(ArgStr[1],MModDir,False);
       if (AdrType!=ModNone)
	BEGIN
	 BAsmCode[0]=0xe0+OpSize; BAsmCode[1]=AdrVals[0];
	 AdrInt=EvalIntExpression(ArgStr[2],MemInt,&OK)-(EProgCounter()+3);
	 if (OK)
	  if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
	  else
	   BEGIN
	    CodeLen=3; BAsmCode[2]=AdrInt & 0xff;
	   END
	END
      END
     return;
    END

   if ((Memo("JBC")) OR (Memo("JBS")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       BAsmCode[0]=EvalIntExpression(ArgStr[2],UInt3,&OK);
       if (OK)
	BEGIN
	 BAsmCode[0]+=0x30+(Ord(Memo("JBS")) << 3);
	 OpSize=0; DecodeAdr(ArgStr[1],MModDir,False);
	 if (AdrType!=ModNone)
	  BEGIN
	   BAsmCode[1]=AdrVals[0];
	   AdrInt=EvalIntExpression(ArgStr[3],MemInt,&OK)-(EProgCounter()+3);
	   if (OK)
	    if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
	    else
	     BEGIN
	      CodeLen=3; BAsmCode[2]=AdrInt & 0xff;
	     END
	  END
	END
      END
     return;
    END

   if (Memo("ECALL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80196N) WrError(1500);
     else  
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],MemInt,&OK)-(EProgCounter()+4);
       if (OK)
        BEGIN
         BAsmCode[0]=0xf1;
         BAsmCode[1]=AdrInt & 0xff;
         BAsmCode[2]=(AdrInt >> 8) & 0xff; 
         BAsmCode[3]=(AdrInt >> 16) & 0xff;
         CodeLen=4;
        END
      END
     return;
    END

   if ((Memo("EJMP")) OR (Memo("EBR")))
    BEGIN
     OpSize=1;
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80196N) WrError(1500);
     else if (*ArgStr[1]=='[')
      BEGIN
       DecodeAdr(ArgStr[1],MModMem,False);
       if (AdrType==ModMem)
        if (AdrMode!=2) WrError(1350);
        else if (Odd(AdrVals[0])) WrError(1350);
        else
         BEGIN
          BAsmCode[0]=0xe3; BAsmCode[1]=AdrVals[0]+1;
          CodeLen=2;
         END
      END
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],MemInt,&OK)-(EProgCounter()+4);
       if (OK)
        BEGIN
         BAsmCode[0]=0xe6;
         BAsmCode[1]=AdrInt & 0xff;
         BAsmCode[2]=(AdrInt >> 8) & 0xff;
         BAsmCode[3]=(AdrInt >> 16) & 0xff;
         CodeLen=4;
        END
      END  
     return;
    END    

   WrXError(1200,OpPart);
END

	static void InitCode_96(void)
BEGIN
   SaveInitProc();
   WSRVal=0; CalcWSRWindow();
   WSR1Val=0; CalcWSR1Window();
END

	static Boolean IsDef_96(void)
BEGIN
   return False;
END

        static void SwitchFrom_96(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_96(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x39; NOPCode=0xfd;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
   SegLimits[SegCode] = (MomCPU >= CPU80196N) ? 0xffffffl : 0xffff;

   MakeCode=MakeCode_96; IsDef=IsDef_96;
   SwitchFrom=SwitchFrom_96;

   if (MomCPU>=CPU80196N) MemInt=UInt24;
   else MemInt=UInt16;

   InitFields();
END

	void code96_init(void)
BEGIN
   CPU8096  =AddCPU("8096"  ,SwitchTo_96);
   CPU80196 =AddCPU("80196" ,SwitchTo_96);
   CPU80196N=AddCPU("80196N",SwitchTo_96);
   CPU80296 =AddCPU("80296" ,SwitchTo_96);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_96;
END
