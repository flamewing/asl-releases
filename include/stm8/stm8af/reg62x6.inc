		ifndef	__stm8af62x6inc	; avoid multiple inclusion
__stm8af62x6inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG62X6.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8AF6246/48/66/68               *
;*   source: DocID14952 Rev 11                                              *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

E2START		label	$4000		; start address internal EEPROM
		if	SUBSTR(MOMCPUNAME,8,1)=="4"
E2END		label	E2START+$01ff	; 6248/6246: 0.5K EEPROM
		elseif
E2END		label	E2START+$03ff	; 6268/6266: 1K EEPROM
		endif

FLASHSTART	label	$8000		; start address internal Flash

RAMSTART	label	$0000		; start address internal RAM
RAMEND		label	$07ff		; end     "        "      "

BLSTART		label	$6000		; start address boot loader
BLEND		label	$67ff		; end     "       "    "

;----------------------------------------------------------------------------
; Option Bytes

OPT0		label	$4800		; Read-out protection
ROP		bfield	OPT0,0,8
OPT1		label	$4801		; User boot code
UBC		bfield	OPT1,0,6
NOPT1		label	$4802
NUBC		bfield	NOPT1,0,6
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
OPT6		label	$480b		; TMU
TMU		bfield	OPT6,0,8	;  Enable temporary memory unprotection
NOPT6		label	$480c
NTMU		bfield	NOPT6,0,8
OPT7		label	$480d		; Flash Wait States
WAITSTATE	bit	OPT7,0
NOPT7		label	$480e
NWAITSTATE	bit	OPT7,0
OPT8		label	$4810		; TMU
TMU_KEY1	bfield	OPT8,0,8
OPT9		label	$4811
TMU_KEY2	bfield	OPT9,0,8
OPT10		label	$4812
TMU_KEY3	bfield	OPT10,0,8
OPT11		label	$4813
TMU_KEY4	bfield	OPT11,0,8
OPT12		label	$4814
TMU_KEY5	bfield	OPT12,0,8
OPT13		label	$4815
TMU_KEY6	bfield	OPT13,0,8
OPT14		label	$4816
TMU_KEY7	bfield	OPT14,0,8
OPT15		label	$4817
TMU_KEY8	bfield	OPT15,0,8
OPT16		label	$4818
TMU_MAXATT	bfield	OPT16,0,8
OPT17		label	$487e		; Boot Loader
BL		bfield	OPT17,0,8
NOPT17		label	$487f
NBL		bfield	NOPT17,0,8

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
SPI_vect	label	$8030		; End of transfer
TIM1_vect	label	$8034		; TIM1 update/overflow/underflow/trigger/break
TIM1_CAPT_vect	label	$8038		; TIM1 capture/compare
TIM2_vect	label	$803c		; TIM2 update/overflow/underflow/trigger/break
TIM2_CAPT_vect	label	$8040		; TIM2 capture/compare
TIM3_vect	label	$8044		; TIM3 update/overflow/underflow/trigger/break
TIM3_CAPT_vect	label	$8048		; TIM3 capture/compare
I2C_vect	label	$8054		; I2C interrupt
LINUART_TX_vect	label	$8058		; Tx complete
LINUART_RX_vect	label	$805c		; Receive register DATA FULL
ADC_vect	label	$8060		; ADC1 end of conversion/analog watchdog interrupt
TIM4_vect	label	$8064		; TIM4 update/overflow
EEPROM_vect	label	$8068		; EOP/WR_PG_DIS

;----------------------------------------------------------------------------
; GPIO

		include	"stm8/stm8s/gpio.inc"
		__defgpio "PA",$5000
		__defgpio "PB",$5005
		__defgpio "PC",$500a
		__defgpio "PD",$500f
		__defgpio "PE",$5014
		__defgpio "PF",$5019
		__defgpio "PG",$501e

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

		include "stm8/stm8s/uart2.inc"
		__defusart2 "UART2",$5240

;----------------------------------------------------------------------------
; Timer 1

		include	"stm8/stm8s/tim1.inc"
		__deftim1 $5250

;----------------------------------------------------------------------------
; Timer 2

		include	"stm8/stm8s/tim2.inc"
		__deftim2 $5300,0

;----------------------------------------------------------------------------
; Timer 3

		include	"stm8/stm8s/tim3.inc"
		__deftim3 $5320,0

;----------------------------------------------------------------------------
; Timer 4

		include	"stm8/stm8s/tim4.inc"
		__deftim4 $5340,0

;----------------------------------------------------------------------------
; A/D Converter 1

		include "stm8/stm8s/adc1.inc"
		__defadc1 "ADC",$53e0,$5400

;----------------------------------------------------------------------------
; Temporary Memory Unprotection

		include	"tmu.inc"
		__deftmu $5800

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

                endif			; __stm8af62x6inc
