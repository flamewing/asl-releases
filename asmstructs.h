#ifndef ASMSTRUCTS_H
#define ASMSTRUCTS_H
/* asmstructs.h  */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* functions for structure handling                                          */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"
#include "symbolsize.h"

struct sStructElem;
struct sStrComp;

typedef void (*tStructElemExpandFnc)(const struct sStrComp *pVarName, const struct sStructElem *pStructElem, LargeWord Base);

typedef struct sStructElem
{
  struct sStructElem *Next;
  char *pElemName, *pRefElemName;
  Boolean IsStruct;
  tStructElemExpandFnc ExpandFnc;
  LongInt Offset;
  ShortInt BitPos; /* -1 -> no bit position */
  ShortInt BitWidthM1; /* -1 -> no bit field, otherwise actual width minus one */
  tSymbolSize OpSize;
} TStructElem, *PStructElem;

typedef struct sStructRec
{
  LongInt TotLen;
  PStructElem Elems;
  char ExtChar;
  Boolean DoExt;
  Boolean IsUnion;
} TStructRec, *PStructRec;

typedef struct sStructStack
{
  struct sStructStack *Next;
  char *Name, *pBaseName;
  LargeWord SaveCurrPC, SaveOffsetToInnermost;
  PStructRec StructRec;
} TStructStack, *PStructStack;

extern PStructStack StructStack, pInnermostNamedStruct;
extern int StructSaveSeg;

extern PStructRec CreateStructRec(void);

extern void DestroyStructElem(PStructElem pStructElem);

extern void DestroyStructRec(PStructRec StructRec);

extern void BuildStructName(char *pResult, unsigned ResultLen, const char *pName);

extern PStructElem CreateStructElem(const struct sStrComp *pElemName);

extern PStructElem CloneStructElem(const struct sStrComp *pCloneElemName, const struct sStructElem *pSrc);

extern Boolean AddStructElem(PStructRec pStructRec, PStructElem pElement);

extern void SetStructElemSize(PStructRec pStructRec, const char *pElemName, tSymbolSize Size);

extern void AddStructSymbol(const char *pName, LargeWord Value);

extern void ResolveStructReferences(PStructRec pStructRec);

extern void BumpStructLength(PStructRec StructRec, LongInt Length);

extern void AddStruct(PStructRec StructRec, char *Name, Boolean Protest);

extern Boolean FoundStruct(PStructRec *Erg, const char *pName);

extern void ResetStructDefines(void);

extern void PrintStructList(void);

extern void ClearStructList(void);

extern void ExpandStruct(PStructRec StructRec);

extern void asmstruct_init(void);

#endif /* ASMSTRUCTS_H */
