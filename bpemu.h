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

extern char *FSearch(char *File, char *Path);

extern long FileSize(FILE *file);

extern Byte Lo(Word inp);

extern Byte Hi(Word inp);

extern Boolean Odd (int inp);

extern Boolean DirScan(char *Mask, charcallback callback);

extern LongInt GetFileTime(char *Name);

extern void bpemu_init(void);
#endif /* _BPEMU_H */
