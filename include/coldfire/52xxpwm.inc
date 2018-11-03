		ifndef	__52xxpwminc		; avoid multiple inclusion
__52xxpwminc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXPWM.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx PWM Module       *
;*                                                                          *
;****************************************************************************

PWME		equ		MBAR_PWM+$020	; PWM Enable Register (8b)
PWME7		cfbit		PWME,7		;  PWM Channel 7 Enable.
PWME5		cfbit		PWME,5		;  PWM Channel 5 Output Enable.
PWME3		cfbit		PWME,3		;  PWM Channel 3 Output Enable.
PWME1		cfbit		PWME,1		;  PWM Channel 1 Output Enable.
PWMPOL		equ		MBAR_PWM+$021	; PWM Polarity Register (8b)
PPOL7		cfbit		PWMPOL,7	;  PWM channel 7 polarity.
PPOL5		cfbit		PWMPOL,5	;  PWM channel 5 polarity.
PPOL3		cfbit		PWMPOL,3	;  PWM channel 3 polarity.
PPOL1		cfbit		PWMPOL,1	;  PWM channel 1 polarity.
PWMCLK		equ		MBAR_PWM+$022	; PWM Clock Select Register (8b)
PCLK7		cfbit		PWMCLK,7	;  PWM channel 7 clock select.
PCLK5		cfbit		PWMCLK,5	;  PWM channel 5 clock select.
PCLK3		cfbit		PWMCLK,3	;  PWM channel 3 clock select.
PCLK1		cfbit		PWMCLK,1	;  PWM channel 1 clock select.
PWMPRCLK	equ		MBAR_PWM+$023	; PWM Prescale Clock Select Register (8b)
PCKB		cffield		PWMPRCLK,4,3	;  Clock B prescaler select.
PCKA		cffield		PWMPRCLK,0,3	;  Clock A prescaler select.
PWMCAE		equ		MBAR_PWM+$024	; PWM Center Align Enable Register (8b)
CAE7		cfbit		PWMCAE,7	;  Center align enable for channel 7.
CAE5		cfbit		PWMCAE,5	;  Center align enable for channel 5.
CAE3		cfbit		PWMCAE,3	;  Center align enable for channel 3.
CAE1		cfbit		PWMCAE,1	;  Center align enable for channel 1.
PWMCTL		equ		MBAR_PWM+$025	; PWM Control Register (8b)
CON67		cfbit		PWMCTL,7	;  Concatenates PWM channels 6 and 7 to form one 16-bit PWM channel.
CON45		cfbit		PWMCTL,6	;  Concatenates PWM channels 4 and 5 to form one 16-bit PWM channel.
CON23		cfbit		PWMCTL,5	;  Concatenates PWM channels 2 and 3 to form one 16-bit PWM channel.
CON01		cfbit		PWMCTL,4	;  Concatenates PWM channels 0 and 1 to form one 16-bit PWM channel.
PSWAI		cfbit		PWMCTL,3	;  PWM stops in doze mode.
PFRZ		cfbit		PWMCTL,2	;  PWM counters stop in debug mode
PWMSCLA		equ		MBAR_PWM+$028	; PWM Scale A Register (8b)
SCALEA		cffield		PWMSCLA,0,8	;  Part of divisor used to form Clock SA from Clock A.
PWMSCLB		equ		MBAR_PWM+$029	; PWM Scale B Register (8b)
SCALEB		cffield		PWMSCLB,0,8	;  Divisor used to form Clock SB from Clock B.
__N		set		0
		rept		8
		__decstr	__NS,__N
PWMCNT{"\{__NS}"}	equ	MBAR_PWM+$02C+__N	; PWM Channel n Counter Register (8b)
COUNT		cffield		PWMCNT{"\{__NS}"},0,8	;  Current value of the PWM up counter.
PWMPER{"\{__NS}"}	equ	MBAR_PWM+$034+__N	; PWM Channel n Period Register (8b)
PERIOD		cffield		PWMPER{"\{__NS}"},0,8	;  Period counter for the output PWM signal.
PWMDTY{"\{__NS}"}	equ	MBAR_PWM+$03C+__N	; PWM Channel n Duty Register (8b)
DUTY		cffield		PWMDTY{"\{__NS}"},0,8	;  Contains the duty value used to determine when a transition occurs on the PWM output signal
__N		set		__N+1
		endm
PWMSDN		equ		MBAR_PWM+$044	; PWM Shutdown Register (8b)
IF		cfbit		PWMSDN,7	;  PWM interrupt flag.
IE		cfbit		PWMSDN,6	;  PWM interrupt enable.
RESTART		cfbit		PWMSDN,5	;  PWM restart.
LVL		cfbit		PWMSDN,4	;  PWM shutdown output level.
PWM7IN		cfbit		PWMSDN,2	;  PWM channel 7 input status.
PWM7IL		cfbit		PWMSDN,1	;  PWM channel 7 input polarity.
SDNEN		cfbit		PWMSDN,0	;  PWM emergency shutdown enable.

		restore				; re-enable listing

                endif                           ; __52xxpwminc
