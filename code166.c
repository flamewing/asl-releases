/* code166.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator Siemens 80C16x                                           */
/*                                                                           */
/* Historie: 11.11.1996 (alaaf) Grundsteinlegung                             */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "stringutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Word Code1,Code2;
         } BaseOrder;

typedef struct
         { 
          char *Name;
          Byte Code;
         } SimpOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;


#define FixedOrderCount 10
#define ConditionCount 20
#define ALU2OrderCount 8
#define ShiftOrderCount 5
#define Bit2OrderCount 6
#define LoopOrderCount 4
#define DivOrderCount 4
#define BJmpOrderCount 4
#define MulOrderCount 3

#define DPPCount 4
static char *RegNames[6]={"DPP0","DPP1","DPP2","DPP3","CP","SP"};

static CPUVar CPU80C166,CPU80C167;

static BaseOrder *FixedOrders;
static Condition *Conditions;
static int TrueCond;
static char **ALU2Orders;
static SimpOrder *ShiftOrders;
static SimpOrder *Bit2Orders;
static SimpOrder *LoopOrders;
static char **DivOrders;
static char **BJmpOrders;
static char **MulOrders;

static void (*SaveInitProc)(void);
static LongInt DPPAssumes[DPPCount];
static IntType MemInt,MemInt2;
static Byte OpSize;

static Boolean DPPChanged[DPPCount],N_DPPChanged[DPPCount];
static Boolean SPChanged,CPChanged,N_SPChanged,N_CPChanged;

static ShortInt ExtCounter;
static enum {MemModeStd,MemModeNoCheck,MemModeZeroPage,MemModeFixedBank,MemModeFixedPage} MemMode;
	     /* normal    EXTS Rn        EXTP Rn         EXTS nn          EXTP nn        */
static Word MemPage;
static Boolean ExtSFRs;

/*-------------------------------------------------------------------------*/

static int InstrZ;


	static void AddFixed(char *NName, CPUVar NMin, Word NCode1, Word NCode2)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ].Code1=NCode1;
   FixedOrders[InstrZ++].Code2=NCode2;
END

	static void AddShift(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ShiftOrderCount) exit(255);
   ShiftOrders[InstrZ].Name=NName;
   ShiftOrders[InstrZ++].Code=NCode;
END

	static void AddBit2(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=Bit2OrderCount) exit(255);
   Bit2Orders[InstrZ].Name=NName;
   Bit2Orders[InstrZ++].Code=NCode;
END

	static void AddLoop(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=LoopOrderCount) exit(255);
   LoopOrders[InstrZ].Name=NName;
   LoopOrders[InstrZ++].Code=NCode;
END

	static void AddCondition(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ConditionCount) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(BaseOrder *) malloc(FixedOrderCount*sizeof(BaseOrder)); InstrZ=0;
   AddFixed("DISWDT",CPU80C166,0x5aa5,0xa5a5);
   AddFixed("EINIT" ,CPU80C166,0x4ab5,0xb5b5);
   AddFixed("IDLE"  ,CPU80C166,0x7887,0x8787);
   AddFixed("NOP"   ,CPU80C166,0x00cc,0x0000);
   AddFixed("PWRDN" ,CPU80C166,0x6897,0x9797);
   AddFixed("RET"   ,CPU80C166,0x00cb,0x0000);
   AddFixed("RETI"  ,CPU80C166,0x88fb,0x0000);
   AddFixed("RETS"  ,CPU80C166,0x00db,0x0000);
   AddFixed("SRST"  ,CPU80C166,0x48b7,0xb7b7);
   AddFixed("SRVWDT",CPU80C166,0x58a7,0xa7a7);

   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCount); InstrZ=0;
   TrueCond=InstrZ; AddCondition("UC" ,0x0); AddCondition("Z"  ,0x2);
   AddCondition("NZ" ,0x3); AddCondition("V"  ,0x4);
   AddCondition("NV" ,0x5); AddCondition("N"  ,0x6);
   AddCondition("NN" ,0x7); AddCondition("C"  ,0x8);
   AddCondition("NC" ,0x9); AddCondition("EQ" ,0x2);
   AddCondition("NE" ,0x3); AddCondition("ULT",0x8);
   AddCondition("ULE",0xf); AddCondition("UGE",0x9);
   AddCondition("UGT",0xe); AddCondition("SLT",0xc);
   AddCondition("SLE",0xb); AddCondition("SGE",0xd);
   AddCondition("SGT",0xa); AddCondition("NET",0x1);

   ALU2Orders=(char **) malloc(sizeof(char *)*ALU2OrderCount); InstrZ=0;
   ALU2Orders[InstrZ++]="ADD" ; ALU2Orders[InstrZ++]="ADDC";
   ALU2Orders[InstrZ++]="SUB" ; ALU2Orders[InstrZ++]="SUBC";
   ALU2Orders[InstrZ++]="CMP" ; ALU2Orders[InstrZ++]="XOR" ;
   ALU2Orders[InstrZ++]="AND" ; ALU2Orders[InstrZ++]="OR"  ;

   ShiftOrders=(SimpOrder *) malloc(sizeof(SimpOrder)*ShiftOrderCount); InstrZ=0;
   AddShift("ASHR",0xac); AddShift("ROL" ,0x0c);
   AddShift("ROR" ,0x2c); AddShift("SHL" ,0x4c);
   AddShift("SHR" ,0x6c);

   Bit2Orders=(SimpOrder *) malloc(sizeof(SimpOrder)*Bit2OrderCount); InstrZ=0;
   AddBit2("BAND",0x6a); AddBit2("BCMP" ,0x2a);
   AddBit2("BMOV",0x4a); AddBit2("BMOVN",0x3a);
   AddBit2("BOR" ,0x5a); AddBit2("BXOR" ,0x7a);

   LoopOrders=(SimpOrder *) malloc(sizeof(SimpOrder)*LoopOrderCount); InstrZ=0;
   AddLoop("CMPD1",0xa0); AddLoop("CMPD2",0xb0);
   AddLoop("CMPI1",0x80); AddLoop("CMPI2",0x90);

   DivOrders=(char **) malloc(sizeof(char *)*DivOrderCount); InstrZ=0;
   DivOrders[InstrZ++]="DIV";  DivOrders[InstrZ++]="DIVU";
   DivOrders[InstrZ++]="DIVL"; DivOrders[InstrZ++]="DIVUL";

   BJmpOrders=(char **) malloc(sizeof(char *)*BJmpOrderCount);  InstrZ=0;
   BJmpOrders[InstrZ++]="JB";  BJmpOrders[InstrZ++]="JNB";
   BJmpOrders[InstrZ++]="JBC"; BJmpOrders[InstrZ++]="JNBS";

   MulOrders=(char **) malloc(sizeof(char *)*MulOrderCount); InstrZ=0;
   MulOrders[InstrZ++]="MUL";   MulOrders[InstrZ++]="MULU";
   MulOrders[InstrZ++]="PRIOR";
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(Conditions);
   free(ALU2Orders);
   free(ShiftOrders);
   free(Bit2Orders);
   free(LoopOrders);
   free(DivOrders);
   free(BJmpOrders);
   free(MulOrders);
END

/*-------------------------------------------------------------------------*/

#define ModNone -1
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModImm 1
#define MModImm (1 << ModImm)
#define ModIReg 2
#define MModIReg (1 << ModIReg)
#define ModPreDec 3
#define MModPreDec (1 << ModPreDec)
#define ModPostInc 4
#define MModPostInc (1 << ModPostInc)
#define ModIndex 5
#define MModIndex (1 << ModIndex)
#define ModAbs 6
#define MModAbs (1 << ModAbs)
#define ModMReg 7
#define MModMReg (1 << ModMReg)
#define ModLAbs 8
#define MModLAbs (1 << ModLAbs)

static Byte AdrCnt,AdrMode;
static Byte AdrVals[2];
static ShortInt AdrType;

	static Boolean IsReg(char *Asc, Byte *Erg, Boolean WordWise)
BEGIN
   Boolean err;

   if ((strlen(Asc)<2) OR (toupper(*Asc)!='R')) return False;
   else if ((strlen(Asc)>2) AND (toupper(Asc[1])=='L') AND (NOT WordWise))
    BEGIN
     *Erg=ConstLongInt(Asc+2,&err); *Erg<<=1;
     return ((err) AND (*Erg<=15));
    END
   else if ((strlen(Asc)>2) AND (toupper(Asc[1])=='H') AND (NOT WordWise))
    BEGIN
     *Erg=ConstLongInt(Asc+2,&err); *Erg<<=1; (*Erg)++;
     return ((err) AND (*Erg<=15));
    END
   else
    BEGIN
     *Erg=ConstLongInt(Asc+1,&err);
     return ((err) AND (*Erg<=15));
    END
END

	static Boolean IsRegM1(char *Asc, Byte *Erg, Boolean WordWise)
BEGIN
   char tmp;
   int l;
   Boolean b;

   if (*Asc!='\0')
    BEGIN
     tmp=Asc[l=(strlen(Asc)-1)]; Asc[l]='\0';
     b=IsReg(Asc,Erg,WordWise);
     Asc[l]=tmp;
     return b;
    END
   else return False;
END

	static LongInt SFRStart(void)
BEGIN
   return (ExtSFRs) ? 0xf000 : 0xfe00;
END

	static LongInt SFREnd(void)
BEGIN
   return (ExtSFRs) ? 0xf1de : 0xffde;
END

	static Boolean CalcPage(LongInt *Adr, Boolean DoAnyway)
BEGIN
   Integer z;
   Word Bank;

   switch (MemMode)
    BEGIN
     case MemModeStd:
      z=0;
      while ((z<=3) AND (((*Adr) >> 14)!=DPPAssumes[z])) z++;
      if (z>3)
       BEGIN
        WrError(110); (*Adr)&=0xffff; return (DoAnyway);
       END
      else
       BEGIN
        *Adr=((*Adr) & 0x3fff)+(z << 14);
        if (DPPChanged[z]) WrXError(200,RegNames[z]);
        return True;
       END
     case MemModeZeroPage:
      (*Adr)&=0x3fff;
      return True;
     case MemModeFixedPage:
      Bank=(*Adr) >> 14; (*Adr)&=0x3fff;
      if (Bank!=MemPage)
       BEGIN
        WrError(110); return (DoAnyway);
       END
      else return True;
     case MemModeNoCheck:
      (*Adr)&=0xffff;
      return True;
     case MemModeFixedBank:
      Bank=(*Adr) >> 16; (*Adr)&=0xffff;
      if (Bank!=MemPage)
       BEGIN
        WrError(110); return (DoAnyway);
       END
      else return True;
     default:
      return False;
    END
END

	static void DecideAbsolute(Boolean InCode, LongInt DispAcc, Word Mask, Boolean Dest)
BEGIN
#define DPPAdr 0xfe00
#define SPAdr 0xfe12
#define CPAdr 0xfe10

   Integer z;

   if (InCode)
    if (((EProgCounter() >> 16)==(DispAcc >> 16)) AND ((Mask & MModAbs)!=0))
     BEGIN
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
     END
    else
     BEGIN
      AdrType=ModLAbs; AdrCnt=2; AdrMode=DispAcc >> 16;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
     END
   else if (((Mask & MModMReg)!=0) AND (DispAcc>=SFRStart()) AND (DispAcc<=SFREnd()) AND ((DispAcc&1)==0))
    BEGIN
     AdrType=ModMReg; AdrCnt=1; AdrVals[0]=(DispAcc-SFRStart()) >> 1;
    END
   else switch (MemMode)
    BEGIN
     case MemModeStd:
      z=0;
      while ((z<=3) AND ((DispAcc >> 14)!=DPPAssumes[z])) z++;
      if (z>3)
       BEGIN
        WrError(110); z=(DispAcc >> 14) & 3;
       END
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=(Hi(DispAcc) & 0x3f)+(z << 6);
      if (DPPChanged[z]) WrXError(200,RegNames[z]);
      break;
     case MemModeZeroPage:
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc) & 0x3f;
      break;
     case MemModeFixedPage:
      if ((DispAcc >> 14)!=MemPage) WrError(110);
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc) & 0x3f;
      break;
     case MemModeNoCheck:
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
      break;
     case MemModeFixedBank:
      if ((DispAcc >> 16)!=MemPage) WrError(110);
      AdrType=ModAbs; AdrCnt=2;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
      break;
    END

   if ((AdrType!=ModNone) AND (Dest))
    switch ((Word)DispAcc)
     BEGIN
      case SPAdr    : N_SPChanged=True; break;
      case CPAdr    : N_CPChanged=True; break;
      case DPPAdr   :
      case DPPAdr+1 : N_DPPChanged[0]=True; break;
      case DPPAdr+2 :
      case DPPAdr+3 : N_DPPChanged[1]=True; break;
      case DPPAdr+4 :
      case DPPAdr+5 : N_DPPChanged[2]=True; break;
      case DPPAdr+6 :
      case DPPAdr+7 : N_DPPChanged[3]=True; break;
     END
END

	static void DecodeAdr(char *Asc, Word Mask, Boolean InCode, Boolean Dest)
BEGIN
   LongInt HDisp,DispAcc;
   char *PPos,*MPos;
   String Part;
   Boolean OK,NegFlag,NNegFlag;
   Byte HReg;

   AdrType=ModNone; AdrCnt=0;

   /* immediate ? */

   if (*Asc=='#')
    BEGIN
     switch (OpSize)
      BEGIN
       case 0:
	AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
	AdrVals[1]=0;
        break;
       case 1:
	HDisp=EvalIntExpression(Asc+1,Int16,&OK);
	AdrVals[0]=Lo(HDisp); AdrVals[1]=Hi(HDisp);
        break;
      END
     if (OK)
      BEGIN
       AdrType=ModImm; AdrCnt=OpSize+1;
      END
    END

   /* Register ? */

   else if (IsReg(Asc,&AdrMode,OpSize==1))
    BEGIN
     if ((Mask & MModReg)!=0) AdrType=ModReg;
     else
      BEGIN
       AdrType=ModMReg; AdrVals[0]=0xf0+AdrMode; AdrCnt=1;
      END
     if (CPChanged) WrXError(200,RegNames[4]);
    END

   /* indirekt ? */

   else if ((*Asc=='[') AND (Asc[strlen(Asc)-1]==']'))
    BEGIN
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';

     /* Predekrement ? */

     if ((strlen(Asc)>2) AND (*Asc=='-') AND (IsReg(Asc+1,&AdrMode,True)))
      AdrType=ModPreDec;

     /* Postinkrement ? */

     else if ((strlen(Asc)>2) AND (Asc[strlen(Asc)-1]=='+') AND (IsRegM1(Asc,&AdrMode,True)))
      AdrType=ModPostInc;

     /* indiziert ? */

     else
      BEGIN
       NegFlag=False; DispAcc=0; AdrMode=0xff;
       while (*Asc!='\0')
	BEGIN
	 MPos=QuotPos(Asc,'-'); PPos=QuotPos(Asc,'+');
	 if (((MPos<PPos) OR (PPos==Nil)) AND (MPos!=Nil))
	  BEGIN
	   PPos=MPos; NNegFlag=True;
	  END
	 else NNegFlag=False;
         if (PPos==Nil)
          BEGIN
           strmaxcpy(Part,Asc,255); *Asc='\0';
          END
         else
          BEGIN
           *PPos='\0'; strmaxcpy(Part,Asc,255); strcpy(Asc,PPos+1);
          END
	 if (IsReg(Part,&HReg,True))
	  if ((NegFlag) OR (AdrMode!=0xff)) WrError(1350); else AdrMode=HReg;
	 else
	  BEGIN
	   if (*Part=='#') strcpy(Part,Part+1);
	   HDisp=EvalIntExpression(Part,Int32,&OK);
	   if (OK)
	    if (NegFlag) DispAcc-=HDisp; else DispAcc+=HDisp;
	  END
	 NegFlag=NNegFlag;
	END
       if (AdrMode==0xff) DecideAbsolute(InCode,DispAcc,Mask,Dest);
       else if (DispAcc==0) AdrType=ModIReg;
       else if (DispAcc>0xffff) WrError(1320);
       else if (DispAcc<-0x8000) WrError(1315);
       else
	BEGIN
	 AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
	 AdrType=ModIndex; AdrCnt=2;
	END
      END
    END
   else
    BEGIN
     DispAcc=EvalIntExpression(Asc,MemInt,&OK);
     if (OK) DecideAbsolute(InCode,DispAcc,Mask,Dest);
    END

   if ((AdrType!=ModNone) AND (((1 << AdrType) & Mask)==0))
    BEGIN
     WrError(1350); AdrType=ModNone; AdrCnt=0;
    END
END

	static Integer DecodeCondition(char *Name)
BEGIN
   Integer z;

   NLS_UpString(Name);
   for (z=0; z<ConditionCount; z++)
    if (strcmp(Conditions[z].Name,Name)==0) break;
   return z;
END

	static Boolean DecodeBitAddr(char *Asc, Word *Adr, Byte *Bit, Boolean MayBeOut)
BEGIN
   char *p;
   Word LAdr;
   Byte Reg;
   Boolean OK;

   p=QuotPos(Asc,'.');
   if (p==Nil)
    BEGIN
     LAdr=EvalIntExpression(Asc,UInt16,&OK) & 0x1fff;
     if (OK)
      BEGIN
       if ((NOT MayBeOut) AND ((LAdr >> 12)!=Ord(ExtSFRs)))
	BEGIN
	 WrError(1335); return False;
	END
       *Adr=LAdr >> 4; *Bit=LAdr & 15;
       if (NOT MayBeOut) *Adr=Lo(*Adr);
       return True;
      END
     else return False;
    END
   else if (p<=Asc+1) return False;
   else
    BEGIN
     *p='\0';
     if (IsReg(Asc,&Reg,True)) *Adr=0xf0+Reg;
     else
      BEGIN
       FirstPassUnknown=False;
       LAdr=EvalIntExpression(Asc,UInt16,&OK); if (NOT OK) return False;
       if (FirstPassUnknown) LAdr=0xfd00;
       if ((LAdr&1)==1)
	BEGIN
	 WrError(1325); return False;
	END
       if ((LAdr>=0xfd00) AND (LAdr<=0xfdfe)) *Adr=(LAdr-0xfd00)/2;
       else if ((LAdr>=0xff00) AND (LAdr<=0xffde))
	BEGIN
	 if ((ExtSFRs) AND (NOT MayBeOut))
	  BEGIN
	   WrError(1335); return False;
	  END
	 *Adr=0x80+((LAdr-0xff00)/2);
	END
       else if ((LAdr>=0xf100) AND (LAdr<=0xf1de))
	BEGIN
	 if ((NOT ExtSFRs) AND (NOT MayBeOut))
	  BEGIN
	   WrError(1335); return False;
	  END
	 *Adr=0x80+((LAdr-0xf100)/2);
	 if (MayBeOut) (*Adr)+=0x100;
	END
       else
	BEGIN
	 WrError(1320); return False;
	END
      END

     *Bit=EvalIntExpression(p+1,UInt4,&OK);
     return OK;
    END
END

	static Word WordVal(void)
BEGIN
   return AdrVals[0]+(((Word)AdrVals[1]) << 8);
END

	static Boolean DecodePref(char *Asc, Byte *Erg)
BEGIN
   Boolean OK;

   if (*Asc!='#')
    BEGIN
     WrError(1350); return False;
    END
   strcpy(Asc,Asc+1);
   FirstPassUnknown=False;
   *Erg=EvalIntExpression(Asc,UInt3,&OK);
   if (FirstPassUnknown) *Erg=1;
   if (NOT OK) return False;
   if (*Erg<1) WrError(1315);
   else if (*Erg>4) WrError(1320);
   else
    BEGIN
     (*Erg)--; return True;
    END
   return False;
END

/*-------------------------------------------------------------------------*/

#define ASSUME166Count 4
static ASSUMERec ASSUME166s[ASSUME166Count]=
	     {{"DPP0", DPPAssumes+0, 0, 15, -1},
	      {"DPP1", DPPAssumes+1, 0, 15, -1},
	      {"DPP2", DPPAssumes+2, 0, 15, -1},
	      {"DPP3", DPPAssumes+3, 0, 15, -1}};

	static Boolean DecodePseudo(void)
BEGIN
   Word Adr;
   Byte Bit;

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME166s,ASSUME166Count);
     return True;
    END

   if (Memo("BIT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeBitAddr(ArgStr[1],&Adr,&Bit,True))
      BEGIN
       PushLocHandle(-1);
       EnterIntSymbol(LabPart,(Adr << 4)+Bit,SegNone,False);
       PopLocHandle();
       sprintf(ListLine,"=%02xH.%1x",Adr,Bit);
      END
     return True;
    END

   return False;
END

	static Boolean BMemo(char *Name)
BEGIN
   int l;

   if (strncmp(OpPart,Name,l=strlen(Name))!=0) return False;
   switch (OpPart[l])
    BEGIN
     case '\0':
      OpSize=1; return True;
     case 'B':
      if (OpPart[l+1]=='\0')
       BEGIN
        OpSize=0; return True;
       END
      else return False;
     default:
      return False;
    END
END

	static void MakeCode_166(void)
BEGIN
   Integer z,Cond;
   Word AdrWord;
   Byte AdrBank,HReg;
   Byte BOfs1,BOfs2;
   Word BAdr1,BAdr2;
   LongInt AdrLong;
   Boolean OK;

   CodeLen=0; DontPrint=False; OpSize=1;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   /* Pipeline-Flags weiterschalten */

   SPChanged=N_SPChanged; N_SPChanged=False;
   CPChanged=N_CPChanged; N_CPChanged=False;
   for (z=0; z<DPPCount; z++)
    BEGIN
     DPPChanged[z]=N_DPPChanged[z];
     N_DPPChanged[z]=False;
    END

   /* Praefixe herunterzaehlen */

   if (ExtCounter>=0)
    if (--ExtCounter<0)
     BEGIN
      MemMode=MemModeStd;
      ExtSFRs=False;
     END

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
	CodeLen=2;
	BAsmCode[0]=Lo(FixedOrders[z].Code1);
        BAsmCode[1]=Hi(FixedOrders[z].Code1);
	if (FixedOrders[z].Code2!=0)
	 BEGIN
	  CodeLen=4;
	  BAsmCode[2]=Lo(FixedOrders[z].Code2);
          BAsmCode[3]=Hi(FixedOrders[z].Code2);
	  if ((strncmp(OpPart,"RET",3)==0) AND (SPChanged)) WrXError(200,RegNames[5]);
	 END
       END
      return;
     END

   /* Datentransfer */

   if (BMemo("MOV"))
    BEGIN
     Cond=1-OpSize;
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModMReg+MModIReg+MModPreDec+MModPostInc+MModIndex+MModAbs,False,True);
       switch (AdrType)
        BEGIN
         case ModReg:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModImm+MModIReg+MModPostInc+MModIndex+MModAbs,False,False);
	  switch (AdrType)
           BEGIN
	    case ModReg:
	     CodeLen=2; BAsmCode[0]=0xf0+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModImm:
	     if (WordVal()<=15)
	      BEGIN
	       CodeLen=2; BAsmCode[0]=0xe0+Cond;
	       BAsmCode[1]=(WordVal() << 4)+HReg;
	      END
	     else
	      BEGIN
	       CodeLen=4; BAsmCode[0]=0xe6+Cond; BAsmCode[1]=HReg+0xf0;
	       memcpy(BAsmCode+2,AdrVals,2);
	      END
             break;
	    case ModIReg:
	     CodeLen=2; BAsmCode[0]=0xa8+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModPostInc:
	     CodeLen=2; BAsmCode[0]=0x98+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModIndex:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xd4+(Cond << 5);
	     BAsmCode[1]=(HReg << 4)+AdrMode; memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	    case ModAbs:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf2+Cond;
	     BAsmCode[1]=0xf0+HReg; memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	   END
	  break;
         case ModMReg:
	  BAsmCode[1]=AdrVals[0];
          DecodeAdr(ArgStr[2],MModImm+MModMReg+((DPPAssumes[3]==3)?MModIReg:0)+MModAbs,False,False);
	  switch (AdrType)
           BEGIN
	    case ModImm:
	     CodeLen=4; BAsmCode[0]=0xe6+Cond;
	     memcpy(BAsmCode+2,AdrVals,2);
	     break;
            case ModMReg: /* BAsmCode[1] sicher absolut darstellbar, da Rn vorher */
                          /* abgefangen wird! */
             BAsmCode[0]=0xf6+Cond;
             AdrLong=0xfe00+(((Word)BAsmCode[1]) << 1);
             CalcPage(&AdrLong,True);
	     BAsmCode[2]=Lo(AdrLong);
             BAsmCode[3]=Hi(AdrLong);
             BAsmCode[1]=AdrVals[0];
             CodeLen=4;
             break;
	    case ModIReg:
	     CodeLen=4; BAsmCode[0]=0x94+(Cond << 5);
	     BAsmCode[2]=BAsmCode[1] << 1;
	     BAsmCode[3]=0xfe + (BAsmCode[1] >> 7); /* ANSI :-0 */
	     BAsmCode[1]=AdrMode;
	     break;
	    case ModAbs:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xf2+Cond;
	     memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	   END
	  break;
         case ModIReg:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPostInc+MModAbs,False,False);
	  switch (AdrType)
           BEGIN
	    case ModReg:
	     CodeLen=2; BAsmCode[0]=0xb8+Cond;
	     BAsmCode[1]=HReg+(AdrMode << 4);
	     break;
	    case ModIReg:
	     CodeLen=2; BAsmCode[0]=0xc8+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModPostInc:
	     CodeLen=2; BAsmCode[0]=0xe8+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	    case ModAbs:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0x84+(Cond << 5);
	     BAsmCode[1]=HReg; memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	   END
	  break;
         case ModPreDec:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg,False,False);
	  switch (AdrType)
           BEGIN
	    case ModReg:
	     CodeLen=2; BAsmCode[0]=0x88+Cond;
	     BAsmCode[1]=HReg+(AdrMode << 4);
	     break;
	   END
	  break;
         case ModPostInc:
	  HReg=AdrMode;
	  DecodeAdr(ArgStr[2],MModIReg,False,False);
	  switch (AdrType)
           BEGIN
	    case ModIReg:
 	     CodeLen=2; BAsmCode[0]=0xd8+Cond;
	     BAsmCode[1]=(HReg << 4)+AdrMode;
	     break;
	   END
	  break;
         case ModIndex:
	  BAsmCode[1]=AdrMode; memcpy(BAsmCode+2,AdrVals,AdrCnt);
	  DecodeAdr(ArgStr[2],MModReg,False,False);
	  switch (AdrType)
           BEGIN
	    case ModReg:
	     BAsmCode[0]=0xc4+(Cond << 5);
	     CodeLen=4; BAsmCode[1]+=AdrMode << 4;
	     break;
	   END
	  break;
         case ModAbs:
	  memcpy(BAsmCode+2,AdrVals,AdrCnt);
	  DecodeAdr(ArgStr[2],MModIReg+MModMReg,False,False);
	  switch (AdrType)
           BEGIN
	    case ModIReg:
	     CodeLen=4; BAsmCode[0]=0x94+(Cond << 5);
	     BAsmCode[1]=AdrMode;
	     break;
	    case ModMReg:
	     CodeLen=4; BAsmCode[0]=0xf6+Cond;
	     BAsmCode[1]=AdrVals[0];
	     break;
	   END
	  break;
        END
      END
     return;
    END

   if ((Memo("MOVBS")) OR (Memo("MOVBZ")))
    BEGIN
     Cond=Ord(Memo("MOVBS")) << 4;
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OpSize=1;
       DecodeAdr(ArgStr[1],MModReg+MModMReg+MModAbs,False,True);
       OpSize=0;
       switch (AdrType)
        BEGIN
         case ModReg:
	  HReg=AdrMode; DecodeAdr(ArgStr[2],MModReg,False,False);
	  switch (AdrType)
           BEGIN
	    case ModReg:
	     CodeLen=2; BAsmCode[0]=0xc0+Cond;
	     BAsmCode[1]=HReg+(AdrMode << 4);
	     break;
	   END
	  break;
         case ModMReg:
	  BAsmCode[1]=AdrVals[0];
	  DecodeAdr(ArgStr[2],MModAbs+MModMReg,False,False);
	  switch (AdrType)
           BEGIN
            case ModMReg: /* BAsmCode[1] sicher absolut darstellbar, da Rn vorher */
                          /* abgefangen wird! */
             BAsmCode[0]=0xc5+Cond;
             AdrLong=0xfe00+(((Word)BAsmCode[1]) << 1);
             CalcPage(&AdrLong,True);
	     BAsmCode[2]=Lo(AdrLong);
             BAsmCode[3]=Hi(AdrLong);
             BAsmCode[1]=AdrVals[0];
             CodeLen=4;
             break;
	    case ModAbs:
	     CodeLen=2+AdrCnt; BAsmCode[0]=0xc2+Cond;
	     memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     break;
	   END
	  break;
         case ModAbs:
	  memcpy(BAsmCode+2,AdrVals,AdrCnt);
	  DecodeAdr(ArgStr[2],MModMReg,False,False);
	  switch (AdrType)
           BEGIN
	    case ModMReg:
	     CodeLen=4; BAsmCode[0]=0xc5+Cond;
	     BAsmCode[1]=AdrVals[0];
	     break;
	   END
	  break;
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModMReg,False,Memo("POP"));
       switch (AdrType)
        BEGIN
         case ModMReg:
	  CodeLen=2; BAsmCode[0]=0xec+(Ord(Memo("POP")) << 4);
	  BAsmCode[1]=AdrVals[0];
	  if (SPChanged) WrXError(200,RegNames[5]);
	  break;
        END
      END
     return;
    END

   if (Memo("SCXT"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModMReg,False,True);
       switch (AdrType)
        BEGIN
         case ModMReg:
	  BAsmCode[1]=AdrVals[0];
	  DecodeAdr(ArgStr[2],MModAbs+MModImm,False,False);
	  if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=4; BAsmCode[0]=0xc6+(Ord(AdrType==ModAbs) << 4);
	    memcpy(BAsmCode+2,AdrVals,2);
	   END
	  break;
        END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<ALU2OrderCount; z++)
    if (BMemo(ALU2Orders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	Cond=(1-OpSize)+(z << 4);
	DecodeAdr(ArgStr[1],MModReg+MModMReg+MModAbs,False,True);
	switch (AdrType)
         BEGIN
	  case ModReg:
	   HReg=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPostInc+MModAbs+MModImm,False,False);
	   switch (AdrType)
            BEGIN
	     case ModReg:
	      CodeLen=2; BAsmCode[0]=Cond;
	      BAsmCode[1]=(HReg << 4)+AdrMode;
	      break;
	     case ModIReg:
	      if (AdrMode>3) WrError(1350);
	      else
	       BEGIN
	        CodeLen=2; BAsmCode[0]=0x08+Cond;
	        BAsmCode[1]=(HReg << 4)+8+AdrMode;
	       END
              break;
	     case ModPostInc:
	      if (AdrMode>3) WrError(1350);
	      else
	       BEGIN
	        CodeLen=2; BAsmCode[0]=0x08+Cond;
	        BAsmCode[1]=(HReg << 4)+12+AdrMode;
	       END
              break;
	     case ModAbs:
	      CodeLen=4; BAsmCode[0]=0x02+Cond; BAsmCode[1]=0xf0+HReg;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	      break;
	     case ModImm:
	      if (WordVal()<=7)
	       BEGIN
	        CodeLen=2; BAsmCode[0]=0x08+Cond;
	        BAsmCode[1]=(HReg << 4)+AdrVals[0];
	       END
	      else
	       BEGIN
	        CodeLen=4; BAsmCode[0]=0x06+Cond; BAsmCode[1]=0xf0+HReg;
	        memcpy(BAsmCode+2,AdrVals,2);
	       END
              break;
	    END
	   break;
	  case ModMReg:
	   BAsmCode[1]=AdrVals[0];
	   DecodeAdr(ArgStr[2],MModAbs+MModMReg+MModImm,False,False);
	   switch (AdrType)
            BEGIN
	     case ModAbs:
	      CodeLen=4; BAsmCode[0]=0x02+Cond;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	      break;
             case ModMReg: /* BAsmCode[1] sicher absolut darstellbar, da Rn vorher */
                           /* abgefangen wird! */
              BAsmCode[0]=0x04+Cond;
              AdrLong=0xfe00+(((Word)BAsmCode[1]) << 1);
              CalcPage(&AdrLong,True);
	      BAsmCode[2]=Lo(AdrLong);
              BAsmCode[3]=Hi(AdrLong);
              BAsmCode[1]=AdrVals[0];
              CodeLen=4;
              break;
	     case ModImm:
	      CodeLen=4; BAsmCode[0]=0x06+Cond;
	      memcpy(BAsmCode+2,AdrVals,2);
	      break;
	    END
	   break;
	  case ModAbs:
	   memcpy(BAsmCode+2,AdrVals,AdrCnt);
	   DecodeAdr(ArgStr[2],MModMReg,False,False);
	   switch (AdrType)
            BEGIN
	     case ModMReg:
	      CodeLen=4; BAsmCode[0]=0x04+Cond; BAsmCode[1]=AdrVals[0];
	      break;
	    END
	   break;
	 END
       END
      return;
     END

   if ((BMemo("CPL")) OR (BMemo("NEG")))
    BEGIN
     Cond=0x81+((1-OpSize) << 5);
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg,False,True);
       if (AdrType==ModReg)
	BEGIN
	 CodeLen=2; BAsmCode[0]=Cond+(Ord(BMemo("CPL")) << 4);
	 BAsmCode[1]=AdrMode << 4;
	END
      END
     return;
    END

   for (z=0; z<DivOrderCount; z++)
    if (Memo(DivOrders[z]))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
	DecodeAdr(ArgStr[1],MModReg,False,False);
	if (AdrType==ModReg)
	 BEGIN
	  CodeLen=2; BAsmCode[0]=0x4b+(z << 4);
	  BAsmCode[1]=AdrMode*0x11;
	 END
       END
      return;
     END

   for (z=0; z<LoopOrderCount; z++)
    if (Memo(LoopOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg,False,True);
        if (AdrType==ModReg)
         BEGIN
          BAsmCode[1]=AdrMode;
          DecodeAdr(ArgStr[2],MModAbs+MModImm,False,False);
          switch (AdrType)
           BEGIN
            case ModAbs:
             CodeLen=4; BAsmCode[0]=LoopOrders[z].Code+2; BAsmCode[1]+=0xf0;
             memcpy(BAsmCode+2,AdrVals,2);
             break;
            case ModImm:
             if (WordVal()<16)
              BEGIN
               CodeLen=2; BAsmCode[0]=LoopOrders[z].Code;
               BAsmCode[1]+=(WordVal() << 4);
              END
             else
              BEGIN
               CodeLen=4; BAsmCode[0]=LoopOrders[z].Code+6; BAsmCode[1]+=0xf0;
               memcpy(BAsmCode+2,AdrVals,2);
              END
             break;
           END
         END
       END
      return;
     END

   for (z=0; z<MulOrderCount; z++)
    if (Memo(MulOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	DecodeAdr(ArgStr[1],MModReg,False,False);
	switch (AdrType)
         BEGIN
	  case ModReg:
	   HReg=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg,False,False);
	   switch (AdrType)
            BEGIN
	     case ModReg:
	      CodeLen=2; BAsmCode[0]=0x0b+(z << 4);
	      BAsmCode[1]=(HReg << 4)+AdrMode;
	      break;
	    END
	   break;
 	 END
       END
      return;
     END

   /* Logik */

   for (z=0; z<ShiftOrderCount; z++)
    if (Memo(ShiftOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        OpSize=1;
        DecodeAdr(ArgStr[1],MModReg,False,True);
        switch (AdrType)
         BEGIN
          case ModReg:
           HReg=AdrMode;
           DecodeAdr(ArgStr[2],MModReg+MModImm,False,True);
           switch (AdrType)
            BEGIN
             case ModReg:
              BAsmCode[0]=ShiftOrders[z].Code; BAsmCode[1]=AdrMode+(HReg << 4);
              CodeLen=2;
              break;
             case ModImm:
              if (WordVal()>15) WrError(1320);
              else
               BEGIN
                BAsmCode[0]=ShiftOrders[z].Code+0x10;
                BAsmCode[1]=(WordVal() << 4)+HReg;
                CodeLen=2;
               END
              break;
            END
           break;
         END
       END
      return;
     END

   for (z=0; z<Bit2OrderCount; z++)
    if (Memo(Bit2Orders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       if (DecodeBitAddr(ArgStr[1],&BAdr1,&BOfs1,False))
       if (DecodeBitAddr(ArgStr[2],&BAdr2,&BOfs2,False))
        BEGIN
         CodeLen=4; BAsmCode[0]=Bit2Orders[z].Code;
         BAsmCode[1]=BAdr2; BAsmCode[2]=BAdr1;
         BAsmCode[3]=(BOfs2 << 4)+BOfs1;
        END
      return;
     END

   if ((Memo("BCLR")) OR (Memo("BSET")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeBitAddr(ArgStr[1],&BAdr1,&BOfs1,False))
      BEGIN
       CodeLen=2; BAsmCode[0]=(BOfs1 << 4)+0x0e + Ord(Memo("BSET")); /* ANSI :-0 */
       BAsmCode[1]=BAdr1;
      END
     return;
    END

   if ((Memo("BFLDH")) OR (Memo("BFLDL")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else 
      BEGIN
       strmaxcat(ArgStr[1],".0",255);
       if (DecodeBitAddr(ArgStr[1],&BAdr1,&BOfs1,False))
        BEGIN
         OpSize=0; BAsmCode[1]=BAdr1;
         DecodeAdr(ArgStr[2],MModImm,False,False);
         if (AdrType==ModImm)
  	  BEGIN
  	   BAsmCode[2]=AdrVals[0];
  	   DecodeAdr(ArgStr[3],MModImm,False,False);
  	   if (AdrType==ModImm)
  	    BEGIN
  	     BAsmCode[3]=AdrVals[0];
  	     CodeLen=4; BAsmCode[0]=0x0a;
  	     if (Memo("BFLDH"))
  	      BEGIN
  	       BAdr1=BAsmCode[2]; BAsmCode[2]=BAsmCode[3]; BAsmCode[3]=BAdr1;
  	       BAsmCode[0]+=0x10;
  	      END
  	    END
  	  END
        END
      END
     return;
    END

   /*Spruenge */

   if (Memo("JMP"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModAbs+MModLAbs+MModIReg,True,False);
	 switch (AdrType)
          BEGIN
	   case ModLAbs:
	    if (Cond!=TrueCond) WrXError(1360,ArgStr[1]);
	    else
	     BEGIN
	      CodeLen=2+AdrCnt; BAsmCode[0]=0xfa; BAsmCode[1]=AdrMode;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     END
            break;       
	   case ModAbs:
	    AdrLong=WordVal()-(EProgCounter()+2);
	    if ((AdrLong<=254) AND (AdrLong>=-256) AND ((AdrLong&1)==0))
	     BEGIN
	      CodeLen=2; BAsmCode[0]=0x0d+(Conditions[Cond].Code << 4);
	      BAsmCode[1]=(AdrLong/2)&0xff;
	     END
	    else
	     BEGIN
	      CodeLen=2+AdrCnt; BAsmCode[0]=0xea;
	      BAsmCode[1]=Conditions[Cond].Code << 4;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     END
	    break;
	   case ModIReg:
	    CodeLen=2; BAsmCode[0]=0x9c;
	    BAsmCode[1]=(Conditions[Cond].Code << 4)+AdrMode;
	    break;
	  END
	END
      END
     return;
    END

   if (Memo("CALL"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModAbs+MModLAbs+MModIReg,True,False);
	 switch (AdrType)
          BEGIN
	   case ModLAbs:
	    if (Cond!=TrueCond) WrXError(1360,ArgStr[1]);
	    else
	     BEGIN
	      CodeLen=2+AdrCnt; BAsmCode[0]=0xda; BAsmCode[1]=AdrMode;
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     END
            break;
	   case ModAbs:
	    AdrLong=WordVal()-(EProgCounter()+2);
	    if ((AdrLong<=254) AND (AdrLong>=-256) AND ((AdrLong&1)==0) AND (Cond==TrueCond))
	     BEGIN
	      CodeLen=2; BAsmCode[0]=0xbb;
	      BAsmCode[1]=(AdrLong/2) & 0xff;
	     END
	    else
	     BEGIN
	      CodeLen=2+AdrCnt; BAsmCode[0]=0xca;
	      BAsmCode[1]=0xc0+(Conditions[Cond].Code << 4);
	      memcpy(BAsmCode+2,AdrVals,AdrCnt);
	     END
	    break;
	   case ModIReg:
	    CodeLen=2; BAsmCode[0]=0xab;
	    BAsmCode[1]=(Conditions[Cond].Code << 4)+AdrMode;
	    break;
	  END
	END
      END
     return;
    END

   if (Memo("JMPR"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	BEGIN
	 AdrLong=EvalIntExpression(ArgStr[ArgCnt],MemInt,&OK)-(EProgCounter()+2);
	 if (OK)
	  if ((AdrLong&1)==1) WrError(1375);
	  else if ((NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256))) WrError(1370);
	  else
	   BEGIN
	    CodeLen=2; BAsmCode[0]=0x0d+(Conditions[Cond].Code << 4);
	    BAsmCode[1]=(AdrLong/2) & 0xff;
	   END
	END
      END
     return;
    END

   if (Memo("CALLR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[ArgCnt],MemInt,&OK)-(EProgCounter()+2);
       if (OK)
	if ((AdrLong&1)==1) WrError(1375);
	else if ((NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256))) WrError(1370);
	else
	 BEGIN
	  CodeLen=2; BAsmCode[0]=0xbb;
	  BAsmCode[1]=(AdrLong/2) & 0xff;
	 END
      END
     return;
    END

   if ((Memo("JMPA")) OR (Memo("CALLA")))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	BEGIN
	 AdrLong=EvalIntExpression(ArgStr[ArgCnt],MemInt,&OK);
	 if (OK)
	  if ((AdrLong >> 16)!=(EProgCounter() >> 16)) WrError(1910);
	  else
	   BEGIN
	    CodeLen=4;
            BAsmCode[0]=(Memo("JMPA")) ? 0xea : 0xca;
	    BAsmCode[1]=0x00+(Conditions[Cond].Code << 4);
	    BAsmCode[2]=Lo(AdrLong); BAsmCode[3]=Hi(AdrLong);
	   END
	END
      END
     return;
    END

   if ((Memo("JMPS")) OR (Memo("CALLS")))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==1)
	BEGIN
	 AdrLong=EvalIntExpression(ArgStr[1],MemInt,&OK);
	 AdrWord=AdrLong & 0xffff; AdrBank=AdrLong >> 16;
	END
       else
	BEGIN
	 AdrWord=EvalIntExpression(ArgStr[2],UInt16,&OK);
	 if (OK) AdrBank=EvalIntExpression(ArgStr[1],MemInt2,&OK); else AdrBank=0;
	END
       if (OK)
	BEGIN
	 CodeLen=4;
         BAsmCode[0]=(Memo("JMPS")) ? 0xfa : 0xda;
	 BAsmCode[1]=AdrBank;
	 BAsmCode[2]=Lo(AdrWord); BAsmCode[3]=Hi(AdrWord);
	END
      END
     return;
    END

   if ((Memo("JMPI")) OR (Memo("CALLI")))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       Cond=(ArgCnt==1) ? TrueCond : DecodeCondition(ArgStr[1]);
       if (Cond>=ConditionCount) WrXError(1360,ArgStr[1]);
       else
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModIReg,True,False);
	 switch (AdrType)
          BEGIN
	   case ModIReg:
	    CodeLen=2;
            BAsmCode[0]=(Memo("JMPI")) ? 0x9c : 0xab;
	    BAsmCode[1]=AdrMode+(Conditions[Cond].Code << 4);
	    break;
	  END
	END
      END
     return;
    END

   for (z=0; z<BJmpOrderCount; z++)
    if (Memo(BJmpOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (DecodeBitAddr(ArgStr[1],&BAdr1,&BOfs1,False))
       BEGIN
	AdrLong=EvalIntExpression(ArgStr[2],MemInt,&OK)-(EProgCounter()+4);
	if (OK)
	 if ((AdrLong&1)==1) WrError(1375);
	 else if ((NOT SymbolQuestionable) AND ((AdrLong<-256) OR (AdrLong>254))) WrError(1370);
	 else
	  BEGIN
	   CodeLen=4; BAsmCode[0]=0x8a+(z << 4);
	   BAsmCode[1]=BAdr1;
	   BAsmCode[2]=(AdrLong/2) & 0xff;
	   BAsmCode[3]=BOfs1 << 4;
	  END
       END
      return;
     END

   if (Memo("PCALL"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModMReg,False,False);
       switch (AdrType)
        BEGIN
         case ModMReg:
	  BAsmCode[1]=AdrVals[0];
	  DecodeAdr(ArgStr[2],MModAbs,True,False);
	  switch (AdrType)
           BEGIN
	    case ModAbs:
	     CodeLen=4; BAsmCode[0]=0xe2; memcpy(BAsmCode+2,AdrVals,2);
	     break;
	   END
	  break;
        END
      END
     return;
    END

   if (Memo("RETP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModMReg,False,False);
       switch (AdrType)
        BEGIN
         case ModMReg:
	  BAsmCode[1]=AdrVals[0]; BAsmCode[0]=0xeb; CodeLen=2;
	  if (SPChanged) WrXError(200,RegNames[5]);
	  break;
        END
      END
     return;
    END

   if (Memo("TRAP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*ArgStr[1]!='#') WrError(1350);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[1]+1,UInt7,&OK) << 1;
       if (OK)
	BEGIN
	 BAsmCode[0]=0x9b; CodeLen=2;
	END
      END
     return;
    END

   /* spezielle Steuerbefehle */

   if (Memo("ATOMIC"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80C167) WrError(1500);
     else if (DecodePref(ArgStr[1],&HReg))
      BEGIN
       CodeLen=2; BAsmCode[0]=0xd1; BAsmCode[1]=HReg << 4;
      END
     return;
    END

   if (Memo("EXTR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU80C167) WrError(1500);
     else if (DecodePref(ArgStr[1],&HReg))
      BEGIN
       CodeLen=2; BAsmCode[0]=0xd1; BAsmCode[1]=0x80+(HReg << 4);
       ExtCounter=HReg+1; ExtSFRs=True;
      END
     return;
    END

   if ((Memo("EXTP")) OR (Memo("EXTPR")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80C167) WrError(1500);
     else if (DecodePref(ArgStr[2],&HReg))
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModImm,False,False);
       switch (AdrType)
        BEGIN
         case ModReg:
	  CodeLen=2; BAsmCode[0]=0xdc; BAsmCode[1]=0x40+(HReg << 4)+AdrMode;
	  if (Memo("EXTPR")) BAsmCode[1]+=0x80;
	  ExtCounter=HReg+1; MemMode=MemModeZeroPage;
	  break;
         case ModImm:
	  CodeLen=4; BAsmCode[0]=0xd7; BAsmCode[1]=0x40+(HReg << 4);
	  if (Memo("EXTPR")) BAsmCode[1]+=0x80;
	  BAsmCode[2]=(WordVal() >> 2) & 0x7f; BAsmCode[3]=WordVal() & 3;
	  ExtCounter=HReg+1; MemMode=MemModeFixedPage; MemPage=WordVal() & 0x3ff;
	  break;
        END
      END
     return;
    END

   if ((Memo("EXTS")) OR (Memo("EXTSR")))
    BEGIN
     OpSize=0;
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPU80C167) WrError(1500);
     else if (DecodePref(ArgStr[2],&HReg))
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModImm,False,False);
       switch (AdrType)
        BEGIN
         case ModReg:
	  CodeLen=2; BAsmCode[0]=0xdc; BAsmCode[1]=0x00+(HReg << 4)+AdrMode;
	  if (Memo("EXTSR")) BAsmCode[1]+=0x80;
	  ExtCounter=HReg+1; MemMode=MemModeNoCheck;
	  break;
         case ModImm:
	  CodeLen=4; BAsmCode[0]=0xd7; BAsmCode[1]=0x00+(HReg << 4);
	  if (Memo("EXTSR")) BAsmCode[1]+=0x80;
	  BAsmCode[2]=AdrVals[0]; BAsmCode[3]=0;
	  ExtCounter=HReg+1; MemMode=MemModeFixedBank; MemPage=AdrVals[0];
	  break;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static void InitCode_166(void)
BEGIN
   Integer z;

   SaveInitProc();
   for (z=0; z<DPPCount; z++)
    BEGIN
     DPPAssumes[z]=z; N_DPPChanged[z]=False;
    END
   N_CPChanged=False; N_SPChanged=False;

   MemMode=MemModeStd; ExtSFRs=False; ExtCounter=-1;
END

	static Boolean ChkPC_166(void)
BEGIN
   if (ActPC==SegCode) return (ProgCounter()<0x40000);
   else return False;
END

	static Boolean IsDef_166(void)
BEGIN
   return (Memo("BIT"));
END

	static void SwitchFrom_166(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_166(void)
BEGIN
   Byte z;

   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;
   OpSize=1;

   PCSymbol="$"; HeaderID=0x4c; NOPCode=0xcc00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_166; ChkPC=ChkPC_166; IsDef=IsDef_166;
   SwitchFrom=SwitchFrom_166;

   if (MomCPU==CPU80C166)
    BEGIN
     MemInt=UInt18; MemInt2=UInt2; ASSUME166s[0].Max=15;
    END
   else
    BEGIN
     MemInt=UInt24; MemInt2=UInt8; ASSUME166s[0].Max=1023;
    END
   for (z=1; z<4; z++) ASSUME166s[z].Max=ASSUME166s[0].Max;

   InitFields();
END

	void code166_init(void)
BEGIN
   CPU80C166=AddCPU("80C166",SwitchTo_166);
   CPU80C167=AddCPU("80C167",SwitchTo_166);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_166;
END
