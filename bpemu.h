/* bpemu.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Emulation einiger Borland-Pascal-Funktionen                               */                      
/*                                                                           */
/* Historie: 20. 5.1996 Grundsteinlegung                                     */                      
/*                                                                           */
/*****************************************************************************/

extern char *FExpand(char *Src);

extern char *FSearch(char *File, char *Path);

extern long FileSize(FILE *file);

extern Byte Lo(Word inp);

extern Byte Hi(Word inp);

extern Boolean Odd (int inp);

extern void bpemu_init(void);
