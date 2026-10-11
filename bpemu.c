/* bpemu.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Emulation einiger Borland-Pascal-Funktionen                               */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "bpemu.h"

#include "strutil.h"

#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef __MSDOS__
#    include <dir.h>
#    include <dos.h>
#endif

#if defined(__EMX__) || defined(__IBMC__)
#    include <os2.h>
#endif

#ifdef __MINGW32__
#    include <direct.h>
#endif

void FExpand(char* p_dest, size_t dest_size, char const* p_src) {
    String Copy;
#ifdef DRSEP
    String DrvPart;
#endif /* DRSEP */
    char *p, *p2;

    strmaxcpy(Copy, p_src, sizeof(Copy));

#ifdef DRSEP
    p = strchr(Copy, DRSEP);
    if (p) {
        memcpy(DrvPart, Copy, p - Copy);
        DrvPart[p - Copy] = '\0';
        strmov(Copy, p + 1);
    } else {
        *DrvPart = '\0';
    }
#endif

#if (defined __MSDOS__)
    {
        int DrvNum;

        if (*DrvPart == '\0') {
            DrvNum     = getdisk();
            *DrvPart   = DrvNum + 'A';
            DrvPart[1] = '\0';
            DrvNum++;
        } else {
            DrvNum = toupper(*DrvPart) - '@';
        }
        getcurdir(DrvNum, p_dest);
    }
#elif (defined __EMX__) || (defined __IBMC__)
    {
        ULONG DrvNum, Dummy;

        if (*DrvPart == '\0') {
            DosQueryCurrentDisk(&DrvNum, &Dummy);
            *DrvPart   = DrvNum + '@';
            DrvPart[1] = '\0';
        } else {
            DrvNum = toupper(*DrvPart) - '@';
        }
        Dummy = 255;
        DosQueryCurrentDir(DrvNum, (PBYTE)p_dest, &Dummy);
    }

#elif (defined _WIN32)
#    if (defined __MINGW32__) || (defined _MSC_VER)

    {
        int DrvNum;

        if (!*DrvPart) {
            DrvNum     = _getdrive();
            *DrvPart   = DrvNum + '@';
            DrvPart[1] = '\0';
        } else {
            DrvNum = toupper(*DrvPart) - '@';
        }
        _getdcwd(DrvNum, p_dest, dest_size);
        if (p_dest[1] == ':') {
            strmov(p_dest, p_dest + 2);
        }
    }

#    else /* CygWIN */
    if (!getcwd(p_dest, dest_size)) {
        0 [p_dest] = '\0';
    }
    for (p = p_dest; *p; p++) {
        if (*p == '/') {
            *p = '\\';
        }
    }

#    endif
#else /* UNIX */
    if (!getcwd(p_dest, dest_size)) {
        0 [p_dest] = '\0';
    }
#endif

    if ((*p_dest) && (p_dest[strlen(p_dest) - 1] != PATHSEP)) {
        strmaxcat(p_dest, SPATHSEP, dest_size);
    }
    if (*p_dest != PATHSEP) {
        strmaxprep(p_dest, SPATHSEP, dest_size);
    }

    if (*Copy == PATHSEP) {
        strmaxcpy(p_dest, SPATHSEP, dest_size);
        strmov(Copy, Copy + 1);
    }

#ifdef DRSEP
#    ifdef __CYGWIN32__
    /* win32 getcwd() does not deliver current drive letter, therefore only prepend a
       drive letter if there was one before. */
    if (*DrvPart)
#    endif
    {
        strmaxprep(p_dest, SDRSEP, dest_size);
        strmaxprep(p_dest, DrvPart, dest_size);
    }
#endif

    while (True) {
        p = strchr(Copy, PATHSEP);
        if (!p) {
            break;
        }
        *p = '\0';
        if (!strcmp(Copy, "."))
            ;
        else if ((!strcmp(Copy, "..")) && (strlen(p_dest) > 1)) {
            p_dest[strlen(p_dest) - 1] = '\0';
            p2                         = strrchr(p_dest, PATHSEP);
            p2[1]                      = '\0';
        } else {
            strmaxcat(p_dest, Copy, dest_size);
            strmaxcat(p_dest, SPATHSEP, dest_size);
        }
        strmov(Copy, p + 1);
    }

    strmaxcat(p_dest, Copy, dest_size);
}

/*!------------------------------------------------------------------------
 * \fn     FSearch(char *pDest, size_t DestSize, const char *pFileToSearch, const char *pCurrFileName, const char *pSearchPath)
 * \brief  search for file in given path(s)
 * \param  pDest where to put result
 * \param  DestSize size of result buffer
 * \param  pFileToSearch file to search for
 * \param  pCurrFileName file this file was referenced from
 * \param  pSearchPath list of directories to search
 * \return 0 if found or error code
 * ------------------------------------------------------------------------ */

static int AssembleAndCheck(
        char* pDest, size_t DestSize, char const* pPath, unsigned PathLen,
        char const* pFileToSearch) {
    FILE* pDummy;

    if (PathLen > DestSize - 1) {
        PathLen = DestSize - 1;
    }
    if (pPath) {
        memcpy(pDest, pPath, PathLen);
    } else {
        PathLen = 0;
    }
    pDest[PathLen] = '\0';
#ifdef __CYGWIN32__
    DeCygwinPath(pDest);
#endif
    if (PathLen > 0) {
        strmaxcat(pDest, SPATHSEP, DestSize);
    }
    strmaxcat(pDest, pFileToSearch, DestSize);
    pDummy = fopen(pDest, "r");
    if (pDummy) {
        fclose(pDummy);
        return 0;
    } else {
        return 2;
    }
}

int FSearch(
        char* pDest, size_t DestSize, char const* pFileToSearch,
        char const* pCurrFileName, char const* pSearchPath) {
    /* If the file has an absolute path ('/....', '\....', 'X:....'), do not search
       relative to current file's directory: */

    Boolean     Absolute = (*pFileToSearch == '/');
    char const *pPos, *pStart;

#if 0
  fprintf(stderr, "FSearch(..., %u, \"%s\", \"%s\", \"%s\")\n",
          (unsigned)DestSize, pFileToSearch, pCurrFileName, pSearchPath);
#endif

#if (defined _WIN32) || (defined __EMX__) || (defined __IBMC__) || (defined __MSDOS__)
    if (*pFileToSearch == PATHSEP) {
        Absolute = True;
    }
#endif
#ifdef DRSEP
    if ((as_islower(*pFileToSearch) || as_isupper(*pFileToSearch))
        && (pFileToSearch[1] == DRSEP)) {
        Absolute = True;
    }
#endif

    if (pCurrFileName && !Absolute) {
#if (defined _WIN32) || (defined __EMX__) || (defined __IBMC__) || (defined __MSDOS__)
        /* On systems with \ as path separator, we may get a mixture of / and \ in the
           path. Assure we find the last one of either: */

        pPos = strrmultchr(pCurrFileName, SPATHSEP "/");
#else
        pPos = strrchr(pCurrFileName, PATHSEP);
#endif
        if (!AssembleAndCheck(
                    pDest, DestSize, pCurrFileName, pPos ? pPos - pCurrFileName : 0,
                    pFileToSearch)) {
            return 0;
        }
    } else {
        if (!AssembleAndCheck(pDest, DestSize, NULL, 0, pFileToSearch)) {
            return 0;
        }
    }

    /* TODO: if the file has an absolute path, searching the include path should be
     * pointless: */

    pStart = pSearchPath;
    while (True) {
        pPos = strchr(pStart, DIRSEP);

        if (!AssembleAndCheck(
                    pDest, DestSize, pStart, pPos ? pPos - pStart : (int)strlen(pStart),
                    pFileToSearch)) {
            return 0;
        }
        if (pPos) {
            pStart = pPos + 1;
        } else {
            break;
        }
    }

    *pDest = '\0';
    return 2;
}

long FileSize(FILE* file) {
    long Save = ftell(file), Size;

    if ((Save < 0) || (fseek(file, 0, SEEK_END) != 0)) {
        return -1;
    }
    Size = ftell(file);
    if (fseek(file, Save, SEEK_SET) != 0) {
        return -1;
    }
    return Size;
}

Byte Lo(Word inp) {
    return (inp & 0xff);
}

Byte Hi(Word inp) {
    return ((inp >> 8) & 0xff);
}

unsigned LoWord(LongWord Src) {
    return (Src & 0xffff);
}

unsigned HiWord(LongWord Src) {
    return ((Src >> 16) & 0xffff);
}

unsigned long LoDWord(LargeWord Src) {
    return Src & 0xfffffffful;
}

Boolean Odd(int inp) {
    return ((inp & 1) == 1);
}

/* Instruct MinGW to perform wildcard expansion, since CMD.EXE
   does not expand wildcards, unlike a UNIX shell.  An alternate
   implementation may make use of glob():
   glob_t glob = NULL;
   glob(pattern, GLOB_NOCHECK, NULL, &glob);
   ....
   globfree(&glob);
 */

#if (defined _WIN32) && (defined __MINGW32__)
int _dowildcard = -1;
#endif

Boolean DirScan(char const* Mask, charcallback callback) {
    char Name[1024];

#ifdef __MSDOS__
    struct ffblk blk;
    int          res;
    char const*  pos;

    res = findfirst(
            Mask, &blk,
            FA_RDONLY | FA_HIDDEN | FA_SYSTEM | FA_LABEL | FA_DIREC | FA_ARCH);
    if (res < 0) {
        return False;
    }
    pos = strrchr(Mask, PATHSEP);
    if (!pos) {
        pos = strrchr(Mask, DRSEP);
    }
    pos = pos ? pos + 1 : Mask;
    memcpy(Name, Mask, pos - Mask);
    while (res == 0) {
        if ((blk.ff_attrib & (FA_LABEL | FA_DIREC)) == 0) {
            strcpy(Name + (pos - Mask), blk.ff_name);
            callback(Name);
        }
        res = findnext(&blk);
    }
    return True;
#else
#    if defined(__EMX__) || defined(__IBMC__)
    HDIR         hdir = 1;
    FILEFINDBUF3 buf;
    ULONG        rescnt;
    USHORT       res;
    char*        pos;

    rescnt = 1;
    res    = DosFindFirst(Mask, &hdir, 0x16, &buf, sizeof(buf), &rescnt, 1);
    if (res) {
        return False;
    }
    pos = strrchr(Mask, PATHSEP);
    if (!pos) {
        pos = strrchr(Mask, DRSEP);
    }
    pos = pos ? pos + 1 : Mask;
    memcpy(Name, Mask, pos - Mask);
    while (res == 0) {
        strcpy(Name + (pos - Mask), buf.achName);
        callback(Name);
        res = DosFindNext(hdir, &buf, sizeof(buf), &rescnt);
    }
    return True;
#    else
    strmaxcpy(Name, Mask, sizeof(Name));
    callback(Name);
    return True;
#    endif
#endif
}

LongInt MyGetFileTime(char* Name) {
    struct stat st;

    if (stat(Name, &st) == -1) {
        return 0;
    } else {
        return st.st_mtime;
    }
}

#ifdef __CYGWIN32__

/* convert CygWin-style paths back to something usable by other Win32 apps */

char* DeCygWinDirList(char* pStr) {
    char* pRun;

    for (pRun = pStr; *pRun; pRun++) {
        if (*pRun == ':') {
            *pRun = ';';
        }
    }

    return pStr;
}

char* DeCygwinPath(char* pStr) {
    char* pRun;

    if ((strlen(pStr) >= 4) && (pStr[0] == '/') && (pStr[1] == '/') && (pStr[3] == '/')
        && (isalpha(pStr[2]))) {
        strmov(pStr, pStr + 1);
        pStr[0] = pStr[1];
        pStr[1] = ':';
    }

    if ((strlen(pStr) >= 4) && (pStr[0] == '\\') && (pStr[1] == '\\') && (pStr[3] == '\\')
        && (isalpha(pStr[2]))) {
        strmov(pStr, pStr + 1);
        pStr[0] = pStr[1];
        pStr[1] = ':';
    }

    for (pRun = pStr; *pRun; pRun++) {
        if (*pRun == '/') {
            *pRun = '\\';
        }
    }

    return pStr;
}
#endif /* __CYGWIN32__ */

/*!------------------------------------------------------------------------
 * \fn     as_fsize(const char *p_path, LargeWord *p_size)
 * \brief  retrieve file size
 * \param  p_path path to file
 * \param  p_size return value buffer
 * \return 0 if succeeded
 * ------------------------------------------------------------------------ */

int as_fsize(char const* p_path, LargeWord* p_size) {
    int ret;
#ifdef _WIN32
    struct _stat status;
    ret = _stat(p_path, &status);
#else
    struct stat status;
    ret = stat(p_path, &status);
#endif
    if (!ret) {
        *p_size = status.st_size;
    }
    return ret;
}

void bpemu_init(void) {}
