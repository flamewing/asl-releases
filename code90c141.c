/* code90c141.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Toshiba TLCS-90                                             */
/*                                                                           */
/* Historie: 30.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "insttree.h"
#include "codepseudo.h"


typedef struct
         { 
          Byte Code;
         } FixedOrder;

typedef struct
         { 
          Byte Code;
          Boolean MayReg;
         } ShiftOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;


#define AccReg 6
#define HLReg 2

#define FixedOrderCnt 18
#define MoveOrderCnt 8
#define ShiftOrderCnt 10
#define BitOrderCnt 4
#define AccOrderCnt 3
#define ALU2OrderCnt 8
#define ConditionCnt 24

#define ModNone -1
#define ModReg8    0
#define MModReg8   (1 << ModReg8)
#define ModReg16   1
#define MModReg16  (1 << ModReg16)
#define ModIReg16  2
#define MModIReg16 (1 << ModIReg16)
#define ModIndReg  3
#define MModIndReg (1 << ModIndReg)
#define ModIdxReg  4
#define MModIdxReg (1 << ModIdxReg)
#define ModDir     5
#define MModDir    (1 << ModDir)
#define ModMem     6
#define MModMem    (1 << ModMem)
#define ModImm     7
#define MModImm    (1 << ModImm)

static int DefaultCondition;

static ShortInt AdrType;
static Byte AdrMode;
static Byte AdrCnt;
static ShortInt OpSize;
static Byte AdrVals[10];
static Boolean MinOneIs0;

static FixedOrder *FixedOrders;
static FixedOrder *MoveOrders;
static ShiftOrder *ShiftOrders;
static FixedOrder *BitOrders;
static FixedOrder *AccOrders;
static char **ALU2Orders;
static Condition *Conditions;
static PInstTreeNode ITree;

static CPUVar CPU90C141;

/*---------------------------------------------------------------------------*/

	static void ChkAdr(Byte Erl)
BEGIN
   if (AdrType!=ModNone)
    if (((1 << AdrType) & Erl)==0)
     BEGIN
      WrError(1350); AdrType=ModNone; AdrCnt=0;
     END
END

	static void SetOpSize(ShortInt New)
BEGIN
   if (OpSize==-1) OpSize=New;
   else if (OpSize!=New)
    BEGIN
     WrError(1131); AdrType=ModNone; AdrCnt=0;
    END
END

	static void DecodeAdr(char *Asc, Byte Erl)
BEGIN
#define Reg8Cnt 7
   static char *Reg8Names[Reg8Cnt]={"B","C","D","E","H","L","A"};
#define Reg16Cnt 7
   static char *Reg16Names[Reg16Cnt]={"BC","DE","HL","\0","IX","IY","SP"};
#define IReg16Cnt 3
   static char *IReg16Names[IReg16Cnt]={"IX","IY","SP"};

   Integer z;
   char *p,*ppos,*mpos;
   LongInt DispAcc,DispVal;
   Byte OccFlag,BaseReg;
   Boolean ok,fnd,NegFlag,NNegFlag,Unknown;
   String Part;

   AdrType=ModNone; AdrCnt=0;

   /* 1. 8-Bit-Register */

   for (z=0; z<Reg8Cnt; z++)
    if (strcasecmp(Asc,Reg8Names[z])==0)
     BEGIN
      AdrType=ModReg8; AdrMode=z; SetOpSize(0);
      ChkAdr(Erl); return;
     END

   /* 2. 16-Bit-Register, indiziert */

   if ((Erl & MModIReg16)!=0)
    for (z=0; z<IReg16Cnt; z++)
     if (strcasecmp(Asc,IReg16Names[z])==0)
      BEGIN
       AdrType=ModIReg16; AdrMode=z; SetOpSize(1);
       ChkAdr(Erl); return;
      END

   /* 3. 16-Bit-Register, normal */

   for (z=0; z<Reg16Cnt; z++)
    if (strcasecmp(Asc,Reg16Names[z])==0)
     BEGIN
      AdrType=ModReg16; AdrMode=z; SetOpSize(1);
      ChkAdr(Erl); return;
     END

   /* Speicheradresse */

   if (IsIndirect(Asc))
    BEGIN
     OccFlag=0; BaseReg=0; DispAcc=0; ok=True; NegFlag=False; Unknown=False;
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
     do
      BEGIN
       ppos=QuotPos(Asc,'+');
       mpos=QuotPos(Asc,'-');
       if (ppos==Nil) p=mpos; 
       else if (mpos==Nil) p=ppos;
       else p=min(mpos,ppos);
       NNegFlag=((p!=Nil) AND (*p=='-')); 
       if (p==Nil)
        BEGIN
         strmaxcpy(Part,Asc,255); *Asc='\0';
        END
       else
        BEGIN
         *p='\0'; strmaxcpy(Part,Asc,255); strcpy(Asc,p+1);
        END
       fnd=False;
       if (strcasecmp(Part,"A")==0)
        BEGIN
 	 fnd=True;
	 ok=((NOT NegFlag) AND ((OccFlag & 1)==0));
	 if (ok) OccFlag+=1; else WrError(1350);
        END
       if (NOT fnd)
        for (z=0; z<Reg16Cnt; z++)
	 if (strcasecmp(Part,Reg16Names[z])==0)
	  BEGIN
	   fnd=True; BaseReg=z;
	   ok=((NOT NegFlag) AND ((OccFlag & 2)==0));
	   if (ok) OccFlag+=2; else WrError(1350);
	  END
       if (NOT fnd)
        BEGIN
	 FirstPassUnknown=False;
	 DispVal=EvalIntExpression(Part,Int32,&ok);
	 if (ok)
	  BEGIN
	   if (NegFlag) DispAcc-=DispVal; else DispAcc+=DispVal;
	   if (FirstPassUnknown) Unknown=True;
	  END
        END
       NegFlag=NNegFlag;
      END
     while ((*Asc!='\0') AND (ok));
     if (NOT ok) return;
     if (Unknown) DispAcc&=0x7f;
     switch (OccFlag)
      BEGIN
       case 1:
        WrError(1350); break;
       case 3:
        if ((BaseReg!=2) OR (DispAcc!=0)) WrError(1350);
        else
 	 BEGIN
	  AdrType=ModIdxReg; AdrMode=3;
	 END
        break;
       case 2:
        if ((DispAcc>127) OR (DispAcc<-128)) WrError(1320);
        else if (DispAcc==0)
	 BEGIN
	  AdrType=ModIndReg; AdrMode=BaseReg;
 	 END
        else if (BaseReg<4) WrError(1350);
        else
 	 BEGIN
	  AdrType=ModIdxReg; AdrMode=BaseReg-4;
	  AdrCnt=1; AdrVals[0]=DispAcc & 0xff;
	 END
        break;
       case 0:
        if (DispAcc>0xffff) WrError(1925);
        else if ((Hi(DispAcc)==0xff) AND ((Erl & MModDir)!=0))
	 BEGIN
	  AdrType=ModDir; AdrCnt=1; AdrVals[0]=Lo(DispAcc);
	 END
        else
	 BEGIN
	  AdrType=ModMem; AdrCnt=2;
          AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
	 END
        break;
      END
    END

   /* immediate */

   else
    BEGIN
     if ((OpSize==-1) AND (MinOneIs0)) OpSize=0;
     switch (OpSize)
      BEGIN
       case -1:
        WrError(1130); break;
       case 0:
	AdrVals[0]=EvalIntExpression(Asc,Int8,&ok);
	if (ok)
	 BEGIN
	  AdrType=ModImm; AdrCnt=1;
	 END
        break;
       case 1:
	DispVal=EvalIntExpression(Asc,Int16,&ok);
	if (ok)
	 BEGIN
	  AdrType=ModImm; AdrCnt=2;
	  AdrVals[0]=Lo(DispVal); AdrVals[1]=Hi(DispVal);
	 END
        break;
      END
    END

   /* gefunden */

   ChkAdr(Erl);
END

/*--------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   return False;
END

	static Boolean WMemo(char *Name)
BEGIN
   String tmp;

   if (Memo(Name)) return True;

   sprintf(tmp,"%sW",Name);
   if (Memo(tmp))
    BEGIN
     OpSize=1; return True;
    END
   else return False;
END

	static Boolean ArgPair(char *Arg1, char *Arg2)
BEGIN
   return  (((strcasecmp(ArgStr[1],Arg1)==0) AND (strcasecmp(ArgStr[2],Arg2)==0))
	 OR ((strcasecmp(ArgStr[1],Arg2)==0) AND (strcasecmp(ArgStr[2],Arg1)==0)));
END

/*-------------------------------------------------------------------------*/

/* ohne Argument */

	static void CodeFixed(Word Index)
BEGIN
   if (ArgCnt!=0) WrError(1110);
   else
    BEGIN
     CodeLen=1; BAsmCode[0]=FixedOrders[Index].Code;
    END
END

        static void CodeMove(Word Index)
BEGIN
   if (ArgCnt!=0) WrError(1110);
   else
    BEGIN
     CodeLen=2; BAsmCode[0]=0xfe; BAsmCode[1]=MoveOrders[Index].Code;
    END
END

        static void CodeShift(Word Index)
BEGIN
   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],((ShiftOrders[Index].MayReg)?MModReg8:0)+MModIndReg+MModIdxReg+MModMem+MModDir);
     switch (AdrType)
      BEGIN
       case ModReg8:
        CodeLen=2; BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=ShiftOrders[Index].Code;
        if (AdrMode==AccReg) WrError(10);
        break;
       case ModIndReg:
        CodeLen=2; BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=ShiftOrders[Index].Code;
        break;
       case ModIdxReg:
        CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
        memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=ShiftOrders[Index].Code;
        break;
       case ModDir:
        CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0];
        BAsmCode[2]=ShiftOrders[Index].Code;
        break;
       case ModMem:
        CodeLen=4; BAsmCode[0]=0xe3;
        memcpy(BAsmCode+1,AdrVals,AdrCnt);
        BAsmCode[3]=ShiftOrders[Index].Code;
        break;
      END
    END
END

/* Logik */

        static void CodeBit(Word Index)
BEGIN
   Byte HReg;
   Boolean OK;

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     HReg=EvalIntExpression(ArgStr[1],UInt3,&OK);
     if (OK)
      BEGIN
       DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModMem+MModDir);
       switch (AdrType)
        BEGIN
         case ModReg8:
          CodeLen=2;
          BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=BitOrders[Index].Code+HReg;
          break;
         case ModIndReg:
          CodeLen=2;
          BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=BitOrders[Index].Code+HReg;
          break;
         case ModIdxReg:
          CodeLen=2+AdrCnt; memcpy(BAsmCode+1,AdrVals,AdrCnt);
          BAsmCode[0]=0xf0+AdrMode; BAsmCode[1+AdrCnt]=BitOrders[Index].Code+HReg;
          break;
         case ModMem:
          CodeLen=4; memcpy(BAsmCode+1,AdrVals,AdrCnt);
          BAsmCode[0]=0xe3; BAsmCode[1+AdrCnt]=BitOrders[Index].Code+HReg;
          break;
         case ModDir:
          BAsmCode[1]=AdrVals[0];
          if (Index==4)
           BEGIN
            BAsmCode[0]=0xe7; BAsmCode[2]=BitOrders[Index].Code+HReg; CodeLen=3;
           END
          else
           BEGIN
            BAsmCode[0]=BitOrders[Index].Code+HReg; CodeLen=2;
           END
          break;
        END
      END
    END
END

        static void CodeAcc(Word Index)
BEGIN
   if (ArgCnt!=1) WrError(1110);
   else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
   else
    BEGIN
     CodeLen=1; BAsmCode[0]=AccOrders[Index].Code;
    END
END

        static void MakeCode_90C141(void)
BEGIN
   Integer z;
   Integer AdrInt;
   Boolean OK;
   Byte HReg;

   CodeLen=0; DontPrint=False; OpSize=-1;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   if (SearchInstTree(ITree)) return;

   /* Datentransfer */

   if (WMemo("LD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem);
       switch (AdrType)
        BEGIN
         case ModReg8:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	  switch (AdrType)
           BEGIN
	    case ModReg8:
	     if (HReg==AccReg)
	      BEGIN
	       CodeLen=1; BAsmCode[0]=0x20+AdrMode;
	      END
	     else if (AdrMode==AccReg)
	      BEGIN
	       CodeLen=1; BAsmCode[0]=0x28+HReg;
	      END
	     else
	      BEGIN
	       CodeLen=2; BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=0x30+HReg;
	      END
             break;
	    case ModIndReg:
	     CodeLen=2; BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=0x28+HReg;
	     break;
	    case ModIdxReg:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x28+HReg;
	     break;
	    case ModDir:
	     if (HReg==AccReg)
	      BEGIN
	       CodeLen=2; BAsmCode[0]=0x27; BAsmCode[1]=AdrVals[0];
	      END
	     else
	      BEGIN
	       CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0];
	       BAsmCode[2]=0x28+HReg;
	      END
             break;
	    case ModMem:
	     CodeLen=4; BAsmCode[0]=0xe3;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[3]=0x28+HReg;
	     break;
	    case ModImm:
	     CodeLen=2; BAsmCode[0]=0x30+HReg; BAsmCode[1]=AdrVals[0];
	     break;
	   END
	  break;
         case ModReg16:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	  switch (AdrType)
           BEGIN
	    case ModReg16:
	     if (HReg==HLReg)
	      BEGIN
	       CodeLen=1; BAsmCode[0]=0x40+AdrMode;
	      END
	     else if (AdrMode==HLReg)
	      BEGIN
	       CodeLen=1; BAsmCode[0]=0x48+HReg;
	      END
	     else
	      BEGIN
	       CodeLen=2; BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=0x38+HReg;
	      END
             break;
	    case ModIndReg:
	     CodeLen=2;
	     BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=0x48+HReg;
	     break;
	    case ModIdxReg:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x48+HReg;
	     break;
	    case ModDir:
	     if (HReg==HLReg)
	      BEGIN
	       CodeLen=2; BAsmCode[0]=0x47; BAsmCode[1]=AdrVals[0];
	      END
	     else
	      BEGIN
	       CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0];
	       BAsmCode[2]=0x48+HReg;
	      END
             break;
	    case ModMem:
	     CodeLen=4; BAsmCode[0]=0xe3; BAsmCode[3]=0x48+HReg;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     break;
            case ModImm:
             CodeLen=3; BAsmCode[0]=0x38+HReg;
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
             break;
	   END
	  break;
         case ModIndReg:
         case ModIdxReg:
         case ModDir:
         case ModMem:
	  MinOneIs0=True; HReg=AdrCnt; memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  switch (AdrType)
           BEGIN
	    case ModIndReg: BAsmCode[0]=0xe8+AdrMode; break;
	    case ModIdxReg: BAsmCode[0]=0xf4+AdrMode; break;
	    case ModMem:    BAsmCode[0]=0xeb; break;
	    case ModDir:    BAsmCode[0]=0x0f; break;
	   END
	  DecodeAdr(ArgStr[2],MModReg16+MModReg8+MModImm);
	  if (BAsmCode[0]==0x0f)
	   switch (AdrType)
            BEGIN
	     case ModReg8:
	      if (AdrMode==AccReg)
	       BEGIN
	        CodeLen=2; BAsmCode[0]=0x2f;
	       END
	      else
	       BEGIN
	        CodeLen=3; BAsmCode[0]=0xef; BAsmCode[2]=0x20+AdrMode;
	       END
              break;
	     case ModReg16:
	      if (AdrMode==HLReg)
	       BEGIN
	        CodeLen=2; BAsmCode[0]=0x4f;
	       END
	      else
	       BEGIN
	        CodeLen=3; BAsmCode[0]=0xef; BAsmCode[2]=0x40+AdrMode;
	       END
              break;
	     case ModImm:
	      CodeLen=3+OpSize; BAsmCode[0]=0x37+(OpSize << 3);
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	      break;
	    END
	   else
	    BEGIN
	     switch (AdrType)
              BEGIN
	       case ModReg8:  BAsmCode[1+HReg]=0x20+AdrMode; break;
	       case ModReg16: BAsmCode[1+HReg]=0x40+AdrMode; break;
	       case ModImm:   BAsmCode[1+HReg]=0x37+(OpSize << 3); break;
	      END
	     memcpy(BAsmCode+2+HReg,AdrVals,AdrCnt);
	     CodeLen=1+HReg+1+AdrCnt;
	    END
	  break;
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       HReg=Ord(Memo("POP")) << 3;
       if (strcasecmp(ArgStr[1],"AF")==0)
	BEGIN
	 CodeLen=1; BAsmCode[0]=0x56+HReg;
	END
       else
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg16);
	 if (AdrType==ModReg16)
	  if (AdrMode==6) WrError(1350);
	  else
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0x50+HReg+AdrMode;
	   END
	END
      END
     return;
    END

   if (Memo("LDA"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       if (AdrType==ModReg16)
	BEGIN
	 HReg=0x38+AdrMode;
	 DecodeAdr(ArgStr[2],MModIndReg+MModIdxReg);
	 switch (AdrType)
          BEGIN
	   case ModIndReg:
	    if (AdrMode<4) WrError(1350);
	    else
	     BEGIN
	      CodeLen=3; BAsmCode[0]=0xf0+AdrMode;
	      BAsmCode[1]=0; BAsmCode[2]=HReg;
	     END
            break;
	   case ModIdxReg:
	    CodeLen=2+AdrCnt; BAsmCode[0]=0xf4+AdrMode;
	    memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=HReg;
	    break;
	  END
	END
      END
     return;
    END

   if (Memo("LDAR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"HL")!=0) WrError(1350);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK)-(EProgCounter()+2);
       if (OK)
	BEGIN
	 CodeLen=3; BAsmCode[0]=0x17;
	 BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	END
      END
     return;
    END

   if (Memo("EX"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (ArgPair("DE","HL"))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x08;
      END
     else if ((ArgPair("AF","AF\'") OR ArgPair("AF","AF`")))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x09;
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16+MModIndReg+MModIdxReg+MModMem+MModDir);
       switch (AdrType)
        BEGIN
         case ModReg16:
	  HReg=0x50+AdrMode;
	  DecodeAdr(ArgStr[2],MModIndReg+MModIdxReg+MModMem+MModDir);
	  switch (AdrType)
           BEGIN
	    case ModIndReg:
	     CodeLen=2; BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=HReg;
	     break;
	    case ModIdxReg:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
	     memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     BAsmCode[1+AdrCnt]=HReg;
	     break;
	    case ModDir:
	     CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0];
	     BAsmCode[2]=HReg;
	     break;
	    case ModMem:
	     CodeLen=4; BAsmCode[0]=0xe3; memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     BAsmCode[3]=HReg;
	     break;
 	   END
	  break;
         case ModIndReg:
         case ModIdxReg:
         case ModDir:
         case ModMem:
 	  switch (AdrType)
           BEGIN
 	    case ModIndReg:  BAsmCode[0]=0xe0+AdrMode; break;
	    case ModIdxReg:  BAsmCode[0]=0xf0+AdrMode; break;
	    case ModDir:     BAsmCode[0]=0xe7; break;
	    case ModMem:     BAsmCode[0]=0xe3; break;
	   END
	  memcpy(BAsmCode+1,AdrVals,AdrCnt); HReg=2+AdrCnt;
	  DecodeAdr(ArgStr[2],MModReg16);
	  if (AdrType==ModReg16)
	   BEGIN
	    BAsmCode[HReg-1]=0x50+AdrMode; CodeLen=HReg;
	   END
	  break;
        END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<ALU2OrderCnt; z++)
    if (Memo(ALU2Orders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIdxReg+MModIndReg+MModDir+MModMem);
	switch (AdrType)
         BEGIN
	  case ModReg8:
           DecodeAdr(ArgStr[2],MModImm+(((HReg=AdrMode)==AccReg)?MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem:0));
	   switch(AdrType)
            BEGIN
	     case ModReg8:
	      CodeLen=2;
	      BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=0x60+z;
	      break;
	     case ModIndReg:
	      CodeLen=2;
	      BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=0x60+z;
	      break;
	     case ModIdxReg:
	      CodeLen=2+AdrCnt;
	      BAsmCode[0]=0xf0+AdrMode;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      BAsmCode[1+AdrCnt]=0x60+z;
	      break;
	     case ModDir:
	      CodeLen=2;
	      BAsmCode[0]=0x60+z;
	      BAsmCode[1]=AdrVals[0];
	      break;
	     case ModMem:
	      CodeLen=4;
	      BAsmCode[0]=0xe3; BAsmCode[3]=0x60+z;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      break;
	     case ModImm:
	      if (HReg==AccReg)
	       BEGIN
	        CodeLen=2;
	        BAsmCode[0]=0x68+z; BAsmCode[1]=AdrVals[0];
	       END
	      else
	       BEGIN
	        CodeLen=3;
	        BAsmCode[0]=0xf8+HReg; BAsmCode[1]=0x68+z;
	        BAsmCode[2]=AdrVals[0];
	       END
	      break;
	    END
           break;
	  case ModReg16:
	   if ((AdrMode==2) OR ((z==0) AND (AdrMode>=4)))
	    BEGIN
	     HReg=AdrMode;
	     DecodeAdr(ArgStr[2],MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	     switch (AdrType)
              BEGIN
	       case ModReg16:
	        CodeLen=2;
	        BAsmCode[0]=0xf8+AdrMode;
	        BAsmCode[1]=(HReg>=4) ? 0x14+HReg-4 : 0x70+z;
	        break;
	       case ModIndReg:
	        CodeLen=2;
	        BAsmCode[0]=0xe0+AdrMode;
	        BAsmCode[1]=(HReg>=4) ? 0x14+HReg-4 : 0x70+z;
	        break;
	       case ModIdxReg:
	        CodeLen=2+AdrCnt;
	        BAsmCode[0]=0xf0+AdrMode;
	        memcpy(BAsmCode+1,AdrVals,AdrCnt);
                BAsmCode[1+AdrCnt]=(HReg>=4) ? 0x14+HReg-4 : 0x70+z;
	        break;
	       case ModDir:
	        if (HReg>=4)
	         BEGIN
	          CodeLen=3;
	          BAsmCode[0]=0xe7;
	          BAsmCode[1]=AdrVals[0];
	          BAsmCode[2]=0x10+HReg;
	         END
	        else
	         BEGIN
	          CodeLen=2;
	          BAsmCode[0]=0x70+z; BAsmCode[1]=AdrVals[0];
	         END
                break;
	       case ModMem:
	        CodeLen=4;
	        BAsmCode[0]=0xe3;
	        memcpy(BAsmCode+1,AdrVals,2);
                BAsmCode[3]=(HReg>=4) ? 0x14+HReg-4 : 0x70+z;
	        break;
	       case ModImm:
	        CodeLen=3;
                BAsmCode[0]=(HReg>=4) ? 0x14+HReg-4 : 0x78+z;
	        memcpy(BAsmCode+1,AdrVals,AdrCnt);
	        break;
	      END
	    END
	   else WrError(1350);
           break;
	  case ModIndReg:
          case ModIdxReg:
          case ModDir:
          case ModMem:
	   OpSize=0;
	   switch (AdrType)
            BEGIN
	     case ModIndReg:
	      HReg=3;
	      BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0x68+z;
	      break;
	     case ModIdxReg:
	      HReg=3+AdrCnt;
	      BAsmCode[0]=0xf4+AdrMode; BAsmCode[1+AdrCnt]=0x68+z;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      break;
	     case ModDir:
	      HReg=4;
	      BAsmCode[0]=0xef; BAsmCode[1]=AdrVals[0]; BAsmCode[2]=0x68+z;
	      break;
	     case ModMem:
	      HReg=5;
	      BAsmCode[0]=0xeb; memcpy(BAsmCode+1,AdrVals,2); BAsmCode[3]=0x68+z;
	      break;
             default:
              HReg=0;
	    END
	   DecodeAdr(ArgStr[2],MModImm);
	   if (AdrType==ModImm)
	    BEGIN
	     BAsmCode[HReg-1]=AdrVals[0]; CodeLen=HReg;
	    END
	   break;
	 END
       END
      return;
     END

   if ((WMemo("INC")) OR (WMemo("DEC")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       HReg=Ord(WMemo("DEC")) << 3;
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem);
       if (OpSize==-1) OpSize=0;
       switch (AdrType)
        BEGIN
         case ModReg8:
	  CodeLen=1; BAsmCode[0]=0x80+HReg+AdrMode;
	  break;
         case ModReg16:
	  CodeLen=1; BAsmCode[0]=0x90+HReg+AdrMode;
	  break;
         case ModIndReg:
	  CodeLen=2; BAsmCode[0]=0xe0+AdrMode;
	  BAsmCode[1]=0x87+(OpSize << 4)+HReg;
	  break;
         case ModIdxReg:
	  CodeLen=2+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  BAsmCode[1+AdrCnt]=0x87+(OpSize << 4)+HReg;
	  break;
         case ModDir:
	  CodeLen=2; BAsmCode[0]=0x87+(OpSize << 4)+HReg;
	  BAsmCode[1]=AdrVals[0];
	  break;
         case ModMem:
	  CodeLen=4; BAsmCode[0]=0xe3;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  BAsmCode[3]=0x87+(OpSize << 4)+HReg;
	  BAsmCode[1]=AdrVals[0];
	  break;
        END
      END
     return;
    END

   if ((Memo("INCX")) OR (Memo("DECX")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModDir);
       if (AdrType==ModDir)
	BEGIN
	 CodeLen=2;
	 BAsmCode[0]=0x07+(Ord(Memo("DECX")) << 3);
	 BAsmCode[1]=AdrVals[0];
	END
      END
     return;
    END

   if ((Memo("MUL")) OR (Memo("DIV")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"HL")!=0) WrError(1350);
     else
      BEGIN
       HReg=0x12+Ord(Memo("DIV")); OpSize=0;
       DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
       switch (AdrType)
        BEGIN
         case ModReg8:
	  CodeLen=2;
	  BAsmCode[0]=0xf8+AdrMode; BAsmCode[1]=HReg;
	  break;
         case ModIndReg:
	  CodeLen=2;
	  BAsmCode[0]=0xe0+AdrMode; BAsmCode[1]=HReg;
	  break;
         case ModIdxReg:
	  CodeLen=2+AdrCnt;
	  BAsmCode[0]=0xf0+AdrMode; BAsmCode[1+AdrCnt]=HReg;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  break;
         case ModDir:
	  CodeLen=3; BAsmCode[0]=0xe7;
	  BAsmCode[1]=AdrVals[0]; BAsmCode[2]=HReg;
	  break;
         case ModMem:
	  CodeLen=4; BAsmCode[0]=0xe3; BAsmCode[3]=HReg;
	  memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  break;
         case ModImm:
          CodeLen=3; BAsmCode[0]=0xe7; BAsmCode[1]=AdrVals[0]; BAsmCode[2]=HReg;
          break;
        END
      END
     return;
    END

   /* Spruenge */

   if (Memo("JR"))
    BEGIN
     if ((ArgCnt==0) OR (ArgCnt>2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) z=DefaultCondition;
       else
        BEGIN
         NLS_UpString(ArgStr[1]);
         for (z=0; z<ConditionCnt; z++)
          if (strcmp(ArgStr[1],Conditions[z].Name)==0) break;
        END
       if (z>=ConditionCnt) WrError(1360);
       else
	BEGIN
	 AdrInt=EvalIntExpression(ArgStr[ArgCnt],Int16,&OK)-(EProgCounter()+2);
	 if (OK)
	  if ((NOT SymbolQuestionable) AND ((AdrInt>127) OR (AdrInt<-128))) WrError(1370);
	  else
	   BEGIN
	    CodeLen=2;
	    BAsmCode[0]=0xc0+Conditions[z].Code;
	    BAsmCode[1]=AdrInt & 0xff;
	   END
	END
      END
     return;
    END

   if ((Memo("CALL")) OR (Memo("JP")))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) z=DefaultCondition;
       else
        BEGIN
         NLS_UpString(ArgStr[1]);
         for (z=0; z<ConditionCnt; z++)
          if (strcmp(Conditions[z].Name,ArgStr[1])==0) break;
        END
       if (z>=ConditionCnt) WrError(1360);
       else
	BEGIN
	 OpSize=1; HReg=Ord(Memo("CALL"));
	 DecodeAdr(ArgStr[ArgCnt],MModIndReg+MModIdxReg+MModMem+MModImm);
	 if (AdrType==ModImm) AdrType=ModMem;
	 switch (AdrType)
          BEGIN
	   case ModIndReg:
	    CodeLen=2;
	    BAsmCode[0]=0xe8+AdrMode;
	    BAsmCode[1]=0xc0+(HReg << 4)+Conditions[z].Code;
	    break;
	   case ModIdxReg:
	    CodeLen=2+AdrCnt;
	    BAsmCode[0]=0xf4+AdrMode;
	    memcpy(BAsmCode+1,AdrVals,AdrCnt);
	    BAsmCode[1+AdrCnt]=0xc0+(HReg << 4)+Conditions[z].Code;
	    break;
	   case ModMem:
	    if (z==DefaultCondition)
	     BEGIN
	      CodeLen=3;
	      BAsmCode[0]=0x1a+(HReg << 1);
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	     END
	    else
	     BEGIN
	      CodeLen=4;
	      BAsmCode[0]=0xeb;
	      memcpy(BAsmCode+1,AdrVals,AdrCnt);
	      BAsmCode[3]=0xc0+(HReg << 4)+Conditions[z].Code;
	     END
	    break;
	  END
        END
      END
     return;
    END

   if (Memo("RET"))
    BEGIN
     if ((ArgCnt!=0) AND (ArgCnt!=1)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==0) z=DefaultCondition;
       else
        BEGIN
         NLS_UpString(ArgStr[1]);
         for (z=0; z<ConditionCnt; z++)
          if (strcmp(ArgStr[1],Conditions[z].Name)==0) break;
        END
       if (z>=ConditionCnt) WrError(1360);
       if (z==DefaultCondition)
	BEGIN
	 CodeLen=1; BAsmCode[0]=0x1e;
	END
       else if (z<ConditionCnt)
	BEGIN
	 CodeLen=2; BAsmCode[0]=0xfe;
	 BAsmCode[1]=0xd0+Conditions[z].Code;
	END
      END
     return;
    END

   if (Memo("DJNZ"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1)
	BEGIN
	 AdrType=ModReg8; AdrMode=0; OpSize=0;
	END
       else DecodeAdr(ArgStr[1],MModReg8+MModReg16);
       if (AdrType!=ModNone)
	if (AdrMode!=0) WrError(1350);
	else
	 BEGIN
	  AdrInt=EvalIntExpression(ArgStr[ArgCnt],Int16,&OK)-(EProgCounter()+2);
	  if (OK)
	   if ((NOT SymbolQuestionable) AND ((AdrInt>127) OR (AdrInt<-128))) WrError(1370);
	   else
	    BEGIN
	     CodeLen=2;
	     BAsmCode[0]=0x18+OpSize;
	     BAsmCode[1]=AdrInt & 0xff;
	    END
	 END
      END
     return;
    END

   if ((Memo("JRL")) OR (Memo("CALR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK)-(EProgCounter()+2);
       if (OK)
	BEGIN
	 CodeLen=3;
	 if (Memo("JRL"))
	  BEGIN
	   BAsmCode[0]=0x1b;
	   if ((AdrInt>=-128) AND (AdrInt<=127)) WrError(20);
	  END
	 else BAsmCode[0]=0x1d;
	 BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
	END
      END
     return;
    END

   WrXError(1200,OpPart);
END

/*-------------------------------------------------------------------------*/

static int InstrZ;


	static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Code=NCode;
   AddInstTree(&ITree,NName,CodeFixed,InstrZ++);
END

        static void AddMove(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=MoveOrderCnt) exit(255);
   MoveOrders[InstrZ].Code=NCode;
   AddInstTree(&ITree,NName,CodeMove,InstrZ++);
END

        static void AddShift(char *NName, Byte NCode, Boolean NMay)
BEGIN
   if (InstrZ>=ShiftOrderCnt) exit(255);
   ShiftOrders[InstrZ].Code=NCode;
   ShiftOrders[InstrZ].MayReg=NMay;
   AddInstTree(&ITree,NName,CodeShift,InstrZ++);
END

        static void AddBit(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ].Code=NCode;
   AddInstTree(&ITree,NName,CodeBit,InstrZ++);
END

        static void AddAcc(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Code=NCode;
   AddInstTree(&ITree,NName,CodeAcc,InstrZ++);
END

        static void AddCondition(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ConditionCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   ITree=Nil;

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("EXX" ,0x0a); AddFixed("CCF" ,0x0e);
   AddFixed("SCF" ,0x0d); AddFixed("RCF" ,0x0c);
   AddFixed("NOP" ,0x00); AddFixed("HALT",0x01);
   AddFixed("DI"  ,0x02); AddFixed("EI"  ,0x03);
   AddFixed("SWI" ,0xff); AddFixed("RLCA",0xa0);
   AddFixed("RRCA",0xa1); AddFixed("RLA" ,0xa2);
   AddFixed("RRA" ,0xa3); AddFixed("SLAA",0xa4);
   AddFixed("SRAA",0xa5); AddFixed("SLLA",0xa6);
   AddFixed("SRLA",0xa7); AddFixed("RETI",0x1f);

   MoveOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*MoveOrderCnt); InstrZ=0;
   AddMove("LDI" ,0x58);
   AddMove("LDIR",0x59);
   AddMove("LDD" ,0x5a);
   AddMove("LDDR",0x5b);
   AddMove("CPI" ,0x5c);
   AddMove("CPIR",0x5d);
   AddMove("CPD" ,0x5e);
   AddMove("CPDR",0x5f);

   ShiftOrders=(ShiftOrder *) malloc(sizeof(ShiftOrder)*ShiftOrderCnt); InstrZ=0;
   AddShift("RLC",0xa0,True );
   AddShift("RRC",0xa1,True );
   AddShift("RL" ,0xa2,True );
   AddShift("RR" ,0xa3,True );
   AddShift("SLA",0xa4,True );
   AddShift("SRA",0xa5,True );
   AddShift("SLL",0xa6,True );
   AddShift("SRL",0xa7,True );
   AddShift("RLD",0x10,False);
   AddShift("RRD",0x11,False);

   BitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BitOrderCnt); InstrZ=0;
   AddBit("BIT" ,0xa8);
   AddBit("SET" ,0xb8);
   AddBit("RES" ,0xb0);
   AddBit("TSET",0x18);

   AccOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*AccOrderCnt); InstrZ=0;
   AddAcc("DAA",0x0b);
   AddAcc("CPL",0x10);
   AddAcc("NEG",0x11);

   ALU2Orders=(char **) malloc(sizeof(char *)*ALU2OrderCnt); InstrZ=0;
   ALU2Orders[InstrZ++]="ADD"; ALU2Orders[InstrZ++]="ADC";
   ALU2Orders[InstrZ++]="SUB"; ALU2Orders[InstrZ++]="SBC";
   ALU2Orders[InstrZ++]="AND"; ALU2Orders[InstrZ++]="XOR";
   ALU2Orders[InstrZ++]="OR";  ALU2Orders[InstrZ++]="CP";

   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCnt); InstrZ=0;
   AddCondition("F"  ,  0); DefaultCondition=InstrZ; AddCondition("T"  ,  8);
   AddCondition("Z"  ,  6); AddCondition("NZ" , 14);
   AddCondition("C"  ,  7); AddCondition("NC" , 15);
   AddCondition("PL" , 13); AddCondition("MI" ,  5);
   AddCondition("P"  , 13); AddCondition("M"  ,  5);
   AddCondition("NE" , 14); AddCondition("EQ" ,  6);
   AddCondition("OV" ,  4); AddCondition("NOV", 12);
   AddCondition("PE" ,  4); AddCondition("PO" , 12);
   AddCondition("GE" ,  9); AddCondition("LT" ,  1);
   AddCondition("GT" , 10); AddCondition("LE" ,  2);
   AddCondition("UGE", 15); AddCondition("ULT",  7);
   AddCondition("UGT", 11); AddCondition("ULE",  3);
END

	static void DeinitFields(void)
BEGIN
   ClearInstTree(&ITree);

   free(FixedOrders);
   free(MoveOrders);
   free(ShiftOrders);
   free(BitOrders);
   free(AccOrders);
   free(ALU2Orders);
   free(Conditions);
END

/*-------------------------------------------------------------------------*/

	static Boolean ChkPC_90C141(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode: return (ProgCounter()<=0xffff);
     default: return False;
    END
END

	static Boolean IsDef_90C141(void)
BEGIN
   return False;
END

        static void SwitchFrom_90C141(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_90C141(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=True;

   PCSymbol="$"; HeaderID=0x53; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_90C141; ChkPC=ChkPC_90C141; IsDef=IsDef_90C141;
   SwitchFrom=SwitchFrom_90C141; InitFields();
END

	void code90c141_init(void)
BEGIN
   CPU90C141=AddCPU("90C141",SwitchTo_90C141);
END
