/* insttree.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Opcode-Abfrage als Binaerbaum                                             */
/*                                                                           */
/* Historie: 30.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef void (*InstProc)(Word Index);
typedef struct _TInstTreeNode
         { 
          struct _TInstTreeNode *Left,*Right;
          InstProc Proc;
          char *Name;
          Word Index;
          ShortInt Balance;
         } TInstTreeNode,*PInstTreeNode;

extern void AddInstTree(PInstTreeNode *Root, char *NName, InstProc NProc, Word NIndex);

extern void ClearInstTree(PInstTreeNode *Root);

extern Boolean SearchInstTree(PInstTreeNode Root);

extern void PrintInstTree(PInstTreeNode Root);

extern void insttree_init(void);
