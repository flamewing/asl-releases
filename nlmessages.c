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

static char *IdentString = "AS Message Catalog - not readable\n\032\004";

static char *EOpenMsg = "cannot open msg file %s";
static char *ERdMsg = "cannot read from msg file";
static char *EIndMsg = "string table index error";

static TMsgCat DefaultCatalog =
{
  NULL, NULL, 0
};

/*****************************************************************************/

static void error(char *Msg)
{
  fprintf(stderr, "message catalog handling: %s - program terminated\n", Msg);
  exit(255);
}

char *catgetmessage(PMsgCat Catalog, int Num)
{
  if ((Num >= 0) && (Num < Catalog->MsgCount))
    return Catalog->MsgBlock + Catalog->StrPosis[Num];
  else
  {
    static char *umess = NULL;

    if (!umess)
      umess = (char*)malloc(sizeof(char) * STRINGSIZE);
    sprintf(umess, "catgetmessage: message number %d does not exist", Num);
    return umess;
  }
}

char *getmessage(int Num)
{
  return catgetmessage(&DefaultCatalog, Num);
}

FILE *myopen(const char *name, LongInt MsgId1, LongInt MsgId2)
{
  FILE *tmpfile;
  String line;
  LongInt RId1, RId2;
  Boolean EForm = False;

  tmpfile = fopen(name, OPENRDMODE);
  if (!tmpfile)
    return NULL;
  if (fread(line, 1, strlen(IdentString), tmpfile) != strlen(IdentString))
    EForm = True;
  if (memcmp(line, IdentString, strlen(IdentString)))
    EForm = True;
  if (!Read4(tmpfile, &RId1))
    EForm = True;
  if (RId1 != MsgId1)
    EForm = True;
  if (!Read4(tmpfile, &RId2))
    EForm = True;
  if (RId2 != MsgId2)
    EForm = True;
  if (EForm)
  {
    fclose(tmpfile);
    fprintf(stderr, "message catalog handling: warning: %s has invalid format or is out of date\n", name);
    return NULL;
  }
  else
    return tmpfile;
}

#define MSGPATHNAME "AS_MSGPATH"

void opencatalog(PMsgCat Catalog, const char *File, const char *Path, LongInt MsgId1, LongInt MsgId2)
{
  FILE *MsgFile;
  char str[2048], *ptr;
  const char *pSep;
#if defined(DOS_NLS) || defined (OS2_NLS)
  NLS_CountryInfo NLSInfo;
#else
  char *lcstring;
#endif
  LongInt DefPos = -1, MomPos, DefLength = 0, MomLength, z, StrStart, CtryCnt, Ctrys[100];
  Boolean fi, Gotcha;

  /* get reference for finding out which language set to use */

#if defined(DOS_NLS) || defined (OS2_NLS)
  NLS_GetCountryInfo(&NLSInfo);
#else
  lcstring = getenv("LC_MESSAGES");
  if (!lcstring)
    lcstring = getenv("LC_ALL");
  if (!lcstring)
    lcstring = getenv("LANG");
  if (!lcstring)
    lcstring = "";
#endif

  /* find first valid message file */

  MsgFile = myopen(File, MsgId1, MsgId2);
  if (!MsgFile)
  {
    if (*Path != '\0')
    {
#ifdef __CYGWIN32__
      for (ptr = Path; *ptr != '\0'; ptr++)
        if (*ptr == '/') *ptr = '\\';
#endif
      pSep = strrchr(Path, PATHSEP);
      if (!pSep)
        pSep = Path + strlen(Path);
      memcpy(str, Path, pSep - Path); str[pSep - Path] = '\0';
      strcat(str, SPATHSEP);
      strcat(str, File);
      MsgFile = myopen(str, MsgId1, MsgId2);
    }
    if (!MsgFile)
    {
      ptr = getenv(MSGPATHNAME);
      if (ptr)
      {
        sprintf(str, "%s/%s", ptr, File);
        MsgFile = myopen(str, MsgId1, MsgId2);
      }
      else
      {
        ptr = getenv("PATH");
        if (!ptr)
          MsgFile = NULL;
        else
        {
          strmaxcpy(str, ptr, 255);
#ifdef __CYGWIN32__
          DeCygWinDirList(str);
#endif
          ptr = FSearch(File, str);
          MsgFile = (*ptr != '\0') ? myopen(ptr, MsgId1, MsgId2) : NULL;
        }
      }
      if (!MsgFile)
      {
        sprintf(str, "%s/%s", LIBDIR, File);
        MsgFile = myopen(str, MsgId1, MsgId2);
        if (!MsgFile)
        {
          sprintf(str, EOpenMsg, File);
          error(str);
        }
      }
    }
  }

  Gotcha = False;
  do
  {
    ptr = str;
    do
    {
      if (fread(ptr, 1, 1, MsgFile) != 1)
        error(ERdMsg);
      fi = (*ptr == '\0');
      if (!fi) ptr++;
    }
    while (!fi);
    if (*str != '\0')
    {
      if (!Read4(MsgFile, &MomLength))
        error(ERdMsg);
      if (!Read4(MsgFile, &CtryCnt))
        error(ERdMsg);
      for (z = 0; z < CtryCnt; z++)
        if (!Read4(MsgFile, Ctrys + z))
          error(ERdMsg);
      if (!Read4(MsgFile, &MomPos))
        error(ERdMsg);
      if (DefPos == -1)
      {
        DefPos = MomPos;
        DefLength = MomLength;
      }
#if defined(DOS_NLS) || defined (OS2_NLS)
      for (z = 0; z < CtryCnt; z++)
        if (Ctrys[z] == NLSInfo.Country)
          Gotcha = True;
#else
      Gotcha = !strncasecmp(lcstring, str, strlen(str));
#endif
    }
  }
  while ((*str != '\0') && (!Gotcha));
  if (*str == '\0')
  {
    MomPos = DefPos;
    MomLength = DefLength;
  }

  /* read pointer table */

  fseek(MsgFile, MomPos, SEEK_SET);
  if (!Read4(MsgFile, &StrStart))
    error(ERdMsg);
  Catalog->MsgCount = (StrStart - MomPos) >> 2;
  Catalog->StrPosis = (LongInt *) malloc(sizeof(LongInt)*Catalog->MsgCount);
  Catalog->StrPosis[0] = 0;
  if (fread(Catalog->StrPosis + 1, 4, Catalog->MsgCount - 1, MsgFile) + 1 != Catalog->MsgCount)
    error(ERdMsg);
  if (BigEndian)
    DSwap(Catalog->StrPosis + 1, (Catalog->MsgCount - 1) << 2);
  for (z = 1; z < Catalog->MsgCount; z++)
  {
    Catalog->StrPosis[z] -= StrStart;
    if ((Catalog->StrPosis[z] < 0) || (Catalog->StrPosis[z] >= MomLength))
      error(EIndMsg);
  }

  /* read string table */

  fseek(MsgFile, StrStart, SEEK_SET);
  Catalog->MsgBlock = (char *) malloc(MomLength);
  if (fread(Catalog->MsgBlock, 1, MomLength, MsgFile) != MomLength)
    error(ERdMsg);

  fclose(MsgFile);
}

void nlmessages_init(char *File, char *ProgPath, LongInt MsgId1, LongInt MsgId2)
{
  opencatalog(&DefaultCatalog, File, ProgPath, MsgId1, MsgId2);
}
