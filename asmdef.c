/* asmdef.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* global benutzte Variablen                                                 */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           24. 6.1998 Zeichenübersetzungstabellen                          */
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

#include "stdinc.h"

#include "stringlists.h"
#include "chunks.h"

#include "asmdef.h"
#include "asmsub.h"

char SrcSuffix[]=".asm";             /* Standardendungen: Hauptdatei */
char IncSuffix[]=".inc";             /* Includedatei */
char PrgSuffix[]=".p";               /* Programmdatei */
char LstSuffix[]=".lst";             /* Listingdatei */
char MacSuffix[]=".mac";             /* Makroausgabe */
char PreSuffix[]=".i";               /* Ausgabe Makroprozessor */
char LogSuffix[]=".log";             /* Fehlerdatei */
char MapSuffix[]=".map";             /* Debug-Info/Map-Format */
char OBJSuffix[]=".obj";

char *EnvName="ASCMD";                /* Environment-Variable fuer Default-
					Parameter */

char *SegNames[PCMax + 2] = {"NOTHING", "CODE", "DATA", "IDATA", "XDATA", "YDATA",
                             "BITDATA", "IO", "REG", "ROMDATA", "STRUCT"};
char SegShorts[PCMax + 2] = {'-','C','D','I','X','Y','B','P','R','O','S'};

/** ValidSymChars:SET OF Char=['A'..'Z','a'..'z',#128..#165,'0'..'9','_','.']; **/

   StringPtr SourceFile;                    /* Hauptquelldatei */

   StringPtr ClrEol;       	            /* String fuer loeschen bis Zeilenende */
   StringPtr CursUp;		            /*   "     "  Cursor hoch */

   LargeWord PCs[StructSeg+1];              /* Programmzaehler */
   Boolean RelSegs;                         /* relokatibles Segment ? */
   LargeWord StartAdr;                      /* Programmstartadresse */
   Boolean StartAdrPresent;                 /*          "           definiert? */
   LargeWord Phases[StructSeg+1];           /* Verschiebungen */
   Word Grans[StructSeg+1]; 	            /* Groesse der Adressierungselemente */
   Word ListGrans[StructSeg+1];             /* Wortgroesse im Listing */
   ChunkList SegChunks[StructSeg+1];        /* Belegungen */
   Integer ActPC;                           /* gewaehlter Programmzaehler */
   Boolean PCsUsed[StructSeg+1];            /* PCs bereits initialisiert ? */
   LargeInt SegInits[PCMax+1];              /* Segmentstartwerte */
   LargeInt SegLimits[PCMax+1];             /* Segmentgrenzwerte */
   LongInt ValidSegs;                       /* erlaubte Segmente */
   Boolean ENDOccured;	                    /* END-Statement aufgetreten ? */
   Boolean Retracted;			    /* Codes zurueckgenommen ? */
   Boolean ListToStdout,ListToNull;         /* Listing auf Konsole/Nulldevice ? */

   Word TypeFlag;		    /* Welche Adressraeume genutzt ? */
   ShortInt SizeFlag;		    /* Welche Operandengroessen definiert ? */

   Integer PassNo;                  /* Durchlaufsnummer */
   Integer JmpErrors;               /* Anzahl fraglicher Sprungfehler */
   Boolean ThrowErrors;             /* Fehler verwerfen bei Repass ? */
   Boolean Repass;		    /* noch ein Durchlauf erforderlich */
   Byte MaxSymPass;	            /* Pass, nach dem Symbole definiert sein muessen */
   Byte ShareMode;                  /* 0=kein SHARED,1=Pascal-,2=C-Datei, 3=ASM-Datei */
   DebugType DebugMode;             /* Ausgabeformat Debug-Datei */
   Word NoICEMask;                  /* which symbols to use in NoICE dbg file */
   Byte ListMode;                   /* 0=kein Listing,1=Konsole,2=auf Datei */
   Byte ListOn;		    	    /* Listing erzeugen ? */
   Boolean MakeUseList; 	    /* Belegungsliste ? */
   Boolean MakeCrossList;	    /* Querverweisliste ? */
   Boolean MakeSectionList;	    /* Sektionsliste ? */
   Boolean MakeIncludeList;         /* Includeliste ? */
   Boolean RelaxedMode;		    /* alle Integer-Syntaxen zulassen ? */
   Byte ListMask;		    /* Listingmaske */
   ShortInt ExtendErrors;	    /* erweiterte Fehlermeldungen */
   Boolean NumericErrors;	    /* Fehlermeldungen mit Nummer */
   Boolean CodeOutput;		    /* Code erzeugen */
   Boolean MacProOutput;	    /* Makroprozessorausgabe schreiben */
   Boolean MacroOutput;             /* gelesene Makros schreiben */
   Boolean QuietMode;               /* keine Meldungen */
   Boolean HardRanges;              /* Bereichsfehler echte Fehler ? */
   char *DivideChars;               /* Trennzeichen fuer Parameter. Inhalt Read Only! */
   Boolean HasAttrs;                /* Opcode hat Attribut */
   char *AttrChars;                 /* Zeichen, mit denen Attribut abgetrennt wird */
   Boolean MsgIfRepass;		    /* Meldungen, falls neuer Pass erforderlich */
   Integer PassNoForMessage;        /* falls ja: ab welchem Pass ? */
   Boolean CaseSensitive;           /* Gross/Kleinschreibung unterscheiden ? */
   LongInt NestMax;                 /* max. nesting level of a macro */
   Boolean GNUErrors;               /* GNU-error-style messages ? */

   FILE *PrgFile;                   /* Codedatei */

   StringPtr ErrorPath,ErrorName;   /* Ausgabedatei Fehlermeldungen */
   StringPtr OutName;               /* Name Code-Datei */
   Boolean IsErrorOpen;
   StringPtr CurrFileName;          /* mom. bearbeitete Datei */
   LongInt MomLineCounter;          /* Position in mom. Datei */
   LongInt CurrLine;       	    /* virtuelle Position */
   LongInt LineSum;		    /* Gesamtzahl Quellzeilen */
   LongInt MacLineSum;              /* inkl. Makroexpansion */

   LongInt NOPCode;                 /* Maschinenbefehl NOP zum Stopfen */
   Boolean TurnWords;		    /* TRUE  = Motorola-Wortformat */
				    /* FALSE = Intel-Wortformat */
   Byte HeaderID;		    /* Kennbyte des Codeheaders */
   char *PCSymbol;		    /* Symbol, womit Programmzaehler erreicht wird. Inhalt Read Only! */
   TConstMode ConstMode;
   Boolean SetIsOccupied;           /* TRUE: SET ist Prozessorbefehl */
#ifdef __PROTOS__
   void (*MakeCode)(void);          /* Codeerzeugungsprozedur */
   Boolean (*ChkPC)(LargeWord Addr);/* ueberprueft Codelaengenueberschreitungen */
   Boolean (*IsDef)(void);	    /* ist Label nicht als solches zu werten ? */
   void (*SwitchFrom)(void);        /* bevor von einer CPU weggeschaltet wird */
   void (*InternSymbol)(char *Asc, TempResult *Erg); /* vordefinierte Symbole ? */
   void (*InitPassProc)(void);	    /* Funktion zur Vorinitialisierung vor einem Pass */
   void (*ClearUpProc)(void);       /* Aufraeumen nach Assemblierung */
#else
   void (*MakeCode)();
   Boolean (*ChkPC)();
   Boolean (*IsDef)();
   void (*SwitchFrom)();
   void (*InternSymbol)();
   void (*InitPassProc)();
   void (*ClearUpProc)();
#endif

   StringPtr IncludeList;	    /* Suchpfade fuer Includedateien */
   Integer IncDepth,NextIncDepth;   /* Verschachtelungstiefe INCLUDEs */
   FILE *ErrorFile;		    /* Fehlerausgabe */
   FILE *LstFile;                   /* Listdatei */
   FILE *ShareFile;                 /* Sharefile */
   FILE *MacProFile;		    /* Makroprozessorausgabe */
   FILE *MacroFile;		    /* Ausgabedatei Makroliste */
   Boolean InMacroFlag;             /* momentan wird Makro expandiert */
   StringPtr LstName;		    /* Name der Listdatei */
   StringPtr MacroName,MacProName;   
   Boolean DoLst,NextDoLst;         /* Listing an */
   StringPtr ShareName;             /* Name des Sharefiles */
/**   PrgName:String;                  { Name der Codedatei }**/

   CPUVar MomCPU,MomVirtCPU;        /* definierter/vorgegaukelter Prozessortyp */
   char DefCPU[20];                 /* per Kommandozeile vorgegebene CPU */
   char MomCPUIdent[10];            /* dessen Name in ASCII */
   PCPUDef FirstCPUDef;		    /* Liste mit Prozessordefinitionen */
   CPUVar CPUCnt;	    	    /* Gesamtzahl Prozessoren */

   Boolean FPUAvail;		    /* Koprozessor erlaubt ? */
   Boolean DoPadding;		    /* auf gerade Byte-Zahl ausrichten ? */
   Boolean Packing;                 /* gepackte Ablage ? */
   Boolean SupAllowed;              /* Supervisormode freigegeben */
   Boolean Maximum;		    /* CPU nicht kastriert */
   Boolean DoBranchExt;             /* Spruenge automatisch verlaengern */

   LargeWord RadixBase;             /* Default-Zahlensystem im Formelparser*/
   LargeWord OutRadixBase;          /* dito fuer Ausgabe */

   StringPtr LabPart,OpPart,AttrPart, /* Komponenten der Zeile */
          ArgPart,CommPart,LOpPart;
   char AttrSplit;
   ArgStrField ArgStr;              /* Komponenten des Arguments */
   Byte ArgCnt;                     /* Argumentzahl */
   StringPtr OneLine;               /* eingelesene Zeile */

   Byte LstCounter;                 /* Zeilenzaehler fuer automatischen Umbruch */
   Word PageCounter[ChapMax+1];     /* hierarchische Seitenzaehler */
   Byte ChapDepth;                  /* momentane Kapitelverschachtelung */
   StringPtr ListLine;		    /* alternative Ausgabe vor Listing fuer EQU */
   Byte PageLength,PageWidth;       /* Seitenlaenge/breite in Zeilen/Spalten */
   Boolean LstMacroEx;              /* Makroexpansionen auflisten */
   StringPtr PrtInitString;	    /* Druckerinitialisierungsstring */
   StringPtr PrtExitString;	    /* Druckerdeinitialisierungsstring */
   StringPtr PrtTitleString;	    /* Titelzeile */
   StringPtr ExtendError;           /* erweiterte Fehlermeldung */

   LongInt MomSectionHandle;        /* mom. Namensraum */
   PSaveSection SectionStack;	    /* gespeicherte Sektionshandles */

   LongInt CodeLen;                 /* Laenge des erzeugten Befehls */
   LongWord *DAsmCode;              /* Zwischenspeicher erzeugter Code */
   Word *WAsmCode;
   Byte *BAsmCode;

   Boolean DontPrint;               /* Flag:PC veraendert, aber keinen Code erzeugt */
   Word ActListGran;                /* uebersteuerte List-Granularitaet */
/**   MultiFace:RECORD Case Byte OF
		    0:(Feld:WordField);
		    1:(Val32:Single);
		    2:(Val64:Double);
		    3:(Val80:Extended);
		    4:(ValCo:Comp);
		    END;**/

   Byte StopfZahl;                  /* Anzahl der im 2.Pass festgestellten
				       ueberfluessigen Worte, die mit NOP ge-
				       fuellt werden muessen */

   Boolean SuppWarns;

   PTransTable TransTables,         /* Liste mit Codepages */
               CurrTransTable;      /* aktuelle Codepage */

   PFunction FirstFunction;	    /* Liste definierter Funktionen */

   PDefinement FirstDefine;         /* Liste von Praeprozessor-Defines */

   PStructure StructureStack;       /* momentan offene Strukturen */
   int StructSaveSeg;               /* gesichertes Segment waehrend Strukturdef.*/

   PSaveState FirstSaveState;	    /* gesicherte Zustaende */

   Boolean MakeDebug;		    /* Debugginghilfe */
   FILE *Debug;


	void AsmDefInit(void)
BEGIN
   LongInt z;

   DoLst=True; PassNo=1; MaxSymPass=1;

   LineSum=0;

   for (z=0; z<=ChapMax; PageCounter[z++]=0);
   LstCounter=0; ChapDepth=0;

   PrtInitString[0]='\0'; PrtExitString[0]='\0'; PrtTitleString[0]='\0';

   ExtendError[0]='\0';

   CurrFileName[0]='\0'; MomLineCounter=0;

   FirstFunction=Nil; FirstDefine=Nil; FirstSaveState=Nil;
END

	void NullProc(void)
BEGIN
END

        void Default_InternSymbol(char *Asc, TempResult *Erg)
BEGIN
   UNUSED(Asc);

   Erg->Typ=TempNone;
END

	static char *GetString(void)
BEGIN
   return malloc(256*sizeof(char));
END

	void asmdef_init(void)
BEGIN
   int z;

   InitPassProc=NullProc;
   ClearUpProc=NullProc;
   FirstCPUDef=Nil;
   CPUCnt=0;
   SwitchFrom=NullProc;
   InternSymbol=Default_InternSymbol;

   DAsmCode=(LongWord *) malloc(MaxCodeLen/4);
   WAsmCode=(Word *) DAsmCode;
   BAsmCode=(Byte *) DAsmCode;

   RelaxedMode=True; ConstMode=ConstModeC;

   /* auf diese Weise wird PCSymbol defaultmaessig nicht erreichbar
      da das schon von den Konstantenparsern im Formelparser abgefangen
      wuerde */

   PCSymbol = "--PC--SYMBOL--";
   *DefCPU = '\0';

   for (z=0; z<=ParMax; z++) ArgStr[z]=GetString();
   SourceFile=GetString();
   ClrEol=GetString();
   CursUp=GetString();
   ErrorPath=GetString();
   ErrorName=GetString();
   OutName=GetString();
   CurrFileName=GetString();
   IncludeList=GetString();
   LstName=GetString();
   MacroName=GetString();
   MacProName=GetString();
   ShareName=GetString();
   LabPart=GetString();
   OpPart=GetString();
   AttrPart=GetString();
   ArgPart=GetString();
   CommPart=GetString();
   LOpPart=GetString();
   OneLine=GetString();
   ListLine=GetString();
   PrtInitString=GetString();
   PrtExitString=GetString();
   PrtTitleString=GetString();
   ExtendError=GetString();
END
