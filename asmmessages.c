/* asmmessages.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Einlesen und Verwalten von Meldungs-Strings                               */
/*                                                                           */
/* Historie: 13. 8.1997 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "endian.h"
#include "bpemu.h"
#include "nls.h"
#include "as_msgs.h"

/*****************************************************************************/

static char *IdentString="AS Message Catalog - not readable\n\032\004";

static char *EOpenMsg="cannot open msg file %s";
static char *ERdMsg="cannot read from msg file";
static char *EIndMsg="string table index error";

static char *MsgBlock;
static LongInt *StrPosis;
static LongInt MsgCount=0;

/*****************************************************************************/

	static void error(char *Msg)
BEGIN
   fprintf(stderr,"message catalog handling: %s - program terminated\n",Msg);
   exit(255);
END

	char *getmessage(int Num)
BEGIN
   static char umess[256];
 
   if ((Num>=0) AND (Num<MsgCount)) return MsgBlock+StrPosis[Num];
   else 
    BEGIN
     sprintf(umess,"Hey sie Sack, Message-Nummer %d gibbet nich!",Num);
     return umess;
    END
END

	FILE *myopen(char *name)
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

	void asmmessages_init(char *File, char *ProgPath)
BEGIN
   FILE *MsgFile;
   char str[2048],*ptr;
#if defined(DOS_NLS) || defined (OS2_NLS)
   NLS_CountryInfo NLSInfo;
#else
   char *lcstring;
#endif
   LongInt DefPos=-1,MomPos,DefLength=0,MomLength,z,StrStart,CtryCnt,Ctrys[100];
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

   MsgFile=myopen(File);
   if (MsgFile==Nil)
    BEGIN
     if (*ProgPath!='\0') 
      BEGIN
       ptr=strrchr(ProgPath,PATHSEP); if (ptr==Nil) ptr=ProgPath+strlen(ProgPath);
       memcpy(str,ProgPath,ptr-ProgPath); str[ptr-ProgPath]='\0';
       strcat(str,SPATHSEP); strcat(str,File);
       MsgFile=myopen(str);
      END
     if (MsgFile==Nil)
      BEGIN
       sprintf(str,"%s/%s",LIBDIR,File);
       MsgFile=myopen(str);
        if (MsgFile==Nil)
         BEGIN
          sprintf(str,EOpenMsg,File); error(str);
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
        if (Ctrys[z]==NLSInfo.CountryCode) Gotcha=True;
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
   MsgCount=(StrStart-MomPos)>>2;
   StrPosis=(LongInt *) malloc(sizeof(LongInt)*MsgCount);
   StrPosis[0]=0;
   if (fread(StrPosis+1,4,MsgCount-1,MsgFile)!=MsgCount-1) error(ERdMsg);
   if (BigEndian) DSwap(StrPosis+1,(MsgCount-1)<<2);
   for (z=1; z<MsgCount; z++)
    BEGIN
     StrPosis[z]-=StrStart;
     if ((StrPosis[z]<0) OR (StrPosis[z]>=MomLength)) error(EIndMsg);
    END

   /* read string table */

   fseek(MsgFile,StrStart,SEEK_SET);
   MsgBlock=(char *) malloc(MomLength);
   if (fread(MsgBlock,1,MomLength,MsgFile)!=MomLength) error(ERdMsg);

   fclose(MsgFile);
END