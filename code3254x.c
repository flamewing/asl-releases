/* code3254x.c */
/*****************************************************************************/
/* Macro Assembler AS                                                        */
/*                                                                           */
/* code generator for TI C54x DSP devices                                    */
/*                                                                           */
/* history:  2001-07-07: begun                                               */
/*           2001-07-30: added simple accumulator instructions               */
/*           2001-07-31: added address decoder                               */
/*           2001-08-03: ADD SUB                                             */
/*           2001-08-05: MemAccOrders                                        */
/*           2001-08-18: MemConstOrders                                      */
/*           2001-08-19: multiply orders begun                               */
/*           2001-08-27: MPYA SQUR                                           */
/*           2001-08-30: MACx begun                                          */
/*           2001-08-31: MACD MACP MACSU MAS MASR MASAR                      */
/*           2001-09-16: DADD                                                */
/*           2001-09-29: AND OR XOR SFTA SFTL                                */
/*           2001-09-30: FIRS BIT BITF                                       */
/*           2001-10-03: CMPR                                                */
/*                                                                           */
/*****************************************************************************/

/*-------------------------------------------------------------------------*/
/* Includes */

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
#include "codevars.h"
#include "asmitree.h"
#include "fileformat.h"
#include "headids.h"

/*-------------------------------------------------------------------------*/
/* Data Structures */

#define FixedOrderCnt 12
#define AccOrderCnt 16
#define Acc2OrderCnt 5
#define MemOrderCnt 9
#define XYOrderCnt 3
#define MemAccOrderCnt 16
#define MemConstOrderCnt 5
#define MacOrderCnt 3

typedef struct
        {
          Word Code;
          Boolean IsRepeatable;
        } FixedOrder;

typedef struct
        {
          Word Code;
          Boolean IsRepeatable, Swap;
          IntType ConstType;
        } MemConstOrder;

typedef enum {ModNone = - 1, ModAcc, ModMem, ModImm, ModAReg} ModType;
#define MModAcc  (1 << ModAcc)
#define MModMem  (1 << ModMem)
#define MModImm  (1 << ModImm)
#define MModAReg (1 << ModAReg)

static LongInt Reg_CPL, Reg_DP, Reg_SP;

static Boolean ThisRep, LastRep;

static PInstTable InstTable;
static FixedOrder *FixedOrders, *AccOrders, *Acc2Orders, *MemOrders, *XYOrders,
                  *MemAccOrders, *MacOrders;
static MemConstOrder *MemConstOrders;

static SimpProc SaveInitProc;

static CPUVar CPU320C541;

static IntType OpSize;
static ShortInt AdrMode;
static Word AdrVals[3];
static int AdrCnt;

/*-------------------------------------------------------------------------*/
/* Address Decoder */

static char ShortConds[4][4] = {"EQ", "LT", "GT", "NEQ"};

static Boolean IsAcc(char *Asc)
{
  return ((Asc[1] == '\0') && (toupper(*Asc) >= 'A') && (toupper(*Asc) <= 'B'));
}

static Boolean DecodeAdr(char *Asc, int Mask)
{
#define IndirCnt 16
  static char *Patterns[IndirCnt] = /* leading star is omitted since constant */
              { "ARx",      "ARx-",     "ARx+",      "+ARx",
                "ARx-0B",   "ARx-0",    "ARx+0",     "ARx+0B",
                "ARx-%",    "ARx-0%",   "ARx+%",     "ARx+0%",
                "ARx(n)",   "+ARx(n)",  "+ARx(n)%",  "(n)"};
  Boolean OK;

  AdrMode = ModNone; AdrCnt = 0;

  /* accumulators */

  if (IsAcc(Asc))
  {
    AdrMode = ModAcc; *AdrVals = toupper(*Asc) - 'A';
    goto done;
  }

  /* aux registers */

  if ((strlen(Asc) == 3) && (!strncasecmp(Asc, "AR", 2)) && (Asc[2] >= '0') && (Asc[2] <= '7'))
  {
    AdrMode = ModAReg; *AdrVals = Asc[2] - '0';
    goto done;
  }

  /* immediate */

  if (*Asc == '#')
  {
    Boolean OK;

    *AdrVals = EvalIntExpression(Asc + 1, OpSize, &OK);
    if (OK)
      AdrMode = ModImm;
    goto done;
  }

  /* indirect */

  if (*Asc == '*')
  {
    int z;
    Word RegNum;
    char *pConstStart, *pConstEnd;

    /* check all possible patterns */

    for (z = 0; z < IndirCnt; z++)
    {
      char *pPattern = Patterns[z], *pComp = Asc + 1;

      /* pattern comparison */

      RegNum = 0; pConstStart = pConstEnd = NULL; OK = TRUE;
      while ((*pPattern) && (*pComp) && (OK))
      {
        switch (*pPattern)
        {
          case 'x': /* embedded number */
            RegNum = *pComp - '0';
            OK = RegNum < 8;
            break;
          case 'n': /* constant */
            pConstStart = pComp;
            pConstEnd = QuotPos(pComp, pPattern[1]);
            if (pConstEnd)
              pComp = pConstEnd - 1;
            else
              OK = False;
            break;
          default:  /* compare verbatim */
            if (toupper(*pPattern) != toupper(*pComp))
              OK = False;
        }
        if (OK)
        {
          pPattern++; pComp++;
        }
      }

      /* for a successful comparison, we must have reached the end of both strings
         simultaneously. */

      OK = OK && (!*pPattern) && (!*pComp);
      if (OK)
        break;
    }

    if (!OK) WrError(1350);
    else
    {
      /* decode offset ? pConst... /must/ be set if such a pattern was successfully
         decoded! */

      if (strchr(Patterns[z], 'n'))
      {
        char Save = *pConstEnd;
        *pConstEnd = '\0';
        AdrVals[1] = EvalIntExpression(pConstStart, Int16, &OK);
        *pConstEnd = Save;
        if (OK)
          AdrCnt = 1;
      }

      /* all fine until now? Then do the rest... */

      if (OK)
      {
        AdrMode = ModMem; AdrVals[0] = 0x80 | (z << 3) | RegNum;
      }
    }

    goto done;
  }

  /* then try absolute */

  FirstPassUnknown = FALSE;
  *AdrVals = EvalIntExpression(Asc, UInt16, &OK);
  if (OK)
  {
    if (Reg_CPL) /* short address rel. to SP? */
    {
      *AdrVals -= Reg_SP;
      if ((NOT FirstPassUnknown) && (*AdrVals > 127))
        WrError(110);
    }
    else         /* on DP page ? */
    {
      if ((NOT FirstPassUnknown) && ((*AdrVals >> 7) != Reg_DP))
        WrError(110);
    }
    AdrVals[0] &= 127;
    AdrMode = ModMem;
  }

done:
  if (!(Mask & (1 << AdrMode)))
  {
    AdrMode = ModNone; AdrCnt = 0;
    WrError(1350);
  }
  return (AdrMode != ModNone);
}

static Boolean MakeXY(Word *Dest, Boolean Quarrel)
{
  Boolean Result = False;

  if (AdrMode != ModMem);  /* should never occur, if address mask specified correctly before */
  else
  {
    Word Mode = (*AdrVals >> 3) & 15, Reg = *AdrVals & 7;

    if ((Reg < 2) || (Reg > 5));
    else if ((Mode != 0) && (Mode != 1) && (Mode != 2) && (Mode != 11));
    else
    {
      *Dest = (Reg - 2) | ((Mode & 3) << 2);
      Result = True;
    }
  }

  if ((Quarrel) && (!Result))
    WrError(1350);

  return Result;
}

/*-------------------------------------------------------------------------*/
/* Decoders */

static void DecodeFixed(Word Index)
{
  FixedOrder *POrder = FixedOrders + Index;

  if (ArgCnt != 0) WrError(1110);
  else if ((LastRep) && (!POrder->IsRepeatable)) WrError(1560);
  else
  {
    WAsmCode[0] = POrder->Code; CodeLen = 1;
  }
}

static void DecodeAcc(Word Index)
{
  FixedOrder *POrder = AccOrders + Index;

  if (ArgCnt != 1) WrError(1110);
  else if ((LastRep) && (!POrder->IsRepeatable)) WrError(1560);
  else
  {
    if (DecodeAdr(ArgStr[1], MModAcc))
    {
      WAsmCode[0] = POrder->Code | (AdrVals[0] << 8); CodeLen = 1;
    }
  }
}

static void DecodeAcc2(Word Index)
{
  FixedOrder *POrder = Acc2Orders + Index;
  Boolean OK;

  if ((ArgCnt < 1) || (ArgCnt > 2)) WrError(1110);
  else if ((LastRep) && (!POrder->IsRepeatable)) WrError(1560);
  else
  {
    if ((OK = DecodeAdr(ArgStr[1], MModAcc)))
    {
      WAsmCode[0] = POrder->Code | (AdrVals[0] << 9);
      if (ArgCnt == 2)
        OK = DecodeAdr(ArgStr[2], MModAcc);
      if (OK)
      {
        WAsmCode[0] |= (AdrVals[0] << 8);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeMem(Word Index)
{
  FixedOrder *POrder = MemOrders + Index;

  if (ArgCnt < 1) WrError(1110);
  else if ((LastRep) && (!POrder->IsRepeatable)) WrError(1560);
  else if (DecodeAdr(ArgStr[1], MModMem))
  {
    memcpy(WAsmCode, AdrVals, (AdrCnt + 1) << 1);
    WAsmCode[0] |= POrder->Code;
    CodeLen = 1 + AdrCnt;
  }
}

static void DecodeXY(Word Index)
{
  FixedOrder *POrder = XYOrders + Index;
  Word TmpX, TmpY;

  if (ArgCnt != 2) WrError(1350);
  else if ((LastRep) && (!POrder->IsRepeatable)) WrError(1560);
  else
  {
    if (DecodeAdr(ArgStr[1], MModMem))
      if (MakeXY(&TmpX, True))
        if (DecodeAdr(ArgStr[2], MModMem))
          if (MakeXY(&TmpY, True))
          {
            WAsmCode[0] = POrder->Code | (TmpX << 4) | TmpY;
            CodeLen = 1;
          }
  }
}

static void DecodeADDSUB(Word Index)
{
  Boolean OK;
  Integer Shift;
  Word DestAcc;

  if ((ArgCnt < 2) || (ArgCnt > 4)) WrError(1110);
  else
  {
    OpSize = SInt16;
    DecodeAdr(ArgStr[1], MModAcc | MModMem | MModImm);
    switch (AdrMode)
    {
      case ModAcc:  /* ADD src, SHIFT|ASM [,dst] */
        if (ArgCnt == 4) WrError(1110);
        else
        {
          Word SrcAcc = *AdrVals;

          /* SrcAcc remains in AdrVals[0] if no 3rd operand, therefore
             no extra assignment needed! */

          if (ArgCnt == 3)
          {
            if (!DecodeAdr(ArgStr[3], MModAcc))
              break;
          }

          /* distinguish variants of shift specification: */

          if (!strcasecmp(ArgStr[2], "ASM"))
          {
            WAsmCode[0] = 0xf480 | Index | (SrcAcc << 9) | (*AdrVals << 8);
            CodeLen = 1;
          }
          else
          {
            WAsmCode[0] = EvalIntExpression(ArgStr[2], SInt5, &OK);
            if (OK)
            {
              WAsmCode[0] = (WAsmCode[0] & 0x1f) | 0xf400 | (Index << 5) | (SrcAcc << 9) | (*AdrVals << 8);
              CodeLen = 1;
            }
          }
        }
        break;

      case ModMem: /* ADD mem[, TS | SHIFT | Ymem], src[, dst] */
      {
        int HCnt;

        /* rescue decoded address values */

        memcpy(WAsmCode, AdrVals, (AdrCnt + 1) << 1);
        HCnt = AdrCnt;

        /* no shift? this is the case for two operands or three operands and the second is an accumulator */

        if (ArgCnt == 2)
          Shift = 0;
        else if ((ArgCnt == 3) && (IsAcc(ArgStr[2])))
          Shift = 0;

        /* special shift value ? */

        else if (!strcasecmp(ArgStr[2], "TS"))
          Shift = 255;

        /* shift address operand ? */

        else if (*ArgStr[2] == '*')
        {
          /* break down source operand to reduced variant */

          if (!MakeXY(WAsmCode, True))
            break;
          WAsmCode[0] = WAsmCode[0] << 4;

          /* merge in second operand */

          if (!DecodeAdr(ArgStr[2], MModMem))
            break;
          if (!MakeXY(&Shift, True))
            break;
          WAsmCode[0] |= Shift;
          Shift = 254;
        }

        /* normal immediate shift */

        else
        {
          Shift = EvalIntExpression(ArgStr[2], SInt6, &OK);
          if (!OK)
            break;
          if ((FirstPassUnknown) && (Shift > 16))
            Shift &= 15;
          if (!ChkRange(Shift, -16 ,16))
            break;
        }

        /* decode destination accumulator */

        if (!DecodeAdr(ArgStr[ArgCnt], MModAcc))
          break;
        DestAcc = *AdrVals;

        /* optionally decode source accumulator.  If no second accumulator, result
           again remains in AdrVals */

        if ((ArgCnt == 4) || ((ArgCnt == 3) && (IsAcc(ArgStr[2]))))
        {
          if (!DecodeAdr(ArgStr[ArgCnt - 1], MModAcc))
            break;
        }

	/* now start applying the variants */        

        if (Shift == 255) /* TS case */
        {
          if (*AdrVals != DestAcc) WrError(1350);
          else
          {
            WAsmCode[0] |= 0x0400 | (Index << 11) | (DestAcc << 8);
            CodeLen = 1 + HCnt;
          }
        }

        else if (Shift == 254) /* XY case */
        {
          if (*AdrVals != DestAcc) WrError(1350);
          else
          {   
            WAsmCode[0] |= 0xa000 | (Index << 9) | (DestAcc << 8);
            CodeLen = 1;
          }
        }

        else if (Shift == 16) /* optimization for 16 shifts */
        {
          WAsmCode[0] |= (0x3c00 + (Index << 10)) | (*AdrVals << 9) | (DestAcc << 8);
          CodeLen = 1 + HCnt;
        }

        else if ((DestAcc == *AdrVals) && (Shift == 0)) /* shortform without shift and with one accu only */
        {
          WAsmCode[0] |= 0x0000 | (Index << 11) | (DestAcc << 8);
          CodeLen = 1 + HCnt;
        }

        else 
        {
          Word SrcAcc = *AdrVals;

          /* fool MakeXY a bit */

          AdrMode = ModMem; AdrVals[0] = WAsmCode[0];

          if ((Shift >= 0) && (DestAcc == SrcAcc) && (MakeXY(WAsmCode, False))) /* X-Addr and positive shift */
          {
            WAsmCode[0] = 0x9000 | (Index << 9) | (WAsmCode[0] << 4) | (DestAcc << 8) | Shift;
            CodeLen = 1;
          }
          else /* last resort... */
          {
            WAsmCode[0] |= 0x6f00;
            WAsmCode[2] = WAsmCode[1]; /* shift optional address offset */
            WAsmCode[1] = 0x0c00 | (Index << 5) | (SrcAcc << 9) | (DestAcc << 8) | (Shift & 0x1f);
            CodeLen = 2 + HCnt;
          }
        }

        break;
      }

      case ModImm:  /* ADD #lk[, SHIFT|16], src[, dst] */
      {
        /* store away constant */

        WAsmCode[1] = *AdrVals;

        /* no shift? this is the case for two operands or three operands and the second is an accumulator */

        if (ArgCnt == 2)
          Shift = 0;
        else if ((ArgCnt == 3) && (IsAcc(ArgStr[2])))
          Shift = 0;

        /* otherwise shift is second argument */

        else
        {
          FirstPassUnknown = False;
          Shift = EvalIntExpression(ArgStr[2], UInt5, &OK);
          if (!OK)
            break;
          if ((FirstPassUnknown) && (Shift > 16))
            Shift &= 15;
          if (!ChkRange(Shift, 0 ,16))
            break;
        }

        /* decode destination accumulator */

        if (!DecodeAdr(ArgStr[ArgCnt], MModAcc))
          break;
        DestAcc = *AdrVals;

        /* optionally decode source accumulator.  If no second accumulator, result
           again remains in AdrVals */

        if ((ArgCnt == 4) || ((ArgCnt == 3) && (IsAcc(ArgStr[2]))))
        {
          if (!DecodeAdr(ArgStr[ArgCnt - 1], MModAcc))
            break;
        }

        /* distinguish according to shift count */

        if (Shift == 16)
        {
          WAsmCode[0] = 0xf060 | Index | (DestAcc << 8) | (*AdrVals << 9);
          CodeLen = 2;
        }
        else
        {
          WAsmCode[0] = 0xf000 | (Index << 5) | (DestAcc << 8) | (*AdrVals << 9) | (Shift & 15);
          CodeLen = 2;
        }

        break;
      }
    }
  }
}

static void DecodeMemAcc(Word Index)
{
  FixedOrder *POrder = MemAccOrders + Index;

  if (ArgCnt != 2) WrError(1110);
  else if ((LastRep) && (!POrder->IsRepeatable)) WrError(1560);
  else if (DecodeAdr(ArgStr[2], MModAcc))
  {
    WAsmCode[0] = POrder->Code | (AdrVals[0] << 8);
    if (DecodeAdr(ArgStr[1], MModMem))
    {
      WAsmCode[0] |= *AdrVals;
      if (AdrCnt)
        WAsmCode[1] = AdrVals[1];
      CodeLen = 1 + AdrCnt;
    }
  }
}

static void DecodeMemConst(Word Index)
{
  MemConstOrder *POrder = MemConstOrders + Index;
  int HCnt;

  if (ArgCnt != 2) WrError(1110);
  else if ((LastRep) && (!POrder->IsRepeatable)) WrError(1560);
  else if (DecodeAdr(ArgStr[2 - POrder->Swap], MModMem))
  {
    WAsmCode[0] = POrder->Code | 0[AdrVals];
    if ((HCnt = AdrCnt))
      WAsmCode[1] = AdrVals[1];
    OpSize = POrder->ConstType;
    if (DecodeAdr(ArgStr[1 +  POrder->Swap], MModImm))
    {
      WAsmCode[1 + HCnt] = *AdrVals;
      CodeLen = 2 + HCnt;
    }
  }
}

static void DecodeMPY(Word Index)
{
  Word DestAcc;

  (void)Index;

  if ((ArgCnt != 2) && (ArgCnt != 3)) WrError(1110);
  else if (DecodeAdr(ArgStr[ArgCnt], MModAcc))
  {
    DestAcc = (*AdrVals) << 8;
    if (ArgCnt == 3)
    {
      Word XMode;

      if (DecodeAdr(ArgStr[1], MModMem))
        if (MakeXY(&XMode, True))
          if (DecodeAdr(ArgStr[2], MModMem))
            if (MakeXY(WAsmCode, True))
            {
              *WAsmCode |= 0xa400 | DestAcc | (XMode << 4);
              CodeLen = 1;
            }
    }
    else
    {
      OpSize = SInt16;
      DecodeAdr(ArgStr[1], MModImm | MModMem);
      switch (AdrMode)
      {
        case ModImm:
          WAsmCode[0] = 0xf066 | DestAcc;
          WAsmCode[1] = *AdrVals;
          CodeLen = 2;
          break;
        case ModMem:
          WAsmCode[0] = 0x2000 | DestAcc | 0[AdrVals];
          if (AdrCnt)
            WAsmCode[1] = AdrVals[1];
          CodeLen = 1 + AdrCnt;
          break;
      }
    }
  }
}

static void DecodeMPYA(Word Index)
{
  (void) Index;

  if (ArgCnt != 1) WrError(1110);
  else
  {
    DecodeAdr(ArgStr[1], MModAcc | MModMem);
    switch (AdrMode)
    {
      case ModMem:
        WAsmCode[0] = 0x3100 | AdrVals[0];
        if (AdrCnt)
          WAsmCode[1] = AdrVals[1];
        CodeLen = 1 + AdrCnt;
        break;
      case ModAcc:
        WAsmCode[0] = 0xf48c | (*AdrVals << 8);
        CodeLen = 1;
        break;
    }
  }
}

static void DecodeSQUR(Word Index)
{
  (void)Index;

  if (ArgCnt != 2) WrError(1110);
  else if (DecodeAdr(ArgStr[2], MModAcc))
  {
    0[WAsmCode] = *AdrVals << 8;
    DecodeAdr(ArgStr[1], MModAcc | MModMem);
    switch (AdrMode)
    {
      case ModAcc:
        if (*AdrVals) WrError(1350);
        else
        {
          WAsmCode[0] |= 0xf48d;
          CodeLen = 1;
        }
        break;
      case ModMem:
        WAsmCode[0] |= 0x2600 | *AdrVals;
        if (AdrCnt)
          WAsmCode[1] = AdrVals[1];
        CodeLen = 1 + AdrCnt;
        break;
    }
  }
}

static void DecodeMAC(Word Index)
{
  (void) Index;

  if ((ArgCnt < 2) || (ArgCnt > 4)) WrError(1110);
  else if (DecodeAdr(ArgStr[ArgCnt], MModAcc))
  {
    *WAsmCode = (*AdrVals) << 8;
    OpSize = SInt16;
    DecodeAdr(ArgStr[1], MModImm | MModMem);

    /* handle syntax 3: immediate op first */

    if (AdrMode == ModImm)
    {
      if (ArgCnt == 4) WrError(1110);
      else
      {
        *WAsmCode |= 0xf067; WAsmCode[1] = *AdrVals;
        if (ArgCnt == 2)
        {
          *WAsmCode |= ((*WAsmCode & 0x100) << 1);
          CodeLen = 2;
        }
        else if (DecodeAdr(ArgStr[2], MModAcc))
        {
          *WAsmCode |= ((*AdrVals) << 9);
          CodeLen = 2;
        }
      }
    }

    /* syntax 1/2/4 have memory operand in front */

    else if (AdrMode == ModMem)
    {
      /* save [first] memory operand */

      Word HMode = *AdrVals, HCnt = AdrCnt;
      if (AdrCnt)
        WAsmCode[1] = AdrVals[1];
     
      /* syntax 2+4 have at least 3 operands, handle syntax 1 */

      if (ArgCnt == 2)
      {
        *WAsmCode |= 0x2800 | HMode;
        CodeLen = 1 + AdrCnt;
      }
      else 
      {
        /* both syntax 2+4 have optional second accumulator */

        if (ArgCnt == 3)
          *WAsmCode |= ((*WAsmCode & 0x100) << 1);
        else if (DecodeAdr(ArgStr[3], MModAcc))
          *WAsmCode |= ((*AdrVals) << 9);

        /* if no second accu, AdrMode is still set from previous decode */

        if (AdrMode != ModNone)
        {
          /* differentiate & handle syntax 2 & 4. OpSize still set from above! */

          DecodeAdr(ArgStr[2], MModMem | MModImm);
          switch (AdrMode)
          {
            case ModMem:
              if (MakeXY(AdrVals, TRUE))
              {
                WAsmCode[0] |= (*AdrVals);
                *AdrVals = HMode;
                if (MakeXY(&HMode, TRUE))
                {
                  WAsmCode[0] |= 0xb000 | (HMode << 4);
                  CodeLen = 1;
                }
              }
              break;
            case ModImm:
              WAsmCode[1 + HCnt] = *AdrVals;
              WAsmCode[0] |= 0x6400 | HMode;
              CodeLen = 2 + HCnt;
              break;
          }
        }
      }
    }
  }
}

static void DecodeMACDP(Word Index)
{
  Boolean OK;

  if (ArgCnt != 3) WrError(1110);
  else if (DecodeAdr(ArgStr[3], MModAcc))
  {
    *WAsmCode = Index | (0[AdrVals] << 8);
    if (DecodeAdr(ArgStr[1], MModMem))
    {
      *WAsmCode |= *AdrVals;
      if (AdrCnt)
        WAsmCode[1] = AdrVals[1];
      WAsmCode[1 + AdrCnt] = EvalIntExpression(ArgStr[2], UInt16, &OK);
      if (OK)
      {
        ChkSpace(Index & 0x200 ? SegData : SegCode);
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

static void DecodeFIRS(Word Index)
{
  Boolean OK;

  (void)Index;
 
  if (ArgCnt != 3) WrError(1110);
  else if (DecodeAdr(ArgStr[1], MModMem))
    if (MakeXY(WAsmCode, TRUE))
    {
      0[WAsmCode] = 0xe000 | ((*WAsmCode) << 4);
      if (DecodeAdr(ArgStr[2], MModMem))
        if (MakeXY(AdrVals, TRUE))
        {
          0[WAsmCode] |= *AdrVals;
          WAsmCode[1] = EvalIntExpression(ArgStr[3], UInt16, &OK);
          if (OK)
          {
            ChkSpace(SegCode);
            CodeLen = 2;
          }
        }
    }
}

static void DecodeBIT(Word Index)
{
  Boolean OK;

  (void)Index;

  if (ArgCnt != 2) WrError(1110);
  else if (DecodeAdr(ArgStr[1], MModMem))
    if (MakeXY(AdrVals, TRUE))
    {
      WAsmCode[0] = EvalIntExpression(ArgStr[2], UInt4, &OK);
      if (OK)
      {
        WAsmCode[0] |= 0x9600 | (AdrVals[0] << 4);
        CodeLen = 1;
      }
    }
}

static void DecodeBITF(Word Index)
{
  if (ArgCnt != 2) WrError(1110);
  else
  {
    OpSize = UInt16;
    if (DecodeAdr(ArgStr[2], MModImm))
    {
      WAsmCode[1] = *AdrVals;
      if (DecodeAdr(ArgStr[1], MModMem))
      {
        *WAsmCode = 0x6100 | *AdrVals;
        if (AdrCnt)
        {
          WAsmCode[2] = WAsmCode[1];
          WAsmCode[1] = AdrVals[1];
        }
        CodeLen = 2 + AdrCnt;
      }
    }
  }
}

/*-------------------------------------------------------------------------*/
/* Pseudo Instructions */

static Boolean DecodePseudo(void)
{
#define ASSUME3254xCount 3
static ASSUMERec ASSUME3254xs[ASSUME3254xCount] = 
               {{"CPL", &Reg_CPL, 0,      1,       0},
                {"DP" , &Reg_DP , 0,  0x1ff,   0x200},
                {"SP" , &Reg_SP , 0, 0xffff, 0x10000}};

  if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME3254xs,ASSUME3254xCount);
     return True;
    END

  return False;
}

static void DecodeMACR(Word Index)
{
  (void) Index;

  if ((ArgCnt < 2) || (ArgCnt > 4)) WrError(1110);
  else if (DecodeAdr(ArgStr[ArgCnt], MModAcc))
  {
    *WAsmCode = *AdrVals << 8;
    if (DecodeAdr(ArgStr[1], MModMem))
    {
      if (ArgCnt == 2)
      {
        WAsmCode[0] |= 0x2a00 | *AdrVals;
        if (AdrCnt)
          WAsmCode[1] = AdrVals[1];
        CodeLen = 1 + AdrCnt;
      }
      else
      {
        if (MakeXY(AdrVals, True))
        {
          WAsmCode[0] |= 0xb400 | ((*AdrVals) << 4);
          if (DecodeAdr(ArgStr[2], MModMem))
            if (MakeXY(AdrVals, True))
            {
              WAsmCode[0] |= *AdrVals;
              if (ArgCnt == 4)
              {
                if (DecodeAdr(ArgStr[3], MModAcc))
                  WAsmCode[0] |= (*AdrVals) << 9;
              }
              else
                *WAsmCode |= ((*WAsmCode & 0x100) << 1);
              if (AdrMode != ModNone)
                CodeLen = 1;
            }
        }
      }
    }
  }
}

static void DecodeMac(Word Index)
{
  FixedOrder *POrder = MacOrders + Index;

  if (!ArgCnt) WrError(1110);
  else if (!strcasecmp(ArgStr[1], "T"))
  {
    if (ArgCnt > 3) WrError(1110);
    else if (DecodeAdr(ArgStr[ArgCnt], MModAcc))
    {
      WAsmCode[0] = 0xf480 | (POrder->Code & 0xff) | ((*AdrVals) << 8);
      if (ArgCnt == 3)
        DecodeAdr(ArgStr[2], MModAcc);
      if (AdrMode != ModNone)
      {
        WAsmCode[0] |= ((*AdrVals) << 9);
        CodeLen = 1;
      }
    }
  }
  else if (ArgCnt > 2) WrError(1110);
  else
  {
    if ((ArgCnt == 2) && (strcasecmp(ArgStr[2], "B"))) WrError(1350);
    else if (DecodeAdr(ArgStr[1], MModMem))
    {
      WAsmCode[0] = (POrder->Code & 0xff00) | (*AdrVals);
      if (AdrCnt)
        WAsmCode[1] = AdrVals[1];
      CodeLen = 1 + AdrCnt;
    }
  }
}

static void DecodeMACSU(Word Index)
{
  (void)Index;

  if (ArgCnt != 3) WrError(1110);
  else if (DecodeAdr(ArgStr[3], MModAcc))
  {
    *WAsmCode = 0xa600 | ((*AdrVals) << 8);
    if (DecodeAdr(ArgStr[1], MModMem))
      if (MakeXY(AdrVals, TRUE))
      {
        *WAsmCode |= ((*AdrVals) << 4);
        if (DecodeAdr(ArgStr[2], MModMem))
          if (MakeXY(AdrVals, TRUE))
          {
            *WAsmCode |= *AdrVals;
            CodeLen = 1;
          }
      }
  }
}

static void DecodeMAS(Word Index)
{
  if ((ArgCnt < 2) || (ArgCnt > 4)) WrError(1110);
  else if (DecodeAdr(ArgStr[ArgCnt], MModAcc))
  {
    *WAsmCode = ((*AdrVals) << 8);
    if (DecodeAdr(ArgStr[1], MModMem))
    {
      if (ArgCnt == 2)
      {
        *WAsmCode |= 0x2c00 | Index | *AdrVals;
        if (AdrCnt)
          1[WAsmCode] = AdrVals[1];
        CodeLen = 1 + AdrCnt;
      }
      else if (MakeXY(AdrVals, TRUE))
      {
        *WAsmCode |= 0xb800 | (Index << 1) | ((*AdrVals) << 4);
        if (DecodeAdr(ArgStr[2], MModMem))
          if (MakeXY(AdrVals, TRUE))
          {
            *WAsmCode |= *AdrVals;
            if (ArgCnt == 4)
            {
              if (DecodeAdr(ArgStr[3], MModAcc))
                *WAsmCode |= ((*AdrVals) << 9);
            }
            else
              *WAsmCode |= ((*WAsmCode & 0x100) << 1);
            if (AdrMode != ModNone)
              CodeLen = 1;            
          }
      }
    }
  }
}

static void DecodeMASAR(Word Index)
{
  (void)Index;

  if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
  else if (strcasecmp(ArgStr[1], "T")) WrError(1350);
  else if (DecodeAdr(ArgStr[ArgCnt], MModAcc))
  {
    WAsmCode[0] = (*AdrVals << 8);
    if (ArgCnt == 3)
      DecodeAdr(ArgStr[2], MModAcc);
    if (AdrMode != ModNone)
    {
      *WAsmCode |= 0xf48b | ((*AdrVals) << 9);
      CodeLen = 1;
    }
  }
}

static void DecodeDADD(Word Index)
{
  (void)Index;

  if ((ArgCnt < 2) || (ArgCnt > 3)) WrError(1110);
  else if (DecodeAdr(ArgStr[ArgCnt], MModAcc))
  {
    WAsmCode[0] = 0x5000 | (AdrVals[0] << 8);
    if (ArgCnt == 3)
      DecodeAdr(ArgStr[2], MModAcc);
    if (AdrMode != ModNone)
    {
      WAsmCode[0] |= (AdrVals[0] << 9);
      if (DecodeAdr(ArgStr[1], MModMem))
      {
        WAsmCode[0] |= AdrVals[0];
        if (AdrCnt)
          WAsmCode[1] = AdrVals[1];
        CodeLen = 1 + AdrCnt;
      }
    }
  }
}

static void DecodeLog(Word Index)
{
  Word Acc, Shift;
  Boolean OK;

  if ((ArgCnt < 1) || (ArgCnt > 4)) WrError(1110);
  else
  {
    OpSize = UInt16;
    DecodeAdr(ArgStr[1], MModAcc | MModMem | MModImm);
    switch (AdrMode)
    {
      case ModAcc:  /* Variant 4 */
        if (ArgCnt == 4) WrError(1110);
        else
        {
          Acc = *AdrVals << 9;
          *WAsmCode = 0xf080 | Acc | (Index << 5);
          Shift = 0; OK = True;
          if (((ArgCnt == 2) && IsAcc(ArgStr[2])) || (ArgCnt == 3))
          {
            OK = DecodeAdr(ArgStr[ArgCnt], MModAcc);
            if (OK)
              Acc = *AdrVals << 8;
          }
          else
            Acc = Acc >> 1;
          if (OK)
            if (((ArgCnt == 2) && (!IsAcc(ArgStr[2]))) || (ArgCnt == 3))
            {
              Shift = EvalIntExpression(ArgStr[2], SInt5, &OK);
            }
          if (OK)
          {
            *WAsmCode |= Acc | (Shift & 0x1f);
            CodeLen = 1;
          }
        }
        break;

      case ModMem:  /* Variant 1 */
        if (ArgCnt != 2) WrError(1110);
        else  
        {
          *WAsmCode = 0x1800 | (*AdrVals) | (Index << 9);
          if (AdrCnt)
            WAsmCode[1] = AdrVals[1];
          CodeLen = AdrCnt + 1;
          if (DecodeAdr(ArgStr[2], MModAcc))
            *WAsmCode |= (*AdrVals) << 8;
          else
            CodeLen = 0;
        }
        break;

      case ModImm:  /* Variant 2,3 */
        if (ArgCnt == 1) WrError(1110);
        else  
        {     
          WAsmCode[1] = *AdrVals;
          if (DecodeAdr(ArgStr[ArgCnt], MModAcc))
          {
            *WAsmCode = Acc = *AdrVals << 8;
            Shift = 0; OK = True;
            if (((ArgCnt == 3) && IsAcc(ArgStr[2])) || (ArgCnt == 4))
            {
               OK = DecodeAdr(ArgStr[ArgCnt - 1], MModAcc);
               if (OK)
                Acc = (*AdrVals) << 9;
            }
            else
              Acc = Acc << 1;
            if (OK)
              if (((ArgCnt == 3) && (!IsAcc(ArgStr[2]))) || (ArgCnt == 4))
              {
                FirstPassUnknown = False;
                Shift = EvalIntExpression(ArgStr[2], UInt5, &OK);
                if (FirstPassUnknown)
                  Shift &= 15;
                OK = ChkRange(Shift, 0, 16);
              }
            if (OK)
            {
              *WAsmCode |= Acc;
              if (Shift == 16) /* Variant 3 */
              {
                *WAsmCode |= 0xf063 + Index;
              }
              else             /* Variant 2 */
              {
                *WAsmCode |= 0xf000 | ((Index + 3) << 4) | Shift;
              }
              CodeLen = 2;
            }
          }
        }
        break;
    }
  }
}

static void DecodeSFT(Word Index)
{
  Boolean OK;
  int Shift;

  if ((ArgCnt != 2) && (ArgCnt != 3)) WrError(1110);
  else if (DecodeAdr(ArgStr[1], MModAcc))
  {
    0[WAsmCode] = Index | ((*AdrVals) << 9);
    if (ArgCnt == 3)
      DecodeAdr(ArgStr[3], MModAcc);
    if (AdrMode != ModNone)
    {
      0[WAsmCode] |= ((*AdrVals) << 8);
      Shift = EvalIntExpression(ArgStr[2], SInt5, &OK);
      if (OK)
      {
        0[WAsmCode] |= (Shift & 0x1f);
        CodeLen = 1;
      }
    }
  }
}

static void DecodeCMPR(Word Index)
{
  Word z;
  Boolean OK;

  (void) Index;

  if (ArgCnt != 2) WrError(1110);
  else if (DecodeAdr(ArgStr[2], MModAReg))
  {
    OK = False;
    for (z = 0; z < 4; z++)
      if (!strcasecmp(ArgStr[1], ShortConds[z]))
      {
        OK = True;
        break;
      }
    if (!OK)
      z = EvalIntExpression(ArgStr[1], UInt2, &OK);
    if (OK)
    {
      0[WAsmCode] = 0xf4a8 | (*AdrVals) | (z << 8);
      CodeLen = 1;
    }
  }
}

/*-------------------------------------------------------------------------*/
/* Code Table Handling */

static int InstrZ;

static void AddFixed(char *Name, Word Code, Boolean IsRepeatable)
{
  if (InstrZ >= FixedOrderCnt)
    exit(0);

  FixedOrders[InstrZ].Code         = Code;
  FixedOrders[InstrZ].IsRepeatable = IsRepeatable;
  AddInstTable(InstTable, Name, InstrZ++, DecodeFixed);
}

static void AddAcc(char *Name, Word Code, Boolean IsRepeatable)
{
  if (InstrZ >= AccOrderCnt)
    exit(0);

  AccOrders[InstrZ].Code         = Code;
  AccOrders[InstrZ].IsRepeatable = IsRepeatable;
  AddInstTable(InstTable, Name, InstrZ++, DecodeAcc);
}

static void AddAcc2(char *Name, Word Code, Boolean IsRepeatable)
{
  if (InstrZ >= Acc2OrderCnt)
    exit(0);

  Acc2Orders[InstrZ].Code         = Code;
  Acc2Orders[InstrZ].IsRepeatable = IsRepeatable;
  AddInstTable(InstTable, Name, InstrZ++, DecodeAcc2);
}

static void AddMem(char *Name, Word Code, Boolean IsRepeatable)
{
  if (InstrZ >= MemOrderCnt)
    exit(0);

  MemOrders[InstrZ].Code         = Code;
  MemOrders[InstrZ].IsRepeatable = IsRepeatable;
  AddInstTable(InstTable, Name, InstrZ++, DecodeMem);
}

static void AddXY(char *Name, Word Code, Boolean IsRepeatable)
{
  if (InstrZ >= XYOrderCnt)
    exit(0);

  XYOrders[InstrZ].Code         = Code;
  XYOrders[InstrZ].IsRepeatable = IsRepeatable;
  AddInstTable(InstTable, Name, InstrZ++, DecodeXY);
}

static void AddMemAcc(char *Name, Word Code, Boolean IsRepeatable)
{
  if (InstrZ >= MemAccOrderCnt)
    exit(0);

  MemAccOrders[InstrZ].Code         = Code;
  MemAccOrders[InstrZ].IsRepeatable = IsRepeatable;
  AddInstTable(InstTable, Name, InstrZ++, DecodeMemAcc);
}

static void AddMemConst(char *Name, Word Code, Boolean IsRepeatable, Boolean Swap, IntType ConstType)
{
  if (InstrZ >= MemConstOrderCnt)
    exit(0);

  MemConstOrders[InstrZ].Code         = Code;
  MemConstOrders[InstrZ].IsRepeatable = IsRepeatable;
  MemConstOrders[InstrZ].Swap         = Swap;
  MemConstOrders[InstrZ].ConstType    = ConstType;
  AddInstTable(InstTable, Name, InstrZ++, DecodeMemConst);
}

static void AddMac(char *Name, Word Code, Boolean IsRepeatable)
{
  if (InstrZ >= MacOrderCnt)
    exit(0);

  MacOrders[InstrZ].Code         = Code;
  MacOrders[InstrZ].IsRepeatable = IsRepeatable;
  AddInstTable(InstTable, Name, InstrZ++, DecodeMac);
}

static void InitFields(void)
{
  InstTable = CreateInstTable(203);

  FixedOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * FixedOrderCnt); InstrZ = 0;
  AddFixed("FRET"  , 0xf4e4, FALSE);
  AddFixed("FRETD" , 0xf6e4, FALSE);
  AddFixed("FRETE" , 0xf4e5, FALSE);
  AddFixed("FRETED", 0xf6e5, FALSE);
  AddFixed("NOP"   , NOPCode, TRUE);
  AddFixed("RESET" , 0xf7e0, FALSE);
  AddFixed("RET"   , 0xfc00, FALSE);
  AddFixed("RETD"  , 0xfe00, FALSE);
  AddFixed("RETE"  , 0xf4eb, FALSE);
  AddFixed("RETED" , 0xf6eb, FALSE);
  AddFixed("RETF"  , 0xf49b, FALSE);
  AddFixed("RETFD" , 0xf69b, FALSE);

  AccOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * AccOrderCnt); InstrZ = 0;
  AddAcc  ("EXP"   , 0xf48e, TRUE );
  AddAcc  ("MAX"   , 0xf486, TRUE );
  AddAcc  ("MIN"   , 0xf487, TRUE );
  AddAcc  ("SAT"   , 0xf483, TRUE );
  AddAcc  ("ROL"   , 0xf491, TRUE );
  AddAcc  ("ROLTC" , 0xf492, TRUE );
  AddAcc  ("ROR"   , 0xf490, TRUE );
  AddAcc  ("SFTC"  , 0xf494, TRUE );
  AddAcc  ("CALA"  , 0xf4e3, FALSE);
  AddAcc  ("CALAD" , 0xf6e3, FALSE);
  AddAcc  ("FCALA" , 0xf4e7, FALSE);
  AddAcc  ("FCALAD", 0xf6e7, FALSE);
  AddAcc  ("BACC"  , 0xf4e2, FALSE);
  AddAcc  ("BACCD" , 0xf6e2, FALSE);
  AddAcc  ("FBACC" , 0xf4e6, FALSE);
  AddAcc  ("FBACCD", 0xf6e6, FALSE);

  Acc2Orders = (FixedOrder*) malloc(sizeof(FixedOrder) * Acc2OrderCnt); InstrZ = 0;
  AddAcc2 ("NEG"   , 0xf484, TRUE );
  AddAcc2 ("NORM"  , 0xf48f, TRUE );
  AddAcc2 ("RND"   , 0xf49f, TRUE );
  AddAcc2 ("ABS"   , 0xf485, TRUE );
  AddAcc2 ("CMPL"  , 0xf493, TRUE );

  MemOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * MemOrderCnt); InstrZ = 0;
  AddMem  ("DELAY" , 0x4d00, TRUE );
  AddMem  ("POLY"  , 0x3600, TRUE );
  AddMem  ("BITT"  , 0x3400, TRUE );
  AddMem  ("POPD"  , 0x8b00, TRUE );
  AddMem  ("PSHD"  , 0x4b00, TRUE );
  AddMem  ("MAR"   , 0x6d00, TRUE );
  AddMem  ("LTD"   , 0x4c00, TRUE );
  AddMem  ("READA" , 0x7e00, TRUE );
  AddMem  ("WRITA" , 0x7f00, TRUE );

  XYOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * XYOrderCnt); InstrZ = 0;
  AddXY   ("ABDST" , 0xe300, TRUE );
  AddXY   ("LMS"   , 0xe100, TRUE );
  AddXY   ("SQDST" , 0xe200, TRUE );

  AddInstTable(InstTable, "ADD", 0, DecodeADDSUB);
  AddInstTable(InstTable, "SUB", 1, DecodeADDSUB);

  MemAccOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * MemAccOrderCnt); InstrZ = 0;
  AddMemAcc("ADDC" , 0x0600, TRUE );
  AddMemAcc("ADDS" , 0x0200, TRUE );
  AddMemAcc("SUBB" , 0x0e00, TRUE );
  AddMemAcc("SUBC" , 0x1e00, TRUE );
  AddMemAcc("SUBS" , 0x0a00, TRUE );
  AddMemAcc("MPYR" , 0x2200, TRUE );
  AddMemAcc("MPYU" , 0x2400, TRUE );
  AddMemAcc("SQURA", 0x3800, TRUE );
  AddMemAcc("SQURS", 0x3a00, TRUE );
  AddMemAcc("DADST", 0x5a00, TRUE );
  AddMemAcc("DRSUB", 0x5800, TRUE );
  AddMemAcc("DSADT", 0x5e00, TRUE );
  AddMemAcc("DSUB" , 0x5400, TRUE );
  AddMemAcc("DSUBT", 0x5c00, TRUE );
  AddMemAcc("DLD"  , 0x5600, TRUE );
  AddMemAcc("LDU"  , 0x1200, TRUE );

  MacOrders = (FixedOrder*) malloc(sizeof(FixedOrder) * MacOrderCnt); InstrZ = 0;
  AddMac("MACA"  , 0x3508, TRUE );
  AddMac("MACAR" , 0x3709, TRUE );
  AddMac("MASA"  , 0x330a, TRUE );

  MemConstOrders = (MemConstOrder*) malloc(sizeof(MemConstOrder) * MemConstOrderCnt); InstrZ = 0;
  AddMemConst("ADDM", 0x6b00, FALSE, FALSE, SInt16);
  AddMemConst("ANDM", 0x6800, FALSE, FALSE, UInt16);
  AddMemConst("CMPM", 0x6000, TRUE , TRUE , SInt16);
  AddMemConst("ORM" , 0x6900, FALSE, FALSE, UInt16);
  AddMemConst("XORM", 0x6a00, FALSE, FALSE, UInt16);

  AddInstTable(InstTable, "AND"  , 0, DecodeLog);
  AddInstTable(InstTable, "OR"   , 1, DecodeLog);
  AddInstTable(InstTable, "XOR"  , 2, DecodeLog);
  AddInstTable(InstTable, "MPY"  , 0, DecodeMPY);
  AddInstTable(InstTable, "MPYA" , 0, DecodeMPYA);
  AddInstTable(InstTable, "SQUR" , 0, DecodeSQUR);
  AddInstTable(InstTable, "MAC"  , 0, DecodeMAC);
  AddInstTable(InstTable, "MACR" , 0, DecodeMACR);
  AddInstTable(InstTable, "MACD" , 0x7a00, DecodeMACDP);
  AddInstTable(InstTable, "MACP" , 0x7800, DecodeMACDP);
  AddInstTable(InstTable, "MACSU", 0, DecodeMACSU);
  AddInstTable(InstTable, "MAS"  , 0x000, DecodeMAS);
  AddInstTable(InstTable, "MASR" , 0x200, DecodeMAS);
  AddInstTable(InstTable, "MASAR", 0, DecodeMASAR);
  AddInstTable(InstTable, "DADD" , 0, DecodeDADD);
  AddInstTable(InstTable, "FIRS" , 0, DecodeFIRS);
  AddInstTable(InstTable, "SFTA" , 0xf460, DecodeSFT);
  AddInstTable(InstTable, "SFTL" , 0xf0e0, DecodeSFT);
  AddInstTable(InstTable, "BIT"  , 0, DecodeBIT);
  AddInstTable(InstTable, "BITF" , 0, DecodeBITF);
  AddInstTable(InstTable, "CMPR" , 0, DecodeCMPR);
}

static void DeinitFields(void)
{
  DestroyInstTable(InstTable);
  free(FixedOrders);
  free(AccOrders);
  free(Acc2Orders);
  free(MemOrders);
  free(XYOrders);
  free(MemAccOrders);
  free(MemConstOrders);
  free(MacOrders);
}

/*-------------------------------------------------------------------------*/
/* Linking Routines */

static void MakeCode_32054x(void)
{
  CodeLen = 0; DontPrint = False;

  /* zu ignorierendes */

  if (*OpPart=='\0') return;

  if (DecodePseudo()) return;

  /* search */

  ThisRep = False;
  if (!LookupInstTable(InstTable, OpPart))
    WrXError(1200,OpPart);
  LastRep = ThisRep;
}

static void InitCode_32054x(void)
{
  SaveInitProc();
  Reg_CPL = 0;
  Reg_DP = 0;
  Reg_SP = 0;
}

static Boolean IsDef_32054x(void)
{
  return (!strcmp(LabPart, "||"));
}

static void SwitchFrom_32054x(void)
{
  DeinitFields();
}

static void SwitchTo_32054x(void)
{
  PFamilyDescr FoundDescr;

  FoundDescr = FindFamilyByName("TMS320C54x");

  TurnWords = False; ConstMode = ConstModeIntel; SetIsOccupied = False;

  PCSymbol = "$"; HeaderID = FoundDescr->Id; NOPCode = 0xf495;
  DivideChars = ","; HasAttrs = False;

  ValidSegs = (1 << SegCode) | (1 << SegData) | (1 << SegIO);
  Grans[SegCode] = 2; ListGrans[SegCode] = 2; SegInits[SegCode] = 2; SegLimits[SegCode] = 0xffff;
  Grans[SegData] = 2; ListGrans[SegData] = 2; SegInits[SegData] = 2; SegLimits[SegData] = 0xffff;
  Grans[SegIO  ] = 2; ListGrans[SegIO  ] = 2; SegInits[SegIO  ] = 2; SegLimits[SegIO  ] = 0xffff;

  MakeCode = MakeCode_32054x; IsDef = IsDef_32054x;
  
  InitFields(); SwitchFrom = SwitchFrom_32054x;
  ThisRep = LastRep = False;
}

/*-------------------------------------------------------------------------*/
/* Global Interface */

void code32054x_init(void)
{
  CPU320C541 = AddCPU("320C541", SwitchTo_32054x);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_32054x;
}
