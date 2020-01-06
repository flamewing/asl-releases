/* asmitree.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Opcode-Abfrage als Binaerbaum                                             */
/*                                                                           */
/* Historie: 30.10.1996 Grundsteinlegung                                     */
/*            8.10.1997 Hash-Tabelle                                         */
/*            6.12.1998 dynamisches Kopieren der Namen                       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "chunks.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"

#include "asmitree.h"

/*---------------------------------------------------------------------------*/

static Boolean AddSingle(PInstTreeNode *Node, char *NName, InstProc NProc, Word NIndex)
{
  PInstTreeNode p1, p2;
  Boolean Result = False;

  ChkStack();

  if (!*Node)
  {
    *Node = (PInstTreeNode) malloc(sizeof(TInstTreeNode));
    (*Node)->Left = NULL;
    (*Node)->Right = NULL;
    (*Node)->Proc = NProc;
    (*Node)->Index = NIndex;
    (*Node)->Balance = 0;
    (*Node)->Name = as_strdup(NName);
    Result = True;
  }
  else if (strcmp(NName, (*Node)->Name) < 0)
  {
    if (AddSingle(&((*Node)->Left), NName, NProc, NIndex))
      switch ((*Node)->Balance)
      {
        case 1:
          (*Node)->Balance = 0;
          break;
        case 0:
          (*Node)->Balance = -1;
          Result = True;
          break;
        case -1:
          p1 = (*Node)->Left;
          if (p1->Balance == -1)
          {
            (*Node)->Left = p1->Right;
            p1->Right = (*Node);
            (*Node)->Balance = 0;
            *Node = p1;
          }
         else
         {
           p2 = p1->Right;
           p1->Right = p2->Left;
           p2->Left = p1;
           (*Node)->Left = p2->Right;
           p2->Right = (*Node);
           (*Node)->Balance = (p2->Balance == -1) ? 1 : 0;
           p1->Balance = (p2->Balance == 1) ? -1 : 0;
           *Node = p2;
         }
         (*Node)->Balance = 0;
         break;
     }
  }
  else
  {
    if (AddSingle(&((*Node)->Right), NName, NProc, NIndex))
      switch ((*Node)->Balance)
      {
        case -1:
          (*Node)->Balance = 0;
          break;
        case 0:
          (*Node)->Balance = 1;
          Result = True;
          break;
        case 1:
          p1 = (*Node)->Right;
          if (p1->Balance == 1)
          {
            (*Node)->Right = p1->Left;
            p1->Left = (*Node);
            (*Node)->Balance = 0;
            *Node = p1;
          }
          else
          {
            p2 = p1->Left;
            p1->Left = p2->Right;
            p2->Right = p1;
            (*Node)->Right = p2->Left;
            p2->Left = (*Node);
            (*Node)->Balance = (p2->Balance == 1) ? -1 : 0;
            p1->Balance = (p2->Balance == -1) ? 1 : 0;
            *Node = p2;
          }
          (*Node)->Balance = 0;
          break;
      }
  }
  return Result;
}

void AddInstTree(PInstTreeNode *Root, char *NName, InstProc NProc, Word NIndex)
{
  AddSingle(Root, NName, NProc, NIndex);
}

static void ClearSingle(PInstTreeNode *Node)
{
  ChkStack();

  if (*Node)
  {
    free((*Node)->Name);
    ClearSingle(&((*Node)->Left));
    ClearSingle(&((*Node)->Right));
    free(*Node);
    *Node = NULL;
  }
}

void ClearInstTree(PInstTreeNode *Root)
{
  ClearSingle(Root);
}

Boolean SearchInstTree(PInstTreeNode Root, char *OpPart)
{
  int z;

  z = 0;
  while ((Root) && (strcmp(Root->Name, OpPart)))
  {
    Root = (strcmp(OpPart, Root->Name) < 0) ? Root->Left : Root->Right;
    z++;
  }

  if (!Root)
    return False;
  else
  {
    Root->Proc(Root->Index);
    return True;
  }
}

static void PNode(PInstTreeNode Node, Word Lev)
{
  ChkStack();
  if (Node)
  {
    PNode(Node->Left, Lev + 1);
    printf("%*s %s %p %p %d\n", 5 * Lev, "", Node->Name, (void*)Node->Left, (void*)Node->Right, Node->Balance);
    PNode(Node->Right, Lev + 1);
  }
}

void PrintInstTree(PInstTreeNode Root)
{
  PNode(Root, 0);
}

/*----------------------------------------------------------------------------*/

static int GetKey(const char *Name, LongWord TableSize)
{
  register unsigned char *p;
  LongWord tmp = 0;

  for (p = (unsigned char *)Name; *p != '\0'; p++)
    tmp = (tmp << 2) + ((LongWord)*p);
  return tmp % TableSize;
}

PInstTable CreateInstTable(int TableSize)
{
  int z;
  PInstTableEntry tmp;
  PInstTable tab;

  tmp = (PInstTableEntry) malloc(sizeof(TInstTableEntry) * TableSize);
  for (z = 0; z < TableSize; z++)
    tmp[z].Name = NULL;
  tab = (PInstTable) malloc(sizeof(TInstTable));
  tab->Fill = 0;
  tab->Size = TableSize;
  tab->Entries = tmp;
  tab->Dynamic = FALSE;
  return tab;
}

void SetDynamicInstTable(PInstTable Table)
{
  Table->Dynamic = TRUE;
}

void DestroyInstTable(PInstTable tab)
{
  int z;

  if (tab->Dynamic)
    for (z = 0; z < tab->Size; z++)
      free(tab->Entries[z].Name);

  free(tab->Entries);
  free(tab);
}

void AddInstTable(PInstTable tab, const char *Name, Word Index, InstProc Proc)
{
  LongWord h0 = GetKey(Name, tab->Size), z = 0;

  /* mindestens ein freies Element lassen, damit der Sucher garantiert terminiert */

  if (tab->Size - 1 <= tab->Fill)
  {
    fprintf(stderr, "\nhash table overflow\n");
    exit(255);
  }
  while (1)
  {
    if (!tab->Entries[h0].Name)
    {
      tab->Entries[h0].Name = (tab->Dynamic) ? as_strdup(Name) : (char*)Name;
      tab->Entries[h0].Proc = Proc;
      tab->Entries[h0].Index = Index;
      tab->Entries[h0].Coll = z;
      tab->Fill++;
      return;
    }
    if (!strcmp(tab->Entries[h0].Name, Name))
    {
      printf("%s double in table\n", Name);
      exit(255);
    }
    z++;
    if ((LongInt)(++h0) == tab->Size)
      h0 = 0;
  }
}

void RemoveInstTable(PInstTable tab, const char *Name)
{
  LongWord h0 = GetKey(Name, tab->Size);

  while (1)
  {
    if (!tab->Entries[h0].Name)
      return;
    else if (!strcmp(tab->Entries[h0].Name, Name))
    {
      tab->Entries[h0].Name = NULL;
      tab->Entries[h0].Proc = NULL;
      tab->Fill--;
      return;
    }
    if ((LongInt)(++h0) == tab->Size)
      h0 = 0;
  }
}

Boolean LookupInstTable(PInstTable tab, const char *Name)
{
  LongWord h0 = GetKey(Name, tab->Size);

  while (1)
  {
    if (!tab->Entries[h0].Name)
      return False;
    else if (!strcmp(tab->Entries[h0].Name, Name))
    {
      tab->Entries[h0].Proc(tab->Entries[h0].Index);
      return True;
    }
    if ((LongInt)(++h0) == tab->Size)
      h0 = 0;
  }
}

void PrintInstTable(FILE *stream, PInstTable tab)
{
  int z;

  for (z = 0; z < tab->Size; z++)
    if (tab->Entries[z].Name)
      fprintf(stream, "[%3d]: %-10s Index %4d Coll %2d\n", z,
             tab->Entries[z].Name, tab->Entries[z].Index, tab->Entries[z].Coll);
}

/*----------------------------------------------------------------------------*/

void asmitree_init(void)
{
}
