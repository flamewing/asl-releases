		ifndef  stddef47inc     ; avoid multiple inclusion
stddef47inc     equ     1

		save
		listing off		; no listing over this file
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF47.INC                                            *
;*                                                                          *
;*   Contains the "extended instruction set" for TLCS-47(0(A*)) Processors  *                                            *
;*                                                                          *
;****************************************************************************

                if      (MOMCPU<>293888)&&(MOMCPU<>4656128)&&(MOMCPU<>74492928)
		 fatal  "wrong target selected: only 47C00, 470C00, oder 470AC00 allowed"
		endif

                if      MOMPASS=1
		 message "TLCS-47 Macro Definitions"
		 message "(C) 1994 Alfred Arnold"
                 message "Target Processor: \{MOMCPU}"
		endif

;----------------------------------------------------------------------------
; Conditional Jumps

bz		macro	adr		; Compare and Branch if ZF=1
		testp	zf
		b	adr
		endm


bnz		macro	adr		; Compare and Branch if ZF=0
		testp	zf
		b	s1
		b	adr
s1:
		endm


bc		macro	adr		; Compare and Branch if CF=1
		testp	cf
		b	adr
		endm


bnc		macro	adr		; Compare and Branch if CF=0
		test	cf
		b	adr
		endm


be		macro	dest,src,adr	; Compare and Branch if equal
		cmpr	dest,src
		testp	zf
		b	adr
		endm


bne		macro	dest,src,adr	; Compare and Branch if unequal
		cmpr	dest,src
                if      UpString("DEST")<>"A"
		 testp	zf
		 b	s1
		endif
		b	adr
s1:
		endm


bge		macro	dest,src,adr	; Compare and Branch if greater than or equal
		cmpr	dest,src
		testp	zf
		b	adr
		test	cf
		b	adr
		endm


bgt		macro	dest,src,adr	; Compare and Branch if greater than
		cmpr	dest,src
		test	cf
		b	adr
		endm


ble		macro	dest,src,adr	; Compare and Branch if less than or equal
		cmpr	dest,src
		testp	cf
		b	adr
		endm


blt		macro	dest,src,adr	; Compare and Branch if less than
		cmpr	dest,src
		testp	zf
		b	s1
		testp	cf
		b	adr
s1:
		endm

;----------------------------------------------------------------------------
; Sub Program Calls

callss		macro	vector		; Via Vector
		if	vector=0
		 calls	86h
		elseif
		 calls	(vector<<3)+6
		endif
		endm


callz		macro	adr		; If ZF=1
		testp	zf
		b	s1
		b	s2
s1:		call	adr
s2:
		endm


callnz		macro	adr		; If ZF=0
		testp	zf
		b	s1
		call	adr
s1:
		endm


callc		macro	adr		; If CF=1
		test	cf
		b	s1
		call	adr
s1:
		endm


callnc		macro	adr		; If CF=0
		testp	cf
		b	s1
		call	adr
s1:
		endm

;----------------------------------------------------------------------------
; Sub Program Returns

retz		macro			; If ZF=1
		testp	zf
		b	s1
		b	s2
s1:		ret
s2:
		endm


retnz		macro			; If ZF=0
		testp	zf
		b	s1
		ret
s1:
		endm


retc		macro			; If CF=1
		test	cf
		b	s1
		ret
s1:
		endm


retnc		macro			; If CF=0
		testp	cf
		b	s1
		ret
s1:
		endm

;----------------------------------------------------------------------------
; Interrupt Returns

retiz		macro			; If ZF=1
		testp	zf
		b	s1
		b	s2
s1:		reti
s2:
		endm


retinz		macro			; If ZF=0
		testp	zf
		b	s1
		reti
s1:
		endm


retic		macro			; If CF=1
		test	cf
		b	s1
		reti
s1:
		endm


retinc		macro			; If CF=0
		testp	cf
		b	s1
		reti
s1:
		endm

;----------------------------------------------------------------------------
; Extended Shift Instructions

shl		macro	op,cnt		; Logical Shift Left
                if      UpString("OP")<>"A"
		 xch	a,op
		endif
		rept	cnt
		 test	cf
		 rolc	a
		endm
                if      UpString("OP")<>"A"
		 xch	a,op
		endif
		endm

shr		macro	op,cnt		; Logical Shift Right
                if      UpString("OP")<>"A"
		 xch	a,op
		endif
		rept	cnt
		 test	cf
		 rorc	a
		endm
                if      UpString("OP")<>"A"
		 xch	a,op
		endif
		endm

;----------------------------------------------------------------------------
; Interrupt Enable/Disable

ei		macro
		eiclr	il,3fh
		endm

di		macro
		diclr	il,3fh
		endm

;---------------------------------------------------------------------------

		restore			; re-allow listing

		endif			; stddef47inc
