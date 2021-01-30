		ifndef	__pms133inc	; avoid multiple inclusion
__pms133inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File PMS133.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for PMS133/PMS134                     *
;*                                                                          *
;*   Source: PMS133/PMS134 Data Sheet, Ver. 1.03, Nov. 13, 2018             *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Interrupt Control

inten		sfr	0x04		; Interrupt Enable
timer3_inten	bit	inten.7		;  Timer3 Interrupt Enable
timer2_inten	bit	inten.6		;  Timer2 Interrupt Enable
pwmg0_inten	bit	inten.5		;  PWMG0 Interrupt Enable
comp_inten	bit	inten.4		;  Comparator Interrupt Enable
adc_inten	bit	inten.3		;  ADC Interrupt Enable
timer16_inten	bit	inten.2		;  Timer16 Interrupt Enable
pb0_inten	bit	inten.1		;  PB0/PA4/PA3/PA6 Interrupt Enable
pa0_inten	bit	inten.0		;  PA0/PB5/PA2/PA7 Interrupt Enable

intrq		sfr	0x05		; Interrupt Request Register
timer3_intrq	bit	intrq.7		;  Timer3 Interrupt Request
timer2_intrq	bit	intrq.6		;  Timer2 Interrupt Request
pwmg0_intrq	bit	intrq.5		;  PWMG0 Interrupt Request
comp_intrq	bit	intrq.4		;  Comparator Interrupt Request
adc_intrq	bit	intrq.3		;  ADC Interrupt Request
timer16_intrq	bit	intrq.2		;  Timer16 Interrupt Request
pb0_intrq	bit	intrq.1		;  PB0/PA4/PA3/PA6 Interrupt Request
pa0_intrq	bit	intrq.0		;  PA0/PB5/PA2/PA7 Interrupt Request
	
integs		sfr	0x0c		; Interrupt Edge Register
timer16_egs	bit	integs.4	;  Timer16 Edge Selection
pb0_egs		_bfield	integs,2,2	;  PB0/PA4/PA3/PA6 Edge Selection
pa0_egs		_bfield	integs,0,2	;  PA0/PB5/PA2/PA7 Edge Selection

;----------------------------------------------------------------------------
; CPU Core

clkmd		sfr	0x03		; Clock Mode Register
clkselect	_bfield	clkmd,5,3	;  System Clock Selection
ihrc_enable	bit	clkmd.4		;  IHRC Enable
clktype		bit	clkmd.3		;  Clock Type Select
ilrc_enable	bit	clkmd.2		;  ILRC Enable
wd_enable	bit	clkmd.1		;  Watch Dog Enable
pa5_prst	bit	clkmd.0		;  Pin PA5/PRSTB Function

eoscr		sfr	0x0a		; External Oscillator Setting Register
enxtal		bit	eoscr.7		;  Enable external crystal
xtalsel		_bfield	eoscr,5,2	;  External Crystal Oscillator Selection
pwrdn		bit	eoscr.0		;  Power Down Band Gap and LVR Hardware

misc		sfr	0x26		; MISC Register
en_fwkup	bit	misc.5		;  Enable Fast Wakeup
en_vdd2_gen	bit	misc.4		;  Enable VDD2/2 Bias Voltage Generator
dis_lvr		bit	misc.2		;  Disable LVR Function
wdperiod	_bfield	misc,0,2	;  Watchdog Timeout Period

;----------------------------------------------------------------------------
; Multiplier

mulop		sfr	0x08		; Multiplier Operand Register
mulrh		sfr	0x09		; Multiplier Result High Byte Register (ro)

;----------------------------------------------------------------------------
; GPIO

padier		sfr	0x0d		; Port A Digital Input Enable Register
pbdier		sfr	0x0e		; Port B Digital Input Enable Register
pcdier		sfr	0x0f		; Port C Digital Input Enable Register

pa		sfr	0x10		; Port A Data Register
pb		sfr	0x13		; Port B Data Register
pc		sfr	0x16		; Port C Data Register

pac		sfr	0x11		; Port A Control Register
pbc		sfr	0x14		; Port B Control Register
pcc		sfr	0x17		; Port C Control Register

paph		sfr	0x12		; Port A Pull High Register
pbph		sfr	0x15		; Port B Pull High Register
pcph		sfr	0x18		; Port C Pull High Register

;----------------------------------------------------------------------------
; Comparator

gpcc		sfr	0x2b		; Comparator Control
gpcc_en		bit	gpcc.7		;  Enable
gpcc_res	bit	gpcc.6		;  Result
gpcc_samp	bit	gpcc.5		;  Output sampled by TM2_CLK?
gpcc_pol	bit	gpcc.4		;  Output Polarity Selection
gpcc_minp	_bfield	gpcc,1,3	;  Minus Input Selection
gpcc_pinp	bit	gpcc.0		;  Plus Input Selection

gpcs		sfr	0x2c		; Comparator Selection Register
gps_oe		bit	gpcs.7		;  Output Enable
gps_hrng	bit	gpcs.5		;  High Range Select
gps_lrng	bit	gpcs.4		;  Low Range Select
gps_lvl		_bfield	gpcs,0,4	;  Voltage Level

;----------------------------------------------------------------------------
; ADC

adcc		sfr	0x20		; ADC Control Register
adc_en		bit	adcc.7		;  Enable ADC
adc_pr_ctl	bit	adcc.6		;  ADC Process Control Bit
adc_chsel	_bfield	adcc,2,4	;  ADC Channel Select

adcrgc		sfr	0x24		; ADC Regulator Control Register
adc_refsel	_bfield	adcrgc,5,3	;  Reference Select
adc_chfsel	bit	adcrgc,4	;  ADC Channel F Select
adc_bgsel	_bfield	adcrgc,2,2	;  Band Gap Reference Voltage Select

adcm		sfr	0x21		; ADC Mode Register
adc_clksel	_bfield	adcm,1,3	;  Clock Source Select

adcrh		sfr	0x22		; ADC Result High Register
adcrl		sfr	0x23		; ADC Result Low Register

;----------------------------------------------------------------------------
; Timer

t16m		sfr	0x06		; Timer 16 Mode Register
tm16_clksrc	_bfield	t16m,5,3	;  Timer Clock Source Selection
tm16_clkdiv	_bfield t16m,3,2	;  Internal Clock Divider
tm16_isrc	_bfield	t16m,0,3	;  Interrupt Source

tm2c		sfr	0x30		; Timer2 Control Register
tm2_clksel	_bfield	tm2c,4,4	;  Clock Selection
tm2_outsel	_bfield	tm2c,2,2	;  Output Selection
tm2_mode	bit	tm2c.1		;  Mode Selection
tm2_pol		bit	tm2c.0		;  Inverse Polarity of Output
tm2ct		sfr	0x31		; Timer 2 Counter Register
tm2s		sfr	0x32		; Timer 2 Scaler Register
tm2_pwmsel	bit	tm2s,7		;  PWM Resolution Selection
tm2_prescal	_bfield	tm2s,5,2	;  Clock Prescaler
tm2_clkscal	_bfield	tm2s,0,5	;  Clock Scaler
tm2b		sfr	0x33		; Timer 2 Bound Register (w/only, read is multiplier result)

tm3c		sfr	0x34		; Timer3 Control Register
tm3_clksel	_bfield	tm3c,4,4	;  Clock Selection
tm3_outsel	_bfield	tm3c,2,2	;  Output Selection
tm3_mode	bit	tm3c.1		;  Mode Selection
tm3_pol		bit	tm3c.0		;  Inverse Polarity of Output
tm3ct		sfr	0x35		; Timer 3 Counter Register
tm3s		sfr	0x36		; Timer 3 Scaler Register
tm3_pwmsel	bit	tm3s,7		;  PWM Resolution Selection
tm3_prescal	_bfield	tm3s,5,2	;  Clock Prescaler
tm3_clkscal	_bfield	tm3s,0,5	;  Clock Scaler
tm3b		sfr	0x37		; Timer 3 Bound Register

;----------------------------------------------------------------------------
; PWM

pwmg0c		sfr	0x40		; PWMG0 Control Register
pwm0_en		bit	pwmg0c.7	;  Enable
pwm0_out	bit	pwmg0c.6	;  Output Value
pwm0_pol	bit	pwmg0c.5	;  Output Polarity
pwm0_res	bit	pwmg0c.4	;  Counter Reset
pwm0_osel	_bfield	pwmg0c,1,3	;  Output Selection
pwm0_clksrc	bit	pwmg0c.0	;  Clock Source

pwmg0s		sfr	0x41		; PWMG0 Scalar Register
pwm0_imode	bit	pwmg0s,7	;  Interrupt Mode
pwm0_prescal	_bfield	pwmg0s,5,2	;  Prescaler
pwm0_clkdiv	_bfield	pwmg0s,0,5	;  Clock Divider

pwmg0cubh	sfr	0x44		; PWMG0 Counter Upper Bound High Register
pwmg0cubl	sfr	0x45		; PWMG0 Counter Upper Bound Low Register
pwmg0dth	sfr	0x42		; PWMG0 Duty Value High Register
pwmg0dtl	sfr	0x43		; PWMG0 Duty Value Low Register

pwmg1c		sfr	0x46		; PWMG1 Control Register
pwm1_en		bit	pwmg1c.7	;  Enable
pwm1_out	bit	pwmg1c.6	;  Output Value
pwm1_pol	bit	pwmg1c.5	;  Output Polarity
pwm1_res	bit	pwmg1c.4	;  Counter Reset
pwm1_osel	_bfield	pwmg1c,1,3	;  Output Selection
pwm1_clksrc	bit	pwmg1c.0	;  Clock Source

pwmg1s		sfr	0x47		; PWMG1 Scalar Register
pwm1_imode	bit	pwmg1s,7	;  Interrupt Mode
pwm1_prescal	_bfield	pwmg1s,5,2	;  Prescaler
pwm1_clkdiv	_bfield	pwmg1s,0,5	;  Clock Divider

pwmg1cubh	sfr	0x4a		; PWMG1 Counter Upper Bound High Register
pwmg1cubl	sfr	0x4b		; PWMG1 Counter Upper Bound Low Register
pwmg1dth	sfr	0x48		; PWMG1 Duty Value High Register
pwmg1dtl	sfr	0x49		; PWMG1 Duty Value Low Register

pwmg2c		sfr	0x4c		; PWMG2 Control Register
pwm2_en		bit	pwmg2c.7	;  Enable
pwm2_out	bit	pwmg2c.6	;  Output Value
pwm2_pol	bit	pwmg2c.5	;  Output Polarity
pwm2_res	bit	pwmg2c.4	;  Counter Reset
pwm2_osel	_bfield	pwmg2c,1,3	;  Output Selection
pwm2_clksrc	bit	pwmg2c.0	;  Clock Source

pwmg2s		sfr	0x4d		; PWMG2 Scalar Register
pwm2_imode	bit	pwmg2s,7	;  Interrupt Mode
pwm2_prescal	_bfield	pwmg2s,5,2	;  Prescaler
pwm2_clkdiv	_bfield	pwmg2s,0,5	;  Clock Divider

pwmg2cubh	sfr	0x50		; PWMG2 Counter Upper Bound High Register
pwmg2cubl	sfr	0x51		; PWMG2 Counter Upper Bound Low Register
pwmg2dth	sfr	0x4e		; PWMG2 Duty Value High Register
pwmg2dtl	sfr	0x4f		; PWMG2 Duty Value Low Register

		restore
		endif			; __pms133inc
