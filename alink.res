;* alink.res
;*****************************************************************************
;* AS-Portierung                                                             *
;*                                                                           *
;* Stringdefinitionen fuer ALINK                                             *
;*                                                                           *
;* Historie:  2. 7.2000 Grundsteinlegung                                     *
;*            4. 7.2000 misc. info messages                                  *
;*                                                                           *
;*****************************************************************************

Include header.res

Include tools2.res

;-----------------------------------------------------------------------------
; info messages

Message InfoMsgOpenSrc
 "&Ouml;ffne Quelldatei"
 "opening source file"

;-----------------------------------------------------------------------------
; Fehlermeldungen

Message ErrMsgSrcMissing
 "Quelldateiangabe(n) fehlt!"
 "source file specification(s) missing!"

Message FormatRelocInfoMissing
 "Relokationsinformation fehlt hinter Datensatz"
 "relocation info for relocatable record missing"

Message DoubleDefSymbol
 "doppelt definiertes Symbol"
 "double defined symbol"

Message UndefSymbol
 "undefiniertes Symbol"
 "undefined symbol"

Message SumUndefSymbol
 "undefiniertes Symbol"
 "undefined symbol"

Message SumUndefSymbols
 "undefiniertes Symbole"
 "undefined symbols"

;-----------------------------------------------------------------------------
; Ansagen

Message InfoMessHead2
 " <Quelldatei(en)> <Zieldatei> [Optionen]"
 " <source file(s)> <target file> [options]"

Message InfoMessHelp
 "\n" \
 "Optionen: -v                : ausf&uuml;hrliche Meldungen\n"
 "\n" \
 "options:  -v                : verbose messages\n"
