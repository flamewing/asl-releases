;* as.res
;*****************************************************************************
;* AS-Portierung                                                             *
;*                                                                           *
;* String-Definitionen fuer AS                                               *
;*                                                                           *
;* Historie:  4. 5.1996 Grundsteinlegung                                     *
;*           19. 1.1997 Kommandozeilenoption U                               *
;*           21. 1.1997 Warnung nicht bitadressierbare Speicherstelle        *
;*           22. 1.1997 Fehler;Warnungen fuer Stacks                         *
;*            1. 2.1997 Warnung wegen NUL-Zeichen                            *
;*           29. 3.1997 Kommandozeilenoption g                               *
;*           30. 5.1997 Warnung wg. inkorrektem Listing                      *
;*           12. 7.1997 Kommandozeilenoption Y                               *
;*            5. 8.1997 Meldungen fuer Strukturen                            *
;*            7. 9.1997 Warnung Bereichsueberschreitung                      *
;*           24. 9.1997 Kopfzeile Registerdefinitionsliste                   *
;*           19.10.1997 Warnung neg. DUP-Anzahl                              *
;*           26. 6.1998 Fehlermeldung Codepage nicht gefunden                *
;*           27. 6.1998 Meldungen für Codepage-Liste                         *
;*           18. 4.1999 Kommandozeilenoptionen cpu, shareout                 *
;*            2. 5.1999 'order' --> 'instruction'                            *
;*           13. 7.1999 Fehlermeldungen fuer extern-Symbole                  *
;*           13. 2.2000 Kommandozeilenoption olist                           *
;*            1. 6.2000 changed error message 1850                           *
;*           21. 7.2001 not repeatable error message                         *
;*           2001-10-03 warning implicit X-conversion                        *
;*                                                                           *
;*****************************************************************************

Include header.res

;-----------------------------------------------------------------------------
; Fehlermeldungen

Message ErrName
 ": Fehler "
 ": error "

Message WarnName
 ": Warnung "
 ": warning "

Message InLineName
 " in Zeile "
 " in line "

Message ErrMsgUselessDisp
 "Displacement=0, &uuml;berfl&uuml;ssig"
 "useless displacement 0"

Message ErrMsgShortAddrPossible
 "Kurzadressierung m&ouml;glich"
 "short addressing possible"

Message ErrMsgShortJumpPossible
 "kurzer Sprung m&ouml;glich"
 "short jump possible"

Message ErrMsgNoShareFile
 "kein Sharefile angelegt, SHARED ignoriert"
 "no sharefile created, SHARED ignored"

Message ErrMsgBigDecFloat
 "FPU liest Wert evtl. nicht korrekt ein (>=1E1000)"
 "FPU possibly cannot read this value (> 1E1000)"

Message ErrMsgPrivOrder
 "privilegierte Anweisung"
 "privileged instruction"

Message ErrMsgDistNull
 "Distanz 0 nicht bei Kurzsprung erlaubt (NOP erzeugt)"
 "distance of 0 not allowed for short jump (NOP created instead)"

Message ErrMsgWrongSegment
 "Symbol aus falschem Segment"
 "symbol out of wrong segment"

Message ErrMsgInAccSegment
 "Segment nicht adressierbar"
 "segment not accessible"

Message ErrMsgPhaseErr
 "&Auml;nderung des Symbolwertes erzwingt zus&auml;tzlichen Pass"
 "change of symbol values forces additional pass"

Message ErrMsgOverlap
 "&uuml;berlappende Speicherbelegung"
 "overlapping memory usage"

Message ErrMsgNoCaseHit
 "keine CASE-Bedingung zugetroffen"
 "none of the CASE conditions was true"

Message ErrMsgInAccPage
 "Seite m&ouml;glicherweise nicht adressierbar"
 "page might not be addressable"

Message ErrMsgRMustBeEven
 "Registernummer mu&szlig; gerade sein"
 "register number must be even"

Message ErrMsgObsolete
 "veralteter Befehl"
 "obsolete instruction, usage discouraged"

Message ErrMsgUnpredictable
 "nicht vorhersagbare Ausf&uuml;hrung dieser Anweisung"
 "unpredictable execution of this instruction"

Message ErrMsgAlphaNoSense
 "Lokaloperator au&szlig;erhalb einer Sektion &uuml;berfl&uuml;ssig"
 "localization operator senseless out of a section"

Message ErrMsgSenseless
 "sinnlose Operation"
 "senseless instruction"

Message ErrMsgRepassUnknown
 "unbekannter Symbolwert erzwingt zus&auml;tzlichen Pass"
 "unknown symbol value forces additional pass"

Message ErrMsgAddrNotAligned
 "Adresse nicht ausgerichtet"
 "address is not properly aligned"

Message ErrMsgIOAddrNotAllowed
 "I/O-Adresse darf nicht verwendet werden"
 "I/O-address must not be used here"

Message ErrMsgPipeline
 "m&ouml;gliche Pipelining-Effekte"
 "possible pipelining effects"

Message ErrMsgDoubleAdrRegUse
 "mehrfache Adre&szlig;registerbenutzung in einer Anweisung"
 "multiple use of address register in one instruction"

Message ErrMsgNotBitAddressable
 "Speicherstelle nicht bitadressierbar"
 "memory location is not bit addressable"

Message ErrMsgStackNotEmpty
 "Stack ist nicht leer"
 "stack is not empty"

Message ErrMsgNULCharacter
 "NUL-Zeichen in String, Ergebnis undefiniert"
 "NUL character in string, result is undefined"

Message ErrMsgPageCrossing
 "Befehl &uuml;berschreitet Seitengrenze"
 "instruction crosses page boundary" 

Message ErrMsgWOverRange
 "Bereichs&uuml;berschreitung"
 "range overflow"

Message ErrMsgNegDUP
 "negatives Argument f&uuml;r DUP"
 "negative argument for DUP"

Message ErrMsgConvIndX
 "einzelner X-Operand wird als indizierte und nicht als implizite Adressierung interpretiert"
 "single X operand interpreted as indexed and not implicit addressing"

;*****

Message ErrMsgDoubleDef
 "Symbol doppelt definiert"
 "symbol double defined"

Message ErrMsgSymbolUndef
 "Symbol nicht definiert"
 "symbol undefined"

Message ErrMsgInvSymName
 "ung&uuml;ltiger Symbolname"
 "invalid symbol name"

Message ErrMsgInvFormat
 "ung&uuml;ltiges Format"
  "invalid format"

Message ErrMsgUseLessAttr
 "&uuml;berfl&uuml;ssiges Attribut"
 "useless attribute"

Message ErrMsgUndefAttr
 "undefiniertes Attribut"
 "undefined attribute"

Message ErrMsgTooLongAttr
 "Attribut darf nur 1 Zeichen lang sein"
 "attribute may only be one character long"

Message ErrMsgWrongArgCnt
 "unpassende Operandenzahl"
 "wrong number of operands"

Message ErrMsgWrongOptCnt
 "unpassende Optionszahl"
 "wrong number of options"

Message ErrMsgOnlyImmAddr
 "nur immediate-Adressierung erlaubt"
 "addressing mode must be immediate"

Message ErrMsgInvOpsize
 "unpassende Operandengr&ouml;&szlig;e"
 "invalid operand size"

Message ErrMsgConfOpSizes
 "widersprechende Operandengr&ouml;&szlig;en"
 "conflicting operand sizes"

Message ErrMsgUndefOpSizes
 "undefinierte Operandengr&ouml;&szlig;e"
 "undefined operand size"

Message ErrMsgInvOpType
 "unpassender Operandentyp"
 "invalid operand type"

Message ErrMsgTooMuchArgs
 "zuviele Argumente"
 "too many arguments"

Message ErrMsgUnknownOpcode
 "unbekannter Befehl"
 "unknown opcode"

Message ErrMsgBrackErr
 "Klammerfehler"
 "number of opening/closing parentheses does not match"

Message ErrMsgDivByZero
 "Division durch 0"
 "division by 0"

Message ErrMsgUnderRange
 "Bereichsunterschreitung"
 "range underflow"

Message ErrMsgOverRange
 "Bereichs&uuml;berschreitung"
 "range overflow"

Message ErrMsgNotAligned
 "Adresse nicht ausgerichtet"
 "address is not properly aligned"

Message ErrMsgDistTooBig
 "Distanz zu gro&szlig;"
 "distance too big"

Message ErrMsgInAccReg
 "Register nicht zugreifbar"
 "register not accessible"

Message ErrMsgNoShortAddr
 "Kurzadressierung nicht m&ouml;glich"
 "short addressing not allowed"

Message ErrMsgInvAddrMode
 "unerlaubter Adressierungsmodus"
 "addressing mode not allowed here"

Message ErrMsgMustBeEven
 "Nummer mu&szlig; ausgerichtet sein"
 "number must be aligned"

Message ErrMsgInvParAddrMode
 "Adressierungsmodus im Parallelbetrieb nicht erlaubt"
 "addressing mode not allowed in parallel operation"

Message ErrMsgUndefCond
 "undefinierte Bedingung"
 "undefined condition"

Message ErrMsgJmpDistTooBig
 "Sprungdistanz zu gro&szlig;"
 "jump distance too big"

Message ErrMsgDistIsOdd
 "Sprungdistanz ist ungerade"
 "jump distance is odd"

Message ErrMsgInvShiftArg
 "ung&uuml;ltiges Schiebeargument"
 "invalid argument for shifting"

Message ErrMsgRange18
 "nur Bereich 1..8 erlaubt"
 "operand must be in range 1..8"

Message ErrMsgShiftCntTooBig
 "Schiebezahl zu gro&szlig;"
 "shift amplitude too big"

Message ErrMsgInvRegList
 "ung&uuml;ltige Registerliste"
 "invalid register list"

Message ErrMsgInvCmpMode
 "ung&uuml;ltiger Modus mit CMP"
 "invalid addressing mode for CMP"

Message ErrMsgInvCPUType
 "ung&uuml;ltiger Prozessortyp"
 "invalid CPU type"

Message ErrMsgInvCtrlReg
 "ung&uuml;ltiges Kontrollregister"
 "invalid control register"

Message ErrMsgInvReg
 "ung&uuml;ltiges Register"
 "invalid register"

Message ErrMsgNoSaveFrame
 "RESTORE ohne SAVE"
 "RESTORE without SAVE"

Message ErrMsgNoRestoreFrame
 "fehlendes RESTORE"
 "missing RESTORE"

Message ErrMsgUnknownMacArg
 "unbekannte Makro-Steueranweisung"
 "unknown macro control instruction"

Message ErrMsgMissEndif
 "fehlendes ENDIF/ENDCASE"
 "missing ENDIF/ENDCASE"

Message ErrMsgInvIfConst
 "ung&uuml;ltiges IF-Konstrukt"
 "invalid IF-structure"

Message ErrMsgDoubleSection
 "doppelter Sektionsname"
 "section name double defined"

Message ErrMsgInvSection
 "unbekannte Sektion"
 "unknown section"

Message ErrMsgMissingEndSect
 "fehlendes ENDSECTION"
 "missing ENDSECTION"

Message ErrMsgWrongEndSect
 "falsches ENDSECTION"
 "wrong ENDSECTION"

Message ErrMsgNotInSection
 "ENDSECTION ohne SECTION"
 "ENDSECTION without SECTION"

Message ErrMsgUndefdForward
 "nicht aufgel&ouml;ste Vorw&auml;rtsdeklaration"
 "unresolved forward declaration"

Message ErrMsgContForward
 "widersprechende FORWARD <-> PUBLIC-Deklaration"
 "conflicting FORWARD <-> PUBLIC-declaration"

Message ErrMsgInvFuncArgCnt
 "falsche Argumentzahl f&uuml;r Funktion"
 "wrong numbers of function arguments"

Message ErrMsgMissingLTORG
 "unaufgel&ouml;ste Literale (LTORG fehlt)"
 "unresolved literals (missing LTORG)"

Message ErrMsgNotOnThisCPU1
 "Befehl auf dem "
 "instruction not allowed on "

Message ErrMsgNotOnThisCPU2
 " nicht vorhanden"
 ""

Message ErrMsgNotOnThisCPU3
 "Adressierungsart auf dem "
 "addressing mode not allowed on "

Message ErrMsgInvBitPos
 "ung&uuml;ltige Bitstelle"
 "invalid bit position"

Message ErrMsgOnlyOnOff
 "nur ON/OFF erlaubt"
 "only ON/OFF allowed"

Message ErrMsgStackEmpty
 "Stack ist leer oder nicht definiert"
 "stack is empty or undefined"

Message ErrMsgNotOneBit
 "Nicht genau ein Bit gesetzt"
 "not exactly one bit set"

Message ErrMsgMissingStruct
 "ENDSTRUCT ohne STRUCT"
 "ENDSTRUCT without STRUCT"

Message ErrMsgOpenStruct
 "offene Strukturdefinition"
 "open structure definition"

Message ErrMsgWrongStruct
 "falsches ENDSTRUCT"
 "wrong ENDSTRUCT"

Message ErrMsgPhaseDisallowed
 "Phasendefinition nicht in Strukturen erlaubt"
 "phase definition not allowed in structure definition"

Message ErrMsgInvStructDir
 "Ung&uuml;ltige STRUCT-Direktive"
 "invalid STRUCT directive"

Message ErrMsgNotRepeatable
 "Anweisung nicht wiederholbar"
 "Instruction is not repeatable"

Message ErrMsgShortRead
 "vorzeitiges Dateiende"
 "unexpected end of file"

Message ErrMsgUnknownCodepage
 "unbekannte Zeichentabelle"
 "unknown codepage"

Message ErrMsgRomOffs063
 "ROM-Offset geht nur von 0..63"
 "ROM-offset must be in range 0..63"

Message ErrMsgInvFCode
 "ung&uuml;ltiger Funktionscode"
 "invalid function code"

Message ErrMsgInvFMask
 "ung&uuml;ltige Funktionscodemaske"
 "invalid function code mask"

Message ErrMsgInvMMUReg
 "ung&uuml;ltiges MMU-Register"
 "invalid MMU register"

Message ErrMsgLevel07
 "Level nur von 0..7"
 "level must be in range 0..7"

Message ErrMsgInvBitMask
 "ung&uuml;ltige Bitmaske" 
 "invalid bit mask"

Message ErrMsgInvRegPair
 "ung&uuml;ltiges Registerpaar"
 "invalid register pair"

Message ErrMsgOpenMacro
 "offene Makrodefinition"
 "open macro definition"

Message ErrMsgDoubleMacro
 "doppelte Makrodefinition"
 "macro double defined"

Message ErrMsgTooManyMacParams
 "mehr als 10 Makroparameter"
 "more than 10 macro parameters"

Message ErrMsgEXITMOutsideMacro
 "EXITM au&szlig;erhalb eines Makrorumpfes"
 "EXITM not called from within macro"

Message ErrMsgFirstPassCalc
 "Ausdruck mu&szlig; im ersten Pass berechenbar sein"
 "expression must be evaluatable in first pass"

Message ErrMsgTooManyNestedIfs
 "zu viele verschachtelte IFs"
 "too many nested IFs"

Message ErrMsgMissingIf
 "ELSEIF/ENDIF ohne IF"
 "ELSEIF/ENDIF without IF"

Message ErrMsgRekMacro
 "zu tief verschachtelter/rekursiver Makroaufruf"
 "too deeply nested/recursive makro call"

Message ErrMsgUnknownFunc
 "unbekannte Funktion"
 "unknown function"

Message ErrMsgInvFuncArg
 "Funktionsargument au&szlig;erhalb Definitionsbereich"
 "function argument out of definition range"

Message ErrMsgFloatOverflow
 "Gleitkomma&uuml;berlauf"
 "floating point overflow"

Message ErrMsgInvArgPair
 "ung&uuml;ltiges Wertepaar"
 "invalid value pair"

Message ErrMsgNotOnThisAddress
 "Befehl darf nicht auf dieser Adresse liegen"
 "instruction must not start on this address"

Message ErrMsgNotFromThisAddress
 "ung&uuml;ltiges Sprungziel"
 "invalid jump target"

Message ErrMsgTargOnDiffPage
 "Sprungziel nicht auf gleicher Seite"
 "jump target not on same page"

Message ErrMsgCodeOverflow
 "Code&uuml;berlauf"
 "code overflow"

Message ErrMsgMixDBDS
 "Konstanten und Platzhalter nicht mischbar"
 "constants and placeholders cannot be mixed"

Message ErrMsgNotInStruct
 "Codeerzeugung in Strukturdefinition nicht zul&auml;ssig"
 "code must not be generated in structure definition"

Message ErrMsgParNotPossible
 "paralleles Konstrukt nicht m&ouml;glich"
 "parallel construct not possible here"

Message ErrMsgAdrOverflow
 "Adre&szlig;&uuml;berlauf"
 "address overflow"

Message ErrMsgInvSegment
 "ung&uuml;ltiges Segment"
 "invalid segment"

Message ErrMsgUnknownSegment
 "unbekanntes Segment"
 "unknown segment"

Message ErrMsgUnknownSegReg
 "unbekanntes Segmentregister"
 "unknown segment register"

Message ErrMsgInvString
 "ung&uuml;ltiger String"
 "invalid string"

Message ErrMsgInvRegName
 "ung&uuml;ltiger Registername"
 "invalid register name"

Message ErrMsgInvArg
 "ung&uuml;ltiges Argument"
 "invalid argument"

Message ErrMsgNoIndir
 "keine Indirektion erlaubt"
 "indirect mode not allowed"

Message ErrMsgNotInThisSegment
 "nicht im aktuellen Segment erlaubt"
 "not allowed in current segment"

Message ErrMsgNotInMaxmode
 "nicht im Maximum-Modus zul&auml;ssig"
 "not allowed in maximum mode"

Message ErrMsgOnlyInMaxmode
 "nicht im Minimum-Modus zul&auml;ssig"
 "not allowed in minimum mode"

Message ErrMsgOpeningFile
 "Fehler beim &Ouml;ffnen der Datei"
 "error in opening file"

Message ErrMsgListWrError
 "Listingschreibfehler"
 "error in writing listing"

Message ErrMsgFileReadError
 "Dateilesefehler"
 "file read error"

Message ErrMsgFileWriteError
 "Dateischreibfehler"
 "file write error"

Message ErrMsgIntError
 "interne(r) Fehler/Warnung"
 "internal error/warning"

Message ErrMsgHeapOvfl
 "Speicher&uuml;berlauf"
 "heap overflow"

Message ErrMsgStackOvfl
 "Stapel&uuml;berlauf"
 "stack overflow"
 
Message ErrMsgIsFatal
 "Fataler Fehler, Assembler abgebrochen"
 "fatal error, assembly terminated"

Message ErrMsgOvlyError
 "Overlayfehler - Programmabbruch"
 "overlay error - program terminated"

Message PrevDefMsg
 "vorherige Definition in"
 "previous definition in"

Message ErrMsgInvSwapSize
 "ung&uuml;ltige Gr&ouml;&szlig;enangabe f&uuml;r Swapfile - Programmabbruch"
 "swap file size not correctly specified - program terminated"

Message ErrMsgSwapTooBig
 "zuwenig Platz f&uuml;r Swapfile - Programmabbruch"
 "insufficient space for swap file - program terminated"

Message ErrMsgNoRelocs
 "relokatible Symbole nicht erlaubt"
 "relocatable symbols not allowed"

Message ErrMsgUnresRelocs
 "unverarbeitete externe Referenzen"
 "unprocessed external references"

Message ErrMsgUnexportable
 "Symbol nicht exportierbar"
 "cannot export this symbol"

;----------------------------------------------------------------------------
; Strings in Listingkopfzeile

Message HeadingFileNameLab
 " - Quelle "
 " - source file "

Message HeadingPageLab
 " - Seite "
 " - page "

;----------------------------------------------------------------------------
; Strings in Listing

Message ListSymListHead1
 "  Symboltabelle (*=unbenutzt):"
 "  symbol table (* = unused):"

Message ListSymListHead2
 "  ----------------------------"
 "  ------------------------"

Message ListSymSumMsg
 " Symbol"
 " symbol"

Message ListSymSumsMsg
 " Symbole"
 " symbols"

Message ListUSymSumMsg
 " unbenutztes Symbol"
 " unused symbol"

Message ListUSymSumsMsg
 " unbenutzte Symbole"
 " unused symbols"

Message ListRegDefListHead1
 "  Registerdefinitionen (*=unbenutzt):"
 "  register definitions (*=unused):"

Message ListRegDefListHead2
 "  -----------------------------------"
 "  --------------------------------"

Message ListRegDefSumMsg
 " Definition"
 " definition"

Message ListRegDefSumsMsg
 " Definitionen"
 " definitions"

Message ListRegDefUSumMsg
 " unbenutzte Definition"
 " unused definition"

Message ListRegDefUSumsMsg
 " unbenutzte Definitionen"
 " unused definitions"

Message ListCodepageListHead1
 "  Zeichentabellen:"
 "  codepages:"

Message ListCodepageListHead2
 "  ----------------"
 "  ----------"

Message ListCodepageChange
 " ver&auml;ndertes Zeichen"
 " changed character"

Message ListCodepagePChange
 " ver&auml;nderte Zeichen"
 " changed characters"

Message ListCodepageSumMsg
 " Zeichentabelle"
 " code page"

Message ListCodepageSumsMsg
 " Zeichentabellen"
 " code pages"

Message ListMacListHead1
 "  definierte Makros:"
 "  defined macros:"

Message ListMacListHead2
 "  ------------------"
 "  ---------------"

Message ListMacSumMsg
 " Makro"
 " macro"

Message ListMacSumsMsg
 " Makros"
 " macros"

Message ListFuncListHead1
 "  definierte Funktionen:"
 "  defined functions:"

Message ListFuncListHead2
 "  ----------------------"
 "  ------------------"

Message ListDefListHead1
 "  DEFINEs:"
 "  DEFINEs:"

Message ListDefListHead2
 "  --------"
 "  --------"

Message ListSegListHead1
 "in "
 "space used in "

Message ListSegListHead2
 " belegte Bereiche:"
 " :"

Message ListCrossListHead1
 "  Querverweisliste:"
 "  cross reference list:"

Message ListCrossListHead2
 "  -----------------"
 "  ---------------------"

Message ListSectionListHead1
 "  Sektionen:"
 "  sections:"

Message ListSectionListHead2
 "  ----------"
 "  ---------"

Message ListIncludeListHead1
 "  Include-Verschachtelung:"
 "  nested include files:"

Message ListIncludeListHead2
 "  ------------------------"
 "  ---------------------"

Message ListCrossSymName
 "Symbol "
 "symbol "

Message ListCrossFileName
 "Datei "
 "file "

Message ListPlurName
 "n"
 "s"

Message ListHourName
 " Stunde"
 " hour"

Message ListMinuName
 " Minute"
 " minute"

Message ListSecoName
 " Sekunde"
 " second"

;---------------------------------------------------------------------------
; Durchsagen...

Message InfoMessAssembling
 "Assembliere "
 "assembling "

Message InfoMessPass
 "PASS "
 "PASS "

Message InfoMessPass1
 "PASS 1                             "
 "PASS 1                             "

Message InfoMessPass2
 "PASS 2                             "
 "PASS 2                             "

Message InfoMessAssTime
 " Assemblierzeit"
 " assembly time"

Message InfoMessAssLine
 " Zeile Quelltext"
 " line source file"

Message InfoMessAssLines
 " Zeilen Quelltext"
 " lines source file"

Message InfoMessPassCnt
 " Durchlauf"
 " pass"

Message InfoMessPPassCnt
 " Durchl&auml;ufe"
 " passes"

Message InfoMessNoPass
 "        zus&auml;tzliche erforderliche Durchl&auml;ufe wegen Fehlern nicht\n        durchgef&uuml;hrt, Listing m&ouml;glicherweise inkorrekt"
 "        additional necessary passes not started due to\n        errors, listing possibly incorrect"

Message InfoMessMacAssLine
 " Zeile inkl. Makroexpansionen"
 " line incl. macro expansions"

Message InfoMessMacAssLines
 " Zeilen inkl. Makroexpansionen"
 " lines incl. macro expansions"

Message InfoMessWarnCnt
 " Warnung"
 " warning"

Message InfoMessWarnPCnt
 "en"
 "s"

Message InfoMessErrCnt
 " Fehler"
 " error"

Message InfoMessErrPCnt
 ""
 "s"

Message InfoMessRemainMem
 " KByte verf&uuml;gbarer Restspeicher"
 " Kbytes available memory"

Message InfoMessRemainStack
 " Byte verf&uuml;gbarer Stack"
 " bytes available stack"

Message InfoMessNFilesFound
 ": keine Datei(en) zu assemblieren!"
 ": no file(s) to assemble!"

Message InfoMessMacroAss
 "Makroassembler "
 "macro assembler "

Message InfoMessVar
 "Version"
 "version"

Message InfoMessHead1
 "Aufruf : "
 "calling convention : "

Message InfoMessHead2
 " [Optionen] [Datei] [Optionen] ..."
 " [options] [file] [options] ..."

Message KeyWaitMsg
 "--- weiter mit <ENTER> ---"
 "--- <ENTER> to go on ---"

Message ErrMsgInvParam
 "ung&uuml;ltige Option: "
 "invalid option: "

Message ErrMsgInvEnvParam
 "ung&uuml;ltige Environment-Option: "
 "invalid environment option: "

Message InvMsgSource
 "Quelldatei?"
 "source file?"

Message InfoMessHelp
 "--------\n" \
 "\n" \
 "Optionen :\n" \
 "----------\n" \
 "\n" \
 "-p : Sharefile im Pascal-Format       -c : Sharefile im C-Format\n" \
 "-a : Sharefile im AS-Format\n" \
 "-o <Name> : Namen der Code-Datei neu setzen\n" \
 "-olist <Name> : Namen der Listdatei neu setzen\n" \
 "-shareout <Name> : Namen des Sharefiles neu setzen\n" \
 "-q, -quiet : Stille &Uuml;bersetzung\n" \
 "-cpu <Name> : Zielprozessor setzen\n" \
 "-alias <neu>=<alt> : Prozessor-Alias definieren\n" \
 "-l : Listing auf Konsole              -L : Listing auf Datei\n" \
 "-i <Pfad>[:Pfad]... : Pfadliste f&uuml;r Includedateien\n" \
 "-D <Symbol>[,Symbol]... : Symbole vordefinieren\n" \
 "-E [Name] : Zieldatei f&uuml;r Fehlerliste,\n" \
 "            !0..!4 f&uuml;r Standardhandles\n" \
 "            Default <Quelldatei>.LOG\n" \
 "-r : Meldungen erzeugen, falls zus&auml;tzlicher Pass erforderlich\n" \
 "-Y : Sprungfehlerunterdr&uuml;ckung (siehe Anleitung)\n" \
 "-w : Warnungen unterdr&uuml;cken           +G : Code-Erzeugung unterdr&uuml;cken\n" \
 "-s : Sektionsliste erzeugen           -t : Listing-Teile ein/ausblenden\n" \
 "-u : Belegungsliste erzeugen          -C : Querverweisliste erzeugen\n" \
 "-I : Include-Verschachtelungsliste ausgeben\n" \
 "-g : Debug-Informationen schreiben\n" \
 "-A : kompaktere Symbolablage\n" \
 "-U : Case-sensitiv arbeiten\n" \
 "-x : erweiterte Fehlermeldungen       -n : Fehlermeldungen mit Nummer\n" \
 "-P : Makroprozessorausgabe erzeugen   -M : Makrodefinitionen extrahieren\n" \
 "-h : Hexadezimalzahlen mit Kleinbuchstaben\n" \
 "\n" \
 "Quelldateiangabe darf Jokerzeichen enthalten\n" \
 "\n" \
 "implementierte Prozessoren :\n"
 "--------------------\n" \
 "\n" \
 "options :\n" \
 "---------\n" \
 "\n" \
 "-p : share file formatted for Pascal  -c : share file formatted for C\n" \
 "-a : share file formatted for AS\n" \
 "-o <name> : change name of code file\n" \
 "-olist <nname> : change name of list file\n" \
 "-shareout <nname> : change name of share file\n" \
 "-q,  -quiet : silent compilation\n" \
 "-cpu <name> : set target processor\n" \
 "-alias <new>=<old> : define processor alias\n" \
 "-l : listing to console               -L : listing to file\n" \
 "-i <path>[;path]... : list of paths for include files\n" \
 "-D <symbol>[,symbol]... : predefine symbols\n" \
 "-E <name> : target file for error list,\n" \
 "            !0..!4 for standard handles\n" \
 "            default is <srcname>.LOG\n" \
 "-r : generate messages if repassing necessary\n" \
 "-Y : branch error suppression (see manual)\n" \
 "-w : suppress warnings                +G : suppress code generation\n" \
 "-s : generate section list            -t : enable/disable parts of listing\n" \
 "-u : generate usage list              -C : generate cross reference list\n" \
 "-I : generate include nesting list\n" \
 "-g : write debug info\n" \
 "-A : compact symbol table\n" \
 "-U : case-sensitive operation\n" \
 "-x : extended error messages          -n : add error #s to error messages\n" \
 "-P : write macro processor output     -M : extract macro definitions\n" \
 "-h : use lower case in hexadecimal output\n" \
 "\n" \
 "source file specification may contain wildcards\n" \
 "\n" \
 "implemented processors :\n"
