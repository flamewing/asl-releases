{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei AS.RSC - enthÑlt Stringdefinitionen fÅr AS                   *}
{* 									    *}
{* Historie :20. 1.1993 Grundsteinlegung                                    *}
{*   			SPEEDUP-Symbol					    *}
{*           21. 1.1993 Fehlermeldungen                                     *}
{*           12. 6.1993 E/A-Fehlermeldungen                                 *}
{*            4. 9.1993 Lokalfehlermeldungen                                *}
{*           27.10.1993 Optionen G, M                                       *}
{*            1.11.1993 Erkennung Programmname                              *}
{*           14.11.1993 Warnung veraltete Anweisung                         *}
{*           15.11.1993 Option E                                            *}
{*           16. 1.1994 Meldungen nicht ausgerichtet, sinnlos               *}
{*           23. 1.1994 Meldungen wg. Repass                                *}
{*           20. 2.1994 Warnung Adressierung IO-Register	            *}
{*           15. 4.1994 zusÑtzliche Fehlerstrings                           *}
{*           24. 4.1994 Fehlermeldung ungÅltiges Format                     *}
{*	      3. 5.1994 Fehlermeldung doppeltes Makro                       *}
{*           24. 7.1994 Meldung freier Stack                                *}
{*           20. 8.1994 Kopfzeilen Define-Liste                             *}
{*           21. 8.1994 Meldung undefiniertes Attribut                      *}
{*            8. 9.1994 Meldung Parallelisierung geht nicht                 *}
{*           25. 9.1994 restliche Meldungen TMS320C3x                       *}
{*           16.10.1994 Meldung unaufgelîste Literale                       *}
{*           10.12.1994 Kommandozeilenoption o                              *}
{*           25.12.1994 Kommandozeilenoption q                              *}
{*           15. 6.1995 Meldung ungÅltige Optionszahl                       *}
{*           30.12.1995 Meldung vorherige Definition                        *}
{*           26. 1.1996 Kopfzeilen Includeliste                             *}
{*           24. 3.1996 Parameterausgabe vervollstÑndigt                    *}
{*            6. 9.1996 EXITM-Fehlermeldung                                 *}
{*           14.10.1996 BINCLUDE-Fehlermeldung                              *}
{*           19. 1.1997 Kommandozeilenparameter U                           *}
{*           21. 1.1997 Warnung nicht bitadressierbare Speicherstelle       *}
{*           22. 1.1997 Fehler/Warnungen fÅr Stacks                         *}
{*           25. 1.1997 Fehler fÅr Bitmaskenfunktion                        *}
{*            1. 2.1997 Warnung wegen NUL-Zeichen                           *}
{*           10. 2.1997 Warnung wegen SeitenÅberschreitung                  *}
{*           29. 3.1997 Kommandozeilenoption g                              *}
{*           29. 5.1997 Warnung wg. inkorrektem Listing                     *}
{*           12. 7.1997 Kommandozeilenoption Y                              *}
{*            5. 8.1997 Meldungen fÅr Strukturen                            *}
{*           23. 8.1997 Atmel-Debug-Format                                  *}
{*            7. 9.1997 Warnung BereichsÅberschreitung                      *}
{*           24. 9.1997 Kopfzeile Registerdefinitionsliste                  *}
{*           19.10.1997 Warnung neg. DUP-Anzahl                             *}
{*           11. 1.1998 C6x-Fehlermeldungen                                 *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ wegnehmen, falls reines Pascal gewÅnscht }

{****************************************************************************}
{ Fehlermeldungen }

CONST
   ErrName                  = ' : Fehler ';
   WarnName                 = ' : Warnung ';
   InLineName               = ' in Zeile ';

   ErrMsgUselessDisp        = 'Displacement=0,'+Ch_ue+'berfl'+Ch_ue+'ssig';
   ErrMsgShortAddrPossible  = 'Kurzadressierung m'+Ch_oe+'glich';
   ErrMsgShortJumpPossible  = 'kurzer Sprung m'+Ch_oe+'glich';
   ErrMsgNoShareFile        = 'kein Sharefile angelegt, SHARED ignoriert';
   ErrMsgBigDecFloat        = 'FPU liest Wert evtl. nicht korrekt ein (>=1E1000)';
   ErrMsgPrivOrder          = 'Privilegierte Anweisung';
   ErrMsgDistNull	    = 'Distanz 0 nicht bei Kurzsprung erlaubt (NOP erzeugt)';
   ErrMsgWrongSegment       = 'Symbol aus falschem Segment';
   ErrMsgInAccSegment       = 'Segment nicht adressierbar';
   ErrMsgPhaseErr           = Ch_gae+'nderung des Symbolwertes erzwingt zus'+Ch_ae+'tzlichen Pass';
   ErrMsgOverlap	    = Ch_ue+'berlappende Speicherbelegung';
   ErrMsgNoCaseHit	    = 'keine CASE-Bedingung zugetroffen';
   ErrMsgInAccPage          = 'Seite m'+Ch_oe+'glicherweise nicht adressierbar';
   ErrMsgRMustBeEven        = 'Registernummer mu'+Ch_sz+' gerade sein';
   ErrMsgObsolete	    = 'veralteter Befehl';
   ErrMsgUnpredictable      = 'Nicht vorhersagbare Ausf'+Ch_ue+'hrung dieser Anweisung';
   ErrMsgAlphaNoSense       = 'Lokaloperator au'+Ch_sz+'+erhalb einer Sektion '+Ch_ue+'berfl'+Ch_ue+'ssig';
   ErrMsgSenseless          = 'sinnlose Operation';
   ErrMsgRepassUnknown      = 'unbekannter Symbolwert erzwingt zus'+Ch_ae+'tzlichen Pass';
   ErrMsgAddrNotAligned     = 'Adresse nicht ausgerichtet';
   ErrMsgIOAddrNotAllowed   = 'I/O-Adresse darf nicht verwendet werden';
   ErrMsgPipeline           = 'm'+Ch_oe+'gliche Pipelining-Effekte';
   ErrMsgDoubleAdrRegUse    = 'mehrfache Adre'+Ch_sz+'registerbenutzung in einer Anweisung';
   ErrMsgNotBitAddressable  = 'Speicherstelle nicht bitadressierbar';
   ErrMsgStackNotEmpty      = 'Stack ist nicht leer';
   ErrMsgNULCharacter       = 'NUL-Zeichen in Strings, Ergebnis undefiniert';
   ErrMsgPageCrossing       = 'Befehl '+Ch_ue+'berschreitet Seitengrenze';
   ErrMsgWOverRange         = 'Bereichs'+Ch_ue+'berschreitung';
   ErrMsgNegDUP             = 'negatives Argument f'+Ch_ue+'r DUP';

   ErrMsgDoubleDef          = 'Symbol doppelt definiert';
   ErrMsgSymbolUndef        = 'Symbol nicht definiert';
   ErrMsgInvSymName	    = 'Ung'+Ch_ue+'ltiger Symbolname';
   ErrMsgInvFormat          = 'Ung'+Ch_ue+'ltiges Format';
   ErrMsgUselessAttr	    = Ch_ue+'berfl'+Ch_ue+'ssiges Attribut';
   ErrMsgUndefAttr          = 'Undefiniertes Attribut';
   ErrMsgTooLongAttr	    = 'Attribut darf nur 1 Zeichen lang sein';
   ErrMsgWrongArgCnt	    = 'Unpassende Operandenzahl';
   ErrMsgWrongOptCnt	    = 'Unpassende Optionszahl';
   ErrMsgOnlyImmAddr	    = 'Nur immediate-Adressierung erlaubt';
   ErrMsgInvOpsize	    = 'Unpassende Operandengr'+Ch_oe+Ch_sz+'e';
   ErrMsgConfOpSizes        = 'Widersprechende Operandengr'+Ch_oe+Ch_sz+'en';
   ErrMsgUndefOpSizes	    = 'Undefinierte Operandengr'+Ch_oe+Ch_sz+'e';
   ErrMsgInvOpType	    = 'Unpassender Operandentyp';
   ErrMsgTooMuchArgs	    = 'Zuviele Argumente';
   ErrMsgUnknownOpcode	    = 'Unbekannter Befehl';
   ErrMsgBrackErr	    = 'Klammerfehler';
   ErrMsgDivByZero	    = 'Division durch 0';
   ErrMsgUnderRange         = 'Bereichsunterschreitung';
   ErrMsgOverRange	    = 'Bereichs'+Ch_ue+'berschreitung';
   ErrMsgNotAligned         = 'Adresse nicht ausgerichtet';
   ErrMsgDistTooBig	    = 'Distanz zu gro'+Ch_sz;
   ErrMsgInAccReg           = 'Register nicht zugreifbar';
   ErrMsgNoShortAddr	    = 'Kurzadressierung nicht m'+Ch_oe+'glich';
   ErrMsgInvAddrMode	    = 'Unerlaubter Adressierungsmodus';
   ErrMsgMustBeEven         = 'Nummer mu'+Ch_sz+' ausgerichtet sein';
   ErrMsgInvParAddrMode	    = 'Adressierungsmodus im Parallelbetrieb nicht erlaubt';
   ErrMsgUndefCond	    = 'Undefinierte Bedingung';
   ErrMsgJmpDistTooBig	    = 'Sprungdistanz zu gro'+Ch_sz;
   ErrMsgDistIsOdd          = 'Sprungdistanz ist ungerade';
   ErrMsgInvShiftArg	    = 'Ung'+Ch_ue+'ltiges Schiebeargument';
   ErrMsgRange18	    = 'nur Bereich 1..8 erlaubt';
   ErrMsgShiftCntTooBig	    = 'Schiebezahl zu gro'+Ch_sz;
   ErrMsgInvRegList	    = 'Ung'+Ch_ue+'ltige Registerliste';
   ErrMsgInvCmpMode	    = 'Ung'+Ch_ue+'ltiger Modus mit CMP';
   ErrMsgInvCPUType	    = 'Ung'+Ch_ue+'ltiger Prozessortyp';
   ErrMsgInvCtrlReg	    = 'Ung'+Ch_ue+'ltiges Kontrollregister';
   ErrMsgInvReg	    	    = 'Ung'+Ch_ue+'ltiges Register';
   ErrMsgNoSaveFrame        = 'RESTORE ohne SAVE';
   ErrMsgNoRestoreFrame     = 'fehlendes RESTORE';
   ErrMsgUnknownMacArg      = 'unbekannte Makro-Steueranweisung';
   ErrMsgMissEndif          = 'fehlendes ENDIF/ENDCASE';
   ErrMsgInvIfConst         = 'ung'+Ch_ue+'ltiges IF-Konstrukt';
   ErrMsgDoubleSection      = 'doppelter Sektionsname';
   ErrMsgInvSection         = 'unbekannte Sektion';
   ErrMsgMissingEndSect     = 'fehlendes ENDSECTION';
   ErrMsgWrongEndSect       = 'falsches ENDSECTION';
   ErrMsgNotInSection       = 'ENDSECTION ohne SECTION';
   ErrMsgUndefdForward      = 'nicht aufgel'+Ch_oe+'ste Vorw'+Ch_ae+'rtsdeklaration';
   ErrMsgContForward        = 'widersprechende FORWARD <-> PUBLIC-Deklaration';
   ErrMsgInvFuncArgCnt 	    = 'falsche Argumentzahl f'+Ch_ue+'r Funktion';
   ErrMsgMissingLTORG       = 'unaufgel'+Ch_oe+'ste Literale (LTORG fehlt)';
   ErrMsgNotOnThisCPU1	    = 'Befehl auf dem ';
   ErrMsgNotOnThisCPU2      = ' nicht vorhanden';
   ErrMsgNotOnThisCPU3	    = 'Adressierungsart auf dem ';
   ErrMsgInvBitPos	    = 'Ung'+Ch_ue+'ltige Bitstelle';
   ErrMsgOnlyOnOff	    = 'Nur ON/OFF erlaubt';
   ErrMsgStackEmpty         = 'Stack ist leer oder nicht definiert';
   ErrMsgNotOneBit          = 'Nicht genau ein Bit gesetzt';
   ErrMsgMissingStruct      = 'ENDSTRUCT ohne STRUCT';
   ErrMsgOpenStruct         = 'offene Strukturdefinition';
   ErrMsgWrongStruct        = 'falsches ENDSTRUCT';
   ErrMsgPhaseDisallowed    = 'Phasendefinition nicht in Strukturen erlaubt';
   ErrMsgInvStructDir       = 'Ung'+Ch_ue+'ltige STRUCT-Direktive';
   ErrMsgShortRead          = 'vorzeitiges Dateiende';
   ErrMsgRomOffs063	    = 'Rom-Offset geht nur von 0..63';
   ErrMsgInvFCode	    = 'Ung'+Ch_ue+'ltiger Funktionscode';
   ErrMsgInvFMask	    = 'Ung'+Ch_ue+'ltige Funktionscodemaske';
   ErrMsgInvMMUReg	    = 'Ung'+Ch_ue+'ltiges MMU-Register';
   ErrMsgLevel07	    = 'Level nur von 0..7';
   ErrMsgInvBitMask	    = 'ung'+Ch_ue+'ltige Bitmaske';
   ErrMsgInvRegPair	    = 'ung'+Ch_ue+'ltiges Registerpaar';
   ErrMsgOpenMacro	    = 'Offene Makrodefinition';
   ErrMsgDoubleMacro        = 'Doppelte Makrodefinition';
   ErrMsgTooManyMacParams   = 'mehr als 10 Makroparameter';
   ErrMsgEXITMOutsideMacro  = 'EXITM au'+Ch_sz+'erhalb eines Makrorumpfes';
   ErrMsgFirstPassCalc	    = 'Ausdruck mu'+Ch_sz+' im ersten Pass berechenbar sein';
   ErrMsgTooManyNestedIfs   = 'zu viele verschachtelte IFs';
   ErrMsgMissingIf	    = 'ELSEIF/ENDIF ohne IF';
   ErrMsgRekMacro	    = 'verschachtelter / rekursiver Makroaufruf';
   ErrMsgUnknownFunc	    = 'unbekannte Funktion';
   ErrMsgInvFuncArg	    = 'Funktionsargument au'+Ch_sz+'erhalb Definitionsbereich';
   ErrMsgFloatOverflow	    = 'Gleitkomma'+Ch_ue+'berlauf';
   ErrMsgInvArgPair	    = 'Ung'+Ch_gue+'ltiges Wertepaar';
   ErrMsgNotOnThisAddress   = 'Befehl darf nicht auf dieser Adresse liegen';
   ErrMsgNotFromThisAddress = 'ung'+Ch_ue+'ltiges Sprungziel';
   ErrMsgTargOnDiffPage	    = 'Sprungziel nicht auf gleicher Seite';
   ErrMsgCodeOverflow	    = 'Code'+Ch_ue+'berlauf';
   ErrMsgMixDBDS	    = 'Konstanten und Platzhalter nicht mischbar';
   ErrMsgNotInStruct        = 'Codeerzeugung in Strukturdefinition nicht zul'+Ch_ae+'ssig';
   ErrMsgParNotPossible     = 'Paralleles Konstrukt nicht m'+Ch_oe+'glich';
   ErrMsgAdrOverflow	    = 'Adre'+Ch_sz+Ch_ue+'berlauf';
   ErrMsgInvSegment	    = 'ung'+Ch_ue+'ltiges Segment';
   ErrMsgUnknownSegment     = 'unbekanntes Segment';
   ErrMsgUnknownSegReg      = 'unbekanntes Segmentregister';
   ErrMsgInvString	    = 'ung'+Ch_ue+'ltiger String';
   ErrMsgInvRegName	    = 'ung'+Ch_ue+'ltiger Registername';
   ErrMsgInvArg             = 'ung'+Ch_ue+'ltiges Argument';
   ErrMsgNoIndir            = 'keine Indirektion erlaubt';
   ErrMsgNotInThisSegment   = 'nicht im aktuellen Segment erlaubt';
   ErrMsgNotInMaxmode       = 'nicht im Maximum-Modus zul'+Ch_ae+'ssig';
   ErrMsgOnlyInMaxmode      = 'nicht im Minimum-Modus zul'+Ch_ae+'ssig';
   ErrMsgPacketNotAligned   = 'Instruktionspaket '+Ch_ue+'berschreitet 8-Wort-Grenze';
   ErrMsgDoubleUnitUsed     = 'Funktionseinheit doppelt benutzt';
   ErrMsgLongReads          = 'mehr als ein langer Leseoperand';
   ErrMsgLongWrites         = 'mehr als ein langer Schreiboperand';
   ErrMsgLongMem            = 'langer Leseoperand mit Speicherzugriff';
   ErrMsgTooManyReads       = 'zu viele Lesezugriffe auf Register';
   ErrMsgWriteConflict      = 'Schreibkonflikt auf Register';
   ErrMsgOpeningFile	    = 'Fehler beim '+Ch_goe+'ffnen der Datei';
   ErrMsgListWrError	    = 'Listingschreibfehler';
   ErrMsgFileReadError	    = 'Dateilesefehler';
   ErrMsgFileWriteError	    = 'Dateischreibfehler';
   ErrMsgIntError	    = 'Interne(r) Fehler/Warnung';
   ErrMsgHeapOvfl	    = 'Speicher'+Ch_ue+'berlauf';
   ErrMsgStackOvfl	    = 'Stapel'+Ch_ue+'berlauf';

   ErrMsgIsFatal	    = 'Fataler Fehler, Assembler abgebrochen';

   ErrMsgOvlyError    	    = 'Overlayfehler - Programmabbruch';

   PrevDefMsg               = 'vorherige Definition in';

{$IFDEF DPMI}
   ErrMsgInvSwapSize        = 'ung'+Ch_ue+'ltige Gr+'+Ch_oe+Ch_sz+'enangabe f'+Ch_ue+'r Swapfile - Programmabbruch';
   ErrMsgSwapTooBig         = 'zuwenig Platz f'+Ch_ue+'r Swapfile - Programmabbruch';
{$ENDIF}

{****************************************************************************}
{ Strings in Listingkopfzeile }

   HeadingFileNameLab = ' - Quelle ';
   HeadingPageLab     = ' - Seite ';

{****************************************************************************}
{ Strings in Listing }

   ListSymListHead1     = '  Symboltabelle (*=unbenutzt):';
   ListSymListHead2     = '  ----------------------------';
   ListSymSumMsg        = ' Symbol';
   ListSymSumsMsg       = ' Symbole';
   ListUSymSumMsg       = ' unbenutztes Symbol';
   ListUSymSumsMsg      = ' unbenutzte Symbole';
   ListRegDefListHead1  = '  Registerdefinitionen (*=unbenutzt):';
   ListRegDefListHead2  = '  ------------------------------------';
   ListRegDefSumMsg     = ' Definition';
   ListRegDefSumsMsg    = ' Definitionen';
   ListRegDefUSumMsg    = ' unbenutztes Definition';
   ListRegDefUSumsMsg   = ' unbenutzte Definitionen';
   ListMacListHead1     = '  definierte Makros:';
   ListMacListHead2     = '  ------------------';
   ListMacSumMsg        = ' Makro';
   ListMacSumsMsg       = ' Makros';
   ListFuncListHead1    = '  definierte Funktionen:';
   ListFuncListHead2    = '  ----------------------';
   ListDefListHead1     = '  DEFINEs:';
   ListDefListHead2     = '  --------';
   ListSegListHead1     = 'in ';
   ListSegListHead2     = ' belegte Bereiche:';
   ListCrossListHead1   = '  Querverweisliste:';
   ListCrossListHead2   = '  -----------------';
   ListSectionListHead1 = '  Sektionen:';
   ListSectionListHead2 = '  ----------';
   ListIncludeListHead1 = '  Include-Verschachtelung:';
   ListIncludeListHead2 = '  ------------------------';
   ListCrossSymName     = 'Symbol ';
   ListCrossFileName    = 'Datei ';

   ListPlurName         = 'n';
   ListHourName         = ' Stunde';
   ListMinuName         = ' Minute';
   ListSecoName         = ' Sekunde';

{****************************************************************************}
{ Durchsagen... }

   InfoMessAssembling = 'Assembliere ';
   InfoMessPass       = 'PASS ';
   InfoMessPass1      = 'PASS 1                             ';
   InfoMessPass2      = 'PASS 2                             ';
   InfoMessAssTime    = ' Assemblierzeit';
   InfoMessAssLine    = ' Zeile Quelltext';
   InfoMessAssLines   = ' Zeilen Quelltext';
   InfoMessPassCnt    = ' Durchlauf';
   InfoMessPPassCnt   = ' Durchl'+Ch_ae+'ufe';
   InfoMessNoPass     = '        zus'+Ch_ae+'tzliche erforderliche Durchl'+Ch_ae+'ufe wegen Fehlern nicht'+#13+#10+
                        '        durchgef'+Ch_ue+'hrt, Listing m'+Ch_oe+'glicherweise inkorrekt';
   InfoMessMacAssLine = ' Zeile inkl. Makroexpansionen';
   InfoMessMacAssLines= ' Zeilen inkl. Makroexpansionen';
   InfoMessWarnCnt    = ' Warnung';
   InfoMessWarnPCnt   = 'en';
   InfoMessErrCnt     = ' Fehler';
   InfoMessErrPCnt    = '';
   InfoMessRemainMem  = ' KByte verf'+Ch_ue+'gbarer Restspeicher';
   InfoMessRemainStack= ' Byte verf'+Ch_ue+'gbarer Stack';

   InfoMessNFilesFound= ': keine Datei(en) zu assemblieren!';

{$IFDEF OS2}
   InfoMessMacroAss   = 'Makroassembler/2 ';
{$ELSE}
   InfoMessMacroAss   = 'Makroassembler ';
{$ENDIF}
   InfoMessVar        = 'Version';

{$IFDEF MAIN}
   InfoMessHead1      = 'Aufruf : ';
   InfoMessHead2      = ' [Optionen] [Datei] [Optionen] ...';
   InfoMessHelpCnt    = 32;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
		  ('--------',
		   '',
		   'Optionen :',
		   '----------',
		   '',
		   '-p : Sharefile im Pascal-Format       -c : Sharefile im C-Format',
		   '-a : Sharefile im AS-Format',
                   '-o <Name> : Namen der Code-Datei neu setzen',
                   '-q, -quiet : Stille '+Ch_gue+'bersetzung',
                   '-alias <neu>=<alt> : Prozessor-Alias definieren',
		   '-l : Listing auf Konsole              -L : Listing auf Datei',
		   '-i <Pfad>[;Pfad]... : Pfadliste f'+Ch_ue+'r Includedateien',
		   '-D <Symbol>[,Symbol]... : Symbole vordefinieren',
		   '-E [Name] : Zieldatei f'+Ch_ue+'r Fehlerliste,',
		   '            !0..!4 f'+Ch_ue+'r Standardhandles',
		   '            Default <Quelldatei>.LOG',
                   '-r : Meldungen erzeugen, falls zus'+Ch_ae+'tzlicher Pass erforderlich',
                   '-Y : SprungfehlerunterdrÅckung (siehe Anleitung)',
                   '-w : Warnungen unterdr'+Ch_ue+'cken           +G : Code-Erzeugung unterdr'+Ch_ue+'cken',
                   '-s : Sektionsliste erzeugen           -t : Listing-Teile ein/ausblenden',
		   '-u : Belegungsliste erzeugen          -C : Querverweisliste erzeugen',
                   '-I : Include-Verschachtelungsliste ausgeben',
                   '-g : Debug-Informationen schreiben [MAP/ATMEL]',
                   '-A : kompaktere Symbolablage',
                   '-U : Case-sensitiv arbeiten',
		   '-x : erweiterte Fehlermeldungen       -n : Fehlermeldungen mit Nummer',
		   '-P : Makroprozessorausgabe erzeugen   -M : Makrodefinitionen extrahieren',
		   '-h : Hexadezimalzahlen mit Kleinbuchstaben',
		   '',
		   'Quelldateiangabe darf Jokerzeichen enthalten',
		   '',
		   'implementierte Prozessoren :');

   KeyWaitMsg         = '--- weiter mit <ENTER> ---';

   ErrMsgInvParam     = 'ung'+Ch_ue+'ltige Option: ';
   ErrMsgInvEnvParam  = 'ung'+Ch_ue+'ltige Environment-Option: ';

   InvMsgSource       = 'Quelldatei?';

{$ENDIF}
