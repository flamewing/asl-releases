                ifndef  regm16cinc      ; avoid multiple inclusion
regm16cinc      equ     1

                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGM16C.INC                                             *
;*   								            *
;*   Contains Register Definitions for the M16C                             *
;*                                                                          *
;****************************************************************************

                if      MOMPASS=1
                 switch  MOMCPUNAME
                 case    "M16C"
                  fatal   "Please be more specific ; do not use the generic processor type for this header file."
                 case    "M30600M8"
                  message "Including M30600M8 Registers"
		 case    "M30610"
		  message "Including M30610 Registers"
		 case 	 "M30620"
		  message "Including M30620 Registers"
                 elsecase
                  fatal   "Invalid processor type: only M30600M8, M30610, or M30620 allowed."
                 endcase
                endif

;----------------------------------------------------------------------------
; Required Macros

__bitreg        macro   Name,Adr,Mask
Name            equ     Adr
                irp     BIT,0,1,2,3,4,5,6,7
                if      Mask&(1<<BIT)
Name{"BIT"}     equ     Adr<<3+BIT
                endif
                endm
                endm

;----------------------------------------------------------------------------
; Processor Control

                __bitreg PM,0004h,0ffh  ; CPU Mode Register
                __bitreg CM0,0006h,0ffh ; System Clock Control 0
                __bitreg CM1,0007h,02bh ; System Clock Control 1
PRCR            equ     000ah           ; Protection Register
PRC0            equ     PRCR<<3+0       ;  Write Protection CM0/CM1
PRC1            equ     PRCR<<3+1       ;  Write Protection PM
PRC2            equ     PRCR<<3+2       ;  Write Protection PD9

;----------------------------------------------------------------------------
; Ports

P0              equ     03e0h           ; Port 0 Data Register
PD0             equ     03e2h           ; Port 0 Data Direction Register

P1              equ     03e1h           ; Port 1 Data Register
PD1             equ     03e3h           ; Port 1 Data Direction Register

P2              equ     03e4h           ; Port 2 Data Register
PD2             equ     03e6h           ; Port 2 Data Direction Register

P3              equ     03e5h           ; Port 3 Data Register
PD3             equ     03e7h           ; Port 3 Data Direction Register

P4              equ     03e8h           ; Port 4 Data Register
PD4             equ     03eah           ; Port 4 Data Direction Register

P5              equ     03e9h           ; Port 5 Data Register
PD5             equ     03ebh           ; Port 5 Data Direction Register

P6              equ     03ech           ; Port 6 Data Register
PD6             equ     03eeh           ; Port 6 Data Direction Register

P7              equ     03edh           ; Port 7 Data Register
PD7             equ     03efh           ; Port 7 Data Direction Register

P8              equ     03f0h           ; Port 8 Data Register
PD8             equ     03f2h           ; Port 8 Data Direction Register

P9              equ     03f1h           ; Port 9 Data Register
PD9             equ     03f3h           ; Port 9 Data Direction Register

P10             equ     03f4h           ; Port 10 Data Register
PD10            equ     03f6h           ; Port 10 Data Direction Register

PUR0            equ     03fch           ; Pull Up Registers
PUR1            equ     03fdh
PUR2            equ     03feh
PUR3            equ     03ffh

;----------------------------------------------------------------------------
; Interrupt Control

DM0IC           equ     004bh           ; DMA Channel 0
DM1IC           equ     004ch           ; DMA Channel 1
KUPIC           equ     004dh           ; Keyboard Input
ADIC            equ     004eh           ; A/D Converter
S0TIC           equ     0051h           ; UART0 Transmit
S0RIC           equ     0052h           ; UART0 Receive
S1TIC           equ     0053h           ; UART1 Transmit
S1RIC           equ     0054h           ; UART1 Receive
TA0IC           equ     0055h           ; Timer A0
TA1IC           equ     0056h           ; Timer A1
TA2IC           equ     0057h           ; Timer A2
TA3IC           equ     0058h           ; Timer A3
TA4IC           equ     0059h           ; Timer A4
TB0IC           equ     005ah           ; Timer B0
TB1IC           equ     005bh           ; Timer B1
TB2IC           equ     005ch           ; Timer B2
INT0IC          equ     005dh           ; External Imterrupt 0
INT1IC          equ     005eh           ; External Interrupt 1
INT2IC          equ     005fh           ; External Interrupt 2

_ILVL0          equ     0               ; Priority Level
_ILVL1          equ     1
_ILVL2          equ     2
_IR             equ     3               ; Interrupt Flags
_POL            equ     4               ; Polarity Selection (only INTxIC)
_LVS            equ     5               ; Levell/Edge Trigger (nur INTxIC)

;----------------------------------------------------------------------------
; Chip Selects

CSR             equ     0008h
CS0             equ     CSR<<3+0        ;  Chip Select Enable
CS1             equ     CSR<<3+1
CS2             equ     CSR<<3+2
CS3             equ     CSR<<3+3
CS0W            equ     CSR<<3+4        ;  With or Without Wait State
CS1W            equ     CSR<<3+5
CS2W            equ     CSR<<3+6
CS3W            equ     CSR<<3+7

;----------------------------------------------------------------------------
; Address Comparator

RMAD0           equ     0010h           ; Comparison Address 0
RMAD1           equ     0014h           ; Comparison Address 1
                __bitreg AIER,0009h,03h ; Interrupt Enable

;----------------------------------------------------------------------------
; DMA Controller

DM0SL           equ     03b8h           ; Channel 0 Request Selection
DM1SL           equ     03bah           ; Channel 1 Request Selection
_DSEL0          equ     0               ; Request Reason
_DSEL1          equ     1
_DSEL2          equ     2
_DSEL3          equ     3
_DSR            equ     7               ; Software Trigger

DM0CON          equ     002ch           ; Channel 0 Control Register
DM1CON          equ     003ch           ; Channel 1 Control Register
_DMBIT          equ     0               ; Transfer Unit (1/2 Bytes)
_DMASL          equ     1               ; Single/Block Transfer
_DMAS           equ     2               ; Request Indication
_DMAE           equ     3               ; Enable
_DSD            equ     4               ; Source Address Fixed or Auto-Increment
_DAD            equ     5               ; Destination Address Fixed or Auto-Increment

SAR0            equ     0020h           ; Channel 0 Source Address
DAR0            equ     0024h           ; Channel 0 Destination Address
TCR0            equ     0028h           ; Channel 0 Count
SAR1            equ     0030h           ; Channel 1 Source Address
DAR1            equ     0034h           ; Channel 1 Destination Address
TCR1            equ     0038h           ; Channel 1 Count

;----------------------------------------------------------------------------
; Timer

TA0MR           equ     0396h           ; Timer A0 Mode Register
TA1MR           equ     0397h           ; Timer A1 Mode Register
TA2MR           equ     0398h           ; Timer A2 Mode Register
TA3MR           equ     0399h           ; Timer A3 Mode Register
TA4MR           equ     039ah           ; Timer A4 Mode Register
TB0MR           equ     039bh           ; Timer B0 Mode Register
TB1MR           equ     039ch           ; Timer B1 Mode Register
TB2MR           equ     039dh           ; Timer B2 Mode Register
_TMOD0          equ     0               ; Mode Selection
_TMOD1          equ     1
_MR0            equ     2               ; Sub Function Selection
_MR1            equ     3
_MR2            equ     4
_MR3            equ     5
_TCK0           equ     6               ; Clock Source Selection
_TCK1           equ     7

TA0             equ     0386h           ; Timer A0 Count Register
TA1             equ     0388h           ; Timer A1 Count Register
TA2             equ     038ah           ; Timer A2 Count Register
TA3             equ     038ch           ; Timer A3 Count Register
TA4             equ     038eh           ; Timer A4 Count Register
TB0             equ     0390h           ; Timer B0 Count Register
TB1             equ     0392h           ; Timer B1 Count Register
TB2             equ     0394h           ; Timer B2 Count Register

TABSR           equ     0380h           ; Start/Stop Registers:
TA0S            equ     TABSR<<3+0      ; Timer A0
TA1S            equ     TABSR<<3+1      ; Timer A1
TA2S            equ     TABSR<<3+2      ; Timer A2
TA3S            equ     TABSR<<3+3      ; Timer A3
TA4S            equ     TABSR<<3+4      ; Timer A4
TB0S            equ     TABSR<<3+5      ; Timer B0
TB1S            equ     TABSR<<3+6      ; Timer B1
TB2S            equ     TABSR<<3+7      ; Timer B2

CPSRF           equ     0381h           ; Prescaler Register:
CPSR            equ     CPSRF<<3+7      ; Prescaler Reset

ONSF            equ     0382h           ; Monoflop Trigger Bits
TA0OS           equ     ONSF<<3+0       ; Timer A0
TA1OS           equ     ONSF<<3+1       ; Timer A1
TA2OS           equ     ONSF<<3+2       ; Timer A2
TA3OS           equ     ONSF<<3+3       ; Timer A3
TA4OS           equ     ONSF<<3+4       ; Timer A4
TA0TGL          equ     ONSF<<3+6       ; Timer A0 Trigger Selection
TA0TGH          equ     ONSF<<3+7

TRGSR           equ     0383h           ; Trigger Selection Register
TA1TGL          equ     TRGSR<<3+0      ; Timer A1 Trigger Selection
TA1TGH          equ     TRGSR<<3+1
TA2TGL          equ     TRGSR<<3+2      ; Timer A2 Trigger Selection
TA2TGH          equ     TRGSR<<3+3
TA3TGL          equ     TRGSR<<3+4      ; Timer A3 Trigger Selection
TA3TGH          equ     TRGSR<<3+5
TA4TGL          equ     TRGSR<<3+6      ; Timer A4 Trigger Selection
TA4TGH          equ     TRGSR<<3+7

UDF             equ     0384h           ; Count Directions (A) / Dual Phase Modes (A2-A4)
TA0UD           equ     UDF<<3+0        ; Timer A0
TA1UD           equ     UDF<<3+1        ; Timer A1
TA2UD           equ     UDF<<3+2        ; Timer A2
TA3UD           equ     UDF<<3+3        ; Timer A3
TA4UD           equ     UDF<<3+4        ; Timer A4
TA2P            equ     UDF<<3+5        ; Timer A2
TA3P            equ     UDF<<3+6        ; Timer A3
TA4P            equ     UDF<<3+7        ; Timer A4

;----------------------------------------------------------------------------
; Watchdog

WDTS            equ     000eh           ; Start Value
                __bitreg WDC,000fh,80h  ; Prescaler Selection / Upper Counter Bits

;----------------------------------------------------------------------------
; serielle Ports:

U0TB            equ     03a2h           ; UART0 Transmit Register
U1TB            equ     03aah           ; UART1 Transmit Register

U0RB            equ     03a6h           ; UART0 Receive Register
U1RB            equ     03aeh           ; UART1 Receive Register
_OER            equ     4               ; Overrun Error
_FER            equ     5               ; Framing Error
_PER            equ     6               ; Parity Error
_SUM            equ     7               ; Summ Error

U0BRG           equ     03a1h           ; UART0 Baud Rate Generator
U1BRG           equ     03a9h           ; UART1 Baud Rate Generator

U0MR            equ     03a0h           ; UART0 Mode Register
U1MR            equ     03a8h           ; UART1 Mode Register
_SMD0           equ     0               ; Mode
_SMD1           equ     1
_SMD2           equ     2
_CKDIR          equ     3               ; Internal/External Clock
_STPS           equ     4               ; Anzahl Stopbits
_PRY            equ     5               ; Parity Enable
_PRYE           equ     6               ; Parity Type
_SLEP           equ     7               ; Sleep-Modus

U0C0            equ     03a4h           ; UART0 Control Register 0
U1C0            equ     03ach           ; UART0 Control Register 1
_CLK0           equ     0               ; Baud Rate Generator Clock Source
_CLK1           equ     1
_CRS            equ     2               ; RTS/CTS Selection
_TXEPT          equ     3               ; Transmit Register Full
_CRD            equ     4               ; CTS/RTS Selection
_NCH            equ     5               ; TXD in Open Collector Mode
_CKPOL          equ     6               ; Clock Polarity Selection
_UFORM          equ     7               ; Bit Order

U0C1            equ     03a5h           ; UART0 Control Register 1
U1C1            equ     03adh           ; UART1 Control Register 1
_TE             equ     0               ; Enable Transmitter
_TI             equ     1               ; Transmit Buffer Empty
_RE             equ     2               ; Receiver Enable
_RI             equ     3               ; Receive Buffer Full

UCON            equ     03b0h           ; UART0+1 Control Register 2
U0IRS           equ     UCON<<3+0       ; UART0 Transmitter Interrupt Cause
U1IRS           equ     UCON<<3+1       ; UART1 Transmitter Interrupt Cause
U0RRM           equ     UCON<<3+2       ; UART0 Continuous Receive Mode
U1RRM           equ     UCON<<3+3       ; UART1 Continuous Receive Mode
CLKMD0          equ     UCON<<3+4       ; Clock Output on CLK1/CLKS1
CLKMD1          equ     UCON<<3+5
RCSP            equ     UCON<<3+6       ; RTS/CTS Common/Single

;----------------------------------------------------------------------------
; CRC Generator

CRCD            equ     03bch           ; CRC Computation Result

CRCIN           equ     03beh           ; CRC Computation Input

;----------------------------------------------------------------------------
; A/D Converter

AD0             equ     03c0h           ; Channel 0 Data Register
AD1             equ     03c2h           ; Channel 1 Data Register
AD2             equ     03c4h           ; Channel 2 Data Register
AD3             equ     03c6h           ; Channel 3 Data Register
AD4             equ     03c8h           ; Channel 4 Data Register
AD5             equ     03cah           ; Channel 5 Data Register
AD6             equ     03cch           ; Channel 6 Data Register
AD7             equ     03ceh           ; Channel 7 Data Register

ADCON0          equ     03d6h           ; A/D-Control Register 0
CH0             equ     ADCON0<<3+0     ; Channel Selection
CH1             equ     ADCON0<<3+1
CH2             equ     ADCON0<<3+2
MD0             equ     ADCON0<<3+3     ; Mode
MD1             equ     ADCON0<<3+4
TRG             equ     ADCON0<<3+5     ; Soft/Hardware Trigger
ADST            equ     ADCON0<<3+6     ; Start Conversion
CKS0            equ     ADCON0<<3+7     ; Frequeny Selection

ADCON1          equ     03d7h           ; A/D Control Register 1
SCAN0           equ     ADCON1<<3+0     ; Sweep Mode
SCAN1           equ     ADCON1<<3+1
MD2             equ     ADCON1<<3+2     ; Mode
BITS            equ     ADCON1<<3+3     ; Width Selection (8/10 Bits)
VCUT            equ     ADCON1<<3+5
OPA0            equ     ADCON1<<3+6
OPA1            equ     ADCON1<<3+7

ADCON2		equ	03d4h
SMP		equ	ADCON2<<3+0	; A/D Conversion Method Bit

;----------------------------------------------------------------------------
; D/A Converter

DA0             equ     03d8h           ; D/A Converter 0 Value
DA1             equ     03dah           ; D/A Converter 1 Value

DACON           equ     03dch           ;  D/A ConverterControl Register
DA0E            equ     DACON<<3+0      ; D/A Converter 0 Enable
DA1E            equ     DACON<<3+1      ; D/A Converter 1 Enable

;----------------------------------------------------------------------------
; Fixed Vectors

Vec_UndefInstr  equ     0fffdch         ; Undefined Instruction
Vec_Overflow    equ     0fffe0h         ; INTO + Overflow
Vec_BRK         equ     0fffe4h         ; BRK
Vec_AddrMatch   equ     0fffe8h         ; Address Match
Vec_SingleStep  equ     0fffech         ; Single Step
Vec_WDT         equ     0ffff0h         ; Watchdog
Vec_DBC         equ     0ffff4h
Vec_NMI         equ     0ffff8h         ; Non Maskable Interrupt
Vec_Reset       equ     0ffffch

;----------------------------------------------------------------------------
; Variable Vectors; here the Offsets to INTB

Vec_BRK2        equ     0               ; BRK (if fixed vector disabled)
Vec_DMA0        equ     44
Vec_DMA1        equ     48
Vec_KeyInp      equ     52
Vec_AD          equ     56
Vec_UART0T      equ     68
Vec_UART0R      equ     72
Vec_UART1T      equ     76
Vec_UART1R      equ     80
Vec_TA0         equ     84
Vec_TA1         equ     88
Vec_TA2         equ     92
Vec_TA3         equ     96
Vec_TA4         equ     100
Vec_TB0         equ     104
Vec_TB1         equ     108
Vec_TB2         equ     112
Vec_INT0        equ     116
Vec_INT1        equ     120
Vec_INT2        equ     124

;----------------------------------------------------------------------------
; Memory Areas

                switch  MOMCPUNAME
                case    "M30600M8"
IROM            equ     0f0000h         ; 64K ROM at End of Address Space
IROMEND         equ     0fffffh
IRAM            equ     000400h         ; 10K RAM behind SFRs
IRAMEND         equ     002c00h
		case	"M30610"	; 128K ROM
IROM		equ	0e0000h
IROMEND		equ	0fffffh
IRAM		equ	000400h
IRAMEND		equ	002c00h		; 10K RAM
		case	"M30620"
IROM            equ     0e0000h
IROMEND         equ     0fffffh
IRAM            equ     000400h
IRAMEND         equ     002c00h         ; 10K RAM
                endcase

;----------------------------------------------------------------------------

                endif			; regm16cinc

                restore			; re-enable listing

