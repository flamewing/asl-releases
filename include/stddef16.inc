		ifndef  stddef16inc     ; avoid multiple inclusion
stddef16inc     equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF16.INC                                            *
;*   								            *
;*   Contains SFR and Macro Definitions for PIC 16C5x MCUs                  *
;* 									    *
;****************************************************************************

                if      (MOMCPU<93268)&&(MOMCPU>93271)
		 fatal  "wrong target selected: only 16C54..16C57 supported"
		endif

                if      MOMPASS=1
		 message "PIC16C5x Definitions (C) 1993 Alfred Arnold"
		endif

;----------------------------------------------------------------------------
; Special Registers

Indirect	SFR	0		; Indirect Addressierung
PCLo            SFR     2               ; Program Counter LSB
RTCC            SFR     1               ; Counter
Status		SFR	3		; Status Register
FSR		SFR	4		; File Select Register
Port_A		SFR	5		; Ports
Port_B		SFR	6
Port_C		SFR	7

;----------------------------------------------------------------------------
; Bits in Status Register

Flag_C		EQU	0		; Carry
Flag_DC		EQU	1		; Digit Carry
Flag_Z		EQU	2		; Zero

PA1		EQU	5		; Bank-Select Bit 9
PA2		EQU	6		;  "     "     "  10

;----------------------------------------------------------------------------
; Set/Clear Flags

clrc		macro			; Clear Carry
		bcf	Status,Flag_C
		endm

setc		macro			; Set Carry
		bsf	Status,Flag_C
		endm

clrdc		macro			; Clear Digit Carry
		bcf	Status,Flag_DC
		endm

setdc		macro			; Set Digit Carry
		bsf	Status,Flag_DC
		endm

clrz		macro			; Clear Zero Flag
		bcf	Status,Flag_Z
		endm

setz		macro			; Set Zero Flag
                bsf     Status,Flag_Z
		endm

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Simplified Skip Instructions

skpc		macro			; Skip if C=1
		btfss	Status,Flag_C
		endm

skpnc		macro			; Skip if C=0
		btfsc	Status,Flag_C
		endm

skpdc		macro			; Skip if DC=1
		btfss	Status,Flag_DC
		endm

skpndc		macro			; Skip if DC=0
		btfsc	Status,Flag_DC
		endm

skpz		macro			; Skip if Z=1
		btfss	Status,Flag_Z
		endm

skpnz		macro			; Skip if Z=0
		btfsc	Status,Flag_Z
		endm

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Conditional Branches
; CAUTION: Multi-Word Instructions, can not be skipped!!!!

b		macro	target		; Unconditional Branch
		goto	target
		endm

bc		macro	target		; Branch if C=1
		btfsc	Status,Flag_C
		goto	target
		endm

bnc		macro	target		; Branch if C=0
		btfss	Status,Flag_C
		goto	target
		endm

bdc		macro	target		; Branch if DC=1
		btfsc	Status,Flag_DC
		goto	target
		endm

bndc		macro	target		; Branch if DC=0
		btfss	Status,Flag_DC
		goto	target
		endm

bz		macro	target		; Branch if Z=1
		btfss	Status,Flag_Z
		goto	target
		endm

bnz		macro	target		; Branch if Z=0
		btfsc	Status,Flag_Z
		goto	target
		endm


;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Carry Arithmetic
; see above for CAUTION

addcf		macro	reg,dest	; Add Carry to Register
		btfsc	Status,Flag_C
		incf	reg,dest
		endm

adddcf		macro	reg,dest	; Add Digit Carry to Register
		btfsc	Status,Flag_DC
		incf	reg,dest
		endm

subcf		macro	reg,dest	; Subtract Carry from Register
		btfsc	Status,Flag_C
		decf	reg,dest
		endm

subdcf		macro	reg,dest	; Substract Digit Carry from Register
		btfsc	Status,Flag_DC
		decf	reg,dest
		endm

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Miscellaneous

movfw		macro	reg		; Transfer Register-->W
		movf	reg,0
		endm

negf		macro	reg,dest	; Two's Complement
		comf	reg,1
		incf	reg,dest
		endm

tstf		macro	reg		; Set Flags according to Register Content
		movf	reg,1
		endm

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                restore                 ; allow listing again

		endif			; stddef16inc
