/* code96c141.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TLCS-900(L)                                                 */
/*                                                                           */
/* Historie: 27. 6.1996 Grundsteinlegung                                     */
/*            9. 1.1999 ChkPC jetzt mit Adresse als Parameter                */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           22. 5.2000 fixed decoding xhl0...xhl3                           */
/*           14. 6.2000 fixed coding of minc/mdec                            */
/*           18. 6.2000 fixed coding of bs1b                                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           19. 8.2001 fixed errors for lower halves of XIX...XSP           */
/*                                                                           */
/*****************************************************************************/
/* $Id: code96c141.c,v 1.8 2007/11/24 22:48:06 alfred Exp $                          */
/*****************************************************************************
 * $Log: code96c141.c,v $
 * Revision 1.8  2007/11/24 22:48:06  alfred
 * - some NetBSD changes
 *
 * Revision 1.7  2006/08/05 18:06:05  alfred
 * - remove debug printf
 *
 * Revision 1.6  2006/08/05 12:20:03  alfred
 * - regard spaces in indexed expressions for TLCS-900
 *
 * Revision 1.5  2005/09/08 16:53:42  alfred
 * - use common PInstTable
 *
 * Revision 1.4  2005/09/08 16:10:03  alfred
 * - fix Qxxn register decoding
 *
 * Revision 1.3  2004/11/16 17:43:09  alfred
 * - add missing hex mark on short mask
 *
 * Revision 1.2  2004/05/29 11:33:02  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:22  alfred
 * - recreated
 *
 * Revision 1.2  2002/10/20 09:22:25  alfred
 * - work around the parser problem related to the ' character
 *
 * Revision 1.7  2002/10/07 20:25:01  alfred
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "codepseudo.h"
#include "intpseudo.h"
#include "asmitree.h"
#include "codevars.h"

/*-------------------------------------------------------------------------*/
/* Daten */

typedef struct 
         {
          char *Name;
          Word Code;
          Byte CPUFlag;
          Boolean InSup;
         } FixedOrder;

typedef struct 
         {
          char *Name;
          Word Code;
          Boolean InSup;
          Byte MinMax,MaxMax;
          ShortInt Default;
         } ImmOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Byte OpMask;
         } RegOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } ALU2Order;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;

#define FixedOrderCnt 13
#define ImmOrderCnt 3 
#define RegOrderCnt 8
#define ALU2OrderCnt 8
#define ShiftOrderCnt 8
#define MulDivOrderCnt 4
#define BitCFOrderCnt 5
#define BitOrderCnt 5
#define ConditionCnt 24

#define ModNone (-1)
#define ModReg 0
#define MModReg (1  << ModReg)
#define ModXReg 1
#define MModXReg (1 << ModXReg)
#define ModMem 2
#define MModMem (1  << ModMem)
#define ModImm 3
#define MModImm (1  << ModImm)
#define ModCReg 4
#define MModCReg (1 << ModCReg)

static FixedOrder *FixedOrders;
static RegOrder *RegOrders;
static ImmOrder *ImmOrders;
static ALU2Order *ALU2Orders;
static char **ShiftOrders;
static char **MulDivOrders;
static ALU2Order *BitCFOrders;
static char **BitOrders;
static Condition *Conditions;
static LongInt DefaultCondition;

static ShortInt AdrType;
static ShortInt OpSize;        /* -1/0/1/2 = nix/Byte/Word/Long */
static Byte AdrMode;
static Byte AdrVals[10];
static Boolean MinOneIs0;

static CPUVar CPU96C141,CPU93C141;

/*---------------------------------------------------------------------------*/
/* Adressparser */

        static Boolean IsRegBase(Byte No, Byte Size)
BEGIN
   return ((Size==2) OR ((Size==1) AND (No<0xf0) AND (NOT Maximum) AND ((No & 3)==0)));
END

        static void ChkMaximum(Boolean MustMax, Byte *Result)
BEGIN
   if (Maximum!=MustMax)
    BEGIN
     *Result=1;
     WrError((MustMax)?1997:1996);
    END
END

        static Boolean IsQuot(char Ch)
BEGIN
   return ((Ch=='\'') OR (Ch=='`'));
END

        static Byte CodeEReg(char *Asc, Byte *ErgNo, Byte *ErgSize)
BEGIN
#define RegCnt 8
   static char Reg8Names[RegCnt+1]="AWCBEDLH";
   static char *Reg16Names[RegCnt]=
              {"WA" ,"BC" ,"DE" ,"HL" ,"IX" ,"IY" ,"IZ" ,"SP" };
   static char *Reg32Names[RegCnt]=
              {"XWA","XBC","XDE","XHL","XIX","XIY","XIZ","XSP"};

   int z,l=strlen(Asc);
   char *pos;
   String HAsc,Asc_N;
   Byte Result;

   strmaxcpy(Asc_N, Asc, 255); NLS_UpString(Asc_N); Asc = Asc_N;

   Result = 2;

   /* mom. Bank ? */

   if ((l == 1) AND ((pos = strchr(Reg8Names,*Asc)) != Nil))
    BEGIN
     z = pos - Reg8Names;
     *ErgNo = 0xe0 + ((z & 6) << 1)+(z & 1);
     *ErgSize = 0; return Result;
    END
   for (z = 0; z < RegCnt; z++)
    BEGIN
     if (strcmp(Asc,Reg16Names[z]) == 0)
      BEGIN
       *ErgNo = 0xe0 + (z << 2); *ErgSize = 1; return Result;
      END
     if (strcmp(Asc,Reg32Names[z]) == 0)
      BEGIN
       *ErgNo = 0xe0 + (z << 2); *ErgSize = 2;
       if (z < 4) ChkMaximum(True, &Result);
       return Result;
      END
    END

   /* Bankregister, 8 Bit ? */

   if ((l == 3) AND ((*Asc == 'Q') OR (*Asc == 'R')) AND ((Asc[2] >= '0') AND (Asc[2] <= '7')))
    for (z = 0; z < RegCnt; z++)
     if (Asc[1] == Reg8Names[z])
      BEGIN
       *ErgNo = ((Asc[2] - '0') << 4)+((z & 6) << 1) + (z & 1);
       if (*Asc == 'Q')
        BEGIN
         *ErgNo |= 2; ChkMaximum(True, &Result);
        END
       if (((*Asc == 'Q') OR (Maximum)) AND (Asc[2] > '3'))
        BEGIN
         WrError(1320); Result = 1;
        END
       *ErgSize = 0; return Result;
      END

   /* Bankregister, 16 Bit ? */

   if ((l == 4) AND ((*Asc == 'Q') OR (*Asc == 'R')) AND ((Asc[3] >= '0') AND (Asc[3] <= '7')))
    BEGIN
     strcpy(HAsc, Asc + 1); HAsc[2] = '\0';
     for (z = 0; z < RegCnt >> 1; z++)
      if (strcmp(HAsc, Reg16Names[z]) == 0)
       BEGIN
        *ErgNo = ((Asc[3] - '0') << 4) + (z << 2);
        if (*Asc == 'Q')
         BEGIN
          *ErgNo |= 2; ChkMaximum(True, &Result);
         END
        if (((*Asc == 'Q') OR (Maximum)) AND (Asc[3] > '3'))
         BEGIN
          WrError(1320); Result = 1;
         END
        *ErgSize = 1; return Result;
       END
    END

   /* Bankregister, 32 Bit ? */

   if ((l == 4) AND ((Asc[3] >= '0') AND (Asc[3] <= '7')))
    BEGIN
     for (z = 0; z < RegCnt >> 1; z++)
      if (strncmp(Asc, Reg32Names[z], 3) == 0)
       BEGIN
        *ErgNo = ((Asc[3] - '0') << 4) + (z << 2);
        ChkMaximum(True, &Result);
        if (Asc[3] > '3')
         BEGIN
          WrError(1320); Result = 1;
         END
        *ErgSize = 2; return Result;
       END
    END

   /* obere 8-Bit-Haelften momentaner Bank ? */

   if ((l == 2) AND (*Asc == 'Q'))
    for (z = 0; z < RegCnt; z++)
     if (Asc[1] == Reg8Names[z])
      BEGIN
       *ErgNo = 0xe2 + ((z & 6) << 1) + (z & 1);
       ChkMaximum(True,&Result);
       *ErgSize = 0; return Result;
      END

   /* obere 16-Bit-Haelften momentaner Bank und von XIX..XSP ? */

   if ((l == 3) AND (*Asc == 'Q'))
    BEGIN
     for (z = 0; z < RegCnt; z++)
      if (strcmp(Asc + 1, Reg16Names[z]) == 0)
       BEGIN
        *ErgNo = 0xe2 + (z << 2);
        if (z < 4) ChkMaximum(True, &Result);
        *ErgSize = 1; return Result;
       END
    END

   /* 8-Bit-Teile von XIX..XSP ? */

   if (((l == 3) OR ((l == 4) AND (*Asc == 'Q')))
   AND ((Asc[l - 1] == 'L') OR (Asc[l - 1] == 'H')))
    BEGIN
     strcpy(HAsc, Asc + (l - 3)); HAsc[2] = '\0';
     for (z = 0; z < RegCnt >> 1; z++)
      if (strcmp(HAsc, Reg16Names[z + 4]) == 0)
       BEGIN
        *ErgNo = 0xf0 + (z << 2) + ((l - 3) << 1)+(Ord(Asc[l - 1] == 'H'));
        *ErgSize = 0; return Result;
       END
    END

   /* 8-Bit-Teile vorheriger Bank ? */

   if (((l == 2) OR ((l == 3) AND (*Asc == 'Q'))) AND (IsQuot(Asc[l - 1])))
    for (z = 0; z < RegCnt; z++)
     if (Asc[l - 2] == Reg8Names[z])
      BEGIN
       *ErgNo = 0xd0 + ((z & 6) << 1) + ((strlen(Asc) - 2) << 1) + (z & 1);
       if (l == 3) ChkMaximum(True, &Result);
       *ErgSize = 0; return Result;
      END;

   /* 16-Bit-Teile vorheriger Bank ? */

   if (((l == 3) OR ((l == 4) AND (*Asc == 'Q'))) AND (IsQuot(Asc[l - 1])))
    BEGIN
     strcpy(HAsc, Asc + 1); HAsc[l - 2] = '\0';
     for (z = 0; z < RegCnt >> 1; z++)
      if (strcmp(HAsc, Reg16Names[z]) == 0)
       BEGIN
        *ErgNo = 0xd0 + (z << 2) + ((strlen(Asc) - 3) << 1);
        if (l == 4) ChkMaximum(True, &Result);
        *ErgSize = 1; return Result;
       END
    END

   /* 32-Bit-Register vorheriger Bank ? */

   if ((l == 4) AND (IsQuot(Asc[3])))
    BEGIN
     strcpy(HAsc, Asc); HAsc[3] = '\0';
     for (z = 0; z < RegCnt >> 1; z++)
      if (strcmp(HAsc, Reg32Names[z]) == 0)
       BEGIN
        *ErgNo = 0xd0 + (z << 2);
        ChkMaximum(True, &Result);
        *ErgSize = 2; return Result;
       END
    END

   return (Result = 0);
END

        static void ChkL(CPUVar Must, Byte *Result)
BEGIN
   if (MomCPU!=Must)
    BEGIN
     WrError(1440); *Result=0;
    END
END

        static Byte CodeCReg(char *Asc, Byte *ErgNo, Byte *ErgSize)
BEGIN
   Byte Result=2;

   if (strcasecmp(Asc,"NSP")==0)
    BEGIN
     *ErgNo=0x3c; *ErgSize=1;
     ChkL(CPU96C141,&Result);
     return Result;
    END
   if (strcasecmp(Asc,"XNSP")==0)
    BEGIN
     *ErgNo=0x3c; *ErgSize=2;
     ChkL(CPU96C141,&Result);
     return Result;
    END
   if (strcasecmp(Asc,"INTNEST")==0)
    BEGIN
     *ErgNo=0x3c; *ErgSize=1;
     ChkL(CPU93C141,&Result);
     return Result;
    END
   if ((strlen(Asc)==5) AND (strncasecmp(Asc,"DMA",3)==0) AND (Asc[4]>='0') AND (Asc[4]<='3'))
   switch (mytoupper(Asc[3]))
    BEGIN
     case 'S':
      *ErgNo=(Asc[4]-'0')*4; *ErgSize=2; return Result;
     case 'D':
      *ErgNo=(Asc[4]-'0')*4+0x10; *ErgSize=2; return Result;
     case 'M':
      *ErgNo=(Asc[4]-'0')*4+0x22; *ErgSize=0; return Result;
     case 'C':
      *ErgNo=(Asc[4]-'0')*4+0x20; *ErgSize=1; return Result;
    END

   return (Result=0);
END


typedef struct
         {
          char *Name;
          Byte Num;
          Boolean InMax,InMin;
         } RegDesc;


        static void SetOpSize(ShortInt NewSize)
BEGIN
   if (OpSize==-1) OpSize=NewSize;
   else if (OpSize!=NewSize)
    BEGIN
     WrError(1131); AdrType=ModNone;
    END
END

        static Boolean IsRegCurrent(Byte No, Byte Size, Byte *Erg)
BEGIN
   switch (Size)
    BEGIN
     case 0:
      if ((No & 0xf2)==0xe0)
       BEGIN
        *Erg=((No & 0x0c) >> 1)+((No & 1) ^ 1);
        return True;
       END
      else return False;
     case 1:
     case 2:
      if ((No & 0xe3)==0xe0)
       BEGIN
        *Erg=((No & 0x1c) >> 2);
        return True;
       END
      else return False;
     default:
      return False;
    END
END

        static void ChkAdr(Byte Erl)
BEGIN
   if (AdrType!=ModNone)
    if ((Erl&(1 << AdrType))==0)
     BEGIN
      WrError(1350); AdrType=ModNone;
     END
END

        static void DecodeAdr(char *Asc, Byte Erl)
BEGIN
   String HAsc,Rest;
   Byte HNum,HSize;
   Boolean OK,NegFlag,NNegFlag,MustInd,FirstFlag;
   Byte BaseReg,BaseSize;
   Byte IndReg,IndSize;
   Byte PartMask;
   LongInt DispPart,DispAcc;
   char *MPos,*PPos,*EPos;

   AdrType=ModNone;

   /* Register ? */

   switch (CodeEReg(Asc,&HNum,&HSize))
    BEGIN
     case 1:
      ChkAdr(Erl); return;
     case 2:
      if (IsRegCurrent(HNum,HSize,&AdrMode)) AdrType=ModReg;
      else
       BEGIN
        AdrType=ModXReg; AdrMode=HNum;
       END
      SetOpSize(HSize);
      ChkAdr(Erl); return;
    END

   /* Steuerregister ? */

   if (CodeCReg(Asc,&HNum,&HSize)==2)
    BEGIN
     AdrType=ModCReg; AdrMode=HNum;
     SetOpSize(HSize);
     ChkAdr(Erl); return;
    END

   /* Predekrement ? */

   if ((strlen(Asc)>4) AND (Asc[strlen(Asc)-1]==')') AND (strncmp(Asc,"(-",2)==0))
    BEGIN
     strcpy(HAsc,Asc+2); HAsc[strlen(HAsc)-1]='\0';
     if (CodeEReg(HAsc,&HNum,&HSize)!=2) WrError(1350);
     else if (NOT IsRegBase(HNum,HSize)) WrError(1350);
     else
      BEGIN
       AdrType=ModMem; AdrMode=0x44;
       AdrCnt=1; AdrVals[0]=HNum; if (OpSize!=-1) AdrVals[0]+=OpSize;
      END
     ChkAdr(Erl); return;
    END

   /* Postinkrement ? */

   if ((strlen(Asc)>4) AND (Asc[0]=='(') AND (strncmp(Asc+strlen(Asc)-2,"+)",2)==0))
    BEGIN
     strcpy(HAsc,Asc+1); HAsc[strlen(HAsc)-2]='\0';
     if (CodeEReg(HAsc,&HNum,&HSize)!=2) WrError(1350);
     else if (NOT IsRegBase(HNum,HSize)) WrError(1350);
     else
      BEGIN
       AdrType=ModMem; AdrMode=0x45;
       AdrCnt=1; AdrVals[0]=HNum; if (OpSize!=-1) AdrVals[0]+=OpSize;
      END
     ChkAdr(Erl); return;
    END

   /* Speicheroperand ? */

   if (IsIndirect(Asc))
    BEGIN
     NegFlag=False; NNegFlag=False; FirstFlag=False;
     PartMask=0; DispAcc=0; BaseReg=IndReg=BaseSize=IndSize=0xff;
     strcpy(Rest,Asc+1); Rest[strlen(Rest)-1]='\0';

     do
      BEGIN
       MPos=QuotPos(Rest,'-'); PPos=QuotPos(Rest,'+');
       if ((PPos!=Nil) AND ((MPos==Nil) OR (PPos<MPos)))
        BEGIN
         EPos=PPos; NNegFlag=False;
        END
       else if ((MPos!=Nil) AND ((PPos==Nil) OR (MPos<PPos)))
        BEGIN
         EPos=MPos; NNegFlag=True;
        END
       else EPos=Rest+strlen(Rest);
       if ((EPos==Rest) OR (EPos==Rest+strlen(Rest)-1))
        BEGIN
         WrError(1350); return;
        END
       strncpy(HAsc,Rest,EPos-Rest); HAsc[EPos-Rest]='\0';
       if (EPos<Rest+strlen(Rest)) strcpy(Rest,EPos+1); else *Rest='\0';
       KillPrefBlanks(HAsc); KillPostBlanks(HAsc);

       switch (CodeEReg(HAsc,&HNum,&HSize))
        BEGIN
         case 0:
          FirstPassUnknown=False;
          DispPart=EvalIntExpression(HAsc,Int32,&OK);
          if (FirstPassUnknown) FirstFlag=True;
          if (NOT OK) return;
          if (NegFlag) DispAcc-=DispPart; else DispAcc+=DispPart;
          PartMask|=1;
          break;
         case 1:
          break;
         case 2:
          if (NegFlag)
           BEGIN
            WrError(1350); return;
           END
          else
           BEGIN
            if (HSize==0) MustInd=True;
            else if (HSize==2) MustInd=False;
            else if (NOT IsRegBase(HNum,HSize)) MustInd=True;
            else if ((PartMask & 4)!=0) MustInd=True;
            else MustInd=False;
            if (MustInd)
             if ((PartMask & 2)!=0)
              BEGIN
               WrError(1350); return;
              END
             else
              BEGIN
               IndReg=HNum; PartMask|=2;
               IndSize=HSize;
              END
            else
             if ((PartMask & 4)!=0)
              BEGIN
               WrError(1350); return;
              END
             else
              BEGIN
               BaseReg=HNum; PartMask|=4;
               BaseSize=HSize;
              END
           END
          break;
        END

       NegFlag=NNegFlag; NNegFlag=False;
      END
     while (*Rest!='\0');

     if ((DispAcc==0) AND (PartMask!=1)) PartMask&=6;
     if ((PartMask==5) AND (FirstFlag)) DispAcc&=0x7fff;

     switch (PartMask)
      BEGIN
       case 0:
       case 2:
       case 3:
       case 7:
        WrError(1350);
        break;
       case 1:
        if (DispAcc<=0xff)
         BEGIN
          AdrType=ModMem; AdrMode=0x40; AdrCnt=1;
          AdrVals[0]=DispAcc;
         END
        else if (DispAcc<0xffff)
         BEGIN
          AdrType=ModMem; AdrMode=0x41; AdrCnt=2;
          AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
         END
        else if (DispAcc<0xffffff)
         BEGIN
          AdrType=ModMem; AdrMode=0x42; AdrCnt=3;
          AdrVals[0]=DispAcc         & 0xff;
          AdrVals[1]=(DispAcc >>  8) & 0xff;
          AdrVals[2]=(DispAcc >> 16) & 0xff;
         END
        else WrError(1925);
        break;
       case 4:
        if (IsRegCurrent(BaseReg,BaseSize,&AdrMode))
         BEGIN
          AdrType=ModMem; AdrCnt=0;
         END
        else
         BEGIN
          AdrType=ModMem; AdrMode=0x43; AdrCnt=1;
          AdrVals[0]=BaseReg;
         END
        break;
       case 5:
        if ((DispAcc<=127) AND (DispAcc>=-128) AND (IsRegCurrent(BaseReg,BaseSize,&AdrMode)))
         BEGIN
          AdrType=ModMem; AdrMode+=8; AdrCnt=1;
          AdrVals[0]=DispAcc & 0xff;
         END
        else if ((DispAcc<=32767) AND (DispAcc>=-32768))
         BEGIN
          AdrType=ModMem; AdrMode=0x43; AdrCnt=3;
          AdrVals[0]=BaseReg+1;
          AdrVals[1]=DispAcc & 0xff;
          AdrVals[2]=(DispAcc >> 8) & 0xff;
         END
        else WrError(1320);
        break;
       case 6:
        AdrType=ModMem; AdrMode=0x43; AdrCnt=3;
        AdrVals[0]=3+(IndSize << 2);
        AdrVals[1]=BaseReg;
        AdrVals[2]=IndReg;
        break;
      END
     ChkAdr(Erl); return;
    END

   /* bleibt nur noch immediate... */

   if ((MinOneIs0) AND (OpSize==-1)) OpSize=0;
   switch (OpSize)
    BEGIN
     case -1:
      WrError(1132); 
      break;
     case 0:
      AdrVals[0]=EvalIntExpression(Asc,Int8,&OK);
      if (OK)
       BEGIN
        AdrType=ModImm; AdrCnt=1;
       END
      break;
     case 1:
      DispAcc=EvalIntExpression(Asc,Int16,&OK);
      if (OK)
       BEGIN
        AdrType=ModImm; AdrCnt=2;
        AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
       END
      break;
     case 2:
      DispAcc=EvalIntExpression(Asc,Int32,&OK);
      if (OK)
       BEGIN
        AdrType=ModImm; AdrCnt=4;
        AdrVals[0]=Lo(DispAcc); AdrVals[1]=Hi(DispAcc);
        AdrVals[2]=Lo(DispAcc >> 16); AdrVals[3]=Hi(DispAcc >> 16);
       END
      break;
    END
END

/*---------------------------------------------------------------------------*/

        static void CorrMode(Byte Ref, Byte Adr)
BEGIN
   if ((BAsmCode[Ref] & 0x4e)==0x44)
    BAsmCode[Adr]=(BAsmCode[Adr] & 0xfc) | OpSize;
END

        static Boolean ArgPair(const char *Val1, const char *Val2)
BEGIN
   return  (((strcasecmp(ArgStr[1],Val1)==0) AND (strcasecmp(ArgStr[2],Val2)==0)) OR
            ((strcasecmp(ArgStr[1],Val2)==0) AND (strcasecmp(ArgStr[2],Val1)==0)));
END

        static LongInt ImmVal(void)
BEGIN
   LongInt tmp;

   tmp=AdrVals[0];
   if (OpSize>=1) tmp+=((LongInt)AdrVals[1]) << 8;
   if (OpSize==2)
    BEGIN
     tmp+=((LongInt)AdrVals[2]) << 16;
     tmp+=((LongInt)AdrVals[3]) << 24;
    END
   return tmp;
END

        static Boolean IsPwr2(LongInt Inp, Byte *Erg)
BEGIN
   LongInt Shift;

   Shift=1; *Erg=0;
   do
    BEGIN
     if (Inp==Shift) return True;
     Shift+=Shift; (*Erg)++;
    END
   while (Shift!=0);
   return False;
END

        static Boolean IsShort(Byte Code)
BEGIN
   return ((Code & 0x4e)==0x40);
END

        static void CheckSup(void)
BEGIN
   if (MomCPU==CPU96C141)
    if (NOT SupAllowed) WrError(50);
END

/*---------------------------------------------------------------------------*/

        static void DecodeMULA(Word Index)
BEGIN
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],MModReg+MModXReg);
     if ((AdrType!=ModNone) AND (OpSize!=2)) WrError(1130);
     else 
      switch (AdrType)
       BEGIN
        case ModReg:
         CodeLen=2;
         BAsmCode[0]=0xd8+AdrMode;
         BAsmCode[1]=0x19;
         break;
        case ModXReg:
         CodeLen=3;
         BAsmCode[0]=0xd7;
         BAsmCode[1]=AdrMode;
         BAsmCode[2]=0x19;
         break;
       END
    END
END

        static void DecodeJPCALL(Word Index)
BEGIN
   int z;

   if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
   else
    BEGIN
     if (ArgCnt==1)  z=DefaultCondition;
     else
      BEGIN
       z=0; NLS_UpString(ArgStr[1]);
       while ((z<ConditionCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
      END
     if (z>=ConditionCnt) WrError(1360);
     else
      BEGIN
       OpSize=2;
       DecodeAdr(ArgStr[ArgCnt],MModMem+MModImm);
       if (AdrType==ModImm) 
        BEGIN
         if (AdrVals[3]!=0) 
          BEGIN
           WrError(1320); AdrType=ModNone;
          END
         else if (AdrVals[2]!=0) 
          BEGIN
           AdrType=ModMem; AdrMode=0x42; AdrCnt=3;
          END
         else
          BEGIN
           AdrType=ModMem; AdrMode=0x41; AdrCnt=2;
          END
        END
       if (AdrType==ModMem) 
        BEGIN
         if ((z==DefaultCondition) AND ((AdrMode==0x41) OR (AdrMode==0x42))) 
          BEGIN
           CodeLen=1+AdrCnt;
           BAsmCode[0]=0x1a+2*Index+(AdrCnt-2);
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
          END
         else
          BEGIN
           CodeLen=2+AdrCnt;
           BAsmCode[0]=0xb0+AdrMode;
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
           BAsmCode[1+AdrCnt]=0xd0+(Index << 4)+(Conditions[z].Code);
          END
        END
      END
    END
END

        static void DecodeJR(Word Index)
BEGIN
   Boolean OK;
   int z;
   LongInt AdrLong;

   if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
   else
    BEGIN
     if (ArgCnt==1) z=DefaultCondition;
     else
      BEGIN
       z=0; NLS_UpString(ArgStr[1]);
       while ((z<ConditionCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
      END
     if (z>=ConditionCnt) WrError(1360);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[ArgCnt],Int32,&OK);
       if (OK) 
        BEGIN
         if (Index==1) 
          BEGIN
           AdrLong-=EProgCounter()+3;
           if (((AdrLong>0x7fffl) OR (AdrLong<-0x8000l)) AND (NOT SymbolQuestionable)) WrError(1330);
           else
            BEGIN
             CodeLen=3; BAsmCode[0]=0x70+Conditions[z].Code;
             BAsmCode[1]=Lo(AdrLong); BAsmCode[2]=Hi(AdrLong);
             if (NOT FirstPassUnknown) 
              BEGIN
               AdrLong++;
               if ((AdrLong>=-128) AND (AdrLong<=127)) WrError(20);
              END
            END
          END
         else
          BEGIN
           AdrLong-=EProgCounter()+2;
           if (((AdrLong>127) OR (AdrLong<-128)) AND (NOT SymbolQuestionable)) WrError(1330);
           else
            BEGIN
             CodeLen=2; BAsmCode[0]=0x60+Conditions[z].Code;
             BAsmCode[1]=Lo(AdrLong);
            END
          END
        END
      END
    END
END

        static void DecodeCALR(Word Index)
BEGIN
   LongInt AdrLong;
   Boolean OK;
   UNUSED(Index);
   
   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK)-(EProgCounter()+3);
     if (OK) 
      BEGIN
       if (((AdrLong<-32768) OR (AdrLong>32767)) AND (NOT SymbolQuestionable)) WrError(1330);
       else
        BEGIN
         CodeLen=3; BAsmCode[0]=0x1e;
         BAsmCode[1]=Lo(AdrLong); BAsmCode[2]=Hi(AdrLong);
        END
      END
    END
END

        static void DecodeRET(Word Index)
BEGIN
    int z;
    UNUSED(Index);

    if (ArgCnt>1) WrError(1110);
    else
     BEGIN
      if (ArgCnt==0) z=DefaultCondition;
      else
       BEGIN
        z=0; NLS_UpString(ArgStr[1]);
        while ((z<ConditionCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
       END
      if (z>=ConditionCnt) WrError(1360);
      else if (z==DefaultCondition) 
       BEGIN
        CodeLen=1; BAsmCode[0]=0x0e;
       END
      else
       BEGIN
        CodeLen=2; BAsmCode[0]=0xb0;
        BAsmCode[1]=0xf0+Conditions[z].Code;
       END
     END
END

        static void DecodeRETD(Word Index)
BEGIN
   Word AdrWord;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     AdrWord=EvalIntExpression(ArgStr[1],Int16,&OK);
     if (OK) 
      BEGIN
       CodeLen=3; BAsmCode[0]=0x0f;
       BAsmCode[1]=Lo(AdrWord); BAsmCode[2]=Hi(AdrWord);
      END
    END
END

        static void DecodeDJNZ(Word Index)
BEGIN
   LongInt AdrLong;
   Boolean OK; 
   UNUSED(Index);

   if ((ArgCnt!=2) AND (ArgCnt!=1)) WrError(1110);
   else
    BEGIN
     if (ArgCnt==1) 
      BEGIN
       AdrType=ModReg; AdrMode=2; OpSize=0;
      END
     else DecodeAdr(ArgStr[1],MModReg+MModXReg);
     if (AdrType!=ModNone) 
      BEGIN
       if (OpSize==2) WrError(1130);
       else
        BEGIN
         AdrLong=EvalIntExpression(ArgStr[ArgCnt],Int32,&OK)-(EProgCounter()+3+Ord(AdrType==ModXReg));
         if (OK) 
          BEGIN
           if (((AdrLong<-128) OR (AdrLong>127)) AND (NOT SymbolQuestionable)) WrError(1370);
           else 
            switch (AdrType)
             BEGIN
              case ModReg:
                CodeLen=3;
               BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
               BAsmCode[1]=0x1c;
               BAsmCode[2]=AdrLong & 0xff;
               break;
              case ModXReg:
               CodeLen=4;
               BAsmCode[0]=0xc7+(OpSize << 4);
               BAsmCode[1]=AdrMode;
               BAsmCode[2]=0x1c;
               BAsmCode[3]=AdrLong & 0xff;
               break;
             END
          END
        END
      END
    END
END

        static void DecodeEX(Word Index)
BEGIN
   Byte HReg;
   UNUSED(Index);

   /* work around the parser problem related to the ' character */

   if (!strncasecmp(ArgStr[2], "F\'", 2))
     ArgStr[2][2] = '\0';

   if (ArgCnt!=2) WrError(1110);
   else if ((ArgPair("F","F\'")) OR (ArgPair("F`","F"))) 
    BEGIN
     CodeLen=1; BAsmCode[0]=0x16;
    END
   else
    BEGIN
     DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
     if (OpSize==2) WrError(1130);
     else 
      switch (AdrType)
       BEGIN
        case ModReg:
         HReg=AdrMode;
         DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
         switch (AdrType)
          BEGIN
           case ModReg:
            CodeLen=2;
            BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
            BAsmCode[1]=0xb8+HReg;
            break;
           case ModXReg:
            CodeLen=3;
            BAsmCode[0]=0xc7+(OpSize << 4);
            BAsmCode[1]=AdrMode;
            BAsmCode[2]=0xb8+HReg;
            break;
           case ModMem:
            CodeLen=2+AdrCnt;
            BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
            memcpy(BAsmCode+1,AdrVals,AdrCnt);
            BAsmCode[1+AdrCnt]=0x30+HReg;
            break;
          END
         break;
        case ModXReg:
         HReg=AdrMode;
         DecodeAdr(ArgStr[2],MModReg);
         if (AdrType==ModReg) 
          BEGIN
           CodeLen=3;
           BAsmCode[0]=0xc7+(OpSize << 4);
           BAsmCode[1]=HReg;
           BAsmCode[2]=0xb8+AdrMode;
          END
         break;
        case ModMem:
         MinOneIs0=True;
         HReg=AdrCnt; BAsmCode[0]=AdrMode;
         memcpy(BAsmCode+1,AdrVals,AdrCnt);
         DecodeAdr(ArgStr[2],MModReg);
         if (AdrType==ModReg) 
          BEGIN
           CodeLen=2+HReg; CorrMode(0,1);
           BAsmCode[0]+=0x80+(OpSize << 4);
           BAsmCode[1+HReg]=0x30+AdrMode;
          END
         break;
       END
    END
END

        static void DecodeBS1x(Word Index)
BEGIN
   if (ArgCnt!=2) WrError(1110);
   else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1135);
   else
    BEGIN
     DecodeAdr(ArgStr[2],MModReg+MModXReg);
     if (OpSize!=1) WrError(1130);
     else
      switch (AdrType)
       BEGIN
        case ModReg:
         CodeLen=2;
         BAsmCode[0]=0xd8+AdrMode;
         BAsmCode[1]=0x0e +Index; /* ANSI */
         break;
        case ModXReg:
         CodeLen=3;
         BAsmCode[0]=0xd7;
         BAsmCode[1]=AdrMode;
         BAsmCode[2]=0x0e +Index; /* ANSI */
         break;
       END
    END
END

        static void DecodeLDA(Word Index)
BEGIN
   Byte HReg;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],MModReg);
     if (AdrType!=ModNone) 
      BEGIN
       if (OpSize<1) WrError(1130);
       else
        BEGIN
         HReg=AdrMode;
         DecodeAdr(ArgStr[2],MModMem);
         if (AdrType!=ModNone) 
          BEGIN
           CodeLen=2+AdrCnt;
           BAsmCode[0]=0xb0+AdrMode;
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
           BAsmCode[1+AdrCnt]=0x20+((OpSize-1) << 4)+HReg;
          END
        END
      END
    END
END

        static void DecodeLDAR(Word Index)
BEGIN
   LongInt AdrLong;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     AdrLong=EvalIntExpression(ArgStr[2],Int32,&OK)-(EProgCounter()+4);
     if (OK) 
      BEGIN
       if (((AdrLong<-32768) OR (AdrLong>32767)) AND (NOT SymbolQuestionable)) WrError(1330);
       else
        BEGIN
         DecodeAdr(ArgStr[1],MModReg);
         if (AdrType!=ModNone) 
          BEGIN
           if (OpSize<1) WrError(1130);
           else
            BEGIN
             CodeLen=5;
             BAsmCode[0]=0xf3; BAsmCode[1]=0x13;
             BAsmCode[2]=Lo(AdrLong); BAsmCode[3]=Hi(AdrLong);
             BAsmCode[4]=0x20+((OpSize-1) << 4)+AdrMode;
            END
          END
        END
      END
    END
END

        static void DecodeLDC(Word Index)
BEGIN
   Byte HReg;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],MModReg+MModXReg+MModCReg);
     HReg=AdrMode;
     switch (AdrType)
      BEGIN
       case ModReg:
        DecodeAdr(ArgStr[2],MModCReg);
        if (AdrType!=ModNone) 
         BEGIN
          CodeLen=3;
          BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
          BAsmCode[1]=0x2f;
          BAsmCode[2]=AdrMode;
         END
        break;
       case ModXReg:
        DecodeAdr(ArgStr[2],MModCReg);
        if (AdrType!=ModNone) 
         BEGIN
          CodeLen=4;
          BAsmCode[0]=0xc7+(OpSize << 4);
          BAsmCode[1]=HReg;
          BAsmCode[2]=0x2f;
          BAsmCode[3]=AdrMode;
         END;
        break;
       case ModCReg:
        DecodeAdr(ArgStr[2],MModReg+MModXReg);
        switch (AdrType)
         BEGIN
          case ModReg:
           CodeLen=3;
           BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
           BAsmCode[1]=0x2e;
           BAsmCode[2]=HReg;
           break;
          case ModXReg:
           CodeLen=4;
           BAsmCode[0]=0xc7+(OpSize << 4);
           BAsmCode[1]=AdrMode;
           BAsmCode[2]=0x2e;
           BAsmCode[3]=HReg;
           break;
         END
        break;
      END
    END
END

        static void DecodeLDX(Word Index)
BEGIN
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],MModMem);
     if (AdrType!=ModNone) 
      BEGIN
       if (AdrMode!=0x40) WrError(1350);
       else
        BEGIN
         BAsmCode[4]=EvalIntExpression(ArgStr[2],Int8,&OK);
         if (OK) 
          BEGIN
           CodeLen=6;
           BAsmCode[0]=0xf7; BAsmCode[1]=0;
           BAsmCode[2]=AdrVals[0]; BAsmCode[3]=0;
           BAsmCode[5]=0;
          END
        END
      END
    END
END

        static void DecodeLINK(Word Index)
BEGIN
   Word AdrWord;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     AdrWord=EvalIntExpression(ArgStr[2],Int16,&OK);
     if (OK) 
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModXReg);
       if ((AdrType!=ModNone) AND (OpSize!=2)) WrError(1130);
       else 
        switch (AdrType)
         BEGIN
          case ModReg:
           CodeLen=4;
           BAsmCode[0]=0xe8+AdrMode;
           BAsmCode[1]=0x0c;
           BAsmCode[2]=Lo(AdrWord);
           BAsmCode[3]=Hi(AdrWord);
           break;
          case ModXReg:
           CodeLen=5;
           BAsmCode[0]=0xe7;
           BAsmCode[1]=AdrMode;
           BAsmCode[2]=0x0c;
           BAsmCode[3]=Lo(AdrWord);
           BAsmCode[4]=Hi(AdrWord);
           break;
         END
      END
    END
END

        static void DecodeLD(Word Index)
BEGIN
   Byte HReg;
   Boolean ShDest,ShSrc,OK;

   if (Index<3) OpSize=Index;

   if (ArgCnt!=2) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
     switch (AdrType)
      BEGIN
       case ModReg:
        HReg=AdrMode;
        DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
        switch (AdrType)
         BEGIN
          case ModReg:
           CodeLen=2;
           BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
           BAsmCode[1]=0x88+HReg;
           break;
          case ModXReg:
           CodeLen=3;
           BAsmCode[0]=0xc7+(OpSize << 4);
           BAsmCode[1]=AdrMode;
           BAsmCode[2]=0x88+HReg;
           break;
          case ModMem:
           CodeLen=2+AdrCnt;
           BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
           BAsmCode[1+AdrCnt]=0x20+HReg;
           break;
          case ModImm:
           if ((ImmVal()<=7) AND (ImmVal()>=0))
            BEGIN
             CodeLen=2;
             BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
             BAsmCode[1]=0xa8+AdrVals[0];
            END
           else
            BEGIN
             CodeLen=1+AdrCnt;
             BAsmCode[0]=((OpSize+2) << 4)+HReg;
             memcpy(BAsmCode+1,AdrVals,AdrCnt);
            END
           break;
         END
        break;
       case ModXReg:
        HReg=AdrMode;
        DecodeAdr(ArgStr[2],MModReg+MModImm);
        switch (AdrType)
         BEGIN
          case ModReg:
           CodeLen=3;
           BAsmCode[0]=0xc7+(OpSize << 4);
           BAsmCode[1]=HReg;
           BAsmCode[2]=0x98+AdrMode;
           break;
          case ModImm:
           if ((ImmVal()<=7) AND (ImmVal()>=0))
            BEGIN
             CodeLen=3;
             BAsmCode[0]=0xc7+(OpSize << 4);
             BAsmCode[1]=HReg;
             BAsmCode[2]=0xa8+AdrVals[0];
            END
           else
            BEGIN
             CodeLen=3+AdrCnt;
             BAsmCode[0]=0xc7+(OpSize << 4);
             BAsmCode[1]=HReg;
             BAsmCode[2]=3;
             memcpy(BAsmCode+3,AdrVals,AdrCnt);
            END
           break; 
         END
        break;
       case ModMem:
        BAsmCode[0]=AdrMode;
        HReg=AdrCnt; MinOneIs0=True;
        memcpy(BAsmCode+1,AdrVals,AdrCnt);
        DecodeAdr(ArgStr[2],MModReg+MModMem+MModImm);
        switch (AdrType)
         BEGIN
          case ModReg:
           CodeLen=2+HReg;
           BAsmCode[0]+=0xb0; CorrMode(0,1);
           BAsmCode[1+HReg]=0x40+(OpSize << 4)+AdrMode;
           break;
          case ModMem:
           if (OpSize==-1) OpSize=0;
           ShDest=IsShort(BAsmCode[0]); ShSrc=IsShort(AdrMode);
           if (NOT (ShDest OR ShSrc)) WrError(1350);
           else
            BEGIN
             if ((ShDest AND (NOT ShSrc))) OK=True;
             else if (ShSrc AND (NOT ShDest)) OK=False;
             else if (AdrMode==0x40) OK=True;
             else OK=False;
             if (OK)   /* dest=(dir8/16) */
              BEGIN
               CodeLen=4+AdrCnt; HReg=BAsmCode[0];
               if (BAsmCode[0]==0x40) BAsmCode[3+AdrCnt]=0;
               else BAsmCode[3+AdrCnt]=BAsmCode[2];
               BAsmCode[2+AdrCnt]=BAsmCode[1];
               BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
               AdrMode=HReg; CorrMode(0,1);
               memcpy(BAsmCode+1,AdrVals,AdrCnt);
               BAsmCode[1+AdrCnt]=0x19;
              END
             else
              BEGIN
               CodeLen=4+HReg;
               BAsmCode[2+HReg]=AdrVals[0];
               if (AdrMode==0x40) BAsmCode[3+HReg]=0;
               else BAsmCode[3+HReg]=AdrVals[1];
               BAsmCode[0]+=0xb0; CorrMode(0,1);
               BAsmCode[1+HReg]=0x14+(OpSize << 1);
              END
            END
           break;
          case ModImm:
           if (BAsmCode[0]==0x40)
            BEGIN
             CodeLen=2+AdrCnt;
             BAsmCode[0]=0x08+(OpSize << 1);
             memcpy(BAsmCode+2,AdrVals,AdrCnt);
            END
           else
            BEGIN
             CodeLen=2+HReg+AdrCnt;
             BAsmCode[0]+=0xb0;
             BAsmCode[1+HReg]=OpSize << 1;
             memcpy(BAsmCode+2+HReg,AdrVals,AdrCnt);
            END
           break;
         END
        break;
      END
    END
END

        static void DecodeFixed(Word Index)
BEGIN
   FixedOrder *FixedZ=FixedOrders+Index;

   if (ArgCnt!=0) WrError(1110);
   else if ((FixedZ->CPUFlag & (1 << (MomCPU-CPU96C141)))==0) WrError(1500);
   else
    BEGIN
     if (Hi(FixedZ->Code)==0)
      BEGIN
       CodeLen=1;
       BAsmCode[0]=Lo(FixedZ->Code);
      END
     else
      BEGIN
       CodeLen=2; 
       BAsmCode[0]=Hi(FixedZ->Code);
       BAsmCode[1]=Lo(FixedZ->Code);
      END
     if (FixedZ->InSup) CheckSup();
    END
END

        static void DecodeImm(Word Index)
BEGIN
   ImmOrder *ImmZ=ImmOrders+Index;
   Word AdrWord;
   Boolean OK;

   if ((ArgCnt>1) OR ((ImmZ->Default==-1) AND (ArgCnt==0))) WrError(1110);
   else
    BEGIN
     if (ArgCnt==0) 
      BEGIN
       AdrWord=ImmZ->Default; OK=True;
      END
     else AdrWord=EvalIntExpression(ArgStr[1],Int8,&OK);
     if (OK) 
      BEGIN
       if (((Maximum) AND (AdrWord>ImmZ->MaxMax)) OR ((NOT Maximum) AND (AdrWord>ImmZ->MinMax))) WrError(1320);
       else if (Hi(ImmZ->Code)==0) 
        BEGIN
         CodeLen=1; BAsmCode[0]=Lo(ImmZ->Code)+AdrWord;
        END
       else
        BEGIN
         CodeLen=2; BAsmCode[0]=Hi(ImmZ->Code);
         BAsmCode[1]=Lo(ImmZ->Code)+AdrWord;
        END
      END
     if (ImmZ->InSup) CheckSup();
    END
END

        static void DecodeReg(Word Index)
BEGIN
   RegOrder *RegZ=RegOrders+Index;

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     DecodeAdr(ArgStr[1],MModReg+MModXReg);
     if (AdrType!=ModNone) 
      BEGIN
       if (((1 << OpSize) & RegZ->OpMask)==0) WrError(1130);
       else if (AdrType==ModReg) 
        BEGIN
         BAsmCode[0]=Hi(RegZ->Code)+8+(OpSize << 4)+AdrMode;
         BAsmCode[1]=Lo(RegZ->Code);
         CodeLen=2;
        END
       else
        BEGIN
         BAsmCode[0]=Hi(RegZ->Code)+7+(OpSize << 4);
         BAsmCode[1]=AdrMode;
         BAsmCode[2]=Lo(RegZ->Code);
         CodeLen=3;
        END
      END
    END
END

/*---------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode, Byte NFlag, Boolean NSup)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ].CPUFlag=NFlag;
   FixedOrders[InstrZ].InSup=NSup;
   AddInstTable(InstTable,NName,InstrZ++,DecodeFixed);
END

        static void AddReg(char *NName, Word NCode, Byte NMask)
BEGIN
   if (InstrZ>=RegOrderCnt) exit(255);
   RegOrders[InstrZ].Name=NName;
   RegOrders[InstrZ].Code=NCode;
   RegOrders[InstrZ].OpMask=NMask;
   AddInstTable(InstTable,NName,InstrZ++,DecodeReg);
END

        static void AddImm(char *NName, Word NCode, Boolean NInSup,
                           Byte NMinMax, Byte NMaxMax, ShortInt NDefault)
BEGIN
   if (InstrZ>=ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name=NName;
   ImmOrders[InstrZ].Code=NCode;
   ImmOrders[InstrZ].InSup=NInSup;
   ImmOrders[InstrZ].MinMax=NMinMax;
   ImmOrders[InstrZ].MaxMax=NMaxMax;
   ImmOrders[InstrZ].Default=NDefault;
   AddInstTable(InstTable,NName,InstrZ++,DecodeImm);
END

        static void AddALU2(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ALU2OrderCnt) exit(255);
   ALU2Orders[InstrZ].Name=NName;
   ALU2Orders[InstrZ++].Code=NCode;
END

        static void AddShift(char *NName)
BEGIN
   if (InstrZ>=ShiftOrderCnt) exit(255);
   ShiftOrders[InstrZ++]=NName;
END

        static void AddMulDiv(char *NName)
BEGIN
   if (InstrZ>=MulDivOrderCnt) exit(255);
   MulDivOrders[InstrZ++]=NName;
END

        static void AddBitCF(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=BitCFOrderCnt) exit(255);
   BitCFOrders[InstrZ].Name=NName;
   BitCFOrders[InstrZ++].Code=NCode;
END

        static void AddBit(char *NName)
BEGIN
   if (InstrZ>=BitOrderCnt) exit(255);
   BitOrders[InstrZ++]=NName;
END

        static void AddCondition(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ConditionCnt) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   InstTable=CreateInstTable(201);
   AddInstTable(InstTable,"MULA"  ,0,DecodeMULA);
   AddInstTable(InstTable,"JP"    ,0,DecodeJPCALL);
   AddInstTable(InstTable,"CALL"  ,1,DecodeJPCALL);
   AddInstTable(InstTable,"JR"    ,0,DecodeJR);
   AddInstTable(InstTable,"JRL"   ,1,DecodeJR);
   AddInstTable(InstTable,"CALR"  ,0,DecodeCALR);
   AddInstTable(InstTable,"RET"   ,0,DecodeRET);
   AddInstTable(InstTable,"RETD"  ,0,DecodeRETD);
   AddInstTable(InstTable,"DJNZ"  ,0,DecodeDJNZ);
   AddInstTable(InstTable,"EX"    ,0,DecodeEX);
   AddInstTable(InstTable,"BS1F"  ,0,DecodeBS1x);
   AddInstTable(InstTable,"BS1B"  ,1,DecodeBS1x);
   AddInstTable(InstTable,"LDA"   ,0,DecodeLDA);
   AddInstTable(InstTable,"LDAR"  ,0,DecodeLDAR);
   AddInstTable(InstTable,"LDC"   ,0,DecodeLDC);
   AddInstTable(InstTable,"LDX"   ,0,DecodeLDX);
   AddInstTable(InstTable,"LINK"  ,0,DecodeLINK);
   AddInstTable(InstTable,"LDB"   ,0,DecodeLD);
   AddInstTable(InstTable,"LDW"   ,1,DecodeLD);
   AddInstTable(InstTable,"LDL"   ,2,DecodeLD);
   AddInstTable(InstTable,"LD"    ,3,DecodeLD);

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("CCF"   , 0x0012, 3, False);
   AddFixed("DECF"  , 0x000d, 3, False);
   AddFixed("DI"    , 0x0607, 3, True );
   AddFixed("HALT"  , 0x0005, 3, True );
   AddFixed("INCF"  , 0x000c, 3, False);
   AddFixed("MAX"   , 0x0004, 1, True );
   AddFixed("MIN"   , 0x0004, 2, True );
   AddFixed("NOP"   , 0x0000, 3, False);
   AddFixed("NORMAL", 0x0001, 1, True );
   AddFixed("RCF"   , 0x0010, 3, False);
   AddFixed("RETI"  , 0x0007, 3, True );
   AddFixed("SCF"   , 0x0011, 3, False);
   AddFixed("ZCF"   , 0x0013, 3, False);

   RegOrders=(RegOrder *) malloc(sizeof(RegOrder)*RegOrderCnt); InstrZ=0;
   AddReg("CPL" , 0xc006, 3);
   AddReg("DAA" , 0xc010, 1);
   AddReg("EXTS", 0xc013, 6);
   AddReg("EXTZ", 0xc012, 6);
   AddReg("MIRR", 0xc016, 2);
   AddReg("NEG" , 0xc007, 3);
   AddReg("PAA" , 0xc014, 6);
   AddReg("UNLK", 0xc00d, 4);

   ImmOrders=(ImmOrder *) malloc(sizeof(ImmOrder)*ImmOrderCnt); InstrZ=0;
   AddImm("EI"  , 0x0600, True,  7, 7,  0);
   AddImm("LDF" , 0x1700, False, 7, 3, -1);
   AddImm("SWI" , 0x00f8, False, 7, 7,  7);

   ALU2Orders=(ALU2Order *) malloc(sizeof(ALU2Order)*ALU2OrderCnt); InstrZ=0;
   AddALU2("ADC", 1);
   AddALU2("ADD", 0);
   AddALU2("AND", 4);
   AddALU2("OR" , 6);
   AddALU2("SBC", 3);
   AddALU2("SUB", 2);
   AddALU2("XOR", 5);
   AddALU2("CP" , 7);

   ShiftOrders=(char **) malloc(sizeof(char*)*ShiftOrderCnt); InstrZ=0;
   AddShift("RLC");
   AddShift("RRC");
   AddShift("RL");
   AddShift("RR");
   AddShift("SLA");
   AddShift("SRA");
   AddShift("SLL");
   AddShift("SRL");

   MulDivOrders=(char **) malloc(sizeof(char*)*MulDivOrderCnt); InstrZ=0;
   AddMulDiv("MUL");
   AddMulDiv("MULS");
   AddMulDiv("DIV");
   AddMulDiv("DIVS");

   BitCFOrders=(ALU2Order *) malloc(sizeof(ALU2Order)*BitCFOrderCnt); InstrZ=0;
   AddBitCF("ANDCF" , 0);
   AddBitCF("LDCF"  , 3);
   AddBitCF("ORCF"  , 1);
   AddBitCF("STCF"  , 4);
   AddBitCF("XORCF" , 2);

   BitOrders=(char **) malloc(sizeof(char*)*BitOrderCnt); InstrZ=0;
   AddBit("RES");
   AddBit("SET");
   AddBit("CHG");
   AddBit("BIT");
   AddBit("TSET");

   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCnt); InstrZ=0;
   AddCondition("F"   ,  0); 
   DefaultCondition=InstrZ;  AddCondition("T"   ,  8);
   AddCondition("Z"   ,  6); AddCondition("NZ"  , 14);
   AddCondition("C"   ,  7); AddCondition("NC"  , 15);
   AddCondition("PL"  , 13); AddCondition("MI"  ,  5);
   AddCondition("P"   , 13); AddCondition("M"   ,  5);
   AddCondition("NE"  , 14); AddCondition("EQ"  ,  6);
   AddCondition("OV"  ,  4); AddCondition("NOV" , 12);
   AddCondition("PE"  ,  4); AddCondition("PO"  , 12);
   AddCondition("GE"  ,  9); AddCondition("LT"  ,  1);
   AddCondition("GT"  , 10); AddCondition("LE"  ,  2);
   AddCondition("UGE" , 15); AddCondition("ULT" ,  7);
   AddCondition("UGT" , 11); AddCondition("ULE" ,  3);
END

        static void DeinitFields(void)
BEGIN
   DestroyInstTable(InstTable);
   free(FixedOrders);
   free(RegOrders);
   free(ImmOrders);
   free(ALU2Orders);
   free(ShiftOrders);
   free(MulDivOrders);
   free(BitCFOrders);
   free(BitOrders);
   free(Conditions);
END

        static Boolean WMemo(char *Asc_O)
BEGIN
   int l=strlen(Asc_O);

   if (strncmp(OpPart,Asc_O,l)!=0) return False;
   switch (OpPart[l])
    BEGIN
     case '\0': 
      return True;
     case 'W':
      if (OpPart[l+1]=='\0')
       BEGIN
        OpSize=1; return True;  
       END
      else return False;
     case 'L':
      if (OpPart[l+1]=='\0')
       BEGIN
        OpSize=2; return True;  
       END
      else return False;
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

        static Boolean DecodePseudo(void)
BEGIN
   return False;
END

        static Boolean CodeMove(void)
BEGIN
   if ((WMemo("POP")) OR (WMemo("PUSH")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"F")==0)
      BEGIN
       CodeLen=1;
       BAsmCode[0]=0x18+Ord(Memo("POP"));
      END
     else if (strcasecmp(ArgStr[1],"A")==0)
      BEGIN
       CodeLen=1;
       BAsmCode[0]=0x14+Ord(Memo("POP"));
      END
     else if (strcasecmp(ArgStr[1],"SR")==0)
      BEGIN
       CodeLen=1;
       BAsmCode[0]=0x02+Ord(Memo("POP"));
       CheckSup();
      END
     else
      BEGIN
       MinOneIs0=True;
       DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem+
                           (WMemo("PUSH")?MModImm:0));
       switch (AdrType)
        BEGIN
         case ModReg:
          if (OpSize==0)
           BEGIN
            CodeLen=2;
            BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
            BAsmCode[1]=0x04+Ord(Memo("POP"));
           END
          else
           BEGIN
            CodeLen=1;
            BAsmCode[0]=0x28+(Ord(Memo("POP")) << 5)+((OpSize-1) << 4)+AdrMode;
           END
          break;
         case ModXReg:
          CodeLen=3;
          BAsmCode[0]=0xc7+(OpSize << 4);
          BAsmCode[1]=AdrMode;
          BAsmCode[2]=0x04+Ord(Memo("POP"));
          break;
         case ModMem:
          if (OpSize==-1) OpSize=0;
          CodeLen=2+AdrCnt;
          if (strncmp(OpPart,"POP",3)==0)
           BAsmCode[0]=0xb0+AdrMode;
           else BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          if (strncmp(OpPart,"POP",3)==0)
           BAsmCode[1+AdrCnt]=0x04+(OpSize << 1);
           else BAsmCode[1+AdrCnt]=0x04;
          break;
         case ModImm:
          if (OpSize==-1) OpSize=0;
          BAsmCode[0]=9+(OpSize << 1);
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
          break;
        END
      END
     return True;
    END

   return False;
END

        static void MakeCode_96C141(void)
BEGIN
   int z;
   Word AdrWord;
   Boolean OK;
   Byte HReg;
   char *CmpStr;
   char LChar;

   CodeLen=0; DontPrint=False; OpSize=(-1); MinOneIs0=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   if (CodeMove()) return;

   /* vermischt */

   if (LookupInstTable(InstTable,OpPart)) return;

   for (z=0; z<ALU2OrderCnt; z++)
    if (WMemo(ALU2Orders[z].Name)) 
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
        switch (AdrType)
         BEGIN
          case ModReg:
           HReg=AdrMode;
           DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
           switch (AdrType)
            BEGIN
             case ModReg:
              CodeLen=2;
              BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
              BAsmCode[1]=0x80+(ALU2Orders[z].Code << 4)+HReg;
              break;
             case ModXReg:
              CodeLen=3;
              BAsmCode[0]=0xc7+(OpSize << 4);
              BAsmCode[1]=AdrMode;
              BAsmCode[2]=0x80+(ALU2Orders[z].Code << 4)+HReg;
              break;
             case ModMem:
              CodeLen=2+AdrCnt;
              BAsmCode[0]=0x80+AdrMode+(OpSize << 4);
              memcpy(BAsmCode+1,AdrVals,AdrCnt);
              BAsmCode[1+AdrCnt]=0x80+HReg+(ALU2Orders[z].Code << 4);
              break;
             case ModImm:
              if ((ALU2Orders[z].Code==7) AND (OpSize!=2) AND (ImmVal()<=7) AND (ImmVal()>=0))
               BEGIN
                CodeLen=2;
                BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
                BAsmCode[1]=0xd8+AdrVals[0];
               END
              else
               BEGIN
                CodeLen=2+AdrCnt;
                BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
                BAsmCode[1]=0xc8+ALU2Orders[z].Code;
                memcpy(BAsmCode+2,AdrVals,AdrCnt);
               END
              break;
            END
           break;
          case ModXReg:
           HReg=AdrMode;
           DecodeAdr(ArgStr[2],MModImm);
           switch (AdrType)
            BEGIN
             case ModImm:
              if ((ALU2Orders[z].Code==7) AND (OpSize!=2) AND (ImmVal()<=7) AND (ImmVal()>=0))
               BEGIN
                CodeLen=3;
                BAsmCode[0]=0xc7+(OpSize << 4);
                BAsmCode[1]=HReg;
                BAsmCode[2]=0xd8+AdrVals[0];
               END
              else
               BEGIN
                CodeLen=3+AdrCnt;
                BAsmCode[0]=0xc7+(OpSize << 4);
                BAsmCode[1]=HReg;
                BAsmCode[2]=0xc8+ALU2Orders[z].Code;
                memcpy(BAsmCode+3,AdrVals,AdrCnt);
               END
              break;
            END
           break;
          case ModMem:
           MinOneIs0=True;
           HReg=AdrCnt; BAsmCode[0]=AdrMode;
           memcpy(BAsmCode+1,AdrVals,AdrCnt);
           DecodeAdr(ArgStr[2],MModReg+MModImm);
           switch (AdrType)
            BEGIN
             case ModReg:
              CodeLen=2+HReg; CorrMode(0,1);
              BAsmCode[0]+=0x80+(OpSize << 4);
              BAsmCode[1+HReg]=0x88+(ALU2Orders[z].Code << 4)+AdrMode;
              break;
             case ModImm:
              CodeLen=2+HReg+AdrCnt;
              BAsmCode[0]+=0x80+(OpSize << 4);
              BAsmCode[1+HReg]=0x38+ALU2Orders[z].Code;
              memcpy(BAsmCode+2+HReg,AdrVals,AdrCnt);
              break;
            END;
           break;
         END
       END
      return;
     END

   for (z=0; z<ShiftOrderCnt; z++)
    if (WMemo(ShiftOrders[z])) 
     BEGIN
      if ((ArgCnt!=1) AND (ArgCnt!=2))  WrError(1110);
      else
       BEGIN
        OK=True;
        if (ArgCnt==1) HReg=1;
        else if (strcasecmp(ArgStr[1],"A")==0) HReg=0xff;
        else
         BEGIN
          FirstPassUnknown=False;
          HReg=EvalIntExpression(ArgStr[1],Int8,&OK);
          if (OK) 
           BEGIN
            if (FirstPassUnknown) HReg&=0x0f;
            else
             if ((HReg==0) OR (HReg>16)) 
              BEGIN
               WrError(1320); OK=False;
              END
             else HReg&=0x0f;
           END
         END
        if (OK) 
         BEGIN
          DecodeAdr(ArgStr[ArgCnt],MModReg+MModXReg+((HReg==0xff)?0:MModMem));
          switch (AdrType)
           BEGIN
            case ModReg:
             CodeLen=2+Ord(HReg!=0xff);
             BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
             BAsmCode[1]=0xe8+z;
             if (HReg==0xff) BAsmCode[1]+=0x10;
             else BAsmCode[2]=HReg;
             break;
            case ModXReg:
             CodeLen=3+Ord(HReg!=0xff);
             BAsmCode[0]=0xc7+(OpSize << 4);
             BAsmCode[1]=AdrMode;
             BAsmCode[2]=0xe8+z;
             if (HReg==0xff) BAsmCode[2]+=0x10;
             else BAsmCode[3]=HReg;
             break;
            case ModMem:
             if (HReg!=1) WrError(1350);
             else
              BEGIN
               if (OpSize==-1) OpSize=0;
               CodeLen=2+AdrCnt;
               BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
               memcpy(BAsmCode+1,AdrVals,AdrCnt);
               BAsmCode[1+AdrCnt]=0x78+z;
              END
             break;
           END
         END
       END
      return;
     END

   for (z=0;z<MulDivOrderCnt; z++)
    if (Memo(MulDivOrders[z])) 
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg+MModXReg);
        if (OpSize==0) WrError(1130);
        else
         BEGIN
          if ((AdrType==ModReg) AND (OpSize==1))
           BEGIN
            if (AdrMode>3)
             BEGIN
              AdrType=ModXReg; AdrMode=0xe0+(AdrMode << 2);
             END
            else AdrMode+=1+AdrMode;
           END
          OpSize--;
          HReg=AdrMode;
          switch (AdrType)
           BEGIN
            case ModReg:
             DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
             switch (AdrType)
              BEGIN
               case ModReg:
                CodeLen=2;
                BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
                BAsmCode[1]=0x40+(z << 3)+HReg;
                break;
               case ModXReg:
                CodeLen=3;
                BAsmCode[0]=0xc7+(OpSize << 4);
                BAsmCode[1]=AdrMode;
                BAsmCode[2]=0x40+(z << 3)+HReg;
                break;
               case ModMem:
                CodeLen=2+AdrCnt;
                BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
                memcpy(BAsmCode+1,AdrVals,AdrCnt);
                BAsmCode[1+AdrCnt]=0x40+(z << 3)+HReg;
                break;
               case ModImm:
                CodeLen=2+AdrCnt;
                BAsmCode[0]=0xc8+(OpSize << 4)+HReg;
                BAsmCode[1]=0x08+z;
                memcpy(BAsmCode+2,AdrVals,AdrCnt);
                break;
              END
             break;
            case ModXReg:
             DecodeAdr(ArgStr[2],MModImm);
             if (AdrType==ModImm) 
              BEGIN
               CodeLen=3+AdrCnt;
               BAsmCode[0]=0xc7+(OpSize << 4);
               BAsmCode[1]=HReg;
               BAsmCode[2]=0x08+z;
               memcpy(BAsmCode+3,AdrVals,AdrCnt);
              END
             break;
           END
         END
       END
      return;
     END

   for (z=0; z<BitCFOrderCnt; z++)
    if (Memo(BitCFOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
        if (AdrType!=ModNone) 
         BEGIN
          if (OpSize==2) WrError(1130);
          else
           BEGIN
            if (AdrType==ModMem) OpSize=0;
            if (strcasecmp(ArgStr[1],"A")==0) 
             BEGIN
              HReg=0xff; OK=True;
             END
            else
             BEGIN
              FirstPassUnknown=False;
              HReg=EvalIntExpression(ArgStr[1],(OpSize==0)?UInt3:Int4,&OK);
             END
            if (OK) 
             switch (AdrType)
              BEGIN
               case ModReg:
                CodeLen=2+Ord(HReg!=0xff);
                BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
                BAsmCode[1]=0x20+(Ord(HReg==0xff) << 3)+BitCFOrders[z].Code;
                if (HReg!=0xff) BAsmCode[2]=HReg;
                break;
               case ModXReg:
                CodeLen=3+Ord(HReg!=0xff);
                BAsmCode[0]=0xc7+(OpSize << 4);
                BAsmCode[1]=AdrMode;
                BAsmCode[2]=0x20+(Ord(HReg==0xff) << 3)+BitCFOrders[z].Code;
                if (HReg!=0xff) BAsmCode[3]=HReg;
                break;
               case ModMem:
                CodeLen=2+AdrCnt;
                BAsmCode[0]=0xb0+AdrMode;
                memcpy(BAsmCode+1,AdrVals,AdrCnt);
                if (HReg==0xff) BAsmCode[1+AdrCnt]=0x28+BitCFOrders[z].Code;
                else BAsmCode[1+AdrCnt]=0x80+(BitCFOrders[z].Code << 3)+HReg;
                break;
              END
           END
         END
       END
      return;
     END

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z])) 
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
        if (AdrType==ModMem) OpSize=0;
        if (AdrType!=ModNone) 
         BEGIN
          if (OpSize==2) WrError(1130);
          else
           BEGIN
            HReg=EvalIntExpression(ArgStr[1],(OpSize==0)?UInt3:Int4,&OK);
            if (OK) 
             switch (AdrType)
              BEGIN
               case ModReg:
                CodeLen=3;
                BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
                BAsmCode[1]=0x30+z;
                BAsmCode[2]=HReg;
                break;
               case ModXReg:
                CodeLen=4;
                BAsmCode[0]=0xc7+(OpSize << 4);
                BAsmCode[1]=AdrMode;
                BAsmCode[2]=0x30+z;
                BAsmCode[3]=HReg;
                break;
               case ModMem:
                CodeLen=2+AdrCnt;
                if (z==4) z=0; else z++;
                BAsmCode[0]=0xb0+AdrMode;
                memcpy(BAsmCode+1,AdrVals,AdrCnt);
                BAsmCode[1+AdrCnt]=0xa8+(z << 3)+HReg;
                break;
              END
           END
         END
       END
      return;
     END

   if ((WMemo("INC")) OR (WMemo("DEC"))) 
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       if (ArgCnt==1) 
        BEGIN
         HReg=1; OK=True;
        END
       else HReg=EvalIntExpression(ArgStr[1],Int4,&OK);
       if (OK) 
        BEGIN
         if (FirstPassUnknown) HReg&=7;
         else if ((HReg<1) OR (HReg>8)) 
          BEGIN
           WrError(1320); OK=False;
          END
        END
       if (OK) 
        BEGIN
         HReg&=7;    /* 8-->0 */
         DecodeAdr(ArgStr[ArgCnt],MModReg+MModXReg+MModMem);
         switch (AdrType)
          BEGIN
           case ModReg:
            CodeLen=2;
            BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
            BAsmCode[1]=0x60+(Ord(WMemo("DEC")) << 3)+HReg;
            break;
           case ModXReg:
            CodeLen=3;
            BAsmCode[0]=0xc7+(OpSize << 4);
            BAsmCode[1]=AdrMode;
            BAsmCode[2]=0x60+(Ord(WMemo("DEC")) << 3)+HReg;
            break;
           case ModMem:
            if (OpSize==-1) OpSize=0;
            CodeLen=2+AdrCnt;
            BAsmCode[0]=0x80+AdrMode+(OpSize << 4);
            memcpy(BAsmCode+1,AdrVals,AdrCnt);
            BAsmCode[1+AdrCnt]=0x60+(Ord(WMemo("DEC")) << 3)+HReg;
            break;
          END
        END
      END
     return;
    END

   if ((Memo("CPD")) OR (Memo("CPDR")) OR (Memo("CPI")) OR (Memo("CPIR"))) 
    BEGIN
     if ((ArgCnt!=0) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==0) 
        BEGIN
         OK=True; OpSize=0; AdrMode=3;
        END
       else
        BEGIN
         OK=True;
         if (strcasecmp(ArgStr[1],"A")==0) OpSize=0;
         else if (strcasecmp(ArgStr[1],"WA")==0) OpSize=1;
         if (OpPart[2]=='I') CmpStr="+)"; else CmpStr="-)";
         if (OpSize==-1) OK=False;
         else if ((*ArgStr[2]!='(') OR (strcasecmp(ArgStr[2]+strlen(ArgStr[2])-2,CmpStr)!=0)) OK=False;
         else
          BEGIN
           ArgStr[2][strlen(ArgStr[2])-2]='\0';
           if (CodeEReg(ArgStr[2]+1,&AdrMode,&HReg)!=2) OK=False;
           else if (NOT IsRegBase(AdrMode,HReg)) OK=False;
           else if (NOT IsRegCurrent(AdrMode,HReg,&AdrMode)) OK=False;
          END
         if (NOT OK) WrError(1135);
        END
       if (OK) 
        BEGIN
         CodeLen=2;
         BAsmCode[0]=0x80+(OpSize << 4)+AdrMode;
         BAsmCode[1]=0x14+(Ord(OpPart[2]=='D') << 1)+(strlen(OpPart)-3);
        END
      END
     return;
    END

   if ((WMemo("LDD")) OR (WMemo("LDDR")) OR (WMemo("LDI")) OR (WMemo("LDIR"))) 
    BEGIN
     if (OpSize==-1) OpSize=0;
     if (OpSize==2) WrError(1130);
     else if ((ArgCnt!=0) AND (ArgCnt!=2)) WrError(1110);
     else
      BEGIN
       if (ArgCnt==0) 
        BEGIN
         OK=True; HReg=0;
        END
       else
        BEGIN
         OK=True;
         if (OpPart[2]=='I') CmpStr="+)"; else CmpStr="-)";
         if ((*ArgStr[1]!='(') OR (*ArgStr[2]!='(') OR
             (strcasecmp(ArgStr[1]+strlen(ArgStr[1])-2,CmpStr)!=0) OR
             (strcasecmp(ArgStr[2]+strlen(ArgStr[2])-2,CmpStr)!=0))  OK=False;
         else
          BEGIN
           strcpy(ArgStr[1],ArgStr[1]+1); ArgStr[1][strlen(ArgStr[1])-2]='\0';
           strcpy(ArgStr[2],ArgStr[2]+1); ArgStr[2][strlen(ArgStr[2])-2]='\0';
           if ((strcasecmp(ArgStr[1],"XIX")==0) AND (strcasecmp(ArgStr[2],"XIY")==0)) HReg=2;
           else if ((Maximum) AND (strcasecmp(ArgStr[1],"XDE")==0) AND (strcasecmp(ArgStr[2],"XHL")==0)) HReg=0;
           else if ((NOT Maximum) AND (strcasecmp(ArgStr[1],"DE")==0) AND (strcasecmp(ArgStr[2],"HL")==0)) HReg=0;
           else OK=False;
          END
        END
       if (NOT OK) WrError(1350);
       else
        BEGIN
         CodeLen=2;
         BAsmCode[0]=0x83+(OpSize << 4)+HReg;
         BAsmCode[1]=0x10+(Ord(OpPart[2]=='D') << 1)+Ord(strchr(OpPart,'R')!=Nil);
        END
      END
     return;
    END

   LChar = OpPart[strlen(OpPart) - 1];
   if (((strncmp(OpPart,"MDEC",4) == 0) OR (strncmp(OpPart,"MINC",4) == 0))
   AND (LChar >= '1') AND (LChar <= '4'))
    BEGIN
     if (LChar == '3') WrError(1135);
     else if (ArgCnt != 2) WrError(1110);
     else
      BEGIN
       AdrWord = EvalIntExpression(ArgStr[1], Int16, &OK);
       if (OK) 
        BEGIN
         if (NOT IsPwr2(AdrWord, &HReg)) WrError(1135);
         else if ((HReg == 0) OR ((LChar == '2') AND (HReg < 2)) OR ((LChar == '4') AND (HReg < 3))) WrError(1135);
         else
          BEGIN
           AdrWord -= LChar - '0';
           IsPwr2(LChar - '0', &HReg);
           DecodeAdr(ArgStr[2], MModReg + MModXReg);
           if ((AdrType != ModNone) AND (OpSize != 1)) WrError(1130);
           else 
            switch (AdrType)
             BEGIN
              case ModReg:
               CodeLen = 4;
               BAsmCode[0] = 0xd8 + AdrMode;
               BAsmCode[1] = 0x38 + (Ord(OpPart[1] == 'D') << 2) + HReg;
               BAsmCode[2] = Lo(AdrWord);
               BAsmCode[3] = Hi(AdrWord);
               break;
              case ModXReg:
               CodeLen = 5;
               BAsmCode[0] = 0xd7;
               BAsmCode[1] = AdrMode;
               BAsmCode[2] = 0x38 + (Ord(OpPart[1] == 'D') << 2) + HReg;
               BAsmCode[3] = Lo(AdrWord);
               BAsmCode[4] = Hi(AdrWord);
               break;
             END
          END
        END
      END
     return;
    END

   if ((Memo("RLD")) OR (Memo("RRD"))) 
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else if ((ArgCnt==2) AND (strcasecmp(ArgStr[1],"A")!=0)) WrError(1350);
     else
      BEGIN
       DecodeAdr(ArgStr[ArgCnt],MModMem);
       if (AdrType!=ModNone) 
        BEGIN
         CodeLen=2+AdrCnt;
         BAsmCode[0]=0x80+AdrMode;
         memcpy(BAsmCode+1,AdrVals,AdrCnt);
         BAsmCode[1+AdrCnt]=0x06+Ord(Memo("RRD"));
        END
      END
     return;
    END

   if (Memo("SCC")) 
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       z=0; NLS_UpString(ArgStr[1]);
       while ((z<ConditionCnt) AND (strcmp(Conditions[z].Name,ArgStr[1])!=0)) z++;
       if (z>=ConditionCnt) WrError(1360);
       else
        BEGIN
         DecodeAdr(ArgStr[2],MModReg+MModXReg);
         if (OpSize>1) WrError(1110);
         else 
          switch (AdrType)
           BEGIN
            case ModReg:
             CodeLen=2;
             BAsmCode[0]=0xc8+(OpSize << 4)+AdrMode;
             BAsmCode[1]=0x70+Conditions[z].Code;
             break;
            case ModXReg:
             CodeLen=3;
             BAsmCode[0]=0xc7+(OpSize << 4);
             BAsmCode[1]=AdrMode;
             BAsmCode[2]=0x70+Conditions[z].Code;
             break;
           END
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean ChkPC_96C141(LargeWord Addr)
BEGIN
   Boolean ok;

   switch (ActPC)
    BEGIN
     case SegCode:
      if (Maximum) ok=(Addr<=0xffffff);
              else ok=(Addr<=0xffff);
      break;
     default: 
      ok=False;
    END
   return (ok);
END


        static Boolean IsDef_96C141(void)
BEGIN
   return False;
END

        static void SwitchFrom_96C141(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void SwitchTo_96C141(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=True;

   PCSymbol="$"; HeaderID=0x52; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_96C141; ChkPC=ChkPC_96C141; IsDef=IsDef_96C141;
   SwitchFrom=SwitchFrom_96C141;
   AddONOFF("MAXMODE", &Maximum   , MaximumName   ,False);
   AddONOFF("SUPMODE", &SupAllowed, SupAllowedName,False);

   InitFields();
END

        void code96c141_init(void)
BEGIN
   CPU96C141=AddCPU("96C141",SwitchTo_96C141);
   CPU93C141=AddCPU("93C141",SwitchTo_96C141);
END
