/* codexa.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator Philips XA                                               */
/*                                                                           */
/* Historie: 25.10.1996 Grundsteinlegung                                     */
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
#include "codevars.h"

/*-------------------------------------------------------------------------*/

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModMem 1
#define MModMem (1 << ModMem)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModAbs 3
#define MModAbs (1 << ModAbs)

#define FixedOrderCnt 5
#define JBitOrderCnt 3
#define ALUOrderCnt 8
#define RegOrderCnt 4
#define ShiftOrderCount 4
#define RotateOrderCount 4
#define RelOrderCount 17
#define StackOrderCount 4

typedef struct
         {   
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {   
          char *Name;
          Byte SizeMask;
          Byte Code;
         } RegOrder;

static CPUVar CPUXAG1,CPUXAG2,CPUXAG3;

static FixedOrder *FixedOrders;
static FixedOrder *JBitOrders;
static FixedOrder *StackOrders;
static char **ALUOrders;
static RegOrder *RegOrders;
static char **ShiftOrders;
static FixedOrder *RotateOrders;
static FixedOrder *RelOrders;

static LongInt Reg_DS;
static SimpProc SaveInitProc;

static ShortInt AdrMode;
static Byte AdrPart,MemPart;
static Byte AdrVals[4];
static ShortInt OpSize;

/*-------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddJBit(char *NName, Word NCode)
BEGIN
   if (InstrZ>=JBitOrderCnt) exit(255);
   JBitOrders[InstrZ].Name=NName;
   JBitOrders[InstrZ++].Code=NCode;
END

        static void AddStack(char *NName, Word NCode)
BEGIN
   if (InstrZ>=StackOrderCount) exit(255);
   StackOrders[InstrZ].Name=NName;
   StackOrders[InstrZ++].Code=NCode;
END

        static void AddReg(char *NName, Byte NMask, Byte NCode)
BEGIN
   if (InstrZ>=RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name=NName;
   RegOrders[InstrZ].Code=NCode;
   RegOrders[InstrZ++].SizeMask=NMask;
END

        static void AddRotate(char *NName, Word NCode)
BEGIN
   if (InstrZ>=RotateOrderCount) exit(255);
   RotateOrders[InstrZ].Name=NName;
   RotateOrders[InstrZ++].Code=NCode;
END

        static void AddRel(char *NName, Word NCode)
BEGIN
   if (InstrZ>=RelOrderCount) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("NOP"  ,0x0000);
   AddFixed("RET"  ,0xd680);
   AddFixed("RETI" ,0xd690);
   AddFixed("BKPT" ,0x00ff);
   AddFixed("RESET",0xd610);

   JBitOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*JBitOrderCnt); InstrZ=0;
   AddJBit("JB"  ,0x80);
   AddJBit("JBC" ,0xc0);
   AddJBit("JNB" ,0xa0);

   StackOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*StackOrderCount); InstrZ=0;
   AddStack("POP"  ,0x1027);
   AddStack("POPU" ,0x0037);
   AddStack("PUSH" ,0x3007);
   AddStack("PUSHU",0x2017);

   ALUOrders=(char **) malloc(sizeof(char *)*ALUOrderCnt); InstrZ=0;
   ALUOrders[InstrZ++]="ADD";  ALUOrders[InstrZ++]="ADDC";
   ALUOrders[InstrZ++]="SUB";  ALUOrders[InstrZ++]="SUBB";
   ALUOrders[InstrZ++]="CMP";  ALUOrders[InstrZ++]="AND";
   ALUOrders[InstrZ++]="OR";   ALUOrders[InstrZ++]="XOR";

   RegOrders=(RegOrder *) malloc(sizeof(RegOrder)*RegOrderCnt); InstrZ=0;
   AddReg("NEG" ,3,0x0b);
   AddReg("CPL" ,3,0x0a);
   AddReg("SEXT",3,0x09);
   AddReg("DA"  ,1,0x08);

   ShiftOrders=(char **) malloc(sizeof(char *)*ShiftOrderCount); InstrZ=0;
   ShiftOrders[InstrZ++]="LSR"; ShiftOrders[InstrZ++]="ASL";
   ShiftOrders[InstrZ++]="ASR"; ShiftOrders[InstrZ++]="NORM";

   RotateOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RotateOrderCount); InstrZ=0;
   AddRotate("RR" ,0xb0); AddRotate("RL" ,0xd3);
   AddRotate("RRC",0xb7); AddRotate("RLC",0xd7);

   RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCount); InstrZ=0;
   AddRel("BCC",0xf0); AddRel("BCS",0xf1); AddRel("BNE",0xf2);
   AddRel("BEQ",0xf3); AddRel("BNV",0xf4); AddRel("BOV",0xf5);
   AddRel("BPL",0xf6); AddRel("BMI",0xf7); AddRel("BG" ,0xf8);
   AddRel("BL" ,0xf9); AddRel("BGE",0xfa); AddRel("BLT",0xfb);
   AddRel("BGT",0xfc); AddRel("BLE",0xfd); AddRel("BR" ,0xfe);
   AddRel("JZ" ,0xec); AddRel("JNZ",0xee);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(JBitOrders);
   free(StackOrders);
   free(ALUOrders);
   free(RegOrders);
   free(ShiftOrders);
   free(RotateOrders);
   free(RelOrders);
END

/*-------------------------------------------------------------------------*/

	static void SetOpSize(ShortInt NSize)
BEGIN
   if (OpSize==-1) OpSize=NSize;
   else if (OpSize!=NSize)
    BEGIN
     AdrMode=ModNone; AdrCnt=0; WrError(1131);
    END
END

	static Boolean DecodeReg(char *Asc, ShortInt *NSize, Byte *Erg)
BEGIN
   if (strcasecmp(Asc,"SP")==0)
    BEGIN
     *Erg=7; *NSize=1; return True;
    END
   else if ((strlen(Asc)>=2) AND (toupper(*Asc)=='R') AND (Asc[1]>='0') AND (Asc[1]<='7'))
    if (strlen(Asc)==2)
     BEGIN
      *Erg=Asc[1]-'0';
      if (OpSize==2)
       BEGIN
        if ((*Erg&1)==1)
         BEGIN
          WrError(1760); (*Erg)--;
         END
        *NSize=2;
        return True;
       END
      else
       BEGIN
        *NSize=1;
        return True;
       END
     END
    else if ((strlen(Asc)==3) AND (toupper(Asc[2])=='L'))
     BEGIN
      *Erg=(Asc[1]-'0') << 1; *NSize=0; return True;
     END
    else if ((strlen(Asc)==3) AND (toupper(Asc[2])=='H'))
     BEGIN
      *Erg=((Asc[1]-'0') << 1)+1; *NSize=0; return True;
     END
    else return False;
   return False;
END

	static void ChkAdr(Word Mask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     WrError(1350); AdrMode=ModNone; AdrCnt=0;
    END
END

	static void DecodeAdr(char *Asc, Word Mask)
BEGIN
   ShortInt NSize;
   LongInt DispAcc,DispPart,AdrLong;
   Boolean FirstFlag,NegFlag,NextFlag,ErrFlag,OK;
   char *PPos,*MPos;
   Word AdrInt;
   Byte Reg;
   String Part;

   AdrMode=ModNone; AdrCnt=0; KillBlanks(Asc);

   if (DecodeReg(Asc,&NSize,&AdrPart))
    BEGIN
     if ((Mask & MModReg)!=0)
      BEGIN
       AdrMode=ModReg; SetOpSize(NSize);
      END
     else
      BEGIN
       AdrMode=ModMem; MemPart=1; SetOpSize(NSize);
      END
     ChkAdr(Mask); return;
    END

   if (*Asc=='#')
    BEGIN
     switch (OpSize)
      BEGIN
       case -4:
        AdrVals[0]=EvalIntExpression(Asc+1,UInt5,&OK);
        if (OK)
         BEGIN
          AdrCnt=1; AdrMode=ModImm;
         END
        break;
       case -3:
        AdrVals[0]=EvalIntExpression(Asc+1,SInt4,&OK);
        if (OK)
         BEGIN
          AdrCnt=1; AdrMode=ModImm;
         END
        break;
       case -2:
        AdrVals[0]=EvalIntExpression(Asc+1,UInt4,&OK);
        if (OK)
         BEGIN
          AdrCnt=1; AdrMode=ModImm;
         END
        break;
       case -1:
        WrError(1132);
        break;
       case 0:
        AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
        if (OK)
         BEGIN
          AdrCnt=1; AdrMode=ModImm;
         END
        break;
       case 1:
        AdrInt=EvalIntExpression(Asc+1,Int16,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=Hi(AdrInt); AdrVals[1]=Lo(AdrInt);
          AdrCnt=2; AdrMode=ModImm;
         END
        break;
       case 2:
        AdrLong=EvalIntExpression(Asc+1,Int32,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=(AdrLong >> 24) & 0xff;
          AdrVals[1]=(AdrLong >> 16) & 0xff;
          AdrVals[2]=(AdrLong >> 8) & 0xff;
          AdrVals[3]=AdrLong & 0xff;
          AdrCnt=4; AdrMode=ModImm;
         END
        break;
      END
     ChkAdr(Mask); return;
    END

   if ((*Asc=='[') AND (Asc[strlen(Asc)-1]==']'))
    BEGIN
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
     if (Asc[strlen(Asc)-1]=='+')
      BEGIN
       Asc[strlen(Asc)-1]='\0';
       if (NOT DecodeReg(Asc,&NSize,&AdrPart)) WrXError(1445,Asc);
       else if (NSize!=1) WrError(1350);
       else
        BEGIN
         AdrMode=ModMem; MemPart=3;
        END
      END
     else
      BEGIN
       FirstFlag=False; ErrFlag=False;
       DispAcc=0; AdrPart=0xff; NegFlag=False;
       while ((*Asc!='\0') AND (NOT ErrFlag))
        BEGIN
         PPos=QuotPos(Asc,'+'); MPos=QuotPos(Asc,'-');
         if (PPos==Nil) PPos=MPos;
         else if ((MPos!=Nil) AND (PPos>MPos)) PPos=MPos;
         NextFlag=((PPos!=Nil) AND (*PPos=='-'));
         if (PPos==Nil)
          BEGIN
           strmaxcpy(Part,Asc,255); *Asc='\0';
          END
         else
          BEGIN
           *PPos='\0'; strmaxcpy(Part,Asc,255); strcpy(Asc,PPos+1);
          END
         if (DecodeReg(Part,&NSize,&Reg))
          if ((NSize!=1) OR (AdrPart!=0xff) OR (NegFlag))
           BEGIN
            WrError(1350); ErrFlag=True;
           END
          else AdrPart=Reg;
         else
          BEGIN
           FirstPassUnknown=False;
           DispPart=EvalIntExpression(Part,Int32,&ErrFlag);
           ErrFlag=NOT ErrFlag;
           if (NOT ErrFlag)
            BEGIN
             FirstFlag=FirstFlag OR FirstPassUnknown;
             if (NegFlag) DispAcc-=DispPart;
             else DispAcc+=DispPart;
            END
          END
         NegFlag=NextFlag;
        END
       if (FirstFlag) DispAcc&=0x7fff;
       if (AdrPart==0xff) WrError(1350);
       else if (DispAcc==0)
        BEGIN
         AdrMode=ModMem; MemPart=2;
        END
       else if ((DispAcc>=-128) AND (DispAcc<127))
        BEGIN
         AdrMode=ModMem; MemPart=4;
         AdrVals[0]=DispAcc & 0xff; AdrCnt=1;
        END
       else if (ChkRange(DispAcc,-0x8000l,0x7fffl))
        BEGIN
         AdrMode=ModMem; MemPart=5;
         AdrVals[0]=(DispAcc >> 8) & 0xff;
         AdrVals[1]=DispAcc & 0xff;
         AdrCnt=2;
        END
      END
     ChkAdr(Mask); return;
    END

   FirstPassUnknown=False;
   AdrLong=EvalIntExpression(Asc,UInt24,&OK);
   if (OK)
    BEGIN
     if (FirstPassUnknown)
      BEGIN
       if ((Mask & MModAbs)==0) AdrLong&=0x3ff;
      END
     if ((AdrLong & 0xffff)>0x7ff) WrError(1925);
     else if ((AdrLong & 0xffff)<=0x3ff)
      BEGIN
       if ((AdrLong >> 16)!=Reg_DS) WrError(110);
       ChkSpace(SegData);
       AdrMode=ModMem; MemPart=6;
       AdrPart=Hi(AdrLong); AdrVals[0]=Lo(AdrLong);
       AdrCnt=1;
      END
     else if (AdrLong>0x7ff) WrError(1925);
     else
      BEGIN
       ChkSpace(SegIO);
       AdrMode=ModMem; MemPart=6;
       AdrPart=Hi(AdrLong); AdrVals[0]=Lo(AdrLong);
       AdrCnt=1;
      END
    END

   ChkAdr(Mask);
END

	static Boolean DecodeBitAddr(char *Asc, LongInt *Erg)
BEGIN
   char *p;
   Byte BPos,Reg;
   ShortInt Size,Res;
   LongInt AdrLong;
   Boolean OK;

   p=RQuotPos(Asc,'.'); Res=0;
   if (p==Nil)
    BEGIN
     FirstPassUnknown=False;
     AdrLong=EvalIntExpression(Asc,UInt24,&OK);
     if (FirstPassUnknown) AdrLong&=0x3ff;
     *Erg=AdrLong; Res=1;
    END
   else
    BEGIN
     FirstPassUnknown=False; *p='\0';
     BPos=EvalIntExpression(p+1,UInt4,&OK);
     if (FirstPassUnknown) BPos&=7;
     if (OK)
      BEGIN
       if (DecodeReg(Asc,&Size,&Reg))
        if ((Size==0) AND (BPos>7)) WrError(1320);
        else
         BEGIN
          if (Size==0) *Erg=(Reg << 3)+BPos;
          else *Erg=(Reg << 4)+BPos;
          Res=1;
         END
       else if (BPos>7) WrError(1320);
       else
        BEGIN
         FirstPassUnknown=False;
         AdrLong=EvalIntExpression(Asc,UInt24,&OK);
         if ((TypeFlag & (1 << SegIO))!=0)
          BEGIN
           ChkSpace(SegIO);
           if (FirstPassUnknown) AdrLong=(AdrLong & 0x3f) | 0x400;
           if (ChkRange(AdrLong,0x400,0x43f))
            BEGIN
             *Erg=0x200+((AdrLong & 0x3f) << 3)+BPos;
             Res=1;
            END
           else Res=(-1);
          END
         else
	  BEGIN
           ChkSpace(SegData);
	   if (FirstPassUnknown) AdrLong=(AdrLong & 0x00ff003f) | 0x20;
	   if (ChkRange(AdrLong & 0xff,0x20,0x3f))
            BEGIN
             *Erg=0x100+((AdrLong & 0x1f) << 3)+BPos+(AdrLong & 0xff0000);
             Res=1;
            END
           else Res=(-1);
          END
        END
      END
     *p='.';
    END
   if (Res==0) WrError(1350);
   return (Res==1);
END

	static void ChkBitPage(LongInt Adr)
BEGIN
   if ((Adr >> 16)!=Reg_DS) WrError(110);
END

/*-------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
#define ASSUMEXACount 1
static ASSUMERec ASSUMEXAs[ASSUMEXACount]=
             {{"DS", &Reg_DS, 0, 0xff, 0x100}};
#define ONOFFXACount 1
static ONOFFRec ONOFFXAs[ONOFFXACount]=
             {{"SUPMODE", &SupAllowed, SupAllowedName}};

   LongInt BAdr;

   if (Memo("PORT"))
    BEGIN
     CodeEquate(SegIO,0x400,0x7ff);
     return True;
    END

   if (Memo("BIT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (DecodeBitAddr(ArgStr[1],&BAdr))
      BEGIN
       EnterIntSymbol(LabPart,BAdr,SegNone,False);
       switch ((BAdr & 0x3ff) >> 8)
        BEGIN
         case 0:
          sprintf(ListLine,"=R%d.%d",(BAdr >> 4) & 15,BAdr & 15);
          break;
         case 1:
          sprintf(ListLine,"=%x:%x.%d",(BAdr >> 16) & 255,(BAdr & 0x1f8) >> 3,BAdr & 7);
          break;
         default:
          sprintf(ListLine,"=S:%x.%d",((BAdr >> 3) & 0x3f)+0x400,BAdr & 7);
          break;
        END
      END
     return True;
    END

   if (CodeONOFF(ONOFFXAs,ONOFFXACount)) return True;

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUMEXAs,ASSUMEXACount);
     return True;
    END

   return False;
END

	static Boolean IsRealDef(void)
BEGIN
   return ((Memo("PORT")) OR (Memo("BIT")));
END

	static void ForceAlign(void)
BEGIN
   if ((EProgCounter()&1)==1)
    BEGIN
     BAsmCode[0]=NOPCode; CodeLen=1;
    END
END

	static void MakeCode_XA(void)
BEGIN
   Byte HReg,HMem,HCnt,HPart;
   Byte HVals[3];
   Integer z,i;
   Word Mask;
   LongInt AdrLong;
   Boolean OK;

   CodeLen=0; DontPrint=False; OpSize=(-1);

   /* Operandengroesse */

   if (*AttrPart!='\0')
    switch (toupper(*AttrPart))
     BEGIN
      case 'B': SetOpSize(0); break;
      case 'W': SetOpSize(1); break;
      case 'D': SetOpSize(2); break;
      default : WrError(1107); return;
     END

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* Labels muessen auf geraden Adressen liegen */

   if ((ActPC==SegCode) AND (NOT IsRealDef()) AND (*LabPart!='\0'))
    BEGIN
     ForceAlign();
     EnterIntSymbol(LabPart,EProgCounter()+CodeLen,ActPC,False);
    END

   if (DecodeMoto16Pseudo(OpSize,False)) return;
   if (DecodeIntelPseudo(False)) return;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Anweisungen ohne Operanden */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        if (Hi(FixedOrders[z].Code)!=0) BAsmCode[CodeLen++]=Hi(FixedOrders[z].Code);
        BAsmCode[CodeLen++]=Lo(FixedOrders[z].Code);
        if ((Memo("RETI")) AND (NOT SupAllowed)) WrError(50);
       END
      return;
     END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"C")==0)
      BEGIN
       if (DecodeBitAddr(ArgStr[2],&AdrLong))
        if (*AttrPart!='\0') WrError(1100);
        else
         BEGIN
          ChkBitPage(AdrLong);
          BAsmCode[CodeLen++]=0x08;
          BAsmCode[CodeLen++]=0x20+Hi(AdrLong);
          BAsmCode[CodeLen++]=Lo(AdrLong);
         END
      END
     else if (strcasecmp(ArgStr[2],"C")==0)
      BEGIN
       if (DecodeBitAddr(ArgStr[1],&AdrLong))
        if (*AttrPart!='\0') WrError(1100);
        else
         BEGIN
          ChkBitPage(AdrLong);
          BAsmCode[CodeLen++]=0x08;
          BAsmCode[CodeLen++]=0x30+Hi(AdrLong);
          BAsmCode[CodeLen++]=Lo(AdrLong);
         END
      END
     else if (strcasecmp(ArgStr[1],"USP")==0)
      BEGIN
       SetOpSize(1);
       DecodeAdr(ArgStr[2],MModReg);
       if (AdrMode==ModReg)
        BEGIN
         BAsmCode[CodeLen++]=0x98;
         BAsmCode[CodeLen++]=(AdrPart << 4)+0x0f;
        END
      END
     else if (strcasecmp(ArgStr[2],"USP")==0)
      BEGIN
       SetOpSize(1);
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode==ModReg)
        BEGIN
         BAsmCode[CodeLen++]=0x90;
         BAsmCode[CodeLen++]=(AdrPart << 4)+0x0f;
        END
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModMem);
       switch (AdrMode)
        BEGIN
         case ModReg:
          if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
          else
           BEGIN
            HReg=AdrPart;
            DecodeAdr(ArgStr[2],MModMem+MModImm);
            switch (AdrMode)
             BEGIN
              case ModMem:
               BAsmCode[CodeLen++]=0x80+(OpSize << 3)+MemPart;
               BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
               memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
               CodeLen+=AdrCnt;
               if ((MemPart==3) AND ((HReg >> (1-OpSize))==AdrPart)) WrError(140);
               break;
              case ModImm:
               BAsmCode[CodeLen++]=0x91+(OpSize << 3);
               BAsmCode[CodeLen++]=0x08+(HReg << 4);
               memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
               CodeLen+=AdrCnt;
               break;
             END
           END
          break;
         case ModMem:
          memcpy(HVals,AdrVals,AdrCnt); HCnt=AdrCnt; HPart=MemPart; HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg+MModMem+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg:
             if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
             else
              BEGIN
               BAsmCode[CodeLen++]=0x80+(OpSize << 3)+HPart;
               BAsmCode[CodeLen++]=(AdrPart << 4)+0x08+HReg;
               memcpy(BAsmCode+CodeLen,HVals,HCnt);
               CodeLen+=HCnt;
               if ((HPart==3) AND ((AdrPart >> (1-OpSize))==HReg)) WrError(140);
              END
             break;
            case ModMem:
             if (OpSize==-1) WrError(1132);
             else if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
             else if ((HPart==6) AND (MemPart==6))
              BEGIN
               BAsmCode[CodeLen++]=0x97+(OpSize << 3);
               BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
               BAsmCode[CodeLen++]=HVals[0];
               BAsmCode[CodeLen++]=AdrVals[0];
              END
             else if ((HPart==6) AND (MemPart==2))
              BEGIN
               BAsmCode[CodeLen++]=0xa0+(OpSize << 3);
               BAsmCode[CodeLen++]=0x80+(AdrPart << 4)+HReg;
               BAsmCode[CodeLen++]=HVals[0];
              END
             else if ((HPart==2) AND (MemPart==6))
              BEGIN
               BAsmCode[CodeLen++]=0xa0+(OpSize << 3);
               BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
               BAsmCode[CodeLen++]=AdrVals[0];
              END
             else if ((HPart==3) AND (MemPart==3))
              BEGIN
               BAsmCode[CodeLen++]=0x90+(OpSize << 3);
               BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
               if (HReg==AdrPart) WrError(140);
              END
             else WrError(1350);
             break;
            case ModImm:
             if (OpSize==-1) WrError(1132);
             else if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
             else
              BEGIN
               BAsmCode[CodeLen++]=0x90+(OpSize << 3)+HPart;
               BAsmCode[CodeLen++]=0x08+(HReg << 4);
               memcpy(BAsmCode+CodeLen,HVals,HCnt);
               memcpy(BAsmCode+CodeLen+HCnt,AdrVals,AdrCnt);
               CodeLen+=HCnt+AdrCnt;
              END
             break;
           END
          break;
        END
      END
     return;
    END

   if (Memo("MOVC"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if ((*AttrPart=='\0') AND (strcasecmp(ArgStr[1],"A")==0)) OpSize=0;
       if (strcasecmp(ArgStr[2],"[A+DPTR]")==0)
        if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
        else if (OpSize!=0) WrError(1130);
        else
         BEGIN
          BAsmCode[CodeLen++]=0x90;
          BAsmCode[CodeLen++]=0x4e;
         END
       else if (strcasecmp(ArgStr[2],"[A+PC]")==0)
        if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
        else if (OpSize!=0) WrError(1130);
        else
         BEGIN
          BAsmCode[CodeLen++]=0x90;
          BAsmCode[CodeLen++]=0x4c;
         END
       else
        BEGIN
         DecodeAdr(ArgStr[1],MModReg);
         if (AdrMode!=ModNone)
          if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
          else
           BEGIN
            HReg=AdrPart;
            DecodeAdr(ArgStr[2],MModMem);
            if (AdrMode!=ModNone)
             if (MemPart!=3) WrError(1350);
             else
              BEGIN
               BAsmCode[CodeLen++]=0x80+(OpSize << 3);
               BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
               if ((MemPart==3) AND ((HReg >> (1-OpSize))==AdrPart)) WrError(140);
              END
           END
        END
      END
     return;
    END

   if (Memo("MOVX"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       if (AdrMode==ModMem)
        switch (MemPart)
         BEGIN
          case 1:
           if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
           else
            BEGIN
             HReg=AdrPart; DecodeAdr(ArgStr[2],MModMem);
             if (AdrMode==ModMem)
              if (MemPart!=2) WrError(1350);
              else
               BEGIN
                BAsmCode[CodeLen++]=0xa7+(OpSize << 3);
                BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
               END
            END
           break;
          case 2:
           HReg=AdrPart; DecodeAdr(ArgStr[2],MModReg);
           if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
           else
            BEGIN
             BAsmCode[CodeLen++]=0xa7+(OpSize << 3);
             BAsmCode[CodeLen++]=0x08+(AdrPart << 4)+HReg;
            END
           break;
          default:
           WrError(1350);
         END
      END
     return;
    END

   for  (z=0; z<StackOrderCount; z++)
    if (Memo(StackOrders[z].Name))
     BEGIN
      if (ArgCnt<1) WrError(1110);
      else
       BEGIN
        HReg=0xff; OK=True; Mask=0;
        for (i=1; i<=ArgCnt; i++)
         if (OK)
          BEGIN
           DecodeAdr(ArgStr[i],MModMem);
           if (AdrMode==ModNone) OK=False;
           else switch (MemPart)
            BEGIN
             case 1:
              if (HReg==0)
               BEGIN
                WrError(1350); OK=False;
               END
              else
               BEGIN
                HReg=1; Mask|=(1 << AdrPart);
               END
              break;
             case 6:
              if (HReg!=0xff)
               BEGIN
                WrError(1350); OK=False;
               END
              else HReg=0;
              break;
             default:
              WrError(1350); OK=False;
            END
          END
        if (OK)
         if (OpSize==-1) WrError(1132);
         else if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
         else if (HReg==0)
          BEGIN
           BAsmCode[CodeLen++]=0x87+(OpSize << 3);
           BAsmCode[CodeLen++]=Hi(StackOrders[z].Code)+AdrPart;
           BAsmCode[CodeLen++]=AdrVals[0];
          END
         else if (z<2)  /* POP: obere Register zuerst */
          BEGIN
           if (Hi(Mask)!=0)
            BEGIN
             BAsmCode[CodeLen++]=Lo(StackOrders[z].Code)+(OpSize << 3)+0x40;
             BAsmCode[CodeLen++]=Hi(Mask);
            END
           if (Lo(Mask)!=0)
            BEGIN
             BAsmCode[CodeLen++]=Lo(StackOrders[z].Code)+(OpSize << 3);
             BAsmCode[CodeLen++]=Lo(Mask);
            END
           if ((OpSize==1) AND (Memo("POP")) AND ((Mask & 0x80)!=0)) WrError(140);
          END
         else              /* PUSH: untere Register zuerst */
          BEGIN
           if (Lo(Mask)!=0)
            BEGIN
             BAsmCode[CodeLen++]=Lo(StackOrders[z].Code)+(OpSize << 3);
             BAsmCode[CodeLen++]=Lo(Mask);
            END
           if (Hi(Mask)!=0)
            BEGIN
             BAsmCode[CodeLen++]=Lo(StackOrders[z].Code)+(OpSize << 3)+0x40;
             BAsmCode[CodeLen++]=Hi(Mask);
            END
          END
       END
      return;
     END

   if (Memo("XCH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       if (AdrMode==ModMem)
        switch (MemPart)
         BEGIN
          case 1:
           HReg=AdrPart; DecodeAdr(ArgStr[2],MModMem);
           if (AdrMode==ModMem)
            if ((OpSize!=1) AND (OpSize!=0)) WrError(1130);
            else switch (MemPart)
             BEGIN
              case 1:
               BAsmCode[CodeLen++]=0x60+(OpSize << 3);
               BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
               if (HReg==AdrPart) WrError(140);
               break;
              case 2:
               BAsmCode[CodeLen++]=0x50+(OpSize << 3);
               BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
               break;
              case 6:
               BAsmCode[CodeLen++]=0xa0+(OpSize << 3);
               BAsmCode[CodeLen++]=0x08+(HReg << 4)+AdrPart;
               BAsmCode[CodeLen++]=AdrVals[0];
               break;
              default:
               WrError(1350);
             END
           break;
          case 2:
           HReg=AdrPart;
           DecodeAdr(ArgStr[2],MModReg);
           if (AdrMode==ModReg)
            if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
            else
             BEGIN
              BAsmCode[CodeLen++]=0x50+(OpSize << 3);
              BAsmCode[CodeLen++]=(AdrPart << 4)+HReg;
             END
           break;
          case 6:
           HPart=AdrPart; HVals[0]=AdrVals[0];
           DecodeAdr(ArgStr[2],MModReg);
           if (AdrMode==ModReg)
            if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
            else
             BEGIN
              BAsmCode[CodeLen++]=0xa0+(OpSize << 3);
              BAsmCode[CodeLen++]=0x08+(AdrPart << 4)+HPart;
              BAsmCode[CodeLen++]=HVals[0];
             END
           break;
          default:
           WrError(1350);
         END
      END
     return;
    END

   /* Arithmetik */

   for (z=0; z<ALUOrderCnt; z++)
    if (Memo(ALUOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg+MModMem);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if (OpSize>=2) WrError(1130);
           else if (OpSize==-1) WrError(1132);
           else
            BEGIN
             HReg=AdrPart;
             DecodeAdr(ArgStr[2],MModMem+MModImm);
             switch (AdrMode)
              BEGIN
               case ModMem:
                BAsmCode[CodeLen++]=(z << 4)+(OpSize << 3)+MemPart;
                BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
                memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
                CodeLen+=AdrCnt;
                if ((MemPart==3) AND ((HReg >> (1-OpSize))==AdrPart)) WrError(140);
                break;
               case ModImm:
                BAsmCode[CodeLen++]=0x91+(OpSize << 3);
                BAsmCode[CodeLen++]=(HReg << 4)+z;
                memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
                CodeLen+=AdrCnt;
                break;
              END
            END
           break;
          case ModMem:
           HReg=AdrPart; HMem=MemPart; HCnt=AdrCnt;
           memcpy(HVals,AdrVals,AdrCnt);
           DecodeAdr(ArgStr[2],MModReg+MModImm);
           switch (AdrMode)
            BEGIN
             case ModReg:
              if (OpSize==2) WrError(1130);
              else if (OpSize==-1) WrError(1132);
              else
               BEGIN
                BAsmCode[CodeLen++]=(z << 4)+(OpSize << 3)+HMem;
                BAsmCode[CodeLen++]=(AdrPart << 4)+8+HReg;
                memcpy(BAsmCode+CodeLen,HVals,HCnt);
                CodeLen+=HCnt;
                if ((HMem==3) AND ((AdrPart >> (1-OpSize))==HReg)) WrError(140);
               END
              break;
             case ModImm:
              if (OpSize==2) WrError(1130);
              else if (OpSize==-1) WrError(1132);
              else
               BEGIN
                BAsmCode[CodeLen++]=0x90+HMem+(OpSize << 3);
                BAsmCode[CodeLen++]=(HReg << 4)+z;
                memcpy(BAsmCode+CodeLen,HVals,HCnt);
                memcpy(BAsmCode+CodeLen+HCnt,AdrVals,AdrCnt);
                CodeLen+=AdrCnt+HCnt;
               END
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
        DecodeAdr(ArgStr[1],MModReg);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if ((RegOrders[z].SizeMask & (1 << OpSize))==0) WrError(1130);
           else
            BEGIN
             BAsmCode[CodeLen++]=0x90+(OpSize << 3);
             BAsmCode[CodeLen++]=(AdrPart << 4)+RegOrders[z].Code;
            END
           break;
         END
       END
      return;
     END

   if ((Memo("ADDS")) OR (Memo("MOVS")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       HMem=OpSize; OpSize=(-3);
       DecodeAdr(ArgStr[2],MModImm);
       switch (AdrMode)
        BEGIN
         case ModImm:
          HReg=AdrVals[0]; OpSize=HMem;
          DecodeAdr(ArgStr[1],MModMem);
          switch (AdrMode)
           BEGIN
            case ModMem:
             if (OpSize==2) WrError(1130);
             else if (OpSize==-1) WrError(1132);
             else
              BEGIN
               BAsmCode[CodeLen++]=0xa0+(Ord(Memo("MOVS")) << 4)+(OpSize << 3)+MemPart;
               BAsmCode[CodeLen++]=(AdrPart << 4)+(HReg & 0x0f);
               memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
               CodeLen+=AdrCnt;
              END
             break;
           END
          break;
        END 
      END
     return;
    END

   if (Memo("DIV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode==ModReg)
        if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
        else
         BEGIN
          HReg=AdrPart; OpSize--; DecodeAdr(ArgStr[2],MModReg+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg:
             BAsmCode[CodeLen++]=0xe7+(OpSize << 3);
             BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
             break;
            case ModImm:
             BAsmCode[CodeLen++]=0xe8+OpSize;
             BAsmCode[CodeLen++]=(HReg << 4)+0x0b-(OpSize << 1);
             memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
             CodeLen+=AdrCnt;
             break;
           END
         END
      END
     return;
    END

   if (Memo("DIVU"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode==ModReg)
        if ((OpSize==0) AND ((AdrPart&1)==1)) WrError(1445);
        else
         BEGIN
          HReg=AdrPart; z=OpSize; if (OpSize!=0) OpSize--;
	  DecodeAdr(ArgStr[2],MModReg+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg:
             BAsmCode[CodeLen++]=0xe1+(z << 2);
             if (z==2) BAsmCode[CodeLen-1]+=4;
             BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
             break;
            case ModImm:
             BAsmCode[CodeLen++]=0xe8+Ord(z==2);
             BAsmCode[CodeLen++]=(HReg << 4)+0x01+(Ord(z==1) << 1);
             memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
             CodeLen+=AdrCnt;
             break;
           END
         END
      END
     return;
    END

   if (Memo("MUL"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode==ModReg)
        if (OpSize!=1) WrError(1130);
        else if ((AdrPart&1)==1) WrError(1445);
        else
         BEGIN
          HReg=AdrPart; DecodeAdr(ArgStr[2],MModReg+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg:
             BAsmCode[CodeLen++]=0xe6;
             BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
             break;
            case ModImm:
             BAsmCode[CodeLen++]=0xe9;
             BAsmCode[CodeLen++]=(HReg << 4)+0x08;
             memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
             CodeLen+=AdrCnt;
             break;
           END
         END
      END
     return;
    END

   if (Memo("MULU"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode==ModReg)
        if ((AdrPart&1)==1) WrError(1445);
        else
         BEGIN
          HReg=AdrPart;
	  DecodeAdr(ArgStr[2],MModReg+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg:
             BAsmCode[CodeLen++]=0xe0+(OpSize << 2);
             BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
             break;
            case ModImm:
             BAsmCode[CodeLen++]=0xe8+OpSize;
             BAsmCode[CodeLen++]=(HReg << 4);
             memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
             CodeLen+=AdrCnt;
             break;
           END
         END
      END
     return;
    END

   if (Memo("LEA"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       if (AdrMode==ModReg)
        if (OpSize!=1) WrError(1130);
        else
         BEGIN
          HReg=AdrPart;
          strmaxprep(ArgStr[2],"[",255); strmaxcat(ArgStr[2],"]",255);
          DecodeAdr(ArgStr[2],MModMem);
          if (AdrMode==ModMem)
           switch (MemPart)
            BEGIN
             case 4:
             case 5:
              BAsmCode[CodeLen++]=0x20+(MemPart << 3);
              BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
              memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
              CodeLen+=AdrCnt;
              break;
             default:
              WrError(1350);
            END
         END
      END
     return;
    END

   /* Logik */

   if ((Memo("ANL")) OR (Memo("ORL")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (strcasecmp(ArgStr[1],"C")!=0) WrError(1350);
     else
      BEGIN
       if (*ArgStr[2]=='/')
        BEGIN
         OK=True; strcpy(ArgStr[2],ArgStr[2]+1);
        END
       else OK=False;
       if (DecodeBitAddr(ArgStr[2],&AdrLong))
        BEGIN
         ChkBitPage(AdrLong);
         BAsmCode[CodeLen++]=0x08;
         BAsmCode[CodeLen++]=0x40+(Ord(Memo("ORL")) << 5)+(Ord(OK) << 4)+(Hi(AdrLong) & 3);
         BAsmCode[CodeLen++]=Lo(AdrLong);
        END
      END
     return;
    END

   if ((Memo("CLR")) OR (Memo("SETB")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (DecodeBitAddr(ArgStr[1],&AdrLong))
      BEGIN
       ChkBitPage(AdrLong);
       BAsmCode[CodeLen++]=0x08;
       BAsmCode[CodeLen++]=(Ord(Memo("SETB")) << 4)+(Hi(AdrLong) & 3);
       BAsmCode[CodeLen++]=Lo(AdrLong);
      END
     return;
    END

   for (z=0; z<ShiftOrderCount; z++)
    if Memo(ShiftOrders[z])
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (OpSize>2) WrError(1130);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg);
        switch (AdrMode)
         BEGIN
          case ModReg:
           HReg=AdrPart; HMem=OpSize;
           if (*ArgStr[2]=='#') OpSize=(HMem==2)?-4:-2;
	   else OpSize=0;
           DecodeAdr(ArgStr[2],MModReg+((z==3)?0:MModImm));
           switch (AdrMode)
            BEGIN
             case ModReg:
              BAsmCode[CodeLen++]=0xc0+((HMem & 1) << 3)+z;
              if (HMem==2) BAsmCode[CodeLen-1]+=12;
              BAsmCode[CodeLen++]=(HReg << 4)+AdrPart;
              if (Memo("NORM"))
               if (HMem==2)
                BEGIN
                 if ((AdrPart >> 2)==(HReg >> 1)) WrError(140);
                END
               else if ((AdrPart >> HMem)==HReg) WrError(140);
              break;
             case ModImm:
              BAsmCode[CodeLen++]=0xd0+((HMem & 1) << 3)+z;
              if (HMem==2)
	       BEGIN
                BAsmCode[CodeLen-1]+=12;
                BAsmCode[CodeLen++]=((HReg & 14) << 4)+AdrVals[0];
               END
              else BAsmCode[CodeLen++]=(HReg << 4)+AdrVals[0];
              break;
	    END
           break;
         END
       END
      return;
     END

   for (z=0; z<RotateOrderCount; z++)
    if (Memo(RotateOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg);
        switch (AdrMode)
         BEGIN
          case ModReg:
           if (OpSize==2) WrError(1130);
           else
            BEGIN
             HReg=AdrPart; HMem=OpSize; OpSize=(-2);
             DecodeAdr(ArgStr[2],MModImm);
             switch (AdrMode)
              BEGIN
               case ModImm:
                BAsmCode[CodeLen++]=RotateOrders[z].Code+(HMem << 3);
                BAsmCode[CodeLen++]=(HReg << 4)+AdrVals[0];
                break;
              END
            END
           break;
         END
       END
      return;
     END

   /* vermischtes */

   if (Memo("TRAP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       OpSize=(-2);
       DecodeAdr(ArgStr[1],MModImm);
       switch (AdrMode)
        BEGIN
         case ModImm:
          BAsmCode[CodeLen++]=0xd6;
          BAsmCode[CodeLen++]=0x30+AdrVals[0];
          break;
        END
      END
     return;
    END

   /* Spruenge */

   for (z=0; z<RelOrderCount; z++)
    if (Memo(RelOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        FirstPassUnknown=True;
        AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
        if (OK)
         BEGIN
          ChkSpace(SegCode);
#ifdef __STDC__
          if (FirstPassUnknown) AdrLong&=0xfffffffeu;
#else
          if (FirstPassUnknown) AdrLong&=0xfffffffe;
#endif
          AdrLong-=(EProgCounter()+CodeLen+2) & 0xfffffe;
          if ((NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256))) WrError(1370);
          else if ((AdrLong&1)==1) WrError(1325);
          else
           BEGIN
            BAsmCode[CodeLen++]=RelOrders[z].Code;
            BAsmCode[CodeLen++]=(AdrLong >> 1) & 0xff;
           END
         END
       END
      return;
     END

   if (Memo("CALL"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[1]=='[')
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       if (AdrMode!=ModNone)
        if (MemPart!=2) WrError(1350);
        else
         BEGIN
          BAsmCode[CodeLen++]=0xc6;
          BAsmCode[CodeLen++]=AdrPart;
         END
      END
     else
      BEGIN
       FirstPassUnknown=False;
       AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (OK)
        BEGIN
         ChkSpace(SegCode);
#ifdef __STDC__
         if (FirstPassUnknown) AdrLong&=0xfffffffeu;
#else
         if (FirstPassUnknown) AdrLong&=0xfffffffe;
#endif
         AdrLong-=(EProgCounter()+CodeLen+3) & 0xfffffe;
         if ((NOT SymbolQuestionable) AND ((AdrLong>65534) OR (AdrLong<-65536))) WrError(1370);
         else if ((AdrLong&1)==1) WrError(1325);
         else
          BEGIN
           AdrLong>>=1;
           BAsmCode[CodeLen++]=0xc5;
           BAsmCode[CodeLen++]=(AdrLong >> 8) & 0xff;
           BAsmCode[CodeLen++]=AdrLong & 0xff;
          END
        END
      END
     return;
    END

   if (Memo("JMP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (strcasecmp(ArgStr[1],"[A+DPTR]")==0)
      BEGIN
       BAsmCode[CodeLen++]=0xd6;
       BAsmCode[CodeLen++]=0x46;
      END
     else if (strncmp(ArgStr[1],"[[",2)==0)
      BEGIN
       ArgStr[1][strlen(ArgStr[1])-1]='\0';
       DecodeAdr(ArgStr[1]+1,MModMem);
       if (AdrMode==ModMem)
        switch (MemPart)
         BEGIN
          case 3:
           BAsmCode[CodeLen++]=0xd6;
           BAsmCode[CodeLen++]=0x60+AdrPart;
           break;
          default:
           WrError(1350);
         END
      END
     else if (*ArgStr[1]=='[')
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       if (AdrMode==ModMem)
        switch (MemPart)
         BEGIN
          case 2:
           BAsmCode[CodeLen++]=0xd6;
           BAsmCode[CodeLen++]=0x70+AdrPart;
           break;
          default:
           WrError(1350);
         END
      END
     else
      BEGIN
       FirstPassUnknown=False;
       AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (OK)
        BEGIN
         ChkSpace(SegCode);
#ifdef __STDC__
         if (FirstPassUnknown) AdrLong&=0xfffffffeu;
#else
         if (FirstPassUnknown) AdrLong&=0xfffffffe;
#endif
         AdrLong-=(EProgCounter()+CodeLen+3) & 0xfffffe;
         if ((NOT SymbolQuestionable) AND ((AdrLong>65534) OR (AdrLong<-65536))) WrError(1370);
         else if ((AdrLong&1)==1) WrError(1325);
         else
          BEGIN
           AdrLong>>=1;
           BAsmCode[CodeLen++]=0xd5;
           BAsmCode[CodeLen++]=(AdrLong >> 8) & 0xff;
           BAsmCode[CodeLen++]=AdrLong & 0xff;
          END
        END
      END
     return;
    END

   if (Memo("CJNE"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrLong=EvalIntExpression(ArgStr[3],UInt24,&OK);
       if (FirstPassUnknown) AdrLong&=0xfffffe;
       if (OK)
        BEGIN
         ChkSpace(SegCode); OK=False; HReg=0;
         DecodeAdr(ArgStr[1],MModMem);
         if (AdrMode==ModMem)
          switch (MemPart)
           BEGIN
            case 1:
             if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
             else
              BEGIN
               HReg=AdrPart; DecodeAdr(ArgStr[2],MModMem+MModImm);
               switch (AdrMode)
                BEGIN
                 case ModMem:
                  if (MemPart!=6) WrError(1350);
                  else
                   BEGIN
                    BAsmCode[CodeLen]=0xe2+(OpSize << 3);
                    BAsmCode[CodeLen+1]=(HReg << 4)+AdrPart;
                    BAsmCode[CodeLen+2]=AdrVals[0];
                    HReg=CodeLen+3;
                    CodeLen+=4; OK=True;
                   END
                  break;
                 case ModImm:
                  BAsmCode[CodeLen]=0xe3+(OpSize << 3);
                  BAsmCode[CodeLen+1]=HReg << 4;
                  HReg=CodeLen+2;
                  memcpy(BAsmCode+CodeLen+3,AdrVals,AdrCnt);
                  CodeLen+=3+AdrCnt; OK=True;
                  break;
                END
              END
             break;
            case 2:
             if ((OpSize!=-1) AND (OpSize!=0) AND (OpSize!=1)) WrError(1130);
             else
              BEGIN
               HReg=AdrPart; DecodeAdr(ArgStr[2],MModImm);
               if (AdrMode==ModImm)
                BEGIN
                 BAsmCode[CodeLen]=0xe3+(OpSize << 3);
                 BAsmCode[CodeLen+1]=(HReg << 4)+8;
                 HReg=CodeLen+2;
                 memcpy(BAsmCode+CodeLen+3,AdrVals,AdrCnt);
                 CodeLen+=3+AdrCnt; OK=True;
                END
              END
             break;
            default:
             WrError(1350);
           END
         if (OK)
          BEGIN
           AdrLong-=(EProgCounter()+CodeLen) & 0xfffffe; OK=False;
           if ((NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256))) WrError(1370);
           else if ((AdrLong&1)==1) WrError(1325);
           else
	    BEGIN
	     BAsmCode[HReg]=(AdrLong >> 1) & 0xff; OK=True;
            END
          END
         if (NOT OK) CodeLen=0;
        END
      END
     return;
    END

   if (Memo("DJNZ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrLong=EvalIntExpression(ArgStr[2],UInt24,&OK);
       if (FirstPassUnknown) AdrLong&=0xfffffe;
       if (OK)
        BEGIN
         ChkSpace(SegCode); HReg=0;
         DecodeAdr(ArgStr[1],MModMem);
         OK=False; DecodeAdr(ArgStr[1],MModMem);
         if (AdrMode==ModMem)
          switch (MemPart)
           BEGIN
            case 1:
             if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
             else
              BEGIN
               BAsmCode[CodeLen]=0x87+(OpSize << 3);
               BAsmCode[CodeLen+1]=(AdrPart << 4)+0x08;
               HReg=CodeLen+2;
               CodeLen+=3; OK=True;
              END
             break;
            case 6:
             if (OpSize==-1) WrError(1132);
	     else if ((OpSize!=0) AND (OpSize!=1)) WrError(1130);
             else
              BEGIN
               BAsmCode[CodeLen]=0xe2+(OpSize << 3);
               BAsmCode[CodeLen+1]=0x08+AdrPart;
               BAsmCode[CodeLen+2]=AdrVals[0];
               HReg=CodeLen+3;
               CodeLen+=4; OK=True;
              END
             break;
            default:
             WrError(1350);
           END
         if (OK)
          BEGIN
           AdrLong-=(EProgCounter()+CodeLen) & 0xfffffe; OK=False;
           if ((NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256))) WrError(1370);
           else if ((AdrLong&1)==1) WrError(1325);
           else
	    BEGIN
	     BAsmCode[HReg]=(AdrLong >> 1) & 0xff; OK=True;
            END
          END
         if (NOT OK) CodeLen=0;
        END
      END
     return;
    END

   if ((Memo("FCALL")) OR (Memo("FJMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (FirstPassUnknown) AdrLong&=0xfffffe;
       if (OK)
        if ((AdrLong&1)==1) WrError(1325);
        else
         BEGIN
          BAsmCode[CodeLen++]=0xc4+(Ord(Memo("FJMP")) << 4);
          BAsmCode[CodeLen++]=(AdrLong >> 8) & 0xff;
          BAsmCode[CodeLen++]=AdrLong & 0xff;
          BAsmCode[CodeLen++]=(AdrLong >> 16) & 0xff;
         END
      END
     return;
    END

   for (z=0; z<JBitOrderCnt; z++)
    if (Memo(JBitOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else if (DecodeBitAddr(ArgStr[1],&AdrLong))
       BEGIN
        BAsmCode[CodeLen]=0x97;
        BAsmCode[CodeLen+1]=JBitOrders[z].Code+Hi(AdrLong);
        BAsmCode[CodeLen+2]=Lo(AdrLong);
        FirstPassUnknown=False;
        AdrLong=EvalIntExpression(ArgStr[2],UInt24,&OK);
        if (FirstPassUnknown) AdrLong&=0xfffffe;
        AdrLong-=(EProgCounter()+CodeLen+4) & 0xfffffe;
        if (OK)
         if ((NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256))) WrError(1370);
         else if ((AdrLong&1)==1) WrError(1325);
          else
           BEGIN
            BAsmCode[CodeLen+3]=(AdrLong >> 1) & 0xff;
            CodeLen+=4;
           END
        END
       return;
      END

   WrXError(1200,OpPart);
END

	static void InitCode_XA(void)
BEGIN
   SaveInitProc();
   Reg_DS=0;
END

	static Boolean ChkPC_XA(void)
BEGIN
   switch (ActPC)
    BEGIN
     case SegCode:
     case SegData:
      return (ProgCounter()<0x1000000);
     case SegIO:
      return ((ProgCounter()>0x3ff) AND (ProgCounter()<0x800));
     default:
      return False;
    END
END

	static Boolean IsDef_XA(void)
BEGIN
   return (ActPC==SegCode);
END

        static void SwitchFrom_XA(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_XA(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x3c; NOPCode=0x00;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=(1<<SegCode)|(1<<SegData)|(1<<SegIO);
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
   Grans[SegData ]=1; ListGrans[SegData ]=1; SegInits[SegData ]=0;
   Grans[SegIO   ]=1; ListGrans[SegIO   ]=1; SegInits[SegIO   ]=0x400;

   MakeCode=MakeCode_XA; ChkPC=ChkPC_XA; IsDef=IsDef_XA;
   SwitchFrom=SwitchFrom_XA; InitFields();
END

	void codexa_init(void)
BEGIN
   CPUXAG1=AddCPU("XAG1",SwitchTo_XA);
   CPUXAG2=AddCPU("XAG2",SwitchTo_XA);
   CPUXAG3=AddCPU("XAG3",SwitchTo_XA);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_XA;
END
