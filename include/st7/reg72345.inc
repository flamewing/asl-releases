		ifndef	__reg72345inc	; avoid multiple inclusion
__reg72345inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG72345.INC                                            *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72345                           *
;*                                                                          *
;*   Source: ST72345/45 Data Sheet, Rev. 6, July 2012                       *
;*                                                                          *
;****************************************************************************

		include	"reg72344.inc"

;----------------------------------------------------------------------------
; I2C3SNS

I2C3SNS_12_vect	label	$ffec		; I2C3SNS Address 1&2 Interrupt
I2C3SNS_3_vect	label	$ffee		; I2C3SNS Address 3 Interrupt

I2C3SCR1	label	$0060		; I2C3SNS Control Register 1
PL		bfield	I2C3SCR1,6,2	;  Page length configuration
ITER		bit	I2C3SCR1,4	;  BERR / NACK Interrupt enable
ITRE3		bit	I2C3SCR1,3	;  Interrupt enable on read from Slave 3
ITRE12 		bit	I2C3SCR1,2	;  Interrupt enable on read from Slave 1 or 2
ITWE3		bit	I2C3SCR1,1	;  Interrupt enable on write to Slave 3
ITWE12		bit	I2C3SCR1,0	;  Interrupt enable on write to Slave 1 or 2
I2C3SCR2	label	$0061		; I2C3SNS Control Register 2
WP2		bit	I2C3SCR2,4	;  Write Protect enable for Slave 2
WP1		bit	I2C3SCR2,3	;  Write Protect enable for Slave 1
I2C3_PE		bit	I2C3SCR2,2	;  Peripheral enable
BusyW		bit	I2C3SCR2,1	;  Busy on Write to RAM Buffer
B_W		bit	I2C3SCR2,0	;  Byte / Word Mode
I2C3SSR		label	$0062		; I2C3SNS Status Register
I2C3SBCR	label	$0063		; I2C3SNS Byte Count Register
I2C3SSAR1	label	$0064		; I2C3SNS Slave Address 1 Register
I2C3SCAR1	label	$0065		; I2C3SNS Current Address 1 Register
I2C3SSAR2	label	$0066		; I2C3SNS Slave Address 2 Register
I2C3SCAR2	label	$0067		; I2C3SNS Current Address 2 Register
I2C3SSAR3	label	$0068		; I2C3SNS Slave Address 3 Register
I2C3SCAR3	label	$0069		; I2C3SNS Current Address 3 Register

		restore

		endif			; __reg72345inc
