		ifndef	__spi2inc	; avoid multiple inclusion
__spi2inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI2.INC                                                *
;*                                                                          *
;*  Contains SFR and Bit Definitions for ST62xx more complex                *
;*  Serial Peripheral Interface                                             *
;*                                                                          *
;****************************************************************************

DIV		sfr	0e1h		; SPI Divider
SPINT		bit	7,DIV		;  Interrupt Flag
SPDIV		bfield	DIV,3,4		;  Burst Mode Bit Clock Period Selection
CD		bfield	DIV,0,3		;  Base/Bit Clock Rate Selection
SPIDSR		sfr	0e0h		; SPI Data/Shift Register
MOD		sfr	0e2h		; SPI Mode Register
SPRUN		bit	7,MOD		;  SPI Run
SPIE		bit	6,MOD		;  SPI Interrupt Enable
CPHA		bit	5,MOD		;  Clock Phase Selection
SPCLK		bit	4,MOD		;  Base Clock Selection
SPIN		bit	3,MOD		;  Input Selection
SPSTRT		bit	2,MOD		;  Start Selection
EFILT		bit	1,MOD		;  Enable Filters
CPOL		bit	0,MOD		;  Clock Polarity
MISCR		sfr	0ddh		; Miscellaneous Register
D0		bit	0,MISCR		;  Selects the Sout Pin as the SPI Output Line

		restore
		endif			; __spi2inc
                                
                                