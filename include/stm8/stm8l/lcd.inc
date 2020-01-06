		ifndef	__stm8llcdinc	; avoid multiple inclusion
__stm8llcdinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File LCD.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L LCD Controller              *
;*                                                                          *
;*   Source: DocID15226 Rev 14 / RM0031                                     *
;*                                                                          *
;****************************************************************************

__deflcd	macro	Base,NumSegs,NumRAM,HasCR4
LCD_CR1		label	Base+$00	; LCD control register 1
LCD_BLINK	bfield	LCD_CR1,6,2	;  Blink enable
LCD_BLINKF	bfield	LCD_CR1,3,3	;  Blink frequency
LCD_DUTY	bfield	LCD_CR1,1,2	;  Duty ratio selection
LCD_B2		bit	LCD_CR1,0	;  Bias selector
LCD_CR2 	label	Base+$01	; LCD control register 2
PON		bfield	LCD_CR2,5,3	;  Pulse ON duration
HD		bit	LCD_CR2,4	;  High drive enable
CC		bfield	LCD_CR2,1,3	;  Contrast control
VSEL		bit	LCD_CR2,0	;  LCD voltage source
LCD_CR3 	label	Base+$02	; LCD control register 3
LCD_LCDEN	bit	LCD_CR3,6	;  LCD enable
LCD_SOFIE	bit	LCD_CR3,5	;  Interrupt enable
LCD_SOF		bit	LCD_CR3,4	;  Start of frame flag
LCD_SOFC	bit	LCD_CR3,3	;  SOF flag clear
LCD_DEAD	bfield	LCD_CR3,0,3	;  Dead time enable
LCD_FRQ 	label	Base+$03	; LCD frequency selection register
LCD_PS		bfield	LCD_FRQ,4,4	;  CLK 16-bit prescaler
LCD_DIV		bfield	LCD_FRQ,0,4	;  Divider from 16 to 31
		if	HasCR4
LCD_CR4		 label	$542f		; LCD control register 4
LCD_PAGE_COM	 bit	LCD_CR4,2	;  LCD_RAM page selector
LCD_DUTY8	 bit	LCD_CR4,1	;  1/8 duty enable
LCDB4		 bit	LCD_CR4,0	;  1/4 bias enable
		endif
__I		set	0
		rept	NumSegs
		if	(__I&7)==0
		__decstr __NRS,__I/8
LCD_PM{__NRS}	label	Base+$04+(__I/8) ; Port mask register n
		endif
		__dec02str __NS,__I
LCD_SEG{__NS}	bit	LCD_PM{__NRS},__I&7 ; Port mask for SEG i
__I		set	__I+1
		endm
__N		set	0
		rept	NumRAM
		__decstr __NS,__N
LCD_RAM{__NS}	label	Base+$0c+__N	; LCD display memory n
__N		set	__N+1
		endm
		endm

		restore
		endif			; __stm8llcdinc
