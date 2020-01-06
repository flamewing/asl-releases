		ifndef	_artim16inc	; avoid multiple inclusion
_artim16inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ARTIM16.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST62xx 16 Bit AR Timer            *
;*                                                                          *
;****************************************************************************

SCR1		sfr	0e8h		; Status Control Register 1
PSC		bfield	SCR1,6,2	;  Clock Prescaler
RELOAD		bit	5,SCR1		;  Reload Enabled
RUNRES		bit	4,SCR1		;  Run/Reset
OVFIEN		bit	3,SCR1		;  Overflow Interrupt Enable
OVFFLG		bit	2,SCR1		;  Overflow Occured
OVFMD		bit	1,SCR1		;  Overflow Output Mode
SCR2		sfr	0e1h		; Status Control Register 2
CP1ERR		bit	6,SCR2		;  CP1 Error Flag
CP2ERR		bit	5,SCR2		;  CP2 Error Flag
CP1IEN		bit	4,SCR2		;  CP1 Interrupt Enable
CP1FLG		bit	3,SCR2		;  CP1 Interrupt Flag
CP1POL		bit	2,SCR2		;  CP1 Edge Polarity Select
RLDSEL		bfield	SCR2,0,2	;  Reload Source Select
SCR3		sfr	0e2h		; Status Control Register 3
CP2POL		bit	7,SCR3		;  CP2 Edge Polarity Select
CP2IEN		bit	6,SCR3		;  CP2 Interrupt Enable
CP2FLG		bit	5,SCR3		;  CP2 Interrupt Flag
CMPIEN		bit	4,SCR3		;  Compare Interrupt Enable
CMFLG		bit	3,SCR3		;  Compare Flag
ZEROIEN		bit	2,SCR3		;  Compare to Zero Int Enable
ZEROFLG		bit	1,SCR3		;  Compare to Zero Flag
PWMMD		bit	0,SCR3		;  PWM Output Mode Control
SCR4		sfr	0e3h		; Status Control Register 4
OVFPOL		bit	3,SCR4		;  Overflow Output Polarity
OVFEN		bit	2,SCR4		;  Overflow Output Enable
PMPOL		bit	1,SCR4		;  PWM Output Polarity
PWMEN		bit	0,SCR4		;  PWM Output Enable
RLCPH		sfr	0e9h		; Reload/Capture Register High Byte
RLCPL		sfr	0eah		; Reload/Capture Register Low Byte
CPH		sfr	0ebh		; Capture Register High Byte
CPL		sfr	0ech		; Capture Register Low Byte
CMPH		sfr	0edh		; Compare Register High Byte
CMPL		sfr	0eeh		; Compare Register Low Byte
MASKH		sfr	0efh		; Mask Register High Byte
MASKL		sfr	0e0h		; Mask Register Low Byte

		restore
		endif			; _artim16inc
