/* code8008.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Intel 8008                                                  */
/*                                                                           */
/* Historie:  3.12.1998 Grundsteinlegung                                     */
/*            4.12.1998 FixedOrders begonnen                                 */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "headids.h"

/*---------------------------------------------------------------------------*/
/* Variablen */

typedef struct
         {
          Byte Code;
         } FixedOrder;

#define FixedOrderCnt 10
#define ImmOrderCnt 10
#define JmpOrderCnt 10

static CPUVar CPU8008;
static PInstTable InstTable;

static FixedOrder *FixedOrders, *ImmOrders, *JmpOrders;

static char *RegNames = "ABCDEHLM";

/*---------------------------------------------------------------------------*/
/* Parser */

	static Boolean DecodeReg(char *Asc, Byte *Erg)
BEGIN
   char *p;

   if (strlen(Asc) != 2) return False;
 
   p = strchr(RegNames, toupper(*Asc));
   if (p != NULL) *Erg = p - RegNames;
   return (p!= NULL);
END

/*---------------------------------------------------------------------------*/
/* Hilfsdekoder */

	static void DecodeFixed(Word Index)
BEGIN
   if (ArgCnt != 0) WrError(1110);
   else
    BEGIN
     BAsmCode[0] = FixedOrders[Index].Code;
     CodeLen = 1;
    END
END

	static void DecodeImm(Word Index)
BEGIN
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     BAsmCode[1] = EvalIntExpression(ArgStr[1], Int8, &OK);
     if (OK)
      BEGIN
       BAsmCode[0] = ImmOrders[Index].Code;
       CodeLen = 2;
      END
    END
END

	static void DecodeJmp(Word Index)
BEGIN
   Boolean OK;
   Word AdrWord;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     AdrWord = EvalIntExpression(ArgStr[1], UInt14, &OK);
     if (OK)
      BEGIN
       BAsmCode[0] = JmpOrders[Index].Code;
       BAsmCode[1] = Lo(AdrWord);
       BAsmCode[2] = Hi(AdrWord) & 0x3f;
       CodeLen = 3;
       ChkSpace(SegCode);
      END
    END
END

	static void DecodeRST(Word Index)
BEGIN
   Boolean OK;
   Word AdrWord;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     FirstPassUnknown = False;
     AdrWord = EvalIntExpression(ArgStr[1], UInt14, &OK);
     if (FirstPassUnknown) AdrWord &= 0x38;
     if (OK)
      if (ChkRange(AdrWord, 0, 0x38))
       if ((AdrWord & 7) != 0) WrError(1325);
       else
        BEGIN
         BAsmCode[0] = AdrWord + 0x05;
         CodeLen = 1;
         ChkSpace(SegCode);
        END
    END
END

	static void DecodeINP(Word Index)
BEGIN
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     BAsmCode[0] = 0x41 + (EvalIntExpression(ArgStr[1], UInt3, &OK) << 1);
     if (OK)
      BEGIN
       CodeLen = 1;
       ChkSpace(SegIO);
      END
    END
END

	static void DecodeOUT(Word Index)
BEGIN
   Boolean OK;

   if (ArgCnt != 1) WrError(1110);
   else
    BEGIN
     BAsmCode[0] = 0x41 + (EvalIntExpression(ArgStr[1], UInt3, &OK) << 1);
     if (OK)
      BEGIN
       CodeLen = 1;
       ChkSpace(SegIO);
      END
    END
END

	static Boolean DecodePseudo(void)
BEGIN
   return False;
END

/*---------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static int InstrZ;
static char *FlagNames = "CZSP";

	static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
END

	static void AddFixeds(char *NName, Byte NCode, int Pos)
BEGIN
   char Memo[10], *p;
   int z;

   strcpy(Memo, NName); p = strchr(Memo, '*');
   for (z = 0; z < 8; z++)
    BEGIN
     *p = RegNames[z];
     AddFixed(Memo, NCode + (z << Pos));
    END
END

	static void AddImm(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeImm);
END

	static void AddImms(char *NName, Byte NCode, int Pos)
BEGIN
   char Memo[10], *p;
   int z;

   strcpy(Memo, NName); p = strchr(Memo, '*');
   for (z = 0; z < 8; z++)
    BEGIN
     *p = RegNames[z];
     AddImm(Memo, NCode + (z << Pos));
    END
END

	static void AddJmp(char *NName, Byte NCode)
BEGIN
   if (InstrZ >= JmpOrderCnt) exit(255);
   JmpOrders[InstrZ].Code = NCode;
   AddInstTable(InstTable, NName, InstrZ++, DecodeJmp);
END

        static void AddJmps(char *NName, Byte NCode, int Pos)
BEGIN
   char Memo[10], *p;
   int z;
   
   strcpy(Memo, NName); p = strchr(Memo, '*');
   for (z = 0; z < 4; z++) 
    BEGIN
     *p = FlagNames[z];  
     AddJmp(Memo, NCode + (z << Pos));
    END
END

	static void InitFields(void)
BEGIN
   SetDynamicInstTable(InstTable=CreateInstTable(401));

   FixedOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * FixedOrderCnt); InstrZ = 0;
   AddFixeds("L*A", 0xc0, 3); AddFixeds("L*B", 0xc1, 3);
   AddFixeds("L*C", 0xc2, 3); AddFixeds("L*D", 0xc3, 3);
   AddFixeds("L*E", 0xc4, 3); AddFixeds("L*H", 0xc5, 3);
   AddFixeds("L*L", 0xc6, 3); AddFixeds("L*M", 0xc7, 3);

   AddFixeds("IN*", 0x00, 3);
   AddFixeds("DC*", 0x01, 3);
   AddFixeds("AD*", 0x80, 0);
   AddFixeds("AC*", 0x88, 0);
   AddFixeds("SU*", 0x90, 0);
   AddFixeds("SB*", 0x98, 0);
   AddFixeds("NR*", 0xa0, 0);
   AddFixeds("XR*", 0xa8, 0);
   AddFixeds("OR*", 0xb0, 0);
   AddFixeds("CP*", 0xb8, 0);
   AddFixed ("RLC", 0x02);
   AddFixed ("RRC", 0x0a);
   AddFixed ("RAL", 0x12);
   AddFixed ("RAR", 0x1a);

   AddFixed("RET", 0x07); 
   AddFixed("RFC", 0x03); AddFixed("RFZ", 0x0b);
   AddFixed("RFS", 0x13); AddFixed("RFP", 0x1b);
   AddFixed("RTC", 0x23); AddFixed("RTZ", 0x2b);  
   AddFixed("RTS", 0x33); AddFixed("RTP", 0x3b);
   AddFixed("HLT", 0x00);

   ImmOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * ImmOrderCnt); InstrZ = 0;
   AddImms("L*I", 0x06, 3);
   AddImm ("ADI", 0x04);
   AddImm ("ACI", 0x0c);
   AddImm ("SUI", 0x14);
   AddImm ("SBI", 0x1c);
   AddImm ("NDI", 0x24);
   AddImm ("XRI", 0x2c);
   AddImm ("ORI", 0x34);
   AddImm ("CPI", 0x3c);

   JmpOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * JmpOrderCnt); InstrZ = 0;
   AddJmp ("JMP", 0x44);
   AddJmps("JF*", 0x40, 3);
   AddJmps("JT*", 0x60, 3);
   AddJmp ("CAL", 0x46);
   AddJmps("CF*", 0x42, 3);
   AddJmps("CT*", 0x62, 3);

   AddInstTable(InstTable, "RST", 0, DecodeRST);
   AddInstTable(InstTable, "INP", 0, DecodeINP);
   AddInstTable(InstTable, "OUT", 0, DecodeOUT);
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(FixedOrders);
   free(ImmOrders);
   free(JmpOrders);
END

/*---------------------------------------------------------------------------*/
/* Callbacks */

	static void MakeCode_8008(void)
BEGIN
   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* der Rest */

   if (NOT LookupInstTable(InstTable, OpPart)) WrXError(1200, OpPart);
END

        static Boolean IsDef_8008(void)
BEGIN
   return False;
END

        static void SwitchFrom_8008(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_8008(void)
BEGIN
   PFamilyDescr FoundDescr;

   FoundDescr=FindFamilyByName("8008");

   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=FoundDescr->Id; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegIO);
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
   SegLimits[SegCode] = 0x3fff;
   Grans[SegIO   ]=1; ListGrans[SegIO   ]=1; SegInits[SegIO   ]=0;
   SegLimits[SegIO] = 7;

   MakeCode=MakeCode_8008; IsDef=IsDef_8008;
   SwitchFrom=SwitchFrom_8008;

   InitFields();
END

/*---------------------------------------------------------------------------*/
/* Initialisierung */

	void code8008_init(void)
BEGIN
   CPU8008=AddCPU("8008",SwitchTo_8008);
END
