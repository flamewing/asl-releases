/* asmstructs.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* structure handling                                                        */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmstructs.c,v 1.6 2002/11/20 20:25:04 alfred Exp $                 */
/*****************************************************************************
 * $Log: asmstructs.c,v $
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

typedef struct _TStructNode
        {
          TTree Tree;
          Boolean Defined;
          PStructRec StructRec;
        } TStructNode, *PStructNode;

/*****************************************************************************/

PStructStack StructStack;        /* momentan offene Strukturen */
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

void AddStructElem(PStructRec StructRec, char *Name, LongInt Offset)
{
  PStructElem Neu, Run;

  Neu = (PStructElem) malloc(sizeof(TStructElem));
  if (Neu)
  {
    Neu->Name = strdup(Name);
    if (NOT CaseSensitive) NLS_UpString(Neu->Name);
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
    if (Protest) WrXError(1815,Neu->Name);
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

  Node = (PStructNode) malloc(sizeof(TStructNode));
  if (Node)
  {
    Node->Tree.Left = Node->Tree.Right = NULL;
    Node->Tree.Name = strdup(Name);
    if (NOT CaseSensitive) NLS_UpString(Node->Tree.Name);
    Node->Tree.Attribute = MomSectionHandle;
    Node->Defined = FALSE;
    Node->StructRec = StructRec;
    EnterTree((PTree*)&StructRoot, &(Node->Tree), StructAdder, &Protest);
  }
}

static PStructRec FoundStruct_FNode(LongInt Handle, char *Name)
{
   PStructNode Lauf;

   Lauf = (PStructNode) SearchTree((PTree)StructRoot, Name, Handle);
   return Lauf ? Lauf->StructRec : NULL;
}

Boolean FoundStruct(PStructRec *Erg)
{
   PSaveSection Lauf;
   String Part;

   strmaxcpy(Part, LOpPart, 255);
   if (!CaseSensitive) NLS_UpString(Part);

   if ((*Erg = FoundStruct_FNode(MomSectionHandle, Part))) return True;
   Lauf = SectionStack;
   while (Lauf != Nil)
   {
     if ((*Erg = FoundStruct_FNode(Lauf->Handle, Part))) return True;
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
    sprintf(s, "%3d ", Elem->Offset);
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
  sprintf(s, "%d%s", Context.Sum, 
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
  DestroyTree((PTree*)&StructRoot, ClearNode, NULL);
}

void ExpandStruct(PStructRec StructRec)
{
  PStructElem StructElem;
  String CompName;
  int llen;
  char *dptr;

  if (!LabPart[0]) WrError(2040);
  else
  {
    strmaxcpy(CompName, LabPart, 254);
    llen = strlen(CompName);
    dptr = CompName + llen;
    *dptr++ = StructRec->ExtChar; llen++;
    llen = 254 - llen;
    for (StructElem = StructRec->Elems; StructElem;  StructElem = StructElem->Next)
    {
      strmaxcpy(dptr, StructElem->Name, llen);
      HandleLabel(CompName, EProgCounter() + StructElem->Offset);
    }
    CodeLen = StructRec->TotLen;
    DontPrint = True;
  }
}

void asmstruct_init(void)
{
   StructRoot = NULL;
}
