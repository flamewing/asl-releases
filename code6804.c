/* code6804.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codeenerator Motorola/ST 6804                                          */
/*                                                                           */
/* Historie: 17.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"

typedef struct
         {
          char *Name;
          LongInt Code;
         } BaseOrder;

#define FixedOrderCnt 19
#define RelOrderCnt 6
#define ALUOrderCnt 4

#define ModNone -1
#define ModInd 0
#define MModInd (1 << ModInd)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModImm 2
#define MModImm (1 << ModImm)

static ShortInt AdrMode;
static Byte AdrVal;

static CPUVar CPU6804;

static BaseOrder *FixedOrders;
static BaseOrder *RelOrders;
static BaseOrder *ALUOrders;

/*--------------------------------------------------------------------------*/

static int InstrZ;

	static void AddFixed(char *NName, LongInt NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddRel(char *NName, LongInt NCode)   
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
END

        static void AddALU(char *NName, LongInt NCode)   
BEGIN
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name=NName;
   ALUOrders[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CLRA", 0x00fbff);
   AddFixed("CLRX", 0xb08000);
   AddFixed("CLRY", 0xb08100);
   AddFixed("COMA", 0x0000b4);
   AddFixed("ROLA", 0x0000b5);
   AddFixed("ASLA", 0x00faff);
   AddFixed("INCA", 0x00feff);
   AddFixed("INCX", 0x0000a8);
   AddFixed("INCY", 0x0000a9);
   AddFixed("DECA", 0x00ffff);
   AddFixed("DECX", 0x0000b8);
   AddFixed("DECY", 0x0000b9);
   AddFixed("TAX" , 0x0000bc);
   AddFixed("TAY" , 0x0000bd);
   AddFixed("TXA" , 0x0000ac);
   AddFixed("TYA" , 0x0000ad);
   AddFixed("RTS" , 0x0000b3);
   AddFixed("RTI" , 0x0000b2);
   AddFixed("NOP" , 0x000020);

   RelOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*RelOrderCnt); InstrZ=0;
   AddRel("BCC", 0x40);
   AddRel("BHS", 0x40);
   AddRel("BCS", 0x60);
   AddRel("BLO", 0x60);
   AddRel("BNE", 0x00);
   AddRel("BEQ", 0x20);

   ALUOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*ALUOrderCnt); InstrZ=0;
   AddALU("ADD", 0x02);
   AddALU("SUB", 0x03);
   AddALU("CMP", 0x04);
   AddALU("AND", 0x05);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(RelOrders);
   free(ALUOrders);
END

/*--------------------------------------------------------------------------*/

	static void ChkAdr(Boolean MayImm)
BEGIN
   if ((AdrMode==ModImm) AND (NOT MayImm))
    BEGIN
     WrError(1350); AdrMode=ModNone;
    END
END

	static void DecodeAdr(char *Asc, Boolean MayImm)
BEGIN
   Boolean OK;

   AdrMode=ModNone;

   if (strcasecmp(Asc,"(X)")==0)
    BEGIN
     AdrMode=ModInd; AdrVal=0x00; ChkAdr(MayImm); return;
    END
   if (strcasecmp(Asc,"(Y)")==0)
    BEGIN
     AdrMode=ModInd; AdrVal=0x10; ChkAdr(MayImm); return;
    END

   if (*Asc=='#')
    BEGIN
     AdrVal=EvalIntExpression(Asc+1,Int8,&OK);
     if (OK) AdrMode=ModImm; ChkAdr(MayImm); return;
    END

   AdrVal=EvalIntExpression(Asc,Int8,&OK);
   if (OK)
    BEGIN
     AdrMode=ModDir; ChkAdr(MayImm); return;
    END

   ChkAdr(MayImm);
END

/*--------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,0xff);
     return True;
    END

   return False;
END

	static Boolean IsShort(Byte Adr)
BEGIN
   return ((Adr & 0xfc)==0x80);
END

	static void MakeCode_6804(void)
BEGIN
   Integer z,AdrInt;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(True)) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if Memo(FixedOrders[z].Name)
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        if ((FixedOrders[z].Code >> 16)!=0) CodeLen=3;
        else CodeLen=1+Ord(Hi(FixedOrders[z].Code)!=0);
        if (CodeLen==3) BAsmCode[0]=FixedOrders[z].Code >> 16;
        if (CodeLen>=2) BAsmCode[CodeLen-2]=Hi(FixedOrders[z].Code);
        BAsmCode[CodeLen-1]=Lo(FixedOrders[z].Code);
       END
      return;
     END

   /* relative/absolute Spruenge */

   for (z=0; z<RelOrderCnt; z++)
    if Memo(RelOrders[z].Name)
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK)-(EProgCounter()+1);
        if (OK)
         if ((NOT SymbolQuestionable) AND ((AdrInt<-16) OR (AdrInt>15))) WrError(1370);
         else
          BEGIN
           CodeLen=1; BAsmCode[0]=RelOrders[z].Code+(AdrInt & 0x1f);
           ChkSpace(SegCode);
          END
       END
      return;
     END

   if ((Memo("JSR")) OR (Memo("JMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],UInt12,&OK);
       if (OK)
	BEGIN
	 CodeLen=2; BAsmCode[1]=Lo(AdrInt);
	 BAsmCode[0]=0x80+(Ord(Memo("JMP")) << 4)+(Hi(AdrInt) & 15);
         ChkSpace(SegCode);
	END
      END
     return;
    END

   /* AKKU-Operationen */

   for (z=0; z<ALUOrderCnt; z++)
    if (Memo(ALUOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],True);
        switch (AdrMode)
         BEGIN
          case ModInd:
       	   CodeLen=1; BAsmCode[0]=0xe0+AdrVal+ALUOrders[z].Code;
           break;
          case ModDir:
       	   CodeLen=2; BAsmCode[0]=0xf8+ALUOrders[z].Code; BAsmCode[1]=AdrVal;
           break;
          case ModImm:
       	   CodeLen=2; BAsmCode[0]=0xe8+ALUOrders[z].Code; BAsmCode[1]=AdrVal;
           break;
       	 END
       END
      return;
     END

   /* Datentransfer */

   if ((Memo("LDA")) OR (Memo("STA")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],Memo("LDA"));
       AdrInt=Ord(Memo("STA"));
       switch (AdrMode)
        BEGIN
         case ModInd:
	  CodeLen=1; BAsmCode[0]=0xe0+AdrInt+AdrVal;
	  break;
         case ModDir:
          if (IsShort(AdrVal))
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0xac+(AdrInt << 4)+(AdrVal & 3);
	   END
	  else
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0xf8+AdrInt; BAsmCode[1]=AdrVal;
	   END
          break;
         case ModImm:
	  CodeLen=2; BAsmCode[0]=0xe8+AdrInt; BAsmCode[1]=AdrVal;
	  break;
        END
      END
     return;
    END

   if ((Memo("LDXI")) OR (Memo("LDYI")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      BEGIN
       BAsmCode[2]=EvalIntExpression(ArgStr[1]+1,Int8,&OK);
       if (OK)
	BEGIN
	 CodeLen=3; BAsmCode[0]=0xb0;
	 BAsmCode[1]=0x80+Ord(Memo("LDYI"));
	END
      END
     return;
    END

   if (Memo("MVI"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*ArgStr[2]!='#') WrError(1350);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[1],Int8,&OK);
       if (OK)
	BEGIN
	 ChkSpace(SegData);
	 BAsmCode[2]=EvalIntExpression(ArgStr[2]+1,Int8,&OK);
	 if (OK)
	  BEGIN
	   BAsmCode[0]=0xb0; CodeLen=3;
	  END
	END
      END
     return;
    END

   /* Read/Modify/Write-Operationen */

   if ((Memo("INC")) OR (Memo("DEC")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],False);
       AdrInt=Ord(Memo("DEC"));
       switch (AdrMode)
        BEGIN
         case ModInd:
	  CodeLen=1; BAsmCode[0]=0xe6+AdrInt+AdrVal;
	  break;
         case ModDir:
          if (IsShort(AdrVal))
	   BEGIN
	    CodeLen=1; BAsmCode[0]=0xa8+(AdrInt << 4)+(AdrVal & 3);
	   END
	  else
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0xfe + AdrInt;  /* ANSI :-O */
            BAsmCode[1]=AdrVal;
	   END
          break;
        END
      END
     return;
    END

   /* Bitbefehle */

   if ((Memo("BSET")) OR (Memo("BCLR")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       AdrVal=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
	BEGIN
	 BAsmCode[0]=0xd0+(Ord(Memo("BSET")) << 3)+AdrVal;
	 BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
	 if (OK)
	  BEGIN
	   CodeLen=2; ChkSpace(SegData);
	  END
	END
      END
     return;
    END

   if ((Memo("BRSET")) OR (Memo("BRCLR")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       AdrVal=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
	BEGIN
	 BAsmCode[0]=0xc0+(Ord(Memo("BRSET")) << 3)+AdrVal;
	 BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
	 if (OK)
	  BEGIN
	   ChkSpace(SegData);
	   AdrInt=EvalIntExpression(ArgStr[3],Int16,&OK)-(EProgCounter()+3);
	   if (OK)
	    if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
	    else
	     BEGIN
	      ChkSpace(SegCode); BAsmCode[2]=AdrInt & 0xff; CodeLen=3;
	     END
	  END
	END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static Boolean ChkPC_6804(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode: return ProgCounter()<=0xfff;
     case SegData: return ProgCounter()<=0xff;
     default: return False;
    END
END

	static Boolean IsDef_6804(void)
BEGIN
   return (Memo("SFR"));
END

        static void SwitchFrom_6804(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_6804(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="PC"; HeaderID=0x64; NOPCode=0x20;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;

   MakeCode=MakeCode_6804; ChkPC=ChkPC_6804; IsDef=IsDef_6804;
   SwitchFrom=SwitchFrom_6804; InitFields();
END

	void code6804_init(void)
BEGIN
   CPU6804=AddCPU("6804",SwitchTo_6804);
END
