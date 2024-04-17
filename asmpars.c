/* asmpars.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Symbolen und das ganze Drumherum...                        */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "as_endian.h"
#include "bpemu.h"
#include "nls.h"
#include "nlmessages.h"
#include "as.rsc"
#include "strutil.h"
#include "strcomp.h"

#include "asmdef.h"
#include "asmsub.h"
#include "errmsg.h"
#include "asmfnums.h"
#include "asmrelocs.h"
#include "asmstructs.h"
#include "chunks.h"
#include "trees.h"
#include "operator.h"
#include "function.h"
#include "intformat.h"
#include "ieeefloat.h"
#include "sha1.h"

#include "asmpars.h"

#define LOCSYMSIGHT 3       /* max. sight for nameless temporary symbols */

#define LEAVE  goto func_exit
#define LEAVE2 goto func_exit2

/* Mask, Min 6 Max are computed at initialization */

tIntTypeDef IntTypeDefs[IntTypeCnt] =
{
  { 0x0001, 0, 0, 0 }, /* UInt1 */
  { 0x0002, 0, 0, 0 }, /* UInt2 */
  { 0x0003, 0, 0, 0 }, /* UInt3 */
  { 0x8004, 0, 0, 0 }, /* SInt4 */
  { 0x0004, 0, 0, 0 }, /* UInt4 */
  { 0xc004, 0, 0, 0 }, /* Int4 */
  { 0x8005, 0, 0, 0 }, /* SInt5 */
  { 0x0005, 0, 0, 0 }, /* UInt5 */
  { 0xc005, 0, 0, 0 }, /* Int5 */
  { 0x8006, 0, 0, 0 }, /* SInt6 */
  { 0x0006, 0, 0, 0 }, /* UInt6 */
  { 0x8007, 0, 0, 0 }, /* SInt7 */
  { 0x0007, 0, 0, 0 }, /* UInt7 */
  { 0x8008, 0, 0, 0 }, /* SInt8 */
  { 0x0008, 0, 0, 0 }, /* UInt8 */
  { 0xc008, 0, 0, 0 }, /* Int8 */
  { 0x8009, 0, 0, 0 }, /* SInt9 */
  { 0x0009, 0, 0, 0 }, /* UInt9 */
  { 0x000a, 0, 0, 0 }, /* UInt10 */
  { 0xc00a, 0, 0, 0 }, /* Int10 */
  { 0x000b, 0, 0, 0 }, /* UInt11 */
  { 0x000c, 0, 0, 0 }, /* UInt12 */
  { 0xc00c, 0, 0, 0 }, /* Int12 */
  { 0x000d, 0, 0, 0 }, /* UInt13 */
  { 0x000e, 0, 0, 0 }, /* UInt14 */
  { 0xc00e, 0, 0, 0 }, /* Int14 */
  { 0x800f, 0, 0, 0 }, /* SInt15 */
  { 0x000f, 0, 0, 0 }, /* UInt15 */
  { 0x8010, 0, 0, 0 }, /* SInt16 */
  { 0x0010, 0, 0, 0 }, /* UInt16 */
  { 0xc010, 0, 0, 0 }, /* Int16 */
  { 0x0011, 0, 0, 0 }, /* UInt17 */
  { 0x0012, 0, 0, 0 }, /* UInt18 */
  { 0x0013, 0, 0, 0 }, /* UInt19 */
  { 0x8014, 0, 0, 0 }, /* SInt20 */
  { 0x0014, 0, 0, 0 }, /* UInt20 */
  { 0xc014, 0, 0, 0 }, /* Int20 */
  { 0x0015, 0, 0, 0 }, /* UInt21 */
  { 0x0016, 0, 0, 0 }, /* UInt22 */
  { 0x0017, 0, 0, 0 }, /* UInt23 */
  { 0x8018, 0, 0, 0 }, /* SInt24 */
  { 0x0018, 0, 0, 0 }, /* UInt24 */
  { 0xc018, 0, 0, 0 }, /* Int24 */
  { 0x801e, 0, 0, 0 }, /* SInt30 */
  { 0x001e, 0, 0, 0 }, /* UInt30 */
  { 0xc01e, 0, 0, 0 }, /* Int30 */
  { 0x8020, 0, 0, 0 }, /* SInt32 */
  { 0x0020, 0, 0, 0 }, /* UInt32 */
  { 0xc020, 0, 0, 0 }, /* Int32 */
#ifdef HAS64
  { 0x8040, 0, 0, 0 }, /* SInt64 */
  { 0x0040, 0, 0, 0 }, /* UInt64 */
  { 0xc040, 0, 0, 0 }, /* Int64 */
#endif
};

typedef struct
{
  Boolean Back;
  LongInt Counter;
} TTmpSymLog;

LongInt MomLocHandle;          /* Merker, den lokale Symbole erhalten        */
LongInt FwdSymCounter,
        BackSymCounter;
String TmpSymCounterVal;     /* representation as string                   */
TTmpSymLog TmpSymLog[LOCSYMSIGHT];
LongInt TmpSymLogDepth;

LongInt LocHandleCnt;          /* mom. verwendeter lokaler Handle            */

typedef struct sSymbolEntry
{
  TTree Tree;
  Boolean Defined, Used, Changeable;
  TempResult SymWert;
  PCrossRef RefList;
  Byte FileNum;
  LongInt LineNum;
} TSymbolEntry, *PSymbolEntry;

typedef struct sSymbolStackEntry
{
  struct sSymbolStackEntry *Next;
  TempResult Contents;
} TSymbolStackEntry, *PSymbolStackEntry;

typedef struct sSymbolStack
{
  struct sSymbolStack *Next;
  char *Name;
  PSymbolStackEntry Contents;
} TSymbolStack, *PSymbolStack;

typedef struct sDefSymbol
{
  struct sDefSymbol *Next;
  char *SymName;
  TempResult Wert;
} TDefSymbol, *PDefSymbol;

typedef struct sCToken
{
  struct sCToken *Next;
  char *Name;
  LongInt Parent;
  ChunkList Usage;
} TCToken, *PCToken;

typedef struct sLocHeap
{
  struct sLocHeap *Next;
  LongInt Cont;
} TLocHeap, *PLocHandle;

typedef struct sRegDefList
{
  struct sRegDefList *Next;
  LongInt Section;
  char *Value;
  Boolean Used;
} TRegDefList, *PRegDefList;

typedef struct sRegDef
{
  struct sRegDef *Left, *Right;
  char *Orig;
  PRegDefList Defs, DoneDefs;
} TRegDef, *PRegDef;

static PSymbolEntry FirstSymbol, FirstLocSymbol;
static PDefSymbol FirstDefSymbol;
/*static*/ PCToken FirstSection;
static Boolean DoRefs,              /* Querverweise protokollieren */
               RegistersDefined;
static PLocHandle FirstLocHandle;
static PSymbolStack FirstStack;
static PCToken MomSection;
static char *LastGlobSymbol;
static PFunction FirstFunction;	        /* Liste definierter Funktionen */

void AsmParsInit(void)
{
  FirstSymbol = NULL;

  FirstLocSymbol = NULL; MomLocHandle = -1; SetMomSection(-1);
  FirstSection = NULL;
  FirstLocHandle = NULL;
  FirstStack = NULL;
  FirstFunction = NULL;
  DoRefs = True;
  RadixBase = 10;
  OutRadixBase = 16;
  RegistersDefined = False;
}


Boolean RangeCheck(LargeInt Wert, IntType Typ)
{
#ifndef HAS64
  if (((int)Typ) >= ((int)SInt32))
    return True;
#else
  if (((int)Typ) >= ((int)SInt64))
    return True;
#endif
  else
    return ((Wert >= IntTypeDefs[(int)Typ].Min) && (Wert <= IntTypeDefs[(int)Typ].Max));
}

Boolean FloatRangeCheck(Double Wert, FloatType Typ)
{
  /* NaN/Infinity is representable in all formats */

  int numclass = as_fpclassify(Wert);
  if ((numclass == AS_FP_NAN) || (numclass == AS_FP_INFINITE))
    return True;

  switch (Typ)
  {
    case Float16:
      return (fabs(Wert) <= 65504.0);
    case Float32:
      return (fabs(Wert) <= 3.4e38);
    case Float64:
      return (fabs(Wert) <= 1.7e308);
/**     case FloatCo: return fabs(Wert) <= 9.22e18; */
    case Float80:
      return True;
    case FloatDec:
      return True;
    default:
      return False;
  }
/**   if (Typ == FloatDec) && (fabs(Wert) > 1e1000) WrError(ErrNum_BigDecFloat);**/
}

Boolean SingleBit(LargeInt Inp, LargeInt *Erg)
{
  *Erg = 0;
  do
  {
    if (!Odd(Inp))
      (*Erg)++;
    if (!Odd(Inp))
      Inp = Inp >> 1;
  }
  while ((*Erg != LARGEBITS) && (!Odd(Inp)));
  return (*Erg != LARGEBITS) && (Inp == 1);
}

IntType GetSmallestUIntType(LargeWord MaxValue)
{
  IntType Result;

  Result = (IntType) 0;
  for (Result = (IntType) 0; Result < IntTypeCnt; Result++)
  {
    if (IntTypeDefs[Result].Min < 0)
      continue;
    if (IntTypeDefs[Result].Max >= (LargeInt)MaxValue)
      return Result;
  }
  return UInt32;
}

IntType GetUIntTypeByBits(unsigned Bits)
{
  IntType Result;
  for (Result = (IntType) 0; Result < IntTypeCnt; Result++)
  {
    if (IntTypeDefs[Result].SignAndWidth & 0x8000)
      continue;
    if (Lo(IntTypeDefs[Result].SignAndWidth) == Bits)
      return Result;
  }
  fprintf(stderr, "define unsigned int type with %u bits\n", Bits);
  exit(255);
}

static Boolean ProcessBk(char **Start, char *Erg)
{
  LongInt System = 0, Acc = 0, Digit = 0;
  char ch;
  int cnt;
  Boolean Finish;

  switch (as_toupper(**Start))
  {
    case '\'': case '\\': case '"':
      *Erg = **Start;
      (*Start)++;
      return True;
    case 'H':
      *Erg = '\'';
      (*Start)++;
      return True;
    case 'I':
      *Erg = '"';
      (*Start)++;
    return True;
    case 'B':
      *Erg = Char_BS;
      (*Start)++;
      return True;
    case 'A':
      *Erg = Char_BEL;
      (*Start)++;
      return True;
    case 'E':
      *Erg = Char_ESC;
      (*Start)++;
       return True;
    case 'T':
      *Erg = Char_HT;
      (*Start)++;
       return True;
    case 'N':
      *Erg = Char_LF;
      (*Start)++;
      return True;
    case 'R':
      *Erg = Char_CR;
      (*Start)++;
      return True;
    case 'X':
      System = 16;
      (*Start)++;
      /* fall-through */
    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      if (System == 0)
        System = (**Start == '0') ? 8 : 10;
      cnt = (System == 16) ? 1 : ((System == 10) ? 0 : -1);
      do
      {
        ch = as_toupper(**Start);
        Finish = False;
        if ((ch >= '0') && (ch <= '9'))
          Digit = ch - '0';
        else if ((System == 16) && (ch >= 'A') && (ch <= 'F'))
          Digit = (ch - 'A') + 10;
        else
          Finish = True;
        if (!Finish)
        {
          (*Start)++;
          cnt++;
          if (Digit >= System)
          {
            WrError(ErrNum_OverRange);
            return False;
          }
          Acc = (Acc * System) + Digit;
        }
      }
      while ((!Finish) && (cnt < 3));
      if (!ChkRange(Acc, 0, 255))
        return False;
      *Erg = Acc;
      return True;
    default:
      WrError(ErrNum_InvEscSequence);
      return False;
  }
}

/*!------------------------------------------------------------------------
 * \fn     NonZString2Int(const struct as_nonz_dynstr *p_str)
 * \brief  convert string to its "ASCII representation"
 * \param  p_str string containing characters
 * \return -1 or converted int
 * ------------------------------------------------------------------------ */

LargeInt NonZString2Int(const struct as_nonz_dynstr *p_str)
{
  if ((p_str->len > 0) && (p_str->len <= 4))
  {
    const char *pRun;
    Byte Digit;
    LargeInt Result;

    Result = 0;
    for (pRun = p_str->p_str;
         pRun < p_str->p_str + p_str->len;
         pRun++)
    {
      Digit = (usint) *pRun;
      Result = (Result << 8) | CharTransTable[Digit & 0xff];
    }
    return Result;
  }
  return -1;
}

Boolean Int2NonZString(struct as_nonz_dynstr *p_str, LargeInt Src)
{
  int Search;
  Byte Digit;
  char *pDest;

  if (p_str->capacity < 32)
    as_nonz_dynstr_realloc(p_str, 32);
  p_str->len = 0;
  pDest = &p_str->p_str[p_str->capacity];
  while (Src && (p_str->len < p_str->capacity))
  {
    Digit = Src & 0xff;
    Src = (Src >> 8) & 0xfffffful;
    for (Search = 0; Search < 256; Search++)
      if (CharTransTable[Search] == Digit)
      {
        *(--pDest) = Search;
        p_str->len++;
        break;
      }
  }
  memmove(p_str->p_str, pDest, p_str->len);
  return True;
}

/*!------------------------------------------------------------------------
 * \fn     TempResultToInt(TempResult *pResult)
 * \brief  convert TempResult to integer
 * \param  pResult tempresult to convert
 * \return 0 or error code
 * ------------------------------------------------------------------------ */

int TempResultToInt(TempResult *pResult)
{
  switch (pResult->Typ)
  {
    case TempInt:
      break;
    case TempString:
    {
      LargeInt Result = NonZString2Int(&pResult->Contents.str);
      if (Result >= 0)
      {
        as_tempres_set_int(pResult, Result);
        break;
      }
      /* else */
    }
    /* fall-through */
    default:
      pResult->Typ = TempNone;
      return -1;
  }
  return 0;
}

/*!------------------------------------------------------------------------
 * \fn     MultiCharToInt(TempResult *pResult, unsigned MaxLen)
 * \brief  optionally convert multi-character constant to integer
 * \param  pResult holding value
 * \param  MaxLen maximum lenght of multi-character constant
 * \return True if converted
 * ------------------------------------------------------------------------ */

Boolean MultiCharToInt(TempResult *pResult, unsigned MaxLen)
{
  if ((pResult->Typ == TempString)
   && (pResult->Contents.str.len <= MaxLen)
   && (pResult->Flags & eSymbolFlag_StringSingleQuoted))
  {
    TempResultToInt(pResult);
    return True;
  }
  return False;
}

/*!------------------------------------------------------------------------
 * \fn     ExpandStrSymbol(char *pDest, size_t DestSize, const tStrComp *pSrc)
 * \brief  expand symbol name from string component
 * \param  pDest dest buffer
 * \param  DestSize size of dest buffer
 * \param  pSrc source component
 * \return True if success
 * ------------------------------------------------------------------------ */

Boolean ExpandStrSymbol(char *pDest, size_t DestSize, const tStrComp *pSrc)
{
  tStrComp SrcComp;
  const char *pStart;

  *pDest = '\0'; StrCompRefRight(&SrcComp, pSrc, 0);
  while (True)
  {
    pStart = strchr(SrcComp.str.p_str, '{');
    if (pStart)
    {
      unsigned ls = pStart - SrcComp.str.p_str, ld = strlen(pDest);
      String Expr, Result;
      tStrComp ExprComp;
      tEvalResult EvalResult;
      const char *pStop;

      if (ld + ls + 1 > DestSize)
        ls = DestSize - 1 - ld;
      memcpy(pDest + ld, SrcComp.str.p_str, ls);
      pDest[ld + ls] = '\0';

      pStop = QuotPos(pStart + 1, '}');
      if (!pStop)
      {
        WrStrErrorPos(ErrNum_InvSymName, pSrc);
        return False;
      }
      StrCompMkTemp(&ExprComp, Expr, sizeof(Expr));
      StrCompCopySub(&ExprComp, &SrcComp, pStart + 1 - SrcComp.str.p_str, pStop - pStart - 1);
      EvalStrStringExpressionWithResult(&ExprComp, &EvalResult, Result);
      if (!EvalResult.OK)
        return False;
      if (mFirstPassUnknown(EvalResult.Flags))
      {
        WrStrErrorPos(ErrNum_FirstPassCalc, &ExprComp);
        return False;
      }
      if (!CaseSensitive)
        UpString(Result);
      strmaxcat(pDest, Result, DestSize);
      StrCompIncRefLeft(&SrcComp, pStop + 1 - SrcComp.str.p_str);
    }
    else
    {
      strmaxcat(pDest, SrcComp.str.p_str, DestSize);
      return True;
    }
  }
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* check whether this is a local symbol and expand local counter if yes.  They
   have to be handled in different places of the parser, therefore two separate
   functions */

void InitTmpSymbols(void)
{
  FwdSymCounter = BackSymCounter = 0;
  *TmpSymCounterVal = '\0';
  TmpSymLogDepth = 0;
  *LastGlobSymbol = '\0';
}

static void AddTmpSymLog(Boolean Back, LongInt Counter)
{
  /* shift out oldest value */

  if (TmpSymLogDepth)
  {
    LongInt ShiftCnt = min(TmpSymLogDepth, LOCSYMSIGHT - 1);

    memmove(TmpSymLog + 1, TmpSymLog, sizeof(TTmpSymLog) * (ShiftCnt));
  }

  /* insert new one */

  TmpSymLog[0].Back = Back;
  TmpSymLog[0].Counter = Counter;
  if (TmpSymLogDepth < LOCSYMSIGHT)
    TmpSymLogDepth++;
}

static Boolean ChkTmp1(char *Name)
{
  char *Src, *Dest;
  Boolean Result = FALSE;

  /* $$-Symbols: append current $$-counter */

  if (!strncmp(Name, "$$", 2))
  {
    /* manually copy since this will implicitly give us the point to append
       the number */

    for (Src = Name + 2, Dest = Name; *Src; *(Dest++) = *(Src++));

    /* append number. only generate the number once */

    if (*TmpSymCounterVal == '\0')
    {
      SHA1_CTX sha;
      Byte results[20];

      SHA1Init(&sha);
      SHA1Update(&sha, (unsigned char *)LastGlobSymbol, strlen(LastGlobSymbol));
      SHA1Final(results, &sha);
      SHA1ToHexString(results, TmpSymCounterVal);
    }
    strcpy(Dest, TmpSymCounterVal);
    Result = TRUE;
  }

  return Result;
}

static Boolean ChkTmp2(char *pDest, const char *pSrc, as_symbol_source_t symbol_source)
{
  const char *pRun, *pBegin, *pEnd;
  int Cnt;
  Boolean Result = FALSE;

  for (pBegin = pSrc; as_isspace(*pBegin); pBegin++);
  for (pEnd = pSrc + strlen(pSrc); (pEnd > pBegin) && as_isspace(*(pEnd - 1)); pEnd--);

  /* Note: We have to deal with three symbol definitions:

      "-" for backward-only referencing
      "+" for forward-only referencing
      "/" for either way of referencing

      "/" and "+" are both expanded to forward symbol names, so the
      forward refencing to both types is unproblematic, however
      only "/" and "-" are stored in the backlog of the three
      most-recent symbols for backward referencing.
  */

  /* backward references ? */

  if (*pBegin == '-')
  {
    for (pRun = pBegin; *pRun; pRun++)
      if (*pRun != '-')
        break;
    Cnt = pRun - pBegin;
    if (pRun == pEnd)
    {
      if ((symbol_source != e_symbol_source_none) && (Cnt == 1))
      {
        as_snprintf(pDest, STRINGSIZE, "__back%d", (int)BackSymCounter);
        AddTmpSymLog(TRUE, BackSymCounter);
        BackSymCounter++;
        Result = TRUE;
      }

      /* TmpSymLogDepth cannot become larger than LOCSYMSIGHT, so we only
         have to check against the log's actual depth. */

      else if (Cnt <= TmpSymLogDepth)
      {
        Cnt--;
        as_snprintf(pDest, STRINGSIZE, "__%s%d",
                    TmpSymLog[Cnt].Back ? "back" : "forw",
                    (int)TmpSymLog[Cnt].Counter);
        Result = TRUE;
      }
    }
  }

  /* forward references ? */

  else if (*pBegin == '+')
  {
    for (pRun = pBegin; *pRun; pRun++)
      if (*pRun != '+')
        break;
    Cnt = pRun - pBegin;
    if (pRun == pEnd)
    {
      if ((symbol_source != e_symbol_source_none) && (Cnt == 1))
      {
        as_snprintf(pDest, STRINGSIZE, "__forw%d", (int)FwdSymCounter++);
        Result = TRUE;
      }
      else if (Cnt <= LOCSYMSIGHT)
      {
        as_snprintf(pDest, STRINGSIZE, "__forw%d", (int)(FwdSymCounter + (Cnt - 1)));
        Result = TRUE;
      }
    }
  }

  /* slash: only allowed for definition, but add to log for backward ref. */

  else if ((pEnd - pBegin == 1) && (*pBegin == '/') && (symbol_source != e_symbol_source_none))
  {
    AddTmpSymLog(FALSE, FwdSymCounter);
    as_snprintf(pDest, STRINGSIZE, "__forw%d", (int)FwdSymCounter);
    FwdSymCounter++;
    Result = TRUE;
  }

  return Result;
}

static Boolean ChkTmp3(char *Name, as_symbol_source_t symbol_source)
{
  if ('.' == *Name)
  {
    strmaxprep2(Name, LastGlobSymbol, STRINGSIZE);
    return True;
  }

  if (symbol_source != e_symbol_source_none) {
    strmaxcpy(LastGlobSymbol, Name, STRINGSIZE);
    *TmpSymCounterVal = '\0';
  }
  return False;
}

static Boolean ChkTmp(char *Name, as_symbol_source_t symbol_source)
{
  if (ChkTmp1(Name))
  {
    return TRUE;
  }
  if (ChkTmp2(Name, Name, symbol_source))
  {
    return TRUE;
  }
  if (ChkTmp3(Name, symbol_source))
  {
    return TRUE;
  }
  return FALSE;
}

Boolean IdentifySection(const tStrComp *pName, LongInt *Erg)
{
  PSaveSection SLauf;
  String ExpName;
  sint Depth;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pName))
    return False;
  if (!CaseSensitive)
    NLS_UpString(ExpName);

  if (*ExpName == '\0')
  {
    *Erg = -1;
    return True;
  }
  else if (((strlen(ExpName) == 6) || (strlen(ExpName) == 7))
       && (!as_strncasecmp(ExpName, "PARENT", 6))
       && ((strlen(ExpName) == 6) || ((ExpName[6] >= '0') && (ExpName[6] <= '9'))))
  {
    Depth = (strlen(ExpName) == 6) ? 1 : ExpName[6] - AscOfs;
    SLauf = SectionStack;
    *Erg = MomSectionHandle;
    while ((Depth > 0) && (*Erg != -2))
    {
      if (!SLauf) *Erg = -2;
      else
      {
        *Erg = SLauf->Handle;
        SLauf = SLauf->Next;
      }
      Depth--;
    }
    if (*Erg == -2)
    {
      WrError(ErrNum_InvSection);
      return False;
    }
    else
      return True;
  }
  else if (!strcmp(ExpName, GetSectionName(MomSectionHandle)))
  {
    *Erg = MomSectionHandle;
    return True;
  }
  else
  {
    SLauf = SectionStack;
    while ((SLauf) && (strcmp(GetSectionName(SLauf->Handle), ExpName)))
      SLauf = SLauf->Next;
    if (!SLauf)
    {
      WrError(ErrNum_InvSection);
      return False;
    }
    else
    {
      *Erg = SLauf->Handle;
      return True;
    }
  }
}

static Boolean GetSymSection(char *Name, LongInt *Erg, const tStrComp *pUnexpComp)
{
  String Part;
  tStrComp TmpComp;
  char *q;
  int l = strlen(Name);

  if (Name[l - 1] != ']')
  {
    *Erg = -2;
    return True;
  }

  Name[l - 1] = '\0';
  q = RQuotPos(Name, '[');
  Name[l - 1] = ']';
  if (Name + l - q <= 1)
  {
    if (pUnexpComp)
      WrStrErrorPos(ErrNum_InvSymName, pUnexpComp);
    else
      WrXError(ErrNum_InvSymName, Name);
    return False;
  }

  Name[l - 1] = '\0';
  strmaxcpy(Part, q + 1, STRINGSIZE);
  *q = '\0';

  StrCompMkTemp(&TmpComp, Part, sizeof(Part));
  return IdentifySection(&TmpComp, Erg);
}

/*****************************************************************************
 * Function:    ConstIntVal
 * Purpose:     evaluate integer constant
 * Result:      integer value
 *****************************************************************************/

static LargeInt ConstIntVal(const char *pExpr, IntType Typ, Boolean *pResult)
{
  LargeInt Wert;
  Boolean NegFlag = False;
  int Digit;
  tIntCheckCtx Ctx;
  const tIntFormatList *pIntFormat;

  /* empty string is interpreted as 0 */

  if (!*pExpr)
  {
    *pResult = True;
    return 0;
  }

  *pResult = False;

  /* sign: */

  switch (*pExpr)
  {
    case '-':
      NegFlag = True;
      /* else fall-through */
    case '+':
      pExpr++;
      break;
  }
  Ctx.pExpr = pExpr;
  Ctx.ExprLen = strlen(pExpr);
  Ctx.Base = -1;

  for (pIntFormat = IntFormatList; pIntFormat->Check; pIntFormat++)
    if (pIntFormat->Check(&Ctx, pIntFormat->Ch))
    {
      Ctx.Base = (pIntFormat->Base > 0) ? pIntFormat->Base : RadixBase;
      break;
    }
  if (Ctx.Base <= 0)
    return -1;

  /* we may have decremented Ctx.ExprLen, so do not run until string end */

  Wert = 0;
  while (Ctx.ExprLen > 0)
  {
    Digit = DigitVal(as_toupper(*Ctx.pExpr), Ctx.Base);
    if (Digit == -1)
      return -1;
    Wert = Wert * Ctx.Base + Digit;
    Ctx.pExpr++; Ctx.ExprLen--;
  }

  if (NegFlag)
    Wert = -Wert;

  /* post-processing, range check */

  *pResult = RangeCheck(Wert, Typ);
  if (*pResult)
    return Wert;
  else if (HardRanges)
  {
    WrError(ErrNum_OverRange);
    return -1;
  }
  else
  {
    *pResult = True;
    WrError(ErrNum_WOverRange);
    return Wert & IntTypeDefs[(int)Typ].Mask;
  }
}

/*****************************************************************************
 * Function:    ConstFloatVal
 * Purpose:     evaluate floating point constant
 * Result:      value
 *****************************************************************************/

static Double ConstFloatVal(const char *pExpr, FloatType Typ, Boolean *pResult)
{
  Double Erg;
  char *pEnd;

  UNUSED(Typ);

  if (*pExpr)
  {
    /* Some strtod() implementations interpret hex constants starting with '0x'.  We
       don't want this here.  Either 0x for hex constants is allowed, then it should
       have been parsed before by ConstIntVal(), or not, then we don't want the constant
       be stored as float. */

    if ((strlen(pExpr) >= 2)
     && (pExpr[0] == '0')
     && (toupper(pExpr[1]) == 'X'))
    {
      Erg = 0;
      *pResult = False;
    }

    else
    {
      Erg = strtod(pExpr, &pEnd);
      *pResult = (*pEnd == '\0');
    }
  }
  else
  {
    Erg = 0.0;
    *pResult = True;
  }
  return Erg;
}

/*****************************************************************************
 * Function:    ConstStringVal
 * Purpose:     evaluate string constant
 * Result:      value
 *****************************************************************************/

static void ConstStringVal(const tStrComp *pExpr, TempResult *pDest, Boolean *pResult)
{
  tStrComp Raw, Copy, Remainder;
  char *pPos, QuoteChar;
  int l, TLen;

  *pResult = False;

  l = strlen(pExpr->str.p_str);
  if (l < 2)
    return;
  switch (*pExpr->str.p_str)
  {
    case '"':
    case '\'':
      QuoteChar = *pExpr->str.p_str;
      if (pExpr->str.p_str[l - 1] == QuoteChar)
      {
        if ('\'' == QuoteChar)
          pDest->Flags |= eSymbolFlag_StringSingleQuoted;
        break;
      }
      /* conditional fall-through */
    default:
      return;
  }

  StrCompAlloc(&Raw, STRINGSIZE);
  StrCompCopySub(&Raw, pExpr, 1, l - 2);
  /* use LEAVE from now on instead of return */

  /* go through source */

  as_tempres_set_c_str(pDest, "");
  StrCompRefRight(&Copy, &Raw, 0);
  while (1)
  {
    pPos = strchr(Copy.str.p_str, '\\');
    if (pPos)
      StrCompSplitRef(&Copy, &Remainder, &Copy, pPos);

    /* " before \ -> not a simple string but something like "...." ... " */

    if (strchr(Copy.str.p_str, QuoteChar))
    {
      as_tempres_set_none(pDest);
      LEAVE;
    }

    /* copy part up to next '\' verbatim: */

    as_nonz_dynstr_append_raw(&pDest->Contents.str, Copy.str.p_str, strlen(Copy.str.p_str));

    /* are we done? If not, advance pointer to behind '\' */

    if (!pPos)
      break;
    Copy = Remainder;

    /* treat escaped section: stringification? */

    if (*Copy.str.p_str == '{')
    {
      TempResult t;
      char *pStr;
      String Str;
      Boolean OK = True;

      as_tempres_ini(&t);
      StrCompIncRefLeft(&Copy, 1);

      /* cut out part in {...} */

      pPos = QuotPos(Copy.str.p_str, '}');
      if (!pPos)
      {
        OK = False;
        LEAVE2;
      }
      StrCompSplitRef(&Copy, &Remainder, &Copy, pPos);
      KillPrefBlanksStrCompRef(&Copy);
      KillPostBlanksStrComp(&Copy);

      /* evaluate expression */

      EvalStrExpression(&Copy, &t);
      if (t.Relocs)
      {
        WrStrErrorPos(ErrNum_NoRelocs, &Copy);
        FreeRelocs(&t.Relocs);
        as_tempres_set_none(&t);
      }

      /* append result */

      switch (t.Typ)
      {
        case TempInt:
          TLen = SysString(Str, sizeof(Str), t.Contents.Int, OutRadixBase, 0, False, HexStartCharacter, SplitByteCharacter);
          pStr = Str;
          break;
        case TempFloat:
          FloatString(Str, sizeof(Str), t.Contents.Float);
          pStr = Str;
          TLen = strlen(pStr);
          break;
        case TempString:
          pStr = t.Contents.str.p_str;
          TLen = t.Contents.str.len;
          break;
        default:
          *pResult = True;
          OK = False;
      }
      if (OK)
      {
        as_nonz_dynstr_append_raw(&pDest->Contents.str, pStr, TLen);
        pDest->Flags |= t.Flags & eSymbolFlags_Promotable;
      }

      /* advance source pointer to behind '}' */

      Copy = Remainder;

   func_exit2:
      as_tempres_free(&t);
      if (!OK)
      {
        as_tempres_set_none(pDest);
        LEAVE;
      }
    }

    /* simple character escape: */

    else
    {
      char Res, *pNext = Copy.str.p_str;

      if (!ProcessBk(&pNext, &Res))
      {
        as_tempres_set_none(pDest);
        LEAVE;
      }
      as_nonz_dynstr_append_raw(&pDest->Contents.str, &Res, 1);
      StrCompIncRefLeft(&Copy, pNext - Copy.str.p_str);
    }
  }

  *pResult = True;
func_exit:
  StrCompFree(&Raw);
}


static PSymbolEntry FindLocNode(const char *Name, TempType SearchType);

static PSymbolEntry FindNode(const char *Name, TempType SearchType);

/*!------------------------------------------------------------------------
 * \fn     EvalResultClear(tEvalResult *pResult)
 * \brief  reset all elements of EvalResult
 * ------------------------------------------------------------------------ */

void EvalResultClear(tEvalResult *pResult)
{
  pResult->OK = False;
  pResult->Flags = eSymbolFlag_None;
  pResult->AddrSpaceMask = 0;
  pResult->DataSize = eSymbolSizeUnknown;
}

/*****************************************************************************
 * Function:    EvalStrExpression
 * Purpose:     evaluate expression
 * Result:      implicitly in pErg
 *****************************************************************************/

#define LEAVE goto func_exit

static tErrorNum DeduceExpectTypeErrMsgMask(unsigned Mask, TempType ActType)
{
  switch (ActType)
  {
    case TempInt:
      switch (Mask)
      {
        case TempString:
          return ErrNum_StringButInt;
        /* int is convertible to float, so combinations are impossible: */
        case TempFloat:
        case TempFloat | TempString:
        default:
          return ErrNum_InternalError;
      }
    case TempFloat:
      switch (Mask)
      {
        case TempInt:
          return ErrNum_IntButFloat;
        case TempString:
          return ErrNum_StringButFloat;
        case TempInt | TempString:
          return ErrNum_StringOrIntButFloat;
        default:
          return ErrNum_InternalError;
      }
    case TempString:
      switch (Mask)
      {
        case TempInt:
          return ErrNum_IntButString;
        case TempFloat:
          return ErrNum_FloatButString;
        case TempInt | TempFloat:
          return ErrNum_IntOrFloatButString;
        default:
          return ErrNum_InternalError;
      }
    case TempReg:
      switch (Mask)
      {
        case TempInt:
          return ErrNum_ExpectInt;
        case TempString:
          return ErrNum_ExpectString;
        case TempInt | TempString:
          return ErrNum_ExpectIntOrString;
        case TempInt | TempFloat | TempString:
          return ErrNum_StringOrIntOrFloatButReg;
        default:
          return ErrNum_InternalError;
      }
    default:
      return ErrNum_InternalError;
  }
}

static Byte GetOpTypeMask(Byte TotMask, int OpIndex)
{
  return (TotMask >> (OpIndex * 4)) & 15;
}

static Byte TryConvert(Byte TypeMask, TempType ActType, int OpIndex)
{
  if (TypeMask & ActType)
    return 0 << (4 * OpIndex);
  if ((TypeMask & TempFloat) && (ActType == TempInt))
    return 1 << (4 * OpIndex);
  if ((TypeMask & TempInt) && (ActType == TempString))
    return 2 << (4 * OpIndex);
  if ((TypeMask & TempFloat) && (ActType == TempString))
    return (1|2) << (4 * OpIndex);
  return 255;
}

void EvalStrExpression(const tStrComp *pExpr, TempResult *pErg)
{
  const Operator *pOp;
  const Operator *FOps[OPERATOR_MAXCNT];
  LongInt FOpCnt = 0;

  Boolean OK;
  tStrComp InArgs[3];
  TempResult InVals[3];
  int z1, cnt;
  char Save = '\0';
  sint LKlamm, RKlamm, WKlamm, zop;
  sint OpMax, OpPos = -1;
  Boolean InSgl, InDbl, NextEscaped, ThisEscaped;
  PFunction ValFunc;
  tStrComp CopyComp, STempComp;
  char *KlPos, *zp, *pOpPos;
  const tFunction *pFunction;
  PRelocEntry TReloc;
  tSymbolFlags PromotedFlags;
  unsigned PromotedAddrSpaceMask;
  tSymbolSize PromotedDataSize;

  for (z1 = 0; z1 < 3; z1++)
    as_tempres_ini(&InVals[z1]);
  StrCompAlloc(&CopyComp, STRINGSIZE);
  StrCompAlloc(&STempComp, STRINGSIZE);

  if (MakeDebug)
    fprintf(Debug, "Parse '%s'\n", pExpr->str.p_str);

  /* Annahme Fehler */

  as_tempres_set_none(pErg);
  pErg->Relocs = NULL;
  pErg->Flags = eSymbolFlag_None;
  pErg->AddrSpaceMask = 0;
  pErg->DataSize = eSymbolSizeUnknown;

  StrCompCopy(&CopyComp, pExpr);
  KillPrefBlanksStrComp(&CopyComp);
  KillPostBlanksStrComp(&CopyComp);

  /* sort out local symbols like - and +++.  Do it now to get them out of the
     formula parser's way. */

  ChkTmp2(CopyComp.str.p_str, CopyComp.str.p_str, e_symbol_source_none);
  StrCompCopy(&STempComp, &CopyComp);

  /* Programmzaehler ? */

  if (PCSymbol && (!as_strcasecmp(CopyComp.str.p_str, PCSymbol)))
  {
    as_tempres_set_int(pErg, EProgCounter());
    pErg->Relocs = NULL;
    pErg->AddrSpaceMask |= 1 << ActPC;
    LEAVE;
  }

  /* Konstanten ? */

  pErg->Contents.Int = ConstIntVal(CopyComp.str.p_str, (IntType) (IntTypeCnt - 1), &OK);
  if (OK)
  {
    pErg->Typ = TempInt;
    pErg->Relocs = NULL;
    LEAVE;
  }

  pErg->Contents.Float = ConstFloatVal(CopyComp.str.p_str, Float80, &OK);
  if (OK)
  {
    pErg->Typ = TempFloat;
    pErg->Relocs = NULL;
    LEAVE;
  }

  ConstStringVal(&CopyComp, pErg, &OK);
  if (OK)
  {
    pErg->Relocs = NULL;
    LEAVE;
  }

  /* durch Codegenerator gegebene Konstanten ? */

  pErg->Relocs = NULL;
  InternSymbol(CopyComp.str.p_str, pErg);
  if (pErg->Typ != TempNone)
  {
    LEAVE;
  }

  /* find out which operators *might* occur in expression */

  OpMax = 0;
  LKlamm = 0;
  RKlamm = 0;
  WKlamm = 0;
  InSgl =
  InDbl =
  ThisEscaped =
  NextEscaped = False;
  for (pOp = Operators + 1; pOp->Id; pOp++)
  {
    pOpPos = (pOp->IdLen == 1) ? (strchr(CopyComp.str.p_str, *pOp->Id)) : (strstr(CopyComp.str.p_str, pOp->Id));
    if (pOpPos)
      FOps[FOpCnt++] = pOp;
  }

  /* nach Operator hoechster Rangstufe ausserhalb Klammern suchen */

  for (zp = CopyComp.str.p_str; *zp; zp++, ThisEscaped = NextEscaped)
  {
    NextEscaped = False;
    switch (*zp)
    {
      case '(':
        if (!(InSgl || InDbl))
          LKlamm++;
        break;
      case ')':
        if (!(InSgl || InDbl))
          RKlamm++;
        break;
      case '{':
        if (!(InSgl || InDbl))
          WKlamm++;
        break;
      case '}':
        if (!(InSgl || InDbl))
          WKlamm--;
        break;
      case '"':
        if (!InSgl && !ThisEscaped)
          InDbl = !InDbl;
        break;
      case '\'':
        if (!InDbl && !ThisEscaped)
        {
          if (InSgl || !QualifyQuote || QualifyQuote(CopyComp.str.p_str, zp))
            InSgl = !InSgl;
        }
        break;
      case '\\':
        if ((InDbl || InSgl) && !ThisEscaped)
          NextEscaped = True;
        break;
      default:
        if ((LKlamm == RKlamm) && (WKlamm == 0) && (!InSgl) && (!InDbl))
        {
          Boolean OpFnd = False;
          sint OpLen = 0, LocOpMax = 0;

          for (zop = 0; zop < FOpCnt; zop++)
          {
            pOp = FOps[zop];
            if ((!strncmp(zp, pOp->Id, pOp->IdLen)) && (pOp->IdLen >= OpLen))
            {
              OpFnd = True;
              OpLen = pOp->IdLen;
              LocOpMax = pOp - Operators;
              if (Operators[LocOpMax].Priority >= Operators[OpMax].Priority)
              {
                OpMax = LocOpMax;
                OpPos = zp - CopyComp.str.p_str;
              }
            }
          }
          if (OpFnd)
            zp += Operators[LocOpMax].IdLen - 1;
        }
    }
  }

  /* Klammerfehler ? */

  if (LKlamm != RKlamm)
  {
    WrStrErrorPos(ErrNum_BrackErr, &CopyComp);
    LEAVE;
  }

  /* Operator gefunden ? */

  if (OpMax)
  {
    int ThisArgCnt, CompLen, z, z2;
    Byte ThisOpMatch, BestOpMatch, BestOpMatchIdx, SumCombinations, TypeMask;

    pOp = Operators + OpMax;

    /* Minuszeichen sowohl mit einem als auch 2 Operanden */

    if (!strcmp(pOp->Id, "-"))
    {
      if (!OpPos)
        pOp = &MinusMonadicOperator;
    }

    else if (!strcmp(pOp->Id, "^"))
    {
      if (!OpPos && pPotMonadicOperator)
        pOp = pPotMonadicOperator;
    }

    /* Operandenzahl pruefen */

    CompLen = strlen(CopyComp.str.p_str);
    if (CompLen <= 1)
      ThisArgCnt = 0;
    else if (!OpPos || (OpPos == (int)strlen(CopyComp.str.p_str) - 1))
      ThisArgCnt = 1;
    else
      ThisArgCnt = 2;
    if (!ChkArgCntExtPos(ThisArgCnt, pOp->Dyadic ? 2 : 1, pOp->Dyadic ? 2 : 1, &CopyComp.Pos))
      LEAVE;

    /* Teilausdruecke rekursiv auswerten */

    Save = StrCompSplitRef(&InArgs[0], &InArgs[1], &CopyComp, CopyComp.str.p_str + OpPos);
    StrCompIncRefLeft(&InArgs[1], strlen(pOp->Id) - 1);
    EvalStrExpression(&InArgs[1], &InVals[1]);
    if (pOp->Dyadic)
      EvalStrExpression(&InArgs[0], &InVals[0]);
    else if (InVals[1].Typ == TempFloat)
      as_tempres_set_float(&InVals[0], 0.0);
    else
    {
      as_tempres_set_int(&InVals[0], 0);
      InVals[0].Relocs = NULL;
    }
    CopyComp.str.p_str[OpPos] = Save;

    /* Abbruch, falls dabei Fehler */

    if ((InVals[0].Typ == TempNone) || (InVals[1].Typ == TempNone))
      LEAVE;

    /* relokatible Symbole nur fuer + und - erlaubt */

    if ((OpMax != 12) && (OpMax != 13) && (InVals[0].Relocs || InVals[1].Relocs))
    {
      WrStrErrorPos(ErrNum_NoRelocs, &CopyComp);
      LEAVE;
    }

    /* see whether data types match operator's restrictions: */

    BestOpMatch = 255; BestOpMatchIdx = OPERATOR_MAXCOMB;
    SumCombinations = 0;
    for (z = 0; z < OPERATOR_MAXCOMB; z++)
    {
      if (!pOp->TypeCombinations[z])
        break;
      SumCombinations |= pOp->TypeCombinations[z];

      ThisOpMatch = 0;
      for (z2 = pOp->Dyadic ? 0 : 1; z2 < 2; z2++)
        ThisOpMatch |= TryConvert(GetOpTypeMask(pOp->TypeCombinations[z], z2), InVals[z2].Typ, z2);
      if (ThisOpMatch < BestOpMatch)
      {
        BestOpMatch = ThisOpMatch;
        BestOpMatchIdx = z;
      }
      if (!BestOpMatch)
        break;
    }

    /* did not find a way to satisfy restrictions, even by conversions? */

    if (BestOpMatch >= 255)
    {
      for (z2 = pOp->Dyadic ? 0 : 1; z2 < 2; z2++)
      {
        TypeMask = GetOpTypeMask(SumCombinations, z2);
        if (!(TypeMask & InVals[z2].Typ))
          WrStrErrorPos(DeduceExpectTypeErrMsgMask(TypeMask, InVals[z2].Typ), &InArgs[z2]);
      }
      LEAVE;
    }

    /* necessary conversions: */

    for (z2 = pOp->Dyadic ? 0 : 1; z2 < 2; z2++)
    {
      TypeMask = (BestOpMatch >> (z2 * 4)) & 15;
      if (TypeMask & 2)  /* String -> Int */
        TempResultToInt(&InVals[z2]);
      if (TypeMask & 1) /* Int -> Float */
        TempResultToFloat(&InVals[z2]);
    }

    /* actual operation */

    (void)BestOpMatchIdx;
    pOp->pFunc(pErg, &InVals[0], &InVals[1]);
    LEAVE;
  } /* if (OpMax) */

  /* kein Operator gefunden: Klammerausdruck ? */

  if (LKlamm != 0)
  {
    tStrComp FName, FArg, Remainder;

    /* erste Klammer suchen, Funktionsnamen abtrennen */

    KlPos = strchr(CopyComp.str.p_str, '(');

    /* Funktionsnamen abschneiden */

    StrCompSplitRef(&FName, &FArg, &CopyComp, KlPos);
    StrCompShorten(&FArg, 1);
    KillPostBlanksStrComp(&FName);

    /* Nullfunktion: nur Argument */

    if (*FName.str.p_str == '\0')
    {
      EvalStrExpression(&FArg, pErg);
      LEAVE;
    }

    /* selbstdefinierte Funktion ? */

    ValFunc = FindFunction(FName.str.p_str);
    if (ValFunc)
    {
      as_dynstr_t CompArgStr;
      tStrComp CompArg;
      as_dynstr_t stemp;

      PromotedFlags = eSymbolFlag_None;
      PromotedAddrSpaceMask = 0;
      PromotedDataSize = eSymbolSizeUnknown;
      as_dynstr_ini_c_str(&CompArgStr, ValFunc->Definition);
      as_dynstr_ini(&stemp, STRINGSIZE);
      for (z1 = 1; z1 <= ValFunc->ArguCnt; z1++)
      {
        if (!*FArg.str.p_str)
        {
          WrError(ErrNum_InvFuncArgCnt);
          LEAVE2;
        }

        KlPos = QuotPos(FArg.str.p_str, ',');
        if (KlPos)
          StrCompSplitRef(&FArg, &Remainder, &FArg, KlPos);

        EvalStrExpression(&FArg, &InVals[0]);
        if (InVals[0].Relocs)
        {
          WrStrErrorPos(ErrNum_NoRelocs, &FArg);
          FreeRelocs(&InVals[0].Relocs);
          LEAVE2;
        }
        PromotedFlags |= InVals[0].Flags & eSymbolFlags_Promotable;
        PromotedAddrSpaceMask |= InVals[0].AddrSpaceMask;
        if (PromotedDataSize == eSymbolSizeUnknown) PromotedDataSize = InVals[0].DataSize;

        if (KlPos)
          FArg = Remainder;
        else
          StrCompReset(&FArg);

        as_sdprintf(&stemp, "(");
        if (as_tempres_append_dynstr(&stemp, &InVals[0]))
          LEAVE2;
        as_sdprcatf(&stemp, ")");
        ExpandLine(stemp.p_str, z1, &CompArgStr);
      }
      if (*FArg.str.p_str)
      {
        WrError(ErrNum_InvFuncArgCnt);
        LEAVE2;
      }
      StrCompMkTemp(&CompArg, CompArgStr.p_str, CompArgStr.capacity);
      EvalStrExpression(&CompArg, pErg);
      pErg->Flags |= PromotedFlags;
      pErg->AddrSpaceMask |= PromotedAddrSpaceMask;
      if (pErg->DataSize == eSymbolSizeUnknown) pErg->DataSize = PromotedDataSize;
func_exit2:
      as_dynstr_free(&stemp);
      as_dynstr_free(&CompArgStr);
      LEAVE;
    }

    /* hier einmal umwandeln ist effizienter */

    NLS_UpString(FName.str.p_str);

    /* symbolbezogene Funktionen */

    if (!strcmp(FName.str.p_str, "SYMTYPE"))
    {
      as_tempres_set_int(pErg, GetSymbolType(&FArg));
      LEAVE;
    }

    else if (!strcmp(FName.str.p_str, "DEFINED"))
    {
      as_tempres_set_int(pErg, !!IsSymbolDefined(&FArg));
      LEAVE;
    }

    else if (!strcmp(FName.str.p_str, "ASSUMEDVAL"))
    {
      unsigned IdxAssume;

      for (IdxAssume = 0; IdxAssume < ASSUMERecCnt; IdxAssume++)
        if (!as_strcasecmp(FArg.str.p_str, pASSUMERecs[IdxAssume].Name))
        {
          as_tempres_set_int(pErg, *(pASSUMERecs[IdxAssume].Dest));
          LEAVE;
        }
      WrStrErrorPos(ErrNum_SymbolUndef, &FArg);
      LEAVE;
    }

    /* Unterausdruck auswerten (interne Funktionen maxmimal mit drei Argumenten) */

    cnt = 0;
    PromotedFlags = eSymbolFlag_None;
    PromotedAddrSpaceMask = 0;
    PromotedDataSize = eSymbolSizeUnknown;
    do
    {
      zp = QuotPos(FArg.str.p_str, ',');
      if (zp)
        StrCompSplitRef(&InArgs[cnt], &Remainder, &FArg, zp);
      else
        InArgs[cnt] = FArg;
      if (cnt < 3)
      {
        EvalStrExpression(&InArgs[cnt], &InVals[cnt]);
        if (InVals[cnt].Typ == TempNone)
          LEAVE;
        TReloc = InVals[cnt].Relocs;
      }
      else
      {
        WrError(ErrNum_InvFuncArgCnt);
        LEAVE;
      }
      if (TReloc)
      {
        WrStrErrorPos(ErrNum_NoRelocs, &InArgs[cnt]);
        FreeRelocs(&TReloc);
        LEAVE;
      }
      if (zp)
        FArg = Remainder;
      PromotedFlags |= InVals[cnt].Flags & eSymbolFlags_Promotable;
      PromotedAddrSpaceMask |= InVals[cnt].AddrSpaceMask;
      if (PromotedDataSize == eSymbolSizeUnknown) PromotedDataSize = InVals[0].DataSize;
      cnt++;
    }
    while (zp);

    /* search function */

    for (pFunction = Functions; pFunction->pName; pFunction++)
      if (!strcmp(FName.str.p_str, pFunction->pName))
        break;
    if (!pFunction->pName)
    {
      WrStrErrorPos(ErrNum_UnknownFunc, &FName);
      LEAVE;
    }

    /* argument checking */

    if ((cnt < pFunction->MinNumArgs) || (cnt > pFunction->MaxNumArgs))
    {
      WrError(ErrNum_InvFuncArgCnt);
      LEAVE;
    }
    for (z1 = 0; z1 < cnt; z1++)
    {
      if ((InVals[z1].Typ == TempInt) && (!(pFunction->ArgTypes[z1] & (1 << TempInt))))
        TempResultToFloat(&InVals[z1]);
      if (!(pFunction->ArgTypes[z1] & (1 << InVals[z1].Typ)))
      {
        WrStrErrorPos(DeduceExpectTypeErrMsgMask(pFunction->ArgTypes[z1], InVals[z1].Typ), &InArgs[z1]);
        LEAVE;
      }
    }
    pFunction->pFunc(pErg, InVals, cnt);
    pErg->Flags |= PromotedFlags;
    pErg->AddrSpaceMask |= PromotedAddrSpaceMask;
    if (pErg->DataSize == eSymbolSizeUnknown) pErg->DataSize = PromotedDataSize;

    LEAVE;
  }

  /* nichts dergleichen, dann einfaches Symbol: urspruenglichen Wert wieder
     herstellen, dann Pruefung auf $$-temporaere Symbole */

  StrCompCopy(&CopyComp, &STempComp);
  KillPrefBlanksStrComp(&CopyComp);
  KillPostBlanksStrComp(&CopyComp);

  ChkTmp1(CopyComp.str.p_str);

  /* interne Symbole ? */

  if (!as_strcasecmp(CopyComp.str.p_str, "MOMFILE"))
  {
    as_tempres_set_c_str(pErg, CurrFileName);
    LEAVE;
  }

  if (!as_strcasecmp(CopyComp.str.p_str, "MOMLINE"))
  {
    as_tempres_set_int(pErg, CurrLine);
    LEAVE;
  }

  if (!as_strcasecmp(CopyComp.str.p_str, "MOMPASS"))
  {
    as_tempres_set_int(pErg, PassNo);
    LEAVE;
  }

  if (!as_strcasecmp(CopyComp.str.p_str, "MOMSECTION"))
  {
    as_tempres_set_c_str(pErg, GetSectionName(MomSectionHandle));
    LEAVE;
  }

  if (!as_strcasecmp(CopyComp.str.p_str, "MOMSEGMENT"))
  {
    as_tempres_set_c_str(pErg, SegNames[ActPC]);
    LEAVE;
  }

  /* plain symbol */

  LookupSymbol(&CopyComp, pErg, True, TempAll);

func_exit:

  StrCompFree(&CopyComp);
  StrCompFree(&STempComp);

  for (z1 = 0; z1 < 3; z1++)
  {
    if (InVals[z1].Relocs)
      FreeRelocs(&InVals[z1].Relocs);
    as_tempres_free(&InVals[z1]);
  }
}

void EvalExpression(const char *pExpr, TempResult *pErg)
{
  tStrComp Expr;

  StrCompMkTemp(&Expr, (char*)pExpr, 0);
  EvalStrExpression(&Expr, pErg);
}

LargeInt EvalStrIntExpressionWithResult(const tStrComp *pComp, IntType Type, tEvalResult *pResult)
{
  TempResult t;
  LargeInt Result = -1;

  as_tempres_ini(&t);
  EvalResultClear(pResult);

  EvalStrExpression(pComp, &t);
  SetRelocs(t.Relocs);

  switch (t.Typ)
  {
    case TempInt:
      Result = t.Contents.Int;
      pResult->Flags = t.Flags;
      pResult->AddrSpaceMask = t.AddrSpaceMask;
      pResult->DataSize = t.DataSize;
      break;
    case TempString:
    {
      int l = t.Contents.str.len;

      if ((l > 0) && (l <= 4))
      {
        char *pRun;
        Byte Digit;

        Result = 0;
        for (pRun = t.Contents.str.p_str;
             pRun < t.Contents.str.p_str + l;
             pRun++)
        {
          Digit = (usint) *pRun;
          Result = (Result << 8) | CharTransTable[Digit & 0xff];
        }
        pResult->Flags = t.Flags;
        pResult->AddrSpaceMask = t.AddrSpaceMask;
        pResult->DataSize = t.DataSize;
        break;
      }
      else
      {
        WrStrErrorPos(ErrNum_IntButString, pComp);
        FreeRelocs(&LastRelocs);
        LEAVE;
      }
    }
    /* else fall-through */
    default:
      if (t.Typ != TempNone)
        WrStrErrorPos(DeduceExpectTypeErrMsgMask(TempInt | TempString, t.Typ), pComp);
      FreeRelocs(&LastRelocs);
      LEAVE;
  }

  if (mFirstPassUnknown(t.Flags))
    Result &= IntTypeDefs[(int)Type].Mask;

  if (!RangeCheck(Result, Type))
  {
    if (HardRanges)
    {
      FreeRelocs(&LastRelocs);
      WrStrErrorPos(ErrNum_OverRange, pComp);
      Result = -1;
      LEAVE;
    }
    else
    {
      WrStrErrorPos(ErrNum_WOverRange, pComp);
      pResult->OK = True;
      Result &= IntTypeDefs[(int)Type].Mask;
    }
  }
  else
    pResult->OK = True;

func_exit:
  as_tempres_free(&t);
  return Result;
}

LargeInt EvalStrIntExpressionWithFlags(const tStrComp *pComp, IntType Type, Boolean *pResult, tSymbolFlags *pFlags)
{
  tEvalResult EvalResult;
  LargeInt Result = EvalStrIntExpressionWithResult(pComp, Type, &EvalResult);

  *pResult = EvalResult.OK;
  if (pFlags)
    *pFlags = EvalResult.Flags;
  return Result;
}

LargeInt EvalStrIntExpression(const tStrComp *pComp, IntType Type, Boolean *pResult)
{
  tEvalResult EvalResult;
  LargeInt Result = EvalStrIntExpressionWithResult(pComp, Type, &EvalResult);

  *pResult = EvalResult.OK;
  return Result;
}

LargeInt EvalStrIntExpressionOffsWithResult(const tStrComp *pExpr, int Offset, IntType Type, tEvalResult *pResult)
{
  if (Offset)
  {
    tStrComp Comp;

    StrCompRefRight(&Comp, pExpr, Offset);
    return EvalStrIntExpressionWithResult(&Comp, Type, pResult);
  }
  else
    return EvalStrIntExpressionWithResult(pExpr, Type, pResult);
}

LargeInt EvalStrIntExpressionOffsWithFlags(const tStrComp *pComp, int Offset, IntType Type, Boolean *pResult, tSymbolFlags *pFlags)
{
  tEvalResult EvalResult;
  LargeInt Result = EvalStrIntExpressionOffsWithResult(pComp, Offset, Type, &EvalResult);

  *pResult = EvalResult.OK;
  if (pFlags)
    *pFlags = EvalResult.Flags;
  return Result;
}

LargeInt EvalStrIntExpressionOffs(const tStrComp *pComp, int Offset, IntType Type, Boolean *pResult)
{
  tEvalResult EvalResult;
  LargeInt Result = EvalStrIntExpressionOffsWithResult(pComp, Offset, Type, &EvalResult);

  *pResult = EvalResult.OK;
  return Result;
}

Double EvalStrFloatExpressionWithResult(const tStrComp *pExpr, FloatType Type, tEvalResult *pResult)
{
  TempResult t;
  Double Result = -1;

  as_tempres_ini(&t);
  EvalResultClear(pResult);

  EvalStrExpression(pExpr, &t);
  switch (t.Typ)
  {
    case TempNone:
      LEAVE;
    case TempInt:
      Result = t.Contents.Int;
      pResult->Flags = t.Flags;
      pResult->AddrSpaceMask = t.AddrSpaceMask;
      pResult->DataSize = t.DataSize;
      break;
    case TempString:
    {
      WrStrErrorPos(ErrNum_FloatButString, pExpr);
      LEAVE;
    }
    default:
      Result = t.Contents.Float;
  }

  if (!FloatRangeCheck(Result, Type))
  {
    WrStrErrorPos(ErrNum_OverRange, pExpr);
    LEAVE;
  }

  pResult->OK = True;
func_exit:
  as_tempres_free(&t);
  return Result;
}

Double EvalStrFloatExpression(const tStrComp *pExpr, FloatType Type, Boolean *pResult)
{
  Double Ret;
  tEvalResult Result;

  Ret = EvalStrFloatExpressionWithResult(pExpr, Type, &Result);
  *pResult = Result.OK;
  return Ret;
}

void EvalStrStringExpressionWithResult(const tStrComp *pExpr, tEvalResult *pResult, char *pEvalResult)
{
  TempResult t;

  as_tempres_ini(&t);
  EvalResultClear(pResult);

  EvalStrExpression(pExpr, &t);
  if (t.Typ == TempInt)
  {
    as_snprintf(pEvalResult, STRINGSIZE, "%" PRId64, t.Contents.Int);
    pResult->Flags = t.Flags;
    pResult->AddrSpaceMask = t.AddrSpaceMask;
    pResult->DataSize = t.DataSize;
    pResult->OK = True;
  }
  else if (t.Typ != TempString)
  {
    *pEvalResult = '\0';
    if (t.Typ != TempNone)
    {
      if (mFirstPassUnknown(t.Flags))
      {
        *pEvalResult = '\0';
        pResult->Flags = t.Flags;
        pResult->AddrSpaceMask = t.AddrSpaceMask;
        pResult->DataSize = t.DataSize;
        pResult->OK = True;
      }
      else
        WrStrErrorPos(DeduceExpectTypeErrMsgMask(TempString, t.Typ), pExpr);
    }
  }
  else
  {
    as_nonz_dynstr_to_c_str(pEvalResult, &t.Contents.str, STRINGSIZE);
    pResult->Flags = t.Flags;
    pResult->AddrSpaceMask = t.AddrSpaceMask;
    pResult->DataSize = t.DataSize;
    pResult->OK = True;
  }
  as_tempres_free(&t);
}

void EvalStrStringExpression(const tStrComp *pExpr, Boolean *pResult, char *pEvalResult)
{
  tEvalResult Result;

  EvalStrStringExpressionWithResult(pExpr, &Result, pEvalResult);
  *pResult = Result.OK;
}

/*!------------------------------------------------------------------------
 * \fn     EvalStrRegExpressionWithResult(const struct sStrComp *pExpr, struct sRegDescr *pResult, struct sEvalResult *pEvalResult, Boolean IssueErrors)
 * \brief  retrieve/evaluate register expression
 * \param  pExpr source code expression
 * \param  pResult retrieved register
 * \param  pEvalResult success flag, symbol size & flags
 * \param  IssueErrors print errors at all?
 * \return occurred error
 * ------------------------------------------------------------------------ */

PSymbolEntry ExpandAndFindNode(const struct sStrComp *pComp, TempType SearchType)
{
  PSymbolEntry pEntry;
  String ExpName;
  const char *pKlPos;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pComp))
    return NULL;

  /* just [...] without symbol name itself is not valid */

  pKlPos = strchr(ExpName, '[');
  if ((pKlPos == ExpName) || (ChkSymbNameUpTo(ExpName, pKlPos) != pKlPos))
    return NULL;

  pEntry = FindLocNode(ExpName, SearchType);
  if (!pEntry)
    pEntry = FindNode(ExpName, SearchType);
  return pEntry;
}

tErrorNum EvalStrRegExpressionWithResult(const struct sStrComp *pExpr, tRegDescr *pResult, tEvalResult *pEvalResult)
{
  PSymbolEntry pEntry;

  EvalResultClear(pEvalResult);

  pEntry = ExpandAndFindNode(pExpr, TempReg);
  if (!pEntry)
    return ErrNum_SymbolUndef;

  pEntry->Used = True;
  pEvalResult->DataSize = pEntry->SymWert.DataSize;

  if (pEntry->SymWert.Typ != TempReg)
    return ErrNum_ExpectReg;
  *pResult = pEntry->SymWert.Contents.RegDescr;

  if (pEntry->SymWert.Contents.RegDescr.Dissect != DissectReg)
    return ErrNum_RegWrongTarget;

  pEvalResult->OK = True;
  return ErrNum_None;
}

/*!------------------------------------------------------------------------
 * \fn     EvalStrRegExpressionAsOperand(const tStrComp *pArg, struct sRegDescr *pResult, struct sEvalResult *pEvalResult, tSymbolSize ReqSize, Boolean MustBeReg)
 * \brief  check for possible register in instruction operand
 * \param  pArg source argument
 * \param  ReqSize possible fixed operand size
 * \param  MustBeReg operand cannot be anything else but register
 * \return eIsReg: argument is a register
           eIsNoReg: argument is no register
           eRegAbort: argument is faulty, abort anyway (only if !MustBeReg)
 * ------------------------------------------------------------------------ */

tRegEvalResult EvalStrRegExpressionAsOperand(const tStrComp *pArg, struct sRegDescr *pResult, struct sEvalResult *pEvalResult, tSymbolSize ReqSize, Boolean MustBeReg)
{
  tErrorNum ErrorNum;

  ErrorNum = EvalStrRegExpressionWithResult(pArg, pResult, pEvalResult);
  if (pEvalResult->OK && (ReqSize != eSymbolSizeUnknown) && (pEvalResult->DataSize != ReqSize))
  {
    pEvalResult->OK = False;
    ErrorNum = ErrNum_InvOpSize;
  }
  switch (ErrorNum)
  {
    case ErrNum_None:
      return eIsReg;
    case ErrNum_SymbolUndef:
      if (MustBeReg)
      {
        if (PassNo <= MaxSymPass)
        {
          pResult->Reg = 0;
          pResult->Dissect = NULL;
          Repass = True;
          return eIsReg;
        }
        else
        {
          WrStrErrorPos(ErrNum_InvReg, pArg);
          return eIsNoReg;
        }
      }
      else
        return eIsNoReg;
    case ErrNum_ExpectReg:
      if (MustBeReg)
        WrStrErrorPos(ErrorNum, pArg);
      return eIsNoReg;
    default:
      WrStrErrorPos(ErrorNum, pArg);
      return MustBeReg ? eIsNoReg : eRegAbort;
  }
}

static void FreeSymbolEntry(PSymbolEntry *Node, Boolean Destroy)
{
  PCrossRef Lauf;

  if ((*Node)->Tree.Name)
  {
    free((*Node)->Tree.Name);
   (*Node)->Tree.Name = NULL;
  }

  as_tempres_free(&(*Node)->SymWert);

  while ((*Node)->RefList)
  {
    Lauf = (*Node)->RefList->Next;
    free((*Node)->RefList);
    (*Node)->RefList = Lauf;
  }

  FreeRelocs(&((*Node)->SymWert.Relocs));

  if (Destroy)
  {
    free(*Node);
    Node = NULL;
  }
}

static char *serr, *snum;
typedef struct
{
  Boolean MayChange, DoCross;
} TEnterStruct, *PEnterStruct;

static Boolean SymbolAdder(PTree *PDest, PTree Neu, void *pData)
{
  PSymbolEntry NewEntry = (PSymbolEntry)Neu, *Node;
  PEnterStruct EnterStruct = (PEnterStruct) pData;

  /* added to an empty leaf ? */

  if (!PDest)
  {
    NewEntry->Defined = True;
    NewEntry->Used = False;
    NewEntry->Changeable = EnterStruct->MayChange;
    NewEntry->RefList = NULL;
    if (EnterStruct->DoCross)
    {
      NewEntry->FileNum = GetFileNum(CurrFileName);
      NewEntry->LineNum = CurrLine;
    }
    return True;
  }

  /* replace en entry: check for validity */

  Node = (PSymbolEntry*)PDest;

  /* tried to redefine a symbol with EQU ? */

  if (((*Node)->Defined) && (!(*Node)->Changeable) && (!EnterStruct->MayChange))
  {
    strmaxcpy(serr, (*Node)->Tree.Name, STRINGSIZE);
    if (EnterStruct->DoCross)
      as_snprcatf(serr, STRINGSIZE, ",%s %s:%ld",
                  getmessage(Num_PrevDefMsg),
                  GetFileName((*Node)->FileNum), (long)((*Node)->LineNum));
    WrXError(ErrNum_DoubleDef, serr);
    FreeSymbolEntry(&NewEntry, TRUE);
    return False;
  }

  /* tried to reassign a constant (EQU) a value with SET and vice versa ? */

  else if ( ((*Node)->Defined) && (EnterStruct->MayChange != (*Node)->Changeable) )
  {
    strmaxcpy(serr, (*Node)->Tree.Name, STRINGSIZE);
    if (EnterStruct->DoCross)
      as_snprcatf(serr, STRINGSIZE, ",%s %s:%ld",
                  getmessage(Num_PrevDefMsg),
                  GetFileName((*Node)->FileNum), (long)((*Node)->LineNum));
    WrXError((*Node)->Changeable ? ErrNum_VariableRedefinedAsConstant : ErrNum_ConstantRedefinedAsVariable, serr);
    FreeSymbolEntry(&NewEntry, TRUE);
    return False;
  }

  else
  {
    if (!EnterStruct->MayChange)
    {
      /* TODO TempResult */
      if ((NewEntry->SymWert.Typ != (*Node)->SymWert.Typ)
       || ((NewEntry->SymWert.Typ == TempString) && (as_nonz_dynstr_cmp(&NewEntry->SymWert.Contents.str, &(*Node)->SymWert.Contents.str)))
       || ((NewEntry->SymWert.Typ == TempFloat ) && (NewEntry->SymWert.Contents.Float != (*Node)->SymWert.Contents.Float))
       || ((NewEntry->SymWert.Typ == TempInt   ) && (NewEntry->SymWert.Contents.Int   != (*Node)->SymWert.Contents.Int  )))
       {
         if ((!Repass) && (JmpErrors>0))
         {
           if (ThrowErrors)
             ErrorCount -= JmpErrors;
           JmpErrors = 0;
         }
         Repass = True;
         if ((MsgIfRepass) && (PassNo >= PassNoForMessage))
         {
           strmaxcpy(serr, Neu->Name, STRINGSIZE);
           if (Neu->Attribute != -1)
           {
             strmaxcat(serr, "[", STRINGSIZE);
             strmaxcat(serr, GetSectionName(Neu->Attribute), STRINGSIZE);
             strmaxcat(serr, "]", STRINGSIZE);
           }
           WrXError(ErrNum_PhaseErr, serr);
         }
       }
    }
    if (EnterStruct->DoCross)
    {
      NewEntry->LineNum = (*Node)->LineNum;
      NewEntry->FileNum = (*Node)->FileNum;
    }
    NewEntry->RefList = (*Node)->RefList;
    (*Node)->RefList = NULL;
    NewEntry->Defined = True;
    NewEntry->Used = (*Node)->Used;
    NewEntry->Changeable = EnterStruct->MayChange;
    FreeSymbolEntry(Node, False);
    return True;
  }
}

static void EnterLocSymbol(PSymbolEntry Neu)
{
  TEnterStruct EnterStruct;
  PTree TreeRoot;

  Neu->Tree.Attribute = MomLocHandle;
  if (!CaseSensitive)
    NLS_UpString(Neu->Tree.Name);
  EnterStruct.MayChange = EnterStruct.DoCross = FALSE;
  TreeRoot = &FirstLocSymbol->Tree;
  EnterTree(&TreeRoot, (&Neu->Tree), SymbolAdder, &EnterStruct);
  FirstLocSymbol = (PSymbolEntry)TreeRoot;
}

static void EnterSymbol_Search(PForwardSymbol *Lauf, PForwardSymbol *Prev,
                               PForwardSymbol **RRoot, PSymbolEntry Neu,
                               PForwardSymbol *Root, Byte ResCode, Byte *SearchErg)
{
  *Lauf = (*Root);
  *Prev = NULL;
  *RRoot = Root;
  while ((*Lauf) && (strcmp((*Lauf)->Name, Neu->Tree.Name)))
  {
    *Prev = (*Lauf);
    *Lauf = (*Lauf)->Next;
  }
  if (*Lauf)
    *SearchErg = ResCode;
}

static void EnterSymbol(PSymbolEntry Neu, Boolean MayChange, LongInt ResHandle)
{
  PForwardSymbol Lauf, Prev;
  PForwardSymbol *RRoot;
  Byte SearchErg;
  String CombName;
  PSaveSection RunSect;
  LongInt MSect;
  PSymbolEntry Copy;
  TEnterStruct EnterStruct;
  PTree TreeRoot = &(FirstSymbol->Tree);

  if (!CaseSensitive)
    NLS_UpString(Neu->Tree.Name);

  SearchErg = 0;
  EnterStruct.MayChange = MayChange;
  EnterStruct.DoCross = MakeCrossList;
  Neu->Tree.Attribute = (ResHandle == -2) ? MomSectionHandle : ResHandle;
  if ((SectionStack) && (Neu->Tree.Attribute == MomSectionHandle))
  {
    EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu, &(SectionStack->LocSyms),
                       1, &SearchErg);
    if (!Lauf)
      EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu,
                         &(SectionStack->GlobSyms), 2, &SearchErg);
    if (!Lauf)
      EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu,
                         &(SectionStack->ExportSyms), 3, &SearchErg);
    if (SearchErg == 2)
      Neu->Tree.Attribute = Lauf->DestSection;
    if (SearchErg == 3)
    {
      strmaxcpy(CombName, Neu->Tree.Name, STRINGSIZE);
      RunSect = SectionStack;
      MSect = MomSectionHandle;
      while ((MSect != Lauf->DestSection) && (RunSect))
      {
        strmaxprep(CombName, "_", STRINGSIZE);
        strmaxprep(CombName, GetSectionName(MSect), STRINGSIZE);
        MSect = RunSect->Handle;
        RunSect = RunSect->Next;
      }
      Copy = (PSymbolEntry) calloc(1, sizeof(TSymbolEntry));
      *Copy = (*Neu);
      Copy->Tree.Name = as_strdup(CombName);
      Copy->Tree.Attribute = Lauf->DestSection;
      Copy->SymWert.Relocs = DupRelocs(Neu->SymWert.Relocs);
      if (Copy->SymWert.Typ == TempString)
      {
        size_t l = Neu->SymWert.Contents.str.len;
        Copy->SymWert.Contents.str.p_str = (char*)malloc(l);
        memcpy(Copy->SymWert.Contents.str.p_str, Neu->SymWert.Contents.str.p_str,
               Copy->SymWert.Contents.str.len = Copy->SymWert.Contents.str.capacity = l);
      }
      EnterTree(&TreeRoot, &(Copy->Tree), SymbolAdder, &EnterStruct);
    }
    if (Lauf)
    {
      free(Lauf->Name);
      free(Lauf->pErrorPos);
      if (!Prev)
        *RRoot = Lauf->Next;
      else
        Prev->Next = Lauf->Next;
      free(Lauf);
    }
  }
  EnterTree(&TreeRoot, &(Neu->Tree), SymbolAdder, &EnterStruct);
  FirstSymbol = (PSymbolEntry)TreeRoot;
}

void PrintSymTree(char *Name)
{
  fprintf(Debug, "---------------------\n");
  fprintf(Debug, "Enter Symbol %s\n\n", Name);
  PrintSymbolTree();
  PrintSymbolDepth();
}

/*!------------------------------------------------------------------------
 * \fn     ChangeSymbol(PSymbolEntry pEntry, LargeInt Value)
 * \brief  change value of symbol in symbol table (use with caution)
 * \param  pEntry symbol entry to modify
 * \param  Value new (integer)value
 * ------------------------------------------------------------------------ */

void ChangeSymbol(PSymbolEntry pEntry, LargeInt Value)
{
  as_tempres_set_int(&pEntry->SymWert, Value);
}

/*!------------------------------------------------------------------------
 * \fn     CreateSymbolEntry(const tStrComp *pName, LongInt *pDestHandle, tSymbolFlags symbol_flags)
 * \brief  create empty container for symbol tabl eentry
 * \param  pName unexpanded symbol name
 * \param  pDestHandle section handle (out)
 * \param  symbol_flags symbol flags
 * \return * to container or NULL
 * ------------------------------------------------------------------------ */

PSymbolEntry CreateSymbolEntry(const tStrComp *pName, LongInt *pDestHandle, tSymbolFlags symbol_flags)
{
  PSymbolEntry pNeu;
  String ExtName;

  if (!ExpandStrSymbol(ExtName, sizeof(ExtName), pName))
    return NULL;
  if (!GetSymSection(ExtName, pDestHandle, pName))
    return NULL;
  (void)ChkTmp(ExtName, (symbol_flags & eSymbolFlag_Label) ? e_symbol_source_label : e_symbol_source_define);
  if (!ChkSymbName(ExtName))
  {
    WrStrErrorPos(ErrNum_InvSymName, pName);
    return NULL;
  }
  pNeu = (PSymbolEntry) calloc(1, sizeof(TSymbolEntry));
  pNeu->Tree.Name = as_strdup(ExtName);
  as_tempres_ini(&pNeu->SymWert);
  return pNeu;
}

/*!------------------------------------------------------------------------
 * \fn     EnterIntSymbolWithFlags(const tStrComp *pName, LargeInt Wert, as_addrspace_t addrspace, Boolean MayChange, tSymbolFlags Flags)
 * \brief  add integer symbol to symbol table
 * \param  pName unexpanded name
 * \param  Wert integer value
 * \param  addrspace symbol's address space
 * \param  MayChange constant or variable?
 * \param  Flags additional flags
 * \return * to newly created entry in tree
 * ------------------------------------------------------------------------ */

PSymbolEntry EnterIntSymbolWithFlags(const tStrComp *pName, LargeInt Wert, as_addrspace_t addrspace, Boolean MayChange, tSymbolFlags Flags)
{
  LongInt DestHandle;
  PSymbolEntry pNeu = CreateSymbolEntry(pName, &DestHandle, Flags);

  if (!pNeu)
    return NULL;

  as_tempres_set_int(&pNeu->SymWert, Wert);
  pNeu->SymWert.AddrSpaceMask = (addrspace != SegNone) ? 1 << addrspace : 0;
  pNeu->SymWert.Flags = Flags;
  pNeu->SymWert.DataSize = eSymbolSizeUnknown;
  pNeu->RefList = NULL;
  pNeu->SymWert.Relocs = NULL;

  if ((MomLocHandle == -1) || (DestHandle != -2))
  {
    EnterSymbol(pNeu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(pNeu->Tree.Name);
  }
  else
    EnterLocSymbol(pNeu);
  return pNeu;
}

/*!------------------------------------------------------------------------
 * \fn     EnterExtSymbol(const tStrComp *pName, LargeInt Wert, as_addrspace_t addrspace, Boolean MayChange)
 * \brief  create extended symbol
 * \param  pName unexpanded name
 * \param  Wert symbol value
 * \param  AddrSpace symbol's address space
 * \param  MayChange variable or constant?
 * ------------------------------------------------------------------------ */

void EnterExtSymbol(const tStrComp *pName, LargeInt Wert, as_addrspace_t addrspace, Boolean MayChange)
{
  LongInt DestHandle;
  PSymbolEntry pNeu = CreateSymbolEntry(pName, &DestHandle, eSymbolFlag_None);

  if (!pNeu)
    return;

  as_tempres_set_int(&pNeu->SymWert, Wert);
  pNeu->SymWert.AddrSpaceMask = (addrspace != SegNone) ? 1 << addrspace : 0;
  pNeu->SymWert.Flags = eSymbolFlag_None;
  pNeu->SymWert.DataSize = eSymbolSizeUnknown;
  pNeu->RefList = NULL;
  pNeu->SymWert.Relocs = (PRelocEntry) malloc(sizeof(TRelocEntry));
  pNeu->SymWert.Relocs->Next = NULL;
  pNeu->SymWert.Relocs->Ref = as_strdup(pNeu->Tree.Name);
  pNeu->SymWert.Relocs->Add = True;

  if ((MomLocHandle == -1) || (DestHandle != -2))
  {
    EnterSymbol(pNeu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(pNeu->Tree.Name);
  }
  else
    EnterLocSymbol(pNeu);
}

/*!------------------------------------------------------------------------
 * \fn     EnterRelSymbol(const tStrComp *pName, LargeInt Wert, as_addrspace_t addrspace, Boolean MayChange)
 * \brief  enter relocatable symbol
 * \param  pName unexpanded name
 * \param  Wert symbol value
 * \param  addrspace symbol's address space
 * \param  MayChange variable or constant?
 * \return * to created entry in tree
 * ------------------------------------------------------------------------ */

PSymbolEntry EnterRelSymbol(const tStrComp *pName, LargeInt Wert, as_addrspace_t addrspace, Boolean MayChange)
{
  LongInt DestHandle;
  PSymbolEntry pNeu = CreateSymbolEntry(pName, &DestHandle, eSymbolFlag_None);

  if (!pNeu)
    return NULL;

  as_tempres_set_int(&pNeu->SymWert, Wert);
  pNeu->SymWert.AddrSpaceMask = (addrspace != SegNone) ? 1 << addrspace : 0;
  pNeu->SymWert.Flags = eSymbolFlag_None;
  pNeu->SymWert.DataSize = eSymbolSizeUnknown;
  pNeu->RefList = NULL;
  pNeu->SymWert.Relocs = (PRelocEntry) malloc(sizeof(TRelocEntry));
  pNeu->SymWert.Relocs->Next = NULL;
  pNeu->SymWert.Relocs->Ref = as_strdup(RelName_SegStart);
  pNeu->SymWert.Relocs->Add = True;

  if ((MomLocHandle == -1) || (DestHandle != -2))
  {
    EnterSymbol(pNeu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(pNeu->Tree.Name);
  }
  else
    EnterLocSymbol(pNeu);

  return pNeu;
}

/*!------------------------------------------------------------------------
 * \fn     EnterFloatSymbol(const tStrComp *pName, Double Wert, Boolean MayChange)
 * \brief  enter floating point symbol
 * \param  pName unexpanded name
 * \param  Wert symbol value
 * \param  MayChange variable or constant?
 * ------------------------------------------------------------------------ */

void EnterFloatSymbol(const tStrComp *pName, Double Wert, Boolean MayChange)
{
  LongInt DestHandle;
  PSymbolEntry pNeu = CreateSymbolEntry(pName, &DestHandle, eSymbolFlag_None);

  if (!pNeu)
    return;

  as_tempres_set_float(&pNeu->SymWert, Wert);
  pNeu->SymWert.AddrSpaceMask = 0;
  pNeu->SymWert.Flags = eSymbolFlag_None;
  pNeu->SymWert.DataSize = eSymbolSizeUnknown;
  pNeu->RefList = NULL;
  pNeu->SymWert.Relocs = NULL;

  if ((MomLocHandle == -1) || (DestHandle != -2))
  {
    EnterSymbol(pNeu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(pNeu->Tree.Name);
  }
  else
    EnterLocSymbol(pNeu);
}

/*!------------------------------------------------------------------------
 * \fn     EnterNonZStringSymbolWithFlags(const tStrComp *pName, const as_nonz_dynstr_t *p_value, Boolean MayChange, tSymbolFlags Flags)
 * \brief  enter string symbol
 * \param  pName unexpanded name
 * \param  pValue symbol value
 * \param  MayChange variable or constant?
 * \param  Flags special symbol flags to store
 * ------------------------------------------------------------------------ */

void EnterNonZStringSymbolWithFlags(const tStrComp *pName, const as_nonz_dynstr_t *p_value, Boolean MayChange, tSymbolFlags Flags)
{
  LongInt DestHandle;
  PSymbolEntry pNeu = CreateSymbolEntry(pName, &DestHandle, Flags);

  if (!pNeu)
    return;

  /* TODO: TempRes alloc exact len */
  pNeu->SymWert.Contents.str.p_str = (char*)malloc(p_value->len);
  memcpy(pNeu->SymWert.Contents.str.p_str, p_value->p_str, p_value->len);
  pNeu->SymWert.Contents.str.len =
  pNeu->SymWert.Contents.str.capacity = p_value->len;
  pNeu->SymWert.Typ = TempString;
  pNeu->SymWert.AddrSpaceMask = 0;
  pNeu->SymWert.Flags = Flags;
  pNeu->SymWert.DataSize = eSymbolSizeUnknown;
  pNeu->RefList = NULL;
  pNeu->SymWert.Relocs = NULL;

  if ((MomLocHandle == -1) || (DestHandle != -2))
  {
    EnterSymbol(pNeu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(pNeu->Tree.Name);
  }
  else
    EnterLocSymbol(pNeu);
}

/*!------------------------------------------------------------------------
 * \fn     EnterStringSymbol(const tStrComp *pName, const char *pValue, Boolean MayChange)
 * \brief  enter string symbol
 * \param  pName unexpanded name
 * \param  pValue symbol value
 * \param  MayChange variable or constant?
 * ------------------------------------------------------------------------ */

void EnterStringSymbol(const tStrComp *pName, const char *pValue, Boolean MayChange)
{
  as_nonz_dynstr_t NonZString;

  as_nonz_dynstr_ini_c_str(&NonZString, pValue);
  EnterNonZStringSymbol(pName, &NonZString, MayChange);
  as_nonz_dynstr_free(&NonZString);
}

/*!------------------------------------------------------------------------
 * \fn     EnterRegSymbol(const struct sStrComp *pName, const tRegDescr *pDescr, tSymbolSize Size, Boolean MayChange, Boolean AddList)
 * \brief  enter register symbol
 * \param  pName unexpanded name
 * \param  pDescr register's numeric value & associated dissector
 * \param  Size register's data size
 * \param  MayChange variable or constant?
 * \param  AddList add value to listing?
 * ------------------------------------------------------------------------ */

void EnterRegSymbol(const struct sStrComp *pName, const tRegDescr *pDescr, tSymbolSize Size, Boolean MayChange, Boolean AddList)
{
  LongInt DestHandle;
  PSymbolEntry pNeu = CreateSymbolEntry(pName, &DestHandle, eSymbolFlag_None);

  if (!pNeu)
    return;

  as_tempres_set_reg(&pNeu->SymWert, pDescr);
  pNeu->SymWert.AddrSpaceMask = 0;
  pNeu->SymWert.Flags = eSymbolFlag_None;
  pNeu->SymWert.DataSize = Size;
  pNeu->RefList = NULL;
  pNeu->SymWert.Relocs = NULL;

  if ((MomLocHandle == -1) || (DestHandle != -2))
  {
    EnterSymbol(pNeu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(pNeu->Tree.Name);
    RegistersDefined = True;
  }
  else
    EnterLocSymbol(pNeu);

  if (AddList)
  {
    *ListLine = '=';
    pDescr->Dissect(&ListLine[1], STRINGSIZE - 1, pDescr->Reg, Size);
  }
}

static void AddReference(PSymbolEntry Node)
{
  PCrossRef Lauf, Neu;

  /* Speicher belegen */

  Neu = (PCrossRef) malloc(sizeof(TCrossRef));
  Neu->LineNum = CurrLine;
  Neu->OccNum = 1;
  Neu->Next = NULL;

  /* passende Datei heraussuchen */

  Neu->FileNum = GetFileNum(CurrFileName);

  /* suchen, ob Eintrag schon existiert */

  Lauf = Node->RefList;
  while ((Lauf)
     && ((Lauf->FileNum != Neu->FileNum) || (Lauf->LineNum != Neu->LineNum)))
   Lauf = Lauf->Next;

  /* schon einmal in dieser Datei in dieser Zeile aufgetaucht: nur Zaehler
    rauf: */

  if (Lauf)
  {
    Lauf->OccNum++;
   free(Neu);
  }

  /* ansonsten an Kettenende anhaengen */

  else if (!Node->RefList) Node->RefList = Neu;

  else
  {
    Lauf = Node->RefList;
    while (Lauf->Next)
      Lauf = Lauf->Next;
    Lauf->Next = Neu;
  }
}

static PSymbolEntry FindNode_FNode(char *Name, TempType SearchType, LongInt Handle)
{
  PSymbolEntry Lauf;

  Lauf = (PSymbolEntry) SearchTree((PTree)FirstSymbol, Name, Handle);

  if (Lauf)
  {
    if (Lauf->SymWert.Typ & SearchType)
    {
      if (MakeCrossList && DoRefs)
        AddReference(Lauf);
    }
    else
      Lauf = NULL;
  }

  return Lauf;
}

static Boolean FindNode_FSpec(char *Name, PForwardSymbol Root)
{
  while ((Root) && (strcmp(Root->Name, Name)))
    Root = Root->Next;
  return (Root != NULL);
}

static PSymbolEntry FindNode(const char *Name_O, TempType SearchType)
{
  PSaveSection Lauf;
  LongInt DestSection;
  PSymbolEntry Result = NULL;
  String Name;

  strmaxcpy(Name, Name_O, STRINGSIZE);
  ChkTmp3(Name, e_symbol_source_none);

  /* TODO: pass StrComp */
  if (!GetSymSection(Name, &DestSection, NULL))
    return NULL;

  if (!CaseSensitive)
    NLS_UpString(Name);

  if (SectionStack)
    if (PassNo <= MaxSymPass)
      if (FindNode_FSpec(Name, SectionStack->LocSyms)) DestSection = MomSectionHandle;

  if (DestSection == -2)
  {
    Result = FindNode_FNode(Name, SearchType, MomSectionHandle);
    if (Result)
      return Result;
    Lauf = SectionStack;
    while (Lauf)
    {
      Result = FindNode_FNode(Name, SearchType, Lauf->Handle);
      if (Result)
        break;
      Lauf = Lauf->Next;
    }
  }
  else
    Result = FindNode_FNode(Name, SearchType, DestSection);

  return Result;
}

static PSymbolEntry FindLocNode_FNode(char *Name, TempType SearchType, LongInt Handle)
{
  PSymbolEntry Lauf;

  Lauf = (PSymbolEntry) SearchTree((PTree)FirstLocSymbol, Name, Handle);

  if (Lauf)
  {
    if (!(Lauf->SymWert.Typ & SearchType))
      Lauf = NULL;
  }

  return Lauf;
}

static PSymbolEntry FindLocNode(const char *Name_O, TempType SearchType)
{
  PLocHandle RunLocHandle;
  PSymbolEntry Result = NULL;
  String Name;

  strmaxcpy(Name, Name_O, STRINGSIZE);
  ChkTmp3(Name, e_symbol_source_none);
  if (!CaseSensitive)
    NLS_UpString(Name);

  if (MomLocHandle == -1)
    return NULL;

  Result = FindLocNode_FNode(Name, SearchType, MomLocHandle);
  if (Result)
    return Result;

  RunLocHandle = FirstLocHandle;
  while ((RunLocHandle) && (RunLocHandle->Cont != -1))
  {
    Result = FindLocNode_FNode(Name, SearchType, RunLocHandle->Cont);
    if (Result)
      break;
    RunLocHandle = RunLocHandle->Next;
  }

  return Result;
}
/**
void SetSymbolType(const tStrComp *pName, Byte NTyp)
{
  PSymbolEntry Lauf;
  Boolean HRef;
  String ExpName;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pName))
    return;
  HRef = DoRefs;
  DoRefs = False;
  Lauf = FindLocNode(ExpName, TempInt);
  if (!Lauf)
    Lauf = FindNode(ExpName, TempInt);
  if (Lauf)
    Lauf->SymType = NTyp;
  DoRefs = HRef;
}
**/

void LookupSymbol(const struct sStrComp *pComp, TempResult *pValue, Boolean WantRelocs, TempType ReqType)
{
  PSymbolEntry pEntry;
  String ExpName;
  const char *pKlPos;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pComp))
  {
    pValue->Typ = TempNone;
    return;
  }

  pKlPos = strchr(ExpName, '[');
  if (ChkSymbNameUpTo(ExpName, pKlPos) != pKlPos)
  {
    WrStrErrorPos(ErrNum_InvSymName, pComp);
    pValue->Typ = TempNone;
    return;
  }

  pEntry = FindLocNode(ExpName, ReqType);
  if (!pEntry)
    pEntry = FindNode(ExpName, ReqType);
  if (pEntry)
  {
    as_tempres_copy_value(pValue, &pEntry->SymWert);
    if (pValue->Typ != TempNone)
    {
      if (WantRelocs)
        pValue->Relocs = DupRelocs(pEntry->SymWert.Relocs);
      pValue->Flags = pEntry->SymWert.Flags;
    }
    pValue->AddrSpaceMask = pEntry->SymWert.AddrSpaceMask;
    if ((pEntry->SymWert.DataSize != eSymbolSizeUnknown) && (pValue->DataSize == eSymbolSizeUnknown))
      pValue->DataSize = pEntry->SymWert.DataSize;
    if (!pEntry->Defined)
    {
      if (Repass)
        pValue->Flags |= eSymbolFlag_Questionable;
      pValue->Flags |= eSymbolFlag_UsesForwards;
    }
    pEntry->Used = True;
  }

  /* Symbol evtl. im ersten Pass unbekannt */

  else if (PassNo <= MaxSymPass) /* !pEntry */
  {
    as_tempres_set_int(pValue, EProgCounter());
    Repass = True;
    if ((MsgIfRepass) && (PassNo >= PassNoForMessage))
      WrStrErrorPos(ErrNum_RepassUnknown, pComp);
    pValue->Flags |= eSymbolFlag_FirstPassUnknown;
  }
  else
    WrStrErrorPos(ErrNum_SymbolUndef, pComp);
}

/*!------------------------------------------------------------------------
 * \fn     SetSymbolOrStructElemSize(const struct sStrComp *pName, tSymbolSize Size)
 * \brief  set (integer) data size associated with a symbol
 * \param  pName unexpanded name of symbol
 * \param  Size operand size to set
 * ------------------------------------------------------------------------ */

void SetSymbolOrStructElemSize(const struct sStrComp *pName, tSymbolSize Size)
{
  if (pInnermostNamedStruct)
    SetStructElemSize(pInnermostNamedStruct->StructRec, pName->str.p_str, Size);
  else
  {
    PSymbolEntry pEntry;
    Boolean HRef;
    String ExpName;

    if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pName))
      return;
    HRef = DoRefs;
    DoRefs = False;
    pEntry = FindLocNode(ExpName, TempInt);
    if (!pEntry)
      pEntry = FindNode(ExpName, TempInt);
    if (pEntry)
      pEntry->SymWert.DataSize = Size;
    DoRefs = HRef;
  }
}

/*!------------------------------------------------------------------------
 * \fn     GetSymbolSize(const struct sStrComp *pName)
 * \brief  get symbol's integer size
 * \param  pName unexpanded symbol name
 * \return symbol size or -1 if symbol does not exist
 * ------------------------------------------------------------------------ */

ShortInt GetSymbolSize(const struct sStrComp *pName)
{
  PSymbolEntry pEntry;
  String ExpName;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pName))
     return -1;
  pEntry = FindLocNode(ExpName, TempInt);
  if (!pEntry)
    pEntry = FindNode(ExpName, TempInt);
  return pEntry ? pEntry->SymWert.DataSize : eSymbolSizeUnknown;
}

/*!------------------------------------------------------------------------
 * \fn     IsSymbolDefined(const struct sStrComp *pName)
 * \brief  check whether symbol nas been used so far
 * \param  pName unexpanded symbol name
 * \return true if symbol exists and has been defined so far
 * ------------------------------------------------------------------------ */

Boolean IsSymbolDefined(const struct sStrComp *pName)
{
  PSymbolEntry pEntry;
  String ExpName;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pName))
    return False;

  pEntry = FindLocNode(ExpName, TempAll);
  if (!pEntry)
    pEntry = FindNode(ExpName, TempAll);
  return pEntry && pEntry->Defined;
}

/*!------------------------------------------------------------------------
 * \fn     IsSymbolUsed(const struct sStrComp *pName)
 * \brief  check whether symbol nas been used so far
 * \param  pName unexpanded symbol name
 * \return true if symbol exists and has been used
 * ------------------------------------------------------------------------ */

Boolean IsSymbolUsed(const struct sStrComp *pName)
{
  PSymbolEntry pEntry;
  String ExpName;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pName))
    return False;

  pEntry = FindLocNode(ExpName, TempAll);
  if (!pEntry)
    pEntry = FindNode(ExpName, TempAll);
  return pEntry && pEntry->Used;
}

/*!------------------------------------------------------------------------
 * \fn     IsSymbolChangeable(const struct sStrComp *pName)
 * \brief  check whether symbol's value may be changed or is constant
 * \param  pName unexpanded symbol name
 * \return true if symbol exists and is changeable
 * ------------------------------------------------------------------------ */

Boolean IsSymbolChangeable(const struct sStrComp *pName)
{
  PSymbolEntry pEntry;
  String ExpName;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pName))
    return False;

  pEntry = FindLocNode(ExpName, TempAll);
  if (!pEntry)
    pEntry = FindNode(ExpName, TempAll);
  return pEntry && pEntry->Changeable;
}

/*!------------------------------------------------------------------------
 * \fn     GetSymbolType(const struct sStrComp *pName)
 * \brief  retrieve type (int/float/string) of symbol
 * \param  pName unexpanded name
 * \return type or -1 if non-existent
 * ------------------------------------------------------------------------ */

as_addrspace_t addrspace_from_mask(unsigned mask)
{
  as_addrspace_t space;

  for (space = SegCode; space < SegCount; space++)
    if (mask & (1 << space))
      return space;
  return SegNone;
}

Integer GetSymbolType(const struct sStrComp *pName)
{
  PSymbolEntry pEntry;
  String ExpName;

  if (!ExpandStrSymbol(ExpName, sizeof(ExpName), pName))
    return -1;

  pEntry = FindLocNode(ExpName, TempAll);
  if (!pEntry)
    pEntry = FindNode(ExpName, TempAll);

  if (!pEntry)
    return -1;
  else if (pEntry->SymWert.Typ == TempReg)
    return 0x80;
  else
    return addrspace_from_mask(pEntry->SymWert.AddrSpaceMask);
}

typedef struct
{
  int Width, cwidth;
  LongInt Sum, USum;
  as_dynstr_t Zeilenrest;
  int ZeilenrestLen,
      ZeilenrestVisibleLen;
  as_dynstr_t s1, sh;
} TListContext;

static void PrintSymbolList_AddOut(char *s, TListContext *pContext)
{
  int AddVisibleLen = visible_strlen(s),
      AddLen = strlen(s);

  if (AddVisibleLen + pContext->ZeilenrestVisibleLen > pContext->Width)
  {
    pContext->Zeilenrest.p_str[pContext->ZeilenrestLen - 1] = '\0';
    WrLstLine(pContext->Zeilenrest.p_str);
    as_dynstr_copy_c_str(&pContext->Zeilenrest, s);
    pContext->ZeilenrestLen = AddLen;
    pContext->ZeilenrestVisibleLen = AddVisibleLen;
  }
  else
  {
    as_dynstr_append_c_str(&pContext->Zeilenrest, s);
    pContext->ZeilenrestLen += AddLen;
    pContext->ZeilenrestVisibleLen += AddVisibleLen;
  }
}

static void PrintSymbolList_PNode(PTree Tree, void *pData)
{
  PSymbolEntry Node = (PSymbolEntry) Tree;

  if (Node->SymWert.Typ != TempReg)
  {
    TListContext *pContext = (TListContext*) pData;
    int l1, nBlanks;
    const TempResult *pValue = &Node->SymWert;

    if ((pValue->Typ == TempInt) && DissectBit && (pValue->AddrSpaceMask & (1 << SegBData)))
      DissectBit(pContext->s1.p_str, pContext->s1.capacity, pValue->Contents.Int);
    else
      StrSym(pValue, False, &pContext->s1, ListRadixBase);

    as_sdprintf(&pContext->sh, "%c%s : ", Node->Used ? ' ' : '*', Tree->Name);
    if (Tree->Attribute != -1)
      as_sdprcatf(&pContext->sh, " [%s]", GetSectionName(Tree->Attribute));
    l1 = (strlen(pContext->s1.p_str) + visible_strlen(pContext->sh.p_str) + 4);
    for (nBlanks = pContext->cwidth - 1 - l1; nBlanks < 0; nBlanks += pContext->cwidth);
    as_sdprcatf(&pContext->sh, "%s%s %c | ", Blanks(nBlanks), pContext->s1.p_str, SegShorts[addrspace_from_mask(pValue->AddrSpaceMask)]);
    PrintSymbolList_AddOut(pContext->sh.p_str, pContext);
    pContext->Sum++;
    if (!Node->Used)
      pContext->USum++;
  }
}

void PrintSymbolList(void)
{
  int ActPageWidth;
  TListContext Context;

  as_dynstr_ini(&Context.Zeilenrest, STRINGSIZE);
  as_dynstr_ini(&Context.s1, STRINGSIZE);
  as_dynstr_ini(&Context.sh, STRINGSIZE);
  Context.Width = (PageWidth == 0) ? 80 : PageWidth;
  NewPage(ChapDepth, True);
  WrLstLine(getmessage(Num_ListSymListHead1));
  WrLstLine(getmessage(Num_ListSymListHead2));
  WrLstLine("");

  Context.ZeilenrestLen =
  Context.ZeilenrestVisibleLen = 0;
  Context.Sum = Context.USum = 0;
  ActPageWidth = (PageWidth == 0) ? 80 : PageWidth;
  Context.cwidth = ActPageWidth >> 1;
  IterTree((PTree)FirstSymbol, PrintSymbolList_PNode, &Context);
  if (Context.Zeilenrest.p_str[0] != '\0')
  {
    Context.Zeilenrest.p_str[strlen(Context.Zeilenrest.p_str) - 1] = '\0';
    WrLstLine(Context.Zeilenrest.p_str);
  }
  WrLstLine("");
  as_sdprintf(&Context.Zeilenrest, "%7lu%s",
              (unsigned long)Context.Sum,
              getmessage((Context.Sum == 1) ? Num_ListSymSumMsg : Num_ListSymSumsMsg));
  WrLstLine(Context.Zeilenrest.p_str);
  as_sdprintf(&Context.Zeilenrest, "%7lu%s",
              (unsigned long)Context.USum,
              getmessage((Context.USum == 1) ? Num_ListUSymSumMsg : Num_ListUSymSumsMsg));
  WrLstLine(Context.Zeilenrest.p_str);
  WrLstLine("");
  as_dynstr_free(&Context.s1);
  as_dynstr_free(&Context.sh);
  as_dynstr_free(&Context.Zeilenrest);
}

typedef struct
{
  FILE *f;
  Boolean HWritten;
  as_addrspace_t Space;
  as_dynstr_t s;
} TDebContext;

static void PrintDebSymbols_PNode(PTree Tree, void *pData)
{
  PSymbolEntry Node = (PSymbolEntry) Tree;
  TDebContext *DebContext = (TDebContext*) pData;
  int l1;

  if (!((Node->SymWert.AddrSpaceMask >> DebContext->Space) & 1))
    return;

  if (!DebContext->HWritten)
  {
    fprintf(DebContext->f, "\n"); ChkIO(ErrNum_FileWriteError);
    fprintf(DebContext->f, "Symbols in Segment %s\n", SegNames[DebContext->Space]); ChkIO(ErrNum_FileWriteError);
    DebContext->HWritten = True;
  }

  fprintf(DebContext->f, "%s", Node->Tree.Name); ChkIO(ErrNum_FileWriteError);
  l1 = strlen(Node->Tree.Name);
  if (Node->Tree.Attribute != -1)
  {
    as_sdprintf(&DebContext->s, "[%d]", (int)Node->Tree.Attribute);
    fprintf(DebContext->f, "%s", DebContext->s.p_str); ChkIO(ErrNum_FileWriteError);
    l1 += strlen(DebContext->s.p_str);
  }
  fprintf(DebContext->f, "%s ", Blanks(37 - l1)); ChkIO(ErrNum_FileWriteError);
  switch (Node->SymWert.Typ)
  {
    case TempInt:
      fprintf(DebContext->f, "Int    ");
      break;
    case TempFloat:
      fprintf(DebContext->f, "Float  ");
      break;
    case TempString:
      fprintf(DebContext->f, "String ");
      break;
    default:
      break;
  }
  ChkIO(ErrNum_FileWriteError);
  if (Node->SymWert.Typ == TempString)
  {
    errno = 0;
    l1 = fstrlenprint(DebContext->f, Node->SymWert.Contents.str.p_str, Node->SymWert.Contents.str.len);
    ChkIO(ErrNum_FileWriteError);
  }
  else
  {
    StrSym(&Node->SymWert, False, &DebContext->s, 16);
    l1 = strlen(DebContext->s.p_str);
    fprintf(DebContext->f, "%s", DebContext->s.p_str); ChkIO(ErrNum_FileWriteError);
  }
  fprintf(DebContext->f, "%s %-3d %d %d\n", Blanks(25 - l1), Node->SymWert.DataSize, (int)Node->Used, (int)Node->Changeable);
  ChkIO(ErrNum_FileWriteError);
}

void PrintDebSymbols(FILE *f)
{
  TDebContext DebContext;

  as_dynstr_ini(&DebContext.s, 256);
  DebContext.f = f;
  for (DebContext.Space = SegNone; DebContext.Space < SegCount; DebContext.Space++)
  {
    DebContext.HWritten = False;
    IterTree((PTree)FirstSymbol, PrintDebSymbols_PNode, &DebContext);
  }
  as_dynstr_free(&DebContext.s);
}

typedef struct
{
  FILE *f;
  LongInt Handle;
} TNoISymContext;

static void PrNoISection(PTree Tree, void *pData)
{
  PSymbolEntry Node = (PSymbolEntry)Tree;
  TNoISymContext *pContext = (TNoISymContext*) pData;

  if ((Node->SymWert.AddrSpaceMask & NoICEMask) && (Node->Tree.Attribute == pContext->Handle) && (Node->SymWert.Typ == TempInt))
  {
    errno = 0; fprintf(pContext->f, "DEFINE %s 0x", Node->Tree.Name); ChkIO(ErrNum_FileWriteError);
    errno = 0; fprintf(pContext->f, "%" PRIx64, (LargeWord)Node->SymWert.Contents.Int); ChkIO(ErrNum_FileWriteError);
    errno = 0; fprintf(pContext->f, "\n"); ChkIO(ErrNum_FileWriteError);
  }
}

void PrintNoISymbols(FILE *f)
{
  PCToken CurrSection;
  TNoISymContext Context;

  Context.f = f;
  Context.Handle = -1;
  IterTree((PTree)FirstSymbol, PrNoISection, &Context);
  Context.Handle++;
  for (CurrSection = FirstSection; CurrSection; CurrSection = CurrSection->Next)
   if (ChunkSum(&CurrSection->Usage)>0)
   {
     fprintf(f, "FUNCTION %s ", CurrSection->Name); ChkIO(ErrNum_FileWriteError);
     fprintf(f, "%" PRIu64, ChunkMin(&CurrSection->Usage)); ChkIO(ErrNum_FileWriteError);
     fprintf(f, "\n"); ChkIO(ErrNum_FileWriteError);
     IterTree((PTree)FirstSymbol, PrNoISection, &Context);
     Context.Handle++;
     fprintf(f, "}FUNC "); ChkIO(ErrNum_FileWriteError);
     fprintf(f, "%" PRIu64, ChunkMax(&CurrSection->Usage)); ChkIO(ErrNum_FileWriteError);
     fprintf(f, "\n"); ChkIO(ErrNum_FileWriteError);
   }
}

void PrintSymbolTree(void)
{
  DumpTree((PTree)FirstSymbol);
}

static void ClearSymbolList_ClearNode(PTree Node, void *pData)
{
  PSymbolEntry SymbolEntry = (PSymbolEntry) Node;
  UNUSED(pData);

  FreeSymbolEntry(&SymbolEntry, FALSE);
}

void ClearSymbolList(void)
{
  PTree TreeRoot;

  TreeRoot = &(FirstSymbol->Tree);
  FirstSymbol = NULL;
  DestroyTree(&TreeRoot, ClearSymbolList_ClearNode, NULL);
  TreeRoot = &(FirstLocSymbol->Tree);
  FirstLocSymbol = NULL;
  DestroyTree(&TreeRoot, ClearSymbolList_ClearNode, NULL);
}

/*-------------------------------------------------------------------------*/
/* Stack-Verwaltung */

Boolean PushSymbol(const tStrComp *pSymName, const tStrComp *pStackName)
{
  PSymbolEntry pSrc;
  PSymbolStack LStack, NStack, PStack;
  PSymbolStackEntry Elem;
  String ExpSymName, ExpStackName;

  if (!ExpandStrSymbol(ExpSymName, sizeof(ExpSymName), pSymName))
    return False;

  pSrc = FindNode(ExpSymName, TempAll);
  if (!pSrc)
  {
    WrStrErrorPos(ErrNum_SymbolUndef, pSymName);
    return False;
  }

  if (*pStackName->str.p_str)
  {
    if (!ExpandStrSymbol(ExpStackName, sizeof(ExpStackName), pStackName))
      return False;
  }
  else
    strmaxcpy(ExpStackName, DefStackName, STRINGSIZE);
  if (!ChkSymbName(ExpStackName))
  {
    WrStrErrorPos(ErrNum_InvSymName, pStackName);
    return False;
  }

  LStack = FirstStack;
  PStack = NULL;
  while ((LStack) && (strcmp(LStack->Name, ExpStackName) < 0))
  {
    PStack = LStack;
    LStack = LStack->Next;
  }

  if ((!LStack) || (strcmp(LStack->Name, ExpStackName) > 0))
  {
    NStack = (PSymbolStack) malloc(sizeof(TSymbolStack));
    NStack->Name = as_strdup(ExpStackName);
    NStack->Contents = NULL;
    NStack->Next = LStack;
    if (!PStack)
      FirstStack = NStack;
    else
      PStack->Next = NStack;
    LStack = NStack;
  }

  Elem = (PSymbolStackEntry) malloc(sizeof(TSymbolStackEntry));
  Elem->Next = LStack->Contents;
  Elem->Contents = pSrc->SymWert;
  LStack->Contents = Elem;

  return True;
}

Boolean PopSymbol(const tStrComp *pSymName, const tStrComp *pStackName)
{
  PSymbolEntry pDest;
  PSymbolStack LStack, PStack;
  PSymbolStackEntry Elem;
  String ExpSymName, ExpStackName;

  if (!ExpandStrSymbol(ExpSymName, sizeof(ExpSymName), pSymName))
    return False;

  pDest = FindNode(ExpSymName, TempAll);
  if (!pDest)
  {
    WrStrErrorPos(ErrNum_SymbolUndef, pSymName);
    return False;
  }

  if (*pStackName->str.p_str)
  {
    if (!ExpandStrSymbol(ExpStackName, sizeof(ExpStackName), pStackName))
      return False;
  }
  else
    strmaxcpy(ExpStackName, DefStackName, STRINGSIZE);
  if (!ChkSymbName(ExpStackName))
  {
    WrStrErrorPos(ErrNum_InvSymName, pStackName);
    return False;
  }

  LStack = FirstStack;
  PStack = NULL;
  while ((LStack) && (strcmp(LStack->Name, ExpStackName) < 0))
  {
    PStack = LStack;
    LStack = LStack->Next;
  }

  if ((!LStack) || (strcmp(LStack->Name, ExpStackName) > 0))
  {
    WrStrErrorPos(ErrNum_StackEmpty, pStackName);
    return False;
  }

  Elem = LStack->Contents;
  pDest->SymWert = Elem->Contents;
  LStack->Contents = Elem->Next;
  if (!LStack->Contents)
  {
    if (!PStack)
      FirstStack = LStack->Next;
    else
      PStack->Next = LStack->Next;
    free(LStack->Name);
    free(LStack);
  }
  free(Elem);

  return True;
}

void ClearStacks(void)
{
  PSymbolStack Act;
  PSymbolStackEntry Elem;
  int z;
  String s;

  while (FirstStack)
  {
    z = 0;
    Act = FirstStack;
    while (Act->Contents)
    {
      Elem = Act->Contents;
      Act->Contents = Elem->Next;
      free(Elem);
      z++;
    }
    as_snprintf(s, sizeof(s), "%s(%d)", Act->Name, z);
    WrXError(ErrNum_StackNotEmpty, s);
    free(Act->Name);
    FirstStack = Act->Next;
    free(Act);
  }
}

/*-------------------------------------------------------------------------*/
/* Funktionsverwaltung */

void EnterFunction(const tStrComp *pComp, char *FDefinition, Byte NewCnt)
{
  PFunction Neu;
  String FName_N;
  const char *pFName;

  if (!CaseSensitive)
  {
    strmaxcpy(FName_N, pComp->str.p_str, STRINGSIZE);
    NLS_UpString(FName_N);
    pFName = FName_N;
  }
  else
     pFName = pComp->str.p_str;

  if (!ChkSymbName(pFName))
  {
    WrStrErrorPos(ErrNum_InvSymName, pComp);
    return;
  }

  if (FindFunction(pFName))
  {
    if (PassNo == 1)
      WrStrErrorPos(ErrNum_DoubleDef, pComp);
    return;
  }

  Neu = (PFunction) malloc(sizeof(TFunction));
  Neu->Next = FirstFunction;
  Neu->ArguCnt = NewCnt;
  Neu->Name = as_strdup(pFName);
  Neu->Definition = as_strdup(FDefinition);
  FirstFunction = Neu;
}

PFunction FindFunction(const char *Name)
{
  PFunction Lauf = FirstFunction;
  String Name_N;

  if (!CaseSensitive)
  {
    strmaxcpy(Name_N, Name, STRINGSIZE);
    NLS_UpString(Name_N);
    Name = Name_N;
  }

  while ((Lauf) && (strcmp(Lauf->Name, Name)))
    Lauf = Lauf->Next;
  return Lauf;
}

void PrintFunctionList(void)
{
  PFunction Lauf;
  String OneS;
  Boolean cnt;

  if (!FirstFunction)
    return;

  NewPage(ChapDepth, True);
  WrLstLine(getmessage(Num_ListFuncListHead1));
  WrLstLine(getmessage(Num_ListFuncListHead2));
  WrLstLine("");

  OneS[0] = '\0';
  Lauf = FirstFunction;
  cnt = False;
  while (Lauf)
  {
    strmaxcat(OneS, Lauf->Name, STRINGSIZE);
    if (strlen(Lauf->Name) < 37)
      strmaxcat(OneS, Blanks(37-strlen(Lauf->Name)), STRINGSIZE);
    if (!cnt) strmaxcat(OneS, " | ", STRINGSIZE);
    else
    {
      WrLstLine(OneS);
      OneS[0] = '\0';
    }
    cnt = !cnt;
    Lauf = Lauf->Next;
  }
  if (cnt)
  {
    OneS[strlen(OneS)-1] = '\0';
    WrLstLine(OneS);
  }
  WrLstLine("");
}

void ClearFunctionList(void)
{
  PFunction Lauf;

  while (FirstFunction)
  {
    Lauf = FirstFunction->Next;
    free(FirstFunction->Name);
    free(FirstFunction->Definition);
    free(FirstFunction);
    FirstFunction = Lauf;
  }
}

/*-------------------------------------------------------------------------*/

static void ResetSymbolDefines_ResetNode(PTree Node, void *pData)
{
  PSymbolEntry SymbolEntry = (PSymbolEntry) Node;
  UNUSED(pData);

  SymbolEntry->Defined = False;
  SymbolEntry->Used = False;
}

void ResetSymbolDefines(void)
{
  IterTree(&(FirstSymbol->Tree), ResetSymbolDefines_ResetNode, NULL);
  IterTree(&(FirstLocSymbol->Tree), ResetSymbolDefines_ResetNode, NULL);
}

void SetFlag(Boolean *Flag, const char *Name, Boolean Wert)
{
  tStrComp TmpComp;

  *Flag = Wert;
  StrCompMkTemp(&TmpComp, (char*)Name, 0);
  EnterIntSymbol(&TmpComp, *Flag ? 1 : 0, SegNone, True);
}

void AddDefSymbol(char *Name, TempResult *Value)
{
  PDefSymbol Neu;

  Neu = FirstDefSymbol;
  while (Neu)
  {
    if (!strcmp(Neu->SymName, Name))
      return;
    Neu = Neu->Next;
  }

  Neu = (PDefSymbol) malloc(sizeof(TDefSymbol));
  Neu->Next = FirstDefSymbol;
  Neu->SymName = as_strdup(Name);
  Neu->Wert = (*Value);
  FirstDefSymbol = Neu;
}

void RemoveDefSymbol(char *Name)
{
  PDefSymbol Save, Lauf;

  if (!FirstDefSymbol)
    return;

  if (!strcmp(FirstDefSymbol->SymName, Name))
  {
    Save = FirstDefSymbol;
    FirstDefSymbol = FirstDefSymbol->Next;
  }
  else
  {
    Lauf = FirstDefSymbol;
    while ((Lauf->Next) && (strcmp(Lauf->Next->SymName, Name)))
      Lauf = Lauf->Next;
    if (!Lauf->Next)
      return;
    Save = Lauf->Next;
    Lauf->Next = Lauf->Next->Next;
  }
  free(Save->SymName);
  free(Save);
}

void CopyDefSymbols(void)
{
  PDefSymbol Lauf;
  tStrComp TmpComp;

  Lauf = FirstDefSymbol;
  while (Lauf)
  {
    StrCompMkTemp(&TmpComp, Lauf->SymName, 0);
    switch (Lauf->Wert.Typ)
    {
      case TempInt:
        EnterIntSymbol(&TmpComp, Lauf->Wert.Contents.Int, SegNone, True);
        break;
      case TempFloat:
        EnterFloatSymbol(&TmpComp, Lauf->Wert.Contents.Float, True);
        break;
      case TempString:
      {
        EnterNonZStringSymbol(&TmpComp, &Lauf->Wert.Contents.str, True);
        break;
      }
      default:
        break;
    }
    Lauf = Lauf->Next;
  }
}

const TempResult *FindDefSymbol(const char *pName)
{
  PDefSymbol pRun;

  for (pRun = FirstDefSymbol; pRun; pRun = pRun->Next)
    if (!strcmp(pName, pRun->SymName))
      return &pRun->Wert;
  return NULL;
}

void PrintSymbolDepth(void)
{
  LongInt TreeMin, TreeMax;

  GetTreeDepth(&(FirstSymbol->Tree), &TreeMin, &TreeMax);
  fprintf(Debug, " MinTree %ld\n", (long)TreeMin);
  fprintf(Debug, " MaxTree %ld\n", (long)TreeMax);
}

LongInt GetSectionHandle(char *SName_O, Boolean AddEmpt, LongInt Parent)
{
  PCToken Lauf, Prev;
  LongInt z;
  String SName;

  strmaxcpy(SName, SName_O, STRINGSIZE);
  if (!CaseSensitive)
    NLS_UpString(SName);

  Lauf = FirstSection;
  Prev = NULL;
  z = 0;
  while ((Lauf) && ((strcmp(Lauf->Name, SName)) || (Lauf->Parent != Parent)))
  {
    z++;
    Prev = Lauf;
    Lauf = Lauf->Next;
  }

  if (!Lauf)
  {
    if (AddEmpt)
    {
      Lauf = (PCToken) malloc(sizeof(TCToken));
      Lauf->Parent = MomSectionHandle;
      Lauf->Name = as_strdup(SName);
      Lauf->Next = NULL;
      InitChunk(&(Lauf->Usage));
      if (!Prev)
        FirstSection = Lauf;
      else
        Prev->Next = Lauf;
    }
    else
      z = -2;
  }
  return z;
}

const char *GetSectionName(LongInt Handle)
{
  PCToken Lauf = FirstSection;
  static const char *Dummy = "";

  if (Handle == -1)
    return Dummy;
  while ((Handle > 0) && (Lauf))
  {
    Lauf = Lauf->Next;
    Handle--;
  }
  return Lauf ? Lauf->Name : Dummy;
}

void SetMomSection(LongInt Handle)
{
  LongInt z;

  MomSectionHandle = Handle;
  if (Handle < 0)
    MomSection = NULL;
  else
  {
    MomSection = FirstSection;
    for (z = 1; z <= Handle; z++)
      if (MomSection)
        MomSection = MomSection->Next;
  }
}

void AddSectionUsage(LongInt Start, LongInt Length)
{
  if ((ActPC != SegCode) || (!MomSection))
    return;
  AddChunk(&(MomSection->Usage), Start, Length, False);
}

void ClearSectionUsage(void)
{
  PCToken Tmp;

  for (Tmp = FirstSection; Tmp; Tmp = Tmp->Next)
    ClearChunk(&(Tmp->Usage));
}

static void PrintSectionList_PSection(LongInt Handle, int Indent)
{
  PCToken Lauf;
  LongInt Cnt;
  String h;

  if (Handle != -1)
  {
    strmaxcpy(h, Blanks(Indent << 1), STRINGSIZE);
    strmaxcat(h, GetSectionName(Handle), STRINGSIZE);
    WrLstLine(h);
  }
  Lauf = FirstSection;
  Cnt = 0;
  while (Lauf)
  {
    if (Lauf->Parent == Handle)
      PrintSectionList_PSection(Cnt, Indent + 1);
    Lauf = Lauf->Next;
    Cnt++;
  }
}

void PrintSectionList(void)
{
  if (!FirstSection)
    return;

  NewPage(ChapDepth, True);
  WrLstLine(getmessage(Num_ListSectionListHead1));
  WrLstLine(getmessage(Num_ListSectionListHead2));
  WrLstLine("");
  PrintSectionList_PSection(-1, 0);
}

void PrintDebSections(FILE *f)
{
  PCToken Lauf;
  LongInt Cnt, z, l, s;
  char Str[30];

  Lauf = FirstSection; Cnt = 0;
  while (Lauf)
  {
    fputs("\nInfo for Section ", f); ChkIO(ErrNum_FileWriteError);
    fprintf(f, "%" PRId32, Cnt); ChkIO(ErrNum_FileWriteError);
    fputc(' ', f); ChkIO(ErrNum_FileWriteError);
    fputs(GetSectionName(Cnt), f); ChkIO(ErrNum_FileWriteError);
    fputc(' ', f); ChkIO(ErrNum_FileWriteError);
    fprintf(f, "%" PRId32, Lauf->Parent); ChkIO(ErrNum_FileWriteError);
    fputc('\n', f); ChkIO(ErrNum_FileWriteError);
    for (z = 0; z < Lauf->Usage.RealLen; z++)
    {
      l = Lauf->Usage.Chunks[z].Length;
      s = Lauf->Usage.Chunks[z].Start;
      HexString(Str, sizeof(Str), s, 0);
      fputs(Str, f);
      ChkIO(ErrNum_FileWriteError);
      if (l == 1)
        fprintf(f, "\n");
      else
      {
        HexString(Str, sizeof(Str), s + l - 1, 0);
        fprintf(f, "-%s\n", Str);
      }
      ChkIO(ErrNum_FileWriteError);
    }
    Lauf = Lauf->Next;
    Cnt++;
  }
}

void ClearSectionList(void)
{
  PCToken Tmp;

  while (FirstSection)
  {
    Tmp = FirstSection;
    free(Tmp->Name);
    ClearChunk(&(Tmp->Usage));
    FirstSection = Tmp->Next; free(Tmp);
  }
}

/*---------------------------------------------------------------------------------*/

/*!------------------------------------------------------------------------
 * \fn     PrintCrossList_PNode(PTree Node, void *pData)
 * \brief  printf cross refence list of a single symbol table entry
 * \param  Node node base object
 * \param  pData actual symbol entry
 * \return
 * ------------------------------------------------------------------------ */

static void PrintCrossList_PNode(PTree Node, void *pData)
{
  int FileZ;
  PCrossRef pCross;
  String LineAcc;
  String h;
  char LineStr[30];
  PSymbolEntry SymbolEntry = (PSymbolEntry) Node;
  as_dynstr_t *p_val_str = (as_dynstr_t*)pData;
  Boolean First;

  if (!SymbolEntry->RefList)
    return;

  StrSym(&SymbolEntry->SymWert, False, p_val_str, ListRadixBase);
  as_snprintf(LineStr, sizeof(LineStr), "%" PRId32, SymbolEntry->LineNum);

  as_snprintf(h, sizeof(h), "%s%s",
              getmessage(Num_ListCrossSymName), Node->Name);
  if (Node->Attribute != -1)
    as_snprcatf(h, sizeof(h), "[%s]", GetSectionName(Node->Attribute));
  as_snprcatf(h, sizeof(h), " (=%s, %s:%s):",
              p_val_str->p_str, GetFileName(SymbolEntry->FileNum), LineStr);

  WrLstLine(h);

  for (FileZ = 0; FileZ < GetFileCount(); FileZ++)
  {
    First = True;
    strcpy(LineAcc, "  ");
    for (pCross = SymbolEntry->RefList; pCross; pCross = pCross->Next)
      if (pCross->FileNum == FileZ)
      {
        if (First)
        {
          strcpy(h, " ");
          strmaxcat(h, getmessage(Num_ListCrossFileName), STRINGSIZE);
          strmaxcat(h, GetFileName(FileZ), STRINGSIZE);
          strmaxcat(h, " :", STRINGSIZE);
          WrLstLine(h);
          First = False;
        }
        as_snprcatf(LineAcc, sizeof(LineAcc), "%5ld", (long)pCross->LineNum);
        if (pCross->OccNum != 1)
          as_snprcatf(LineAcc, sizeof(LineAcc), "(%2ld)", (long)pCross->OccNum);
        else
          strmaxcat(LineAcc, "    ", STRINGSIZE);
        if (strlen(LineAcc) >= 72)
        {
          WrLstLine(LineAcc);
          strcpy(LineAcc, "  ");
        }
      }
    if (strcmp(LineAcc, "  "))
      WrLstLine(LineAcc);
  }
  WrLstLine("");
}

void PrintCrossList(void)
{
  as_dynstr_t val_str;

  as_dynstr_ini(&val_str, 256);
  WrLstLine("");
  WrLstLine(getmessage(Num_ListCrossListHead1));
  WrLstLine(getmessage(Num_ListCrossListHead2));
  WrLstLine("");
  IterTree(&(FirstSymbol->Tree), PrintCrossList_PNode, &val_str);
  WrLstLine("");
  as_dynstr_free(&val_str);
}

static void ClearCrossList_CNode(PTree Tree, void *pData)
{
  PCrossRef Lauf;
  PSymbolEntry SymbolEntry = (PSymbolEntry) Tree;
  UNUSED(pData);

  while (SymbolEntry->RefList)
  {
    Lauf = SymbolEntry->RefList->Next;
    free(SymbolEntry->RefList);
    SymbolEntry->RefList = Lauf;
  }
}

void ClearCrossList(void)
{
  IterTree(&(FirstSymbol->Tree), ClearCrossList_CNode, NULL);
}

/*--------------------------------------------------------------------------*/

LongInt GetLocHandle(void)
{
  return LocHandleCnt++;
}

void PushLocHandle(LongInt NewLoc)
{
  PLocHandle NewLocHandle;

  NewLocHandle = (PLocHandle) malloc(sizeof(TLocHeap));
  NewLocHandle->Cont = MomLocHandle;
  NewLocHandle->Next = FirstLocHandle;
  FirstLocHandle = NewLocHandle; MomLocHandle = NewLoc;
}

void PopLocHandle(void)
{
  PLocHandle OldLocHandle;

  OldLocHandle = FirstLocHandle;
  if (!OldLocHandle) return;
  MomLocHandle = OldLocHandle->Cont;
  FirstLocHandle = OldLocHandle->Next;
  free(OldLocHandle);
}

void ClearLocStack()
{
  while (MomLocHandle != -1)
    PopLocHandle();
}

/*--------------------------------------------------------------------------*/

static void PrintRegList_PNode(PTree Tree, void *pData)
{
  PSymbolEntry Node = (PSymbolEntry) Tree;

  if (Node->SymWert.Typ == TempReg)
  {
    TListContext *pContext = (TListContext*) pData;
    String tmp, tmp2;

    if (Node->SymWert.Contents.RegDescr.Dissect)
      Node->SymWert.Contents.RegDescr.Dissect(tmp2, sizeof(tmp2), Node->SymWert.Contents.RegDescr.Reg, Node->SymWert.DataSize);
    else
      *tmp2 = '\0';
    *tmp = '\0';
    if (Tree->Attribute != -1)
      as_snprcatf(tmp, sizeof(tmp), "[%s]", GetSectionName(Tree->Attribute));
    as_snprcatf(tmp, sizeof(tmp), "%c%s --> %s", Node->Used ? ' ' : '*', Tree->Name, tmp2);
    if ((int)strlen(tmp) > pContext->cwidth - 3)
    {
      if (*pContext->Zeilenrest.p_str)
        WrLstLine(pContext->Zeilenrest.p_str);
      *pContext->Zeilenrest.p_str = '\0';
      WrLstLine(tmp);
    }
    else
    {
      strmaxcat(tmp, Blanks(pContext->cwidth - 3 - strlen(tmp)), STRINGSIZE);
      if (!*pContext->Zeilenrest.p_str)
        as_dynstr_copy_c_str(&pContext->Zeilenrest, tmp);
      else
      {
        as_sdprcatf(&pContext->Zeilenrest, " | %s", tmp);
        WrLstLine(pContext->Zeilenrest.p_str);
        *pContext->Zeilenrest.p_str = '\0';
      }
    }
    pContext->Sum++;
    if (!Node->Used)
      pContext->USum++;
  }
}

void PrintRegDefs(void)
{
  String buf;
  LongInt ActPageWidth;
  TListContext Context;

  if (!RegistersDefined)
    return;

  NewPage(ChapDepth, True);
  WrLstLine(getmessage(Num_ListRegDefListHead1));
  WrLstLine(getmessage(Num_ListRegDefListHead2));
  WrLstLine("");

  as_dynstr_ini(&Context.Zeilenrest, STRINGSIZE);
  as_dynstr_ini(&Context.s1, 1);
  as_dynstr_ini(&Context.sh, 1);
  Context.Sum = Context.USum = 0;
  ActPageWidth = (PageWidth == 0) ? 80 : PageWidth;
  Context.cwidth = ActPageWidth >> 1;
  IterTree((PTree)FirstSymbol, PrintRegList_PNode, &Context);

  if (*Context.Zeilenrest.p_str)
    WrLstLine(Context.Zeilenrest.p_str);
  WrLstLine("");
  as_snprintf(buf, sizeof(buf), "%7ld%s",
              (long) Context.Sum,
              getmessage((Context.Sum == 1) ? Num_ListRegDefSumMsg : Num_ListRegDefSumsMsg));
  WrLstLine(buf);
  as_snprintf(buf, sizeof(buf), "%7ld%s",
              (long)Context.USum,
              getmessage((Context.USum == 1) ? Num_ListRegDefUSumMsg : Num_ListRegDefUSumsMsg));
  WrLstLine("");
  as_dynstr_free(&Context.Zeilenrest);
  as_dynstr_free(&Context.s1);
  as_dynstr_free(&Context.sh);
}

/*--------------------------------------------------------------------------*/

void ClearCodepages(void)
{
  PTransTable Old;

  while (TransTables)
  {
    Old = TransTables;
    TransTables = Old->Next;
    free(Old->Name);
    free(Old->Table);
    free(Old);
  }
}

void PrintCodepages(void)
{
  char buf[500];
  PTransTable Table;
  int z, cnt, cnt2;

  NewPage(ChapDepth, True);
  WrLstLine(getmessage(Num_ListCodepageListHead1));
  WrLstLine(getmessage(Num_ListCodepageListHead2));
  WrLstLine("");

  cnt2 = 0;
  for (Table = TransTables; Table; Table = Table->Next)
  {
    for (z = cnt = 0; z < 256; z++)
      if (Table->Table[z] != z)
        cnt++;
    as_snprintf(buf, sizeof(buf), "%s (%d%s)", Table->Name, cnt,
                getmessage((cnt == 1) ? Num_ListCodepageChange : Num_ListCodepagePChange));
    WrLstLine(buf);
    cnt2++;
  }
  WrLstLine("");
  as_snprintf(buf, sizeof(buf), "%d%s", cnt2,
              getmessage((cnt2 == 1) ? Num_ListCodepageSumMsg : Num_ListCodepageSumsMsg));
  WrLstLine(buf);
}

/*--------------------------------------------------------------------------*/

void asmpars_init(void)
{
  tIntTypeDef *pCurr;

  serr = (char*)malloc(sizeof(char) * STRINGSIZE);
  snum = (char*)malloc(sizeof(char) * STRINGSIZE);
  FirstDefSymbol = NULL;
  FirstFunction = NULL;
  BalanceTrees = False;

  for (pCurr = IntTypeDefs; pCurr < IntTypeDefs + (sizeof(IntTypeDefs) / sizeof(*IntTypeDefs)); pCurr++)
  {
    unsigned SignType = Hi(pCurr->SignAndWidth);
    unsigned Bits, Cnt;

    Bits = Lo(pCurr->SignAndWidth) - ((SignType == 0x80) ? 1 : 0);
    for (Cnt = 0, pCurr->Mask = 0; Cnt < Bits; Cnt++)
      pCurr->Mask = (pCurr->Mask << 1) | 1;

    pCurr->Max = (LargeInt)pCurr->Mask;

    switch (SignType & 0xc0)
    {
      case 0x80:
        pCurr->Min = -pCurr->Max - 1;
        break;
      case 0xc0:
        pCurr->Min = (LargeInt)(pCurr->Mask / 2);
        pCurr->Min = -pCurr->Min - 1;
        break;
      default:
        pCurr->Min = 0;
        break;
    }
  }

  LastGlobSymbol = (char*)malloc(sizeof(char) * STRINGSIZE);
}
