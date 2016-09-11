#ifndef _TIPSEUDO_H
#define _TIPSEUDO_H
/* tipseudo.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Texas Instruments Pseudo-Befehle                       */
/*                                                                           */
/*****************************************************************************/
/* $Id: tipseudo.h,v 1.3 2016/08/24 12:13:19 alfred Exp $                    */
/***************************************************************************** 
 * $Log: tipseudo.h,v $
 * Revision 1.3  2016/08/24 12:13:19  alfred
 * - begun with 320C4x support
 *
 * Revision 1.2  2014/11/03 17:36:12  alfred
 * - relocate IsDef() for common TI pseudo instructions
 *
 * Revision 1.1  2004/05/29 12:18:06  alfred
 * - relocated DecodeTIPseudo() to separate module
 *
 *****************************************************************************/

#include "bpemu.h"

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

struct sInstTable;

extern Boolean DecodeTIPseudo(void);

extern Boolean IsTIDef(void);

extern Boolean ExtToTIC34xShort(Double Inp, Word *Erg);

extern Boolean ExtToTIC34xSingle(Double Inp, LongWord *Erg);

extern Boolean ExtToTIC34xExt(Double Inp, LongWord *ErgL, LongWord *ErgH);

extern void AddTI34xPseudo(struct sInstTable *pInstTable);

#endif /* _TIPSEUDO_H */
