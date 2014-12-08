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

typedef void (*TSwitchProc)(
#ifdef __PROTOS__
void
#endif
);


extern Word ErrorCount,WarnCount;


extern void AsmSubInit(void);


extern long GTime(void);


extern CPUVar AddCPU(char *NewName, TSwitchProc Switcher);

extern Boolean AddCPUAlias(char *OrigName, char *AliasName);

extern void PrintCPUList(TSwitchProc NxtProc);

extern void ClearCPUList(void);


extern void UpString(char *s);

extern char *QuotPos(const char *s, char Zeichen);

extern char *RQuotPos(char *s, char Zeichen);

extern char *FirstBlank(char *s);

extern void SplitString(char *Source, char *Left, char *Right, char *Trenner);

extern void TranslateString(char *s, int Length);

extern ShortInt StrCmp(const char *s1, const char *s2, LongInt Hand1, LongInt Hand2);

extern ShortInt StrCaseCmp(const char *s1, const char *s2, LongInt Hand1, LongInt Hand2);

#ifdef PROFILE_MEMO
static inline Boolean Memo(const char *s)
{
  NumMemo++;
  return !strcmp(OpPart, s);
}
#else
# define Memo(s) (!strcmp(OpPart,(s)))
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


extern LargeWord ProgCounter(void);

extern LargeWord EProgCounter(void);

extern Word Granularity(void);

extern Word ListGran(void);

extern void ChkSpace(Byte Space);


extern void PrintChunk(ChunkList *NChunk);

extern void PrintUseList(void);

extern void ClearUseList(void);


extern void CompressLine(char *TokNam, Byte Num, char *Line, Boolean CompressLine);

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


extern Boolean ChkRange(LargeInt Value, LargeInt Min, LargeInt Max);


extern void ChkIO(Word ErrNo);
extern void ChkXIO(Word ErrNo, const char *pPath);


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
