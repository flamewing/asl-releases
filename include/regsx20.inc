		ifndef	regsx20			; avoid multiple inclusion
regsx20inc       equ     1

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File regsx20.inc                                             *
;*   								            *
;*   Contins register & bit definitions for Parallax SX20                   *
;*                                                                          *
;****************************************************************************

                save
                listing off		 ; disable listing on this file

		switch	MOMCPUNAME
     		case   	"SX20","SX28"
     		elsecase
     		fatal  "wrong processor type: only SX20 or SX28 supported."
     		endcase

                if      MOMPASS=1
		 message "Parallax SX20/28 SFR Definitions (C) 2019 Alfred Arnold"
		endif

INDF		sfr	$00		; access (FSR)
PC		sfr	$02		; bits 0..7 of program counter
STATUS		sfr	$03		; Status Register
C		bit	STATUS.0	;  Carry
DC		bit	STATUS.1	;  Digit Carry
Z		bit	STATUS.2	;  Zero Flag
PD		bit	STATUS.3	;  Power Down
TO		bit	STATUS.4	;  Time Out
PA0		bit	STATUS.5	;  (Code) Page Address Bits
PA1		bit	STATUS.6
PA2		bit	STATUS.7
FSR		sfr	$04		; File Select Register

RTCC		sfr	$01		; Real-Time Clock/Counter

RA		sfr	$05		; Port A
RB		sfr	$06		; Port B
		if	MOMCPUNAME="SX28"
RC		sfr	$07		; Port C
		endif


;----------------------------------------------------------------------------

		restore                 ; re-enable listing

		endif			; regsx20.inc
