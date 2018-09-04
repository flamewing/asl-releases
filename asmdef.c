/* asmdef.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* global benutzte Variablen                                                 */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           24. 6.1998 Zeichenuebersetzungstabellen                         */
/*           25. 7.1998 PassNo --> Integer                                   */
/*           17. 8.1998 InMacroFlag hierher verschoben                       */
/*           18. 8.1998 RadixBase hinzugenommen                              */
/*                      ArgStr-Feld war eins zu kurz                         */
/*           19. 8.1998 BranchExt-Variablen                                  */
/*           29. 8.1998 ActListGran hinzugenommen                            */
/*           11. 9.1998 ROMDATA-Segment hinzugenommen                        */
/*            1. 1.1999 SegLimits dazugenommen                               */
/*                      SegInits --> LargeInt                                */
/*            9. 1.1999 ChkPC jetzt mit Adresse als Parameter                */
/*           18. 1.1999 PCSymbol initialisiert                               */
/*           17. 4.1999 DefCPU hinzugenommen                                 */
/*           30. 5.1999 OutRadixBase hinzugenommen                           */
/*            5.11.1999 ExtendErrors von Boolean nach ShortInt               */
/*            7. 5.2000 Packing hinzugefuegt                                 */
/*            1. 6.2000 added NestMax                                        */
/*            2. 7.2000 updated year in copyright                            */
/*            1.11.2000 added RelSegs flag                                   */
/*           24.12.2000 added NoICEMask                                      */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           27. 3.2001 don't use a number as default PC symbol              */
/*           2001-09-29 add segment name for STRUCT (just to be sure...)     */
/*           2001-10-20 added GNU error flag                                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmdef.c,v 1.15 2017/04/02 11:10:36 alfred Exp $                     */
/*****************************************************************************
 * $Log: asmdef.c,v $
 * Revision 1.15  2017/04/02 11:10:36  alfred
 * - allow more fine-grained macro expansion in listing
 *
 * Revision 1.14  2016/11/25 16:29:36  alfred
 * - allow SELECT as alternative to SWITCH
 *
 * Revision 1.13  2016/11/24 22:41:46  alfred
 * - add SELECT as alternative to SWITCH
 *
 * Revision 1.12  2015/08/28 17:22:26  alfred
 * - add special handling for labels following BSR
 *
 * Revision 1.11  2014/12/01 15:14:43  alfred
 * - rework to current style
 *
 * Revision 1.10  2014/11/23 18:29:29  alfred
 * - correct buffer overflow in MomCPUName
 *
 * Revision 1.9  2014/11/06 11:22:01  alfred
 * - replace hook chain for ClearUp, document new mechanism
 *
 * Revision 1.8  2014/11/05 15:47:13  alfred
 * - replace InitPass callchain with registry
 *
 * Revision 1.7  2014/06/15 09:17:08  alfred
 * - optional Memo profiling
 *
 * Revision 1.6  2014/03/08 21:06:35  alfred
 * - rework ASSUME framework
 *
 * Revision 1.5  2013/12/21 19:46:50  alfred
 * - dynamically resize code buffer
 *
 * Revision 1.4  2006/08/05 20:05:27  alfred
 * - correct allocation size
 *
 * Revision 1.3  2006/08/05 18:25:47  alfred
 * - make some arrays dynamic to save data segment space
 *
 * Revision 1.2  2004/09/20 18:43:51  alfred
 * - correct allocation size
 *
 * Revision 1.1  2003/11/06 02:49:18  alfred
 * - recreated
 *
 * Revision 1.5  2003/09/21 21:15:54  alfred
 * - fix string length
 *
 * Revision 1.4  2002/11/23 15:53:27  alfred
 * - SegLimits are unsigned now
 *
 * Revision 1.3  2002/11/17 16:09:12  alfred
 * - added DottedStructs
 *
 *****************************************************************************/

#include "stdinc.h"

#include <errno.h>

#include "strutil.h"
#include "stringlists.h"
#include "chunks.h"

#include "asmdef.h"
#include "asmsub.h"

char SrcSuffix[] = ".asm";             /* Standardendungen: Hauptdatei */
char IncSuffix[] = ".inc";             /* Includedatei */
char PrgSuffix[] = ".p";               /* Programmdatei */
char LstSuffix[] = ".lst";             /* Listingdatei */
char MacSuffix[] = ".mac";             /* Makroausgabe */
char PreSuffix[] = ".i";               /* Ausgabe Makroprozessor */
char LogSuffix[] = ".log";             /* Fehlerdatei */
char MapSuffix[] = ".map";             /* Debug-Info/Map-Format */
char OBJSuffix[] = ".obj";

char *EnvName = "ASCMD";               /* Environment-Variable fuer Default-
                                          Parameter */

char *SegNames[PCMax + 2] =
{
  "NOTHING", "CODE", "DATA", "IDATA", "XDATA", "YDATA",
  "BITDATA", "IO", "REG", "ROMDATA", "STRUCT"
};
char SegShorts[PCMax + 2] =
{
  '-','C','D','I','X','Y','B','P','R','O','S'
};

/** ValidSymChars:SET OF Char=['A'..'Z','a'..'z',#128..#165,'0'..'9','_','.']; **/

StringPtr SourceFile;                    /* Hauptquelldatei */

StringPtr ClrEol;       	            /* String fuer loeschen bis Zeilenende */
StringPtr CursUp;		            /*   "     "  Cursor hoch */

LargeWord *PCs;                          /* Programmzaehler */
Boolean RelSegs;                         /* relokatibles Segment ? */
LargeWord StartAdr;                      /* Programmstartadresse */
Boolean StartAdrPresent;                 /*          "           definiert? */
LargeWord AfterBSRAddr;                  /* address right behind last BSR */
LargeWord *Phases;                       /* Verschiebungen */
Word Grans[StructSeg + 1];               /* Groesse der Adressierungselemente */
Word ListGrans[StructSeg + 1];           /* Wortgroesse im Listing */
ChunkList SegChunks[StructSeg + 1];      /* Belegungen */
Integer ActPC;                           /* gewaehlter Programmzaehler */
Boolean PCsUsed[StructSeg + 1];          /* PCs bereits initialisiert ? */
LargeWord *SegInits;                     /* Segmentstartwerte */
LargeWord *SegLimits;                    /* Segmentgrenzwerte */
LongInt ValidSegs;                       /* erlaubte Segmente */
Boolean ENDOccured;	                 /* END-Statement aufgetreten ? */
Boolean Retracted;	                 /* Codes zurueckgenommen ? */
Boolean ListToStdout, ListToNull;        /* Listing auf Konsole/Nulldevice ? */

unsigned ASSUMERecCnt;
const ASSUMERec *pASSUMERecs;
void (*pASSUMEOverride)(void);

Word TypeFlag;	                         /* Welche Adressraeume genutzt ? */
ShortInt SizeFlag;		         /* Welche Operandengroessen definiert ? */

Integer PassNo;                          /* Durchlaufsnummer */
Integer JmpErrors;                       /* Anzahl fraglicher Sprungfehler */
Boolean ThrowErrors;                     /* Fehler verwerfen bei Repass ? */
Boolean Repass;		                 /* noch ein Durchlauf erforderlich */
Byte MaxSymPass;	                 /* Pass, nach dem Symbole definiert sein muessen */
Byte ShareMode;                          /* 0=kein SHARED,1=Pascal-,2=C-Datei, 3=ASM-Datei */
DebugType DebugMode;                     /* Ausgabeformat Debug-Datei */
Word NoICEMask;                          /* which symbols to use in NoICE dbg file */
Byte ListMode;                           /* 0=kein Listing,1=Konsole,2=auf Datei */
Byte ListOn;		    	         /* Listing erzeugen ? */
Boolean MakeUseList;                     /* Belegungsliste ? */
Boolean MakeCrossList;	                 /* Querverweisliste ? */
Boolean MakeSectionList;                 /* Sektionsliste ? */
Boolean MakeIncludeList;                 /* Includeliste ? */
Boolean RelaxedMode;		         /* alle Integer-Syntaxen zulassen ? */
Word ListMask;                           /* Listingmaske */
ShortInt ExtendErrors;	                 /* erweiterte Fehlermeldungen */
Integer EnumSegment;                     /* ENUM state & config */
LongInt EnumIncrement, EnumCurrentValue;
Boolean NumericErrors;                   /* Fehlermeldungen mit Nummer */
Boolean CodeOutput;		         /* Code erzeugen */
Boolean MacProOutput;                    /* Makroprozessorausgabe schreiben */
Boolean MacroOutput;                     /* gelesene Makros schreiben */
Boolean QuietMode;                       /* keine Meldungen */
Boolean HardRanges;                      /* Bereichsfehler echte Fehler ? */
char *DivideChars;                       /* Trennzeichen fuer Parameter. Inhalt Read Only! */
Boolean HasAttrs;                        /* Opcode hat Attribut */
char *AttrChars;                         /* Zeichen, mit denen Attribut abgetrennt wird */
Boolean MsgIfRepass;                     /* Meldungen, falls neuer Pass erforderlich */
Integer PassNoForMessage;                /* falls ja: ab welchem Pass ? */
Boolean CaseSensitive;                   /* Gross/Kleinschreibung unterscheiden ? */
LongInt NestMax;                         /* max. nesting level of a macro */
Boolean GNUErrors;                       /* GNU-error-style messages ? */

FILE *PrgFile;                           /* Codedatei */

StringPtr ErrorPath, ErrorName;          /* Ausgabedatei Fehlermeldungen */
StringPtr OutName;                       /* Name Code-Datei */
Boolean IsErrorOpen;
StringPtr CurrFileName;                  /* mom. bearbeitete Datei */
LongInt MomLineCounter;                  /* Position in mom. Datei */
LongInt CurrLine;       	         /* virtuelle Position */
LongInt LineSum;                         /* Gesamtzahl Quellzeilen */
LongInt MacLineSum;                      /* inkl. Makroexpansion */

LongInt NOPCode;                         /* Maschinenbefehl NOP zum Stopfen */
Boolean TurnWords;                       /* TRUE  = Motorola-Wortformat */
                                         /* FALSE = Intel-Wortformat */
Byte HeaderID;	                         /* Kennbyte des Codeheaders */
char *PCSymbol;	                         /* Symbol, womit Programmzaehler erreicht wird. Inhalt Read Only! */
TConstMode ConstMode;
Boolean SetIsOccupied,			 /* TRUE: SET/SWITCH/PAGE ist Prozessorbefehl */
        SwitchIsOccupied,
        PageIsOccupied;
#ifdef __PROTOS__
void (*MakeCode)(void);                  /* Codeerzeugungsprozedur */
Boolean (*ChkPC)(LargeWord Addr);        /* ueberprueft Codelaengenueberschreitungen */
Boolean (*IsDef)(void);	                 /* ist Label nicht als solches zu werten ? */
void (*SwitchFrom)(void);                /* bevor von einer CPU weggeschaltet wird */
void (*InternSymbol)(char *Asc, TempResult *Erg); /* vordefinierte Symbole ? */
#else
void (*MakeCode)();
Boolean (*ChkPC)();
Boolean (*IsDef)();
void (*SwitchFrom)();
void (*InternSymbol)();
#endif
DissectBitProc DissectBit;

StringPtr IncludeList;	                /* Suchpfade fuer Includedateien */
Integer IncDepth, NextIncDepth;         /* Verschachtelungstiefe INCLUDEs */
FILE *ErrorFile;                        /* Fehlerausgabe */
FILE *LstFile;                          /* Listdatei */
FILE *ShareFile;                        /* Sharefile */
FILE *MacProFile;                       /* Makroprozessorausgabe */
FILE *MacroFile;                        /* Ausgabedatei Makroliste */
Boolean InMacroFlag;                    /* momentan wird Makro expandiert */
StringPtr LstName;                      /* Name der Listdatei */
StringPtr MacroName, MacProName;
tLstMacroExp DoLst, NextDoLst;          /* Listing an */
StringPtr ShareName;                    /* Name des Sharefiles */

CPUVar MomCPU,MomVirtCPU;               /* definierter/vorgegaukelter Prozessortyp */
char DefCPU[20];                        /* per Kommandozeile vorgegebene CPU */
char MomCPUIdent[20];                   /* dessen Name in ASCII */

Boolean FPUAvail;                       /* Koprozessor erlaubt ? */
Boolean DoPadding;                      /* auf gerade Byte-Zahl ausrichten ? */
Boolean Packing;                        /* gepackte Ablage ? */
Boolean SupAllowed;                     /* Supervisormode freigegeben */
Boolean Maximum;                        /* CPU nicht kastriert */
Boolean DoBranchExt;                    /* Spruenge automatisch verlaengern */

LargeWord RadixBase;                    /* Default-Zahlensystem im Formelparser*/
LargeWord OutRadixBase;                 /* dito fuer Ausgabe */

tStrComp ArgStr[ArgCntMax + 1];         /* Komponenten der Zeile */
StringPtr LOpPart;
tStrComp LabPart, CommPart, ArgPart, OpPart, AttrPart;
char AttrSplit;
int ArgCnt;                             /* Argumentzahl */
StringPtr OneLine;                      /* eingelesene Zeile */
#ifdef PROFILE_MEMO
unsigned NumMemo;
unsigned long NumMemoCnt, NumMemoSum;
#endif

Byte LstCounter;                        /* Zeilenzaehler fuer automatischen Umbruch */
Word PageCounter[ChapMax + 1];          /* hierarchische Seitenzaehler */
Byte ChapDepth;                         /* momentane Kapitelverschachtelung */
StringPtr ListLine;                     /* alternative Ausgabe vor Listing fuer EQU */
Byte PageLength, PageWidth;             /* Seitenlaenge/breite in Zeilen/Spalten */
tLstMacroExp LstMacroExp;               /* Makroexpansionen auflisten */
tLstMacroExpMod LstMacroExpOverride;    /* Override macro expansion ? */
Boolean DottedStructs;                  /* structure elements with dots */
StringPtr PrtInitString;                /* Druckerinitialisierungsstring */
StringPtr PrtExitString;                /* Druckerdeinitialisierungsstring */
StringPtr PrtTitleString;               /* Titelzeile */

LongInt MomSectionHandle;               /* mom. Namensraum */
PSaveSection SectionStack;              /* gespeicherte Sektionshandles */
tSavePhase *pPhaseStacks[PCMax];	/* saves nested PHASE values */

LongWord MaxCodeLen = 0;                /* max. length of generated code */
LongInt CodeLen;                        /* Laenge des erzeugten Befehls */
LongWord *DAsmCode;                     /* Zwischenspeicher erzeugter Code */
Word *WAsmCode;
Byte *BAsmCode;

Boolean DontPrint;                      /* Flag:PC veraendert, aber keinen Code erzeugt */
Word ActListGran;                       /* uebersteuerte List-Granularitaet */

Byte StopfZahl;                         /* Anzahl der im 2.Pass festgestellten
                                           ueberfluessigen Worte, die mit NOP ge-
                                           fuellt werden muessen */

Boolean SuppWarns;

PTransTable TransTables,                /* Liste mit Codepages */
            CurrTransTable;             /* aktuelle Codepage */

PFunction FirstFunction;	        /* Liste definierter Funktionen */

PDefinement FirstDefine;                /* Liste von Praeprozessor-Defines */

PSaveState FirstSaveState;              /* gesicherte Zustaende */

Boolean MakeDebug;                      /* Debugginghilfe */
FILE *Debug;


void AsmDefInit(void)
{
  LongInt z;

  DoLst = True;
  PassNo = 1;
  MaxSymPass = 1;

  LineSum = 0;

  for (z = 0; z <= ChapMax; PageCounter[z++] = 0);
  LstCounter = 0;
  ChapDepth = 0;

  PrtInitString[0] = '\0';
  PrtExitString[0] = '\0';
  PrtTitleString[0] = '\0';

  CurrFileName[0] = '\0';
  MomLineCounter = 0;

  FirstFunction = NULL;
  FirstDefine = NULL;
  FirstSaveState = NULL;
}

void NullProc(void)
{
}

void Default_InternSymbol(char *Asc, TempResult *Erg)
{
  UNUSED(Asc);

  Erg->Typ = TempNone;
}

void Default_DissectBit(char *pDest, int DestSize, LargeWord BitSpec)
{
  HexString(pDest, DestSize, BitSpec, 0);
}

static char *GetString(void)
{
  return malloc(STRINGSIZE * sizeof(char));
}

int SetMaxCodeLen(LongWord NewMaxCodeLen)
{
  if (NewMaxCodeLen > MaxCodeLen_Max)
    return ENOMEM;
  if (NewMaxCodeLen > MaxCodeLen)
  {
    void *pNewMem;

    if (!MaxCodeLen)
      pNewMem = (LongWord *) malloc(NewMaxCodeLen);
    else
      pNewMem = (LongWord *) realloc(DAsmCode, NewMaxCodeLen);
    if (!pNewMem)
      return ENOMEM;

    DAsmCode = (LongWord *)pNewMem;
    WAsmCode = (Word *) DAsmCode;
    BAsmCode = (Byte *) DAsmCode;
    MaxCodeLen = NewMaxCodeLen;
  }
  return 0;
}

void asmdef_init(void)
{
  int z;

  SwitchFrom = NullProc;
  InternSymbol = Default_InternSymbol;
  DissectBit = Default_DissectBit;

  SetMaxCodeLen(MaxCodeLen_Ini);

  RelaxedMode = True;
  ConstMode = ConstModeC;

  /* auf diese Weise wird PCSymbol defaultmaessig nicht erreichbar
     da das schon von den Konstantenparsern im Formelparser abgefangen
     wuerde */

  PCSymbol = "--PC--SYMBOL--";
  *DefCPU = '\0';

  for (z = 0; z <= ArgCntMax; z++)
    StrCompAlloc(&ArgStr[z]);
  SourceFile = GetString();
  ClrEol = GetString();
  CursUp = GetString();
  ErrorPath = GetString();
  ErrorName = GetString();
  OutName = GetString();
  CurrFileName = GetString();
  IncludeList = GetString();
  LstName = GetString();
  MacroName = GetString();
  MacProName = GetString();
  ShareName = GetString();
  StrCompAlloc(&LabPart);
  StrCompAlloc(&OpPart);
  StrCompAlloc(&AttrPart);
  StrCompAlloc(&ArgPart);
  StrCompAlloc(&CommPart);
  LOpPart = GetString();
  OneLine = GetString();
  ListLine = GetString();
  PrtInitString = GetString();
  PrtExitString = GetString();
  PrtTitleString = GetString();

  SegInits = (LargeWord*)malloc((PCMax + 1) * sizeof(LargeWord));
  SegLimits = (LargeWord*)malloc((PCMax + 1) * sizeof(LargeWord));
  Phases = (LargeWord*)malloc((StructSeg + 1) * sizeof(LargeWord));
  PCs = (LargeWord*)malloc((StructSeg + 1) * sizeof(LargeWord));
}
