#ifndef _IOERRS_H
#define _IOERRS_H
/* ioerrs.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Abliefern der I/O-Fehlermeldungen                                         */
/*                                                                           */
/* Historie: 11.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

extern char *GetErrorMsg(int number);

extern void ioerrs_init(char *ProgPath);
#endif /* _IOERRS_H */
