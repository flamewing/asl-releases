/* lang_DE/as.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* String-Definitionen fuer AS - deutsch                                     */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           19. 1.1997 Kommandozeilenoption U                               */
/*           21. 1.1997 Warnung nicht bitadressierbare Speicherstelle        */
/*           22. 1.1997 Fehler/Warnungen fuer Stacks                         */
/*            1. 2.1997 Warnung wegen NUL-Zeichen                            */
/*           29. 3.1997 Kommandozeilenoption g                               */
/*           30. 5.1997 Warnung wg. inkorrektem Listing                      */
/*           12. 7.1997 Kommandozeilenoption Y                               */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Fehlermeldungen */

#ifdef ERRMSG
#define ErrName             " : Fehler "
#define WarnName            " : Warnung "
#define InLineName          " in Zeile "

#define ErrMsgUselessDisp          "Displacement=0, "CH_ue"berfl"CH_ue"ssig"
#define ErrMsgShortAddrPossible    "Kurzadressierung m"CH_oe"glich"
#define ErrMsgShortJumpPossible    "kurzer Sprung m"CH_oe"glich"
#define ErrMsgNoShareFile          "kein Sharefile angelegt, SHARED ignoriert"
#define ErrMsgBigDecFloat          "FPU liest Wert evtl. nicht korrekt ein (>=1E1000)"
#define ErrMsgPrivOrder            "privilegierte Anweisung"
#define ErrMsgDistNull	           "Distanz 0 nicht bei Kurzsprung erlaubt (NOP erzeugt)"
#define ErrMsgWrongSegment         "Symbol aus falschem Segment"
#define ErrMsgInAccSegment         "Segment nicht adressierbar"
#define ErrMsgPhaseErr      	   CH_Ae"nderung des Symbolwertes erzwingt zus"CH_ae"tzlichen Pass"
#define ErrMsgOverlap	           ""CH_ue"berlappende Speicherbelegung"
#define ErrMsgNoCaseHit	           "keine CASE-Bedingung zugetroffen"
#define ErrMsgInAccPage            "Seite m"CH_oe"glicherweise nicht adressierbar"
#define ErrMsgRMustBeEven          "Registernummer mu"CH_sz" gerade sein"
#define ErrMsgObsolete	           "veralteter Befehl"
#define ErrMsgUnpredictable        "nicht vorhersagbare Ausf"CH_ue"hrung dieser Anweisung"
#define ErrMsgAlphaNoSense         "Lokaloperator au"CH_sz"erhalb einer Sektion "CH_ue"berfl"CH_ue"ssig"
#define ErrMsgSenseless            "sinnlose Operation"
#define ErrMsgRepassUnknown        "unbekannter Symbolwert erzwingt zus"CH_ae"tzlichen Pass"
#define ErrMsgAddrNotAligned       "Adresse nicht ausgerichtet"
#define ErrMsgIOAddrNotAllowed     "I/O-Adresse darf nicht verwendet werden"
#define ErrMsgPipeline             "m"CH_oe"gliche Pipelining-Effekte"
#define ErrMsgDoubleAdrRegUse      "mehrfache Adre"CH_sz"registerbenutzung in einer Anweisung"
#define ErrMsgNotBitAddressable    "Speicherstelle nicht bitadressierbar"
#define ErrMsgStackNotEmpty        "Stack ist nicht leer"
#define ErrMsgNULCharacter         "NUL-Zeichen in Strings, Ergebnis undefiniert"
#define ErrMsgDoubleDef            "Symbol doppelt definiert"
#define ErrMsgSymbolUndef          "Symbol nicht definiert"
#define ErrMsgInvSymName           "ung"CH_ue"ltiger Symbolname"
#define ErrMsgInvFormat            "ung"CH_ue"ltiges Format"
#define ErrMsgUseLessAttr	   ""CH_ue"berfl"CH_ue"ssiges Attribut"
#define ErrMsgUndefAttr            "undefiniertes Attribut"
#define ErrMsgTooLongAttr	   "Attribut darf nur 1 Zeichen lang sein"
#define ErrMsgWrongArgCnt	   "unpassende Operandenzahl"
#define ErrMsgWrongOptCnt	   "unpassende Optionszahl"
#define ErrMsgOnlyImmAddr	   "nur immediate-Adressierung erlaubt"
#define ErrMsgInvOpsize	           "unpassende Operandengr"CH_oe""CH_sz"e"
#define ErrMsgConfOpSizes          "widersprechende Operandengr"CH_oe""CH_sz"en"
#define ErrMsgUndefOpSizes	   "undefinierte Operandengr"CH_oe""CH_sz"e"
#define ErrMsgInvOpType	           "unpassender Operandentyp"
#define ErrMsgTooMuchArgs	   "zuviele Argumente"
#define ErrMsgUnknownOpcode        "unbekannter Befehl"
#define ErrMsgBrackErr	           "Klammerfehler"
#define ErrMsgDivByZero	           "Division durch 0"
#define ErrMsgUnderRange           "Bereichsunterschreitung"
#define ErrMsgOverRange	           "Bereichs"CH_ue"berschreitung"
#define ErrMsgNotAligned           "Adresse nicht ausgerichtet"
#define ErrMsgDistTooBig	   "Distanz zu gro"CH_sz""
#define ErrMsgInAccReg             "Register nicht zugreifbar"
#define ErrMsgNoShortAddr	   "Kurzadressierung nicht m"CH_oe"glich"
#define ErrMsgInvAddrMode	   "unerlaubter Adressierungsmodus"
#define ErrMsgMustBeEven           "Nummer mu"CH_sz" ausgerichtet sein"
#define ErrMsgInvParAddrMode	   "Adressierungsmodus im Parallelbetrieb nicht erlaubt"
#define ErrMsgUndefCond	           "undefinierte Bedingung"
#define ErrMsgJmpDistTooBig	   "Sprungdistanz zu gro"CH_sz""
#define ErrMsgDistIsOdd            "Sprungdistanz ist ungerade"
#define ErrMsgInvShiftArg	   "ung"CH_ue"ltiges Schiebeargument"
#define ErrMsgRange18	           "nur Bereich 1..8 erlaubt"
#define ErrMsgShiftCntTooBig	   "Schiebezahl zu gro"CH_sz""
#define ErrMsgInvRegList	   "ung"CH_ue"ltige Registerliste"
#define ErrMsgInvCmpMode	   "ung"CH_ue"ltiger Modus mit CMP"
#define ErrMsgInvCPUType	   "ung"CH_ue"ltiger Prozessortyp"
#define ErrMsgInvCtrlReg	   "ung"CH_ue"ltiges Kontrollregister"
#define ErrMsgInvReg	    	   "ung"CH_ue"ltiges Register"
#define ErrMsgNoSaveFrame          "RESTORE ohne SAVE"
#define ErrMsgNoRestoreFrame       "fehlendes RESTORE"
#define ErrMsgUnknownMacArg        "unbekannte Makro-Steueranweisung"
#define ErrMsgMissEndif            "fehlendes ENDIF/ENDCASE"
#define ErrMsgInvIfConst           "ung"CH_ue"ltiges IF-Konstrukt"
#define ErrMsgDoubleSection        "doppelter Sektionsname"
#define ErrMsgInvSection           "unbekannte Sektion"
#define ErrMsgMissingEndSect       "fehlendes ENDSECTION"
#define ErrMsgWrongEndSect         "falsches ENDSECTION"
#define ErrMsgNotInSection         "ENDSECTION ohne SECTION"
#define ErrMsgUndefdForward        "nicht aufgel"CH_oe"ste Vorw"CH_ae"rtsdeklaration"
#define ErrMsgContForward          "widersprechende FORWARD <-> PUBLIC-Deklaration"
#define ErrMsgInvFuncArgCnt 	   "falsche Argumentzahl f"CH_ue"r Funktion"
#define ErrMsgMissingLTORG         "unaufgel"CH_oe"ste Literale (LTORG fehlt)"
#define ErrMsgNotOnThisCPU1	   "Befehl auf dem "
#define ErrMsgNotOnThisCPU2        " nicht vorhanden"
#define ErrMsgNotOnThisCPU3	   "Adressierungsart auf dem "
#define ErrMsgInvBitPos	           "ung"CH_ue"ltige Bitstelle"
#define ErrMsgOnlyOnOff	           "nur ON/OFF erlaubt"
#define ErrMsgStackEmpty           "Stack ist leer oder nicht definiert"
#define ErrMsgShortRead            "vorzeitiges Dateiende"
#define ErrMsgRomOffs063	   "ROM-Offset geht nur von 0..63"
#define ErrMsgInvFCode	           "ung"CH_ue"ltiger Funktionscode"
#define ErrMsgInvFMask	           "ung"CH_ue"ltige Funktionscodemaske"
#define ErrMsgInvMMUReg	           "ung"CH_ue"ltiges MMU-Register"
#define ErrMsgLevel07	           "Level nur von 0..7"
#define ErrMsgInvBitMask	   "ung"CH_ue"ltige Bitmaske"
#define ErrMsgInvRegPair	   "ung"CH_ue"ltiges Registerpaar"
#define ErrMsgOpenMacro	           "offene Makrodefinition"
#define ErrMsgDoubleMacro          "doppelte Makrodefinition"
#define ErrMsgTooManyMacParams     "mehr als 10 Makroparameter"
#define ErrMsgEXITMOutsideMacro    "EXITM au"CH_sz"erhalb eines Makrorumpfes"
#define ErrMsgFirstPassCalc	   "Ausdruck mu"CH_sz" im ersten Pass berechenbar sein"
#define ErrMsgTooManyNestedIfs     "zu viele verschachtelte IFs"
#define ErrMsgMissingIf	           "ELSEIF/ENDIF ohne IF"
#define ErrMsgRekMacro	           "verschachtelter / rekursiver Makroaufruf"
#define ErrMsgUnknownFunc	   "unbekannte Funktion"
#define ErrMsgInvFuncArg	   "Funktionsargument au"CH_sz"erhalb Definitionsbereich"
#define ErrMsgFloatOverflow	   "Gleitkomma"CH_ue"berlauf"
#define ErrMsgInvArgPair	   "ung"CH_ue"ltiges Wertepaar"
#define ErrMsgNotOnThisAddress     "Befehl darf nicht auf dieser Adresse liegen"
#define ErrMsgNotFromThisAddress   "ung"CH_ue"ltiges Sprungziel"
#define ErrMsgTargOnDiffPage	   "Sprungziel nicht auf gleicher Seite"
#define ErrMsgCodeOverflow	   "Code"CH_ue"berlauf"
#define ErrMsgMixDBDS	           "Konstanten und Platzhalter nicht mischbar"
#define ErrMsgOnlyInCode	   "Codeerzeugung nur im Codesegment zul"CH_ae"ssig"
#define ErrMsgParNotPossible       "paralleles Konstrukt nicht m"CH_oe"glich"
#define ErrMsgAdrOverflow	   "Adre"CH_sz""CH_ue"berlauf"
#define ErrMsgInvSegment	   "ung"CH_ue"ltiges Segment"
#define ErrMsgUnknownSegment       "unbekanntes Segment"
#define ErrMsgUnknownSegReg        "unbekanntes Segmentregister"
#define ErrMsgInvString	           "ung"CH_ue"ltiger String"
#define ErrMsgInvRegName	   "ung"CH_ue"ltiger Registername"
#define ErrMsgInvArg               "ung"CH_ue"ltiges Argument"
#define ErrMsgNoIndir              "keine Indirektion erlaubt"
#define ErrMsgNotInThisSegment     "nicht im aktuellen Segment erlaubt"
#define ErrMsgNotInMaxmode         "nicht im Maximum-Modus zul"CH_ae"ssig"
#define ErrMsgOnlyInMaxmode        "nicht im Minimum-Modus zul"CH_ae"ssig"
#define ErrMsgOpeningFile          "Fehler beim "CH_Oe"ffnen der Datei"
#define ErrMsgListWrError	   "Listingschreibfehler"
#define ErrMsgFileReadError	   "Dateilesefehler"
#define ErrMsgFileWriteError	   "Dateischreibfehler"
#define ErrMsgIntError	           "interne(r) Fehler/Warnung"
#define ErrMsgHeapOvfl	           "Speicher"CH_ue"berlauf"
#define ErrMsgStackOvfl	           "Stapel"CH_ue"berlauf"

#define ErrMsgIsFatal	    "Fataler Fehler, Assembler abgebrochen"

#define ErrMsgOvlyError     "Overlayfehler - Programmabbruch"

#define PrevDefMsg          "vorherige Definition in"
#endif

/****************************************************************************/
/* Strings in Listingkopfzeile */

#define HeadingFileNameLab  " - Quelle "
#define HeadingPageLab      " - Seite "

/****************************************************************************/
/* Strings in Listing */

#define ListSymListHead1      "  Symboltabelle (*=unbenutzt):"
#define ListSymListHead2      "  ----------------------------"
#define ListSymSumMsg         " Symbol"
#define ListSymSumsMsg        " Symbole"
#define ListUSymSumMsg        " unbenutztes Symbol"
#define ListUSymSumsMsg       " unbenutzte Symbole"
#define ListMacListHead1      "  definierte Makros:"
#define ListMacListHead2      "  ------------------"
#define ListMacSumMsg         " Makro"
#define ListMacSumsMsg        " Makros"
#define ListFuncListHead1     "  definierte Funktionen:"
#define ListFuncListHead2     "  ----------------------"
#define ListDefListHead1      "  DEFINEs:"
#define ListDefListHead2      "  --------"
#define ListSegListHead1      "in "
#define ListSegListHead2      " belegte Bereiche:"
#define ListCrossListHead1    "  Querverweisliste:"
#define ListCrossListHead2    "  -----------------"
#define ListSectionListHead1  "  Sektionen:"
#define ListSectionListHead2  "  ----------"
#define ListIncludeListHead1  "  Include-Verschachtelung:"
#define ListIncludeListHead2  "  ------------------------"
#define ListCrossSymName      "Symbol "
#define ListCrossFileName     "Datei "

#define ListPlurName          "n"
#define ListHourName          " Stunde"
#define ListMinuName          " Minute"
#define ListSecoName          " Sekunde"

/****************************************************************************/
/* Durchsagen... */

#define InfoMessAssembling  "Assembliere "
#define InfoMessPass        "PASS "
#define InfoMessPass1       "PASS 1                             "
#define InfoMessPass2       "PASS 2                             "
#define InfoMessAssTime     " Assemblierzeit"
#define InfoMessAssLine     " Zeile Quelltext"
#define InfoMessAssLines    " Zeilen Quelltext"
#define InfoMessPassCnt     " Durchlauf"
#define InfoMessPPassCnt    " Durchl"CH_ae"ufe"
#define InfoMessNoPass      "        zus"CH_ae"tzliche erforderliche Durchl"CH_ae"ufe wegen Fehlern nicht\n        durchgef"CH_ue"hrt, Listing m"CH_oe"glicherweise inkorrekt"
#define InfoMessMacAssLine  " Zeile inkl. Makroexpansionen"
#define InfoMessMacAssLines " Zeilen inkl. Makroexpansionen"
#define InfoMessWarnCnt     " Warnung"
#define InfoMessWarnPCnt    "en"
#define InfoMessErrCnt      " Fehler"
#define InfoMessErrPCnt     ""
#define InfoMessRemainMem   " KByte verf"CH_ue"gbarer Restspeicher"
#define InfoMessRemainStack " Byte verf"CH_ue"gbarer Stack"

#define InfoMessNFilesFound ": keine Datei(en) zu assemblieren!"

#define InfoMessMacroAss    "Makroassembler "
#define InfoMessVar         "Version"

#ifdef MAIN
#define InfoMessHead1      "Aufruf : "
#define InfoMessHead2      " [Optionen] [Datei] [Optionen] ..."
#define InfoMessHelpCnt    32
static char *InfoMessHelp[InfoMessHelpCnt]=
		  {"--------",
		   "",
		   "Optionen :",
		   "----------",
		   "",
		   "-p : Sharefile im Pascal-Format       -c : Sharefile im C-Format",
		   "-a : Sharefile im AS-Format",
                   "-o <Name> : Namen der Code-Datei neu setzen",
                   "-q, -quiet : Stille "CH_Ue"bersetzung",
                   "-alias <neu>=<alt> : Prozessor-Alias definieren",
		   "-l : Listing auf Konsole              -L : Listing auf Datei",
		   "-i <Pfad>[:Pfad]... : Pfadliste f"CH_ue"r Includedateien",
		   "-D <Symbol>[,Symbol]... : Symbole vordefinieren",
		   "-E [Name] : Zieldatei f"CH_ue"r Fehlerliste,",
		   "            !0..!4 f"CH_ue"r Standardhandles",
		   "            Default <Quelldatei>.LOG",
                   "-r : Meldungen erzeugen, falls zus"CH_ae"tzlicher Pass erforderlich",
                   "-Y : Sprungfehlerunterdr"CH_ue"ckung (siehe Anleitung)",
                   "-w : Warnungen unterdr"CH_ue"cken           +G : Code-Erzeugung unterdr"CH_ue"cken",
                   "-s : Sektionsliste erzeugen           -t : Listing-Teile ein/ausblenden",
		   "-u : Belegungsliste erzeugen          -C : Querverweisliste erzeugen",
                   "-I : Include-Verschachtelungsliste ausgeben",
                   "-g : Debug-Informationen schreiben",
		   "-A : kompaktere Symbolablage",
		   "-U : Case-sensitiv arbeiten",
		   "-x : erweiterte Fehlermeldungen       -n : Fehlermeldungen mit Nummer",
		   "-P : Makroprozessorausgabe erzeugen   -M : Makrodefinitionen extrahieren",
		   "-h : Hexadezimalzahlen mit Kleinbuchstaben",
		   "",
		   "Quelldateiangabe darf Jokerzeichen enthalten",
		   "",
		   "implementierte Prozessoren :"};

#define KeyWaitMsg         "--- weiter mit <ENTER> ---"

#define ErrMsgInvParam     "ung"CH_ue"ltige Option: "
#define ErrMsgInvEnvParam  "ung"CH_ue"ltige Environment-Option: "

#define InvMsgSource       "Quelldatei?"
#endif
