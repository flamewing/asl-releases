#ifndef TREES_H
#define TREES_H
/* trees.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Tree management                                                           */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

extern Boolean BalanceTrees;

typedef struct tag_TTree
{
  struct tag_TTree *Left, *Right;
  ShortInt Balance;
  char *Name;
  LongInt Attribute;
} TTree, *PTree;

typedef void (*TTreeCallback)(PTree Node, void *pData);

typedef Boolean (*TTreeAdder)(PTree *PDest, PTree Neu, void *pData);

extern void IterTree(PTree Tree, TTreeCallback Callback, void *pData);

extern void GetTreeDepth(PTree Tree, LongInt *pMin, LongInt *pMax);

extern void DestroyTree(PTree *Tree, TTreeCallback Callback, void *pData);

extern void DumpTree(PTree Tree);

extern PTree SearchTree(PTree Tree, char *Name, LongInt Attribute);

extern Boolean EnterTree(PTree *PDest, PTree Neu, TTreeAdder Adder, void *pData);

#endif /* TREES_H */
