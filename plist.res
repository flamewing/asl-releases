;* plist.res
;*****************************************************************************
;* AS-Portierung                                                             *
;*                                                                           *
;* Stringdefinitionen fuer PLIST                                             *
;*                                                                           *
;* Historie: 31. 5.1996 Grundsteinlegung                                     *
;*            3.12.1996 Erweiterung um Segment-Spalte                        *
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
 "Codetyp     Segment  Startadresse   L&auml;nge (Byte)  Endadresse"
 "code type   segment   start address length (byte)  end address"

Message MessHeaderLine2
 "------------------------------------------------------------"
 "--------------------------------------------------------------"

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

Message InfoMessHead2
 " [Programmdateiname]"
 " [name of program file]"

Message InfoMessHelp
 "\n"
 "\n"
