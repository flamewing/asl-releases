#ifndef _ASMSTRUCTS_H
#define _ASMSTRUCTS_H
/* asmstructs.h  */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* functions for structure handling                                          */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmstructs.h,v 1.5 2002/11/20 20:25:04 alfred Exp $                          */
/*****************************************************************************
 * $Log: asmstructs.h,v $
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
   
typedef struct _TStructElem
        {
          struct _TStructElem *Next;
          char *Name;
          LongInt Offset;
        } TStructElem, *PStructElem;

typedef struct _TStructRec
        {
          LongInt TotLen;
          PStructElem Elems;
          char ExtChar;
          Boolean DoExt;
          Boolean IsUnion;
        } TStructRec, *PStructRec;

typedef struct _TStructStack
         {
          struct _TStructStack *Next;
          char *Name;
          LargeWord CurrPC;
          PStructRec StructRec;
         } TStructStack, *PStructStack;

extern PStructStack StructStack;
extern int StructSaveSeg;

extern PStructRec CreateStructRec(void);

extern void DestroyStructRec(PStructRec StructRec);

extern void AddStructElem(PStructRec StructRec, char *Name, LongInt Offset);

extern void BumpStructLength(PStructRec StructRec, LongInt Length);

extern void AddStruct(PStructRec StructRec, char *Name, Boolean Protest);

extern Boolean FoundStruct(PStructRec *Erg);

extern void ResetStructDefines(void);

extern void PrintStructList(void);

extern void ClearStructList(void);

extern void ExpandStruct(PStructRec StructRec);

extern void asmstruct_init(void);

#endif /* _ASMSTRUCTS_H */
