/* intformat.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* enums regarding integer constant notations                                */
/*                                                                           */
/*****************************************************************************/

#include "intformat.h"

#include "datatypes.h"
#include "strutil.h"

#include <stdlib.h>
#include <string.h>

static Byte const BaseVals[3] = {2, 8, 16};

LongWord        NativeIntConstModeMask, OtherIntConstModeMask;
tIntFormatList* IntFormatList = NULL;
tIntConstMode   IntConstMode;
Boolean         IntConstModeIBMNoTerm, RelaxedMode;
int             RadixBase;

static Boolean ChkIntFormatCHex(tIntCheckCtx* pCtx, char Ch) {
    if ((pCtx->ExprLen > 2) && (*pCtx->pExpr == '0') && (RadixBase <= Ch - 'A' + 10)
        && (as_toupper(pCtx->pExpr[1]) == Ch)) {
        pCtx->pExpr += 2;
        pCtx->ExprLen -= 2;
        return True;
    }
    return False;
}

static Boolean ChkIntFormatCBin(tIntCheckCtx* pCtx, char Ch) {
    if ((pCtx->ExprLen > 2) && (*pCtx->pExpr == '0') && (RadixBase <= Ch - 'A' + 10)
        && (as_toupper(pCtx->pExpr[1]) == Ch)) {
        char const* pRun;

        for (pRun = pCtx->pExpr + 2; pRun < pCtx->pExpr + pCtx->ExprLen; pRun++) {
            if (DigitVal(*pRun, 2) < 0) {
                return False;
            }
        }
        pCtx->pExpr += 2;
        pCtx->ExprLen -= 2;
        return True;
    }
    return False;
}

static Boolean ChkIntFormatMot(tIntCheckCtx* pCtx, char Ch) {
    if ((pCtx->ExprLen > 1) && (*pCtx->pExpr == Ch)) {
        pCtx->pExpr++;
        pCtx->ExprLen--;
        return True;
    }
    return False;
}

static Boolean ChkIntFormatInt(tIntCheckCtx* pCtx, char Ch) {
    if ((pCtx->ExprLen < 2) || !as_isdigit(*pCtx->pExpr)) {
        return False;
    }
    if ((RadixBase <= Ch - 'A' + 10)
        && (as_toupper(pCtx->pExpr[pCtx->ExprLen - 1]) == Ch)) {
        pCtx->ExprLen--;
        return True;
    }
    return False;
}

static Boolean ChkIntFormatIBM(tIntCheckCtx* pCtx, char Ch) {
    if ((pCtx->ExprLen < 3) || (as_toupper(*pCtx->pExpr) != Ch)
        || (pCtx->pExpr[1] != '\'')) {
        return False;
    }
    if ((pCtx->ExprLen > 3) && (pCtx->pExpr[pCtx->ExprLen - 1] == '\'')) {
        pCtx->pExpr += 2;
        pCtx->ExprLen -= 3;
        return True;
    } else if (IntConstModeIBMNoTerm) {
        pCtx->pExpr += 2;
        pCtx->ExprLen -= 2;
        return True;
    }
    return False;
}

static Boolean ChkIntFormatCOct(tIntCheckCtx* pCtx, char Ch) {
    char const* pRun;
    UNUSED(Ch);

    if ((pCtx->ExprLen < 2) || (*pCtx->pExpr != '0')) {
        return False;
    }
    for (pRun = pCtx->pExpr + 1; pRun < pCtx->pExpr + pCtx->ExprLen; pRun++) {
        if (DigitVal(*pRun, 8) < 0) {
            return False;
        }
    }
    return True;
}

static Boolean ChkIntFormatNatHex(tIntCheckCtx* pCtx, char Ch) {
    char const* pRun;
    UNUSED(Ch);

    if ((pCtx->ExprLen < 2) || (*pCtx->pExpr != '0')) {
        return False;
    }
    for (pRun = pCtx->pExpr + 1; pRun < pCtx->pExpr + pCtx->ExprLen; pRun++) {
        if (!as_isxdigit(*pRun)) {
            return False;
        }
    }
    return True;
}

static Boolean ChkIntFormatDef(tIntCheckCtx* pCtx, char Ch) {
    UNUSED(pCtx);
    UNUSED(Ch);
    return True;
}

static tIntFormatList const IntFormatList_All[] = {
        {  ChkIntFormatCHex,     eIntFormatCHex, 16,  'X',  "0xhex"},
        {  ChkIntFormatCBin,     eIntFormatCBin,  2,  'B',  "0bbin"},
        {   ChkIntFormatMot,   eIntFormatMotHex, 16,  '$',   "$hex"},
        {   ChkIntFormatMot,   eIntFormatMotBin,  2,  '%',   "%bin"},
        {   ChkIntFormatMot,   eIntFormatMotOct,  8,  '@',   "@oct"},
        {   ChkIntFormatInt,   eIntFormatIntHex, 16,  'H',   "hexh"},
        {   ChkIntFormatInt,   eIntFormatIntBin,  2,  'B',   "binb"},
        {   ChkIntFormatInt,  eIntFormatIntOOct,  8,  'O',   "octo"},
        {   ChkIntFormatInt,  eIntFormatIntQOct,  8,  'Q',   "octq"},
        {   ChkIntFormatIBM,  eIntFormatIBMHHex, 16,  'H', "h'hex'"},
        {   ChkIntFormatIBM,  eIntFormatIBMXHex, 16,  'X', "x'hex'"},
        {   ChkIntFormatIBM,   eIntFormatIBMBin,  2,  'B', "b'bin'"},
        {   ChkIntFormatIBM,   eIntFormatIBMOct,  8,  'O', "o'oct'"},
        {  ChkIntFormatCOct,     eIntFormatCOct,  8,  '0',   "0oct"},
        {ChkIntFormatNatHex,   eIntFormatNatHex, 16,  '0',   "0hex"},
        {   ChkIntFormatDef, eIntFormatDefRadix, -1, '\0',    "dec"}, /* -1 -> RadixBase */
        {              NULL,    (tIntFormatId)0,  0, '\0',       ""}
};

/*!------------------------------------------------------------------------
 * \fn     GetIntConstIntelSuffix(unsigned Radix)
 * \brief  return Intel-style suffix letter fitting to number system
 * \param  Radix req'd number system
 * \return * to suffix string (may be empty)
 * ------------------------------------------------------------------------ */

char const* GetIntConstIntelSuffix(unsigned Radix) {
    static char const BaseLetters[3] = {'B', 'O', 'H'};
    unsigned          BaseIdx;

    for (BaseIdx = 0; BaseIdx < sizeof(BaseVals) / sizeof(*BaseVals); BaseIdx++) {
        if (Radix == BaseVals[BaseIdx]) {
            static char Result[2] = {'\0', '\0'};

            Result[0] = BaseLetters[BaseIdx] + (HexStartCharacter - 'A');
            return Result;
        }
    }
    return "";
}

/*!------------------------------------------------------------------------
 * \fn     GetIntConstMotoPrefix(unsigned Radix)
 * \brief  return Motorola-style prefix letter fitting to number system
 * \param  Radix req'd number system
 * \return * to prefix string (may be empty)
 * ------------------------------------------------------------------------ */

char const* GetIntConstMotoPrefix(unsigned Radix) {
    static char const BaseIds[3] = {'%', '@', '$'};
    unsigned          BaseIdx;

    for (BaseIdx = 0; BaseIdx < sizeof(BaseVals) / sizeof(*BaseVals); BaseIdx++) {
        if (Radix == BaseVals[BaseIdx]) {
            static char Result[2] = {'\0', '\0'};

            Result[0] = BaseIds[BaseIdx];
            return Result;
        }
    }
    return "";
}

/*!------------------------------------------------------------------------
 * \fn     GetIntConstCPrefix(unsigned Radix)
 * \brief  return C-style prefix letter fitting to number system
 * \param  Radix req'd number system
 * \return * to prefix string (may be empty)
 * ------------------------------------------------------------------------ */

char const* GetIntConstCPrefix(unsigned Radix) {
    static char const BaseIds[3][3] = {"0b", "0", "0x"};
    unsigned          BaseIdx;

    for (BaseIdx = 0; BaseIdx < sizeof(BaseVals) / sizeof(*BaseVals); BaseIdx++) {
        if (Radix == BaseVals[BaseIdx]) {
            return BaseIds[BaseIdx];
        }
    };
    return "";
}

/*!------------------------------------------------------------------------
 * \fn     GetIntConstIBMPrefix(unsigned Radix)
 * \brief  return IBM-style prefix letter fitting to number system
 * \param  Radix req'd number system
 * \return * to prefix string (may be empty)
 * ------------------------------------------------------------------------ */

char const* GetIntConstIBMPrefix(unsigned Radix) {
    static char const BaseIds[3] = {'B', 'O', 'X'};
    unsigned          BaseIdx;

    for (BaseIdx = 0; BaseIdx < sizeof(BaseVals) / sizeof(*BaseVals); BaseIdx++) {
        if (Radix == BaseVals[BaseIdx]) {
            static char Result[3] = {'\0', '\'', '\0'};

            Result[0] = BaseIds[BaseIdx] + (HexStartCharacter - 'A');
            return Result;
        }
    }
    return "";
}

/*!------------------------------------------------------------------------
 * \fn     GetIntConstIBMSuffix(unsigned Radix)
 * \brief  return IBM-style suffix fitting to number system
 * \param  Radix req'd number system
 * \return * to prefix string (may be empty)
 * ------------------------------------------------------------------------ */

char const* GetIntConstIBMSuffix(unsigned Radix) {
    unsigned BaseIdx;

    for (BaseIdx = 0; BaseIdx < sizeof(BaseVals) / sizeof(*BaseVals); BaseIdx++) {
        if (Radix == BaseVals[BaseIdx]) {
            return "\'";
        }
    }
    return "";
}

/*!------------------------------------------------------------------------
 * \fn     SetIntConstModeByMask(LongWord Mask)
 * \brief  set new (non-relaxed) integer constant mode by bit mask
 * \param  Mask modes to set
 * ------------------------------------------------------------------------ */

void SetIntConstModeByMask(LongWord Mask) {
    tIntFormatList const* pSrc;
    tIntFormatList*       pDest;

    if (!IntFormatList) {
        IntFormatList = (tIntFormatList*)malloc(sizeof(IntFormatList_All));
    }
    for (pDest = IntFormatList, pSrc = IntFormatList_All; pSrc->Check; pSrc++) {
        if (!((Mask >> pSrc->Id) & 1)) {
            continue;
        }
        *pDest++ = *pSrc;
    }
    memset(pDest, 0, sizeof(*pDest));
}

/*!------------------------------------------------------------------------
 * \fn     ModifyIntConstModeByMask(LongWord ANDMask, LongWord ORMask)
 * \brief  add or remove integer notations to/from native list
 * \param  ANDMask notations to remove
 * \param  ORMask notations to add
 * \return True if mask was set up successfully
 * ------------------------------------------------------------------------ */

#define BadMask ((1ul << eIntFormatCOct) | (1ul << eIntFormatNatHex))

Boolean ModifyIntConstModeByMask(LongWord ANDMask, LongWord ORMask) {
    LongWord NewMask = (NativeIntConstModeMask & ~ANDMask) | ORMask;

    if ((NewMask & BadMask) == BadMask) {
        return False;
    } else {
        NativeIntConstModeMask = NewMask;
        SetIntConstModeByMask(
                NativeIntConstModeMask | (RelaxedMode ? OtherIntConstModeMask : 0));
        return True;
    }
}

/*!------------------------------------------------------------------------
 * \fn     SetIntConstMode(tIntConstMode Mode)
 * \brief  set new (non-relaxed) integer constant mode
 * \param  Mode mode to set
 * ------------------------------------------------------------------------ */

void SetIntConstMode(tIntConstMode Mode) {
    IntConstMode = Mode;
    switch (Mode) {
    case eIntConstModeC:
        NativeIntConstModeMask = eIntFormatMaskC;
        OtherIntConstModeMask
                = eIntFormatMaskIntel | eIntFormatMaskMoto | eIntFormatMaskIBM;
        break;
    case eIntConstModeIntel:
        NativeIntConstModeMask = eIntFormatMaskIntel;
        OtherIntConstModeMask  = eIntFormatMaskC | eIntFormatMaskMoto | eIntFormatMaskIBM;
        break;
    case eIntConstModeMoto:
        NativeIntConstModeMask = eIntFormatMaskMoto;
        OtherIntConstModeMask = eIntFormatMaskC | eIntFormatMaskIntel | eIntFormatMaskIBM;
        break;
    case eIntConstModeIBM:
        NativeIntConstModeMask = eIntFormatMaskIBM;
        OtherIntConstModeMask
                = eIntFormatMaskC | eIntFormatMaskIntel | eIntFormatMaskMoto;
        break;
    default:
        NativeIntConstModeMask = 0;
    }
    NativeIntConstModeMask |= (1ul << eIntFormatDefRadix);
    SetIntConstModeByMask(
            NativeIntConstModeMask | (RelaxedMode ? OtherIntConstModeMask : 0));
}

/*!------------------------------------------------------------------------
 * \fn     SetIntConstRelaxedMode(Boolean NewRelaxedMode)
 * \brief  update relaxed mode - parser list
 * \param  NewRelaxedMode mode to set
 * ------------------------------------------------------------------------ */

void SetIntConstRelaxedMode(Boolean NewRelaxedMode) {
    SetIntConstModeByMask(
            NativeIntConstModeMask | (NewRelaxedMode ? OtherIntConstModeMask : 0));
}

/*!------------------------------------------------------------------------
 * \fn     GetIntFormatId(const char *pIdent)
 * \brief  transform identifier to id
 * \param  pIdent textual identifier
 * \return resulting Id or None if not found
 * ------------------------------------------------------------------------ */

tIntFormatId GetIntFormatId(char const* pIdent) {
    tIntFormatList const* pList;
    for (pList = IntFormatList_All; pList->Check; pList++) {
        if (!as_strcasecmp(pIdent, pList->Ident)) {
            return (tIntFormatId)pList->Id;
        }
    }
    return eIntFormatNone;
}

/*!------------------------------------------------------------------------
 * \fn     intformat_init(void)
 * \brief  module initialization
 * ------------------------------------------------------------------------ */

void intformat_init(void) {
    /* Allow all int const modes for handling possible -D options: */

    RelaxedMode = True;
    SetIntConstMode(eIntConstModeC);
}
