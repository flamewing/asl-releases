		ifndef  stddef04inc     ; avoid multiple inclusion
stddef04inc     equ     1

		save
		listing off		; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF04.INC                                            *
;*   								            *
;*   Contains SFR Definitions for the 6804                                  *
;*   One may look into STDDEF62.INC for some nice macros...                 *
;*                                                                          *
;****************************************************************************

                if      (MOMCPU<>26628)
		 fatal  "wrong target selected: only 6804 supported"
		endif

                if      MOMPASS=1
		 message "6804 SFR Definitions (C) 1993 Alfred Arnold"
		endif

;----------------------------------------------------------------------------

XP              SFR     $80             ; Register Addresses (the assembler
YP              SFR     $81             ; also has internal symbols for them,
RegA            SFR     $ff             ; just for completeness)

PA              SFR     $00             ; Port A Data Register
DDRA            SFR     $04		; Port A Data Direction Register
PB              SFR     $01             ; Port B Data Register
DDRB            SFR     $05		; Port B Data Direction Register
PC		SFR	$02		; Port C Data Register
DDRC		SFR	$06		; Port C Data Direction Register

TSCR            SFR     $09             ; Timer : Control/Status
TCR             SFR     $fe		;         Data
TPR             SFR     $fd		;         Prescaler

USERROM		SFR	$20		; User Defined ROM (up to $5f)
USERRAM		SFR	$82		; Free RAM (up to $9f)

;---------------------------------------------------------------------------

                restore                 ; re-allow listing

                endif			; stddef04inc
