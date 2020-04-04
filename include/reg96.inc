		ifndef  reg96inc     	; avoid multiple inclusion
reg96inc     	equ     1
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG96.INC                                               *
;*   								            *
;*   Contains Macro, SFR and Address Definitions for MCS-96 Processors      *
;* 									    *
;****************************************************************************

		save
                listing off		; no listing over this file

                if      (MOMCPUNAME<>"8096")&&(MOMCPUNAME<>"80196")&&(MOMCPUNAME<>"80196N")&&(MOMCPUNAME<>"80296")
                 fatal  "wrong target selected: only 8096, 80196, 80196N, or 80299 supported"
		endif


                if      MOMPASS=1
		 message "MCS-96 SFR Definitions (C) 1994 Alfred Arnold"
		 message "binde \{MOMCPU}-SFRs ein"
		endif

;----------------------------------------------------------------------------
; CPU Registers

R0		equ	0		; Zero Register
ZERO_REG        equ     R0
ONES_REG        equ     2               ; All Ones
SP    		equ	18h		; Stack Pointer
                if      MOMCPU>=80196h
WSR              equ     14h            ; RAM Window in Register Bank
                 if      MOMCPU>=80296h
WSR1              equ     15h           ; RAM-Window in lower Half of Register Bank
                 endif
                endif

                if      MOMCPU>=80296h  ; Signal Processor Registers
ACC_00           equ     000ch          ; Accumulator
ACC_02           equ     000eh
ACC_04           equ     0006h
ACC_STAT         equ     000bh
ICB0             equ     1fc3h          ; Index Register
ICB1             equ     1fc7h
ICX0             equ     0010h
ICX1             equ     0016h
IDX0             equ     1fc0h
IDX1             equ     1fc4h
RPT_CNT          equ     0004h          ; Count Register
                endif

;----------------------------------------------------------------------------
; SFRs:

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; A/D Converter

                switch  MOMCPUNAME

                case    "80296"         ; Suppress Warning


                case    "80196N"
AD_RESULT        equ     1faah          ; Conversion Result (read as word)
AD_COMMAND       equ     1fach          ; A/D Converter Control Register
AD_TEST          equ     1faeh          ; Adjustment
AD_TIME          equ     1fafh          ; Conversion Speed


                case    "80196"
AD_Result        equ     1faah          ; Conversion Result (read as word)
AD_Command       equ     1fach          ; A/D Converter Control Register
AD_Test          equ     1faeh          ; Adjustment
AD_Time          equ     1fafh          ; Conversion Speed


                case    "8096"
AD_Command       equ     2              ; A/D Converter Control Register
AD_Result_Lo     equ     2              ; A/D Converter Result
AD_Result_Hi     equ     3              ; ONLY read as bytes!


                endcase

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; HSI/HSO/EPA

                switch  MOMCPUNAME

                case    "80296"
EPA_MASK         equ     1f9ch          ; Interrupt Mask
EPA_PEND         equ     1f9eh          ; Interrupts Pending
__CNT            set     0
                 rept    4
EPA{"\{__CNT}"}_CON equ  1f60h+(__CNT*4) ; Capture/Compare Units Control
EPA{"\{__CNT}"}_TIME equ 1f62h+(__CNT*4) ;    "            "     Time
__CNT             set    __CNT+1
                 endm


                case    "80196N"
COMP0_CON        equ     1f88h          ; Comparator 0 Control
COMP0_TIME       equ     1f8ah          ; Comparator 0 Time

COMP1_CON        equ     1f8ch          ; ditto Comparator 1
COMP1_CON        equ     1f8eh

EPA_MASK         equ     1fa0h          ; Interrupt Masks
EPA_MASK1        equ     1fa4h
EPA_PEND         equ     1fa2h          ; Interrupts Pending
EPA_PEND1        equ     1fa6h
__CNT            set     0
                 rept    10
EPA{"\{__CNT}"}_CON equ  1f60h+(__CNT*4) ; Capture/Compare Units Control
EPA{"\{__CNT}"}_TIME equ 1f62h+(__CNT*4) ;    "            "     Time
__CNT             set    __CNT+1
                 endm
EPAIPV           equ     1fa8h          ; Interrupt Priority


                case    "80196"
EPA_Mask         equ     1fa0h          ; (w) EPA Interrupt Masks 4..9, 0..1, Overrun 0..9/0/1
EPA_Mask1        equ     1fa4h
EPA_Pend         equ     1fa2h          ; (w) EPA Interrupts Pending 4..9, 0..1, Overrun 0..9/0/1
EPA_Pend1        equ     1fa6h
EPAIpv           equ     1fa8h          ; EPA Interrupts Priority

EPA_Control0     equ     1f60h          ; Capture/Compare Control Channels 0..9 Control
EPA_Time0        equ     1f62h          ; (w) Channels 0..9 Saved Time
EPA_Control1     equ     1f64h
EPA_Time1        equ     1f66h
EPA_Control2     equ     1f68h
EPA_Time2        equ     1f6ah
EPA_Control3     equ     1f6ch
EPA_Time3        equ     1f6eh
EPA_Control4     equ     1f70h
EPA_Time4        equ     1f72h
EPA_Control5     equ     1f74h
EPA_Time5        equ     1f76h
EPA_Control6     equ     1f78h
EPA_Time6        equ     1f7ah
EPA_Control7     equ     1f7ch
EPA_Time7        equ     1f7eh
EPA_Control8     equ     1f80h
EPA_Time8        equ     1f82h
EPA_Control9     equ     1f84h
EPA_Time9        equ     1f86h
Comp_Control0    equ     1f88h          ; Compare Channels 0..1 Control
Comp_Time0       equ     1f8ah          ; Compare Channels 0..1 Time
Comp_Control1    equ     1f8ch
Comp_Time1       equ     1f8eh


                case    "8096"
HSI_Time         equ     4              ; HSI Trigger Time (word-only)
HSI_Mode         equ     3              ; HSI Mode Register
HSI_Status       equ     6              ; HSI Status Register

HSO_Time         equ     4              ; HSO Time Setting (word-only)
HSO_Command      equ     6              ; HSO Control Register


                endcase

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Serial

                switch  MOMCPUNAME

                case    "80296"
SP_CON           equ     1fbbh          ; SIO Control
SP_BAUD          equ     1fbch          ; (w) SIO Baud Rate
SP_STATUS        equ     1fb9h          ; SIO Status
SBUF_RX          equ     1fb8h          ; Receive Data
SBUF_TX          equ     1fbah          ; Transmit Data


                case    "80196N"
SP_CON           equ     1fbbh          ; SIO Control
SP_BAUD          equ     1fbch          ; (w) SIO Baud Rate
SP_STATUS        equ     1fb9h          ; SIO Status
SBUF_RX          equ     1fb8h          ; Receive Data
SBUF_TX          equ     1fbah          ; Transmit Data

SSIO_BAUD        equ     1fb4h          ; SSIO Baud Rate
SSIO0_BUF        equ     1fb0h          ; SSIO0 Data Register
SSIO1_BUF        equ     1fb2h          ; SSIO1 Data Register
SSIO0_CON        equ     1fb1h          ; SSIO0 Mode
SSIO1_CON        equ     1fb3h          ; SSIO1 Mode


                case    "80196"
SSIO_StB0        equ     1fb0h          ; SSIO Channel 0 Data Buffer
SSIO_StCr0       equ     1fb1h          ; SSIO Channel 0 Transmitter/Receiver Control
SSIO_StB1        equ     1fb2h          ; SSIO Channel 1 Data Buffer
SSIO_StCr1       equ     1fb3h          ; SSIO Channel 1 Transmitter/Receiver Control
SSIO_Baud        equ     1fb4h          ; SSIO Baud Rate
SBuf_RX          equ     1fb8h          ; Receive Data
SP_Stat          equ     1fb9h          ; SIO State
SBuf_TX          equ     1fbah          ; Transmit Data
SP_Con           equ     1fbbh          ; SIO Control
SP_Baud          equ     1fbch          ; (w) SIO Baud Rate


                case    "8096"
SBUF             equ     7              ; UART Data Register
SPStat           equ     11h            ; UART Status Register
SPCon            equ     11h            ; UART Control Register
Baud_Reg         equ     0eh            ; UART Data Rate

                endcase

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Interrupt Control

                switch  MOMCPUNAME

                case    "80296"
INT_MASK         equ     08h            ; Interrupt Masks
INT_MASK1        equ     13h
INT_PEND         equ     09h            ; Interrupts Pending
INT_PEND1        equ     12h
EXTINT_CON       equ     1fcch          ; External Interrupts Edge Selection
IN_PROG0         equ     1fc8h          ; Interrupt Handling in Progress
IN_PROG1         equ     1fcah
INT_CON0         equ     1fe8h          ; Priority Setting
INT_CON1         equ     1feah
INT_CON2         equ     1fech
INT_CON3         equ     1feeh
NMI_PEND         equ     1fc9h          ; NMI Pending
VECT_ADDR        equ     1ff0h          ; Vekcor Base

                case    "80196N"
INT_MASK         equ     08h            ; Interrupt Masks
INT_MASK1        equ     13h
INT_PEND         equ     09h            ; Interrupts Pending
INT_PEND1        equ     12h

PTSSRV           equ     06h            ; (w) PTS Interrupt Served
PTSSEL           equ     04h            ; (w) Enable PTS Sources


                case    "80196"
Int_Mask_Lo      equ     08h            ; Interrupt Masks
Int_Mask_Hi      equ     13h
Int_Pend_Lo      equ     09h            ; Interrupts Pending
Int_Pend_Hi      equ     12h

PTS_Srv          equ     06h            ; (w) PTS Interrupt Served
PTS_Select       equ     04h            ; (w) Enable PTS Sources


                case    "8096"
INT_Mask         equ     8              ; Interrupt Mask
INT_Pending      equ     9              ; Interrupts Pending

                endcase

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Timer

Watchdog	equ	0ah		; Reset Watchdog

                switch  MOMCPUNAME


                case    "80296"
CON_REG0         equ     1fb6h          ; PWM Control
PWM0_CONTROL     equ     1fb0h          ; PWMn Duty Cycle
PWM1_CONTROL     equ     1fb2h
PWM2_CONTROL     equ     1fb4h

T1CONTROL        equ     1f90h          ; Timer 1 Control
TIMER1           equ     1f92h          ; (w) Timer 1 Data
T2CONTROL        equ     1f94h          ; Timer 2 Control
TIMER22          equ     1f96h          ; (w) Timer 2 Data


                case    "80196N"
T1CONTROL        equ     1f98h          ; Timer 1 Control
TIMER1           equ     1f9ah          ; (w) Timer 1 Data
T2CONTROL        equ     1f9ch          ; Timer 2 Control
TIMER22          equ     1f9eh          ; (w) Timer 2 Data


                case    "80196"
Timer1_Control   equ     1f98h          ; Timer 1 Control
Timer1           equ     1f9ah          ; (w) Timer 1 Data
Timer2_Control   equ     1f9ch          ; Timer 2 Control
Timer2           equ     1f9eh          ; (w) Timer 2 Data


                case    "8096"
Timer1           equ     0ah            ; Timer 1 (word-only)
Timer2           equ     0ch            ; Timer 2 (word-only)

PWM_Control      equ     17h            ; PWM Duty Cycle Setting


                endcase

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Ports

                switch  MOMCPUNAME


                case    "80296"

P1_DIR           equ     1fd2h          ; Port 1 Direction Register
P1_MODE          equ     1fd0h          ; Port 1 Special Functions vs. IO Port Selection
P1_PIN           equ     1fd6h          ; Port 1 Pin Status
P1_REG           equ     1fd4h          ; Port 1 Output Data

P2_DIR           equ     1fd3h          ; the same for remaining ports 
P2_MODE          equ     1fd1h
P2_PIN           equ     1fd7h
P2_REG           equ     1fd5h

P3_DIR           equ     1fdah
P3_MODE          equ     1fd8h
P3_PIN           equ     1fdeh
P3_REG           equ     1fdch

P4_DIR           equ     1fdbh
P4_MODE          equ     1fd9h
P4_PIN           equ     1fdfh
P4_REG           equ     1fddh

EP_DIR           equ     1fe3h
EP_MODE          equ     1fe1h
EP_PIN           equ     1fe7h
EP_REG           equ     1fe5h


                case    "80196N"
P0_PIN           equ     1fdah          ; Port 0 Data

P1_DIR           equ     1fd2h          ; Port 1 Direction Register
P1_MODE          equ     1fd0h          ; Port 1 Special Functions vs. IO Port Selection
P1_PIN           equ     1fd6h          ; Port 1 Pin Status
P1_REG           equ     1fd4h          ; Port 1 Output Data

P2_DIR           equ     1fcbh          ; the same for remaining ports
P2_MODE          equ     1fc9h
P2_PIN           equ     1fcfh
P2_REG           equ     1fcdh

P3_PIN           equ     1ffeh
P3_REG           equ     1ffch

P4_PIN           equ     1fffh
P4_REG           equ     1ffdh

P34_DRV          equ     1ff4h          ; Configure Ports 3/4 as Open Drain or Push-Pull

P5_DIR           equ     1ff3h
P5_MODE          equ     1ff1h
P5_PIN           equ     1ff7h
P5_REG           equ     1ff5h

P6_DIR           equ     1fd3h
P6_MODE          equ     1fd1h
P6_PIN           equ     1fd7h
P6_REG           equ     1fd5h

EP_DIR           equ     1fe3h
EP_MODE          equ     1fe1h
EP_PIN           equ     1fe7h
EP_REG           equ     1fe5h


                case    "80196"
P0PIn            equ     1fdah          ; Port 0 Data

P1SSel           equ     1fd0h          ; Port 1 Special Functions Selection
P1IO             equ     1fd2h          ; Port 1 Direction Register
P1Reg            equ     1fd4h          ; Port 1 Data Output
P1PIn            equ     1fd6h          ; Port 1 Input State

P2SSel           equ     1fc9h          ; the same for remaining ports
P2IO             equ     1fcbh
P2Reg            equ     1fcdh
P2PIn            equ     1fcfh

P3Reg            equ     1ffch
P3PIn            equ     1ffeh

P4Reg            equ     1ffdh
P4PIn            equ     1fffh

P5SSel           equ     1ff1h
P5IO             equ     1ff3h
P5Reg            equ     1ff5h
P5PIn            equ     1ff7h

P6SSel           equ     1fd1h
P6IO             equ     1fd3h
P6Reg            equ     1fd5h
P6PIn            equ     1fd7h


                case    "8096"
Port0            equ     0eh            ; Port 0
Port1            equ     0fh            ; Port 1
Port2            equ     10h            ; Port 2
Port3            equ     1ffeh          ; Port 3 (if no external memory)
Port4            equ     1fffh          ; Port 4 (if no external memory)

IOS0             equ     15h            ; I/O Status Register 0
IOS1             equ     16h            ; I/O Status Register 1
IOC0             equ     15h            ; I/O Control Register 0
IOC1             equ     16h            ; I/O Control Register 1

                endcase

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Slave Port

                switch  MOMCPUNAME

                case    "80296"         ; Suppress Warning


                case    "80196N"
SLP_CMD          equ     1ffah          ; Command Register
SLP_CON          equ     1ffbh
SLP_STAT         equ     1ff8h


                case    "80196"         ; Suppress Warning


                case    "8096"
SlpStat          equ     1ff8h          ; Slave Port Status
SlpCmd           equ     1ffah          ; Slave Port Command Register
SlpFunReg        equ     1ffbh          ; Slave-Port Control

                endcase

;----------------------------------------------------------------------------
; Interrupt Vectors

                if      MOMCPU>=80196h
EPAIntX_Vec      equ     2000h          ; EPA Channel 4...9/0/1 Vector
EPAInt3_Vec      equ     2002h          ; EPA Channel 3 Vector
EPAInt2_Vec      equ     2004h          ; EPA Channel 2 Vector
EPAInt1_Vec      equ     2006h          ; EPA Channel 1 Vector
EPAInt0_Vec      equ     2008h          ; EPA Channel 0 Vector
AD_Complete_Vec  equ     200ah          ; A/D Conversion Complete Vector
OBE_Slp_Vec      equ     200ch          ; Output Buffer Empty Vector
IBF_Slp_Vec      equ     200eh          ; Input Buffer Full Vector
Trap_Vec         equ     2010h          ; Software Trap Vector
Ill_Opcode_Vec   equ     2012h          ; Undefined Opcode Vector
CMD_Full_Vec     equ     2030h          ; Command Buffer Full Vector
SSIO0_Trans_Vec  equ     2032h          ; SSIO0 Transfer Completed Vector
SSIO1_Trans_Vec  equ     2034h          ; SSIO1 Transfer Completed Vector
SIO_TrInt_Vec    equ     2036h          ; SIO Character Transmitted Vector
SIO_RcInt_Vec    equ     2038h          ; SIO Character Received Vector
ExtInt_Vec       equ     203ch          ; External Interrupt Vector
NMI_Vec          equ     203eh          ; Non Maskable Interrupt
                elseif
TOverInt_Vec     equ     2000h          ; Timer Overflow Vector
ADInt_Vec        equ     2002h          ; AD Conversion Completed Vector
HSIDataInt_Vec   equ     2004h          ; HSI Data Available Vector
HSOInt_Vec       equ     2006h          ; HSO Vector
HSI0Int_Vec      equ     2008h          ; HSI.0 Vector
TimeInt_Vec      equ     200ah          ; Timer Vector
SerialInt_Vec    equ     200ch          ; Serial Interrupt Vector
ExtInt_Vec       equ     200eh          ; External Interrupt Vector
SoftInt_Vec      equ     2010h          ; TRAP Interrupt Vector
Self_Jump        equ     201ah          ; Infinite Loop
                endif

;----------------------------------------------------------------------------
; Meory Layout, Reserved Addresses

                switch  MOMCPUNAME


                case    "80296"
IRAM             equ     1ah            ; Beginn Internal RAM
IRAMEND          equ     1ffh           ; Ende Internal RAM
CODERAM          equ     0f800h         ; Beginn Code-RAM
CODERAMEND       equ     0ffffh         ; Ende Code-RAM
EXTMEM1          equ     400h           ; Beginn External Memory Area 1
EXTMEM1END       equ     1bffh          ; Ende External Memory Area 1
EXTMEM2          equ     02000h         ; Beginn External Memory Area 2
EXTMEM2END       equ     0f7ffh         ; Ende External Memory Area 2
EXTMEM3          equ     0ff0400h       ; Beginn External Memory Area 3
EXTMEM3END       equ     0fff7ffh       ; Ende External Memory Area 3
CCB0             equ     0ff2018h       ; Chip Configuration
CCB1             equ     0ff201ah       ; Extended Chip Configuration
Reset_Location   equ     0ff2080h       ; RESET Entry


                case    "80196N"
IRAM             equ     1ah            ; Internal RAM Start
IRAMEND          equ     3ffh           ; Internal RAM End
IROM             equ     0ff2000h       ; Internal Program ROM Start
IROMEND          equ     0ff9fffh       ; Internal Program ROM End
EXTMEM1          equ     600h           ; External Memory Area 1 Start
EXTMEM1END       equ     1effh          ; External Memory Area 1 End
EXTMEM2          equ     0a000h         ; External Memory Area 2 Start
EXTMEM2END       equ     0ffffh         ; External Memory Area 2 End
EXTMEM3          equ     0ff0100h       ; External Memory Area 3 Start
EXTMEM3END       equ     0ff03ffh       ; External Memory Area 3 End
EXTMEM4          equ     0ff0600h       ; External Memory Area 4 Start
EXTMEM4END       equ     0ff1fffh       ; External Memory Area 4 End
EXTMEM5          equ     0ffa000h       ; External Memory Area 5 Start
EXTMEM5END       equ     0ffffffh       ; External Memory Area 5 End
CCB0             equ     0ff2018h       ; Chip Configuration
CCB1             equ     0ff201ah       ; Extended Chip Configuration
CCB2             equ     0ff201ch
Security_Key     equ     0ff2020h       ; Key
Reset_Location   equ     0ff2080h       ; RESET Entry
USFR             equ     1ff6h          ; ???
IRAM_CON         equ     1fe0h          ; Konfiguration Code-RAM


                case    "80196"
IRAM             equ     1ah            ; Internal RAM Start
IRAMEND          equ     1ffh           ; Internal RAM End
PDRAM            equ     IRAM           ; Power Down RAM Start
PDRAMEND         equ     1ffh           ; Power Down RAM End
CODERAM          equ     400h           ; Code RAM Start (NO Windowing!)
CODERAMEND       equ     4ffh           ; Code RAM End
IROM             equ     2000h          ; Internal Program ROM Start
IROMEND          equ     3fffh          ; Internal Program ROM End
EXTMEM1          equ     500h           ; External Memory Area 1 Start
EXTMEM1END       equ     1effh          ; External Memory Area 1 End
EXTMEM2          equ     4000h          ; External Memory Area 2 Start
EXTMEM2END       equ     0ffffh         ; External Memory Area 2 End
CCB              equ     2018h          ; Chip Configuration
CCB1             equ     201ah          ; Extended Chip Configuration
Security_Key     equ     2020h          ; Key
Reset_Location   equ     2080h          ; RESET Entry
USFR             equ     1ff6h          ; ???


                case    "8096"
IRAM             equ     1ah            ; Internal RAM Start
IRAMEND          equ     0ffh           ; Internal RAM End
PDRAM            equ     0f0h           ; Power-Down-RAM Start
PDRAMEND         equ     0ffh           ; Power-Down-RAM End
IROM             equ     2000h          ; Internal Program ROM Start
IROMEND          equ     3fffh          ; Internal Program ROM End
EXTMEM1          equ     100h           ; External Memory Area 1 Start
EXTMEM1END       equ     1effh          ; External Memory Area 1 End
EXTMEM2          equ     4000h          ; External Memory Area 2 Start
EXTMEM2END       equ     0ffffh         ; External Memory Area 2 End
CCB              equ     2018h          ; Chip Configuration
Security_Key     equ     2020h          ; Key
Reset_Location   equ     2080h          ; RESET Entry


                endcase

Security_Key_End equ    Security_Key+15

;--------------------------------------------------------------------------
; Long Jumps

__DefLongJmp    macro   new,old
new             macro   Adr,{NoExpand}
                old     Skip
                ljmp    Adr
Skip:
                endm
                endm

                __DefLongJmp bc,jnc
                __DefLongJmp be,jne
                __DefLongJmp bge,jlt
                __DefLongJmp bgt,jle
                __DefLongJmp bh,jnh
                __DefLongJmp ble,jgt
                __DefLongJmp blt,jge
                __DefLongJmp bnc,jc
                __DefLongJmp bne,je
                __DefLongJmp bnh,jh
                __DefLongJmp bnst,jst
                __DefLongJmp bnv,jv
                __DefLongJmp bnvt,jvt
                __DefLongJmp bst,jnst
                __DefLongJmp bv,jnv
                __DefLongJmp bvt,jnvt

;--------------------------------------------------------------------------

		restore                 ; re-enable listing

                endif			; reg96inc
