/* code48.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul MCS-48-Familie                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*            5. 1.2000 fixed accessing P1/P2 with lower case in ANL/ORL     */
/*                                                                           */
/*****************************************************************************/
/* $Id: code48.c,v 1.2 2004/05/29 11:33:00 alfred Exp $                      */
/*****************************************************************************
 * $Log: code48.c,v $
 * Revision 1.2  2004/05/29 11:33:00  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "stringlists.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "intpseudo.h"
#include "codevars.h"

typedef struct
         {
          char *Name;
          Byte Code;
          Byte May2X;
          Byte UPIFlag;
         } CondOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } AccOrder;

typedef struct
         {
          char *Name;
          Byte Code;
          Boolean Is22;
          Boolean IsNUPI;
         } SelOrder;

#define ModImm 0
#define ModReg 1
#define ModInd 2
#define ModAcc 3
#define ModNone (-1)

#define ClrCplCnt 4
#define CondOrderCnt 22
#define AccOrderCnt 6
#define SelOrderCnt 6

#define D_CPU8021  0
#define D_CPU8022  1
#define D_CPU8039  2
#define D_CPU8048  3
#define D_CPU80C39 4
#define D_CPU80C48 5
#define D_CPU8041  6
#define D_CPU8042  7

static ShortInt AdrMode;
static Byte AdrVal;
static CPUVar CPU8021,CPU8022,CPU8039,CPU8048,CPU80C39,CPU80C48,CPU8041,CPU8042;

static char **ClrCplVals;
static Byte *ClrCplCodes;
static CondOrder *CondOrders;
static AccOrder *AccOrders;
static SelOrder *SelOrders;

/****************************************************************************/

        static void AddAcc(char *Name, Byte Code)
BEGIN
   if (InstrZ==AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=Name;
   AccOrders[InstrZ++].Code=Code;
END

        static void AddCond(char *Name, Byte Code, Byte May2X, Byte UPIFlag)
BEGIN
   if (InstrZ==CondOrderCnt) exit(255);
   CondOrders[InstrZ].Name=Name;
   CondOrders[InstrZ].Code=Code;
   CondOrders[InstrZ].May2X=May2X;
   CondOrders[InstrZ++].UPIFlag=UPIFlag;
END

        static void AddSel(char *Name, Byte Code, Byte Is22, Byte IsNUPI)
BEGIN
   if (InstrZ==SelOrderCnt) exit(255);
   SelOrders[InstrZ].Name=Name;
   SelOrders[InstrZ].Code=Code;
   SelOrders[InstrZ].Is22=Is22;
   SelOrders[InstrZ++].IsNUPI=IsNUPI;
END

        static void InitFields(void)
BEGIN
   ClrCplVals=(char **) malloc(sizeof(char *)*ClrCplCnt);
   ClrCplCodes=(Byte *) malloc(sizeof(Byte)*ClrCplCnt);
   ClrCplVals[0]="A"; ClrCplVals[1]="C"; ClrCplVals[2]="F0"; ClrCplVals[3]="F1";
   ClrCplCodes[0]=0x27; ClrCplCodes[1]=0x97; ClrCplCodes[2]=0x85; ClrCplCodes[3]=0xa5;
   
   CondOrders=(CondOrder *) malloc(sizeof(CondOrder)*CondOrderCnt); InstrZ=0;
   AddCond("JTF"  ,0x16, 2, 3); AddCond("JNI"  ,0x86, 0, 2);
   AddCond("JC"   ,0xf6, 2, 3); AddCond("JNC"  ,0xe6, 2, 3);
   AddCond("JZ"   ,0xc6, 2, 3); AddCond("JNZ"  ,0x96, 2, 3);
   AddCond("JT0"  ,0x36, 1, 3); AddCond("JNT0" ,0x26, 1, 3);
   AddCond("JT1"  ,0x56, 2, 3); AddCond("JNT1" ,0x46, 2, 3);
   AddCond("JF0"  ,0xb6, 0, 3); AddCond("JF1"  ,0x76, 0, 3);
   AddCond("JNIBF",0xd6, 2, 1); AddCond("JOBF" ,0x86, 2, 1);
   AddCond("JB0"  ,0x12, 0, 3); AddCond("JB1"  ,0x32, 0, 3);
   AddCond("JB2"  ,0x52, 0, 3); AddCond("JB3"  ,0x72, 0, 3);
   AddCond("JB4"  ,0x92, 0, 3); AddCond("JB5"  ,0xb2, 0, 3);
   AddCond("JB6"  ,0xd2, 0, 3); AddCond("JB7"  ,0xf2, 0, 3);

   AccOrders=(AccOrder *) malloc(sizeof(AccOrder)*AccOrderCnt); InstrZ=0;
   AddAcc("DA"  ,0x57);
   AddAcc("RL"  ,0xe7);
   AddAcc("RLC" ,0xf7);
   AddAcc("RR"  ,0x77);
   AddAcc("RRC" ,0x67);
   AddAcc("SWAP",0x47);

   SelOrders=(SelOrder *) malloc(sizeof(SelOrder)*SelOrderCnt); InstrZ=0;
   AddSel("MB0" ,0xe5, False, True );
   AddSel("MB1" ,0xf5, False, True );
   AddSel("RB0" ,0xc5, False, False);
   AddSel("RB1" ,0xd5, False, False);
   AddSel("AN0" ,0x95, True , False);
   AddSel("AN1" ,0x85, True , False);
END

        static void DeinitFields(void)
BEGIN
   free(ClrCplVals); 
   free(ClrCplCodes);
   free(CondOrders);
   free(AccOrders);
   free(SelOrders);
END

/****************************************************************************/

        static void DecodeAdr(char *Asc_O)
BEGIN
   Boolean OK;
   String Asc;

   strmaxcpy(Asc,Asc_O,255);
   AdrMode=ModNone;

   if (*Asc=='\0') return;

   if (strcasecmp(Asc,"A")==0) AdrMode=ModAcc;

   else if (*Asc=='#')
    BEGIN
     AdrVal=EvalIntExpression(Asc+1,Int8,&OK);
     if (OK)
      BEGIN
       AdrMode=ModImm; BAsmCode[1]=AdrVal;
      END
    END

   else if ((strlen(Asc)==2) AND (toupper(*Asc)=='R'))
    BEGIN
     if ((Asc[1]>='0') AND (Asc[1]<='7'))
      BEGIN
       AdrMode=ModReg; AdrVal=Asc[1]-'0';
      END
    END

   else if ((strlen(Asc)==3) AND (*Asc=='@') AND (toupper(Asc[1])=='R'))
    BEGIN
     if ((Asc[2]>='0') AND (Asc[2]<='1'))
      BEGIN
       AdrMode=ModInd; AdrVal=Asc[2]-'0';
      END
    END
END

        static void ChkN802X(void)
BEGIN
   if (CodeLen==0) return;
   if ((MomCPU==CPU8021) OR (MomCPU==CPU8022))
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static void Chk802X(void)
BEGIN
   if (CodeLen==0) return;
   if ((MomCPU!=CPU8021) AND (MomCPU!=CPU8022))
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static void ChkNUPI(void)
BEGIN
   if (CodeLen==0) return;
   if ((MomCPU==CPU8041) OR (MomCPU==CPU8042))
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static void ChkUPI(void)
BEGIN
   if (CodeLen==0) return;
   if ((MomCPU!=CPU8041) AND (MomCPU!=CPU8042))
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static void ChkExt(void)
BEGIN
   if (CodeLen==0) return;
   if ((MomCPU==CPU8039) OR (MomCPU==CPU80C39))
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        void MakeCode_48(void)
BEGIN
   Boolean OK;
   Word AdrWord;
   int z;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   if ((Memo("ADD")) OR (Memo("ADDC")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1135);
     else
      BEGIN
       DecodeAdr(ArgStr[2]);
       if ((AdrMode==ModNone) OR (AdrMode==ModAcc)) WrError(1350);
       else
        BEGIN
         switch (AdrMode)
          BEGIN
           case ModImm:
            CodeLen=2; BAsmCode[0]=0x03;
            break;
           case ModReg:
            CodeLen=1; BAsmCode[0]=0x68+AdrVal;
            break;
           case ModInd:
            CodeLen=1; BAsmCode[0]=0x60+AdrVal;
            break;
          END
         if (strlen(OpPart)==4) BAsmCode[0]+=0x10;
        END
      END
     return;
    END

   if ((Memo("ANL")) OR (Memo("ORL")) OR (Memo("XRL")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")==0)
      BEGIN
       DecodeAdr(ArgStr[2]);
       if ((AdrMode==-1) OR (AdrMode==ModAcc)) WrError(1350);
       else
        BEGIN
         switch (AdrMode)
          BEGIN
           case ModImm:
            CodeLen=2; BAsmCode[0]=0x43;
            break;
           case ModReg:
            CodeLen=1; BAsmCode[0]=0x48+AdrVal;
            break;
           case ModInd:
            CodeLen=1; BAsmCode[0]=0x40+AdrVal;
            break;
          END
         if (Memo("ANL")) BAsmCode[0]+=0x10;
         else if (Memo("XRL")) BAsmCode[0]+=0x90;
        END
      END
     else if ((strcasecmp(ArgStr[1],"BUS")==0) OR (strcasecmp(ArgStr[1],"P1")==0) OR (strcasecmp(ArgStr[1],"P2")==0))
      BEGIN
       if (Memo("XRL")) WrError(1350);
       else
        BEGIN
         DecodeAdr(ArgStr[2]);
         if (AdrMode!=ModImm) WrError(1350);
         else
          BEGIN
           CodeLen=2; BAsmCode[0]=0x88;
           if (toupper(*ArgStr[1])=='P') BAsmCode[0]+=ArgStr[1][1]-'0';
           if (Memo("ANL")) BAsmCode[0]+=0x10;
           if (strcasecmp(ArgStr[1],"BUS")==0)
            BEGIN
             ChkExt(); ChkNUPI();
            END
           ChkN802X();
          END
        END
      END
     else WrError(1350);
     return;
    END

   if ((Memo("CALL")) OR (Memo("JMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if ((EProgCounter()&0x7fe)==0x7fe) WrError(1900);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        BEGIN
         if (AdrWord>0xfff) WrError(1320);
         else
          BEGIN
           if ((((int)EProgCounter())&0x800)!=(AdrWord&0x800))
            BEGIN
             BAsmCode[0]=0xe5+((AdrWord&0x800)>>7); CodeLen=1;
            END
           BAsmCode[CodeLen+1]=AdrWord&0xff;
           BAsmCode[CodeLen]=0x04+((AdrWord&0x700)>>3);
           if (Memo("CALL")) BAsmCode[CodeLen]+=0x10;
           CodeLen+=2; ChkSpace(SegCode);
          END
        END
      END
     return;
    END

   if ((Memo("CLR")) OR (Memo("CPL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       z=0; OK=False; NLS_UpString(ArgStr[1]);
       do
        BEGIN
         if (strcmp(ClrCplVals[z],ArgStr[1])==0)
          BEGIN
           CodeLen=1; BAsmCode[0]=ClrCplCodes[z]; OK=True;
           if (*ArgStr[1]=='F') ChkN802X();
          END
         z++;
        END
       while ((z<ClrCplCnt) AND (CodeLen!=1));
       if (NOT OK) WrError(1135);
       else if (Memo("CPL")) BAsmCode[0]+=0x10;
      END
     return;
    END

   for (z=0; z<AccOrderCnt; z++)
    if (Memo(AccOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
      else
       BEGIN
        CodeLen=1; BAsmCode[0]=AccOrders[z].Code;
       END
      return;
     END

   if (Memo("DEC"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          CodeLen=1; BAsmCode[0]=0x07;
          break;
         case ModReg:
          CodeLen=1; BAsmCode[0]=0xc8+AdrVal;
          ChkN802X();
          break;
         default:
          WrError(1350);
        END
      END
     return;
    END

   if ((Memo("DIS")) OR (Memo("EN")))
    BEGIN
     NLS_UpString(ArgStr[1]);
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       NLS_UpString(ArgStr[1]);
       if (strcmp(ArgStr[1],"I")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x05;
        END
       else if (strcmp(ArgStr[1],"TCNTI")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x25;
        END
       else if ((Memo("EN")) AND (strcmp(ArgStr[1],"DMA")==0))
        BEGIN
         BAsmCode[0]=0xe5; CodeLen=1; ChkUPI();
        END
       else if ((Memo("EN")) AND (strcmp(ArgStr[1],"FLAGS")==0))
        BEGIN
         BAsmCode[0]=0xf5; CodeLen=1; ChkUPI();
        END
       else WrError(1350);
       if (CodeLen!=0)
        BEGIN
         if (Memo("DIS")) BAsmCode[0]+=0x10;
         if (MomCPU==CPU8021)
          BEGIN
           WrError(1500); CodeLen=0;
          END
        END
      END
     return;
    END

   if (Memo("DJNZ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       if (AdrMode!=ModReg) WrError(1350);
       else
        BEGIN
         AdrWord=EvalIntExpression(ArgStr[2],Int16,&OK);
         if (OK)
          BEGIN
           if (((((int)EProgCounter())+1)&0xff00)!=(AdrWord&0xff00)) WrError(1910);
           else
            BEGIN
             CodeLen=2; BAsmCode[0]=0xe8+AdrVal; BAsmCode[1]=AdrWord&0xff;
            END
          END
        END
      END
     return;
    END

   if (Memo("ENT0"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CLK")!=0) WrError(1135);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x75;
       ChkN802X(); ChkNUPI();
      END
     return;
    END

   if (Memo("INC"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrMode)
        BEGIN
         case ModAcc:
          CodeLen=1; BAsmCode[0]=0x17;
          break;
         case ModReg:
          CodeLen=1; BAsmCode[0]=0x18+AdrVal;
          break;
         case ModInd:
          CodeLen=1; BAsmCode[0]=0x10+AdrVal;
          break;
         default:
          WrError(1350);
        END
      END
     return;
    END

   if (Memo("IN"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
     else if (strcasecmp(ArgStr[2],"DBB")==0)
      BEGIN
       CodeLen=1; BAsmCode[0]=0x22; ChkUPI();
      END
     else if ((strlen(ArgStr[2])!=2) OR (toupper(*ArgStr[2])!='P')) WrError(1350);
     else switch (ArgStr[2][1])
      BEGIN
       case '0':
       case '1':
       case '2':
        CodeLen=1; BAsmCode[0]=0x08+ArgStr[2][1]-'0';
        if (ArgStr[2][1]=='0') Chk802X();
        break;
       default:
        WrError(1350);
      END
     return;
    END

   if (Memo("INS"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
     else if (strcasecmp(ArgStr[2],"BUS")!=0) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x08; ChkExt(); ChkNUPI();
      END
     return;
    END

   if (Memo("JMPP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"@A")!=0) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0xb3;
      END
     return;
    END

   for (z=0; z<CondOrderCnt; z++)
    if (Memo(CondOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrWord=EvalIntExpression(ArgStr[1],UInt12,&OK);
        if (NOT OK);
        else if (((((int)EProgCounter())+1)&0xff00)!=(AdrWord&0xff00)) WrError(1910);
        else
         BEGIN
          CodeLen=2; BAsmCode[0]=CondOrders[z].Code; BAsmCode[1]=AdrWord&0xff;
          ChkSpace(SegCode);
          if (CondOrders[z].May2X==0) ChkN802X();
          else if ((CondOrders[z].May2X==1) AND (MomCPU==CPU8021))
           BEGIN
            WrError(1500); CodeLen=0;
           END
          if (CondOrders[z].UPIFlag==1) ChkUPI();
          else if (CondOrders[z].UPIFlag==2) ChkNUPI();
         END
       END
      return;
     END

   if (Memo("JB"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       AdrVal=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
        BEGIN
         AdrWord=EvalIntExpression(ArgStr[2],UInt12,&OK);
         if (NOT OK);
         else if (((((int)EProgCounter())+1)&0xff00)!=(AdrWord&0xff00)) WrError(1910);
         else
          BEGIN
           CodeLen=2; BAsmCode[0]=0x12+(AdrVal<<5);
           BAsmCode[1]=AdrWord&0xff;
           ChkN802X();
          END
        END
      END
     return;
    END

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")==0)
      BEGIN
       if (strcasecmp(ArgStr[2],"T")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x42;
        END
       else if (strcasecmp(ArgStr[2],"PSW")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0xc7; ChkN802X();
        END
       else
        BEGIN
          DecodeAdr(ArgStr[2]);
          switch (AdrMode)
           BEGIN
            case ModReg:
             CodeLen=1; BAsmCode[0]=0xf8+AdrVal;
             break;
            case ModInd:
             CodeLen=1; BAsmCode[0]=0xf0+AdrVal;
             break;
            case ModImm:
             CodeLen=2; BAsmCode[0]=0x23;
             break;
            default:
             WrError(1350);
           END
        END
      END
     else if (strcasecmp(ArgStr[2],"A")==0)
      BEGIN
       if (strcasecmp(ArgStr[1],"STS")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x90; ChkUPI();
        END
       else if (strcasecmp(ArgStr[1],"T")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x62;
        END
       else if (strcasecmp(ArgStr[1],"PSW")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0xd7; ChkN802X();
        END
       else
        BEGIN
         DecodeAdr(ArgStr[1]);
          switch (AdrMode)
           BEGIN
            case ModReg:
             CodeLen=1; BAsmCode[0]=0xa8+AdrVal;
             break;
            case ModInd:
             CodeLen=1; BAsmCode[0]=0xa0+AdrVal;
             break;
            default:
             WrError(1350);
           END
        END
      END
     else if (*ArgStr[2]=='#')
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[2]+1,Int8,&OK);
       if (OK)
        BEGIN
         DecodeAdr(ArgStr[1]);
         switch (AdrMode)
          BEGIN
           case ModReg:
            CodeLen=2; BAsmCode[0]=0xb8+AdrVal; BAsmCode[1]=AdrWord;
            break;
           case ModInd:
            CodeLen=2; BAsmCode[0]=0xb0+AdrVal; BAsmCode[1]=AdrWord;
            break;
           default:
            WrError(1350);
          END
        END
      END
     else WrError(1135);
     return;
    END

   if ((Memo("ANLD")) OR (Memo("ORLD")) OR (Memo("MOVD")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OK=False;
       if ((Memo("MOVD")) AND (strcasecmp(ArgStr[1],"A")==0))
        BEGIN
         strcpy(ArgStr[1],ArgStr[2]); strmaxcpy(ArgStr[2],"A",255); OK=True;
        END
       if (strcasecmp(ArgStr[2],"A")!=0) WrError(1350);
       else if ((strlen(ArgStr[1])!=2) OR (toupper(*ArgStr[1])!='P')) WrError(1350);
       else if ((ArgStr[1][1]<'4') OR (ArgStr[1][1]>'7')) WrError(1320);
       else
        BEGIN
         CodeLen=1; BAsmCode[0]=0x0c+ArgStr[1][1]-'4';
         if (Memo("ANLD")) BAsmCode[0]+=0x90;
         else if (Memo("ORLD")) BAsmCode[0]+=0x80;
         else if (NOT OK) BAsmCode[0]+=0x30;
         ChkN802X();
        END
      END
     return;
    END

   if ((Memo("MOVP")) OR (Memo("MOVP3")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"A")!=0) OR (strcasecmp(ArgStr[2],"@A")!=0)) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0xa3;
       if (Memo("MOVP3")) BAsmCode[0]+=0x40;
       ChkN802X();
      END
     return;
    END

   if (Memo("MOVX"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OK=False;
       if (strcasecmp(ArgStr[2],"A")==0)
        BEGIN
         strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"A",255); OK=True;
        END
       if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
       else
        BEGIN
         DecodeAdr(ArgStr[2]);
         if (AdrMode!=ModInd) WrError(1350);
         else
          BEGIN
           CodeLen=1; BAsmCode[0]=0x80+AdrVal;
           if (OK) BAsmCode[0]+=0x10;
           ChkN802X(); ChkNUPI();
          END
        END
      END
     return;
    END

   if (Memo("NOP"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x00;
      END
     return;
    END

   if (Memo("OUT"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"DBB")!=0) WrError(1350);
     else if (strcasecmp(ArgStr[2],"A")!=0) WrError(1350);
     else
      BEGIN
       BAsmCode[0]=0x02; CodeLen=1; ChkUPI();
      END
     return;
    END

   if (Memo("OUTL"))
    BEGIN
     NLS_UpString(ArgStr[1]);
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       NLS_UpString(ArgStr[1]);
       if (strcasecmp(ArgStr[2],"A")!=0) WrError(1350);
       else if (strcmp(ArgStr[1],"BUS")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x02;
         ChkN802X(); ChkExt(); ChkNUPI();
        END
       else if (strcmp(ArgStr[1],"P0")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x90;
        END
       else if ((strcmp(ArgStr[1],"P1")==0) OR (strcmp(ArgStr[1],"P2")==0))
        BEGIN
         CodeLen=1; BAsmCode[0]=0x38+ArgStr[1][1]-'0';
        END
       else WrError(1350);
      END
     return;
    END

   if ((Memo("RET")) OR (Memo("RETR")))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x83;
       if (strlen(OpPart)==4)
        BEGIN
         BAsmCode[0]+=0x10; ChkN802X();
        END
      END
     return;
    END

   if (Memo("SEL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU==CPU8021) WrError(1500);
     else
      BEGIN
       OK=False; NLS_UpString(ArgStr[1]);
       for (z=0; z<SelOrderCnt; z++)
       if (strcmp(ArgStr[1],SelOrders[z].Name)==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=SelOrders[z].Code; OK=True;
         if ((SelOrders[z].Is22) AND (MomCPU!=CPU8022))
          BEGIN
           CodeLen=0; WrError(1500);
          END
         if (SelOrders[z].IsNUPI) ChkNUPI();
        END
       if (NOT OK) WrError(1350);
      END
     return;
    END

   if (Memo("STOP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"TCNT")!=0) WrError(1350);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x65;
      END
     return;
    END

   if (Memo("STRT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       NLS_UpString(ArgStr[1]);
       if (strcmp(ArgStr[1],"CNT")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x45;
        END
       else if (strcmp(ArgStr[1],"T")==0)
        BEGIN
         CodeLen=1; BAsmCode[0]=0x55;
        END
       else WrError(1350);
      END
     return;
    END

   if (Memo("XCH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (strcasecmp(ArgStr[2],"A")==0)
        BEGIN
         strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"A",255);
        END
       if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
       else
        BEGIN
         DecodeAdr(ArgStr[2]);
         switch (AdrMode)
          BEGIN
           case ModReg:
            CodeLen=1; BAsmCode[0]=0x28+AdrVal;
            break;
           case ModInd:
            CodeLen=1; BAsmCode[0]=0x20+AdrVal;
            break;
           default:
            WrError(1350);
          END
        END
      END
     return;
    END

   if (Memo("XCHD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (strcasecmp(ArgStr[2],"A")==0)
        BEGIN
         strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"A",255);
        END
       if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
       else
        BEGIN
         DecodeAdr(ArgStr[2]);
         if (AdrMode!=ModInd) WrError(1350);
         else
          BEGIN
           CodeLen=1; BAsmCode[0]=0x30+AdrVal;
          END
        END
      END
     return;
    END

   if (Memo("RAD"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (MomCPU!=CPU8022) WrError(1500);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x80;
      END
     return;
    END

   if (Memo("RETI"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (MomCPU!=CPU8022) WrError(1500);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x93;
      END
     return;
    END

   if (Memo("IDL"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if ((MomCPU!=CPU80C39) AND (MomCPU!=CPU80C48)) WrError(1500);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x01;
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_48(void)
BEGIN
   return False;
END

        static void SwitchFrom_48(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_48(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x21; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegIData)|(1<<SegXData);
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
   switch (MomCPU-CPU8021)
    BEGIN
     case D_CPU8041: SegLimits[SegCode] = 0x3ff; break;
     case D_CPU8042: SegLimits[SegCode] = 0x7ff; break;
     default       : SegLimits[SegCode] = 0xfff; break;
    END
   Grans[SegIData]=1; ListGrans[SegIData]=1; SegInits[SegIData]=0x20;
   SegLimits[SegIData] = 0xff;
   Grans[SegXData]=1; ListGrans[SegXData]=1; SegInits[SegXData]=0;
   SegLimits[SegXData] = 0xff;

   MakeCode=MakeCode_48; IsDef=IsDef_48;
   SwitchFrom=SwitchFrom_48; InitFields();
END

        void code48_init(void)
BEGIN
   CPU8021 =AddCPU("8021" ,SwitchTo_48);
   CPU8022 =AddCPU("8022" ,SwitchTo_48);
   CPU8039 =AddCPU("8039" ,SwitchTo_48);
   CPU8048 =AddCPU("8048" ,SwitchTo_48);
   CPU80C39=AddCPU("80C39",SwitchTo_48);
   CPU80C48=AddCPU("80C48",SwitchTo_48);
   CPU8041 =AddCPU("8041" ,SwitchTo_48);
   CPU8042 =AddCPU("8042" ,SwitchTo_48);
END
