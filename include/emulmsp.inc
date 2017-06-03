                save
                listing off   ; kein Listing über diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.41 - Datei EMULMSP.INC                                            *
;*   					 			            *
;*   Purpose : contains emulated MSP430 instructions (obsolete)             *
;* 									    *
;****************************************************************************
;* $Id: emulmsp.inc,v 1.1 2017/04/15 13:18:22 alfred Exp $                    
;****************************************************************************
;* $Log: emulmsp.inc,v $
;* Revision 1.1  2017/04/15 13:18:22  alfred
;* - implement emulated MSP instructions no longer via macros
;*
;****************************************************************************

                ifndef  emulmspinc      ; verhindert Mehrfacheinbindung

emulmspinc       equ     1

                if      (MOMCPUNAME<>"MSP430")&&(MOMCPUNAME<>"MSP430X")
                 fatal  "Falscher Prozessortyp eingestellt: nur MSP430 erlaubt!"
		endif

                if      MOMPASS=1
                 message "MSP430-Befehlsdefinitionen (C) 1996/2007 Alfred Arnold, Jose Da Silva"
		endif

;----------------------------------------------------------------------------
; Arithmetik

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
; Logik

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
; Daten bewegen ;-)

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
; Sprungbefehle

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
                .word   04303h         ; den symbolischen Befehl würde AS zurückweisen
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

		endif			; von IFDEF...
		restore			; wieder erlauben
