#ifndef _ASMFNUMS_H
#define _ASMFNUMS_H
/* asmfnums.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Datei-Nummern                                              */
/*                                                                           */
/* Historie: 15. 5.96 Grundsteinlegung                                       */
/*                                                                           */
/*****************************************************************************/

extern void InitFileList(void);

extern void ClearFileList(void);

extern void AddFile(char *FName);

extern Integer GetFileNum(char *Name);

extern char *GetFileName(int Num);

extern Integer GetFileCount(void);

extern void AddAddressRange(int File, LargeWord Start, LargeWord Len);

extern void GetAddressRange(int File, LargeWord *Start, LargeWord *End);

extern void ResetAddressRanges(void);

extern void asmfnums_init(void);
#endif /* _ASMFNUMS_H */
