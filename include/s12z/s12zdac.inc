		ifndef	__s12z_dac_inc
__s12z_dac_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZDAC.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Digital/Analog Converter  *
;*                                                                          *
;****************************************************************************

DACCTL		equ	$0680		; Control Register
FVR		s12zbit	DACCTL,7	;  Full Voltage Range
DRIVE		s12zbit	DACCTL,6	;  Drive Select
DACM		s12zfld	DACCTL,3:0	;  Mode Select
DACVOL		equ	$0682		; Analog Output Voltage Level Register
VOLTAGE		s12zfld	DACVOL,8:0	;  analog output voltage
DACDEBUG	equ	$0687		; reserved

		restore			; re-enable listing

		endif			; __s12z_dac_inc
