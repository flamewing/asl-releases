#ifndef _ASMPARS_H
#define _ASMPARS_H
/* asmpars.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Symbolen und das ganze Drumherum...                        */
/*                                                                           */
/*****************************************************************************/

#include "symbolsize.h"

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
  Int64   ,
#endif
  IntTypeCnt
} IntType;

typedef struct
{
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

struct sStrComp;
struct sRelocEntry;

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


extern Boolean IdentifySection(char *Name, LongInt *Erg);


extern Boolean ExpandSymbol(char *Name);

extern void EnterIntSymbolWithFlags(const char *Name_O, LargeInt Wert, Byte Typ, Boolean MayChange, tSymbolFlags Flags);

#define EnterIntSymbol(Name_O, Wert, Typ, MayChange) EnterIntSymbolWithFlags(Name_O, Wert, Typ, MayChange, 0)

extern void EnterExtSymbol(const char *Name_O, LargeInt Wert, Byte Typ, Boolean MayChange);

extern void EnterRelSymbol(const char *Name_O, LargeInt Wert, Byte Typ, Boolean MayChange);

extern void EnterFloatSymbol(const char *Name_O, Double Wert, Boolean MayChange);

extern void EnterStringSymbol(const char *Name_O, const char *pValue, Boolean MayChange);

extern void EnterDynStringSymbol(const char *Name_O, const tDynString *pValue, Boolean MayChange);

extern Boolean GetIntSymbol(const char *Name, LargeInt *Wert, struct sRelocEntry **Relocs);

extern Boolean GetFloatSymbol(const char *Name, Double *Wert);

extern Boolean GetStringSymbol(const char *Name, char *Wert);

extern void PrintSymbolList(void);

extern void PrintDebSymbols(FILE *f);

extern void PrintNoISymbols(FILE *f);

extern void PrintSymbolTree(void);

extern void ClearSymbolList(void);

extern void ResetSymbolDefines(void);

extern void PrintSymbolDepth(void);


extern void SetSymbolOrStructElemSize(const char *Name, ShortInt Size);

extern ShortInt GetSymbolSize(const char *Name);

extern Boolean IsSymbolFloat(const char *Name);

extern Boolean IsSymbolString(const char *Name);

extern Boolean IsSymbolDefined(const char *Name);

extern Boolean IsSymbolUsed(const char *Name);

extern Boolean IsSymbolChangeable(const char *Name);

extern Integer GetSymbolType(const char *Name);

extern void EvalExpression(const char *pExpr, TempResult *Erg);

extern void EvalStrExpression(const struct sStrComp *pExpr, TempResult *pErg);

extern LargeInt EvalStrIntExpressionWithFlags(const struct sStrComp *pExpr, IntType Type, Boolean *pResult, tSymbolFlags *pFlags);
#define EvalStrIntExpression(pExpr, Type, pResult) EvalStrIntExpressionWithFlags(pExpr, Type, pResult, NULL)

extern LargeInt EvalStrIntExpressionOffsWithFlags(const struct sStrComp *pExpr, int Offset, IntType Type, Boolean *pResult, tSymbolFlags *pFlags);
#define EvalStrIntExpressionOffs(pExpr, Offset, Type, pResult) EvalStrIntExpressionOffsWithFlags(pExpr, Offset, Type, pResult, NULL)

extern Double EvalStrFloatExpression(const struct sStrComp *pExpr, FloatType Typ, Boolean *pResult);

extern void EvalStrStringExpression(const struct sStrComp *pExpr, Boolean *pResult, char *pEvalResult);


extern Boolean PushSymbol(char *SymName_O, char *StackName_O);

extern Boolean PopSymbol(char *SymName_O, char *StackName_O);

extern void ClearStacks(void);
 

extern void EnterFunction(char *FName, char *FDefinition, Byte NewCnt);

extern PFunction FindFunction(char *Name);

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

extern void SetLstMacroExp(tLstMacroExp NewMacroExp);


extern LongInt GetLocHandle(void);

extern void PushLocHandle(LongInt NewLoc);

extern void PopLocHandle(void);

extern void ClearLocStack(void);


extern void AddRegDef(char *Orig, char *Repl);

extern Boolean FindRegDef(const char *Name, char **Erg);

extern void TossRegDefs(LongInt Sect);

extern void CleanupRegDefs(void);

extern void ClearRegDefs(void);

extern void PrintRegDefs(void);


extern void ClearCodepages(void);

extern void PrintCodepages(void);


extern void asmpars_init(void);

#endif /* _ASMPARS_H */
