/* asmfnums.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Datei-Nummern                                              */
/*                                                                           */
/* Historie: 15. 5.96 Grundsteinlegung                                       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "stringutil.h"
#include "chunks.h"
#include "asmdef.h"

#include "asmfnums.h"


typedef struct _TToken
         {
          struct _TToken *Next;
          char *Name;
         } TToken,*PToken;

static PToken FirstFile;


        void InitFileList(void)
BEGIN
   FirstFile=Nil;
END


        void ClearFileList(void)
BEGIN
   PToken F;

   while (FirstFile!=Nil)
    BEGIN
     F=FirstFile->Next;
     free(FirstFile->Name);
     free(FirstFile);
     FirstFile=F;
    END
END


        void AddFile(char *FName)
BEGIN
   PToken Lauf,Neu;

   if (GetFileNum(FName)!=-1) return;

   Neu=(PToken) malloc(sizeof(TToken));
   Neu->Next=Nil;
   Neu->Name=strdup(FName);
   if (FirstFile==Nil) FirstFile=Neu;
   else
    BEGIN
     Lauf=FirstFile;
     while (Lauf->Next!=Nil) Lauf=Lauf->Next;
     Lauf->Next=Neu;
    END
END


        Integer GetFileNum(char *Name)
BEGIN
   PToken FLauf=FirstFile;
   Integer Cnt=0;

   while ((FLauf!=Nil) AND (strcmp(FLauf->Name,Name)!=0))
    BEGIN
     Cnt++;
     FLauf=FLauf->Next;
    END
   return (FLauf==Nil)?(-1):(Cnt);
END


        char *GetFileName(Byte Num)
BEGIN
   PToken Lauf;
   Integer z;
   static char *Dummy="";

   Lauf=FirstFile;
   for (z=0; z<Num; z++)
    if (Lauf!=Nil) Lauf=Lauf->Next;
   return (Lauf==Nil)?(Dummy):(Lauf->Name);
END


        Integer GetFileCount(void)
BEGIN
   PToken Lauf=FirstFile;
   Integer z=0;

   while (Lauf!=Nil)
    BEGIN
     z++; Lauf=Lauf->Next;
    END;
   return z;
END


	void asmfnums_init(void)
BEGIN
   FirstFile=Nil;
END

