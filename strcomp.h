#ifndef _STRCOMP_H
#define _STRCOMP_H
/* strcomp.h */
/*****************************************************************************/
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

extern void StrCompReset(tStrComp *pComp);
extern void LineCompReset(tLineComp *pComp);

extern void StrCompCopy(tStrComp *pDest, tStrComp *pSrc);

#endif /* _STRCOMP_H */
