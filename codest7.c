/* codest7.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator SGS-Thomson ST7                                             */
/*                                                                           */
/* Historie: 21. 5.1997 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC ersetzt                                        */ 
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codest7.c,v 1.2 2004/05/29 12:04:48 alfred Exp $                     */
/*****************************************************************************
 * $Log: codest7.c,v $
 * Revision 1.2  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "nls.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "codevars.h"

#include "codest7.h"

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Boolean MayImm;
         } AriOrder;


#define FixedOrderCnt 11
#define AriOrderCnt 8
#define RMWOrderCnt 13
#define RelOrderCnt 20


#define ModNone (-1)
#define ModImm 0
#define MModImm (1l << ModImm)
#define ModAbs8 1
#define MModAbs8 (1l << ModAbs8)
#define ModAbs16 2
#define MModAbs16 (1l << ModAbs16)
#define ModIX 3
#define MModIX (1l << ModIX)
#define ModIX8 4
#define MModIX8 (1l << ModIX8)
#define ModIX16 5
#define MModIX16 (1l << ModIX16)
#define ModIY 6
#define MModIY (1l << ModIY)
#define ModIY8 7
#define MModIY8 (1l << ModIY8)
#define ModIY16 8
#define MModIY16 (1l << ModIY16)
#define ModIAbs8 9
#define MModIAbs8 (1l << ModIAbs8)
#define ModIAbs16 10
#define MModIAbs16 (1l << ModIAbs16)
#define ModIXAbs8 11
#define MModIXAbs8 (1l << ModIXAbs8)
#define ModIXAbs16 12
#define MModIXAbs16 (1l << ModIXAbs16)
#define ModIYAbs8 13
#define MModIYAbs8 (1l << ModIYAbs8)
#define ModIYAbs16 14
#define MModIYAbs16 (1l << ModIYAbs16)
#define ModA 15
#define MModA (1l << ModA)
#define ModX 16
#define MModX (1l << ModX)
#define ModY 17
#define MModY (1l << ModY)
#define ModS 18
#define MModS (1l << ModS)
#define ModCCR 19
#define MModCCR (1l << ModCCR)


static CPUVar CPUST7;

static FixedOrder *FixedOrders;
static AriOrder *AriOrders;
static FixedOrder *RMWOrders;
static FixedOrder *RelOrders;

static ShortInt AdrType;
static Byte AdrPart,OpSize,PrefixCnt;
static Byte AdrVals[3];

/*--------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddAri(char *NName, Byte NCode, Boolean NMay)
BEGIN
   if (InstrZ>=AriOrderCnt) exit(255);
   AriOrders[InstrZ].Name=NName;
   AriOrders[InstrZ].Code=NCode;
   AriOrders[InstrZ++].MayImm=NMay;
END

        static void AddRMW(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=RMWOrderCnt) exit(255);
   RMWOrders[InstrZ].Name=NName;
   RMWOrders[InstrZ++].Code=NCode;
END

        static void AddRel(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   InstrZ=0; FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt);
   AddFixed("HALT" ,0x8e); AddFixed("IRET" ,0x80); AddFixed("NOP"  ,0x9d);
   AddFixed("RCF"  ,0x98); AddFixed("RET"  ,0x81); AddFixed("RIM"  ,0x9a);
   AddFixed("RSP"  ,0x9c); AddFixed("SCF"  ,0x99); AddFixed("SIM"  ,0x9b);
   AddFixed("TRAP" ,0x83); AddFixed("WFI"  ,0x8f);

   InstrZ=0; AriOrders=(AriOrder *) malloc(sizeof(AriOrder)*AriOrderCnt);
   AddAri("ADC" ,0x09,True ); AddAri("ADD" ,0x0b,True ); AddAri("AND" ,0x04,True );
   AddAri("BCP" ,0x05,True ); AddAri("OR"  ,0x0a,True ); AddAri("SBC" ,0x02,True );
   AddAri("SUB" ,0x00,True ); AddAri("XOR" ,0x08,True );

   InstrZ=0; RMWOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RMWOrderCnt);
   AddRMW("CLR" ,0x0f); AddRMW("CPL" ,0x03); AddRMW("DEC" ,0x0a);
   AddRMW("INC" ,0x0c); AddRMW("NEG" ,0x00); AddRMW("RLC" ,0x09);
   AddRMW("RRC" ,0x06); AddRMW("SLA" ,0x08); AddRMW("SLL" ,0x08);
   AddRMW("SRA" ,0x07); AddRMW("SRL" ,0x04); AddRMW("SWAP",0x0e);
   AddRMW("TNZ" ,0x0d);

   InstrZ=0; RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCnt);
   AddRel("CALLR",0xad); AddRel("JRA"  ,0x20); AddRel("JRC"  ,0x25);
   AddRel("JREQ" ,0x27); AddRel("JRF"  ,0x21); AddRel("JRH"  ,0x29);
   AddRel("JRIH" ,0x2f); AddRel("JRIL" ,0x2e); AddRel("JRM"  ,0x2d);
   AddRel("JRMI" ,0x2b); AddRel("JRNC" ,0x24); AddRel("JRNE" ,0x26);
   AddRel("JRNH" ,0x28); AddRel("JRNM" ,0x2c); AddRel("JRPL" ,0x2a);
   AddRel("JRT"  ,0x20); AddRel("JRUGE",0x24); AddRel("JRUGT",0x22);
   AddRel("JRULE",0x23); AddRel("JRULT",0x25);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(AriOrders);
   free(RMWOrders);
   free(RelOrders);
END

/*--------------------------------------------------------------------------*/

        static void AddPrefix(Byte Pref)
BEGIN
   BAsmCode[PrefixCnt++]=Pref;
END

        static void DecideSize(LongInt Mask, char *Asc, LongInt Type1, LongInt Type2, Byte Part1, Byte Part2)
BEGIN
   enum {None,I8,I16} Size;
   Word Value;
   Boolean OK;
   int l=strlen(Asc);

   if ((l>=3) AND (Asc[l-2]=='.'))
    BEGIN
     if (toupper(Asc[l-1])=='B')
      BEGIN
       Size=I8; Asc[l-2]='\0'; 
      END
     else if (toupper(Asc[l-1])=='W')
      BEGIN
       Size=I16; Asc[l-2]='\0';
      END
     else Size=None;
    END
   else Size=None;

   if (Size==I8) Value=EvalIntExpression(Asc,UInt8,&OK);
   else Value=EvalIntExpression(Asc,Int16,&OK);

   if (OK)
    BEGIN
     if ((Size==I8) OR (((Mask & (1l << Type1))!=0) AND (Size==None) AND (Hi(Value)==0)))
      BEGIN
       AdrVals[0]=Lo(Value); AdrCnt=1;
       AdrPart=Part1; AdrType=Type1;
      END
     else
      BEGIN
       AdrVals[0]=Hi(Value); AdrVals[1]=Lo(Value); AdrCnt=2;
       AdrPart=Part2; AdrType=Type2;
      END
    END
END

        static void DecideASize(LongInt Mask, char *Asc, LongInt Type1, LongInt Type2, Byte Part1, Byte Part2)
BEGIN
   Boolean I16;
   Boolean OK;
   int l=strlen(Asc);

   if ((l>=3) AND (Asc[l-2]=='.') AND (toupper(Asc[l-1])=='W'))
    BEGIN
     I16=True; Asc[l-2]='\0';
    END
   else if (((Mask & (1l << Type1)))==0) I16=True;
   else I16=False;

   AdrVals[0]=EvalIntExpression(Asc,UInt8,&OK);
   if (OK)
    BEGIN
     AdrCnt=1;
     if (I16)
      BEGIN
       AdrPart=Part2; AdrType=Type2;
      END
     else
      BEGIN
       AdrPart=Part1; AdrType=Type1;
      END
    END
END

        static void ChkAdr(LongInt Mask)
BEGIN
   if ( (AdrType!=ModNone) AND ((Mask & (1l << AdrType))==0) )
    BEGIN
     WrError(1350); AdrType=ModNone; AdrCnt=0;
    END
END

        static void DecodeAdr(char *Asc_O, LongInt Mask)
BEGIN
   Boolean OK,YReg;
   String Asc,Asc2;
   char *p;

   strmaxcpy(Asc,Asc_O,255);

   AdrType=ModNone; AdrCnt=0;

   /* Register ? */

   if (strcasecmp(Asc,"A")==0)
    BEGIN
     AdrType=ModA; ChkAdr(Mask); return;
    END

   if (strcasecmp(Asc,"X")==0)
    BEGIN
     AdrType=ModX; ChkAdr(Mask); return;
    END

   if (strcasecmp(Asc,"Y")==0)
    BEGIN
     AdrType=ModY; AddPrefix(0x90); ChkAdr(Mask); return;
    END

   if (strcasecmp(Asc,"S")==0)
    BEGIN
     AdrType=ModS; ChkAdr(Mask); return;
    END

   if (strcasecmp(Asc,"CC")==0)
    BEGIN
     AdrType=ModCCR; ChkAdr(Mask); return;
    END

   /* immediate ? */

   if (*Asc=='#')
    BEGIN
     AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
     if (OK)
      BEGIN
       AdrType=ModImm; AdrPart=0xa; AdrCnt=1;
      END
     ChkAdr(Mask); return;
    END

   /* speicherindirekt ? */

   if ((*Asc=='[') AND (Asc[strlen(Asc)-1]==']'))
    BEGIN
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
     DecideASize(Mask,Asc,ModIAbs8,ModIAbs16,0xb,0xc);
     if (AdrType!=ModNone) AddPrefix(0x92);
     ChkAdr(Mask); return;
    END

   /* sonstwie indirekt ? */

   if (IsIndirect(Asc))
    BEGIN
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';

     /* ein oder zwei Argumente ? */

     p=QuotPos(Asc,',');
     if (p==Nil)
      BEGIN
       AdrPart=0xf;
       if (strcasecmp(Asc,"X")==0) AdrType=ModIX;
       else if (strcasecmp(Asc,"Y")==0)
        BEGIN
         AdrType=ModIY; AddPrefix(0x90);
        END
       else WrXError(1445,Asc);
       ChkAdr(Mask); return;
      END

     strmaxcpy(Asc2,p+1,255); *p='\0';

     if (strcasecmp(Asc,"X")==0)
      BEGIN
       strmaxcpy(Asc,Asc2,255); YReg=False;
      END
     else if (strcasecmp(Asc2,"X")==0) YReg=False;
     else if (strcasecmp(Asc,"Y")==0)
      BEGIN
       strmaxcpy(Asc,Asc2,255); YReg=True;
      END
     else if (strcasecmp(Asc2,"Y")==0) YReg=True;
     else
      BEGIN
       WrError(1350); return;
      END

     /* speicherindirekt ? */

     if ((*Asc=='[') AND (Asc[strlen(Asc)-1]==']'))
      BEGIN
       strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
       if (YReg)
        BEGIN
         DecideASize(Mask,Asc,ModIYAbs8,ModIYAbs16,0xe,0xd);
         if (AdrType!=ModNone) AddPrefix(0x91);
        END
       else
        BEGIN
         DecideASize(Mask,Asc,ModIXAbs8,ModIXAbs16,0xe,0xd);
         if (AdrType!=ModNone) AddPrefix(0x92);
        END
      END
     else
      BEGIN
       if (YReg) DecideSize(Mask,Asc,ModIY8,ModIY16,0xe,0xd);
       else DecideSize(Mask,Asc,ModIX8,ModIX16,0xe,0xd);
       if ((AdrType!=ModNone) AND (YReg)) AddPrefix(0x90);
      END

     ChkAdr(Mask); return;
    END

   /* dann absolut */

   DecideSize(Mask,Asc,ModAbs8,ModAbs16,0xb,0xc);

   ChkAdr(Mask);
END

/*--------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        static void MakeCode_ST7(void)
BEGIN
   Integer AdrInt;
   int z;
   LongInt Mask;
   Boolean OK;

   CodeLen=0; DontPrint=False; OpSize=1; PrefixCnt=0;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Attribut verarbeiten */

   if (*AttrPart!='\0')
    switch (toupper(*AttrPart))
     BEGIN
      case 'B': OpSize=0; break;
      case 'W': OpSize=1; break;
      case 'L': OpSize=2; break;
      case 'Q': OpSize=3; break;
      case 'S': OpSize=4; break;
      case 'D': OpSize=5; break;
      case 'X': OpSize=6; break;
      case 'P': OpSize=7; break;
      default:
       WrError(1107); return;
     END

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(True)) return;
   if (DecodeMoto16Pseudo(OpSize,True)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        BAsmCode[PrefixCnt]=FixedOrders[z].Code; CodeLen=PrefixCnt+1;
       END
      return;
     END

   /* Datentransfer */

   if (Memo("LD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModA+MModX+MModY+MModS+
                 MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
                 MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                 MModIYAbs8+MModIYAbs16);

       switch (AdrType)
        BEGIN
         case ModA:
          DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
                    MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                    MModIYAbs8+MModIYAbs16+MModX+MModY+MModS);
          switch (AdrType)
           BEGIN
            case ModX:
            case ModY:
             BAsmCode[PrefixCnt]=0x9f; CodeLen=PrefixCnt+1;
             break;
            case ModS:
             BAsmCode[PrefixCnt]=0x9e; CodeLen=PrefixCnt+1;
             break;
            default:
             if (AdrType!=ModNone)
              BEGIN
               BAsmCode[PrefixCnt]=0x06+(AdrPart << 4);
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               CodeLen=PrefixCnt+1+AdrCnt;
              END
           END
          break;
         case ModX:
          DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+
                    MModIX16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                    MModA+MModY+MModS);
          switch (AdrType)
           BEGIN
            case ModA:
             BAsmCode[PrefixCnt]=0x97; CodeLen=PrefixCnt+1;
             break;
            case ModY:
             BAsmCode[0]=0x93; CodeLen=1;
             break;
            case ModS:
             BAsmCode[PrefixCnt]=0x96; CodeLen=PrefixCnt+1;
             break;
            default:
             if (AdrType!=ModNone)
              BEGIN
               BAsmCode[PrefixCnt]=0x0e + (AdrPart << 4); /* ANSI :-O */
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               CodeLen=PrefixCnt+1+AdrCnt;
              END
           END
          break;
         case ModY:
          PrefixCnt=0;
          DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIY+MModIY8+
                    MModIY16+MModIAbs8+MModIAbs16+MModIYAbs8+MModIYAbs16+
                    MModA+MModX+MModS);
          switch (AdrType)
           BEGIN
            case ModA:
             AddPrefix(0x90); BAsmCode[PrefixCnt]=0x97; CodeLen=PrefixCnt+1;
             break;
            case ModX:
             AddPrefix(0x90); BAsmCode[PrefixCnt]=0x93; CodeLen=PrefixCnt+1;
             break;
            case ModS:
             AddPrefix(0x90); BAsmCode[PrefixCnt]=0x96; CodeLen=PrefixCnt+1;
             break;
            default:
             if (AdrType!=ModNone)
              BEGIN
               if (PrefixCnt==0) AddPrefix(0x90);
               if (BAsmCode[0]==0x92) BAsmCode[0]--;
               BAsmCode[PrefixCnt]=0x0e + (AdrPart << 4); /* ANSI :-O */
               memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
               CodeLen=PrefixCnt+1+AdrCnt;
              END
           END
          break;
         case ModS:
          DecodeAdr(ArgStr[2],MModA+MModX+MModY);
          switch (AdrType)
           BEGIN
            case ModA:
             BAsmCode[PrefixCnt]=0x95; CodeLen=PrefixCnt+1;
             break;
            case ModX:
            case ModY:
             BAsmCode[PrefixCnt]=0x94; CodeLen=PrefixCnt+1;
             break;
           END
          break;
         default:
          if (AdrType!=ModNone)
           BEGIN
            PrefixCnt=0; DecodeAdr(ArgStr[2],MModA+MModX+MModY);
            switch (AdrType)
             BEGIN
              case ModA:
               Mask=MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
                    MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                    MModIYAbs8+MModIYAbs16;
               DecodeAdr(ArgStr[1],Mask);
               if (AdrType!=ModNone)
                BEGIN
                 BAsmCode[PrefixCnt]=0x07+(AdrPart << 4);
                 memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
                 CodeLen=PrefixCnt+1+AdrCnt;
                END
               break;
              case ModX:
               DecodeAdr(ArgStr[1],MModAbs8+MModAbs16+MModIX+MModIX8+
                         MModIX16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16);
               if (AdrType!=ModNone)
                BEGIN
                 BAsmCode[PrefixCnt]=0x0f+(AdrPart << 4);
                 memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
                 CodeLen=PrefixCnt+1+AdrCnt;
                END
               break;
              case ModY:
               PrefixCnt=0;
               DecodeAdr(ArgStr[1],MModAbs8+MModAbs16+MModIY+MModIY8+
                         MModIY16+MModIAbs8+MModIAbs16+MModIYAbs8+MModIYAbs16);
               if (AdrType!=ModNone)
                BEGIN
                 if (PrefixCnt==0) AddPrefix(0x90);
                 if (BAsmCode[0]==0x92) BAsmCode[0]--;
                 BAsmCode[PrefixCnt]=0x0f+(AdrPart << 4);
                 memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
                 CodeLen=PrefixCnt+1+AdrCnt;
                END
               break;
             END
           END
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModA+MModX+MModY+MModCCR);
       if (AdrType!=ModNone)
        BEGIN
         switch (AdrType)
          BEGIN
           case ModA: BAsmCode[PrefixCnt]=0x84; break;
           case ModX:
           case ModY: BAsmCode[PrefixCnt]=0x85; break;
           case ModCCR: BAsmCode[PrefixCnt]=0x86; break;
          END
         if (Memo("PUSH")) BAsmCode[PrefixCnt]+=4;
         CodeLen=PrefixCnt+1;
        END
      END
     return;
    END

   /* Arithmetik */

   if (Memo("CP"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModA+MModX+MModY);
       switch (AdrType)
        BEGIN
         case ModA:
          Mask=MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
               MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
               MModIYAbs8+MModIYAbs16;
          DecodeAdr(ArgStr[2],Mask);
          if (AdrType!=ModNone)
           BEGIN
            BAsmCode[PrefixCnt]=0x01+(AdrPart << 4);
            memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
            CodeLen=PrefixCnt+1+AdrCnt;
           END
          break;
         case ModX:
          DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+
                    MModIX16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16);
          if (AdrType!=ModNone)
           BEGIN
            BAsmCode[PrefixCnt]=0x03+(AdrPart << 4);
            memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
            CodeLen=PrefixCnt+1+AdrCnt;
           END
          break;
         case ModY:
          PrefixCnt=0;
          DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIY+MModIY8+
                    MModIY16+MModIAbs8+MModIAbs16+MModIYAbs8+MModIYAbs16);
          if (AdrType!=ModNone)
           BEGIN
            if (PrefixCnt==0) AddPrefix(0x90);
            if (BAsmCode[0]==0x92) BAsmCode[0]--;
            BAsmCode[PrefixCnt]=0x03+(AdrPart << 4);
            memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
            CodeLen=PrefixCnt+1+AdrCnt;
           END
          break;
        END
      END
     return;
    END

   for (z=0; z<AriOrderCnt; z++)
    if (Memo(AriOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModA);
        if (AdrType==ModA)
         BEGIN
          Mask=MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
               MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
               MModIYAbs8+MModIYAbs16;
          if (AriOrders[z].MayImm) Mask+=MModImm;
          DecodeAdr(ArgStr[2],Mask);
          if (AdrType!=ModNone)
           BEGIN
            BAsmCode[PrefixCnt]=AriOrders[z].Code+(AdrPart << 4);
            memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
            CodeLen=PrefixCnt+1+AdrCnt;
           END
         END
       END
      return;
     END

   for (z=0; z<RMWOrderCnt; z++)
    if (Memo(RMWOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModA+MModX+MModY+MModAbs8+MModIX+MModIX8+
                            MModIY+MModIY8+MModIAbs8+MModIXAbs8+MModIYAbs8);
        switch (AdrType)
         BEGIN
          case ModA:
           BAsmCode[PrefixCnt]=0x40+RMWOrders[z].Code; CodeLen=PrefixCnt+1;
           break;
          case ModX:
          case ModY:
           BAsmCode[PrefixCnt]=0x50+RMWOrders[z].Code; CodeLen=PrefixCnt+1;
           break;
          default:
           if (AdrType!=ModNone)
            BEGIN
             BAsmCode[PrefixCnt]=RMWOrders[z].Code+((AdrPart-8) << 4);
             memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
             CodeLen=PrefixCnt+1+AdrCnt;
            END
         END
       END
      return;
     END

   if (Memo("MUL"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModA);
       if (AdrType!=ModNone)
        BEGIN
         DecodeAdr(ArgStr[1],MModX+MModY);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[PrefixCnt]=0x42; CodeLen=PrefixCnt+1;
          END
        END
      END
     return;
    END

   /* Bitbefehle */

   if ((Memo("BRES")) OR (Memo("BSET")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[2]!='#') WrError(1350);
     else
      BEGIN
       z=EvalIntExpression(ArgStr[2]+1,UInt3,&OK);
       if (OK)
        BEGIN
         DecodeAdr(ArgStr[1],MModAbs8+MModIAbs8);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[PrefixCnt]=0x10+Ord(Memo("BRES"))+(z << 1);
           memcpy(BAsmCode+1+PrefixCnt,AdrVals,AdrCnt);
           CodeLen=PrefixCnt+1+AdrCnt;
          END
        END
      END
     return;
    END

   if ((Memo("BTJF")) OR (Memo("BTJT")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[2]!='#') WrError(1350);
     else
      BEGIN
       z=EvalIntExpression(ArgStr[2]+1,UInt3,&OK);
       if (OK)
        BEGIN
         DecodeAdr(ArgStr[1],MModAbs8+MModIAbs8);
         if (AdrType!=ModNone)
          BEGIN
           BAsmCode[PrefixCnt]=0x00+Ord(Memo("BTJF"))+(z << 1);
           memcpy(BAsmCode+1+PrefixCnt,AdrVals,AdrCnt);
           AdrInt=EvalIntExpression(ArgStr[3],UInt16,&OK)-(EProgCounter()+PrefixCnt+1+AdrCnt);
           if (OK)
            BEGIN
             if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
             else
              BEGIN
               BAsmCode[PrefixCnt+1+AdrCnt]=AdrInt & 0xff;
               CodeLen=PrefixCnt+1+AdrCnt+1;
              END
            END
          END
        END
      END
     return;
    END

   /* Spruenge */

   if ((Memo("JP")) OR (Memo("CALL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       Mask=MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
            MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
            MModIYAbs8+MModIYAbs16;
       DecodeAdr(ArgStr[1],Mask);
       if (AdrType!=ModNone)
        BEGIN
         BAsmCode[PrefixCnt]=0x0c+Ord(Memo("CALL"))+(AdrPart << 4);
         memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
         CodeLen=PrefixCnt+1+AdrCnt;
        END
      END
     return;
    END

   for (z=0; z<RelOrderCnt; z++)
    if (Memo(RelOrders[z].Name))
     BEGIN
      if (*AttrPart!='\0') WrError(1100);
      else if (ArgCnt!=1) WrError(1110);
      else if (*ArgStr[1]=='[')
       BEGIN
        DecodeAdr(ArgStr[1],MModIAbs8);
        if (AdrType!=ModNone)
         BEGIN
          BAsmCode[PrefixCnt]=RelOrders[z].Code;
          memcpy(BAsmCode+PrefixCnt+1,AdrVals,AdrCnt);
          CodeLen=PrefixCnt+1+AdrCnt;
         END
       END
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+2);
        if (OK)
         BEGIN
          if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
          else
           BEGIN
            BAsmCode[0]=RelOrders[z].Code; BAsmCode[1]=AdrInt & 0xff;
            CodeLen=2;
           END
         END
       END
      return;
     END

   /* nix gefunden */

   WrXError(1200,OpPart);
END

        static Boolean IsDef_ST7(void)
BEGIN
   return False;
END

        static void SwitchFrom_ST7(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_ST7(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="PC"; HeaderID=0x33; NOPCode=0x9d;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   MakeCode=MakeCode_ST7; IsDef=IsDef_ST7;
   SwitchFrom=SwitchFrom_ST7; InitFields();
   AddMoto16PseudoONOFF();

   SetFlag(&DoPadding,DoPaddingName,False);
END

        void codest7_init(void)
BEGIN
   CPUST7=AddCPU("ST7",SwitchTo_ST7);
END


