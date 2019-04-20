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
{
  int cnt;

  for (cnt = 0; isspace((unsigned int) tmp[cnt]); cnt++);
  if (cnt > 0)
    strmov(tmp, tmp + cnt);
}

Boolean ProcessedEmpty(CMDProcessed Processed)
{
  int z;
   
  for (z = 1; z <= ParamCount; z++)
    if (Processed[z])
      return False;
   return True; 
}

static void ProcessFile(char *Name_O, CMDRec *Def, Integer Cnt, CMDErrCallback ErrProc);

static CMDResult ProcessParam(CMDRec *Def, Integer Cnt, char *O_Param,
                              char *O_Next, Boolean AllowLink,
                              CMDErrCallback ErrProc)
{
  int Start;
  Boolean Negate;
  int z, Search;
  CMDResult TempRes;
  String s, Param, Next;

  strmaxcpy(Param, O_Param, STRINGSIZE);
  strmaxcpy(Next, O_Next, STRINGSIZE);

  if ((*Next == '-')
   || (*Next == '+')
#ifdef SLASHARGS
   || (*Next == '/')
#endif
   || (*Next == '@'))
    *Next = '\0';
  if (*Param == '@')
  {
    if (AllowLink)
    {
      ProcessFile(Param + 1, Def, Cnt, ErrProc);
      return CMDOK;
    }
    else
    {
      fprintf(stderr, "%s\n", catgetmessage(&MsgCat, Num_ErrMsgNoKeyInFile));
      return CMDErr;
    }
  }
  if ((*Param == '-')
#ifdef SLASHARGS
   || (*Param == '/')
#endif
   || (*Param == '+')) 
  {
    Negate = (*Param == '+');
    Start = 1;

    if (Param[Start] == '#')
    {
      for (z = Start + 1; z < (int)strlen(Param); z++)
        Param[z] = mytoupper(Param[z]);
      Start++;
    }
    else if (Param[Start] == '~')
    {
      for (z = Start + 1; z < (int)strlen(Param); z++)
        Param[z] = mytolower(Param[z]);
      Start++;
    }

    TempRes = CMDOK;

    Search = 0;
    strmaxcpy(s, Param + Start, STRINGSIZE);
    for (z = 0; z < (int)strlen(s); z++)
      s[z] = mytoupper(s[z]);
    for (Search = 0; Search < Cnt; Search++)
      if ((strlen(Def[Search].Ident) > 1) && (!strcmp(s, Def[Search].Ident)))
        break;
    if (Search < Cnt) 
      TempRes = Def[Search].Callback(Negate,Next);

    else
    {
      for (z = Start; z < (int)strlen(Param); z++) 
        if (TempRes != CMDErr)
        {
          Search = 0;
          for (Search = 0; Search < Cnt; Search++)
            if ((strlen(Def[Search].Ident) == 1) && (Def[Search].Ident[0] == Param[z]))
              break;
          if (Search >= Cnt)
            TempRes = CMDErr;
          else
          {
            switch (Def[Search].Callback(Negate,Next))
            {
              case CMDErr:
                TempRes = CMDErr;
                break;
              case CMDArg:
                TempRes = CMDArg;
                break;
              case CMDOK:
                break;
              case CMDFile:
                break; /** **/
            }
          }
        }
    }
    return TempRes;
  }
  else
    return CMDFile;
}

static void DecodeLine(CMDRec *Def, Integer Cnt, char *OneLine,
                       CMDErrCallback ErrProc)
{
  int z;
  char *EnvStr[256], *start, *p;
  int EnvCnt = 0;

  ClrBlanks(OneLine); 
  if ((*OneLine != '\0') && (*OneLine != ';'))
  {
    start = OneLine;
    while (*start != '\0')
    {
      EnvStr[EnvCnt++] = start;
      p = strchr(start, ' ');
      if (!p)
        p = strchr(start, '\t');
      if (p)
      {
        *p = '\0';
        start = p + 1;
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
{
  FILE *KeyFile;
  String Name, OneLine;

  strmaxcpy(Name, Name_O, STRINGSIZE);
  ClrBlanks(OneLine);

  KeyFile = fopen(Name, "r");
  if (!KeyFile)
    ErrProc(True, catgetmessage(&MsgCat, Num_ErrMsgKeyFileNotFound));
  while (!feof(KeyFile))
  {
    errno = 0;
    ReadLn(KeyFile, OneLine);
    if ((errno != 0) && (!feof(KeyFile)))
      ErrProc(True, catgetmessage(&MsgCat, Num_ErrMsgKeyFileError));
    DecodeLine(Def, Cnt, OneLine, ErrProc);
  }
  fclose(KeyFile);
}

void ProcessCMD(CMDRec *Def, Integer Cnt, CMDProcessed Unprocessed,
                char *EnvName, CMDErrCallback ErrProc)
{
  int z;
  String OneLine;
  char *pEnv;

  pEnv = getenv(EnvName);
  strmaxcpy(OneLine, pEnv ? pEnv : "", STRINGSIZE);

  if (OneLine[0] == '@')
    ProcessFile(OneLine + 1, Def, Cnt, ErrProc);
  else
    DecodeLine(Def,Cnt,OneLine,ErrProc);

  for (z = 0; z <= ParamCount; z++)
    Unprocessed[z] = (z != 0);
  for (z = 1; z <= ParamCount; z++)
    if (Unprocessed[z])
      switch (ProcessParam(Def, Cnt, ParamStr[z], (z < ParamCount) ? ParamStr[z + 1] : "",
                           True, ErrProc))
      {
        case CMDErr:
          ErrProc(False, ParamStr[z]);
          break;
        case CMDOK:
          Unprocessed[z] = False;
          break;
        case CMDArg:
          Unprocessed[z] = Unprocessed[z + 1] = False;
          break;
        case CMDFile:
          AddStringListLast(&FileArgList, ParamStr[z]);
          break;
      }
}

const char *GetEXEName(void)
{
  char *pos;

  pos = strrchr(ParamStr[0], '/');
  return (pos) ? pos + 1 : ParamStr[0];
}

void cmdarg_init(char *ProgPath)
{
  InitStringList(&FileArgList);
  opencatalog(&MsgCat, "cmdarg.msg", ProgPath, MsgId1, MsgId2);
}

