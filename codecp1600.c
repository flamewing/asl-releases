/* codecp1600.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator GI CP-1600                                                  */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "codepseudo.h"

#include "codecp1600.h"

#include "asmallg.h"
#include "asmcode.h"
#include "asmdef.h"
#include "asmitree.h"
#include "asmpars.h"
#include "asmsub.h"
#include "codevars.h"
#include "errmsg.h"
#include "headids.h"
#include "nls.h"
#include "onoff_common.h"
#include "strutil.h"

#include <ctype.h>
#include <string.h>

/*---------------------------------------------------------------------------*/

#define OPCODE_SDBD 0x0001
#define REG_PC      7
#define REG_SP      6

static CPUVar CPUCP1600;

static Word    Bits;
static Word    Mask;
static Boolean PrefixedSDBD;

/*---------------------------------------------------------------------------*/

/*!------------------------------------------------------------------------
 * \fn     decode_reg_core(const char *p_arg, Word *p_reg)
 * \brief  core register decode routine
 * \param  p_arg source argument
 * \param  p_reg result buffer
 * \return True if argument names a register
 * ------------------------------------------------------------------------ */

static Boolean decode_reg_core(char const* p_arg, Word* p_reg) {
    if (strlen(p_arg) != 2) {
        return False;
    }

    if (toupper(p_arg[0]) == 'S' && toupper(p_arg[1]) == 'P') {
        *p_reg = REG_SP | REGSYM_FLAG_ALIAS;
        return True;
    }
    if (toupper(p_arg[0]) == 'P' && toupper(p_arg[1]) == 'C') {
        *p_reg = REG_PC | REGSYM_FLAG_ALIAS;
        return True;
    }
    if (toupper(p_arg[0]) == 'R' && p_arg[1] >= '0' && p_arg[1] <= '7') {
        *p_reg = p_arg[1] - '0';
        return True;
    }
    return False;
}

/*!------------------------------------------------------------------------
 * \fn     dissect_reg_cp1600(char *p_dest, size_t dest_size, tRegInt value, tSymbolSize inp_size)
 * \brief  dissect register symbols - CP-1600 variant
 * \param  p_dest destination buffer
 * \param  dest_size destination buffer size
 * \param  value numeric register value
 * \param  inp_size register size
 * ------------------------------------------------------------------------ */

static void dissect_reg_cp1600(
        char* p_dest, size_t dest_size, tRegInt value, tSymbolSize inp_size) {
    switch (inp_size) {
    case eSymbolSize16Bit:
        switch (value) {
        case REGSYM_FLAG_ALIAS | REG_PC:
            as_snprintf(p_dest, dest_size, "PC");
            break;
        case REGSYM_FLAG_ALIAS | REG_SP:
            as_snprintf(p_dest, dest_size, "SP");
            break;
        default:
            as_snprintf(p_dest, dest_size, "R%u", (unsigned)(value & 7));
        }
        break;
    default:
        as_snprintf(p_dest, dest_size, "%d-%u", (int)inp_size, (unsigned)value);
    }
}

/*!------------------------------------------------------------------------
 * \fn     DecReg(const tStrComp *p_arg, Word *p_reg, Boolean errMsg)
 * \brief  decode register (core name or symbol)
 * \param  p_arg source argument
 * \param  p_reg result buffer
 * \param  errMsg report error if argument is no register?
 * \return True if argument names a register
 * ------------------------------------------------------------------------ */

static Boolean DecReg(tStrComp const* p_arg, Word* p_reg, Boolean errMsg) {
    tRegDescr      reg_descr;
    tEvalResult    eval_result;
    tRegEvalResult reg_eval_result;

    if (decode_reg_core(p_arg->str.p_str, p_reg)) {
        reg_descr.Reg        = *p_reg;
        eval_result.DataSize = eSymbolSize16Bit;
        reg_eval_result      = eIsReg;
    } else {
        reg_eval_result = EvalStrRegExpressionAsOperand(
                p_arg, &reg_descr, &eval_result, eSymbolSize16Bit, True);
    }

    switch (reg_eval_result) {
    case eIsReg:
        *p_reg = reg_descr.Reg & ~REGSYM_FLAG_ALIAS;
        return True;
    default:
        if (errMsg) {
            WrStrErrorPos(ErrNum_InvRegName, p_arg);
        }
        return False;
    }
}

/*---------------------------------------------------------------------------*/

static void DecodeRegAdrMVO(Word Index) {
    PrefixedSDBD = False;

    if (ChkArgCnt(2, 2)) {
        Word         reg;
        Word         adr;
        Boolean      OK;
        tSymbolFlags flags;

        if (!DecReg(&ArgStr[1], &reg, True)) {
            return;
        }

        adr = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt16, &OK, &flags);
        if (!OK) {
            return;
        }

        if (adr & Mask) {
            WrError(ErrNum_OverRange);
            return;
        }

        WAsmCode[0] = Index | reg;
        set_w_guessed(flags, 1, 1, 0x3ff);
        WAsmCode[1] = adr;
        CodeLen     = 2;
    }
}

static void DecodeRegRegMVO(Word Index) {
    Word regs;
    Word regd;

    PrefixedSDBD = False;

    if (ChkArgCnt(2, 2)) {
        if (!DecReg(&ArgStr[1], &regs, True)) {
            return;
        }

        if (!DecReg(&ArgStr[2], &regd, True)) {
            return;
        }
        if (regd == 0 || regd == 7) {
            WrError(ErrNum_InvReg);
            return;
        }

        WAsmCode[0] = Index | (regd << 3) | regs;
        CodeLen     = 1;
    }
}

static void DecodeRegImmMVO(Word Index) {
    PrefixedSDBD = False;

    if (ChkArgCnt(2, 2)) {
        Word         reg;
        LongInt      val;
        Boolean      OK;
        tSymbolFlags flags;

        if (!DecReg(&ArgStr[1], &reg, True)) {
            return;
        }

        val = EvalStrIntExpressionWithFlags(&ArgStr[2], Int32, &OK, &flags);
        if (!OK) {
            return;
        }
        if (!ChkRange(val, -32768, 65535)) {
            return;
        }

        if (val & Mask) {
            WrError(ErrNum_OverRange);
            return;
        }

        WAsmCode[0] = Index | reg;
        set_w_guessed(flags, 1, 1, 0xffff - Mask);
        WAsmCode[1] = val;
        CodeLen     = 2;
    }
}

static void DecodeAdrReg(Word Index) {
    PrefixedSDBD = False;

    if (ChkArgCnt(2, 2)) {
        Word         reg;
        Word         adr;
        Boolean      OK;
        tSymbolFlags flags;

        adr = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt16, &OK, &flags);
        if (!OK) {
            return;
        }

        if (adr & Mask) {
            WrError(ErrNum_OverRange);
            return;
        }

        if (!DecReg(&ArgStr[2], &reg, True)) {
            return;
        }

        WAsmCode[0] = Index | reg;
        set_w_guessed(flags, 1, 1, 0xffff - Mask);
        WAsmCode[1] = adr;
        CodeLen     = 2;
    }
}

static void DecodeRegReg(Word Index) {
    Word regs;
    Word regd;

    PrefixedSDBD = False;

    if (ChkArgCnt(2, 2)) {
        if (!DecReg(&ArgStr[1], &regs, True)) {
            return;
        }
        if (Index & 0x0200) {
            if (regs == 0 || regs == 7) {
                WrError(ErrNum_InvReg);
                return;
            }
        }

        if (!DecReg(&ArgStr[2], &regd, True)) {
            return;
        }

        WAsmCode[0] = Index | (regs << 3) | regd;
        CodeLen     = 1;
    }
}

static void DecodeImmReg(Word Index) {
    LongInt      val;
    Word         regd;
    Boolean      OK;
    tSymbolFlags flags;
    Boolean      prefixed = PrefixedSDBD;

    PrefixedSDBD = False;

    if (ChkArgCnt(2, 2)) {
        val = EvalStrIntExpressionWithFlags(&ArgStr[1], Int32, &OK, &flags);
        if (!OK) {
            return;
        }
        if (!ChkRange(val, -32768, 65535)) {
            return;
        }

        if (!DecReg(&ArgStr[2], &regd, True)) {
            return;
        }

        if (prefixed) {
            WAsmCode[0] = Index | regd;
            set_w_guessed(flags, 1, 2, 0xff);
            WAsmCode[1] = val & 0x00ff;
            WAsmCode[2] = (val >> 8) & 0x00ff;
            CodeLen     = 3;
        } else {
            if (val & Mask) {
                WAsmCode[0] = OPCODE_SDBD;
                WAsmCode[1] = Index | regd;
                set_w_guessed(flags, 2, 2, 0xff);
                WAsmCode[2] = val & 0x00ff;
                WAsmCode[3] = (val >> 8) & 0x00ff;
                CodeLen     = 4;
            } else {
                WAsmCode[0] = Index | regd;
                set_w_guessed(flags, 1, 1, 0xff);
                WAsmCode[1] = val;
                CodeLen     = 2;
            }
        }
    }
}

static void DecodeRegDup(Word Index) {
    Word reg;

    PrefixedSDBD = False;

    if (ChkArgCnt(1, 1)) {
        if (!DecReg(&ArgStr[1], &reg, True)) {
            return;
        }

        WAsmCode[0] = Index | (reg << 3) | reg;
        CodeLen     = 1;
    }
}

static void DecodeBranch(Word Index) {
    Word         cond = 0, w0_guess_mask = 0x0000;
    Word         adr;
    Boolean      OK;
    tSymbolFlags flags;
    Word         PC = EProgCounter();

    PrefixedSDBD = False;

    if (Index & 0x0010) {
        /* BEXT */
        if (!ChkArgCnt(2, 2)) {
            return;
        }

        cond = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt4, &OK, &flags);
        if (mFirstPassUnknownOrQuestionable(flags)) {
            w0_guess_mask |= 0x000f;
        }
    } else {
        if (!ChkArgCnt(1, 1)) {
            return;
        }

        cond = 0;
    }

    adr = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt16, &OK, &flags);
    if (!OK) {
        return;
    }
    if (mFirstPassUnknownOrQuestionable(flags)) {
        w0_guess_mask |= 0x0020;
    }

    set_wasmcode_guessed(0, 1, w0_guess_mask);
    set_w_guessed(flags, 1, 1, 0x03ff);
    if (adr >= PC + 2) {
        WAsmCode[0] = Index | cond;
        WAsmCode[1] = adr - (PC + 2);
    } else {
        WAsmCode[0] = Index | 0x0020 | cond;
        WAsmCode[1] = PC + 2 - adr - 1;
    }

    if (WAsmCode[1] & Mask) {
        WrError(ErrNum_JmpDistTooBig);
        return;
    }

    CodeLen = 2;
}

static void DecodeNOPP(Word Index) {
    PrefixedSDBD = False;

    if (ChkArgCnt(0, 0)) {
        WAsmCode[0] = Index;
        WAsmCode[1] = 0;
        CodeLen     = 2;
    }
}

static void DecodeReg(Word Index) {
    Word reg;

    PrefixedSDBD = False;

    if (ChkArgCnt(1, 1)) {
        if (!DecReg(&ArgStr[1], &reg, True)) {
            return;
        }

        if (Index == 0x0030) {
            /* GSWD */
            if (reg > 3) {
                WrError(ErrNum_InvReg);
                return;
            }
        }

        WAsmCode[0] = Index | reg;
        CodeLen     = 1;
    }
}

static void DecodeShift(Word Index) {
    Word reg;
    Word bit = 0;

    PrefixedSDBD = False;

    if (ChkArgCnt(1, 2)) {
        tSymbolFlags flags = eSymbolFlag_None;

        if (!DecReg(&ArgStr[1], &reg, True)) {
            return;
        }
        if (reg > 3) {
            WrError(ErrNum_InvReg);
            return;
        }

        if (ArgCnt == 2) {
            Word    val;
            Boolean OK;

            val = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt16, &OK, &flags);
            if (!OK) {
                return;
            }

            if (val == 1) {
                bit = 0x0000;
            } else if (val == 2) {
                bit = 0x0004;
            } else {
                WrError(ErrNum_InvShiftArg);
                return;
            }
        }

        set_w_guessed(flags, 0, 1, 0x0004);
        WAsmCode[0] = Index | bit | reg;
        CodeLen     = 1;
    }
}

static void DecodeOptionalImm(Word Index) {
    Word bit = 0, bit_unknown_mask = 0x0000;

    PrefixedSDBD = False;

    if (ChkArgCnt(0, 1)) {
        if (ArgCnt == 0) {
            bit = 0x0000;
        } else {
            Word         val;
            Boolean      OK;
            tSymbolFlags flags;

            val = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt16, &OK, &flags);
            if (!OK) {
                return;
            }

            if (val == 1) {
                bit = 0x0000;
            } else if (val == 2) {
                bit = 0x0001;
            } else {
                WrError(ErrNum_InvShiftArg);
                return;
            }
            if (mFirstPassUnknownOrQuestionable(flags)) {
                bit_unknown_mask |= 0x0001;
            }
        }
        set_wasmcode_guessed(0, 1, bit_unknown_mask);
        WAsmCode[0] = Index | bit;
        CodeLen     = 1;
    }
}

static void DecodeFixed(Word Index) {
    if (ChkArgCnt(0, 0)) {
        WAsmCode[0] = Index;
        CodeLen     = 1;
    }

    if (Index == OPCODE_SDBD) {
        PrefixedSDBD = True;
    } else {
        PrefixedSDBD = False;
    }
}

static void DecodeJump(Word Index) {
    Word         adr;
    Boolean      OK;
    Word         reg;
    tSymbolFlags flags;

    PrefixedSDBD = False;

    if (Index & 0x0300) {
        /* J/JD/JE */
        if (!ChkArgCnt(1, 1)) {
            return;
        }

        reg = 0;

        adr = EvalStrIntExpressionWithFlags(&ArgStr[1], UInt16, &OK, &flags);
    } else {
        /* JSR/JSRD/JSRE */
        if (!ChkArgCnt(2, 2)) {
            return;
        }

        if (!DecReg(&ArgStr[1], &reg, True)) {
            return;
        }
        if (reg < 4 || reg > 6) {
            WrError(ErrNum_InvReg);
            return;
        }
        reg = (reg - 4) << 8;

        adr = EvalStrIntExpressionWithFlags(&ArgStr[2], UInt16, &OK, &flags);
    }
    if (!OK) {
        return;
    }

    WAsmCode[0] = 0x0004;
    set_w_guessed(flags, 1, 1, 0x00fc);
    WAsmCode[1] = Index | reg | ((adr & 0xfc00) >> 8);
    set_w_guessed(flags, 2, 1, 0x03ff);
    WAsmCode[2] = adr & 0x03ff;
    CodeLen     = 3;
}

static void DecodeJR(Word Index) {
    Word reg;

    PrefixedSDBD = False;

    if (ChkArgCnt(1, 1)) {
        if (!DecReg(&ArgStr[1], &reg, True)) {
            return;
        }

        WAsmCode[0] = Index | (reg << 3);
        CodeLen     = 1;
    }
}

static void DecodeRES(Word Index) {
    Word         Size;
    Boolean      OK;
    tSymbolFlags Flags;
    int          i;

    PrefixedSDBD = False;

    if (!ChkArgCnt(1, 1)) {
        return;
    }

    Size = EvalStrIntExpressionWithFlags(&ArgStr[1], Int16, &OK, &Flags);
    if (!OK) {
        return;
    }
    if (mFirstPassUnknown(Flags)) {
        WrStrErrorPos(ErrNum_FirstPassCalc, &ArgStr[1]);
        return;
    }

    if (Index) {
        for (i = 0; i < Size; i++) {
            WAsmCode[i] = 0;
        }
    } else {
        DontPrint = True;
    }
    CodeLen = Size;
    BookKeeping();
}

static void PutByte(Word value, int* p_half, tSymbolFlags flags) {
    if (*p_half & 1) {
        or_w_guessed(flags, CodeLen - 1, 1, 0xff00);
        WAsmCode[CodeLen - 1] |= value << 8;
    } else {
        set_w_guessed(flags, CodeLen, 1, 0xff);
        WAsmCode[CodeLen++] = value & 0xFF;
    }
    if (Packing) {
        (*p_half)++;
    }
}

static void DecodeWORD(Word Index) {
    int        c;
    int        b = 0;
    TempResult t;

    PrefixedSDBD = False;

    as_tempres_ini(&t);
    if (ChkArgCnt(1, ArgCntMax)) {
        Boolean   OK = True;
        tStrComp* pArg;

        forallargs(pArg, OK) {
            EvalStrExpression(pArg, &t);
            if (mFirstPassUnknown(t.Flags) && t.Typ == TempInt) {
                t.Contents.Int &= 65535;
            }

            switch (t.Typ) {
            case TempInt:
                switch (Index) {
                case 0x0000: /* WORD */
                    if (ChkRange(t.Contents.Int, -32768, 65535)) {
                        set_w_guessed(t.Flags, CodeLen, 1, 0xffff);
                        WAsmCode[CodeLen++] = t.Contents.Int;
                    }
                    b = 0;
                    break;
                case 0x0001: /* BYTE */
                    if (ChkRange(t.Contents.Int, -32768, 65535)) {
                        set_w_guessed(t.Flags, CodeLen, 2, 0x00ff);
                        WAsmCode[CodeLen++] = t.Contents.Int & 0x00FF;
                        WAsmCode[CodeLen++] = (t.Contents.Int >> 8) & 0x00FF;
                    }
                    b = 0;
                    break;
                case 0x0002: /* TEXT */
                    if (ChkRange(t.Contents.Int, 0, 255)) {
                        PutByte(t.Contents.Int & 0xff, &b, t.Flags);
                    }
                    break;
                default:
                    OK = False;
                }
                break;
            case TempFloat:
                WrStrErrorPos(ErrNum_StringOrIntButFloat, pArg);
                OK = False;
                break;
            case TempString:
                if (Index != 0x0002) {
                    OK = False;
                    break;
                }
                TransTableCheckRead();
                if (as_chartrans_xlate_nonz_dynstr(
                            CurrTransTable->p_table, &t.Contents.str, pArg)) {
                    OK = False;
                } else {
                    for (c = 0; c < (int)t.Contents.str.len; c++) {
                        PutByte(t.Contents.str.p_str[c], &b, t.Flags);
                    }
                }
                break;
            default:
                OK = False;
            }
        }
        if (!OK) {
            CodeLen = 0;
        }
    }
    as_tempres_free(&t);
}

static void DecodeBITS(Word Index) {
    Word    val;
    Boolean OK;

    UNUSED(Index);

    PrefixedSDBD = False;

    if (!ChkArgCnt(1, 1)) {
        return;
    }

    val = EvalStrIntExpression(&ArgStr[1], Int16, &OK);
    if (!OK) {
        return;
    }
    if (ChkRange(val, 10, 16)) {
        Bits = val;
        Mask = ~((1 << val) - 1);
    }
}

/*---------------------------------------------------------------------------*/

static void AddRegAdrMVO(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeRegAdrMVO);
}

static void AddRegRegMVO(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeRegRegMVO);
}

static void AddRegImmMVO(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeRegImmMVO);
}

static void AddAdrReg(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeAdrReg);
}

static void AddRegReg(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeRegReg);
}

static void AddImmReg(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeImmReg);
}

static void AddRegDup(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeRegDup);
}

static void AddBranch(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeBranch);
}

static void AddNOPP(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeNOPP);
}

static void AddReg(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeReg);
}

static void AddShift(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeShift);
}

static void AddOptionalImm(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeOptionalImm);
}

static void AddFixed(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeFixed);
}

static void AddJump(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeJump);
}

static void AddJR(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeJR);
}

static void AddRES(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeRES);
}

static void AddWORD(char const* NName, Word NCode) {
    AddInstTable(InstTable, NName, NCode, DecodeWORD);
}

static void InitFields(void) {
    InstTable = CreateInstTable(128);

    add_null_pseudo(InstTable);

    /* Data Access Instructions */
    AddRegAdrMVO("MVO", 0x0240);
    AddRegRegMVO("MVO@", 0x0240);
    AddRegImmMVO("MVOI", 0x0278);
    AddAdrReg("MVI", 0x0280);
    AddRegReg("MVI@", 0x0280);
    AddImmReg("MVII", 0x02B8);
    AddAdrReg("ADD", 0x02C0);
    AddRegReg("ADD@", 0x02C0);
    AddImmReg("ADDI", 0x02F8);
    AddAdrReg("SUB", 0x300);
    AddRegReg("SUB@", 0x0300);
    AddImmReg("SUBI", 0x0338);
    AddAdrReg("CMP", 0x0340);
    AddRegReg("CMP@", 0x0340);
    AddImmReg("CMPI", 0x0378);
    AddAdrReg("AND", 0x0380);
    AddRegReg("AND@", 0x0380);
    AddImmReg("ANDI", 0x03B8);
    AddAdrReg("XOR", 0x03C0);
    AddRegReg("XOR@", 0x03C0);
    AddImmReg("XORI", 0x03F8);

    /* Conditional Branch Instructions */
    AddBranch("B", 0x0200);
    AddBranch("BC", 0x0201);
    AddBranch("BLGE", 0x0201);
    AddBranch("BNC", 0x0209);
    AddBranch("BLLT", 0x0209);
    AddBranch("BOV", 0x0202);
    AddBranch("BNOV", 0x020a);
    AddBranch("BPL", 0x0203);
    AddBranch("BMI", 0x020b);
    AddBranch("BZE", 0x0204);
    AddBranch("BEQ", 0x0204);
    AddBranch("BNZE", 0x020c);
    AddBranch("BNEQ", 0x020c);
    AddBranch("BLT", 0x0205);
    AddBranch("BGE", 0x020d);
    AddBranch("BLE", 0x0206);
    AddBranch("BGT", 0x020e);
    AddBranch("BUSC", 0x0207);
    AddBranch("BESC", 0x020f);
    AddBranch("BEXT", 0x0210);
    AddNOPP("NOPP", 0x0208);

    /* Register to Register */
    AddRegReg("MOVR", 0x0080);
    AddRegReg("ADDR", 0x00C0);
    AddRegReg("SUBR", 0x0100);
    AddRegReg("CMPR", 0x0140);
    AddRegReg("ANDR", 0x0180);
    AddRegReg("XORR", 0x01C0);

    /* Register Shift */
    AddShift("SWAP", 0x0040);
    AddShift("SLL", 0x0048);
    AddShift("RLC", 0x0050);
    AddShift("SLLC", 0x0058);
    AddShift("SLR", 0x0060);
    AddShift("SAR", 0x0068);
    AddShift("RRC", 0x0070);
    AddShift("SARC", 0x0078);

    /* Single Register */
    AddReg("INCR", 0x0008);
    AddReg("DECR", 0x0010);
    AddReg("COMR", 0x0018);
    AddReg("NEGR", 0x0020);
    AddReg("ADCR", 0x0028);
    AddReg("GSWD", 0x0030);
    /* AddFixed("NOP", 0x0034);	*/
    AddOptionalImm("NOP", 0x0034);
    /* AddFixed("SIN", 0x0036);	*/
    AddOptionalImm("SIN", 0x0036);
    AddReg("RSWD", 0x0038);

    /* Internal Control */
    AddFixed("HLT", 0x0000);
    AddFixed("SDBD", OPCODE_SDBD);
    AddFixed("EIS", 0x0002);
    AddFixed("DIS", 0x0003);
    AddFixed("TCI", 0x0005);
    AddFixed("CLRC", 0x0006);
    AddFixed("SETC", 0x0007);

    /* Jump / Jump and Save Return */
    AddJump("J", 0x0300);    /* Second Word */
    AddJump("JE", 0x0301);   /* Second Word */
    AddJump("JD", 0x0302);   /* Second Word */
    AddJump("JSR", 0x0000);  /* Second Word */
    AddJump("JSRE", 0x0001); /* Second Word */
    AddJump("JSRD", 0x0002); /* Second Word */

    /* Alias */
    AddRegDup("TSTR", 0x0080);
    AddRegDup("CLRR", 0x01C0);
    AddJR("JR", 0x0087);
    AddReg("PSHR", 0x0270);
    AddReg("PULR", 0x02B0);

    /* Pseudo Instructions */
    AddRES("RES", 0x0000);   /* Flag Word */
    AddRES("ZERO", 0x0001);  /* Flag Word */
    AddWORD("WORD", 0x0000); /* Flag Word */
    AddWORD("BYTE", 0x0001); /* Flag Word */
    AddWORD("TEXT", 0x0002); /* Flag Word */
    AddInstTable(InstTable, "BITS", 0x0000, DecodeBITS);

    AddInstTable(InstTable, "REG", 0, CodeREG);
}

static void DeinitFields(void) {
    DestroyInstTable(InstTable);
}

/*---------------------------------------------------------------------------*/

static void MakeCode_CP1600(void) {
    if (!LookupInstTable(InstTable, OpPart.str.p_str)) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
    }
}

static Boolean IsDef_CP1600(void) {
    return Memo("REG");
}

/*!------------------------------------------------------------------------
 * \fn     intern_symbol_cp1600(char *pArg, TempResult *pResult)
 * \brief  handle built-in (register) symbols for CP-1600
 * \param  p_arg source argument
 * \param  p_result result buffer
 * ------------------------------------------------------------------------ */

static void intern_symbol_cp1600(char* p_arg, TempResult* p_result) {
    Word reg_num;

    if (decode_reg_core(p_arg, &reg_num)) {
        p_result->Typ                       = TempReg;
        p_result->Contents.RegDescr.Reg     = reg_num;
        p_result->Contents.RegDescr.Dissect = dissect_reg_cp1600;
        p_result->DataSize                  = eSymbolSize16Bit;
        p_result->Contents.RegDescr.compare = NULL;
    }
}

static void SwitchFrom_CP1600(void) {
    DeinitFields();
}

static void SwitchTo_CP1600(void) {
    TFamilyDescr const* pDescr;

    TurnWords = True;
    /* SetIntConstMode(eIntConstModeIBM); */
    NativeIntConstModeMask = (1ul << eIntFormatCOct) | (1ul << eIntFormatIBMXHex)
                             | (1ul << eIntFormatDefRadix);
    OtherIntConstModeMask  = eIntFormatMaskC | eIntFormatMaskIntel | eIntFormatMaskMoto
                             | eIntFormatMaskIBM;
    SetIntConstModeByMask(
            NativeIntConstModeMask | (RelaxedMode ? OtherIntConstModeMask : 0));

    pDescr      = FindFamilyByName("CP1600");
    PCSymbol    = "*";
    HeaderID    = pDescr->Id;
    NOPCode     = 0x0034;
    DivideChars = ",";
    HasAttrs    = False;

    ValidSegs          = (1 << SegCode);
    Grans[SegCode]     = 2;
    ListGrans[SegCode] = 2;
    SegInits[SegCode]  = 0;
    SegLimits[SegCode] = 0xffff;

    Bits         = 16;
    Mask         = 0x0000;
    PrefixedSDBD = False;

    onoff_packing_add(True);

    MakeCode     = MakeCode_CP1600;
    IsDef        = IsDef_CP1600;
    SwitchFrom   = SwitchFrom_CP1600;
    InternSymbol = intern_symbol_cp1600;
    DissectReg   = dissect_reg_cp1600;
    InitFields();
}

void codecp1600_init(void) {
    CPUCP1600 = AddCPU("CP-1600", SwitchTo_CP1600);

    AddCopyright("GI CP-1600-Generator (C) 2021 Haruo Asano");
}
