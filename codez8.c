/* codez8.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator Zilog Z8                                                    */
/*                                                                           */
/* Historie:  8.11.1996 Grundsteinlegung                                     */
/*            2. 1.1998 ChkPC ersetzt                                        */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codez8.c,v 1.15 2014/12/05 11:15:29 alfred Exp $                          *
 *****************************************************************************
 * $Log: codez8.c,v $
 * Revision 1.15  2014/12/05 11:15:29  alfred
 * - eliminate AND/OR/NOT
 *
 * Revision 1.14  2014/11/05 15:47:16  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.13  2014/03/08 21:06:37  alfred
 * - rework ASSUME framework
 *
 * Revision 1.12  2012-12-09 12:32:06  alfred
 * - Bld85
 *
 * Revision 1.11  2011-10-20 14:00:40  alfred
 * - SRP handling more graceful on Z8
 *
 * Revision 1.10  2011-08-01 20:01:10  alfred
 * - rework Z8 work register addressing
 *
 * Revision 1.9  2010-12-05 22:58:58  alfred
 * - allow arbitrary RP values on eZ8
 *
 * Revision 1.8  2010/04/17 13:14:24  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.7  2007/11/24 22:48:08  alfred
 * - some NetBSD changes
 *
 * Revision 1.6  2006/08/05 18:06:43  alfred
 * - silence some compiler warnings
 *
 * Revision 1.5  2005/10/02 10:00:46  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.4  2005/09/08 16:53:43  alfred
 * - use common PInstTable
 *
 * Revision 1.3  2004/11/21 20:35:29  alfred
 * - added ATM/LDWX
 *
 * Revision 1.2  2004/05/29 11:33:04  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:24  alfred
 * - recreated
 *
 * Revision 1.12  2003/08/16 17:24:46  alfred
 * - added call to SaveinitProc()
 *
 * Revision 1.11  2003/08/16 16:43:53  alfred
 * - change head id for eZ8, finished addressing modes
 *
 * Revision 1.10  2003/08/13 20:58:26  alfred
 * - added BTJ
 *
 * Revision 1.9  2003/08/12 17:05:16  alfred
 * - added BIT/LEA
 *
 * Revision 1.8  2003/08/11 21:31:08  alfred
 * - ad ModIndRR
 *
 * Revision 1.7  2003/08/11 21:26:42  alfred
 * - a bit more LDX
 *
 * Revision 1.6  2003/08/10 17:33:32  alfred
 * - fixed missing break
 *
 * Revision 1.5  2003/08/10 17:14:40  alfred
 * - so far...
 *
 * Revision 1.4  2003/08/03 19:10:58  alfred
 * - extended register set, register pointer
 *
 * Revision 1.3  2003/08/02 19:05:12  alfred
 * - cleanrd up, changed to hash table, begun with eZ8
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
#include "headids.h"
#include "errmsg.h"

#include "codez8.h"

typedef struct
{
  Byte Code;
} FixedOrder;

typedef struct
{
  Byte Code;
  Boolean Is16;
  Byte Ext;
} ALU1Order;

typedef struct
{
  Byte Code;
  Byte Ext;
} ALU2Order;

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;


#define WorkOfs 0xe0        /* address work regs with 8-bit-addresses */
#define LongWorkOfs 0xee0   /* ditto with 12-bit-addresses */

#define EXTPREFIX 0x1f

#define FixedOrderCnt 13

#define ALU2OrderCnt 11

#define ALU1OrderCnt 15

#define ALUXOrderCnt 11

#define CondCnt 20

#define ModNone  (-1)
#define ModWReg   0
#define MModWReg   (1 << ModWReg)
#define ModReg    1
#define MModReg    (1 << ModReg)
#define ModIWReg  2
#define MModIWReg  (1 << ModIWReg)
#define ModIReg   3
#define MModIReg   (1 << ModIReg)
#define ModImm    4
#define MModImm    (1 << ModImm)
#define ModRReg   5
#define MModRReg   (1 << ModRReg)
#define ModIWRReg  6
#define MModIWRReg  (1 << ModIWRReg)
#define ModInd    7
#define MModInd    (1 << ModInd)
#define ModXReg   8
#define MModXReg   (1 << ModXReg)
#define ModIndRR  9
#define MModIndRR  (1 << ModIndRR)
#define ModWeird 10
#define MModWeird (1 << ModWeird)

static ShortInt AdrType;
static Byte AdrVal,AdrIndex;
static Word AdrWVal;

static FixedOrder *FixedOrders;
static ALU2Order *ALU2Orders;
static ALU2Order *ALUXOrders;
static ALU1Order *ALU1Orders;
static Condition *Conditions;

static int TrueCond;

static CPUVar CPUZ8601, CPUZ8604, CPUZ8608, CPUZ8630, CPUZ8631, CPUeZ8;
static Boolean IsEncore;

static LongInt RPVal;
static IntType RegSpaceType;

#define ASSUMEeZ8Count 1
static ASSUMERec ASSUMEeZ8s[ASSUMEeZ8Count] =
{
  {"RP" , &RPVal , 0, 0xff, 0x100, NULL}
};

/*--------------------------------------------------------------------------*/ 
/* address expression decoding routines */

static Boolean IsWReg(const char *Asc, Byte *Erg)
{
   Boolean Err;
   char *pAlias;

   if (FindRegDef(Asc, &pAlias))
     Asc = pAlias;

   if ((strlen(Asc) < 2) || (mytoupper(*Asc) != 'R')) return False;
   else
   {
     *Erg = ConstLongInt(Asc + 1, &Err, 10);
     if (!Err)
       return False;
     else
       return (*Erg <= 15);
   }
}

static Boolean IsRReg(const char *Asc, Byte *Erg)
{
   Boolean Err;
   char *pAlias;

   if (FindRegDef(Asc, &pAlias))
     Asc = pAlias;

   if ((strlen(Asc) < 3) || (strncasecmp(Asc, "RR", 2) != 0)) return False;
   else
   {
     *Erg = ConstLongInt(Asc + 2, &Err, 10);
     if (!Err)
       return False;
     else
       return (*Erg <= 15);
   }
}

static Boolean CorrMode8(Byte Mask, ShortInt Old, ShortInt New)
{
   if ((AdrType == Old) && ((Mask & (1 << Old)) == 0) && ((Mask & (1 << New)) != 0))
   {
     AdrType = New;
     AdrVal += WorkOfs;
     return True;
   }
   else
     return False;
}

static Boolean CorrMode12(Word Mask, ShortInt Old, ShortInt New)
{
   if ((AdrType == Old) && ((Mask & (1 << Old)) == 0) && ((Mask & (1 << New)) != 0))
   {
     AdrType = New;
     AdrWVal = AdrVal + LongWorkOfs;
     return True;
   }
   else
     return False;
}

static void ChkAdr(Word Mask, Boolean Is16)
{
   if (!Is16)
   {
     CorrMode8(Mask, ModWReg, ModReg);
     CorrMode12(Mask, ModWReg, ModXReg);
     CorrMode8(Mask, ModIWReg, ModIReg);
   }

   if ((AdrType != ModNone) && ((Mask & (1 << AdrType)) == 0))
   {
     WrError(ErrNum_InvAddrMode); AdrType = ModNone;
   }
}     

static Boolean IsWRegAddress(Word Address)
{
  return ((RPVal <= 0xff)
       && (((Address >> 4) & 15) == ((RPVal >> 4) & 15))
       && (((Address >> 8) & 15) == ((RPVal >> 0) & 15)));
}

static Boolean IsRegAddress(Word Address)
{
  /* simple Z8 does not support 12 bit register addresses, so
     always force this to TRUE for it */

  if (!IsEncore)
    return TRUE;
  return ((RPVal <= 0xff)
       && (Hi(Address) == (RPVal & 15)));
}

static void DecodeAdr(const tStrComp *pArg, Word Mask, Boolean Is16)
{
   Boolean OK;
   char  *p;
   int z, ForceLen, l;

   if (!IsEncore)
     Mask &= ~(MModXReg | MModIndRR | MModWeird);

   AdrType = ModNone;

   /* immediate ? */

   if (*pArg->Str == '#')
   {
     AdrVal = EvalStrIntExpressionOffs(pArg, 1, Int8, &OK);
     if (OK) AdrType = ModImm;
     ChkAdr(Mask, Is16); return;
   }

   /* Register ? */

   if (IsWReg(pArg->Str, &AdrVal))
   {
     AdrType = ModWReg; ChkAdr(Mask, Is16); return;
   }

   if (IsRReg(pArg->Str, &AdrVal))
   {
     if ((AdrVal & 1) == 1) WrError(ErrNum_MustBeEven);
     else AdrType = ModRReg;
     ChkAdr(Mask, Is16); return;
   }

   /* treat absolute address as register? */

   if (*pArg->Str == '!')
   {
     FirstPassUnknown = FALSE;
     AdrWVal = EvalStrIntExpressionOffs(pArg, 1, UInt16, &OK);
     if (OK)
     {
       if ((!FirstPassUnknown) && (!IsWRegAddress(AdrWVal)))
         WrError(ErrNum_InAccPage);
       AdrType = ModWReg;
       AdrVal = AdrWVal & 15;
       ChkAdr(Mask, Is16);
     }
     return;
   }

   /* indirekte Konstrukte ? */

   if (*pArg->Str == '@')
   {
     tStrComp Comp;

     StrCompRefRight(&Comp, pArg, 1);
     if ((strlen(Comp.Str) >= 6) && (!strncasecmp(Comp.Str, ".RR", 3)) && (IsIndirect(Comp.Str + 3)))
     {
       AdrVal = EvalStrIntExpressionOffs(&Comp, 3, Int8, &OK);
       if (OK)
       {
         AdrType = ModWeird; ChkSpace(SegData);
       }
     }
     else if (IsWReg(Comp.Str, &AdrVal)) AdrType = ModIWReg;
     else if (IsRReg(Comp.Str, &AdrVal))
     {
       if (AdrVal & 1) WrError(ErrNum_MustBeEven);
       else AdrType = ModIWRReg;
     }
     else
     {
       AdrVal = EvalStrIntExpression(&Comp, Int8, &OK);
       if (OK)
       {
         AdrType = ModIReg; ChkSpace(SegData);
       }
     }
     ChkAdr(Mask, Is16); return;
   }

   /* indiziert ? */

   l = strlen(pArg->Str);
   if ((l > 4) && (pArg->Str[l - 1] == ')'))
   {
     tStrComp Left = *pArg;

     p = pArg->Str + l - 1; *p = '\0';
     while ((p >= pArg->Str) && (*p != '(')) p--;
     if (*p != '(') WrError(ErrNum_BrackErr);
     else if (IsWReg(p + 1, &AdrVal))
     {
       *p = '\0';
       Left.Pos.Len = p - pArg->Str;
       AdrIndex = EvalStrIntExpression(&Left, Int8, &OK);
       if (OK)
       {
         AdrType = ModInd; ChkSpace(SegData);
       }
       ChkAdr(Mask, Is16); return;
     }
     else if (IsRReg(p + 1, &AdrVal))
     {
       *p = '\0';
       Left.Pos.Len = p - pArg->Str;
       AdrIndex = EvalStrIntExpression(&Left, Int8, &OK);
       if (OK)
       {
         AdrType = ModIndRR; ChkSpace(SegData);
       }
       ChkAdr(Mask, Is16); return;
     }
     else
       WrXError(ErrNum_InvReg, p + 1);
   }

   /* einfache direkte Adresse ? */

   ForceLen = 0;
   for (z = 0; z < 2; z++)
     if (pArg->Str[ForceLen] == '>')
       ForceLen++;
     else
       break;
   FirstPassUnknown = FALSE;
   AdrWVal = EvalStrIntExpressionOffs(pArg, ForceLen, (Is16) ? UInt16 : RegSpaceType, &OK);
   if (OK)
   {
     if (Is16)
     {
       AdrType = ModReg;
       ChkSpace(SegCode);
     }
     else
     {
       if (FirstPassUnknown && (!(Mask & ModXReg)))
         AdrWVal = Lo(AdrWVal) | ((RPVal & 15) << 8);
       if ((IsWRegAddress(AdrWVal)) && (Mask & MModWReg) && (ForceLen <= 0))
       {
         AdrType = ModWReg;
         AdrVal = AdrWVal & 15;
       }
       else if ((IsRegAddress(AdrWVal)) && (Mask & MModReg) && (ForceLen <= 1))
       {
         AdrType = ModReg;
         AdrVal = Lo(AdrWVal);
       }
       else
         AdrType = ModXReg;
       ChkSpace(SegData);
     }
     ChkAdr(Mask, Is16); return;
   }

   ChkAdr(Mask, Is16); 
}

static int DecodeCond(const tStrComp *pArg)
{
  int z;

  NLS_UpString(pArg->Str);
  for (z = 0; z < CondCnt; z++)
    if (strcmp(Conditions[z].Name, pArg->Str) == 0)
      break;

  if (z >= CondCnt)
    WrStrErrorPos(ErrNum_UndefCond, pArg);

  return z;
}

/*--------------------------------------------------------------------------*/
/* instruction decoders */

static void DecodeFixed(Word Index)
{
  FixedOrder *pOrder = FixedOrders + Index;

  if (!ChkArgCnt(0, 0));
  else
  {
    CodeLen = 1;
    BAsmCode[0] = pOrder->Code;
  }
}

static void DecodeALU2(Word Index)
{
  ALU2Order *pOrder = ALU2Orders + Index;
  Byte Save;
  int l = 0;

  if (ChkArgCnt(2, 2)
   && ((!pOrder->Ext) || ChkMinCPU(CPUeZ8)))
  {
    if (pOrder->Ext)
      BAsmCode[l++] = pOrder->Ext;
    DecodeAdr(&ArgStr[1], MModReg | MModWReg | MModIReg, False);
    switch (AdrType)
    {
      case ModReg:
       Save = AdrVal;
       DecodeAdr(&ArgStr[2], MModReg | MModIReg | MModImm, False);
       switch (AdrType)
       {
         case ModReg:
          BAsmCode[l++] = pOrder->Code + 4;
          BAsmCode[l++] = AdrVal;
          BAsmCode[l++] = Save;
          CodeLen = l;
          break;
         case ModIReg:
          BAsmCode[l++] = pOrder->Code + 5;
          BAsmCode[l++] = AdrVal;
          BAsmCode[l++] = Save;
          CodeLen = l;
          break;
         case ModImm:
          BAsmCode[l++] = pOrder->Code + 6;
          BAsmCode[l++] = Save;
          BAsmCode[l++] = AdrVal;
          CodeLen = l;
          break;
       }
       break;
      case ModWReg:
       Save = AdrVal;
       DecodeAdr(&ArgStr[2], MModWReg| MModReg | MModIWReg | MModIReg | MModImm, False);
       switch (AdrType)
       {
         case ModWReg:
          BAsmCode[l++] = pOrder->Code + 2;
          BAsmCode[l++] = (Save << 4) + AdrVal;
          CodeLen = l;
          break;
         case ModReg:
          BAsmCode[l++] = pOrder->Code + 4;
          BAsmCode[l++] = AdrVal;
          BAsmCode[l++] = WorkOfs + Save;
          CodeLen = l;
          break;
         case ModIWReg:
          BAsmCode[l++] = pOrder->Code + 3;
          BAsmCode[l++] = (Save << 4) + AdrVal;
          CodeLen = l;
          break;
         case ModIReg:
          BAsmCode[l++] = pOrder->Code + 5;
          BAsmCode[l++] = AdrVal;
          BAsmCode[l++] = WorkOfs + Save;
          CodeLen = l;
          break;
         case ModImm:
          BAsmCode[l++] = pOrder->Code + 6;
          BAsmCode[l++] = Save + WorkOfs;
          BAsmCode[l++] = AdrVal;
          CodeLen = l;
          break;
       }
       break;
      case ModIReg:
       Save = AdrVal;
       DecodeAdr(&ArgStr[2], MModImm, False);
       switch (AdrType)
       {
         case ModImm:
          BAsmCode[l++] = pOrder->Code + 7;
          BAsmCode[l++] = Save;
          BAsmCode[l++] = AdrVal;
          CodeLen = l;
          break;
       }
       break;
    }
  }
}

static void DecodeALUX(Word Index)
{
  ALU2Order *pOrder = ALUXOrders + Index;
  int l = 0;

  if (ChkArgCnt(2, 2)
   && ChkMinCPU(CPUeZ8))
  {
    if (pOrder->Ext)
      BAsmCode[l++] = pOrder->Ext;
    DecodeAdr(&ArgStr[1], MModXReg, False);
    if (AdrType == ModXReg)
    {
      BAsmCode[l + 3] = Lo(AdrWVal);
      BAsmCode[l + 2] = Hi(AdrWVal) & 15;
      DecodeAdr(&ArgStr[2], MModXReg | MModImm, False);
      switch (AdrType)
      {
        case ModXReg:
          BAsmCode[l + 0] = pOrder->Code;
          BAsmCode[l + 1] = AdrWVal >> 4;
          BAsmCode[l + 2] |= (AdrWVal & 15) << 4;
          CodeLen = l + 4;
          break;
        case ModImm:
          BAsmCode[l + 0] = pOrder->Code + 1;
          BAsmCode[l + 1] = AdrVal;
          CodeLen = l + 4;
          break;
      }
    }
  }
}

static void DecodeALU1(Word Index)
{
  ALU1Order *pOrder = ALU1Orders + Index;
  int l = 0;

  if (ChkArgCnt(1, 1)
   && ((!pOrder->Ext) || ChkMinCPU(CPUeZ8)))
  {
    if (pOrder->Ext)
      BAsmCode[l++] = pOrder->Ext;
    DecodeAdr(&ArgStr[1], ((pOrder->Is16) ? MModRReg : 0) | MModReg | MModIReg, False);
    switch (AdrType)
    {
      case ModReg:
       BAsmCode[l++] = pOrder->Code;
       BAsmCode[l++] = AdrVal;
       CodeLen = l;
       break;
      case ModRReg:
       BAsmCode[l++] = pOrder->Code;
       BAsmCode[l++] = WorkOfs + AdrVal;
       CodeLen = l;
       break;
      case ModIReg:
       BAsmCode[l++] = pOrder->Code + 1;
       BAsmCode[l++] = AdrVal;
       CodeLen = l;
       break;
    }
  }
}

static void DecodeLD(Word Index)
{
  Word Save;

  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else
  {
    DecodeAdr(&ArgStr[1], MModReg | MModWReg | MModIReg | MModIWReg | MModInd, False);
    switch (AdrType)
    {
     case ModReg:
      Save = AdrVal;
      DecodeAdr(&ArgStr[2], MModReg | MModWReg | MModIReg | MModImm, False);
      switch (AdrType)
      {
       case ModReg:
        BAsmCode[0] = 0xe4;
        BAsmCode[1] = AdrVal;
        BAsmCode[2] = Save;
        CodeLen = 3;
        break;
       case ModWReg:
        if (IsEncore)
        {
          BAsmCode[0] = 0xe4;
          BAsmCode[1] = AdrVal + WorkOfs;
          BAsmCode[2] = Save;
          CodeLen = 3;
        }
        else /** non-eZ8 **/
        {
          BAsmCode[0] = (AdrVal << 4)+9;
          BAsmCode[1] = Save;
          CodeLen = 2;
        }
        break;
       case ModIReg:
        BAsmCode[0] = 0xe5;
        BAsmCode[1] = AdrVal;
        BAsmCode[2] = Save;
        CodeLen = 3;
        break;
       case ModImm:
        BAsmCode[0] = 0xe6;
        BAsmCode[1] = Save;
        BAsmCode[2] = AdrVal;
        CodeLen = 3;
        break;
      }
      break;
     case ModWReg:
      Save = AdrVal;
      DecodeAdr(&ArgStr[2], MModWReg | MModReg | MModIWReg | MModIReg | MModImm | MModInd, False);
      switch (AdrType)
      {
       case ModWReg: 
        if (IsEncore)
        {
          BAsmCode[0] = 0xe4;   
          BAsmCode[1] = AdrVal + WorkOfs;
          BAsmCode[2] = Save + WorkOfs;
          CodeLen = 3;
        }
        else /** non-eZ8 */
        {
          BAsmCode[0] = (Save << 4) + 8;
          BAsmCode[1] = AdrVal + WorkOfs;
          CodeLen = 2;
        }
        break;
       case ModReg: 
        if (IsEncore)
        {
          BAsmCode[0] = 0xe4;
          BAsmCode[1] = AdrVal;
          BAsmCode[2] = Save + WorkOfs;
          CodeLen = 3;
        }
        else /** non-eZ8 **/
        {
          BAsmCode[0] = (Save << 4) + 8;
          BAsmCode[1] = AdrVal;
          CodeLen = 2;
        }
        break;
       case ModIWReg:
        BAsmCode[0] = 0xe3;
        BAsmCode[1] = (Save << 4) + AdrVal;
        CodeLen = 2;
        break;
       case ModIReg:
        BAsmCode[0] = 0xe5;
        BAsmCode[1] = AdrVal;
        BAsmCode[2] = WorkOfs + Save;
        CodeLen = 3;
        break;
       case ModImm:
        BAsmCode[0] = (Save << 4) + 12;
        BAsmCode[1] = AdrVal;
        CodeLen = 2;
        break;
       case ModInd:
        BAsmCode[0] = 0xc7;
        BAsmCode[1] = (Save << 4) + AdrVal;
        BAsmCode[2] = AdrIndex;
        CodeLen = 3;
        break;
      }
      break;
     case ModIReg:
      Save = AdrVal;
      DecodeAdr(&ArgStr[2], MModReg | MModImm, False);
      switch (AdrType)
      {
       case ModReg:
        BAsmCode[0] = 0xf5;
        BAsmCode[1] = AdrVal;
        BAsmCode[2] = Save;
        CodeLen = 3;
        break;
       case ModImm:
        BAsmCode[0] = 0xe7;
        BAsmCode[1] = Save;
        BAsmCode[2] = AdrVal;
        CodeLen = 3;
        break;
      }
      break;
     case ModIWReg:
      Save = AdrVal;
      DecodeAdr(&ArgStr[2], MModWReg | MModReg | MModImm, False);
      switch (AdrType)
      {
       case ModWReg:
        BAsmCode[0] = 0xf3;
        BAsmCode[1] = (Save << 4) + AdrVal;
        CodeLen = 2;
        break;
       case ModReg:
        BAsmCode[0] = 0xf5;
        BAsmCode[1] = AdrVal;
        BAsmCode[2] = WorkOfs + Save;
        CodeLen = 3;
        break;
       case ModImm:
        BAsmCode[0] = 0xe7;
        BAsmCode[1] = WorkOfs + Save;
        BAsmCode[2] = AdrVal;
        CodeLen = 3;
        break;
      }
      break;
     case ModInd:
      Save = AdrVal;
      DecodeAdr(&ArgStr[2], MModWReg, False);
      switch (AdrType)
      {
       case ModWReg:
        BAsmCode[0] = 0xd7;
        BAsmCode[1] = (AdrVal << 4) + Save;
        BAsmCode[2] = AdrIndex;
        CodeLen = 3;
        break;
      }
      break;
    }
  }
}

static void DecodeLDCE(Word Index)
{
  Byte Save;

  if (!ChkArgCnt(2, 2));
  else
  {
    DecodeAdr(&ArgStr[1], MModWReg | MModIWRReg | (((IsEncore) && (Index == 0xc2)) ? MModIWReg : 0), False);
    switch (AdrType)
    {
     case ModWReg:
      Save = AdrVal; DecodeAdr(&ArgStr[2], MModIWRReg, False);
      if (AdrType != ModNone)
      {
        BAsmCode[0] = Index;
        BAsmCode[1] = (Save << 4) | AdrVal;
        CodeLen = 2;
      }
      break;
     case ModIWReg:
      Save = AdrVal; DecodeAdr(&ArgStr[2], MModIWRReg, False);
      if (AdrType != ModNone)
      {
        BAsmCode[0] = 0xc5;
        BAsmCode[1] = (Save << 4) | AdrVal;
        CodeLen = 2;
      }
      break;
     case ModIWRReg:
      Save = AdrVal; DecodeAdr(&ArgStr[2], MModWReg, False);
      if (AdrType != ModNone)
      {
        BAsmCode[0] = Index + 0x10;
        BAsmCode[1] = (AdrVal << 4) | Save;
        CodeLen = 2;
      }
      break;
    }
  }
}

static void DecodeLDCEI(Word Index)
{
  Byte Save;

  if (!ChkArgCnt(2, 2));
  else
  {
    DecodeAdr(&ArgStr[1], MModIWReg | MModIWRReg, False);
    switch (AdrType)
    {
      case ModIWReg:
        Save = AdrVal; DecodeAdr(&ArgStr[2], MModIWRReg, False);
        if (AdrType != ModNone)
        {
          BAsmCode[0] = Index;
          BAsmCode[1] = (Save << 4) + AdrVal;
          CodeLen = 2;
        }
        break;
      case ModIWRReg:
        Save = AdrVal; DecodeAdr(&ArgStr[2], MModIWReg, False);
        if (AdrType != ModNone)
        {
          BAsmCode[0] = Index + 0x10;
          BAsmCode[1] = (AdrVal << 4) + Save;
          CodeLen = 2;
        }
        break;
    }
  }
}

static void DecodeINC(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else
  {
    DecodeAdr(&ArgStr[1], MModWReg | MModReg | MModIReg, False);
    switch (AdrType)
    {
      case ModWReg:
        BAsmCode[0] = (AdrVal << 4) + 0x0e;
        CodeLen = 1;
        break;
      case ModReg:
        BAsmCode[0] = 0x20;
        BAsmCode[1] = AdrVal;
        CodeLen = 2;
        break;
      case ModIReg:
        BAsmCode[0] = 0x21;
        BAsmCode[1] = AdrVal;
        CodeLen = 2;
        break;
    }
  }
}

static void DecodeJR(Word Index)
{
  Integer AdrInt;
  int z;
  Boolean OK;

  UNUSED(Index);

  if (!ChkArgCnt(1, 2));
  else
  {
    z = (ArgCnt == 1) ? TrueCond : DecodeCond(&ArgStr[1]);
    if (z < CondCnt)
    {
      AdrInt = EvalStrIntExpression(&ArgStr[ArgCnt], Int16, &OK) - (EProgCounter() + 2);
      if (OK)
      {
        if ((!SymbolQuestionable)
         && ((AdrInt > 127) || (AdrInt < -128))) WrError(ErrNum_JmpDistTooBig);
        else
        {
          ChkSpace(SegCode);
          BAsmCode[0] = (Conditions[z].Code << 4) + 0x0b;
          BAsmCode[1] = Lo(AdrInt);
          CodeLen = 2;
        }
      }
    }
  }
}

static void DecodeDJNZ(Word Index)
{
  Integer AdrInt;
  Boolean OK;

  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else
  {
    DecodeAdr(&ArgStr[1], MModWReg, False);
    if (AdrType != ModNone)
    {
      AdrInt = EvalStrIntExpression(&ArgStr[2], Int16, &OK) - (EProgCounter() + 2);
      if (OK)
      {
        if ((!SymbolQuestionable)
         && ((AdrInt > 127) || (AdrInt < -128))) WrError(ErrNum_JmpDistTooBig);
        else
        {
          BAsmCode[0] = (AdrVal << 4) + 0x0a;
          BAsmCode[1] = Lo(AdrInt);
          CodeLen = 2;
        }
      }
    }
  }
}

static void DecodeCALL(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else
  {
    DecodeAdr(&ArgStr[1], MModIWRReg | MModIReg | MModReg, True);
    switch (AdrType)
    {
      case ModIWRReg:
        BAsmCode[0] = 0xd4;
        BAsmCode[1] = WorkOfs + AdrVal;
        CodeLen = 2;
        break;
      case ModIReg:
        BAsmCode[0] = 0xd4;
        BAsmCode[1] = AdrVal;
        CodeLen = 2;
        break;
      case ModReg:
        BAsmCode[0] = 0xd6;
        BAsmCode[1] = Hi(AdrWVal);
        BAsmCode[2] = Lo(AdrWVal);
        CodeLen = 3;
        break;
    }
  }
}

static void DecodeJP(Word Index)
{
  int z;

  UNUSED(Index);

  if (!ChkArgCnt(1, 2));
  else
  {
    z = (ArgCnt == 1) ? TrueCond : DecodeCond(&ArgStr[1]);                 
    if (z < CondCnt)
    {
      DecodeAdr(&ArgStr[ArgCnt], MModIWRReg | MModIReg | MModReg, True);
      switch (AdrType)
      {
        case ModIWRReg:
          if (z != TrueCond) WrError(ErrNum_InvAddrMode);
          else
          {
            BAsmCode[0] = IsEncore ? 0xc4 : 0x30;
            BAsmCode[1] = 0xe0 + AdrVal;
            CodeLen = 2;
          }
          break;
        case ModIReg:
          if (z != TrueCond) WrError(ErrNum_InvAddrMode);
          else
          {
            BAsmCode[0] = IsEncore ? 0xc4 : 0x30;
            BAsmCode[1] = AdrVal;
            CodeLen = 2;
          }
          break;
        case ModReg:
          BAsmCode[0] = (Conditions[z].Code << 4) + 0x0d;
          BAsmCode[1] = Hi(AdrWVal);
          BAsmCode[2] = Lo(AdrWVal);
          CodeLen = 3;
          break;
      }
    }
  }
}

static void DecodeSRP(Word Index)
{
  Boolean Valid;

  UNUSED(Index);

  if (!ChkArgCnt(1, 1));
  else
  {
    DecodeAdr(&ArgStr[1], MModImm, False);
    if (AdrType == ModImm)
    {
      if (IsEncore)
        Valid = True;
      else
        Valid = (((AdrVal & 15) == 0) && ((AdrVal <= 0x70) || (AdrVal >= 0xf0)));
      if (!Valid) WrError(ErrNum_InvRegisterPointer);
      BAsmCode[0] = IsEncore ? 0x01 : 0x31;
      BAsmCode[1] = AdrVal;
      CodeLen = 2;
    }
  }
}

static void DecodeStackExt(Word Index)
{
  if (ChkArgCnt(1, 1)
   && ChkMinCPU(CPUeZ8))
  {
    DecodeAdr(&ArgStr[1], MModXReg, False);
    if (AdrType == ModXReg)
    {
      BAsmCode[0] = Index;
      BAsmCode[1] = AdrWVal >> 4;
      BAsmCode[2] = (AdrWVal & 15) << 4;
      CodeLen = 3;
    }
  }
}

static void DecodeTRAP(Word Index)
{
  UNUSED(Index);
 
  if (ChkArgCnt(1, 1)
   && ChkMinCPU(CPUeZ8))
  {
    DecodeAdr(&ArgStr[1], MModImm, False);
    if (AdrType == ModImm)
    {
      BAsmCode[0] = 0xf2;
      BAsmCode[1] = AdrVal;
      CodeLen = 2;
    }
  }
}

static void DecodeBSWAP(Word Index)
{
  UNUSED(Index);
 
  if (ChkArgCnt(1, 1)
   && ChkMinCPU(CPUeZ8))
  {
    DecodeAdr(&ArgStr[1], MModReg, False);
    if (AdrType == ModReg)
    {
      BAsmCode[0] = 0xd5;
      BAsmCode[1] = AdrVal;
      CodeLen = 2;
    }
  }
}

static void DecodeMULT(Word Index)
{
  UNUSED(Index);
 
  if (ChkArgCnt(1, 1)
   && ChkMinCPU(CPUeZ8))
  {
    DecodeAdr(&ArgStr[1], MModRReg | MModReg, False);
    switch (AdrType)
    {
      case ModRReg:
        BAsmCode[0] = 0xf4;
        BAsmCode[1] = AdrVal + WorkOfs;
        CodeLen = 2;
        break;
      case ModReg:
        BAsmCode[0] = 0xf4;
        BAsmCode[1] = AdrVal;
        CodeLen = 2;
        break;
    }
  }
}

static void DecodeLDX(Word Index)
{
  Word Save;

  UNUSED(Index);

  if (ChkArgCnt(2, 2)
   && ChkMinCPU(CPUeZ8))
  {
    DecodeAdr(&ArgStr[1], MModWReg | MModIWReg | MModReg | MModIReg | MModIWRReg | MModIndRR | MModXReg | MModWeird, False);
    switch (AdrType)
    {
      case ModWReg:
        Save = AdrVal;
        DecodeAdr(&ArgStr[2], MModXReg | MModIndRR | MModImm, False);
        switch (AdrType)
        {
          case ModXReg:
            BAsmCode[0] = 0x84;
            BAsmCode[1] = (Save << 4) | Hi(AdrWVal);
            BAsmCode[2] = Lo(AdrWVal);
            CodeLen = 3;
            break;
          case ModIndRR:
            BAsmCode[0] = 0x88;
            BAsmCode[1] = (Save << 4) | AdrVal;
            BAsmCode[2] = AdrIndex;
            CodeLen = 3;
            break;
          case ModImm:
            BAsmCode[0] = 0xe9; 
            BAsmCode[1] = AdrVal;
            BAsmCode[2] = Hi(LongWorkOfs | Save);
            BAsmCode[3] = Lo(LongWorkOfs | Save);
            CodeLen = 4;
            break;
        }
        break;
      case ModIWReg:
        Save = AdrVal;
        DecodeAdr(&ArgStr[2], MModXReg, False);
        switch (AdrType)
        {
          case ModXReg:
            BAsmCode[0] = 0x85;
            BAsmCode[1] = (Save << 4) | AdrVal;
            BAsmCode[2] = AdrIndex;
            CodeLen = 3;
            break;
        }
        break;
      case ModReg:
        Save = AdrVal;
        DecodeAdr(&ArgStr[2], MModIReg, False);
        switch (AdrType)
        {
          case ModIReg:
            BAsmCode[0] = 0x86;
            BAsmCode[1] = AdrVal;
            BAsmCode[2] = Save;
            CodeLen = 3;
            break;
        }
        break;
      case ModIReg:
        Save = AdrVal;
        DecodeAdr(&ArgStr[2], MModReg | MModWeird, False);
        switch (AdrType)
        {
          case ModReg:
            BAsmCode[0] = 0x96;
            BAsmCode[1] = AdrVal;
            BAsmCode[2] = Save;  
            CodeLen = 3;
            break;
          case ModWeird:
            BAsmCode[0] = 0x87;
            BAsmCode[1] = AdrVal;
            BAsmCode[2] = Save;  
            CodeLen = 3;
            break;
        }
        break;
      case ModIWRReg:
        Save = WorkOfs + AdrVal;
        DecodeAdr(&ArgStr[2], MModReg, False);
        switch (AdrType)
        {
          case ModReg:
            BAsmCode[0] = 0x96;
            BAsmCode[1] = AdrVal;
            BAsmCode[2] = Save;  
            CodeLen = 3;
            break;
        }
        break;
      case ModIndRR:
        BAsmCode[2] = AdrIndex;
        Save = AdrVal;
        DecodeAdr(&ArgStr[2], MModWReg, False);
        switch (AdrType)
        {
          case ModWReg:
            BAsmCode[0] = 0x89;
            BAsmCode[1] = (Save << 4) | AdrVal;
            CodeLen = 3;
            break;
        }
        break;
      case ModXReg:
        Save = AdrWVal;
        DecodeAdr(&ArgStr[2], MModWReg | MModIWReg | MModXReg | MModImm, False);
        switch (AdrType)
        {
          case ModWReg:
            BAsmCode[0] = 0x94; 
            BAsmCode[1] = (AdrVal << 4) | (Hi(Save) & 15);
            BAsmCode[2] = Lo(Save);
            CodeLen = 3;
            break;
          case ModIWReg:
            BAsmCode[0] = 0x95; 
            BAsmCode[1] = (AdrVal << 4) | (Hi(Save) & 15);
            BAsmCode[2] = Lo(Save);
            CodeLen = 3;
            break;
          case ModXReg:
            BAsmCode[0] = 0xe8; 
            BAsmCode[1] = AdrWVal >> 4;
            BAsmCode[2] = ((AdrWVal & 15) << 4) | (Hi(Save) & 15);
            BAsmCode[3] = Lo(Save);
            CodeLen = 4;
            break;
          case ModImm:
            BAsmCode[0] = 0xe9; 
            BAsmCode[1] = AdrVal;
            BAsmCode[2] = (Hi(Save) & 15);
            BAsmCode[3] = Lo(Save);
            CodeLen = 4;
            break;
        }
        break;
      case ModWeird:
        Save = AdrVal;
        DecodeAdr(&ArgStr[2], MModIReg, False);
        switch (AdrType)
        {
          case ModIReg:
            BAsmCode[0] = 0x97;
            BAsmCode[1] = AdrVal;
            BAsmCode[2] = Save;
            CodeLen = 3;
            break;
        }
        break;
    }
  }
}

static void DecodeLDWX(Word Index)
{
  UNUSED(Index);

  if (!ChkArgCnt(2, 2));
  else
  {
    DecodeAdr(&ArgStr[1], MModXReg, False);
    switch (AdrType)
    {
      case ModXReg:
        BAsmCode[0] = 0x1f;
        BAsmCode[1] = 0xe8;
        BAsmCode[3] = Hi(AdrWVal);
        BAsmCode[4] = Lo(AdrWVal);
        DecodeAdr(&ArgStr[2], MModXReg, False);
        switch (AdrType)
        {
          case ModXReg:
            BAsmCode[2] = AdrWVal >> 4;
            BAsmCode[3] |= (AdrWVal & 0x0f) << 4;
            CodeLen = 5;
            break;
        }
        break;
    }
  }  
}

static void DecodeLEA(Word Index)
{
  Byte Save;

  UNUSED(Index);

  if (ChkArgCnt(2, 2)
   && ChkMinCPU(CPUeZ8))
  {
    DecodeAdr(&ArgStr[1], MModWReg | MModRReg, False);
    switch (AdrType)
    {
      case ModWReg:
        Save = AdrVal;
        DecodeAdr(&ArgStr[2], MModInd, False);
        switch (AdrType)
        {
          case ModInd:
            BAsmCode[0] = 0x98;
            BAsmCode[1] = (Save << 4) | AdrVal;
            BAsmCode[2] = AdrIndex;
            CodeLen = 3;
            break;
        }
        break;
      case ModRReg:
        Save = AdrVal;
        DecodeAdr(&ArgStr[2], MModIndRR, False);
        switch (AdrType)
        {
          case ModIndRR:
            BAsmCode[0] = 0x99;
            BAsmCode[1] = (Save << 4) | AdrVal;
            BAsmCode[2] = AdrIndex;
            CodeLen = 3;
            break;
        }
        break;
    }
  }
}

static void DecodeBIT(Word Index)
{
  Boolean OK;

  UNUSED(Index);

  if (ChkArgCnt(3, 3)
   && ChkMinCPU(CPUeZ8))
  {
    BAsmCode[1] = EvalStrIntExpression(&ArgStr[1], UInt1, &OK) << 7;
    if (OK)
    {
      BAsmCode[1] |= EvalStrIntExpression(&ArgStr[2], UInt3, &OK) << 4;
      if (OK)
      {
        DecodeAdr(&ArgStr[3], MModWReg, False);
        switch (AdrType)
        {
          case ModWReg:
            BAsmCode[0] = 0xe2;
            BAsmCode[1] |= AdrVal;
            CodeLen = 2;
            break;
        }
      }
    }
  }
}

static void DecodeBit(Word Index)
{
  Boolean OK;

  if (ChkArgCnt(2, 2)
   && ChkMinCPU(CPUeZ8))
  {
    BAsmCode[1] |= EvalStrIntExpression(&ArgStr[1], UInt3, &OK) << 4;
    if (OK)
    {
      DecodeAdr(&ArgStr[2], MModWReg, False);
      switch (AdrType)
      {
        case ModWReg:
          BAsmCode[0] = 0xe2;
          BAsmCode[1] |= Index | AdrVal;
          CodeLen = 2;
          break;
      }
    }
  }
}

static void DecodeBTJ(Word Index)
{
  Boolean OK;
  Integer AdrInt;

  UNUSED(Index);

  if (ChkArgCnt(4, 4)
   && ChkMinCPU(CPUeZ8))
  {
    BAsmCode[1] = EvalStrIntExpression(&ArgStr[1], UInt1, &OK) << 7;
    if (OK)
    {
      BAsmCode[1] |= EvalStrIntExpression(&ArgStr[2], UInt3, &OK) << 4;
      if (OK)
      {
        DecodeAdr(&ArgStr[3], MModWReg | MModIWReg, False);
        switch (AdrType)
        {
          case ModWReg:
            BAsmCode[0] = 0xf6;
            BAsmCode[1] |= AdrVal;
            break;
          case ModIWReg:
            BAsmCode[0] = 0xf7;
            BAsmCode[1] |= AdrVal;
            break;
        }
        if (AdrType != ModNone)
        {
          AdrInt = EvalStrIntExpression(&ArgStr[4], Int16, &OK) - (EProgCounter() + 3);
         if (OK)
         {
           if ((!SymbolQuestionable)
            && ((AdrInt > 127) || (AdrInt < -128))) WrError(ErrNum_JmpDistTooBig);
           else
           {
             ChkSpace(SegCode);
             BAsmCode[2] = Lo(AdrInt);
             CodeLen = 3;
           }
         }
        }
      }
    }
  }
}

static void DecodeBtj(Word Index)
{
  Boolean OK;
  Integer AdrInt;

  UNUSED(Index);

  if (ChkArgCnt(3, 3)
   && ChkMinCPU(CPUeZ8))
  {
    BAsmCode[1] = Index | EvalStrIntExpression(&ArgStr[1], UInt3, &OK) << 4;
    if (OK)
    {
      DecodeAdr(&ArgStr[2], MModWReg | MModIWReg, False);
      switch (AdrType)
      {
        case ModWReg:
          BAsmCode[0] = 0xf6;
          BAsmCode[1] |= AdrVal;
          break;
        case ModIWReg:
          BAsmCode[0] = 0xf7;
          BAsmCode[1] |= AdrVal;
          break;
      }
      if (AdrType != ModNone)
      {
        AdrInt = EvalStrIntExpression(&ArgStr[3], Int16, &OK) - (EProgCounter() + 3);
       if (OK)
       {
         if ((!SymbolQuestionable)
          && ((AdrInt > 127) || (AdrInt < -128))) WrError(ErrNum_JmpDistTooBig);
         else
         {
           ChkSpace(SegCode);
           BAsmCode[2] = Lo(AdrInt);
           CodeLen = 3;
         }
       }
      }
    }
  }
}

/*--------------------------------------------------------------------------*/
/* instruction table buildup/teardown */

static void AddFixed(char *NName, Byte NCode)
{
  if (InstrZ >= FixedOrderCnt)
    exit(255);
  FixedOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddALU2(char *NName, Byte NCode, Byte NExt)
{
  if (InstrZ >= ALU2OrderCnt) 
    exit(255);
  ALU2Orders[InstrZ].Code = NCode;
  ALU2Orders[InstrZ].Ext = NExt;
  AddInstTable(InstTable, NName, InstrZ++, DecodeALU2);
}

static void AddALUX(char *NName, Byte NCode, Byte NExt)
{
  if (InstrZ >= ALUXOrderCnt) 
    exit(255);
  ALUXOrders[InstrZ].Code = NCode;
  ALUXOrders[InstrZ].Ext = NExt;
  AddInstTable(InstTable, NName, InstrZ++, DecodeALUX);
}

static void AddALU1(char *NName, Byte NCode, Boolean NIs, Byte NExt)
{
  if (InstrZ >= ALU1OrderCnt)
    exit(255);
  ALU1Orders[InstrZ].Is16 = NIs;
  ALU1Orders[InstrZ].Code = NCode;
  ALU1Orders[InstrZ].Ext = NExt;
  AddInstTable(InstTable, NName, InstrZ++, DecodeALU1);
}

static void AddCondition(char *NName, Byte NCode)
{
  if (InstrZ >= CondCnt)
    exit(255);
  Conditions[InstrZ].Name = NName;
  Conditions[InstrZ++].Code = NCode;
}
   
static void InitFields(void)
{
  InstTable = CreateInstTable(201);

  FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder) * FixedOrderCnt);
  InstrZ = 0;
  AddFixed("CCF" , 0xef);  AddFixed("DI"  , 0x8f);
  AddFixed("EI"  , 0x9f);  AddFixed("HALT", 0x7f);
  AddFixed("IRET", 0xbf);  AddFixed("NOP" , IsEncore ? 0x0f : 0xff);
  AddFixed("RCF" , 0xcf);  AddFixed("RET" , 0xaf);
  AddFixed("SCF" , 0xdf);  AddFixed("STOP", 0x6f);
  AddFixed("ATM" , 0x2f);  
  if (IsEncore)
    AddFixed("BRK" , 0x00);
  else
    AddFixed("WDH" , 0x4f);
  AddFixed("WDT" , 0x5f);

  ALU2Orders = (ALU2Order *) malloc(sizeof(ALU2Order) * ALU2OrderCnt);
  InstrZ = 0;
  AddALU2("ADD" , 0x00, 0x00);
  AddALU2("ADC" , 0x10, 0x00);
  AddALU2("SUB" , 0x20, 0x00);
  AddALU2("SBC" , 0x30, 0x00);
  AddALU2("OR"  , 0x40, 0x00);
  AddALU2("AND" , 0x50, 0x00);
  AddALU2("TCM" , 0x60, 0x00);
  AddALU2("TM"  , 0x70, 0x00);
  AddALU2("CP"  , 0xa0, 0x00);
  AddALU2("XOR" , 0xb0, 0x00);
  AddALU2("CPC" , 0xa0, EXTPREFIX);

  ALUXOrders = (ALU2Order *) malloc(sizeof(ALU2Order) * ALUXOrderCnt);
  InstrZ = 0;
  AddALUX("ADDX", 0x08, 0x00);
  AddALUX("ADCX", 0x18, 0x00);
  AddALUX("SUBX", 0x28, 0x00);
  AddALUX("SBCX", 0x38, 0x00);
  AddALUX("ORX" , 0x48, 0x00);
  AddALUX("ANDX", 0x58, 0x00);
  AddALUX("TCMX", 0x68, 0x00);
  AddALUX("TMX" , 0x78, 0x00);
  AddALUX("CPX" , 0xa8, 0x00);
  AddALUX("XORX", 0xb8, 0x00);
  AddALUX("CPCX", 0xa8, EXTPREFIX);

  ALU1Orders = (ALU1Order *) malloc(sizeof(ALU1Order) * ALU1OrderCnt);
  InstrZ = 0;
  AddALU1("DEC" , IsEncore ? 0x30 : 0x00, False, 0x00);
  AddALU1("RLC" , 0x10, False, 0x00);
  AddALU1("DA"  , 0x40, False, 0x00);
  AddALU1("POP" , 0x50, False, 0x00);
  AddALU1("COM" , 0x60, False, 0x00);
  AddALU1("PUSH", 0x70, False, 0x00);
  AddALU1("DECW", 0x80, True , 0x00);
  AddALU1("RL"  , 0x90, False, 0x00);
  AddALU1("INCW", 0xa0, True , 0x00);
  AddALU1("CLR" , 0xb0, False, 0x00);
  AddALU1("RRC" , 0xc0, False, 0x00);
  AddALU1("SRA" , 0xd0, False, 0x00);
  AddALU1("RR"  , 0xe0, False, 0x00);
  AddALU1("SWAP", 0xf0, False, 0x00);
  AddALU1("SRL" , 0xc0, False, EXTPREFIX);

  Conditions=(Condition *) malloc(sizeof(Condition)*CondCnt); InstrZ=0;
  AddCondition("F"  , 0); TrueCond=InstrZ; AddCondition("T"  , 8);
  AddCondition("C"  , 7); AddCondition("NC" ,15);
  AddCondition("Z"  , 6); AddCondition("NZ" ,14);
  AddCondition("MI" , 5); AddCondition("PL" ,13);
  AddCondition("OV" , 4); AddCondition("NOV",12);
  AddCondition("EQ" , 6); AddCondition("NE" ,14);
  AddCondition("LT" , 1); AddCondition("GE" , 9);
  AddCondition("LE" , 2); AddCondition("GT" ,10);
  AddCondition("ULT", 7); AddCondition("UGE",15);
  AddCondition("ULE", 3); AddCondition("UGT",11);

  AddInstTable(InstTable, "LD", 0, DecodeLD);
  AddInstTable(InstTable, "LDX", 0, DecodeLDX);
  AddInstTable(InstTable, "LDWX", 0, DecodeLDWX);
  AddInstTable(InstTable, "LDC", 0xc2, DecodeLDCE);
  AddInstTable(InstTable, "LDE", 0x82, DecodeLDCE);
  AddInstTable(InstTable, "LDCI", 0xc3, DecodeLDCEI);
  AddInstTable(InstTable, "LDEI", 0x83, DecodeLDCEI);
  AddInstTable(InstTable, "INC", 0, DecodeINC);
  AddInstTable(InstTable, "JR", 0, DecodeJR);
  AddInstTable(InstTable, "JP", 0, DecodeJP);
  AddInstTable(InstTable, "CALL", 0, DecodeCALL);
  AddInstTable(InstTable, "SRP", 0, DecodeSRP);
  AddInstTable(InstTable, "DJNZ", 0, DecodeDJNZ);
  AddInstTable(InstTable, "LEA", 0, DecodeLEA);
  
  AddInstTable(InstTable, "POPX" , 0xd8, DecodeStackExt);
  AddInstTable(InstTable, "PUSHX", 0xc8, DecodeStackExt);
  AddInstTable(InstTable, "TRAP" , 0, DecodeTRAP);
  AddInstTable(InstTable, "BSWAP", 0, DecodeBSWAP);
  AddInstTable(InstTable, "MULT" , 0, DecodeMULT);
  AddInstTable(InstTable, "BIT", 0, DecodeBIT);
  AddInstTable(InstTable, "BCLR", 0x00, DecodeBit);
  AddInstTable(InstTable, "BSET", 0x80, DecodeBit);
  AddInstTable(InstTable, "BTJ", 0, DecodeBTJ);
  AddInstTable(InstTable, "BTJZ", 0x00, DecodeBtj);
  AddInstTable(InstTable, "BTJNZ", 0x80, DecodeBtj);
}

static void DeinitFields(void)
{
  free(FixedOrders);
  free(ALU2Orders);
  free(ALU1Orders);
  free(ALUXOrders);
  free(Conditions);

  DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------*/

static Boolean DecodePseudo(void)
{
  if (Memo("SFR"))
  {
    CodeEquate(SegData, 0, 0xff);
    return True;
  }

  if (Memo("REG"))
  {
    if (!ChkArgCnt(1, 1));
    else AddRegDef(LabPart.Str, ArgStr[1].Str);
    return True;
  }

  return False;
}

static void MakeCode_Z8(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Pseudoanweisungen */

  if (DecodePseudo()) return;

  if (DecodeIntelPseudo(True))
    return;

  if (!LookupInstTable(InstTable, OpPart.Str))
    WrStrErrorPos(ErrNum_UnknownOpcode, &OpPart);
}

static void InitCode_Z8(void)
{
  RPVal = 0;
}

static Boolean IsDef_Z8(void)
{
   return ((Memo("SFR")) || (Memo("REG")));
}

static void SwitchFrom_Z8(void)
{
   DeinitFields();
}

static void SwitchTo_Z8(void)
{
  PFamilyDescr pDescr;

  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

  IsEncore = (MomCPU == CPUeZ8);

  pDescr = FindFamilyByName(IsEncore ? "eZ8" : "Z8");

  PCSymbol = "$"; HeaderID = pDescr->Id;
  NOPCode = IsEncore ? 0x0f : 0xff;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData);
  Grans[SegCode] = 1; ListGrans[SegCode] = 1; SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xffff;
  Grans[SegData] = 1; ListGrans[SegData] = 1; SegInits[SegData] = 0;
  if (IsEncore)
  {
    RegSpaceType = UInt12;
    SegLimits[SegData] = 0xfff;
    ValidSegs |= 1 << SegXData;
    Grans[SegXData] = 1; ListGrans[SegXData] = 1; SegInits[SegXData] = 0;
    SegLimits[SegXData] = 0xffff;
  }
  else
  {
    RegSpaceType = UInt8;
    SegLimits[SegData] = 0xff; 
  }

  pASSUMERecs = ASSUMEeZ8s;
  ASSUMERecCnt = ASSUMEeZ8Count;

  MakeCode = MakeCode_Z8;
  IsDef = IsDef_Z8;
  SwitchFrom = SwitchFrom_Z8;
  InitFields();
}

void codez8_init(void)
{
  CPUZ8601 = AddCPU("Z8601", SwitchTo_Z8);
  CPUZ8604 = AddCPU("Z8604", SwitchTo_Z8);
  CPUZ8608 = AddCPU("Z8608", SwitchTo_Z8);
  CPUZ8630 = AddCPU("Z8630", SwitchTo_Z8);
  CPUZ8631 = AddCPU("Z8631", SwitchTo_Z8);
  CPUeZ8 = AddCPU("eZ8", SwitchTo_Z8);

  AddInitPassProc(InitCode_Z8);
}
