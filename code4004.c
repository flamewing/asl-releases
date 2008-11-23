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
/* $Id: code4004.c,v 1.5 2008/11/23 10:39:16 alfred Exp $                          */
/*****************************************************************************
 * $Log: code4004.c,v $
 * Revision 1.5  2008/11/23 10:39:16  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.4  2007/11/24 22:48:04  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 16:53:41  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2005/05/21 16:35:04  alfred
 * - removed variables available globally
 *
 * Revision 1.1  2003/11/06 02:49:20  alfred
 * - recreated
 *
 * Revision 1.5  2003/05/24 21:16:59  alfred
 * - added 4040
 *
 * Revision 1.4  2003/05/02 21:23:10  alfred
 * - strlen() updates
 *
 * Revision 1.3  2003/02/01 18:04:53  alfred
 * - fixed JCN arguments
 *
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
#include "codevars.h"
#include "headids.h"

/*---------------------------------------------------------------------------*/
/* Variablen */

#define FixedOrderCnt (35+14)
#define OneRegOrderCnt 1
#define OneRRegOrderCnt 3
#define AccRegOrderCnt 4
#define Imm4OrderCnt 2

typedef struct
         {
          Byte Code;
          CPUVar MinCPU;
         } FixedOrder;

static CPUVar CPU4004, CPU4040;

static FixedOrder *FixedOrders;
static FixedOrder *OneRegOrders;
static FixedOrder *OneRRegOrders;
static FixedOrder *AccRegOrders;
static FixedOrder *Imm4Orders;

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

   if ((strlen(Asc) != 2) OR (mytoupper(*Asc)!='R')) return False;
   else
    BEGIN
     *Erg = RegVal(mytoupper(Asc[1]));
     return (*Erg != 0xff);
    END
END

        static Boolean DecodeRReg(char *Asc, Byte *Erg)
BEGIN
   Byte h;
   char *s;

   if (FindRegDef(Asc,&s)) Asc = s;

   if ((strlen(Asc) != 4) OR (mytoupper(*Asc) != 'R') OR (mytoupper(Asc[2]) != 'R')) return False;
   else
    BEGIN
     *Erg = RegVal(mytoupper(Asc[1]));
     h = RegVal(mytoupper(Asc[3]));
     return ((*Erg != 0xff) AND (h != 0xff) AND (h == (*Erg) + 1) AND (Odd(h)));
    END
END

/*---------------------------------------------------------------------------*/
/* Hilfsdekoder */

static void DecodeFixed(Word Index)
{
  FixedOrder *Instr=FixedOrders+Index;

  if (ArgCnt != 0) WrError(1110);
  else if (MomCPU < Instr->MinCPU) WrError(1500);
  else
  {
    BAsmCode[0]=Instr->Code; CodeLen=1;
  }
}

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
{
   Word AdrInt;
   Boolean OK;
   char *pCond;

   UNUSED(Index);

   if (ArgCnt != 2) WrError(1110);
   else
   {
     BAsmCode[0] = 0x10;
     for (pCond = ArgStr[1]; *pCond; pCond++)
       switch (mytoupper(*pCond))
       {
         case 'Z': BAsmCode[0] |= 4; break;
         case 'C': BAsmCode[0] |= 2; break;
         case 'T': BAsmCode[0] |= 1; break;
         case 'N': BAsmCode[0] |= 8; break;
         default: BAsmCode[0] = 0xff;
       }
     if (BAsmCode[0] == 0xff) WrXError(1360, ArgStr[1]);
     else
     {
       AdrInt = EvalIntExpression(ArgStr[2], UInt12, &OK);
       if (OK)
       {
         if ((NOT SymbolQuestionable) AND (Hi(EProgCounter() + 2) != Hi(AdrInt))) WrError(1370);
         else
         {
           BAsmCode[1] = Lo(AdrInt);
           CodeLen = 2;
         }
       }
     }
   }
}

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
             for (z2 = 0; z2 < (int)t.Contents.Ascii.Length; z2++)
             {
               Ch = CharTransTable[((usint) t.Contents.Ascii.Contents[z2]) & 0xff];
               if (ActPC == SegCode)
                 BAsmCode[CodeLen++] = Ch;
               else
               {
                 BAsmCode[CodeLen++] = Ch >> 4;
                 BAsmCode[CodeLen++] = Ch & 15;
               }
             }
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

static void AddFixed(char *NName, Byte NCode, CPUVar NMin)
{
  if (InstrZ >= FixedOrderCnt) exit(255);

  FixedOrders[InstrZ].Code = NCode;
  FixedOrders[InstrZ].MinCPU = NMin;
   
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

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
   AddFixed("NOP" ,0x00, CPU4004); AddFixed("WRM" ,0xe0, CPU4004);
   AddFixed("WMP" ,0xe1, CPU4004); AddFixed("WRR" ,0xe2, CPU4004);
   AddFixed("WPM" ,0xe3, CPU4004); AddFixed("WR0" ,0xe4, CPU4004);
   AddFixed("WR1" ,0xe5, CPU4004); AddFixed("WR2" ,0xe6, CPU4004);
   AddFixed("WR3" ,0xe7, CPU4004); AddFixed("SBM" ,0xe8, CPU4004);
   AddFixed("RDM" ,0xe9, CPU4004); AddFixed("RDR" ,0xea, CPU4004);
   AddFixed("ADM" ,0xeb, CPU4004); AddFixed("RD0" ,0xec, CPU4004);
   AddFixed("RD1" ,0xed, CPU4004); AddFixed("RD2" ,0xee, CPU4004);
   AddFixed("RD3" ,0xef, CPU4004); AddFixed("CLB" ,0xf0, CPU4004);
   AddFixed("CLC" ,0xf1, CPU4004); AddFixed("IAC" ,0xf2, CPU4004);
   AddFixed("CMC" ,0xf3, CPU4004); AddFixed("CMA" ,0xf4, CPU4004);
   AddFixed("RAL" ,0xf5, CPU4004); AddFixed("RAR" ,0xf6, CPU4004);
   AddFixed("TCC" ,0xf7, CPU4004); AddFixed("DAC" ,0xf8, CPU4004);
   AddFixed("TCS" ,0xf9, CPU4004); AddFixed("STC" ,0xfa, CPU4004);
   AddFixed("DAA" ,0xfb, CPU4004); AddFixed("KBP" ,0xfc, CPU4004);
   AddFixed("DCL" ,0xfd, CPU4004); AddFixed("AD0" ,0xec, CPU4004);
   AddFixed("AD1" ,0xed, CPU4004); AddFixed("AD2" ,0xee, CPU4004);
   AddFixed("AD3" ,0xef, CPU4004);

   AddFixed("HLT" ,0x01, CPU4040); AddFixed("BBS" ,0x02, CPU4040);
   AddFixed("LCR" ,0x03, CPU4040); AddFixed("OR4" ,0x04, CPU4040);
   AddFixed("OR5" ,0x05, CPU4040); AddFixed("AN6" ,0x06, CPU4040);
   AddFixed("AN7" ,0x07, CPU4040); AddFixed("DB0" ,0x08, CPU4040);
   AddFixed("DB1" ,0x09, CPU4040); AddFixed("SB0" ,0x0a, CPU4040);
   AddFixed("SB1" ,0x0b, CPU4040); AddFixed("EIN" ,0x0c, CPU4040);
   AddFixed("DIN" ,0x0d, CPU4040); AddFixed("RPM" ,0x0e, CPU4040);

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
   CPU4040=AddCPU("4040",SwitchTo_4004);
END
