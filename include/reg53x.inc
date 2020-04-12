		ifndef  reg53xinc       ; avoid multiple inclusion
reg53xinc       equ     1

                save
                listing off   		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG53x.INC                                              *
;*   								            *
;*   Contains Register and Address Definitions for H8/53x                   *
;*                                                                          *
;****************************************************************************

                if      (MOMCPUNAME<>"HD6475328")&&(MOMCPUNAME<>"HD6475348")&&(MOMCPUNAME<>"HD6475368")&&(MOMCPUNAME<>"HD6475388")
                 fatal  "wrong target selected: only HD6475328, HD6475348, HD6475368, or HD6475388 supported"
		endif


                if      MOMPASS=1
		 message "H8/53x Definitions (C) 1995 Alfred Arnold"
		endif

__cpunum        equ     MOMCPU-$6475000 ; more comfortable this way...

		if	__cpunum=$328	; For H8/532, register base moves
__regbase	 equ	$ff00		; one page upward
                elseif
__regbase	 equ	$fe00
		endif

;----------------------------------------------------------------------------
; Adressen:

		if 	__cpunum=$328
IRAM		 equ	$fb80		; Internal RAM Start (1 kByte)
IRAMEND		 equ	$ff7f		; Internal RAM End
                elseif
IRAM		 equ	$f680		; Internal RAM Start (2 kByte)
IRAMEND		 equ	$fe7f		; Internal RAM End
		endif

IROM		equ	$0000		; Internal ROM Start
                if	__cpunum=$368
IROMEND		 equ	$f67f		; Internal ROM End (62 kByte)
                elseif
IROMEND		 equ	$7fff		; Internal ROM End (32 kByte)
		endif


;----------------------------------------------------------------------------
; CPU Configuration

		if	__cpunum=$328
__sysbase	 equ	$fff8
                elseif
__sysbase	 equ	$ff10
		endif

WCR		equ	__sysbase+0	; Wait State Control Register
WC0		equ	0		; Number of Wait Cycles (rw)
WC1		equ	1
WMS0		equ	2		; Wait Mode (rw)
WMS1		equ	3

RAMCR		equ	__sysbase+1	; RAM Control Register
RAME		equ	7		; Internal RAM Enable

MDCR		equ	__sysbase+2	; Mode Control Register
MDS0		equ	0		; Mode Select (r)
MDS1		equ	1
MDS2		equ	2

SBYCR		equ	__sysbase+3	; Software Standby Control Register
SSBY		equ	7		; Sleep/Standby Mode Select (rw)

;----------------------------------------------------------------------------
; Interrupt Control

		if	__cpunum=$328
IPRA		 equ	$fff0		; IRQ1 / IRQ0 Interrupt Priority (rw)
IPRB		 equ	$fff1		; FRT2 / FRT1 Interrupt Priority (rw)
IPRC		 equ	$fff2		; 8 Bit Timer Interrupt Priority / FRT3 (rw)
IPRD		 equ	$fff3		; AD Converter / SCI Interrupt Priority (rw)
                elseif
IPRA		 equ	$ff00		; IRQ1 / IRQ0|I.-Timer Interrupt Priority (rw)
IPRB		 equ	$ff01		; IRQ5|IRQ4 / IRQ3|IRQ2 Interrupt Priority (rw)
IPRC		 equ	$ff02		; FRT2 / FRT1 Interrupt Priority (rw)
IPRD		 equ	$ff03		; 8 Bit Timer / FTR3 Interrupt Priority (rw)
IPRE		 equ	$ff04		; SCI2 / SCI1 Interrupt Priority (rw)
IPRF		 equ	$ff05		; - / AD Converter Interrupt Priority
                endif

;----------------------------------------------------------------------------
; Data Transfer Controller:

		if	__cpunum=$328
DTEA		 equ	$fff4		; Normal/DTC Interrupt Selection
DTEB		 equ	$fff5		; (Population of Nibbles analogous to IPRx)
DTEC		 equ	$fff6
DTED		 equ	$fff7
                elseif
DTEA		 equ	$ff08
DTEB		 equ	$ff09
DTEC		 equ	$ff0a
DTED		 equ	$ff0b
DTEE		 equ	$ff0c
DTEF		 equ	$ff0d
                endif

;----------------------------------------------------------------------------
; Ports:

__portbase	equ	__regbase+$80

P1DR		equ	__portbase+2	; Port 1 Data Register (rw/r)
P1DDR		equ	__portbase+0	; Port 1 Data Direction Register (w)
		if	__cpunum=$328
P1CR		 equ	$fffc		; Port 1 Control Register (rw)
                elseif
SYSCR1		 equ	$fefc		; System Control Register 1 (rw)
		endif
BRLE		equ	3		; P1CR/SYSCR1: Bus Release Modus
NMIEG		equ	4		;              Edge to trigger NMI Input
IRQ0E		equ	5		;              Use P15 as IRQ0 ?
IRQ1E		equ	6		;              Use P16 as IRQ1 ?

P2DR		equ	__portbase+3	; Port 2 Data Register (rw)
P2DDR		equ	__portbase+1	; Port 2 Data Direction Register (w)

P3DR		equ	__portbase+6	; Port 3 Data Register (rw)
P3DDR		equ	__portbase+4	; Port 3 Data Direction Register (w)

P4DR		equ	__portbase+7	; Port 4 Data Register (rw)
P4DDR		equ	__portbase+5	; Port 4 Data Direction Register (w)

P5DR		equ	__portbase+10	; Port 5 Data Register (rw)
P5DDR		equ	__portbase+8	; Port 5 Data Direction Register (w)

P6DR		equ	__portbase+11	; Port 6 Data Register (rw)
P6DDR		equ	__portbase+9	; Port 6 Data Direction Register (w)
		if	__cpunum<>$328
SYSCR2		 equ	$fefd		; System Control Register 2 (w)
P9SCI2E		 equ	0		; Use P92..P94 for SCI2
P9PWME		 equ	1		; Use P92..P94 for PWM
P6PWME		 equ	2		; Use P61..P63 as PWM Output
IRQ2E		 equ	3		; Use P60 as IRQ2 ?
IRQ3E		 equ	4		; Use P61 as IRQ3 ?
IRQ4E		 equ	5		; Use P62 as IRQ4 ?
IRQ5E		 equ	6		; Use P63 as IRQ5 ?
                endif

P7DR            equ     __portbase+14   ; Port 7 Data Register (rw)
P7DDR		equ	__portbase+12	; Port 7 Data Direction Register (w)

P8DR		equ	__portbase+15	; Port 8 Data Register (r)

                if      __cpunum=$388
P9DR             equ    __portbase+18   ; Port 9 Data Register (r)

PADR             equ    __portbase+19   ; Port A Data Register (rw)
PADDR            equ    __portbase+17   ; Port A Data Direction Register (w)

PBDR             equ    __portbase+22   ; Port B Data Register (rw)
PBDDR            equ    __portbase+20   ; Port B Data Direction Register (w)
PBPCR            equ    __portbase+24   ; Port B Pullup Register (w)

PCDR             equ    __portbase+23   ; Port C Data Register (rw)
PCDDR            equ    __portbase+21   ; Port C Data Direction Register (w)
PCPCR            equ    __portbase+25   ; Port C Pullup Register (w)
                elseif  __cpunum=$328
P9DR             equ    __portbase+$7f  ; Port 9 Data Register (rw)
P9DDR            equ    __portbase+$7e  ; Port 9 Data Direction Register (w)
                elseif
P9DR             equ    __portbase+$ff  ; Port 9 Data Register (rw)
P9DDR            equ    __portbase+$fe  ; Port 9 Data Direction Register (w)
                endif

;----------------------------------------------------------------------------
; Timer:

__deftimer      macro   Base,NAME
TCR{NAME}	equ	Base+0		; Control Register (rw)
TCSR{NAME}	equ	Base+1		; Control/Status Register (rw/r)
FRC{NAME}	equ	Base+2		; Count Register (rw, 16 Bit)
OCRA{NAME}	equ	Base+4		; Comparator A (rw, 16 Bit)
OCRB{NAME}	equ	Base+6		; Comparator B (rw, 16 Bit)
ICR{NAME}	equ	Base+8		; Input Capture Register (r, 16 Bit)
		endm

		__deftimer __regbase+$90,"1"
		__deftimer __regbase+$a0,"2"
		__deftimer __regbase+$b0,"3"

CKS0		equ	0		; TCRx: Clock Source Select (rw)
CKS1		equ	1
OEA		equ	2		;       Output Comparison Result OCRA (rw)
OEB		equ	3		;       Output Comparison Result OCRB (rw)
OVIE		equ	4		;       FRC Overflow Interrupt Enable (rw)
OCIEA		equ	5		;       OCRA Compare Match Interrupt Enable (rw)
OCIEB		equ	6		;       OCRB Compare Match Interrupt Enable (rw)
ICIE		equ	7		;       ICR Capture Interrupt Enable (rw)

CCLRA		equ	0		; TCSRx: Reset FRC upon OCRA Compare Match (rw)
IEDG		equ	1		;	 Input Capture Edge Select (rw)
OLVLA		equ	2		;	 OCRA Output Signal Polarity Select (rw)
OLVLB		equ	3		;	 OCRB Output Signal Polarity Select (rw)
OVF		equ	4		;	 FRC Overflow Flag (r)
OCFA		equ	5		;        FRC=OCRA ? (r)
OCFB		equ	6		;        FRC=OCRB ? (r)
ICF		equ	7		;        Input Capture Flag (r)

; 8-Bit-Timer:

__tcntbase	equ	__regbase+$d0

TCR4		equ	__tcntbase+0	; Control Register (rw)
CKS2		equ	2		; Clock Select (Remainder identical to FRCx)
CCLR0		equ	3		; Reset Mode
CCLR1		equ	4
OVIE_4		equ	5		; Overflow Interrupt Enable
CMIEA		equ	6		; TCNT=TCORA Interrupt Enable
CMIEB		equ	7		; TCNT=TCORB Interrupt Enable

TCSR4		equ	__tcntbase+1	; Control/Status Register (rw/r)
OS0		equ	0		; TCORA=TCNT -> TMO
OS1		equ	1
OS2		equ	2		; TCORB=TCNT -> TMO
OS3		equ	3
OVF_4		equ	5		; Overflow Flag
CMFA		equ	6		; TCNT=TCORA Flag
CMFB		equ	7		; TCNT=TCORB Flag

TCORA		equ	__tcntbase+2	; Time Constant 1 (rw)
TCORB		equ	__tcntbase+3	; Time Constant 2 (rw)
TCNT		equ	__tcntbase+4	; Count Value (rw)

;----------------------------------------------------------------------------
; Puls Width Modulators:

__defpwm        macro   Base,NAME
PW{NAME}_TCR	equ	Base		; Control Register (rw)
PW{NAME}_DTR	equ	Base+1		; Pulse Width Register (rw)
PW{NAME}_TCNT	equ	Base+2		; Count Value (rw)
		endm

		__defpwm __regbase+$c0,"1"
		__defpwm __regbase+$c4,"2"
		__defpwm __regbase+$c8,"3"

OS		equ	6		; PWx_TCR: Output Logic
OE		equ	7		;          Output Enable

;----------------------------------------------------------------------------
; Watchdog:

__wdtbase	equ	__regbase+$ec

WDT_TCSR_R	equ	__wdtbase+0	; Control/Status Register (different
WDT_TCSR_W	equ	__wdtbase+1	; Write/Read Addresses)
TME		equ	5		; Timer Enable
WTIT		equ	6		; Watchdog/Timer Mode
WDT_OVF		equ	7		; Watchdog Overflow

WDT_TCNT	equ	__wdtbase+1	; Count Register (rw)

		if	__cpunum<>$328
RSTCSR_W	 equ	$ff14		; Reset Register (different
RSTCSR_R	 equ	$ff15		; Write/Read Addresses)
RSTOE		 equ	6		; Output Watchdog Reset
WRST		 equ	7               ; Reset by Watchdog ?
                endif

;----------------------------------------------------------------------------
; serielle Schnittstelle:

__defsci        macro   Base,NAME
SMR{NAME}	equ	Base+0		; Mode Register (rw)
BRR{NAME}	equ	Base+1		; Bit Raten Register (rw)
SCR{NAME}	equ	Base+2		; Control Register (rw)
TDR{NAME}	equ	Base+3		; Transmit Data (rw)
SSR{NAME}	equ	Base+4		; Status Register (rw)
RDR{NAME}	equ	Base+5		; Receive Data (r)
		endm

		if	__cpunum=$328
		 __defsci __regbase+$d8,""
                elseif
		 __defsci __regbase+$d8,"1"
		 __defsci __regbase+$f0,"2"
		endif

STOP		equ	3		; SMRx: Number of Stop Bits
O_E		equ	4		;       odd/even Parity
PE		equ	5		;       With/Without Parity
CHR		equ	6		;       7/8 Data Bits
C_A		equ	7		;       synchronous/asynchronous

CKE0		equ	0		; SCRx: Output Clock if communicating synchronously
CKE1		equ	1		;       Clock internal/external (-->SCK Input or Output)
RE		equ	4		;       Receiver Enable
TE		equ	5		;       Transmitter Enable
RIE		equ	6		;       Receive Interrupt Enable
TIE		equ	7		;       Transmit Interrupt Enable

PER		equ	3		; SSRx: Parity Error
FER		equ	4		;       Framing Error
ORER		equ	5		;       Receiver Overflow
RDRF		equ     6		;       Receiver Full
TDRE		equ	7		;       Transmitter Empty

;----------------------------------------------------------------------------
; A/D Converter

__adbase	equ	__regbase+$e0

ADDRA		equ	__adbase+0	; Channel A Conversion Result (r, 16Bit)
ADDRB		equ	__adbase+2	; Channel B Conversion Result (r, 16Bit)
ADDRC		equ	__adbase+4	; Channel C Conversion Result (r, 16Bit)
ADDRD		equ	__adbase+6	; Channel D Conversion Result (r, 16Bit)

ADCSR		equ	__adbase+8	; Control/Status Register (rw)
CH0		equ	0		; Channel Selection
CH1		equ	1
CH2		equ	2
CKS		equ	3		; 274/138 Clocks Conversion Time
SCAN		equ	4		; Single Channel/Scan-Mode
ADST		equ	5		; Start/Stop Converter
ADIE		equ	6		; End of Conversion Interrupt Enable
ADF		equ	7		; End of Conversion Flag

		if	__cpunum<>$328
ADCR		 equ	__adbase+9	; Control Register (rw)
TRGE             equ    7               ; External Trigger
		endif

;----------------------------------------------------------------------------

		restore                 ; re-allow listing

		endif			; reg53xinc
