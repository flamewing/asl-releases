#ifndef _TREES_H
#define _TREES_H
/* trees.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Tree management                                                           */
/*                                                                           */
/*****************************************************************************/
/* $Id: trees.h,v 1.1 2003/11/06 02:49:25 alfred Exp $                       */
/***************************************************************************** 
 * $Log: trees.h,v $
 * Revision 1.1  2003/11/06 02:49:25  alfred
 * - recreated
 *
 * Revision 1.1  2002/11/10 15:08:34  alfred
 * - use tree functions
 *
 *****************************************************************************/
           
typedef struct _TTree
        {
          struct _TTree *Left, *Right;
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
           
#endif /* _TREES_H */
