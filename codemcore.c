/* codemcore.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator MCORE-Familie                                               */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "codemcore.h"

#include "as.rsc"
#include "nlmessages.h"
#include "asmallg.h"
#include "asmdef.h"
#include "asmitree.h"
#include "asmpars.h"
#include "asmsub.h"
#include "as_endian.h"
#include "bpemu.h"
#include "codepseudo.h"
#include "codevars.h"
#include "errmsg.h"
#include "headids.h"
#include "motpseudo.h"
#include "onoff_common.h"
#include "strutil.h"

#include <string.h>

/*--------------------------------------------------------------------------*/
/* Variablen */

#define REG_SP 0
#define REG_LR 15

typedef struct {
    Word    Code;
    Boolean Priv;
} FixedOrder;

typedef struct {
    Word Code;
    Word Min, Ofs;
} ImmOrder;

typedef struct {
    char const* Name;
    Word        Code;
} CReg;

static CPUVar      CPUMCORE;
static tSymbolSize OpSize;

static FixedOrder* FixedOrders;
static FixedOrder* OneRegOrders;
static FixedOrder* TwoRegOrders;
static ImmOrder*   UImm5Orders;
static FixedOrder* LJmpOrders;
static CReg*       CRegs;

/*--------------------------------------------------------------------------*/
/* Hilfsdekoder */

static Word const AllRegMask = 0xffff;

/*!------------------------------------------------------------------------
 * \fn     DecodeRegCore(const char *pArg, Word *pResult)
 * \brief  check whether argument is a register
 * \param  pArg argument
 * \param  pResult register number if it is
 * \return True if it is
 * ------------------------------------------------------------------------ */

static Boolean DecodeRegCore(char const* pArg, Word* pResult) {
    if (!as_strcasecmp(pArg, "SP")) {
        *pResult = REGSYM_FLAG_ALIAS | REG_SP;
    } else if (!as_strcasecmp(pArg, "LR")) {
        *pResult = REGSYM_FLAG_ALIAS | REG_LR;
    } else if (as_toupper(*pArg) != 'R') {
        return False;
    } else {
        char* endptr;

        long value = strtol(pArg + 1, &endptr, 10);
        if ((endptr == pArg + 1) || *endptr || (value < 0) || (value > 15)) {
            return False;
        }
        *pResult = value;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     DissectReg_MCORE(char *pDest, size_t DestSize, tRegInt Value, tSymbolSize InpSize)
 * \brief  dissect register symbols - M-CORE variant
 * \param  pDest destination buffer
 * \param  DestSize destination buffer size
 * \param  Value numeric register value
 * \param  InpSize register size
 * ------------------------------------------------------------------------ */

static void DissectReg_MCORE(
        char* pDest, size_t DestSize, tRegInt Value, tSymbolSize InpSize) {
    switch (InpSize) {
    case eSymbolSize32Bit:
        switch (Value) {
        case REGSYM_FLAG_ALIAS | REG_SP:
            as_snprintf(pDest, DestSize, "SP");
            break;
        case REGSYM_FLAG_ALIAS | REG_LR:
            as_snprintf(pDest, DestSize, "LR");
            break;
        default:
            as_snprintf(pDest, DestSize, "R%u", (unsigned)Value);
        }
        break;
    default:
        as_snprintf(pDest, DestSize, "%d-%u", (int)InpSize, (unsigned)Value);
    }
}

/*!------------------------------------------------------------------------
 * \fn     DecodeReg(const tStrComp *pArg, Word *pResult, Word Mask, Boolean MustBeReg)
 * \brief  check whether argument is a CPU register or register alias
 * \param  pArg argument
 * \param  pResult register number if it is
 * \param  Mask bit mask of allowed registers
 * \param  MustBeReg operand is expected to be a register
 * \return True if it is an allowed register
 * ------------------------------------------------------------------------ */

static void register_choices(char* choices, size_t size, Word mask) {
    unsigned reg;
    choices[0] = '\0';
    for (reg = 0; reg < 16; reg++) {
        if ((mask >> reg) & 1) {
            unsigned last = reg;
            char piece[32];
            while ((last < 15) && ((mask >> (last + 1)) & 1)) {
                last++;
            }
            if (last == reg) {
                as_snprintf(piece, sizeof(piece), "R%u", reg);
            } else {
                as_snprintf(piece, sizeof(piece), "R%u..R%u", reg, last);
            }
            if (*choices) {
                strmaxcat(choices, ", ", size);
            }
            strmaxcat(choices, piece, size);
            reg = last;
        }
    }
}

static tRegEvalResult DecodeReg(
        tStrComp const* pArg, Word* pResult, Word Mask, Boolean MustBeReg) {
    tRegEvalResult RegEvalResult;

    if (DecodeRegCore(pArg->str.p_str, pResult)) {
        RegEvalResult = eIsReg;
    } else {
        tRegDescr   RegDescr;
        tEvalResult EvalResult;

        RegEvalResult = EvalStrRegExpressionAsOperand(
                pArg, &RegDescr, &EvalResult, eSymbolSize32Bit, MustBeReg);
        if (RegEvalResult != eIsReg) {
            return RegEvalResult;
        }
        *pResult = RegDescr.Reg;
        if (!EvalResult.OK && MustBeReg) {
            unsigned reg;
            for (reg = 0; reg < 16; reg++) {
                if ((Mask >> reg) & 1) {
                    *pResult = reg;
                    return eIsReg;
                }
            }
        }
    }
    *pResult &= ~REGSYM_FLAG_ALIAS;
    if ((*pResult > 15) || !(Mask & (1u << *pResult))) {
        char choices[120], detail[220];
        register_choices(choices, sizeof(choices), Mask);
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgAVRRegisterChoices),
                (unsigned)*pResult, choices);
        WrXErrorPos(ErrNum_InvReg, detail, &pArg->Pos);
        return MustBeReg ? eRegAbort : eIsNoReg;
    }
    return RegEvalResult;
}

static Boolean DecodeArgReg(int Index, Word* pErg, Word Mask) {
    return (DecodeReg(&ArgStr[Index], pErg, Mask, True) == eIsReg);
}

static Boolean DecodeArgIReg(int Index, Word* pErg, Word Mask) {
    tStrComp    RegComp;
    char const* pArg = ArgStr[Index].str.p_str;
    int         l    = strlen(pArg);

    if ((l <= 3) || (pArg[0] != '(') || (pArg[l - 1] != ')')) {
        char choices[120], detail[200];
        register_choices(choices, sizeof(choices), Mask);
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgMCoreIndirectRegister), choices);
        WrXErrorPos(ErrNum_InvAddrMode, detail, &ArgStr[Index].Pos);
        return False;
    }
    StrCompRefRight(&RegComp, &ArgStr[Index], 1);
    StrCompShorten(&RegComp, 1);
    return (DecodeReg(&RegComp, pErg, Mask, True) == eIsReg);
}

static Boolean DecodeArgRegPair(
        int Index, Word* pFrom, Word FromMask, Word* pTo, Word ToMask) {
    tStrComp FromComp, ToComp;
    char*    pSep = strchr(ArgStr[Index].str.p_str, '-');

    if (!pSep) {
        char from_choices[120], to_choices[120], detail[320];
        register_choices(from_choices, sizeof(from_choices), FromMask);
        register_choices(to_choices, sizeof(to_choices), ToMask);
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgMCoreRegisterRange),
                from_choices, to_choices);
        WrXErrorPos(ErrNum_InvAddrMode, detail, &ArgStr[Index].Pos);
        return False;
    }

    StrCompSplitRef(&FromComp, &ToComp, &ArgStr[Index], pSep);
    return (DecodeReg(&FromComp, pFrom, FromMask, True) == eIsReg)
           && (DecodeReg(&ToComp, pTo, ToMask, True) == eIsReg);
}

static Boolean DecodeCReg(char* Asc, Word* Erg) {
    char* endptr;
    int   z;

    for (z = 0; CRegs[z].Name; z++) {
        if (!as_strcasecmp(Asc, CRegs[z].Name)) {
            *Erg = CRegs[z].Code;
            return True;
        }
    }

    if ((as_toupper(*Asc) != 'C') || (as_toupper(Asc[1]) != 'R')) {
        return False;
    } else {
        long value = strtol(Asc + 2, &endptr, 10);
        if ((endptr == Asc + 2) || *endptr || (value < 0) || (value > 31)) {
            return False;
        }
        *Erg = value;
        return True;
    }
}

static Boolean DecodeArgCReg(int Index, Word* pErg) {
    Boolean Result = DecodeCReg(ArgStr[Index].str.p_str, pErg);

    if (!Result) {
        char choices[180] = "CR0..CR31", detail[240];
        unsigned reg;
        for (reg = 0; CRegs[reg].Name; reg++) {
            strmaxcat(choices, ", ", sizeof(choices));
            strmaxcat(choices, CRegs[reg].Name, sizeof(choices));
        }
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgOperandChoices), choices);
        WrXErrorPos(ErrNum_InvCtrlReg, detail, &ArgStr[Index].Pos);
    }
    return Result;
}

static void report_operand_sizes(void) {
    char detail[160];
    as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgSupportedOperandSizes),
            ".B (8), .H (16), .W (32)");
    WrXErrorPos(ErrNum_InvOpSize, detail,
            *AttrPart.str.p_str ? &AttrPart.Pos : &OpPart.Pos);
}

static void report_address_mode(tStrComp const* arg, int message) {
    WrXErrorPos(ErrNum_InvAddrMode, getmessage(message), &arg->Pos);
}

static Boolean DecodeAdr(tStrComp const* pArg, Word* Erg) {
    Word         Base    = 0xff, Tmp;
    /* Keep the expression width when adding separately validated Int32 terms. */
    LargeInt     DispAcc = 0;
    LongInt      DMask, DMax;
    Boolean      OK, FirstFlag = False;
    tSymbolFlags Flags;
    char*        Pos;
    tStrComp     Arg, Remainder;

    if ((OpSize < eSymbolSize8Bit) || (OpSize > eSymbolSize32Bit)) {
        report_operand_sizes();
        return False;
    }
    DMask = (1 << OpSize) - 1;
    DMax = 15 << OpSize;

    if (!IsIndirect(pArg->str.p_str)) {
        report_address_mode(pArg, Num_ErrMsgMCoreAddressSyntax);
        return False;
    }

    StrCompRefRight(&Arg, pArg, 1);
    StrCompShorten(&Arg, 1);
    do {
        Pos = QuotPos(Arg.str.p_str, ',');
        if (Pos) {
            StrCompSplitRef(&Arg, &Remainder, &Arg, Pos);
        }
        switch (DecodeReg(&Arg, &Tmp, AllRegMask, False)) {
        case eIsReg:
            if (Base == 0xff) {
                Base = Tmp;
            } else {
                report_address_mode(&Arg, Num_ErrMsgMCoreMultipleBases);
                return False;
            }
            break;
        case eIsNoReg:
            DispAcc += EvalStrIntExpressionWithFlags(&Arg, Int32, &OK, &Flags);
            if (mFirstPassUnknownOrQuestionable(Flags)) {
                FirstFlag = True;
            }
            if (!OK) {
                return False;
            }
            break;
        default:
            return False;
        }
        if (Pos) {
            Arg = Remainder;
        }
    } while (Pos);

    if (Base == 0xff) {
        if (FirstFlag) {
            /* An unresolved term may become a register alias in a later pass. */
            Base = 0;
        } else {
            report_address_mode(pArg, Num_ErrMsgMCoreMissingBase);
            return False;
        }
    }

    if (FirstFlag) {
        DispAcc -= DispAcc & DMask;
        if (DispAcc < 0) {
            DispAcc = 0;
        }
        if (DispAcc > DMax) {
            DispAcc = DMax;
        }
    }

    if ((DispAcc & DMask) != 0) {
        char detail[180];
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgMCoreDisplacementAlignment),
                (LargeInt)DispAcc, (LargeInt)(1 << OpSize));
        WrXErrorPos(ErrNum_NotAligned, detail, &pArg->Pos);
        return False;
    }
    if ((DispAcc < 0) || (DispAcc > DMax)) {
        char detail[180];
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgEADispRange),
                (LargeInt)DispAcc, (LargeInt)0, (LargeInt)DMax);
        WrXErrorPos((DispAcc < 0) ? ErrNum_UnderRange : ErrNum_OverRange, detail, &pArg->Pos);
        return False;
    }
    *Erg = Base + ((DispAcc >> OpSize) << 4);
    return True;
}

static void DecodeFixed(Word Index) {
    FixedOrder* Instr = FixedOrders + Index;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (ChkArgCnt(0, 0)) {
        if ((Instr->Priv) && (!SupAllowed)) {
            WrError(ErrNum_PrivOrder);
        }
        WAsmCode[0] = Instr->Code;
        CodeLen     = 2;
    }
}

static void DecodeOneReg(Word Index) {
    FixedOrder* Instr = OneRegOrders + Index;
    Word        RegX;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (ChkArgCnt(1, 1) && DecodeArgReg(1, &RegX, AllRegMask)) {
        if ((Instr->Priv) && (!SupAllowed)) {
            WrError(ErrNum_PrivOrder);
        }
        WAsmCode[0] = Instr->Code + RegX;
        CodeLen     = 2;
    }
}

static void DecodeTwoReg(Word Index) {
    FixedOrder* Instr = TwoRegOrders + Index;
    Word        RegX, RegY;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (
            ChkArgCnt(2, 2) && DecodeArgReg(1, &RegX, AllRegMask)
            && DecodeArgReg(2, &RegY, AllRegMask)) {
        if ((Instr->Priv) && (!SupAllowed)) {
            WrError(ErrNum_PrivOrder);
        }
        WAsmCode[0] = Instr->Code + (RegY << 4) + RegX;
        CodeLen     = 2;
    }
}

static void DecodeUImm5(Word Index) {
    ImmOrder*    Instr = UImm5Orders + Index;
    Word         RegX, ImmV;
    Boolean      OK;
    tSymbolFlags Flags;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (ChkArgCnt(2, 2) && DecodeArgReg(1, &RegX, AllRegMask)) {
        ImmV = EvalStrIntExpressionWithFlags(
                &ArgStr[2], (Instr->Ofs > 0) ? UInt6 : UInt5, &OK, &Flags);
        if (!OK) {
            return;
        }
        {
            Word minimum = (Instr->Min > Instr->Ofs) ? Instr->Min : Instr->Ofs;
            Word maximum = 31 + Instr->Ofs;
            if ((ImmV < minimum) || (ImmV > maximum)) {
                if (mFirstPassUnknownOrQuestionable(Flags)) {
                    ImmV = minimum;
                } else {
                    char detail[180];
                    as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgImmediateValueRange),
                            (LargeInt)ImmV, (LargeInt)minimum, (LargeInt)maximum);
                    WrXErrorPos((ImmV < minimum) ? ErrNum_UnderRange : ErrNum_OverRange,
                            detail, &ArgStr[2].Pos);
                    return;
                }
            }
        }
        if (OK) {
            WAsmCode[0] = Instr->Code + ((ImmV - Instr->Ofs) << 4) + RegX;
            CodeLen     = 2;
        }
    }
}

static void report_branch_constraint(LongInt distance, LongInt minimum,
        LongInt maximum, tErrorNum error, tStrComp const* target) {
    char detail[200];
    if (error == ErrNum_DistIsOdd) {
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgBranchDispAlignment),
                (LargeInt)distance, (LargeInt)2, (LargeWord)(EProgCounter() + 2));
    } else {
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgBranchDispRangeAligned),
                (LargeInt)distance, (LargeInt)minimum, (LargeInt)maximum,
                (LargeWord)(EProgCounter() + 2), (LargeInt)2);
    }
    WrXErrorPos(error, detail, &target->Pos);
}

static void report_literal_alignment(LongInt address, tStrComp const* target) {
    char detail[160];
    as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgOperandAlignmentN),
            (LargeWord)address, (LargeInt)4);
    WrXErrorPos(ErrNum_NotAligned, detail, &target->Pos);
}

static void report_literal_range(LongInt address, LongInt index, tStrComp const* target) {
    char detail[180];
    as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgAddressDispRangePC),
            (LargeInt)index * 4, (LargeInt)0, (LargeInt)1020,
            (LargeWord)(address - index * 4));
    WrXErrorPos(ErrNum_JmpDistTooBig, detail, &target->Pos);
}

static void DecodeLJmp(Word Index) {
    FixedOrder*  Instr = LJmpOrders + Index;
    LongInt      Dest;
    Boolean      OK;
    tSymbolFlags Flags;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (ChkArgCnt(1, 1)) {
        Dest = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt32, &OK, &Flags)
               - (EProgCounter() + 2);
        if (OK) {
            if (!mFirstPassUnknownOrQuestionable(Flags) && (Dest & 1)) {
                report_branch_constraint(Dest, -2048, 2046, ErrNum_DistIsOdd, &ArgStr[1]);
            } else if (!mFirstPassUnknownOrQuestionable(Flags) && ((Dest > 2046) || (Dest < -2048))) {
                report_branch_constraint(Dest, -2048, 2046, ErrNum_JmpDistTooBig, &ArgStr[1]);
            } else {
                if ((Instr->Priv) && (!SupAllowed)) {
                    WrError(ErrNum_PrivOrder);
                }
                WAsmCode[0] = Instr->Code + ((Dest >> 1) & 0x7ff);
                CodeLen     = 2;
            }
        }
    }
}

static void DecodeSJmp(Word Index) {
    LongInt      Dest, Address;
    Boolean      OK;
    tSymbolFlags Flags;
    int          l = 0;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (!ChkArgCnt(1, 1))
        ;
    else if (
            (*ArgStr[1].str.p_str != '[')
            || (ArgStr[1].str.p_str[l = strlen(ArgStr[1].str.p_str) - 1] != ']')) {
        char detail[120];
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgOperandChoices), "[address]");
        WrXErrorPos(ErrNum_InvAddrMode, detail, &ArgStr[1].Pos);
    } else {
        ArgStr[1].str.p_str[l] = '\0';
        Dest = EvalStrIntExpressionOffsWithFlags(&ArgStr[1], 1, UInt32, &OK, &Flags);
        if (OK) {
            Address = Dest;
            if (!mFirstPassUnknownOrQuestionable(Flags) && (Dest & 3)) {
                report_literal_alignment(Address, &ArgStr[1]);
            } else {
                Dest = (Dest - (EProgCounter() + 2)) >> 2;
                if ((EProgCounter() & 3) < 2) {
                    Dest++;
                }
                if (!mFirstPassUnknownOrQuestionable(Flags) && ((Dest < 0) || (Dest > 255))) {
                    report_literal_range(Address, Dest, &ArgStr[1]);
                } else {
                    WAsmCode[0] = 0x7000 + (Index << 8) + (Dest & 0xff);
                    CodeLen     = 2;
                }
            }
        }
    }
}

static void DecodeBGENI(Word Index) {
    Word    RegX, ImmV;
    Boolean OK;
    UNUSED(Index);

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (ChkArgCnt(2, 2) && DecodeArgReg(1, &RegX, AllRegMask)) {
        ImmV = EvalStrIntExpression(&ArgStr[2], UInt5, &OK);
        if (OK) {
            if (ImmV > 6) {
                WAsmCode[0] = 0x3200 + (ImmV << 4) + RegX;
            } else {
                WAsmCode[0] = 0x6000 + (1 << (4 + ImmV)) + RegX;
            }
            CodeLen = 2;
        }
    }
}

static void DecodeBMASKI(Word Index) {
    Word         RegX, ImmV;
    Boolean      OK;
    tSymbolFlags Flags;

    UNUSED(Index);

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (ChkArgCnt(2, 2) && DecodeArgReg(1, &RegX, AllRegMask)) {
        ImmV = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt6, &OK, &Flags);
        if (mFirstPassUnknown(Flags) && ((ImmV < 1) || (ImmV > 32))) {
            ImmV = 8;
        }
        if (OK) {
            if (ChkRange(ImmV, 1, 32)) {
                ImmV &= 31;
                if ((ImmV < 1) || (ImmV > 7)) {
                    WAsmCode[0] = 0x2c00 + (ImmV << 4) + RegX;
                } else {
                    WAsmCode[0] = 0x6000 + (((1 << ImmV) - 1) << 4) + RegX;
                }
                CodeLen = 2;
            }
        }
    }
}

static void DecodeLdSt(Word Index) {
    Word RegX, RegZ, NSize;

    if (*AttrPart.str.p_str && (Lo(Index) != 0xff)) {
        WrError(ErrNum_UseLessAttr);
    } else if (!ChkArgCnt(2, 2))
        ;
    else if (OpSize > eSymbolSize32Bit) {
        report_operand_sizes();
    } else {
        if (Lo(Index) != 0xff) {
            OpSize = (tSymbolSize)Lo(Index);
        }
        if (DecodeArgReg(1, &RegZ, AllRegMask) && DecodeAdr(&ArgStr[2], &RegX)) {
            NSize       = (OpSize == eSymbolSize32Bit) ? 0 : OpSize + 1;
            WAsmCode[0] = 0x8000 + (NSize << 13) + (Hi(Index) << 12) + (RegZ << 8) + RegX;
            CodeLen     = 2;
        }
    }
}

static void DecodeLdStm(Word Index) {
    Word RegF, RegL, RegI;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (
            ChkArgCnt(2, 2) && DecodeArgIReg(2, &RegI, 0x0001)
            && DecodeArgRegPair(1, &RegF, 0x7ffe, &RegL, 0x8000)) {
        WAsmCode[0] = 0x0060 + (Index << 4) + RegF;
        CodeLen     = 2;
    }
}

static void DecodeLdStq(Word Index) {
    Word RegF, RegL, RegX;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (
            ChkArgCnt(2, 2) && DecodeArgIReg(2, &RegX, 0xff0f)
            && DecodeArgRegPair(1, &RegF, 0x0010, &RegL, 0x0080)) {
        WAsmCode[0] = 0x0040 + (Index << 4) + RegX;
        CodeLen     = 2;
    }
}

static void DecodeLoopt(Word Index) {
    Word         RegY;
    LongInt      Dest;
    Boolean      OK;
    tSymbolFlags Flags;
    UNUSED(Index);

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (ChkArgCnt(2, 2) && DecodeArgReg(1, &RegY, AllRegMask)) {
        Dest = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt32, &OK, &Flags)
               - (EProgCounter() + 2);
        if (OK) {
            if (!mFirstPassUnknownOrQuestionable(Flags) && (Dest & 1)) {
                report_branch_constraint(Dest, -32, -2, ErrNum_DistIsOdd, &ArgStr[2]);
            } else if (!mFirstPassUnknownOrQuestionable(Flags) && ((Dest > -2) || (Dest < -32))) {
                report_branch_constraint(Dest, -32, -2, ErrNum_JmpDistTooBig, &ArgStr[2]);
            } else {
                WAsmCode[0] = 0x0400 + (RegY << 4) + ((Dest >> 1) & 15);
                CodeLen     = 2;
            }
        }
    }
}

static void DecodeLrm(Word Index) {
    LongInt      Dest, Address;
    Word         RegZ;
    Boolean      OK;
    tSymbolFlags Flags;
    int          l = 0;
    UNUSED(Index);

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (!ChkArgCnt(2, 2))
        ;
    else if (!DecodeArgReg(1, &RegZ, 0x7ffe))
        ;
    else if (
            (*ArgStr[2].str.p_str != '[')
            || (ArgStr[2].str.p_str[l = strlen(ArgStr[2].str.p_str) - 1] != ']')) {
        char detail[120];
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgOperandChoices), "[address]");
        WrXErrorPos(ErrNum_InvAddrMode, detail, &ArgStr[2].Pos);
    } else {
        ArgStr[2].str.p_str[l] = '\0';
        Dest = EvalStrIntExpressionOffsWithFlags(&ArgStr[2], 1, UInt32, &OK, &Flags);
        if (OK) {
            Address = Dest;
            if (!mFirstPassUnknownOrQuestionable(Flags) && (Dest & 3)) {
                report_literal_alignment(Address, &ArgStr[2]);
            } else {
                Dest = (Dest - (EProgCounter() + 2)) >> 2;
                if ((EProgCounter() & 3) < 2) {
                    Dest++;
                }
                if (!mFirstPassUnknownOrQuestionable(Flags) && ((Dest < 0) || (Dest > 255))) {
                    report_literal_range(Address, Dest, &ArgStr[2]);
                } else {
                    WAsmCode[0] = 0x7000 + (RegZ << 8) + (Dest & 0xff);
                    CodeLen     = 2;
                }
            }
        }
    }
}

static void DecodeMcr(Word Index) {
    Word RegX, CRegY;

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (
            ChkArgCnt(2, 2) && DecodeArgReg(1, &RegX, AllRegMask)
            && DecodeArgCReg(2, &CRegY)) {
        if (!SupAllowed) {
            WrError(ErrNum_PrivOrder);
        }
        WAsmCode[0] = 0x1000 + (Index << 11) + (CRegY << 4) + RegX;
        CodeLen     = 2;
    }
}

static void DecodeMovi(Word Index) {
    Word    RegX, ImmV;
    Boolean OK;
    UNUSED(Index);

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (ChkArgCnt(2, 2) && DecodeArgReg(1, &RegX, AllRegMask)) {
        ImmV = EvalStrIntExpression(&ArgStr[2], UInt7, &OK);
        if (OK) {
            WAsmCode[0] = 0x6000 + ((ImmV & 127) << 4) + RegX;
            CodeLen     = 2;
        }
    }
}

static void DecodeTrap(Word Index) {
    Word    ImmV;
    Boolean OK;
    UNUSED(Index);

    if (*AttrPart.str.p_str) {
        WrError(ErrNum_UseLessAttr);
    } else if (!ChkArgCnt(1, 1))
        ;
    else if (*ArgStr[1].str.p_str != '#') {
        WrError(ErrNum_OnlyImmAddr);
    } else {
        ImmV = EvalStrIntExpressionOffs(&ArgStr[1], 1, UInt2, &OK);
        if (OK) {
            WAsmCode[0] = 0x0008 + ImmV;
            CodeLen     = 2;
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     check_pc_even(Word index)
 * \brief  assure machine instruction  is at even address
 * ------------------------------------------------------------------------ */

static void check_pc_even(Word index) {
    UNUSED(index);
    if (Odd(EProgCounter())) {
        char detail[160];
        as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgInstructionAlignment),
                (LargeWord)EProgCounter());
        WrXErrorPos(ErrNum_AddrNotAligned, detail, &OpPart.Pos);
    }
}

/*--------------------------------------------------------------------------*/
/* Codetabellenverwaltung */

static void AddFixed(char const* NName, Word NCode, Boolean NPriv) {
    order_array_rsv_end(FixedOrders, FixedOrder);
    FixedOrders[InstrZ].Code = NCode;
    FixedOrders[InstrZ].Priv = NPriv;
    AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddOneReg(char const* NName, Word NCode, Boolean NPriv) {
    order_array_rsv_end(OneRegOrders, FixedOrder);
    OneRegOrders[InstrZ].Code = NCode;
    OneRegOrders[InstrZ].Priv = NPriv;
    AddInstTable(InstTable, NName, InstrZ++, DecodeOneReg);
}

static void AddTwoReg(char const* NName, Word NCode, Boolean NPriv) {
    order_array_rsv_end(TwoRegOrders, FixedOrder);
    TwoRegOrders[InstrZ].Code = NCode;
    TwoRegOrders[InstrZ].Priv = NPriv;
    AddInstTable(InstTable, NName, InstrZ++, DecodeTwoReg);
}

static void AddUImm5(char const* NName, Word NCode, Word NMin, Word NOfs) {
    order_array_rsv_end(UImm5Orders, ImmOrder);
    UImm5Orders[InstrZ].Code = NCode;
    UImm5Orders[InstrZ].Min  = NMin;
    UImm5Orders[InstrZ].Ofs  = NOfs;
    AddInstTable(InstTable, NName, InstrZ++, DecodeUImm5);
}

static void AddLJmp(char const* NName, Word NCode, Boolean NPriv) {
    order_array_rsv_end(LJmpOrders, FixedOrder);
    LJmpOrders[InstrZ].Code = NCode;
    LJmpOrders[InstrZ].Priv = NPriv;
    AddInstTable(InstTable, NName, InstrZ++, DecodeLJmp);
}

static void AddCReg(char const* NName, Word NCode) {
    order_array_rsv_end(CRegs, CReg);
    CRegs[InstrZ].Name   = NName;
    CRegs[InstrZ++].Code = NCode;
}

static void InitFields(void) {
    InstTable = CreateInstTable(201);

    AddInstTable(InstTable, "REG", 0, CodeREG);
    AddMoto16Pseudo(InstTable, e_moto_pseudo_flags_be);

    inst_table_set_prefix_proc(InstTable, check_pc_even, 0);

    InstrZ = 0;
    AddFixed("BKPT", 0x0000, False);
    AddFixed("DOZE", 0x0006, True);
    AddFixed("RFI", 0x0003, True);
    AddFixed("RTE", 0x0002, True);
    AddFixed("STOP", 0x0004, True);
    AddFixed("SYNC", 0x0001, False);
    AddFixed("WAIT", 0x0005, True);

    InstrZ = 0;
    AddOneReg("ABS", 0x01e0, False);
    AddOneReg("ASRC", 0x3a00, False);
    AddOneReg("BREV", 0x00f0, False);
    AddOneReg("CLRF", 0x01d0, False);
    AddOneReg("CLRT", 0x01c0, False);
    AddOneReg("DECF", 0x0090, False);
    AddOneReg("DECGT", 0x01a0, False);
    AddOneReg("DECLT", 0x0180, False);
    AddOneReg("DECNE", 0x01b0, False);
    AddOneReg("DECT", 0x0080, False);
    AddOneReg("DIVS", 0x3210, False);
    AddOneReg("DIVU", 0x2c10, False);
    AddOneReg("FF1", 0x00e0, False);
    AddOneReg("INCF", 0x00b0, False);
    AddOneReg("INCT", 0x00a0, False);
    AddOneReg("JMP", 0x00c0, False);
    AddOneReg("JSR", 0x00d0, False);
    AddOneReg("LSLC", 0x3c00, False);
    AddOneReg("LSRC", 0x3e00, False);
    AddOneReg("MVC", 0x0020, False);
    AddOneReg("MVCV", 0x0030, False);
    AddOneReg("NOT", 0x01f0, False);
    AddOneReg("SEXTB", 0x0150, False);
    AddOneReg("SEXTH", 0x0170, False);
    AddOneReg("TSTNBZ", 0x0190, False);
    AddOneReg("XSR", 0x3800, False);
    AddOneReg("XTRB0", 0x0130, False);
    AddOneReg("XTRB1", 0x0120, False);
    AddOneReg("XTRB2", 0x0110, False);
    AddOneReg("XTRB3", 0x0100, False);
    AddOneReg("ZEXTB", 0x0140, False);
    AddOneReg("ZEXTH", 0x0160, False);

    InstrZ = 0;
    AddTwoReg("ADDC", 0x0600, False);
    AddTwoReg("ADDU", 0x1c00, False);
    AddTwoReg("AND", 0x1600, False);
    AddTwoReg("ANDN", 0x1f00, False);
    AddTwoReg("ASR", 0x1a00, False);
    AddTwoReg("BGENR", 0x1300, False);
    AddTwoReg("CMPHS", 0x0c00, False);
    AddTwoReg("CMPLT", 0x0d00, False);
    AddTwoReg("CMPNE", 0x0f00, False);
    AddTwoReg("IXH", 0x1d00, False);
    AddTwoReg("IXW", 0x1500, False);
    AddTwoReg("LSL", 0x1b00, False);
    AddTwoReg("LSR", 0x0b00, False);
    AddTwoReg("MOV", 0x1200, False);
    AddTwoReg("MOVF", 0x0a00, False);
    AddTwoReg("MOVT", 0x0200, False);
    AddTwoReg("MULT", 0x0300, False);
    AddTwoReg("OR", 0x1e00, False);
    AddTwoReg("RSUB", 0x1400, False);
    AddTwoReg("SUBC", 0x0700, False);
    AddTwoReg("SUBU", 0x0500, False);
    AddTwoReg("TST", 0x0e00, False);
    AddTwoReg("XOR", 0x1700, False);

    InstrZ = 0;
    AddUImm5("ADDI", 0x2000, 0, 1);
    AddUImm5("ANDI", 0x2e00, 0, 0);
    AddUImm5("ASRI", 0x3a00, 1, 0);
    AddUImm5("BCLRI", 0x3000, 0, 0);
    AddUImm5("BSETI", 0x3400, 0, 0);
    AddUImm5("BTSTI", 0x3600, 0, 0);
    AddUImm5("CMPLTI", 0x2200, 0, 1);
    AddUImm5("CMPNEI", 0x2a00, 0, 0);
    AddUImm5("LSLI", 0x3c00, 1, 0);
    AddUImm5("LSRI", 0x3e00, 1, 0);
    AddUImm5("ROTLI", 0x3800, 1, 0);
    AddUImm5("RSUBI", 0x2800, 0, 0);
    AddUImm5("SUBI", 0x2400, 0, 1);

    InstrZ = 0;
    AddLJmp("BF", 0xe800, False);
    AddLJmp("BR", 0xf000, False);
    AddLJmp("BSR", 0xf800, False);
    AddLJmp("BT", 0xe000, False);

    InstrZ = 0;
    AddCReg("PSR", 0);
    AddCReg("VBR", 1);
    AddCReg("EPSR", 2);
    AddCReg("FPSR", 3);
    AddCReg("EPC", 4);
    AddCReg("FPC", 5);
    AddCReg("SS0", 6);
    AddCReg("SS1", 7);
    AddCReg("SS2", 8);
    AddCReg("SS3", 9);
    AddCReg("SS4", 10);
    AddCReg("GCR", 11);
    AddCReg("GSR", 12);
    AddCReg(NULL, 0);

    AddInstTable(InstTable, "BGENI", 0, DecodeBGENI);
    AddInstTable(InstTable, "BMASKI", 0, DecodeBMASKI);
    AddInstTable(InstTable, "JMPI", 0, DecodeSJmp);
    AddInstTable(InstTable, "JSRI", 15, DecodeSJmp);
    AddInstTable(InstTable, "LD", 0x0ff, DecodeLdSt);
    AddInstTable(InstTable, "LDB", 0x000, DecodeLdSt);
    AddInstTable(InstTable, "LDH", 0x001, DecodeLdSt);
    AddInstTable(InstTable, "LDW", 0x002, DecodeLdSt);
    AddInstTable(InstTable, "ST", 0x1ff, DecodeLdSt);
    AddInstTable(InstTable, "STB", 0x100, DecodeLdSt);
    AddInstTable(InstTable, "STH", 0x101, DecodeLdSt);
    AddInstTable(InstTable, "STW", 0x102, DecodeLdSt);
    AddInstTable(InstTable, "LDM", 0, DecodeLdStm);
    AddInstTable(InstTable, "STM", 1, DecodeLdStm);
    AddInstTable(InstTable, "LDQ", 0, DecodeLdStq);
    AddInstTable(InstTable, "STQ", 1, DecodeLdStq);
    AddInstTable(InstTable, "LOOPT", 0, DecodeLoopt);
    AddInstTable(InstTable, "LRM", 0, DecodeLrm);
    AddInstTable(InstTable, "MFCR", 0, DecodeMcr);
    AddInstTable(InstTable, "MTCR", 1, DecodeMcr);
    AddInstTable(InstTable, "MOVI", 0, DecodeMovi);
    AddInstTable(InstTable, "TRAP", 0, DecodeTrap);
}

static void DeinitFields(void) {
    DestroyInstTable(InstTable);
    order_array_free(FixedOrders);
    order_array_free(OneRegOrders);
    order_array_free(TwoRegOrders);
    order_array_free(UImm5Orders);
    order_array_free(LJmpOrders);
    order_array_free(CRegs);
}

/*--------------------------------------------------------------------------*/
/* Callbacks */

/*!------------------------------------------------------------------------
 * \fn     InternSymbol_MCORE(char *pArg, TempResult *pResult)
 * \brief  handle built-in (register) symbols for M-CORE
 * \param  pArg source argument
 * \param  pResult buffer for possible result
 * ------------------------------------------------------------------------ */

static void InternSymbol_MCORE(char* pArg, TempResult* pResult) {
    Word RegNum;

    if (DecodeRegCore(pArg, &RegNum)) {
        pResult->Typ                       = TempReg;
        pResult->DataSize                  = eSymbolSize32Bit;
        pResult->Contents.RegDescr.Dissect = DissectReg_MCORE;
        pResult->Contents.RegDescr.compare = NULL;
        pResult->Contents.RegDescr.Reg     = RegNum;
    }
}

static Boolean DecodeAttrPart_MCORE(void) {
    if (strlen(AttrPart.str.p_str) > 1) {
        WrStrErrorPos(ErrNum_UndefAttr, &AttrPart);
        return False;
    }
    /* operand size identifiers slightly differ from '68K Standard': */

    switch (as_toupper(*AttrPart.str.p_str)) {
    case 'H':
        AttrPartOpSize[0] = eSymbolSize16Bit;
        break;
    case 'W':
        AttrPartOpSize[0] = eSymbolSize32Bit;
        break;
    case 'L':
        WrStrErrorPos(ErrNum_UndefAttr, &AttrPart);
        return False;
    default:
        return DecodeMoto16AttrSize(*AttrPart.str.p_str, &AttrPartOpSize[0], False);
    }
    return True;
}

static void MakeCode_MCORE(void) {
    if (AttrPartOpSize[0] == eSymbolSizeUnknown) {
        AttrPartOpSize[0] = eSymbolSize32Bit;
    }
    OpSize = AttrPartOpSize[0];

    /* Nullanweisung */

    if ((*OpPart.str.p_str == '\0') && !*AttrPart.str.p_str && (ArgCnt == 0)) {
        return;
    }

    if (!LookupInstTable(InstTable, OpPart.str.p_str)) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
    }
}

static Boolean IsDef_MCORE(void) {
    return Memo("REG");
}

static void SwitchTo_MCORE(void) {
    TFamilyDescr const* p_descr = FindFamilyByName("M-CORE");

    TurnWords = True;
    SetIntConstMode(eIntConstModeMoto);

    PCSymbol    = "*";
    HeaderID    = p_descr->Id;
    NOPCode     = 0x1200; /* ==MOV r0,r0 */
    DivideChars = ",";
    HasAttrs    = True;
    AttrChars   = ".";

    ValidSegs          = (1 << SegCode);
    Grans[SegCode]     = 1;
    ListGrans[SegCode] = 2;
    SegInits[SegCode]  = 0;
    SegLimits[SegCode] = (LargeWord)IntTypeDefs[UInt32].Max;

    DecodeAttrPart = DecodeAttrPart_MCORE;
    MakeCode       = MakeCode_MCORE;
    IsDef          = IsDef_MCORE;
    InternSymbol   = InternSymbol_MCORE;
    DissectReg     = DissectReg_MCORE;

    SwitchFrom = DeinitFields;
    InitFields();
    onoff_supmode_add();
    AddMoto16PseudoONOFF(True);
}

/*--------------------------------------------------------------------------*/
/* Initialisierung */

void codemcore_init(void) {
    CPUMCORE = AddCPU("MCORE", SwitchTo_MCORE);
}
