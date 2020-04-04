                ifndef  reg68332inc     ; avoid multiple inclusion
reg6833xinc     equ     1

                save
                listing off             ; no listing over this file
                macexp  off             ; saves a bit of time

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - Datei REG683XX.INC                                           *
;*   								            *
;*   Contains Register Address Definitions for 68332, 68340, and 68360      *
;*                                                                          *
;****************************************************************************

                if      (MOMCPUNAME<>"68332")&&(MOMCPUNAME<>"68340")&&(MOMCPUNAME<>"68360")
                 fatal   "wrong target sleected: only 68332, 68340, or 68360 supported"
		endif


                if      MOMPASS=1
                 message "CPU32 Register Definitions (C) 1994 Alfred Arnold"
                 message "including \{MOMCPU} registers"
		endif

;-----------------------------------------------------------------------------
; The base is either $fffa00 or $7fa000, this has to be set in advance
; (or you live with the default :-) ).
; On the 68340, the base may be anywhere.
; Since the 68332 does not expose A31..A24, one could place the registers at
; $fffffa00 and use short addresses.  Anyone ever tried that?
; An alternative is to set the base to 0 befor eincluding this file, so the
; symbols may be used as offsets relative to the base.

                ifndef  SIMBase
                 if      MOMCPU=$68332
SIMBase           equ     $fffa00
                 elseif
SIMBase           equ     $000000
                 endif
                endif

;=============================================================================
; Since 68360, 68340, and 68332 differ significantly in their register set, 
; I did not bother to sort out common registers.

                switch  MOMCPUNAME

;-----------------------------------------------------------------------------

                case    "68360"

;-----------------------------------------------------------------------------

MBAR            equ     $0003ff00       ; [L] Peripherals Start Address (CPU Space!)
MBARE           equ     $0003ff04       ; [L] Disable/Enable MBAR

RegBase         equ     SIMBase+$1000   ; Register Start Address

MCR             equ     RegBase+$0000   ; [L] SIM Module Configuration

AVR             equ     RegBase+$0008   ; [B] Enable Auto Vector Interrupts
RSR             equ     RegBase+$0009   ; [B] Reset Status
CLK0CR          equ     RegBase+$000c   ; [B] Clock Output 2 & 1 Control
PLLCR           equ     RegBase+$0010   ; [W] PLL Control
CDVCR           equ     RegBase+$0014   ; [W] "Slow" Clock Control
PEPAR           equ     RegBase+$0016   ; [W] Port E I/O Pins Assignment
SYPCR           equ     RegBase+$0022   ; [B] System Monitors, Bus Timimg
SWIV            equ     RegBase+$0023   ; [B] Watchdog Interrupt Vector
PICR            equ     RegBase+$0026   ; [W] Periodic Interrupt Interrupt Level and Vector
PITR            equ     RegBase+$002a   ; [W] Periodic Interrupt  Counter Value and  Prescaler
SWSR            equ     RegBase+$002f   ; [B] Reset Watchdog
BKAR            equ     RegBase+$0030   ; [L] Breakpoint Address
BKCR            equ     RegBase+$0034   ; [L] Breakpoint Control

GMR             equ     RegBase+$0040   ; [L] Memory Controller Global Control
MSTAT           equ     RegBase+$0044   ; [W] Memory-Controller Status
BR0             equ     RegBase+$0050   ; [L] CS0 SRAM/DRAM Base
OR0             equ     RegBase+$0054   ; [L] CS0 DRAM/SRAM Options
BR1             equ     RegBase+$0060   ; [L] CS1 SRAM/DRAM Base
OR1             equ     RegBase+$0064   ; [L] CS1 DRAM/SRAM Options
BR2             equ     RegBase+$0070   ; [L] CS2 SRAM/DRAM Base
OR2             equ     RegBase+$0074   ; [L] CS2 DRAM/SRAM Options
BR3             equ     RegBase+$0080   ; [L] CS3 SRAM/DRAM Base
OR3             equ     RegBase+$0084   ; [L] CS3 DRAM/SRAM Options
BR4             equ     RegBase+$0090   ; [L] CS4 SRAM/DRAM Base
OR4             equ     RegBase+$0094   ; [L] CS4 DRAM/SRAM Options
BR5             equ     RegBase+$00a0   ; [L] CS5 SRAM/DRAM Base
OR5             equ     RegBase+$00a4   ; [L] CS5 DRAM/SRAM Options
BR6             equ     RegBase+$00b0   ; [L] CS6 SRAM/DRAM Base
OR6             equ     RegBase+$00b4   ; [L] CS6 DRAM/SRAM Options
BR7             equ     RegBase+$00c0   ; [L] CS7 SRAM/DRAM Base
OR7             equ     RegBase+$00c4   ; [L] CS7 DRAM/SRAM Options

;-----------------------------------------------------------------------------
; Communications Controller:

RAMBase         equ     SIMBase         ; [ ] RAM Base Address

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; IDMA:

IDMA1Base       equ     RAMBase+$0e70
IDMA2Base       equ     RAMBase+$0f70

ICCR            equ     RegBase+$0500   ; [W] IDMA Channels Configuration
CMR1            equ     RegBase+$0504   ; [W] IDMA1 Mode
CMR2            equ     RegBase+$0526   ; [W] IDMA2 Mode
__defidma       macro   NAME,Adr,IDMABase
SAPR{NAME}      equ     Adr             ; [L] Source Address for Memory Copy Transactions
DAPR{NAME}      equ     Adr+4           ; [L] Target Address  "    "      "       "
BCR{NAME}       equ     Adr+8           ; [L] IDMA Count Register
FCR{NAME}       equ     Adr+12          ; [B] Functions Codes
CMAR{NAME}      equ     Adr+14          ; [B] Channel Mask
CSR{NAME}       equ     Adr+16          ; [B] IDMA Channel Status
IDMA{NAME}_IBASE         equ      IDMABase+0     ; [W] Descriptor Base Address
IDMA{NAME}_IBPTR         equ      IDMABase+0     ; [W] Descriptor Pointer
IDMA{NAME}_ISTATE        equ      IDMABase+0     ; [L] Internal Status
IDMA{NAME}_ITEMP         equ      IDMABase+0     ; [L] Temporary Storage
                endm
                __defidma "1",RegBase+$508
                __defidma "2",RegBase+$528

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; SDMA:

SDSR            equ     RegBase+$051c   ; [B] SDMA Status
SDCR            equ     RegBase+$051e   ; [W] SDMA Channel Configuration
SDAR            equ     RegBase+$0520   ; [L] SDMA Address Register

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; CPIC:

CICR            equ     RegBase+$0540   ; [L] Interrupt Configuration
CIPR            equ     RegBase+$0544   ; [L] Interrupt Flags
CIMR            equ     RegBase+$0548   ; [L] Interrupt Masks
CISR            equ     RegBase+$054c   ; [L] Interrupts Pending

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; PIO:

PADIR           equ     RegBase+$0550   ; [W] Port A Data Direction Register
PAPAR           equ     RegBase+$0552   ; [W] Port A Assignment 
PAODR           equ     RegBase+$0554   ; [W] Port A Open Drain Control
PADAT           equ     RegBase+$0556   ; [W] Port A Data Register

PCDIR           equ     RegBase+$0560   ; [W] Port C Data Direction Register
PCPAR           equ     RegBase+$0562   ; [W] Port C Assignment
PCSO            equ     RegBase+$0564   ; [W] Port C Special Options
PCDAT           equ     RegBase+$0566   ; [W] Port C Data Register
PCINT           equ     RegBase+$0568   ; [W] Port C Interrupt Control

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; TIMER:

TimerBase       equ     RAMBase+$0db0

TGCR            equ     RegBase+$0560   ; [W] Timer Global Configuration
TMR1            equ     RegBase+$0590   ; [W] Timer 1 Mode
TRR1            equ     RegBase+$0594   ; [W] Timer 1 Reference Value
TCR1            equ     RegBase+$0598   ; [W] Timer 1 Capture Value
TCN1            equ     RegBase+$059c   ; [W] Timer 1 Counter Value
TER1            equ     RegBase+$05b0   ; [W] Timer 1 Event Report
TMR2            equ     RegBase+$0592
TRR2            equ     RegBase+$0596
TCR2            equ     RegBase+$059a
TCN2            equ     RegBase+$059e
TER2            equ     RegBase+$05b2
TMR3            equ     RegBase+$05a0
TRR3            equ     RegBase+$05a4
TCR3            equ     RegBase+$05a8
TCN3            equ     RegBase+$05ac
TER3            equ     RegBase+$05b4
TMR4            equ     RegBase+$05a2
TRR4            equ     RegBase+$05a6
TCR4            equ     RegBase+$05aa
TCN4            equ     RegBase+$05ae
TER4            equ     RegBase+$05b6
TIMER_TM_BASE   equ     TimerBase+$00   ; [W] Table Base Address
TIMER_TM_ptr    equ     TimerBase+$02   ; [W] Table Pointer
TIMER_R_TMR     equ     TimerBase+$04   ; [W] Mode
TIMER_R_TMV     equ     TimerBase+$06   ; [W] Valid Register
TIMER_TM_cmd    equ     TimerBase+$08   ; [L] Command Register
TIMER_TM_cnt    equ     TimerBase+$0c   ; [L] Internal Counter

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; CP:

MiscBase        equ     RAMBase+$0cb0

CR              equ     RegBase+$05c0   ; [W] Command Register
RCCR            equ     RegBase+$05c4   ; [W] RISC-Controller Configuration
RTER            equ     RegBase+$05d6   ; [W] Timer Events
RTMR            equ     RegBase+$05da   ; [W] Timer Mask
CP_REV_num      equ     MiscBase        ; [W] Microcode Revision Number

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; BRG:

BRGC1           equ     RegBase+$05f0   ; [L] Baud Rate Generator 1 Configration
BRGC2           equ     RegBase+$05f4
BRGC3           equ     RegBase+$05f8
BRGC4           equ     RegBase+$05fc

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; SCC:

SCC1Base        equ     RAMBase+$0c00
SCC2Base        equ     RAMBase+$0d00
SCC3Base        equ     RAMBase+$0e00
SCC4Base        equ     RAMBase+$0f00

__defscc        macro   NAME,Adr,SCCBase
GSMR_L{NAME}    equ     Adr+0           ; [Q] Mode
GSMR_H{NAME}    equ     Adr+4
PSMR{NAME}      equ     Adr+8           ; [W] Protocol Specific Mode
TODR{NAME}      equ     Adr+12          ; [W] Force Transmission Start
DSR{NAME}       equ     Adr+14          ; [W] SCCx Synchronisation Pattern
SCCE{NAME}      equ     Adr+16          ; [W] UART Event Register
SCCM{NAME}      equ     Adr+20          ; [W] UART Event Mask
SCCS{NAME}      equ     Adr+23          ; [B] UART Status
SCC{NAME}_RBASE           equ     SCCBase+$00     ; [W] Receive Buffer Start Address
SCC{NAME}_TBASE           equ     SCCBase+$02     ; [W] Transmit Buffer Start Address
SCC{NAME}_RFCR            equ     SCCBase+$04     ; [B] Receive Address Space
SCC{NAME}_TFCR            equ     SCCBase+$05     ; [B] Transmit Address Space
SCC{NAME}_MRBLR           equ     SCCBase+$06     ; [W] Receive Buffer Length
SCC{NAME}_RSTATE          equ     SCCBase+$08     ; [L] Receiver Status
SCC{NAME}_RBPTR           equ     SCCBase+$10     ; [W] Receive Address Pointer
SCC{NAME}_TSTATE          equ     SCCBase+$18     ; [L] Transmitter Status
SCC{NAME}_TBPTR           equ     SCCBase+$20     ; [W] Transmit Address Pointer
SCC{NAME}_RCRC            equ     SCCBase+$28     ; [L] Receive CRC
SCC{NAME}_TCRC            equ     SCCBase+$2c     ; [L] Transmit CRC
SCC{NAME}_MAX_IDL         equ     SCCBase+$38     ; [W] --UART-- Maximum Number of Idle Characters
SCC{NAME}_IDLC            equ     SCCBase+$3a     ; [W] Temporary Idle Counter
SCC{NAME}_BRKCR           equ     SCCBase+$3c     ; [W] Number of Transmit Breaks
SCC{NAME}_PAREC           equ     SCCBase+$3e     ; [W] Parity Error Counter
SCC{NAME}_FRMEC           equ     SCCBase+$40     ; [W] Framing Error Counter
SCC{NAME}_NOSEC           equ     SCCBase+$42     ; [W] Noise Counter
SCC{NAME}_BRKEC           equ     SCCBase+$44     ; [W] Break Condition
SCC{NAME}_BRKLN           equ     SCCBase+$46     ; [W] Length of most recent Break
SCC{NAME}_UADDR1          equ     SCCBase+$48     ; [W] Slave Addresse
SCC{NAME}_UADDR2          equ     SCCBase+$4a     ; [W]
SCC{NAME}_RTEMP           equ     SCCBase+$4c     ; [W] Temporary Storage
SCC{NAME}_TOSEQ           equ     SCCBase+$4e     ; [W] Out-of-Sequence Characters
SCC{NAME}_CHARACTER1      equ     SCCBase+$50     ; [W] Characters that generate Interrupts
SCC{NAME}_CHARACTER2      equ     SCCBase+$52     ; [W]
SCC{NAME}_CHARACTER3      equ     SCCBase+$54     ; [W]
SCC{NAME}_CHARACTER4      equ     SCCBase+$56     ; [W]
SCC{NAME}_CHARACTER5      equ     SCCBase+$58     ; [W]
SCC{NAME}_CHARACTER6      equ     SCCBase+$5a     ; [W]
SCC{NAME}_CHARACTER7      equ     SCCBase+$5c     ; [W]
SCC{NAME}_CHARACTER8      equ     SCCBase+$5e     ; [W]
SCC{NAME}_RCCM            equ     SCCBase+$60     ; [W] Received Characters Mask
SCC{NAME}_RCCR            equ     SCCBase+$62     ; [W] Received Character
SCC{NAME}_RLBC            equ     SCCBase+$64     ; [W] Most Recent Break Character
SCC{NAME}_C_MASK          equ     SCCBase+$34     ; [L] --HDLC-- CRC Polynom
SCC{NAME}_C_PRES          equ     SCCBase+$38     ; [L] CRC Start Value
SCC{NAME}_DISFC           equ     SCCBase+$3c     ; [W] Discarded Frames Counter
SCC{NAME}_CRCEC           equ     SCCBase+$3e     ; [W] CRC Errors Counter
SCC{NAME}_ABTSC           equ     SCCBase+$40     ; [W] Aborts Counter
SCC{NAME}_NMARC           equ     SCCBase+$42     ; [W] Non-Matching Addresses Counter
SCC{NAME}_RETRC           equ     SCCBase+$44     ; [W] Retransmissions Counter
SCC{NAME}_MFLR            equ     SCCBase+$46     ; [W] Maximal Frame Length
SCC{NAME}_MAX_cnt         equ     SCCBase+$48     ; [W] Length Counter
SCC{NAME}_RFTHR           equ     SCCBase+$4a     ; [W] Received Frames Threshold
SCC{NAME}_RFCNT           equ     SCCBase+$4c     ; [W] Received Frames Count
SCC{NAME}_HMASK           equ     SCCBase+$4e     ; [W] Address Mask
SCC{NAME}_HADDR1          equ     SCCBase+$50     ; [W] Addresses
SCC{NAME}_HADDR2          equ     SCCBase+$52     ; [W]
SCC{NAME}_HADRR3          equ     SCCBase+$54     ; [W]
SCC{NAME}_HADDR4          equ     SCCBase+$56     ; [W]
SCC{NAME}_TMP             equ     SCCBase+$58     ; [W] Temporary Storage
SCC{NAME}_TMP_MB          equ     SCCBase+$5a     ; [W]     "        "
SCC{NAME}_CRCC            equ     SCCBase+$34     ; [L] --BISYNC-- Temporary CRC Value
SCC{NAME}_PRCRC           equ     SCCBase+$38     ; [W] Receiver Preset for CRC
SCC{NAME}_PTCRC           equ     SCCBase+$3a     ; [W] Transmitter Preset for CRC
SCC{NAME}_B_PAREC         equ     SCCBase+$3c     ; [W] Receiver Parity Errors Counter
SCC{NAME}_BSYNC           equ     SCCBase+$3e     ; [W] SYNC Characters
SCC{NAME}_BDLE            equ     SCCBase+$40     ; [W] DLE Characters
SCC{NAME}_B_CHARACTER1    equ     SCCBase+$42     ; [W] Control Characters
SCC{NAME}_B_CHARACTER2    equ     SCCBase+$44     ; [W]
SCC{NAME}_B_CHARACTER3    equ     SCCBase+$46     ; [W]
SCC{NAME}_B_CHARACTER4    equ     SCCBase+$48     ; [W]
SCC{NAME}_B_CHARACTER5    equ     SCCBase+$4a     ; [W]
SCC{NAME}_B_CHARACTER6    equ     SCCBase+$4c     ; [W]
SCC{NAME}_B_CHARACTER7    equ     SCCBase+$4e     ; [W]
SCC{NAME}_B_CHARACTER8    equ     SCCBase+$50     ; [W]
SCC{NAME}_B_RCCM          equ     SCCBase+$52     ; [W] Receive Control Character Mask
SCC{NAME}_CRC_P           equ     SCCBase+$30     ; [L] --Transparent-- CRC Preset
SCC{NAME}_CRC_C           equ     SCCBase+$34     ; [L] CRC Constant
SCC{NAME}_E_C_PRES        equ     SCCBase+$30     ; [L] --Ethernet-- CRC Preset
SCC{NAME}_E_C_MASK        equ     SCCBase+$34     ; [L] CRC Mask
SCC{NAME}_E_CRCEC         equ     SCCBase+$38     ; [L] CRC Error Counter
SCC{NAME}_ALEC            equ     SCCBase+$3c     ; [L] Alignment Error Counter
SCC{NAME}_E_DISFC         equ     SCCBase+$40     ; [L] Discarded Frames Counter
SCC{NAME}_PADS            equ     SCCBase+$44     ; [W] Padding Characters for Short Frames
SCC{NAME}_RET_Lim         equ     SCCBase+$46     ; [W] Maximum Number of Retries
SCC{NAME}_RET_cnt         equ     SCCBase+$48     ; [W] Current Number of Retries
SCC{NAME}_E_MFLR          equ     SCCBase+$4a     ; [W] Maximum Frame Length
SCC{NAME}_MINFLR          equ     SCCBase+$4c     ; [W] Minimum Frame Length
SCC{NAME}_MAXD1           equ     SCCBase+$4e     ; [W] Maximal Length DMA1
SCC{NAME}_MAXD2           equ     SCCBase+$50     ; [W] Maximal Length DMA2
SCC{NAME}_MAXD            equ     SCCBase+$52     ; [W] Rx Max DMA
SCC{NAME}_DMA_cnt         equ     SCCBase+$54     ; [W] DMA Counter Reception
SCC{NAME}_MAX_b           equ     SCCBase+$56     ; [W] Maximum BD Byte Count
SCC{NAME}_GADDR1          equ     SCCBase+$58     ; [W] Group Address Filter
SCC{NAME}_GADDR2          equ     SCCBase+$5a     ; [W]
SCC{NAME}_GADDR3          equ     SCCBase+$5c     ; [W]
SCC{NAME}_GADDR4          equ     SCCBase+$5e     ; [W]
SCC{NAME}_TBUF0.data0     equ     SCCBase+$60     ; [L] Save Areas - Current Frame
SCC{NAME}_TBUF0.data1     equ     SCCBase+$64     ; [L]
SCC{NAME}_TBUF0.rba0      equ     SCCBase+$68     ; [L]
SCC{NAME}_TBUF0.crc       equ     SCCBase+$6c     ; [L]
SCC{NAME}_TBUF0.bcnt      equ     SCCBase+$70     ; [W]
SCC{NAME}_PADDR1_H        equ     SCCBase+$72     ; [W] Physical Address
SCC{NAME}_PADDR1_M        equ     SCCBase+$74     ; [W]
SCC{NAME}_PADDR1_L        equ     SCCBase+$76     ; [W]
SCC{NAME}_P_Per           equ     SCCBase+$78     ; [W] Persistence
SCC{NAME}_RFBD_ptr        equ     SCCBase+$7a     ; [W] Rx First BD Counter
SCC{NAME}_TFBD_ptr        equ     SCCBase+$7c     ; [W] Tx First BD Pointer
SCC{NAME}_TLBD_ptr        equ     SCCBase+$7e     ; [W] Tx Last BD Pointer
SCC{NAME}_TBUF1.data0     equ     SCCBase+$80     ; [L] Save Areas - Next Frame
SCC{NAME}_TBUF1.data1     equ     SCCBase+$84     ; [L]
SCC{NAME}_TBUF1.rba0      equ     SCCBase+$88     ; [L]
SCC{NAME}_TBUF1.crc       equ     SCCBase+$8c     ; [L]
SCC{NAME}_TBUF1.bcnt      equ     SCCBase+$90     ; [W]
SCC{NAME}_TX_len          equ     SCCBase+$92     ; [W] Tx Frame Length Counter
SCC{NAME}_IADDR1          equ     SCCBase+$94     ; [W] Individual Address Filters
SCC{NAME}_IADDR2          equ     SCCBase+$96     ; [W]
SCC{NAME}_IADDR3          equ     SCCBase+$98     ; [W]
SCC{NAME}_IADDR4          equ     SCCBase+$9a     ; [W]
SCC{NAME}_BOFF_CNT        equ     SCCBase+$9c     ; [W] Backoff Counter
SCC{NAME}_TADDR_H         equ     SCCBase+$9e     ; [W] Temporary Address
SCC{NAME}_TADDR_M         equ     SCCBase+$9a     ; [W]
SCC{NAME}_TADDR_L         equ     SCCBase+$a0     ; [W]
                endm
                __defscc "1",RegBase+$0600,SCC1Base
                __defscc "2",RegBase+$0620,SCC2Base
                __defscc "3",RegBase+$0640,SCC3Base
                __defscc "4",RegBase+$0660,SCC4Base

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; SMC:

SMC1Base        equ     RAMBase+$0e80
SMC2Base        equ     RAMBase+$0f80

__defsmc        macro   NAME,Adr,SMCBase
SMCMR{NAME}     equ     Adr+0           ; [W] Transparent Mode
SMCE{NAME}      equ     Adr+4           ; [B] Event Register
SMCM{NAME}      equ     Adr+8           ; [W] Mode
SMC{NAME}_RBASE           equ     SMCBase+$00     ; [W] Receive Buffer Descriptor Address
SMC{NAME}_TBASE           equ     SMCBase+$02     ; [W] Transmit Buffer Descriptor Address
SMC{NAME}_RFCR            equ     SMCBase+$04     ; [B] Receive Function Code
SMC{NAME}_TFCR            equ     SMCBase+$05     ; [B] Transmit Function Code
SMC{NAME}_MRBLR           equ     SMCBase+$06     ; [W] Maximum Length Receive Buffer
SMC{NAME}_RSTATE          equ     SMCBase+$08     ; [L] Internal Receiver Status
SMC{NAME}_RBPTR           equ     SMCBase+$10     ; [W] Rx Buffer Descriptor pointer
SMC{NAME}_TSTATE          equ     SMCBase+$18     ; [L] Internal Transmitter Status
SMC{NAME}_TBPTR           equ     SMCBase+$20     ; [W] Tx Buffer Descriptor Pointer
SMC{NAME}_MAX_IDL         equ     SMCBase+$28     ; [W] --UART-- Maximum Number Idle Characters
SMC{NAME}_IDLC            equ     SMCBase+$28     ; [W] Idle Counter
SMC{NAME}_BRKLN           equ     SMCBase+$28     ; [W] Length of last Break Character
SMC{NAME}_BRKEC           equ     SMCBase+$28     ; [W] Receive Break Condition Counter
SMC{NAME}_BRKCR           equ     SMCBase+$28     ; [W] Transmit Break Counter
SMC{NAME}_R_mask          equ     SMCBase+$28     ; [W] Temporary Bit Mask
SMC{NAME}_M_RxBD          equ     SMCBase+$00     ; [W] --GCI-- Monitor Channel Rx
SMC{NAME}_M_TxBD          equ     SMCBase+$02     ; [W] Monitor Channel Tx
SMC{NAME}_CI_RxBD         equ     SMCBase+$04     ; [W] C/I Channel Rx
SMC{NAME}_CI_TxBD         equ     SMCBase+$06     ; [W] C/I Channel Tx
SMC{NAME}_M_RxD           equ     SMCBase+$0c     ; [W] Monitor Rx Data
SMC{NAME}_M_TxD           equ     SMCBase+$0e     ; [W] Monitor Tx Data
SMC{NAME}_CI_RxD          equ     SMCBase+$10     ; [W] C/I Rx Data
SMC{NAME}_CI_TxD          equ     SMCBase+$12     ; [W] C/I Tx Data
                endm
                __defsmc "1",RegBase+$0682,SMC1Base
                __defsmc "2",RegBase+$0692,SMC2Base

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; SPI:

SPIBase         equ     RAMBase+$0d80

SPMODE          equ     RegBase+$06a0   ; [W] Mode Rregister
SPIE            equ     RegBase+$06a6   ; [B] Event Register
SPIM            equ     RegBase+$06aa   ; [B] Mask Register
SPICOM          equ     RegBase+$06ad   ; [B] Command Register
SPI_RBASE       equ     SPIBase+$00     ; [W] Receive Descriptor Address
SPI_TBASE       equ     SPIBase+$02     ; [W] Transmit Descriptor Address
SPI_RFCR        equ     SPIBase+$04     ; [B] Receive Function Code
SPI_TFCR        equ     SPIBase+$05     ; [B] Transmit Function Code
SPI_MRBLR       equ     SPIBase+$06     ; [W] Maximum Length Receive Buffer
SPI_RSTATE      equ     SPIBase+$08     ; [L] Receiver Status
SPI_RBPTR       equ     SPIBase+$10     ; [W] Currently Active Receive Descriptor
SPI_TSTATE      equ     SPIBase+$18     ; [L] Trannsmitter Status
SPI_TBPTR       equ     SPIBase+$20     ; [W] Currently Active Transmit Descriptor

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; PIP:

PIPBase         equ     SMC2Base

PIPC            equ     RegBase+$06b2   ; [W] Configuration Register
PTPR            equ     RegBase+$06b6   ; [W] Timing Parameters
PIPE            equ     SMCE2           ; [B] Event Register, overlayed!!
PBDIR           equ     RegBase+$06b8   ; [L] Port B Data Direction Register
PBPAR           equ     RegBase+$06bc   ; [L] Port B Assignment
PBODR           equ     RegBase+$06c2   ; [W] Port B Open Drain Control Bits
PBDAT           equ     RegBase+$06c4   ; [L] Port B Data Register
PIP_RBASE       equ     PIPBase+$00     ; [W] Receive Descriptor Address
PIP_TBASE       equ     PIPBase+$02     ; [W] Transmit Descriptor Address
PIP_CFCR        equ     PIPBase+$04     ; [B] Funktion Code
PIP_SMASK       equ     PIPBase+$05     ; [B] Status Mask
PIP_MRBLR       equ     PIPBase+$06     ; [W] Maximum Length of Receive Buffer
PIP_RSTATE      equ     PIPBase+$08     ; [L] Receiver Status
PIP_R_PTR       equ     PIPBase+$0c     ; [L] Internal Receive Data Pointer
PIP_RBPTR       equ     PIPBase+$10     ; [W] Current Receive Descriptor
PIP_R_CNT       equ     PIPBase+$12     ; [W] Receive Byte Counter
PIP_RTEMP       equ     PIPBase+$14     ; [L] Temporary Storage
PIP_TSTATE      equ     PIPBase+$18     ; [L] Transmitter Status
PIP_T_PTR       equ     PIPBase+$1c     ; [L] Current Transmit Data Pointer
PIP_TBPTR       equ     PIPBase+$20     ; [W] Current Transmit Data Descriptor
PIP_T_CNT       equ     PIPBase+$22     ; [W] Transmit Byte Counter
PIP_TTEMP       equ     PIPBase+$24     ; [L] Temporary Storage
PIP_MAX_SL      equ     PIPBase+$28     ; [W] Maximuma Sleep Time
PIP_SL_CNT      equ     PIPBase+$2a     ; [W] Sleep Counter
PIP_CHARACTER1  equ     PIPBase+$2c     ; [W] Control Characters
PIP_CHARACTER2  equ     PIPBase+$2e
PIP_CHARACTER3  equ     PIPBase+$30
PIP_CHARACTER4  equ     PIPBase+$32
PIP_CHARACTER5  equ     PIPBase+$34
PIP_CHARACTER6  equ     PIPBase+$36
PIP_CHARACTER7  equ     PIPBase+$38
PIP_CHARACTER8  equ     PIPBase+$3a
PIP_RCCM        equ     PIPBase+$3c     ; [W] Control Character Mask
PIP_RCCR        equ     PIPBase+$3e     ; [W] Control Character Register

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; SI:

SIMODE          equ     RegBase+$06e0   ; [L] Serial Interface Mode
SIGMR           equ     RegBase+$06e4   ; [B] Global Mode Setting
SISTR           equ     RegBase+$06e6   ; [B] Address of Router RAM
SICMR           equ     RegBase+$06e7   ; [B] Serial Interface Command Register
SICR            equ     RegBase+$06ec   ; [L] Serial Interface Clock Distribution
SIRP            equ     RegBase+$06f2   ; [L] RAM Pointer
SIRAM           equ     RegBase+$0700   ; [ ] Routing RAM

;=============================================================================

                case    "68340"

;-----------------------------------------------------------------------------
; Comments may eventually be a bit less elaborate, since Motorola's Technical
; Summary for the 68340 is not so detailed:

SIMBAR          equ     $0003ff00       ; [L] Peripheral Address Setting

MCR             equ     SIMBase+$000    ; [W] SIM Module Configuration

SYNCR           equ     SIMBase+$004    ; [W] Clock Synthesizer Control
AVR             equ     SIMBase+$006    ; [B] Auto Vectors
RSR             equ     SIMBase+$007    ; [B] Reset Status

PORTA           equ     SIMBase+$011    ; [B] Port A Data Register
DDRA            equ     SIMBase+$013    ; [B] Port A Data Direction Register
PPRA1           equ     SIMBase+$015    ; [B] Port A Pin Assignment
PPRA2           equ     SIMBase+$017    ; [B] 
PORTB           equ     SIMBase+$019    ; [B] Port B Data Register
PORTB1          equ     SIMBase+$01b    ; [B] ditto
DDRB            equ     SIMBase+$01d    ; [B] Port B Data Direction Register
PPRARB          equ     SIMBase+$01f    ; [B] Port B Pin Assignment
SWIV            equ     SIMBase+$020    ; [B] Software Vectors
SYPCR           equ     SIMBase+$021    ; [B] System Protection
PICR            equ     SIMBase+$022    ; [W] PIT Control
PITR            equ     SIMBase+$024    ; [W] PIT Data Register
SWSR            equ     SIMBase+$027    ; [B] Software Service

;-----------------------------------------------------------------------------
; Chip Selects:

__cnt           set     0
                rept    4
__name           set     "\{__CNT}"
CS{__name}AM1    set     SIMBase+$040+__cnt*8 ; [W] CSn Address Mask 1
CS{__name}AM2    set     SIMBase+$042+__cnt*8 ; [W] CSn Address Mask 2
CS{__name}BA1    set     SIMBase+$044+__cnt*8 ; [W] CSn Base Address 1
CS{__name}BA2    set     SIMBase+$046+__cnt*8 ; [W] CSn Base Address 2
__cnt            set     __cnt+1
                endm

;-----------------------------------------------------------------------------
; DMA:

DMABase         equ     SIMBase+$780
DMAMCR1         equ     DMABase+$000    ; [W] DMA Channel 1 Module Configuration
DMAINTR1        equ     DMABase+$004    ; [W] DMA Channel 1 Interrupts
DMACCR1         equ     DMABase+$008    ; [W] DMA Channel 1 Control Register
DMACSR1         equ     DMABase+$00a    ; [B] DMA Channel 1 Status Register
DMAFCR1         equ     DMABase+$00b    ; [B] DMA Channel 1 Function Code Register
DMASAR1         equ     DMABase+$00c    ; [L] DMA Channel 1 Source Address
DMADAR1         equ     DMABase+$010    ; [L] DMA Channel 1 Destination Address
DMABTC1         equ     DMABase+$014    ; [L] DMA Channel 1 Byte Counter
DMAMCR2         equ     DMABase+$020    ; ditto for Channel 2
DMAINTR2        equ     DMABase+$024
DMACCR2         equ     DMABase+$028
DMACSR2         equ     DMABase+$02a
DMAFCR2         equ     DMABase+$02b
DMASAR2         equ     DMABase+$02c
DMADAR2         equ     DMABase+$030
DMABTC2         equ     DMABase+$034

;-----------------------------------------------------------------------------
; Serial Stuff

SMBase          equ     SIMBase+$700
SMMCR           equ     SMBase+$000     ; [W] SIM Module Configuration
SMILR           equ     SMBase+$004     ; [B] Interrupt Level
SMIVR           equ     SMBase+$005     ; [B] Interrupt Vector
SMIPCR          equ     SMBase+$014     ; [BR] Pin Change Register
SMACR           equ     SMBase+$014     ; [BW] Auxiliary Control register
SMISR           equ     SMBase+$015     ; [BR] Interrupt Flags
SMIER           equ     SMBase+$015     ; [BW] Interupt Enables
SMOPCR          equ     SMBase+$01d     ; [BW] Output Ports Control
SMIP            equ     SMBase+$01d     ; [BR] Input Ports Status
SMOPS           equ     SMBase+$01e     ; [BW] Individually Set Port Bits
SMOPR           equ     SMBase+$01f     ; [BW] Individually Clear Port Bits
SMMR1A          equ     SMBase+$010     ; [B] Channel A Mode Register
SMMR2A          equ     SMBase+$020     ; [B] Channel A Mode Register
SMCSRA          equ     SMBase+$011     ; [BR] Channel A Clock Selection
SMSRA           equ     SMBase+$011     ; [BW] Channel A Status Register
SMCRA           equ     SMBase+$012     ; [BW] Channel A Command Register
SMRBA           equ     SMBase+$013     ; [BR] Channel A Receive Data Register
SMTBA           equ     SMBase+$013     ; [BW] Channel A Transmit Data Register
SMMR1B          equ     SMBase+$018     ; [B] Channel B Mode Register 1
SMMR2B          equ     SMBase+$021     ; [B] Channel B Mode Register 2
SMCSRB          equ     SMBase+$019     ; [BR] Channel B Clock Selection
SMSRB           equ     SMBase+$019     ; [BW] Channel B Status Register
SMCRB           equ     SMBase+$01a     ; [BW] Channel B Command Register
SMRBB           equ     SMBase+$01b     ; [BR] Channel B Receive Data Register
SMTBB           equ     SMBase+$01b     ; [BW] Channel B Transmit Data Register

;-----------------------------------------------------------------------------
; Timer:

TMBase          equ     SIMBase+$600
TM1MCR          equ     TMBase+$000     ; [W] Timer 1 Module Configuration
TM1IR           equ     TMBase+$004     ; [W] Timer 1 Interrupt Configuration
TM1CR           equ     TMBase+$006     ; [W] Timer 1 Control
TM1SR           equ     TMBase+$008     ; [W] Timer 1 Status/Prescaler
TM1CNTR         equ     TMBase+$00a     ; [W] Timer 1 Count Register
TM1PREL1        equ     TMBase+$00c     ; [W] Timer 1 Preset 1
TM1PREL2        equ     TMBase+$00e     ; [W] Timer 1 Preset 2
TM1COM          equ     TMBase+$010     ; [W] Timer 1 Compare Register
TM2MCR          equ     TMBase+$040     ; ditto for Timer 2
TM2IR           equ     TMBase+$044
TM2CR           equ     TMBase+$046
TM2SR           equ     TMBase+$048
TM2CNTR         equ     TMBase+$04a
TM2PREL1        equ     TMBase+$04c
TM2PREL2        equ     TMBase+$04e
TM2COM          equ     TMBase+$050

;=============================================================================
; 68332 Registers start here

                case    "68332"

;-----------------------------------------------------------------------------
; Fundamental SIM Control Registers

SIMCR           equ     SIMBase+$00     ; [W] MCU Configuration
SIYPCR          equ     SIMBase+$21     ; [W] Watchdog, Bus Monitor Control
SWSR            equ     SIMBase+$27     ; [B] Watchdog Reset (write $55/$aa)
PICR            equ     SIMBase+$22     ; [W] Timer Interrupt Control
PITR            equ     SIMBase+$24     ; [W] Timer Counter Value

;-----------------------------------------------------------------------------
; Processor Clock Synthesizer

SYNCR           equ     SIMBase+$04     ; [W] Clock Synthesizer Control

;-----------------------------------------------------------------------------
; Chip Select Outputs

CSPAR0          equ     SIMBase+$44     ; [W] CSBOOT,CS0..CS5 Control
CSPAR1          equ     SIMBase+$46     ; [W] CS6..CS10 Control
CSBARBT         equ     SIMBase+$48     ; [W] Boot ROM Start Address
CSORBT          equ     SIMBase+$4a     ; [W] Boot-ROM Options
__cnt           set     0
                rept    10              ; only generate 0..9 to avoid hex names
__name           set     "\{__CNT}"
CSBAR{__name}    equ     SIMBase+$4c+__cnt*4 ; [W] CSn Start Address
CSOR{__name}     equ     SIMBase+$4e+__cnt*4 ; [W] CSn Options
__cnt            set     __cnt+1
                endm
CSBAR10         equ     SIMBase+$74     ; [W] CS10 Start Address
CSOR10          equ     SIMBase+$76     ; [W] CS10 Options

;-----------------------------------------------------------------------------
; Nutzung der SIM-Bits als einfache I/O-Ports

PORTC           equ     SIMBase+$41     ; [B] Port C Data Bits
PORTE0          equ     SIMBase+$11     ; [B] Port E Data Bits
PORTE1          equ     SIMBase+$13     ; [B] ditto
DDRE            equ     SIMBase+$15     ; [B] Port E Data Direction Bits
PEPAR           equ     SIMBase+$17     ; [B] Port E Pins as Ports or Bus Signals Control
PORTF0          equ     SIMBase+$19     ; [B] Port F Data Bits
PORTF1          equ     SIMBase+$1b     ; [B] ditto
DDRF            equ     SIMBase+$1d     ; [B] Port F Data Direction Bits
PFPAR           equ     SIMBase+$1f     ; [B] Port F Pins as Ports or Bus Signals Control

;-----------------------------------------------------------------------------
; Boundary Scan Test of SIM Registers (for Motorola use only...)

SIMTR           equ     SIMBase+$02     ; [W] SIM Test Register
SIMTRE          equ     SIMBase+$08     ; [W] E Clock Test Register
TSTMSRA         equ     SIMBase+$30     ; [W] Shift Register A (Boundary Scan)
TSTMSRB         equ     SIMBase+$32     ; [W] Shift Register B (Boundary Scan)
TSTSC           equ     SIMBase+$34     ; [W] Shift Count Register
TSTRC           equ     SIMBase+$36     ; [W] Repeat Count Register
CREG            equ     SIMBase+$38     ; [W] Boundary Scan Control Register
DREG            equ     SIMBase+$3a     ; [W] Distributed Register (?!)

;-----------------------------------------------------------------------------
; Programmable Timers:

TPUBase         equ     SIMBase+$400    ; TPU Register Set Base Address
TPUMCR          equ     TPUBase+$00     ; [W] TPU Base Configuration
TICR            equ     TPUBase+$08     ; [W] TPU Interrupt Control
CIER            equ     TPUBase+$0a     ; [W] TPU Interrupt Enable
CISR            equ     TPUBase+$20     ; [W] TPU Interrupt Status
CFSR0           equ     TPUBase+$0c     ; [W] TPU Operating Modes Channels 12..15
CFSR1           equ     TPUBase+$0e     ; [W] TPU Operating Modes Channels  8..11
CFSR2           equ     TPUBase+$10     ; [W] TPU Operating Modes Channels  4.. 7
CFSR3           equ     TPUBase+$12     ; [W] TPU Operating Modes Channels  0.. 3
HSQR0           equ     TPUBase+$14     ; [W] TPU Sub Operating Modes Channels 8..15
HSQR1           equ     TPUBase+$16     ; [W] TPU Sub -Operating Modes Channels 0.. 7
HSRR0           equ     TPUBase+$18     ; [W] TPU Service Request Bits Channels 8..15
HSRR1           equ     TPUBase+$1a     ; [W] TPU Service Request Bits Channels 0.. 7
CPR0            equ     TPUBase+$1c     ; [W] TPU Priority Channels 8..15
CPR1            equ     TPUBase+$1e     ; [W] TPU Priority Channels 0.. 7
DSCR            equ     TPUBase+$04     ; [W] Debug and Test Registers
DSSR            equ     TPUBase+$06
LR              equ     TPUBase+$22
SGLR            equ     TPUBase+$24
DCNR            equ     TPUBase+$26
TCR             equ     TPUBase+$02

;-----------------------------------------------------------------------------
; TPU Command RAM:

TPURAMBase      equ     SIMBase+$100    ; TPURAM Base Address Control Register
TRAMMCR         equ     TPURAMBase+$00  ; [B] TPURAM Base Configuration
TRAMTST         equ     TPURAMBase+$02  ; [W] TPURAM Test Register
TRAMBAR         equ     TPURAMBase+$04  ; [W] TPURAM Base Address

;-----------------------------------------------------------------------------
; serielles:

QSMBase         equ     SIMBase+$200    ; Serial Interface Base Address
QSMCR           equ     QSMBase+$00     ; [W] QSM Base Configuration
QTEST           equ     QSMBase+$02     ; [W] QSM Test Register
QILR            equ     QSMBase+$04     ; [B] QSM Interrupt Priorities
QIVR            equ     QSMBase+$05     ; [B] QSM Interrupt Vector
PORTQS          equ     QSMBase+$15     ; [B] QSM Parallel Port Data Bits
PQSPAR          equ     QSMBase+$16     ; [B] Selection Port Bits QSM/Parallel Port
DDRQS           equ     QSMBase+$17     ; [B] QSM Parallel Port Data Direction Register
SPCR0           equ     QSMBase+$18     ; [W] QSPI Control Register 0
SPCR1           equ     QSMBase+$1a     ; [W] QSPI Control Register 1
SPCR2           equ     QSMBase+$1c     ; [W] QSPI Control Register 2
SPCR3           equ     QSMBase+$1e     ; [B] QSPI Control Register 3
SPSR            equ     QSMBase+$1f     ; [B] QSPI Status Register
__cnt           set     0               ; QSPI RAM Definition
                rept    16
__name           set     "\{__CNT}"
RR{__name}       equ     QSMBase+$100+__cnt*2 ; [W] Data RAM Reception Side
TR{__name}       equ     QSMBase+$120+__cnt*2 ; [W] Data RAM Transmission Side
CR{__name}       equ     QSMBase+$140+__cnt   ; [B] Command RAM
__cnt            set     __cnt+1
                endm
SCCR0           equ     QSMBase+$08     ; [W] SCI Control Register 0
SCCR1           equ     QSMBase+$0a     ; [W] SCI Control Register 1
SCSR            equ     QSMBase+$0c     ; [W] SCI Status Register
SCDR            equ     QSMBase+$0e     ; [W] SCI Data Register

;-----------------------------------------------------------------------------

                endcase                 ; of processor distinction

		restore                 ; re-allow listing

                endif                   ; reg6833xinc
