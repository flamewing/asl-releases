		ifndef	__lcdinc
__lcdinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File LCD.INC                                                 *
;*                                                                          *
;*   Contains bit & register definitions for ATmega 169/329(0)/649(0) LCD   *
;*                                                                          *
;****************************************************************************

LCDCRA		sfr	0xe4		; LCD Control and Status Register A
LCDEN		equ	7		; LCD Enable
LCDAB		equ	6		; LCD Low Power Waveform
LCDIF		equ	4		; LCD Interrupt Flag
LCDIE		equ	3		; LCD Interrupt Enable
LCDBD		equ	2		; LCD Buffer Disable
LCDCCD		equ	1		; LCD Contrast Control Disable
LCDBL		equ	0		; LCD Blanking

LCDCRB		sfr	0xe5		; LCD Control and Status Register B
LCDCS		equ	7		; LCD Clock Select
LCD2B		equ	6		; LCD 1/2 Bias Select
LCDMUX1		equ	5		; LCD Mux Select
LCDMUX0		equ	4
LCDPM3		equ	3		; LCD Port Mask
LCDPM2		equ	2
LCDPM1		equ	1
LCDPM0		equ	0

LCDFRR		sfr	0xe6		; LCD Frame Rate Register
LCDPS2		equ	6		; LCD Prescaler Select
LCDPS1		equ	5
LCDPS0		equ	4
LCDCD2		equ	2		; LCD Clock Divide
LCDCD1		equ	1
LCDCD0		equ	0

LCDCCR		sfr	0xe7		; LCD Contrast Control Register
LCDDC2		equ	7		; LDC Display Configuration
LCDDC1		equ	6
LCDDC0		equ	5
LCDNDT		equ	4		; LCD Maximum Drive Time
LCDCC3		equ	3		; LCD Contrast Control
LCDCC2		equ	2
LCDCC1		equ	1
LCDCC0		equ	0

LCDDR00		sfr	0xec		; LCD Data Register 0
SEG000		equ	0
SEG001		equ	1
SEG002		equ	2
SEG003		equ	3
SEG004		equ	4
SEG005		equ	5
SEG006		equ	6
SEG007		equ	7
LCDDR01		sfr	0xed		; LCD Data Register 1
SEG008		equ	0
SEG009		equ	1
SEG010		equ	2
SEG011		equ	3
SEG012		equ	4
SEG013		equ	5
SEG014		equ	6
SEG015		equ	7
LCDDR02		sfr	0xee		; LCD Data Register 2
SEG016		equ	0
SEG017		equ	1
SEG018		equ	2
SEG019		equ	3
SEG020		equ	4
SEG021		equ	5
SEG022		equ	6
SEG023		equ	7
LCDDR03		sfr	0xef		; LCD Data Register 3
SEG024		equ	0
		if (MOMCPUNAME="ATMEGA3290")||(MOMCPUNAME="ATMEGA6490")
SEG025		equ	1
SEG026		equ	2
SEG027		equ	3
SEG028		equ	4
SEG029		equ	5
SEG030		equ	6
SEG031		equ	7
LCDDR04		sfr	0xf0		; LCD Data Register 4
SEG032		equ	0
SEG033		equ	1
SEG034		equ	2
SEG035		equ	3
SEG036		equ	4
SEG037		equ	5
SEG038		equ	6
SEG039		equ	7
		endif
LCDDR05		sfr	0xf1		; LCD Data Register 5
SEG100		equ	0
SEG101		equ	1
SEG102		equ	2
SEG103		equ	3
SEG104		equ	4
SEG105		equ	5
SEG106		equ	6
SEG107		equ	7
LCDDR06		sfr	0xf2		; LCD Data Register 6
SEG108		equ	0
SEG109		equ	1
SEG110		equ	2
SEG111		equ	3
SEG112		equ	4
SEG113		equ	5
SEG114		equ	6
SEG115		equ	7
LCDDR07		sfr	0xf3		; LCD Data Register 7
SEG116		equ	0
SEG117		equ	1
SEG118		equ	2
SEG119		equ	3
SEG120		equ	4
SEG121		equ	5
SEG122		equ	6
SEG123		equ	7
LCDDR08		sfr	0xf4		; LCD Data Register 8
SEG124		equ	0
		if (MOMCPUNAME="ATMEGA3290")||(MOMCPUNAME="ATMEGA6490")
SEG125		equ	1
SEG126		equ	2
SEG127		equ	3
SEG128		equ	4
SEG129		equ	5
SEG130		equ	6
SEG131		equ	7
LCDDR09		sfr	0xf5		; LCD Data Register 9
SEG132		equ	0
SEG133		equ	1
SEG134		equ	2
SEG135		equ	3
SEG136		equ	4
SEG137		equ	5
SEG138		equ	6
SEG139		equ	7
		endif
LCDDR10		sfr	0xf6		; LCD Data Register 10
SEG200		equ	0
SEG201		equ	1
SEG202		equ	2
SEG203		equ	3
SEG204		equ	4
SEG205		equ	5
SEG206		equ	6
SEG207		equ	7
LCDDR11		sfr	0xf7		; LCD Data Register 11
SEG208		equ	0
SEG209		equ	1
SEG210		equ	2
SEG211		equ	3
SEG212		equ	4
SEG213		equ	5
SEG214		equ	6
SEG215		equ	7
LCDDR12		sfr	0xf8		; LCD Data Register 12
SEG216		equ	0
SEG217		equ	1
SEG218		equ	2
SEG219		equ	3
SEG220		equ	4
SEG221		equ	5
SEG222		equ	6
SEG223		equ	7
LCDDR13		sfr	0xf9		; LCD Data Register 13
SEG224		equ	0
		if (MOMCPUNAME="ATMEGA3290")||(MOMCPUNAME="ATMEGA6490")
SEG225		equ	1
SEG226		equ	2
SEG227		equ	3
SEG228		equ	4
SEG229		equ	5
SEG230		equ	6
SEG231		equ	7
LCDDR14		sfr	0xfa		; LCD Data Register 14
SEG232		equ	0
SEG233		equ	1
SEG234		equ	2
SEG235		equ	3
SEG236		equ	4
SEG237		equ	5
SEG238		equ	6
SEG239		equ	7
		endif
LCDDR15		sfr	0xfb		; LCD Data Register 15
SEG300		equ	0
SEG301		equ	1
SEG302		equ	2
SEG303		equ	3
SEG304		equ	4
SEG305		equ	5
SEG306		equ	6
SEG307		equ	7
LCDDR16		sfr	0xfc		; LCD Data Register 16
SEG308		equ	0
SEG309		equ	1
SEG310		equ	2
SEG311		equ	3
SEG312		equ	4
SEG313		equ	5
SEG314		equ	6
SEG315		equ	7
LCDDR17		sfr	0xfd		; LCD Data Register 17
SEG316		equ	0
SEG317		equ	1
SEG318		equ	2
SEG319		equ	3
SEG320		equ	4
SEG321		equ	5
SEG322		equ	6
SEG323		equ	7
LCDDR18		sfr	0xfe		; LCD Data Register 18
SEG324		equ	0
		if (MOMCPUNAME="ATMEGA3290")||(MOMCPUNAME="ATMEGA6490")
SEG325		equ	1
SEG326		equ	2
SEG327		equ	3
SEG328		equ	4
SEG329		equ	5
SEG330		equ	6
SEG331		equ	7
LCDDR19		sfr	0xff		; LCD Data Register 19
SEG332		equ	0
SEG333		equ	1
SEG334		equ	2
SEG335		equ	3
SEG336		equ	4
SEG337		equ	5
SEG338		equ	6
SEG339		equ	7
		endif

		restore			; re-enable listing

		endif			; __lcdinc
