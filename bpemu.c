/* bpemu.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Emulation einiger Borland-Pascal-Funktionen                               */
/*                                                                           */
/* Historie: 20. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <sys/types.h>
#include <ctype.h>

#include "stringutil.h"

#ifdef __MSDOS__
#include <dir.h>
#endif

#ifdef __EMX__
#include <os2.h>
#endif

	char *FExpand(char *Src)
BEGIN
   static String CurrentDir;
   String Copy;
#ifdef DRSEP
   String DrvPart;
#ifdef __EMX__
   ULONG DrvNum,Dummy;
#else      
   int DrvNum;
#endif
#endif
   char *p,*p2;

   strmaxcpy(Copy,Src,255);

#ifdef DRSEP
   p=strchr(Copy,DRSEP);
   if (p!=Nil)
    BEGIN
     memcpy(DrvPart,Copy,p-Copy); DrvPart[p-Copy]='\0'; strcpy(Copy,p+1);
    END
   else *DrvPart='\0';
#endif

#ifdef __MSDOS__
   if (*DrvPart=='\0')
    BEGIN
     DrvNum=getdisk(); *DrvPart=DrvNum+'A'; DrvPart[1]='\0'; DrvNum++;
    END
   else DrvNum=toupper(*DrvPart)-'@';
   getcurdir(DrvNum,CurrentDir);
#else
#ifdef __EMX__
   if (*DrvPart=='\0')
    BEGIN
     DosQueryCurrentDisk(&DrvNum,&Dummy);
     *DrvPart=DrvNum+'@'; DrvPart[1]='\0';
    END
   else DrvNum=toupper(*DrvPart)-'@';
   Dummy=255; DosQueryCurrentDir(DrvNum,(PBYTE) CurrentDir,&Dummy);
#else
   getcwd(CurrentDir,255);
#endif   
#endif

   if (CurrentDir[strlen(CurrentDir)-1]!=PATHSEP) strmaxcat(CurrentDir,SPATHSEP,255);
   if (*CurrentDir!=PATHSEP) strmaxprep(CurrentDir,SPATHSEP,255);

   if (*Copy==PATHSEP) 
    BEGIN
     strmaxcpy(CurrentDir,SPATHSEP,255); strcpy(Copy,Copy+1);
    END

#ifdef DRSEP
   strmaxprep(CurrentDir,SDRSEP,255);
   strmaxprep(CurrentDir,DrvPart,255);
#endif

   while((p=strchr(Copy,PATHSEP))!=Nil)
    BEGIN
     *p='\0';
     if (strcmp(Copy,".")==0);
     else if ((strcmp(Copy,"..")==0) AND (strlen(CurrentDir)>1))
      BEGIN
       CurrentDir[strlen(CurrentDir)-1]='\0';
       p2=strrchr(CurrentDir,PATHSEP); p2[1]='\0';
      END
     else
      BEGIN
       strmaxcat(CurrentDir,Copy,255); strmaxcat(CurrentDir,SPATHSEP,255);
      END
     strcpy(Copy,p+1);
    END

   strmaxcat(CurrentDir,Copy,255);

   return CurrentDir; 
END

	char *FSearch(char *File, char *Path)
BEGIN
   static String Component;
   char *p,*start,Save='\0';
   FILE *Dummy; 
   Boolean OK;  

   Dummy=fopen(File,"r"); OK=(Dummy!=Nil);
   if (OK)
    BEGIN
     fclose(Dummy);
     strmaxcpy(Component,File,255); return Component;
    END

   start=Path;
   do
    BEGIN
     if (*start=='\0') break;
     p=strchr(start,':');
     if (p!=Nil) 
      BEGIN
       Save=(*p); *p='\0';
      END
     strmaxcpy(Component,start,255);
     strmaxcat(Component,SPATHSEP,255);
     strmaxcat(Component,File,255);
     if (p!=Nil) *p=Save;
     Dummy=fopen(Component,"r"); OK=(Dummy!=Nil); 
     if (OK)
      BEGIN
       fclose(Dummy);
       return Component;
      END
     start=p+1;
    END
   while (p!=Nil);

   *Component='\0'; return Component;
END

	long FileSize(FILE *file)
BEGIN
   long Save=ftell(file),Size;

   fseek(file,0,SEEK_END); 
   Size=ftell(file);
   fseek(file,Save,SEEK_SET);
   return Size;
END

	Byte Lo(Word inp)
BEGIN
   return (inp&0xff);
END

	Byte Hi(Word inp)
BEGIN
   return ((inp>>8)&0xff);
END

	Boolean Odd(int inp)
BEGIN
   return ((inp&1)==1);
END

	void bpemu_init(void)
BEGIN
END
