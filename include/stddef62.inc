		ifndef  stddef62inc     ; avoid multiple inclusion
stddef62inc     equ     1

                save
                listing off		; no  listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF62.INC                                            *
;*   								            *
;*   Contains SFR and Macro Definitions for ST62xx Processors               *
;*   NOTE: This file is obsolete, use REGST6.INC in new projects.           *
;* 									    *
;*                                                                          *
;****************************************************************************

                if      (MOMCPU<>25104)&&(MOMCPU<>25109)&&(MOMCPU<>25120)&&(MOMCPU<>25125)
		 fatal  "Wrong target selected: only ST6210, ST6215, ST6220 oder ST6225 supported"
		endif

                if      MOMPASS=1
		 message "ST62xx SFR Definitions (C) 1993 Alfred Arnold"
		 message "Target Processor: ST\{MOMCPU}"
		endif

;----------------------------------------------------------------------------
; Things Present on All Variants:

RegX            SFR     80h             ; Register Addresses (also defined as
RegY            SFR     81h             ; internal sysmbols, just for complete-
RegV            SFR     82h             ; ness)
RegW            SFR     83h
RegA            SFR     0ffh

RomBase         SFR     40h             ; Base Addresse ROM Window (consider
RDW             SFR     0c9h            ; using ASSUME)

PA              SFR     0c0h            ; Port A
DDRA            SFR     0c4h
OPTA            SFR     0cch
PB              SFR     0c1h            ; Port B
DDRB            SFR     0c5h
OPTB            SFR     0cdh

IOPT            SFR     0c8h            ; Interrupt Control

ADDATA          SFR     0d0h            ; A/D Converter
ADCTRL          SFR     0d1h

TPSC            SFR     0d2h            ; Timer
TDATA           SFR     0d3h
TSCR            SFR     0d4h

WDOG            SFR     0d8h            ; Watchdog

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Port C only on 28-Pin Devices:

                if      (MOMCPU=6215H)||(MOMCPU=6225H)
PC               SFR    0c2h            ; Port C
DDRC             SFR    0c6h
OPTC             SFR    0ceh
                endif

		include	"st6/instr.inc"

;---------------------------------------------------------------------------

		restore                 ; enable listing again

		endif			; stddef62inc
