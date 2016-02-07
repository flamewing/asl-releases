/* code51.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator fuer MCS-51/252 Prozessoren                                 */
/*                                                                           */
/* Historie:  5. 6.1996 Grundsteinlegung                                     */
/*            9. 8.1998 kurze 8051-Bitadressen wurden im 80251-Sourcemodus   */
/*                      immer lang gemacht                                   */
/*           24. 8.1998 Kodierung fuer MOV dir8,Rm war falsch (Fehler im     */
/*                      Manual!)                                             */
/*            2. 1.1998 ChkPC-Routine entfernt                               */
/*           19. 1.1999 Angefangen, Relocs zu uebertragen                    */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           30.10.2000 started adding immediate relocs                      */
/*            7. 1.2001 silenced warnings about unused parameters            */
/*           2002-01-23 symbols defined with BIT must not be macro-local     */
/*                                                                           */
/*****************************************************************************/
/* $Id: code51.c,v 1.15 2016/01/30 17:15:01 alfred Exp $                      */
/***************************************************************************** 
 * $Log: code51.c,v $
 * Revision 1.15  2016/01/30 17:15:01  alfred
 * - allow register symbols on MCS-(2)51
 *
 * Revision 1.14  2014/12/14 17:58:47  alfred
 * - remove static variables in strutil.c
 *
 * Revision 1.13  2014/12/07 19:13:59  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.12  2014/12/05 11:58:15  alfred
 * - collapse STDC queries into one file
 *
 * Revision 1.11  2014/12/01 18:29:39  alfred
 * - replace Nil -> NULL
 *
 * Revision 1.10  2014/11/16 13:15:07  alfred
 * - remove some superfluous semicolons
 *
 * Revision 1.9  2014/11/08 17:47:31  alfred
 * - adapt to new style
 *
 * Revision 1.8  2014/11/05 15:47:14  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.7  2010/04/17 13:14:20  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.6  2007/11/24 22:48:04  alfred
 * - some NetBSD changes
 *
 * Revision 1.5  2006/02/27 20:21:17  alfred
 * - correct bit range for RAM bit areas
 *
 * Revision 1.4  2005/10/02 10:00:44  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2005/09/08 16:53:41  alfred
 * - use common PInstTable
 *
 * Revision 1.2  2004/05/29 11:33:00  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 * Revision 1.1  2003/11/06 02:49:20  alfred
 * - recreated
 *
 * Revision 1.4  2003/06/22 13:14:43  alfred
 * - added 80251T
 *
 * Revision 1.3  2002/05/31 19:19:17  alfred
 * - added DS80C390 instruction variations
 *
 * Revision 1.2  2002/03/10 11:55:12  alfred
 * - do not issue futher error messages after failed address evaluation
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmrelocs.h"
#include "codepseudo.h"
#include "intpseudo.h"
#include "asmitree.h"
#include "codevars.h"
#include "fileformat.h"
#include "intconsts.h"

/*-------------------------------------------------------------------------*/
/* Daten */

typedef struct
{ 
  CPUVar MinCPU;
  Word Code;
} FixedOrder;

enum
{
  ModNone = -1,
  ModReg = 1,
  ModIReg8 = 2,
  ModIReg = 3,
  ModInd = 5,
  ModImm = 7,
  ModImmEx = 8,
  ModDir8 = 9,
  ModDir16 = 10,
  ModAcc = 11,
  ModBit51 = 12,
  ModBit251 = 13,
};

#define MModReg (1 << ModReg)
#define MModIReg8 (1 << ModIReg8)
#define MModIReg (1 << ModIReg)
#define MModInd (1 << ModInd)
#define MModImm (1 << ModImm)
#define MModImmEx (1 << ModImmEx)
#define MModDir8 (1 << ModDir8)
#define MModDir16 (1 << ModDir16)
#define MModAcc (1 << ModAcc)
#define MModBit51 (1 << ModBit51)
#define MModBit251 (1 << ModBit251)

#define MMod51 (MModReg | MModIReg8 | MModImm | MModAcc | MModDir8)
#define MMod251 (MModIReg | MModInd | MModImmEx | MModDir16)

#define AccOrderCnt 6
#define FixedOrderCnt 5
#define CondOrderCnt 13
#define BCondOrderCnt 3

#define AccReg 11


static FixedOrder *FixedOrders;
static FixedOrder *AccOrders;
static FixedOrder *CondOrders;
static FixedOrder *BCondOrders;

static Byte AdrVals[5];
static Byte AdrPart,AdrSize;
static ShortInt AdrMode,OpSize;
static Boolean MinOneIs0;

static Boolean SrcMode, BigEndian;

static CPUVar CPU87C750, CPU8051, CPU8052, CPU80C320,
       CPU80501, CPU80502, CPU80504, CPU80515, CPU80517,
       CPU80C390,
       CPU80251, CPU80251T;

static PRelocEntry AdrRelocInfo, BackupAdrRelocInfo;
static LongWord AdrOffset, AdrRelocType,
                BackupAdrOffset, BackupAdrRelocType;

/*-------------------------------------------------------------------------*/
/* Adressparser */

static void SetOpSize(ShortInt NewSize)
{
  if (OpSize == -1)
    OpSize = NewSize;
  else if (OpSize != NewSize)
  {
    WrError(1131);
    AdrMode = ModNone;
    AdrCnt = 0;
  }
}

static Boolean DecodeReg(const char *pAsc, Byte *Erg, Byte *Size)
{
  static Byte Masks[3] = { 0, 1, 3 };

  const char *Start;
  char *pRepl;
  int alen;
  Boolean IO;

  if (FindRegDef(pAsc, &pRepl))
    pAsc = pRepl;
  alen = strlen(pAsc);

  if (!strcasecmp(pAsc, "DPX"))
  {
    *Erg = 14;
    *Size = 2;
    return True;
  }

  if (!strcasecmp(pAsc, "SPX"))
  {
    *Erg = 15;
    *Size = 2;
    return True;
  }

  if ((alen >= 2) && (mytoupper(*pAsc) == 'R'))
  {
    Start = pAsc + 1;
    *Size = 0;
  }
  else if ((MomCPU >= CPU80251) && (alen >= 3) && (mytoupper(*pAsc) == 'W') && (mytoupper(pAsc[1]) == 'R'))
  {
    Start = pAsc + 2;
    *Size = 1;
  }
  else if ((MomCPU >= CPU80251) && (alen >= 3) && (mytoupper(*pAsc) == 'D') && (mytoupper(pAsc[1]) == 'R'))
  {
    Start = pAsc + 2;
    *Size = 2;
  }
  else
    return False;

  *Erg = ConstLongInt(Start, &IO, 10);
  if (!IO) return False;
  else if ((*Erg) & Masks[*Size]) return False;
  else
  {
    (*Erg) >>= (*Size);
    switch (*Size)
    {
      case 0:
        return (((*Erg) < 8) || ((MomCPU >= CPU80251) && ((*Erg) < 16)));
      case 1:
        return ((*Erg) < 16);
      case 2:
        return (((*Erg) < 8) || ((*Erg) == 14) || ((*Erg) == 15));
      default:
        return False;
    }
  }
}

static void SaveAdrRelocs(LongWord Type, LongWord Offset)
{
  AdrOffset = Offset;
  AdrRelocType = Type;
  AdrRelocInfo = LastRelocs;
  LastRelocs = NULL;
}

static void SaveBackupAdrRelocs(void)
{
  BackupAdrOffset = AdrOffset;
  BackupAdrRelocType = AdrRelocType;
  BackupAdrRelocInfo = AdrRelocInfo;
  AdrRelocInfo = NULL;
}

static void TransferAdrRelocs(LongWord Offset)
{
  TransferRelocs2(AdrRelocInfo, ProgCounter() + AdrOffset + Offset, AdrRelocType);
  AdrRelocInfo = NULL;
}

static void TransferBackupAdrRelocs(LargeWord Offset)
{
  TransferRelocs2(BackupAdrRelocInfo, ProgCounter() + BackupAdrOffset + Offset, BackupAdrRelocType);
  AdrRelocInfo = NULL;
}

static void DecodeAdr(char *Asc_O, Word Mask)
{
  Boolean OK, FirstFlag;
  Byte HSize;
  Word H16;
  int SegType;
  char *PPos, *MPos, *DispPos, Save = '\0';
  LongWord H32;
  String Asc, Part;
  Word ExtMask;

  strmaxcpy(Asc, Asc_O, 255);

  AdrMode = ModNone; AdrCnt = 0;

  ExtMask = MMod251 & Mask;
  if (MomCPU < CPU80251) Mask &= MMod51;

  if (*Asc == '\0') return;

  if (!strcasecmp(Asc, "A"))
  {
    if (!(Mask & MModAcc))
    {
      AdrMode = ModReg;
      AdrPart = AccReg;
    }
    else
      AdrMode = ModAcc;
    SetOpSize(0);
    goto chk;
  }

  if (*Asc == '#')
  {
    if ((OpSize == -1) && (MinOneIs0)) OpSize = 0;
    switch (OpSize)
    {
      case -1:
        WrError(1132);
        break;
      case 0:
        AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
        if (OK)
        {
          AdrMode = ModImm;
          AdrCnt = 1;
          SaveAdrRelocs(RelocTypeB8, 0);
        }
        break;
      case 1:
        H16 = EvalIntExpression(Asc + 1, Int16, &OK);
        if (OK)
        {
          AdrVals[0] = Hi(H16);
          AdrVals[1] = Lo(H16);
          AdrMode = ModImm;
          AdrCnt = 2;
          SaveAdrRelocs(RelocTypeB16, 0);
        }
        break;
      case 2:
        FirstPassUnknown = False;
        H32 = EvalIntExpression(Asc + 1, Int32, &OK);
        if (FirstPassUnknown)
          H32 &= 0xffff;
        if (OK)
        {
          AdrVals[1] = H32 & 0xff; 
          AdrVals[0] = (H32 >> 8) & 0xff;
          H32 >>= 16; 
          if (H32 == 0)
            AdrMode = ModImm;
          else if ((H32 == 1) || (H32 == 0xffff))
            AdrMode = ModImmEx;
          else
            WrError(1132);
          if (AdrMode != ModNone)
            AdrCnt = 2;
          SaveAdrRelocs(RelocTypeB16, 0);
        }
        break;
      case 3:
        H32 = EvalIntExpression(Asc + 1, Int24, &OK);
        if (OK)
        {
          AdrVals[0] = (H32 >> 16) & 0xff;
          AdrVals[1] = (H32 >> 8) & 0xff;
          AdrVals[2] = H32 & 0xff; 
          AdrCnt = 3;
          AdrMode = ModImm;
          SaveAdrRelocs(RelocTypeB24, 0);
        }
        break;
    }
    goto chk;
  }

  if (DecodeReg(Asc, &AdrPart, &HSize))
  {
    if ((MomCPU >= CPU80251) && ((Mask & MModReg) == 0))
      AdrMode = ((HSize == 0) && (AdrPart == AccReg)) ? ModAcc : ModReg;
    else
      AdrMode = ModReg;
    SetOpSize(HSize);
    goto chk;
  }

  if (*Asc == '@')
  {
    PPos = strchr(Asc, '+');
    MPos = strchr(Asc, '-');
    if ((MPos) && ((MPos < PPos) || (!PPos)))
      PPos = MPos;
    if (PPos) 
    {
      Save = (*PPos);
      *PPos = '\0';
    }
    if (DecodeReg(Asc + 1, &AdrPart, &HSize))
    {
      if (!PPos)
      {
        H32 = 0;
        OK = True;
      }
      else
      {
        *PPos = Save;
        DispPos = PPos;
        if (*DispPos == '+')
          DispPos++;
        H32 = EvalIntExpression(DispPos, SInt16, &OK);
      }
      if (OK)
        switch (HSize)
        {
          case 0:
            if ((AdrPart>1) || (H32 != 0)) WrError(1350);
            else
              AdrMode = ModIReg8;
            break;
          case 1:
            if (H32 == 0)
            {
              AdrMode = ModIReg;
              AdrSize = 0;
            }
            else
            {
              AdrMode = ModInd;
              AdrSize = 0;
              AdrVals[1] = H32 & 0xff; 
              AdrVals[0] = (H32 >> 8) & 0xff;
              AdrCnt = 2;
            }
            break;
          case 2:
            if (H32 == 0)
            {
              AdrMode = ModIReg;
              AdrSize = 2;
            }
            else
            {
              AdrMode = ModInd;
              AdrSize = 2;
              AdrVals[1] = H32 & 0xff; 
              AdrVals[0] = (H32 >> 8) & 0xff;
              AdrCnt = 2;
            }
            break;
        }
    }
    else
      WrError(1350);
    if (PPos)
      *PPos = Save;
    goto chk;
  }

  FirstFlag = False;
  SegType = -1;
  PPos = QuotPos(Asc, ':');
  if (PPos)
  {
    if (MomCPU < CPU80251)
    {
      WrError(1350);
      return;
    }
    else
    {
      SplitString(Asc, Part, Asc, PPos);
      if (!strcasecmp(Part, "S"))
        SegType = -2;
      else
      {
        FirstPassUnknown = False;
        SegType = EvalIntExpression(Asc, UInt8, &OK);
        if (!OK)
          return;
        if (FirstPassUnknown)
          FirstFlag = True;
      }
    }
  }

  FirstPassUnknown = False;
  switch (SegType)
  {
    case -2:
      H32 = EvalIntExpression(Asc, UInt9, &OK);
      ChkSpace(SegIO);
      if (FirstPassUnknown)
        H32 = (H32 & 0xff) | 0x80;
      break;
    case -1:
      H32 = EvalIntExpression(Asc, UInt24, &OK);
      break;
    default:
      H32 = EvalIntExpression(Asc, UInt16, &OK);
  }
  if (FirstPassUnknown)
    FirstFlag = True;
  if (!OK)
    return;

  if ((SegType == -2) || ((SegType == -1) && ((TypeFlag & (1 << SegIO)) != 0)))
  {
    if (ChkRange(H32, 0x80, 0xff))
    {
      SaveAdrRelocs(RelocTypeB8, 0);
      AdrMode = ModDir8;
      AdrVals[0] = H32 & 0xff;
      AdrCnt = 1;
    }
  }

  else
  {
    if (SegType >= 0)
      H32 += ((LongWord)SegType) << 16;
    if (FirstFlag)
      H32 &= ((MomCPU < CPU80251) || ((Mask & ModDir16) == 0)) ? 0xff : 0xffff;

    if (((H32 < 128) || ((H32 < 256) && (MomCPU < CPU80251))) && ((Mask & MModDir8) != 0))
    {
      if (MomCPU < CPU80251)
        ChkSpace(SegData);
      SaveAdrRelocs(RelocTypeB8, 0);
      AdrMode = ModDir8;
      AdrVals[0] = H32 &0xff;
      AdrCnt = 1;
    }
    else if ((MomCPU < CPU80251) || (H32 > 0xffff)) WrError(1925);
    else
    {
      AdrMode = ModDir16;
      AdrCnt = 2;
      AdrVals[1] = H32 & 0xff; 
      AdrVals[0] = (H32 >> 8) & 0xff;
    }
  }

chk:
  if ((AdrMode != ModNone) && ((Mask & (1 << AdrMode)) == 0))
  {
    WrError((ExtMask & (1 << AdrMode)) ? 1505 : 1350);
    AdrCnt = 0;
    AdrMode = ModNone;
  }
}

static ShortInt DecodeBitAdr(char *Asc, LongInt *Erg, Boolean MayShorten)
{
  Boolean OK;
  char *PPos, Save;

  PPos = RQuotPos(Asc, '.');
  if (MomCPU < CPU80251)
  {
    if (PPos == NULL)
    {
      *Erg = EvalIntExpression(Asc, UInt8, &OK);
      if (OK)
      {
        ChkSpace(SegBData);
        return ModBit51;
      }
      else
        return ModNone;
    }
    else
    {
      Save = *PPos;
      *PPos = '\0';
      FirstPassUnknown = False;
      *Erg = EvalIntExpression(Asc, UInt8, &OK);
      if (FirstPassUnknown)
        *Erg = 0x20;
      *PPos = Save;
      if (!OK) return ModNone;
      else
      {
        ChkSpace(SegData);
        Save = EvalIntExpression(PPos + 1, UInt3, &OK);
        if (!OK) return ModNone;
        else
        {
          if (*Erg > 0x7f)
          {
            if ((*Erg) & 7)
              WrError(220);
          }
          else
          {
            if (((*Erg) & 0xe0) != 0x20)
              WrError(220);
            *Erg = (*Erg - 0x20) << 3;
          }
          *Erg += Save;
          return ModBit51;
        }
      }
    }
  }
  else
  {
    if (!PPos)
    {
      FirstPassUnknown = False;
      *Erg = EvalIntExpression(Asc, Int32, &OK);
      if (FirstPassUnknown)
        (*Erg) &= 0x070000ff;
      if (((*Erg) & INTCONST_f8ffff00) != 0)
      {
        WrError(1510);
        OK = False;
      }
    }
    else
    {
      Save = (*PPos);
      *PPos = '\0';
      DecodeAdr(Asc, MModDir8);
      *PPos = Save;
      if (AdrMode == ModNone)
        OK = False;
      else
      {
        *Erg = EvalIntExpression(PPos + 1, UInt3, &OK) << 24;
        if (OK)
          (*Erg) += AdrVals[0];
      }
    }
    if (!OK)
      return ModNone;
    else if (MayShorten)
    {
      if (((*Erg) & 0x87) == 0x80)
      {
        *Erg = ((*Erg) & 0xf8) + ((*Erg) >> 24);
        return ModBit51;
      }
      else if (((*Erg) & 0xf0) == 0x20)
      {
        *Erg = (((*Erg) & 0x0f) << 3) + ((*Erg) >> 24);
        return ModBit51;
      }
      else
        return ModBit251;
    }
    else
      return ModBit251;
  }
}

static Boolean Chk504(LongInt Adr)
{
  return ((MomCPU == CPU80504) && ((Adr & 0x7ff) == 0x7fe));
}

static Boolean NeedsPrefix(Word Opcode)
{
  return (((Opcode&0x0f) >= 6) && ((SrcMode != 0) != ((Hi(Opcode) != 0) != 0)));
}

static void PutCode(Word Opcode)
{
  if (((Opcode&0x0f) < 6) || ((SrcMode != 0) != ((Hi(Opcode) == 0) != 0)))
  {
    BAsmCode[0] = Lo(Opcode);
    CodeLen = 1;
  }
  else
  {
    BAsmCode[0] = 0xa5;
    BAsmCode[1] = Lo(Opcode);
    CodeLen = 2;
  }
}

static Boolean IsCarry(const char *pArg)
{
  return (!strcasecmp(pArg, "C")) || (!strcasecmp(pArg, "CY"));
}

/*-------------------------------------------------------------------------*/
/* Einzelfaelle */

static void DecodeMOV(Word Index)
{
  LongInt AdrLong;
  Byte HSize, HReg;
  Integer AdrInt;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (IsCarry(ArgStr[1]))
  {
    switch (DecodeBitAdr(ArgStr[2], &AdrLong, True))
    {
      case ModBit51:
        PutCode(0xa2);
        BAsmCode[CodeLen] = AdrLong & 0xff;
        CodeLen++;
        break;
      case ModBit251:
        PutCode(0x1a9);
        BAsmCode[CodeLen  ] = 0xa0 + (AdrLong >> 24);
        BAsmCode[CodeLen + 1] = AdrLong & 0xff;
        CodeLen+=2;
        break;
    }
  }
  else if ((!strcasecmp(ArgStr[2], "C")) || (!strcasecmp(ArgStr[2], "CY")))
  {
    switch (DecodeBitAdr(ArgStr[1], &AdrLong, True))
    {
      case ModBit51:
        PutCode(0x92);
        BAsmCode[CodeLen] = AdrLong & 0xff;
        CodeLen++;
        break;
      case ModBit251:
        PutCode(0x1a9);
        BAsmCode[CodeLen] = 0x90 + (AdrLong >> 24);
        BAsmCode[CodeLen + 1] = AdrLong & 0xff;
        CodeLen+=2;
        break;
    }
  }
  else if (!strcasecmp(ArgStr[1], "DPTR"))
  {
    SetOpSize((MomCPU == CPU80C390) ? 3 : 1);
    DecodeAdr(ArgStr[2], MModImm);
    switch (AdrMode)
    {
      case ModImm:
        PutCode(0x90);
        memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
        TransferAdrRelocs(CodeLen);
        CodeLen += AdrCnt;
        break;
    }
  }
  else
  {
    DecodeAdr(ArgStr[1], MModAcc | MModReg | MModIReg8 | MModIReg | MModInd | MModDir8 | MModDir16);
    switch (AdrMode)
    {
      case ModAcc:
        DecodeAdr(ArgStr[2], MModReg | MModIReg8 | MModIReg | MModInd | MModDir8 | MModDir16 | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            if ((AdrPart < 8) && (!SrcMode))
              PutCode(0xe8 + AdrPart);
            else if (MomCPU < CPU80251) WrError(1505);
            else
            {
              PutCode(0x17c);
              BAsmCode[CodeLen++] = (AccReg << 4) + AdrPart;
            }
            break;
          case ModIReg8:
            PutCode(0xe6 + AdrPart);
            break;
          case ModIReg:
            PutCode(0x17e);
            BAsmCode[CodeLen++] = (AdrPart << 4) + 0x09 + AdrSize;
            BAsmCode[CodeLen++] = (AccReg << 4);
            break;
          case ModInd:
            PutCode(0x109 + (AdrSize << 4));
            BAsmCode[CodeLen++] = (AccReg << 4) + AdrPart;
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
          case ModDir8:
            PutCode(0xe5);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModDir16:
            PutCode(0x17e);
            BAsmCode[CodeLen++] = (AccReg << 4) + 0x03;
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
          case ModImm:
            PutCode(0x74);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModReg:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg | MModIReg8 | MModIReg | MModInd | MModDir8 | MModDir16 | MModImm | MModImmEx);
        switch (AdrMode)
        {
          case ModReg:
            if ((OpSize == 0) && (AdrPart == AccReg) && (HReg < 8))
              PutCode(0xf8 + HReg);
            else if ((OpSize == 0) && (HReg == AccReg) && (AdrPart < 8))
              PutCode(0xe8 + AdrPart);
            else if (MomCPU < CPU80251) WrError(1505);
            else             
            {
              PutCode(0x17c + OpSize); 
              if (OpSize == 2)
                BAsmCode[CodeLen - 1]++;
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            }
            break;
          case ModIReg8:
            if ((OpSize != 0) || (HReg != AccReg)) WrError(1350);
            else
              PutCode(0xe6 + AdrPart);
            break;
          case ModIReg:
            if (OpSize == 0)
            {
              PutCode(0x17e);
              BAsmCode[CodeLen++] = (AdrPart << 4) + 0x09 + AdrSize;
              BAsmCode[CodeLen++] = HReg << 4;
            }
            else if (OpSize == 1)
            {
              PutCode(0x10b);
              BAsmCode[CodeLen++] = (AdrPart << 4) + 0x08 + AdrSize;
              BAsmCode[CodeLen++] = HReg << 4;
            }
            else
              WrError(1350);
            break;
          case ModInd:
            if (OpSize == 2) WrError(1350);
            else
            {
              PutCode(0x109 + (AdrSize << 4) + (OpSize << 6));
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
            }
            break;
          case ModDir8:
            if ((OpSize == 0) && (HReg == AccReg))
            {
              PutCode(0xe5);
              TransferAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            else if ((OpSize == 0) && (HReg < 8) && (!SrcMode))
            {
              PutCode(0xa8 + HReg);
              TransferAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            else if (MomCPU < CPU80251) WrError(1505);
            else
            {
              PutCode(0x17e);
              BAsmCode[CodeLen++] = 0x01 + (HReg << 4) + (OpSize << 2);
              if (OpSize == 2)
                BAsmCode[CodeLen - 1] += 4;
              TransferAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            break;
          case ModDir16: 
            PutCode(0x17e);
            BAsmCode[CodeLen++] = 0x03 + (HReg << 4) + (OpSize << 2);
            if (OpSize == 2)
              BAsmCode[CodeLen - 1] += 4;
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
          case ModImm:
            if ((OpSize == 0) && (HReg == AccReg))
            {
              PutCode(0x74);
              TransferAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            else if ((OpSize == 0) && (HReg < 8) && (!SrcMode))
            {
              PutCode(0x78 + HReg);
              TransferAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            else if (MomCPU < CPU80251) WrError(1505);
            else
            {
              PutCode(0x17e);
              BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2);
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
            }
            break;
          case ModImmEx:
            PutCode(0x17e);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = 0x0c + (HReg << 4);
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
        }
        break;
      case ModIReg8:
        SetOpSize(0); HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModAcc | MModDir8 | MModImm);
        switch (AdrMode)
        {
          case ModAcc:
            PutCode(0xf6 + HReg);
            break;
          case ModDir8:
            PutCode(0xa6 + HReg);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModImm:
            PutCode(0x76 + HReg);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModIReg:
        HReg = AdrPart; HSize = AdrSize;
        DecodeAdr(ArgStr[2], MModReg);
        switch (AdrMode)
        {
          case ModReg:
            if (OpSize == 0)
            {
              PutCode(0x17a);
              BAsmCode[CodeLen++] = (HReg << 4) + 0x09 + HSize;
              BAsmCode[CodeLen++] = AdrPart << 4;
            }
            else if (OpSize == 1)
            {
              PutCode(0x11b);
              BAsmCode[CodeLen++] = (HReg << 4) + 0x08 + HSize;
              BAsmCode[CodeLen++] = AdrPart << 4;
            }
            else
              WrError(1350);
        }
        break;
      case ModInd:
        HReg = AdrPart; HSize = AdrSize;
        AdrInt = (((Word)AdrVals[0]) << 8) + AdrVals[1];
        DecodeAdr(ArgStr[2], MModReg);
        switch (AdrMode)
        {
          case ModReg:
            if (OpSize == 2) WrError(1350);
            else
            {
              PutCode(0x119 + (HSize << 4) + (OpSize << 6));
              BAsmCode[CodeLen++] = (AdrPart << 4) + HReg;
              BAsmCode[CodeLen++] = Hi(AdrInt);
              BAsmCode[CodeLen++] = Lo(AdrInt);
            }
        }
        break;
      case ModDir8:
        MinOneIs0 = True;
        HReg = AdrVals[0];
        SaveBackupAdrRelocs();
        DecodeAdr(ArgStr[2], MModReg | MModIReg8 | MModDir8 | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            if ((OpSize == 0) && (AdrPart == AccReg))
            {
              PutCode(0xf5);
              TransferBackupAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = HReg;
            }
            else if ((OpSize == 0) && (AdrPart < 8) && (!SrcMode))
            {
              PutCode(0x88 + AdrPart);
              TransferBackupAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = HReg;
            }
            else if (MomCPU < CPU80251) WrError(1505);
            else
            {
              PutCode(0x17a);
              BAsmCode[CodeLen++] = 0x01 + (AdrPart << 4) + (OpSize << 2);
              if (OpSize == 2)
                BAsmCode[CodeLen - 1] += 4;
              BAsmCode[CodeLen++] = HReg;
            }
            break;
          case ModIReg8:
            PutCode(0x86 + AdrPart);
            TransferBackupAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = HReg;
            break;
          case ModDir8:
            PutCode(0x85);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            TransferBackupAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = HReg;
            break;
          case ModImm:
            PutCode(0x75);
            TransferBackupAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = HReg;
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModDir16:
        AdrInt = (((Word)AdrVals[0]) << 8) + AdrVals[1];
        DecodeAdr(ArgStr[2], MModReg);
        switch (AdrMode)
        {
          case ModReg:
            PutCode(0x17a);
            BAsmCode[CodeLen++] = 0x03 + (AdrPart << 4) + (OpSize << 2);
            if (OpSize == 2) BAsmCode[CodeLen - 1] += 4;
            BAsmCode[CodeLen++] = Hi(AdrInt);
            BAsmCode[CodeLen++] = Lo(AdrInt);
            break;
        }
        break;
    }
  }
}

static void DecodeLogic(Word Index)
{
  Byte HReg;
  LongInt AdrLong;
  int z;

  /* Index: ORL=0 ANL=1 XRL=2 */

  if (ArgCnt != 2) WrError(1110);
  else if (IsCarry(ArgStr[1]))
  {
    if (Index == 2) WrError(1350);
    else
    {
      Boolean InvFlag;
      char *pArg2 = ArgStr[2];

      HReg = Index << 4;
      InvFlag = (*pArg2 == '/');
      if (InvFlag)
        pArg2++;
      switch (DecodeBitAdr(pArg2, &AdrLong, True))
      {
        case ModBit51:
          PutCode((InvFlag) ? 0xa0 + HReg : 0x72 + HReg);
          BAsmCode[CodeLen++] = AdrLong & 0xff;
          break;
        case ModBit251:
          PutCode(0x1a9);
          BAsmCode[CodeLen++] = ((InvFlag) ? 0xe0 : 0x70) + HReg + (AdrLong >> 24);
          BAsmCode[CodeLen++] = AdrLong & 0xff;
          break;
      }
    }
  }
  else
  {
    z = (Index << 4) + 0x40;
    DecodeAdr(ArgStr[1], MModAcc | MModReg | MModDir8);
    switch (AdrMode)
    {
      case ModAcc:
        DecodeAdr(ArgStr[2], MModReg | MModIReg8 | MModIReg | MModDir8 | MModDir16 | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            if ((AdrPart < 8) && (!SrcMode)) PutCode(z + 8 + AdrPart);
            else
            {
              PutCode(z + 0x10c);
              BAsmCode[CodeLen++] = AdrPart + (AccReg << 4);
            }
            break;
          case ModIReg8:
            PutCode(z + 6 + AdrPart);
            break;
          case ModIReg:
            PutCode(z + 0x10e);
            BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
            BAsmCode[CodeLen++] = AccReg << 4;
            break;
          case ModDir8:
            PutCode(z + 0x05);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModDir16:
            PutCode(0x10e + z);
            BAsmCode[CodeLen++] = 0x03 + (AccReg << 4);
            memcpy(BAsmCode+CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
          case ModImm:
            PutCode(z + 0x04);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModReg:
        if (MomCPU < CPU80251) WrError(1350);
        else
        {
          HReg = AdrPart;
          DecodeAdr(ArgStr[2], MModReg | MModIReg8 | MModIReg | MModDir8 | MModDir16 | MModImm);
          switch (AdrMode)
          {
            case ModReg:
              if (OpSize == 2) WrError(1350);
              else
              {
                PutCode(z + 0x10c + OpSize);
                BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              }
              break;
            case ModIReg8:
              if ((OpSize != 0) || (HReg != AccReg)) WrError(1350);
              else
                PutCode(z + 0x06 + AdrPart);
              break;
            case ModIReg:
              if (OpSize != 0) WrError(1350);
              else
              {
                PutCode(0x10e + z);
                BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
                BAsmCode[CodeLen++] = HReg << 4;
              }
              break;
            case ModDir8:
              if ((OpSize == 0) && (HReg == AccReg))
              {
                PutCode(0x05 + z);
                TransferAdrRelocs(CodeLen);
                BAsmCode[CodeLen++] = AdrVals[0];
              }
              else if (OpSize == 2) WrError(1350);
              else
              {
                PutCode(0x10e + z);
                BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 1;
                BAsmCode[CodeLen++] = AdrVals[0];
              }
              break;
            case ModDir16:
              if (OpSize == 2) WrError(1350);
              else
              {
                PutCode(0x10e + z);
                BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 3;
                memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                CodeLen += AdrCnt;
              }
              break;
            case ModImm:
              if ((OpSize == 0) && (HReg == AccReg))
              {
                PutCode(0x04 + z);
                TransferAdrRelocs(CodeLen);
                BAsmCode[CodeLen++] = AdrVals[0];
              }
              else if (OpSize == 2) WrError(1350);
              else
              {
                PutCode(0x10e + z);
                BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2);
                TransferAdrRelocs(CodeLen);
                memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                CodeLen += AdrCnt;
              }
              break;
          }
        }
        break;
      case ModDir8:
        HReg = AdrVals[0];
        SaveBackupAdrRelocs();
        SetOpSize(0);
        DecodeAdr(ArgStr[2], MModAcc | MModImm);
        switch (AdrMode)
        {
          case ModAcc:
            PutCode(z + 0x02);
            TransferBackupAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = HReg;
            break;
          case ModImm:
            PutCode(z + 0x03);
            TransferBackupAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = HReg;
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
    }
  }
}

static void DecodeMOVC(Word Index)
{
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModAcc);
    switch (AdrMode)
    {
      case ModAcc:
        if (!strcasecmp(ArgStr[2], "@A+DPTR"))
          PutCode(0x93);
        else if (!strcasecmp(ArgStr[2], "@A+PC"))
          PutCode(0x83);
        else
          WrError(1350);
        break;
    }
  }
}

static void DecodeMOVH(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPU80251) WrError(1500);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    switch (AdrMode)
    {
      case ModReg:
        if (OpSize != 2) WrError(1350);
        else
        {
          HReg = AdrPart;
          OpSize--;
          DecodeAdr(ArgStr[2], MModImm);
          switch (AdrMode)
          {
            case ModImm:
              PutCode(0x17a);
              BAsmCode[CodeLen++] = 0x0c + (HReg << 4);
              TransferAdrRelocs(CodeLen);
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
              break;
          }
        }
        break;
    }
  }
}

static void DecodeMOVZS(Word Index)
{
  Byte HReg;
  int z;
  UNUSED(Index);

  z = Ord(Memo("MOVS")) << 4;
  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPU80251) WrError(1500);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    switch (AdrMode)
    {
      case ModReg:
        if (OpSize != 1) WrError(1350);
        else
        {
          HReg = AdrPart;
          OpSize--;
          DecodeAdr(ArgStr[2], MModReg);
          switch (AdrMode)
          {
            case ModReg:
             PutCode(0x10a + z);
             BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
             break;
          }
        }
        break;
    }
  }
}

static void DecodeMOVX(Word Index)
{
  int z;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    z = 0;
    if ((!strcasecmp(ArgStr[2], "A")) || ((MomCPU >= CPU80251) && (!strcasecmp(ArgStr[2], "R11"))))
    {
      z = 0x10;
      strcpy(ArgStr[2], ArgStr[1]);
      strmaxcpy(ArgStr[1], "A", 255);
    }
    if ((strcasecmp(ArgStr[1], "A")) && ((MomCPU < CPU80251) || (!strcasecmp(ArgStr[2], "R11")))) WrError(1350);
    else if (!strcasecmp(ArgStr[2], "@DPTR"))
      PutCode(0xe0 + z);
    else
    {
      DecodeAdr(ArgStr[2], MModIReg8);
      switch (AdrMode)
      {
        case ModIReg8:
          PutCode(0xe2 + AdrPart + z);
          break;
      }
    }
  }
}

static void DecodeStack(Word Index)
{
  int z;

  /* Index: PUSH=0 POP=1 PUSHW=2 */

  z = (Index & 1) << 4;
  if (ArgCnt != 1) WrError(1110);
  else
  {
    if (*ArgStr[1] == '#')
      SetOpSize(Ord(Index == 2));
    DecodeAdr(ArgStr[1], MModDir8 | MModReg | ((z == 0x10) ? 0 : MModImm));
    switch (AdrMode)
    {
      case ModDir8:
        PutCode(0xc0 + z);
        TransferAdrRelocs(CodeLen);
        BAsmCode[CodeLen++] = AdrVals[0];
        break;
      case ModReg:
        if (MomCPU < CPU80251) WrError(1505);
        else
        {
          PutCode(0x1ca + z);
          BAsmCode[CodeLen++] = 0x08 + (AdrPart << 4) + OpSize + (Ord(OpSize == 2));
        }
        break;
      case ModImm:
        if (MomCPU < CPU80251) WrError(1505);
        else
        {
          PutCode(0x1ca);
          BAsmCode[CodeLen++] = 0x02 + (OpSize << 2);
          TransferAdrRelocs(CodeLen);
          memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
          CodeLen += AdrCnt;
        }
        break;
    }
  }
}

static void DecodeXCH(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModAcc | MModReg | MModIReg8 | MModDir8);
    switch (AdrMode)
    {
      case ModAcc:
        DecodeAdr(ArgStr[2], MModReg | MModIReg8 | MModDir8);
        switch (AdrMode)
        {
          case ModReg:
            if (AdrPart > 7) WrError(1350);
            else
              PutCode(0xc8 + AdrPart);
            break;
          case ModIReg8:
            PutCode(0xc6 + AdrPart);
            break;
          case ModDir8:
            PutCode(0xc5);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
      case ModReg:
        if ((OpSize != 0) || (AdrPart > 7)) WrError(1350);
        else
        {
          HReg = AdrPart;
          DecodeAdr(ArgStr[2], MModAcc);
          switch (AdrMode)
          {
            case ModAcc:
              PutCode(0xc8 + HReg);
              break;
          }
        }
        break;
      case ModIReg8:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModAcc);
        switch (AdrMode)
        {
          case ModAcc:
            PutCode(0xc6 + HReg);
            break;
        }
        break;
      case ModDir8:
        HReg = AdrVals[0]; SaveBackupAdrRelocs();
        DecodeAdr(ArgStr[2], MModAcc);
        switch (AdrMode)
        {
          case ModAcc:
            PutCode(0xc5);
            TransferBackupAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = HReg;
            break;
        }
        break;
    }
  }
}

static void DecodeXCHD(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModAcc | MModIReg8);
    switch (AdrMode)
    {
      case ModAcc:
        DecodeAdr(ArgStr[2], MModIReg8);
        switch (AdrMode)
        {
          case ModIReg8:
            PutCode(0xd6 + AdrPart);
            break;
        }
        break;
      case ModIReg8:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModAcc);
        switch (AdrMode)
        {
          case ModAcc:
            PutCode(0xd6 + HReg);
            break;
        }
        break;
    }
  }
}

#define RelocTypeABranch11 (11 | RelocFlagBig | RelocFlagPage | (5 << 8) | (3 << 12)) | (0 << 16)
#define RelocTypeABranch19 (19 | RelocFlagBig | RelocFlagPage | (5 << 8) | (3 << 12)) | (0 << 16)

static void DecodeABranch(Word Index)
{
  Boolean OK;
  LongInt AdrLong;

  /* Index: AJMP = 0 ACALL = 1 */

  if (ArgCnt != 1) WrError(1110);
  else
  {
    AdrLong = EvalIntExpression(ArgStr[1], Int24, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      if (MomCPU == CPU80C390)
      {
        if ((!SymbolQuestionable) && (((((long)EProgCounter()) + 3) >> 19) != (AdrLong >> 19))) WrError(1910);
        else
        {
          PutCode(0x01 + (Index << 4) + (((AdrLong >> 16) & 7) << 5));
          BAsmCode[CodeLen++] = Hi(AdrLong);
          BAsmCode[CodeLen++] = Lo(AdrLong);
          TransferRelocs(ProgCounter() - 3, RelocTypeABranch19);
        }
      }
      else
      {
        if ((!SymbolQuestionable) && (((((long)EProgCounter()) + 2) >> 11) != (AdrLong >> 11))) WrError(1910);
        else if (Chk504(EProgCounter())) WrError(1900);
        else
        {
          PutCode(0x01 + (Index << 4) + ((Hi(AdrLong) & 7) << 5));
          BAsmCode[CodeLen++] = Lo(AdrLong);
          TransferRelocs(ProgCounter() - 2, RelocTypeABranch11);
        }
      }
    }
  }
}

static void DecodeLBranch(Word Index)
{
  LongInt AdrLong;
  Boolean OK;

  /* Index: LJMP=0 LCALL=1 */

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < CPU8051) WrError(1500);
  else if (*ArgStr[1] == '@')
  {
    DecodeAdr(ArgStr[1], MModIReg);
    switch (AdrMode)
    {
      case ModIReg:
        if (AdrSize != 0) WrError(1350);
        else
        {
          PutCode(0x189 + (Index << 4));
          BAsmCode[CodeLen++] = 0x04 + (AdrPart << 4);
        }
        break;
    }
  }
  else
  {
    AdrLong = EvalIntExpression(ArgStr[1], (MomCPU < CPU80C390) ? Int16 : Int24, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      if (MomCPU == CPU80C390)
      {
        PutCode(0x02 + (Index << 4));
        BAsmCode[CodeLen++] = (AdrLong >> 16) & 0xff;
        BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
        BAsmCode[CodeLen++] = AdrLong & 0xff;
        TransferRelocs(ProgCounter() + 1, RelocTypeB24);
      }
      else
      {
        if ((MomCPU >= CPU80251) && (((((long)EProgCounter()) + 3) >> 16) != (AdrLong >> 16))) WrError(1910);
        else
        {
          ChkSpace(SegCode);
          PutCode(0x02 + (Index << 4));
          BAsmCode[CodeLen++] = (AdrLong >> 8) & 0xff;
          BAsmCode[CodeLen++] = AdrLong & 0xff;
          TransferRelocs(ProgCounter() + 1, RelocTypeB16);
        }
      }
    }
  }
}

static void DecodeEBranch(Word Index)
{
  LongInt AdrLong;
  Boolean OK;

  /* Index: AJMP=0 ACALL=1 */

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < CPU80251) WrError(1500);
  else if (*ArgStr[1] == '@')
  {
    DecodeAdr(ArgStr[1], MModIReg);
    switch (AdrMode)
    {
      case ModIReg:
        if (AdrSize != 2) WrError(1350);
        else
        {
          PutCode(0x189 + (Index << 4));
          BAsmCode[CodeLen++] = 0x08 + (AdrPart << 4);
        }
        break;
    }
  }
  else
  {
    AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
    if (OK)
    {
      ChkSpace(SegCode);
      PutCode(0x18a + (Index << 4));
      BAsmCode[CodeLen++] = (AdrLong >> 16) & 0xff;
      BAsmCode[CodeLen++] = (AdrLong >>  8) & 0xff;
      BAsmCode[CodeLen++] =  AdrLong        & 0xff;
    }
  }
}

static void DecodeJMP(Word Index)
{
  LongInt AdrLong, Dist;
  Boolean OK;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "@A+DPTR"))
    PutCode(0x73);
  else if (*ArgStr[1] == '@')
  {
    DecodeAdr(ArgStr[1], MModIReg);
    switch (AdrMode)
    {
      case ModIReg:
        PutCode(0x189);
        BAsmCode[CodeLen++] = 0x04 + (AdrSize << 1) + (AdrPart << 4);
        break;
    }
  }
  else
  {
    AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
    if (OK)
    {
      Dist = AdrLong - (EProgCounter() + 2);
      if ((Dist<=127) && (Dist >= -128))
      {
        PutCode(0x80);
        BAsmCode[CodeLen++] = Dist & 0xff;
      }
      else if ((!Chk504(EProgCounter())) && ((AdrLong >> 11) == ((((long)EProgCounter()) + 2) >> 11)))
      {
        PutCode(0x01 + ((Hi(AdrLong) & 7) << 5));
        BAsmCode[CodeLen++] = Lo(AdrLong);
      }
      else if (MomCPU < CPU8051) WrError(1910);
      else if (((((long)EProgCounter()) + 3) >> 16) == (AdrLong >> 16))
      {
        PutCode(0x02);
        BAsmCode[CodeLen++] = Hi(AdrLong); 
        BAsmCode[CodeLen++] = Lo(AdrLong);
      }
      else if (MomCPU < CPU80251) WrError(1910);
      else
      {
        PutCode(0x18a);
        BAsmCode[CodeLen++] = (AdrLong >> 16) & 0xff;
        BAsmCode[CodeLen++] = (AdrLong >>  8) & 0xff;
        BAsmCode[CodeLen++] =  AdrLong        & 0xff;
      }
    }
  }
}

static void DecodeCALL(Word Index)
{
  LongInt AdrLong;
  Boolean OK;
  UNUSED(Index);

    if (ArgCnt != 1) WrError(1110);
    else if (*ArgStr[1] == '@')
    {
      DecodeAdr(ArgStr[1], MModIReg);
      switch (AdrMode)
      {
        case ModIReg:
          PutCode(0x199);
          BAsmCode[CodeLen++] = 0x04 + (AdrSize << 1) + (AdrPart << 4);
          break;
      }
    }
    else
    {
      AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
      if (OK)
      {
        if ((!Chk504(EProgCounter())) && ((AdrLong >> 11) == ((((long)EProgCounter()) + 2) >> 11)))
        {
          PutCode(0x11 + ((Hi(AdrLong) & 7) << 5));
          BAsmCode[CodeLen++] = Lo(AdrLong);
        }
        else if (MomCPU < CPU8051) WrError(1910);
        else if ((AdrLong >> 16) != ((((long)EProgCounter()) + 3) >> 16)) WrError(1910);
        else
        {
          PutCode(0x12);
          BAsmCode[CodeLen++] = Hi(AdrLong);
          BAsmCode[CodeLen++] = Lo(AdrLong);
        }
      }
    }
}

static void DecodeDJNZ(Word Index)
{
  LongInt AdrLong;
  Boolean OK, Questionable;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    AdrLong = EvalIntExpression(ArgStr[2], UInt24, &OK);
    SubPCRefReloc();
    Questionable = SymbolQuestionable;
    if (OK)
    {
      DecodeAdr(ArgStr[1], MModReg | MModDir8);
      switch (AdrMode)
      {
        case ModReg:
          if ((OpSize != 0) || (AdrPart > 7)) WrError(1350);
          else
          {
            AdrLong -= EProgCounter() + 2 + Ord(NeedsPrefix(0xd8 + AdrPart));
            if (((AdrLong < -128) || (AdrLong > 127)) && (!Questionable)) WrError(1370);
            else
            {
              PutCode(0xd8 + AdrPart);
              BAsmCode[CodeLen++] = AdrLong & 0xff;
            }
          }
          break;
        case ModDir8:
          AdrLong -= EProgCounter() + 3 + Ord(NeedsPrefix(0xd5));
          if (((AdrLong < -128) || (AdrLong > 127)) && (!Questionable)) WrError(1370);
          else
          {
            PutCode(0xd5);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            BAsmCode[CodeLen++] = Lo(AdrLong);
          }
          break;
      }
    }
  }
}

static void DecodeCJNE(Word Index)
{
  LongInt AdrLong;
  Boolean OK, Questionable;
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 3) WrError(1110);
  else
  {
    AdrLong = EvalIntExpression(ArgStr[3], UInt24, &OK);
    SubPCRefReloc();
    Questionable = SymbolQuestionable;
    if (OK)
    {
      DecodeAdr(ArgStr[1], MModAcc | MModIReg8 | MModReg);
      switch (AdrMode)
      {
        case ModAcc:
          DecodeAdr(ArgStr[2], MModDir8 | MModImm);
          switch (AdrMode)
          {
            case ModDir8:
              AdrLong -= EProgCounter() + 3 + Ord(NeedsPrefix(0xb5));
              if (((AdrLong < -128) || (AdrLong > 127)) && (!Questionable)) WrError(1370);
              else
              {
                PutCode(0xb5);
                TransferAdrRelocs(CodeLen);
                BAsmCode[CodeLen++] = AdrVals[0];
                BAsmCode[CodeLen++] = AdrLong & 0xff;
              }
              break;
            case ModImm:
              AdrLong -= EProgCounter() + 3 + Ord(NeedsPrefix(0xb5));
              if (((AdrLong < -128) || (AdrLong > 127)) && (!Questionable)) WrError(1370);
              else
              {
                PutCode(0xb4);
                TransferAdrRelocs(CodeLen);
                BAsmCode[CodeLen++] = AdrVals[0];
                BAsmCode[CodeLen++] = AdrLong & 0xff;
              }
              break;
          }
          break;
        case ModReg:
          if ((OpSize != 0) || (AdrPart > 7)) WrError(1350);
          else
          {
            HReg = AdrPart;
            DecodeAdr(ArgStr[2], MModImm);
            switch (AdrMode)
            {
              case ModImm:
                AdrLong -= EProgCounter() + 3 + Ord(NeedsPrefix(0xb8 + HReg));
                if (((AdrLong < -128) || (AdrLong > 127)) && (!Questionable)) WrError(1370);
                else
                {
                  PutCode(0xb8 + HReg);
                  TransferAdrRelocs(CodeLen);
                  BAsmCode[CodeLen++] = AdrVals[0];
                  BAsmCode[CodeLen++] = AdrLong & 0xff;
                }
                break;
            }
          }
          break;
        case ModIReg8:
          HReg = AdrPart; SetOpSize(0);
          DecodeAdr(ArgStr[2], MModImm);
          switch (AdrMode)
          {
            case ModImm:
              AdrLong -= EProgCounter() + 3 + Ord(NeedsPrefix(0xb6 + HReg));
              if (((AdrLong < -128) || (AdrLong > 127)) && (!Questionable)) WrError(1370);
              else
              {
                PutCode(0xb6 + HReg);
                TransferAdrRelocs(CodeLen);
                BAsmCode[CodeLen++] = AdrVals[0];
                BAsmCode[CodeLen++] = AdrLong & 0xff;
              }
              break;
          }
          break;
      }
    }
  }
}

static void DecodeADD(Word Index)
{
  Byte HReg;
  UNUSED(Index);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModAcc | MModReg);
    switch (AdrMode)
    {
      case ModAcc:
        DecodeAdr(ArgStr[2], MModImm | MModDir8 | MModDir16 | MModIReg8 | MModIReg | MModReg);
        switch (AdrMode)
        {
          case ModImm:
            PutCode(0x24);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModDir8:
            PutCode(0x25);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModDir16:
            PutCode(0x12e);
            BAsmCode[CodeLen++] = (AccReg << 4) + 3;
            memcpy(BAsmCode + CodeLen, AdrVals, 2);
            CodeLen += 2;
            break;
          case ModIReg8:
            PutCode(0x26 + AdrPart);
            break;
          case ModIReg:
            PutCode(0x12e);
            BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
            BAsmCode[CodeLen++] = AccReg << 4;
            break;
          case ModReg:
            if ((AdrPart < 8) && (!SrcMode)) PutCode(0x28 + AdrPart);
            else if (MomCPU < CPU80251) WrError(1505);
            else
            {
              PutCode(0x12c);
              BAsmCode[CodeLen++] = AdrPart + (AccReg << 4);
            }
            break;
        }
        break;
      case ModReg:
        if (MomCPU < CPU80251) WrError(1505);
        else
        {
          HReg = AdrPart;
          DecodeAdr(ArgStr[2], MModImm | MModReg | MModDir8 | MModDir16 | MModIReg8 | MModIReg);
          switch (AdrMode)
          {
            case ModImm:
              if ((OpSize == 0) && (HReg == AccReg))
              {
                PutCode(0x24);
                TransferAdrRelocs(CodeLen);
                BAsmCode[CodeLen++] = AdrVals[0];
              }
              else
              {
                PutCode(0x12e);
                BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2);
                memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                CodeLen += AdrCnt;
              }
              break;
            case ModReg:
              PutCode(0x12c + OpSize); 
              if (OpSize == 2) BAsmCode[CodeLen - 1]++;
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
              break;
            case ModDir8:
              if (OpSize == 2) WrError(1350);
              else if ((OpSize == 0) && (HReg == AccReg))
              {
                PutCode(0x25);
                TransferAdrRelocs(CodeLen);
                BAsmCode[CodeLen++] = AdrVals[0];
              }
              else
              {
                PutCode(0x12e);
                BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 1;
                BAsmCode[CodeLen++] = AdrVals[0];
              }
              break;
            case ModDir16:
              if (OpSize == 2) WrError(1350);
              else
              {
                PutCode(0x12e);
                BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 3;
                memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
                CodeLen += AdrCnt;
              }
              break;
            case ModIReg8:
              if ((OpSize != 0) || (HReg != AccReg)) WrError(1350);
              else PutCode(0x26 + AdrPart);
              break;
            case ModIReg:
              if (OpSize != 0) WrError(1350);
              else
              {
                PutCode(0x12e);
                BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
                BAsmCode[CodeLen++] = HReg << 4;
              }
              break;
          }
        }
        break;
    }
  }
}

static void DecodeSUBCMP(Word Index)
{
  int z;
  Byte HReg;

  /* Index: SUB=0 CMP=1 */

  z = 0x90 + (Index << 5);
  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU < CPU80251) WrError(1500);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    switch (AdrMode)
    {
      case ModReg:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModImm | MModReg | MModDir8 | MModDir16 | MModIReg | (Index ? MModImmEx : 0));
        switch (AdrMode)
        {
          case ModImm:
            PutCode(0x10e + z);
            BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2);
            TransferAdrRelocs(CodeLen);
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
          case ModImmEx:
            PutCode(0x10e + z);
            BAsmCode[CodeLen++] = (HReg << 4) + 0x0c;
            TransferAdrRelocs(CodeLen);
            memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
            CodeLen += AdrCnt;
            break;
          case ModReg:
            PutCode(0x10c + z + OpSize);
            if (OpSize == 2)
              BAsmCode[CodeLen - 1]++;
            BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            break;
          case ModDir8:
            if (OpSize == 2) WrError(1350);
            else
            {
              PutCode(0x10e + z);
              BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 1;
              TransferAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            break;
          case ModDir16:
            if (OpSize == 2) WrError(1350);
            else
            {
              PutCode(0x10e + z);
              BAsmCode[CodeLen++] = (HReg << 4) + (OpSize << 2) + 3;
              memcpy(BAsmCode + CodeLen, AdrVals, AdrCnt);
              CodeLen += AdrCnt;
            }
            break;
          case ModIReg:
            if (OpSize != 0) WrError(1350);
            else
            {
              PutCode(0x10e + z);
              BAsmCode[CodeLen++] = 0x09 + AdrSize + (AdrPart << 4);
              BAsmCode[CodeLen++] = HReg << 4;
            }
            break;
        }
        break;
    }
  }
}

static void DecodeADDCSUBB(Word Index)
{
  Byte HReg;

  /* Index: ADDC=0 SUBB=1 */

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModAcc);
    switch (AdrMode)
    {
      case ModAcc:
        HReg = 0x30 + (Index*0x60);
        DecodeAdr(ArgStr[2], MModReg | MModIReg8 | MModDir8 | MModImm);
        switch (AdrMode)
        {
          case ModReg:
            if (AdrPart > 7) WrError(1350);
            else
              PutCode(HReg + 0x08 + AdrPart);
            break;
          case ModIReg8:
            PutCode(HReg + 0x06 + AdrPart);
            break;
          case ModDir8:
            PutCode(HReg + 0x05);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
          case ModImm:
            PutCode(HReg + 0x04);
            TransferAdrRelocs(CodeLen);
            BAsmCode[CodeLen++] = AdrVals[0];
            break;
        }
        break;
    }
  }
}

static void DecodeINCDEC(Word Index)
{
  Byte HReg;
  int z;
  Boolean OK;
  const char *pArg2 = (ArgCnt < 2) ? "#1" : ArgStr[2];

  /* Index: INC=0 DEC=1 */

  z = Index << 4;
  if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
  else if (*pArg2 != '#') WrError(1350);
  else
  {
    FirstPassUnknown = False;
    HReg = EvalIntExpression(pArg2 + 1, UInt3, &OK);
    if (FirstPassUnknown)
      HReg = 1;
    if (OK)
    {
      OK = True;
      if (HReg == 1)
        HReg = 0;
      else if (HReg == 2)
        HReg = 1;
      else if (HReg == 4)
        HReg = 2;
      else
        OK = False;
      if (!OK) WrError(1320);
      else if (!strcasecmp(ArgStr[1], "DPTR"))
      {
        if (Index == 1) WrError(1350);
        else if (HReg != 0) WrError(1320);
        else
          PutCode(0xa3);
      }
      else
      {
        DecodeAdr(ArgStr[1], MModAcc | MModReg | MModDir8 | MModIReg8);
        switch (AdrMode)
        {
          case ModAcc:
            if (HReg == 0)
              PutCode(0x04 + z);
            else if (MomCPU < CPU80251) WrError(1320);
            else
            {
              PutCode(0x10b + z);
              BAsmCode[CodeLen++] = (AccReg << 4) + HReg;
            }
            break;
          case ModReg:
            if ((OpSize == 0) && (AdrPart == AccReg) && (HReg == 0))
              PutCode(0x04 + z);
            else if ((AdrPart < 8) && (OpSize == 0) && (HReg == 0) && (!SrcMode))
              PutCode(0x08 + z + AdrPart);
            else if (MomCPU < CPU80251) WrError(1505);
            else
            {
              PutCode(0x10b + z);
              BAsmCode[CodeLen++] = (AdrPart << 4) + (OpSize << 2) + HReg;
              if (OpSize == 2)
                BAsmCode[CodeLen - 1] += 4;
            }
            break;
          case ModDir8:
            if (HReg != 0) WrError(1320);
            else
            {
              PutCode(0x05 + z);
              TransferAdrRelocs(CodeLen);
              BAsmCode[CodeLen++] = AdrVals[0];
            }
            break;
          case ModIReg8:
            if (HReg != 0) WrError(1320);
            else
              PutCode(0x06 + z + AdrPart);
            break;
        }
      }
    }
  }
}

static void DecodeMULDIV(Word Index)
{
  int z;
  Byte HReg;

  /* Index: DIV=0 MUL=1 */

  z = Index << 5;
  if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
  else if (ArgCnt == 1) 
  {
    if (strcasecmp(ArgStr[1], "AB")) WrError(1350);
    else
      PutCode(0x84 + z);
  }
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    switch (AdrMode)
    {
      case ModReg:
        HReg = AdrPart;
        DecodeAdr(ArgStr[2], MModReg);
        switch (AdrMode)
        {
          case ModReg:
            if (MomCPU < CPU80251) WrError(1505);
            else if (OpSize == 2) WrError(1350);
            else
            {
              PutCode(0x18c + z + OpSize);
              BAsmCode[CodeLen++] = (HReg << 4) + AdrPart;
            }
            break;
        }
        break;
    }
  }
}

static void DecodeBits(Word Index)
{
  LongInt AdrLong;
  int z;

  /* Index: CPL=0 CLR=1 SETB=2 */

  z = Index << 4;
  if (ArgCnt != 1) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "A"))
  {
    if (Memo("SETB")) WrError(1350);
    else
      PutCode(0xf4 - z);
  }
  else if (IsCarry(ArgStr[1]))
    PutCode(0xb3 + z);
  else 
    switch (DecodeBitAdr(ArgStr[1], &AdrLong, True))
    {
      case ModBit51:
        PutCode(0xb2 + z);
        BAsmCode[CodeLen++] = AdrLong & 0xff;
        break;
      case ModBit251:
        PutCode(0x1a9);
        BAsmCode[CodeLen++] = 0xb0 + z + (AdrLong >> 24);
        BAsmCode[CodeLen++] = AdrLong & 0xff;
        break;
    }
}

static void DecodeShift(Word Index)
{
  int z;

  /* Index: SRA=0 SRL=1 SLL=3 */

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < CPU80251) WrError(1500);
  else
  {
    z = Index << 4;
    DecodeAdr(ArgStr[1], MModReg);
    switch (AdrMode)
    {
      case ModReg:
        if (OpSize == 2) WrError(1350);
        else
        {
          PutCode(0x10e + z);
          BAsmCode[CodeLen++] = (AdrPart << 4) + (OpSize << 2);
        }
        break;
    }
  }
}

static void DecodeCond(Word Index)
{
  FixedOrder *FixedZ = CondOrders + Index;
  LongInt AdrLong;
  Boolean OK;

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < FixedZ->MinCPU) WrError(1500);
  else
  {
    AdrLong = EvalIntExpression(ArgStr[1], UInt24, &OK);
    SubPCRefReloc();
    if (OK)
    {
      AdrLong -= EProgCounter() + 2 + Ord(NeedsPrefix(FixedZ->Code));
      if (((AdrLong < -128) || (AdrLong > 127)) && (!SymbolQuestionable)) WrError(1370);
      else
      {
        ChkSpace(SegCode);
        PutCode(FixedZ->Code);
        BAsmCode[CodeLen++] = AdrLong & 0xff;
      }
    }
  }
}

static void DecodeBCond(Word Index)
{
  FixedOrder *FixedZ = BCondOrders + Index;
  LongInt AdrLong, BitLong;
  Boolean OK, Questionable;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    AdrLong = EvalIntExpression(ArgStr[2], UInt24, &OK);
    SubPCRefReloc();
    Questionable = SymbolQuestionable;
    if (OK)
    {
      ChkSpace(SegCode);
      switch (DecodeBitAdr(ArgStr[1], &BitLong, True))
      {
        case ModBit51:
          AdrLong -= EProgCounter() + 3 + Ord(NeedsPrefix(FixedZ->Code));
          if (((AdrLong < -128) || (AdrLong > 127)) && (!Questionable)) WrError(1370);
          else
          {
            PutCode(FixedZ->Code);
            BAsmCode[CodeLen++] = BitLong & 0xff;
            BAsmCode[CodeLen++] = AdrLong & 0xff;
          }
          break;
        case ModBit251:
          AdrLong -= EProgCounter() + 4 + Ord(NeedsPrefix(0x1a9));
          if (((AdrLong < -128) || (AdrLong > 127)) && (!Questionable)) WrError(1370);
          else
          {
            PutCode(0x1a9);
            BAsmCode[CodeLen++] = FixedZ->Code + (BitLong >> 24);
            BAsmCode[CodeLen++] = BitLong & 0xff;
            BAsmCode[CodeLen++] = AdrLong & 0xff;
          }
          break;
      }
    }
  }
}

static void DecodeAcc(Word Index)
{
  FixedOrder *FixedZ = AccOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < FixedZ->MinCPU) WrError(1500);
  else
  {
    DecodeAdr(ArgStr[1], MModAcc);
    switch (AdrMode)
    {
      case ModAcc: 
        PutCode(FixedZ->Code);
        break;
    }
  }
}

static void DecodeFixed(Word Index)
{
  FixedOrder *FixedZ = FixedOrders + Index;

  if (ArgCnt != 0) WrError(1110);
  else if (MomCPU < FixedZ->MinCPU) WrError(1500);
  else
    PutCode(FixedZ->Code);
}


static void DecodeSFR(Word Index)
{
  Word AdrByte;
  Boolean OK;
  int DSeg;
  UNUSED(Index);

  FirstPassUnknown = False;
  if (ArgCnt != 1) WrError(1110);
  else if ((Memo("SFRB")) && (MomCPU >= CPU80251)) WrError(1500);
  else
  {
    AdrByte = EvalIntExpression(ArgStr[1], (MomCPU >= CPU80251) ? UInt9 : UInt8, &OK);
    if ((OK) && (!FirstPassUnknown))
    {
      PushLocHandle(-1);
      DSeg = (MomCPU >= CPU80251) ? SegIO : SegData;
      EnterIntSymbol(LabPart, AdrByte, DSeg, False);
      if (MakeUseList)
      {
        if (AddChunk(SegChunks + DSeg, AdrByte, 1, False))
          WrError(90);
      }
      if (Memo("SFRB"))
      {
        Byte BitStart;

        if (AdrByte > 0x7f)
        {
          if ((AdrByte & 7) != 0) WrError(220);
          BitStart = AdrByte;
        }
        else
        {
          if ((AdrByte & 0xe0) != 0x20) WrError(220);
          BitStart = (AdrByte - 0x20) << 3;
        }
        if (MakeUseList)
          if (AddChunk(SegChunks + SegBData, BitStart, 8, False)) WrError(90);
        sprintf(ListLine, "=%02XH-%02XH", BitStart, BitStart + 7);
      }
      else
        sprintf(ListLine, "=%02XH", AdrByte);
      PopLocHandle();
    }
  }
}

static void DecodeBIT(Word Index)
{
  LongInt AdrLong;
  UNUSED(Index);

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU >= CPU80251)
  {
    if (DecodeBitAdr(ArgStr[1], &AdrLong, False) == ModBit251)
    {
      char ByteStr[20], BitStr[10];

      PushLocHandle(-1);
      EnterIntSymbol(LabPart, AdrLong, SegNone, False);
      PopLocHandle();
      HexString(ByteStr, sizeof(ByteStr), AdrLong & 0xff, 2);
      HexString(BitStr, sizeof(BitStr), AdrLong >> 24, 1);
      sprintf(ListLine, "=%sH.%s", ByteStr, BitStr);
    }
  }
  else
  {
    if (DecodeBitAdr(ArgStr[1], &AdrLong, False) == ModBit51)
    {
      char ByteStr[20];

      PushLocHandle(-1);
      EnterIntSymbol(LabPart, AdrLong, SegBData, False);
      PopLocHandle();
      HexString(ByteStr, sizeof(ByteStr), AdrLong, 2);
      sprintf(ListLine, "=%s", ByteStr);
    }
  }
}

static void DecodePORT(Word Index)
{
  UNUSED(Index);

  if (MomCPU < CPU80251) WrError(1500);
  else
    CodeEquate(SegIO, 0, 0x1ff);
}

static void DecodeREG(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
    AddRegDef(LabPart, ArgStr[1]);
}

/*-------------------------------------------------------------------------*/
/* dynamische Codetabellenverwaltung */

static void AddFixed(char *NName, Word NCode, CPUVar NCPU)
{
  if (InstrZ >= FixedOrderCnt) exit(255);
  FixedOrders[InstrZ].Code = NCode; 
  FixedOrders[InstrZ].MinCPU = NCPU;
  AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddAcc(char *NName, Word NCode, CPUVar NCPU)
{
  if (InstrZ >= AccOrderCnt) exit(255);
  AccOrders[InstrZ].Code = NCode; 
  AccOrders[InstrZ].MinCPU = NCPU;
  AddInstTable(InstTable, NName, InstrZ++, DecodeAcc);
}

static void AddCond(char *NName, Word NCode, CPUVar NCPU)
{
  if (InstrZ >= CondOrderCnt) exit(255);
  CondOrders[InstrZ].Code = NCode;
  CondOrders[InstrZ].MinCPU = NCPU;
  AddInstTable(InstTable, NName, InstrZ++, DecodeCond);
}

static void AddBCond(char *NName, Word NCode, CPUVar NCPU)
{
  if (InstrZ >= BCondOrderCnt) exit(255);
  BCondOrders[InstrZ].Code = NCode;
  BCondOrders[InstrZ].MinCPU = NCPU;
  AddInstTable(InstTable, NName, InstrZ++, DecodeBCond);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);
  AddInstTable(InstTable, "MOV"  , 0, DecodeMOV);
  AddInstTable(InstTable, "ANL"  , 1, DecodeLogic);
  AddInstTable(InstTable, "ORL"  , 0, DecodeLogic);
  AddInstTable(InstTable, "XRL"  , 2, DecodeLogic);
  AddInstTable(InstTable, "MOVC" , 0, DecodeMOVC);
  AddInstTable(InstTable, "MOVH" , 0, DecodeMOVH);
  AddInstTable(InstTable, "MOVZ" , 0, DecodeMOVZS);
  AddInstTable(InstTable, "MOVS" , 0, DecodeMOVZS); 
  AddInstTable(InstTable, "MOVX" , 0, DecodeMOVX);
  AddInstTable(InstTable, "POP"  , 1, DecodeStack);
  AddInstTable(InstTable, "PUSH" , 0, DecodeStack);
  AddInstTable(InstTable, "PUSHW", 2, DecodeStack);
  AddInstTable(InstTable, "XCH"  , 0, DecodeXCH);
  AddInstTable(InstTable, "XCHD" , 0, DecodeXCHD);
  AddInstTable(InstTable, "AJMP" , 0, DecodeABranch);
  AddInstTable(InstTable, "ACALL", 1, DecodeABranch);
  AddInstTable(InstTable, "LJMP" , 0, DecodeLBranch);
  AddInstTable(InstTable, "LCALL", 1, DecodeLBranch);
  AddInstTable(InstTable, "EJMP" , 0, DecodeEBranch);
  AddInstTable(InstTable, "ECALL", 1, DecodeEBranch);
  AddInstTable(InstTable, "JMP"  , 0, DecodeJMP);
  AddInstTable(InstTable, "CALL" , 0, DecodeCALL);
  AddInstTable(InstTable, "DJNZ" , 0, DecodeDJNZ);
  AddInstTable(InstTable, "CJNE" , 0, DecodeCJNE);
  AddInstTable(InstTable, "ADD"  , 0, DecodeADD);
  AddInstTable(InstTable, "SUB"  , 0, DecodeSUBCMP);
  AddInstTable(InstTable, "CMP"  , 1, DecodeSUBCMP);
  AddInstTable(InstTable, "ADDC" , 0, DecodeADDCSUBB);
  AddInstTable(InstTable, "SUBB" , 1, DecodeADDCSUBB);
  AddInstTable(InstTable, "INC"  , 0, DecodeINCDEC);
  AddInstTable(InstTable, "DEC"  , 1, DecodeINCDEC);
  AddInstTable(InstTable, "MUL"  , 1, DecodeMULDIV);
  AddInstTable(InstTable, "DIV"  , 0, DecodeMULDIV);
  AddInstTable(InstTable, "CLR"  , 1, DecodeBits);
  AddInstTable(InstTable, "CPL"  , 0, DecodeBits);
  AddInstTable(InstTable, "SETB" , 2, DecodeBits);
  AddInstTable(InstTable, "SRA"  , 0, DecodeShift);
  AddInstTable(InstTable, "SRL"  , 1, DecodeShift);
  AddInstTable(InstTable, "SLL"  , 3, DecodeShift);
  AddInstTable(InstTable, "SFR"  , 0, DecodeSFR);
  AddInstTable(InstTable, "SFRB" , 1, DecodeSFR);
  AddInstTable(InstTable, "BIT"  , 0, DecodeBIT);
  AddInstTable(InstTable, "PORT" , 0, DecodePORT);

  FixedOrders = (FixedOrder *) malloc(FixedOrderCnt*sizeof(FixedOrder)); 
  InstrZ = 0;
  AddFixed("NOP" , 0x0000, CPU87C750);
  AddFixed("RET" , 0x0022, CPU87C750);
  AddFixed("RETI", 0x0032, CPU87C750);
  AddFixed("ERET", 0x01aa, CPU80251);
  AddFixed("TRAP", 0x01b9, CPU80251);

  AccOrders = (FixedOrder *) malloc(AccOrderCnt*sizeof(FixedOrder)); 
  InstrZ = 0;
  AddAcc("DA"  , 0x00d4, CPU87C750);
  AddAcc("RL"  , 0x0023, CPU87C750);
  AddAcc("RLC" , 0x0033, CPU87C750);
  AddAcc("RR"  , 0x0003, CPU87C750);
  AddAcc("RRC" , 0x0013, CPU87C750);
  AddAcc("SWAP", 0x00c4, CPU87C750);

  CondOrders = (FixedOrder *) malloc(CondOrderCnt*sizeof(FixedOrder));
  InstrZ = 0;
  AddCond("JC"  , 0x0040, CPU87C750);
  AddCond("JE"  , 0x0168, CPU80251);
  AddCond("JG"  , 0x0138, CPU80251);
  AddCond("JLE" , 0x0128, CPU80251);
  AddCond("JNC" , 0x0050, CPU87C750);
  AddCond("JNE" , 0x0178, CPU80251);
  AddCond("JNZ" , 0x0070, CPU87C750);
  AddCond("JSG" , 0x0118, CPU80251);
  AddCond("JSGE", 0x0158, CPU80251);
  AddCond("JSL" , 0x0148, CPU80251);
  AddCond("JSLE", 0x0108, CPU80251);
  AddCond("JZ"  , 0x0060, CPU87C750);
  AddCond("SJMP", 0x0080, CPU87C750);

  BCondOrders = (FixedOrder *) malloc(BCondOrderCnt*sizeof(FixedOrder));
  InstrZ = 0;
  AddBCond("JB" , 0x0020, CPU87C750);
  AddBCond("JBC", 0x0010, CPU87C750);
  AddBCond("JNB", 0x0030, CPU87C750);

  AddInstTable(InstTable, "REG"  , 0, DecodeREG);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(AccOrders);
  free(CondOrders);
  free(BCondOrders);
}

/*-------------------------------------------------------------------------*/
/* Instruktionsdecoder */

static void MakeCode_51(void)
{
  CodeLen = 0;
  DontPrint = False;
  OpSize = -1;
  MinOneIs0 = False;

  /* zu ignorierendes */

  if (*OpPart == '\0') return;

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(BigEndian))
    return;

  /* suchen */

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200, OpPart);
}

static Boolean IsDef_51(void)
{
  switch (*OpPart)
  {
    case 'B':
      return Memo("BIT");
    case 'S':
      if (Memo("SFR")) return True;
      if (MomCPU >= CPU80251) return False;
      return Memo("SFRB");
    case 'P':
      return (MomCPU >= CPU80251) ? Memo("PORT") : False;
    case 'R':
      return Memo("REG");
    default:
      return False;
  }
}

static void InitPass_51(void)
{
  SetFlag(&SrcMode, SrcModeName, False);
  SetFlag(&BigEndian, BigEndianName, False);
}

static void SwitchFrom_51(void)
{
  DeinitFields();
  ClearONOFF();
}

static void SwitchTo_51(void)
{
  TurnWords = False;
  ConstMode = ConstModeIntel;
  SetIsOccupied = False;

  PCSymbol = "$";
  HeaderID = 0x31;
  NOPCode = 0x00;
  DivideChars = ",";
  HasAttrs = False;

  /* C251 is entirely different... */

  if (MomCPU >= CPU80251)
  {
    ValidSegs = (1 << SegCode) | (1 << SegIO);
    Grans[SegCode ] = 1; ListGrans[SegCode ] = 1; SegInits[SegCode ] = 0;
    SegLimits[SegCode ] = 0xffffffl;
    Grans[SegIO   ] = 1; ListGrans[SegIO   ] = 1; SegInits[SegIO   ] = 0;
    SegLimits[SegIO   ] = 0x1ff;
  }
 
  /* rest of the pack... */

  else
  {
    ValidSegs=(1 << SegCode) | (1 << SegData) | (1 << SegIData) | (1 << SegXData) | (1 << SegBData);

    Grans[SegCode ] = 1; ListGrans[SegCode ] = 1; SegInits[SegCode ] = 0;
    if (MomCPU == CPU80C390)
      SegLimits[SegCode ] = 0xffffff;
    else if (MomCPU == CPU87C750)
      SegLimits[SegCode ] = 0x7ff;
    else
      SegLimits[SegCode ] = 0xffff;


    Grans[SegXData] = 1; ListGrans[SegXData] = 1; SegInits[SegXData] = 0;
    if (MomCPU == CPU80C390)
      SegLimits[SegXData] = 0xffffff;
    else
      SegLimits[SegXData] = 0xffff;

    Grans[SegData ] = 1; ListGrans[SegData ] = 1; SegInits[SegData ] = 0x30;
    SegLimits[SegData ] = 0xff;
    Grans[SegIData] = 1; ListGrans[SegIData] = 1; SegInits[SegIData] = 0x80;
    SegLimits[SegIData] = 0xff;
    Grans[SegBData] = 1; ListGrans[SegBData] = 1; SegInits[SegBData] = 0;
    SegLimits[SegBData] = 0xff;
  }

  MakeCode = MakeCode_51;
  IsDef = IsDef_51;

  InitFields();
  SwitchFrom = SwitchFrom_51;
  AddONOFF("SRCMODE"  , &SrcMode  , SrcModeName  , False);
  AddONOFF("BIGENDIAN", &BigEndian, BigEndianName, False);
}


void code51_init(void)
{
  CPU87C750 = AddCPU("87C750", SwitchTo_51);
  CPU8051   = AddCPU("8051"  , SwitchTo_51);
  CPU8052   = AddCPU("8052"  , SwitchTo_51);
  CPU80C320 = AddCPU("80C320", SwitchTo_51);
  CPU80501  = AddCPU("80C501", SwitchTo_51);
  CPU80502  = AddCPU("80C502", SwitchTo_51);
  CPU80504  = AddCPU("80C504", SwitchTo_51);
  CPU80515  = AddCPU("80515" , SwitchTo_51);
  CPU80517  = AddCPU("80517" , SwitchTo_51);
  CPU80C390 = AddCPU("80C390", SwitchTo_51);
  CPU80251  = AddCPU("80C251", SwitchTo_51);
  CPU80251T = AddCPU("80C251T", SwitchTo_51);

  AddInitPassProc(InitPass_51);
}
