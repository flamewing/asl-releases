		ifndef	__s12z_spi_inc
__s12z_spi_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZSPI.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Serial/Parallel Interface *
;*                                                                          *
;****************************************************************************

		padding	off

S12ZSPI		struct	dots
CR1		ds.b	1		; ($00) SPI Control Register 1
SPIE		s12zbit	CR1,7		;  SPI Interrupt Enable Bit
SPE		s12zbit	CR1,6		;  SPI System Enable Bit
SPTIE		s12zbit	CR1,5		;  SPI Transmit Interrupt Enable
MSTR		s12zbit	CR1,4		;  SPI Master/Slave Mode Select Bit
CPOL		s12zbit	CR1,3		;  SPI Clock Polarity Bit
CPHA		s12zbit	CR1,2		;  SPI Clock Phase Bit
SSOE		s12zbit	CR1,1		;  Slave Select Output Enable
LSBFE		s12zbit	CR1,0		;  LSB-First Enable
CR2		ds.b	1		; ($01) SPI Control Register 2
XFRW		s12zbit	CR2,6		;  Transfer Width
MODFEN		s12zbit	CR2,4		;  Mode Fault Enable Bit
BIDIROE		s12zbit	CR2,3		;  Output Enable in the Bidirectional Mode of Operation
SPISWAI		s12zbit	CR2,1		;  SPI Stop in Wait Mode Bit
SPC0		s12zbit	CR2,0		;  Serial Pin Control Bit 0
BR		ds.b	1		; ($02) SPI Baud Rate Register
SPPR		s12zfld	BR,3:4		;  SPI Baud Rate Preselection Bits
SPR		s12zfld	BR,3:0		;  SPI Baud Rate Selection Bits
SR		ds.b	1		; ($03) SPI Status Register
SPIF		s12zbit	SR,7		;  SPIF Interrupt Flag
SPTEF		s12zbit	SR,5		;  SPI Transmit Empty Interrupt Flag
MODF		s12zbit	SR,4		;  Mode Fault Flag
DR		ds.w	0		; ($04) SPI Data Register (16 bits)
DRH		ds.b	1		; ($04) SPI Data Register MSB
DRL		ds.b	1		; ($05) SPI Data Register LBS
		ends	S12ZSPI

		restore			; re-enable listing

		endif			; __s12z_spi_inc
