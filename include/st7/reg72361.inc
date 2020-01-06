		ifndef	__reg72361inc	; avoid multiple inclusion
__reg72361inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG72361.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72[AR/J/K]361                   *
;*                                                                          *
;*   Source: ST72361 Data Sheet, Rev. 3, August 2010, Doc ID 12468          *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; start address internal RAM
		switch	substr(MOMCPUNAME,9,1)
		case	"4","6"
RAMEND		label	$067f		; end     "        "      "
		case	"7","9"
RAMEND		label	$087f		; end     "        "      "
		endcase
		
;----------------------------------------------------------------------------
; Interrupt Vectors

PWM_ART_vect	label	$ffe0		; PWM ART Interrupt
LINSCI1_vect	label	$ffe2		; LINSCI1 Interrupt Vector
LINSCI2_vect	label	$ffe4		; LINSCI2 Interrupt Vector
TIMER16_vect	label	$ffe6		; 16-bit Timer Interrupt Vector
TIMER8_vect	label	$ffe8		; 8-bit Timer Interrupt Vector
SPI_vect	label	$ffea		; SPI Interrupt Vector
EI3_vect	label	$fff0		; External Interrupt Vector B7..4
EI2_vect	label	$fff2		; External Interrupt Vector B3..0
EI1_vect	label	$fff4		; External Interrupt Vector F2..0
EI0_vect	label	$fff6		; External Interrupt Vector A3..0, shared with
AWU_vect	label	$fff6		; Auto wake up from halt
MCC_RTC_vect	label	$fff8		; Main clock controller time base interrupt
TLI_vect	label	$fffa		; Top Level Interrupt
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

;----------------------------------------------------------------------------
; SPI

		include	"spi2.inc"
		__defspi $0021

;----------------------------------------------------------------------------
; Flash

FCSR		label	$0024		; Flash Control/Status Register

;----------------------------------------------------------------------------
; ITC

ISPR0		label	$0025		; Interrupt Software Priority Register 0
I0_0		bit	ISPR0,0
I1_0		bit	ISPR0,1
I0_1		bit	ISPR0,2
I1_1		bit	ISPR0,3
I0_2		bit	ISPR0,4
I1_2		bit	ISPR0,5
I0_3		bit	ISPR0,6
I1_3		bit	ISPR0,7
ISPR1		label	$0026		; Interrupt Software Priority Register 1
I0_4		bit	ISPR1,0
I1_4		bit	ISPR1,1
I0_5		bit	ISPR1,2
I1_5		bit	ISPR1,3
I0_6		bit	ISPR1,4
I1_6		bit	ISPR1,5
I0_7		bit	ISPR1,6
I1_7		bit	ISPR1,7
ISPR2		label	$0027		; Interrupt Software Priority Register 2
I0_8		bit	ISPR2,0
I1_8		bit	ISPR2,1
I0_9		bit	ISPR2,2
I1_9		bit	ISPR2,3
I0_10		bit	ISPR2,4
I1_10		bit	ISPR2,5
I0_11		bit	ISPR2,6
I1_11		bit	ISPR2,7
ISPR3		label	$0028		; Interrupt Software Priority Register 3
I0_12		bit	ISPR3,0
I1_12		bit	ISPR3,1
I0_13		bit	ISPR3,2
I1_13		bit	ISPR3,3
EICR0		label	$0029		; External Interrupt Control Register 0
IS3		bfield	EICR0,6,2	;  ei3 sensitivity
IS2		bfield	EICR0,4,2	;  ei2 sensitivity
IS1		bfield	EICR0,2,2	;  ei1 sensitivity
IS0		bfield	EICR0,0,2	;  ei0 sensitivity
EICR1		label	$002a		; External Interrupt Control Register 1
TLIS		bit	EICR1,1		;  Top Level Interrupt sensitivity
TLIE		bit	EICR1,0		;  Top Level Interrupt enable

;----------------------------------------------------------------------------
; PWM ART

		include	"pwm_art.inc"
		__defpwmart $0031

;----------------------------------------------------------------------------
; 8-bit Timer

T8CR2		label	$003c		; Timer Control Register 2
T8ICIE		bit	T8CR2,7		;  Input Capture Interrupt Enable
T8OCIE		bit	T8CR2,6		;  Output Compare Interrupt Enable
T8TOIE		bit	T8CR2,5		;  Timer Overflow Interrupt Enable
T8FOLV2		bit	T8CR2,4		;  Forced Output Compare 2
T8FOLV1		bit	T8CR2,3		;  Forced Output Compare 1
T8OLVL2		bit	T8CR2,2		;  Output Level 2
T8IEDG1		bit	T8CR2,1		;  Input Edge 1
T8OLVL1		bit	T8CR2,0		;  Output Level 1
T8CR1		label	$003d		; Timer Control Register 1
T8OC1E		bit	T8CR1,7		;  Output Compare 1 Pin Enable
T8OC2E		bit	T8CR1,6		;  Output Compare 2 Pin Enable
T8OPM		bit	T8CR1,5		;  One Pulse Mode
T8PWM		bit	T8CR1,4		;  Pulse Width Modulation
T8CC		bfield	T8CR1,2,2	;  Clock Control
T8IEDG2		bit	T8CR1,1		;  Input Edge 2
T8CSR		label	$003e		; Timer Control/Status Register
T8ICF1		bit	T8CSR,7		;  Input Capture Flag 1
T8OCF1		bit	T8CSR,6		;  Output Compare Flag 1
T8TOF		bit	T8CSR,5		;  Timer Overflow Flag
T8ICF2		bit	T8CSR,4		;  Input Capture Flag 2
T8OCF2		bit	T8CSR,3		;  Output Compare Flag 2
T8TIMD		bit	T8CSR,2		;  Timer disable
T8IC1R		label	$003f		; Timer Input Capture 1 Register
T8OC1R		label	$0040		; Timer Output Compare 1 Register
T8CTR		label	$0041		; Timer Counter Register
T8ACTR		label	$0042		; Timer Alternate Counter Register
T8IC2R		label	$0043		; Timer Input Capture 2 Register
T8OC2R		label	$0044		; Timer Output Compare 2 Register

;----------------------------------------------------------------------------
; 16-bit Timer

		include	"timer.inc"
		__deftimer "T16",$0050
T16CSR		label	T16SR		; Control/Status Register
T16TIMD		bit	T16CSR,2	;  Timer Disable

;----------------------------------------------------------------------------
; LINSCI

__deflinsci	macro	Name,Base
__NS		set	"\{NAME}"
SCI{__NS}SR	label	$0050		; SCI Status Register
SCI{__NS}TDRE	bit	SCI{__NS}SR,7	;  Transmit data register empty
SCI{__NS}TC	bit	SCI{__NS}SR,6	;  Transmission complete
SCI{__NS}RDRF	bit	SCI{__NS}SR,5	;  Received data ready flag
SCI{__NS}IDLE	bit	SCI{__NS}SR,4	;  Idle line detect
SCI{__NS}OR	bit	SCI{__NS}SR,3	;  Overrun error
SCI{__NS}NF	bit	SCI{__NS}SR,2	;  Noise flag
SCI{__NS}FE	bit	SCI{__NS}SR,1	;  Framing error
SCI{__NS}PE	bit	SCI{__NS}SR,0	;  Parity Error
SCI{__NS}DR	label	Base+$01	; SCI Data Register
SCI{__NS}BRR	label	Base+$02	; SCI Baud Rate Register
SCI{__NS}SCP	bfield	SCI{__NS}BRR,6,2	;  First SCI Prescaler [1:0]
SCI{__NS}SCT	bfield	SCI{__NS}BRR,3,3	;  SCI Transmitter rate divisor [2:0]
SCI{__NS}SCR	bfield	SCI{__NS}BRR,0,3	;  SCI Receiver rate divisor [2:0]
SCI{__NS}CR1	label	Base+$03	; SCI Control Register 1
SCI{__NS}R8	bit	SCI{__NS}CR1,7	;  Receive data bit 8
SCI{__NS}T8	bit	SCI{__NS}CR1,6	;  Transmit data bit 8
SCI{__NS}SCID	bit	SCI{__NS}CR1,5	;  SCI Disable
SCI{__NS}M	bit	SCI{__NS}CR1,4	;  Word length
SCI{__NS}WAKE	bit	SCI{__NS}CR1,3	;  Wake-Up method
SCI{__NS}PCE	bit	SCI{__NS}CR1,2	;  Parity control enable
SCI{__NS}PS	bit	SCI{__NS}CR1,1	;  Parity selection
SCI{__NS}PIE	bit	SCI{__NS}CR1,0	;  Parity interrupt enable
SCI{__NS}CR2	label	Base+$04	; SCI Control Register 2
SCI{__NS}TIE	bit	SCI{__NS}CR2,7	;  Transmitter interrupt enable
SCI{__NS}TCIE	bit	SCI{__NS}CR2,6	;  Transmission complete interrupt enable
SCI{__NS}RIE	bit	SCI{__NS}CR2,5	;  Receiver interrupt enable
SCI{__NS}ILIE	bit	SCI{__NS}CR2,4	;  Idle line interrupt enable
SCI{__NS}TE	bit	SCI{__NS}CR2,3	;  Transmitter enable
SCI{__NS}RE	bit	SCI{__NS}CR2,2	;  Receiver enable
SCI{__NS}RWU	bit	SCI{__NS}CR2,1	;  Receiver wake-up
SCI{__NS}SBK	bit	SCI{__NS}CR2,0	;  Send break
SCI{__NS}CR3	label	Base+$05	; SCI Control Register 3
SCI{__NS}LINE	bit	SCI{__NS}CR3,6	;  LIN Mode Enable
SCI{__NS}CLKEN	bit	SCI{__NS}CR3,3	;  Clock Enable
SCI{__NS}CPOL	bit	SCI{__NS}CR3,2	;  Clock Polarity
SCI{__NS}CPHA	bit	SCI{__NS}CR3,1	;  Clock Phase
SCI{__NS}LBCL	bit	SCI{__NS}CR3,0	;  Last bit clock pulse
SCI{__NS}ERPR	label	Base+$06	; SCI Extended Receive Prescaler Register
SCI{__NS}ETPR	label	Base+$07	; SCI Extended Transmit Prescaler Register
		endm
		__deflinsci "1",$0048
		__deflinsci "2",$0060

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc10.inc"
		__defadc10 $0045
SLOW		bit	ADCCSR,4	;  A/D Clock Selection

		restore

		endif			; __reg72361inc
