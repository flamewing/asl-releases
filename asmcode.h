/* asmcode.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung der Code-Datei                                                 */
/*                                                                           */
/* Historie: 18. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

extern Word LenSoFar;
extern LongInt RecPos;


extern void DreheCodes(void);

extern void NewRecord(LargeWord NStart);

extern void OpenFile(void);

extern void CloseFile(void);

extern void WriteBytes(void);

extern void RetractWords(Word Cnt);
 
extern void asmcode_init(void);
