/* cmdarg.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verarbeitung Kommandozeilenparameter                                      */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*            1. 6.1996 Empty-Funktion                                       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"
#include "cmdarg.h"
#include "nls.h"
#include "nlmessages.h"
#include "cmdarg.rsc"

LongInt ParamCount;                   /* Kommandozeilenparameter */
char **ParamStr;

TMsgCat MsgCat;

        static void ClrBlanks(char *tmp)
BEGIN
   int cnt;

   for (cnt=0; isspace((unsigned int) tmp[cnt]); cnt++);
   if (cnt>0) strcpy(tmp,tmp+cnt);
END

	Boolean ProcessedEmpty(CMDProcessed Processed)
BEGIN
   int z;
   
   for (z=1; z<=ParamCount; z++)
    if (Processed[z]) return False;
   return True; 
END

	static CMDResult ProcessParam(CMDRec *Def, Integer Cnt, 
                                      char *O_Param, char *O_Next)
BEGIN
   int Start;
   Boolean Negate;
   int z,Search;
   CMDResult TempRes;
   String s,Param,Next;

   strncpy(Param,O_Param,255); 
   strncpy(Next,O_Next,255);

   if ((*Next=='-') OR (*Next=='+')) *Next='\0';
   if ((*Param=='-') OR (*Param=='+')) 
    BEGIN
     Negate=(*Param=='+'); Start=1;

     if (Param[Start]=='#')
      BEGIN
       for (z=Start+1; z<strlen(Param); z++) Param[z]=toupper(Param[z]);
       Start++;
      END
     else if (Param[Start]=='~')
      BEGIN
       for (z=Start+1; z<strlen(Param); z++) Param[z]=tolower(Param[z]);
       Start++;
      END

     TempRes=CMDOK;

     Search=0;
     strncpy(s,Param+Start,255);
     for (z=0; z<strlen(s); z++) s[z]=toupper(s[z]);
     for (Search=0; Search<Cnt; Search++)
      if ((strlen(Def[Search].Ident)>1) AND (strcmp(s,Def[Search].Ident)==0)) break;
     if (Search<Cnt) 
      TempRes=Def[Search].Callback(Negate,Next);

     else
      for (z=Start; z<strlen(Param); z++) 
       if (TempRes!=CMDErr)
        BEGIN
         Search=0;
         for (Search=0; Search<Cnt; Search++)
          if ((strlen(Def[Search].Ident)==1) AND (Def[Search].Ident[0]==Param[z])) break;
         if (Search>=Cnt) TempRes=CMDErr;
         else
	  switch (Def[Search].Callback(Negate,Next))
           BEGIN
	    case CMDErr: TempRes=CMDErr; break;
	    case CMDArg: TempRes=CMDArg; break;
	    case CMDOK:  break;
            case CMDFile: break; /** **/
	   END
        END
     return TempRes;
    END
   else return CMDFile;
END

	static void DecodeLine(CMDRec *Def, Integer Cnt, char *OneLine,
                               CMDErrCallback ErrProc)
BEGIN
   int z;
   char *EnvStr[256],*start,*p;
   int EnvCnt=0;

   ClrBlanks(OneLine); 
   if ((*OneLine!='\0') AND (*OneLine!=';'))
    BEGIN
     start=OneLine;
     while (*start!='\0')
      BEGIN
       EnvStr[EnvCnt++]=start;
       p=strchr(start,' '); if (p==Nil) p=strchr(start,9);
       if (p!=Nil)
        BEGIN
         *p='\0'; start=p+1;
         while (isspace((unsigned int) *start)) start++;
        END
       else start+=strlen(start);
      END
     EnvStr[EnvCnt]=start;

     for (z=0; z<EnvCnt; z++)
      switch (ProcessParam(Def,Cnt,EnvStr[z],EnvStr[z+1]))
       BEGIN
        case CMDErr:
        case CMDFile: ErrProc(True,EnvStr[z]); break;
        case CMDArg:  z++; break;
        case CMDOK: break;
       END
    END
END

	void ProcessCMD(CMDRec *Def, Integer Cnt, CMDProcessed Unprocessed,
                        char *EnvName, CMDErrCallback ErrProc)
BEGIN
   int z;
   String OneLine;
   FILE *KeyFile;

   if (getenv(EnvName)==Nil) OneLine[0]='\0';
   else strncpy(OneLine,getenv(EnvName),255);

   if (OneLine[0]=='@')
    BEGIN
     strcpy(OneLine,OneLine+1); ClrBlanks(OneLine);
     KeyFile=fopen(OneLine,"r");
     if (KeyFile==Nil) ErrProc(True,catgetmessage(&MsgCat,Num_ErrMsgKeyFileNotFound));
     while (NOT feof(KeyFile))
      BEGIN
       errno=0; ReadLn(KeyFile,OneLine);
       if ((errno!=0) AND (NOT feof(KeyFile))) ErrProc(True,catgetmessage(&MsgCat,Num_ErrMsgKeyFileError));
       DecodeLine(Def,Cnt,OneLine,ErrProc);
      END
     fclose(KeyFile);
    END

   else DecodeLine(Def,Cnt,OneLine,ErrProc);

   for (z=0; z<=ParamCount; Unprocessed[z++]=(z!=0));
   for (z=1; z<=ParamCount; z++)
    if (Unprocessed[z])
     BEGIN
     switch (ProcessParam(Def,Cnt,ParamStr[z],(z<ParamCount)?ParamStr[z+1]:""))
      BEGIN
       case CMDErr: ErrProc(False,ParamStr[z]); break;
       case CMDOK:  Unprocessed[z]=False; break;
       case CMDArg: Unprocessed[z]=Unprocessed[z+1]=False; break;
       case CMDFile: break; /** **/
      END
     END
END

        char *GetEXEName(void)
BEGIN
   static String s;
   char *pos;

   strcpy(s,ParamStr[0]);
   do
    BEGIN
     pos=strchr(s,'/');
     if (pos!=Nil) strcpy(s,pos+1);
    END
   while (pos!=Nil);
   pos=strchr(s,'.'); if (pos!=Nil) *pos='\0';
   return s;
END

	void cmdarg_init(char *ProgPath)
BEGIN
   opencatalog(&MsgCat,"cmdarg.msg",ProgPath,MsgId1,MsgId2);
END

