/* codez8.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Zilog Z8                                                    */
/*                                                                           */
/* Historie: 8.11.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
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
          Boolean Is16;
         } ALU1Order;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;


#define WorkOfs 0xe0


#define FixedOrderCnt 12

#define ALU2OrderCnt 10

#define ALU1OrderCnt 14

#define CondCnt 20

#define ModNone  (-1)
#define ModWReg   0
#define MModWReg   (1 << ModWReg)
#define ModReg    1
#define MModReg    (1 << ModReg)
#define ModIWReg  2
#define MModIWReg  (1 << ModIWReg)
#define ModIReg   3
#define MModIReg   (1 << ModIReg)
#define ModImm    4
#define MModImm    (1 << ModImm)
#define ModRReg   5
#define MModRReg   (1 << ModRReg)
#define ModIRReg  6
#define MModIRReg  (1 << ModIRReg)
#define ModInd    7
#define MModInd    (1 << ModInd)

static ShortInt AdrType;
static Byte AdrMode,AdrIndex;
static Word AdrWMode;

static FixedOrder *FixedOrders;
static FixedOrder *ALU2Orders;
static ALU1Order *ALU1Orders;
static Condition *Conditions;
static int TrueCond;

static CPUVar CPUZ8601,CPUZ8604,CPUZ8608,CPUZ8630,CPUZ8631;

/*--------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddALU2(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Name=NName;
   ALU2Orders[InstrZ++].Code=NCode;
END

        static void AddALU1(char *NName, Byte NCode, Boolean NIs)
BEGIN
   if (InstrZ>=ALU1OrderCnt) exit(255);
   ALU1Orders[InstrZ].Name=NName;
   ALU1Orders[InstrZ].Is16=NIs;
   ALU1Orders[InstrZ++].Code=NCode;
END

        static void AddCondition(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=CondCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END
   
	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CCF" , 0xef);  AddFixed("DI"  , 0x8f);
   AddFixed("EI"  , 0x9f);  AddFixed("HALT", 0x7f);
   AddFixed("IRET", 0xbf);  AddFixed("NOP" , 0xff);
   AddFixed("RCF" , 0xcf);  AddFixed("RET" , 0xaf);
   AddFixed("SCF" , 0xdf);  AddFixed("STOP", 0x6f);
   AddFixed("WDH" , 0x4f);  AddFixed("WDT" , 0x5f);

   ALU2Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALU2OrderCnt); InstrZ=0;
   AddALU2("ADD" ,0x00);
   AddALU2("ADC" ,0x10);
   AddALU2("SUB" ,0x20);
   AddALU2("SBC" ,0x30);
   AddALU2("OR"  ,0x40);
   AddALU2("AND" ,0x50);
   AddALU2("TCM" ,0x60);
   AddALU2("TM"  ,0x70);
   AddALU2("CP"  ,0xa0);
   AddALU2("XOR" ,0xb0);

   ALU1Orders=(ALU1Order *) malloc(sizeof(ALU1Order)*ALU1OrderCnt); InstrZ=0;
   AddALU1("DEC" , 0x00, False);
   AddALU1("RLC" , 0x10, False);
   AddALU1("DA"  , 0x40, False);
   AddALU1("POP" , 0x50, False);
   AddALU1("COM" , 0x60, False);
   AddALU1("PUSH", 0x70, False);
   AddALU1("DECW", 0x80, True );
   AddALU1("RL"  , 0x90, False);
   AddALU1("INCW", 0xa0, True );
   AddALU1("CLR" , 0xb0, False);
   AddALU1("RRC" , 0xc0, False);
   AddALU1("SRA" , 0xd0, False);
   AddALU1("RR"  , 0xe0, False);
   AddALU1("SWAP", 0xf0, False);

   Conditions=(Condition *) malloc(sizeof(Condition)*CondCnt); InstrZ=0;
   AddCondition("F"  , 0); TrueCond=InstrZ; AddCondition("T"  , 8);
   AddCondition("C"  , 7); AddCondition("NC" ,15);
   AddCondition("Z"  , 6); AddCondition("NZ" ,14);
   AddCondition("MI" , 5); AddCondition("PL" ,13);
   AddCondition("OV" , 4); AddCondition("NOV",12);
   AddCondition("EQ" , 6); AddCondition("NE" ,14);
   AddCondition("LT" , 1); AddCondition("GE" , 9);
   AddCondition("LE" , 2); AddCondition("GT" ,10);
   AddCondition("ULT", 7); AddCondition("UGE",15);
   AddCondition("ULE", 3); AddCondition("UGT",11);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(ALU2Orders);
   free(ALU1Orders);
   free(Conditions);
END

/*--------------------------------------------------------------------------*/ 

	static Boolean IsWReg(char *Asc, Byte *Erg)
BEGIN
   Boolean Err;

   if ((strlen(Asc)<2) OR (toupper(*Asc)!='R')) return False;
   else
    BEGIN
     *Erg=ConstLongInt(Asc+1,&Err);
     if (NOT Err) return False;
     else return (*Erg<=15);
    END
END

	static Boolean IsRReg(char *Asc, Byte *Erg)
BEGIN
   Boolean Err;

   if ((strlen(Asc)<3) OR (strncasecmp(Asc,"RR",2)!=0)) return False;
   else
    BEGIN
     *Erg=ConstLongInt(Asc+2,&Err);
     if (NOT Err) return False;
     else return (*Erg<=15);
    END
END

	static void CorrMode(Byte Mask, ShortInt Old, ShortInt New)
BEGIN
   if ((AdrType==Old) AND ((Mask & (1 << Old))==0))
    BEGIN
     AdrType=New; AdrMode+=WorkOfs;
    END
END

	static void ChkAdr(Byte Mask, Boolean Is16)
BEGIN
   if (NOT Is16)
    BEGIN
     CorrMode(Mask,ModWReg,ModReg);
     CorrMode(Mask,ModIWReg,ModIReg);
    END

   if ((AdrType!=ModNone) AND ((Mask & (1 << AdrType))==0))
    BEGIN
     WrError(1350); AdrType=ModNone;
    END
END	

	static void DecodeAdr(char *Asc, Byte Mask, Boolean Is16)
BEGIN
   Boolean OK;
   char  *p;

   AdrType=ModNone;

   /* immediate ? */

   if (*Asc=='#')
    BEGIN
     AdrMode=EvalIntExpression(Asc+1,Int8,&OK);
     if (OK) AdrType=ModImm;
     ChkAdr(Mask,Is16); return;
    END;

   /* Register ? */

   if (IsWReg(Asc,&AdrMode))
    BEGIN
     AdrType=ModWReg; ChkAdr(Mask,Is16); return;
    END

   if (IsRReg(Asc,&AdrMode))
    BEGIN
     if ((AdrMode&1)==1) WrError(1351); else AdrType=ModRReg;
     ChkAdr(Mask,Is16); return;
    END

   /* indirekte Konstrukte ? */

   if (*Asc=='@')
    BEGIN
     strcpy(Asc,Asc+1);
     if (IsWReg(Asc,&AdrMode)) AdrType=ModIWReg;
     else if (IsRReg(Asc,&AdrMode))
      BEGIN
       if ((AdrMode&1)==1) WrError(1351); else AdrType=ModIRReg;
      END
     else
      BEGIN
       AdrMode=EvalIntExpression(Asc,Int8,&OK);
       if (OK)
	BEGIN
	 AdrType=ModIReg; ChkSpace(SegData);
	END
      END
     ChkAdr(Mask,Is16); return;
    END

   /* indiziert ? */

   if ((Asc[strlen(Asc)-1]==')') AND (strlen(Asc)>4))
    BEGIN
     p=Asc+strlen(Asc)-1; *p='\0';
     while ((p>=Asc) AND (*p!='(')) p--;
     if (*p!='(') WrError(1300);
     else if (NOT IsWReg(p+1,&AdrMode)) WrXError(1445,p+1);
     else
      BEGIN
       *p='\0';
       AdrIndex=EvalIntExpression(Asc,Int8,&OK);
       if (OK)
	BEGIN
	 AdrType=ModInd; ChkSpace(SegData);
	END
       ChkAdr(Mask,Is16); return;
      END
    END

   /* einfache direkte Adresse ? */

   if (Is16) AdrWMode=EvalIntExpression(Asc,UInt16,&OK);
   else AdrMode=EvalIntExpression(Asc,UInt8,&OK);
   if (OK)
    BEGIN
     AdrType=ModReg;
     ChkSpace((Is16)?SegCode:SegData);
     ChkAdr(Mask,Is16); return;
    END

   ChkAdr(Mask,Is16); 
END

/*---------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,0xff);
     return True;
    END

   return False;
END

	static void MakeCode_Z8(void)
BEGIN
   Integer AdrInt;
   int z;
   Byte Save;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(True)) return;

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
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModWReg+MModIReg+MModIWReg+MModInd,False);
       switch (AdrType)
        BEGIN
         case ModReg:
	  Save=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModWReg+MModIReg+MModImm,False);
	  switch (AdrType)
           BEGIN
	    case ModReg:
	     BAsmCode[0]=0xe4;
	     BAsmCode[1]=AdrMode; BAsmCode[2]=Save;
	     CodeLen=3;
	     break;
	    case ModWReg:
	     BAsmCode[0]=(AdrMode << 4)+9;
	     BAsmCode[1]=Save;
	     CodeLen=2;
	     break;
	    case ModIReg:
	     BAsmCode[0]=0xe5;
	     BAsmCode[1]=AdrMode; BAsmCode[2]=Save;
	     CodeLen=3;
	     break;
	    case ModImm:
	     BAsmCode[0]=0xe6;
	     BAsmCode[1]=Save; BAsmCode[2]=AdrMode;
	     CodeLen=3;
	     break;
	   END
	  break;
         case ModWReg:
 	  Save=AdrMode;
	  DecodeAdr(ArgStr[2],MModWReg+MModReg+MModIWReg+MModIReg+MModImm+MModInd,False);
	  switch (AdrType)
           BEGIN
	    case ModWReg:
	     BAsmCode[0]=(Save << 4)+8; BAsmCode[1]=AdrMode+WorkOfs;
	     CodeLen=2;
	     break;
	    case ModReg:
	     BAsmCode[0]=(Save << 4)+8; BAsmCode[1]=AdrMode;
	     CodeLen=2;
	     break;
	    case ModIWReg:
	     BAsmCode[0]=0xe3; BAsmCode[1]=(Save << 4)+AdrMode;
	     CodeLen=2;
	     break;
	    case ModIReg:
	     BAsmCode[0]=0xe5;
	     BAsmCode[1]=AdrMode; BAsmCode[2]=WorkOfs+Save;
	     CodeLen=3;
	     break;
	    case ModImm:
	     BAsmCode[0]=(Save << 4)+12; BAsmCode[1]=AdrMode;
	     CodeLen=2;
	     break;
	    case ModInd:
	     BAsmCode[0]=0xc7;
	     BAsmCode[1]=(Save << 4)+AdrMode; BAsmCode[2]=AdrIndex;
	     CodeLen=3;
	     break;
	   END
	  break;
         case ModIReg:
	  Save=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModImm,False);
	  switch (AdrType)
           BEGIN
	    case ModReg:
	     BAsmCode[0]=0xf5;
	     BAsmCode[1]=AdrMode; BAsmCode[2]=Save;
	     CodeLen=3;
	     break;
	    case ModImm:
	     BAsmCode[0]=0xe7;
	     BAsmCode[1]=Save; BAsmCode[2]=AdrMode;
	     CodeLen=3;
	     break;
	   END
	  break;
         case ModIWReg:
	  Save=AdrMode;
	  DecodeAdr(ArgStr[2],MModWReg+MModReg+MModImm,False);
	  switch (AdrType)
           BEGIN
	    case ModWReg:
	     BAsmCode[0]=0xf3; BAsmCode[1]=(Save << 4)+AdrMode;
	     CodeLen=2;
	     break;
	    case ModReg:
	     BAsmCode[0]=0xf5;
	     BAsmCode[1]=AdrMode; BAsmCode[2]=WorkOfs+Save;
	     CodeLen=3;
	     break;
	    case ModImm:
	     BAsmCode[0]=0xe7;
	     BAsmCode[1]=WorkOfs+Save; BAsmCode[2]=AdrMode;
	     CodeLen=3;
	     break;
	   END
	  break;
         case ModInd:
	  Save=AdrMode;
	  DecodeAdr(ArgStr[2],MModWReg,False);
	  switch (AdrType)
           BEGIN
	    case ModWReg:
	     BAsmCode[0]=0xd7;
	     BAsmCode[1]=(AdrMode << 4)+Save; BAsmCode[2]=AdrIndex;
	     CodeLen=3;
	     break;
	   END
          break;
        END
      END
     return;
    END

   if ((Memo("LDC")) OR (Memo("LDE")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg+MModIRReg,False);
       switch (AdrType)
        BEGIN
         case ModWReg:
	  Save=AdrMode; DecodeAdr(ArgStr[2],MModIRReg,False);
	  if (AdrType!=ModNone)
	   BEGIN
	    BAsmCode[0]=(Memo("LDC")) ? 0xc2 : 0x82;
	    BAsmCode[1]=(Save << 4)+AdrMode;
	    CodeLen=2;
	   END
	  break;
         case ModIRReg:
	  Save=AdrMode; DecodeAdr(ArgStr[2],MModWReg,False);
	  if (AdrType!=ModNone)
	   BEGIN
	    BAsmCode[0]=(Memo("LDC")) ? 0xd2 : 0x92;
	    BAsmCode[1]=(AdrMode << 4)+Save;
	    CodeLen=2;
	   END
	  break;
        END
      END
     return;
    END

   if ((Memo("LDCI")) OR (Memo("LDEI")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModIWReg+MModIRReg,False);
       switch (AdrType)
        BEGIN
         case ModIWReg:
	  Save=AdrMode; DecodeAdr(ArgStr[2],MModIRReg,False);
	  if (AdrType!=ModNone)
	   BEGIN
	    BAsmCode[0]=(Memo("LDCI")) ? 0xc3 : 0x83;
	    BAsmCode[1]=(Save << 4)+AdrMode;
	    CodeLen=2;
	   END
	  break;
         case ModIRReg:
	  Save=AdrMode; DecodeAdr(ArgStr[2],MModIWReg,False);
	  if (AdrType!=ModNone)
	   BEGIN
	    BAsmCode[0]=(Memo("LDCI")) ? 0xd3 : 0x93;
	    BAsmCode[1]=(AdrMode << 4)+Save;
	    CodeLen=2;
	   END
	  break;
        END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<ALU2OrderCnt; z++)
    if (Memo(ALU2Orders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg+MModWReg+MModIReg,False);
        switch (AdrType)
         BEGIN
          case ModReg:
           Save=AdrMode;
           DecodeAdr(ArgStr[2],MModReg+MModIReg+MModImm,False);
           switch (AdrType)
            BEGIN
             case ModReg:
              BAsmCode[0]=ALU2Orders[z].Code+4;
              BAsmCode[1]=AdrMode;
              BAsmCode[2]=Save;
              CodeLen=3;
              break;
             case ModIReg:
              BAsmCode[0]=ALU2Orders[z].Code+5;
              BAsmCode[1]=AdrMode;
              BAsmCode[2]=Save;
              CodeLen=3;
              break;
             case ModImm:
              BAsmCode[0]=ALU2Orders[z].Code+6;
              BAsmCode[1]=Save;
              BAsmCode[2]=AdrMode;
              CodeLen=3;
              break;
            END
           break;
          case ModWReg:
           Save=AdrMode;
           DecodeAdr(ArgStr[2],MModWReg+MModReg+MModIWReg+MModIReg+MModImm,False);
           switch (AdrType)
            BEGIN
             case ModWReg:
              BAsmCode[0]=ALU2Orders[z].Code+2;
              BAsmCode[1]=(Save << 4)+AdrMode;
              CodeLen=2;
              break;
             case ModReg:
              BAsmCode[0]=ALU2Orders[z].Code+4;
              BAsmCode[1]=AdrMode;
              BAsmCode[2]=WorkOfs+Save;
              CodeLen=3;
              break;
             case ModIWReg:
              BAsmCode[0]=ALU2Orders[z].Code+3;
              BAsmCode[1]=(Save << 4)+AdrMode;
              CodeLen=2;
              break;
             case ModIReg:
              BAsmCode[0]=ALU2Orders[z].Code+5;
              BAsmCode[1]=AdrMode;
              BAsmCode[2]=WorkOfs+Save;
              CodeLen=3;
              break;
             case ModImm:
              BAsmCode[0]=ALU2Orders[z].Code+6;
              BAsmCode[1]=Save+WorkOfs;
              BAsmCode[2]=AdrMode;
              CodeLen=3;
              break;
            END
           break;
          case ModIReg:
           Save=AdrMode;
           DecodeAdr(ArgStr[2],MModImm,False);
           switch (AdrType)
            BEGIN
             case ModImm:
              BAsmCode[0]=ALU2Orders[z].Code+7;
              BAsmCode[1]=Save;
              BAsmCode[2]=AdrMode;
              CodeLen=3;
              break;
            END
           break;
         END
       END
      return;
     END

   /* INC hat eine Optimierungsmoeglichkeit */

   if (Memo("INC"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg+MModReg+MModIReg,False);
       switch (AdrType)
        BEGIN
         case ModWReg:
 	  BAsmCode[0]=(AdrMode << 4)+0x0e; CodeLen=1;
	  break;
         case ModReg:
	  BAsmCode[0]=0x20; BAsmCode[1]=AdrMode; CodeLen=2;
	  break;
         case ModIReg:
	  BAsmCode[0]=0x21; BAsmCode[1]=AdrMode; CodeLen=2;
	  break;
        END
      END
     return;
    END

   /* ...alle anderen nicht */

   for (z=0; z<ALU1OrderCnt; z++)
    if (Memo(ALU1Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],((ALU1Orders[z].Is16)?MModRReg:0)+MModReg+MModIReg,False);
        switch (AdrType)
         BEGIN
          case ModReg:
           BAsmCode[0]=ALU1Orders[z].Code; BAsmCode[1]=AdrMode; CodeLen=2;
           break;
          case ModRReg:
           BAsmCode[0]=ALU1Orders[z].Code; BAsmCode[1]=WorkOfs+AdrMode; CodeLen=2;
           break;
          case ModIReg:
           BAsmCode[0]=ALU1Orders[z].Code+1; BAsmCode[1]=AdrMode; CodeLen=2;
           break;
         END
       END
      return;
     END

  /* Spruenge */

   if (Memo("JR"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) z=TrueCond;
       else
	BEGIN
	 z=0; NLS_UpString(ArgStr[1]);
	 while ((z<CondCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
	 if (z>=CondCnt) WrError(1360);
	END
       if (z<CondCnt)
	BEGIN
	 AdrInt=EvalIntExpression(ArgStr[ArgCnt],Int16,&OK)-(EProgCounter()+2);
	 if (OK)
	  if ((NOT SymbolQuestionable) AND ((AdrInt>127) OR (AdrInt<-128))) WrError(1370);
	  else
	   BEGIN
	    ChkSpace(SegCode);
            BAsmCode[0]=(Conditions[z].Code << 4)+0x0b;
	    BAsmCode[1]=Lo(AdrInt);
	    CodeLen=2;
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
       DecodeAdr(ArgStr[1],MModWReg,False);
       if (AdrType!=ModNone)
	BEGIN
	 AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK)-(EProgCounter()+2);
	 if (OK)
	  if ((NOT SymbolQuestionable) AND ((AdrInt>127) OR (AdrInt<-128))) WrError(1370);
	  else
	   BEGIN
	    BAsmCode[0]=(AdrMode << 4)+0x0a;
	    BAsmCode[1]=Lo(AdrInt);
	    CodeLen=2;
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
       DecodeAdr(ArgStr[1],MModIRReg+MModIReg+MModReg,True);
       switch (AdrType)
        BEGIN
         case ModIRReg:
	  BAsmCode[0]=0xd4; BAsmCode[1]=0xe0+AdrMode; CodeLen=2;
	  break;
         case ModIReg:
	  BAsmCode[0]=0xd4; BAsmCode[1]=AdrMode; CodeLen=2;
	  break;
         case ModReg:
	  BAsmCode[0]=0xd6;
	  BAsmCode[1]=Hi(AdrWMode); BAsmCode[2]=Lo(AdrWMode);
	  CodeLen=3;
	  break;
        END
      END
     return;
    END

   if (Memo("JP"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) z=TrueCond;
       else
	BEGIN
	 z=0; NLS_UpString(ArgStr[1]);
	 while ((z<CondCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
	 if (z>=CondCnt) WrError(1360);
	END
       if (z<CondCnt)
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModIRReg+MModIReg+MModReg,True);
	 switch (AdrType)
          BEGIN
	   case ModIRReg:
	    if (z!=TrueCond) WrError(1350);
	    else
	     BEGIN
	      BAsmCode[0]=0x30; BAsmCode[1]=0xe0+AdrMode; CodeLen=2;
	     END
            break;
	   case ModIReg:
	    if (z!=TrueCond) WrError(1350);
	    else
	     BEGIN
	      BAsmCode[0]=0x30; BAsmCode[1]=AdrMode; CodeLen=2;
	     END
            break;
	   case ModReg:
	    BAsmCode[0]=(Conditions[z].Code << 4)+0x0d;
	    BAsmCode[1]=Hi(AdrWMode); BAsmCode[2]=Lo(AdrWMode);
	    CodeLen=3;
	    break;
	  END
	END
      END
     return;
    END

   /* Sonderbefehle */

   if (Memo("SRP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModImm,False);
       if (AdrType==ModImm)
	if (((AdrMode & 15)!=0) OR ((AdrMode>0x70) AND (AdrMode<0xf0))) WrError(120);
	else
	 BEGIN
	  BAsmCode[0]=0x31; BAsmCode[1]=AdrMode;
	  CodeLen=2;
	 END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static Boolean ChkPC_Z8(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode  : return (ProgCounter()<0x10000);
     case SegData  : return (ProgCounter()<  0x100);
     default: return False;
    END
END


	static Boolean IsDef_Z8(void)
BEGIN
   return (Memo("SFR"));
END

        static void SwitchFrom_Z8(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_Z8(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x79; NOPCode=0xff;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;

   MakeCode=MakeCode_Z8; ChkPC=ChkPC_Z8; IsDef=IsDef_Z8;
   SwitchFrom=SwitchFrom_Z8; InitFields();
END

	void codez8_init(void)
BEGIN
   CPUZ8601=AddCPU("Z8601",SwitchTo_Z8);
   CPUZ8604=AddCPU("Z8604",SwitchTo_Z8);
   CPUZ8608=AddCPU("Z8608",SwitchTo_Z8);
   CPUZ8630=AddCPU("Z8630",SwitchTo_Z8);
   CPUZ8631=AddCPU("Z8631",SwitchTo_Z8);
END

