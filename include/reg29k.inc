                ifndef  reg29kinc       ; avoid multiple inclusion
reg29kinc       equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REG29K.INC                                              *
;*   								            *
;*   Contains Address Definitions for 2924x Processors                      *
;* 									    *
;****************************************************************************

                if      (MOMCPU<>168512)&&(MOMCPU<>168515)&&(MOMCPU<>168517)
                 fatal  "wrong target selected: only AM29240, AM29243 oder AM29245 supported"
		endif


                if      MOMPASS=1
                 message "AM2924x SFR Definitions (C) 1995 Alfred Arnold"
                 message "including AM\{MOMCPU} SFRs"
		endif

;----------------------------------------------------------------------------
; Register Base

RegBase         equ     0x80000000

;----------------------------------------------------------------------------
; ROM Controller

RMCT            equ     RegBase+0x00    ; ROM Control Register
RMCF            equ     RegBase+0x04    ; ROM Configuration Register

;----------------------------------------------------------------------------
; DRAM Controller

DRCT            equ     RegBase+0x08    ; DRAM Control Register
DRCF            equ     RegBase+0x0c    ; DRAM Configuration Register

;----------------------------------------------------------------------------
; PIA

PICT0           equ     RegBase+0x20    ; PIA Control Register 0
PICT1           equ     RegBase+0x24    ; PIA Control Register 1

;----------------------------------------------------------------------------
; DMA Controller

DMCT0           equ     RegBase+0x30    ; Channel 0 Control Register
DMAD0           equ     RegBase+0x34    ; Channel 0 Address Register
TAD0            equ     RegBase+0x70    ; Channel 0 Queued Address Register
DMCN0           equ     RegBase+0x38    ; Channel 0 Count Register
TCN0            equ     RegBase+0x3c    ; Channel 0 Queued Count Register
DMCT1           equ     RegBase+0x40    ; Channel 1 Control Register
DMAD1           equ     RegBase+0x44    ; Channel 1 Address Register
TAD1            equ     RegBase+0x74    ; Channel 1 Queued-Address Register
DMCN1           equ     RegBase+0x48    ; Channel 1 Count Register
TCN1            equ     RegBase+0x4c    ; Channel 1 Queued-Count Register
                if      MOMCPU<>0x29245
DMCT2            equ     RegBase+0x50   ; Channel 2 Control Register
DMAD2            equ     RegBase+0x54   ; Channel 2 Address Register
TAD2             equ     RegBase+0x78   ; Channel 2 Queued-Address Register
DMCN2            equ     RegBase+0x58   ; Channel 2 Count Register
TCN2             equ     RegBase+0x5c   ; Channel 2 Queued-Count Register
DMCT3            equ     RegBase+0x60   ; Channel 3 Control Register
DMAD3            equ     RegBase+0x64   ; Channel 3 Address Register
TAD3             equ     RegBase+0x7c   ; Channel 3 Queued-Address Register
DMCN3            equ     RegBase+0x68   ; Channel 3 Count Register
TCN3             equ     RegBase+0x6c   ; Channel 3 Queued-Count Register
                endif

;----------------------------------------------------------------------------
; PIO

POCT            equ     RegBase+0xd0    ; PIO Control Register
PIN             equ     RegBase+0xd4    ; PIO Input Register
POUT            equ     RegBase+0xd8    ; PIO Output Register
POEN            equ     RegBase+0xdc    ; PIO Direction Control

;----------------------------------------------------------------------------
; Parallelport

PPCT            equ     RegBase+0xc0    ; Control Register
PPST            equ     RegBase+0xc8    ; Status Register
PPDT            equ     RegBase+0xc4    ; Data Register

;----------------------------------------------------------------------------
; Serial Ports

SPCTA           equ     RegBase+0x80    ; Channel A Control Register
SPSTA           equ     RegBase+0x84    ; Channel A Status Register
SPTHA           equ     RegBase+0x88    ; Channel A Transmit Register
SPRBA           equ     RegBase+0x8c    ; Channel A Receive Register
BAUDA           equ     RegBase+0x90    ; Channel A Baud Rate Register
                if      MOMCPU<>0x29245
SPCTB            equ     RegBase+0xa0   ; Channel B Control Register
SPSTB            equ     RegBase+0xa4   ; Channel B Status Register
SPTHB            equ     RegBase+0xa8   ; Channel B Transmit Register
SPRBB            equ     RegBase+0xac   ; Channel B Receive Register
BAUDB            equ     RegBase+0xb0   ; Channel B Baud Rate Register
                endif

;----------------------------------------------------------------------------
; Video Interface

                if      MOMCPU<>0x29243
VCT              equ     RegBase+0xe0   ; Control Register
TOP              equ     RegBase+0xe4   ; Line Number Upper Border
SIDE             equ     RegBase+0xe8   ; Column Number Left/Right Border
VDT              equ     RegBase+0xec   ; Data Register
                endif

;----------------------------------------------------------------------------
; Interrupt Control

ICT             equ     RegBase+0x28    ; Control Register
IMASK           equ     RegBase+0x2c    ; Mask Register

;----------------------------------------------------------------------------

                endif

                restore
