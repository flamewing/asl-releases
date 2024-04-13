#ifndef _TEXTOC_H
#define _TEXTOC_H
/* textoc.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* TeX-->ASCII/HTML Converter: Table Of Contents                             */
/*                                                                           */
/*****************************************************************************/

extern void InitToc(void);

extern void AddToc(const char *pLine, unsigned Indent);

extern void PrintToc(char *pFileName);

extern void FreeToc(void);

#endif /* _TEXTOC_H */
