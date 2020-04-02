                ifndef  emulmspinc      ; avoid multiple inclusion
emulmspinc       equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File EMULMSP.INC                                             *
;*   					 			            *
;*   Purpose : contains emulated MSP430 instructions (obsolete)             *
;* 									    *
;****************************************************************************

                if      (MOMCPUNAME<>"MSP430")&&(MOMCPUNAME<>"MSP430X")
                 fatal  "Wrong target selected: only MSP430(X) allowed."
		endif

                if      MOMPASS=1
                 message "MSP430 Instruction Definitions (C) 1996/2007 Alfred Arnold, Jose Da Silva"
		endif

;----------------------------------------------------------------------------
; Arithmetic Instructions

adc             macro   op
                addc.attribute #0,op
                endm

dadc            macro   op
                dadd.attribute #0,op
                endm

dec             macro   op
                sub.attribute #1,op
                endm

decd            macro   op
                sub.attribute #2,op
                endm

inc             macro   op
                add.attribute #1,op
                endm

incd            macro   op
                add.attribute #2,op
                endm

sbc             macro   op
                subc.attribute #0,op
                endm

;----------------------------------------------------------------------------
; Logic Instructions

inv             macro   op
                xor.attribute #-1,op
                endm

rla             macro   op
                add.attribute op,op
                endm

rlc             macro   op
                addc.attribute op,op
                endm

;----------------------------------------------------------------------------
; Data Transfer

clr             macro   op
                mov.attribute #0,op
                endm

clrc            macro
                bic     #1,sr
                endm

clrn            macro
                bic     #4,sr
                endm

clrz            macro
                bic     #2,sr
                endm

pop             macro   op
                mov     @sp+,op
                endm

setc            macro
                bis     #1,sr
                endm

setn            macro
                bis     #4,sr
                endm

setz            macro
                bis     #2,sr
                endm

tst             macro   op
                cmp.attribute #0,op
                endm

;----------------------------------------------------------------------------
; Branching and Jumping

br              macro   op
                mov     op,pc
                endm

dint            macro
                bic     #8,sr
                endm

eint            macro
                bis     #8,sr
                endm

nop             macro
                .word   04303h         ; AS would reject the symbolic instruction
                endm

ret             macro
                mov     @sp+,pc
                endm

jlo		macro	label
		jnc	label
		endm

jhs		macro	label
		jc	label
		endm

jeq		macro	label
		jz	label
		endm

		restore			; re-allow listing
		
		endif			; emulmspinc
