/* asmitree.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Opcode-Abfrage als Binaerbaum                                             */
/*                                                                           */
/* Historie: 30.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef void (*InstProc)(
#ifdef __PROTOS__
Word Index
#endif
);
typedef struct _TInstTreeNode
         { 
          struct _TInstTreeNode *Left,*Right;
          InstProc Proc;
          char *Name;
          Word Index;
          ShortInt Balance;
         } TInstTreeNode,*PInstTreeNode;

typedef struct _TInstTableEntry
         {
          InstProc Proc;
          char *Name;
          Word Index;
          int Coll;
         }
        TInstTableEntry,*PInstTableEntry;

typedef struct
         {
          int Fill,Size;
          PInstTableEntry Entries;
         } TInstTable,*PInstTable;

extern void AddInstTree(PInstTreeNode *Root, char *NName, InstProc NProc, Word NIndex);

extern void ClearInstTree(PInstTreeNode *Root);

extern Boolean SearchInstTree(PInstTreeNode Root, char *OpPart);

extern void PrintInstTree(PInstTreeNode Root);


extern PInstTable CreateInstTable(int TableSize);

extern void DestroyInstTable(PInstTable tab);

extern void AddInstTable(PInstTable tab, char *Name, Word Index, InstProc Proc);

extern void RemoveInstTable(PInstTable tab, char *Name);

extern Boolean LookupInstTable(PInstTable tab, char *Name);

extern void PrintInstTable(FILE *stream, PInstTable tab);

extern void asmitree_init(void);
