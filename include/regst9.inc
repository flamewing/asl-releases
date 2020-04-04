                ifndef  regst9inc       ; avoid multiple inclusion
regst9inc       equ     1

                save
                listing off   		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - Datei REGST9.INC                                             *
;*                                                                          *
;*   Contains SFR, Macro, and Addres Definitions for the ST9 Family         *
;*                                                                          *
;****************************************************************************

                if      (MOMCPUNAME<>"ST9020")&&(MOMCPUNAME<>"ST9030")&&(MOMCPUNAME<>"ST9040")&&(MOMCPUNAME<>"ST9050")
                 fatal  "wrong target selected: only ST9020,ST9030,ST9040, or ST9050 supported"
		endif

                if      MOMPASS=1
                 message "ST9 SFR Definitions (C) 1997 Alfred Arnold"
		endif

;----------------------------------------------------------------------------
; Register Banks

__CNT           set     0
                rept    16
BK{"\{__CNT}"}0 equ     __CNT*2
BK{"\{__CNT}"}1 equ     __CNT*2+1
__CNT           set     __CNT+1
                endm

BK_SYS          equ     BKE0            ; Group System Definition
BK_F            equ     BKF0            ; Page Register Definition

;----------------------------------------------------------------------------
; Definition of a Bits:
; This exploits the internal representation of Bit symbols: rrrrbbbi

__defbit        macro   NAME,REG,BITPOS
NAME            bit     ((REG&15)<<4)+(BITPOS<<1)
{"NAME"}m       equ     1<<BITPOS
                endm

;----------------------------------------------------------------------------
; System Group

FCW             reg     RR230           ; Flag and Control Word
CICR            reg     R230            ; Central Interrupt Control Register
                __defbit gcen,CICR,7    ; Global Counter Enable
                __defbit tlip,CICR,6    ; Top Level Interrupt Pending Bit
                __defbit tli,CICR,5     ; Top Level Interrupt Bit
                __defbit ien,CICR,4     ; Interrupt Enable Flag
                __defbit iam,CICR,3     ; Interrupt Arbitration Mode
                __defbit cpl2,CICR,2    ; Current Priority Level Bit 2
                __defbit cpl1,CICR,1    ; Current Priority Level Bit 1
                __defbit cpl0,CICR,0    ; Current Priority Level Bit 0
cplm            equ     cpl2m|cpl1m|cpl0m ; Current Priority Level

FLAGR           reg     R231            ; Flags Register
                __defbit c,FLAGR,7      ; Carry Flag
                __defbit z,FLAGR,6      ; Zero Flag
                __defbit s,FLAGR,5      ; Sign Flag
                __defbit v,FLAGR,4      ; Overflow Flag
                __defbit d,FLAGR,3      ; Decimal Adjust Flag
                __defbit h,FLAGR,2      ; Half Carry Flag
                __defbit uf,FLAGR,1     ; User Flag 1
                __defbit dp,FLAGR,0     ; Data/Program Memory Flag

RPP             reg     RR232           ; Register Pointer Pair

RP0R            reg     R232            ; Register Pointer #0
                __defbit rp0s,RP0R,2    ; Register Pointer Selector

RP1R            reg     R233            ; Register Pointer #1
                __defbit rp1s,RP1R,2    ; Register Pointer Selector

PPR             reg     R234            ; Page Pointer Register

MODER           reg     R235            ; Mode Register
                __defbit ssp,MODER,7    ; System Stack Pointer Flag (int/ext)
                __defbit usp,MODER,6    ; User Stack Pointer Flag (int/ext)
                __defbit div2,MODER,5   ; External Clock Divided by 2
                __defbit prs2,MODER,4   ; Internal Clock Prescaling Bit 2
                __defbit prs1,MODER,3   ; Internal Clock Prescaling Bit 1
                __defbit prs0,MODER,2   ; Internal Clock Prescaling Bit 0
                __defbit brqen,MODER,1  ; Bus Request Enable
                __defbit himp,MODER,0   ; High Impedance Enable
prsm            equ     prs2m|prs1m|prs0m ; Internal Clock Prescaler

USPR            reg     RR236           ; User Stack Pointer
USPHR           reg     R236
USPLR           reg     R237

SSPR            reg     RR238           ; System Stack Pointer
SSPHR           reg     R238
SSPLR           reg     R239

;----------------------------------------------------------------------------
; EEPROM

                if      MOMCPUNAME="ST9040"

EEP_PG          equ     0               ; EEPROM Register Page

EECR            reg     R241            ; EEPROM Control Register
                __defbit verify,EECR,6  ; EEPROM Verify Mode
                __defbit eestby,EECR,5  ; EEPROM Stand-By
                __defbit eeien,EECR,4   ; EEPROM Interrupt Enable
                __defbit pllst,EECR,3   ; Parallel Write Start
                __defbit pllen,EECR,2   ; Parallel Write Enable
                __defbit eebusy,EECR,1  ; EEPROM Busy
                __defbit eewen,EECR,0   ; EEPROM Write Enable

                endif

;----------------------------------------------------------------------------
; Interrupts

EXINT_PG        equ     0               ; EXTERNAL Interrupt Register Page

EITR            reg     R242            ; External Interrupt Trigger Level Register
                __defbit tea0,EITR,0    ; Trigger Event A0 Bit
                __defbit tea1,EITR,1    ; Trigger Event A1 Bit
                __defbit teb0,EITR,2    ; Trigger Event B0 Bit
                __defbit teb1,EITR,3    ; Trigger Event B1 Bit
                __defbit tec0,EITR,4    ; Trigger Event C0 Bit
                __defbit tec1,EITR,5    ; Trigger Event C1 Bit
                __defbit ted0,EITR,6    ; Trigger Event D0 Bit
                __defbit ted1,EITR,7    ; Trigger Event D1 Bit

EIPR            reg     R243            ; External Interrupt Pending Register
                __defbit ipa0,EIPR,0    ; Interrupt Pending Bit Channel A0
                __defbit ipa1,EIPR,1    ; Interrupt Pending Bit " A1
                __defbit ipb0,EIPR,2    ; Interrupt Pending Bit " B0
                __defbit ipb1,EIPR,3    ; Interrupt Pending Bit " B1
                __defbit ipc0,EIPR,4    ; Interrupt Pending Bit " C0
                __defbit ipc1,EIPR,5    ; Interrupt Pending Bit " C1
                __defbit ipd0,EIPR,6    ; Interrupt Pending Bit " D0
                __defbit ipd1,EIPR,7    ; Interrupt Pending Bit " D1

EIMR            reg     R244            ; External Interrupt Mask Register
                __defbit ima0,EIMR,0    ; Int. A0 Bit
                __defbit ima1,EIMR,1    ; Int. A1 Bit
                __defbit imb0,EIMR,2    ; Int. B0 Bit
                __defbit imb1,EIMR,3    ; Int. B1 Bit
                __defbit imc0,EIMR,4    ; Int. C0 Bit
                __defbit imc1,EIMR,5    ; Int. C1 Bit
                __defbit imd0,EIMR,6    ; Int. D0 Bit
                __defbit imd1,EIMR,7    ; Int. D1 Bit

EIPLR           reg     R245            ; Ext. Interrupt Priority Level Register

EIVR            reg     R246            ; External Interrupt Vector Register
                __defbit ewen,EIVR,0    ; External Wait Enable
                __defbit ia0s,EIVR,1    ; Interrupt A0 Selection
                __defbit tlis,EIVR,2    ; Top Level Input Selection
                __defbit tltev,EIVR,3   ; Top Level Trigger Event

NICR            reg     R247            ; Nested Interrupt Control Register

                __defbit tlnm,NICR,7    ; Top Level not Maskable

;----------------------------------------------------------------------------
; Watchdog

WDT_PG          equ     0               ; Timer Watchdog Page

WDTR            reg     RR248           ; TWD Timer Constant Register
WDTHR           reg     R248            ; TWD Timer High Constant Register
WDTLR           reg     R249            ; TWD Timer Low Constant Register

WDTPR           reg     R250            ; TWD Timer Prescaler Constant Register

WDTCR           reg     R251            ; TWD Timer Control Register
                __defbit WD_stsp8,WDTCR,7 ; TWD Start Stop
                __defbit WD_sc,WDTCR,6    ; TWD Single Continuous Mode
                __defbit WD_inmd1,WDTCR,5 ; Input Mode 1
                __defbit WD_inmd2,WDTCR,4 ; Input Mode 2
                __defbit WD_inen,WDTCR,3  ; TWD Input Section Enable/Disable
                __defbit WD_outmd,WDTCR,2 ; TWD Output Mode
                __defbit WD_wrout,WDTCR,1 ; TWD Output Bit
                __defbit WD_outen,WDTCR,0 ; TWD Output Enable
inm_evc         equ     0               ; TWD Input Mode Event Counter
inm_g           equ     010h            ; TWD Input Mode Gated
inm_t           equ     020h            ; TWD Input Mode Triggerable
inm_r           equ     030h            ; TWD Input Mode Retriggerable

WCR             reg     R252            ; Wait Control Register
                __defbit WD_wden,WCR,6  ; TWD Timer Enable
wdm2            equ     1 << 5          ; Data Memory Wait Cycle
wdm1            equ     1 << 4
wdm0            equ     1 << 3
wpm2            equ     1 << 2          ; Program Memory Wait Cycle
wpm1            equ     1 << 1
wpm0            equ     1 << 0
dmwc1           equ     wdm0            ; 1 Wait Cycle on Data M.
dmwc2           equ     wdm1            ; 2 Wait Cycles on Data M.
dmwc3           equ     wdm1|wdm0       ; 3 Wait Cycles on Data M.
dmwc4           equ     wdm2            ; 4 Wait Cycles on Data M.
dmwc5           equ     wdm2|wdm0       ; 5 Wait Cycles on Data M.
dmwc6           equ     wdm2|wdm1       ; 6 Wait Cycles on Data M.
dmwc7           equ     wdm2|wdm1|wdm0  ; 7 Wait Cycles on Data M.
pmwc1           equ     wpm0            ; 1 Wait Cycle on Prog M.
pmwc2           equ     wpm1            ; 2 Wait Cycles on Prog M.
pmwc3           equ     wpm1|wpm0       ; 3 Wait Cycles on Prog M.
pmwc4           equ     wpm2            ; 4 Wait Cycles on Prog M.
pmwc5           equ     wpm2|wpm0       ; 5 Wait Cycles on Prog M.
pmwc6           equ     wpm2|wpm1       ; 6 Wait Cycles on Prog M.
pmwc7           equ     wpm2|wpm1|wpm0  ; 7 Wait Cycles on Prog M.

;----------------------------------------------------------------------------
; SPI

SPI_PG          equ     0               ; SPI Register Page
SPIDR           reg     R253            ; SPI Data Register

SPICR           reg     R254            ; SPI Control Register
                __defbit SP_spen,SPICR,7 ; Serial Peripheral Enable
                __defbit SP_bms,SPICR,6  ; SBUS/I2C Bus Mode Selector
                __defbit SP_arb,SPICR,5  ; Arbitration Flag Bit
                __defbit SP_busy,SPICR,4 ; SPI Busy Flag
                __defbit SP_cpol,SPICR,3 ; SPI Transmission Clock Polarity
                __defbit SP_cpha,SPICR,2 ; SPI Transmission Clock Phase
                __defbit SP_spr1,SPICR,1 ; SPI Rate Bit 1
                __defbit SP_spr0,SPICR,0 ; SPI Rate Bit 0

SP_8            equ     0               ; SPI Clock Divider 8 = 1500 kHz (12MHz)
SP_16           equ     1               ; SPI Clock Divider 16 = 750 kHz (12MHz)
SP_128          equ     2               ; SPI Clock Divider 128 = 93.75 kHz (12MHz)
SP_256          equ     3               ; SPI Clock Divider 256 = 46.87 kHz (12MHz)
RW_PG           equ     0               ; R/W Signal Programming Page

;----------------------------------------------------------------------------
; Read/Write Register

                if      MOMCPUNAME="ST9050"

RWR             reg     R255            ; R/W Signal Programming Register
                __defbit RW_rw,RWR,0    ; R/W Bit
                __defbit RW_bs,RWR,1    ; Bank Switch Port Timing

                endif

;----------------------------------------------------------------------------
; Ports

P0C_PG          equ     2               ; Port 0 Control Registers Page

P0DR            reg     R224            ; Port 0 Data Register
P0C0R           reg     R240            ; Port 0 Control Register 0
P0C1R           reg     R241            ; Port 0 Control Register 1
P0C2R           reg     R242            ; Port 0 Control Register 2

P1C_PG          equ     2               ; Port 1 Control Registers Page
P1DR            reg     R225            ; Port 1 Data Register
P1C0R           reg     R244            ; Port 1 Control Register 0
P1C1R           reg     R245            ; Port 1 Control Register 1
P1C2R           reg     R246            ; Port 1 Control Register 2

P2C_PG          equ     2               ; Port 2 Control Registers Page
P2DR            reg     R226            ; Port 2 Data Register
BS_DSR          reg     R226            ; Bank Switch Data Segment Register
P2C0R           reg     R248            ; Port 2 Control Register 0
BS_DDSR         reg     R248            ; Bank Switch Data DMA Segment Register
P2C1R           reg     R249            ; Port 2 Control Register 1
BS_PDSR         reg     R249            ; Bank Switch Program DMA Segment Register
P2C2R           reg     R250            ; Port 2 Control Register 2

P3C_PG          equ     2               ; Port 3 Control Registers Page
P3DR            reg     R227            ; Port 3 Data Register
BS_PSR          reg     R227            ; Bank Switch Program Segment Register
P3C0R           reg     R252            ; Port 3 Control Register 0
P3C1R           reg     R253            ; Port 3 Control Register 1
P3C2R           reg     R254            ; Port 3 Control Register 2

P4C_PG          equ     3               ; Port 4 Control Registers Page
P4DR            reg     R228            ; Port 4 Data Register
P4C0R           reg     R240            ; Port 4 Control Register 0
P4C1R           reg     R241            ; Port 4 Control Register 1
P4C2R           reg     R242            ; Port 4 Control Register 2

P5C_PG          equ     3               ; Port 5 Control Registers Page
P5DR            reg     R229            ; Port 5 Data Register
P5C0R           reg     R244            ; Port 5 Control Register 0
P5C1R           reg     R245            ; Port 5 Control Register 1
P5C2R           reg     R246            ; Port 5 Control Register 2

P6C_PG          equ     3               ; Port 6 Control Registers Page
P6D_PG          equ     3               ; Port 6 Data Register Page
P6DR            reg     R251            ; Port 6 Data Register
P6C0R           reg     R248            ; Port 6 Control Register 0
P6C1R           reg     R249            ; Port 6 Control Register 1
P6C2R           reg     R250            ; Port 6 Control Register 2

P7C_PG          equ     3               ; Port 7 Control Registers Page
P7D_PG          equ     3               ; Port 7 Data Register Page
P7DR            reg     R255            ; Port 7 Data Register
P7C0R           reg     R252            ; Port 7 Control Register 0
P7C1R           reg     R253            ; Port 7 Control Register 1
P7C2R           reg     R254            ; Port 7 Control Register 2

P8C_PG          equ     43              ; Port 8 Control Registers Page
P8D_PG          equ     43              ; Port 8 Data Register Page
P8DR            reg     R251            ; Port 8 Data Register
P8C0R           reg     R248            ; Port 8 Control Register 0
P8C1R           reg     R249            ; Port 8 Control Register 1
P8C2R           reg     R250            ; Port 8 Control Register 2

P9C_PG          equ     43              ; Port 9 Control Registers Page
P9D_PG          equ     43              ; Port 9 Data Register Page
P9DR            reg     R255            ; Port 9 Data Register
P9C0R           reg     R252            ; Port 9 Control Register 0
P9C1R           reg     R253            ; Port 9 Control Register 1
P9C2R           reg     R254            ; Port 9 Control Register 2

HDCTL2R         reg     R251            ; Port 2 handshake DMA Control Register
HDCTL3R         reg     R255            ; Port 3 handshake DMA Control Register
HDCTL4R         reg     R243            ; Port 4 handshake DMA Control Register
HDCTL5R         reg     R247            ; Port 5 handshake DMA Control Register

; Handshake DMA Control Register configuration
hsdis           equ     0E0h            ; Handshake Disabled Mask
hso2            equ     0C0h            ; Handshake Output 2 Lines Mask
hso1            equ     040h            ; Handshake Output 1 Line Mask
hsi2            equ     0A0h            ; Handshake Input 2 Lines Mask
hsi1            equ     020h            ; Handshake Input 1 Line Mask
hsb             equ     000h            ; Handshake bidirectional Mask
den             equ     000h            ; DMA Enable Mask
ddi             equ     010h            ; DMA Disable Mask
ddw             equ     000h            ; Data Direction Output Mask (Write)
ddr             equ     008h            ; Data Direction Input Mask (read)
dst             equ     004h            ; DMA Strobe on Chip Event Mask
dcp0            equ     000h            ; DMA Channel Capture0 Mask
dcm0            equ     002h            ; DMA Channel Compare0 Mask

;----------------------------------------------------------------------------
; Multi Function Timer

T0D_PG          equ     10              ; MFTimer 0 Data Registers Page
T0C_PG          equ     9               ; MFTimer 0 Control Registers Page
T1D_PG          equ     8               ; MFTimer 1 Data Registers Page
T1C_PG          equ     9               ; MFTimer 1 Control Registers Page
T2D_PG          equ     14              ; MFTimer 2 Data Registers Page
T2C_PG          equ     13              ; MFTimer 2 Control Registers Page
T3D_PG          equ     12              ; MFTimer 3 Data Registers Page
T3C_PG          equ     13              ; MFTimer 3 Control Registers Page

T_REG0R         reg     RR240           ; MFTimer REG0 Load and Capture Register
T_REG0HR        reg     R240            ; Register 0 High Register
T_REG0LR        reg     R241            ; Register 0 Low Register
T_REG1R         reg     RR242           ; MFTimer REG1 Load Constant
T_REG1HR        reg     R242            ; Register 1 High Register
T_REG1LR        reg     R243            ; Register 1 Low Register
T_CMP0R         reg     RR244           ; MFTimer CMP0 Store Compare Constant
T_CMP0HR        reg     R244            ; Compare 0 High Register
T_CMP0LR        reg     R245            ; Compare 0 Low Register
T_CMP1R         reg     RR246           ; MFTimer CMP1 Store Compare Constant
T_CMP1HR        reg     R246            ; Compare 1 High Register
T_CMP1LR        reg     R247            ; Compare 1 Low Register

T_TCR           reg     R248             ; MFTimer Control Register
                __defbit T_cs,T_TCR,0    ; Counter Status
                __defbit T_of0,T_TCR,1   ; Over/UnderfLow on CAP on REG0
                __defbit T_udcs,T_TCR,2  ; Up/Down Count Status
                __defbit T_udc,T_TCR,3   ; Up/Down Count
                __defbit T_ccl,T_TCR,4   ; Counter Clear
                __defbit T_ccmp0,T_TCR,5 ; Clear on Compare 0
                __defbit T_ccp0,T_TCR,6  ; Clear on Capture
                __defbit T_cen,T_TCR,7   ; Counter Enable

T_TMR           reg     R249            ; MFTimer Mode Register
                __defbit T_co,T_TMR,0   ; Continuous/One Shot Bit
                __defbit T_ren,T_TMR,1  ; Retrigger Enable Bit
                __defbit T_eck,T_TMR,2  ; Enable Clocking Mode Bit
                __defbit T_rm0,T_TMR,3  ; Register 0 Mode Bit
                __defbit T_rm1,T_TMR,4  ; Register 1 Mode Bit
                __defbit T_bm,T_TMR,5   ; Bivalue Mode Bit
                __defbit T_oe0,T_TMR,6  ; Output 0 Enable Bit
                __defbit T_oe1,T_TMR,7  ; Output 1 Enable Bit

T_ICR           reg     R250            ; MFTimer External Input Control Register

exb_f           equ     01h             ; External B Falling Edge Sensitive Mask
exb_r           equ     02h             ; External B Rising Edge Sensitive Mask
exb_rf          equ     03h             ; External B Falling and Rising Edge Mask
exa_f           equ     04h             ; External A Falling Edge Sensitive Mask
exa_r           equ     08h             ; External A Rising Edge Sensitive Mask
exa_rf          equ     0Ch             ; External A Falling and Rising Edge Mask
ab_ii           equ     00h             ; A I/O B I/O Mask
ab_it           equ     10h             ; A I/O B Trigger Mask
ab_gi           equ     20h             ; A Gate B I/O Mask
ab_gt           equ     30h             ; A Gate B Trigger Mask
ab_ie           equ     40h             ; A I/O B External Clock Mask
ab_ti           equ     50h             ; A Trigger B I/O Mask
ab_ge           equ     60h             ; A Gate B External Clock Mask
ab_tt           equ     70h             ; A Trigger B Trigger Mask
ab_cucd         equ     80h             ; A Clock Up B Clock Down Mask
ab_ue           equ     90h             ; A Clock Up/Down B External Clock Mask
ab_tutd         equ     0A0h            ; A Trigger Up B Trigger Down Mask
ab_ui           equ     0B0h            ; A Up/Down Clock B I/O Mask
ab_aa           equ     0C0h            ; A Autodiscr. B Autodiscr. Mask
ab_te           equ     0D0h            ; A Trigger B External Clock Mask
ab_et           equ     0E0h            ; A External Clock B Trigger Mask
ab_tg           equ     0F0h            ; A Trigger B Gate Mask

T_PRSR          reg     R251            ; MFTimer Prescaler Register

T_OACR          reg     R252            ; MFTimer Output A Control Register

cev             equ     02h             ; On Chip Event Bit on Compare 0 Mask

T_OBCR          reg     R253            ; MFTimer Output B Control Register

op              equ     01h             ; Output Preset Bit Mask
oev             equ     02h             ; On Chip Event Bit on OVF/UDF Mask
ou_set          equ     00h             ; Overflow Underflow Set Mask
ou_tog          equ     04h             ; Overflow Underflow Toggle Mask
ou_res          equ     08h             ; Overflow Underflow Reset Mask
ou_nop          equ     0Ch             ; Overflow Underflow NOP Mask
c1_set          equ     00h             ; Compare 1 Set Mask
c1_tog          equ     10h             ; Compare 1 toggle Mask
c1_res          equ     20h             ; Compare 1 Reset Mask
c1_nop          equ     30h             ; Compare 1 NOP Mask
c0_set          equ     00h             ; Compare 0 Set Mask
c0_tog          equ     40h             ; Compare 0 Toggle Mask
c0_res          equ     80h             ; Compare 0 Reset Mask
c0_nop          equ     0C0h            ; Compare 0 NOP Mask

T_FLAGR         reg     R254            ; MFTimer Flags Register
                __defbit T_ao,T_FLAGR,0   ; AND/OR on Capture Interrupt
                __defbit T_ocm0,T_FLAGR,1 ; Overrun Compare 0
                __defbit T_ocp0,T_FLAGR,2 ; Overrun Capture 0
                __defbit T_ouf,T_FLAGR,3  ; Overflow Underflow Flag
                __defbit T_cm1,T_FLAGR,4  ; Successful Compare 1
                __defbit T_cm0,T_FLAGR,5  ; Successful Compare 0
                __defbit T_cp1,T_FLAGR,6  ; Successful Capture 1
                __defbit T_cp0,T_FLAGR,7  ; Successful Capture 0

T_IDMR          equ     R255            ; MFTimer Interrupt DMA Mask Register
                __defbit T_oui,T_IDMR,0   ; Overflow Underflow Interrupt
                __defbit T_cm1i,T_IDMR,1  ; Compare 1 Interrupt
                __defbit T_cm0i,T_IDMR,2  ; Compare 0 Interrupt
                __defbit T_cm0d,T_IDMR,3  ; Compare 0 DMA
                __defbit T_cp1i,T_IDMR,4  ; Capture 1 Interrupt
                __defbit T_cp0i,T_IDMR,5  ; Capture 0 Interrupt
                __defbit T_cp0d,T_IDMR,6  ; Capture 0 DMA
                __defbit T_gtien,T_IDMR,7 ; Global Timer Interrupt Enable

T0_DCPR         reg     R240            ; MFTimer 0 DMA Counter Pointer Register
T1_DCPR         reg     R244            ; MFTimer 1 DMA Counter Pointer Register
T0_DAPR         reg     R241            ; MFTimer 0 DMA Address Pointer Register
T1_DAPR         reg     R245            ; MFTimer 1 DMA Address Pointer Register
T0_IVR          reg     R242            ; MFTimer 0 Interrupt Vector Register
T1_IVR          reg     R246            ; MFTimer 1 Interrupt Vector Register
T0_IDCR         reg     R243            ; MFTimer 0 Interrupt/DMA Control Register
T1_IDCR         reg     R247            ; MFTimer 1 Interrupt/DMA Control Register
T2_DCPR         reg     R240            ; MFTimer 2 DMA Counter Pointer Register
T3_DCPR         reg     R244            ; MFTimer 3 DMA Counter Pointer Register
T2_DAPR         reg     R241            ; MFTimer 2 DMA Address Pointer Register
T3_DAPR         reg     R245            ; MFTimer 3 DMA Address Pointer Register
T2_IVR          reg     R242            ; MFTimer 2 Interrupt Vector Register
T3_IVR          reg     R246            ; MFTimer 3 Interrupt Vector Register
T2_IDCR         reg     R243            ; MFTimer 2 Interrupt/DMA Control Register
T3_IDCR         reg     R247            ; MFTimer 3 Interrupt/DMA Control Register

plm             equ     07h             ; Priority Level Mask
swen            equ     08h             ; Swap Function Enable Mask
dctd            equ     10h             ; DMA Compare Transaction Destination Mask
dcts            equ     20h             ; DMA Capture Transaction Source Mask
cme             equ     40h             ; Compare 0 End of Block Mask
cpe             equ     80h             ; Capture 0 End of Block Mask

T_IOCR          reg     R248            ; MFTimer I/O Connection Register

sc0             equ     01h             ; TxOUTA and TxINA Connection Bit (even)

sc1             equ     02h             ; TxOUTA and TxINA Connection Bit (odd)

;----------------------------------------------------------------------------
; AD Converter

                if      MOMCPUNAME<>"ST9020"

AD0_PG          equ     63              ; A/D Converter Registers Page
AD1_PG          equ     62              ; Second A/D unit

AD_D0R          reg     R240            ; Channel 0 Data Register
AD_D1R          reg     R241            ; Channel 1 Data Register
AD_D2R          reg     R242            ; Channel 2 Data Register
AD_D3R          reg     R243            ; Channel 3 Data Register
AD_D4R          reg     R244            ; Channel 4 Data Register
AD_D5R          reg     R245            ; Channel 5 Data Register
AD_D6R          reg     R246            ; Channel 6 Data Register
AD_D7R          reg     R247            ; Channel 7 Data Register
AD_LT6R         reg     R248            ; Channel 6 Lower Threshold Register
AD_LT7R         reg     R249            ; Channel 7 Lower Threshold Register
AD_UT6R         reg     R250            ; Channel 6 Upper Threshold Register
AD_UT7R         reg     R251            ; Channel 7 Upper Threshold Register

AD_CRR          reg     R252            ; Compare Result Register
                __defbit AD_c6l,AD_CRR,4 ; Compare Channel 6 Lower Bit
                __defbit AD_c7l,AD_CRR,5 ; Compare Channel 7 Lower Bit
                __defbit AD_c6u,AD_CRR,6 ; Compare Channel 6 Upper Bit
                __defbit AD_c7u,AD_CRR,7 ; Compare Channel 7 Upper Bit

AD_CLR          reg     R253            ; Control Logic Register
                __defbit AD_st,AD_CLR,0   ; Start/Stop Bit
                __defbit AD_cont,AD_CLR,1 ; Continuous Mode
                __defbit AD_pow,AD_CLR,2  ; Power Up/Down Control
                __defbit AD_intg,AD_CLR,3 ; Internal Trigger
                __defbit AD_extg,AD_CLR,4 ; External Trigger

sch             equ     0E0h            ; Scan Channel Selection Mask

AD_ICR          reg     R254            ; Interrupt Control Register
                __defbit AD_awdi,AD_ICR,4 ; Analog Watchdog Interrupt
                __defbit AD_eci,AD_ICR,5  ; End of Count Interrupt
                __defbit AD_awd,AD_ICR,6  ; Analog Watchdog Pending Flag
                __defbit AD_ecv,AD_ICR,7  ; End of conversion Pending Flag

AD_prl          equ     07h             ; Priority Level Mask

AD_IVR          reg     R255            ; Interrupt Vector Register

                endif

;----------------------------------------------------------------------------
; Serial Interface

SCI1_PG         equ     24              ; SCI1 Control Registers Page
SCI2_PG         equ     25              ; SCI2 Control Registers Page
SCI3_PG         equ     26              ; SCI3 Control Registers Page
SCI4_PG         equ     27              ; SCI4 Control Registers Page

S_RDCPR         reg     R240            ; Receive DMA Counter Pointer Register
S_RDAPR         reg     R241            ; Receive DMA Address Pointer Register
S_TDCPR         reg     R242            ; Transmit DMA Counter Pointer Register
S_TDAPR         reg     R243            ; Transmit DMA address Pointer Register
S_IVR           reg     R244            ; Interrupt Vector Register
S_ACR           reg     R245            ; Address Compare Register

S_IMR           reg     R246            ; Interrupt Mask Register
                __defbit S_txdi,S_IMR,0  ; Transmitter Data Interrupt
                __defbit S_rxdi,S_IMR,1  ; Receiver Data Interrupt
                __defbit S_rxb,S_IMR,2   ; Receiver Break
                __defbit S_rxa,S_IMR,3   ; Receiver Address
                __defbit S_rxe,S_IMR,4   ; Receiver Error
                __defbit S_txeob,S_IMR,5 ; Transmit End of Block
                __defbit S_rxeob,S_IMR,6 ; Receive End of Block
                __defbit S_hsn,S_IMR,7   ; Holding or Shift Register Empty

S_ISR           reg     R247            ; Interrupt Status Register
                __defbit S_txsem,S_ISR,0 ; Transmit Shift Register Empty
                __defbit S_txhem,S_ISR,1 ; Transmit Hold Register Empty
                __defbit S_rxdp,S_ISR,2  ; Received Data Pending Bit
                __defbit S_rxbp,S_ISR,3  ; Received Break Pending Bit
                __defbit S_rxap,S_ISR,4  ; Received Address Pending Bit
                __defbit S_pe,S_ISR,5    ; Parity Error Pending Bit
                __defbit S_fe,S_ISR,6    ; Framing Error Pending Bit
                __defbit S_oe,S_ISR,7    ; Overrun Error Pending Bit

S_RXBR          reg     R248            ; Receive Buffer Register
S_TXBR          reg     R248            ; Transmit Buffer Register

S_IDPR          reg     R249            ; Interrupt/DMA Priority Register
                __defbit S_txd,S_IDPR,3  ; Transmitter DMA
                __defbit S_rxd,S_IDPR,4  ; Receiver DMA
                __defbit S_sa,S_IDPR,5   ; Set Address
                __defbit S_sb,S_IDPR,6   ; Set Break
                __defbit S_amen,S_IDPR,7 ; Address Mode Enable
S_pri           equ     07h             ; Interrupt/DMA Priority Mask

S_CHCR          reg     R250            ; Character Configuration Register

wl5             equ     000h            ; 5 Bits Data Word Mask
wl6             equ     001h            ; 6 Bits Data Word Mask
wl7             equ     002h            ; 7 Bits Data Word Mask
wl8             equ     003h            ; 8 Bits Data Word Mask
sb10            equ     000h            ; 1.0 Stop Bit Mask
sb15            equ     004h            ; 1.5 Stop Bit Mask
sb20            equ     008h            ; 2.0 Stop Bit Mask
sb25            equ     00Ch            ; 2.5 Stop Bit Mask
ab              equ     010h            ; Address Bit Insertion Mask
pen             equ     020h            ; Parity Enable Mask
ep              equ     040h            ; Even Parity Mask
oddp            equ     000h            ; Odd Parity Mask
am              equ     080h            ; Address Mode Mask

S_CCR           reg     R251            ; Clock Configuration Register
                __defbit S_stpen,S_CCR,0 ; Stick Parity Enable
                __defbit S_lben,S_CCR,1  ; Loop Back Enable
                __defbit S_aen,S_CCR,2   ; Auto Echo Enable
                __defbit S_cd,S_CCR,3    ; Clock Divider
                __defbit S_xbrg,S_CCR,4  ; External Baud Rate Generator Source
                __defbit S_xrx,S_CCR,5   ; External Receiver Source
                __defbit S_oclk,S_CCR,6  ; Output Clock Selection
                __defbit S_txclk,S_CCR,7 ; Transmit Clock Selection

S_BRGR          reg     RR252           ; Baud Rate Generator Register
S_BRGHR         reg     R252            ; Baud Rate Generator Register High
S_BRGLR         reg     R253            ; Baud Rate Generator Register Low

;----------------------------------------------------------------------------
; Security Registers:

SEC_PG          equ     59              ; Security Register Page

SECR            reg     R255
                __defbit tlck,SECR,0  ; Test Lock Bit
                __defbit wf1,SECR,1   ; Write Fuse 1 Bit
                __defbit hlck,SECR,2  ; Hardware Lock Bit
                __defbit wf2,SECR,3   ; Write Fuse 2 Bit
                __defbit f2tst,SECR,4 ; Select Fuse 2 Bit
                __defbit slck,SECR,7  ; Software Lock Bit

;----------------------------------------------------------------------------

                endif			; regst9inc

                restore                 ; re-enable listing
