#ifndef _BPEMU_H
#define _BPEMU_H
/* bpemu.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Emulation einiger Borland-Pascal-Funktionen                               */                      
/*                                                                           */
/* Historie: 20. 5.1996 Grundsteinlegung                                     */                      
/*                                                                           */
/*****************************************************************************/

typedef void (*charcallback)(
#ifdef __PROTOS__
char *Name
#endif
);

extern char *FExpand(char *Src);

extern char *FSearch(const char *File, char *Path);

extern long FileSize(FILE *file);

extern Byte Lo(Word inp);

extern Byte Hi(Word inp);

extern LongWord HiWord(LongWord Src);

extern Boolean Odd (int inp);

extern Boolean DirScan(char *Mask, charcallback callback);

extern LongInt MyGetFileTime(char *Name);

#ifdef __CYGWIN32__
extern char *DeCygWinDirList(char *pStr);

extern char *DeCygwinPath(char *pStr);
#endif

extern void bpemu_init(void);
#endif /* _BPEMU_H */
