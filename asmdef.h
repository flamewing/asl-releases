/* asmdef.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* global benutzte Variablen und Definitionen                                */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
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
	  void (*SwitchProc)(void);
	 } TCPUDef,*PCPUDef;

typedef enum {TempInt,TempFloat,TempString,TempNone} TempType;
typedef struct
         {
          TempType Typ;
          union
           {
            LargeInt Int;
            Double Float;
            String Ascii;
           } Contents;
         } TempResult;

typedef enum {DebugNone,DebugMAP,DebugAOUT,DebugCOFF,DebugELF} DebugType;

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

extern char *SrcSuffix,*IncSuffix,*PrgSuffix,*LstSuffix,
            *MacSuffix,*PreSuffix,*LogSuffix,*MapSuffix;
            
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
#define FlagTrueName     "TRUE"	      /* Flagkonstanten */
#define FlagFalseName    "FALSE"
#define PiName           "CONSTPI"    /* Zahl Pi */
#define DateName         "DATE"       /* Datum & Zeit */
#define TimeName         "TIME"
#define VerName          "VERSION"    /* speichert Versionsnummer */
#define CaseSensName     "CASESENSITIVE" /* zeigt Gross/Kleinunterscheidung an */
#define Has64Name        "HAS64"      /* arbeitet Parser mit 64-Bit-Integers ? */
#define AttrName         "ATTRIBUTE"  /* Attributansprache in Makros */
#define DefStackName     "DEFSTACK"   /* Default-Stack */

extern char *Version;
extern LongInt VerNo;

extern char *EnvName;

#define ParMax 20

#define ChapMax 4

extern char *SegNames[PCMax+1];
extern char SegShorts[PCMax+1];

extern LongInt Magic;

#define AscOfs '0'

#define MaxCodeLen 1024

extern char *InfoMessCopyright;

typedef void (*SimpProc)(void);

typedef Word WordField[6];          /* fuer Zahlenumwandlung */
typedef String ArgStrField[ParMax]; /* Feld mit Befehlsparametern */
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

typedef struct _TSaveState
	 {
	  struct _TSaveState *Next;
	  CPUVar SaveCPU;
	  Integer SavePC;
	  Byte SaveListOn;
	  Boolean SaveLstMacroEx;
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


extern String SourceFile;

extern String ClrEol;
extern String CursUp;

extern LargeWord PCs[PCMax+1];
extern LargeWord StartAdr;
extern Boolean StartAdrPresent;
extern LargeWord Phases[PCMax+1];
extern Word Grans[PCMax+1];
extern Word ListGrans[PCMax+1];
extern ChunkList SegChunks[PCMax+1];
extern Integer ActPC;
extern Boolean PCsUsed[PCMax+1];
extern LongInt SegInits[PCMax+1]; 
extern LongInt ValidSegs;
extern Boolean ENDOccured;
extern Boolean Retracted;

extern Word TypeFlag;
extern ShortInt SizeFlag;

extern Byte PassNo;
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
extern Boolean ExtendErrors;

extern LongInt MomSectionHandle;
extern PSaveSection SectionStack;

extern LongInt CodeLen;
extern Byte *BAsmCode;
extern Word *WAsmCode;
extern LongWord *DAsmCode;

extern Boolean DontPrint;

extern Boolean NumericErrors;
extern Boolean CodeOutput;
extern Boolean MacProOutput;
extern Boolean MacroOutput;
extern Boolean QuietMode;
extern char *DivideChars;
extern Boolean HasAttrs;
extern char *AttrChars;
extern Boolean MsgIfRepass;
extern Integer PassNoForMessage;
extern Boolean CaseSensitive;

extern FILE *PrgFile;

extern String ErrorPath,ErrorName;
extern String OutName;
extern Boolean IsErrorOpen;
extern String CurrFileName;
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
extern Boolean (*ChkPC)(void);
extern Boolean (*IsDef)(void);
extern void (*SwitchFrom)(void);
extern void (*InternSymbol)(char *Asc, TempResult *Erg);
extern void (*InitPassProc)(void);
extern void (*ClearUpProc)(void);

extern String IncludeList;
extern Integer IncDepth,NextIncDepth;
extern FILE *ErrorFile;
extern FILE *LstFile;
extern FILE *ShareFile;
extern FILE *MacProFile;
extern FILE *MacroFile;
extern String LstName;
extern Boolean DoLst,NextDoLst;
extern String ShareName;
extern CPUVar MomCPU,MomVirtCPU;
extern char MomCPUIdent[10];
extern PCPUDef FirstCPUDef;
extern CPUVar CPUCnt;

extern Boolean FPUAvail;
extern Boolean DoPadding;
extern Boolean SupAllowed;
extern Boolean Maximum;

extern String LabPart,OpPart,AttrPart,ArgPart,CommPart,LOpPart;
extern char AttrSplit;
extern ArgStrField ArgStr;
extern Byte ArgCnt;
extern String OneLine;

extern Byte LstCounter;
extern Word PageCounter[ChapMax+1];
extern Byte ChapDepth;
extern String ListLine;
extern String ErrorPos;
extern Byte PageLength,PageWidth;
extern Boolean LstMacroEx;
extern String PrtInitString;
extern String PrtExitString;
extern String PrtTitleString;
extern String ExtendError;

extern Byte StopfZahl;

extern Boolean SuppWarns;

extern unsigned char CharTransTable[256];

extern PFunction FirstFunction;

extern PDefinement FirstDefine;

extern PSaveState FirstSaveState;

extern Boolean MakeDebug;
extern FILE *Debug;


extern void AsmDefInit(void);

extern void NullProc(void);

extern void Default_InternSymbol(char *Asc, TempResult *Erg);


extern void asmdef_init(void);
