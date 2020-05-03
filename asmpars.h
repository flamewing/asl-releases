#ifndef _ASMPARS_H
#define _ASMPARS_H
/* asmpars.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Symbolen und das ganze Drumherum...                        */
/*                                                                           */
/*****************************************************************************/

#include "symbolsize.h"
#include "tempresult.h"

typedef enum
{
  UInt1    ,
  UInt2    ,
  UInt3    ,
  SInt4    , UInt4   , Int4    ,
  SInt5    , UInt5   , Int5    ,
  SInt6    , UInt6   ,
  SInt7    , UInt7   ,
  SInt8    , UInt8   , Int8    ,
  SInt9    , UInt9    ,
  UInt10   , Int10   ,
  UInt11   ,
  UInt12   , Int12   ,
  UInt13   ,
  UInt14   , Int14   ,
  SInt15   , UInt15  ,
  SInt16   , UInt16  , Int16   ,
  UInt17   ,
  UInt18   ,
  UInt19   ,
  SInt20   , UInt20  , Int20   ,
  UInt21   ,
  UInt22   ,
  UInt23   ,
  SInt24   , UInt24  , Int24   ,
  SInt32   , UInt32  , Int32   ,
#ifdef HAS64
  SInt64   , UInt64  , Int64   ,
#endif
  IntTypeCnt
} IntType;

#ifdef HAS64
#define LargeWordType UInt64
#else
#define LargeWordType UInt32
#endif

typedef struct
{
  Word SignAndWidth;
  LargeWord Mask;
  LargeInt Min, Max;
} tIntTypeDef;

typedef enum
{
  Float32,
  Float64,
  Float80,
  FloatDec,
  FloatCo,
  FloatTypeCnt
} FloatType;

typedef enum
{
  eSymbolFlag_None = 0,
  eSymbolFlag_NextLabelAfterBSR = 1 << 0
} tSymbolFlags;

typedef struct _TFunction
{
  struct _TFunction *Next;
  Byte ArguCnt;
  StringPtr Name, Definition;
} TFunction, *PFunction;

struct sStrComp;
struct sRelocEntry;
struct sSymbolEntry;

extern tIntTypeDef IntTypeDefs[IntTypeCnt];
extern Boolean FirstPassUnknown;
extern Boolean SymbolQuestionable;
extern Boolean UsesForwards;
extern LongInt MomLocHandle;
extern LongInt TmpSymCounter,
               FwdSymCounter,
               BackSymCounter;
extern char TmpSymCounterVal[10];
extern LongInt LocHandleCnt;
extern LongInt MomLocHandle;


extern void AsmParsInit(void);

extern void InitTmpSymbols(void);

extern Boolean SingleBit(LargeInt Inp, LargeInt *Erg);


extern IntType GetSmallestUIntType(LargeWord MaxValue);


extern Boolean RangeCheck(LargeInt Wert, IntType Typ);

extern Boolean FloatRangeCheck(Double Wert, FloatType Typ);


extern Boolean IdentifySection(const struct sStrComp *pName, LongInt *Erg);


extern Boolean ExpandStrSymbol(char *pDest, unsigned DestSize, const struct sStrComp *pSrc);

extern void ChangeSymbol(struct sSymbolEntry *pEntry, LargeInt Value);

extern struct sSymbolEntry *EnterIntSymbolWithFlags(const struct sStrComp *pName, LargeInt Wert, Byte Typ, Boolean MayChange, tSymbolFlags Flags);

#define EnterIntSymbol(pName, Wert, Typ, MayChange) EnterIntSymbolWithFlags(pName, Wert, Typ, MayChange, 0)

extern void EnterExtSymbol(const struct sStrComp *pName, LargeInt Wert, Byte Typ, Boolean MayChange);

extern struct sSymbolEntry *EnterRelSymbol(const struct sStrComp *pName, LargeInt Wert, Byte Typ, Boolean MayChange);

extern void EnterFloatSymbol(const struct sStrComp *pName, Double Wert, Boolean MayChange);

extern void EnterStringSymbol(const struct sStrComp *pName, const char *pValue, Boolean MayChange);

extern void EnterDynStringSymbol(const struct sStrComp *pName, const tDynString *pValue, Boolean MayChange);

extern void LookupSymbol(const struct sStrComp *pName, TempResult *pValue, Boolean WantRelocs, TempType ReqType);

extern void PrintSymbolList(void);

extern void PrintDebSymbols(FILE *f);

extern void PrintNoISymbols(FILE *f);

extern void PrintSymbolTree(void);

extern void ClearSymbolList(void);

extern void ResetSymbolDefines(void);

extern void PrintSymbolDepth(void);


extern void SetSymbolOrStructElemSize(const struct sStrComp *pName, ShortInt Size);

extern ShortInt GetSymbolSize(const struct sStrComp *pName);

extern Boolean IsSymbolDefined(const struct sStrComp *pName);

extern Boolean IsSymbolUsed(const struct sStrComp *pName);

extern Boolean IsSymbolChangeable(const struct sStrComp *pName);

extern Integer GetSymbolType(const struct sStrComp *pName);

extern void EvalExpression(const char *pExpr, TempResult *Erg);

extern void EvalStrExpression(const struct sStrComp *pExpr, TempResult *pErg);

extern LargeInt EvalStrIntExpressionWithFlags(const struct sStrComp *pExpr, IntType Type, Boolean *pResult, tSymbolFlags *pFlags);
#define EvalStrIntExpression(pExpr, Type, pResult) EvalStrIntExpressionWithFlags(pExpr, Type, pResult, NULL)

extern LargeInt EvalStrIntExpressionOffsWithFlags(const struct sStrComp *pExpr, int Offset, IntType Type, Boolean *pResult, tSymbolFlags *pFlags);
#define EvalStrIntExpressionOffs(pExpr, Offset, Type, pResult) EvalStrIntExpressionOffsWithFlags(pExpr, Offset, Type, pResult, NULL)

extern Double EvalStrFloatExpression(const struct sStrComp *pExpr, FloatType Typ, Boolean *pResult);

extern void EvalStrStringExpression(const struct sStrComp *pExpr, Boolean *pResult, char *pEvalResult);


extern const char *GetIntelSuffix(unsigned Radix);


extern Boolean PushSymbol(const struct sStrComp *pSymName, const struct sStrComp *pStackName);

extern Boolean PopSymbol(const struct sStrComp *pSymName, const struct sStrComp *pStackName);

extern void ClearStacks(void);
 

extern void EnterFunction(const struct sStrComp *pComp, char *FDefinition, Byte NewCnt);

extern PFunction FindFunction(const char *Name);

extern void PrintFunctionList(void);

extern void ClearFunctionList(void);


extern void AddDefSymbol(char *Name, TempResult *Value);

extern void RemoveDefSymbol(char *Name);

extern void CopyDefSymbols(void);

extern const TempResult *FindDefSymbol(const char *pName);

extern void PrintCrossList(void);

extern void ClearCrossList(void);


extern LongInt GetSectionHandle(char *SName_O, Boolean AddEmpt, LongInt Parent);

extern char *GetSectionName(LongInt Handle);

extern void SetMomSection(LongInt Handle);

extern void AddSectionUsage(LongInt Start, LongInt Length);

extern void ClearSectionUsage(void);

extern void PrintSectionList(void);

extern void PrintDebSections(FILE *f);

extern void ClearSectionList(void);


extern void SetFlag(Boolean *Flag, const char *Name, Boolean Wert);

extern LongInt GetLocHandle(void);

extern void PushLocHandle(LongInt NewLoc);

extern void PopLocHandle(void);

extern void ClearLocStack(void);


extern void AddRegDef(const struct sStrComp *pOrigComp, const struct sStrComp *pReplComp);

extern Boolean FindRegDef(const char *Name, char **Erg);

extern void TossRegDefs(LongInt Sect);

extern void CleanupRegDefs(void);

extern void ClearRegDefs(void);

extern void PrintRegDefs(void);


extern void ClearCodepages(void);

extern void PrintCodepages(void);


extern void asmpars_init(void);

#endif /* _ASMPARS_H */
