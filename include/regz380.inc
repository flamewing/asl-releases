                ifndef  regz380inc      ; avoid multiple inclusion
reg380inc       equ     1

                save
                listing off		; no listing over this file
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGZ380.INC                                             *
;*   								            *
;*   Contains Register Definitions for the Z380                             *
;*   These Registers may only be accessed via the instructions IN0, OUT0,   *
;*   and TSTIO.                                                             *
;*                                                                          *
;****************************************************************************

                if      (MOMCPU<>896)
                 fatal  "wrong target selected: only Z380 allowed"
		endif


                if      MOMPASS=1
                 message "Z380 Register Definitions (C) 1994 Alfred Arnold, Leonhard Schneider"
		endif

;----------------------------------------------------------------------------

LMCS0           port    00h             ; Lower Memory Chip Select Registers
LMCS1           port    01h
UMCS0           port    02h             ; Upper Memory Chip Select Registers
UMCS1           port    03h
MMCS0           port    04h             ; Midrange Memory Chip Select Registers
MMCS1           port    05h
MMCS2           port    06h
MMCS3           port    07h
LMWR            port    08h             ; Lower Memory Waits Register
UMWR            port    09h             ; Upper Memory Waits Register
MMWR0           port    0ah             ; Midrange Memory Waits Registers
MMWR1           port    0bh
MMWR2           port    0ch
MMWR3           port    0dh
IOWR            port    0eh             ; I/O Waits Register
RFWR            port    0fh             ; Refresh Waits Register
MSMER           port    10h             ; Memory Select Master Enable Register
IOCR0           port    11h             ; I/O Bus Control Registers
IOCR1           port    12h
RFSHR0          port    13h             ; Refresh Registers
RFSHR1          port    14h
RFSHR2          port    15h
SMCR            port    16h             ; Standby Mode Control Register
IER             port    17h             ; Interrupt Enable
AVBR            port    18h             ; Interrupt Vectors Offset
TRPBK           port    19h             ; indicates whether trap or break occured

CHIPVERSION     port    0ffh            ; Chip Version (00=Z380MPU)

;----------------------------------------------------------------------------

		restore                 ; re-allow listing

                endif                   ; reg380inc


