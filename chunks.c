/* chunks.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Adressbereichslisten                                       */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "stringutil.h"

#include "chunks.h"

/*--------------------------------------------------------------------------*/
/* eine Chunkliste initialisieren */

        void InitChunk(ChunkList *NChunk)
BEGIN
   NChunk->RealLen=0; 
   NChunk->AllocLen=0;
   NChunk->Chunks=Nil;
END

/*--------------------------------------------------------------------------*/
/* eine Chunkliste um einen Eintrag erweitern */

        static Boolean Overlap(LargeWord Start1, LargeWord Len1, LargeWord Start2, LargeWord Len2)
BEGIN
   return   ((Start1==Start2)
          OR ((Start2>Start1) AND (Start1+Len1>=Start2))
          OR ((Start1>Start2) AND (Start2+Len2>=Start1)));
END

        static void SetChunk(OneChunk *NChunk, LargeWord Start1, LargeWord Len1, 
                                               LargeWord Start2, LargeWord Len2)
BEGIN
   NChunk->Start =min(Start1,Start2);
   NChunk->Length=max(Start1+Len1-1,Start2+Len2-1)-NChunk->Start+1;
END

        static void IncChunk(ChunkList *NChunk)
BEGIN
   if (NChunk->RealLen+1>NChunk->AllocLen)
    BEGIN
     if (NChunk->RealLen==0) 
      NChunk->Chunks=(OneChunk *) malloc(sizeof(OneChunk));
     else
      NChunk->Chunks=(OneChunk *) realloc(NChunk->Chunks,sizeof(OneChunk)*(NChunk->RealLen+1));
     NChunk->AllocLen=NChunk->RealLen+1;
    END
END

        Boolean AddChunk(ChunkList *NChunk, LargeWord NewStart, LargeWord NewLen, Boolean Warn)
BEGIN
   Word z,f1=0,f2=0;
   Boolean Found;
   LongInt PartSum;
   Boolean Result;

   Result=False;

   if (NewLen==0) return Result;

   /* herausfinden, ob sich das neue Teil irgendwo mitanhaengen laesst */

   Found=False;
   for (z=0; z<NChunk->RealLen; z++)
    if (Overlap(NewStart,NewLen,NChunk->Chunks[z].Start,NChunk->Chunks[z].Length))
     BEGIN
      Found=True; f1=z; break;
     END

   /* Fall 1: etwas gefunden : */

   if (Found)
    BEGIN
     /* gefundene Chunk erweitern */

     PartSum=NChunk->Chunks[f1].Length+NewLen;
     SetChunk(NChunk->Chunks+f1,NewStart,NewLen,NChunk->Chunks[f1].Start,NChunk->Chunks[f1].Length);
     if (Warn)
      if (PartSum!=NChunk->Chunks[f1].Length) Result=True;

     /* schauen, ob sukzessiv neue Chunks angebunden werden koennen */

     do
      BEGIN
       Found=False;
       for (z=1; z<NChunk->RealLen; z++)
        if (z!=f1)
         if (Overlap(NChunk->Chunks[z].Start,NChunk->Chunks[z].Length,NChunk->Chunks[f1].Start,NChunk->Chunks[f1].Length))
          BEGIN
           Found=True; f2=z; break;
          END
       if (Found)
        BEGIN
        SetChunk(NChunk->Chunks+f1,NChunk->Chunks[f1].Start,NChunk->Chunks[f1].Length,NChunk->Chunks[f2].Start,NChunk->Chunks[f2].Length);
        NChunk->Chunks[f2]=NChunk->Chunks[--NChunk->RealLen];
        END
      END
     while (Found);
    END

   /* ansonsten Feld erweitern und einschreiben */

   else
    BEGIN
     IncChunk(NChunk);
     
     NChunk->Chunks[NChunk->RealLen].Length=NewLen; 
     NChunk->Chunks[NChunk->RealLen].Start=NewStart;
     NChunk->RealLen++;
    END

   return Result;
END

/*--------------------------------------------------------------------------*/
/* Ein Stueck wieder austragen */

        void DeleteChunk(ChunkList *NChunk, LargeWord DelStart, LargeWord DelLen)
BEGIN
   Word z;
   LargeWord OStart;

   if (DelLen==0) return;

   z=0;
   while (z<=NChunk->RealLen)
    BEGIN
     if (Overlap(DelStart,DelLen,NChunk->Chunks[z].Start,NChunk->Chunks[z].Length))
      BEGIN
       if (NChunk->Chunks[z].Start>=DelStart)
        if (DelStart+DelLen>=NChunk->Chunks[z].Start+NChunk->Chunks[z].Length)
         BEGIN
          /* ganz loeschen */
          NChunk->Chunks[z]=NChunk->Chunks[--NChunk->RealLen];
         END
        else
         BEGIN
          /* unten abschneiden */
          OStart=NChunk->Chunks[z].Start; NChunk->Chunks[z].Start=DelStart+DelLen;
          NChunk->Chunks[z].Start-=NChunk->Chunks[z].Start-OStart;
         END
       else
        if (DelStart+DelLen>=NChunk->Chunks[z].Start+NChunk->Chunks[z].Length)
         BEGIN
          /* oben abschneiden */
          NChunk->Chunks[z].Length=DelStart-NChunk->Chunks[z].Start;
          /* wenn Laenge 0, ganz loeschen */
          if (NChunk->Chunks[z].Length==0)
           BEGIN
            NChunk->Chunks[z]=NChunk->Chunks[--NChunk->RealLen];
           END
         END
        else
         BEGIN
          /* teilen */
          IncChunk(NChunk);
          NChunk->Chunks[NChunk->RealLen].Start=DelStart+DelLen;
          NChunk->Chunks[NChunk->RealLen].Length=NChunk->Chunks[z].Start+NChunk->Chunks[z].Length-NChunk->Chunks[NChunk->RealLen].Start;
          NChunk->Chunks[z].Length=DelStart-NChunk->Chunks[z].Start;
         END
      END
     z++;
    END
END

	void chunks_init(void)
BEGIN
END
