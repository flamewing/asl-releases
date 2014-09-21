/* asmmac.c  */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen des Makroprozessors                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*            1. 7.1998 Korrektur Boyer-Moore-Algorithmus, wenn Ungleichheit */
/*                      nicht direkt am Ende                                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "nlmessages.h"
#include "as.rsc"
#include "stringlists.h"
#include "strutil.h"
#include "chunks.h"
#include "trees.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmif.h"

#include "asmmac.h"


PInputTag FirstInputTag;
POutputTag FirstOutputTag;

/*=== Praeprozessor =======================================================*/

/*-------------------------------------------------------------------------*/
/* Verwaltung define-Symbole */

	static void FreeDefine(PDefinement P)
BEGIN
   free(P->TransFrom);
   free(P->TransTo);
   free(P);
END

	static void EnterDefine(char *Name, char *Definition)
BEGIN
   PDefinement Neu;
   int z,l;

   if (NOT ChkSymbName(Name))
    BEGIN
     WrXError(1020,Name); return;
    END;

   Neu=FirstDefine;
   while (Neu!=Nil)
    BEGIN
     if (strcmp(Neu->TransFrom,Name)==0)
      BEGIN
       if (PassNo==1) WrXError(1000,Name); return;
      END;
     Neu=Neu->Next;
    END

   Neu=(PDefinement) malloc(sizeof(TDefinement));
   Neu->Next=FirstDefine;
   Neu->TransFrom=strdup(Name); if (NOT CaseSensitive) NLS_UpString(Neu->TransFrom);
   Neu->TransTo=strdup(Definition);
   l=strlen(Name);
   for (z=0; z<256; Neu->Compiled[z++]=l);
   for (z=0; z<l-1; z++) Neu->Compiled[(unsigned int)Neu->TransFrom[z]]=l-(z+1);
   FirstDefine=Neu;
END

	static void RemoveDefine(char *Name_O)
BEGIN
   PDefinement Lauf,Del;
   String Name;

   strmaxcpy(Name,Name_O,255); 
   if (NOT CaseSensitive) NLS_UpString(Name);

   Del=Nil;

   if (FirstDefine!=Nil)
    BEGIN
     if (strcmp(FirstDefine->TransFrom,Name)==0)
      BEGIN
       Del=FirstDefine; FirstDefine=FirstDefine->Next;
      END
     else
      BEGIN
       Lauf=FirstDefine;
       while ((Lauf->Next!=Nil) AND (strcmp(Lauf->Next->TransFrom,Name)!=0))
        Lauf=Lauf->Next;
       if (Lauf->Next!=Nil)
        BEGIN
 	Del=Lauf->Next; Lauf->Next=Del->Next;
        END
      END
     END

    if (Del==Nil) WrXError(1010,Name);
    else FreeDefine(Del);
END

	void PrintDefineList(void)
BEGIN
   PDefinement Lauf;
   String OneS;

   if (FirstDefine==Nil) return;

   NewPage(ChapDepth,True);
   WrLstLine(getmessage(Num_ListDefListHead1));
   WrLstLine(getmessage(Num_ListDefListHead2));
   WrLstLine("");

   Lauf=FirstDefine;
   while (Lauf!=Nil)
    BEGIN
     strmaxcpy(OneS,Lauf->TransFrom,255);
     strmaxcat(OneS,Blanks(10-(strlen(Lauf->TransFrom)%10)),255);
     strmaxcat(OneS," = ",255);
     strmaxcat(OneS,Lauf->TransTo,255);
     WrLstLine(OneS);
     Lauf=Lauf->Next;
    END
   WrLstLine("");
END

	void ClearDefineList(void)
BEGIN
   PDefinement Temp;

   while (FirstDefine!=Nil)
    BEGIN
     Temp=FirstDefine; FirstDefine=FirstDefine->Next;
     FreeDefine(Temp);
    END
END

/*------------------------------------------------------------------------*/
/* Interface */

	void Preprocess(void)
BEGIN
   String h,Cmd,Arg;
   char *p;

   p=strchr(OneLine,'#')+1;
   strmaxcpy(h,p,255);
   p=FirstBlank(h);
   if (p==Nil)
    BEGIN
     strmaxcpy(Cmd,h,255); *h='\0';
    END
   else SplitString(h,Cmd,h,p);

   KillPrefBlanks(h); KillPostBlanks(h);

   if (NOT IfAsm) return;

   if (strcasecmp(Cmd,"DEFINE")==0)
    BEGIN
     p=FirstBlank(h);
     if (p!=Nil)
      BEGIN
       SplitString(h,Arg,h,p); KillPrefBlanks(h);
       EnterDefine(Arg,h);
      END
    END
   else if (strcasecmp(Cmd,"UNDEF")==0) RemoveDefine(h);

   CodeLen=0;
END

	static Boolean ExpandDefines_NErl(char inp)
BEGIN
   return (((inp>='0') AND (inp<='9')) OR ((inp>='A') AND (inp<='Z')) OR ((inp>='a') AND (inp<='z')));
END

#define t_toupper(ch) ((CaseSensitive) ? (ch) : (mytoupper(ch)))

	void ExpandDefines(char *Line)
BEGIN
   PDefinement Lauf;
   sint LPos,Diff,p,p2,p3,z,z2,FromLen,ToLen,LineLen;
   
   Lauf=FirstDefine;
   while (Lauf!=Nil)
    BEGIN
     LPos=0; FromLen=strlen(Lauf->TransFrom); ToLen=strlen(Lauf->TransTo);
     Diff=ToLen-FromLen;
     do
      BEGIN
       /* Stelle, ab der verbatim, suchen -->p */
       p=LPos;
       while ((p<(int)strlen(Line)) AND (Line[p]!='\'') AND (Line[p]!='"')) p++;
       /* nach Quellstring suchen, ersetzen, bis keine Treffer mehr */
       p2=LPos;
       do
        BEGIN
         z2=0; 
         while ((z2>=0) AND (p2<=p-FromLen))
          BEGIN
           z2=FromLen-1; z=p2+z2;
           while ((z2>=0) AND (t_toupper(Line[z])==Lauf->TransFrom[z2]))
            BEGIN
             z2--; z--;
            END
           if (z2>=0) p2+=Lauf->Compiled[(unsigned int)t_toupper(Line[p2+FromLen-1])];
          END
         if (z2==-1)
          BEGIN
           if (((p2==0) OR (NOT ExpandDefines_NErl(Line[p2-1])))
           AND ((p2+FromLen==p) OR (NOT ExpandDefines_NErl(Line[p2+FromLen]))))
            BEGIN
             if (Diff!=0)
              memmove(Line+p2+ToLen,Line+p2+FromLen,strlen(Line)-p2-FromLen+1);
             memcpy(Line+p2,Lauf->TransTo,ToLen);
             p+=Diff; /* !!! */
             p2+=ToLen;
            END
           else p2+=FromLen;
          END
        END
       while (z2==-1);
       /* Endposition verbatim suchen */
       p3=p+1; LineLen=strlen(Line);
       while ((p3<LineLen) AND (Line[p3]!=Line[p])) p3++;
       /* Zaehler entsprechend herauf */
       LPos=p3+1;
      END
     while (LPos<=LineLen-FromLen);
     Lauf=Lauf->Next;
    END
END

/*=== Makrolistenverwaltung ===============================================*/

typedef struct _TMacroNode
         {
          TTree Tree;
	  Boolean Defined;
	  PMacroRec Contents;
         } TMacroNode,*PMacroNode;

static PMacroNode MacroRoot;

static Boolean MacroAdder(PTree *PDest, PTree Neu, void *pData)
{
  PMacroNode NewNode = (PMacroNode) Neu, *Node;
  Boolean Protest = *((Boolean*)pData), Result = False;

  if (!PDest)
  {
    NewNode->Defined = TRUE; 
    return True;
  }

  Node = (PMacroNode*) PDest;
  if ((*Node)->Defined)
  {
    if (Protest) WrXError(1815,Neu->Name);
    else
    {
      ClearMacroRec(&((*Node)->Contents), TRUE); (*Node)->Contents = NewNode->Contents;
    }
  }
  else
  {
    ClearMacroRec(&((*Node)->Contents), TRUE); (*Node)->Contents = NewNode->Contents;
    (*Node)->Defined = True;
    return True;
  }
  return Result;
}

void AddMacro(PMacroRec Neu, LongInt DefSect, Boolean Protest)
{
   PMacroNode NewNode;
   PTree TreeRoot;

   if (NOT CaseSensitive) NLS_UpString(Neu->Name);
   NewNode = (PMacroNode) malloc(sizeof(TMacroNode));
   NewNode->Tree.Left = Nil; NewNode->Tree.Right = Nil;
   NewNode->Tree.Name = strdup(Neu->Name);
   NewNode->Tree.Attribute = DefSect;
   NewNode->Contents = Neu;
   
   TreeRoot = &(MacroRoot->Tree);
   EnterTree(&TreeRoot, &(NewNode->Tree), MacroAdder, &Protest);
   MacroRoot = (PMacroNode)TreeRoot;
}

static PMacroRec FoundMacro_FNode(LongInt Handle, char *Part)
{
   PMacroNode Lauf;
   PMacroRec Result = NULL;

   Lauf = (PMacroNode) SearchTree((PTree)MacroRoot, Part, Handle);
   if (Lauf != Nil) Result = Lauf->Contents;
   return Result;
}

	Boolean FoundMacro(PMacroRec *Erg)
BEGIN
   PSaveSection Lauf;
   String Part;

   strmaxcpy(Part,LOpPart,255); if (NOT CaseSensitive) NLS_UpString(Part);

   if ((*Erg = FoundMacro_FNode(MomSectionHandle, Part))) return True;
   Lauf = SectionStack;
   while (Lauf != Nil)
   {
     if ((*Erg = FoundMacro_FNode(Lauf->Handle, Part))) return True;
     Lauf = Lauf->Next;
   }
   return False;
END

	static void ClearMacroList_ClearNode(PTree Tree, void *pData)
{
   PMacroNode Node = (PMacroNode) Tree;
   UNUSED(pData);

   ClearMacroRec(&(Node->Contents), TRUE);
}

void ClearMacroList(void)
{
   PTree TreeRoot;

   TreeRoot = &(MacroRoot->Tree); MacroRoot = NULL;
   DestroyTree(&TreeRoot, ClearMacroList_ClearNode, NULL);
}

static void ResetMacroDefines_ResetNode(PTree Tree, void *pData)
{
   PMacroNode Node = (PMacroNode)Tree;
   UNUSED(pData);

   Node->Defined = False;
}

void ResetMacroDefines(void)
{
   IterTree((PTree)MacroRoot, ResetMacroDefines_ResetNode, NULL);
}

void ClearMacroRec(PMacroRec *Alt, Boolean Complete)
{
   if ((*Alt)->Name)
   {
     free((*Alt)->Name); (*Alt)->Name = NULL;
   }
   ClearStringList(&((*Alt)->FirstLine));
   ClearStringList(&((*Alt)->ParamNames));
   ClearStringList(&((*Alt)->ParamDefVals));

   if (Complete)
   {
     free(*Alt); *Alt = Nil;
   }
END

typedef struct
        {
          LongInt Sum;
          Boolean cnt;
          String OneS;
        } TMacroListContext;

        static void PrintMacroList_PNode(PTree Tree, void *pData)
BEGIN
   PMacroNode Node = (PMacroNode)Tree;
   TMacroListContext *pContext = (TMacroListContext*) pData;
   String h;

   strmaxcpy(h,Node->Contents->Name,255);
   if (Node->Tree.Attribute!=-1)
    BEGIN
     strmaxcat(h,"[",255);
     strmaxcat(h,GetSectionName(Node->Tree.Attribute),255);
     strmaxcat(h,"]",255);
    END
   strmaxcat(pContext->OneS, h, 255);
   if (strlen(h) < 37) strmaxcat(pContext->OneS, Blanks(37 - strlen(h)), 255);
   if (NOT (pContext->cnt)) strmaxcat(pContext->OneS, " | ", 255);
   else
    BEGIN
     WrLstLine(pContext->OneS); pContext->OneS[0]='\0';
    END
   pContext->cnt = NOT pContext->cnt;
   pContext->Sum++;
END

void PrintMacroList(void)
{
   TMacroListContext Context;

   if (MacroRoot==Nil) return;

   NewPage(ChapDepth, True);
   WrLstLine(getmessage(Num_ListMacListHead1));
   WrLstLine(getmessage(Num_ListMacListHead2));
   WrLstLine("");

   Context.OneS[0] = '\0'; Context.cnt = False; Context.Sum = 0; 
   IterTree((PTree)MacroRoot, PrintMacroList_PNode, &Context);
   if (Context.cnt)
   {
     Context.OneS[strlen(Context.OneS) - 1] = '\0';
     WrLstLine(Context.OneS);
   }
   WrLstLine("");
   sprintf(Context.OneS, "%7lu%s",
           (unsigned long)Context.Sum,
           getmessage((Context.Sum == 1) ? Num_ListMacSumMsg : Num_ListMacSumsMsg));
   WrLstLine(Context.OneS);
   WrLstLine("");
}

/*=== Eingabefilter Makroprozessor ========================================*/


	void asmmac_init(void)
BEGIN
   MacroRoot=Nil;
END
