/* code17c4x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator PIC17C4x                                                    */
/*                                                                           */
/* Historie: 21.8.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

/*---------------------------------------------------------------------------*/

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word DefaultDir;
          Word Code;
         } AriOrder;

#define FixedOrderCnt 5
#define LittOrderCnt 8
#define AriOrderCnt 23
#define BitOrderCnt 5
#define FOrderCnt 5


/*---------------------------------------------------------------------------*/

static FixedOrder *FixedOrders;
static FixedOrder *LittOrders;
static AriOrder *AriOrders;
static FixedOrder *BitOrders;
static FixedOrder *FOrders;

static CPUVar CPU17C42;

/*---------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

	static void AddLitt(char *NName, Word NCode)
BEGIN
   if (InstrZ>=LittOrderCnt) exit(255);
   LittOrders[InstrZ].Name=NName;
   LittOrders[InstrZ++].Code=NCode;
END

        static void AddAri(char *NName, Word NDef, Word NCode)
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
   AddFixed("RETFIE", 0x0005);
   AddFixed("RETURN", 0x0002);
   AddFixed("CLRWDT", 0x0004);
   AddFixed("NOP"   , 0x0000);
   AddFixed("SLEEP" , 0x0003);

   LittOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LittOrderCnt); InstrZ=0;
   AddLitt("MOVLB", 0xb800);
   AddLitt("ADDLW", 0xb100);
   AddLitt("ANDLW", 0xb500);
   AddLitt("IORLW", 0xb300);
   AddLitt("MOVLW", 0xb000);
   AddLitt("SUBLW", 0xb200);
   AddLitt("XORLW", 0xb400);
   AddLitt("RETLW", 0xb600);

   AriOrders=(AriOrder *) malloc(sizeof(AriOrder)*AriOrderCnt); InstrZ=0;
   AddAri("ADDWF" , 0, 0x0e00);
   AddAri("ADDWFC", 0, 0x1000);
   AddAri("ANDWF" , 0, 0x0a00);
   AddAri("CLRF"  , 1, 0x2800);
   AddAri("COMF"  , 1, 0x1200);
   AddAri("DAW"   , 1, 0x2e00);
   AddAri("DECF"  , 1, 0x0600);
   AddAri("INCF"  , 1, 0x1400);
   AddAri("IORWF" , 0, 0x0800);
   AddAri("NEGW"  , 1, 0x2c00);
   AddAri("RLCF"  , 1, 0x1a00);
   AddAri("RLNCF" , 1, 0x2200);
   AddAri("RRCF"  , 1, 0x1800);
   AddAri("RRNCF" , 1, 0x2000);
   AddAri("SETF"  , 1, 0x2a00);
   AddAri("SUBWF" , 0, 0x0400);
   AddAri("SUBWFB", 0, 0x0200);
   AddAri("SWAPF" , 1, 0x1c00);
   AddAri("XORWF" , 0, 0x0c00);
   AddAri("DECFSZ", 1, 0x1600);
   AddAri("DCFSNZ", 1, 0x2600);
   AddAri("INCFSZ", 1, 0x1e00);
   AddAri("INFSNZ", 1, 0x2400);

   BitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BitOrderCnt); InstrZ=0;
   AddBit("BCF"  , 0x8800);
   AddBit("BSF"  , 0x8000);
   AddBit("BTFSC", 0x9800);
   AddBit("BTFSS", 0x9000);
   AddBit("BTG"  , 0x3800);

   FOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FOrderCnt); InstrZ=0;
   AddF("MOVWF" , 0x0100);
   AddF("CPFSEQ", 0x3100);
   AddF("CPFSGT", 0x3200);
   AddF("CPFSLT", 0x3000);
   AddF("TSTFSZ", 0x3300);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(LittOrders);
   free(AriOrders);
   free(BitOrders);
   free(FOrders);
END

/*---------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   Word Size;
   Boolean ValOK;
   int z,z2;
   TempResult t;
   LongInt MinV,MaxV;

   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,0xff);
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
     MaxV=(ActPC==SegCode)?65535:255; MinV=(-((MaxV+1) >> 1));
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
	      if (ActPC==SegCode) WAsmCode[CodeLen++]=t.Contents.Int;
              else BAsmCode[CodeLen++]=t.Contents.Int;
             break;
	    case TempFloat:
	     WrError(1135); ValOK=False;
             break;
	    case TempString:
	     for (z2=0; z2<strlen(t.Contents.Ascii); z2++)
              BEGIN
               Size=CharTransTable[(Byte) t.Contents.Ascii[z2]];
               if (ActPC==SegData) BAsmCode[CodeLen++]=Size;
	       else if ((z2&1)==0) WAsmCode[CodeLen++]=Size;
	       else WAsmCode[CodeLen-1]+=Size<<8;
	      END
             break;
	    default: ValOK=False;
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

	static void MakeCode_17c4x(void)
BEGIN
   Boolean OK;
   Word AdrWord;
   int z;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* kein Argument */

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

   /* konstantes Argument */

   for (z=0; z<LittOrderCnt; z++)
    if (Memo(LittOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
	AdrWord=EvalIntExpression(ArgStr[1],Int8,&OK);
	if (OK)
	 BEGIN
	  WAsmCode[0]=LittOrders[z].Code+(AdrWord & 0xff); CodeLen=1;
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
	AdrWord=EvalIntExpression(ArgStr[1],Int8,&OK);
	if (OK)
	 BEGIN
	  ChkSpace(SegData);
	  WAsmCode[0]=AriOrders[z].Code+(AdrWord & 0xff);
	  if (ArgCnt==1)
	   BEGIN
	    CodeLen=1; WAsmCode[0]+=AriOrders[z].DefaultDir << 8;
	   END
	  else if (strcasecmp(ArgStr[2],"W")==0) CodeLen=1;
	  else if (strcasecmp(ArgStr[2],"F")==0)
 	   BEGIN
	    CodeLen=1; WAsmCode[0]+=0x100;
	   END
	  else
	   BEGIN
	    AdrWord=EvalIntExpression(ArgStr[2],UInt1,&OK);
	    if (OK)
	     BEGIN
	      CodeLen=1; WAsmCode[0]+=(AdrWord << 8);
	     END
	   END
	 END
       END
      return;
     END

   /* Bitoperationen */

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	AdrWord=EvalIntExpression(ArgStr[2],UInt3,&OK);
	if (OK)
	 BEGIN
	  WAsmCode[0]=EvalIntExpression(ArgStr[1],Int8,&OK);
	   if (OK)
	    BEGIN
	     CodeLen=1; WAsmCode[0]+=BitOrders[z].Code+(AdrWord << 8);
	     ChkSpace(SegData);
	    END
	 END
       END
      return;
     END

   /* Register als Operand */

   for (z=0; z<FOrderCnt; z++)
    if (Memo(FOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
	AdrWord=EvalIntExpression(ArgStr[1],Int8,&OK);
	if (OK)
	 BEGIN
	  CodeLen=1; WAsmCode[0]=FOrders[z].Code+AdrWord;
	  ChkSpace(SegData);
	 END
       END
      return;
     END

   if ((Memo("MOVFP")) OR (Memo("MOVPF")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (Memo("MOVFP"))
	BEGIN
	 strcpy(ArgStr[3],ArgStr[1]); 
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
	END
       AdrWord=EvalIntExpression(ArgStr[1],UInt5,&OK);
       if (OK)
	BEGIN
	 WAsmCode[0]=EvalIntExpression(ArgStr[2],Int8,&OK);
	 if (OK)
	  BEGIN
	   WAsmCode[0]=Lo(WAsmCode[0])+(AdrWord << 8)+0x4000;
	   if (Memo("MOVFP")) WAsmCode[0]+=0x2000;
	   CodeLen=1;
	  END
	END
      END
     return;
    END

   if ((Memo("TABLRD")) OR (Memo("TABLWT")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       WAsmCode[0]=Lo(EvalIntExpression(ArgStr[3],Int8,&OK));
       if (OK)
	BEGIN
	 AdrWord=EvalIntExpression(ArgStr[2],UInt1,&OK);
	 if (OK)
	  BEGIN
	   WAsmCode[0]+=AdrWord << 8;
	   AdrWord=EvalIntExpression(ArgStr[1],UInt1,&OK);
	   if (OK)
	    BEGIN
	     WAsmCode[0]+=0xa800+(AdrWord << 9);
	     if (Memo("TABLWT")) WAsmCode[0]+=0x400;
	     CodeLen=1;
	    END
	  END
	END
      END;
     return;
    END

   if ((Memo("TLRD")) OR (Memo("TLWT")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       WAsmCode[0]=Lo(EvalIntExpression(ArgStr[2],Int8,&OK));
       if (OK)
	BEGIN
	 AdrWord=EvalIntExpression(ArgStr[1],UInt1,&OK);
	 if (OK)
	  BEGIN
	   WAsmCode[0]+=(AdrWord << 9)+0xa000;
	   if (Memo("TLWT")) WAsmCode[0]+=0x400;
	   CodeLen=1;
	  END
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
	if (((ProgCounter() ^ AdrWord) & 0xe000)!=0) WrError(1910);
	else
	 BEGIN
	  WAsmCode[0]=0xc000+(AdrWord & 0x1fff);
	  if (Memo("CALL")) WAsmCode[0]+=0x2000;
	  CodeLen=1;
	 END
      END
     return;
    END

   if (Memo("LCALL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
	BEGIN
	 CodeLen=3;
	 WAsmCode[0]=0xb000+Hi(AdrWord);
	 WAsmCode[1]=0x0103;
	 WAsmCode[2]=0xb700+Lo(AdrWord);
	END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static Boolean ChkPC_17c4x(void)
BEGIN
   Boolean ok;

   switch (ActPC)
    BEGIN
     case SegCode: ok=ProgCounter()<=0xffff; break;
     case SegData: ok=ProgCounter()<=0xff; break;
     default: ok=False;
    END
   return (ok);
END

	static Boolean IsDef_17c4x(void)
BEGIN
   return Memo("SFR");
END

        static void SwitchFrom_17c4x(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_17c4x(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x72; NOPCode=0x0000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)+(1<<SegData);
   Grans[SegCode]=2; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;

   MakeCode=MakeCode_17c4x; ChkPC=ChkPC_17c4x; IsDef=IsDef_17c4x;
   SwitchFrom=SwitchFrom_17c4x; InitFields();
END

	void code17c4x_init(void)
BEGIN
   CPU17C42=AddCPU("17C42",SwitchTo_17c4x);
END
