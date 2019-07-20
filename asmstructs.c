/* asmstructs.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* structure handling                                                        */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "nls.h"
#include "nlmessages.h"
#include "strutil.h"

#include "trees.h"
#include "errmsg.h"
#include "symbolsize.h"

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

void ExpandStructStd(const tStrComp *pVarName, const struct sStructElem *pStructElem, LargeWord Base)
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
    tStrComp TmpComp;

    sprintf(tmp2, "%s%c", pInnermostNamedStruct->Name, pInnermostNamedStruct->StructRec->ExtChar);
    strmaxcpy(tmp, pName, sizeof(tmp));
    strmaxprep(tmp, tmp2, sizeof(tmp));
    StrCompMkTemp(&TmpComp, tmp);
    EnterIntSymbol(&TmpComp, Value, SegNone, False);
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

  strmaxcpy(Part, pName, STRINGSIZE);
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
  strmaxcpy(s, Node->Tree.Name, STRINGSIZE);
  if (Node->Tree.Attribute != -1)
  {
    strmaxcat(s, "[", STRINGSIZE);
    strmaxcat(s, GetSectionName(Node->Tree.Attribute), STRINGSIZE);
    strmaxcat(s, "]", STRINGSIZE);
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
      sprintf(NumStr, "(%s)", GetSymbolSizeName(Elem->OpSize));
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
  tStrComp TmpComp;

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
      StrCompMkTemp(&TmpComp, pVarPrefix);
      StructElem->ExpandFnc(&TmpComp, StructElem, Base);
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
    strmaxcpy(CompStructName, pLOpPart, sizeof(CompStructName));
    ExpandStruct_One(StructRec, LabPart.Str, pLOpPart, EProgCounter());
    CodeLen = StructRec->TotLen;
    BookKeeping();
    DontPrint = True;
  }
}

void asmstruct_init(void)
{
   StructRoot = NULL;
}
