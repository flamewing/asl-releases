/* bpemu.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Emulation einiger Borland-Pascal-Funktionen                               */
/*                                                                           */
/* Historie: 20. 5.1996 Grundsteinlegung                                     */
/*           2001-04-13 Win32 fixes                                          */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

#include "strutil.h"
#include "bpemu.h"

#ifdef __MSDOS__
#include <dos.h>
#include <dir.h>
#endif

#if defined( __EMX__ ) || defined( __IBMC__ )
#include <os2.h>
#endif

	char *FExpand(char *Src)
BEGIN
   static String CurrentDir;
   String Copy;
#ifdef DRSEP
   String DrvPart;
#if defined( __EMX__ ) || defined( __IBMC__ )
   ULONG DrvNum,Dummy;
#else
#ifndef _WIN32
   int DrvNum;
#endif /* _WIN32 */
#endif /* EMX... */
#endif /* DRSEP */
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
#if defined( __EMX__ ) || defined( __IBMC__ )
   if (*DrvPart=='\0')
    BEGIN
     DosQueryCurrentDisk(&DrvNum,&Dummy);
     *DrvPart=DrvNum+'@'; DrvPart[1]='\0';
    END
   else DrvNum=toupper(*DrvPart)-'@';
   Dummy=255; DosQueryCurrentDir(DrvNum,(PBYTE) CurrentDir,&Dummy);
#else
#ifdef _WIN32
   getcwd(CurrentDir,255);
   for (p=CurrentDir; *p!='\0'; p++)
    if (*p=='/') *p='\\';
#else
   getcwd(CurrentDir,255);
#endif
#endif   
#endif

   if ((*CurrentDir) && (CurrentDir[strlen(CurrentDir)-1]!=PATHSEP))
     strmaxcat(CurrentDir,SPATHSEP,255);
   if (*CurrentDir!=PATHSEP)
     strmaxprep(CurrentDir,SPATHSEP,255);

   if (*Copy==PATHSEP) 
    BEGIN
     strmaxcpy(CurrentDir,SPATHSEP,255); strcpy(Copy,Copy+1);
    END

#ifdef DRSEP
#ifdef _WIN32
   /* win32 getcwd() does not deliver current drive letter, therefore only prepend a drive letter
      if there was one before. */
   if (*DrvPart)
#endif
    BEGIN
     strmaxprep(CurrentDir,SDRSEP,255);
     strmaxprep(CurrentDir,DrvPart,255);
    END
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
     p=strchr(start,DIRSEP);
     if (p!=Nil) 
      BEGIN
       Save=(*p); *p='\0';
      END
     strmaxcpy(Component,start,255);
#ifdef _WIN32
     DeCygwinPath(Component);
#endif
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

	Boolean DirScan(char *Mask, charcallback callback)
BEGIN
   char Name[1024];

#ifdef __MSDOS__
   struct ffblk blk;
   int res;
   char *pos;

   res=findfirst(Mask,&blk,FA_RDONLY|FA_HIDDEN|FA_SYSTEM|FA_LABEL|FA_DIREC|FA_ARCH);
   if (res<0) return False;
   pos=strrchr(Mask,PATHSEP); if (pos==Nil) pos=strrchr(Mask,DRSEP);
   if (pos==Nil) pos=Mask; else pos++;
   memcpy(Name,Mask,pos-Mask);
   while (res==0)
    BEGIN
     if ((blk.ff_attrib&(FA_LABEL|FA_DIREC))==0)
      BEGIN
       strcpy(Name+(pos-Mask),blk.ff_name);
       callback(Name);
      END
     res=findnext(&blk);
    END
   return True;
#else
#if defined ( __EMX__ ) || defined ( __IBMC__ )
   HDIR hdir=1;
   FILEFINDBUF3 buf;
   ULONG rescnt;
   USHORT res;
   char *pos;

   rescnt=1; res=DosFindFirst(Mask,&hdir,0x16,&buf,sizeof(buf),&rescnt,1);
   if (res!=0) return False;
   pos=strrchr(Mask,PATHSEP); if (pos==Nil) pos=strrchr(Mask,DRSEP);
   if (pos==Nil) pos=Mask; else pos++;
   memcpy(Name,Mask,pos-Mask);
   while (res==0)
    BEGIN
     strcpy(Name+(pos-Mask),buf.achName); callback(Name);
     res=DosFindNext(hdir,&buf,sizeof(buf),&rescnt);
    END
   return True;
#else
   strmaxcpy(Name,Mask,255); callback(Name); return True;
#endif
#endif
END

	LongInt GetFileTime(char *Name)
BEGIN
   struct stat st;

   if (stat(Name,&st)==-1) return 0;
   else return st.st_mtime;
END

#ifdef _WIN32

/* convert CygWin-style paths back to something usable by other Win32 apps */

char *DeCygWinDirList(char *pStr)
{
  char *pRun;

  for (pRun = pStr; *pRun; pRun++)
    if (*pRun == ':')
      *pRun = ';';

  return pStr;
}

char *DeCygwinPath(char *pStr)
{
  char *pRun;

  if ((strlen(pStr) >= 4)
   && (pStr[0] =='/') && (pStr[1] == '/') && (pStr[3] == '/')
   && (isalpha(pStr[2])))
  {
    strcpy(pStr, pStr + 1);
    pStr[0] = pStr[1];
    pStr[1] = ':';
  }

  if ((strlen(pStr) >= 4)
   && (pStr[0] =='\\') && (pStr[1] == '\\') && (pStr[3] == '\\')
   && (isalpha(pStr[2])))
  {
    strcpy(pStr, pStr + 1);
    pStr[0] = pStr[1];
    pStr[1] = ':';
  }

  for (pRun = pStr; *pRun; pRun++)
    if (*pRun == '/')
      *pRun = '\\';

  return pStr;
}
#endif

	void bpemu_init(void)
BEGIN
END
