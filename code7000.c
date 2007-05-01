/* code7000.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator SH7x00                                                      */
/*                                                                           */
/* Historie: 25.12.1996 Grundsteinlegung                                     */
/*           12. 4.1998 SH7700-Erweiterungen                                 */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code7000.c,v 1.5 2006/12/19 17:50:18 alfred Exp $                    */
/*****************************************************************************
 * $Log: code7000.c,v $
 * Revision 1.5  2006/12/19 17:50:18  alfred
 * - eliminate static local variable
 *
 * Revision 1.4  2005/10/02 10:00:45  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2005/09/08 17:31:04  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 12:04:47  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
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
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "codevars.h"

#include "code7000.h"

#define FixedOrderCount 13
#define OneRegOrderCount 22
#define TwoRegOrderCount 20
#define MulRegOrderCount 3
#define BWOrderCount 3
#define LogOrderCount 4
#define SRegCnt 9


#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModIReg 1
#define MModIReg (1 << ModIReg)
#define ModPreDec 2
#define MModPreDec (1 << ModPreDec)
#define ModPostInc 3
#define MModPostInc (1 << ModPostInc)
#define ModIndReg 4
#define MModIndReg (1 << ModIndReg)
#define ModR0Base 5
#define MModR0Base (1 << ModR0Base)
#define ModGBRBase 6
#define MModGBRBase (1 << ModGBRBase)
#define ModGBRR0 7
#define MModGBRR0 (1 << ModGBRR0)
#define ModPCRel 8
#define MModPCRel (1 << ModPCRel)
#define ModImm 9
#define MModImm (1 << ModImm)


#define CompLiteralsName "COMPRESSEDLITERALS"

#define DSPAvailName "HASDSP"

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Boolean Priv;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Boolean Priv;
          Word Code;
          ShortInt DefSize;
         } TwoRegOrder;

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Word Code;
         } FixedMinOrder;

typedef struct
        {
          char *Name;
          Word Code;
          CPUVar MinCPU;
          Boolean NeedsDSP;
        } TRegDef;

typedef struct _TLiteral
         {
          struct _TLiteral *Next;
          LongInt Value,FCount;
          Boolean Is32,IsForward;
          Integer PassNo;
          LongInt DefSection;
         } *PLiteral,TLiteral;

static ShortInt OpSize;     /* Groesse=8*(2^OpSize) */
static ShortInt AdrMode;    /* Ergebnisadressmodus */
static Word AdrPart;        /* Adressierungsmodusbits im Opcode */

static PLiteral FirstLiteral;
static LongInt ForwardCount;
static SimpProc SaveInitProc;

static CPUVar CPU7000,CPU7600,CPU7700;

static FixedOrder *FixedOrders;
static FixedMinOrder *OneRegOrders;
static TwoRegOrder *TwoRegOrders;
static FixedMinOrder *MulRegOrders;
static FixedOrder *BWOrders;
static TRegDef *RegDefs;
static char **LogOrders;

static Boolean CurrDelayed, PrevDelayed, CompLiterals, DSPAvail;
static LongInt DelayedAdr;

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

        static void AddFixed(char *NName, Word NCode, Boolean NPriv, CPUVar NMin)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Priv=NPriv;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddOneReg(char *NName, Word NCode, CPUVar NMin)
BEGIN
   if (InstrZ>=OneRegOrderCount) exit(255);
   OneRegOrders[InstrZ].Name=NName;
   OneRegOrders[InstrZ].Code=NCode;
   OneRegOrders[InstrZ++].MinCPU=NMin;
END

        static void AddTwoReg(char *NName, Word NCode, Boolean NPriv, CPUVar NMin, ShortInt NDef)
BEGIN
   if (InstrZ>=TwoRegOrderCount) exit(255);
   TwoRegOrders[InstrZ].Name=NName;
   TwoRegOrders[InstrZ].Priv=NPriv;
   TwoRegOrders[InstrZ].DefSize=NDef;
   TwoRegOrders[InstrZ].MinCPU=NMin;
   TwoRegOrders[InstrZ++].Code=NCode;
END

        static void AddMulReg(char *NName, Word NCode, CPUVar NMin)
BEGIN
   if (InstrZ>=MulRegOrderCount) exit(255);
   MulRegOrders[InstrZ].Name=NName;
   MulRegOrders[InstrZ].Code=NCode;
   MulRegOrders[InstrZ++].MinCPU=NMin;
END

        static void AddBW(char *NName, Word NCode)
BEGIN
   if (InstrZ>=BWOrderCount) exit(255);
   BWOrders[InstrZ].Name=NName;
   BWOrders[InstrZ++].Code=NCode;
END

static void AddSReg(char *NName, Word NCode, CPUVar NMin, Boolean NDSP)
{
  if (InstrZ >= SRegCnt) exit(255);
  RegDefs[InstrZ].Name = NName;
  RegDefs[InstrZ].Code = NCode;
  RegDefs[InstrZ].MinCPU = NMin;
  RegDefs[InstrZ++].NeedsDSP = NDSP;
}

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("CLRT"  ,0x0008,False,CPU7000);
   AddFixed("CLRMAC",0x0028,False,CPU7000);
   AddFixed("NOP"   ,0x0009,False,CPU7000);
   AddFixed("RTE"   ,0x002b,False,CPU7000);
   AddFixed("SETT"  ,0x0018,False,CPU7000);
   AddFixed("SLEEP" ,0x001b,False,CPU7000);
   AddFixed("RTS"   ,0x000b,False,CPU7000);
   AddFixed("DIV0U" ,0x0019,False,CPU7000);
   AddFixed("BRK"   ,0x0000,True ,CPU7000);
   AddFixed("RTB"   ,0x0001,True ,CPU7000);
   AddFixed("CLRS"  ,0x0048,False,CPU7700);
   AddFixed("SETS"  ,0x0058,False,CPU7700);
   AddFixed("LDTLB" ,0x0038,True ,CPU7700);

   OneRegOrders=(FixedMinOrder *) malloc(sizeof(FixedMinOrder)*OneRegOrderCount); InstrZ=0;
   AddOneReg("MOVT"  ,0x0029,CPU7000); AddOneReg("CMP/PZ",0x4011,CPU7000);
   AddOneReg("CMP/PL",0x4015,CPU7000); AddOneReg("ROTL"  ,0x4004,CPU7000);
   AddOneReg("ROTR"  ,0x4005,CPU7000); AddOneReg("ROTCL" ,0x4024,CPU7000);
   AddOneReg("ROTCR" ,0x4025,CPU7000); AddOneReg("SHAL"  ,0x4020,CPU7000);
   AddOneReg("SHAR"  ,0x4021,CPU7000); AddOneReg("SHLL"  ,0x4000,CPU7000);
   AddOneReg("SHLR"  ,0x4001,CPU7000); AddOneReg("SHLL2" ,0x4008,CPU7000);
   AddOneReg("SHLR2" ,0x4009,CPU7000); AddOneReg("SHLL8" ,0x4018,CPU7000);
   AddOneReg("SHLR8" ,0x4019,CPU7000); AddOneReg("SHLL16",0x4028,CPU7000);
   AddOneReg("SHLR16",0x4029,CPU7000); AddOneReg("LDBR"  ,0x0021,CPU7000);
   AddOneReg("STBR"  ,0x0020,CPU7000); AddOneReg("DT"    ,0x4010,CPU7600);
   AddOneReg("BRAF"  ,0x0032,CPU7600); AddOneReg("BSRF"  ,0x0003,CPU7600);

   TwoRegOrders=(TwoRegOrder *) malloc(sizeof(TwoRegOrder)*TwoRegOrderCount); InstrZ=0;
   AddTwoReg("XTRCT" ,0x200d,False,CPU7000,2);
   AddTwoReg("ADDC"  ,0x300e,False,CPU7000,2);
   AddTwoReg("ADDV"  ,0x300f,False,CPU7000,2);
   AddTwoReg("CMP/HS",0x3002,False,CPU7000,2);
   AddTwoReg("CMP/GE",0x3003,False,CPU7000,2);
   AddTwoReg("CMP/HI",0x3006,False,CPU7000,2);
   AddTwoReg("CMP/GT",0x3007,False,CPU7000,2);
   AddTwoReg("CMP/STR",0x200c,False,CPU7000,2);
   AddTwoReg("DIV1"  ,0x3004,False,CPU7000,2);
   AddTwoReg("DIV0S" ,0x2007,False,CPU7000,-1);
   AddTwoReg("MULS"  ,0x200f,False,CPU7000,1);
   AddTwoReg("MULU"  ,0x200e,False,CPU7000,1);
   AddTwoReg("NEG"   ,0x600b,False,CPU7000,2);
   AddTwoReg("NEGC"  ,0x600a,False,CPU7000,2);
   AddTwoReg("SUB"   ,0x3008,False,CPU7000,2);
   AddTwoReg("SUBC"  ,0x300a,False,CPU7000,2);
   AddTwoReg("SUBV"  ,0x300b,False,CPU7000,2);
   AddTwoReg("NOT"   ,0x6007,False,CPU7000,2);
   AddTwoReg("SHAD"  ,0x400c,False,CPU7700,2);
   AddTwoReg("SHLD"  ,0x400d,False,CPU7700,2);

   MulRegOrders=(FixedMinOrder *) malloc(sizeof(FixedMinOrder)*MulRegOrderCount); InstrZ=0;
   AddMulReg("MUL"   ,0x0007,CPU7600);
   AddMulReg("DMULU" ,0x3005,CPU7600);
   AddMulReg("DMULS" ,0x300d,CPU7600);

   BWOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*BWOrderCount); InstrZ=0;
   AddBW("SWAP",0x6008); AddBW("EXTS",0x600e); AddBW("EXTU",0x600c);

   LogOrders=(char **) malloc(sizeof(char *)*LogOrderCount); InstrZ=0;
   LogOrders[InstrZ++]="TST"; LogOrders[InstrZ++]="AND";
   LogOrders[InstrZ++]="XOR"; LogOrders[InstrZ++]="OR" ;

   RegDefs = (TRegDef*) malloc(sizeof(TRegDef) * SRegCnt); InstrZ = 0;
   AddSReg("MACH",  0, CPU7000, FALSE);
   AddSReg("MACL",  1, CPU7000, FALSE);
   AddSReg("PR"  ,  2, CPU7000, FALSE);
   AddSReg("DSR" ,  6, CPU7000, TRUE );
   AddSReg("A0"  ,  7, CPU7000, TRUE );
   AddSReg("X0"  ,  8, CPU7000, TRUE );
   AddSReg("X1"  ,  9, CPU7000, TRUE );
   AddSReg("Y0"  , 10, CPU7000, TRUE );
   AddSReg("Y1"  , 11, CPU7000, TRUE );
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(OneRegOrders);
   free(TwoRegOrders);
   free(MulRegOrders);
   free(BWOrders);
   free(LogOrders);
   free(RegDefs);
END

/*-------------------------------------------------------------------------*/
/* die PC-relative Adresse: direkt nach verzoegerten Spruengen = Sprungziel+2 */

        static LongInt PCRelAdr(void)
BEGIN
   if (PrevDelayed) return DelayedAdr+2;
   else return EProgCounter()+4;
END

        static void ChkDelayed(void)
BEGIN
   if (PrevDelayed) WrError(200);
END

/*-------------------------------------------------------------------------*/
/* Adressparsing */

        static char *LiteralName(PLiteral Lit, char *Result)
BEGIN
   String Tmp;

   if (Lit->IsForward) sprintf(Tmp,"F_%s",HexString(Lit->FCount,8));
   else if (Lit->Is32) sprintf(Tmp,"L_%s",HexString(Lit->Value,8));
   else sprintf(Tmp,"W_%s",HexString(Lit->Value,4));
   sprintf(Result,"LITERAL_%s_%s",Tmp,HexString(Lit->PassNo,0));
   return Result;
END
/*
        static void PrintLiterals(void)
BEGIN
   PLiteral Lauf;
   String Name;

   WrLstLine("LiteralList");
   Lauf=FirstLiteral;
   while (Lauf!=Nil)
    BEGIN
     LiteralName(Lauf, Name);
     WrLstLine(Name); Lauf=Lauf->Next;
    END
END
*/
        static void SetOpSize(ShortInt Size)
BEGIN
   if (OpSize==-1) OpSize=Size;
   else if (Size!=OpSize)
    BEGIN
     WrError(1131); AdrMode=ModNone;
    END
END

        static Boolean DecodeReg(char *Asc, Word *Erg)
BEGIN
   Boolean Err;

   if (strcasecmp(Asc,"SP")==0)
    BEGIN
     *Erg=15; return True;
    END
   else if ((strlen(Asc)<2) OR (strlen(Asc)>3) OR (toupper(*Asc)!='R')) return False;
   else
    BEGIN
     *Erg=ConstLongInt(Asc + 1, &Err, 10);
     return (Err AND (*Erg<=15));
    END
END

        static Boolean DecodeCtrlReg(char *Asc, Word *Erg)
BEGIN
   CPUVar MinCPU=CPU7000;

   *Erg=0xff;
   if (strcasecmp(Asc,"SR")==0) *Erg=0;
   else if (strcasecmp(Asc,"GBR")==0) *Erg=1;
   else if (strcasecmp(Asc,"VBR")==0) *Erg=2;
   else if (strcasecmp(Asc,"SSR")==0)
    BEGIN
     *Erg=3; MinCPU=CPU7700;
    END
   else if (strcasecmp(Asc,"SPC")==0)
    BEGIN
     *Erg=4; MinCPU=CPU7700;
    END
   else if ((strlen(Asc)==7) AND (toupper(*Asc)=='R')
       AND (strcasecmp(Asc+2,"_BANK")==0)
       AND (Asc[1]>='0') AND (Asc[1]<='7'))
    BEGIN
     *Erg=Asc[1]-'0'+8; MinCPU=CPU7700;
    END
   if ((*Erg==0xff) OR (MomCPU<MinCPU))
    BEGIN
     WrXError(1440,Asc); return False;
    END
   else return True;
END

static Boolean DecodeSReg(char *Asc, Word *Erg)
{
  int z;
  Boolean Result = FALSE;

  for (z = 0; z < SRegCnt; z++)
    if (!strcasecmp(Asc, RegDefs[z].Name))
      break;
  if (z < SRegCnt)
  {
    if (MomCPU < RegDefs[z].MinCPU);
    else if ((!DSPAvail) && RegDefs[z].NeedsDSP);
    else
    {
      Result = TRUE;
      *Erg = RegDefs[z].Code;
    }
  }
  return Result;
}

        static void ChkAdr(Word Mask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     WrError(1350); AdrMode=ModNone;
    END
END

        static LongInt ExtOp(LongInt Inp, Byte Src, Boolean Signed)
BEGIN
   switch (Src)
    BEGIN
     case 0: Inp&=0xff; break;
     case 1: Inp&=0xffff; break;
    END
   if (Signed)
    BEGIN
     if (Src<1)
      if ((Inp & 0x80)==0x80) Inp+=0xff00;
     if (Src<2)
      if ((Inp & 0x8000)==0x8000) Inp+=0xffff0000;
    END
   return Inp;
END

        static LongInt OpMask(ShortInt OpSize)
BEGIN
   switch (OpSize)
    BEGIN
     case 0: return 0xff;
     case 1: return 0xffff;
     case 2: return 0xffffffff;
     default: return 0;
    END
END

        static void DecodeAdr(char *Asc, Word Mask, Boolean Signed)
BEGIN
#define RegNone (-1)
#define RegPC (-2)
#define RegGBR (-3)

   Byte p;
   Word HReg;
   char *pos;
   ShortInt BaseReg,IndReg,DOpSize;
   LongInt DispAcc;
   String AdrStr,LStr;
   Boolean OK,FirstFlag,NIs32,Critical,Found,LDef;
   PLiteral Lauf,Last;
   String Name;

   AdrMode=ModNone;

   if (DecodeReg(Asc,&HReg))
    BEGIN
     AdrPart=HReg; AdrMode=ModReg; ChkAdr(Mask); return;
    END

   if (*Asc=='@')
    BEGIN
     strcpy(Asc,Asc+1);
     if (IsIndirect(Asc))
      BEGIN
       strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
       BaseReg=RegNone; IndReg=RegNone;
       DispAcc=0; FirstFlag=False; OK=True;
       while ((*Asc!='\0') AND (OK))
        BEGIN
         pos=QuotPos(Asc,',');
         if (pos==Nil)
          BEGIN
           strmaxcpy(AdrStr,Asc,255); *Asc='\0';
          END
         else
          BEGIN
           *pos='\0'; strmaxcpy(AdrStr,Asc,255); strcpy(Asc,pos+1);
          END
         if (strcasecmp(AdrStr,"PC")==0)
          if (BaseReg==RegNone) BaseReg=RegPC;
          else
           BEGIN
            WrError(1350); OK=False;
           END
         else if (strcasecmp(AdrStr,"GBR")==0)
          if (BaseReg==RegNone) BaseReg=RegGBR;
          else
           BEGIN
            WrError(1350); OK=False;
           END
         else if (DecodeReg(AdrStr,&HReg))
          if (IndReg==RegNone) IndReg=HReg;
          else if ((BaseReg==RegNone) AND (HReg==0)) BaseReg=0;
          else if ((IndReg==0) AND (BaseReg==RegNone))
           BEGIN
            BaseReg=0; IndReg=HReg;
           END
          else
           BEGIN
            WrError(1350); OK=False;
           END
         else
          BEGIN
           FirstPassUnknown=False;
           DispAcc+=EvalIntExpression(AdrStr,Int32,&OK);
           if (FirstPassUnknown) FirstFlag=True;
          END
        END
       if (FirstFlag) DispAcc=0;
       if ((OK) AND ((DispAcc & ((1 << OpSize)-1))!=0))
        BEGIN
         WrError(1325); OK=False;
        END
       else if ((OK) AND (DispAcc<0))
        BEGIN
         WrXError(1315,"Disp<0"); OK=False;
        END
       else DispAcc=DispAcc >> OpSize;
       if (OK)
        BEGIN
         switch (BaseReg)
          BEGIN
           case 0:
            if ((IndReg<0) OR (DispAcc!=0)) WrError(1350);
            else
             BEGIN
              AdrMode=ModR0Base; AdrPart=IndReg;
             END
            break;
           case RegGBR:
            if ((IndReg==0) AND (DispAcc==0)) AdrMode=ModGBRR0;
            else if (IndReg!=RegNone) WrError(1350);
            else if (DispAcc>255) WrError(1320);
            else
             BEGIN
              AdrMode=ModGBRBase; AdrPart=DispAcc;
             END
            break;
           case RegNone:
            if (IndReg==RegNone) WrError(1350);
            else if (DispAcc>15) WrError(1320);
            else
             BEGIN
              AdrMode=ModIndReg; AdrPart=(IndReg << 4)+DispAcc;
             END
            break;
           case RegPC:
            if (IndReg!=RegNone) WrError(1350);
            else if (DispAcc>255) WrError(1320);
            else
             BEGIN
              AdrMode=ModPCRel; AdrPart=DispAcc;
             END
            break;
          END
        END
       ChkAdr(Mask); return;
      END
     else
      BEGIN
       if (DecodeReg(Asc,&HReg))
        BEGIN
         AdrPart=HReg; AdrMode=ModIReg;
        END
       else if ((strlen(Asc)>1) AND (*Asc=='-') AND (DecodeReg(Asc+1,&HReg)))
        BEGIN
         AdrPart=HReg; AdrMode=ModPreDec;
        END
       else if ((strlen(Asc)>1) AND (Asc[strlen(Asc)-1]=='+'))
        BEGIN
         strmaxcpy(AdrStr,Asc,255); AdrStr[strlen(AdrStr)-1]='\0';
         if (DecodeReg(AdrStr,&HReg))
          BEGIN
           AdrPart=HReg; AdrMode=ModPostInc;
          END
         else WrError(1350);
        END
       else WrError(1350);
       ChkAdr(Mask); return;
      END
    END

   if (*Asc=='#')
    BEGIN
     FirstPassUnknown=False;
     switch (OpSize)
      BEGIN
       case 0: DispAcc=EvalIntExpression(Asc+1,Int8,&OK); break;
       case 1: DispAcc=EvalIntExpression(Asc+1,Int16,&OK); break;
       case 2: DispAcc=EvalIntExpression(Asc+1,Int32,&OK); break;
       default: DispAcc=0; OK=True;
      END
     Critical=FirstPassUnknown OR UsesForwards;
     if (OK)
      BEGIN
       /* minimale Groesse optimieren */
       DOpSize=(OpSize==0) ? 0 : Ord(Critical);
       while (((ExtOp(DispAcc,DOpSize,Signed) ^ DispAcc) & OpMask(OpSize))!=0) DOpSize++;
       if (DOpSize==0)
        BEGIN
         AdrPart=DispAcc & 0xff;
         AdrMode=ModImm;
        END
       else if ((Mask & MModPCRel)!=0)
        BEGIN
         /* Literalgroesse ermitteln */
         NIs32=(DOpSize==2);
         if (NOT NIs32) DispAcc&=0xffff;
         /* Literale sektionsspezifisch */
         strcpy(AdrStr,"[PARENT0]");
         /* schon vorhanden ? */
         Lauf=FirstLiteral; p=0; OK=False; Last=Nil; Found=False;
         while ((Lauf!=Nil) AND (NOT Found))
          BEGIN
           Last=Lauf;
           if ((NOT Critical) AND (NOT Lauf->IsForward)
           AND (Lauf->DefSection==MomSectionHandle))
            BEGIN
             if (((Lauf->Is32==NIs32) AND (DispAcc==Lauf->Value))
             OR  ((Lauf->Is32) AND (NOT NIs32) AND (DispAcc==(Lauf->Value >> 16)))) Found=True;
             else if ((Lauf->Is32) AND (NOT NIs32) AND (DispAcc==(Lauf->Value & 0xffff)))
              BEGIN
               Found=True; p=2;
              END
            END
           if (NOT Found) Lauf=Lauf->Next;
          END
         /* nein - erzeugen */
         if (NOT Found)
          BEGIN
           Lauf=(PLiteral) malloc(sizeof(TLiteral));
           Lauf->Is32=NIs32; Lauf->Value=DispAcc;
           Lauf->IsForward=Critical;
           if (Critical) Lauf->FCount=ForwardCount++;
           Lauf->Next=Nil; Lauf->PassNo=1; Lauf->DefSection=MomSectionHandle;
           do
            BEGIN
             LiteralName(Lauf, Name);
             sprintf(LStr,"%s%s",Name,AdrStr);
             LDef=IsSymbolDefined(LStr);
             if (LDef) Lauf->PassNo++;
            END
           while (LDef);
           if (Last==Nil) FirstLiteral=Lauf; else Last->Next=Lauf;
          END
         /* Distanz abfragen - im naechsten Pass... */
         FirstPassUnknown=False;
         LiteralName(Lauf,Name);
         sprintf(LStr,"%s%s",Name,AdrStr);
         DispAcc=EvalIntExpression(LStr,Int32,&OK)+p;
         if (OK)
          BEGIN
           if (FirstPassUnknown)
            DispAcc=0;
           else if (NIs32)
            DispAcc=(DispAcc-(PCRelAdr() & 0xfffffffc)) >> 2;
           else
            DispAcc=(DispAcc-PCRelAdr()) >> 1;
           if (DispAcc<0)
            BEGIN
             WrXError(1315,"Disp<0"); OK=False;
            END
           else if ((DispAcc>255) AND (NOT SymbolQuestionable)) WrError(1330);
           else
            BEGIN
             AdrMode=ModPCRel; AdrPart=DispAcc; OpSize=Ord(NIs32)+1;
            END
          END
        END
       else WrError(1350);
      END
     ChkAdr(Mask); return;
    END

   /* absolut ueber PC-relativ abwickeln */

   if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
   else
    BEGIN
     FirstPassUnknown=False;
     DispAcc=EvalIntExpression(Asc,Int32,&OK);
     if (FirstPassUnknown) DispAcc=0;
     else if (OpSize==2) DispAcc-=(PCRelAdr() & 0xfffffffc);
     else DispAcc-=PCRelAdr();
     if (DispAcc<0) WrXError(1315,"Disp<0");
     else if ((DispAcc & ((1 << OpSize)-1))!=0) WrError(1325);
     else
      BEGIN
       DispAcc=DispAcc >> OpSize;
       if (DispAcc>255) WrError(1320);
       else
        BEGIN
         AdrMode=ModPCRel; AdrPart=DispAcc;
        END
      END
    END

   ChkAdr(Mask);
END

/*-------------------------------------------------------------------------*/

        static void LTORG_16(void)
BEGIN
   PLiteral Lauf;
   String Name;

   Lauf=FirstLiteral;
   while (Lauf!=Nil)
    BEGIN
     if ((NOT Lauf->Is32) AND (Lauf->DefSection==MomSectionHandle))
      BEGIN
       WAsmCode[CodeLen >> 1]=Lauf->Value;
       LiteralName(Lauf,Name);
       EnterIntSymbol(Name,EProgCounter()+CodeLen,SegCode,False);
       Lauf->PassNo=(-1);
       CodeLen+=2;
      END
     Lauf=Lauf->Next;
    END
END

        static void LTORG_32(void)
BEGIN
   PLiteral Lauf,EqLauf;
   String Name;

   Lauf=FirstLiteral;
   while (Lauf!=Nil)
    BEGIN
     if ((Lauf->Is32) AND (Lauf->DefSection==MomSectionHandle) AND (Lauf->PassNo>=0))
      BEGIN
       if (((EProgCounter()+CodeLen) & 2)!=0)
        BEGIN
         WAsmCode[CodeLen >> 1]=0; CodeLen+=2;
        END
       WAsmCode[CodeLen >> 1]=(Lauf->Value >> 16);
       WAsmCode[(CodeLen >> 1)+1]=(Lauf->Value & 0xffff);
       LiteralName(Lauf,Name);
       EnterIntSymbol(Name,EProgCounter()+CodeLen,SegCode,False);
       Lauf->PassNo=(-1);
       if (CompLiterals)
        BEGIN
         EqLauf=Lauf->Next;
         while (EqLauf!=Nil)
          BEGIN
           if ((EqLauf->Is32) AND (EqLauf->PassNo>=0) AND
               (EqLauf->DefSection==MomSectionHandle) AND
               (EqLauf->Value==Lauf->Value))
            BEGIN
             LiteralName(EqLauf,Name);
             EnterIntSymbol(Name,EProgCounter()+CodeLen,SegCode,False);
             EqLauf->PassNo=(-1);
            END
           EqLauf=EqLauf->Next;
          END
        END
       CodeLen+=4;
      END
     Lauf=Lauf->Next;
    END
END

        static Boolean DecodePseudo(void)
BEGIN
   PLiteral Lauf,Tmp,Last;

   /* ab hier (und weiter in der Hauptroutine) stehen die Befehle,
      die Code erzeugen, deshalb wird der Merker fuer verzoegerte
      Spruenge hier weiter geschaltet. */

   PrevDelayed=CurrDelayed; CurrDelayed=False;

   if (Memo("LTORG"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       if ((EProgCounter() & 3)==0)
        BEGIN
         LTORG_32(); LTORG_16();
        END
       else
        BEGIN
         LTORG_16(); LTORG_32();
        END
       Lauf=FirstLiteral; Last=Nil;
       while (Lauf!=Nil)
        BEGIN
         if ((Lauf->DefSection==MomSectionHandle) AND (Lauf->PassNo<0))
          BEGIN
           Tmp=Lauf->Next;
           if (Last==Nil) FirstLiteral=Tmp; else Last->Next=Tmp;
           free(Lauf); Lauf=Tmp;
          END
         else
          BEGIN
           Last=Lauf; Lauf=Lauf->Next;
          END
        END
      END
     return True;
    END

   return False;
END

static Boolean DecodeDSP(void)
{
  Word Cond = 0;

  /* strip off DSP condition */

  if ((Memo("DCT")) || (Memo("DCF")))
  {
    char *pos;
    int z;

    Cond = Memo("DCT") ? 1 : 2;
    if (ArgCnt < 1) 
    {
      WrError(1110);
      return True;
    }
    
    pos = FirstBlank(ArgStr[1]);
    if (!pos)
    {
      strcpy(OpPart, ArgStr[1]);
      for (z = 1; z < ArgCnt; z++)
        strcpy(ArgStr[z], ArgStr[z + 1]);
      ArgCnt--;
    }
    else
    {
      *pos = '\0';
      strcpy(OpPart, ArgStr[1]);
      strcpy(ArgStr[1], pos + 1);
    }
  }

  return False;
}

        static void SetCode(Word Code)
BEGIN
   CodeLen=2; WAsmCode[0]=Code;
END

        static void MakeCode_7000(void)
BEGIN
   int z;
   LongInt AdrLong;
   Boolean OK;
   Word HReg;

   CodeLen=0; DontPrint=False; OpSize=(-1);

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* Attribut verwursten */

   if (*AttrPart!='\0')
    BEGIN
     if (strlen(AttrPart)!=1)
      BEGIN
       WrError(1105); return;
      END
     switch (toupper(*AttrPart))
      BEGIN
       case 'B': SetOpSize(0); break;
       case 'W': SetOpSize(1); break;
       case 'L': SetOpSize(2); break;
       case 'Q': SetOpSize(3); break;
       case 'S': SetOpSize(4); break;
       case 'D': SetOpSize(5); break;
       case 'X': SetOpSize(6); break;
       case 'P': SetOpSize(7); break;
       default:
        WrError(1107); return;
      END
    END

   if (DecodeMoto16Pseudo(OpSize,True)) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else if (MomCPU<FixedOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        SetCode(FixedOrders[z].Code);
        if ((NOT SupAllowed) AND (FixedOrders[z].Priv)) WrError(50);
       END
      return;
     END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     if (OpSize==-1) SetOpSize(2);
     if (ArgCnt!=2) WrError(1110);
     else if (OpSize>2) WrError(1130);
     else if (DecodeReg(ArgStr[1],&HReg))
      BEGIN
       DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPreDec+MModIndReg+MModR0Base+MModGBRBase,True);
       switch (AdrMode)
        BEGIN
         case ModReg:
          if (OpSize!=2) WrError(1130);
          else SetCode(0x6003+(HReg << 4)+(AdrPart << 8));
          break;
         case ModIReg:
          SetCode(0x2000+(HReg << 4)+(AdrPart << 8)+OpSize);
          break;
         case ModPreDec:
          SetCode(0x2004+(HReg << 4)+(AdrPart << 8)+OpSize);
          break;
         case ModIndReg:
          if (OpSize==2)
           SetCode(0x1000+(HReg << 4)+(AdrPart & 15)+((AdrPart & 0xf0) << 4));
          else if (HReg!=0) WrError(1350);
          else SetCode(0x8000+AdrPart+(((Word)OpSize) << 8));
          break;
         case ModR0Base:
          SetCode(0x0004+(AdrPart << 8)+(HReg << 4)+OpSize);
          break;
         case ModGBRBase:
          if (HReg!=0) WrError(1350);
          else SetCode(0xc000+AdrPart+(((Word)OpSize) << 8));
          break;
        END
      END
     else if (DecodeReg(ArgStr[2],&HReg))
      BEGIN
       DecodeAdr(ArgStr[1],MModImm+MModPCRel+MModIReg+MModPostInc+MModIndReg+MModR0Base+MModGBRBase,True);
       switch (AdrMode)
        BEGIN
         case ModIReg:
          SetCode(0x6000+(AdrPart << 4)+(((Word)HReg) << 8)+OpSize);
          break;
         case ModPostInc:
          SetCode(0x6004+(AdrPart << 4)+(((Word)HReg) << 8)+OpSize);
          break;
         case ModIndReg:
          if (OpSize==2)
           SetCode(0x5000+(((Word)HReg) << 8)+AdrPart);
          else if (HReg!=0) WrError(1350);
          else SetCode(0x8400+AdrPart+(((Word)OpSize) << 8));
          break;
         case ModR0Base:
          SetCode(0x000c+(AdrPart << 4)+(((Word)HReg) << 8)+OpSize);
          break;
         case ModGBRBase:
          if (HReg!=0) WrError(1350);
          else SetCode(0xc400+AdrPart+(((Word)OpSize) << 8));
          break;
         case ModPCRel:
          if (OpSize==0) WrError(1350);
          else SetCode(0x9000+(((Word)OpSize-1) << 14)+(((Word)HReg) << 8)+AdrPart);
          break;
         case ModImm:
          SetCode(0xe000+(((Word)HReg) << 8)+AdrPart);
          break;
        END
      END
     else WrError(1350);
     return;
    END

   if (Memo("MOVA"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[2],&HReg)) WrError(1350);
     else if (HReg!=0) WrError(1350);
     else
      BEGIN
       SetOpSize(2);
       DecodeAdr(ArgStr[1],MModPCRel,False);
       if (AdrMode!=ModNone)
        BEGIN
         CodeLen=2; WAsmCode[0]=0xc700+AdrPart;
        END
      END
     return;
    END

   if (Memo("PREF"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg,False);
       if (AdrMode!=ModNone)
        BEGIN
         CodeLen=2; WAsmCode[0]=0x0083+(AdrPart << 8);
        END;
      END;
     return;
    END

   if ((Memo("LDC")) OR (Memo("STC")))
    BEGIN
     if (OpSize==-1) SetOpSize(2);
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (Memo("LDC"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
        END
       if (DecodeCtrlReg(ArgStr[1],&HReg))
        BEGIN
         DecodeAdr(ArgStr[2],MModReg+((Memo("LDC"))?MModPostInc:MModPreDec),False);
         switch (AdrMode)
          BEGIN
           case ModReg:
            if (Memo("LDC")) SetCode(0x400e + (AdrPart << 8)+(HReg << 4)); /* ANSI :-0 */
            else SetCode(0x0002+(AdrPart << 8)+(HReg << 4));
            break;
           case ModPostInc:
            SetCode(0x4007+(AdrPart << 8)+(HReg << 4));
            break;
           case ModPreDec:
            SetCode(0x4003+(AdrPart << 8)+(HReg << 4));
            break;
          END
         if ((AdrMode!=ModNone) AND (NOT SupAllowed)) WrError(50);
        END
      END
     return;
    END

   if ((Memo("LDS")) OR (Memo("STS")))
    BEGIN
     if (OpSize==-1) SetOpSize(2);
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (Memo("LDS"))
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
        END
       if (!DecodeSReg(ArgStr[1], &HReg)) WrError(1440);
       else
        BEGIN
         DecodeAdr(ArgStr[2],MModReg+((Memo("LDS"))?MModPostInc:MModPreDec),False);
         switch (AdrMode)
          BEGIN
           case ModReg:
            if (Memo("LDS")) SetCode(0x400a+(AdrPart << 8)+(HReg << 4));
            else SetCode(0x000a+(AdrPart << 8)+(HReg << 4));
            break;
           case ModPostInc:
            SetCode(0x4006+(AdrPart << 8)+(HReg << 4));
            break;
           case ModPreDec:
            SetCode(0x4002+(AdrPart << 8)+(HReg << 4));
            break;
          END
        END
      END
     return;
    END

   /* nur ein Register als Argument */

   for (z=0; z<OneRegOrderCount; z++)
    if (Memo(OneRegOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else if (MomCPU<OneRegOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg,False);
        if (AdrMode!=ModNone)
         SetCode(OneRegOrders[z].Code+(AdrPart << 8));
        if ((NOT SupAllowed) AND ((Memo("STBR")) OR (Memo("LDBR")))) WrError(50);
        if (*OpPart=='B')
         BEGIN
          CurrDelayed=True; DelayedAdr=0x7fffffff;
          ChkDelayed();
         END
       END
      return;
     END

   if (Memo("TAS"))
    BEGIN
     if (OpSize==-1) SetOpSize(0);
     if (ArgCnt!=1) WrError(1110);
     else if (OpSize!=0) WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg,False);
       if (AdrMode!=ModNone) SetCode(0x401b+(AdrPart << 8));
      END
     return;
    END

   /* zwei Register */

   for (z=0; z<TwoRegOrderCount; z++)
    if (Memo(TwoRegOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if ((*AttrPart!='\0') AND (OpSize!=TwoRegOrders[z].DefSize)) WrError(1100);
      else if (MomCPU<TwoRegOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg,False);
        if (AdrMode!=ModNone)
         BEGIN
          WAsmCode[0]=TwoRegOrders[z].Code+(AdrPart << 4);
          DecodeAdr(ArgStr[2],MModReg,False);
          if (AdrMode!=ModNone) SetCode(WAsmCode[0]+(((Word)AdrPart) << 8));
          if ((NOT SupAllowed) AND (TwoRegOrders[z].Priv)) WrError(50);
         END
       END
      return;
     END

   for (z=0; z<MulRegOrderCount; z++)
    if (Memo(MulRegOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (MomCPU<MulRegOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        if (*AttrPart=='\0') OpSize=2;
        if (OpSize!=2) WrError(1130);
        else
         BEGIN
          DecodeAdr(ArgStr[1],MModReg,False);
          if (AdrMode!=ModNone)
           BEGIN
            WAsmCode[0]=MulRegOrders[z].Code+(AdrPart << 4);
            DecodeAdr(ArgStr[2],MModReg,False);
            if (AdrMode!=ModNone) SetCode(WAsmCode[0]+(((Word)AdrPart) << 8));
           END
         END
       END
      return;
     END

   for (z=0; z<BWOrderCount; z++)
    if (Memo(BWOrders[z].Name))
     BEGIN
      if (OpSize==-1) SetOpSize(1);
      if (ArgCnt!=2) WrError(1110);
      else if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg,False);
        if (AdrMode!=ModNone)
         BEGIN
          WAsmCode[0]=BWOrders[z].Code+OpSize+(AdrPart << 4);
          DecodeAdr(ArgStr[2],MModReg,False);
          if (AdrMode!=ModNone) SetCode(WAsmCode[0]+(((Word)AdrPart) << 8));
         END
       END
      return;
     END

   if (Memo("MAC"))
    BEGIN
     if (OpSize==-1) SetOpSize(1);
     if (ArgCnt!=2) WrError(1110);
     else if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
     else if ((OpSize==2) AND (MomCPU<CPU7600)) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModPostInc,False);
       if (AdrMode!=ModNone)
        BEGIN
         WAsmCode[0]=0x000f+(AdrPart << 4)+(((Word)2-OpSize) << 14);
         DecodeAdr(ArgStr[2],MModPostInc,False);
         if (AdrMode!=ModNone) SetCode(WAsmCode[0]+(((Word)AdrPart) << 8));
        END
      END
     return;
    END

   if (Memo("ADD"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg,False);
       if (AdrMode!=ModNone)
        BEGIN
         HReg=AdrPart; OpSize=2;
         DecodeAdr(ArgStr[1],MModReg+MModImm,True);
         switch (AdrMode)
          BEGIN
           case ModReg:
            SetCode(0x300c+(((Word)HReg) << 8)+(AdrPart << 4));
            break;
           case ModImm:
            SetCode(0x7000+AdrPart+(((Word)HReg) << 8));
            break;
          END
        END
      END
     return;
    END

   if (Memo("CMP/EQ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       DecodeAdr(ArgStr[2],MModReg,False);
       if (AdrMode!=ModNone)
        BEGIN
         HReg=AdrPart; OpSize=2; DecodeAdr(ArgStr[1],MModReg+MModImm,True);
         switch (AdrMode)
          BEGIN
           case ModReg:
            SetCode(0x3000+(((Word)HReg) << 8)+(AdrPart << 4));
            break;
           case ModImm:
            if (HReg!=0) WrError(1350);
            else SetCode(0x8800+AdrPart);
            break;
          END
        END
      END
     return;
    END

   for (z=0; z<LogOrderCount; z++)
    if (Memo(LogOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[2],MModReg+MModGBRR0,False);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if ((*AttrPart!='\0') AND (OpSize!=2)) WrError(1130);
           else
            BEGIN
             OpSize=2;
             HReg=AdrPart; DecodeAdr(ArgStr[1],MModReg+MModImm,False);
             switch (AdrMode)
              BEGIN
               case ModReg:
                SetCode(0x2008+z+(((Word)HReg) << 8)+(AdrPart << 4));
                break;
               case ModImm:
                if (HReg!=0) WrError(1350);
                else SetCode(0xc800+(z << 8)+AdrPart);
                break;
              END
            END
           break;
          case ModGBRR0:
           DecodeAdr(ArgStr[1],MModImm,False);
           if (AdrMode!=ModNone)
            SetCode(0xcc00+(z << 8)+AdrPart);
           break;
         END
       END
      return;
     END

   /* Miszellaneen.. */

   if (Memo("TRAPA"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       OpSize=0;
       DecodeAdr(ArgStr[1],MModImm,False);
       if (AdrMode==ModImm) SetCode(0xc300+AdrPart);
       ChkDelayed();
      END
     return;
    END

   /* Spruenge */

   if ((Memo("BF")) OR (Memo("BT"))
   OR  (Memo("BF/S")) OR (Memo("BT/S")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1110);
     else if ((strlen(OpPart)==4) AND (MomCPU<CPU7600)) WrError(1500);
     else
      BEGIN
       DelayedAdr=EvalIntExpression(ArgStr[1],Int32,&OK);
       AdrLong=DelayedAdr-(EProgCounter()+4);
       if (OK)
        BEGIN
         if (Odd(AdrLong)) WrError(1375);
         else if (((AdrLong<-256) OR (AdrLong>254)) AND (NOT SymbolQuestionable)) WrError(1370);
         else
          BEGIN
           WAsmCode[0]=0x8900+((AdrLong >> 1) & 0xff);
           if (OpPart[1]=='F') WAsmCode[0]+=0x200;
           if (strlen(OpPart)==4)
            BEGIN
             WAsmCode[0]+=0x400; CurrDelayed=True;
            END
           CodeLen=2;
           ChkDelayed();
          END
        END
      END
     return;
    END

   if ((Memo("BRA")) OR (Memo("BSR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1110);
     else
      BEGIN
       DelayedAdr=EvalIntExpression(ArgStr[1],Int32,&OK);
       AdrLong=DelayedAdr-(EProgCounter()+4);
       if (OK)
        BEGIN
         if (Odd(AdrLong)) WrError(1375);
         else if (((AdrLong<-4096) OR (AdrLong>4094)) AND (NOT SymbolQuestionable)) WrError(1370);
         else
          BEGIN
           WAsmCode[0]=0xa000+((AdrLong >> 1) & 0xfff);
           if (Memo("BSR")) WAsmCode[0]+=0x1000;
           CodeLen=2;
           CurrDelayed=True; ChkDelayed();
          END
        END
      END
     return;
    END

   if ((Memo("JSR")) OR (Memo("JMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1130);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg,False);
       if (AdrMode!=ModNone)
        BEGIN
         SetCode(0x400b+(AdrPart << 8)+(Ord(Memo("JMP")) << 5));
         CurrDelayed=True; DelayedAdr=0x7fffffff;
         ChkDelayed();
        END
      END
     return;
    END

   if (DSPAvail)
   {
     if (DecodeDSP())
       return;
   }

   WrXError(1200,OpPart);
END

        static void InitCode_7000(void)
BEGIN
   SaveInitProc();
   FirstLiteral = Nil; ForwardCount = 0;
   SetFlag(&DSPAvail, DSPAvailName, False);
END

        static Boolean IsDef_7000(void)
BEGIN
   return False;
END

        static void SwitchFrom_7000(void)
BEGIN
   DeinitFields();
   if (FirstLiteral!=Nil) WrError(1495);
   ClearONOFF();
END

        static void SwitchTo_7000(void)
BEGIN
   TurnWords=True; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x6c; NOPCode=0x0009;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;
#ifdef __STDC__
   SegLimits[SegCode] = 0xfffffffful;
#else
   SegLimits[SegCode] = 0xffffffffl;
#endif

   MakeCode=MakeCode_7000; IsDef=IsDef_7000;
   SwitchFrom=SwitchFrom_7000; InitFields();
   AddONOFF("SUPMODE",      &SupAllowed,   SupAllowedName  ,False);
   AddONOFF("COMPLITERALS", &CompLiterals, CompLiteralsName,False);
   AddMoto16PseudoONOFF();

   AddONOFF("DSP"     , &DSPAvail  , DSPAvailName , False);

   CurrDelayed=False; PrevDelayed=False;

   SetFlag(&DoPadding,DoPaddingName,False);
END

        void code7000_init(void)
BEGIN
   CPU7000=AddCPU("SH7000",SwitchTo_7000);
   CPU7600=AddCPU("SH7600",SwitchTo_7000);
   CPU7700=AddCPU("SH7700",SwitchTo_7000);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_7000;
   FirstLiteral=Nil;
END
