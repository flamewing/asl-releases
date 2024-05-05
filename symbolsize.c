/* symbolsize.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* Macro Assembler AS                                                        */
/*                                                                           */
/* Definition of a symbol's/instruction's operand type & size                */
/*                                                                           */
/*****************************************************************************/

#include "symbolsize.h"

#include <string.h>

typedef struct {
    tSymbolSize Size;
    char const* pName;
    unsigned    Bytes;
} tSizeDescr;

static tSizeDescr const Descrs[] = {
        {      eSymbolSizeUnknown,   "?",  0},
        {         eSymbolSize8Bit,  "I8",  1},
        {        eSymbolSize16Bit, "I16",  2},
        {        eSymbolSize32Bit, "I32",  4},
        {        eSymbolSize64Bit, "I64",  8},
        {        eSymbolSize80Bit, "F80", 10},
        {   eSymbolSizeFloat32Bit, "F32",  4},
        {   eSymbolSizeFloat64Bit, "F64",  8},
        {   eSymbolSizeFloat96Bit, "F96", 12},
        {        eSymbolSize24Bit, "I24",  3},
        {eSymbolSizeFloatDec96Bit, "D96", 12},
        {      eSymbolSizeUnknown,  NULL,  0},
};

/*!------------------------------------------------------------------------
 * \fn     GetSymbolSizeName(tSymbolSize Size)
 * \brief  retrieve human-readable name of symbol size
 * \param  Size size to query
 * \return name
 * ------------------------------------------------------------------------ */

char const* GetSymbolSizeName(tSymbolSize Size) {
    tSizeDescr const* pDescr;

    for (pDescr = Descrs; pDescr->pName; pDescr++) {
        if (pDescr->Size == Size) {
            return pDescr->pName;
        }
    }
    return "?";
}

/*!------------------------------------------------------------------------
 * \fn     GetSymbolSizeBytes(tSymbolSize Size)
 * \brief  retrieve # of bytes of symbol size
 * \param  Size size to query
 * \return # of bytes
 * ------------------------------------------------------------------------ */

unsigned GetSymbolSizeBytes(tSymbolSize Size) {
    tSizeDescr const* pDescr;

    for (pDescr = Descrs; pDescr->pName; pDescr++) {
        if (pDescr->Size == Size) {
            return pDescr->Bytes;
        }
    }
    return 0;
}
