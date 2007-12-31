/* cmdarg.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verarbeitung Kommandozeilenparameter                                      */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*            1. 6.1996 Empty-Funktion                                       */
/*           17. 4.1999 Key-Files in Kommandozeile                           */
/*            3. 8.2000 added command line args as slashes                   */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"
#include "stringlists.h"
#include "cmdarg.h"
#include "nls.h"
#include "nlmessages.h"
#include "cmdarg.rsc"

LongInt ParamCount;                   /* Kommandozeilenparameter */
char **ParamStr;

TMsgCat MsgCat;
StringList FileArgList;

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

        static void ProcessFile(char *Name_O, CMDRec *Def, Integer Cnt, CMDErrCallback ErrProc);

	static CMDResult ProcessParam(CMDRec *Def, Integer Cnt, char *O_Param,
                                      char *O_Next, Boolean AllowLink,
                                      CMDErrCallback ErrProc)
BEGIN
   int Start;
   Boolean Negate;
   int z,Search;
   CMDResult TempRes;
   String s,Param,Next;

   strncpy(Param, O_Param, 255U); 
   strncpy(Next, O_Next, 255U);

   if ((*Next == '-') OR (*Next == '+')
#ifdef SLASHARGS
    OR (*Next == '/')
#endif
    OR (*Next == '@'))
    *Next = '\0';
   if (*Param == '@')
    BEGIN
     if (AllowLink)
      BEGIN
       ProcessFile(Param + 1, Def, Cnt, ErrProc);
       return CMDOK;
      END
     else
      BEGIN
       fprintf(stderr, "%s\n", catgetmessage(&MsgCat, Num_ErrMsgNoKeyInFile));
       return CMDErr;
      END
    END
   if ((*Param == '-')
#ifdef SLASHARGS
    OR (*Param == '/')
#endif
    OR (*Param == '+')) 
    BEGIN
     Negate=(*Param=='+'); Start=1;

     if (Param[Start]=='#')
      BEGIN
       for (z=Start+1; z<(int)strlen(Param); z++) Param[z]=mytoupper(Param[z]);
       Start++;
      END
     else if (Param[Start]=='~')
      BEGIN
       for (z=Start+1; z<(int)strlen(Param); z++) Param[z]=mytolower(Param[z]);
       Start++;
      END

     TempRes=CMDOK;

     Search=0;
     strncpy(s,Param+Start,255);
     for (z=0; z<(int)strlen(s); z++) s[z]=mytoupper(s[z]);
     for (Search=0; Search<Cnt; Search++)
      if ((strlen(Def[Search].Ident)>1) AND (strcmp(s,Def[Search].Ident)==0)) break;
     if (Search<Cnt) 
      TempRes=Def[Search].Callback(Negate,Next);

     else
      for (z=Start; z<(int)strlen(Param); z++) 
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
{
   int z;
   char *EnvStr[256], *start, *p;
   int EnvCnt=0;

   ClrBlanks(OneLine); 
   if ((*OneLine != '\0') AND (*OneLine != ';'))
   {
     start = OneLine;
     while (*start != '\0')
     {
       EnvStr[EnvCnt++] = start;
       p=strchr(start, ' '); if (p == Nil) p = strchr(start, '\t');
       if (p != Nil)
       {
         *p = '\0'; start = p + 1;
         while (isspace((unsigned int) *start))
           start++;
       }
       else
         start += strlen(start);
     }
     EnvStr[EnvCnt] = start;

     for (z = 0; z < EnvCnt; z++)
      switch (ProcessParam(Def, Cnt, EnvStr[z], EnvStr[z + 1], False, ErrProc))
      {
        case CMDFile:
          AddStringListLast(&FileArgList, EnvStr[z]);
          break;
        case CMDErr:
          ErrProc(True, EnvStr[z]);
          break;
        case CMDArg:
          z++;
          break;
        case CMDOK:
          break;
      }
   }
}

	static void ProcessFile(char *Name_O, CMDRec *Def, Integer Cnt, CMDErrCallback ErrProc)
BEGIN
   FILE *KeyFile;
   String Name, OneLine;

   strmaxcpy(Name, Name_O, 255);
   ClrBlanks(OneLine);

   KeyFile=fopen(Name, "r");
   if (KeyFile == Nil) ErrProc(True, catgetmessage(&MsgCat, Num_ErrMsgKeyFileNotFound));
   while (NOT feof(KeyFile))
    BEGIN
     errno = 0; ReadLn(KeyFile, OneLine);
     if ((errno != 0) AND (NOT feof(KeyFile)))
      ErrProc(True, catgetmessage(&MsgCat, Num_ErrMsgKeyFileError));
     DecodeLine(Def, Cnt, OneLine, ErrProc);
    END
   fclose(KeyFile);
END

	void ProcessCMD(CMDRec *Def, Integer Cnt, CMDProcessed Unprocessed,
                        char *EnvName, CMDErrCallback ErrProc)
BEGIN
   int z;
   String OneLine;

   if (getenv(EnvName)==Nil) OneLine[0]='\0';
   else strncpy(OneLine,getenv(EnvName),255);

   if (OneLine[0]=='@')
     ProcessFile(OneLine + 1, Def, Cnt, ErrProc);
   else DecodeLine(Def,Cnt,OneLine,ErrProc);

   for (z = 0; z <= ParamCount; z++)
    Unprocessed[z] = (z != 0);
   for (z = 1; z <= ParamCount; z++)
    if (Unprocessed[z])
     switch (ProcessParam(Def, Cnt, ParamStr[z], (z < ParamCount) ? ParamStr[z + 1] : "",
                          True, ErrProc))
      BEGIN
       case CMDErr: ErrProc(False,ParamStr[z]); break;
       case CMDOK:  Unprocessed[z]=False; break;
       case CMDArg: Unprocessed[z]=Unprocessed[z+1]=False; break;
       case CMDFile: AddStringListLast(&FileArgList, ParamStr[z]); break;
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
   InitStringList(&FileArgList);
   opencatalog(&MsgCat,"cmdarg.msg",ProgPath,MsgId1,MsgId2);
END

