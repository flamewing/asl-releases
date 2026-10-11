/* code7000.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator SH7x00                                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "code7000.h"

#include "as.h"
#include "asmallg.h"
#include "asmcode.h"
#include "asmdef.h"
#include "asmitree.h"
#include "asmpars.h"
#include "asmsub.h"
#include "bpemu.h"
#include "codepseudo.h"
#include "codevars.h"
#include "errmsg.h"
#include "headids.h"
#include "literals.h"
#include "motpseudo.h"
#include "nls.h"
#include "onoff_common.h"
#include "strutil.h"

#include <string.h>

typedef enum {
    ModReg          = 0,
    ModIReg         = 1,
    ModPreDec       = 2,
    ModPostInc      = 3,
    ModIndReg       = 4,
    ModR0Base       = 5,
    ModGBRBase      = 6,
    ModGBRR0        = 7,
    ModPCRel        = 8,
    ModImm          = 9,
    ModFReg         = 10,
    ModDReg         = 11,
    ModFVReg        = 12,
    ModIndReg12     = 13,
    ModTBRBase      = 14,
    ModPostIncByReg = 15,
    ModNone         = -1
} adr_mode_t;

#define MModReg          (1 << ModReg)
#define MModIReg         (1 << ModIReg)
#define MModPreDec       (1 << ModPreDec)
#define MModPostInc      (1 << ModPostInc)
#define MModIndReg       (1 << ModIndReg)
#define MModR0Base       (1 << ModR0Base)
#define MModGBRBase      (1 << ModGBRBase)
#define MModGBRR0        (1 << ModGBRR0)
#define MModPCRel        (1 << ModPCRel)
#define MModImm          (1 << ModImm)
#define MModFReg         (1 << ModFReg)
#define MModDReg         (1 << ModDReg)
#define MModFVReg        (1 << ModFVReg)
#define MModIndReg12     (1 << ModIndReg12)
#define MModTBRBase      (1 << ModTBRBase)
#define MModPostIncByReg (1 << ModPostIncByReg)

#define REG_FP 14
#define REG_SP 15

#define CompLiteralsName "COMPRESSEDLITERALS"

typedef enum {
    e_core_sh1,
    e_core_sh2,
    e_core_sh2e,
    e_core_sh2a,
    e_core_sh3,
    e_core_sh3e,
    e_core_sh4,
    e_core_sh4a,
    e_core_sh5
} core_t;

typedef enum {
    e_core_mask_sh1         = 1 << e_core_sh1,
    e_core_mask_sh2         = 1 << e_core_sh2,
    e_core_mask_sh2e        = 1 << e_core_sh2e,
    e_core_mask_sh2a        = 1 << e_core_sh2a,
    e_core_mask_sh3         = 1 << e_core_sh3,
    e_core_mask_sh3e        = 1 << e_core_sh3e,
    e_core_mask_sh4         = 1 << e_core_sh4,
    e_core_mask_sh4a        = 1 << e_core_sh4a,
    e_core_mask_sh5         = 1 << e_core_sh5,
    e_core_mask_sh4a_higher = e_core_mask_sh4a | e_core_mask_sh5,
    e_core_mask_sh4_higher  = e_core_mask_sh4 | e_core_mask_sh4a_higher,
    e_core_mask_sh3e_higher = e_core_mask_sh3e | e_core_mask_sh4_higher,
    e_core_mask_sh3_higher  = e_core_mask_sh3 | e_core_mask_sh3e_higher,
    e_core_mask_sh2a_higher = e_core_mask_sh2a | e_core_mask_sh3_higher,
    e_core_mask_sh2e_higher = e_core_mask_sh2e | e_core_mask_sh2a_higher,
    e_core_mask_sh2_higher  = e_core_mask_sh2 | e_core_mask_sh2e_higher,
    e_core_mask_all         = e_core_mask_sh1 | e_core_mask_sh2_higher,
    e_core_mask_none        = 0
} core_mask_t;

typedef enum {
    e_core_flag_none  = 0,
    e_core_flag_fpu32 = 1 << 0,
    e_core_flag_fpu64 = 1 << 1,
    e_core_flag_dsp   = 1 << 2,
    e_core_flag_fpugr = 1 << 3
} core_flag_t;

typedef struct {
    core_mask_t core_mask;
    core_flag_t core_flags;
    Boolean     privileged;
    Word        code;
} FixedOrder;

typedef struct {
    core_mask_t core_mask;
    Boolean     privileged;
    Word        code;
    Boolean     Delayed;
} OneRegOrder;

typedef struct {
    core_t   min_core;
    Boolean  privileged;
    Word     code;
    ShortInt DefSize;
} TwoRegOrder;

typedef struct {
    core_t min_core;
    Word   code;
} FixedMinOrder;

typedef struct {
    char const* Name;
    Word        code;
    core_t      min_core;
    core_flag_t core_flag;
} TRegDef;

typedef struct {
    char        name[7];
    core_t      core;
    core_flag_t flags;
} cpu_props_t;

typedef struct {
    Word         part;
    Word         part_guess_mask;
    tSymbolFlags part_flags;
    tSymbolSize  part_op_size;
} adr_vals_t;

typedef enum {
    e_dsp_par_alu,
    e_dsp_par_pmul,
    e_dsp_par_movx,
    e_dsp_par_movy,
    e_dsp_par_count
} dsp_par_component_t;

#ifdef __cplusplus
#    include "code7000.hpp"
#endif

#define m_dsp_par_alu  (1 << e_dsp_par_alu)
#define m_dsp_par_pmul (1 << e_dsp_par_pmul)

static tSymbolSize op_size;

static cpu_props_t const* p_curr_cpu_props;

static FixedOrder*    fixed_orders;
static OneRegOrder*   one_reg_orders;
static TwoRegOrder*   two_reg_orders;
static FixedMinOrder* mul_reg_orders;
static FixedOrder*    bw_orders;
static TRegDef*       reg_defs;

static Boolean curr_delayed, prev_delayed, compress_literals, dsp_avail, this_par;
static LongInt delayed_addr;

static Word     dsp_condition;
static Word     dsp_par_acc[2], dsp_par_dest[e_dsp_par_count];
static unsigned dsp_par_mask;

/*-------------------------------------------------------------------------*/
/* die PC-relative Adresse: direkt nach verzoegerten Spruengen = Sprungziel+2 */

static LongInt pc_rel_adr(void) {
    if (prev_delayed) {
        return delayed_addr + 2;
    } else {
        return EProgCounter() + 4;
    }
}

static void chk_delayed(void) {
    if (prev_delayed) {
        WrError(ErrNum_Pipeline);
    }
}

/*!------------------------------------------------------------------------
 * \fn     get_act_cpu_flags(void)
 * \brief  get CPU core flags, reduced to what is enabled
 * \return flags
 * ------------------------------------------------------------------------ */

static core_flag_t get_act_cpu_flags(void) {
    core_flag_t flags = p_curr_cpu_props->flags;
    if (!dsp_avail) {
        flags &= ~e_core_flag_dsp;
    }
    if (!FPUAvail) {
        flags &= ~(e_core_flag_fpu32 | e_core_flag_fpu64);
    }
    return flags;
}

/*-------------------------------------------------------------------------*/
/* Register encoding/decoding */

/*!------------------------------------------------------------------------
 * \fn     decode_cpu_reg_core(const char *p_arg, Word *p_result, tSymbolSize *p_size)
 * \brief  check whether argument is a CPU/FPU register
 * \param  p_arg source argument
 * \param  p_result register # if yes
 * \param  p_size register size if yes
 * \return True if yes
 * ------------------------------------------------------------------------ */

static Boolean decode_cpu_reg_core(
        char const* p_arg, Word* p_result, tSymbolSize* p_size) {
    size_t l;
    char*  p_end;

    struct reg_dscr_t {
        char        prefix[3];
        Byte        prefix_len, offset;
        tSymbolSize op_size;
        core_flag_t core_flags;
    };
    const struct reg_dscr_t reg_dscrs[] = {
            { "R", 1,  0,       eSymbolSize32Bit,  e_core_flag_none},
            {"FR", 2,  0,  eSymbolSizeFloat32Bit, e_core_flag_fpu32},
            {"XF", 2, 16,  eSymbolSizeFloat32Bit, e_core_flag_fpugr},
            {"DR", 2,  0,  eSymbolSizeFloat64Bit, e_core_flag_fpu64},
            {"XD", 2,  1,  eSymbolSizeFloat64Bit, e_core_flag_fpugr},
            {"FV", 2,  0, eSymbolSizeFloat128Bit, e_core_flag_fpugr},
            {  "", 0,  0,     eSymbolSizeUnknown,  e_core_flag_none}
    };
    const struct reg_dscr_t* p_reg_dscr;

    if (!as_strcasecmp(p_arg, "SP")) {
        *p_result = REG_SP | REGSYM_FLAG_ALIAS;
        if (p_size) {
            *p_size = eSymbolSize32Bit;
        }
        return True;
    }

    if (!as_strcasecmp(p_arg, "FP")) {
        *p_result = REG_FP | REGSYM_FLAG_ALIAS;
        if (p_size) {
            *p_size = eSymbolSize32Bit;
        }
        return True;
    }

    l = strlen(p_arg);
    for (p_reg_dscr = reg_dscrs; p_reg_dscr->prefix_len; p_reg_dscr++) {
        if ((!p_reg_dscr->core_flags
             || (p_curr_cpu_props->flags & p_reg_dscr->core_flags))
            && !as_strncasecmp(p_arg, p_reg_dscr->prefix, p_reg_dscr->prefix_len)) {
            if (p_size) {
                *p_size = p_reg_dscr->op_size;
            }
            p_arg += p_reg_dscr->prefix_len;
            l -= p_reg_dscr->prefix_len;
            break;
        }
    }
    if (!p_reg_dscr->prefix_len || (l < 1) || (l > 2)) {
        return False;
    }

    *p_result = strtoul(p_arg, &p_end, 10);
    if (*p_end || (*p_result > 15)
        || (*p_result & ((GetSymbolSizeBytes(p_reg_dscr->op_size) / 4) - 1))) {
        return False;
    }
    *p_result += p_reg_dscr->offset;
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     dissect_reg_sh(char *p_dest, size_t dest_size, tRegInt value, tSymbolSize inp_size)
 * \brief  dissect register symbols - SH variant
 * \param  p_dest destination buffer
 * \param  dest_size destination buffer size
 * \param  value numeric register value
 * \param  inp_size register size
 * ------------------------------------------------------------------------ */

static void dissect_reg_sh(
        char* p_dest, size_t dest_size, tRegInt value, tSymbolSize inp_size) {
    switch (inp_size) {
    case eSymbolSize32Bit:
        if (value == (REG_SP | REGSYM_FLAG_ALIAS)) {
            as_snprintf(p_dest, dest_size, "SP");
        } else if (value == (REG_FP | REGSYM_FLAG_ALIAS)) {
            as_snprintf(p_dest, dest_size, "FP");
        } else if (value > 15) {
            goto noneofall;
        } else {
            as_snprintf(p_dest, dest_size, "R%u", (unsigned)value);
        }
        break;
    case eSymbolSizeFloat32Bit:
        if (value > 31) {
            goto noneofall;
        } else if (value > 15) {
            as_snprintf(p_dest, dest_size, "XF%u", (unsigned)(value - 16));
        } else {
            as_snprintf(p_dest, dest_size, "FR%u", (unsigned)value);
        }
        break;
    case eSymbolSizeFloat64Bit:
        if (value > 15) {
            goto noneofall;
        } else {
            as_snprintf(
                    p_dest, dest_size, "%s%u", (value & 1) ? "XD" : "DR",
                    (unsigned)(value & 14));
        }
        break;
    case eSymbolSizeFloat128Bit:
        if ((value > 15) || (value & 3)) {
            goto noneofall;
        } else {
            as_snprintf(p_dest, dest_size, "FV%u", (unsigned)value);
        }
        break;
    default:
    noneofall:
        as_snprintf(p_dest, dest_size, "%d-%u", (int)inp_size, (unsigned)value);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_cpu_reg(const tStrComp *p_arg, Word *p_result, tSymbolSize *p_size, tSymbolSize req_size, Boolean must_be_reg)
 * \brief  check whether argument is a CPU/FPU register or register alias
 * \param  p_arg source argument
 * \param  p_result register # if yes
 * \param  p_size register size/type if yes
 * \param  req_size requested size
 * \param  must_be_reg argument is expected to be a register
 * \return eval result
 * ------------------------------------------------------------------------ */

static tRegEvalResult decode_cpu_reg(
        tStrComp const* p_arg, Word* p_result, tSymbolSize* p_size, tSymbolSize req_size,
        Boolean must_be_reg) {
    tRegEvalResult reg_eval_result;
    tEvalResult    eval_result;
    tRegDescr      reg_descr;

    if (decode_cpu_reg_core(p_arg->str.p_str, p_result, &eval_result.DataSize)) {
        *p_result &= ~REGSYM_FLAG_ALIAS;
        reg_eval_result = eIsReg;
    } else {
        reg_eval_result = EvalStrRegExpressionAsOperand(
                p_arg, &reg_descr, &eval_result, req_size, must_be_reg);
        if (reg_eval_result == eIsReg) {
            *p_result = reg_descr.Reg & ~REGSYM_FLAG_ALIAS;
        }
    }

    if (reg_eval_result == eIsReg) {
        if ((req_size != eSymbolSizeUnknown) && (eval_result.DataSize != req_size)) {
            WrStrErrorPos(ErrNum_InvOpSize, p_arg);
            reg_eval_result = must_be_reg ? eIsNoReg : eRegAbort;
        }
    }

    if (p_size) {
        *p_size = eval_result.DataSize;
    }
    return reg_eval_result;
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_ds(const tStrComp *p_arg, Word *p_ret)
 * \brief  handle Ds DSP register argument
 * \param  p_arg source argument
 * \param  p_ret encoded register
 * \return True if encoded
 * ------------------------------------------------------------------------ */

#define REG_DSP_NONE 0
#define REG_DSP_A0   7
#define REG_DSP_A0G  15
#define REG_DSP_A1   5
#define REG_DSP_A1G  13
#define REG_DSP_X0   8
#define REG_DSP_X1   9
#define REG_DSP_Y0   10
#define REG_DSP_Y1   11
#define REG_DSP_M0   12
#define REG_DSP_M1   14

static Boolean decode_dsp_reg_ds(tStrComp const* p_arg, Word* p_ret) {
    char const* p_str = p_arg->str.p_str;
    int         step  = 1;

    switch (as_toupper(p_str[0])) {
    case 'A':
        *p_ret = REG_DSP_A0;
        step   = REG_DSP_A1 - REG_DSP_A0;
        goto num;
    case 'X':
        *p_ret = REG_DSP_X0;
        goto num;
    case 'Y':
        *p_ret = REG_DSP_Y0;
        goto num;
    case 'M':
        *p_ret = REG_DSP_M0;
        step   = REG_DSP_M1 - REG_DSP_M0;
        goto num;
    default:
        break;
    num:
        if ((p_str[1] >= '0') && (p_str[1] <= '1')) {
            *p_ret += step * (p_str[1] - '0');
            if (!p_str[2]) {
                return True;
            } else if ((as_toupper(p_str[2]) == 'G') && !p_str[3]) {
                *p_ret += REG_DSP_A0G - REG_DSP_A0;
                return True;
            }
        }
    }
    return False;
}

/*!------------------------------------------------------------------------
 * \fn     encode_dsp_reg_ds(char *p_dest, size_t dest_size, Word reg)
 * \brief  transform encoded register back to textual form
 * \param  p_dest dest buffer
 * \param  dest_size dest buffer size
 * \param  reg register encoded in Ds format
 * ------------------------------------------------------------------------ */

static void encode_dsp_reg_ds(char* p_dest, size_t dest_size, Word reg) {
    switch (reg) {
    case REG_DSP_X0:
    case REG_DSP_X1:
        as_snprintf(p_dest, dest_size, "X%c", (char)(reg - REG_DSP_X0 + '0'));
        break;
    case REG_DSP_Y0:
    case REG_DSP_Y1:
        as_snprintf(p_dest, dest_size, "Y%c", (char)(reg - REG_DSP_X0 + '0'));
        break;
    case REG_DSP_M0:
        strmaxcpy(p_dest, "M0", dest_size);
        break;
    case REG_DSP_M1:
        strmaxcpy(p_dest, "M1", dest_size);
        break;
    case REG_DSP_A0:
        strmaxcpy(p_dest, "A0", dest_size);
        break;
    case REG_DSP_A1:
        strmaxcpy(p_dest, "A1", dest_size);
        break;
    case REG_DSP_A0G:
        strmaxcpy(p_dest, "A0G", dest_size);
        break;
    case REG_DSP_A1G:
        strmaxcpy(p_dest, "A1G", dest_size);
        break;
    default:
        as_snprintf(p_dest, dest_size, "%u", (unsigned)reg);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_dz(const tStrComp *p_arg, Word *p_ret)
 * \brief  handle Dz DSP register argument
 * \param  p_arg source argument
 * \param  p_ret encoded register
 * \return True if encoded
 * ------------------------------------------------------------------------ */

static Boolean decode_dsp_reg_dz(tStrComp const* p_arg, Word* p_ret) {
    return decode_dsp_reg_ds(p_arg, p_ret) && (*p_ret != REG_DSP_A0G)
           && (*p_ret != REG_DSP_A1G);
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_da(const tStrComp *p_arg, Word *p_ret)
 * \brief  handle Da DSP register argument
 * \param  p_arg source argument
 * \param  p_ret encoded register
 * \return True if encoded
 * ------------------------------------------------------------------------ */

static Boolean decode_dsp_reg_da(tStrComp const* p_arg, Word* p_ret) {
    if (!decode_dsp_reg_dz(p_arg, p_ret)) {
        return False;
    }
    switch (*p_ret) {
    case REG_DSP_A0:
        *p_ret = 0;
        break;
    case REG_DSP_A1:
        *p_ret = 1;
        break;
    default:
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_dxy(const tStrComp *p_arg, dsp_par_component_t dsp_par, Word *p_Dz, Word *p_ret)
 * \brief  handle Dx/Dy DSP register argument
 * \param  p_arg source argument
 * \param  dsp_par Dx or Dy?
 * \param  p_Dz encoded register in Dz format
 * \param  p_ret encoded register in Dx/Dy format
 * \return True if encoded
 * ------------------------------------------------------------------------ */

static Boolean decode_dsp_reg_dxy(
        tStrComp const* p_arg, dsp_par_component_t dsp_par, Word* p_Dz, Word* p_ret) {
    if (!decode_dsp_reg_dz(p_arg, p_Dz)) {
        return False;
    }
    switch (*p_Dz) {
    case REG_DSP_X0:
    case REG_DSP_X1:
        if (dsp_par != e_dsp_par_movx) {
            return False;
        }
        *p_ret = *p_Dz - REG_DSP_X0;
        break;
    case REG_DSP_Y0:
    case REG_DSP_Y1:
        if (dsp_par != e_dsp_par_movy) {
            return False;
        }
        *p_ret = *p_Dz - REG_DSP_Y0;
        break;
    default:
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_sx(const tStrComp *p_arg, Word *p_ret)
 * \brief  handle Sx DSP register argument
 * \param  p_arg source argument
 * \param  p_ret encoded register
 * \return True if encoded
 * ------------------------------------------------------------------------ */

static Boolean decode_dsp_reg_sx(tStrComp const* p_arg, Word* p_ret) {
    if (!decode_dsp_reg_dz(p_arg, p_ret)) {
        return False;
    }
    switch (*p_ret) {
    case REG_DSP_X0:
    case REG_DSP_X1:
        *p_ret -= REG_DSP_X0;
        break;
    case REG_DSP_A0:
        *p_ret = 2;
        break;
    case REG_DSP_A1:
        *p_ret = 3;
        break;
    default:
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_sy(const tStrComp *p_arg, Word *p_ret)
 * \brief  handle Sy DSP register argument
 * \param  p_arg source argument
 * \param  p_ret encoded register
 * \return True if encoded
 * ------------------------------------------------------------------------ */

static Boolean decode_dsp_reg_sy(tStrComp const* p_arg, Word* p_ret) {
    if (!decode_dsp_reg_dz(p_arg, p_ret)) {
        return False;
    }
    switch (*p_ret) {
    case REG_DSP_Y0:
    case REG_DSP_Y1:
        *p_ret -= REG_DSP_Y0;
        break;
    case REG_DSP_M0:
        *p_ret = 2;
        break;
    case REG_DSP_M1:
        *p_ret = 3;
        break;
    default:
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_se(const tStrComp *p_arg, Word *p_ret)
 * \brief  handle Se DSP register argument
 * \param  p_arg source argument
 * \param  p_ret encoded register
 * \return True if encoded
 * ------------------------------------------------------------------------ */

static Boolean decode_dsp_reg_se(tStrComp const* p_arg, Word* p_ret) {
    if (!decode_dsp_reg_dz(p_arg, p_ret)) {
        return False;
    }
    switch (*p_ret) {
    case REG_DSP_X0:
    case REG_DSP_X1:
        *p_ret -= REG_DSP_X0;
        break;
    case REG_DSP_Y0:
        *p_ret = 2;
        break;
    case REG_DSP_A1:
        *p_ret = 3;
        break;
    default:
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_sf(const tStrComp *p_arg, Word *p_ret)
 * \brief  handle Sf DSP register argument
 * \param  p_arg source argument
 * \param  p_ret encoded register
 * \return True if encoded
 * ------------------------------------------------------------------------ */

static Boolean decode_dsp_reg_sf(tStrComp const* p_arg, Word* p_ret) {
    if (!decode_dsp_reg_dz(p_arg, p_ret)) {
        return False;
    }
    switch (*p_ret) {
    case REG_DSP_Y0:
    case REG_DSP_Y1:
        *p_ret -= REG_DSP_Y0;
        break;
    case REG_DSP_X0:
        *p_ret = 2;
        break;
    case REG_DSP_A1:
        *p_ret = 3;
        break;
    default:
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_reg_dg(const tStrComp *p_arg, Word *p_Dz, Word *p_ret)
 * \brief  handle Dg DSP register argument
 * \param  p_arg source argument
 * \param  p_Dz encoded register in Dz format
 * \param  p_ret encoded register in Dg format
 * \return True if encoded
 * ------------------------------------------------------------------------ */

static Boolean decode_dsp_reg_dg(tStrComp const* p_arg, Word* p_Dz, Word* p_ret) {
    if (!decode_dsp_reg_dz(p_arg, p_Dz)) {
        return False;
    }
    switch (*p_Dz) {
    case REG_DSP_M0:
        *p_ret = 0;
        break;
    case REG_DSP_M1:
        *p_ret = 1;
        break;
    case REG_DSP_A0:
        *p_ret = 2;
        break;
    case REG_DSP_A1:
        *p_ret = 3;
        break;
    default:
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     dsp_reg_ds_to_du(Word Dz, Word *p_Du)
 * \brief  check whether register matches Du restrictions
 * \param  Dz register in Dz encoding
 * \param  p_Du register in Du encoding
 * \return True if Dz could be transformed to Du
 * ------------------------------------------------------------------------ */

static Boolean dsp_reg_ds_to_du(Word Dz, Word* p_Du) {
    switch (Dz) {
    case REG_DSP_X0:
        *p_Du = 0;
        break;
    case REG_DSP_Y0:
        *p_Du = 1;
        break;
    case REG_DSP_A0:
        *p_Du = 2;
        break;
    case REG_DSP_A1:
        *p_Du = 3;
        break;
    default:
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     decode_ctrl_reg(const tStrComp *p_arg_comp, Word *p_ret)
 * \brief  check whether argument is control register
 * \param  p_arg_comp source argument
 * \param  p_ret return buffer for encoded register
 * \return True if argument is contorl register
 * ------------------------------------------------------------------------ */

#define CTRL_REG_SR     0
#define CTRL_REG_GBR    1
#define CTRL_REG_VBR    2
#define CTRL_REG_SSR    3
#define CTRL_REG_SPC    4
#define CTRL_REG_R0BANK 8
#define CTRL_REG_SGR    0x13
#define CTRL_REG_TBR    0x14
#define CTRL_REG_DBR    0x1f

static Boolean decode_ctrl_reg(tStrComp const* p_arg_comp, Word* p_ret) {
    core_mask_t core_mask = e_core_mask_all;
    char const* p_arg     = p_arg_comp->str.p_str;

    *p_ret = 0xff;
    if (!as_strcasecmp(p_arg, "SR")) {
        *p_ret = CTRL_REG_SR;
    } else if (!as_strcasecmp(p_arg, "GBR")) {
        *p_ret = CTRL_REG_GBR;
    } else if (!as_strcasecmp(p_arg, "VBR")) {
        *p_ret = CTRL_REG_VBR;
    } else if (!as_strcasecmp(p_arg, "SSR")) {
        *p_ret    = CTRL_REG_SSR;
        core_mask = e_core_mask_sh3_higher;
    } else if (!as_strcasecmp(p_arg, "SPC")) {
        *p_ret    = CTRL_REG_SPC;
        core_mask = e_core_mask_sh3_higher;
    } else if (!as_strcasecmp(p_arg, "DBR")) {
        *p_ret    = CTRL_REG_DBR;
        core_mask = e_core_mask_sh4_higher;
    } else if (!as_strcasecmp(p_arg, "SGR")) {
        *p_ret    = CTRL_REG_SGR;
        core_mask = e_core_mask_sh4_higher;
    } else if (!as_strcasecmp(p_arg, "TBR")) {
        *p_ret    = CTRL_REG_TBR;
        core_mask = e_core_mask_sh2a;
    } else if (
            (strlen(p_arg) == 7) && (as_toupper(*p_arg) == 'R')
            && (!as_strcasecmp(p_arg + 2, "_BANK")) && (p_arg[1] >= '0')
            && (p_arg[1] <= '7')) {
        *p_ret    = p_arg[1] - '0' + CTRL_REG_R0BANK;
        core_mask = e_core_mask_sh3_higher;
    }
    if ((*p_ret == 0xff) || !((core_mask >> p_curr_cpu_props->core) & 1)) {
        WrStrErrorPos(ErrNum_InvCtrlReg, p_arg_comp);
        return False;
    } else {
        return True;
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_s_reg(const char *p_arg, Word *p_ret)
 * \brief  decode s(pecial?) register
 * \param  p_arg source argument
 * \param  p_ret encoded register #
 * \return True if success
 * ------------------------------------------------------------------------ */

static Boolean decode_s_reg(char const* p_arg, Word* p_ret) {
    int     z;
    Boolean result = False;

    for (z = 0; reg_defs[z].Name; z++) {
        if (!as_strcasecmp(p_arg, reg_defs[z].Name)) {
            break;
        }
    }
    if (reg_defs[z].Name) {
        if (p_curr_cpu_props->core < reg_defs[z].min_core)
            ;
        else if (reg_defs[z].core_flag && !(get_act_cpu_flags() & reg_defs[z].core_flag))
            ;
        else {
            result = True;
            *p_ret = reg_defs[z].code;
        }
    }
    return result;
}

/*-------------------------------------------------------------------------*/
/* Adressparsing */

static LongInt ext_op(LongInt Inp, tSymbolSize op_size, Boolean Signed) {
    switch (op_size) {
    case eSymbolSize8Bit:
        Inp &= 0xff;
        break;
    case eSymbolSize16Bit:
        Inp &= 0xffff;
        break;
    default:
        break;
    }
    if (Signed) {
        if (op_size < eSymbolSize16Bit) {
            if ((Inp & 0x80) == 0x80) {
                Inp += 0xff00;
            }
        }
        if (op_size < eSymbolSize32Bit) {
            if ((Inp & 0x8000) == 0x8000) {
                Inp += 0xffff0000;
            }
        }
    }
    return Inp;
}

static LongInt op_mask(ShortInt op_size) {
    switch (op_size) {
    case eSymbolSize8Bit:
        return 0xff;
    case eSymbolSize16Bit:
        return 0xffff;
    case eSymbolSize32Bit:
        return 0xffffffff;
    default:
        return 0;
    }
}

/*!------------------------------------------------------------------------
 * \fn     set_opsize(tSymbolSize size, const tStrComp *p_arg)
 * \brief  assure certain operand size is set - change from unknown and check conflict
 * \param  size operand size to set/check
 * \param  p_arg associated source arg defining this size
 * \return True if OK
 * ------------------------------------------------------------------------ */

static Boolean set_opsize(tSymbolSize size, tStrComp const* p_arg) {
    if (op_size == eSymbolSizeUnknown) {
        op_size = size;
    } else if (size != op_size) {
        WrStrErrorPos(
                (p_arg == &AttrPart) ? ErrNum_InvOpSize : ErrNum_ConfOpSizes, p_arg);
        return False;
    }
    return True;
}

static void set_adr_part_guess_mask(adr_vals_t* p_vals, Word mask) {
    p_vals->part_guess_mask
            = mFirstPassUnknownOrQuestionable(p_vals->part_flags) ? mask : 0;
}

static void reset_adr_vals(adr_vals_t* p_vals) {
    p_vals->part = p_vals->part_guess_mask = 0;
    p_vals->part_flags                     = eSymbolFlag_None;
    p_vals->part_op_size                   = eSymbolSizeUnknown;
}

#define IREG_NONE ((Word) - 1)
#define IREG_PC   ((Word)(-2))
#define IREG_GBR  ((Word)(-3))
#define IREG_TBR  ((Word)(-4))

static adr_mode_t decode_adr(
        tStrComp const* pArg, adr_vals_t* p_vals, Word Mask, Boolean Signed) {
    int        num_indir;
    adr_mode_t adr_mode;

    adr_mode = ModNone;
    reset_adr_vals(p_vals);

    if ((op_size != eSymbolSizeUnknown) && !GetSymbolSizeBytes(op_size)) {
        WrStrErrorPos(ErrNum_InvOpSize, pArg);
        return adr_mode;
    }

    switch (decode_cpu_reg(
            pArg, &p_vals->part, &p_vals->part_op_size, eSymbolSizeUnknown, False)) {
    case eIsReg:
        if (p_vals->part_op_size == eSymbolSizeFloat128Bit) {
            adr_mode = ModFVReg;
        } else if (p_vals->part_op_size == eSymbolSizeFloat64Bit) {
            adr_mode = ModDReg;
        } else if (p_vals->part_op_size == eSymbolSizeFloat32Bit) {
            adr_mode = ModFReg;
        } else {
            adr_mode = ModReg;
        }
        goto chk;
    case eIsNoReg:
        break;
    case eRegAbort:
        return adr_mode;
    }

    for (num_indir = 0; (num_indir < 2) && (pArg->str.p_str[num_indir] == '@');
         num_indir++)
        ;
    if (num_indir > 0) {
        tStrComp Arg;

        StrCompRefRight(&Arg, pArg, num_indir);
        if (IsIndirect(Arg.str.p_str)) {
            tStrComp Remainder;
            char*    pos;
            Word     base_reg = IREG_NONE, ind_reg = IREG_NONE;
            LongInt  disp_acc = 0;
            Boolean  ok       = True;

            StrCompIncRefLeft(&Arg, 1);
            StrCompShorten(&Arg, 1);
            p_vals->part_flags = eSymbolFlag_None;
            do {
                Word this_reg;

                pos = QuotPos(Arg.str.p_str, ',');
                if (pos) {
                    StrCompSplitRef(&Arg, &Remainder, &Arg, pos);
                }
                if (!as_strcasecmp(Arg.str.p_str, "PC")) {
                    if (base_reg == IREG_NONE) {
                        base_reg = IREG_PC;
                    } else {
                        WrStrErrorPos(ErrNum_InvAddrMode, &Arg);
                        ok = False;
                    }
                } else if (!as_strcasecmp(Arg.str.p_str, "GBR")) {
                    if (base_reg == IREG_NONE) {
                        base_reg = IREG_GBR;
                    } else {
                        WrStrErrorPos(ErrNum_InvAddrMode, &Arg);
                        ok = False;
                    }
                } else if (!as_strcasecmp(Arg.str.p_str, "TBR")) {
                    if (base_reg == IREG_NONE) {
                        base_reg = IREG_TBR;
                    } else {
                        WrStrErrorPos(ErrNum_InvAddrMode, &Arg);
                        ok = False;
                    }
                } else {
                    switch (decode_cpu_reg(
                            &Arg, &this_reg, NULL, eSymbolSize32Bit, False)) {
                    case eIsReg:
                        if (ind_reg == IREG_NONE) {
                            ind_reg = this_reg;
                        } else if ((base_reg == IREG_NONE) && (this_reg == 0)) {
                            base_reg = 0;
                        } else if ((ind_reg == 0) && (base_reg == IREG_NONE)) {
                            base_reg = 0;
                            ind_reg  = this_reg;
                        } else {
                            WrStrErrorPos(ErrNum_InvAddrMode, &Arg);
                            ok = False;
                        }
                        break;
                    case eIsNoReg: {
                        tSymbolFlags flags;

                        disp_acc += EvalStrIntExpressionWithFlags(
                                &Arg, Int32, &ok, &flags);
                        p_vals->part_flags |= flags;
                        break;
                    }
                    case eRegAbort:
                        ok = False;
                    }
                }
                if (pos) {
                    Arg = Remainder;
                }
            } while (pos && ok);
            if (mFirstPassUnknownOrQuestionable(p_vals->part_flags)) {
                disp_acc = 0;
            }
            if (ok && (op_size != eSymbolSizeUnknown)
                && ((disp_acc & ((GetSymbolSizeBytes(op_size)) - 1)) != 0)) {
                WrStrErrorPos(ErrNum_NotAligned, pArg);
                ok = False;
            } else if (ok && (disp_acc < 0)) {
                WrXErrorPos(ErrNum_UnderRange, "Disp < 0", &pArg->Pos);
                ok = False;
            } else if (op_size != eSymbolSizeUnknown) {
                disp_acc /= GetSymbolSizeBytes(op_size);
            }
            if (ok) {
                switch (base_reg) {
                case 0:
                    if ((ind_reg > 15) || (disp_acc != 0) || (num_indir != 1)) {
                        WrStrErrorPos(ErrNum_InvAddrMode, pArg);
                    } else {
                        adr_mode     = ModR0Base;
                        p_vals->part = ind_reg;
                    }
                    break;
                case IREG_GBR:
                    if (num_indir != 1) {
                        WrStrErrorPos(ErrNum_InvAddrMode, pArg);
                    } else if ((ind_reg == 0) && (disp_acc == 0)) {
                        adr_mode = ModGBRR0;
                    } else if (ind_reg != IREG_NONE) {
                        WrStrErrorPos(ErrNum_InvAddrMode, pArg);
                    } else if (disp_acc > 255) {
                        WrStrErrorPos(ErrNum_OverRange, pArg);
                    } else {
                        adr_mode     = ModGBRBase;
                        p_vals->part = disp_acc;
                        set_adr_part_guess_mask(p_vals, 0xff);
                    }
                    break;
                case IREG_TBR:
                    if ((ind_reg != IREG_NONE) || (num_indir != 2)) {
                        WrStrErrorPos(ErrNum_InvAddrMode, pArg);
                    } else if (disp_acc > 255) {
                        WrStrErrorPos(ErrNum_OverRange, pArg);
                    } else {
                        adr_mode     = ModTBRBase;
                        p_vals->part = disp_acc;
                        set_adr_part_guess_mask(p_vals, 0xff);
                    }
                    break;
                case IREG_NONE:
                    if ((ind_reg == IREG_NONE) || (num_indir != 1)) {
                        WrStrErrorPos(ErrNum_InvAddrMode, pArg);
                    } else {
                        Boolean allow4  = !!(Mask & MModIndReg),
                                allow12 = !!(Mask & MModIndReg12);
                        if (mFirstPassUnknownOrQuestionable(p_vals->part_flags)) {
                            disp_acc &= allow12 ? 0xfff : 0xf;
                        }
                        if ((disp_acc <= 15) && (allow4 || !allow12)) {
                            adr_mode     = ModIndReg;
                            p_vals->part = (ind_reg << 4) + disp_acc;
                            set_adr_part_guess_mask(p_vals, 0xf);
                        } else if (!allow12 || (disp_acc > 0xfff)) {
                            WrStrErrorPos(ErrNum_OverRange, pArg);
                        } else {
                            adr_mode     = ModIndReg12;
                            p_vals->part = (ind_reg << 12) | (disp_acc & 0xfff);
                            set_adr_part_guess_mask(p_vals, 0xfff);
                        }
                    }
                    break;
                case IREG_PC:
                    if ((ind_reg != IREG_NONE) || (num_indir != 1)) {
                        WrStrErrorPos(ErrNum_InvAddrMode, pArg);
                    } else if (disp_acc > 255) {
                        WrStrErrorPos(ErrNum_OverRange, pArg);
                    } else {
                        adr_mode     = ModPCRel;
                        p_vals->part = disp_acc;
                        set_adr_part_guess_mask(p_vals, 0xff);
                    }
                    break;
                }
            }
            goto chk;
        } else /* !IsIndirect */
        {
            int ArgLen = strlen(Arg.str.p_str);

            if ((ArgLen > 1) && (*Arg.str.p_str == '-')) {
                StrCompIncRefLeft(&Arg, 1);
                if (decode_cpu_reg(&Arg, &p_vals->part, NULL, eSymbolSize32Bit, True)
                    == eIsReg) {
                    adr_mode = ModPreDec;
                }
            } else {
                char* p_inc = strchr(Arg.str.p_str, '+');

                if (!p_inc) {
                    if (decode_cpu_reg(&Arg, &p_vals->part, NULL, eSymbolSize32Bit, True)
                        == eIsReg) {
                        if (Mask & MModIReg) {
                            adr_mode = ModIReg;
                        } else if (Mask & MModIndReg) {
                            p_vals->part <<= 4;
                            adr_mode = ModIndReg;
                        } else {
                            p_vals->part <<= 12;
                            adr_mode = ModIndReg12;
                        }
                    }
                } else if (p_inc == Arg.str.p_str) /* no pre-increment */
                {
                    WrStrErrorPos(ErrNum_InvAddrMode, &Arg);
                } else if (!p_inc[1]) {
                    StrCompShorten(&Arg, 1);
                    if (decode_cpu_reg(&Arg, &p_vals->part, NULL, eSymbolSize32Bit, True)
                        == eIsReg) {
                        adr_mode = ModPostInc;
                    }
                } else {
                    tStrComp base_arg, inc_arg;
                    Word     inc_reg;

                    StrCompSplitRef(&base_arg, &inc_arg, &Arg, p_inc);
                    if ((decode_cpu_reg(
                                 &base_arg, &p_vals->part, NULL, eSymbolSize32Bit, True)
                         == eIsReg)
                        && (decode_cpu_reg(
                                    &inc_arg, &inc_reg, NULL, eSymbolSize32Bit, True)
                            == eIsReg)) {
                        if ((p_vals->part < 2) || (p_vals->part > 7)
                            || (inc_reg != 8 + !!(p_vals->part >= 6))) {
                            WrStrErrorPos(ErrNum_InvAddrMode, &Arg);
                        } else {
                            adr_mode = ModPostIncByReg;
                        }
                    }
                }
            }
            goto chk;
        }
    }

    if (*pArg->str.p_str == '#') {
        LongInt imm_value;
        Boolean ok;

        switch (op_size) {
        case eSymbolSize8Bit:
            imm_value = EvalStrIntExpressionOffsWithFlags(
                    pArg, 1, Int8, &ok, &p_vals->part_flags);
            break;
        case eSymbolSize16Bit:
            imm_value = EvalStrIntExpressionOffsWithFlags(
                    pArg, 1, Int16, &ok, &p_vals->part_flags);
            break;
        case eSymbolSize32Bit:
            imm_value = EvalStrIntExpressionOffsWithFlags(
                    pArg, 1, Int32, &ok, &p_vals->part_flags);
            break;
        default:
            imm_value          = 0;
            ok                 = True;
            p_vals->part_flags = eSymbolFlag_None;
        }
        if (ok) {
            Boolean     Critical = mFirstPassUnknown(p_vals->part_flags)
                                   || mUsesForwards(p_vals->part_flags);
            tSymbolSize DOpSize;

            /* minimale Groesse optimieren */

            DOpSize = (op_size == eSymbolSize8Bit)
                              ? eSymbolSize8Bit
                              : (Critical ? eSymbolSize16Bit : eSymbolSize8Bit);
            while (((ext_op(imm_value, DOpSize, Signed) ^ imm_value) & op_mask(op_size))
                   != 0) {
                DOpSize++;
            }
            if (DOpSize == eSymbolSize8Bit) {
                p_vals->part = imm_value & 0xff;
                adr_mode     = ModImm;
            } else if (Mask & MModPCRel) {
                tStrComp    LStrComp;
                String      LStr;
                tSymbolSize lit_size;
                Byte        data_offset = 0;
                LongInt     displacement;
                Boolean     ok;

                StrCompMkTemp(&LStrComp, LStr, sizeof(LStr));

                lit_size = (DOpSize == 2) ? eSymbolSize32Bit : eSymbolSize16Bit;

                literal_make(&LStrComp, &data_offset, imm_value, lit_size, Critical);

                /* Distanz abfragen - im naechsten Pass... */

                displacement = EvalStrIntExpressionWithFlags(
                                       &LStrComp, Int32, &ok, &p_vals->part_flags)
                               + data_offset;
                if (ok) {
                    if (mFirstPassUnknown(p_vals->part_flags)) {
                        displacement = 0;
                    } else if (lit_size == eSymbolSize32Bit) {
                        displacement = (displacement - (pc_rel_adr() & 0xfffffffc)) >> 2;
                    } else {
                        displacement = (displacement - pc_rel_adr()) >> 1;
                    }
                    if (displacement < 0) {
                        WrXError(ErrNum_UnderRange, "Disp < 0");
                    } else if (
                            (displacement > 255)
                            && !mSymbolQuestionable(p_vals->part_flags)) {
                        WrStrErrorPos(ErrNum_DistTooBig, &LStrComp);
                    } else {
                        adr_mode     = ModPCRel;
                        p_vals->part = displacement;
                        set_adr_part_guess_mask(p_vals, 0xff);
                        op_size = lit_size;
                    }
                }
            } else {
                WrStrErrorPos(ErrNum_InvAddrMode, pArg);
            }
        }
        goto chk;
    }

    /* absolut ueber PC-relativ abwickeln */

    if ((op_size != eSymbolSize16Bit) && (op_size != eSymbolSize32Bit)) {
        WrStrErrorPos(ErrNum_InvOpSize, pArg);
    } else {
        Boolean  ok;
        LongWord address
                = EvalStrIntExpressionWithFlags(pArg, UInt32, &ok, &p_vals->part_flags);

        if (ok) {
            LongInt displacement;
            if (mFirstPassUnknown(p_vals->part_flags)) {
                displacement = 0;
            } else if (op_size == eSymbolSize32Bit) {
                displacement = address - (pc_rel_adr() & 0xfffffffc);
            } else {
                displacement = address - pc_rel_adr();
            }
            if (displacement < 0) {
                WrXErrorPos(ErrNum_UnderRange, "Disp < 0", &pArg->Pos);
            } else if ((displacement & (GetSymbolSizeBytes(op_size) - 1))) {
                WrStrErrorPos(ErrNum_NotAligned, pArg);
            } else {
                displacement /= GetSymbolSizeBytes(op_size);
                if (displacement > 255) {
                    WrStrErrorPos(ErrNum_OverRange, pArg);
                } else {
                    adr_mode     = ModPCRel;
                    p_vals->part = displacement & 0xff;
                    set_adr_part_guess_mask(p_vals, 0xff);
                }
            }
        }
    }

chk:
    if ((adr_mode != ModNone) && ((Mask & (1 << adr_mode)) == 0)) {
        WrStrErrorPos(ErrNum_InvAddrMode, pArg);
        reset_adr_vals(p_vals);
        adr_mode = ModNone;
    }
    return adr_mode;
}

/*-------------------------------------------------------------------------*/
/* Instruction Decoder Helpers */

/*!------------------------------------------------------------------------
 * \fn     reset_dsp_par_state(void)
 * \brief  reset state of current parallel DSP construct
 * ------------------------------------------------------------------------ */

static void reset_dsp_par_state(void) {
    dsp_par_component_t comp;

    dsp_par_acc[0] = dsp_par_acc[1] = 0x0000;
    dsp_par_mask                    = 0;
    for (comp = (dsp_par_component_t)0; comp < e_dsp_par_count; comp++) {
        dsp_par_dest[comp] = 0;
    }
}

/*!------------------------------------------------------------------------
 * \fn     set_code(Word code)
 * \brief  set single word machine code
 * \param  code machine code to set
 * ------------------------------------------------------------------------ */

static void set_code(Word code) {
    if (dsp_condition) {
        WrStrErrorPos(ErrNum_NoCondExec, &OpPart);
        dsp_condition = 0;
        return;
    }
    CodeLen     = 2;
    WAsmCode[0] = code;
    reset_dsp_par_state();
}

/*!------------------------------------------------------------------------
 * \fn     set_double_code(Word code1, Word code2)
 * \brief  set double word machine code
 * \param  code1, code2 machine codes to set
 * ------------------------------------------------------------------------ */

static void set_double_code(Word code1, Word code2) {
    if (dsp_condition) {
        WrStrErrorPos(ErrNum_NoCondExec, &OpPart);
        dsp_condition = 0;
        return;
    }
    CodeLen     = 4;
    WAsmCode[0] = code1;
    WAsmCode[1] = code2;
    reset_dsp_par_state();
}

/*!------------------------------------------------------------------------
 * \fn     augment_dsp_par_instr(Word code, dsp_par_component_t component, Word dest_reg)
 * \brief  augment/create DSP parallel instruction
 * \param  code machine code
 * \param  component component to set (ALU/X/Y)
 * \param  dest_reg used destination register in Dz format
 * ------------------------------------------------------------------------ */

static void augment_dsp_par_instr(
        Word code, dsp_par_component_t component, Word dest_reg) {
    dsp_par_component_t comp;

#if 0
  fprintf(stderr, "augment this_par %d comp %d code 0x%04x\n", this_par, component, code);
#endif

    /* merge in code or create new parallel context */

    if (this_par) {
        if (!dsp_par_mask || (dsp_par_mask & (1 << component))) {
            WrError(ErrNum_InvParConstruct);
            return;
        } else {
            RetractWords((dsp_par_mask & (m_dsp_par_alu | m_dsp_par_pmul)) ? 4 : 2);
            dsp_par_mask |= 1 << component;
        }
    } else {
        dsp_par_acc[0] = 0xf000;
        dsp_par_mask   = 1 << component;
    }
    if (dsp_par_mask & (m_dsp_par_alu | m_dsp_par_pmul)) {
        dsp_par_acc[0] |= 0x0800;
    }
    if ((component == e_dsp_par_alu) || (component == e_dsp_par_pmul)) {
        dsp_par_acc[1] = code;
    } else {
        dsp_par_acc[0] |= code;
    }

    /* check for same destination on instruction in same parallel construct */

    dsp_par_dest[component] = dest_reg;
    if (dest_reg) {
        for (comp = (dsp_par_component_t)0; comp < e_dsp_par_count; comp++) {
            if ((comp != component) && (dsp_par_mask & (1 << comp))
                && (dsp_par_dest[comp] == dest_reg)) {
                char buf[10];
                encode_dsp_reg_ds(buf, sizeof buf, dest_reg);
                WrXError(ErrNum_ConflictingParallelDest, buf);
            }
        }
    }

    /* write out (replacement) code */

    WAsmCode[0] = dsp_par_acc[0];
    CodeLen     = 2;
    if (dsp_par_acc[0] & 0x0800) {
        WAsmCode[1] = dsp_par_acc[1];
        CodeLen     = 4;
    }
}

/*!------------------------------------------------------------------------
 * \fn     set_dsp_double_code(Word code2, Boolean allow_conditional)
 * \brief  set double word DSP machine code
 * \param  code2 second machine code to set
 * \param  allow_conditional DCT/DCF prefix allowed?
 * ------------------------------------------------------------------------ */

static void set_dsp_double_code(Word code2, Boolean allow_conditional) {
    if (dsp_condition && !allow_conditional) {
        WrStrErrorPos(ErrNum_NoCondExec, &OpPart);
        dsp_condition = 0;
        return;
    }
    augment_dsp_par_instr(code2 + (dsp_condition << 8), e_dsp_par_alu, code2 & 0x000f);
    dsp_condition = 0;
}

/*!------------------------------------------------------------------------
 * \fn     chk_min_core(core_t min_core)
 * \brief  check instruction-specific minimum CPU core
 * \param  min_core min CPU core enum
 * \return True if fulfilled
 * ------------------------------------------------------------------------ */

static Boolean chk_min_core(core_t min_core) {
    if (p_curr_cpu_props->core < min_core) {
        WrError(ErrNum_InstructionNotSupported);
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     chk_core_mask(core_mask_t core_mask)
 * \brief  check instruction-specific CPU core list
 * \param  min_core min CPU core enum
 * \return True if fulfilled
 * ------------------------------------------------------------------------ */

static Boolean chk_core_mask(core_mask_t core_mask) {
    if ((core_mask >> p_curr_cpu_props->core) & 1) {
        return True;
    }
    WrError(ErrNum_InstructionNotSupported);
    return False;
}

/*!------------------------------------------------------------------------
 * \fn     Boolean chk_fpu(core_flag_t flags)
 * \brief  check for availibility of FPU
 * \param  flags type of FPU required
 * \return True if available
 * ------------------------------------------------------------------------ */

static Boolean chk_fpu(core_flag_t flags) {
    if (!flags) {
        return True;
    }
    if (!(p_curr_cpu_props->flags & flags)) {
        WrStrErrorPos(ErrNum_InstructionNotSupported, &OpPart);
        return False;
    } else if (!FPUAvail) {
        WrStrErrorPos(ErrNum_FPUNotEnabled, &OpPart);
        return False;
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     chk_no_opsize(void)
 * \brief  assure no attribute/operand size is set
 * \return True if OK
 * ------------------------------------------------------------------------ */

static Boolean chk_no_opsize(void) {
    if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
        return False;
    }
    return True;
}

/*-------------------------------------------------------------------------*/
/* Instruction Decoders */

/*!------------------------------------------------------------------------
 * \fn     decode_fixed(Word index)
 * \brief  Handle instructions with no argument
 * \param  index * to instruction table
 * ------------------------------------------------------------------------ */

static void decode_fixed(Word index) {
    FixedOrder const* p_order = fixed_orders + index;

    if (ChkArgCnt(0, 0) && chk_no_opsize() && chk_core_mask(p_order->core_mask)
        && chk_fpu(p_order->core_flags)) {
        set_code(p_order->code);
        if (!SupAllowed && p_order->privileged) {
            WrStrErrorPos(ErrNum_PrivOrder, &OpPart);
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_mov(Word code)
 * \brief  handle MOV instruction
 * ------------------------------------------------------------------------ */

static void decode_mov(Word code) {
    Word       add_mode_ask = (p_curr_cpu_props->core == e_core_sh2a)
                                      ? (MModPostInc | MModPreDec | MModIndReg12)
                                      : 0;
    adr_vals_t src_adr_vals;
    UNUSED(code);

    if (op_size == eSymbolSizeUnknown) {
        set_opsize(eSymbolSize32Bit, NULL);
    }
    if (!ChkArgCnt(2, 2))
        ;
    else if (op_size > eSymbolSize32Bit) {
        WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
    } else {
        switch (decode_adr(
                &ArgStr[1], &src_adr_vals,
                MModReg | MModImm | MModPCRel | MModIReg | MModPostInc | MModIndReg
                        | MModR0Base | MModGBRBase | add_mode_ask,
                True)) {
        case ModReg: {
            adr_vals_t dest_adr_vals;

            switch (decode_adr(
                    &ArgStr[2], &dest_adr_vals,
                    MModReg | MModIReg | MModPreDec | MModIndReg | MModR0Base
                            | MModGBRBase | add_mode_ask,
                    True)) {
            case ModReg:
                if (op_size != eSymbolSize32Bit) {
                    WrStrErrorPos(ErrNum_InvOpSize, &ArgStr[2]);
                } else {
                    set_code(
                            0x6003 + (src_adr_vals.part << 4)
                            + (dest_adr_vals.part << 8));
                }
                break;
            case ModIReg:
                set_code(
                        0x2000 + (src_adr_vals.part << 4) + (dest_adr_vals.part << 8)
                        + op_size);
                break;
            case ModPreDec:
                set_code(
                        0x2004 + (src_adr_vals.part << 4) + (dest_adr_vals.part << 8)
                        + op_size);
                break;
            case ModPostInc:
                if (src_adr_vals.part != 0) {
                    WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
                } else {
                    set_code(0x408b + (dest_adr_vals.part << 8) + (op_size << 4));
                }
                break;
            case ModIndReg:
                if (op_size == eSymbolSize32Bit) {
                    set_wasmcode_guessed(
                            0, 1,
                            (dest_adr_vals.part_guess_mask & 15)
                                    | ((dest_adr_vals.part_guess_mask & 0xf0) << 4));
                    set_code(
                            0x1000 + (src_adr_vals.part << 4) + (dest_adr_vals.part & 15)
                            + ((dest_adr_vals.part & 0xf0) << 4));
                } else if (src_adr_vals.part != 0) {
                    WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
                } else {
                    set_wasmcode_guessed(0, 1, dest_adr_vals.part_guess_mask);
                    set_code(0x8000 + dest_adr_vals.part + (((Word)op_size) << 8));
                }
                break;
            case ModIndReg12:
                set_wasmcode_guessed(1, 1, dest_adr_vals.part_guess_mask);
                set_double_code(
                        0x3001 + (src_adr_vals.part << 4)
                                + ((dest_adr_vals.part >> 4) & 0x0f00),
                        (op_size << 12) + (dest_adr_vals.part & 0xfff));
                break;
            case ModR0Base:
                set_code(
                        0x0004 + (dest_adr_vals.part << 8) + (src_adr_vals.part << 4)
                        + op_size);
                break;
            case ModGBRBase:
                if (src_adr_vals.part != 0) {
                    WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
                } else {
                    set_wasmcode_guessed(0, 1, dest_adr_vals.part_guess_mask);
                    set_code(0xc000 + dest_adr_vals.part + (((Word)op_size) << 8));
                }
                break;
            default:
                break;
            }
            break;
        }
        case ModIReg: {
            Word dest_reg;

            if (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                == eIsReg) {
                set_code(0x6000 + (src_adr_vals.part << 4) + (dest_reg << 8) + op_size);
            }
            break;
        }
        case ModPreDec: {
            Word dest_reg;

            if (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                != eIsReg) {
            } else if (dest_reg != 0) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[2]);
            } else {
                set_code(0x40cb + (src_adr_vals.part << 8) + (op_size << 4));
            }
            break;
        }
        case ModPostInc: {
            Word dest_reg;

            if (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                == eIsReg) {
                set_code(0x6004 + (src_adr_vals.part << 4) + (dest_reg << 8) + op_size);
            }
            break;
        }
        case ModIndReg: {
            Word dest_reg;

            if (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                != eIsReg) {
            } else if (op_size == eSymbolSize32Bit) {
                set_wasmcode_guessed(0, 1, src_adr_vals.part_guess_mask);
                set_code(0x5000 + (dest_reg << 8) + src_adr_vals.part);
            } else if (dest_reg != 0) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[2]);
            } else {
                set_wasmcode_guessed(0, 1, src_adr_vals.part_guess_mask);
                set_code(0x8400 + src_adr_vals.part + (((Word)op_size) << 8));
            }
            break;
        }
        case ModIndReg12: {
            Word dest_reg;

            if (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                == eIsReg) {
                set_wasmcode_guessed(1, 1, src_adr_vals.part_guess_mask);
                set_double_code(
                        0x3001 + (dest_reg << 8) + ((src_adr_vals.part >> 8) & 0x00f0),
                        0x4000 + (op_size << 12) + (src_adr_vals.part & 0xfff));
            }
            break;
        }
        case ModR0Base: {
            Word dest_reg;

            if (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                == eIsReg) {
                set_code(0x000c + (src_adr_vals.part << 4) + (dest_reg << 8) + op_size);
            }
            break;
        }
        case ModGBRBase: {
            Word dest_reg;

            if (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                != eIsReg) {
            } else if (dest_reg != 0) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
            } else {
                set_wasmcode_guessed(0, 1, src_adr_vals.part_guess_mask);
                set_code(0xc400 + src_adr_vals.part + (((Word)op_size) << 8));
            }
            break;
        }
        case ModPCRel: {
            Word dest_reg;

            if (op_size == eSymbolSize8Bit) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
            } else if (
                    decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                    == eIsReg) {
                set_wasmcode_guessed(0, 1, src_adr_vals.part_guess_mask);
                set_code(
                        0x9000 + (((Word)op_size - 1) << 14) + (dest_reg << 8)
                        + src_adr_vals.part);
            }
            break;
        }
        case ModImm: {
            Word dest_reg;
            if (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True)
                == eIsReg) {
                set_wasmcode_guessed(0, 1, src_adr_vals.part_guess_mask);
                set_code(0xe000 + (dest_reg << 8) + src_adr_vals.part);
            }
            break;
        }
        default:
            break;
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_mova(Word code)
 * \brief  handle MOVA instruction
 * ------------------------------------------------------------------------ */

static void decode_mova(Word code) {
    Word HReg;

    UNUSED(code);

    if (!ChkArgCnt(2, 2))
        ;
    else if (!decode_cpu_reg(&ArgStr[2], &HReg, NULL, eSymbolSize32Bit, True))
        ;
    else if (HReg != 0) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[2]);
    } else {
        adr_vals_t adr_vals;

        set_opsize(eSymbolSize32Bit, &ArgStr[2]);
        if (decode_adr(&ArgStr[1], &adr_vals, MModPCRel, False) != ModNone) {
            set_wasmcode_guessed(0, 1, adr_vals.part_guess_mask);
            set_code(0xc700 + adr_vals.part);
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_pref_ocb(Word code)
 * \brief  Handle OCBxx/PREF(I)/ICBI instructions
 * \param  code machine code/minimum CPU
 * ------------------------------------------------------------------------ */

static void decode_pref_ocb(Word code) {
    adr_vals_t adr_vals;

    if (ChkArgCnt(1, 1) && chk_min_core((core_t)((code >> 8) & 15)) && chk_no_opsize()
        && (decode_adr(&ArgStr[1], &adr_vals, MModIReg, False) == ModIReg)) {
        set_code(WAsmCode[0] = (code & 0xf0ff) | (adr_vals.part << 8));
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_ldc_stc(Word is_ldc)
 * \brief  Handle LDC/STC instructions
 * \param  is_ldc instruction type
 * ------------------------------------------------------------------------ */

static void decode_ldc_stc(Word is_ldc) {
    if (op_size == eSymbolSizeUnknown) {
        set_opsize(eSymbolSize32Bit, NULL);
    }

    if (ChkArgCnt(2, 2)) {
        tStrComp *pArg1 = is_ldc ? &ArgStr[2] : &ArgStr[1],
                 *pArg2 = is_ldc ? &ArgStr[1] : &ArgStr[2];
        Word ctrl_reg;

        if (decode_ctrl_reg(pArg1, &ctrl_reg)) {
            adr_vals_t adr_vals;

            if (is_ldc && (ctrl_reg == CTRL_REG_SGR)
                && (p_curr_cpu_props->core < e_core_sh4a)) {
                WrStrErrorPos(ErrNum_InvCtrlReg, pArg1);
                return;
            } else if (
                    (ctrl_reg != CTRL_REG_GBR) && (ctrl_reg != CTRL_REG_TBR)
                    && !SupAllowed) {
                WrStrErrorPos(ErrNum_PrivOrder, &OpPart);
            } else {
                switch (decode_adr(
                        pArg2, &adr_vals, MModReg | (is_ldc ? MModPostInc : MModPreDec),
                        False)) {
                case ModReg:
                    set_code(
                            (is_ldc ? (0x400e - ((ctrl_reg & 16) >> 2))
                                    : (0x0002 + ((ctrl_reg & 16) >> 1)))
                            | (adr_vals.part << 8) | ((ctrl_reg & 15) << 4));
                    break;
                case ModPostInc:
                    set_code(
                            (0x4007 - ((ctrl_reg & 16) >> 4)) | (adr_vals.part << 8)
                            | ((ctrl_reg & 15) << 4));
                    break;
                case ModPreDec:
                    set_code(
                            (0x4003 - ((ctrl_reg & 16) >> 4)) | (adr_vals.part << 8)
                            | ((ctrl_reg & 15) << 4));
                    break;
                default:
                    break;
                }
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_movca(Word code)
 * \brief  handle MOVCA instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_movca(Word code) {
    if (ChkArgCnt(2, 2) && set_opsize(eSymbolSize32Bit, &AttrPart)) {
        adr_vals_t adr_vals;

        if (decode_adr(&ArgStr[1], &adr_vals, MModReg, False) == ModReg) {
            if (adr_vals.part != 0) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
            } else {
                if (decode_adr(&ArgStr[2], &adr_vals, MModIReg, False) == ModIReg) {
                    set_code(code | (adr_vals.part << 8));
                }
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_movco(Word code)
 * \brief  handle MOVCO instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_movco(Word code) {
    adr_vals_t adr_vals;

    if (!ChkArgCnt(2, 2) || !chk_min_core(e_core_sh4a)
        || !set_opsize(eSymbolSize32Bit, &AttrPart)
        || (decode_adr(&ArgStr[1], &adr_vals, MModReg, False) != ModReg)) {
        return;
    }
    if (adr_vals.part != 0) {
        WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
        return;
    }
    if (decode_adr(&ArgStr[2], &adr_vals, MModIReg, False) == ModIReg) {
        set_code(code | (adr_vals.part << 8));
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_movli(Word code)
 * \brief  handle MOVLI instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_movli(Word code) {
    adr_vals_t adr_vals;

    if (!ChkArgCnt(2, 2) || !chk_min_core(e_core_sh4a)
        || !set_opsize(eSymbolSize32Bit, &AttrPart)
        || (decode_adr(&ArgStr[2], &adr_vals, MModReg, False) != ModReg)) {
        return;
    }
    if (adr_vals.part != 0) {
        WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[2]);
        return;
    }
    if (decode_adr(&ArgStr[1], &adr_vals, MModIReg, False) == ModIReg) {
        set_code(code | (adr_vals.part << 8));
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_movua(Word code)
 * \brief  handle MOVUA instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_movua(Word code) {
    adr_vals_t adr_vals;

    if (!ChkArgCnt(2, 2) || !chk_min_core(e_core_sh4a)
        || !set_opsize(eSymbolSize32Bit, &AttrPart)
        || (decode_adr(&ArgStr[2], &adr_vals, MModReg, False) != ModReg)) {
        return;
    }
    if (adr_vals.part != 0) {
        WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[2]);
        return;
    }
    switch (decode_adr(&ArgStr[1], &adr_vals, MModIReg | MModPostInc, False)) {
    case ModIReg:
        set_code(code | (adr_vals.part << 8));
        break;
    case ModPostInc:
        set_code(code | (1 << 6) | (adr_vals.part << 8));
        break;
    default:
        break;
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_movi(Word code)
 * \brief  handle MOVI20(S) instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_movi20(Word code) {
    adr_vals_t adr_vals;

    if (ChkArgCnt(2, 2) && chk_core_mask(e_core_mask_sh2a)
        && set_opsize(eSymbolSize32Bit, &AttrPart)
        && (decode_adr(&ArgStr[2], &adr_vals, MModReg, False) == ModReg)) {
        tEvalResult eval_result;
        LongInt     value = EvalStrIntExpressionOffsWithResult(
                &ArgStr[1], !!(*ArgStr[1].str.p_str == '#'), SInt20, &eval_result);
        if (eval_result.OK) {
            set_w_guessed(eval_result.Flags, 0, 1, 0x00f0);
            set_w_guessed(eval_result.Flags, 1, 1, 0xffff);
            set_double_code(
                    code | (adr_vals.part << 8) | ((value >> 12) & 0xf0),
                    value & 0xffffu);
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_movm(Word code)
 * \brief  handle MOVML/MOVMU instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_movm(Word code) {
    adr_vals_t adr_vals;

    if (ChkArgCnt(2, 2) && chk_core_mask(e_core_mask_sh2a)
        && set_opsize(eSymbolSize32Bit, &AttrPart)) {
        switch (decode_adr(&ArgStr[1], &adr_vals, MModReg | MModPostInc, False)) {
        case ModReg:
            code |= adr_vals.part << 8;
            if (decode_adr(&ArgStr[2], &adr_vals, MModPreDec, False) == ModPreDec) {
                if (adr_vals.part != REG_SP) {
                    WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[2]);
                } else {
                    set_code(code);
                }
            }
            break;
        case ModPostInc:
            if (adr_vals.part != REG_SP) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
            } else if (decode_adr(&ArgStr[2], &adr_vals, MModReg, False) == ModReg) {
                set_code(code | (adr_vals.part << 8) | 0x0004);
            }
            break;
        default:
            break;
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_movu(Word code)
 * \brief  handle MOVU instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_movu(Word code) {
    Word       dest_reg;
    adr_vals_t adr_vals;

    if (ChkArgCnt(2, 2) && chk_core_mask(e_core_mask_sh2a)
        && (decode_cpu_reg(&ArgStr[2], &dest_reg, NULL, eSymbolSize32Bit, True) == eIsReg)
        && (decode_adr(&ArgStr[1], &adr_vals, MModIndReg12, False) == ModIndReg12)) {
        switch (op_size) {
        case eSymbolSizeUnknown:
            WrError(ErrNum_UndefOpSizes);
            break;
        case eSymbolSize8Bit:
        case eSymbolSize16Bit:
            set_wasmcode_guessed(1, 1, adr_vals.part_guess_mask & 0xfff);
            set_double_code(
                    code | (dest_reg << 8) | ((adr_vals.part >> 8) & 0x00f0),
                    0x8000 | (((Word)op_size) << 12) | (adr_vals.part & 0xfff));
            break;
        default:
            WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_lds_sts(Word IsLDS)
 * \brief  handle LDS/STS instructions
 * \param  IsLDS type of instruction
 * ------------------------------------------------------------------------ */

static void decode_lds_sts(Word IsLDS) {
    if (op_size == eSymbolSizeUnknown) {
        set_opsize(eSymbolSize32Bit, NULL);
    }

    if (ChkArgCnt(2, 2)) {
        tStrComp *pArg1 = IsLDS ? &ArgStr[2] : &ArgStr[1],
                 *pArg2 = IsLDS ? &ArgStr[1] : &ArgStr[2];
        Word HReg;

        if (!decode_s_reg(pArg1->str.p_str, &HReg)) {
            WrStrErrorPos(ErrNum_InvCtrlReg, pArg1);
        } else {
            adr_vals_t adr_vals;

            switch (decode_adr(
                    pArg2, &adr_vals, MModReg | (IsLDS ? MModPostInc : MModPreDec),
                    False)) {
            case ModReg:
                set_code((IsLDS << 14) + 0x000a + (adr_vals.part << 8) + (HReg << 4));
                break;
            case ModPostInc:
                set_code(0x4006 + (adr_vals.part << 8) + (HReg << 4));
                break;
            case ModPreDec:
                set_code(0x4002 + (adr_vals.part << 8) + (HReg << 4));
                break;
            default:
                break;
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_one_reg(Word index)
 * \brief  handle instructions with one register argument
 * \param  index index to instruction table
 * ------------------------------------------------------------------------ */

static void decode_one_reg(Word index) {
    OneRegOrder const* p_order = one_reg_orders + index;

    if (!ChkArgCnt(1, 1))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else if (chk_core_mask(p_order->core_mask)) {
        adr_vals_t adr_vals;

        if (decode_adr(&ArgStr[1], &adr_vals, MModReg, False) != ModNone) {
            set_code(p_order->code + (adr_vals.part << 8));
        }
        if (!SupAllowed && p_order->privileged) {
            WrStrErrorPos(ErrNum_PrivOrder, &OpPart);
        }
        if (p_order->Delayed) {
            curr_delayed = True;
            delayed_addr = 0x7fffffff;
            chk_delayed();
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_tas(Word code)
 * \brief  handle TAS instruction
 * ------------------------------------------------------------------------ */

static void decode_tas(Word code) {
    UNUSED(code);

    if (op_size == eSymbolSizeUnknown) {
        set_opsize(eSymbolSize8Bit, NULL);
    }
    if (!ChkArgCnt(1, 1))
        ;
    else if (op_size != eSymbolSize8Bit) {
        WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
    } else {
        adr_vals_t adr_vals;

        if (decode_adr(&ArgStr[1], &adr_vals, MModIReg, False) != ModNone) {
            set_code(0x401b + (adr_vals.part << 8));
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_two_reg(Word index)
 * \brief  handle instructions with two register arguments
 * \param  index index to instruction table
 * ------------------------------------------------------------------------ */

static void decode_two_reg(Word index) {
    TwoRegOrder const* p_order = two_reg_orders + index;

    if (!ChkArgCnt(2, 2))
        ;
    else if (*AttrPart.str.p_str && (op_size != p_order->DefSize)) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else if (chk_min_core(p_order->min_core)) {
        adr_vals_t adr_vals;

        if (decode_adr(&ArgStr[1], &adr_vals, MModReg, False) != ModNone) {
            WAsmCode[0] = p_order->code + (adr_vals.part << 4);
            if (decode_adr(&ArgStr[2], &adr_vals, MModReg, False) != ModNone) {
                set_code(WAsmCode[0] + (adr_vals.part << 8));
            }
            if (!SupAllowed && p_order->privileged) {
                WrStrErrorPos(ErrNum_PrivOrder, &OpPart);
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_mul_reg(Word index)
 * \brief  handle register-to-register multiply instructions
 * \param  index index to instruction table
 * ------------------------------------------------------------------------ */

static void decode_mul_reg(Word index) {
    FixedMinOrder const* p_order = mul_reg_orders + index;

    if (ChkArgCnt(2, 2) && chk_min_core(p_order->min_core)) {
        if (!*AttrPart.str.p_str) {
            op_size = eSymbolSize32Bit;
        }
        if (op_size != eSymbolSize32Bit) {
            WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
        } else {
            adr_vals_t adr_vals;

            if (decode_adr(&ArgStr[1], &adr_vals, MModReg, False) != ModNone) {
                WAsmCode[0] = p_order->code + (adr_vals.part << 4);
                if (decode_adr(&ArgStr[2], &adr_vals, MModReg, False) != ModNone) {
                    set_code(WAsmCode[0] + (adr_vals.part << 8));
                }
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_bw(Word index)
 * \brief  handle 8/16 bit instructions with one register argument
 * \param  index index to instruction table
 * ------------------------------------------------------------------------ */

static void decode_bw(Word index) {
    FixedOrder const* p_order = bw_orders + index;

    if (op_size == eSymbolSizeUnknown) {
        set_opsize(eSymbolSize16Bit, NULL);
    }
    if (!ChkArgCnt(2, 2))
        ;
    else if ((op_size != eSymbolSize8Bit) && (op_size != eSymbolSize16Bit)) {
        WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
    } else {
        adr_vals_t adr_vals;

        if (decode_adr(&ArgStr[1], &adr_vals, MModReg, False) != ModNone) {
            WAsmCode[0] = p_order->code + op_size + (adr_vals.part << 4);
            if (decode_adr(&ArgStr[2], &adr_vals, MModReg, False) != ModNone) {
                set_code(WAsmCode[0] + (adr_vals.part << 8));
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_mac(Word code)
 * \brief  handle MAC instruction
 * ------------------------------------------------------------------------ */

static void decode_mac(Word code) {
    UNUSED(code);

    if (op_size == eSymbolSizeUnknown) {
        set_opsize(eSymbolSize16Bit, NULL);
    }
    if (!ChkArgCnt(2, 2))
        ;
    else if ((op_size != eSymbolSize16Bit) && (op_size != eSymbolSize32Bit)) {
        WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
    } else if ((op_size == eSymbolSize32Bit) && !chk_min_core(e_core_sh2))
        ;
    else {
        adr_vals_t adr_vals;

        if (decode_adr(&ArgStr[1], &adr_vals, MModPostInc, False) != ModNone) {
            WAsmCode[0] = 0x000f + (adr_vals.part << 4) + (((Word)2 - op_size) << 14);
            if (decode_adr(&ArgStr[2], &adr_vals, MModPostInc, False) != ModNone) {
                set_code(WAsmCode[0] + (adr_vals.part << 8));
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_add(Word code)
 * \brief  handle ADD instruction
 * ------------------------------------------------------------------------ */

static void decode_add(Word code) {
    UNUSED(code);

    if (!ChkArgCnt(2, 2))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else {
        adr_vals_t dest_adr_vals;

        if (decode_adr(&ArgStr[2], &dest_adr_vals, MModReg, False) != ModNone) {
            adr_vals_t src_adr_vals;

            op_size = eSymbolSize32Bit;
            switch (decode_adr(&ArgStr[1], &src_adr_vals, MModReg | MModImm, True)) {
            case ModReg:
                set_code(0x300c + (dest_adr_vals.part << 8) + (src_adr_vals.part << 4));
                break;
            case ModImm:
                set_wasmcode_guessed(0, 1, src_adr_vals.part_guess_mask);
                set_code(0x7000 + src_adr_vals.part + (dest_adr_vals.part << 8));
                break;
            default:
                break;
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_cmp_eq(Word code)
 * \brief  handle CMP/EQ instruction
 * ------------------------------------------------------------------------ */

static void decode_cmp_eq(Word code) {
    UNUSED(code);

    if (!ChkArgCnt(2, 2))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else {
        adr_vals_t dest_adr_vals;

        if (decode_adr(&ArgStr[2], &dest_adr_vals, MModReg, False) != ModNone) {
            adr_vals_t src_adr_vals;

            op_size = eSymbolSize32Bit;
            switch (decode_adr(&ArgStr[1], &src_adr_vals, MModReg | MModImm, True)) {
            case ModReg:
                set_code(0x3000 + (dest_adr_vals.part << 8) + (src_adr_vals.part << 4));
                break;
            case ModImm:
                if (dest_adr_vals.part != 0) {
                    WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
                } else {
                    set_wasmcode_guessed(0, 1, src_adr_vals.part_guess_mask);
                    set_code(0x8800 + src_adr_vals.part);
                }
                break;
            default:
                break;
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     DecodeLog(Word code)
 * \brief  handle AND/OR/XOR instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void DecodeLog(Word code) {
    if (ChkArgCnt(2, 2)) {
        adr_vals_t dest_adr_vals;

        switch (decode_adr(&ArgStr[2], &dest_adr_vals, MModReg | MModGBRR0, False)) {
        case ModReg:
            if (*AttrPart.str.p_str && (op_size != eSymbolSize32Bit)) {
                WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
            } else {
                adr_vals_t src_adr_vals;

                op_size = eSymbolSize32Bit;
                switch (decode_adr(&ArgStr[1], &src_adr_vals, MModReg | MModImm, False)) {
                case ModReg:
                    set_code(
                            0x2008 + code + (dest_adr_vals.part << 8)
                            + (src_adr_vals.part << 4));
                    break;
                case ModImm:
                    if (dest_adr_vals.part != 0) {
                        WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
                    } else {
                        set_code(0xc800 + (code << 8) + src_adr_vals.part);
                    }
                    break;
                default:
                    break;
                }
            }
            break;
        case ModGBRR0: {
            adr_vals_t src_adr_vals;
            if (decode_adr(&ArgStr[1], &src_adr_vals, MModImm, False) != ModNone) {
                set_wasmcode_guessed(0, 1, src_adr_vals.part_guess_mask);
                set_code(0xcc00 + (code << 8) + src_adr_vals.part);
            }
            break;
        }
        default:
            break;
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_trapa(Word code)
 * \brief  handle TRAPA instruction
 * ------------------------------------------------------------------------ */

static void decode_trapa(Word code) {
    UNUSED(code);

    if (!ChkArgCnt(1, 1))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else {
        adr_vals_t adr_vals;

        op_size = eSymbolSize8Bit;
        if (decode_adr(&ArgStr[1], &adr_vals, MModImm, False) == ModImm) {
            set_code(0xc300 + adr_vals.part);
        }
        chk_delayed();
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_bt_bf(Word code)
 * \brief  handle BT/BF instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_bt_bf(Word code) {
    if (!ChkArgCnt(1, 1))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else if ((code & 0x400) && !chk_min_core(e_core_sh2))
        ;
    else {
        Boolean      OK;
        tSymbolFlags Flags;
        LongInt      AdrLong;

        delayed_addr = EvalStrIntExpressionWithFlags(&ArgStr[1], Int32, &OK, &Flags);
        AdrLong      = delayed_addr - (EProgCounter() + 4);
        if (OK) {
            if (Odd(AdrLong)) {
                WrStrErrorPos(ErrNum_DistIsOdd, &ArgStr[1]);
            } else if (
                    ((AdrLong < -256) || (AdrLong > 254))
                    && !mSymbolQuestionable(Flags)) {
                WrStrErrorPos(ErrNum_JmpDistTooBig, &ArgStr[1]);
            } else {
                set_w_guessed(Flags, 0, 1, 0xff);
                set_code(code + ((AdrLong >> 1) & 0xff));
                if (code & 0x400) {
                    curr_delayed = True;
                }
                chk_delayed();
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_bra_bsr(Word code)
 * \brief  handle BRA/BSR instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_bra_bsr(Word code) {
    if (!ChkArgCnt(1, 1))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else {
        Boolean      OK;
        tSymbolFlags Flags;
        LongInt      AdrLong;

        delayed_addr = EvalStrIntExpressionWithFlags(&ArgStr[1], Int32, &OK, &Flags);
        AdrLong      = delayed_addr - (EProgCounter() + 4);
        if (OK) {
            if (Odd(AdrLong)) {
                WrStrErrorPos(ErrNum_DistIsOdd, &ArgStr[1]);
            } else if (
                    ((AdrLong < -4096) || (AdrLong > 4094))
                    && !mSymbolQuestionable(Flags)) {
                WrStrErrorPos(ErrNum_JmpDistTooBig, &ArgStr[1]);
            } else {
                set_w_guessed(Flags, 0, 1, 0xfff);
                set_code(code + ((AdrLong >> 1) & 0xfff));
                curr_delayed = True;
                chk_delayed();
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_jsr_jmp(Word code)
 * \brief  Handle JMP/JSR instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_jsr_jmp(Word code) {
    if (!ChkArgCnt(1, 1))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else {
        adr_vals_t adr_vals;

        if (decode_adr(&ArgStr[1], &adr_vals, MModIReg, False) != ModNone) {
            set_code(code + (adr_vals.part << 8));
            curr_delayed = True;
            delayed_addr = 0x7fffffff;
            chk_delayed();
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_jsr_n(Word code)
 * \brief  handle JSR/N instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_jsr_n(Word code) {
    if (!ChkArgCnt(1, 1))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else if (chk_core_mask(e_core_mask_sh2a)) {
        adr_vals_t adr_vals;

        op_size = eSymbolSize32Bit; /* for @@(disp8,tbr) */
        switch (decode_adr(&ArgStr[1], &adr_vals, MModIReg | MModTBRBase, False)) {
        case ModIReg:
            set_code(code | (adr_vals.part << 8));
            chk_delayed();
            break;
        case ModTBRBase:
            set_wasmcode_guessed(0, 1, adr_vals.part_guess_mask);
            set_code(0x8300 | adr_vals.part);
            chk_delayed();
            break;
        case ModNone:
            break;
        default:
            WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dct_dcf(word condition)
 * \brief  handle DSP condition prefix
 * \param  condition actual condition
 * ------------------------------------------------------------------------ */

static void make_code_sh_core(void);

static void decode_dct_dcf(Word condition) {
    char* pos;
    int   z;

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
        return;
    }

    if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
        return;
    }

    /* strip off DSP condition */

    if (!ChkArgCnt(1, ArgCntMax)) {
        return;
    }

    pos = FirstBlank(ArgStr[1].str.p_str);
    if (!pos) {
        StrCompCopy(&OpPart, &ArgStr[1]);
        for (z = 1; z < ArgCnt; z++) {
            StrCompCopy(&ArgStr[z], &ArgStr[z + 1]);
        }
        ArgCnt--;
        as_separate_attribute_from_oppart();
        DecodeAttrPart();
    } else {
        StrCompSplitLeft(&ArgStr[1], &OpPart, pos);
        KillPrefBlanksStrComp(&ArgStr[1]);
    }
    NLS_UpString(OpPart.str.p_str);

    dsp_condition = condition;
    make_code_sh_core();
}

/*!------------------------------------------------------------------------
 * \fn     decode_clip(Word code)
 * \brief  handle CLIPU/CLIPS instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_clip(Word code) {
    adr_vals_t adr_vals;

    if (ChkArgCnt(1, 1) && chk_core_mask(e_core_mask_sh2a)
        && (decode_adr(&ArgStr[1], &adr_vals, MModReg, False) == ModReg)) {
        switch (op_size) {
        case eSymbolSize8Bit:
        case eSymbolSize16Bit:
            set_code(code | (adr_vals.part << 8) | (op_size << 2));
            break;
        case eSymbolSizeUnknown:
            WrError(ErrNum_UndefOpSizes);
            break;
        default:
            WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_clip(Word code)
 * \brief  handle DIVS/DIVU/MULR (SH2A specific) instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_mul_div32(Word code) {
    adr_vals_t adr_vals;

    if (ChkArgCnt(2, 2) && chk_core_mask(e_core_mask_sh2a)
        && set_opsize(eSymbolSize32Bit, &AttrPart)
        && (decode_adr(&ArgStr[2], &adr_vals, MModReg, False) == ModReg)) {
        code |= adr_vals.part << 8;
        if (decode_adr(&ArgStr[1], &adr_vals, MModReg, False) != ModReg)
            ;
        else if (adr_vals.part != 0) {
            WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[1]);
        } else {
            set_code(code);
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_ldbank_stbank(Word code)
 * \brief  handle LDBANK/STBANK instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_ldbank_stbank(Word code) {
    if (ChkArgCnt(2, 2) && chk_core_mask(e_core_mask_sh2a)) {
        int        r0_index = ((code >> 2) & 1) + 1;
        adr_vals_t adr_vals;

        if (decode_adr(&ArgStr[r0_index], &adr_vals, MModReg, False) != ModReg)
            ;
        else if (adr_vals.part != 0) {
            WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[r0_index]);
        } else if (
                decode_adr(&ArgStr[3 - r0_index], &adr_vals, MModIReg, False)
                == ModIReg) {
            set_code(code | (adr_vals.part << 8));
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_sh2a_bit(Word code)
 * \brief  Handle SH2A-specific bit operation instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_sh2a_bit(Word code) {
    if (ChkArgCnt(2, 2) && chk_core_mask(e_core_mask_sh2a)
        && set_opsize(eSymbolSize8Bit, &AttrPart)) {
        tSymbolFlags bpos_flags;
        Boolean      bpos_ok;
        Word         bpos = EvalStrIntExpressionOffsWithFlags(
                             &ArgStr[1], !!(ArgStr[1].str.p_str[0] == '#'), UInt3, &bpos_ok,
                             &bpos_flags),
                     mask = MModIndReg12 | ((code & 0x0fff) ? MModReg : 0);
        adr_vals_t   adr_vals;

        if (!bpos_ok) {
            return;
        }
        switch (decode_adr(&ArgStr[2], &adr_vals, mask, False)) {
        case ModReg:
            set_w_guessed(bpos_flags, 0, 1, 0x0007);
            set_code(0x8000 | (code & 0x0fff) | (adr_vals.part << 4) | (bpos & 7));
            break;
        case ModIndReg12:
            set_w_guessed(bpos_flags, 0, 1, 0x0070);
            set_wasmcode_guessed(1, 1, adr_vals.part_guess_mask);
            set_double_code(
                    0x3009 | ((bpos & 7) << 4) | ((adr_vals.part >> 4) & 0x0f00),
                    (code & 0xf000u) | (adr_vals.part & 0x0fff));
            break;
        default:
            break;
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_one_f_reg(Word code)
 * \brief  handle instructions with one FRn register as argument
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static Word get_freg_mode_mask(Boolean allow64) {
    return MModFReg
           | (((get_act_cpu_flags() & e_core_flag_fpu64) && allow64) ? MModDReg : 0);
}

static Boolean chk_no_xd_reg(Word reg_num, tStrComp const* p_arg) {
    if (reg_num & 1) {
        WrStrErrorPos(ErrNum_InvReg, p_arg);
        return False;
    } else {
        return True;
    }
}

static void decode_one_f_reg(Word code) {
    if (ChkArgCnt(1, 1) && chk_fpu(e_core_flag_fpu32)) {
        adr_vals_t adr_vals;
        switch (decode_adr(
                &ArgStr[1], &adr_vals, get_freg_mode_mask(!!(code & 0x0100)), False)) {
        case ModDReg:
            if (!chk_no_xd_reg(adr_vals.part, &ArgStr[1])) {
                return;
            }
            /* FALL-THRU */
        case ModFReg:
            if (set_opsize(adr_vals.part_op_size, &AttrPart)) {
                set_code((code & 0xf0ffu) | (adr_vals.part << 8));
            }
            break;
        default:
            break;
        }
    }
}

static void decode_one_f_reg_sh4a(Word code) {
    if (chk_min_core(e_core_sh4a)) {
        decode_one_f_reg(code);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_two_f_reg(Word code)
 * \brief  handle instructions with two FRn registers as argument
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_two_f_reg(Word code) {
    if (ChkArgCnt(2, 2) && chk_fpu(e_core_flag_fpu32)) {
        adr_vals_t adr_vals;
        adr_mode_t src_adr_mode = decode_adr(
                &ArgStr[1], &adr_vals, get_freg_mode_mask(!!(code & 0x0100)), False);
        switch (src_adr_mode) {
        case ModDReg:
            if (!chk_no_xd_reg(adr_vals.part, &ArgStr[1])) {
                return;
            }
            /* FALL-THRU */
        case ModFReg:
            if (set_opsize(adr_vals.part_op_size, &AttrPart)) {
                code |= adr_vals.part << 4;
                switch (decode_adr(&ArgStr[2], &adr_vals, 1 << src_adr_mode, False)) {
                case ModDReg:
                    if (!chk_no_xd_reg(adr_vals.part, &ArgStr[2])) {
                        return;
                    }
                    /* FALL-THRU */
                case ModFReg:
                    set_code((code & 0xf0ffu) | (adr_vals.part << 8));
                    break;
                default:
                    break;
                }
            }
        default:
            break;
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_one_f_reg_fpul(Word code)
 * \brief  handle instructions with one FRn register and FPUL
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_one_f_reg_fpul(Word code) {
    if (ChkArgCnt(2, 2) && chk_fpu(e_core_flag_fpu32)) {
        int        f_reg_idx = (code >> 8) & 3;
        adr_vals_t adr_vals;

        switch (decode_adr(
                &ArgStr[f_reg_idx], &adr_vals, get_freg_mode_mask(!!(code & 0x0800)),
                False)) {
        case ModFReg:
        case ModDReg:
            if (!set_opsize(adr_vals.part_op_size, &AttrPart))
                ;
            else if (as_strcasecmp(ArgStr[3 - f_reg_idx].str.p_str, "FPUL")) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[3 - f_reg_idx]);
            } else {
                set_code((code & 0xf0ffu) | (adr_vals.part << 8));
            }
            break;
        default:
            break;
        }
    }
}

static void decode_one_f_reg_fpul_64(Word code) {
    if (ChkArgCnt(2, 2) && chk_fpu(e_core_flag_fpu64)) {
        int        f_reg_idx = (code >> 8) & 3;
        adr_vals_t adr_vals;

        switch (decode_adr(&ArgStr[f_reg_idx], &adr_vals, MModDReg, False)) {
        case ModDReg:
            if (!set_opsize(adr_vals.part_op_size, &AttrPart))
                ;
            else if (as_strcasecmp(ArgStr[3 - f_reg_idx].str.p_str, "FPUL")) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[3 - f_reg_idx]);
            } else {
                set_code((code & 0xf0ffu) | (adr_vals.part << 8));
            }
            break;
        default:
            break;
        }
    }
}

static void decode_one_f_reg_fpul_64_sh4a(Word code) {
    if (chk_min_core(e_core_sh4a)) {
        decode_one_f_reg_fpul_64(code);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_fmac(Word code)
 * \brief  handle FMAC instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_fmac(Word code) {
    if (ChkArgCnt(3, 3) && chk_fpu(e_core_flag_fpu32)
        && set_opsize(eSymbolSizeFloat32Bit, &AttrPart)) {
        adr_vals_t adr_vals;

        if (ModFReg == decode_adr(&ArgStr[1], &adr_vals, MModFReg, False)) {
            if (adr_vals.part != 0) {
                WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
            } else if (ModFReg == decode_adr(&ArgStr[2], &adr_vals, MModFReg, False)) {
                code |= adr_vals.part << 4;
                if (ModFReg == decode_adr(&ArgStr[3], &adr_vals, MModFReg, False)) {
                    set_code(code | (adr_vals.part << 8));
                }
            }
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_fmov(Word code)
 * \brief  handle FMOV instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_fmov(Word code) {
    if (ChkArgCnt(2, 2) && chk_fpu(e_core_flag_fpu32)) {
        Word sh2a_modes = (p_curr_cpu_props->core == e_core_sh2a) ? MModIndReg12 : 0;
        adr_mode_t src_adr_mode;
        adr_vals_t src_adr_vals;

        src_adr_mode = decode_adr(
                &ArgStr[1], &src_adr_vals,
                get_freg_mode_mask(True) | MModIReg | MModPostInc | MModR0Base
                        | sh2a_modes,
                False);
        switch (src_adr_mode) {
        case ModFReg:
        case ModDReg:
            if (set_opsize(src_adr_vals.part_op_size, &ArgStr[1])) {
                adr_vals_t dest_adr_vals;

                code |= (src_adr_vals.part << 4);
                switch (decode_adr(
                        &ArgStr[2], &dest_adr_vals,
                        (1 << src_adr_mode) | MModIReg | MModPreDec | MModR0Base
                                | sh2a_modes,
                        False)) {
                case ModFReg:
                case ModDReg:
                    if (set_opsize(dest_adr_vals.part_op_size, &ArgStr[2])) {
                        set_code(code | (dest_adr_vals.part << 8) | 0xc);
                    }
                    break;
                case ModIReg:
                    set_code(code | (dest_adr_vals.part << 8) | 0xa);
                    break;
                case ModPreDec:
                    set_code(code | (dest_adr_vals.part << 8) | 0xb);
                    break;
                case ModR0Base:
                    set_code(code | (dest_adr_vals.part << 8) | 0x7);
                    break;
                case ModIndReg12:
                    set_wasmcode_guessed(1, 1, dest_adr_vals.part_guess_mask);
                    set_double_code(
                            0x3001 | ((dest_adr_vals.part >> 4) & 0x0f00),
                            0x3000 | (dest_adr_vals.part & 0x0fff));
                default:
                    break;
                }
            }
            break;
        case ModIReg: {
            adr_vals_t dest_adr_vals;

            code |= (src_adr_vals.part << 4);
            switch (decode_adr(
                    &ArgStr[2], &dest_adr_vals, get_freg_mode_mask(True), False)) {
            case ModFReg:
            case ModDReg:
                if (set_opsize(dest_adr_vals.part_op_size, &ArgStr[2])) {
                    set_code(code | (dest_adr_vals.part << 8) | 0x8);
                }
                break;
            default:
                break;
            }
            break;
        }
        case ModPostInc: {
            adr_vals_t dest_adr_vals;

            code |= (src_adr_vals.part << 4);
            switch (decode_adr(
                    &ArgStr[2], &dest_adr_vals, get_freg_mode_mask(True), False)) {
            case ModFReg:
            case ModDReg:
                if (set_opsize(dest_adr_vals.part_op_size, &ArgStr[2])) {
                    set_code(code | (dest_adr_vals.part << 8) | 0x9);
                }
                break;
            default:
                break;
            }
            break;
        }
        case ModR0Base: {
            adr_vals_t dest_adr_vals;

            code |= (src_adr_vals.part << 4);
            switch (decode_adr(
                    &ArgStr[2], &dest_adr_vals, get_freg_mode_mask(True), False)) {
            case ModFReg:
            case ModDReg:
                if (set_opsize(dest_adr_vals.part_op_size, &ArgStr[2])) {
                    set_code(code | (dest_adr_vals.part << 8) | 0x6);
                }
                break;
            default:
                break;
            }
            break;
        }
        case ModIndReg12: {
            Word       code1 = 0x3001 | ((src_adr_vals.part >> 8) & 0x00f0),
                       code2 = 0x7000 | (src_adr_vals.part & 0x0fff);
            adr_vals_t dest_adr_vals;

            set_wasmcode_guessed(1, 1, src_adr_vals.part_guess_mask & 0x0fff);
            switch (decode_adr(
                    &ArgStr[2], &dest_adr_vals, get_freg_mode_mask(True), False)) {
            case ModFReg:
            case ModDReg:
                if (set_opsize(dest_adr_vals.part_op_size, &ArgStr[2])) {
                    set_double_code(code1 | dest_adr_vals.part << 8, code2);
                }
                break;
            default:
                break;
            }
            break;
        }
        default:
            break;
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_fipr(Word code)
 * \brief  handle FIPR instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_fipr(Word code) {
    adr_vals_t adr_vals;

    if (!ChkArgCnt(2, 2))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else if (
            chk_min_core(e_core_sh4) && chk_fpu(e_core_flag_fpu64)
            && (ModNone != decode_adr(&ArgStr[1], &adr_vals, MModFVReg, False))) {
        code |= adr_vals.part << 6;
        if (ModNone != decode_adr(&ArgStr[2], &adr_vals, MModFVReg, False)) {
            set_code(code | (adr_vals.part << 8));
        }
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_ftrv(Word code)
 * \brief  handle FTRV instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_ftrv(Word code) {
    adr_vals_t adr_vals;

    if (!ChkArgCnt(2, 2))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else if (as_strcasecmp(ArgStr[1].str.p_str, "XMTRX")) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
    } else if (
            chk_min_core(e_core_sh4) && chk_fpu(e_core_flag_fpu64)
            && (ModNone != decode_adr(&ArgStr[2], &adr_vals, MModFVReg, False))) {
        set_code(code | (adr_vals.part << 8));
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_alu_three(Word code)
 * \brief  handle DSP instructions with 3 operands and optional condition
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static Boolean code_is_padd_psub(Word code) {
    return (code & 0xef00) == 0xa100;
}

static void decode_dsp_alu_three(Word code) {
    Word Sx, Sy, Dz;

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
        return;
    }

    if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
        return;
    }

    if (!ChkArgCnt(3, 3))
        ;
    else if (!decode_dsp_reg_sx(&ArgStr[1], &Sx)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
    } else if (!decode_dsp_reg_sy(&ArgStr[2], &Sy)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[2]);
    } else if (!decode_dsp_reg_dz(&ArgStr[3], &Dz)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[3]);
    } else if (this_par && (dsp_par_mask & (1 << e_dsp_par_pmul))) {
        Word Du;

        /* only non-conditional PADD/PSUB can be paralleled with PMULS */
        if (!code_is_padd_psub(code) || dsp_condition) {
            WrError(ErrNum_InvParConstruct);
        } else if (!dsp_reg_ds_to_du(Dz, &Du)) {
            WrStrErrorPos(ErrNum_InvParAddrMode, &ArgStr[3]);
        } else {
            augment_dsp_par_instr(/* Se, Sf, and Dg have same bit positions and can be copied: */
                            0x6000 | (code & 0x1000) | (dsp_par_acc[1] & 0x0f0c)
                          | (Sx << 6) | (Sy << 4) | (Du << 0), e_dsp_par_alu, Dz);
        }
    } else {
        set_dsp_double_code(code | (Sx << 6) | (Sy << 4) | Dz, !!(code & 0x0100));
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_shift(Word code)
 * \brief  handle DSP shift instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_dsp_shift(Word code) {
    switch (ArgCnt) {
    case 2: {
        Word Dz;
        if (decode_dsp_reg_dz(&ArgStr[2], &Dz)) {
            tEvalResult eval_result;
            Word        shift = EvalStrIntExpressionOffsWithResult(
                    &ArgStr[1], !!(ArgStr[1].str.p_str[0] == '#'), SInt7, &eval_result);
            if (eval_result.OK) {
                set_w_guessed(eval_result.Flags, 1, 1, 0x7f << 4);
                set_dsp_double_code(code | ((shift << 4) & 0x07f0) | Dz, False);
            }
        }
        break;
    }
    case 3:
        decode_dsp_alu_three(0x9100 - code);
        break;
    default:
        (void)ChkArgCnt(2, 3);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_alu_two_xy(Word code)
 * \brief  handle DSP instructions with 2 operands and optional condition
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_dsp_alu_two_xy(Word code) {
    Word Sx, Sy, Dz;

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
        return;
    }

    if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
        return;
    }

    if (!ChkArgCnt(2, 2))
        ;
    else if (!decode_dsp_reg_dz(&ArgStr[2], &Dz)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[2]);
    } else if (decode_dsp_reg_sx(&ArgStr[1], &Sx)) {
        set_dsp_double_code(code | (Sx << 6) | Dz, !!(code & 0x0100));
    } else if (decode_dsp_reg_sy(&ArgStr[1], &Sy)) {
        set_dsp_double_code(code | (1 << 13) | (Sy << 4) | Dz, !!(code & 0x0100));
    } else {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_alu_one_dz(Word code)
 * \brief  handle DSP instructions with 1 operand
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_dsp_alu_one_dz(Word code) {
    Word Dz;

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
        return;
    }

    if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
        return;
    }

    if (!ChkArgCnt(1, 1))
        ;
    else if (!decode_dsp_reg_dz(&ArgStr[1], &Dz)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
    } else {
        set_dsp_double_code(code | Dz, !!(code & 0x0100));
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_cmp(Word code)
 * \brief  handle PCMP instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_dsp_cmp(Word code) {
    Word Sx, Sy;

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
        return;
    }

    if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
        return;
    }

    if (!ChkArgCnt(2, 2))
        ;
    else if (!decode_dsp_reg_sx(&ArgStr[1], &Sx)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
    } else if (!decode_dsp_reg_sy(&ArgStr[2], &Sy)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[2]);
    } else {
        set_dsp_double_code(code | (Sx << 6) | (Sy << 4), False);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_lds_sts(Word code)
 * \brief  handle PLDS/PSTS instructions
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_dsp_lds_sts(Word code) {
    Word Dz, reg;
    int  dz_index = !(code & 0x2000) + 1;

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
        return;
    }

    if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
        return;
    }

    if (!ChkArgCnt(2, 2))
        ;
    else if (!decode_dsp_reg_dz(&ArgStr[dz_index], &Dz)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[dz_index]);
    } else if (!decode_s_reg(ArgStr[3 - dz_index].str.p_str, &reg) || (reg > 1)) {
        WrStrErrorPos(ErrNum_InvCtrlReg, &ArgStr[3 - dz_index]);
    } else {
        set_dsp_double_code(code | Dz | (reg << 12), True);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_muls(Word code)
 * \brief  handle PMULS instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_dsp_muls(Word code) {
    Word Se, Sf, mul_Dz, Dg;

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
    } else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else if (!ChkArgCnt(3, 3))
        ;
    else if (dsp_condition) {
        WrError(ErrNum_InvParConstruct);
    } else if (!decode_dsp_reg_se(&ArgStr[1], &Se)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[1]);
    } else if (!decode_dsp_reg_sf(&ArgStr[2], &Sf)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[2]);
    } else if (!decode_dsp_reg_dg(&ArgStr[3], &mul_Dz, &Dg)) {
        WrStrErrorPos(ErrNum_InvReg, &ArgStr[3]);
    } else if (this_par && (dsp_par_mask & (1 << e_dsp_par_alu))) {
        /* pmuls parallel only to non-conditional padd/psub */
        if (!code_is_padd_psub(dsp_par_acc[1])) {
            WrError(ErrNum_InvParConstruct);
        } else {
            Word add_Dz = dsp_par_acc[1] & 0x000f, Du;
            if (!dsp_reg_ds_to_du(add_Dz, &Du)) {
                WrError(ErrNum_InvParAddrMode);
            } else {
                augment_dsp_par_instr(/* ADD/SUB bit, Sx, and Sy have same bit positions and can be copied: */
                              0x6000 | (dsp_par_acc[1] & 0x10f0)
                              | (Se << 10) | (Sf << 8) | (Dg << 2) | (Du << 0),
                              e_dsp_par_pmul, mul_Dz);
            }
        }
    } else {
        augment_dsp_par_instr(
                code | (Se << 10) | (Sf << 8) | (Dg << 2), e_dsp_par_pmul, mul_Dz);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_movs(Word code)
 * \brief  handle MOVS instruction
 * \param  code machine code
 * ------------------------------------------------------------------------ */

static void decode_dsp_movs(Word code) {
    Word       Ds;
    int        mem_idx;
    adr_vals_t adr_vals;

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
        return;
    }

    if (op_size == eSymbolSizeUnknown) {
        op_size = eSymbolSize32Bit;
    }
    switch (op_size) {
    case eSymbolSize16Bit:
        break;
    case eSymbolSize32Bit:
        code |= 2;
        break;
    default:
        WrStrErrorPos(ErrNum_InvOpSize, &AttrPart);
        return;
    }

    if (!ChkArgCnt(2, 2)) {
        return;
    }

    if (decode_dsp_reg_ds(&ArgStr[1], &Ds)) {
        code |= 1;
        mem_idx = 2;
    } else if (decode_dsp_reg_ds(&ArgStr[2], &Ds)) {
        mem_idx = 1;
    } else {
        WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[2]);
        return;
    }
    code |= (Ds << 4);

    switch (decode_adr(
            &ArgStr[mem_idx], &adr_vals,
            MModIReg | MModPreDec | MModPostInc | MModPostIncByReg, False)) {
    case ModIReg:
        code |= (1 << 2);
        break;
    case ModPreDec:
        code |= (0 << 2);
        break;
    case ModPostInc:
        code |= (2 << 2);
        break;
    case ModPostIncByReg:
        code |= (3 << 2);
        break;
    default:
        return;
    }
    switch (adr_vals.part) {
    case 4:
    case 5:
        set_code(code | ((adr_vals.part - 4) << 8));
        break;
    case 2:
    case 3:
        set_code(code | (adr_vals.part << 8));
        break;
    default:
        WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[mem_idx]);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_nopxy(Word arg)
 * \brief  handle NOPX/NOPY instructions
 * \param  arg x/y component
 * ------------------------------------------------------------------------ */

static void decode_dsp_nopxy(Word arg) {
    dsp_par_component_t dsp_par = (dsp_par_component_t)arg;
    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
        return;
    }

    if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else if (!ChkArgCnt(0, 0))
        ;
    else {
        augment_dsp_par_instr(0x0000, dsp_par, REG_DSP_NONE);
    }
}

/*!------------------------------------------------------------------------
 * \fn     decode_dsp_movxy(Word arg)
 * \brief  handle MOVX/MOVY instructions
 * \param  arg x/y component
 * ------------------------------------------------------------------------ */

static void decode_dsp_movxy(Word arg) {
    dsp_par_component_t dsp_par = (dsp_par_component_t)arg;
    Word                Dz, D, code = 0;
    int                 mem_arg_idx, field_shift = !!(dsp_par == e_dsp_par_movx);

    if (!dsp_avail) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
    } else if (!ChkArgCnt(2, 2))
        ;
    else if (!set_opsize(eSymbolSize16Bit, &AttrPart))
        ;
    else {
        adr_vals_t adr_vals;

        if (decode_dsp_reg_dxy(&ArgStr[2], dsp_par, &Dz, &D)) {
            mem_arg_idx = 1;
            code |= ((0 << 4) | (D << 6)) << field_shift;
        } else if (decode_dsp_reg_da(&ArgStr[1], &D)) {
            mem_arg_idx = 2;
            code |= ((1 << 4) | (D << 6)) << field_shift;
            Dz = 0;
        } else {
            WrError(ErrNum_InvAddrMode);
            return;
        }
        switch (decode_adr(
                &ArgStr[mem_arg_idx], &adr_vals,
                MModIReg | MModPostInc | MModPostIncByReg, False)) {
        case ModIReg:
            code |= (1 << (2 * field_shift));
            goto chk_mem_reg;
        case ModPostInc:
            code |= (2 << (2 * field_shift));
            goto chk_mem_reg;
        case ModPostIncByReg:
            code |= (3 << (2 * field_shift));
            goto chk_mem_reg;
        default:
            break;
        chk_mem_reg:
            /*  R4/R5 for X, R6/R7 for Y */
            if ((adr_vals.part & ~1) != 6 - (field_shift * 2)) {
                WrStrErrorPos(ErrNum_InvAddrMode, &ArgStr[mem_arg_idx]);
                return;
            }
            code |= (adr_vals.part & 1) << (8 + field_shift);
        }
        augment_dsp_par_instr(code, dsp_par, Dz);
    }
}

/*!------------------------------------------------------------------------
 * \fn     is_dsp_par_inst(const char *p_inst)
 * \brief  is instruction a DSP instruction allowing parallel execution?
 * \param  p_inst instruction in source
 * \return True if so
 * ------------------------------------------------------------------------ */

static Boolean is_dsp_par_inst(char const* p_inst) {
    TInstTableEntry const* p_entry = inst_table_search(InstTable, p_inst);
    return (p_entry
            && ((p_entry->Procs[0] == decode_dct_dcf)
                || (p_entry->Procs[0] == decode_dsp_alu_three)
                || (p_entry->Procs[0] == decode_dsp_shift)
                || (p_entry->Procs[0] == decode_dsp_alu_two_xy)
                || (p_entry->Procs[0] == decode_dsp_alu_one_dz)
                || (p_entry->Procs[0] == decode_dsp_cmp)
                || (p_entry->Procs[0] == decode_dsp_lds_sts)
                || (p_entry->Procs[0] == decode_dsp_muls)
                || (p_entry->Procs[0] == decode_dsp_nopxy)
                || (p_entry->Procs[0] == decode_dsp_movxy)));
}

/*!------------------------------------------------------------------------
 * \fn     decode_ltorg(Word code)
 * \brief  handle LTORG instruction
 * ------------------------------------------------------------------------ */

static LargeInt ltorg_16(as_literal_t const* p_lit, tStrComp* p_name) {
    LargeInt ret;

    SetMaxCodeLen(CodeLen + 2);
    WAsmCode[CodeLen >> 1] = p_lit->value;
    ret                    = EProgCounter() + CodeLen;
    EnterIntSymbol(p_name, ret, ActPC, False);
    CodeLen += 2;
    return ret;
}

static LargeInt ltorg_32(as_literal_t const* p_lit, tStrComp* p_name) {
    LargeInt ret;

    SetMaxCodeLen(CodeLen + 6);
    if (((EProgCounter() + CodeLen) & 2) != 0) {
        WAsmCode[CodeLen >> 1] = 0;
        CodeLen += 2;
    }
    WAsmCode[CodeLen >> 1]       = (p_lit->value >> 16);
    WAsmCode[(CodeLen >> 1) + 1] = (p_lit->value & 0xffff);
    ret                          = EProgCounter() + CodeLen;
    EnterIntSymbol(p_name, ret, ActPC, False);
    CodeLen += 4;
    return ret;
}

static void decode_ltorg(Word code) {
    UNUSED(code);

    if (!ChkArgCnt(0, 0))
        ;
    else if (*AttrPart.str.p_str) {
        WrStrErrorPos(ErrNum_UseLessAttr, &AttrPart);
    } else {
        if ((EProgCounter() & 3) == 0) {
            literals_dump(ltorg_32, eSymbolSize32Bit, MomSectionHandle, True);
            literals_dump(ltorg_16, eSymbolSize16Bit, MomSectionHandle, False);
        } else {
            literals_dump(ltorg_16, eSymbolSize16Bit, MomSectionHandle, False);
            literals_dump(ltorg_32, eSymbolSize32Bit, MomSectionHandle, True);
        }
        reset_dsp_par_state();
    }
}

/*-------------------------------------------------------------------------*/
/* dynamische Belegung/Freigabe Codetabellen */

/*!------------------------------------------------------------------------
 * \fn     init_fields(void)
 * \brief  build up hash tables
 * ------------------------------------------------------------------------ */

static void AddFixed(
        char const* NName, Word NCode, Boolean NPriv, core_mask_t core_mask,
        core_flag_t core_flags) {
    order_array_rsv_end(fixed_orders, FixedOrder);
    fixed_orders[InstrZ].privileged = NPriv;
    fixed_orders[InstrZ].core_mask  = core_mask;
    fixed_orders[InstrZ].core_flags = core_flags;
    fixed_orders[InstrZ].code       = NCode;
    AddInstTable(InstTable, NName, InstrZ++, decode_fixed);
}

static void AddOneReg(
        char const* NName, Word NCode, core_mask_t core_mask, Boolean NPriv,
        Boolean NDel) {
    order_array_rsv_end(one_reg_orders, OneRegOrder);
    one_reg_orders[InstrZ].code       = NCode;
    one_reg_orders[InstrZ].core_mask  = core_mask;
    one_reg_orders[InstrZ].privileged = NPriv;
    one_reg_orders[InstrZ].Delayed    = NDel;
    AddInstTable(InstTable, NName, InstrZ++, decode_one_reg);
}

static void AddTwoReg(
        char const* NName, Word NCode, Boolean NPriv, core_t min_core, ShortInt NDef) {
    order_array_rsv_end(two_reg_orders, TwoRegOrder);
    two_reg_orders[InstrZ].privileged = NPriv;
    two_reg_orders[InstrZ].DefSize    = NDef;
    two_reg_orders[InstrZ].min_core   = min_core;
    two_reg_orders[InstrZ].code       = NCode;
    AddInstTable(InstTable, NName, InstrZ++, decode_two_reg);
}

static void AddMulReg(char const* NName, Word NCode, core_t min_core) {
    order_array_rsv_end(mul_reg_orders, FixedMinOrder);
    mul_reg_orders[InstrZ].code     = NCode;
    mul_reg_orders[InstrZ].min_core = min_core;
    AddInstTable(InstTable, NName, InstrZ++, decode_mul_reg);
}

static void AddBW(char const* NName, Word NCode) {
    order_array_rsv_end(bw_orders, FixedOrder);
    bw_orders[InstrZ].code = NCode;
    AddInstTable(InstTable, NName, InstrZ++, decode_bw);
}

static void AddSReg(char const* NName, Word NCode, core_t min_core, core_flag_t flag) {
    order_array_rsv_end(reg_defs, TRegDef);
    reg_defs[InstrZ].Name      = NName;
    reg_defs[InstrZ].code      = NCode;
    reg_defs[InstrZ].min_core  = min_core;
    reg_defs[InstrZ].core_flag = flag;
    InstrZ++;
}

static void init_fields(void) {
    InstTable = CreateInstTable(201);

    add_null_pseudo(InstTable);

    AddInstTable(InstTable, "MOV", 0, decode_mov);
    AddInstTable(InstTable, "MOVA", 0, decode_mova);
    AddInstTable(InstTable, "OCBI", 0x0093 | (e_core_sh4 << 8), decode_pref_ocb);
    AddInstTable(InstTable, "OCBP", 0x00a3 | (e_core_sh4 << 8), decode_pref_ocb);
    AddInstTable(InstTable, "OCBWB", 0x00b3 | (e_core_sh4 << 8), decode_pref_ocb);
    AddInstTable(InstTable, "ICBI", 0x00e3 | (e_core_sh4a << 8), decode_pref_ocb);
    AddInstTable(InstTable, "PREF", 0x0083 | (e_core_sh2 << 8), decode_pref_ocb);
    AddInstTable(InstTable, "PREFI", 0x00d3 | (e_core_sh4a << 8), decode_pref_ocb);
    AddInstTable(InstTable, "LDC", 1, decode_ldc_stc);
    AddInstTable(InstTable, "STC", 0, decode_ldc_stc);
    AddInstTable(InstTable, "LDS", 1, decode_lds_sts);
    AddInstTable(InstTable, "STS", 0, decode_lds_sts);
    AddInstTable(InstTable, "TAS", 0, decode_tas);
    AddInstTable(InstTable, "MAC", 0, decode_mac);
    AddInstTable(InstTable, "ADD", 0, decode_add);
    AddInstTable(InstTable, "CMP/EQ", 0, decode_cmp_eq);
    AddInstTable(InstTable, "TRAPA", 0, decode_trapa);
    AddInstTable(InstTable, "BF", 0x8b00, decode_bt_bf);
    AddInstTable(InstTable, "BT", 0x8900, decode_bt_bf);
    AddInstTable(InstTable, "BF/S", 0x8f00, decode_bt_bf);
    AddInstTable(InstTable, "BT/S", 0x8d00, decode_bt_bf);
    AddInstTable(InstTable, "BRA", 0xa000, decode_bra_bsr);
    AddInstTable(InstTable, "BSR", 0xb000, decode_bra_bsr);
    AddInstTable(InstTable, "JSR", 0x400b, decode_jsr_jmp);
    AddInstTable(InstTable, "JMP", 0x402b, decode_jsr_jmp);
    AddInstTable(InstTable, "JSR/N", 0x404b, decode_jsr_n);
    AddInstTable(InstTable, "MOVCA", 0x00c3, decode_movca);
    AddInstTable(InstTable, "MOVCO", 0x0073, decode_movco);
    AddInstTable(InstTable, "MOVLI", 0x0063, decode_movli);
    AddInstTable(InstTable, "MOVUA", 0x40a9, decode_movua);
    AddInstTable(InstTable, "MOVI20", 0x0000, decode_movi20);
    AddInstTable(InstTable, "MOVI20S", 0x0001, decode_movi20);
    AddInstTable(InstTable, "MOVML", 0x40f1, decode_movm);
    AddInstTable(InstTable, "MOVMU", 0x40f0, decode_movm);
    AddInstTable(InstTable, "MOVU", 0x3001, decode_movu);
    AddInstTable(InstTable, "CLIPS", 0x4091, decode_clip);
    AddInstTable(InstTable, "CLIPU", 0x4081, decode_clip);
    AddInstTable(InstTable, "DIVS", 0x4094, decode_mul_div32);
    AddInstTable(InstTable, "DIVU", 0x4084, decode_mul_div32);
    AddInstTable(InstTable, "MULR", 0x4080, decode_mul_div32);
    AddInstTable(InstTable, "LDBANK", 0x40e5, decode_ldbank_stbank);
    AddInstTable(InstTable, "STBANK", 0x40e1, decode_ldbank_stbank);
    AddInstTable(InstTable, "LTORG", 0, decode_ltorg);

    InstrZ = 0;
    AddFixed("CLRT", 0x0008, False, e_core_mask_all, e_core_flag_none);
    AddFixed("CLRMAC", 0x0028, False, e_core_mask_all, e_core_flag_none);
    AddFixed("NOP", 0x0009, False, e_core_mask_all, e_core_flag_none);
    AddFixed("RTE", 0x002b, False, e_core_mask_all, e_core_flag_none);
    AddFixed("SETT", 0x0018, False, e_core_mask_all, e_core_flag_none);
    AddFixed("NOTT", 0x00d0, False, e_core_mask_sh2a, e_core_flag_none);
    AddFixed("SLEEP", 0x001b, False, e_core_mask_all, e_core_flag_none);
    AddFixed("RTS", 0x000b, False, e_core_mask_all, e_core_flag_none);
    AddFixed("RTS/N", 0x006b, False, e_core_mask_sh2a, e_core_flag_none);
    AddFixed("DIV0U", 0x0019, False, e_core_mask_all, e_core_flag_none);
    AddFixed("BRK", 0x0000, True, e_core_mask_all, e_core_flag_none);
    AddFixed("RTB", 0x0001, True, e_core_mask_all, e_core_flag_none);
    AddFixed("CLRS", 0x0048, False, e_core_mask_sh3_higher, e_core_flag_none);
    AddFixed("SETS", 0x0058, False, e_core_mask_sh3_higher, e_core_flag_none);
    AddFixed("LDTLB", 0x0038, True, e_core_mask_sh3_higher, e_core_flag_none);
    AddFixed("FRCHG", 0xfbfd, False, e_core_mask_sh4_higher, e_core_flag_fpu64);
    AddFixed("FSCHG", 0xf3fd, False, e_core_mask_sh2a_higher, e_core_flag_fpu64);
    AddFixed("FPCHG", 0xf7fd, False, e_core_mask_sh4a_higher, e_core_flag_fpu64);
    AddFixed("SYNCO", 0x00ab, False, e_core_mask_sh4a_higher, e_core_flag_none);
    AddFixed("RESBANK", 0x005b, False, e_core_mask_sh2a, e_core_flag_none);

    InstrZ = 0;
    AddOneReg("MOVT", 0x0029, e_core_mask_all, False, False);
    AddOneReg("MOVRT", 0x0039, e_core_mask_sh2a, False, False);
    AddOneReg("CMP/PZ", 0x4011, e_core_mask_all, False, False);
    AddOneReg("CMP/PL", 0x4015, e_core_mask_all, False, False);
    AddOneReg("ROTL", 0x4004, e_core_mask_all, False, False);
    AddOneReg("ROTR", 0x4005, e_core_mask_all, False, False);
    AddOneReg("ROTCL", 0x4024, e_core_mask_all, False, False);
    AddOneReg("ROTCR", 0x4025, e_core_mask_all, False, False);
    AddOneReg("SHAL", 0x4020, e_core_mask_all, False, False);
    AddOneReg("SHAR", 0x4021, e_core_mask_all, False, False);
    AddOneReg("SHLL", 0x4000, e_core_mask_all, False, False);
    AddOneReg("SHLR", 0x4001, e_core_mask_all, False, False);
    AddOneReg("SHLL2", 0x4008, e_core_mask_all, False, False);
    AddOneReg("SHLR2", 0x4009, e_core_mask_all, False, False);
    AddOneReg("SHLL8", 0x4018, e_core_mask_all, False, False);
    AddOneReg("SHLR8", 0x4019, e_core_mask_all, False, False);
    AddOneReg("SHLL16", 0x4028, e_core_mask_all, False, False);
    AddOneReg("SHLR16", 0x4029, e_core_mask_all, False, False);
    AddOneReg("LDBR", 0x0021, e_core_mask_all, True, False);
    AddOneReg("STBR", 0x0020, e_core_mask_all, True, False);
    AddOneReg("DT", 0x4010, e_core_mask_sh2_higher, False, False);
    AddOneReg("BRAF", 0x0023, e_core_mask_sh2_higher, False, True);
    AddOneReg("BSRF", 0x0003, e_core_mask_sh2_higher, False, True);
    AddOneReg("RTV/N", 0x007b, e_core_mask_sh2a, False, False);

    InstrZ = 0;
    AddTwoReg("XTRCT", 0x200d, False, e_core_sh1, 2);
    AddTwoReg("ADDC", 0x300e, False, e_core_sh1, 2);
    AddTwoReg("ADDV", 0x300f, False, e_core_sh1, 2);
    AddTwoReg("CMP/HS", 0x3002, False, e_core_sh1, 2);
    AddTwoReg("CMP/GE", 0x3003, False, e_core_sh1, 2);
    AddTwoReg("CMP/HI", 0x3006, False, e_core_sh1, 2);
    AddTwoReg("CMP/GT", 0x3007, False, e_core_sh1, 2);
    AddTwoReg("CMP/STR", 0x200c, False, e_core_sh1, 2);
    AddTwoReg("DIV1", 0x3004, False, e_core_sh1, 2);
    AddTwoReg("DIV0S", 0x2007, False, e_core_sh1, -1);
    AddTwoReg("MULS", 0x200f, False, e_core_sh1, 1);
    AddTwoReg("MULU", 0x200e, False, e_core_sh1, 1);
    AddTwoReg("NEG", 0x600b, False, e_core_sh1, 2);
    AddTwoReg("NEGC", 0x600a, False, e_core_sh1, 2);
    AddTwoReg("SUB", 0x3008, False, e_core_sh1, 2);
    AddTwoReg("SUBC", 0x300a, False, e_core_sh1, 2);
    AddTwoReg("SUBV", 0x300b, False, e_core_sh1, 2);
    AddTwoReg("NOT", 0x6007, False, e_core_sh1, 2);
    AddTwoReg("SHAD", 0x400c, False, e_core_sh2a, 2);
    AddTwoReg("SHLD", 0x400d, False, e_core_sh2a, 2);

    InstrZ = 0;
    AddMulReg("MUL", 0x0007, e_core_sh2);
    AddMulReg("DMULU", 0x3005, e_core_sh2);
    AddMulReg("DMULS", 0x300d, e_core_sh2);

    InstrZ = 0;
    AddBW("SWAP", 0x6008);
    AddBW("EXTS", 0x600e);
    AddBW("EXTU", 0x600c);

    InstrZ = 0;
    AddInstTable(InstTable, "TST", InstrZ++, DecodeLog);
    AddInstTable(InstTable, "AND", InstrZ++, DecodeLog);
    AddInstTable(InstTable, "XOR", InstrZ++, DecodeLog);
    AddInstTable(InstTable, "OR", InstrZ++, DecodeLog);

    AddInstTable(InstTable, "BAND", 0x4000, decode_sh2a_bit);
    AddInstTable(InstTable, "BANDNOT", 0xc000, decode_sh2a_bit);
    AddInstTable(InstTable, "BCLR", 0x0600, decode_sh2a_bit);
    AddInstTable(InstTable, "BLD", 0x3708, decode_sh2a_bit);
    AddInstTable(InstTable, "BLDNOT", 0xb000, decode_sh2a_bit);
    AddInstTable(InstTable, "BOR", 0x5000, decode_sh2a_bit);
    AddInstTable(InstTable, "BORNOT", 0xd000, decode_sh2a_bit);
    AddInstTable(InstTable, "BSET", 0x1608, decode_sh2a_bit);
    AddInstTable(InstTable, "BST", 0x2700, decode_sh2a_bit);
    AddInstTable(InstTable, "BXOR", 0x6000, decode_sh2a_bit);

    AddInstTable(InstTable, "REG", 0, CodeREG);
    AddMoto16Pseudo(InstTable, e_moto_pseudo_flags_be);

    InstrZ = 0;
    AddSReg("MACH", 0, e_core_sh1, e_core_flag_none);
    AddSReg("MACL", 1, e_core_sh1, e_core_flag_none);
    AddSReg("PR", 2, e_core_sh1, e_core_flag_none);
    AddSReg("DSR", 6, e_core_sh1, e_core_flag_dsp);
    AddSReg("A0", 7, e_core_sh1, e_core_flag_dsp);
    AddSReg("X0", 8, e_core_sh1, e_core_flag_dsp);
    AddSReg("X1", 9, e_core_sh1, e_core_flag_dsp);
    AddSReg("Y0", 10, e_core_sh1, e_core_flag_dsp);
    AddSReg("Y1", 11, e_core_sh1, e_core_flag_dsp);
    AddSReg("FPUL", 5, e_core_sh1, e_core_flag_fpu32);
    AddSReg("FPSCR", 6, e_core_sh1, e_core_flag_fpu32);
    AddSReg(NULL, 0, e_core_sh1, e_core_flag_none);

    AddInstTable(InstTable, "FLDI0", 0xf08d, decode_one_f_reg);
    AddInstTable(InstTable, "FLDI1", 0xf09d, decode_one_f_reg);
    AddInstTable(InstTable, "FABS", 0xf15d, decode_one_f_reg);
    AddInstTable(InstTable, "FNEG", 0xf14d, decode_one_f_reg);
    AddInstTable(InstTable, "FSQRT", 0xf16d, decode_one_f_reg);
    AddInstTable(InstTable, "FSRRA", 0xf07d, decode_one_f_reg_sh4a);
    AddInstTable(InstTable, "FADD", 0xf100, decode_two_f_reg);
    AddInstTable(InstTable, "FCMP/EQ", 0xf104, decode_two_f_reg);
    AddInstTable(InstTable, "FCMP/GT", 0xf105, decode_two_f_reg);
    AddInstTable(InstTable, "FDIV", 0xf103, decode_two_f_reg);
    AddInstTable(InstTable, "FMUL", 0xf102, decode_two_f_reg);
    AddInstTable(InstTable, "FSUB", 0xf101, decode_two_f_reg);
    AddInstTable(InstTable, "FLDS", 0xf11d, decode_one_f_reg_fpul);
    AddInstTable(InstTable, "FSTS", 0xf20d, decode_one_f_reg_fpul);
    AddInstTable(InstTable, "FCNVDS", 0xf9bd, decode_one_f_reg_fpul_64);
    AddInstTable(InstTable, "FCNVSD", 0xfaad, decode_one_f_reg_fpul_64);
    AddInstTable(InstTable, "FSCA", 0xf2fd, decode_one_f_reg_fpul_64_sh4a);
    AddInstTable(InstTable, "FLOAT", 0xfa2d, decode_one_f_reg_fpul);
    AddInstTable(InstTable, "FTRC", 0xf93d, decode_one_f_reg_fpul);
    AddInstTable(InstTable, "FMAC", 0xf00e, decode_fmac);
    AddInstTable(InstTable, "FMOV", 0xf000, decode_fmov);
    AddInstTable(InstTable, "FIPR", 0xf0ed, decode_fipr);
    AddInstTable(InstTable, "FTRV", 0xf3fd, decode_ftrv);

    AddInstTable(InstTable, "DCT", 1, decode_dct_dcf);
    AddInstTable(InstTable, "DCF", 2, decode_dct_dcf);
    AddInstTable(InstTable, "PADD", 0xb100, decode_dsp_alu_three);
    AddInstTable(InstTable, "PADDC", 0xb000, decode_dsp_alu_three);
    AddInstTable(InstTable, "PSUB", 0xa100, decode_dsp_alu_three);
    AddInstTable(InstTable, "PSUBC", 0xa000, decode_dsp_alu_three);
    AddInstTable(InstTable, "PAND", 0x9500, decode_dsp_alu_three);
    AddInstTable(InstTable, "POR", 0xb500, decode_dsp_alu_three);
    AddInstTable(InstTable, "PXOR", 0xa500, decode_dsp_alu_three);
    AddInstTable(InstTable, "PSHA", 0x0000, decode_dsp_shift);
    AddInstTable(InstTable, "PSHL", 0x1000, decode_dsp_shift);
    AddInstTable(InstTable, "PABS", 0x8800, decode_dsp_alu_two_xy);
    AddInstTable(InstTable, "PCOPY", 0xd900, decode_dsp_alu_two_xy);
    AddInstTable(InstTable, "PNEG", 0xc900, decode_dsp_alu_two_xy);
    AddInstTable(InstTable, "PDEC", 0x8900, decode_dsp_alu_two_xy);
    AddInstTable(InstTable, "PINC", 0x9900, decode_dsp_alu_two_xy);
    AddInstTable(InstTable, "PDMSB", 0x9d00, decode_dsp_alu_two_xy);
    AddInstTable(InstTable, "PRND", 0x9800, decode_dsp_alu_two_xy);
    AddInstTable(InstTable, "PCLR", 0x8d00, decode_dsp_alu_one_dz);
    AddInstTable(InstTable, "PCMP", 0x8400, decode_dsp_cmp);
    AddInstTable(InstTable, "PLDS", 0xed00, decode_dsp_lds_sts);
    AddInstTable(InstTable, "PSTS", 0xcd00, decode_dsp_lds_sts);
    AddInstTable(InstTable, "PMULS", 0x4000, decode_dsp_muls);
    AddInstTable(InstTable, "MOVS", 0xf400, decode_dsp_movs);
    AddInstTable(InstTable, "NOPX", e_dsp_par_movx, decode_dsp_nopxy);
    AddInstTable(InstTable, "NOPY", e_dsp_par_movy, decode_dsp_nopxy);
    AddInstTable(InstTable, "MOVX", e_dsp_par_movx, decode_dsp_movxy);
    AddInstTable(InstTable, "MOVY", e_dsp_par_movy, decode_dsp_movxy);
}

/*!------------------------------------------------------------------------
 * \fn     de_init_fields(void)
 * \brief  tear down hash tables
 * ------------------------------------------------------------------------ */

static void de_init_fields(void) {
    DestroyInstTable(InstTable);
    order_array_free(fixed_orders);
    order_array_free(one_reg_orders);
    order_array_free(two_reg_orders);
    order_array_free(mul_reg_orders);
    order_array_free(bw_orders);
    order_array_free(reg_defs);
}

/*-------------------------------------------------------------------------*/

/*!------------------------------------------------------------------------
 * \fn     decode_attr_part_sh(void)
 * \brief  transform attribute to operand size
 * ------------------------------------------------------------------------ */

static Boolean decode_attr_part_sh(void) {
    if (*AttrPart.str.p_str) {
        if (strlen(AttrPart.str.p_str) != 1) {
            WrStrErrorPos(ErrNum_TooLongAttr, &AttrPart);
            return False;
        }
        if (!DecodeMoto16AttrSize(*AttrPart.str.p_str, &AttrPartOpSize[0], False)) {
            return False;
        }
    }
    return True;
}

/*!------------------------------------------------------------------------
 * \fn     make_code_sh(void)
 * \brief  actual code generation from source
 * ------------------------------------------------------------------------ */

static void make_code_sh_core(void) {
    /* ab hier (und weiter in der Hauptroutine) stehen die Befehle,
       die Code erzeugen, deshalb wird der Merker fuer verzoegerte
       Spruenge hier weiter geschaltet. */

    prev_delayed = curr_delayed;
    curr_delayed = False;

    /* Attribut verwursten */

    if (*AttrPart.str.p_str) {
        set_opsize(AttrPartOpSize[0], &AttrPart);
    }

    if (!LookupInstTable(InstTable, OpPart.str.p_str)) {
        WrStrErrorPos(ErrNum_UnknownInstruction, &OpPart);
    }
}

static void make_code_sh(void) {
    dsp_condition = 0;
    op_size       = eSymbolSizeUnknown;

    if (this_par)
        ;
    else if (
            dsp_avail && (strlen(OpPart.str.p_str) > 1) && (*OpPart.str.p_str == '+')
            && is_dsp_par_inst(OpPart.str.p_str + 1)) {
        this_par = True;
        StrCompCutLeft(&OpPart, 1);
    } else {
        this_par = False;
    }

    make_code_sh_core();

    this_par = False;
}

/*!------------------------------------------------------------------------
 * \fn     intern_symbol_sh(char *pArg, TempResult *pResult)
 * \brief  handle built-in symbols in SH7x00
 * \param  pArg source argument
 * \param  pResult result buffer
 * ------------------------------------------------------------------------ */

static void intern_symbol_sh(char* pArg, TempResult* pResult) {
    Word reg;

    if (decode_cpu_reg_core(pArg, &reg, &pResult->DataSize)) {
        pResult->Typ                       = TempReg;
        pResult->Contents.RegDescr.Reg     = reg;
        pResult->Contents.RegDescr.Dissect = dissect_reg_sh;
        pResult->Contents.RegDescr.compare = NULL;
    }
}

/*!------------------------------------------------------------------------
 * \fn     is_def_sh(void)
 * \brief  Does statement consume label field itself?
 * \return True if so
 * ------------------------------------------------------------------------ */

static Boolean is_def_sh(void) {
    /* REG is like EQU */

    if (Memo("REG")) {
        return True;
    }

    /* + as parallel marker only for DSP instructions supporting this: */

    else if (strcmp(LabPart.str.p_str, "+") || !dsp_avail) {
        return False;
    } else if (is_dsp_par_inst(OpPart.str.p_str)) {
        this_par = True;
        return True;
    } else {
        return False;
    }
}

/*!------------------------------------------------------------------------
 * \fn     switch_from_sh(void)
 * \brief  cleanups after switching away from target
 * ------------------------------------------------------------------------ */

static void switch_from_sh(void) {
    de_init_fields();
}

/*!------------------------------------------------------------------------
 * \fn     switch_to_sh(void *p_user)
 * \brief  initializations when switching to target
 * \param  p_user description of specific target
 * ------------------------------------------------------------------------ */

static void switch_to_sh(void* p_user) {
    TFamilyDescr const* p_descr = FindFamilyByName("SH7xx0");

    p_curr_cpu_props = (cpu_props_t const*)p_user;

    TurnWords = True;
    SetIntConstMode(eIntConstModeMoto);

    PCSymbol    = "*";
    HeaderID    = p_descr->Id;
    NOPCode     = 0x0009;
    DivideChars = ",";
    HasAttrs    = True;
    AttrChars   = ".";

    ValidSegs          = 1 << SegCode;
    Grans[SegCode]     = 1;
    ListGrans[SegCode] = 2;
    SegInits[SegCode]  = 0;
    SegLimits[SegCode] = (LargeWord)IntTypeDefs[UInt32].Max;

    DecodeAttrPart = decode_attr_part_sh;
    MakeCode       = make_code_sh;
    IsDef          = is_def_sh;
    InternSymbol   = intern_symbol_sh;
    DissectReg     = dissect_reg_sh;
    SwitchFrom     = switch_from_sh;
    init_fields();
    onoff_supmode_add();
    AddONOFF("COMPLITERALS", &compress_literals, CompLiteralsName, False);
    AddMoto16PseudoONOFF(False);

    if (p_curr_cpu_props->flags & e_core_flag_dsp) {
        if (!onoff_test_and_set(e_onoff_reg_dsp)) {
            SetFlag(&dsp_avail, DSPSymName, False);
        }
        AddONOFF(DSPCmdName, &dsp_avail, DSPSymName, False);
    }
    if (p_curr_cpu_props->flags & (e_core_flag_fpu32 | e_core_flag_fpu64)) {
        onoff_fpu_add();
    }

    curr_delayed = False;
    prev_delayed = False;
}

static cpu_props_t cpu_props[] = {
        {"SH7000",  e_core_sh1,           e_core_flag_none                               },
        {"SH7600",  e_core_sh2,                                          e_core_flag_none},
        {"SH7615",  e_core_sh2,                                           e_core_flag_dsp},
        {"SH7201", e_core_sh2a,                                          e_core_flag_none},
        {"SH7205", e_core_sh2a,                     e_core_flag_fpu32 | e_core_flag_fpu64},
        {"SH7700",  e_core_sh3,                                          e_core_flag_none},
        {"SH7720",  e_core_sh3,                                           e_core_flag_dsp},
        {"SH7750",  e_core_sh4, e_core_flag_fpu32 | e_core_flag_fpu64 | e_core_flag_fpugr},
        {"SH7780", e_core_sh4a,
         e_core_flag_fpu32 | e_core_flag_fpu64 | e_core_flag_fpugr                       },
        {      "",  e_core_sh1,                                          e_core_flag_none}
};

void code7000_init(void) {
    cpu_props_t const* p_prop;

    for (p_prop = cpu_props; p_prop->name[0]; p_prop++) {
        (void)AddCPUUser(p_prop->name, switch_to_sh, (void*)p_prop, NULL);
    }
}
