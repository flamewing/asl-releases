#ifndef _ASMDEF_H
#define _ASMDEF_H
/* asmdef.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* global benutzte Variablen und Definitionen                                */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           24. 6.1998 Zeichenuebersetzungstabellen                         */
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
/*            7. 5.2000 Packing hinzugefuegt                                 */
/*           20. 5.2000 added ArgCName, AllArgName                           */
/*            1. 6.2000 added NestMax                                        */
/*           26. 6.2000 added exports                                        */
/*            1.11.2000 added RelSegs flag                                   */
/*           24.12.2000 added NoICEMask                                      */
/*           2001-09-29 add segment name for STRUCT (just to be sure...)     */
/*           2001-10-20 added GNU error flag                                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmdef.h,v 1.2 2006/06/17 17:03:14 alfred Exp $                      */
/*****************************************************************************
 * $Log: asmdef.h,v $
 * Revision 1.2  2006/06/17 17:03:14  alfred
 * - add HCS08 remark
 *
 * Revision 1.1  2003/11/06 02:49:18  alfred
 * - recreated
 *
 * Revision 1.7  2002/11/23 15:53:27  alfred
 * - SegLimits are unsigned now
 *
 * Revision 1.6  2002/11/17 16:09:12  alfred
 * - added DottedStructs
 *
 * Revision 1.5  2002/11/11 21:12:05  alfred
 * - ListMAsk is 16 bits
 *
 * Revision 1.4  2002/11/10 09:43:07  alfred
 * - relocated symbol node type
 *
 * Revision 1.3  2002/11/04 19:19:08  alfred
 * - add separation character to structs
 *
 * Revision 1.2  2002/05/18 16:09:49  alfred
 * - TempTypes are bit masks
 *
 *****************************************************************************/

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

typedef enum {TempNone = 0, TempInt = 1, TempFloat = 2, TempString = 4, TempAll = 7} TempType;

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

typedef enum {DebugNone,DebugMAP,DebugAOUT,DebugCOFF,DebugELF,DebugAtmel,DebugNoICE} DebugType;

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
#define LabelName        "__LABEL__"  /* Labelansprache in Makros */
#define ArgCName         "ARGCOUNT"   /* Argumentzahlansprache in Makros */
#define AllArgName       "ALLARGS"    /* Ansprache Argumentliste in Makros */
#define DefStackName     "DEFSTACK"   /* Default-Stack */
#define NestMaxName      "NESTMAX"    /* max. nesting level of a macro */
#define DottedStructsName "DOTTEDSTRUCTS" /* struct elements by default with . */

extern char *EnvName;

#define ParMax 20

#define ChapMax 4

#define StructSeg (PCMax+1)

extern char *SegNames[PCMax + 2];
extern char SegShorts[PCMax + 2];

#define AscOfs '0'

#define MaxCodeLen 1024

#define DEF_NESTMAX 256

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

extern StringPtr SourceFile;

extern StringPtr ClrEol;
extern StringPtr CursUp;

extern LargeWord PCs[StructSeg+1];
extern Boolean RelSegs;
extern LargeWord StartAdr;
extern Boolean StartAdrPresent;
extern LargeWord Phases[StructSeg+1];
extern Word Grans[StructSeg+1];
extern Word ListGrans[StructSeg+1];
extern ChunkList SegChunks[StructSeg+1];
extern Integer ActPC;
extern Boolean PCsUsed[StructSeg+1];
extern LargeWord SegInits[PCMax+1]; 
extern LargeWord SegLimits[PCMax+1]; 
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
extern LongInt NestMax;
extern Boolean GNUErrors;

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
extern Boolean Packing;
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
extern Boolean LstMacroEx, DottedStructs;
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

extern PSaveState FirstSaveState;

extern Boolean MakeDebug;
extern FILE *Debug;


extern void AsmDefInit(void);

extern void NullProc(void);

extern void Default_InternSymbol(char *Asc, TempResult *Erg);


extern void asmdef_init(void);
#endif /* _ASMDEF_H */
