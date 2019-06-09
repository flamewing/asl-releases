		ifndef	__stm8laesinc	; avoid multiple inclusion
__stm8laesinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File AES.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L AES Unit                    *
;*   source: DocID15226 Rev 14 (RM0031)                                     *
;*                                                                          *
;****************************************************************************

__defaes	macro	Base
AES_CR		label	Base+$00	; AES control register
AES_DMAEN	bit	AES_CR,7	;  DMA enable
AES_ERRIE	bit	AES_CR,6	;  Error interrupt enable
AES_CCIE	bit	AES_CR,5	;  Computation complete interrupt enable
AES_ERRFC	bit	AES_CR,4	;  Error flags clear
AES_CCFC	bit	AES_CR,3	;  Computation complete flag clear
AES_MODE	bfield	AES_CR,1,2	;  AES mode of operation
AES_EN		bit	AES_CR,0	;  Enable
AES_SR		label	Base+$01	; AES status register
AES_WRERR	bit	AES_SR,2	;  Write error flag
AES_RDERR	bit	AES_SR,1	;  Read error flag
AES_CCF		bit	AES_SR,0	;  Computation complete flag
AES_DINR	label	Base+$02	; AES data input register
AES_DOUTR	label	Base+$03	; AES data output register
		endm

		restore
		endif			; __stm8laesinc
