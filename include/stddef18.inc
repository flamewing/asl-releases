		ifndef  stddef18inc     ; avoid multiple inclusion
stddef18inc     equ     1

		save
		listing off		; no listing over this file
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF18.INC                                            *
;*   								            *
;*   Contains SFR Definitions for the PIC 16C8x Family                      *
;*   Macro Definitions, if needed, may be taken from STDDEF16.INC           *
;*									    *
;****************************************************************************

                if      (MOMCPU<93316)&&(MOMCPU<>93284)
                 fatal  "wrong target selected: only 16C84 or 16C64 allowed"
		endif

                if      MOMPASS=1
		 message "PIC16C8x Definitions (C) 1993 Alfred Arnold"
                 message "including \{MOMCPU} registers"
		endif

;----------------------------------------------------------------------------
; Special Purpose Registers

INDF            SFR     0               ; Indirekte Addressing
PCL             SFR     2               ; Program Couter LSB
PCLATH          SFR     10              ; Program Couter MSB
Status          SFR     3               ; Status Register
FSR             SFR     4               ; File Select Register
OPTION          SFR     $81             ; Chip Configuration
                if      MOMCPU=$16C64
PCON             SFR    $8e
                endif

;----------------------------------------------------------------------------
; Ports

PortA           SFR     5               ; Port A Data Register
TrisA           SFR     $85             ; Port A Data Direction Register
PortB           SFR     6               ; Port B Data Register
TrisB           SFR     $86             ; Port B Data Direction Register
                if      MOMCPU=$16C64
PortC            SFR    7               ; Port C Data Register
TrisC            SFR    $87             ; Port C Data Direction Register
PortD            SFR    8               ; Port D Data Register
TrisD            SFR    $88             ; Port D Data Direction Register
PortE            SFR    9               ; Port E Data Register
TrisE            SFR    $89             ; Port E Data Direction Register
                endif

;----------------------------------------------------------------------------
; Timer

                if      MOMCPU=$16C64
TMR0             SFR    1               ; Timer 0
TMR1L            SFR    14              ; Timer 1
TMR1H            SFR    15
TMR2             SFR    17              ; Timer 2
CCPR1L           SFR    $15             ; Compare / Pulse Width Register
CCPR1H           SFR    $16
T1CON            SFR    16              ; Timer 1 Control
T2CON            SFR    18              ; Timer 2 Control
PR2              SFR    $92             ; Timer 2 Period
CCP1CON          SFR    $17             ; Compare Mode
                elseif
RTCC             SFR    1               ; Timer
                endif

;----------------------------------------------------------------------------
; Serial Interface

                if      MOMCPU=$16C64
SSPBuf           SFR    $13             ; Serial Data Register
SSPCon           SFR    $14             ; Configuration
SSPAD0           SFR    $93             ; I2C Address Register
SSPSTAT          SFR    $94             ; Interface Status
                endif

;----------------------------------------------------------------------------
; EEPROM

                if      MOMCPU=$16C84
EEData           SFR    8               ; EEPROM Data Register
EEAdr            SFR    9               ; EEPROM Address Register
EECON1           SFR    $88             ; EEPROM Configuration
EECON2           SFR    $89
                endif

;----------------------------------------------------------------------------
; Interrupts

INTCON          SFR     11              ; Interrupt Control
                if      MOMCPU=$16C64
PIR              SFR    12              ; Interrupt Flags
PIE              SFR    $8c             ; Interrupt Enable
                endif

;============================================================================
; Status Register Bits

Flag_C		EQU	0		; Carry
Flag_DC		EQU	1		; Digit-Carry
Flag_Z		EQU	2		; Zero
Flag_PD		EQU	3		; Wake-Up from SLEEP Instruction
Flag_TO		EQU	4		; Watchdog Reset Occured
Flag_RP0	EQU	5		; Register Bank Bit 8
                if      MOMCPU=$16C84
Flag_RP1         EQU    6               ; Register Bank Bit 9
Flag_IRP         EQU    7               ; Register Bank Bit 9 (indirect)
                endif

;----------------------------------------------------------------------------
; Bits in TrisE

                if      MOMCPU=$16C64
TRISE0           EQU    0               ; RE0 Data Direction
TRISE1           EQU    1               ; RE1 Data Direction
TRISE2           EQU    2               ; RE2 Data Direction
PSPMODE          EQU    4               ; Port D&E Mode
IBOV             EQU    5               ; Input Buffer Overflow
OBF              EQU    6               ; Output Buffer Full
IBF              EQU    7               ; Input Buffer Full
                endif

;----------------------------------------------------------------------------
; Bits in T1CON

                if      MOMCPU=$16C64
TMR1ON           EQU    0               ; Timer 1 Start
TMR1CS           EQU    1               ; Timer 1 Internal/External Clock Select
T1INSYNC         EQU    2               ; Timer 1 External Synchronisation
T1OSCEN          EQU    3               ; Timer 1 Oscillator Enable
T1CKPS0          EQU    4               ; Timer 1 Prescaler Selection
T1CKPS1          EQU    5
                endif

; Bits in T2CON

                if      MOMCPU=$16C64
T2CKPS0          EQU    0               ; Timer 2 Prescaler Selection
T2CKPS1          EQU    1
TMR2ON           EQU    2               ; Timer 2 Start/Stop
TOUTPS0          EQU    3               ; Timer 2 Post-Scaler Selection
TOUTPS1          EQU    4
TOUTPS2          EQU    5
TOUTPS3          EQU    7
                endif

; Bits in CCP1CON

                if      MOMCPU=$16C64
CCP1M0           EQU    0               ; CCP1 Mode
CCP1M1           EQU    1
CCP1M2           EQU    2
CCP1M3           EQU    3
CCP1Y            EQU    4               ; PWM Bits 0,1
CCP1X            EQU    5
                endif

;----------------------------------------------------------------------------
; Bits in SSPSTAT

                if      MOMCPU=$16C64
BF               EQU    0               ; Buffer Full
UA               EQU    1               ; New Address must be written to SSPADD
RW               EQU    2               ; I2C Read/Write Select
S                EQU    3               ; Start Bit Detected
P                EQU    4               ; Stop Bit Detected
DA               EQU    5               ; I2C Datum/Adresse Received
                endif

; Bits in SSPCON

                if      MOMCPU=$16C64
SSPM0            EQU    0               ; Interface Mode
SSPM1            EQU    1
SSPM2            EQU    2
SSPM3            EQU    3
CKP              EQU    4               ; Clock Polarity
SSPEN            EQU    5               ; Port Enable
SSPOV            EQU    6               ; Receiver Overflow
WCOL             EQU    7               ; Transmitter Collision
                endif

;----------------------------------------------------------------------------
; Bits in INTCON

RBIF		EQU	0		; PB4..7 Level Change Indication
INTF		EQU	1		; Interrupt Flag
RTIF		EQU	2		; RTCC Interrupt
RBIE		EQU	3		; Bit 0 Interrupt Enable
INTE		EQU	4		; Bit 1 Interrupt Enable
RTIE		EQU	5		; Bit 2 Interrupt Enable
                if      MOMCPU=$16C84
EEIE             EQU    6               ; EEPROM Interrupt Enable
                elseif
PEIE             EQU    6               ; Peripheral Interrupt Enable
                endif
GIE		EQU	7		; Global Interrupt Enable

; Bits in PIE1

                if      MOMCPU=$16C64
TMR1IE           EQU    0               ; Timer 1 Interrupt Enable
TMR2IE           EQU    1               ; Timer 2 Interrupt Enable
CCP1IE           EQU    2               ; CCP1 Interrupt Enable
SSPIE            EQU    3               ; Serial Interface Interrupt Enable
PSPIE            EQU    7               ; Parallel Port Interrupt Enable
                endif

; Bits in PIR1

                if      MOMCPU=$16C64
TMR1IF           EQU    0               ; Timer 1 Interrupt Flag
TMR2IF           EQU    1               ; Timer 2 Interrupt Flag
CCP1IF           EQU    2               ; CCP1 Interrupt CCP1 Flag
SSPIF            EQU    3               ; Serial Interface Interrupt Flag
PSPIF            EQU    7               ; Parallel Port Interrupt Flag
                endif

;----------------------------------------------------------------------------
; Bits in OPTION

PS0		EQU	0		; Prescaler Select
PS1		EQU	1
PS2		EQU	2
PSA		EQU	3		; Prescaler Assignment
RTE		EQU	4		; RTCC Trigger Edge
RTS		EQU	5		; RTCC Signal Source
INTEDG		EQU	6		; INT TriggerEdge
RBPU		EQU	7		; Port B Pull-Up Enable

; Bits in PCON

                if      MOMCPU=$16C64
POR              EQU    1               ; Power On Reset Flag
                endif

;----------------------------------------------------------------------------
; Bits in EECON1

                if      MOMCPU=$16C84
RD               EQU    0               ; Start Read Cycle
WR               EQU    1               ; Start Write Cycle
WREN             EQU    2               ; Write Enable
WRERR            EQU    3               ; Write Error
EEIF             EQU    4               ; Write Cycle Completed
                endif

;----------------------------------------------------------------------------

		restore                 ; re-enable listing

		endif			; stddef18inc
