/* code75k0.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator NEC 75K0                                                    */
/*                                                                           */
/* Historie: 31.12.1996 Grundsteinlegung                                     */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code75k0.c,v 1.6 2014/03/08 21:06:36 alfred Exp $                    */
/*****************************************************************************
 * $Log: code75k0.c,v $
 * Revision 1.6  2014/03/08 21:06:36  alfred
 * - rework ASSUME framework
 *
 * Revision 1.5  2010/04/17 13:14:22  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.4  2005/10/02 10:00:45  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2005/09/08 17:31:04  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:01  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"  
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"


#define ModNone (-1)
#define ModReg4 0
#define MModReg4 (1 << ModReg4)
#define ModReg8 1
#define MModReg8 (1 << ModReg8)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModInd 3
#define MModInd (1 << ModInd)
#define ModAbs 4
#define MModAbs (1 << ModAbs)

#define FixedOrderCount 6
#define AriOrderCount 3
#define LogOrderCount 3

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;


static SimpProc SaveInitProc;

static FixedOrder *FixedOrders;
static char **AriOrders;
static char **LogOrders;

static LongInt MBSValue,MBEValue;
static Boolean MinOneIs0;
static CPUVar
   CPU75402,CPU75004,CPU75006,CPU75008,
   CPU75268,CPU75304,CPU75306,CPU75308,
   CPU75312,CPU75316,CPU75328,CPU75104,
   CPU75106,CPU75108,CPU75112,CPU75116,
   CPU75206,CPU75208,CPU75212,CPU75216,
   CPU75512,CPU75516;
static Word ROMEnd;

static ShortInt OpSize;
static Byte AdrPart;
static ShortInt AdrMode;

/*-------------------------------------------------------------------------*/
/* dynamische Codetabellenverwaltung */

        static void AddFixed(char *NewName, Word NewCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NewName;
   FixedOrders[InstrZ++].Code=NewCode;
END

        static void InitFields(void)
BEGIN
   Boolean Err;

   ROMEnd=ConstLongInt(MomCPUName+3,&Err,10);
   if (ROMEnd>2) ROMEnd%=10;
   ROMEnd=(ROMEnd << 10)-1;

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("RET" ,0x00ee);
   AddFixed("RETS",0x00e0);
   AddFixed("RETI",0x00ef);
   AddFixed("HALT",0xa39d);
   AddFixed("STOP",0xb39d);
   AddFixed("NOP" ,0x0060);

   AriOrders=(char **) malloc(sizeof(char *)*AriOrderCount); InstrZ=0;
   AriOrders[InstrZ++]="ADDC";
   AriOrders[InstrZ++]="SUBS";
   AriOrders[InstrZ++]="SUBC";

   LogOrders=(char **) malloc(sizeof(char *)*LogOrderCount); InstrZ=0;
   LogOrders[InstrZ++]="AND";
   LogOrders[InstrZ++]="OR";
   LogOrders[InstrZ++]="XOR";
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(AriOrders);
   free(LogOrders);
END

/*-------------------------------------------------------------------------*/
/* Untermengen von Befehlssatz abpruefen */

        static void CheckCPU(CPUVar MinCPU)
BEGIN
   if (MomCPU<MinCPU)
    BEGIN
     WrError(1500); CodeLen=0;
    END
END

/*-------------------------------------------------------------------------*/
/* Adressausdruck parsen */

        static Boolean SetOpSize(ShortInt NewSize)
BEGIN
   if (OpSize==-1) OpSize=NewSize;
   else if (NewSize!=OpSize)
    BEGIN
     WrError(1131); return False;
    END
   return True;
END

        static void ChkDataPage(Word Adr)
BEGIN
   switch (MBEValue)
    BEGIN
     case 0: if ((Adr>0x7f) AND (Adr<0xf80)) WrError(110); break;
     case 1: if (Hi(Adr)!=MBSValue) WrError(110); break;
    END
END

static void CheckMBE(void)
{
  if ((MomCPU == CPU75402) && (MBEValue != 0))
  {
    MBEValue = 0; WrError(1440);
  }
}

        static void ChkAdr(Byte Mask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     WrError(1350); AdrMode=ModNone;
    END
END 

        static void DecodeAdr(char *Asc, Byte Mask)
BEGIN
   static char *RegNames="XAHLDEBC";

   char *p;
   int pos;
   Boolean OK;
   String s;

   AdrMode=ModNone;

   /* Register ? */

   memcpy(s,Asc,2); s[2]='\0'; NLS_UpString(s);
   p=strstr(RegNames,s);

   if (p!=Nil)
    BEGIN
     pos=p-RegNames;

     /* 8-Bit-Register ? */

     if (strlen(Asc)==1)
      BEGIN
       AdrPart=pos ^ 1;
       if (SetOpSize(0))
        BEGIN
         if ((AdrPart>4) AND (MomCPU<CPU75004)) WrError(1505);
         else AdrMode=ModReg4;
        END
       ChkAdr(Mask); return;
      END

     /* 16-Bit-Register ? */

     if ((strlen(Asc)==2) AND (NOT Odd(pos)))
      BEGIN
       AdrPart=pos;
       if (SetOpSize(1))
        BEGIN
         if ((AdrPart>2) AND (MomCPU<CPU75004)) WrError(1505);
         else AdrMode=ModReg8;
        END
       ChkAdr(Mask); return;
      END

     /* 16-Bit-Schattenregister ? */

     if ((strlen(Asc)==3) AND ((Asc[2]=='\'') OR (Asc[2]=='`')) AND (NOT Odd(pos)))
      BEGIN
       AdrPart=pos+1;
       if (SetOpSize(1))
        BEGIN
         if (MomCPU<CPU75104) WrError(1505);
         else AdrMode=ModReg8;
        END
       ChkAdr(Mask); return;
      END
    END

   /* immediate? */

   if (*Asc=='#')
    BEGIN
     if ((OpSize==-1) AND (MinOneIs0)) OpSize=0;
     FirstPassUnknown=False;
     switch (OpSize)
      BEGIN
       case -1: WrError(1132); break;
       case 0: AdrPart=EvalIntExpression(Asc+1,Int4,&OK) & 15; break;
       case 1: AdrPart=EvalIntExpression(Asc+1,Int8,&OK); break;
      END;
     if (OK) AdrMode=ModImm;
     ChkAdr(Mask); return;
    END

   /* indirekt ? */

   if (*Asc=='@')
    BEGIN
     strmaxcpy(s,Asc+1,255);
     if (strcasecmp(s,"HL")==0) AdrPart=1;
     else if (strcasecmp(s,"HL+")==0) AdrPart=2;
     else if (strcasecmp(s,"HL-")==0) AdrPart=3;
     else if (strcasecmp(s,"DE")==0) AdrPart=4;
     else if (strcasecmp(s,"DL")==0) AdrPart=5;
     else AdrPart=0;
     if (AdrPart!=0)
      BEGIN
       if ((MomCPU<CPU75004) AND (AdrPart!=1)) WrError(1505);
       else if ((MomCPU<CPU75104) AND ((AdrPart==2) OR (AdrPart==3))) WrError(1505);
       else AdrMode=ModInd;
       ChkAdr(Mask); return;
      END
    END

   /* absolut */

   FirstPassUnknown=False;
   pos=EvalIntExpression(Asc,UInt12,&OK);
   if (OK)
    BEGIN
     AdrPart=Lo(pos); AdrMode=ModAbs;
     ChkSpace(SegData);
     if (NOT FirstPassUnknown) ChkDataPage(pos);
    END

   ChkAdr(Mask);
END

static String BName;

        static Boolean DecodeBitAddr(char *Asc, Word *Erg)
BEGIN
   char *p;
   int Num;
   Boolean OK;
   Word Adr;
   String bpart;

   p=QuotPos(Asc,'.');
   if (p==Nil)
    BEGIN
     *Erg=EvalIntExpression(Asc,Int16,&OK);
     if (Hi(*Erg)!=0) ChkDataPage(((*Erg >> 4) & 0xf00)+Lo(*Erg));
     return OK;
    END

   *p='\0';
   strmaxcpy(bpart,p+1,255);

   if (strcasecmp(bpart,"@L")==0)
    BEGIN
     FirstPassUnknown=False;
     Adr=EvalIntExpression(Asc,UInt12,&OK);
     if (FirstPassUnknown) Adr=(Adr & 0xffc) | 0xfc0;
     if (OK)
      BEGIN
       ChkSpace(SegData);
       if ((Adr & 3)!=0) WrError(1325);
       else if (Adr<0xfc0) WrError(1315);
       else if (MomCPU<CPU75004) WrError(1505);
       else
        BEGIN
         *Erg=0x40+((Adr & 0x3c) >> 2);
         sprintf(BName,"%sH.@L",HexString(Adr,3));
         return True;
        END
      END
    END
   else
    BEGIN
     Num=EvalIntExpression(bpart,UInt2,&OK);
     if (OK)
      BEGIN
       if (strncasecmp(Asc,"@H",2)==0)
        BEGIN
         Adr=EvalIntExpression(Asc+2,UInt4,&OK);
         if (OK)
          BEGIN
           if (MomCPU<CPU75004) WrError(1505);
           else
            BEGIN
             *Erg=(Num << 4)+Adr;
             sprintf(BName,"@H%s.%c",HexString(Adr,1),Num+'0');
             return True;
            END
          END
        END
       else
        BEGIN
         FirstPassUnknown=False;
         Adr=EvalIntExpression(Asc,UInt12,&OK);
         if (FirstPassUnknown) Adr=(Adr | 0xff0);
         if (OK)
          BEGIN
           ChkSpace(SegData);
           if ((Adr>=0xfb0) AND (Adr<0xfc0))
            *Erg=0x80+(Num << 4)+(Adr & 15);
           else if (Adr>=0xff0)
            *Erg=0xc0+(Num << 4)+(Adr & 15);
           else
            *Erg=0x400+(((Word)Num) << 8)+Lo(Adr)+(Hi(Adr) << 12);
           sprintf(BName,"%sH.%c",HexString(Adr,3),'0'+Num);
           return True;
          END
        END
      END
    END
   return False;
END

        static Boolean DecodeIntName(char *Asc, Byte *Erg)
BEGIN
   Word HErg;
   Byte LPart;
   String Asc_N;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   if (MomCPU<=CPU75402) LPart=0;
   else if (MomCPU<CPU75004) LPart=1;
   else if (MomCPU<CPU75104) LPart=2;
   else LPart=3;
        if (strcmp(Asc,"IEBT")==0)   HErg=0x000;
   else if (strcmp(Asc,"IEW")==0)    HErg=0x102;
   else if (strcmp(Asc,"IETPG")==0)  HErg=0x203;
   else if (strcmp(Asc,"IET0")==0)   HErg=0x104;
   else if (strcmp(Asc,"IECSI")==0)  HErg=0x005;
   else if (strcmp(Asc,"IECSIO")==0) HErg=0x205;
   else if (strcmp(Asc,"IE0")==0)    HErg=0x006;
   else if (strcmp(Asc,"IE2")==0)    HErg=0x007;
   else if (strcmp(Asc,"IE4")==0)    HErg=0x120;
   else if (strcmp(Asc,"IEKS")==0)   HErg=0x123;
   else if (strcmp(Asc,"IET1")==0)   HErg=0x224;
   else if (strcmp(Asc,"IE1")==0)    HErg=0x126;
   else if (strcmp(Asc,"IE3")==0)    HErg=0x227;
   else HErg=0xfff;
   if (HErg==0xfff) return False;
   else if (Hi(HErg)>LPart) return False;
   else
    BEGIN
     *Erg=Lo(HErg); return True;
    END
END

/*-------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
   Word BErg;

   if (Memo("SFR"))
    BEGIN
     CodeEquate(SegData,0,0xfff);
     return True;
    END

   if (Memo("BIT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       if (DecodeBitAddr(ArgStr[1],&BErg))
        if (NOT FirstPassUnknown)
         BEGIN
          PushLocHandle(-1);
          EnterIntSymbol(LabPart,BErg,SegNone,False);
          sprintf(ListLine,"=%s",BName);
          PopLocHandle();
         END
      END
     return True;
    END

   return False;
END

        static void PutCode(Word Code)
BEGIN
   BAsmCode[0]=Lo(Code);
   if (Hi(Code)==0) CodeLen=1;
   else
    BEGIN
     BAsmCode[1]=Hi(Code); CodeLen=2;
    END
END

        static void MakeCode_75K0(void)
BEGIN
   Integer AdrInt,Dist;
   int z;
   Byte HReg;
   Word BVal;
   Boolean OK,BrRel,BrLong;

   CodeLen=0; DontPrint=False; OpSize=(-1); MinOneIs0=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(True)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else PutCode(FixedOrders[z].Code);
      return;
     END

   /* Datentransfer */

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd+MModAbs);
       switch (AdrMode)
        BEGIN
         case ModReg4:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModInd+MModAbs+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg4:
             if (HReg==0)
              BEGIN
               PutCode(0x7899+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
              END
             else if (AdrPart==0)
              BEGIN
               PutCode(0x7099+(((Word)HReg) << 8)); CheckCPU(CPU75004);
              END
             else WrError(1350);
             break;
            case ModInd:
             if (HReg!=0) WrError(1350);
             else PutCode(0xe0+AdrPart);
             break;
            case ModAbs:
             if (HReg!=0) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xa3; BAsmCode[1]=AdrPart; CodeLen=2;
              END
             break;
            case ModImm:
             if (HReg==0) PutCode(0x70+AdrPart);
             else
              BEGIN
               PutCode(0x089a+(((Word)AdrPart) << 12)+(((Word)HReg) << 8));
               CheckCPU(CPU75004);
              END
             break;
           END
          break;
         case ModReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModInd+MModImm);
          switch (AdrMode)
           BEGIN
            case ModReg8:
             if (HReg==0)
              BEGIN
               PutCode(0x58aa+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
              END
             else if (AdrPart==0)
              BEGIN
               PutCode(0x50aa+(((Word)HReg) << 8)); CheckCPU(CPU75004);
              END
             else WrError(1350);
             break;
            case ModAbs:
             if (HReg!=0) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xa2; BAsmCode[1]=AdrPart; CodeLen=2;
               if ((NOT FirstPassUnknown) AND (Odd(AdrPart))) WrError(180);
              END
             break;
            case ModInd:
             if ((HReg!=0) OR (AdrPart!=1)) WrError(1350);
             else
              BEGIN
               PutCode(0x18aa); CheckCPU(CPU75004);
              END
             break;
            case ModImm:
             if (Odd(HReg)) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0x89+HReg; BAsmCode[1]=AdrPart; CodeLen=2;
              END
             break;
           END
          break;
         case ModInd:
          if (AdrPart!=1) WrError(1350);
          else
           BEGIN
            DecodeAdr(ArgStr[2],MModReg4+MModReg8);
            switch (AdrMode)
             BEGIN
              case ModReg4:
               if (AdrPart!=0) WrError(1350);
               else
                BEGIN
                 PutCode(0xe8); CheckCPU(CPU75004);
                END
               break;
              case ModReg8:
               if (AdrPart!=0) WrError(1350);
               else
                BEGIN
                 PutCode(0x10aa); CheckCPU(CPU75004);
                END
               break;
             END
           END
          break;
         case ModAbs:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModReg8);
          switch (AdrMode)
           BEGIN
            case ModReg4:
             if (AdrPart!=0) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0x93; BAsmCode[1]=HReg; CodeLen=2;
              END
             break; 
            case ModReg8:
             if (AdrPart!=0) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0x92; BAsmCode[1]=HReg; CodeLen=2;
               if ((NOT FirstPassUnknown) AND (Odd(HReg))) WrError(180);
              END
             break;
           END
          break;
        END
      END
     return;
    END

   if (Memo("XCH"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModAbs+MModInd);
       switch (AdrMode)
        BEGIN
         case ModReg4:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModAbs+MModInd);
          switch (AdrMode)
           BEGIN
            case ModReg4:
             if (HReg==0) PutCode(0xd8+AdrPart);
             else if (AdrPart==0) PutCode(0xd8+HReg);
             else WrError(1350);
             break; 
            case ModAbs:
             if (HReg!=0) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xb3; BAsmCode[1]=AdrPart; CodeLen=2;
              END
             break;
            case ModInd:
             if (HReg!=0) WrError(1350);
             else PutCode(0xe8+AdrPart);
             break;
           END
          break;
         case ModReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModInd);
          switch (AdrMode)
           BEGIN 
            case ModReg8:
             if (HReg==0)
              BEGIN
               PutCode(0x40aa+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
              END
             else if (AdrPart==0)
              BEGIN
               PutCode(0x40aa+(((Word)HReg) << 8)); CheckCPU(CPU75004);
              END
             else WrError(1350);
             break;
            case ModAbs:
             if (HReg!=0) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xb2; BAsmCode[1]=AdrPart; CodeLen=2;
               if ((FirstPassUnknown) AND (Odd(AdrPart))) WrError(180);
              END
             break;
            case ModInd:
             if ((AdrPart!=1) OR (HReg!=0)) WrError(1350);
             else
              BEGIN
               PutCode(0x11aa); CheckCPU(CPU75004);
              END
             break;
           END
          break;
         case ModAbs:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModReg8);
          switch (AdrMode)
           BEGIN
            case ModReg4:
             if (AdrPart!=0) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xb3; BAsmCode[1]=HReg; CodeLen=2;
              END
             break;
            case ModReg8:
             if (AdrPart!=0) WrError(1350);
             else
              BEGIN
               BAsmCode[0]=0xb2; BAsmCode[1]=HReg; CodeLen=2;
               if ((FirstPassUnknown) AND (Odd(HReg))) WrError(180);
              END
             break;
           END
          break;
         case ModInd:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModReg4+MModReg8);
          switch (AdrMode)
           BEGIN 
            case ModReg4:
             if (AdrPart!=0) WrError(1350);
             else PutCode(0xe8+HReg);
             break;
            case ModReg8:
             if ((AdrPart!=0) OR (HReg!=1)) WrError(1350);
             else
              BEGIN
               PutCode(0x11aa); CheckCPU(CPU75004);
              END;
             break;
           END
          break;
        END
      END
     return;
    END

   if (Memo("MOVT"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"XA")!=0) WrError(1350);
     else if (strcasecmp(ArgStr[2],"@PCDE")==0)
      BEGIN
       PutCode(0xd4); CheckCPU(CPU75004);
      END
     else if (strcasecmp(ArgStr[2],"@PCXA")==0) PutCode(0xd0);
     else WrError(1350);
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     OK=Memo("PUSH");
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"BS")==0)
      BEGIN
       PutCode(0x0699+(Ord(OK) << 8)); CheckCPU(CPU75004);
      END
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8);
       switch (AdrMode)
        BEGIN
         case ModReg8:
          if (Odd(AdrPart)) WrError(1350);
          else PutCode(0x48+Ord(OK)+AdrPart);
          break;
        END
      END
     return;
    END

   if ((Memo("IN")) OR (Memo("OUT")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       z=Ord(Memo("IN"));
       if (z>0)
        BEGIN
         strcpy(ArgStr[3],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[3]);
        END
       if (strncasecmp(ArgStr[1],"PORT",4)!=0) WrError(1350);
       else
        BEGIN
         BAsmCode[1]=0xf0+EvalIntExpression(ArgStr[1]+4,UInt4,&OK);
         if (OK)
          BEGIN
           DecodeAdr(ArgStr[2],MModReg8+MModReg4);
           switch (AdrMode)
            BEGIN
             case ModReg4:
              if (AdrPart!=0) WrError(1350);
              else
               BEGIN
                BAsmCode[0]=0x93+(z << 4); CodeLen=2;
               END
              break;
             case ModReg8:
              if (AdrPart!=0) WrError(1350);
              else
               BEGIN
                BAsmCode[0]=0x92+(z << 4); CodeLen=2;
                CheckCPU(CPU75004);
               END
              break;
            END
          END
        END
      END
     return;
    END

   /* Arithmetik */

   if (Memo("ADDS"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8);
       switch (AdrMode)
        BEGIN
         case ModReg4:
          if (AdrPart!=0) WrError(1350);
          else
           BEGIN
            DecodeAdr(ArgStr[2],MModImm+MModInd);
            switch (AdrMode)
             BEGIN
              case ModImm: 
               PutCode(0x60+AdrPart); break;
              case ModInd:
               if (AdrPart==1) PutCode(0xd2); else WrError(1350);
               break;
             END
           END
          break;
         case ModReg8:
          if (AdrPart==0)
           BEGIN
            DecodeAdr(ArgStr[2],MModReg8+MModImm);
            switch (AdrMode)
             BEGIN
              case ModReg8:
               PutCode(0xc8aa+(((Word)AdrPart) << 8));
               CheckCPU(CPU75104);
               break;
              case ModImm:
               BAsmCode[0]=0xb9; BAsmCode[1]=AdrPart;
               CodeLen=2;
               CheckCPU(CPU75104);
               break;
             END
           END
          else if (strcasecmp(ArgStr[2],"XA")!=0) WrError(1350);
          else
           BEGIN
            PutCode(0xc0aa+(((Word)AdrPart) << 8));
            CheckCPU(CPU75104);
           END
          break;
        END
      END
     return;
    END

   for (z=0; z<AriOrderCount; z++)
    if (Memo(AriOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg4+MModReg8);
        switch (AdrMode)
         BEGIN
          case ModReg4:
           if (AdrPart!=0) WrError(1350);
           else
            BEGIN
             DecodeAdr(ArgStr[2],MModInd);
             switch (AdrMode)
              BEGIN
               case ModInd:
                if (AdrPart==1)
                 BEGIN
                  BAsmCode[0]=0xa8;
                  if (z==0) BAsmCode[0]++;
                  if (z==2) BAsmCode[0]+=0x10;
                  CodeLen=1;
                  if (NOT Memo("ADDC")) CheckCPU(CPU75004);
                 END
                else WrError(1350);
               break;
              END
            END
           break;
          case ModReg8:
           if (AdrPart==0)
            BEGIN
             DecodeAdr(ArgStr[2],MModReg8);
             switch (AdrMode)
              BEGIN
               case ModReg8:
                PutCode(0xc8aa+((z+1) << 12)+(((Word)AdrPart) << 8));
                CheckCPU(CPU75104);
                break;
              END
            END
           else if (strcasecmp(ArgStr[2],"XA")!=0) WrError(1350);
           else
            BEGIN
             PutCode(0xc0aa+((z+1) << 12)+(((Word)AdrPart) << 8));
             CheckCPU(CPU75104);
            END
           break;
         END
       END
      return;
     END

   for (z=0; z<LogOrderCount; z++)
    if (Memo(LogOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],MModReg4+MModReg8);
        switch (AdrMode)
         BEGIN
          case ModReg4:
           if (AdrPart!=0) WrError(1350);
           else
            BEGIN
             DecodeAdr(ArgStr[2],MModImm+MModInd);
             switch (AdrMode)
              BEGIN
               case ModImm:
                PutCode(0x2099+(((Word)AdrPart & 15) << 8)+((z+1) << 12));
                CheckCPU(CPU75004);
                break;
               case ModInd:
                if (AdrPart==1) PutCode(0x80+((z+1) << 4)); else WrError(1350);
                break;
              END
            END
           break;
          case ModReg8:
           if (AdrPart==0)
            BEGIN
             DecodeAdr(ArgStr[2],MModReg8);
             switch (AdrMode)
              BEGIN
               case ModReg8:
                PutCode(0x88aa+(((Word)AdrPart) << 8)+((z+1) << 12));
                CheckCPU(CPU75104);
                break;
              END
            END
           else if (strcasecmp(ArgStr[2],"XA")!=0) WrError(1350);
           else
            BEGIN
             PutCode(0x80aa+(((Word)AdrPart) << 8)+((z+1) << 12));
             CheckCPU(CPU75104);
            END
           break;
         END
       END
      return;
     END

   if (Memo("INCS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd+MModAbs);
       switch (AdrMode)
        BEGIN
         case ModReg4:
          PutCode(0xc0+AdrPart);
          break;
         case ModReg8:
          if ((AdrPart<1) OR (Odd(AdrPart))) WrError(1350);
          else
           BEGIN
            PutCode(0x88+AdrPart); CheckCPU(CPU75104);
           END
          break;
         case ModInd:
          if (AdrPart==1)
           BEGIN
            PutCode(0x0299); CheckCPU(CPU75004);
           END
          else WrError(1350);
          break;
         case ModAbs:
          BAsmCode[0]=0x82; BAsmCode[1]=AdrPart; CodeLen=2;
          break;
        END
      END
     return;
    END

   if (Memo("DECS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8);
       switch (AdrMode)
        BEGIN
         case ModReg4:
          PutCode(0xc8+AdrPart);
          break;
         case ModReg8:
          PutCode(0x68aa+(((Word)AdrPart) << 8));
          CheckCPU(CPU75104);
          break;
        END
      END
     return;
    END

   if (Memo("SKE"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd);
       switch (AdrMode)
        BEGIN
         case ModReg4:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModImm+MModInd+MModReg4);
          switch (AdrMode)
           BEGIN
            case ModReg4:
             if (HReg==0)
              BEGIN
               PutCode(0x0899+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
              END
             else if (AdrPart==0)
              BEGIN
               PutCode(0x0899+(((Word)HReg) << 8)); CheckCPU(CPU75004);
              END
             else WrError(1350);
             break;
            case ModImm:
             BAsmCode[0]=0x9a; BAsmCode[1]=(AdrPart << 4)+HReg;
             CodeLen=2;
             break;
            case ModInd:
             if ((AdrPart==1) AND (HReg==0)) PutCode(0x80);
             else WrError(1350);
             break;
           END
          break;
         case ModReg8:
          HReg=AdrPart;
          DecodeAdr(ArgStr[2],MModInd+MModReg8);
          switch (AdrMode)
           BEGIN
            case ModReg8:
             if (HReg==0)
              BEGIN
               PutCode(0x48aa+(((Word)AdrPart) << 8)); CheckCPU(CPU75104);
              END
             else if (AdrPart==0)
              BEGIN
               PutCode(0x48aa+(((Word)HReg) << 8)); CheckCPU(CPU75104);
              END
             else WrError(1350);
             break;
            case ModInd:
             if (AdrPart==1)
              BEGIN
               PutCode(0x19aa); CheckCPU(CPU75104);
              END
             else WrError(1350);
             break;
           END
          break;
         case ModInd:
          if (AdrPart!=1) WrError(1350);
          else
           BEGIN
            MinOneIs0=True;
            DecodeAdr(ArgStr[2],MModImm+MModReg4+MModReg8);
            switch (AdrMode)
             BEGIN
              case ModImm:
               PutCode(0x6099+(((Word)AdrPart) << 8)); CheckCPU(CPU75004);
               break;
              case ModReg4:
               if (AdrPart==0) PutCode(0x80); else WrError(1350);
               break;
              case ModReg8:
               if (AdrPart==0)
                BEGIN
                 PutCode(0x19aa); CheckCPU(CPU75004);
                END
               else WrError(1350);
               break;
             END
           END
          break;
        END
      END
     return;
    END

   if ((Memo("RORC")) OR (Memo("NOT")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
     else if (Memo("RORC")) PutCode(0x98);
     else PutCode(0x5f99);
     return;
    END

   /* Bitoperationen */

   if (Memo("MOV1"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       OK=True;
       if (strcasecmp(ArgStr[1],"CY")==0) z=0xbd;
       else if (strcasecmp(ArgStr[2],"CY")==0) z=0x9b;
       else OK=False;
       if (NOT OK) WrError(1350);
       else if (DecodeBitAddr(ArgStr[((z >> 2) & 3)-1],&BVal))
        BEGIN
         if (Hi(BVal)!=0) WrError(1350);
         else
          BEGIN
           BAsmCode[0]=z; BAsmCode[1]=BVal; CodeLen=2;
           CheckCPU(CPU75104);
          END
        END
      END
     return;
    END

   if ((Memo("SET1")) OR (Memo("CLR1")))
    BEGIN
     OK=Memo("SET1");
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CY")==0) PutCode(0xe6+Ord(OK));
     else if (DecodeBitAddr(ArgStr[1],&BVal))
      BEGIN
       if (Hi(BVal)!=0)
        BEGIN
         BAsmCode[0]=0x84+Ord(OK)+(Hi(BVal & 0x300) << 4);
         BAsmCode[1]=Lo(BVal); CodeLen=2;
        END
       else
        BEGIN
         BAsmCode[0]=0x9c+Ord(OK); BAsmCode[1]=BVal; CodeLen=2;
        END
      END
     return;
    END

   if ((Memo("SKT")) OR (Memo("SKF")))
    BEGIN
     OK=Memo("SKT");
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CY")==0)
      BEGIN
       if (Memo("SKT")) PutCode(0xd7);
       else WrError(1350);
      END
     else if (DecodeBitAddr(ArgStr[1],&BVal))
      BEGIN
       if (Hi(BVal)!=0)
        BEGIN
         BAsmCode[0]=0x86+Ord(OK)+(Hi(BVal & 0x300) << 4);
         BAsmCode[1]=Lo(BVal); CodeLen=2;
        END
       else
        BEGIN
         BAsmCode[0]=0xbe + Ord(OK); /* ANSI :-0 */
         BAsmCode[1]=BVal; CodeLen=2;
        END
      END
     return;
    END

   if (Memo("NOT1"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"CY")!=0) WrError(1350);
     else PutCode(0xd6);
     return;
    END

   if (Memo("SKTCLR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (DecodeBitAddr(ArgStr[1],&BVal))
      BEGIN
       if (Hi(BVal)!=0) WrError(1350);
       else
        BEGIN
         BAsmCode[0]=0x9f; BAsmCode[1]=BVal; CodeLen=2;
        END
      END
     return;
    END

   if (OpPart[strlen(OpPart)-1]=='1')
   for (z=0; z<LogOrderCount; z++)
    if (strncmp(LogOrders[z],OpPart,strlen(LogOrders[z]))==0)
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (strcasecmp(ArgStr[1],"CY")!=0) WrError(1350);
      else if (DecodeBitAddr(ArgStr[2],&BVal))
       BEGIN
        if (Hi(BVal)!=0) WrError(1350);
        else
         BEGIN
          BAsmCode[0]=0xac+((z & 1) << 1)+((z & 2) << 3);
          BAsmCode[1]=BVal; CodeLen=2;
         END
       END
      return;
     END

   /* Spruenge */

   if (Memo("BR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"PCDE")==0)
      BEGIN
       PutCode(0x0499); CheckCPU(CPU75004);
      END
     else if (strcasecmp(ArgStr[1],"PCXA")==0)
      BEGIN
       BAsmCode[0]=0x99; BAsmCode[1]=0x00; CodeLen=2;
       CheckCPU(CPU75104);
      END
     else
      BEGIN
       char *pArg1 = ArgStr[1];

       BrRel = False; BrLong = False;
       if (*pArg1 == '$')
       {
         BrRel = True; pArg1++;
       }
       else if (*pArg1 == '!')
       {
         BrLong = True; pArg1++;
       }
       AdrInt = EvalIntExpression(pArg1, UInt16, &OK);
       if (OK)
        BEGIN
         Dist=AdrInt-EProgCounter();
         if ((BrRel) OR ((Dist<=16) AND (Dist>=-15) AND (Dist!=0)))
          BEGIN
           if (Dist>0)
            BEGIN
             Dist--;
             if ((Dist>15) AND (NOT SymbolQuestionable)) WrError(1370);
             else PutCode(0x00+Dist);
            END
           else
            BEGIN
             if ((Dist<-15) AND (NOT SymbolQuestionable)) WrError(1370);
             else PutCode(0xf0+15+Dist);
            END
          END
         else if ((NOT BrLong) AND ((AdrInt >> 12)==(EProgCounter() >> 12)) AND ((EProgCounter() & 0xfff)<0xffe))
          BEGIN
           BAsmCode[0]=0x50+((AdrInt >> 8) & 15);
           BAsmCode[1]=Lo(AdrInt);
           CodeLen=2;
          END
         else
          BEGIN
           BAsmCode[0]=0xab;
           BAsmCode[1]=Hi(AdrInt & 0x3fff);
           BAsmCode[2]=Lo(AdrInt);
           CodeLen=3;
           CheckCPU(CPU75004);
          END
         ChkSpace(SegCode);
        END
      END
     return;
    END

   if (Memo("BRCB"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK);
       if (OK)
        BEGIN
         if ((AdrInt >> 12)!=(EProgCounter() >> 12)) WrError(1910);
         else if ((EProgCounter() & 0xfff)>=0xffe) WrError(1905);
         else
          BEGIN
           BAsmCode[0]=0x50+((AdrInt >> 8) & 15);
           BAsmCode[1]=Lo(AdrInt);
           CodeLen=2;
           ChkSpace(SegCode);
          END
        END
      END
     return;
    END

   if (Memo("CALL"))
   {
     if (ArgCnt != 1) WrError(1110);
     else
     {
       char *pArg1 = ArgStr[1];

       if (*pArg1 == '!')
       {
         pArg1++; BrLong = True;
       }
       else BrLong = False;
       FirstPassUnknown = False;
       AdrInt = EvalIntExpression(pArg1, UInt16, &OK);
       if (FirstPassUnknown) AdrInt &= 0x7ff;
       if (OK)
       {
         if ((BrLong) || (AdrInt > 0x7ff))
         {
           BAsmCode[0] = 0xab;
           BAsmCode[1] = 0x40 + Hi(AdrInt & 0x3fff);
           BAsmCode[2] = Lo(AdrInt);
           CodeLen = 3;
           CheckCPU(CPU75004);
         }
         else
         {
           BAsmCode[0] = 0x40 + Hi(AdrInt & 0x7ff);
           BAsmCode[1] = Lo(AdrInt);
           CodeLen = 2;
         }
         ChkSpace(SegCode);
       }
     }
     return;
   }

   if (Memo("CALLF"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
     {
       char *pArg1 = ArgStr[1];

       if (*pArg1 == '!') pArg1++;
       AdrInt = EvalIntExpression(pArg1, UInt11, &OK);
       if (OK)
       {
         BAsmCode[0] = 0x40 + Hi(AdrInt);
         BAsmCode[1] = Lo(AdrInt);
         CodeLen = 2;
         ChkSpace(SegCode);
       }
     }
     return;
    END

   if (Memo("GETI"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       BAsmCode[0]=EvalIntExpression(ArgStr[1],UInt6,&OK);
       CodeLen=Ord(OK);
       CheckCPU(CPU75004);
      END
     return;
    END

   /* Steueranweisungen */

   if ((Memo("EI")) OR (Memo("DI")))
    BEGIN
     OK=Memo("EI");
     if (ArgCnt==0) PutCode(0xb29c+Ord(OK));
     else if (ArgCnt!=1) WrError(1110);
     else if (DecodeIntName(ArgStr[1],&HReg)) PutCode(0x989c+Ord(OK)+(((Word)HReg) << 8));
     else WrError(1440);
     return;
    END

   if (Memo("SEL"))
    BEGIN
     BAsmCode[0]=0x99;
     if (ArgCnt!=1) WrError(1110);
     else if (strncasecmp(ArgStr[1],"RB",2)==0)
      BEGIN
       BAsmCode[1]=0x20+EvalIntExpression(ArgStr[1]+2,UInt2,&OK);
       if (OK)
        BEGIN
         CodeLen=2; CheckCPU(CPU75104);
        END
      END
     else if (strncasecmp(ArgStr[1],"MB",2)==0)
      BEGIN
       BAsmCode[1]=0x10+EvalIntExpression(ArgStr[1]+2,UInt4,&OK);
       if (OK)
        BEGIN
         CodeLen=2; CheckCPU(CPU75004);
        END
      END
     else WrError(1350);
     return;
    END

   WrXError(1200,OpPart);
END

        static void InitCode_75K0(void)
BEGIN
   SaveInitProc();
   MBSValue=0; MBEValue=0;
END

        static Boolean IsDef_75K0(void)
BEGIN
   return ((Memo("SFR")) OR (Memo("BIT")));
END

        static void SwitchFrom_75K0(void)
BEGIN
   DeinitFields();
END

#define ASSUME75Count 2
   static ASSUMERec ASSUME75s[ASSUME75Count]=
             {{"MBS", &MBSValue, 0, 0x0f, 0x10},
              {"MBE", &MBEValue, 0, 0x01, 0x01, CheckMBE}};

        static void SwitchTo_75K0(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="PC"; HeaderID=0x7b; NOPCode=0x60;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   Grans[SegData]=1; ListGrans[SegData]=1; SegInits[SegData]=0;
   SegLimits[SegData] = 0xfff;

   pASSUMERecs = ASSUME75s;
   ASSUMERecCnt = ASSUME75Count;

   MakeCode=MakeCode_75K0; IsDef=IsDef_75K0;
   SwitchFrom=SwitchFrom_75K0; InitFields();
   SegLimits[SegCode] = ROMEnd;
END

        void code75k0_init(void) 
BEGIN
   CPU75402=AddCPU("75402",SwitchTo_75K0);
   CPU75004=AddCPU("75004",SwitchTo_75K0);
   CPU75006=AddCPU("75006",SwitchTo_75K0);
   CPU75008=AddCPU("75008",SwitchTo_75K0);
   CPU75268=AddCPU("75268",SwitchTo_75K0);
   CPU75304=AddCPU("75304",SwitchTo_75K0);
   CPU75306=AddCPU("75306",SwitchTo_75K0);
   CPU75308=AddCPU("75308",SwitchTo_75K0);
   CPU75312=AddCPU("75312",SwitchTo_75K0);
   CPU75316=AddCPU("75316",SwitchTo_75K0);
   CPU75328=AddCPU("75328",SwitchTo_75K0);
   CPU75104=AddCPU("75104",SwitchTo_75K0);
   CPU75106=AddCPU("75106",SwitchTo_75K0);
   CPU75108=AddCPU("75108",SwitchTo_75K0);
   CPU75112=AddCPU("75112",SwitchTo_75K0);
   CPU75116=AddCPU("75116",SwitchTo_75K0);
   CPU75206=AddCPU("75206",SwitchTo_75K0);
   CPU75208=AddCPU("75208",SwitchTo_75K0);
   CPU75212=AddCPU("75212",SwitchTo_75K0);
   CPU75216=AddCPU("75216",SwitchTo_75K0);
   CPU75512=AddCPU("75512",SwitchTo_75K0);
   CPU75516=AddCPU("75516",SwitchTo_75K0);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_75K0;
END

