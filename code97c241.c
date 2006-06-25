/* code97c241.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TLCS-9000                                                   */
/*                                                                           */
/* Historie:                                                                 */
/*            2. 1.1999 ChkPC umgebaut                                       */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code97c241.c,v 1.5 2006/06/15 20:48:36 alfred Exp $                  */
/*****************************************************************************
 * $Log: code97c241.c,v $
 * Revision 1.5  2006/06/15 20:48:36  alfred
 * - correct function call
 *
 * Revision 1.4  2005/10/02 10:00:45  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2005/09/08 17:31:04  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:02  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h"

#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"
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
          Byte Code;
          Byte Mask;     /* B0..2=OpSizes, B4=-MayImm, B5=-MayReg */
         } RMWOrder;

typedef struct
         {
          char *Name;
          Byte Code;
          Byte Mask;     /* B7: DD in A-Format gedreht */
          enum {Equal,FirstCounts,SecondCounts,Op2Half} SizeType;
          Boolean ImmKorr,ImmErl,RegErl;
         } GAOrder;
           

#define ConditionCount 20
#define FixedOrderCount 20
#define RMWOrderCount 14
#define TrinomOrderCount 4
#define StringOrderCount 4
#define BFieldOrderCount 3
#define GAEqOrderCount 10
#define GAFirstOrderCount 6
#define GASecondOrderCount 4
#define GAHalfOrderCount 4
#define GASI1OrderCount 4
#define GASI2OrderCount 3
#define BitOrderCount 4
#define ShiftOrderCount 8


static CPUVar CPU97C241;

static int OpSize,OpSize2;
static Integer LowLim4,LowLim8;

static Boolean AdrOK;
static Byte AdrMode,AdrMode2;
static Byte AdrCnt2;
static Word AdrVals[2],AdrVals2[2];
static int AdrInc;
static Word Prefs[2];
static Boolean PrefUsed[2];
static char Format;
static Boolean MinOneIs0;

static FixedOrder *FixedOrders;
static RMWOrder *RMWOrders;
static char **GASI1Orders;
static char **GASI2Orders;
static char **TrinomOrders;
static char **BitOrders;
static char **ShiftOrders;
static char **BFieldOrders;
static FixedOrder *GAEqOrders;
static FixedOrder *GAHalfOrders;
static FixedOrder *GAFirstOrders;
static FixedOrder *GASecondOrders;
static char **StringOrders;
static char **Conditions;

/*--------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddRMW(char *NName, Byte NCode, Byte NMask)
BEGIN
   if (InstrZ>=RMWOrderCount) exit(255);
   RMWOrders[InstrZ].Name=NName;
   RMWOrders[InstrZ].Mask=NMask;
   RMWOrders[InstrZ++].Code=NCode;
END

        static void AddGAEq(char *NName, Word NCode)
BEGIN
   if (InstrZ>=GAEqOrderCount) exit(255);
   GAEqOrders[InstrZ].Name=NName;
   GAEqOrders[InstrZ++].Code=NCode;
END

        static void AddGAHalf(char *NName, Word NCode)
BEGIN
   if (InstrZ>=GAHalfOrderCount) exit(255);
   GAHalfOrders[InstrZ].Name=NName;
   GAHalfOrders[InstrZ++].Code=NCode;
END

        static void AddGAFirst(char *NName, Word NCode)
BEGIN
   if (InstrZ>=GAFirstOrderCount) exit(255);
   GAFirstOrders[InstrZ].Name=NName;
   GAFirstOrders[InstrZ++].Code=NCode;
END

        static void AddGASecond(char *NName, Word NCode)
BEGIN
   if (InstrZ>=GASecondOrderCount) exit(255);
   GASecondOrders[InstrZ].Name=NName;
   GASecondOrders[InstrZ++].Code=NCode;
END                               

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("CCF" , 0x7f82);
   AddFixed("CSF" , 0x7f8a);
   AddFixed("CVF" , 0x7f86);
   AddFixed("CZF" , 0x7f8e);
   AddFixed("DI"  , 0x7fa1);
   AddFixed("EI"  , 0x7fa3);
   AddFixed("HALT", 0x7fa5);
   AddFixed("NOP" , 0x7fa0);
   AddFixed("RCF" , 0x7f80);
   AddFixed("RET" , 0x7fa4);
   AddFixed("RETI", 0x7fa9);
   AddFixed("RETS", 0x7fab);
   AddFixed("RSF" , 0x7f88);
   AddFixed("RVF" , 0x7f84);
   AddFixed("RZF" , 0x7f8c);
   AddFixed("SCF" , 0x7f81);
   AddFixed("SSF" , 0x7f89);
   AddFixed("SVF" , 0x7f85);
   AddFixed("SZF" , 0x7f8b);
   AddFixed("UNLK", 0x7fa2);

   RMWOrders=(RMWOrder *) malloc(sizeof(RMWOrder)*RMWOrderCount); InstrZ=0;
   AddRMW("CALL" , 0x35, 0x36);
   AddRMW("CLR"  , 0x2b, 0x17);
   AddRMW("CPL"  , 0x28, 0x17);
   AddRMW("EXTS" , 0x33, 0x16);
   AddRMW("EXTZ" , 0x32, 0x16);
   AddRMW("JP"   , 0x34, 0x36);
   AddRMW("MIRR" , 0x23, 0x17);
   AddRMW("NEG"  , 0x29, 0x17);
   AddRMW("POP"  , 0x20, 0x17);
   AddRMW("PUSH" , 0x21, 0x07);
   AddRMW("PUSHA", 0x31, 0x36);
   AddRMW("RVBY" , 0x22, 0x17);
   AddRMW("TJP"  , 0x36, 0x16);
   AddRMW("TST"  , 0x2a, 0x17);

   GASI1Orders=(char **) malloc(sizeof(char *)*GASI1OrderCount); InstrZ=0;
   GASI1Orders[InstrZ++]="ADD";
   GASI1Orders[InstrZ++]="SUB";
   GASI1Orders[InstrZ++]="CP";
   GASI1Orders[InstrZ++]="LD";

   GASI2Orders=(char **) malloc(sizeof(char *)*GASI2OrderCount); InstrZ=0;
   GASI2Orders[InstrZ++]="AND";
   GASI2Orders[InstrZ++]="OR";
   GASI2Orders[InstrZ++]="XOR";

   TrinomOrders=(char **) malloc(sizeof(char *)*TrinomOrderCount); InstrZ=0;
   TrinomOrders[InstrZ++]="ADD3";
   TrinomOrders[InstrZ++]="SUB3";
   TrinomOrders[InstrZ++]="MAC";
   TrinomOrders[InstrZ++]="MACS";

   BitOrders=(char **) malloc(sizeof(char *)*BitOrderCount); InstrZ=0;
   BitOrders[InstrZ++]="BRES";
   BitOrders[InstrZ++]="BSET";
   BitOrders[InstrZ++]="BCHG";
   BitOrders[InstrZ++]="BTST";

   ShiftOrders=(char **) malloc(sizeof(char *)*ShiftOrderCount); InstrZ=0;
   ShiftOrders[InstrZ++]="SLL";
   ShiftOrders[InstrZ++]="SRL";
   ShiftOrders[InstrZ++]="SLA";
   ShiftOrders[InstrZ++]="SRA";
   ShiftOrders[InstrZ++]="RL";
   ShiftOrders[InstrZ++]="RR";
   ShiftOrders[InstrZ++]="RLC";
   ShiftOrders[InstrZ++]="RRC";

   BFieldOrders=(char **) malloc(sizeof(char *)*BFieldOrderCount); InstrZ=0;
   BFieldOrders[InstrZ++]="BFEX";
   BFieldOrders[InstrZ++]="BFEXS";
   BFieldOrders[InstrZ++]="BFIN";

   GAEqOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*GAEqOrderCount); InstrZ=0;
   AddGAEq("ABCD" , 0x10);
   AddGAEq("ADC"  , 0x04);
   AddGAEq("CBCD" , 0x12);
   AddGAEq("CPC"  , 0x06);
   AddGAEq("MAX"  , 0x16);
   AddGAEq("MAXS" , 0x17);
   AddGAEq("MIN"  , 0x14);
   AddGAEq("MINS" , 0x15);
   AddGAEq("SBC"  , 0x05);
   AddGAEq("SBCD" , 0x11);

   GAHalfOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*GAHalfOrderCount); InstrZ=0;
   AddGAHalf("DIV"  , 0x26);
   AddGAHalf("DIVS" , 0x27);
   AddGAHalf("MUL"  , 0x24);
   AddGAHalf("MULS" , 0x25);

   GAFirstOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*GAFirstOrderCount); InstrZ=0;
   AddGAFirst("ANDCF", 0x44);
   AddGAFirst("LDCF" , 0x47);
   AddGAFirst("ORCF" , 0x45);
   AddGAFirst("STCF" , 0x43);
   AddGAFirst("TSET" , 0x70);
   AddGAFirst("XORCF", 0x46);

   GASecondOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*GASecondOrderCount); InstrZ=0;
   AddGASecond("BS0B" , 0x54);
   AddGASecond("BS0F" , 0x55);
   AddGASecond("BS1B" , 0x56);
   AddGASecond("BS1F" , 0x57);
   
   StringOrders=(char **) malloc(sizeof(char *)*StringOrderCount); InstrZ=0;
   StringOrders[InstrZ++]="CPSZ";
   StringOrders[InstrZ++]="CPSN";
   StringOrders[InstrZ++]="";
   StringOrders[InstrZ++]="LDS";

   Conditions=(char **) malloc(sizeof(char *)*ConditionCount); InstrZ=0;
   Conditions[InstrZ++]="C";   Conditions[InstrZ++]="NC";
   Conditions[InstrZ++]="Z";   Conditions[InstrZ++]="NZ";
   Conditions[InstrZ++]="OV";  Conditions[InstrZ++]="NOV";
   Conditions[InstrZ++]="MI";  Conditions[InstrZ++]="PL";
   Conditions[InstrZ++]="LE";  Conditions[InstrZ++]="GT";
   Conditions[InstrZ++]="LT";  Conditions[InstrZ++]="GE";
   Conditions[InstrZ++]="ULE"; Conditions[InstrZ++]="UGT";
   Conditions[InstrZ++]="N";   Conditions[InstrZ++]="A";
   Conditions[InstrZ++]="ULT"; Conditions[InstrZ++]="UGE";
   Conditions[InstrZ++]="EQ";  Conditions[InstrZ++]="NE";
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(RMWOrders);
   free(GASI1Orders);
   free(GASI2Orders);
   free(TrinomOrders);
   free(BitOrders);
   free(ShiftOrders);
   free(BFieldOrders);
   free(GAEqOrders);
   free(GAHalfOrders);
   free(GAFirstOrders);
   free(GASecondOrders);
   free(StringOrders);
   free(Conditions);
END

/*--------------------------------------------------------------------------*/

        static void AddSignedPrefix(Byte Index, Byte MaxBits, LongInt Value)
BEGIN
   LongInt Max;

   Max=1l << (MaxBits-1);
   if ((Value<-Max) OR (Value>=Max))
    BEGIN
     PrefUsed[Index]=True;
     Prefs[Index]=(Value >> MaxBits) & 0x7ff;
    END
END

        static Boolean AddRelPrefix(Byte Index, Byte MaxBits, LongInt *Value)
BEGIN
   LongInt Max1,Max2;

   Max1=1l << (MaxBits-1);
   Max2=1l << (MaxBits+10);
   if ((*Value<-Max2) OR (*Value>=Max2)) WrError(1370);
   else
    BEGIN
     if ((*Value<-Max1) OR (*Value>=Max1))
      BEGIN
       PrefUsed[Index]=True;
       Prefs[Index]=((*Value) >> MaxBits) & 0x7ff;
      END
     return True;
    END
   return False;
END

        static void AddAbsPrefix(Byte Index, Byte MaxBits, LongInt Value)
BEGIN
   LongInt Dist;

   Dist=1l << (MaxBits-1);
   if ((Value>=Dist) AND (Value<0x1000000-Dist))
    BEGIN
     PrefUsed[Index]=True;
     Prefs[Index]=(Value >> MaxBits) & 0x7ff;
    END
END

        static void InsertSinglePrefix(Byte Index)
BEGIN
   if (PrefUsed[Index])
    BEGIN
     memmove(WAsmCode+1,WAsmCode+0,CodeLen);
     WAsmCode[0]=Prefs[Index]+0xd000+(((Word)Index) << 11);
     CodeLen+=2;
    END
END

        static Boolean DecodeReg(char *Asc, Byte *Result)
BEGIN
   Byte tmp;
   Boolean Err;

   if ((strlen(Asc)>4) OR (strlen(Asc)<3) OR (toupper(*Asc)!='R')) return False;
   switch (toupper(Asc[1]))
    BEGIN
     case 'B': *Result=0x00; break;
     case 'W': *Result=0x40; break;
     case 'D': *Result=0x80; break;
     default: return False;
    END
   tmp=ConstLongInt(Asc+2,&Err,10);
   if ((NOT Err) OR (tmp>15)) return False;
   if ((*Result==0x80) AND (Odd(tmp))) return False;
   *Result+=tmp; return True;
END

        static Boolean DecodeSpecReg(char *Asc, Byte *Result)
BEGIN
   if (strcasecmp(Asc,"SP")==0) *Result=0x8c;
   else if (strcasecmp(Asc,"ISP")==0) *Result=0x81;
   else if (strcasecmp(Asc,"ESP")==0) *Result=0x83;
   else if (strcasecmp(Asc,"PBP")==0) *Result=0x05;
   else if (strcasecmp(Asc,"CBP")==0) *Result=0x07;
   else if (strcasecmp(Asc,"PSW")==0) *Result=0x89;
   else if (strcasecmp(Asc,"IMC")==0) *Result=0x0b;
   else if (strcasecmp(Asc,"CC")==0)  *Result=0x0e;
   else return False;
   return True;
END

        static Boolean DecodeRegAdr(char *Asc, Byte *Erg)
BEGIN
   if (NOT DecodeReg(Asc,Erg)) return False;
   if (OpSize==-1) OpSize=(*Erg) >> 6;
   if (((*Erg) >> 6)!=OpSize)
    BEGIN
     WrError(1132); return False;
    END
   *Erg&=0x3f;
   return True;
END

        static void DecodeAdr(char *Asc, Byte PrefInd, Boolean MayImm, Boolean MayReg)
BEGIN
#define FreeReg 0xff
#define SPReg 0xfe
#define PCReg 0xfd

   Byte Reg;
   String AdrPart,tmp;
   Byte BaseReg,IndReg,ScaleFact;
   LongInt DispPart,DispAcc;
   Boolean OK,MinFlag,NMinFlag;
   char *MPos,*PPos,*EPos;
   int l;

   AdrCnt=0; AdrOK=False;

   /* I. Speicheradresse */

   if (IsIndirect(Asc))
    BEGIN
     /* I.1. vorkonditionieren */

     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0'; KillBlanks(Asc);

     /* I.2. Predekrement */

     if ((*Asc=='-') AND (Asc[1]=='-')
     AND (DecodeReg(Asc+2,&Reg)))
      BEGIN
       switch (Reg >> 6)
        BEGIN
         case 0:
          WrError(1350); break;
         case 1:
          AdrMode=0x50+(Reg & 15); AdrOK=True;
          break;
         case 2:
          AdrMode=0x71+(Reg & 14); AdrOK=True;
          break;
        END
       return;
      END

     /* I.3. Postinkrement */

     l=strlen(Asc);
     if ((Asc[l-1]=='+') AND (Asc[l-2]=='+'))
      BEGIN
       strmaxcpy(AdrPart,Asc,255); AdrPart[l-2]='\0';
       if (DecodeReg(AdrPart,&Reg))
        BEGIN
         switch (Reg >> 6)
          BEGIN
           case 0:
            WrError(1350); break;
           case 1:
            AdrMode=0x40+(Reg & 15); AdrOK=True;
            break;
           case 2:
            AdrMode=0x70+(Reg & 14); AdrOK=True;
            break;
          END
         return;
        END
      END

     /* I.4. Adresskomponenten zerlegen */

     BaseReg=FreeReg; IndReg=FreeReg; ScaleFact=0;
     DispAcc=AdrInc; MinFlag=False;
     while (*Asc!='\0')
      BEGIN

       /* I.4.a. Trennzeichen suchen */

       MPos=QuotPos(Asc,'-'); PPos=QuotPos(Asc,'+');
       if (PPos==Nil) EPos=MPos;
       else if (MPos==Nil) EPos=PPos;
       else EPos=min(MPos,PPos);
       NMinFlag=((EPos!=Nil) AND (*EPos=='-'));
       if (EPos==Nil)
        BEGIN
         strmaxcpy(AdrPart,Asc,255); *Asc='\0';
        END
       else
        BEGIN
         *EPos='\0'; strmaxcpy(AdrPart,Asc,255); strcpy(Asc,EPos+1);
        END

       /* I.4.b. Indexregister mit Skalierung */

       EPos=QuotPos(AdrPart,'*');
       if (EPos!=Nil)
        BEGIN
         strcpy(tmp,AdrPart); tmp[EPos-AdrPart]='\0';
        END
       l=strlen(AdrPart);
       if ((EPos==AdrPart+l-2) 
       AND ((AdrPart[l-1]=='1') OR (AdrPart[l-1]=='2') OR (AdrPart[l-1]=='4') OR (AdrPart[l-1]=='8'))
       AND (DecodeReg(tmp,&Reg)))
        BEGIN
         if (((Reg >> 6)==0) OR (MinFlag) OR (IndReg!=FreeReg))
          BEGIN
           WrError(1350); return;
          END
         IndReg=Reg;
         switch (AdrPart[l-1])
          BEGIN
           case '1': ScaleFact=0; break;
           case '2': ScaleFact=1; break;
           case '4': ScaleFact=2; break;
           case '8': ScaleFact=3; break;
          END
        END

       /* I.4.c. Basisregister */

       else if (DecodeReg(AdrPart,&Reg))
        BEGIN
         if (((Reg >> 6)==0) OR (MinFlag))
          BEGIN
           WrError(1350); return;
          END
         if (BaseReg==FreeReg) BaseReg=Reg;
         else if (IndReg==FreeReg)
          BEGIN
           IndReg=Reg; ScaleFact=0;
          END
         else
          BEGIN
           WrError(1350); return;
          END
        END

       /* I.4.d. Sonderregister */

       else if ((strcasecmp(AdrPart,"PC")==0) OR (strcasecmp(AdrPart,"SP")==0))
        BEGIN
         if ((BaseReg!=FreeReg) AND (IndReg==FreeReg))
          BEGIN
           IndReg=BaseReg; BaseReg=FreeReg; ScaleFact=0;
          END;
         if ((BaseReg!=FreeReg) OR (MinFlag))
          BEGIN
           WrError(1350); return;
          END
         BaseReg=(strcasecmp(AdrPart,"SP")==0) ? SPReg : PCReg;
        END

       /* I.4.e. Displacement */

       else
        BEGIN
         FirstPassUnknown=False;
         DispPart=EvalIntExpression(AdrPart,Int32,&OK);
         if (NOT OK) return;
         if (FirstPassUnknown) DispPart=1;
         if (MinFlag) DispAcc-=DispPart; else DispAcc+=DispPart;
        END
       MinFlag=NMinFlag;
      END

     /* I.5. Indexregister mit Skalierung 1 als Basis behandeln */

     if ((BaseReg==FreeReg) AND (IndReg!=FreeReg) AND (ScaleFact==0))
      BEGIN
       BaseReg=IndReg; IndReg=FreeReg;
      END

     /* I.6. absolut */

     if ((BaseReg==FreeReg) AND (IndReg==FreeReg))
      BEGIN
       AdrMode=0x20;
       AdrVals[0]=0xe000+(DispAcc & 0x1fff); AdrCnt=2;
       AddAbsPrefix(PrefInd,13,DispAcc);
       AdrOK=True; return;
      END

     /* I.7. Basis [mit Displacement] */

     if ((BaseReg!=FreeReg) AND (IndReg==FreeReg))
      BEGIN

       /* I.7.a. Basis ohne Displacement */

       if (DispAcc==0)
        BEGIN
         if ((BaseReg >> 6)==1) AdrMode=0x10+(BaseReg & 15);
         else AdrMode=0x61+(BaseReg & 14);
         AdrOK=True; return;
        END

       /* I.7.b. Nullregister mit Displacement muss in Erweiterungswort */

       else if ((BaseReg & 15)==0)
        BEGIN
         if (DispAcc>0x7ffff) WrError(1320);
         else if (DispAcc<-0x80000) WrError(1315);
         else
          BEGIN
           AdrMode=0x20;
           if ((BaseReg >> 6)==1) AdrVals[0]=((Word)BaseReg & 15) << 11;
           else AdrVals[0]=(((Word)BaseReg & 14) << 11)+0x8000;
           AdrVals[0]+=DispAcc & 0x1ff; AdrCnt=2;
           AddSignedPrefix(PrefInd,9,DispAcc);
           AdrOK=True;
          END
         return;
        END

       /* I.7.c. Stack mit Displacement: Optimierung moeglich */

       else if (BaseReg==SPReg)
        BEGIN
         if (DispAcc>0x7ffff) WrError(1320);
         else if (DispAcc<-0x80000) WrError(1315);
         else if ((DispAcc>=0) AND (DispAcc<=127))
          BEGIN
           AdrMode=0x80+(DispAcc & 0x7f); AdrOK=True;
          END
         else
          BEGIN
           AdrMode=0x20;
           AdrVals[0]=0xd000+(DispAcc & 0x1ff); AdrCnt=2;
           AddSignedPrefix(PrefInd,9,DispAcc);
           AdrOK=True;
          END
         return;
        END

       /* I.7.d. Programmzaehler mit Displacement: keine Optimierung */

       else if (BaseReg==PCReg)
        BEGIN
         if (DispAcc>0x7ffff) WrError(1320);
         else if (DispAcc<-0x80000) WrError(1315);
         else
          BEGIN
           AdrMode=0x20;
           AdrVals[0]=0xd800+(DispAcc & 0x1ff); AdrCnt=2;
           AddSignedPrefix(PrefInd,9,DispAcc);
           AdrOK=True;
          END
         return;
        END

       /* I.7.e. einfaches Basisregister mit Displacement */

       else
        BEGIN
         if (DispAcc>0x7fffff) WrError(1320);
         else if (DispAcc<-0x800000) WrError(1315);
         else
          BEGIN
           if ((BaseReg >> 6)==1) AdrMode=0x20+(BaseReg & 15);
           else AdrMode=0x60+(BaseReg & 14);
           AdrVals[0]=0xe000+(DispAcc & 0x1fff); AdrCnt=2;
           AddSignedPrefix(PrefInd,13,DispAcc);
           AdrOK=True;
          END
         return;
        END
      END

     /* I.8. Index- [und Basisregister] */

     else
      BEGIN
       if (DispAcc>0x7ffff) WrError(1320);
       else if (DispAcc<-0x80000) WrError(1315);
       else if ((IndReg & 15)==0) WrError(1350);
       else
        BEGIN
         if ((IndReg >> 6)==1) AdrMode=0x20+(IndReg & 15);
         else AdrMode=0x60+(IndReg & 14);
         switch (BaseReg)
          BEGIN
           case FreeReg :AdrVals[0]=0xc000; break;
           case SPReg   :AdrVals[0]=0xd000; break;
           case PCReg   :AdrVals[0]=0xd800; break;
           case 0x40: case 0x41: case 0x42: case 0x43:
           case 0x44: case 0x45: case 0x46: case 0x47:
           case 0x48: case 0x49: case 0x4a: case 0x4b:
           case 0x4c: case 0x4d: case 0x4e: case 0x4f:
            AdrVals[0]=((Word)BaseReg & 15) << 11; break;
           case 0x80: case 0x81: case 0x82: case 0x83:
           case 0x84: case 0x85: case 0x86: case 0x87:
           case 0x88: case 0x89: case 0x8a: case 0x8b:
           case 0x8c: case 0x8d: case 0x8e:
            AdrVals[0]=0x8000+(((Word)BaseReg & 14) << 10); break;
          END
         AdrVals[0]+=(((Word)ScaleFact) << 9)+(DispAcc & 0x1ff);
         AdrCnt=2;
         AddSignedPrefix(PrefInd,9,DispAcc);
         AdrOK=True;
        END
       return;
      END;
    END

   /* II. Arbeitsregister */

   else if (DecodeReg(Asc,&Reg))
    BEGIN
     if (NOT MayReg) WrError(1350);
     else
      BEGIN
       if (OpSize==-1) OpSize=Reg >> 6;
       if ((Reg >> 6)!=OpSize) WrError(1131);
       else
        BEGIN
         AdrMode=Reg & 15; AdrOK=True;
        END
      END
     return;
    END

   /* III. Spezialregister */

   else if (DecodeSpecReg(Asc,&Reg))
    BEGIN
     if (NOT MayReg) WrError(1350);
     else
      BEGIN
       if (OpSize==-1) OpSize=Reg >> 6;
       if ((Reg >> 6)!=OpSize) WrError(1131);
       else
        BEGIN
         AdrMode=0x30+(Reg & 15); AdrOK=True;
        END
      END
     return;
    END

   else if (NOT MayImm) WrError(1350);
   else
    BEGIN
     if ((OpSize==-1) AND (MinOneIs0)) OpSize=0;
     if (OpSize==-1) WrError(1132);
     else
      BEGIN
       AdrMode=0x30;
       switch (OpSize)
        BEGIN
         case 0:
          AdrVals[0]=EvalIntExpression(Asc,Int8,&OK) & 0xff;
          if (OK)
           BEGIN
            AdrCnt=2; AdrOK=True;
           END
          break;
         case 1:
          AdrVals[0]=EvalIntExpression(Asc,Int16,&OK);
          if (OK)
           BEGIN
            AdrCnt=2; AdrOK=True;
           END
          break;
         case 2:
          DispAcc=EvalIntExpression(Asc,Int32,&OK);
          if (OK)
           BEGIN
            AdrVals[0]=DispAcc & 0xffff;
            AdrVals[1]=DispAcc >> 16;
            AdrCnt=4; AdrOK=True;
           END
          break;
        END
      END
    END
END

        static void CopyAdr(void)
BEGIN
   OpSize2=OpSize;
   AdrMode2=AdrMode;
   AdrCnt2=AdrCnt;
   memcpy(AdrVals2,AdrVals,AdrCnt);
END

        static Boolean IsReg(void)
BEGIN
   return (AdrMode<=15);
END

        static Boolean Is2Reg(void)
BEGIN
   return (AdrMode2<=15);
END

        static Boolean IsImmediate(void)
BEGIN
   return (AdrMode==0x30);
END

        static Boolean Is2Immediate(void)
BEGIN
   return (AdrMode2==0x30);
END

        static LongInt ImmVal(void)
BEGIN
   LongInt Tmp1;
   Integer Tmp2;
   ShortInt Tmp3;

   switch (OpSize)
    BEGIN
     case 0:
      Tmp3=AdrVals[0] & 0xff; return Tmp3;
     case 1:
      Tmp2=AdrVals[0]; return Tmp2;
     case 2:
      Tmp1=(((LongInt)AdrVals[1]) << 16)+AdrVals[0]; return Tmp1;
     default:
      WrError(10000); return 0;
    END
END

        static LongInt ImmVal2(void)
BEGIN
   LongInt Tmp1;
   Integer Tmp2;
   ShortInt Tmp3;

   switch (OpSize)
    BEGIN
     case 0:
      Tmp3=AdrVals2[0] & 0xff; return Tmp3;
     case 1:
      Tmp2=AdrVals2[0]; return Tmp2;
     case 2:
      Tmp1=(((LongInt)AdrVals2[1]) << 16)+AdrVals2[0]; return Tmp1;
     default:
      WrError(10000); return 0;
    END
END

        static Boolean IsAbsolute(void)
BEGIN
   return (((AdrMode==0x20) OR (AdrMode==0x60)) AND (AdrCnt==2)
           AND ((AdrVals[0] & 0xe000)==0xe000));
END

        static Boolean Is2Absolute(void)
BEGIN
   return (((AdrMode2==0x20) OR (AdrMode2==0x60)) AND (AdrCnt2==2)
           AND ((AdrVals2[0] & 0xe000)==0xe000));
END

        static Boolean IsShort(void)
BEGIN
   if (AdrMode<0x30) return True;
   else if (AdrMode==0x30) return ((ImmVal()>=LowLim4) AND (ImmVal()<=7));
   else return False;
END

        static Boolean Is2Short(void)
BEGIN
   if (AdrMode2<0x30) return True;
   else if (AdrMode2==0x30) return ((ImmVal2()>=LowLim4) AND (ImmVal2()<=7));
   else return False;
END

        static void ConvertShort(void)
BEGIN
   if (AdrMode==0x30)
    BEGIN
     AdrMode+=ImmVal() & 15; AdrCnt=0;
    END
END

        static void Convert2Short(void)
BEGIN
   if (AdrMode2==0x30)
    BEGIN
     AdrMode2+=ImmVal2() & 15; AdrCnt2=0;
    END
END

        static void SetULowLims(void)
BEGIN
   LowLim4=0; LowLim8=0;
END

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        static void AddPrefixes(void)
BEGIN
   if (CodeLen!=0)
    BEGIN
     InsertSinglePrefix(1);
     InsertSinglePrefix(0);
    END
END

        static Boolean CodeAri(void)
BEGIN
   int z,Cnt;
   Byte Reg;

   for (z=0; z<GASI1OrderCount; z++)
    if (Memo(GASI1Orders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],1,False,True);
        if (AdrOK)
         BEGIN
          CopyAdr(); DecodeAdr(ArgStr[2],0,True,True);
          if (AdrOK)
           BEGIN
            if (OpSize==-1) WrError(1132);
            else
             BEGIN
              if (Format==' ')
               BEGIN
                if (((IsReg()) AND (Is2Short()))
                OR  ((Is2Reg()) AND (IsShort()))) Format='S';
                else if (((IsAbsolute()) AND (Is2Short()))
                     OR  ((Is2Absolute()) AND (IsShort()))) Format='A';
                else if ((IsImmediate()) AND (OpSize>0) AND ((ImmVal()>127) OR (ImmVal()<-128))) Format='I';
                else Format='G';
               END
              switch (Format)
               BEGIN
                case 'G':
                 WAsmCode[0]=0x700+(((Word)OpSize+1) << 14);
                 if ((IsImmediate()) AND (ImmVal()<=127) AND (ImmVal()>=-128))
                  BEGIN
                   AdrMode=ImmVal() & 0xff; AdrCnt=0;
                  END
                 else WAsmCode[0]+=0x800;
                 WAsmCode[0]+=AdrMode;
                 memcpy(WAsmCode+1,AdrVals,AdrCnt);
                 WAsmCode[1+(AdrCnt >> 1)]=0x8400+(z << 8)+AdrMode2;
                 memcpy(WAsmCode+2+(AdrCnt >> 1),AdrVals2,AdrCnt2);
                 CodeLen=4+AdrCnt+AdrCnt2;
                 break;
                case 'A':
                 if ((IsShort()) AND (Is2Absolute()))
                  BEGIN
                   ConvertShort();
                   WAsmCode[0]=0x3900+(((Word)OpSize+1) << 14)
                               +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15);
                   memcpy(WAsmCode+1,AdrVals,AdrCnt);
                   WAsmCode[1+(AdrCnt >> 1)]=(AdrVals2[0] & 0x1fff)+(z << 13);
                   CodeLen=4+AdrCnt;
                  END
                 else if ((Is2Short()) AND (IsAbsolute()))
                  BEGIN
                   Convert2Short();
                   WAsmCode[0]=0x3980+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   WAsmCode[1+(AdrCnt2 >> 1)]=(AdrVals[0] & 0x1fff)+(z << 13);
                   CodeLen=4+AdrCnt2;
                  END
                 else WrError(1350);
                 break;
                case 'S':
                 if ((IsShort()) AND (Is2Reg()))
                  BEGIN
                   ConvertShort();
                   WAsmCode[0]=0x0000+(((Word)OpSize+1) << 14)
                              +(AdrMode & 15)+(((Word)AdrMode & 0xf0) << 5)
                              +(((Word)AdrMode2 & 1) << 12)+((AdrMode2 & 14) << 4)
                              +((z & 1) << 4)+((z & 2) << 10);
                   memcpy(WAsmCode+1,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
                  END
                 else if ((Is2Short()) AND (IsReg()))
                  BEGIN
                   Convert2Short();
                   WAsmCode[0]=0x0100+(((Word)OpSize+1) << 14)
                              +(AdrMode2 & 15)+(((Word)AdrMode2 & 0xf0) << 5)
                              +(((Word)AdrMode & 1) << 12)+((AdrMode & 14) << 4)
                              +((z & 1) << 4)+((z & 2) << 11);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2); CodeLen=2+AdrCnt2;
                  END
                 else WrError(1350);
                 break;
                case 'I':
                 if ((NOT IsImmediate()) OR (OpSize==0)) WrError(1350);
                 else
                  BEGIN
                   WAsmCode[0]=AdrMode2+(((Word)OpSize-1) << 11)+(z << 8);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   memcpy(WAsmCode+1+(AdrCnt2 >> 1),AdrVals,AdrCnt);
                   CodeLen=2+AdrCnt+AdrCnt2;
                  END
                 break;
                default:
                 WrError(1090);
               END
             END
           END
         END
       END
      AddPrefixes(); return True;
     END

   for (z=0; z<GASI2OrderCount; z++)
    if (Memo(GASI2Orders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],1,False,True);
        if (AdrOK)
         BEGIN
          CopyAdr(); DecodeAdr(ArgStr[2],0,True,True);
          if (AdrOK)
           BEGIN
            if (OpSize==-1) WrError(1132);
            else
             BEGIN
              if (Format==' ')
               BEGIN
                if ((IsReg()) AND (Is2Reg())) Format='S';
                else if (((IsAbsolute()) AND  (Is2Short()))
                      OR ((Is2Absolute()) AND (IsShort()))) Format='A';
                else if ((IsImmediate()) AND (OpSize>0) AND ((ImmVal()>127) OR (ImmVal()<-128))) Format='I';
                else Format='G';
               END
              switch (Format)
               BEGIN
                case 'G':
                 WAsmCode[0]=0x700+(((Word)OpSize+1) << 14);
                 if ((IsImmediate()) AND (ImmVal()<=127) AND (ImmVal()>=-128))
                  BEGIN
                   AdrMode=ImmVal() & 0xff; AdrCnt=0;
                  END
                 else WAsmCode[0]+=0x800;
                 WAsmCode[0]+=AdrMode;
                 memcpy(WAsmCode+1,AdrVals,AdrCnt);
                 WAsmCode[1+(AdrCnt >> 1)]=0xc400+(z << 8)+AdrMode2;
                 memcpy(WAsmCode+2+(AdrCnt >> 1),AdrVals2,AdrCnt2);
                 CodeLen=4+AdrCnt+AdrCnt2;
                 break;
                case 'A':
                 if ((IsShort()) AND (Is2Absolute()))
                  BEGIN
                   ConvertShort();
                   WAsmCode[0]=0x3940+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15);
                   memcpy(WAsmCode+1,AdrVals,AdrCnt);
                   WAsmCode[1+(AdrCnt >> 1)]=(AdrVals2[0] & 0x1fff)+(z << 13);
                   CodeLen=4+AdrCnt;
                  END
                 else if ((Is2Short()) AND (IsAbsolute()))
                  BEGIN
                   Convert2Short();
                   WAsmCode[0]=0x39c0+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   WAsmCode[1+(AdrCnt2 >> 1)]=(AdrVals[0] & 0x1fff)+(z << 13);
                   CodeLen=4+AdrCnt2;
                  END
                 else WrError(1350);
                 break;
                case 'S':
                 if ((IsReg()) AND (Is2Reg()))
                  BEGIN
                   WAsmCode[0]=0x3800+(((Word)OpSize+1) << 14)
                              +(AdrMode & 15)+(AdrMode2 << 4)
                              +(z << 9);
                   CodeLen=2;
                  END
                 else WrError(1350);
                 break;
                case 'I':
                 if ((NOT IsImmediate()) OR (OpSize==0)) WrError(1350);
                 else
                  BEGIN
                   WAsmCode[0]=0x400+AdrMode2+(((Word)OpSize-1) << 11)+(z << 8);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   memcpy(WAsmCode+1+(AdrCnt2 >> 1),AdrVals,AdrCnt);
                   CodeLen=2+AdrCnt+AdrCnt2;
                  END
                 break;
                default:
                 WrError(1090);
               END
             END
           END
         END
       END
      AddPrefixes(); return True;
     END

   for (z=0; z<TrinomOrderCount; z++)
    if (Memo(TrinomOrders[z]))
     BEGIN
      if (Memo("MAC")) LowLim8=0;
      if (ArgCnt!=3) WrError(1110);
      else if (NOT DecodeRegAdr(ArgStr[1],&Reg)) WrError(1350);
      else
       BEGIN
        if (z>=2) OpSize--;
        if (OpSize<0) WrError(1130);
        else
         BEGIN
          DecodeAdr(ArgStr[3],0,True,True);
          if (AdrOK)
           BEGIN
            WAsmCode[0]=0x700;
            if ((IsImmediate()) AND (ImmVal()<127) AND (ImmVal()>LowLim8))
             BEGIN
              AdrMode=ImmVal() & 0xff; AdrCnt=0;
             END
            else WAsmCode[0]+=0x800;
            WAsmCode[0]+=(((Word)OpSize+1) << 14)+AdrMode;
            memcpy(WAsmCode+1,AdrVals,AdrCnt); Cnt=AdrCnt;
            DecodeAdr(ArgStr[2],1,False,True);
            if (AdrOK)
             BEGIN
              WAsmCode[1+(Cnt >> 1)]=AdrMode+(z << 8)+(((Word)Reg) << 11);
              memcpy(WAsmCode+2+(Cnt >> 1),AdrVals,AdrCnt);
              CodeLen=4+Cnt+AdrCnt;
             END
           END
         END
       END
      AddPrefixes(); return True;
     END

   if ((Memo("RLM")) OR (Memo("RRM")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (NOT DecodeReg(ArgStr[2],&Reg)) WrError(1350);
     else if ((Reg >> 6)!=1) WrError(1130);
     else
      BEGIN
       Reg&=0x3f;
       DecodeAdr(ArgStr[3],0,True,True);
       if (AdrOK)
        BEGIN
         WAsmCode[0]=0x700;
         if ((IsImmediate()) AND (ImmVal()<127) AND (ImmVal()>-128))
          BEGIN
           AdrMode=ImmVal() & 0xff; AdrCnt=0;
          END
         else WAsmCode[0]+=0x800;
         WAsmCode[0]+=AdrMode;
         memcpy(WAsmCode+1,AdrVals,AdrCnt); Cnt=AdrCnt;
         DecodeAdr(ArgStr[1],1,False,True);
         if (AdrOK)
          BEGIN
           if (OpSize==-1) WrError(1132);
           else
            BEGIN
             WAsmCode[0]+=((Word)OpSize+1) << 14;
             WAsmCode[1+(Cnt >> 1)]=0x400+(((Word)Reg) << 11)+AdrMode;
             if (Memo("RRM")) WAsmCode[1+(Cnt >> 1)]+=0x100;
             memcpy(WAsmCode+2+(Cnt >> 1),AdrVals,AdrCnt);
             CodeLen=4+AdrCnt+Cnt;
            END
          END
        END
      END
     AddPrefixes(); return True;
    END

   return False;
END

        static void MakeCode_97C241(void)
BEGIN
   int z,Cnt;
   Byte Reg,Num1,Num2;
   char *p;
   LongInt AdrInt,AdrLong;
   Boolean OK;

   CodeLen=0; DontPrint=False; PrefUsed[0]=False; PrefUsed[1]=False;
   AdrInc=0; MinOneIs0=False; LowLim4=(-8); LowLim8=(-128);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Formatangabe abspalten */

   switch (AttrSplit)
    BEGIN
     case '.':
      p=strchr(AttrPart,':');
      if (p!=0)
       BEGIN
        if (p<AttrPart+strlen(AttrPart)-1) Format=p[1];
        else Format=' ';
        *p='\0';
       END
      else Format=' ';
      break;
     case ':':
      p=strchr(AttrPart,'.');
      if (p==Nil)
       BEGIN
        Format=(*AttrPart); *AttrPart='\0';
       END
      else
       BEGIN
        if (p==AttrPart) Format=' '; else Format=(*AttrPart);
        strcpy(AttrPart,p+1);
       END
      break;
     default:
      Format=' ';
    END
   Format=toupper(Format);

   /* Attribut abarbeiten */

   if (*AttrPart=='\0') OpSize=(-1);
   else
    switch (toupper(*AttrPart))
     BEGIN
      case 'B': OpSize=0; break;
      case 'W': OpSize=1; break;
      case 'D': OpSize=2; break;
      default:
       WrError(1107); return;
     END

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        WAsmCode[0]=FixedOrders[z].Code;
        CodeLen=2;
       END
      return;
     END

   /* ein Operand */

   for (z=0; z<RMWOrderCount; z++)
    if (Memo(RMWOrders[z].Name))
     BEGIN
      if ((OpSize==-1) AND ((RMWOrders[z].Mask & 0x20)!=0)) OpSize=2;
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        if ((NOT IsIndirect(ArgStr[1])) AND ((RMWOrders[z].Mask & 0x20)!=0))
         BEGIN
          sprintf(ArgStr[2],"(%s)",ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]);
         END
        DecodeAdr(ArgStr[1],0,(RMWOrders[z].Mask & 0x10)==0,(RMWOrders[z].Mask & 0x20)==0);
        if (AdrOK)
         BEGIN
          if (OpSize==-1) WrError(1132);
          else if ((RMWOrders[z].Mask & (1 << OpSize))==0) WrError(1130);
          else
           BEGIN
            WAsmCode[0]=(((Word)OpSize+1) << 14)+(((Word)RMWOrders[z].Code) << 8)+AdrMode;
            memcpy(WAsmCode+1,AdrVals,AdrCnt);
            CodeLen=2+AdrCnt;
           END
         END
       END
      AddPrefixes(); return;
     END

   /* Arithmetik */

   if (CodeAri()) return;

   for (z=0; z<BitOrderCount; z++)
    if (Memo(BitOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],1,False,True);
        if (AdrOK)
         BEGIN
          if (OpSize==-1) WrError(1132);
          else
           BEGIN
            CopyAdr(); OpSize=(-1); MinOneIs0=True;
            DecodeAdr(ArgStr[2],0,True,True);
            if (AdrOK)
             BEGIN
              OpSize=OpSize2;
              if (Format==' ')
               BEGIN
                if ((Is2Reg()) AND (IsImmediate())
                AND (ImmVal()>0) AND (ImmVal()<(1 << (OpSize+3)))) Format='S';
                else if (((IsShort()) AND (Is2Absolute()))
                     OR  ((Is2Short()) AND (IsAbsolute()))) Format='A';
                else Format='G';
               END
              switch (Format)
               BEGIN
                case 'G':
                 WAsmCode[0]=0x700+(((Word)OpSize+1) << 14);
                 if ((IsImmediate()) AND (ImmVal()>=LowLim8) AND (ImmVal()<127))
                  BEGIN
                   AdrMode=ImmVal() & 0xff; AdrCnt=0;
                  END
                 else WAsmCode[0]+=0x800;
                 WAsmCode[0]+=AdrMode;
                 memcpy(WAsmCode+1,AdrVals,AdrCnt);
                 WAsmCode[1+(AdrCnt >> 1)]=0xd400+(z << 8)+AdrMode2;
                 memcpy(WAsmCode+2+(AdrCnt >> 1),AdrVals2,AdrCnt2);
                 CodeLen=4+AdrCnt+AdrCnt2;
                 break;
                case 'A':
                 if ((IsAbsolute()) AND (Is2Short()))
                  BEGIN
                   Convert2Short();
                   WAsmCode[0]=0x39d0+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   WAsmCode[1+(AdrCnt2 >> 1)]=(AdrVals[0] & 0x1fff)+(z << 13);
                   CodeLen=4+AdrCnt2;
                  END
                 else if ((Is2Absolute()) AND (IsShort()))
                  BEGIN
                   ConvertShort();
                   WAsmCode[0]=0x3950+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15);
                   memcpy(WAsmCode+1,AdrVals,AdrCnt);
                   WAsmCode[1+(AdrCnt >> 1)]=(AdrVals2[0] & 0x1fff)+(z << 13);
                   CodeLen=4+AdrCnt;
                  END
                 else WrError(1350);
                 break;
                case 'S':
                 if ((Is2Reg()) AND (IsImmediate()) AND (ImmVal()>=0) AND (ImmVal()<(1 << (3+OpSize))))
                  BEGIN
                   if (OpSize==2)
                    BEGIN
                     if (ImmVal()>=16)
                      BEGIN
                       AdrVals[0]-=16; AdrMode2++;
                      END
                     OpSize=1;
                    END
                   if (OpSize==1)
                    BEGIN
                     if (ImmVal()<8) OpSize=0;
                     else AdrVals[0]-=8;
                    END
                   WAsmCode[0]=0x1700+(((Word)OpSize+1) << 14)
                              +((z & 1) << 7)+((z & 2) << 10)
                              +(ImmVal() << 4)+AdrMode2;
                   CodeLen=2;
                  END
                 else WrError(1350);
                 break;
                default:
                 WrError(1090);
               END
             END
           END
         END
       END
      AddPrefixes(); return;
     END

   for (z=0; z<ShiftOrderCount; z++)
    if (Memo(ShiftOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],1,False,True);
        if (AdrOK)
         BEGIN
          if (OpSize==-1) WrError(1132);
          else
           BEGIN
            CopyAdr(); OpSize=(-1); MinOneIs0=True;
            DecodeAdr(ArgStr[2],0,True,True);
            if (AdrOK)
             BEGIN
              OpSize=OpSize2;
              if (Format==' ')
               BEGIN
                if ((IsImmediate()) AND (ImmVal()==1)) Format='S';
                else if (((IsShort()) AND (Is2Absolute()))
                     OR  ((Is2Short()) AND (IsAbsolute()))) Format='A';
                else Format='G';
               END
              switch (Format)
               BEGIN
                case 'G':
                 WAsmCode[0]=0x700+(((Word)OpSize+1) << 14);
                 if ((IsImmediate()) AND (ImmVal()>=LowLim8) AND (ImmVal()<127))
                  BEGIN
                   AdrMode=ImmVal() & 0xff; AdrCnt=0;
                  END
                 else WAsmCode[0]+=0x800;
                 WAsmCode[0]+=AdrMode;
                 memcpy(WAsmCode+1,AdrVals,AdrCnt);
                 WAsmCode[1+(AdrCnt >> 1)]=0xb400+((z & 3) << 8)+((z & 4) << 9)+AdrMode2;
                 memcpy(WAsmCode+2+(AdrCnt >> 1),AdrVals2,AdrCnt2);
                 CodeLen=4+AdrCnt+AdrCnt2;
                 break;
                case 'A':
                 if ((IsAbsolute()) AND (Is2Short()))
                  BEGIN
                   Convert2Short();
                   WAsmCode[0]=0x39b0+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   WAsmCode[1+(AdrCnt2 >> 1)]=(AdrVals[0] & 0x1fff)+(z << 13);
                   CodeLen=4+AdrCnt2;
                  END
                 else if ((Is2Absolute()) AND (IsShort()))
                  BEGIN
                   ConvertShort();
                   WAsmCode[0]=0x3930+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15);
                   memcpy(WAsmCode+1,AdrVals,AdrCnt);
                   WAsmCode[1+(AdrCnt >> 1)]=(AdrVals2[0] & 0x1fff)+(z << 13);
                   CodeLen=4+AdrCnt;
                  END
                 else WrError(1350);
                 break;
                case 'S':
                 if ((IsImmediate()) AND (ImmVal()==1))
                  BEGIN
                   WAsmCode[0]=0x2400+(((Word)OpSize+1) << 14)+AdrMode2
                              +((z & 3) << 8)+((z & 4) << 9);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   CodeLen=2+AdrCnt2;
                  END
                 else WrError(1350);
                 break;
                default:
                 WrError(1090);
               END
             END
           END
         END
       END
      AddPrefixes(); return;
     END

   for (z=0; z<BFieldOrderCount; z++)
    if (Memo(BFieldOrders[z]))
     BEGIN
      if (ArgCnt!=4) WrError(1110);
      else
       BEGIN
        if (z==2)
         BEGIN
          strcpy(ArgStr[5],ArgStr[1]);
          strcpy(ArgStr[1],ArgStr[2]);
          strcpy(ArgStr[2],ArgStr[5]);
         END
        if (NOT DecodeReg(ArgStr[1],&Reg)) WrError(1350);
        else if ((Reg >> 6)!=1) WrError(1130);
        else
         BEGIN
          Reg&=0x3f;
          Num2=EvalIntExpression(ArgStr[4],Int5,&OK);
          if (OK)
           BEGIN
            if (FirstPassUnknown) Num2&=15; Num2--;
            if (Num2>15) WrError(1320);
            else if ((OpSize==-1) AND (NOT DecodeRegAdr(ArgStr[2],&Num1))) WrError(1132);
            else
             BEGIN
              switch (OpSize)
               BEGIN
                case 0: Num1=EvalIntExpression(ArgStr[3],UInt3,&OK) & 7; break;
                case 1: Num1=EvalIntExpression(ArgStr[3],Int4,&OK) & 15; break;
                case 2: Num1=EvalIntExpression(ArgStr[3],Int5,&OK) & 31; break;
               END
              if (OK)
               BEGIN
                if ((OpSize==2) AND (Num1>15)) AdrInc=2;
                DecodeAdr(ArgStr[2],1,False,True);
                if (AdrOK)
                 BEGIN
                  if ((OpSize==2) AND (Num1>15))
                   BEGIN
                    Num1-=16; OpSize--;
                    if ((AdrMode & 0xf0)==0) AdrMode++;
                   END
                  WAsmCode[0]=0x7000+(((Word)OpSize+1) << 8)+AdrMode;
                  memcpy(WAsmCode+1,AdrVals,AdrCnt);
                  WAsmCode[1+(AdrCnt >> 1)]=(((Word)Reg) << 11)
                                           +Num2+(((Word)Num1) << 5)
                                           +((z & 1) << 10)
                                           +((z & 2) << 14);
                  CodeLen=4+AdrCnt;
                 END
               END
             END
           END
         END
       END
      AddPrefixes(); return;
     END

   for (z=0; z<GAEqOrderCount; z++)
    if (Memo(GAEqOrders[z].Name))
     BEGIN
      if ((Memo("ABCD")) OR (Memo("SBCD")) OR (Memo("CBCD"))
       OR (Memo("MAX")) OR (Memo("MIN")) OR (Memo("SBC"))) SetULowLims();
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],1,False,True);
        if (AdrOK)
         BEGIN
          CopyAdr();
          DecodeAdr(ArgStr[2],0,True,True);
          if (AdrOK)
           BEGIN
            if (OpSize==-1) WrError(1132);
            else
             BEGIN
              if (OpSize==0) LowLim8=(-128);
              if (Format==' ')
               BEGIN
                if (((Is2Absolute()) AND (IsShort()))
                 OR ((Is2Short()) AND (IsAbsolute()))) Format='A';
                else Format='G';
               END
              switch (Format)
               BEGIN
                case 'G':
                 WAsmCode[0]=0x700+(((Word)OpSize+1) << 14);
                 if ((IsImmediate()) AND (ImmVal()<127) AND (ImmVal()>LowLim8))
                  BEGIN
                   AdrMode=ImmVal() & 0xff; AdrCnt=0;
                  END
                 else WAsmCode[0]+=0x800;
                 WAsmCode[0]+=AdrMode;
                 memcpy(WAsmCode+1,AdrVals,AdrCnt);
                 WAsmCode[1+(AdrCnt >> 1)]=0x8400+AdrMode2
                                          +(((Word)GAEqOrders[z].Code & 0xf0) << 8)
                                          +(((Word)GAEqOrders[z].Code & 4) << 9)
                                          +(((Word)GAEqOrders[z].Code & 3) << 8);
                 memcpy(WAsmCode+2+(AdrCnt >> 1),AdrVals2,AdrCnt2);
                 CodeLen=4+AdrCnt+AdrCnt2;
                 break;
                case 'A':
                 if ((IsAbsolute()) AND (Is2Short()))
                  BEGIN
                   Convert2Short();
                   WAsmCode[0]=0x3980+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15)
                              +(GAEqOrders[z].Code & 0xf0);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   WAsmCode[1+(AdrCnt2 >> 1)]=(AdrVals[0] & 0x1fff)
                                             +(((Word)GAEqOrders[z].Code & 15) << 13);
                   CodeLen=4+AdrCnt2;
                  END
                 else if ((Is2Absolute()) AND (IsShort()))
                  BEGIN
                   ConvertShort();
                   WAsmCode[0]=0x3900+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15)
                              +(GAEqOrders[z].Code & 0xf0);
                   memcpy(WAsmCode+1,AdrVals,AdrCnt);
                   WAsmCode[1+(AdrCnt >> 1)]=(AdrVals2[0] & 0x1fff)
                                            +(((Word)GAEqOrders[z].Code & 15) << 13);
                   CodeLen=4+AdrCnt;
                  END
                 else WrError(1350);
                 break;
                default:
                 WrError(1090);
               END
             END
           END
         END
       END
      AddPrefixes(); return;
     END

   for (z=0; z<GAHalfOrderCount; z++)
    if (Memo(GAHalfOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],1,False,True);
        if (AdrOK)
         BEGIN
          if (OpSize==0) WrError(1130);
          else
           BEGIN
            if (OpSize!=-1) OpSize--; CopyAdr();
            DecodeAdr(ArgStr[2],0,True,True);
            if (AdrOK)
             BEGIN
              if (OpSize==2) WrError(1130);
              else if (OpSize==-1) WrError(1132);
              else
               BEGIN
                if (Format==' ')
                 BEGIN
                  if (((Is2Absolute()) AND (IsShort()))
                   OR ((Is2Short()) AND (IsAbsolute()))) Format='A';
                  else Format='G';
                 END
                switch (Format)
                 BEGIN
                  case 'G':
                   WAsmCode[0]=0x700+(((Word)OpSize+1) << 14);
                   if ((IsImmediate()) AND (ImmVal()<127) AND (ImmVal()>LowLim8))
                    BEGIN
                     AdrMode=ImmVal() & 0xff; AdrCnt=0;
                    END
                   else WAsmCode[0]+=0x800;
                   WAsmCode[0]+=AdrMode;
                   memcpy(WAsmCode+1,AdrVals,AdrCnt);
                   WAsmCode[1+(AdrCnt >> 1)]=0x8400+AdrMode2
                                            +(((Word)GAHalfOrders[z].Code & 0xf0) << 8)
                                            +(((Word)GAHalfOrders[z].Code & 4) << 9)
                                            +(((Word)GAHalfOrders[z].Code & 3) << 8);
                   memcpy(WAsmCode+2+(AdrCnt >> 1),AdrVals2,AdrCnt2);
                   CodeLen=4+AdrCnt+AdrCnt2;
                   break;
                  case 'A':
                   if ((IsAbsolute()) AND (Is2Short()))
                    BEGIN
                     Convert2Short();
                     WAsmCode[0]=0x3980+(((Word)OpSize+1) << 14)
                                +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15)
                                +(GAHalfOrders[z].Code & 0xf0);
                     memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                     WAsmCode[1+(AdrCnt2 >> 1)]=(AdrVals[0] & 0x1fff)
                                               +(((Word)GAHalfOrders[z].Code & 15) << 13);
                     CodeLen=4+AdrCnt2;
                    END
                   else if ((Is2Absolute()) AND (IsShort()))
                    BEGIN
                     ConvertShort();
                     WAsmCode[0]=0x3900+(((Word)OpSize+1) << 14)
                                +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15)
                                +(GAHalfOrders[z].Code & 0xf0);
                     memcpy(WAsmCode+1,AdrVals,AdrCnt);
                     WAsmCode[1+(AdrCnt >> 1)]=(AdrVals2[0] & 0x1fff)
                                              +(((Word)GAHalfOrders[z].Code & 15) << 13);
                     CodeLen=4+AdrCnt;
                    END
                   else WrError(1350);
                   break;
                  default:
                   WrError(1090);
                 END
               END
             END
           END
         END
       END
      AddPrefixes(); return;
     END

   for (z=0; z<GAFirstOrderCount; z++)
    if (Memo(GAFirstOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],1,NOT(Memo("STCF") OR Memo("TSET")),True);
        if (AdrOK)
         BEGIN
          if (OpSize==-1) WrError(1132);
          else
           BEGIN
            CopyAdr(); OpSize=(-1); MinOneIs0=True;
            DecodeAdr(ArgStr[2],0,True,True);
            OpSize=OpSize2;
            if (AdrOK)
             BEGIN
              if (Format==' ')
               BEGIN
                if (((Is2Absolute()) AND (IsShort()))
                 OR ((Is2Short()) AND (IsAbsolute()))) Format='A';
                else Format='G';
               END
              switch (Format)
               BEGIN
                case 'G':
                 WAsmCode[0]=0x700+(((Word)OpSize+1) << 14);
                 if ((IsImmediate()) AND (ImmVal()<127) AND (ImmVal()>LowLim8))
                  BEGIN
                   AdrMode=ImmVal() & 0xff; AdrCnt=0;
                  END
                 else WAsmCode[0]+=0x800;
                 WAsmCode[0]+=AdrMode;
                 memcpy(WAsmCode+1,AdrVals,AdrCnt);
                 WAsmCode[1+(AdrCnt >> 1)]=0x8400+AdrMode2
                                          +(((Word)GAFirstOrders[z].Code & 0xf0) << 8)
                                          +(((Word)GAFirstOrders[z].Code & 4) << 9)
                                          +(((Word)GAFirstOrders[z].Code & 3) << 8);
                 memcpy(WAsmCode+2+(AdrCnt >> 1),AdrVals2,AdrCnt2);
                 CodeLen=4+AdrCnt+AdrCnt2;
                 break;
                case 'A':
                 if ((IsAbsolute()) AND (Is2Short()))
                  BEGIN
                   Convert2Short();
                   WAsmCode[0]=0x3980+(((Word)OpSize+1) << 14)
                                     +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15)
                                     +(GAFirstOrders[z].Code & 0xf0);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   WAsmCode[1+(AdrCnt2 >> 1)]=(AdrVals[0] & 0x1fff)
                                             +(((Word)GAFirstOrders[z].Code & 15) << 13);
                   CodeLen=4+AdrCnt2;
                  END
                 else if ((Is2Absolute()) AND (IsShort()))
                  BEGIN
                   ConvertShort();
                   WAsmCode[0]=0x3900+(((Word)OpSize+1) << 14)
                                     +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15)
                                     +(GAFirstOrders[z].Code & 0xf0);
                   memcpy(WAsmCode+1,AdrVals,AdrCnt);
                   WAsmCode[1+(AdrCnt >> 1)]=(AdrVals2[0] & 0x1fff)
                                            +(((Word)GAFirstOrders[z].Code & 15) << 13);
                   CodeLen=4+AdrCnt;
                  END
                 else WrError(1350);
                 break;
                default:
                 WrError(1090);
               END
             END
           END
         END
       END
      AddPrefixes(); return;
     END

   for (z=0; z<GASecondOrderCount; z++)
    if (Memo(GASecondOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[2],0,True,True);
        if (AdrOK)
         BEGIN
          if (OpSize==-1) WrError(1132);
          else
           BEGIN
            CopyAdr(); OpSize=(-1);
            DecodeAdr(ArgStr[1],1,False,True);
            OpSize=OpSize2;
            if (AdrOK)
             BEGIN
              if (Format==' ')
               BEGIN
                if (((Is2Absolute()) AND (IsShort()))
                 OR ((Is2Short()) AND (IsAbsolute()))) Format='A';
                else Format='G';
               END
              switch (Format)
               BEGIN
                case 'G':
                 WAsmCode[0]=0x700+(((Word)OpSize+1) << 14);
                 if ((Is2Immediate()) AND (ImmVal2()<127) AND (ImmVal2()>LowLim8))
                  BEGIN
                   AdrMode2=ImmVal2() & 0xff; AdrCnt=0;
                  END
                 else WAsmCode[0]+=0x800;
                 WAsmCode[0]+=AdrMode2;
                 memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                 WAsmCode[1+(AdrCnt2 >> 1)]=0x8400+AdrMode
                                           +(((Word)GASecondOrders[z].Code & 0xf0) << 8)
                                           +(((Word)GASecondOrders[z].Code & 4) << 9)
                                           +(((Word)GASecondOrders[z].Code & 3) << 8);
                 memcpy(WAsmCode+2+(AdrCnt2 >> 1),AdrVals,AdrCnt);
                 CodeLen=4+AdrCnt+AdrCnt2;
                 break;
                case 'A':
                 if ((IsAbsolute()) AND (Is2Short()))
                  BEGIN
                   Convert2Short();
                   WAsmCode[0]=0x3900+(((Word)OpSize+1) << 14)
                              +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15)
                              +(GASecondOrders[z].Code & 0xf0);
                   memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                   WAsmCode[1+(AdrCnt2 >> 1)]=(AdrVals[0] & 0x1fff)
                                             +(((Word)GASecondOrders[z].Code & 15) << 13);
                   CodeLen=4+AdrCnt2;
                  END
                 else if ((Is2Absolute()) AND (IsShort()))
                  BEGIN
                   ConvertShort();
                   WAsmCode[0]=0x3980+(((Word)OpSize+1) << 14)
                                     +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15)
                                     +(GASecondOrders[z].Code & 0xf0);
                   memcpy(WAsmCode+1,AdrVals,AdrCnt);
                   WAsmCode[1+(AdrCnt >> 1)]=(AdrVals2[0] & 0x1fff)
                                            +(((Word)GASecondOrders[z].Code & 15) << 13);
                   CodeLen=4+AdrCnt;
                  END
                 else WrError(1350);
                 break;
                default:
                 WrError(1090);
               END
             END
           END
         END
       END
      AddPrefixes(); return;
     END

   if ((Memo("CHK")) OR (Memo("CHKS")))
    BEGIN
     if (Memo("CHK")) SetULowLims();
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],1,False,True);
       if (AdrOK)
        BEGIN
         if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
         else if (OpSize==-1) WrError(1132);
         else
          BEGIN
           CopyAdr();
           DecodeAdr(ArgStr[1],0,False,False);
           if (AdrOK)
            BEGIN
             if (OpSize==0) LowLim8=(-128);
             if (Format==' ')
              BEGIN
               if (((Is2Absolute()) AND (IsShort()))
               OR  ((Is2Short()) AND (IsAbsolute()))) Format='A';
               else Format='G';
              END
             switch (Format)
              BEGIN
               case 'G':
                WAsmCode[0]=0xf00+(((Word)OpSize+1) << 14)+AdrMode2;
                memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                WAsmCode[1+(AdrCnt2 >> 1)]=0xa600+AdrMode
                                          +(((Word)Memo("CHKS")) << 8);
                memcpy(WAsmCode+2+(AdrCnt2 >> 1),AdrVals,AdrCnt);
                CodeLen=4+AdrCnt+AdrCnt2;
                break;
               case 'A':
                if ((IsAbsolute()) AND (Is2Short()))
                 BEGIN
                  Convert2Short();
                  WAsmCode[0]=0x3920+(((Word)OpSize+1) << 14)
                             +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15);
                  memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                  WAsmCode[1+(AdrCnt2 >> 1)]=0x4000+(AdrVals[0] & 0x1fff)
                                            +(((Word)Memo("CHKS")) << 13);
                  CodeLen=4+AdrCnt2;
                 END
                else if ((Is2Absolute()) AND (IsShort()))
                 BEGIN
                  ConvertShort();
                  WAsmCode[0]=0x39a0+(((Word)OpSize+1) << 14)
                             +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15);
                  memcpy(WAsmCode+1,AdrVals,AdrCnt);
                  WAsmCode[1+(AdrCnt >> 1)]=0x4000+(AdrVals2[0] & 0x1fff)
                                           +(((Word)Memo("CHKS")) << 13);
                  CodeLen=4+AdrCnt;
                 END
                else WrError(1350);
                break;
               default:
                WrError(1090);
              END
            END
          END
        END
      END
     AddPrefixes(); return;
    END

   /* Datentransfer */

   for (z=0; z<StringOrderCount; z++)
    if (Memo(StringOrders[z]))
     BEGIN
      if (ArgCnt!=3) WrError(1110);
      else if (NOT DecodeReg(ArgStr[3],&Reg)) WrError(1350);
      else if ((Reg >> 6)!=1) WrError(1130);
      else
       BEGIN
        Reg&=0x3f;
        DecodeAdr(ArgStr[2],0,True,True);
        if (AdrOK)
         BEGIN
          WAsmCode[0]=0x700;
          if ((IsImmediate()) AND (ImmVal()<127) AND (ImmVal()>LowLim8))
           BEGIN
            AdrMode=ImmVal() & 0xff; AdrCnt=0;
           END
          else WAsmCode[0]+=0x800;
          WAsmCode[0]+=AdrMode;
          memcpy(WAsmCode+1,AdrVals,AdrCnt); Cnt=AdrCnt;
          DecodeAdr(ArgStr[1],1,True,True);
          if (AdrOK)
           BEGIN
            if (OpSize==-1) WrError(1132);
            else
             BEGIN
              WAsmCode[0]+=((Word)OpSize+1) << 14;
              WAsmCode[1+(Cnt >> 1)]=0x8000+AdrMode+(z << 8)+(((Word)Reg) << 11);
              memcpy(WAsmCode+2+(Cnt >> 1),AdrVals,AdrCnt);
              CodeLen=4+AdrCnt+Cnt;
             END
           END
         END
       END
      AddPrefixes(); return;
     END;

   if (Memo("EX"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],1,False,True);
       if (AdrOK)
        BEGIN
         CopyAdr(); DecodeAdr(ArgStr[2],0,False,True);
         if (AdrOK)
          BEGIN
           if (OpSize==-1) WrError(1132);
           else
            BEGIN
             if (Format==' ')
              BEGIN
               if ((IsReg()) AND (Is2Reg())) Format='S';
               else if (((IsShort()) AND (Is2Absolute()))
                    OR ((Is2Short()) AND (IsAbsolute()))) Format='A';
               else Format='G';
              END
             switch (Format)
              BEGIN
               case 'G':
                WAsmCode[0]=0x0f00+(((Word)OpSize+1) << 14)+AdrMode;
                memcpy(WAsmCode+1,AdrVals,AdrCnt);
                WAsmCode[1+(AdrCnt >> 1)]=0x8f00+AdrMode2;
                memcpy(WAsmCode+2+(AdrCnt >> 1),AdrVals2,AdrCnt2);
                CodeLen=4+AdrCnt+AdrCnt2;
                break;
               case 'A':
                if ((IsAbsolute()) AND (Is2Short()))
                 BEGIN
                  Convert2Short();
                  WAsmCode[0]=0x3980+(((Word)OpSize+1) << 14)
                             +(((Word)AdrMode2 & 0xf0) << 5)+(AdrMode2 & 15);
                  memcpy(WAsmCode+1,AdrVals2,AdrCnt2);
                  WAsmCode[1+(AdrCnt2 >> 1)]=AdrVals[0];
                  CodeLen=4+AdrCnt2;
                 END
                else if ((Is2Absolute()) AND (IsShort()))
                 BEGIN
                  ConvertShort();
                  WAsmCode[0]=0x3900+(((Word)OpSize+1) << 14)
                             +(((Word)AdrMode & 0xf0) << 5)+(AdrMode & 15);
                  memcpy(WAsmCode+1,AdrVals,AdrCnt);
                  WAsmCode[1+(AdrCnt >> 1)]=AdrVals2[0];
                  CodeLen=4+AdrCnt;
                 END
                else WrError(1350);
                break;
               case 'S':
                if ((IsReg()) AND (Is2Reg()))
                 BEGIN
                  WAsmCode[0]=0x3e00+(((Word)OpSize+1) << 14)+(AdrMode2 << 4)+AdrMode;
                  CodeLen=2;
                 END
                else WrError(1350);
                break;
               default:
                WrError(1090);
              END
            END
          END
        END
      END
     AddPrefixes(); return;
    END

   /* Spruenge */

   if ((Memo("CALR")) OR (Memo("JR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],Int32,&OK)-EProgCounter();
       if (OK)
        if (AddRelPrefix(0,13,&AdrInt))
         BEGIN
          if (Odd(AdrInt)) WrError(1375);
          else
           BEGIN
            WAsmCode[0]=0x2000+(AdrInt & 0x1ffe)+Ord(Memo("CALR"));
            CodeLen=2;
           END
         END
      END
     AddPrefixes(); return;
    END

   if (Memo("JRC"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       NLS_UpString(ArgStr[1]);
       for (z=0; z<ConditionCount; z++)
        if (strcmp(ArgStr[1],Conditions[z])==0) break;
       if (z>=ConditionCount) WrError(1360);
       else
        BEGIN
         z%=16;
         AdrInt=EvalIntExpression(ArgStr[2],Int32,&OK)-EProgCounter();
         if (OK)
          if (AddRelPrefix(0,9,&AdrInt))
           BEGIN
            if (Odd(AdrInt)) WrError(1375);
            else
             BEGIN
              WAsmCode[0]=0x1000+((z & 14) << 8)+(AdrInt & 0x1fe)+(z & 1);
              CodeLen=2;
             END
           END
        END
      END
     AddPrefixes(); return;
    END

   if ((Memo("JRBC")) OR (Memo("JRBS")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       z=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
        BEGIN
         FirstPassUnknown=False;
         AdrLong=EvalIntExpression(ArgStr[2],Int24,&OK);
         if (OK)
          BEGIN
           AddAbsPrefix(1,13,AdrLong);
           AdrInt=EvalIntExpression(ArgStr[3],Int32,&OK)-EProgCounter();
           if (OK)
            if (AddRelPrefix(0,9,&AdrInt))
             BEGIN
              if (Odd(AdrInt)) WrError(1375);
              else
               BEGIN
                CodeLen=4;
                WAsmCode[1]=(z << 13)+(AdrLong & 0x1fff);
                WAsmCode[0]=0x1e00+(AdrInt & 0x1fe)+Ord(Memo("JRBS"));
               END
             END
          END
        END
      END
     AddPrefixes(); return;
    END

   if (Memo("DJNZ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],0,False,True);
       if (AdrOK)
        BEGIN
         if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
         else
          BEGIN
           AdrInt=EvalIntExpression(ArgStr[2],Int32,&OK)-(EProgCounter()+4+AdrCnt+2*Ord(PrefUsed[0]));
           if (OK)
            if (AddRelPrefix(1,13,&AdrInt))
             BEGIN
              if (Odd(AdrInt)) WrError(1375);
              else
               BEGIN
                WAsmCode[0]=0x3700+(((Word)OpSize+1) << 14)+AdrMode;
                memcpy(WAsmCode+1,AdrVals,AdrCnt);
                WAsmCode[1+(AdrCnt >> 1)]=0xe000+(AdrInt & 0x1ffe);
                CodeLen=4+AdrCnt;
               END
             END
          END
        END
      END
     AddPrefixes(); return;
    END

   if (Memo("DJNZC"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       NLS_UpString(ArgStr[2]);
       for (z=0; z<ConditionCount; z++)
        if (strcmp(ArgStr[2],Conditions[z])==0) break;
       if (z>=ConditionCount) WrError(1360);
       else
        BEGIN
         z%=16;
         DecodeAdr(ArgStr[1],0,False,True);
         if (AdrOK)
          BEGIN
           if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
           else
            BEGIN
             AdrInt=EvalIntExpression(ArgStr[3],Int32,&OK)-EProgCounter();
             if (OK)
              if (AddRelPrefix(1,13,&AdrInt))
               BEGIN
                if (Odd(AdrInt)) WrError(1375);
                else
                 BEGIN
                  WAsmCode[0]=0x3700+(((Word)OpSize+1) << 14)+AdrMode;
                  memcpy(WAsmCode+1,AdrVals,AdrCnt);
                  WAsmCode[1+(AdrCnt >> 1)]=((z & 14) << 12)+(AdrInt & 0x1ffe)+(z & 1);
                  CodeLen=4+AdrCnt;
                 END
               END
            END
          END
        END
      END
     AddPrefixes(); return;
    END

   /* vermischtes... */

   if ((Memo("LINK")) OR (Memo("RETD")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrInt=EvalIntExpression(ArgStr[1],Int32,&OK);
       if (FirstPassUnknown) AdrInt&=0x1fe;
       if (AdrInt>0x7ffff) WrError(1320);
       else if (AdrInt<-0x80000) WrError(1315);
       else if (Odd(AdrInt)) WrError(1325);
       else
        BEGIN
         WAsmCode[0]=0xc001+(AdrInt & 0x1fe);
         if (Memo("RETD")) WAsmCode[0]+=0x800;
         AddSignedPrefix(0,9,AdrInt);
         CodeLen=2;
        END
      END
     AddPrefixes(); return;
    END

   if (Memo("SWI"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       WAsmCode[0]=EvalIntExpression(ArgStr[1],Int4,&OK)+0x7f90;
       if (OK) CodeLen=2;
      END
     return;
    END

   if (Memo("LDA"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2],0,False,False);
       if (AdrOK)
        BEGIN
         WAsmCode[0]=0x3000+AdrMode;
         z=AdrCnt; memcpy(WAsmCode+1,AdrVals,z);
         DecodeAdr(ArgStr[1],1,False,True);
         if (AdrOK)
          BEGIN
           if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
           else
            BEGIN
             WAsmCode[0]+=((Word)OpSize) << 14;
             WAsmCode[1+(z >> 1)]=0x9700+AdrMode;
             memcpy(WAsmCode+2+(z >> 1),AdrVals,AdrCnt);
             CodeLen=4+z+AdrCnt;
            END
          END
        END
      END
     AddPrefixes(); return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_97C241(void)
BEGIN
   return False;
END

        static void SwitchFrom_97C241(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_97C241(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x56; NOPCode=0x7fa0;
   DivideChars=","; HasAttrs=True; AttrChars=".:";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffffffl;

   MakeCode=MakeCode_97C241; IsDef=IsDef_97C241;
   SwitchFrom=SwitchFrom_97C241; InitFields();
END

        void code97c241_init(void)
BEGIN
   CPU97C241=AddCPU("97C241",SwitchTo_97C241);
END
