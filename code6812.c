/* code6812.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul CPU12                                                  */
/*                                                                           */
/* Historie: 13.10.1996 Grundsteinlegung                                     */
/*           25.10.1998 dir. 16-Bit-Modus von ...11 auf ...10 korrigiert     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Boolean MayImm,MayDir,MayExt;
          ShortInt ThisOpSize;
         } GenOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Boolean MayDir;
         } JmpOrder;


#define ModNone (-1)
#define ModImm 0
#define MModImm (1 << ModImm)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModExt 2
#define MModExt (1 << ModExt)
#define ModIdx 3
#define MModIdx (1 << ModIdx)
#define ModIdx1 4
#define MModIdx1 (1 << ModIdx1)
#define ModIdx2 5
#define MModIdx2 (1 << ModIdx2)
#define ModDIdx 6
#define MModDIdx (1 << ModDIdx)
#define ModIIdx2 7
#define MModIIdx2 (1 << ModIIdx2)

#define MModAllIdx (MModIdx | MModIdx1 | MModIdx2 | MModDIdx | MModIIdx2)

#define FixedOrderCount 87
#define BranchOrderCount 20
#define GenOrderCount 56 
#define LoopOrderCount 6
#define LEAOrderCount 3
#define JmpOrderCount 2

static ShortInt OpSize;
static ShortInt AdrMode;
static ShortInt ExPos;
static Byte AdrVals[4];
static CPUVar CPU6812;

static FixedOrder *FixedOrders;
static FixedOrder *BranchOrders;
static GenOrder *GenOrders;
static FixedOrder *LoopOrders;
static FixedOrder *LEAOrders;
static JmpOrder *JmpOrders;

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddBranch(char *NName, Word NCode)
BEGIN
   if (InstrZ>=BranchOrderCount) exit(255);
   BranchOrders[InstrZ].Name=NName;
   BranchOrders[InstrZ++].Code=NCode;
END

        static void AddGen(char *NName, Word NCode,
                           Boolean NMayI, Boolean NMayD, Boolean NMayE,
                           ShortInt NSize)
BEGIN
   if (InstrZ>=GenOrderCount) exit(255);
   GenOrders[InstrZ].Name=NName;
   GenOrders[InstrZ].Code=NCode;
   GenOrders[InstrZ].MayImm=NMayI;
   GenOrders[InstrZ].MayDir=NMayD;
   GenOrders[InstrZ].MayExt=NMayE;
   GenOrders[InstrZ++].ThisOpSize=NSize;
END

        static void AddLoop(char *NName, Word NCode)
BEGIN
   if (InstrZ>=LoopOrderCount) exit(255);
   LoopOrders[InstrZ].Name=NName;
   LoopOrders[InstrZ++].Code=NCode;
END

        static void AddLEA(char *NName, Word NCode)
BEGIN
   if (InstrZ>=LEAOrderCount) exit(255);
   LEAOrders[InstrZ].Name=NName;
   LEAOrders[InstrZ++].Code=NCode;
END

        static void AddJmp(char *NName, Word NCode, Boolean NDir)
BEGIN
   if (InstrZ>=JmpOrderCount) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ].Code=NCode;
   JmpOrders[InstrZ++].MayDir=NDir;
END


        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(FixedOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddFixed("ABA"  ,0x1806); AddFixed("ABX"  ,0x1ae5);
   AddFixed("ABY"  ,0x19ed); AddFixed("ASLA" ,0x0048);
   AddFixed("ASLB" ,0x0058); AddFixed("ASLD" ,0x0059);
   AddFixed("ASRA" ,0x0047); AddFixed("ASRB" ,0x0057);
   AddFixed("BGND" ,0x0000); AddFixed("CBA"  ,0x1817);
   AddFixed("CLC"  ,0x10fe); AddFixed("CLI"  ,0x10ef);
   AddFixed("CLRA" ,0x0087); AddFixed("CLRB" ,0x00c7);
   AddFixed("CLV"  ,0x10fd); AddFixed("COMA" ,0x0041);
   AddFixed("COMB" ,0x0051); AddFixed("DAA"  ,0x1807);
   AddFixed("DECA" ,0x0043); AddFixed("DECB" ,0x0053);
   AddFixed("DES"  ,0x1b9f); AddFixed("DEX"  ,0x0009);
   AddFixed("DEY"  ,0x0003); AddFixed("EDIV" ,0x0011);
   AddFixed("EDIVS",0x1814); AddFixed("EMUL" ,0x0013);
   AddFixed("EMULS",0x1813); AddFixed("FDIV" ,0x1811);
   AddFixed("IDIV" ,0x1810); AddFixed("IDIVS",0x1815);
   AddFixed("INCA" ,0x0042); AddFixed("INCB" ,0x0052);
   AddFixed("INS"  ,0x1b81); AddFixed("INX"  ,0x0008);
   AddFixed("INY"  ,0x0002); AddFixed("LSLA" ,0x0048);
   AddFixed("LSLB" ,0x0058); AddFixed("LSLD" ,0x0059);
   AddFixed("LSRA" ,0x0044); AddFixed("LSRB" ,0x0054);
   AddFixed("LSRD" ,0x0049); AddFixed("MEM"  ,0x0001);
   AddFixed("MUL"  ,0x0012); AddFixed("NEGA" ,0x0040);
   AddFixed("NEGB" ,0x0050); AddFixed("NOP"  ,0x00a7);
   AddFixed("PSHA" ,0x0036); AddFixed("PSHB" ,0x0037);
   AddFixed("PSHC" ,0x0039); AddFixed("PSHD" ,0x003b);
   AddFixed("PSHX" ,0x0034); AddFixed("PSHY" ,0x0035);
   AddFixed("PULA" ,0x0032); AddFixed("PULB" ,0x0033);
   AddFixed("PULC" ,0x0038); AddFixed("PULD" ,0x003a);
   AddFixed("PULX" ,0x0030); AddFixed("PULY" ,0x0031);
   AddFixed("REV"  ,0x183a); AddFixed("REVW" ,0x183b);
   AddFixed("ROLA" ,0x0045); AddFixed("ROLB" ,0x0055);
   AddFixed("RORA" ,0x0046); AddFixed("RORB" ,0x0056);
   AddFixed("RTC"  ,0x000a); AddFixed("RTI"  ,0x000b);
   AddFixed("RTS"  ,0x003d); AddFixed("SBA"  ,0x1816);
   AddFixed("SEC"  ,0x1401); AddFixed("SEI"  ,0x1410);
   AddFixed("SEV"  ,0x1402); AddFixed("STOP" ,0x183e);
   AddFixed("SWI"  ,0x003f); AddFixed("TAB"  ,0x180e);
   AddFixed("TAP"  ,0xb702); AddFixed("TBA"  ,0x180f);
   AddFixed("TPA"  ,0xb720); AddFixed("TSTA" ,0x0097);
   AddFixed("TSTB" ,0x00d7); AddFixed("TSX"  ,0xb775);
   AddFixed("TSY"  ,0xb776); AddFixed("TXS"  ,0xb757);
   AddFixed("TYS"  ,0xb767); AddFixed("WAI"  ,0x003e);
   AddFixed("WAV"  ,0x183c); AddFixed("XGDX" ,0xb7c5);
   AddFixed("XGDY" ,0xb7c6);

   BranchOrders=(FixedOrder *) malloc(BranchOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddBranch("BGT",0x2e);    AddBranch("BGE",0x2c);
   AddBranch("BEQ",0x27);    AddBranch("BLE",0x2f);
   AddBranch("BLT",0x2d);    AddBranch("BHI",0x22);
   AddBranch("BHS",0x24);    AddBranch("BCC",0x24);
   AddBranch("BNE",0x26);    AddBranch("BLS",0x23);
   AddBranch("BLO",0x25);    AddBranch("BCS",0x25);
   AddBranch("BMI",0x2b);    AddBranch("BVS",0x29);
   AddBranch("BRA",0x20);    AddBranch("BPL",0x2a);
   AddBranch("BGT",0x2e);    AddBranch("BRN",0x21);
   AddBranch("BVC",0x28);    AddBranch("BSR",0x07);

   GenOrders=(GenOrder *) malloc(sizeof(GenOrder)*GenOrderCount); InstrZ=0;
   AddGen("ADCA" ,0x0089,True ,True ,True , 0);
   AddGen("ADCB" ,0x00c9,True ,True ,True , 0);
   AddGen("ADDA" ,0x008b,True ,True ,True , 0);
   AddGen("ADDB" ,0x00cb,True ,True ,True , 0);
   AddGen("ADDD" ,0x00c3,True ,True ,True , 1);
   AddGen("ANDA" ,0x0084,True ,True ,True , 0);
   AddGen("ANDB" ,0x00c4,True ,True ,True , 0);
   AddGen("ASL"  ,0x0048,False,False,True ,-1);
   AddGen("ASR"  ,0x0047,False,False,True ,-1);
   AddGen("BITA" ,0x0085,True ,True ,True , 0);
   AddGen("BITB" ,0x00c5,True ,True ,True , 0);
   AddGen("CLR"  ,0x0049,False,False,True ,-1);
   AddGen("CMPA" ,0x0081,True ,True ,True , 0);
   AddGen("CMPB" ,0x00c1,True ,True ,True , 0);
   AddGen("COM"  ,0x0041,False,False,True ,-1);
   AddGen("CPD"  ,0x008c,True ,True ,True , 1);
   AddGen("CPS"  ,0x008f,True ,True ,True , 1);
   AddGen("CPX"  ,0x008e,True ,True ,True , 1);
   AddGen("CPY"  ,0x008d,True ,True ,True , 1);
   AddGen("DEC"  ,0x0043,False,False,True ,-1);
   AddGen("EMAXD",0x18fa,False,False,False,-1);
   AddGen("EMAXM",0x18fe,False,False,False,-1);
   AddGen("EMIND",0x18fb,False,False,False,-1);
   AddGen("EMINM",0x18ff,False,False,False,-1);
   AddGen("EORA" ,0x0088,True ,True ,True , 0);
   AddGen("EORB" ,0x00c8,True ,True ,True , 0);
   AddGen("INC"  ,0x0042,False,False,True ,-1);
   AddGen("LDAA" ,0x0086,True ,True ,True , 0);
   AddGen("LDAB" ,0x00c6,True ,True ,True , 0);
   AddGen("LDD"  ,0x00cc,True ,True ,True , 1);
   AddGen("LDS"  ,0x00cf,True ,True ,True , 1);
   AddGen("LDX"  ,0x00ce,True ,True ,True , 1);
   AddGen("LDY"  ,0x00cd,True ,True ,True , 1);
   AddGen("LSL"  ,0x0048,False,False,True ,-1);
   AddGen("LSR"  ,0x0044,False,False,True ,-1);
   AddGen("MAXA" ,0x18f8,False,False,False,-1);
   AddGen("MAXM" ,0x18fc,False,False,False,-1);
   AddGen("MINA" ,0x18f9,False,False,False,-1);
   AddGen("MINM" ,0x18fd,False,False,False,-1);
   AddGen("NEG"  ,0x0040,False,False,True ,-1);
   AddGen("ORAA" ,0x008a,True ,True ,True , 0);
   AddGen("ORAB" ,0x00ca,True ,True ,True , 0);
   AddGen("ROL"  ,0x0045,False,False,True ,-1);
   AddGen("ROR"  ,0x0046,False,False,True ,-1);
   AddGen("SBCA" ,0x0082,True ,True ,True , 0);
   AddGen("SBCB" ,0x00c2,True ,True ,True , 0);
   AddGen("STAA" ,0x004a,False,True ,True , 0);
   AddGen("STAB" ,0x004b,False,True ,True , 0);
   AddGen("STD"  ,0x004c,False,True ,True ,-1);
   AddGen("STS"  ,0x004f,False,True ,True ,-1);
   AddGen("STX"  ,0x004e,False,True ,True ,-1);
   AddGen("STY"  ,0x004d,False,True ,True ,-1);
   AddGen("SUBA" ,0x0080,True ,True ,True , 0);
   AddGen("SUBB" ,0x00c0,True ,True ,True , 0);
   AddGen("SUBD" ,0x0083,True ,True ,True , 1);
   AddGen("TST"  ,0x00c7,False,False,True ,-1);

   LoopOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LoopOrderCount); InstrZ=0;
   AddLoop("DBEQ",0x00); AddLoop("DBNE",0x20);
   AddLoop("IBEQ",0x80); AddLoop("IBNE",0xa0);
   AddLoop("TBEQ",0x40); AddLoop("TBNE",0x60);

   LEAOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LEAOrderCount); InstrZ=0;
   AddLEA("LEAS",0x1b);
   AddLEA("LEAX",0x1a);
   AddLEA("LEAY",0x19);

   JmpOrders=(JmpOrder *) malloc(sizeof(JmpOrder)*JmpOrderCount); InstrZ=0;
   AddJmp("JMP",0x06,False);
   AddJmp("JSR",0x16,True);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(BranchOrders);
   free(GenOrders);
   free(LoopOrders);
   free(LEAOrders);
   free(JmpOrders);
END

/*---------------------------------------------------------------------------*/

#define PCReg 3

	static Boolean DecodeReg16(char *Asc, Byte *Erg)
BEGIN
   *Erg=0xff;
   if (strcasecmp(Asc,"X")==0) *Erg=0;
   else if (strcasecmp(Asc,"Y")==0) *Erg=1;
   else if (strcasecmp(Asc,"SP")==0) *Erg=2;
   else if (strcasecmp(Asc,"PC")==0) *Erg=PCReg;
   return (*Erg!=0xff);
END

	static Boolean ValidReg(char *Asc_o)
BEGIN
   Byte Dummy;
   String Asc;
   int l=strlen(Asc_o);

   strmaxcpy(Asc,Asc_o,255);

   if ((*Asc=='-') OR (*Asc=='+')) strcpy(Asc,Asc+1);
   else if ((Asc[l-1]=='-') OR (Asc[l-1]=='+')) Asc[l-1]='\0';
   return DecodeReg16(Asc,&Dummy);
END

	static Boolean DecodeReg8(char *Asc, Byte *Erg)
BEGIN
   *Erg=0xff;
   if (strcasecmp(Asc,"A")==0) *Erg=0;
   else if (strcasecmp(Asc,"B")==0) *Erg=1;
   else if (strcasecmp(Asc,"D")==0) *Erg=2;
   return (*Erg!=0xff);
END

	static Boolean DecodeReg(char *Asc, Byte *Erg)
BEGIN
   if (DecodeReg8(Asc,Erg))
    BEGIN
     if (*Erg==2) *Erg=4; return True;
    END
   else if (DecodeReg16(Asc,Erg))
    BEGIN
     *Erg+=5; return (*Erg!=PCReg);
    END
   else if (strcasecmp(Asc,"CCR")==0)
    BEGIN
     *Erg=2; return True;
    END
   else return False;
END

	static void CutShort(char *Asc, Integer *ShortMode)
BEGIN
   if (*Asc=='>')
    BEGIN
     *ShortMode=1;
     strcpy(Asc,Asc+1);
    END
   else if (*Asc=='<')
    BEGIN
     *ShortMode=2;
     strcpy(Asc,Asc+1);
     if (*Asc=='<')
      BEGIN
       *ShortMode=3;
       strcpy(Asc,Asc+1);
      END
    END
   else *ShortMode=0;
END

	static Boolean DistFits(Byte Reg, Integer Dist, Integer Offs, LongInt Min, LongInt Max)
BEGIN
   if (Reg==PCReg) Dist-=Offs;
   return (((Dist>=Min) AND (Dist<=Max)) OR ((Reg==PCReg) AND SymbolQuestionable));
END

	static void ChkAdr(Word Mask)
BEGIN
  if ((AdrMode!=ModNone) AND (((1 << AdrMode) & Mask)==0))
   BEGIN
    AdrMode=ModNone; AdrCnt=0; WrError(1350);
   END
END
	
        static void DecodeAdr(int Start, int Stop, Word Mask)
BEGIN
   Integer AdrWord,ShortMode;
   int l;
   char *p;
   Boolean OK;
   Boolean DecFlag,AutoFlag,PostFlag;

   AdrMode=ModNone; AdrCnt=0;

   if (Stop-Start==0)
    BEGIN

     /* immediate */

     if (*ArgStr[Start]=='#')
      BEGIN
       switch (OpSize)
        BEGIN
         case -1:WrError(1132); break;
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
            AdrVals[0]=AdrWord >> 8; AdrVals[1]=AdrWord & 0xff;
            AdrCnt=2; AdrMode=ModImm;
           END
          break;
        END
       ChkAdr(Mask); return;
      END

     /* indirekt */

     if ((*ArgStr[Start]=='[') AND (ArgStr[Start][strlen(ArgStr[Start])-1]==']'))
      BEGIN
       strcpy(ArgStr[Start],ArgStr[Start]+1);
       ArgStr[Start][strlen(ArgStr[Start])-1]='\0';
       p=QuotPos(ArgStr[Start],','); if (p!=Nil) *p='\0';
       if (p==Nil) WrError(1350);
       else if (NOT DecodeReg16(p+1,AdrVals))
        WrXError(1445,p+1);
       else if (strcasecmp(ArgStr[Start],"D")==0)
        BEGIN
         AdrVals[0]=(AdrVals[0] << 3) | 0xe7;
         AdrCnt=1; AdrMode=ModDIdx;
        END
       else
        BEGIN
         AdrWord=EvalIntExpression(ArgStr[Start],Int16,&OK);
         if (OK)
          BEGIN
           if (AdrVals[0]==PCReg) AdrWord-=EProgCounter()+ExPos+3;
           AdrVals[0]=(AdrVals[0] << 3) | 0xe3;
           AdrVals[1]=AdrWord >> 8;
           AdrVals[2]=AdrWord & 0xff;
           AdrCnt=3; AdrMode=ModIIdx2;
          END
        END
       ChkAdr(Mask); return;
      END

     /* dann absolut */

     CutShort(ArgStr[Start],&ShortMode);

     if ((ShortMode==2) OR ((ShortMode==0) AND ((Mask & MModExt)==0)))
      AdrWord=EvalIntExpression(ArgStr[Start],UInt8,&OK);
     else
      AdrWord=EvalIntExpression(ArgStr[Start],UInt16,&OK);

     if (OK)
      if ((ShortMode!=1) AND ((AdrWord & 0xff00)==0) AND ((Mask & MModDir)!=0))
       BEGIN
        AdrMode=ModDir; AdrVals[0]=AdrWord & 0xff; AdrCnt=1;
       END
      else
       BEGIN
        AdrMode=ModExt;
	AdrVals[0]=(AdrWord >> 8) & 0xff; AdrVals[1]=AdrWord & 0xff;
	AdrCnt=2;
       END
     ChkAdr(Mask); return;
    END

   else if (Stop-Start==1)
    BEGIN

     /* Autoin/-dekrement abspalten */

     l=strlen(ArgStr[Stop]);
     if ((*ArgStr[Stop]=='-') OR (*ArgStr[Stop]=='+'))
      BEGIN
       DecFlag=(*ArgStr[Stop]=='-');
       AutoFlag=True; PostFlag=False; strcpy(ArgStr[Stop],ArgStr[Stop]+1);
      END
     else if ((ArgStr[Stop][l-1]=='-') OR (ArgStr[Stop][l-1]=='+'))
      BEGIN
       DecFlag=(ArgStr[Stop][l-1]=='-');
       AutoFlag=True; PostFlag=True; ArgStr[Stop][l-1]='\0';
      END
     else AutoFlag=DecFlag=PostFlag=False;

     if (AutoFlag)
      BEGIN
       if (NOT DecodeReg16(ArgStr[Stop],AdrVals)) WrXError(1445,ArgStr[Stop]);
       else if (AdrVals[0]==PCReg) WrXError(1445,ArgStr[Stop]);
       else
        BEGIN
         FirstPassUnknown=False;
         AdrWord=EvalIntExpression(ArgStr[Start],SInt8,&OK);
         if (FirstPassUnknown) AdrWord=1;
         if (AdrWord==0)
          BEGIN
           AdrVals[0]=0; AdrCnt=1; AdrMode=ModIdx;
          END
         else if (AdrWord>8) WrError(1320);
         else if (AdrWord<-8) WrError(1315);
         else
          BEGIN
           if (AdrWord<0)
            BEGIN
             DecFlag=NOT DecFlag; AdrWord=(-AdrWord);
            END
           if (DecFlag) AdrWord=8-AdrWord; else AdrWord--;
           AdrVals[0]=(AdrVals[0] << 6)+0x20+(Ord(PostFlag) << 4)+(Ord(DecFlag) << 3)+(AdrWord & 7);
           AdrCnt=1; AdrMode=ModIdx;
          END
        END
       ChkAdr(Mask); return;
      END

     else
      BEGIN
       if (NOT DecodeReg16(ArgStr[Stop],AdrVals)) WrXError(1445,ArgStr[Stop]);
       else if (DecodeReg8(ArgStr[Start],AdrVals+1))
        BEGIN
         AdrVals[0]=(AdrVals[0] << 3)+AdrVals[1]+0xe4;
         AdrCnt=1; AdrMode=ModIdx;
        END
       else
        BEGIN
         CutShort(ArgStr[Start],&ShortMode);
         AdrWord=EvalIntExpression(ArgStr[Start],Int16,&OK);
         if (AdrVals[0]==PCReg) AdrWord-=EProgCounter()+ExPos;
         if (OK)
          if ((ShortMode!=1) AND (ShortMode!=2) AND ((Mask & MModIdx)!=0) AND (DistFits(AdrVals[0],AdrWord,1,-16,15)))
           BEGIN
            if (AdrVals[0]==PCReg) AdrWord--;
            AdrVals[0]=(AdrVals[0] << 6)+(AdrWord & 0x1f);
            AdrCnt=1; AdrMode=ModIdx;
           END
          else if ((ShortMode!=1) AND (ShortMode!=3) AND ((Mask & MModIdx1)!=0) AND DistFits(AdrVals[0],AdrWord,2,-256,255))
           BEGIN
            if (AdrVals[0]==PCReg) AdrWord-=2;
            AdrVals[0]=0xe0+(AdrVals[0] << 3)+((AdrWord >> 8) & 1);
            AdrVals[1]=AdrWord & 0xff;
            AdrCnt=2; AdrMode=ModIdx1;
           END
          else
           BEGIN
            if (AdrVals[0]==PCReg) AdrWord-=3;
            AdrVals[0]=0xe2+(AdrVals[0] << 3);
            AdrVals[1]=(AdrWord >> 8) & 0xff;
            AdrVals[2]=AdrWord & 0xff;
            AdrCnt=3; AdrMode=ModIdx2;
           END
        END
       ChkAdr(Mask); return;
      END
    END

   else WrError(1350);

   ChkAdr(Mask);
END

/*---------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
   return False;
END

	static void Try2Split(int Src)
BEGIN
   char *p;
   int z;

   KillPrefBlanks(ArgStr[Src]); KillPostBlanks(ArgStr[Src]);
   p=ArgStr[Src]+strlen(ArgStr[Src])-1;
   while ((p>=ArgStr[Src]) AND (NOT isspace(((unsigned int) *p)&0xff))) p--;
   if (p>=ArgStr[Src])
    BEGIN
     for (z=ArgCnt; z>=Src; z--) strcpy(ArgStr[z+1],ArgStr[z]); ArgCnt++;
     *p='\0'; strcpy(ArgStr[Src+1],p+1);
     KillPostBlanks(ArgStr[Src]); KillPrefBlanks(ArgStr[Src+1]);
    END
END

        static void MakeCode_6812(void)
BEGIN
   int z;
   LongInt Address;
   Byte HReg,HCnt;
   Boolean OK;
   Word Mask;

   CodeLen=0; DontPrint=False; OpSize=(-1);

   /* Operandengroesse festlegen */

   if (*AttrPart!='\0')
    switch (toupper(*AttrPart))
     BEGIN
      case 'B':OpSize=0; break;
      case 'W':OpSize=1; break;
      case 'L':OpSize=2; break;
      case 'Q':OpSize=3; break;
      case 'S':OpSize=4; break;
      case 'D':OpSize=5; break;
      case 'X':OpSize=6; break;
      case 'P':OpSize=7; break;
      default:
       WrError(1107); return;
     END

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(True)) return;
   if (DecodeMoto16Pseudo(OpSize,True)) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if Memo(FixedOrders[z].Name)
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        CodeLen=1+Ord(Hi(FixedOrders[z].Code)!=0);
        if (CodeLen==2) BAsmCode[0]=Hi(FixedOrders[z].Code);
        BAsmCode[CodeLen-1]=Lo(FixedOrders[z].Code);
       END
      return;
     END

   /* einfacher Adressoperand */

   for (z=0; z<GenOrderCount; z++)
    if Memo(GenOrders[z].Name)
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        ExPos=1+Ord(Hi(GenOrders[z].Code)!=0); OpSize=GenOrders[z].ThisOpSize;
        Mask=MModAllIdx;
        if (GenOrders[z].MayImm) Mask+=MModImm;
        if (GenOrders[z].MayDir) Mask+=MModDir;
        if (GenOrders[z].MayExt) Mask+=MModExt;
        DecodeAdr(1,ArgCnt,Mask);
        if (AdrMode!=ModNone)
         if (Hi(GenOrders[z].Code)==0)
          BEGIN
           BAsmCode[0]=GenOrders[z].Code; CodeLen=1;
          END
         else
          BEGIN
           BAsmCode[0]=Hi(GenOrders[z].Code); 
           BAsmCode[1]=Lo(GenOrders[z].Code); CodeLen=2;
          END;
        switch (AdrMode)
         BEGIN
          case ModImm: break;
          case ModDir: BAsmCode[CodeLen-1]+=0x10; break;
          case ModIdx:
          case ModIdx1:
          case ModIdx2:
          case ModDIdx:
          case ModIIdx2: BAsmCode[CodeLen-1]+=0x20; break;
          case ModExt: BAsmCode[CodeLen-1]+=0x30; break;
         END
        if (AdrMode!=ModNone)
         BEGIN
          memcpy(BAsmCode+CodeLen,AdrVals,AdrCnt);
          CodeLen+=AdrCnt;
         END
       END
      return;
     END

   /* Arithmetik */

   for (z=0; z<LEAOrderCount; z++)
    if (Memo(LEAOrders[z].Name))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        ExPos=1;
        DecodeAdr(1,ArgCnt,MModIdx+MModIdx1+MModIdx2);
        if (AdrMode!=ModNone)
         BEGIN
          BAsmCode[0]=LEAOrders[z].Code;
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
         END
       END
      return;
     END

   if ((Memo("TBL")) OR (Memo("ETBL")))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       ExPos=2;
       DecodeAdr(1,ArgCnt,MModIdx);
       if (AdrMode==ModIdx)
        BEGIN
         BAsmCode[0]=0x18;
         BAsmCode[1]=0x3d+(Ord(Memo("ETBL")) << 1);
         memcpy(BAsmCode+2,AdrVals,AdrCnt);
         CodeLen=2+AdrCnt;
        END
      END
     return;
    END

   if (Memo("EMACS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       Address=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=0x18;
         BAsmCode[1]=0x12;
         BAsmCode[2]=(Address >> 8) & 0xff;
         BAsmCode[3]=Address & 0xff;
         CodeLen=4;
        END
      END
     return;
    END

   /* Transfer */

   if ((Memo("TFR")) OR (Memo("EXG")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (NOT DecodeReg(ArgStr[2],BAsmCode+1)) WrXError(1445,ArgStr[2]);
     else if (NOT DecodeReg(ArgStr[1],&HReg)) WrXError(1445,ArgStr[1]);
     else
      BEGIN
       BAsmCode[0]=0xb7;
       BAsmCode[1]+=(Ord(Memo("EXG")) << 7)+(HReg << 4);
       CodeLen=2;
      END
     return;
    END

   if (Memo("SEX"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (NOT DecodeReg(ArgStr[2],BAsmCode+1)) WrXError(1445,ArgStr[2]);
     else if (BAsmCode[1]<4) WrXError(1445,ArgStr[2]);
     else if (NOT DecodeReg(ArgStr[1],&HReg)) WrXError(1445,ArgStr[1]);
     else if (HReg>3) WrXError(1445,ArgStr[1]);
     else
      BEGIN
       BAsmCode[0]=0xb7;
       BAsmCode[1]+=(Ord(Memo("EXG")) << 7)+(HReg << 4);
       CodeLen=2;
      END
     return;
    END

   if ((Memo("MOVB")) OR (Memo("MOVW")))
    BEGIN
     switch (ArgCnt)
      BEGIN
       case 1:Try2Split(1); break;
       case 2:Try2Split(1); if (ArgCnt==2) Try2Split(2); break;
       case 3:Try2Split(2); break;
      END
     if ((ArgCnt<2) OR (ArgCnt>4)) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       if (ArgCnt==2) HReg=2;
       else if (ArgCnt==4) HReg=3;
       else if (ValidReg(ArgStr[2])) HReg=3;
       else HReg=2;
       OpSize=Ord(Memo("MOVW")); ExPos=2;
       BAsmCode[0]=0x18; BAsmCode[1]=Ord(Memo("MOVB")) << 3;
       DecodeAdr(1,HReg-1,MModImm+MModExt+MModIdx);
       switch (AdrMode)
        BEGIN
         case ModImm:
          memmove(AdrVals+2,AdrVals,AdrCnt); HCnt=AdrCnt;
          ExPos=4+2*OpSize; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
          switch (AdrMode)
           BEGIN
            case ModExt:
             BAsmCode[1]+=3;
             memcpy(BAsmCode+2,AdrVals+2,HCnt);
             memcpy(BAsmCode+2+HCnt,AdrVals,AdrCnt);
             CodeLen=2+HCnt+AdrCnt;
             break;
            case ModIdx:
             BAsmCode[2]=AdrVals[0];
             memcpy(BAsmCode+3,AdrVals+2,HCnt);
             CodeLen=3+HCnt;
             break;
           END
          break;
         case ModExt:
          memmove(AdrVals+2,AdrVals,AdrCnt); HCnt=AdrCnt;
          ExPos=6; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
          switch (AdrMode)
           BEGIN
            case ModExt:
             BAsmCode[1]+=4;
             memcpy(BAsmCode+2,AdrVals+2,HCnt);
             memcpy(BAsmCode+2+HCnt,AdrVals,AdrCnt);
             CodeLen=2+HCnt+AdrCnt;
             break;
            case ModIdx:
             BAsmCode[1]+=1;
             BAsmCode[2]=AdrVals[0];
             memcpy(BAsmCode+3,AdrVals+2,HCnt);
             CodeLen=3+HCnt;
             break;
           END;
          break;
         case ModIdx:
          HCnt=AdrVals[0];
          ExPos=4; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
          switch (AdrMode)
           BEGIN
            case ModExt:
             BAsmCode[1]+=5;
             BAsmCode[2]=HCnt;
             memcpy(BAsmCode+3,AdrVals,AdrCnt);
             CodeLen=3+AdrCnt;
             break;
            case ModIdx:
             BAsmCode[1]+=2;
             BAsmCode[2]=HCnt;
             BAsmCode[3]=AdrVals[0];
             CodeLen=4;
             break;
           END
          break;
        END
      END
     return;
    END

   /* Logik */

   if ((Memo("ANDCC")) OR (Memo("ORCC")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       OpSize=0; DecodeAdr(1,1,MModImm);
       if (AdrMode==ModImm)
        BEGIN
         BAsmCode[0]=0x10+(Ord(Memo("ORCC")) << 2);
         BAsmCode[1]=AdrVals[0];
         CodeLen=2;
        END
      END
     return;
    END

    if ((Memo("BSET")) OR (Memo("BCLR")))
     BEGIN
      if ((ArgCnt==1) OR (ArgCnt==2)) Try2Split(ArgCnt);
      if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        if (*ArgStr[ArgCnt]=='#') strcpy(ArgStr[ArgCnt],ArgStr[ArgCnt]+1);
        HReg=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
        if (OK)
         BEGIN
          ExPos=2; /* wg. Masken-Postbyte */
          DecodeAdr(1,ArgCnt-1,MModDir+MModExt+MModIdx+MModIdx1+MModIdx2);
          if (AdrMode!=ModNone)
           BEGIN
            BAsmCode[0]=0x0c+Ord(Memo("BCLR"));
            switch (AdrMode)
             BEGIN
              case ModDir: BAsmCode[0]+=0x40; break;
              case ModExt: BAsmCode[0]+=0x10; break;
             END
            memcpy(BAsmCode+1,AdrVals,AdrCnt);
            BAsmCode[1+AdrCnt]=HReg;
            CodeLen=2+AdrCnt;
           END
         END
       END
      return;
     END

   /* Spruenge */

   for (z=0; z<BranchOrderCount; z++)
    if Memo(BranchOrders[z].Name)
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        Address=EvalIntExpression(ArgStr[1],UInt16,&OK)-EProgCounter()-2;
        if (OK)
         if (((Address<-128) OR (Address>127)) AND (NOT SymbolQuestionable)) WrError(1370);
         else
          BEGIN
           BAsmCode[0]=BranchOrders[z].Code; BAsmCode[1]=Address & 0xff; CodeLen=2;
          END
       END
      return;
     END
    else if ((*OpPart=='L') AND (strcmp(OpPart+1,BranchOrders[z].Name)==0))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        Address=EvalIntExpression(ArgStr[1],UInt16,&OK)-EProgCounter()-4;
        if (OK)
         BEGIN
          BAsmCode[0]=0x18;
          BAsmCode[1]=BranchOrders[z].Code;
          BAsmCode[2]=(Address >> 8) & 0xff;
          BAsmCode[3]=Address & 0xff;
          CodeLen=4;
         END
       END
      return;
     END

   for (z=0; z<LoopOrderCount; z++)
    if Memo(LoopOrders[z].Name)
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        if (DecodeReg8(ArgStr[1],&HReg))
         BEGIN
          OK=True; if (HReg==2) HReg=4;
         END
        else if (DecodeReg16(ArgStr[1],&HReg))
         BEGIN
          OK=(HReg!=PCReg); HReg+=5;
         END
        else OK=False;
        if (NOT OK) WrXError(1445,ArgStr[1]);
        else
         BEGIN
          Address=EvalIntExpression(ArgStr[2],UInt16,&OK)-(EProgCounter()+3);
          if (OK)
           if (((Address<-256) OR (Address>255)) AND (NOT SymbolQuestionable)) WrError(1370);
           else
            BEGIN
             BAsmCode[0]=0x04;
             BAsmCode[1]=LoopOrders[z].Code+HReg+((Address >> 4) & 0x10);
             BAsmCode[2]=Address & 0xff;
             CodeLen=3;
            END
         END
       END
      return;
     END

   for (z=0; z<JmpOrderCount; z++)
    if (Memo(JmpOrders[z].Name))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else if (*AttrPart!='\0') WrError(1100);
      else
       BEGIN
        Mask=MModAllIdx+MModExt; if (JmpOrders[z].MayDir) Mask+=MModDir;
        ExPos=1; DecodeAdr(1,ArgCnt,Mask);
        if (AdrMode!=ModNone)
         BEGIN
          switch (AdrMode)
           BEGIN
            case ModExt: BAsmCode[0]=JmpOrders[z].Code; break;
            case ModDir: BAsmCode[0]=JmpOrders[z].Code+1; break;
            case ModIdx:
            case ModIdx1:
            case ModIdx2:
            case ModDIdx:
            case ModIIdx2: BAsmCode[0]=JmpOrders[z].Code-1; break;
           END
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
         END
       END
      return;
     END

   if (Memo("CALL"))
    BEGIN
     if (ArgCnt<1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (*ArgStr[1]=='[')
      BEGIN
       if (ArgCnt!=1) WrError(1110);
       else
        BEGIN
         ExPos=1; DecodeAdr(1,1,MModDIdx+MModIIdx2);
         if (AdrMode!=ModNone)
          BEGIN
           BAsmCode[0]=0x4b;
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
           CodeLen=1+AdrCnt;
          END
        END
      END
     else
      BEGIN
       if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
       else
        BEGIN
         HReg=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
         if (OK)
          BEGIN
           ExPos=2; /* wg. Seiten-Byte eins mehr */
           DecodeAdr(1,ArgCnt-1,MModExt+MModIdx+MModIdx1+MModIdx2);
           if (AdrMode!=ModNone)
            BEGIN
             BAsmCode[0]=0x4a+Ord(AdrMode!=ModExt);
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
             BAsmCode[1+AdrCnt]=HReg;
             CodeLen=2+AdrCnt;
            END
          END
        END
      END
     return;
    END

   if ((Memo("BRSET")) OR (Memo("BRCLR")))
    BEGIN
     if (ArgCnt==1)
      BEGIN
       Try2Split(1); Try2Split(1);
      END
     else if (ArgCnt==2)
      BEGIN
       Try2Split(ArgCnt); Try2Split(2);
      END
     if ((ArgCnt<3) OR (ArgCnt>4)) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       if (*ArgStr[ArgCnt-1]=='#') strcpy(ArgStr[ArgCnt-1],ArgStr[ArgCnt-1]+1);
       HReg=EvalIntExpression(ArgStr[ArgCnt-1],UInt8,&OK);
       if (OK)
        BEGIN
         Address=EvalIntExpression(ArgStr[ArgCnt],UInt16,&OK)-EProgCounter();
         if (OK)
          BEGIN
           ExPos=3; /* Opcode, Maske+Distanz */
           DecodeAdr(1,ArgCnt-2,MModDir+MModExt+MModIdx+MModIdx1+MModIdx2);
           if (AdrMode!=ModNone)
            BEGIN
             BAsmCode[0]=0x0e + Ord(Memo("BRCLR")); /* ANSI :-O */
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
             switch (AdrMode)
              BEGIN
               case ModDir: BAsmCode[0]+=0x40; break;
               case ModExt: BAsmCode[0]+=0x10; break;
              END
             BAsmCode[1+AdrCnt]=HReg;
             Address-=3+AdrCnt;
             if (((Address<-128) OR (Address>127)) AND (NOT SymbolQuestionable)) WrError(1370);
             else
              BEGIN
               BAsmCode[2+AdrCnt]=Address & 0xff;
               CodeLen=3+AdrCnt;
              END
            END
          END
        END
      END
     return;
    END

   if (Memo("TRAP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else
      BEGIN
       FirstPassUnknown=False;
       if (*ArgStr[1]=='#') strcpy(ArgStr[1],ArgStr[1]+1);
       BAsmCode[1]=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if (FirstPassUnknown) BAsmCode[1]=0x30;
       if (OK)
        if ((BAsmCode[1]<0x30) OR ((BAsmCode[1]>0x39) AND (BAsmCode[1]<0x40))) WrError(1320);
        else
         BEGIN
          BAsmCode[0]=0x18; CodeLen=2;
         END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_6812(void)
BEGIN
   return False;
END

        static void SwitchFrom_6812(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_6812(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x66; NOPCode=0xa7;
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   MakeCode=MakeCode_6812; IsDef=IsDef_6812;
   SwitchFrom=SwitchFrom_6812; InitFields();
   AddMoto16PseudoONOFF();

   SetFlag(&DoPadding,DoPaddingName,False);
END

	void code6812_init(void)
BEGIN
   CPU6812=AddCPU("68HC12",SwitchTo_6812);
END
