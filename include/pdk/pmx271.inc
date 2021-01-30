		ifndef	__pmx271inc	; avoid multiple inclusion
__pmx271inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File PMX271.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for PMC271/PMS271                     *
;*                                                                          *
;*   Sources: PMC2717PMS271 Data Sheet, Ver. 1.05, Dec 18, 2018             *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Interrupt Control

inten		sfr	0x04		; Interrupt Enable
adc_inten	bit	inten.3		;  ADC Interrupt Enable
timer16_inten	bit	inten.2		;  Timer16 Interrupt Enable
pb0_inten	bit	inten.1		;  PB0 Interrupt Enable
pa0_inten	bit	inten.0		;  PA0 Interrupt Enable

intrq		sfr	0x05		; Interrupt Request Register
adc_intrq	bit	intrq.3		;  ADC Interrupt Request
timer16_intrq	bit	intrq.2		;  Timer16 Interrupt Request
pb0_intrq	bit	intrq.1		;  PB0 Interrupt Request
pa0_intrq	bit	intrq.0		;  PA0 Interrupt Request
	
integs		sfr	0x0c		; Interrupt Edge Register
timer16_egs	bit	integs.4	;  Timer16 Edge Selection
pb0_egs		_bfield	integs,2,2	;  PB0 Edge Selection
pa0_egs		_bfield	integs,0,2	;  PA0 Edge Selection

;----------------------------------------------------------------------------
; CPU Core

__numcpus	equ	2

clkmd		sfr	0x03		; Clock Mode Register
clkselect	_bfield	clkmd,5,3	;  System Clock Selection
ihrc_enable	bit	clkmd.4		;  IHRC Enable
clktype		bit	clkmd.3		;  Clock Type Select
ilrc_enable	bit	clkmd.2		;  ILRC Enable
wd_enable	bit	clkmd.1		;  Watch Dog Enable
pa5_prst	bit	clkmd.0		;  Pin PA5/RESET# Function

ihrcr		sfr	0x0b		; Internal High RC Oscillator Control Register

eoscr		sfr	0x0a		; External Oscillator Setting Register
enxtal		bit	eoscr.7		;  Enable external crystal
xtalsel		_bfield	eoscr,5,2	;  External Crystal Oscillator Selection
pwrdn		bit	eoscr.0		;  Power Down Band Gap and LVR Hardware

misc		sfr	0x3b		; MISC Register
en32k_lcur	bit	misc.6		;  Enable 32 kHz low current after osc.
en_fwkup	bit	misc.5		;  Enable Fast Wakeup
en_vdd2_gen	bit	misc.4		;  Enable VDD2/2 Bias Voltage Generator
lvr_rec		bit	misc.3		;  LVR Recover Time
dis_lvr		bit	misc.2		;  Disable LVR Function
wdperiod	_bfield	misc,0,2	;  Watchdog Timeout Period

rstst		sfr	0x25		; Reset Status Register
pa5_rst		bit	rstst.3		;  PA5 External Reset Flag
vdd_bel_4v	bit	rstst.2		;  Vdd below 4V
vdd_bel_3v	bit	rstst.1		;  Vdd below 3V
vdd_bel_2v	bit	rstst.0		;  Vdd below 2V

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
; ADC

adcc		sfr	0x20		; ADC Control Register
adc_en		bit	adcc.7		;  Enable ADC
adc_pr_ctl	bit	adcc.6		;  ADC Process Control Bit
adc_chsel	_bfield	adcc,2,4	;  ADC Channel Select

adcrhc		sfr	0x1c		; ADC Reference High Control Register
adc_refhisel	_bfield	adcrhc,4,4	;  Reference High Select
adc_enrefpa2	bit	adcrhc,0	;  Enable reference output on PA2

adcm		sfr	0x21		; ADC Mode Register
adc_clksel	_bfield	adcm,1,4	;  Clock Source Select

adcr		sfr	0x22		; ADC Result Register

;----------------------------------------------------------------------------
; Timer

t16m		sfr	0x06		; Timer 16 Mode Register
tm16_clksrc	_bfield	t16m,5,3	;  Timer Clock Source Selection
tm16_clkdiv	_bfield t16m,3,2	;  Internal Clock Divider
tm16_isrc	_bfield	t16m,0,3	;  Interrupt Source

		restore
		endif			; __pmx271inc
