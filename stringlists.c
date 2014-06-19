/* stringlists.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Implementation von String-Listen                                          */
/*                                                                           */
/* Historie:  5. 4.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include "strutil.h"
#include "stringlists.h"

	void InitStringList(StringList *List)
BEGIN
   *List=Nil;
END

        void ClearStringEntry(StringRecPtr *Elem)
BEGIN
   free((*Elem)->Content); free(*Elem); *Elem=Nil;
END

        void ClearStringList(StringList *List)
BEGIN
   StringRecPtr Hilf;

   while (*List!=Nil)
    BEGIN
     Hilf=(*List); *List=(*List)->Next;
     ClearStringEntry(&Hilf);
    END
END

        void AddStringListFirst(StringList *List, const char *NewStr)
BEGIN
   StringRecPtr Neu;

   Neu=(StringRecPtr) malloc(sizeof(StringRec));
   Neu->Content=strdup(NewStr); 
   Neu->Next=(*List);
   *List=Neu;
END

        void AddStringListLast(StringList *List, const char *NewStr)
BEGIN
   StringRecPtr Neu,Lauf;

   Neu=(StringRecPtr) malloc(sizeof(StringRec)); 
   Neu->Content=strdup(NewStr); Neu->Next=Nil;
   if (*List==Nil) *List=Neu;
   else
    BEGIN
     Lauf=(*List); while (Lauf->Next!=Nil) Lauf=Lauf->Next;
     Lauf->Next=Neu;
    END
END

        void RemoveStringList(StringList *List, const char *OldStr)
BEGIN
   StringRecPtr Lauf,Hilf;

   if (*List==Nil) return;
   if (strcmp((*List)->Content,OldStr)==0)
    BEGIN
     Hilf=(*List); *List=(*List)->Next; ClearStringEntry(&Hilf);
    END
   else
    BEGIN
     Lauf=(*List);
     while ((Lauf->Next!=Nil) AND (strcmp(Lauf->Next->Content,OldStr)!=0)) Lauf=Lauf->Next;
     if (Lauf->Next!=Nil)
      BEGIN
       Hilf=Lauf->Next; Lauf->Next=Hilf->Next; ClearStringEntry(&Hilf);
      END
    END
END

        char *GetStringListFirst(StringList List, StringRecPtr *Lauf)
BEGIN
   static char *Dummy="",*tmp;

   *Lauf=List;
   if (*Lauf==Nil) return Dummy;
   else
    BEGIN
     tmp=(*Lauf)->Content;
     *Lauf=(*Lauf)->Next;
     return tmp;
    END
END

        char *GetStringListNext(StringRecPtr *Lauf)
BEGIN
   static char *Dummy="",*tmp;

   if (*Lauf==Nil) return Dummy;
   else
    BEGIN
     tmp=(*Lauf)->Content;
     *Lauf=(*Lauf)->Next;
     return tmp;
    END
END

	char *GetAndCutStringList(StringList *List)
BEGIN
   StringRecPtr Hilf;
   static String Result;

   if (*List==Nil) Result[0]='\0';
   else
    BEGIN
     Hilf=(*List); *List=(*List)->Next;
     strmaxcpy(Result,Hilf->Content,255);
     free(Hilf->Content); free(Hilf);
    END
   return Result;
END

        Boolean StringListEmpty(StringList List)
BEGIN
   return (List==Nil);
END

        StringList DuplicateStringList(StringList Src)
BEGIN
   StringRecPtr Lauf;
   StringList Dest;

   InitStringList(&Dest);
   if (Src!=Nil)
    BEGIN
     AddStringListLast(&Dest,GetStringListFirst(Src,&Lauf));
     while (Lauf!=Nil)
      AddStringListLast(&Dest,GetStringListNext(&Lauf));
    END
   return Dest;
END

        Boolean StringListPresent(StringList List, char *Search)
BEGIN
   while ((List!=Nil) AND (strcmp(List->Content,Search)!=0)) List=List->Next;
   return (List!=Nil);
END

	void stringlists_init(void)
BEGIN
END
