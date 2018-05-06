#ifndef _ASMSTRUCTS_H
#define _ASMSTRUCTS_H
/* asmstructs.h  */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* functions for structure handling                                          */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmstructs.h,v 1.5 2015/10/28 17:54:33 alfred Exp $                          */
/*****************************************************************************
 * $Log: asmstructs.h,v $
 * Revision 1.5  2015/10/28 17:54:33  alfred
 * - allow substructures of same name in different structures
 *
 * Revision 1.4  2015/10/23 08:43:33  alfred
 * - beef up & fix structure handling
 *
 * Revision 1.3  2015/10/18 20:08:52  alfred
 * - when expanding structure, also regard sub-structures
 *
 * Revision 1.2  2015/10/18 19:02:16  alfred
 * - first reork/fix of nested structure handling
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.5  2002/11/20 20:25:04  alfred
 * - added unions
 *
 * Revision 1.4  2002/11/16 20:52:35  alfred
 * - added expansion routine
 *
 * Revision 1.3  2002/11/15 23:30:31  alfred
 * - added search routine
 *
 * Revision 1.2  2002/11/11 21:12:32  alfred
 * - first working edition
 *
 * Revision 1.1  2002/11/11 19:24:57  alfred
 * - new module for structs
 *
 * Revision 1.9  2002/11/10 16:27:32  alfred
 *****************************************************************************/
   
struct sStructElem;

typedef void (*tStructElemExpandFnc)(const char *pVarName, const struct sStructElem *pStructElem, LargeWord Base);

typedef struct sStructElem
{
  struct sStructElem *Next;
  char *pElemName, *pRefElemName;
  Boolean IsStruct;
  tStructElemExpandFnc ExpandFnc;
  LongInt Offset;
  ShortInt BitPos; /* -1 -> no bit position */
  ShortInt BitWidthM1; /* -1 -> no bit field, otherwise actual width minus one */
  ShortInt OpSize;
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

extern void DestroyStructRec(PStructRec StructRec);

extern void BuildStructName(char *pResult, unsigned ResultLen, const char *pName);

extern PStructElem CreateStructElem(const char *pElemName);

extern void AddStructElem(PStructRec pStructRec, PStructElem pElement);

extern void SetStructElemSize(PStructRec pStructRec, const char *pElemName, ShortInt Size);

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

#endif /* _ASMSTRUCTS_H */
