		ifndef	__s12z_pwm_inc
__s12z_pwm_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZPWM.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z PWM                       *
;*                                                                          *
;****************************************************************************

		padding	off

S12ZPWM		STRUCT	dots
PWME		ds.b	1		; ($00) PWM Enable Register
PWME7		s12zbit	PWME,7		;  Enable Pulse Width Channel n
PWME6		s12zbit	PWME,6
PWME5		s12zbit	PWME,5
PWME4		s12zbit	PWME,4
PWME3		s12zbit	PWME,3
PWME2		s12zbit	PWME,2
PWME1		s12zbit	PWME,1
PWME0		s12zbit	PWME,0
PWMPOL		ds.b	1		; ($01) PWM Polarity Register
PPOL7		s12zbit	PWMPOL,7	;  Pulse Width Channel 7-0 n
PPOL6		s12zbit	PWMPOL,6
PPOL5		s12zbit	PWMPOL,5
PPOL4		s12zbit	PWMPOL,4
PPOL3		s12zbit	PWMPOL,3
PPOL2		s12zbit	PWMPOL,2
PPOL1		s12zbit	PWMPOL,1
PPOL0		s12zbit	PWMPOL,0
PWMCLK		ds.b	1		; ($02) PWM Clock Select Register
PCLK7		s12zbit	PWMCLK,7	;  Pulse Width Channel n Clock Select
PCLK6		s12zbit	PWMCLK,6
PCLK5		s12zbit	PWMCLK,5
PCLK4		s12zbit	PWMCLK,4
PCLK3		s12zbit	PWMCLK,3
PCLK2		s12zbit	PWMCLK,2
PCLK1		s12zbit	PWMCLK,1
PCLK0		s12zbit	PWMCLK,0
PWMPRCLK	ds.b	1		; ($03) PWM Prescale Clock Select Register
PCKB		s12zfld	PWMPRCLK,3:4	;  Prescaler Select for Clock B
PCKA		s12zfld	PWMPRCLK,3:0	;  Prescaler Select for Clock A
PWMCAE		ds.b	1		; ($04) PWM Center Align Enable Register
CAE7		s12zbit	PWMCAE,7	;  Center Aligned Output Modes on Channel n
CAE6		s12zbit	PWMCAE,6
CAE5		s12zbit	PWMCAE,5
CAE4		s12zbit	PWMCAE,4
CAE3		s12zbit	PWMCAE,3
CAE2		s12zbit	PWMCAE,2
CAE1		s12zbit	PWMCAE,1
CAE0		s12zbit	PWMCAE,0
PWMCTL		ds.b	1		; ($05) PWM Control Register
CON67		s12zbit	PWMCTL,7	;  Concatenate Channels 6 and 7
CON45		s12zbit	PWMCTL,6	;  Concatenate Channels 4 and 5
CON23		s12zbit	PWMCTL,5	;  Concatenate Channels 2 and 3
CON01		s12zbit	PWMCTL,4	;  Concatenate Channels 0 and 1
PSWAI		s12zbit	PWMCTL,3	;  PWM Stops in Wait Mode
PFRZ		s12zbit	PWMCTL,2	;  PWM Counters Stop in Freeze Mode
PWMCLKAB	ds.b	1		; ($06) PWM Clock A/B Select Register
PCLKAB7		s12zbit	PWMCLKAB,7	;  Pulse Width Channel n Clock A/B Select
PCLKAB6		s12zbit	PWMCLKAB,6
PCLKAB5		s12zbit	PWMCLKAB,5
PCLKAB4		s12zbit	PWMCLKAB,4
PCLKAB3		s12zbit	PWMCLKAB,3
PCLKAB2		s12zbit	PWMCLKAB,2
PCLKAB1		s12zbit	PWMCLKAB,1
PCLKAB0		s12zbit	PWMCLKAB,0
		ds.b	1
PWMSCLA		ds.b	1		; ($08) PWM Scale A Register
PWMSCLB		ds.b	1		; ($09) PWM Scale B Register
		ds.b	2
PWMCNT0		ds.b	1		; ($0c) PWM Channel 0 Counter Register
PWMCNT1		ds.b	1		; ($0d) PWM Channel 1 Counter Register
PWMCNT2		ds.b	1		; ($0e) PWM Channel 2 Counter Register
PWMCNT3		ds.b	1		; ($0f) PWM Channel 3 Counter Register
PWMCNT4		ds.b	1		; ($10) PWM Channel 4 Counter Register
PWMCNT5		ds.b	1		; ($11) PWM Channel 5 Counter Register
PWMCNT6		ds.b	1		; ($12) PWM Channel 6 Counter Register
PWMCNT7		ds.b	1		; ($13) PWM Channel 7 Counter Register
PWMPER0		ds.b	1		; ($14) PWM Channel 0 Period Register
PWMPER1		ds.b	1		; ($15) PWM Channel 1 Period Register
PWMPER2		ds.b	1		; ($16) PWM Channel 2 Period Register
PWMPER3		ds.b	1		; ($17) PWM Channel 3 Period Register
PWMPER4		ds.b	1		; ($18) PWM Channel 4 Period Register
PWMPER5		ds.b	1		; ($19) PWM Channel 5 Period Register
PWMPER6		ds.b	1		; ($1a) PWM Channel 6 Period Register
PWMPER7		ds.b	1		; ($1b) PWM Channel 7 Period Register
PWMDTY0		ds.b	1		; ($1c) PWM Channel 0 Duty Register
PWMDTY1		ds.b	1		; ($1d) PWM Channel 1 Duty Register
PWMDTY2		ds.b	1		; ($1e) PWM Channel 2 Duty Register
PWMDTY3		ds.b	1		; ($1f) PWM Channel 3 Duty Register
PWMDTY4		ds.b	1		; ($20) PWM Channel 4 Duty Register
PWMDTY5		ds.b	1		; ($21) PWM Channel 5 Duty Register
PWMDTY6		ds.b	1		; ($22) PWM Channel 6 Duty Register
PWMDTY7		ds.b	1		; ($23) PWM Channel 7 Duty Register
		ends	S12ZPWM

		restore			; re-enable listing

		endif			; __s12z_tim_inc
