		ifndef	__stm8l151xxinc	; avoid multiple inclusion
__stm8l151xxinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG151XX.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L151X{2|3}                    *
;*   source: DS7204 Rev 11                                                  *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

E2START		label	$1000		; start address internal EEPROM
E2END		label	E2START+255	; end     "        "       "

FLASHSTART	label	$8000		; start address internal Flash

RAMSTART	label	$0000		; start address internal RAM
RAMEND		label	$03ff		; end     "        "      "

BLSTART		label	$6000		; start address boot loader
BLEND		label	$67ff		; end     "      "     "

;----------------------------------------------------------------------------
; Unique Id

		include "stm8/stm8s/uid.inc"
		__defuid $4926

;----------------------------------------------------------------------------
; Factory conversion registers

VREFINT_Factory_CONV	label	$4910	; Value of the internal reference voltage measured during the factory phase
TS_Factory_CONV_V90	label	$4911	; Value of the temperature sensor output voltage measured during the factory phase

;----------------------------------------------------------------------------
; Option Bytes

OPT0		label	$4800		; Read-out protection
ROP		bfield	OPT0,0,8	;  Memory readout protection
OPT1		label	$4802		; User boot code size
UBC		bfield	OPT1,0,8	;  Size of the user boot code area
OPT3		label	$4808		; Independent watchdog option
IWDG_HW		bit	OPT3,0		;  Independent watchdog activated by SW/HW
IWDG_HALT	bit	OPT3,1		;  Independent window watchdog off on Halt/Active-halt
WWDG_HW		bit	OPT3,2		;  Window watchdog activated by SW/HW
WWDG_HALT	bit	OPT3,3		;  Window window watchdog reset on Halt/Active-halt
OPT4		label	$4809		; Number of stabilization clock cycles for HSE and LSE oscillators
HSECNT		bfield	OPT4,0,2	;  Number of HSE oscillator stabilization clock cycles
LSECNT		bfield	OPT4,2,2	;  Number of LSE oscillator stabilization clock cycles
OPT5		label	$480a		; Brownout reset
BOR_TH		bfield	OPT5,1,3	;  Brownout reset thresholds
BOR_ON		bit	OPT5,0		;  Brownout reset on/off
OPTBL		label	$480b		; Bootloader option bytes (2 bytes)

;----------------------------------------------------------------------------
; Vectors

RESET_vect	label	$8000		; Reset
TRAP_vect	label	$8004		; Software interrupt
TLI_vect	label	$8008		; External top level interrupt
FLASH_vect	label	$800c		; FLASH end of programing/write attempted to protected page interrupt
DMA1_01_vect	label	$8010		; DMA1 channels 0/1 half transaction/transaction complete interrupt
DMA1_23_vect	label	$8014		; DMA1 channels 2/3 half transaction/transaction complete interrupt
RTC_vect	label	$8018		; RTC alarm A/wakeup/tamper 1/tamper 2/tamper 3
EXTIE_vect	label	$801c		; External interrupt port E, shared with...
PVD_vect	label	$801c		; PVD interrupt
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
CLK_vect	label	$804c		; CLK system clock switch/CSS interrupt
COMP_vect	label	$8050		; COMP1/COMP2 interrupt, shared with...
ADC1_vect	label	$8050		; ADC1 end of conversion/analog watchdog/overrun interrupt
TIM2_vect	label	$8054		; TIM2 update/overflow interrupt
TIM2_CAPT_vect	label	$8058		; TIM2 capture/compare interrupt
TIM3_vect	label	$805c		; TIM3 update/overflow/underflow/trigger/break interrupt
TIM3_CAPT_vect	label	$8060		; TIM3 capture/compare interrupt
RI_vect		label	$8064		; RI trigger interrupt
TIM4_vect	label	$806c		; TIM4 update/overflow interrupt
SPI1_vect	label	$8070		; SPI1 TX buffer empty/RX buffer not empty/error/wakeup interrupt
USART1_TX_vect	label	$8074		; USART1 transmit data register empty/transmission complete interrupt
USART1_RX_vect	label	$8078		; USART1 received data ready/overrun error/idle line detected/parity error/global error interrupt
I2C1_vect	label	$807c		; I2C1 interrupt

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

		include	"flash.inc"
		__defflash $5050

;----------------------------------------------------------------------------
; DMA

		include	"dma.inc"
		__defdma "DMA1",$5070

;----------------------------------------------------------------------------
; System Configuration

		include	"syscfg.inc"
		__defsyscfg $509d,1

;----------------------------------------------------------------------------
; Wait for Events

		include	"wfe.inc"
		__defwfe $50a6,1

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
PEIS		bfield	EXTI_CR3,4,2	;  Port E external interrupt sensitivity bits
PFIS		bfield	EXTI_CR3,6,2	;  Port F external interrupt sensitivity bits
EXTI_CR4	label	$50a0+$0a	; External interrupt control register 4
PGIS		bfield	EXTI_CR4,0,2	;  Port G external interrupt sensitivity bits
PHIS		bfield	EXTI_CR4,2,2	;  Port H external interrupt sensitivity bits
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
PHF		bit	EXTI_SR2,5	;  Port x external interrupt flag
PGF		bit	EXTI_SR2,4
PFF		bit	EXTI_SR2,3
PEF		bit	EXTI_SR2,2
PDF		bit	EXTI_SR2,1
PBF		bit	EXTI_SR2,0
EXTI_CONF1	label	$50a0+$05	; External interrupt port select register
PFES		bit	EXTI_CONF1,7	;  Port F or port E external interrupt select
PFLIS		bit	EXTI_CONF1,6	;  Port F[3:0] external interrupt select
PEHIS		bit	EXTI_CONF1,5	;  Port E[7:4] external interrupt select
PELIS		bit	EXTI_CONF1,4	;  Port E[3:0] external interrupt select
PDHIS		bit	EXTI_CONF1,3	;  Port D[7:4] external interrupt select
PDLIS		bit	EXTI_CONF1,2	;  Port D[3:0] external interrupt select
PBHIS		bit	EXTI_CONF1,1	;  Port B[7:4] external interrupt select
PBLIS		bit	EXTI_CONF1,0	;  Port B[3:0] external interrupt select
EXTI_CONF2	label	$50a0+$0b	; External interrupt port select register
PHDS		bit	EXTI_CONF2,6	;  Port H or port D external interrupt select
PGBS		bit	EXTI_CONF2,5	;  Port G or port B external interrupt select
PHHIS		bit	EXTI_CONF2,4	;  Port H[7:4] external interrupt select
PHLIS		bit	EXTI_CONF2,3	;  Port H[3:0] external interrupt select
PGHIS		bit	EXTI_CONF2,2	;  Port G[7:4] external interrupt select
PGLIS		bit	EXTI_CONF2,1	;  Port G[3:0] external interrupt select
PFHIS		bit	EXTI_CONF2,0	;  Port F[7:4] external interrupt select

		__defitc $7f70,30

;----------------------------------------------------------------------------
; Reset Controller

		include	"rst.inc"
		__defrst $50b0

;----------------------------------------------------------------------------
; Power Control

		include	"pwr.inc"
		__defpwr $50b2

;----------------------------------------------------------------------------
; Clock Controller

		include	"stm8/stm8s/clk.inc"
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
; Beeper

		include	"beep.inc"
		__defbeep $50f0

;----------------------------------------------------------------------------
; RTC

		include	"rtc.inc"
		__defrtc $5140

;----------------------------------------------------------------------------
; Serial Peripheral Interface

		include	"spi.inc"
		__defspi "SPI1",$5200

;----------------------------------------------------------------------------
; I2C

		include	"i2c.inc"
		__defi2c "I2C1",$5210,1

;----------------------------------------------------------------------------
; USART1

		include "usart.inc"
		__defusart "USART1",$5230

;----------------------------------------------------------------------------
; Timer 2/3

		include	"tim235.inc"
		__deftim235 "TIM2",$5250,1
		__deftim235 "TIM3",$5280,1

;----------------------------------------------------------------------------
; Timer 4

		include	"tim4.inc"
		__deftim4 $52e0,1

;----------------------------------------------------------------------------
; Infrared

		include	"irtim.inc"
		__defirtim $52ff

;----------------------------------------------------------------------------
; Routing Interface

RI_ICR1		label	$5431		; RI Timer input capture routing register 1
RI_IC2CS	bfield	RI_ICR1,0,5	;  TIM1 Input Capture 2 I/O selection
RI_ICR2		label	$5432		; RI Timer input capture routing register 2
RI_IC3CS	bfield	RI_ICR2,0,5	;  TIM1 Input Capture 3 I/O selection
RI_IOIR1	label	$5433		; RI I/O input register 1
RI_CH22I	bit	RI_IOIR1,7	;  I/O pin input value
RI_CH19I	bit	RI_IOIR1,6	
RI_CH16I	bit	RI_IOIR1,5
RI_CH13I	bit	RI_IOIR1,4
RI_CH10I	bit	RI_IOIR1,3
RI_CH7I		bit	RI_IOIR1,2
RI_CH4I		bit	RI_IOIR1,1
RI_CH1I		bit	RI_IOIR1,0
RI_IOIR2	label	$5434		; RI I/O input register 2
RI_CH23I	bit	RI_IOIR2,7	;  I/O pin input value
RI_CH20I	bit	RI_IOIR2,6
RI_CH17I	bit	RI_IOIR2,5
RI_CH14I	bit	RI_IOIR2,4
RI_CH11I	bit	RI_IOIR2,3
RI_CH8I		bit	RI_IOIR2,2
RI_CH5I		bit	RI_IOIR2,1
RI_CH2I		bit	RI_IOIR2,0
RI_IOIR3	label	$5435		; RI I/O input register 3
RI_CH24I	bit	RI_IOIR3,7	;  I/O pin input value
RI_CH21I	bit	RI_IOIR3,6
RI_CH18I	bit	RI_IOIR3,5
RI_CH15I	bit	RI_IOIR3,4
RI_CH12I	bit	RI_IOIR3,3
RI_CH9I		bit	RI_IOIR3,2
RI_CH6I		bit	RI_IOIR3,1
RI_CH3I		bit	RI_IOIR3,0
RI_IOCMR1	label	$5436		; RI I/O control mode register 1
RI_CH22M	bit	RI_IOCMR1,7	;  I/O control mode
RI_CH19M	bit	RI_IOCMR1,6	
RI_CH16M	bit	RI_IOCMR1,5
RI_CH13M	bit	RI_IOCMR1,4
RI_CH10M	bit	RI_IOCMR1,3
RI_CH7M		bit	RI_IOCMR1,2
RI_CH4M		bit	RI_IOCMR1,1
RI_CH1M		bit	RI_IOCMR1,0
RI_IOCMR2	label	$5437		; RI I/O control mode register 2
RI_CH23M	bit	RI_IOCMR2,7	;  I/O control mode
RI_CH20M	bit	RI_IOCMR2,6
RI_CH17M	bit	RI_IOCMR2,5
RI_CH14M	bit	RI_IOCMR2,4
RI_CH11M	bit	RI_IOCMR2,3
RI_CH8M		bit	RI_IOCMR2,2
RI_CH5M		bit	RI_IOCMR2,1
RI_CH2M		bit	RI_IOCMR2,0
RI_IOCMR3	label	$5438		; RI I/O control mode register 3
RI_CH24M	bit	RI_IOCMR3,7	;  I/O control mode
RI_CH21M	bit	RI_IOCMR3,6
RI_CH18M	bit	RI_IOCMR3,5
RI_CH53M	bit	RI_IOCMR3,4
RI_CH12M	bit	RI_IOCMR3,3
RI_CH9M		bit	RI_IOCMR3,2
RI_CH6M		bit	RI_IOCMR3,1
RI_CH3M		bit	RI_IOCMR3,0
RI_IOSR1	label	$5439		; RI I/O switch register 1
RI_CH22E	bit	RI_IOSR1,7	;  I/O switch control
RI_CH19E	bit	RI_IOSR1,6
RI_CH16E	bit	RI_IOSR1,5
RI_CH13E	bit	RI_IOSR1,4
RI_CH10E	bit	RI_IOSR1,3
RI_CH7E		bit	RI_IOSR1,2
RI_CH4E		bit	RI_IOSR1,1
RI_CH1E		bit	RI_IOSR1,0
RI_IOSR2	label	$543a		; RI I/O switch register 2
RI_CH23E	bit	RI_IOSR2,7	;  I/O switch control
RI_CH20E	bit	RI_IOSR2,6
RI_CH17E	bit	RI_IOSR2,5
RI_CH14E	bit	RI_IOSR2,4
RI_CH11E	bit	RI_IOSR2,3
RI_CH8E		bit	RI_IOSR2,2
RI_CH5E		bit	RI_IOSR2,1
RI_CH2E		bit	RI_IOSR2,0
RI_IOSR3	label	$543b		; RI I/O switch register 3
RI_CH24E	bit	RI_IOSR3,7	;  I/O switch control
RI_CH21E	bit	RI_IOSR3,6
RI_CH18E	bit	RI_IOSR3,5
RI_CH15E	bit	RI_IOSR3,4
RI_CH12E	bit	RI_IOSR3,3
RI_CH9E		bit	RI_IOSR3,2
RI_CH6E		bit	RI_IOSR3,1
RI_CH3E		bit	RI_IOSR3,0
RI_IOGCR	label	$543c		; RI I/O group control register
RI_IOM4		bfield	RI_IOGCR,6,2	;  I/O mode 4
RI_IOM3		bfield	RI_IOGCR,4,2	;  I/O mode 3
RI_IOM2		bfield	RI_IOGCR,2,2	;  I/O mode 2
RI_IOM1		bfield	RI_IOGCR,0,2	;  I/O mode 1
RI_ASCR1	label	$543d		; Analog switch register 1
RI_AS7		bit	RI_ASCR1,7	;  Analog switch control
RI_AS6		bit	RI_ASCR1,6
RI_AS5		bit	RI_ASCR1,5
RI_AS4		bit	RI_ASCR1,4
RI_AS3		bit	RI_ASCR1,3
RI_AS2		bit	RI_ASCR1,2
RI_AS1		bit	RI_ASCR1,1
RI_AS0		bit	RI_ASCR1,0
RI_ASCR2	label	$543e		; Analog switch register 2
RI_AS14		bit	RI_ASCR2,6	;  Analog switch control
RI_AS11		bit	RI_ASCR2,3
RI_AS10		bit	RI_ASCR2,2
RI_AS9		bit	RI_ASCR2,1
RI_AS8		bit	RI_ASCR2,0
RI_RCR		label	$543f		; Resistor control register
RI_400KPD	bit	RI_RCR,3	;  400 kOhm pull-down resistor
RI_10KPD	bit	RI_RCR,2	;  10 kOhm pull-down resistor
RI_400KPU	bit	RI_RCR,1	;  400 kOhm pull-up resistor
RI_10KPU	bit	RI_RCR,0	;  10 kOhm pull-up resistor
RI_CR		label	$5450		; I/O control register
RI_THALT	bit	RI_CR,3		;  Timer halted mode
RI_AM		bit	RI_CR,2		;  Acquisition mode
RI_TIF		bit	RI_CR,1		;  Trigger interrupt flag
RI_TIE		bit	RI_CR,0		;  Trigger interrupt enable
RI_MASKR1	label	$5451		; I/O mask register 1
RI_CH22EM	bit	RI_MASKR1,7	;  I/O event mask
RI_CH19EM	bit	RI_MASKR1,6
RI_CH16EM	bit	RI_MASKR1,5
RI_CH13EM	bit	RI_MASKR1,4
RI_CH10EM	bit	RI_MASKR1,3
RI_CH7EM	bit	RI_MASKR1,2
RI_CH4EM	bit	RI_MASKR1,1
RI_CH1EM	bit	RI_MASKR1,0
RI_MASKR2	label	$5452		; I/O mask register 2
RI_CH23EM	bit	RI_MASKR2,7	;  I/O event mask
RI_CH20EM	bit	RI_MASKR2,6
RI_CH17EM	bit	RI_MASKR2,5
RI_CH14EM	bit	RI_MASKR2,4
RI_CH11EM	bit	RI_MASKR2,3
RI_CH8EM	bit	RI_MASKR2,2
RI_CH5EM	bit	RI_MASKR2,1
RI_CH2EM	bit	RI_MASKR2,0
RI_MASKR3	label	$5453		; I/O mask register 3
RI_CH24EM	bit	RI_MASKR3,7	;  I/O event mask
RI_CH21EM	bit	RI_MASKR3,6
RI_CH18EM	bit	RI_MASKR3,5
RI_CH15EM	bit	RI_MASKR3,4
RI_CH12EM	bit	RI_MASKR3,3
RI_CH9EM	bit	RI_MASKR3,2
RI_CH6EM	bit	RI_MASKR3,1
RI_CH3EM	bit	RI_MASKR3,0
RI_MASKR4	label	$5454		; I/O mask register 4
RI_CH28EM	bit	RI_MASKR4,7	;  I/O event mask
RI_CH27EM	bit	RI_MASKR4,6
RI_CH26EM	bit	RI_MASKR4,1
RI_CH29EM	bit	RI_MASKR4,0
RI_IOIR4	label	$5455		; I/O input register 4
RI_CH28I	bit	RI_IOIR4,7	;  I/O pin input value
RI_CH27I	bit	RI_IOIR4,6
RI_CH26I	bit	RI_IOIR4,1
RI_CH29I	bit	RI_IOIR4,0
RI_IOCMR4	label	$5456		; I/O control mode register 4
RI_CH28M	bit	RI_IOCMR4,7	;  I/O control mode
RI_CH27M	bit	RI_IOCMR4,6
RI_CH26M	bit	RI_IOCMR4,1
RI_CH29M	bit	RI_IOCMR4,0
RI_IOSR4	label	$5457		; I/O switch register 4
RI_CH28E	bit	RI_IOSR4,7	;  I/O switch control
RI_CH27E	bit	RI_IOSR4,6
RI_CH26E	bit	RI_IOSR4,1
RI_CH29E	bit	RI_IOSR4,0

;----------------------------------------------------------------------------
; A/D Converter 1

		include "adc1.inc"
		__defadc1 "ADC",,$5340

;----------------------------------------------------------------------------
; Comparators

		include "comp.inc"
		__defcomp $5440

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

                endif			; __stm8l151xxinc
