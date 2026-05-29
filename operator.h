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

#define OPERATOR_MAXCNT  32
#define OPERATOR_MAXCOMB 5

typedef enum {
    e_op_monadic      = 0,
    e_op_dyadic       = 1,
    e_op_dyadic_short = 2
} as_operator_prop_t;

typedef struct as_operator {
    char const* Id;
    int         IdLen;
    Byte        op_type;
    Byte        Priority;
    Byte        TypeCombinations[OPERATOR_MAXCOMB];
    Boolean (*pFunc)(TempResult* pErg, TempResult* pLVal, TempResult* pRVal);
} as_operator_t;

extern as_operator_t const operators[], *target_operators, no_operators[];

extern int reg_cmp(TempResult const* p_val1, TempResult const* p_val2);

#endif /* OPERATOR_H */
