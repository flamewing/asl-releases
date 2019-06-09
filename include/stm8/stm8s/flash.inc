		ifndef	__stm8sflashinc	; avoid multiple inclusion
__stm8sflashinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File FLASH.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S Flash Controller            *
;*                                                                          *
;****************************************************************************

__defflash 	macro	Base
FLASH_CR1	label	Base+$00	; Flash control register 1
HALT		bit	FLASH_CR1,3	;  Power-down in Halt mode
AHALT		bit	FLASH_CR1,2	;  Power-down in Active-halt mode
IE		bit	FLASH_CR1,1	;  Flash Interrupt enable
FIX		bit	FLASH_CR1,0	;  Fixed Byte programming time
FLASH_CR2	label	Base+$01	; Flash control register 2
OPT		bit	FLASH_CR2,7	;  Write option bytes
WPRG		bit	FLASH_CR2,6	;  Word programming
ERASE		bit	FLASH_CR2,5	;  Block erasing
FPRG		bit	FLASH_CR2,4	;  Fast block programming
PRG		bit	FLASH_CR2,0	;  Standard block programming
FLASH_NCR2	label	Base+$02	; Flash complementary control register 2
NOPT		bit	FLASH_NCR2,7	;  Write option bytes
NWPRG		bit	FLASH_NCR2,6	;  Word programming
NERASE		bit	FLASH_NCR2,5	;  Block erase
NFPRG		bit	FLASH_NCR2,4	;  Fast block programming
NPRG		bit	FLASH_NCR2,0	;  Block programming
FLASH_FPR	label	Base+$03	; Flash protection register
WPB5		bit	FLASH_FPR,5	;  User boot code area protection bits
WPB4		bit	FLASH_FPR,4
WPB3		bit	FLASH_FPR,3
WPB2		bit	FLASH_FPR,2
WPB1		bit	FLASH_FPR,1
WPB0		bit	FLASH_FPR,0
FLASH_NFPR	label	Base+$04	; Flash complementary protection register
NWPB5		bit	FLASH_NFPR,5	;  User boot code area protection bits
NWPB4		bit	FLASH_NFPR,4
NWPB3		bit	FLASH_NFPR,3
NWPB2		bit	FLASH_NFPR,2
NWPB1		bit	FLASH_NFPR,1
NWPB0		bit	FLASH_NFPR,0
FLASH_IAPSR	label	Base+$05	; Flash in-application programming status register
HVOFF		bit	FLASH_IAPSR,6	;  End of high voltage flag
DUL		bit	FLASH_IAPSR,3	;  Data EEPROM area unlocked flag
EOP		bit	FLASH_IAPSR,2	;  End of programming (write or erase operation) flag
PUL		bit	FLASH_IAPSR,1	;  Flash Program memory unlocked flag
WR_PG_DIS	bit	FLASH_IAPSR,0	;  Write attempted to protected page flag
FLASH_PUKR	label	Base+$08	; Flash Program memory unprotection register
PUK		bfield	FLASH_IAPSR,0,8	;  Main program memory unlock keys
FLASH_DUKR	label	Base+$0a	; Data EEPROM unprotection register
DUK		bfield	FLASH_DUKR,0,8	;  Data EEPROM write unlock keys
		endm

		restore
		endif			; __stm8sflashinc
