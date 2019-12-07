		ifndef	__st6instrinc
__st6instrinc	equ	1

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File INSTR.INC                                               *
;*                                                                          *
;*   contains macros for some commonly missing instructions on ST6          *
;*                                                                          *
;****************************************************************************

; Add/Subtract with Carry: first operand must be A (accumulator)

adc             macro   dst,src
		jrnc    NoCarry
		inc     dst
NoCarry:        add     dst,src         ; Rest normal addieren
		endm

sbc             macro   dst,src
		jrnc    NoCarry
		dec     dst
NoCarry:        sub     dst,src
		endm

; OR operator:
; uses De Morgan's rule: A OR B = /((/A) AND (/B))
; uses W register
; dst must be A (accumulator)

or              macro   dst,src
		com     dst
		ld      w,dst
		ld      dst,src
		com     dst
		and     dst,w
		com     dst
		endm

; XOR Operator:
; uses the principle of four NAND gates, i.e. does not have to use the OR macro
; uses V and W register
; dst must be A (accumulator)

xor             macro   dst,src
		ld      v,dst           ; Op1 retten
		and     dst,src         ; Hilfsergebnis A NAND B bilden
		com     dst
		ld      w,dst           ; in W retten
		ld      dst,v           ; Op1 nochmal holen
		and     dst,w           ; 1. Teilergebnis A NAND W bilden
		com     dst
		ld      v,dst           ; in V retten
		ld      dst,src         ; 2. Teilergebnis B NAND W bilden
		and     dst,w
		com     dst
		and     dst,v
		com     dst
		endif
		endm

; rotate right with carry
; fairly brute force, I did not have any better idea...
; dst must be A (accumulator)

rrc             macro   dst
		rept    7
		 rlc    dst
		endm
		endm

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; For those missing the ROMWIN statement:

romwin		macro	adr
		assume	rombase:adr>>6
		endm

		endif			; __st6instrinc
