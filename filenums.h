/* filenums.h */
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

extern char *GetFileName(Byte Num);

extern Integer GetFileCount(void);

extern void filenums_init(void);
