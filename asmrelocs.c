/* asmrelocs.c */
/****************************************************************************/
/* AS-Portierung                                                            */
/*                                                                          */
/* Verwaltung von Relokationslisten                                         */
/*                                                                          */
/* Historie: 25. 7.1999 Grundsteinlegung                                    */
/*            1. 8.1999 Merge-Funktion implementiert                        */
/*            8. 8.1999 Reloc-Liste gespeichert                             */
/*           15. 9.1999 fehlende Includes                                   */
/*                      Add in Merge                                        */
/*           19. 1.2000 TransferRelocs begonnen                             */
/*            8. 3.2000 'ambigious else'-Warnungen beseitigt                */
/*           30.10.2000 added transfer of arbitrary lists                   */
/*                                                                          */
/****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmcode.h"
#include "asmrelocs.h"
#include "fileformat.h"

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
      BEGIN
       if (PLast == Nil) PRes = PLast = PRun1;
       else 
        BEGIN
         PLast->Next = PRun1; PLast = PRun1;
        END
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

	void TransferRelocs2(PRelocEntry RelocList, LargeWord Addr, LongWord Type)
BEGIN
   PPatchEntry NewPatch;
   PRelocEntry Curr;

   while (RelocList != Nil)
    BEGIN
     Curr = RelocList;
     NewPatch = (PPatchEntry) malloc(sizeof(TPatchEntry));
     NewPatch->Next = Nil;
     NewPatch->Address = Addr;
     NewPatch->Ref = Curr->Ref;
     NewPatch->RelocType = Type;
     if (NOT Curr->Add) NewPatch->RelocType |= RelocFlagSUB;
     if (PatchLast == NULL) PatchList = NewPatch;
     else PatchLast->Next = NewPatch;
     PatchLast = NewPatch;
     RelocList = Curr->Next;
     free(Curr);
    END
END

	void TransferRelocs(LargeWord Addr, LongWord Type)
BEGIN
   TransferRelocs2(LastRelocs, Addr, Type);
   LastRelocs = NULL;
END

	void SubPCRefReloc(void)
BEGIN
   PRelocEntry Run, Prev, New;

   if (!RelSegs) return;
  
   /* search if PC subtraction evens out against an addition */

   for (Prev = Nil, Run = LastRelocs; Run != Nil; Prev = Run, Run = Run->Next)
    if ((Run->Add) AND (strcmp(Run->Ref, RelName_SegStart) == 0))
     BEGIN
      free(Run->Ref);
      if (Prev) Prev->Next = Run->Next; else LastRelocs = Run->Next;
      free(Run);
      return;
     END

   /* in case we did not find one, add a new one */

   New = (PRelocEntry) malloc(sizeof(TRelocEntry));
   New->Ref = strdup(RelName_SegStart);
   New->Add = FALSE;
   New->Next = Nil;
   if (Prev) Prev->Next = New; else LastRelocs = New;
END

	void AddExport(char *Name, LargeInt Value, LongWord Flags)
BEGIN
   PExportEntry PNew;

   PNew = (PExportEntry) malloc(sizeof(TExportEntry));
   PNew->Next = Nil;
   PNew->Name = strdup(Name);
   PNew->Value = Value;
   PNew->Flags = Flags;
   if (ExportList == Nil) ExportList = PNew;
   else ExportLast->Next = PNew;
   ExportLast = PNew;
END
