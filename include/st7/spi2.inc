		ifndef	__st7spi2inc	; avoid multiple inclusion
__st7spi2inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72xxx SPI                       *
;*                                                                          *
;****************************************************************************

__defspi	macro	Base
SPIDR		label	Base+$00	; Data I/O Register
SPICR		label	Base+$01	; Control Register
SPIE		bit	SPICR,7		;  Serial peripheral interrupt enable
SPE		bit	SPICR,6		;  Serial peripheral output enable
SPR2		bit	SPICR,5		;  Divider Enable
MSTR		bit	SPICR,4		;  Master
CPOL		bit	SPICR,3		;  Clock polarity
CPHA		bit	SPICR,2		;  Clock phase
SPR1		bit	SPICR,1		;  Serial peripheral rate
SPR0		bit	SPICR,0
SPICSR		label	Base+$02	; Control/Status Register
SPIF		bit	SPICSR,7	;  Serial Peripheral data transfer flag
WCOL		bit	SPICSR,6	;  Write Collision status
OVR		bit	SPICSR,5	;  Overrun error
MODF		bit	SPICSR,4	;  Mode Fault flag
SOD		bit	SPICSR,2	;  SPI Output Disable
SSM		bit	SPICSR,1	;  /SS Management
SSI		bit	SPICSR,0	;  /SS Internal Mode
		endm

		restore
		endif			; __st7spi2inc
