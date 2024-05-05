#ifndef OPERATOR_H
#define OPERATOR_H
/* operator.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* defintion of operators                                                    */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "tempresult.h"

#define OPERATOR_MAXCNT  30
#define OPERATOR_MAXCOMB 5

typedef struct {
    char const* Id;
    int         IdLen;
    Boolean     Dyadic;
    Byte        Priority;
    Byte        TypeCombinations[OPERATOR_MAXCOMB];
    void (*pFunc)(TempResult* pErg, TempResult* pLVal, TempResult* pRVal);
} Operator;

extern Operator const Operators[], MinusMonadicOperator, *pPotMonadicOperator;

#endif /* OPERATOR_H */
