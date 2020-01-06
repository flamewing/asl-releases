		ifndef	__stm8af62x3inc	; avoid multiple inclusion
__stm8af62x3inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG62X3.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8AF6213/6223/6226              *
;*   source: DS9884 Rev 8                                                   *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

E2START		label	$4000		; start address internal EEPROM
E2END		label	E2START+639	; end     "        "       "

FLASHSTART	label	$8000		; start address internal Flash

RAMSTART	label	$0000		; start address internal RAM
		if	SUBSTR(MOMCPUNAME,9,1)=="6"
RAMEND		label	$07ff
		elseif
RAMEND		label	$03ff		; end     "        "      "
		endif

;----------------------------------------------------------------------------
; Option Bytes

OPT0		label	$4800		; Read-out protection
ROP		bfield	OPT0,0,8
OPT1		label	$4801		; User boot code
UBC		bfield	OPT1,0,8
NOPT1		label	$4802
NUBC		bfield	NOPT1,0,8
OPT2		label	$4803		; Alternate function remapping
AFR		bfield	OPT2,0,8
NOPT2		label	$4804
NAFR		bfield	NOPT2,0,8
OPT3		label	$4805		; Misc. option
OPT_HSITRIM	bit	OPT3,4		;  high-speed internal clock trimming register size
LSI_EN		bit	OPT3,3		;  Low speed internal clock enable
IWDG_HW		bit	OPT3,2		;  Independent watchdog activation
WWDG_HW		bit	OPT3,1		;  Window watchdog activation
WWDG_HALT	bit	OPT3,0		;  Window watchdog reset on Halt
NOPT3		label	$4806
NHSITRIM	bit	NOPT3,4
NLSI_EN		bit	NOPT3,3
NIWDG_HW	bit	NOPT3,2
NWWDG_HW	bit	NOPT3,1
NWWDG_HALT	bit	NOPT3,0
OPT4		label	$4807		; Clock option
EXTCLK		bit	OPT4,3		;  External clock selection
CKAWUSEL	bit	OPT4,2		;  Auto-wakeup unit/clock
PRSC		bfield	OPT4,0,2	;  AWU clock prescaler
NOPT4		label	$4808
NEXTCLK		bit	NOPT4,3
NCKAWUSEL	bit	NOPT4,2
NPRSC		bfield	NOPT4,0,2
OPT5		label	$4809		; HSE clock startup
HSECNT		bfield	OPT5,0,8	;  HSE crystal oscillator stabilization time
NOPT5		label	$480a
NHSECNT		bfield	NOPT5,0,8

;----------------------------------------------------------------------------
; Vectors

RESET_vect	label	$8000		; Reset
TRAP_vect	label	$8004		; Software interrupt
TLI_vect	label	$8008		; External top level interrupt
AWU_vect	label	$800c		; Auto wake up from halt
CLK_vect	label	$8010		; Clock controller
EXTI0_vect	label	$8014		; Port A external interrupts
EXTI1_vect	label	$8018		; Port B external interrupts
EXTI2_vect	label	$801c		; Port C external interrupts
EXTI3_vect	label	$8020		; Port D external interrupts
EXTI4_vect	label	$8024		; Port E external interrupts
EXTI5_vect	label	$8028		; Port F external interrupts
SPI_vect	label	$8030		; End of transfer
TIM1_vect	label	$8034		; TIM1 update/overflow/underflow/trigger/break
TIM1_CAPT_vect	label	$8038		; TIM1 capture/compare
TIM5_vect	label	$803c		; TIM5 update /overflow
TIM5_CAPT_vect	label	$8040		; TIM5 capture/compare
LINUART_TX_vect	label	$804c		; Tx complete
LINUART_RX_vect	label	$8050		; Receive register DATA FULL
I2C_vect	label	$8054		; I2C interrupt
ADC_vect	label	$8060		; ADC1 end of conversion/analog watchdog interrupt
TIM6_vect	label	$8064		; TIM4 update/overflow
EEPROM_vect	label	$8068		; EOP/WR_PG_DIS

;----------------------------------------------------------------------------
; Unique ID

		include "stm8/stm8s/uid.inc"
		__defuid $4865

;----------------------------------------------------------------------------
; GPIO

		include	"stm8/stm8s/gpio.inc"
		__defgpio "PA",$5000
		__defgpio "PB",$5005
		__defgpio "PC",$500a
		__defgpio "PD",$500f
		__defgpio "PE",$5014
		__defgpio "PF",$5019

;----------------------------------------------------------------------------
; Flash

		include	"stm8/stm8s/flash.inc"
		__defflash $505a

;----------------------------------------------------------------------------
; Interrupt Controller

		include	"stm8/stm8s/itc.inc"
		__defexti $50a0,7
		__defitc $7f70,30

;----------------------------------------------------------------------------
; Reset Controller

		include	"stm8/stm8s/rst.inc"
		__defrst $50b3

;----------------------------------------------------------------------------
; Clock Controller

		include	"stm8/stm8s/clk.inc"
		__defclk $50c0

;----------------------------------------------------------------------------
; Window Watchdog

		include	"stm8/stm8s/wwdg.inc"
		__defwwdg $50d1

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

		include	"stm8/stm8s/spi.inc"
		__defspi $5200

;----------------------------------------------------------------------------
; I2C

		include	"stm8/stm8s/i2c.inc"
		__defi2c $5210

;----------------------------------------------------------------------------
; LINUART (TODO: no CR5)

		include "stm8/stm8s/uart3.inc"
		__defusart3 "UART4",$5230

;----------------------------------------------------------------------------
; Timer 1

		include	"stm8/stm8s/tim1.inc"
		__deftim1 $5250

;----------------------------------------------------------------------------
; Timer 5

		include	"stm8/stm8s/tim5.inc"
		__deftim5 $5340

;----------------------------------------------------------------------------
; Timer 6

		include	"stm8/stm8s/tim6.inc"
		__deftim6 $5340

;----------------------------------------------------------------------------
; A/D Converter 1

		include "stm8/stm8s/adc1.inc"
		__defadc1 "ADC",$53e0,$5400

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

                endif			; __stm8af62x3inc
