/* code6805.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 68(HC)05/08                                                 */
/*                                                                           */
/* Historie:  9.10.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*           13. 3.2000 Adressraum fuer HC08 jetzt 64K                       */
/*           2001-09-03 added warning message about X-indexed conversion     */
/*           2001-09-03 added inx as alias for incx                          */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "bpemu.h"
#include "strutil.h"

#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Byte Code;
         } BaseOrder;

typedef struct 
         {
          char *Name;
          CPUVar MinCPU;
          Byte Code;
          Word Mask;
          ShortInt Size;
         } ALUOrder;

typedef struct 
         {
          char *Name;
          CPUVar MinCPU;
          Byte Code;
          Word Mask;
         } RMWOrder;

#define FixedOrderCnt 53
#define RelOrderCnt 23
#define ALUOrderCnt 19
#define RMWOrderCnt 12

#define ModNone (-1)
#define ModImm 0
#define MModImm (1 << ModImm)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModExt 2
#define MModExt (1 << ModExt)
#define ModIx2 3
#define MModIx2 (1 << ModIx2)
#define ModIx1 4
#define MModIx1 (1 << ModIx1)
#define ModIx 5
#define MModIx  (1 << ModIx)
#define ModSP2 6
#define MModSP2 (1 << ModSP2)
#define ModSP1 7
#define MModSP1 (1 << ModSP1)
#define ModIxP 8
#define MModIxP (1 << ModIxP)
#define MMod05 (MModImm+MModDir+MModExt+MModIx2+MModIx1+MModIx)
#define MMod08 (MModSP2+MModSP1+MModIxP)

static ShortInt AdrMode,OpSize;
static Byte AdrVals[2];

static CPUVar CPU6805,CPU6808;

static BaseOrder *FixedOrders;
static BaseOrder *RelOrders;
static RMWOrder *RMWOrders;
static ALUOrder *ALUOrders;

/*--------------------------------------------------------------------------*/

        static void AddFixed(char *NName, CPUVar NMin, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddRel(char *NName, CPUVar NMin, Byte NCode)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ].MinCPU=NMin;
   RelOrders[InstrZ++].Code=NCode;
END

        static void AddALU(char *NName, CPUVar NMin, Byte NCode, Word NMask, ShortInt NSize)
BEGIN
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name=NName;
   ALUOrders[InstrZ].MinCPU=NMin;
   ALUOrders[InstrZ].Code=NCode;
   ALUOrders[InstrZ].Mask=NMask;
   ALUOrders[InstrZ++].Size=NSize;
END

        static void AddRMW(char *NName, CPUVar NMin, Byte NCode ,Word NMask)
BEGIN
   if (InstrZ>=RMWOrderCnt) exit(255);
   RMWOrders[InstrZ].Name=NName;
   RMWOrders[InstrZ].MinCPU=NMin;
   RMWOrders[InstrZ].Code=NCode;
   RMWOrders[InstrZ++].Mask=NMask;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("RTI" ,CPU6805,0x80); AddFixed("RTS" ,CPU6805,0x81);
   AddFixed("SWI" ,CPU6805,0x83); AddFixed("TAX" ,CPU6805,0x97);
   AddFixed("CLC" ,CPU6805,0x98); AddFixed("SEC" ,CPU6805,0x99);
   AddFixed("CLI" ,CPU6805,0x9a); AddFixed("SEI" ,CPU6805,0x9b);
   AddFixed("RSP" ,CPU6805,0x9c); AddFixed("NOP" ,CPU6805,0x9d);
   AddFixed("TXA" ,CPU6805,0x9f); AddFixed("NEGA",CPU6805,0x40);
   AddFixed("NEGX",CPU6805,0x50); AddFixed("COMA",CPU6805,0x43);
   AddFixed("COMX",CPU6805,0x53); AddFixed("LSRA",CPU6805,0x44);
   AddFixed("LSRX",CPU6805,0x54); AddFixed("RORA",CPU6805,0x46);
   AddFixed("RORX",CPU6805,0x56); AddFixed("ASRA",CPU6805,0x47);
   AddFixed("ASRX",CPU6805,0x57); AddFixed("ASLA",CPU6805,0x48);
   AddFixed("ASLX",CPU6805,0x58); AddFixed("LSLA",CPU6805,0x48);
   AddFixed("LSLX",CPU6805,0x58); AddFixed("ROLA",CPU6805,0x49);
   AddFixed("ROLX",CPU6805,0x59); AddFixed("DECA",CPU6805,0x4a);
   AddFixed("DECX",CPU6805,0x5a); AddFixed("INCA",CPU6805,0x4c);
   AddFixed("INCX",CPU6805,0x5c); AddFixed("TSTA",CPU6805,0x4d);
   AddFixed("TSTX",CPU6805,0x5d); AddFixed("CLRA",CPU6805,0x4f);
   AddFixed("CLRX",CPU6805,0x5f); AddFixed("CLRH",CPU6808,0x8c);
   AddFixed("DAA" ,CPU6808,0x72); AddFixed("DIV" ,CPU6808,0x52);
   AddFixed("MUL" ,CPU6805,0x42); AddFixed("NSA" ,CPU6808,0x62);
   AddFixed("PSHA",CPU6808,0x87); AddFixed("PSHH",CPU6808,0x8b);
   AddFixed("PSHX",CPU6808,0x89); AddFixed("PULA",CPU6808,0x86);
   AddFixed("PULH",CPU6808,0x8a); AddFixed("PULX",CPU6808,0x88);
   AddFixed("STOP",CPU6805,0x8e); AddFixed("TAP" ,CPU6808,0x84);
   AddFixed("TPA" ,CPU6808,0x85); AddFixed("TSX" ,CPU6808,0x95);
   AddFixed("TXS" ,CPU6808,0x94); AddFixed("WAIT",CPU6805,0x8f);
   AddFixed("INX" ,CPU6805,0x5c); 

   RelOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*RelOrderCnt); InstrZ=0;
   AddRel("BRA" ,CPU6805,0x20);   AddRel("BRN" ,CPU6805,0x21);
   AddRel("BHI" ,CPU6805,0x22);   AddRel("BLS" ,CPU6805,0x23);
   AddRel("BCC" ,CPU6805,0x24);   AddRel("BCS" ,CPU6805,0x25);
   AddRel("BNE" ,CPU6805,0x26);   AddRel("BEQ" ,CPU6805,0x27);
   AddRel("BHCC",CPU6805,0x28);   AddRel("BHCS",CPU6805,0x29);
   AddRel("BPL" ,CPU6805,0x2a);   AddRel("BMI" ,CPU6805,0x2b);
   AddRel("BMC" ,CPU6805,0x2c);   AddRel("BMS" ,CPU6805,0x2d);
   AddRel("BIL" ,CPU6805,0x2e);   AddRel("BIH" ,CPU6805,0x2f);
   AddRel("BSR" ,CPU6805,0xad);   AddRel("BGE" ,CPU6808,0x90);
   AddRel("BGT" ,CPU6808,0x92);   AddRel("BHS" ,CPU6805,0x24);
   AddRel("BLE" ,CPU6808,0x93);   AddRel("BLO" ,CPU6805,0x25);
   AddRel("BLT" ,CPU6808,0x91);

   ALUOrders=(ALUOrder *) malloc(sizeof(ALUOrder)*ALUOrderCnt); InstrZ=0;
   AddALU("SUB" ,CPU6805,0x00,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("CMP" ,CPU6805,0x01,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("SBC" ,CPU6805,0x02,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("CPX" ,CPU6805,0x03,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("AND" ,CPU6805,0x04,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("BIT" ,CPU6805,0x05,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("LDA" ,CPU6805,0x06,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("STA" ,CPU6805,0x07,        MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("EOR" ,CPU6805,0x08,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("ADC" ,CPU6805,0x09,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("ORA" ,CPU6805,0x0a,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("ADD" ,CPU6805,0x0b,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("JMP" ,CPU6805,0x0c,        MModDir+MModExt+MModIx+MModIx1+MModIx2                ,-1);
   AddALU("JSR" ,CPU6805,0x0d,        MModDir+MModExt+MModIx+MModIx1+MModIx2                ,-1);
   AddALU("LDX" ,CPU6805,0x0e,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("STX" ,CPU6805,0x0f,        MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU("CPHX",CPU6808,0xc5,MModImm+MModDir                                               ,1);
   AddALU("LDHX",CPU6808,0xa5,MModImm+MModDir                                               ,1);
   AddALU("STHX",CPU6808,0x85,        MModDir                                               ,1);

   RMWOrders=(RMWOrder *) malloc(sizeof(RMWOrder)*RMWOrderCnt); InstrZ=0;
   AddRMW("NEG",CPU6805,0x00,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("COM",CPU6805,0x03,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("LSR",CPU6805,0x04,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("ROR",CPU6805,0x06,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("ASR",CPU6805,0x07,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("ASL",CPU6805,0x08,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("LSL",CPU6805,0x08,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("ROL",CPU6805,0x09,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("DEC",CPU6805,0x0a,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("INC",CPU6805,0x0c,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("TST",CPU6805,0x0d,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW("CLR",CPU6805,0x0f,MModDir+       MModIx+MModIx1+        MModSP1        );
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(RelOrders);
   free(ALUOrders);
   free(RMWOrders);
END

/*--------------------------------------------------------------------------*/


        static void ChkZero(char *s, char *serg, Byte *Erg)
BEGIN
   if (*s=='<')
    BEGIN
     strcpy(serg,s+1); *Erg=2;
    END
   else if (*s=='>')
    BEGIN
     strcpy(serg,s+1); *Erg=1;
    END
   else
    BEGIN
     strcpy(serg,s); *Erg=0;
    END
END

        static void ChkAdr(Word Mask, Word Mask08)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     WrError( (((1 << AdrMode) & Mask08)==0) ? 1350 : 1505);
     AdrMode=ModNone; AdrCnt=0;
    END
END

        static void DecodeAdr(Byte Start, Byte Stop, Word Mask)
BEGIN
   Boolean OK;
   Word AdrWord,Mask08;
   Byte ZeroMode;
   String s;
   ShortInt tmode1,tmode2;

   AdrMode=ModNone; AdrCnt=0;

   Mask08=Mask & MMod08;
   if (MomCPU==CPU6805) Mask&=MMod05;

   if (Stop-Start==1)
    BEGIN
     if (strcasecmp(ArgStr[Stop],"X")==0)
      BEGIN
       tmode1=ModIx1; tmode2=ModIx2;
      END
     else if (strcasecmp(ArgStr[Stop],"SP")==0)
      BEGIN
       tmode1=ModSP1; tmode2=ModSP2;
       if (MomCPU<CPU6808)
        BEGIN
         WrXError(1445,ArgStr[Stop]); ChkAdr(Mask,Mask08); return;
        END
      END
     else
      BEGIN
       WrXError(1445,ArgStr[Stop]); ChkAdr(Mask,Mask08); return;
      END

     ChkZero(ArgStr[Start],s,&ZeroMode);
     FirstPassUnknown=False;
     AdrWord=EvalIntExpression(s,(ZeroMode==2)?Int8:Int16,&OK);

     if (OK)
      BEGIN
       if ((ZeroMode==0) AND (AdrWord==0) AND (Mask AND MModIx!=0) AND (tmode1==ModIx1)) AdrMode=ModIx;

       else if (((Mask AND (1 << tmode2))==0) OR (ZeroMode==2) OR ((ZeroMode==0) AND (Hi(AdrWord)==0)))
        BEGIN
         if (FirstPassUnknown) AdrWord&=0xff;
         if (Hi(AdrWord)!=0) WrError(1340);
         else
          BEGIN
           AdrCnt=1; AdrVals[0]=Lo(AdrWord); AdrMode=tmode1;
          END
        END

       else
        BEGIN
         AdrVals[0]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
         AdrCnt=2; AdrMode=tmode2;
        END
      END
    END

   else if (Stop==Start)
    BEGIN
     /* Postinkrement */

     if (strcasecmp(ArgStr[Start],"X+")==0)
      BEGIN
       AdrMode=ModIxP; ChkAdr(Mask,Mask08); return;
      END

     /* X-indirekt */

     if (strcasecmp(ArgStr[Start],"X")==0)
      BEGIN
       WrError(280);
       AdrMode=ModIx; ChkAdr(Mask,Mask08);
       return;
      END

     /* immediate */

     if (*ArgStr[Start]=='#')
      BEGIN
       switch (OpSize)
        BEGIN
         case -1:
          WrError(1132); break;
         case 0:
          AdrVals[0]=EvalIntExpression(ArgStr[Start]+1,Int8,&OK);
          if (OK)
           BEGIN
            AdrCnt=1; AdrMode=ModImm;
           END
          break;
         case 1:
          AdrWord=EvalIntExpression(ArgStr[Start]+1,Int16,&OK);
          if (OK)
           BEGIN
            AdrVals[0]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
            AdrCnt=2; AdrMode=ModImm;
           END
          break;
        END
       ChkAdr(Mask,Mask08); return;
      END

     /* absolut */

     ChkZero(ArgStr[Start],s,&ZeroMode);
     FirstPassUnknown=False;
     AdrWord=EvalIntExpression(s,(ZeroMode==2)?UInt8:UInt16,&OK);

     if (OK)
      BEGIN
       if (((Mask & MModExt)==0) OR (ZeroMode==2) OR ((ZeroMode==0) AND (Hi(AdrWord)==0)))
        BEGIN
         if (FirstPassUnknown) AdrWord&=0xff;
         if (Hi(AdrWord)!=0) WrError(1340);
         else
          BEGIN
           AdrCnt=1; AdrVals[0]=Lo(AdrWord); AdrMode=ModDir;
          END
        END
       else
        BEGIN
         AdrVals[0]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
         AdrCnt=2; AdrMode=ModExt;
        END
       ChkAdr(Mask,Mask08); return;
      END
    END

   else WrError(1110);


   ChkAdr(Mask,Mask08);
END

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        static void MakeCode_6805(void)
BEGIN
   int z;
   Integer AdrInt;
   Boolean OK;
   char ch;

   CodeLen=0; DontPrint=False; OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(True)) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<FixedOrders[z].MinCPU) WrXError(1500,OpPart);
      else
       BEGIN
        CodeLen=1; BAsmCode[0]=FixedOrders[z].Code;
       END
      return;
     END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU6808) WrXError(1500,OpPart);
     else
      BEGIN
       OpSize=0; DecodeAdr(1,1,MModImm+MModDir+MModIxP);
       switch (AdrMode)
        BEGIN
         case ModImm:
          BAsmCode[1]=AdrVals[0]; DecodeAdr(2,2,MModDir);
          if (AdrMode==ModDir)
           BEGIN
            BAsmCode[0]=0x6e; BAsmCode[2]=AdrVals[0]; CodeLen=3;
           END
          break;
         case ModDir:
          BAsmCode[1]=AdrVals[0]; DecodeAdr(2,2,MModDir+MModIxP);
          switch (AdrMode)
           BEGIN
            case ModDir:
             BAsmCode[0]=0x4e; BAsmCode[2]=AdrVals[0]; CodeLen=3;
             break;
            case ModIxP:
             BAsmCode[0]=0x5e; CodeLen=2;
             break;
           END
          break;
         case ModIxP:
          DecodeAdr(2,2,MModDir);
          if (AdrMode==ModDir)
           BEGIN
            BAsmCode[0]=0x7e; BAsmCode[1]=AdrVals[0]; CodeLen=2;
           END
          break;
        END
      END
     return;
    END

   /* relative Spruenge */

   for (z=0; z<RelOrderCnt; z++)
    if Memo(RelOrders[z].Name)
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<RelOrders[z].MinCPU) WrXError(1500,OpPart);
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+2);
        if (OK)
         BEGIN
          if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
          else
           BEGIN
            CodeLen=2; BAsmCode[0]=RelOrders[z].Code; BAsmCode[1]=Lo(AdrInt);
           END
         END
       END
      return;
     END

   if ((Memo("CBEQA")) OR (Memo("CBEQX")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU6808) WrXError(1500,OpPart);
     else
      BEGIN
       OpSize=0; DecodeAdr(1,1,MModImm);
       if (AdrMode==ModImm)
        BEGIN
         BAsmCode[1]=AdrVals[0];
         AdrInt=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+3);
         if (OK)
          BEGIN
           if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
           else
            BEGIN
             BAsmCode[0]=0x41+(Ord(Memo("CBEQX")) << 4);
             BAsmCode[2]=AdrInt & 0xff;
             CodeLen=3;
            END
          END
        END
      END
     return;
    END

   if (Memo("CBEQ"))
    BEGIN
     if (MomCPU<CPU6808) WrXError(1500,OpPart);
     else if (ArgCnt==2)
      BEGIN
       DecodeAdr(1,1,MModDir+MModIxP);
       switch (AdrMode)
        BEGIN
         case ModDir:
          BAsmCode[0]=0x31; BAsmCode[1]=AdrVals[0]; z=3;
          break;
         case ModIxP:
          BAsmCode[0]=0x71; z=2;
          break;
        END;
       if (AdrMode!=ModNone)
        BEGIN
         AdrInt=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+z);
         if (OK)
          BEGIN
           if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
           else
            BEGIN
             BAsmCode[z-1]=AdrInt & 0xff; CodeLen=z;
            END
          END
        END
      END
     else if (ArgCnt==3)
      BEGIN
       OK=True;
       if (strcasecmp(ArgStr[2],"X+")==0) z=3;
       else if (strcasecmp(ArgStr[2],"SP")==0)
        BEGIN
         BAsmCode[0]=0x9e; z=4;
        END
       else
        BEGIN
         WrXError(1445,ArgStr[2]); OK=False;
        END
       if (OK)
        BEGIN
         BAsmCode[z-3]=0x61;
         BAsmCode[z-2]=EvalIntExpression(ArgStr[1],UInt8,&OK);
         if (OK)
          BEGIN
           AdrInt=EvalIntExpression(ArgStr[3],UInt16,&OK)-(EProgCounter()+z);
           if (OK)
            BEGIN
             if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
             else
              BEGIN
               BAsmCode[z-1]=AdrInt & 0xff; CodeLen=z;
              END
            END
          END
        END
      END
     else WrError(1110);
     return;
    END

   if ((Memo("DBNZA")) OR (Memo("DBNZX")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU6808) WrXError(1500,OpPart);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+2);
       if (OK)
        BEGIN
         if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
         else
          BEGIN
           BAsmCode[0]=0x4b+(Ord(Memo("DBNZX")) << 4);
           BAsmCode[1]=AdrInt & 0xff;
           CodeLen=2;
          END
        END
      END
     return;
    END

   if (Memo("DBNZ"))
    BEGIN
     if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
     else if (MomCPU<CPU6808) WrXError(1500,OpPart);
     else
      BEGIN
       DecodeAdr(1,ArgCnt-1,MModDir+MModIx+MModIx1+MModSP1);
       switch (AdrMode)
        BEGIN
         case ModDir:
          BAsmCode[0]=0x3b; BAsmCode[1]=AdrVals[0]; z=3;
          break;
         case ModIx:
          BAsmCode[0]=0x7b; z=2;
          break;
         case ModIx1:
          BAsmCode[0]=0x6b; BAsmCode[1]=AdrVals[0]; z=3;
          break;
         case ModSP1:
          BAsmCode[0]=0x9e; BAsmCode[1]=0x6b; BAsmCode[2]=AdrVals[0]; z=4;
          break;
        END
       if (AdrMode!=ModNone)
        BEGIN
         AdrInt=EvalIntExpression(ArgStr[ArgCnt],UInt16,&OK)-(EProgCounter()+z);
         if (OK)
          BEGIN
           if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
           else
            BEGIN
             BAsmCode[z-1]=AdrInt & 0xff; CodeLen=z;
            END
          END
        END
      END
     return;
    END

   /* ALU-Operationen */

   for (z=0; z<ALUOrderCnt; z++)
    if Memo(ALUOrders[z].Name)
     BEGIN
      if (MomCPU<ALUOrders[z].MinCPU) WrXError(1500,OpPart);
      else
       BEGIN
        OpSize=ALUOrders[z].Size;
        DecodeAdr(1,ArgCnt,ALUOrders[z].Mask);
        if (AdrMode!=ModNone)
         BEGIN
          switch (AdrMode)
           BEGIN
            case ModImm : 
             BAsmCode[0]=0xa0+ALUOrders[z].Code; CodeLen=1;
             break;
            case ModDir :
             BAsmCode[0]=0xb0+ALUOrders[z].Code; CodeLen=1;
             break;
            case ModExt :
             BAsmCode[0]=0xc0+ALUOrders[z].Code; CodeLen=1;
             break;
            case ModIx  :
             BAsmCode[0]=0xf0+ALUOrders[z].Code; CodeLen=1;
             break;
            case ModIx1 :
             BAsmCode[0]=0xe0+ALUOrders[z].Code; CodeLen=1;
             break;
            case ModIx2 :
             BAsmCode[0]=0xd0+ALUOrders[z].Code; CodeLen=1;
             break;
            case ModSP1 :
             BAsmCode[0]=0x9e; BAsmCode[1]=0xe0+ALUOrders[z].Code; CodeLen=2;
             break;
            case ModSP2 :
             BAsmCode[0]=0x9e; BAsmCode[1]=0xd0+ALUOrders[z].Code; CodeLen=2;
             break;
           END
          memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt); CodeLen+=AdrCnt;
         END
       END
      return;
     END

   if ((Memo("AIX")) OR (Memo("AIS")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU6808) WrXError(1500,OpPart);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[1]+1,SInt8,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=0xa7+(Ord(Memo("AIX")) << 3); CodeLen=2;
        END
      END
     return;
    END

   /* Read/Modify/Write-Operationen */

   for (z=0; z<RMWOrderCnt; z++)
    if Memo(RMWOrders[z].Name)
     BEGIN
      if (MomCPU<RMWOrders[z].MinCPU) WrXError(1500,OpPart);
      else
       BEGIN
        DecodeAdr(1,ArgCnt,RMWOrders[z].Mask);
        if (AdrMode!=ModNone)
         BEGIN
          switch (AdrMode)
           BEGIN
            case ModDir :
             BAsmCode[0]=0x30+RMWOrders[z].Code; CodeLen=1;
             break;
            case ModIx  :
             BAsmCode[0]=0x70+RMWOrders[z].Code; CodeLen=1;
             break;
            case ModIx1 :
             BAsmCode[0]=0x60+RMWOrders[z].Code; CodeLen=1;
             break;
            case ModSP1 :
             BAsmCode[0]=0x9e; BAsmCode[1]=0x60+RMWOrders[z].Code; CodeLen=2;
             break;
           END
          memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt); CodeLen+=AdrCnt;
         END
       END
      return;
     END

   ch=OpPart[strlen(OpPart)-1];
   if ((ch>='0') AND (ch<='7'))
    BEGIN
     for (z=ArgCnt; z>=1; z--) strcpy(ArgStr[z+1],ArgStr[z]);
     *ArgStr[1]=ch; ArgStr[1][1]='\0'; ArgCnt++;
     OpPart[strlen(OpPart)-1]='\0';
    END

   if ((Memo("BSET")) OR (Memo("BCLR")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=EvalIntExpression(ArgStr[1],UInt3,&OK);
         if (OK)
          BEGIN
           CodeLen=2; BAsmCode[0]=0x10+(BAsmCode[0] << 1)+Ord(Memo("BCLR"));
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
       BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=EvalIntExpression(ArgStr[1],UInt3,&OK);
         if (OK)
          BEGIN
           AdrInt=EvalIntExpression(ArgStr[3],UInt16,&OK)-(EProgCounter()+3);
           if (OK)
            BEGIN
             if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
             else
              BEGIN
               CodeLen=3; BAsmCode[0]=(BAsmCode[0] << 1)+Ord(Memo("BRCLR"));
               BAsmCode[2]=Lo(AdrInt);
              END
            END
          END
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_6805(void)
BEGIN
   return False;
END

        static void SwitchFrom_6805(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_6805(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x62; NOPCode=0x9d;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = (MomCPU == CPU6808) ? 0xffff : 0x1fff;

   MakeCode=MakeCode_6805; IsDef=IsDef_6805;
   SwitchFrom=SwitchFrom_6805; InitFields();
END

        void code6805_init(void)
BEGIN
   CPU6805=AddCPU("6805",SwitchTo_6805);
   CPU6808=AddCPU("68HC08",SwitchTo_6805);
END
