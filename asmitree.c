/* asmitree.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Opcode-Abfrage als Binaerbaum                                             */
/*                                                                           */
/* Historie: 30.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "chunks.h"
#include "stringutil.h"
#include "asmdef.h"
#include "asmsub.h"

#include "asmitree.h"

/*---------------------------------------------------------------------------*/

        static Boolean AddSingle(PInstTreeNode *Node, char *NName, InstProc NProc, Word NIndex)
BEGIN
   PInstTreeNode p1,p2;
   Boolean Result=False;

   ChkStack();

   if (*Node==Nil)
    BEGIN
     *Node=(PInstTreeNode) malloc(sizeof(TInstTreeNode));
     (*Node)->Left=Nil; (*Node)->Right=Nil; 
     (*Node)->Proc=NProc; (*Node)->Index=NIndex;
     (*Node)->Balance=0;
     (*Node)->Name=strdup(NName);
     Result=True;
    END
   else if (strcmp(NName,(*Node)->Name)<0)
    BEGIN
     if (AddSingle(&((*Node)->Left),NName,NProc,NIndex))
      switch ((*Node)->Balance)
       BEGIN
        case 1:
         (*Node)->Balance=0;
         break;
        case 0:
         (*Node)->Balance=(-1);
         Result=True;
         break;
        case -1:
         p1=(*Node)->Left;
         if (p1->Balance==-1)
          BEGIN
           (*Node)->Left=p1->Right; p1->Right=(*Node);
           (*Node)->Balance=0; *Node=p1;
          END
         else
          BEGIN
           p2=p1->Right;
           p1->Right=p2->Left; p2->Left=p1;
           (*Node)->Left=p2->Right; p2->Right=(*Node);
           if (p2->Balance==-1) (*Node)->Balance=   1; else (*Node)->Balance=0;
           if (p2->Balance== 1) p1     ->Balance=(-1); else p1     ->Balance=0;
           *Node=p2;
          END
         (*Node)->Balance=0;
         break;
       END
    END
   else
    BEGIN
     if (AddSingle(&((*Node)->Right),NName,NProc,NIndex))
      switch ((*Node)->Balance)
       BEGIN
        case -1:
         (*Node)->Balance=0;
         break;
        case 0:
         (*Node)->Balance=1;
         Result=True;
         break;
        case 1:
         p1=(*Node)->Right;
         if (p1->Balance==1)
          BEGIN
           (*Node)->Right=p1->Left; p1->Left=(*Node);
           (*Node)->Balance=0; *Node=p1;
          END
         else
          BEGIN
           p2=p1->Left;
           p1->Left=p2->Right; p2->Right=p1;
           (*Node)->Right=p2->Left; p2->Left=(*Node);
           if (p2->Balance== 1) (*Node)->Balance=(-1); else (*Node)->Balance=0;
           if (p2->Balance==-1) p1     ->Balance=   1; else p1     ->Balance=0;
           *Node=p2;
          END
         (*Node)->Balance=0;
         break;
       END
    END
   return Result;
END

        void AddInstTree(PInstTreeNode *Root, char *NName, InstProc NProc, Word NIndex)
BEGIN
   AddSingle(Root,NName,NProc,NIndex);
END

        static void ClearSingle(PInstTreeNode *Node)
BEGIN
   ChkStack();

   if (*Node!=Nil)
    BEGIN
     free((*Node)->Name);
     ClearSingle(&((*Node)->Left));
     ClearSingle(&((*Node)->Right));
     free(*Node); *Node=Nil;
    END
END

        void ClearInstTree(PInstTreeNode *Root)
BEGIN
   ClearSingle(Root);
END

        Boolean SearchInstTree(PInstTreeNode Root)
BEGIN
   Integer z;

   z=0;
   while ((Root!=Nil) AND (NOT Memo(Root->Name)))
    BEGIN
     Root=(strcmp(OpPart,Root->Name)<0)? Root->Left : Root->Right;
     z++;
    END

   if (Root==Nil) return False;
   else
    BEGIN
     Root->Proc(Root->Index);
     return True;
    END
END

static char Format[20];

	static void PNode(PInstTreeNode Node, Word Lev)
BEGIN
   ChkStack();
   if (Node!=Nil)
    BEGIN
     PNode(Node->Left,Lev+1);
     sprintf(Format,"%%%ds %%s %%p %%p %%d\n",5*Lev);
     printf(Format,"",Node->Name,Node->Left,Node->Right,Node->Balance);
     PNode(Node->Right,Lev+1);
    END
END

	void PrintInstTree(PInstTreeNode Root)
BEGIN
   PNode(Root,0);
END

	void asmitree_init(void)
BEGIN
END
