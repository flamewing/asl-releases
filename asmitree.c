/* asmitree.c */
/*****************************************************************************/
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

        Boolean SearchInstTree(PInstTreeNode Root, char *OpPart)
BEGIN
   int z;

   z=0;
   while ((Root!=Nil) AND (strcmp(Root->Name,OpPart)!=0))
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

	static void PNode(PInstTreeNode Node, Word Lev)
BEGIN
   ChkStack();
   if (Node!=Nil)
    BEGIN
     PNode(Node->Left,Lev+1);
     printf("%*s %s %p %p %d\n",5*Lev,"",Node->Name,Node->Left,Node->Right,Node->Balance);
     PNode(Node->Right,Lev+1);
    END
END

	void PrintInstTree(PInstTreeNode Root)
BEGIN
   PNode(Root,0);
END

/*----------------------------------------------------------------------------*/

    static int GetKey(char *Name, LongWord TableSize)
BEGIN
   register unsigned char *p;
   LongWord tmp=0;

   for (p=(unsigned char *)Name; *p!='\0'; p++) tmp=(tmp<<2)+((LongWord)*p);
   return tmp%TableSize;
END

	PInstTable CreateInstTable(int TableSize)
BEGIN
   int z;
   PInstTableEntry tmp;
   PInstTable tab;
   
   tmp=(PInstTableEntry) malloc(sizeof(TInstTableEntry)*TableSize);
   for (z=0; z<TableSize; z++) tmp[z].Name=Nil;
   tab=(PInstTable) malloc(sizeof(TInstTable));
   tab->Fill=0; tab->Size=TableSize; tab->Entries=tmp; tab->Dynamic=FALSE;
   return tab;
END

	void SetDynamicInstTable(PInstTable Table)
BEGIN
   Table->Dynamic=TRUE;
END

	void DestroyInstTable(PInstTable tab)
BEGIN
   int z;

   if (tab->Dynamic)
    for (z=0; z<tab->Size; z++)
     free(tab->Entries[z].Name);

   free(tab->Entries);
   free(tab);
END

	void AddInstTable(PInstTable tab, char *Name, Word Index, InstProc Proc)
BEGIN
   LongWord h0=GetKey(Name,tab->Size),z=0;

   /* mindestens ein freies Element lassen, damit der Sucher garantiert terminiert */
 
   if (tab->Size-1<=tab->Fill) exit(255);
   while (1)
    BEGIN
     if (tab->Entries[h0].Name==Nil)
      BEGIN
       tab->Entries[h0].Name=(tab->Dynamic) ? strdup(Name) : Name;
       tab->Entries[h0].Proc=Proc;
       tab->Entries[h0].Index=Index;
       tab->Entries[h0].Coll=z;
       tab->Fill++;
       return;
      END
     z++;
     if ((LongInt)(++h0)==tab->Size) h0=0;
    END
END

	void RemoveInstTable(PInstTable tab, char *Name)
BEGIN
   LongWord h0=GetKey(Name,tab->Size);

   while (1)
    BEGIN
     if (tab->Entries[h0].Name==Nil) return;
     else if (strcmp(tab->Entries[h0].Name,Name)==0)
      BEGIN
       tab->Entries[h0].Name=Nil;
       tab->Entries[h0].Proc=Nil;
       tab->Fill--;
       return;
      END
     if ((LongInt)(++h0)==tab->Size) h0=0;
    END
END

	Boolean LookupInstTable(PInstTable tab, char *Name)
BEGIN
   LongWord h0=GetKey(Name,tab->Size);

   while (1)
    BEGIN
     if (tab->Entries[h0].Name==Nil) return False;
     else if (strcmp(tab->Entries[h0].Name,Name)==0)
      BEGIN
       tab->Entries[h0].Proc(tab->Entries[h0].Index);
       return True;
      END
     if ((LongInt)(++h0)==tab->Size) h0=0;
    END
END

	void PrintInstTable(FILE *stream, PInstTable tab)
BEGIN
   int z;

   for (z=0; z<tab->Size; z++)
    if (tab->Entries[z].Name!=Nil)
     fprintf(stream,"[%3d]: %-10s Index %4d Coll %2d\n",z,tab->Entries[z].Name,tab->Entries[z].Index,tab->Entries[z].Coll);
END

/*----------------------------------------------------------------------------*/ 

	void asmitree_init(void)
BEGIN
END
