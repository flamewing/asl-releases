/* asmdef.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* global benutzte Variablen und Definitionen                                */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           24. 6.1998 Zeichenübersetzungstabellen                          */
/*           24. 7.1998 Debug-Modus NoICE                                    */
/*           25. 7.1998 PassNo --> Integer                                   */
/*           17. 8.1998 InMacroFlag hierher verschoben                       */
/*           18. 8.1998 RadixBase hinzugenommen                              */
/*                      ArgStr-Feld war eins zu kurz                         */
/*           19. 8.1998 BranchExt-Variablen                                  */
/*           29. 8.1998 ActListGran hinzugenommen                            */
/*            1. 1.1999 SegLimits dazugenommen                               */
/*                      SegInits --> LargeInt                                */
/*            9. 1.1999 ChkPC jetzt mit Adresse als Parameter                */
/*           17. 4.1999 DefCPU hinzugenommen                                 */
/*           30. 5.1999 OutRadixBase hinzugenommen                           */
/*           10. 7.1999 Symbolrecord hierher verschoben                      */
/*           22. 9.1999 RelocEntry definiert                                 */
/*            5.11.1999 ExtendErrors von Boolean nach ShortInt               */
/*                                                                           */
/*****************************************************************************/

#include "chunks.h"

#include "fileformat.h"

typedef Byte CPUVar;

typedef struct _TCPUDef
         {
	  struct _TCPUDef *Next;
	  char *Name;
	  CPUVar Number,Orig;
	  void (*SwitchProc)(
#ifdef __PROTOS__
                             void
#endif
                            );
	 } TCPUDef,*PCPUDef;

typedef enum {TempInt,TempFloat,TempString,TempNone} TempType;

typedef struct _RelocEntry
         {
          struct _RelocEntry *Next;
          char *Ref;
          Byte Add;
         } TRelocEntry, *PRelocEntry;

typedef struct
         {
          TempType Typ;
          PRelocEntry Relocs;
          union
           {
            LargeInt Int;
            Double Float;
            String Ascii;
           } Contents;
         } TempResult;

typedef struct
         {
          TempType Typ;
          TRelocEntry *Relocs;
          union
           {
            LargeInt IWert;
            Double FWert;
            char *SWert;
           } Contents;
         } SymbolVal;

typedef struct _TCrossRef
         {
          struct _TCrossRef *Next;
          Byte FileNum;
          LongInt LineNum;
          Integer OccNum;
         } TCrossRef,*PCrossRef;

typedef struct _SymbolEntry
         {
          struct _SymbolEntry *Left,*Right;
          ShortInt Balance;
          LongInt Attribute;
          char *SymName;
          Byte SymType;
          ShortInt SymSize;
          Boolean Defined,Used,Changeable;
          SymbolVal SymWert;
          PCrossRef RefList;
          Byte FileNum;
          LongInt LineNum;
          TRelocEntry *Relocs;
         } SymbolEntry,*SymbolPtr;

typedef struct _TPatchEntry
        {
          struct _TPatchEntry *Next;
          LargeWord Address;
          Byte *RelocType;
        } TPatchEntry, *PPatchEntry;

typedef enum {DebugNone,DebugMAP,DebugAOUT,DebugCOFF,DebugELF,DebugAtmel,DebugNoICE} DebugType;

#define Char_NUL 0
#define Char_BEL '\a'
#define Char_BS '\b'
#define Char_HT 9
#define Char_LF '\n'
#define Char_FF 12
#define Char_CR 13
#define Char_ESC 27

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
#define MaximumName      "INMAXMODE"  /* CPU im Maximum-Modus */
#define FPUAvailName     "HASFPU"     /* FPU-Befehle erlaubt */
#define LstMacroExName   "MACEXP"     /* expandierte Makros anzeigen */
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
#define DefStackName     "DEFSTACK"   /* Default-Stack */

extern char *EnvName;

#define ParMax 20

#define ChapMax 4

#define StructSeg (PCMax+1)

extern char *SegNames[PCMax+1];
extern char SegShorts[PCMax+1];

extern LongInt Magic;

#define AscOfs '0'

#define MaxCodeLen 1024

extern char *InfoMessCopyright;

typedef void (*SimpProc)(
#ifdef __PROTOS__
void
#endif
);

typedef Word WordField[6];          /* fuer Zahlenumwandlung */
typedef char *ArgStrField[ParMax+1];/* Feld mit Befehlsparametern */
typedef char *StringPtr;

typedef enum {ConstModeIntel,	    /* Hex xxxxh, Okt xxxxo, Bin xxxxb */
	       ConstModeMoto,	    /* Hex $xxxx, Okt @xxxx, Bin %xxxx */
	       ConstModeC}          /* Hex 0x..., Okt 0...., Bin ----- */
             TConstMode;

typedef struct _TFunction
         {
	  struct _TFunction *Next;
	  Byte ArguCnt;
          StringPtr Name,Definition;
	 } TFunction,*PFunction;

typedef struct _TTransTable
         {
          struct _TTransTable *Next;
          char *Name;
          unsigned char *Table;
         } TTransTable,*PTransTable;

typedef struct _TSaveState
	 {
	  struct _TSaveState *Next;
	  CPUVar SaveCPU;
	  Integer SavePC;
	  Byte SaveListOn;
	  Boolean SaveLstMacroEx;
	  PTransTable SaveTransTable;
	 } TSaveState,*PSaveState;

typedef struct _TForwardSymbol
         {
	  struct _TForwardSymbol *Next;
          StringPtr Name;
	  LongInt DestSection;
	 } TForwardSymbol,*PForwardSymbol;

typedef struct _TSaveSection
         {
	  struct _TSaveSection *Next;
          PForwardSymbol LocSyms,GlobSyms,ExportSyms;
	  LongInt Handle;
	 } TSaveSection,*PSaveSection;

typedef struct _TDefinement
         {
 	  struct _TDefinement *Next;
          StringPtr TransFrom,TransTo;
          Byte Compiled[256];
	 } TDefinement,*PDefinement;

typedef struct _TStructure
         {
          struct _TStructure *Next;
          Boolean DoExt;
          char *Name;
          LargeWord CurrPC;
         } TStructure,*PStructure;

extern StringPtr SourceFile;

extern StringPtr ClrEol;
extern StringPtr CursUp;

extern LargeWord PCs[StructSeg+1];
extern LargeWord StartAdr;
extern Boolean StartAdrPresent;
extern LargeWord Phases[StructSeg+1];
extern Word Grans[StructSeg+1];
extern Word ListGrans[StructSeg+1];
extern ChunkList SegChunks[StructSeg+1];
extern Integer ActPC;
extern Boolean PCsUsed[StructSeg+1];
extern LargeInt SegInits[PCMax+1]; 
extern LargeInt SegLimits[PCMax+1]; 
extern LongInt ValidSegs;
extern Boolean ENDOccured;
extern Boolean Retracted;
extern Boolean ListToStdout,ListToNull;

extern Word TypeFlag;
extern ShortInt SizeFlag;

extern Integer PassNo;
extern Integer JmpErrors;
extern Boolean ThrowErrors;
extern Boolean Repass;
extern Byte MaxSymPass;
extern Byte ShareMode;
extern DebugType DebugMode;
extern Byte ListMode;
extern Byte ListOn;
extern Boolean MakeUseList;
extern Boolean MakeCrossList;
extern Boolean MakeSectionList;
extern Boolean MakeIncludeList;
extern Boolean RelaxedMode;
extern Byte ListMask;
extern ShortInt ExtendErrors;

extern LongInt MomSectionHandle;
extern PSaveSection SectionStack;

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

extern FILE *PrgFile;

extern StringPtr ErrorPath,ErrorName;
extern StringPtr OutName;
extern Boolean IsErrorOpen;
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
extern Boolean SetIsOccupied;
extern void (*MakeCode)(void);
extern Boolean (*ChkPC)(LargeWord Addr);
extern Boolean (*IsDef)(void);
extern void (*SwitchFrom)(void);
extern void (*InternSymbol)(char *Asc, TempResult *Erg);
extern void (*InitPassProc)(void);
extern void (*ClearUpProc)(void);

extern StringPtr IncludeList;
extern Integer IncDepth,NextIncDepth;
extern FILE *ErrorFile;
extern FILE *LstFile;
extern FILE *ShareFile;
extern FILE *MacProFile;
extern FILE *MacroFile;
extern Boolean InMacroFlag;
extern StringPtr LstName,MacroName,MacProName;
extern Boolean DoLst,NextDoLst;
extern StringPtr ShareName;
extern CPUVar MomCPU,MomVirtCPU;
extern char DefCPU[20];
extern char MomCPUIdent[10];
extern PCPUDef FirstCPUDef;
extern CPUVar CPUCnt;

extern Boolean FPUAvail;
extern Boolean DoPadding;
extern Boolean SupAllowed;
extern Boolean Maximum;
extern Boolean DoBranchExt;

extern LargeWord RadixBase, OutRadixBase;

extern StringPtr LabPart,OpPart,AttrPart,ArgPart,CommPart,LOpPart;
extern char AttrSplit;
extern ArgStrField ArgStr;
extern Byte ArgCnt;
extern StringPtr OneLine;

extern Byte LstCounter;
extern Word PageCounter[ChapMax+1];
extern Byte ChapDepth;
extern StringPtr ListLine;
extern Byte PageLength,PageWidth;
extern Boolean LstMacroEx;
extern StringPtr PrtInitString;
extern StringPtr PrtExitString;
extern StringPtr PrtTitleString;
extern StringPtr ExtendError;

extern Byte StopfZahl;

extern Boolean SuppWarns;

#define CharTransTable CurrTransTable->Table
extern PTransTable TransTables,CurrTransTable;

extern PFunction FirstFunction;

extern PDefinement FirstDefine;

extern PStructure StructureStack;
extern int StructSaveSeg;

extern PSaveState FirstSaveState;

extern Boolean MakeDebug;
extern FILE *Debug;


extern void AsmDefInit(void);

extern void NullProc(void);

extern void Default_InternSymbol(char *Asc, TempResult *Erg);


extern void asmdef_init(void);
