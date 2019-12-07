                save
                listing off   ; kein Listing über diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.39 - Datei STDDEF62.INC                                           *
;*   								            *
;*   Sinn : enthält SFR- und Makrodefinitionen für die ST62xx-Prozessoren   *
;* 									    *
;*   letzte Änderungen :  7. 8.1993                                         *
;*                       11. 9.1993 Meldung nur im 1.Pass                   *
;*                       21.11.1993 ROMWIN eingebaut                        *
;*                                                                          *
;****************************************************************************

		ifndef  stddef62inc     ; verhindert Mehrfacheinbindung

stddef62inc     equ     1

                if      (MOMCPU<>25104)&&(MOMCPU<>25109)&&(MOMCPU<>25120)&&(MOMCPU<>25125)
		 fatal  "Falscher Prozessortyp eingestellt: nur ST6210, ST6215, ST6220 oder ST6225 erlaubt!"
		endif

                if      MOMPASS=1
		 message "ST62xx-SFR-Definitionen (C) 1993 Alfred Arnold"
		 message "Zielprozessor: ST\{MOMCPU}"
		endif

;----------------------------------------------------------------------------
; erstmal die Sachen, die es überall gibt :

RegX            SFR     80h             ; Registeradressen (die kennt AS aber
RegY            SFR     81h             ; auch schon intern, nur der Voll-
RegV            SFR     82h             ; ständigkeit halber)
RegW            SFR     83h
RegA            SFR     0ffh

RomBase         SFR     40h             ; Basisadresse ROM-Fenster (besser
RDW             SFR     0c9h            ; aber das ASSUME-Feature nutzen)

PA              SFR     0c0h            ; Port A
DDRA            SFR     0c4h
OPTA            SFR     0cch
PB              SFR     0c1h            ; Port B
DDRB            SFR     0c5h
OPTB            SFR     0cdh

IOPT            SFR     0c8h            ; Interrupt-Steuerung

ADDATA          SFR     0d0h            ; A/D-Wandler
ADCTRL          SFR     0d1h

TPSC            SFR     0d2h            ; Timer
TDATA           SFR     0d3h
TSCR            SFR     0d4h

WDOG            SFR     0d8h            ; Watchdog

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Port C nur mit den 28-Pinnern:

                if      (MOMCPU=6215H)||(MOMCPU=6225H)
PC               SFR    0c2h            ; Port C
DDRC             SFR    0c6h
OPTC             SFR    0ceh
                endif

		include	"st6/instr.inc"

;---------------------------------------------------------------------------

		endif

		restore                 ; wieder erlauben

