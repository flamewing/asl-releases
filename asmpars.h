#ifndef _ASMPARS_H
#define _ASMPARS_H
/* asmpars.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Symbolen und das ganze Drumherum...                        */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*           26. 6.1998 Codepages                                            */
/*           16. 8.1998 NoICE-Symbolausgabe                                  */
/*            6.12.1998 UInt14                                               */
/*           12. 7.1999 angefangen mit externen Symbolen                     */
/*           21. 5.2000 added TmpSymCounter                                  */
/*           24. 5.2001 added UInt21 type                                    */
/*            3. 8.2001 added SInt6 type                                     */
/*           2001-10-20 added UInt23                                         */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmpars.h,v 1.12 2017/04/02 11:10:36 alfred Exp $                     */
/***************************************************************************** 
 * $Log: asmpars.h,v $
 * Revision 1.12  2017/04/02 11:10:36  alfred
 * - allow more fine-grained macro expansion in listing
 *
 * Revision 1.11  2017/02/26 16:57:48  alfred
 * - make some arguments const
 *
 * Revision 1.10  2016/10/07 20:03:04  alfred
 * - make some arguments const
 *
 * Revision 1.9  2016/09/29 16:43:36  alfred
 * - introduce common DecodeDATA/DecodeRES functions
 *
 * Revision 1.8  2015/08/28 17:22:27  alfred
 * - add special handling for labels following BSR
 *
 * Revision 1.7  2014/11/30 10:09:54  alfred
 * - rework to current style
 *
 * Revision 1.6  2009/04/10 08:58:31  alfred
 * - correct address ranges for AVRs
 *
 * Revision 1.5  2008/11/23 10:39:16  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.4  2005/10/02 10:00:44  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2004/05/30 20:51:43  alfred
 * - major cleanups in Const... functions
 *
 * Revision 1.2  2004/05/28 16:12:08  alfred
 * - added some const definitions
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.5  2003/02/26 19:18:26  alfred
 * - add/use EvalIntDisplacement()
 *
 * Revision 1.4  2002/10/07 20:25:01  alfred
 * - added '/' nameless temporary symbols
 *
 * Revision 1.3  2002/09/29 17:05:41  alfred
 * - ass +/- temporary symbols
 *
 * Revision 1.2  2002/05/19 13:44:52  alfred
 * - added ClearSectionUsage()
 *
 *****************************************************************************/
           
typedef enum
{
  UInt1    ,
  UInt2    ,
  UInt3    ,
  SInt4    ,UInt4    , Int4    ,
  SInt5    ,UInt5    , Int5    ,
  SInt6    ,UInt6    ,
  SInt7    ,UInt7    ,
  SInt8    ,UInt8    , Int8    ,
  UInt9    ,
  UInt10   , Int10   ,
  UInt11   ,
  UInt12   , Int12   ,
  UInt13   ,
  UInt14   , Int14   ,
  UInt15   ,
  SInt16   ,UInt16   , Int16   ,
  UInt17   ,
  UInt18   ,
  SInt20   ,UInt20   , Int20   ,
  UInt21   ,
  UInt22   ,
  UInt23   ,
  SInt24   ,UInt24   , Int24   ,
  SInt32   ,UInt32   , Int32   ,
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


extern LargeInt ConstIntVal(const char *pExpr, IntType Typ, Boolean *pResult);

extern Double ConstFloatVal(const char *pExpr, FloatType Typ, Boolean *pResult);

extern void ConstStringVal(const char *pExpr, tDynString *pDest, Boolean *pResult);


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

extern Boolean GetIntSymbol(const char *Name, LargeInt *Wert, PRelocEntry *Relocs);

extern Boolean GetFloatSymbol(const char *Name, Double *Wert);

extern Boolean GetStringSymbol(const char *Name, char *Wert);

extern void PrintSymbolList(void);

extern void PrintDebSymbols(FILE *f);

extern void PrintNoISymbols(FILE *f);

extern void PrintSymbolTree(void);

extern void ClearSymbolList(void);

extern void ResetSymbolDefines(void);

extern void PrintSymbolDepth(void);


extern void SetSymbolSize(const char *Name, ShortInt Size);

extern ShortInt GetSymbolSize(const char *Name);

extern Boolean IsSymbolFloat(const char *Name);

extern Boolean IsSymbolString(const char *Name);

extern Boolean IsSymbolDefined(const char *Name);

extern Boolean IsSymbolUsed(const char *Name);

extern Boolean IsSymbolChangeable(const char *Name);

extern Integer GetSymbolType(const char *Name);

extern void EvalExpression(const char *pExpr, TempResult *Erg);

extern LargeInt EvalIntExpressionWithFlags(const char *pExpr, IntType Type, Boolean *pResult, tSymbolFlags *pFlags);

#define EvalIntExpression(pExpr, Type, pResult) EvalIntExpressionWithFlags(pExpr, Type, pResult, NULL)

extern LargeInt EvalIntDisplacement(char *pExpr, IntType Type, Boolean *pResult);

extern Double EvalFloatExpression(const char *pExpr, FloatType Typ, Boolean *pResult);

extern void EvalStringExpression(const char *pExpr, Boolean *pResult, char *pEvalResult);


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
