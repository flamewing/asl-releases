/* codem16c.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator M16C                                                        */
/*                                                                           */
/* Historie: 17.12.1996 Grundsteinlegung                                     */
/*           21. 9.1998 Kodierungsfehler: mov.b:s #0,...->dests vergessen    */
/*                                        mov.x:s #imm,reg->OpSize invertiert*/
/*                                        sub.x:q #imm4,...->falscher Opcode */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*   {RMS}    6. 2.1999 Fixed remaining code generation errors - M16C is now */
/*                      100% correct, validated against reference assemblers */
/*                      and official Mitsubishi documentation.               */
/*                      Search for RMS: tags to see changes                  */
/*   {RMS}    8. 2.1999 Fixed ChkPC SegLimit typo [M16s have 20 bits]        */
/*   {RMS}   10. 2.1999 Accomodate JMP.S crossing 64k boundary bug in M16C   */
/*   {RMS}    2. 4.1999 Made the JMP.S promotion fix CPU-dependent, and made */
/*                      repairs to the JMP.S handling for forward label refs */
/*                      so they now work. [JMP.S symbol] now works.          */
/*   {RMS}   13.12.1999 One MORE fix to JMP[.S] - this time for something    */
/*                      silly, and probably not that common.  But it caused  */
/*                      a very annoying bug all the same: infinite phase     */
/*                      errors causing an infinite number of passes! Oops. :)*/
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: codem16c.c,v 1.12 2014/12/07 19:14:01 alfred Exp $                    */
/*****************************************************************************
 * $Log: codem16c.c,v $
 * Revision 1.12  2014/12/07 19:14:01  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.11  2014/12/05 11:15:28  alfred
 * - eliminate AND/OR/NOT
 *
 * Revision 1.10  2014/12/05 08:53:45  alfred
 * - eliminate remaining BEGIN/END
 *
 * Revision 1.9  2014/06/13 20:58:48  alfred
 * - adapt to current style
 *
 * Revision 1.8  2010/04/11 11:38:43  alfred
 * - correct SHA coding
 *
 * Revision 1.7  2010/04/04 08:31:16  alfred
 * - correct some coding details
 *
 * Revision 1.6  2010/03/27 14:21:04  alfred
 * - correct address range check
 *
 * Revision 1.5  2007/11/24 22:48:07  alfred
 * - some NetBSD changes
 *
 * Revision 1.4  2005/09/30 12:53:49  alfred
 * - correct include statements
 *
 * Revision 1.3  2005/09/08 17:06:29  alfred
 * - dynamically allocate string
 *
 * Revision 1.2  2004/05/29 11:33:03  alfred
 * - relocated DecodeIntelPseudo() into own module
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
#include "asmitree.h"
#include "intpseudo.h"
#include "codevars.h"

#define ModNone (-1)
#define ModGen 0
#define MModGen (1 << ModGen)
#define ModAbs20 1
#define MModAbs20 (1 << ModAbs20)
#define ModAReg32 2
#define MModAReg32 (1 << ModAReg32)
#define ModDisp20 3
#define MModDisp20 (1 << ModDisp20)
#define ModReg32 4
#define MModReg32 (1 << ModReg32)
#define ModIReg32 5
#define MModIReg32 (1 << ModIReg32)
#define ModImm 6
#define MModImm (1 << ModImm)
#define ModSPRel 7
#define MModSPRel (1 << ModSPRel)

#define FixedOrderCnt 8
#define Gen2OrderCnt 6
#define DivOrderCnt 3
#define ConditionCnt 18
#define BCDOrderCnt 4

typedef struct
{
  Byte Code1,Code2,Code3;
} Gen2Order;

static char *Flags = "CDZSBOIU";

static CPUVar CPUM16C, CPUM30600M8, CPUM30610, CPUM30620;

static char *Format;
static Byte FormatCode;
static ShortInt OpSize;
static Byte AdrMode,AdrMode2;
static ShortInt AdrType,AdrType2;
static Byte AdrCnt2;
static Byte AdrVals[3],AdrVals2[3];

static Gen2Order *Gen2Orders;
static Gen2Order *DivOrders;

/*------------------------------------------------------------------------*/
/* Adressparser */

static void SetOpSize(ShortInt NSize)
{
  if (OpSize == -1)
    OpSize = NSize;
  else if (NSize != OpSize)
  {
    WrError(1131);
    AdrCnt = 0;
    AdrType = ModNone;
  }
}

static void DecodeAdr(char *Asc, Word Mask)
{
  LongInt DispAcc;
  String RegPart;
  char *p;
  Boolean OK;
  int l = strlen(Asc);

  AdrCnt = 0; AdrType = ModNone;

  /* Datenregister 8 Bit */

  if ((l == 3)
   && (mytoupper(*Asc) == 'R')
   && (Asc[1] >= '0') && (Asc[1] <= '1')
   && ((mytoupper(Asc[2]) == 'L') || (mytoupper(Asc[2]) == 'H')))
  {
    AdrType = ModGen;
    AdrMode = ((Asc[1] - '0') << 1) + Ord(mytoupper(Asc[2]) == 'H');
    SetOpSize(0);
    goto chk;
  }

  /* Datenregister 16 Bit */

  if ((l == 2)
   && (mytoupper(*Asc) == 'R')
   && (Asc[1] >= '0') && (Asc[1] <= '3'))
  {
    AdrType = ModGen;
    AdrMode = Asc[1] - '0';
    SetOpSize(1);
    goto chk;
  }

  /* Datenregister 32 Bit */

  if (!strcasecmp(Asc, "R2R0"))
  {
    AdrType = ModReg32;
    AdrMode = 0;
    SetOpSize(2);
    goto chk;
  }
  if (!strcasecmp(Asc, "R3R1"))
  {
    AdrType = ModReg32;
    AdrMode = 1;
    SetOpSize(2);
    goto chk;
  }

  /* Adressregister */

  if ((l == 2)
   && (mytoupper(*Asc) == 'A')
   && (Asc[1] >= '0') && (Asc[1] <= '1'))
  {
    AdrType = ModGen;
    AdrMode = Asc[1] - '0' + 4;
    goto chk;
  }

  /* Adressregister 32 Bit */

  if (!strcasecmp(Asc,"A1A0"))
  {
    AdrType = ModAReg32;
    SetOpSize(2);
    goto chk;
  }

  /* indirekt */

  p = strchr(Asc, '[');
  if ((p) && (Asc[l - 1] == ']'))
  {
    strmaxcpy(RegPart, p + 1, 255);
    RegPart[strlen(RegPart) - 1]='\0';
    if ((!strcasecmp(RegPart, "A0")) || (!strcasecmp(RegPart, "A1")))
    {
      *p = '\0';
      DispAcc = EvalIntExpression(Asc, (Mask & MModDisp20)  ? Int20 : Int16, &OK);
      if (OK)
      {
        if ((DispAcc == 0) && (Mask & MModGen))
        {
          AdrType = ModGen;
          AdrMode = RegPart[1] - '0' + 6;
        }
        else if ((DispAcc >= 0) && (DispAcc <= 255) && (Mask & MModGen))
        {
          AdrType = ModGen;
          AdrVals[0] = DispAcc & 0xff;
          AdrCnt = 1;
          AdrMode = RegPart[1] - '0' + 8;
        }
        else if ((DispAcc >= -32768) && (DispAcc <= 65535) && (Mask & MModGen))
        {
          AdrType = ModGen;
          AdrVals[0] = DispAcc & 0xff;
          AdrVals[1] = (DispAcc >> 8) & 0xff;
          AdrCnt = 2;
          AdrMode = RegPart[1] - '0' + 12;
        }
        else if (strcasecmp(RegPart, "A0")) WrError(1350);
        else
        {
          AdrType = ModDisp20;
          AdrVals[0] = DispAcc & 0xff;
          AdrVals[1] = (DispAcc >> 8) & 0xff;
          AdrVals[2] = (DispAcc >> 16) & 0x0f;
          AdrCnt = 3;
          AdrMode = RegPart[1] - '0';
        }
      }
    }
    else if (!strcasecmp(RegPart, "SB"))
    {
      *p = '\0';
      DispAcc = EvalIntExpression(Asc, Int16, &OK);
      if (OK)
      {
        if ((DispAcc >= 0) && (DispAcc <= 255))
        {
          AdrType = ModGen;
          AdrVals[0] = DispAcc & 0xff;
          AdrCnt = 1;
          AdrMode = 10;
        }
        else
        {
          AdrType = ModGen;
          AdrVals[0] = DispAcc & 0xff;
          AdrVals[1] = (DispAcc >> 8) & 0xff;
          AdrCnt = 2;
          AdrMode = 14;
        }
      }
    }
    else if (!strcasecmp(RegPart, "FB"))
    {
      *p = '\0';
      DispAcc = EvalIntExpression(Asc, SInt8, &OK);
      if (OK)
      {
        AdrType = ModGen;
        AdrVals[0] = DispAcc & 0xff;
        AdrCnt = 1;
        AdrMode = 11;
      }
    }
    else if (!strcasecmp(RegPart,"SP"))
    {
      *p = '\0';
      DispAcc = EvalIntExpression(Asc, SInt8, &OK);
      if (OK)
      {
        AdrType = ModSPRel;
        AdrVals[0] = DispAcc & 0xff;
        AdrCnt = 1;
      }
    }
    else if (!strcasecmp(RegPart,"A1A0"))
    {
      *p = '\0';
      DispAcc = EvalIntExpression(Asc, SInt8, &OK);
      if (OK)
      {
        if (DispAcc != 0) WrError(1320);
        else AdrType = ModIReg32;
      }
    }
    goto chk;
  }

  /* immediate */

  if (*Asc == '#')
  {
    switch (OpSize)
    {
      case -1:
        WrError(1132);
        break;
      case 0:
        AdrVals[0] = EvalIntExpression(Asc + 1, Int8, &OK);
        if (OK)
        {
          AdrType = ModImm;
          AdrCnt = 1;
        }
        break;
      case 1:
        DispAcc = EvalIntExpression(Asc + 1, Int16, &OK);
        if (OK)
        {
          AdrType = ModImm;
          AdrVals[0] = DispAcc & 0xff;
          AdrVals[1] = (DispAcc >> 8) & 0xff;
          AdrCnt = 2;
        }
        break;
    }
    goto chk;
  }

  /* dann absolut */

  DispAcc = EvalIntExpression(Asc, (Mask & MModAbs20) ? UInt20 : UInt16, &OK);
  if ((DispAcc <= 0xffff) && ((Mask & MModGen) != 0))
  {
    AdrType = ModGen;
    AdrMode = 15;
    AdrVals[0] = DispAcc & 0xff;
    AdrVals[1] = (DispAcc >> 8) & 0xff;
    AdrCnt = 2;
  }
  else
  {
    AdrType = ModAbs20;
    AdrVals[0] = DispAcc & 0xff;
    AdrVals[1] = (DispAcc >> 8) & 0xff;
    AdrVals[2] = (DispAcc >> 16) & 0x0f;
    AdrCnt = 3;
  }

chk:
  if ((AdrType != ModNone) && ((Mask & (1 << AdrType)) == 0))
  {
     AdrCnt = 0;
     AdrType = ModNone;
     WrError(1350);
  }
}

static Boolean DecodeReg(char *Asc, Byte *Erg)
{
  if (!strcasecmp(Asc, "FB"))
    *Erg = 7;
  else if (!strcasecmp(Asc, "SB"))
    *Erg = 6;
  else if ((strlen(Asc) == 2)
        && (mytoupper(*Asc) == 'A')
        && (Asc[1] >= '0') && (Asc[1] <= '1'))
    *Erg = Asc[1] - '0' + 4;
  else if ((strlen(Asc) == 2)
        && (mytoupper(*Asc) == 'R')
        && (Asc[1] >= '0') && (Asc[1] <= '3'))
    *Erg = Asc[1] - '0';
  else
    return False;
  return True;
}

static Boolean DecodeCReg(char *Asc, Byte *Erg)
{
  if (!strcasecmp(Asc, "INTBL")) *Erg = 1;
  else if (!strcasecmp(Asc, "INTBH")) *Erg = 2;
  else if (!strcasecmp(Asc, "FLG")) *Erg = 3;
  else if (!strcasecmp(Asc, "ISP")) *Erg = 4;
  else if (!strcasecmp(Asc, "SP")) *Erg = 5;
  else if (!strcasecmp(Asc, "SB")) *Erg = 6;
  else if (!strcasecmp(Asc, "FB")) *Erg = 7;
  else
  {
    WrXError(1440, Asc);
    return False;
  }
  return True;
}

static void DecodeDisp(char *Asc, IntType Type1, IntType Type2, LongInt *DispAcc, Boolean *OK)
{
  if (ArgCnt == 2)
    *DispAcc += EvalIntExpression(Asc, Type2, OK) * 8;
  else
    *DispAcc = EvalIntExpression(Asc, Type1, OK);
}

static Boolean DecodeBitAdr(Boolean MayShort)
{
  LongInt DispAcc;
  Boolean OK;
  char *Pos1;
  String Asc,Reg;

  AdrCnt = 0;

  /* Nur 1 oder 2 Argumente zugelassen */

  if ((ArgCnt < 1) || (ArgCnt>2))
  {
    WrError(1110);
    return False;
  }

  /* Ist Teil 1 ein Register ? */

  if ((DecodeReg(ArgStr[ArgCnt], &AdrMode)))
  {
    if (AdrMode < 6)
    {
      if (ArgCnt != 2) WrError(1110);
      else
      {
        AdrVals[0] = EvalIntExpression(ArgStr[1], UInt4, &OK);
        if (OK)
        {
          AdrCnt = 1;
          return True;
        }
      }
      return False;
    }
  }

  /* Bitnummer ? */

  if (ArgCnt == 2)
  {
    DispAcc = EvalIntExpression(ArgStr[1], UInt16, &OK); /* RMS 02: The displacement can be 16 bits */
    if (!OK)
      return False;
  }
  else
    DispAcc = 0;

  /* Registerangabe ? */

  strmaxcpy(Asc, ArgStr[ArgCnt], 255);
  Pos1 = QuotPos(Asc, '[');

  /* nein->absolut */

  if (!Pos1)
  {
    DecodeDisp(Asc, UInt16, UInt13, &DispAcc, &OK);
    if ((OK) && (DispAcc < 0x10000))     /* RMS 09: This is optional, it detects rollover of the bit address. */ 
    {
      AdrMode = 15;
      AdrVals[0] = DispAcc & 0xff;
      AdrVals[1] = (DispAcc >> 8) & 0xff;
      AdrCnt = 2;
      return True;
    }
    WrError(1510);                     /* RMS 08: Notify user there's a problem with address */
    return False;
  }

  /* Register abspalten */

  if ((*Asc) && (Asc[strlen(Asc) - 1] != ']'))
  {
    WrError(1350);
    return False;
  }
  *Pos1 = '\0';
  strmaxcpy(Reg, Pos1 + 1, 255);
  Reg[strlen(Reg) - 1] = '\0';

  if ((strlen(Reg) == 2) && (mytoupper(*Reg) == 'A') && (Reg[1] >= '0') && (Reg[1] <= '1'))
  {
    AdrMode = Reg[1] - '0';
    DecodeDisp(Asc, UInt16, UInt16, &DispAcc, &OK); /* RMS 03: The offset is a full 16 bits */
    if (OK)
    {
      if (DispAcc == 0) AdrMode += 6;
      else if ((DispAcc > 0) && (DispAcc < 256))
      {
        AdrMode += 8;
        AdrVals[0] = DispAcc & 0xff;
        AdrCnt = 1;
      }
      else
      {
        AdrMode += 12;
        AdrVals[0] = DispAcc & 0xff;
        AdrVals[1] = (DispAcc >> 8) & 0xff;
        AdrCnt = 2;
      }
      return True;
    }
    WrError(1510);             /* RMS 08: Notify user there's a problem with the offset */
    return False;
  }
  else if (!strcasecmp(Reg,"SB"))
  {
    DecodeDisp(Asc, UInt13, UInt16, &DispAcc, &OK);
    if (OK)
    {
      if ((MayShort) && (DispAcc <= 0x7ff))
      {
        AdrMode = 16 + (DispAcc & 7);
        AdrVals[0] = DispAcc >> 3;
        AdrCnt = 1;
      }
      else if ((DispAcc > 0) && (DispAcc < 256))
      {
        AdrMode = 10;
        AdrVals[0] = DispAcc & 0xff;
        AdrCnt = 1;
      }
      else
      {
        AdrMode = 14;
        AdrVals[0] = DispAcc & 0xff;
        AdrVals[1] = (DispAcc >> 8) & 0xff;
        AdrCnt = 2;
      }
      return True;
    }
    WrError(1510);             /* RMS 08: Notify user there's a problem with the offset */
    return False;
  }
  else if (!strcasecmp(Reg,"FB"))
  {
    DecodeDisp(Asc, SInt5, SInt8, &DispAcc, &OK);
    if (OK)
    {
      AdrMode = 11;
      AdrVals[0] = DispAcc & 0xff;
      AdrCnt = 1;
      return True;
    }
    WrError(1510);             /* RMS 08: Notify user there's a problem with the offset */
    return False;
  }
  else
  {
    WrXError(1445, Reg);
    return False;
  }
}

static Boolean CheckFormat(char *FSet)
{
  char *p;

  if (!strcmp(Format, " "))
  {
    FormatCode = 0;
    return True;
  }
  else
  {
    p = strchr(FSet, *Format);
    if (!p) WrError(1090);
    else FormatCode = p - FSet + 1;
    return (p != 0);
  }
}

static Integer ImmVal(void)
{
  if (OpSize == 0)
    return (ShortInt)AdrVals[0];
  else
    return (((Integer)AdrVals[1]) << 8) + AdrVals[0];
}

static Boolean IsShort(Byte GenMode, Byte *SMode)
{
  switch (GenMode)
  {
    case 0:  *SMode = 4; break;
    case 1:  *SMode = 3; break;
    case 10: *SMode = 5; break;
    case 11: *SMode = 6; break;
    case 15: *SMode = 7; break;
    default: return False;
  }
  return True;
}

static void CopyAdr(void)
{
  AdrType2 = AdrType;
  AdrMode2 = AdrMode;
  AdrCnt2 = AdrCnt;
  memcpy(AdrVals2, AdrVals, AdrCnt2);
}

static void CodeGen(Byte GenCode,Byte Imm1Code,Byte Imm2Code)
{
  if (AdrType == ModImm)
  {
    BAsmCode[0] = Imm1Code + OpSize;
    BAsmCode[1] = Imm2Code + AdrMode2;
    memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
    memcpy(BAsmCode + 2 + AdrCnt2, AdrVals, AdrCnt);
  }
  else
  {
    BAsmCode[0] = GenCode + OpSize;
    BAsmCode[1] = (AdrMode << 4) + AdrMode2;
    memcpy(BAsmCode + 2, AdrVals, AdrCnt);
    memcpy(BAsmCode + 2 + AdrCnt, AdrVals2, AdrCnt2);
  }
  CodeLen = 2 + AdrCnt + AdrCnt2;
}

/*------------------------------------------------------------------------*/
/* instruction decoders */

static void DecodeFixed(Word Code)
{
  if (ArgCnt != 0) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (strcmp(Format, " ")) WrError(1090);
  else if (!Hi(Code))
  {
    BAsmCode[0] = Lo(Code);
    CodeLen = 1;
  }
  else
  {
    BAsmCode[0] = Hi(Code);
    BAsmCode[1] = Lo(Code);
    CodeLen = 2;
  }
}

static void DecodeString(Word Code)
{
  if (OpSize == -1) OpSize = 1;
  if (ArgCnt != 0) WrError(1110);
  else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
  else if (strcmp(Format," ")) WrError(1090);
  else if (!Hi(Code))
  {
    BAsmCode[0] = Lo(Code) + OpSize;
    CodeLen = 1;
  }
  else
  {
    BAsmCode[0] = Hi(Code) + OpSize;
    BAsmCode[1] = Lo(Code);
    CodeLen = 2;
  }
}

static void DecodeMOV(Word Code)
{
  Integer Num1;
  Byte SMode;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("GSQZ"))
  {
    DecodeAdr(ArgStr[2], MModGen | MModSPRel);
    if (AdrType != ModNone)
    {
      CopyAdr();
      DecodeAdr(ArgStr[1], MModGen | MModSPRel | MModImm);
      if (AdrType != ModNone)
      {
        if (OpSize == -1) WrError(1132);
        else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
        else
        {
          if (FormatCode == 0)
          {
            if ((AdrType2 == ModSPRel) || (AdrType == ModSPRel))
              FormatCode = 1;
            else if ((OpSize == 0) && (AdrType == ModImm) && (IsShort(AdrMode2, &SMode)))
              FormatCode = (ImmVal() == 0) ? 4 : 2;
            else if ((AdrType == ModImm) && (ImmVal() >= -8) && (ImmVal() <= 7))
              FormatCode = 3;
            else if ((AdrType == ModImm) && ((AdrMode2 & 14) == 4))
              FormatCode = 2;
            else if ((OpSize == 0) && (AdrType == ModGen) && (IsShort(AdrMode, &SMode)) && ((AdrMode2 & 14) == 4)
                  && ((AdrMode >= 2) || (Odd(AdrMode ^ AdrMode2))))
              FormatCode = 2;
            else if ((OpSize == 0) && (AdrType == ModGen) && (AdrMode <= 1) && (IsShort(AdrMode2, &SMode))
                  && ((AdrMode2 >= 2) || (Odd(AdrMode ^ AdrMode2))))
              FormatCode = 2;
            else if ((OpSize == 0) && (AdrMode2 <= 1) && (AdrType == ModGen) && (IsShort(AdrMode, &SMode))
                  && ((AdrMode >= 2) || (Odd(AdrMode ^ AdrMode2))))
              FormatCode = 2;
            else
              FormatCode = 1;
          }
          switch (FormatCode)
          {
            case 1:
              if (AdrType == ModSPRel)
              {
                BAsmCode[0] = 0x74 + OpSize;
                BAsmCode[1] = 0xb0 + AdrMode2;
                memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                memcpy(BAsmCode + 2 + AdrCnt2, AdrVals, AdrCnt);
                CodeLen = 2 + AdrCnt + AdrCnt2;
              }
              else if (AdrType2 == ModSPRel)
              {
                BAsmCode[0] = 0x74 + OpSize;
                BAsmCode[1] = 0x30 + AdrMode;
                memcpy(BAsmCode + 2, AdrVals, AdrCnt);
                memcpy(BAsmCode + 2 + AdrCnt, AdrVals2, AdrCnt2);
                CodeLen = 2 + AdrCnt2 + AdrCnt;
              }
              else
                CodeGen(0x72, 0x74, 0xc0);
              break;
            case 2:
              if (AdrType == ModImm)
              {
                if (AdrType2 != ModGen) WrError(1350);
                else if ((AdrMode2 & 14) == 4)
                {
                  BAsmCode[0] = 0xe2 - (OpSize << 6) + ((AdrMode2 & 1) << 3);
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                  CodeLen = 1 + AdrCnt;
                }
                else if (IsShort(AdrMode2, &SMode))
                {
                  if (OpSize != 0) WrError(1130);
                  else
                  {
                    BAsmCode[0] = 0xc0 + SMode;
                    memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                    memcpy(BAsmCode + 1 + AdrCnt, AdrVals2, AdrCnt2);
                    CodeLen = 1 + AdrCnt + AdrCnt2;
                  }
                }
                else
                  WrError(1350);
              }
              else if ((AdrType == ModGen) && (IsShort(AdrMode, &SMode)))
              {
                if (AdrType2 != ModGen) WrError(1350);
                else if ((AdrMode2 & 14) == 4)
                {
                  if ((AdrMode <= 1) && (!Odd(AdrMode ^ AdrMode2))) WrError(1350);
                  else
                  {
                    if (SMode == 3) SMode++;
                    BAsmCode[0] = 0x30 + ((AdrMode2 & 1) << 2) + (SMode & 3);
                    memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                    CodeLen = 1 + AdrCnt;
                  }
                }
                else if ((AdrMode2 & 14) == 0)
                {
                  if ((AdrMode <= 1) && (!Odd(AdrMode ^ AdrMode2))) WrError(1350);
                  else
                  {
                    if (SMode == 3) SMode++;
                    BAsmCode[0] = 0x08 + ((AdrMode2 & 1) << 2) + (SMode & 3);
                    memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                    CodeLen = 1 + AdrCnt;
                  }
                }
                else if (((AdrMode & 14) != 0) || (!IsShort(AdrMode2, &SMode))) WrError(1350);
                else if ((AdrMode2 <= 1) && (!Odd(AdrMode ^ AdrMode2))) WrError(1350);
                else
                {
                  if (SMode == 3) SMode++;
                  BAsmCode[0] = 0x00 + ((AdrMode & 1) << 2) + (SMode & 3);
                  memcpy(BAsmCode + 1, AdrVals, AdrCnt2);
                  CodeLen = 1 + AdrCnt2;
                }
              }
              else
                WrError(1350);
              break;
            case 3:
              if (AdrType != ModImm) WrError(1350);
              else
              {
                Num1 = ImmVal();
                if (ChkRange(Num1, -8, 7))
                {
                  BAsmCode[0] = 0xd8 + OpSize;
                  BAsmCode[1] = (Num1 << 4) + AdrMode2;
                  memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                  CodeLen = 2 + AdrCnt2;
                }
              }
              break;
            case 4:
              if (OpSize != 0) WrError(1130);
              else if (AdrType != ModImm) WrError(1350);
              else if (!IsShort(AdrMode2, &SMode)) WrError(1350);
              else
              {
                Num1 = ImmVal();
                if (ChkRange(Num1, 0, 0))
                {
                  BAsmCode[0] = 0xb0 + SMode;
                  memcpy(BAsmCode + 1, AdrVals2, AdrCnt2);
                  CodeLen = 1 + AdrCnt2;
                }
              }
              break;
          }
        }
      }
    }
  }
}

static void DecodeLDC_STC(Word IsSTC)
{
  Byte CReg;

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("G"))
  {
    char *pRegArg = IsSTC ? ArgStr[1] : ArgStr[2],
         *pMemArg = IsSTC ? ArgStr[2] : ArgStr[1];

    if (!strcasecmp(pRegArg, "PC"))
    {
      if (!IsSTC) WrError(1350);
      else
      {
        DecodeAdr(pMemArg, MModGen | MModReg32 | MModAReg32);
        if (AdrType == ModAReg32)
          AdrMode = 4;
        if ((AdrType == ModGen) && (AdrMode < 6)) WrError(1350);
        else
        {
          BAsmCode[0] = 0x7c;
          BAsmCode[1] = 0xc0 + AdrMode;
          memcpy(BAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 2 + AdrCnt;
        }
      }
    }
    else if (DecodeCReg(pRegArg, &CReg))
    {
      SetOpSize(1);
      DecodeAdr(pMemArg, MModGen | (IsSTC ? 0 : MModImm));
      if (AdrType == ModImm)
      {
        BAsmCode[0] = 0xeb;
        BAsmCode[1] = CReg << 4;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
      }
      else if (AdrType == ModGen)
      {
        BAsmCode[0] = 0x7a + IsSTC;
        BAsmCode[1] = 0x80 + (CReg << 4) + AdrMode;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeLDCTX_STCTX(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else
  {
    DecodeAdr(ArgStr[1], MModGen);
    if (AdrType == ModGen)
    {
      if (AdrMode != 15) WrError(1350);
      else
      {
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        DecodeAdr(ArgStr[2], MModAbs20);
        if (AdrType == ModAbs20)
        {
          memcpy(BAsmCode + 4, AdrVals, AdrCnt);
          BAsmCode[0] = Code;
          BAsmCode[1] = 0xf0;
          CodeLen = 7;
        }
      }
    }
  }
}

static void DecodeLDE_STE(Word IsLDE)
{
  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("G"))
  {
    char *pArg1 = IsLDE ? ArgStr[2] : ArgStr[1],
         *pArg2 = IsLDE ? ArgStr[1] : ArgStr[2];

    DecodeAdr(pArg1, MModGen);
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if (OpSize > 1) WrError(1130);
      else
      {
        CopyAdr();
        DecodeAdr(pArg2, MModAbs20 | MModDisp20 | MModIReg32);
        if (AdrType != ModNone)
        {
          BAsmCode[0] = 0x74 + OpSize;
          BAsmCode[1] = (IsLDE << 7) + AdrMode2;
          switch (AdrType)
          {
            case ModDisp20: BAsmCode[1] += 0x10; break;
            case ModIReg32: BAsmCode[1] += 0x20; break;
          }
          memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
          memcpy(BAsmCode + 2 + AdrCnt2, AdrVals, AdrCnt);
          CodeLen = 2 + AdrCnt2 + AdrCnt;
        }
      }
    }
  }
}

static void DecodeMOVA(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[1], MModGen);
    if (AdrType != ModNone)
    {
      if (AdrMode < 8) WrError(1350);
      else
      {
        CopyAdr(); DecodeAdr(ArgStr[2], MModGen);
        if (AdrType != ModNone)
        {
          if (AdrMode > 5) WrError(1350);
          else
          {
            BAsmCode[0] = 0xeb;
            BAsmCode[1] = (AdrMode << 4) + AdrMode2;
            memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
            CodeLen = 2 + AdrCnt2;
          }
        }
      }
    }
  }
}

static void DecodeDir(Word Code)
{
  Boolean OK;
  Integer Num1;

  if (ArgCnt != 2) WrError(1110);
  else if (OpSize > 0) WrError(1130);
  else if (CheckFormat("G"))
  {
    OK = True; Num1 = 0;
    if (!strcasecmp(ArgStr[2], "R0L"));
    else if (!strcasecmp(ArgStr[1], "R0L")) Num1 = 1;
    else OK = False;
    if (!OK) WrError(1350);
    else
    {
      DecodeAdr(ArgStr[Num1 + 1], MModGen);
      if (AdrType != ModNone)
      {
        if (((AdrMode & 14) == 4) || ((AdrMode == 0) && (Num1 == 1))) WrError(1350);
        else
        {
          BAsmCode[0] = 0x7c;
          BAsmCode[1] = (Num1 << 7) + (Code << 4) + AdrMode;
          memcpy(BAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 2 + AdrCnt;
        }
      }
    }
  }
}

static void DecodePUSH_POP(Word IsPOP)
{
  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("GS"))
  {
    DecodeAdr(ArgStr[1], MModGen | (IsPOP ? 0 : MModImm));
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if (OpSize > 1) WrError(1130);
      else
      {
        if (FormatCode == 0)
        {
          if ((AdrType != ModGen))
            FormatCode = 1;
          else if ((OpSize == 0) && (AdrMode < 2))
            FormatCode = 2;
          else if ((OpSize == 1) && ((AdrMode & 14) == 4))
            FormatCode = 2;
          else
            FormatCode = 1;
        }
        switch (FormatCode)
        {
          case 1:
            if (AdrType == ModImm)
            {
              BAsmCode[0] = 0x7c + OpSize;
              BAsmCode[1] = 0xe2;
            }
            else
            {
              BAsmCode[0] = 0x74 + OpSize;
              BAsmCode[1] = 0x40 + (IsPOP * 0x90) + AdrMode;
            }
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
            break;
          case 2:
            if (AdrType != ModGen) WrError(1350);
            else if ((OpSize == 0) && (AdrMode < 2))
            {
              BAsmCode[0] = 0x82 + (AdrMode << 3) + (IsPOP << 4);
              CodeLen = 1;
            }
            else if ((OpSize == 1) && ((AdrMode & 14) == 4))
            {
              BAsmCode[0] = 0xc2 + ((AdrMode & 1) << 3) + (IsPOP << 4);
              CodeLen = 1;
            }
            else
              WrError(1350);
            break;
        }
      }
    }
  }
}

static void DecodePUSHC_POPC(Word Code)
{
  Byte CReg;

  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("G"))
   if (DecodeCReg(ArgStr[1], &CReg))
   {
     BAsmCode[0] = 0xeb;
     BAsmCode[1] = Code + (CReg << 4);
     CodeLen = 2;
   }
}

static void DecodePUSHM_POPM(Word IsPOPM)
{
  int z;
  Boolean OK;
  Byte Reg;

  if (ArgCnt < 1) WrError(1110);
  else
  {
    BAsmCode[1] = 0; OK = True; z = 1;
    while ((OK) && (z <= ArgCnt))
    {
      OK = DecodeReg(ArgStr[z], &Reg);
      if (OK)
      {
        BAsmCode[1] |= 1 << (IsPOPM ? Reg : 7 - Reg);
        z++;
      }
    }
    if (!OK)
      WrXError(1440, ArgStr[z]);
    else
    {
      BAsmCode[0] = 0xec + IsPOPM;
      CodeLen = 2;
    }
  }
}

static void DecodePUSHA(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[1], MModGen);
    if (AdrType != ModNone)
    {
      if (AdrMode < 8) WrError(1350);
      else
      {
        BAsmCode[0] = 0x7d;
        BAsmCode[1] = 0x90 + AdrMode;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeXCHG(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[1], MModGen);
    if (AdrType != ModNone)
    {
      CopyAdr();
      DecodeAdr(ArgStr[2], MModGen);
      if (AdrType != ModNone)
      {
        if (OpSize == -1) WrError(1132);
        else if (OpSize > 1) WrError(1130);
        else if (AdrMode2 < 4)
        {
          BAsmCode[0] = 0x7a + OpSize;
          BAsmCode[1] = (AdrMode2 << 4) + AdrMode;
          memcpy(BAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 2 + AdrCnt;
        }
        else if (AdrMode < 4)
        {
          BAsmCode[0] = 0x7a + OpSize;
          BAsmCode[1] = (AdrMode << 4) + AdrMode2;
          memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
          CodeLen = 2 + AdrCnt2;
        }
        else
          WrError(1350);
      }
    }
  }
}

static void DecodeSTZ_STNZ(Word Code)
{
  Byte SMode;

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("G"))
  {
    if (OpSize == -1) OpSize++;
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      if (!IsShort(AdrMode, &SMode)) WrError(1350);
      else
      {
        CopyAdr();
        DecodeAdr(ArgStr[1], MModImm);
        if (AdrType != ModNone)
        {
          BAsmCode[0] = Code + SMode;
          BAsmCode[1] = AdrVals[0];
          memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
          CodeLen =2 + AdrCnt2;
        }
      }
    }
  }
}

static void DecodeSTZX(Word Code)
{
  Integer Num1;
  Byte SMode;

  UNUSED(Code);

  if (ArgCnt != 3) WrError(1110);
  else if (CheckFormat("G"))
  {
    if (OpSize == -1) OpSize++;
    DecodeAdr(ArgStr[3], MModGen);
    if (AdrType != ModNone)
    {
      if (!IsShort(AdrMode, &SMode)) WrError(1350);
      else
      {
        CopyAdr();
        DecodeAdr(ArgStr[1], MModImm);
        if (AdrType != ModNone)
        {
          Num1 = AdrVals[0];
          DecodeAdr(ArgStr[2], MModImm);
          if (AdrType != ModNone)
          {
            BAsmCode[0] = 0xd8 + SMode;
            BAsmCode[1] = Num1;
            memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
            BAsmCode[2 + AdrCnt2] = AdrVals[0];
            CodeLen = 3 + AdrCnt2;
          }
        }
      }
    }
  }
}

static void DecodeADD(Word Code)
{
  Integer Num1;
  Byte SMode;
  LongInt AdrLong;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (!strcasecmp(ArgStr[2], "SP"))
  {
    if (OpSize == -1) OpSize = 1;
    if (CheckFormat("GQ"))
    {
      DecodeAdr(ArgStr[1], MModImm);
      if (AdrType != ModNone)
      {
        if (OpSize == -1) WrError(1132);
        else if (OpSize > 1) WrError(1130);
        else
        {
          AdrLong = ImmVal();
          if (FormatCode == 0)
          {
            if ((AdrLong >= -8) && (AdrLong <= 7)) FormatCode = 2;
            else FormatCode = 1;
          }
          switch (FormatCode)
          {
            case 1:
              BAsmCode[0] = 0x7c + OpSize;
              BAsmCode[1] = 0xeb;
              memcpy(BAsmCode + 2, AdrVals, AdrCnt);
              CodeLen = 2 + AdrCnt;
              break;
            case 2:
              if (ChkRange(AdrLong, -8, 7))
              {
                BAsmCode[0] = 0x7d;
                BAsmCode[1] = 0xb0 + (AdrLong & 15);
                CodeLen = 2;
              }
              break;
          }
        }
      }
    }
  }
  else if (CheckFormat("GQS"))
  {
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      CopyAdr();
      DecodeAdr(ArgStr[1], MModImm | MModGen);
      if (AdrType != ModNone)
      {
        if (OpSize == -1) WrError(1132);
        else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
        else
        {
          if (FormatCode == 0)
          {
            if (AdrType == ModImm)
            {
              if ((ImmVal() >= -8) && (ImmVal() <= 7))
                FormatCode = 2;
              else if ((IsShort(AdrMode2, &SMode)) && (OpSize == 0))
                FormatCode = 3;
              else
                FormatCode = 1;
            }
            else
            {
              if ((OpSize == 0) && (IsShort(AdrMode, &SMode)) && (AdrMode2<=1) &&
                  ((AdrMode > 1) || (Odd(AdrMode ^ AdrMode2))))
                FormatCode = 3;
              else
                FormatCode = 1;
            }
          }
          switch (FormatCode)
          {
            case 1:
              CodeGen(0xa0, 0x76, 0x40);
              break;
            case 2:
              if (AdrType != ModImm) WrError(1350);
              else
              {
                Num1 = ImmVal();
                if (ChkRange(Num1, -8, 7))
                {
                  BAsmCode[0] = 0xc8 + OpSize;
                  BAsmCode[1] = (Num1 << 4) + AdrMode2;
                  memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                  CodeLen = 2 + AdrCnt2;
                }
              }
              break;
            case 3:
              if (OpSize != 0) WrError(1130);
              else if (!IsShort(AdrMode2, &SMode)) WrError(1350);
              else if (AdrType == ModImm)
              {
                BAsmCode[0] = 0x80 + SMode;
                BAsmCode[1] = AdrVals[0];
                memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                CodeLen = 2 + AdrCnt2;
              }
              else if ((AdrMode2 >= 2) || (!IsShort(AdrMode, &SMode))) WrError(1350);
              else if ((AdrMode < 2) && (!Odd(AdrMode ^ AdrMode2))) WrError(1350);
              else
              {
                if (SMode == 3) SMode++;
                BAsmCode[0] = 0x20 + ((AdrMode2 & 1) << 2) + (SMode & 3);    /* RMS 05: Just like #04 */
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
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
  Byte SMode;
  Integer Num1;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("GQS"))
  {
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      CopyAdr();
      DecodeAdr(ArgStr[1], MModImm | MModGen);
      if (AdrType != ModNone)
      {
        if (OpSize == -1) WrError(1132);
        else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
        else
        {
          if (FormatCode == 0)
          {
            if (AdrType == ModImm)
            {
              if ((ImmVal() >= -8) && (ImmVal() <= 7))
                FormatCode = 2;
              else if ((IsShort(AdrMode2, &SMode)) && (OpSize == 0))
                FormatCode = 3;
              else
                FormatCode = 1;
            }
            else
            {
              if ((OpSize == 0) && (IsShort(AdrMode, &SMode)) && (AdrMode2 <= 1) &&
                  ((AdrMode > 1) || (Odd(AdrMode ^ AdrMode2))))
                FormatCode = 3;
              else
                FormatCode = 1;
            }
          }
          switch (FormatCode)
          {
            case 1:
              CodeGen(0xc0, 0x76, 0x80);
              break;
            case 2:
              if (AdrType != ModImm) WrError(1350);
              else
              {
                Num1 = ImmVal();
                if (ChkRange(Num1, -8, 7))
                {
                  BAsmCode[0] = 0xd0 +OpSize;
                  BAsmCode[1] = (Num1 << 4) + AdrMode2;
                  memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                  CodeLen = 2 + AdrCnt2;
                }
                /* else? */
              }
              break; 
            case 3:
              if (OpSize != 0) WrError(1130);
              else if (!IsShort(AdrMode2, &SMode)) WrError(1350);
              else if (AdrType == ModImm)
              {
                BAsmCode[0] = 0xe0 + SMode;
                BAsmCode[1] = AdrVals[0];
                memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                CodeLen = 2 + AdrCnt2;
              }
              else if ((AdrMode2 >= 2) || (!IsShort(AdrMode, &SMode))) WrError(1350);
              else if ((AdrMode < 2) && (!Odd(AdrMode ^ AdrMode2))) WrError(1350);
              else
              {
                if (SMode == 3) SMode++;
                BAsmCode[0] = 0x38 + ((AdrMode2 & 1) << 2) + (SMode & 3); /* RMS 04: destination reg is bit 2! */
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
              }
              break;
          }
        }
      }
    }
  }
}

static void DecodeSUB(Word Code)
{
  Byte SMode;
  Integer Num1;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("GQS"))
  {
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      CopyAdr();
      DecodeAdr(ArgStr[1], MModImm | MModGen);
      if (AdrType != ModNone)
      {
        if (OpSize == -1) WrError(1132);
        else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
        else
        {
          if (FormatCode == 0)
          {
            if (AdrType == ModImm)
            {
              if ((ImmVal() >= -7) && (ImmVal() <= 8))
                FormatCode = 2;
              else if ((IsShort(AdrMode2, &SMode)) && (OpSize == 0))
                FormatCode = 3;
              else
                FormatCode = 1;
            }
            else
            {
              if ((OpSize == 0) && (IsShort(AdrMode, &SMode)) & (AdrMode2 <= 1) &&
                  ((AdrMode > 1) || (Odd(AdrMode ^ AdrMode2))))
                FormatCode = 3;
              else
                FormatCode = 1;
            }
          }
          switch (FormatCode)
          {
            case 1:
              CodeGen(0xa8,0x76,0x50);
              break;
            case 2:
              if (AdrType != ModImm) WrError(1350);
              else
              {
                Num1 = ImmVal();
                if (ChkRange(Num1, -7, 8))
                {
                  BAsmCode[0] = 0xc8 + OpSize;
                  BAsmCode[1] = ((-Num1) << 4) + AdrMode2;
                  memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                  CodeLen = 2 + AdrCnt2;
                }
              }
              break;
            case 3:
              if (OpSize != 0) WrError(1130);
              else if (!IsShort(AdrMode2, &SMode)) WrError(1350);
              else if (AdrType == ModImm)
              {
                BAsmCode[0] = 0x88 + SMode;
                BAsmCode[1] = AdrVals[0];
                memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                CodeLen = 2 + AdrCnt2;
              }
              else if ((AdrMode2 >= 2) || (!IsShort(AdrMode, &SMode))) WrError(1350);
              else if ((AdrMode < 2) && (!Odd(AdrMode ^ AdrMode2))) WrError(1350);
              else
              {
                if (SMode == 3) SMode++;
                BAsmCode[0] = 0x28 + ((AdrMode2 & 1) << 2) + (SMode & 3);    /* RMS 06: just like RMS 04 */
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
              }
              break;
          }
        }
      }
    }
  }
}

static void DecodeGen1(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[1], MModGen);
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
      else
      {
        BAsmCode[0] = Hi(Code) + OpSize;
        BAsmCode[1] = Lo(Code) + AdrMode;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeGen2(Word Index)
{
  Gen2Order *pOrder = Gen2Orders + Index;

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      CopyAdr();
      DecodeAdr(ArgStr[1], MModGen | MModImm);
      if (AdrType != ModNone)
      {
        if (OpSize == -1) WrError(1132);
        else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
        else if ((*OpPart == 'M') && ((AdrMode2 == 3) || (AdrMode2 == 5) || (AdrMode2 - OpSize == 1))) WrError(1350);
        else
          CodeGen(pOrder->Code1, pOrder->Code2, pOrder->Code3);
      }
    }
  }
}

static void DecodeINC_DEC(Word IsDEC)
{
  Byte SMode;

  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[1], MModGen);
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if ((OpSize == 1) && ((AdrMode & 14) == 4))
      {
        BAsmCode[0] = 0xb2 + (IsDEC << 6) + ((AdrMode & 1) << 3);
        CodeLen = 1;
      }
      else if (!IsShort(AdrMode, &SMode)) WrError(1350);
      else if (OpSize != 0) WrError(1130);
      else
      {
        BAsmCode[0] = 0xa0 + (IsDEC << 3) + SMode;
        memcpy(BAsmCode + 1, AdrVals, AdrCnt);
        CodeLen = 1 + AdrCnt;
      }
    }
  }
}

static void DecodeDiv(Word Index)
{
  Gen2Order *pOrder = DivOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[1], MModImm | MModGen);
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if ((OpSize != 0) && (OpSize != 1)) WrError(1130);
      else if (AdrType == ModImm)
      {
        BAsmCode[0] = 0x7c + OpSize;
        BAsmCode[1] = pOrder->Code1;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
      }
      else
      {
        BAsmCode[0] = pOrder->Code2 + OpSize;
        BAsmCode[1] = pOrder->Code3 + AdrMode;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeBCD(Word Code)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      if (AdrMode != 0) WrError(1350);
      else
      {
        DecodeAdr(ArgStr[1], MModGen | MModImm);
        if (AdrType != ModNone)
        {
          if (AdrType == ModImm)
          {
            BAsmCode[0] = 0x7c + OpSize;
            BAsmCode[1] = 0xec + Code;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
          }
          else if (AdrMode != 1) WrError(1350);
          else
          {
            BAsmCode[0] = 0x7c + OpSize;
            BAsmCode[1] = 0xe4 + Code;
            CodeLen = 2;
          }
        }
      }
    }
  }
}

static void DecodeEXTS(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[1], MModGen);
    if (OpSize == -1) OpSize = 0;
    if (AdrType!=ModNone)
    {
      if (OpSize == 0)
      {
        if ((AdrMode == 1) || ((AdrMode >= 3) && (AdrMode <= 5))) WrError(1350);
        else
        {
          BAsmCode[0] = 0x7c;
          BAsmCode[1] = 0x60 + AdrMode;
          memcpy(BAsmCode + 2, AdrVals, AdrCnt);
          CodeLen = 2 + AdrCnt;
        }
      }
      else if (OpSize == 1)
      {
        if (AdrMode != 0) WrError(1350);
        else
        {
          BAsmCode[0] = 0x7c;
          BAsmCode[1] = 0xf3;
          CodeLen = 2;
        }
      }
      else WrError(1130);
    }
  }
}

static void DecodeNOT(Word Code)
{
  Byte SMode;

  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("GS"))
  {
    DecodeAdr(ArgStr[1], MModGen);
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if (OpSize > 1) WrError(1130);
      else
      {
        if (FormatCode == 0)
        {
          if ((OpSize == 0) && (IsShort(AdrMode, &SMode)))
            FormatCode = 2;
          else
            FormatCode = 1;
        }
        switch (FormatCode)
        {
          case 1:
            BAsmCode[0] = 0x74 + OpSize;
            BAsmCode[1] = 0x70 + AdrMode;
            memcpy(BAsmCode + 2, AdrVals, AdrCnt);
            CodeLen = 2 + AdrCnt;
            break;
          case 2:
            if (OpSize != 0) WrError(1130);
            else if (!IsShort(AdrMode, &SMode)) WrError(1350);
            else
            {
              BAsmCode[0] = 0xb8 + SMode;
              memcpy(BAsmCode + 1, AdrVals, AdrCnt);
              CodeLen = 1 + AdrCnt;
            }
            break;
        }
      }
    }
  }
}

static void DecodeAND_OR(Word IsOR)
{
  Byte SMode;

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("GS"))        /* RMS 01: The format codes are G and S, not G and Q */
  {
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      CopyAdr();
      DecodeAdr(ArgStr[1], MModGen | MModImm);
      if (AdrType != ModNone)
      {
        if (OpSize == -1) WrError(1132);
        else if (OpSize > 1) WrError(1130);
        else
        {
          if (FormatCode == 0)
          {
            if (AdrType == ModImm)
            {
              if ((OpSize == 0) && (IsShort(AdrMode2, &SMode)))
                FormatCode = 2;
              else
                FormatCode = 1;
            }
            else
            {
              if ((AdrMode2 <= 1) && (IsShort(AdrMode, &SMode)) && ((AdrMode > 1) || Odd(AdrMode ^ AdrMode2)))
                FormatCode = 2;
              else
                FormatCode = 1;
            }
          }
          switch (FormatCode)
          {
            case 1:
              CodeGen(0x90 + (IsOR << 3), 0x76, 0x20 + (IsOR << 4));
              break;
            case 2:
              if (OpSize != 0) WrError(1130);
              else if (AdrType == ModImm)
              if (!IsShort(AdrMode2, &SMode)) WrError(1350);
              else
              {
                BAsmCode[0] = 0x90 + (IsOR << 3) + SMode;
                BAsmCode[1] = ImmVal();
                memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
                CodeLen = 2 + AdrCnt2;
              }
              else if ((!IsShort(AdrMode, &SMode)) || (AdrMode2 >1 )) WrError(1350);
              else if ((AdrMode <= 1) && (!Odd(AdrMode ^ AdrMode2))) WrError(1350);
              else
              {
                if (SMode == 3) SMode++;
                BAsmCode[0] = 0x10+(IsOR << 3) + ((AdrMode2 & 1) << 2) + (SMode & 3);
                memcpy(BAsmCode + 1, AdrVals, AdrCnt);
                CodeLen = 1 + AdrCnt;
              }
              break;
          }
        }
      }
    }
  }
}

static void DecodeROT(Word Code)
{
  ShortInt OpSize2;

  UNUSED(Code);

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if (OpSize > 1) WrError(1130);
      else
      {
        OpSize2 = OpSize;
        OpSize = 0;
        CopyAdr();
        DecodeAdr(ArgStr[1], MModGen | MModImm);
        if (AdrType == ModGen)
        {
          if (AdrMode != 3) WrError(1350);
          else if (AdrMode2 + 2 * OpSize2 == 3) WrError(1350);
          else
          {
            BAsmCode[0] = 0x74 + OpSize2;
            BAsmCode[1] = 0x60 + AdrMode2;
            memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
            CodeLen = 2 + AdrCnt2;
          }
        }
        else if (AdrType==ModImm)
        {
          Integer Num1 = ImmVal();
          if (Num1 == 0) WrError(1315);
          else if (ChkRange(Num1, -8, 8))
          {
            Num1 = (Num1 > 0) ? (Num1 - 1) : -9 -Num1;
            BAsmCode[0] = 0xe0 + OpSize2;
            BAsmCode[1] = (Num1 << 4) + AdrMode2;
            memcpy(BAsmCode + 2, AdrVals2, AdrCnt);
            CodeLen = 2 + AdrCnt2;
          }
        }
      }
    }
  }
}

static void DecodeSHA_SHL(Word IsSHA)
{
  ShortInt OpSize2;

  if (ArgCnt != 2) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[2], MModGen | MModReg32);
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if ((OpSize > 2) || ((OpSize == 2) && (AdrType == ModGen))) WrError(1130);
      else
      {
        CopyAdr(); OpSize2 = OpSize; OpSize = 0;
        DecodeAdr(ArgStr[1], MModImm | MModGen);
        if (AdrType == ModGen)
        {
          if (AdrMode != 3) WrError(1350);
          else if (AdrMode2 + 2 * OpSize2 == 3) WrError(1350);
          else
          {
            if (OpSize2 == 2)
            {
              BAsmCode[0] = 0xeb;
              BAsmCode[1] = 0x01 | (AdrMode2 << 4) | (IsSHA << 5);
            }
            else
            {
              BAsmCode[0] = 0x74 | OpSize2;
              BAsmCode[1] = 0xe0 | (IsSHA << 4) | AdrMode2;
            }
            memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
            CodeLen = 2 + AdrCnt2;
          }
        }
        else if (AdrType == ModImm)
        {
          Integer Num1 = ImmVal();
          if (Num1 == 0) WrError(1315);
          else if (ChkRange(Num1, -8, 8))
          {
            if (Num1 > 0) Num1--; else Num1 = (-9) - Num1;
            if (OpSize2 == 2)
            {
              BAsmCode[0] = 0xeb;
              BAsmCode[1] = 0x80 | (AdrMode2 << 4) | (IsSHA << 5) | (Num1 & 15);
            }
            else
            {
              BAsmCode[0] = (0xe8 + (IsSHA << 3)) | OpSize2;
              BAsmCode[1] = (Num1 << 4) | AdrMode2;
            }
            memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
            CodeLen = 2 + AdrCnt2;
          }
        }
      }
    }
  }
}

static void DecodeBit(Word Code)
{
  Boolean MayShort = (Code & 12) == 8;

  if (CheckFormat(MayShort ? "GS" : "G"))
  {
    if (DecodeBitAdr((FormatCode != 1) && (MayShort)))
    {
      if (AdrMode>=16)
      {
        BAsmCode[0] = 0x40 + ((Code - 8) << 3) + (AdrMode & 7);
        BAsmCode[1] = AdrVals[0];
        CodeLen = 2;
      }
      else
      {
        BAsmCode[0] = 0x7e;
        BAsmCode[1] = (Code << 4) + AdrMode;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeFCLR_FSET(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (strlen(ArgStr[1]) != 1) WrError(1350);
  else
  {
    char *p = strchr(Flags, mytoupper(*ArgStr[1]));
    if (!p) WrXError(1440, ArgStr[1]);
    else
    {
      BAsmCode[0] = 0xeb;
      BAsmCode[1] = Code + ((p - Flags) << 4);
      CodeLen = 2;
    }
  }
}

static void DecodeJMP(Word Code)
{
  LongInt AdrLong, Diff;
  Boolean OK;

  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    FirstPassUnknown = False;     /* RMS 12: Mod to allow JMP.S to work */           
    AdrLong = EvalIntExpression(ArgStr[1],UInt20,&OK);
    Diff = AdrLong - EProgCounter();

    /* RMS 12: Repaired JMP.S forward-label as follows:

               If it's an unknown symbol, make PC+1 the "safe" value, otherwise
               the user will get OUT OF RANGE errors on every attempt to use JMP.S
               Since the instruction can only branch forward, and AS stuffs the PC
               back for a "temp" forward reference value, the range-checking will 
               always fail.

               One side-effect also is that for auto-determination purposes, one
               fewer pass is needed.  Before, the first pass would signal JMP.B,
               then once the forward reference is known, it'd signal JMP.S, which
               would cause a "phase error" forcing another pass.
    */

    if ( (FirstPassUnknown) && (Diff == 0) )
      Diff = 1;

    if (OpSize == -1)
    {                                                        /* RMS 13: The other part of the */
      if ((Diff >= 1) && (Diff <= 9))                        /* "looping phase error" fix  */
        OpSize = 4;
      else if ((Diff >= -127) && (Diff <= 128))
        OpSize = 0;
      else if ((Diff >= -32767) && (Diff <= 32768))
        OpSize = 1;
      else
        OpSize = 7;
    }
    /*
       The following code is to deal with a silicon bug in the first generation of
       M16C CPUs (the so-called M16C/60 group).  It has been observed that this 
       silicon bug has been fixed as of the M16C/61, so we disable JMP.S promotion
       to JMP.B when the target crosses a 64k boundary for those CPUs.
       
       Since M16C is a "generic" specification, we do JMP.S promotion for that
       CPU specification, as follows:

         RMS 11: According to Mitsubishi App Note M16C-06-9612 
         JMP.S cannot cross a 64k boundary.. so trim up to JMP.B

       It is admittedly a very low likelihood of occurrence [JMP.S has only 8
       possible targets, being a 3 bit "jump addressing mode"], but since the
       occurrence of this bug could cause such evil debugging issues, I have
       taken the liberty of addressing it in the assembler.  Heck, it's JUST one
       extra byte.  One byte's worth the peace of mind, isn't it? :)
    */
    if ((MomCPU == CPUM16C) || (MomCPU == CPUM30600M8))
    {
      if (OpSize == 4) 
      {
        if ( (AdrLong & 0x0f0000) != (((int)EProgCounter()) & 0x0f0000) )
          OpSize = 0;
      }            /* NOTE! This not an ASX bug, but rather in the CPU!! */
    }
    switch (OpSize)
    {
      case 4:         
        if (((Diff < 1) || (Diff > 9)) && (!SymbolQuestionable) ) WrError(1370); 
        else
        {
          if (Diff == 1)
          {                                   /* RMS 13: To avoid infinite loop phase errors... Turn a */
            BAsmCode[0] = 0x04;               /* JMP (or JMP.S) to following instruction into a NOP */
          }
          else
          {
            BAsmCode[0] = 0x60 + ((Diff - 2) & 7);
          }
          CodeLen = 1;
        }
        break;
      case 0:
        if (((Diff < -127) || (Diff > 128)) && (!SymbolQuestionable)) WrError(1370);
        else
        {
          BAsmCode[0] = 0xfe;
          BAsmCode[1] = (Diff - 1) & 0xff;
          CodeLen = 2;
        }
        break;
      case 1:
        if (((Diff< -32767) || (Diff > 32768)) && (!SymbolQuestionable)) WrError(1370);
        else
        {
          BAsmCode[0] = 0xf4;
          Diff--;
          BAsmCode[1] = Diff & 0xff;
          BAsmCode[2] = (Diff >> 8) & 0xff;
          CodeLen = 3;
        }
        break;
      case 7:
        BAsmCode[0] = 0xfc;
        BAsmCode[1] = AdrLong & 0xff;
        BAsmCode[2] = (AdrLong >> 8) & 0xff;
        BAsmCode[3] = (AdrLong >> 16) & 0xff;
        CodeLen = 4;
        break;
      default:
        WrError(1130);
    }
  }
}

static void DecodeJSR(Word Code)
{
  LongInt AdrLong, Diff;
  Boolean OK;

  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else
  {
    AdrLong = EvalIntExpression(ArgStr[1], UInt20, &OK);
    Diff = AdrLong-EProgCounter();
    if (OpSize == -1)
      OpSize = ((Diff >= -32767) && (Diff <= 32768)) ? 1 : 7;
    switch (OpSize)
    {
      case 1:
        if (((Diff <- 32767) || (Diff > 32768)) && (!SymbolQuestionable)) WrError(1370);
        else
        {
          BAsmCode[0] = 0xf5;
          Diff--;
          BAsmCode[1] = Diff & 0xff;
          BAsmCode[2] = (Diff >> 8) & 0xff;
          CodeLen = 3;
        }
        break;
      case 7:
        BAsmCode[0] = 0xfd;
        BAsmCode[1] = AdrLong & 0xff;
        BAsmCode[2] = (AdrLong >> 8) & 0xff;
        BAsmCode[3] = (AdrLong >> 16) & 0xff;
        CodeLen = 4;
        break;
      default:
        WrError(1130);
    }
  }
}

static void DecodeJMPI_JSRI(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else if (CheckFormat("G"))
  {
    if (OpSize == 7)
      OpSize = 2;
    DecodeAdr(ArgStr[1], MModGen | MModDisp20 | MModReg32| MModAReg32);
    if ((AdrType == ModGen) && ((AdrMode & 14) == 12))
      AdrVals[AdrCnt++] = 0;
    if ((AdrType == ModGen) && ((AdrMode & 14) == 4))
    {
      if (OpSize == -1) OpSize = 1;
      else if (OpSize != 1)
      {
        AdrType = ModNone; WrError(1131);
      }
    }
    if (AdrType == ModAReg32) AdrMode = 4;
    if (AdrType != ModNone)
    {
      if (OpSize == -1) WrError(1132);
      else if ((OpSize != 1) && (OpSize != 2)) WrError(1130);
      else
      {
        BAsmCode[0] = 0x7d;
        BAsmCode[1] = Code + (Ord(OpSize == 1) << 5) + AdrMode;
        memcpy(BAsmCode + 2, AdrVals, AdrCnt);
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeJMPS_JSRS(Word Code)
{
  if (ArgCnt != 1) WrError(1110);
  else
  {
    OpSize = 0;
    FirstPassUnknown = False;
    DecodeAdr(ArgStr[1], MModImm);
    if ((FirstPassUnknown) && (AdrVals[0] < 18))
      AdrVals[0] = 18;
    if (AdrType != ModNone)
    {
      if (AdrVals[0] < 18) WrError(1315);
      else
      {
        BAsmCode[0] = Code;
        BAsmCode[1] = AdrVals[0];
        CodeLen = 2;
      }
    }
  }
}

static void DecodeADJNZ_SBJNZ(Word IsSBJNZ)
{
  if (ArgCnt != 3) WrError(1110);
  else if (CheckFormat("G"))
  {
    DecodeAdr(ArgStr[2], MModGen);
    if (AdrType != ModNone)
    {
      ShortInt OpSize2;

      if (OpSize == -1) WrError(1132);
      else if (OpSize > 1) WrError(1130);
      else
      {
        Integer Num1;

        CopyAdr();
        OpSize2 = OpSize;
        OpSize = 0;
        FirstPassUnknown = False;
        DecodeAdr(ArgStr[1], MModImm);
        Num1 = ImmVal();
        if (FirstPassUnknown)
          Num1 = 0;
        if (IsSBJNZ)
          Num1 = -Num1;
        if (ChkRange(Num1, -8, 7))
        {
          LongInt AdrLong;
          Boolean OK;

          AdrLong = EvalIntExpression(ArgStr[3], UInt20, &OK)-(EProgCounter() + 2);
          if (OK)
          {
            if (((AdrLong < -128) || (AdrLong > 127)) && (!SymbolQuestionable)) WrError(1370);
            else
            {
              BAsmCode[0] = 0xf8 +OpSize2;
              BAsmCode[1] = (Num1 << 4) + AdrMode2;
              memcpy(BAsmCode + 2, AdrVals2, AdrCnt2);
              BAsmCode[2 + AdrCnt2] = AdrLong & 0xff;
              CodeLen = 3 + AdrCnt2;
            }
          }
        }
      }
    }
  }
}

static void DecodeINT(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (*ArgStr[1] != '#') WrError(1350);
  else
  {
    Boolean OK;

    BAsmCode[1] = 0xc0 + EvalIntExpression(ArgStr[1] + 1, UInt6, &OK);
    if (OK)
    {
      BAsmCode[0] = 0xeb;
      CodeLen = 2;
    }
  }
}

static void DecodeBM(Word Code)
{
  if ((ArgCnt == 1) && (!strcasecmp(ArgStr[1], "C")))
  {
    BAsmCode[0] = 0x7d;
    BAsmCode[1] = 0xd0 + Code;
    CodeLen = 2;
  }
  else if (DecodeBitAdr(False))
  {
    BAsmCode[0] = 0x7e;
    BAsmCode[1] = 0x20 + AdrMode;
    memcpy(BAsmCode + 2, AdrVals, AdrCnt);
    if ((Code >= 4) && (Code < 12))
      Code ^= 12;
    if (Code >= 8)
       Code += 0xf0;
    BAsmCode[2 + AdrCnt] = Code;
    CodeLen = 3 + AdrCnt;
  }
}

static void DecodeJ(Word Code)
{
  Integer Num1 = 1 + Ord(Code >= 8);
  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (strcmp(Format, " ")) WrError(1090);
  else
  {
    Boolean OK;
    LongInt AdrLong = EvalIntExpression(ArgStr[1], UInt20, &OK) - (EProgCounter() + Num1);
    if (OK)
    {
      if ((!SymbolQuestionable) && ((AdrLong >127) || (AdrLong < -128))) WrError(1370);
      else if (Code >= 8)
      {
        BAsmCode[0] = 0x7d;
        BAsmCode[1] = 0xc0 + Code;
        BAsmCode[2] = AdrLong & 0xff;
        CodeLen = 3;
      }
      else
      {
        BAsmCode[0] = 0x68 + Code;
        BAsmCode[1] = AdrLong & 0xff;
        CodeLen = 2;
      }
    }
  }
}

static void DecodeENTER(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (*ArgStr[1] != '#') WrError(1350);
  else
  {
    Boolean OK;

    BAsmCode[2] = EvalIntExpression(ArgStr[1] + 1, UInt8, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x7c;
      BAsmCode[1] = 0xf2;
      CodeLen = 3;
    }
  }
}

static void DecodeLDINTB(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (*ArgStr[1] != '#') WrError(1350);
  else
  {
    Boolean OK;
    LongInt AdrLong = EvalIntExpression(ArgStr[1] + 1, UInt20, &OK);
    if (OK)
    {
      BAsmCode[0] = 0xeb;
      BAsmCode[1] = 0x20;
      BAsmCode[2] = (AdrLong >> 16) & 0xff;
      BAsmCode[3] = 0;
      BAsmCode[4] = 0xeb;
      BAsmCode[5] = 0x10;
      BAsmCode[7] = (AdrLong >> 8) & 0xff;
      BAsmCode[6] = AdrLong & 0xff; /* RMS 07: needs to be LSB, MSB order */
      CodeLen = 8;
    }
  }
}

static void DecodeLDIPL(Word Code)
{
  UNUSED(Code);

  if (ArgCnt != 1) WrError(1110);
  else if (*AttrPart != '\0') WrError(1100);
  else if (*ArgStr[1] != '#') WrError(1350);
  else
  {
    Boolean OK;

    BAsmCode[1] = 0xa0 + EvalIntExpression(ArgStr[1] + 1, UInt3, &OK);
    if (OK)
    {
      BAsmCode[0] = 0x7d;
      CodeLen = 2;
    }
  }
}

/*------------------------------------------------------------------------*/
/* code table handling */

static void AddFixed(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddString(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeString);
}

static void AddGen1(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeGen1);
}

static void AddGen2(char *NName, Byte NCode1, Byte NCode2, Byte NCode3)
{
  if (InstrZ >= Gen2OrderCnt) exit(255);
  Gen2Orders[InstrZ].Code1 = NCode1;
  Gen2Orders[InstrZ].Code2 = NCode2;
  Gen2Orders[InstrZ].Code3 = NCode3;
  AddInstTable(InstTable, NName, InstrZ++, DecodeGen2);
}

static void AddDiv(char *NName, Byte NCode1, Byte NCode2, Byte NCode3)
{
  if (InstrZ >= DivOrderCnt) exit(255);
  DivOrders[InstrZ].Code1 = NCode1;
  DivOrders[InstrZ].Code2 = NCode2;
  DivOrders[InstrZ].Code3 = NCode3;
  AddInstTable(InstTable, NName, InstrZ++, DecodeDiv);
}

static void AddCondition(char *BMName, char *JName, Word NCode)
{
  AddInstTable(InstTable, BMName, NCode, DecodeBM);
  AddInstTable(InstTable, JName, NCode, DecodeJ);
}

static void AddBCD(char *NName)
{
  AddInstTable(InstTable, NName, InstrZ++, DecodeBCD);
}

static void AddDir(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeDir);
}

static void AddBit(char *NName, Word NCode)
{
  AddInstTable(InstTable, NName, NCode, DecodeBit);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(403);
  AddInstTable(InstTable, "MOV", 0, DecodeMOV);
  AddInstTable(InstTable, "LDC", 0, DecodeLDC_STC);
  AddInstTable(InstTable, "STC", 1, DecodeLDC_STC);
  AddInstTable(InstTable, "LDCTX", 0x7c, DecodeLDCTX_STCTX);
  AddInstTable(InstTable, "STCTX", 0x7d, DecodeLDCTX_STCTX);
  AddInstTable(InstTable, "LDE", 1, DecodeLDE_STE);
  AddInstTable(InstTable, "STE", 0, DecodeLDE_STE);
  AddInstTable(InstTable, "MOVA", 0, DecodeMOVA);
  AddInstTable(InstTable, "POP", 1, DecodePUSH_POP);
  AddInstTable(InstTable, "PUSH", 0, DecodePUSH_POP);
  AddInstTable(InstTable, "POPC", 0x03, DecodePUSHC_POPC);
  AddInstTable(InstTable, "PUSHC", 0x02, DecodePUSHC_POPC);
  AddInstTable(InstTable, "POPM", 1, DecodePUSHM_POPM);
  AddInstTable(InstTable, "PUSHM", 0, DecodePUSHM_POPM);
  AddInstTable(InstTable, "PUSHA", 0, DecodePUSHA);
  AddInstTable(InstTable, "XCHG", 0, DecodeXCHG);
  AddInstTable(InstTable, "STZ", 0xc8, DecodeSTZ_STNZ);
  AddInstTable(InstTable, "STNZ", 0xd0, DecodeSTZ_STNZ);
  AddInstTable(InstTable, "STZX", 0, DecodeSTZX);
  AddInstTable(InstTable, "ADD", 0, DecodeADD);
  AddInstTable(InstTable, "CMP", 0, DecodeCMP);
  AddInstTable(InstTable, "SUB", 0, DecodeSUB);
  AddInstTable(InstTable, "INC", 0, DecodeINC_DEC);
  AddInstTable(InstTable, "DEC", 1, DecodeINC_DEC);
  AddInstTable(InstTable, "EXTS", 0, DecodeEXTS);
  AddInstTable(InstTable, "NOT", 0, DecodeNOT);
  AddInstTable(InstTable, "AND", 0, DecodeAND_OR);
  AddInstTable(InstTable, "OR", 1, DecodeAND_OR);
  AddInstTable(InstTable, "ROT", 0, DecodeROT);
  AddInstTable(InstTable, "SHA", 1, DecodeSHA_SHL);
  AddInstTable(InstTable, "SHL", 0, DecodeSHA_SHL);
  AddInstTable(InstTable, "FCLR", 0x05, DecodeFCLR_FSET);
  AddInstTable(InstTable, "FSET", 0x04, DecodeFCLR_FSET);
  AddInstTable(InstTable, "JMP", 0, DecodeJMP);
  AddInstTable(InstTable, "JSR", 0, DecodeJSR);
  AddInstTable(InstTable, "JMPI", 0x00, DecodeJMPI_JSRI);
  AddInstTable(InstTable, "JSRI", 0x10, DecodeJMPI_JSRI);
  AddInstTable(InstTable, "JMPS", 0xee, DecodeJMPS_JSRS);
  AddInstTable(InstTable, "JSRS", 0xef, DecodeJMPS_JSRS);
  AddInstTable(InstTable, "ADJNZ", 0, DecodeADJNZ_SBJNZ);
  AddInstTable(InstTable, "SBJNZ", 1, DecodeADJNZ_SBJNZ);
  AddInstTable(InstTable, "INT", 0, DecodeINT);
  AddInstTable(InstTable, "ENTER", 0, DecodeENTER);
  AddInstTable(InstTable, "LDINTB", 0, DecodeLDINTB);
  AddInstTable(InstTable, "LDIPL", 0, DecodeLDIPL);

  Format = (char*)malloc(sizeof(Char) * STRINGSIZE);

  AddFixed("BRK"   , 0x0000);
  AddFixed("EXITD" , 0x7df2);
  AddFixed("INTO"  , 0x00f6);
  AddFixed("NOP"   , 0x0004);
  AddFixed("REIT"  , 0x00fb);
  AddFixed("RTS"   , 0x00f3);
  AddFixed("UND"   , 0x00ff);
  AddFixed("WAIT"  , 0x7df3);

  AddString("RMPA" , 0x7cf1);
  AddString("SMOVB", 0x7ce9);
  AddString("SMOVF", 0x7ce8);
  AddString("SSTR" , 0x7cea);

  AddGen1("ABS" , 0x76f0);
  AddGen1("ADCF", 0x76e0);
  AddGen1("NEG" , 0x7450);
  AddGen1("ROLC", 0x76a0);
  AddGen1("RORC", 0x76b0);

  InstrZ = 0; Gen2Orders = (Gen2Order *) malloc(sizeof(Gen2Order) * Gen2OrderCnt);
  AddGen2("ADC" , 0xb0, 0x76, 0x60);
  AddGen2("SBB" , 0xb8, 0x76, 0x70);
  AddGen2("TST" , 0x80, 0x76, 0x00);
  AddGen2("XOR" , 0x88, 0x76, 0x10);
  AddGen2("MUL" , 0x78, 0x7c, 0x50);
  AddGen2("MULU", 0x70, 0x7c, 0x40);

  InstrZ=0; DivOrders=(Gen2Order *) malloc(sizeof(Gen2Order)*DivOrderCnt);
  AddDiv("DIV" ,0xe1,0x76,0xd0);
  AddDiv("DIVU",0xe0,0x76,0xc0);
  AddDiv("DIVX",0xe3,0x76,0x90);

  AddCondition("BMGEU", "JGEU",  0); AddCondition("BMC"  , "JC"  ,  0);
  AddCondition("BMGTU", "JGTU",  1); AddCondition("BMEQ" , "JEQ" ,  2);
  AddCondition("BMZ"  , "JZ"  ,  2); AddCondition("BMN"  , "JN"  ,  3);
  AddCondition("BMLTU", "JLTU",  4); AddCondition("BMNC" , "JNC" ,  4);
  AddCondition("BMLEU", "JLEU",  5); AddCondition("BMNE" , "JNE" ,  6);
  AddCondition("BMNZ" , "JNZ" ,  6); AddCondition("BMPZ" , "JPZ" ,  7);
  AddCondition("BMLE" , "JLE" ,  8); AddCondition("BMO"  , "JO"  ,  9);
  AddCondition("BMGE" , "JGE" , 10); AddCondition("BMGT" , "JGT" , 12);
  AddCondition("BMNO" , "JNO" , 13); AddCondition("BMLT" , "JLT" , 14);

  InstrZ = 0;
  AddBCD("DADD"); AddBCD("DSUB"); AddBCD("DADC"); AddBCD("DSBB");

  InstrZ = 0;
  AddDir("MOVLL", InstrZ++);
  AddDir("MOVHL", InstrZ++);
  AddDir("MOVLH", InstrZ++);
  AddDir("MOVHH", InstrZ++);

  AddBit("BAND"  , 4); AddBit("BNAND" , 5);
  AddBit("BNOR"  , 7); AddBit("BNTST" , 3);
  AddBit("BNXOR" ,13); AddBit("BOR"   , 6);
  AddBit("BTSTC" , 0); AddBit("BTSTS" , 1);
  AddBit("BXOR"  ,12); AddBit("BCLR"  , 8);
  AddBit("BNOT"  ,10); AddBit("BSET"  , 9);
  AddBit("BTST"  ,11);
}

static void DeinitFields(void)
{
  free(Format);
  free(Gen2Orders);
  free(DivOrders);

  DestroyInstTable(InstTable);
}

/*------------------------------------------------------------------------*/

static void MakeCode_M16C(void)
{
  char *p;

  OpSize = -1;

  /* zu ignorierendes */

  if (Memo("")) return;

  /* Formatangabe abspalten */

  switch (AttrSplit)
  {
    case '.':
      p = strchr(AttrPart, ':');
      if (p)
      {
        if (p < AttrPart + strlen(AttrPart) - 1)
          strmaxcpy(Format, p + 1, STRINGSIZE - 1);
        *p = '\0';
      }
      else
        strcpy(Format,  " ");
      break;
    case ':':
      p = strchr(AttrPart, '.');
      if (!p)
      {
        strmaxcpy(Format, AttrPart, STRINGSIZE - 1);
        *AttrPart = '\0';
      }
      else
      {
        *p = '\0';
        strmaxcpy(Format, (p == AttrPart) ? " " : AttrPart, STRINGSIZE - 1);
      }
      break;
    default:
      strcpy(Format, " ");
  }

  /* Attribut abarbeiten */

  switch (mytoupper(*AttrPart))
  {
    case '\0': OpSize = -1; break;
    case 'B': OpSize = 0; break;
    case 'W': OpSize = 1; break;
    case 'L': OpSize = 2; break;
    case 'Q': OpSize = 3; break;
    case 'S': OpSize = 4; break;
    case 'D': OpSize = 5; break;
    case 'X': OpSize = 6; break;
    case 'A': OpSize = 7; break;
    default: 
      WrError(1107); return;
  }
  NLS_UpString(Format);

  /* Pseudoanweisungen */

  if (DecodeIntelPseudo(False)) return;

  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200,OpPart);
}

static Boolean IsDef_M16C(void)
{
  return False;
}

static void SwitchFrom_M16C(void)
{
  DeinitFields();
}

static void SwitchTo_M16C(void)
{
  TurnWords = True;
  ConstMode = ConstModeIntel;
  SetIsOccupied = False;

  PCSymbol = "$";
  HeaderID = 0x14;
  NOPCode = 0x04;
  DivideChars = ",";
  HasAttrs = True;
  AttrChars = ".:";

  ValidSegs = 1 << SegCode;
  Grans[SegCode] = 1;
  ListGrans[SegCode] = 1;
  SegInits[SegCode] = 0;
  SegLimits[SegCode] = 0xfffff;        /* RMS 10: Probably a typo (was 0xffff) */

  MakeCode = MakeCode_M16C;
  IsDef = IsDef_M16C;
  SwitchFrom = SwitchFrom_M16C;
  InitFields();
}

void codem16c_init(void)
{
  CPUM16C = AddCPU("M16C", SwitchTo_M16C);
  CPUM30600M8 = AddCPU("M30600M8", SwitchTo_M16C);
  CPUM30610 = AddCPU("M30610", SwitchTo_M16C);
  CPUM30620 = AddCPU("M30620", SwitchTo_M16C);

  AddCopyright("Mitsubishi M16C-Generator also (C) 1999 RMS");
}
