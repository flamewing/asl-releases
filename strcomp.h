#ifndef _STRCOMP_H
#define _STRCOMP_H
/* strcomp.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* Macro Assembler AS                                                        */
/*                                                                           */
/* Definition of a source line's component present after parsing             */
/*                                                                           */
/*****************************************************************************/

typedef char *StringPtr;

struct sLineComp
{
  int StartCol;
  unsigned Len;
};
typedef struct sLineComp tLineComp;

struct sStrComp
{
  tLineComp Pos;
  StringPtr Str;
};
typedef struct sStrComp tStrComp;

extern void StrCompAlloc(tStrComp *pComp);
extern void StrCompFree(tStrComp *pComp);

extern void StrCompReset(tStrComp *pComp);
extern void LineCompReset(tLineComp *pComp);

extern void StrCompMkTemp(tStrComp *pComp, char *pStr);

extern void StrCompRefRight(tStrComp *pDest, const tStrComp *pSrc, int StartOffs);

extern void StrCompCopy(tStrComp *pDest, const tStrComp *pSrc);

extern void StrCompCopySub(tStrComp *pDest, const tStrComp *pSrc, unsigned Start, unsigned Count);

extern void StrCompSplitRight(tStrComp *pSrc, tStrComp *pDest, char *pSrcSplitPos);

extern void StrCompSplitLeft(tStrComp *pSrc, tStrComp *pDest, char *pSrcSplitPos);

extern void StrCompSplitCopy(tStrComp *pLeft, tStrComp *pRight, const tStrComp *pSrc, char *pSplitPos);
extern char StrCompSplitRef(tStrComp *pLeft, tStrComp *pRight, const tStrComp *pSrc, char *pSplitPos);

extern void StrCompShorten(struct sStrComp *pComp, int Delta);

extern int StrCompCutLeft(struct sStrComp *pComp, int Delta);
extern void StrCompIncRefLeft(struct sStrComp *pComp, int Delta);

extern void KillPrefBlanksStrComp(struct sStrComp *pComp);
extern void KillPrefBlanksStrCompRef(struct sStrComp *pComp);

extern void KillPostBlanksStrComp(struct sStrComp *pComp);

extern void DumpStrComp(const char *pTitle, const struct sStrComp *pComp);

#endif /* _STRCOMP_H */
