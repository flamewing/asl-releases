/* code4500.c */ 
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MELPS-4500                                                  */
/*                                                                           */
/* Historie: 31.12.1996 (23.44!!) Grundsteinlegung                           */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 BookKeeping-Aufruf bei RES                           */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

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
          IntType Max;
         } ConstOrder;


#define FixedOrderCount 79
#define ConstOrderCount 12


static CPUVar CPU4500;

static FixedOrder *FixedOrders;
static ConstOrder *ConstOrders;

/*---------------------------------------------------------------------------*/

	static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

	static void AddConst(char *NName, Word NCode, IntType NMax)
BEGIN
   if (InstrZ>=ConstOrderCount) exit(255);
   ConstOrders[InstrZ].Name=NName;
   ConstOrders[InstrZ].Code=NCode;
   ConstOrders[InstrZ++].Max=NMax;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("AM"  ,0x00a);  AddFixed("AMC" ,0x00b);  AddFixed("AND" ,0x018);
   AddFixed("CLD" ,0x011);  AddFixed("CMA" ,0x01c);  AddFixed("DEY" ,0x017);
   AddFixed("DI"  ,0x004);  AddFixed("EI"  ,0x005);  AddFixed("IAP0",0x260);
   AddFixed("IAP1",0x261);  AddFixed("IAP2",0x262);  AddFixed("IAP3",0x263);
   AddFixed("IAP4",0x264);  AddFixed("INY" ,0x013);  AddFixed("NOP" ,0x000);
   AddFixed("OR"  ,0x019);  AddFixed("OP0A",0x220);  AddFixed("OP1A",0x221);
   AddFixed("POF" ,0x002);  AddFixed("POF2",0x008);  AddFixed("RAR" ,0x01d);
   AddFixed("RC"  ,0x006);  AddFixed("RC3" ,0x2ac);  AddFixed("RC4" ,0x2ae);
   AddFixed("RD"  ,0x014);  AddFixed("RT"  ,0x044);  AddFixed("RTI" ,0x046);
   AddFixed("RTS" ,0x045);  AddFixed("SC"  ,0x007);  AddFixed("SC3" ,0x2ad);
   AddFixed("SC4" ,0x2af);  AddFixed("SD"  ,0x015);  AddFixed("SEAM",0x026);
   AddFixed("SNZ0",0x038);  AddFixed("SNZP",0x003);  AddFixed("SNZT1",0x280);
   AddFixed("SNZT2",0x281); AddFixed("SNZT3",0x282); AddFixed("SPCR",0x299);
   AddFixed("STCR",0x298);  AddFixed("SZC" ,0x02f);  AddFixed("T1R1",0x2ab);
   AddFixed("T3AB",0x232);  AddFixed("TAB" ,0x01e);  AddFixed("TAB3",0x272);
   AddFixed("TABE",0x02a);  AddFixed("TAD" ,0x051);  AddFixed("TAI1",0x253);
   AddFixed("TAL1",0x24a);  AddFixed("TAMR",0x252);  AddFixed("TASP",0x050);
   AddFixed("TAV1",0x054);  AddFixed("TAW1",0x24b);  AddFixed("TAW2",0x24c);
   AddFixed("TAW3",0x24d);  AddFixed("TAX" ,0x052);  AddFixed("TAY" ,0x01f);
   AddFixed("TAZ" ,0x053);  AddFixed("TBA" ,0x00e);  AddFixed("TC1A",0x2a8);
   AddFixed("TC2A",0x2a9);  AddFixed("TDA" ,0x029);  AddFixed("TEAB",0x01a);
   AddFixed("TI1A",0x217);  AddFixed("TL1A",0x20a);  AddFixed("TL2A",0x20b);
   AddFixed("TL3A",0x20c);  AddFixed("TLCA",0x20d);  AddFixed("TMRA",0x216);
   AddFixed("TPTA",0x2a5);  AddFixed("TPAA",0x2aa);  AddFixed("TR1A",0x2a6);
   AddFixed("TR1AB",0x23f); AddFixed("TV1A",0x03f);  AddFixed("TW1A",0x20e);
   AddFixed("TW2A",0x20f);  AddFixed("TW3A",0x210);  AddFixed("TYA" ,0x00c);
   AddFixed("WRST",0x2a0);

   ConstOrders=(ConstOrder *) malloc(sizeof(ConstOrder)*ConstOrderCount); InstrZ=0;
   AddConst("A"   ,0x060,UInt4);  AddConst("LA"  ,0x070,UInt4);
   AddConst("LZ"  ,0x048,UInt2);  AddConst("RB"  ,0x04c,UInt2);
   AddConst("SB"  ,0x05c,UInt2);  AddConst("SZB" ,0x020,UInt2);
   AddConst("TABP",0x080,UInt6);  AddConst("TAM" ,0x2c0,UInt4);
   AddConst("TMA" ,0x2b0,UInt4);  AddConst("XAM" ,0x2d0,UInt4);
   AddConst("XAMD",0x2f0,UInt4);  AddConst("XAMI",0x2e0,UInt4);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(ConstOrders);
END

/*-------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   Boolean ValOK;
   Word Size,z,z2;
   TempResult t;
   char Ch;

   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,415);
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
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       ValOK=True;
       for (z=1; z<=ArgCnt; z++)
	if (ValOK)
	 BEGIN
          FirstPassUnknown=False;
	  EvalExpression(ArgStr[z],&t);
          if ((t.Typ==TempInt) AND (FirstPassUnknown))
           if (ActPC==SegData) t.Contents.Int&=7; else t.Contents.Int&=511;
	  switch (t.Typ)
           BEGIN
            case TempInt:
             if (ActPC==SegCode)
              BEGIN
               if (NOT RangeCheck(t.Contents.Int,Int10))
                BEGIN
                 WrError(1320); ValOK=False;
                END
               else WAsmCode[CodeLen++]=t.Contents.Int & 0x3ff;
              END
             else
              BEGIN
               if (NOT RangeCheck(t.Contents.Int,Int4))
                BEGIN
                 WrError(1320); ValOK=False;
                END
               else BAsmCode[CodeLen++]=t.Contents.Int & 0x0f;
              END
             break;
            case TempFloat:
             WrError(1135); ValOK=False;
             break;
            case TempString:
             for (z2=0; z2<strlen(t.Contents.Ascii); z2++)
              BEGIN
               Ch=CharTransTable[((usint) t.Contents.Ascii[z2])&0xff];
               if (ActPC==SegCode)
                WAsmCode[CodeLen++]=Ch;
               else
                BEGIN
                 BAsmCode[CodeLen++]=Ch >> 4;
                 BAsmCode[CodeLen++]=Ch & 15;
                END
              END
             break;
	    default:
             ValOK=False;
	   END
	 END
       if (NOT ValOK) CodeLen=0;
      END
     return True;
    END

   return False;
END

        static void MakeCode_4500(void)
BEGIN
   int z;
   Word AdrWord;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        CodeLen=1; WAsmCode[0]=FixedOrders[z].Code;
       END
      return;
     END

   if (Memo("SZD"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else
      BEGIN
       CodeLen=2; WAsmCode[0]=0x024; WAsmCode[1]=0x02b;
      END
     return;
    END

   for (z=0; z<ConstOrderCount; z++)
    if (Memo(ConstOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        WAsmCode[0]=EvalIntExpression(ArgStr[1],ConstOrders[z].Max,&OK);
        if (OK)
         BEGIN
          CodeLen=1; WAsmCode[0]+=ConstOrders[z].Code;
         END
       END
      return;
     END

   if (Memo("SEA"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       WAsmCode[1]=EvalIntExpression(ArgStr[1],UInt4,&OK);
       if (OK)
        BEGIN
         CodeLen=2; WAsmCode[1]+=0x070; WAsmCode[0]=0x025;
        END
      END
     return;
    END

   if (Memo("B"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],UInt13,&OK);
       if (OK)
        if ((NOT SymbolQuestionable) AND ((EProgCounter() >> 7)!=(AdrWord >> 7))) WrError(1910);
        else
         BEGIN
          CodeLen=1; WAsmCode[0]=0x180+(AdrWord&0x7f);
         END
      END
     return;
    END

   if ((Memo("BL")) OR (Memo("BML")))
    BEGIN
     if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) AdrWord=EvalIntExpression(ArgStr[1],UInt13,&OK);
       else
        BEGIN
         AdrWord=EvalIntExpression(ArgStr[1],UInt6,&OK) << 7;
         if (OK) AdrWord+=EvalIntExpression(ArgStr[2],UInt7,&OK);
        END;
       if (OK)
        BEGIN
         CodeLen=2;
	 WAsmCode[1]=0x200+(AdrWord & 0x7f)+((AdrWord >> 12) << 7);
	 WAsmCode[0]=0x0c0+(Ord(Memo("BL")) << 5)+((AdrWord >> 7) & 0x1f);
        END
      END
     return;
    END

   if ((Memo("BLA")) OR (Memo("BMLA")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],UInt6,&OK);
       if (OK)
        BEGIN
         CodeLen=2;
	 WAsmCode[1]=0x200+(AdrWord & 0x0f)+((AdrWord & 0x30) << 2);
	 WAsmCode[0]=0x010+(Ord(Memo("BMLA")) << 5);
        END
      END
     return;
    END

   if (Memo("BM"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1],UInt13,&OK);
       if (OK)
	if ((AdrWord >> 7)!=2) WrError(1905);
        else
         BEGIN
         CodeLen=1;
	 WAsmCode[0]=0x100+(AdrWord & 0x7f);
        END
      END
     return;
    END

   if (Memo("LXY"))
    BEGIN
     if ((ArgCnt==0) OR (ArgCnt>2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1) AdrWord=EvalIntExpression(ArgStr[1],Int8,&OK);
       else
        BEGIN
         AdrWord=EvalIntExpression(ArgStr[1],Int4,&OK) << 4;
         if (OK) AdrWord+=EvalIntExpression(ArgStr[2],Int4,&OK);
	END
       if (OK)
        BEGIN
         CodeLen=1;
         WAsmCode[0]=0x300+AdrWord;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_4500(void)
BEGIN
   return (Memo("SFR"));
END

        static void SwitchFrom_4500(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_4500(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x12; NOPCode=0x000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode ]=2; ListGrans[SegCode ]=2; SegInits[SegCode ]=0;
   SegLimits[SegCode] = 0x1fff;
   Grans[SegData ]=1; ListGrans[SegData ]=1; SegInits[SegData ]=0;
   SegLimits[SegData] = 415;

   MakeCode=MakeCode_4500; IsDef=IsDef_4500;
   SwitchFrom=SwitchFrom_4500;

   InitFields();
END

	void code4500_init(void)
BEGIN
   CPU4500=AddCPU("MELPS4500" ,SwitchTo_4500);
END
