/* asmrelocs.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Relokationslisten                                          */
/*                                                                           */
/* Historie: 25. 7.1999 Grundsteinlegung                                     */
/*            1. 8.1999 Merge-Funktion implementiert                         */
/*            8. 8.1999 Reloc-Liste gespeichert                              */
/*           15. 9.1999 fehlende Includes                                    */
/*                      Add in Merge                                         */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmrelocs.h"

/*---------------------------------------------------------------------------*/

PRelocEntry LastRelocs = Nil;

/*---------------------------------------------------------------------------*/

	PRelocEntry MergeRelocs(PRelocEntry *list1, PRelocEntry *list2,
                                Boolean Add)
BEGIN
   PRelocEntry PRun1, PRun2, PPrev, PNext, PLast, PRes;

   PRun1 = *list1; PLast = PRes = Nil;

   /* ueber alle in Liste 1 */

   while (PRun1 != Nil)
    BEGIN
     /* Uebereinstimmung suchen, die sich aufhebt */

     PNext = PRun1->Next;
     PRun2 = *list2; PPrev = Nil;
     while (PRun2 != Nil)
      if ((strcasecmp(PRun1->Ref, PRun2->Ref) == 0) AND ((PRun1->Add != PRun2->Add) != Add))
       BEGIN
        /* gefunden -> beide weg */

        free(PRun1->Ref); free(PRun2->Ref);
        if (PPrev == Nil) *list2 = PRun2->Next;
        else PPrev->Next = PRun2->Next;
        free(PRun2); free(PRun1); PRun1 = Nil;
        break;
       END
      else
       BEGIN
        PPrev = PRun2; PRun2 = PRun2->Next;
       END

     /* ansonsten an Ergebnisliste anhaengen */

     if (PRun1 != Nil)
      if (PLast == Nil) PRes = PLast = PRun1;
      else 
       BEGIN
        PLast->Next = PRun1; PLast = PRun1;
       END
     PRun1 = PNext;
    END

   /* Reste aus Liste 2 nicht vergessen */

   if (PLast == Nil) PRes = *list2;
   else PLast->Next = *list2;

   /* Quellisten jetzt leer */

   *list1 = *list2 = Nil;

   /* fertich */

   return PRes;
END

	void InvertRelocs(PRelocEntry *erg, PRelocEntry *src)
BEGIN
   PRelocEntry SRun;

   for (SRun = *src; SRun != Nil; SRun = SRun->Next)
    SRun->Add = NOT (SRun->Add);

   *erg = *src;
END

	void FreeRelocs(PRelocEntry *list)
BEGIN
   PRelocEntry Run;

   while (*list != Nil)
    BEGIN
     Run = *list;
     *list = (*list)->Next;
     free(Run->Ref);
     free(Run);
    END
END

	 PRelocEntry DupRelocs(PRelocEntry src)
BEGIN
   PRelocEntry First, Run, SRun, Neu;

   First = Run = Nil;
   for (SRun = src; SRun != Nil; SRun = SRun->Next)
    BEGIN
     Neu = (PRelocEntry) malloc(sizeof(TRelocEntry));
     Neu->Next = Nil;
     Neu->Ref = strdup(SRun->Ref);
     Neu->Add = SRun->Add;
     if (First == Nil) First = Neu;
     else Run->Next = Neu;
     Run = Neu;
    END

   return First;
END

	void SetRelocs(PRelocEntry List)
BEGIN
   if (LastRelocs != Nil)
    BEGIN
     WrError(1155);
     FreeRelocs(&LastRelocs);
    END
   LastRelocs = List;
END
