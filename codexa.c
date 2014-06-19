/* codexa.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* AS-Codegenerator Philips XA                                               */
/*                                                                           */
/* Historie: 25.10.1996 Grundsteinlegung                                     */
/*           19. 8.1998 autom. Verlaengerung Spruenge                        */
/*           23. 8.1998 Umbau auf Hash-Tabelle                               */
/*                      lange Spruenge fuer JBC                              */
/*           24. 8.1998 lange Spruenge fuer DJNZ CJNE                        */
/*           25. 8.1998 nach Hashing ueberfluessige Namensfelder entfernt    */
/*           14.10.1998 BRANCHEXT auch fuer BR-Befehle                       */
/*                      Padding-Byte auch fuer Sprung auf sich selber        */
/*                      Das $ zu verschieben ist aber noch etwas tricky...   */
/*            9. 1.1999 ChkPC jetzt mit Adresse als Parameter                */
/*           20. 1.1999 Formate maschinenunabhaengig gemacht                 */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*                                                                           */
/*****************************************************************************/
/* $Id: codexa.c,v 1.9 2014/06/15 09:18:09 alfred Exp $                      */
/*****************************************************************************
 * $Log: codexa.c,v $
 * Revision 1.9  2014/06/15 09:18:09  alfred
 * - optimize IsDef a bit
 *
 * Revision 1.8  2014/06/08 14:47:46  alfred
 * - update to current style
 *
 * Revision 1.7  2014/03/08 21:06:37  alfred
 * - rework ASSUME framework
 *
 * Revision 1.6  2010/04/17 13:14:24  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.5  2007/11/24 22:48:07  alfred
 * - some NetBSD changes
 *
 * Revision 1.4  2005/09/08 16:53:43  alfred
 * - use common PInstTable
 *
 * Revision 1.3  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 * Revision 1.2  2004/05/29 11:33:04  alfred
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
#include "asmallg.h"
#include "asmitree.h"
#include "codepseudo.h" 
#include "intpseudo.h"
#include "motpseudo.h"
#include "codevars.h"

#include "codexa.h"

/*-------------------------------------------------------------------------*/
/* Definitionen */

#ifdef __STDC__
# define Even32Mask 0xfffffffeu
#else
# define Even32Mask 0xfffffffe
#endif

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModMem 1
#define MModMem (1 << ModMem)
#define ModImm 2
#define MModImm (1 << ModImm)
#define ModAbs 3
#define MModAbs (1 << ModAbs)

#define JBitOrderCnt 3
#define RegOrderCnt 4
#define RelOrderCount 17

#define RETICode 0xd690

typedef struct
{   
  Byte SizeMask;
  Byte Code;
} RegOrder;

typedef struct
{
  char *Name;
  Byte SizeMask;
  Byte Code;
  Byte Inversion;
} InvOrder;

static CPUVar CPUXAG1,CPUXAG2,CPUXAG3;

static InvOrder *JBitOrders;
static RegOrder *RegOrders;
static InvOrder *RelOrders;

static LongInt Reg_DS;
static SimpProc SaveInitProc;

static ShortInt AdrMode;
static Byte AdrPart,MemPart;
static Byte AdrVals[4];
static ShortInt OpSize;

#define ASSUMEXACount 1
static ASSUMERec ASSUMEXAs[ASSUMEXACount] =
{
  {"DS", &Reg_DS, 0, 0xff, 0x100}
};

/*-------------------------------------------------------------------------*/
/* Hilfsroutinen */

static void SetOpSize(ShortInt NSize)
{
  if (OpSize == -1) OpSize = NSize;
  else if (OpSize != NSize)
  {
    AdrMode = ModNone; AdrCnt = 0; WrError(1131);
  }
}

static Boolean DecodeReg(char *Asc, ShortInt *NSize, Byte *Erg)
{
  int len = strlen(Asc);

  if (!strcasecmp(Asc, "SP"))
  {
    *Erg = 7; *NSize = 1; return True;
  }
  else if ((len >= 2) && (mytoupper(*Asc) == 'R') && (Asc[1] >= '0') && (Asc[1] <= '7'))
  {
    if (len == 2)
    {
      *Erg = Asc[1] - '0';
      if (OpSize == 2)
      {
        if ((*Erg & 1) == 1)
        {
          WrError(1760); (*Erg)--;
        }
        *NSize = 2;
        return True;
      }
      else
      {
        *NSize = 1;
        return True;
      }
    }
    else if ((len == 3) && (mytoupper(Asc[2]) == 'L'))
    {
      *Erg = (Asc[1] - '0') << 1; *NSize = 0; return True;
    }
    else if ((len == 3) && (mytoupper(Asc[2]) == 'H'))
    {
      *Erg = ((Asc[1] - '0') << 1) + 1; *NSize = 0; return True;
    }
    else return False;
  }
  return False;
}

static void ChkAdr(Word Mask)
{
  if ((AdrMode != ModNone) && (!(Mask & (1 << AdrMode))))
  {
    WrError(1350); AdrMode = ModNone; AdrCnt = 0;
  }
}

static void DecodeAdr(char *Asc, Word Mask)
{
  ShortInt NSize;
  LongInt DispAcc, DispPart, AdrLong;
  Boolean FirstFlag, NegFlag, NextFlag, ErrFlag, OK;
  char *PPos, *MPos;
  Word AdrInt;
  Byte Reg;

  AdrMode = ModNone; AdrCnt = 0; KillBlanks(Asc);

  if (DecodeReg(Asc, &NSize, &AdrPart))
  {
    if ((Mask & MModReg) != 0)
    {
      AdrMode = ModReg; SetOpSize(NSize);
    }
    else
    {
      AdrMode = ModMem; MemPart = 1; SetOpSize(NSize);
    }
    ChkAdr(Mask); return;
  }

  if (*Asc == '#')
  {
    switch (OpSize)
    {
      case -4:
        AdrVals[0] = EvalIntExpression(Asc + 1, UInt5, &OK);
        if (OK)
        {
          AdrCnt = 1; AdrMode = ModImm;
        }
        break;
      case -3:
        AdrVals[0] = EvalIntExpression(Asc + 1, SInt4, &OK);
        if (OK)
        {
          AdrCnt = 1; AdrMode = ModImm;
        }
        break;
      case -2:
        AdrVals[0] = EvalIntExpression(Asc + 1, UInt4, &OK);
        if (OK)
        {
          AdrCnt = 1; AdrMode = ModImm;
        }
        break;
      case -1:
        WrError(1132);
        break;
      case 0:
        AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
        if (OK)
        {
          AdrCnt = 1; AdrMode = ModImm;
        }
        break;
      case 1:
        AdrInt = EvalIntExpression(Asc + 1, Int16, &OK);
        if (OK)
        {
          AdrVals[0] = Hi(AdrInt);
          AdrVals[1] = Lo(AdrInt);
          AdrCnt = 2;
          AdrMode = ModImm;
        }
        break;
      case 2:
        AdrLong = EvalIntExpression(Asc + 1, Int32, &OK);
        if (OK)
        {
          AdrVals[0] = (AdrLong >> 24) & 0xff;
          AdrVals[1] = (AdrLong >> 16) & 0xff;
          AdrVals[2] = (AdrLong >> 8) & 0xff;
          AdrVals[3] = AdrLong & 0xff;
          AdrCnt = 4;
          AdrMode = ModImm;
        }
        break;
    }
    ChkAdr(Mask); return;
  }

  if ((*Asc == '[') && (Asc[strlen(Asc) - 1] == ']'))
  {
    Asc++; Asc[strlen(Asc) - 1] = '\0';
    if (Asc[strlen(Asc) - 1] == '+')
    {
      Asc[strlen(Asc) - 1] = '\0';
      if (!DecodeReg(Asc, &NSize, &AdrPart)) WrXError(1445, Asc);
      else if (NSize != 1) WrError(1350);
      else
      {
        AdrMode = ModMem; MemPart = 3;
      }
    }
    else
    {
      char *pRemAsc, *pNextAsc;

      FirstFlag = False; ErrFlag = False;
      DispAcc = 0; AdrPart = 0xff; NegFlag = False;
      pRemAsc = Asc;
      while ((*pRemAsc != '\0') && (!ErrFlag))
      {
        PPos = QuotPos(pRemAsc, '+'); MPos = QuotPos(pRemAsc, '-');
        if (!PPos) PPos = MPos;
        else if ((MPos) && (PPos > MPos)) PPos = MPos;
        NextFlag = ((PPos) && (*PPos == '-'));
        if (!PPos)
        {
          pNextAsc = pRemAsc + strlen(pRemAsc);
        }
        else
        {
          pNextAsc = PPos + 1; *PPos = '\0';
        }
        if (DecodeReg(pRemAsc, &NSize, &Reg))
        {
          if ((NSize != 1) || (AdrPart != 0xff) || (NegFlag))
          {
            WrError(1350); ErrFlag = True;
          }
          else
            AdrPart = Reg;
        }
        else
        {
          FirstPassUnknown = False;
          DispPart = EvalIntExpression(pRemAsc, Int32, &ErrFlag);
          ErrFlag = !ErrFlag;
          if (!ErrFlag)
          {
            FirstFlag = FirstFlag || FirstPassUnknown;
            if (NegFlag) DispAcc -= DispPart;
            else DispAcc += DispPart;
          }
        }
        NegFlag = NextFlag;
        pRemAsc = pNextAsc;
      }
      if (FirstFlag) DispAcc &= 0x7fff;
      if (AdrPart == 0xff) WrError(1350);
      else if (DispAcc == 0)
      {
        AdrMode = ModMem; MemPart = 2;
      }
      else if ((DispAcc >= -128) && (DispAcc < 127))
      {
        AdrMode = ModMem; MemPart = 4;
        AdrVals[0] = DispAcc & 0xff; AdrCnt = 1;
      }
      else if (ChkRange(DispAcc, -0x8000l, 0x7fffl))
      {
        AdrMode = ModMem; MemPart = 5;
        AdrVals[0] = (DispAcc >> 8) & 0xff;
        AdrVals[1] = DispAcc & 0xff;
        AdrCnt = 2;
      }
    }
    ChkAdr(Mask); return;
  }

  FirstPassUnknown = False;
  AdrLong = EvalIntExpression(Asc, UInt24, &OK);
  if (OK)
  {
    if (FirstPassUnknown)
    {
      if (!(Mask & MModAbs)) AdrLong &= 0x3ff;
    }
    if ((AdrLong & 0xffff) > 0x7ff) WrError(1925);
    else if ((AdrLong & 0xffff) <= 0x3ff)
    {
      if ((AdrLong >> 16) != Reg_DS) WrError(110);
      ChkSpace(SegData);
      AdrMode = ModMem; MemPart = 6;
      AdrPart = Hi(AdrLong);
      AdrVals[0] = Lo(AdrLong);
      AdrCnt = 1;
    }
    else if (AdrLong > 0x7ff) WrError(1925);
    else
    {
      ChkSpace(SegIO);
      AdrMode = ModMem; MemPart = 6;
      AdrPart = Hi(AdrLong);
      AdrVals[0] = Lo(AdrLong);
      AdrCnt = 1;
    }
  }

  ChkAdr(Mask);
}

static Boolean DecodeBitAddr(char *Asc, LongInt *Erg)
{
  char *p;
  Byte BPos, Reg;
  ShortInt Size, Res;
  LongInt AdrLong;
  Boolean OK;

  p = RQuotPos(Asc, '.'); Res = 0;
  if (!p)
  {
    FirstPassUnknown = False;
    AdrLong = EvalIntExpression(Asc, UInt24, &OK);
    if (FirstPassUnknown) AdrLong &= 0x3ff;
    *Erg = AdrLong; Res = 1;
  }
  else
  {
    FirstPassUnknown = False; *p = '\0';
    BPos = EvalIntExpression(p + 1, UInt4, &OK);
    if (FirstPassUnknown) BPos &= 7;
    if (OK)
    {
      if (DecodeReg(Asc, &Size, &Reg))
      {
        if ((Size == 0) && (BPos > 7)) WrError(1320);
        else
        {
          if (Size == 0) *Erg = (Reg << 3) + BPos;
          else *Erg = (Reg << 4) + BPos;
          Res = 1;
        }
      }
      else if (BPos > 7) WrError(1320);
      else
      {
        FirstPassUnknown = False;
        AdrLong = EvalIntExpression(Asc, UInt24, &OK);
        if ((TypeFlag & (1 << SegIO)) != 0)
        {
          ChkSpace(SegIO);
          if (FirstPassUnknown) AdrLong = (AdrLong & 0x3f) | 0x400;
          if (ChkRange(AdrLong, 0x400, 0x43f))
          {
            *Erg = 0x200 + ((AdrLong & 0x3f) << 3) + BPos;
            Res = 1;
          }
          else
            Res = -1;
        }
        else
        {
          ChkSpace(SegData);
          if (FirstPassUnknown) AdrLong = (AdrLong & 0x00ff003f) | 0x20;
          if (ChkRange(AdrLong & 0xff, 0x20, 0x3f))
          {
            *Erg = 0x100 + ((AdrLong & 0x1f) << 3) + BPos + (AdrLong & 0xff0000);
            Res = 1;
          }
          else
            Res = -1;
        }
      }
    }
    *p = '.';
  }
  if (Res == 0) WrError(1350);
  return (Res == 1);
}

static void ChkBitPage(LongInt Adr)
{
  if ((Adr >> 16) != Reg_DS) WrError(110);
}

/*-------------------------------------------------------------------------*/
/* Befehlsdekoder */

static void DecodePORT(Word Index)
{
  UNUSED(Index);

  CodeEquate(SegIO, 0x400, 0x7ff);
}

static void DecodeBIT(Word Index)
{
  LongInt BAdr;

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (DecodeBitAddr(ArgStr[1], &BAdr))
  {
    EnterIntSymbol(LabPart, BAdr, SegNone, False);
    switch ((BAdr & 0x3ff) >> 8)
    {
      case 0:
        sprintf(ListLine, "=R%d.%d", (int)((BAdr >> 4) & 15),
                (int) (BAdr & 15));
        break;
      case 1:
        sprintf(ListLine, "=%x:%x.%d", (int)((BAdr >> 16) & 255),
                (int)((BAdr & 0x1f8) >> 3), (int)(BAdr & 7));
        break;
      default:
        sprintf(ListLine, "=S:%x.%d", (int)(((BAdr >> 3) & 0x3f)+0x400),
                (int)(BAdr & 7));
        break;
    }
  }
}

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else
  {
    if (Hi(Code) != 0) BAsmCode[CodeLen++] = Hi(Code);
    BAsmCode[CodeLen++] = Lo(Code);
    if ((Code == RETICode) && (!SupAllowed)) WrError(50);
  }
}

static void DecodeStack(Word Code)
{
  Byte HReg;
  Boolean OK;
  Word Mask;
  int i;

  if (ArgCnt < 1) WrError(1110);
  else
  {
    HReg = 0xff; OK = True; Mask = 0;
    for (i = 1; i <= ArgCnt; i++)
      if (OK)
      {
        DecodeAdr(ArgStr[i], MModMem);
        if (AdrMode == ModNone) OK = False;
        else switch (MemPart)
        {
          case 1:
            if (HReg == 0)
            {
              WrError(1350); OK = False;
            }
            else
            {
              HReg = 1; Mask |= (1 << AdrPart);
            }
            break;
          case 6:
            if (HReg != 0xff)
            {
              WrError(1350); OK = False;
            }
            else
              HReg = 0;
            break;
          default:
            WrError(1350); OK = False;
        }
      }
    if (OK)
    {
      if (OpSize == -1) WrError(1132);
      else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
      else if (HReg == 0)
      {
        BAsmCode[CodeLen++] = 0x87 + (OpSize << 3);
        BAsmCode[CodeLen++] = Hi(Code) + AdrPart;
        BAsmCode[CodeLen++] = AdrVals[0];
      }
      else if (Code < 0x2000)  /* POP(U): obere Register zuerst */
      {
        if (Hi(Mask) != 0)
        {
          BAsmCode[CodeLen++] = Lo(Code) + (OpSize << 3) + 0x40;
          BAsmCode[CodeLen++]=Hi(Mask);
        }
        if (Lo(Mask) != 0)
        {
          BAsmCode[CodeLen++] = Lo(Code) + (OpSize << 3);
          BAsmCode[CodeLen++] = Lo(Mask);
        }
        if ((OpSize == 1) && (Code == 0x1027) && (Mask & 0x80)) WrError(140);
      }
      else              /* PUSH(U): untere Register zuerst */
      {
        if (Lo(Mask) != 0)
        {
          BAsmCode[CodeLen++] = Lo(Code) + (OpSize << 3);
          BAsmCode[CodeLen++] = Lo(Mask);
        }
        if (Hi(Mask) != 0)
        {
          BAsmCode[CodeLen++] = Lo(Code) + (OpSize << 3) + 0x40;
          BAsmCode[CodeLen++] = Hi(Mask);
        }
      }
    }
  }
}

static void DecodeALU(Word Index)
{
  Byte HReg, HCnt, HVals[3], HMem;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg | MModMem);
    switch (AdrMode)
    {
      case ModReg:
        if (OpSize >= 2) WrError(1130);
        else if (OpSize == -1) WrError(1132);
        else
        {
          HReg = AdrPart;
          DecodeAdr(ArgStr[2], MModMem | MModImm);
          switch (AdrMode)
          {
            case ModMem:
              BAsmCode[CodeLen++] = (Index << 4) + (OpSize << 3) + MemPart;
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
              if ((MemPart == 3) && ((HReg >> (1 - OpSize)) == AdrPart)) WrError(140);
              break;
            case ModImm:
              BAsmCode[CodeLen++] = 0x91 + (OpSize << 3);
              BAsmCode[CodeLen++] = (HReg << 4) + Index;
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
              break;
          }
        }
        break;
      case ModMem:
        HReg = AdrPart; HMem = MemPart; HCnt = AdrCnt;
        memcpy(HVals, AdrVals, AdrCnt);
        DecodeAdr(ArgStr[2], MModReg | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            if (OpSize == 2) WrError(1130);
            else if (OpSize == -1) WrError(1132);
            else
            {
              BAsmCode[CodeLen++] = (Index << 4) + (OpSize << 3) + HMem;
              BAsmCode[CodeLen++] = (AdrPart << 4) + 8 + HReg;
              memcpy(BAsmCode + CodeLen, HVals, HCnt);
              CodeLen += HCnt;
              if ((HMem == 3) && ((AdrPart >> (1 - OpSize)) == HReg)) WrError(140);
            }
            break;
          case ModImm:
            if (OpSize == 2) WrError(1130);
            else if (OpSize == -1) WrError(1132);
            else
            {
              BAsmCode[CodeLen++] = 0x90 + HMem + (OpSize << 3);
              BAsmCode[CodeLen++] = (HReg << 4) + Index;
              memcpy(BAsmCode + CodeLen, HVals, HCnt);
              memcpy(BAsmCode + CodeLen + HCnt, AdrVals, AdrCnt);
              CodeLen += AdrCnt + HCnt;
             END
            break;
        }
        break;
    }
  }
}

static void DecodeRegO(Word Index)
{
  RegOrder *Op = RegOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    switch (AdrMode)
    {
      case ModReg:
        if ((Op->SizeMask & (1 << OpSize)) == 0) WrError(1130);
        else
        {
          BAsmCode[CodeLen++] = 0x90 + (OpSize << 3);
          BAsmCode[CodeLen++] = (AdrPart << 4) + Op->Code;
        }
        break;
    }
  }
}

static void DecodeShift(Word Index)
{
  Byte HReg, HMem;

  if (ArgCnt != 2) WrError(1110);
  else if (OpSize > 2) WrError(1130);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    switch (AdrMode)
    {
      case ModReg:
        HReg = AdrPart; HMem = OpSize;
        if (*ArgStr[2] == '#')
          OpSize = (HMem == 2) ? -4 : -2;
        else
          OpSize = 0;
        DecodeAdr(ArgStr[2], MModReg | ((Index == 3) ? 0 : MModImm));
        switch (AdrMode)
        {
          case ModReg:
            BAsmCode[CodeLen++] = 0xc0 + ((HMem & 1) << 3) + Index;
            if (HMem == 2) BAsmCode[CodeLen - 1] += 12;
            BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            if (Index == 3)
            {
              if (HMem == 2)
              {
                if ((AdrPart >> 2) == (HReg >> 1)) WrError(140);
              }
              else if ((AdrPart >> HMem) == HReg) WrError(140);
            }
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0xd0 + ((HMem & 1) << 3) + Index;
            if (HMem == 2)
            {
              BAsmCode[CodeLen - 1] += 12;
              BAsmCode[CodeLen++] = ((HReg & 14) << 4) + AdrVals[0];
            }
            else
              BAsmCode[CodeLen++] = (HReg << 4) + AdrVals[0];
            break;
        }
        break;
    }
  }
}

static void DecodeRotate(Word Code)
{
  Byte HReg, HMem;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    switch (AdrMode)
    {
      case ModReg:
        if (OpSize == 2) WrError(1130);
        else
        {
          HReg = AdrPart; HMem = OpSize; OpSize = -2;
          DecodeAdr(ArgStr[2], MModImm);
          switch (AdrMode)
          {
            case ModImm:
              BAsmCode[CodeLen++] = Code + (HMem << 3);
              BAsmCode[CodeLen++] = (HReg << 4) + AdrVals[0];
              break;
          }
        }
        break;
    }
  }
}

static void DecodeRel(Word Index)
{
  InvOrder *Op = RelOrders + Index;
  Boolean OK;
  LongInt SaveLong, AdrLong;

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    FirstPassUnknown = True;
    AdrLong = SaveLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      if (FirstPassUnknown) AdrLong &= Even32Mask;
      AdrLong -= (EProgCounter() + CodeLen + 2) & 0xfffffe;
      if (AdrLong & 1) WrError(1325);
      else if ((SymbolQuestionable) || ((AdrLong <= 254) && (AdrLong >= -256)))
      {
        BAsmCode[CodeLen++] = Op->Code;
        BAsmCode[CodeLen++] = (AdrLong >> 1) & 0xff;
      }
      else if (!DoBranchExt) WrError(1370);
      else if (Op->Inversion == 255)  /* BR */
      {
        AdrLong = SaveLong - ((EProgCounter() + CodeLen + 3) & 0xfffffe);
        if ((!SymbolQuestionable) && ((AdrLong > 65534) || (AdrLong < -65536))) WrError(1370);
        else if (AdrLong & 1) WrError(1325);
        else
        {
          AdrLong >>= 1;
          BAsmCode[CodeLen++] = 0xd5;
          BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
          BAsmCode[CodeLen++] = AdrLong & 0xff;
        }
      }
      else
      {
        AdrLong = SaveLong - ((EProgCounter() + CodeLen + 5) & 0xfffffe);
        if ((AdrLong > 65534) || (AdrLong < -65536)) WrError(1370);
        else
        {
          BAsmCode[CodeLen++] = RelOrders[Op->Inversion].Code;
          BAsmCode[CodeLen++] = 2;
          BAsmCode[CodeLen++] = 0xd5;
          BAsmCode[CodeLen++] = (AdrLong >> 9) & 0xff;
          BAsmCode[CodeLen++] = (AdrLong >> 1) & 0xff;
          if (Odd(EProgCounter() + CodeLen)) BAsmCode[CodeLen++] = 0;
        }
      }
    }
  }
}

static void DecodeJBit(Word Index)
{
  LongInt BitAdr, AdrLong, SaveLong, odd;
  Boolean OK;
  InvOrder *Op = JBitOrders + Index;

  if (ArgCnt != 2) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (DecodeBitAddr(ArgStr[1], &BitAdr))
  {
    FirstPassUnknown = False;
    AdrLong = SaveLong = EvalIntExpression(ArgStr[2], UInt24, &OK);
    if (OK)
    {
      if (FirstPassUnknown) AdrLong &= Even32Mask;
      AdrLong -= (EProgCounter() + CodeLen + 4) & 0xfffffe;
      if (AdrLong & 1) WrError(1325);
      else if ((SymbolQuestionable) || ((AdrLong <= 254) && (AdrLong >= -256)))
      {
        BAsmCode[CodeLen++] = 0x97;
        BAsmCode[CodeLen++] = Op->Code + Hi(BitAdr);
        BAsmCode[CodeLen++] = Lo(BitAdr);
        BAsmCode[CodeLen++] = (AdrLong >> 1) & 0xff;
      }
      else if (!DoBranchExt) WrError(1370);
      else if (Op->Inversion == 255)
      {
        odd = EProgCounter() & 1;
        AdrLong = SaveLong - ((EProgCounter() + CodeLen + 9 + odd) & 0xfffffe);
        if ((AdrLong > 65534) || (AdrLong < -65536)) WrError(1370);
        else                 
        {
          BAsmCode[CodeLen++] = 0x97;
          BAsmCode[CodeLen++] = Op->Code + Hi(BitAdr);
          BAsmCode[CodeLen++] = Lo(BitAdr);
          BAsmCode[CodeLen++] = 1 + odd;
          BAsmCode[CodeLen++] = 0xfe;
          BAsmCode[CodeLen++] = 2 + odd;
          if (odd) BAsmCode[CodeLen++] = 0;
          BAsmCode[CodeLen++] = 0xd5;
          BAsmCode[CodeLen++] = (AdrLong >> 9) & 0xff;
          BAsmCode[CodeLen++] = (AdrLong >> 1) & 0xff;
          BAsmCode[CodeLen++] = 0;
        }
      }
      else
      {
        AdrLong = SaveLong - ((EProgCounter() + CodeLen + 7) & 0xfffffe);
        if ((AdrLong > 65534) || (AdrLong < -65536)) WrError(1370);  
        else
        {
          BAsmCode[CodeLen++] = 0x97;
          BAsmCode[CodeLen++] = JBitOrders[Op->Inversion].Code + Hi(BitAdr);
          BAsmCode[CodeLen++] = Lo(BitAdr);
          BAsmCode[CodeLen++] = 2;
          BAsmCode[CodeLen++] = 0xd5;
          BAsmCode[CodeLen++] = (AdrLong >> 9) & 0xff;
          BAsmCode[CodeLen++] = (AdrLong >> 1) & 0xff;
          if (Odd(EProgCounter() + CodeLen)) BAsmCode[CodeLen++] = 0;
        }
      }
    }
  }
}


static void DecodeMOV(Word Index)
{
  LongInt AdrLong;
  Byte HVals[3], HReg, HPart, HCnt;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (!strcasecmp(ArgStr[1],"C"))
  {
    if (DecodeBitAddr(ArgStr[2], &AdrLong))
    {
      if (*AttrPart != '\0') WrError(1100);
      else
      {
        ChkBitPage(AdrLong);
        BAsmCode[CodeLen++] = 0x08;
        BAsmCode[CodeLen++] = 0x20 + Hi(AdrLong);
        BAsmCode[CodeLen++] = Lo(AdrLong);
      }
    }
  }
  else if (!strcasecmp(ArgStr[2], "C"))
  {
    if (DecodeBitAddr(ArgStr[1], &AdrLong))
    {
      if (*AttrPart != '\0') WrError(1100);
      else
      {
        ChkBitPage(AdrLong);
        BAsmCode[CodeLen++] = 0x08;
        BAsmCode[CodeLen++] = 0x30 + Hi(AdrLong);
        BAsmCode[CodeLen++] = Lo(AdrLong);
      }
    }
  }
  else if (!strcasecmp(ArgStr[1], "USP"))
  {
    SetOpSize(1);
    DecodeAdr(ArgStr[2], MModReg);
    if (AdrMode == ModReg)
    {
      BAsmCode[CodeLen++] = 0x98;
      BAsmCode[CodeLen++] = (AdrPart << 4) + 0x0f;
    }
  }
  else if (!strcasecmp(ArgStr[2], "USP"))
  {
    SetOpSize(1);
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode == ModReg)
    {
      BAsmCode[CodeLen++] = 0x90;
      BAsmCode[CodeLen++] = (AdrPart << 4)+0x0f;
    }
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg | MModMem);
    switch (AdrMode)
    {
      case ModReg:
        if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
        else
        {
          HReg = AdrPart;
          DecodeAdr(ArgStr[2], MModMem | MModImm);
          switch (AdrMode)
          {
            case ModMem:
              BAsmCode[CodeLen++] = 0x80 + (OpSize << 3) + MemPart;
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
              if ((MemPart == 3) && ((HReg >> (1 - OpSize)) == AdrPart)) WrError(140);
              break;
            case ModImm:
              BAsmCode[CodeLen++] = 0x91 + (OpSize << 3);
              BAsmCode[CodeLen++] = 0x08 + (HReg << 4);
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
              break;
          }
        }
        break;
      case ModMem:
        memcpy(HVals, AdrVals, AdrCnt); HCnt = AdrCnt; HPart = MemPart; HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg | MModMem | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
            else
            {
              BAsmCode[CodeLen++] = 0x80 + (OpSize << 3) + HPart;
              BAsmCode[CodeLen++] = (AdrPart << 4) + 0x08 + HReg;
              memcpy(BAsmCode + CodeLen, HVals, HCnt);
              CodeLen += HCnt;
              if ((HPart == 3) && ((AdrPart >> (1 - OpSize)) == HReg)) WrError(140);
            }
            break;
          case ModMem:
            if (OpSize == -1) WrError(1132);
            else if ((OpSize != 0) & (OpSize != 1)) WrError(1130);
            else if ((HPart == 6) && (MemPart == 6))
            {
              BAsmCode[CodeLen++] = 0x97 + (OpSize << 3);
              BAsmCode[CodeLen++] = (HReg << 4)+AdrPart;
              BAsmCode[CodeLen++] = HVals[0];
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            else if ((HPart == 6) && (MemPart == 2))
            {
              BAsmCode[CodeLen++] = 0xa0 + (OpSize << 3);
              BAsmCode[CodeLen++] = 0x80 + (AdrPart << 4) + HReg;
              BAsmCode[CodeLen++] = HVals[0];
            }
            else if ((HPart == 2) && (MemPart == 6))
            {
              BAsmCode[CodeLen++] = 0xa0 + (OpSize << 3);
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            else if ((HPart == 3) && (MemPart == 3))
            {
              BAsmCode[CodeLen++] = 0x90 + (OpSize << 3);
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              if (HReg == AdrPart) WrError(140);
            }
            else
              WrError(1350);
            break;
          case ModImm:
            if (OpSize == -1) WrError(1132);
            else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
            else
            {
              BAsmCode[CodeLen++] = 0x90 + (OpSize << 3) + HPart;
              BAsmCode[CodeLen++] = 0x08 + (HReg << 4);
              memcpy(BAsmCode + CodeLen, HVals, HCnt);
              memcpy(BAsmCode + CodeLen + HCnt, AdrVals, AdrCnt);
              CodeLen += HCnt + AdrCnt;
            }
            break;
        }
        break;
    }
  }
}

static void DecodeMOVC(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    if ((*AttrPart == '\0') && (!strcasecmp(ArgStr[1], "A"))) OpSize = 0;
    if (!strcasecmp(ArgStr[2], "[A+DPTR]"))
    {
      if (strcasecmp(ArgStr[1], "A")) WrError(1350);
      else if (OpSize != 0) WrError(1130);
      else
      {
        BAsmCode[CodeLen++] = 0x90;
        BAsmCode[CodeLen++] = 0x4e;
      }
    }
    else if (!strcasecmp(ArgStr[2], "[A+PC]"))
    {
      if (strcasecmp(ArgStr[1], "A")) WrError(1350);
      else if (OpSize != 0) WrError(1130);
      else
      {
        BAsmCode[CodeLen++] = 0x90;
        BAsmCode[CodeLen++] = 0x4c;
      }
    }
    else
    {
      DecodeAdr(ArgStr[1], MModReg);
      if (AdrMode != ModNone)
      {
        if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
        else
        {
          HReg = AdrPart;
          DecodeAdr(ArgStr[2], MModMem);
          if (AdrMode != ModNone)
          {
            if (MemPart != 3) WrError(1350);
            else
            {
              BAsmCode[CodeLen++] = 0x80 + (OpSize << 3);
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              if ((MemPart == 3) && ((HReg >> (1 - OpSize)) == AdrPart)) WrError(140);
            }
          }
        }
      }
    }
  }
}

static void DecodeMOVX(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModMem);
    if (AdrMode == ModMem)
    {
      switch (MemPart)
      {
        case 1:
          if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
          else
          {
            HReg = AdrPart; DecodeAdr(ArgStr[2], MModMem);
            if (AdrMode == ModMem)
            {
              if (MemPart != 2) WrError(1350);
              else
              {
                BAsmCode[CodeLen++] = 0xa7 + (OpSize << 3);
                BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              }
            }
          }
          break;
        case 2:
          HReg = AdrPart; DecodeAdr(ArgStr[2], MModReg);
          if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
          else
          {
            BAsmCode[CodeLen++] = 0xa7 + (OpSize << 3);
            BAsmCode[CodeLen++] = 0x08 + (AdrPart << 4) + HReg;
          }
          break;
        default:
          WrError(1350);
      }
    }
  }
}

static void DecodeXCH(Word Index)
{
  Byte HReg, HPart, HVals[3];
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModMem);
    if (AdrMode == ModMem)
    {
      switch (MemPart)
      {
        case 1:
          HReg = AdrPart; DecodeAdr(ArgStr[2], MModMem);
          if (AdrMode == ModMem)
          {
            if ((OpSize != 1) && (OpSize != 0)) WrError(1130);
            else switch (MemPart)
            {
              case 1:
                BAsmCode[CodeLen++] = 0x60 + (OpSize << 3);
                BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                if (HReg == AdrPart) WrError(140);
                break;
              case 2:
                BAsmCode[CodeLen++] = 0x50 + (OpSize << 3);
                BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
                break;
              case 6:
                BAsmCode[CodeLen++] = 0xa0 + (OpSize << 3);
                BAsmCode[CodeLen++] = 0x08 + (HReg << 4) + AdrPart;
                BAsmCode[CodeLen++] = AdrVals[0];
                break;
              default:
                WrError(1350);
            }
          }
          break;
        case 2:
          HReg = AdrPart;
          DecodeAdr(ArgStr[2], MModReg);
          if (AdrMode == ModReg)
          {
            if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
            else
            {
              BAsmCode[CodeLen++] = 0x50 + (OpSize << 3);
              BAsmCode[CodeLen++] = (AdrPart << 4) + HReg;
            }
          }
          break;
        case 6:
          HPart = AdrPart; HVals[0] = AdrVals[0];
          DecodeAdr(ArgStr[2], MModReg);
          if (AdrMode == ModReg)
          {
            if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
            else
            {
              BAsmCode[CodeLen++] = 0xa0 + (OpSize << 3);
              BAsmCode[CodeLen++] = 0x08 + (AdrPart << 4) + HPart;
              BAsmCode[CodeLen++] = HVals[0];
            }
          }
          break;
        default:
          WrError(1350);
      }
    }
  }
}

static void DecodeADDSMOVS(Word Index)
{
  Byte HReg,HMem;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    HMem = OpSize; OpSize = -3;
    DecodeAdr(ArgStr[2], MModImm);
    switch (AdrMode)
    {
      case ModImm:
        HReg = AdrVals[0]; OpSize = HMem;
        DecodeAdr(ArgStr[1], MModMem);
        switch (AdrMode)
        {
          case ModMem:
            if (OpSize == 2) WrError(1130);
            else if (OpSize == -1) WrError(1132);
            else
            {
               BAsmCode[CodeLen++] = 0xa0 + (Index << 4) + (OpSize << 3) + MemPart;
               BAsmCode[CodeLen++] = (AdrPart << 4) + (HReg & 0x0f);
               memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
               CodeLen += AdrCnt;
            }
            break;
        }
        break;
    } 
  }
}

static void DecodeDIV(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode == ModReg)
    {
      if ((OpSize != 1) && (OpSize != 2)) WrError(1130);
      else
      {
        HReg = AdrPart; OpSize--; DecodeAdr(ArgStr[2], MModReg | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            BAsmCode[CodeLen++] = 0xe7 + (OpSize << 3);
            BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0xe8 + OpSize;
            BAsmCode[CodeLen++] = (HReg << 4) + 0x0b - (OpSize << 1);
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
        }
      }
    }
  }
}

static void DecodeDIVU(Word Index)
{
  Byte HReg;
  int z;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode == ModReg)
    {
      if ((OpSize == 0) && (AdrPart & 1)) WrError(1445);
      else
      {
        HReg = AdrPart; z = OpSize; if (OpSize != 0) OpSize--;
        DecodeAdr(ArgStr[2], MModReg | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            BAsmCode[CodeLen++] = 0xe1 + (z << 2);
            if (z == 2) BAsmCode[CodeLen - 1] += 4;
            BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0xe8 + Ord(z == 2);
            BAsmCode[CodeLen++] = (HReg << 4) + 0x01 + (Ord(z == 1) << 1);
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
        }
      }
    }
  }
}

static void DecodeMUL(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode == ModReg)
    {
      if (OpSize != 1) WrError(1130);
      else if (AdrPart & 1) WrError(1445);
      else
      {
        HReg = AdrPart; DecodeAdr(ArgStr[2], MModReg | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            BAsmCode[CodeLen++] = 0xe6;
            BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0xe9;
            BAsmCode[CodeLen++] = (HReg << 4) + 0x08;
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
        }
      }
    }
  }
}

static void DecodeMULU(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode == ModReg)
    {
      if (AdrPart & 1) WrError(1445);
      else
      {
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            BAsmCode[CodeLen++] = 0xe0 + (OpSize << 2);
            BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            break;
          case ModImm:
            BAsmCode[CodeLen++] = 0xe8 + OpSize;
            BAsmCode[CodeLen++] = (HReg << 4);
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
        }
      }
    }
  }
}

static void DecodeLEA(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode == ModReg)
    {
      if (OpSize != 1) WrError(1130);
      else
      {
        HReg = AdrPart;
        strmaxprep(ArgStr[2], "[", 255);
        strmaxcat(ArgStr[2], "]", 255);
        DecodeAdr(ArgStr[2], MModMem);
        if (AdrMode == ModMem)
          switch (MemPart)
          {
            case 4:
            case 5:
              BAsmCode[CodeLen++] = 0x20 + (MemPart << 3);
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
              break;
            default:
              WrError(1350);
          }
      }
    }
  }
}

static void DecodeANLORL(Word Index)
{
  if (ArgCnt != 2) WrError(1110);
  else if (*AttrPart) WrError(1100);
  else if (strcasecmp(ArgStr[1], "C")) WrError(1350);
  else
  {
    char *pArg2 = ArgStr[2];
    Boolean Invert = False;
    LongInt AdrLong;

    if (*pArg2 == '/')
    {
      Invert = True; pArg2++;
    }
    if (DecodeBitAddr(pArg2, &AdrLong))
    {
      ChkBitPage(AdrLong);
      BAsmCode[CodeLen++] = 0x08;
      BAsmCode[CodeLen++] = 0x40 | (Index << 5) | (Ord(Invert) << 4) | (Hi(AdrLong) & 3);
      BAsmCode[CodeLen++] = Lo(AdrLong);
    }
  }
}

static void DecodeCLRSETB(Word Index)
{
  LongInt AdrLong;

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (DecodeBitAddr(ArgStr[1], &AdrLong))
  {
    ChkBitPage(AdrLong);
    BAsmCode[CodeLen++] = 0x08;
    BAsmCode[CodeLen++] = (Index << 4) + (Hi(AdrLong) & 3);
    BAsmCode[CodeLen++] = Lo(AdrLong);
  }
}

static void DecodeTRAP(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    OpSize = -2;
    DecodeAdr(ArgStr[1], MModImm);
    switch (AdrMode)
    {
      case ModImm:
        BAsmCode[CodeLen++] = 0xd6;
        BAsmCode[CodeLen++] = 0x30 + AdrVals[0];
        break;
    }
  }
}

static void DecodeCALL(Word Index)
{
  LongInt AdrLong;
  Boolean OK;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (*ArgStr[1] == '[')
  {
    DecodeAdr(ArgStr[1], MModMem);
    if (AdrMode != ModNone)
    {
      if (MemPart != 2) WrError(1350);
      else
      {
        BAsmCode[CodeLen++] = 0xc6;
        BAsmCode[CodeLen++] = AdrPart;
      }
    }
  }
  else
  {
    FirstPassUnknown = False;
    AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      if (FirstPassUnknown) AdrLong &= Even32Mask;
      AdrLong -= (EProgCounter() + CodeLen + 3) & 0xfffffe;
      if ((!SymbolQuestionable) && ((AdrLong > 65534) || (AdrLong < -65536))) WrError(1370);
      else if (AdrLong & 1) WrError(1325);
      else
      {
        AdrLong >>= 1;
        BAsmCode[CodeLen++] = 0xc5;
        BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
        BAsmCode[CodeLen++] = AdrLong & 0xff;
      }
    }
  }
}

static void DecodeJMP(Word Index)
{
  LongInt AdrLong;
  Boolean OK;

  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (!strcasecmp(ArgStr[1], "[A+DPTR]"))
  {
    BAsmCode[CodeLen++] = 0xd6;
    BAsmCode[CodeLen++] = 0x46;
  }
  else if (!strncmp(ArgStr[1], "[[", 2))
  {
    ArgStr[1][strlen(ArgStr[1]) - 1] = '\0';
    DecodeAdr(ArgStr[1] + 1, MModMem);
    if (AdrMode == ModMem)
     switch (MemPart)
     {
       case 3:
         BAsmCode[CodeLen++] = 0xd6;
         BAsmCode[CodeLen++] = 0x60 + AdrPart;
         break;
       default:
         WrError(1350);
     }
  }
  else if (*ArgStr[1] == '[')
  {
    DecodeAdr(ArgStr[1], MModMem);
    if (AdrMode == ModMem)
      switch (MemPart)
      {
        case 2:
          BAsmCode[CodeLen++] = 0xd6;
          BAsmCode[CodeLen++] = 0x70 + AdrPart;
          break;
        default:
          WrError(1350);
      }
  }
  else
  {
    FirstPassUnknown = False;
    AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      if (FirstPassUnknown) AdrLong &= Even32Mask;
      AdrLong -= ((EProgCounter() + CodeLen + 3) & 0xfffffe);
      if ((!SymbolQuestionable) && ((AdrLong > 65534) || (AdrLong < -65536))) WrError(1370);
      else if (AdrLong & 1) WrError(1325);
      else
      {
        AdrLong >>= 1;
        BAsmCode[CodeLen++] = 0xd5;
        BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
        BAsmCode[CodeLen++] = AdrLong & 0xff;
      }
    }
  }
}

static void DecodeCJNE(Word Index)
{
  LongInt AdrLong, SaveLong, odd;
  Boolean OK;
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 3) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    AdrLong = SaveLong = EvalIntExpression(ArgStr[3], UInt24, &OK);
    if (FirstPassUnknown) AdrLong &= 0xfffffe;
    if (OK)
    {
      ChkSpace(SegCode); OK = False; HReg = 0;
      DecodeAdr(ArgStr[1], MModMem);
      if (AdrMode == ModMem)
      {
        switch (MemPart)
        {
          case 1:
            if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
            else
            {
              HReg = AdrPart; DecodeAdr(ArgStr[2], MModMem | MModImm);
              switch (AdrMode)
              {
                case ModMem:
                  if (MemPart != 6) WrError(1350);
                  else
                  {
                    BAsmCode[CodeLen] = 0xe2 + (OpSize << 3);
                    BAsmCode[CodeLen + 1] = (HReg << 4) + AdrPart;
                    BAsmCode[CodeLen + 2] = AdrVals[0];
                    HReg=CodeLen + 3;
                    CodeLen += 4; OK = True;
                  }
                  break;
                case ModImm:
                  BAsmCode[CodeLen] = 0xe3 + (OpSize << 3);
                  BAsmCode[CodeLen + 1] = HReg << 4;
                  HReg=CodeLen + 2;
                  memcpy(BAsmCode + CodeLen + 3, AdrVals, AdrCnt);
                  CodeLen += 3 + AdrCnt; OK = True;
                  break;
              }
            }
            break;
          case 2:
            if ((OpSize != -1) && (OpSize != 0) && (OpSize != 1)) WrError(1130);
            else
            {
              HReg = AdrPart; DecodeAdr(ArgStr[2], MModImm);
              if (AdrMode == ModImm)
              {
                BAsmCode[CodeLen] = 0xe3 + (OpSize << 3);
                BAsmCode[CodeLen + 1] = (HReg << 4)+8;
                HReg = CodeLen + 2;
                memcpy(BAsmCode + CodeLen + 3, AdrVals, AdrCnt);
                CodeLen += 3 + AdrCnt; OK = True;
              }
            }
            break;
          default:
            WrError(1350);
        }
      }
      if (OK)
      {
        AdrLong -= (EProgCounter() + CodeLen) & 0xfffffe; OK = False;
        if (AdrLong & 1) WrError(1325);
        else if ((SymbolQuestionable) || ((AdrLong <= 254) && (AdrLong >= -256)))
        {
          BAsmCode[HReg] = (AdrLong >> 1) & 0xff; OK = True;
        }
        else if (!DoBranchExt) WrError(1370);
        else
        {
          odd = (EProgCounter() + CodeLen) & 1;
          AdrLong = SaveLong - ((EProgCounter() + CodeLen + 5 + odd) & 0xfffffe);
          if ((AdrLong < -65536) || (AdrLong > 65534)) WrError(1370);
          else
          {
            BAsmCode[HReg] = 1 + odd;
            BAsmCode[CodeLen++] = 0xfe;
            BAsmCode[CodeLen++] = 2 + odd;
            if (odd) BAsmCode[CodeLen++] = 0;
            BAsmCode[CodeLen++] = 0xd5;
            BAsmCode[CodeLen++] = (AdrLong >> 9) & 0xff;
            BAsmCode[CodeLen++] = (AdrLong >> 1) & 0xff;
            BAsmCode[CodeLen++] = 0;
            OK = True;
          }
        }
      }
      if (!OK)
        CodeLen = 0;
    }
  }
}

static void DecodeDJNZ(Word Index)
{
  LongInt AdrLong, SaveLong, odd;
  Boolean OK;
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    FirstPassUnknown = False;
    SaveLong = AdrLong = EvalIntExpression(ArgStr[2], UInt24, &OK);
    if (FirstPassUnknown) AdrLong &= 0xfffffe;
    if (OK)
    {
      ChkSpace(SegCode); HReg = 0;
      DecodeAdr(ArgStr[1], MModMem);
      OK=False; DecodeAdr(ArgStr[1], MModMem);
      if (AdrMode == ModMem)
        switch (MemPart)
        {
          case 1:
            if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
            else
            {
              BAsmCode[CodeLen] = 0x87 + (OpSize << 3);
              BAsmCode[CodeLen+1] = (AdrPart << 4) + 0x08;
              HReg=CodeLen + 2;
              CodeLen += 3; OK = True;
            }
            break;
          case 6:
            if (OpSize == -1) WrError(1132);
            else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
            else
            {
              BAsmCode[CodeLen] = 0xe2 + (OpSize << 3);
              BAsmCode[CodeLen+1] = 0x08 + AdrPart;
              BAsmCode[CodeLen+2] = AdrVals[0];
              HReg=CodeLen + 3;
              CodeLen += 4; OK = True;
            }
            break;
          default:
            WrError(1350);
        }
      if (OK)
      {
        AdrLong -= (EProgCounter() + CodeLen) & 0xfffffe; OK = False;
        if (AdrLong & 1) WrError(1325);
        else if ((SymbolQuestionable) || ((AdrLong <= 254) && (AdrLong >= -256)))
        {
          BAsmCode[HReg] = (AdrLong >> 1) & 0xff;
          OK = True;
        }
        else if (!DoBranchExt) WrError(1370);
        else
        {
          odd = (EProgCounter() + CodeLen) & 1;
          AdrLong = SaveLong - ((EProgCounter() + CodeLen + 5 + odd) & 0xfffffe);
          if ((AdrLong < -65536) || (AdrLong > 65534)) WrError(1370);
          else
          {
            BAsmCode[HReg] = 1 + odd;
            BAsmCode[CodeLen++] = 0xfe;
            BAsmCode[CodeLen++] = 2 + odd;
            if (odd) BAsmCode[CodeLen++] = 0;
            BAsmCode[CodeLen++] = 0xd5;
            BAsmCode[CodeLen++] = (AdrLong >> 9) & 0xff;
            BAsmCode[CodeLen++] = (AdrLong >> 1) & 0xff;
            BAsmCode[CodeLen++] = 0;
            OK = True;
          }
        }
      }
      if (!OK)
        CodeLen = 0;
    }
  }
}

static void DecodeFCALLJMP(Word Index)
{
  LongInt AdrLong;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    FirstPassUnknown = False;
    AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
    if (FirstPassUnknown) AdrLong &= 0xfffffe;
    if (OK)
    {
      if (AdrLong & 1) WrError(1325);
      else
      {
        BAsmCode[CodeLen++] = 0xc4 | (Index << 4);
        BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
        BAsmCode[CodeLen++] = AdrLong & 0xff;
        BAsmCode[CodeLen++] = (AdrLong >> 16) & 0xff;
      }
    }
  }
}

static Boolean IsRealDef(void)
{
  switch (*OpPart)
  {
    case 'P':
      return Memo("PORT");
    case 'B':
      return Memo("BIT");
    default:
      return FALSE;
  }
}

static void ForceAlign(void)
{
  if (EProgCounter() & 1)
  {
    BAsmCode[0] = NOPCode; CodeLen = 1;
  }
}

static void MakeCode_XA(void)
{
  CodeLen = 0; DontPrint = False; OpSize = -1;

   /* Operandengroesse */

  if (*AttrPart != '\0')
    switch (mytoupper(*AttrPart))
    {
      case 'B': SetOpSize(0); break;
      case 'W': SetOpSize(1); break;
      case 'D': SetOpSize(2); break;
      default : WrError(1107); return;
    }

  /* Labels muessen auf geraden Adressen liegen */

  if ( (ActPC == SegCode) && (!IsRealDef()) &&
       ((*LabPart != '\0') ||((ArgCnt == 1) && (!strcmp(ArgStr[1], "$")))) )
  {
    ForceAlign();
    if (*LabPart != '\0') 
      EnterIntSymbol(LabPart, EProgCounter() + CodeLen, ActPC, False);
  }

  if (DecodeMoto16Pseudo(OpSize,False)) return;
  if (DecodeIntelPseudo(False)) return;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* via Tabelle suchen */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

/*-------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddJBit(char *NName, Word NCode)
{
  if (InstrZ >= JBitOrderCnt) exit(255);
  JBitOrders[InstrZ].Name = NName;
  JBitOrders[InstrZ].Inversion = 255;
  JBitOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeJBit);
}

static void AddStack(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeStack);
}

static void AddReg(char *NName, Byte NMask, Byte NCode)
{
  if (InstrZ >= RegOrderCnt) exit(255);
  RegOrders[InstrZ].Code = NCode;
  RegOrders[InstrZ].SizeMask = NMask;
  AddInstTable(InstTable, NName, InstrZ++, DecodeRegO);
}

static void AddRotate(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeRotate);
}

static void AddRel(char *NName, Word NCode)
{
  if (InstrZ >= RelOrderCount) exit(255);
  RelOrders[InstrZ].Name = NName;
  RelOrders[InstrZ].Inversion = 255;
  RelOrders[InstrZ].Code = NCode;
  AddInstTable(InstTable, NName, InstrZ++, DecodeRel);
}

static void SetInv(char *Name1, char *Name2, InvOrder *Orders)
{
  InvOrder *Order1, *Order2;

  for (Order1 = Orders; strcmp(Order1->Name, Name1); Order1++);
  for (Order2 = Orders; strcmp(Order2->Name, Name2); Order2++);
  Order1->Inversion = Order2 - Orders;
  Order2->Inversion = Order1 - Orders;
}

static void InitFields(void)
{
  InstTable = CreateInstTable(201); 
  AddInstTable(InstTable, "MOV"  , 0, DecodeMOV);
  AddInstTable(InstTable, "MOVC" , 0, DecodeMOVC);
  AddInstTable(InstTable, "MOVX" , 0, DecodeMOVX);
  AddInstTable(InstTable, "XCH"  , 0, DecodeXCH);
  AddInstTable(InstTable, "ADDS" , 0, DecodeADDSMOVS);
  AddInstTable(InstTable, "MOVS" , 1, DecodeADDSMOVS);
  AddInstTable(InstTable, "DIV"  , 0, DecodeDIV);
  AddInstTable(InstTable, "DIVU" , 0, DecodeDIVU);
  AddInstTable(InstTable, "MUL"  , 0, DecodeMUL);
  AddInstTable(InstTable, "DIVU" , 0, DecodeDIVU);
  AddInstTable(InstTable, "MULU" , 0, DecodeMULU);
  AddInstTable(InstTable, "LEA"  , 0, DecodeLEA);
  AddInstTable(InstTable, "ANL"  , 0, DecodeANLORL);
  AddInstTable(InstTable, "ORL"  , 1, DecodeANLORL);
  AddInstTable(InstTable, "CLR"  , 0, DecodeCLRSETB);
  AddInstTable(InstTable, "SETB" , 1, DecodeCLRSETB);
  AddInstTable(InstTable, "TRAP" , 0, DecodeTRAP);
  AddInstTable(InstTable, "CALL" , 0, DecodeCALL);
  AddInstTable(InstTable, "JMP"  , 0, DecodeJMP);
  AddInstTable(InstTable, "CJNE" , 0, DecodeCJNE);
  AddInstTable(InstTable, "DJNZ" , 0, DecodeDJNZ);
  AddInstTable(InstTable, "FCALL", 0, DecodeFCALLJMP);
  AddInstTable(InstTable, "FJMP" , 1, DecodeFCALLJMP);
  AddInstTable(InstTable, "PORT" , 0, DecodePORT);
  AddInstTable(InstTable, "BIT"  , 0, DecodeBIT);

  AddFixed("NOP"  , 0x0000);
  AddFixed("RET"  , 0xd680);
  AddFixed("RETI" , RETICode);
  AddFixed("BKPT" , 0x00ff);
  AddFixed("RESET", 0xd610);

  JBitOrders = (InvOrder *) malloc(sizeof(InvOrder) * JBitOrderCnt); InstrZ = 0;
  AddJBit("JB"  , 0x80);
  AddJBit("JBC" , 0xc0);
  AddJBit("JNB" , 0xa0);
  SetInv("JB", "JNB", JBitOrders);

  AddStack("POP"  , 0x1027);
  AddStack("POPU" , 0x0037);
  AddStack("PUSH" , 0x3007);
  AddStack("PUSHU", 0x2017);

  InstrZ = 0;
  AddInstTable(InstTable, "ADD" , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "ADDC", InstrZ++, DecodeALU);
  AddInstTable(InstTable, "SUB" , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "SUBB", InstrZ++, DecodeALU);
  AddInstTable(InstTable, "CMP" , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "AND" , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "OR"  , InstrZ++, DecodeALU);
  AddInstTable(InstTable, "XOR" , InstrZ++, DecodeALU);

  RegOrders = (RegOrder *) malloc(sizeof(RegOrder) * RegOrderCnt); InstrZ = 0;
  AddReg("NEG" , 3, 0x0b);
  AddReg("CPL" , 3, 0x0a);
  AddReg("SEXT", 3, 0x09);
  AddReg("DA"  , 1, 0x08);

  AddInstTable(InstTable, "LSR" , 0, DecodeShift);
  AddInstTable(InstTable, "ASL" , 1, DecodeShift);
  AddInstTable(InstTable, "ASR" , 2, DecodeShift);
  AddInstTable(InstTable, "NORM", 3, DecodeShift);

  AddRotate("RR" , 0xb0); AddRotate("RL" , 0xd3);
  AddRotate("RRC", 0xb7); AddRotate("RLC", 0xd7);

  RelOrders = (InvOrder *) malloc(sizeof(InvOrder) * RelOrderCount); InstrZ = 0;
  AddRel("BCC", 0xf0); AddRel("BCS", 0xf1); AddRel("BNE", 0xf2);
  AddRel("BEQ", 0xf3); AddRel("BNV", 0xf4); AddRel("BOV", 0xf5);
  AddRel("BPL", 0xf6); AddRel("BMI", 0xf7); AddRel("BG" , 0xf8);
  AddRel("BL" , 0xf9); AddRel("BGE", 0xfa); AddRel("BLT", 0xfb);
  AddRel("BGT", 0xfc); AddRel("BLE", 0xfd); AddRel("BR" , 0xfe);
  AddRel("JZ" , 0xec); AddRel("JNZ", 0xee);
  SetInv("BCC", "BCS", RelOrders);
  SetInv("BNE", "BEQ", RelOrders);
  SetInv("BNV", "BOV", RelOrders);
  SetInv("BPL", "BMI", RelOrders);
  SetInv("BG" , "BL" , RelOrders);
  SetInv("BGE", "BLT", RelOrders);
  SetInv("BGT", "BLE", RelOrders);
  SetInv("JZ" , "JNZ", RelOrders);
}

static void DeinitFields(void)
{
  free(JBitOrders);
  free(RegOrders);
  free(RelOrders);

  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/
/* Callbacks */

static void InitCode_XA(void)
{
  SaveInitProc();
  Reg_DS = 0;
}

static Boolean ChkPC_XA(LargeWord Addr)
{
  switch (ActPC)
  {
    case SegCode:
    case SegData:
      return (Addr<0x1000000);
    case SegIO:
      return ((Addr > 0x3ff) && (Addr < 0x800));
    default:
      return False;
  }
}

static Boolean IsDef_XA(void)
{
  return (ActPC == SegCode);
}

static void SwitchFrom_XA(void)
{
  DeinitFields(); ClearONOFF();
}

static void SwitchTo_XA(void)
{
  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

  PCSymbol = "$"; HeaderID = 0x3c; NOPCode = 0x00;
  DivideChars = ","; HasAttrs = True; AttrChars = ".";

  ValidSegs =(1 << SegCode) | (1 << SegData) | (1 << SegIO);
  Grans[SegCode ] = 1; ListGrans[SegCode ] = 1; SegInits[SegCode ] = 0;
  Grans[SegData ] = 1; ListGrans[SegData ] = 1; SegInits[SegData ] = 0;
  Grans[SegIO   ] = 1; ListGrans[SegIO   ] = 1; SegInits[SegIO   ] = 0x400;

  MakeCode = MakeCode_XA; ChkPC = ChkPC_XA; IsDef = IsDef_XA;
  SwitchFrom = SwitchFrom_XA; InitFields();
  AddONOFF("SUPMODE",   &SupAllowed,  SupAllowedName, False);
  AddONOFF("BRANCHEXT", &DoBranchExt, BranchExtName , False);
  AddMoto16PseudoONOFF();

  pASSUMERecs = ASSUMEXAs;
  ASSUMERecCnt = ASSUMEXACount;

  SetFlag(&DoPadding, DoPaddingName, False);
}

void codexa_init(void)
{
  CPUXAG1 = AddCPU("XAG1", SwitchTo_XA);
  CPUXAG2 = AddCPU("XAG2", SwitchTo_XA);
  CPUXAG3 = AddCPU("XAG3", SwitchTo_XA);

  SaveInitProc = InitPassProc; InitPassProc = InitCode_XA;
}
