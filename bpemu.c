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

#ifdef __MINGW32__
#include <direct.h>
#endif

char *FExpand(char *Src)
{
  static String CurrentDir;
  String Copy;
#ifdef DRSEP
  String DrvPart;
#endif /* DRSEP */
  char *p, *p2;

  strmaxcpy(Copy, Src, 255);

#ifdef DRSEP
  p = strchr(Copy,DRSEP);
  if (p)
  {
    memcpy(DrvPart, Copy, p - Copy);
    DrvPart[p - Copy] = '\0';
    strmov(Copy, p + 1);
  }
  else
    *DrvPart = '\0';
#endif

#if (defined __MSDOS__)
  {
    int DrvNum;

    if (*DrvPart == '\0')
    {
      DrvNum = getdisk();
      *DrvPart = DrvNum + 'A';
      DrvPart[1] = '\0';
      DrvNum++;
    }
    else
      DrvNum = toupper(*DrvPart) - '@';
    getcurdir(DrvNum, CurrentDir);
  }
#elif (defined __EMX__) || (defined __IBMC__)
  {
    ULONG DrvNum, Dummy;

    if (*DrvPart == '\0')
    {
      DosQueryCurrentDisk(&DrvNum, &Dummy);
      *DrvPart = DrvNum + '@';
      DrvPart[1] = '\0';
    }
    else
      DrvNum = toupper(*DrvPart) - '@';
    Dummy = 255;
    DosQueryCurrentDir(DrvNum, (PBYTE) CurrentDir, &Dummy);
  }
#elif (defined __MINGW32__)
  {
    int DrvNum;
 
    if (!*DrvPart)
    {
      DrvNum = _getdrive();
      *DrvPart = DrvNum + '@';
      DrvPart[1] = '\0';
    }
    else
      DrvNum = toupper(*DrvPart) - '@';
    _getdcwd(DrvNum, CurrentDir, 255);
    if (CurrentDir[1] == ':')
      strmov(CurrentDir, CurrentDir + 2);
  }
#elif (defined _WIN32) /* CygWIN */
  if (!getcwd(CurrentDir, 255))
    0[CurrentDir] = '\0';
  for (p = CurrentDir; *p; p++)
    if (*p == '/') *p = '\\';
#else /* UNIX */
  if (!getcwd(CurrentDir, 255))
    0[CurrentDir] = '\0';
#endif

  if ((*CurrentDir) && (CurrentDir[strlen(CurrentDir) - 1] != PATHSEP))
    strmaxcat(CurrentDir, SPATHSEP, 255);
  if (*CurrentDir!=PATHSEP)
    strmaxprep(CurrentDir, SPATHSEP, 255);

  if (*Copy == PATHSEP) 
  {
    strmaxcpy(CurrentDir, SPATHSEP, 255);
    strmov(Copy, Copy + 1);
  }

#ifdef DRSEP
#ifdef __CYGWIN32__
  /* win32 getcwd() does not deliver current drive letter, therefore only prepend a drive letter
     if there was one before. */
  if (*DrvPart)
#endif
  {
    strmaxprep(CurrentDir, SDRSEP, 255);
    strmaxprep(CurrentDir, DrvPart, 255);
  }
#endif

  while (True)
  {
    p = strchr(Copy, PATHSEP);
    if (!p)
      break;
    *p = '\0';
    if (!strcmp(Copy, "."));
    else if ((!strcmp(Copy, "..")) && (strlen(CurrentDir) > 1))
    {
      CurrentDir[strlen(CurrentDir) - 1] = '\0';
      p2 = strrchr(CurrentDir, PATHSEP); p2[1] = '\0';
    }
    else
    {
      strmaxcat(CurrentDir, Copy, 255);
      strmaxcat(CurrentDir, SPATHSEP, 255);
    }
    strmov(Copy, p + 1);
  }

  strmaxcat(CurrentDir, Copy, 255);

  return CurrentDir; 
}

char *FSearch(const char *File, char *Path)
{
  static String Component;
  char *p, *start, Save = '\0';
  FILE *Dummy;
  Boolean OK;  

  Dummy = fopen(File,"r");
  OK = (Dummy != NULL);
  if (OK)
  {
    fclose(Dummy);
    strmaxcpy(Component, File, 255);
    return Component;
  }

  start = Path;
  do
  {
    if (*start == '\0')
      break;
    p = strchr(start,DIRSEP);
    if (p) 
    {
      Save = *p;
      *p = '\0';
    }
    strmaxcpy(Component, start, 255);
#ifdef __CYGWIN32__
    DeCygwinPath(Component);
#endif
    strmaxcat(Component, SPATHSEP, 255);
    strmaxcat(Component, File, 255);
    if (p)
      *p = Save;
    Dummy = fopen(Component, "r");
    OK = Dummy != NULL;
    if (OK)
    {
      fclose(Dummy);
      return Component;
    }
    start = p + 1;
  }
  while (p);

  *Component='\0';
  return Component;
}

long FileSize(FILE *file)
{
  long Save = ftell(file), Size;

  fseek(file, 0, SEEK_END); 
  Size=ftell(file);
  fseek(file, Save, SEEK_SET);
  return Size;
}

Byte Lo(Word inp)
{
  return (inp & 0xff);
}

Byte Hi(Word inp)
{
  return ((inp >> 8) & 0xff);
}

LongWord HiWord(LongWord Src)
{
  return ((Src >> 16) & 0xffff);
}

Boolean Odd(int inp)
{
  return ((inp & 1) == 1);
}

Boolean DirScan(char *Mask, charcallback callback)
{
  char Name[1024];

#ifdef __MSDOS__
  struct ffblk blk;
  int res;
  char *pos;

  res = findfirst(Mask, &blk, FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL | FA_DIREC | FA_ARCH);
  if (res < 0)
    return False;
  pos = strrchr(Mask, PATHSEP);
  if (!pos)
    pos = strrchr(Mask, DRSEP);
  pos = pos ? pos + 1 : Mask;
  memcpy(Name, Mask, pos - Mask);
  while (res==0)
  {
    if ((blk.ff_attrib & (FA_LABEL|FA_DIREC)) == 0)
    {
      strcpy(Name + (pos - Mask), blk.ff_name);
      callback(Name);
    }
    res = findnext(&blk);
  }
  return True;
#else
#if defined ( __EMX__ ) || defined ( __IBMC__ )
  HDIR hdir = 1;
  FILEFINDBUF3 buf;
  ULONG rescnt;
  USHORT res;
  char *pos;

  rescnt = 1;
  res = DosFindFirst(Mask, &hdir, 0x16, &buf, sizeof(buf), &rescnt, 1);
  if (res)
    return False;
  pos = strrchr(Mask, PATHSEP);
  if (!pos)
    pos = strrchr(Mask, DRSEP);
  pos = pos ? pos + 1 : Mask;
  memcpy(Name, Mask, pos - Mask);
  while (res == 0)
  {
    strcpy(Name + (pos - Mask), buf.achName);
    callback(Name);
    res = DosFindNext(hdir, &buf, sizeof(buf), &rescnt);
  }
  return True;
#else
  strmaxcpy(Name, Mask, 255);
  callback(Name);
  return True;
#endif
#endif
}

LongInt MyGetFileTime(char *Name)
{
  struct stat st;

  if (stat(Name, &st) == -1)
    return 0;
  else
    return st.st_mtime;
}

#ifdef __CYGWIN32__

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
    strmov(pStr, pStr + 1);
    pStr[0] = pStr[1];
    pStr[1] = ':';
  }

  if ((strlen(pStr) >= 4)
   && (pStr[0] =='\\') && (pStr[1] == '\\') && (pStr[3] == '\\')
   && (isalpha(pStr[2])))
  {
    strmov(pStr, pStr + 1);
    pStr[0] = pStr[1];
    pStr[1] = ':';
  }

  for (pRun = pStr; *pRun; pRun++)
    if (*pRun == '/')
      *pRun = '\\';

  return pStr;
}
#endif /* __CYGWIN32__ */

void bpemu_init(void)
{
}
