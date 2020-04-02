                save
		listing off   ; no listing over this file
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - Datei 80C50x.INC                                             *
;*   								            *
;*   Contains SFR and Bit Definitions for SAB C50x Processors               *
;*            (derived from STDDEF51.INC)                                   *
;* 									    *
;*   Last Changes 1997-03-26 Initial Version                                *
;*                1997-06-08 added corrections from Mr. Schmid              * 
;*                                                                          *
;****************************************************************************

                ifndef  reg50xinc       ; avoid multiple inclusion

reg50xinc       equ     1

                if      (MOMCPUNAME<>"80C501")&&(MOMCPUNAME<>"80C502")&&(MOMCPUNAME<>"80C504")
                 fatal  "wrong target: only 80C501, 80C502, or 80C504 supported"
		endif


                if      MOMPASS=1
                 message "SAB C50x SFR Definitions (C) 1993 Alfred Arnold, Steffen Schmid"
                 message "including SFRS for \{MOMCPUNAME}"
                endif

;----------------------------------------------------------------------------
; Prozessorkern

ACC             SFRB    0e0h            ; Accumulator
B               SFRB    0f0h            ; Auxiliary Accumulator for MUL/DIV
SP              SFR     81h             ; Stack Pointer
DPL             SFR     82h             ; Data Pointer Bit 0..7
DPH             SFR     83h             ;      "       Bit 8..15
PSW             SFRB    0d0h            ; Status Word
CY              BIT     PSW.7
AC              BIT     PSW.6
F0              BIT     PSW.5
RS1             BIT     PSW.4
RS0             BIT     PSW.3
OV              BIT     PSW.2
P               BIT     PSW.0
SYSCON          SFR     0b1h            ; System Configuration
PCON            SFR     87h             ; "Power Management"
PCON1           SFRB    88h
EWPD            BIT     PCON1.7         ; Enable Wake-Up via INT0

                if      MOMCPUNAME="80C502"
XPAGE            SFR     91h            ; XRAM Control 80C502
XCON             SFR     94h
DPSEL            SFR     92h
                endif

;----------------------------------------------------------------------------
; Ports

P0              SFRB    80h             ; GPIO
P1		SFRB	90h
P2              SFRB    0a0h
P3		SFRB	0b0h
T2EX            BIT     P1.1
T2              BIT     P1.0
                if      MOMCPUNAME="80C504"
P1ANA            SFRB   90h             ; Select Port 1/3 as analog inputs
P3ANA            SFRB   0b0h
EAN0             BIT    P1ANA.0
EAN1             BIT    P1ANA.1
EAN2             BIT    P1ANA.2
EAN3             BIT    P1ANA.3
EAN4             BIT    P1ANA.2
EAN5             BIT    P1ANA.3
EAN6             BIT    P1ANA.4
EAN7             BIT    P1ANA.5
                endif
RD              BIT     P3.7            ; Port 3: Write Control
WR		BIT	P3.6		;         Read Control
T1              BIT     P3.5            ;         Test Pin 1
T0		BIT	P3.4		;	  Test Pin 0
INT1		BIT	P3.3		;         External Interrupt 1
INT0		BIT	P3.2		;	  External Interrupt 0
TXD		BIT	P3.1		;	  Serial Output
RXD		BIT	P3.0		;	  Serial Input

;----------------------------------------------------------------------------
; serielles

SCON		SFRB	98h		; Serial Interface: Control Register
SM0		BIT     SCON.7          ; Operation Modes
SM1		BIT     SCON.6
SM2		BIT     SCON.5
REN		BIT	SCON.4		; Enable Receiver
TB8		BIT	SCON.3		; 9th Bit to be Sent
RB8		BIT	SCON.2		; 9th Bit Received
TI		BIT	SCON.1		; Transmit Interrupt Flag
RI		BIT	SCON.0		; Receiver Interrupt Flag
SBUF		SFR	99h		; Data Register
                if      MOMCPUNAME="80C502"
SRELL            SFR     0aah           ; Baud Rate Generator Value
SRELH            SFR     0bah
BAUD             SFRB    0d8h
BD               BIT     BAUD.7         ; Enable Baud Rate Generator
                ENDIF

;----------------------------------------------------------------------------
; Timer

TCON		SFRB	88h		; Timer 0/1 Control Register
TF1		BIT	TCON.7		; Overflow Timer 1
TR1		BIT	TCON.6		; Timer 1 Run/Stop
TF0		BIT	TCON.5		; Overflow Timer 0
TR0		BIT	TCON.4		; Timer 0 Run/Stop
IE1		BIT	TCON.3		; External Interrupt 1 Flag
IT1		BIT	TCON.2		; Edge Selection External Interrupt 1
IE0		BIT	TCON.1		; External Interrupt 0 Flag
IT0		BIT	TCON.0		; Edge Selection External Interrupt 0
TMOD		SFR 	89h		; Timer 0/1 Operation Mode Register
TL0		SFR	8ah		; Timer 0 Value
TL1		SFR	8bh
TH0		SFR	8ch		; Timer 1 Value
TH1		SFR	8dh
T2CON           SFRB    0c8h            ; Control Register Timer 2
TL2             SFR     0cch            ; Timer 2 Value
TH2             SFR     0cdh
RC2L            SFR     0cah            ; Timer 2 Capture Value
RC2H            SFR     0cbh
TF2             BIT     T2CON.7         ; Overflow Timer 2
EXF2            BIT     T2CON.6         ; Timer 2 Reload Occured
RCLK            BIT     T2CON.5         ; Timer 2 Supplies RxD Clock
TCLK            BIT     T2CON.4         ; Timer 2 Supplies TxD Clock
EXEN2           BIT     T2CON.3         ; External Enable Timer 2
TR2             BIT     T2CON.2         ; Start Timer 2
CT2             BIT     T2CON.1         ; Timer 2 Operate as Counter
CPRL2           BIT     T2CON.0         ; Timer 2 Enable Capture

;---------------------------------------------------------------------------
; Watchdog

                if      MOMCPU>=80C504h
WDCON            SFRB    0c0h           ; Watchdog Control
SWDT             BIT     WDCON.0        ; Start Watchdog
WDT              BIT     WDCON.1        ; Reset Watchdog
WDTS             BIT     WDCON.2        ; Watchdog Reset Flag
OWDS             BIT     WDCON.3        ; Oszillator Watchdog Flag
WDTREL           SFR     86h
                endif

;---------------------------------------------------------------------------
; Capture/Compare-Einheit

                if      MOMCPUNAME="80C504"
CT1CON           SFR     0e1h           ; Comparator 1 Control
CCPL             SFR     0deh           ; Comparator 1 Period
CCPH             SFR     0dfh
CT1OFL           SFR     0e6h           ; Comparator 1 Offset
CT1OFH           SFR     0e7h
CMSEL0           SFR     0e3h           ; Mode
CMSEL1           SFR     0e4h
COINI            SFR     0e2h           ; Initialize Comparator Output
TRCON            SFR     0cfh           ; Enable Trap
CCL0             SFR     0c2h           ; Compare/Capture Value 0
CCH0             SFR     0c3h
CCL1             SFR     0c4h           ; Compare/Capture Value 1
CCH1             SFR     0c5h
CCL2             SFR     0c6h           ; Compare/Capture Value 2
CCH2             SFR     0c7h
CCIR             SFR     0e5h           ; Interrupt Flags
CCIE             SFR     0d6h           ; Interrupt Masks
CT2CON           SFR     0c1h           ; Comparator 2 Control
CP2L             SFR     0d2h           ; Comparator 2 Period
CP2H             SFR     0d3h
CMP2L            SFR     0d4h           ; Timer 2 Compare/Capture Value
CMP2H            SFR     0d5h
BCON             SFR     0d7h           ; Block Communication Control
                endif

;---------------------------------------------------------------------------
; A/D-Wandler

                if      MOMCPUNAME="80C504"
ADCON0           SFRB    0d8h           ; Configuration
IADC             BIT     ADCON0.5       ; Interrupt Flag (End of Conversion)
BSY              BIT     ADCON0.4       ; Busy Flag
ADM              BIT     ADCON0.3       ; Single/Continuous Conversion
MX2              BIT     ADCON0.2       ; Channel Selection
MX1              BIT     ADCON0.1
MX0              BIT     ADCON0.0
ADCON1           SFR     0dch
ADDATH           SFR     0d9h           ; Data
ADDATL           SFR     0dah
		endif

;-------------------------------------------------------------------------
; Interruptsteuerregister:

IEN0            SFRB    0a8h            ; Interrupt Enable Bits
IE              SFRB    IEN0
EA              BIT     IEN0.7          ; Global Enable
ET2             BIT     IEN0.5          ; Timer 2
ES              BIT     IEN0.4          ; Serial Interface
ET1             BIT     IEN0.3          ; Overflow Timer 1
EX1             BIT     IEN0.2          ; External Interrupt 1
ET0             BIT     IEN0.1          ; Timer 0 Overflow
EX0             BIT     IEN0.0          ; External Interrupt 0
IP0             SFRB    0b8h            ; Priorities
IP              SFRB    IP0
PT2             BIT     IP0.5           ; see Enable Bits
PS              BIT     IP0.4
PT1             BIT     IP0.3
PX1             BIT     IP0.2
PT0             BIT     IP0.1
PX0             BIT     IP0.0
                if      MOMCPUNAME="80C504"
IEN1             SFR     0a9h
IP1              SFR     0b9h
ITCON            SFR     09ah           ; Trigger Conditions
                endif

;---------------------------------------------------------------------------
; Since the 8051 has no instructions to pus the registers, this has to be done
; via direct addressing, which requires knowledge of the currently active bank.
; The macro USING is provided for doing this.  It holds the addresses of the
; currently active registers in symbols AR0..AR7.  USING expects the bank
; number as argument.

Bank0		equ	0		; For Completeness...
Bank1		equ	1
Bank2		equ	2
Bank3		equ	3

using		macro	bank
		if	(bank<0)||(bank>3)          ; only bank 0..3 allowed
                 error  "Falsche Banknummer: \{BANK}"
                endif

                ifdef	RegUsage	; bookkeeping about used banks
RegUsage	 set	RegUsage|(2^bank)
		elseif
RegUsage	 set	2^bank
		endif

ar0		set	bank*8		; define symbols
ar1		set	ar0+1
ar2		set	ar0+2
ar3		set	ar0+3
ar4		set	ar0+4
ar5		set	ar0+5
ar6		set	ar0+6
ar7		set	ar0+7
		endm

                endif

                restore                 ; re-allow listing
