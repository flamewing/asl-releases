/* asmsub.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterfunktionen, vermischtes                                              */
/*                                                                           */
/* Historie:  4. 5. 1996  Grundsteinlegung                                   */
/*                                                                           */
/*****************************************************************************/

typedef void (*TSwitchProc)(void);


extern Word ErrorCount,WarnCount;


extern void AsmSubInit(void);


extern long GTime(void);


extern CPUVar AddCPU(char *NewName, TSwitchProc Switcher);

extern Boolean AddCPUAlias(char *OrigName, char *AliasName);

extern void PrintCPUList(TSwitchProc NxtProc);

extern void ClearCPUList(void);


extern void UpString(char *s);

extern char *QuotPos(char *s, char Zeichen);

extern char *RQuotPos(char *s, char Zeichen);

extern char *FirstBlank(char *s);

extern void SplitString(char *Source, char *Left, char *Right, char *Trenner);

extern void KillBlanks(char *s);

extern void KillPrefBlanks(char *s);

extern void KillPostBlanks(char *s);

extern void TranslateString(char *s);

extern ShortInt StrCmp(char *s1, char *s2, LongInt Hand1, LongInt Hand2);

#define Memo(s) (strcmp(OpPart,(s))==0)


extern void AddSuffix(char *s, char *Suff);

extern void KillSuffix(char *s);

extern char *NamePart(char *Name);


extern char *FloatString(Double f);

extern void StrSym(TempResult *t, Boolean WithSystem, char *Dest);


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


extern void CompressLine(char *TokNam, Byte Num, char *Line);

extern void ExpandLine(char *TokNam, Byte Num, char *Line);

extern void KillCtrl(char *Line);


extern void AddCopyright(char *NewLine);

extern void WriteCopyrights(TSwitchProc NxtProc);


extern Boolean ChkSymbName(char *sym);

extern Boolean ChkMacSymbName(char *sym);


extern void WrErrorString(char *Message, char *Add, Boolean Warning, Boolean Fatal);


extern void WrError(Word Num);

extern void WrXError(Word Num, char *Message);

extern Boolean ChkRange(LargeInt Value, LargeInt Min, LargeInt Max);


extern void ChkIO(Word ErrNo);


extern void AddIncludeList(char *NewPath);

extern void RemoveIncludeList(char *RemPath);


extern void ClearOutList(void);

extern void AddToOutList(char *NewName);

extern void RemoveFromOutList(char *OldName);

extern char *GetFromOutList(void);


extern long DTime(long t1, long t2);


extern void asmsub_init(void);
