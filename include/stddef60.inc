                ifndef  stddef60inc     ; avoid multiple inclusion
stddef60inc     equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - STDDEF60.INC                                                 *
;*   								            *
;*   Contains Macro Definitions for PowerPC                                 *
;* 									    *
;****************************************************************************

                if      MOMPASS=1
                 message "PowerPC Macro Definitions (C) 1994,2012 Alfred Arnold"
                 switch  MOMCPUNAME
                 case    "RS6000"
                  message "Target System RS6000"
                 case    "MPC601"
                  message "Target System MPC601"
                 case    "MPC505"
                  message "Target System MPC505"
                 case    "PPC403"
                  message "Target System PPC403"
                 case    "PPC403GC"
                  message "Target System PPC403GC"
                 case    "MPC821"
                  message "Target System MPC821"
                 elsecase
                  fatal  "Wrong target selected: only MPC601, MPC505, PPC403[GC], MPC821 or RS6000 allowed"
                 endcase
		endif

		
;============================================================================
; Device Control Register

__defdcr        macro   NAME,val,{NoExpand}
NAME            equ     val
mt{"NAME"}      macro      reg
                mtdcr   NAME,reg
                endm
mf{"NAME"}      macro      reg
                mfdcr   reg,NAME
                endm
		endm

                if      (MOMCPU=0x403)||(MOMCPU=0x403c)
                 __defdcr BEAR,0x90     ; Bus Error Address
                 __defdcr BESR,0x91     ; Bus Error Syndrome
                 __defdcr BR0,0x80      ; Bank Registers 0..7
                 __defdcr BR1,0x81
                 __defdcr BR2,0x82
                 __defdcr BR3,0x83
                 __defdcr BR4,0x84
                 __defdcr BR5,0x85
                 __defdcr BR6,0x86
                 __defdcr BR7,0x87
                 __defdcr DMACC0,0xc4   ; DMA Chain Counter
                 __defdcr DMACR0,0xc0   ; DMA Control Register Channel 0..3
                 __defdcr DMACR1,0xc8
                 __defdcr DMACR2,0xd0
                 __defdcr DMACR3,0xd8
                 __defdcr DMACT0,0xc1   ; DMA Count Register Channel 0..3
                 __defdcr DMACT1,0xc9
                 __defdcr DMACT2,0xd1
                 __defdcr DMACT3,0xd9
                 __defdcr DMADA0,0xc2   ; DMA Target Address Channel 0..3
                 __defdcr DMADA1,0xca
                 __defdcr DMADA2,0xd2
                 __defdcr DMADA3,0xda
                 __defdcr DMASA0,0xc3   ; DMA Source Address Channel 0..3
                 __defdcr DMASA1,0xcb
                 __defdcr DMASA2,0xd3
                 __defdcr DMASA3,0xdb
                 __defdcr DMASR,0xe0    ; DMA Status Register
                 __defdcr EXISR,0x40    ; External Interrupt Status Register
                 __defdcr EXIER,0x42    ; External Interrupt Enable Register
                 __defdcr IOCR,0xa0     ; I/O Configuration
                endif

;============================================================================
; Special Purpose Registers

__defspr        macro   NAME,val,{NoExpand}
NAME            equ     val
mt{"NAME"}      macro      reg
                mtspr   NAME,reg
                endm
mf{"NAME"}      macro      reg
                mfspr   reg,NAME
                endm
		endm

                __defspr XER,0x001      ; Integer Exception Register
                __defspr LR,0x008       ; Link Register
                __defspr CTR,0x009      ; Count Register
                __defspr SRR0,0x01a     ; Save/Restore Register
                __defspr SRR1,0x01b
                __defspr SPRG0,0x110    ; Special Purpose Register
                __defspr SPRG1,0x111
                __defspr SPRG2,0x112
                __defspr SPRG3,0x113
                switch MOMCPU
                case 0x403,0x403c
                 __defspr DAC1,0x3f6    ; Data Address Compare Register
                 __defspr DAC2,0x3f7
                 __defspr DBCR,0x3f2    ; Debug Control Register
                 __defspr DBSR,0x3f0    ; Debug Status Register
                 __defspr DCCR,0x3fa    ; Data Cache Control Register
                 __defspr DEAR,0x3d5    ; Data Exception Addresse Register
                 __defspr ESR,0x3d4     ; Exception Syndrome Datea Access
                 __defspr EVPR,0x3d6    ; Exception Vector Prefix
                 __defspr IAC1,0x3f4    ; Code Address Compare Register
                 __defspr IAC2,0x3f5
                 __defspr ICCR,0x3fb    ; Instruction Cache Control Register
                 __defspr PBL1,0x3fc    ; Lower Bounds
                 __defspr PBL2,0x3fe
                 __defspr PBU1,0x3fd    ; Upper Bounds
                 __defspr PBU2,0x3ff
                 __defspr PIT,0x3db     ; Timer
                 __defspr PVR,0x11f     ; Processor Version
                 __defspr SRR2,0x3de    ; Save Restore Registers
                 __defspr SRR3,0x3df
                 __defspr TBHI,0x3dc    ; Time Base
                 __defspr TBLO,0x3dd
                 __defspr TCR,0x3da     ; Timer Control Register
                 __defspr TSR,0x3d8     ; Timer Status Register
                 __defspr SGR,0x3b9     ; ????
                case 0x505
                 __defspr TBL,268       ; Time Base
                 __defspr TBU,269
                 __defspr DSISR,18      ; Reason for Alignment Exceptions
                 __defspr DAR,19        ; Erroneous Data Address after Exception
                 __defspr DEC,22        ; counts @ 1 MHz
                 __defspr EIE,80        ; External Interrupt Enable
                 __defspr EID,81        ; External Interrupt Disable
                 __defspr NRE,82        ; Non Recoverable Exception
                 __defspr TBL_S,284     ; another Time Base ?!
                 __defspr TBU_S,285
                 __defspr PVR,287       ; Processor Version
                 __defspr ICCST,560     ; Instruction Cache Control & Status
                 __defspr ICADR,561     ; Instruktion Cache Address Register
                 __defspr ICDAT,562     ; Instruktion Cache Data Register
                 __defspr FPECR,1022    ; Floating Point Exception
                 __defspr CMPA,144      ; Comparison Value A..D
                 __defspr CMPB,145
                 __defspr CMPC,146
                 __defspr CMPD,147
                 __defspr ECR,148       ; Debug Exception Cause
                 __defspr DER,149       ; Debug Enable Register
                 __defspr COUNTA,150    ; Breakpoint Counter
                 __defspr COUNTB,151
                 __defspr CMPE,152      ; Comparison Value  E..H
                 __defspr CMPF,153
                 __defspr CMPG,154
                 __defspr CMPH,155
                 __defspr LCTRL1,156    ; Debug Control Comparator L Bus
                 __defspr LCTRL2,157
                 __defspr ICTRL,158     ; Debug Control I-Bus
                 __defspr BAR,159       ; Breakpoint Address
                 __defspr DPDR,630      ; Development Port Data
                 __defspr DPIR,631      ;     "        "   Instructions
                case 0x601
                 __defspr RTCU,0x004    ; Counter
                 __defspr RTCL,0x005
                 __defspr DEC,0x006
                 __defspr DSISR,0x012
                 __defspr DAR,0x013
                 __defspr DEC2,0x016
                 __defspr SDR1,0x019
                 __defspr EAR,0x11a
                 __defspr BAT0U,0x210
                 __defspr BAT0L,0x211
                 __defspr BAT1U,0x212
                 __defspr BAT1L,0x213
                 __defspr BAT2U,0x214
                 __defspr BAT2L,0x215
                 __defspr BAT3U,0x216
                 __defspr BAT3L,0x217
                 __defspr HID0,0x3f0
                 __defspr HID1,0x3f1
                 __defspr HID2,0x3f2
                 __defspr HID5,0x3f5
                 __defspr HID15,0x3ff
                case 0x6000
                 __defspr MQ,0x000      ; Upper Half Divident/Product
                case 0x821
                 __defspr EIE,80        ; External Interrupts Enable
                 __defspr EID,81        ; External Interrupts Disable
                 __defspr NRI,82        ; Non Recoverable Exception
                 __defspr NRE,82        ; (alias)
                 __defspr CMPA,144      ; Comparison Value A..D
                 __defspr CMPB,145
                 __defspr CMPC,146
                 __defspr CMPD,147
                 __defspr ICR,148       ; Debug Exception Cause
                 __defspr ECR,148       ; (alias)
                 __defspr DER,149       ; Debug Feature Enable
                 __defspr COUNTA,150    ; Breakpoint Counter
                 __defspr COUNTB,151
                 __defspr CMPE,152      ; Comparison Value E..H
                 __defspr CMPF,153
                 __defspr CMPG,154
                 __defspr CMPH,155
                 __defspr LCTRL1,156    ; Debug Control Comparator L-Bus
                 __defspr LCTRL2,157
                 __defspr ICTRL,158     ; Debug Control I-Bus
                 __defspr BAR,159       ; Breakpoint Address
                 __defspr DPDR,630      ; Development Port Data
                 __defspr DPIR,631      ;     "        "   Instructions
                 __defspr IMMR,638
                 __defspr IC_CST,560    ; Instruktion Cache Control & Status Control & Status
                 __defspr ICCST,560     ; (alias)
                 __defspr ICCSR,560     ; (alias)
                 __defspr IC_ADR,561    ; Instruktion Cache Control & Status Address Register
                 __defspr ICADR,561     ; (alias)
                 __defspr IC_DAT,562    ; Instruktion Cache Control & Status Data Register
                 __defspr ICDAT,562     ; (alias)
                 __defspr DC_CST,568    ; Steuerung & Status Data Cache
                 __defspr DCCST,568     ; (alias)
                 __defspr DCCSR,568     ; (alias)
                 __defspr DC_ADR,569    ; Data Cache Address Register
                 __defspr DCADR,569     ; (alias)
                 __defspr DC_DAT,570    ; Data Cache Data Register
                 __defspr DCDAT,570     ; (alias)
                 __defspr MI_CTR,784
                 __defspr MICTR,784
                 __defspr MI_AP,786
                 __defspr MIAP,786
                 __defspr MI_EPN,787
                 __defspr MIEPN,787
                 __defspr MI_TWC,789
                 __defspr MITWC,789
                 __defspr MIL1DL2P,789  ; (alias)
                 __defspr MI_RPN,790
                 __defspr MIRPN,790
                 __defspr MI_DBCAM,816
                 __defspr MICAM,816
                 __defspr MI_DBRAM0,817
                 __defspr MIRAM0,817
                 __defspr MI_DBRAM1,818
                 __defspr MIRAM1,818
                 __defspr MD_CTR,792
                 __defspr MDCTR1,792
                 __defspr M_CASID,793
                 __defspr MCASID,793
                 __defspr MD_AP,794
                 __defspr MDAP,794
                 __defspr MD_EPN,795
                 __defspr MDEPN,795
                 __defspr M_TWP,796
                 __defspr MDTWB,796
                 __defspr MDL1P,796     ; (alias)
                 __defspr MD_TWC,797
                 __defspr MDTWC,797
                 __defspr MDL1DL2P,797  ; (alias)
                 __defspr MD_RPN,798
                 __defspr MDRPN,798
                 __defspr M_TW,799
                 __defspr MDTW,799
                 __defspr MDSAVE,799   ; (alias)
                 __defspr MD_DBCAM,824
                 __defspr MDCAM,824
                 __defspr MD_DBRAM0,825
                 __defspr MDRAM0,825
                 __defspr MD_DBRAM1,826
                 __defspr MDRAM1,826
                endcase
                if MOMCPU=0x403c
                 __defspr pid, 0x3b1
                endif

;============================================================================
; Serial Port PPC403:

                if      (MOMCPU=0x403)||(MOMCPU=0x403c)
spls             equ     0x40000000     ; Line State
sphs             equ     0x40000002     ; State of Handshake Lines
brdh             equ     0x40000004     ; Baud Raten Divider
brdl             equ     0x40000005
spctl            equ     0x40000006     ; Control Register
sprc             equ     0x40000007     ; Command Register Receiver
sptc             equ     0x40000008     ; Command Register Transmitter
sprb             equ     0x40000009     ; Transmit/Receive Buffer
sptb             equ     sprb
                endif

;============================================================================
; SIU MPC505:
; erinnert irgendwie an die vom 6833x...

                if      MOMCPU=0x505
siumcr           equ     0x8007fc00     ; Base Control Register
siutest1         equ     0x8007fc04
memmap           equ     0x8007fc20     ; Memory Layout
specaddr         equ     0x8007fc24     ; Allow/Block Speculative Loads
specmask         equ     0x8007fc28
termstat         equ     0x8007fc2c
picsr            equ     0x8007fc40     ; Periodic Interrupts Control
pit              equ     0x8007fc44     ; Periodic Interrupt Timer Count Value
bmcr             equ     0x8007fc48     ; Bus Monitor Control
rsr              equ     0x8007fc4c     ; Reset Status
sccr             equ     0x8007fc50     ; System Clock Control
sccsr            equ     0x8007fc54     ; System Clock Status
portbase         equ     0x8007fc60
ddrm             equ     portbase+0x00  ; Data Direction Register Port M
pmpar            equ     portbase+0x04  ; Pin Assignment Port M
portm            equ     portbase+0x08  ; Data Register Port M
papar            equ     portbase+0x24  ; Pin Assignment Port A+B
pbpar            equ     papar
porta            equ     portbase+0x28  ; Data Register Port A+B
portb            equ     porta
ddri             equ     portbase+0x38  ; Data Direction Register Port I..L
ddrj             equ     ddri
ddrk             equ     ddri
ddrl             equ     ddri
pipar            equ     portbase+0x38  ; Pin Assignment Port I..L
pjpar            equ     pipar
pkpar            equ     pipar
plpar            equ     pipar
porti            equ     portbase+0x40  ; Data Register Port I..L
portj            equ     porti
portk            equ     porti
portl            equ     porti
csbase           equ     0x8007fd00
csbtbar          equ     csbase+0xf8    ; Base Address Boot EPROM
csbtsbbar        equ     csbase+0xf0
csbar1           equ     csbase+0xe0    ; Base Addresses /CS1../CS5
csbar2           equ     csbase+0xd8
csbar3           equ     csbase+0xd0
csbar4           equ     csbase+0xc8
csbar5           equ     csbase+0xc0
csbtor           equ     csbase+0xfc    ; Boot EPROM Options
csbtsbor         equ     csbase+0xf4
csor0            equ     csbase+0xec    ; Options /CS1../CS11
csor1            equ     csbase+0xe4
csor2            equ     csbase+0xdc
csor3            equ     csbase+0xd4
csor4            equ     csbase+0xcc
csor5            equ     csbase+0xc4
csor6            equ     csbase+0xbc
csor7            equ     csbase+0xb4
csor8            equ     csbase+0xac
csor9            equ     csbase+0xa4
csor10           equ     csbase+0x9c
csor11           equ     csbase+0x94
                endif

;----------------------------------------------------------------------------
; Peripheral Control Unit MPC505

                if      MOMCPU=0x505
pcubase          equ     0x8007ef80
pcumcr           equ     pcubase+0x00   ; Base Configuration
tstmsra          equ     pcubase+0x10
tstmsrb          equ     tstmsra
tstcntrab        equ     pcubase+0x14
tstreps          equ     tstcntrab
tstcreg1         equ     pcubase+0x18
tstcreg2         equ     tstcreg1
tstdreg          equ     pcubase+0x1c
irqpend          equ     pcubase+0x20   ; Pending Interrupts
irqand           equ     pcubase+0x24   ; Enabled & Pending Interrupts
irqenable        equ     pcubase+0x28   ; Enabled Interrupts
pitqil           equ     pcubase+0x2c   ; Interrupt Level PortQ/PIT
swsr             equ     pcubase+0x40   ; Trigger Reload Watchdog
swcr             equ     pcubase+0x44   ; Watchdog Control
swtc             equ     swcr
swr              equ     pcubase+0x48
pqedgdat         equ     pcubase+0x50   ; Edge Selection PortQ
pqpar            equ     pcubase+0x54   ; Pin Assignment PortQ
                endif

;----------------------------------------------------------------------------
; SRAM Module MPC505

                if      MOMCPU=0x505
srammcr          equ     0x8007f00      ; Basiskonfiguration SRAM
                endif

;============================================================================
; SUBI may have two or three arguments

subi            macro   dest,src,VAL
                if      "VAL"=""
                 addi   dest,dest,-src
                elseif
                 addi   dest,src,-VAL
                endif
                endm

;----------------------------------------------------------------------------
; Comparisons

cmpw            macro   cr,REG1,REG2
                if      "REG2"=""
                cmp     0,0,cr,REG1
                elseif
                cmp     cr,0,REG1,REG2
                endif
                endm

cmpwi           macro   cr,REG1,IMM
                if      "IMM"=""
                cmpi    0,0,cr,IMM
                elseif
                cmpi    cr,0,REG1,imm
                endif
                endm

cmplw           macro   cr,REG1,REG2
                if      "REG2"=""
                cmpl    0,0,cr,REG1
                elseif
                cmpl    cr,0,REG1,REG2
                endif
                endm

cmplwi          macro   cr,REG1,IMM
                if      "IMM"=""
                cmpli   0,0,cr,IMM
                elseif
                cmpli   cr,0,REG1,IMM
                endif
                endm

;----------------------------------------------------------------------------
; Extended Instructions Condition Code Register

crset           macro   bx
                creqv   bx,bx,bx
                endm

crnot           macro   bx,by
                crnor   bx,by,by
                endm

crmove          macro   bx,by
                cror    bx,by,by
                endm

;----------------------------------------------------------------------------
; Extended Logic Instructions

not             macro   dest,SRC
                if      "SRC"=""
                 nor    dest,dest
                elseif
                 nor    dest,SRC,SRC
                endif
                endm

not.            macro   dest,SRC
                if      "SRC"=""
                 nor.   dest,dest
                elseif
                 nor.   dest,SRC,SRC
                endif
                endm

mr              macro   dest,src
                or      dest,src,src
                endm

mr.             macro   dest,src
                or.     dest,src,src
                endm

nop             macro
                ori     0,0,0
                endm

;----------------------------------------------------------------------------
; Simplified Shift-In Instructions

inslwi          macro   ra,rs,n,b
                rlwimi  ra,rs,32-b,b,b+n-1
                endm
inslwi.         macro   ra,rs,n,b
                rlwimi. ra,rs,32-b,b,b+n-1
                endm

insrwi          macro   ra,rs,n,b
                rlwimi  ra,rs,32-b-n,b,b+n-1
                endm
insrwi.         macro   ra,rs,n,b
                rlwimi. ra,rs,32-b-n,b,b+n-1
                endm

__defins1       macro   NAME,par1,par2,par3,{NoExpand}
{"NAME"}        macro   ra,rs,n
                rlwinm  ra,rs,par1,par2,par3
                endm
{"NAME"}.       macro   ra,rs,n
                rlwinm. ra,rs,par1,par2,par3
                endm
                endm

__defins2       macro   NAME,par1,par2,par3,{NoExpand}
{"NAME"}        macro   ra,rs,b,n
                rlwinm  ra,rs,par1,par2,par3
                endm
{"NAME"}.       macro   ra,rs,b,n
                rlwinm. ra,rs,par1,par2,par3
                endm
                endm

                __defins1 clrlwi,0,n,31
                __defins2 clrlslwi,n,b-n,31-n
                __defins1 clrrwi,0,0,31-n
                __defins2 extlwi,b,0,n-1
                __defins2 extrwi,b+n,32-n,31
                __defins1 rotlwi,n,0,31
                __defins1 rotrwi,32-n,0,31
                __defins1 slwi,n,0,31-n
                __defins1 srwi,32-n,n,31

rotlw           macro   ra,rs,rb
                rlwnm   ra,rs,rb,0,31
                endm
rotlw.          macro   ra,rs,rb
                rlwnm.  ra,rs,rb,0,31
                endm

;----------------------------------------------------------------------------
; Simplified Jumps

__defjmp1       macro   NAME,m1,m2,{NoExpand}
{"NAME"}        macro   adr
                bc      m1,m2,adr
                endm
{"NAME"}a       macro   adr
                bca     m1,m2,adr
                endm
{"NAME"}l       macro   adr
                bcl     m1,m2,adr
                endm
{"NAME"}la      macro   adr
                bcla    m1,m2,adr
                endm
                endm

                __defjmp1 bdnz,16,0
                __defjmp1 bdz,18,0

__defjmp2       macro   NAME,m1,{NoExpand}
{"NAME"}        macro   cr,adr
                bc      m1,cr,adr
                endm
{"NAME"}a       macro   cr,adr
                bca     m1,cr,adr
                endm
{"NAME"}l       macro   cr,adr
                bcl     m1,cr,adr
                endm
{"NAME"}la      macro   cr,adr
                bcla    m1,cr,adr
                endm
                endm

                __defjmp2 bdnzf,0
                __defjmp2 bdnzt,8
                __defjmp2 bdzf,2
                __defjmp2 bdzt,10
                __defjmp2 bf,4
                __defjmp2 bt,12

__defjmp3       macro   NAME,mask,ofs,{NoExpand}
{"NAME"}        macro   cr,ADR
                if      "ADR"=""
                 bc      mask,ofs,cr
                elseif
                 bc      mask,cr*4+ofs,adr
                endif
                endm
{"NAME"}a       macro   cr,ADR
                if      "ADR"=""
                 bca     mask,ofs,cr
                elseif
                 bca     mask,cr*4+ofs,adr
                endif
                endm
{"NAME"}l       macro   cr,ADR
                if      "ADR"=""
                 bcl     mask,ofs,cr
                elseif
                 bcl     mask,cr*4+ofs,adr
                endif
                endm
{"NAME"}la      macro   cr,ADR
                if      "ADR"=""
                 bcla    mask,ofs,cr
                elseif
                 bcla    mask,cr*4+ofs,adr
                endif
                endm
{"NAME"}ctr     macro   CR
                if      "CR"=""
                 bcctr   mask,ofs
                elseif
                 bc      mask,CR*4+ofs
                endif
                endm
{"NAME"}ctrl    macro   CR
                if      "CR"=""
                 bcl     mask,ofs
                elseif
                 bcl     mask,CR*4+ofs
                endif
                endm
{"NAME"}lr      macro   CR
                if      "CR"=""
                bclr    mask,ofs
                elseif
                bclr    mask,4*CR+ofs
                endif
                endm
{"NAME"}lrl     macro   CR
                if      "CR"=""
                bclrl   mask,ofs
                elseif
                bclrl   mask,4*CR+ofs
                endif
                endm
                endm

                __defjmp3 beq,12,2
                __defjmp3 bge,4,0
                __defjmp3 bgt,12,1
                __defjmp3 ble,4,1
                __defjmp3 blt,12,0
                __defjmp3 bne,4,2
                __defjmp3 bng,4,1
                __defjmp3 bnl,4,0
                __defjmp3 bns,4,3
                __defjmp3 bnu,4,3
                __defjmp3 bso,12,3
                __defjmp3 bun,12,3

bctr            macro
                bcctr   20,0
                endm
bctrl           macro
                bcctrl  20,0
                endm

__defjmp4       macro   NAME,mask,{NoExpand}
{"NAME"}        macro   cr
                bcctr   mask,cr
                endm
{"NAME"}l       macro   cr
                bcctrl  mask,cr
                endm
                endm

                __defjmp4 bfctr,4
                __defjmp4 btctr,12

__defjmp6       macro   NAME,mask,bit,{NoExpand}
{"NAME"}        macro
                bclr    mask,bit
                endm
{"NAME"}l       macro
                bclrl   mask,bit
                endm
                endm

                __defjmp6 blr,20,0
                __defjmp6 bdnzlr,16,0
                __defjmp6 bdzlr,18,0

__defjmp7       macro   NAME,mask,{NoExpand}
{"NAME"}        macro   cr
                bclr    mask,cr
                endm
{"NAME"}l       macro   cr
                bclrl   mask,cr
                endm
                endm

                __defjmp7 bdnzflr,0
                __defjmp7 bdnztlr,8
                __defjmp7 bdzflr,2
                __defjmp7 bdztlr,10
                __defjmp7 bflr,4
                __defjmp7 btlr,12

;-------------------------------------------------------------------------
; Traps

trap            macro   ra,rb
                tw      31,ra,rb
                endm

__deftrap       macro   NAME,mask,{NoExpand}
{"NAME"}        macro   ra,rb
                tw      mask,ra,rb
                endm
{"NAME"}i       macro   ra,im
                twi     mask,ra,im
                endm
                endm

                __deftrap tweq,4
                __deftrap twge,12
                __deftrap twgt,8
                __deftrap twle,20
                __deftrap twlge,5
                __deftrap twlgt,1
                __deftrap twlle,6
                __deftrap twllt,2
                __deftrap twlng,6
                __deftrap twlnl,5
                __deftrap twlt,16
                __deftrap twne,24
                __deftrap twng,20
                __deftrap twnl,12

;-------------------------------------------------------------------------
; MMU Macros PPC403G[BC]

		if (MOMCPU=0x403c)||(MOMCPU=0x403c)

tlbrehi		macro	rt,ra
		tlbre	rt,ra,0
		endm

tlbrelo		macro	rt,ra
		tlbre	rt,ra,1
		endm

tlbwehi		macro	rt,ra
		tlbwe	rt,ra,0
		endm

tlbwelo		macro	rt,ra
		tlbwe	rt,ra,1
		endm

		endif

;=========================================================================

                restore                 ; allow listing again

                endif			; stddef60inc
