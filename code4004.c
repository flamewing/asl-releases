/* code4004.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* <Zweck>                                                                   */
/*                                                                           */
/* Historie: 1.2.1998 Grundsteinlegung                                       */
/*           3.3.1998 weitere Fixed-Befehle hinzugefuegt                     */
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

/*---------------------------------------------------------------------------*/
/* Variablen */

#define FixedOrderCnt 31
#define OneRegOrderCnt 1
#define AccRegOrderCnt 4
#define Imm4OrderCnt 2

typedef struct
         {
          Byte Code;
         } FixedOrder;

static CPUVar CPU4004,CPU4040;

static FixedOrder *FixedOrders;
static FixedOrder *OneRegOrders;
static FixedOrder *AccRegOrders;
static FixedOrder *Imm4Orders;
static PInstTable InstTable;

/*---------------------------------------------------------------------------*/
/* Parser */

	static Boolean DecodeReg(char *Asc, Byte *Erg)
BEGIN
   char *endptr,*s;

   if (FindRegDef(Asc,&s)) Asc=s;

   if (toupper(*Asc)!='R') return False;
   else
    BEGIN
     *Erg=strtol(Asc+1,&endptr,10);
     return ((*endptr=='\0') AND (*Erg<=15));
    END
END

/*---------------------------------------------------------------------------*/
/* Hilfsdekoder */

	static void DecodeFixed(Word Index)
BEGIN
   FixedOrder *Instr=FixedOrders+Index;

   if (ArgCnt!=0) WrError(1110);
   else
    BEGIN
     BAsmCode[0]=Instr->Code; CodeLen=1;
    END
END

	static void DecodeOneReg(Word Index)
BEGIN
   FixedOrder *Instr=OneRegOrders+Index;
   Byte Erg;

   if (ArgCnt!=1) WrError(1110);
   else if (NOT DecodeReg(ArgStr[1],&Erg)) WrXError(1445,ArgStr[1]);
   else
    BEGIN
     BAsmCode[0]=Instr->Code+Erg; CodeLen=1;
    END
END

        static void DecodeAccReg(Word Index)
BEGIN
   FixedOrder *Instr=AccRegOrders+Index;
   Byte Erg;

   if (ArgCnt!=2) WrError(1110);
   else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
   else if (NOT DecodeReg(ArgStr[2],&Erg)) WrXError(1445,ArgStr[2]);
   else
    BEGIN
     BAsmCode[0]=Instr->Code+Erg; CodeLen=1;
    END
END

        static void DecodeImm4(Word Index)
BEGIN
   FixedOrder *Instr=Imm4Orders+Index;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else 
    BEGIN
     BAsmCode[0]=EvalIntExpression(ArgStr[1],UInt4,&OK);
     if (OK)
      BEGIN
       BAsmCode[0]+=Instr->Code; CodeLen=1;
      END
    END
END

	static void DecodeFullJmp(Word Index)
BEGIN
   Word Adr;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     Adr=EvalIntExpression(ArgStr[1],UInt12,&OK);
     if (OK)
      BEGIN
       BAsmCode[0]=0x40+(Index << 4)+Hi(Adr);
       BAsmCode[1]=Lo(Adr);
       CodeLen=2;
      END
    END
END

	static void DecodeISZ(Word Index)
BEGIN
   Word Adr;
   Boolean OK;
   Byte Erg;
   
   if (ArgCnt!=2) WrError(1110);
   else if (NOT DecodeReg(ArgStr[1],&Erg)) WrXError(1445,ArgStr[1]);
   else
    BEGIN
     Adr=EvalIntExpression(ArgStr[1],UInt12,&OK);
     if (OK)
      if ((NOT SymbolQuestionable) AND (Hi(EProgCounter()+1)!=Hi(Adr))) WrError(1910);
      else
       BEGIN
        BAsmCode[0]=0x70+Erg; BAsmCode[1]=Lo(Adr); CodeLen=2;
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

	static void AddFixed(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeFixed);
END

	static void AddOneReg(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=OneRegOrderCnt) exit(255);
   OneRegOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeOneReg);
END

	static void AddAccReg(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=AccRegOrderCnt) exit(255);
   AccRegOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeAccReg);
END

	static void AddImm4(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=Imm4OrderCnt) exit(255);
   Imm4Orders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeImm4);
END

	static void InitFields(void)
BEGIN
   InstTable=CreateInstTable(101);

   InstrZ=0; FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt);
   AddFixed("NOP" ,0x00); AddFixed("WRM" ,0xe0);
   AddFixed("WMP" ,0xe1); AddFixed("WRR" ,0xe2);
   AddFixed("WPM" ,0xe3); AddFixed("WR0" ,0xe4);
   AddFixed("WR1" ,0xe5); AddFixed("WR2" ,0xe6);
   AddFixed("WR3" ,0xe7); AddFixed("SBM" ,0xe8);    
   AddFixed("RDM" ,0xe9); AddFixed("RDR" ,0xea);    
   AddFixed("ADM" ,0xeb); AddFixed("RD0" ,0xec);
   AddFixed("RD1" ,0xed); AddFixed("RD2" ,0xee);    
   AddFixed("RD3" ,0xef); AddFixed("CLB" ,0xf0);
   AddFixed("CLC" ,0xf1); AddFixed("IAC" ,0xf2);
   AddFixed("CMC" ,0xf3); AddFixed("CMA" ,0xf4);    
   AddFixed("RAL" ,0xf5); AddFixed("RAR" ,0xf6);
   AddFixed("TCC" ,0xf7); AddFixed("DAC" ,0xf8);
   AddFixed("TCS" ,0xf9); AddFixed("STC" ,0xfa);
   AddFixed("DAA" ,0xfb); AddFixed("KBP" ,0xfc);
   AddFixed("DCL" ,0xfd);

   InstrZ=0; OneRegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*OneRegOrderCnt);
   AddOneReg("INC" ,0x60);

   InstrZ=0; AccRegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*AccRegOrderCnt);
   AddAccReg("ADD" ,0x80); AddAccReg("SUB" ,0x90);
   AddAccReg("LD"  ,0xa0); AddAccReg("XCH" ,0xb0);

   InstrZ=0; Imm4Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Imm4OrderCnt);
   AddImm4("BBL" ,0xc0); AddImm4("LDM" ,0xd0);

   AddInstTable(InstTable,"JUN",0,DecodeFullJmp);
   AddInstTable(InstTable,"JMS",1,DecodeFullJmp);
   AddInstTable(InstTable,"ISZ",0,DecodeISZ);
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(FixedOrders);
   free(OneRegOrders);
   free(AccRegOrders);
   free(Imm4Orders);
END

/*---------------------------------------------------------------------------*/
/* Callbacks */

	static void MakeCode_4004(void)
BEGIN
   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   WrXError(1200,OpPart);
END

	static Boolean ChkPC_4004(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode : return (ProgCounter() < 4095);
     case SegData : return (ProgCounter() < 255);
     default: return False;
    END
END

        static Boolean IsDef_4004(void)
BEGIN
   return False;
END

        static void SwitchFrom_4004(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_4004(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x3f; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
   Grans[SegData ]=1; ListGrans[SegData ]=1; SegInits[SegCode ]=0;

   MakeCode=MakeCode_4004; ChkPC=ChkPC_4004; IsDef=IsDef_4004;
   SwitchFrom=SwitchFrom_4004;

   InitFields();
END

/*---------------------------------------------------------------------------*/
/* Initialisierung */

	void code4004_init(void)
BEGIN
   CPU4004=AddCPU("4004",SwitchTo_4004);
   CPU4040=AddCPU("4040",SwitchTo_4004);
END