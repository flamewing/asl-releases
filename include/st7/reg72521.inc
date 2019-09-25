		ifndef	__reg72521inc	; avoid multiple inclusion
__reg72521inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG72521.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72521                           *
;*                                                                          *
;*   Source: ST72521 Data Sheet, Rev. 5, May 2005                           *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; start address internal RAM
		switch	substr(MOMCPUNAME,9,1)
		case	"6"
RAMEND		label	$047f		; end     "        "      "
		case	"9"
RAMEND		label	$087f		; end     "        "      "
		elsecase
		fatal	"cannot deduce RAM size"
		endcase
		
;----------------------------------------------------------------------------
; Interrupt Vectors

PWM_ART_vect	label	$ffe0		; PWM ART interrupt
I2C_vect	label	$ffe2		; I2C Peripheral interrupts
AVD_vect	label	$ffe4		; Auxiliary Voltage detector interrupt
SCI_vect	label	$ffe6		; SCI Interrupt Vector
TIMB_vect	label	$ffe8		; TIMER B Interrupt Vector
TIMA_vect	label	$ffea		; TIMER A Interrupt Vector
SPI_vect	label	$ffec		; SPI Interrupt Vector
CAN_vect	label	$ffee		; CAN Interrupt Vector
EI3_vect	label	$fff0		; External Interrupt Vector B7..4
EI2_vect	label	$fff2		; External Interrupt Vector B3..0
EI1_vect	label	$fff4		; External Interrupt Vector F2..0
EI0_vect	label	$fff6		; External Interrupt Vector A3..0
MCC_RTC_vect	label	$fff8		; Main clock controller time base interrupt
TLI_vect	label	$fffa		; External top level interrupt
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "PA",$0000
		__defgpio "PB",$0003
		__defgpio "PC",$0006
		__defgpio "PD",$0009
		__defgpio "PE",$000c
		__defgpio "PF",$000f
		__defgpio "PG",$0012
		__defgpio "PH",$0015

;----------------------------------------------------------------------------
; Miscellaneous

;----------------------------------------------------------------------------
; I2C

		include	"i2c.inc"
		__defi2c $0018

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"
		__defspi $0021

;----------------------------------------------------------------------------
; ITC

ISPR0		label	$0024		; Interrupt Software Priority Register 0
I0_0		bit	ISPR0,0		;  TLI
I1_0		bit	ISPR0,1
I0_1		bit	ISPR0,2		;  MCC+SI
I1_1		bit	ISPR0,3
I0_2		bit	ISPR0,4		;  EI0
I1_2		bit	ISPR0,5
I0_3		bit	ISPR0,6		;  EI1
I1_3		bit	ISPR0,7
ISPR1		label	$0025		; Interrupt Software Priority Register 1
I0_4		bit	ISPR1,0		;  EI2
I1_4		bit	ISPR1,1
I0_5		bit	ISPR1,2		;  EI3
I1_5		bit	ISPR1,3
I0_6		bit	ISPR1,4
I1_6		bit	ISPR1,5
I0_7		bit	ISPR1,6		;  SPI
I1_7		bit	ISPR1,7
ISPR2		label	$0026		; Interrupt Software Priority Register 2
I0_8		bit	ISPR2,0		;  Timer A
I1_8		bit	ISPR2,1
I0_9		bit	ISPR2,2		;  Timer B
I1_9		bit	ISPR2,3
I0_10		bit	ISPR2,4		;  SCI
I1_10		bit	ISPR2,5
I0_11		bit	ISPR2,6		;  AVD
I1_11		bit	ISPR2,7
ISPR3		label	$0027		; Interrupt Software Priority Register 3
I0_12		bit	ISPR3,0		;  I2C
I1_12		bit	ISPR3,1
I0_13		bit	ISPR3,2		;  PWMART
I1_13		bit	ISPR3,3
EICR		label	$0028		; External Interrupt Control Register
IS1		bfield	EICR,6,2	;  ei2 and ei3 sensitivity
IPB		bit	EICR,5		;  Interrupt polarity for port B
IS2		bfield	EICR,3,2	;  ei0 and ei1 sensitivity
IPA		bit	EICR,2		;  Interrupt polarity for port A
TLIS		bit	EICR,1		;  TLI sensitivity
TLIE		bit	EICR,0		;  TLI enable

;----------------------------------------------------------------------------
; Flash

FCSR		label	$0029		; Flash Control/Status Register

;----------------------------------------------------------------------------
; Watchdog

WDGCR		label	$002a		; Watchdog Control Register
WDGA		bit	WDGCR,7		; Activation bit

;----------------------------------------------------------------------------

SICSR		label	$002b		; System Integrity Control/Status Register
AVDS		bit	SICSR,7		;  Voltage Detection selection
AVDIE		bit	SICSR,6		;  Voltage Detector interrupt enable
AVDF		bit	SICSR,5		;  Voltage Detector flag
LVDRF		bit	SICSR,4		;  LVD reset flag
WDGRF		bit	SICSR,0		;  Watchdog reset flag

;----------------------------------------------------------------------------
; MCC

MCCSR		label	$002c		; Main Clock Control / Status Register
MCO		bit	MCCSR,7		;  Main clock out selection
CP		bfield	MCCSR,5,2	;  CPU clock prescaler
SMS		bit	MCCSR,4		;  Slow mode select
TB		bfield	MCCSR,2,2	;  Time base control
OIE		bit	MCCSR,1		;  Oscillator interrupt enable
OIF		bit	MCCSR,0		;  Oscillator interrupt flag
MCCBCR		label	$002d		; Main Clock Controller: Beep Control Register
BC		bfield	MCCBCR,0,2	;  Beep control

;----------------------------------------------------------------------------
; Timer A/B

		include	"timer.inc"		
		__deftimer "TA",$0030
		__deftimer "TB",$0040

;----------------------------------------------------------------------------
; Serial Communications Interface

		include	"sci2.inc"
		__defsci2 $0050,5

;----------------------------------------------------------------------------
; CAN

CANISR		label	$005a		; CAN Interrupt Status Register
RXIF3		bit	CANISR,7	;  Receive Interrupt Flag for Buffer 3
RXIF2		bit	CANISR,6	;  Receive Interrupt Flag for Buffer 2
RXIF1		bit	CANISR,5	;  Receive Interrupt Flag for Buffer 1
TXIF		bit	CANISR,4	;  Transmit Interrupt Flag
SCIF		bit	CANISR,3	;  Status Change Interrupt Flag
ORIF		bit	CANISR,2	;  Overrun Interrupt Flag
TEIF		bit	CANISR,1	;  Transmit Error Interrupt Flag
EPND		bit	CANISR,0	;  Error Interrupt Pending
CANICR		label	$005b		; CAN Interrupt Control Register
ESCI		bit	CANICR,6	;  Extended Status Change Interrupt
RXIE		bit	CANICR,5	;  Receive Interrupt Enable
TXIE		bit	CANICR,4	;  Transmit Interrupt Enable
SCIE		bit	CANICR,3	;  Status Change Interrupt Enable
ORIE		bit	CANICR,2	;  Overrun Interrupt Enable
TEIE		bit	CANICR,1	;  Transmit Error Interrupt Enable
CANCSR		label	$005c		; CAN Control / Status Register
BOFF		bit	CANCSR,6	;  Bus-Off State
EPSV		bit	CANCSR,5	;  Error Passive State
SRTE		bit	CANCSR,4	;  Simultaneous Receive/Transmit Enable
NRTX		bit	CANCSR,3	;  No Retransmission
FSYN		bit	CANCSR,2	;  Fast Synchronization
WKPS		bit	CANCSR,1	;  Wake-up Pulse
RUN		bit	CANCSR,0	;  CAN Enable
CANBRPR		label	$005d		; CAN Baud Rate Prescaler Register
RJW		bfield	CANBRPR,6,2	;  maximum number of time quanta by which a bit period may be shortened or lengthened to achieve resynchronization
BRP		bfield	CANBRPR,0,6	;  CAN system clock cycle
CANBTR		label	$005e		; CAN Bit Timing Register
BS2		bfield	CANBTR,4,3	;  length of Bit Segment 2
BS1		bfield	CANBTR,0,4	;  length of Bit Segment 1
CANPSR		label	$005f		; CAN Page Selection Register
CAN_P0		label	$0060		; Paged Registers
CAN_P1		label	$0061
CAN_P2		label	$0062
CAN_P3		label	$0063
CAN_P4		label	$0064
CAN_P5		label	$0065
CAN_P6		label	$0066
CAN_P7		label	$0067
CAN_P8		label	$0068
CAN_P9		label	$0069
CAN_P10		label	$006a
CAN_P11		label	$006b
CAN_P12		label	$006c
CAN_P13		label	$006d
CAN_P14		label	$006e
CAN_P15		label	$006f
CAN_LIDHR	label	CAN_P0		; [Page 0] Last Identifier High Register
CAN_LIDLR	label	CAN_P1		; [Page 0] Last Identifier Low Register
CAN_TECR	label	CAN_P14		; [Page 0] Transmit Error Counter Register
CAN_RECR	label	CAN_P15		; [Page 0] Receive Error Counter Register
__N		set	1
		rept	3
__NS		set	"\{__N}"
CAN_IDHR{__NS}	label	CAN_P0		; [Page 1..3] Identifier High Register
CAN_IDLR{__NS}	label	CAN_P1		; [Page 1..3] Identifier Low Register
CAN_DATA0{__NS}	label	CAN_P2		; [Page 1..3] Data Registers
CAN_DATA1{__NS}	label	CAN_P3		; [Page 1..3]
CAN_DATA2{__NS}	label	CAN_P4		; [Page 1..3]
CAN_DATA3{__NS}	label	CAN_P5		; [Page 1..3]
CAN_DATA4{__NS}	label	CAN_P6		; [Page 1..3]
CAN_DATA5{__NS}	label	CAN_P7		; [Page 1..3]
CAN_DATA6{__NS}	label	CAN_P8		; [Page 1..3]
CAN_DATA7{__NS}	label	CAN_P9		; [Page 1..3]
CAN_BCSR{__NS}	label	CAN_P15		; [Page 1..3] Buffer Control/Status Register
CAN_ACC{__NS}	bit	CAN_BCSR{__NS},3;  Acceptance Code
CAN_RDY{__NS}	bit	CAN_BCSR{__NS},2;  Message Ready
CAN_BUSY{__NS}	bit	CAN_BCSR{__NS},1;  Busy Buffer
CAN_LOCK{__NS}	bit	CAN_BCSR{__NS},0;  Lock Buffer
__N		set	__N+1
		endm
CAN_FHR0	label	CAN_P0		; [Page 4] Filter High Register 0
CAN_FLR0	label	CAN_P1		; [Page 4] Filter Low Register 0
CAN_MHR0	label	CAN_P2		; [Page 4] Mask High Register 0
CAN_MLR0	label	CAN_P3		; [Page 4] Mask Low Register 0
CAN_FHR1	label	CAN_P4		; [Page 4] Filter High Register 1
CAN_FLR1	label	CAN_P5		; [Page 4] Filter Low Register 1
CAN_MHR1	label	CAN_P6		; [Page 4] Mask High Register 1
CAN_MLR1	label	CAN_P7		; [Page 4] Mask Low Register 1

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc10.inc"
		__defadc10 $0070

;----------------------------------------------------------------------------
; PWM ART

		include	"pwm_art.inc"
		__defpwmart $0073

		restore
		endif			; __reg72521inc
