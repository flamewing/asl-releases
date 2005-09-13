/* code9900.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS99xx                                                     */     
/*                                                                           */
/* Historie:  9. 3.1997 Grundsteinlegung                                     */     
/*           18. 8.1998 BookKeeping-Aufruf bei BSS                           */
/*            2. 1.1999 ChkPC angepasst                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code9900.c,v 1.2 2005/09/08 17:31:05 alfred Exp $                    */
/***************************************************************************** 
 * $Log: code9900.c,v $
 * Revision 1.2  2005/09/08 17:31:05  alfred
 * - add missing include
 *
 * Revision 1.1  2003/11/06 02:49:22  alfred
 * - recreated
 *
 * Revision 1.4  2003/05/02 21:23:11  alfred
 * - strlen() updates
 *
 * Revision 1.3  2002/08/14 18:43:49  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.2  2002/07/14 18:39:59  alfred
 * - fixed TempAll-related warnings
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"
#include "endian.h"
#include "bpemu.h"
#include "nls.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"


#define TwoOrderCount 6
#define OneOrderCount 6
#define SingOrderCount 16
#define SBitOrderCount 3
#define JmpOrderCount 13
#define ShiftOrderCount 4
#define ImmOrderCount 5
#define RegOrderCount 4
#define FixedOrderCount 6


typedef struct
         {
          char *Name;
          int NameLen;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          int NameLen;
          Word Code;
          Boolean MustSup;
         } SupOrder;


static CPUVar CPU9900;

static Boolean IsWord;
static Word AdrVal,AdrPart;

static FixedOrder *TwoOrders;
static FixedOrder *OneOrders;
static SupOrder *SingOrders;
static FixedOrder *SBitOrders;
static FixedOrder *JmpOrders;
static FixedOrder *ShiftOrders;
static FixedOrder *ImmOrders;
static FixedOrder *RegOrders;
static SupOrder *FixedOrders;

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

        static void AddTwo(char *NName, Word NCode)
BEGIN
   if (InstrZ>=TwoOrderCount) exit(255);
   TwoOrders[InstrZ].Name=NName;
   TwoOrders[InstrZ].Code=NCode;
   TwoOrders[InstrZ++].NameLen=strlen(NName);
END

        static void AddOne(char *NName, Word NCode)
BEGIN
   if (InstrZ>=OneOrderCount) exit(255);
   OneOrders[InstrZ].Name=NName;
   OneOrders[InstrZ++].Code=NCode;
END

        static void AddSing(char *NName, Word NCode,Boolean NSup)
BEGIN
   if (InstrZ>=SingOrderCount) exit(255);
   SingOrders[InstrZ].Name=NName;
   SingOrders[InstrZ].MustSup=NSup;
   SingOrders[InstrZ++].Code=NCode;
END

        static void AddSBit(char *NName, Word NCode)
BEGIN
   if (InstrZ>=SBitOrderCount) exit(255);
   SBitOrders[InstrZ].Name=NName;
   SBitOrders[InstrZ++].Code=NCode;
END

        static void AddJmp(char *NName, Word NCode)
BEGIN
   if (InstrZ>=JmpOrderCount) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ++].Code=NCode;
END

        static void AddShift(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ShiftOrderCount) exit(255);
   ShiftOrders[InstrZ].Name=NName;
   ShiftOrders[InstrZ++].Code=NCode;
END

        static void AddImm(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ImmOrderCount) exit(255);
   ImmOrders[InstrZ].Name=NName;
   ImmOrders[InstrZ++].Code=NCode;
END

        static void AddReg(char *NName, Word NCode)
BEGIN
   if (InstrZ>=RegOrderCount) exit(255);
   RegOrders[InstrZ].Name=NName;
   RegOrders[InstrZ++].Code=NCode;
END

        static void AddFixed(char *NName, Word NCode, Boolean NSup)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].MustSup=NSup;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   TwoOrders=(FixedOrder *) malloc(TwoOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddTwo("A"   ,5); AddTwo("C"   ,4); AddTwo("S"   ,3);
   AddTwo("SOC" ,7); AddTwo("SZC" ,2); AddTwo("MOV" ,6);

   OneOrders=(FixedOrder *) malloc(OneOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddOne("COC" ,0x08); AddOne("CZC" ,0x09); AddOne("XOR" ,0x0a);
   AddOne("MPY" ,0x0e); AddOne("DIV" ,0x0f); AddOne("XOP" ,0x0b);

   SingOrders=(SupOrder *) malloc(SingOrderCount*sizeof(SupOrder)); InstrZ=0;
   AddSing("B"   ,0x0440,False); AddSing("BL"  ,0x0680,False); AddSing("BLWP",0x0400,False);
   AddSing("CLR" ,0x04c0,False); AddSing("SETO",0x0700,False); AddSing("INV" ,0x0540,False);
   AddSing("NEG" ,0x0500,False); AddSing("ABS" ,0x0740,False); AddSing("SWPB",0x06c0,False);
   AddSing("INC" ,0x0580,False); AddSing("INCT",0x05c0,False); AddSing("DEC" ,0x0600,False);
   AddSing("DECT",0x0640,True ); AddSing("X"   ,0x0480,False); AddSing("LDS" ,0x0780,True );
   AddSing("LDD" ,0x07c0,True );

   SBitOrders=(FixedOrder *) malloc(SBitOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddSBit("SBO" ,0x1d); AddSBit("SBZ",0x1e); AddSBit("TB" ,0x1f);

   JmpOrders=(FixedOrder *) malloc(JmpOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddJmp("JEQ",0x13); AddJmp("JGT",0x15); AddJmp("JH" ,0x1b);
   AddJmp("JHE",0x14); AddJmp("JL" ,0x1a); AddJmp("JLE",0x12);
   AddJmp("JLT",0x11); AddJmp("JMP",0x10); AddJmp("JNC",0x17);
   AddJmp("JNE",0x16); AddJmp("JNO",0x19); AddJmp("JOC",0x18);
   AddJmp("JOP",0x1c);

   ShiftOrders=(FixedOrder *) malloc(ShiftOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddShift("SLA",0x0a); AddShift("SRA",0x08);
   AddShift("SRC",0x0b); AddShift("SRL",0x09);

   ImmOrders=(FixedOrder *) malloc(ImmOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddImm("AI"  ,0x011); AddImm("ANDI",0x012); AddImm("CI"  ,0x014);
   AddImm("LI"  ,0x010); AddImm("ORI" ,0x013);

   RegOrders=(FixedOrder *) malloc(RegOrderCount*sizeof(FixedOrder)); InstrZ=0;
   AddReg("STST",0x02c); AddReg("LST",0x008);
   AddReg("STWP",0x02a); AddReg("LWP",0x009);

   FixedOrders=(SupOrder *) malloc(FixedOrderCount*sizeof(SupOrder)); InstrZ=0;
   AddFixed("RTWP",0x0380,False); AddFixed("IDLE",0x0340,True );
   AddFixed("RSET",0x0360,True ); AddFixed("CKOF",0x03c0,True );
   AddFixed("CKON",0x03a0,True ); AddFixed("LREX",0x03e0,True );
END

        static void DeinitFields(void)
BEGIN
   free(TwoOrders);
   free(OneOrders);
   free(SingOrders);
   free(SBitOrders);
   free(JmpOrders);
   free(ShiftOrders);
   free(ImmOrders);
   free(RegOrders);
   free(FixedOrders);
END

/*-------------------------------------------------------------------------*/
/* Adressparser */

        static Boolean DecodeReg(char *Asc, Word *Erg)
BEGIN
   Boolean OK;
   *Erg=EvalIntExpression(Asc,UInt4,&OK); return OK;
END

        static char *HasDisp(char *Asc)
BEGIN
   char *p;
   int Lev;

   if ((*Asc) && (Asc[strlen(Asc)-1]==')'))
    BEGIN
     p=Asc+strlen(Asc)-2; Lev=0;
     while ((p>=Asc) AND (Lev!=-1))
      BEGIN
       switch (*p)
        BEGIN
         case '(': Lev--; break;
         case ')': Lev++; break;
        END
       if (Lev!=-1) p--;
      END
     if (Lev!=-1)
      BEGIN
       WrError(1300); p=Nil;
      END
    END
   else p=Nil;

   return p;
END

        static Boolean DecodeAdr(char *Asc)
BEGIN
   Boolean IncFlag;
   String Reg;
   Boolean OK;
   char *p;

   AdrCnt=0;

   if (*Asc=='*')
    BEGIN
     Asc++;
     if (Asc[strlen(Asc)-1]=='+')
      BEGIN
       IncFlag=True; Asc[strlen(Asc)-1]='\0';
      END
     else IncFlag=False;
     if (DecodeReg(Asc,&AdrPart))
      BEGIN
       AdrPart+=0x10+(Ord(IncFlag) << 5);
       return True;
      END
     return False;
    END

   if (*Asc=='@')
    BEGIN
     Asc++; p=HasDisp(Asc);
     if (p==Nil)
      BEGIN
       FirstPassUnknown=False;
       AdrVal=EvalIntExpression(Asc,UInt16,&OK);
       if (OK)
        BEGIN
         AdrPart=0x20; AdrCnt=1;
         if ((NOT FirstPassUnknown) AND (IsWord) AND (Odd(AdrVal))) WrError(180);
         return True;
        END
      END
     else
      BEGIN
       strmaxcpy(Reg,p+1,255); Reg[strlen(Reg)-1]='\0';
       if (DecodeReg(Reg,&AdrPart))
        BEGIN
         if (AdrPart==0) WrXError(1445,Reg);
         else
          BEGIN
           *p='\0';
           AdrVal=EvalIntExpression(Asc,Int16,&OK);
           if (OK)
            BEGIN
             AdrPart+=0x20; AdrCnt=1; return True;
            END
          END
        END
      END
     return False;
    END  

   if (DecodeReg(Asc,&AdrPart)) return True;
   else
    BEGIN
     WrError(1350); return False;
    END
END

/*-------------------------------------------------------------------------*/

        static void PutByte(Byte Value)
BEGIN
   if (((CodeLen&1)==1) AND (NOT BigEndian))
    BEGIN
     BAsmCode[CodeLen]=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]=Value;
    END
   else
    BEGIN
     BAsmCode[CodeLen]=Value;
    END
   CodeLen++;
END

        static Boolean DecodePseudo(void)
BEGIN
   int z;
   char *p;
   Boolean OK;
   TempResult t;
   Word HVal16;

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
            OK=False;
            break;
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
         if ((DoPadding) AND (Odd(HVal16))) HVal16++;
         if (!HVal16) WrError(290);
         DontPrint=True; CodeLen=HVal16;
         BookKeeping();
        END
      END
     return True;
    END

   return False;
END

        static void MakeCode_9900(void)
BEGIN
   Word HPart;
   Integer AdrInt;
   int z;
   Boolean OK;

   CodeLen=0; DontPrint=False; IsWord=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* zwei Operanden */

   for (z=0; z<TwoOrderCount; z++)
    if ((strncmp(OpPart,TwoOrders[z].Name,TwoOrders[z].NameLen)==0) AND (((OpPart[TwoOrders[z].NameLen]=='B') AND (OpPart[TwoOrders[z].NameLen+1]=='\0')) OR (OpPart[TwoOrders[z].NameLen]=='\0')))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (DecodeAdr(ArgStr[1]))
       BEGIN
        WAsmCode[0]=AdrPart; WAsmCode[1]=AdrVal;
        HPart=AdrCnt;
        if (DecodeAdr(ArgStr[2]))
         BEGIN
          WAsmCode[0]+=AdrPart << 6; WAsmCode[1+HPart]=AdrVal;
          CodeLen=(1+HPart+AdrCnt) << 1;
          if (OpPart[strlen(OpPart)-1]=='B') WAsmCode[0]+=0x1000;
          WAsmCode[0]+=TwoOrders[z].Code << 13;
         END
       END
      return;
     END

   for (z=0; z<OneOrderCount; z++)
    if (Memo(OneOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (DecodeAdr(ArgStr[1]))
       BEGIN
        WAsmCode[0]=AdrPart; WAsmCode[1]=AdrVal;
        if (DecodeReg(ArgStr[2],&HPart))
         BEGIN
          WAsmCode[0]+=(HPart << 6)+(OneOrders[z].Code << 10);
          CodeLen=(1+AdrCnt) << 1;
         END
       END
      return;
     END

   if ((Memo("LDCR")) OR (Memo("STCR")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (DecodeAdr(ArgStr[1]))
      BEGIN
       WAsmCode[0]=0x3000+(Ord(Memo("STCR")) << 10)+AdrPart;
       WAsmCode[1]=AdrVal;
       FirstPassUnknown=False;
       HPart=EvalIntExpression(ArgStr[2],UInt5,&OK);
       if (FirstPassUnknown) HPart=1;
       if (OK)
        if (ChkRange(HPart,1,16))
         BEGIN
          WAsmCode[0]+=(HPart & 15) << 6;
          CodeLen=(1+AdrCnt) << 1;
         END
      END
     return;
    END

   for (z=0; z<ShiftOrderCount; z++)
    if (Memo(ShiftOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (DecodeReg(ArgStr[1],WAsmCode+0))
       BEGIN
        if (DecodeReg(ArgStr[2],&HPart))
         BEGIN
          WAsmCode[0]+=(HPart << 4)+(ShiftOrders[z].Code << 8);
          CodeLen=2;
         END
       END
      return;
     END

   for (z=0; z<ImmOrderCount; z++)
    if (Memo(ImmOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (DecodeReg(ArgStr[1],WAsmCode+0))
       BEGIN
        WAsmCode[1]=EvalIntExpression(ArgStr[2],Int16,&OK);
        if (OK)
         BEGIN
          WAsmCode[0]+=(ImmOrders[z].Code << 5); CodeLen=4;
         END
       END
      return;
     END

   for (z=0; z<RegOrderCount; z++)
    if (Memo(RegOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (DecodeReg(ArgStr[1],WAsmCode+0))
       BEGIN
        WAsmCode[0]+=RegOrders[z].Code << 4; CodeLen=2;
       END
      return;
     END;

   if (Memo("LMF"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (DecodeReg(ArgStr[1],WAsmCode+0))
      BEGIN
       WAsmCode[0]+=0x320+(EvalIntExpression(ArgStr[2],UInt1,&OK) << 4);
       if (OK) CodeLen=2;
       if (NOT SupAllowed) WrError(50);
      END
     return;
    END

   /* ein Operand */

   if ((Memo("MPYS")) OR (Memo("DIVS")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeAdr(ArgStr[1]))
      BEGIN
       WAsmCode[0]=0x0180+(Ord(Memo("MPYS")) << 6)+AdrPart;
       WAsmCode[1]=AdrVal;
       CodeLen=(1+AdrCnt) << 1;
      END
     return;
    END

   for (z=0; z<SingOrderCount; z++)
    if (Memo(SingOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (DecodeAdr(ArgStr[1]))
       BEGIN
        WAsmCode[0]=SingOrders[z].Code+AdrPart;
        WAsmCode[1]=AdrVal;
        CodeLen=(1+AdrCnt) << 1;
        if ((SingOrders[z].MustSup) AND (NOT SupAllowed)) WrError(50);
       END
      return;
     END

   for (z=0; z<SBitOrderCount; z++)
    if (Memo(SBitOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        WAsmCode[0]=EvalIntExpression(ArgStr[1],SInt8,&OK);
        if (OK)
         BEGIN
          WAsmCode[0]=(WAsmCode[0] & 0xff) | (SBitOrders[z].Code << 8);
          CodeLen=2;
         END
       END
      return;
     END

   for (z=0; z<JmpOrderCount; z++)
    if (Memo(JmpOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK)-(EProgCounter()+2);
        if (OK)
         BEGIN
          if (Odd(AdrInt)) WrError(1375);
          else if ((NOT SymbolQuestionable) AND ((AdrInt<-256) OR (AdrInt>254))) WrError(1370);
          else
           BEGIN
            WAsmCode[0]=((AdrInt>>1) & 0xff) | (JmpOrders[z].Code << 8);
            CodeLen=2;
           END
         END
       END
      return;
     END

   if ((Memo("LWPI")) OR (Memo("LIMI")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       WAsmCode[1]=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        BEGIN
         WAsmCode[0]=(0x017+Ord(Memo("LIMI"))) << 5;
         CodeLen=4;
        END
      END
     return;
    END

   /* kein Operand */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        WAsmCode[0]=FixedOrders[z].Code; CodeLen=2;
        if ((FixedOrders[z].MustSup) AND (NOT SupAllowed)) WrError(50);
       END
      return;
     END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_9900(void)
BEGIN
   return False;
END

        static void SwitchFrom_9900(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void InternSymbol_9900(char *Asc, TempResult*Erg)
BEGIN
   Boolean err;
   char *h=Asc;

   Erg->Typ=TempNone;
   if ((strlen(Asc)>=2) AND (toupper(*Asc)=='R')) h=Asc+1;
   else if ((strlen(Asc)>=3) AND (toupper(*Asc)=='W') AND (toupper(Asc[1])=='R')) h=Asc+2;

   Erg->Contents.Int=ConstLongInt(h,&err);
   if ((NOT err) OR (Erg->Contents.Int<0) OR (Erg->Contents.Int>15)) return;

   Erg->Typ=TempInt;
END

        static void SwitchTo_9900()
BEGIN
   TurnWords=True; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x48; NOPCode=0x0000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   MakeCode=MakeCode_9900; IsDef=IsDef_9900;
   SwitchFrom=SwitchFrom_9900; InternSymbol=InternSymbol_9900;
   AddONOFF("PADDING", &DoPadding , DoPaddingName ,False);
   AddONOFF("SUPMODE", &SupAllowed, SupAllowedName,False);

   InitFields();
END

        void code9900_init(void)
BEGIN
   CPU9900=AddCPU("TMS9900",SwitchTo_9900);
END
