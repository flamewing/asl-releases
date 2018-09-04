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
#include "errmsg.h"

#include "as.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmstructs.h"
#include "errmsg.h"

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
    if (Old->pElemName) free(Old->pElemName);
    if (Old->pRefElemName) free(Old->pRefElemName);
    free(Old);
  }
  free(StructRec);
}

/*!------------------------------------------------------------------------
 * \fn     CreateStructElem(const char *pElemName)
 * \brief  create a new entry for a structure definition
 * \param  pElemName name of the structure element
 * \return * to element or NULL if allocation failed
 * ------------------------------------------------------------------------ */

void ExpandStructStd(const char *pVarName, const struct sStructElem *pStructElem, LargeWord Base)
{
  HandleLabel(pVarName, Base + pStructElem->Offset);
}

PStructElem CreateStructElem(const char *pElemName)
{
  PStructElem pNeu;

  pNeu = (PStructElem) calloc(1, sizeof(TStructElem));
  if (pNeu)
  {
    pNeu->pElemName = as_strdup(pElemName);
    if (!CaseSensitive)
      NLS_UpString(pNeu->pElemName);
    pNeu->pRefElemName = NULL;
    pNeu->ExpandFnc = ExpandStructStd;
    pNeu->Offset = 0;
    pNeu->BitPos = pNeu->BitWidthM1 = -1;
    pNeu->OpSize = eSymbolSizeUnknown;
    pNeu->Next = NULL;
  }
  return pNeu;
}

/*!------------------------------------------------------------------------
 * \fn     AddStructElem(PStructRec pStructRec, PStructElem pElement)
 * \brief  add a new element to a structure definition
 * \param  pStructRec structure to extend
 * \param  pElement new element
 * ------------------------------------------------------------------------ */

void AddStructElem(PStructRec pStructRec, PStructElem pElement)
{
  PStructElem pRun;

  if (!CaseSensitive && pElement->pRefElemName)
    NLS_UpString(pElement->pRefElemName);

  if (!pStructRec->Elems)
    pStructRec->Elems = pElement;
  else
  {
    for (pRun = pStructRec->Elems; pRun->Next; pRun = pRun->Next);
    pRun->Next = pElement;
  }
  BumpStructLength(pStructRec, pElement->Offset);
}

/*!------------------------------------------------------------------------
 * \fn     SetStructElemSize(PStructRec pStructRec, const char *pElemName, ShortInt Size)
 * \brief  set the operand size of a structure's element
 * \param  pStructRec structure the element is contained in
 * \param  pElemName element's name
 * \param  Sizeoperand size to set
 * ------------------------------------------------------------------------ */

void SetStructElemSize(PStructRec pStructRec, const char *pElemName, ShortInt Size)
{
  PStructElem pRun;
  int Match;

  for (pRun = pStructRec->Elems; pRun; pRun = pRun->Next)
  {
    Match = CaseSensitive ? strcmp(pElemName, pRun->pElemName) : strcasecmp(pElemName, pRun->pElemName);
    if (!Match)
    {
      pRun->OpSize = Size;
      break;
    }
  }
}

/*!------------------------------------------------------------------------
 * \fn     ResolveStructReferences(PStructRec pStructRec)
 * \brief  resolve referenced elements in structure
 * \param  pStructRec structure to work on
 * ------------------------------------------------------------------------ */

void ResolveStructReferences(PStructRec pStructRec)
{
  Boolean AllResolved, DidResolve;
  PStructElem pRun, pRef;

  /* iterate over list until all symbols resolved, or we failed to resolve at least one per pass */

  do
  {
    AllResolved = True;
    DidResolve = False;

    for (pRun = pStructRec->Elems; pRun; pRun = pRun->Next)
      if (pRun->pRefElemName)
      {
        /* Only resolve to elements that already have been resolved.
           That's why we may need more than one pass.  */

        for (pRef = pStructRec->Elems; pRef; pRef = pRef->Next)
        {
          if (!strcmp(pRun->pRefElemName, pRef->pElemName) && !pRef->pRefElemName)
          {
            pRun->Offset = pRef->Offset;
            if (pRun->OpSize == eSymbolSizeUnknown)
              pRun->OpSize = pRef->OpSize;
            free(pRun->pRefElemName);
            pRun->pRefElemName = NULL;
            DidResolve = True;
            break;
          }
        }
        if (!pRef)
          AllResolved = False;
      }
  }
  while (!AllResolved && DidResolve);
  if (!AllResolved)
  {
    String Str;

    for (pRun = pStructRec->Elems; pRun; pRun = pRun->Next)
      if (pRun->pRefElemName)
      {
        strmaxcpy(Str, pRun->pElemName, STRINGSIZE);
        strmaxcat(Str, " -> ", STRINGSIZE);
        strmaxcat(Str, pRun->pRefElemName, STRINGSIZE);
        WrXError(ErrNum_UnresolvedStructRef, Str);
      }
  }
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
      WrXError(ErrNum_DoubleStruct, Neu->Name);
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
    Node->Tree.Name = as_strdup(Name);
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
  char NumStr[30], NumStr2[30];
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
    sprintf(s, "%3" PRILongInt, Elem->Offset);
    if (Elem->BitPos >= 0)
    {
      if (Elem->BitWidthM1 >= 0)
        sprintf(NumStr, ".%d-%d", Elem->BitPos, Elem->BitPos + Elem->BitWidthM1);
      else
        sprintf(NumStr, ".%d", Elem->BitPos);
    }
    else
      *NumStr = '\0';
    sprintf(NumStr2, "%-6s", NumStr);
    strmaxcat(s, NumStr2, STRINGSIZE);
    if (Elem->OpSize != eSymbolSizeUnknown)
    {
      sprintf(NumStr, "(%d)", Elem->OpSize);
      strmaxcat(s, NumStr, STRINGSIZE);
    }
    else
      strmaxcat(s, "   ", STRINGSIZE);
    strmaxcat(s, " ", STRINGSIZE);
    strmaxcat(s, Elem->pElemName, STRINGSIZE);
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
      strmaxcpy(pVarPrefix + VarLen + 1, StructElem->pElemName, RemVarLen);
      StructElem->ExpandFnc(pVarPrefix, StructElem, Base);
      if (StructElem->IsStruct)
      {
        TStructRec *pSubStruct;
        Boolean Found;

        strmaxcpy(pStructPrefix + StructLen + 1, StructElem->pElemName, RemStructLen);
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
  if (!LabPart.Str[0]) WrError(ErrNum_StructNameMissing);
  else
  {
    String CompVarName, CompStructName;

    strmaxcpy(CompVarName, LabPart.Str, sizeof(CompVarName));
    strmaxcpy(CompStructName, LOpPart, sizeof(CompStructName));
    ExpandStruct_One(StructRec, LabPart.Str, LOpPart, EProgCounter());
    CodeLen = StructRec->TotLen;
    BookKeeping();
    DontPrint = True;
  }
}

void asmstruct_init(void)
{
   StructRoot = NULL;
}
