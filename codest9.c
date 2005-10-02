/* codest9.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator SGS-Thomson ST9                                             */
/*                                                                           */
/* Historie: 10. 2.1997 Grundsteinlegung                                     */
/*            1. 7.1998 Warnungen bei is...()-Funktionen beseitigt           */
/*            2. 1.1999 ChkPC umgestellt                                     */
/*           30. 1.1999 Formate maschinenunabhaengig gemacht                 */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codest9.c,v 1.4 2005/10/02 10:00:46 alfred Exp $                     */
/*****************************************************************************
 * $Log: codest9.c,v $
 * Revision 1.4  2005/10/02 10:00:46  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2005/09/08 17:31:05  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:03  alfred
 * - relocated DecodeIntelPseudo() into own module
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
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"


typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          int len;
          Word Code;
         } ALUOrder;


#define WorkOfs 0xd0

#define ModNone (-1)
#define ModReg 0          
#define MModReg (1l << ModReg)                  /* Rn */
#define ModWReg 1         
#define MModWReg (1l << ModWReg)                /* rn */
#define ModRReg 2         
#define MModRReg (1l << ModRReg)                /* RRn */
#define ModWRReg 3        
#define MModWRReg (1l << ModWRReg)              /* rrn */
#define ModIReg 4         
#define MModIReg (1l << ModIReg)                /* (Rn) */
#define ModIWReg 5        
#define MModIWReg (1l << ModIWReg)              /* (rn) */
#define ModIRReg 6        
#define MModIRReg (1l << ModIRReg)              /* (RRn) */
#define ModIWRReg 7       
#define MModIWRReg (1l << ModIWRReg)            /* (rrn) */
#define ModIncWReg 8      
#define MModIncWReg (1l << ModIncWReg)          /* (rn)+ */
#define ModIncWRReg 9     
#define MModIncWRReg (1l << ModIncWRReg)        /* (rrn)+ */
#define ModDecWRReg 10    
#define MModDecWRReg (1l << ModDecWRReg)        /* -(rrn) */
#define ModDisp8WReg 11   
#define MModDisp8WReg (1l << ModDisp8WReg)      /* d8(rn) */
#define ModDisp8WRReg 12  
#define MModDisp8WRReg (1l << ModDisp8WRReg)    /* d8(rrn) */
#define ModDisp16WRReg 13 
#define MModDisp16WRReg (1l << ModDisp16WRReg)  /* d16(rrn) */
#define ModDispRWRReg 14  
#define MModDispRWRReg (1l << ModDispRWRReg)    /* rrm(rrn */
#define ModAbs 15         
#define MModAbs (1l << ModAbs)                  /* NN */
#define ModImm 16         
#define MModImm (1l << ModImm)                  /* #N/#NN */
#define ModDisp8RReg 17   
#define MModDisp8RReg (1l << ModDisp8RReg)      /* d8(RRn) */
#define ModDisp16RReg 18  
#define MModDisp16RReg (1l << ModDisp16RReg)    /* d16(RRn) */

#define FixedOrderCnt 12
#define ALUOrderCnt 10
#define RegOrderCnt 13
#define Reg16OrderCnt 8
#define Bit2OrderCnt 4
#define Bit1OrderCnt 4
#define ConditionCnt 20
#define LoadOrderCnt 4


static CPUVar CPUST9020,CPUST9030,CPUST9040,CPUST9050;

static FixedOrder *FixedOrders;
static ALUOrder *ALUOrders;
static FixedOrder *RegOrders;
static FixedOrder *Reg16Orders;
static FixedOrder *Bit2Orders;
static FixedOrder *Bit1Orders;
static FixedOrder *Conditions;
static FixedOrder *LoadOrders;

static ShortInt AdrMode,AbsSeg;
static Byte AdrPart,OpSize;
static Byte AdrVals[3];

static SimpProc SaveInitProc;
static LongInt DPAssume;

/*--------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddALU(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name=NName;
   ALUOrders[InstrZ].len=strlen(NName);
   ALUOrders[InstrZ++].Code=NCode;
END

        static void AddReg(char *NName, Word NCode)
BEGIN
   if (InstrZ>=RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name=NName;
   RegOrders[InstrZ++].Code=NCode;
END

        static void AddReg16(char *NName, Word NCode)
BEGIN
   if (InstrZ>=Reg16OrderCnt) exit(255);
   Reg16Orders[InstrZ].Name=NName;
   Reg16Orders[InstrZ++].Code=NCode;
END

        static void AddBit2(char *NName, Word NCode)
BEGIN
   if (InstrZ>=Bit2OrderCnt) exit(255);
   Bit2Orders[InstrZ].Name=NName;
   Bit2Orders[InstrZ++].Code=NCode;
END

        static void AddBit1(char *NName, Word NCode)
BEGIN
   if (InstrZ>=Bit1OrderCnt) exit(255);
   Bit1Orders[InstrZ].Name=NName;
   Bit1Orders[InstrZ++].Code=NCode;
END

        static void AddCondition(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ConditionCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END

        static void AddLoad(char *NName, Word NCode)
BEGIN
   if (InstrZ>=LoadOrderCnt) exit(255);
   LoadOrders[InstrZ].Name=NName;
   LoadOrders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CCF" ,0x0061); AddFixed("DI"  ,0x0010);
   AddFixed("EI"  ,0x0000); AddFixed("HALT",0xbf01);
   AddFixed("IRET",0x00d3); AddFixed("NOP" ,0x00ff);
   AddFixed("RCF" ,0x0011); AddFixed("RET" ,0x0046);
   AddFixed("SCF" ,0x0001); AddFixed("SDM" ,0x00fe);
   AddFixed("SPM" ,0x00ee); AddFixed("WFI" ,0xef01);

   ALUOrders=(ALUOrder *) malloc(sizeof(ALUOrder)*ALUOrderCnt); InstrZ=0;
   AddALU("ADC", 3); AddALU("ADD", 4); AddALU("AND", 1);
   AddALU("CP" , 9); AddALU("OR" , 0); AddALU("SBC", 2);
   AddALU("SUB", 5); AddALU("TCM", 8); AddALU("TM" ,10);
   AddALU("XOR", 6);

   RegOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RegOrderCnt); InstrZ=0;
   AddReg("CLR" ,0x90); AddReg("CPL" ,0x80); AddReg("DA"  ,0x70);
   AddReg("DEC" ,0x40); AddReg("INC" ,0x50); AddReg("POP" ,0x76);
   AddReg("POPU",0x20); AddReg("RLC" ,0xb0); AddReg("ROL" ,0xa0);
   AddReg("ROR" ,0xc0); AddReg("RRC" ,0xd0); AddReg("SRA" ,0xe0);
   AddReg("SWAP",0xf0);

   Reg16Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Reg16OrderCnt); InstrZ=0;
   AddReg16("DECW" ,0xcf); AddReg16("EXT"  ,0xc6);
   AddReg16("INCW" ,0xdf); AddReg16("POPUW",0xb7);
   AddReg16("POPW" ,0x75); AddReg16("RLCW" ,0x8f);
   AddReg16("RRCW" ,0x36); AddReg16("SRAW" ,0x2f);

   Bit2Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Bit2OrderCnt); InstrZ=0;
   AddBit2("BAND",0x1f); AddBit2("BLD" ,0xf2);
   AddBit2("BOR" ,0x0f); AddBit2("BXOR",0x6f);

   Bit1Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Bit1OrderCnt); InstrZ=0;
   AddBit1("BCPL",0x6f); AddBit1("BRES" ,0x1f);
   AddBit1("BSET",0x0f); AddBit1("BTSET",0xf2);

   Conditions=(FixedOrder *) malloc(sizeof(FixedOrder)*ConditionCnt); InstrZ=0;
   AddCondition("F"   ,0x0); AddCondition("T"   ,0x8);
   AddCondition("C"   ,0x7); AddCondition("NC"  ,0xf);
   AddCondition("Z"   ,0x6); AddCondition("NZ"  ,0xe);
   AddCondition("PL"  ,0xd); AddCondition("MI"  ,0x5);
   AddCondition("OV"  ,0x4); AddCondition("NOV" ,0xc);
   AddCondition("EQ"  ,0x6); AddCondition("NE"  ,0xe);
   AddCondition("GE"  ,0x9); AddCondition("LT"  ,0x1);
   AddCondition("GT"  ,0xa); AddCondition("LE"  ,0x2);
   AddCondition("UGE" ,0xf); AddCondition("UL"  ,0x7);
   AddCondition("UGT" ,0xb); AddCondition("ULE" ,0x3);

   LoadOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LoadOrderCnt); InstrZ=0;
   AddLoad("LDPP",0x00); AddLoad("LDDP",0x10);
   AddLoad("LDPD",0x01); AddLoad("LDDD",0x11);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(ALUOrders);
   free(RegOrders);
   free(Reg16Orders);
   free(Bit2Orders);
   free(Bit1Orders);
   free(Conditions);
   free(LoadOrders);
END

/*--------------------------------------------------------------------------*/

        static Boolean DecodeReg(char *Asc_O, Byte *Erg, Byte *Size)
BEGIN
   Boolean Res;
   char *Asc;

   Asc=Asc_O;

   if (strlen(Asc)<2) return False;
   if (*Asc!='r') return False; Asc++;
   if (*Asc=='r') 
    BEGIN
     if (strlen(Asc)<2) return False;
     *Size=1; Asc++;
    END
   else *Size=0;

   *Erg=ConstLongInt(Asc,&Res,10);
   if ((NOT Res) OR (*Erg>15)) return False;
   if ((*Size==1) AND (Odd(*Erg))) return False;

   return True;
END

        static void ChkAdr(LongWord Mask)
BEGIN
   if ((AdrMode!=ModNone) AND (((1l << AdrMode) & Mask)==0)) 
    BEGIN
     WrError(1350); AdrMode=ModNone; AdrCnt=0;
    END
END

        static void DecodeAdr(char *Asc_O, LongWord Mask)
BEGIN
   Word AdrWord;
   int level;
   Byte flg,Size;
   Boolean OK;
   String Reg,Asc;
   char *p;

   AdrMode=ModNone; AdrCnt=0;
   strmaxcpy(Asc,Asc_O,255); 

   /* immediate */

   if (*Asc=='#') 
    BEGIN
     switch (OpSize)
      BEGIN
       case 0:
        AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
        break;
       case 1:
        AdrWord=EvalIntExpression(Asc+1,Int16,&OK);
        AdrVals[0]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
        break;
      END
     if (OK) 
      BEGIN
       AdrMode=ModImm; AdrCnt=OpSize+1;
      END
     ChkAdr(Mask); return;
    END

   /* Arbeitsregister */

   if (DecodeReg(Asc,&AdrPart,&Size)) 
    BEGIN
     if (Size==0) 
      if ((Mask & MModWReg)!=0) AdrMode=ModWReg;
      else
       BEGIN
        AdrVals[0]=WorkOfs+AdrPart; AdrCnt=1; AdrMode=ModReg;
       END
     else
      if ((Mask & MModWRReg)!=0)  AdrMode=ModWRReg;
      else
       BEGIN
        AdrVals[0]=WorkOfs+AdrPart; AdrCnt=1; AdrMode=ModRReg;
       END
     ChkAdr(Mask); return;
    END

   /* Postinkrement */

   if ((*Asc) && (Asc[strlen(Asc)-1]=='+'))
    BEGIN
     if ((*Asc!='(') OR (Asc[strlen(Asc)-2]!=')')) WrError(1350);
     else
      BEGIN
       strcpy(Asc,Asc+1); Asc[strlen(Asc)-2]='\0';
       if (NOT DecodeReg(Asc,&AdrPart,&Size)) WrXError(1445,Asc);
       AdrMode=(Size==0) ? ModIncWReg : ModIncWRReg;
      END
     ChkAdr(Mask); return;
    END

   /* Predekrement */

   if ((*Asc=='-') AND (Asc[1]=='(') AND (Asc[strlen(Asc)-1]==')')) 
    BEGIN
     strcpy(Reg,Asc+2); Reg[strlen(Reg)-1]='\0';
     if (DecodeReg(Reg,&AdrPart,&Size)) 
      BEGIN
       if (Size==0) WrError(1350); else AdrMode=ModDecWRReg;
       ChkAdr(Mask); return;
      END
    END

   /* indirekt<->direkt */

   if ((strlen(Asc)<3) || (Asc[strlen(Asc)-1]!=')'))
    BEGIN
     OK=False; p=Asc;
    END
   else
    BEGIN
     level=0; p=Asc+strlen(Asc)-2; flg=0;
     while ((p>=Asc) AND (level>=0))
      BEGIN
       switch (*p)
        BEGIN
         case '(': if (flg==0) level--; break;
         case ')': if (flg==0) level++; break;
         case '\'': if (((flg & 2)==0))  flg^=1; break;
         case '"': if (((flg & 1)==0))  flg^=2; break;
        END
       p--;
      END
     OK=(level==-1) AND ((p<Asc) OR ((*p=='.') OR (*p=='_') OR (isdigit(((unsigned int)*p)&0xff)) OR (isalpha(((unsigned int)*p)&0xff))));
    END

   /* indirekt */

   if (OK) 
    BEGIN
     strcpy(Reg,p+2); Reg[strlen(Reg)-1]='\0'; p[1]='\0';
     if (DecodeReg(Reg,&AdrPart,&Size)) 
      if (Size==0)   /* d(r) */
       BEGIN
        AdrVals[0]=EvalIntExpression(Asc,Int8,&OK);
        if (OK) 
         BEGIN
          if (((Mask & MModIWReg)!=0) AND (AdrVals[0]==0)) AdrMode=ModIWReg;
          else if (((Mask & MModIReg)!=0) AND (AdrVals[0]==0)) 
           BEGIN
            AdrVals[0]=WorkOfs+AdrPart; AdrMode=ModIReg; AdrCnt=1;
           END
          else
           BEGIN
            AdrMode=ModDisp8WReg; AdrCnt=1;
           END
         END
       END
      else            /* ...(rr) */
       BEGIN
        if (DecodeReg(Asc,AdrVals,&Size)) 
         BEGIN        /* rr(rr) */
          if (Size!=1) WrError(1350);
          else
           BEGIN
            AdrMode=ModDispRWRReg; AdrCnt=1;
           END
         END
        else
         BEGIN        /* d(rr) */
          AdrWord=EvalIntExpression(Asc,Int16,&OK);
          if ((AdrWord==0) AND ((Mask & (MModIRReg+MModIWRReg))!=0)) 
           BEGIN
            if (((Mask & MModIWRReg)!=0)) AdrMode=ModIWRReg;
            else
             BEGIN
              AdrMode=ModIRReg; AdrVals[0]=AdrPart+WorkOfs; AdrCnt=1;
             END
           END
          else if ((AdrWord<0x100) AND ((Mask & (MModDisp8WRReg+MModDisp8RReg))!=0)) 
           BEGIN
            if (((Mask & MModDisp8WRReg)!=0)) 
             BEGIN
              AdrVals[0]=Lo(AdrWord); AdrCnt=1; AdrMode=ModDisp8WRReg;
             END
            else
             BEGIN
              AdrVals[0]=AdrPart+WorkOfs; AdrVals[1]=Lo(AdrWord);
              AdrCnt=2; AdrMode=ModDisp8RReg;
             END
           END
          else if (((Mask & MModDisp16WRReg)!=0)) 
           BEGIN
            AdrVals[0]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
            AdrCnt=2; AdrMode=ModDisp16WRReg;
           END
          else
           BEGIN
            AdrVals[0]=AdrPart+WorkOfs;
            AdrVals[2]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
            AdrCnt=3; AdrMode=ModDisp16RReg;
           END
         END
       END

     else             /* ...(RR) */
      BEGIN
       AdrWord=EvalIntExpression(Reg,UInt9,&OK);
       if (((TypeFlag & (1 << SegReg))==0))  WrError(1350);
       else if (AdrWord<0xff) 
        BEGIN
         AdrVals[0]=Lo(AdrWord);
         AdrWord=EvalIntExpression(Asc,Int8,&OK);
         if (AdrWord!=0) WrError(1320);
         else
          BEGIN
           AdrCnt=1; AdrMode=ModIReg;
          END
        END
       else if ((AdrWord>0x1ff) OR (Odd(AdrWord))) WrError(1350);
       else
        BEGIN
         AdrVals[0]=Lo(AdrWord);
         AdrWord=EvalIntExpression(Asc,Int16,&OK);
         if ((AdrWord==0) AND ((Mask & MModIRReg)!=0)) 
          BEGIN
           AdrCnt=1; AdrMode=ModIRReg;
          END
         else if ((AdrWord<0x100) AND ((Mask & MModDisp8RReg)!=0)) 
          BEGIN
           AdrVals[1]=Lo(AdrWord); AdrCnt=2; AdrMode=ModDisp8RReg;
          END
         else
          BEGIN
           AdrVals[2]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
           AdrCnt=3; AdrMode=ModDisp16RReg;
          END
        END
      END
     ChkAdr(Mask); return;
    END

   /* direkt */

   AdrWord=EvalIntExpression(Asc,UInt16,&OK);
   if (OK)
    BEGIN 
     if (((TypeFlag & (1 << SegReg)))==0) 
      BEGIN
       AdrMode=ModAbs;
       AdrVals[0]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
       AdrCnt=2; ChkSpace(AbsSeg);
      END
     else if (AdrWord<0xff) 
      BEGIN
       AdrMode=ModReg; AdrVals[0]=Lo(AdrWord); AdrCnt=1;
      END
     else if ((AdrWord>0x1ff) OR (Odd(AdrWord))) WrError(1350);
     else
      BEGIN
       AdrMode=ModRReg; AdrVals[0]=Lo(AdrWord); AdrCnt=1;
      END
    END

   ChkAdr(Mask);
END

        static Boolean SplitBit(char *Asc, Byte *Erg)
BEGIN
   char *p;
   Integer val;
   Boolean OK,Inv;

   p=RQuotPos(Asc,'.');
   if ((p==Nil) OR (p==Asc+strlen(Asc)+1)) 
    BEGIN
     if (*Asc=='!') 
      BEGIN
       Inv=True; strcpy(Asc,Asc+1);
      END
     else Inv=False;
     val=EvalIntExpression(Asc,UInt8,&OK);
     if (OK) 
      BEGIN
       *Erg=val & 15; if (Inv) *Erg^=1;
       sprintf(Asc, "r%d", (int)(val >> 4));
       return True;
      END
     return False;
    END

   if (p[1]=='!') 
    *Erg=1+(EvalIntExpression(p+2,UInt3,&OK) << 1);
   else
    *Erg=EvalIntExpression(p+1,UInt3,&OK) << 1;
   *p='\0';
   return OK;
END

/*--------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
#define ASSUMEST9Count 1
static ASSUMERec ASSUMEST9s[ASSUMEST9Count]=
               {{"DP", &DPAssume, 0,  1, 0x0}};
   Byte Bit;

   if (Memo("REG")) 
    BEGIN
     CodeEquate(SegReg,0,0x1ff);
     return True;
    END

   if (Memo("BIT")) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (SplitBit(ArgStr[1],&Bit)) 
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg);
       if (AdrMode==ModWReg) 
        BEGIN
         PushLocHandle(-1);
         EnterIntSymbol(LabPart,(AdrPart << 4)+Bit,SegNone,False);
         PopLocHandle();
         sprintf(ListLine,"=r%d.%s%c", (int)AdrPart,
                 (Odd(Bit))?"!":"", (Bit >> 1)+AscOfs);
        END
      END
     return True;
    END

   if (Memo("ASSUME")) 
    BEGIN
     CodeASSUME(ASSUMEST9s,ASSUMEST9Count);
     return True;
    END

   return False;
END

        static void MakeCode_ST9(void)
BEGIN
   Integer AdrInt;
   int z;
   Boolean OK;
   Byte HReg,HPart;
   Word Mask1,Mask2,AdrWord;

   CodeLen=0; DontPrint=False; OpSize=0;
   AbsSeg=(DPAssume==1) ? SegData : SegCode;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(True)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name)) 
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        OK=Hi(FixedOrders[z].Code)!=0;
        if (OK) BAsmCode[0]=Hi(FixedOrders[z].Code);
        CodeLen=Ord(OK);
        BAsmCode[CodeLen++]=Lo(FixedOrders[z].Code);
       END
      return;
     END

   /* Datentransfer */

   if ((Memo("LD")) OR (Memo("LDW"))) 
    BEGIN
     if (ArgCnt!=2)  WrError(1110);
     else
      BEGIN
       if (OpPart[strlen(OpPart)-1]=='W') 
        BEGIN
         OpSize=1; Mask1=MModWRReg; Mask2=MModRReg;
        END
       else
        BEGIN
         Mask1=MModWReg; Mask2=MModReg;
        END;
       DecodeAdr(ArgStr[1],Mask1+Mask2+MModIWReg+MModDisp8WReg+MModIncWReg+
                           MModIWRReg+MModIncWRReg+MModDecWRReg+MModDisp8WRReg+
                           MModDisp16WRReg+MModDispRWRReg+MModAbs+MModIRReg);
       switch (AdrMode)
        BEGIN
         case ModWReg:
         case ModWRReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],Mask1+Mask2+MModIWReg+MModDisp8WReg+MModIWRReg+
                              MModIncWRReg+MModDecWRReg+MModDispRWRReg+MModDisp8WRReg+
                              MModDisp16WRReg+MModAbs+MModImm);
          switch (AdrMode)
           BEGIN
            case ModWReg:
             BAsmCode[0]=(HReg << 4)+8; BAsmCode[1]=WorkOfs+AdrPart;
             CodeLen=2;
             break;
            case ModWRReg:
             BAsmCode[0]=0xe3; BAsmCode[1]=(HReg << 4)+AdrPart;
             CodeLen=2;
             break;
            case ModReg:
             BAsmCode[0]=(HReg << 4)+8; BAsmCode[1]=AdrVals[0];
             CodeLen=2;
             break;
            case ModRReg:
             BAsmCode[0]=0xef; BAsmCode[1]=AdrVals[0];
             BAsmCode[2]=HReg+WorkOfs; CodeLen=3;
             break;
            case ModIWReg:
             if (OpSize==0) 
              BEGIN
               BAsmCode[0]=0xe4; BAsmCode[1]=(HReg << 4)+AdrPart;
               CodeLen=2;
              END
             else
              BEGIN
               BAsmCode[0]=0xa6; BAsmCode[1]=0xf0+AdrPart;
               BAsmCode[2]=WorkOfs+HReg; CodeLen=3;
              END
             break;
            case ModDisp8WReg:
             BAsmCode[0]=0xb3+(OpSize*0x2b);
             BAsmCode[1]=(HReg << 4)+AdrPart;
             BAsmCode[2]=AdrVals[0]; CodeLen=3;
             break;
            case ModIWRReg:
             BAsmCode[0]=0xb5+(OpSize*0x2e);
             BAsmCode[1]=(HReg << 4)+AdrPart+OpSize;
             CodeLen=2;
             break;
            case ModIncWRReg:
             BAsmCode[0]=0xb4+(OpSize*0x21);
             BAsmCode[1]=0xf1+AdrPart;
             BAsmCode[2]=WorkOfs+HReg; CodeLen=3;
             break;
            case ModDecWRReg:
             BAsmCode[0]=0xc2+OpSize;
             BAsmCode[1]=0xf1+AdrPart;
             BAsmCode[2]=WorkOfs+HReg; CodeLen=3;
             break;
            case ModDispRWRReg:
             BAsmCode[0]=0x60;
             BAsmCode[1]=(0x10*(1-OpSize))+(AdrVals[0] << 4)+AdrPart;
             BAsmCode[2]=0xf0+HReg; CodeLen=3;
             break;
            case ModDisp8WRReg:
             BAsmCode[0]=0x7f+(OpSize*7);
             BAsmCode[1]=0xf1+AdrPart;
             BAsmCode[2]=AdrVals[0]; BAsmCode[3]=HReg+WorkOfs;
             CodeLen=4;
             break;
            case ModDisp16WRReg:
             BAsmCode[0]=0x7f+(OpSize*7);
             BAsmCode[1]=0xf0+AdrPart;
             memcpy(BAsmCode+2,AdrVals,AdrCnt); BAsmCode[2+AdrCnt]=HReg+WorkOfs;
             CodeLen=3+AdrCnt;
             break;
            case ModAbs:
             BAsmCode[0]=0xc4+(OpSize*0x1e);
             BAsmCode[1]=0xf0+AdrPart;
             memcpy(BAsmCode+2,AdrVals,AdrCnt);
             CodeLen=2+AdrCnt;
             break;
            case ModImm:
             if (OpSize==0) 
              BEGIN
               BAsmCode[0]=(HReg << 4)+0x0c;
               memcpy(BAsmCode+1,AdrVals,AdrCnt);
               CodeLen=1+AdrCnt;
              END
             else
              BEGIN
               BAsmCode[0]=0xbf; BAsmCode[1]=WorkOfs+HReg;
               memcpy(BAsmCode+2,AdrVals,AdrCnt);
               CodeLen=2+AdrCnt;
              END
             break;
           END
          break;
         case ModReg:
         case ModRReg:
          HReg=AdrVals[0];
          DecodeAdr(ArgStr[2],Mask1+Mask2+MModIWReg+MModIWRReg+MModIncWRReg+
                              MModDecWRReg+MModDispRWRReg+MModDisp8WRReg+MModDisp16WRReg+
                              MModImm);
          switch (AdrMode)
           BEGIN
            case ModWReg:
             BAsmCode[0]=(AdrPart << 4)+0x09; BAsmCode[1]=HReg; CodeLen=2;
             break;
            case ModWRReg:
             BAsmCode[0]=0xef; BAsmCode[1]=WorkOfs+AdrPart;
             BAsmCode[2]=HReg; CodeLen=3;
             break;
            case ModReg:
            case ModRReg:
             BAsmCode[0]=0xf4-(OpSize*5);
             BAsmCode[1]=AdrVals[0];
             BAsmCode[2]=HReg; CodeLen=3;
             break;
            case ModIWReg:
             BAsmCode[0]=0xe7-(0x41*OpSize); BAsmCode[1]=0xf0+AdrPart;
             BAsmCode[2]=HReg; CodeLen=3;
             break;
            case ModIWRReg:
             BAsmCode[0]=0x72+(OpSize*12);
             BAsmCode[1]=0xf1+AdrPart-OpSize; BAsmCode[2]=HReg;
             CodeLen=3;
             break;
            case ModIncWRReg:
             BAsmCode[0]=0xb4+(0x21*OpSize);
             BAsmCode[1]=0xf1+AdrPart; BAsmCode[2]=HReg;
             CodeLen=3;
             break;
            case ModDecWRReg:
             BAsmCode[0]=0xc2+OpSize;
             BAsmCode[1]=0xf1+AdrPart; BAsmCode[2]=HReg;
             CodeLen=3;
             break;
            case ModDisp8WRReg:
             BAsmCode[0]=0x7f+(OpSize*7);
             BAsmCode[1]=0xf1+AdrPart;
             BAsmCode[2]=AdrVals[0];
             BAsmCode[3]=HReg; CodeLen=4;
             break;
            case ModDisp16WRReg:
             BAsmCode[0]=0x7f+(OpSize*7);
             BAsmCode[1]=0xf0+AdrPart;
             memcpy(BAsmCode+2,AdrVals,AdrCnt);
             BAsmCode[2+AdrCnt]=HReg; CodeLen=3+AdrCnt;
             break;
            case ModImm:
             BAsmCode[0]=0xf5-(OpSize*0x36);
             BAsmCode[1]=HReg;
             memcpy(BAsmCode+2,AdrVals,2); CodeLen=2+AdrCnt;
             break;
           END
          break;
         case ModIWReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],Mask2+Mask1);
          switch (AdrMode)
           BEGIN
            case ModWReg:
             BAsmCode[0]=0xe5; BAsmCode[1]=(HReg << 4)+AdrPart; CodeLen=2;
             break;
            case ModWRReg:
             BAsmCode[0]=0x96; BAsmCode[1]=WorkOfs+AdrPart;
             BAsmCode[2]=0xf0+HReg; CodeLen=3;
             break;
            case ModReg:
            case ModRReg:
             BAsmCode[0]=0xe6-(0x50*OpSize); BAsmCode[1]=AdrVals[0];
             BAsmCode[2]=0xf0+HReg; CodeLen=3;
             break;
           END
          break;
         case ModDisp8WReg:
          BAsmCode[2]=AdrVals[0]; HReg=AdrPart;
          DecodeAdr(ArgStr[2],Mask1);
          switch (AdrMode)
           BEGIN
            case ModWReg:
            case ModWRReg:
             BAsmCode[0]=0xb2+(OpSize*0x2c);
             BAsmCode[1]=(AdrPart << 4)+(OpSize << 4)+HReg;
             CodeLen=3;
             break;
           END
          break;
         case ModIncWReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModIncWRReg*(1-OpSize));
          switch (AdrMode)
           BEGIN
            case ModIncWRReg:
             BAsmCode[0]=0xd7; BAsmCode[1]=(HReg << 4)+AdrPart+1; CodeLen=2;
             break;
           END
          break;
         case ModIWRReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],(MModIWReg*(1-OpSize))+Mask1+Mask2+MModIWRReg+MModImm);
          switch (AdrMode)
           BEGIN
            case ModIWReg:
             BAsmCode[0]=0xb5; BAsmCode[1]=(AdrPart << 4)+HReg+1; CodeLen=2;
             break;
            case ModWReg:
             BAsmCode[0]=0x72; BAsmCode[1]=0xf0+HReg;
             BAsmCode[2]=AdrPart+WorkOfs;
             CodeLen=3;
             break;
            case ModWRReg:
             BAsmCode[0]=0xe3; BAsmCode[1]=(HReg << 4)+0x10+AdrPart;
             CodeLen=2;
             break;
            case ModReg:
            case ModRReg:
             BAsmCode[0]=0x72+(OpSize*0x4c);
             BAsmCode[1]=0xf0+HReg+OpSize; BAsmCode[2]=AdrVals[0];
             CodeLen=3;
             break;
            case ModIWRReg:
             if (OpSize==0) 
              BEGIN
               BAsmCode[0]=0x73;
               BAsmCode[1]=0xf0+AdrPart;
               BAsmCode[2]=WorkOfs+HReg; CodeLen=3;
              END
             else
              BEGIN
               BAsmCode[0]=0xe3; BAsmCode[1]=0x11+(HReg << 4)+AdrPart;
               CodeLen=2;
              END
             break;
            case ModImm:
             BAsmCode[0]=0xf3-(OpSize*0x35);
             BAsmCode[1]=0xf0+HReg;
             memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
             break;
           END
          break;
         case ModIncWRReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],Mask2+(MModIncWReg*(1-OpSize)));
          switch (AdrMode)
           BEGIN
            case ModReg:
            case ModRReg:
             BAsmCode[0]=0xb4+(OpSize*0x21);
             BAsmCode[1]=0xf0+HReg; BAsmCode[2]=AdrVals[0];
             CodeLen=3;
             break;
            case ModIncWReg:
             BAsmCode[0]=0xd7; BAsmCode[1]=(AdrPart << 4)+HReg;
             CodeLen=2;
             break;
           END;
          break;
         case ModDecWRReg:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],Mask2);
          switch (AdrMode)
           BEGIN
            case ModReg:
            case ModRReg:
             BAsmCode[0]=0xc2+OpSize;
             BAsmCode[1]=0xf0+HReg; BAsmCode[2]=AdrVals[0];
             CodeLen=3;
             break;
           END
          break;
         case ModDispRWRReg:
          HReg=AdrPart; HPart=AdrVals[0];
          DecodeAdr(ArgStr[2],Mask1);
          switch (AdrMode)
           BEGIN
            case ModWReg:
            case ModWRReg:
             BAsmCode[0]=0x60;
             BAsmCode[1]=(0x10*(1-OpSize))+0x01+(HPart << 4)+HReg;
             BAsmCode[2]=0xf0+AdrPart;
             CodeLen=3;
             break;
           END
          break;
         case ModDisp8WRReg:
          BAsmCode[2]=AdrVals[0]; HReg=AdrPart;
          DecodeAdr(ArgStr[2],Mask2+(OpSize*MModImm));
          switch (AdrMode)
           BEGIN
            case ModReg:
            case ModRReg:
             BAsmCode[0]=0x26+(OpSize*0x60);
             BAsmCode[1]=0xf1+HReg; BAsmCode[3]=AdrVals[0];
             CodeLen=4;
             break;
            case ModImm:
             BAsmCode[0]=0x06; BAsmCode[1]=0xf1+HReg;
             memcpy(BAsmCode+3,AdrVals,AdrCnt); CodeLen=3+AdrCnt;
             break;
           END
          break;
         case ModDisp16WRReg:
          memcpy(BAsmCode+2,AdrVals,2); HReg=AdrPart;
          DecodeAdr(ArgStr[2],Mask2+(OpSize*MModImm));
          switch (AdrMode)
           BEGIN
            case ModReg:
            case ModRReg:
             BAsmCode[0]=0x26+(OpSize*0x60);
             BAsmCode[1]=0xf0+HReg; BAsmCode[4]=AdrVals[0];
             CodeLen=5;
             break;
            case ModImm:
             BAsmCode[0]=0x06; BAsmCode[1]=0xf0+HReg;
             memcpy(BAsmCode+4,AdrVals,AdrCnt); CodeLen=4+AdrCnt;
             break;
           END
          break;
         case ModAbs:
          memcpy(BAsmCode+2,AdrVals,2);
          DecodeAdr(ArgStr[2],Mask1+MModImm);
          switch (AdrMode)
           BEGIN
            case ModWReg:
            case ModWRReg:
             BAsmCode[0]=0xc5+(OpSize*0x1d);
             BAsmCode[1]=0xf0+AdrPart+OpSize;
             CodeLen=4;
             break;
            case ModImm:
             memmove(BAsmCode+2+AdrCnt,BAsmCode+2,2); BAsmCode[0]=0x2f+(OpSize*7);
             BAsmCode[1]=0xf1; memcpy(BAsmCode+2,AdrVals,AdrCnt);
             CodeLen=4+AdrCnt;
             break;
           END
          break;
         case ModIRReg:
          HReg=AdrVals[0];
          DecodeAdr(ArgStr[2],MModIWRReg*(1-OpSize));
          switch (AdrMode)
           BEGIN
            case ModIWRReg:
             BAsmCode[0]=0x73; BAsmCode[1]=0xf0+AdrPart; BAsmCode[2]=HReg;
             CodeLen=3;
             break;
           END
          break;
        END
      END
     return;
    END

   for (z=0; z<LoadOrderCnt; z++)
    if (Memo(LoadOrders[z].Name)) 
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModIncWRReg);
        if (AdrMode==ModIncWRReg)
         BEGIN
          HReg=AdrPart << 4;
          DecodeAdr(ArgStr[2],MModIncWRReg);
          if (AdrMode==ModIncWRReg)
           BEGIN
            BAsmCode[0]=0xd6;
            BAsmCode[1]=LoadOrders[z].Code+HReg+AdrPart;
            CodeLen=2;
           END
         END
       END
      return;
     END

   if ((Memo("PEA")) OR (Memo("PEAU"))) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModDisp8RReg+MModDisp16RReg);
       if (AdrMode!=ModNone)
        BEGIN
         BAsmCode[0]=0x8f;
         BAsmCode[1]=0x01+(2*Ord(Memo("PEAU")));
         memcpy(BAsmCode+2,AdrVals,AdrCnt);
         BAsmCode[2]+=AdrCnt-2;
         CodeLen=2+AdrCnt;
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("PUSHU"))) 
    BEGIN
     z=Ord(Memo("PUSHU"));
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModIReg+MModImm);
       switch (AdrMode)
        BEGIN
         case ModReg:
          BAsmCode[0]=0x66-(z*0x36); BAsmCode[1]=AdrVals[0];
          CodeLen=2;
          break;
         case ModIReg:
          BAsmCode[0]=0xf7-(z*0xc6); BAsmCode[1]=AdrVals[0];
          CodeLen=2;
          break;
         case ModImm:
          BAsmCode[0]=0x8f; BAsmCode[1]=0xf1+(z*2);
          BAsmCode[2]=AdrVals[0]; CodeLen=3;
          break;
        END
      END
     return;
    END

   if ((Memo("PUSHW")) OR (Memo("PUSHUW"))) 
    BEGIN
     z=Ord(Memo("PUSHUW")); OpSize=1;
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModRReg+MModImm);
       switch (AdrMode)
        BEGIN
         case ModRReg:
          BAsmCode[0]=0x74+(z*0x42); BAsmCode[1]=AdrVals[0];
          CodeLen=2;
          break;
         case ModImm:
          BAsmCode[0]=0x8f; BAsmCode[1]=0xc1+(z*2);
          memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
          break;
        END
      END
     return;
    END

   if (Memo("XCH")) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode==ModReg)
        BEGIN
         BAsmCode[2]=AdrVals[0];
         DecodeAdr(ArgStr[2],MModReg);
         if (AdrMode==ModReg)
          BEGIN
           BAsmCode[1]=AdrVals[0];
           BAsmCode[0]=0x16;
           CodeLen=3;
          END
        END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<ALUOrderCnt; z++)
    if ((strncmp(OpPart,ALUOrders[z].Name,ALUOrders[z].len)==0) AND ((OpPart[ALUOrders[z].len]=='\0') OR (OpPart[ALUOrders[z].len]=='W')))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        if (OpPart[strlen(OpPart)-1]=='W') 
         BEGIN
          OpSize=1; Mask1=MModWRReg; Mask2=MModRReg;
         END
        else
         BEGIN
          Mask1=MModWReg; Mask2=MModReg;
         END
        DecodeAdr(ArgStr[1],Mask1+Mask2+MModIWReg+MModIWRReg+MModIncWRReg+
                            MModDecWRReg+MModDispRWRReg+MModDisp8WRReg+MModDisp16WRReg+
                            MModAbs+MModIRReg);
        switch (AdrMode)
         BEGIN
          case ModWReg:
          case ModWRReg:
           HReg=AdrPart;
           DecodeAdr(ArgStr[2],Mask1+MModIWReg+Mask2+MModIWRReg+MModIncWRReg+
                               MModDecWRReg+MModDispRWRReg+MModDisp8WRReg+MModDisp16WRReg+
                               MModAbs+MModImm);
           switch (AdrMode)
            BEGIN
             case ModWReg:
             case ModWRReg:
              BAsmCode[0]=(ALUOrders[z].Code << 4)+2+(OpSize*12);
              BAsmCode[1]=(HReg << 4)+AdrPart;
              CodeLen=2;
              break;
             case ModIWReg:
              if (OpSize==0) 
               BEGIN
                BAsmCode[0]=(ALUOrders[z].Code << 4)+3; BAsmCode[1]=(HReg << 4)+AdrPart;
                CodeLen=2;
               END
              else
               BEGIN
                BAsmCode[0]=0xa6; BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart;
                BAsmCode[2]=WorkOfs+HReg; CodeLen=3;
               END
              break;
             case ModReg:
             case ModRReg:
              BAsmCode[0]=(ALUOrders[z].Code << 4)+4+(OpSize*3);
              BAsmCode[1]=AdrVals[0]; BAsmCode[2]=HReg+WorkOfs;
              CodeLen=3;
              break;
             case ModIWRReg:
              if (OpSize==0) 
               BEGIN
                BAsmCode[0]=0x72; BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+1;
                BAsmCode[2]=HReg+WorkOfs;
                CodeLen=3;
               END
              else
               BEGIN
                BAsmCode[0]=(ALUOrders[z].Code << 4)+0x0e;
                BAsmCode[1]=(HReg << 4)+AdrPart+1;
                CodeLen=2;
               END
              break;
             case ModIncWRReg:
              BAsmCode[0]=0xb4+(OpSize*0x21);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+1;
              BAsmCode[2]=HReg+WorkOfs;
              CodeLen=3;
              break;
             case ModDecWRReg:
              BAsmCode[0]=0xc2+OpSize;
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+1;
              BAsmCode[2]=HReg+WorkOfs;
              CodeLen=3;
              break;
             case ModDispRWRReg:
              BAsmCode[0]=0x60;
              BAsmCode[1]=0x10*(1-OpSize)+(AdrVals[0] << 4)+AdrPart;
              BAsmCode[2]=(ALUOrders[z].Code << 4)+HReg;
              CodeLen=3;
              break;
             case ModDisp8WRReg:
              BAsmCode[0]=0x7f+(OpSize*7);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+1;
              BAsmCode[2]=AdrVals[0]; BAsmCode[3]=WorkOfs+HReg;
              CodeLen=4;
              break;
             case ModDisp16WRReg:
              BAsmCode[0]=0x7f+(OpSize*7);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart;
              memcpy(BAsmCode+2,AdrVals,AdrCnt);
              BAsmCode[2+AdrCnt]=WorkOfs+HReg;
              CodeLen=3+AdrCnt;
              break;
             case ModAbs:
              BAsmCode[0]=0xc4+(OpSize*30);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg;
              memcpy(BAsmCode+2,AdrVals,AdrCnt);
              CodeLen=2+AdrCnt;
              break;
             case ModImm:
              BAsmCode[0]=(ALUOrders[z].Code << 4)+5+(OpSize*2);
              BAsmCode[1]=WorkOfs+HReg+OpSize;
              memcpy(BAsmCode+2,AdrVals,AdrCnt);
              CodeLen=2+AdrCnt;
              break;
            END
           break;
          case ModReg:
          case ModRReg:
           HReg=AdrVals[0];
           DecodeAdr(ArgStr[2],Mask2+MModIWReg+MModIWRReg+MModIncWRReg+MModDecWRReg+
                               MModDisp8WRReg+MModDisp16WRReg+MModImm);
           switch (AdrMode)
            BEGIN
             case ModReg:
             case ModRReg:
              BAsmCode[0]=(ALUOrders[z].Code << 4)+4+(OpSize*3); CodeLen=3;
              BAsmCode[1]=AdrVals[0]; BAsmCode[2]=HReg;
              break;
             case ModIWReg:
              BAsmCode[0]=0xa6+65*(1-OpSize);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart;
              BAsmCode[2]=HReg; CodeLen=3;
              break;
             case ModIWRReg:
              BAsmCode[0]=0x72+(OpSize*12);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+(1-OpSize);
              BAsmCode[2]=HReg; CodeLen=3;
              break;
             case ModIncWRReg:
              BAsmCode[0]=0xb4+(OpSize*0x21);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+1;
              BAsmCode[2]=HReg; CodeLen=3;
              break;
             case ModDecWRReg:
              BAsmCode[0]=0xc2+OpSize;
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+1;
              BAsmCode[2]=HReg; CodeLen=3;
              break;
             case ModDisp8WRReg:
              BAsmCode[0]=0x7f+(OpSize*7);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+1;
              BAsmCode[2]=AdrVals[0]; BAsmCode[3]=HReg; CodeLen=4;
              break;
             case ModDisp16WRReg:
              BAsmCode[0]=0x7f+(OpSize*7);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart;
              memcpy(BAsmCode+2,AdrVals,AdrCnt); BAsmCode[2+AdrCnt]=HReg;
              CodeLen=3+AdrCnt;
              break;
             case ModImm:
              BAsmCode[0]=(ALUOrders[z].Code << 4)+5+(OpSize*2);
              memcpy(BAsmCode+2,AdrVals,AdrCnt); BAsmCode[1]=HReg+OpSize;
              CodeLen=2+AdrCnt;
              break;
            END
           break;
          case ModIWReg:
           HReg=AdrPart;
           DecodeAdr(ArgStr[2],Mask2);
           switch (AdrMode)
            BEGIN
             case ModReg:
             case ModRReg:
              BAsmCode[0]=0xe6-(OpSize*0x50); BAsmCode[1]=AdrVals[0];
              BAsmCode[2]=(ALUOrders[z].Code << 4)+HReg; CodeLen=3;
              break;
            END
           break;
          case ModIWRReg:
           HReg=AdrPart;
           DecodeAdr(ArgStr[2],(OpSize*MModWRReg)+Mask2+MModIWRReg+MModImm);
           switch (AdrMode)
            BEGIN
             case ModWRReg:
              BAsmCode[0]=(ALUOrders[z].Code << 4)+0x0e;
              BAsmCode[1]=(HReg << 4)+0x10+AdrPart; CodeLen=2;
              break;
             case ModReg:
             case ModRReg:
              BAsmCode[0]=0x72+(OpSize*76);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg+OpSize;
              BAsmCode[2]=AdrVals[0]; CodeLen=3;
              break;
             case ModIWRReg:
              if (OpSize==0) 
               BEGIN
                BAsmCode[0]=0x73; BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart;
                BAsmCode[2]=HReg+WorkOfs; CodeLen=3;
               END
              else
               BEGIN
                BAsmCode[0]=(ALUOrders[z].Code << 4)+0x0e;
                BAsmCode[1]=0x11+(HReg << 4)+AdrPart;
                CodeLen=2;
               END
              break;
             case ModImm:
              BAsmCode[0]=0xf3-(OpSize*0x35);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg;
              memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
              break;
            END
           break;
          case ModIncWRReg:
           HReg=AdrPart;
           DecodeAdr(ArgStr[2],Mask2);
           switch (AdrMode)
            BEGIN
             case ModReg:
             case ModRReg:
              BAsmCode[0]=0xb4+(OpSize*0x21);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg;
              BAsmCode[2]=AdrVals[0]; CodeLen=3;
              break;
            END
           break;
          case ModDecWRReg:
           HReg=AdrPart;
           DecodeAdr(ArgStr[2],Mask2);
           switch (AdrMode)
            BEGIN
             case ModReg:
             case ModRReg:
              BAsmCode[0]=0xc2+OpSize;
              BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg;
              BAsmCode[2]=AdrVals[0]; CodeLen=3;
              break;
            END
           break;
          case ModDispRWRReg:
           HReg=AdrPart; HPart=AdrVals[0];
           DecodeAdr(ArgStr[2],Mask1);
           switch (AdrMode)
            BEGIN
             case ModWReg:
             case ModWRReg:
              BAsmCode[0]=0x60;
              BAsmCode[1]=0x11-(OpSize*0x10)+(HPart << 4)+HReg;
              BAsmCode[2]=(ALUOrders[z].Code << 4)+AdrPart; CodeLen=3;
              break;
            END
           break;
          case ModDisp8WRReg:
           HReg=AdrPart; BAsmCode[2]=AdrVals[0];
           DecodeAdr(ArgStr[2],Mask2+(OpSize*MModImm));
           switch (AdrMode)
            BEGIN
             case ModReg:
             case ModRReg:
              BAsmCode[0]=0x26+(OpSize*0x60);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg+1;
              BAsmCode[3]=AdrVals[0]+OpSize; CodeLen=4;
              break;
             case ModImm:
              BAsmCode[0]=0x06; BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg+1;
              memcpy(BAsmCode+3,AdrVals,AdrCnt); CodeLen=3+AdrCnt;
              break;
            END
           break;
          case ModDisp16WRReg:
           HReg=AdrPart; memcpy(BAsmCode+2,AdrVals,2);
           DecodeAdr(ArgStr[2],Mask2+(OpSize*MModImm));
           switch (AdrMode)
            BEGIN
             case ModReg:
             case ModRReg:
              BAsmCode[0]=0x26+(OpSize*0x60);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg;
              BAsmCode[4]=AdrVals[0]+OpSize; CodeLen=5;
              break;
             case ModImm:
              BAsmCode[0]=0x06; BAsmCode[1]=(ALUOrders[z].Code << 4)+HReg;
              memcpy(BAsmCode+4,AdrVals,AdrCnt); CodeLen=4+AdrCnt;
              break;
            END
           break;
          case ModAbs:
           memcpy(BAsmCode+2,AdrVals,2);
           DecodeAdr(ArgStr[2],Mask1+MModImm);
           switch (AdrMode)
            BEGIN
             case ModWReg:
             case ModWRReg:
              BAsmCode[0]=0xc5+(OpSize*29);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart+OpSize;
              CodeLen=4;
              break;
             case ModImm:
              memmove(BAsmCode+2+AdrCnt,BAsmCode+2,2); memcpy(BAsmCode+2,AdrVals,AdrCnt);
              BAsmCode[0]=0x2f+(OpSize*7);
              BAsmCode[1]=(ALUOrders[z].Code << 4)+1;
              CodeLen=4+AdrCnt;
              break;
            END
           break;
          case ModIRReg:
           HReg=AdrVals[0];
           DecodeAdr(ArgStr[2],MModIWRReg*(1-OpSize));
           switch (AdrMode)
            BEGIN
             case ModIWRReg:
              BAsmCode[0]=0x73; BAsmCode[1]=(ALUOrders[z].Code << 4)+AdrPart;
              BAsmCode[2]=HReg; CodeLen=3;
              break;
            END
           break;
         END
       END
      return;
     END

   for (z=0; z<RegOrderCnt; z++)
    if (Memo(RegOrders[z].Name)) 
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg+MModIReg);
        switch (AdrMode)
         BEGIN
          case ModReg:
           BAsmCode[0]=RegOrders[z].Code; BAsmCode[1]=AdrVals[0]; CodeLen=2;
           break;
          case ModIReg:
           BAsmCode[0]=RegOrders[z].Code+1; BAsmCode[1]=AdrVals[0]; CodeLen=2;
           break;
         END
       END
      return;
     END

   for (z=0; z<Reg16OrderCnt; z++)
    if (Memo(Reg16Orders[z].Name)) 
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModRReg);
        switch (AdrMode)
         BEGIN
          case ModRReg:
           BAsmCode[0]=Reg16Orders[z].Code; BAsmCode[1]=AdrVals[0]+Ord(Memo("EXT"));
           CodeLen=2;
           break;
         END
       END
      return;
     END

   if ((Memo("DIV")) OR (Memo("MUL"))) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWRReg);
       if (AdrMode==ModWRReg) 
        BEGIN
         HReg=AdrPart;
         DecodeAdr(ArgStr[2],MModWReg);
         if (AdrMode==ModWReg) 
          BEGIN
           BAsmCode[0]=0x5f-(0x10*Ord(Memo("MUL")));
           BAsmCode[1]=(HReg << 4)+AdrPart;
           CodeLen=2;
          END
        END
      END
     return;
    END

   if (Memo("DIVWS")) 
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWRReg);
       if (AdrMode==ModWRReg) 
        BEGIN
         HReg=AdrPart;
         DecodeAdr(ArgStr[2],MModWRReg);
         if (AdrMode==ModWRReg) 
          BEGIN
           BAsmCode[2]=(HReg << 4)+AdrPart;
           DecodeAdr(ArgStr[3],MModRReg);
           if (AdrMode==ModRReg) 
            BEGIN
             BAsmCode[0]=0x56; BAsmCode[1]=AdrVals[0];
             CodeLen=3;
            END
          END
        END
      END
     return;
    END

   /* Bitoperationen */

   for (z=0; z<Bit2OrderCnt; z++)
    if (Memo(Bit2Orders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (SplitBit(ArgStr[1],&HReg))
       BEGIN
        if (Odd(HReg)) WrError(1350);
        else
         BEGIN
          DecodeAdr(ArgStr[1],MModWReg);
          if (AdrMode==ModWReg)
           BEGIN
            HReg=(HReg << 4)+AdrPart;
            if (SplitBit(ArgStr[2],&HPart))
             BEGIN
              DecodeAdr(ArgStr[2],MModWReg);
              if (AdrMode==ModWReg)
               BEGIN
                HPart=(HPart << 4)+AdrPart;
                BAsmCode[0]=Bit2Orders[z].Code;
                if (Memo("BLD"))
                 BEGIN
                  BAsmCode[1]=HPart | 0x10; BAsmCode[2]=HReg | (HPart & 0x10);
                 END
                else if (Memo("BXOR"))
                 BEGIN
                  BAsmCode[1]=0x10+HReg; BAsmCode[2]=HPart;
                 END
                else
                 BEGIN
                  BAsmCode[1]=0x10+HReg; BAsmCode[2]=HPart ^ 0x10;
                 END
                CodeLen=3;
               END
             END
           END
         END
       END
      return;
     END

   for (z=0; z<Bit1OrderCnt; z++)
    if (Memo(Bit1Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (SplitBit(ArgStr[1],&HReg))
       BEGIN
        if (Odd(HReg)) WrError(1350);
        else
         BEGIN
          DecodeAdr(ArgStr[1],MModWReg+(Ord(Memo("BTSET"))*MModIWRReg));
          switch (AdrMode)
           BEGIN
            case ModWReg:
             BAsmCode[0]=Bit1Orders[z].Code; BAsmCode[1]=(HReg << 4)+AdrPart;
             CodeLen=2;
             break;
            case ModIWRReg:
             BAsmCode[0]=0xf6; BAsmCode[1]=(HReg << 4)+AdrPart;
             CodeLen=2;
             break;
           END
         END
       END
      return;
     END

   /* Spruenge */

   if ((Memo("BTJF")) OR (Memo("BTJT"))) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (SplitBit(ArgStr[1],&HReg)) 
      BEGIN
       if (Odd(HReg)) WrError(1350);
       else
        BEGIN
         DecodeAdr(ArgStr[1],MModWReg);
         if (AdrMode==ModWReg) 
          BEGIN
           BAsmCode[1]=(HReg << 4)+AdrPart+(Ord(Memo("BTJF")) << 4);
           AdrInt=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+3);
           if (OK) 
            BEGIN
             if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
             else
              BEGIN
               BAsmCode[0]=0xaf; BAsmCode[2]=AdrInt & 0xff; CodeLen=3;
               ChkSpace(SegCode);
              END
            END
          END
        END
      END
     return;
    END

   if ((Memo("JP")) OR (Memo("CALL"))) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AbsSeg=SegCode;
       DecodeAdr(ArgStr[1],MModIRReg+MModAbs);
       switch (AdrMode)
        BEGIN
         case ModIRReg:
          BAsmCode[0]=0x74+(Ord(Memo("JP"))*0x60);
          BAsmCode[1]=AdrVals[0]+Ord(Memo("CALL")); CodeLen=2;
          break;
         case ModAbs:
          BAsmCode[0]=0x8d+(Ord(Memo("CALL"))*0x45);
          memcpy(BAsmCode+1,AdrVals,AdrCnt); CodeLen=1+AdrCnt;
          break;
        END
      END
     return;
    END

   if ((Memo("CPJFI")) OR (Memo("CPJTI"))) 
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg);
       if (AdrMode==ModWReg) 
        BEGIN
         HReg=AdrPart;
         DecodeAdr(ArgStr[2],MModIWRReg);
         if (AdrMode==ModIWRReg) 
          BEGIN
           BAsmCode[1]=(AdrPart << 4)+(Ord(Memo("CPJTI")) << 4)+HReg;
           AdrInt=EvalIntExpression(ArgStr[3],UInt16,&OK)-(EProgCounter()+3);
           if (OK) 
            BEGIN
             if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
             else
              BEGIN
               ChkSpace(SegCode);
               BAsmCode[0]=0x9f; BAsmCode[2]=AdrInt & 0xff;
               CodeLen=3;
              END
            END
          END
        END
      END
     return;
    END

   if (Memo("DJNZ")) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg);
       if (AdrMode==ModWReg) 
        BEGIN
         BAsmCode[0]=(AdrPart << 4)+0x0a;
         AdrInt=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+2);
         if (OK) 
          BEGIN
           if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
           else
            BEGIN
             ChkSpace(SegCode);
             BAsmCode[1]=AdrInt & 0xff;
             CodeLen=2;
            END
          END
        END
      END
     return;
    END

   if (Memo("DWJNZ")) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModRReg);
       if (AdrMode==ModRReg) 
        BEGIN
         BAsmCode[1]=AdrVals[0];
         AdrInt=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+3);
         if (OK) 
          BEGIN
           if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
           else
            BEGIN
             ChkSpace(SegCode);
             BAsmCode[0]=0xc6; BAsmCode[2]=AdrInt & 0xff;
             CodeLen=3;
            END
          END
        END
      END
     return;
    END

   for (z=0; z<ConditionCnt; z++)
    if ((*OpPart=='J') AND (OpPart[1]=='P') AND (strcmp(OpPart+2,Conditions[z].Name)==0))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
        if (OK)
         BEGIN
          ChkSpace(SegCode);
          BAsmCode[0]=0x0d+(Conditions[z].Code << 4);
          BAsmCode[1]=Hi(AdrWord); BAsmCode[2]=Lo(AdrWord);
          CodeLen=3;
         END
       END
      return;
     END
    else if ((*OpPart=='J') AND (OpPart[1]=='R') AND (strcmp(OpPart+2,Conditions[z].Name)==0))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+2);
        if (OK)
         BEGIN
          if ((NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
          else
           BEGIN
            ChkSpace(SegCode);
            BAsmCode[0]=0x0b+(Conditions[z].Code << 4); BAsmCode[1]=AdrInt & 0xff;
            CodeLen=2;
           END
         END
       END
      return;
     END

   /* Besonderheiten */

   if (Memo("SPP")) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      BEGIN
       BAsmCode[1]=(EvalIntExpression(ArgStr[1]+1,UInt6,&OK) << 2)+0x02;
       if (OK) 
        BEGIN
         BAsmCode[0]=0xc7; CodeLen=2;
        END
      END
     return;
    END

   if ((Memo("SRP")) OR (Memo("SRP0")) OR (Memo("SRP1"))) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[1]+1,UInt5,&OK) << 3;
       if (OK)
        BEGIN
         BAsmCode[0]=0xc7; CodeLen=2;
         if (strlen(OpPart)==4) BAsmCode[1]+=4;
         if (OpPart[strlen(OpPart)-1]=='1') BAsmCode[1]++;
        END
      END
     return;
    END

   /* Fakes... */

   if (Memo("SLA")) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg+MModReg+MModIWRReg);
       switch (AdrMode)
        BEGIN
         case ModWReg:
          BAsmCode[0]=0x42; BAsmCode[1]=(AdrPart << 4)+AdrPart;
          CodeLen=2;
          break;
         case ModReg:
          BAsmCode[0]=0x44;
          BAsmCode[1]=AdrVals[0];
          BAsmCode[2]=AdrVals[0];
          CodeLen=3;
          break;
         case ModIWRReg:
          BAsmCode[0]=0x73; BAsmCode[1]=0x40+AdrPart;
          BAsmCode[2]=WorkOfs+AdrPart; CodeLen=3;
          break;
        END
      END
     return;
    END

   if (Memo("SLAW")) 
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModWRReg+MModRReg+MModIWRReg);
       switch (AdrMode)
        BEGIN
         case ModWRReg:
          BAsmCode[0]=0x4e; BAsmCode[1]=(AdrPart << 4)+AdrPart;
          CodeLen=2;
          break;
         case ModRReg:
          BAsmCode[0]=0x47;
          BAsmCode[1]=AdrVals[0];
          BAsmCode[2]=AdrVals[0];
          CodeLen=3;
          break;
         case ModIWRReg:
          BAsmCode[0]=0x4e; BAsmCode[1]=0x11+(AdrPart << 4)+AdrPart;
          CodeLen=2;
          break;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static void InitCode_ST9(void)
BEGIN
   SaveInitProc();

   DPAssume=0;
END

        static Boolean IsDef_ST9(void)
BEGIN
   return (Memo("REG") OR Memo("BIT"));
END

        static void SwitchFrom_ST9(void)
BEGIN
   DeinitFields();
END

        static void InternSymbol_ST9(char *Asc, TempResult *Erg)
BEGIN
   String h;
   Boolean Err;
   Boolean Pair;

   Erg->Typ=TempNone;
   if ((strlen(Asc)<2) OR (*Asc!='R')) return;

   strmaxcpy(h,Asc+1,255);
   if (*h=='R')
    BEGIN
     if (strlen(h)<2) return;
     Pair=True; strcpy(h,h+1);
    END
   else Pair=False;
   Erg->Contents.Int=ConstLongInt(h,&Err,10);
   if ((NOT Err) OR (Erg->Contents.Int<0) OR (Erg->Contents.Int>255)) return;
   if ((Erg->Contents.Int & 0xf0)==0xd0) return;
   if ((Pair) AND (Odd(Erg->Contents.Int))) return;

   if (Pair) Erg->Contents.Int+=0x100;
   Erg->Typ=TempInt; TypeFlag|=(1 << SegReg);
END

        static void SwitchTo_ST9(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="PC"; HeaderID=0x32; NOPCode=0xff;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData)|(1<<SegReg);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;
   SegLimits[SegData] = 0xffff;
   Grans[SegReg ]=1; ListGrans[SegReg ]=1; SegInits[SegReg ]=0;
   SegLimits[SegReg ] = 0xff;

   MakeCode=MakeCode_ST9; IsDef=IsDef_ST9;
   SwitchFrom=SwitchFrom_ST9; InternSymbol=InternSymbol_ST9;

   InitFields();
END

        void codest9_init(void)
BEGIN
   CPUST9020=AddCPU("ST9020",SwitchTo_ST9);
   CPUST9030=AddCPU("ST9030",SwitchTo_ST9);
   CPUST9040=AddCPU("ST9040",SwitchTo_ST9);
   CPUST9050=AddCPU("ST9050",SwitchTo_ST9);
   SaveInitProc=InitPassProc; InitPassProc=InitCode_ST9;
END


