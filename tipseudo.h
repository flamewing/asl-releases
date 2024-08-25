#ifndef TIPSEUDO_H
#define TIPSEUDO_H
/* tipseudo.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Texas Instruments Pseudo-Befehle                       */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

struct sInstTable;

extern Boolean DecodeTIPseudo(void);

extern Boolean IsTIDef(void);

extern void AddTI34xPseudo(struct sInstTable* pInstTable);

#endif /* TIPSEUDO_H */
