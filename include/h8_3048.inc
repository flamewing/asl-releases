                ifndef  reg3048inc	; avoid multiple inclusion
reg3048inc	equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG3048.INC                                             *
;*   								            *
;*   Contains SFR, Macro, and Address Definitions for H8/3048               *
;* 									    *
;****************************************************************************

                if      (MOMCPUNAME<>"HD6413309")&&(MOMCPUNAME<>"H8/300H")
                 fatal  "wrong target selected: only H8/300H supported."
		endif


                if      MOMPASS=1
                 message "H8/3048 SFR Definitions, (C) 1995 Christian Stelter"
		endif

;-----------------------------------------------------------------------------
; MCU Operating Modes: (Sec.3 p.55-68 & Sec.20 p.615-628)


MDCR            equ     $fff1           ; CPU Operation Mode
SYSCR           equ     $fff2           ; Standby Mode Register
MSTCR           equ     $ff5e           ; Module Standby Control Register

; MDCR Register

MD0             equ     0
MD1             equ     1
MD2             equ     2


; SYSCR Register

SSBY            equ     7               ; Software Standby
STS2            equ     6               ; Standby Timer Select
STS1            equ     5
STS0            equ     4
UE              equ     3               ; User Bit Enable
NMIEG           equ     2               ; NMI Edge
RAME            equ     0               ; Enable Internal RAM 


; MSTCR Register

PSTOP           equ     7               ; Phi Clock Stop
MSTOP5          equ     5               ; Module Standby
MSTOP4          equ     4
MSTOP3          equ     3
MSTOP2          equ     2
MSTOP1          equ     1
MSTOP0          equ     0

;-----------------------------------------------------------------------------
; Bus Controller (Sec.6 p.107-142)


ABWCR           equ     $ffec           ; Bus Width Control Register

ASTCR           equ     $ffed           ; Access State Control Register

WCR             equ     $ffee           ; Wait Control Register
WMS0            equ     2               ; Mode
WMS1            equ     3
WC0             equ     0               ; Number of Waitstates
WC1             equ     1

WCER            equ     $ffef           ; Wait State Controller Enable Register

BRCR            equ     $fff3           ; Bus Release Control Register
A23E            equ     7               ; Address 23 Enable
A22E            equ     6               ;         22
A21E            equ     5               ;         21
BRLE            equ     0               ; Bus Release Enable


CSCR            equ     $ff5f           ; Chip Select Control Register
CS7E            equ     7               ; Chip Select 7 Enable
CS6E            equ     6
CS5E            equ     5
CS4E            equ     4

;-----------------------------------------------------------------------------
; Interrupt Controller

ISCR            equ     $fff4           ; IRQ Sense Control Register
IER             equ     $fff5           ; IRQ Enable Register
ISR             equ     $fff6           ; IRQ Status Register
IPRA            equ     $fff8           ; Priority Control
IPRB            equ     $fff9           ;


;-----------------------------------------------------------------------------
; Position of Exception and Interrupt Vectoren (Sec.4 p.69-78)

__defvec        macro   Name,Num
Name             equ     Num<<2
                endm

                __defvec Reset,0
                __defvec NMI,7
                __defvec TRAP0,8
                __defvec TRAP1,9
                __defvec TRAP2,10
                __defvec TRAP3,11
                __defvec IRQ0,12
                __defvec IRQ1,13
                __defvec IRQ2,14
                __defvec IRQ3,15
                __defvec IRQ4,16
                __defvec IRQ5,17
                __defvec WOVI,20
                __defvec CMI,21
                __defvec IMIA0,24
                __defvec IMIB0,25
                __defvec OVI0,26
                __defvec IMIA1,28
                __defvec IMIB1,29
                __defvec OVI1,30
                __defvec IMIA2,32
                __defvec IMIB2,33
                __defvec OVI2,34
                __defvec IMIA3,36
                __defvec IMIB3,37
                __defvec OVI3,38
                __defvec IMIA4,40
                __defvec IMIB4,41
                __defvec OVI4,42
                __defvec DEND0A,44
                ;__defvec DEND0A,45
                __defvec DEND1B,46
                ;__defvec DEND1B,47
                __defvec ERI0,52
                __defvec RXI0,53
                __defvec TXI0,54
                __defvec TEI0,55
                __defvec ERI1,56
                __defvec RXI1,57
                __defvec TXI1,58
                __defvec TEI1,59
                __defvec ADI,60


;-----------------------------------------------------------------------------
; DMA Controller (Sec.6 p.181-238)

DTEA            equ     $fff4           ; Enable Data Transfers
DTEB            equ     $fff5
DTEC            equ     $fff6
DTED            equ     $fff7

__defdma        macro   Base,Name
MAR{Name}AR     equ     Base            ; Memory Address Register AR
MAR{Name}ER     equ     Base+1          ; Memory Address Register AE
MAR{Name}AL     equ     Base+2          ; Memory Address Register AL
MAR{Name}AH     equ     Base+3          ; Memory Address Register AH
ETCR{Name}AH    equ     Base+4          ; Execute Transfer Count Register AH
ETCR{Name}AL    equ     Base+5          ;                                 AL
IOAR{Name}A     equ     Base+6          ; I/O Address Register A
DTCR{Name}A     equ     Base+7          ; Data Transfer Control Register A
MAR{Name}BR     equ     Base+8          ; Memory Address Register BR
MAR{Name}BE     equ     Base+9          ; Memory Address Register BE
MAR{Name}BH     equ     Base+10         ; Memory Address Register BH
MAR{Name}BL     equ     Base+11         ; Memory Address Register BL
ETCR{Name}BH    equ     Base+12         ; Execute Transfer Count Register BH
ETCR{Name}BL    equ     Base+13         ; Execute Transfer Count Register BL
IOAR{Name}B     equ     Base+14         ; I/O Address Register B
DTCR{Name}B     equ     Base+15         ; Data Transfer Control Register
                endm

                __defdma $ff20,"0"
                __defdma $ff30,"1"

; DTCR Register

; Short Address Mode
DTE             equ     7               ; Data Transfer Enable
DTSZ            equ     6               ; Data Transfer Size
DTID            equ     5               ; Data Transfer Inc/Dec
RPE             equ     4               ; Repeat Enable 
DTIE            equ     3               ; Data Transfer Interrupt Enable
DTS2            equ     2               ; Data Transfer Select
DTS1            equ     1
DTS0            equ     0

; Full Address Mode
SAID            equ     5               ; Source Address Inc/Dec
SAIE            equ     4               ; Source Address Inc/Dec Enable
DTS2A           equ     2               ; Data Transfer Select
DTS1A           equ     1
DTS0A           equ     0

; DTCRB Register
DTME            equ     7               ; Data Transfer Master Enable
DAID            equ     5               ; Destination Address Inc/Dec Bit
DAIE            equ     4               ;                             Enable
TMS             equ     3               ; Transfer Mode Select
DTS2B           equ     2               ; Data Transfer Select
DTS1B           equ     1
DTS0B           equ     0

;-----------------------------------------------------------------------------
; I/O-Ports (Sec.9 p.239-280)

P1DDR           equ     $ffc0           ; Data Direction Port 1
P1DR            equ     $ffc2           ; Data Port 1

P2DDR           equ     $ffc1           ; Data Direction Port 2
P2DR            equ     $ffc3           ; Data Port 2
P2PCR           equ     $ffd8           ; Input Pull Up Control Register Port 3

P3DDR           equ     $ffc4           ; Data Direction Port 3
P3DR            equ     $ffc6           ; Data Port 3

P4DDR           equ     $ffc5           ; Data Direction Port 4
P4DR            equ     $ffc7           ; Data Port 4
P4PCR           equ     $ffda           ; Input Pull Up Control Register Port 4

P5DDR           equ     $ffc8           ; Data Direction Port 5
P5DR            equ     $ffca           ; Data Port 5
P5PCR           equ     $ffcb           ; Input Pull Up Control Register Port 5

P6DDR           equ     $ffc9           ; Data Direction Port 6
P6DR            equ     $ffcb           ; Data Port 6

P8DDR           equ     $ffcd           ; Data Direction Port 8
P8DR            equ     $ffcf           ; Data Port 8

P9DDR           equ     $ffd0           ; Data Direction Port 9
P9DR            equ     $ffd2           ; Data Port 9

PADDR           equ     $ffd1           ; Data Direction Port A
PADR            equ     $ffd3           ; Data Port A

PBDDR           equ     $ffd4           ; Data Direction Port B
PBDR            equ     $ffd6           ; Data Port B

;------------------------------------------------------------------------------
;Integrated Timer Unit (ITU) (Sec.10 p.281-380)

;common
TSTR            equ     $ff60           ; Timer Start Register
TSNC            equ     $ff61           ; Timer Synchro Register
TMDR            equ     $ff62           ; Timer Mode Register
TFCR            equ     $ff63           ; Timer Function Control Register
TOER            equ     $ff90           ; Timer Output Master Enable Register
TOCR            equ     $ff91           ; Timer Output Control Register

__deftimer      macro   Base,Name
TCR{Name}       equ     Base            ; Timer Control Register
TIOR{Name}      equ     Base+1          ; Timer I/O Control Register
TIER{Name}      equ     Base+2          ; Timer Interrupt Enable Register
TSR{Name}       equ     Base+3          ; Timer Status Register
TCNT{Name}H     equ     Base+4          ; Timer Counter H
TCNT{Name}L     equ     Base+5          ; Timer Counter L
GRA{Name}H      equ     Base+6          ; General Register A (high)
GRA{Name}L      equ     Base+7          ; General Register A (low)
GRB{Name}H      equ     Base+8          ; General Register B (high)
GRB{Name}L      equ     Base+9          ; General Register B (low)
                endm

                __deftimer $ff64,"0"
                __deftimer $ff6e,"1"
                __deftimer $ff78,"2"
                __deftimer $ff82,"3"

BRA3H           equ     $ff8c           ; Buffer Register A3 (high)
BRA3L           equ     $ff8d           ; Buffer Register A3 (low)
BRB3H           equ     $ff8e           ; Buffer Register B3 (high)
BRB3L           equ     $ff8f           ; Buffer Register B3 (low)

                __deftimer $ff82,"4"

BRA4H           equ     $ff9c           ; Buffer Register A4 (high)
BRA4L           equ     $ff9d           ; Buffer Register A4 (low)
BRB4H           equ     $ff9e           ; Buffer Register B4 (high)
BRB4L           equ     $ff9f           ; Buffer Register B4 (low)

; TMDR Register

MDF             equ     6               ; Phase Counting Mode Flag
FDIR            equ     5               ; Flag Direction
PWM4            equ     4               ; PWM Mode
PWM3            equ     3
PWM2            equ     2
PWM1            equ     1
PWM0            equ     0

; TFCR Register

CMD1            equ     5               ; Combination Mode
CMD0            equ     4
BFB4            equ     3               ; Buffer Mode B4
BFA4            equ     2               ; Buffer Mode A4
BFB3            equ     1               ; Buffer Mode B3
BFA3            equ     0               ; Buffer Mode A3

; TOER Register

EXB4            equ     5               ; Master Enable TOCXB4
EXA4            equ     4               ; Master Enable TOCXA4
EB3             equ     3               ; Master Enable TIOCB3
EB4             equ     2               ; Master Enable TIOCB4
EA4             equ     1               ; Master Enable TIOCA4
EA3             equ     0               ; Master Enable TIOCA3

; TOCR Register

XTGD            equ     4               ; External trigger disable
OLS4            equ     1               ; Output level select 4
OLS3            equ     0               ; Output level select 3

; TCR Register

CCLR1           equ     6               ; Counter Clear
CCLR0           equ     5
CKEG1           equ     4               ; Counter Edge
CKEG0           equ     3
TPSC2           equ     2               ; Timer Prescaler
TPSC1           equ     1
TPSC0           equ     0


; TIOR Register

IOB2            equ     6               ; I/O Control B2
IOB1            equ     5               ; I/O Control B1
IOB0            equ     4               ; I/O Control B0
IOA2            equ     2               ; I/O Control A2
IOA1            equ     1               ; I/O Control A1
IOA0            equ     0               ; I/O Control A0


; TSR-Register

OVF             equ     2               ; Overflow Flag
IMFB            equ     1               ; Input Capture / Compare Match Flag B
IMFA            equ     0               ; Input Capture / Compare Match Flag A


; TIER-Register

OVIE            equ     2               ; Overflow Interrupt Enable
IMIEB           equ     1               ; Input Capture / Compare Match Interrupt Enable B
IMIEA           equ     0               ; Input Capture / Compare Match Interrupt Enable A

;-----------------------------------------------------------------------------
; Programmable Timing Pattern Controller (Sec.11 p.381-406)

TPMR            equ     $ffa0           ; TPC Output Mode Register
TPCR            equ     $ffa1           ; TPC Output Control Register
NDERB           equ     $ffa2           ; Next Data Enable Register B
NDERA           equ     $ffa3           ; Next Data Enable Register A
NDRA            equ     $ffa5           ; Next Data Register A
NDRB            equ     $ffa4           ; Next Data Register B
NDRA1           equ     $ffa5           ; Next Data Register A Group 1
NDRA0           equ     $ffa7           ; Next Data Register A Group 0
NDRB3           equ     $ffa4           ; Next Data Register B Group 3
NDRB2           equ     $ffa6           ; Next Data Register B Group 2

;-----------------------------------------------------------------------------
; Watchdog (Sec.12 p.407-422)

WDT_TCSR        equ     $ffa8           ; Timer Control/Status Register
WDT_TCNT        equ     $ffa9           ; Timer Counter
WDT_RSTCSR      equ     $ffab           ; Reset Control/Status Register
WDT_RSTCSRW     equ     $ffaa           ; ditto, for word accesses (p.415)


; TCSR Register

WDT_OVF         equ     7               ; Overflow Flag
WDT_WTIT        equ     6               ; Timer Mode Select
WDT_TME         equ     5               ; Timer Enable
WDT_CKS2        equ     2               ; Clock Select
WDT_CKS1        equ     1
WDT_CKS0        equ     0


; RSTCSR Register

WDT_WRST        equ     7               ; Watchdog Timer Reset
WDT_RSTOE       equ     6               ; Reset Output Enable


;-----------------------------------------------------------------------------
; Serial Interface (Sec.13 p.423-482)

__defSCI        macro   Base,Name
SMR{Name}       equ     Base            ; Serial Mode Register
BRR{Name}       equ     Base+1          ; Bit Rate Register
SCR{Name}       equ     Base+2          ; Serial Control Register
TDR{Name}       equ     Base+3          ; Transmit Data Register
SSR{Name}       equ     Base+4          ; Serial Status Register
RDR{Name}       equ     Base+5          ; Receive Data Register
                endm

                __defSCI $ffb0,"0"
                __defSCI $ffb8,"1"

; SMR Register

CA              equ     7               ; Communication Mode
CHR             equ     6               ; Character Length
PE              equ     5               ; Parity Enable
OE              equ     4               ; Parity Mode
STOP            equ     3               ; Stop Bit Length
MP              equ     2               ; Multiprocessor Mode
CKS1            equ     1               ; Clock Select 1
CKS0            equ     0


; SCR Register

TIE             equ     7               ; Transmit Interrupt Enable
RIE             equ     6               ; Receive      "        "
TE              equ     5               ; Transmit Enable
RE              equ     4               ; Receive Enable
MPIE            equ     3               ; Multiprozessor Interrupt Enable
TEIE            equ     2               ; Transmit-end Interrupt Enable
CKE1            equ     1               ; Clock Enable 1
CKE0            equ     0               ; Clock Enable 0

; SSR Register

TDRE            equ     7               ; Transmit Data Register Empty
RDRF            equ     6               ; Receive Data Register Full
ORER            equ     5               ; Overrun Error
FER             equ     4               ; Framing Error
PER             equ     3               ; Parity Error
TEND            equ     2               ; Transmit End
MPB             equ     1               ; Multiprocessor Bit
MPBT            equ     0               ; Multiprocessor Bit Transfer

;-----------------------------------------------------------------------------
; Smart Card Interface

; not implemented yet

;-----------------------------------------------------------------------------
; A/D Converter (Sec.15 p.505-526)

ADDRA           equ     $ffe0
ADDRAH          equ     $ffe0
ADDRAL          equ     $ffe1

ADDRB           equ     $ffe2
ADDRBH          equ     $ffe2
ADDRBL          equ     $ffe3

ADDRC           equ     $ffe4
ADDRCH          equ     $ffe4
ADDRCL          equ     $ffe5


ADDRD           equ     $ffe6
ADDRDH          equ     $ffe6
ADDRDL          equ     $ffe7



ADCSR           equ     $ffe8           ; Control/Status Register:

ADF             equ     7               ; Conversion Completed
ADIE            equ     6               ; Interrupt on Conversion End?
ADST            equ     5               ; Start Conversion
SCAN            equ     4               ; Scan Mode
CKS             equ     3               ; Conversion Time
CH2             equ     2               ; Channel Selection
CH1             equ     1
CH0             equ     0


ADCR            equ     $ffe9           ; A/D Control Register

TRGE            equ     7               ; Trigger Enable


;-----------------------------------------------------------------------------
; D/A-Wandler (Sec.16 p.527-533)

DADR0           equ     $ffdc           ; D/A Data Register 0
DADR1           equ     $ffdd           ; D/A Data Register 1
DACR            equ     $ffde           ; D/A Control Register
DASTCR          equ     $ff5c           ; D/A Standby Control Register


; DACR Register

DAOE1           equ     7               ; D/A Output Enable
DAOE0           equ     6
DAE             equ     5               ; D/A Enable


; DASTCR Register

DASTE           equ     0               ; D/A Standby Enable




;-----------------------------------------------------------------------------
; Clock-Pulse Generator (Sec.19 p.607-614)

DIVCR           equ     $ff5d           ; Divison Control Register


DIV1            equ     1
DIV0            equ     0



;-----------------------------------------------------------------------------

                restore			; allow listing again

                endif			; reg3048inc
