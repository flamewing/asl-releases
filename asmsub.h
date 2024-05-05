#ifndef ASMSUB_H
#define ASMSUB_H
/* asmsub.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterfunktionen, vermischtes                                              */
/*                                                                           */
/*****************************************************************************/

#include "asmdef.h"
#include "datatypes.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define LISTLINESPACE 20

struct sLineComp;
struct sStrComp;
struct as_dynstr;

typedef void (*TSwitchProc)(void);

extern void AsmSubPassInit(void);

extern long GTime(void);

extern void UpString(char* s);

extern char* QuotPosQualify(
        char const* s, char Zeichen, tQualifyQuoteFnc QualifyQuoteFnc);
#define QuotPos(s, Zeichen) QuotPosQualify(s, Zeichen, NULL)
extern char* QuotMultPosQualify(
        char const* s, char const* pSearch, tQualifyQuoteFnc QualifyQuoteFnc);
#define QuotMultPos(s, pSearch) QuotMultPosQualify(s, pSearch, NULL)
extern char* QuotSMultPosQualify(
        char const* s, char const* pStrs, tQualifyQuoteFnc QualifyQuoteFnc);
#define QuotSMultPos(s, pStrs) QuotSMultPosQualify(s, pStrs, NULL)

extern char* RQuotPos(char* s, char Zeichen);

extern char* FirstBlank(char const* s);

extern void SplitString(char* Source, char* Left, char* Right, char* Trenner);

extern void TranslateString(char* s, int Length);

extern ShortInt StrCaseCmp(char const* s1, char const* s2, LongInt Hand1, LongInt Hand2);

extern char* MatchChars(char const* pStr, char const* pPattern, ...);
extern char* MatchCharsRev(char const* pStr, char const* pPattern, ...);

extern char* FindClosingParenthese(char const* pStr);

extern char* FindOpeningParenthese(
        char const* pStrBegin, char const* pStrEnd, char const Bracks[2]);

#ifdef PROFILE_MEMO
static inline Boolean Memo(char const* s) {
    NumMemo++;
    return !strcmp(OpPart.str.p_str, s);
}
#else
#    define Memo(s) (!strcmp(OpPart.str.p_str, (s)))
#endif

extern void AddSuffix(char* s, char const* Suff);

extern void KillSuffix(char* s);

extern char const* NamePart(char const* Name);

extern char* PathPart(char* Name);

extern void FloatString(char* pDest, size_t DestSize, Double f);

extern void StrSym(
        TempResult const* t, Boolean WithSystem, struct as_dynstr* p_dest,
        unsigned Radix);

extern void ResetPageCounter(void);

extern void NewPage(ShortInt Level, Boolean WithFF);

extern void WrLstLine(char const* Line);

extern void SetListLineVal(TempResult* t);

extern void LimitListLine(void);

extern void PrintOneLineMuted(
        FILE* pFile, char const* pLine, const struct sLineComp* pMuteComponent,
        const struct sLineComp* pMuteComponent2);
extern void PrLineMarker(
        FILE* pFile, char const* pLine, char const* pPrefix, char const* pTrailer,
        char Marker, const struct sLineComp* pLineComp);

extern LargeWord ProgCounter(void);

extern LargeWord EProgCounter(void);

extern Word Granularity(void);

extern Word ListGran(void);

extern void ChkSpace(Byte AddrSpace, unsigned AddrSpaceMask);

extern void PrintUseList(void);

extern void ClearUseList(void);

extern int CompressLine(
        char const* TokNam, unsigned TokenNum, struct as_dynstr* p_str,
        Boolean CaseSensitive);

extern void ExpandLine(char const* TokNam, unsigned TokenNum, struct as_dynstr* p_str);

extern void KillCtrl(char* Line);

extern void AddCopyright(char const* NewLine);

extern void WriteCopyrights(TSwitchProc NxtProc);

extern char*   ChkSymbNameUpTo(char const* pSym, char const* pUpTo);
extern Boolean ChkSymbName(char const* pSym);

extern char*   ChkMacSymbNameUpTo(char const* pSym, char const* pUpTo);
extern Boolean ChkMacSymbName(char const* pSym);

extern unsigned visible_strlen(char const* pSym);

extern void AddIncludeList(char* NewPath);

extern void RemoveIncludeList(char* RemPath);

extern void ClearOutList(void);

extern void AddToOutList(char const* NewName);

extern void RemoveFromOutList(char const* OldName);

extern char* GetFromOutList(void);

extern void ClearShareOutList(void);

extern void AddToShareOutList(char const* NewName);

extern void RemoveFromShareOutList(char const* OldName);

extern char* GetFromShareOutList(void);

extern void ClearListOutList(void);

extern void AddToListOutList(char const* NewName);

extern void RemoveFromListOutList(char const* OldName);

extern char* GetFromListOutList(void);

extern void BookKeeping(void);

extern long DTime(long t1, long t2);

extern void InitPass(void);
extern void AddInitPassProc(SimpProc NewProc);

extern void ClearUp(void);
extern void AddClearUpProc(SimpProc NewProc);

extern void asmsub_init(void);

#include "asmerr.h"

#endif /* ASMSUB_H */
