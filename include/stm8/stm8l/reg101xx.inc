		ifndef	__stm8l101xxinc	; avoid multiple inclusion
__stm8l101xxinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG101XX.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L101x[123]                    *
;*   source: DocID15275 Rev 16                                              *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

FLASHSTART	label	$8000		; start address internal Flash

RAMSTART	label	$0000		; start address internal RAM
RAMEND		label	$05ff		; end     "        "      "

;----------------------------------------------------------------------------
; unique id

		include "stm8/stm8s/uid.inc"
		__defuid $4925

;----------------------------------------------------------------------------
; Option Bytes

OPT1		label	$4800		; Read-out protection
ROP		bfield	OPT1,0,8	;  Memory readout protection
OPT2		label	$4802		; User boot code size
UBC		bfield	OPT2,0,8	;  Size of the user boot code area
OPT3		label	$4803		; Size of the data EEPROM area
DATASIZE	bfield	OPT3,0,8
OPT4		label	$4808		; Number of stabilization clock cycles for HSE and LSE oscillators
IWDG_HW		bit	OPT4,0		;  Independent watchdog activated by SW/HW
IWDG_HALT	bit	OPT4,1		;  Independent window watchdog off on Halt/Active-halt

;----------------------------------------------------------------------------
; Vectors

RESET_vect	label	$8000		; Reset
TRAP_vect	label	$8004		; Software interrupt
FLASH_vect	label	$800c		; FLASH end of programing/write attempted to protected page interrupt
AWU_vect	label	$8018		; Auto wakeup from Halt
EXTIB_vect	label	$8020		; External interrupt port B
EXTID_vect	label	$8024		; External interrupt port D
EXTI0_vect	label	$8028		; External interrupt 0
EXTI1_vect	label	$802c		; External interrupt 1
EXTI2_vect	label	$8030		; External interrupt 2
EXTI3_vect	label	$8034		; External interrupt 3
EXTI4_vect	label	$8038		; External interrupt 4
EXTI5_vect	label	$803c		; External interrupt 5
EXTI6_vect	label	$8040		; External interrupt 6
EXTI7_vect	label	$8044		; External interrupt 7
COMP_vect	label	$8050		; Comparators
TIM2_vect	label	$8054		; TIM2 update/overflow interrupt
TIM2_CAPT_vect	label	$8058		; TIM2 capture/compare interrupt
TIM3_vect	label	$805c		; TIM3 update/overflow/underflow/trigger/break interrupt
TIM3_CAPT_vect	label	$8060		; TIM3 capture/compare interrupt
TIM4_vect	label	$806c		; TIM4 update/overflow interrupt
SPI_vect	label	$8070		; SPI TX buffer empty/RX buffer not empty/error/wakeup interrupt
USART1_TX_vect	label	$8074		; USART1 transmit data register empty/transmission complete interrupt
USART1_RX_vect	label	$8078		; USART1 received data ready/overrun error/idle line detected/parity error/global error interrupt
I2C_vect	label	$807c		; I2C interrupt

;----------------------------------------------------------------------------
; GPIO

		include	"stm8/stm8s/gpio.inc"
		__defgpio "PA",$5000
		__defgpio "PB",$5005
		__defgpio "PC",$500a
		__defgpio "PD",$500f

;----------------------------------------------------------------------------
; Flash

		include	"flash01.inc"
		__defflash $5050

;----------------------------------------------------------------------------
; Wait for Events

		include	"wfe01.inc"
		__defwfe $50a6

;----------------------------------------------------------------------------
; Interrupt Controller

		include	"itc.inc"
EXTI_CR1	label	$50a0+$00	; External interrupt control register 1
P0IS		bfield	EXTI_CR1,0,2	;  Portx bit 0 external interrupt sensitivity bits
P1IS		bfield	EXTI_CR1,2,2	;  Portx bit 1 external interrupt sensitivity bits
P2IS		bfield	EXTI_CR1,4,2	;  Portx bit 2 external interrupt sensitivity bits
P3IS		bfield	EXTI_CR1,6,2	;  Portx bit 3 external interrupt sensitivity bits
EXTI_CR2	label	$50a0+$01	; External interrupt control register 2
P4IS		bfield	EXTI_CR2,0,2	;  Portx bit 4 external interrupt sensitivity bits
P5IS		bfield	EXTI_CR2,2,2	;  Portx bit 5 external interrupt sensitivity bits
P6IS		bfield	EXTI_CR2,4,2	;  Portx bit 6 external interrupt sensitivity bits
P7IS		bfield	EXTI_CR2,6,2	;  Portx bit 7 external interrupt sensitivity bits
EXTI_CR3	label	$50a0+$02	; External interrupt control register 3
PBIS		bfield	EXTI_CR3,0,2	;  Port B external interrupt sensitivity bits
PDIS		bfield	EXTI_CR3,2,2	;  Port D external interrupt sensitivity bits
EXTI_SR1	label	$50a0+$03	; External interrupt status register 1
P7F		bit	EXTI_SR1,7	;  Port A/B/C/D/E/F bit x external interrupt flag
P6F		bit	EXTI_SR1,6
P5F		bit	EXTI_SR1,5
P4F		bit	EXTI_SR1,4
P3F		bit	EXTI_SR1,3
P2F		bit	EXTI_SR1,2
P1F		bit	EXTI_SR1,1
P0F		bit	EXTI_SR1,0
EXTI_SR2	label	$50a0+$04	; External interrupt status register 2
PDF		bit	EXTI_SR2,1
PBF		bit	EXTI_SR2,0
EXTI_CONF	label	$50a0+$05	; External interrupt port select register
PDHIS		bit	EXTI_CONF,3	;  Port D[7:4] external interrupt select
PDLIS		bit	EXTI_CONF,2	;  Port D[3:0] external interrupt select
PBHIS		bit	EXTI_CONF,1	;  Port B[7:4] external interrupt select
PBLIS		bit	EXTI_CONF,0	;  Port B[3:0] external interrupt select

		__defitc $7f70,30

;----------------------------------------------------------------------------
; Reset Controller

		include	"rst01.inc"
		__defrst01 $50b0

;----------------------------------------------------------------------------
; Clock Controller

		include	"clk01.inc"
		__defclk01 $50c0

;----------------------------------------------------------------------------
; Independent Watchdog

		include	"stm8/stm8s/iwdg.inc"
		__defiwdg $50e0

;----------------------------------------------------------------------------
; Auto Wakeup Unit

		include	"stm8/stm8s/awu.inc"
		__defawu $50f0

;----------------------------------------------------------------------------
; Beeper

		include	"stm8/stm8s/beep.inc"
		__defbeep $50f3

;----------------------------------------------------------------------------
; Serial Peripheral Interface

		include	"spi01.inc"
		__defspi01 "SPI",$5200

;----------------------------------------------------------------------------
; I2C

		include	"i2c01.inc"
		__defi2c01 "I2C",$5210,0

;----------------------------------------------------------------------------
; USART1

		include "usart01.inc"
		__defusart01 "USART1",$5230

;----------------------------------------------------------------------------
; Timer 2/3

		include	"tim235.inc"
		__deftim235 "TIM2",$5250,0
		__deftim235 "TIM3",$5280,0

;----------------------------------------------------------------------------
; Timer 4

		include	"tim4.inc"
		__deftim4 $52e0,0

;----------------------------------------------------------------------------
; Infrared

		include	"irtim.inc"
		__defirtim $52ff

;----------------------------------------------------------------------------
; Comparator

		include "comp01.inc"
		__defcomp01 $5300

;----------------------------------------------------------------------------
; CPU

		include	"stm8/cpuregs.inc"
		__defcpuregs $7f00

;----------------------------------------------------------------------------
; Single Wire Interface Module

		include	"stm8/swim.inc"
		__defswim $7f80

;----------------------------------------------------------------------------
; Debug Module

		include	"stm8/dm.inc"
		__defdm	$7f90

                restore                 ; allow again

                endif			; __stm8l101xxinc
