/* code47c00.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Toshiba TLCS-47(0(A))                                       */
/*                                                                           */
/* Historie: 30.12.1996 Grundsteinlegung                                     */
/*            3. 1.1999 ChkPC-Anpassung                                      */
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


typedef struct
         {
          char *Name;
          Byte Code;
         } FixedOrder;

#define FixedOrderCnt 3
#define BitOrderCnt 4


#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModL 1
#define MModL (1 << ModL)
#define ModH 2
#define MModH (1 << ModH)
#define ModHL 3
#define MModHL (1 << ModHL)
#define ModIHL 4
#define MModIHL (1 << ModIHL)
#define ModAbs 5
#define MModAbs (1 << ModAbs)
#define ModPort 6
#define MModPort (1 << ModPort)
#define ModImm 7
#define MModImm (1 << ModImm)
#define ModSAbs 8
#define MModSAbs (1 << ModSAbs)

static CPUVar CPU47C00,CPU470C00,CPU470AC00;
static ShortInt AdrType,OpSize;
static Byte AdrVal;
static LongInt DMBAssume;
static SimpProc SaveInitProc;

static FixedOrder *FixedOrders;
static char **BitOrders;

/*---------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("RET" , 0x2a);
   AddFixed("RETI", 0x2b);
   AddFixed("NOP" , 0x00);

   BitOrders=(char **) malloc(sizeof(char *)*BitOrderCnt); InstrZ=0;
   BitOrders[InstrZ++]="SET";
   BitOrders[InstrZ++]="CLR";
   BitOrders[InstrZ++]="TEST";
   BitOrders[InstrZ++]="TESTP";
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(BitOrders);
END

/*---------------------------------------------------------------------------*/

	static Word RAMEnd(void)
BEGIN
   if (MomCPU==CPU47C00) return 0xff;
   else if (MomCPU==CPU470C00) return 0x1ff;
   else return 0x3ff;
END

	static Word ROMEnd(void)
BEGIN
   if (MomCPU==CPU47C00) return 0xfff;
   else if (MomCPU==CPU470C00) return 0x1fff;
   else return 0x3fff;
END

	static Word PortEnd(void)
BEGIN
   if (MomCPU==CPU47C00) return 0x0f;
   else return 0x1f;
END

	static void SetOpSize(ShortInt NewSize)
BEGIN
   if (OpSize==-1) OpSize=NewSize;
   else if (OpSize!=NewSize)
    BEGIN
     WrError(1131); AdrType=ModNone;
    END
END

	static void ChkAdr(Word Mask)
BEGIN
   if ((AdrType!=ModNone) AND (((1 << AdrType) & Mask)==0))
    BEGIN
     WrError(1350); AdrType=ModNone;
    END
END

	static void DecodeAdr(char *Asc, Word Mask)
BEGIN
   static char *RegNames[ModIHL+1]={"A","L","H","HL","@HL"};

   Byte z;
   Word AdrWord;
   Boolean OK;

   AdrType=ModNone;

   for (z=0; z<=ModIHL; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      AdrType=z;
      if (z!=ModIHL) SetOpSize(Ord(z==ModHL));
      ChkAdr(Mask); return;
     END

   if (*Asc=='#')
    BEGIN
     switch (OpSize)
      BEGIN
       case -1:
        WrError(1132); break;
       case 2:
	AdrVal=EvalIntExpression(Asc+1,UInt2,&OK) & 3;
	if (OK) AdrType=ModImm;
        break;
       case 0:
	AdrVal=EvalIntExpression(Asc+1,Int4,&OK) & 15;
	if (OK) AdrType=ModImm;
        break;
       case 1:
	AdrVal=EvalIntExpression(Asc+1,Int8,&OK);
	if (OK) AdrType=ModImm;
        break;
      END
     ChkAdr(Mask); return;
    END

   if (*Asc=='%')
    BEGIN
     AdrVal=EvalIntExpression(Asc+1,Int5,&OK);
     if (OK)
      BEGIN
       AdrType=ModPort; ChkSpace(SegIO);
      END
     ChkAdr(Mask); return;
    END

   FirstPassUnknown=False;
   AdrWord=EvalIntExpression(Asc,Int16,&OK);
   if (OK)
    BEGIN
     ChkSpace(SegData);

     if (FirstPassUnknown) AdrWord&=RAMEnd();
     else if (Hi(AdrWord)!=DMBAssume) WrError(110);

     AdrVal=Lo(AdrWord);
     if (FirstPassUnknown) AdrVal&=15;

     if (((Mask & MModSAbs)!=0) AND (AdrVal<16))
      AdrType=ModSAbs;
     else AdrType=ModAbs;
    END

   ChkAdr(Mask);
END

/*--------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
#define ASSUME47Count 1
static ASSUMERec ASSUME47s[ASSUME47Count]=
	     {{"DMB", &DMBAssume, 0, 3, 4}};

   if (Memo("PORT"))
    BEGIN
     CodeEquate(SegIO,0,0x1f);
     return True;
    END

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME47s,ASSUME47Count);
     return True;
    END

   return False;
END

	static void ChkCPU(Byte Mask)
BEGIN
   Byte NMask=(1 << (MomCPU-CPU47C00));

   /* Don't ask me why, but NetBSD/Sun3 doesn't like writing 
      everything in one formula when using -O3 :-( */

   if ((Mask & NMask)==0)
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

	static Boolean DualOp(char *s1, char *s2)
BEGIN
   return (((strcasecmp(ArgStr[1],s1)==0) AND (strcasecmp(ArgStr[2],s2)==0))
	OR ((strcasecmp(ArgStr[2],s1)==0) AND (strcasecmp(ArgStr[1],s2)==0)));
END

	static void MakeCode_47C00(void)
BEGIN
   Boolean OK;
   Word AdrWord;
   int z;
   Byte HReg;

   CodeLen=0; DontPrint=False; OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodeIntelPseudo(False)) return;

   if (DecodePseudo()) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        CodeLen=1; BAsmCode[0]=FixedOrders[z].Code;
       END
      return;
     END

   /* Datentransfer */

   if (Memo("LD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"DMB")==0)
      BEGIN
       SetOpSize(2); DecodeAdr(ArgStr[2],MModImm+MModIHL);
       switch (AdrType)
        BEGIN
         case ModIHL:
	  CodeLen=3; BAsmCode[0]=0x03; BAsmCode[1]=0x3a; BAsmCode[2]=0xe9;
	  ChkCPU(4);
	  break;
         case ModImm:
	  CodeLen=3; BAsmCode[0]=0x03;
	  BAsmCode[1]=0x2c; BAsmCode[2]=0x09+(AdrVal << 4);
	  ChkCPU(4);
	  break;
        END
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModHL+MModH+MModL);
       switch (AdrType)
        BEGIN
         case ModAcc:
	  DecodeAdr(ArgStr[2],MModIHL+MModAbs+MModImm);
	  switch (AdrType)
           BEGIN
	    case ModIHL:
	     CodeLen=1; BAsmCode[0]=0x0c;
	     break;
	    case ModAbs:
	     CodeLen=2; BAsmCode[0]=0x3c; BAsmCode[1]=AdrVal;
	     break;
	    case ModImm:
	     CodeLen=1; BAsmCode[0]=0x40+AdrVal;
	     break;
	   END
	  break;
         case ModHL:
	  DecodeAdr(ArgStr[2],MModAbs+MModImm);
	  switch (AdrType)
           BEGIN
	    case ModAbs:
	     if ((AdrVal & 3)!=0) WrError(1325);
	     else
	      BEGIN
	       CodeLen=2; BAsmCode[0]=0x28; BAsmCode[1]=AdrVal;
	      END
             break;
	    case ModImm:
	     CodeLen=2;
	     BAsmCode[0]=0xc0+(AdrVal >> 4);
	     BAsmCode[1]=0xe0+(AdrVal & 15);
	     break;
	   END
	  break;
         case ModH:
         case ModL:
          BAsmCode[0]=0xc0+(Ord(AdrType==ModL) << 5);
	  DecodeAdr(ArgStr[2],MModImm);
	  if (AdrType!=ModNone)
	   BEGIN
            CodeLen=1; BAsmCode[0]+=AdrVal;
	   END
	  break;
        END
      END
     return;
    END

   if (Memo("LDL"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"A")!=0) OR (strcasecmp(ArgStr[2],"@DC")!=0)) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x33;
      END
     return;
    END

   if (Memo("LDH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"A")!=0) OR (strcasecmp(ArgStr[2],"@DC+")!=0)) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x32;
      END
     return;
    END

   if (Memo("ST"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"DMB")==0)
      BEGIN
       DecodeAdr(ArgStr[2],MModIHL);
       if (AdrType!=ModNone)
	BEGIN
	 CodeLen=3; BAsmCode[0]=0x03; BAsmCode[1]=0x3a; BAsmCode[2]=0x69;
	 ChkCPU(4);
	END
      END
     else
      BEGIN
       OpSize=0; DecodeAdr(ArgStr[1],MModImm+MModAcc);
       switch (AdrType)
        BEGIN
         case ModAcc:
	  if (strcasecmp(ArgStr[2],"@HL+")==0)
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0x1a;
	   END
	  else if (strcasecmp(ArgStr[2],"@HL-")==0)
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0x1b;
	   END
	  else
	   BEGIN
	    DecodeAdr(ArgStr[2],MModAbs+MModIHL);
	    switch (AdrType)
             BEGIN
	      case ModAbs:
	       CodeLen=2; BAsmCode[0]=0x3f; BAsmCode[1]=AdrVal;
	       break;
	      case ModIHL:
	       CodeLen=1; BAsmCode[0]=0x0f;
	       break;
	     END
	   END
          break;
         case ModImm:
	  HReg=AdrVal;
	  if (strcasecmp(ArgStr[2],"@HL+")==0)
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0xf0+HReg;
	   END
	  else
	   BEGIN
	    DecodeAdr(ArgStr[2],MModSAbs);
	    if (AdrType!=ModNone)
	     if (AdrVal>0x0f) WrError(1320);
	     else
	      BEGIN
	       CodeLen=2; BAsmCode[0]=0x2d;
	       BAsmCode[1]=(HReg << 4)+AdrVal;
	      END
	   END
          break;
	END
      END
     return;
    END

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"A")==0) AND (strcasecmp(ArgStr[2],"DMB")==0))
      BEGIN
       CodeLen=3; BAsmCode[0]=0x03; BAsmCode[1]=0x3a; BAsmCode[2]=0xa9;
       ChkCPU(4);
      END
     else if ((strcasecmp(ArgStr[1],"DMB")==0) AND (strcasecmp(ArgStr[2],"A")==0))
      BEGIN
       CodeLen=3; BAsmCode[0]=0x03; BAsmCode[1]=0x3a; BAsmCode[2]=0x29;
       ChkCPU(4);
      END
     else if ((strcasecmp(ArgStr[1],"A")==0) AND (strcasecmp(ArgStr[2],"SPW13")==0))
      BEGIN
       CodeLen=2; BAsmCode[0]=0x3a; BAsmCode[1]=0x84;
       ChkCPU(4);
      END
     else if ((strcasecmp(ArgStr[1],"STK13")==0) AND (strcasecmp(ArgStr[2],"A")==0))
      BEGIN
       CodeLen=2; BAsmCode[0]=0x3a; BAsmCode[1]=0x04;
       ChkCPU(4);
      END
     else if (strcasecmp(ArgStr[2],"A")!=0) WrError(1350);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModH+MModL);
       if (AdrType!=ModNone)
	BEGIN
	 CodeLen=1; BAsmCode[0]=0x10+Ord(AdrType==ModL);
	END
      END
     return;
    END

   if (Memo("XCH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (DualOp("A","EIR"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x13;
      END
     else if (DualOp("A","@HL"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x0d;
      END
     else if (DualOp("A","H"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x30;
      END
     else if (DualOp("A","L"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x31;
      END
     else
      BEGIN
       if ((strcasecmp(ArgStr[1],"A")!=0) AND (strcasecmp(ArgStr[1],"HL")!=0))
	BEGIN
	 strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
	END
       if ((strcasecmp(ArgStr[1],"A")!=0) AND (strcasecmp(ArgStr[1],"HL")!=0)) WrError(1350);
       else
	BEGIN
	 DecodeAdr(ArgStr[2],MModAbs);
	 if (AdrType!=ModNone)
	  if ((strcasecmp(ArgStr[1],"HL")==0) AND ((AdrVal & 3)!=0)) WrError(1325);
	  else
	   BEGIN
	    CodeLen=2;
	    BAsmCode[0]=0x29+(0x14*Ord(strcasecmp(ArgStr[1],"A")==0));
	    BAsmCode[1]=AdrVal;
	   END
	END
      END
     return;
    END

   if (Memo("IN"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModPort);
       if (AdrType!=ModNone)
	BEGIN
	 HReg=AdrVal;
	 DecodeAdr(ArgStr[2],MModAcc+MModIHL);
	 switch (AdrType)
          BEGIN
	   case ModAcc:
	    CodeLen=2; BAsmCode[0]=0x3a;
	    BAsmCode[1]=(HReg & 0x0f)+(((HReg & 0x10) ^ 0x10) << 1);
	    break;
	   case ModIHL:
	    CodeLen=2; BAsmCode[0]=0x3a;
	    BAsmCode[1]=0x40+(HReg & 0x0f)+(((HReg & 0x10) ^ 0x10) << 1);
	    break;
	  END
	END
      END
     return;
    END

   if (Memo("OUT"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModPort);
       if (AdrType!=ModNone)
	BEGIN
	 HReg=AdrVal; OpSize=0;
	 DecodeAdr(ArgStr[1],MModAcc+MModIHL+MModImm);
	 switch (AdrType)
          BEGIN
	   case ModAcc:
	    CodeLen=2; BAsmCode[0]=0x3a;
	    BAsmCode[1]=0x80+((HReg & 0x10) << 1)+((HReg & 0x0f) ^ 4);
	    break;
	   case ModIHL:
	    CodeLen=2; BAsmCode[0]=0x3a;
	    BAsmCode[1]=0xc0+((HReg & 0x10) << 1)+((HReg & 0x0f) ^ 4);
	    break;
	   case ModImm:
	    if (HReg>0x0f) WrError(1110);
	    else
	     BEGIN
	      CodeLen=2; BAsmCode[0]=0x2c;
	      BAsmCode[1]=(AdrVal << 4)+HReg;
	     END
            break;
	  END
	END
      END
     return;
    END

   if (Memo("OUTB"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"@HL")!=0) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x12;
      END
     return;
    END

   /* Arithmetik */

   if (Memo("CMPR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModSAbs+MModH+MModL);
       switch (AdrType)
        BEGIN
         case ModAcc:
	  DecodeAdr(ArgStr[2],MModIHL+MModAbs+MModImm);
	  switch (AdrType)
           BEGIN
	    case ModIHL:
	     CodeLen=1; BAsmCode[0]=0x16;
	     break;
	    case ModAbs:
	     CodeLen=2; BAsmCode[0]=0x3e; BAsmCode[1]=AdrVal;
	     break;
	    case ModImm:
	     CodeLen=1; BAsmCode[0]=0xd0+AdrVal;
	     break;
	   END
	  break;
         case ModSAbs:
	  OpSize=0; HReg=AdrVal; DecodeAdr(ArgStr[2],MModImm);
	  if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x2e;
	    BAsmCode[1]=(AdrVal << 4)+HReg;
	   END
	  break;
         case ModH:
         case ModL:
	  HReg=AdrType;
	  DecodeAdr(ArgStr[2],MModImm);
	  if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x38;
	    BAsmCode[1]=0x90+(Ord(HReg==ModH) << 6)+AdrVal;
	   END
	  break;
        END
      END
     return;
    END

   if (Memo("ADD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModIHL+MModSAbs+MModL+MModH);
       switch (AdrType)
        BEGIN
         case ModAcc:
	  DecodeAdr(ArgStr[2],MModIHL+MModImm);
	  switch (AdrType)
           BEGIN
	    case ModIHL:
	     CodeLen=1; BAsmCode[0]=0x17;
	     break;
	    case ModImm:
	     CodeLen=2; BAsmCode[0]=0x38; BAsmCode[1]=AdrVal;
	     break;
	   END
	  break;
         case ModIHL:
	  OpSize=0; DecodeAdr(ArgStr[2],MModImm);
	  if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x38; BAsmCode[1]=0x40+AdrVal;
	   END
	  break;
         case ModSAbs:
	  HReg=AdrVal; OpSize=0; DecodeAdr(ArgStr[2],MModImm);
	  if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x2f; BAsmCode[1]=(AdrVal << 4)+HReg;
	   END
	  break;
         case ModH:
         case ModL:
	  HReg=Ord(AdrType==ModH); DecodeAdr(ArgStr[2],MModImm);
	  if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x38; BAsmCode[1]=0x80+(HReg << 6)+AdrVal;
	   END
	  break;
        END
      END
     return;
    END

   if (Memo("ADDC"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"A")!=0) OR (strcasecmp(ArgStr[2],"@HL")!=0)) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x15;
      END
     return;
    END

   if (Memo("SUBRC"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"A")!=0) OR (strcasecmp(ArgStr[2],"@HL")!=0)) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x14;
      END
     return;
    END

   if (Memo("SUBR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OpSize=0; DecodeAdr(ArgStr[2],MModImm);
       if (AdrType!=ModNone)
	BEGIN
	 HReg=AdrVal;
	 DecodeAdr(ArgStr[1],MModAcc+MModIHL);
	 switch (AdrType)
          BEGIN
	   case ModAcc:
	    CodeLen=2; BAsmCode[0]=0x38; BAsmCode[1]=0x10+HReg;
	    break;
	   case ModIHL:
	    CodeLen=2; BAsmCode[0]=0x38; BAsmCode[1]=0x50+HReg;
	    break;
	  END
	END
      END
     return;
    END

   if ((Memo("INC")) OR (Memo("DEC")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       HReg=Ord(Memo("DEC"));
       DecodeAdr(ArgStr[1],MModAcc+MModIHL+MModL);
       switch (AdrType)
        BEGIN
         case ModAcc:
	  CodeLen=1; BAsmCode[0]=0x08+HReg;
	  break;
         case ModL:
	  CodeLen=1; BAsmCode[0]=0x18+HReg;
	  break;
         case ModIHL:
	  CodeLen=1; BAsmCode[0]=0x0a+HReg;
	  break;
        END
      END
     return;
    END

   /* Logik */

   if ((Memo("AND")) OR (Memo("OR")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       HReg=Ord(Memo("OR"));
       DecodeAdr(ArgStr[1],MModAcc+MModIHL);
       switch (AdrType)
        BEGIN
         case ModAcc:
	  DecodeAdr(ArgStr[2],MModImm+MModIHL);
	  switch (AdrType)
           BEGIN
	    case ModIHL:
	     CodeLen=1; BAsmCode[0]=0x1e - HReg; /* ANSI :-0 */
	     break;
	    case ModImm:
	     CodeLen=2; BAsmCode[0]=0x38; BAsmCode[1]=0x30-(HReg << 4)+AdrVal;
	     break;
	   END
	  break;
         case ModIHL:
	  SetOpSize(0); DecodeAdr(ArgStr[2],MModImm);
	  if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x38; BAsmCode[1]=0x70-(HReg << 4)+AdrVal;
	   END
	  break;
        END
      END
     return;
    END

   if (Memo("XOR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"A")!=0) OR (strcasecmp(ArgStr[2],"@HL")!=0)) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x1f;
      END
     return;
    END

   if ((Memo("ROLC")) OR (Memo("RORC")))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
     else
      BEGIN
       if (ArgCnt==1)
	BEGIN
	 HReg=1; OK=True;
	END
       else HReg=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
	BEGIN
	 BAsmCode[0]=0x05+(Ord(Memo("RORC")) << 1);
	 for (z=1; z<HReg; z++) BAsmCode[z]=BAsmCode[0];
	 CodeLen=HReg;
	 if (HReg>=4) WrError(160);
	END
      END
     return;
    END

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z]))
     BEGIN
      if (ArgCnt==1)
       if (strcasecmp(ArgStr[1],"@L")==0)
	BEGIN
	 if (Memo("TESTP")) WrError(1350);
	 else
	  BEGIN
	   if (z==2) z=3; CodeLen=1; BAsmCode[0]=0x34+z;
	  END
	END
       else if (strcasecmp(ArgStr[1],"CF")==0)
	BEGIN
	 if (z<2) WrError(1350);
	 else
	  BEGIN
	   CodeLen=1; BAsmCode[0]=10-2*z;
	  END
	END
       else if (strcasecmp(ArgStr[1],"ZF")==0)
	BEGIN
	 if (z!=3) WrError(1350);
	 else
	  BEGIN
	   CodeLen=1; BAsmCode[0]=0x0e;
	  END
	END
       else if (strcasecmp(ArgStr[1],"GF")==0)
	BEGIN
	 if (z==2) WrError(1350);
	 else
	  BEGIN
	   CodeLen=1; BAsmCode[0]=(z==3) ? 1 : 3-z;
	   ChkCPU(1);
	  END
	END
       else if ((strcasecmp(ArgStr[1],"DMB")==0) OR (strcasecmp(ArgStr[1],"DMB0")==0))
	BEGIN
	 CodeLen=2; BAsmCode[0]=0x3b; BAsmCode[1]=0x39+(z << 6);
	 ChkCPU(1 << (1+Ord(strcasecmp(ArgStr[1],"DMB0")==0)));
	END
       else if (strcasecmp(ArgStr[1],"DMB1")==0)
	BEGIN
	 CodeLen=3; BAsmCode[0]=3; BAsmCode[1]=0x3b;
	 BAsmCode[2]=0x19+(z << 6);
	 ChkCPU(4);
	END
       else if (strcasecmp(ArgStr[1],"STK13")==0)
	BEGIN
	 if (z>1) WrError(1350);
	 else
	  BEGIN
	   CodeLen=3; BAsmCode[0]=3-z; BAsmCode[1]=0x3a; BAsmCode[2]=0x84;
	   ChkCPU(4);
	  END
	END
       else WrError(1350);
      else if (ArgCnt==2)
       if (strcasecmp(ArgStr[1],"IL")==0)
	BEGIN
	 if (z!=1) WrError(1350);
	 else
	  BEGIN
	   HReg=EvalIntExpression(ArgStr[2],UInt6,&OK);
	   if (OK)
	    BEGIN
	     CodeLen=2; BAsmCode[0]=0x36; BAsmCode[1]=0xc0+HReg;
	    END
	  END
	END
       else
	BEGIN
	 HReg=EvalIntExpression(ArgStr[2],UInt2,&OK);
	 if (OK)
	  BEGIN
	   DecodeAdr(ArgStr[1],MModAcc+MModIHL+MModPort+MModSAbs);
	   switch (AdrType)
            BEGIN
	     case ModAcc:
	      if (z!=2) WrError(1350);
	      else
	       BEGIN
	        CodeLen=1; BAsmCode[0]=0x5c+HReg;
	       END
              break;
	     case ModIHL:
	      if (z==3) WrError(1350);
	      else
	       BEGIN
	        CodeLen=1; BAsmCode[0]=0x50+HReg+(z << 2);
	       END
              break;
	     case ModPort:
	      if (AdrVal>15) WrError(1320);
	      else
	       BEGIN
	        CodeLen=2; BAsmCode[0]=0x3b;
	        BAsmCode[1]=(z << 6)+(HReg << 4)+AdrVal;
	       END
              break;
	     case ModSAbs:
	      CodeLen=2; BAsmCode[0]=0x39;
	      BAsmCode[1]=(z << 6)+(HReg << 4)+AdrVal;
	      break;
	    END
	  END
	END
      else WrError(1110);
      return;
     END

   if ((Memo("EICLR")) OR (Memo("DICLR")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"IL")!=0) WrError(1350);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[2],UInt6,&OK);
       if (OK)
	BEGIN
	 CodeLen=2; BAsmCode[0]=0x36;
	 BAsmCode[1]+=0x40*(1+Ord(Memo("DICLR")));
	END
      END
     return;
    END

   /* Spruenge */

   if (Memo("BSS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	if ((NOT SymbolQuestionable) AND ((AdrWord >> 6)!=((EProgCounter()+1) >> 6))) WrError(1910);
	else
	 BEGIN
	  ChkSpace(SegCode);
	  CodeLen=1; BAsmCode[0]=0x80+(AdrWord & 0x3f);
	 END
      END
     return;
    END

   if (Memo("BS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	if (((NOT SymbolQuestionable) AND (AdrWord >> 12)!=((EProgCounter()+2) >> 12))) WrError(1910);
	else
	 BEGIN
	  ChkSpace(SegCode);
	  CodeLen=2; BAsmCode[0]=0x60+(Hi(AdrWord) & 15);
	  BAsmCode[1]=Lo(AdrWord);
	 END
      END
     return;
    END

   if (Memo("BSL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	if (AdrWord>ROMEnd()) WrError(1320);
	else
	 BEGIN
	  ChkSpace(SegCode);
	  CodeLen=3;
	  switch (AdrWord >> 12)
           BEGIN
     	    case 0: BAsmCode[0]=0x02; break;
     	    case 1: BAsmCode[0]=0x03; break;
     	    case 2: BAsmCode[0]=0x1c; break;
            case 3: BAsmCode[0]=0x01; break;
	   END
	  BAsmCode[1]=0x60+(Hi(AdrWord) & 0x0f); BAsmCode[2]=Lo(AdrWord);
	 END
      END
     return;
    END

   if (Memo("B"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	if (AdrWord>ROMEnd()) WrError(1320);
	else
	 BEGIN
	  ChkSpace(SegCode);
	  if ((AdrWord >> 6)==((EProgCounter()+1) >> 6))
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0x80+(AdrWord & 0x3f);
	   END
	  else if ((AdrWord >> 12)==((EProgCounter()+2) >> 12))
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x60+(Hi(AdrWord) & 0x0f);
	    BAsmCode[1]=Lo(AdrWord);
	   END
	  else
	   BEGIN
	    CodeLen=3;
	    switch (AdrWord >> 12)
             BEGIN
	      case 0: BAsmCode[0]=0x02; break;
	      case 1: BAsmCode[0]=0x03; break;
	      case 2: BAsmCode[0]=0x1c; break;
	      case 3: BAsmCode[0]=0x01; break;
	     END
	    BAsmCode[1]=0x60+(Hi(AdrWord) & 0x0f); BAsmCode[2]=Lo(AdrWord);
	   END
	 END
      END
     return;
    END

   if (Memo("CALLS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	BEGIN
	 if (AdrWord==0x86) AdrWord=0x06;
	 if ((AdrWord & 0xff87)!=6) WrError(1135);
	 else
	  BEGIN
	   CodeLen=1; BAsmCode[0]=(AdrWord >> 3)+0x70;
	  END
	END
      END
     return;
    END

   if (Memo("CALL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
	if ((NOT SymbolQuestionable) AND (((AdrWord ^ EProgCounter()) & 0x3800)!=0)) WrError(1910);
	else
	 BEGIN
	  ChkSpace(SegCode);
	  CodeLen=2; BAsmCode[0]=0x20+(Hi(AdrWord) & 7);
	  BAsmCode[1]=Lo(AdrWord);
	 END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static Boolean IsDef_47C00(void)
BEGIN
   return (Memo("PORT"));
END

        static void SwitchFrom_47C00(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_47C00(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=True;

   PCSymbol="$"; HeaderID=0x55; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData)|(1<<SegIO);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = ROMEnd();
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;
   SegLimits[SegData] = RAMEnd();
   Grans[SegIO  ]=1; ListGrans[SegIO  ]=1; SegInits[SegIO  ]=0;
   SegLimits[SegIO  ] = PortEnd();

   MakeCode=MakeCode_47C00; IsDef=IsDef_47C00;
   SwitchFrom=SwitchFrom_47C00; InitFields();
END

	static void InitCode_47C00(void)
BEGIN
   SaveInitProc();

   DMBAssume=0;
END

	void code47c00_init(void)
BEGIN
   CPU47C00=AddCPU("47C00",SwitchTo_47C00);
   CPU470C00=AddCPU("470C00",SwitchTo_47C00);
   CPU470AC00=AddCPU("470AC00",SwitchTo_47C00);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_47C00;
END
