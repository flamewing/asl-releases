/* code601.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator PowerPC-Familie                                             */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "code601.h"

#include "asmallg.h"
#include "asmcode.h"
#include "asmdef.h"
#include "asmitree.h"
#include "asmpars.h"
#include "asmsub.h"
#include "as_endian.h"
#include "codevars.h"
#include "errmsg.h"
#include "as.rsc"
#include "nlmessages.h"
#include "headids.h"
#include "intpseudo.h"
#include "onoff_common.h"
#include "strutil.h"

#include <string.h>

typedef struct {
    char const* Name;
    LongWord    Code;
    Byte        CPUMask;
} BaseOrder;

static BaseOrder* FixedOrders;
static BaseOrder* Reg1Orders;
static BaseOrder* CReg1Orders;
static BaseOrder* CBit1Orders;
static BaseOrder* FReg1Orders;
static BaseOrder* Reg2Orders;
static BaseOrder* CReg2Orders;
static BaseOrder* FReg2Orders;
static BaseOrder* Reg2BOrders;
static BaseOrder* Reg2SwapOrders;
static BaseOrder* NoDestOrders;
static BaseOrder* Reg3Orders;
static BaseOrder* CReg3Orders;
static BaseOrder* FReg3Orders;
static BaseOrder* Reg3SwapOrders;
static BaseOrder* MixedOrders;
static BaseOrder* FReg4Orders;
static BaseOrder* RegDispOrders;
static BaseOrder* FRegDispOrders;
static BaseOrder* Reg2ImmOrders;
static BaseOrder* Imm16Orders;
static BaseOrder* Imm16SwapOrders;

static CPUVar CPU403, CPU403C, CPU505, CPU601, CPU821, CPU6000;

#define M_403  0x01
#define M_403C 0x02
#define M_505  0x04
#define M_601  0x08
#define M_821  0x10
#define M_6000 0x20
#define M_SUP  0x80

#define TO_U32(_n) ((LongWord)_n)

static char           ZeroStr[] = "0";
static tStrComp const ZeroComp  = {
        {-1, 0},
        {0, ZeroStr, 0}
};

/*-------------------------------------------------------------------------*/

static void PutCode(LongWord Code) {
#if 0
  memcpy(BAsmCode, &Code, 4);
  if (!TargetBigEndian)
    DSwap((void *)BAsmCode, 4);
#endif
    DAsmCode[0] = Code;
}

/*-------------------------------------------------------------------------*/

/*!------------------------------------------------------------------------
 * \fn     DecodeGenRegCore(const char *pArg, LongWord *pValue)
 * \brief  check whether argument is general register
 * \param  pArg source argument
 * \param  pValue register # if it's a register
 * \return True if it's a register
 * ------------------------------------------------------------------------ */

static Boolean DecodeGenRegCore(char const* pArg, LongWord* pValue) {
    if ((strlen(pArg) < 2) || (as_toupper(*pArg) != 'R')) {
        return False;
    } else {
        char* p_end;

        *pValue = strtoul(pArg + 1, &p_end, 10);
        return (!*p_end && (*pValue <= 31));
    }
}

/*!------------------------------------------------------------------------
 * \fn     DecodeFPRegCore(const char *pArg, LongWord *pValue)
 * \brief  check whether argument is floating point register
 * \param  pArg source argument
 * \param  pValue register # if it's a register
 * \return True if it's a register
 * ------------------------------------------------------------------------ */

static Boolean DecodeFPRegCore(char const* pArg, LongWord* pValue) {
    if ((strlen(pArg) < 3) || (as_toupper(*pArg) != 'F')
        || (as_toupper(pArg[1]) != 'R')) {
        return False;
    } else {
        char* p_end;

        *pValue = strtoul(pArg + 2, &p_end, 10);
        return !*p_end && (*pValue <= 31);
    }
}

/*!------------------------------------------------------------------------
 * \fn     DecodeGenReg(const tStrComp *pArg, LongWord *pValue)
 * \brief  check whether argument is general register, including aliases
 * \param  pArg source argument
 * \param  pValue register # if it's a register
 * \return True if it's a register
 * ------------------------------------------------------------------------ */

static Boolean DecodeGenReg(tStrComp const* pArg, LongWord* pValue) {
    tRegDescr      RegDescr;
    tEvalResult    EvalResult;
    tRegEvalResult RegEvalResult;

    if (DecodeGenRegCore(pArg->str.p_str, pValue)) {
        return True;
    }

    RegEvalResult = EvalStrRegExpressionAsOperand(
            pArg, &RegDescr, &EvalResult, eSymbolSize32Bit, True);
    if (RegEvalResult == eIsReg) {
        *pValue = RegDescr.Reg;
    }
    return (RegEvalResult == eIsReg);
}

/*!------------------------------------------------------------------------
 * \fn     DissectReg_601(char *pDest, size_t DestSize, tRegInt Value, tSymbolSize InpSize)
 * \brief  dissect register symbols - PPC variant
 * \param  pDest destination buffer
 * \param  DestSize destination buffer size
 * \param  Value numeric register value
 * \param  InpSize register size
 * ------------------------------------------------------------------------ */

static void DissectReg_601(
        char* pDest, size_t DestSize, tRegInt Value, tSymbolSize InpSize) {
    switch (InpSize) {
    case eSymbolSize32Bit:
        as_snprintf(pDest, DestSize, "R%u", (unsigned)Value);
        break;
    case eSymbolSizeFloat64Bit:
        as_snprintf(pDest, DestSize, "FR%u", (unsigned)Value);
        break;
    default:
        as_snprintf(pDest, DestSize, "%d-%u", (int)InpSize, (unsigned)Value);
    }
}

/*!------------------------------------------------------------------------
 * \fn     DecodeFPReg(const tStrComp *pArg, LongWord *pValue)
 * \brief  check whether argument is floating point register, including aliases
 * \param  pArg source argument
 * \param  pValue register # if it's a register
 * \return True if it's a register
 * ------------------------------------------------------------------------ */

static Boolean DecodeFPReg(tStrComp const* pArg, LongWord* pValue) {
    tRegDescr      RegDescr;
    tEvalResult    EvalResult;
    tRegEvalResult RegEvalResult;

    if (DecodeFPRegCore(pArg->str.p_str, pValue)) {
        return True;
    }

    RegEvalResult = EvalStrRegExpressionAsOperand(
            pArg, &RegDescr, &EvalResult, eSymbolSizeFloat64Bit, True);
    if (RegEvalResult == eIsReg) {
        *pValue = RegDescr.Reg;
    }
    return (RegEvalResult == eIsReg);
}

/*!------------------------------------------------------------------------
 * \fn     DecodeCondReg(const tStrComp *p_comp, LongWord *p_ret, LongWord *p_guess_mask)
 * \brief  handle condition code field index argument
 * \param  p_comp source argument
 * \param  p_ret encoded condition code field index
 * \param  p_guess_mask associated guess flag bits
 * \return True if parsing OK
 * ------------------------------------------------------------------------ */

static Boolean DecodeCondReg(
        tStrComp const* p_comp, LongWord* p_ret, LongWord* p_guess_mask) {
    Boolean      OK;
    tSymbolFlags flags;

    LongInt value = EvalStrIntExpressionWithFlags(p_comp, UInt3, &OK, &flags);
    if (OK) {
        *p_ret = (LongWord)value << 2;
        *p_guess_mask = (mFirstPassUnknownOrQuestionable(flags) ? 7 : 0) << 2;
    }
    return OK;
}

/*!------------------------------------------------------------------------
 * \fn     DecodeCondBit(const tStrComp *p_comp, LongWord *p_ret, LongWord *p_guess_mask)
 * \brief  handle condition code bit argument
 * \param  p_comp source argument
 * \param  p_ret encoded condition code
 * \param  p_guess_mask associated guess flag bits
 * \return True if parsing OK
 * ------------------------------------------------------------------------ */

static Boolean DecodeCondBit(
        tStrComp const* p_comp, LongWord* p_ret, LongWord* p_guess_mask) {
    Boolean      OK;
    tSymbolFlags flags;

    *p_ret        = EvalStrIntExpressionWithFlags(p_comp, UInt5, &OK, &flags);
    *p_guess_mask = mFirstPassUnknownOrQuestionable(flags) ? 31 : 0;
    return OK;
}

/*!------------------------------------------------------------------------
 * \fn     DecodeRegDisp(tStrComp *p_comp, LongWord *p_ret, LongWord *p_guess_mask)
 * \brief  parse register/displacement argument
 * \param  p_comp source argument
 * \param  p_ret encoded result
 * \param  p_guess_mask associated guess flag bits
 * \return True if parsing OK
 * ------------------------------------------------------------------------ */

static Boolean DecodeRegDisp(tStrComp* p_comp, LongWord* p_ret, LongWord* p_guess_mask) {
    char*        p;
    int          l = strlen(p_comp->str.p_str);
    LongInt      Disp;
    Boolean      OK;
    tSymbolFlags flags;
    tStrComp     DispArg, RegArg;

    if (p_comp->str.p_str[l - 1] != ')') {
        WrStrErrorPos(ErrNum_InvAddrMode, p_comp);
        return False;
    }
    p_comp->str.p_str[--l] = '\0';
    p                      = p_comp->str.p_str + l - 1;
    while ((p >= p_comp->str.p_str) && (*p != '(')) {
        p--;
    }
    if (p < p_comp->str.p_str) {
        WrStrErrorPos(ErrNum_InvAddrMode, p_comp);
        return False;
    }
    StrCompSplitRef(&DispArg, &RegArg, p_comp, p);
    if (!DecodeGenReg(&RegArg, p_ret)) {
        return False;
    }
    *p   = '\0';
    Disp = EvalStrIntExpressionWithFlags(&DispArg, Int16, &OK, &flags);
    if (!OK) {
        return False;
    }

    *p_ret        = (*p_ret << 16) + (Disp & 0xffff);
    *p_guess_mask = mFirstPassUnknownOrQuestionable(flags) ? 0xffff : 0;
    return True;
}

/*-------------------------------------------------------------------------*/

static LongWord ExtractPoint(Word Code) {
    return (Code >> 15) & 1;
}

static void ChkSup(void) {
    if (!SupAllowed) {
        WrError(ErrNum_PrivOrder);
    }
}

static void SwapCode(LongWord* Code) {
    *Code = ((*Code & 0x1f) << 5) | ((*Code >> 5) & 0x1f);
}

/*-------------------------------------------------------------------------*/

/* ohne Argument */

static void DecodeFixed(Word Index) {
    BaseOrder const* pOrder = FixedOrders + Index;

    if (ChkArgCnt(0, 0) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)) {
        CodeLen = 4;
        PutCode(pOrder->Code);
        if (pOrder->CPUMask & M_SUP) {
            ChkSup();
        }
    }
}

/* ein Register */

static void DecodeReg1(Word Index) {
    BaseOrder const* pOrder = Reg1Orders + Index;
    LongWord         Dest;

    if (ChkArgCnt(1, 1) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21));
        if (pOrder->CPUMask & M_SUP) {
            ChkSup();
        }
    }
}

/* ein Steuerregister */

static void DecodeCReg1(Word Index) {
    BaseOrder const* pOrder = CReg1Orders + Index;
    LongWord         Dest, dest_guess_mask;

    if (!ChkArgCnt(1, 1))
        ;
    else if (ChkExactCPUMask(pOrder->CPUMask, CPU403) < 0)
        ;
    else if (!DecodeCondReg(&ArgStr[1], &Dest, &dest_guess_mask))
        ;
    else if (Dest & 3) {
        WrStrErrorPos(ErrNum_AddrMustBeAligned, &ArgStr[1]);
    } else {
        CodeLen = 4;
        or_dasmcode_guessed(0, 1, dest_guess_mask << 21);
        PutCode(pOrder->Code + (Dest << 21));
    }
}

/* ein Steuerregisterbit */

static void DecodeCBit1(Word Index) {
    BaseOrder const* pOrder = CBit1Orders + Index;
    LongWord         Dest, dest_guess_mask;

    if (ChkArgCnt(1, 1) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeCondBit(&ArgStr[1], &Dest, &dest_guess_mask)) {
        CodeLen = 4;
        or_dasmcode_guessed(0, 1, dest_guess_mask << 21);
        PutCode(pOrder->Code + (Dest << 21));
    }
}

/* ein Gleitkommaregister */

static void DecodeFReg1(Word Index) {
    BaseOrder const* pOrder = FReg1Orders + Index;
    LongWord         Dest;

    if (ChkArgCnt(1, 1) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeFPReg(&ArgStr[1], &Dest)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21));
    }
}

/* 1/2 Integer-Register */

static void DecodeReg2(Word Index) {
    BaseOrder const* pOrder = Reg2Orders + Index;
    LongWord         Dest, Src1;
    tStrComp const*  pArg2 = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[1];

    if (ChkArgCnt(1, 2) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest) && DecodeGenReg(pArg2, &Src1)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 16));
    }
}

/* 2 Bedingungs-Bits */

static void DecodeCReg2(Word Index) {
    BaseOrder const* pOrder = CReg2Orders + Index;
    LongWord         Dest, Src1, dest_guess_mask, src1_guess_mask;

    if (!ChkArgCnt(2, 2))
        ;
    else if (ChkExactCPUMask(pOrder->CPUMask, CPU403) < 0)
        ;
    else if (!DecodeCondReg(&ArgStr[1], &Dest, &dest_guess_mask))
        ;
    else if (Dest & 3) {
        WrStrErrorPos(ErrNum_AddrMustBeAligned, &ArgStr[1]);
    } else if (!DecodeCondReg(&ArgStr[2], &Src1, &src1_guess_mask))
        ;
    else if (Src1 & 3) {
        WrStrErrorPos(ErrNum_AddrMustBeAligned, &ArgStr[2]);
    } else {
        CodeLen = 4;
        or_dasmcode_guessed(0, 1, (dest_guess_mask << 21) | (src1_guess_mask << 16));
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 16));
    }
}

/* 1/2 Float-Register */

static void DecodeFReg2(Word Index) {
    BaseOrder const* pOrder = FReg2Orders + Index;
    LongWord         Dest, Src1;
    tStrComp const*  pArg2 = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[1];

    if (ChkArgCnt(1, 2) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeFPReg(&ArgStr[1], &Dest) && DecodeFPReg(pArg2, &Src1)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 11));
    }
}

/* 1/2 Integer-Register, Quelle in B */

static void DecodeReg2B(Word Index) {
    BaseOrder const* pOrder = Reg2BOrders + Index;
    LongWord         Dest, Src1;
    tStrComp const*  pArg2 = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[1];

    if (ChkArgCnt(1, 2) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest) && DecodeGenReg(pArg2, &Src1)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 11));
        ChkSup();
    }
}

/* 1/2 Integer-Register, getauscht */

static void DecodeReg2Swap(Word Index) {
    BaseOrder const* pOrder = Reg2SwapOrders + Index;
    LongWord         Dest, Src1;
    tStrComp const*  pArg2 = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[1];

    if (ChkArgCnt(1, 2) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest) && DecodeGenReg(pArg2, &Src1)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 16) + (Src1 << 21));
    }
}

/* 2 Integer-Register, kein Ziel */

static void DecodeNoDest(Word Index) {
    BaseOrder const* pOrder = NoDestOrders + Index;
    LongWord         Src2, Src1;

    if (ChkArgCnt(2, 2) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Src1) && DecodeGenReg(&ArgStr[2], &Src2)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Src1 << 16) + (Src2 << 11));
    }
}

/* 2/3 Integer-Register */

static void DecodeReg3(Word Index) {
    BaseOrder const* pOrder = Reg3Orders + Index;
    tStrComp const * pArg2  = (ArgCnt == 2) ? &ArgStr[1] : &ArgStr[2],
                   *pArg3   = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[3];
    LongWord Src2, Src1, Dest;

    if (ChkArgCnt(2, 3) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest) && DecodeGenReg(pArg2, &Src1)
        && DecodeGenReg(pArg3, &Src2)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
    }
}

/* 2/3 Bedingungs-Bits */

static void DecodeCReg3(Word Index) {
    BaseOrder const* pOrder = CReg3Orders + Index;
    tStrComp const * pArg2  = (ArgCnt == 2) ? &ArgStr[1] : &ArgStr[2],
                   *pArg3   = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[3];
    LongWord Src2, Src1, Dest, src2_guess_mask, src1_guess_mask, dest_guess_mask;

    if (ChkArgCnt(2, 3) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeCondBit(&ArgStr[1], &Dest, &dest_guess_mask)
        && DecodeCondBit(pArg2, &Src1, &src1_guess_mask)
        && DecodeCondBit(pArg3, &Src2, &src2_guess_mask)) {
        CodeLen = 4;
        or_dasmcode_guessed(
                0, 1,
                (dest_guess_mask << 21) | (src1_guess_mask << 16)
                        | (src2_guess_mask << 11));
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
    }
}

/* 2/3 Float-Register */

static void DecodeFReg3(Word Index) {
    BaseOrder const* pOrder = FReg3Orders + Index;
    tStrComp const * pArg2  = (ArgCnt == 2) ? &ArgStr[1] : &ArgStr[2],
                   *pArg3   = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[3];
    LongWord Src2, Src1, Dest;

    if (ChkArgCnt(2, 3) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeFPReg(&ArgStr[1], &Dest) && DecodeFPReg(pArg2, &Src1)
        && DecodeFPReg(pArg3, &Src2)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
    }
}

/* 2/3 Integer-Register, Ziel & Quelle 1 getauscht */

static void DecodeReg3Swap(Word Index) {
    BaseOrder const* pOrder = Reg3SwapOrders + Index;
    tStrComp const * pArg2  = (ArgCnt == 2) ? &ArgStr[1] : &ArgStr[2],
                   *pArg3   = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[3];
    LongWord Src2, Src1, Dest;

    if (ChkArgCnt(2, 3) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest) && DecodeGenReg(pArg2, &Src1)
        && DecodeGenReg(pArg3, &Src2)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 16) + (Src1 << 21) + (Src2 << 11));
    }
}

/* 1 Float und 2 Integer-Register */

static void DecodeMixed(Word Index) {
    BaseOrder const* pOrder = MixedOrders + Index;
    LongWord         Src2, Src1, Dest;

    if (ChkArgCnt(3, 3) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeFPReg(&ArgStr[1], &Dest) && DecodeGenReg(&ArgStr[2], &Src1)
        && DecodeGenReg(&ArgStr[3], &Src2)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11));
    }
}

/* 3/4 Float-Register */

static void DecodeFReg4(Word Index) {
    BaseOrder const* pOrder = FReg4Orders + Index;
    LongWord         Src3, Src2, Src1, Dest;
    tStrComp const * pArg2 = (ArgCnt == 3) ? &ArgStr[1] : &ArgStr[2],
                   *pArg3  = (ArgCnt == 3) ? &ArgStr[2] : &ArgStr[3],
                   *pArg4  = (ArgCnt == 3) ? &ArgStr[3] : &ArgStr[4];

    if (ChkArgCnt(3, 4) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeFPReg(&ArgStr[1], &Dest) && DecodeFPReg(pArg2, &Src1)
        && DecodeFPReg(pArg3, &Src3) && DecodeFPReg(pArg4, &Src2)) {
        CodeLen = 4;
        PutCode(pOrder->Code + (Dest << 21) + (Src1 << 16) + (Src2 << 11) + (Src3 << 6));
    }
}

/* Register mit indiziertem Speicheroperanden */

static void DecodeRegDispOrder(Word Index) {
    BaseOrder const* pOrder = RegDispOrders + Index;
    LongWord         Src1, Dest, src1_guess_mask;

    if (ChkArgCnt(2, 2) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest)
        && DecodeRegDisp(&ArgStr[2], &Src1, &src1_guess_mask)) {
        or_dasmcode_guessed(0, 1, src1_guess_mask);
        PutCode(pOrder->Code + (Dest << 21) + Src1);
        CodeLen = 4;
    }
}

/* Gleitkommaregister mit indiziertem Speicheroperandem */

static void DecodeFRegDisp(Word Index) {
    BaseOrder const* pOrder = FRegDispOrders + Index;
    LongWord         Src1, Dest, src1_guess_mask;

    if (ChkArgCnt(2, 2) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeFPReg(&ArgStr[1], &Dest)
        && DecodeRegDisp(&ArgStr[2], &Src1, &src1_guess_mask)) {
        or_dasmcode_guessed(0, 1, src1_guess_mask);
        PutCode(pOrder->Code + (Dest << 21) + Src1);
        CodeLen = 4;
    }
}

/* 2 verdrehte Register mit immediate */

static void DecodeReg2Imm(Word Index) {
    BaseOrder const* pOrder = Reg2ImmOrders + Index;
    LongWord         Src1, Dest, Src2;
    Boolean          OK;

    if (ChkArgCnt(3, 3) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest) && DecodeGenReg(&ArgStr[2], &Src1)) {
        tSymbolFlags flags;

        Src2 = EvalStrIntExpressionWithFlags(&ArgStr[3], UInt5, &OK, &flags);
        if (OK) {
            or_d_guessed(flags, 0, 1, 0x1f << 11);
            PutCode(pOrder->Code + (Src1 << 21) + (Dest << 16) + (Src2 << 11));
            CodeLen = 4;
        }
    }
}

/* 2 Register+immediate */

static void DecodeImm16(Word Index) {
    BaseOrder const* pOrder = Imm16Orders + Index;
    LongWord         Src1, Dest, Imm;
    Boolean          OK;
    tStrComp const * pArg2 = (ArgCnt == 2) ? &ArgStr[1] : &ArgStr[2],
                   *pArg3  = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[3];

    if (ChkArgCnt(2, 3) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest) && DecodeGenReg(pArg2, &Src1)) {
        tSymbolFlags flags;

        Imm = EvalStrIntExpressionWithFlags(pArg3, Int16, &OK, &flags);
        if (OK) {
            CodeLen = 4;
            or_d_guessed(flags, 0, 1, 0x1f << 11);
            PutCode(pOrder->Code + (Dest << 21) + (Src1 << 16) + (Imm & 0xffff));
        }
    }
}

/* 2 Register+immediate, Ziel & Quelle 1 getauscht */

static void DecodeImm16Swap(Word Index) {
    BaseOrder const* pOrder = Imm16SwapOrders + Index;
    LongWord         Src1, Dest, Imm;
    Boolean          OK;
    tStrComp const * pArg2 = (ArgCnt == 2) ? &ArgStr[1] : &ArgStr[2],
                   *pArg3  = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[3];

    if (ChkArgCnt(2, 3) && (ChkExactCPUMask(pOrder->CPUMask, CPU403) >= 0)
        && DecodeGenReg(&ArgStr[1], &Dest) && DecodeGenReg(pArg2, &Src1)) {
        tSymbolFlags flags;

        Imm = EvalStrIntExpressionWithFlags(pArg3, Int16, &OK, &flags);
        if (OK) {
            CodeLen = 4;
            or_d_guessed(flags, 0, 1, 0x1f << 11);
            PutCode(pOrder->Code + (Dest << 16) + (Src1 << 21) + (Imm & 0xffff));
        }
    }
}

/* Ausreisser... */

static void DecodeFMUL_FMULS(Word Code) {
    tStrComp const *pArg2 = (ArgCnt == 2) ? &ArgStr[1] : &ArgStr[2],
                   *pArg3 = (ArgCnt == 2) ? &ArgStr[2] : &ArgStr[3];
    LongWord Dest, Src1, Src2, LCode = (Code & 0x7fff);

    if (ChkArgCnt(2, 3) && DecodeFPReg(&ArgStr[1], &Dest) && DecodeFPReg(pArg2, &Src1)
        && DecodeFPReg(pArg3, &Src2)) {
        PutCode((LCode << 26) + (25 << 1) + (Dest << 21) + (Src1 << 16) + (Src2 << 6)
                + ExtractPoint(Code));
        CodeLen = 4;
    }
}

static void DecodeLSWI_STSWI(Word Code) {
    LongWord Dest, Src1, Src2, LCode = Code;
    Boolean  OK;

    if (ChkArgCnt(3, 3) && DecodeGenReg(&ArgStr[1], &Dest)
        && DecodeGenReg(&ArgStr[2], &Src1)) {
        tSymbolFlags flags;

        Src2 = EvalStrIntExpressionWithFlags(&ArgStr[3], UInt5, &OK, &flags);
        if (OK) {
            or_d_guessed(flags, 0, 1, 0x1f << 11);
            PutCode((TO_U32(31) << 26) + (LCode << 1) + (Dest << 21) + (Src1 << 16)
                    + (Src2 << 11));
            CodeLen = 4;
        }
    }
}

static void DecodeMTFB_MTTB(Word Code) {
    LongWord        LCode = Code, Src1, Dest;
    Boolean         OK;
    tStrComp const *pArg1 = &ArgStr[1], *pArg2 = &ArgStr[2];
    tStrComp        TmpComp = {
            {-1, 0},
            {0, NULL, 0}
    };

    if (ChkExactCPUList(ErrNum_InstructionNotSupported, CPU821, CPU505, CPUNone) < 0)
        ;
    else if (ArgCnt == 1) {
        pArg1 = &ArgStr[1];
        if ((Memo("MFTB")) || (Memo("MFTBL"))) {
            TmpComp.str.p_str = (char*)"268";
        } else if (Memo("MFTBU")) {
            TmpComp.str.p_str = (char*)"269";
        } else if ((Memo("MTTB")) || (Memo("MTTBL"))) {
            TmpComp.str.p_str = (char*)"284";
        } else if (Memo("MTTBU")) {
            TmpComp.str.p_str = (char*)"285";
        }
        pArg2 = &TmpComp;
        /* already swapped */
    } else if ((ArgCnt == 2) && (Code == 467)) /* MTxx */
    {
        pArg1 = &ArgStr[2];
        pArg2 = &ArgStr[1];
    }
    if (ChkArgCnt(1, 2) && DecodeGenReg(pArg1, &Dest)) {
        tSymbolFlags flags;

        Src1 = EvalStrIntExpressionWithFlags(pArg2, UInt10, &OK, &flags);
        if (OK) {
            if ((Src1 == 268) || (Src1 == 269) || (Src1 == 284) || (Src1 == 285)) {
                SwapCode(&Src1);
                or_d_guessed(flags, 0, 1, 0x3ff << 11);
                PutCode((TO_U32(31) << 26) + (Dest << 21) + (Src1 << 11) + (LCode << 1));
                CodeLen = 4;
            } else {
                WrError(ErrNum_InvCtrlReg);
            }
        }
    }
}

static void DecodeMFSPR_MTSPR(Word Code) {
    LongWord        Dest, Src1, LCode = Code;
    tStrComp const *pArg1 = (Code == 467) ? &ArgStr[2] : &ArgStr[1],
                   *pArg2 = (Code == 467) ? &ArgStr[1] : &ArgStr[2];
    Boolean OK;

    if (ChkArgCnt(2, 2) && DecodeGenReg(pArg1, &Dest)) {
        tSymbolFlags flags;

        Src1 = EvalStrIntExpressionWithFlags(pArg2, UInt10, &OK, &flags);
        if (OK) {
            SwapCode(&Src1);
            or_d_guessed(flags, 0, 1, 0x3ff << 11);
            PutCode((TO_U32(31) << 26) + (Dest << 21) + (Src1 << 11) + (LCode << 1));
            CodeLen = 4;
        }
    }
}

static void DecodeMFDCR_MTDCR(Word Code) {
    LongWord        LCode = Code, Src1, Dest;
    tStrComp const *pArg1 = (Code == 451) ? &ArgStr[2] : &ArgStr[1],
                   *pArg2 = (Code == 451) ? &ArgStr[1] : &ArgStr[2];
    Boolean OK;

    if (ChkArgCnt(2, 2)
        && (ChkExactCPUList(ErrNum_InstructionNotSupported, CPU403, CPU403C, CPUNone)
            >= 0)
        && DecodeGenReg(pArg1, &Dest)) {
        tSymbolFlags flags;

        Src1 = EvalStrIntExpressionWithFlags(pArg2, UInt10, &OK, &flags);
        if (OK) {
            SwapCode(&Src1);
            or_d_guessed(flags, 0, 1, 0x3ff << 11);
            PutCode((TO_U32(31) << 26) + (Dest << 21) + (Src1 << 11) + (LCode << 1));
            CodeLen = 4;
        }
    }
}

static void DecodeMFSR_MTSR(Word Code) {
    LongWord        LCode = Code, Src1, Dest;
    tStrComp const *pArg1 = (Code == 210) ? &ArgStr[2] : &ArgStr[1],
                   *pArg2 = (Code == 210) ? &ArgStr[1] : &ArgStr[2];
    Boolean OK;

    if (ChkArgCnt(2, 2) && DecodeGenReg(pArg1, &Dest)) {
        tSymbolFlags flags;

        Src1 = EvalStrIntExpressionWithFlags(pArg2, UInt4, &OK, &flags);
        if (OK) {
            or_d_guessed(flags, 0, 1, 0xf << 16);
            PutCode((TO_U32(31) << 26) + (Dest << 21) + (Src1 << 16) + (LCode << 1));
            CodeLen = 4;
            ChkSup();
        }
    }
}

static void DecodeMTCRF(Word Code) {
    LongWord Src1;

    UNUSED(Code);

    if (ChkArgCnt(1, 2) && DecodeGenReg(&ArgStr[ArgCnt], &Src1)) {
        Boolean      OK    = True;
        tSymbolFlags flags = eSymbolFlag_None;
        LongWord     Dest
                = (ArgCnt == 1)
                          ? 0xff
                          : EvalStrIntExpressionWithFlags(&ArgStr[1], UInt8, &OK, &flags);

        if (OK) {
            or_d_guessed(flags, 0, 1, 0xff << 12);
            PutCode((TO_U32(31) << 26) + (Src1 << 21) + (Dest << 12) + (144 << 1));
            CodeLen = 4;
        }
    }
}

static void DecodeMTFSF(Word Code) {
    LongWord Src1;

    UNUSED(Code);

    if (ChkArgCnt(2, 2) && DecodeFPReg(&ArgStr[2], &Src1)) {
        tSymbolFlags flags;
        Boolean      OK;
        LongWord     Dest = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt8, &OK, &flags);

        if (OK) {
            or_d_guessed(flags, 0, 1, 0xff << 17);
            PutCode((TO_U32(63) << 26) + (Dest << 17) + (Src1 << 11) + (711 << 1)
                    + ExtractPoint(Code));
            CodeLen = 4;
        }
    }
}

static void DecodeMTFSFI(Word Code) {
    LongWord Dest, dest_guess_mask;

    if (!ChkArgCnt(2, 2))
        ;
    else if (!DecodeCondReg(&ArgStr[1], &Dest, &dest_guess_mask))
        ;
    else if (Dest & 3) {
        WrStrErrorPos(ErrNum_AddrMustBeAligned, &ArgStr[1]);
    } else {
        tSymbolFlags flags;
        Boolean      OK;
        LongWord     Src1 = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt4, &OK, &flags);

        if (OK) {
            or_dasmcode_guessed(0, 1, dest_guess_mask << 21);
            or_d_guessed(flags, 0, 1, 0xf << 12);
            PutCode((TO_U32(63) << 26) + (Dest << 21) + (Src1 << 12) + (134 << 1)
                    + ExtractPoint(Code));
            CodeLen = 4;
        }
    }
}

static void DecodeRLMI(Word Code) {
    LongWord Dest, Src1, Src2;

    if (ChkArgCnt(5, 5) && ChkMinCPU(CPU6000) && DecodeGenReg(&ArgStr[1], &Dest)
        && DecodeGenReg(&ArgStr[2], &Src1) && DecodeGenReg(&ArgStr[3], &Src2)) {
        Boolean      OK;
        tSymbolFlags src3_flags;
        LongWord     Src3
                = EvalStrIntExpressionWithFlags(&ArgStr[4], UInt5, &OK, &src3_flags);
        if (OK) {
            tSymbolFlags imm_flags;
            Integer      Imm
                    = EvalStrIntExpressionWithFlags(&ArgStr[5], UInt5, &OK, &imm_flags);
            if (OK) {
                or_d_guessed(src3_flags, 0, 1, 0x1f << 6);
                or_d_guessed(imm_flags, 0, 1, 0x1f << 1);
                PutCode((TO_U32(22) << 26) + (Src1 << 21) + (Dest << 16) + (Src2 << 11)
                        + (Src3 << 6) + (Imm << 1) + ExtractPoint(Code));
                CodeLen = 4;
            }
        }
    }
}

static void DecodeRLWNM(Word Code) {
    LongWord Dest, Src1, Src2;

    if (ChkArgCnt(5, 5) && DecodeGenReg(&ArgStr[1], &Dest)
        && DecodeGenReg(&ArgStr[2], &Src1) && DecodeGenReg(&ArgStr[3], &Src2)) {
        Boolean      OK;
        tSymbolFlags src3_flags;
        LongWord     Src3
                = EvalStrIntExpressionWithFlags(&ArgStr[4], UInt5, &OK, &src3_flags);
        if (OK) {
            tSymbolFlags imm_flags;
            Integer      Imm
                    = EvalStrIntExpressionWithFlags(&ArgStr[5], UInt5, &OK, &imm_flags);
            if (OK) {
                or_d_guessed(src3_flags, 0, 1, 0x1f << 6);
                or_d_guessed(imm_flags, 0, 1, 0x1f << 1);
                PutCode((TO_U32(23) << 26) + (Src1 << 21) + (Dest << 16) + (Src2 << 11)
                        + (Src3 << 6) + (Imm << 1) + ExtractPoint(Code));
                CodeLen = 4;
            }
        }
    }
}

static void DecodeRLWIMI_RLWINM(Word Code) {
    LongWord Dest, Src1, LCode = Code & 0x7fff;

    if (ChkArgCnt(5, 5) && DecodeGenReg(&ArgStr[1], &Dest)
        && DecodeGenReg(&ArgStr[2], &Src1)) {
        Boolean      OK;
        tSymbolFlags src2_flags;
        LongWord     Src2
                = EvalStrIntExpressionWithFlags(&ArgStr[3], UInt5, &OK, &src2_flags);
        if (OK) {
            tSymbolFlags src3_flags;
            LongWord     Src3
                    = EvalStrIntExpressionWithFlags(&ArgStr[4], UInt5, &OK, &src3_flags);
            if (OK) {
                tSymbolFlags imm_flags;
                Integer      Imm = EvalStrIntExpressionWithFlags(
                        &ArgStr[5], UInt5, &OK, &imm_flags);
                if (OK) {
                    or_d_guessed(src2_flags, 0, 1, 0x1f << 11);
                    or_d_guessed(src3_flags, 0, 1, 0x1f << 6);
                    or_d_guessed(imm_flags, 0, 1, 0x1f << 1);
                    PutCode((TO_U32(20) << 26) + (Dest << 16) + (Src1 << 21)
                            + (Src2 << 11) + (Src3 << 6) + (Imm << 1) + (LCode << 26)
                            + ExtractPoint(Code));
                    CodeLen = 4;
                }
            }
        }
    }
}

static void DecodeTLBIE(Word Code) {
    LongWord Src1;

    UNUSED(Code);

    if (ChkArgCnt(1, 1) && DecodeGenReg(&ArgStr[1], &Src1)) {
        PutCode((TO_U32(31) << 26) + (Src1 << 11) + (306 << 1));
        CodeLen = 4;
        ChkSup();
    }
}

static void DecodeTW(Word Code) {
    LongWord Src1, Src2;

    UNUSED(Code);

    if (ChkArgCnt(3, 3) && DecodeGenReg(&ArgStr[2], &Src1)
        && DecodeGenReg(&ArgStr[3], &Src2)) {
        Boolean      OK;
        tSymbolFlags dest_flags;
        LongWord     Dest
                = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt5, &OK, &dest_flags);
        if (OK) {
            or_d_guessed(dest_flags, 0, 1, 0x1f << 21);
            PutCode((TO_U32(31) << 26) + (Dest << 21) + (Src1 << 16) + (Src2 << 11)
                    + (4 << 1));
            CodeLen = 4;
        }
    }
}

static void DecodeTWI(Word Code) {
    LongWord Src1;

    UNUSED(Code);

    if (ChkArgCnt(3, 3) && DecodeGenReg(&ArgStr[2], &Src1)) {
        Boolean      OK;
        tSymbolFlags imm_flags;
        Integer Imm = EvalStrIntExpressionWithFlags(&ArgStr[3], Int16, &OK, &imm_flags);
        if (OK) {
            tSymbolFlags dest_flags;
            LongWord     Dest
                    = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt5, &OK, &dest_flags);
            if (OK) {
                or_d_guessed(imm_flags, 0, 1, 0xffff);
                or_d_guessed(dest_flags, 0, 1, 0x1f << 21);
                PutCode((TO_U32(3) << 26) + (Dest << 21) + (Src1 << 16) + (Imm & 0xffff));
                CodeLen = 4;
            }
        }
    }
}

static void DecodeWRTEEI(Word Code) {
    UNUSED(Code);

    if (ChkArgCnt(1, 1)
        && (ChkExactCPUList(ErrNum_InstructionNotSupported, CPU403, CPU403C, CPUNone)
            >= 0)) {
        Boolean      OK;
        tSymbolFlags src1_flags;
        LongWord Src1 = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt1, &OK, &src1_flags)
                        << 15;
        if (OK) {
            or_d_guessed(src1_flags, 0, 1, 1 << 0);
            PutCode((TO_U32(31) << 26) + Src1 + (163 << 1));
            CodeLen = 4;
        }
    }
}

static void DecodeCMP_CMPL(Word Code) {
    LongWord        Src1, Src2, Dest, LCode = Code, dest_guess_mask;
    tStrComp const *pArg4 = (ArgCnt == 3) ? &ArgStr[3] : &ArgStr[4],
                   *pArg3 = (ArgCnt == 3) ? &ArgStr[2] : &ArgStr[3],
                   *pArg2 = (ArgCnt == 3) ? &ZeroComp : &ArgStr[2];

    if (!ChkArgCnt(3, 4))
        ;
    else if (!DecodeGenReg(pArg4, &Src2))
        ;
    else if (!DecodeGenReg(pArg3, &Src1))
        ;
    else if (!DecodeCondReg(&ArgStr[1], &Dest, &dest_guess_mask))
        ;
    else if (Dest & 3) {
        WrStrErrorPos(ErrNum_AddrMustBeAligned, &ArgStr[1]);
    } else {
        Boolean      OK;
        tSymbolFlags src3_flags;
        LongWord     Src3 = EvalStrIntExpressionWithFlags(pArg2, UInt1, &OK, &src3_flags);
        if (OK) {
            or_d_guessed(src3_flags, 0, 1, 1 << 21);
            or_dasmcode_guessed(0, 1, dest_guess_mask << 21);
            PutCode((TO_U32(31) << 26) + (Dest << 21) + (Src3 << 21) + (Src1 << 16)
                    + (Src2 << 11) + (LCode << 1));
            CodeLen = 4;
        }
    }
}

/* Vergleiche */

static void DecodeFCMPO_FCMPU(Word Code) {
    LongWord Src1, Src2, Dest, LCode = Code, dest_guess_mask;

    if (!ChkArgCnt(3, 3))
        ;
    else if (!DecodeFPReg(&ArgStr[3], &Src2))
        ;
    else if (!DecodeFPReg(&ArgStr[2], &Src1))
        ;
    else if (!DecodeCondReg(&ArgStr[1], &Dest, &dest_guess_mask))
        ;
    else if (Dest & 3) {
        WrStrErrorPos(ErrNum_AddrMustBeAligned, &ArgStr[1]);
    } else {
        or_dasmcode_guessed(0, 1, dest_guess_mask << 21);
        PutCode((TO_U32(63) << 26) + (Dest << 21) + (Src1 << 16) + (Src2 << 11)
                + (LCode << 1));
        CodeLen = 4;
    }
}

static void DecodeCMPI_CMPLI(Word Code) {
    LongWord        Src1, Src3, Dest, LCode = Code, dest_guess_mask;
    tStrComp const *pArg4 = (ArgCnt == 3) ? &ArgStr[3] : &ArgStr[4],
                   *pArg3 = (ArgCnt == 3) ? &ArgStr[2] : &ArgStr[3],
                   *pArg2 = (ArgCnt == 3) ? &ZeroComp : &ArgStr[2];

    if (ChkArgCnt(3, 4)) {
        Boolean      OK;
        tSymbolFlags src2_flags;
        LongWord     Src2 = EvalStrIntExpressionWithFlags(pArg4, Int16, &OK, &src2_flags);
        if (OK) {
            if (!DecodeGenReg(pArg3, &Src1))
                ;
            else if (!DecodeCondReg(&ArgStr[1], &Dest, &dest_guess_mask))
                ;
            else if (Dest & 3) {
                WrStrErrorPos(ErrNum_AddrMustBeAligned, &ArgStr[1]);
            } else {
                tSymbolFlags src3_flags;
                Src3 = EvalStrIntExpressionWithFlags(pArg2, UInt1, &OK, &src3_flags);
                if (OK) {
                    or_d_guessed(src2_flags, 0, 1, 0xffff);
                    or_d_guessed(src3_flags, 0, 1, 1 << 21);
                    or_dasmcode_guessed(0, 1, dest_guess_mask << 21);
                    PutCode((TO_U32(10) << 26) + (Dest << 21) + (Src3 << 21)
                            + (Src1 << 16) + (Src2 & 0xffff) + (LCode << 26));
                    CodeLen = 4;
                }
            }
        }
    }
}

/* Spruenge */

static void report_branch_constraint(
        LongInt value, LongInt minimum, LongInt maximum, Word code,
        tErrorNum error, tStrComp const* arg) {
    char detail[240];
    Boolean absolute = !!(code & 2);
    if (error == ErrNum_DistIsOdd) {
        as_snprintf(detail, sizeof(detail),
                getmessage(absolute ? Num_ErrMsgBranchAddressAlignment
                                    : Num_ErrMsgBranchDisplacementAlignment),
                (LargeInt)value);
    } else {
        as_snprintf(detail, sizeof(detail),
                getmessage(absolute ? Num_ErrMsgBranchAddressRange
                                    : Num_ErrMsgBranchFieldRange),
                (LargeInt)value, (LargeInt)minimum, (LargeInt)maximum);
    }
    WrXErrorPos(error, detail, &arg->Pos);
}

static void DecodeB_BL_BA_BLA(Word Code) {
    LongWord     LCode = Code;
    LongInt      Dist;
    Boolean      OK;
    tSymbolFlags Flags;

    if (ChkArgCnt(1, 1)) {
        Dist = EvalStrIntExpressionWithFlags(&ArgStr[1], Int32, &OK, &Flags);
        if (OK) {
            if (!(Code & 2)) {
                Dist -= EProgCounter();
            }
            if (!mFirstPassUnknownOrQuestionable(Flags) && (Dist > 0x1ffffff)) {
                report_branch_constraint(Dist, -0x2000000l, 0x1fffffcl, Code,
                        ErrNum_OverRange, &ArgStr[1]);
            } else if (!mFirstPassUnknownOrQuestionable(Flags) && (Dist < -0x2000000l)) {
                report_branch_constraint(Dist, -0x2000000l, 0x1fffffcl, Code,
                        ErrNum_UnderRange, &ArgStr[1]);
            } else if (((Dist & 3) != 0) && !mFirstPassUnknownOrQuestionable(Flags)) {
                report_branch_constraint(Dist, 0, 0, Code, ErrNum_DistIsOdd, &ArgStr[1]);
            } else {
                or_d_guessed(Flags, 0, 1, 0x03fffffc);
                PutCode((TO_U32(18) << 26) + (Dist & 0x03fffffc) + LCode);
                CodeLen = 4;
            }
        }
    }
}

static void DecodeBC_BCL_BCA_BCLA(Word Code) {
    LongWord LCode = Code;

    if (ChkArgCnt(3, 3)) {
        Boolean      OK;
        tSymbolFlags src1_flags;
        LongWord     Src1 = EvalStrIntExpressionWithFlags(
                &ArgStr[1], UInt5, &OK, &src1_flags); /* BO */
        if (OK) {
            tSymbolFlags src2_flags;
            LongWord     Src2 = EvalStrIntExpressionWithFlags(
                    &ArgStr[2], UInt5, &OK, &src2_flags); /* BI */
            if (OK) {
                tSymbolFlags Flags;
                LongInt      Dist = EvalStrIntExpressionWithFlags(
                        &ArgStr[3], Int32, &OK, &Flags); /* ADR */
                if (OK) {
                    if (!(Code & 2)) {
                        Dist -= EProgCounter();
                    }
                    if (!mFirstPassUnknownOrQuestionable(Flags) && (Dist > 0x7fff)) {
                        report_branch_constraint(Dist, -0x8000l, 0x7ffcl, Code,
                                ErrNum_OverRange, &ArgStr[3]);
                    } else if (!mFirstPassUnknownOrQuestionable(Flags) && (Dist < -0x8000l)) {
                        report_branch_constraint(Dist, -0x8000l, 0x7ffcl, Code,
                                ErrNum_UnderRange, &ArgStr[3]);
                    } else if (((Dist & 3) != 0) && !mFirstPassUnknownOrQuestionable(Flags)) {
                        report_branch_constraint(Dist, 0, 0, Code, ErrNum_DistIsOdd, &ArgStr[3]);
                    } else {
                        or_d_guessed(src1_flags, 0, 1, 0x1f << 21);
                        or_d_guessed(src2_flags, 0, 1, 0x1f << 16);
                        or_d_guessed(Flags, 0, 1, 0xfffc);
                        PutCode((TO_U32(16) << 26) + (Src1 << 21) + (Src2 << 16)
                                + (Dist & 0xfffc) + LCode);
                        CodeLen = 4;
                    }
                }
            }
        }
    }
}

static void DecodeBCCTR_BCCTRL_BCLR_BCLRL(Word Code) {
    LongWord LCode = Code;

    if (ChkArgCnt(2, 2)) {
        Boolean      OK;
        tSymbolFlags src1_flags;
        LongWord     Src1
                = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt5, &OK, &src1_flags);
        if (OK) {
            tSymbolFlags src2_flags;
            LongWord     Src2
                    = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt5, &OK, &src2_flags);
            if (OK) {
                or_d_guessed(src1_flags, 0, 1, 0x1f << 21);
                or_d_guessed(src2_flags, 0, 1, 0x1f << 16);
                PutCode((TO_U32(19) << 26) + (Src1 << 21) + (Src2 << 16) + LCode);
                CodeLen = 4;
            }
        }
    }
}

static void DecodeTLBRE_TLBWE(Word Code) {
    LongWord Src1, Src2, LCode = Code;

    if (ChkArgCnt(3, 3) && ChkExactCPU(CPU403C) && DecodeGenReg(&ArgStr[1], &Src1)
        && DecodeGenReg(&ArgStr[2], &Src2)) {
        Boolean      OK;
        tSymbolFlags src3_flags;
        LongWord     Src3
                = EvalStrIntExpressionWithFlags(&ArgStr[3], UInt1, &OK, &src3_flags);
        if (OK) {
            or_d_guessed(src3_flags, 0, 1, 1 << 11);
            PutCode((TO_U32(31) << 26) + (Src1 << 21) + (Src2 << 16) + (Src3 << 11)
                    + (946 << 1) + (LCode << 1));
            CodeLen = 4;
        }
    }
}

/*-------------------------------------------------------------------------*/

static void AddFixed(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(FixedOrders, BaseOrder);
    FixedOrders[InstrZ].Code    = NCode;
    FixedOrders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, MomCPU == CPU6000 ? NName2 : NName1, InstrZ++, DecodeFixed);
}

static void AddReg1(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Reg1Orders, BaseOrder);
    Reg1Orders[InstrZ].Code    = NCode;
    Reg1Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, MomCPU == CPU6000 ? NName2 : NName1, InstrZ++, DecodeReg1);
}

static void AddCReg1(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(CReg1Orders, BaseOrder);
    CReg1Orders[InstrZ].Code    = NCode;
    CReg1Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, MomCPU == CPU6000 ? NName2 : NName1, InstrZ++, DecodeCReg1);
}

static void AddCBit1(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(CBit1Orders, BaseOrder);
    CBit1Orders[InstrZ].Code    = NCode;
    CBit1Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, MomCPU == CPU6000 ? NName2 : NName1, InstrZ++, DecodeCBit1);
}

static void AddFReg1(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(FReg1Orders, BaseOrder);
    FReg1Orders[InstrZ].Code    = NCode;
    FReg1Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, MomCPU == CPU6000 ? NName2 : NName1, InstrZ++, DecodeFReg1);
}

static void AddSReg2(char const* NName, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Reg2Orders, BaseOrder);
    Reg2Orders[InstrZ].Code    = NCode;
    Reg2Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeReg2);
}

static void AddReg2(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask,
        Boolean WithOE, Boolean WithFL) {
    String      NName;
    char const* pSrcName = (MomCPU == CPU6000) ? NName2 : NName1;

    AddSReg2(pSrcName, NCode, NMask);
    if (WithOE) {
        as_snprintf(NName, sizeof(NName), "%sO", pSrcName);
        AddSReg2(NName, NCode | 0x400, NMask);
    }
    if (WithFL) {
        as_snprintf(NName, sizeof(NName), "%s.", pSrcName);
        AddSReg2(NName, NCode | 0x001, NMask);
        if (WithOE) {
            as_snprintf(NName, sizeof(NName), "%sO.", pSrcName);
            AddSReg2(NName, NCode | 0x401, NMask);
        }
    }
}

static void AddCReg2(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(CReg2Orders, BaseOrder);
    CReg2Orders[InstrZ].Code    = NCode;
    CReg2Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, (MomCPU == CPU6000) ? NName2 : NName1, InstrZ++, DecodeCReg2);
}

static void AddSFReg2(char const* NName, LongWord NCode, Byte NMask) {
    order_array_rsv_end(FReg2Orders, BaseOrder);
    if (!NName) {
        exit(255);
    }
    FReg2Orders[InstrZ].Code    = NCode;
    FReg2Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeFReg2);
}

static void AddFReg2(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask,
        Boolean WithFL) {
    char const* pSrcName = (MomCPU == CPU6000) ? NName2 : NName1;

    AddSFReg2(pSrcName, NCode, NMask);
    if (WithFL) {
        String NName;

        as_snprintf(NName, sizeof(NName), "%s.", pSrcName);
        AddSFReg2(NName, NCode | 0x001, NMask);
    }
}

static void AddReg2B(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Reg2BOrders, BaseOrder);
    Reg2BOrders[InstrZ].Code    = NCode;
    Reg2BOrders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, (MomCPU == CPU6000) ? NName2 : NName1, InstrZ++, DecodeReg2B);
}

static void AddSReg2Swap(char const* NName, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Reg2SwapOrders, BaseOrder);
    if (!NName) {
        exit(255);
    }
    Reg2SwapOrders[InstrZ].Code    = NCode;
    Reg2SwapOrders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeReg2Swap);
}

static void AddReg2Swap(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask,
        Boolean WithOE, Boolean WithFL) {
    String      NName;
    char const* pSrcName = (MomCPU == CPU6000) ? NName2 : NName1;

    AddSReg2Swap(pSrcName, NCode, NMask);
    if (WithOE) {
        as_snprintf(NName, sizeof(NName), "%sO", pSrcName);
        AddSReg2Swap(NName, NCode | 0x400, NMask);
    }
    if (WithFL) {
        as_snprintf(NName, sizeof(NName), "%s.", pSrcName);
        AddSReg2Swap(NName, NCode | 0x001, NMask);
        if (WithOE) {
            as_snprintf(NName, sizeof(NName), "%sO.", pSrcName);
            AddSReg2Swap(NName, NCode | 0x401, NMask);
        }
    }
}

static void AddNoDest(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(NoDestOrders, BaseOrder);
    NoDestOrders[InstrZ].Code    = NCode;
    NoDestOrders[InstrZ].CPUMask = NMask;
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? NName2 : NName1, InstrZ++, DecodeNoDest);
}

static void AddSReg3(char const* NName, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Reg3Orders, BaseOrder);
    Reg3Orders[InstrZ].Code    = NCode;
    Reg3Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeReg3);
}

static void AddReg3(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask,
        Boolean WithOE, Boolean WithFL) {
    String      NName;
    char const* pSrcName = (MomCPU == CPU6000) ? NName2 : NName1;

    AddSReg3(pSrcName, NCode, NMask);
    if (WithOE) {
        as_snprintf(NName, sizeof(NName), "%sO", pSrcName);
        AddSReg3(NName, NCode | 0x400, NMask);
        NName[strlen(NName) - 1] = '\0';
    }
    if (WithFL) {
        as_snprintf(NName, sizeof(NName), "%s.", pSrcName);
        AddSReg3(NName, NCode | 0x001, NMask);
        NName[strlen(NName) - 1] = '\0';
        if (WithOE) {
            as_snprintf(NName, sizeof(NName), "%sO.", pSrcName);
            AddSReg3(NName, NCode | 0x401, NMask);
        }
    }
}

static void AddCReg3(char const* NName, LongWord NCode, CPUVar NMask) {
    order_array_rsv_end(CReg3Orders, BaseOrder);
    CReg3Orders[InstrZ].Code    = NCode;
    CReg3Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeCReg3);
}

static void AddSFReg3(char const* NName, LongWord NCode, Byte NMask) {
    order_array_rsv_end(FReg3Orders, BaseOrder);
    FReg3Orders[InstrZ].Code    = NCode;
    FReg3Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeFReg3);
}

static void AddFReg3(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask,
        Boolean WithFL) {
    String      NName;
    char const* pSrcName = (MomCPU == CPU6000) ? NName2 : NName1;

    AddSFReg3(pSrcName, NCode, NMask);
    if (WithFL) {
        as_snprintf(NName, sizeof(NName), "%s.", pSrcName);
        AddSFReg3(NName, NCode | 0x001, NMask);
    }
}

static void AddSReg3Swap(char const* NName, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Reg3SwapOrders, BaseOrder);
    Reg3SwapOrders[InstrZ].Code    = NCode;
    Reg3SwapOrders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeReg3Swap);
}

static void AddReg3Swap(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask,
        Boolean WithFL) {
    String      NName;
    char const* pSrcName = (MomCPU == CPU6000) ? NName2 : NName1;

    AddSReg3Swap(pSrcName, NCode, NMask);
    if (WithFL) {
        as_snprintf(NName, sizeof(NName), "%s.", pSrcName);
        AddSReg3Swap(NName, NCode | 0x001, NMask);
    }
}

static void AddMixed(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(MixedOrders, BaseOrder);
    MixedOrders[InstrZ].Code    = NCode;
    MixedOrders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, (MomCPU == CPU6000) ? NName2 : NName1, InstrZ++, DecodeMixed);
}

static void AddSFReg4(char const* NName, LongWord NCode, Byte NMask) {
    order_array_rsv_end(FReg4Orders, BaseOrder);
    FReg4Orders[InstrZ].Code    = NCode;
    FReg4Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeFReg4);
}

static void AddFReg4(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask,
        Boolean WithFL) {
    String      NName;
    char const* pSrcName = (MomCPU == CPU6000) ? NName2 : NName1;

    AddSFReg4(pSrcName, NCode, NMask);
    if (WithFL) {
        as_snprintf(NName, sizeof(NName), "%s.", pSrcName);
        AddSFReg4(NName, NCode | 0x001, NMask);
    }
}

static void AddRegDisp(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(RegDispOrders, BaseOrder);
    RegDispOrders[InstrZ].Code    = NCode;
    RegDispOrders[InstrZ].CPUMask = NMask;
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? NName2 : NName1, InstrZ++,
            DecodeRegDispOrder);
}

static void AddFRegDisp(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(FRegDispOrders, BaseOrder);
    FRegDispOrders[InstrZ].Name    = (MomCPU == CPU6000) ? NName2 : NName1;
    FRegDispOrders[InstrZ].Code    = NCode;
    FRegDispOrders[InstrZ].CPUMask = NMask;
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? NName2 : NName1, InstrZ++, DecodeFRegDisp);
}

static void AddSReg2Imm(char const* NName, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Reg2ImmOrders, BaseOrder);
    if (!NName) {
        exit(255);
    }
    Reg2ImmOrders[InstrZ].Code    = NCode;
    Reg2ImmOrders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, NName, InstrZ++, DecodeReg2Imm);
}

static void AddReg2Imm(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask,
        Boolean WithFL) {
    String      NName;
    char const* pSrcName = (MomCPU == CPU6000) ? NName2 : NName1;

    AddSReg2Imm(pSrcName, NCode, NMask);
    if (WithFL) {
        as_snprintf(NName, sizeof(NName), "%s.", pSrcName);
        AddSReg2Imm(NName, NCode | 0x001, NMask);
    }
}

static void AddImm16(char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Imm16Orders, BaseOrder);
    Imm16Orders[InstrZ].Code    = NCode;
    Imm16Orders[InstrZ].CPUMask = NMask;
    AddInstTable(InstTable, (MomCPU == CPU6000) ? NName2 : NName1, InstrZ++, DecodeImm16);
}

static void AddImm16Swap(
        char const* NName1, char const* NName2, LongWord NCode, Byte NMask) {
    order_array_rsv_end(Imm16SwapOrders, BaseOrder);
    Imm16SwapOrders[InstrZ].Code    = NCode;
    Imm16SwapOrders[InstrZ].CPUMask = NMask;
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? NName2 : NName1, InstrZ++, DecodeImm16Swap);
}

static void AddPoint(char const* pName, Word Code, InstProc Proc) {
    char PointName[30];

    AddInstTable(InstTable, pName, Code, Proc);
    as_snprintf(PointName, sizeof(PointName), "%s.", pName);
    AddInstTable(InstTable, PointName, Code | 0x8000, Proc);
}

static void InitFields(void) {
    InstTable = CreateInstTable(407);
    SetDynamicInstTable(InstTable);

    AddPoint("FMULS", 59, DecodeFMUL_FMULS);
    AddPoint((MomCPU == CPU6000) ? "FM" : "FMUL", 63, DecodeFMUL_FMULS);
    AddInstTable(InstTable, (MomCPU == CPU6000) ? "LSI" : "LSWI", 597, DecodeLSWI_STSWI);
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? "STSI" : "STSWI", 597 + 128,
            DecodeLSWI_STSWI);
    AddInstTable(InstTable, "MFTB", 371, DecodeMTFB_MTTB);
    AddInstTable(InstTable, "MFTBU", 371, DecodeMTFB_MTTB);
    AddInstTable(InstTable, "MFTBL", 371, DecodeMTFB_MTTB);
    AddInstTable(InstTable, "MTTB", 467, DecodeMTFB_MTTB);
    AddInstTable(InstTable, "MTTBU", 467, DecodeMTFB_MTTB);
    AddInstTable(InstTable, "MTTBL", 467, DecodeMTFB_MTTB);
    AddInstTable(InstTable, "MFSPR", 339, DecodeMFSPR_MTSPR);
    AddInstTable(InstTable, "MTSPR", 467, DecodeMFSPR_MTSPR);
    AddInstTable(InstTable, "MFDCR", 323, DecodeMFDCR_MTDCR);
    AddInstTable(InstTable, "MTDCR", 451, DecodeMFDCR_MTDCR);
    AddInstTable(InstTable, "MFSR", 595, DecodeMFSR_MTSR);
    AddInstTable(InstTable, "MTSR", 210, DecodeMFSR_MTSR);
    AddInstTable(InstTable, "MTCRF", 0, DecodeMTCRF);
    AddPoint("MTFSF", 0, DecodeMTFSF);
    AddPoint("MTFSFI", 0, DecodeMTFSFI);
    AddPoint("RLMI", 0, DecodeRLMI);
    AddPoint((MomCPU == CPU6000) ? "RLNM" : "RLWNM", 0, DecodeRLWNM);
    AddPoint((MomCPU == CPU6000) ? "RLIMI" : "RLWIMI", 0, DecodeRLWIMI_RLWINM);
    AddPoint((MomCPU == CPU6000) ? "RLINM" : "RLWINM", 1, DecodeRLWIMI_RLWINM);
    AddInstTable(InstTable, (MomCPU == CPU6000) ? "TLBI" : "TLBIE", 0, DecodeTLBIE);
    AddInstTable(InstTable, (MomCPU == CPU6000) ? "T" : "TW", 0, DecodeTW);
    AddInstTable(InstTable, (MomCPU == CPU6000) ? "TI" : "TWI", 0, DecodeTWI);
    AddInstTable(InstTable, "WRTEEI", 0, DecodeWRTEEI);
    AddInstTable(InstTable, "CMP", 0, DecodeCMP_CMPL);
    AddInstTable(InstTable, "CMPL", 32, DecodeCMP_CMPL);
    AddInstTable(InstTable, "FCMPO", 32, DecodeFCMPO_FCMPU);
    AddInstTable(InstTable, "FCMPU", 0, DecodeFCMPO_FCMPU);
    AddInstTable(InstTable, "CMPI", 1, DecodeCMPI_CMPLI);
    AddInstTable(InstTable, "CMPLI", 0, DecodeCMPI_CMPLI);
    AddInstTable(InstTable, "B", 0, DecodeB_BL_BA_BLA);
    AddInstTable(InstTable, "BL", 1, DecodeB_BL_BA_BLA);
    AddInstTable(InstTable, "BA", 2, DecodeB_BL_BA_BLA);
    AddInstTable(InstTable, "BLA", 3, DecodeB_BL_BA_BLA);
    AddInstTable(InstTable, "BC", 0, DecodeBC_BCL_BCA_BCLA);
    AddInstTable(InstTable, "BCL", 1, DecodeBC_BCL_BCA_BCLA);
    AddInstTable(InstTable, "BCA", 2, DecodeBC_BCL_BCA_BCLA);
    AddInstTable(InstTable, "BCLA", 3, DecodeBC_BCL_BCA_BCLA);
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? "BCC" : "BCCTR", 528 << 1,
            DecodeBCCTR_BCCTRL_BCLR_BCLRL);
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? "BCCL" : "BCCTRL", (528 << 1) | 1,
            DecodeBCCTR_BCCTRL_BCLR_BCLRL);
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? "BCR" : "BCLR", 16 << 1,
            DecodeBCCTR_BCCTRL_BCLR_BCLRL);
    AddInstTable(
            InstTable, (MomCPU == CPU6000) ? "BCRL" : "BCLRL", (16 << 1) | 1,
            DecodeBCCTR_BCCTRL_BCLR_BCLRL);
    AddInstTable(InstTable, "TLBRE", 0, DecodeTLBRE_TLBWE);
    AddInstTable(InstTable, "TLBWE", 32, DecodeTLBRE_TLBWE);
    AddInstTable(InstTable, "REG", 0, CodeREG);

    /* --> 0 0 0 */

    InstrZ = 0;
    AddFixed(
            "EIEIO", "EIEIO", (TO_U32(31) << 26) + (854 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddFixed(
            "ISYNC", "ICS", (TO_U32(19) << 26) + (150 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddFixed(
            "RFI", "RFI", (TO_U32(19) << 26) + (50 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000 | M_SUP);
    AddFixed(
            "SC", "SVCA", (TO_U32(17) << 26) + (1 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddFixed(
            "SYNC", "DCS", (TO_U32(31) << 26) + (598 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddFixed("RFCI", "RFCI", (TO_U32(19) << 26) + (51 << 1), M_403 | M_403C);
    AddFixed("TLBIA", "TLBIA", (TO_U32(31) << 26) + (370 << 1), M_403C | M_821);
    AddFixed("TLBSYNC", "TLBSYNC", (TO_U32(31) << 26) + (566 << 1), M_403C | M_821);

    /* D --> D 0 0 */

    InstrZ = 0;
    AddReg1("MFCR", "MFCR", (TO_U32(31) << 26) + (19 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddReg1("MFMSR", "MFMSR", (TO_U32(31) << 26) + (83 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddReg1("MTMSR", "MTMSR", (TO_U32(31) << 26) + (146 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000 | M_SUP);
    AddReg1("WRTEE", "WRTEE", (TO_U32(31) << 26) + (131 << 1),
            M_403 | M_403C | M_505 | M_601 | M_6000);

    /* crD --> D 0 0 */

    InstrZ = 0;
    AddCReg1(
            "MCRXR", "MCRXR", (TO_U32(31) << 26) + (512 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);

    /* crbD --> D 0 0 */

    InstrZ = 0;
    AddCBit1("MTFSB0", "MTFSB0", (TO_U32(63) << 26) + (70 << 1), M_601 | M_6000);
    AddCBit1("MTFSB0.", "MTFSB0.", (TO_U32(63) << 26) + (70 << 1) + 1, M_601 | M_6000);
    AddCBit1("MTFSB1", "MTFSB1", (TO_U32(63) << 26) + (38 << 1), M_601 | M_6000);
    AddCBit1("MTFSB1.", "MTFSB1.", (TO_U32(63) << 26) + (38 << 1) + 1, M_601 | M_6000);

    /* frD --> D 0 0 */

    InstrZ = 0;
    AddFReg1("MFFS", "MFFS", (TO_U32(63) << 26) + (583 << 1), M_601 | M_6000);
    AddFReg1("MFFS.", "MFFS.", (TO_U32(63) << 26) + (583 << 1) + 1, M_601 | M_6000);

    /* D,A --> D A 0 */

    InstrZ = 0;
    AddReg2("ABS", "ABS", (TO_U32(31) << 26) + (360 << 1), M_6000, True, True);
    AddReg2("ADDME", "AME", (TO_U32(31) << 26) + (234 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg2("ADDZE", "AZE", (TO_U32(31) << 26) + (202 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg2("CLCS", "CLCS", (TO_U32(31) << 26) + (531 << 1), M_6000, False, False);
    AddReg2("NABS", "NABS", (TO_U32(31) << 26) + (488 << 1), M_6000, True, True);
    AddReg2("NEG", "NEG", (TO_U32(31) << 26) + (104 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg2("SUBFME", "SFME", (TO_U32(31) << 26) + (232 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg2("SUBFZE", "SFZE", (TO_U32(31) << 26) + (200 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);

    /* cD,cS --> D S 0 */

    InstrZ = 0;
    AddCReg2(
            "MCRF", "MCRF", (TO_U32(19) << 26) + (0 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddCReg2("MCRFS", "MCRFS", (TO_U32(63) << 26) + (64 << 1), M_601 | M_6000);

    /* fD,fB --> D 0 B */

    InstrZ = 0;
    AddFReg2("FABS", "FABS", (TO_U32(63) << 26) + (264 << 1), M_601 | M_6000, True);
    AddFReg2("FCTIW", "FCTIW", (TO_U32(63) << 26) + (14 << 1), M_601 | M_6000, True);
    AddFReg2("FCTIWZ", "FCTIWZ", (TO_U32(63) << 26) + (15 << 1), M_601 | M_6000, True);
    AddFReg2("FMR", "FMR", (TO_U32(63) << 26) + (72 << 1), M_601 | M_6000, True);
    AddFReg2("FNABS", "FNABS", (TO_U32(63) << 26) + (136 << 1), M_601 | M_6000, True);
    AddFReg2("FNEG", "FNEG", (TO_U32(63) << 26) + (40 << 1), M_601 | M_6000, True);
    AddFReg2("FRSP", "FRSP", (TO_U32(63) << 26) + (12 << 1), M_601 | M_6000, True);

    /* D,B --> D 0 B */

    InstrZ = 0;
    AddReg2B("MFSRIN", "MFSRIN", (TO_U32(31) << 26) + (659 << 1), M_601 | M_6000);
    AddReg2B("MTSRIN", "MTSRI", (TO_U32(31) << 26) + (242 << 1), M_601 | M_6000);

    /* A,S --> S A 0 */

    InstrZ = 0;
    AddReg2Swap(
            "CNTLZW", "CNTLZ", (TO_U32(31) << 26) + (26 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, True);
    AddReg2Swap(
            "EXTSB ", "EXTSB", (TO_U32(31) << 26) + (954 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, True);
    AddReg2Swap(
            "EXTSH ", "EXTS", (TO_U32(31) << 26) + (922 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, True);

    /* A,B --> 0 A B */

    InstrZ = 0;
    AddNoDest(
            "DCBF", "DCBF", (TO_U32(31) << 26) + (86 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddNoDest(
            "DCBI", "DCBI", (TO_U32(31) << 26) + (470 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddNoDest(
            "DCBST", "DCBST", (TO_U32(31) << 26) + (54 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddNoDest(
            "DCBT", "DCBT", (TO_U32(31) << 26) + (278 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddNoDest(
            "DCBTST", "DCBTST", (TO_U32(31) << 26) + (246 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddNoDest(
            "DCBZ", "DCLZ", (TO_U32(31) << 26) + (1014 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddNoDest("DCCCI", "DCCCI", (TO_U32(31) << 26) + (454 << 1), M_403 | M_403C);
    AddNoDest(
            "ICBI", "ICBI", (TO_U32(31) << 26) + (982 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddNoDest("ICBT", "ICBT", (TO_U32(31) << 26) + (262 << 1), M_403 | M_403C);
    AddNoDest("ICCCI", "ICCCI", (TO_U32(31) << 26) + (966 << 1), M_403 | M_403C);

    /* D,A,B --> D A B */

    InstrZ = 0;
    AddReg3("ADD", "CAX", (TO_U32(31) << 26) + (266 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("ADDC", "A", (TO_U32(31) << 26) + (10 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("ADDE", "AE", (TO_U32(31) << 26) + (138 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("DIV", "DIV", (TO_U32(31) << 26) + (331 << 1), M_6000, True, True);
    AddReg3("DIVS", "DIVS", (TO_U32(31) << 26) + (363 << 1), M_6000, True, True);
    AddReg3("DIVW", "DIVW", (TO_U32(31) << 26) + (491 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("DIVWU", "DIVWU", (TO_U32(31) << 26) + (459 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("DOZ", "DOZ", (TO_U32(31) << 26) + (264 << 1), M_6000, True, True);
    AddReg3("ECIWX", "ECIWX", (TO_U32(31) << 26) + (310 << 1), M_821 | M_6000, False,
            False);
    AddReg3("LBZUX", "LBZUX", (TO_U32(31) << 26) + (119 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LBZX", "LBZX", (TO_U32(31) << 26) + (87 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LHAUX", "LHAUX", (TO_U32(31) << 26) + (375 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LHAX", "LHAX", (TO_U32(31) << 26) + (343 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LHBRX", "LHBRX", (TO_U32(31) << 26) + (790 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LHZUX", "LHZUX", (TO_U32(31) << 26) + (311 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LHZX", "LHZX", (TO_U32(31) << 26) + (279 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LSCBX", "LSCBX", (TO_U32(31) << 26) + (277 << 1), M_6000, False, True);
    AddReg3("LSWX", "LSX", (TO_U32(31) << 26) + (533 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LWARX", "LWARX", (TO_U32(31) << 26) + (20 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LWBRX", "LBRX", (TO_U32(31) << 26) + (534 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LWZUX", "LUX", (TO_U32(31) << 26) + (55 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("LWZX", "LX", (TO_U32(31) << 26) + (23 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("MUL", "MUL", (TO_U32(31) << 26) + (107 << 1), M_6000, True, True);
    AddReg3("MULHW", "MULHW", (TO_U32(31) << 26) + (75 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, True);
    AddReg3("MULHWU", "MULHWU", (TO_U32(31) << 26) + (11 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, True);
    AddReg3("MULLW", "MULS", (TO_U32(31) << 26) + (235 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("STBUX", "STBUX", (TO_U32(31) << 26) + (247 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STBX", "STBX", (TO_U32(31) << 26) + (215 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STHBRX", "STHBRX", (TO_U32(31) << 26) + (918 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STHUX", "STHUX", (TO_U32(31) << 26) + (439 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STHX", "STHX", (TO_U32(31) << 26) + (407 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STSWX", "STSX", (TO_U32(31) << 26) + (661 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STWBRX", "STBRX", (TO_U32(31) << 26) + (662 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STWCX.", "STWCX.", (TO_U32(31) << 26) + (150 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STWUX", "STUX", (TO_U32(31) << 26) + (183 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("STWX", "STX", (TO_U32(31) << 26) + (151 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, False, False);
    AddReg3("SUBF", "SUBF", (TO_U32(31) << 26) + (40 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("SUB", "SUB", (TO_U32(31) << 26) + (40 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("SUBFC", "SF", (TO_U32(31) << 26) + (8 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("SUBC", "SUBC", (TO_U32(31) << 26) + (8 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("SUBFE", "SFE", (TO_U32(31) << 26) + (136 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True, True);
    AddReg3("TLBSX", "TLBSX", (TO_U32(31) << 26) + (914 << 1), M_403C, False, True);

    /* cD,cA,cB --> D A B */

    InstrZ = 0;
    AddCReg3(
            "CRAND", (TO_U32(19) << 26) + (257 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddCReg3(
            "CRANDC", (TO_U32(19) << 26) + (129 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddCReg3(
            "CREQV", (TO_U32(19) << 26) + (289 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddCReg3(
            "CRNAND", (TO_U32(19) << 26) + (225 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddCReg3(
            "CRNOR", (TO_U32(19) << 26) + (33 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddCReg3(
            "CROR", (TO_U32(19) << 26) + (449 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddCReg3(
            "CRORC", (TO_U32(19) << 26) + (417 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddCReg3(
            "CRXOR", (TO_U32(19) << 26) + (193 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);

    /* fD,fA,fB --> D A B */

    InstrZ = 0;
    AddFReg3("FADD", "FA", (TO_U32(63) << 26) + (21 << 1), M_601 | M_6000, True);
    AddFReg3("FADDS", "FADDS", (TO_U32(59) << 26) + (21 << 1), M_601 | M_6000, True);
    AddFReg3("FDIV", "FD", (TO_U32(63) << 26) + (18 << 1), M_601 | M_6000, True);
    AddFReg3("FDIVS", "FDIVS", (TO_U32(59) << 26) + (18 << 1), M_601 | M_6000, True);
    AddFReg3("FSUB", "FS", (TO_U32(63) << 26) + (20 << 1), M_601 | M_6000, True);

    /* A,S,B --> S A B */

    InstrZ = 0;
    AddReg3Swap(
            "AND", "AND", (TO_U32(31) << 26) + (28 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap(
            "ANDC", "ANDC", (TO_U32(31) << 26) + (60 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap(
            "ECOWX", "ECOWX", (TO_U32(31) << 26) + (438 << 1), M_821 | M_601 | M_6000,
            False);
    AddReg3Swap(
            "EQV", "EQV", (TO_U32(31) << 26) + (284 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap("MASKG", "MASKG", (TO_U32(31) << 26) + (29 << 1), M_6000, True);
    AddReg3Swap("MASKIR", "MASKIR", (TO_U32(31) << 26) + (541 << 1), M_6000, True);
    AddReg3Swap(
            "NAND", "NAND", (TO_U32(31) << 26) + (476 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap(
            "NOR", "NOR", (TO_U32(31) << 26) + (124 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap(
            "OR", "OR", (TO_U32(31) << 26) + (444 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap(
            "ORC", "ORC", (TO_U32(31) << 26) + (412 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap("RRIB", "RRIB", (TO_U32(31) << 26) + (537 << 1), M_6000, True);
    AddReg3Swap("SLE", "SLE", (TO_U32(31) << 26) + (153 << 1), M_6000, True);
    AddReg3Swap("SLEQ", "SLEQ", (TO_U32(31) << 26) + (217 << 1), M_6000, True);
    AddReg3Swap("SLLQ", "SLLQ", (TO_U32(31) << 26) + (216 << 1), M_6000, True);
    AddReg3Swap("SLQ", "SLQ", (TO_U32(31) << 26) + (152 << 1), M_6000, True);
    AddReg3Swap(
            "SLW", "SL", (TO_U32(31) << 26) + (24 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap("SRAQ", "SRAQ", (TO_U32(31) << 26) + (920 << 1), M_6000, True);
    AddReg3Swap(
            "SRAW", "SRA", (TO_U32(31) << 26) + (792 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap("SRE", "SRE", (TO_U32(31) << 26) + (665 << 1), M_6000, True);
    AddReg3Swap("SREA", "SREA", (TO_U32(31) << 26) + (921 << 1), M_6000, True);
    AddReg3Swap("SREQ", "SREQ", (TO_U32(31) << 26) + (729 << 1), M_6000, True);
    AddReg3Swap("SRLQ", "SRLQ", (TO_U32(31) << 26) + (728 << 1), M_6000, True);
    AddReg3Swap("SRQ", "SRQ", (TO_U32(31) << 26) + (664 << 1), M_6000, True);
    AddReg3Swap(
            "SRW", "SR", (TO_U32(31) << 26) + (536 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg3Swap(
            "XOR", "XOR", (TO_U32(31) << 26) + (316 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);

    /* fD,A,B --> D A B */

    InstrZ = 0;
    AddMixed("LFDUX", "LFDUX", (TO_U32(31) << 26) + (631 << 1), M_601 | M_6000);
    AddMixed("LFDX", "LFDX", (TO_U32(31) << 26) + (599 << 1), M_601 | M_6000);
    AddMixed("LFSUX", "LFSUX", (TO_U32(31) << 26) + (567 << 1), M_601 | M_6000);
    AddMixed("LFSX", "LFSX", (TO_U32(31) << 26) + (535 << 1), M_601 | M_6000);
    AddMixed("STFDUX", "STFDUX", (TO_U32(31) << 26) + (759 << 1), M_601 | M_6000);
    AddMixed("STFDX", "STFDX", (TO_U32(31) << 26) + (727 << 1), M_601 | M_6000);
    AddMixed("STFSUX", "STFSUX", (TO_U32(31) << 26) + (695 << 1), M_601 | M_6000);
    AddMixed("STFSX", "STFSX", (TO_U32(31) << 26) + (663 << 1), M_601 | M_6000);

    /* fD,fA,fC,fB --> D A B C */

    InstrZ = 0;
    AddFReg4("FMADD", "FMA", (TO_U32(63) << 26) + (29 << 1), M_601 | M_6000, True);
    AddFReg4("FMADDS", "FMADDS", (TO_U32(59) << 26) + (29 << 1), M_601 | M_6000, True);
    AddFReg4("FMSUB", "FMS", (TO_U32(63) << 26) + (28 << 1), M_601 | M_6000, True);
    AddFReg4("FMSUBS", "FMSUBS", (TO_U32(59) << 26) + (28 << 1), M_601 | M_6000, True);
    AddFReg4("FNMADD", "FNMA", (TO_U32(63) << 26) + (31 << 1), M_601 | M_6000, True);
    AddFReg4("FNMADDS", "FNMADDS", (TO_U32(59) << 26) + (31 << 1), M_601 | M_6000, True);
    AddFReg4("FNMSUB", "FNMS", (TO_U32(63) << 26) + (30 << 1), M_601 | M_6000, True);
    AddFReg4("FNMSUBS", "FNMSUBS", (TO_U32(59) << 26) + (30 << 1), M_601 | M_6000, True);

    /* D,d(A) --> D A d */

    InstrZ = 0;
    AddRegDisp(
            "LBZ", "LBZ", (TO_U32(34) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "LBZU", "LBZU", (TO_U32(35) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "LHA", "LHA", (TO_U32(42) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "LHAU", "LHAU", (TO_U32(43) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "LHZ", "LHZ", (TO_U32(40) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "LHZU", "LHZU", (TO_U32(41) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "LMW", "LM", (TO_U32(46) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "LWZ", "L", (TO_U32(32) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "LWZU", "LU", (TO_U32(33) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "STB", "STB", (TO_U32(38) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "STBU", "STBU", (TO_U32(39) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "STH", "STH", (TO_U32(44) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "STHU", "STHU", (TO_U32(45) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "STMW", "STM", (TO_U32(47) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "STW", "ST", (TO_U32(36) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddRegDisp(
            "STWU", "STU", (TO_U32(37) << 26),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);

    /* fD,d(A) --> D A d */

    InstrZ = 0;
    AddFRegDisp("LFD", "LFD", (TO_U32(50) << 26), M_601 | M_6000);
    AddFRegDisp("LFDU", "LFDU", (TO_U32(51) << 26), M_601 | M_6000);
    AddFRegDisp("LFS", "LFS", (TO_U32(48) << 26), M_601 | M_6000);
    AddFRegDisp("LFSU", "LFSU", (TO_U32(49) << 26), M_601 | M_6000);
    AddFRegDisp("STFD", "STFD", (TO_U32(54) << 26), M_601 | M_6000);
    AddFRegDisp("STFDU", "STFDU", (TO_U32(55) << 26), M_601 | M_6000);
    AddFRegDisp("STFS", "STFS", (TO_U32(52) << 26), M_601 | M_6000);
    AddFRegDisp("STFSU", "STFSU", (TO_U32(53) << 26), M_601 | M_6000);

    /* A,S,Imm5 --> S A Imm */

    InstrZ = 0;
    AddReg2Imm("SLIQ", "SLIQ", (TO_U32(31) << 26) + (184 << 1), M_6000, True);
    AddReg2Imm("SLLIQ", "SLLIQ", (TO_U32(31) << 26) + (248 << 1), M_6000, True);
    AddReg2Imm("SRAIQ", "SRAIQ", (TO_U32(31) << 26) + (952 << 1), M_6000, True);
    AddReg2Imm(
            "SRAWI", "SRAI", (TO_U32(31) << 26) + (824 << 1),
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000, True);
    AddReg2Imm("SRIQ", "SRIQ", (TO_U32(31) << 26) + (696 << 1), M_6000, True);
    AddReg2Imm("SRLIQ", "SRLIQ", (TO_U32(31) << 26) + (760 << 1), M_6000, True);

    /* D,A,Imm --> D A Imm */

    InstrZ = 0;
    AddImm16(
            "ADDI", "CAL", TO_U32(14) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16(
            "ADDIC", "AI", TO_U32(12) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16(
            "ADDIC.", "AI.", TO_U32(13) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16(
            "ADDIS", "CAU", TO_U32(15) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16("DOZI", "DOZI", TO_U32(9) << 26, M_6000);
    AddImm16(
            "MULLI", "MULI", TO_U32(7) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16(
            "SUBFIC", "SFI", TO_U32(8) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);

    /* A,S,Imm --> S A Imm */

    InstrZ = 0;
    AddImm16Swap(
            "ANDI.", "ANDIL.", TO_U32(28) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16Swap(
            "ANDIS.", "ANDIU.", TO_U32(29) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16Swap(
            "ORI", "ORIL", TO_U32(24) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16Swap(
            "ORIS", "ORIU", TO_U32(25) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16Swap(
            "XORI", "XORIL", TO_U32(26) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);
    AddImm16Swap(
            "XORIS", "XORIU", TO_U32(27) << 26,
            M_403 | M_403C | M_505 | M_821 | M_601 | M_6000);

    AddIntelPseudo(InstTable, eIntPseudoFlag_DynEndian);
}

static void DeinitFields(void) {
    DestroyInstTable(InstTable);
    order_array_free(FixedOrders);
    order_array_free(Reg1Orders);
    order_array_free(FReg1Orders);
    order_array_free(CReg1Orders);
    order_array_free(CBit1Orders);
    order_array_free(Reg2Orders);
    order_array_free(CReg2Orders);
    order_array_free(FReg2Orders);
    order_array_free(Reg2BOrders);
    order_array_free(Reg2SwapOrders);
    order_array_free(NoDestOrders);
    order_array_free(Reg3Orders);
    order_array_free(CReg3Orders);
    order_array_free(FReg3Orders);
    order_array_free(Reg3SwapOrders);
    order_array_free(MixedOrders);
    order_array_free(FReg4Orders);
    order_array_free(RegDispOrders);
    order_array_free(FRegDispOrders);
    order_array_free(Reg2ImmOrders);
    order_array_free(Imm16Orders);
    order_array_free(Imm16SwapOrders);
}

/*-------------------------------------------------------------------------*/

static void MakeCode_601(void) {
    /* Nullanweisung */

    if (Memo("") && !*AttrPart.str.p_str && (ArgCnt == 0)) {
        return;
    }

    if (!LookupInstTable(InstTable, OpPart.str.p_str)) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
    }
}

static Boolean IsDef_601(void) {
    return Memo("REG");
}

/*!------------------------------------------------------------------------
 * \fn     InternSymbol_601(char *Asc, TempResult *Erg)
 * \brief  handle built.in symbols for PPC
 * \param  Asc source argument
 * \param  Erg result buffer
 * ------------------------------------------------------------------------ */

static void InternSymbol_601(char* Asc, TempResult* Erg) {
    LongWord RegValue;
    int      l = strlen(Asc);

    Erg->Typ = TempNone;
    if (((l == 3) || (l == 4))
        && ((as_toupper(*Asc) == 'C') && (as_toupper(Asc[1]) == 'R'))
        && ((Asc[l - 1] >= '0') && (Asc[l - 1] <= '7'))
        && ((l == 3) != ((as_toupper(Asc[2]) == 'F') || (as_toupper(Asc[3]) == 'B')))) {
        as_tempres_set_int(Erg, Asc[l - 1] - '0');
    } else if (DecodeGenRegCore(Asc, &RegValue)) {
        Erg->Typ                       = TempReg;
        Erg->Contents.RegDescr.Dissect = DissectReg_601;
        Erg->Contents.RegDescr.compare = NULL;
        Erg->Contents.RegDescr.Reg     = RegValue;
        Erg->DataSize                  = eSymbolSize32Bit;
    } else if (DecodeFPRegCore(Asc, &RegValue)) {
        Erg->Typ                       = TempReg;
        Erg->Contents.RegDescr.Dissect = DissectReg_601;
        Erg->Contents.RegDescr.compare = NULL;
        Erg->Contents.RegDescr.Reg     = RegValue;
        Erg->DataSize                  = eSymbolSizeFloat64Bit;
    }
}

static void SwitchTo_601(void) {
    TFamilyDescr const* FoundDscr;

    TurnWords = True;
    SetIntConstMode(eIntConstModeC);

    FoundDscr = FindFamilyByName("MPC601");
    if (!FoundDscr) {
        exit(255);
    }

    PCSymbol    = "*";
    HeaderID    = FoundDscr->Id;
    NOPCode     = 0x000000000;
    DivideChars = ",";
    HasAttrs    = False;

    ValidSegs          = (1 << SegCode);
    Grans[SegCode]     = 1;
    ListGrans[SegCode] = 4;
    SegInits[SegCode]  = 0;
    SegLimits[SegCode] = (LargeWord)IntTypeDefs[UInt32].Max;

    MakeCode     = MakeCode_601;
    IsDef        = IsDef_601;
    SwitchFrom   = DeinitFields;
    InternSymbol = InternSymbol_601;
    DissectReg   = DissectReg_601;
    onoff_supmode_add();
    onoff_bigendian_add();

    InitFields();
}

void code601_init(void) {
    CPU403  = AddCPU("PPC403", SwitchTo_601);
    CPU403C = AddCPU("PPC403GC", SwitchTo_601);
    CPU505  = AddCPU("MPC505", SwitchTo_601);
    CPU601  = AddCPU("MPC601", SwitchTo_601);
    CPU821  = AddCPU("MPC821", SwitchTo_601);
    CPU6000 = AddCPU("RS6000", SwitchTo_601);

    AddCopyright("Motorola MPC821 Additions (C) 2012 Marcin Cieslak");
}
