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

#define t_toupper(ch) ((CaseSensitive) ? (ch) : (toupper(ch)))

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
       while ((p<strlen(Line)) AND (Line[p]!='\'') AND (Line[p]!='"')) p++;
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
	  struct _TMacroNode *Left,*Right; /* Soehne im Baum */
          ShortInt Balance;
	  LongInt DefSection;              /* Gueltigkeitssektion */
	  Boolean Defined;
	  PMacroRec Contents;
         } TMacroNode,*PMacroNode;

static PMacroNode MacroRoot;

        static Boolean AddMacro_AddNode(PMacroNode *Node, PMacroRec Neu, 
                                        LongInt DefSect, Boolean Protest)
BEGIN
   Boolean Grown;
   PMacroNode p1,p2;
   Boolean Result;
   int SErg;

   ChkStack();


   if (*Node==Nil)
    BEGIN
     *Node=(PMacroNode) malloc(sizeof(TMacroNode));
     (*Node)->Left=Nil; (*Node)->Right=Nil; 
     (*Node)->Balance=0; (*Node)->Defined=True;
     (*Node)->DefSection=DefSect; (*Node)->Contents=Neu;
     return True;
    END
   else Result=False;

   SErg=StrCmp(Neu->Name,(*Node)->Contents->Name,DefSect,(*Node)->DefSection);
   if (SErg>0)
    BEGIN
     Grown=AddMacro_AddNode(&((*Node)->Right),Neu,DefSect,Protest);
     if ((BalanceTree) AND (Grown))
      switch ((*Node)->Balance)
       BEGIN
        case -1:
         (*Node)->Balance=0;
         break;
        case 0:
         (*Node)->Balance=1; Result=True;
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
   else if (SErg<0)
    BEGIN
     Grown=AddMacro_AddNode(&((*Node)->Left),Neu,DefSect,Protest);
     if ((BalanceTree) AND (Grown))
      switch ((*Node)->Balance)
       BEGIN
        case 1:
         (*Node)->Balance=0;
         break;
        case 0:
         (*Node)->Balance=(-1); Result=True; 
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
     if ((*Node)->Defined)
      if (Protest) WrXError(1815,Neu->Name);
      else
       BEGIN
        ClearMacroRec(&((*Node)->Contents)); (*Node)->Contents=Neu;
        (*Node)->DefSection=DefSect;
       END
     else 
      BEGIN
       ClearMacroRec(&((*Node)->Contents)); (*Node)->Contents=Neu;
       (*Node)->DefSection=DefSect; (*Node)->Defined=True;
      END
    END

   return Result;
END

	void AddMacro(PMacroRec Neu, LongInt DefSect, Boolean Protest)
BEGIN
   if (NOT CaseSensitive) NLS_UpString(Neu->Name);
   AddMacro_AddNode(&MacroRoot,Neu,DefSect,Protest);
END

	static Boolean FoundMacro_FNode(LongInt Handle,PMacroRec *Erg, char *Part)
BEGIN
   PMacroNode Lauf;
   int CErg;

   Lauf=MacroRoot; CErg=2;
   while ((Lauf!=Nil) AND (CErg!=0))
    BEGIN
     if ((CErg=StrCmp(Part,Lauf->Contents->Name,Handle,Lauf->DefSection))<0) Lauf=Lauf->Left;
     else if (CErg>0) Lauf=Lauf->Right;
    END
   if (Lauf!=Nil) *Erg=Lauf->Contents;
   return (Lauf!=Nil);
END

	Boolean FoundMacro(PMacroRec *Erg)
BEGIN
   PSaveSection Lauf;
   String Part;

   strmaxcpy(Part,LOpPart,255); if (NOT CaseSensitive) NLS_UpString(Part);

   if (FoundMacro_FNode(MomSectionHandle,Erg,Part)) return True;
   Lauf=SectionStack;
   while (Lauf!=Nil)
    BEGIN
     if (FoundMacro_FNode(Lauf->Handle,Erg,Part)) return True;
     Lauf=Lauf->Next;
    END
   return False;
END

	static void ClearMacroList_ClearNode(PMacroNode *Node)
BEGIN
   ChkStack();

   if (*Node==Nil) return;

   ClearMacroList_ClearNode(&((*Node)->Left)); 
   ClearMacroList_ClearNode(&((*Node)->Right));

   ClearMacroRec(&((*Node)->Contents)); free(*Node); *Node=Nil;
END

	void ClearMacroList(void)
BEGIN
   ClearMacroList_ClearNode(&MacroRoot);
END

	static void ResetMacroDefines_ResetNode(PMacroNode Node)
BEGIN
   ChkStack();

   if (Node==Nil) return;

   ResetMacroDefines_ResetNode(Node->Left); 
   ResetMacroDefines_ResetNode(Node->Right);
   Node->Defined=False;
END

	void ResetMacroDefines(void)
BEGIN
   ResetMacroDefines_ResetNode(MacroRoot);
END

	void ClearMacroRec(PMacroRec *Alt)
BEGIN
   free((*Alt)->Name);
   ClearStringList(&((*Alt)->FirstLine));
   free(*Alt); *Alt=Nil;
END

        static void PrintMacroList_PNode(PMacroNode Node, LongInt *Sum, Boolean *cnt, char *OneS)
BEGIN
   String h;

   strmaxcpy(h,Node->Contents->Name,255);
   if (Node->DefSection!=-1)
    BEGIN
     strmaxcat(h,"[",255);
     strmaxcat(h,GetSectionName(Node->DefSection),255);
     strmaxcat(h,"]",255);
    END
   strmaxcat(OneS,h,255);
   if (strlen(h)<37) strmaxcat(OneS,Blanks(37-strlen(h)),255);
   if (NOT (*cnt)) strmaxcat(OneS," | ",255);
   else
    BEGIN
     WrLstLine(OneS); OneS[0]='\0';
    END
   *cnt=NOT (*cnt); (*Sum)++;
END

	static void PrintMacroList_PrintNode(PMacroNode Node, LongInt *Sum, Boolean *cnt, char *OneS)
BEGIN
   if (Node==Nil) return;
   ChkStack();

   PrintMacroList_PrintNode(Node->Left,Sum,cnt,OneS);

   PrintMacroList_PNode(Node,Sum,cnt,OneS);

   PrintMacroList_PrintNode(Node->Right,Sum,cnt,OneS);
END

	void PrintMacroList(void)
BEGIN
   String OneS;
   Boolean cnt;
   LongInt Sum;

   if (MacroRoot==Nil) return;

   NewPage(ChapDepth,True);
   WrLstLine(getmessage(Num_ListMacListHead1));
   WrLstLine(getmessage(Num_ListMacListHead2));
   WrLstLine("");

   OneS[0]='\0'; cnt=False; Sum=0; 
   PrintMacroList_PrintNode(MacroRoot,&Sum,&cnt,OneS);
   if (cnt)
    BEGIN
     OneS[strlen(OneS)-1]='\0';
     WrLstLine(OneS);
    END
   WrLstLine("");
   sprintf(OneS,"%7d",Sum);
   strmaxcat(OneS,getmessage((Sum==1)?Num_ListMacSumMsg:Num_ListMacSumsMsg),255);
   WrLstLine(OneS);
   WrLstLine("");
END

/*=== Eingabefilter Makroprozessor ========================================*/


	void asmmac_init(void)
BEGIN
   MacroRoot=Nil;
END
