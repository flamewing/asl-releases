                save
                listing off   ; kein Listing über diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.41 - Datei REGMSP.INC                                             *
;*   					 			            *
;*   Sinn : enthält Makro- und Registerdefinitionen für den MSP430          *
;* 									    *
;*   letzte Änderungen : 2002-01-11                                         *
;*                                                                          *
;****************************************************************************

                ifndef  regmspinc      ; verhindert Mehrfacheinbindung

regmspinc       equ     1

                if      (MOMCPUNAME<>"MSP430")
                 fatal  "Falscher Prozessortyp eingestellt: nur MSP430 erlaubt!"
		endif

                if      MOMPASS=1
                 message "MSP430-Register+Befehlsdefinitionen (C) 1996 Alfred Arnold"
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

;----------------------------------------------------------------------------
; General Memory Layout
; ----------------------
;
; 0000 - 000f : Special Function Registers
; 0010 - 00ff : 8bit Peripheral Modules
; 0100 - 01ff : 16bit Peripheral Modules
; 0200 - .... : RAM Memory
; .... - ffdf : Flash Memory
; ffe0 - ffff : Interrupt Vector Table
;
;----------------------------------------------------------------------------

;----------------------------------------------------------------------------
; Special Function Register of MSP430x1xx Family, Byte Access

IE1             equ     000h            ; Interrupt Enable
IE2             equ     001h
IFG1            equ     002h            ; Interrupt Flag
IFG2            equ     003h
ME1             equ     004h            ; Modul Enable
ME2             equ     005h

;----------------------------------------------------------------------------
; Digital I/O, Byte Access

P0IN            equ     010h            ; Leseregister (Pinzustand)
P0OUT           equ     011h            ; Schreibregister (Latches)
P0DIR           equ     012h            ; Richtungsregister
P0IFG           equ     013h            ; Interrupt-Flags
P0IES           equ     014h            ; Interrupf-Flankenwahl
P0IE            equ     015h            ; Interrupt-Freigaben

P3IN		equ	018h		; Input Register
P3OUT		equ	019h		; Output Register
P3DIR		equ	01Ah		; Direction Register
P3SEL		equ	01Bh		; Function select

P4IN		equ	01Ch		; Input Register
P4OUT		equ	01Dh		; Output Register
P4DIR		equ	01Eh		; Direction Register
P4SEL		equ	01Fh		; Function select

P1IN		equ	020h		; Input Register
P1OUT		equ	021h		; Output Register
P1DIR		equ	022h		; Direction Register
P1IFG		equ	023h		; Interrupt Flags
P1IES		equ	024h		; Interrupt Edge select
P1IE		equ	025h		; Interrupt enable
P1SEL		equ	026h		; Function select

P2IN		equ	028h		; Input Register
P2OUT		equ	029h		; Output Register
P2DIR		equ	02Ah		; Direction Register
P2IFG		equ	02Bh		; Interrupt Flags
P2IES		equ	02Ch		; Interrupt Edge select
P2IE		equ	02Dh		; Interrupt enable
P2SEL		equ	02Eh		; Function select

P5IN		equ	030h		; Input Register
P5OUT		equ	031h		; Output Register
P5DIR		equ	032h		; Direction Register
P5SEL		equ	033h		; Function select

P6IN		equ	034h		; Input Register
P6OUT		equ	035h		; Output Register
P6DIR		equ	036h		; Direction Register
P6SEL		equ	037h		; Function select

;----------------------------------------------------------------------------
; LCD-Interface

LCDCTL          equ     030h            ; Steuerung
LCD_Start       equ     031h            ; Startadresse
LCD_Stop        equ     03fh            ; Endadresse
__TMP           set     1               ; Einzeldefinitionen
                rept    9
LCD{"\{__TMP}"} equ     030h+__TMP
__TMP           set     __TMP+1
                endm
                rept    6
LCD1{"\{__TMP-10}"} equ 030h+__TMP
__TMP           set     __TMP+1
                endm
;----------------------------------------------------------------------------
; Timer

BTCTL           equ     040h            ; Basis-Steuerregister Timer 1

TCCTL           equ     042h
TCPLD           equ     043h            ; Vorladewert
TCDAT           equ     044h            ; Zählwert

BTCNT1          equ     046h            ; Zählregister
BTCNT2          equ     047h

TPCTL           equ     04Bh            ; Timer/Port Steuerregister
TPCNT1          equ     04Ch            ; Zählregister
TPCNT2          equ     04Dh
TPD             equ     04Eh            ; Datenregister
TPE             equ     04Fh            ; Freigaberegister

;----------------------------------------------------------------------------
; Taktgenerator

SCFI0           equ     050h            ; Integrator
SCFI1           equ     051h
SCFQCTL         equ     052h            ; Multiplikator Quarzfrequenz
CBCTL           equ     053h            ; Puffersteuerung

;----------------------------------------------------------------------------
; EPROM Control Register, Byte Access

EPCTL           equ     054h            ; EPROM-Steuerung

;----------------------------------------------------------------------------
; Basic Clock Registers, Byte Access

DCOCTL		equ	056h
BCSCTL1		equ	057h
BCSCTL2		equ	058h

;----------------------------------------------------------------------------
; Comparator_A Registers, Byte Access

CACTL1		equ	059h		; Comparator A control register 1
CACTL2		equ	05Ah		; Comparator A control register 2
CAPD		equ	05Bh		; Comparator A port disable

;----------------------------------------------------------------------------
; PWM

PWMCTL          equ     058h            ; Zählwert
PWMDTB          equ     059h            ; Pulsweite (Puffer)
PWMDTR          equ     05Ah            ; Pulsweite
PWMCNT          equ     05Bh            ; Steuerung

;----------------------------------------------------------------------------
; USART 0

U0CTL		equ	070h
U0TCTL		equ	071h
U0RCTL		equ	072h
U0MCTL		equ	073h
U0BR0		equ	074h
U0BR1		equ	075h
U0RXBUF		equ	076h
U0TXBUF		equ	077h

;----------------------------------------------------------------------------
; USART 1

U1CTL		equ	078h
U1TCTL		equ	079h
U1RCTL		equ	07Ah
U1MCTL		equ	07Bh
U1BR0		equ	07Ch
U1BR1		equ	07Dh
U1RXBUF		equ	07Eh
U1TXBUF		equ	07Fh

;----------------------------------------------------------------------------
; ADC12 low bytes

ADC12MCTL0	equ	080h
ADC12MCTL1	equ	081h
ADC12MCTL2	equ	082h
ADC12MCTL3	equ	083h
ADC12MCTL4	equ	084h
ADC12MCTL5	equ	085h
ADC12MCTL6	equ	086h
ADC12MCTL7	equ	087h
ADC12MCTL8	equ	088h
ADC12MCTL9	equ	089h
ADC12MCTL10	equ	08Ah
ADC12MCTL11	equ	08Bh
ADC12MCTL12	equ	08Ch
ADC12MCTL13	equ	08Dh
ADC12MCTL14	equ	08Eh
ADC12MCTL15	equ	08Fh

;----------------------------------------------------------------------------
; LCD Registers

LCDC		equ	090h
__TMP           set     1               ; Einzeldefinitionen
                rept    19
LCDmemory{"\{__TMP}"} equ     LCDC+__TMP
__TMP           set     __TMP+1
                endm

;----------------------------------------------------------------------------
; A/D-Wandler, Word Access

AIN             equ     0110h           ; Eingaberegister
AEN             equ     0112h           ; Eingabefreigaben
ACTL            equ     0114h           ; Steuerung
ADAT            equ     0118h           ; Daten

;----------------------------------------------------------------------------
; Timer_B Interrupt Vector, Word Access

TBIV		equ	011Eh

;----------------------------------------------------------------------------
; Watchdog/Timer, Word Access

WDTCTL          equ     0120h

;----------------------------------------------------------------------------
; Timer_A Interrupt Vector, Word Access

TAIV		equ	012Eh

;----------------------------------------------------------------------------
; Flash Control, Word Access

FCTL1		equ	0128h
FCTL2		equ	012Ah
FCTL3		equ	012Ch

;----------------------------------------------------------------------------
; Hardware Multiplier, Word Access

MPY		equ	0130h		; Multiply unsigned
MPYS		equ	0132h		; Multiply signed
MAC		equ	0134h		; MPY+ACC
MACS		equ	0136h		; MPYS+ACC
OP2		equ	0138h		; Second Operand
ResLo		equ	013Ah		; Result low word
ResHi		equ	013Ch		; Result high word
SumExt		equ	013Eh		; Sum extend

;----------------------------------------------------------------------------
; ADC12 high bytes, Word Access

ADC12MEM0	equ	0140h
ADC12MEM1	equ	0142h
ADC12MEM2	equ	0144h
ADC12MEM3	equ	0146h
ADC12MEM4	equ	0148h
ADC12MEM5	equ	014Ah
ADC12MEM6	equ	014Ch
ADC12MEM7	equ	014Eh
ADC12MEM8	equ	0150h
ADC12MEM9	equ	0152h
ADC12MEM10	equ	0154h
ADC12MEM11	equ	0156h
ADC12MEM12	equ	0158h
ADC12MEM13	equ	015Ah
ADC12MEM14	equ	015Ch
ADC12MEM15	equ	015Eh

;----------------------------------------------------------------------------
; Timer_A Registers, Word Access

TACTL		equ	0160h
CCTL0		equ	0162h
CCTL1		equ	0164h
CCTL2		equ	0166h
CCTL3		equ	0168h
CCTL4		equ	016Ah
TAR		equ	0170h
CCR0		equ	0172h
CCR1		equ	0174h
CCR2		equ	0176h
CCR3		equ	0178h
CCR4		equ	017Ah

;----------------------------------------------------------------------------
; Timer_B Registers, Word Access

TBCTL		equ	0180h
TBCCTL0		equ	0182h
TBCCTL1		equ	0184h
TBCCTL2		equ	0186h
TBCCTL3		equ	0188h
TBCCTL4		equ	018Ah
TBCCTL5		equ	018Ch
TBCCTL6		equ	018Eh
TBR		equ	0190h
TBCCR0		equ	0192h
TBCCR1		equ	0194h
TBCCR2		equ	0196h
TBCCR3		equ	0198h
TBCCR4		equ	019Ah
TBCCR5		equ	019Ch
TBCCR6		equ	019Eh

;----------------------------------------------------------------------------
; ADC12 Registers, Byte and Word Access

ADC12CTL0	equ	01A0h
ADC12CTL1	equ	01A2h
ADC12FG		equ	01A4h
ADC12IE		equ	01A6h

;----------------------------------------------------------------------------

                endif                   ; von IFDEF...
		restore                 ; wieder erlauben
