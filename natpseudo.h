#ifndef NATPSEUDO_H
#define NATPSEUDO_H
/* natpseudo.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Pseudo Instructions commonly used on National targets                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

#include "datatypes.h"
struct sInstTable;

extern void AddNatPseudo(struct sInstTable* InstTable);

#endif /* NATPSEUDO_H */
