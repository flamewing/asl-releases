#ifndef STDHANDL_H
#define STDHANDL_H
/* stdhandl.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Bereitstellung von fuer AS benoetigten Handle-Funktionen                  */
/*                                                                           */
/* Historie:  5. 4.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>

typedef enum {NoRedir,RedirToDevice,RedirToFile} TRedirected;  /* Umleitung von Handles */

#define NumStdIn 0
#define NumStdOut 1
#define NumStdErr 2

extern TRedirected Redirected;

extern void OpenWithStandard(FILE **ppFile, char *Path);

extern void CloseIfOpen(FILE **ppFile);

extern void stdhandl_init(void);
#endif /* STDHANDL_H */
