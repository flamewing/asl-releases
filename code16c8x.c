/* code16c8x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator PIC16C8x                                                 */
/*                                                                           */
/* Historie: 21.8.1996 Grundsteinlegung                                      */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 Bookkeeping-Aufruf bei RES                           */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*                      Sonderadressbereich PIC16C84                         */
/*           2. 10.1999 ChkPC wurde nicht angebunden...                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "chunks.h"
#include "strutil.h"
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
          Word Code;
          Byte DefaultDir;
         } AriOrder;

#define D_CPU16C64 0
#define D_CPU16C84 1

#define FixedOrderCnt 7
#define LitOrderCnt 7
#define AriOrderCnt 14
#define BitOrderCnt 4
#define FOrderCnt 2

static FixedOrder *FixedOrders;
static FixedOrder *LitOrders;
static AriOrder *AriOrders;
static FixedOrder *BitOrders;
static FixedOrder *FOrders;

static CPUVar CPU16C64,CPU16C84;

/*--------------------------------------------------------------------------*/

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

        static void AddAri(char *NName, Word NCode, Byte NDir)
BEGIN
   if (InstrZ>=AriOrderCnt) exit(255);
   AriOrders[InstrZ].Name=NName;
   AriOrders[InstrZ].Code=NCode;
   AriOrders[InstrZ++].DefaultDir=NDir;
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
   AddFixed("CLRW"  , 0x0100);
   AddFixed("NOP"   , 0x0000);
   AddFixed("CLRWDT", 0x0064);
   AddFixed("OPTION", 0x0062);
   AddFixed("SLEEP" , 0x0063);
   AddFixed("RETFIE", 0x0009);
   AddFixed("RETURN", 0x0008);
  
   LitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LitOrderCnt); InstrZ=0;
   AddLit("ADDLW", 0x3e00);
   AddLit("ANDLW", 0x3900);
   AddLit("IORLW", 0x3800);
   AddLit("MOVLW", 0x3000);
   AddLit("RETLW", 0x3400);
   AddLit("SUBLW", 0x3c00);
   AddLit("XORLW", 0x3a00);

   AriOrders=(AriOrder *) malloc(sizeof(AriOrder)*AriOrderCnt); InstrZ=0;
   AddAri("ADDWF" , 0x0700, 0);
   AddAri("ANDWF" , 0x0500, 0);
   AddAri("COMF"  , 0x0900, 1);
   AddAri("DECF"  , 0x0300, 1);
   AddAri("DECFSZ", 0x0b00, 1);
   AddAri("INCF"  , 0x0a00, 1);
   AddAri("INCFSZ", 0x0f00, 1);
   AddAri("IORWF" , 0x0400, 0);
   AddAri("MOVF"  , 0x0800, 0);
   AddAri("RLF"   , 0x0d00, 1);
   AddAri("RRF"   , 0x0c00, 1);
   AddAri("SUBWF" , 0x0200, 0);
   AddAri("SWAPF" , 0x0e00, 1);
   AddAri("XORWF" , 0x0600, 0);

   BitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BitOrderCnt); InstrZ=0;
   AddBit("BCF"  , 0x1000);
   AddBit("BSF"  , 0x1400);
   AddBit("BTFSC", 0x1800);
   AddBit("BTFSS", 0x1c00);

   FOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FOrderCnt); InstrZ=0;
   AddF("CLRF" , 0x0180);
   AddF("MOVWF", 0x0080);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(LitOrders);
   free(AriOrders);
   free(BitOrders);
   free(FOrders);
END

/*--------------------------------------------------------------------------*/

        static Word ROMEnd(void)
BEGIN
   switch (MomCPU-CPU16C64)
    BEGIN
     case D_CPU16C64: return 0x7ff;
     case D_CPU16C84: return 0x3ff;
     default: return 0;
    END
END

        static Word EvalFExpression(char *Asc, Boolean *OK)
BEGIN
   LongInt h;

   h=EvalIntExpression(Asc,UInt9,OK);
   if (*OK)
    BEGIN
     ChkSpace(SegData); return (h & 0x7f);
    END
   else return 0;
END

        static Boolean DecodePseudo(void)
BEGIN
   Word Size;
   Boolean ValOK;
   int z;
   char *p;
   TempResult t;
   LongInt MinV,MaxV;

   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,511);
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
         BookKeeping();
        END
      END
     return True;
    END

   if (Memo("DATA"))
    BEGIN
     MaxV=(ActPC==SegCode)?16383:255; MinV=(-((MaxV+1) >> 1));
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
              BEGIN
               if (ActPC==SegCode) WAsmCode[CodeLen++]=t.Contents.Int & MaxV;
               else BAsmCode[CodeLen++]=t.Contents.Int & MaxV;
              END
             break;
            case TempFloat:
             WrError(1135); ValOK=False; 
             break;
            case TempString:
             for (p=t.Contents.Ascii; *p!='\0'; p++)
              if (ActPC==SegCode)
               WAsmCode[CodeLen++]=CharTransTable[((usint) *p)&0xff];
              else
               BAsmCode[CodeLen++]=CharTransTable[((usint) *p)&0xff];
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
        BEGIN
         if (Size << 1>MaxCodeLen) WrError(1920);
         else
          BEGIN
           CodeLen=Size;
           memset(WAsmCode,0,2*Size);
          END
        END
      END
     return True;
    END

   return False;
END

        static void MakeCode_16c8x(void)
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
        if (Memo("OPTION")) WrError(130);
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
        AdrWord=EvalFExpression(ArgStr[1],&OK);
        if (OK)
         BEGIN
          WAsmCode[0]=AriOrders[z].Code+AdrWord;
          if (ArgCnt==1)
           BEGIN
            CodeLen=1; WAsmCode[0]+=AriOrders[z].DefaultDir << 7;
           END
          else if (strcasecmp(ArgStr[2],"W")==0) CodeLen=1;
          else if (strcasecmp(ArgStr[2],"F")==0)
           BEGIN
            CodeLen=1; WAsmCode[0]+=0x80;
           END
          else
           BEGIN
            AdrWord=EvalIntExpression(ArgStr[2],UInt1,&OK);
            if (OK)
             BEGIN
              CodeLen=1; WAsmCode[0]+=AdrWord << 7;
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
          WAsmCode[0]=EvalFExpression(ArgStr[1],&OK);
          if (OK)
           BEGIN
            CodeLen=1; WAsmCode[0]+=BitOrders[z].Code+(AdrWord << 7);
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
        AdrWord=EvalFExpression(ArgStr[1],&OK);
        if (OK)
         BEGIN
          CodeLen=1; WAsmCode[0]=FOrders[z].Code+AdrWord;
         END
       END
      return;
     END

   if (Memo("TRIS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrWord=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (FirstPassUnknown) AdrWord=5;
       if (OK)
        if (ChkRange(AdrWord,5,6))
         BEGIN
          CodeLen=1; WAsmCode[0]=0x0060+AdrWord;
          ChkSpace(SegData); WrError(130);
         END
      END
     return;
    END

   if ((Memo("CALL")) OR (Memo("GOTO")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        BEGIN
         if (AdrWord>ROMEnd()) WrError(1320);
         else
          BEGIN
           ChkSpace(SegCode);
           if (((ProgCounter() ^ AdrWord) & 0x800)!=0)
            WAsmCode[CodeLen++]=0x118a+((AdrWord & 0x800) >> 1); /* BCF/BSF 10,3 */
           if (((ProgCounter() ^ AdrWord) & 0x1000)!=0)
            WAsmCode[CodeLen++]=0x120a+((AdrWord & 0x400) >> 2); /* BCF/BSF 10,4 */
           if (Memo("CALL")) WAsmCode[CodeLen++]=0x2000+(AdrWord & 0x7ff);
           else WAsmCode[CodeLen++]=0x2800+(AdrWord & 0x7ff);
          END
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_16c8x(void)
BEGIN
   return (Memo("SFR"));
END

        static Boolean ChkPC_16c8x(LargeWord Addr)
BEGIN

   if ((ActPC == SegCode) AND (Addr > SegLimits[SegCode]))
    BEGIN
     return ((Addr >= 0x2000) AND (Addr <= 0x2007));
    END
   else return (Addr <= SegLimits[ActPC]);
END

        static void SwitchFrom_16c8x(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_16c8x(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x70; NOPCode=0x0000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)+(1<<SegData);
   Grans[SegCode]=2; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   SegLimits[SegCode] = ROMEnd() + 0x300;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;
   SegLimits[SegData] = 0x1ff;
   ChkPC = ChkPC_16c8x;

   MakeCode=MakeCode_16c8x; IsDef=IsDef_16c8x;
   SwitchFrom=SwitchFrom_16c8x; InitFields();
END

        void code16c8x_init(void)
BEGIN
   CPU16C64=AddCPU("16C64",SwitchTo_16c8x);
   CPU16C84=AddCPU("16C84",SwitchTo_16c8x);
END
