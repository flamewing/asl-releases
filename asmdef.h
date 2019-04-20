#ifndef _ASMDEF_H
#define _ASMDEF_H
/* asmdef.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* global benutzte Variablen und Definitionen                                */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>

#include "chunks.h"
#include "fileformat.h"
#include "dynstring.h"
#include "strcomp.h"
#include "lstmacroexp.h"
#include "cpulist.h"
#include "tempresult.h"

typedef Word tSymbolFlags;
enum
{
  NextLabelFlag_AfterBSR = 1 << 0
};

struct sRelocEntry;

typedef struct
{
  TempType Typ;
  struct sRelocEntry *Relocs;
  union
  {
    LargeInt IWert;
    Double FWert;
    struct
    {
      char *Contents;
      unsigned Length;
    } String;
  } Contents;
} SymbolVal;

typedef struct _TCrossRef
{
  struct _TCrossRef *Next;
  Byte FileNum;
  LongInt LineNum;
  Integer OccNum;
} TCrossRef,*PCrossRef;


typedef struct _TPatchEntry
{
  struct _TPatchEntry *Next;
  LargeWord Address;
  char *Ref;
  Word len;
  LongWord RelocType;
} TPatchEntry, *PPatchEntry;

typedef struct _TExportEntry
{
  struct _TExportEntry *Next;
  char *Name;
  Word len;
  LongWord Flags;
  LargeWord Value;
} TExportEntry, *PExportEntry;

typedef enum
{
  DebugNone,
  DebugMAP,
  DebugAOUT,
  DebugCOFF,
  DebugELF,
  DebugAtmel,
  DebugNoICE
} DebugType;

#define Char_NUL 0
#define Char_BEL '\a'
#define Char_BS '\b'
#define Char_HT 9
#define Char_LF '\n'
#define Char_FF 12
#define Char_CR 13
#define Char_ESC 27

#define ListMask_FormFeed         (1 << 0)
#define ListMask_SymbolList       (1 << 1)
#define ListMask_MacroList        (1 << 2)
#define ListMask_FunctionList     (1 << 3)
#define ListMask_LineNums         (1 << 4)
#define ListMask_DefineList       (1 << 5)
#define ListMask_RegDefList       (1 << 6)
#define ListMask_Codepages        (1 << 7)
#define ListMask_StructList       (1 << 8)

#ifdef HAS64
#define MaxLargeInt 0x7fffffffffffffffll
#else
#define MaxLargeInt 0x7fffffffl
#endif

extern char SrcSuffix[],IncSuffix[],PrgSuffix[],LstSuffix[],
            MacSuffix[],PreSuffix[],LogSuffix[],MapSuffix[],
            OBJSuffix[];

#define MomCPUName       "MOMCPU"     /* mom. Prozessortyp */
#define MomCPUIdentName  "MOMCPUNAME" /* mom. Prozessortyp */
#define SupAllowedName   "INSUPMODE"  /* privilegierte Befehle erlaubt */
#define DoPaddingName    "PADDING"    /* Padding an */
#define PackingName      "PACKING"    /* gepackte Ablage an */
#define MaximumName      "INMAXMODE"  /* CPU im Maximum-Modus */
#define FPUAvailName     "HASFPU"     /* FPU-Befehle erlaubt */
#define LstMacroExpName  "MACEXP"     /* expandierte Makros anzeigen */
#define ListOnName       "LISTON"     /* Listing an/aus */
#define RelaxedName      "RELAXED"    /* alle Zahlenschreibweisen zugelassen */
#define SrcModeName      "INSRCMODE"  /* CPU im Quellcode-kompatiblen Modus */
#define BigEndianName    "BIGENDIAN"  /* Datenablage MSB first */
#define BranchExtName    "BRANCHEXT"  /* Spruenge autom. verlaengern */
#define FlagTrueName     "TRUE"	      /* Flagkonstanten */
#define FlagFalseName    "FALSE"
#define PiName           "CONSTPI"    /* Zahl Pi */
#define DateName         "DATE"       /* Datum & Zeit */
#define TimeName         "TIME"
#define VerName          "VERSION"    /* speichert Versionsnummer */
#define CaseSensName     "CASESENSITIVE" /* zeigt Gross/Kleinunterscheidung an */
#define Has64Name        "HAS64"         /* arbeitet Parser mit 64-Bit-Integers ? */
#define ArchName         "ARCHITECTURE"  /* Zielarchitektur von AS */
#define AttrName         "ATTRIBUTE"  /* Attributansprache in Makros */
#define LabelName        "__LABEL__"  /* Labelansprache in Makros */
#define ArgCName         "ARGCOUNT"   /* Argumentzahlansprache in Makros */
#define AllArgName       "ALLARGS"    /* Ansprache Argumentliste in Makros */
#define DefStackName     "DEFSTACK"   /* Default-Stack */
#define NestMaxName      "NESTMAX"    /* max. nesting level of a macro */
#define DottedStructsName "DOTTEDSTRUCTS" /* struct elements by default with . */

extern char *EnvName;

/* This results from the tokenized representation of macro arguments
   in macro bodys: (31*16) - 4 for special arguments: */

#define ArgCntMax 476

#define ChapMax 4

#define StructSeg (PCMax + 1)

extern char *SegNames[PCMax + 2];
extern char SegShorts[PCMax + 2];

#define AscOfs '0'

#define MaxCodeLen_Ini 256
#define MaxCodeLen_Max 65535
extern LongWord MaxCodeLen;

#define DEF_NESTMAX 256

typedef void (*SimpProc)(
#ifdef __PROTOS__
void
#endif
);

typedef void (*DissectBitProc)(
#ifdef __PROTOS__
char *pDest, int DestSize, LargeWord Inp
#endif
);


typedef Word WordField[6];          /* fuer Zahlenumwandlung */
typedef enum
{
  ConstModeIntel,     /* Hex xxxxh, Oct xxxxo, Bin xxxxb */
  ConstModeMoto,      /* Hex $xxxx, Oct @xxxx, Bin %xxxx */
  ConstModeC,         /* Hex 0x..., Oct 0...., Bin ----- */
  ConstModeWeird      /* Hex [xh]'xxxx', Oct o'xxxx', Bin b'xxxx' */
} TConstMode;

typedef struct _TFunction
{
  struct _TFunction *Next;
  Byte ArguCnt;
  StringPtr Name, Definition;
} TFunction, *PFunction;

typedef struct _TTransTable
{
  struct _TTransTable *Next;
  char *Name;
  unsigned char *Table;
} TTransTable, *PTransTable;

typedef struct _TSaveState
{
  struct _TSaveState *Next;
  CPUVar SaveCPU;
  Integer SavePC;
  Byte SaveListOn;
  tLstMacroExp SaveLstMacroExp;
  tLstMacroExpMod SaveLstMacroExpOverride;
  PTransTable SaveTransTable;
  Integer SaveEnumSegment;
  LongInt SaveEnumCurrentValue, SaveEnumIncrement;
} TSaveState,*PSaveState;

typedef struct _TForwardSymbol
{
  struct _TForwardSymbol *Next;
  StringPtr Name;
  LongInt DestSection;
  StringPtr pErrorPos;
} TForwardSymbol, *PForwardSymbol;

typedef struct _TSaveSection
{
  struct _TSaveSection *Next;
  PForwardSymbol LocSyms, GlobSyms, ExportSyms;
  LongInt Handle;
} TSaveSection, *PSaveSection;

typedef struct sSavePhase
{
  struct sSavePhase *pNext;
  LargeWord SaveValue;
} tSavePhase;

typedef struct _TDefinement
{
  struct _TDefinement *Next;
  StringPtr TransFrom, TransTo;
  Byte Compiled[256];
} TDefinement, *PDefinement;

typedef struct _ASSUMERec
{
  char *Name;
  LongInt *Dest;
  LongInt Min,Max;
  LongInt NothingVal;
  void (*pPostProc)(void);
} ASSUMERec;

extern StringPtr SourceFile;

extern StringPtr ClrEol;
extern StringPtr CursUp;

extern LargeWord *PCs;
extern Boolean RelSegs;
extern LargeWord StartAdr;
extern LargeWord AfterBSRAddr;
extern Boolean StartAdrPresent;
extern LargeWord *Phases;
extern Word Grans[StructSeg+1];
extern Word ListGrans[StructSeg+1];
extern ChunkList SegChunks[StructSeg+1];
extern Integer ActPC;
extern Boolean PCsUsed[StructSeg+1];
extern LargeWord *SegInits;
extern LargeWord *SegLimits;
extern LongInt ValidSegs;
extern Boolean ENDOccured;
extern Boolean Retracted;
extern Boolean ListToStdout,ListToNull;

extern unsigned ASSUMERecCnt;
extern const ASSUMERec *pASSUMERecs;
extern void (*pASSUMEOverride)(void);

extern Word TypeFlag;
extern ShortInt SizeFlag;

extern Integer PassNo;
extern Integer JmpErrors;
extern Boolean ThrowErrors;
extern Boolean Repass;
extern Byte MaxSymPass;
extern Byte ShareMode;
extern DebugType DebugMode;
extern Word NoICEMask;
extern Byte ListMode;
extern Byte ListOn;
extern Boolean MakeUseList;
extern Boolean MakeCrossList;
extern Boolean MakeSectionList;
extern Boolean MakeIncludeList;
extern Boolean RelaxedMode;
extern Word ListMask;
extern ShortInt ExtendErrors;
extern Integer EnumSegment;
extern LongInt EnumIncrement, EnumCurrentValue;
extern LongWord MaxErrors;
extern Boolean TreatWarningsAsErrors;

extern LongInt MomSectionHandle;
extern PSaveSection SectionStack;
extern tSavePhase *pPhaseStacks[PCMax];

extern LongInt CodeLen;
extern Byte *BAsmCode;
extern Word *WAsmCode;
extern LongWord *DAsmCode;

extern Boolean DontPrint;
extern Word ActListGran;

extern Boolean NumericErrors;
extern Boolean CodeOutput;
extern Boolean MacProOutput;
extern Boolean MacroOutput;
extern Boolean QuietMode;
extern Boolean HardRanges;
extern char *DivideChars;
extern Boolean HasAttrs;
extern char *AttrChars;
extern Boolean MsgIfRepass;
extern Integer PassNoForMessage;
extern Boolean CaseSensitive;
extern LongInt NestMax;
extern Boolean GNUErrors;

extern FILE *PrgFile;

extern StringPtr ErrorPath,ErrorName;
extern StringPtr OutName;
extern StringPtr CurrFileName;
extern LongInt CurrLine;
extern LongInt MomLineCounter;
extern LongInt LineSum;
extern LongInt MacLineSum;

extern LongInt NOPCode;
extern Boolean TurnWords;
extern Byte HeaderID;
extern char *PCSymbol;
extern TConstMode ConstMode;
extern Boolean SetIsOccupied, SwitchIsOccupied, PageIsOccupied;
extern void (*MakeCode)(void);
extern Boolean (*ChkPC)(LargeWord Addr);
extern Boolean (*IsDef)(void);
extern void (*SwitchFrom)(void);
extern void (*InternSymbol)(char *Asc, TempResult *Erg);
extern DissectBitProc DissectBit;

extern StringPtr IncludeList;
extern Integer IncDepth,NextIncDepth;
extern FILE *ErrorFile;
extern FILE *LstFile;
extern FILE *ShareFile;
extern FILE *MacProFile;
extern FILE *MacroFile;
extern Boolean InMacroFlag;
extern StringPtr LstName, MacroName, MacProName;
extern tLstMacroExp DoLst, NextDoLst;
extern StringPtr ShareName;
extern CPUVar MomCPU,MomVirtCPU;
extern char DefCPU[20];
extern char MomCPUIdent[20];

extern Boolean FPUAvail;
extern Boolean DoPadding;
extern Boolean Packing;
extern Boolean SupAllowed;
extern Boolean Maximum;
extern Boolean DoBranchExt;

extern LargeWord RadixBase, OutRadixBase;

extern tStrComp *ArgStr;
extern StringPtr LOpPart;
extern tStrComp LabPart, CommPart, ArgPart, OpPart, AttrPart;
extern char AttrSplit;
extern int ArgCnt;
extern StringPtr OneLine;
#ifdef PROFILE_MEMO
extern unsigned NumMemo;
extern unsigned long NumMemoSum, NumMemoCnt;
#endif

#define forallargs(pArg, cond) \
        for (pArg = ArgStr + 1; (cond) && (pArg <= ArgStr + ArgCnt); pArg++)

extern Byte LstCounter;
extern Word PageCounter[ChapMax+1];
extern Byte ChapDepth;
extern StringPtr ListLine;
extern Byte PageLength, PageWidth;
extern tLstMacroExp LstMacroExp;
extern tLstMacroExpMod LstMacroExpOverride;
extern Boolean DottedStructs;
extern StringPtr PrtInitString;
extern StringPtr PrtExitString;
extern StringPtr PrtTitleString;

extern Byte StopfZahl;

extern Boolean SuppWarns;

#define CharTransTable CurrTransTable->Table
extern PTransTable TransTables, CurrTransTable;

extern PFunction FirstFunction;

extern PDefinement FirstDefine;

extern PSaveState FirstSaveState;

extern Boolean MakeDebug;
extern FILE *Debug;


extern void AsmDefInit(void);

extern void NullProc(void);

extern int SetMaxCodeLen(LongWord NewMaxCodeLen);

extern void Default_InternSymbol(char *Asc, TempResult *Erg);

extern void Default_DissectBit(char *pDest, int DestSize, LargeWord BitSpec);

extern void IncArgCnt(void);


extern void asmdef_init(void);
#endif /* _ASMDEF_H */
