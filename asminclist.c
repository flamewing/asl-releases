/* asminclist.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Include-Verschachtelungsliste                              */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "strutil.h"
#include "chunks.h"
#include "nls.h"
#include "nlmessages.h"
#include "as.rsc"
#include "asmfnums.h"
#include "asmdef.h"
#include "asmsub.h"

#include "asminclist.h"


typedef void **PFileArray;
typedef struct _TFileNode
         {
          Integer Name;
          Integer Len;
          struct _TFileNode *Parent;
          PFileArray Subs;
         } TFileNode,*PFileNode;

static PFileNode Root,Curr;


  	void PushInclude(char *S)
BEGIN
   PFileNode Neu;

   Neu=(PFileNode) malloc(sizeof(TFileNode));
   Neu->Name=GetFileNum(S);
   Neu->Len=0; Neu->Subs=Nil;
   Neu->Parent=Curr;
   if (Root==Nil) Root=Neu;
   if (Curr==Nil) Curr=Neu;
   else
    BEGIN
     if (Curr->Len==0)
      Curr->Subs=(PFileArray) malloc(sizeof(void *));
     else
      Curr->Subs=(PFileArray) realloc(Curr->Subs,sizeof(void *)*(Curr->Len+1));
     Curr->Subs[Curr->Len++]=(void *)Neu;
     Curr=Neu;
    END
END


	void PopInclude(void)
BEGIN
   if (Curr!=Nil) Curr=Curr->Parent;
END


        static void PrintIncludeList_PrintNode(PFileNode Node, int Indent)
BEGIN
   int z;
   String h;

   ChkStack();

   if (Node!=Nil)
    BEGIN
     strmaxcpy(h,Blanks(Indent),255);
     strmaxcat(h,GetFileName(Node->Name),255);
     WrLstLine(h);
     for (z=0; z<Node->Len; z++) PrintIncludeList_PrintNode(Node->Subs[z],Indent+5);
    END
END

	void PrintIncludeList(void)
BEGIN
   NewPage(ChapDepth,True);
   WrLstLine(getmessage(Num_ListIncludeListHead1));
   WrLstLine(getmessage(Num_ListIncludeListHead2));
   WrLstLine("");
   PrintIncludeList_PrintNode(Root,0);
END


        static void ClearIncludeList_ClearNode(PFileNode Node)
BEGIN
   int z; 

   ChkStack();

   if (Node!=Nil)
    BEGIN
     for (z=0; z<Node->Len; ClearIncludeList_ClearNode(Node->Subs[z++]));
     if (Node->Len>0) free(Node->Subs);
     free(Node);
    END
END

	void ClearIncludeList(void)
BEGIN
   ClearIncludeList_ClearNode(Root);
   Curr=Nil; Root=Nil;
END


	void asminclist_init(void)
BEGIN
  Root=Nil; Curr=Nil;
END
