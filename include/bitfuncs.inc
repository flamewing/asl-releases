		ifndef   bitfuncsinc    ; avoid multiple inclusion
bitfuncsinc     equ      1

                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File BITFUNCS.INC                                            *
;*   								            *
;*   Contains Functions For Bit Manipulation                                *
;*									    *
;****************************************************************************

		if	 mompass=1
		 message "Standard Bit Manipulation Functions (C) 1993 Alfred Arnold"
		endif

;----------------------------------------------------------------------------
; some sub-functions for a start:

; delivers a mask with 'bits' bits set, starting at position 'start', used for
; masking individual bits:

mask            function start,bits,((1<<bits)-1)<<start

; the same in inverted form to clear bit groups:

invmask         function start,bits,~mask(start,bits)

; delivers the bits from 'start' to 'start'+'bits'-1 from 'x':

cutout          function x,start,bits,x&mask(start,bits)

;----------------------------------------------------------------------------
; some commonly used bit masks:

; upper byte of a 16 bit word:

hi              function x,(x>>8)&255

; the same for the lower byte:

lo              function x,x&255

; upper half of a 32 bit word:

hiword          function x,(x>>16)&65535

; the same for the lower half:

loword          function x,x&65535

; boolean functions, whether a number is odd or even::

odd             function x,(x&1)=1
even            function x,(x&1)=0

; delivers bit 'n' from 'x':

getbit          function x,n,(x>>n)&1

;----------------------------------------------------------------------------
; Shift Functions:

; Shift a word of 'size' bits by 'n' positions to the left or right:

shln            function x,size,n,(x<<n)&mask(0,size)
shrn            function x,size,n,(x>>n)&mask(0,size-n)

; Rotate a word of 'size' bits by 'n' positions to the left or right;
; the first sub-term leaves the remaining bits unchanged and may be deleted if undesired:

rotln           function x,size,n,cutout(x,size,32-size)|shln(x,size,n)|shrn(x,size,size-n)
rotrn           function x,size,n,cutout(x,size,32-size)|shrn(x,size,n)|shln(x,size,size-n)

                restore                 ; allow listing again

                endif			; bitfuncsinc

