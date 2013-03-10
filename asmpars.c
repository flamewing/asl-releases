/* asmpars.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Symbolen und das ganze Drumherum...                        */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*            4. 1.1997 Umstellung wg. case-sensitiv                         */
/*           24. 9.1997 Registersymbole                                      */
/*           26. 6.1998 Codepages                                            */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           17. 7.1998 Korrektur Maskentabellen                             */
/*           16. 8.1998 NoICE-Symbolausgabe                                  */
/*           18. 8.1998 Benutzung RadixBase                                  */
/*           19. 8.1998 == als Alias fuer = - Operator                       */
/*            1. 9.1998 RefList nicht initialisiert bei Symbolen             */
/*                      ACOT korrigiert                                      */
/*            6.12.1998 UInt14                                               */
/*           30. 1.1999 Formate maschinenunabhaengig gemacht                 */
/*           12. 2.1999 Compilerwarnungen beseitigt                          */
/*           17. 4.1999 Abfrage auf PCSymbol gegen Nullzeigerzugriff ge-     */
/*                      schuetzt.                                            */
/*           30. 5.1999 OutRadixBase beruecksichtigt                         */
/*           12. 7.1999 angefangen mit externen Symbolen                     */
/*           14. 7.1999 Relocs im Parser beruecksichtigt                     */
/*            1. 8.1999 Relocs im Formelparser durch                         */
/*            8. 8.1999 Relocs in EvalIntExpression beruecksichtigt          */
/*            8. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*           21. 5.2000 added TmpSymCounter                                  */
/*            1. 6.2000 dump symbols explicitly as hex for NoICE             */
/*           26. 6.2000 GetIntSymbol sets FirstPassUnknown                   */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           25. 5.2001 added UInt21                                         */
/*            3. 8.2001 added SInt6                                          */
/*           2001-10-04 better check for ASCII-like integer consts           */
/*           2001-10-20 added UInt23                                         */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmpars.c,v 1.20 2013-03-09 16:15:08 alfred Exp $                     */
/***************************************************************************** 
 * $Log: asmpars.c,v $
 * Revision 1.20  2013-03-09 16:15:08  alfred
 * - add NEC 75xx
 *
 * Revision 1.19  2010/04/17 13:14:19  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.18  2010/03/07 11:16:53  alfred
 * - allow DC.(float) on string operands
 *
 * Revision 1.17  2009/06/07 09:32:25  alfred
 * - add named temporary symbols
 *
 * Revision 1.16  2009/04/10 08:58:30  alfred
 * - correct address ranges for AVRs
 *
 * Revision 1.15  2008/11/23 10:39:15  alfred
 * - allow strings with NUL characters
 *
 * Revision 1.14  2008/10/21 16:33:04  alfred
 * - added charfromstr() function
 *
 * Revision 1.13  2007/11/24 22:48:02  alfred
 * - some NetBSD changes
 *
 * Revision 1.12  2007/09/24 17:39:02  alfred
 * - correct handling of '-' operator
 *
 * Revision 1.11  2007/04/30 18:37:51  alfred
 * - add weird integer coding
 *
 * Revision 1.10  2006/10/10 10:41:41  alfred
 * - free up space in data segment
 *
 * Revision 1.9  2005/12/13 19:28:37  alfred
 * - correct format strings for 16-bit platforms
 *
 * Revision 1.8  2005/10/30 13:24:28  alfred
 * - allow strings as int constants
 *
 * Revision 1.7  2005/10/02 10:00:43  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.6  2004/05/31 12:47:40  alfred
 * - clean up operator handling
 *
 * Revision 1.5  2004/05/30 20:51:42  alfred
 * - major cleanups in Const... functions
 *
 * Revision 1.4  2004/05/28 16:12:07  alfred
 * - added some const definitions
 *
 * Revision 1.3  2004/01/17 16:18:38  alfred
 * - fix some more GCC 3.3 quarrel
 *
 * Revision 1.2  2004/01/17 16:12:49  alfred
 * - some quirks for GCC 3.3
 *
 * Revision 1.1  2003/11/06 02:49:18  alfred
 * - recreated
 *
 * Revision 1.17  2003/10/04 15:38:46  alfred
 * - differentiate constant/variable messages
 *
 * Revision 1.16  2003/05/20 17:45:02  alfred
 * - StrSym with length spec
 *
 * Revision 1.15  2003/05/02 21:23:08  alfred
 * - strlen() updates
 *
 * Revision 1.14  2003/02/26 19:18:25  alfred
 * - add/use EvalIntDisplacement()
 *
 * Revision 1.13  2003/02/02 12:15:21  alfred
 * - added exptype() function
 *
 * Revision 1.12  2002/11/10 15:08:34  alfred
 * - use tree functions
 *
 * Revision 1.11  2002/11/10 09:43:07  alfred
 * - relocated symbol node type
 *
 * Revision 1.10  2002/11/04 19:04:26  alfred
 * - prevent modification of constants with SET
 *
 * Revision 1.9  2002/10/10 17:11:33  alfred
 * - repaired $$ temp symbols
 *
 * Revision 1.8  2002/10/07 20:25:01  alfred
 * - added '/' nameless temporary symbols
 *
 * Revision 1.7  2002/09/29 17:05:40  alfred
 * - ass +/- temporary symbols
 *
 * Revision 1.6  2002/05/25 21:15:20  alfred
 * - Fix array definition
 *
 * Revision 1.5  2002/05/19 13:44:52  alfred
 * - added ClearSectionUsage()
 *
 * Revision 1.4  2002/05/18 16:10:14  alfred
 * - optimize search via Find(Loc)Node
 *
 * Revision 1.3  2002/05/13 18:14:09  alfred
 * - use error msg 2010
 *
 * Revision 1.2  2002/03/10 11:55:42  alfred
 * - state which operand type was expected/got
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "bpemu.h"
#include "nls.h"
#include "nlmessages.h"
#include "as.rsc"
#include "strutil.h"

#include "asmdef.h"
#include "asmsub.h"
#include "asmfnums.h"
#include "asmrelocs.h"
#include "chunks.h"
#include "trees.h"

#include "asmpars.h"

#define LOCSYMSIGHT 3       /* max. sight for nameless temporary symbols */

LargeWord IntMasks[IntTypeCnt]=
            {0x00000001l,                         /* UInt1  */
             0x00000003l,                         /* UInt2  */
             0x00000007l,                         /* UInt3  */
             0x00000007l,0x0000000fl,0x0000000fl, /* SInt4  UInt4  Int4  */
             0x0000000fl,0x0000001fl,0x0000001fl, /* SInt5  UInt5  Int5  */
             0x0000001fl,0x0000003fl,             /* SInt6  UInt6  */
             0x0000003fl,0x0000007fl,             /* SInt7  UInt7  */
             0x0000007fl,0x000000ffl,0x000000ffl, /* SInt8  UInt8  Int8  */
             0x000001ffl,                         /* UInt9  */
             0x000003ffl,0x000003ffl,             /* UInt10 Int10 */
             0x000007ffl,                         /* UInt11 */
             0x00000fffl,0x00000fffl,             /* UInt12 Int12 */
             0x00001fffl,                         /* UInt13 */
             0x00003fffl,                         /* UInt14 */
             0x00007fffl,                         /* UInt15 */
             0x00007fffl,0x0000ffffl,0x0000ffffl, /* SInt16 UInt16 Int16 */
             0x0001ffffl,                         /* UInt17 */
             0x0003ffffl,                         /* UInt18 */
             0x0007ffffl,0x000fffffl,0x000fffffl, /* SInt20 UInt20 Int20 */
             0x001fffffl,                         /* UInt21 */
             0x003fffffl,                         /* UInt22 */
             0x007fffffl,                         /* UInt23 */
             0x007fffffl,0x00ffffffl,0x00ffffffl, /* SInt24 UInt24 Int24 */
             0xffffffffl,0xffffffffl,0xffffffffl  /* SInt32 UInt32 Int32 */
#ifdef HAS64
             ,0xffffffffffffffffllu               /* Int64 */
#endif
            };

LargeInt IntMins[IntTypeCnt]=
            {          0l,                            /* UInt1  */
                       0l,                            /* UInt2  */
                       0l,                            /* UInt3  */
                      -8l,          0l,         -8l,  /* SInt4  UInt4  Int4  */
                     -16l,          0l,        -16l,  /* SInt5  UInt5  Int5  */
                     -32l,          0l,               /* SInt6  UInt6  */
                     -64l,          0l,               /* SInt7  UInt7  */
                    -128l,          0l,       -128l,  /* SInt8  UInt8  Int8  */
                       0l,                            /* UInt9  */
                       0l,       -512l,               /* UInt10 Int10 */
                       0l,                            /* UInt11 */
                       0l,      -2047l,               /* UInt12 Int12 */
                       0l,                            /* UInt13 */
                       0l,                            /* UInt14 */
                       0l,                            /* UInt15 */
                  -32768l,          0l,     -32768l,  /* SInt16 UInt16 Int16 */
                       0,                             /* UInt17 */
                       0l,                            /* UInt18 */
                 -524288l,          0l,    -524288l,  /* SInt20 UInt20 Int20 */
                       0l,                            /* UInt21 */
                       0l,                            /* UInt22 */
                       0l,                            /* UInt23 */
                -8388608l,          0l,   -8388608l,  /* SInt24 UInt24 Int24 */
             -2147483647l,          0l,-2147483647l   /* SInt32 UInt32 Int32 */
#ifdef HAS64
             ,-9223372036854775807ll                  /* Int64 */
#endif
            };

LargeInt IntMaxs[IntTypeCnt]=
            {          1l,                            /* UInt1  */
                       3l,                            /* UInt2  */
                       7l,                            /* UInt3  */
                       7l,         15l,         15l,  /* SInt4  UInt4  Int4  */
                      15l,         31l,         31l,  /* SInt5  UInt5  Int5  */
                      31l,         63l,               /* SInt6  UInt6  */
                      63l,        127l,               /* SInt7  UInt7  */
                     127l,        255l,        255l,  /* SInt8  UInt8  Int8  */
                     511l,                            /* UInt9  */
                    1023l,       1023l,               /* UInt10 Int10 */
                    2047l,                            /* UInt11 */
                    4095l,       4095l,               /* UInt12 Int12 */
                    8191l,                            /* UInt13 */
                   16383l,                            /* UInt14 */
                   32767l,                            /* UInt15 */
                   32767l,      65535l,      65535l,  /* SInt16 UInt16 Int16 */
                  131071l,                            /* UInt17 */
                  262143l,                            /* UInt18 */
                  524287l,                            /* SInt20 */
                 1048575l,    1048575l,               /* UInt20 Int20 */
                 2097151l,                            /* UInt21 */
                 4194303l,                            /* UInt22 */
                 8388608l,                            /* UInt23 */
#ifdef __STDC__
                 8388607l,   16777215l,   16777215l,  /* SInt24 UInt24 Int24 */
              2147483647l, 4294967295ul,4294967295ul  /* SInt32 UInt32 Int32 */
#else
                 8388607l,   16777215l,   16777215l,  /* SInt24 UInt24 Int24 */
              2147483647l, 4294967295l, 4294967295l   /* SInt32 UInt32 Int32 */
#endif
#ifdef HAS64
             , 9223372036854775807ll                  /* Int64 */
#endif
            };

typedef struct
        {
          Boolean Back;
          LongInt Counter;
        } TTmpSymLog;

Boolean FirstPassUnknown;      /* Hinweisflag: evtl. im ersten Pass unbe-
                                  kanntes Symbol, Ausdruck nicht ausgewertet */
Boolean SymbolQuestionable;    /* Hinweisflag:  Dadurch, dass Phasenfehler
                                  aufgetreten sind, ist dieser Symbolwert evtl.
                                  nicht mehr aktuell                         */
Boolean UsesForwards;          /* Hinweisflag: benutzt Vorwaertsdefinitionen */
LongInt MomLocHandle;          /* Merker, den lokale Symbole erhalten        */
LongInt TmpSymCounter,         /* counters for local symbols                 */
        FwdSymCounter,
        BackSymCounter;
char TmpSymCounterVal[10];     /* representation as string                   */
TTmpSymLog TmpSymLog[LOCSYMSIGHT];
LongInt TmpSymLogDepth;

LongInt LocHandleCnt;          /* mom. verwendeter lokaler Handle            */

Boolean BalanceTree;           /* Symbolbaum ausbalancieren                  */

static char BaseIds[3]={'%','@','$'};
static char BaseLetters[4]={'B','O','H','Q'};
static Byte BaseVals[4]={2,8,16,8};

typedef struct _TSymbolEntry
         {
          TTree Tree;
          Byte SymType;
          ShortInt SymSize;
          Boolean Defined,Used,Changeable;
          SymbolVal SymWert;
          PCrossRef RefList;
          Byte FileNum;
          LongInt LineNum;
          TRelocEntry *Relocs;
         } TSymbolEntry,*PSymbolEntry;

typedef struct _TSymbolStackEntry
         {
          struct _TSymbolStackEntry *Next;
          SymbolVal Contents;
         } TSymbolStackEntry,*PSymbolStackEntry;

typedef struct _TSymbolStack
         {
          struct _TSymbolStack *Next;
          char *Name;
          PSymbolStackEntry Contents;
         } TSymbolStack,*PSymbolStack;

typedef struct _TDefSymbol
         {
          struct _TDefSymbol *Next;
          char *SymName;
          TempResult Wert;
         } TDefSymbol,*PDefSymbol;

typedef struct _TCToken
         {
          struct _TCToken *Next;
          char *Name;
          LongInt Parent;
          ChunkList Usage;
         } TCToken,*PCToken;

typedef struct Operator
         {
          char *Id;
          int IdLen;
          Boolean Dyadic;
          Byte Priority;
          Boolean MayInt;
          Boolean MayFloat;
          Boolean MayString;
          Boolean Present;
          void (*pFunc)(TempResult *pErg, TempResult *pLVal, TempResult *pRVal);
         } Operator;

typedef struct _TLocHeap
         {
          struct _TLocHeap *Next;
          LongInt Cont;
         } TLocHeap,*PLocHandle;

typedef struct _TRegDefList
         {
          struct _TRegDefList *Next;
          LongInt Section;
          char *Value;
          Boolean Used;
         } TRegDefList,*PRegDefList;

typedef struct _TRegDef
         {
          struct _TRegDef *Left,*Right;
          char *Orig;
          PRegDefList Defs,DoneDefs;
         } TRegDef,*PRegDef;

static PSymbolEntry FirstSymbol,FirstLocSymbol;
static PDefSymbol FirstDefSymbol;
/*static*/ PCToken FirstSection;
static PRegDef FirstRegDef;
static Boolean DoRefs;              /* Querverweise protokollieren */
static PLocHandle FirstLocHandle;
static PSymbolStack FirstStack;
static PCToken MomSection;
static char *LastGlobSymbol;

        void AsmParsInit(void)
BEGIN
   FirstSymbol=Nil;

   FirstLocSymbol = Nil; MomLocHandle = (-1); SetMomSection(-1);
   FirstSection = Nil;
   FirstLocHandle = Nil;
   FirstStack = Nil;
   FirstRegDef = Nil;
   DoRefs = True;
   RadixBase = 10;
   OutRadixBase = 16;
END


        Boolean RangeCheck(LargeInt Wert, IntType Typ)
BEGIN
#ifndef HAS64
   if (((int)Typ)>=((int)SInt32)) return True;
#else
   if (((int)Typ)>=((int)Int64)) return True;
#endif
   else return ((Wert>=IntMins[(int)Typ]) AND (Wert<=IntMaxs[(int)Typ]));
END

        Boolean FloatRangeCheck(Double Wert, FloatType Typ)
BEGIN
   switch (Typ)
    BEGIN
     case Float32  : return (fabs(Wert)<=3.4e38);
     case Float64  : return (fabs(Wert)<=1.7e308);
/**     case FloatCo  : FloatRangeCheck:=Abs(Wert)<=9.22e18; */
     case Float80  : return True;
     case FloatDec : return True;
     default: return False;
    END
/**   IF (Typ=FloatDec) AND (Abs(Wert)>1e1000) THEN WrError(40);**/
END


	Boolean SingleBit(LargeInt Inp, LargeInt *Erg)
BEGIN
   *Erg=0;
   do
    BEGIN
     if (NOT Odd(Inp)) (*Erg)++;
     if (NOT Odd(Inp)) Inp=Inp>>1;
    END
   while ((*Erg!=LARGEBITS) AND (NOT Odd(Inp)));
   return (*Erg!=LARGEBITS) AND (Inp==1);
END	
	

	static Boolean ProcessBk(char **Start, char *Erg)
BEGIN
   LongInt System=0,Acc=0,Digit=0;
   char ch;
   int cnt;
   Boolean Finish;

   switch (mytoupper(**Start))
    BEGIN
     case '\'': case '\\': case '"':
      *Erg=**Start; (*Start)++; return True;
     case 'H':
      *Erg='\''; (*Start)++; return True;
     case 'I':
      *Erg='"'; (*Start)++; return True;
     case 'B':
      *Erg=Char_BS; (*Start)++; return True;
     case 'A':
      *Erg=Char_BEL; (*Start)++; return True;
     case 'E':
      *Erg=Char_ESC; (*Start)++; return True;
     case 'T':
      *Erg=Char_HT; (*Start)++; return True;
     case 'N':
      *Erg=Char_LF; (*Start)++; return True;
     case 'R':
      *Erg=Char_CR; (*Start)++; return True;
     case 'X':
      System=16; (*Start)++;
     case '0': case '1': case '2': case '3': case '4':
     case '5': case '6': case '7': case '8': case '9':
      if (System==0) System=(**Start=='0')?8:10;
      cnt=(System==16) ? 1 : ((System==10) ? 0 : -1);
      do
       BEGIN
        ch=mytoupper(**Start); Finish=False;
        if ((ch>='0') AND (ch<='9')) Digit=ch-'0';
        else if ((System==16) AND (ch>='A') AND (ch<='F')) Digit=(ch-'A')+10;
        else Finish=True;
        if (NOT Finish)
         BEGIN
          (*Start)++; cnt++;
          if (Digit>=System)
           BEGIN
            WrError(1320); return False;
           END
          Acc=(Acc*System)+Digit;
         END
       END
      while ((NOT Finish) AND (cnt<3));
      if (NOT ChkRange(Acc,0,255)) return False;
      *Erg=Acc; return True;
     default:
      WrError(2010); return False;
    END
END

        static void ReplaceBkSlashes(char *s)
BEGIN
   char *p,*n;
   char ErgChar;

   p=strchr(s,'\\');
   while (p!=Nil)
    BEGIN
     n=p+1; if (ProcessBk(&n,&ErgChar)) *p=ErgChar;
     strcpy(p+1,n);
     p=strchr(p+1,'\\');
    END
END


        Boolean ExpandSymbol(char *Name)
BEGIN
   char *p1,*p2;
   String h;
   Boolean OK;

   do
    BEGIN
     if ((p1=strchr(Name,'{'))==Nil) return True;
     strmaxcpy(h,p1+1,255);
     if ((p2=QuotPos(h,'}'))==Nil)
      BEGIN
       WrXError(1020,Name);
       return False;
      END
     strcpy(p1,p2+1); *p2='\0';
     FirstPassUnknown=False;
     EvalStringExpression(h,&OK,h);
     if (FirstPassUnknown)
      BEGIN
       WrError(1820); return False;
      END
     if (NOT CaseSensitive) UpString(h);
     strmaxins(Name,h,p1-Name,255);
    END
   while (p1!=Nil);
   return True;
END

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
/* check whether this is a local symbol and expand local counter if yes.  They
   have to be handled in different places of the parser, therefore two separate 
   functions */

	void InitTmpSymbols(void)
{
  TmpSymCounter = FwdSymCounter = BackSymCounter = 0;
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

static Boolean ChkTmp1(char *Name, Boolean Define)
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
     sprintf(TmpSymCounterVal, "%d", TmpSymCounter);
    strcpy(Dest, TmpSymCounterVal);
    Result = TRUE;
  }

  /* no special local symbol: increment $$-counter */

  else if (Define)
  {
    TmpSymCounter++;
    *TmpSymCounterVal = '\0';
  }

  return Result;
}

static Boolean ChkTmp2(char *Name, Boolean Define)
{
  char *Src;
  int Cnt;
  Boolean Result = FALSE;

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

  if (*Name == '-')
  {
    for (Src = Name; *Src; Src++)
      if (*Src != '-')
        break;
    Cnt = Src - Name;
    if (!*Src)
    {
      if ((Define) && (Cnt == 1))
      {
        sprintf(Name, "__back%d", BackSymCounter);
        AddTmpSymLog(TRUE, BackSymCounter);
        BackSymCounter++;
        Result = TRUE;
      }

      /* TmpSymLogDepth cannot become larger than LOCSYMSIGHT, so we only
         have to check against the log's actual depth. */

      else if (Cnt <= TmpSymLogDepth)
      {
        Cnt--;
        sprintf(Name, "__%s%d", 
                TmpSymLog[Cnt].Back ? "back" : "forw",
                TmpSymLog[Cnt].Counter);
        Result = TRUE;
      }
    }
  }

  /* forward references ? */

  else if (*Name == '+')
  {
    for (Src = Name; *Src; Src++)
      if (*Src != '+')
        break;
    Cnt = Src - Name;
    if (!*Src)
    {
      if ((Define) && (Cnt == 1))
      {
        sprintf(Name, "__forw%d", FwdSymCounter++);
        Result = TRUE;
      }
      else if (Cnt <= LOCSYMSIGHT)
      {
        sprintf(Name, "__forw%d", FwdSymCounter + (Cnt - 1));
        Result = TRUE;
      }
    }
  }

  /* slash: only allowed for definition, but add to log for backward ref. */

  else if ((!strcmp(Name, "/")) && (Define))
  {
    AddTmpSymLog(FALSE, FwdSymCounter);
    sprintf(Name, "__forw%d", FwdSymCounter);
    FwdSymCounter++;
    Result = TRUE;
  }

  return Result;
}

static Boolean ChkTmp3(char *Name, Boolean Define)
{
  Boolean Result = FALSE;

  if ('.' == *Name)
  {
    String Tmp;

    strmaxcpy(Tmp, LastGlobSymbol, 255);
    strmaxcat(Tmp, Name, 255);
    strmaxcpy(Name, Tmp, 255);

    Result = TRUE;
  }
  else if (Define)
  {
    strmaxcpy(LastGlobSymbol, Name, 255);
  }

  return Result;
}

static Boolean ChkTmp(char *Name, Boolean Define)
{
  Boolean Result = FALSE;

  if (ChkTmp1(Name, Define))
    Result = TRUE;
  if (ChkTmp2(Name, Define))
    Result = TRUE;
  if (ChkTmp3(Name, Define))
    Result = TRUE;
  return Result;
}

        Boolean IdentifySection(char *Name, LongInt *Erg)
BEGIN
   PSaveSection SLauf;
   sint Depth;

   if (NOT ExpandSymbol(Name)) return False;
   if (NOT CaseSensitive) NLS_UpString(Name);

   if (*Name=='\0')
    BEGIN
     *Erg=(-1); return True;
    END
   else if (((strlen(Name)==6) OR (strlen(Name)==7))
        AND (strncasecmp(Name,"PARENT",6)==0)
        AND ((strlen(Name)==6) OR ((Name[6]>='0') AND (Name[6]<='9'))))
    BEGIN
     if (strlen(Name)==6) Depth=1; else Depth=Name[6]-AscOfs;
     SLauf=SectionStack; *Erg=MomSectionHandle;
     while ((Depth>0) AND (*Erg!=(-2)))
      BEGIN
       if (SLauf==Nil) *Erg=(-2);
       else
        BEGIN
         *Erg=SLauf->Handle;
         SLauf=SLauf->Next;
        END
       Depth--;
      END
     if (*Erg==(-2))
      BEGIN
       WrError(1484); return False;
      END
     else return True;
    END
   else if (strcmp(Name,GetSectionName(MomSectionHandle))==0)
    BEGIN
     *Erg=MomSectionHandle; return True;
    END
   else
    BEGIN
     SLauf=SectionStack;
     while ((SLauf!=Nil) AND (strcmp(GetSectionName(SLauf->Handle),Name)!=0))
      SLauf=SLauf->Next;
     if (SLauf==Nil)
      BEGIN
       WrError(1484); return False;
      END
     else
      BEGIN
       *Erg=SLauf->Handle; return True;
      END
    END
END

        static Boolean GetSymSection(char *Name, LongInt *Erg)
{
   String Part;
   char *q;
   int l = strlen(Name);

   if (Name[l - 1] != ']')
   {
     *Erg = (-2); return True;
   }

   Name[l - 1] = '\0';
   q = RQuotPos(Name,'[');
   Name[l - 1] = ']';
   if (Name + l - q <= 1)
   {
     WrXError(1020, Name); return False; 
   }

   Name[l - 1] = '\0';
   strmaxcpy(Part, q + 1, 255); 
   *q = '\0';

   return IdentifySection(Part, Erg);
}

/*****************************************************************************
 * Function:    DigitVal
 * Purpose:     return value of an integer digit
 * Result:      value or -1 for error
 *****************************************************************************/

static int DigitVal(char ch, int Base)
{
   int erg;

   if ((ch >= '0') && (ch <= '9'))
     erg = ch - '0';
   else if ((ch >= 'A') && (ch <= 'Z'))
     erg = ch - 'A' + 10;
   else
     return - 1;

   if (erg >= Base)
     return -1;

   return erg;
}

/*****************************************************************************
 * Function:    ConstIntVal
 * Purpose:     evaluate integer constant
 * Result:      integer value
 *****************************************************************************/

LargeInt ConstIntVal(const char *pExpr, IntType Typ, Boolean *pResult)
{
  Byte Digit;
  LargeInt Wert;
  int l;

  /* empty string is interpreted as 0 */

  if (!*pExpr)
  {
    *pResult = True;
    return 0;
  }

  *pResult = False; Wert = 0;

  /* ASCII herausfiltern */

  if (*pExpr == '\'')
  {
    const char *pRun;
    String Copy;

    /* consistency check: closing ' must be present precisely at end; skip escaped characters */

    for (pRun = pExpr + 1; (*pRun) && (*pRun != '\''); pRun++)
    {
      if (*pRun == '\\')
        pRun++;
    }
    if ((*pRun != '\'') || (pRun[1]))
      return -1;

    strmaxcpy(Copy, pExpr + 1, STRINGSIZE);
    l = strlen(Copy);
    Copy[l - 1] = '\0'; ReplaceBkSlashes(Copy);

    for (pRun = Copy; *pRun; pRun++)
    {
      Digit = (usint) *pRun;
      Wert = (Wert << 8) + CharTransTable[Digit & 0xff];
    }
  }

  /* Zahlenkonstante */

  else
  {
    Boolean NegFlag = False;
    TConstMode ActMode = ConstModeC;
    int Search;
    Byte Base;
    char ch;
    Boolean Found;

    /* sign: */

    switch (*pExpr)
    {
      case '-':
        NegFlag = True;
        /* explicitly no break */
      case '+':
        pExpr++;
        break;
    }
    l = strlen(pExpr);

    /* automatic syntax determination: */

    if (RelaxedMode)
    {
      Found = False;

      if ((l >= 2) && (*pExpr == '0') && (mytoupper(pExpr[1]) == 'X'))
      {
        ActMode = ConstModeC;
        Found = True;
      }

      if ((!Found) && (l >= 2))
      {
        for (Search = 0; Search < 3; Search++)
          if (*pExpr == BaseIds[Search])
          {
            ActMode = ConstModeMoto;
            Found = True;
            break;
          }
      }

      if ((!Found) && (l >= 2) && (*pExpr >= '0') && (*pExpr <= '9'))
      {
        ch = mytoupper(pExpr[l - 1]);
        if (DigitVal(ch, RadixBase) == -1)
        {
          for (Search = 0; Search < sizeof(BaseLetters) / sizeof(*BaseLetters); Search++)
            if (ch==BaseLetters[Search])
            {
              ActMode = ConstModeIntel;
              Found = True;
              break;
            }
        }
      }

      if ((!Found) && (l >= 3) && (pExpr[1] == '\'') && (pExpr[l - 1] == '\''))
      {
        switch (mytoupper(*pExpr))
        {
          case 'H':
          case 'X':
          case 'B':
          case 'O':
            ActMode = ConstModeWeird;
            Found = True;
            break;
        }
      }

      if (!Found)
        ActMode = ConstModeC;
    }
    else /* !RelaxedMode */
      ActMode = ConstMode;

    /* Zahlensystem ermitteln/pruefen */

    Base = RadixBase;
    switch (ActMode)
    {
      case ConstModeIntel:
        ch = mytoupper(pExpr[l - 1]);
        if (DigitVal(ch, RadixBase) == -1)
        {
          for (Search = 0; Search < sizeof(BaseLetters) / sizeof(*BaseLetters); Search++) 
            if (ch == BaseLetters[Search])
            {
              Base = BaseVals[Search];
              l--;
              break;
            }
        }
        break;
      case ConstModeMoto:
        for (Search = 0; Search < 3; Search++)
          if (*pExpr == BaseIds[Search])
          {
            Base = BaseVals[Search];
            pExpr++; l--;
            break;
          }
        break;
      case ConstModeC:
        if (!strcmp(pExpr, "0"))
        {
          *pResult = True;
          return 0;
        }
        if (*pExpr != '0') Base = RadixBase;
        else if (l < 2) return -1;
        else
        {
          pExpr++; l--;
          ch = mytoupper(*pExpr);
          if ((RadixBase != 10) && (DigitVal(ch, RadixBase) != -1))
            Base = RadixBase;
          else
            switch (mytoupper(*pExpr))
            {
              case 'X': pExpr++;  l--; Base = 16; break;
              case 'B': pExpr++;  l--; Base = 2; break;
              default: Base = 8;
            }
        }
        break;
      case ConstModeWeird:
        if ((l < 3) || (pExpr[1] != '\'') || (pExpr[l - 1] != '\''))
          return -1;
        switch (mytoupper(*pExpr))
        {
          case 'X':
          case 'H':
            Base = 16; break;
          case 'B':
            Base = 2; break;
          case 'O':
            Base = 8; break;
          default:
            return -1;
        }
        pExpr += 2; l -= 3;
        break;
    }

    if (!*pExpr)
      return -1;

    if (ActMode == ConstModeIntel)
    {
      if ((*pExpr < '0') || (*pExpr > '9'))
        return -1;
    }

    /* we may have decremented l, so do not run until string end */

    while (l > 0)
    {
      Search = DigitVal(mytoupper(*pExpr), Base);
      if (Search == -1)
        return -1;
      Wert = Wert * Base + Search;
      pExpr++; l--;
    }

    if (NegFlag)
      Wert = (-Wert);
  }

  /* post-processing, range check */

  *pResult = RangeCheck(Wert, Typ);
  if (*pResult)
    return Wert;
  else if (HardRanges)
  {
    WrError(1320);
    return -1;
  }
  else
  {
    *pResult = True;
    WrError(260);
    return Wert&IntMasks[(int)Typ];
  }
}

/*****************************************************************************
 * Function:    ConstFloatVal
 * Purpose:     evaluate floating point constant
 * Result:      value
 *****************************************************************************/

Double ConstFloatVal(const char *pExpr, FloatType Typ, Boolean *pResult)
{
  Double Erg;
  char *pEnd;

  UNUSED(Typ);

  if (*pExpr)
  {
    Erg = strtod(pExpr, &pEnd);
    *pResult = (*pEnd == '\0');
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

void ConstStringVal(const char *pExpr, tDynString *pDest, Boolean *pResult)
{
  String Copy;
  char *pPos, *pCurr;
  int l, TLen;

  *pResult = False;

  l = strlen(pExpr);
  if ((l < 2) || (*pExpr != '"') || (pExpr[l - 1] != '"'))
    return;

  strmaxcpy(Copy, pExpr + 1, STRINGSIZE);
  Copy[strlen(Copy) - 1] = '\0';

  /* go through source */

  pCurr = Copy;
  pDest->Length = 0;
  while (*pCurr)
  {
    /* copy part up to next '\' verbatim: */

    pPos = strchr(pCurr, '\\');
    if (pPos)
      *pPos = '\0';
    if (strchr(pCurr, '"'))
      return;
    TLen = strlen(pCurr);
    DynStringAppend(pDest, pCurr, TLen);

    /* are we done? If not, advance pointer to behind '\' */

    if (!pPos)
      break;
    pCurr = pPos + 1;

    if (pPos)
    {
      /* stringification? */

      if (*pCurr == '{')
      {
        TempResult t;
        char *pStr;

        /* cut out part in {...} */

        pPos = QuotPos(++pCurr, '}');
        if (!pPos)
          return;
        *pPos = '\0';
        KillBlanks(pCurr);

        /* evaluate expression */

        FirstPassUnknown = False;
        EvalExpression(pCurr, &t);
        if (FirstPassUnknown) 
        {
          WrXError(1820, pCurr);
          return;
        }
        if (t.Relocs != Nil)
        {
          WrError(1150);
          FreeRelocs(&t.Relocs);
          return;
        }

        /* append result */

        switch(t.Typ)
        {
          case TempInt:
            pStr = SysString(t.Contents.Int, OutRadixBase, 0);
            TLen = strlen(pStr);
            break;
          case TempFloat:
            pStr = FloatString(t.Contents.Float);
            TLen = strlen(pStr);
            break;
          case TempString: 
            pStr = t.Contents.Ascii.Contents;
            TLen = t.Contents.Ascii.Length;
            break;
          default:
            return;
        }
        DynStringAppend(pDest, pStr, TLen);

        /* advance source pointer to behind '}' */

        pCurr = pPos + 1;
      }

      /* simple character escape: */

      else
      {
        char Res;

        if (!ProcessBk(&pCurr, &Res))
          return;
        DynStringAppend(pDest, &Res, 1);
      }
    }
  }

  *pResult = True;
}


        static PSymbolEntry FindLocNode(
#ifdef __PROTOS__
char *Name, TempType SearchType
#endif
);

        static PSymbolEntry FindNode(
#ifdef __PROTOS__
char *Name, TempType SearchType
#endif
);

/*****************************************************************************
 * Function:    EvalExpression
 * Purpose:     evaluate expression
 * Result:      implicitly in pErg
 *****************************************************************************/


static void EvalExpression_ChgFloat(TempResult *pTemp)
{
   if (pTemp->Typ != TempInt)
     return;
   pTemp->Typ = TempFloat;
   pTemp->Contents.Float = pTemp->Contents.Int;
}

static void EvalExpression_DummyOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  UNUSED(pLVal); UNUSED(pRVal); UNUSED(pErg);
}

static void EvalExpression_OneComplOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  UNUSED(pLVal);
  pErg->Typ = TempInt;
  pErg->Contents.Int = ~(pRVal->Contents.Int);
}

static void EvalExpression_ShLeftOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int << pRVal->Contents.Int;
}
 
static void EvalExpression_ShRightOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int >> pRVal->Contents.Int;
}
 
static void EvalExpression_BitMirrorOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  int z;

  if ((pRVal->Contents.Int < 1) || (pRVal->Contents.Int > 32)) WrError(1320);
  else
  {
    pErg->Typ = TempInt;
    pErg->Contents.Int = (pLVal->Contents.Int >> pRVal->Contents.Int) << pRVal->Contents.Int;
    pRVal->Contents.Int--;
    for (z = 0; z <= pRVal->Contents.Int; z++)
    {
      if ((pLVal->Contents.Int & (1 << (pRVal->Contents.Int - z))) != 0)
        pErg->Contents.Int |= (1 << z);
    }
  }
}

static void EvalExpression_BinAndOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int & pRVal->Contents.Int;
}
 
static void EvalExpression_BinOrOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int | pRVal->Contents.Int;
}
 
static void EvalExpression_BinXorOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  pErg->Contents.Int = pLVal->Contents.Int ^ pRVal->Contents.Int;
}
 
static void EvalExpression_PotOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  LargeInt HVal;

  switch (pErg->Typ = pLVal->Typ)
  {
    case TempInt:
      if (pRVal->Contents.Int < 0) pErg->Contents.Int = 0;
      else
      {
        pErg->Contents.Int = 1;
        while (pRVal->Contents.Int > 0)
        {
          if ((pRVal->Contents.Int&1) == 1) pErg->Contents.Int *= pLVal->Contents.Int;
          pRVal->Contents.Int >>= 1;
          if (pRVal->Contents.Int != 0) pLVal->Contents.Int *= pLVal->Contents.Int;
        }
      }
      break;
    case TempFloat:
      if (pRVal->Contents.Float == 0.0) pErg->Contents.Float = 1.0;
      else if (pLVal->Contents.Float == 0.0) pErg->Contents.Float = 0.0;
      else if (pLVal->Contents.Float > 0) pErg->Contents.Float = pow(pLVal->Contents.Float, pRVal->Contents.Float);
      else if ((abs(pRVal->Contents.Float) <= ((double)MaxLongInt)) AND (floor(pRVal->Contents.Float) == pRVal->Contents.Float))
      {
        HVal = (LongInt) floor(pRVal->Contents.Float+0.5);
        if (HVal < 0)
        {
          pLVal->Contents.Float = 1 / pLVal->Contents.Float; HVal = (-HVal);
        }
        pErg->Contents.Float = 1.0;
        while (HVal > 0)
        {
          if ((HVal & 1) == 1) pErg->Contents.Float *= pLVal->Contents.Float;
          pLVal->Contents.Float *= pLVal->Contents.Float; HVal >>= 1;
        }
      }
      else
      {
        WrError(1890); pErg->Typ = TempNone;
      }
      break;
    default:
     break;
  }
}
 
static void EvalExpression_MultOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  switch (pErg->Typ = pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = pLVal->Contents.Int * pRVal->Contents.Int;
      break;
    case TempFloat:
      pErg->Contents.Float = pLVal->Contents.Float * pRVal->Contents.Float;
      break;
    default:
      break;
  }
}  
   
static void EvalExpression_DivOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)  
{
  switch (pLVal->Typ)
  {
    case TempInt:
     if (pRVal->Contents.Int == 0) WrError(1310);
     else
     {
       pErg->Typ = TempInt;
       pErg->Contents.Int = pLVal->Contents.Int / pRVal->Contents.Int;
     }
     break;
    case TempFloat:
     if (pRVal->Contents.Float == 0.0) WrError(1310);
     else
     {
       pErg->Typ = TempFloat;
       pErg->Contents.Float = pLVal->Contents.Float / pRVal->Contents.Float;
     }
    default: 
     break;
  }
}

static void EvalExpression_ModOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  if (pRVal->Contents.Int == 0) WrError(1310);
  else
  {
    pErg->Typ = TempInt;
    pErg->Contents.Int = pLVal->Contents.Int % pRVal->Contents.Int;
  }
}

static void EvalExpression_AddOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  switch (pErg->Typ = pLVal->Typ)
  {
    case TempInt   : 
      pErg->Contents.Int = pLVal->Contents.Int + pRVal->Contents.Int;
      pErg->Relocs = MergeRelocs(&(pLVal->Relocs), &(pRVal->Relocs), TRUE);
      break;
    case TempFloat :
      pErg->Contents.Float = pLVal->Contents.Float + pRVal->Contents.Float;
      break;
    case TempString: 
      DynString2DynString(&pErg->Contents.Ascii, &pLVal->Contents.Ascii);
      DynStringAppendDynString(&pErg->Contents.Ascii, &pRVal->Contents.Ascii);
      break;
    default:
      break;
  }
}  

static void EvalExpression_SubOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  switch (pErg->Typ = pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = pLVal->Contents.Int - pRVal->Contents.Int;
      pErg->Relocs = MergeRelocs(&(pLVal->Relocs), &(pRVal->Relocs), FALSE);
      break;
    case TempFloat:
      pErg->Contents.Float = pLVal->Contents.Float - pRVal->Contents.Float;
      break;
    default:
      break;
  }
}  

static void EvalExpression_LogNotOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)    
{
  UNUSED(pLVal);
  pErg->Typ = TempInt;
  pErg->Contents.Int = (pRVal->Contents.Int == 0) ? 1 : 0;
}

static void EvalExpression_LogAndOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{                
  pErg->Typ = TempInt;
  pErg->Contents.Int = ((pLVal->Contents.Int != 0) && (pRVal->Contents.Int != 0)) ? 1 : 0;
}

static void EvalExpression_LogOrOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{                
  pErg->Typ = TempInt;
  pErg->Contents.Int = ((pLVal->Contents.Int != 0) || (pRVal->Contents.Int != 0)) ? 1 : 0;
}

static void EvalExpression_LogXorOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{                
  pErg->Typ = TempInt;
  pErg->Contents.Int = ((pLVal->Contents.Int != 0) != (pRVal->Contents.Int != 0)) ? 1 : 0;
}
 
static void EvalExpression_EqOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt: 
      pErg->Contents.Int = (pLVal->Contents.Int == pRVal->Contents.Int) ? 1 : 0;
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float == pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) == 0) ? 1 : 0;
      break;
    default:
      break;
  }
}
                                                                                         
static void EvalExpression_GtOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt: 
      pErg->Contents.Int = (pLVal->Contents.Int > pRVal->Contents.Int) ? 1 : 0;
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float > pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) > 0) ? 1 : 0;
      break;
    default:
      break;
  }
}                           
                            
static void EvalExpression_LtOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{                                                                                
  pErg->Typ = TempInt;                                                           
  switch (pLVal->Typ)                                                              
  {                                                                              
    case TempInt:                                                                
      pErg->Contents.Int = (pLVal->Contents.Int < pRVal->Contents.Int) ? 1 : 0;  
      break;                                                                     
    case TempFloat:                                                              
      pErg->Contents.Int = (pLVal->Contents.Float < pRVal->Contents.Float) ? 1 : 0;
      break;                                                                     
    case TempString:                                                             
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) < 0) ? 1 : 0;
      break;                                                                     
    default:                                                                     
      break;                                                                     
  }                                                                              
}                                                                                
                                                                                 
static void EvalExpression_LeOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int <= pRVal->Contents.Int) ? 1 : 0;  
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float <= pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) <= 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

static void EvalExpression_GeOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int >= pRVal->Contents.Int) ? 1 : 0;  
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float >= pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) >= 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

static void EvalExpression_UneqOp(TempResult *pErg, TempResult *pLVal, TempResult *pRVal)
{
  pErg->Typ = TempInt;
  switch (pLVal->Typ)
  {
    case TempInt:
      pErg->Contents.Int = (pLVal->Contents.Int != pRVal->Contents.Int) ? 1 : 0;  
      break;
    case TempFloat:
      pErg->Contents.Int = (pLVal->Contents.Float != pRVal->Contents.Float) ? 1 : 0;
      break;
    case TempString:
      pErg->Contents.Int = (DynStringCmp(&pLVal->Contents.Ascii, &pRVal->Contents.Ascii) != 0) ? 1 : 0;
      break;
    default:
      break;
  }
}

#define LEAVE goto func_exit

void EvalExpression(const char *pExpr, TempResult *pErg)
{
static Operator Operators[] =
              {{" " , 1 , False,  0, False, False, False, False, EvalExpression_DummyOp},
               {"~" , 1 , False,  1, True , False, False, False, EvalExpression_OneComplOp},
               {"<<", 2 , True ,  3, True , False, False, False, EvalExpression_ShLeftOp},
               {">>", 2 , True ,  3, True , False, False, False, EvalExpression_ShRightOp},
               {"><", 2 , True ,  4, True , False, False, False, EvalExpression_BitMirrorOp},
               {"&" , 1 , True ,  5, True , False, False, False, EvalExpression_BinAndOp},
               {"|" , 1 , True ,  6, True , False, False, False, EvalExpression_BinOrOp},
               {"!" , 1 , True ,  7, True , False, False, False, EvalExpression_BinXorOp},
               {"^" , 1 , True ,  8, True , True , False, False, EvalExpression_PotOp},
               {"*" , 1 , True , 11, True , True , False, False, EvalExpression_MultOp},
               {"/" , 1 , True , 11, True , True , False, False, EvalExpression_DivOp},
               {"#" , 1 , True , 11, True , False, False, False, EvalExpression_ModOp},
               {"+" , 1 , True , 13, True , True , True , False, EvalExpression_AddOp},
               {"-" , 1 , True , 13, True , True , False, False, EvalExpression_SubOp},
               {"~~", 2 , False,  2, True , False, False, False, EvalExpression_LogNotOp},
               {"&&", 2 , True , 15, True , False, False, False, EvalExpression_LogAndOp},
               {"||", 2 , True , 16, True , False, False, False, EvalExpression_LogOrOp},
               {"!!", 2 , True , 17, True , False, False, False, EvalExpression_LogXorOp},
               {"=" , 1 , True , 23, True , True , True , False, EvalExpression_EqOp},
               {"==", 2 , True , 23, True , True , True , False, EvalExpression_EqOp},
               {">" , 1 , True , 23, True , True , True , False, EvalExpression_GtOp},
               {"<" , 1 , True , 23, True , True , True , False, EvalExpression_LtOp},
               {"<=", 2 , True , 23, True , True , True , False, EvalExpression_LeOp},
               {">=", 2 , True , 23, True , True , True , False, EvalExpression_GeOp},
               {"<>", 2 , True , 23, True , True , True , False, EvalExpression_UneqOp},
               /* termination marker */
               {NULL, 0 , False,  0, False, False, False, False, NULL}},
               /* minus may have one or two operands */
                MinusMonadicOperator = 
               {"-" ,1 , False, 13, True , True , False, False, EvalExpression_SubOp};
   Operator *pOp;
   Operator *FOps[sizeof(Operators) / sizeof(*Operators)];
   LongInt FOpCnt = 0;

   Boolean OK, FFound;
   TempResult LVal, RVal, MVal;
   int z1, cnt;
   char Save='\0';
   sint LKlamm, RKlamm, WKlamm, zop;
   sint OpMax, OpPos = -1;
   Boolean InHyp, InQuot;
   Double  FVal;
   PSymbolEntry Ptr;
   PFunction ValFunc;
   String Copy, stemp, ftemp;
   char *KlPos, *zp, *DummyPtr, *pOpPos;
   PRelocEntry TReloc;

   ChkStack();

   if (MakeDebug)
     fprintf(Debug, "Parse '%s'\n", pExpr);

   memset(&LVal, 0, sizeof(LVal));
   memset(&RVal, 0, sizeof(RVal));

   /* Annahme Fehler */

   pErg->Typ = TempNone;
   pErg->Relocs = Nil;

   (void)CopyNoBlanks(Copy, pExpr, STRINGSIZE);

   /* sort out local symbols like - and +++.  Do it now to get them out of the
      formula parser's way. */

   if (ChkTmp2(Copy, FALSE))
     strmaxcpy(stemp, Copy, STRINGSIZE);
   else 
     strmaxcpy(stemp, pExpr, STRINGSIZE);

   /* Programmzaehler ? */

   if ((PCSymbol) && (!strcasecmp(Copy, PCSymbol)))
   {
     pErg->Typ = TempInt;
     pErg->Contents.Int = EProgCounter();
     pErg->Relocs = Nil;
     LEAVE;
   }

   /* Konstanten ? */

   pErg->Contents.Int = ConstIntVal(Copy, (IntType) (IntTypeCnt-1), &OK);
   if (OK)
   {
     pErg->Typ = TempInt;
     pErg->Relocs = Nil;
     LEAVE;
   }

   pErg->Contents.Float = ConstFloatVal(Copy, Float80, &OK);
   if (OK)
   {
     pErg->Typ = TempFloat;
     pErg->Relocs = Nil;
     LEAVE;
   }

   ConstStringVal(Copy, &pErg->Contents.Ascii, &OK);
   if (OK)
   {
     pErg->Typ = TempString;
     pErg->Relocs = Nil;
     LEAVE;
   }

   /* durch Codegenerator gegebene Konstanten ? */

   pErg->Relocs = Nil;
   InternSymbol(Copy, pErg);
   if (pErg->Typ != TempNone)
     LEAVE;

   /* find out which operators *might* occur in expression */

   OpMax = 0; LKlamm = 0; RKlamm = 0; WKlamm = 0;
   InHyp = False; InQuot = False;
   for (pOp = Operators + 1; pOp->Id; pOp++)
   {
     pOpPos = (pOp->IdLen == 1) ? (strchr(Copy, *pOp->Id)) : (strstr(Copy, pOp->Id));
     if (pOpPos)
       FOps[FOpCnt++] = pOp;
   }

   /* nach Operator hoechster Rangstufe ausserhalb Klammern suchen */

   for (zp = Copy; *zp; zp++)
   {
     switch (*zp)
     {
       case '(':
         if (!(InHyp || InQuot)) LKlamm++; break;
       case ')':
         if (!(InHyp || InQuot)) RKlamm++; break;
       case '{':
         if (!(InHyp || InQuot)) WKlamm++; break;
       case '}':
         if (!(InHyp || InQuot)) WKlamm--; break;
       case '"':
         if (!InHyp) InQuot = !InQuot; break;
       case '\'':
         if (!InQuot) InHyp = !InHyp; break;
       default: 
         if ((LKlamm == RKlamm) && (WKlamm == 0) && (!InHyp) && (!InQuot))
         {
           Boolean OpFnd = False;
           sint OpLen = 0, LocOpMax = 0;

           for (zop = 0; zop < FOpCnt; zop++)
           {
             pOp = FOps[zop];
             if ((!strncmp(zp, pOp->Id, pOp->IdLen)) && (pOp->IdLen >= OpLen))
             {
               OpFnd = True; OpLen = pOp->IdLen;
               LocOpMax = pOp - Operators;
               if (Operators[LocOpMax].Priority >= Operators[OpMax].Priority)
               {
                 OpMax = LocOpMax; OpPos = zp - Copy;
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
     WrXError(1300, Copy); LEAVE;
   }

   /* Operator gefunden ? */

   if (OpMax)
   {
     pOp = Operators + OpMax;

     /* Minuszeichen sowohl mit einem als auch 2 Operanden */

     if (strcmp(pOp->Id, "-") == 0)
     {
       if (!OpPos)
         pOp = &MinusMonadicOperator;
     }

     /* Operandenzahl pruefen */

     if (((pOp->Dyadic) == (OpPos == 0))
      || (OpPos == (int)strlen(Copy)-1))
     {
       WrError(1110); LEAVE;
     }

     /* Teilausdruecke rekursiv auswerten */

     Save = Copy[OpPos]; Copy[OpPos] = '\0';
     EvalExpression(Copy + OpPos + strlen(pOp->Id), &RVal);
     if (pOp->Dyadic)
       EvalExpression(Copy, &LVal);
     else if (RVal.Typ == TempFloat)
     {
       LVal.Typ = TempFloat; LVal.Contents.Float = 0.0;
     }
     else
     {
       LVal.Typ = TempInt; LVal.Contents.Int = 0; LVal.Relocs = Nil;
     }
     Copy[OpPos] = Save;

     /* Abbruch, falls dabei Fehler */

     if ((LVal.Typ == TempNone) || (RVal.Typ == TempNone))
       LEAVE;

     /* relokatible Symbole nur fuer + und - erlaubt */

     if ((OpMax != 12) && (OpMax != 13) && ((LVal.Relocs != Nil) || (RVal.Relocs != Nil)))
     {
       WrError(1150);
       LEAVE;
     }

     /* both operands for a dyadic operator must have same type */

     if ((pOp->Dyadic) && (LVal.Typ != RVal.Typ))
     {
       if ((LVal.Typ == TempString) || (RVal.Typ == TempString))
       {
         WrError(1135); LEAVE;
       }
       if (LVal.Typ == TempInt) EvalExpression_ChgFloat(&LVal);
       if (RVal.Typ == TempInt) EvalExpression_ChgFloat(&RVal);
     }

     /* optionally convert int to float if operator only supports float */

     switch (RVal.Typ)
     {
       case TempInt:
         if (!pOp->MayInt)
         {
           if (!pOp->MayFloat)
           {
             WrError(1135); LEAVE;
           }
           else
           {
             EvalExpression_ChgFloat(&RVal); 
             if (pOp->Dyadic) EvalExpression_ChgFloat(&LVal);
           }
         }
         break;
       case TempFloat: 
         if (!pOp->MayFloat)
         {
           WrError(1135); LEAVE;
         }
         break;
       case TempString:
         if (!pOp->MayString)
         {
           WrError(1135); LEAVE;
         }
         break;
       default:
         break;
     }

     /* Operanden abarbeiten */

     pOp->pFunc(pErg, &LVal, &RVal);
     LEAVE;
   } /* if (OpMax) */

   /* kein Operator gefunden: Klammerausdruck ? */

   if (LKlamm!=0)
    BEGIN

     /* erste Klammer suchen, Funktionsnamen abtrennen */

     KlPos=strchr(Copy,'(');

     /* Funktionsnamen abschneiden */

     *KlPos='\0'; strmaxcpy(ftemp,Copy,255);
     strmov(Copy,KlPos+1); Copy[strlen(Copy)-1]='\0'; 

     /* Nullfunktion: nur Argument */

     if (ftemp[0]=='\0')
      BEGIN
       EvalExpression(Copy,&LVal);
       *pErg=LVal; LEAVE;
      END

     /* selbstdefinierte Funktion ? */

     if ((ValFunc = FindFunction(ftemp)))
     {
       strmaxcpy(ftemp, ValFunc->Definition, 255);
       for (z1 = 1; z1 <= ValFunc->ArguCnt; z1++)
       {
         if (!Copy[0])
         {
           WrError(1490); LEAVE;
         }

         KlPos = QuotPos(Copy,',');
         if (KlPos)
           *KlPos = '\0';

         EvalExpression(Copy, &LVal);
         if (LVal.Relocs != Nil)
         {
           WrError(1150); FreeRelocs(&LVal.Relocs); return;
         }

         if (!KlPos)
           Copy[0] = '\0';
         else
           strcpy(Copy, KlPos + 1);

         strmaxcpy(stemp, "(", 255);
         switch (LVal.Typ)
         {
           case TempInt:
             sprintf(stemp + 1, "%s", LargeString(LVal.Contents.Int));
             break;
           case TempFloat:
             sprintf(stemp + 1, "%0.16e", LVal.Contents.Float); 
             KillBlanks(stemp);
             break;
           case TempString:
             stemp[1] = '"';
             snstrlenprint(stemp + 2, 252, LVal.Contents.Ascii.Contents, LVal.Contents.Ascii.Length);
             strmaxcat(stemp,"\"",255);
             break;
           default:
             LEAVE;
         }
         strmaxcat(stemp,")", 255);
         ExpandLine(stemp, z1, ftemp);
       }
       if (Copy[0]!='\0')
        BEGIN
         WrError(1490); LEAVE;
        END
       EvalExpression(ftemp,pErg);
       LEAVE;
     }

     /* hier einmal umwandeln ist effizienter */

     NLS_UpString(ftemp);

     /* symbolbezogene Funktionen */

     if (strcmp(ftemp,"SYMTYPE")==0)
      BEGIN
       pErg->Typ=TempInt;
       if (FindRegDef(Copy,&DummyPtr)) pErg->Contents.Int=0x80;
       else pErg->Contents.Int=GetSymbolType(Copy);
       LEAVE;
      END

     /* Unterausdruck auswerten (interne Funktionen maxmimal mit drei Argumenten) */

     z1 = 0; KlPos = Copy;
     do
      BEGIN
       zp = QuotPos(KlPos, ',');
       if (zp != Nil) *zp = '\0';
       switch (z1)
        BEGIN
         case 0:
          EvalExpression(KlPos, &LVal);
          if (LVal.Typ == TempNone) LEAVE;
          TReloc = LVal.Relocs;
          break;
         case 1:
          EvalExpression(KlPos, &MVal);
          if (MVal.Typ == TempNone) LEAVE;
          TReloc = MVal.Relocs;
          break;
         case 2:
          EvalExpression(KlPos, &RVal);
          if (RVal.Typ == TempNone) LEAVE;
          TReloc = RVal.Relocs;
          break;
         default:
          WrError(1490); LEAVE;
        END
       if (TReloc != Nil)
        BEGIN
         WrError(1150); FreeRelocs(&TReloc); LEAVE;
        END
       if (zp != Nil) KlPos = zp + 1;
       z1++;
      END
     while (zp!=Nil);

     /* ein paar Funktionen mit zwei,drei Argumenten */

     if (z1 == 3)
     {
       if (!strcmp(ftemp, "SUBSTR"))
       {
         if ((LVal.Typ != TempString) || (MVal.Typ != TempInt) || (RVal.Typ != TempInt)) WrError(1135);
         else
         {
           cnt = LVal.Contents.Ascii.Length - MVal.Contents.Int;
           if ((RVal.Contents.Int != 0) && (RVal.Contents.Int < cnt))
             cnt = RVal.Contents.Int;
           if (cnt < 0)
             cnt = 0;
           pErg->Contents.Ascii.Length = 0;
           DynStringAppend(&pErg->Contents.Ascii, LVal.Contents.Ascii.Contents + MVal.Contents.Int, cnt);
           pErg->Typ = TempString;
         }
       }
       else
         WrXError(1860,ftemp);
       LEAVE;
     }
     else if (z1 == 2)
     {
       if (!strcmp(ftemp, "STRSTR"))
       { 
         if ((LVal.Typ != TempString) || (MVal.Typ != TempString)) WrError(1135);
         else
         {
           pErg->Contents.Int = DynStringFind(&LVal.Contents.Ascii, &MVal.Contents.Ascii);
           pErg->Typ = TempInt;
         }
       }
       else if (!strcmp(ftemp, "CHARFROMSTR"))
       {
         if ((LVal.Typ != TempString) || (MVal.Typ != TempInt)) WrError(1135);
         else  
         {
           pErg->Typ = TempInt;
           pErg->Contents.Int = ((MVal.Contents.Int >= 0) && (MVal.Contents.Int < LVal.Contents.Ascii.Length)) ? LVal.Contents.Ascii.Contents[MVal.Contents.Int] : -1;
         }
       }
       else
         WrXError(1860,ftemp);
       LEAVE;
     }

     /* expression type ? */

     if (!strcmp(ftemp, "EXPRTYPE"))
     {
       pErg->Typ=TempInt;
       switch (LVal.Typ)
       {
         case TempInt: pErg->Contents.Int = 0; break;
         case TempFloat: pErg->Contents.Int = 1; break;
         case TempString: pErg->Contents.Int = 2; break;
         default: pErg->Contents.Int = -1;
       }
       LEAVE;
     }

     /* Funktionen fuer Stringargumente */

     if (LVal.Typ == TempString)
     {
       /* in Grossbuchstaben wandeln ? */

       if (!strcmp(ftemp, "UPSTRING"))
       {
         pErg->Typ = TempString;
         DynString2DynString(&pErg->Contents.Ascii, &LVal.Contents.Ascii);
         for (KlPos = pErg->Contents.Ascii.Contents;
              KlPos < pErg->Contents.Ascii.Contents + pErg->Contents.Ascii.Length;
              KlPos++)
           *KlPos = mytoupper(*KlPos);
       }

       /* in Kleinbuchstaben wandeln ? */

       else if (!strcmp(ftemp, "LOWSTRING"))
       {
         pErg->Typ = TempString;
         DynString2DynString(&pErg->Contents.Ascii, &LVal.Contents.Ascii);
         for (KlPos = pErg->Contents.Ascii.Contents;
              KlPos < pErg->Contents.Ascii.Contents + pErg->Contents.Ascii.Length;
              KlPos++)
           *KlPos = mytolower(*KlPos);
       }

       /* Laenge ermitteln ? */

       else if (!strcmp(ftemp, "STRLEN"))
       {
         pErg->Typ = TempInt; pErg->Contents.Int = LVal.Contents.Ascii.Length;
       }

       /* Parser aufrufen ? */

       else if (!strcmp(ftemp, "VAL"))
       {
         String Tmp;

         DynString2CString(Tmp, &LVal.Contents.Ascii, sizeof(Tmp));
         EvalExpression(Tmp, pErg);
       }

       /* nix gefunden ? */

       else
       {
         WrXError(1860, ftemp); pErg->Typ = TempNone;
       }
     }

     /* Funktionen fuer Zahlenargumente */

     else
      BEGIN
       FFound=False; pErg->Typ=TempNone;

       /* reine Integerfunktionen */

       if (strcmp(ftemp,"TOUPPER")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else if ((LVal.Contents.Int<0) OR (LVal.Contents.Int>255)) WrError(1320);
         else
          BEGIN
           pErg->Typ=TempInt;
           pErg->Contents.Int=toupper(LVal.Contents.Int);
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"TOLOWER")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else if ((LVal.Contents.Int<0) OR (LVal.Contents.Int>255)) WrError(1320);
         else
          BEGIN
           pErg->Typ=TempInt;
           pErg->Contents.Int=tolower(LVal.Contents.Int);
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"BITCNT")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else
          BEGIN
           pErg->Typ=TempInt;
           pErg->Contents.Int=0;
           for (z1=0; z1<LARGEBITS; z1++)
            BEGIN
             pErg->Contents.Int+=(LVal.Contents.Int & 1);
             LVal.Contents.Int=LVal.Contents.Int >> 1;
            END
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"FIRSTBIT")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else
          BEGIN
           pErg->Typ=TempInt;
           pErg->Contents.Int=0;
           do
            BEGIN
             if (NOT Odd(LVal.Contents.Int)) pErg->Contents.Int++;
             LVal.Contents.Int=LVal.Contents.Int >> 1;
            END
           while ((pErg->Contents.Int<LARGEBITS) AND (NOT Odd(LVal.Contents.Int)));
           if (pErg->Contents.Int>=LARGEBITS) pErg->Contents.Int=(-1);
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"LASTBIT")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else
          BEGIN
           pErg->Typ=TempInt;
           pErg->Contents.Int=(-1);
           for (z1=0; z1<LARGEBITS; z1++)
            BEGIN
             if (Odd(LVal.Contents.Int)) pErg->Contents.Int=z1;
             LVal.Contents.Int=LVal.Contents.Int >> 1;
            END
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"BITPOS")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else
          BEGIN
           pErg->Typ=TempInt;
           if (NOT SingleBit(LVal.Contents.Int,&pErg->Contents.Int))
            BEGIN
             pErg->Contents.Int=(-1); WrError(1540);
            END
          END
         FFound=True;
        END

       /* variable Integer/Float-Funktionen */

       else if (strcmp(ftemp,"ABS")==0)
        BEGIN
         switch (pErg->Typ=LVal.Typ)
          BEGIN
           case TempInt: pErg->Contents.Int=abs(LVal.Contents.Int); break;
           case TempFloat: pErg->Contents.Float=fabs(LVal.Contents.Float);break;
           default: break;
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"SGN")==0)
        BEGIN
         pErg->Typ=TempInt;
         switch (LVal.Typ)
          BEGIN
           case TempInt:
            if (LVal.Contents.Int<0) pErg->Contents.Int=(-1);
            else if (LVal.Contents.Int>0) pErg->Contents.Int=1;
            else pErg->Contents.Int=0;
            break;
           case TempFloat:
            if (LVal.Contents.Float<0) pErg->Contents.Int=(-1);
            else if (LVal.Contents.Float>0) pErg->Contents.Int=1;
            else pErg->Contents.Int=0;
            break;
           default:
            break;
          END
         FFound=True;
        END

       /* Funktionen Float und damit auch Int */

       if (NOT FFound)
        BEGIN
         /* Typkonvertierung */

         EvalExpression_ChgFloat(&LVal); 
         pErg->Typ=TempFloat;

         /* Integerwandlung */

         if (strcmp(ftemp,"INT")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)>MaxLargeInt)
            BEGIN
             pErg->Typ=TempNone; WrError(1320);
            END
           else
            BEGIN
             pErg->Typ=TempInt; pErg->Contents.Int=(LargeInt) floor(LVal.Contents.Float);
            END
          END

         /* Quadratwurzel */

         else if (strcmp(ftemp,"SQRT")==0)
          BEGIN
           if (LVal.Contents.Float<0)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=sqrt(LVal.Contents.Float);
          END

         /* trigonometrische Funktionen */

         else if (strcmp(ftemp,"SIN")==0) pErg->Contents.Float=sin(LVal.Contents.Float);
         else if (strcmp(ftemp,"COS")==0) pErg->Contents.Float=cos(LVal.Contents.Float);
         else if (strcmp(ftemp,"TAN")==0) 
          BEGIN
           if (cos(LVal.Contents.Float)==0.0)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=tan(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"COT")==0)
          BEGIN
           if ((FVal=sin(LVal.Contents.Float))==0.0)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=cos(LVal.Contents.Float)/FVal;
          END

         /* inverse trigonometrische Funktionen */

         else if (strcmp(ftemp,"ASIN")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)>1)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=asin(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"ACOS")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)>1)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=acos(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"ATAN")==0) pErg->Contents.Float=atan(LVal.Contents.Float);
         else if (strcmp(ftemp,"ACOT")==0) pErg->Contents.Float=M_PI/2-atan(LVal.Contents.Float);

         /* exponentielle & hyperbolische Funktionen */

         else if (strcmp(ftemp,"EXP")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=exp(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"ALOG")==0)
          BEGIN
           if (LVal.Contents.Float>308)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=exp(LVal.Contents.Float*log(10.0));
          END
         else if (strcmp(ftemp,"ALD")==0)
          BEGIN
           if (LVal.Contents.Float>1022)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=exp(LVal.Contents.Float*log(2.0));
          END
         else if (strcmp(ftemp,"SINH")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=sinh(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"COSH")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=cosh(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"TANH")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=tanh(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"COTH")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else if ((FVal=tanh(LVal.Contents.Float))==0.0)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=1.0/FVal;
          END

         /* logarithmische & inverse hyperbolische Funktionen */

         else if (strcmp(ftemp,"LN")==0)
          BEGIN
           if (LVal.Contents.Float<=0)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=log(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"LOG")==0)
          BEGIN
           if (LVal.Contents.Float<=0)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=log10(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"LD")==0)
          BEGIN
           if (LVal.Contents.Float<=0)
            BEGIN
             pErg->Typ=TempNone; WrError(1870);
            END
           else pErg->Contents.Float=log(LVal.Contents.Float)/log(2.0);
          END
         else if (strcmp(ftemp,"ASINH")==0) 
          pErg->Contents.Float=log(LVal.Contents.Float+sqrt(LVal.Contents.Float*LVal.Contents.Float+1));
         else if (strcmp(ftemp,"ACOSH")==0)
          BEGIN
           if (LVal.Contents.Float<1)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=log(LVal.Contents.Float+sqrt(LVal.Contents.Float*LVal.Contents.Float-1));
          END
         else if (strcmp(ftemp,"ATANH")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)>=1)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=0.5*log((1+LVal.Contents.Float)/(1-LVal.Contents.Float));
          END
         else if (strcmp(ftemp,"ACOTH")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)<=1)
            BEGIN
             pErg->Typ=TempNone; WrError(1880);
            END
           else pErg->Contents.Float=0.5*log((LVal.Contents.Float+1)/(LVal.Contents.Float-1));
          END

         /* nix gefunden ? */

         else
          BEGIN
           WrXError(1860,ftemp); pErg->Typ=TempNone;
          END
        END
      END
     LEAVE;
    END

   /* nichts dergleichen, dann einfaches Symbol: urspruenglichen Wert wieder
      herstellen, dann Pruefung auf $$-tempraere Symbole */

   strmaxcpy(Copy,stemp,255); KillPrefBlanks(Copy); KillPostBlanks(Copy);

   ChkTmp1(Copy, FALSE);

   /* interne Symbole ? */

   if (strcasecmp(Copy,"MOMFILE")==0)
   {
     pErg->Typ = TempString;
     CString2DynString(&pErg->Contents.Ascii, CurrFileName);
     LEAVE;
    END;

   if (strcasecmp(Copy,"MOMLINE")==0)
    BEGIN
     pErg->Typ=TempInt;
     pErg->Contents.Int=CurrLine;
     LEAVE;
    END

   if (strcasecmp(Copy,"MOMPASS")==0)
    BEGIN
     pErg->Typ=TempInt;
     pErg->Contents.Int=PassNo;
     LEAVE;
    END

   if (!strcasecmp(Copy, "MOMSECTION"))
   {
     pErg->Typ = TempString;
     CString2DynString(&pErg->Contents.Ascii, GetSectionName(MomSectionHandle));
     LEAVE;
   }

   if (!strcasecmp(Copy, "MOMSEGMENT"))
   {
     pErg->Typ = TempString;
     CString2DynString(&pErg->Contents.Ascii, SegNames[ActPC]);
     LEAVE;
   }

   if (NOT ExpandSymbol(Copy)) LEAVE;

   KlPos=strchr(Copy,'[');
   if (KlPos!=Nil) 
    BEGIN
     Save=(*KlPos); *KlPos='\0';
    END
   OK=ChkSymbName(Copy);
   if (KlPos!=Nil) *KlPos=Save;
   if (NOT OK)
    BEGIN
     WrXError(1020,Copy); LEAVE;
    END;

   Ptr = FindLocNode(Copy, TempAll);
   if (Ptr == Nil) Ptr=FindNode(Copy, TempAll);
   if (Ptr != Nil)
   {
     switch (pErg->Typ = Ptr->SymWert.Typ)
     {
       case TempInt:
         pErg->Contents.Int = Ptr->SymWert.Contents.IWert;
         break;
       case TempFloat:
         pErg->Contents.Float = Ptr->SymWert.Contents.FWert;
         break;
       case TempString:
         pErg->Contents.Ascii.Length = 0;
         DynStringAppend(&pErg->Contents.Ascii, Ptr->SymWert.Contents.String.Contents, Ptr->SymWert.Contents.String.Length);
         break;
       default:
         break;
     }
     if (pErg->Typ != TempNone) pErg->Relocs = DupRelocs(Ptr->Relocs);
     if (Ptr->SymType != 0) TypeFlag |= (1 << Ptr->SymType);
     if ((Ptr->SymSize != (-1)) AND (SizeFlag == (-1))) SizeFlag = Ptr->SymSize;
     if (!Ptr->Defined)
     {
       if (Repass) SymbolQuestionable = True;
       UsesForwards = True;
     }
     Ptr->Used = True;
     LEAVE;
   }

   /* Symbol evtl. im ersten Pass unbekannt */

   if (PassNo<=MaxSymPass)
    BEGIN
     pErg->Typ=TempInt; pErg->Contents.Int=EProgCounter();
     Repass=True;
     if ((MsgIfRepass) AND (PassNo>=PassNoForMessage)) WrXError(170,Copy);
     FirstPassUnknown=True;
    END

   /* alles war nix, Fehler */

   else WrXError(1010,Copy);

func_exit:

   if (LVal.Relocs != NULL) FreeRelocs(&LVal.Relocs);
   if (RVal.Relocs != NULL) FreeRelocs(&RVal.Relocs);
END

static int TypeNums[] = {0, Num_OpTypeInt, Num_OpTypeFloat, 0, Num_OpTypeString, 0, 0, 0};

LargeInt EvalIntExpression(const char *pExpr, IntType Type, Boolean *pResult)
{
  TempResult t;
  LargeInt Result;

  *pResult = False;
  TypeFlag = 0; SizeFlag = (-1);
  UsesForwards = False;
  SymbolQuestionable = False;
  FirstPassUnknown = False;

  EvalExpression(pExpr, &t);
  SetRelocs(t.Relocs);
  switch (t.Typ)
  {
    case TempInt:
      Result = t.Contents.Int;
      break;
    case TempString:
    {
      int l = t.Contents.Ascii.Length;

      if ((l > 0) && (l <= 4))
      {
        char *pRun;
        Byte Digit;

        Result = 0;
        for (pRun = t.Contents.Ascii.Contents;
             pRun < t.Contents.Ascii.Contents + l;
             pRun++)
        {
          Digit = (usint) *pRun;
          Result = (Result << 8) | CharTransTable[Digit & 0xff];
        }
        break;
      }
    }
    default:
      if (t.Typ != TempNone)
      {
        char Msg[50];

        sprintf(Msg, "%s %s %s %s", 
                getmessage(Num_ErrMsgExpected), getmessage(Num_OpTypeInt),
                getmessage(Num_ErrMsgButGot), getmessage(TypeNums[t.Typ]));
        WrXError(1135, Msg);
      }
      FreeRelocs(&LastRelocs);
      return -1;
  }

  if (FirstPassUnknown)
    Result &= IntMasks[(int)Type];

  if (!RangeCheck(Result, Type))
  {
    if (HardRanges)
    {
      FreeRelocs(&LastRelocs);
      WrError(1320); return -1;
    }
    else
    {
      WrError(260);
      *pResult = True;
      return Result & IntMasks[(int)Type];
    }
  }
  else
  {
    *pResult = True;
    return Result;
  }
}

LargeInt EvalIntDisplacement(char *pExpr, IntType Type, Boolean *pResult)
{
  char SaveLeft;
  LargeInt Result;

  /* save the original character.  We assume there is space to the left! */

  pExpr--;
  SaveLeft = *pExpr; *pExpr = '0';
  
  /* evaluate */

  Result = EvalIntExpression(pExpr, Type, pResult);

  /* restore character */

  *pExpr = SaveLeft;

  return Result;
}

Double EvalFloatExpression(const char *pExpr, FloatType Type, Boolean *pResult)
{
  TempResult t;

  *pResult = False;
  TypeFlag = 0; SizeFlag = -1;
  UsesForwards = False;
  SymbolQuestionable = False;
  FirstPassUnknown = False;

  EvalExpression(pExpr, &t);
  switch (t.Typ)
  {
    case TempNone:
     return -1;
    case TempInt:
     t.Contents.Float = t.Contents.Int;
     break;
    case TempString:
    {
     char Msg[50];

     sprintf(Msg, "%s %s %s %s",
              getmessage(Num_ErrMsgExpected), getmessage(Num_OpTypeFloat),
              getmessage(Num_ErrMsgButGot), getmessage(Num_OpTypeString)); 
     WrXError(1135, Msg);
     return -1;
    }
    default:
     break;
  }

  if (!FloatRangeCheck(t.Contents.Float, Type))
  {
    WrError(1320); return -1;
  }

  *pResult = True; return t.Contents.Float;
}

void EvalStringExpression(const char *pExpr, Boolean *pResult, char *pEvalResult)
{
  TempResult t;

  *pResult = False;
  TypeFlag = 0; SizeFlag = -1;
  UsesForwards = False;
  SymbolQuestionable = False;
  FirstPassUnknown = False;

  EvalExpression(pExpr, &t);
  if (t.Typ != TempString)
  {
    *pEvalResult = '\0';
    if (t.Typ != TempNone)
    {
      char Msg[50];

      sprintf(Msg, "%s %s %s %s",
              getmessage(Num_ErrMsgExpected), getmessage(Num_OpTypeString),
              getmessage(Num_ErrMsgButGot), getmessage(TypeNums[t.Typ]));
      WrXError(1135, Msg);
    }
  }
  else
  {
    DynString2CString(pEvalResult, &t.Contents.Ascii, 255);
    *pResult = True;
  }
}


static void FreeSymbolEntry(PSymbolEntry *Node, Boolean Destroy)
{
   PCrossRef Lauf;

   if ((*Node)->Tree.Name)
   {
     free((*Node)->Tree.Name); (*Node)->Tree.Name = NULL;
   }

   if ((*Node)->SymWert.Typ == TempString)
     free((*Node)->SymWert.Contents.String.Contents);

   while ((*Node)->RefList != Nil)
   {
     Lauf = (*Node)->RefList->Next;
     free((*Node)->RefList);
     (*Node)->RefList = Lauf;
   }

   FreeRelocs(&((*Node)->Relocs));

   if (Destroy)
   {
     free(*Node); Node = NULL;
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
    NewEntry->Defined = True; NewEntry->Used = False;
    NewEntry->Changeable = EnterStruct->MayChange;
    NewEntry->RefList = Nil;
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

  if (((*Node)->Defined) AND (!(*Node)->Changeable) AND (NOT EnterStruct->MayChange))
  {
    strmaxcpy(serr, (*Node)->Tree.Name, 255);
    if (EnterStruct->DoCross)
    {
      sprintf(snum, ",%s %s:%ld", getmessage(Num_PrevDefMsg),
              GetFileName((*Node)->FileNum), (long)((*Node)->LineNum));
      strmaxcat(serr, snum, 255);
    }
    WrXError(1000, serr);
    FreeSymbolEntry(&NewEntry, TRUE);
    return False;
  }

  /* tried to reassign a constant (EQU) a value with SET and vice versa ? */

  else if ( ((*Node)->Defined) AND (EnterStruct->MayChange != (*Node)->Changeable) )
  {
    strmaxcpy(serr, (*Node)->Tree.Name, 255);
    if (EnterStruct->DoCross)
    {
      sprintf(snum, ",%s %s:%ld", getmessage(Num_PrevDefMsg),
              GetFileName((*Node)->FileNum), (long)((*Node)->LineNum));
      strmaxcat(serr, snum, 255);
    }
    WrXError((*Node)->Changeable ? 2035 : 2030, serr);
    FreeSymbolEntry(&NewEntry, TRUE);
    return False;
  }

  else
  {
    if (NOT EnterStruct->MayChange)
    {
      if ((NewEntry->SymWert.Typ != (*Node)->SymWert.Typ)
       OR ((NewEntry->SymWert.Typ == TempString) AND (strlencmp(NewEntry->SymWert.Contents.String.Contents, NewEntry->SymWert.Contents.String.Length, (*Node)->SymWert.Contents.String.Contents, (*Node)->SymWert.Contents.String.Length) != 0))
       OR ((NewEntry->SymWert.Typ == TempFloat ) AND (NewEntry->SymWert.Contents.FWert != (*Node)->SymWert.Contents.FWert))
       OR ((NewEntry->SymWert.Typ == TempInt   ) AND (NewEntry->SymWert.Contents.IWert != (*Node)->SymWert.Contents.IWert)))
       {
         if ((NOT Repass) AND (JmpErrors>0))
         {
           if (ThrowErrors) ErrorCount -= JmpErrors;
           JmpErrors = 0;
         }
         Repass = True;
         if ((MsgIfRepass) AND (PassNo >= PassNoForMessage))
         {
           strmaxcpy(serr, Neu->Name, 255);
           if (Neu->Attribute != (-1)) 
           {
             strmaxcat(serr, "[", 255);
             strmaxcat(serr, GetSectionName(Neu->Attribute), 255);
             strmaxcat(serr, "]", 255);
           }
           WrXError(80, serr);
         }
       }
    }
    if (EnterStruct->DoCross)
    {
      NewEntry->LineNum = (*Node)->LineNum; NewEntry->FileNum = (*Node)->FileNum;
    }
    NewEntry->RefList = (*Node)->RefList; (*Node)->RefList = Nil;
    NewEntry->Defined = True; NewEntry->Used = (*Node)->Used; NewEntry->Changeable = EnterStruct->MayChange;
    FreeSymbolEntry(Node, False);
    return True;
  }
}

static void EnterLocSymbol(PSymbolEntry Neu)
{
   TEnterStruct EnterStruct;
   PTree TreeRoot;

   Neu->Tree.Attribute = MomLocHandle;
   if (!CaseSensitive) NLS_UpString(Neu->Tree.Name);
   EnterStruct.MayChange = EnterStruct.DoCross = FALSE;
   TreeRoot = &FirstLocSymbol->Tree;
   EnterTree(&TreeRoot, (&Neu->Tree), SymbolAdder, &EnterStruct);
   FirstLocSymbol = (PSymbolEntry)TreeRoot;
}

        static void EnterSymbol_Search(PForwardSymbol *Lauf, PForwardSymbol *Prev,
                                       PForwardSymbol **RRoot, PSymbolEntry Neu,
                                       PForwardSymbol *Root, Byte ResCode, Byte *SearchErg)
BEGIN
   *Lauf=(*Root); *Prev=Nil; *RRoot=Root;
   while ((*Lauf!=Nil) AND (strcmp((*Lauf)->Name,Neu->Tree.Name)!=0))
    BEGIN
     *Prev=(*Lauf); *Lauf=(*Lauf)->Next;
    END
   if (*Lauf!=Nil) *SearchErg=ResCode;
END

static void EnterSymbol(PSymbolEntry Neu, Boolean MayChange, LongInt ResHandle)
{
   PForwardSymbol Lauf,Prev;
   PForwardSymbol *RRoot;
   Byte SearchErg;
   String CombName;
   PSaveSection RunSect;
   LongInt MSect;
   PSymbolEntry Copy;
   TEnterStruct EnterStruct;
   PTree TreeRoot = &(FirstSymbol->Tree);

   if (NOT CaseSensitive) NLS_UpString(Neu->Tree.Name);

   SearchErg = 0;
   EnterStruct.MayChange = MayChange; EnterStruct.DoCross = MakeCrossList;
   Neu->Tree.Attribute = (ResHandle == (-2)) ? (MomSectionHandle) : (ResHandle);
   if ((SectionStack != Nil) && (Neu->Tree.Attribute == MomSectionHandle))
   {
     EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu, &(SectionStack->LocSyms),
                        1, &SearchErg);
     if (Lauf == Nil)
       EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu,
                          &(SectionStack->GlobSyms), 2, &SearchErg);
     if (Lauf == Nil)
       EnterSymbol_Search(&Lauf, &Prev, &RRoot, Neu,
                          &(SectionStack->ExportSyms), 3, &SearchErg);
     if (SearchErg == 2)
       Neu->Tree.Attribute = Lauf->DestSection;
     if (SearchErg == 3)
     {
       strmaxcpy(CombName, Neu->Tree.Name, 255);
       RunSect = SectionStack; MSect = MomSectionHandle;
       while ((MSect != Lauf->DestSection) AND (RunSect != Nil))
       {
         strmaxprep(CombName, "_", 255);
         strmaxprep(CombName, GetSectionName(MSect), 255);
         MSect = RunSect->Handle; RunSect = RunSect->Next;
       }
       Copy = (PSymbolEntry) malloc(sizeof(TSymbolEntry)); *Copy = (*Neu);
       Copy->Tree.Name = strdup(CombName);
       Copy->Tree.Attribute = Lauf->DestSection;
       Copy->Relocs = DupRelocs(Neu->Relocs);
       if (Copy->SymWert.Typ == TempString)
       {
         Copy->SymWert.Contents.String.Contents = (char*)malloc(Neu->SymWert.Contents.String.Length);
         memcpy(Copy->SymWert.Contents.String.Contents, Neu->SymWert.Contents.String.Contents,
                Copy->SymWert.Contents.String.Length = Neu->SymWert.Contents.String.Length);
       }
       EnterTree(&TreeRoot, &(Copy->Tree), SymbolAdder, &EnterStruct);
     }
     if (Lauf != Nil)
     {
       free(Lauf->Name);
       if (Prev == Nil) *RRoot = Lauf->Next;
       else Prev->Next = Lauf->Next;
       free(Lauf);
     }
   }
   EnterTree(&TreeRoot, &(Neu->Tree), SymbolAdder, &EnterStruct);
   FirstSymbol = (PSymbolEntry)TreeRoot;
}

        void PrintSymTree(char *Name)
BEGIN
   fprintf(Debug,"---------------------\n");
   fprintf(Debug,"Enter Symbol %s\n\n",Name);
   PrintSymbolTree(); PrintSymbolDepth();
END

void EnterIntSymbol(char *Name_O, LargeInt Wert, Byte Typ, Boolean MayChange)
{
  PSymbolEntry Neu;
  LongInt DestHandle;   
  String Name;

  strmaxcpy(Name, Name_O, 255);
  if (!ExpandSymbol(Name))
    return;
  if (!GetSymSection(Name, &DestHandle))
    return;
  (void)ChkTmp(Name, TRUE);
  if (!ChkSymbName(Name))
  {
    WrXError(1020, Name);
    return;
  }

  Neu = (PSymbolEntry) malloc(sizeof(TSymbolEntry));
  Neu->Tree.Name = strdup(Name);
  Neu->SymWert.Typ = TempInt;
  Neu->SymWert.Contents.IWert = Wert;
  Neu->SymType = Typ;
  Neu->SymSize = (-1);
  Neu->RefList = Nil;
  Neu->Relocs = Nil;

  if ((MomLocHandle == (-1)) || (DestHandle != (-2)))
  {
    EnterSymbol(Neu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(Name);
  }
  else
    EnterLocSymbol(Neu);
}

        void EnterExtSymbol(char *Name_O, LargeInt Wert, Byte Typ, Boolean MayChange)
BEGIN
   PSymbolEntry Neu;
   LongInt DestHandle;
   String Name;

   strmaxcpy(Name, Name_O, 255);
   if (NOT ExpandSymbol(Name)) return;
   if (NOT GetSymSection(Name, &DestHandle)) return;
   if (NOT ChkSymbName(Name))
    BEGIN
     WrXError(1020, Name); return;
    END

   Neu=(PSymbolEntry) malloc(sizeof(TSymbolEntry));
   Neu->Tree.Name = strdup(Name);
   Neu->SymWert.Typ = TempInt;
   Neu->SymWert.Contents.IWert = Wert;
   Neu->SymType = Typ;
   Neu->SymSize = (-1);
   Neu->RefList = Nil;
   Neu->Relocs = (PRelocEntry) malloc(sizeof(TRelocEntry));
   Neu->Relocs->Next = Nil;
   Neu->Relocs->Ref = strdup(Name);
   Neu->Relocs->Add = True;

   if ((MomLocHandle == (-1)) OR (DestHandle != (-2)))
   {
     EnterSymbol(Neu, MayChange, DestHandle);
     if (MakeDebug)
       PrintSymTree(Name);
   }
   else EnterLocSymbol(Neu);
END

        void EnterRelSymbol(char *Name_O, LargeInt Wert, Byte Typ, Boolean MayChange)
BEGIN
   PSymbolEntry Neu;
   LongInt DestHandle;
   String Name;

   strmaxcpy(Name, Name_O, 255);
   if (NOT ExpandSymbol(Name)) return;
   if (NOT GetSymSection(Name, &DestHandle)) return;
   if (NOT ChkSymbName(Name))
    BEGIN
     WrXError(1020, Name); return;
    END

   Neu=(PSymbolEntry) malloc(sizeof(TSymbolEntry));
   Neu->Tree.Name = strdup(Name);
   Neu->SymWert.Typ = TempInt;
   Neu->SymWert.Contents.IWert = Wert;
   Neu->SymType = Typ;
   Neu->SymSize = (-1);
   Neu->RefList = Nil;
   Neu->Relocs = (PRelocEntry) malloc(sizeof(TRelocEntry));
   Neu->Relocs->Next = Nil;
   Neu->Relocs->Ref = strdup(RelName_SegStart);
   Neu->Relocs->Add = True;

   if ((MomLocHandle == (-1)) OR (DestHandle != (-2)))
    BEGIN
     EnterSymbol(Neu, MayChange, DestHandle);
     if (MakeDebug) PrintSymTree(Name);
    END
   else EnterLocSymbol(Neu);
END

void EnterFloatSymbol(char *Name_O, Double Wert, Boolean MayChange)
{
  PSymbolEntry Neu;
  LongInt DestHandle;
  String Name;

  strmaxcpy(Name, Name_O, 255);
  if (!ExpandSymbol(Name))
    return;
  if (!GetSymSection(Name, &DestHandle))
    return;
  (void)ChkTmp(Name, TRUE);
  if (!ChkSymbName(Name))
  {
    WrXError(1020, Name);
    return;
  }
  Neu = (PSymbolEntry) malloc(sizeof(TSymbolEntry));
  Neu->Tree.Name = strdup(Name);
  Neu->SymWert.Typ = TempFloat;
  Neu->SymWert.Contents.FWert = Wert;
  Neu->SymType = 0;
  Neu->SymSize = (-1);
  Neu->RefList = Nil;
  Neu->Relocs = Nil;

  if ((MomLocHandle == (-1)) || (DestHandle != (-2)))
  {
    EnterSymbol(Neu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(Name);
  }
  else
    EnterLocSymbol(Neu);
}

void EnterDynStringSymbol(char *Name_O, const tDynString *pValue, Boolean MayChange)
{
  PSymbolEntry Neu;
  LongInt DestHandle;
  String Name;

  strmaxcpy(Name, Name_O, 255);
  if (!ExpandSymbol(Name)) return;
  if (!GetSymSection(Name, &DestHandle)) return;
  (void)ChkTmp(Name, TRUE);
  if (!ChkSymbName(Name))
  {
    WrXError(1020, Name); return;
  }
  Neu = (PSymbolEntry) malloc(sizeof(TSymbolEntry));
  Neu->Tree.Name = strdup(Name);
  Neu->SymWert.Contents.String.Contents = (char*)malloc(pValue->Length);
  memcpy(Neu->SymWert.Contents.String.Contents, pValue->Contents, pValue->Length);
  Neu->SymWert.Contents.String.Length = pValue->Length;
  Neu->SymWert.Typ = TempString;
  Neu->SymType = 0;
  Neu->SymSize = -1;
  Neu->RefList = Nil;
  Neu->Relocs = Nil;

  if ((MomLocHandle == (-1)) || (DestHandle != (-2)))
  {
    EnterSymbol(Neu, MayChange, DestHandle);
    if (MakeDebug)
      PrintSymTree(Name);
  }
  else EnterLocSymbol(Neu);
}

void EnterStringSymbol(char *Name_O, const char *pValue, Boolean MayChange)
{
  tDynString DynString;

  DynString.Length = 0;
  DynStringAppend(&DynString, pValue, -1);
  EnterDynStringSymbol(Name_O, &DynString, MayChange);
}

        static void AddReference(PSymbolEntry Node)
BEGIN
   PCrossRef Lauf,Neu;

   /* Speicher belegen */

   Neu=(PCrossRef) malloc(sizeof(TCrossRef));
   Neu->LineNum=CurrLine; Neu->OccNum=1; Neu->Next=Nil;

   /* passende Datei heraussuchen */

   Neu->FileNum=GetFileNum(CurrFileName);

   /* suchen, ob Eintrag schon existiert */

   Lauf=Node->RefList;
   while ((Lauf!=Nil)
      AND ((Lauf->FileNum!=Neu->FileNum) OR (Lauf->LineNum!=Neu->LineNum)))
    Lauf=Lauf->Next;

   /* schon einmal in dieser Datei in dieser Zeile aufgetaucht: nur Zaehler
     rauf: */

   if (Lauf!=Nil)
    BEGIN
     Lauf->OccNum++; free(Neu);
    END

   /* ansonsten an Kettenende anhaengen */

   else if (Node->RefList==Nil) Node->RefList=Neu;

   else
    BEGIN
     Lauf=Node->RefList;
     while (Lauf->Next!=Nil) Lauf=Lauf->Next;
     Lauf->Next=Neu;
    END
END

static PSymbolEntry FindNode_FNode(char *Name, TempType SearchType, LongInt Handle)
{
   PSymbolEntry Lauf;
 
   Lauf = (PSymbolEntry) SearchTree((PTree)FirstSymbol, Name, Handle);

   if (Lauf!=Nil)
   {
     if (Lauf->SymWert.Typ & SearchType)
     {
       if (MakeCrossList AND DoRefs) AddReference(Lauf);
     }
     else
       Lauf = NULL;
   }
   
   return Lauf;
}

        static Boolean FindNode_FSpec(char *Name, PForwardSymbol Root)
BEGIN
   while ((Root!=Nil) AND (strcmp(Root->Name,Name)!=0)) Root=Root->Next;
   return (Root!=Nil);
END

static PSymbolEntry FindNode(char *Name_O, TempType SearchType)
{
  PSaveSection Lauf;
  LongInt DestSection;
  PSymbolEntry Result = NULL;
  String Name;

  strmaxcpy(Name, Name_O, 255);
  ChkTmp3(Name, FALSE);

  if (!GetSymSection(Name, &DestSection))
    return NULL;

  if (!CaseSensitive)
    NLS_UpString(Name);

  if (SectionStack != Nil)
    if (PassNo <= MaxSymPass)
      if (FindNode_FSpec(Name, SectionStack->LocSyms)) DestSection = MomSectionHandle;

  if (DestSection == (-2))
  {
    if ((Result = FindNode_FNode(Name, SearchType, MomSectionHandle)))
      return Result;
    Lauf = SectionStack;
    while (Lauf)
    {
      if ((Result = FindNode_FNode(Name, SearchType, Lauf->Handle))) break;
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

static PSymbolEntry FindLocNode(char *Name_O, TempType SearchType)
{
  PLocHandle RunLocHandle;
  PSymbolEntry Result = NULL;
  String Name;

  strmaxcpy(Name,Name_O, 255);
  ChkTmp3(Name, FALSE);
  if (!CaseSensitive)
    NLS_UpString(Name);

  if (MomLocHandle == (-1))
    return NULL;

  if ((Result = FindLocNode_FNode(Name, SearchType, MomLocHandle)))
    return Result;

  RunLocHandle = FirstLocHandle;
  while ((RunLocHandle) && (RunLocHandle->Cont != -1))
  {
    if ((Result = FindLocNode_FNode(Name, SearchType, RunLocHandle->Cont)) )
      break;
    RunLocHandle = RunLocHandle->Next;
  }

  return Result;
}
/**
        void SetSymbolType(char *Name, Byte NTyp)
BEGIN
   Lauf:PSymbolEntry;
   HRef:Boolean;

   IF NOT ExpandSymbol(Name) THEN Exit;
   HRef:=DoRefs; DoRefs:=False;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf<>Nil THEN Lauf^.SymType:=NTyp;
   DoRefs:=HRef;
END
**/

        Boolean GetIntSymbol(char *Name, LargeInt *Wert, PRelocEntry *Relocs)
BEGIN
   PSymbolEntry Lauf;
   String NName;

   strmaxcpy(NName, Name, 255);
   if (NOT ExpandSymbol(NName)) return False;
   Lauf=FindLocNode(NName, TempInt);
   if (Lauf == Nil) Lauf = FindNode(NName, TempInt);
   if (Lauf != Nil)
    BEGIN
     *Wert = Lauf->SymWert.Contents.IWert;
     if (Relocs) *Relocs = Lauf->Relocs;
     if (Lauf->SymType != 0) TypeFlag |= (1 << Lauf->SymType);
     if ((Lauf->SymSize != (-1)) AND (SizeFlag != (-1))) SizeFlag = Lauf->SymSize;
     Lauf->Used = True;
    END
   else
    BEGIN
     if (PassNo > MaxSymPass) WrXError(1010, Name);
     else FirstPassUnknown = True;
     *Wert = EProgCounter();
    END
   return (Lauf != Nil);
END

        Boolean GetFloatSymbol(char *Name, Double *Wert)
BEGIN
   PSymbolEntry Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;
   Lauf=FindLocNode(Name,TempFloat);
   if (Lauf==Nil) Lauf=FindNode(NName,TempFloat);
   if (Lauf!=Nil)
    BEGIN
     *Wert=Lauf->SymWert.Contents.FWert;
     Lauf->Used=True;
    END
   else
    BEGIN
     if (PassNo>MaxSymPass) WrXError(1010,Name);
     *Wert=0;
    END
   return (Lauf!=Nil);
END

Boolean GetStringSymbol(char *Name, char *Wert)
{
  PSymbolEntry Lauf;
  String NName;

  strmaxcpy(NName, Name, 255);
  if (!ExpandSymbol(NName)) return False;
  Lauf = FindLocNode(NName,TempString);
  if (!Lauf)
    Lauf = FindNode(NName, TempString);
  if (Lauf != Nil)
  {
    memcpy(Wert, Lauf->SymWert.Contents.String.Contents, Lauf->SymWert.Contents.String.Length);
    Wert[Lauf->SymWert.Contents.String.Length] = '\0';
    Lauf->Used = True;
  }
  else
  {
    if (PassNo > MaxSymPass) WrXError(1010, Name);
    *Wert = '\0';
  }
  return (Lauf != Nil);
}

        void SetSymbolSize(char *Name, ShortInt Size)
BEGIN
   PSymbolEntry Lauf;
   Boolean HRef;
   String NName;

   strmaxcpy(NName,Name,255); 
   if (NOT ExpandSymbol(NName)) return;
   HRef=DoRefs; DoRefs=False;
   Lauf=FindLocNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindNode(Name,TempInt);
   if (Lauf!=Nil) Lauf->SymSize=Size;
   DoRefs=HRef;
END

        ShortInt GetSymbolSize(char *Name)
BEGIN
   PSymbolEntry Lauf;
   String NName;
   
   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return -1;
   Lauf=FindLocNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindNode(NName,TempInt);
   return ((Lauf!=Nil) ? Lauf->SymSize : -1);
END

        Boolean IsSymbolFloat(char *Name)
BEGIN
   PSymbolEntry Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf=FindLocNode(NName,TempFloat);
   if (Lauf==Nil) Lauf=FindNode(NName,TempFloat);
   return ((Lauf!=Nil) AND (Lauf->SymWert.Typ==TempFloat));
END

        Boolean IsSymbolString(char *Name)
BEGIN
   PSymbolEntry Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf=FindLocNode(NName,TempString);
   if (Lauf==Nil) Lauf=FindNode(NName,TempString);
   return ((Lauf!=Nil) AND (Lauf->SymWert.Typ==TempString));
END

        Boolean IsSymbolDefined(char *Name)
BEGIN
   PSymbolEntry Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf = FindLocNode(NName, TempAll);
   if (Lauf == Nil) Lauf = FindNode(NName, TempAll);
   return ((Lauf!=Nil) AND (Lauf->Defined));
END

        Boolean IsSymbolUsed(char *Name)
BEGIN
   PSymbolEntry Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf = FindLocNode(NName, TempAll);
   if (Lauf == Nil) Lauf = FindNode(NName, TempAll);
   return ((Lauf != Nil) AND (Lauf->Used));
END

        Boolean IsSymbolChangeable(char *Name)
BEGIN
   PSymbolEntry Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf = FindLocNode(NName, TempAll);
   if (Lauf == Nil) Lauf = FindNode(NName, TempAll);
   return ((Lauf != Nil) AND (Lauf->Changeable));
END

        Integer GetSymbolType(char *Name)
BEGIN
   PSymbolEntry Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return -1;

   Lauf = FindLocNode(Name, TempAll);
   if (Lauf == Nil) Lauf=FindNode(Name, TempAll);
   return (Lauf == Nil) ? -1 : Lauf->SymType;
END

static void ConvertSymbolVal(SymbolVal *Inp, TempResult *Outp)
{
  switch (Outp->Typ = Inp->Typ)
  {
    case TempInt:
      Outp->Contents.Int = Inp->Contents.IWert;
      break;
    case TempFloat:
      Outp->Contents.Float = Inp->Contents.FWert;
      break;
    case TempString:
      Outp->Contents.Ascii.Length = 0;
      DynStringAppend(&Outp->Contents.Ascii, Inp->Contents.String.Contents, Inp->Contents.String.Length);
      break;
    default:
      break;
  }
}

typedef struct
        {
          int Width, cwidth;
          LongInt Sum, USum;
          String Zeilenrest;
        } TListContext;

static void PrintSymbolList_AddOut(char *s, char *Zeilenrest, int Width)
{
   if ((int)(strlen(s) + strlen(Zeilenrest)) > Width)
   {
     Zeilenrest[strlen(Zeilenrest) - 1] = '\0';
     WrLstLine(Zeilenrest); strmaxcpy(Zeilenrest, s, 255);
   }
   else strmaxcat(Zeilenrest, s, 255);
}

static void PrintSymbolList_PNode(PTree Tree, void *pData)
{
   PSymbolEntry Node = (PSymbolEntry) Tree;
   TListContext *pContext = (TListContext*) pData;
   String s1,sh;
   int l1;
   TempResult t;

   ConvertSymbolVal(&(Node->SymWert), &t);
   StrSym(&t, False, s1, sizeof(s1));

   strmaxcpy(sh, Tree->Name, 255);
   if (Tree->Attribute != -1) 
   {
     strmaxcat(sh, " [", 255);
     strmaxcat(sh, GetSectionName(Tree->Attribute), 255);
     strmaxcat(sh, "]", 255);
   }
   strmaxprep(sh, (Node->Used) ? " " : "*", 255);
   l1 = (strlen(s1) + strlen(sh) + 6) % (pContext->cwidth);
   if (l1 < pContext->cwidth - 2) strmaxprep(s1, Blanks(pContext->cwidth - 2 - l1), 255);
   strmaxprep(s1, " : ", 255);
   strmaxprep(s1, sh, 255);
   strmaxcat(s1, " ", 255);
   s1[l1 = strlen(s1)] = SegShorts[Node->SymType]; s1[l1 + 1]='\0';
   strmaxcat(s1, " | ", 255);
   PrintSymbolList_AddOut(s1, pContext->Zeilenrest, pContext->Width);
   pContext->Sum++;
   if (NOT Node->Used) pContext->USum++;
}

void PrintSymbolList(void)
{
   int ActPageWidth;
   TListContext Context;

   Context.Width = (PageWidth == 0) ? 80 : PageWidth;
   NewPage(ChapDepth, True);
   WrLstLine(getmessage(Num_ListSymListHead1));
   WrLstLine(getmessage(Num_ListSymListHead2));
   WrLstLine("");

   Context.Zeilenrest[0] = '\0';
   Context.Sum = Context.USum = 0;
   ActPageWidth = (PageWidth == 0) ? 80 : PageWidth;
   Context.cwidth = ActPageWidth >> 1;
   IterTree((PTree)FirstSymbol, PrintSymbolList_PNode, &Context);
   if (Context.Zeilenrest[0] != '\0')
   {
     Context.Zeilenrest[strlen(Context.Zeilenrest) - 1] = '\0';
     WrLstLine(Context.Zeilenrest);
   }
   WrLstLine("");
   sprintf(Context.Zeilenrest, "%7lu%s",
           (unsigned long)Context.Sum,
           getmessage((Context.Sum == 1) ? Num_ListSymSumMsg : Num_ListSymSumsMsg));
   WrLstLine(Context.Zeilenrest);
   sprintf(Context.Zeilenrest,"%7lu%s",
           (unsigned long)Context.USum,
           getmessage((Context.USum == 1) ? Num_ListUSymSumMsg : Num_ListUSymSumsMsg));
   WrLstLine(Context.Zeilenrest);
   WrLstLine("");
}

typedef struct
        {
          FILE *f;
          Boolean HWritten;
          int Space;
        } TDebContext;

static void PrintDebSymbols_PNode(PTree Tree, void *pData)
{
  PSymbolEntry Node = (PSymbolEntry) Tree;
  TDebContext *DebContext = (TDebContext*) pData;
  int l1;
  TempResult t;
  String s;

  if (Node->SymType != DebContext->Space) 
    return;

  if (!DebContext->HWritten)
  {
    fprintf(DebContext->f, "\n"); ChkIO(10004);
    fprintf(DebContext->f, "Symbols in Segment %s\n", SegNames[DebContext->Space]); ChkIO(10004);
    DebContext->HWritten = True;
  }

  fprintf(DebContext->f, "%s", Node->Tree.Name); ChkIO(10004); l1 = strlen(Node->Tree.Name);
  if (Node->Tree.Attribute != (-1))
  {
    sprintf(s, "[%d]", (int)Node->Tree.Attribute);
    fprintf(DebContext->f, "%s", s); ChkIO(10004);
    l1 += strlen(s);
  }
  fprintf(DebContext->f, "%s ", Blanks(37 - l1)); ChkIO(10004);
  switch (Node->SymWert.Typ)
  {
    case TempInt:    fprintf(DebContext->f,"Int    "); break;
    case TempFloat:  fprintf(DebContext->f,"Float  "); break;
    case TempString: fprintf(DebContext->f,"String "); break;
    default: break;
  }
  ChkIO(10004);
  if (Node->SymWert.Typ == TempString)
  {
    errno = 0;
    l1 = fstrlenprint(DebContext->f, Node->SymWert.Contents.String.Contents, Node->SymWert.Contents.String.Length);
    ChkIO(10004);
  }
  else
  {
    ConvertSymbolVal(&(Node->SymWert),&t);
    StrSym(&t, False, s, sizeof(s));
    l1=strlen(s);
    fprintf(DebContext->f,"%s",s); ChkIO(10004);
  }
  fprintf(DebContext->f,"%s %-3d %d\n",Blanks(25-l1),Node->SymSize,(int)Node->Used);
  ChkIO(10004);
}

void PrintDebSymbols(FILE *f)
{
   TDebContext DebContext;

   DebContext.f = f;
   for (DebContext.Space = 0; DebContext.Space < PCMax; DebContext.Space++)
   {
     DebContext.HWritten = False;
     IterTree((PTree)FirstSymbol, PrintDebSymbols_PNode, &DebContext);
   }
}

typedef struct
        {
          FILE *f;
          LongInt Handle;
        } TNoISymContext;

	static void PrNoISection(PTree Tree, void *pData)
BEGIN
   PSymbolEntry Node = (PSymbolEntry)Tree;
   TNoISymContext *pContext = (TNoISymContext*) pData;

   if (((1 << Node->SymType) & NoICEMask) AND (Node->Tree.Attribute == pContext->Handle) AND (Node->SymWert.Typ == TempInt))
    BEGIN
     errno = 0; fprintf(pContext->f, "DEFINE %s 0x", Node->Tree.Name); ChkIO(10004);
     errno = 0; fprintf(pContext->f, LargeHIntFormat, Node->SymWert.Contents.IWert); ChkIO(10004);
     errno = 0; fprintf(pContext->f, "\n"); ChkIO(10004);
    END
END

	void PrintNoISymbols(FILE *f)
BEGIN
   PCToken CurrSection;
   TNoISymContext Context;
   
   Context.f = f;
   Context.Handle = -1;
   IterTree((PTree)FirstSymbol, PrNoISection, &Context); 
   Context.Handle++;
   for (CurrSection=FirstSection; CurrSection!=Nil; CurrSection=CurrSection->Next)
    if (ChunkSum(&CurrSection->Usage)>0)
     BEGIN
      fprintf(f,"FUNCTION %s ",CurrSection->Name); ChkIO(10004);
      fprintf(f,LargeIntFormat,ChunkMin(&CurrSection->Usage)); ChkIO(10004);
      fprintf(f,"\n"); ChkIO(10004);
      IterTree((PTree)FirstSymbol, PrNoISection, &Context);
      Context.Handle++;
      fprintf(f,"ENDFUNC "); ChkIO(10004);
      fprintf(f,LargeIntFormat,ChunkMax(&CurrSection->Usage)); ChkIO(10004);
      fprintf(f,"\n"); ChkIO(10004);
     END
END

        void PrintSymbolTree(void)
BEGIN
   DumpTree((PTree)FirstSymbol);
END

        static void ClearSymbolList_ClearNode(PTree Node, void *pData)
BEGIN
   PSymbolEntry SymbolEntry = (PSymbolEntry) Node;
   UNUSED(pData);

   FreeSymbolEntry(&SymbolEntry, FALSE);
END

        void ClearSymbolList(void)
BEGIN
   PTree TreeRoot;

   TreeRoot = &(FirstSymbol->Tree); FirstSymbol = NULL;
   DestroyTree(&TreeRoot, ClearSymbolList_ClearNode, NULL);
   TreeRoot = &(FirstLocSymbol->Tree); FirstLocSymbol = NULL;
   DestroyTree(&TreeRoot, ClearSymbolList_ClearNode, NULL);
END

/*-------------------------------------------------------------------------*/
/* Stack-Verwaltung */

        Boolean PushSymbol(char *SymName_O, char *StackName_O)
BEGIN
   PSymbolEntry Src;
   PSymbolStack LStack,NStack,PStack;
   PSymbolStackEntry Elem;
   String SymName,StackName;

   strmaxcpy(SymName,SymName_O,255);
   if (NOT ExpandSymbol(SymName)) return False;

   Src = FindNode(SymName, TempAll);
   if (Src==Nil)
    BEGIN
     WrXError(1010,SymName); return False;
    END

   strmaxcpy(StackName,(*StackName_O=='\0')?DefStackName:StackName_O,255);
   if (NOT ExpandSymbol(StackName)) return False;
   if (NOT ChkSymbName(StackName))
    BEGIN
     WrXError(1020,StackName); return False;
    END

   LStack=FirstStack; PStack=Nil;
   while ((LStack!=Nil) AND (strcmp(LStack->Name,StackName)<0))
    BEGIN
     PStack=LStack;
     LStack=LStack->Next;
    END

   if ((LStack==Nil) OR (strcmp(LStack->Name,StackName)>0))
    BEGIN
     NStack=(PSymbolStack) malloc(sizeof(TSymbolStack));
     NStack->Name=strdup(StackName);
     NStack->Contents=Nil;
     NStack->Next=LStack;
     if (PStack==Nil) FirstStack=NStack; else PStack->Next=NStack;
     LStack=NStack;
    END

   Elem=(PSymbolStackEntry) malloc(sizeof(TSymbolStackEntry));
   Elem->Next=LStack->Contents;
   Elem->Contents=Src->SymWert;
   LStack->Contents=Elem;

   return True;
END

        Boolean PopSymbol(char *SymName_O, char *StackName_O)
BEGIN
   PSymbolEntry Dest;
   PSymbolStack LStack,PStack;
   PSymbolStackEntry Elem;
   String SymName,StackName;

   strmaxcpy(SymName,SymName_O,255);
   if (NOT ExpandSymbol(SymName)) return False;

   Dest = FindNode(SymName, TempAll);
   if (Dest==Nil) 
    BEGIN
     WrXError(1010,SymName); return False;
    END

   strmaxcpy(StackName,(*StackName_O=='\0')?DefStackName:StackName_O,255);
   if (NOT ExpandSymbol(StackName)) return False;
   if (NOT ChkSymbName(StackName))
    BEGIN
     WrXError(1020,StackName); return False;
    END

   LStack=FirstStack; PStack=Nil;
   while ((LStack!=Nil) AND (strcmp(LStack->Name,StackName)<0))
    BEGIN
     PStack=LStack;
     LStack=LStack->Next;
    END

   if ((LStack==Nil) OR (strcmp(LStack->Name,StackName)>0))
    BEGIN
     WrXError(1530,StackName); return False;
    END

   Elem=LStack->Contents;
   Dest->SymWert=Elem->Contents;
   if ((LStack->Contents=Elem->Next)==Nil)
    BEGIN
     if (PStack==Nil) FirstStack=LStack->Next; else PStack->Next=LStack->Next;
     free(LStack->Name);
     free(LStack);
    END
   free(Elem);

   return True;
END

        void ClearStacks(void)
BEGIN
   PSymbolStack Act;
   PSymbolStackEntry Elem;
   int z;
   String s;

   while (FirstStack!=Nil)
    BEGIN
     z=0; Act=FirstStack;
     while (Act->Contents!=Nil)
      BEGIN
       Elem=Act->Contents; Act->Contents=Elem->Next;
       free(Elem); z++;
      END
     sprintf(s,"%s(%d)", Act->Name, z);
     WrXError(230,s);
     free(Act->Name);
     FirstStack=Act->Next; free(Act);
    END
END

/*-------------------------------------------------------------------------*/
/* Funktionsverwaltung */

        void EnterFunction(char *FName, char *FDefinition, Byte NewCnt)
BEGIN
   PFunction Neu;
   String FName_N;

   if (NOT CaseSensitive)
    BEGIN
     strmaxcpy(FName_N,FName,255); NLS_UpString(FName_N); FName=FName_N;
    END

   if (NOT ChkSymbName(FName))
    BEGIN
     WrXError(1020,FName); return;
    END

   if (FindFunction(FName)!=Nil)
    BEGIN
     if (PassNo==1) WrXError(1000,FName); return;
    END

   Neu=(PFunction) malloc(sizeof(TFunction));
   Neu->Next=FirstFunction; Neu->ArguCnt=NewCnt;
   Neu->Name=strdup(FName);
   Neu->Definition=strdup(FDefinition);
   FirstFunction=Neu;
END

        PFunction FindFunction(char *Name)
BEGIN
   PFunction Lauf=FirstFunction;  
   String Name_N;

   if (NOT CaseSensitive)
    BEGIN
     strmaxcpy(Name_N,Name,255); NLS_UpString(Name_N); Name=Name_N;
    END
 
   while ((Lauf!=Nil) AND (strcmp(Lauf->Name,Name)!=0)) Lauf=Lauf->Next;
   return Lauf;
END

        void PrintFunctionList(void)
BEGIN
   PFunction Lauf;
   String OneS;
   Boolean cnt;

   if (FirstFunction==Nil) return;

   NewPage(ChapDepth,True);
   WrLstLine(getmessage(Num_ListFuncListHead1));
   WrLstLine(getmessage(Num_ListFuncListHead2));
   WrLstLine("");

   OneS[0]='\0'; Lauf=FirstFunction; cnt=False;
   while (Lauf!=Nil)
    BEGIN
     strmaxcat(OneS,Lauf->Name,255);
     if (strlen(Lauf->Name)<37) strmaxcat(OneS,Blanks(37-strlen(Lauf->Name)),255);
     if (NOT cnt) strmaxcat(OneS," | ",255);
     else
      BEGIN
       WrLstLine(OneS); OneS[0]='\0';
      END
     cnt=NOT cnt;
     Lauf=Lauf->Next;
    END
   if (cnt)
    BEGIN
     OneS[strlen(OneS)-1]='\0';
     WrLstLine(OneS);
    END
   WrLstLine("");
END

        void ClearFunctionList(void)
BEGIN
   PFunction Lauf;

   while (FirstFunction!=Nil)
    BEGIN
     Lauf=FirstFunction->Next;
     free(FirstFunction->Name);
     free(FirstFunction->Definition);
     free(FirstFunction);
     FirstFunction=Lauf;
    END
END

/*-------------------------------------------------------------------------*/

static void ResetSymbolDefines_ResetNode(PTree Node, void *pData)
{
   PSymbolEntry SymbolEntry = (PSymbolEntry) Node;
   UNUSED(pData);

   SymbolEntry->Defined=False;
   SymbolEntry->Used=False;
}

void ResetSymbolDefines(void)
{
   IterTree(&(FirstSymbol->Tree), ResetSymbolDefines_ResetNode, NULL);
   IterTree(&(FirstLocSymbol->Tree), ResetSymbolDefines_ResetNode, NULL);
}

        void SetFlag(Boolean *Flag, char *Name, Boolean Wert)
BEGIN
   *Flag=Wert; EnterIntSymbol(Name,(*Flag)?1:0,0,True);
END

        void AddDefSymbol(char *Name, TempResult *Value)
BEGIN
   PDefSymbol Neu;

   Neu=FirstDefSymbol;
   while (Neu!=Nil)
    BEGIN
     if (strcmp(Neu->SymName,Name)==0) return;
     Neu=Neu->Next;
    END

   Neu=(PDefSymbol) malloc(sizeof(TDefSymbol));
   Neu->Next=FirstDefSymbol;
   Neu->SymName=strdup(Name);
   Neu->Wert=(*Value);
   FirstDefSymbol=Neu;
END

        void RemoveDefSymbol(char *Name)
BEGIN
  PDefSymbol Save,Lauf;
 
  if (FirstDefSymbol==Nil) return;

  if (strcmp(FirstDefSymbol->SymName,Name)==0)
   BEGIN
    Save=FirstDefSymbol; FirstDefSymbol=FirstDefSymbol->Next;
   END
  else
   BEGIN
    Lauf=FirstDefSymbol;
    while ((Lauf->Next!=Nil) AND (strcmp(Lauf->Next->SymName,Name)!=0)) Lauf=Lauf->Next;
    if (Lauf->Next==Nil) return;
    Save=Lauf->Next; Lauf->Next=Lauf->Next->Next;
   END
  free(Save->SymName); free(Save);
END

void CopyDefSymbols(void)
{
  PDefSymbol Lauf;

  Lauf = FirstDefSymbol;
  while (Lauf != Nil)
  {
    switch (Lauf->Wert.Typ)
    {
      case TempInt:
        EnterIntSymbol(Lauf->SymName, Lauf->Wert.Contents.Int, 0, True);
        break;
      case TempFloat:
        EnterFloatSymbol(Lauf->SymName, Lauf->Wert.Contents.Float, True);
        break;
      case TempString:
        EnterDynStringSymbol(Lauf->SymName, &Lauf->Wert.Contents.Ascii, True);
        break;
      default:
        break;
    }
    Lauf = Lauf->Next;
  }
}

void PrintSymbolDepth(void)
{
   LongInt TreeMin, TreeMax;

   GetTreeDepth(&(FirstSymbol->Tree), &TreeMin, &TreeMax);
   fprintf(Debug," MinTree %ld\n", (long)TreeMin);
   fprintf(Debug," MaxTree %ld\n", (long)TreeMax);
}

        LongInt GetSectionHandle(char *SName_O, Boolean AddEmpt, LongInt Parent)
BEGIN
   PCToken Lauf,Prev;
   LongInt z;
   String SName;

   strmaxcpy(SName,SName_O,255); if (NOT CaseSensitive) NLS_UpString(SName);

   Lauf=FirstSection; Prev=Nil; z=0;
   while ((Lauf!=Nil) AND ((strcmp(Lauf->Name,SName)!=0) OR (Lauf->Parent!=Parent)))
    BEGIN
     z++; Prev=Lauf; Lauf=Lauf->Next;
    END

   if (Lauf == Nil)
    BEGIN
     if (AddEmpt)
      BEGIN
       Lauf = (PCToken) malloc(sizeof(TCToken));
       Lauf->Parent = MomSectionHandle;
       Lauf->Name = strdup(SName);
       Lauf->Next = Nil;
       InitChunk(&(Lauf->Usage));
       if (Prev == Nil)
        FirstSection = Lauf;
       else
        Prev->Next = Lauf;
      END
     else z = (-2);
    END
   return z;
END

        char *GetSectionName(LongInt Handle)
BEGIN
   PCToken Lauf=FirstSection;
   static char *Dummy="";

   if (Handle==(-1)) return Dummy;
   while ((Handle>0) AND (Lauf!=Nil))
    BEGIN
     Lauf=Lauf->Next; Handle--;
    END
   return (Lauf==Nil)?Dummy:Lauf->Name;
END

        void SetMomSection(LongInt Handle)
BEGIN
   LongInt z;

   MomSectionHandle=Handle;
   if (Handle<0) MomSection=Nil;
   else
    BEGIN
     MomSection=FirstSection;
     for (z=1; z<=Handle; z++)
      if (MomSection!=Nil) MomSection=MomSection->Next;
    END
END

        void AddSectionUsage(LongInt Start,LongInt Length)
BEGIN
   if ((ActPC!=SegCode) OR (MomSection==Nil)) return;
   AddChunk(&(MomSection->Usage),Start,Length,False);
END

        void ClearSectionUsage(void)
{
  PCToken Tmp;

  for (Tmp = FirstSection; Tmp; Tmp = Tmp->Next)
    ClearChunk(&(Tmp->Usage));
}

        static void PrintSectionList_PSection(LongInt Handle, int Indent)
BEGIN
   PCToken Lauf;
   LongInt Cnt;
   String h;

   ChkStack();
   if (Handle!=(-1)) 
    BEGIN
     strmaxcpy(h,Blanks(Indent<<1),255); 
     strmaxcat(h,GetSectionName(Handle),255);
     WrLstLine(h);
    END
   Lauf=FirstSection; Cnt=0;
   while (Lauf!=Nil)
    BEGIN
     if (Lauf->Parent==Handle) PrintSectionList_PSection(Cnt,Indent+1);
     Lauf=Lauf->Next; Cnt++;
    END
END

        void PrintSectionList(void)
BEGIN
   if (FirstSection==Nil) return;

   NewPage(ChapDepth,True);
   WrLstLine(getmessage(Num_ListSectionListHead1));
   WrLstLine(getmessage(Num_ListSectionListHead2));
   WrLstLine("");
   PrintSectionList_PSection(-1,0);
END

        void PrintDebSections(FILE *f)
BEGIN
   PCToken Lauf;
   LongInt Cnt,z,l,s;

   Lauf=FirstSection; Cnt=0;
   while (Lauf!=Nil)
    BEGIN
     fputs("\nInfo for Section ", f); ChkIO(10004);
     fprintf(f, LongIntFormat, Cnt); ChkIO(10004);
     fputc(' ', f); ChkIO(10004);
     fputs(GetSectionName(Cnt), f); ChkIO(10004);
     fputc(' ', f); ChkIO(10004);
     fprintf(f, LongIntFormat, Lauf->Parent); ChkIO(10004);
     fputc('\n', f); ChkIO(10004);
     for (z=0; z<Lauf->Usage.RealLen; z++)
      BEGIN
       l=Lauf->Usage.Chunks[z].Length;
       s=Lauf->Usage.Chunks[z].Start;
       fprintf(f,"%s",HexString(s,0)); ChkIO(10004);
       if (l==1) fprintf(f,"\n"); else fprintf(f,"-%s\n",HexString(s+l-1,0)); ChkIO(10004);
      END
     Lauf=Lauf->Next;
     Cnt++;
    END
END

        void ClearSectionList(void)
BEGIN
   PCToken Tmp;

   while (FirstSection!=Nil)
    BEGIN
     Tmp=FirstSection;
     free(Tmp->Name);
     ClearChunk(&(Tmp->Usage));
     FirstSection=Tmp->Next; free(Tmp);
    END
END

/*---------------------------------------------------------------------------------*/

static void PrintCrossList_PNode(PTree Node, void *pData)
{
   int FileZ;
   PCrossRef Lauf;
   String LinePart,LineAcc;
   String h,h2;
   TempResult t;
   PSymbolEntry SymbolEntry = (PSymbolEntry) Node;
   UNUSED(pData);

   if (SymbolEntry->RefList==Nil) return;

   ConvertSymbolVal(&(SymbolEntry->SymWert),&t);
   strcpy(h," (=");
   StrSym(&t, False, h2, sizeof(h2));
   strmaxcat(h,h2,255);
   strmaxcat(h,",",255);
   strmaxcat(h,GetFileName(SymbolEntry->FileNum),255);
   strmaxcat(h,":",255);
   sprintf(h2, LongIntFormat, SymbolEntry->LineNum); strmaxcat(h,h2,255);
   strmaxcat(h,"):",255);
   if (Node->Attribute!=(-1))
   {
     strmaxprep(h,"] ",255);
     strmaxprep(h,GetSectionName(Node->Attribute),255);
     strmaxprep(h," [",255);
   }

   strmaxprep(h,Node->Name,255);
   strmaxprep(h,getmessage(Num_ListCrossSymName),255);
   WrLstLine(h);

   for (FileZ=0; FileZ<GetFileCount(); FileZ++)
   {
     Lauf=SymbolEntry->RefList;

     while ((Lauf!=Nil) AND (Lauf->FileNum!=FileZ)) Lauf=Lauf->Next;

     if (Lauf!=Nil)
     {
       strcpy(h," ");
       strmaxcat(h,getmessage(Num_ListCrossFileName),255);
       strmaxcat(h,GetFileName(FileZ),255);
       strmaxcat(h," :",255);
       WrLstLine(h);
       strcpy(LineAcc,"   ");
       while (Lauf!=Nil)
       {
         sprintf(LinePart,"%5ld", (long)Lauf->LineNum);
         strmaxcat(LineAcc,LinePart,255);
         if (Lauf->OccNum!=1)
         {
           sprintf(LinePart,"(%2ld)", (long)Lauf->OccNum);
           strmaxcat(LineAcc,LinePart,255);
         }
         else strmaxcat(LineAcc,"    ",255);
         if (strlen(LineAcc)>=72)
         {
           WrLstLine(LineAcc); strcpy(LineAcc,"  ");
         }
         Lauf=Lauf->Next;
       }
       if (strcmp(LineAcc,"  ")!=0) WrLstLine(LineAcc);
     }
   }
   WrLstLine("");
}

void PrintCrossList(void)
{
   WrLstLine("");
   WrLstLine(getmessage(Num_ListCrossListHead1));
   WrLstLine(getmessage(Num_ListCrossListHead2));
   WrLstLine("");
   IterTree(&(FirstSymbol->Tree), PrintCrossList_PNode, NULL);
   WrLstLine("");
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
END

void ClearCrossList(void)
{
   IterTree(&(FirstSymbol->Tree), ClearCrossList_CNode, NULL);
}

/*--------------------------------------------------------------------------*/

        LongInt GetLocHandle(void)
BEGIN
   return LocHandleCnt++;
END

        void PushLocHandle(LongInt NewLoc)
BEGIN
   PLocHandle NewLocHandle;

   NewLocHandle=(PLocHandle) malloc(sizeof(TLocHeap));
   NewLocHandle->Cont=MomLocHandle;
   NewLocHandle->Next=FirstLocHandle;
   FirstLocHandle=NewLocHandle; MomLocHandle=NewLoc;
END

        void PopLocHandle(void)
BEGIN
   PLocHandle OldLocHandle;

   OldLocHandle=FirstLocHandle;
   if (OldLocHandle==Nil) return;
   MomLocHandle=OldLocHandle->Cont; 
   FirstLocHandle=OldLocHandle->Next;
   free(OldLocHandle);
END

        void ClearLocStack()
BEGIN
   while (MomLocHandle!=(-1)) PopLocHandle();
END

/*--------------------------------------------------------------------------*/

	static PRegDef LookupReg(char *Name, Boolean CreateNew)
BEGIN
   PRegDef Run,Neu,Prev;
   int cmperg=0;

   Prev=Nil; Run=FirstRegDef;
   while ((Run!=Nil) AND ((cmperg=strcmp(Run->Orig,Name))!=0))
    BEGIN
     Prev=Run; Run=(cmperg<0) ? Run->Left : Run->Right;
    END
   if ((Run==Nil) AND (CreateNew))
    BEGIN
     Neu=(PRegDef) malloc(sizeof(TRegDef));
     Neu->Orig=strdup(Name);
     Neu->Left=Neu->Right=Nil;
     Neu->Defs=Nil;
     Neu->DoneDefs=Nil;
     if (Prev==Nil) FirstRegDef=Neu;
     else if (cmperg<0) Prev->Left=Neu; else Prev->Right=Neu;
     return Neu;
    END
   else return Run;
END

        void AddRegDef(char *Orig_N, char *Repl_N)
BEGIN
   PRegDef Node;
   PRegDefList Neu;
   String Orig,Repl;

   strmaxcpy(Orig,Orig_N,255); strmaxcpy(Repl,Repl_N,255);
   if (NOT CaseSensitive)
    BEGIN
     NLS_UpString(Orig); NLS_UpString(Repl);
    END
   if (NOT ChkSymbName(Orig))
    BEGIN
     WrXError(1020,Orig); return;
    END
   if (NOT ChkSymbName(Repl))
    BEGIN
     WrXError(1020,Repl); return;
    END
   Node=LookupReg(Orig,True);
   if ((Node->Defs!=Nil) AND (Node->Defs->Section==MomSectionHandle))
    WrXError(1000,Orig);
   else
    BEGIN
     Neu=(PRegDefList) malloc(sizeof(TRegDefList));
     Neu->Next=Node->Defs; Neu->Section=MomSectionHandle;
     Neu->Value=strdup(Repl);
     Neu->Used=False;
     Node->Defs=Neu;
    END
END

        Boolean FindRegDef(const char *Name_N, char **Erg)
BEGIN
   LongInt Sect;
   PRegDef Node;
   PRegDefList Def;
   String Name;
   
   if (*Name_N=='[') return FALSE;

   strmaxcpy(Name,Name_N,255);

   if (NOT GetSymSection(Name,&Sect)) return False;
   if (NOT CaseSensitive) NLS_UpString(Name);
   Node=LookupReg(Name,False);
   if (Node==Nil) return False;
   Def=Node->Defs;
   if (Sect!=-2)
    while ((Def!=Nil) AND (Def->Section!=Sect)) Def=Def->Next;
   if (Def==Nil) return False;
   else
    BEGIN
     *Erg=Def->Value; Def->Used=True; return True;
    END
END

        static void TossRegDefs_TossSingle(PRegDef Node, LongInt Sect)
BEGIN
   PRegDefList Tmp;

   if (Node==Nil) return; ChkStack();

   if ((Node->Defs!=Nil) AND (Node->Defs->Section==Sect))
    BEGIN
     Tmp=Node->Defs; Node->Defs=Node->Defs->Next;
     Tmp->Next=Node->DoneDefs; Node->DoneDefs=Tmp;
    END

   TossRegDefs_TossSingle(Node->Left,Sect);
   TossRegDefs_TossSingle(Node->Right,Sect);
END

        void TossRegDefs(LongInt Sect)
BEGIN
   TossRegDefs_TossSingle(FirstRegDef,Sect);
END

        static void ClearRegDefList(PRegDefList Start)
BEGIN
   PRegDefList Tmp;

   while (Start!=Nil)
    BEGIN
     Tmp=Start; Start=Start->Next;
     free(Tmp->Value);
     free(Tmp);
    END
END

        static void CleanupRegDefs_CleanupNode(PRegDef Node)
BEGIN
   if (Node==Nil) return; ChkStack();
   ClearRegDefList(Node->DoneDefs); Node->DoneDefs=Nil;
   CleanupRegDefs_CleanupNode(Node->Left);
   CleanupRegDefs_CleanupNode(Node->Right);
END

        void CleanupRegDefs(void)
BEGIN
   CleanupRegDefs_CleanupNode(FirstRegDef);
END

        static void ClearRegDefs_ClearNode(PRegDef Node)
BEGIN
   if (Node==Nil) return; ChkStack();
   ClearRegDefList(Node->Defs); Node->Defs=Nil;
   ClearRegDefList(Node->DoneDefs); Node->DoneDefs=Nil;
   ClearRegDefs_ClearNode(Node->Left); ClearRegDefs_ClearNode(Node->Right);
   free(Node->Orig);
   free(Node);
END

        void ClearRegDefs(void)
BEGIN
   ClearRegDefs_ClearNode(FirstRegDef);
END

static int cwidth;

        static void PrintRegDefs_PNode(PRegDef Node, char *buf, LongInt *Sum, LongInt *USum)
BEGIN
   PRegDefList Lauf;
   String tmp,tmp2;

   for (Lauf=Node->DoneDefs; Lauf!=Nil; Lauf=Lauf->Next)
    BEGIN
     if (Lauf->Section!=-1)
      sprintf(tmp2,"[%s]",GetSectionName(Lauf->Section));
     else
      *tmp2='\0';
     sprintf(tmp,"%c%s%s --> %s",(Lauf->Used) ? ' ' : '*',Node->Orig,tmp2,Lauf->Value);
     if ((int)strlen(tmp)>cwidth-3)
      BEGIN
       if (*buf!='\0') WrLstLine(buf); *buf='\0'; WrLstLine(tmp);
      END
     else
      BEGIN
       strmaxcat(tmp,Blanks(cwidth-3-strlen(tmp)),255);
       if (*buf=='\0') strcpy(buf,tmp);
       else
        BEGIN
         strcat(buf," | "); strcat(buf,tmp);
         WrLstLine(buf); *buf='\0';
        END
      END
     (*Sum)++; if (NOT Lauf->Used) (*USum)++;
    END
END

        static void PrintRegDefs_PrintSingle(PRegDef Node, char *buf, LongInt *Sum, LongInt *USum)
BEGIN
   if (Node==Nil) return; ChkStack();

   PrintRegDefs_PrintSingle(Node->Left,buf,Sum,USum);
   PrintRegDefs_PNode(Node,buf,Sum,USum);
   PrintRegDefs_PrintSingle(Node->Right,buf,Sum,USum);
END

        void PrintRegDefs(void)
BEGIN
   String buf;
   LongInt Sum,USum;
   LongInt ActPageWidth;

   if (FirstRegDef==Nil) return;

   NewPage(ChapDepth,True);
   WrLstLine(getmessage(Num_ListRegDefListHead1));
   WrLstLine(getmessage(Num_ListRegDefListHead2));
   WrLstLine("");

   *buf='\0'; Sum=0; USum=0;
   ActPageWidth=(PageWidth==0) ? 80 : PageWidth;
   cwidth=ActPageWidth>>1;
   PrintRegDefs_PrintSingle(FirstRegDef,buf,&Sum,&USum);

   if (*buf!='\0') WrLstLine(buf);
   WrLstLine("");
   sprintf(buf,"%7ld%s",
           (long) Sum,
           getmessage((Sum==1)?Num_ListRegDefSumMsg:Num_ListRegDefSumsMsg));
   WrLstLine(buf);
   sprintf(buf,"%7ld%s",
           (long)USum,
           getmessage((USum==1)?Num_ListRegDefUSumMsg:Num_ListRegDefUSumsMsg));
   WrLstLine("");
END

/*--------------------------------------------------------------------------*/

	void ClearCodepages(void)
BEGIN
   PTransTable Old;

   while (TransTables!=Nil)
    BEGIN
     Old=TransTables; TransTables=Old->Next;
     free(Old->Name); free(Old->Table); free(Old);
    END
END

        void PrintCodepages(void)
BEGIN
   char buf[500];
   PTransTable Table;
   int z,cnt,cnt2;

   NewPage(ChapDepth,True);
   WrLstLine(getmessage(Num_ListCodepageListHead1));
   WrLstLine(getmessage(Num_ListCodepageListHead2));
   WrLstLine("");

   cnt2=0;
   for (Table=TransTables; Table!=Nil; Table=Table->Next)
    BEGIN
     for (z=cnt=0; z<256; z++)
      if (Table->Table[z]!=z) cnt++;
     sprintf(buf,"%s (%d%s)",Table->Name,cnt,
             getmessage((cnt==1) ? Num_ListCodepageChange : Num_ListCodepagePChange));
     WrLstLine(buf);
     cnt2++;
    END
   WrLstLine("");
   sprintf(buf,"%d%s",cnt2,
           getmessage((cnt2==1) ? Num_ListCodepageSumMsg : Num_ListCodepageSumsMsg));
END

/*--------------------------------------------------------------------------*/

	void asmpars_init(void)
BEGIN
   serr = (char*)malloc(sizeof(char) * STRINGSIZE);
   snum = (char*)malloc(sizeof(char) * STRINGSIZE);
   FirstDefSymbol=Nil;
   FirstFunction=Nil;
   BalanceTree=False;
   IntMins[(int)Int32]--;
   IntMins[(int)SInt32]--;
#ifdef HAS64
   IntMins[(int)Int64]--;
#endif
   LastGlobSymbol = (char*)malloc(sizeof(char) * 256);
END

