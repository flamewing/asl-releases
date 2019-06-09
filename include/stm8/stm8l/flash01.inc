		ifndef	__stm8lflash01inc	; avoid multiple inclusion
__stm8lflash01inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File FLASH01.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8Lx01 Flash Controller         *
;*                                                                          *
;****************************************************************************

__defflash 	macro	Base
FLASH_CR1	label	Base+$00	; Flash control register 1
IE		bit	FLASH_CR1,1	;  Flash Interrupt enable
FIX		bit	FLASH_CR1,0	;  Fixed Byte programming time
FLASH_CR2	label	Base+$01	; Flash control register 2
OPT		bit	FLASH_CR2,7	;  Write option bytes
WPRG		bit	FLASH_CR2,6	;  Word programming
ERASE		bit	FLASH_CR2,5	;  Block erasing
FPRG		bit	FLASH_CR2,4	;  Fast block programming
PRG		bit	FLASH_CR2,0	;  Standard block programming
FLASH_PUKR	label	Base+$02	; Flash Program memory unprotection register
PUK		bfield	FLASH_IAPSR,0,8	;  Main program memory unlock keys
FLASH_DUKR	label	Base+$03	; Data EEPROM unprotection register
DUK		bfield	FLASH_DUKR,0,8	;  Data EEPROM write unlock keys
FLASH_IAPSR	label	Base+$04	; Flash in-application programming status register
DUL		bit	FLASH_IAPSR,3	;  Data EEPROM area unlocked flag
EOP		bit	FLASH_IAPSR,2	;  End of programming (write or erase operation) flag
PUL		bit	FLASH_IAPSR,1	;  Flash Program memory unlocked flag
WR_PG_DIS	bit	FLASH_IAPSR,0	;  Write attempted to protected page flag
		endm

		restore
		endif			; __stm8lflash01inc
