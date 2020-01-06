		ifndef	__stm8lusartinc	; avoid multiple inclusion
__stm8lusartinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File USART.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L U(S)ART                     *
;*                                                                          *
;****************************************************************************

		include	"stm8/stm8l/usart01.inc"
__defusart	macro	NAME,Base
		__defusart01 NAME,Base
__NS		set	"\{NAME}_"
{__NS}CR5	label	Base+8		; control register 5
{__NS}DMAT	bit	{__NS}CR5,7	;  DMA Enable transmitter
{__NS}DMAR	bit	{__NS}CR5,6	;  DMA Enable receiver
{__NS}SCEN	bit	{__NS}CR5,5	;  Smartcard mode enable
{__NS}NACK	bit	{__NS}CR5,4	;  Smartcard NACK enable
{__NS}HDSEL	bit	{__NS}CR5,3	;  Half-Duplex Selection
{__NS}IRLP	bit	{__NS}CR5,2	;  IrDA Low Power
{__NS}IREN	bit	{__NS}CR5,1	;  IrDA mode Enable
{__NS}GTR	label	Base+9		; guard time register
{__NS}PSCR	label	Base+10		; prescaler register
		endm

		restore
		endif			; __stm8lusartinc
