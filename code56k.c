/* code56k.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegeneratormodul fuer die DSP56K-Familie                             */
/*                                                                           */
/* Historie: 10. 6.1996 Grundsteinlegung                                     */
/*            7. 6.1998 563xx-Erweiterungen fertiggestellt                   */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 BookKeeping-Aufruf bei RES                           */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"
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
          CPUVar MinCPU;
         } FixedOrder;

typedef enum {ParAB,ParABShl1,ParABShl2,ParXYAB,ParABXYnAB,ParABBA,ParXYnAB,ParMul,ParFixAB} ParTyp;
typedef struct 
         {
          char *Name;
          ParTyp Typ;
          Byte Code;
         } ParOrder;

#define FixedOrderCnt 14

#define ParOrderCnt 31

#define BitOrderCnt 4
static char *BitOrders[BitOrderCnt]={"BCLR","BSET","BCHG","BTST"};

#define BitJmpOrderCnt 4
static char *BitJmpOrders[BitJmpOrderCnt]={"JCLR","JSET","JSCLR","JSSET"};

#define BitBrOrderCnt 4
static char *BitBrOrders[BitBrOrderCnt]={"BRCLR","BRSET","BSCLR","BSSET"};

#define ImmMacOrderCnt 4
static char *ImmMacOrders[ImmMacOrderCnt]={"MPYI","MPYRI","MACI","MACRI"};

static Byte MacTable[4][4]={{0,2,5,4},{2,0xff,6,7},{5,6,1,3},{4,7,3,0xff}};

static Byte Mac4Table[4][4]={{0,13,10,4},{5,1,14,11},{2,6,8,15},{12,3,7,9}};

static Byte Mac2Table[4]={1,3,2,0};

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
#define ModDisp 9
#define MModDisp (1 << ModDisp)

#define MModNoExt (MModIReg+MModPreDec+MModPostDec+MModPostInc+MModIndex+MModModDec+MModModInc)
#define MModNoImm (MModAbs+MModNoExt)
#define MModAll (MModNoImm+MModImm)

#define SegLData (SegYData+1)

#define MSegCode (1 << SegCode)
#define MSegXData (1 << SegXData)
#define MSegYData (1 << SegYData)
#define MSegLData (1 << SegLData)

static CPUVar CPU56000,CPU56002,CPU56300;
static IntType AdrInt;
static LargeWord MemLimit;
static ShortInt AdrType;
static LongInt AdrMode;
static LongInt AdrVal;
static Byte AdrSeg;

static FixedOrder *FixedOrders;
static ParOrder *ParOrders;

/*----------------------------------------------------------------------------------------------*/

	static void AddFixed(char *Name, LongWord Code, CPUVar NMin) 
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   
   FixedOrders[InstrZ].Name=Name;
   FixedOrders[InstrZ].Code=Code;
   FixedOrders[InstrZ++].MinCPU=NMin;
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
   AddFixed("NOP"    , 0x000000, CPU56000);
   AddFixed("ENDDO"  , 0x00008c, CPU56000);
   AddFixed("ILLEGAL", 0x000005, CPU56000);
   AddFixed("RESET"  , 0x000084, CPU56000);
   AddFixed("RTI"    , 0x000004, CPU56000);
   AddFixed("RTS"    , 0x00000c, CPU56000);
   AddFixed("STOP"   , 0x000087, CPU56000);
   AddFixed("SWI"    , 0x000006, CPU56000);
   AddFixed("WAIT"   , 0x000086, CPU56000);
   AddFixed("DEBUG"  , 0x000200, CPU56300);
   AddFixed("PFLUSH" , 0x000003, CPU56300);
   AddFixed("PFLUSHUN",0x000001, CPU56300);
   AddFixed("PFREE"  , 0x000002, CPU56300);
   AddFixed("TRAP"   , 0x000006, CPU56300);

   ParOrders=(ParOrder *) malloc(sizeof(ParOrder)*ParOrderCnt); InstrZ=0;
   AddPar("ABS" ,ParAB,     0x26);
   AddPar("ASL" ,ParABShl1, 0x32);
   AddPar("ASR" ,ParABShl1, 0x22);
   AddPar("CLR" ,ParAB,     0x13);
   AddPar("LSL" ,ParABShl2, 0x33);
   AddPar("LSR" ,ParABShl2, 0x23);
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
   AddPar("MAX" ,ParFixAB,  0x1d);
   AddPar("MAXM",ParFixAB,  0x15);
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

        static void CutSize(char *Asc, Byte *ShortMode)
BEGIN
   switch (*Asc)
    BEGIN
     case '>':
      strcpy(Asc,Asc+1); *ShortMode=2;
      break;
     case '<':
      strcpy(Asc,Asc+1); *ShortMode=1;
      break;
     default:
      *ShortMode=0;
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

	static Boolean DecodeXYAB0Reg(char *Asc, LongInt *Erg)
BEGIN
#undef RegCount
#define RegCount 6
   static char *RegNames[RegCount]=
	    {"A0","B0","X0","Y0","X1","Y1"};
   Word z;

   for (z=0; z<RegCount; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      *Erg=z+2; return True;
     END

   return False;
END

	static Boolean DecodeXYAB1Reg(char *Asc, LongInt *Erg)
BEGIN
#undef RegCount
#define RegCount 6
   static char *RegNames[RegCount]=
	    {"A1","B1","X0","Y0","X1","Y1"};
   Word z;

   for (z=0; z<RegCount; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      *Erg=z+2; return True;
     END

   return False;
END

	static Boolean DecodePCReg(char *Asc, LongInt *Erg)
BEGIN
#undef RegCount
#define RegCount 8              /** vvvv ab 56300 ? */
   static char *RegNames[RegCount]={"SZ","SR","OMR","SP","SSH","SSL","LA","LC"};
   Word z;

   for (z=0; z<RegCount; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      (*Erg)=z; return True;
     END

   return False;
END

	static Boolean DecodeAddReg(char *Asc, LongInt *Erg)
BEGIN
   if ((strlen(Asc)==2) AND (toupper(*Asc)=='M') AND (Asc[1]>='0') AND (Asc[1]<='7'))
    BEGIN
     *Erg=Asc[1]-'0'; return True;
    END
   /* >=56300 ? */
   if (strcasecmp(Asc,"EP")==0)
    BEGIN
     *Erg=0x0a; return True;
    END
   if (strcasecmp(Asc,"VBA")==0)
    BEGIN
     *Erg=0x10; return True;
    END
   if (strcasecmp(Asc,"SC")==0)
    BEGIN
     *Erg=0x11; return True;
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
   if (DecodeAddReg(Asc,Erg))
    BEGIN
     (*Erg)+=0x20; return True;
    END
   return False;
END

        static Boolean DecodeCtrlReg(char *Asc, LongInt *Erg)
BEGIN
   if (DecodeAddReg(Asc,Erg)) return True;
   if (DecodePCReg(Asc,Erg))
    BEGIN
     (*Erg)+=0x18; return True;
    END
   return False;
END

	static Boolean DecodeControlReg(char *Asc, LongInt *Erg)
BEGIN
   Boolean Result=True;

   if (strcasecmp(Asc,"MR")==0) *Erg=0;
   else if (strcasecmp(Asc,"CCR")==0) *Erg=1;
   else if ((strcasecmp(Asc,"OMR")==0) OR (strcasecmp(Asc,"COM")==0)) *Erg=2;
   else if ((strcasecmp(Asc,"EOM")==0) AND (MomCPU>=CPU56000)) *Erg=3;
   else Result=False;

   return Result;
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
   int z,l;
   Boolean OK;
   Byte OrdVal;
   String Asc;
   char *pp,*np,save;

   AdrType=ModNone; AdrCnt=0; strmaxcpy(Asc,Asc_O,255);

   /* Adressierungsmodi vom 56300 abschneiden */
   
   if ((MomCPU<CPU56300) AND ((Erl & MModDisp)!=0)) Erl-=MModDisp;
      
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

   /* Register mit Displacement bei 56300 */

   if (IsIndirect(Asc))
    BEGIN
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
     pp=strchr(Asc,'+'); np=strchr(Asc,'-');
     if ((pp==Nil) OR ((np!=Nil) AND (np<pp))) pp=np;
     if (pp!=0)
      BEGIN
       save=*pp; *pp='\0';
       if ((DecodeGeneralReg(Asc,&AdrMode)) AND (AdrMode>=16) AND (AdrMode<=23))
        BEGIN
         *pp=save;
         AdrMode-=16; FirstPassUnknown=False;
         AdrVal=EvalIntExpression(pp,Int24,&OK);
         if (OK)
          BEGIN
           if (FirstPassUnknown) AdrVal&=63;
           AdrType=ModDisp;
          END
         ChkMask(Erl,ErlSeg); return;
        END 
       *pp=save;
      END
    END

   /* dann absolut */

   AdrVal=EvalIntExpression(Asc,AdrInt,&OK);
   if (OK)
    BEGIN
     AdrType=ModAbs; AdrMode=0x30; AdrCnt=1;
     if ((AdrSeg & ((1<<SegCode)|(1<<SegXData)|(1<<SegYData)))!=0) ChkSpace(AdrSeg);
     ChkMask(Erl,ErlSeg); return;
    END
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

        static Boolean DecodeRR(char *Asc, LongInt *Erg)
BEGIN
   LongInt Part1,Part2;
   String Left,Right;

   SplitArg(Asc,Left,Right);
   if (NOT DecodeGeneralReg(Right,&Part2)) return False;
   if ((Part2<16) OR (Part2>23)) return False;
   if (NOT DecodeGeneralReg(Left,&Part1)) return False;
   if ((Part1<16) OR (Part1>23)) return False;
   *Erg=(Part2 & 7)+((Part1 & 7) << 8);
   return True;
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

	static Boolean DecodeMOVE_0(void)
BEGIN
   DAsmCode[0]=0x200000; CodeLen=1;
   return True;
END

	static Boolean DecodeMOVE_1(int Start)
BEGIN
   String Left,Right;
   LongInt RegErg,RegErg2,IsY,MixErg,l;
   char c;
   Word Condition;
   Boolean Result=False;
   Byte SegMask;

   if (strncasecmp(ArgStr[Start],"IF",2)==0)
    BEGIN
     l=strlen(ArgStr[Start]);
     if (strcasecmp(ArgStr[Start]+l-2,".U")==0)
      BEGIN
       RegErg=0x1000; l-=2;
      END
     else RegErg=0;
     c=ArgStr[Start][l]; ArgStr[Start][l]='\0';
     if (DecodeCondition(ArgStr[Start]+2,&Condition))
      if (MomCPU<CPU56300) WrError(1505);
      else
       BEGIN
        DAsmCode[0]=0x202000+(Condition << 8)+RegErg;
        CodeLen=1;
        return True;
       END
     ArgStr[Start][l]=c;
    END

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
     AdrSeg=SegNone;
     if (DecodeReg(Left,&RegErg2))
      BEGIN
       Result=True;
       DAsmCode[0] = 0x200000 + (RegErg << 8) + (RegErg2 << 13);
       CodeLen=1;
      END
     else
      BEGIN
       /* A und B gehen auch als L:..., in L-Zweig zwingen! */
       SegMask=MSegXData+MSegYData;
       if ((RegErg==14) OR (RegErg==15)) SegMask|=MSegLData;
       DecodeAdr(Left,MModAll+MModDisp,SegMask);
       if (AdrSeg!=SegLData)
        BEGIN
         IsY=Ord(AdrSeg==SegYData);
         MixErg = ((RegErg & 0x18) << 17) + (IsY << 19) + ((RegErg & 7) << 16);
         if (AdrType==ModDisp)
          if ((AdrVal<63) AND (AdrVal>-64) AND (RegErg<=15))
           BEGIN
            DAsmCode[0]=0x020090+((AdrVal & 1) << 6)+((AdrVal & 0x7e) << 10)
                        +(AdrMode << 8)+(IsY << 5)+RegErg;
            CodeLen=1;
           END
          else
           BEGIN
            DAsmCode[0]=0x0a70c0+(AdrMode << 8)+(IsY << 16)+RegErg;
            DAsmCode[1]=AdrVal;
            CodeLen=2;
           END
#ifdef __STDC__
         else if ((AdrType==ModImm) AND ((AdrVal & 0xffffff00u)==0))
#else
         else if ((AdrType==ModImm) AND ((AdrVal & 0xffffff00)==0))
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
      END
     if (AdrSeg!=SegLData) return Result;
    END

   /* 3. Quelle ist Register */

   if (DecodeReg(Left,&RegErg))
    BEGIN
     /* A und B gehen auch als L:..., in L-Zweig zwingen! */
     SegMask=MSegXData+MSegYData;
     if ((RegErg==14) OR (RegErg==15)) SegMask|=MSegLData;
     DecodeAdr(Right,MModNoImm+MModDisp,SegMask);
     if (AdrSeg!=SegLData)
      BEGIN
       IsY=Ord(AdrSeg==SegYData);
       MixErg=((RegErg & 0x18) << 17) + (IsY << 19) + ((RegErg & 7) << 16);
       if (AdrType==ModDisp)
        if ((AdrVal<63) AND (AdrVal>-64) AND (RegErg<=15))
         BEGIN
          DAsmCode[0]=0x020080+((AdrVal & 1) << 6)+((AdrVal & 0x7e) << 10)
                      +(AdrMode << 8)+(IsY << 5)+RegErg;
          CodeLen=1;
         END
        else
         BEGIN
          DAsmCode[0]=0x0a7080+(AdrMode << 8)+(IsY << 16)+RegErg;
          DAsmCode[1]=AdrVal;
          CodeLen=2;
         END
       else if ((AdrType==ModAbs) AND (AdrVal<=63) AND (AdrVal>=0))
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

	static Boolean DecodeMOVE_2(int Start)
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

	static Boolean DecodeMOVE(int Start)
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

        static Boolean DecodeMix(char *Asc, Word *Erg)
BEGIN
   if (strcmp(Asc,"SS")==0) *Erg=0;
   else if (strcmp(Asc,"SU")==0) *Erg=0x100;
   else if (strcmp(Asc,"UU")==0) *Erg=0x140;
   else return False;
   return True;
END


	static Boolean DecodePseudo(void)
BEGIN
   Boolean OK;
   int BCount;
   Word AdrWord,z,z2;
/*   Byte Segment;*/
   TempResult t;
   LongInt HInt;


   if (Memo("XSFR")) 
    BEGIN
     CodeEquate(SegXData,0,MemLimit);
     return True;
    END

   if (Memo("YSFR"))
    BEGIN
     CodeEquate(SegYData,0,MemLimit);
     return True;
    END

   if (Memo("DS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrWord=EvalIntExpression(ArgStr[1],AdrInt,&OK);
       if (FirstPassUnknown) WrError(1820);
       if ((OK) AND (NOT FirstPassUnknown))
	BEGIN
	 CodeLen=AdrWord; DontPrint=True;
         BookKeeping();
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
               HInt=CharTransTable[((usint) HInt)&0xff];
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

static int ErrCode;
static char *ErrString;

	static void SetError(int Code)
BEGIN
   ErrCode=Code; ErrString="";
END

	static void SetXError(int Code, char *Ext)
BEGIN
   ErrCode=Code; ErrString=Ext;
END

	static void PrError(void)
BEGIN
   if (*ErrString!='\0') WrXError(ErrCode,ErrString);
   else if (ErrCode!=0) WrError(ErrCode);
END

	static void MakeCode_56K(void)
BEGIN
   int z;
   LongInt AddVal,h=0,h2,Reg1,Reg2,Reg3;
   LongInt HVal,HCnt,HMode,HSeg,Dist;
   LargeInt LAddVal;
   Word Condition;
   Boolean OK,DontAdd;
   String Left,Mid,Right;
   Byte Size;

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
      else if (MomCPU<FixedOrders[z].MinCPU) WrError(1500);
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
        ErrCode=0; ErrString=""; DontAdd=False;
        switch (ParOrders[z].Typ)
         BEGIN
          case ParAB:
           if (NOT DecodeALUReg(ArgStr[1],&Reg1,False,False,True)) SetXError(1445,ArgStr[1]);
           else h=Reg1 << 3;
           break;
          case ParFixAB:
           if (strcasecmp(ArgStr[1],"A,B")!=0) SetError(1760);
           else h=0;
           break;
          case ParABShl1:
           if (strchr(ArgStr[1],',')==Nil)
            BEGIN
             if (NOT DecodeALUReg(ArgStr[1],&Reg1,False,False,True)) SetXError(1445,ArgStr[1]);
             else h=Reg1 << 3;
            END
           else if (ArgCnt!=1) SetError(1950);
           else if (MomCPU<CPU56300) SetError(1500);
           else
            BEGIN
             SplitArg(ArgStr[1],Left,Right);
             if (strchr(Right,',')==Nil) strcpy(Mid,Right);
             else SplitArg(Right,Mid,Right);
             if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) SetXError(1445,Right);
             else if (NOT DecodeALUReg(Mid,&Reg2,False,False,True)) SetXError(1445,Mid);
             else if (*Left=='#')
              BEGIN
               AddVal=EvalIntExpression(Left+1,UInt6,&OK);
               if (OK)
                BEGIN
                 DAsmCode[0]=0x0c1c00+((ParOrders[z].Code & 0x10) << 4)+(Reg2 << 7)+
                             (AddVal << 1)+Reg1;
                 CodeLen=1; DontAdd=True;
                END
              END
             else if (NOT DecodeXYAB1Reg(Left,&Reg3)) SetXError(1445,Left);
             else
              BEGIN
               DAsmCode[0]=0x0c1e60-((ParOrders[z].Code & 0x10) << 2)+(Reg2 << 4)+
                           (Reg3 << 1)+Reg1;
               CodeLen=1; DontAdd=True;
              END
            END
           break;
          case ParABShl2:
           if (strchr(ArgStr[1],',')==Nil)
            BEGIN
             if (NOT DecodeALUReg(ArgStr[1],&Reg1,False,False,True)) SetXError(1445,ArgStr[1]);
             else h=Reg1 << 3;
            END
           else if (ArgCnt!=1) SetError(1950);
           else if (MomCPU<CPU56300) SetError(1500);
           else
            BEGIN
             SplitArg(ArgStr[1],Left,Right);
             if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) SetXError(1445,Right);
             else if (*Left=='#')
              BEGIN
               AddVal=EvalIntExpression(Left+1,UInt5,&OK);
               if (OK)
                BEGIN
                 DAsmCode[0]=0x0c1e80+((0x33-ParOrders[z].Code) << 2)+
                             (AddVal << 1)+Reg1;
                 CodeLen=1; DontAdd=True;
                END
              END
             else if (NOT DecodeXYAB1Reg(Left,&Reg3)) SetXError(1445,Left);
             else
              BEGIN
               DAsmCode[0]=0x0c1e10+((0x33-ParOrders[z].Code) << 1)+
                           (Reg3 << 1)+Reg1;
               CodeLen=1; DontAdd=True;
              END
            END
           break;
          case ParXYAB:
           SplitArg(ArgStr[1],Left,Right);
           if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) SetXError(1445,Right);
           else if (NOT DecodeLReg(Left,&Reg1)) SetXError(1445,Left);
           else if ((Reg1<2) OR (Reg1>3)) SetXError(1445,Left);
           else h=(Reg2 << 3) + ((Reg1-2) << 4);
           break;
          case ParABXYnAB:
           SplitArg(ArgStr[1],Left,Right);
           if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) SetXError(1445,Right);
           else if (*Left=='#')
            BEGIN
             if (Memo("CMPM")) SetError(1350);
             else if (MomCPU<CPU56300) SetError(1500);
             else if (ArgCnt!=1) SetError(1950);
             else
              BEGIN
               AddVal=EvalIntExpression(Left+1,Int24,&OK);
               if (NOT OK) SetError(-1);
               else if ((AddVal>=0) AND (AddVal<=63))
                BEGIN
                 DAsmCode[0]=0x014000+(AddVal << 8); h=0x80+(Reg2 << 3);
                END
               else
                BEGIN
                 DAsmCode[0]=0x014000; h=0xc0+(Reg2 << 3);
                 DAsmCode[1]=AddVal & 0xffffff; CodeLen=2;
                END
              END
            END
           else
            if (NOT DecodeXYABReg(Left,&Reg1)) SetXError(1445,Left);
            else if ((Reg1 ^ Reg2)==1) SetError(1760);
            else if ((Memo("CMPM")) AND ((Reg1 &6)==2)) SetXError(1445,Left);
            else
             BEGIN
              if (Reg1<2) Reg1=Ord(NOT Memo("CMPM"));
              h=(Reg2 << 3) + (Reg1 << 4);
             END
           break;
          case ParABBA:
           if  (strcasecmp(ArgStr[1],"B,A")==0) h=0;
           else if (strcasecmp(ArgStr[1],"A,B")==0) h=8;
           else SetXError(1760,ArgStr[1]);
           break;
          case ParXYnAB:
           SplitArg(ArgStr[1],Left,Right);
           if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) SetXError(1445,Right);
           else if (*Left=='#')
            BEGIN
             if (MomCPU<CPU56300) SetError(1500);
             else if (ArgCnt!=1) SetError(1950);
             else
              BEGIN
               AddVal=EvalIntExpression(Left+1,Int24,&OK);
               if (NOT OK) SetError(-1);
               else if ((AddVal>=0) AND (AddVal<=63))
                BEGIN
                 DAsmCode[0]=0x014080+(AddVal << 8)+(Reg2 << 3)+(ParOrders[z].Code&7);
                 CodeLen=1;
                 DontAdd=True;
                END
               else
                BEGIN
                 DAsmCode[0]=0x0140c0+(Reg2 << 3)+(ParOrders[z].Code&7);
                 DAsmCode[1]=AddVal & 0xffffff; CodeLen=2;
                 DontAdd=True;
                END
              END
            END
           else
            if (NOT DecodeReg(Left,&Reg1)) SetXError(1445,Left);
            else if ((Reg1<4) OR (Reg1>7)) SetXError(1445,Left);
            else h=(Reg2 << 3) + (TurnXY(Reg1) << 4);
           break;
          case ParMul:
           SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right); h=0;
           if (*Left=='-')
            BEGIN
             strcpy(Left,Left+1); h+=4;
            END
           else if (*Left=='+') strcpy(Left,Left+1);
           if (NOT DecodeALUReg(Right,&Reg3,False,False,True)) SetXError(1445,Right);
           else if (NOT DecodeReg(Left,&Reg1)) SetXError(1445,Left);
           else if ((Reg1<4) OR (Reg1>7)) SetXError(1445,Left);
           else if (*Mid=='#')
            BEGIN
             if (ArgCnt!=1) WrError(1110);
             else if (MomCPU<CPU56300) WrError(1500);
             else
              BEGIN
               FirstPassUnknown=False;
               AddVal=EvalIntExpression(Mid+1,UInt24,&OK);
               if (FirstPassUnknown) AddVal=1;
               if ((NOT (SingleBit(AddVal,&LAddVal))) OR (LAddVal>22)) WrError(1540);
               else
                BEGIN
                 LAddVal=23-LAddVal;
                 DAsmCode[0]=0x010040+(LAddVal << 8)+(Mac2Table[Reg1 & 3] << 4)
                             +(Reg3 << 3);
                 CodeLen=1;
                END
              END
            END

           else if (NOT DecodeReg(Mid,&Reg2)) SetXError(1445,Mid);
           else if ((Reg2<4) OR (Reg2>7)) SetXError(1445,Mid);
           else if (MacTable[Reg1-4][Reg2-4]==0xff) SetError(1760);
           else h+=(Reg3 << 3) + (MacTable[Reg1-4][Reg2-4] << 4);
           break;
         END
        if (ErrCode==0)
         BEGIN
          if (NOT DontAdd) DAsmCode[0]+=ParOrders[z].Code+h;
         END
        else
         BEGIN
          if (ErrCode>0) PrError(); CodeLen=0;
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
    
   for (z=0; z<ImmMacOrderCnt; z++)
    if (Memo(ImmMacOrders[z]))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<CPU56300) WrError(1500);
      else
       BEGIN
        SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
        h=0;
        switch (*Left)
         BEGIN
          case '-':
           h=4;
          case '+':
           strcpy(Left,Left+1);
         END
        if ((*Mid=='\0') OR (*Right=='\0')) WrError(1110);
        else if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) WrXError(1445,Right);
        else if (NOT DecodeXYABReg(Mid,&Reg2)) WrXError(1445,Mid);
        else if ((Reg2<4) OR (Reg2>7)) WrXError(1445,Mid);
        else if (*Left!='#') WrError(1120);
        else
         BEGIN
          DAsmCode[1]=EvalIntExpression(Left+1,Int24,&OK);
          if (OK)
           BEGIN
            DAsmCode[0]=0x0141c0+z+h+(Reg1 << 3)+((Reg2 & 3) << 4);
            CodeLen=2;
           END
         END
       END
      return;
     END

   if ((strncmp(OpPart,"DMAC",4)==0) AND (DecodeMix(OpPart+4,&Condition)))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
       if (*Left=='-')
        BEGIN
         strcpy(Left,Left+1); Condition+=16;
        END
       else if (*Left=='+') strcpy(Left,Left+1);
       if ((*Mid=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) WrXError(1445,Right);
       else if (NOT DecodeXYAB1Reg(Mid,&Reg2)) WrXError(1445,Mid);
       else if (Reg2<4) WrXError(1445,Mid);
       else if (NOT DecodeXYAB1Reg(Left,&Reg3)) WrXError(1445,Left);
       else if (Reg3<4) WrXError(1445,Left);
       else
        BEGIN
         DAsmCode[0]=0x012480+Condition+(Reg1 << 5)+Mac4Table[Reg3-4][Reg2-4];
         CodeLen=1;
        END
      END
     return;
    END

   if (((strncmp(OpPart,"MAC",3)==0) OR (strncmp(OpPart,"MPY",3)==0))
   AND (DecodeMix(OpPart+3,&Condition)))
    BEGIN
     if (Condition==0) WrXError(1200,OpPart);
     else if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       if (OpPart[1]=='A') Condition-=0x100;
       SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
       if (*Left=='-')
        BEGIN
         strcpy(Left,Left+1); Condition+=16;
        END
       else if (*Left=='+') strcpy(Left,Left+1);
       if ((*Mid=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) WrXError(1445,Right);
       else if (NOT DecodeXYAB1Reg(Mid,&Reg2)) WrXError(1445,Mid);
       else if (Reg2<4) WrXError(1445,Mid);
       else if (NOT DecodeXYAB1Reg(Left,&Reg3)) WrXError(1445,Left);
       else if (Reg3<4) WrXError(1445,Left);
       else
        BEGIN
         DAsmCode[0]=0x012680+Condition+(Reg1 << 5)+Mac4Table[Reg3-4][Reg2-4];
         CodeLen=1;
        END
      END
     return;
    END

   if ((Memo("INC")) OR (Memo("DEC")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56002) WrError(1500);
     else if (NOT DecodeALUReg(ArgStr[1],&Reg1,False,False,True)) WrXError(1445,ArgStr[1]);
     else
      BEGIN
       DAsmCode[0]=0x000008+(Ord(Memo("DEC")) << 1)+Reg1;
       CodeLen=1;
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

   if (Memo("NORMF"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if (*Right=='\0') WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) WrXError(1445,Right);
       else if (NOT DecodeXYAB1Reg(Left,&Reg1)) WrXError(1445,Left);
       else
	BEGIN
         CodeLen=1; DAsmCode[0]=0x0c1e20+Reg2+(Reg1 << 1);
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
             else if ((AdrType==ModAbs) AND (AdrVal>=MemLimit-0x3f) AND (AdrVal<=MemLimit))
              BEGIN
               CodeLen=1;
               DAsmCode[0] = 0x0a8000 + h + ((AdrVal & 0x3f) << 8) + Reg3 + Reg2;
              END
             else if ((AdrType==ModAbs) AND (MomCPU>=CPU56300) AND (AdrVal>=MemLimit-0x7f) AND (AdrVal<=MemLimit-0x40))
              BEGIN
               Reg2=((z & 1) << 5) + (((LongInt) z >> 1) << 14);
               CodeLen=1;
               DAsmCode[0] = 0x010000 + h + ((AdrVal & 0x3f) << 8) + Reg3 + Reg2;
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

   if ((Memo("EXTRACT")) OR (Memo("EXTRACTU")))
    BEGIN
     z=Ord(Memo("EXTRACTU")) << 7;
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
       if ((*Mid=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) WrXError(1445,Right);
       else if (NOT DecodeALUReg(Mid,&Reg2,False,False,True)) WrXError(1445,Mid);
       else if (*Left=='#')
        BEGIN
         DAsmCode[1]=EvalIntExpression(Left+1,Int24,&OK);
         if (OK)
          BEGIN
           DAsmCode[0]=0x0c1800+z+Reg1+(Reg2 << 4); CodeLen=2;
          END
        END
       else if (NOT DecodeXYAB1Reg(Left,&Reg3)) WrXError(1445,Left);
       else
        BEGIN
         DAsmCode[0]=0x0c1a00+z+Reg1+(Reg2 << 4)+(Reg3 << 1);
         CodeLen=1;
        END
      END
     return;
    END

   if (Memo("INSERT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
       if ((*Mid=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) WrXError(1445,Right);
       else if (NOT DecodeXYAB0Reg(Mid,&Reg2)) WrXError(1445,Mid);
       else if (*Left=='#')
        BEGIN
         DAsmCode[1]=EvalIntExpression(Left+1,Int24,&OK);
         if (OK)
          BEGIN
           DAsmCode[0]=0x0c1900+Reg1+(Reg2 << 4); CodeLen=2;
          END
        END
       else if (NOT DecodeXYAB1Reg(Left,&Reg3)) WrXError(1445,Left);
       else
        BEGIN
         DAsmCode[0]=0x0c1b00+Reg1+(Reg2 << 4)+(Reg3 << 1);
         CodeLen=1;
        END
      END
     return;
    END

   if (Memo("MERGE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if (*Right=='\0') WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) WrXError(1445,Right);
       else if (NOT DecodeXYAB1Reg(Left,&Reg2)) WrXError(1445,Left);
       else
        BEGIN
         DAsmCode[0]=0x0c1b80+Reg1+(Reg2 << 1);
         CodeLen=1;
        END
      END
     return;
    END

   if (Memo("CLB"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if (*Right=='\0') WrError(1110);
       else if (NOT DecodeALUReg(Left,&Reg1,False,False,True)) WrXError(1445,Left);
       else if (NOT DecodeALUReg(Right,&Reg2,False,False,True)) WrXError(1445,Right);
       else
        BEGIN
         DAsmCode[0]=0x0c1e00+Reg2+(Reg1 << 1);
         CodeLen=1;
        END
      END
     return;
    END

   if (Memo("CMPU"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if (*Right=='\0') WrError(1110);
       else if (NOT DecodeALUReg(Right,&Reg1,False,False,True)) WrXError(1445,Right);
       else if (NOT DecodeXYABReg(Left,&Reg2)) WrXError(1445,Left);
       else if ((Reg1 ^ Reg2)==1) WrError(1760);
       else if ((Reg2 & 6)==2) WrXError(1445,Left);
       else
        BEGIN
         if (Reg2<2) Reg2=0;
         DAsmCode[0]=0x0c1ff0+(Reg2 << 1)+Reg1;
         CodeLen=1;
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
       if (*Right=='\0') WrError(1110);
       else if (DecodeCtrlReg(Left,&Reg1))
        if (DecodeGeneralReg(Right,&Reg2))
         BEGIN
          DAsmCode[0]=0x0440a0+(Reg2 << 8)+Reg1; CodeLen=1;
         END
        else
         BEGIN
          DecodeAdr(Right,MModNoImm,MSegXData+MSegYData);
          Reg3=(Ord(AdrSeg==SegYData)) << 6;
          if ((AdrType==ModAbs) AND (AdrVal<=63))
           BEGIN
            DAsmCode[0]=0x050020+(AdrVal << 8)+Reg3+Reg1; CodeLen=1;
           END
          else
           BEGIN
            DAsmCode[0]=0x054020+(AdrMode << 8)+Reg3+Reg1;
            DAsmCode[1]=AdrVal; CodeLen=1+AdrCnt;
           END
         END
       else if (NOT DecodeCtrlReg(Right,&Reg1)) WrXError(1440,Right);
       else
        if (DecodeGeneralReg(Left,&Reg2))
         BEGIN
          DAsmCode[0]=0x04c0a0+(Reg2 << 8)+Reg1; CodeLen=1;
         END
        else
         BEGIN
          DecodeAdr(Left,MModAll,MSegXData+MSegYData);
          Reg3=(Ord(AdrSeg==SegYData)) << 6;
          if ((AdrType==ModAbs) AND (AdrVal<=63))
           BEGIN
            DAsmCode[0]=0x058020+(AdrVal << 8)+Reg3+Reg1; CodeLen=1;
           END
          else if ((AdrType==ModImm) AND (AdrVal<=255))
           BEGIN
            DAsmCode[0]=0x0500a0+(AdrVal << 8)+Reg1; CodeLen=1;
           END
          else
           BEGIN
            DAsmCode[0]=0x05c020+(AdrMode << 8)+Reg3+Reg1;
            DAsmCode[1]=AdrVal; CodeLen=1+AdrCnt;
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
       else if (NOT DecodeGeneralReg(Right,&Reg2)) WrXError(1445,Right);
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
	  if ((AdrVal<=MemLimit) AND (AdrVal>=MemLimit-0x3f))
	   BEGIN
	    CodeLen=1;
	    DAsmCode[0] = 0x08c000 + (Ord(AdrSeg==SegYData) << 16) +
			  (AdrVal & 0x3f) + (Reg1 << 8);
	   END
          else if ((MomCPU>=CPU56300) AND (AdrVal<=MemLimit-0x40) AND (AdrVal>=MemLimit-0x7f))
           BEGIN
            CodeLen=1;
            DAsmCode[0]=0x04c000+(Ord(AdrSeg==SegYData) << 5)+
                        (Ord(AdrSeg==SegXData) << 7)+
                        (AdrVal & 0x1f)+((AdrVal & 0x20) << 1)+(Reg1 << 8);
           END
          else WrError(1315);
	END
       else if (DecodeGeneralReg(Right,&Reg2))
	BEGIN
	 DecodeAdr(Left,MModAbs,MSegXData+MSegYData);
	 if (AdrType!=ModNone)
	  if ((AdrVal<=MemLimit) AND (AdrVal>=MemLimit-0x3f))
	   BEGIN
	    CodeLen=1;
	    DAsmCode[0] = 0x084000 + (Ord(AdrSeg==SegYData) << 16) +
			  (AdrVal & 0x3f) + (Reg2 << 8);
	   END
          else if ((MomCPU>=CPU56300) AND (AdrVal<=MemLimit-0x40) AND (AdrVal>=MemLimit-0x7f))
           BEGIN
            CodeLen=1;
            DAsmCode[0]=0x044000+(Ord(AdrSeg==SegYData) << 5)+
                        (Ord(AdrSeg==SegXData) << 7)+
                        (AdrVal & 0x1f)+((AdrVal & 0x20) << 1)+(Reg2 << 8);
           END
          else WrError(1315);
	END
       else
	BEGIN
	 DecodeAdr(Left,MModAll,MSegXData+MSegYData+MSegCode);
	 if ((AdrType==ModAbs) AND (AdrSeg!=SegCode) AND (AdrVal>=MemLimit-0x3f) AND (AdrVal<=MemLimit))
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
	 else if ((AdrType==ModAbs) AND (MomCPU>=CPU56300) AND (AdrSeg!=SegCode) AND (AdrVal>=MemLimit-0x7f) AND (AdrVal<=MemLimit-0x40))
	  BEGIN
	   HVal=AdrVal & 0x3f; HSeg=AdrSeg;
	   DecodeAdr(Right,MModNoImm,MSegXData+MSegYData+MSegCode);
	   if (AdrType!=ModNone)
	    if (AdrSeg==SegCode)
	     BEGIN
	      CodeLen=1+AdrCnt; 
              DAsmCode[1] = AdrVal;
	      DAsmCode[0] = 0x008000 + HVal + (AdrMode << 8) +
			    (Ord(HSeg==SegYData) << 6);
	     END
	    else
	     BEGIN
	      CodeLen=1+AdrCnt; 
              DAsmCode[1] = AdrVal;
	      DAsmCode[0] = 0x070000 + HVal + (AdrMode << 8) +
			    (Ord(HSeg==SegYData) << 7) +
			    (Ord(HSeg==SegXData) << 14) +
			    (Ord(AdrSeg==SegYData) << 6);
	     END
	  END
	 else if (AdrType!=ModNone)
	  BEGIN
	   HVal=AdrVal; HCnt=AdrCnt; HMode=AdrMode; HSeg=AdrSeg;
	   DecodeAdr(Right,MModAbs,MSegXData+MSegYData);
	   if (AdrType!=ModNone)
	    if ((AdrVal>=MemLimit-0x3f) AND (AdrVal<=MemLimit))
	     BEGIN
	      if (HSeg==SegCode)
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
	    else if ((MomCPU>=CPU56300) AND (AdrVal>=MemLimit-0x7f) AND (AdrVal<=MemLimit-0x40))
	     BEGIN
	      if (HSeg==SegCode)
	       BEGIN
  	        CodeLen=1+HCnt; 
                DAsmCode[1] = HVal;
	        DAsmCode[0] = 0x00c000 + (AdrVal & 0x3f) + (HMode << 8) +
	  		      (Ord(AdrSeg==SegYData) << 6);
 	       END
	      else
	       BEGIN
	        CodeLen=1+HCnt;
                DAsmCode[1] = HVal;
	        DAsmCode[0] = 0x078000 + (((Word)AdrVal) & 0x3f) + (HMode << 8) +
		  	      (Ord(AdrSeg==SegYData) << 7) +
		  	      (Ord(AdrSeg==SegXData) << 14) +
			      (Ord(HSeg==SegYData) << 6);
	       END
	     END
	    else WrError(1315);
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
     else if (DecodeTFR(ArgStr[1],&Reg1))
      BEGIN
       if (ArgCnt==1)
        BEGIN
         CodeLen=1;
         DAsmCode[0]=0x020000+(Condition << 12)+(Reg1 << 3);
        END
       else if (NOT DecodeRR(ArgStr[2],&Reg2)) WrError(1350);
       else
        BEGIN
         CodeLen=1;
         DAsmCode[0]=0x030000+(Condition << 12)+(Reg1 << 3)+Reg2;
        END
      END
     else if (ArgCnt!=1) WrError(1110);
     else if (NOT DecodeRR(ArgStr[1],&Reg1)) WrError(1350);
     else
      BEGIN
       DAsmCode[0]=0x020800+(Condition << 12)+Reg1;
       CodeLen=1;
      END
     return;
    END

   for (z=0; z<BitBrOrderCnt; z++)
    if (Memo(BitBrOrders[z]))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<CPU56300) WrError(1500);
      else
       BEGIN
        h=(z & 1) << 5;
        h2=(((LongInt) z) & 2) << 15;
        SplitArg(ArgStr[1],Left,Right); SplitArg(Right,Mid,Right);
        if ((*Left=='\0') OR (*Right=='\0') OR (*Mid=='\0')) WrError(1110);
        else if (*Left!='#') WrError(1120);
        else
         BEGIN
          AddVal=EvalIntExpression(Left+1,Int8,&OK);
          if (FirstPassUnknown) AddVal&=15;
          if (OK)
           if ((AddVal<0) OR (AddVal>23)) WrError(1320);
           else if (DecodeGeneralReg(Mid,&Reg1))
            BEGIN
             CodeLen=1;
             DAsmCode[0]=0x0cc080+AddVal+(Reg1 << 8)+h+h2;
            END
           else
            BEGIN
             FirstPassUnknown=False;
             DecodeAdr(Mid,MModNoImm,MSegXData+MSegYData);
             Reg3=Ord(AdrSeg==SegYData) << 6;
             if ((AdrType==ModAbs) AND (FirstPassUnknown)) AdrVal&=0x3f;
             if ((AdrType==ModAbs) AND (AdrVal<=63) AND (AdrVal>=0))
              BEGIN
               CodeLen=1;
               DAsmCode[0]=0x0c8080+AddVal+(AdrVal << 8)+Reg3+h+h2;
              END
             else if ((AdrType==ModAbs) AND (AdrVal>=MemLimit-0x3f) AND (AdrVal<=MemLimit))
              BEGIN
               CodeLen=1;
               DAsmCode[0]=0x0cc000+AddVal+((AdrVal & 0x3f) << 8)+Reg3+h+h2;
              END
             else if ((AdrType==ModAbs) AND (AdrVal>=MemLimit-0x7f) AND (AdrVal<=MemLimit-0x40))
              BEGIN
               CodeLen=1;
               DAsmCode[0]=0x048000+AddVal+((AdrVal & 0x3f) << 8)+Reg3+h+(h2 >> 9);
              END
             else if (AdrType==ModAbs) WrError(1350);
             else if (AdrType!=ModNone)
              BEGIN
               CodeLen=1;
               DAsmCode[0]=0x0c8000+AddVal+(AdrMode << 8)+Reg3+h+h2;
              END
            END
         END
        if (CodeLen==1)
         BEGIN
          Dist=EvalIntExpression(Right,AdrInt,&OK)-(EProgCounter()+2);
          if (OK)
           BEGIN
            DAsmCode[1]=Dist & 0xffffff; CodeLen=2;
           END
          else CodeLen=0;
         END
       END
      return;
     END

   if ((Memo("BRA")) OR (Memo("BSR")))
    BEGIN
     z=Memo("BRA") ?  0x40 : 0;
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else if (DecodeReg(ArgStr[1],&Reg1))
      BEGIN
       if ((Reg1<16) OR (Reg1>23)) WrXError(1445,ArgStr[1]);
       else
        BEGIN
         Reg1-=16;
         DAsmCode[0]=0x0d1880+(Reg1 << 8)+z;
         CodeLen=1;
        END
      END
     else
      BEGIN
       CutSize(ArgStr[1],&Size);
       Dist=EvalIntExpression(ArgStr[1],AdrInt,&OK)-(EProgCounter()+1);
       if (Size==0)
        Size=((Dist>-256) AND (Dist<255)) ? 1 : 2;
        switch (Size)
         BEGIN
          case 1:
           if ((NOT SymbolQuestionable) AND ((Dist<-256) OR (Dist>255))) WrError(1370);
           else
            BEGIN
             Dist&=0x1ff;
             DAsmCode[0]=0x050800+(z << 4)+((Dist & 0x1e0) << 1)+(Dist & 0x1f);
             CodeLen=1;
            END
           break;
          case 2:
           Dist--;
           DAsmCode[0]=0x0d1080+z;
           DAsmCode[1]=Dist & 0xffffff;
           CodeLen=2;
           break;
         END
      END
     return;
    END

   if ((*OpPart=='B') AND (DecodeCondition(OpPart+1,&Condition)))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else if (DecodeReg(ArgStr[1],&Reg1))
      BEGIN
       if ((Reg1<16) OR (Reg1>23)) WrXError(1445,ArgStr[1]);
       else
        BEGIN
         Reg1-=16;
         DAsmCode[0]=0x0d1840+(Reg1 << 8)+Condition;
         CodeLen=1;
        END
      END
     else
      BEGIN
       CutSize(ArgStr[1],&Size);
       Dist=EvalIntExpression(ArgStr[1],AdrInt,&OK)-(EProgCounter()+1);
       if (Size==0)
        Size=((Dist>-256) AND (Dist<255)) ? 1 : 2;
       switch (Size)
        BEGIN
         case 1:
          if ((NOT SymbolQuestionable) AND ((Dist<-256) OR (Dist>255))) WrError(1370);
          else
           BEGIN
            Dist&=0x1ff;
            DAsmCode[0]=0x050400+(Condition << 12)+((Dist & 0x1e0) << 1)+(Dist & 0x1f);
            CodeLen=1;
           END
          break;
         case 2:
          Dist--;
          DAsmCode[0]=0x0d1040+Condition;
          DAsmCode[1]=Dist & 0xffffff;
          CodeLen=2;
          break;
        END
      END
     return;
    END

   if ((strncmp(OpPart,"BS",2)==0) AND (DecodeCondition(OpPart+2,&Condition)))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else if (DecodeReg(ArgStr[1],&Reg1))
      BEGIN
       if ((Reg1<16) OR (Reg1>23)) WrXError(1445,ArgStr[1]);
       else
        BEGIN
         Reg1-=16;
         DAsmCode[0]=0x0d1800+(Reg1 << 8)+Condition;
         CodeLen=1;
        END
      END
     else
      BEGIN
       CutSize(ArgStr[1],&Size);
       Dist=EvalIntExpression(ArgStr[1],AdrInt,&OK)-(EProgCounter()+1);
       if (Size==0)
        Size=((Dist>-256) AND (Dist<255)) ? 1 : 2;
       switch (Size)
        BEGIN
         case 1:
          if ((NOT SymbolQuestionable) AND ((Dist<-256) OR (Dist>255))) WrError(1370);
          else
           BEGIN
            Dist&=0x1ff;
            DAsmCode[0]=0x050000+(Condition << 12)+((Dist & 0x1e0) << 1)+(Dist & 0x1f);
            CodeLen=1;
           END
          break;
         case 2:
          Dist--;
          DAsmCode[0]=0x0d1000+Condition;
          DAsmCode[1]=Dist & 0xffffff;
          CodeLen=2;
          break;
        END
      END
     return;
    END

   if ((Memo("LUA")) OR (Memo("LEA")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else if (NOT DecodeReg(Right,&Reg1)) WrXError(1445,Right);
       else if (Reg1>31) WrXError(1445,Right);
       else
	BEGIN
         DecodeAdr(Left,MModModInc+MModModDec+MModPostInc+MModPostDec+MModDisp,MSegXData);
         if (AdrType==ModDisp)
          BEGIN
           if (ChkRange(AdrVal,-64,63))
            BEGIN
             AdrVal&=0x7f;
             DAsmCode[0]=0x040000+(Reg1-16)+(AdrMode << 8)+
                         ((AdrVal & 0x0f) << 4)+
                         ((AdrVal & 0x70) << 7);
              CodeLen=1;
            END
          END
         else if (AdrType!=ModNone)
          BEGIN
           CodeLen=1;
           DAsmCode[0]=0x044000+(AdrMode << 8)+Reg1;
          END
        END
      END
     return;
    END

   if (Memo("LRA"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       if (*Right=='\0') WrError(1110);
       else if (NOT DecodeGeneralReg(Right,&Reg1)) WrXError(1445,Right);
       else if (Reg1>0x1f) WrXError(1445,Right);
       else if (DecodeGeneralReg(Left,&Reg2))
        BEGIN
         if ((Reg2<16) OR (Reg2>23)) WrXError(1445,Left);
         else
          BEGIN
           DAsmCode[0]=0x04c000+((Reg2 & 7) << 8)+Reg1;
           CodeLen=1;
          END
        END
       else
        BEGIN
         DAsmCode[1]=EvalIntExpression(Left,AdrInt,&OK)-(EProgCounter()+2);
         if (OK)
          BEGIN
           DAsmCode[0]=0x044040+Reg1;
           CodeLen=2;
          END
        END
      END
     return;
    END

   if (Memo("PLOCK"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       if (AdrType!=ModNone)
        BEGIN
         DAsmCode[0]=0x0ac081+(AdrMode << 8); DAsmCode[1]=AdrVal;
         CodeLen=2;
        END
      END
     return;
    END

   if ((Memo("PLOCKR")) OR (Memo("PUNLOCKR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       DAsmCode[1]=(EvalIntExpression(ArgStr[1],AdrInt,&OK)-(EProgCounter()+2)) & 0xffffff;
       if (OK)
        BEGIN              /* ANSI :-O */
         DAsmCode[0]=0x00000e + Ord(Memo("PUNLOCKR")); CodeLen=2;
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
	if ((AdrVal & 0xfff000)==0)
	 BEGIN
	  CodeLen=1; DAsmCode[0] = 0x0c0000 + AddVal + (AdrVal & 0xfff);
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
	if ((AdrVal & 0xfff000)==0)
	 BEGIN
	  CodeLen=1; 
          DAsmCode[0] = 0x0e0000 + (Condition << 12) + (AdrVal & 0xfff);
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
	if ((AdrVal & 0xfff000)==0)
	 BEGIN
	  CodeLen=1; 
          DAsmCode[0] = 0x0f0000 + (Condition << 12) + (AdrVal & 0xfff);
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
	  DAsmCode[1]=EvalIntExpression(Right,AdrInt,&OK);
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
		 DAsmCode[0] = 0x0ac000 + h + Reg2 + (Reg1 << 8);
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
		  else if ((AdrVal>=MemLimit-0x3f) AND (AdrVal<=MemLimit))
		   BEGIN
		    CodeLen=2;
		    DAsmCode[0] = 0x0a8080 + h + Reg2 + Reg3 + ((AdrVal & 0x3f) << 8);
		   END
		  else if ((MomCPU>=CPU56300) AND (AdrVal>=MemLimit-0x7f) AND (AdrVal<=MemLimit-0x40))
		   BEGIN
		    CodeLen=2;
  	            Reg2=((z & 1) << 5) + (((LongInt)(z >> 1)) << 14);
		    DAsmCode[0] = 0x018080 + h + Reg2 + Reg3 + ((AdrVal & 0x3f) << 8);
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

   if ((Memo("DO")) OR (Memo("DOR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       z=Ord(Memo("DOR"));
       SplitArg(ArgStr[1],Left,Right);
       if ((*Left=='\0') OR (*Right=='\0')) WrError(1110);
       else
	BEGIN
	 DAsmCode[1]=EvalIntExpression(Right,AdrInt,&OK)-1;
	 if (OK)
	  BEGIN
	   ChkSpace(SegCode);
	   if (strcasecmp(Left,"FOREVER")==0)
	    BEGIN
	     if (MomCPU<CPU56300) WrError(1500);
	     else
	      BEGIN
	       DAsmCode[0]=0x000203-z;
	       CodeLen=2;
	      END
	    END
	   else if (DecodeGeneralReg(Left,&Reg1))
	    BEGIN
	     if (Reg1==0x3c) WrXError(1445,Left); /* kein SSH!! */
	     else
	      BEGIN
  	       CodeLen=2;
	       DAsmCode[0]=0x06c000+(Reg1 << 8)+(z << 4);
	      END
	    END
	   else if (*Left=='#')
	    BEGIN
	     Reg1=EvalIntExpression(Left+1,UInt12,&OK);
	     if (OK)
	      BEGIN
	       CodeLen=2;
	       DAsmCode[0]=0x060080+(Reg1 >> 8)+((Reg1 & 0xff) << 8)+(z << 4);
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
		DAsmCode[0]=0x060000+(AdrVal << 8)+(Ord(AdrSeg==SegYData) << 6)+(z << 4);
	       END
	     else
	      BEGIN
	       CodeLen=2;
	       DAsmCode[0]=0x064000+(AdrMode << 8)+(Ord(AdrSeg==SegYData) << 6)+(z << 4);
	      END
	    END
	  END
	END
      END
     return;
    END

   if ((strncmp(OpPart,"BRK",3)==0) AND (DecodeCondition(OpPart+3,&Condition)))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       DAsmCode[0]=0x00000210+Condition;
       CodeLen=1;
      END
     return;
    END

   if ((strncmp(OpPart,"TRAP",4)==0) AND (DecodeCondition(OpPart+4,&Condition)))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       DAsmCode[0]=0x000010+Condition;
       CodeLen=1;
      END
     return;
    END

   if ((strncmp(OpPart,"DEBUG",5)==0) AND (DecodeCondition(OpPart+5,&Condition)))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (MomCPU<CPU56300) WrError(1500);
     else
      BEGIN
       DAsmCode[0]=0x00000300+Condition;
       CodeLen=1;
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
       else if (AdrType==ModAbs)
        if ((AdrVal<0) OR (AdrVal>63)) WrError(1320);
        else
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
   
   if (MomCPU==CPU56300)
    BEGIN
     AdrInt=UInt24; MemLimit=0xffffffl;
    END
   else
    BEGIN
     AdrInt=UInt16; MemLimit=0xffff;
    END

   ValidSegs=(1<<SegCode)|(1<<SegXData)|(1<<SegYData);
   Grans[SegCode ]=4; ListGrans[SegCode ]=4; SegInits[SegCode ]=0;
   SegLimits[SegCode ] = MemLimit;
   Grans[SegXData]=4; ListGrans[SegXData]=4; SegInits[SegXData]=0;
   SegLimits[SegXData] = MemLimit;
   Grans[SegYData]=4; ListGrans[SegYData]=4; SegInits[SegYData]=0;
   SegLimits[SegYData] = MemLimit;

   MakeCode=MakeCode_56K; IsDef=IsDef_56K;
   SwitchFrom=SwitchFrom_56K; InitFields();
END

	void code56k_init(void)
BEGIN
   CPU56000=AddCPU("56000",SwitchTo_56K);
   CPU56002=AddCPU("56002",SwitchTo_56K);
   CPU56300=AddCPU("56300",SwitchTo_56K);
END

