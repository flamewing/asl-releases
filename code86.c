/* code86.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 8086/V-Serie                                                */
/*                                                                           */
/* Historie:                                                                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "codepseudo.h"
#include "codevars.h"
#include "asmitree.h"

/*---------------------------------------------------------------------------*/

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          CPUVar MinCPU;
          Word Code;
          Byte Add;
         } AddOrder;


#define FixedOrderCnt 41
#define FPUFixedOrderCnt 29
#define StringOrderCnt 14
#define ReptOrderCnt 7
#define RelOrderCnt 36
#define ModRegOrderCnt 4
#define ShiftOrderCnt 8
#define Reg16OrderCnt 3
#define FPUStOrderCnt 2
#define FPU16OrderCnt 5
#define MulOrderCnt 4 
#define Bit1OrderCnt 4

#define SegRegCnt 3
static char *SegRegNames[SegRegCnt+1]={"ES","CS","SS","DS"};
static Byte SegRegPrefixes[SegRegCnt+1]={0x26,0x2e,0x36,0x3e};

#define TypeNone (-1)
#define TypeReg8 0
#define TypeReg16 1
#define TypeRegSeg 2
#define TypeMem 3
#define TypeImm 4
#define TypeFReg 5

static ShortInt AdrType;
static Byte AdrMode;
static Byte AdrVals[6];
static ShortInt OpSize;
static Boolean UnknownFlag;

static Boolean NoSegCheck;

static Byte Prefixes[6];
static Byte PrefixLen;

static Byte SegAssumes[SegRegCnt+1];

static SimpProc SaveInitProc;

static CPUVar CPU8086,CPU80186,CPUV30,CPUV35;

static FixedOrder *FixedOrders;
static FixedOrder *FPUFixedOrders;
static FixedOrder *FPUStOrders;
static FixedOrder *FPU16Orders;
static FixedOrder *StringOrders;
static FixedOrder *ReptOrders;
static FixedOrder *RelOrders;
static FixedOrder *ModRegOrders;
static FixedOrder *ShiftOrders;
static AddOrder *Reg16Orders;
static char **MulOrders;
static char **Bit1Orders;
static PInstTable InstTable;

/*------------------------------------------------------------------------------------*/

	static void PutCode(Word Code)
BEGIN
   if (Hi(Code)!=0) BAsmCode[CodeLen++]=Hi(Code);
   BAsmCode[CodeLen++]=Lo(Code);
END

	static void MoveAdr(int Dest)
BEGIN
   memcpy(BAsmCode+CodeLen+Dest,AdrVals,AdrCnt);
END

	static Byte Sgn(Byte inp)
BEGIN
   return (inp>127) ? 0xff : 0;
END

	static void AddPrefix(Byte Prefix)
BEGIN
   Prefixes[PrefixLen++]=Prefix;
END

	static void AddPrefixes(void)
BEGIN
   if ((CodeLen!=0) AND (PrefixLen!=0))
    BEGIN
     memmove(BAsmCode+PrefixLen,BAsmCode,CodeLen);
     memcpy(BAsmCode,Prefixes,PrefixLen);
     CodeLen+=PrefixLen;
    END
END

	static Boolean AbleToSign(Word Arg)
BEGIN
   return ((Arg<=0x7f) OR (Arg>=0xff80));
END

	static Boolean MinOneIs0(void)
BEGIN
   if ((UnknownFlag) AND (OpSize==-1))
    BEGIN
     OpSize=0; return True;
    END
   else return False;
END

	static void ChkOpSize(ShortInt NewSize)
BEGIN
   if (OpSize==-1) OpSize=NewSize;
   else if (OpSize!=NewSize)
    BEGIN
     AdrType=TypeNone; WrError(1131);
    END
END

	static void ChkSingleSpace(Byte Seg, Byte EffSeg, Byte MomSegment)
BEGIN
   Byte z;

   /* liegt Operand im zu pruefenden Segment? nein-->vergessen */

   if ((MomSegment & (1 << Seg))==0) return;

   /* zeigt bish. benutztes Segmentregister auf dieses Segment? ja-->ok */

   if (EffSeg==Seg) return;

   /* falls schon ein Override gesetzt wurde, nur warnen */

   if (PrefixLen>0) WrError(70);

   /* ansonsten ein passendes Segment suchen und warnen, falls keines da */

   else
    BEGIN
     z=0;
     while ((z<=SegRegCnt) AND (SegAssumes[z]!=Seg)) z++;
     if (z>SegRegCnt) WrXError(75,SegNames[Seg]);
     else AddPrefix(SegRegPrefixes[z]);
    END
END

	static void ChkSpaces(ShortInt SegBuffer, Byte MomSegment)
BEGIN
   Byte EffSeg;

   if (NoSegCheck) return;

   /* in welches Segment geht das benutzte Segmentregister ? */

   EffSeg=SegAssumes[SegBuffer];

   /* Zieloperand in Code-/Datensegment ? */

   ChkSingleSpace(SegCode,EffSeg,MomSegment);
   ChkSingleSpace(SegXData,EffSeg,MomSegment);
   ChkSingleSpace(SegData,EffSeg,MomSegment);
END

	static void DecodeAdr(char *Asc)
BEGIN
#define RegCnt 7
   static char *Reg16Names[RegCnt+1]=
	      {"AX","CX","DX","BX","SP","BP","SI","DI"};
   static char *Reg8Names[RegCnt+1]=
	      {"AL","CL","DL","BL","AH","CH","DH","BH"};
   static Byte RMCodes[8]={11,12,21,22,1,2,20,10};

   int RegZ,z;
   Boolean IsImm;
   ShortInt IndexBuf,BaseBuf;
   Byte SumBuf;
   LongInt DispAcc,DispSum;
   char *p,*p1,*p2;
   Boolean HasAdr;
   Boolean OK,OldNegFlag,NegFlag;
   String AdrPart,AddPart;
   ShortInt SegBuffer;
   Byte MomSegment;
   ShortInt FoundSize;

   AdrType=TypeNone; AdrCnt=0;
   SegBuffer=(-1); MomSegment=0;

   for (RegZ=0; RegZ<=RegCnt; RegZ++)
    BEGIN
     if (strcasecmp(Asc,Reg16Names[RegZ])==0)
      BEGIN
       AdrType=TypeReg16; AdrMode=RegZ;
       ChkOpSize(1);
       return;
      END
     if (strcasecmp(Asc,Reg8Names[RegZ])==0)
      BEGIN
       AdrType=TypeReg8; AdrMode=RegZ;
       ChkOpSize(0);
       return;
      END
    END

   for (RegZ=0; RegZ<=SegRegCnt; RegZ++)
    if (strcasecmp(Asc,SegRegNames[RegZ])==0)
     BEGIN
      AdrType=TypeRegSeg; AdrMode=RegZ;
      ChkOpSize(1);
      return;
     END

   if (FPUAvail)
    BEGIN
     if (strcasecmp(Asc,"ST")==0)
      BEGIN
       AdrType=TypeFReg; AdrMode=0;
       ChkOpSize(4);
       return;
      END

     if ((strlen(Asc)>4) AND (strncasecmp(Asc,"ST(",3)==0) AND (Asc[strlen(Asc)-1]==')'))
      BEGIN
       Asc[strlen(Asc)-1]='\0';
       AdrMode=EvalIntExpression(Asc+3,UInt3,&OK);
       if (OK)
	BEGIN
	 AdrType=TypeFReg;
	 ChkOpSize(4);
	END
       return;
      END
    END

   IsImm=True;
   IndexBuf=0; BaseBuf=0;
   DispAcc=0; FoundSize=(-1);

   if (strncasecmp(Asc,"WORD PTR",8)==0)
    BEGIN
     strcpy(Asc,Asc+8); FoundSize=1; IsImm=False;
     KillBlanks(Asc);
    END
   else if (strncasecmp(Asc,"BYTE PTR",8)==0)
    BEGIN
     strcpy(Asc,Asc+8); FoundSize=0; IsImm=False;
     KillBlanks(Asc);
    END
   else if (strncasecmp(Asc,"DWORD PTR",9)==0)
    BEGIN
     strcpy(Asc,Asc+9); FoundSize=2; IsImm=False;
     KillBlanks(Asc);
    END
   else if (strncasecmp(Asc,"QWORD PTR",9)==0)
    BEGIN
     strcpy(Asc,Asc+9); FoundSize=3; IsImm=False;
     KillBlanks(Asc);
    END
   else if (strncasecmp(Asc,"TBYTE PTR",9)==0)
    BEGIN
     strcpy(Asc,Asc+9); FoundSize=4; IsImm=False;
     KillBlanks(Asc);
    END

   if ((strlen(Asc)>2) AND (Asc[2]==':'))
    BEGIN
     strncpy(AddPart,Asc,2); AddPart[2]='\0';
     for (z=0; z<=SegRegCnt; z++)
      if (strcasecmp(AddPart,SegRegNames[z])==0)
       BEGIN
	strcpy(Asc,Asc+3); SegBuffer=z;
	AddPrefix(SegRegPrefixes[SegBuffer]);
       END
    END

   do
    BEGIN
     p=QuotPos(Asc,'['); HasAdr=(p!=Nil);


     if (p!=Asc)
      BEGIN
       FirstPassUnknown=False; if (p!=Nil) *p='\0';
       DispAcc+=EvalIntExpression(Asc,Int16,&OK);
       if (NOT OK) return;
       UnknownFlag=UnknownFlag OR FirstPassUnknown;
       MomSegment|=TypeFlag;
       if (FoundSize==-1) FoundSize=SizeFlag;
       if (p==Nil) *Asc='\0';
       else
        BEGIN
         *p='['; strcpy(Asc,p);
        END
      END

     if (HasAdr)
      BEGIN
       IsImm=False;

       p=RQuotPos(Asc,']'); if (p==Nil)
        BEGIN
         WrError(1300); return;
        END

       *p='\0'; strmaxcpy(AdrPart,Asc+1,255); strcpy(Asc,p+1);
       OldNegFlag=False;

       do
        BEGIN
         NegFlag=False;
         p1=QuotPos(AdrPart,'+'); p2=QuotPos(AdrPart,'-');
         if (((p1>p2) OR (p1==Nil)) AND (p2!=Nil))
 	  BEGIN
 	   p=p2; NegFlag=True;
 	  END
         else p=p1;

         if (p==Nil)
          BEGIN
           strcpy(AddPart,AdrPart); *AdrPart='\0';
          END
         else
          BEGIN
           *p='\0'; strcpy(AddPart,AdrPart); strcpy(AdrPart,p+1);
          END

         if (strcasecmp(AddPart,"BX")==0)
 	  BEGIN
 	   if ((OldNegFlag) OR (BaseBuf!=0)) return; else BaseBuf=1;
 	  END
         else if (strcasecmp(AddPart,"BP")==0)
 	  BEGIN
 	   if ((OldNegFlag) OR (BaseBuf!=0)) return; else BaseBuf=2;
 	  END
         else if (strcasecmp(AddPart,"SI")==0)
 	  BEGIN
 	   if ((OldNegFlag) OR (IndexBuf!=0)) return; else IndexBuf=1;
 	  END
         else if (strcasecmp(AddPart,"DI")==0)
 	  BEGIN
 	   if ((OldNegFlag) OR (IndexBuf!=0)) return; else IndexBuf=2;
 	  END
         else
 	  BEGIN
 	   FirstPassUnknown=False;
 	   DispSum=EvalIntExpression(AddPart,Int16,&OK);
 	   if (NOT OK) return;
 	   UnknownFlag=UnknownFlag OR FirstPassUnknown;
 	   if (OldNegFlag) DispAcc-=DispSum; else DispAcc+=DispSum;
 	   MomSegment|=TypeFlag;
 	   if (FoundSize==-1) FoundSize=SizeFlag;
 	  END
         OldNegFlag=NegFlag;
        END
       while (*AdrPart!='\0');
      END
    END
   while (*Asc!='\0');

   SumBuf=BaseBuf*10+IndexBuf;

   /* welches Segment effektiv benutzt ? */

   if (SegBuffer==-1) SegBuffer=(BaseBuf==2) ? 2 : 3;

   /* nur Displacement */

   if (SumBuf==0)

    /* immediate */

    if (IsImm)
     BEGIN
      if (((UnknownFlag) AND (OpSize==0)) OR (MinOneIs0())) DispAcc&=0xff;
      switch (OpSize)
       BEGIN
        case -1:
         WrError(1132); break;
        case 0:
         if ((DispAcc<-128) OR (DispAcc>255)) WrError(1320);
	 else
	  BEGIN
	   AdrType=TypeImm; AdrVals[0]=DispAcc & 0xff; AdrCnt=1;
	  END
         break;
        case 1:
	 AdrType=TypeImm;
	 AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc); AdrCnt=2;
	 break;
       END
     END

    /* absolut */

    else
     BEGIN
      AdrType=TypeMem; AdrMode=0x06;
      AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc); AdrCnt=2;
      if (FoundSize!=-1) ChkOpSize(FoundSize);
      ChkSpaces(SegBuffer,MomSegment);
     END

   /* kombiniert */

   else
    BEGIN
     AdrType=TypeMem;
     for (z=0; z<8; z++)
      if (SumBuf==RMCodes[z]) AdrMode=z;
     if (DispAcc==0)
      BEGIN
       if (SumBuf==20)
	BEGIN
	 AdrMode+=0x40; AdrVals[0]=0; AdrCnt=1;
	END
      END
     else if (AbleToSign(DispAcc))
      BEGIN
       AdrMode+=0x40;
       AdrVals[0]=DispAcc & 0xff; AdrCnt=1;
      END
     else
      BEGIN
       AdrMode+=0x80;
       AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc); AdrCnt=2;
      END
     ChkSpaces(SegBuffer,MomSegment);
     if (FoundSize!=-1) ChkOpSize(FoundSize);
    END
END

/*---------------------------------------------------------------------------*/

	static void DecodeMOV(Word Index)
BEGIN
   Byte AdrByte;

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1]);
     switch (AdrType)
      BEGIN
       case TypeReg8:
       case TypeReg16:
        AdrByte=AdrMode;
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
           BAsmCode[CodeLen++]=0x8a+OpSize;
           BAsmCode[CodeLen++]=0xc0+(AdrByte << 3)+AdrMode;
           break;
          case TypeMem:
           if ((AdrByte==0) AND (AdrMode==6))
            BEGIN
             BAsmCode[CodeLen]=0xa0+OpSize;
             MoveAdr(1);
             CodeLen+=1+AdrCnt;
            END
           else
            BEGIN
             BAsmCode[CodeLen++]=0x8a+OpSize;
             BAsmCode[CodeLen++]=AdrMode+(AdrByte << 3);
             MoveAdr(0); CodeLen+=AdrCnt;
            END
           break;
          case TypeRegSeg:
           if (OpSize==0) WrError(1131);
           else
            BEGIN
             BAsmCode[CodeLen++]=0x8c;
             BAsmCode[CodeLen++]=0xc0+(AdrMode << 3)+AdrByte;
            END
           break;
          case TypeImm:
           BAsmCode[CodeLen++]=0xb0+(OpSize << 3)+AdrByte;
           MoveAdr(0); CodeLen+=AdrCnt;
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       case TypeMem:
        BAsmCode[CodeLen+1]=AdrMode;
        MoveAdr(2); AdrByte=AdrCnt;
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
           if ((AdrMode==0) AND (BAsmCode[CodeLen+1]==6))
            BEGIN
             BAsmCode[CodeLen]=0xa2+OpSize;
             memmove(BAsmCode+CodeLen+1,BAsmCode+CodeLen+2,AdrByte);
             CodeLen+=1+AdrByte;
            END
           else
            BEGIN
             BAsmCode[CodeLen]=0x88+OpSize;
             BAsmCode[CodeLen+1]+=AdrMode << 3;
             CodeLen+=2+AdrByte;
            END
           break;
          case TypeRegSeg:
            BAsmCode[CodeLen]=0x8c;
           BAsmCode[CodeLen+1]+=AdrMode << 3;
           CodeLen+=2+AdrByte;
           break;
          case TypeImm:
           BAsmCode[CodeLen]=0xc6+OpSize;
           MoveAdr(2+AdrByte);
           CodeLen+=2+AdrByte+AdrCnt;
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       case TypeRegSeg:
        BAsmCode[CodeLen+1]=AdrMode << 3;
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg16:
            BAsmCode[CodeLen++]=0x8e;
           BAsmCode[CodeLen++]+=0xc0+AdrMode;
           break;
          case TypeMem:
           BAsmCode[CodeLen]=0x8e;
           BAsmCode[CodeLen+1]+=AdrMode;
           MoveAdr(2);
           CodeLen+=2+AdrCnt;
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       default:
        if (AdrType!=TypeNone) WrError(1350);
      END
    END
   AddPrefixes();
END

	static void DecodeINCDEC(Word Index)
BEGIN
   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1]);
     switch (AdrType)
      BEGIN
       case TypeReg16:
        BAsmCode[CodeLen]=0x40+AdrMode+Index;
        CodeLen++;
        break;
       case TypeReg8:
        BAsmCode[CodeLen]=0xfe;
        BAsmCode[CodeLen+1]=0xc0+AdrMode+Index;
        CodeLen+=2;
        break;
       case TypeMem:
        MinOneIs0();
        if (OpSize==-1) WrError(1132);
        else
         BEGIN
          BAsmCode[CodeLen]=0xfe + OpSize; /* ANSI :-0 */
          BAsmCode[CodeLen+1]=AdrMode+Index;
          MoveAdr(2);
          CodeLen+=2+AdrCnt;
         END
        break;
       default:
        if (AdrType!=TypeNone) WrError(1350);
      END
    END
   AddPrefixes();
END

	static void DecodeINT(Word Index)
BEGIN
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     BAsmCode[CodeLen+1]=EvalIntExpression(ArgStr[1],Int8,&OK);
     if (OK)
      if (BAsmCode[1]==3) BAsmCode[CodeLen++]=0xcc;
      else
       BEGIN
        BAsmCode[CodeLen]=0xcd; CodeLen+=2;
       END
    END
   AddPrefixes();
END

	static void DecodeINOUT(Word Index)
BEGIN
   Boolean OK;

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     if (Index!=0)
      BEGIN
       strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
      END
     DecodeAdr(ArgStr[1]);
     switch (AdrType)
      BEGIN
       case TypeReg8:
       case TypeReg16:
        if (AdrMode!=0) WrError(1350);
        else if (strcasecmp(ArgStr[2],"DX")==0)
          BAsmCode[CodeLen++]=0xec+OpSize+Index;
        else
         BEGIN
          BAsmCode[CodeLen+1]=EvalIntExpression(ArgStr[2],UInt8,&OK);
          if (OK)
           BEGIN
            ChkSpace(SegIO);
            BAsmCode[CodeLen]=0xe4+OpSize+Index;
            CodeLen+=2;
           END
         END
        break;
       default:
        if (AdrType!=TypeNone) WrError(1350);
      END
    END
   AddPrefixes();
END

	static void DecodeCALLJMP(Word Index)
BEGIN
   Byte AdrByte;
   Word AdrWord;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     if (strncmp(ArgStr[1],"SHORT ",6)==0)
      BEGIN
       AdrByte=2; strcpy(ArgStr[1],ArgStr[1]+6); KillPrefBlanks(ArgStr[1]);
      END
     else if ((strncmp(ArgStr[1],"LONG ",5)==0) OR (strncmp(ArgStr[1],"NEAR ",5)==0))
      BEGIN
       AdrByte=1; strcpy(ArgStr[1],ArgStr[1]+5); KillPrefBlanks(ArgStr[1]);
      END
     else AdrByte=0;
     OK=True;
     if (Index==0)
      if (AdrByte==2)
       BEGIN
        WrError(1350); OK=False;
       END
      else AdrByte=1;

     if (OK)
      BEGIN
       OpSize=1; DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeReg16:
	   BAsmCode[0]=0xff;
	   BAsmCode[1]=0xd0+AdrMode+(Index<<4);
	   CodeLen=2;
	   break;
         case TypeMem:
	   BAsmCode[0]=0xff;
	   BAsmCode[1]=AdrMode+0x10+(Index<<4);
	   MoveAdr(2);
	   CodeLen=2+AdrCnt;
	   break;
         case TypeImm:
	   ChkSpace(SegCode);
	   AdrWord=(((Word) AdrVals[1]) << 8)+AdrVals[0];
	   if ((AdrByte==2) OR ((AdrByte==0) AND (AbleToSign(AdrWord-EProgCounter()-2))))
	    BEGIN
	     AdrWord-=EProgCounter()+2;
	     if (NOT AbleToSign(AdrWord)) WrError(1330);
	     else
	      BEGIN
	       BAsmCode[0]=0xeb;
	       BAsmCode[1]=Lo(AdrWord);
	       CodeLen=2;
	      END
	    END
	   else
	    BEGIN
	     AdrWord-=EProgCounter()+3;
	     ChkSpace(SegCode);
	     BAsmCode[0]=0xe8+Index;
	     BAsmCode[1]=Lo(AdrWord);
	     BAsmCode[2]=Hi(AdrWord);
	     CodeLen=3;
	     AdrWord++;
	    END
	   break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
    END
   AddPrefixes();
END

	static void DecodePUSHPOP(Word Index)
BEGIN
   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     OpSize=1; DecodeAdr(ArgStr[1]);
     switch (AdrType)
      BEGIN
       case TypeReg16:
        BAsmCode[CodeLen]=0x50+AdrMode+(Index<<3);
        CodeLen++;
        break;
       case TypeRegSeg:
        BAsmCode[CodeLen]=0x06+(AdrMode << 3)+Index;
        CodeLen++;
        break;
       case TypeMem:
        BAsmCode[CodeLen]=0x8f; BAsmCode[CodeLen+1]=AdrMode;
        if (Index==0)
         BEGIN
          BAsmCode[CodeLen]+=0x70;
          BAsmCode[CodeLen+1]+=0x30;
         END
        MoveAdr(2);
        CodeLen+=2+AdrCnt;
        break;
       case TypeImm:
        if (MomCPU<CPU80186) WrError(1500);
        else if (Index==1) WrError(1350);
        else
         BEGIN
          BAsmCode[CodeLen]=0x68;
          BAsmCode[CodeLen+1]=AdrVals[0];
          if (Sgn(AdrVals[0])==AdrVals[1])
           BEGIN
            BAsmCode[CodeLen]+=2; CodeLen+=2;
           END
          else
           BEGIN
            BAsmCode[CodeLen+2]=AdrVals[1]; CodeLen+=3;
           END
         END
        break;
       default:
        if (AdrType!=TypeNone) WrError(1350);
      END
    END
   AddPrefixes();
END

	static void DecodeNOTNEG(Word Index)
BEGIN
   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1]);
     MinOneIs0();
     BAsmCode[CodeLen]=0xf6+OpSize;
     BAsmCode[CodeLen+1]=0x10+Index;
     switch (AdrType)
      BEGIN
       case TypeReg8:
       case TypeReg16:
        BAsmCode[CodeLen+1]+=0xc0+AdrMode;
        CodeLen+=2;
        break;
       case TypeMem:
        if (OpSize==-1) WrError(1132);
        else
         BEGIN
          BAsmCode[CodeLen+1]+=AdrMode;
          MoveAdr(2);
          CodeLen+=2+AdrCnt;
         END
        break;
       default:
        if (AdrType!=TypeNone) WrError(1350);
      END
    END
   AddPrefixes();
END

	static void DecodeRET(Word Index)
BEGIN
   Word AdrWord;
   Boolean OK;

   if (ArgCnt>1) WrError(1110);
   else if (ArgCnt==0)
    BAsmCode[CodeLen++]=0xc3+Index;
   else
    BEGIN
     AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
     if (OK)
      BEGIN
       BAsmCode[CodeLen++]=0xc2+Index;
       BAsmCode[CodeLen++]=Lo(AdrWord);
       BAsmCode[CodeLen++]=Hi(AdrWord);
      END
    END
END

	static void DecodeTEST(Word Index)
BEGIN
   Byte AdrByte;

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1]);
     switch (AdrType)
      BEGIN
       case TypeReg8:
       case TypeReg16:
        BAsmCode[CodeLen+1]=(AdrMode << 3);
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
           BAsmCode[CodeLen+1]+=0xc0+AdrMode;
           BAsmCode[CodeLen]=0x84+OpSize;
           CodeLen+=2;
           break;
          case TypeMem:
           BAsmCode[CodeLen+1]+=AdrMode;
           BAsmCode[CodeLen]=0x84+OpSize;
           MoveAdr(2);
           CodeLen+=2+AdrCnt;
           break;
          case TypeImm:
           if (((BAsmCode[CodeLen+1] >> 3) & 7)==0)
            BEGIN
             BAsmCode[CodeLen]=0xa8+OpSize;
             MoveAdr(1);
             CodeLen+=1+AdrCnt;
            END
           else
            BEGIN
             BAsmCode[CodeLen]=OpSize+0xf6;
             BAsmCode[CodeLen+1]=(BAsmCode[CodeLen+1] >> 3)+0xc0;
             MoveAdr(2);
             CodeLen+=2+AdrCnt;
            END
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       case TypeMem:
        BAsmCode[CodeLen+1]=AdrMode;
        AdrByte=AdrCnt; MoveAdr(2);
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
           BAsmCode[CodeLen]=0x84+OpSize;
           BAsmCode[CodeLen+1]+=(AdrMode << 3);
           CodeLen+=2+AdrByte;
           break;
          case TypeImm:
           BAsmCode[CodeLen]=OpSize+0xf6;
           MoveAdr(2+AdrByte);
           CodeLen+=2+AdrCnt+AdrByte;
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       default:
        if (AdrType!=TypeNone) WrError(1350);
      END
    END
   AddPrefixes();
END

	static void DecodeXCHG(Word Index)
BEGIN
   Byte AdrByte;

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1]);
     switch (AdrType)
      BEGIN
       case TypeReg8:
       case TypeReg16:
        AdrByte=AdrMode;
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
           if ((OpSize==1) AND ((AdrMode==0) OR (AdrByte==0)))
            BEGIN
             BAsmCode[CodeLen]=0x90+AdrMode+AdrByte;
             CodeLen++;
            END
           else
            BEGIN
             BAsmCode[CodeLen]=0x86+OpSize;
             BAsmCode[CodeLen+1]=AdrMode+0xc0+(AdrByte << 3);
             CodeLen+=2;
            END
           break;
          case TypeMem:
           BAsmCode[CodeLen]=0x86+OpSize;
           BAsmCode[CodeLen+1]=AdrMode+(AdrByte << 3);
           MoveAdr(2);
           CodeLen+=AdrCnt+2;
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       case TypeMem:
        BAsmCode[CodeLen+1]=AdrMode;
        MoveAdr(2); AdrByte=AdrCnt;
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
           BAsmCode[CodeLen]=0x86+OpSize;
           BAsmCode[CodeLen+1]+=(AdrMode << 3);
           CodeLen+=AdrByte+2;
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       default:
        if (AdrType!=TypeNone) WrError(1350);
      END
    END
   AddPrefixes();
END

	static void DecodeCALLJMPF(Word Index)
BEGIN
   char *p;
   Word AdrWord;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     p=QuotPos(ArgStr[1],':');
     if (p==Nil)
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeMem:
          BAsmCode[CodeLen]=0xff;
          BAsmCode[CodeLen+1]=AdrMode+0x18+Index;
          MoveAdr(2);
          CodeLen+=2+AdrCnt;
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     else
      BEGIN
       *p='\0';
       AdrWord=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        BEGIN
         BAsmCode[CodeLen+3]=Lo(AdrWord);
         BAsmCode[CodeLen+4]=Hi(AdrWord);
         AdrWord=EvalIntExpression(p+1,UInt16,&OK);
         if (OK)
          BEGIN
           BAsmCode[CodeLen+1]=Lo(AdrWord);
           BAsmCode[CodeLen+2]=Hi(AdrWord);
           BAsmCode[CodeLen]=0x9a+Index;
           CodeLen+=5;
          END
        END
      END
    END
   AddPrefixes();
END

	static void DecodeENTER(Word Index)
BEGIN
   Word AdrWord;
   Boolean OK;

   if (ArgCnt!=2) WrError(1110);
   else if (MomCPU<CPU80186) WrError(1500);
   else
    BEGIN
     AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
     if (OK)
      BEGIN
       BAsmCode[CodeLen+1]=Lo(AdrWord);
       BAsmCode[CodeLen+2]=Hi(AdrWord);
       BAsmCode[CodeLen+3]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
        BEGIN
         BAsmCode[CodeLen]=0xc8; CodeLen+=4;
        END
      END
    END
   AddPrefixes();
END

	static void DecodeFixed(Word Index)
BEGIN
   FixedOrder *FixedZ=FixedOrders+Index;

   if (ArgCnt!=0) WrError(1110);
   else if (MomCPU<FixedZ->MinCPU) WrError(1500);
   else PutCode(FixedZ->Code);
   AddPrefixes();
END

	static void DecodeALU2(Word Index)
BEGIN
   Byte AdrByte;

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1]);
     switch (AdrType)
      BEGIN
       case TypeReg8:
       case TypeReg16:
        BAsmCode[CodeLen+1]=AdrMode << 3;
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
           BAsmCode[CodeLen+1]+=0xc0+AdrMode;
           BAsmCode[CodeLen]=(Index << 3)+2+OpSize;
           CodeLen+=2;
           break;
          case TypeMem:
           BAsmCode[CodeLen+1]+=AdrMode;
           BAsmCode[CodeLen]=(Index << 3)+2+OpSize;
           MoveAdr(2);
           CodeLen+=2+AdrCnt;
           break;
          case TypeImm:
           if (((BAsmCode[CodeLen+1] >> 3) & 7)==0)
            BEGIN
             BAsmCode[CodeLen]=(Index << 3)+4+OpSize;
             MoveAdr(1);
             CodeLen+=1+AdrCnt;
            END
           else
            BEGIN
             BAsmCode[CodeLen]=OpSize+0x80;
             if ((OpSize==1) AND (Sgn(AdrVals[0])==AdrVals[1]))
              BEGIN
               AdrCnt=1; BAsmCode[CodeLen]+=2;
              END
             BAsmCode[CodeLen+1]=(BAsmCode[CodeLen+1] >> 3)+0xc0+(Index << 3);
             MoveAdr(2);
             CodeLen+=2+AdrCnt;
            END
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       case TypeMem:
        BAsmCode[CodeLen+1]=AdrMode;
        AdrByte=AdrCnt; MoveAdr(2);
        DecodeAdr(ArgStr[2]);
        switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
           BAsmCode[CodeLen]=(Index << 3)+OpSize;
           BAsmCode[CodeLen+1]+=(AdrMode << 3);
           CodeLen+=2+AdrByte;
           break;
          case TypeImm:
           BAsmCode[CodeLen]=OpSize+0x80;
           if ((OpSize==1) AND (Sgn(AdrVals[0])==AdrVals[1]))
            BEGIN
             AdrCnt=1; BAsmCode[CodeLen]+=2;
            END
           BAsmCode[CodeLen+1]+=(Index << 3);
           MoveAdr(2+AdrByte);
           CodeLen+=2+AdrCnt+AdrByte;
           break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
        break;
       default: 
        if (AdrType!=TypeNone) WrError(1350);
      END
    END
   AddPrefixes();
END

	static void DecodeRel(Word Index)
BEGIN
   FixedOrder *RelZ=RelOrders+Index;
   Word AdrWord;
   Boolean OK;

   if (ArgCnt!=1) WrError(1110);
   else if (MomCPU<RelZ->MinCPU) WrError(1500);
   else
    BEGIN
     AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
     if (OK)
      BEGIN
       ChkSpace(SegCode);
       AdrWord-=EProgCounter()+2;
       if (Hi(RelZ->Code)!=0) AdrWord--;
       if ((AdrWord>=0x80) AND (AdrWord<0xff80) AND (NOT SymbolQuestionable)) WrError(1370);
       else
        BEGIN
         PutCode(RelZ->Code); BAsmCode[CodeLen++]=Lo(AdrWord);
        END
     END
    END
END

/*---------------------------------------------------------------------------*/

	static void AddFixed(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255); 
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].MinCPU=NMin;
   FixedOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeFixed);
END

        static void AddFPUFixed(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=FPUFixedOrderCnt) exit(255); 
   FPUFixedOrders[InstrZ].Name=NName;
   FPUFixedOrders[InstrZ].MinCPU=NMin;
   FPUFixedOrders[InstrZ++].Code=NCode;
END

        static void AddFPUSt(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=FPUStOrderCnt) exit(255); 
   FPUStOrders[InstrZ].Name=NName;
   FPUStOrders[InstrZ].MinCPU=NMin;
   FPUStOrders[InstrZ++].Code=NCode;
END

        static void AddFPU16(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=FPU16OrderCnt) exit(255);
   FPU16Orders[InstrZ].Name=NName;
   FPU16Orders[InstrZ].MinCPU=NMin;
   FPU16Orders[InstrZ++].Code=NCode;
END

	static void AddString(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=StringOrderCnt) exit(255); 
   StringOrders[InstrZ].Name=NName;
   StringOrders[InstrZ].MinCPU=NMin;
   StringOrders[InstrZ++].Code=NCode;
END

        static void AddRept(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=ReptOrderCnt) exit(255);
   ReptOrders[InstrZ].Name=NName;
   ReptOrders[InstrZ].MinCPU=NMin;
   ReptOrders[InstrZ++].Code=NCode;
END

	static void AddRel(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255); 
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ].MinCPU=NMin;
   RelOrders[InstrZ].Code=NCode;
   AddInstTable(InstTable,NName,InstrZ++,DecodeRel);
END

        static void AddModReg(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=ModRegOrderCnt) exit(255);
   ModRegOrders[InstrZ].Name=NName; 
   ModRegOrders[InstrZ].MinCPU=NMin; 
   ModRegOrders[InstrZ++].Code=NCode;
END

        static void AddShift(char *NName, CPUVar NMin, Word NCode)
BEGIN
   if (InstrZ>=ShiftOrderCnt) exit(255);
   ShiftOrders[InstrZ].Name=NName;
   ShiftOrders[InstrZ].MinCPU=NMin; 
   ShiftOrders[InstrZ++].Code=NCode;
END

        static void AddReg16(char *NName, CPUVar NMin, Word NCode, Byte NAdd)
BEGIN
   if (InstrZ>=Reg16OrderCnt) exit(255);
   Reg16Orders[InstrZ].Name=NName;
   Reg16Orders[InstrZ].MinCPU=NMin;
   Reg16Orders[InstrZ].Code=NCode;
   Reg16Orders[InstrZ++].Add=NAdd;
END

	static void InitFields(void)
BEGIN
   InstTable=CreateInstTable(201);
   AddInstTable(InstTable,"MOV"  ,0,DecodeMOV);
   AddInstTable(InstTable,"INC"  ,0,DecodeINCDEC);
   AddInstTable(InstTable,"DEC"  ,8,DecodeINCDEC);
   AddInstTable(InstTable,"INT"  ,0,DecodeINT);
   AddInstTable(InstTable,"IN"   ,0,DecodeINOUT);
   AddInstTable(InstTable,"OUT"  ,2,DecodeINOUT);
   AddInstTable(InstTable,"CALL" ,0,DecodeCALLJMP);
   AddInstTable(InstTable,"JMP"  ,1,DecodeCALLJMP);
   AddInstTable(InstTable,"PUSH" ,0,DecodePUSHPOP);
   AddInstTable(InstTable,"POP"  ,1,DecodePUSHPOP);
   AddInstTable(InstTable,"NOT"  ,0,DecodeNOTNEG);
   AddInstTable(InstTable,"NEG"  ,8,DecodeNOTNEG);
   AddInstTable(InstTable,"RET"  ,0,DecodeRET);
   AddInstTable(InstTable,"RETF" ,8,DecodeRET);
   AddInstTable(InstTable,"TEST" ,0,DecodeTEST);
   AddInstTable(InstTable,"XCHG" ,0,DecodeXCHG);
   AddInstTable(InstTable,"CALLF",16,DecodeCALLJMPF);
   AddInstTable(InstTable,"JMPF" ,0,DecodeCALLJMPF);
   AddInstTable(InstTable,"ENTER",0,DecodeENTER);

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("AAA",   CPU8086,  0x0037);  AddFixed("AAS",   CPU8086,  0x003f);
   AddFixed("AAM",   CPU8086,  0xd40a);  AddFixed("AAD",   CPU8086,  0xd50a);
   AddFixed("CBW",   CPU8086,  0x0098);  AddFixed("CLC",   CPU8086,  0x00f8);
   AddFixed("CLD",   CPU8086,  0x00fc);  AddFixed("CLI",   CPU8086,  0x00fa);
   AddFixed("CMC",   CPU8086,  0x00f5);  AddFixed("CWD",   CPU8086,  0x0099);
   AddFixed("DAA",   CPU8086,  0x0027);  AddFixed("DAS",   CPU8086,  0x002f);
   AddFixed("HLT",   CPU8086,  0x00f4);  AddFixed("INTO",  CPU8086,  0x00ce);
   AddFixed("IRET",  CPU8086,  0x00cf);  AddFixed("LAHF",  CPU8086,  0x009f);
   AddFixed("LOCK",  CPU8086,  0x00f0);  AddFixed("NOP",   CPU8086,  0x0090);
   AddFixed("POPF",  CPU8086,  0x009d);  AddFixed("PUSHF", CPU8086,  0x009c);
   AddFixed("SAHF",  CPU8086,  0x009e);  AddFixed("STC",   CPU8086,  0x00f9);
   AddFixed("STD",   CPU8086,  0x00fd);  AddFixed("STI",   CPU8086,  0x00fb);
   AddFixed("WAIT",  CPU8086,  0x009b);  AddFixed("XLAT",  CPU8086,  0x00d7);
   AddFixed("LEAVE", CPU80186, 0x00c9);  AddFixed("PUSHA", CPU80186, 0x0060);
   AddFixed("POPA",  CPU80186, 0x0061);  AddFixed("ADD4S", CPUV30,   0x0f20);
   AddFixed("SUB4S", CPUV30,   0x0f22);  AddFixed("CMP4S", CPUV30,   0x0f26);
   AddFixed("STOP",  CPUV35,   0x0f9e);  AddFixed("RETRBI",CPUV35,   0x0f91);
   AddFixed("FINT",  CPUV35,   0x0f92);  AddFixed("MOVSPA",CPUV35,   0x0f25);
   AddFixed("SEGES", CPU8086,  0x0026);  AddFixed("SEGCS", CPU8086,  0x002e);
   AddFixed("SEGSS", CPU8086,  0x0036);  AddFixed("SEGDS", CPU8086,  0x003e);
   AddFixed("FWAIT", CPU8086,  0x009b);  

   FPUFixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FPUFixedOrderCnt); InstrZ=0;
   AddFPUFixed("FCOMPP", CPU8086, 0xded9); AddFPUFixed("FTST",   CPU8086, 0xd9e4);
   AddFPUFixed("FXAM",   CPU8086, 0xd9e5); AddFPUFixed("FLDZ",   CPU8086, 0xd9ee);
   AddFPUFixed("FLD1",   CPU8086, 0xd9e8); AddFPUFixed("FLDPI",  CPU8086, 0xd9eb);
   AddFPUFixed("FLDL2T", CPU8086, 0xd9e9); AddFPUFixed("FLDL2E", CPU8086, 0xd9ea);
   AddFPUFixed("FLDLG2", CPU8086, 0xd9ec); AddFPUFixed("FLDLN2", CPU8086, 0xd9ed);
   AddFPUFixed("FSQRT",  CPU8086, 0xd9fa); AddFPUFixed("FSCALE", CPU8086, 0xd9fd);
   AddFPUFixed("FPREM",  CPU8086, 0xd9f8); AddFPUFixed("FRNDINT",CPU8086, 0xd9fc);
   AddFPUFixed("FXTRACT",CPU8086, 0xd9f4); AddFPUFixed("FABS",   CPU8086, 0xd9e1);
   AddFPUFixed("FCHS",   CPU8086, 0xd9e0); AddFPUFixed("FPTAN",  CPU8086, 0xd9f2);
   AddFPUFixed("FPATAN", CPU8086, 0xd9f3); AddFPUFixed("F2XM1",  CPU8086, 0xd9f0);
   AddFPUFixed("FYL2X",  CPU8086, 0xd9f1); AddFPUFixed("FYL2XP1",CPU8086, 0xd9f9);
   AddFPUFixed("FINIT",  CPU8086, 0xdbe3); AddFPUFixed("FENI",   CPU8086, 0xdbe0);
   AddFPUFixed("FDISI",  CPU8086, 0xdbe1); AddFPUFixed("FCLEX",  CPU8086, 0xdbe2);
   AddFPUFixed("FINCSTP",CPU8086, 0xd9f7); AddFPUFixed("FDECSTP",CPU8086, 0xd9f6);
   AddFPUFixed("FNOP",   CPU8086, 0xd9d0);

   FPUStOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FPUStOrderCnt); InstrZ=0;
   AddFPUSt("FXCH",  CPU8086, 0xd9c8);
   AddFPUSt("FFREE", CPU8086, 0xddc0);

   FPU16Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*FPU16OrderCnt); InstrZ=0;
   AddFPU16("FLDCW",  CPU8086, 0xd928);
   AddFPU16("FSTCW",  CPU8086, 0xd938);
   AddFPU16("FSTSW",  CPU8086, 0xdd38);
   AddFPU16("FSTENV", CPU8086, 0xd930);
   AddFPU16("FLDENV", CPU8086, 0xd920);

   StringOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*StringOrderCnt); InstrZ=0;
   AddString("CMPSB", CPU8086,  0x00a6);
   AddString("CMPSW", CPU8086,  0x00a7);
   AddString("LODSB", CPU8086,  0x00ac);
   AddString("LODSW", CPU8086,  0x00ad);
   AddString("MOVSB", CPU8086,  0x00a4);
   AddString("MOVSW", CPU8086,  0x00a5);
   AddString("SCASB", CPU8086,  0x00ae);
   AddString("SCASW", CPU8086,  0x00af);
   AddString("STOSB", CPU8086,  0x00aa);
   AddString("STOSW", CPU8086,  0x00ab);
   AddString("INSB",  CPU80186, 0x006c);
   AddString("INSW",  CPU80186, 0x006d);
   AddString("OUTSB", CPU80186, 0x006e);
   AddString("OUTSW", CPU80186, 0x006f);

   ReptOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*ReptOrderCnt); InstrZ=0;
   AddRept("REP",   CPU8086,  0x00f3);
   AddRept("REPE",  CPU8086,  0x00f3);
   AddRept("REPZ",  CPU8086,  0x00f3);
   AddRept("REPNE", CPU8086,  0x00f2);
   AddRept("REPNZ", CPU8086,  0x00f2);
   AddRept("REPC",  CPUV30,   0x0065);
   AddRept("REPNC", CPUV30,   0x0064);

   RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCnt); InstrZ=0;
   AddRel("JA",    CPU8086, 0x0077); AddRel("JNBE",  CPU8086, 0x0077);
   AddRel("JAE",   CPU8086, 0x0073); AddRel("JNB",   CPU8086, 0x0073);
   AddRel("JB",    CPU8086, 0x0072); AddRel("JNAE",  CPU8086, 0x0072);
   AddRel("JBE",   CPU8086, 0x0076); AddRel("JNA",   CPU8086, 0x0076);
   AddRel("JC",    CPU8086, 0x0072); AddRel("JCXZ",  CPU8086, 0x00e3);
   AddRel("JE",    CPU8086, 0x0074); AddRel("JZ",    CPU8086, 0x0074);
   AddRel("JG",    CPU8086, 0x007f); AddRel("JNLE",  CPU8086, 0x007f);
   AddRel("JGE",   CPU8086, 0x007d); AddRel("JNL",   CPU8086, 0x007d);
   AddRel("JL",    CPU8086, 0x007c); AddRel("JNGE",  CPU8086, 0x007c);
   AddRel("JLE",   CPU8086, 0x007e); AddRel("JNG",   CPU8086, 0x007e);
   AddRel("JNC",   CPU8086, 0x0073); AddRel("JNE",   CPU8086, 0x0075);
   AddRel("JNZ",   CPU8086, 0x0075); AddRel("JNO",   CPU8086, 0x0071);
   AddRel("JNS",   CPU8086, 0x0079); AddRel("JNP",   CPU8086, 0x007b);
   AddRel("JPO",   CPU8086, 0x007b); AddRel("JO",    CPU8086, 0x0070);
   AddRel("JP",    CPU8086, 0x007a); AddRel("JPE",   CPU8086, 0x007a);
   AddRel("JS",    CPU8086, 0x0078); AddRel("LOOP",  CPU8086, 0x00e2);
   AddRel("LOOPE", CPU8086, 0x00e1); AddRel("LOOPZ", CPU8086, 0x00e1);
   AddRel("LOOPNE",CPU8086, 0x00e0); AddRel("LOOPNZ",CPU8086, 0x00e0);

   ModRegOrders=(FixedOrder *) malloc (sizeof(FixedOrder)*ModRegOrderCnt); InstrZ=0;
   AddModReg("LDS",   CPU8086,  0x00c5);
   AddModReg("LEA",   CPU8086,  0x008d);
   AddModReg("LES",   CPU8086,  0x00c4);
   AddModReg("BOUND", CPU80186, 0x0062);

   ShiftOrders=(FixedOrder *) malloc (sizeof(FixedOrder)*ShiftOrderCnt); InstrZ=0;
   AddShift("SHL",   CPU8086, 4); AddShift("SAL",   CPU8086, 4);
   AddShift("SHR",   CPU8086, 5); AddShift("SAR",   CPU8086, 7);
   AddShift("ROL",   CPU8086, 0); AddShift("ROR",   CPU8086, 1);
   AddShift("RCL",   CPU8086, 2); AddShift("RCR",   CPU8086, 3);

   Reg16Orders=(AddOrder *) malloc(sizeof(AddOrder)*Reg16OrderCnt); InstrZ=0;
   AddReg16("BRKCS", CPUV35, 0x0f2d, 0xc0);
   AddReg16("TSKSW", CPUV35, 0x0f94, 0xf8);
   AddReg16("MOVSPB",CPUV35, 0x0f95, 0xf8);

   InstrZ=0;
   AddInstTable(InstTable,"ADD",InstrZ++,DecodeALU2);
   AddInstTable(InstTable,"OR" ,InstrZ++,DecodeALU2);
   AddInstTable(InstTable,"ADC",InstrZ++,DecodeALU2);
   AddInstTable(InstTable,"SBB",InstrZ++,DecodeALU2);
   AddInstTable(InstTable,"AND",InstrZ++,DecodeALU2);
   AddInstTable(InstTable,"SUB",InstrZ++,DecodeALU2);
   AddInstTable(InstTable,"XOR",InstrZ++,DecodeALU2);
   AddInstTable(InstTable,"CMP",InstrZ++,DecodeALU2);

   MulOrders=(char **) malloc(sizeof(char *)*MulOrderCnt); InstrZ=0;
   MulOrders[InstrZ++]="MUL"; MulOrders[InstrZ++]="IMUL";
   MulOrders[InstrZ++]="DIV"; MulOrders[InstrZ++]="IDIV";

   Bit1Orders=(char **) malloc(sizeof(char *)*Bit1OrderCnt); InstrZ=0;
   Bit1Orders[InstrZ++]="TEST1";
   Bit1Orders[InstrZ++]="CLR1";
   Bit1Orders[InstrZ++]="SET1";
   Bit1Orders[InstrZ++]="NOT1";
END

	static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(FixedOrders);
   free(FPUFixedOrders);
   free(FPUStOrders);
   free(FPU16Orders);
   free(StringOrders);
   free(ReptOrders);
   free(RelOrders);
   free(ModRegOrders);
   free(ShiftOrders);
   free(Reg16Orders);
   free(MulOrders);
   free(Bit1Orders);
END

	static Boolean FMemo(char *Name)
BEGIN
   String tmp;

   if (Memo(Name)) 
    BEGIN
     AddPrefix(0x9b); return True;
    END
   else
    BEGIN
     strmaxcpy(tmp,Name,255);
     memmove(tmp+2,tmp+1,strlen(tmp));
     tmp[1]='N';
     return Memo(tmp);
    END
END

	static Boolean DecodePseudo(void)
BEGIN
   Boolean OK;
   int z,z2,z3;
   char *p;
   String SegPart,ValPart;

   if (Memo("PORT"))
    BEGIN
     CodeEquate(SegIO,0,0xffff);
     return True;
    END

   if (Memo("ASSUME"))
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       z=1; OK=True;
       while ((z<=ArgCnt) AND (OK))
	BEGIN
	 OK=False; p=QuotPos(ArgStr[z],':');
         if (p!=Nil)
          BEGIN
           *p='\0'; strmaxcpy(SegPart,ArgStr[z],255); strmaxcpy(ValPart,p+1,255);
          END
         else
          BEGIN
           strmaxcpy(SegPart,ArgStr[z],255); *ValPart='\0';
          END
	 z2=0;
	 while ((z2<=SegRegCnt) AND (strcasecmp(SegPart,SegRegNames[z2])!=0)) z2++;
	 if (z2>SegRegCnt) WrXError(1962,SegPart);
	 else
	  BEGIN
	   z3=0;
	   while ((z3<=PCMax) AND (strcasecmp(ValPart,SegNames[z3])!=0)) z3++;
	   if (z3>PCMax) WrXError(1961,ValPart);
	   else if ((z3!=SegCode) AND (z3!=SegData) AND (z3!=SegXData) AND (z3!=SegNone)) WrError(1960);
	   else
	    BEGIN
	     SegAssumes[z2]=z3; OK=True;
	    END
	  END
	 z++;
	END
      END
     return True;
    END

   return False;
END

	static Boolean DecodeFPU(void)
BEGIN
   int z;
   Byte OpAdd;

   if (*OpPart!='F') return False;
   if (NOT FPUAvail) return False;

   for (z=0; z<FPUFixedOrderCnt; z++)
    if (FMemo(FPUFixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<FPUFixedOrders[z].MinCPU) WrError(1500);
      else PutCode(FPUFixedOrders[z].Code);
      return True;
     END

   for (z=0; z<FPUStOrderCnt; z++)
    if (FMemo(FPUStOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
	DecodeAdr(ArgStr[1]);
        if (AdrType==TypeFReg)
	 BEGIN
	  PutCode(FPUStOrders[z].Code); BAsmCode[CodeLen-1]+=AdrMode;
         END
        else if (AdrType!=TypeNone) WrError(1350);
       END
      return True;
     END

   if (FMemo("FLD"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  BAsmCode[CodeLen]=0xd9; BAsmCode[CodeLen+1]=0xc0+AdrMode;
	  CodeLen+=2;
	  break;
         case TypeMem:
          if ((OpSize==-1) AND (UnknownFlag)) OpSize=2;
          if (OpSize==-1) WrError(1132);
	  else if (OpSize<2) WrError(1130);
	  else
	   BEGIN
	    MoveAdr(2);
	    BAsmCode[CodeLen+1]=AdrMode;
	    switch (OpSize)
             BEGIN
	      case 2: BAsmCode[CodeLen]=0xd9; break;
	      case 3: BAsmCode[CodeLen]=0xdd; break;
	      case 4: BAsmCode[CodeLen]=0xdb; BAsmCode[CodeLen+1]+=0x28; break;
	     END
	    CodeLen+=2+AdrCnt;
	   END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if (FMemo("FILD"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeMem:
          if ((OpSize==-1) AND (UnknownFlag)) OpSize=1;
          if (OpSize==-1) WrError(1132);
          else if ((OpSize<1) OR (OpSize>3)) WrError(1130);
          else
           BEGIN
            MoveAdr(2);
            BAsmCode[CodeLen+1]=AdrMode;
            switch (OpSize)
             BEGIN
              case 1: BAsmCode[CodeLen]=0xdf; break;
              case 2: BAsmCode[CodeLen]=0xdb; break;
              case 3: BAsmCode[CodeLen]=0xdf; BAsmCode[CodeLen+1]+=0x28; break;
             END
            CodeLen+=2+AdrCnt;
           END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if (FMemo("FBLD"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeMem:
          if ((OpSize==-1) AND (UnknownFlag)) OpSize=4;
          if (OpSize==-1) WrError(1132);
	  else if (OpSize!=4) WrError(1130);
          else
           BEGIN
            BAsmCode[CodeLen]=0xdf;
            MoveAdr(2);
            BAsmCode[CodeLen+1]=AdrMode+0x20;
            CodeLen+=2+AdrCnt;
           END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FST")) OR (FMemo("FSTP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  BAsmCode[CodeLen]=0xdd; BAsmCode[CodeLen+1]=0xd0+AdrMode;
	  if (FMemo("FSTP")) BAsmCode[CodeLen+1]+=8;
	  CodeLen+=2;
	  break;
         case TypeMem:
          if ((OpSize==-1) AND (UnknownFlag)) OpSize=2;
 	  if (OpSize==-1) WrError(1132);
	  else if ((OpSize<2) OR ((OpSize==4) AND (FMemo("FST")))) WrError(1130);
	  else
	   BEGIN
	    MoveAdr(2);
	    BAsmCode[CodeLen+1]=AdrMode+0x10;
	    if (FMemo("FSTP")) BAsmCode[CodeLen+1]+=8;
	    switch (OpSize)
             BEGIN
	      case 2: BAsmCode[CodeLen]=0xd9; break;
	      case 3: BAsmCode[CodeLen]=0xdd;
	      case 4: BAsmCode[CodeLen]=0xdb; BAsmCode[CodeLen+1]+=0x20; break;
	     END
	    CodeLen+=2+AdrCnt;
	   END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FIST")) OR (FMemo("FISTP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeMem:
          if ((OpSize==-1) AND (UnknownFlag)) OpSize=1;
          if (OpSize==-1) WrError(1132);
	  else if ((OpSize<1) OR (OpSize==4) OR ((OpSize==3) AND (FMemo("FIST")))) WrError(1130);
          else
           BEGIN
            MoveAdr(2);
            BAsmCode[CodeLen+1]=AdrMode+0x10;
            if (FMemo("FISTP")) BAsmCode[CodeLen+1]+=8;
            switch (OpSize)
             BEGIN
              case 1: BAsmCode[CodeLen]=0xdf; break;
              case 2: BAsmCode[CodeLen]=0xdb; break;
              case 3: BAsmCode[CodeLen]=0xdf; BAsmCode[CodeLen+1]=0x20; break;
             END
            CodeLen+=2+AdrCnt;
           END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if (FMemo("FBSTP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeMem:
          if ((OpSize==-1) AND (UnknownFlag)) OpSize=1;
	  if (OpSize==-1) WrError(1132);
	  else if (OpSize!=4) WrError(1130);
 	  else
	   BEGIN
	    BAsmCode[CodeLen]=0xdf; BAsmCode[CodeLen+1]=AdrMode+0x30;
	    MoveAdr(2);
	    CodeLen+=2+AdrCnt;
	   END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FCOM")) OR (FMemo("FCOMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  BAsmCode[CodeLen]=0xd8; BAsmCode[CodeLen+1]=0xd0+AdrMode;
	  if (FMemo("FCOMP")) BAsmCode[CodeLen+1]+=8;
	  CodeLen+=2;
	  break;
         case TypeMem:
          if ((OpSize==-1) AND (UnknownFlag)) OpSize=1;
	  if (OpSize==-1) WrError(1132);
	  else if ((OpSize!=2) AND (OpSize!=3)) WrError(1130);
 	  else
	   BEGIN
	    BAsmCode[CodeLen]=(OpSize==2) ? 0xd8 : 0xdc;
	    BAsmCode[CodeLen+1]=AdrMode+0x10;
	    if (FMemo("FCOMP")) BAsmCode[CodeLen+1]+=8;
	    MoveAdr(2);
	    CodeLen+=2+AdrCnt;
	   END;
          break;
         default: 
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FICOM")) OR (FMemo("FICOMP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeMem:
          if ((OpSize==-1) AND (UnknownFlag)) OpSize=1;
          if (OpSize==-1) WrError(1132);
	  else if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
	  else
	   BEGIN
	    BAsmCode[CodeLen]=(OpSize==1) ? 0xde  : 0xda;
	    BAsmCode[CodeLen+1]=AdrMode+0x10;
	    if (FMemo("FICOMP")) BAsmCode[CodeLen+1]+=8;
	    MoveAdr(2);
	    CodeLen+=2+AdrCnt;
	   END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FADD")) OR (FMemo("FMUL")))
    BEGIN
     OpAdd=0; if (FMemo("FMUL")) OpAdd+=8;
     if (ArgCnt==0)
      BEGIN
       BAsmCode[CodeLen]=0xde; BAsmCode[CodeLen+1]=0xc1+OpAdd;
       CodeLen+=2;
       return True;
      END
     if (ArgCnt==1)
      BEGIN
       strcpy (ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"ST",255); ArgCnt++;
      END
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]); OpSize=(-1);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  if (AdrMode!=0)   /* ST(i) ist Ziel */
 	   BEGIN
	    BAsmCode[CodeLen+1]=AdrMode;
	    DecodeAdr(ArgStr[2]);
	    if ((AdrType!=TypeFReg) OR (AdrMode!=0)) WrError(1350);
	    else
	     BEGIN
	      BAsmCode[CodeLen]=0xdc; BAsmCode[CodeLen+1]+=0xc0+OpAdd;
	      CodeLen+=2;
	     END
	   END
	  else                      /* ST ist Ziel */
	   BEGIN
	    DecodeAdr(ArgStr[2]);
	    switch (AdrType)
             BEGIN
	      case TypeFReg:
	       BAsmCode[CodeLen]=0xd8;
               BAsmCode[CodeLen+1]=0xc0+AdrMode+OpAdd;
	       CodeLen+=2;
	       break;
	      case TypeMem:
               if ((OpSize==-1) AND (UnknownFlag)) OpSize=2;
	       if (OpSize==-1) WrError(1132);
	       else if ((OpSize!=2) AND (OpSize!=3)) WrError(1130);
	       else
	        BEGIN
                 BAsmCode[CodeLen]=(OpSize==2) ? 0xd8 : 0xdc;
                 BAsmCode[CodeLen+1]=AdrMode+OpAdd;
	         MoveAdr(2);
	         CodeLen+=AdrCnt+2;
	        END
               break;
              default:
               if (AdrType!=TypeNone) WrError(1350);
	     END
	   END;
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FIADD")) OR (FMemo("FIMUL")))
    BEGIN
     OpAdd=0; if (FMemo("FIIMUL")) OpAdd+=8;
     if (ArgCnt==1)
      BEGIN
       ArgCnt=2; strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"ST",255);
      END
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  if (AdrMode!=0) WrError(1350);
	  else
	   BEGIN
	    OpSize=(-1);
	    DecodeAdr(ArgStr[2]);
	    if ((AdrType!=TypeMem) AND (AdrType!=TypeNone)) WrError(1350);
            else if (AdrType!=TypeNone)
             BEGIN
              if ((OpSize==-1) AND (UnknownFlag)) OpSize=1;
              if (OpSize==-1) WrError(1132);
	      else if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
	      else
	       BEGIN
                BAsmCode[CodeLen]=(OpSize==1) ? 0xde : 0xda;
	        BAsmCode[CodeLen+1]=AdrMode+OpAdd;
	        MoveAdr(2);
	        CodeLen+=2+AdrCnt;
	       END
             END
	   END
          break;
         default: 
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FADDP")) OR (FMemo("FMULP")))
    BEGIN
     OpAdd=0; if (FMemo("FMULP")) OpAdd+=8;
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2]);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  if (AdrMode!=0) WrError(1350);
	  else
	   BEGIN
	    DecodeAdr(ArgStr[1]);
	    if ((AdrType!=TypeFReg) AND (AdrType!=TypeNone)) WrError(1350);
	    else if (AdrType!=TypeNone)
	     BEGIN
	      BAsmCode[CodeLen]=0xde;
	      BAsmCode[CodeLen+1]=0xc0+AdrMode+OpAdd;
	      CodeLen+=2;
	     END
	   END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FSUB")) OR (FMemo("FSUBR")) OR (FMemo("FDIV")) OR (FMemo("FDIVR")))
    BEGIN
     OpAdd=0;
     if ((FMemo("FSUBR")) OR (FMemo("FDIVR"))) OpAdd+=8;
     if ((FMemo("FDIV")) OR (FMemo("FDIVR"))) OpAdd+=16;
     if (ArgCnt==0)
      BEGIN
       BAsmCode[CodeLen]=0xde; BAsmCode[CodeLen+1]=0xe1+(OpAdd ^ 8);
       CodeLen+=2;
       return True;
      END
     if (ArgCnt==1)
      BEGIN
       strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"ST",255); ArgCnt++;
      END
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]); OpSize=(-1);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  if (AdrMode!=0)   /* ST(i) ist Ziel */
	   BEGIN
	    BAsmCode[CodeLen+1]=AdrMode;
	    DecodeAdr(ArgStr[2]);
	    switch (AdrType)
             BEGIN
	      case TypeFReg:
	       if (AdrMode!=0) WrError(1350);
	       else
	        BEGIN
	         BAsmCode[CodeLen]=0xdc; BAsmCode[CodeLen+1]+=0xe0+(OpAdd ^ 8);
	         CodeLen+=2;
	        END
               break;
              default:
               if (AdrType!=TypeNone) WrError(1350);
             END
	   END
	  else               /* ST ist Ziel */
	   BEGIN
	    DecodeAdr(ArgStr[2]);
	    switch (AdrType)
             BEGIN
	      case TypeFReg:
	       BAsmCode[CodeLen]=0xd8;
               BAsmCode[CodeLen+1]=0xe0+AdrMode+OpAdd;
	       CodeLen+=2;
	       break;
	      case TypeMem:
               if ((OpSize==-1) AND (UnknownFlag)) OpSize=2;
               if (OpSize==-1) WrError(1132);
	       else if ((OpSize!=2) AND (OpSize!=3)) WrError(1130);
	       else
	        BEGIN
                 BAsmCode[CodeLen]=(OpSize==2) ? 0xd8 : 0xdc;
	         BAsmCode[CodeLen+1]=AdrMode+0x20+OpAdd;
	         MoveAdr(2);
	         CodeLen+=AdrCnt+2;
	        END
               break;
              default:
               if (AdrType!=TypeNone) WrError(1350);
	     END
	   END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FISUB")) OR (FMemo("FISUBR")) OR (FMemo("FIDIV")) OR (FMemo("FIDIVR")))
    BEGIN
     OpAdd=0;
     if ((FMemo("FISUBR")) OR (Memo("FIDIVR"))) OpAdd+=8;
     if ((FMemo("FIDIV")) OR (Memo("FIDIVR"))) OpAdd+=16;
     if (ArgCnt==1)
      BEGIN
       ArgCnt=2; strcpy(ArgStr[2],ArgStr[1]); strmaxcpy(ArgStr[1],"ST",255);
      END
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  if (AdrMode!=0) WrError(1350);
	  else
	   BEGIN
	    OpSize=(-1);
	    DecodeAdr(ArgStr[2]);
	    switch (AdrType)
             BEGIN
	      case TypeMem:
               if ((OpSize==-1) AND (UnknownFlag)) OpSize=1;
               if (OpSize==-1) WrError(1132);
	       else if ((OpSize!=1) AND (OpSize!=2)) WrError(1130);
	       else
	        BEGIN
                 BAsmCode[CodeLen]=(OpSize==1) ? 0xde : 0xda;
	         BAsmCode[CodeLen+1]=AdrMode+0x20+OpAdd;
	         MoveAdr(2);
	         CodeLen+=2+AdrCnt;
	        END
               break;
              default:
               if (AdrType!=TypeNone) WrError(1350);
             END
           END
          break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   if ((FMemo("FSUBP")) OR (FMemo("FSUBRP")) OR (FMemo("FDIVP")) OR (FMemo("FDIVRP")))
    BEGIN
     OpAdd=0;
     if ((Memo("FSUBRP")) OR (Memo("FDIVRP"))) OpAdd+=8;
     if ((Memo("FDIVP")) OR (Memo("FDIVRP"))) OpAdd+=16;
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[2]);
       switch (AdrType)
        BEGIN
         case TypeFReg:
	  if (AdrMode!=0) WrError(1350);
	  else
	   BEGIN
	    DecodeAdr(ArgStr[1]);
	    switch (AdrType)
             BEGIN
	      case TypeFReg:
	       BAsmCode[CodeLen]=0xde;
	       BAsmCode[CodeLen+1]=0xe0+AdrMode+(OpAdd ^ 8);
	       CodeLen+=2;
	       break;
              default:
               if (AdrType!=TypeNone) WrError(1350);
             END
           END
	  break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   for (z=0; z<FPU16OrderCnt; z++)
    if (FMemo(FPU16Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
	OpSize=1;
	DecodeAdr(ArgStr[1]);
	switch (AdrType)
         BEGIN
	  case TypeMem:
	   PutCode(FPU16Orders[z].Code);
	   BAsmCode[CodeLen-1]+=AdrMode;
	   MoveAdr(0);
	   CodeLen+=AdrCnt;
	   break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
       END
      return True;
     END

   if ((FMemo("FSAVE")) OR (FMemo("FRSTOR")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       switch (AdrType)
        BEGIN
         case TypeMem:
	  BAsmCode[CodeLen]=0xdd; BAsmCode[CodeLen+1]=AdrMode+0x20;
	  if (Memo("FSAVE")) BAsmCode[CodeLen+1]+=0x10;
	  MoveAdr(2);
	  CodeLen+=2+AdrCnt;
	  break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     return True;
    END

   return False;
END

	static void MakeCode_86(void)
BEGIN
   Boolean OK;
   Word AdrWord;
   Byte AdrByte;
   int z,z2;

   CodeLen=0; DontPrint=False; OpSize=(-1); PrefixLen=0;
   NoSegCheck=False; UnknownFlag=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   /* vermischtes */

   if (LookupInstTable(InstTable,OpPart)) return;

   /* Koprozessor */

   if (DecodeFPU())
    BEGIN
     AddPrefixes(); return;
    END

   /* Stringoperationen */

   for (z=0; z<StringOrderCnt; z++)
    if (Memo(StringOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<StringOrders[z].MinCPU) WrError(1500);
      else PutCode(StringOrders[z].Code);
      AddPrefixes(); return;
     END

   /* mit Wiederholung */

   for (z=0; z<ReptOrderCnt; z++)
    if (Memo(ReptOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<ReptOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        for (z2=0; z2<StringOrderCnt; z2++)
         if (strcasecmp(StringOrders[z2].Name,ArgStr[1])==0) break;
        if (z2>=StringOrderCnt)WrError(1985);
        else if (MomCPU<StringOrders[z2].MinCPU) WrError(1500);
        else
         BEGIN
          PutCode(ReptOrders[z].Code); PutCode(StringOrders[z2].Code);
         END
       END
      AddPrefixes(); return;
     END

   for (z=0; z<MulOrderCnt; z++)
    if Memo(MulOrders[z])
     BEGIN
      switch (ArgCnt)
       BEGIN
        case 1:
	 DecodeAdr(ArgStr[1]);
	 switch (AdrType)
          BEGIN
	   case TypeReg8:
           case TypeReg16:
	    BAsmCode[CodeLen]=0xf6+OpSize;
	    BAsmCode[CodeLen+1]=0xe0+(z << 3)+AdrMode;
	    CodeLen+=2;
	    break;
	   case TypeMem:
	    MinOneIs0();
	    if (OpSize==-1) WrError(1132);
	    else
	     BEGIN
	      BAsmCode[CodeLen]=0xf6+OpSize;
	      BAsmCode[CodeLen+1]=0x20+(z << 3)+AdrMode;
	      MoveAdr(2);
	      CodeLen+=2+AdrCnt;
	     END
	    break;
	   default:
            if (AdrType!=TypeNone) WrError(1350);
	  END
	 break;
        case 2:
        case 3:
         if (MomCPU<CPU80186) WrError(1500);
	 else if (NOT Memo("IMUL")) WrError(1110);
	 else
	  BEGIN
	   if (ArgCnt==2)
	    BEGIN
	     strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]); ArgCnt++;
	    END
	   BAsmCode[CodeLen]=0x69;
	   DecodeAdr(ArgStr[1]);
	   switch (AdrType)
            BEGIN
	     case TypeReg16:
	      BAsmCode[CodeLen+1]=(AdrMode << 3);
	      DecodeAdr(ArgStr[2]);
	      if (AdrType==TypeReg16)
	       BEGIN
		AdrType=TypeMem; AdrMode+=0xc0;
	       END
	      switch (AdrType)
               BEGIN
	        case TypeMem:
		 BAsmCode[CodeLen+1]+=AdrMode;
		 MoveAdr(2);
		 AdrWord=EvalIntExpression(ArgStr[3],Int16,&OK);
		 if (OK)
		  BEGIN
		   BAsmCode[CodeLen+2+AdrCnt]=Lo(AdrWord);
		   BAsmCode[CodeLen+3+AdrCnt]=Hi(AdrWord);
		   CodeLen+=2+AdrCnt+2;
		   if ((AdrWord>=0xff80) OR (AdrWord<0x80))
		    BEGIN
		     CodeLen--;
		     BAsmCode[CodeLen-AdrCnt-2-1]+=2;
		    END
		  END
	         break;
                default:
                 if (AdrType!=TypeNone) WrError(1350);
               END
	      break;
             default:
              if (AdrType!=TypeNone) WrError(1350);
            END
	  END
         break;
        default: WrError(1110);
       END
      AddPrefixes(); return;
     END;

   for (z=0; z<ModRegOrderCnt; z++)
    if (Memo(ModRegOrders[z].Name))
     BEGIN
      NoSegCheck=Memo("LEA");
      if (ArgCnt!=2) WrError(1110);
      else if (MomCPU<ModRegOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
	DecodeAdr(ArgStr[1]);
	switch (AdrType)
         BEGIN
	  case TypeReg16:
	   OpSize=(Memo("LEA")) ? -1 : 2;
	   AdrByte=(AdrMode << 3);
	   DecodeAdr(ArgStr[2]);
	   switch (AdrType)
            BEGIN
	     case TypeMem:
	      PutCode(ModRegOrders[z].Code);
	      BAsmCode[CodeLen]=AdrByte+AdrMode;
	      MoveAdr(1);
	      CodeLen+=1+AdrCnt;
	      break;
             default:
              if (AdrType!=TypeNone) WrError(1350);
            END
	   break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
       END
      AddPrefixes(); return;
     END

   for (z=0; z<ShiftOrderCnt; z++)
    if (Memo(ShiftOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
	DecodeAdr(ArgStr[1]);
	MinOneIs0();
	if (OpSize==-1) WrError(1132);
        else switch (AdrType)
         BEGIN
          case TypeReg8:
          case TypeReg16:
          case TypeMem:
	   BAsmCode[CodeLen]=OpSize;
	   BAsmCode[CodeLen+1]=AdrMode+(ShiftOrders[z].Code << 3);
	   if (AdrType!=TypeMem) BAsmCode[CodeLen+1]+=0xc0;
	   MoveAdr(2);
	   if (strcasecmp(ArgStr[2],"CL")==0)
	    BEGIN
	     BAsmCode[CodeLen]+=0xd2;
	     CodeLen+=2+AdrCnt;
	    END
	   else
	    BEGIN
	     BAsmCode[CodeLen+2+AdrCnt]=EvalIntExpression(ArgStr[2],Int8,&OK);
	     if (OK)
	      if (BAsmCode[CodeLen+2+AdrCnt]==1)
	       BEGIN
		BAsmCode[CodeLen]+=0xd0;
		CodeLen+=2+AdrCnt;
	       END
	      else if (MomCPU<CPU80186) WrError(1500);
	      else
	       BEGIN
		BAsmCode[CodeLen]+=0xc0;
		CodeLen+=3+AdrCnt;
	       END
	    END
	   break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
       END
      AddPrefixes(); return;
     END

   if ((Memo("ROL4")) OR (Memo("ROR4")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPUV30) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       BAsmCode[CodeLen  ]=0x0f;
       BAsmCode[CodeLen+1]=(Memo("ROL4")) ? 0x28 : 0x2a;
       switch (AdrType)
        BEGIN
         case TypeReg8:
	  BAsmCode[CodeLen+2]=0xc0+AdrMode;
	  CodeLen+=3;
	  break;
         case TypeMem:
	  BAsmCode[CodeLen+2]=AdrMode;
	  MoveAdr(3);
	  CodeLen+=3+AdrCnt;
	  break;
         default:
          if (AdrType!=TypeNone) WrError(1350);
        END
      END
     AddPrefixes(); return;
    END

   for (z=0; z<Bit1OrderCnt; z++)
    if (Memo(Bit1Orders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (MomCPU<CPUV30) WrError(1500);
      else
       BEGIN
	DecodeAdr(ArgStr[1]);
	if ((AdrType==TypeReg8) OR (AdrType==TypeReg16))
	 BEGIN
	  AdrMode+=0xc0; AdrType=TypeMem;
	 END
	MinOneIs0();
	if (OpSize==-1) WrError(1132);
	else switch (AdrType)
         BEGIN
	  case TypeMem:
	   BAsmCode[CodeLen  ]=0x0f;
	   BAsmCode[CodeLen+1]=0x10+(z << 1)+OpSize;
	   BAsmCode[CodeLen+2]=AdrMode;
	   MoveAdr(3);
	   if (strcasecmp(ArgStr[2],"CL")==0) CodeLen+=3+AdrCnt;
	   else
	    BEGIN
	     BAsmCode[CodeLen+1]+=8;
	     BAsmCode[CodeLen+3+AdrCnt]=EvalIntExpression(ArgStr[2],Int4,&OK);
	     if (OK) CodeLen+=4+AdrCnt;
	    END
	   break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
       END
      AddPrefixes(); return;
     END

   if ((Memo("INS")) OR (Memo("EXT")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU<CPUV30) WrError(1500);
     else
      BEGIN
       DecodeAdr(ArgStr[1]);
       if (AdrType!=TypeNone)
        if (AdrType!=TypeReg8) WrError(1350);
        else
 	 BEGIN
 	  BAsmCode[CodeLen  ]=0x0f;
	  BAsmCode[CodeLen+1]=0x31;
	  if (Memo("EXT")) BAsmCode[CodeLen+1]+=2;
	  BAsmCode[CodeLen+2]=0xc0+AdrMode;
	  DecodeAdr(ArgStr[2]);
	  switch (AdrType)
           BEGIN
	    case TypeReg8:
	     BAsmCode[CodeLen+2]+=(AdrMode << 3);
	     CodeLen+=3;
	     break;
	    case TypeImm:
	     if (AdrVals[0]>15) WrError(1320);
	     else
	      BEGIN
	       BAsmCode[CodeLen+1]+=8;
	       BAsmCode[CodeLen+3]=AdrVals[1];
	       CodeLen+=4;
	      END
             break;
	    default:
             if (AdrType!=TypeNone) WrError(1350);
	   END
	 END
      END
     AddPrefixes(); return;
    END

   if (Memo("FPO2"))
    BEGIN
     if ((ArgCnt==0) OR (ArgCnt>2)) WrError(1110);
     else if (MomCPU<CPUV30) WrError(1500);
     else
      BEGIN
       AdrByte=EvalIntExpression(ArgStr[1],Int4,&OK);
       if (OK)
	BEGIN
	 BAsmCode[CodeLen  ]=0x66+(AdrByte >> 3);
	 BAsmCode[CodeLen+1]=(AdrByte & 7) << 3;
	 if (ArgCnt==1)
	  BEGIN
	   BAsmCode[CodeLen+1]+=0xc0;
	   CodeLen+=2;
	  END
	 else
	  BEGIN
	   DecodeAdr(ArgStr[2]);
	   switch (AdrType)
            BEGIN
	     case TypeReg8:
	      BAsmCode[CodeLen+1]+=0xc0+AdrMode;
	      CodeLen+=2;
	      break;
	     case TypeMem:
	      BAsmCode[CodeLen+1]+=AdrMode;
	      MoveAdr(2);
	      CodeLen+=2+AdrCnt;
	      break;
	     default:
              if (AdrType!=TypeNone) WrError(1350);
	    END
	  END
	END
      END
     AddPrefixes(); return;
    END

   if (Memo("BTCLR"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (MomCPU<CPUV35) WrError(1500);
     else
      BEGIN
       BAsmCode[CodeLen  ]=0x0f;
       BAsmCode[CodeLen+1]=0x9c;
       BAsmCode[CodeLen+2]=EvalIntExpression(ArgStr[1],Int8,&OK);
       if (OK)
	BEGIN
	 BAsmCode[CodeLen+3]=EvalIntExpression(ArgStr[2],UInt3,&OK);
	 if (OK)
	  BEGIN
	   AdrWord=EvalIntExpression(ArgStr[3],Int16,&OK)-(EProgCounter()+5);
	   if (OK)
	    if ((NOT SymbolQuestionable) AND ((AdrWord>0x7f) AND (AdrWord<0xff80))) WrError(1330);
	    else
	     BEGIN
	      BAsmCode[CodeLen+4]=Lo(AdrWord);
	      CodeLen+=5;
	     END
	  END
        END
      END
     AddPrefixes(); return;
    END

   for (z=0; z<Reg16OrderCnt; z++)
    if (Memo(Reg16Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (MomCPU<Reg16Orders[z].MinCPU) WrError(1500);
      else
       BEGIN
	DecodeAdr(ArgStr[1]);
	switch (AdrType)
         BEGIN
	  case TypeReg16:
	   PutCode(Reg16Orders[z].Code);
	   BAsmCode[CodeLen++]=Reg16Orders[z].Add+AdrMode;
	   break;
          default:
           if (AdrType!=TypeNone) WrError(1350);
         END
       END
      AddPrefixes(); return;
     END

   WrXError(1200,OpPart); return;
END

	static void InitCode_86(void)
BEGIN
   SaveInitProc();
   SegAssumes[0]=SegNone; /* ASSUME ES:NOTHING */
   SegAssumes[1]=SegCode; /* ASSUME CS:CODE */
   SegAssumes[2]=SegNone; /* ASSUME SS:NOTHING */
   SegAssumes[3]=SegData; /* ASSUME DS:DATA */
END

	static Boolean ChkPC_86(void)
BEGIN
   switch (ActPC) 
    BEGIN
     case SegCode:
     case SegData:
     case SegXData:
     case SegIO:
      return (ProgCounter()<0x10000);
     default:
      return False;
    END
END

	static Boolean IsDef_86(void)
BEGIN
   return (Memo("PORT"));
END

        static void SwitchFrom_86(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

	static void SwitchTo_86(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x42; NOPCode=0x90;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData)|(1<<SegXData)|(1<<SegIO);
   Grans[SegCode ]=1; ListGrans[SegCode ]=1; SegInits[SegCode ]=0;
   Grans[SegData ]=1; ListGrans[SegData ]=1; SegInits[SegData ]=0;
   Grans[SegXData]=1; ListGrans[SegXData]=1; SegInits[SegXData]=0;
   Grans[SegIO   ]=1; ListGrans[SegIO   ]=1; SegInits[SegIO   ]=0;

   MakeCode=MakeCode_86; ChkPC=ChkPC_86; IsDef=IsDef_86;
   SwitchFrom=SwitchFrom_86; InitFields();
   AddONOFF("FPU",&FPUAvail,FPUAvailName,False);
END

	void code86_init(void)
BEGIN
   CPU8086 =AddCPU("8086" ,SwitchTo_86);
   CPU80186=AddCPU("80186",SwitchTo_86);
   CPUV30  =AddCPU("V30"  ,SwitchTo_86);
   CPUV35  =AddCPU("V35"  ,SwitchTo_86);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_86;
END
