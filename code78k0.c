/* code78k0.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 78K0-Familie                                                */
/*                                                                           */
/* Historie: 1.12.1996 Grundsteinlegung                                      */
/*           3. 1.1999 ChkPC-Anpassung                                       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"


#define ModNone (-1)
#define ModReg8 0
#define MModReg8 (1 << ModReg8)
#define ModReg16 1
#define MModReg16 (1 << ModReg16)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModShort 3
#define MModShort (1 << ModShort)
#define ModSFR 4
#define MModSFR (1 << ModSFR)
#define ModAbs 5
#define MModAbs (1 << ModAbs)
#define ModIReg 6
#define MModIReg (1 << ModIReg)
#define ModIndex 7
#define MModIndex (1 << ModIndex)
#define ModDisp 8
#define MModDisp (1 << ModDisp)

#define AccReg 1
#define AccReg16 0

#define FixedOrderCount 11
#define AriOrderCount 8
#define Ari16OrderCount 3
#define ShiftOrderCount 4
#define Bit2OrderCount 3
#define RelOrderCount 4
#define BRelOrderCount 3

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

static FixedOrder *FixedOrders;
static char **AriOrders;
static char **Ari16Orders;
static char **ShiftOrders;
static char **Bit2Orders;
static char **RelOrders;
static char **BRelOrders;

static Byte OpSize,AdrPart;
static Byte AdrVals[2];
static ShortInt AdrMode;

static CPUVar CPU78070;


/*-------------------------------------------------------------------------*/
/* dynamische Codetabellenverwaltung */

   	static void AddFixed(char *NewName, Word NewCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NewName;
   FixedOrders[InstrZ++].Code=NewCode;
END

   	static void AddAri(char *NewName)
BEGIN
   if (InstrZ>=AriOrderCount) exit(255);
   AriOrders[InstrZ++]=NewName;
END

   	static void AddAri16(char *NewName)
BEGIN
   if (InstrZ>=Ari16OrderCount) exit(255);
   Ari16Orders[InstrZ++]=NewName;
END

   	static void AddShift(char *NewName)
BEGIN
   if (InstrZ>=ShiftOrderCount) exit(255);
   ShiftOrders[InstrZ++]=NewName;
END

   	static void AddBit2(char *NewName)
BEGIN
   if (InstrZ>=Bit2OrderCount) exit(255);
   Bit2Orders[InstrZ++]=NewName;
END

   	static void AddRel(char *NewName)
BEGIN
   if (InstrZ>=RelOrderCount) exit(255);
   RelOrders[InstrZ++]=NewName;
END

   	static void AddBRel(char *NewName)
BEGIN
   if (InstrZ>=BRelOrderCount) exit(255);
   BRelOrders[InstrZ++]=NewName;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("BRK"  ,0x00bf);
   AddFixed("RET"  ,0x00af);
   AddFixed("RETB" ,0x009f);
   AddFixed("RETI" ,0x008f);
   AddFixed("HALT" ,0x7110);
   AddFixed("STOP" ,0x7100);
   AddFixed("NOP"  ,0x0000);
   AddFixed("EI"   ,0x7a1e);
   AddFixed("DI"   ,0x7b1e);
   AddFixed("ADJBA",0x6180);
   AddFixed("ADJBS",0x6190);

   AriOrders=(char **) malloc(sizeof(char *)*AriOrderCount); InstrZ=0;
   AddAri("ADD" ); AddAri("SUB" ); AddAri("ADDC"); AddAri("SUBC");
   AddAri("CMP" ); AddAri("AND" ); AddAri("OR"  ); AddAri("XOR" );

   Ari16Orders=(char **) malloc(sizeof(char *)*Ari16OrderCount); InstrZ=0;
   AddAri16("ADDW"); AddAri16("SUBW"); AddAri16("CMPW");

   ShiftOrders=(char **) malloc(sizeof(char *)*ShiftOrderCount); InstrZ=0;
   AddShift("ROR"); AddShift("RORC"); AddShift("ROL"); AddShift("ROLC");

   Bit2Orders=(char **) malloc(sizeof(char *)*Bit2OrderCount); InstrZ=0;
   AddBit2("AND1"); AddBit2("OR1"); AddBit2("XOR1");

   RelOrders=(char **) malloc(sizeof(char *)*RelOrderCount); InstrZ=0;
   AddRel("BC"); AddRel("BNC"); AddRel("BZ"); AddRel("BNZ");

   BRelOrders=(char **) malloc(sizeof(char *)*BRelOrderCount); InstrZ=0;
   AddBRel("BTCLR"); AddBRel("BT"); AddBRel("BF");
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(AriOrders);
   free(Ari16Orders);
   free(ShiftOrders);
   free(Bit2Orders);
   free(RelOrders);
   free(BRelOrders);
END

/*-------------------------------------------------------------------------*/
/* Adressausdruck parsen */

	static void ChkAdr(Word Mask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     WrError(1350);
     AdrMode=ModNone; AdrCnt=0;
    END
END

	static void DecodeAdr(char *Asc, Word Mask)
BEGIN
   static char *RegNames[8]={"X","A","C","B","E","D","L","H"};

   Word AdrWord;
   int z;
   Boolean OK,LongFlag;

   AdrMode=ModNone; AdrCnt=0;

   /* Register */

   for (z=0; z<8; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      AdrMode=ModReg8; AdrPart=z; ChkAdr(Mask); return;
     END

   if (toupper(*Asc)=='R')
    if ((strlen(Asc)==2) AND (Asc[1]>='0') AND (Asc[1]<='7'))
     BEGIN
      AdrMode=ModReg8; AdrPart=Asc[1]-'0'; ChkAdr(Mask); return;
     END
    else if ((strlen(Asc)==3) AND (toupper(Asc[1])=='P') AND (Asc[2]>='0') AND (Asc[2]<='3'))
     BEGIN
      AdrMode=ModReg16; AdrPart=Asc[2]-'0'; ChkAdr(Mask); return;
     END

   if (strlen(Asc)==2)
    for (z=0; z<4; z++)
     if ((toupper(*Asc)==*RegNames[(z<<1)+1]) AND (toupper(Asc[1])==*RegNames[z<<1]))
      BEGIN
       AdrMode=ModReg16; AdrPart=z; ChkAdr(Mask); return;
      END

   /* immediate */

   if (*Asc=='#')
    BEGIN
     switch (OpSize)
      BEGIN
       case 0: 
        AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
        break;
       case 1:
        AdrWord=EvalIntExpression(Asc+1,Int16,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
	 END
        break;
      END
     if (OK)
      BEGIN
       AdrMode=ModImm; AdrCnt=OpSize+1;
      END
     ChkAdr(Mask); return;
    END

   /* indirekt */

   if ((*Asc=='[') AND (Asc[strlen(Asc)-1]==']'))
    BEGIN
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';

     if ((strcasecmp(Asc,"DE")==0) OR (strcasecmp(Asc,"RP2")==0))
      BEGIN
       AdrMode=ModIReg; AdrPart=0;
      END
     else if ((strncasecmp(Asc,"HL",2)!=0) AND (strncasecmp(Asc,"RP3",3)!=0)) WrXError(1445,Asc);
     else
      BEGIN
       strcpy(Asc,Asc+2); if (*Asc=='3') strcpy(Asc,Asc+1);
       if ((strcasecmp(Asc,"+B")==0) OR (strcasecmp(Asc,"+R3")==0))
        BEGIN
         AdrMode=ModIndex; AdrPart=1;
        END
       else if ((strcasecmp(Asc,"+C")==0) OR (strcasecmp(Asc,"+R2")==0))
        BEGIN
         AdrMode=ModIndex; AdrPart=0;
        END
       else
        BEGIN
         AdrVals[0]=EvalIntExpression(Asc,UInt8,&OK);
         if (OK)
          if (AdrVals[0]==0)
           BEGIN
            AdrMode=ModIReg; AdrPart=1;
           END
          else
           BEGIN
            AdrMode=ModDisp; AdrCnt=1;
           END;
        END
      END

     ChkAdr(Mask); return;
    END

   /* erzwungen lang ? */

   if (*Asc=='!')
    BEGIN
     LongFlag=True; strcpy(Asc,Asc+1);
    END
   else LongFlag=False;

   /* -->absolut */

   FirstPassUnknown=True;
   AdrWord=EvalIntExpression(Asc,UInt16,&OK);
   if (FirstPassUnknown)
    BEGIN
     AdrWord&=0xffffe;
     if ((Mask & MModAbs)==0)
      AdrWord=(AdrWord | 0xff00) & 0xff1f;
    END
   if (OK)
    if ((NOT LongFlag) AND ((Mask & MModShort)!=0) AND (AdrWord>=0xfe20) AND (AdrWord<=0xff1f))
     BEGIN
      AdrMode=ModShort; AdrCnt=1; AdrVals[0]=Lo(AdrWord);
     END
    else if ((NOT LongFlag) AND ((Mask & MModSFR)!=0) AND (((AdrWord>=0xff00) AND (AdrWord<=0xffcf)) OR (AdrWord>=0xffe0)))
     BEGIN
      AdrMode=ModSFR; AdrCnt=1; AdrVals[0]=Lo(AdrWord);
     END
    else
     BEGIN
      AdrMode=ModAbs; AdrCnt=2; AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
     END

   ChkAdr(Mask);
END

	static void ChkEven(void)
BEGIN
   if ((AdrMode==ModAbs) OR (AdrMode==ModShort) OR (AdrMode==ModSFR))
    if ((AdrVals[0]&1)==1) WrError(180);
END

	static Boolean DecodeBitAdr(char *Asc, Byte *Erg)
BEGIN
   char *p;
   Boolean OK;

   p=RQuotPos(Asc,'.');
   if (p==Nil)
    BEGIN
     WrError(1510); return False;
    END

   *p='\0';
   *Erg=EvalIntExpression(p+1,UInt3,&OK) << 4;
   if (NOT OK) return False;

   DecodeAdr(Asc,MModShort+MModSFR+MModIReg+MModReg8);
   switch (AdrMode)
    BEGIN
     case ModReg8:
      if (AdrPart!=AccReg)
       BEGIN
        WrError(1350); return False;
       END
      else
       BEGIN
        *Erg+=0x88; return True;
       END
     case ModShort:
      return True;
     case ModSFR:
      *Erg+=0x08; return True;
     case ModIReg:
      if (AdrPart==0)
       BEGIN
        WrError(1350); return False;
       END
      else
       BEGIN
        *Erg+=0x80; return True;
       END
     default:
      return False;
    END
END

/*-------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        static void MakeCode_78K0(void)
BEGIN
   int z;
   Byte HReg;
   Word AdrWord;
   Integer AdrInt;
   Boolean OK;

   CodeLen=0; DontPrint=False; OpSize=0;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (Hi(FixedOrders[z].Code)==0) CodeLen=1;
      else
       BEGIN
        BAsmCode[0]=Hi(FixedOrders[z].Code); CodeLen=2;
       END
      BAsmCode[CodeLen-1]=Lo(FixedOrders[z].Code);
      return;
     END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModShort+MModSFR+MModAbs+MModIReg+MModIndex+MModDisp);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModImm+MModReg8+((HReg==AccReg)?MModShort+MModSFR+MModAbs+MModIReg+MModIndex+MModDisp:0));
          switch (AdrMode)
           BEGIN
            case ModReg8:
             if ((HReg==AccReg)==(AdrPart==AccReg)) WrError(1350);
             else if (HReg==AccReg)
              BEGIN
               CodeLen=1; BAsmCode[0]=0x60+AdrPart;
              END
             else
              BEGIN
               CodeLen=1; BAsmCode[0]=0x70+HReg;
              END
             break;
            case ModImm:
             CodeLen=2; BAsmCode[0]=0xa0+HReg; BAsmCode[1]=AdrVals[0];
             break;
            case ModShort:
             CodeLen=2; BAsmCode[0]=0xf0; BAsmCode[1]=AdrVals[0];
             break;
            case ModSFR:
             CodeLen=2; BAsmCode[0]=0xf4; BAsmCode[1]=AdrVals[0];
             break;
            case ModAbs:
             CodeLen=3; BAsmCode[0]=0xfe; memcpy(BAsmCode+1,AdrVals,AdrCnt);
             break;
            case ModIReg:
             CodeLen=1; BAsmCode[0]=0x85+(AdrPart << 1);
             break;
            case ModIndex:
             CodeLen=1; BAsmCode[0]=0xaa+AdrPart;
             break;
            case ModDisp:
             CodeLen=2; BAsmCode[0]=0xae; BAsmCode[1]=AdrVals[0];
             break;
           END
          break;
         case ModShort:
          BAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[2],MModReg8+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg8:
             if (AdrPart!=AccReg) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xf2; CodeLen=2;
              END
             break;
            case ModImm:
             BAsmCode[0]=0x11; BAsmCode[2]=AdrVals[0]; CodeLen=3;
             break;
           END
          break;
         case ModSFR:
          BAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[2],MModReg8+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg8:
             if (AdrPart!=AccReg) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xf6; CodeLen=2;
              END
             break;
            case ModImm:
             BAsmCode[0]=0x13; BAsmCode[2]=AdrVals[0]; CodeLen=3;
             break;
           END
          break;
         case ModAbs:
          memcpy(BAsmCode+1,AdrVals,2);
          DecodeAdr(ArgStr[2],MModReg8);
          if (AdrMode==ModReg8)
           if (AdrPart!=AccReg) WrError(1350);
           else
            BEGIN
             BAsmCode[0]=0x9e; CodeLen=3;
            END
          break;
         case ModIReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg8);
          if (AdrMode==ModReg8)
           if (AdrPart!=AccReg) WrError(1350);
           else
            BEGIN
             BAsmCode[0]=0x95+(AdrPart << 1); CodeLen=1;
            END
          break;
         case ModIndex:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg8);
          if (AdrMode==ModReg8)
           if (AdrPart!=AccReg) WrError(1350);
           else
            BEGIN
             BAsmCode[0]=0xba+HReg; CodeLen=1;
            END
          break;
         case ModDisp:
          BAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[2],MModReg8);
          if (AdrMode==ModReg8)
           if (AdrPart!=AccReg) WrError(1350);
           else
            BEGIN
             BAsmCode[0]=0xbe; CodeLen=2;
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
       if ((strcasecmp(ArgStr[2],"A")==0) OR (strcasecmp(ArgStr[2],"RP1")==0))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
        END
       DecodeAdr(ArgStr[1],MModReg8);
       if (AdrMode!=ModNone)
        if (AdrPart!=AccReg) WrError(1350);
	else
	 BEGIN
          DecodeAdr(ArgStr[2],MModReg8+MModShort+MModSFR+MModAbs+MModIReg+MModIndex+MModDisp);
          switch (AdrMode)
           BEGIN
            case ModReg8:
             if (AdrPart==AccReg) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0x30+AdrPart; CodeLen=1;
              END
             break;
            case ModShort:
             BAsmCode[0]=0x83; BAsmCode[1]=AdrVals[0]; CodeLen=2;
             break;
            case ModSFR:
             BAsmCode[0]=0x93; BAsmCode[1]=AdrVals[0]; CodeLen=2;
             break;
            case ModAbs:
             BAsmCode[0]=0xce; memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=3;
             break;
            case ModIReg:
             BAsmCode[0]=0x05+(AdrPart << 1); CodeLen=1;
             break;
            case ModIndex:
             BAsmCode[0]=0x31; BAsmCode[1]=0x8a+AdrPart; CodeLen=2;
             break;
            case ModDisp:
             BAsmCode[0]=0xde; BAsmCode[1]=AdrVals[0]; CodeLen=2;
             break;
           END
	 END
      END
     return;
    END

   if (Memo("MOVW"))
    BEGIN
     OpSize=1;
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16+MModShort+MModSFR+MModAbs);
       switch (AdrMode)
        BEGIN
         case ModReg16:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg16+MModImm+((HReg==AccReg16)?MModShort+MModSFR+MModAbs:0));
          switch (AdrMode)
           BEGIN
            case ModReg16:
             if ((HReg==AccReg16)==(AdrPart==AccReg16)) WrError(1350);
             else if (HReg==AccReg16)
              BEGIN
               BAsmCode[0]=0xc0+(AdrPart << 1); CodeLen=1;
              END
             else
              BEGIN
               BAsmCode[0]=0xd0+(HReg << 1); CodeLen=1;
              END
             break;
            case ModImm:
             BAsmCode[0]=0x10+(HReg << 1); memcpy(BAsmCode+1,AdrVals,2);
             CodeLen=3;
             break;
            case ModShort:
             BAsmCode[0]=0x89; BAsmCode[1]=AdrVals[0]; CodeLen=2;
             ChkEven();
             break;
            case ModSFR:
             BAsmCode[0]=0xa9; BAsmCode[1]=AdrVals[0]; CodeLen=2;
             ChkEven();
             break;
            case ModAbs:
             BAsmCode[0]=0x02; memcpy(BAsmCode+1,AdrVals,2); CodeLen=3;
             ChkEven();
             break;
           END
          break;
         case ModShort:
          ChkEven();
          BAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[2],MModReg16+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg16:
             if (AdrPart!=AccReg16) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0x99; CodeLen=2;
              END
             break;
            case ModImm:
             BAsmCode[0]=0xee; memcpy(BAsmCode+2,AdrVals,2); CodeLen=4;
             break;
           END
          break;
         case ModSFR:
          ChkEven();
          BAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[2],MModReg16+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg16:
             if (AdrPart!=AccReg16) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xb9; CodeLen=2;
              END
             break;
            case ModImm:
             BAsmCode[0]=0xfe; memcpy(BAsmCode+2,AdrVals,2); CodeLen=4;
             break;
           END
          break;
         case ModAbs:
          ChkEven();
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          DecodeAdr(ArgStr[2],MModReg16);
          if (AdrMode==ModReg16)
           if (AdrPart!=AccReg16) WrError(1350);
           else
            BEGIN
             BAsmCode[0]=0x03; CodeLen=3;
            END
          break;
        END
      END
     return;
    END

   if (Memo("XCHW"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       if (AdrMode==ModReg16)
        BEGIN
         HReg=AdrPart;
         DecodeAdr(ArgStr[2],MModReg16);
         if (AdrMode==ModReg16)
          if ((HReg==AccReg16)==(AdrPart==AccReg16)) WrError(1350);
          else
	   BEGIN
            BAsmCode[0]=(HReg==AccReg16) ? 0xe0+(AdrPart << 1) : 0xe0+(HReg << 1);
            CodeLen=1;
           END
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     z=Ord(Memo("POP"));
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"PSW")==0)
      BEGIN
       BAsmCode[0]=0x22+z; CodeLen=1;
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       if (AdrMode==ModReg16)
        BEGIN
         BAsmCode[0]=0xb1-z+(AdrPart << 1); CodeLen=1;
        END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<AriOrderCount; z++)
    if (Memo(AriOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg8+MModShort);
        switch (AdrMode)
         BEGIN
          case ModReg8:
           HReg=AdrPart;
           DecodeAdr(ArgStr[2],MModReg8+((HReg==AccReg)?(MModImm+MModShort+MModAbs+MModIReg+MModIndex+MModDisp):0));
           switch (AdrMode)
            BEGIN
             case ModReg8:
              if (AdrPart==AccReg)
	       BEGIN
                BAsmCode[0]=0x61; BAsmCode[1]=(z << 4)+HReg; CodeLen=2;
               END
              else if (HReg==AccReg)
               BEGIN
                BAsmCode[0]=0x61; BAsmCode[1]=0x08+(z << 4)+AdrPart; CodeLen=2;
               END
              else WrError(1350);
              break;
             case ModImm:
              BAsmCode[0]=(z << 4)+0x0d; BAsmCode[1]=AdrVals[0]; CodeLen=2;
              break;
             case ModShort:
              BAsmCode[0]=(z << 4)+0x0e; BAsmCode[1]=AdrVals[0]; CodeLen=2;
              break;
             case ModAbs:
              BAsmCode[0]=(z << 4)+0x08; memcpy(BAsmCode+1,AdrVals,2); CodeLen=3;
              break;
             case ModIReg:
              if (AdrPart==0) WrError(1350);
              else
               BEGIN
                BAsmCode[0]=(z << 4)+0x0f; CodeLen=2;
               END
              break;
             case ModIndex:
              BAsmCode[0]=0x31; BAsmCode[1]=(z << 4)+0x0a+AdrPart; CodeLen=2;
              break;
             case ModDisp:
              BAsmCode[0]=(z << 4)+0x09; BAsmCode[1]=AdrVals[0]; CodeLen=2;
              break;
            END
           break;
          case ModShort:
           BAsmCode[1]=AdrVals[0];
           DecodeAdr(ArgStr[2],MModImm);
           if (AdrMode==ModImm)
            BEGIN
             BAsmCode[0]=(z << 4)+0x88; BAsmCode[2]=AdrVals[0]; CodeLen=3;
            END
           break;
         END
       END
      return;
     END

   for (z=0; z<Ari16OrderCount; z++)
    if (Memo(Ari16Orders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        OpSize=1;
        DecodeAdr(ArgStr[1],MModReg16);
        if (AdrMode==ModReg16)
         BEGIN
          DecodeAdr(ArgStr[2],MModImm);
          if (AdrMode==ModImm)
           BEGIN
            BAsmCode[0]=0xca+(z << 4); memcpy(BAsmCode+1,AdrVals,2); CodeLen=3;
           END
         END
       END
      return;
     END

   if (Memo("MULU"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8);
       if (AdrMode==ModReg8)
        if (AdrPart!=0) WrError(1350);
        else
         BEGIN
          BAsmCode[0]=0x31; BAsmCode[1]=0x88; CodeLen=2;
         END
      END
     return;
    END

   if (Memo("DIVUW"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8);
       if (AdrMode==ModReg8)
        if (AdrPart!=2) WrError(1350);
        else
         BEGIN
          BAsmCode[0]=0x31; BAsmCode[1]=0x82; CodeLen=2;
         END
      END
     return;
    END

   if ((Memo("INC")) OR (Memo("DEC")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       z=Ord(Memo("DEC")) << 4;
       DecodeAdr(ArgStr[1],MModReg8+MModShort);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          BAsmCode[0]=0x40+AdrPart+z; CodeLen=1;
          break;
         case ModShort:
          BAsmCode[0]=0x81+z; BAsmCode[1]=AdrVals[0]; CodeLen=2;
          break;
        END
      END
     return;
    END

   if ((Memo("INCW")) OR (Memo("DECW")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       if (AdrMode==ModReg16)
        BEGIN
         BAsmCode[0]=0x80+(Ord(Memo("DECW")) << 4)+(AdrPart << 1);
         CodeLen=1;
        END
      END
     return;
    END

   for (z=0; z<ShiftOrderCount; z++)
    if (Memo(ShiftOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg8);
        if (AdrMode==ModReg8)
         if (AdrPart!=AccReg) WrError(1350);
         else
          BEGIN
           HReg=EvalIntExpression(ArgStr[2],UInt1,&OK);
           if (OK)
            if (HReg!=1) WrError(1315);
            else
             BEGIN
              BAsmCode[0]=0x24+z; CodeLen=1;
             END
          END
       END
      return;
     END

   if ((Memo("ROL4")) OR (Memo("ROR4")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       if (AdrMode==ModIReg)
        if (AdrPart==0) WrError(1350);
        else
         BEGIN
          BAsmCode[0]=0x31; BAsmCode[1]=0x80+(Ord(Memo("ROR4")) << 4);
	  CodeLen=2;
	 END
      END
     return;
    END

   /* Bitoperationen */

   if (Memo("MOV1"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (strcasecmp(ArgStr[2],"CY")==0)
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         z=1;
        END
       else z=4;
       if (strcasecmp(ArgStr[1],"CY")!=0) WrError(1110);
       else if (DecodeBitAdr(ArgStr[2],&HReg))
        BEGIN
         BAsmCode[0]=0x61+(Ord((HReg & 0x88)!=0x88) << 4);
         BAsmCode[1]=z+HReg;
         memcpy(BAsmCode+2,AdrVals,AdrCnt);
         CodeLen=2+AdrCnt;
        END
      END
     return;
    END

   for (z=0; z<Bit2OrderCount; z++)
    if (Memo(Bit2Orders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (strcasecmp(ArgStr[1],"CY")!=0) WrError(1110);
      else if (DecodeBitAdr(ArgStr[2],&HReg))
       BEGIN
        BAsmCode[0]=0x61+(Ord((HReg & 0x88)!=0x88) << 4);
        BAsmCode[1]=z+5+HReg;
        memcpy(BAsmCode+2,AdrVals,AdrCnt);
        CodeLen=2+AdrCnt;
       END
      return;
     END

   if ((Memo("SET1")) OR (Memo("CLR1")))
    BEGIN
     z=Ord(Memo("CLR1"));
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CY")==0)
      BEGIN
       BAsmCode[0]=0x20+z; CodeLen=1;
      END
     else if (DecodeBitAdr(ArgStr[1],&HReg))
      if ((HReg & 0x88)==0)
       BEGIN
        BAsmCode[0]=0x0a+z+(HReg & 0x70);
        BAsmCode[1]=AdrVals[0];
        CodeLen=2;
       END
      else
       BEGIN
        BAsmCode[0]=0x61+(Ord((HReg & 0x88)!=0x88) << 4);
        BAsmCode[1]=HReg+2+z;
        memcpy(BAsmCode+2,AdrVals,AdrCnt);
        CodeLen=2+AdrCnt;
       END
     return;
    END

   if (Memo("NOT1"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CY")!=0) WrError(1350);
     else
      BEGIN
       BAsmCode[0]=0x01;
       CodeLen=1;
      END
     return;
    END

   /* Spruenge */

   if (Memo("CALL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAbs);
       if (AdrMode==ModAbs)
        BEGIN
         BAsmCode[0]=0x9a; memcpy(BAsmCode+1,AdrVals,2); CodeLen=3;
        END
      END
     return;
    END

   if (Memo("CALLF"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       if (*ArgStr[1]=='!') strcpy(ArgStr[1],ArgStr[1]+1);
       AdrWord=EvalIntExpression(ArgStr[1],UInt11,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=0x0c+(Hi(AdrWord) << 4);
         BAsmCode[1]=Lo(AdrWord);
         CodeLen=2;
        END
      END
     return;
    END

   if (Memo("CALLT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if ((*ArgStr[1]!='[') OR (ArgStr[1][strlen(ArgStr[1])-1]!=']')) WrError(1350);
     else
      BEGIN
       FirstPassUnknown=False;
       ArgStr[1][strlen(ArgStr[1])-1]='\0';
       AdrWord=EvalIntExpression(ArgStr[1]+1,UInt6,&OK);
       if (FirstPassUnknown) AdrWord&=0xfffe;
       if (OK)
        if (Odd(AdrWord)) WrError(1325);
        else
         BEGIN
          BAsmCode[0]=0xc1+(AdrWord & 0x3e);
          CodeLen=1;
         END
      END
     return;
    END

   if (Memo("BR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if ((strcasecmp(ArgStr[1],"AX")==0) OR (strcasecmp(ArgStr[1],"RP0")==0))
      BEGIN
       BAsmCode[0]=0x31; BAsmCode[1]=0x98; CodeLen=2;
      END
     else
      BEGIN
       if (*ArgStr[1]=='!') 
        BEGIN
         strcpy(ArgStr[1],ArgStr[1]+1); HReg=1;
        END
       else if (*ArgStr[1]=='$')
        BEGIN
         strcpy(ArgStr[1],ArgStr[1]+1); HReg=2;
        END
       else HReg=0;
       AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        BEGIN
         if (HReg==0)
          BEGIN
           AdrInt=AdrWord-(EProgCounter()-2);
           HReg=((AdrInt>=-128) AND (AdrInt<127)) ? 2 : 1;
          END
         switch (HReg)
          BEGIN
           case 1:
            BAsmCode[0]=0x9b; BAsmCode[1]=Lo(AdrWord); BAsmCode[2]=Hi(AdrWord);
            CodeLen=3;
            break;
           case 2:
            AdrInt=AdrWord-(EProgCounter()+2);
            if (((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable)) WrError(1370);
            else
             BEGIN
              BAsmCode[0]=0xfa; BAsmCode[1]=AdrInt & 0xff; CodeLen=2;
             END
            break;
          END
        END
      END
     return;
    END

   for (z=0; z<RelOrderCount; z++)
    if (Memo(RelOrders[z]))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        if (*ArgStr[1]=='$') strcpy(ArgStr[1],ArgStr[1]+1);
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+2);
        if (OK)
         if (((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable)) WrError(1370);
         else
          BEGIN
           BAsmCode[0]=0x8b+(z << 4); BAsmCode[1]=AdrInt & 0xff;
           CodeLen=2;
          END
       END
      return;
     END

   for (z=0; z<BRelOrderCount; z++)
    if (Memo(BRelOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (DecodeBitAdr(ArgStr[1],&HReg))
       BEGIN
        if ((z==1) AND ((HReg & 0x88)==0))
         BEGIN
          BAsmCode[0]=0x8c+HReg; BAsmCode[1]=AdrVals[0]; HReg=2;
         END
        else
         BEGIN
          BAsmCode[0]=0x31;
          switch (HReg & 0x88)
           BEGIN
            case 0x00:
             BAsmCode[1]=0x00; break;
            case 0x08:
             BAsmCode[1]=0x04; break;
            case 0x80:
             BAsmCode[1]=0x84; break;
            case 0x88:
             BAsmCode[1]=0x0c; break;
           END
          BAsmCode[1]+=(HReg & 0x70)+z+1;
          BAsmCode[2]=AdrVals[0];
          HReg=2+AdrCnt;
         END
        if (*ArgStr[2]=='$') strcpy(ArgStr[2],ArgStr[2]+1);
        AdrInt=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+HReg+1);
        if (OK)
         if (((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable)) WrError(1370);
         else
          BEGIN
           BAsmCode[HReg]=AdrInt & 0xff;
           CodeLen=HReg+1;
          END
       END
      return;
     END

   if (Memo("DBNZ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModShort);
       if ((AdrMode==ModReg8) AND ((AdrPart & 6)!=2)) WrError(1350);
       else if (AdrMode!=ModNone)
        BEGIN
         BAsmCode[0]=(AdrMode==ModReg8) ? 0x88+AdrPart : 0x04;
         BAsmCode[1]=AdrVals[0];
         if (*ArgStr[2]=='$') strcpy(ArgStr[2],ArgStr[2]+1);
         AdrInt=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+AdrCnt+2);
         if (OK)
          if (((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable)) WrError(1370);
          else
           BEGIN
            BAsmCode[AdrCnt+1]=AdrInt & 0xff;
            CodeLen=AdrCnt+2;
           END
	END
      END
     return;
    END

   /* Steueranweisungen */

   if (Memo("SEL"))
    BEGIN
     if (ArgCnt!=1) WrError(1350);
     else if ((strlen(ArgStr[1])!=3) OR (strncasecmp(ArgStr[1],"RB",2)!=0)) WrError(1350);
     else
      BEGIN
       HReg=ArgStr[1][2]-'0';
       if (ChkRange(HReg,0,3))
        BEGIN
         BAsmCode[0]=0x61;
         BAsmCode[1]=0xd0+((HReg & 1) << 3)+((HReg & 2) << 4);
         CodeLen=2;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_78K0(void)
BEGIN
   return False;
END

        static void SwitchFrom_78K0(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_78K0(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="PC"; HeaderID=0x7c; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   MakeCode=MakeCode_78K0; IsDef=IsDef_78K0;
   SwitchFrom=SwitchFrom_78K0; InitFields();
END

	void code78k0_init(void)
BEGIN
   CPU78070=AddCPU("78070",SwitchTo_78K0);
END



