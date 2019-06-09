		ifndef	__stm8lrtcinc	; avoid multiple inclusion
__stm8lrtcinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File RTC.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L RTC                         *
;*                                                                          *
;****************************************************************************

__defrtc	macro	Base
RTC_TR1		label	Base+$00	; RTC Time register 1
RTC_ST		bfield	RTC_TR1,4,3	;  Second tens in BCD format
RTC_SU		bfield	RTC_TR1,0,4	;  Second units in BCD format
RTC_TR2		label	Base+$01	; RTC Time register 2
RTC_MNT		bfield	RTC_TR2,4,3	;  Minute tens in BCD format
RTC_MNU		bfield	RTC_TR2,0,4	;  Minute units in BCD format
RTC_TR3		label	Base+$02	; RTC Time register 3
RTC_PM		bit	RTC_TR3,7	;  AM/PM notation
RTC_HT		bfield	RTC_TR3,4,2	;  Hour tens in BCD format
RTC_HU		bfield	RTC_TR3,0,4	;  Hour units in BCD format
RTC_DR1		label	Base+$04	; RTC Date register 1
RTC_DT		bfield	RTC_DR1,4,2	;  Date tens in BCD format
RTC_DU		bfield	RTC_DR1,0,4	;  Date units in BCD format
RTC_DR2		label	Base+$05	; RTC Date register 2
RTC_WDU		bfield	RTC_DR2,5,3	;  Week day units
RTC_MT		bit	RTC_DR2,4	;  Month tens in BCD format
RTC_MU		bfield	RTC_DR2,0,4	;  Month units in BCD format
RTC_DR3		label	Base+$06	; RTC Date register 3
RTC_YT		bfield	RTC_DR3,4,4	;  Year tens in BCD format
RTC_YU		bfield	RTC_DR3,0,4	;  Year units in BCD format
RTC_CR1		label	Base+$08	; RTC Control register 1
RTC_FMT		bit	RTC_CR1,6	;  Hour format
RTC_RATIO	bit	RTC_CR1,5	;  System clock (SYSCLK) versus RTCCLK ratio
RTC_BYPSHAD	bit	RTC_CR1,4	;  Bypass the shadow registers
RTC_WUCKSEL	bfield	RTC_CR1,0,3	;  Wakeup clock selection
RTC_CR2		label	Base+$09	; RTC Control register 2
RTC_WUTIE	bit	RTC_CR2,6	;  Wakeup timer interrupt enable
RTC_ALRAIE	bit	RTC_CR2,4	;  Alarm A interrupt enable
RTC_WUTE	bit	RTC_CR2,2	;  Wakeup timer enable
RTC_ALRAE	bit	RTC_CR2,0	;  Alarm A enable
RTC_CR3		label	Base+$0a	; RTC Control register 3
RTC_COE		bit	RTC_CR3,7	;  Calibration output enable
RTC_OSEL	bfield	RTC_CR3,5	;  Output selection
RTC_POL		bit	RTC_CR3,4	;  Output polarity
RTC_COSEL	bit	RTC_CR3,3	;  Calibration output selection
RTC_BCK		bit	RTC_CR3,2	;  Backup
RTC_SUB1H	bit	RTC_CR3,1	;  Subtract 1 hour (winter time change)
RTC_ADD1H	bit	RTC_CR3,0	;  Add 1 hour (summer time change)
RTC_ISR1	label	Base+$0c	; RTC Initialization and status register 1
RTC_INIT	bit	RTC_ISR1,7	;  Initialization mode
RTC_INITF	bit	RTC_ISR1,6	;  Initialization flag
RTC_RSF		bit	RTC_ISR1,5	;  Registers synchronization flag
RTC_INITS	bit	RTC_ISR1,4	;  Initialization status flag
RTC_SHPF	bit	RTC_ISR1,3	;  Shift operation pending
RTC_WUTWF	bit	RTC_ISR1,2	;  Wakeup timer write flag
RTC_RECALPF	bit	RTC_ISR1,1	;  Recalibration pending Flag
RTC_ALRAWF	bit	RTC_ISR1,0	;  Alarm A write flag
RTC_ISR2	label	Base+$0d	; RTC Initialization and Status register 2
RTC_TAMP3F	bit	RTC_ISR2,7	;  TAMPER3 detection flag
RTC_TAMP2F	bit	RTC_ISR2,6	;  TAMPER2 detection flag
RTC_TAMP1F	bit	RTC_ISR2,5	;  TAMPER1 detection flag
RTC_WUTF	bit	RTC_ISR2,2	;  Periodic wakeup flag
RTC_ALRAF	bit	RTC_ISR2,0	;  Alarm A Flag
RTC_SPRERH	label	Base+$10	; RTC Synchronous prescaler register high
RTC_SPRERL	label	Base+$11	; RTC Synchronous prescaler register low
RTC_APRER	label	Base+$12	; RTC Asynchronous prescaler register
RTC_WUTRH	label	Base+$14	; RTC Wakeup timer register high
RTC_WUTRL	label	Base+$15	; RTC Wakeup timer register low
RTC_SSRL	label	Base+$17	; RTC Subsecond register low
RTC_SSRH	label	Base+$18	; RTC Subsecond register high
RTC_WPR		label	Base+$19	; RTC Write protection register
RTC_SHIFTRH	label	Base+$1a	; RTC Shift register high
RTC_ADD1S	bit	RTC_SHIFTRH,7	;  Add one second
RTC_SHIFTRL	label	Base+$1b	; RTC Shift register low
RTC_ALRMAR1	label	Base+$1c	; RTC Alarm A register 1
RTC_MSK1	bit	RTC_ALRMAR1,7	;  Alarm A Seconds mask
RTC_AL_ST	bfield	RTC_ALRMAR1,4,3	;  Second tens in BCD format
RTC_AL_SU	bfield	RTC_ALRMAR1,0,4	;  Second units in BCD format
RTC_ALRMAR2	label	Base+$1d	; RTC Alarm A register 2
RTC_MSK2	bit	RTC_ALRMAR2,7	;  Alarm A minutes mask
RTC_AL_MNT	bfield	RTC_ALRMAR2,4,3	;  Minute tens in BCD format
RTC_AL_MNU	bfield	RTC_ALRMAR2,0,4	;  Minute units in BCD format
RTC_ALRMAR3	label	Base+$1e	; RTC Alarm A register 3
RTC_MSK3	bit	RTC_ALRMAR3,7	;  Alarm A hours mask
RTC_AL_PM	bit	RTC_ALRMAR3,6	;  AM/PM notation
RTC_AL_HT	bfield	RTC_ALRMAR3,4,2	;  Hour tens in BCD format
RTC_AL_HU	bfield	RTC_ALRMAR3,0,4	;  Hour units in BCD format
RTC_ALRMAR4	label	Base+$1f	; RTC Alarm A register 4
RTC_MSK4	bit	RTC_ALRMAR4,7	;  Alarm A Date mask
RTC_WDSEL	bit	RTC_ALRMAR4,6	;  Week day selection
RTC_AL1_DT	bfield	RTC_ALRMAR4,4,2	;  Date tens in BCD format
RTC_AL1_DU	bfield	RTC_ALRMAR4,0,4	;  Date units or Day in BCD format
RTC_ALRMASSRH	label	Base+$24	; RTC Alarm A subsecond register high
RTC_ALRMASSRL	label	Base+$25	; RTC Alarm A subsecond register low
RTC_ALRMASSMSKR	label	Base+$26	; RTC Alarm A masking register
RTC_CALRH	label	Base+$2a	; RTC Calibration register high
RTC_CALP	bit	RTC_CALRH,7	;  Increase of RTC frequency by 488.5 ppm
RTC_CALW8	bit	RTC_CALRH,6	;  Use an 8-second calibration cycle period
RTC_CALW16	bit	RTC_CALRH,5	;  Use a16-second calibration cycle period
RTC_CALRL	label	Base+$2b	; RTC Calibration register low
RTC_TCR1	label	Base+$2c	; RTC Tamper control register 1
RTC_TAMP3TRG	bit	RTC_TCR1,6	;  Active level for tamper input 3
RTC_TAMP3E	bit	RTC_TCR1,5	;  Tamper detection enable for tamper input 3
RTC_TAMP2TRG	bit	RTC_TCR1,4	;  Active level for tamper input 2
RTC_TAMP2E	bit	RTC_TCR1,3	;  Tamper detection enable for tamper input 2
RTC_TAMP1TRG	bit	RTC_TCR1,2	;  Active level for tamper 1
RTC_TAMP1E	bit	RTC_TCR1,1	;  Tamper detection enable for tamper input 1
RTC_TAMPIE	bit	RTC_TCR1,0	;  Tamper interrupt enable
RTC_TCR2	label	Base+$2d	; RTC Tamper control register 2
RTC_TTAMPPUDIS	bit	RTC_TCR2,7	;  TAMPER pull-up disable
RTC_TTAMPPRCH	bfield	RTC_TCR2,5,2	;  Tamper precharge duration
RTC_TTAMPFLT	bfield	RTC_TCR2,3,2	;  Tamper filter count
RTC_TTAMPFREQ	bfield	RTC_TCR2,0,2	;  Tamper sampling frequency
CSSLSE_CSR	label	Base+$50	; CSS on LSE control and status register
SWITCHF		bit	CSSLSE_CSR,4	;  RTC clock switch flag
CSSF		bit	CSSLSE_CSR,3	;  CSS on LSE flag
CSSIE		bit	CSSLSE_CSR,2	;  Clock security system on LSE interrupt enable
SWITCHEN	bit	CSSLSE_CSR,1	;  RTC clock switch to LSI in case of LSE failure enable
LSE_CSSEN	bit	CSSLSE_CSR,0	;  Clock security system on LSE enable
		endm

		restore
		endif			; __stm8lrtcinc
