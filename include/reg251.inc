                ifndef  reg251inc       ; avoid multiple inclusion
reg251inc       equ     1

                save
		listing off		; no listing over this file
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG251.INC                                              *
;*   								            *
;*   Contains SFR/Bit Defintions for MCS-251 Processors                     *
;*                                                                          *
;****************************************************************************

                if      (MOMCPUNAME<>"80C251")&&(MOMCPUNAME<>"80C251T")
                 fatal  "wrong target selected: only 80C251(T) allowed!"
		endif


                if      MOMPASS=1
                 message "MCS-251 SFR Definitions (C) 2003 Alfred Arnold"
		 message "including \{MOMCPUNAME}-SFRsn"
		endif

;----------------------------------------------------------------------------
; CPU Core:
; ACC = A = R11
; B = R10
; SP/SPH = SPX = DR60
; DPL/DPH/DPXL = DPX = DR56
; CY is being used by AS itself

ACC             port    0e0h            ; Accumulator

B               port    0f0h            ; B Register

PSW             port    0d0h            ; 8051-Compatible PSW
CARRY           bit     PSW.7           ; Carry
AC              bit     PSW.6           ; Auxiliary Carry
F0              bit     PSW.5           ; Flag 0
RS1             bit     PSW.4           ; Register Bank Select
RS0             bit     PSW.3
OV              bit     PSW.2           ; Overflow Flag
UD              bit     PSW.1           ; User Flag
P               bit     PSW.0           ; Parity Flag

PSW1            port    0d1h            ; Extended PSW (bits 2-4,6-7 as in PSW)
N               bit     PSW1.5          ; Negative Result
Z               bit     PSW1.1          ; Zero Result

SP              port    81h             ; Stack Pointer (Bits 0..7)
SPH             port    0beh            ; Stack Pointer (Bits 8..15)
DPL             port    082h            ; Data Pointer (Bits 0..7)
DPH             port    083h            ; Data Pointer (Bits 8..15)
DPXL            port    084h            ; Data Pointer (Bits 15..23)

PCON            port    087h            ; Power Down Modes etc.
SMOD1           bit     PCON.7          ; Doubles baud Rate in Modes 1..3
SMOD0           bit     PCON.6          ; Toggles SCON.7 as FE/SM0
POF             bit     PCON.4          ; Power Outage
GF1             bit     PCON.3          ; General Flag 1
GF0             bit     PCON.2          ; General Flag 0
PD              bit     PCON.1          ; Switch to Power Down Mode
IDL             bit     PCON.0          ; Switch to Idle Mode

IE              port    0a8h            ; Interrupt Enables (should be IE0,
                                        ; but that is already occupied)
EA              bit     IE.7            ; General Interrupt Mask
EC              bit     IE.6            ; Enable PCA Interrupts
ET2             bit     IE.5            ; Enable Timer 2 Interrupts
ES              bit     IE.4            ; Enable Serial Port Interrupts
ET1             bit     IE.3            ; Enable Timer 1 Interrupts
EX1             bit     IE.2            ; Enable External Interrupt 1
ET0             bit     IE.1            ; Enable Timer 0 Interrupts
EX0             bit     IE.0            ; Enable External Interrupt 0

IPH0            port    0b7h            ; Interrupt Priorities
IPL0            port    0b8h

                if      MOMCPUNAME="80C251T"
IPH1             port    0b3h
IPL1             port    0b2h
IE01             port    0b1h           ; It's called IE1 in the manual but
                                        ; that's the name of a bit elsewhere;
                                        ; Should probably find a better name
                                        ; for this one, or change IE1
		endif


;----------------------------------------------------------------------------
; Ports with extra functions:

P0              port    080h            ; Port 0

P1              port    090h            ; Port 1
T2              bit     P1.0            ; In/Output Timer 2
T2EX            bit     P1.1            ; Trigger Timer 2
ECI             bit     P1.2            ; External PCA Clock
CEX0            bit     P1.3            ; In/Output PCA Module 0
CEX1            bit     P1.4            ; In/Output PCA Module 1
CEX2            bit     P1.5            ; In/Output PCA Module 2
CEX3            bit     P1.6            ; In/Output PCA Module 3
CEX4            bit     P1.7            ; In/Output PCA Module 4

P2              port    0a0h            ; Port 2

P3              port    0b0h            ; Port 3
RXD             bit     P3.0            ; Serial Receive Line
TXD             bit     P3.1            ; Serial Transmit Line
INT0            bit     P3.2            ; External Interrupt 0
INT1            bit     P3.3            ; External Interrupt 1
T0              bit     P3.4            ; Clock input Timer 0
T1              bit     P3.5            ; Clock input Timer 1
WR              bit     P3.6            ; Read Line
RD              bit     P3.7            ; Write Line

;----------------------------------------------------------------------------
; Serial Port:

SCON            port    098h            ; Configuration
FE              bit     SCON.7          ; Framing Error
SM0             bit     SCON.7          ; Mode Selection
SM1             bit     SCON.6
SM2             bit     SCON.5
REN             bit     SCON.4          ; Enable Receiver
TB8             bit     SCON.3          ; Transmit Data bit 8
RB8             bit     SCON.2          ; Receive Data bit 8
TI              bit     SCON.1          ; Character Sent Completely
RI              bit     SCON.0          ; Character Received Completely

SBUF            port    099h            ; Data Register
SADEN           port    0b9h            ; Slave Address Mask
SADDR           port    0a9h            ; Slave Address

                if      MOMCPUNAME="80C251T"
;----------------------------------------------------------------------------
; Second Serial Port:

SADEN1           port    0bah            ; Slave Address Mask
SADDR1           port    0aah            ; Slave Address
SCON1            port    09ah            ; Configuration, similar to SCON
SBUF1            port    09bh            ; Data Register
BGCON            port    08fh
                endif

;----------------------------------------------------------------------------
; Counters/Watchdog:

TL0             port    08ah            ; Counter Value Counter 0
TH0             port    08ch
TL1             port    08bh            ; Counter Value Counter 1
TH1             port    08dh

TCON            port    088h            ; Counter 0/1 Control
TF1             bit     TCON.7          ; Timer 1 Overflow
TR1             bit     TCON.6          ; Timer 1 Start/Stop
TF0             bit     TCON.5          ; Timer 0 Overflow
TR0             bit     TCON.4          ; Timer 0 Start/Stop
IE1             bit     TCON.3          ; External Interrupt 1 occured
IT1             bit     TCON.2          ; External Interrupt 1 Edge/Level Trigger
IE0             bit     TCON.1          ; External Interrupt 0 occured
IT0             bit     TCON.0          ; External Interrupt 0 Edge/Level Trigger

TMOD            port    089h            ; Counter 0/1 Control/Mode
M00             bit     TMOD.0          ; Timer 0 Mode
M10             bit     TMOD.1
CT0             bit     TMOD.2          ; Timer 0 Counter/Timer Select
GATE0           bit     TMOD.3          ; Timer 0 Enable Mode
M01             bit     TMOD.4          ; Timer 1 Mode
M11             bit     TMOD.5
CT1             bit     TMOD.6          ; Timer 1 Counter/Timer Select
GATE1           bit     TMOD.7          ; Timer 1 Enable Mode


TL2             port    0cch            ; Counter 2 Counter Value
TH2             port    0cdh

T2CON           port    0c8h            ; Counter 2 Control
TF2             bit     T2CON.7         ; Timer 2 Overflow
EXF2            bit     T2CON.6         ; Edge Occured on T2EX
RCLK            bit     T2CON.5         ; Use Timer 2 or 1 for Serial Receive Clock
TCLK            bit     T2CON.4         ; Use Timer 2 or 1 for Serial Transmit Clock
EXEN2           bit     T2CON.3         ; Enable Capture via T2EX
TR2             bit     T2CON.2         ; Timer 2 Start/Stop
CT2             bit     T2CON.1         ; Timer 2 Counter/Timer Select
CPRL2           bit     T2CON.0         ; Capture or reload via T2EX

T2MOD           port    0c9h            ; Counter 2 Control/Mode
T2OE            bit     T2MOD.1         ; Timer 2 Enable Output on T2
DCEN            bit     T2MOD.0         ; Timer 2 Counting Direction

RCAP2L          port    0cah            ; Timer 2 Reload/Capture Value
RCAP2H          port    0cbh

WDTRST          port    0a6h            ; Reset Watchdog
WCON            port    0a7h            ; Watchdog Control

;----------------------------------------------------------------------------
; Counter Field:

CCON            port    0d8h            ; Control Register
CF              bit     CCON.7          ; PCA Overflow
CR              bit     CCON.6          ; Start/Stop PCA Timer
CCF4            bit     CCON.4          ; PCA Module 4..0 Trigger Flags
CCF3            bit     CCON.3
CCF2            bit     CCON.2
CCF1            bit     CCON.1
CCF0            bit     CCON.0

CMOD            port    0d9h            ; Mode Register
CIDL            bit     CMOD.7          ; Turn off PCA during Idle Mode
WDTE            bit     CMOD.6          ; Enable Watchdog Output on PCA Module 4
CPS1            bit     CMOD.2          ; PCA Clock Select
CPS0            bit     CMOD.1
ECF             bit     CMOD.0          ; Interrupt Enable

CL              port    0e9h            ; Counter Value
CH              port    0f9h

__defpcamodule  macro   Offset,NUM
CCAPM{NUM}      port    0d0h+Offset     ; Mode Register
ECOM{NUM}       bit     CCAPM0.6        ; Enable Comparator Module x
CAPP{NUM}       bit     CCAPM0.5        ; Enable Capture Function Module x (Positive Edge)
CAPN{NUM}       bit     CCAPM0.4        ; Enable Capture Function Module x (Negative Edge)
MAT{NUM}        bit     CCAPM0.3        ; Enable Equality Interrupt Module x
TOG{NUM}        bit     CCAPM0.2        ; Enable High-Speed Output Mode Module x
PWM{NUM}        bit     CCAPM0.1        ; Enable PWM Mode Module x
ECCF{NUM}       bit     CCAPM0.0        ; Enable Interrupts via CCFx
CCAP{NUM}L      port    0e0h+Offset     ; Compare/Capture Value Module x
CCAP{NUM}H      port    0f0h+Offset
                endm

                __defpcamodule 0ah,"0"
                __defpcamodule 0bh,"1"
                __defpcamodule 0ch,"2"
                __defpcamodule 0dh,"3"
                __defpcamodule 0eh,"4"

;----------------------------------------------------------------------------

                restore                 ; enable listing again

                endif			; reg251inc
