/* code65.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 65xx/MELPS740                                               */
/*                                                                           */
/* Historie: 17.8.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "codepseudo.h"

/*---------------------------------------------------------------------------*/

#define ModZA    0   /* aa */
#define ModA     1   /* aabb */
#define ModZIX   2   /* aa,X */
#define ModIX    3   /* aabb,X */
#define ModZIY   4   /* aa,Y */
#define ModIY    5   /* aabb,Y */
#define ModIndX  6   /* (aa,X) */
#define ModIndY  7   /* (aa),Y */
#define ModInd16 8   /* (aabb) */
#define ModImm   9   /* #aa */
#define ModAcc  10   /* A */
#define ModNone 11   /* */
#define ModInd8 12   /* (aa) */
#define ModSpec 13   /* \aabb */

typedef struct
         {
	  char *Name;
	  Byte CPUFlag;
	  Byte Code;
	 } FixedOrder;

typedef struct
         {
	  char *Name;
	  Integer Codes[ModSpec+1];
	 } NormOrder;

typedef struct
         {
	  char *Name;
	  Byte CPUFlag;
	  Byte Code;
	 } CondOrder;

#define FixedOrderCount 35
#define NormOrderCount 31
#define CondOrderCount 9


static Boolean CLI_SEI_Flag,ADC_SBC_Flag;

static FixedOrder *FixedOrders;
static NormOrder *NormOrders;
static CondOrder *CondOrders;

static void (*SaveInitProc)(void);
static CPUVar CPU6502,CPU65SC02,CPU65C02,CPUM740;
static LongInt SpecPage;

static ShortInt ErgMode;
static Byte AdrVals[2];
static Byte AdrCnt;

/*---------------------------------------------------------------------------*/

static LongInt InstrZ;

	static void AddFixed(char *NName, Byte NFlag, Byte NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].CPUFlag=NFlag;
   FixedOrders[InstrZ++].Code=NCode;
END

	static void AddNorm(char *NName, Word ZACode, Word ACode, Word ZIXCode,
                            Word IXCode, Word ZIYCode, Word IYCode, Word IndXCode,
                            Word IndYCode, Word Ind16Code, Word ImmCode, Word AccCode,
                            Word NoneCode, Word Ind8Code, Word SpecCode)
BEGIN
   if (InstrZ>=NormOrderCount) exit(255);
   NormOrders[InstrZ].Name=NName;
   NormOrders[InstrZ].Codes[ModZA]=ZACode;
   NormOrders[InstrZ].Codes[ModA]=ACode;
   NormOrders[InstrZ].Codes[ModZIX]=ZIXCode;
   NormOrders[InstrZ].Codes[ModIX]=IXCode;
   NormOrders[InstrZ].Codes[ModZIY]=ZIYCode;  
   NormOrders[InstrZ].Codes[ModIY]=IYCode;
   NormOrders[InstrZ].Codes[ModIndX]=IndXCode;  
   NormOrders[InstrZ].Codes[ModIndY]=IndYCode;
   NormOrders[InstrZ].Codes[ModInd16]=Ind16Code;
   NormOrders[InstrZ].Codes[ModImm]=ImmCode;
   NormOrders[InstrZ].Codes[ModAcc]=AccCode;
   NormOrders[InstrZ].Codes[ModNone]=NoneCode;
   NormOrders[InstrZ].Codes[ModInd8]=Ind8Code;
   NormOrders[InstrZ++].Codes[ModSpec]=SpecCode;
END

        static void AddCond(char *NName, Byte NFlag, Byte NCode)
BEGIN
   if (InstrZ>=CondOrderCount) exit(255);
   CondOrders[InstrZ].Name=NName;
   CondOrders[InstrZ].CPUFlag=NFlag;
   CondOrders[InstrZ++].Code=NCode;
END

	static void InitFields(void)
BEGIN
   Boolean Is740=(MomCPU==CPUM740);

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("RTS", 15, 0x60);  AddFixed("RTI", 15, 0x40);
   AddFixed("TAX", 15, 0xaa);  AddFixed("TXA", 15, 0x8a);
   AddFixed("TAY", 15, 0xa8);  AddFixed("TYA", 15, 0x98);
   AddFixed("TXS", 15, 0x9a);  AddFixed("TSX", 15, 0xba);
   AddFixed("DEX", 15, 0xca);  AddFixed("DEY", 15, 0x88);
   AddFixed("INX", 15, 0xe8);  AddFixed("INY", 15, 0xc8);
   AddFixed("PHA", 15, 0x48);  AddFixed("PLA", 15, 0x68);
   AddFixed("PHP", 15, 0x08);  AddFixed("PLP", 15, 0x28);
   AddFixed("PHX",  6, 0xda);  AddFixed("PLX",  6, 0xfa);
   AddFixed("PHY",  6, 0x5a);  AddFixed("PLY",  6, 0x7a);
   AddFixed("BRK", 15, 0x00);  AddFixed("STP",  8, 0x42);
   AddFixed("SLW",  8, 0xc2);  AddFixed("FST",  8, 0xe2);
   AddFixed("WIT",  8, 0xc2);  AddFixed("NOP", 15, 0xea);
   AddFixed("CLI", 15, 0x58);  AddFixed("SEI", 15, 0x78);
   AddFixed("CLC", 15, 0x18);  AddFixed("SEC", 15, 0x38);
   AddFixed("CLD", 15, 0xd8);  AddFixed("SED", 15, 0xf8);
   AddFixed("CLV", 15, 0xb8);  AddFixed("CLT",  8, 0x12);
   AddFixed("SET",  8, 0x32);

   NormOrders=(NormOrder *) malloc(sizeof(NormOrder)*NormOrderCount); InstrZ=0;
	       /*    ZA      A    ZIX     IX    ZIY     IY     @X     @Y  (n16)    imm    ACC    NON   (n8)   spec */
   AddNorm("LDA",0x0fa5,0x0fad,0x0fb5,0x0fbd,    -1,0x0fb9,0x0fa1,0x0fb1,    -1,0x0fa9,    -1,    -1,0x06b2,    -1);
   AddNorm("LDX",0x0fa6,0x0fae,    -1,    -1,0x0fb6,0x0fbe,    -1,    -1,    -1,0x0fa2,    -1,    -1,    -1,    -1);
   AddNorm("LDY",0x0fa4,0x0fac,0x0fb4,0x0fbc,    -1,    -1,    -1,    -1,    -1,0x0fa0,    -1,    -1,    -1,    -1);
   AddNorm("STA",0x0f85,0x0f8d,0x0f95,0x0f9d,    -1,0x0f99,0x0f81,0x0f91,    -1,    -1,    -1,    -1,0x0692,    -1);
   AddNorm("STX",0x0f86,0x0f8e,    -1,    -1,0x0f96,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("STY",0x0f84,0x0f8c,0x0f94,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("STZ",0x0664,0x069c,0x0674,0x069e,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("ADC",0x0f65,0x0f6d,0x0f75,0x0f7d,    -1,0x0f79,0x0f61,0x0f71,    -1,0x0f69,    -1,    -1,0x0672,    -1);
   AddNorm("SBC",0x0fe5,0x0fed,0x0ff5,0x0ffd,    -1,0x0ff9,0x0fe1,0x0ff1,    -1,0x0fe9,    -1,    -1,0x06f2,    -1);
   AddNorm("MUL",    -1,    -1,0x0862,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("DIV",    -1,    -1,0x08e2,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("AND",0x0f25,0x0f2d,0x0f35,0x0f3d,    -1,0x0f39,0x0f21,0x0f31,    -1,0x0f29,    -1,    -1,0x0632,    -1);
   AddNorm("ORA",0x0f05,0x0f0d,0x0f15,0x0f1d,    -1,0x0f19,0x0f01,0x0f11,    -1,0x0f09,    -1,    -1,0x0612,    -1);
   AddNorm("EOR",0x0f45,0x0f4d,0x0f55,0x0f5d,    -1,0x0f59,0x0f41,0x0f51,    -1,0x0f49,    -1,    -1,0x0652,    -1);
   AddNorm("COM",0x0844,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("BIT",0x0f24,0x0f2c,0x0634,0x063c,    -1,    -1,    -1,    -1,    -1,0x0689,    -1,    -1,    -1,    -1);
   AddNorm("TST",0x0864,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("ASL",0x0f06,0x0f0e,0x0f16,0x0f1e,    -1,    -1,    -1,    -1,    -1,    -1,0x0f0a,0x0f0a,    -1,    -1);
   AddNorm("LSR",0x0f46,0x0f4e,0x0f56,0x0f5e,    -1,    -1,    -1,    -1,    -1,    -1,0x0f4a,0x0f4a,    -1,    -1);
   AddNorm("ROL",0x0f26,0x0f2e,0x0f36,0x0f3e,    -1,    -1,    -1,    -1,    -1,    -1,0x0f2a,0x0f2a,    -1,    -1);
   AddNorm("ROR",0x0f66,0x0f6e,0x0f76,0x0f7e,    -1,    -1,    -1,    -1,    -1,    -1,0x0f6a,0x0f6a,    -1,    -1);
   AddNorm("RRF",0x0882,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("TSB",0x0604,0x060c,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("TRB",0x0614,0x061c,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1);
   AddNorm("INC",0x0fe6,0x0fee,0x0ff6,0x0ffe,    -1,    -1,    -1,    -1,    -1,    -1,
                                                                                       (Is740)?0x0e3a:0x0e1a,
                                                                                              (Is740)?0x0e3a:0x0e1a,
                                                                                                         -1,    -1);
   AddNorm("DEC",0x0fc6,0x0fce,0x0fd6,0x0fde,    -1,    -1,    -1,    -1,    -1,    -1,(Is740)?0x0e1a:0x0e3a,
                                                                                              (Is740)?0x0e1a:0x0e3a,
                                                                                                         -1,    -1);
   AddNorm("CMP",0x0fc5,0x0fcd,0x0fd5,0x0fdd,    -1,0x0fd9,0x0fc1,0x0fd1,    -1,0x0fc9,    -1,    -1,0x06d2,    -1);
   AddNorm("CPX",0x0fe4,0x0fec,    -1,    -1,    -1,    -1,    -1,    -1,    -1,0x0fe0,    -1,    -1,    -1,    -1);
   AddNorm("CPY",0x0fc4,0x0fcc,    -1,    -1,    -1,    -1,    -1,    -1,    -1,0x0fc0,    -1,    -1,    -1,    -1);
   AddNorm("JMP",    -1,0x0f4c,    -1,    -1,    -1,    -1,0x067c,    -1,0x0f6c,    -1,    -1,    -1,0x08b2,    -1);
   AddNorm("JSR",    -1,0x0f20,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,0x0802,0x0822);

   CondOrders=(CondOrder *) malloc(sizeof(CondOrder)*CondOrderCount); InstrZ=0;
   AddCond("BEQ", 15, 0xf0);
   AddCond("BNE", 15, 0xd0);
   AddCond("BPL", 15, 0x10);
   AddCond("BMI", 15, 0x30);
   AddCond("BCC", 15, 0x90);
   AddCond("BCS", 15, 0xb0);
   AddCond("BVC", 15, 0x50);
   AddCond("BVS", 15, 0x70);
   AddCond("BRA", 14, 0x80);
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(NormOrders);
   free(CondOrders);
END

/*---------------------------------------------------------------------------*/

	static void ChkZero(char *Asc, Byte *erg)
BEGIN
   if ((strlen(Asc)>1) AND ((*Asc=='<') OR (*Asc=='>')))
    BEGIN
     *erg=Ord(*Asc=='<')+1; strcpy(Asc,Asc+1);
    END
   else *erg=0;
END

	static Boolean DecodePseudo(void)
BEGIN
#define ASSUME740Count 1
   static ASSUMERec ASSUME740s[ASSUME740Count]=
 	            {{"SP", &SpecPage, 0, 0xff, -1}};

   if (Memo("ASSUME"))
    BEGIN
     if (MomCPU!=CPUM740) WrError(1500);
     else CodeASSUME(ASSUME740s,ASSUME740Count);
     return True;
    END

   return False;
END

	static void ChkFlags(void)
BEGIN
   /* Spezialflags ? */

   CLI_SEI_Flag=(Memo("CLI") OR Memo("SEI"));
   ADC_SBC_Flag=(Memo("ADC") OR Memo("SBC"));
END

	static Boolean CPUAllowed(Byte Flag)
BEGIN
   return (((Flag >> (MomCPU-CPU6502))&1)==1);
END

	static void InsNOP(void)
BEGIN
   memmove(BAsmCode,BAsmCode+1,CodeLen);
   CodeLen++; BAsmCode[0]=NOPCode;
END

	static Boolean IsAllowed(Word Val)
BEGIN
   return (CPUAllowed(Hi(Val)) AND (Val!=0xffff));
END

	static void ChkZeroMode(ShortInt Mode)
BEGIN
   Integer OrderZ;

   for (OrderZ=0; OrderZ<NormOrderCount; OrderZ++)
    if (Memo(NormOrders[OrderZ].Name))
     BEGIN
      if (IsAllowed(NormOrders[OrderZ].Codes[Mode]))
       BEGIN
	ErgMode=Mode; AdrCnt--;
       END
      return;
     END
END

	static void MakeCode_65(void)
BEGIN
   Word OrderZ;
   Byte AdrByte;
   Integer AdrInt;
   Word AdrWord;
   String s1;
   Boolean ValOK,b;
   Byte ZeroMode;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) { ChkFlags(); return; }

   /* Pseudoanweisungen */

   if (DecodePseudo()) { ChkFlags(); return; }

   if (DecodeMotoPseudo(False)) { ChkFlags(); return; }

   /* Anweisungen ohne Argument */

   for (OrderZ=0; OrderZ<FixedOrderCount; OrderZ++)
    if (Memo(FixedOrders[OrderZ].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (NOT CPUAllowed(FixedOrders[OrderZ].CPUFlag)) WrError(1500);
      else
       BEGIN
        CodeLen=1;
        BAsmCode[0]=FixedOrders[OrderZ].Code;
        if (Memo("BRK")) BAsmCode[CodeLen++]=NOPCode;
        else if (MomCPU==CPUM740)
	 BEGIN
          if (Memo("PLP")) BAsmCode[CodeLen++]=NOPCode; 
	  if ((ADC_SBC_Flag) AND (Memo("SEC") OR Memo("CLC") OR Memo("CLD"))) InsNOP();
	 END
       END
      ChkFlags(); return;
     END

    if ((Memo("SEB")) OR (Memo("CLB")))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (MomCPU!=CPUM740) WrError(1500);
      else
       BEGIN
	AdrByte=EvalIntExpression(ArgStr[1],UInt3,&ValOK);
	if (ValOK)
	 BEGIN
	  BAsmCode[0]=0x0b+(AdrByte << 5)+(Ord(Memo("CLB")) << 4);
	  if (strcasecmp(ArgStr[2],"A")==0) CodeLen=1;
	  else
	   BEGIN
            BAsmCode[1]=EvalIntExpression(ArgStr[2],UInt8,&ValOK);
	    if (ValOK)
	     BEGIN
	      CodeLen=2; BAsmCode[0]+=4;
	     END
	   END
	 END
       END
      ChkFlags(); return;
     END

    if ((Memo("BBC")) OR (Memo("BBS")))
     BEGIN
      if (ArgCnt!=3) WrError(1110);
      else if (MomCPU!=CPUM740) WrError(1500);
      else
       BEGIN
	BAsmCode[0]=EvalIntExpression(ArgStr[1],UInt3,&ValOK);
	if (ValOK)
	 BEGIN
	  BAsmCode[0]=(BAsmCode[0] << 5)+(Ord(Memo("BBC")) << 4)+3;
	  b=(strcasecmp(ArgStr[2],"A")!=0);
	  if (NOT b) ValOK=True;
	  else
	   BEGIN
	    BAsmCode[0]+=4;
            BAsmCode[1]=EvalIntExpression(ArgStr[2],UInt8,&ValOK);
	   END
	  if (ValOK)
	   BEGIN
	    AdrInt=EvalIntExpression(ArgStr[3],Int16,&ValOK)-(EProgCounter()+2+Ord(b)+Ord(CLI_SEI_Flag));
	    if (ValOK)
	     if (((AdrInt>127) OR (AdrInt<-128)) AND (NOT SymbolQuestionable)) WrError(1370);
	     else
	      BEGIN
	       CodeLen=2+Ord(b);
	       BAsmCode[CodeLen-1]=AdrInt & 0xff;
	       if (CLI_SEI_Flag) InsNOP();
	      END
	   END
	 END
       END
      ChkFlags(); return;
     END

    if (((strlen(OpPart)==4)
    AND  (OpPart[3]>='0') AND (OpPart[3]<='7')
    AND  ((strncmp(OpPart,"BBR",3)==0) OR (strncmp(OpPart,"BBS",3)==0))))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (MomCPU!=CPU65C02) WrError(1500);
      else
       BEGIN
	BAsmCode[1]=EvalIntExpression(ArgStr[1],UInt8,&ValOK);
	if (ValOK)
	 BEGIN
	  BAsmCode[0]=((OpPart[3]-'0') << 4)+(Ord(OpPart[2]=='S') << 7)+15;
          AdrInt=EvalIntExpression(ArgStr[2],UInt16,&ValOK)-(EProgCounter()+3);
	  if (ValOK)
	   if (((AdrInt>127) OR (AdrInt<-128)) AND (NOT SymbolQuestionable)) WrError(1370);
	   else
	    BEGIN
	     CodeLen=3;
	     BAsmCode[2]=AdrInt & 0xff;
	    END
	 END
       END
      ChkFlags(); return;
     END

    if (((strlen(OpPart)==4)
    AND  (OpPart[3]>='0') AND (OpPart[3]<='7')
    AND  ((strncmp(OpPart,"RMB",3)==0) OR (strncmp(OpPart,"SMB",3)==0))))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU!=CPU65C02) WrError(1500);
      else
       BEGIN
	BAsmCode[1]=EvalIntExpression(ArgStr[1],UInt8,&ValOK);
	if (ValOK)
	 BEGIN
	  BAsmCode[0]=((OpPart[3]-'0') << 4)+(Ord(*OpPart=='S') << 7)+7;
          CodeLen=2;
	 END
       END
      ChkFlags(); return;
     END

   if (Memo("LDM"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU!=CPUM740) WrError(1500);
     else
      BEGIN
       BAsmCode[0]=0x3c;
       BAsmCode[2]=EvalIntExpression(ArgStr[2],UInt8,&ValOK);
       if (ValOK)
	if (*ArgStr[1]!='#') WrError(1350);
	else
	 BEGIN
          BAsmCode[1]=EvalIntExpression(ArgStr[1]+1,Int8,&ValOK);
	  if (ValOK) CodeLen=3;
	 END
      END
     ChkFlags(); return;
    END

   /* normale Anweisungen: Adressausdruck parsen */

   ErgMode=-1;

   if (ArgCnt==0)
    BEGIN
     AdrCnt=0; ErgMode=ModNone;
    END

   else if (ArgCnt==1)
    BEGIN
     /* 1. Akkuadressierung */

     if (strcasecmp(ArgStr[1],"A")==0)
      BEGIN
       AdrCnt=0; ErgMode=ModAcc;
      END

     /* 2. immediate ? */

     else if (*ArgStr[1]=='#')
      BEGIN
       AdrVals[0]=EvalIntExpression(ArgStr[1]+1,Int8,&ValOK);
       if (ValOK)
	BEGIN
	 ErgMode=ModImm; AdrCnt=1;
	END
      END

     /* 3. Special Page ? */

     else if (*ArgStr[1]=='\\')
      BEGIN
       AdrWord=EvalIntExpression(ArgStr[1]+1,UInt16,&ValOK);
       if (ValOK)
	if (Hi(AdrWord)!=SpecPage) WrError(1315);
	else
	 BEGIN
	  ErgMode=ModSpec; AdrVals[0]=Lo(AdrWord); AdrCnt=1;
	 END
      END

     /* 4. X-indirekt ? */

     else if ((strlen(ArgStr[1])>=5) AND (strcasecmp(ArgStr[1]+strlen(ArgStr[1])-3,",X)")==0))
      BEGIN
       if (*ArgStr[1]!='(') WrError(1350);
       else
	BEGIN
         strmaxcpy(s1,ArgStr[1]+1,255); s1[strlen(s1)-3]='\0';
	 ChkZero(s1,&ZeroMode);
         if (Memo("JMP"))
          BEGIN
           AdrWord=EvalIntExpression(s1,UInt16,&ValOK);
           if (ValOK)
            BEGIN
             AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
             ErgMode=ModIndX; AdrCnt=2;
            END
          END
         else
          BEGIN
           AdrVals[0]=EvalIntExpression(s1,UInt8,&ValOK);
           if (ValOK)
            BEGIN
             ErgMode=ModIndX; AdrCnt=1;
            END
          END
	END
      END

     else
      BEGIN
       /* 5. indirekt absolut ? */

       if (IsIndirect(ArgStr[1]))
	BEGIN
         strcpy(s1,ArgStr[1]+1); s1[strlen(s1)-1]='\0';
	 ChkZero(s1,&ZeroMode);
	 if (ZeroMode==2)
	  BEGIN
           AdrVals[0]=EvalIntExpression(s1,UInt8,&ValOK);
	   if (ValOK)
	    BEGIN
	     ErgMode=ModInd8; AdrCnt=1;
	    END
	  END
	 else
	  BEGIN
	   AdrWord=EvalIntExpression(s1,UInt16,&ValOK);
	   if (ValOK)
	    BEGIN
	     ErgMode=ModInd16; AdrCnt=2;
             AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
	     if ((ZeroMode==0) AND (AdrVals[1]==0)) ChkZeroMode(ModInd8);
	    END
	  END
	END

       /* 6. absolut */

       else
	BEGIN
	 ChkZero(ArgStr[1],&ZeroMode);
	 if (ZeroMode==2)
	  BEGIN
           AdrVals[0]=EvalIntExpression(ArgStr[1],UInt8,&ValOK);
	   if (ValOK)
	    BEGIN
	     ErgMode=ModZA; AdrCnt=1;
	    END
	  END
	 else
	  BEGIN
	   AdrWord=EvalIntExpression(ArgStr[1],UInt16,&ValOK);
	   if (ValOK)
	    BEGIN
	     ErgMode=ModA; AdrCnt=2;
             AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
	     if ((ZeroMode==0) AND (AdrVals[1]==0)) ChkZeroMode(ModZA);
	    END
	  END
	END
      END
    END

   else if (ArgCnt==2)
    BEGIN
     /* 7. Y-indirekt ? */

     if ((IsIndirect(ArgStr[1])) AND (strcasecmp(ArgStr[2],"Y")==0))
      BEGIN
       strcpy(s1,ArgStr[1]+1); s1[strlen(s1)-1]='\0';
       ChkZero(s1,&ZeroMode);
       AdrVals[0]=EvalIntExpression(s1,UInt8,&ValOK);
       if (ValOK)
	BEGIN
	 ErgMode=ModIndY; AdrCnt=1;
	END
      END

     /* 8. X,Y-indiziert ? */

     else
      BEGIN
       strcpy(s1,ArgStr[1]); 
       ChkZero(s1,&ZeroMode);
       if (ZeroMode==2)
	BEGIN
         AdrVals[0]=EvalIntExpression(s1,UInt8,&ValOK);
	 if (ValOK)
	  BEGIN
	   AdrCnt=1;
           ErgMode=(strcasecmp(ArgStr[2],"X")==0)?ModZIX:ModZIY;
	  END
	END
       else
	BEGIN
	 AdrWord=EvalIntExpression(s1,Int16,&ValOK);
	 if (ValOK)
	  BEGIN
	   AdrCnt=2;
           AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
           ErgMode=(strcasecmp(ArgStr[2],"X")==0)?ModIX:ModIY;
	   if ((AdrVals[1]==0) AND (ZeroMode==0))
            ChkZeroMode((strcasecmp(ArgStr[2],"X")==0)?ModZIX:ModZIY);
	  END
	END
      END
    END

   else
    BEGIN
     WrError(1110);
     ChkFlags(); return;
    END;

   /* in Tabelle nach Opcode suchen */

   for (OrderZ=0; OrderZ<NormOrderCount; OrderZ++)
    if (Memo(NormOrders[OrderZ].Name))
     BEGIN
      if ((ErgMode==-1)) WrError(1350);
      else
       BEGIN
	if (NormOrders[OrderZ].Codes[ErgMode]==-1)
	 BEGIN
	  if (ErgMode==ModZA) ErgMode=ModA;
	  if (ErgMode==ModZIX) ErgMode=ModIX;
	  if (ErgMode==ModZIY) ErgMode=ModIY;
	  if (ErgMode==ModInd8) ErgMode=ModInd16;
	  AdrVals[AdrCnt++]=0;
	 END
	if (NormOrders[OrderZ].Codes[ErgMode]==-1) WrError(1350);
	else if (NOT CPUAllowed(Hi(NormOrders[OrderZ].Codes[ErgMode]))) WrError(1500);
	else
	 BEGIN
	  BAsmCode[0]=Lo(NormOrders[OrderZ].Codes[ErgMode]); 
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
	  CodeLen=AdrCnt+1;
	  if ((ErgMode==ModInd16) AND (MomCPU!=CPU65C02) AND (BAsmCode[1]==0xff))
	   BEGIN
	    WrError(1900); CodeLen=0;
	   END
	 END
       END
      ChkFlags(); return;
     END

   /* relativer Sprung ? */

   if (ErgMode==ModZA)
    BEGIN
     ErgMode=ModA; AdrVals[1]=0;
    END
   if (ErgMode==ModA)
    for (OrderZ=0; OrderZ<CondOrderCount; OrderZ++)
     if (Memo(CondOrders[OrderZ].Name))
      BEGIN
       AdrInt=(((Integer)AdrVals[1])<<8)+AdrVals[0];
       AdrInt-=EProgCounter()+2;
       if (NOT CPUAllowed(CondOrders[OrderZ].CPUFlag)) WrError(1500);
       else if (((AdrInt>127) OR (AdrInt<-128)) AND (NOT SymbolQuestionable)) WrError(1370);
       else
        BEGIN
         BAsmCode[0]=CondOrders[OrderZ].Code; BAsmCode[1]=AdrInt & 0xff; 
         CodeLen=2;
        END
       ChkFlags(); return;
      END

   WrXError(1200,OpPart);
END

	static void InitCode_65(void)
BEGIN
   SaveInitProc();
   CLI_SEI_Flag=False;
   ADC_SBC_Flag=False;
END

	static Boolean ChkPC_65(void)
BEGIN
   if (ActPC==SegCode) return (ProgCounter()<0x10000);
   else return False;
END

	static Boolean IsDef_65(void)
BEGIN
   return False;
END

        static void SwitchFrom_65(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_65(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=(MomCPU==CPUM740);

   PCSymbol="*";  HeaderID=0x11; NOPCode=0xea;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_65; ChkPC=ChkPC_65; IsDef=IsDef_65;
   SwitchFrom=SwitchFrom_65; InitFields();
END

	void code65_init(void)
BEGIN
   CPU6502  =AddCPU("6502"    ,SwitchTo_65);
   CPU65SC02=AddCPU("65SC02"  ,SwitchTo_65);
   CPU65C02 =AddCPU("65C02"   ,SwitchTo_65);
   CPUM740  =AddCPU("MELPS740",SwitchTo_65);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_65;
END
