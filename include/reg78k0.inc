                ifndef  regz78k0inc     ; avoid multiple inclusion
reg8k0inc       equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG78K0.INC                                             *
;*   								            *
;*   Contains Register Definitions for 78K0 (uPD78070A)                     *
;*                                                                          *
;****************************************************************************

                if      (MOMCPUNAME<>"78070")
                 fatal  "wrong target selected: only 78070 supported"
		endif


                if      MOMPASS=1
                 message "78K0 Register Definitions (C) 1996 Alfred Arnold"
		endif

;----------------------------------------------------------------------------
; CPU Core

SP              equ     0ff1ch          ; Stack Pointer

PSW             equ     0ff1eh          ; Processor Status Word

;----------------------------------------------------------------------------
; Clock Generator

PCC             equ     0fffbh          ; Prozessor Clock Control

OSMS            equ     0fff2h          ; Clock Divider Enable/Disable

OSTS            equ     0fffah          ; Wait Time after STOP Resolution

;----------------------------------------------------------------------------
; Bus Interface

EBTS            equ     0ff3fh          ; Multiplex on/off (hardwired to off)

IMS             equ     0fff0h          ; Size of Internal RAM (hardwired to 1K)

MM              equ     0fff8h          ; Wait State Control

;----------------------------------------------------------------------------
; Interrupt Control

INTM0           equ     0ffech          ; INTP0...INTP2 Edge Selection
INTM1           equ     0ffedh          ; INTP3...INTP6 Edge Selection
SCS             equ     0ff47h          ; INTP0 Sample Rate Selection

IF0L            equ     0ffe0h          ; Interrupt Flags
IF0H            equ     0ffe1h
IF0             equ     IF0L
IF1L            equ     0ffe2h

MK0L            equ     0ffe4h          ; Mask Registers
MK0H            equ     0ffe5h
MK0             equ     IF0L
MK1L            equ     0ffe6h

PR0L            equ     0ffe8h          ; Priorities
PR0H            equ     0ffe9h
PR0             equ     IF0L
PR1L            equ     0ffeah

;----------------------------------------------------------------------------
; Ports

P0              equ     0ff00h          ; Port 0 Data Register
PM0             equ     0ff20h          ; Port 0 Data Direction Register

P1              equ     0ff01h          ; Port 1 Data Register
PM1             equ     0ff21h          ; Port 1 Data Direction Register

P2              equ     0ff02h          ; Port 2 Data Register
PM2             equ     0ff22h          ; Port 2 Data Direction Register

P3              equ     0ff03h          ; Port 3 Data Register
PM3             equ     0ff23h          ; Port 3 Data Direction Register

P6              equ     0ff06h          ; Port 6 Data Register
PM6             equ     0ff26h          ; Port 6 Data Direction Register

P7              equ     0ff07h          ; Port 7 Data Register
PM7             equ     0ff27h          ; Port 7 Data Direction Register

P9              equ     0ff09h          ; Port 9 Data Register
PM9             equ     0ff29h          ; Port 9 Data Direction Register

P10             equ     0ff0ah          ; Port 10 Data Register
PM10            equ     0ff2ah          ; Port 10 Data Direction Register

P12             equ     0ff0ch          ; Port 12 Data Register
PM12            equ     0ff2ch          ; Port 12 Data Direction Register

P13             equ     0ff0dh          ; Port 13 Data Register
PM13            equ     0ff2dh          ; Port 13 Data Direction Register

PUOH            equ     0fff3h          ; Port 0..7 Pull Up Register
PUOL            equ     0fff7h          ; Port 8..13 Pull Up Register

;----------------------------------------------------------------------------
; Timer

TM0             equ     0ff14h          ; Timer 0 Counter Value
CR00            equ     0ff10h          ; Timer 0 Capture/Compare Register 0
CR01            equ     0ff12h          ; Timer 0 Capture/Compare Register 1
TCL0            equ     0ff40h          ; Timer 0 Clock Select
TMC0            equ     0ff48h          ; Timer 0 Operation Mode
CRC0            equ     0ff4ch          ; Timer 0 Capture/Compare Register Control
TOC0            equ     0ff4eh          ; Timer 0 Output Control

TM1             equ     0ff18h          ; Timer 1 Counter Value
CR10            equ     0ff16h          ; Timer 1 Capture/Compare Register
TCL1            equ     0ff41h          ; Timer 1 Clock Select
TMC1            equ     0ff49h          ; Timer 1 Operation Mode

TM2             equ     0ff19h          ; Timer 2 Counter Value
CR20            equ     0ff17h          ; Timer 2 Capture/Compare Register
TCL2            equ     0ff42h          ; Timer 2 / RTC-Timer / Watchdog / Buzzer Clock Select
TMC2            equ     0ff4ah          ; Timer 2 / RTC-Timer Operation Mode

TMS             equ     TM1             ; Timer 1+2 Counter Value in 16 Bit Mode
TOC1            equ     0ff4fh          ; Timer 1+2 Output Control

TM5             equ     0ff51h          ; Timer 5 Counter Value
CR50            equ     0ff50h          ; Timer 5 Compare Register
TCL5            equ     0ff52h          ; Timer 5 Clock Select
TMC5            equ     0ff53h          ; Timer 5 Operation Mode

TM6             equ     0ff55h          ; Timer 6 Counter Value
CR60            equ     0ff54h          ; Timer 6 Compare Register
TCL6            equ     0ff56h          ; Timer 6 Clock Select
TMC6            equ     0ff57h          ; Timer 6 Operation Mode

;----------------------------------------------------------------------------
; Watchdog

WDTM            equ     0fff9h          ; Operation Mode

;----------------------------------------------------------------------------
; Serial Interface

; Kanal 0
SIO0            equ     0ff1ah          ; Data Register
SVA             equ     0ff62h          ; Slave Address
TCL3            equ     0ff43h          ; Channel 0/1 Clock Select
CSIM0           equ     0ff60h          ; Operation Mode
SBIC            equ     0ff61h          ; Bus Control
SINT            equ     0ff63h          ; Interrupt Timing

; Kanal 1
SIO1            equ     0ff1bh          ; Data Register
ADTP            equ     0ff6ah          ; Adress Pointer Block Reception/Transmission
CSIM1           equ     0ff68h          ; Operation Mode
ADTC            equ     0ff69h          ; Block Mode Control
ADTI            equ     0ff6bh          ; Block Mode Speed

; Kanal 2
TXS             equ     0ff74h          ; Data Register
RXB             equ     0ff74h
SIO2            equ     TXS
ASIM            equ     0ff70h          ; Interface Parameters
ASIS            equ     0ff71h          ; Status Register
CSIM2           equ     0ff72h          ; Operation Mode
BRGC            equ     0ff73h          ; Baud Rate Select

;----------------------------------------------------------------------------
; A/D Converter

ADCR            equ     0ff1fh          ; Conversion result Register

ADM             equ     0ff80h          ; Operation Mode

ADIS            equ     0ff84h          ; Port Pin Analog Input Select

;----------------------------------------------------------------------------
; D/A Converter

DACS0           equ     0ff90h          ; Converter 0 Analog Value

DACS1           equ     0ff91h          ; Converter 1 Analog Value

DAM             equ     0ff96h          ; Operation Mode

;----------------------------------------------------------------------------
; Real Time Output

RTBL            equ     0ff30h          ; Output Data
RTBH            equ     0ff31h
RTPM            equ     0ff34h          ; Operation Mode
RTPC            equ     0ff36h          ; Control

;----------------------------------------------------------------------------
; Vector Addresses

Vec_Reset       equ     0000h           ; Reset
Vec_INTWDT      equ     0004h           ; Watchdog
Vec_INTP0       equ     0006h           ; External Interrupts
Vec_INTP1       equ     0008h
Vec_INTP2       equ     000ah
Vec_INTP3       equ     000ch
Vec_INTP4       equ     000eh
Vec_INTP5       equ     0010h
Vec_INTP6       equ     0012h
Vec_INTCSI0     equ     0014h           ; Serial Interfaces
Vec_INTCSI1     equ     0016h
Vec_INTSER      equ     0018h
Vec_INTSR       equ     001ah
Vec_INTCSI2     equ     Vec_INTSR
Vec_INTST       equ     001ch
Vec_INTTM3      equ     001eh           ; RTC Timer
Vec_INTTM00     equ     0020h           ; Timer
Vec_INTTM01     equ     0022h
Vec_INTTM1      equ     0024h           ; Timer 1 Compare
Vec_INTTM2      equ     0026h           ; Timer 2 Compare
Vec_INTAD       equ     0028h           ; A/D Converter
Vec_INTTM5      equ     002ah           ; Timer 5 Compare
Vec_INTTM6      equ     002ch           ; Timer 6 Compare
Vec_BRK         equ     003eh           ; BRK Instruction

;----------------------------------------------------------------------------
; Memory Addresses

BUFRAM          equ     0fac0h          ; Buffer RAM
BUFRAMEND       equ     0fadfh

IRAM            equ     0fb00h          ; Internal RAM
IRAMEND         equ     0fedfh          ; Registers follow

;----------------------------------------------------------------------------

		restore                 ; allow listing again

                endif                   ; reg8k0inc



