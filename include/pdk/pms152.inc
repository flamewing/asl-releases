		ifndef	__pms152inc	; avoid multiple inclusion
__pms152inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File PMS152.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for PMS152                            *
;*                                                                          *
;*   Source: PMS152 Data Sheet, Ver. 1.04, Oct 17, 2019                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Interrupt Control

inten		sfr	0x04		; Interrupt Enable
timer2_inten	bit	inten.6		;  Timer2 Interrupt Enable
pwmg_inten	bit	inten.5		;  PWMG Interrupt Enable
comp_inten	bit	inten.4		;  Comparator Interrupt Enable
timer16_inten	bit	inten.2		;  Timer16 Interrupt Enable
pb0_inten	bit	inten.1		;  PB0/PA4 Interrupt Enable
pa0_inten	bit	inten.0		;  PA0/PB5 Interrupt Enable

intrq		sfr	0x05		; Interrupt Request Register
timer2_intrq	bit	intrq.6		;  Timer2 Interrupt Request
pwmg_intrq	bit	intrq.5		;  PWMG Interrupt Request
comp_intrq	bit	intrq.4		;  Comparator Interrupt Request
timer16_intrq	bit	intrq.2		;  Timer16 Interrupt Request
pb0_intrq	bit	intrq.1		;  PB0/PA4 Interrupt Request
pa0_intrq	bit	intrq.0		;  PA0/PB5 Interrupt Request
	
integs		sfr	0x0c		; Interrupt Edge Register
timer16_egs	bit	io(integs).4	;  Timer16 Edge Selection
pb0_egs		_bfield	integs,2,2	;  PB0/PA4 Edge Selection
pa0_egs		_bfield	integs,0,2	;  PA0/PB5 Edge Selection

;----------------------------------------------------------------------------
; CPU Core

clkmd		sfr	0x03
clkselect	_bfield	clkmd,5,3	; Clock Select
ihrc_enable	bit	clkmd.4		;  IHRC Enable
clktype		bit	clkmd.3		;  Clock Type Select
ilrc_enable	bit	clkmd.2		;  ILRC Enable
wd_enable	bit	clkmd.1		;  Watch Dog Enable
pa5_prst	bit	clkmd.0		;  Pin PA5/PRST# Function

eoscr		sfr	0x0a		; External Oscillator Setting Register
enxtal		bit	eoscr.7		;  Enable external crystal
xtalsel		_bfield	eoscr,5,2	;  External Crystal Oscillator Selection
pwrdn		bit	eoscr.0		;  Power Down Band Gap and LVR Hardware

;ihrcr		*sfr	0x0b		; IHRC Oscillator Control Register

misc		sfr	0x08		; MISC Register
en_fwkup	bit	misc.5		;  Enable Fast Wakeup
dis_lvr		bit	misc.2		;  Disable LVR Function
wdperiod	_bfield	misc,0,2	;  Watchdog Timeout

;----------------------------------------------------------------------------
; GPIO

padier		sfr	0x0d		; Port A Digital Input Enable Register
pbdier		sfr	0x0e		; Port B Digital Input Enable Register

pa		sfr	0x10		; Port A Data Register
pb		sfr	0x14		; Port B Data Register

pac		sfr	0x11		; Port A Control Register
pbc		sfr	0x15		; Port B Control Register

paph		sfr	0x12		; Port A Pull High Register
pbph		sfr	0x16		; Port B Pull High Register

;----------------------------------------------------------------------------
; Timer

t16m		sfr	0x06		; Timer 16 Mode Register
tm16_clksrc	_bfield	t16m,5,3	;  Timer Clock Source Selection
tm16_clkdiv	_bfield t16m,3,2	;  Internal Clock Divider
tm16_isrc	_bfield	t16m,0,3	;  Interrupt Source

tm2c		sfr	0x1c		; Timer2 Control Register
tm2_clksel	_bfield	tm2c,4,4	;  Clock Selection
tm2_outsel	_bfield	tm2c,2,2	;  Output Selection
tm2_mode	bit	tm2c.1		;  Mode Selection
tm2_pol		bit	tm2c.0		;  Inverse Polarity of Output
tm2ct		sfr	0x1d		; Timer 2 Counter Register
tm2s		sfr	0x17		; Timer 2 Scaler Register
tm2_pwmsel	bit	tm2s,7		;  Resolution Selection
tm2_prescal	_bfield	tm2s,5,2	;  Clock Prescaler
tm2_clkscal	_bfield	tm2s,0,5	;  Clock Scaler
tm2b		sfr	0x09		; Timer 2 Bound Register

;----------------------------------------------------------------------------
; Comparator

gpcc		sfr	0x18		; Comparator Control
gpcc_en		bit	gpcc.7		;  Enable
gpcc_res	bit	gpcc.6		;  Result
gpcc_samp	bit	gpcc.5		;  Output sampled by TM2_CLK?
gpcc_pol	bit	gpcc.4		;  Output Polarity Selection
gpcc_minp	_bfield	gpcc,1,3	;  Minus Input Selection
gpcc_pinp	bit	gpcc.0		;  Plus Input Selection

gpcs		sfr	0x19		; Comparator Selection Register
gps_oe		bit	gpcs.7		;  Output Enable
gps_hrng	bit	gpcs.5		;  High Range Select
gps_lrng	bit	gpcs.4		;  Low Range Select
gps_lvl		_bfield	gpcs,0,4	;  Voltage Level

;----------------------------------------------------------------------------
; PWM

pwmg0c		sfr	0x20		; PWMG0 Control Register
pwm0_out	bit	pwmg0c.6	;  Output Value
pwm0_pol	bit	pwmg0c.5	;  Output Polarity
pwm0_osel	bit	pwmg0c.4	;  Output Selection
pwm0_oportsel	_bfield	pwmg0c,1,3	;  Output Port Selection
pwm0_presel	bit	pwmg0c.0	;  Output Pre-Selection

pwmgclk		sfr	0x21		; PWMG Clock Register
pwmg_en		bit	pwmgclk.7	;  PWMG Enable
pwmg_prescal	_bfield	pwmgclk,4,3	;  PWMG Prescaler
pwmg_clksel	bit	pwmgclk,0	;  PWMG Clock Source Selection

pwmgcubh	sfr	0x24		; PWMG Counter Upper Bound High Register
pwmgcubl	sfr	0x25		; PWMG Counter Upper Bound Low Register
pwmg0dth	sfr	0x22		; PWMG0 Duty Value High Register
pwmg0dtl	sfr	0x23		; PWMG0 Duty Value Low Register

pwmg1c		sfr	0x26		; PWMG1 Control Register
pwm1_out	bit	pwmg1c.6	;  Output Value
pwm1_pol	bit	pwmg1c.5	;  Output Polarity
pwm1_osel	bit	pwmg1c.4	;  Output Selection
pwm1_oportsel	_bfield	pwmg1c,1,3	;  Output Port Selection

pwmg1dth	sfr	0x28		; PWMG1 Duty Value High Register
pwmg1dtl	sfr	0x29		; PWMG1 Duty Value Low Register

pwmg2c		sfr	0x2c		; PWMG2 Control Register
pwm2_out	bit	pwmg2c.6	;  Output Value
pwm2_pol	bit	pwmg2c.5	;  Output Polarity
pwm2_osel	bit	pwmg2c.4	;  Output Selection
pwm2_oportsel	_bfield	pwmg2c,1,3	;  Output Port Selection

pwmg2dth	sfr	0x2e		; PWMG2 Duty Value High Register
pwmg2dtl	sfr	0x2f		; PWMG2 Duty Value Low Register

		restore
		endif			; __pms152inc
