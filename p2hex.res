;* p2hex.res
;*****************************************************************************
;* AS-Portierung                                                             *
;*                                                                           *
;* String-Definitionen fuer P2HEX                                            *
;*                                                                           *
;* Historie:  1. 6.1996 Grundsteinlegung                                     *
;*           26.10.1997 kill-Option                                          *
;*            6. 7.1999 Minimalparameter Motorola S-Records                  *
;*           24.10.1999 Parameter Relokation                                 *
;*                                                                           *
;*****************************************************************************

Include header.res

Include tools2.res

;------------------------------------------------------------------------------
; Steuerstrings HEX-Files

Message DSKHeaderLine
 "K_DSKA_1.00_DSK_"
 "K_DSKA_1.00_DSK_"

;------------------------------------------------------------------------------
; Fehlermeldungen

Message ErrMsgAdrOverflow
 "Warnung: Adre&szlig;&uuml;berlauf "
 "warning: address overflow "

;------------------------------------------------------------------------------
; Ansagen

Message InfoMessHead2
 " <Quelldatei(en)> <Zieldatei> [Optionen]"
 " <source file(s)> <target file> [options]"

Message InfoMessHelp
 "\n" \
 "Optionen: -f <Headerliste>  : auszufilternde Records\n" \
 "          -r <Start>-<Stop> : auszufilternder Adre&szlig;bereich\n" \
 "          -R <offset>       : Adressen um Offset verschieben\n" \
 "          ($ = erste bzw. letzte auftauchende Adresse)\n" \
 "          -a                : Adressen relativ zum Start ausgeben\n" \
 "          -l <L&auml;nge>        : L&auml;nge Datenzeile/Bytes\n" \
 "          -i <0|1|2>        : Terminierer f&uuml;r Intel-Hexfiles\n" \
 "          -m <0..3>         : Multibyte-Modus\n" \
 "          -F <Default|Moto|\n" \
 "              Intel|MOS|Tek|\n" \
 "              Intel16|DSK|\n" \
 "              Intel32>      : Zielformat\n" \
 "          +5                : S5-Records unterdr&uuml;cken\n" \
 "          -s                : S-Record-Gruppen einzeln terminieren\n" \
 "          -M <1|2|3>        : Minimallaenge Adressen S-Records\n" \
 "          -d <Start>-<Stop> : Datenbereich festlegen\n" \
 "          -e <Adresse>      : Startadresse festlegen\n" \
 "          -k                : Quelldateien autom. l&ouml;schen\n"  
 "\n" \
 "options: -f <header list>  : records to filter out\n" \
 "         -r <start>-<stop> : address range to filter out\n" \
 "         -R <offset>       : relocate addresses by offset\n" \
 "         ($ = first resp. last occuring address)\n" \
 "         -a                : addresses in hex file relativ to start\n" \
 "         -l <length>       : set length of data line in bytes\n" \
 "         -i <0|1|2>        : terminating line for intel hex\n" \
 "         -m <0..3>         : multibyte mode\n" \
 "         -F <Default|Moto|\n" \
 "             Intel|MOS|Tek|\n" \
 "             Intel16|DSK|\n" \
 "             Intel32>      : target format\n" \
 "         +5                : supress S5-records\n" \
 "         -s                : separate terminators for S-record groups\n" \
 "         -M <1|2|3>        : minimum length of S records addresses\n" \
 "         -d <start>-<stop> : set data range\n" \
 "         -e <address>      : set entry address\n" \
 "         -k                : automatically erase source files\n"
