		ifndef	__lcdinc
__lcdinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File LCD.INC                                                 *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega 169/329(0)/649(0) LCD   *
;*                                                                          *
;****************************************************************************

LCDCRA		sfr	0xe4		; LCD Control and Status Register A
LCDEN		avrbit	LCDCRA,7	; LCD Enable
LCDAB		avrbit	LCDCRA,6	; LCD Low Power Waveform
LCDIF		avrbit	LCDCRA,4	; LCD Interrupt Flag
LCDIE		avrbit	LCDCRA,3	; LCD Interrupt Enable
LCDBD		avrbit	LCDCRA,2	; LCD Buffer Disable
LCDCCD		avrbit	LCDCRA,1	; LCD Contrast Control Disable
LCDBL		avrbit	LCDCRA,0	; LCD Blanking

LCDCRB		sfr	0xe5		; LCD Control and Status Register B
LCDCS		avrbit	LCDCRB,7	; LCD Clock Select
LCD2B		avrbit	LCDCRB,6	; LCD 1/2 Bias Select
LCDMUX1		avrbit	LCDCRB,5	; LCD Mux Select
LCDMUX0		avrbit	LCDCRB,4
LCDPM3		avrbit	LCDCRB,3	; LCD Port Mask
LCDPM2		avrbit	LCDCRB,2
LCDPM1		avrbit	LCDCRB,1
LCDPM0		avrbit	LCDCRB,0

LCDFRR		sfr	0xe6		; LCD Frame Rate Register
LCDPS2		avrbit	LCDFRR,6	; LCD Prescaler Select
LCDPS1		avrbit	LCDFRR,5
LCDPS0		avrbit	LCDFRR,4
LCDCD2		avrbit	LCDFRR,2	; LCD Clock Divide
LCDCD1		avrbit	LCDFRR,1
LCDCD0		avrbit	LCDFRR,0

LCDCCR		sfr	0xe7		; LCD Contrast Control Register
LCDDC2		avrbit	LCDCCR,7	; LDC Display Configuration
LCDDC1		avrbit	LCDCCR,6
LCDDC0		avrbit	LCDCCR,5
LCDNDT		avrbit	LCDCCR,4	; LCD Maximum Drive Time
LCDCC3		avrbit	LCDCCR,3	; LCD Contrast Control
LCDCC2		avrbit	LCDCCR,2
LCDCC1		avrbit	LCDCCR,1
LCDCC0		avrbit	LCDCCR,0

LCDDR00		sfr	0xec		; LCD Data Register 0
SEG000		avrbit	LCDDR00,0
SEG001		avrbit	LCDDR00,1
SEG002		avrbit	LCDDR00,2
SEG003		avrbit	LCDDR00,3
SEG004		avrbit	LCDDR00,4
SEG005		avrbit	LCDDR00,5
SEG006		avrbit	LCDDR00,6
SEG007		avrbit	LCDDR00,7
LCDDR01		sfr	0xed		; LCD Data Register 1
SEG008		avrbit	LCDDR01,0
SEG009		avrbit	LCDDR01,1
SEG010		avrbit	LCDDR01,2
SEG011		avrbit	LCDDR01,3
SEG012		avrbit	LCDDR01,4
SEG013		avrbit	LCDDR01,5
SEG014		avrbit	LCDDR01,6
SEG015		avrbit	LCDDR01,7
LCDDR02		sfr	0xee		; LCD Data Register 2
SEG016		avrbit	LCDDR02,0
SEG017		avrbit	LCDDR02,1
SEG018		avrbit	LCDDR02,2
SEG019		avrbit	LCDDR02,3
SEG020		avrbit	LCDDR02,4
SEG021		avrbit	LCDDR02,5
SEG022		avrbit	LCDDR02,6
SEG023		avrbit	LCDDR02,7
LCDDR03		sfr	0xef		; LCD Data Register 3
SEG024		avrbit	LCDDR03,0
		if (MOMCPUNAME="ATMEGA3290")||(MOMCPUNAME="ATMEGA6490")
SEG025		avrbit	LCDDR03,1
SEG026		avrbit	LCDDR03,2
SEG027		avrbit	LCDDR03,3
SEG028		avrbit	LCDDR03,4
SEG029		avrbit	LCDDR03,5
SEG030		avrbit	LCDDR03,6
SEG031		avrbit	LCDDR03,7
LCDDR04		sfr	0xf0		; LCD Data Register 4
SEG032		avrbit	LCDDR04,0
SEG033		avrbit	LCDDR04,1
SEG034		avrbit	LCDDR04,2
SEG035		avrbit	LCDDR04,3
SEG036		avrbit	LCDDR04,4
SEG037		avrbit	LCDDR04,5
SEG038		avrbit	LCDDR04,6
SEG039		avrbit	LCDDR04,7
		endif
LCDDR05		sfr	0xf1		; LCD Data Register 5
SEG100		avrbit	LCDDR05,0
SEG101		avrbit	LCDDR05,1
SEG102		avrbit	LCDDR05,2
SEG103		avrbit	LCDDR05,3
SEG104		avrbit	LCDDR05,4
SEG105		avrbit	LCDDR05,5
SEG106		avrbit	LCDDR05,6
SEG107		avrbit	LCDDR05,7
LCDDR06		sfr	0xf2		; LCD Data Register 6
SEG108		avrbit	LCDDR06,0
SEG109		avrbit	LCDDR06,1
SEG110		avrbit	LCDDR06,2
SEG111		avrbit	LCDDR06,3
SEG112		avrbit	LCDDR06,4
SEG113		avrbit	LCDDR06,5
SEG114		avrbit	LCDDR06,6
SEG115		avrbit	LCDDR06,7
LCDDR07		sfr	0xf3		; LCD Data Register 7
SEG116		avrbit	LCDDR07,0
SEG117		avrbit	LCDDR07,1
SEG118		avrbit	LCDDR07,2
SEG119		avrbit	LCDDR07,3
SEG120		avrbit	LCDDR07,4
SEG121		avrbit	LCDDR07,5
SEG122		avrbit	LCDDR07,6
SEG123		avrbit	LCDDR07,7
LCDDR08		sfr	0xf4		; LCD Data Register 8
SEG124		avrbit	LCDDR08,0
		if (MOMCPUNAME="ATMEGA3290")||(MOMCPUNAME="ATMEGA6490")
SEG125		avrbit	LCDDR08,1
SEG126		avrbit	LCDDR08,2
SEG127		avrbit	LCDDR08,3
SEG128		avrbit	LCDDR08,4
SEG129		avrbit	LCDDR08,5
SEG130		avrbit	LCDDR08,6
SEG131		avrbit	LCDDR08,7
LCDDR09		sfr	0xf5		; LCD Data Register 9
SEG132		avrbit	LCDDR09,0
SEG133		avrbit	LCDDR09,1
SEG134		avrbit	LCDDR09,2
SEG135		avrbit	LCDDR09,3
SEG136		avrbit	LCDDR09,4
SEG137		avrbit	LCDDR09,5
SEG138		avrbit	LCDDR09,6
SEG139		avrbit	LCDDR09,7
		endif
LCDDR10		sfr	0xf6		; LCD Data Register 10
SEG200		avrbit	LCDDR10,0
SEG201		avrbit	LCDDR10,1
SEG202		avrbit	LCDDR10,2
SEG203		avrbit	LCDDR10,3
SEG204		avrbit	LCDDR10,4
SEG205		avrbit	LCDDR10,5
SEG206		avrbit	LCDDR10,6
SEG207		avrbit	LCDDR10,7
LCDDR11		sfr	0xf7		; LCD Data Register 11
SEG208		avrbit	LCDDR11,0
SEG209		avrbit	LCDDR11,1
SEG210		avrbit	LCDDR11,2
SEG211		avrbit	LCDDR11,3
SEG212		avrbit	LCDDR11,4
SEG213		avrbit	LCDDR11,5
SEG214		avrbit	LCDDR11,6
SEG215		avrbit	LCDDR11,7
LCDDR12		sfr	0xf8		; LCD Data Register 12
SEG216		avrbit	LCDDR12,0
SEG217		avrbit	LCDDR12,1
SEG218		avrbit	LCDDR12,2
SEG219		avrbit	LCDDR12,3
SEG220		avrbit	LCDDR12,4
SEG221		avrbit	LCDDR12,5
SEG222		avrbit	LCDDR12,6
SEG223		avrbit	LCDDR12,7
LCDDR13		sfr	0xf9		; LCD Data Register 13
SEG224		avrbit	LCDDR13,0
		if (MOMCPUNAME="ATMEGA3290")||(MOMCPUNAME="ATMEGA6490")
SEG225		avrbit	LCDDR13,1
SEG226		avrbit	LCDDR13,2
SEG227		avrbit	LCDDR13,3
SEG228		avrbit	LCDDR13,4
SEG229		avrbit	LCDDR13,5
SEG230		avrbit	LCDDR13,6
SEG231		avrbit	LCDDR13,7
LCDDR14		sfr	0xfa		; LCD Data Register 14
SEG232		avrbit	LCDDR14,0
SEG233		avrbit	LCDDR14,1
SEG234		avrbit	LCDDR14,2
SEG235		avrbit	LCDDR14,3
SEG236		avrbit	LCDDR14,4
SEG237		avrbit	LCDDR14,5
SEG238		avrbit	LCDDR14,6
SEG239		avrbit	LCDDR14,7
		endif
LCDDR15		sfr	0xfb		; LCD Data Register 15
SEG300		avrbit	LCDDR15,0
SEG301		avrbit	LCDDR15,1
SEG302		avrbit	LCDDR15,2
SEG303		avrbit	LCDDR15,3
SEG304		avrbit	LCDDR15,4
SEG305		avrbit	LCDDR15,5
SEG306		avrbit	LCDDR15,6
SEG307		avrbit	LCDDR15,7
LCDDR16		sfr	0xfc		; LCD Data Register 16
SEG308		avrbit	LCDDR16,0
SEG309		avrbit	LCDDR16,1
SEG310		avrbit	LCDDR16,2
SEG311		avrbit	LCDDR16,3
SEG312		avrbit	LCDDR16,4
SEG313		avrbit	LCDDR16,5
SEG314		avrbit	LCDDR16,6
SEG315		avrbit	LCDDR16,7
LCDDR17		sfr	0xfd		; LCD Data Register 17
SEG316		avrbit	LCDDR17,0
SEG317		avrbit	LCDDR17,1
SEG318		avrbit	LCDDR17,2
SEG319		avrbit	LCDDR17,3
SEG320		avrbit	LCDDR17,4
SEG321		avrbit	LCDDR17,5
SEG322		avrbit	LCDDR17,6
SEG323		avrbit	LCDDR17,7
LCDDR18		sfr	0xfe		; LCD Data Register 18
SEG324		avrbit	LCDDR18,0
		if (MOMCPUNAME="ATMEGA3290")||(MOMCPUNAME="ATMEGA6490")
SEG325		avrbit	LCDDR18,1
SEG326		avrbit	LCDDR18,2
SEG327		avrbit	LCDDR18,3
SEG328		avrbit	LCDDR18,4
SEG329		avrbit	LCDDR18,5
SEG330		avrbit	LCDDR18,6
SEG331		avrbit	LCDDR18,7
LCDDR19		sfr	0xff		; LCD Data Register 19
SEG332		avrbit	LCDDR19,0
SEG333		avrbit	LCDDR19,1
SEG334		avrbit	LCDDR19,2
SEG335		avrbit	LCDDR19,3
SEG336		avrbit	LCDDR19,4
SEG337		avrbit	LCDDR19,5
SEG338		avrbit	LCDDR19,6
SEG339		avrbit	LCDDR19,7
		endif

		restore			; re-enable listing

		endif			; __lcdinc
