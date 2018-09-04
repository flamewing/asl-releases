#ifndef _ASMSUB_H
#define _ASMSUB_H
/* asmsub.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterfunktionen, vermischtes                                              */
/*                                                                           */
/* Historie:  4. 5. 1996  Grundsteinlegung                                   */
/* Historie: 13. 8.1997 KillBlanks-Funktionen nach stringutil.c geschoben    */
/*           17. 8.1998 Unterfunktion zur Buchhaltung Adressbereiche         */
/*           18. 4.1999 Ausgabeliste Sharefiles                              */
/*           13. 2.2000 Ausgabeliste Listing                                 */
/*                                                                           */
/*****************************************************************************/

struct sLineComp;
struct sStrComp;

typedef void (*TSwitchProc)(
#ifdef __PROTOS__
void
#endif
);

extern Word ErrorCount,WarnCount;


extern void AsmSubInit(void);


extern long GTime(void);


extern void UpString(char *s);

extern char *QuotPos(const char *s, char Zeichen);
extern char *QuotMultPos(const char *s, const char *pSearch);

extern char *RQuotPos(char *s, char Zeichen);

extern char *FirstBlank(const char *s);

extern void SplitString(char *Source, char *Left, char *Right, char *Trenner);

extern void TranslateString(char *s, int Length);

extern ShortInt StrCaseCmp(const char *s1, const char *s2, LongInt Hand1, LongInt Hand2);

#ifdef PROFILE_MEMO
static inline Boolean Memo(const char *s)
{
  NumMemo++;
  return !strcmp(OpPart.Str, s);
}
#else
# define Memo(s) (!strcmp(OpPart.Str,(s)))
#endif


extern void AddSuffix(char *s, char *Suff);

extern void KillSuffix(char *s);

extern char *NamePart(char *Name);

extern char *PathPart(char *Name);


extern char *FloatString(Double f);

extern void StrSym(TempResult *t, Boolean WithSystem, char *Dest, int DestLen);


extern void ResetPageCounter(void);

extern void NewPage(ShortInt Level, Boolean WithFF);

extern void WrLstLine(char *Line);

extern void SetListLineVal(TempResult *t);

extern void PrintOneLineMuted(FILE *pFile, const char *pLine,
                              const struct sLineComp *pMuteComponent,
                              const struct sLineComp *pMuteComponent2);
extern void PrLineMarker(FILE *pFile, const char *pLine, const char *pPrefix, const char *pTrailer,
                         char Marker, const struct sLineComp *pLineComp);
extern void GenLineForMarking(char *pDest, unsigned DestSize, const char *pSrc, const char *pPrefix);
extern void GenLineMarker(char *pDest, unsigned DestSize, char Marker, const struct sLineComp *pLineComp, const char *pPrefix);

extern LargeWord ProgCounter(void);

extern LargeWord EProgCounter(void);

extern Word Granularity(void);

extern Word ListGran(void);

extern void ChkSpace(Byte Space);


extern void PrintUseList(void);

extern void ClearUseList(void);


extern int CompressLine(char *TokNam, Byte Num, char *Line, Boolean CompressLine);

extern void ExpandLine(char *TokNam, Byte Num, char *Line);

extern void KillCtrl(char *Line);

#ifdef __TURBOC__
extern void ChkStack(void);

extern void ResetStack(void);

extern LongWord StackRes(void);
#else
#define ChkStack() {}
#define ResetStack() {}
#define StackRes() 0
#endif


extern void AddCopyright(char *NewLine);

extern void WriteCopyrights(TSwitchProc NxtProc);


extern Boolean ChkSymbName(char *sym);

extern Boolean ChkMacSymbName(char *sym);


extern void ChkIO(Word ErrNo);
extern void ChkStrIO(Word ErrNo, const struct sStrComp *pComp);


extern void AddIncludeList(char *NewPath);

extern void RemoveIncludeList(char *RemPath);


extern void ClearOutList(void);

extern void AddToOutList(const char *NewName);

extern void RemoveFromOutList(const char *OldName);

extern char *GetFromOutList(void);


extern void ClearShareOutList(void);

extern void AddToShareOutList(const char *NewName);

extern void RemoveFromShareOutList(const char *OldName);

extern char *GetFromShareOutList(void);


extern void ClearListOutList(void);

extern void AddToListOutList(const char *NewName);

extern void RemoveFromListOutList(const char *OldName);

extern char *GetFromListOutList(void);


extern void BookKeeping(void);


extern long DTime(long t1, long t2);


extern void InitPass(void);
extern void AddInitPassProc(SimpProc NewProc);

extern void ClearUp(void);
extern void AddClearUpProc(SimpProc NewProc);

extern void asmsub_init(void);

#include "asmerr.h"

#endif /* _ASMSUB_H */
