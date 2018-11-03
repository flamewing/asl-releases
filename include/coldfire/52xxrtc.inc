		ifndef	__52xxrtcinc		; avoid multiple inclusion
__52xxrtcinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXRTC.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Real Time Clock  *
;*                                                                          *
;****************************************************************************

RTC_HOURMIN	equ		MBAR_RTC+$000	; RTC Hours and Minutes Counter Register (32b)
HOURS		cffield		RTC_HOURMIN,8,5	;  Current hour.
MINUTES		cffield		RTC_HOURMIN,0,6	;  Current minutes.
RTC_SECONDS	equ		MBAR_RTC+$004	; RTC Seconds Counter Register (32b)
SECONDS		cffield		RTC_SECONDS,0,6	;  Current seconds.
RTC_ALRM_HM	equ		MBAR_RTC+$008	; RTC Hours and Minutes Alarm Register (32b)
HOURS		cffield		RTC_ALRM_HM,8,5	;  Hours setting of the alarm.
MINUTES		cffield		RTC_ALRM_HM,0,6	;  Minutes setting of the alarm.
RTC_ALRM_SEC	equ		MBAR_RTC+$00C	; RTC Seconds Alarm Register (32b)
SECONDS		cffield		RTC_ALRM_SEC,0,6;  Seconds setting of the alarm.
RTC_CR		equ		MBAR_RTC+$010	; RTC Control Register (32b)
EN		cfbit		RTC_CR,7	;  RTC enable.
SWR		cfbit		RTC_CR,0	;  Software reset.
RTC_ISR		equ		MBAR_RTC+$014	; RTC Interrupt Status Register (32b)
SAM7		cfbit		RTC_ISR,15	;  Sampling timer 7-0 interrupt flags.
SAM6		cfbit		RTC_ISR,14
SAM5		cfbit		RTC_ISR,13
SAM4		cfbit		RTC_ISR,12
SAM3		cfbit		RTC_ISR,11
SAM2		cfbit		RTC_ISR,10
SAM1		cfbit		RTC_ISR,9
SAM0		cfbit		RTC_ISR,8
2HZ		cfbit		RTC_ISR,7	;  2 Hz interrupt flag.
HR		cfbit		RTC_ISR,5	;  Hour interrupt flag.
1HZ		cfbit		RTC_ISR,4	;  1 Hz interrupt flag.
DAY		cfbit		RTC_ISR,3	;  Day interrupt flag.
ALM		cfbit		RTC_ISR,2	;  Alarm interrupt flag.
MIN		cfbit		RTC_ISR,1	;  If enabled, this bit is set on every increment of the minute counter in the RTC_HOURMIN register.
SW		cfbit		RTC_ISR,0	;  Stopwatch flag.
RTC_IER		equ		MBAR_RTC+$018	; RTC Interrupt Enable Register (32b)
SAM7		cfbit		RTC_IER,15	;  Sampling timer 7-0 interrupt enable.
SAM6		cfbit		RTC_IER,14
SAM5		cfbit		RTC_IER,13
SAM4		cfbit		RTC_IER,12
SAM3		cfbit		RTC_IER,11
SAM2		cfbit		RTC_IER,10
SAM1		cfbit		RTC_IER,9
SAM0		cfbit		RTC_IER,8
2HZ		cfbit		RTC_IER,7	;  2 Hz interrupt enable.
HR		cfbit		RTC_IER,5	;  Hour interrupt enable.
1HZ		cfbit		RTC_IER,4	;  1 Hz interrupt enable.
DAY		cfbit		RTC_IER,3	;  Day interrupt enable.
ALM		cfbit		RTC_IER,2	;  Alarm interrupt enable.
MIN		cfbit		RTC_IER,1	;  Minute interrupt enable.
SW		cfbit		RTC_IER,0	;  Stopwatch interrupt enable.
RTC_STPWCH	equ		MBAR_RTC+$01C	; Stopwatch Minutes Register (32b)
CNT		cffield		RTC_STPWCH,0,6	;  Stopwatch count.
RTC_DAYS	equ		MBAR_RTC+$020	; RTC Days Counter Register (32b)
DAYS		cffield		RTC_DAYS,0,16	;  Current day count.
RTC_ALRM_DAY	equ		MBAR_RTC+$024	; RTC Days Alarm Register (32b)
DAYS		cffield		RTC_ALRM_DAY,0,16 ; Current day setting of the alarm.

		restore				; re-enable listing

                endif                           ; __52xxrtcinc
