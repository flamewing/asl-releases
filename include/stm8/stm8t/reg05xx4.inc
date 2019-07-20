		ifndef	__reg05xx4inc	; avoid multiple inclusion
__reg05xx4inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG05XX4.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8TL52x4, STM8TL53x4            *
;*   source: DocID022344 Rev 7                                              *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

FLASHSTART	label	$8000		; start address internal Flash

RAMSTART	label	$0000		; start address internal RAM
RAMEND		label	$0fff		; end     "        "      "

;----------------------------------------------------------------------------
; Unique Id

		include "stm8/stm8s/uid.inc"
		__defuid $4925

;----------------------------------------------------------------------------
; Option Bytes

OPT0		label	$4800		; Read-out protection
ROP		bfield	OPT0,0,8
OPT1		label	$4802		; User boot code
UBC		bfield	OPT1,0,8
OPT2		label	$4803		; Size of the data EEPROM area
DATASIZE	bfield	OPT2,0,8
OPT3		label	$4807		; Size of the proprietary code area
PCODESIZE	bfield	OPT3,0,8
OPT4		label	$4808
IWDG_HW		bit	OPT4,0		; Independent watchdog activated by hardware if set
IWDG_HALT	bit	OPT4,1		; Independent window watchdog stopped on Halt/Active-halt if set
WWDG_HW		bit	OPT4,2		; Window watchdog activated by hardware if set
WWDG_HALT	bit	OPT4,3		; Window watchdog continues running in Halt/Active-halt mode if set

;----------------------------------------------------------------------------
; Vectors

RESET_vect	label	$8000		; Reset
TRAP_vect	label	$8004		; Software interrupt
FLASH_vect	label	$800c		; FLASH end of programing/write attempted to protected page interrupt
PXS_vect	label	$8010		; End of conversion/First conversion completed
AWU_vect	label	$8018		; Auto wake up from halt
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
TIM2_vect	label	$8054		; TIM2 update/overflow/trigger/break
TIM2_CAPT_vect	label	$8058		; TIM2 capture/compare
TIM3_vect	label	$805c		; TIM3 update/overflow/trigger/break
TIM3_CAPT_vect	label	$8060		; TIM3 capture/compare
TIM4_vect	label	$806c		; TIM4 update/overflow
SPI_vect	label	$8070		; End of transfer
USART_TX_vect	label	$8074		; Tx complete
USART_RX_vect	label	$8078		; Receive register DATA FULL
I2C_vect	label	$807c		; I2C interrupt

;----------------------------------------------------------------------------
; GPIO

		include	"stm8/stm8s/gpio.inc"
		__defgpio "PA",$5000
		__defgpio "PB",$5005
		__defgpio "PD",$500f

;----------------------------------------------------------------------------
; Flash

		include	"flash.inc"
		__defflash $5050

;----------------------------------------------------------------------------
; System Configuration

		include	"syscfg.inc"
		__defsyscfg $509e,0

;----------------------------------------------------------------------------
; Interrupt Controller

		include	"stm8/stm8s/itc.inc"
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
; Wait for Events

		include	"wfe.inc"
		__defwfe $50a6

;----------------------------------------------------------------------------
; Reset Controller

		include	"rst.inc"
		__defrst $50b0

;----------------------------------------------------------------------------
; Clock Controller

		include	"clk.inc"
		__defclk $50c0

;----------------------------------------------------------------------------
; Window Watchdog

		include	"stm8/stm8s/wwdg.inc"
		__defwwdg $50d3

;----------------------------------------------------------------------------
; Independent Watchdog

		include	"stm8/stm8s/iwdg.inc"
		__defiwdg $50e0

;----------------------------------------------------------------------------
; AWU

		include	"stm8/stm8s/awu.inc"
		__defawu $50f0

;----------------------------------------------------------------------------
; Beeper

		include	"stm8/stm8s/beep.inc"
		__defbeep $50f3

;----------------------------------------------------------------------------
; Serial Peripheral Interface

		include	"stm8/stm8l/spi01.inc"
		__defspi01 "SPI",$5200

;----------------------------------------------------------------------------
; I2C

		include	"stm8/stm8l/i2c01.inc"
		__defi2c01 "I2C",$5210,1

;----------------------------------------------------------------------------
; USART

		include "stm8/stm8l/usart01.inc"
		__defusart01 "USART",$5230

;----------------------------------------------------------------------------
; Timer 2/3

		include	"stm8/stm8l/tim235.inc"
		__deftim235 "TIM2",$5250,0
		__deftim235 "TIM3",$5280,0

;----------------------------------------------------------------------------
; Timer 4

		include	"stm8/stm8l/tim4.inc"
		__deftim4 $52e0,0

;----------------------------------------------------------------------------
; ProxSense

		include	"pxs.inc"
		__defpxs $5300

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

                endif			; __reg05xx4inc
