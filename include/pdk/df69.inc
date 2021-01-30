		ifndef	__df69inc	; avoid multiple inclusion
__df69inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File DF69.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for DF69                              *
;*                                                                          *
;*   Sources: DF69 Data Sheet, Ver. 0.03, Apr 18, 2018                      *
;*                                                                          *
;****************************************************************************

; Interrupt Control

inten		sfr	0x04		; Interrupt Enable
timer2_inten	bit	inten.6		;  Timer2 Interrupt Enable
pwmg_inten	bit	inten.5		;  PWMG Interrupt Enable
comp_inten	bit	inten.4		;  Hall Comparator Interrupt Enable
adc_inten	bit	inten.3		;  ADC Interrupt Enable
timer16_inten	bit	inten.2		;  Timer16 Interrupt Enable
pb0_inten	bit	inten.1		;  PB0/PB7 Interrupt Enable
pa0_inten	bit	inten.0		;  PA0/PA5 Interrupt Enable

intrq		sfr	0x05		; Interrupt Request Register
timer2_intrq	bit	intrq.6		;  Timer2 Interrupt Request
pwmg_intrq	bit	intrq.5		;  PWMG Interrupt Request
comp_intrq	bit	intrq.4		;  Comparator Interrupt Request
adc_intrq	bit	intrq.3		;  ADC Interrupt Request
timer16_intrq	bit	intrq.2		;  Timer16 Interrupt Request
pb0_intrq	bit	intrq.1		;  PB0/PB7 Interrupt Request
pa0_intrq	bit	intrq.0		;  PA0/PA5 Interrupt Request
	
integs		sfr	0x0c		; Interrupt Edge Register
timer16_egs	bit	io(integs).4	;  Timer16 Edge Selection
pb0_egs		_bfield	integs,2,2	;  PB0/PB7 Edge Selection
pa0_egs		_bfield	integs,0,2	;  PA0/PA5 Edge Selection

;----------------------------------------------------------------------------
; CPU Core

__numcpus	equ	2

clkmd		sfr	0x03
clkselect	_bfield	clkmd,5,3	; Clock Select
ihrc_enable	bit	clkmd.4		;  IHRC Enable
clktype		bit	clkmd.3		;  Clock Type Select
ilrc_enable	bit	clkmd.2		;  ILRC Enable
wd_enable	bit	clkmd.1		;  Watch Dog Enable
pa5_prst	bit	clkmd.0		;  Pin PA5/RESET# Function

eoscr		sfr	0x0a		; External Oscillator Setting Register
enxtal		bit	eoscr.7		;  Enable external crystal
xtalsel		_bfield	eoscr,5,2	;  External Crystal Oscillator Selection
pwrdn		bit	eoscr.0		;  Power Down Band Gap and LVR Hardware

;ihrcr		*sfr	0x0b		; IHRC Oscillator Control Register

misc		sfr	0x3b		; MISC Register
en32k_lcur	bit	misc.6		;  Enable 32 kHz low current after osc.
en_fwkup	bit	misc.5		;  Enable Fast Wakeup
lvr_rec		bit	misc.3		;  LVR Recover Time
dis_lvr		bit	misc.2		;  Disable LVR Function
wdperiod	_bfield	misc,0,2	;  Watchdog Timeout

rop		sfr	0x3e		; Register Option Register
pwmg_res	bit	rop.3		;  PWMG Resolution Selection
tm16_clkprediv	bit	rop.2		;  Timer16 Clock Pre-Divider
extint1_opt	bit	rop.1		;  External Interrupt 1 Pin Selection
extint0_opt	bit	rop.0		;  External Interrupt 0 Pin Selection

rstst		sfr	0x25		; Reset Status Register
wdg_rst		bit	rstst.7		;  Watchdog Reset Flag
inv_opc_rst	bit	rstst.6		;  Invalid Opcode Reset Flag
pa5_rst		bit	rstst.3		;  PA5 External Reset Flag
vdd_bel_4v	bit	rstst.2		;  Vdd below 4V
vdd_bel_3v	bit	rstst.1		;  Vdd below 3V
vdd_bel_2v	bit	rstst.0		;  Vdd below 2V

;----------------------------------------------------------------------------
; Multiplier

mulop		sfr	0x08		; Multiplier Operand Register
mulrh		sfr	0x09		; Multiplier Result High Byte Register (ro)

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

tm2c		sfr	0x3c		; Timer2 Control Register
tm2_clksel	_bfield	tm2c,4,4	;  Clock Selection
tm2ct		sfr	0x3d		; Timer 2 Counter Register
tm2s		sfr	0x37		; Timer 2 Scaler Register
tm2_prescal	_bfield	tm2s,5,2	;  Clock Prescaler
tm2_clkscal	_bfield	tm2s,0,5	;  Clock Scaler
tm2b		sfr	0x09		; Timer 2 Bound Register

;----------------------------------------------------------------------------
; ADC

adcc		sfr	0x20		; ADC Control Register
adc_en		bit	adcc.7		;  Enable ADC
adc_pr_ctl	bit	adcc.6		;  ADC Process Control Bit
adc_chsel	_bfield	adcc,2,4	;  ADC Channel Select

adcm		sfr	0x21		; ADC Mode Register
adc_res		_bfield	adcm,5,3	;  Resolution Select
adc_clksel	_bfield	adcm,1,4	;  Clock Source Select

adcrh		sfr	0x22		; ADC Result High Register
adcrl		sfr	0x23		; ADC Result Low Register

;----------------------------------------------------------------------------
; Hall Comparator

hcc		sfr	0x2a		; Comparator Control
hcc_en		bit	hcc.7		;  Enable
hcc_res1	bit	hcc.6		;  Result 1
hcc_res2	bit	hcc.5		;  Result 2
hcc_out		bit	hcc.4		;  HC_Out
hcc_isrc	_bfield	hcc,0,3		;  Output to PA5 and Interrupt Source Selection

hc1a		sfr	0x2b		; Hall Comparator 1 Adjust Register
hcc_psel	bit	hc1a.7		;  Positive Pin Selection
hcc_msel	_bfield	hc1a,5,2	;  Negative Pin Selection
hcc1_adj	_bfield	hc1a,0,5	;  Comparator 1 Adjust Bits

hc2a		sfr	0x2c		; Hall Comparator 2 Adjust Register
hcc2_adj	_bfield	hc2a,0,5	;  Comparator 2 Adjust Bits

;----------------------------------------------------------------------------
; Comparator

gpcc		sfr	0x3e		; Comparator Control
gpcc_en		bit	gpcc.7		;  Enable
gpcc_res	bit	gpcc.6		;  Result
gpcc_samp	bit	gpcc.5		;  Output sampled by TM2_CLK?
gpcc_pol	bit	gpcc.4		;  Output Polarity Selection
gpcc_minp	_bfield	gpcc,1,3	;  Minus Input Selection
gpcc_pinp	bit	gpcc.0		;  Plus Input Selection

gpcs		sfr	0x22		; Comparator Selection Register
gps_oe		bit	gpcs.7		;  Output Enable
gps_wkup	bit	gpcs.6		;  Wake-Up Enable
gps_hrng	bit	gpcs.5		;  High Range Select
gps_lrng	bit	gpcs.4		;  Low Range Select
gps_lvl		_bfield	gpcs,0,4	;  Voltage Level

;----------------------------------------------------------------------------
; PWM

pwmc		sfr	0x30		; PWM Generator Control Register
pwm_en		bit	pwmc.7		;  Enable
pwm_out		bit	pwmc.6		;  Output Value
pwm_pol		bit	pwmc.5		;  Output Polarity
pwm_res		bit	pwmc.4		;  Counter Reset
pwm_osel	_bfield	pwmc,1,3	;  Output Pin Selection
pwm_clksrc	bit	pwmc.0		;  Clock Source

pwms		sfr	0x31		; PWM Scalar Register
pwm_imode	bit	pwms,7		;  Interrupt Mode
pwm_prescal	_bfield	pwms,5,2	;  Prescaler
pwm_clkdiv	_bfield	pwms,0,5	;  Clock Divider

pwmgcubh	sfr	0x1a		; PWM Counter Upper Bound High Register
pwmgcubl	sfr	0x1b		; PWM Counter Upper Bound Low Register
pwmgdth		sfr	0x32		; PWM Duty Value High Register
pwmgdtl		sfr	0x33		; PWM Duty Value Low Register

pwmptr0		sfr	0x27		; PWM Protect Register 0 (write once)
pwm0_lowdest	_bfield	pwmptr0,5,3	;  PWM Low Side Output Destination
pwm0_prpol	bit	pwmptr0,4	;  PWM Protect Polarity
pwm0_hisel	_bfield	pwmptr0,0,4	;  PWM Hi-Side Selected Pin

pwmptr1		sfr	0x28		; PWM Protect Register 1 (write once)
pwm1_lowdest	_bfield	pwmptr1,5,3	;  PWM Low Side Output Destination
pwm1_prpol	bit	pwmptr1,4	;  PWM Protect Polarity
pwm1_hisel	_bfield	pwmptr1,0,4	;  PWM Hi-Side Selected Pin

;----------------------------------------------------------------------------
; Pulse Capture

plscc		sfr	0x34		; Pulse Capture Control Register
plscc_start	bit	plscc.7		;  Start Pulse Capture
plscc_ovr	bit	plscc.6		;  Pulse Capture Overflow
plscc_fr_eg	bit	plscc.5		;  Front Edge Selection
plscc_bk_eg	bit	plscc.4		;  Back Edge Selection
plscc_src	_bfield	plscc,0,3	;  Source Selection

plscs		sfr	0x35		; Pulse Capture Scaler Register
pls_clksrc	_bfield	plscs,2,2	;  Clock Source
pls_clkdiv	_bfield	plscs,0,2	;  Calpture Clock Divider

plsrh		sfr	0x36		; Pulse Capture Result High
plsrl		sfr	0x37		; Pulse Capture Result Low

		restore
		endif			; __df69inc
