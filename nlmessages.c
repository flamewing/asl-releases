/* nlmessages.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Einlesen und Verwalten von Meldungs-Strings                               */
/*                                                                           */
/* Historie: 13. 8.1997 Grundsteinlegung                                     */
/*           17. 8.1997 Verallgemeinerung auf mehrere Kataloge               */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include "strutil.h"

#include "endian.h"
#include "bpemu.h"
#include "nls.h"

#include "nlmessages.h"

/*****************************************************************************/

static char *IdentString="AS Message Catalog - not readable\n\032\004";

static char *EOpenMsg="cannot open msg file %s";
static char *ERdMsg="cannot read from msg file";
static char *EIndMsg="string table index error";

static TMsgCat DefaultCatalog={Nil,Nil,0};

/*****************************************************************************/

	static void error(char *Msg)
BEGIN
   fprintf(stderr,"message catalog handling: %s - program terminated\n",Msg);
   exit(255);
END

	char *catgetmessage(PMsgCat Catalog, int Num)
BEGIN
   if ((Num>=0) AND (Num<Catalog->MsgCount)) return Catalog->MsgBlock+Catalog->StrPosis[Num];
   else 
    BEGIN
     static char *umess = NULL;
 
     if (!umess)
       umess = (char*)malloc(sizeof(char) * STRINGSIZE);
     sprintf(umess,"catgetmessage: message number %d does not exist", Num);
     return umess;
    END
END

	char *getmessage(int Num)
BEGIN
   return catgetmessage(&DefaultCatalog,Num);
END

	FILE *myopen(char *name, LongInt MsgId1, LongInt MsgId2)
BEGIN
   FILE *tmpfile;
   String line;
   LongInt RId1,RId2;
   Boolean EForm=False;

   tmpfile=fopen(name,OPENRDMODE);
   if (tmpfile==Nil) return Nil;
   if (fread(line,1,strlen(IdentString),tmpfile)!=strlen(IdentString)) EForm=True;
   if (memcmp(line,IdentString,strlen(IdentString))!=0) EForm=True;
   if (NOT Read4(tmpfile,&RId1)) EForm=True;
   if (RId1!=MsgId1) EForm=True;
   if (NOT Read4(tmpfile,&RId2)) EForm=True;
   if (RId2!=MsgId2) EForm=True;
   if (EForm)
    BEGIN
     fclose(tmpfile);
     fprintf(stderr,"message catalog handling: warning: %s has invalid format or is out of date\n",name);
     return Nil;
    END
   else return tmpfile;
END

#define MSGPATHNAME "AS_MSGPATH"

	void opencatalog(PMsgCat Catalog, char *File, char *Path, LongInt MsgId1, LongInt MsgId2)
BEGIN
   FILE *MsgFile;
   char str[2048],*ptr;
#if defined(DOS_NLS) || defined (OS2_NLS)
   NLS_CountryInfo NLSInfo;
#else
   char *lcstring;
#endif
   LongInt DefPos= -1,MomPos,DefLength=0,MomLength,z,StrStart,CtryCnt,Ctrys[100];
   Boolean fi,Gotcha;

   /* get reference for finding out which language set to use */

#if defined(DOS_NLS) || defined (OS2_NLS)
   NLS_GetCountryInfo(&NLSInfo);
#else
   lcstring=getenv("LC_MESSAGES");
   if (lcstring==Nil) lcstring=getenv("LC_ALL");
   if (lcstring==Nil) lcstring=getenv("LANG");
   if (lcstring==Nil) lcstring="";
#endif

   /* find first valid message file */

   MsgFile=myopen(File,MsgId1,MsgId2);
   if (MsgFile==Nil)
    BEGIN
     if (*Path!='\0') 
      BEGIN
#ifdef __CYGWIN32__
       for (ptr=Path; *ptr!='\0'; ptr++)
        if (*ptr=='/') *ptr='\\';
#endif    
       ptr=strrchr(Path,PATHSEP); if (ptr==Nil) ptr=Path+strlen(Path);
       memcpy(str,Path,ptr-Path); str[ptr-Path]='\0';
       strcat(str,SPATHSEP); strcat(str,File);
       MsgFile=myopen(str,MsgId1,MsgId2);
      END
     if (MsgFile==Nil)
      BEGIN
       ptr=getenv(MSGPATHNAME);
       if (ptr!=Nil)
        BEGIN
         sprintf(str,"%s/%s",ptr,File);
         MsgFile=myopen(str,MsgId1,MsgId2);
        END
       else
        BEGIN
         ptr=getenv("PATH");
         if (ptr==Nil) MsgFile=Nil;
         else
          BEGIN
           strmaxcpy(str,ptr,255);
#ifdef __CYGWIN32__
           DeCygWinDirList(str);
#endif
           ptr=FSearch(File,str);
           MsgFile=(*ptr!='\0') ? myopen(ptr,MsgId1,MsgId2) : Nil;
          END
        END
       if (MsgFile==Nil)
        BEGIN
         sprintf(str,"%s/%s",LIBDIR,File);
         MsgFile=myopen(str,MsgId1,MsgId2);
         if (MsgFile==Nil)
          BEGIN
           sprintf(str,EOpenMsg,File); error(str);
          END
        END
      END
    END

   Gotcha=False;
   do
    BEGIN
     ptr=str;
     do 
      BEGIN
       if (fread(ptr,1,1,MsgFile)!=1) error(ERdMsg);
       fi=(*ptr=='\0');
       if (NOT fi) ptr++;
      END
     while (NOT fi);
     if (*str!='\0')
      BEGIN
       if (NOT Read4(MsgFile,&MomLength)) error(ERdMsg);
       if (NOT Read4(MsgFile,&CtryCnt)) error(ERdMsg);
       for (z=0; z<CtryCnt; z++)
        if (NOT Read4(MsgFile,Ctrys+z)) error(ERdMsg);
       if (NOT Read4(MsgFile,&MomPos)) error(ERdMsg);
       if (DefPos==-1) { DefPos=MomPos; DefLength=MomLength; }
#if defined(DOS_NLS) || defined (OS2_NLS)
       for (z=0; z<CtryCnt; z++)
        if (Ctrys[z]==NLSInfo.Country) Gotcha=True;
#else
       Gotcha=(strncasecmp(lcstring,str,strlen(str))==0);
#endif       
      END
    END
   while ((*str!='\0') AND (NOT Gotcha));
   if (*str=='\0') { MomPos=DefPos; MomLength=DefLength; }

   /* read pointer table */

   fseek(MsgFile,MomPos,SEEK_SET);
   if (NOT Read4(MsgFile,&StrStart)) error(ERdMsg);
   Catalog->MsgCount=(StrStart-MomPos)>>2;
   Catalog->StrPosis=(LongInt *) malloc(sizeof(LongInt)*Catalog->MsgCount);
   Catalog->StrPosis[0]=0;
   if (fread(Catalog->StrPosis+1,4,Catalog->MsgCount-1,MsgFile) + 1 != Catalog->MsgCount)
     error(ERdMsg);
   if (BigEndian) DSwap(Catalog->StrPosis+1,(Catalog->MsgCount-1)<<2);
   for (z=1; z<Catalog->MsgCount; z++)
    BEGIN
     Catalog->StrPosis[z]-=StrStart;
     if ((Catalog->StrPosis[z]<0) OR (Catalog->StrPosis[z]>=MomLength)) error(EIndMsg);
    END

   /* read string table */

   fseek(MsgFile,StrStart,SEEK_SET);
   Catalog->MsgBlock=(char *) malloc(MomLength);
   if (fread(Catalog->MsgBlock,1,MomLength,MsgFile)!=MomLength) error(ERdMsg);

   fclose(MsgFile);
END

	void nlmessages_init(char *File, char *ProgPath, LongInt MsgId1, LongInt MsgId2)
BEGIN
   opencatalog(&DefaultCatalog,File,ProgPath,MsgId1,MsgId2);
END
