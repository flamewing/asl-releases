;* p2bin.res
;*****************************************************************************
;* AS-Portierung                                                             *
;*                                                                           *
;* Stringdefinitionen fuer P2BIN                                             *
;*                                                                           *
;* Historie:  3. 6.1996 Grundsteinlegung                                     *
;*           24. 3.2000 added byte messages                                  *
;*                                                                           *
;*****************************************************************************
;* $Id: p2bin.res,v 1.3 2006/12/09 18:27:30 alfred Exp $
/*****************************************************************************
;* $Log: p2bin.res,v $
;* Revision 1.3  2006/12/09 18:27:30  alfred
;* - add warning about empty output
;*
;*****************************************************************************

Include header.res

Include tools2.res

;------------------------------------------------------------------------------
; Ansagen

Message InfoMessChecksum
 "Pr&uuml;fsumme: "
 "checksum: "

Message InfoMessHead2
 " <Quelldatei(en)> <Zieldatei> [Optionen]"
 " <source file(s)> <target file> [options]"

Message Byte
 "byte"
 "Byte"

Message Bytes
 "bytes"
 "Bytes"

Message InfoMessHelp
 "\n" \
 "Optionen: -f <Headerliste>  :  auszufilternde Records\n" \
 "          -r <Start>-<Stop> :  auszufilternder Adre&szlig;bereich\n" \
 "          ($ = erste bzw. letzte auftauchende Adresse)\n" \
 "          -l <8-Bit-Zahl>   :  Inhalt unbenutzter Speicherzellen festlegen\n" \
 "          -s                :  Pr&uuml;fsumme in Datei ablegen\n" \
 "          -m <Modus>        :  EPROM-Modus (odd,even,byte0..byte3)\n" \
 "          -e <Adresse>      :  Startadresse festlegen\n" \
 "          -S [L|B]<L&auml;nge>   :  Startadresse voranstellen\n" \
 "          -k                :  Quelldateien automatisch l&ouml;schen\n"
 "\n" \
 "options: -f <header list>  :  records to filter out\n" \
 "         -r <start>-<stop> :  address range to filter out\n" \
 "         ($ = first resp. last occuring address)\n" \
 "         -l <8-bit-number> :  set filler value for unused cells\n" \
 "         -s                :  put checksum into file\n" \
 "         -m <mode>         :  EPROM-mode (odd,even,byte0..byte3)\n" \
 "         -e <address>      :  set entry address\n" \
 "         -S [L|B]<length>  :  prepend entry address to image\n" \
 "         -k                :  automatically erase source files\n"

Message WarnEmptyFile
 "Warnung: keine Daten &uuml;bertragen, falsche/fehlende -r Option?\n"
 "Warning: no data transferred, wrong/missing -r option?\n"
