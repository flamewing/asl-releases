/* stdhandl.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* reitstellung von fuer AS benoetigten Handle-Funktionen                    */
/*                                                                           */
/* Historie:  5. 4.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <sys/stat.h>
#include "stdhandl.h"

TRedirected Redirected;

/* eine Textvariable auf einen der Standardkanaele umbiegen.  Die Reduzierung
   der Puffergroesse auf fast Null verhindert, dass durch Pufferung evtl. Ausgaben
   durcheinandergehen. */

        static void AssignHandle(FILE **T, Word Num)
BEGIN
   *T=fdopen(dup(Num),"w");
   setbuf(*T,Nil);
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
   struct stat stdout_stat;

   /* wohin zeigt die Standardausgabe ? */

   fstat(fileno(stdout),&stdout_stat);
   if (S_ISREG(stdout_stat.st_mode)) Redirected=RedirToFile;
   else if (S_ISCHR(stdout_stat.st_mode)) Redirected=RedirToDevice;
   else Redirected=NoRedir;
END
