#ifndef _VERSION_H
#define _VERSION_H
/* version.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Lagert die Versionsnummer                                                 */
/*                                                                           */
/* Historie: 14.10.1997 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

extern char *Version;
extern LongInt VerNo;

extern char *InfoMessCopyright;

extern LongInt Magic;

extern void version_init(void);

#endif /* _VERSION_H */
