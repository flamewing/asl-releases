/* stdhandl.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Bereitstellung von fuer AS benoetigten Handle-Funktionen                  */
/*                                                                           */
/* Historie:  5. 4.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <sys/stat.h>
#include "stdhandl.h"

#if defined ( __EMX__ ) || defined ( __IBMC__ )
#include <os2.h>
#endif

#ifdef __TURBOC__
#include <io.h>
#endif

#ifndef S_ISCHR
#ifdef __IBMC__
#define S_ISCHR(m)    ((m) & S_IFCHR)
#else
#define S_ISCHR(m)    (((m) & S_IFMT) == S_IFCHR)
#endif
#endif
#ifndef S_ISREG
#ifdef __IBMC__
#define S_ISREG(m)    ((m) & S_IFREG)
#else
#define S_ISREG(m)    (((m) & S_IFMT) == S_IFREG)
#endif
#endif

TRedirected Redirected;

/* eine Textvariable auf einen der Standardkanaele umbiegen.  Die Reduzierung
   der Puffergroesse auf fast Null verhindert, dass durch Pufferung evtl. Ausgaben
   durcheinandergehen. */

        static void AssignHandle(FILE **T, Word Num)
BEGIN
#ifdef NODUP
   switch (Num)
    BEGIN
     case 0: *T=stdin; break;
     case 1: *T=stdout; break;
     case 2: *T=stderr; break;
     default: *T=Nil;
    END
#else
   *T=fdopen(dup(Num),"w");
#ifndef _WIN32t
   setbuf(*T,Nil);
#endif
#endif
END

/* Eine Datei unter Beruecksichtigung der Standardkanaele oeffnen */

        void RewriteStandard(FILE **T, String Path)
BEGIN
   if ((strlen(Path)==2) AND (Path[0]=='!') AND (Path[1]>='0') AND (Path[1]<='2'))
    AssignHandle(T,Path[1]-'0');
   else *T=fopen(Path,"w");
END

	void stdhandl_init(void)
BEGIN
#ifdef __EMX__
   ULONG HandType,DevAttr;

#else
#ifdef __TURBOC__
   int HandErg;

#else
   struct stat stdout_stat;

#endif
#endif   

   /* wohin zeigt die Standardausgabe ? */

#ifdef __EMX__
   DosQueryHType(1,&HandType,&DevAttr);
   if ((HandType & 0xff)==FHT_DISKFILE) Redirected=RedirToFile;
   else if ((DevAttr & 2)==0) Redirected=RedirToDevice;
   else Redirected=NoRedir;

#else
#ifdef __TURBOC__
   HandErg=ioctl(1,0x00);
   if ((HandErg & 2)==2) Redirected=NoRedir;
   else if ((HandErg & 0x8000)==0) Redirected=RedirToFile;
   else Redirected=RedirToDevice;

#else
   fstat(fileno(stdout),&stdout_stat);
   if (S_ISREG(stdout_stat.st_mode)) Redirected=RedirToFile;
   else if (S_ISFIFO(stdout_stat.st_mode)) Redirected=RedirToDevice;
   else Redirected=NoRedir;

#endif
#endif   
END
