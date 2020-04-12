		ifndef  stddef17inc     ; avoid multiple inclusion
stddef17inc     equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.41 - File STDDEF17.INC                                            *
;*   								            *
;*   Contains SFR Definitions for PIC 17C4x Family                          *
;*   Instruction macros may be taken from STDDEF16.INC.                     *
;*									    *
;****************************************************************************

                if      (MOMCPU<>97346)
		 fatal  "wrong target selected: only 17C42 supported"
		endif

                if      MOMPASS=1
		 message "PIC17C4x Definitions (C) 1993 Alfred Arnold"
		endif

;----------------------------------------------------------------------------
; CPU Registers

Indirect0       SFR     0               ; Indirect Addressing 1
FSR0            SFR     1
Indirect1       SFR     8               ; Indirect Addressing 2
FSR1            SFR     9
PCL             SFR     2               ; Program Counter LSB
PCLATH          SFR     3               ; Program Counter MSB
W               SFR     10              ; Accumulator
TBLPTRL         SFR     13              ; Registers to read from
TBLPTRH         SFR     14              ; ROM Area
ALUSTA          SFR     4               ; Flags, FSR Modes
CPUSTA          SFR     6               ; CPU State
INTSTA          SFR     7               ; Interrupt State

;------------------------------------------------------------------------------
; Counters

RTCSTA          SFR     5               ; Control Register
RTCCL           SFR     11              ; Data
RTCCH           SFR     12

;------------------------------------------------------------------------------

BSR             SFR     15              ; Remainder of Bank Selection

;------------------------------------------------------------------------------
; Interrupt Control

PIR             SFR     $16             ; Interrupt Requests
PIE             SFR     $17             ; Interrupt Enables

;------------------------------------------------------------------------------
; Ports

PORTA           SFR     $10             ; Data Registers
PORTB           SFR     $12
PORTC           SFR     $11
PORTD           SFR     $13
PORTE           SFR     $15

DDRB            SFR     $11             ; Direction Registers
DDRC            SFR     $10
DDRD            SFR     $12
DDRE            SFR     $14

;------------------------------------------------------------------------------
; Serial Port

RCSTA           SFR     $13             ; Receiver State
RCREG           SFR     $14             ; Receiver Data
TXSTA           SFR     $15             ; Transmitter State
TXREG           SFR     $16             ; Transmitter Data
SPBRG           SFR     $17             ; SIO Prescaler

;------------------------------------------------------------------------------
; Timers

TMR1            SFR     $10             ; Timer 1 Data
TMR2            SFR     $11             ; Timer 2 Data
TMR3L           SFR     $12             ; Timer 3 Data
TMR3H           SFR     $13

PR1             SFR     $14             ; Timer 1 Periode
PR2             SFR     $15             ; Timer 2 Periode
PR3L            SFR     $16             ; Timer 3 Periode
PR3H            SFR     $17

CA1L            SFR     $16             ; Capture Value 1
CA1H            SFR     $17
CA2L            SFR     $14             ; Capture Value 2
CA2H            SFR     $15

PW1DCL          SFR     $10             ; PWM 1 Data
PW1DCH          SFR     $12
PW2DCL          SFR     $11             ; PWM 2 Data
PW2DCH          SFR     $13

TCON1           SFR     $16             ; Control Register 1
TCON2           SFR     $17             ; Control Register 2

;------------------------------------------------------------------------------

                restore                 ; allow listing again

                endif			; stddef17inc
