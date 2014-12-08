/* codeh8_3.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator H8/300(L/H)                                                 */
/*                                                                           */
/* Historie: 22.11.1996 Grundsteinlegung                                     */
/*           15.10.1998 TRAPA nachgetragen                                   */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codeh8_3.c,v 1.11 2014/12/07 19:14:01 alfred Exp $                    */
/*****************************************************************************
 * $Log: codeh8_3.c,v $
 * Revision 1.11  2014/12/07 19:14:01  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.10  2014/12/05 11:15:28  alfred
 * - eliminate AND/OR/NOT
 *
 * Revision 1.9  2014/12/05 08:53:45  alfred
 * - eliminate remaining BEGIN/END
 *
 * Revision 1.8  2014/06/16 19:23:18  alfred
 * - adapt to current style
 *
 * Revision 1.7  2011-08-01 19:56:08  alfred
 * - add attribute type to error message
 *
 * Revision 1.6  2010/08/27 14:52:42  alfred
 * - some more overlapping strcpy() cleanups
 *
 * Revision 1.5  2010/04/17 13:14:23  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.4  2007/11/24 22:48:06  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 17:31:05  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 12:04:47  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
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
#include "asmitree.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "codevars.h"

#include "codeh8_3.h"

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModImm 1
#define MModImm (1 << ModImm)
#define ModAbs8 2
#define MModAbs8 (1 << ModAbs8)
#define ModAbs16 3
#define MModAbs16 (1 << ModAbs16)
#define ModAbs24 4
#define MModAbs24 (1 << ModAbs24)
#define MModAbs (MModAbs8 | MModAbs16 | MModAbs24)
#define ModIReg 5
#define MModIReg (1 << ModIReg)
#define ModPreDec 6
#define MModPreDec (1 << ModPreDec)
#define ModPostInc 7
#define MModPostInc (1 << ModPostInc)
#define ModInd16 8
#define MModInd16 (1 << ModInd16)
#define ModInd24 9
#define MModInd24 (1 << ModInd24)
#define ModIIAbs 10
#define MModIIAbs (1 << ModIIAbs)
#define MModInd (MModInd16 | MModInd24)

static ShortInt OpSize;     /* Groesse=8*(2^OpSize) */
static ShortInt AdrMode;    /* Ergebnisadressmodus */
static Byte AdrPart;        /* Adressierungsmodusbits im Opcode */
static Word AdrVals[6];     /* Adressargument */

static CPUVar CPUH8_300L;
static CPUVar CPU6413308,CPUH8_300;
static CPUVar CPU6413309,CPUH8_300H;
static Boolean CPU16;       /* keine 32-Bit-Register */

/*-------------------------------------------------------------------------*/
/* Adressparsing */

typedef enum
{
  SizeNone, Size8, Size16, Size24
} MomSize_t;
static MomSize_t MomSize;

static void SetOpSize(ShortInt Size)
{
  if (OpSize == -1)
    OpSize = Size;
  else if (Size != OpSize)
  {
    WrError(1131);
    AdrMode = ModNone;
    AdrCnt = 0;
  }
}

static Boolean IsNum(char Inp, Byte *Erg)
{
  if ((Inp < '0') || (Inp > '7'))
    return False;
  else
  {
    *Erg = Inp - AscOfs;
    return True;
  }
}

static Boolean DecodeReg(char *Asc, Byte *Erg, ShortInt *Size)
{
  int l = strlen(Asc);

  if (!strcasecmp(Asc, "SP"))
  {
    *Erg = 7;
    *Size = (Maximum) ? 2 : 1;
    return True;
  }

  else if ((l == 3) && (mytoupper(*Asc) == 'R') && (IsNum(Asc[1], Erg)))
  {
    if (mytoupper(Asc[2]) == 'L')
    {
      *Erg += 8;
      *Size = 0;
      return True;
    }
    else if (mytoupper(Asc[2]) == 'H')
    {
      *Size = 0;
      return True;
    }
    else
      return False;
  }

  else if ((l == 2) && (IsNum(Asc[1], Erg)))
  {
    if (mytoupper(*Asc) == 'R')
    {
      *Size = 1;
      return True;
    }
    else if (mytoupper(*Asc) == 'E')
    {
      *Erg += 8;
      *Size = 1;
      return True;
    }
    else
      return False;
  }

  else if ((l == 3) && (mytoupper(*Asc) == 'E') && (mytoupper(Asc[1]) == 'R') & (IsNum(Asc[2], Erg)))
  {
    *Size = 2;
    return True;
  }
  else
    return False;
}

static void CutSize(char *Asc)
{
  int l = strlen(Asc);

  if ((l >= 2) && (Asc[l - 2] == ':') && (Asc[l - 1] == '8'))
  {
    Asc[l - 2] = '\0';
    MomSize = Size8;
  }
  else if ((l >= 3) && (Asc[l - 3] == ':'))
  {
    if ((Asc[l - 2] == '1') && (Asc[l - 1] == '6'))
    {
      Asc[l - 3] = '\0';
      MomSize = Size16;
    }
    else if ((Asc[l - 2] == '2') && (Asc[l - 1] == '4'))
    {
      Asc[l - 3] = '\0';
      MomSize = Size24;
    }
  }
}

static Byte DecodeBaseReg(char *Asc, Byte *Erg)
{
  ShortInt HSize;

  if (!DecodeReg(Asc, Erg, &HSize))
    return 0;
  if ((HSize == 0) || ((HSize == 1) && (*Erg > 7)))
  {
    WrError(1350);
    return 1;
  }
  if ((CPU16) != (HSize == 1))
  {
    WrError(1505);
    return 1;
  }
  return 2;
}

static Boolean Is8(LongInt Address)
{
  if (CPU16)
    return (((Address >> 8) & 0xff) == 0xff);
  else
    return (((Address >> 8) & 0xffff) == 0xffff);
}

static Boolean Is16(LongInt Address)
{
  return (CPU16) ? (True) : (((Address >= 0) && (Address <= 0x7fff)) || ((Address >= 0xff8000) && (Address <= 0xffffff)));
}

static void DecideVAbsolute(LongInt Address, Word Mask)
{
  /* bei Automatik Operandengroesse festlegen */

  if (MomSize == SizeNone)
  {
    if (Is8(Address))
      MomSize = Size8;
    else if (Is16(Address))
      MomSize = Size16;
    else
      MomSize = Size24;
  }

  /* wenn nicht vorhanden, eins rauf */

  if ((MomSize == Size8) && ((Mask & MModAbs8) == 0))
    MomSize = Size16;
  if ((MomSize == Size16) && ((Mask & MModAbs16) == 0))
    MomSize = Size24;

  /* entsprechend Modus Bytes ablegen */

  switch (MomSize)
  {
    case Size8:
      if (!Is8(Address)) WrError(1925);
      else
      {
        AdrCnt = 2;
        AdrVals[0] = Address & 0xff;
        AdrMode = ModAbs8;
      }
      break;
    case Size16:
      if (!Is16(Address)) WrError(1925);
      else
      {
        AdrCnt = 2;
        AdrVals[0] = Address & 0xffff;
        AdrMode = ModAbs16;
      }
      break;
    case Size24:
      AdrCnt = 4;
      AdrVals[1] = Address & 0xffff;
      AdrVals[0] = Lo(Address >> 16);
      AdrMode = ModAbs24;
      break;
    default:
      WrError(10000);
  }
}

static void DecideAbsolute(char *Asc, Word Mask)
{
  LongInt Addr;
  Boolean OK;

  Addr = EvalIntExpression(Asc, Int32, &OK);
  if (OK)
    DecideVAbsolute(Addr, Mask);
}


static void DecodeAdr(char *Asc, Word Mask)
{
  ShortInt HSize;
  Byte HReg;
  LongInt HLong;
  Boolean OK;
  char *p;
  LongInt DispAcc;
  int l;

  AdrMode = ModNone;
  AdrCnt = 0;
  MomSize = SizeNone;

  /* immediate ? */

  if (*Asc == '#')
  {
    switch (OpSize)
    {
      case -1:
        WrError(1132);
        break;
      case 0:
        HReg = EvalIntExpression(Asc + 1, Int8, &OK);
        if (OK)
        {
          AdrCnt = 2;
          AdrVals[0] = HReg;
          AdrMode = ModImm;
        }
        break;
      case 1:
        AdrVals[0] = EvalIntExpression(Asc + 1, Int16, &OK);
        if (OK)
        {
          AdrCnt = 2;
          AdrMode = ModImm;
        }
        break;
      case 2:
        HLong = EvalIntExpression(Asc + 1, Int32, &OK);
        if (OK)
        {
          AdrCnt = 4;
          AdrVals[0] = HLong >> 16;
          AdrVals[1] = HLong & 0xffff;
          AdrMode = ModImm;
        }
        break;
      default:
        WrError(1130);
    }
    goto chk;
  }

  /* Register ? */

  if (DecodeReg(Asc, &HReg, &HSize))
  {
    AdrMode = ModReg;
    AdrPart = HReg;
    SetOpSize(HSize);
    goto chk;
  }

  /* indirekt ? */

  if (*Asc == '@')
  {
    Asc++;

    if (*Asc == '@')
    {
      AdrVals[0] = EvalIntExpression(Asc + 1, UInt8, &OK) & 0xff;
      if (OK)
      {
        AdrCnt = 1;
        AdrMode = ModIIAbs;
      }
      goto chk;
    }

    switch (DecodeBaseReg(Asc, &AdrPart))
    {
      case 1:
        goto chk;
      case 2:
        AdrMode = ModIReg;
        goto chk;
    }

    if (*Asc == '-')
    {
      switch (DecodeBaseReg(Asc + 1, &AdrPart))
      {
        case 1:
          goto chk;
        case 2:
          AdrMode = ModPreDec;
          goto chk;
      }
    }

    if ((*Asc) && (Asc[l = strlen(Asc) - 1] == '+'))
    {
      Asc[l] = '\0';
      switch (DecodeBaseReg(Asc, &AdrPart))
      {
        case 1:
          goto chk;
        case 2:
         AdrMode = ModPostInc;
         goto chk;
      }
      Asc[l] = '+';
    }

    if (IsIndirect(Asc))
    {
      char *Part = Asc + 1;

      Part[strlen(Part) - 1] = '\0';
      AdrPart = 0xff;
      DispAcc = 0;
      do
      {
        p = QuotPos(Part, ',');
        if (p)
          *p = '\0';
        switch (DecodeBaseReg(Part, &HReg))
        {
          case 2:
            if (AdrPart != 0xff)
            {
              WrError(1350);
              goto chk;
            }
            else
              AdrPart = HReg;
            break;
          case 1:
            goto chk;
          case 0:
            CutSize(Part);
            DispAcc += EvalIntExpression(Part, Int32, &OK);
            if (!OK)
            {
              goto chk;
            }
            break;
        }
        if (p)
          Part = p + 1;
      }
      while (p);

      if (AdrPart == 0xff)
        DecideVAbsolute(DispAcc,Mask);
      else
      {
        if ((CPU16) && ((DispAcc & 0xffff8000) == 0x8000))
          DispAcc += 0xffff0000;
        if (MomSize == SizeNone)
          MomSize = ((DispAcc >= -32768) && (DispAcc <= 32767)) ? Size16 : Size24;
        switch (MomSize)
        {
          case Size8:
            WrError(1130);
            break;
          case Size16:
            if (ChkRange(DispAcc, -32768, 32767))
            {
              AdrCnt = 2;
              AdrVals[0] = DispAcc & 0xffff;
              AdrMode = ModInd16;
            }
            break;
          case Size24:
            AdrVals[1] = DispAcc & 0xffff;
            AdrVals[0] = Lo(DispAcc >> 16);
            AdrCnt = 4;
            AdrMode = ModInd24;
            break;
          default:
            WrError(10000);
        }
      }
    }
    else
    {
      CutSize(Asc);
      DecideAbsolute(Asc, Mask);
    }
    goto chk;
  }

  CutSize(Asc);
  DecideAbsolute(Asc,Mask);

chk:
  if (CPU16)
  {
    if (((AdrMode == ModReg) && (OpSize == 2))
     || ((AdrMode == ModReg) && (OpSize == 1) && (AdrPart > 7))
     || (AdrMode == ModAbs24)
     || (AdrMode == ModInd24))
    {
      WrError(1505);
      AdrMode = ModNone;
      AdrCnt = 0;
    }
  }
  if ((AdrMode != ModNone) && ((Mask & (1 << AdrMode)) == 0))
  {
    WrError(1350);
    AdrMode = ModNone;
    AdrCnt = 0;
  }
}

static LongInt ImmVal(void)
{
  switch (OpSize)
  {
    case 0:
      return Lo(AdrVals[0]);
    case 1:
      return AdrVals[0];
    case 2:
      return (((LongInt)AdrVals[0]) << 16) + AdrVals[1];
    default:
      WrError(10000);
      return 0;
  }
}

/*-------------------------------------------------------------------------*/
/* Code Generators */

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    CodeLen = 2;
    WAsmCode[0] = Code;
  }
}

static void DecodeEEPMOV(Word Code)
{
  UNUSED(Code);

  if (OpSize == -1)
    OpSize = Ord(!CPU16);
  if (OpSize > 1) WrError(1130);
  else if (ArgCnt != 0) WrError(1110);
  else if ((OpSize == 1) && (CPU16)) WrError(1500);
  else
  {
    CodeLen = 4;
    WAsmCode[0] = (OpSize == 0) ? 0x7b5c : 0x7bd4;
    WAsmCode[1] = 0x598f;
  }
}

static void DecodeMOV(Word Code)
{
  Byte HReg;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[2], MModReg | MModIReg | MModPreDec | MModInd | MModAbs);
    switch (AdrMode)
    {
      case ModReg:
        HReg = AdrPart;
        DecodeAdr(ArgStr[1], MModReg | MModIReg | MModPostInc | MModInd | MModImm | ((OpSize == 0) ? MModAbs : (MModAbs16 | MModAbs24)));
        switch (AdrMode)
        {
          case ModReg:
          {
            int z = (OpSize == 2) ? 3 : OpSize;

            CodeLen = 2;
            WAsmCode[0] = 0x0c00 + (z << 8) + (AdrPart << 4) + HReg;
            if (OpSize == 2)
              WAsmCode[0] += 0x80;
            break;
          }
          case ModIReg:
            switch (OpSize)
            {
              case 0:
                CodeLen = 2;
                WAsmCode[0] = 0x6800 + (AdrPart << 4) + HReg;
                break;
              case 1:
                CodeLen = 2;
                WAsmCode[0] = 0x6900 + (AdrPart << 4) + HReg;
                break;
              case 2:
                CodeLen = 4;
                WAsmCode[0] = 0x0100;
                WAsmCode[1] = 0x6900 + (AdrPart << 4) + HReg;
                break;
            }
            break;
          case ModPostInc:
            switch (OpSize)
            {
              case 0:
                CodeLen = 2;
                WAsmCode[0] = 0x6c00 + (AdrPart << 4) + HReg;
                break;
              case 1:
                CodeLen = 2;
                WAsmCode[0] = 0x6d00 + (AdrPart << 4) + HReg;
                break;
              case 2:
                CodeLen = 4;
                WAsmCode[0] = 0x0100;
                WAsmCode[1] = 0x6d00 + (AdrPart << 4) + HReg;
                break;
            }
            break;
          case ModInd16:
            switch (OpSize)
            {
              case 0:
                CodeLen = 4;
                WAsmCode[0] = 0x6e00 + (AdrPart << 4) + HReg;
                WAsmCode[1] = AdrVals[0];
                break;
              case 1:
                CodeLen = 4;
                WAsmCode[0] = 0x6f00 + (AdrPart << 4) + HReg;
                WAsmCode[1] = AdrVals[0];
                break;
              case 2:
                CodeLen = 6;
                WAsmCode[0] = 0x0100;
                WAsmCode[1] = 0x6f00 + (AdrPart << 4) + HReg;
                WAsmCode[2] = AdrVals[0];
                break;
            }
            break;
          case ModInd24:
            switch (OpSize)
            {
              case 0:
                CodeLen = 8;
                WAsmCode[0] = 0x7800 + (AdrPart << 4);
                WAsmCode[1] = 0x6a20 + HReg;
                memcpy(WAsmCode + 2, AdrVals, AdrCnt);
                break;
              case 1:
                CodeLen = 8;
                WAsmCode[0] = 0x7800 + (AdrPart << 4);
                WAsmCode[1] = 0x6b20 + HReg;
                memcpy(WAsmCode + 2, AdrVals, AdrCnt);
                break;
              case 2:
                CodeLen = 10;
                WAsmCode[0] = 0x0100;
                WAsmCode[1] = 0x7800 + (AdrPart << 4);
                WAsmCode[2] = 0x6b20 + HReg;
                memcpy(WAsmCode + 3, AdrVals, AdrCnt);
                break;
            }
            break;
          case ModAbs8:
            CodeLen = 2;
            WAsmCode[0] = 0x2000 + (((Word)HReg) << 8) + Lo(AdrVals[0]);
            break;
          case ModAbs16:
            switch (OpSize)
            {
              case 0:
                CodeLen = 4;
                WAsmCode[0] = 0x6a00 + HReg;
                WAsmCode[1] = AdrVals[0];
                break;
              case 1:
                CodeLen = 4;
                WAsmCode[0] = 0x6b00 + HReg;
                WAsmCode[1] = AdrVals[0];
                break;
              case 2:
                CodeLen = 6;
                WAsmCode[0] = 0x0100;
                WAsmCode[1] = 0x6b00 + HReg;
                WAsmCode[2] = AdrVals[0];
                break;
            }
            break;
          case ModAbs24:
            switch (OpSize)
            {
              case 0:
                CodeLen = 6;
                WAsmCode[0] = 0x6a20 + HReg;
                memcpy(WAsmCode + 1, AdrVals, AdrCnt);
                break;
              case 1:
                CodeLen = 6;
                WAsmCode[0] = 0x6b20 + HReg;
                memcpy(WAsmCode + 1, AdrVals, AdrCnt);
                break;
              case 2:
                CodeLen = 8;
                WAsmCode[0] = 0x0100;
                WAsmCode[1] = 0x6b20 + HReg;
                memcpy(WAsmCode + 2, AdrVals, AdrCnt);
                break;
            }
            break;
          case ModImm:
            switch (OpSize)
            {
              case 0:
                CodeLen = 2;
                WAsmCode[0] = 0xf000 + (((Word)HReg) << 8) + Lo(AdrVals[0]);
                break;
              case 1:
                CodeLen = 4;
                WAsmCode[0] = 0x7900 + HReg;
                WAsmCode[1] = AdrVals[0];
                break;
              case 2:
                CodeLen = 6;
                WAsmCode[0] = 0x7a00 + HReg;
                memcpy(WAsmCode + 1, AdrVals, AdrCnt);
                break;
            }
            break;
        }
        break;
      case ModIReg:
        HReg = AdrPart;
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrMode != ModNone)
        {
          switch (OpSize)
          {
            case 0:
              CodeLen = 2;
              WAsmCode[0] = 0x6880 + (HReg << 4) + AdrPart;
              break;
            case 1:
              CodeLen = 2;
              WAsmCode[0] = 0x6980 + (HReg << 4) + AdrPart;
              break;
            case 2:
              CodeLen = 4;
              WAsmCode[0] = 0x0100;
              WAsmCode[1] = 0x6980 + (HReg << 4) + AdrPart;
              break;
          }
        }
        break;
      case ModPreDec:
        HReg = AdrPart;
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrMode != ModNone)
        {
          switch (OpSize)
          {
            case 0:
             CodeLen=2; WAsmCode[0]=0x6c80+(HReg << 4)+AdrPart;
             break;
            case 1:
             CodeLen=2; WAsmCode[0]=0x6d80+(HReg << 4)+AdrPart;
             break;
            case 2:
             CodeLen=4; WAsmCode[0]=0x0100;
             WAsmCode[1]=0x6d80+(HReg << 4)+AdrPart;
             break;
          }
        }
        break;
      case ModInd16:
        HReg = AdrPart;
        WAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrMode!=ModNone)
        {
          switch (OpSize)
          {
            case 0:
              CodeLen = 4;
              WAsmCode[0] = 0x6e80 + (HReg << 4) + AdrPart;
              break;
            case 1:
              CodeLen = 4;
              WAsmCode[0] = 0x6f80 + (HReg << 4) + AdrPart;
              break;
            case 2:
              CodeLen = 6;
              WAsmCode[0] = 0x0100;
              WAsmCode[2] = WAsmCode[1];
              WAsmCode[1] = 0x6f80 + (HReg << 4) + AdrPart;
              break;
          }
        }
        break;
      case ModInd24:
        HReg = AdrPart;
        memcpy(WAsmCode + 2, AdrVals, 4);
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrMode != ModNone)
        {
          switch (OpSize)
          {
            case 0:
              CodeLen = 8;
              WAsmCode[0] = 0x7800 + (HReg << 4);
              WAsmCode[1] = 0x6aa0 + AdrPart;
              break;
            case 1:
              CodeLen = 8;
              WAsmCode[0] = 0x7800 + (HReg << 4);
              WAsmCode[1] = 0x6ba0 + AdrPart;
              break;
            case 2:
              CodeLen = 10;
              WAsmCode[0] = 0x0100;
              WAsmCode[4] = WAsmCode[3];
              WAsmCode[3] = WAsmCode[2];
              WAsmCode[1] = 0x7800 + (HReg << 4);
              WAsmCode[2] = 0x6ba0 + AdrPart;
              break;
          }
        }
        break;
      case ModAbs8:
        HReg = Lo(AdrVals[0]);
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrMode != ModNone)
        {
          switch (OpSize)
          {
            case 0:
              CodeLen = 2;
              WAsmCode[0] = 0x3000 + (((Word)AdrPart) << 8) + HReg;
              break;
            case 1:
              CodeLen = 4;
              WAsmCode[0] = 0x6b80 + AdrPart;
              WAsmCode[1] = 0xff00 + HReg;
              break;
            case 2:
              CodeLen = 6;
              WAsmCode[0] = 0x0100;
              WAsmCode[1] = 0x6b80 + AdrPart;
              WAsmCode[2] = 0xff00 + HReg;
              break;
          }
        }
        break;
      case ModAbs16:
        WAsmCode[1] = AdrVals[0];
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrMode != ModNone)
        {
          switch (OpSize)
          {
            case 0:
              CodeLen = 4;
              WAsmCode[0] = 0x6a80 + AdrPart;
              break;
            case 1:
              CodeLen = 4;
              WAsmCode[0] = 0x6b80 + AdrPart;
              break;
            case 2:
              CodeLen = 6;
              WAsmCode[0] = 0x0100;
              WAsmCode[2] = WAsmCode[1];
              WAsmCode[1] = 0x6b80 + AdrPart;
              break;
          }
        }
        break;
      case ModAbs24:
        memcpy(WAsmCode + 1, AdrVals, 4);
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrMode != ModNone)
        {
          switch (OpSize)
          {
            case 0:
              CodeLen = 6;
              WAsmCode[0] = 0x6aa0 + AdrPart;
              break;
            case 1:
              CodeLen = 6;
              WAsmCode[0] = 0x6ba0 + AdrPart;
              break;
            case 2:
              CodeLen = 8;
              WAsmCode[0] = 0x0100;
              WAsmCode[3] = WAsmCode[2];
              WAsmCode[2] = WAsmCode[1];
              WAsmCode[1] = 0x6ba0 + AdrPart;
              break;
          }
        }
        break;
    }
  }
}

static void DecodeMOVTPE_MOVFPE(Word CodeTPE)
{
  if (ArgCnt != 2) WrError(1110);
  else if (MomCPU <= CPUH8_300L) WrError(1500);
  else
  {
    char *pRegArg = CodeTPE ? ArgStr[1] : ArgStr[2],
         *pMemArg = CodeTPE ? ArgStr[2] : ArgStr[1];

    DecodeAdr(pRegArg, MModReg);
    if (AdrMode != ModNone)
    {
      if (OpSize != 0) WrError(1130);
      else
      {
        Byte HReg = AdrPart;
        DecodeAdr(pMemArg, MModAbs16);
        if (AdrMode != ModNone)
        {
          CodeLen = 4;
          WAsmCode[0] = 0x6a40 + CodeTPE + HReg;
          WAsmCode[1] = AdrVals[0];
        }
      }
    }
  }
}

static void DecodePUSH_POP(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode != ModNone)
    {
      if (OpSize == 0) WrError(1130);
      else if ((CPU16) && (OpSize == 2)) WrError(1500);
      else
      {
        if (OpSize == 2)
          WAsmCode[0]=0x0100;
        CodeLen = 2 * OpSize;
        WAsmCode[(CodeLen - 2) >> 1] = Code + AdrPart;
      }
    }
  }
}

static void DecodeLDC_STC(Word CodeIsSTC)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    char *pRegArg = CodeIsSTC ? ArgStr[1] : ArgStr[2],
         *pMemArg = CodeIsSTC ? ArgStr[2] : ArgStr[1];

    if (strcasecmp(pRegArg, "CCR")) WrError(1350);
    else
    {
       SetOpSize(0);
       DecodeAdr(pMemArg, MModReg | MModIReg | MModInd | MModAbs16 | MModAbs24 | (CodeIsSTC ? MModPreDec : (MModImm | MModPostInc)));
       switch (AdrMode)
       {
         case ModReg:
           CodeLen = 2;
           WAsmCode[0] = 0x0300 + AdrPart - (CodeIsSTC << 1);
           break;
         case ModIReg:
           CodeLen = 4;
           WAsmCode[0] = 0x0140;
           WAsmCode[1] = 0x6900 + CodeIsSTC + (AdrPart << 4);
           break;
         case ModPostInc:
         case ModPreDec:
           CodeLen = 4;
           WAsmCode[0] = 0x0140;
           WAsmCode[1] = 0x6d00 + CodeIsSTC + (AdrPart << 4);
           break;
         case ModInd16:
           CodeLen = 6;
           WAsmCode[0] = 0x0140;
           WAsmCode[2] = AdrVals[0];
           WAsmCode[1] = 0x6f00 + CodeIsSTC + (AdrPart << 4);
           break;
         case ModInd24:
           CodeLen = 10;
           WAsmCode[0] = 0x0140;
           WAsmCode[1] = 0x7800 + (AdrPart << 4);
           WAsmCode[2] = 0x6b20 + CodeIsSTC;
           memcpy(WAsmCode + 3, AdrVals, AdrCnt);
           break;
         case ModAbs16:
           CodeLen = 6;
           WAsmCode[0] = 0x0140;
           WAsmCode[2] = AdrVals[0];
           WAsmCode[1] = 0x6b00 + CodeIsSTC;
           break;
         case ModAbs24:
           CodeLen = 8;
           WAsmCode[0] = 0x0140; 
           WAsmCode[1] = 0x6b20 + CodeIsSTC;
           memcpy(WAsmCode + 2, AdrVals, AdrCnt);
           break;
         case ModImm:
           CodeLen = 2;
           WAsmCode[0] = 0x0700 + Lo(AdrVals[0]);
           break;
       }
    }
  }
}

static void DecodeADD_SUB(Word IsSUB)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[2], MModReg);
    if (AdrMode != ModNone)
    {
      Byte HReg = AdrPart;
      DecodeAdr(ArgStr[1], MModReg | MModImm);
      if (AdrMode != ModNone)
      {
        if ((CPU16) && ((OpSize > 1) || ((OpSize == 1) && (AdrMode == ModImm)))) WrError(1500);
        else
        {
          switch (AdrMode)
          {
            case ModImm:
              switch (OpSize)
              {
                case 0:
                  if (IsSUB) WrError(1350);
                  else
                  {
                    CodeLen = 2;
                    WAsmCode[0] = 0x8000 + (((Word)HReg) << 8) + Lo(AdrVals[0]);
                  }
                  break;
                case 1:
                  CodeLen = 4;
                  WAsmCode[1] = AdrVals[0];
                  WAsmCode[0] = 0x7910 + (IsSUB << 5) + HReg;
                  break;
                case 2:
                  CodeLen = 6;
                  memcpy(WAsmCode + 1, AdrVals, 4);
                  WAsmCode[0] = 0x7a10 + (IsSUB << 5) + HReg;
                  break;
              }
              break;
            case ModReg:
              switch (OpSize)
              {
                case 0:
                  CodeLen = 2;
                  WAsmCode[0] = 0x0800 + (IsSUB << 12) + (AdrPart << 4) + HReg;
                  break;
                case 1:
                  CodeLen = 2;
                  WAsmCode[0] = 0x0900 + (IsSUB << 12) + (AdrPart << 4) + HReg;
                  break;
                case 2:
                  CodeLen = 2;
                  WAsmCode[0] = 0x0a00 + (IsSUB << 12) + 0x80 + (AdrPart << 4) + HReg;
                  break;
              }
              break;
          }
        }
      }
    }
  }
}

static void DecodeCMP(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[2], MModReg);
    if (AdrMode != ModNone)
    {
      Byte HReg = AdrPart;
      DecodeAdr(ArgStr[1], MModReg | MModImm);
      if (AdrMode != ModNone)
      {
        if ((CPU16) && ((OpSize > 1) || ((OpSize == 1) && (AdrMode == ModImm)))) WrError(1500);
        else
        {
          switch (AdrMode)
          {
            case ModImm:
              switch (OpSize)
              {
                case 0:
                  CodeLen = 2;
                  WAsmCode[0] = 0xa000 + (((Word)HReg) << 8) + Lo(AdrVals[0]);
                  break;
                case 1:
                  CodeLen = 4;
                  WAsmCode[1] = AdrVals[0];
                  WAsmCode[0] = 0x7920 + HReg;
                  break;
                case 2:
                  CodeLen = 6;
                  memcpy(WAsmCode + 1, AdrVals, 4);
                  WAsmCode[0] = 0x7a20 + HReg;
              }
              break;
            case ModReg:
              switch (OpSize)
              {
                case 0:
                  CodeLen = 2;
                  WAsmCode[0] = 0x1c00 + (AdrPart << 4) + HReg;
                  break;
                case 1:
                  CodeLen = 2;
                  WAsmCode[0] = 0x1d00 + (AdrPart << 4) + HReg;
                  break;
                case 2:
                  CodeLen = 2;
                  WAsmCode[0] = 0x1f80 + (AdrPart << 4) + HReg;
                  break;
              }
              break;
          }
        }
      }
    }
  }
}

static void DecodeLogic(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[2], MModReg);
    if (AdrMode != ModNone)
    {
      if ((CPU16) && (OpSize > 0)) WrError(1500);
      else
      {
        Byte HReg = AdrPart;
        DecodeAdr(ArgStr[1], MModImm | MModReg);
        switch (AdrMode)
        {
          case ModImm:
            switch (OpSize)
            {
              case 0:
                CodeLen = 2;
                WAsmCode[0] = 0xc000 + (Code << 12) + (((Word)HReg) << 8) + Lo(AdrVals[0]);
                break;
              case 1:
                CodeLen = 4;
                WAsmCode[1] = AdrVals[0];
                WAsmCode[0] = 0x7940 + (Code << 4) + HReg;
                break;
              case 2:
                CodeLen = 6;
                memcpy(WAsmCode + 1, AdrVals, AdrCnt);
                WAsmCode[0] = 0x7a40 + (Code << 4) + HReg;
                break;
            }
            break;
          case ModReg:
            switch (OpSize)
            {
              case 0:
                CodeLen = 2;
                WAsmCode[0] = 0x1400 + (Code << 8) + (AdrPart << 4) + HReg;
                break;
              case 1:
                CodeLen = 2;
                WAsmCode[0] = 0x6400 + (Code << 8) + (AdrPart << 4) + HReg;
                break;
              case 2:
                CodeLen = 4;
                WAsmCode[0] = 0x01f0;
                WAsmCode[1] = 0x6400 + (Code << 8) + (AdrPart << 4) + HReg;
                break;
            }
            break;
        }
      }
    }
  }
}

static void DecodeLogicBit(Word Code)
{
  SetOpSize(0);
  if (ArgCnt != 2) WrError(1110);
  else if (strcasecmp(ArgStr[2], "CCR")) WrError(1350);
  else
  {
    DecodeAdr(ArgStr[1], MModImm);
    if (AdrMode != ModNone)
    {
      CodeLen = 2;
      WAsmCode[0] = 0x0400 + (Code << 8) + Lo(AdrVals[0]);
    }
  }
}

static void DecodeADDX_SUBX(Word IsSUBX)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[2], MModReg);
    if (AdrMode != ModNone)
    {
      if (OpSize != 0) WrError(1130);
      else
      {
        Byte HReg = AdrPart;
        DecodeAdr(ArgStr[1], MModImm | MModReg);
        switch (AdrMode)
        {
          case ModImm:
            CodeLen = 2;
            WAsmCode[0] = 0x9000 + (((Word)HReg) << 8) + Lo(AdrVals[0]) + (IsSUBX << 13);
            break;
          case ModReg:
            CodeLen = 2;
            WAsmCode[0] = 0x0e00 + (AdrPart << 4) + HReg + (IsSUBX << 12);
            break;
        }
      }
    }
  }
}

static void DecodeADDS_SUBS(Word IsSUBS)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[2], MModReg);
    if (AdrMode != ModNone)
    {
      if (((CPU16) && (OpSize != 1)) || ((!CPU16) && (OpSize != 2))) WrError(1130);
      else
      {
        Byte HReg = AdrPart;
        DecodeAdr(ArgStr[1], MModImm);
        if (AdrMode != ModNone)
        {
          LongInt AdrLong = ImmVal();
          if ((AdrLong != 1) && (AdrLong != 2) && (AdrLong != 4)) WrError(1320);
          else
          {
            switch (AdrLong)
            {
              case 1: WAsmCode[0] = 0x0b00; break;
              case 2: WAsmCode[0] = 0x0b80; break;
              case 4: WAsmCode[0] = 0x0b90; break;
            }
            CodeLen = 2;
            WAsmCode[0] += HReg + IsSUBS;
          }
        }
      }
    }
  }
}

static void DecodeMul(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    if (OpSize != -1) OpSize++;
    DecodeAdr(ArgStr[2], MModReg);
    if (AdrMode != ModNone)
    {
      if (OpSize == 0) WrError(1130);
      else if ((CPU16) && (OpSize == 2)) WrError(1500);
      else
      {
        Byte HReg = AdrPart;
        OpSize--;
        DecodeAdr(ArgStr[1], MModReg);
        if (AdrMode != ModNone)
        {
          if ((Code & 2) == 2)
          {
            CodeLen = 4;
            WAsmCode[0] = 0x01c0;
            if ((Code & 1) == 1)
              WAsmCode[0] += 0x10;
          }
          else
            CodeLen=2;
          WAsmCode[CodeLen >> 2] = 0x5000
                                 + (((Word)OpSize) << 9)
                                 + ((Code & 1) << 8)
                                 + (AdrPart << 4) + HReg;
        }
      }
    }
  }
}

static void DecodeBit1(Word Code)
{
  Word OpCode = 0x60 + (Code & 0x7f);

  if (ArgCnt != 2) WrError(1110);
  else
  {
    if (*ArgStr[1] != '#') WrError(1350);
    else
    {
      Boolean OK;
      Byte Bit = EvalIntExpression(ArgStr[1] + 1, UInt3, &OK);
      if (OK)
      {
        DecodeAdr(ArgStr[2], MModReg | MModIReg | MModAbs8);
        if (AdrMode != ModNone)
        {
          if (OpSize > 0) WrError(1130);
          else
          {
            switch (AdrMode)
            {
              case ModReg:
                CodeLen = 2;
                WAsmCode[0] = (OpCode << 8) + (Code & 0x80) + (Bit << 4) + AdrPart;
                break;
              case ModIReg:
                CodeLen = 4;
                WAsmCode[0] = 0x7c00 + (AdrPart << 4);
                WAsmCode[1] = (OpCode << 8) + (Code & 0x80) + (Bit << 4);
                if (OpCode < 0x70)
                  WAsmCode[0] += 0x100;
                break;
              case ModAbs8:
                CodeLen = 4;
                WAsmCode[0] = 0x7e00 + Lo(AdrVals[0]);
                WAsmCode[1] = (OpCode << 8) + (Code & 0x80) + (Bit << 4);
                if (OpCode < 0x70)
                  WAsmCode[0] += 0x100;
                break;
            }
          }
        }
      }
    }
  }
}

static void DecodeBit2(Word Code)
{
  Word OpCode;
  Byte Bit;
  Boolean OK;
  ShortInt HSize;

  if (ArgCnt != 2) WrError(1110);
  else
  {
    if (*ArgStr[1] == '#')
    {
      OpCode = Code + 0x70;
      Bit = EvalIntExpression(ArgStr[1] + 1, UInt3, &OK);
    }
    else
    {
      OpCode = Code + 0x60;
      OK = DecodeReg(ArgStr[1], &Bit, &HSize);
      if (!OK) WrError(1350);
      if ((OK) && (HSize != 0))
      {
        WrError(1130);
        OK = False;
      }
    }
    if (OK)
    {
      DecodeAdr(ArgStr[2], MModReg | MModIReg | MModAbs8);
      if (AdrMode != ModNone)
      {
        if (OpSize > 0) WrError(1130);
        else
        {
          switch (AdrMode)
          {
            case ModReg:
              CodeLen = 2;
              WAsmCode[0] = (OpCode << 8) + (Bit << 4) + AdrPart;
              break;
            case ModIReg:
              CodeLen = 4;
              WAsmCode[0] = 0x7d00 + (AdrPart << 4);
              WAsmCode[1] = (OpCode << 8) + (Bit << 4);
              if (Code == 3)
                WAsmCode[0] -= 0x100;
              break;
            case ModAbs8:
              CodeLen = 4;
              WAsmCode[0] = 0x7f00 + Lo(AdrVals[0]);
              WAsmCode[1] = (OpCode << 8) + (Bit << 4);
              if (Code == 3)
                WAsmCode[0] -= 0x100;
              break;
          }
        }
      }
    }
  }
}

static void DecodeINC_DEC(Word Code)
{
  Boolean OK;
  int z;
  Byte HReg;

  if ((ArgCnt != 1) && (ArgCnt != 2)) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[ArgCnt], MModReg);
    if (AdrMode != ModNone)
    {
      if ((OpSize > 0) && (CPU16)) WrError(1500);
      else
      {
        HReg = AdrPart;
        if (ArgCnt == 1)
        {
          OK = True;
          z = 1;
        }
        else
        {
          DecodeAdr(ArgStr[1], MModImm);
          OK = (AdrMode == ModImm);
          if (OK)
          {
            z = ImmVal();
            if (z < 1)
            {
              WrError(1315);
              OK = False;
            }
            else if (((OpSize == 0) && (z > 1)) || (z > 2))
            {
              WrError(1320);
              OK = False;
            }
          }
        }
        if (OK)
        {
          CodeLen = 2;
          z--;
          switch (OpSize)
          {
            case 0:
              WAsmCode[0] = Code + 0x0a00 + HReg;
              break;
            case 1:
              WAsmCode[0] = Code + 0x0b50 + HReg + (z << 7);
              break;
            case 2:
              WAsmCode[0] = Code + 0x0b70 + HReg + (z << 7);
              break;
          }
        }
      }
    }
  }
}

static void DecodeShift(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode != ModNone)
    {
      if ((OpSize > 0) && (CPU16)) WrError(1500);
      else
      {
        CodeLen = 2;
        switch (OpSize)
        {
          case 0:
            WAsmCode[0] = Code + AdrPart;
            break;
          case 1:
            WAsmCode[0] = Code + AdrPart + 0x10;
            break;
          case 2:
            WAsmCode[0] = Code + AdrPart + 0x30;
            break;
        }
      }
    }
  }
}

static void DecodeNEG_NOT(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode != ModNone)
    {
      if ((OpSize > 0) && (CPU16)) WrError(1500);
      else
      {
        CodeLen = 2;
        switch (OpSize)
        {
          case 0:
            WAsmCode[0] = Code + 0x1700 + AdrPart;
            break;
          case 1:
            WAsmCode[0] = Code + 0x1710 + AdrPart;
            break;
          case 2:
            WAsmCode[0] = Code + 0x1730 + AdrPart;
            break;
        }
      }
    }
  }
}

static void DecodeEXTS_EXTU(Word IsEXTS)
{
  if (ArgCnt != 1) WrError(1110);
  else if (CPU16) WrError(1500);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode != ModNone)
    {
      if ((OpSize != 1) && (OpSize != 2)) WrError(1130);
      else
      {
        CodeLen = 2;
        switch (OpSize)
        {
          case 1:
            WAsmCode[0] = IsEXTS ? 0x17d0 : 0x1750;
            break;
          case 2:
            WAsmCode[0] = IsEXTS ? 0x17f0 : 0x1770;
            break;
        }
        WAsmCode[0] += AdrPart;
      }
    }
  }
}

static void DecodeDAA_DAS(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModReg);
    if (AdrMode != ModNone)
    {
      if (OpSize != 0) WrError(1130);
      else
      {
        CodeLen = 2;
        WAsmCode[0] = Code + AdrPart;
      }
    }
  }
}

static void DecodeCond(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if ((OpSize != -1) && (OpSize != 4) && (OpSize != 2)) WrError(1130);
  else
  {
    Boolean OK;
    LongInt AdrLong = EvalIntExpression(ArgStr[1], Int24, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (OpSize == -1)
      {
        if ((AdrLong >= -128) && (AdrLong <= 127))
          OpSize = 4;
        else
        {
          OpSize = 2;
          AdrLong -= 2;
        }
      }
      else if (OpSize == 2)
        AdrLong-=2;
      if (OpSize == 2)
      {
        if ((!SymbolQuestionable) && ((AdrLong < -32768) || (AdrLong > 32767))) WrError(1370);
        else if (CPU16) WrError(1500);
        else
        {
          CodeLen = 4;
          WAsmCode[0] = 0x5800 + (Code << 4);
          WAsmCode[1] = AdrLong & 0xffff;
        }
      }
      else
      {
        if ((!SymbolQuestionable) && ((AdrLong < -128) || (AdrLong > 127))) WrError(1370);
        else
        {
          CodeLen = 2;
          WAsmCode[0] = 0x4000 + (Code << 8) + (AdrLong & 0xff);
        }
      }
    }
  }
}

static void DecodeJMP_JSR(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModIReg | ((CPU16) ? MModAbs16 : MModAbs24) | MModIIAbs);
    switch (AdrMode)
    {
      case ModIReg:
        CodeLen = 2;
        WAsmCode[0] = 0x5900 + Code + (AdrPart << 4);
        break;
      case ModAbs16:
        CodeLen = 4;
        WAsmCode[0] = 0x5a00 + Code;
        WAsmCode[1] = AdrVals[0];
        break;
      case ModAbs24:
        CodeLen = 4;
        WAsmCode[0] = 0x5a00 + Code + Lo(AdrVals[0]);
        WAsmCode[1] = AdrVals[1];
        break;
      case ModIIAbs:
        CodeLen = 2;
        WAsmCode[0] = 0x5b00 + Code + Lo(AdrVals[0]);
        break;
    }
  }
}

static void DecodeBSR(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if ((OpSize != -1) && (OpSize != 4) && (OpSize != 2)) WrError(1130);
  else
  {
    Boolean OK;
    LongInt AdrLong = EvalIntExpression(ArgStr[1], Int24, &OK) - (EProgCounter() + 2);
    if (OK)
    {
      if (OpSize == -1)
      {
        if ((AdrLong >= -128) && (AdrLong <= 127))
          OpSize = 4;
        else
        {
          OpSize = 2;
          AdrLong -= 2;
        }
      }
      else
      {
        if (OpSize == 2)
          AdrLong -= 2;
      }
      if (OpSize == 2)
      {
        if ((!SymbolQuestionable) && ((AdrLong < -32768) || (AdrLong > 32767))) WrError(1370);
        else if (CPU16) WrError(1500);
        else
        {
          CodeLen = 4;
          WAsmCode[0] = 0x5c00;
          WAsmCode[1] = AdrLong & 0xffff;
        }
      }
      else
      {
        if ((AdrLong < -128) || (AdrLong > 127)) WrError(1370);
        else
        {
          CodeLen = 2;
          WAsmCode[0] = 0x5500 + (AdrLong & 0xff);
        }
      }
    }
  }
}

static void DecodeTRAPA(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (MomCPU < CPU6413309) WrError(1500);
  else
  {
    char *pVal = (*ArgStr[1] == '#') ? ArgStr[1] + 1 : ArgStr[1];
    Boolean OK;

    WAsmCode[0] = EvalIntExpression(pVal, UInt2, &OK) << 4;
    if (OK)
    {
      WAsmCode[0] += 0x5700;
      CodeLen = 2;
    }
  }
}

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddCond(char *NName, Byte NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeCond);
}

static void AddShift(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeShift);
}

static void AddLogic(char *NName, char *NNameBit, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeLogic);
  AddInstTable(InstTable, NNameBit, NCode, DecodeLogicBit);
}

static void AddMul(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeMul);
}

static void AddBit1(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBit1);
}

static void AddBit2(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBit2);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);
  AddInstTable(InstTable, "EEPMOV", 0, DecodeEEPMOV);
  AddInstTable(InstTable, "MOV", 0, DecodeMOV);
  AddInstTable(InstTable, "MOVTPE", 0x80, DecodeMOVTPE_MOVFPE);
  AddInstTable(InstTable, "MOVFPE", 0x00, DecodeMOVTPE_MOVFPE);
  AddInstTable(InstTable, "PUSH", 0x6df0, DecodePUSH_POP);
  AddInstTable(InstTable, "POP", 0x6d70, DecodePUSH_POP);
  AddInstTable(InstTable, "LDC", 0x00, DecodeLDC_STC);
  AddInstTable(InstTable, "STC", 0x80, DecodeLDC_STC);
  AddInstTable(InstTable, "ADD", 0, DecodeADD_SUB);
  AddInstTable(InstTable, "SUB", 1, DecodeADD_SUB);
  AddInstTable(InstTable, "CMP", 0, DecodeCMP);
  AddInstTable(InstTable, "ADDX", 0, DecodeADDX_SUBX);
  AddInstTable(InstTable, "SUBX", 1, DecodeADDX_SUBX);
  AddInstTable(InstTable, "ADDS", 0x0000, DecodeADDS_SUBS);
  AddInstTable(InstTable, "SUBS", 0x1000, DecodeADDS_SUBS);
  AddInstTable(InstTable, "INC", 0x0000, DecodeINC_DEC);
  AddInstTable(InstTable, "DEC", 0x1000, DecodeINC_DEC);
  AddInstTable(InstTable, "NEG", 0x80, DecodeNEG_NOT);
  AddInstTable(InstTable, "NOT", 0x00, DecodeNEG_NOT);
  AddInstTable(InstTable, "EXTS", 1, DecodeEXTS_EXTU);
  AddInstTable(InstTable, "EXTU", 0, DecodeEXTS_EXTU);
  AddInstTable(InstTable, "DAA", 0x0f00, DecodeDAA_DAS);
  AddInstTable(InstTable, "DAS", 0x1f00, DecodeDAA_DAS);
  AddInstTable(InstTable, "JMP", 0x0000, DecodeJMP_JSR);
  AddInstTable(InstTable, "JSR", 0x0400, DecodeJMP_JSR);
  AddInstTable(InstTable, "BSR", 0, DecodeBSR);
  AddInstTable(InstTable, "TRAPA", 0, DecodeTRAPA);

  AddFixed("NOP", 0x0000); AddFixed("RTE"  , 0x5670);
  AddFixed("RTS", 0x5470); AddFixed("SLEEP", 0x0180);

  AddCond("BRA", 0x0); AddCond("BT" , 0x0);
  AddCond("BRN", 0x1); AddCond("BF" , 0x1);
  AddCond("BHI", 0x2); AddCond("BLS", 0x3);
  AddCond("BCC", 0x4); AddCond("BHS", 0x4);
  AddCond("BCS", 0x5); AddCond("BLO", 0x5);
  AddCond("BNE", 0x6); AddCond("BEQ", 0x7);
  AddCond("BVC", 0x8); AddCond("BVS", 0x9);
  AddCond("BPL", 0xa); AddCond("BMI", 0xb);
  AddCond("BGE", 0xc); AddCond("BLT", 0xd);
  AddCond("BGT", 0xe); AddCond("BLE", 0xf);

  AddShift("ROTL" , 0x1280); AddShift("ROTR" , 0x1380);
  AddShift("ROTXL", 0x1200); AddShift("ROTXR", 0x1300);
  AddShift("SHAL" , 0x1080); AddShift("SHAR" , 0x1180);
  AddShift("SHLL" , 0x1000); AddShift("SHLR" , 0x1100);

  AddLogic("OR", "ORC", 0);
  AddLogic("XOR", "XORC", 1);
  AddLogic("AND", "ANDC", 2);

  AddMul("DIVXS", 3);
  AddMul("DIVXU", 1);
  AddMul("MULXS", 2);
  AddMul("MULXU", 0);

  AddBit1("BAND", 0x16); AddBit1("BIAND", 0x96);
  AddBit1("BOR" , 0x14); AddBit1("BIOR" , 0x94);
  AddBit1("BXOR", 0x15); AddBit1("BIXOR", 0x95);
  AddBit1("BLD" , 0x17); AddBit1("BILD" , 0x97);
  AddBit1("BST" , 0x07); AddBit1("BIST" , 0x87);

  AddBit2("BCLR", 2);
  AddBit2("BNOT", 1);
  AddBit2("BSET", 0);
  AddBit2("BTST", 3);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_H8_3(void)
{
  CodeLen = 0; DontPrint = False; OpSize = -1;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Attribut verwursten */

  if (*AttrPart != '\0')
  {
    if (strlen(AttrPart) != 1)
    {
      WrXError(1105, AttrPart);
      return;
    }
    switch (mytoupper(*AttrPart))
    {
      case 'B': SetOpSize(0); break;
      case 'W': SetOpSize(1); break;
      case 'L': SetOpSize(2); break;
      case 'Q': SetOpSize(3); break;
      case 'S': SetOpSize(4); break;
      case 'D': SetOpSize(5); break;
      case 'X': SetOpSize(6); break;
      case 'P': SetOpSize(7); break;
      default:
        WrError(1107); return;
    }
  }

  if (DecodeMoto16Pseudo(OpSize, True)) return;

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200,OpPart);
}

static Boolean IsDef_H8_3(void)
{
  return False;
}

static void SwitchFrom_H8_3(void)
{
  DeinitFields();
  ClearONOFF();
}

static void SwitchTo_H8_3(void)
{
  TurnWords = True;
  ConstMode = ConstModeMoto;
  SetIsOccupied = False;

  PCSymbol = "*";
  HeaderID = 0x68;
  NOPCode = 0x0000;
  DivideChars = ",";
  HasAttrs = True;
  AttrChars = ".";

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1;
  ListGrans[SegCode] = 2;
  SegInits[SegCode] = 0;
  SegLimits[SegCode] = (MomCPU <= CPUH8_300) ? 0xffff : 0xffffffl;

  MakeCode = MakeCode_H8_3;
  IsDef = IsDef_H8_3;
  SwitchFrom = SwitchFrom_H8_3;
  InitFields();
  AddONOFF("MAXMODE", &Maximum   , MaximumName   , False);
  AddONOFF("PADDING", &DoPadding , DoPaddingName , False);
  AddMoto16PseudoONOFF();

  CPU16 = (MomCPU <= CPUH8_300);

  SetFlag(&DoPadding,DoPaddingName,False);
}

void codeh8_3_init(void)
{
  CPUH8_300L = AddCPU("H8/300L"   , SwitchTo_H8_3);
  CPU6413308 = AddCPU("HD6413308" , SwitchTo_H8_3);
  CPUH8_300  = AddCPU("H8/300"    , SwitchTo_H8_3);
  CPU6413309 = AddCPU("HD6413309" , SwitchTo_H8_3);
  CPUH8_300H = AddCPU("H8/300H"   , SwitchTo_H8_3);
}
