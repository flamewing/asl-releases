;* pbind.res
;****************************************************************************
;* Makroassembler AS 							    *
;* 									    *
;* Headerdatei BIND.RSC - enthaelt Stringdefinitionen fuer BIND             *
;* 									    *
;* Historie : 28.1.1997 Grundsteinlegung                                    *
;*                                                                          *
;****************************************************************************

Include header.res

Include tools2.res

;-----------------------------------------------------------------------------
; Fehlermeldungen

Message ErrMsgTargetMissing
 "Zieldateiangabe fehlt!"
 "target file specification is missing!"

;-----------------------------------------------------------------------------
; Ansagen

Message InfoMessHead2
 " <Quelldatei(en)> <Zieldatei> [Optionen]"
 " <source file(s)> <target file> [options]"

Message InfoMessHelp
 "\n" \
 "Optionen: -f <Headerliste>  :  auszufilternde Records"
 "\n" \
 "options: -f <header list>  :  records to filter out"

