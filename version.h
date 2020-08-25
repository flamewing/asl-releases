#ifndef _VERSION_H
#define _VERSION_H
/* version.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Lagert die Versionsnummer                                                 */
/*                                                                           */
/*****************************************************************************/

extern const char *Version;
extern LongInt VerNo;

extern const char *InfoMessCopyright;

extern LongInt Magic;

extern void version_init(void);

#endif /* _VERSION_H */
