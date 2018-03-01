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

;----------------------------------------------------------------------------
; Bits with the name as given in the datasheets contain the plain bit
; position within the register; with the prefix _bit_..., they hold
; register and bit position

avrbit		macro	{intlabel},reg,pos
__label__	equ	pos
		ifndef	_NO_AVR_BITSYMBOLS
_bit___label__	 bit	 reg.pos
		endif
		endm

;----------------------------------------------------------------------------
; include proper CPU-specific register definitions

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

		case	"AT43USB355"
		  include "avr/regu355.inc"

		case	"AT90USB646"
		  include "avr/regu646.inc"

		case	"AT90USB647"
		  include "avr/regu647.inc"

		case	"AT90USB1286"
		  include "avr/regu1286.inc"

		case	"AT90USB1287"
		  include "avr/regu1287.inc"

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

		case	"ATMEGA8U2"
		  include "avr/regm8u2.inc"

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

		case	"ATMEGA16U2"
		  include "avr/regm16u2.inc"

		case	"ATMEGA16U4"
		  include "avr/regm16u4.inc"

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

		case	"ATMEGA32U2"
		  include "avr/regm32u2.inc"

		case	"ATMEGA32U4"
		  include "avr/regm32u4.inc"

		case	"ATMEGA32U6"
		  include "avr/regm32u6.inc"

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
		  error  "AT90USB646, AT90USB647, AT90USB1286, AT90USB1287, AT90USB355,"
		  error	 "ATTINY13(A), ATTINY26, ATTINY2313(A), ATTINY4313, ATTINY24(A), ATTINY44(A), ATTINY84(A), ATTINY25, ATTINY45, ATTINY85,"
		  error	 "ATTINY261(A), ATTINY461(A), ATTINY861(A), ATTINY48, ATTINY88, ATTINY43U, ATTINY441, ATTINY841, ATTINY828, ATTINY1634,"
		  error	 "ATTINY87, ATTINY167, ATTINY4, ATTINY5, ATTINY9, ATTINY10, ATTINY20, ATTINY40, ATTINY102, ATTINY104, ATTINY28,"
		  error	 "ATTINY11, ATTINY12, ATTINY15,"
		  error	 "ATMEGA48, ATMEGA8, ATMEGA8515, ATMEGA8535, ATMEGA88, ATMEGA8U2, ATMEGA16U2, ATMEGA16U4, ATMEGA32U2, ATMEGA32U4, ATMEGA32U6,"
		  error  "ATMEGA16, ATMEGA161, ATMEGA162, ATMEGA164, ATMEGA165, ATMEGA168, ATMEGA169, ATMEGA32, ATMEGA323, ATMEGA324, ATMEGA325, ATMEGA3250, ATMEGA328, ATMEGA329, ATMEGA3290,"
		  error  "ATMEGA406, ATMEGA64, ATMEGA640, ATMEGA644, ATMEGA644RFR2, ATMEGA645, ATMEGA6450, ATMEGA649, ATMEGA6490, ATMEGA103, ATMEGA128, ATMEGA1280, ATMEGA1281, ATMEGA1284, ATMEGA1284RFR2,"
		  fatal  "ATMEGA2560, ATMEGA2561 or ATMEGA2564RFR2 allowed!"
                endcase

                if      MOMPASS=1
                 message "Atmel-AVR-SFR-Definitionen (C) 2017 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; Helper Macros

defreg		macro	newreg,refreg,offset
		switch	symtype(refreg)
		case	7
newreg		port	refreg+offset
		case	2
newreg		sfr	refreg+offset
		endcase
		endm

		; TODO: How to rework this to AVRBIT?

__deducebit	macro	dest,src
		ifdef	src
dest		  equ	 src
		endif
		endm

;----------------------------------------------------------------------------
; Constant Memory Addresses

E2START		equ	0		; start address internal EEPROM
FLASHSTART	label	0		; start address internal Flash

;----------------------------------------------------------------------------
; Constant Vectors

RESET_vect	label   0               ; Reset Entry

;----------------------------------------------------------------------------
; CPU Core

SREG		port	0x3f		; Statusregister:
C		avrbit	SREG,0		;  Carry
Z		avrbit	SREG,1		;  Ergebnis Null
N		avrbit	SREG,2		;  Ergebnis negativ
V		avrbit	SREG,3		;  Zweierkomplement-Ueberlauf
S		avrbit	SREG,4		;  Vorzeichen
H		avrbit	SREG,5		;  Halfcarry
T		avrbit	SREG,6		;  Bitspeicher
I		avrbit	SREG,7		;  globale Interruptsperre

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
RAMPZ0		 avrbit	 RAMPZ,0
		endif
		if FLASHEND>=131072
EIND		 port	 0x3c
EIND0		 equ	 0
RAMPZ1		 avrbit	 RAMPZ,1
		endif

;----------------------------------------------------------------------------
; Deduce remaining GPIO registers

		ifndef	__PORTPREFIX
__PORTPREFIX	 equ	"P"
		endif

		ifdef	PINA
		 ifndef	 __PORTA_BITS
__PORTA_BITS	  equ	  0xff
		 endif
		 if __PORTA_BITS & 1
PINA0		  avrbit PINA,0
		 endif
		 if __PORTA_BITS & 2
PINA1		  avrbit PINA,1
		 endif
		 if __PORTA_BITS & 4
PINA2		  avrbit PINA,2
		 endif
		 if __PORTA_BITS & 8
PINA3		  avrbit PINA,3
		 endif
		 if __PORTA_BITS & 16
PINA4		  avrbit PINA,4
		 endif
		 if __PORTA_BITS & 32
PINA5		  avrbit PINA,5
		 endif
		 if __PORTA_BITS & 64
PINA6		  avrbit PINA,6
		 endif
		 if __PORTA_BITS & 128
PINA7		  avrbit PINA,7
		 endif
		 ifndef	 PACR
		  defreg	 DDRA,PINA,1
		  if __PORTA_BITS & 1
DDA0		   avrbit DDRA,0
		  endif
		  if __PORTA_BITS & 2
DDA1		   avrbit DDRA,1
		  endif
		  if __PORTA_BITS & 4
DDA2		   avrbit DDRA,2
		  endif
		  if __PORTA_BITS & 8
DDA3		   avrbit DDRA,3
		  endif
		  if __PORTA_BITS & 16
DDA4		   avrbit DDRA,4
		  endif
		  if __PORTA_BITS & 32
DDA5		   avrbit DDRA,5
		  endif
		  if __PORTA_BITS & 64
DDA6		   avrbit DDRA,6
		  endif
		  if __PORTA_BITS & 128
DDA7		   avrbit DDRA,7
		  endif
		 endif
		 defreg		 PORTA,PINA,2
		 if __PORTA_BITS & 1
{__PORTPREFIX}A0  avrbit PORTA,0
		 endif
		 if __PORTA_BITS & 2
{__PORTPREFIX}A1  avrbit PORTA,1
		 endif
		 if __PORTA_BITS & 4
{__PORTPREFIX}A2  avrbit PORTA,2
		 endif
		 if __PORTA_BITS & 8
{__PORTPREFIX}A3  avrbit PORTA,3
		 endif
		 if __PORTA_BITS & 16
{__PORTPREFIX}A4  avrbit PORTA,4
		 endif
		 if __PORTA_BITS & 32
{__PORTPREFIX}A5  avrbit PORTA,5
		endif
		 if __PORTA_BITS & 64
{__PORTPREFIX}A6  avrbit PORTA,6
		 endif
		 if __PORTA_BITS & 128
{__PORTPREFIX}A7  avrbit PORTA,7
		 endif
		endif

		ifdef	PINB
		 ifndef	 __PORTB_BITS
__PORTB_BITS	  equ	  0xff
		 endif
		 if __PORTB_BITS & 1
PINB0		  avrbit PINB,0
		 endif
		 if __PORTB_BITS & 2
PINB1		  avrbit PINB,1
		 endif
		 if __PORTB_BITS & 4
PINB2		  avrbit PINB,2
		 endif
		 if __PORTB_BITS & 8
PINB3		  avrbit PINB,3
		 endif
		 if __PORTB_BITS & 16
PINB4		  avrbit PINB,4
		 endif
		 if __PORTB_BITS & 32
PINB5		  avrbit PINB,5
		 endif
		 if __PORTB_BITS & 64
PINB6		  avrbit PINB,6
		 endif
		 if __PORTB_BITS & 128
PINB7		  avrbit PINB,7
		 endif
		 ifndef   PINB_inponly
                  defreg	 DDRB,PINB,1
		  defreg	 PORTB,PINB,2
		  if __PORTB_BITS & 1
DDB0		   avrbit DDRB,0
{__PORTPREFIX}B0   avrbit PORTB,0
		  endif
		  if __PORTB_BITS & 2
DDB1		   avrbit DDRB,1
{__PORTPREFIX}B1   avrbit PORTB,1
		  endif
		  if __PORTB_BITS & 4
DDB2		   avrbit DDRB,2
{__PORTPREFIX}B2   avrbit PORTB,2
		  endif
		  if __PORTB_BITS & 8
DDB3		   avrbit DDRB,3
{__PORTPREFIX}B3   avrbit PORTB,3
		  endif
		  if __PORTB_BITS & 16
DDB4		   avrbit DDRB,4
{__PORTPREFIX}B4   avrbit PORTB,4
		  endif
		  if __PORTB_BITS & 32
DDB5		   avrbit DDRB,5
{__PORTPREFIX}B5   avrbit PORTB,5
		  endif
		  if __PORTB_BITS & 64
DDB6		   avrbit DDRB,6
{__PORTPREFIX}B6   avrbit PORTB,6
		  endif
		  if __PORTB_BITS & 128
DDB7		   avrbit DDRB,7
{__PORTPREFIX}B7   avrbit PORTB,7
		  endif
		 endif
		endif

		ifdef	PINC
		 ifndef	 __PORTC_BITS
__PORTC_BITS	  equ	  0xff
		 endif
		 defreg	 DDRC,PINC,1
		 defreg	 PORTC,PINC,2
		 if	 __PORTC_BITS&1
PINC0		  avrbit PINC,0
DDC0		  avrbit DDRC,0
{__PORTPREFIX}C0  avrbit PORTC,0
		 endif
		 if	 __PORTC_BITS&2
PINC1		  avrbit PINC,1
DDC1		  avrbit DDRC,1
{__PORTPREFIX}C1  avrbit PORTC,1
		 endif
		 if	 __PORTC_BITS&4
PINC2		  avrbit PINC,2
DDC2		  avrbit DDRC,2
{__PORTPREFIX}C2  avrbit PORTC,2
		 endif
		 if	 __PORTC_BITS&8
PINC3		  avrbit PINC,3
DDC3		  avrbit DDRC,3
{__PORTPREFIX}C3  avrbit PORTC,3
		 endif
		 if	 __PORTC_BITS&16
PINC4		  avrbit PINC,4
DDC4		  avrbit DDRC,4
{__PORTPREFIX}C4  avrbit PORTC,4
		 endif
		 if	 __PORTC_BITS&32
PINC5		  avrbit PINC,5
DDC5		  avrbit DDRC,5
{__PORTPREFIX}C5  avrbit PORTC,5
		 endif
		 if	 __PORTC_BITS&64
PINC6		  avrbit PINC,6
DDC6		  avrbit DDRC,6
{__PORTPREFIX}C6  avrbit PORTC,6
		 endif
		 if	 __PORTC_BITS&128
PINC7		  avrbit PINC,7
DDC7		  avrbit DDRC,7
{__PORTPREFIX}C7  avrbit PORTC,7
		 endif
		endif

		ifdef	PIND
		 ifndef	 __PORTD_BITS
__PORTD_BITS	  equ	  0xff
		 endif
		 defreg	 DDRD,PIND,1
		 defreg	 PORTD,PIND,2
		 if __PORTD_BITS & 1
PIND0		  avrbit PIND,0
DDD0		  avrbit DDRD,0
{__PORTPREFIX}D0  avrbit PORTD,0
		 endif
		 if __PORTD_BITS & 3
PIND1		  avrbit PIND,1
DDD1		  avrbit DDRD,1
{__PORTPREFIX}D1  avrbit PORTD,1
		 endif
		 if __PORTD_BITS & 4
PIND2		  avrbit PIND,2
DDD2		  avrbit DDRD,2
{__PORTPREFIX}D2  avrbit PORTD,2
		 endif
		 if __PORTD_BITS & 8
PIND3		  avrbit PIND,3
DDD3		  avrbit DDRD,3
{__PORTPREFIX}D3  avrbit PORTD,3
		 endif
		 if __PORTD_BITS & 16
PIND4		  avrbit PIND,4
DDD4		  avrbit DDRD,4
{__PORTPREFIX}D4  avrbit PORTD,4
		 endif
		 if __PORTD_BITS & 32
PIND5		  avrbit PIND,5
DDD5		  avrbit DDRD,5
{__PORTPREFIX}D5  avrbit PORTD,5
		 endif
		 if __PORTD_BITS & 64
PIND6		  avrbit PIND,6
DDD6		  avrbit DDRD,6
{__PORTPREFIX}D6  avrbit PORTD,6
		 endif
		 if __PORTD_BITS & 128
PIND7		  avrbit PIND,7
DDD7		  avrbit DDRD,7
{__PORTPREFIX}D7  avrbit PORTD,7
		 endif
		endif

		ifdef	PINE
		 ifndef	 __PORTE_BITS
__PORTE_BITS	  equ	  0xff
		 endif
		 defreg	 DDRE,PINE,1
		 defreg	 PORTE,PINE,2
		 if	 __PORTE_BITS&1
PINE0		  avrbit PINE,0
DDE0		  avrbit DDRE,0
{__PORTPREFIX}E0  avrbit PORTE,0
		 endif
		 if	 __PORTE_BITS&2
PINE1		  avrbit PINE,1
DDE1		  avrbit DDRE,1
{__PORTPREFIX}E1  avrbit PORTE,1
		 endif
		 if	 __PORTE_BITS&4
PINE2		  avrbit PINE,2
DDE2		  avrbit DDRE,2
{__PORTPREFIX}E2  avrbit PORTE,2
		 endif
		 if	 __PORTE_BITS&8
PINE3		  avrbit PINE,3
DDE3		  avrbit DDRE,3
{__PORTPREFIX}E3  avrbit PORTE,3
		 endif
		 if	 __PORTE_BITS&16
PINE4		  avrbit PINE,4
DDE4		  avrbit DDRE,4
{__PORTPREFIX}E4  avrbit PORTE,4
		 endif
		 if	 __PORTE_BITS&32
PINE5		  avrbit PINE,5
DDE5		  avrbit DDRE,5
{__PORTPREFIX}E5  avrbit PORTE,5
		 endif
		 if	 __PORTE_BITS&64
PINE6		  avrbit PINE,6
DDE6		  avrbit DDRE,6
{__PORTPREFIX}E6  avrbit PORTE,6
		 endif
		 if	 __PORTE_BITS&128
PINF7		  avrbit PINE,7
DDE7		  avrbit DDRE,7
{__PORTPREFIX}E7  avrbit PORTE,7
		 endif
		endif

		ifdef	PINF
		 ifndef	 __PORTF_BITS
__PORTF_BITS	  equ	  0xff
		 endif
		 ifndef   PINF_inponly
		  ifndef DDRF
		   defreg   DDRF,PINF,1
                  endif
                  ifndef PORTF
		   defreg   PORTF,PINF,2
                  endif
		  if	 __PORTF_BITS&1
DDF0		   avrbit DDRF,0
{__PORTPREFIX}F0   avrbit PORTF,0
		  endif
		  if	 __PORTF_BITS&2
DDF1		   avrbit DDRF,1
{__PORTPREFIX}F1   avrbit PORTF,1
		  endif
		  if	 __PORTF_BITS&4
DDF2		   avrbit DDRF,2
{__PORTPREFIX}F2   avrbit PORTF,2
		  endif
		  if	 __PORTF_BITS&8
DDF3		   avrbit DDRF,3
{__PORTPREFIX}F3   avrbit PORTF,3
		  endif
		  if	 __PORTF_BITS&16
DDF4		   avrbit DDRF,4
{__PORTPREFIX}F4   avrbit PORTF,4
		  endif
		  if	 __PORTF_BITS&32
DDF5		   avrbit DDRF,5
{__PORTPREFIX}F5   avrbit PORTF,5
		  endif
		  if	 __PORTF_BITS&64
DDF6		   avrbit DDRF,6
{__PORTPREFIX}F6   avrbit PORTF,6
		  endif
		  if	 __PORTF_BITS&128
DDF7		   avrbit DDRF,7
{__PORTPREFIX}F7   avrbit PORTF,7
		  endif
PINF_inponly	  equ	   0
		 endif
		endif

		ifdef	PING
		 ifndef	 __PORTG_BITS
__PORTG_BITS	  equ	  0xff
		 endif
		 defreg	 DDRG,PING,1
DDG0		 avrbit	 DDRG,0
DDG1		 avrbit	 DDRG,1
DDG2		 avrbit	 DDRG,2
DDG3		 avrbit	 DDRG,3
DDG4		 avrbit	 DDRG,4
DDG5		 avrbit	 DDRG,5
DDG6		 avrbit	 DDRG,6
DDG7		 avrbit	 DDRG,7
		 defreg	 PORTG,PING,2
{__PORTPREFIX}G0 avrbit	 PORTG,0
{__PORTPREFIX}G1 avrbit	 PORTG,1
{__PORTPREFIX}G2 avrbit	 PORTG,2
{__PORTPREFIX}G3 avrbit	 PORTG,3
{__PORTPREFIX}G4 avrbit	 PORTG,4
{__PORTPREFIX}G5 avrbit	 PORTG,5
{__PORTPREFIX}G6 avrbit	 PORTG,6
{__PORTPREFIX}G7 avrbit	 PORTG,7
		endif

		ifdef	PINH
		 ifndef	 __PORTH_BITS
__PORTH_BITS	  equ	  0xff
		 endif
		 defreg	 DDRH,PINH,1
DDH0		 avrbit	 DDRH,0
DDH1		 avrbit	 DDRH,1
DDH2		 avrbit	 DDRH,2
DDH3		 avrbit	 DDRH,3
DDH4		 avrbit	 DDRH,4
DDH5		 avrbit	 DDRH,5
DDH6		 avrbit	 DDRH,6
DDH7		 avrbit	 DDRH,7
		 defreg	 PORTH,PINH,2
{__PORTPREFIX}H0 avrbit	 PORTH,0
{__PORTPREFIX}H1 avrbit	 PORTH,1
{__PORTPREFIX}H2 avrbit	 PORTH,2
{__PORTPREFIX}H3 avrbit	 PORTH,3
{__PORTPREFIX}H4 avrbit	 PORTH,4
{__PORTPREFIX}H5 avrbit	 PORTH,5
{__PORTPREFIX}H6 avrbit	 PORTH,6
{__PORTPREFIX}H7 avrbit	 PORTH,7
		endif

		ifdef	PINJ
		 ifndef	 __PORTJ_BITS
__PORTJ_BITS	  equ	  0xff
		 endif
		 defreg	 DDRJ,PINJ,1
DDJ0		 avrbit	 DDRJ,0
DDJ1		 avrbit	 DDRJ,1
DDJ2		 avrbit	 DDRJ,2
DDJ3		 avrbit	 DDRJ,3
DDJ4		 avrbit	 DDRJ,4
DDJ5		 avrbit	 DDRJ,5
DDJ6		 avrbit	 DDRJ,6
DDJ7		 avrbit	 DDRJ,7
		 defreg	 PORTJ,PINJ,2
{__PORTPREFIX}J0 avrbit	 PORTJ,0
{__PORTPREFIX}J1 avrbit	 PORTJ,1
{__PORTPREFIX}J2 avrbit	 PORTJ,2
{__PORTPREFIX}J3 avrbit	 PORTJ,3
{__PORTPREFIX}J4 avrbit	 PORTJ,4
{__PORTPREFIX}J5 avrbit	 PORTJ,5
{__PORTPREFIX}J6 avrbit	 PORTJ,6
{__PORTPREFIX}J7 avrbit	 PORTJ,7
		endif

		ifdef	PINK
		 ifndef	 __PORTK_BITS
__PORTK_BITS	  equ	  0xff
		 endif
		 defreg	 DDRK,PINK,1
DDK0		 avrbit	 DDRK,0
DDK1		 avrbit	 DDRK,1
DDK2		 avrbit	 DDRK,2
DDK3		 avrbit	 DDRK,3
DDK4		 avrbit	 DDRK,4
DDK5		 avrbit	 DDRK,5
DDK6		 avrbit	 DDRK,6
DDK7		 avrbit	 DDRK,7
		 defreg	 PORTK,PINK,2
{__PORTPREFIX}K0 avrbit	 PORTK,0
{__PORTPREFIX}K1 avrbit	 PORTK,1
{__PORTPREFIX}K2 avrbit	 PORTK,2
{__PORTPREFIX}K3 avrbit	 PORTK,3
{__PORTPREFIX}K4 avrbit	 PORTK,4
{__PORTPREFIX}K5 avrbit	 PORTK,5
{__PORTPREFIX}K6 avrbit	 PORTK,6
{__PORTPREFIX}K7 avrbit	 PORTK,7
		endif

		ifdef	PINL
		 defreg	 DDRL,PINL,1
DDL0		 avrbit	 DDRL,0
DDL1		 avrbit	 DDRL,1
DDL2		 avrbit	 DDRL,2
DDL3		 avrbit	 DDRL,3
DDL4		 avrbit	 DDRL,4
DDL5		 avrbit	 DDRL,5
DDL6		 avrbit	 DDRL,6
DDL7		 avrbit	 DDRL,7
		 defreg	 PORTL,PINL,2
{__PORTPREFIX}L0 avrbit	 PORTL,0
{__PORTPREFIX}L1 avrbit	 PORTL,1
{__PORTPREFIX}L2 avrbit	 PORTL,2
{__PORTPREFIX}L3 avrbit	 PORTL,3
{__PORTPREFIX}L4 avrbit	 PORTL,4
{__PORTPREFIX}L5 avrbit	 PORTL,5
{__PORTPREFIX}L6 avrbit	 PORTL,6
{__PORTPREFIX}L7 avrbit	 PORTL,7
		endif

		ifdef	 PCMSK0
		ifndef	 PCINT0
PCINT0		 avrbit	  PCMSK0,0
PCINT1		 avrbit	  PCMSK0,1
PCINT2		 avrbit	  PCMSK0,2
PCINT3		 avrbit	  PCMSK0,3
PCINT4		 avrbit	  PCMSK0,4
PCINT5		 avrbit	  PCMSK0,5
PCINT6		 avrbit	  PCMSK0,6
PCINT7		 avrbit	  PCMSK0,7
		endif
		endif
		ifdef	 PCMSK1
		ifndef   PCINT8
PCINT8		 avrbit	  PCMSK1,0
PCINT9		 avrbit	  PCMSK1,1
PCINT10		 avrbit	  PCMSK1,2
PCINT11		 avrbit	  PCMSK1,3
PCINT12		 avrbit	  PCMSK1,4
PCINT13		 avrbit	  PCMSK1,5
PCINT14		 avrbit	  PCMSK1,6
PCINT15		 avrbit	  PCMSK1,7
		endif
		endif
		ifdef	 PCMSK2
		ifndef	 PCINT16
PCINT16		 avrbit	  PCMSK2,0
PCINT17		 avrbit	  PCMSK2,1
PCINT18		 avrbit	  PCMSK2,2
PCINT19		 avrbit	  PCMSK2,3
PCINT20		 avrbit	  PCMSK2,4
PCINT21		 avrbit	  PCMSK2,5
PCINT22		 avrbit	  PCMSK2,6
PCINT23		 avrbit	  PCMSK2,7
		endif
		endif
		ifdef	PCMSK3
		ifndef	PCINT16
PCINT24		 avrbit	  PCMSK3,0
PCINT25		 avrbit	  PCMSK3,1
PCINT26		 avrbit	  PCMSK3,2
PCINT27		 avrbit	  PCMSK3,3
PCINT28		 avrbit	  PCMSK3,4
PCINT29		 avrbit	  PCMSK3,5
PCINT30		 avrbit	  PCMSK3,6
PCINT31		 avrbit	  PCMSK3,7
		endif
		endif

		ifdef PCICR
		 ifdef PCMSK0
PCIE0		  avrbit	PCICR,0
		 endif
		 ifdef PCMSK1
PCIE1		  avrbit	PCICR,1
		 endif
		 ifdef PCMSK2
PCIE2		  avrbit	PCICR,2
		 endif
		 ifdef PCMSK3
PCIE3		  avrbit	PCICR,3
		 endif
		endif

		ifdef PCIFR
		 ifdef PCMSK0
PCIF0		  avrbit	PCIFR,0
		 endif
		 ifdef PCMSK1
PCIF1		  avrbit	PCIFR,1
		 endif
		 ifdef PCMSK2
PCIF2		  avrbit	PCIFR,2
		 endif
		 ifdef PCMSK3
PCIF3		  avrbit	PCIFR,3
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
