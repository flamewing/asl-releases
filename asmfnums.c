/* asmfnums.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Datei-Nummern                                              */
/*                                                                           */
/* Historie: 15. 5.1996 Grundsteinlegung                                     */
/*           25. 7.1998 GetFileName jetzt mit int statt Byte                 */
/*                      Verwaltung Adreﬂbereiche                             */
/*                      Caching FileCount                                    */
/*           16. 8.1998 Ruecksetzen Adressbereiche                           */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"

#include "asmfnums.h"

#ifdef HAS64
#ifdef _STDC_
#define ADRMAX 9223372036854775807ull
#else
#define ADRMAX 9223372036854775807ll
#endif
#else
#ifdef _STDC_
#define ADRMAX 4294967295ul
#else
#define ADRMAX 4294967295l
#endif
#endif


typedef struct _TToken
         {
          struct _TToken *Next;
          LargeWord FirstAddr,LastAddr;
          char *Name;
         } TToken,*PToken;

static PToken FirstFile;
static int FileCount;

        void InitFileList(void)
BEGIN
   FirstFile=Nil; FileCount=0;
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
   FileCount=0;
END


	static PToken SearchToken(int Num)
BEGIN
   PToken Lauf=FirstFile;

   while (Num>0)
    BEGIN
     if (Lauf==Nil) return Nil;
     Num--; Lauf=Lauf->Next;
    END
   return Lauf;
END


        void AddFile(char *FName)
BEGIN
   PToken Lauf,Neu;

   if (GetFileNum(FName)!=-1) return;

   Neu=(PToken) malloc(sizeof(TToken));
   Neu->Next=Nil;
   Neu->Name=strdup(FName);
   Neu->FirstAddr=ADRMAX;
   Neu->LastAddr=0;
   if (FirstFile==Nil) FirstFile=Neu;
   else
    BEGIN
     Lauf=FirstFile;
     while (Lauf->Next!=Nil) Lauf=Lauf->Next;
     Lauf->Next=Neu;
    END
   FileCount++;
END


        Integer GetFileNum(char *Name)
BEGIN
   PToken FLauf=FirstFile;
   int Cnt=0;

   while ((FLauf!=Nil) AND (strcmp(FLauf->Name,Name)!=0))
    BEGIN
     Cnt++;
     FLauf=FLauf->Next;
    END
   return (FLauf==Nil)?(-1):(Cnt);
END


        char *GetFileName(int Num)
BEGIN
   PToken Lauf=SearchToken(Num);
   static char *Dummy="";

   return (Lauf==Nil)?(Dummy):(Lauf->Name);
END


        Integer GetFileCount(void)
BEGIN
   return FileCount;
END


	void AddAddressRange(int File, LargeWord Start, LargeWord Len)
BEGIN
   PToken Lauf=SearchToken(File);

   if (Lauf==Nil) return;

   if (Start<Lauf->FirstAddr) Lauf->FirstAddr=Start;
   if ((Len+=Start-1)>Lauf->LastAddr) Lauf->LastAddr=Len;
END


	void GetAddressRange(int File, LargeWord *Start, LargeWord *End)
BEGIN
   PToken Lauf=SearchToken(File);

   if (Lauf==Nil)
    BEGIN
     *Start=ADRMAX; *End=0;
    END
   else
    BEGIN
     *Start=Lauf->FirstAddr; *End=Lauf->LastAddr;
    END
END

	void ResetAddressRanges(void)
BEGIN
   PToken Run;

   for (Run=FirstFile; Run!=Nil; Run=Run->Next)
    BEGIN
     Run->FirstAddr=ADRMAX;
     Run->LastAddr=0;
    END
END

	void asmfnums_init(void)
BEGIN
   FirstFile=Nil; FileCount=0;
END

