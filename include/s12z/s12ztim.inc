		ifndef	__s12z_tim_inc
__s12z_tim_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZTIM.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Timer                     *
;*                                                                          *
;****************************************************************************

		padding	off

S12ZTIM		struct	dots
TIOS		ds.b	1		; ($00) Timer Input Capture/Output Compare Select
IOS7		s12zbit	TIOS,7		;  Timer Channel n Input Capture or Output Compare Channel Configuration
IOS6		s12zbit	TIOS,6
IOS5		s12zbit	TIOS,5
IOS4		s12zbit	TIOS,4
IOS3		s12zbit	TIOS,3
IOS2		s12zbit	TIOS,2
IOS1		s12zbit	TIOS,1
IOS0		s12zbit	TIOS,0
CFORC		ds.b	1		; ($01) Timer Compare Force Register
FOC7		s12zbit	CFORC,7		;  Timer Channel n Force Output Compare Action
FOC6		s12zbit	CFORC,6
FOC5		s12zbit	CFORC,5
FOC4		s12zbit	CFORC,4
FOC3		s12zbit	CFORC,3
FOC2		s12zbit	CFORC,2
FOC1		s12zbit	CFORC,1
FOC0		s12zbit	CFORC,0
OC7M		ds.b	1		; ($02) Timer Output Compare 7 Mask Register
OC7D		ds.b	1		; ($03) Timer Output Compare 7 Data Register
TCNT		ds.w	0		; ($04) Timer Count Register (16 bit)
TCNTH		ds.b	1		; ($04) Timer Count Register MSB
TCNTL		ds.b	1		; ($05) Timer Count Register LSB
TSCR1		ds.b	1		; ($06)Timer System Control Register 1
TEN		s12zbit	TSCR1,7		;  Timer Enable
TSWAI		s12zbit	TSCR1,6		;  Timer Module Stops While in Wait
TSFRZ		s12zbit	TSCR1,5		;  Timer Stops While in Freeze Mode
TFFCA		s12zbit	TSCR1,4		;  Timer Fast Flag Clear All
PRNT		s12zbit	TSCR1,3		;  Precision Timer
TTOV		ds.b	1		; ($07) Timer Toggle On Overflow Register 1
TOV7		s12zbit	TTOV,7		;  Toggle On Overflow Bits
TOV6		s12zbit	TTOV,6
TOV5		s12zbit	TTOV,5
TOV4		s12zbit	TTOV,4
TOV3		s12zbit	TTOV,3
TOV2		s12zbit	TTOV,2
TOV1		s12zbit	TTOV,1
TOV0		s12zbit	TTOV,0
TCTL1		ds.b	1		; ($08) Timer Control Register 1
OM7		s12zbit	TCTL1,7		;  Output Mode n
OL7		s12zbit	TCTL1,6		;  Output Level n
OM6		s12zbit	TCTL1,5
OL6		s12zbit	TCTL1,4
OM5		s12zbit	TCTL1,3
OL5		s12zbit	TCTL1,2
OM4		s12zbit	TCTL1,1
OL4		s12zbit	TCTL1,0
TCTL2		ds.b	1		; ($09) Timer Control Register 2
OM3		s12zbit	TCTL2,7
OL3		s12zbit	TCTL2,6
OM2		s12zbit	TCTL2,5
OL2		s12zbit	TCTL2,4
OM1		s12zbit	TCTL2,3
OL1		s12zbit	TCTL2,2
OM0		s12zbit	TCTL2,1
OL0		s12zbit	TCTL2,0
TCTL3		ds.b	1		; ($0a) Timer Control Register 3
EDG7B		s12zbit	TCTL3,7		;  Input Capture Edge Control n
EDG7A		s12zbit	TCTL3,6
EDG6B		s12zbit	TCTL3,5
EDG6A		s12zbit	TCTL3,4
EDG5B		s12zbit	TCTL3,3
EDG5A		s12zbit	TCTL3,2
EDG4B		s12zbit	TCTL3,1
EDG4A		s12zbit	TCTL3,0
TCTL4		ds.b	1		; ($0b) Timer Control Register 4
EDG3B		s12zbit	TCTL4,7
EDG3A		s12zbit	TCTL4,6
EDG2B		s12zbit	TCTL4,5
EDG2A		s12zbit	TCTL4,4
EDG1B		s12zbit	TCTL4,3
EDG1A		s12zbit	TCTL4,2
EDG0B		s12zbit	TCTL4,1
EDG0A		s12zbit	TCTL4,0
TIE		ds.b	1		; ($0c) Timer Interrupt Enable Register
C7I		s12zbit	TIE,7		;  Input Capture/Output Compare n Interrupt Enable
C6I		s12zbit	TIE,6
C5I		s12zbit	TIE,5
C4I		s12zbit	TIE,4
C3I		s12zbit	TIE,3
C2I		s12zbit	TIE,2
C1I		s12zbit	TIE,1
C0I		s12zbit	TIE,0
TSCR2		ds.b	1		; ($0d) Timer System Control Register 2
TOI		s12zbit	TSCR2,7		;  Timer Overflow Interrupt Enable
TCRE		s12zbit	TSCR2,3		;  Timer Counter Reset Enable
PR		s12zfld	TSCR2,3:0	;  Timer Prescaler Select
TFLG1		ds.b	1		; ($0e) Timer Main Interrupt Flag 1
C7F		s12zbit	TFLG1,7		;  Input Capture/Output Compare Channel n Flag
C6F		s12zbit	TFLG1,6
C5F		s12zbit	TFLG1,5
C4F		s12zbit	TFLG1,4
C3F		s12zbit	TFLG1,3
C2F		s12zbit	TFLG1,2
C1F		s12zbit	TFLG1,1
C0F		s12zbit	TFLG1,0
TFLG2		ds.b	1		; ($0f) Timer Main Interrupt Flag  2
TOF		s12zbit	TFLG2,7		;  Timer Overflow Flag
TC0		ds.w	0		; ($10) Timer Input Capture/Output Compare Register 0 (16 bit)
TC0H		ds.b	1		; ($10) Timer Input Capture/Output Compare Register 0 MSB
TC0L		ds.b	1		; ($11) Timer Input Capture/Output Compare Register 0 LSB
TC1		ds.w	0		; ($12) Timer Input Capture/Output Compare Register 1 (16 bit)
TC1H		ds.b	1		; ($12) Timer Input Capture/Output Compare Register 1 MSB
TC1L		ds.b	1		; ($13) Timer Input Capture/Output Compare Register 1 LSB
TC2		ds.w	0		; ($14) Timer Input Capture/Output Compare Register 2 (16 bit)
TC2H		ds.b	1		; ($14) Timer Input Capture/Output Compare Register 2 MSB
TC2L		ds.b	1		; ($15) Timer Input Capture/Output Compare Register 2 LSB
TC3		ds.w	0		; ($16) Timer Input Capture/Output Compare Register 3 (16 bit)
TC3H		ds.b	1		; ($16) Timer Input Capture/Output Compare Register 3 MSB
TC3L		ds.b	1		; ($17) Timer Input Capture/Output Compare Register 3 LSB
TC4		ds.w	0		; ($18) Timer Input Capture/Output Compare Register 4 (16 bit)
TC4H		ds.b	1		; ($18) Timer Input Capture/Output Compare Register 4 MSB
TC4L		ds.b	1		; ($19) Timer Input Capture/Output Compare Register 4 LSB
TC5		ds.w	0		; ($1a) Timer Input Capture/Output Compare Register 5 (16 bit)
TC5H		ds.b	1		; ($1a) Timer Input Capture/Output Compare Register 5 MSB
TC5L		ds.b	1		; ($1b) Timer Input Capture/Output Compare Register 5 LSB
TC6		ds.w	0		; ($1c) Timer Input Capture/Output Compare Register 6 (16 bit)
TC6H		ds.b	1		; ($1c) Timer Input Capture/Output Compare Register 6 MSB
TC6L		ds.b	1		; ($1d) Timer Input Capture/Output Compare Register 6 LSB
TC7		ds.w	0		; ($1e) Timer Input Capture/Output Compare Register 7 (16 bit)
TC7H		ds.b	1		; ($1e) Timer Input Capture/Output Compare Register 7 MSB
TC7L		ds.b	1		; ($1f) Timer Input Capture/Output Compare Register 7 LSB
PACTL		ds.b	1		; ($20) Timer 16-Bit Pulse Accumulator Control Register
PAEN		s12zbit	PACTL,6		;  Pulse Accumulator System Enable
PAMOD		s12zbit	PACTL,5		;  Pulse Accumulator Mode
PEDGE		s12zbit	PACTL,4		;  Pulse Accumulator Edge Control
CLK		s12zfld	PACTL,2:2	;  Clock Select Bits
PAOVI		s12zbit	PACTL,1		;  Pulse Accumulator Overflow Interrupt Enable
PAI		s12zbit	PACTL,0		;  Pulse Accumulator Input Interrupt Enable
PAFLG		ds.b	1		; ($21) Timer Pulse Accumulator Flag Register
PAOVF		s12zbit	PAFLG,1		;  Pulse Accumulator Overflow Flag
PAIF		s12zbit	PAFLG,0		;  Pulse Accumulator Input edge Flag
PACNT		ds.w	0		; ($22) Timer Pulse Accumulators Count Register (16 bit)
PACNTH		ds.b	1		; ($22) Timer Pulse Accumulators Count Register MSB
PACNTL		ds.b	1		; ($23) Timer Pulse Accumulators Count Register LSB
		ds.b	8
OCPD		ds.b	1		; ($2c) Timer Output Compare Pin Disconnect Register
OCPD7		s12zbit	OCPD,7		;  Output Compare Pin Disconnect n
OCPD6		s12zbit	OCPD,6
OCPD5		s12zbit	OCPD,5
OCPD4		s12zbit	OCPD,4
OCPD3		s12zbit	OCPD,3
OCPD2		s12zbit	OCPD,2
OCPD1		s12zbit	OCPD,1
OCPD0		s12zbit	OCPD,0
		ds.b	1
PTPSR		ds.b	1		; ($2e) Timer Precision Timer Prescaler Select Register
PTPS		s12zfld	PTPSR,8:0	;  Precision Timer Prescaler Select
		ends	S12ZTIM

		restore			; re-enable listing

		endif			; __s12z_tim_inc
