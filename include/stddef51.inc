		ifndef  stddef51inc     ; avoid multiple inclusion
stddef51inc     equ     1

                save
		listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF51.INC                                            *
;*   								            *
;*   Holds SFR and Bit Definitions for MCS-51 Processors                    *
;* 									    *
;****************************************************************************

                if      (MOMCPUNAME<>"87C750")&&(MOMCPUNAME<>"8051")&&(MOMCPUNAME<>"8052")&&(MOMCPUNAME<>"80C320")&&(MOMCPUNAME<>"80515")&&(MOMCPUNAME<>"80517")
                 fatal  "wrong target selected: only 87C750, 8051, 8052, 80C320, 80515, or 80517 supported"
		endif


                if      MOMPASS=1
                 message "MCS-51 SFR Definitions (C) 1993 Alfred Arnold/Gabriel Jager"
		 message "including \{MOMCPU} SFRs"
		endif

;----------------------------------------------------------------------------
; first of all, the things that exist (almost) everywhere:

P0              SFRB    80h             ; I/O Ports
P1		SFRB	90h
P2              SFRB    0a0h
P3		SFRB	0b0h
RD		BIT	P3.7		; Port 3: Write Line
WR		BIT	P3.6		;         Read Line
T1              BIT     P3.5            ;         Test Line 1
T0		BIT	P3.4		;         Test Line 0
INT1		BIT	P3.3		;         External Interrupt 1
INT0		BIT	P3.2		;	  External Interrupt 0
TXD		BIT	P3.1		;	  Serial Output
RXD		BIT	P3.0		;	  Serial Input
                if      MOMCPU=80C320H
TXD1		 BIT	P1.3		; zweiter Serial Output
RXD1		 BIT	P1.2		; zweiter Serial Input
		endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SP		SFR	81h		; Stack Pointer
DPL		SFR	82h		; Data Pointer Bits 0..7
DPH		SFR	83h		;      "       Bits 8..15
                if      MOMCPU=80C320H
DPL0		 SFR	DPL
DPH0		 SFR	DPH
DPL1		 SFR	84h		; Second Data Pointer 80C320
DPH1		 SFR	DPL1+1
DPS		 SFR	86h		; Bit 0 = Select DPTR0<-->DPTR1
		endif
PSW		SFRB	0d0h		; Processor Status Word
CY		BIT	PSW.7
AC		BIT	PSW.6
F0		BIT	PSW.5
RS1		BIT	PSW.4
RS0		BIT	PSW.3
OV		BIT	PSW.2
P		BIT	PSW.0
ACC		SFRB	0e0h		; Accumulator
B		SFRB	0f0h		; Auxiliary Accumulator for MUL/DIV

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

SCON		SFRB	98h		; Serial Interface: Control Register
SM0		BIT     SCON.7          ; Operating Modes
SM1		BIT     SCON.6
SM2		BIT     SCON.5
REN		BIT	SCON.4		; Receiver Enable
TB8		BIT	SCON.3		; 9th Bit to be sent
RB8		BIT	SCON.2		; 9th received bit
TI		BIT	SCON.1		; Transmit Interrupt Flag
RI		BIT	SCON.0		; Receive Interrupt Flag
SBUF		SFR	99h		; Data Register

                if      MOMCPU=80C320H  ; Registers for Second Serial Port
SCON0		 SFR	SCON
SM0_0		 BIT	SCON0.7
SM1_0		 BIT    SCON0.6
SM2_0 		 BIT    SCON0.5
REN_0		 BIT	SCON0.4
TB8_0		 BIT	SCON0.3
RB8_0		 BIT	SCON0.2
TI_0		 BIT	SCON0.1
RI_0		 BIT	SCON0.0
SBUF0		 SFR    SBUF
SCON1		 SFR	0c0h		; Control Register
SM0_1		 BIT	SCON1.7
SM1_1		 BIT    SCON1.6
SM2_1 		 BIT    SCON1.5
REN_1		 BIT	SCON1.4
TB8_1		 BIT	SCON1.3
RB8_1		 BIT	SCON1.2
TI_1		 BIT	SCON1.1
RI_1		 BIT	SCON1.0
SBUF1            SFR    0c1h            ; Data Register
		endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

PCON		SFR	87h		;  Power Management

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

TCON		SFRB	88h		; Timer 0/1 Control Register
TF1		BIT	TCON.7		; Timer 1 Overflow
TR1		BIT	TCON.6		; Timer 1 Start
TF0		BIT	TCON.5		; Timer 0 Overflow
TR0		BIT	TCON.4		; Timer 0 Start
IE1		BIT	TCON.3		; External Interrupt 1 Flag
IT1		BIT	TCON.2		; External Interrupt 1 Edge Select
IE0		BIT	TCON.1		; External Interrupt 0 Flag
IT0		BIT	TCON.0		; External Interrupt 0 Edge Select
TMOD		SFR 	89h		; Timer 0/1 Operating Modes Register
TL0		SFR	8ah		; Timer 0 Data
TL1		SFR	8bh
TH0		SFR	8ch		; Timer 1 Data
TH1		SFR	8dh

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; No Timer 2 on 8051

		if	MOMCPU<>8051h
T2CON	 	 SFRB	0c8h		; Timer 2 Control Register
TL2	 	 SFR	0cch		; Timer 2 Data
TH2	 	 SFR	0cdh
		 if	(MOMCPU=8052h)||(MOMCPU=80C320h)
RCAP2L	 	  SFR	0cah		; Capture Register
RCAP2H	 	  SFR	0cbh
TF2		  BIT	T2CON.7		; Timer 2 Overflow
EXF2		  BIT	T2CON.6		; Reload Occured
RCLK		  BIT	T2CON.5		; Timer 2 Delivers RxD-Takt
TCLK		  BIT	T2CON.4		; Timer 2 Delivers TxD-Takt
EXEN2		  BIT	T2CON.3		; Timer 2 External Enable
TR2		  BIT	T2CON.2		; Timer 2 Start
CT2		  BIT	T2CON.1		; Timer 2 as Counter
CPRL2		  BIT	T2CON.0		; Enable Capture
		 elseif
CRCL		  SFR   0cah		; other Names on 80515 !!
CRCH		  SFR	0cbh
T2PS		  BIT	T2CON.7
I3FR		  BIT	T2CON.6
I2FR		  BIT	T2CON.5
T2R1		  BIT	T2CON.4
T2R0		  BIT	T2CON.3
T2CM		  BIT	T2CON.2
T2I1		  BIT	T2CON.1
T2I0		  BIT	T2CON.0
		 endif
		endif

                if      MOMCPU=80C320H  ; 80C320 Clock Selection
CKCON		 SFR	8eh		; Bit 3,4,5 <--> Timer 0,1,2
		endif			; Bit 6,7 <--> Watchdog Timeout

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; 80C320 Watchdog

                if      MOMCPU=80C320h
WDCON            SFRB   0d8h
RWT		 BIT 	WDCON.0		; Watchdog Reset
EWT		 BIT	WDCON.1		; Watchdog Enable
WTRF		 BIT	WDCON.2		; Watchdog Reset Flag
WDIF		 BIT	WDCON.3		; Interrupt '512 Clocks to Reset' Flag
PFI		 BIT	WDCON.4		; Power Fail Interrupt Flag
EPFI		 BIT	WDCON.5		; Power Fail Interrupt Enable
POR		 BIT	WDCON.6
WD_SMOD		 BIT	WDCON.7
TA		 SFR	0c7h		; write AA 55 to get access to special
		endif			; registers

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;
                          
                if      MOMCPU=80C320H
SADDR0           SFR    0a9h            ; Serial Port 0 Slave Address
SADDR1           SFR    0aah            ; Serial Port 1 Slave Address
SADEN0           SFR    0b9h            ; Enable Bits in SADDR0
SADEN1           SFR    0bah            ; Enable Bits in SADDR1
		endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Additional 80515/80517 Register

                if      (MOMCPU=80515h)||(MOMCPU=80517h)
P4		 SFRB	0e8h
P5		 SFRB   0f8h
P6               SFR    0DBh

CCEN		 SFR	0c1h
CCH3		 SFR	0c7h
CCL3		 SFR	0c6h
CCH2		 SFR	0c5h
CCL2		 SFR	0c4h
CCH1		 SFR	0c3h
CCL1		 SFR	0c2h

ADCON            SFRB   0d8h            ; Other Names on 80515/80517
BD		 BIT	ADCON.7
CLK		 BIT    ADCON.6
BSY		 BIT	ADCON.4
ADM		 BIT	ADCON.3
MX2		 BIT	ADCON.2
MX1		 BIT	ADCON.1
MX0		 BIT	ADCON.0
ADDAT		 SFR	0d9h
DAPR             SFR    0dah
		endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Additional 80517-Register

               if      MOMCPU=80517h

DPSEL            SFR    92h             ; Data Pointer Select
ADCON1           SFR    0dch            ; A/D Converter Control 1
CTCON            SFR    0e1h            ; Compare Timer Control
IEN2             SFR    9ah
ARCON            SFR    0efh
MD0              SFR    0e9h            ;]      Multiplication
MD1              SFR    0eah            ;]      and
MD2              SFR    0ebh            ;]      Division Registers
MD3              SFR    0ech            ;]
MD4              SFR    0edh            ;]      1 - 5
MD5              SFR    0eeh            ;]
CC4EN            SFR    0c9h
CCH4             SFR    0cfh
CCL4             SFR    0ceh
CMEN             SFR    0f6h
CMH0             SFR    0d3h
CMH1             SFR    0d5h
CMH2             SFR    0d7h
CMH3             SFR    0e3h
CMH4             SFR    0e5h
CMH5             SFR    0e7h
CMH6             SFR    0f3h
CMH7             SFR    0f5h
CML0             SFR    0d2h
CML1             SFR    0d4h
CML2             SFR    0d6h
CML3             SFR    0e2h
CML4             SFR    0e4h
CML5             SFR    0e6h
CML6             SFR    0f8h
CML7             SFR    0f4h
CMSEL            SFR    0f7h
CTRELH           SFR    0dfh
CTRELL           SFR    0deh
P6               SFR    0fah            ; ??? Bit Addressable
P7               SFR    0dbh            ; ??? Bit Addressable
P8               SFR    0ddH            ; ??? Bit Addressable
ADCON0           SFR    0d8h            ; A/D Converter Control 0
S0BUF            SFR    99h             ;]      Control
S0CON            SFR    98h             ;]      of
S1BUF            SFR    9ch             ;]      Serial
S1CON            SFR    9bh             ;]      Interfaces
S1REL            SFR    9dh             ;]      0 and 0
WDTREL           SFR    86h             ;]

                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Interrupt Control Register: not compatible between 8051/52 and 80515/80517 !!!

                if      (MOMCPU=80515h)||(MOMCPU=80517h)
IEN0		 SFRB	0a8h
EAL              BIT    IEN0.7
WDT		 BIT	IEN0.6
ET2		 BIT	IEN0.5
ES		 BIT	IEN0.4
ET1		 BIT	IEN0.3
EX1		 BIT	IEN0.2
ET0		 BIT	IEN0.1
EX0		 BIT	IEN0.0
IEN1		 SFRB	0b8h
EXEN2		 BIT    IEN1.7
SWDT		 BIT	IEN1.6
EX6		 BIT	IEN1.5
EX5		 BIT	IEN1.4
EX4		 BIT	IEN1.3
EX3		 BIT	IEN1.2
EX2		 BIT	IEN1.1
EADC		 BIT	IEN1.0
IP0              SFR    0a9h
IP1              SFR    0b9h
IRCON		 SFRB	0c0h
EXF2             BIT    IRCON.7
TF2              BIT    IRCON.6
IEX6             BIT    IRCON.5
IEX5             BIT    IRCON.4
IEX4             BIT    IRCON.3
IEX3             BIT    IRCON.2
IEX2             BIT    IRCON.1
IADC             BIT    IRCON.0
		elseif
IE		 SFRB	0a8h		; Interrupt Enable
IEC              SFRB   IE
EA		 BIT	IE.7		; Global Interrupt Enable
ES		 BIT    IE.4		; Serial Interface Interrupts Enable
ET1		 BIT	IE.3		; Timer 1 Interrupt Enable
EX1		 BIT	IE.2		; External Interrupt 1 Enable
ET0		 BIT	IE.1		; Timer 0 Interrupt Enable
EX0		 BIT	IE.0		; External Interrupt 0 Enable
IP		 SFRB	0b8h		; Interrupt Priorities
IPC              SFRB   IP
PS		 BIT	IP.4		; Serial Interrupt Priority
PT1		 BIT	IP.3		; Timer 1 Interrupt Priority
PX1		 BIT	IP.2		; External Interrupt 1 Priority
PT0		 BIT	IP.1		; Timer 0 Interrupt Priority
PX0		 BIT	IP.0		; External Interrupt 0 Priority
                 if     MOMCPU=8052h
ET2		  BIT	IE.5		; Timer 2 Interrupt Enable
PT2		  BIT	IP.5		; Timer 2 Interrupt Priority
		 endif
		endif

                if      MOMCPU=80C320H  ; 80C320 Extended Interrupts
EIE              SFRB   0e8h
EWDI		 BIT 	EIE.4		; Watchdog Interrupt Enable
EX5		 BIT	EIE.3		; External Interrupts 2..5 Enable
EX4		 BIT	EIE.2
EX3		 BIT	EIE.1
EX2		 BIT	EIE.0
EIP              SFRB   0f8h
PWDI		 BIT 	EIP.4		; Watchdog-Interrupt Priority
PX5		 BIT	EIP.3		; External Interrupts 2..5 Priority
PX4		 BIT	EIP.2
PX3		 BIT	EIP.1
PX2		 BIT	EIP.0
EXIF		 SFR	91h		; Extended Interrupt Flag Register
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

                ifdef	RegUsage	; Book-Keeping about Used Banks
RegUsage	 set	RegUsage|(2^bank)
		elseif
RegUsage	 set	2^bank
		endif

ar0		set	bank*8		; Set Symbols
ar1		set	ar0+1
ar2		set	ar0+2
ar3		set	ar0+3
ar4		set	ar0+4
ar5		set	ar0+5
ar6		set	ar0+6
ar7		set	ar0+7
		endm

                restore                 ; re-allow listing

                endif			; stddef51inc
