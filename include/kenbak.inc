		ifndef	__kenbak_inc	; avoid multiple inclusion
__kenbak_inc	equ	1		

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - Datei KENBAK.INC                                             *
;*   								            *
;*   Contains Address Definitions for KENBAK(-1)                            *
;*                                                                          *
;****************************************************************************

RegisterA	equ	00h		; Registers
RegisterB	equ	01h
RegisterX	equ	02h
RegisterP	equ	03h

OverflowA	equ	81h		; Overflow & Carry
CAA		bit	1,OverflowA
OFA		bit	0,OverflowA
OverflowB	equ	82h
CAB		bit	1,OverflowB
OFB		bit	0,OverflowB
OverflowX	equ	83h
CAX		bit	1,OverflowX
OFX		bit	0,OverflowX

Input		equ	0ffh		; Switch Input

Lamps		equ	80h		; LED/Lamp Row

		restore			; allow listing again
		endif			; __kenbak_inc
