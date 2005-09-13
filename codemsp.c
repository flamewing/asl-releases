/* codemsp.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MSP430                                                      */
/*                                                                           */
/* Historie:                                                                 */
/*             18. 8.1998 BookKeeping-Aufruf bei BSS                         */
/*              2. 1.1998 ChkPC umgestellt                                   */
/*              9. 3.2000 'ambiguous else'-Warnungen beseitigt               */
/*             2001-11-16 Endianness must be LSB first                       */
/*             2002-01-27 allow immediate addressing for one-op instrs(doj)  */
/*                                                                           */
/*****************************************************************************/
/* $Id: codemsp.c,v 1.2 2005/09/08 17:31:05 alfred Exp $                     */
/***************************************************************************** 
 * $Log: codemsp.c,v $
 * Revision 1.2  2005/09/08 17:31:05  alfred
 * - add missing include
 *
 * Revision 1.1  2003/11/06 02:49:23  alfred
 * - recreated
 *
 * Revision 1.5  2003/05/02 21:23:12  alfred
 * - strlen() updates
 *
 * Revision 1.4  2002/08/14 18:43:49  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.3  2002/07/27 17:44:50  alfred
 * - one more TempAll fix
 *
 * Revision 1.2  2002/07/14 18:39:59  alfred
 * - fixed TempAll-related warnings
 *
 *****************************************************************************/

#include "stdinc.h"

#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "endian.h"
#include "strutil.h"
#include "bpemu.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"  
#include "codepseudo.h"
#include "codevars.h"

#define TwoOpCount 12
#define OneOpCount 6
#define JmpCount 10

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Boolean MayByte;
          Word Code;
         } OneOpOrder;


static CPUVar CPUMSP430;

static FixedOrder *TwoOpOrders;
static OneOpOrder *OneOpOrders;
static FixedOrder *JmpOrders;

static Word AdrMode,AdrMode2,AdrPart,AdrPart2;
static Byte AdrCnt2;
static Word AdrVal,AdrVal2;
static Byte OpSize;
static Word PCDist;

/*-------------------------------------------------------------------------*/

        static void AddTwoOp(char *NName, Word NCode)
BEGIN
   if (InstrZ>=TwoOpCount) exit(255);
   TwoOpOrders[InstrZ].Name=NName;
   TwoOpOrders[InstrZ++].Code=NCode;
END

        static void AddOneOp(char *NName, Boolean NMay, Word NCode)
BEGIN
   if (InstrZ>=OneOpCount) exit(255);
   OneOpOrders[InstrZ].Name=NName;
   OneOpOrders[InstrZ].MayByte=NMay;
   OneOpOrders[InstrZ++].Code=NCode;
END

        static void AddJmp(char *NName, Word NCode)
BEGIN
   if (InstrZ>=JmpCount) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   TwoOpOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*TwoOpCount); InstrZ=0;
   AddTwoOp("MOV" ,0x4000); AddTwoOp("ADD" ,0x5000);
   AddTwoOp("ADDC",0x6000); AddTwoOp("SUBC",0x7000);
   AddTwoOp("SUB" ,0x8000); AddTwoOp("CMP" ,0x9000);
   AddTwoOp("DADD",0xa000); AddTwoOp("BIT" ,0xb000);
   AddTwoOp("BIC" ,0xc000); AddTwoOp("BIS" ,0xd000);
   AddTwoOp("XOR" ,0xe000); AddTwoOp("AND" ,0xf000);

   OneOpOrders=(OneOpOrder *) malloc(sizeof(OneOpOrder)*OneOpCount); InstrZ=0;
   AddOneOp("RRC" ,True ,0x1000); AddOneOp("RRA" ,True ,0x1100);
   AddOneOp("PUSH",True ,0x1200); AddOneOp("SWPB",False,0x1080);
   AddOneOp("CALL",False,0x1280); AddOneOp("SXT" ,False,0x1180);

   JmpOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*JmpCount); InstrZ=0;
   AddJmp("JNE" ,0x2000); AddJmp("JNZ" ,0x2000);
   AddJmp("JE"  ,0x2400); AddJmp("JZ"  ,0x2400);
   AddJmp("JNC" ,0x2800); AddJmp("JC"  ,0x2c00);
   AddJmp("JN"  ,0x3000); AddJmp("JGE" ,0x3400);
   AddJmp("JL"  ,0x3800); AddJmp("JMP" ,0x3C00);
END

        static void DeinitFields(void)
BEGIN
   free(TwoOpOrders);
   free(OneOpOrders);
   free(JmpOrders);
END

/*-------------------------------------------------------------------------*/

        static void ResetAdr(void)
BEGIN
   AdrMode=0xff; AdrCnt=0;
END

        static void ChkAdr(Byte Mask)
BEGIN
    if ((AdrMode!=0xff) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     ResetAdr(); WrError(1350);
    END
END

        static Boolean DecodeReg(char *Asc, Word *Erg)
BEGIN
   Boolean OK;

   if (strcasecmp(Asc,"PC")==0)
    BEGIN
     *Erg=0; return True;
    END
   else if (strcasecmp(Asc,"SP")==0)
    BEGIN
     *Erg=1; return True;
    END
   else if (strcasecmp(Asc,"SR")==0)
    BEGIN
     *Erg=2; return True;
    END
   if ((toupper(*Asc)=='R') AND (strlen(Asc)>=2) AND (strlen(Asc)<=3))
    BEGIN
     *Erg=ConstLongInt(Asc+1,&OK);
     return ((OK) AND (*Erg<16));
    END
   return False;
END

        static void DecodeAdr(char *Asc, Byte Mask, Boolean MayImm)
BEGIN
   Word AdrWord;
   Boolean OK;
   char *p;

   ResetAdr();

   /* immediate */

   if (*Asc=='#')
    BEGIN
     if (NOT MayImm) WrError(1350);
     else
      BEGIN
       AdrWord=EvalIntExpression(Asc+1,(OpSize==1)?Int8:Int16,&OK);
       if (OK)
        BEGIN
         switch (AdrWord)
          BEGIN
           case 0:
            AdrPart=3; AdrMode=0;
            break;
           case 1:
            AdrPart=3; AdrMode=1;
            break;
           case 2:
            AdrPart=3; AdrMode=2;
            break;
           case 4:
            AdrPart=2; AdrMode=2;
            break;
           case 8:
            AdrPart=2; AdrMode=3;
            break;
           case 0xffff:
            AdrPart=3; AdrMode=3;
            break;
           default:
            AdrVal=AdrWord; AdrCnt=1;
            AdrPart=0; AdrMode=3;
            break;
          END
        END
      END
     ChkAdr(Mask); return;
    END

   /* absolut */

   if (*Asc=='&')
    BEGIN
     AdrVal=EvalIntExpression(Asc+1,UInt16,&OK);
     if (OK)
      BEGIN
       AdrMode=1; AdrPart=2; AdrCnt=1;
      END
     ChkAdr(Mask); return;
    END

   /* Register */

   if (DecodeReg(Asc,&AdrPart))
    BEGIN
     if (AdrPart==3) WrXError(1445,Asc);
     else AdrMode=0;
     ChkAdr(Mask); return;
    END

   /* Displacement */

   if ((*Asc) && (Asc[strlen(Asc)-1]==')'))
    BEGIN
     Asc[strlen(Asc)-1]='\0';
     p=RQuotPos(Asc,'(');
     if (p!=Nil)
      BEGIN
       if (DecodeReg(p+1,&AdrPart))
        BEGIN
         *p='\0';
         AdrVal=EvalIntExpression(Asc,Int16,&OK);
         if (OK)
          BEGIN
           if ((AdrPart==2) OR (AdrPart==3)) WrXError(1445,Asc);
           else if ((AdrVal==0) AND ((Mask & 4)!=0)) AdrMode=2;
           else
            BEGIN
             AdrCnt=1; AdrMode=1;
            END
          END
         *p='(';
         ChkAdr(Mask); return;
        END
      END
     Asc[strlen(Asc)]=')';
    END

    /* indirekt mit/ohne Autoinkrement */

    if ((*Asc=='@') OR (*Asc=='*'))
     BEGIN
      if (Asc[strlen(Asc)-1]=='+')
       BEGIN
        AdrWord=1; Asc[strlen(Asc)-1]='\0';
       END
      else AdrWord=0;
      if (NOT DecodeReg(Asc+1,&AdrPart)) WrXError(1445,Asc);
      else if ((AdrPart==2) OR (AdrPart==3)) WrXError(1445,Asc);
      else if ((AdrWord==0) AND ((Mask & 4)==0))
       BEGIN
        AdrVal=0; AdrCnt=1; AdrMode=1;
       END
      else AdrMode=2+AdrWord;
      ChkAdr(Mask); return;
     END

    /* bleibt PC-relativ */

    AdrWord=EvalIntExpression(Asc,UInt16,&OK)-EProgCounter()-PCDist;
    if (OK)
     BEGIN
      AdrPart=0; AdrMode=1; AdrCnt=1; AdrVal=AdrWord;
     END

   ChkAdr(Mask);
END

/*-------------------------------------------------------------------------*/

        static void PutByte(Byte Value)
BEGIN
#if 0
   if (((CodeLen&1)==1) AND (NOT BigEndian))
    BEGIN
     BAsmCode[CodeLen]=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]=Value;
    END
   else
#endif
    BEGIN
     BAsmCode[CodeLen]=Value;
    END
   CodeLen++;
END

        static Boolean DecodePseudo(void)
BEGIN
   TempResult t;
   Word HVal16;
   int z;
   char *p;
   Boolean OK;

   if (Memo("BYTE"))
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       z=1; OK=True;
       do
        BEGIN
         KillBlanks(ArgStr[z]);
         FirstPassUnknown=False;
         EvalExpression(ArgStr[z],&t);
         switch (t.Typ)
          BEGIN
           case TempInt:
            if (FirstPassUnknown) t.Contents.Int&=0xff;
            if (NOT RangeCheck(t.Contents.Int,Int8)) WrError(1320);
            else if (CodeLen==MaxCodeLen)
             BEGIN
              WrError(1920); OK=False;
             END
            else PutByte(t.Contents.Int);
            break;
           case TempFloat:
            WrError(1135); OK=False;
            break;
           case TempString:
            if (strlen(t.Contents.Ascii)+CodeLen>=MaxCodeLen)
             BEGIN
              WrError(1920); OK=False;
             END
            else
             BEGIN
              TranslateString(t.Contents.Ascii);
              for (p=t.Contents.Ascii; *p!='\0'; PutByte(*(p++)));
             END
            break;
           default: 
            OK=False; break;
          END
         z++;
        END
       while ((z<=ArgCnt) AND (OK));
       if (NOT OK) CodeLen=0;
       else if ((Odd(CodeLen)) AND (DoPadding)) PutByte(0);
      END
     return True;
    END

   if (Memo("WORD")) 
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       z=1; OK=True;
       do
        BEGIN
         HVal16=EvalIntExpression(ArgStr[z],Int16,&OK);
         if (OK)
          BEGIN
           WAsmCode[CodeLen >> 1]=HVal16;
           CodeLen+=2;
          END
         z++;
        END
       while ((z<=ArgCnt) AND (OK));
       if (NOT OK) CodeLen=0;
      END
     return True;
    END

   if (Memo("BSS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       HVal16=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (FirstPassUnknown) WrError(1820);
       else if (OK)
        BEGIN
         if ((Odd(HVal16)) AND (DoPadding)) HVal16++;
         if (!HVal16) WrError(290);
         DontPrint=True; CodeLen=HVal16;
         BookKeeping();
         END
      END
     return True;
    END

/*  float exp (8bit bias 128) sign mant (impl. norm.)
   double exp (8bit bias 128) sign mant (impl. norm.) */

   return False;
END

        static void MakeCode_MSP(void)
BEGIN
   int z;
   Integer AdrInt;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Attribut bearbeiten */

   if (*AttrPart=='\0') OpSize=0;
   else if (strlen(AttrPart)>1) WrError(1107);
   else
    switch (toupper(*AttrPart))
     BEGIN
      case 'B': OpSize=1; break;
      case 'W': OpSize=0; break;
      default:  WrError(1107); return;
     END

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* zwei Operanden */

   for (z=0; z<TwoOpCount; z++)
    if (Memo(TwoOpOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        PCDist=2; DecodeAdr(ArgStr[1],15,True);
        if (AdrMode!=0xff)
         BEGIN
          AdrMode2=AdrMode; AdrPart2=AdrPart; AdrCnt2=AdrCnt; AdrVal2=AdrVal;
          PCDist+=AdrCnt2 << 1; DecodeAdr(ArgStr[2],3,False);
          if (AdrMode!=0xff)
           BEGIN
            WAsmCode[0]=TwoOpOrders[z].Code+(AdrPart2 << 8)+(AdrMode << 7)
                       +(OpSize << 6)+(AdrMode2 << 4)+AdrPart;
            memcpy(WAsmCode+1,&AdrVal2,AdrCnt2 << 1);
            memcpy(WAsmCode+1+AdrCnt2,&AdrVal,AdrCnt << 1);
            CodeLen=(1+AdrCnt+AdrCnt2) << 1;
           END
         END
       END
      return;
     END

   /* ein Operand */

   for (z=0; z<OneOpCount; z++)
    if (Memo(OneOpOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if ((OpSize==1) AND (NOT OneOpOrders[z].MayByte)) WrError(1130);
      else
       BEGIN
        PCDist=2; DecodeAdr(ArgStr[1],15,True);
        if (AdrMode!=0xff)
         BEGIN
          WAsmCode[0]=OneOpOrders[z].Code+(OpSize << 6)+(AdrMode << 4)+AdrPart;
          memcpy(WAsmCode+1,&AdrVal,AdrCnt << 1);
          CodeLen=(1+AdrCnt) << 1;
         END
       END
      return;
     END

   /* kein Operand */

   if (Memo("RETI"))
    BEGIN
     if (ArgCnt!=0) WrError(1110);
     else if (*AttrPart!='\0') WrError(1100);
     else if (OpSize!=0) WrError(1130);
     else
      BEGIN
       WAsmCode[0]=0x1300; CodeLen=2;
      END
     return;
    END

   /* Spruenge */

   for (z=0; z<JmpCount; z++)
    if (Memo(JmpOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (OpSize!=0) WrError(1130);
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+2);
        if (OK)
         BEGIN
          if (Odd(AdrInt)) WrError(1375);
          else if ((NOT SymbolQuestionable) AND ((AdrInt<-1024) OR (AdrInt>1022))) WrError(1370);
          else
           BEGIN
            WAsmCode[0]=JmpOrders[z].Code+((AdrInt >> 1) & 0x3ff);
            CodeLen=2;
           END
         END
       END
      return;
     END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_MSP(void)
BEGIN
   return False;
END

        static void SwitchFrom_MSP(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void SwitchTo_MSP(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x4a; NOPCode=0x4303; /* = MOV #0,#0 */
   DivideChars=","; HasAttrs=True; AttrChars=".";

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   AddONOFF("PADDING", &DoPadding, DoPaddingName,False);

   MakeCode=MakeCode_MSP; IsDef=IsDef_MSP;
   SwitchFrom=SwitchFrom_MSP; InitFields();
END

        void codemsp_init(void)
BEGIN
   CPUMSP430=AddCPU("MSP430",SwitchTo_MSP);
END
