/* code4004.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Intel 4004                                                  */
/*                                                                           */
/* Historie:  1. 2.1998 Grundsteinlegung                                     */
/*            3. 3.1998 weitere Fixed-Befehle hinzugefuegt                   */
/*           28.11.1998 LookupInstTable ;-)                                  */
/*                      JCN FIM...                                           */
/*           29.11.1998 HeaderId symbolisch holen                            */
/*            3.12.1998 DATA schrieb 16-Bit-Ints statt 8 Bit                 */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*           23. 1.1999 / / entfernt                                         */
/*            2. 7.1999 Zus. Befehlsvarianten, andere Registersyntax         */
/*            8. 9.1999 REG fehlte                                           */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*                                                                           */
/*****************************************************************************/
/* $Id: code4004.c,v 1.2 2002/08/14 18:43:48 alfred Exp $                          */
/*****************************************************************************
 * $Log: code4004.c,v $
 * Revision 1.2  2002/08/14 18:43:48  alfred
 * - warn null allocation, remove some warnings
 *
 *****************************************************************************/

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

#define FixedOrderCnt 35
#define OneRegOrderCnt 1
#define OneRRegOrderCnt 3
#define AccRegOrderCnt 4
#define Imm4OrderCnt 2

typedef struct
         {
          Byte Code;
         } FixedOrder;

static CPUVar CPU4004/*,CPU4040*/;

static FixedOrder *FixedOrders;
static FixedOrder *OneRegOrders;
static FixedOrder *OneRRegOrders;
static FixedOrder *AccRegOrders;
static FixedOrder *Imm4Orders;
static PInstTable InstTable;

/*---------------------------------------------------------------------------*/
/* Parser */

        static Byte RegVal(char Inp)
BEGIN
   if ((Inp >='0') AND (Inp <= '9')) return Inp - '0';
   else if ((Inp >='A') AND (Inp <= 'F')) return Inp - 'A' + 10;
   else return 0xff;
END

        static Boolean DecodeReg(char *Asc, Byte *Erg)
BEGIN
   char *s;

   if (FindRegDef(Asc,&s)) Asc=s;

   if ((strlen(Asc) != 2) OR (toupper(*Asc)!='R')) return False;
   else
    BEGIN
     *Erg = RegVal(toupper(Asc[1]));
     return (*Erg != 0xff);
    END
END

        static Boolean DecodeRReg(char *Asc, Byte *Erg)
BEGIN
   Byte h;
   char *s;

   if (FindRegDef(Asc,&s)) Asc = s;

   if ((strlen(Asc) != 4) OR (toupper(*Asc) != 'R') OR (toupper(Asc[2]) != 'R')) return False;
   else
    BEGIN
     *Erg = RegVal(toupper(Asc[1]));
     h = RegVal(toupper(Asc[3]));
     return ((*Erg != 0xff) AND (h != 0xff) AND (h == (*Erg) + 1) AND (Odd(h)));
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

        static void DecodeOneRReg(Word Index)
BEGIN
   FixedOrder *Instr=OneRRegOrders+Index;
   Byte Erg;

   if (ArgCnt!=1) WrError(1110);
   else if (NOT DecodeRReg(ArgStr[1],&Erg)) WrXError(1445,ArgStr[1]);
   else
    BEGIN
     BAsmCode[0]=Instr->Code+Erg; CodeLen=1;
    END
END

        static void DecodeAccReg(Word Index)
BEGIN
   FixedOrder *Instr=AccRegOrders+Index;
   Byte Erg;

   if ((ArgCnt != 2) AND (ArgCnt != 1)) WrError(1110);
   else if ((ArgCnt == 2) AND (strcasecmp(ArgStr[1], "A") != 0)) WrError(1350);
   else if (NOT DecodeReg(ArgStr[ArgCnt], &Erg)) WrXError(1445, ArgStr[ArgCnt]);
   else
    BEGIN
     BAsmCode[0] = Instr->Code + Erg; CodeLen = 1;
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
   UNUSED(Index);
   
   if (ArgCnt!=2) WrError(1110);
   else if (NOT DecodeReg(ArgStr[1],&Erg)) WrXError(1445,ArgStr[1]);
   else
    BEGIN
     Adr=EvalIntExpression(ArgStr[2],UInt12,&OK);
     if (OK)
      BEGIN
       if ((NOT SymbolQuestionable) AND (Hi(EProgCounter()+1)!=Hi(Adr))) WrError(1910);
       else
        BEGIN
         BAsmCode[0]=0x70+Erg; BAsmCode[1]=Lo(Adr); CodeLen=2;
        END
      END
    END
END

        static void DecodeJCN(Word Index)
BEGIN
   Word AdrInt;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else
    BEGIN
     BAsmCode[0] = 0x10;
     if (strcasecmp(ArgStr[1], "Z") == 0) BAsmCode[0] += 4;
     else if (strcasecmp(ArgStr[1], "NZ") == 0) BAsmCode[0] += 12;
     else if (strcasecmp(ArgStr[1], "C") == 0) BAsmCode[0] += 2;
     else if (strcasecmp(ArgStr[1], "NC") == 0) BAsmCode[0] += 10;
     else if (strcasecmp(ArgStr[1], "T") == 0) BAsmCode[0] += 1;
     else if (strcasecmp(ArgStr[1], "NT") == 0) BAsmCode[0] += 9;
     if (BAsmCode[0] == 0x10) WrXError(1360, ArgStr[1]);
     else
      BEGIN
       AdrInt = EvalIntExpression(ArgStr[2], UInt12, &OK);
       if (OK)
        BEGIN
         if ((NOT SymbolQuestionable) AND (Hi(EProgCounter() + 2) != Hi(AdrInt))) WrError(1370);
         else
          BEGIN
           BAsmCode[1] = Lo(AdrInt);
           CodeLen = 2;
          END
        END
      END
    END
END

        static void DecodeFIM(Word Index)
BEGIN
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else if (NOT DecodeRReg(ArgStr[1], BAsmCode)) WrXError(1445, ArgStr[1]);
   else
    BEGIN
     BAsmCode[1] = EvalIntExpression(ArgStr[2], Int8, &OK);
     if (OK)
      BEGIN
       BAsmCode[0] |= 0x20;
       CodeLen = 2;
      END
    END
END

        static Boolean DecodePseudo(void)
BEGIN
   Boolean ValOK;
   Word Size;
   int z, z2;
   TempResult t;
   char Ch;

   if (Memo("DS"))
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
         if (!Size) WrError(290);
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
           BEGIN
            if (ActPC==SegData) t.Contents.Int&=7;
            else t.Contents.Int&=127;
           END
          switch (t.Typ)
           BEGIN
            case TempInt:
             if (ActPC==SegCode)
              BEGIN
               if (NOT RangeCheck(t.Contents.Int,Int8))
                BEGIN
                 WrError(1320); ValOK=False;
                END
               else BAsmCode[CodeLen++]=t.Contents.Int & 0xff;
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
                BAsmCode[CodeLen++]=Ch;
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

   if (Memo("REG"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else AddRegDef(LabPart,ArgStr[1]);
     return True;
    END

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

        static void AddOneRReg(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=OneRRegOrderCnt) exit(255);
   OneRRegOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeOneRReg);
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
   AddFixed("DCL" ,0xfd); AddFixed("AD0" ,0xec);
   AddFixed("AD1" ,0xed); AddFixed("AD2" ,0xee);
   AddFixed("AD3" ,0xef);

   InstrZ=0; OneRegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*OneRegOrderCnt);
   AddOneReg("INC" ,0x60);

   InstrZ=0; OneRRegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*OneRRegOrderCnt);
   AddOneRReg("SRC" ,0x21);
   AddOneRReg("FIN" ,0x30);
   AddOneRReg("JIN" ,0x31);

   InstrZ=0; AccRegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*AccRegOrderCnt);
   AddAccReg("ADD" ,0x80); AddAccReg("SUB" ,0x90);
   AddAccReg("LD"  ,0xa0); AddAccReg("XCH" ,0xb0);

   InstrZ=0; Imm4Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Imm4OrderCnt);
   AddImm4("BBL" ,0xc0); AddImm4("LDM" ,0xd0);

   AddInstTable(InstTable,"JCN", 0, DecodeJCN);
   AddInstTable(InstTable,"JCM", 0, DecodeJCN);
   AddInstTable(InstTable,"JUN", 0, DecodeFullJmp);
   AddInstTable(InstTable,"JMS", 1, DecodeFullJmp);
   AddInstTable(InstTable,"ISZ", 0, DecodeISZ);
   AddInstTable(InstTable,"FIM", 0, DecodeFIM);
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(FixedOrders);
   free(OneRegOrders);
   free(OneRRegOrders);
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

   /* der Rest */

   if (NOT LookupInstTable(InstTable, OpPart)) WrXError(1200, OpPart);
END

        static Boolean IsDef_4004(void)
BEGIN
   return Memo("REG");
END

        static void SwitchFrom_4004(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_4004(void)
BEGIN
   PFamilyDescr FoundDescr;

   FoundDescr=FindFamilyByName("4004/4040");

   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=FoundDescr->Id; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
   SegLimits[SegCode] = 0xfff;
   Grans[SegData ]=1; ListGrans[SegData ]=1; SegInits[SegData ]=0;
   SegLimits[SegData] = 0xff;

   MakeCode=MakeCode_4004; IsDef=IsDef_4004;
   SwitchFrom=SwitchFrom_4004;

   InitFields();
END

/*---------------------------------------------------------------------------*/
/* Initialisierung */

        void code4004_init(void)
BEGIN
   CPU4004=AddCPU("4004",SwitchTo_4004);
#if 0
   CPU4040=AddCPU("4040",SwitchTo_4004);
#endif
END
