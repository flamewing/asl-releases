                ifndef  stddef90inc     ; avoid multiple inclusion
stddef90inc     equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF90.INC                                            *
;*                                                                          *
;*   Contains Register and Memory Addresses for TLCS-90 Processors          *
;*                                                                          *
;****************************************************************************

                if      (MOMCPU<>9486657)
		 fatal  "Wrong processor type set: only 90C141 allowed"
                endif

                if      MOMPASS=1
                 message "TLCS-90 Address Definitions"
                 message "(C) 1993 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; Ports

P0              equ     0ffc0h
P1              equ     0ffc1h
P01CR           equ     0ffc2h  ; ==IRFL !!

P2              equ     0ffc4h
P2CR            equ     0ffc5h

P3              equ     0ffc6h
P3CR            equ     0ffc7h

P4              equ     0ffc8h
P4CR            equ     0ffc9h

P5              equ     0ffcah

P6              equ     0ffcch
P7              equ     0ffcdh
P67CR           equ     0ffceh

P8              equ     0ffd0h
P8CR            equ     0ffd1h

; Timers

TMOD            equ     0ffdah
TCLK            equ     0ffd8h
TRUN            equ     0ffdbh
TFFCR           equ     0ffd9h
T4MOD           equ     0ffe4h
T4FFCR          equ     0ffe5h
TREG0           equ     0ffd4h
TREG1           equ     0ffd5h
TREG2           equ     0ffd6h
TREG3           equ     0ffd7h
TREG4L          equ     0ffe0h
TREG4H          equ     0ffe1h
TREG5L          equ     0ffe2h
TREG5H          equ     0ffe3h
CAP1L           equ     0ffdch
CAP1H           equ     0ffddh
CAP2L           equ     0ffdeh
CAL2H           equ     0ffdfh

; Pattern

SMMOD           equ     0ffcbh
SMCR            equ     0ffcfh

; Serial

SCMOD           equ     0ffe9h  ; Operating Mode
SCCR            equ     0ffeah  ; Control Register
SCBUF           equ     0ffebh  ; Data Register

; Watchdog, Misc

BX              equ     0ffech  ; Bank-Registers IX
BY              equ     0ffedh  ;                IY

WDMOD           equ     0ffd2h  ; Watchdog, Halt Mode
WDCR            equ     0ffd3h  ; Control Register

; AD Converter

ADMOD           equ     0ffefh  ; Operating Mode
ADREG           equ     0ffeeh  ; Data Register

; Interrupts

IRFL            equ     0ffc2h  ; Request Flip Flops
IRFH            equ     0ffc3h
INTEL           equ     0ffe6h  ; Interrupt Enable
INTEH           equ     0ffe7h
DMAEH           equ     0ffe8h  ; Micro-DMA Enable

; Fixed Memory Areas

IRAM            equ     0fec0H    ; Internal RAM 256 Byte...
IRAMEND         equ     0ffbfH
IEAREA          equ     00000H    ; Interrupt Entry Area...
IEAREAEND       equ     0007FH    ; ...up to 007FH
IROM            equ     00000H    ; Internal (P)ROM 8K...
IROMEND         equ     01FFFH    ;
EXTMEM          equ     IROMEND+1 ; External Memory
MEMEND          equ     IRAM-1    ; ...up to FFFFH

;---------------------------------------------------------------------------

                restore		; allow listing again

                endif		; stddef90inc
