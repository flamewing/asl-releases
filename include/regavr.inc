                ifndef  __regavrinc     ; avoid multiple inclusion
__regavrinc	equ     1

                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGAVR.INC                                              *
;*                                                                          *
;*   Sinn : contains SFR and Bit Definitionen for AVR Processors            *
;*                                                                          *
;****************************************************************************

; set internal CPU code and memory addresses in one step

		switch	MOMCPUNAME
		case	"AT90S1200"
		  include "avr/reg1200.inc"

		case	"AT90S2313"
		  include "avr/reg2313.inc"

		case	"AT90S2323","AT90S2343"
		  include "avr/reg2323.inc"

		case	"AT90S2333"
		  include "avr/reg2333.inc"

		case	"AT90S4414"
		  include "avr/reg4414.inc"

		case	"AT90S4433"
		  include "avr/reg4433.inc"

		case	"AT90S4434"
		  include "avr/reg4434.inc"

		case	"AT90S8515"
		  include "avr/reg8515.inc"

		case	"AT90C8534"
		  include "avr/reg8534.inc"

		case	"AT90S8535"
		  include "avr/reg8535.inc"

		case	"ATTINY13"
		  include "avr/regtn13.inc"

		case	"ATTINY13A"
		  include "avr/regtn13a.inc"

		case	"ATTINY26"
		  include "avr/regtn26.inc"

                case	"ATTINY2313","ATTINY2313A"
		  include "avr/regt2313.inc"

		case	"ATTINY4313"
		  include "avr/regt4313.inc"

		case	"ATTINY24","ATTINY24A"
		  include "avr/regtn24.inc"

		case	"ATTINY44","ATTINY44A"
		  include "avr/regtn44.inc"

		case	"ATTINY84","ATTINY84A"
		  include "avr/regtn84.inc"

		case	"ATTINY25"
		  include "avr/regtn25.inc"

		case	"ATTINY45"
		  include "avr/regtn45.inc"

		case	"ATTINY85"
		  include "avr/regtn85.inc"

		case	"ATTINY261","ATTINY261A"
		  include "avr/regtn261.inc"

		case	"ATTINY461","ATTINY461A"
		  include "avr/regtn461.inc"

		case	"ATTINY861","ATTINY861A"
		  include "avr/regtn861.inc"

		case	"ATTINY48"
		  include "avr/regtn48.inc"

		case	"ATTINY88"
		  include "avr/regtn88.inc"

		case	"ATTINY43U"
		  include "avr/regtn43u.inc"

		case	"ATTINY441"
		  include "avr/regtn441.inc"

		case	"ATTINY841"
		  include "avr/regtn841.inc"

		case	"ATTINY828"
		  include "avr/regtn828.inc"

		case	"ATTINY1634"
		  include "avr/regtn1634.inc"

		case	"ATTINY87"
		  include "avr/regtn87.inc"

		case	"ATTINY167"
		  include "avr/regtn167.inc"

		case	"ATTINY4"
		  include "avr/regtn4.inc"

		case	"ATTINY5"
		  include "avr/regtn5.inc"

		case	"ATTINY9"
		  include "avr/regtn9.inc"

		case	"ATTINY10"
		  include "avr/regtn10.inc"

		case	"ATTINY20"
		  include "avr/regtn20.inc"

		case	"ATTINY40"
		  include "avr/regtn40.inc"

		case	"ATTINY102"
		  include "avr/regtn102.inc"

		case	"ATTINY104"
		  include "avr/regtn104.inc"

		case	"ATTINY28"
		  include "avr/regtn28.inc"

		case	"ATTINY11"
		  include "avr/regtn11.inc"

		case	"ATTINY12"
		  include "avr/regtn12.inc"

		case	"ATTINY15"
		  include "avr/regtn15.inc"

		case	"ATMEGA48"
		  include "avr/regm48.inc"

		case	"ATMEGA8"
		  include "avr/regm8.inc"

		case	"ATMEGA8515"
		  include "avr/regm8515.inc"

		case	"ATMEGA8535"
		  include "avr/regm8535.inc"

		case	"ATMEGA88"
		  include "avr/regm88.inc"

		case	"ATMEGA16"
		  include "avr/regm16.inc"

		case	"ATMEGA161"
		  include "avr/regm161.inc"

		case	"ATMEGA162"
		  include "avr/regm162.inc"

		case	"ATMEGA163"
		  include "avr/regm163.inc"

		case	"ATMEGA164"
		  include "avr/regm164.inc"

		case	"ATMEGA165"
		  include "avr/regm165.inc"

		case	"ATMEGA168"
		  include "avr/regm168.inc"

		case	"ATMEGA169"
		  include "avr/regm169.inc"

		case	"ATMEGA32"
		  include "avr/regm32.inc"

		case	"ATMEGA323"
		  include "avr/regm323.inc"

		case	"ATMEGA324"
		  include "avr/regm324.inc"

		case	"ATMEGA325"
		  include "avr/regm325.inc"

		case	"ATMEGA3250"
		  include "avr/regm3250.inc"

		case	"ATMEGA328"
		  include "avr/regm328.inc"

		case	"ATMEGA329"
		  include "avr/regm329.inc"

		case	"ATMEGA3290"
		  include "avr/regm3290.inc"

		case	"ATMEGA406"
		  include "avr/regm406.inc"

		case	"ATMEGA64"
		  include "avr/regm64.inc"

		case	"ATMEGA640"
		  include "avr/regm640.inc"

		case	"ATMEGA644"
		  include "avr/regm644.inc"

		case	"ATMEGA644RFR2"
		  include "avr/regm644rfr2.inc"

		case	"ATMEGA645"
		  include "avr/regm645.inc"

		case	"ATMEGA6450"
		  include "avr/regm6450.inc"

		case	"ATMEGA649"
		  include "avr/regm649.inc"

		case	"ATMEGA6490"
		  include "avr/regm6490.inc"

		case	"ATMEGA103"
		  include "avr/regm103.inc"

		case	"ATMEGA128"
		  include "avr/regm128.inc"

		case	"ATMEGA1280"
		  include "avr/regm1280.inc"

		case	"ATMEGA1281"
		  include "avr/regm1281.inc"

		case	"ATMEGA1284"
		  include "avr/regm1284.inc"

		case	"ATMEGA1284RFR2"
		  include "avr/reg1284rfr2.inc"

		case	"ATMEGA2560"
		  include "avr/regm2560.inc"

		case	"ATMEGA2561"
		  include "avr/regm2561.inc"

		case	"ATMEGA2564RFR2"
		  include "avr/reg2564rfr2.inc"

		elsecase
		  error  "wrong processor type set: only AT90S1200, AT90S2313, AT90S4414, AT90S4433, AT90S4434, AT90S8515, AT90C8534, AT90S8535,"
		  error	 "ATTINY13(A), ATTINY26, ATTINY2313(A), ATTINY4313, ATTINY24(A), ATTINY44(A), ATTINY84(A), ATTINY25, ATTINY45, ATTINY85,"
		  error	 "ATTINY261(A), ATTINY461(A), ATTINY861(A), ATTINY48, ATTINY88, ATTINY43U, ATTINY441, ATTINY841, ATTINY828, ATTINY1634,"
		  error	 "ATTINY87, ATTINY167, ATTINY4, ATTINY5, ATTINY9, ATTINY10, ATTINY20, ATTINY40, ATTINY102, ATTINY104, ATTINY28,"
		  error	 "ATTINY11, ATTINY12, ATTINY15,"
		  error	 "ATMEGA48, ATMEGA8, ATMEGA8515, ATMEGA8535, ATMEGA88,"
		  error  "ATMEGA16, ATMEGA161, ATMEGA162, ATMEGA164, ATMEGA165, ATMEGA168, ATMEGA169, ATMEGA32, ATMEGA323, ATMEGA324, ATMEGA325, ATMEGA3250, ATMEGA328, ATMEGA329, ATMEGA3290,"
		  error  "ATMEGA406, ATMEGA64, ATMEGA640, ATMEGA644, ATMEGA644RFR2, ATMEGA645, ATMEGA6450, ATMEGA649, ATMEGA6490, ATMEGA103, ATMEGA128, ATMEGA1280, ATMEGA1281, ATMEGA1284, ATMEGA1284RFR2,"
		  fatal  "ATMEGA2560, ATMEGA2561 or ATMEGA2564RFR2 allowed!"
                endcase

                if      MOMPASS=1
                 message "Atmel-AVR-SFR-Definitionen (C) 2017 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; helper macros

defreg		macro	newreg,refreg,offset
		switch	symtype(refreg)
		case	7
newreg		port	refreg+offset
		case	2
newreg		sfr	refreg+offset
		endcase
		endm

__deducebit	macro	dest,src
		ifdef	src
dest		  equ	 src
		endif
		endm

;----------------------------------------------------------------------------
; constant memory addresses

E2START		equ	0		; start address internal EEPROM
FLASHSTART	label	0		; start address internal Flash

;----------------------------------------------------------------------------
; Constant Vectors

RESET_vect	label   0               ; Reset Entry

;----------------------------------------------------------------------------
; CPU Core

SREG            port    0x3f            ; Statusregister:
C               equ     0               ;  Carry
Z               equ     1               ;  Ergebnis Null
N               equ     2               ;  Ergebnis negativ
V               equ     3               ;  Zweierkomplement-Ueberlauf
S               equ     4               ;  Vorzeichen
H               equ     5               ;  Halfcarry
T               equ     6               ;  Bitspeicher
I               equ     7               ;  globale Interruptsperre

; size of stack pointer depends on size of internal data space
; (if present at all)

                if      RAMEND>=RAMSTART
SPL              port    0x3d           ; Stapelzeiger (LSB)
                 if      RAMEND>=256
SPH               port    0x3e          ;              (MSB)
                 endif
                endif

		if FLASHEND>=65536
RAMPZ		 port	 0x3b
RAMPZ0		 equ	 0
		endif
		if FLASHEND>=131072
EIND		 port	 0x3c
EIND0		 equ	 0
RAMPZ1		 equ	 1
		endif

;----------------------------------------------------------------------------
; Deduce remaining GPIO registers

		ifdef	PINA
		 ifndef	 PACR
		  defreg	 DDRA,PINA,1
DDA0		  equ	 0
DDA1		  equ	 1
DDA2		  equ	 2
DDA3		  equ	 3
DDA4		  equ	 4
DDA5		  equ	 5
DDA6		  equ	 6
DDA7		  equ	 7
		 endif
		 defreg	 PORTA,PINA,2
PA0		 equ	 0
PA1		 equ	 1
PA2		 equ	 2
PA3		 equ	 3
PA4		 equ	 4
PA5		 equ	 5
PA6		 equ	 6
PA7		 equ	 7
		endif

		ifdef	PINB
		 ifndef   PINB_inponly
                  defreg	 DDRB,PINB,1
DDB0		  equ	 0
DDB1		  equ	 1
DDB2		  equ	 2
DDB3		  equ	 3
DDB4		  equ	 4
DDB5		  equ	 5
DDB6		  equ	 6
DDB7		  equ	 7
		  defreg	 PORTB,PINB,2
PB0		  equ	 0
PB1		  equ	 1
PB2		  equ	 2
PB3		  equ	 3
PB4		  equ	 4
PB5		  equ	 5
PB6		  equ	 6
PB7		  equ	 7
		 endif
		endif

		ifdef	PINC
		 defreg	 DDRC,PINC,1
DDC0		 equ	 0
DDC1		 equ	 1
DDC2		 equ	 2
DDC3		 equ	 3
DDC4		 equ	 4
DDC5		 equ	 5
DDC6		 equ	 6
DDC7		 equ	 7
		 defreg	 PORTC,PINC,2
PC0		 equ	 0
PC1		 equ	 1
PC2		 equ	 2
PC3		 equ	 3
PC4		 equ	 4
PC5		 equ	 5
PC6		 equ	 6
PC7		 equ	 7
		endif

		ifdef	PIND
		 defreg	 DDRD,PIND,1
DDD0		 equ	 0
DDD1		 equ	 1
DDD2		 equ	 2
DDD3		 equ	 3
DDD4		 equ	 4
DDD5		 equ	 5
DDD6		 equ	 6
DDD7		 equ	 7
		 defreg	 PORTD,PIND,2
PD0		 equ	 0
PD1		 equ	 1
PD2		 equ	 2
PD3		 equ	 3
PD4		 equ	 4
PD5		 equ	 5
PD6		 equ	 6
PD7		 equ	 7
		endif

		ifdef	PINE
		 defreg	 DDRE,PINE,1
DDE0		 equ	 0
DDE1		 equ	 1
DDE2		 equ	 2
DDE3		 equ	 3
DDE4		 equ	 4
DDE5		 equ	 5
DDE6		 equ	 6
DDE7		 equ	 7
		 defreg	 PORTE,PINE,2
PE0		 equ	 0
PE1		 equ	 1
PE2		 equ	 2
PE3		 equ	 3
PE4		 equ	 4
PE5		 equ	 5
PE6		 equ	 6
PE7		 equ	 7
		endif

		ifdef	PINF
		 ifndef   PINF_inponly
		  ifndef DDRF
		   defreg   DDRF,PINF,1
                  endif
DDF0		  equ	 0
DDF1		  equ	 1
DDF2		  equ	 2
DDF3		  equ	 3
DDF4		  equ	 4
DDF5		  equ	 5
DDF6		  equ	 6
DDF7		  equ	 7
                  ifndef PORTF
		   defreg   PORTF,PINF,2
                  endif
PF0		  equ	 0
PF1		  equ	 1
PF2		  equ	 2
PF3		  equ	 3
PF4		  equ	 4
PF5		  equ	 5
PF6		  equ	 6
PF7		  equ	 7
PINF_inponly	  equ	   0
		 endif
		endif

		ifdef	PING
		 defreg	 DDRG,PING,1
DDG0		 equ	 0
DDG1		 equ	 1
DDG2		 equ	 2
DDG3		 equ	 3
DDG4		 equ	 4
DDG5		 equ	 5
DDG6		 equ	 6
DDG7		 equ	 7
		 defreg	 PORTG,PING,2
PG0		 equ	 0
PG1		 equ	 1
PG2		 equ	 2
PG3		 equ	 3
PG4		 equ	 4
PG5		 equ	 5
PG6		 equ	 6
PG7		 equ	 7
		endif

		ifdef	PINH
		 defreg	 DDRH,PINH,1
DDH0		 equ	 0
DDH1		 equ	 1
DDH2		 equ	 2
DDH3		 equ	 3
DDH4		 equ	 4
DDH5		 equ	 5
DDH6		 equ	 6
DDH7		 equ	 7
		 defreg	 PORTH,PINH,2
PH0		 equ	 0
PH1		 equ	 1
PH2		 equ	 2
PH3		 equ	 3
PH4		 equ	 4
PH5		 equ	 5
PH6		 equ	 6
PH7		 equ	 7
		endif

		ifdef	PINJ
		 defreg	 DDRJ,PINJ,1
DDJ0		 equ	 0
DDJ1		 equ	 1
DDJ2		 equ	 2
DDJ3		 equ	 3
DDJ4		 equ	 4
DDJ5		 equ	 5
DDJ6		 equ	 6
DDJ7		 equ	 7
		 defreg	 PORTJ,PINJ,2
PJ0		 equ	 0
PJ1		 equ	 1
PJ2		 equ	 2
PJ3		 equ	 3
PJ4		 equ	 4
PJ5		 equ	 5
PJ6		 equ	 6
PJ7		 equ	 7
		endif

		ifdef	PINK
		 defreg	 DDRK,PINK,1
DDK0		 equ	 0
DDK1		 equ	 1
DDK2		 equ	 2
DDK3		 equ	 3
DDK4		 equ	 4
DDK5		 equ	 5
DDK6		 equ	 6
DDK7		 equ	 7
		 defreg	 PORTK,PINK,2
PK0		 equ	 0
PK1		 equ	 1
PK2		 equ	 2
PK3		 equ	 3
PK4		 equ	 4
PK5		 equ	 5
PK6		 equ	 6
PK7		 equ	 7
		endif

		ifdef	PINL
		 defreg	 DDRL,PINL,1
DDL0		 equ	 0
DDL1		 equ	 1
DDL2		 equ	 2
DDL3		 equ	 3
DDL4		 equ	 4
DDL5		 equ	 5
DDL6		 equ	 6
DDL7		 equ	 7
		 defreg	 PORTL,PINL,2
PL0		 equ	 0
PL1		 equ	 1
PL2		 equ	 2
PL3		 equ	 3
PL4		 equ	 4
PL5		 equ	 5
PL6		 equ	 6
PL7		 equ	 7
		endif

		ifdef	 PCMSK0
		ifndef	 PCINT0
PCINT0		 equ	  0
PCINT1		 equ	  1
PCINT2		 equ	  2
PCINT3		 equ	  3
PCINT4		 equ	  4
PCINT5		 equ	  5
PCINT6		 equ	  6
PCINT7		 equ	  7
		endif
		endif
		ifdef	 PCMSK1
		ifndef   PCINT8
PCINT8		 equ	  0
PCINT9		 equ	  1
PCINT10		 equ	  2
PCINT11		 equ	  3
PCINT12		 equ	  4
PCINT13		 equ	  5
PCINT14		 equ	  6
PCINT15		 equ	  7
		endif
		endif
		ifdef	 PCMSK2
		ifndef	 PCINT16
PCINT16		 equ	  0
PCINT17		 equ	  1
PCINT18		 equ	  2
PCINT19		 equ	  3
PCINT20		 equ	  4
PCINT21		 equ	  5
PCINT22		 equ	  6
PCINT23		 equ	  7
		endif
		endif
		ifdef	PCMSK3
		ifndef	PCINT16
PCINT24		 equ	  0
PCINT25		 equ	  1
PCINT26		 equ	  2
PCINT27		 equ	  3
PCINT28		 equ	  4
PCINT29		 equ	  5
PCINT30		 equ	  6
PCINT31		 equ	  7
		endif
		endif

		ifdef PCICR
		 ifdef PCMSK0
PCIE0		  equ	0
		 endif
		 ifdef PCMSK1
PCIE1		  equ	1
		 endif
		 ifdef PCMSK2
PCIE2		  equ	2
		 endif
		 ifdef PCMSK3
PCIE3		  equ	3
		 endif
		endif

		ifdef PCIFR
		 ifdef PCMSK0
PCIF0		  equ	0
		 endif
		 ifdef PCMSK1
PCIF1		  equ	1
		 endif
		 ifdef PCMSK2
PCIF2		  equ	2
		 endif
		 ifdef PCMSK3
PCIF3		  equ	3
		 endif
		endif

;----------------------------------------------------------------------------
; Timer

		; deduce interrupt status from interrupt enable bits

		__deducebit TOV0,TOIE0
		__deducebit OCF0,OCIE0
		__deducebit OCF0A,OCIE0A
		__deducebit OCF0B,OCIE0B

		__deducebit TOV1,TOIE1
		__deducebit OCF1A,OCIE1A
		__deducebit OCF1B,OCIE1B
		__deducebit OCF1C,OCIE1C
		__deducebit ICF1,TICIE1

		__deducebit TOV2,TOIE2
		__deducebit OCF2,OCIE2
		__deducebit OCF2A,OCIE2A
		__deducebit OCF2B,OCIE2B
		__deducebit ICF2,TICIE2

		__deducebit TOV3,TOIE3
		__deducebit OCF3,OCIE3
		__deducebit OCF3A,OCIE3A
		__deducebit OCF3B,OCIE3B
		__deducebit OCF3C,OCIE3C
		__deducebit ICF3,TICIE3

		__deducebit TOV4,TOIE4
		__deducebit OCF4,OCIE4
		__deducebit OCF4A,OCIE4A
		__deducebit OCF4B,OCIE4B
		__deducebit OCF4C,OCIE4C
		__deducebit ICF4,TICIE4

		__deducebit TOV5,TOIE5
		__deducebit OCF5,OCIE5
		__deducebit OCF5A,OCIE5A
		__deducebit OCF5B,OCIE5B
		__deducebit OCF5C,OCIE5C
		__deducebit ICF5,TICIE5

;----------------------------------------------------------------------------

                restore                 ; wieder erlauben

                endif			; __regavrinc
