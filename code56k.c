/* code56k.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegeneratormodul fuer die DSP56K-Familie                             */
/*                                                                           */
/* Historie: 10. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"

#include "code56k.h"

typedef struct 
         {
          char *Name;
          LongWord Code;
         } FixedOrder;

typedef enum {ParAB,ParXYAB,ParABXYnAB,ParABBA,ParXYnAB,ParMul} ParTyp;
typedef struct 
         {
          char *Name;
          ParTyp Typ;
          Byte Code;
         } ParOrder;

#define FixedOrderCnt 9

#define ParOrderCnt 29

#define BitOrderCnt 4
static char *BitOrders[BitOrderCnt]={"BCLR","BSET","BCHG","BTST"};

#define BitJmpOrderCnt 4
static char *BitJmpOrders[BitOrderCnt]={"JCLR","JSET","JSCLR","JSSET"};

static Byte MacTable[4][4]={{0,2,5,4},{2,0xff,6,7},{5,6,1,3},{4,7,3,0xff}};

#define ModNone (-1)
#define ModImm 0
#define MModImm (1 << ModImm)
#define ModAbs 1
#define MModAbs (1 << ModAbs)
#define ModIReg 2
#define MModIReg (1 << ModIReg)
#define ModPreDec 3
#define MModPreDec (1 << ModPreDec)
#define ModPostDec 4
#define MModPostDec (1 << ModPostDec)
#define ModPostInc 5
#define MModPostInc (1 << ModPostInc)
#define ModIndex 6
#define MModIndex (1 << ModIndex)
#define ModModDec 7
#define MModModDec (1 << ModModDec)
#define ModModInc 8
#define MModModInc (1 << ModModInc)

#define MModNoExt (MModIReg+MModPreDec+MModPostDec+MModPostInc+MModIndex+MModModDec+MModModInc)
#define MModNoImm (MModAbs+MModNoExt)
#define MModAll (MModNoImm+MModImm)

#define SegLData (SegYData+1)

#define MSegCode (1 << SegCode)
#define MSegXData (1 << SegXData)
#define MSegYData (1 << SegYData)
#define MSegLData (1 << SegLData)

static CPUVar CPU56000;
static ShortInt AdrType;
static Word AdrMode;
static LongInt AdrVal;
static Byte AdrSeg;

static FixedOrder *FixedOrders;
static ParOrder *ParOrders;

/*----------------------------------------------------------------------------------------------*/

	static void AddFixed(char *Name, LongWord Code) 
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   
   FixedOrders[InstrZ].Name=Name;
   FixedOrders[InstrZ++].Code=Code;
END

	static void AddPar(char *Name, ParTyp Typ, LongWord Code) 
BEGIN
   if (InstrZ>=ParOrderCnt) exit(255);
   
   ParOrders[InstrZ].Name=Name;
   ParOrders[InstrZ].Typ=Typ;
   ParOrders[InstrZ++].Code=Code;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("NOP"    , 0x000000);
   AddFixed("ENDDO"  , 0x00008c);
   AddFixed("ILLEGAL", 0x000005);
   AddFixed("RESET"  , 0x000084);
   AddFixed("RTI"    , 0x000004);
   AddFixed("RTS"    , 0x00000c);
   AddFixed("STOP"   , 0x000087);
   AddFixed("SWI"    , 0x000006);
   AddFixed("WAIT"   , 0x000086);

   ParOrders=(ParOrder *) malloc(sizeof(ParOrder)*ParOrderCnt); InstrZ=0;
   AddPar("ABS" ,ParAB,     0x26);
   AddPar("ASL" ,ParAB,     0x32);
   AddPar("ASR" ,ParAB,     0x22);
   AddPar("CLR" ,ParAB,     0x13);
   AddPar("LSL" ,ParAB,     0x33);
   AddPar("LSR" ,ParAB,     0x23);
   AddPar("NEG" ,ParAB,     0x36);
   AddPar("NOT" ,ParAB,     0x17);
   AddPar("RND" ,ParAB,     0x11);
   AddPar("ROL" ,ParAB,     0x37);
   AddPar("ROR" ,ParAB,     0x27);
   AddPar("TST" ,ParAB,     0x03);
   AddPar("ADC" ,ParXYAB,   0x21);
   AddPar("SBC" ,ParXYAB,   0x25);
   AddPar("ADD" ,ParABXYnAB,0x00);
   AddPar("CMP" ,ParABXYnAB,0x05);
   AddPar("CMPM",ParABXYnAB,0x07);
   AddPar("SUB" ,ParABXYnAB,0x04);
   AddPar("ADDL",ParABBA,   0x12);
   AddPar("ADDR",ParABBA,   0x02);
   AddPar("SUBL",ParABBA,   0x16);
   AddPar("SUBR",ParABBA,   0x06);
   AddPar("AND" ,ParXYnAB,  0x46);
   AddPar("EOR" ,ParXYnAB,  0x43);
   AddPar("OR"  ,ParXYnAB,  0x42);
   AddPar("MAC" ,ParMul,    0x82);
   AddPar("MACR",ParMul,    0x83);
   AddPar("MPY" ,ParMul,    0x80);
   AddPar("MPYR",ParMul,    0x81);
END

	static void DeinitFields(void)
BEGIN
  free(FixedOrders);
  free(ParOrders);
END

/*----------------------------------------------------------------------------------------------*/

	static void SplitArg(char *Orig, char *LDest, char *RDest)
BEGIN
   char *p,*str;

   str=strdup(Orig);
   p=QuotPos(str,',');
   if (p==Nil)
    BEGIN
     *RDest='\0'; strcpy(LDest,str);
    END
   else
    BEGIN
     *p='\0';
      strcpy(LDest,str); strcpy(RDest,p+1);
    END
   free(str);
END

	static void ChkMask(Word Erl, Byte ErlSeg)
BEGIN
   if ((AdrType!=ModNone) AND ((Erl & (1 << AdrType))==0))
    BEGIN
     WrError(1350); AdrCnt=0; AdrType=ModNone;
    END
   if ((AdrSeg!=SegNone) AND ((ErlSeg & (1 << AdrSeg))==0))
    BEGIN
     WrError(1960); AdrCnt=0; AdrType=ModNone;
    END
END

	static void DecodeAdr(char *Asc_O, Word Erl, Byte ErlSeg)
BEGIN
   static char *ModMasks[ModModInc+1]=
	    {"","","(Rx)","-(Rx)","(Rx)-","(Rx)+","(Rx+Nx)","(Rx)-Nx","(Rx)+Nx"};
   static Byte ModCodes[ModModInc+1]=
	    {0,0,4,7,2,3,5,0,1};
#define SegCount 4
   static char SegNames[SegCount]={'P','X','Y','L'};
   static Byte SegVals[SegCount]={SegCode,SegXData,SegYData,SegLData};
   Integer z,l;
   Boolean OK;
   Byte OrdVal;
   String Asc;

   AdrType=ModNone; AdrCnt=0; strmaxcpy(Asc,Asc_O,255);

   /* Defaultsegment herausfinden */

   if ((ErlSeg & MSegXData)!=0) AdrSeg=SegXData;
   else if ((ErlSeg & MSegYData)!=0) AdrSeg=SegYData;
   else if ((ErlSeg & MSegCode)!=0) AdrSeg=SegCode;
   else AdrSeg=SegNone;

   /* Zielsegment vorgegeben ? */

   for (z=0; z<SegCount; z++)
    if ((toupper(*Asc)==SegNames[z]) AND (Asc[1]==':'))
     BEGIN
      AdrSeg=SegVals[z]; strcpy(Asc,Asc+2);
     END

   /* Adressausdruecke abklopfen: dazu mit Referenzstring vergleichen */

   for (z=ModIReg; z<=ModModInc; z++)
    if (strlen(Asc)==strlen(ModMasks[z]))
     BEGIN
      AdrMode=0xffff;
      for (l=0; l<=strlen(Asc); l++)
       if (ModMasks[z][l]=='x')
	BEGIN
	 OrdVal=Asc[l]-'0';
	 if (OrdVal>7) break;
	 else if (AdrMode==0xffff) AdrMode=OrdVal;
	 else if (AdrMode!=OrdVal)
	  BEGIN
	   WrError(1760); ChkMask(Erl,ErlSeg); return;
	  END
	END
       else if (ModMasks[z][l]!=toupper(Asc[l])) break;
       if (l>strlen(Asc))
        BEGIN
	 AdrType=z; AdrMode+=ModCodes[z] << 3;
	 ChkMask(Erl,ErlSeg); return;
        END
     END

   /* immediate ? */

   if (*Asc=='#')
    BEGIN
     AdrVal=EvalIntExpression(Asc+1,Int24,&OK);
     if (OK)
      BEGIN
       AdrType=ModImm; AdrCnt=1; AdrMode=0x34; 
       ChkMask(Erl,ErlSeg); return;
      END
    END

   /* dann absolut */

   AdrVal=EvalIntExpression(Asc,Int16,&OK);
   if (OK)
    BEGIN
     AdrType=ModAbs; AdrMode=0x30; AdrCnt=1;
     if ((AdrSeg & ((1<<SegCode)|(1<<SegXData)|(1<<SegYData)))!=0) ChkSpace(AdrSeg);
     ChkMask(Erl,ErlSeg); return;
    END
END

	static Boolean DecodeReg(char *Asc, LongInt *Erg)
BEGIN
#define RegCount 12
   static char *RegNames[RegCount]=
	    {"X0","X1","Y0","Y1","A0","B0","A2","B2","A1","B1","A","B"};
   Word z;

   for (z=0; z<RegCount; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      *Erg=z+4; return True;
     END
   if ((strlen(Asc)==2) AND (Asc[1]>='0') AND (Asc[1]<='7'))
    switch (toupper(*Asc))
     BEGIN
      case 'R':
       *Erg=16+Asc[1]-'0'; return True;
      case 'N':
       *Erg=24+Asc[1]-'0'; return True;
     END
   return False;
END

	static Boolean DecodeALUReg(char *Asc, LongInt *Erg, 
                                    Boolean MayX, Boolean MayY, Boolean MayAcc)
BEGIN
   Boolean Result=False;

   if (NOT DecodeReg(Asc,Erg)) return Result;
   switch (*Erg)
    BEGIN
     case 4:
     case 5:
      if (MayX)
	BEGIN
	 Result=True; (*Erg)-=4;
	END
      break;
     case 6:
     case 7:
      if (MayY)
       BEGIN
	Result=True; (*Erg)-=6;
       END
      break;
     case 14:
     case 15:
      if (MayAcc)
       BEGIN
	Result=True;
	if ((MayX) OR (MayY)) (*Erg)-=12; else (*Erg)-=14;
       END
      break;
    END

   return Result;
END

	static Boolean DecodeLReg(char *Asc, LongInt *Erg)
BEGIN
#undef RegCount
#define RegCount 8
   static char *RegNames[RegCount]=
	    {"A10","B10","X","Y","A","B","AB","BA"};
   Word z;

   for (z=0; z<RegCount; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      *Erg=z; return True;
     END

   return False;
END

	static Boolean DecodeXYABReg(char *Asc, LongInt *Erg)
BEGIN
#undef RegCount
#define RegCount 8
   static char *RegNames[RegCount]=
	    {"B","A","X","Y","X0","Y0","X1","Y1"};
   Word z;

   for (z=0; z<RegCount; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      *Erg=z; return True;
     END

   return False;
END

	static Boolean DecodePCReg(char *Asc, LongInt *Erg)
BEGIN
#undef RegCount
#define RegCount 7
   static char *RegNames[RegCount]={"SR","OMR","SP","SSH","SSL","LA","LC"};
   Word z;

   for (z=0; z<RegCount; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      (*Erg)=z+1; return True;
     END

   return False;
END

	static Boolean DecodeGeneralReg(char *Asc, LongInt *Erg)
BEGIN
   if (DecodeReg(Asc,Erg)) return True;
   if (DecodePCReg(Asc,Erg))
    BEGIN
     (*Erg)+=0x38; return True;
    END
   if ((strlen(Asc)==2) AND (toupper(*Asc)=='M') AND (Asc[1]>='0') AND (Asc[1]<='7'))
    BEGIN
     *Erg=0x20+Asc[1]-'0'; return True;;
    END
   return False;
END

	static Boolean DecodeControlReg(char *Asc, LongInt *Erg)
BEGIN
   Boolean Result=True;

   if (strcasecmp(Asc,"MR")==0) *Erg=0;
   else if (strcasecmp(Asc,"CCR")==0) *Erg=1;
   else if (strcasecmp(Asc,"OMR")==0) *Erg=2;
   else Result=False;

   return Result;
END

	static Boolean DecodeOpPair(char *Left, char *Right, Byte WorkSeg,
			      LongInt *Dir, LongInt *Reg1, LongInt *Reg2,
			      LongInt *AType, LongInt *AMode, LongInt *ACnt,
                              LongInt *AVal)
BEGIN
   Boolean Result=False;

   if (DecodeALUReg(Left,Reg1,WorkSeg==SegXData,WorkSeg==SegYData,True))
    BEGIN
     if (DecodeALUReg(Right,Reg2,WorkSeg==SegXData,WorkSeg==SegYData,True))
      BEGIN
       *Dir=2; Result=True;
      END
     else
      BEGIN
       *Dir=0; *Reg2=(-1);
       DecodeAdr(Right,MModNoImm,1 << WorkSeg);
       if (AdrType!=ModNone)
	BEGIN
	 *AType=AdrType; *AMode=AdrMode; *ACnt=AdrCnt; *AVal=AdrVal;
	 Result=True;
	END
      END
    END
   else if (DecodeALUReg(Right,Reg1,WorkSeg==SegXData,WorkSeg==SegYData,True))
    BEGIN
     *Dir=1; *Reg2=(-1);
     DecodeAdr(Left,MModAll,1 << WorkSeg);
     if (AdrType!=ModNone)
      BEGIN
       *AType=AdrType; *AMode=AdrMode; *ACnt=AdrCnt; *AVal=AdrVal;
       Result=True;
      END
    END

   return Result;
END

	static LongInt TurnXY(LongInt Inp)
BEGIN
   switch (Inp)
    BEGIN
     case 4:
     case 7:
      return Inp-4;
     case 5:
     case 6:
      return 7-Inp;
     default:  /* wird nie erreicht */
      return 0;
    END
END

	static Boolean DecodeTFR(char *Asc, LongInt *Erg)
BEGIN
   LongInt Part1,Part2;
   String Left,Right;

   SplitArg(Asc,Left,Right);
   if (NOT DecodeALUReg(Right,&Part2,False,False,True)) return False;
   if (NOT DecodeReg(Left,&Part1)) return False;
   if ((Part1<4) OR ((Part1>7) AND (Part1<14)) OR (Part1>15)) return False;
   if (Part1>13)
    if (((Part1 ^ Part2) & 1)==0) return False;
    else Part1=0;
   else Part1=TurnXY(Part1)+4;
   *Erg=(Part1 << 1)+Part2;
   return True;
END

	static Boolean DecodeMOVE_0(void)
BEGIN
   DAsmCode[0]=0x200000; CodeLen=1;
   return True;
END

	static Boolean DecodeMOVE_1(Integer Start)
BEGIN
   String Left,Right;
   LongInt RegErg,RegErg2,IsY,MixErg;
   Boolean Result=False;

   SplitArg(ArgStr[Start],Left,Right);

   /* 1. Register-Update */

   if (*Right=='\0')
    BEGIN
     DecodeAdr(Left,MModPostDec+MModPostInc+MModModDec+MModModInc,0);
     if (AdrType!=ModNone)
      BEGIN
       Result=True;
       DAsmCode[0]=0x204000+(AdrMode << 8);
       CodeLen=1;
      END
     return Result;
    END

   /* 2. Ziel ist Register */

   if (DecodeReg(Right,&RegErg))
    BEGIN
     if (DecodeReg(Left,&RegErg2))
      BEGIN
       Result=True;
       DAsmCode[0] = 0x200000 + (RegErg << 8) + (RegErg2 << 13);
       CodeLen=1;
      END
     else
      BEGIN
       DecodeAdr(Left,MModAll,MSegXData+MSegYData);
       IsY=Ord(AdrSeg==SegYData);
       MixErg = ((RegErg & 0x18) << 17) + (IsY << 19) + ((RegErg & 7) << 16);
#ifdef __STDC__
       if ((AdrType==ModImm) AND ((AdrVal & 0xffffff00u)==0))
#else
       if ((AdrType==ModImm) AND ((AdrVal & 0xffffff00)==0))
#endif
	BEGIN
	 Result=True;
	 DAsmCode[0] = 0x200000 + (RegErg << 16) + ((AdrVal & 0xff) << 8);
	 CodeLen=1;
	END
       else if ((AdrType==ModAbs) AND (AdrVal<=63) AND (AdrVal>=0))
	BEGIN
	 Result=True;
	 DAsmCode[0] = 0x408000 + MixErg + (AdrVal << 8);
	 CodeLen=1;
	END
       else if (AdrType!=ModNone)
	BEGIN
	 Result=True;
	 DAsmCode[0] = 0x40c000 + MixErg + (AdrMode << 8);
         DAsmCode[1] = AdrVal;
	 CodeLen=1+AdrCnt;
	END
      END
     return Result;
    END

   /* 3. Quelle ist Register */

   if (DecodeReg(Left,&RegErg))
    BEGIN
     DecodeAdr(Right,MModNoImm,MSegXData+MSegYData);
     IsY=Ord(AdrSeg==SegYData);
     MixErg=((RegErg & 0x18) << 17) + (IsY << 19) + ((RegErg & 7) << 16);
     if ((AdrType==ModAbs) AND (AdrVal<=63) AND (AdrVal>=0))
      BEGIN
       Result=True;
       DAsmCode[0] = 0x400000 + MixErg + (AdrVal << 8);
       CodeLen=1;
      END
     else if (AdrType!=ModNone)
      BEGIN
       Result=True;
       DAsmCode[0] = 0x404000 + MixErg + (AdrMode << 8); 
       DAsmCode[1] = AdrVal;
       CodeLen=1+AdrCnt;
      END
     return Result;
    END

   /* 4. Ziel ist langes Register */

   if (DecodeLReg(Right,&RegErg))
    BEGIN
     DecodeAdr(Left,MModNoImm,MSegLData);
     MixErg=((RegErg & 4) << 17) + ((RegErg & 3) << 16);
     if ((AdrType==ModAbs) AND (AdrVal<=63) AND (AdrVal>=0))
      BEGIN
       Result=True;
       DAsmCode[0] = 0x408000 + MixErg + (AdrVal << 8);
       CodeLen=1;
      END
     else
      BEGIN
       Result=True;
       DAsmCode[0] = 0x40c000 + MixErg + (AdrMode << 8); 
       DAsmCode[1] = AdrVal;
       CodeLen=1+AdrCnt;
      END
     return Result;
    END

   /* 5. Quelle ist langes Register */

   if (DecodeLReg(Left,&RegErg))
    BEGIN
     DecodeAdr(Right,MModNoImm,MSegLData);
     MixErg=((RegErg & 4) << 17)+((RegErg & 3) << 16);
     if ((AdrType==ModAbs) AND (AdrVal<=63) AND (AdrVal>=0))
      BEGIN
       Result=True;
       DAsmCode[0] = 0x400000 + MixErg + (AdrVal << 8);
       CodeLen=1;
      END
     else
      BEGIN
       Result=True;
       DAsmCode[0] = 0x404000 + MixErg + (AdrMode << 8); 
       DAsmCode[1] = AdrVal;
       CodeLen=1+AdrCnt;
      END
     return Result;
    END

   WrError(1350); return Result;
END

	static Boolean DecodeMOVE_2(Integer Start)
BEGIN
   String Left1,Right1,Left2,Right2;
   LongInt RegErg,Reg1L,Reg1R,Reg2L,Reg2R;
   LongInt Mode1,Mode2,Dir1,Dir2,Type1,Type2,Cnt1,Cnt2,Val1,Val2;
   Boolean Result=False;

   SplitArg(ArgStr[Start],Left1,Right1);
   SplitArg(ArgStr[Start+1],Left2,Right2);

   /* 1. Spezialfall X auf rechter Seite ? */

   if (strcasecmp(Left2,"X0")==0)
    BEGIN
     if (NOT DecodeALUReg(Right2,&RegErg,False,False,True)) WrError(1350);
     else if (strcmp(Left1,Right2)!=0) WrError(1350);
     else
      BEGIN
       DecodeAdr(Right1,MModNoImm,MSegXData);
       if (AdrType!=ModNone)
	BEGIN
	 CodeLen=1+AdrCnt;
	 DAsmCode[0] = 0x080000 + (RegErg << 16) + (AdrMode << 8);
	 DAsmCode[1] = AdrVal;
	 Result=True;
	END
      END
     return Result;
    END

   /* 2. Spezialfall Y auf linker Seite ? */

   if (strcasecmp(Left1,"Y0")==0)
    BEGIN
     if (NOT DecodeALUReg(Right1,&RegErg,False,False,True)) WrError(1350);
     else if (strcmp(Left2,Right1)!=0) WrError(1350);
     else
      BEGIN
       DecodeAdr(Right2,MModNoImm,MSegYData);
       if (AdrType!=ModNone)
	BEGIN
	 CodeLen=1+AdrCnt;
	 DAsmCode[0] = 0x088000 + (RegErg << 16) + (AdrMode << 8);
	 DAsmCode[1] = AdrVal;
	 Result=True;
	END
      END
     return Result;
    END

   /* der Rest..... */

   if ((DecodeOpPair(Left1,Right1,SegXData,&Dir1,&Reg1L,&Reg1R,&Type1,&Mode1,&Cnt1,&Val1))
   AND (DecodeOpPair(Left2,Right2,SegYData,&Dir2,&Reg2L,&Reg2R,&Type2,&Mode2,&Cnt2,&Val2)))
    BEGIN
     if ((Reg1R==-1) AND (Reg2R==-1))
      BEGIN
       if ((Mode1 >> 3<1) OR (Mode1 >> 3>4) OR (Mode2 >> 3<1) OR (Mode2 >> 3>4)) WrError(1350);
       else if (((Mode1 ^ Mode2) & 4)==0) WrError(1760);
       else
	BEGIN
	 DAsmCode[0] = 0x800000 + (Dir2 << 22) + (Dir1 << 15) +
		       (Reg1L << 18) + (Reg2L << 16) + ((Mode1 & 0x1f) << 8)+
		       ((Mode2 & 3) << 13) + ((Mode2 & 24) << 17);
	 CodeLen=1; Result=True;
	END
      END
     else if (Reg1R==-1)
      BEGIN
       if ((Reg2L<2) OR (Reg2R>1)) WrError(1350);
       else
	BEGIN
	 DAsmCode[0] = 0x100000 + (Reg1L << 18) + ((Reg2L-2) << 17) + (Reg2R << 16) +
		       (Dir1 << 15) + (Mode1 << 8);
	 DAsmCode[1] = Val1; 
         CodeLen=1+Cnt1; Result=True;
	END
      END
     else if (Reg2R==-1)
      BEGIN
       if ((Reg1L<2) OR (Reg1R>1)) WrError(1350);
       else
	BEGIN
	 DAsmCode[0] = 0x104000 + (Reg2L << 16) + ((Reg1L-2) << 19) + (Reg1R << 18) +
		       (Dir2 << 15) + (Mode2 << 8);
	 DAsmCode[1] = Val2; 
         CodeLen=1+Cnt2; Result=True;
	END
      END
     else WrError(1350);
     return Result;
    END

   WrError(1350); return Result;
END

	static Boolean DecodeMOVE(Integer Start)
BEGIN
   switch (ArgCnt-Start+1)
    BEGIN
     case 0: return DecodeMOVE_0();
     case 1: return DecodeMOVE_1(Start);
     case 2: return DecodeMOVE_2(Start);
     default:
      WrError(1110); 
      return False;
    END
END

	static Boolean DecodeCondition(char *Asc, Word *Erg)
BEGIN
#define CondCount 18
   static char  *CondNames[CondCount]=
  	        {"CC","GE","NE","PL","NN","EC","LC","GT","CS","LT","EQ","MI",
	         "NR","ES","LS","LE","HS","LO"};
   Boolean Result;

   (*Erg)=0;
   while ((*Erg<CondCount) AND (strcasecmp(CondNames[*Erg],Asc)!=0)) (*Erg)++;
   if (*Erg==CondCount-1) *Erg=8;
   Result=(*Erg<CondCount);
   *Erg&=15;
   return Result;
END

	static Boolean DecodePseudo(void)
BEGIN
   Boolean OK;
   Integer BCount;
   Word AdrWord,z,z2;
/*   Byte Segment;*/
   TempResult t;
   LongInt HInt;


   if (Memo("XSFR")) 
    BEGIN
     CodeEquate(SegXData,0,0xffff);
     return True;
    END

   if (Memo("YSFR"))
    BEGIN
     CodeEquate(SegYData,0,0xffff);
     return True;
    END

/*  IF (Memo('XSFR')) OR (Memo('YSFR')) THEN
    BEGIN
     FirstPassUnknown:=False;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF (OK) AND (NOT FirstPassUnknown) THEN
	BEGIN
	 IF Memo('YSFR') THEN Segment:=SegYData ELSE Segment:=SegXData;
         PushLocHandle(-1);
	 EnterIntSymbol(LabPart,AdrWord,Segment,False);
         PopLocHandle;
	 IF MakeUseList THEN AddChunk(SegChunks[Segment],AdrWord,1,False);
	 ListLine:='='+'$'+HexString(AdrWord,4);
	END;
      END;
     Exit;
    END;*/

   if (Memo("DS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (FirstPassUnknown) WrError(1820);
       if ((OK) AND (NOT FirstPassUnknown))
	BEGIN
	 CodeLen=AdrWord; DontPrint=True;
         if (MakeUseList)
      	  if (AddChunk(SegChunks+ActPC,ProgCounter(),CodeLen,ActPC==SegCode)) WrError(90);
	END
      END
     return True;
    END

   if (Memo("DC"))
    BEGIN
     if (ArgCnt<1) WrError(1110);
     else
      BEGIN
       OK=True;
       for (z=1 ; z<=ArgCnt; z++)
	if (OK)
	 BEGIN
	  FirstPassUnknown=False;
	  EvalExpression(ArgStr[z],&t);
	  switch (t.Typ)
           BEGIN
	    case TempInt:
	     if (FirstPassUnknown) t.Contents.Int&=0xffffff;
	     if (NOT RangeCheck(t.Contents.Int,Int24))
	      BEGIN
	       WrError(1320); OK=False;
	      END
	     else
	      BEGIN
	       DAsmCode[CodeLen++]=t.Contents.Int & 0xffffff;
	      END
	     break;
	    case TempString:
	     BCount=2; DAsmCode[CodeLen]=0;
	     for (z2=0; z2<strlen(t.Contents.Ascii); z2++)
	      BEGIN
               HInt=t.Contents.Ascii[z2];
               HInt=CharTransTable[(Byte) HInt];
               HInt<<=(BCount*8);
               DAsmCode[CodeLen]|=HInt;
	       if (--BCount<0)
	        BEGIN
		 BCount=2; DAsmCode[++CodeLen]=0;
	        END
	      END
	     if (BCount!=2) CodeLen++;
	     break;
	    default:
	     WrError(1135); OK=False;
	   END
	 END
      END
     if (NOT OK) CodeLen=0;
     return True;
    END

   return False;
END

	static void MakeCode_56K(void)
BEGIN
   Integer z;
   LongInt AddVal,h=0,Reg1,Reg2,Reg3;
   LongInt HVal,HCnt,HMode,HSeg;
   Word Condition;
   Boolean OK;
   String Left,Mid,Right;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
	CodeLen=1; DAsmCode[0]=FixedOrders[z].Code;
       END;
      return;
     END

   /* ALU */

   for (z=0; z<ParOrderCnt; z++)
    if (Memo(ParOrders[z].Name))
     BEGIN
      if (DecodeMOVE(2))
       BEGIN
        OK=True;
        switch (ParOrders[z].Typ)
         BEGIN
          case ParAB:
           if (DecodeALUReg(ArgStr[1],&Reg1,False,False,True)) h=Reg1 << 3;
           else OK=False;
           break;
          case ParXYAB:
           SplitArg(ArgStr[1],Left,Right);
           if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) OK=False;
           else if (NOT DecodeLReg(Left,&Reg1)) OK=False;
           else if ((Reg1<2) OR (Reg1>3)) OK=False;
           else h=(Reg2 << 3) + ((Reg1-2) << 4);
           break;
          case ParABXYnAB:
           SplitArg(ArgStr[1],Left,Right);
           if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) OK=False;
           else if (NOT DecodeXYABReg(Left,&Reg1)) OK=False;
           else if ((Reg1 ^ Reg2)==1) OK=False;
           else
            BEGIN
             if (Reg1==0) Reg1=1; h=(Reg2 << 3) + (Reg1 << 4);
            END
           break;
          case ParABBA:
           if  (strcasecmp(ArgStr[1],"B,A")==0) h=0;
           else if (strcasecmp(ArgStr[1],"A,B")==0) h=8;
           else OK=False;
           break;
          case ParXYnAB:
           SplitArg(ArgStr[1],Left,Right);
           if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) OK=False;
           else if (NOT DecodeReg(Left,&Reg1)) OK=False;
           else if ((Reg1<4) OR (Reg1>7)) OK=False;
           else h=(Reg2 << 3) + (TurnXY(Reg1) << 4);
           break;
          case ParMul:
           SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right); h=0;
           if (*Left=='-')
            BEGIN
             strcpy(Left,Left+1); h+=4;
            END
           else if (*Left=='+') strcpy(Left,Left+1);
           if (NOT DecodeALUReg(Right,&Reg3,False,False,True)) OK=False;
           else if (NOT DecodeReg(Left,&Reg1)) OK=False;
           else if ((Reg1<4) OR (Reg1>7)) OK=False;
           else if (NOT DecodeReg(Mid,&Reg2)) OK=False;
           else if ((Reg2<4) OR (Reg2>7)) OK=False;
           else if (MacTable[Reg1-4][Reg2-4]==0xff) OK=False;
           else h+=(Reg3 << 3) + (MacTable[Reg1-4][Reg2-4] << 4);
           break;
         END
        if (OK) DAsmCode[0]+=ParOrders[z].Code+h;
        else
         BEGIN
          WrError(1350); CodeLen=0;
         END
       END
      return;
     END

   if (Memo("DIV"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) WrError(1350);
       else if (NOT DecodeReg(Left,&Reg1)) WrError(1350);
       else if ((Reg1<4) OR (Reg1>7)) WrError(1350);
       else
	BEGIN
	 CodeLen=1; 
         DAsmCode[0] = 0x018040 + (Reg2 << 3) + (TurnXY(Reg1) << 4);
	END
      END
     return;
    END

   if ((Memo("ANDI")) OR (Memo("ORI")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeControlReg(Right,&Reg1)) WrError(1350);
       else if (*Left!='#') WrError(1120);
       else
	BEGIN
	 h=EvalIntExpression(Left+1,Int8,&OK);
	 if (OK)
	  BEGIN
	   CodeLen=1;
	   DAsmCode[0] = 0x0000b8 + ((h & 0xff) << 8) + (Ord(Memo("ORI")) << 6) + Reg1;
	  END
	END
      END
     return;
    END

   if (Memo("NORM"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) WrError(1350);
       else if (NOT DecodeReg(Left,&Reg1)) WrError(1350);
       else if ((Reg1<16) OR (Reg1>23)) WrError(1350);
       else
	BEGIN
	 CodeLen=1; 
         DAsmCode[0] = 0x01d815 + ((Reg1 & 7) << 8) + (Reg2 << 3);
	END
      END
     return;
    END

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z]))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        Reg2=((z & 1) << 5) + (((LongInt) z >> 1) << 16);
        SplitArg(ArgStr[1],Left,Right);
        if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
        else if (*Left!='#') WrError(1120);
        else
         BEGIN
          h=EvalIntExpression(Left+1,Int8,&OK);
          if (FirstPassUnknown) h&=15;
          if (OK)
           if ((h<0) OR (h>23)) WrError(1320);
           else if (DecodeGeneralReg(Right,&Reg1))
            BEGIN
             CodeLen=1;
             DAsmCode[0] = 0x0ac040 + h +(Reg1 << 8) + Reg2;
            END
           else
            BEGIN
             DecodeAdr(Right,MModNoImm,MSegXData+MSegYData);
             Reg3=Ord(AdrSeg==SegYData) << 6;
             if ((AdrType==ModAbs) AND (AdrVal<=63) AND (AdrVal>=0))
              BEGIN
               CodeLen=1;
               DAsmCode[0] = 0x0a0000 + h + (AdrVal << 8) + Reg3 + Reg2;
              END
             else if ((AdrType==ModAbs) AND (AdrVal>=0xffc0) AND (AdrVal<=0xffff))
              BEGIN
               CodeLen=1;
               DAsmCode[0] = 0x0a8000 + h + ((AdrVal & 0x3f) << 8) + Reg3 + Reg2;
              END
             else if (AdrType!=ModNone)
              BEGIN
               CodeLen=1+AdrCnt;
               DAsmCode[0] = 0x0a4000 + h + (AdrMode << 8) + Reg3 + Reg2;
               DAsmCode[1] = AdrVal;
              END
            END
         END
       END
      return;
     END

   /* Datentransfer */

   if (Memo("MOVE"))
    BEGIN
     DecodeMOVE(1);
     return;
    END

   if (Memo("MOVEC"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else if (DecodeGeneralReg(Left,&Reg1))
	if (DecodeGeneralReg(Right,&Reg2))
	 if (Reg1>=0x20)                              /* S1,D2 */
	  BEGIN
	   CodeLen=1; DAsmCode[0] = 0x044080 + (Reg2 << 8) + Reg1;
	  END
	 else if (Reg2>=0x20)                         /* S2,D1 */
	  BEGIN
	   CodeLen=1; DAsmCode[0] = 0x04c080 + (Reg1 << 8) + Reg2;
	  END
	 else WrError(1350);
	else if (Reg1<0x20) WrError(1350);
	else                                          /* S1,ea/aa */
	 BEGIN
	  DecodeAdr(Right,MModNoImm,MSegXData+MSegYData);
	  if ((AdrType==ModAbs) AND (AdrVal>=0) AND (AdrVal<=63))
	   BEGIN
	    CodeLen=1;
	    DAsmCode[0] = 0x050000 + (AdrVal << 8) + (Ord(AdrSeg==SegYData) << 6) + Reg1;
	   END
	  else if (AdrType!=ModNone)
	   BEGIN
	    CodeLen=1+AdrCnt; DAsmCode[1]=AdrVal;
	    DAsmCode[0] = 0x054000 + (AdrMode << 8) + (Ord(AdrSeg==SegYData) << 6) + Reg1;
	   END
	 END
       else if (NOT DecodeGeneralReg(Right,&Reg2)) WrError(1350);
       else if (Reg2<0x20) WrError(1350);
       else    					      /* ea/aa,D1 */
	BEGIN
	 DecodeAdr(Left,MModAll,MSegXData+MSegYData);
	 if ((AdrType==ModImm) AND (AdrVal<=0xff) AND (AdrVal>=0))
	  BEGIN
	   CodeLen=1;
	   DAsmCode[0] = 0x050080 + (AdrVal << 8) + Reg2;
	  END
	 else if ((AdrType==ModAbs) AND (AdrVal>=0) AND (AdrVal<=63))
	  BEGIN
	   CodeLen=1;
	   DAsmCode[0] = 0x058000 + (AdrVal << 8) + (Ord(AdrSeg==SegYData) << 6) + Reg2;
	  END
	 else if (AdrType!=ModNone)
	  BEGIN
	   CodeLen=1+AdrCnt; DAsmCode[1]=AdrVal;
	   DAsmCode[0] = 0x05c000 + (AdrMode << 8) + (Ord(AdrSeg==SegYData) << 6) + Reg2;
	  END
	END
      END
     return;
    END

   if (Memo("MOVEM"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else if (DecodeGeneralReg(Left,&Reg1))
	BEGIN
	 DecodeAdr(Right,MModNoImm,MSegCode);
	 if ((AdrType==ModAbs) AND (AdrVal>=0) AND (AdrVal<=63))
	  BEGIN
	   CodeLen=1;
	   DAsmCode[0] = 0x070000 + Reg1 + (AdrVal << 8);
	  END
	 else if (AdrType!=ModNone)
	  BEGIN
	   CodeLen=1+AdrCnt; 
           DAsmCode[1] = AdrVal;
	   DAsmCode[0] = 0x074080 + Reg1 + (AdrMode << 8);
	  END
	END
       else if (NOT DecodeGeneralReg(Right,&Reg2)) WrError(1350);
       else
	BEGIN
	 DecodeAdr(Left,MModNoImm,MSegCode);
	 if ((AdrType==ModAbs) AND (AdrVal>=0) AND (AdrVal<=63))
	  BEGIN
	   CodeLen=1;
	   DAsmCode[0] = 0x078000 + Reg2 + (AdrVal << 8);
	  END
	 else if (AdrType!=ModNone)
	  BEGIN
	   CodeLen = 1+AdrCnt; 
           DAsmCode[1] = AdrVal;
	   DAsmCode[0] = 0x07c080 + Reg2 + (AdrMode << 8);
	  END
	END
      END
     return;
    END

   if (Memo("MOVEP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else if (DecodeGeneralReg(Left,&Reg1))
	BEGIN
	 DecodeAdr(Right,MModAbs,MSegXData+MSegYData);
	 if (AdrType!=ModNone)
	  if ((AdrVal>0xffff) OR (AdrVal<0xffc0)) WrError(1315);
	  else
	   BEGIN
	    CodeLen=1;
	    DAsmCode[0] = 0x08c000 + (Ord(AdrSeg==SegYData) << 16) +
			  (AdrVal & 0x3f) + (Reg1 << 8);
	   END
	END
       else if (DecodeGeneralReg(Right,&Reg2))
	BEGIN
	 DecodeAdr(Left,MModAbs,MSegXData+MSegYData);
	 if (AdrType!=ModNone)
	  if ((AdrVal>0xffff) OR (AdrVal<0xffc0)) WrError(1315);
	  else
	   BEGIN
	    CodeLen=1;
	    DAsmCode[0] = 0x084000 + (Ord(AdrSeg==SegYData) << 16) +
			  (AdrVal & 0x3f) + (Reg2 << 8);
	   END
	END
       else
	BEGIN
	 DecodeAdr(Left,MModAll,MSegXData+MSegYData+MSegCode);
	 if ((AdrType==ModAbs) AND (AdrSeg!=SegCode) AND (AdrVal>=0xffc0) AND (AdrVal<=0xffff))
	  BEGIN
	   HVal=AdrVal & 0x3f; HSeg=AdrSeg;
	   DecodeAdr(Right,MModNoImm,MSegXData+MSegYData+MSegCode);
	   if (AdrType!=ModNone)
	    if (AdrSeg==SegCode)
	     BEGIN
	      CodeLen=1+AdrCnt; 
              DAsmCode[1] = AdrVal;
	      DAsmCode[0] = 0x084040 + HVal + (AdrMode << 8) +
			    (Ord(HSeg==SegYData) << 16);
	     END
	    else
	     BEGIN
	      CodeLen=1+AdrCnt; 
              DAsmCode[1] = AdrVal;
	      DAsmCode[0] = 0x084080 + HVal + (AdrMode << 8) +
			    (Ord(HSeg==SegYData) << 16) +
			    (Ord(AdrSeg==SegYData) << 6);
	     END
	  END
	 else if (AdrType!=ModNone)
	  BEGIN
	   HVal=AdrVal; HCnt=AdrCnt; HMode=AdrMode; HSeg=AdrSeg;
	   DecodeAdr(Right,MModAbs,MSegXData+MSegYData);
	   if (AdrType!=ModNone)
	    if ((AdrVal<0xffc0) OR (AdrVal>0xffff)) WrError(1315);
	    else if (HSeg==SegCode)
	     BEGIN
	      CodeLen=1+HCnt; 
              DAsmCode[1] = HVal;
	      DAsmCode[0] = 0x08c040 + (AdrVal & 0x3f) + (HMode << 8) +
			    (Ord(AdrSeg==SegYData) << 16);
	     END
	    else
	     BEGIN
	      CodeLen=1+HCnt;
              DAsmCode[1] = HVal;
	      DAsmCode[0] = 0x08c080 + (((Word)AdrVal) & 0x3f) + (HMode << 8) +
			    (Ord(AdrSeg==SegYData) << 16) +
			    (Ord(HSeg==SegYData) << 6);
	     END
	  END
	END
      END
     return;
    END

   if (Memo("TFR"))
    BEGIN
     if (ArgCnt<1) WrError(1110);
     else if (DecodeMOVE(2))
      if (DecodeTFR(ArgStr[1],&Reg1))
       BEGIN
	DAsmCode[0] += 0x01 + (Reg1 << 3);
       END
      else
       BEGIN
	WrError(1350); CodeLen=0;
       END
     return;
    END

   if ((*OpPart=='T') AND (DecodeCondition(OpPart+1,&Condition)))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else if (NOT DecodeTFR(ArgStr[1],&Reg1)) WrError(1350);
     else if (ArgCnt==1)
      BEGIN
       CodeLen=1;
       DAsmCode[0] = 0x020000 + (Condition << 12) + (Reg1 << 3);
      END
     else
      BEGIN
       SplitArg(ArgStr[2],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeReg(Left,&Reg2)) WrError(1350);
       else if ((Reg2<16) OR (Reg2>23)) WrError(1350);
       else if (NOT DecodeReg(Right,&Reg3)) WrError(1350);
       else if ((Reg2<16) OR (Reg2>23)) WrError(1350);
       else
	BEGIN
	 Reg2-=16; Reg3-=16;
	 CodeLen=1;
	 DAsmCode[0] = 0x030000 + (Condition << 12) + (Reg2 << 8) + (Reg1 << 3) + Reg3;
	END
      END
     return;
    END

   if (Memo("LUA"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else
	BEGIN
	 DecodeAdr(Left,MModModInc+MModModDec+MModPostInc+MModPostDec,MSegXData);
	 if (AdrType!=ModNone)
	  if (NOT DecodeReg(Right,&Reg1)) WrError(1350);
	  else if ((Reg1<16) OR (Reg1>31)) WrError(1350);
	  else
	   BEGIN
	    CodeLen=1;
	    DAsmCode[0] = 0x044000 + (AdrMode << 8) + Reg1;
	   END
	END
      END
     return;
    END

   /* Spruenge */

   if ((Memo("JMP")) OR (Memo("JSR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AddVal=Ord(Memo("JSR")) << 16;
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       if (AdrType==ModAbs)
	if ((AdrVal & 0xf000)==0)
	 BEGIN
	  CodeLen=1; DAsmCode[0] = 0x0c0000 + AddVal + AdrVal;
	 END
	else
	 BEGIN
	  CodeLen=2; DAsmCode[0] = 0x0af080 + AddVal; 
          DAsmCode[1] = AdrVal;
	 END
       else if (AdrType!=ModNone)
	BEGIN
	 CodeLen=1; DAsmCode[0] = 0x0ac080 + AddVal + (AdrMode << 8);
	END
      END
     return;
    END

   if ((*OpPart=='J') AND (DecodeCondition(OpPart+1,&Condition)))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       if (AdrType==ModAbs)
	if ((AdrVal & 0xf000)==0)
	 BEGIN
	  CodeLen=1; 
          DAsmCode[0] = 0x0e0000 + (Condition << 12) + AdrVal;
	 END
	else
	 BEGIN
	  CodeLen=2; 
          DAsmCode[0] = 0x0af0a0 + Condition; 
          DAsmCode[1] = AdrVal;
	 END
       else if (AdrType!=ModNone)
	BEGIN
	 CodeLen=1; 
         DAsmCode[0] = 0x0ac0a0 + Condition + (AdrMode << 8);
	END
      END
     return;
    END

   if ((strncmp(OpPart,"JS",2)==0) AND (DecodeCondition(OpPart+2,&Condition)))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       if (AdrType==ModAbs)
	if ((AdrVal & 0xf000)==0)
	 BEGIN
	  CodeLen=1; 
          DAsmCode[0] = 0x0f0000 + (Condition << 12) + AdrVal;
	 END
	else
	 BEGIN
	  CodeLen=2; 
          DAsmCode[0] = 0x0bf0a0 + Condition; 
          DAsmCode[1] = AdrVal;
	 END
       else if (AdrType!=ModNone)
	BEGIN
	 CodeLen=1; 
         DAsmCode[0] = 0x0bc0a0 + Condition + (AdrMode << 8);
	END
      END
     return;
    END

   for (z=0; z<BitJmpOrderCnt; z++)
    if (Memo(BitJmpOrders[z]))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
	SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
	if ((*Left=='\0') OR (*Mid=='\0') OR (*Right=='\0')) WrError(1110);
	else if (*Left!='#') WrError(1120);
	else
	 BEGIN
	  DAsmCode[1]=EvalIntExpression(Right,Int16,&OK);
	  if (OK)
	   BEGIN
	    h=EvalIntExpression(Left+1,Int8,&OK);
	    if (FirstPassUnknown) h&=15;
	    if (OK)
	     if ((h<0) OR (h>23)) WrError(1320);
	     else
	      BEGIN
	       Reg2=((z & 1) << 5) + (((LongInt)(z >> 1)) << 16);
	       if (DecodeGeneralReg(Mid,&Reg1))
	        BEGIN
		 CodeLen=2;
		 DAsmCode[0] = 0x0ac080 + h + Reg2 + (Reg1 << 8);
	        END
	       else
	        BEGIN
		 DecodeAdr(Mid,MModNoImm,MSegXData+MSegYData);
		 Reg3=Ord(AdrSeg==SegYData) << 6;
		 if (AdrType==ModAbs)
		  if ((AdrVal>=0) AND (AdrVal<=63))
		   BEGIN
		    CodeLen=2;
		    DAsmCode[0] = 0x0a0080 + h + Reg2 + Reg3 + (AdrVal << 8);
		   END
		  else if ((AdrVal>=0xffc0) AND (AdrVal<=0xffff))
		   BEGIN
		    CodeLen=2;
		    DAsmCode[0] = 0x0a8080 + h + Reg2 + Reg3 + ((AdrVal & 0x3f) << 8);
		   END
		  else WrError(1320);
		 else
		  BEGIN
		   CodeLen=2;
		   DAsmCode[0] = 0x0a4080 + h + Reg2 + Reg3 + (AdrMode << 8);
		  END
	        END
	      END
	   END
	 END
       END
      return;
     END

   if (Memo("DO"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else
	BEGIN
	 DAsmCode[1] = EvalIntExpression(Right,Int16,&OK);
	 if (OK)
	  BEGIN
	   ChkSpace(SegCode);
	   if (DecodeGeneralReg(Left,&Reg1))
	    BEGIN
	     CodeLen=2; DAsmCode[0] = 0x06c000 + (Reg1 << 8);
	    END
	   else if (*Left=='#')
	    BEGIN
	     Reg1=EvalIntExpression(Left+1,Int12,&OK);
	     if (OK)
	      BEGIN
	       CodeLen=2;
	       DAsmCode[0] = 0x060080 + (Reg1 >> 8)+((Reg1 & 0xff) << 8);
	      END
	    END
	   else
	    BEGIN
	     DecodeAdr(Left,MModNoImm,MSegXData+MSegYData);
	     if (AdrType==ModAbs)
	      if ((AdrVal<0) OR (AdrVal>63)) WrError(1320);
	      else
	       BEGIN
		CodeLen=2;
		DAsmCode[0] = 0x060000 + (AdrVal << 8) + (Ord(AdrSeg==SegYData) << 6);
	       END
	     else
	      BEGIN
	       CodeLen=2;
	       DAsmCode[0] = 0x064000 + (AdrMode << 8) + (Ord(AdrSeg==SegYData) << 6);
	      END
	    END
	  END
	END
      END
     return;
    END

   if (Memo("REP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeGeneralReg(ArgStr[1],&Reg1))
      BEGIN
       CodeLen=1;
       DAsmCode[0] = 0x06c020 + (Reg1 << 8);
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModAll,MSegXData+MSegYData);
       if (AdrType==ModImm)
	if ((AdrVal<0) OR (AdrVal>0xfff)) WrError(1320);
	else
	 BEGIN
	  CodeLen=1;
	  DAsmCode[0] = 0x0600a0 + (AdrVal >> 8) + ((AdrVal & 0xff) << 8);
	 END
       else if ((AdrType==ModAbs) AND (AdrVal>=0) AND (AdrVal<=63))
	BEGIN
	 CodeLen=1;
	 DAsmCode[0] = 0x060020 + (AdrVal << 8) + (Ord(AdrSeg==SegYData) << 6);
	END
       else
	BEGIN
	 CodeLen=1+AdrCnt; 
         DAsmCode[1] = AdrVal;
	 DAsmCode[0] = 0x064020 + (AdrMode << 8) + (Ord(AdrSeg==SegYData) << 6);
	END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static Boolean ChkPC_56K(void)
BEGIN
   Boolean ok;

   switch (ActPC)
    BEGIN
     case SegCode:
     case SegXData:
     case SegYData:
      ok=(ProgCounter() <=0xffff);
      break;
     default:
      ok=False;
    END
   return (ok);
END


	static Boolean IsDef_56K(void)
BEGIN
   return ((Memo("XSFR")) OR (Memo("YSFR")));
END

        static void SwitchFrom_56K(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_56K(void)
BEGIN
   TurnWords=True; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x09; NOPCode=0x000000;
   DivideChars=" \009"; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegXData)|(1<<SegYData);
   Grans[SegCode ]=4; ListGrans[SegCode ]=4; SegInits[SegCode ]=0;
   Grans[SegXData]=4; ListGrans[SegXData]=4; SegInits[SegXData]=0;
   Grans[SegYData]=4; ListGrans[SegYData]=4; SegInits[SegYData]=0;

   MakeCode=MakeCode_56K; ChkPC=ChkPC_56K; IsDef=IsDef_56K;
   SwitchFrom=SwitchFrom_56K; InitFields();
END

	void code56k_init(void)
BEGIN
   CPU56000=AddCPU("56000",SwitchTo_56K);
END
