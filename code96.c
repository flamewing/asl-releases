/* code96.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MCS/96-Familie                                              */
/*                                                                           */
/* Historie: 10.11.1996                                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"

typedef struct 
         {
          char *Name;
          Byte Code;
         } FixedOrder;

typedef struct 
         {
          char *Name;
          Byte Code;
          CPUVar MinCPU;
         } BaseOrder;

#define FixedOrderCnt 15

#define ALU3OrderCnt 5

#define ALU2OrderCnt 9

#define ALU1OrderCnt 6

#define ShiftOrderCnt 3

#define RelOrderCnt 16


static char *ShiftOrders[ShiftOrderCnt]={"SHR","SHL","SHRA"};

#define ModNone -1
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

static CPUVar CPU8096,CPU80196;
static void (*SaveInitProc)(void);

static Byte AdrMode;
static ShortInt AdrType;
static Byte AdrVals[3];
static Byte AdrCnt;
static ShortInt OpSize;

static LongInt WSRVal;
static Word WinStart,WinEnd,WinBegin;

/*---------------------------------------------------------------------------*/

static int InstrZ;


   	static void AddFixed(char *NName, Byte NCode, CPUVar NMin)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ++].MinCPU=NMin;
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

   	static void InitFields(void)
BEGIN
   FixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CLRC" ,0xf8,CPU8096 );
   AddFixed("CLRVT",0xfc,CPU8096 );
   AddFixed("DI"   ,0xfa,CPU8096 );
   AddFixed("DPTS" ,0xea,CPU80196);
   AddFixed("EI"   ,0xfb,CPU8096 );
   AddFixed("EPTS" ,0xeb,CPU80196);
   AddFixed("NOP"  ,0xfd,CPU8096 );
   AddFixed("POPA" ,0xf5,CPU80196);
   AddFixed("POPF" ,0xf3,CPU8096 );
   AddFixed("PUSHA",0xf4,CPU80196);
   AddFixed("PUSHF",0xf2,CPU8096 );
   AddFixed("RET"  ,0xf0,CPU8096 );
   AddFixed("RSC"  ,0xff,CPU8096 );
   AddFixed("SETC" ,0xf9,CPU8096 );
   AddFixed("TRAP" ,0xf7,CPU8096 );

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
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(ALU3Orders);
   free(ALU2Orders);
   free(ALU1Orders);
   free(RelOrders);
END

/*-------------------------------------------------------------------------*/

	static void ChkSFR(Word Adr)
BEGIN
   if ((Adr>=SFRStart) & (Adr<=SFRStop)) WrError(190);
END

	static Boolean ChkWork(Word *Adr)
BEGIN
   /* kein Windowing ? */

   if (WinBegin==0) return (*Adr<0xff);

   /* unterhalb Fenster ? */

   else if (*Adr<WinBegin) return True;

   /* in Fenster ? */

   else if ((*Adr>=WinStart) AND (*Adr<=WinEnd))
    BEGIN
     *Adr=*Adr-WinStart+WinBegin; return True;
    END

   /* nix... */

   else return False;
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

	static void DecodeAdr(char *Asc, Byte Mask)
BEGIN
   Integer AdrInt;
   Word AdrWord;
   Boolean OK;
   char *p,*p2;
   int l;
   Byte Reg;

   AdrType=ModNone; AdrCnt=0;

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
       AdrWord=EvalIntExpression(p+1,Int16,&OK);
       if (FirstPassUnknown) AdrWord=0;
       if (OK)
        if (NOT ChkWork(&AdrWord)) WrError(1320);
        else
	 BEGIN
	  Reg=Lo(AdrWord); ChkSFR(Reg);
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
	    AdrInt=EvalIntExpression(Asc,Int16,&OK);
	    if (OK)
	     if (AdrInt==0)
	      BEGIN
	       AdrType=ModMem; AdrMode=2; AdrCnt=1; AdrVals[0]=Reg;
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
     AdrWord=EvalIntExpression(Asc,UInt16,&OK);
     if (OK)
      BEGIN
       if (ChkWork(&AdrWord))
	BEGIN
	 AdrType=ModDir; AdrCnt=1; AdrVals[0]=Lo(AdrWord);
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
       if (((OpSize==1) AND ((AdrWord & 1)!=0))
       OR  ((OpSize==2) AND ((AdrWord & 3)!=0))) WrError(180);
      END
    END

   ChkAdr(Mask);
END

	static void CalcWindow(void)
BEGIN
   if (WSRVal<=0x0f)
    BEGIN
     WinStart=0xffff; WinBegin=0; WinEnd=0;
    END
   else if (WSRVal<=0x1f)
    BEGIN
     WinBegin=0x80;
     if (WSRVal<0x18) WinStart=(WSRVal-0x10) << 7;
     else WinStart=(WSRVal+0x20) << 7;
     WinEnd=WinStart+0x7f;
    END
   else if (WSRVal<=0x3f)
    BEGIN
     WinBegin=0xc0;
     if (WSRVal<0x30) WinStart=(WSRVal-0x20) << 6;
     else WinStart=(WSRVal+0x40) << 6;
     WinEnd=WinStart+0x3f;
    END
   else if (WSRVal<=0x7f)
    BEGIN
     WinBegin=0xe0;
     if (WSRVal<0x60) WinStart=(WSRVal-0x40) << 5;
     else WinStart=(WSRVal+0x80) << 5;
     WinEnd=WinStart+0x1f;
    END
   if (WinEnd>0x1fdf) WinEnd=0x1fdf;
END

	static Boolean DecodePseudo(void)
BEGIN
#define ASSUME96Count 1
   static ASSUMERec ASSUME96s[ASSUME96Count]=
	     {{"WSR", &WSRVal, 0, 0x7f, 0x00}};

   if (Memo("ASSUME"))
    BEGIN
     if (MomCPU<CPU80196) WrError(1500);
     else CodeASSUME(ASSUME96s,ASSUME96Count);
     CalcWindow();
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
   Integer z,AdrInt;
   Byte Start,HReg,Mask;

   CodeLen=0; DontPrint=False; OpSize=-1;

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
        DecodeAdr(ArgStr[ArgCnt],MModImm+MModMem);
        if (AdrType!=ModNone)
         BEGIN
          BAsmCode[Start-1]+=AdrMode;
          memcpy(BAsmCode+Start,AdrVals,AdrCnt); Start+=AdrCnt;
          if ((Special) AND (AdrMode==0)) ChkSFR(AdrVals[0]);
          if (ArgCnt==3)
           BEGIN
            DecodeAdr(ArgStr[2],MModDir);
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
            DecodeAdr(ArgStr[1],MModDir);
            if (AdrType!=ModNone)
             BEGIN
              BAsmCode[Start]=AdrVals[0]; CodeLen=Start+1;
              if (Special)
       	       BEGIN
       	        ChkSFR(AdrVals[0]); ChkAlign(AdrVals[0]);
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
        DecodeAdr(ArgStr[2],Mask);
        if (AdrType!=ModNone)
         BEGIN
          BAsmCode[Start-1]+=AdrMode;
          memcpy(BAsmCode+Start,AdrVals,AdrCnt); Start+=AdrCnt;
          if ((Special) AND (AdrMode==0)) ChkSFR(AdrVals[0]);
          DecodeAdr(ArgStr[1],MModDir);
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
       DecodeAdr(ArgStr[1],MModDir);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[2]=AdrVals[0];
         DecodeAdr(ArgStr[2],MModDir);
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
       DecodeAdr(ArgStr[1],Mask);
       if (AdrType!=ModNone)
	BEGIN
	 CodeLen=1+AdrCnt;
	 BAsmCode[0]=0xc8+AdrMode+(Ord(Memo("POP")) << 2);
	 memcpy(BAsmCode+1,AdrVals,AdrCnt);
	END
      END
     return;
    END

   if ((Memo("BMOV")) OR (Memo("BMOVI")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80196) WrError(1500);
     else
      BEGIN
       OpSize=2; DecodeAdr(ArgStr[1],MModDir);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[2]=AdrVals[0];
         OpSize=1; DecodeAdr(ArgStr[2],MModDir);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[1]=AdrVals[0];
           BAsmCode[0]=(Memo("BMOVI")) ? 0xad : 0xc1;
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
        DecodeAdr(ArgStr[1],MModDir);
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
       DecodeAdr(ArgStr[1],MModMem+MModDir);
       switch (AdrType)
        BEGIN
         case ModMem:
          if (AdrMode==1) WrError(1350);
          else
           BEGIN
            memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=AdrCnt;
            BAsmCode[0]=0x04+((1-OpSize) << 4)+AdrMode;
            DecodeAdr(ArgStr[2],MModDir);
            if (AdrType!=ModNone)
             BEGIN
              BAsmCode[1+HReg]=AdrVals[0]; CodeLen=2+HReg;
             END
           END
          break;
         case ModDir:
          HReg=AdrVals[0];
          DecodeAdr(ArgStr[2],MModMem);
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
       DecodeAdr(ArgStr[2],MModMem+MModImm);
       if (AdrType!=ModNone)
	BEGIN
	 BAsmCode[0]=0xac+(Ord(Memo("LDBSE")) << 4)+AdrMode;
	 memcpy(BAsmCode+1,AdrVals,AdrCnt); Start=1+AdrCnt;
	 OpSize=1; DecodeAdr(ArgStr[1],MModDir);
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
       OpSize=0; DecodeAdr(ArgStr[2],MModDir);
       if (AdrType!=ModNone)
	BEGIN
	 BAsmCode[1]=AdrVals[0];
	 OpSize=1; DecodeAdr(ArgStr[1],MModDir);
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
       OpSize=0; DecodeAdr(ArgStr[1],MModImm);
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
	DecodeAdr(ArgStr[1],MModDir);
	if (AdrType!=ModNone)
	 BEGIN
	  BAsmCode[0]=0x08+z+(Ord(OpSize==0) << 4)+(Ord(OpSize==2) << 2);
	  BAsmCode[2]=AdrVals[0];
	  OpSize=0; DecodeAdr(ArgStr[2],MModDir+MModImm);
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
       OpSize=0; DecodeAdr(ArgStr[1],MModDir);
       if (AdrType!=ModNone)
	BEGIN
	 CodeLen=2; BAsmCode[0]=0; BAsmCode[1]=AdrVals[0];
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
        AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK)-(EProgCounter()+2);
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
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
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
       DecodeAdr(ArgStr[1],MModMem);
       if (AdrType!=ModNone)
	if ((AdrMode!=2) OR ((AdrVals[0]&1)==1)) WrError(1350);
	else
	 BEGIN
	  CodeLen=2; BAsmCode[0]=0xe3; BAsmCode[1]=AdrVals[0];
	 END
      END
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
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
       OpSize=1; DecodeAdr(ArgStr[1],MModDir);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[3]=AdrVals[0];
         DecodeAdr(ArgStr[2],MModDir);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[1]=AdrVals[0];
           OpSize=0; DecodeAdr(ArgStr[3],MModImm);
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
       DecodeAdr(ArgStr[1],MModDir);
       if (AdrType!=ModNone)
	BEGIN
	 BAsmCode[0]=0xe0+OpSize; BAsmCode[1]=AdrVals[0];
	 AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK)-(EProgCounter()+3);
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
	 OpSize=0; DecodeAdr(ArgStr[1],MModDir);
	 if (AdrType!=ModNone)
	  BEGIN
	   BAsmCode[1]=AdrVals[0];
	   AdrInt=EvalIntExpression(ArgStr[3],Int16,&OK)-(EProgCounter()+3);
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

   WrXError(1200,OpPart);
END

	static void InitCode_96(void)
BEGIN
   SaveInitProc();
   WSRVal=0; CalcWindow();
END

	static Boolean ChkPC_96(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode:
      return (ProgCounter()<0x10000);
     default:
      return False;
    END
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

   MakeCode=MakeCode_96; ChkPC=ChkPC_96; IsDef=IsDef_96;
   SwitchFrom=SwitchFrom_96;

   InitFields();
END

	void code96_init(void)
BEGIN
   CPU8096 =AddCPU("8096" ,SwitchTo_96);
   CPU80196=AddCPU("80196",SwitchTo_96);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_96;
END
