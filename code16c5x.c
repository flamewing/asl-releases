/* code16c5x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS - Codegenerator fuer PIC16C5x                                          */
/*                                                                           */
/* Historie: 19.8.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct 
         {
          char *Name;
          Word Code;
          Byte DefaultDir;
         } AriOrder;

#define FixedOrderCnt 5
#define LitOrderCnt 5
#define AriOrderCnt 14
#define BitOrderCnt 4
#define FOrderCnt 2

#define D_CPU16C54 0
#define D_CPU16C55 1
#define D_CPU16C56 2
#define D_CPU16C57 3

static FixedOrder *FixedOrders;
static FixedOrder *LitOrders;
static AriOrder *AriOrders;
static FixedOrder *BitOrders;
static FixedOrder *FOrders;

static CPUVar CPU16C54,CPU16C55,CPU16C56,CPU16C57;

/*-------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddLit(char *NName, Word NCode)
BEGIN
   if (InstrZ>=LitOrderCnt) exit(255);
   LitOrders[InstrZ].Name=NName;
   LitOrders[InstrZ++].Code=NCode;
END

	static void AddAri(char *NName, Word NCode, Byte NDef)
BEGIN
   if (InstrZ>=AriOrderCnt) exit(255);
   AriOrders[InstrZ].Name=NName;
   AriOrders[InstrZ].Code=NCode;
   AriOrders[InstrZ++].DefaultDir=NDef;
END

	static void AddBit(char *NName, Word NCode)
BEGIN
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ].Name=NName;
   BitOrders[InstrZ++].Code=NCode;
END

	static void AddF(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FOrderCnt) exit(255);
   FOrders[InstrZ].Name=NName;
   FOrders[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CLRW"  , 0x040);
   AddFixed("NOP"   , 0x000);
   AddFixed("CLRWDT", 0x004);
   AddFixed("OPTION", 0x002);
   AddFixed("SLEEP" , 0x003);

   LitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LitOrderCnt); InstrZ=0;
   AddLit("ANDLW", 0xe00);
   AddLit("IORLW", 0xd00);
   AddLit("MOVLW", 0xc00);
   AddLit("RETLW", 0x800);
   AddLit("XORLW", 0xf00);

   AriOrders=(AriOrder *) malloc(sizeof(AriOrder)*AriOrderCnt); InstrZ=0;
   AddAri("ADDWF" , 0x1c0, 0);
   AddAri("ANDWF" , 0x140, 0);
   AddAri("COMF"  , 0x240, 1);
   AddAri("DECF"  , 0x0c0, 1);
   AddAri("DECFSZ", 0x2c0, 1);
   AddAri("INCF"  , 0x280, 1);
   AddAri("INCFSZ", 0x3c0, 1);
   AddAri("IORWF" , 0x100, 0);
   AddAri("MOVF"  , 0x200, 0);
   AddAri("RLF"   , 0x340, 1);
   AddAri("RRF"   , 0x300, 1);
   AddAri("SUBWF" , 0x080, 0);
   AddAri("SWAPF" , 0x380, 1);
   AddAri("XORWF" , 0x180, 0);

   BitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BitOrderCnt); InstrZ=0;
   AddBit("BCF"  , 0x400);
   AddBit("BSF"  , 0x500);
   AddBit("BTFSC", 0x600);
   AddBit("BTFSS", 0x700);

   FOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FOrderCnt); InstrZ=0;
   AddF("CLRF" , 0x060);
   AddF("MOVWF", 0x020);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(LitOrders);
   free(AriOrders);
   free(BitOrders);
   free(FOrders);
END

/*-------------------------------------------------------------------------*/

	static Word ROMEnd(void)
BEGIN
   switch (MomCPU-CPU16C54)
    BEGIN
     case D_CPU16C54:
     case D_CPU16C55: return 511;
     case D_CPU16C56: return 1023;
     case D_CPU16C57: return 2047;
     default: return 0;
    END
END

	static Boolean DecodePseudo(void)
BEGIN
   Word Size;
   Boolean ValOK;
   int z;
   TempResult t;
   char *p;
   LongInt MinV,MaxV;

   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,0x1f);
     return True;
    END

   if (Memo("RES"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],Int16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) AND (NOT FirstPassUnknown))
	BEGIN
	 DontPrint=True;
	 CodeLen=Size;
	 if (MakeUseList)
	  if (AddChunk(SegChunks+ActPC,ProgCounter(),CodeLen,ActPC==SegCode)) WrError(90);
	END
      END
     return True;
    END

   if (Memo("DATA"))
    BEGIN
     MaxV=(ActPC==SegCode)?4095:255; MinV=(-((MaxV+1) >> 1));
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       ValOK=True;
       for (z=1; z<=ArgCnt; z++)
	if (ValOK)
	 BEGIN
          FirstPassUnknown=False;
	  EvalExpression(ArgStr[z],&t);
          if ((FirstPassUnknown) AND (t.Typ==TempInt)) t.Contents.Int&=MaxV;
	  switch (t.Typ)
           BEGIN
	    case TempInt:
             if (ChkRange(t.Contents.Int,MinV,MaxV))
              if (ActPC==SegCode) WAsmCode[CodeLen++]=t.Contents.Int & MaxV;
	      else BAsmCode[CodeLen++]=t.Contents.Int & MaxV;
             break;
	    case TempFloat:
	     WrError(1135); ValOK=False; 
             break;
	    case TempString:
	     for (p=t.Contents.Ascii; *p!='\0'; p++)
              if (ActPC==SegCode)
 	       WAsmCode[CodeLen++]=CharTransTable[(Byte) *p];
              else
 	       BAsmCode[CodeLen++]=CharTransTable[(Byte) *p];
	     break;
	    default:
             ValOK=False;
	   END
	 END
       if (NOT ValOK) CodeLen=0;
      END
     return True;
    END

   if (Memo("ZERO"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],Int16,&ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) AND (NOT FirstPassUnknown)) 
        if ((Size << 1)>MaxCodeLen) WrError(1920);
	else
	 BEGIN
	  CodeLen=Size;
	  memset(WAsmCode,0,2*Size);
	 END
      END
     return True;
    END

   return False;
END

	static void MakeCode_16C5X(void)
BEGIN
   Boolean OK;
   Word AdrWord;
   int z;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
	CodeLen=1; WAsmCode[0]=FixedOrders[z].Code;
       END
      return;
     END

   /* nur ein Literal als Argument */

   for (z=0; z<LitOrderCnt; z++)
    if (Memo(LitOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
	AdrWord=EvalIntExpression(ArgStr[1],Int8,&OK);
	if (OK)
	 BEGIN
	  CodeLen=1; WAsmCode[0]=LitOrders[z].Code+(AdrWord & 0xff);
	 END
       END
      return;
     END

   /* W-mit-f-Operationen */

   for (z=0; z<AriOrderCnt; z++)
    if (Memo(AriOrders[z].Name))
     BEGIN
      if ((ArgCnt==0) OR (ArgCnt>2)) WrError(1110);
      else
       BEGIN
	AdrWord=EvalIntExpression(ArgStr[1],UInt5,&OK);
	if (OK)
	 BEGIN
	  ChkSpace(SegData);
	  WAsmCode[0]=AriOrders[z].Code+(AdrWord & 0x1f);
	  if (ArgCnt==1)
	   BEGIN
	    CodeLen=1; WAsmCode[0]+=AriOrders[z].DefaultDir << 5;
	   END
	  else if (strcasecmp(ArgStr[2],"W")==0) CodeLen=1;
	  else if (strcasecmp(ArgStr[2],"F")==0)
	   BEGIN
	    CodeLen=1; WAsmCode[0]+=0x20;
	   END
	  else
	   BEGIN
	    AdrWord=EvalIntExpression(ArgStr[2],UInt1,&OK);
	    if (OK)
             BEGIN
	      CodeLen=1; WAsmCode[0]+=AdrWord << 5;
	     END
	   END
	 END
       END
      return;
     END

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	AdrWord=EvalIntExpression(ArgStr[2],UInt3,&OK);
	if (OK)
	 BEGIN
	  WAsmCode[0]=EvalIntExpression(ArgStr[1],UInt5,&OK);
	  if (OK)
	   BEGIN
	    CodeLen=1; WAsmCode[0]+=BitOrders[z].Code+(AdrWord << 5);
	    ChkSpace(SegData);
	   END
	 END
       END
      return;
     END

   for (z=0; z<FOrderCnt; z++)
    if (Memo(FOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
	AdrWord=EvalIntExpression(ArgStr[1],UInt5,&OK);
	if (OK)
	 BEGIN
	  CodeLen=1; WAsmCode[0]=FOrders[z].Code+AdrWord;
	  ChkSpace(SegData);
	 END
       END
      return;
     END

   if (Memo("TRIS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
	if (ChkRange(AdrWord,5,7))
	 BEGIN
	  CodeLen=1; WAsmCode[0]=0x000+AdrWord;
	  ChkSpace(SegData);
	 END
      END
     return;
    END

   if ((Memo("CALL")) OR (Memo("GOTO")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
	if (AdrWord>ROMEnd()) WrError(1320);
	else if ((Memo("CALL")) AND ((AdrWord & 0x100)!=0)) WrError(1905);
	else
	 BEGIN
	  ChkSpace(SegCode);
	  if (((ProgCounter() ^ AdrWord) & 0x200)!=0)
	   WAsmCode[CodeLen++]=0x4a3+((AdrWord & 0x200) >> 1); /* BCF/BSF 3,5 */
	  if (((ProgCounter() ^ AdrWord) & 0x400)!=0)
	   WAsmCode[CodeLen++]=0x4c3+((AdrWord & 0x400) >> 2); /* BCF/BSF 3,6 */
	  if (Memo("CALL")) WAsmCode[CodeLen++]=0x900+(AdrWord & 0xff);
	  else WAsmCode[CodeLen++]=0xa00+(AdrWord & 0x1ff);
	 END
      END
     return;
    END;

   WrXError(1200,OpPart);
END

	static Boolean ChkPC_16C5X(void)
BEGIN
   Boolean ok;

   switch (ActPC)
    BEGIN
     case SegCode: ok=ProgCounter()<=ROMEnd(); break;
     case SegData: ok=ProgCounter()<=0x1f; break;
     default: ok=False;
    END
   return (ok);
END

	static Boolean IsDef_16C5X(void)
BEGIN
   return Memo("SFR");
END

        static void SwitchFrom_16C5X()
BEGIN
   DeinitFields();
END

	static void SwitchTo_16C5X(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x71; NOPCode=0x000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)+(1<<SegData);
   Grans[SegCode]=2; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_16C5X; ChkPC=ChkPC_16C5X; IsDef=IsDef_16C5X;
   SwitchFrom=SwitchFrom_16C5X; InitFields();
END

	void code16c5x_init(void)
BEGIN
   CPU16C54=AddCPU("16C54",SwitchTo_16C5X);
   CPU16C55=AddCPU("16C55",SwitchTo_16C5X);
   CPU16C56=AddCPU("16C56",SwitchTo_16C5X);
   CPU16C57=AddCPU("16C57",SwitchTo_16C5X);
END
