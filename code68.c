/* code68.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator fuer 68xx Prozessoren                                       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "code68.h"

#include "as.rsc"
#include "asmallg.h"
#include "asmcode.h"
#include "asmdef.h"
#include "asmitree.h"
#include "asmpars.h"
#include "asmsub.h"
#include "assume.h"
#include "bpemu.h"
#include "codepseudo.h"
#include "codevars.h"
#include "cpu2phys.h"
#include "errmsg.h"
#include "function.h"
#include "headids.h"
#include "intpseudo.h"
#include "motpseudo.h"
#include "nlmessages.h"
#include "strutil.h"

#include <string.h>

/*---------------------------------------------------------------------------*/

typedef struct {
    CPUVar MinCPU, MaxCPU;
    Word   Code;
} FixedOrder;

typedef struct {
    CPUVar MinCPU;
    Word   Code;
} RelOrder;

typedef struct {
    Boolean MayImm;
    CPUVar  MinCPU;    /* Shift  andere   ,Y   */
    Byte    PageShift; /* 0 :     nix    Pg 2  */
    Byte    Code;      /* 1 :     Pg 3   Pg 4  */
} ALU16Order;          /* 2 :     nix    Pg 4  */

/* 3 :     Pg 2   Pg 3  */

enum {
    ModNone = -1,
    ModAcc  = 0,
    ModDir  = 1,
    ModExt  = 2,
    ModInd  = 3,
    ModImm  = 4
};

#define MModAcc (1 << ModAcc)
#define MModDir (1 << ModDir)
#define MModExt (1 << ModExt)
#define MModInd (1 << ModInd)
#define MModImm (1 << ModImm)

#define Page2Prefix 0x18
#define Page3Prefix 0x1a
#define Page4Prefix 0xcd

static Byte         PrefCnt;    /* Anzahl Befehlspraefixe */
static ShortInt     AdrMode;    /* Ergebnisadressmodus */
static Byte         AdrPart;    /* Adressierungsmodusbits im Opcode */
static Byte         AdrVals[4]; /* Adressargument */
static tSymbolFlags adr_vals_symflags;

static FixedOrder* FixedOrders;
static RelOrder*   RelOrders;
static ALU16Order* ALU16Orders;

static LongInt Reg_MMSIZ, Reg_MMWBR, Reg_MM1CR, Reg_MM2CR, Reg_INIT, Reg_INIT2,
        Reg_CONFIG;

static CPUVar CPU6800, CPU6801, CPU6301, CPU6811, CPU68HC11K4;

/*---------------------------------------------------------------------------*/

/*!------------------------------------------------------------------------
 * \fn     compute_window(Byte w_size_code, Byte cpu_start_code, LongInt phys_start_code, LongWord phys_offset)
 * \brief  compute single window from MMU registers
 * \param  w_size_code MMSIZ bits (0..3)
 * \param  cpu_start_code Reg_MMWBR bits (0,2,4,6...14)
 * \param  phys_start_code Reg_MMxCR bits
 * \param  phys_offset offset in physical space
 * ------------------------------------------------------------------------ */

static void compute_window(
        Byte w_size_code, Byte cpu_start_code, LongInt phys_start_code,
        LongWord phys_offset) {
    if (w_size_code) {
        Word     size, cpu_start;
        LongWord phys_start;

        /* window size */

        size = 0x1000 << w_size_code;

        /* CPU space start address: assume 8K window, systematically clip out bits for
           larger windows */

        cpu_start = (Word)cpu_start_code << 12;
        if (w_size_code > 1) {
            cpu_start &= ~0x2000;
        }
        if (w_size_code > 2) {
            cpu_start = (cpu_start == 0xc000) ? 0x8000 : cpu_start;
        }

        /* physical space start: mask out lower bits according to window size */

        phys_start = ((phys_start_code & 0x7f & (~((1 << w_size_code) - 1))) << 12)
                     + phys_offset;

        /* set addresses */

        cpu_2_phys_area_add(SegCode, cpu_start, phys_start, size);
    }
}

static void SetK4Ranges(void) {
    Word ee_bank, io_bank, ram_bank;

    cpu_2_phys_area_clear(SegCode);

    /* Add window 1 after window 2, since it has higher priority and may partially overlap
     * window 2 */

    compute_window((Reg_MMSIZ >> 4) & 0x3, (Reg_MMWBR >> 4) & 0x0e, Reg_MM2CR, 0x90000);
    compute_window(Reg_MMSIZ & 0x3, Reg_MMWBR & 0x0e, Reg_MM1CR, 0x10000);

    /* Internal registers, RAM and EEPROM (if enabled) have priority in CPU address space:
     */

    if (Reg_CONFIG & 1) {
        ee_bank = ((Reg_INIT & 15) << 12) + 0x0d80;
        cpu_2_phys_area_add(SegCode, ee_bank, ee_bank, 640);
    }

    io_bank = (Reg_INIT & 15) << 12;
    cpu_2_phys_area_add(SegCode, io_bank, io_bank, 128);

    /* If RAM position overlaps registers, 128 bytes of RAM get relocated to upper end: */

    ram_bank = ((Reg_INIT >> 4) & 15) << 12;
    if (ram_bank == io_bank) {
        ram_bank += 128;
    }
    cpu_2_phys_area_add(SegCode, ram_bank, ram_bank, 768);

    /* Fill the remainder of CPU address space with 1:1 mappings: */

    cpu_2_phys_area_fill(SegCode, 0x0000, 0xffff);
}

/*---------------------------------------------------------------------------*/

static Boolean IsAcc(char Arg, Byte* pReg) {
    static char const Regs[] = "AB";
    char const*       pPos   = strchr(Regs, as_toupper(Arg));

    if (pPos) {
        *pReg = pPos - Regs;
        return True;
    }
    return False;
}

static Boolean DecodeAcc(char const* pArg, Byte* pReg) {
    return (strlen(pArg) == 1) ? IsAcc(*pArg, pReg) : False;
}

static void report_operand_choices(tErrorNum error, tStrComp const* arg, char const* choices) {
    char detail[200];
    as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgOperandChoices), choices);
    WrXErrorPos(error, detail, &arg->Pos);
}

static void report_address_forms(tStrComp const* arg, Byte mask) {
    char forms[350] = "", detail[450];
    if (mask & MModAcc) {
        strmaxcat(forms, "A or B; ", sizeof(forms));
    }
    if (mask & MModImm) {
        strmaxcat(forms, "#value; ", sizeof(forms));
    }
    if (mask & MModDir) {
        strmaxcat(forms, "direct address in [0,255]; ", sizeof(forms));
    }
    if (mask & MModExt) {
        strmaxcat(forms, "extended address; ", sizeof(forms));
    }
    if (mask & MModInd) {
        strmaxcat(forms, MomCPU >= CPU6811 ? "offset,X or offset,Y; " : "offset,X; ", sizeof(forms));
    }
    if (*forms) {
        forms[strlen(forms) - 2] = '\0';
    }
    as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgAddressForms), forms);
    WrXErrorPos(ErrNum_InvAddrMode, detail, &arg->Pos);
}

static void DecodeAdr(int StartInd, int StopInd, tSymbolSize op_size, Byte Erl) {
    tStrComp* pStartArg = &ArgStr[StartInd];
    Boolean   OK, ErrOcc;
    Word      AdrWord;
    Byte      Bit8;

    AdrMode = ModNone;
    AdrPart = 0;
    ErrOcc  = False;

    /* eine Komponente ? */

    if (StartInd == StopInd) {
        /* Akkumulatoren ? */

        if (DecodeAcc(pStartArg->str.p_str, &AdrPart)) {
            if (MModAcc & Erl) {
                AdrMode = ModAcc;
            }
        }

        /* immediate ? */

        else if ((strlen(pStartArg->str.p_str) > 1) && (*pStartArg->str.p_str == '#')) {
            if (MModImm & Erl) {
                if (op_size == eSymbolSize16Bit) {
                    AdrWord = EvalStrIntExpressionOffsWithFlags(
                            pStartArg, 1, Int16, &OK, &adr_vals_symflags);
                    if (OK) {
                        AdrMode           = ModImm;
                        AdrVals[AdrCnt++] = Hi(AdrWord);
                        AdrVals[AdrCnt++] = Lo(AdrWord);
                    } else {
                        ErrOcc = True;
                    }
                } else {
                    AdrVals[AdrCnt] = EvalStrIntExpressionOffsWithFlags(
                            pStartArg, 1, Int8, &OK, &adr_vals_symflags);
                    if (OK) {
                        AdrMode = ModImm;
                        AdrCnt++;
                    } else {
                        ErrOcc = True;
                    }
                }
            }
        }

        /* absolut ? */

        else {
            unsigned Offset = 0;

            Bit8 = 0;
            if (pStartArg->str.p_str[Offset] == '<') {
                Bit8 = 2;
                Offset++;
            } else if (pStartArg->str.p_str[Offset] == '>') {
                Bit8 = 1;
                Offset++;
            }
            if (MomCPU == CPU68HC11K4) {
                LargeWord AdrLWord = EvalStrIntExpressionOffsWithFlags(
                        pStartArg, Offset, UInt21, &OK, &adr_vals_symflags);
                if (OK) {
                    if (!def_phys_2_cpu(SegCode, &AdrLWord)) {
                        if (!mFirstPassUnknownOrQuestionable(adr_vals_symflags)) {
                            char detail[250];
                            as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgPhysicalMapping),
                                        (LargeWord)AdrLWord, SegNames[SegCode], "68HC11K4 MMU");
                            WrXErrorPos(ErrNum_InAccPage, detail, &pStartArg->Pos);
                        }
                        AdrWord = AdrLWord & 0xffffu;
                    } else {
                        AdrWord = AdrLWord;
                    }
                } else {
                    AdrWord = 0;
                }
            } else {
                AdrWord = EvalStrIntExpressionOffsWithFlags(
                        pStartArg, Offset, UInt16, &OK, &adr_vals_symflags);
            }
            if (OK) {
                if ((MModDir & Erl) && (Bit8 != 1)
                    && ((Bit8 == 2) || (!(MModExt & Erl)) || (Hi(AdrWord) == 0))) {
                    if ((Hi(AdrWord) != 0) && !mFirstPassUnknown(adr_vals_symflags)) {
                        char detail[200];
                        as_snprintf(detail, sizeof(detail), getmessage(Bit8 == 2 ? Num_ErrMsgForcedShortAddressRange : Num_ErrMsgAddressRange),
                                    (LargeInt)AdrWord, (LargeInt)0, (LargeInt)255);
                        WrXErrorPos(ErrNum_NoShortAddr, detail, &pStartArg->Pos);
                        ErrOcc = True;
                    } else {
                        AdrMode           = ModDir;
                        AdrPart           = 1;
                        AdrVals[AdrCnt++] = Lo(AdrWord);
                    }
                } else if (MModExt & Erl) {
                    AdrMode           = ModExt;
                    AdrPart           = 3;
                    AdrVals[AdrCnt++] = Hi(AdrWord);
                    AdrVals[AdrCnt++] = Lo(AdrWord);
                }
            } else {
                ErrOcc = True;
            }
        }
    }

    /* zwei Komponenten ? */

    else if (StartInd + 1 == StopInd) {
        Boolean IsX = !as_strcasecmp(ArgStr[StopInd].str.p_str, "X"),
                IsY = !as_strcasecmp(ArgStr[StopInd].str.p_str, "Y");

        /* indiziert ? */

        if (IsX || IsY) {
            if (MModInd & Erl) {
                if (pStartArg->str.p_str[0]) {
                    AdrWord = EvalStrIntExpressionWithFlags(
                            pStartArg, UInt8, &OK, &adr_vals_symflags);
                } else {
                    AdrWord = 0;
                    OK      = True;
                }
                if (OK) {
                    if (IsY && !ChkMinCPUExt(CPU6811, ErrNum_AddrModeNotSupported)) {
                        ErrOcc = True;
                    } else {
                        AdrVals[AdrCnt++] = Lo(AdrWord);
                        AdrMode           = ModInd;
                        AdrPart           = 2;
                        if (IsY) {
                            BAsmCode[PrefCnt++] = 0x18;
                        }
                    }
                } else {
                    ErrOcc = True;
                }
            }
        } else {
            report_operand_choices(ErrNum_InvReg, &ArgStr[StopInd], MomCPU >= CPU6811 ? "X or Y" : "X");
            ErrOcc = True;
        }
    }

    else {
        char Str[100];

        as_snprintf(
                Str, sizeof(Str), getmessage(Num_ErrMsgAddrArgCnt), 1, 2,
                StopInd - StartInd + 1);
        WrXError(ErrNum_WrongArgCnt, Str);
        ErrOcc = True;
    }

    if ((!ErrOcc) && (AdrMode == ModNone)) {
        report_address_forms(pStartArg, Erl);
    }
}

static void append_adr_vals(int dest) {
    set_b_guessed(adr_vals_symflags, dest, AdrCnt, 0xff);
    memcpy(BAsmCode + dest, AdrVals, AdrCnt);
}

static void AddPrefix(Byte Prefix) {
    BAsmCode[PrefCnt++] = Prefix;
}

static void Try2Split(int Src) {
    char*  p;
    size_t SrcLen;

    KillPrefBlanksStrComp(&ArgStr[Src]);
    KillPostBlanksStrComp(&ArgStr[Src]);
    SrcLen = strlen(ArgStr[Src].str.p_str);
    p      = ArgStr[Src].str.p_str + SrcLen - 1;
    while ((p > ArgStr[Src].str.p_str) && !as_isspace(*p)) {
        p--;
    }
    if (p > ArgStr[Src].str.p_str) {
        InsertArg(Src + 1, SrcLen);
        StrCompSplitRight(&ArgStr[Src], &ArgStr[Src + 1], p);
        KillPostBlanksStrComp(&ArgStr[Src]);
        KillPrefBlanksStrComp(&ArgStr[Src + 1]);
    }
}

/*!------------------------------------------------------------------------
 * \fn     extract_acc_arg(const char *p_src_arg, Byte *p_acc_reg)
 * \brief  decode accumulator name from (beginning of) source arg
 * \param  p_src_arg source argument
 * \param  p_acc_reg dest buffer for accumulator #
 * \return NULL if no accumulator given, otherwise * to remainder of argument
 * ------------------------------------------------------------------------ */

static char const* extract_acc_arg(char const* p_src_arg, Byte* p_acc_reg) {
    int l = strlen(p_src_arg);

    if ((l >= 1) && IsAcc(*p_src_arg, p_acc_reg) && as_isspace_or_nul(p_src_arg[1])) {
        char const* p_remainder;

        for (p_remainder = p_src_arg + 1; *p_remainder && as_isspace(*p_remainder);
             p_remainder++)
            ;
        return p_remainder;
    } else {
        return NULL;
    }
}

/*---------------------------------------------------------------------------*/

static void DecodeFixed(Word Index) {
    FixedOrder const* forder = FixedOrders + Index;

    if (!moto8_chk_no_args())
        ;
    else if (!ChkRangeCPU(forder->MinCPU, forder->MaxCPU))
        ;
    else if (Hi(forder->Code) != 0) {
        CodeLen     = 2;
        BAsmCode[0] = Hi(forder->Code);
        BAsmCode[1] = Lo(forder->Code);
    } else {
        CodeLen     = 1;
        BAsmCode[0] = Lo(forder->Code);
    }
}

static void report_branch_distance(Integer distance, LongWord pc_base, tStrComp const* target) {
    char detail[180];
    as_snprintf(detail, sizeof(detail), getmessage(Num_ErrMsgBranchDispRange),
            (LargeInt)distance, (LargeInt)-128, (LargeInt)127, (LargeWord)pc_base);
    WrXErrorPos(ErrNum_JmpDistTooBig, detail, &target->Pos);
}

static void DecodeRel(Word Index) {
    RelOrder const* pOrder = &RelOrders[Index];
    Integer         AdrInt;
    Boolean         OK;
    tSymbolFlags    Flags;

    if (ChkArgCnt(1, 1) && ChkMinCPU(pOrder->MinCPU)) {
        AdrInt = EvalStrIntExpressionWithFlags(&ArgStr[1], Int16, &OK, &Flags);
        if (OK) {
            AdrInt -= EProgCounter() + 2;
            if (((AdrInt < -128) || (AdrInt > 127)) && !mFirstPassUnknownOrQuestionable(Flags)) {
                report_branch_distance(AdrInt, EProgCounter() + 2, &ArgStr[1]);
            } else {
                CodeLen     = 2;
                BAsmCode[0] = pOrder->Code;
                set_b_guessed(Flags, 1, 1, 0xff);
                BAsmCode[1] = Lo(AdrInt);
            }
        }
    }
}

static void DecodeALU16(Word Index) {
    ALU16Order const* forder = ALU16Orders + Index;

    if (ChkArgCnt(1, 2) && ChkMinCPU(forder->MinCPU)) {
        DecodeAdr(
                1, ArgCnt, eSymbolSize16Bit,
                (forder->MayImm ? MModImm : 0) | MModInd | MModExt | MModDir);
        if (AdrMode != ModNone) {
            switch (forder->PageShift) {
            case 1:
                if (PrefCnt == 1) {
                    BAsmCode[PrefCnt - 1] = Page4Prefix;
                } else {
                    AddPrefix(Page3Prefix);
                }
                break;
            case 2:
                if (PrefCnt == 1) {
                    BAsmCode[PrefCnt - 1] = Page4Prefix;
                }
                break;
            case 3:
                if (PrefCnt == 0) {
                    AddPrefix((AdrMode == ModInd) ? Page3Prefix : Page2Prefix);
                }
                break;
            }
            BAsmCode[PrefCnt] = forder->Code + (AdrPart << 4);
            CodeLen           = PrefCnt + 1 + AdrCnt;
            append_adr_vals(1 + PrefCnt);
        }
    }
}

static void DecodeBit63(Word Code) {
    if (ChkArgCnt(2, 3) && ChkExactCPU(CPU6301)) {
        DecodeAdr(1, 1, eSymbolSize8Bit, MModImm);
        if (AdrMode != ModNone) {
            DecodeAdr(2, ArgCnt, eSymbolSizeUnknown, MModDir | MModInd);
            if (AdrMode != ModNone) {
                BAsmCode[PrefCnt] = Code;
                if (AdrMode == ModDir) {
                    BAsmCode[PrefCnt] |= 0x10;
                }
                CodeLen = PrefCnt + 1 + AdrCnt;
                append_adr_vals(1 + PrefCnt);
            }
        }
    }
}

static void DecodeJMP(Word Index) {
    UNUSED(Index);

    if (ChkArgCnt(1, 2)) {
        DecodeAdr(1, ArgCnt, eSymbolSizeUnknown, MModExt | MModInd);
        if (AdrMode != ModNone) {
            CodeLen           = PrefCnt + 1 + AdrCnt;
            BAsmCode[PrefCnt] = 0x4e + (AdrPart << 4);
            append_adr_vals(1 + PrefCnt);
        }
    }
}

static void DecodeJSR(Word Index) {
    UNUSED(Index);

    if (ChkArgCnt(1, 2)) {
        DecodeAdr(
                1, ArgCnt, eSymbolSizeUnknown,
                MModExt | MModInd | ((MomCPU >= CPU6801) ? MModDir : 0));
        if (AdrMode != ModNone) {
            CodeLen           = PrefCnt + 1 + AdrCnt;
            BAsmCode[PrefCnt] = 0x8d + (AdrPart << 4);
            append_adr_vals(1 + PrefCnt);
        }
    }
}

static void DecodeBRxx(Word Index) {
    if (ArgCnt == 1) {
        Try2Split(1);
        Try2Split(1);
    } else if (ArgCnt == 2) {
        Try2Split(ArgCnt);
        Try2Split(2);
    }
    if (ChkArgCnt(3, 4) && ChkMinCPU(CPU6811)) {
        Boolean      OK;
        tSymbolFlags mask_flags;
        Byte         Mask = EvalStrIntExpressionOffsWithFlags(
                &ArgStr[ArgCnt - 1], !!(*ArgStr[ArgCnt - 1].str.p_str == '#'), Int8, &OK,
                &mask_flags);

        if (OK) {
            DecodeAdr(1, ArgCnt - 2, eSymbolSizeUnknown, MModDir | MModInd);
            if (AdrMode != ModNone) {
                tSymbolFlags address_flags;
                Integer      AdrInt = EvalStrIntExpressionWithFlags(
                        &ArgStr[ArgCnt], Int16, &OK, &address_flags);

                if (OK) {
                    AdrInt -= EProgCounter() + 3 + PrefCnt + AdrCnt;
                    if (((AdrInt < -128) || (AdrInt > 127))
                        && !mFirstPassUnknownOrQuestionable(address_flags)) {
                        report_branch_distance(AdrInt,
                                EProgCounter() + 3 + PrefCnt + AdrCnt, &ArgStr[ArgCnt]);
                    } else {
                        CodeLen           = PrefCnt + 3 + AdrCnt;
                        BAsmCode[PrefCnt] = 0x12 + Index;
                        if (AdrMode == ModInd) {
                            BAsmCode[PrefCnt] += 12;
                        }
                        append_adr_vals(PrefCnt + 1);
                        set_b_guessed(mask_flags, PrefCnt + 1, 1, 0xff);
                        BAsmCode[PrefCnt + 1 + AdrCnt] = Mask;
                        set_b_guessed(address_flags, PrefCnt + 2 + AdrCnt, 1, 0xff);
                        BAsmCode[PrefCnt + 2 + AdrCnt] = Lo(AdrInt);
                    }
                }
            }
        }
    }
}

static void DecodeBxx(Word Index) {
    int       AddrStart, AddrEnd;
    tStrComp* pMaskArg;

    if (MomCPU == CPU6301) {
        pMaskArg  = &ArgStr[1];
        AddrStart = 2;
        AddrEnd   = ArgCnt;
    } else {
        if ((ArgCnt >= 1) && (ArgCnt <= 2)) {
            Try2Split(ArgCnt);
        }
        pMaskArg  = &ArgStr[ArgCnt];
        AddrStart = 1;
        AddrEnd   = ArgCnt - 1;
    }
    if (ChkArgCnt(2, 3) && ChkMinCPU(CPU6301)) {
        Boolean      OK;
        tSymbolFlags mask_flags;
        Byte         Mask = EvalStrIntExpressionOffsWithFlags(
                pMaskArg, !!(*pMaskArg->str.p_str == '#'),
                (MomCPU == CPU6301) ? UInt3 : Int8, &OK, &mask_flags);
        if (OK && (MomCPU == CPU6301)) {
            Mask = 1 << Mask;
            if (Index == 1) {
                Mask = 0xff - Mask;
            }
        }
        if (OK) {
            DecodeAdr(AddrStart, AddrEnd, eSymbolSizeUnknown, MModDir | MModInd);
            if (AdrMode != ModNone) {
                CodeLen = PrefCnt + 2 + AdrCnt;
                if (MomCPU == CPU6301) {
                    BAsmCode[PrefCnt] = 0x62 - Index;
                    if (AdrMode == ModDir) {
                        BAsmCode[PrefCnt] += 0x10;
                    }
                    set_b_guessed(mask_flags, 1 + PrefCnt, 1, 0xff);
                    BAsmCode[1 + PrefCnt] = Mask;
                    append_adr_vals(2 + PrefCnt);
                } else {
                    BAsmCode[PrefCnt] = 0x14 + Index;
                    if (AdrMode == ModInd) {
                        BAsmCode[PrefCnt] += 8;
                    }
                    append_adr_vals(1 + PrefCnt);
                    set_b_guessed(mask_flags, 1 + PrefCnt + AdrCnt, 1, 0xff);
                    BAsmCode[1 + PrefCnt + AdrCnt] = Mask;
                }
            }
        }
    }
}

static void DecodeBTxx(Word Index) {
    if (ChkArgCnt(2, 3) && ChkExactCPU(CPU6301)) {
        Boolean      OK;
        tSymbolFlags flags;
        Byte         AdrByte = EvalStrIntExpressionOffsWithFlags(
                &ArgStr[1], !!(*ArgStr[1].str.p_str == '#'), UInt3, &OK, &flags);

        if (OK) {
            DecodeAdr(2, ArgCnt, eSymbolSizeUnknown, MModDir | MModInd);
            if (AdrMode != ModNone) {
                CodeLen = PrefCnt + 2 + AdrCnt;
                set_b_guessed(flags, 1 + PrefCnt, 1, 0xff);
                BAsmCode[1 + PrefCnt] = 1 << AdrByte;
                append_adr_vals(2 + PrefCnt);
                BAsmCode[PrefCnt] = 0x65 + Index;
                if (AdrMode == ModDir) {
                    BAsmCode[PrefCnt] += 0x10;
                }
            }
        }
    }
}

static void DecodeALU8(Word Code) {
    Byte    acc_reg;
    Boolean acc_set   = False;
    int     MinArgCnt = Hi(Code) & 3, start_arg = 1;

    /* Instruction already contains accumulator spec in mnemonic? */

    if (MinArgCnt == 1) {
        acc_reg = (Code >> 14) & 1;
        acc_set = True;
    }

    /* If not, accumulator spec may be the first argument or a
       'prefix' of it: */

    if (!acc_set && (ArgCnt >= 1)) {
        char const* p_remainder = extract_acc_arg(ArgStr[1].str.p_str, &acc_reg);

        if (p_remainder) {
            acc_set = True;
            if (!*p_remainder) {
                start_arg = 2;
            } else {
                StrCompCutLeft(&ArgStr[1], p_remainder - ArgStr[1].str.p_str);
            }
        }
    }

    /* No accumulator arg detected. if mnemonic ends on 'A' or, 'B', assume respective
       accumulator: */

    if (!acc_set && IsAcc(OpPart.str.p_str[2], &acc_reg)) {
        acc_set = True;
    }

    if (!acc_set) {
        report_operand_choices(ErrNum_InvAddrMode, ArgCnt ? &ArgStr[1] : &OpPart, "A or B accumulator");
        return;
    }

    if (ChkArgCnt(start_arg, start_arg + 1)) {
        DecodeAdr(
                start_arg, ArgCnt, eSymbolSize8Bit,
                ((Code & 0x8000) ? MModImm : 0) | MModInd | MModExt | MModDir);
        if (AdrMode != ModNone) {
            BAsmCode[PrefCnt] = Lo(Code) | (AdrPart << 4) | (acc_reg << 6);
            append_adr_vals(1 + PrefCnt);
            CodeLen = PrefCnt + 1 + AdrCnt;
        }
    }
}

static void DecodeSing8(Word Code) {
    if (ChkArgCnt(1, 2)) {
        DecodeAdr(1, ArgCnt, eSymbolSizeUnknown, MModAcc | MModExt | MModInd);
        if (AdrMode != ModNone) {
            CodeLen           = PrefCnt + 1 + AdrCnt;
            BAsmCode[PrefCnt] = Code | (AdrPart << 4);
            append_adr_vals(1 + PrefCnt);
        }
    }
}

static void DecodeSing8_Acc(Word Code) {
    if (moto8_chk_no_args()) {
        BAsmCode[PrefCnt] = Code;
        CodeLen           = PrefCnt + 1;
    }
}

static void DecodePSH_PUL(Word Code) {
    if (ChkArgCnt(1, 1)) {
        DecodeAdr(1, 1, eSymbolSizeUnknown, MModAcc);
        if (AdrMode != ModNone) {
            CodeLen     = 1;
            BAsmCode[0] = Code | AdrPart;
        }
    }
}

static void DecodePRWINS(Word Code) {
    UNUSED(Code);

    if (ChkExactCPU(CPU68HC11K4)) {
        printf("\nMMSIZ $%02x MMWBR $%02x MM1CR $%02x MM2CR $%02x INIT $%02x INIT2 $%02x "
               "CONFIG $%02x\n",
               (unsigned)Reg_MMSIZ, (unsigned)Reg_MMWBR, (unsigned)Reg_MM1CR,
               (unsigned)Reg_MM2CR, (unsigned)Reg_INIT, (unsigned)Reg_INIT2,
               (unsigned)Reg_CONFIG);
        cpu_2_phys_area_dump(SegCode, stdout);
    }
}

/*---------------------------------------------------------------------------*/

static void AddFixed(char const* NName, CPUVar NMin, CPUVar NMax, Word NCode) {
    order_array_rsv_end(FixedOrders, FixedOrder);
    FixedOrders[InstrZ].MinCPU = NMin;
    FixedOrders[InstrZ].MaxCPU = NMax;
    FixedOrders[InstrZ].Code   = NCode;
    AddInstTable(InstTable, NName, InstrZ++, DecodeFixed);
}

static void AddRel(char const* NName, CPUVar NMin, Word NCode) {
    order_array_rsv_end(RelOrders, RelOrder);
    RelOrders[InstrZ].MinCPU = NMin;
    RelOrders[InstrZ].Code   = NCode;
    AddInstTable(InstTable, NName, InstrZ++, DecodeRel);
}

static void AddALU8(char const* p_name, Boolean MayImm, Byte NCode) {
    char name[10];
    Word BaseCode = NCode | (MayImm ? 0x8000 : 0);

    AddInstTable(InstTable, p_name, BaseCode | (2 << 8), DecodeALU8);
    as_snprintf(name, sizeof(name), "%sA", p_name);
    AddInstTable(InstTable, name, BaseCode | (1 << 8), DecodeALU8);
    as_snprintf(name, sizeof(name), "%sB", p_name);
    AddInstTable(InstTable, name, BaseCode | (1 << 8) | 0x4000, DecodeALU8);
}

static void AddALU16(
        char const* NName, Boolean NMay, CPUVar NMin, Byte NShift, Byte NCode) {
    order_array_rsv_end(ALU16Orders, ALU16Order);
    ALU16Orders[InstrZ].MayImm    = NMay;
    ALU16Orders[InstrZ].MinCPU    = NMin;
    ALU16Orders[InstrZ].PageShift = NShift;
    ALU16Orders[InstrZ].Code      = NCode;
    AddInstTable(InstTable, NName, InstrZ++, DecodeALU16);
}

static void AddSing8(char const* p_name, Byte NCode) {
    char name[10];
    AddInstTable(InstTable, p_name, NCode, DecodeSing8);
    as_snprintf(name, sizeof(name), "%sA", p_name);
    AddInstTable(InstTable, name, NCode | 0, DecodeSing8_Acc);
    as_snprintf(name, sizeof(name), "%sB", p_name);
    AddInstTable(InstTable, name, NCode | 0x10, DecodeSing8_Acc);
}

static void InitFields(void) {
    InstTable = CreateInstTable(317);
    SetDynamicInstTable(InstTable);

    add_null_pseudo(InstTable);

    AddInstTable(InstTable, "JMP", 0, DecodeJMP);
    AddInstTable(InstTable, "JSR", 0, DecodeJSR);
    AddInstTable(InstTable, "BRCLR", 1, DecodeBRxx);
    AddInstTable(InstTable, "BRSET", 0, DecodeBRxx);
    AddInstTable(InstTable, "BCLR", 1, DecodeBxx);
    AddInstTable(InstTable, "BSET", 0, DecodeBxx);
    AddInstTable(InstTable, "BTST", 6, DecodeBTxx);
    AddInstTable(InstTable, "BTGL", 0, DecodeBTxx);

    InstrZ = 0;
    AddFixed("ABA", CPU6800, CPU68HC11K4, 0x001b);
    AddFixed("ABX", CPU6801, CPU68HC11K4, 0x003a);
    AddFixed("ABY", CPU6811, CPU68HC11K4, 0x183a);
    AddFixed("ASLD", CPU6801, CPU68HC11K4, 0x0005);
    AddFixed("CBA", CPU6800, CPU68HC11K4, 0x0011);
    AddFixed("CLC", CPU6800, CPU68HC11K4, 0x000c);
    AddFixed("CLI", CPU6800, CPU68HC11K4, 0x000e);
    AddFixed("CLV", CPU6800, CPU68HC11K4, 0x000a);
    AddFixed("DAA", CPU6800, CPU68HC11K4, 0x0019);
    AddFixed("DES", CPU6800, CPU68HC11K4, 0x0034);
    AddFixed("DEX", CPU6800, CPU68HC11K4, 0x0009);
    AddFixed("DEY", CPU6811, CPU68HC11K4, 0x1809);
    AddFixed("FDIV", CPU6811, CPU68HC11K4, 0x0003);
    AddFixed("IDIV", CPU6811, CPU68HC11K4, 0x0002);
    AddFixed("INS", CPU6800, CPU68HC11K4, 0x0031);
    AddFixed("INX", CPU6800, CPU68HC11K4, 0x0008);
    AddFixed("INY", CPU6811, CPU68HC11K4, 0x1808);
    AddFixed("LSLD", CPU6801, CPU68HC11K4, 0x0005);
    AddFixed("LSRD", CPU6801, CPU68HC11K4, 0x0004);
    AddFixed("MUL", CPU6801, CPU68HC11K4, 0x003d);
    AddFixed("NOP", CPU6800, CPU68HC11K4, 0x0001);
    AddFixed("PSHX", CPU6801, CPU68HC11K4, 0x003c);
    AddFixed("PSHY", CPU6811, CPU68HC11K4, 0x183c);
    AddFixed("PULX", CPU6801, CPU68HC11K4, 0x0038);
    AddFixed("PULY", CPU6811, CPU68HC11K4, 0x1838);
    AddFixed("RTI", CPU6800, CPU68HC11K4, 0x003b);
    AddFixed("RTS", CPU6800, CPU68HC11K4, 0x0039);
    AddFixed("SBA", CPU6800, CPU68HC11K4, 0x0010);
    AddFixed("SEC", CPU6800, CPU68HC11K4, 0x000d);
    AddFixed("SEI", CPU6800, CPU68HC11K4, 0x000f);
    AddFixed("SEV", CPU6800, CPU68HC11K4, 0x000b);
    AddFixed("SLP", CPU6301, CPU6301, 0x001a);
    AddFixed("STOP", CPU6811, CPU68HC11K4, 0x00cf);
    AddFixed("SWI", CPU6800, CPU68HC11K4, 0x003f);
    AddFixed("TAB", CPU6800, CPU68HC11K4, 0x0016);
    AddFixed("TAP", CPU6800, CPU68HC11K4, 0x0006);
    AddFixed("TBA", CPU6800, CPU68HC11K4, 0x0017);
    AddFixed("TPA", CPU6800, CPU68HC11K4, 0x0007);
    AddFixed("TSX", CPU6800, CPU68HC11K4, 0x0030);
    AddFixed("TSY", CPU6811, CPU68HC11K4, 0x1830);
    AddFixed("TXS", CPU6800, CPU68HC11K4, 0x0035);
    AddFixed("TYS", CPU6811, CPU68HC11K4, 0x1835);
    AddFixed("WAI", CPU6800, CPU68HC11K4, 0x003e);
    AddFixed("XGDX", CPU6301, CPU68HC11K4, (MomCPU == CPU6301) ? 0x0018 : 0x008f);
    AddFixed("XGDY", CPU6811, CPU68HC11K4, 0x188f);
    AddFixed("HCF", CPU6800, CPU6801, 0x009d);

    InstrZ = 0;
    AddRel("BCC", CPU6800, 0x24);
    AddRel("BCS", CPU6800, 0x25);
    AddRel("BEQ", CPU6800, 0x27);
    AddRel("BGE", CPU6800, 0x2c);
    AddRel("BGT", CPU6800, 0x2e);
    AddRel("BHI", CPU6800, 0x22);
    AddRel("BHS", CPU6800, 0x24);
    AddRel("BLE", CPU6800, 0x2f);
    AddRel("BLO", CPU6800, 0x25);
    AddRel("BLS", CPU6800, 0x23);
    AddRel("BLT", CPU6800, 0x2d);
    AddRel("BMI", CPU6800, 0x2b);
    AddRel("BNE", CPU6800, 0x26);
    AddRel("BPL", CPU6800, 0x2a);
    AddRel("BRA", CPU6800, 0x20);
    AddRel("BRN", CPU6801, 0x21);
    AddRel("BSR", CPU6800, 0x8d);
    AddRel("BVC", CPU6800, 0x28);
    AddRel("BVS", CPU6800, 0x29);

    AddALU8("ADC", True, 0x89);
    AddALU8("ADD", True, 0x8b);
    AddALU8("AND", True, 0x84);
    AddALU8("BIT", True, 0x85);
    AddALU8("CMP", True, 0x81);
    AddALU8("EOR", True, 0x88);
    AddALU8("LDA", True, 0x86);
    AddInstTable(InstTable, "LDB", 0x86 | (1 << 8) | 0xc000, DecodeALU8);
    AddALU8("ORA", True, 0x8a);
    AddInstTable(InstTable, "ORB", 0x8a | (1 << 8) | 0xc000, DecodeALU8);
    AddALU8("SBC", True, 0x82);
    AddALU8("STA", False, 0x87);
    AddInstTable(InstTable, "STB", 0x87 | (1 << 8) | 0xc000, DecodeALU8);
    AddALU8("SUB", True, 0x80);

    InstrZ = 0;
    AddALU16("ADDD", True, CPU6801, 0, 0xc3);
    AddALU16("CPD", True, CPU6811, 1, 0x83);
    AddALU16("CMPD", True, CPU6811, 1, 0x83);
    AddALU16("CPX", True, CPU6800, 2, 0x8c);
    AddALU16("CMPX", True, CPU6800, 2, 0x8c);
    AddALU16("CPY", True, CPU6811, 3, 0x8c);
    AddALU16("CMPY", True, CPU6811, 3, 0x8c);
    AddALU16("LDD", True, CPU6801, 0, 0xcc);
    AddALU16("LDS", True, CPU6800, 0, 0x8e);
    AddALU16("LDX", True, CPU6800, 2, 0xce);
    AddALU16("LDY", True, CPU6811, 3, 0xce);
    AddALU16("STD", False, CPU6801, 0, 0xcd);
    AddALU16("STS", False, CPU6800, 0, 0x8f);
    AddALU16("STX", False, CPU6800, 2, 0xcf);
    AddALU16("STY", False, CPU6811, 3, 0xcf);
    AddALU16("SUBD", True, CPU6801, 0, 0x83);

    AddSing8("ASL", 0x48);
    AddSing8("ASR", 0x47);
    AddSing8("CLR", 0x4f);
    AddSing8("COM", 0x43);
    AddSing8("DEC", 0x4a);
    AddSing8("INC", 0x4c);
    AddSing8("LSL", 0x48);
    AddSing8("LSR", 0x44);
    AddSing8("NEG", 0x40);
    AddSing8("ROL", 0x49);
    AddSing8("ROR", 0x46);
    AddSing8("TST", 0x4d);

    AddInstTable(InstTable, "PSH", 0x36, DecodePSH_PUL);
    AddInstTable(InstTable, "PSHA", 0x36, DecodeSing8_Acc);
    AddInstTable(InstTable, "PSHB", 0x37, DecodeSing8_Acc);
    AddInstTable(InstTable, "PUL", 0x32, DecodePSH_PUL);
    AddInstTable(InstTable, "PULA", 0x32, DecodeSing8_Acc);
    AddInstTable(InstTable, "PULB", 0x33, DecodeSing8_Acc);

    AddInstTable(InstTable, "AIM", 0x61, DecodeBit63);
    AddInstTable(InstTable, "EIM", 0x65, DecodeBit63);
    AddInstTable(InstTable, "OIM", 0x62, DecodeBit63);
    AddInstTable(InstTable, "TIM", 0x6b, DecodeBit63);

    AddInstTable(InstTable, "PRWINS", 0, DecodePRWINS);

    add_moto8_pseudo(InstTable, e_moto_pseudo_flags_be);
    add_moto8_comment_onoff();
    AddMoto16Pseudo(InstTable, e_moto_pseudo_flags_be);
    AddInstTable(
            InstTable, "DB",
            eIntPseudoFlag_BigEndian | eIntPseudoFlag_AllowInt
                    | eIntPseudoFlag_AllowString | eIntPseudoFlag_MotoRep,
            DecodeIntelDB);
    AddInstTable(
            InstTable, "DW",
            eIntPseudoFlag_BigEndian | eIntPseudoFlag_AllowInt
                    | eIntPseudoFlag_AllowString | eIntPseudoFlag_MotoRep,
            DecodeIntelDW);
}

static void DeinitFields(void) {
    DestroyInstTable(InstTable);
    order_array_free(FixedOrders);
    order_array_free(RelOrders);
    order_array_free(ALU16Orders);
}

static Boolean DecodeAttrPart_68(void) {
    if (strlen(AttrPart.str.p_str) > 1) {
        WrStrErrorPos(ErrNum_UndefAttr, &AttrPart);
        return False;
    }
    return DecodeMoto16AttrSize(*AttrPart.str.p_str, &AttrPartOpSize[0], False);
}

static void MakeCode_68(void) {
    PrefCnt = 0;
    AdrCnt  = 0;

    /* Operandengroesse festlegen */

    if (AttrPartOpSize[0] == eSymbolSizeUnknown) {
        AttrPartOpSize[0] = eSymbolSize8Bit;
    }

    /* gehashtes */

    if (!LookupInstTable(InstTable, OpPart.str.p_str)) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
    }
}

static void InitCode_68(void) {
    Reg_MMSIZ = Reg_MMWBR = Reg_MM1CR = Reg_MM2CR = 0;
}

static Boolean IsDef_68(void) {
    return False;
}

static void SwitchTo_68(void) {
    TFamilyDescr const* p_descr = FindFamilyByName("68xx");

    TurnWords = False;
    SetIntConstMode(eIntConstModeMoto);

    PCSymbol    = "*";
    HeaderID    = p_descr->Id;
    NOPCode     = 0x01;
    DivideChars = ",";
    HasAttrs    = True;
    AttrChars   = ".";

    ValidSegs          = 1 << SegCode;
    Grans[SegCode]     = 1;
    ListGrans[SegCode] = 1;
    SegInits[SegCode]  = 0;
    SegLimits[SegCode] = (MomCPU == CPU68HC11K4) ? 0x10ffffl : 0xffff;

    DecodeAttrPart = DecodeAttrPart_68;
    MakeCode       = MakeCode_68;
    IsDef          = IsDef_68;
    SwitchFrom     = DeinitFields;
    InitFields();
    AddMoto16PseudoONOFF(False);

    if (MomCPU == CPU68HC11K4) {
        static as_assume_rec_t const ASSUMEHC11s[] = {
                { "MMSIZ",  &Reg_MMSIZ, 0, 0xff, 0, SetK4Ranges},
                { "MMWBR",  &Reg_MMWBR, 0, 0xff, 0, SetK4Ranges},
                { "MM1CR",  &Reg_MM1CR, 0, 0xff, 0, SetK4Ranges},
                { "MM2CR",  &Reg_MM2CR, 0, 0xff, 0, SetK4Ranges},
                {  "INIT",   &Reg_INIT, 0, 0xff, 0, SetK4Ranges},
                { "INIT2",  &Reg_INIT2, 0, 0xff, 0, SetK4Ranges},
                {"CONFIG", &Reg_CONFIG, 0, 0xff, 0, SetK4Ranges},
        };

        assume_set(ASSUMEHC11s, as_array_size(ASSUMEHC11s));

        SetK4Ranges();
    } else {
        cpu_2_phys_area_clear(SegCode);
    }
}

void code68_init(void) {
    CPU6800     = AddCPU("6800", SwitchTo_68);
    CPU6801     = AddCPU("6801", SwitchTo_68);
    CPU6301     = AddCPU("6301", SwitchTo_68);
    CPU6811     = AddCPU("6811", SwitchTo_68);
    CPU68HC11K4 = AddCPU("68HC11K4", SwitchTo_68);

    AddInitPassProc(InitCode_68);
}
