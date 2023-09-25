#ifndef CODEVARS_H
#define CODEVARS_H
/* codevars.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Gemeinsame Variablen aller Codegeneratoren                                */
/*                                                                           */
/* Historie: 26.5.1997 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#include "asmitree.h"
#include "dyn_array.h"

#define order_array_free(orders) \
    do {                         \
        free(orders);            \
        orders = NULL;           \
    } while (0)

extern int InstrZ;
extern int AdrCnt;

extern PInstTable InstTable;

#endif /* CODEVARS_H */
