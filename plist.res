;* plist.res
;*****************************************************************************
;* AS-Portierung                                                             *
;*                                                                           *
;* Stringdefinitionen fuer PLIST                                             *
;*                                                                           *
;* Historie: 31. 5.1996 Grundsteinlegung                                     *
;*            3.12.1996 Erweiterung um Segment-Spalte                        *
;*           21. 1.2000 Meldungen RelocInfo                                  *
;*           26. 6.2000 Mendung ExportInfo                                   *
;*                                                                           *
;*****************************************************************************

Include header.res

Include tools2.res

;-----------------------------------------------------------------------------
; Ansagen

Message MessFileRequest
 "zu listende Programmdatei [.P] : "
 "program file to list [.P] : "

Message MessHeaderLine1
 "Codetyp     Segment    Startadresse   L&auml;nge (Byte)  Endadresse"
 "Code-Type   Segment    Start-Address  Length (Bytes) End-Address"

Message MessHeaderLine2
 "--------------------------------------------------------------"
 "----------------------------------------------------------------"

Message MessHeaderLine1F
 "Datei "
 "File  "

Message MessHeaderLine2F
 "------"
 "------"

Message MessGenerator
 "Erzeuger : "
 "creator : "

Message MessSum1
 "insgesamt "
 "altogether "

Message MessSumSing
 " Byte  "
 " byte  "

Message MessSumPlur
 " Bytes "
 " bytes "

Message MessEntryPoint
 "<Einsprung>           "
 "<entry point>         "

Message MessRelocInfo
 "<Relokationsinfo>   "
 "<relocation info>   "

Message MessExportInfo
 "<export. symbol>    "
 "<exported symbol>   "

Message InfoMessHead2
 " [-q] [Programmdateiname(n)]"
 " [-q] [name of program file(s)]"

Message InfoMessHelp
 "\n"
 "\n"
