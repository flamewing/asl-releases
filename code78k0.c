/* code78k0.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 78K0-Familie                                                */
/*                                                                           */
/* Historie:  1.12.1996 Grundsteinlegung                                     */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code78k0.c,v 1.8 2014/03/08 09:47:43 alfred Exp $                          *
 ***************************************************************************** 
 * $Log: code78k0.c,v $
 * Revision 1.8  2014/03/08 09:47:43  alfred
 * - fix DBNZ
 *
 * Revision 1.7  2010/08/27 14:52:42  alfred
 * - some more overlapping strcpy() cleanups
 *
 * Revision 1.6  2010/04/17 13:14:22  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.5  2007/11/24 22:48:05  alfred
 * - some NetBSD changes
 *
 * Revision 1.4  2006/12/09 18:01:34  alfred
 * - correct some coding errors
 *
 * Revision 1.3  2005/09/08 16:53:42  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2004/05/29 11:33:01  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:21  alfred
 * - recreated
 *
 * Revision 1.2  2003/10/08 20:57:52  alfred
 * - transformed to hash table
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"
#include "intpseudo.h"
#include "codevars.h"


#define ModNone (-1)
#define ModReg8 0
#define MModReg8 (1 << ModReg8)
#define ModReg16 1
#define MModReg16 (1 << ModReg16)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModShort 3
#define MModShort (1 << ModShort)
#define ModSFR 4
#define MModSFR (1 << ModSFR)
#define ModAbs 5
#define MModAbs (1 << ModAbs)
#define ModIReg 6
#define MModIReg (1 << ModIReg)
#define ModIndex 7
#define MModIndex (1 << ModIndex)
#define ModDisp 8
#define MModDisp (1 << ModDisp)

#define AccReg 1
#define AccReg16 0

#define FixedOrderCount 11
#define AriOrderCount 8
#define Ari16OrderCount 3
#define ShiftOrderCount 4
#define Bit2OrderCount 3
#define RelOrderCount 4
#define BRelOrderCount 3

typedef struct
         {
          Word Code;
         } FixedOrder;

static FixedOrder *FixedOrders;

static Byte OpSize,AdrPart;
static Byte AdrVals[2];
static ShortInt AdrMode;

static CPUVar CPU78070;


/*-------------------------------------------------------------------------*/
/* Adressausdruck parsen */

        static void ChkAdr(Word Mask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask & (1 << AdrMode))==0))
    BEGIN
     WrError(1350);
     AdrMode=ModNone; AdrCnt=0;
    END
END

        static void DecodeAdr(char *Asc, Word Mask)
BEGIN
   static char *RegNames[8]={"X","A","C","B","E","D","L","H"};

   Word AdrWord;
   int z;
   Boolean OK,LongFlag;

   AdrMode=ModNone; AdrCnt=0;

   /* Register */

   for (z=0; z<8; z++)
    if (strcasecmp(Asc,RegNames[z])==0)
     BEGIN
      AdrMode=ModReg8; AdrPart=z; ChkAdr(Mask); return;
     END

   if (mytoupper(*Asc)=='R')
    BEGIN
     if ((strlen(Asc)==2) AND (Asc[1]>='0') AND (Asc[1]<='7'))
      BEGIN
       AdrMode=ModReg8; AdrPart=Asc[1]-'0'; ChkAdr(Mask); return;
      END
     else if ((strlen(Asc)==3) AND (mytoupper(Asc[1])=='P') AND (Asc[2]>='0') AND (Asc[2]<='3'))
      BEGIN
       AdrMode=ModReg16; AdrPart=Asc[2]-'0'; ChkAdr(Mask); return;
      END
    END

   if (strlen(Asc)==2)
    for (z=0; z<4; z++)
     if ((mytoupper(*Asc)==*RegNames[(z<<1)+1]) AND (mytoupper(Asc[1])==*RegNames[z<<1]))
      BEGIN
       AdrMode=ModReg16; AdrPart=z; ChkAdr(Mask); return;
      END

   /* immediate */

   if (*Asc=='#')
    BEGIN
     switch (OpSize)
      BEGIN
       case 0: 
        AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
        break;
       case 1:
        AdrWord=EvalIntExpression(Asc+1,Int16,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
         END
        break;
      END
     if (OK)
      BEGIN
       AdrMode=ModImm; AdrCnt=OpSize+1;
      END
     ChkAdr(Mask); return;
    END

   /* indirekt */

   if ((*Asc=='[') AND (Asc[strlen(Asc)-1]==']'))
    BEGIN
     Asc++; Asc[strlen(Asc)-1]='\0';

     if ((strcasecmp(Asc,"DE")==0) OR (strcasecmp(Asc,"RP2")==0))
      BEGIN
       AdrMode=ModIReg; AdrPart=0;
      END
     else if ((strncasecmp(Asc,"HL",2)!=0) AND (strncasecmp(Asc,"RP3",3)!=0)) WrXError(1445,Asc);
     else
      BEGIN
       Asc += 2; if (*Asc == '3') Asc++;
       if ((strcasecmp(Asc,"+B")==0) OR (strcasecmp(Asc,"+R3")==0))
        BEGIN
         AdrMode=ModIndex; AdrPart=1;
        END
       else if ((strcasecmp(Asc,"+C")==0) OR (strcasecmp(Asc,"+R2")==0))
        BEGIN
         AdrMode=ModIndex; AdrPart=0;
        END
       else
        BEGIN
         AdrVals[0]=EvalIntExpression(Asc,UInt8,&OK);
         if (OK)
          BEGIN
           if (AdrVals[0]==0)
            BEGIN
             AdrMode=ModIReg; AdrPart=1;
            END
           else
            BEGIN
             AdrMode=ModDisp; AdrCnt=1;
            END
          END
        END
      END

     ChkAdr(Mask); return;
    END

   /* erzwungen lang ? */

   if (*Asc=='!')
    BEGIN
     LongFlag=True; Asc++;
    END
   else LongFlag=False;

   /* -->absolut */

   FirstPassUnknown=True;
   AdrWord=EvalIntExpression(Asc,UInt16,&OK);
   if (FirstPassUnknown)
    BEGIN
     AdrWord&=0xffffe;
     if ((Mask & MModAbs)==0)
      AdrWord=(AdrWord | 0xff00) & 0xff1f;
    END
   if (OK)
    BEGIN
     if ((NOT LongFlag) AND ((Mask & MModShort)!=0) AND (AdrWord>=0xfe20) AND (AdrWord<=0xff1f))
      BEGIN
       AdrMode=ModShort; AdrCnt=1; AdrVals[0]=Lo(AdrWord);
      END
     else if ((NOT LongFlag) AND ((Mask & MModSFR)!=0) AND (((AdrWord>=0xff00) AND (AdrWord<=0xffcf)) OR (AdrWord>=0xffe0)))
      BEGIN
       AdrMode=ModSFR; AdrCnt=1; AdrVals[0]=Lo(AdrWord);
      END
     else
      BEGIN
       AdrMode=ModAbs; AdrCnt=2; AdrVals[0]=Lo(AdrWord); AdrVals[1]=Hi(AdrWord);
      END
    END

   ChkAdr(Mask);
END

        static void ChkEven(void)
BEGIN
   if ((AdrMode==ModAbs) OR (AdrMode==ModShort) OR (AdrMode==ModSFR))
    if ((AdrVals[0]&1)==1) WrError(180);
END

        static Boolean DecodeBitAdr(char *Asc, Byte *Erg)
BEGIN
   char *p;
   Boolean OK;

   p=RQuotPos(Asc,'.');
   if (p==Nil)
    BEGIN
     WrError(1510); return False;
    END

   *p='\0';
   *Erg=EvalIntExpression(p+1,UInt3,&OK) << 4;
   if (NOT OK) return False;

   DecodeAdr(Asc,MModShort+MModSFR+MModIReg+MModReg8);
   switch (AdrMode)
    BEGIN
     case ModReg8:
      if (AdrPart!=AccReg)
       BEGIN
        WrError(1350); return False;
       END
      else
       BEGIN
        *Erg+=0x88; return True;
       END
     case ModShort:
      return True;
     case ModSFR:
      *Erg+=0x08; return True;
     case ModIReg:
      if (AdrPart==0)
       BEGIN
        WrError(1350); return False;
       END
      else
       BEGIN
        *Erg+=0x80; return True;
       END
     default:
      return False;
    END
END

/*-------------------------------------------------------------------------*/
/* Instruction Decoders */

/* ohne Argument */

static void DecodeFixed(Word Index)
{
  FixedOrder *pOrder = FixedOrders + Index;

  if (ArgCnt != 0) WrError(1110);
  else if (Hi(pOrder->Code) == 0) CodeLen = 1;
  else
  {
    BAsmCode[0] = Hi(pOrder->Code); CodeLen = 2;
  }
  BAsmCode[CodeLen - 1] = Lo(pOrder->Code);
}

/* Datentransfer */

static void DecodeMOV(Word Index)
{
  Byte HReg;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModShort | MModSFR | MModAbs
                       | MModIReg | MModIndex | MModDisp);
    switch (AdrMode)
    {
      case ModReg8:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModImm | MModReg8
                           | ((HReg == AccReg) ? MModShort | MModSFR | MModAbs | MModIReg | MModIndex | MModDisp : 0));
        switch (AdrMode)
        {
          case ModReg8:
            if ((HReg == AccReg) == (AdrPart == AccReg)) WrError(1350);
            else if (HReg == AccReg)
            {
              CodeLen = 1; BAsmCode[0] = 0x60 + AdrPart;
            }
            else
            {
              CodeLen = 1; BAsmCode[0] = 0x70 + HReg;
            }
            break;
          case ModImm:
            CodeLen = 2; BAsmCode[0] = 0xa0 + HReg; BAsmCode[1] = AdrVals[0];
            break;
          case ModShort:
            CodeLen = 2; BAsmCode[0] = 0xf0; BAsmCode[1] = AdrVals[0];
            break;
          case ModSFR:
            CodeLen = 2; BAsmCode[0] = 0xf4; BAsmCode[1] = AdrVals[0];
            break;
          case ModAbs:
            CodeLen = 3; BAsmCode[0] = 0x8e;
            memcpy(BAsmCode + 1, AdrVals, AdrCnt);
            break;
          case ModIReg:
            CodeLen = 1; BAsmCode[0] = 0x85 + (AdrPart << 1);
            break;
          case ModIndex:
            CodeLen = 1; BAsmCode[0] = 0xaa + AdrPart;
            break;
          case ModDisp:
            CodeLen = 2; BAsmCode[0] = 0xae; BAsmCode[1] = AdrVals[0];
            break;
        }
        break;

      case ModShort:
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModReg8 | MModImm);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrPart != AccReg) WrError(1350);
            else
            {
              BAsmCode[0] = 0xf2; CodeLen = 2;
            }
            break;
          case ModImm:
            BAsmCode[0] = 0x11; BAsmCode[2] = AdrVals[0]; CodeLen = 3;
            break;
        }
        break;

      case ModSFR:
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModReg8 | MModImm);
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrPart != AccReg) WrError(1350);
            else
            {
             BAsmCode[0] = 0xf6; CodeLen = 2;
            }
            break;
          case ModImm:
            BAsmCode[0] = 0x13; BAsmCode[2] = AdrVals[0]; CodeLen = 3;
            break;
        }
        break;

      case ModAbs:
        memcpy(BAsmCode + 1, AdrVals, 2);
        DecodeAdr(ArgStr[2], MModReg8);
        if (AdrMode == ModReg8)
        {
          if (AdrPart != AccReg) WrError(1350);
          else
          {
            BAsmCode[0] = 0x9e; CodeLen = 3;
          }
        }
        break;

      case ModIReg:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg8);
        if (AdrMode == ModReg8)
        {
          if (AdrPart != AccReg) WrError(1350);
          else
          {
            BAsmCode[0] = 0x95 | (HReg << 1); CodeLen = 1;
          }
        }
        break;

      case ModIndex:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg8);
        if (AdrMode == ModReg8)
        {
          if (AdrPart != AccReg) WrError(1350);
          else
          {
            BAsmCode[0] = 0xba + HReg; CodeLen = 1;
          }
        }
        break;

      case ModDisp:
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModReg8);
        if (AdrMode == ModReg8)
        {
          if (AdrPart != AccReg) WrError(1350);
          else
          {
            BAsmCode[0] = 0xbe; CodeLen = 2;
          }
        }
        break;
    }
  }
}

static void DecodeXCH(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    if ((strcasecmp(ArgStr[2], "A") == 0) || (strcasecmp(ArgStr[2], "RP1") == 0))
    {
      strcpy(ArgStr[3], ArgStr[1]);
      strcpy(ArgStr[1], ArgStr[2]);
      strcpy(ArgStr[2], ArgStr[3]);
    }
    DecodeAdr(ArgStr[1], MModReg8);
    if (AdrMode != ModNone)
    {
      if (AdrPart != AccReg) WrError(1350);
      else
      {
        DecodeAdr(ArgStr[2], MModReg8 | MModShort | MModSFR | MModAbs
                           | MModIReg | MModIndex | MModDisp);
        switch (AdrMode)
        {
          case ModReg8:
           if (AdrPart == AccReg) WrError(1350);
           else
           {
             BAsmCode[0] = 0x30 + AdrPart; CodeLen = 1;
           }
           break;
          case ModShort:
           BAsmCode[0] = 0x83; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
           break;
          case ModSFR:
           BAsmCode[0] = 0x93; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
           break;
          case ModAbs:
           BAsmCode[0] = 0xce; memcpy(BAsmCode + 1, AdrVals, AdrCnt); CodeLen = 3;
           break;
          case ModIReg:
           BAsmCode[0] = 0x05 + (AdrPart << 1); CodeLen = 1;
           break;
          case ModIndex:
           BAsmCode[0] = 0x31; BAsmCode[1] = 0x8a + AdrPart; CodeLen = 2;
           break;
          case ModDisp:
           BAsmCode[0] = 0xde; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
           break;
        }
      }
    }
  }
}

static void DecodeMOVW(Word Index)
{
  Byte HReg;

  UNUSED(Index);

  OpSize = 1;

  if (ArgCnt!=2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg16 | MModShort | MModSFR | MModAbs);
    switch (AdrMode)
    {
      case ModReg16:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg16 | MModImm
                           | ((HReg == AccReg16) ? MModShort | MModSFR | MModAbs : 0));
        switch (AdrMode)
        {
          case ModReg16:
            if ((HReg == AccReg16) == (AdrPart == AccReg16)) WrError(1350);
            else if (HReg == AccReg16)
            {
              BAsmCode[0] = 0xc0 + (AdrPart << 1); CodeLen = 1;
            }
            else
            {
              BAsmCode[0] = 0xd0 + (HReg << 1); CodeLen = 1;
            }
            break;
          case ModImm:
            BAsmCode[0] = 0x10 + (HReg << 1); memcpy(BAsmCode + 1, AdrVals, 2);
            CodeLen = 3;
            break;
          case ModShort:
            BAsmCode[0] = 0x89; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
            ChkEven();
            break;
          case ModSFR:
            BAsmCode[0] = 0xa9; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
            ChkEven();
            break;
          case ModAbs:
            BAsmCode[0] = 0x02; memcpy(BAsmCode + 1, AdrVals, 2); CodeLen = 3;
            ChkEven();
            break;
        }
        break;

      case ModShort:
        ChkEven();
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModReg16 | MModImm);
        switch (AdrMode)
        {
          case ModReg16:
            if (AdrPart != AccReg16) WrError(1350);
            else
            {
              BAsmCode[0] = 0x99; CodeLen = 2;
            }
            break;
          case ModImm:
            BAsmCode[0] = 0xee; memcpy(BAsmCode + 2, AdrVals, 2); CodeLen = 4;
            break;
        }
        break;

      case ModSFR:
        ChkEven();
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModReg16 | MModImm);
        switch (AdrMode)
        {
          case ModReg16:
            if (AdrPart != AccReg16) WrError(1350);
            else
            {
              BAsmCode[0] = 0xb9; CodeLen = 2;
            }
           break;
          case ModImm:
            BAsmCode[0] = 0xfe; memcpy(BAsmCode + 2,AdrVals, 2); CodeLen = 4;
            break;
        }
        break;

      case ModAbs:
        ChkEven();
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        DecodeAdr(ArgStr[2], MModReg16);
        if (AdrMode == ModReg16)
        {
          if (AdrPart != AccReg16) WrError(1350);
          else
          {
            BAsmCode[0] = 0x03; CodeLen = 3;
          }
        }
        break;
    }
  }
}

static void DecodeXCHW(Word Index)
{
  Byte HReg;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg16);
    if (AdrMode == ModReg16)
    {
      HReg = AdrPart;
      DecodeAdr(ArgStr[2], MModReg16);
      if (AdrMode == ModReg16)
      {
        if ((HReg == AccReg16) == (AdrPart == AccReg16)) WrError(1350);
        else
        {
          BAsmCode[0] = (HReg == AccReg16) ? 0xe0 + (AdrPart << 1) : 0xe0 + (HReg << 1);
          CodeLen = 1;
        }
      }
    }
  }
}

static void DecodeStack(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "PSW"))
  {
    BAsmCode[0] = 0x22 + Index; CodeLen = 1;
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg16);
    if (AdrMode == ModReg16)
    {
      BAsmCode[0] = 0xb1 - Index + (AdrPart << 1); CodeLen = 1;
    }
  }
}

/* Arithmetik */

static void DecodeAri(Word Index)
{
  Byte HReg;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModShort);
    switch (AdrMode)
    {
      case ModReg8:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg8 | ((HReg == AccReg) ? (MModImm | MModShort | MModAbs | MModIReg | MModIndex | MModDisp) : 0));
        switch (AdrMode)
        {
          case ModReg8:
            if (AdrPart == AccReg)
            {
              BAsmCode[0] = 0x61; BAsmCode[1] = (Index << 4) + HReg; CodeLen = 2;
            }
            else if (HReg == AccReg)
            {
              BAsmCode[0] = 0x61; BAsmCode[1] = 0x08 + (Index << 4) + AdrPart; CodeLen = 2;
            }
            else WrError(1350);
            break;
          case ModImm:
            BAsmCode[0] = (Index << 4) + 0x0d; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
            break;
          case ModShort:
            BAsmCode[0] = (Index << 4) + 0x0e; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
            break;
          case ModAbs:
            BAsmCode[0] = (Index << 4) + 0x08; memcpy(BAsmCode + 1, AdrVals, 2); CodeLen = 3;
            break;
          case ModIReg:
            if (AdrPart == 0) WrError(1350);
            else
            {
              BAsmCode[0] = (Index << 4) + 0x0f; CodeLen = 1;
            }
            break;
          case ModIndex:
            BAsmCode[0] = 0x31; BAsmCode[1] = (Index << 4) + 0x0a + AdrPart; CodeLen = 2;
            break;
          case ModDisp:
            BAsmCode[0] = (Index << 4) + 0x09; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
            break;
        }
        break;

      case ModShort:
        BAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[2], MModImm);
        if (AdrMode == ModImm)
        {
          BAsmCode[0] = (Index << 4) + 0x88; BAsmCode[2] = AdrVals[0]; CodeLen = 3;
        }
        break;
    }
  }
}

static void DecodeAri16(Word Index)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    OpSize = 1;
    DecodeAdr(ArgStr[1], MModReg16);
    if (AdrMode == ModReg16)
    {
      DecodeAdr(ArgStr[2], MModImm);
      if (AdrMode == ModImm)
      {
        BAsmCode[0] = 0xca + (Index << 4); memcpy(BAsmCode + 1, AdrVals, 2); CodeLen = 3;
      }
    }
  }
}

static void DecodeMULU(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8);
    if (AdrMode == ModReg8)
    {
      if (AdrPart != 0) WrError(1350);
      else
      {
        BAsmCode[0] = 0x31; BAsmCode[1] = 0x88; CodeLen = 2;
      }
    }
  }
}

static void DecodeDIVUW(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8);
    if (AdrMode == ModReg8)
    {
      if (AdrPart != 2) WrError(1350);
      else
      {
        BAsmCode[0] = 0x31; BAsmCode[1] = 0x82; CodeLen = 2;
      }
    }
  }
}

static void DecodeINCDEC(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 | MModShort);
    switch (AdrMode)
    {
      case ModReg8:
        BAsmCode[0] = 0x40 + AdrPart + Index; CodeLen = 1;
        break;
      case ModShort:
        BAsmCode[0] = 0x81 + Index; BAsmCode[1] = AdrVals[0]; CodeLen = 2;
        break;
    }
  }
}

static void DecodeINCDECW(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg16);
    if (AdrMode == ModReg16)
    {
      BAsmCode[0] = 0x80 + Index + (AdrPart << 1);
      CodeLen = 1;
    }
  }
}

static void DecodeShift(Word Index)
{
  Byte HReg;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8);
    if (AdrMode == ModReg8)
    {
      if (AdrPart != AccReg) WrError(1350);
      else
      {
        HReg = EvalIntExpression(ArgStr[2], UInt1, &OK);
        if (OK)
        {
          if (HReg != 1) WrError(1315);
          else
          {
            BAsmCode[0] = 0x24 + Index; CodeLen = 1;
          }
        }
      }
    }
  }
}

static void DecodeRot4(Word Index)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModIReg);
    if (AdrMode == ModIReg)
    {
      if (AdrPart == 0) WrError(1350);
      else
      {
        BAsmCode[0] = 0x31; BAsmCode[1] = 0x80 + Index;
        CodeLen = 2;
      }
    }
  }
}

/* Bitoperationen */

static void DecodeMOV1(Word Index)
{
  int z;
  Byte HReg;

  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    if (!strcasecmp(ArgStr[2], "CY"))
    {
      strcpy(ArgStr[3], ArgStr[1]);
      strcpy(ArgStr[1], ArgStr[2]);
      strcpy(ArgStr[2], ArgStr[3]);
      z = 1;
    }
    else z = 4;
    if (strcasecmp(ArgStr[1], "CY") != 0) WrError(1350);
    else if (DecodeBitAdr(ArgStr[2], &HReg))
    {
      BAsmCode[0]=0x61 + (Ord((HReg & 0x88) != 0x88) << 4);
      BAsmCode[1]=z + HReg;
      memcpy(BAsmCode + 2, AdrVals, AdrCnt);
      CodeLen = 2 + AdrCnt;
    }
  }
}

static void DecodeBit2(Word Index)
{
  Byte HReg;
 
  if (ArgCnt != 2) WrError(1110);
  else if (strcasecmp(ArgStr[1], "CY")) WrError(1350);
  else if (DecodeBitAdr(ArgStr[2], &HReg))
  {
    BAsmCode[0] = 0x61 + (Ord((HReg & 0x88) != 0x88) << 4);
    BAsmCode[1] = Index + 5 + HReg;
    memcpy(BAsmCode + 2, AdrVals, AdrCnt);
    CodeLen = 2 + AdrCnt;
  }
}

static void DecodeSETCLR1(Word Index)
{
  Byte HReg;
 
  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "CY"))
  {
    BAsmCode[0] = 0x20 + Index; CodeLen = 1;
  }
  else if (DecodeBitAdr(ArgStr[1], &HReg))
  {
    if ((HReg & 0x88) == 0)
    {
      BAsmCode[0] = 0x0a + Index + (HReg & 0x70);
      BAsmCode[1] = AdrVals[0];
      CodeLen = 2;
    }
    else
    {
      BAsmCode[0] =0x61 + (Ord((HReg & 0x88) != 0x88) << 4);
      BAsmCode[1] =HReg + 2 + Index;
      memcpy(BAsmCode + 2, AdrVals, AdrCnt);
      CodeLen=2 + AdrCnt;
    }
  }
}

static void DecodeNOT1(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if (strcasecmp(ArgStr[1], "CY")) WrError(1350);
  else
  {
    BAsmCode[0] = 0x01;
    CodeLen = 1;
  }
}

/* Spruenge */

static void DecodeCALL(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModAbs);
    if (AdrMode == ModAbs)
    {
      BAsmCode[0] = 0x9a; memcpy(BAsmCode + 1, AdrVals, 2); CodeLen = 3;
    }
  }
}

static void DecodeCALLF(Word Index)
{
  Word AdrWord;
  Boolean OK;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    char *pVal = ArgStr[1];

    if ('!' == *pVal) pVal++;
    AdrWord = EvalIntExpression(pVal, UInt11, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x0c | (Hi(AdrWord) << 4);
      BAsmCode[1] = Lo(AdrWord);
      CodeLen = 2;
    }
  }
}

static void DecodeCALLT(Word Index)
{
  Word AdrWord;
  Boolean OK;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if ((*ArgStr[1] != '[') || (ArgStr[1][strlen(ArgStr[1]) - 1] != ']')) WrError(1350);
  else
  {
    FirstPassUnknown = False;
    ArgStr[1][strlen(ArgStr[1]) - 1] = '\0';
    AdrWord = EvalIntExpression(ArgStr[1] + 1, UInt6, &OK);
    if (FirstPassUnknown) AdrWord &= 0xfffe;
    if (OK)
    {
      if (Odd(AdrWord)) WrError(1325);
      else
      {
        BAsmCode[0] = 0xc1 + (AdrWord & 0x3e);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeBR(Word Index)
{
  Word AdrWord;
  Integer AdrInt;
  Boolean OK;
  Byte HReg;

  UNUSED(Index);  

  if (ArgCnt != 1) WrError(1110);
  else if ((!strcasecmp(ArgStr[1], "AX")) || (!strcasecmp(ArgStr[1], "RP0")))
  {
    BAsmCode[0] = 0x31; BAsmCode[1] = 0x98; CodeLen = 2;
  }
  else
  {
    char *pArg1 = ArgStr[1];

    if (*pArg1 == '!') 
    {
      pArg1++; HReg = 1;
    }
    else if (*pArg1 == '$')
    {
      pArg1++; HReg = 2;
    }
    else HReg = 0;
    AdrWord = EvalIntExpression(pArg1, UInt16, &OK);
    if (OK)
    {
      if (HReg == 0)
      {
        AdrInt = AdrWord - (EProgCounter() - 2);
        HReg = ((AdrInt >= -128) && (AdrInt < 127)) ? 2 : 1;
      }
      switch (HReg)
      {
        case 1:
          BAsmCode[0] = 0x9b; BAsmCode[1] = Lo(AdrWord); BAsmCode[2] = Hi(AdrWord);
          CodeLen = 3;
          break;
        case 2:
          AdrInt = AdrWord - (EProgCounter() + 2);
          if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
          else
          {
            BAsmCode[0] = 0xfa; BAsmCode[1] = AdrInt & 0xff; CodeLen = 2;
          }
          break;
      }
    }
  }
}

static void DecodeRel(Word Index)
{
  Integer AdrInt;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    char *pAdr = ArgStr[1];

    if ('$' == *pAdr)
      pAdr++;
    AdrInt = EvalIntExpression(pAdr, UInt16, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
      else
      {
        BAsmCode[0] = 0x8d + (Index << 4); BAsmCode[1] = AdrInt & 0xff;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeBRel(Word Index)  
{
  Integer AdrInt;
  Byte HReg;
  Boolean OK;

  if (ArgCnt != 2) WrError(1110);
  else if (DecodeBitAdr(ArgStr[1], &HReg))
  {
    char *pAdr;

    if ((Index == 1) && ((HReg & 0x88) == 0))
    {
      BAsmCode[0] = 0x8c + HReg; BAsmCode[1] = AdrVals[0]; HReg = 2;
    }
    else
    {
      BAsmCode[0] = 0x31;
      switch (HReg & 0x88)
      {
        case 0x00:
          BAsmCode[1] = 0x00; break;
        case 0x08:
          BAsmCode[1] = 0x04; break;
        case 0x80:
          BAsmCode[1] = 0x84; break;
        case 0x88:
          BAsmCode[1] = 0x0c; break;
      }
      BAsmCode[1] += (HReg & 0x70) + Index + 1;
      BAsmCode[2] = AdrVals[0];
      HReg = 2 + AdrCnt;
    }
    pAdr = (*ArgStr[2] == '$') ? ArgStr[2] + 1 : ArgStr[2];
    AdrInt = EvalIntExpression(pAdr, UInt16, &OK) - (EProgCounter() + HReg + 1);
    if (OK)
    {
      if (((AdrInt < -128) || (AdrInt > 127)) & (!SymbolQuestionable)) WrError(1370);
      else
      {
        BAsmCode[HReg] = AdrInt & 0xff;
        CodeLen = HReg + 1;
      }
    }
  }
}

static void DecodeDBNZ(Word Index)
{
  Integer AdrInt;
  Boolean OK;

  UNUSED(Index);
 
  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg8 + MModShort);
    if ((AdrMode == ModReg8) && ((AdrPart & 6) != 2)) WrError(1350);
    else if (AdrMode != ModNone)
    {
      char *pAdr;
      BAsmCode[0] = (AdrMode == ModReg8) ? 0x88 + AdrPart : 0x04;
      BAsmCode[1] = AdrVals[0];
      pAdr = (*ArgStr[2] == '$') ? ArgStr[2] + 1 : ArgStr[2];
      AdrInt = EvalIntExpression(pAdr, UInt16, &OK) - (EProgCounter() + AdrCnt + 2);
      if (OK)
      {
        if (((AdrInt < -128) || (AdrInt > 127)) && (!SymbolQuestionable)) WrError(1370);
        else
        {
          BAsmCode[AdrCnt + 1] = AdrInt & 0xff;
          CodeLen = AdrCnt + 2;
        }
      }
    }
  }
}

/* Steueranweisungen */

static void DecodeSEL(Word Index)
{
  Byte HReg;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1350);
  else if ((strlen(ArgStr[1]) != 3) || (strncasecmp(ArgStr[1], "RB", 2) != 0)) WrError(1350);
  else
  {
    HReg = ArgStr[1][2] - '0';
    if (ChkRange(HReg, 0, 3))
    {
      BAsmCode[0] = 0x61;
      BAsmCode[1] = 0xd0 + ((HReg & 1) << 3) + ((HReg & 2) << 4);
      CodeLen = 2;
    }
  }
}

/*-------------------------------------------------------------------------*/
/* dynamische Codetabellenverwaltung */

static void AddFixed(char *NewName, Word NewCode)
{
  if (InstrZ >= FixedOrderCount) exit(255);

  FixedOrders[InstrZ].Code = NewCode;
  AddInstTable(InstTable, NewName, InstrZ++, DecodeFixed);
}

static void AddAri(char *NewName)
{
  if (InstrZ >= AriOrderCount) exit(255);
  AddInstTable(InstTable, NewName, InstrZ++, DecodeAri);
}

static void AddAri16(char *NewName)
{
  if (InstrZ >= Ari16OrderCount) exit(255);
  AddInstTable(InstTable, NewName, InstrZ++, DecodeAri16);
}

static void AddShift(char *NewName)
{
  if (InstrZ >= ShiftOrderCount) exit(255);
  AddInstTable(InstTable, NewName, InstrZ++, DecodeShift);
}

static void AddBit2(char *NewName)
{
  if (InstrZ >= Bit2OrderCount) exit(255);
  AddInstTable(InstTable, NewName, InstrZ++, DecodeBit2);
}

static void AddRel(char *NewName)
{
  if (InstrZ >= RelOrderCount) exit(255);
  AddInstTable(InstTable, NewName, InstrZ++, DecodeRel);
}

static void AddBRel(char *NewName)
{
  if (InstrZ >= BRelOrderCount) exit(255);
  AddInstTable(InstTable, NewName, InstrZ++, DecodeBRel);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(201);

  AddInstTable(InstTable, "MOV"  , 0, DecodeMOV);
  AddInstTable(InstTable, "XCH"  , 0, DecodeXCH);
  AddInstTable(InstTable, "MOVW" , 0, DecodeMOVW);
  AddInstTable(InstTable, "XCHW" , 0, DecodeXCHW);
  AddInstTable(InstTable, "PUSH" , 0, DecodeStack);
  AddInstTable(InstTable, "POP"  , 1, DecodeStack);
  AddInstTable(InstTable, "MULU" , 0, DecodeMULU);
  AddInstTable(InstTable, "DIVUW", 0, DecodeDIVUW);
  AddInstTable(InstTable, "INC"  , 0, DecodeINCDEC);
  AddInstTable(InstTable, "DEC"  ,16, DecodeINCDEC);
  AddInstTable(InstTable, "INCW" , 0, DecodeINCDECW);
  AddInstTable(InstTable, "DECW" ,16, DecodeINCDECW);
  AddInstTable(InstTable, "ROL4" , 0, DecodeRot4);
  AddInstTable(InstTable, "ROR4" ,16, DecodeRot4);
  AddInstTable(InstTable, "MOV1" , 0, DecodeMOV1);
  AddInstTable(InstTable, "SET1" , 0, DecodeSETCLR1);
  AddInstTable(InstTable, "CLR1" , 1, DecodeSETCLR1);
  AddInstTable(InstTable, "NOT1" , 1, DecodeNOT1);
  AddInstTable(InstTable, "CALL" , 0, DecodeCALL);
  AddInstTable(InstTable, "CALLF", 0, DecodeCALLF);
  AddInstTable(InstTable, "CALLT", 0, DecodeCALLT);
  AddInstTable(InstTable, "BR"   , 0, DecodeBR);
  AddInstTable(InstTable, "DBNZ" , 0, DecodeDBNZ); 
  AddInstTable(InstTable, "SEL"  , 0, DecodeSEL);

  FixedOrders = (FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCount); InstrZ=0;
  AddFixed("BRK"  , 0x00bf); AddFixed("RET"  , 0x00af);
  AddFixed("RETB" , 0x009f); AddFixed("RETI" , 0x008f);
  AddFixed("HALT" , 0x7110); AddFixed("STOP" , 0x7100);
  AddFixed("NOP"  , 0x0000); AddFixed("EI"   , 0x7a1e);
  AddFixed("DI"   , 0x7b1e); AddFixed("ADJBA", 0x6180);
  AddFixed("ADJBS", 0x6190);

  InstrZ = 0;
  AddAri("ADD" ); AddAri("SUB" ); AddAri("ADDC"); AddAri("SUBC");
  AddAri("CMP" ); AddAri("AND" ); AddAri("OR"  ); AddAri("XOR" );

  InstrZ = 0;
  AddAri16("ADDW"); AddAri16("SUBW"); AddAri16("CMPW");

  InstrZ = 0;
  AddShift("ROR"); AddShift("RORC"); AddShift("ROL"); AddShift("ROLC");

  InstrZ = 0;
  AddBit2("AND1"); AddBit2("OR1"); AddBit2("XOR1");

  InstrZ = 0;
  AddRel("BC"); AddRel("BNC"); AddRel("BZ"); AddRel("BNZ");

  InstrZ = 0;
  AddBRel("BTCLR"); AddBRel("BT"); AddBRel("BF");
}

static void DeinitFields(void)
{
  free(FixedOrders);

  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/

        static void MakeCode_78K0(void)
BEGIN
   CodeLen = 0; DontPrint = False; OpSize = 0;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodeIntelPseudo(False)) return;

   if (!LookupInstTable(InstTable, OpPart))
     WrXError(1200,OpPart);
}

static Boolean IsDef_78K0(void)
{
  return False;
}

static void SwitchFrom_78K0(void)
{
  DeinitFields();
}

static void SwitchTo_78K0(void)
{
  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

  PCSymbol = "PC"; HeaderID = 0x7c; NOPCode = 0x00;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;

  MakeCode = MakeCode_78K0; IsDef = IsDef_78K0;
  SwitchFrom = SwitchFrom_78K0; InitFields();
}

void code78k0_init(void)
{
   CPU78070 = AddCPU("78070", SwitchTo_78K0);
}
