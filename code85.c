/* code85.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 8080/8085                                                   */
/*                                                                           */
/* Historie: 24.10.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

/*--------------------------------------------------------------------------------------------------*/

typedef struct
         {
          char *Name;
          Boolean May80;
          Byte Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } BaseOrder;

#define FixedOrderCnt 27
#define Op16OrderCnt 22
#define Op8OrderCnt 10
#define ALUOrderCnt 8

static FixedOrder *FixedOrders;
static BaseOrder *Op16Orders;
static BaseOrder *Op8Orders;
static BaseOrder *ALUOrders;

static CPUVar CPU8080,CPU8085;

/*--------------------------------------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Boolean NMay, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].May80=NMay;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddOp16(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=Op16OrderCnt) exit(255);
   Op16Orders[InstrZ].Name=NName;
   Op16Orders[InstrZ++].Code=NCode;
END

        static void AddOp8(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=Op8OrderCnt) exit(255);
   Op8Orders[InstrZ].Name=NName;
   Op8Orders[InstrZ++].Code=NCode;
END                         

        static void AddALU(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name=NName;
   ALUOrders[InstrZ++].Code=NCode;
END           

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("XCHG", True , 0xeb); AddFixed("XTHL", True , 0xe3);
   AddFixed("SPHL", True , 0xf9); AddFixed("PCHL", True , 0xe9);
   AddFixed("RET" , True , 0xc9); AddFixed("RC"  , True , 0xd8);
   AddFixed("RNC" , True , 0xd0); AddFixed("RZ"  , True , 0xc8);
   AddFixed("RNZ" , True , 0xc0); AddFixed("RP"  , True , 0xf0);
   AddFixed("RM"  , True , 0xf8); AddFixed("RPE" , True , 0xe8);
   AddFixed("RPO" , True , 0xe0); AddFixed("RLC" , True , 0x07);
   AddFixed("RRC" , True , 0x0f); AddFixed("RAL" , True , 0x17);
   AddFixed("RAR" , True , 0x1f); AddFixed("CMA" , True , 0x2f);
   AddFixed("STC" , True , 0x37); AddFixed("CMC" , True , 0x3f);
   AddFixed("DAA" , True , 0x27); AddFixed("EI"  , True , 0xfb);
   AddFixed("DI"  , True , 0xf3); AddFixed("NOP" , True , 0x00);
   AddFixed("HLT" , True , 0x76); AddFixed("RIM" , False, 0x20);
   AddFixed("SIM" , False, 0x30);

   Op16Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*Op16OrderCnt); InstrZ=0;
   AddOp16("STA" , 0x32); AddOp16("LDA" , 0x3a);
   AddOp16("SHLD", 0x22); AddOp16("LHLD", 0x2a);
   AddOp16("JMP" , 0xc3); AddOp16("JC"  , 0xda);
   AddOp16("JNC" , 0xd2); AddOp16("JZ"  , 0xca);
   AddOp16("JNZ" , 0xc2); AddOp16("JP"  , 0xf2);
   AddOp16("JM"  , 0xfa); AddOp16("JPE" , 0xea);
   AddOp16("JPO" , 0xe2); AddOp16("CALL", 0xcd);
   AddOp16("CC"  , 0xdc); AddOp16("CNC" , 0xd4);
   AddOp16("CZ"  , 0xcc); AddOp16("CNZ" , 0xc4);
   AddOp16("CP"  , 0xf4); AddOp16("CM"  , 0xfc);
   AddOp16("CPE" , 0xec); AddOp16("CPO" , 0xe4);

   Op8Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*Op8OrderCnt); InstrZ=0;
   AddOp8("IN"  , 0xdb); AddOp8("OUT" , 0xd3);
   AddOp8("ADI" , 0xc6); AddOp8("ACI" , 0xce);
   AddOp8("SUI" , 0xd6); AddOp8("SBI" , 0xde);
   AddOp8("ANI" , 0xe6); AddOp8("XRI" , 0xee);
   AddOp8("ORI" , 0xf6); AddOp8("CPI" , 0xfe);

   ALUOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*ALUOrderCnt); InstrZ=0;
   AddALU("ADD" , 0x80); AddALU("ADC" , 0x88);
   AddALU("SUB" , 0x90); AddALU("SBB" , 0x98);
   AddALU("ANA" , 0xa0); AddALU("XRA" , 0xa8);
   AddALU("ORA" , 0xb0); AddALU("CMP" , 0xb8);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(Op16Orders);
   free(Op8Orders);
   free(ALUOrders);
END

/*--------------------------------------------------------------------------------------------------------*/

        static Boolean DecodeReg8(char *Asc, Byte *Erg)
BEGIN
   static char *RegNames="BCDEHLMA";
   char *p;

   if (strlen(Asc)!=1) return False;
   else
    BEGIN
     p=strchr(RegNames,toupper(*Asc));
     if (p==0) return False;
     else
      BEGIN
       *Erg=p-RegNames; return True;
      END
    END
END

        static Boolean DecodeReg16(char *Asc, Byte *Erg)
BEGIN
   static char *RegNames[4]={"B","D","H","SP"};

   for (*Erg=0; (*Erg)<4; (*Erg)++)
    if (strcasecmp(Asc,RegNames[*Erg])==0) break;

   return ((*Erg)<4);
END

        static Boolean DecodePseudo(void)
BEGIN
   if (Memo("PORT"))
    BEGIN
     CodeEquate(SegIO,0,0xff);
     return True;
    END

   return False;
END

        static void MakeCode_85(void)
BEGIN
   Boolean OK;
   Word AdrWord;
   Byte AdrByte;
   int z;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   /* Anweisungen ohne Operanden */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if ((MomCPU<CPU8085) AND (NOT FixedOrders[z].May80)) WrError(1500);
      else
       BEGIN
        CodeLen=1; BAsmCode[0]=FixedOrders[z].Code;
       END
      return;
     END

   /* ein 16-Bit-Operand */

   for (z=0; z<Op16OrderCnt; z++)
    if (Memo(Op16Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
        if (OK)
         BEGIN
          CodeLen=3; BAsmCode[0]=Op16Orders[z].Code;
          BAsmCode[1]=Lo(AdrWord); BAsmCode[2]=Hi(AdrWord);
          ChkSpace(SegCode);
         END
       END
      return;
     END

   for (z=0; z<Op8OrderCnt; z++)
    if (Memo(Op8Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrByte=EvalIntExpression(ArgStr[1],Int8,&OK);
        if (OK)
         BEGIN
          CodeLen=2; BAsmCode[0]=Op8Orders[z].Code; BAsmCode[1]=AdrByte;
          if (z<2) ChkSpace(SegIO);
         END
       END
      return;
     END

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg8(ArgStr[1],&AdrByte)) WrError(1980);
     else if (NOT DecodeReg8(ArgStr[2],BAsmCode+0)) WrError(1980);
     else
      BEGIN
       BAsmCode[0]+=0x40+(AdrByte << 3);
       if (BAsmCode[0]==0x76) WrError(1760); else CodeLen=1;
      END
     return;
    END

   if (Memo("MVI"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
        BEGIN
         if (NOT DecodeReg8(ArgStr[1],&AdrByte)) WrError(1980);
         else
          BEGIN
           BAsmCode[0]=0x06+(AdrByte << 3); CodeLen=2;
          END
        END
      END
     return;
    END

   if (Memo("LXI"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[2],Int16,&OK);
       if (OK)
        BEGIN
         if (NOT DecodeReg16(ArgStr[1],&AdrByte)) WrError(1980);
         else
          BEGIN
           BAsmCode[0]=0x01+(AdrByte << 4);
           BAsmCode[1]=Lo(AdrWord); BAsmCode[2]=Hi(AdrWord);
           CodeLen=3;
          END
        END
      END
     return;
    END

   if ((Memo("LDAX")) OR (Memo("STAX")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT DecodeReg16(ArgStr[1],&AdrByte)) WrError(1980);
     else switch (AdrByte)
      BEGIN
       case 3:                             /* SP */
        WrError(1135); break;
       case 2:                             /* H --> MOV A,M oder M,A */
        CodeLen=1;
        BAsmCode[0]=(Memo("LDAX")) ? 0x7e : 0x77;
        break;
       default:
        CodeLen=1;
        BAsmCode[0]=0x02+(AdrByte << 4);
        if (Memo("LDAX")) BAsmCode[0]+=8;
        break;
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       if (strcasecmp(ArgStr[1],"PSW")==0) strmaxcpy(ArgStr[1],"SP",255);
       if (NOT DecodeReg16(ArgStr[1],&AdrByte)) WrError(1980);
       else
        BEGIN
         CodeLen=1; BAsmCode[0]=0xc1+(AdrByte << 4);
         if (Memo("PUSH")) BAsmCode[0]+=4;
        END
      END
     return;
    END

   if (Memo("RST"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrByte=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
        BEGIN
         CodeLen=1; BAsmCode[0]=0xc7+(AdrByte << 3);
        END
      END
     return;
    END

   if ((Memo("INR")) OR (Memo("DCR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT DecodeReg8(ArgStr[1],&AdrByte)) WrError(1980);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x04+(AdrByte << 3);
       if (Memo("DCR")) BAsmCode[0]++;
      END
     return;
    END

   if ((Memo("INX")) OR (Memo("DCX")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT DecodeReg16(ArgStr[1],&AdrByte)) WrError(1980);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x03+(AdrByte << 4);
       if (Memo("DCX")) BAsmCode[0]+=8;
      END
     return;
    END

   if (Memo("DAD"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT DecodeReg16(ArgStr[1],&AdrByte)) WrError(1980);
     else
      BEGIN
       CodeLen=1; BAsmCode[0]=0x09+(AdrByte << 4);
      END
     return;
    END

   for (z=0; z<ALUOrderCnt; z++)
    if (Memo(ALUOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (NOT DecodeReg8(ArgStr[1],&AdrByte)) WrError(1980);
      else
       BEGIN
        CodeLen=1; BAsmCode[0]=ALUOrders[z].Code+AdrByte;
       END
      return;
     END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_85(void)
BEGIN
   return (Memo("PORT"));
END

        static void SwitchFrom_85(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_85(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x41; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegIO);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;
   Grans[SegIO  ]=1; ListGrans[SegIO  ]=1; SegInits[SegIO  ]=0;
   SegLimits[SegIO  ] = 0xff;

   MakeCode=MakeCode_85; IsDef=IsDef_85;
   SwitchFrom=SwitchFrom_85; InitFields();
END

        void code85_init(void)
BEGIN
   CPU8080=AddCPU("8080",SwitchTo_85);
   CPU8085=AddCPU("8085",SwitchTo_85);
END
