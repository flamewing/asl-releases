/* codest6.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator ST6-Familie                                                 */
/*                                                                           */
/* Historie: 14.11.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "stringutil.h"
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
          Word Code;
         } AccOrder;


#define FixedOrderCnt 5

#define RelOrderCnt 4

#define ALUOrderCnt 4

#define AccOrderCnt 3


#define ModNone (-1)
#define ModAcc 0
#define MModAcc (1 << ModAcc)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModInd 2
#define MModInd (1 << ModInd)


static Byte AdrMode;
static ShortInt AdrType;
static Byte AdrVal;

static LongInt WinAssume;

static SimpProc SaveInitProc;

static CPUVar CPUST6210,CPUST6215,CPUST6220,CPUST6225;

static FixedOrder *FixedOrders;
static FixedOrder *RelOrders;
static FixedOrder *ALUOrders;
static AccOrder *AccOrders;

/*---------------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddRel(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
END

        static void AddALU(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name=NName;
   ALUOrders[InstrZ++].Code=NCode;
END

        static void AddAcc(char *NName, Word NCode)
BEGIN
   if (InstrZ>=AccOrderCnt) exit(255);
   AccOrders[InstrZ].Name=NName;
   AccOrders[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("NOP" , 0x04);
   AddFixed("RET" , 0xcd);
   AddFixed("RETI", 0x4d);
   AddFixed("STOP", 0x6d);
   AddFixed("WAIT", 0xed);

   RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCnt); InstrZ=0;
   AddRel("JRZ" , 0x04);
   AddRel("JRNZ", 0x00);
   AddRel("JRC" , 0x06);
   AddRel("JRNC", 0x02);

   ALUOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*ALUOrderCnt); InstrZ=0;
   AddALU("ADD" , 0x47);
   AddALU("AND" , 0xa7);
   AddALU("CP"  , 0x27);
   AddALU("SUB" , 0xc7);

   AccOrders=(AccOrder *) malloc(sizeof(AccOrder)*AccOrderCnt); InstrZ=0;
   AddAcc("COM", 0x002d);
   AddAcc("RLC", 0x00ad);
   AddAcc("SLA", 0xff5f);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(RelOrders);
   free(ALUOrders);
   free(AccOrders);
END

/*---------------------------------------------------------------------------------*/

	static void ResetAdr(void)
BEGIN
   AdrType=ModNone; AdrCnt=0;
END

	static void ChkAdr(Byte Mask)
BEGIN
   if ((AdrType!=ModNone) AND ((Mask AND (1 << AdrType))==0))
    BEGIN
     ResetAdr(); WrError(1350);
    END
END

	static void DecodeAdr(char *Asc, Byte Mask)
BEGIN
#define RegCnt 5
   static char *RegNames[RegCnt+1]={"A","V","W","X","Y"};
   static Byte RegCodes[RegCnt+1]={0xff,0x82,0x83,0x80,0x81};

   Boolean OK;
   Integer z,AdrInt;

   ResetAdr();

   if ((strcasecmp(Asc,"A")==0) AND ((Mask & MModAcc)!=0))
    BEGIN
     AdrType=ModAcc; ChkAdr(Mask); return;
    END

   for (z=0; z<RegCnt; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      AdrType=ModDir; AdrCnt=1; AdrVal=RegCodes[z];
      ChkAdr(Mask); return;
     END

   if (strcasecmp(Asc,"(X)")==0)
    BEGIN
     AdrType=ModInd; AdrMode=0; ChkAdr(Mask); return;
    END

   if (strcasecmp(Asc,"(Y)")==0)
    BEGIN
     AdrType=ModInd; AdrMode=1; ChkAdr(Mask); return;
    END

   AdrInt=EvalIntExpression(Asc,UInt16,&OK);
   if (OK)
    if ((TypeFlag & (1 << SegCode))!=0)
     BEGIN
      AdrType=ModDir; AdrVal=(AdrInt & 0x3f)+0x40; AdrCnt=1;
      if (NOT FirstPassUnknown)
       if (WinAssume!=(AdrInt >> 6)) WrError(110);
     END
    else
     BEGIN
      if (FirstPassUnknown) AdrInt=Lo(AdrInt);
      if (AdrInt>0xff) WrError(1320);
      else
       BEGIN
	AdrType=ModDir; AdrVal=AdrInt; ChkAdr(Mask); return;
       END
     END

   ChkAdr(Mask);
END

	static Boolean DecodePseudo(void)
BEGIN
#define ASSUME62Count 1
   static ASSUMERec ASSUME62s[ASSUME62Count]=
   	            {{"ROMBASE", &WinAssume, 0, 0x3f, 0x40}};

   Boolean OK,Flag;
   Integer z;
   String s;

   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,0xff);
     return True;
    END

   if ((Memo("ASCII")) OR (Memo("ASCIZ")))
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       z=1; Flag=Memo("ASCIZ");
       do
        BEGIN
	 EvalStringExpression(ArgStr[z],&OK,s);
	 if (OK)
	  BEGIN
	   TranslateString(s);
	   if (CodeLen+strlen(s)+Ord(Flag)>MaxCodeLen)
	    BEGIN
	     WrError(1920); OK=False;
	    END
	   else
	    BEGIN
	     memcpy(BAsmCode+CodeLen,s,strlen(s)); CodeLen+=strlen(s);
	     if (Flag) BAsmCode[CodeLen++]=0;
	    END
	  END
	 z++;
        END
       while ((OK) AND (z<=ArgCnt));
       if (NOT OK) CodeLen=0;
      END
     return True;
    END

   if (Memo("BYTE"))
    BEGIN
     strmaxcpy(OpPart,"BYT",255); DecodeMotoPseudo(False);
     return True;
    END

   if (Memo("WORD"))
    BEGIN
     strmaxcpy(OpPart,"ADR",255); DecodeMotoPseudo(False);
     return True;
    END

   if (Memo("BLOCK"))
    BEGIN
     strmaxcpy(OpPart,"DFS",255); DecodeMotoPseudo(False);
     return True;
    END

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME62s,ASSUME62Count);
     return True;
    END

   return False;
END

	static Boolean IsReg(Byte Adr)
BEGIN
   return ((Adr & 0xfc)==0x80);
END

        static Byte MirrBit(Byte inp)
BEGIN
   return (((inp & 1) << 2)+(inp & 2)+((inp & 4) >> 2));
END

	static void MakeCode_ST62(void)
BEGIN
   Integer z,AdrInt;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

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
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModDir+MModInd);
       switch (AdrType)
        BEGIN
         case ModAcc:
	  DecodeAdr(ArgStr[2],MModDir+MModInd);
	  switch (AdrType)
           BEGIN
	    case ModDir:
	     if (IsReg(AdrVal))
    	      BEGIN
	       CodeLen=1; BAsmCode[0]=0x35+((AdrVal & 3) << 6);
	      END
	     else
	      BEGIN
	       CodeLen=2; BAsmCode[0]=0x1f; BAsmCode[1]=AdrVal;
	      END
             break;
	    case ModInd:
	     CodeLen=1; BAsmCode[0]=0x07+(AdrMode << 3);
	     break;
	   END
	  break;
         case ModDir:
	  DecodeAdr(ArgStr[2],MModAcc);
	  if (AdrType!=ModNone)
	   if (IsReg(AdrVal))
	    BEGIN
	     CodeLen=1; BAsmCode[0]=0x3d+((AdrVal & 3) << 6);
	    END
	   else
	    BEGIN
	     CodeLen=2; BAsmCode[0]=0x9f; BAsmCode[1]=AdrVal;
	    END
	   break;
         case ModInd:
	  DecodeAdr(ArgStr[2],MModAcc);
	  if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0x87+(AdrMode << 3);
	   END
	  break;
        END
      END
     return;
    END

   if (Memo("LDI"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
	BEGIN
	 DecodeAdr(ArgStr[1],MModAcc+MModDir);
	 switch (AdrType)
          BEGIN
	   case ModAcc:
	    CodeLen=2; BAsmCode[0]=0x17; BAsmCode[1]=Lo(AdrInt);
	    break;
	   case ModDir:
	    CodeLen=3; BAsmCode[0]=0x0d; BAsmCode[1]=AdrVal;
	    BAsmCode[2]=Lo(AdrInt);
	    break;
	  END
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
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+1);
        if (OK)
         if ((NOT SymbolQuestionable) AND ((AdrInt<-16) OR (AdrInt>15))) WrError(1370);
         else
          BEGIN
           CodeLen=1;
           BAsmCode[0]=RelOrders[z].Code+((AdrInt << 3) & 0xf8);
          END
       END
      return;
     END

   if ((Memo("JP")) OR (Memo("CALL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        if ((AdrInt<0) OR (AdrInt>0xfff)) WrError(1925);
        else
         BEGIN
          CodeLen=2;
          BAsmCode[0]=0x01+(Ord(Memo("JP")) << 3)+((AdrInt & 0x00f) << 4);
          BAsmCode[1]=AdrInt >> 4;
         END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<ALUOrderCnt; z++)
    if (strncmp(ALUOrders[z].Name,OpPart,strlen(ALUOrders[z].Name))==0)
     switch (OpPart[strlen(ALUOrders[z].Name)])
      BEGIN
       case '\0':
        if (ArgCnt!=2) WrError(1110);
        else
         BEGIN
          DecodeAdr(ArgStr[1],MModAcc);
          if (AdrType!=ModNone)
           BEGIN
            DecodeAdr(ArgStr[2],MModDir+MModInd);
            switch (AdrType)
             BEGIN
              case ModDir:
               CodeLen=2; BAsmCode[0]=ALUOrders[z].Code+0x18;
               BAsmCode[1]=AdrVal;
               break;
              case ModInd:
               CodeLen=1; BAsmCode[0]=ALUOrders[z].Code+(AdrMode << 3);
               break;
             END
           END
         END
        return;
       case 'I':
        if (ArgCnt!=2) WrError(1110);
        else
         BEGIN
          DecodeAdr(ArgStr[1],MModAcc);
          if (AdrType!=ModNone)
           BEGIN
            BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
            if (OK)
             BEGIN
              CodeLen=2; BAsmCode[0]=ALUOrders[z].Code+0x10;
             END
           END
         END
        return;
      END

   if (Memo("CLR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModDir);
       switch (AdrType)
        BEGIN
         case ModAcc:
	  CodeLen=2; BAsmCode[0]=0xdf; BAsmCode[1]=0xff;
	  break;
         case ModDir:
	  CodeLen=3; BAsmCode[0]=0x0d; BAsmCode[1]=AdrVal; BAsmCode[2]=0;
	  break;
        END
      END
     return;
    END

   for (z=0; z<AccOrderCnt; z++)
    if (Memo(AccOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModAcc);
        if (AdrType!=ModNone)
         BEGIN
          OK=(Hi(AccOrders[z].Code)!=0);
          CodeLen=1+Ord(OK);
          BAsmCode[0]=Lo(AccOrders[z].Code);
          if (OK) BAsmCode[1]=Hi(AccOrders[z].Code);
         END
       END
      return;
     END

   if ((Memo("INC")) OR (Memo("DEC")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModDir+MModInd);
       switch (AdrType)
        BEGIN
         case ModDir:
	  if (IsReg(AdrVal))
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0x15+((AdrVal & 3) << 6);
	    if (Memo("DEC")) BAsmCode[0]+=8;
	   END
	  else
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x7f+(Ord(Memo("DEC")) << 7);
	    BAsmCode[1]=AdrVal;
	   END
          break;
         case ModInd:
	  CodeLen=1;
	  BAsmCode[0]=0x67+(AdrMode << 3)+(Ord(Memo("DEC")) << 7);
	  break;
        END
      END
     return;
    END

   /* Bitbefehle */

   if ((Memo("SET")) OR (Memo("RES")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       BAsmCode[0]=MirrBit(EvalIntExpression(ArgStr[1],UInt3,&OK));
       if (OK)
	BEGIN
	 DecodeAdr(ArgStr[2],MModDir);
	 if (AdrType!=ModNone)
	  BEGIN
	   CodeLen=2;
	   BAsmCode[0]=(BAsmCode[0] << 5)+(Ord(Memo("SET")) << 4)+0x0b;
	   BAsmCode[1]=AdrVal;
	  END
	END
      END
     return;
    END

   if ((Memo("JRR")) OR (Memo("JRS")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       BAsmCode[0]=MirrBit(EvalIntExpression(ArgStr[1],UInt3,&OK));
       if (OK)
	BEGIN
	 BAsmCode[0]=(BAsmCode[0] << 5)+3+(Ord(Memo("JRS")) << 4);
	 DecodeAdr(ArgStr[2],MModDir);
	 if (AdrType!=ModNone)
	  BEGIN
	   BAsmCode[1]=AdrVal;
           AdrInt=EvalIntExpression(ArgStr[3],UInt16,&OK)-(EProgCounter()+3);
	   if (OK)
	    if ((NOT SymbolQuestionable) AND ((AdrInt>127) OR (AdrInt<-128))) WrError(1370);
	    else
	     BEGIN
	      CodeLen=3; BAsmCode[2]=Lo(AdrInt);
	     END
	  END
	END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static void InitCode_ST62(void)
BEGIN
   SaveInitProc();
   WinAssume=0x40;
END

	static Boolean ChkPC_ST62(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode:
      return (ProgCounter()<((MomCPU<CPUST6220)?0x1000:0x800));
     case SegData:
      return (ProgCounter()<0x100);
     default: return False;
    END
END

	static Boolean IsDef_ST62(void)
BEGIN
   return (Memo("SFR"));
END

        static void SwitchFrom_ST62(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_ST62(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=True;

   PCSymbol="PC"; HeaderID=0x78; NOPCode=0x04;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)+(1<<SegData);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;

   MakeCode=MakeCode_ST62; ChkPC=ChkPC_ST62; IsDef=IsDef_ST62;
   SwitchFrom=SwitchFrom_ST62; InitFields();
END

	void codest6_init(void)
BEGIN
   CPUST6210=AddCPU("ST6210",SwitchTo_ST62);
   CPUST6215=AddCPU("ST6215",SwitchTo_ST62);
   CPUST6220=AddCPU("ST6220",SwitchTo_ST62);
   CPUST6225=AddCPU("ST6225",SwitchTo_ST62);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_ST62;
END

