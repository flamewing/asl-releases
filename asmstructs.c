/* asmstructs.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* structure handling                                                        */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmstructs.c,v 1.12 2016/09/22 15:36:15 alfred Exp $                 */
/*****************************************************************************
 * $Log: asmstructs.c,v $
 * Revision 1.12  2016/09/22 15:36:15  alfred
 * - use platform-dependent format string for LongInt
 *
 * Revision 1.11  2016/09/12 19:46:56  alfred
 * - initialize some elements in constructor
 *
 * Revision 1.10  2015/10/28 17:54:33  alfred
 * - allow substructures of same name in different structures
 *
 * Revision 1.9  2015/10/23 08:43:33  alfred
 * - beef up & fix structure handling
 *
 * Revision 1.8  2015/10/18 20:08:52  alfred
 * - when expanding structure, also regard sub-structures
 *
 * Revision 1.7  2015/10/18 19:02:16  alfred
 * - first reork/fix of nested structure handling
 *
 * Revision 1.6  2014/12/07 19:13:59  alfred
 * - silence a couple of Borland C related warnings and errors
 *
 * Revision 1.5  2014/12/05 11:09:10  alfred
 * - eliminate Nil
 *
 * Revision 1.4  2013-02-14 21:05:31  alfred
 * - add missing bookkeeping for expanded structs
 *
 * Revision 1.3  2004/01/17 16:18:38  alfred
 * - fix some more GCC 3.3 quarrel
 *
 * Revision 1.2  2004/01/17 16:12:50  alfred
 * - some quirks for GCC 3.3
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.6  2002/11/20 20:25:04  alfred
 * - added unions
 *
 * Revision 1.5  2002/11/16 20:50:02  alfred
 * - added expansion routine
 *
 * Revision 1.4  2002/11/15 23:30:31  alfred
 * - added search routine
 *
 * Revision 1.3  2002/11/11 21:56:57  alfred
 * - store/display struct elements
 *
 * Revision 1.2  2002/11/11 21:12:32  alfred
 * - first working edition
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "nls.h"
#include "nlmessages.h"
#include "strutil.h"

#include "trees.h"

#include "as.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmstructs.h"

#include "as.rsc"

/*****************************************************************************/

typedef struct sStructNode
{
  TTree Tree;
  Boolean Defined;
  PStructRec StructRec;
} TStructNode, *PStructNode;

/*****************************************************************************/

PStructStack StructStack,        /* momentan offene Strukturen */
             pInnermostNamedStruct;
int StructSaveSeg;               /* gesichertes Segment waehrend Strukturdef.*/
PStructNode StructRoot = NULL;

/*****************************************************************************/

PStructRec CreateStructRec(void)
{
  PStructRec Neu;

  Neu = (PStructRec) malloc(sizeof(TStructRec));
  if (Neu)
  {
    Neu->TotLen = 0;
    Neu->Elems = NULL;
    Neu->ExtChar = '\0';
    Neu->DoExt = Neu->IsUnion = False;
  }
  return Neu;
}

void DestroyStructRec(PStructRec StructRec)
{
  PStructElem Old;

  while (StructRec->Elems)
  {
    Old = StructRec->Elems;
    StructRec->Elems = Old->Next;
    free(Old->Name);
    free(Old);
  }
  free(StructRec);
}

void AddStructElem(PStructRec StructRec, char *Name, Boolean IsStruct, LongInt Offset)
{
  PStructElem Neu, Run;

  Neu = (PStructElem) malloc(sizeof(TStructElem));
  if (Neu)
  {
    Neu->Name = strdup(Name);
    if (!CaseSensitive)
      NLS_UpString(Neu->Name);
    Neu->IsStruct = IsStruct;
    Neu->Offset = Offset;
    Neu->Next = NULL;
    if (!StructRec->Elems)
      StructRec->Elems = Neu;
    else
    {
      for (Run = StructRec->Elems; Run->Next; Run = Run->Next);
      Run->Next = Neu;
    }
  }
  BumpStructLength(StructRec, Offset);
}

void BuildStructName(char *pResult, unsigned ResultLen, const char *pName)
{
  PStructStack ZStruct;
  String tmp2;

  strmaxcpy(pResult, pName, ResultLen);
  for (ZStruct = StructStack; ZStruct; ZStruct = ZStruct->Next)
    if (ZStruct->StructRec->DoExt && ZStruct->Name[0])
    {
      sprintf(tmp2, "%s%c", ZStruct->pBaseName, ZStruct->StructRec->ExtChar);
      strmaxprep(pResult, tmp2, ResultLen);
    }
}

void AddStructSymbol(const char *pName, LargeWord Value)
{
  PStructStack ZStruct;

  /* what we get is offset/length in current structure.  Add to
     it all offsets in parent structures, i.e. leave out SaveCurrPC
     of bottom of stack which contains saved PC of non-struct segment: */

  for (ZStruct = StructStack; ZStruct->Next; ZStruct = ZStruct->Next)
    Value += ZStruct->SaveCurrPC;

  {
    String tmp, tmp2;

    sprintf(tmp2, "%s%c", pInnermostNamedStruct->Name, pInnermostNamedStruct->StructRec->ExtChar);
    strmaxcpy(tmp, pName, sizeof(tmp));
    strmaxprep(tmp, tmp2, sizeof(tmp));
    EnterIntSymbol(tmp, Value, SegNone, False);
  }
}

void BumpStructLength(PStructRec StructRec, LongInt Length)
{
  if (StructRec->TotLen < Length)
    StructRec->TotLen = Length;
}

static Boolean StructAdder(PTree *PDest, PTree Neu, void *pData)
{
  PStructNode NewNode = (PStructNode) Neu, *Node;
  Boolean Protest = *((Boolean*)pData), Result = False;

  if (!PDest)
  {
    NewNode->Defined = TRUE;
    return True;
  }

  Node = (PStructNode*) PDest;
  if ((*Node)->Defined)
  {
    if (Protest)
      WrXError(1815, Neu->Name);
    else
    {
      DestroyStructRec((*Node)->StructRec);
      (*Node)->StructRec = NewNode->StructRec;
    }
  }
  else
  {
    DestroyStructRec((*Node)->StructRec);
    (*Node)->StructRec = NewNode->StructRec;
    (*Node)->Defined = True;
    return True;
  }
  return Result;
}

void AddStruct(PStructRec StructRec, char *Name, Boolean Protest)
{
  PStructNode Node;
  PTree TreeRoot;

  Node = (PStructNode) malloc(sizeof(TStructNode));
  if (Node)
  {
    Node->Tree.Left = Node->Tree.Right = NULL;
    Node->Tree.Name = strdup(Name);
    if (!CaseSensitive)
      NLS_UpString(Node->Tree.Name);
    Node->Tree.Attribute = MomSectionHandle;
    Node->Defined = FALSE;
    Node->StructRec = StructRec;
    TreeRoot = &(StructRoot->Tree);
    EnterTree(&TreeRoot, &(Node->Tree), StructAdder, &Protest);
    StructRoot = (PStructNode)TreeRoot;
  }
}

static PStructRec FoundStruct_FNode(LongInt Handle, char *Name)
{
  PStructNode Lauf;

  Lauf = (PStructNode) SearchTree((PTree)StructRoot, Name, Handle);
  return Lauf ? Lauf->StructRec : NULL;
}

Boolean FoundStruct(PStructRec *Erg, const char *pName)
{
  PSaveSection Lauf;
  String Part;

  strmaxcpy(Part, pName, 255);
  if (!CaseSensitive)
    NLS_UpString(Part);

  *Erg = FoundStruct_FNode(MomSectionHandle, Part);
  if (*Erg)
    return True;
  Lauf = SectionStack;
  while (Lauf)
  {
    *Erg = FoundStruct_FNode(Lauf->Handle, Part);
    if (*Erg)
      return True;
    Lauf = Lauf->Next;
  }
  return False;
}

static void ResDef(PTree Tree, void *pData)
{
  UNUSED(pData);

  ((PStructNode)Tree)->Defined = FALSE;
}

void ResetStructDefines(void)
{
  IterTree((PTree)StructRoot, ResDef, NULL);
}

typedef struct
{
  LongInt Sum;
} TPrintContext;

static void PrintDef(PTree Tree, void *pData)
{
  PStructNode Node = (PStructNode)Tree;
  PStructElem Elem;
  TPrintContext *pContext = (TPrintContext*)pData;
  String s;
  UNUSED(pData);

  WrLstLine("");
  pContext->Sum++;
  strmaxcpy(s, Node->Tree.Name, 255);
  if (Node->Tree.Attribute != -1)
  {
    strmaxcat(s, "[", 255);
    strmaxcat(s, GetSectionName(Node->Tree.Attribute), 255);
    strmaxcat(s, "]", 255);
  }
  WrLstLine(s);
  for (Elem = Node->StructRec->Elems; Elem; Elem = Elem->Next)
  {
    sprintf(s, "%3" PRILongInt " ", Elem->Offset);
    strmaxcat(s, Elem->Name, 255);
    WrLstLine(s);
  }
}

void PrintStructList(void)
{
  TPrintContext Context;
  String s;

  if (!StructRoot)
    return;

  NewPage(ChapDepth, True);
  WrLstLine(getmessage(Num_ListStructListHead1));
  WrLstLine(getmessage(Num_ListStructListHead2));

  Context.Sum = 0;
  IterTree((PTree)StructRoot, PrintDef, &Context);
  sprintf(s, "%" PRILongInt "%s", Context.Sum,
          getmessage((Context.Sum == 1) ? Num_ListStructSumMsg : Num_ListStructSumsMsg));
}

static void ClearNode(PTree Tree, void *pData)
{
  PStructNode Node = (PStructNode) Tree;
  UNUSED(pData);

  DestroyStructRec(Node->StructRec);
}

void ClearStructList(void)
{
  PTree TreeRoot;

  TreeRoot = &(StructRoot->Tree);
  StructRoot = NULL;
  DestroyTree(&TreeRoot, ClearNode, NULL);
}

static void ExpandStruct_One(PStructRec StructRec, char *pVarPrefix, char *pStructPrefix, LargeWord Base)
{
  PStructElem StructElem;
  int VarLen, RemVarLen, StructLen, RemStructLen;

  VarLen = strlen(pVarPrefix);
  pVarPrefix[VarLen] = StructRec->ExtChar;
  RemVarLen = STRINGSIZE - 3 - VarLen;

  StructLen = strlen(pStructPrefix);
  pStructPrefix[StructLen] = StructRec->ExtChar;
  RemStructLen = STRINGSIZE - 3 - StructLen;

  if ((RemVarLen > 1) && (RemStructLen > 1))
  {
    for (StructElem = StructRec->Elems; StructElem; StructElem = StructElem->Next)
    {
      strmaxcpy(pVarPrefix + VarLen + 1, StructElem->Name, RemVarLen);
      HandleLabel(pVarPrefix, Base + StructElem->Offset);
      if (StructElem->IsStruct)
      {
        TStructRec *pSubStruct;
        Boolean Found;

        strmaxcpy(pStructPrefix + StructLen + 1, StructElem->Name, RemStructLen);
        Found = FoundStruct(&pSubStruct, pStructPrefix);
        if (Found)
          ExpandStruct_One(pSubStruct, pVarPrefix, pStructPrefix, Base + StructElem->Offset);
      }
    }
  }
  pVarPrefix[VarLen] = '\0';
  pStructPrefix[StructLen] = '\0';
}

void ExpandStruct(PStructRec StructRec)
{
  if (!LabPart[0]) WrError(2040);
  else
  {
    String CompVarName, CompStructName;

    strmaxcpy(CompVarName, LabPart, sizeof(CompVarName));
    strmaxcpy(CompStructName, LOpPart, sizeof(CompStructName));
    ExpandStruct_One(StructRec, LabPart, LOpPart, EProgCounter());
    CodeLen = StructRec->TotLen;
    BookKeeping();
    DontPrint = True;
  }
}

void asmstruct_init(void)
{
   StructRoot = NULL;
}
