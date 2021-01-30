		ifndef	__pmx150inc	; avoid multiple inclusion
__pmx150inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File PMX150.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for PMx150                            *
;*                                                                          *
;*   Source: PMC150/PMS150 Series Data Sheet, Rev. 1.08, Dec. 11, 2018      *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Interrupt Control

inten		sfr	0x04		; Interrupt Enable
timer_inten	bit	inten.2		;  Timer16 interrupt Enable
pa0_inten	bit	inten.0		;  PA0 Interrupt Enable

intrq		sfr	0x05		; Interrupt Request Register
timer_intrq	bit	intrq.2		;  Timer16 Interrupt Request
pa0_intrq	bit	intrq.0		;  PA0 Interrupt Request

integs		sfr	0x0c		; Interrupt Edge Register
timer_egs	bit	integs.4	;  Timer16 Edge Selection
pa0_egs		_bfield	integs,0,2	;  PA0 Edge Selection

;----------------------------------------------------------------------------
; CPU Core

clkmd		sfr	0x03
clkselect	_bfield	clkmd,5,3	; Clock Select
ihrc_enable	bit	clkmd.4		;  IHRC Enable
clktype		bit	clkmd.3		;  Clock Type Select
ilrc_enable	bit	clkmd.2		;  ILRC Enable
wd_enable	bit	clkmd.2		;  Watch Dog Enable
pa5_prst	bit	clkmd.0		;  Pin PA5/PRST# Function

eoscr		sfr	0x0a		; External Oscillator Setting Register
pwrdn		bit	eoscr.0		;  Power Down Band Gap and LVR Hardware

ihrcr		sfr	0x0b		; IHRC Oscillator Control Register

misc		sfr	0x1b		; MISC Register
en_fwkup	bit	misc.5		;  Enable Fast Wakeup
dis_lvr		bit	misc.2		;  Disable LVR Function
wdperiod	_bfield	misc,0,2	;  Watchdog Timeout

;----------------------------------------------------------------------------
; GPIO

padier		sfr	0x0d		; Port A Digital Input Enable Register

pa		sfr	0x10		; Port A Data Register

pac		sfr	0x11		; Port A Control Register

paph		sfr	0x12		; Port A Pull High Register

;----------------------------------------------------------------------------
; Timer

t16m		sfr	0x06		; Timer 16 Mode Register
tm16_clksrc	_bfield	t16m,5,3	;  Timer Clock Source Selection
tm16_clkdiv	_bfield t16m,3,2	;  Internal Clock Divider
tm16_isrc	_bfield	t16m,0,2	;  Interrupt Source

		restore
		endif			; __pmx150inc
