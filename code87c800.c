/* code87c800.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TLCS-870                                                    */
/*                                                                           */
/* Historie: 29.12.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC umgebaut                                       */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code87c800.c,v 1.5 2010/04/17 13:14:22 alfred Exp $                  */
/*****************************************************************************
 * $Log: code87c800.c,v $
 * Revision 1.5  2010/04/17 13:14:22  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.4  2007/11/24 22:48:05  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 17:31:04  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:02  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h" 

#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"


typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } CondRec;

#define FixedOrderCnt 7
#define ConditionCnt 12
#define RegOrderCnt 7
#define ALUOrderCnt 8


#define ModNone (-1)
#define ModReg8 0
#define MModReg8 (1 << ModReg8)
#define ModReg16 1
#define MModReg16 (1 << ModReg16)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModAbs 3
#define MModAbs (1 << ModAbs)
#define ModMem 4
#define MModMem (1 << ModMem)

#define AccReg 0
#define WAReg 0

#define Reg8Cnt 8
static char *Reg8Names="AWCBEDLH";

static CPUVar CPU87C00,CPU87C20,CPU87C40,CPU87C70;
static ShortInt OpSize;
static Byte AdrVals[4];
static ShortInt AdrType;
static Byte AdrMode;

static FixedOrder *FixedOrders;
static CondRec *Conditions;
static FixedOrder *RegOrders;
static char **ALUOrders;

/*--------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddCond(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ConditionCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END

        static void AddReg(char *NName, Word NCode)
BEGIN
   if (InstrZ>=RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name=NName;
   RegOrders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("DI"  , 0x483a);
   AddFixed("EI"  , 0x403a);
   AddFixed("RET" , 0x0005);
   AddFixed("RETI", 0x0004);
   AddFixed("RETN", 0xe804);
   AddFixed("SWI" , 0x00ff);
   AddFixed("NOP" , 0x0000);

   Conditions=(CondRec *) malloc(sizeof(CondRec)*ConditionCnt); InstrZ=0;
   AddCond("EQ", 0); AddCond("Z" , 0);
   AddCond("NE", 1); AddCond("NZ", 1);
   AddCond("CS", 2); AddCond("LT", 2);
   AddCond("CC", 3); AddCond("GE", 3);
   AddCond("LE", 4); AddCond("GT", 5);
   AddCond("T" , 6); AddCond("F" , 7);

   RegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RegOrderCnt); InstrZ=0;
   AddReg("DAA" , 0x0a);  AddReg("DAS" , 0x0b);
   AddReg("SHLC", 0x1c);  AddReg("SHRC", 0x1d);
   AddReg("ROLC", 0x1e);  AddReg("RORC", 0x1f);
   AddReg("SWAP", 0x01);

   ALUOrders=(char **) malloc(sizeof(char *)*ALUOrderCnt); InstrZ=0; 
   ALUOrders[InstrZ++]="ADDC";
   ALUOrders[InstrZ++]="ADD";
   ALUOrders[InstrZ++]="SUBB";
   ALUOrders[InstrZ++]="SUB";
   ALUOrders[InstrZ++]="AND";
   ALUOrders[InstrZ++]="XOR";
   ALUOrders[InstrZ++]="OR";
   ALUOrders[InstrZ++]="CMP";
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(Conditions);
   free(RegOrders);
   free(ALUOrders);
END

/*--------------------------------------------------------------------------*/

        static void ChkAdr(Byte Erl)
BEGIN
   if ((AdrType!=ModNone) AND (((1<<AdrType)&Erl)==0))
    BEGIN
     AdrType=ModNone; AdrCnt=0; WrError(1350);
    END
END

        static void DecodeAdr(char *Asc, Byte Erl)
BEGIN
#define Reg16Cnt 4
   static char *Reg16Names[Reg16Cnt]={"WA","BC","DE","HL"};
#define AdrRegCnt 5
   static char *AdrRegs[AdrRegCnt]={"HL","DE","C","PC","A"};

   int z;
   Byte RegFlag;
   LongInt DispAcc,DispPart;
   String AdrPart;
   Boolean OK,NegFlag,NNegFlag,FirstFlag;
   char *PPos,*NPos,*EPos;

   AdrType=ModNone; AdrCnt=0;

   if (strlen(Asc)==1)
    for (z=0; z<Reg8Cnt; z++)
     if (mytoupper(*Asc)==Reg8Names[z])
      BEGIN
       AdrType=ModReg8; OpSize=0; AdrMode=z;
       ChkAdr(Erl); return;
      END

   for (z=0; z<Reg16Cnt; z++)
    if (strcasecmp(Asc,Reg16Names[z])==0)
     BEGIN
      AdrType=ModReg16; OpSize=1; AdrMode=z;
      ChkAdr(Erl); return;
     END

   if (IsIndirect(Asc))
    BEGIN
     Asc++; Asc[strlen(Asc)-1]='\0';
     if (strcasecmp(Asc,"-HL")==0)
      BEGIN
       AdrType=ModMem; AdrMode=7; ChkAdr(Erl); return;
      END
     if (strcasecmp(Asc,"HL+")==0)
      BEGIN
       AdrType=ModMem; AdrMode=6; ChkAdr(Erl); return;
      END
     RegFlag=0; DispAcc=0; NegFlag=False; OK=True; FirstFlag=False;
     while ((OK) AND (*Asc!='\0'))
      BEGIN
       PPos=QuotPos(Asc,'+'); NPos=QuotPos(Asc,'-');
       if (PPos==Nil) EPos=NPos;
       else if (NPos==Nil) EPos=PPos;
       else EPos=min(NPos,PPos);
       NNegFlag=((EPos!=Nil) AND (*EPos=='-'));
       if (EPos==Nil)
        BEGIN
         strmaxcpy(AdrPart,Asc,255); *Asc='\0';
        END
       else
        BEGIN
         *EPos='\0'; strmaxcpy(AdrPart,Asc,255); strmov(Asc,EPos+1);
        END
       for (z=0; z<AdrRegCnt; z++)
        if (strcasecmp(AdrPart,AdrRegs[z])==0) break;
       if (z>=AdrRegCnt)
        BEGIN
         FirstPassUnknown=False;
         DispPart=EvalIntExpression(AdrPart,Int32,&OK);
         FirstFlag|=FirstPassUnknown;
         if (NegFlag) DispAcc-=DispPart; else DispAcc+=DispPart;
        END
       else if ((NegFlag) OR ((RegFlag & (1 << z))!=0))
        BEGIN
         WrError(1350); OK=False;
        END
       else RegFlag+=1 << z;
       NegFlag=NNegFlag;
      END
     if (DispAcc!=0) RegFlag+=1 << AdrRegCnt;
     if (OK)
      switch (RegFlag)
       BEGIN
        case 0x20:
         if (FirstFlag) DispAcc&=0xff;
         if (DispAcc>0xff) WrError(1320);
         else
          BEGIN
           AdrType=ModAbs; AdrMode=0;
           AdrCnt=1; AdrVals[0]=DispAcc & 0xff;
          END
         break;
        case 0x02:
         AdrType=ModMem; AdrMode=2;
         break;
        case 0x01:
         AdrType=ModMem; AdrMode=3;
         break;
        case 0x21:
         if (FirstFlag) DispAcc&=0x7f;
         if (DispAcc>127) WrError(1320);
         else if (DispAcc<-128) WrError(1315);
         else
          BEGIN
           AdrType=ModMem; AdrMode=4;
           AdrCnt=1; AdrVals[0]=DispAcc & 0xff;
          END
         break;
        case 0x05:
         AdrType=ModMem; AdrMode=5;
         break;
        case 0x18:
         AdrType=ModMem; AdrMode=1;
         break;
        default:
         WrError(1350);
       END
     ChkAdr(Erl); return;
    END
   else
    switch (OpSize)
     BEGIN
      case -1:
       WrError(1132);
       break;
      case 0:
       AdrVals[0]=EvalIntExpression(Asc,Int8,&OK);
       if (OK)
        BEGIN
         AdrType=ModImm; AdrCnt=1;
        END
       break;
      case 1:
       DispAcc=EvalIntExpression(Asc,Int16,&OK);
       if (OK)
        BEGIN
         AdrType=ModImm; AdrCnt=2;
         AdrVals[0]=DispAcc & 0xff;
         AdrVals[1]=(DispAcc >> 8) & 0xff;
        END
       break;
     END

   ChkAdr(Erl);
END

        static Boolean SplitBit(char *Asc, Byte *Erg)
BEGIN
   char *p;
   String Part;

   p=RQuotPos(Asc,'.');
   if (p==Nil) return False;
   *p='\0'; strmaxcpy(Part,p+1,255);

   if (strlen(Part)!=1) return False;
   else
    if ((*Part>='0') AND (*Part<='7'))
     BEGIN
      *Erg=(*Part)-'0'; return True;
     END
    else
     BEGIN
      for (*Erg=0; *Erg<Reg8Cnt; (*Erg)++)
       if (mytoupper(*Part)==Reg8Names[*Erg]) break;
      if (*Erg<Reg8Cnt)
       BEGIN
        *Erg+=8; return True;
       END
      else return False;
     END
END

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        static void CodeMem(Byte Entry, Byte Opcode)
BEGIN
   BAsmCode[0]=Entry+AdrMode;
   memcpy(BAsmCode+1,AdrVals,AdrCnt);
   BAsmCode[1+AdrCnt]=Opcode;
END

        static void MakeCode_87C800(void)
BEGIN
   int z;
   Integer AdrInt,Condition;
   Byte HReg,HCnt,HMode,HVal;
   Boolean OK;

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
      else
       BEGIN
        CodeLen=0;
        if (Hi(FixedOrders[z].Code)!=0) BAsmCode[CodeLen++]=Hi(FixedOrders[z].Code);
        BAsmCode[CodeLen++]=Lo(FixedOrders[z].Code);
       END
      return;
     END

   /* Datentransfer */

   if (Memo("LD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"SP")==0)
      BEGIN
       OpSize=1;
       DecodeAdr(ArgStr[2],MModImm+MModReg16);
       switch (AdrType)
        BEGIN
         case ModReg16:
          CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0xfa;
          break;
         case ModImm:
          CodeLen=3; BAsmCode[0]=0xfa; memcpy(BAsmCode+1,AdrVals,AdrCnt);
          break;
        END
      END
     else if (strcasecmp(ArgStr[2],"SP")==0)
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       switch (AdrType)
        BEGIN
         case ModReg16:
          CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0xfb;
          break;
        END
      END
     else if (strcasecmp(ArgStr[1],"RBS")==0)
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[2],Int4,&OK);
       if (OK)
        BEGIN
         CodeLen=2; BAsmCode[0]=0x0f;
        END
      END
     else if (strcasecmp(ArgStr[1],"CF")==0)
      BEGIN
       if (NOT SplitBit(ArgStr[2],&HReg)) WrError(1510);
       else
        BEGIN
         DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModMem);
         switch (AdrType)
          BEGIN
           case ModReg8:
            if (HReg>=8) WrError(1350);
            else
             BEGIN
              CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0xd8+HReg;
             END
            break;
           case ModAbs:
            if (HReg>=8) WrError(1350);
            else
             BEGIN
              CodeLen=2; BAsmCode[0]=0xd8+HReg; BAsmCode[1]=AdrVals[0];
             END
            break;
           case ModMem:
            if (HReg<8)
             BEGIN
              CodeLen=2+AdrCnt;
              CodeMem(0xe0,0xd8+HReg);
             END
            else if ((AdrMode!=2) AND (AdrMode!=3)) WrError(1350);
            else
             BEGIN
              CodeLen=2; BAsmCode[0]=0xe0+HReg; BAsmCode[1]=0x9c+AdrMode;
             END
            break;
          END
        END
      END
     else if (strcasecmp(ArgStr[2],"CF")==0)
      BEGIN
       if (NOT SplitBit(ArgStr[1],&HReg)) WrError(1510);
       else
        BEGIN
         DecodeAdr(ArgStr[1],MModReg8+MModAbs+MModMem);
         switch (AdrType)
          BEGIN
           case ModReg8:
            if (HReg>=8) WrError(1350);
            else
             BEGIN
              CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0xc8+HReg;
             END
            break;
           case ModAbs:
           case ModMem:
            if (HReg<8)
             BEGIN
              CodeLen=2+AdrCnt; CodeMem(0xe0,0xc8+HReg);
             END
            else if ((AdrMode!=2) AND (AdrMode!=3)) WrError(1350);
            else
             BEGIN
              CodeLen=2; BAsmCode[0]=0xe0+HReg; BAsmCode[1]=0x98+AdrMode;
             END
            break;
          END
        END
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModAbs+MModMem);
       switch (AdrType)
        BEGIN
         case ModReg8:
          HReg=AdrMode;
          DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModMem+MModImm);
          switch (AdrType)
           BEGIN
            case ModReg8:
             if (HReg==AccReg)
              BEGIN
               CodeLen=1; BAsmCode[0]=0x50+AdrMode;
              END
             else if (AdrMode==AccReg)
              BEGIN
               CodeLen=1; BAsmCode[0]=0x58+HReg;
              END
             else
              BEGIN
               CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0x58+HReg;
              END
             break;
            case ModAbs:
             if (HReg==AccReg)
              BEGIN
               CodeLen=2; BAsmCode[0]=0x22; BAsmCode[1]=AdrVals[0];
              END
             else
              BEGIN
               CodeLen=3; BAsmCode[0]=0xe0; BAsmCode[1]=AdrVals[0];
               BAsmCode[2]=0x58+HReg;
              END
             break;
            case ModMem:
             if ((HReg==AccReg) AND (AdrMode==3))   /* A,(HL) */
              BEGIN
               CodeLen=1; BAsmCode[0]=0x23;
              END
             else
              BEGIN
               CodeLen=2+AdrCnt; CodeMem(0xe0,0x58+HReg);
               if ((HReg>=6) AND (AdrMode==6)) WrError(140);
              END
             break;
            case ModImm:
             CodeLen=2; BAsmCode[0]=0x30+HReg; BAsmCode[1]=AdrVals[0];
             break;
           END
          break;
         case ModReg16:
          HReg=AdrMode;
          DecodeAdr(ArgStr[2],MModReg16+MModAbs+MModMem+MModImm);
          switch (AdrType)
           BEGIN
            case ModReg16:
             CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0x14+HReg;
             break;
            case ModAbs:
             CodeLen=3; BAsmCode[0]=0xe0; BAsmCode[1]=AdrVals[0];
             BAsmCode[2]=0x14+HReg;
             break;
            case ModMem:
             if (AdrMode>5) WrError(1350);   /* (-HL),(HL+) */
             else
              BEGIN
               CodeLen=2+AdrCnt; BAsmCode[0]=0xe0+AdrMode;
               memcpy(BAsmCode+1,AdrVals,AdrCnt);
               BAsmCode[1+AdrCnt]=0x14+HReg;
              END
             break;
            case ModImm:
             CodeLen=3; BAsmCode[0]=0x14+HReg; memcpy(BAsmCode+1,AdrVals,2);
             break;
           END
          break;
         case ModAbs:
          HReg=AdrVals[0]; OpSize=0;
          DecodeAdr(ArgStr[2],MModReg8+MModReg16+MModAbs+MModMem+MModImm);
          switch (AdrType)
           BEGIN
            case ModReg8:
             if (AdrMode==AccReg)
              BEGIN
               CodeLen=2; BAsmCode[0]=0x2a; BAsmCode[1]=HReg;
              END
             else
              BEGIN
               CodeLen=3; BAsmCode[0]=0xf0; BAsmCode[1]=HReg;
               BAsmCode[2]=0x50+AdrMode;
              END
             break;
            case ModReg16:
             CodeLen=3; BAsmCode[0]=0xf0; BAsmCode[1]=HReg;
             BAsmCode[2]=0x10+AdrMode;
             break;
            case ModAbs:
             CodeLen=3; BAsmCode[0]=0x26; BAsmCode[1]=AdrVals[0];
             BAsmCode[2]=HReg;
             break;
            case ModMem:
             if (AdrMode>5) WrError(1350);      /* (-HL),(HL+) */
             else
              BEGIN
               CodeLen=3+AdrCnt; BAsmCode[0]=0xe0+AdrMode;
               memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x26;
               BAsmCode[2+AdrCnt]=HReg;
              END
             break;
            case ModImm:
             CodeLen=3; BAsmCode[0]=0x2c; BAsmCode[1]=HReg;
             BAsmCode[2]=AdrVals[0];
             break;
           END
          break;
         case ModMem:
          HVal=AdrVals[0]; HCnt=AdrCnt; HMode=AdrMode; OpSize=0;
          DecodeAdr(ArgStr[2],MModReg8+MModReg16+MModAbs+MModMem+MModImm);
          switch (AdrType)
           BEGIN
            case ModReg8:
             if ((HMode==3) AND (AdrMode==AccReg))   /* (HL),A */
              BEGIN
               CodeLen=1; BAsmCode[0]=0x2b;
              END
             else if ((HMode==1) OR (HMode==5)) WrError(1350);
             else
              BEGIN
               CodeLen=2+HCnt; BAsmCode[0]=0xf0+HMode;
               memcpy(BAsmCode+1,&HVal,HCnt);
               BAsmCode[1+HCnt]=0x50+AdrMode;
               if ((HMode==6) AND (AdrMode>=6)) WrError(140);
              END
             break;
            case ModReg16:
             if ((HMode<2) OR (HMode>4)) WrError(1350);  /* (HL),(DE),(HL+d) */
             else
              BEGIN
               CodeLen=2+HCnt; BAsmCode[0]=0xf0+HMode;
               memcpy(BAsmCode+1,&HVal,HCnt); BAsmCode[1+HCnt]=0x10+AdrMode;
              END
             break;
            case ModAbs:
             if (HMode!=3) WrError(1350);  /* (HL) */
             else
              BEGIN
               CodeLen=3; BAsmCode[0]=0xe0; BAsmCode[1]=AdrVals[0];
               BAsmCode[2]=0x27;
              END
             break;
            case ModMem:
             if (HMode!=3) WrError(1350);         /* (HL) */
             else if (AdrMode>5) WrError(1350);   /* (-HL),(HL+) */
             else
              BEGIN
               CodeLen=2+AdrCnt; BAsmCode[0]=0xe0+AdrMode;
               memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x27;
              END
             break;
            case ModImm:
             if ((HMode==1) OR (HMode==5)) WrError(1350);  /* (HL+C),(PC+A) */
             else if (HMode==3)               /* (HL) */
              BEGIN
               CodeLen=2; BAsmCode[0]=0x2d; BAsmCode[1]=AdrVals[0];
              END
             else
              BEGIN
               CodeLen=3+HCnt; BAsmCode[0]=0xf0+HMode;
               memcpy(BAsmCode+1,&HVal,HCnt); BAsmCode[1+HCnt]=0x2c;
               BAsmCode[2+HCnt]=AdrVals[0];
              END
             break;
           END
          break;
        END
      END
     return;
    END

   if (Memo("XCH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModAbs+MModMem);
       switch (AdrType)
        BEGIN
         case ModReg8:
          HReg=AdrMode;
          DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModMem);
          switch (AdrType)
           BEGIN
            case ModReg8:
             CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0xa8+HReg;
             break;
            case ModAbs:
            case ModMem:
             CodeLen=2+AdrCnt; CodeMem(0xe0,0xa8+HReg);
             if ((HReg>=6) AND (AdrMode==6)) WrError(140);
             break;
           END
          break;
         case ModReg16:
          HReg=AdrMode;
          DecodeAdr(ArgStr[2],MModReg16);
          if (AdrType!=ModNone)
           BEGIN
            CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0x10+HReg;
           END
          break;
         case ModAbs:
          BAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[2],MModReg8);
          if (AdrType!=ModNone)
           BEGIN
            CodeLen=3; BAsmCode[0]=0xe0; BAsmCode[2]=0xa8+AdrMode;
           END
          break;
         case ModMem:
          BAsmCode[0]=0xe0+AdrMode; memcpy(BAsmCode+1,AdrVals,AdrCnt);
          HReg=AdrCnt;
          DecodeAdr(ArgStr[2],MModReg8);
          if (AdrType!=ModNone)
           BEGIN
            CodeLen=2+HReg; BAsmCode[1+HReg]=0xa8+AdrMode;
            if ((AdrMode>=6) AND ((BAsmCode[0] & 0x0f)==6)) WrError(140);
           END
          break;
        END
      END
     return;
    END

   if (Memo("CLR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CF")==0)
      BEGIN
       CodeLen=1; BAsmCode[0]=0x0c;
      END
     else if (SplitBit(ArgStr[1],&HReg))
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModAbs+MModMem);
       switch (AdrType)
        BEGIN
         case ModReg8:
          if (HReg>=8) WrError(1350);
          else
           BEGIN
            CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0x48+HReg;
           END
          break;
         case ModAbs:
          if (HReg>=8) WrError(1350);
          else
           BEGIN
            CodeLen=2; BAsmCode[0]=0x48+HReg; BAsmCode[1]=AdrVals[0];
           END
          break;
         case ModMem:
          if (HReg<=8)
           BEGIN
            CodeLen=2+AdrCnt; CodeMem(0xe0,0x48+HReg);
           END
          else if ((AdrMode!=2) AND (AdrMode!=3)) WrError(1350);
          else
           BEGIN
            CodeLen=2; BAsmCode[0]=0xe0+HReg; BAsmCode[1]=0x88+AdrMode;
           END
          break;
        END
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModAbs+MModMem);
       switch (AdrType)
        BEGIN
         case ModReg8:
          CodeLen=2; BAsmCode[0]=0x30+AdrMode; BAsmCode[1]=0;
          break;
         case ModReg16:
          CodeLen=3; BAsmCode[0]=0x14+AdrMode; BAsmCode[1]=0; BAsmCode[2]=0;
          break;
         case ModAbs:
          CodeLen=2; BAsmCode[0]=0x2e; BAsmCode[1]=AdrVals[0];
          break;
         case ModMem:
          if ((AdrMode==5) OR (AdrMode==1)) WrError(1350);  /* (PC+A, HL+C) */
          else if (AdrMode==3)     /* (HL) */
           BEGIN
            CodeLen=1; BAsmCode[0]=0x2f;
           END
          else
           BEGIN
            CodeLen=3+AdrCnt; BAsmCode[0]=0xf0+AdrMode;
            memcpy(BAsmCode+1,AdrVals,AdrCnt);
            BAsmCode[1+AdrCnt]=0x2c; BAsmCode[2+AdrCnt]=0;
           END
          break;
        END
      END
     return;
    END

   if (Memo("LDW"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK);
       if (OK)
        BEGIN
         DecodeAdr(ArgStr[1],MModReg16+MModAbs+MModMem);
         switch (AdrType)
          BEGIN
           case ModReg16:
            CodeLen=3; BAsmCode[0]=0x14+AdrMode;
            BAsmCode[1]=AdrInt & 0xff; BAsmCode[2]=AdrInt >> 8;
            break;
           case ModAbs:
            CodeLen=4; BAsmCode[0]=0x24; BAsmCode[1]=AdrVals[0];
            BAsmCode[2]=AdrInt & 0xff; BAsmCode[3]=AdrInt >> 8;
            break;
           case ModMem:
            if (AdrMode!=3) WrError(1350);  /* (HL) */
            else
             BEGIN
              CodeLen=3; BAsmCode[0]=0x25;
              BAsmCode[1]=AdrInt & 0xff; BAsmCode[2]=AdrInt >> 8;
             END
            break;
          END
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     HReg=Ord(Memo("PUSH"))+6;
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"PSW")==0)
      BEGIN
       CodeLen=1; BAsmCode[0]=HReg;
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       if (AdrType!=ModNone)
        BEGIN
         CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=HReg;
        END
      END
     return;
    END

   if ((Memo("TEST")) OR (Memo("CPL")) OR (Memo("SET")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CF")==0)
      BEGIN
       if (Memo("TEST")) WrError(1350);
       else
        BEGIN
         CodeLen=1; BAsmCode[0]=0x0d+Ord(Memo("CPL"));
        END
      END
     else if (NOT SplitBit(ArgStr[1],&HReg)) WrError(1510);
     else
      BEGIN
       if (Memo("TEST")) HVal=0xd8;
       else if (Memo("SET")) HVal=0x40;
       else HVal=0xc0;
       DecodeAdr(ArgStr[1],MModReg8+MModAbs+MModMem);
       switch (AdrType)
        BEGIN
         case ModReg8:
          if (HReg>=8) WrError(1350);
          else
           BEGIN
            CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=HVal+HReg;
           END
          break;
         case ModAbs:
          if (HReg>=8) WrError(1350);
          else if (Memo("CPL"))
           BEGIN
            CodeLen=3; CodeMem(0xe0,HVal+HReg);
           END
          else
           BEGIN
            CodeLen=2; BAsmCode[0]=HVal+HReg; BAsmCode[1]=AdrVals[0];
           END
          break;
         case ModMem:
          if (HReg<8)
           BEGIN
            CodeLen=2+AdrCnt; CodeMem(0xe0,HVal+HReg);
           END
          else if ((AdrMode!=2) AND (AdrMode!=3)) WrError(1350);
          else
           BEGIN
            CodeLen=2; BAsmCode[0]=0xe0+HReg;
            BAsmCode[1]=((HVal & 0x18) >> 1)+((HVal & 0x80) >> 3)+0x80+AdrMode;
           END
          break;
        END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<RegOrderCnt; z++)
    if (Memo(RegOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg8);
        if (AdrType!=ModNone)
         BEGIN
          if (AdrMode==AccReg)
           BEGIN
            CodeLen=1; BAsmCode[0]=RegOrders[z].Code;
           END
          else
           BEGIN
            CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=RegOrders[z].Code;
           END
         END
       END
      return;
     END

   for (z=0; z<ALUOrderCnt; z++)
    if (Memo(ALUOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (strcasecmp(ArgStr[1],"CF")==0)
       BEGIN
        if (NOT Memo("XOR")) WrError(1350);
        else if (NOT SplitBit(ArgStr[2],&HReg)) WrError(1510);
        else if (HReg>=8) WrError(1350);
        else
         BEGIN
          DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModMem);
          switch (AdrType)
           BEGIN
            case ModReg8:
             CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0xd0+HReg;
             break;
            case ModAbs:
            case ModMem:
             CodeLen=2+AdrCnt; CodeMem(0xe0,0xd0+HReg);
             break;
           END
         END
       END
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModMem+MModAbs);
        switch (AdrType)
         BEGIN
          case ModReg8:
           HReg=AdrMode;
           DecodeAdr(ArgStr[2],MModReg8+MModMem+MModAbs+MModImm);
           switch (AdrType)
            BEGIN
             case ModReg8:
              if (HReg==AccReg)
               BEGIN
                CodeLen=2;
                BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0x60+z;
               END
              else if (AdrMode==AccReg)
               BEGIN
                CodeLen=2;
                BAsmCode[0]=0xe8+HReg; BAsmCode[1]=0x68+z;
               END
              else WrError(1350);
              break;
             case ModMem:
              if (HReg!=AccReg) WrError(1350);
              else
               BEGIN
                CodeLen=2+AdrCnt; BAsmCode[0]=0xe0+AdrMode;
                memcpy(BAsmCode+1,AdrVals,AdrCnt);
                BAsmCode[1+AdrCnt]=0x78+z;
               END
              break;
             case ModAbs:
              if (HReg!=AccReg) WrError(1350);
              else
               BEGIN
                CodeLen=2; BAsmCode[0]=0x78+z; BAsmCode[1]=AdrVals[0];
               END
              break;
             case ModImm:
              if (HReg==AccReg)
               BEGIN
                CodeLen=2; BAsmCode[0]=0x70+z; BAsmCode[1]=AdrVals[0];
               END
              else
               BEGIN
                CodeLen=3; BAsmCode[0]=0xe8+HReg;
                BAsmCode[1]=0x70+z; BAsmCode[2]=AdrVals[0];
               END
              break;
            END
           break;
          case ModReg16:
           HReg=AdrMode; DecodeAdr(ArgStr[2],MModImm+MModReg16);
           switch (AdrType)
            BEGIN
             case ModImm:
              CodeLen=4; BAsmCode[0]=0xe8+HReg; BAsmCode[1]=0x38+z;
              memcpy(BAsmCode+2,AdrVals,AdrCnt);
              break;
             case ModReg16:
              if (HReg!=WAReg) WrError(1350);
              else
               BEGIN
                CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=0x30+z;
               END
              break;
            END
           break;
          case ModAbs:
           if (strcasecmp(ArgStr[2],"(HL)")==0)
            BEGIN
             CodeLen=3; BAsmCode[0]=0xe0;
             BAsmCode[1]=AdrVals[0]; BAsmCode[2]=0x60+z;
            END
           else
            BEGIN
             BAsmCode[3]=EvalIntExpression(ArgStr[2],Int8,&OK);
             if (OK)
              BEGIN
               CodeLen=4; BAsmCode[0]=0xe0;
               BAsmCode[1]=AdrVals[0]; BAsmCode[2]=0x70+z;
              END
            END
           break;
          case ModMem:
           if (strcasecmp(ArgStr[2],"(HL)")==0)
            BEGIN
             CodeLen=2+AdrCnt; BAsmCode[0]=0xe0+AdrMode;
             memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x60+z;
            END
           else
            BEGIN
             BAsmCode[2+AdrCnt]=EvalIntExpression(ArgStr[2],Int8,&OK);
             if (OK)
              BEGIN
               CodeLen=3+AdrCnt; BAsmCode[0]=0xe0+AdrMode;
               memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x70+z;
              END
            END
           break;
         END
       END
      return;
     END

   if (Memo("MCMP"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       HReg=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
        BEGIN
         DecodeAdr(ArgStr[1],MModMem+MModAbs);
         if (AdrType!=ModNone)
          BEGIN
           CodeLen=3+AdrCnt; CodeMem(0xe0,0x2f); BAsmCode[2+AdrCnt]=HReg;
          END
        END
      END
     return;
    END

   if ((Memo("DEC")) OR (Memo("INC")))
    BEGIN
     HReg=Ord(Memo("DEC")) << 3;
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModAbs+MModMem);
       switch (AdrType)
        BEGIN
         case ModReg8:
          CodeLen=1; BAsmCode[0]=0x60+HReg+AdrMode;
          break;
         case ModReg16:
          CodeLen=1; BAsmCode[0]=0x10+HReg+AdrMode;
          break;
         case ModAbs:
          CodeLen=2; BAsmCode[0]=0x20+HReg; BAsmCode[1]=AdrVals[0];
          break;
         case ModMem:
          if (AdrMode==3)     /* (HL) */
           BEGIN
            CodeLen=1; BAsmCode[0]=0x21+HReg;
           END
          else
           BEGIN
            CodeLen=2+AdrCnt; BAsmCode[0]=0xe0+AdrMode;
            memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=0x20+HReg;
           END
          break;
        END
      END
     return;
    END

   if (Memo("MUL"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8);
       if (AdrType==ModReg8)
        BEGIN
         HReg=AdrMode;
         DecodeAdr(ArgStr[2],MModReg8);
         if (AdrType==ModReg8)
          BEGIN
           if ((HReg ^ AdrMode)!=1) WrError(1760);
           else
            BEGIN
             HReg=HReg >> 1;
             if (HReg==0)
              BEGIN
               CodeLen=1; BAsmCode[0]=0x02;
              END
             else
              BEGIN
               CodeLen=2; BAsmCode[0]=0xe8+HReg; BAsmCode[1]=0x02;
              END
            END
          END
        END
      END
     return;
    END

   if (Memo("DIV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       if (AdrType==ModReg16)
        BEGIN
         HReg=AdrMode;
         DecodeAdr(ArgStr[2],MModReg8);
         if (AdrType==ModReg8)
          BEGIN
           if (AdrMode!=2) WrError(1350);  /* C */
           else if (HReg==0)
            BEGIN
             CodeLen=1; BAsmCode[0]=0x03;
            END
           else
            BEGIN
             CodeLen=2; BAsmCode[0]=0xe8+HReg; BAsmCode[1]=0x03;
             if (HReg==1) WrError(140);
            END
          END
        END
      END
     return;
    END

   if ((Memo("ROLD")) OR (Memo("RORD")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
     else
      BEGIN
       HReg=Ord(Memo("RORD"))+8;
       DecodeAdr(ArgStr[2],MModAbs+MModMem);
       if (AdrType!=ModNone)
        BEGIN
         CodeLen=2+AdrCnt; CodeMem(0xe0,HReg);
         if (AdrMode==1) WrError(140);
        END
      END
     return;
    END

   /* Spruenge */

   if (Memo("JRS"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       NLS_UpString(ArgStr[1]);
       for (Condition=ConditionCnt-2; Condition<ConditionCnt; Condition++)
        if (strcmp(ArgStr[1],Conditions[Condition].Name)==0) break;
       if (Condition>=ConditionCnt) WrXError(1360,ArgStr[1]);
       else
        BEGIN
         AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK)-(EProgCounter()+2);
         if (OK)
          BEGIN
           if (((AdrInt<-16) OR (AdrInt>15)) AND (NOT SymbolQuestionable)) WrError(1370);
           else
            BEGIN
             CodeLen=1;
             BAsmCode[0]=((Conditions[Condition].Code-2) << 5)+(AdrInt & 0x1f);
            END
          END
        END
      END
     return;
    END

   if (Memo("JR"))
    BEGIN
     if ((ArgCnt!=2) AND (ArgCnt!=1)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) Condition=(-1);
       else
        BEGIN
         NLS_UpString(ArgStr[1]);
         for (Condition=0; Condition<ConditionCnt; Condition++)
          if (strcmp(ArgStr[1],Conditions[Condition].Name)==0) break;
        END
       if (Condition>=ConditionCnt) WrXError(1360,ArgStr[1]);
       else
        BEGIN
         AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK)-(EProgCounter()+2);
         if (OK)
          BEGIN
           if (((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable)) WrError(1370);
           else
            BEGIN
             CodeLen=2;
             if (Condition==-1) BAsmCode[0]=0xfb;
             else BAsmCode[0]=0xd0+Conditions[Condition].Code;
             BAsmCode[1]=AdrInt & 0xff;
            END
          END
        END
      END
     return;
    END

   if ((Memo("JP")) OR (Memo("CALL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       OpSize=1; HReg=0xfc+2*Ord(Memo("JP"));
       DecodeAdr(ArgStr[1],MModReg16+MModAbs+MModMem+MModImm);
       switch (AdrType)
        BEGIN
         case ModReg16:
          CodeLen=2; BAsmCode[0]=0xe8+AdrMode; BAsmCode[1]=HReg;
          break;
         case ModAbs:
          CodeLen=3; BAsmCode[0]=0xe0; BAsmCode[1]=AdrVals[0];
          BAsmCode[2]=HReg;
          break;
         case ModMem:
          if (AdrMode>5) WrError(1350);
          else
           BEGIN
            CodeLen=2+AdrCnt; BAsmCode[0]=0xe0+AdrMode;
            memcpy(BAsmCode+1,AdrVals,AdrCnt); BAsmCode[1+AdrCnt]=HReg;
           END
          break;
         case ModImm:
          if ((AdrVals[1]==0xff) AND (Memo("CALL")))
           BEGIN
            CodeLen=2; BAsmCode[0]=0xfd; BAsmCode[1]=AdrVals[0];
           END
          else
           BEGIN
            CodeLen=3; BAsmCode[0]=HReg; memcpy(BAsmCode+1,AdrVals,AdrCnt);
           END
          break;
        END
      END
     return;
    END

   if (Memo("CALLV"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       HVal=EvalIntExpression(ArgStr[1],Int4,&OK);
       if (OK)
        BEGIN
         CodeLen=1; BAsmCode[0]=0xc0+(HVal & 15);
        END
      END
     return;
    END

   if (Memo("CALLP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        BEGIN
         if ((Hi(AdrInt)!=0xff) AND (Hi(AdrInt)!=0)) WrError(1320);
         else
          BEGIN
           CodeLen=2; BAsmCode[0]=0xfd; BAsmCode[1]=Lo(AdrInt);
          END
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_87C800(void)
BEGIN
   return False;
END

        static void SwitchFrom_87C800(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_87C800(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=True;

   PCSymbol="$"; HeaderID=0x54; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   MakeCode=MakeCode_87C800; IsDef=IsDef_87C800;
   SwitchFrom=SwitchFrom_87C800; InitFields();
END

        void code87c800_init(void)
BEGIN
   CPU87C00=AddCPU("87C00",SwitchTo_87C800);
   CPU87C20=AddCPU("87C20",SwitchTo_87C800);
   CPU87C40=AddCPU("87C40",SwitchTo_87C800);
   CPU87C70=AddCPU("87C70",SwitchTo_87C800);
END
