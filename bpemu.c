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

#include "stringutil.h"

	char *FExpand(char *Src)
BEGIN
   static String CurrentDir;
   String Copy;
   char *p,*p2;

   getcwd(CurrentDir,255);
   if (CurrentDir[strlen(CurrentDir)-1]!='/') strmaxcat(CurrentDir,"/",255);
   if (*CurrentDir!='/') strmaxprep(CurrentDir,"/",255);
   strmaxcpy(Copy,Src,255);
   
   if (*Copy=='/') 
    BEGIN
     strmaxcpy(CurrentDir,"/",255); strcpy(Copy,Copy+1);
    END

   while((p=strchr(Copy,'/'))!=Nil)
    BEGIN
     *p='\0';
     if (strcmp(Copy,".")==0);
     else if ((strcmp(Copy,"..")==0) AND (strlen(CurrentDir)>1))
      BEGIN
       CurrentDir[strlen(CurrentDir)-1]='\0';
       p2=strrchr(CurrentDir,'/'); p2[1]='\0';
      END
     else
      BEGIN
       strmaxcat(CurrentDir,Copy,255); strmaxcat(CurrentDir,"/",255);
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
       Save=*p; *p='\0';
      END
     strmaxcpy(Component,start,255);
     strmaxcat(Component,"/",255);
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
