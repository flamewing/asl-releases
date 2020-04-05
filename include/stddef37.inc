		ifndef  stddef37inc     ; avoid multiple inclusion
stddef37inc     equ     1

		save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF37.INC                                            *
;*   								            *
;*   Contains SFR and Bit Definitions for TMS370 Processors                 *
;*									    *
;****************************************************************************

                if      (MOMCPU<>57720848)&&(MOMCPU<>57720864)&&(MOMCPU<>57720880)&&(MOMCPU<>57720896)&&(MOMCPU<>57720912)
		 fatal  "wrong target selected: only 370C010, 370C020, 370C030, 370C040 oder 370C050 supported"
		endif


                if      MOMPASS=1
		 message "TMS370 Register Definitions (C) 1994 Alfred Arnold"
		 message "including \{MOMCPU} Registers"
		endif

;----------------------------------------------------------------------------
; System Control

sccr0		equ	p010
cold_start	dbit	7,sccr0	; Cold Start
osc_power	dbit	6,sccr0	; Reduce Oscillator Power Consumption
pf_auto_wait	dbit	5,sccr0 ; External Peripherals Wait States
osc_flt_flag	dbit	4,sccr0 ; Oscillator Fault
mc_pin_wpo	dbit	3,sccr0 ; Voltage on MC high enough
mc_pin_data	dbit	2,sccr0	; MC Pin State
up_uc_mode	dbit	0,sccr0	; Micro Processor or Micro Controller Mode

sccr1		equ	p011
auto_wait_disable dbit	4,sccr1	; External Memory Wait States
memory_disable	dbit	2,sccr1	; Disable Internal ROM

sccr2		equ	p012
halt_standby	dbit	7,sccr2 ; Halt or /Standby Mode
pwrdwn_idle	dbit	6,sccr2 ; /Idle or Halt/Standby Mode
bus_stest	dbit	4,sccr2	; Bus Self Test
cpu_stest	dbit	3,sccr2 ; Processor Self Test
int1_nmi	dbit	1,sccr2	; Interrupt 1 Maskable or not
privilege_disable dbit	0,sccr2	; Leave Privileged Mode

;----------------------------------------------------------------------------
; Digital IO

aport1		equ	p020
aport2		equ	p021
adata		equ	p022	; Port A Data Register
adir		equ	p023	; Port A Data Direction Register

                if      (MOMCPU=370C020h)||(MOMCPU>370C030h)
bport1		 equ	p024
bport2		 equ	p025
bdata		 equ	p026	; Port B Data Register
bdir		 equ	p027	; Port B Data Direction Register
		endif

                if      (MOMCPU=370C020h)||(MOMCPU=370C050h)
cport1		 equ	p028
cport2		 equ	p029
cdata		 equ	p02a	; Port C Data Register
cdir		 equ	p02b	; Port C Data Direction Register
		endif

dport1		equ	p02c
dport2		equ	p02d
ddata		equ	p02e	; Port D Data Register
ddir		equ	p02f	; Port D Data Direction Register

;----------------------------------------------------------------------------
; Interrupts

int1		equ	p017
int1_enable	dbit	0,int1	; External Interrupt 1 Enable
int1_priority	dbit	1,int1	; External Interrupt 1 Priority
int1_polarity	dbit	2,int1	; External Interrupt 1 Polarity
int1_pin_data	dbit	6,int1	; External Interrupt 1 Pin State
int1_flag	dbit	7,int1	; External Interrupt 1 Flag

int2		equ	p018
int2_enable	dbit	0,int2	; External Interrupt 2 Enable
int2_priority	dbit	1,int2	; External Interrupt 2 Priority
int2_polarity	dbit	2,int2	; External Interrupt 2 Polarity
int2_data_out	dbit	3,int2	; External Interrupt 2 Pin Output Value
int2_data_dir	dbit	4,int2	; External Interrupt 2 Pin Data Direction
int2_pin_data	dbit	6,int2	; External Interrupt 2 Pin State
int2_flag	dbit	7,int2	; External Interrupt 2 Flag

int3		equ	p019
int3_enable	dbit	0,int3	; External Interrupt 3 Enable
int3_priority	dbit	1,int3	; External Interrupt 3 Priority
int3_polarity	dbit	2,int3	; External Interrupt 3 Polarity
int3_data_out	dbit	3,int3	; External Interrupt 3 Pin Output Value
int3_data_dir	dbit	4,int3	; External Interrupt 3 Pin Data Direction
int3_pin_data	dbit	6,int3	; External Interrupt 3 Pin State
int3_flag	dbit	7,int3	; External Interrupt 3 Flag

;----------------------------------------------------------------------------
; (E)EPROM

deectl		equ	p01a
ee_busy		dbit	7,deectl ; EEPROM Busy
ap		dbit	2,deectl ; Enable Block Programming
w1w0		dbit	1,deectl ; Program Ones or Zeros?
ee_exe		dbit	0,deectl ; Execute Transaction

epctl0		equ	p01c
ep0_busy	dbit	7,epctl0 ; EPROM Part 1 Busy
ep0_vpps	dbit	6,epctl0 ; Enable Programming Voltage
ep0_w0		dbit	1,epctl0 ; Enable Writing Zeros
ep0_exe		dbit	0,epctl0 ; Execute Write Transaction

epctl1		equ	p01e
ep1_busy	dbit	7,epctl1 ; EPROM Part 2 Busy
ep1_vpps	dbit	6,epctl1 ; Enable Programming Voltage
ep1_w0		dbit	1,epctl1 ; Enable Writing Zeros
ep1_exe		dbit	0,epctl1 ; Execute Write Transaction

;----------------------------------------------------------------------------
; Timer 1

t1ctl1		equ	p049	; Controls also the Watchdog
t1ctl2		equ	p04a

                if      MOMCPU<>370C030h
t1cntrh		 equ	p040	; Count Register
t1cntrl		 equ	p041

t1ch		 equ	p042	; Compare Register
t1cl		 equ	p043

t1cch		 equ	p044	; Compare/Capture Register
t1ccl		 equ	p045

t1_input_select0 dbit	0,t1ctl1 ; Clock Source
t1_input_select1 dbit	1,t1ctl1
t1_input_select2 dbit	2,t1ctl1

t1_sw_reset	 dbit	0,t1ctl2 ; Reset Counter
t1_ovrfl_int_flag dbit	3,t1ctl2 ; Overflow Flag
t1_ovrfl_int_ena dbit	4,t1ctl2 ; Enable Overflow Interrupt

t1ctl3		 equ	p04b
t1c1_int_ena	 dbit	0,t1ctl3 ; Compare Register Interrupt Enable
t1c2_int_ena	 dbit	1,t1ctl3 ; Capture/Compare Register Interrupt Enable
t1edge_int_ena	 dbit	2,t1ctl3 ; Edge on T1IC/CR Interrupt Enable
t1c1_int_flag	 dbit	5,t1ctl3 ; Compare Register Interrupt Flag
t1c2_int_flag	 dbit	6,t1ctl3 ; Capture/Compare Register Interrupt Flag
t1edge_int_flag	 dbit	7,t1ctl3 ; Edge on T1IC/CR Interrupt Flag

t1ctl4		 equ	p04c
t1edge_det_ena	 dbit	0,t1ctl4 ; T1IC/CR Edge Detector Enable
t1cr_rst_ena	 dbit	1,t1ctl4 ; External Reset Enable
t1edge_polarity	 dbit	2,t1ctl4 ; T1IC/CR Edge Selection
t1cr_out_ena	 dbit	3,t1ctl4 ; T1IC/CR PWM Toggle Enable
t1c1_rst_ena     dbit   4,t1ctl4 ; Comparator Reset Enable
t1c2_out_ena	 dbit	5,t1ctl4 ; Capture/Compare Register PWM Toggle Enable
t1c1_out_ena	 dbit	6,t1ctl4 ; Compare Register PWM Toggle Enable
t1_mode		 dbit	7,t1ctl4 ; Timer 1 Mode

t1pc1		 equ	p04d
t1evt_data_dir	 dbit	0,t1pc1	; Timer 1 Event Pin Data Direction
t1evt_function	 dbit	1,t1pc1	; Timer 1 Event Pin Function Selection
t1evt_data_out	 dbit	2,t1pc1 ; Timer 1 Event Pin Data Output
t1evt_data_in	 dbit	3,t1pc1 ; Timer 1 Event Pin Data Input

t1pc2		 equ	p04e
t1iccr_data_dir	 dbit	0,t1pc2 ; T1IC/CR Pin Data Direction
t1iccr_function	 dbit	1,t1pc2 ; T1IC/CR Pin Function Selection
t1iccr_data_out	 dbit	2,t1pc2 ; T1IC/CR Pin Data Output
t1iccr_data_in	 dbit	3,t1pc2 ; T1IC/CR Pin Data Input
t1pwm_data_dir	 dbit	4,t1pc2 ; T1PWM Pin Data Direction
t1pwm_function	 dbit	5,t1pc2 ; T1PWM Pin Function Selection
t1pwm_data_out	 dbit	2,t1pc2 ; T1PWM Pin Data Output
t1pwm_data_in	 dbit	7,t1pc2 ; T1PWM Pin Data Input

t1pri		 equ	p04f
t1_priority	 dbit	6,t1pri ; Timer 1 Interrupt Priority
t1_stest	 dbit	7,t1pri ; Timer 1 Self Test
		endif

;----------------------------------------------------------------------------
; Timer 2

                if MOMCPU>370C030H
t2cntrh		 equ	p060	; Count Register
t2cntrl		 equ	p061

t2ch		 equ	p062	; Compare Register
t2cl		 equ	p063

t2cch		 equ	p064	; Capture/Compare Register
t2ccl		 equ	p065

t2ich		 equ	p066	; Capture Register
t2icl		 equ	p067

t2ctl1		 equ	p06a
t2_sw_reset	 dbit	0,t2ctl1 ; Reset Counter
t2_input_select0 dbit	1,t2ctl1 ; Clock Source
t2_input_select1 dbit	2,t2ctl1
t2_ovrfl_int_flag dbit	3,t2ctl1 ; Overflow Interrupt Flag
t2_ovrfl_int_ena dbit	4,t2ctl1 ; Overflow Interrupt Enable

t2ctl2		 equ	p06b
t2c1_int_ena	 dbit	0,t2ctl2 ; Compare Register Interrupt Enable
t2c2_int_ena	 dbit	1,t2ctl2 ; Capture/Compare Register Interrupt Enable
t2edge1_int_ena	 dbit	2,t2ctl2 ; Edge on T2IC1/CR Interrupt Enable
t2c1_int_flag	 dbit	5,t2ctl2 ; Compare Register Interrupt Flag
t2c2_int_flag	 dbit	6,t2ctl2 ; Capture/Compare Register Interrupt Flag
t2edge1_int_flag dbit	7,t2ctl2 ; Edge on T2IC1/CR Interrupt Flag

t2ctl3		 equ	p06c
t2edge1_det_ena	 dbit	0,t2ctl3 ; T2IC1/CR Edge Detector Enable
t2edge1_rst_ena	 dbit	1,t2ctl3 ; External Reset Enable
t2edge2_det_ena	 dbit	1,t2ctl3 ; T2IC2/CR Edge Detector Enable
t2edge1_polarity dbit	2,t2ctl3 ; T2IC1/CR Edge Selection
t2edge1_out_ena	 dbit	3,t2ctl3 ; T2IC1/CR PWM Toggle Enable
t2edge2_polarity dbit	3,t2ctl3 ; T2IC2/CR Edge Selection
t2c1_rst_ena	 dbit	4,t2ctl3 ; Comparator Reset Enable
t2c2_out_ena	 dbit	5,t2ctl3 ; Capture/Compare Register PWM Toggle Enable
t2c1_out_ena	 dbit	6,t2ctl3 ; Capture Register PWM Toggle Enable
t2_mode		 dbit	7,t2ctl3 ; Timer 2 Mode

t2pc1		 equ	p06d
t2evt_data_dir	 dbit	0,t2pc1	; Timer 2 Event Pin Data Direction
t2evt_function	 dbit	1,t2pc1	; Timer 2 Event Pin Function Selection
t2evt_data_out	 dbit	2,t2pc1 ; Timer 2 Event Pin Data Output
t2evt_data_in	 dbit	3,t2pc1 ; Timer 2 Event Pin Data Input

t2pc2		 equ	p06e
t2ic1cr_data_dir dbit	0,t1pc2 ; T2IC1/CR Pin Data Direction
t2ic1cr_function dbit	1,t1pc2 ; T2IC1/CR Pin Function Selection
t2ic1cr_data_out dbit	2,t1pc2 ; T2IC1/CR Pin Data Output
t2ic1cr_data_in	 dbit	3,t1pc2 ; T2IC1/CR Pin Data Input
t2ic2cr_data_dir dbit	4,t1pc2 ; T2IC2/CR Pin Data Direction
t2ic2cr_function dbit	5,t1pc2 ; T2IC2/CR Pin Function Selection
t2ic2cr_data_out dbit	6,t1pc2 ; T2IC2/CR Pin Data Output
t2ic2cr_data_in	 dbit	7,t1pc2 ; T2IC2/CR Pin Data Input
t2pwm_data_dir	 dbit	4,t1pc2 ; T2PWM Pin Data Direction
t2pwm_function	 dbit	5,t1pc2 ; T2PWM Pin Function Selection
t2pwm_data_out	 dbit	6,t1pc2 ; T2PWM Pin Data Output
t2pwm_data_in	 dbit	7,t1pc2 ; T2PWM Pin Data Input

t2pri		 equ	p06f
t2_priority	 dbit	6,t2pri ; Timer 2 Interrupt Priority
t2_stest	 dbit	7,t2pri ; Timer 2 Self Test
		endif

;----------------------------------------------------------------------------
; Watchdog

                if      MOMCPU<>370C030h
wdcntrh          equ    p046    ; Count Register
wdcntrl          equ    p047

wdrst            equ    p048    ; Reset Key Register

wd_input_select0 dbit	4,t1ctl1 ; Clock Source
wd_input_select1 dbit	5,t1ctl1
wd_input_select2 dbit	6,t1ctl1
wd_ovrfl_tap_sel dbit	7,t1ctl1 ; Count Tith 15 or 16 Bits
wd_ovrfl_int_flag dbit	5,t1ctl2 ; Watchdog Reset Flag
wd_ovrfl_int_ena dbit	6,t1ctl2 ; Watchdog Interrupt Enable
wd_ovrfl_rst_ena dbit   7,t1ctl2 ; Watchdog Reset Enable
                endif

;----------------------------------------------------------------------------
; SCI

                if (MOMCPU=370C020h)||(MOMCPU>370C030h)
sciccr		 equ	p050
sci_char0	 dbit	0,sciccr ; Character Length
sci_char1	 dbit	1,sciccr
sci_char2	 dbit	2,sciccr
address_idle_wup dbit	3,sciccr ; Multi Processor Mode
async_iosync	 dbit	4,sciccr ; Synchronous/Asynchronous Mode
parity_enable	 dbit	5,sciccr ; Parity Enable
even_odd_parity	 dbit	6,sciccr ; Even or Odd Parity
stop_bits	 dbit	7,sciccr ; Number of Stopbits

scictl		 equ	p051
rxena		 dbit	0,scictl ; Receiver Enable
txena		 dbit	1,scictl ; Transmitter Enable
sleep		 dbit	2,scictl ; SCI Sleep Mode
txwake		 dbit	3,scictl ; Transmitter Wake
clock		 dbit   4,scictl ; SCI Internal or External Clock
sci_sw_reset	 dbit	5,scictl ; Software Reset

baud_msb	 equ	p052	 ; Baud Rate Generator
baud_lsb	 equ	p053

txctl		 equ	p054
sci_tx_int_ena	 dbit	0,txctl	 ; Transmitter Interrupt Enable
tx_empty	 dbit	6,txctl  ; Transmitter Entirely Empty
txrdy		 dbit	7,txctl  ; Transmitter Redy to Accept Character

rxctl		 equ	p055
sci_rx_int_ena	 dbit	0,rxctl	 ; Receiver Interrupt Enable
rxwake		 dbit	1,rxctl	 ; Receiver Woken Up
sci_pe		 dbit	2,rxctl	 ; Parity Error
sci_oe		 dbit	3,rxctl	 ; Overflow Error
sci_fe		 dbit	4,rxctl	 ; Framing Error
brkdt		 dbit	5,rxctl	 ; Break Detected
rxrdy		 dbit	6,rxctl	 ; Character Detected
rx_error	 dbit	7,rxctl	 ; Receive Error

rxbuf		 equ	p057	 ; Data Register
txbuf		 equ	p059

scipc1		 equ	p05d
sciclk_data_dir	 dbit	0,scipc1 ; SCICLK Pin Data Direction
sciclk_function	 dbit	1,scipc1 ; SCICLK Pin Function Select
sciclk_data_out	 dbit	2,scipc1 ; SCICLK Pin Data Output
sciclk_data_in	 dbit	3,scipc1 ; SCICLK Pin Data Input

scipc2		 equ	p05e
scirxd_data_dir	 dbit	0,scipc2 ; SCIRXD-Pin Data Direction
scirxd_function	 dbit	1,scipc2 ; SCIRXD-Pin Function Select
scirxd_data_out	 dbit	2,scipc2 ; SCIRXD-Pin Data Output
scirxd_data_in	 dbit	3,scipc2 ; SCIRXD-Pin Data Input
scitxd_data_dir	 dbit	4,scipc2 ; SCITXD-Pin Data Direction
scitxd_function	 dbit	5,scipc2 ; SCITXD-Pin Function Select
scitxd_data_out	 dbit	6,scipc2 ; SCITXD-Pin Data Output
scitxd_data_in	 dbit	7,scipc2 ; SCITXD-Pin Data Input

scipri		 equ	p05f
sci_espen	 dbit	4,scipri ; SCI Emulator Suspend
scirx_priority	 dbit	5,scipri ; SCI Receiver Interrupt Priority
scitx_priority	 dbit	6,scipri ; SCI Transmitter Interrupt Priority
sci_test	 dbit	7,scipri ; SCI Self Test
		endif

;----------------------------------------------------------------------------
; SPI

                if (MOMCPU<370C030h)||(MOMCPU=370C050h)
spiccr           equ    p030
spi_char0        dbit   0,spiccr ; Character Length
spi_char1        dbit   1,spiccr
spi_char2        dbit   2,spiccr
spi_bit_rate0    dbit   3,spiccr ; Bit Rate
spi_bit_rate1    dbit   4,spiccr
spi_bit_rate2    dbit   5,spiccr
clock_polarity   dbit   6,spiccr ; Clock Polarity
spi_sw_reset     dbit   7,spiccr ; Software Reset

spictl           equ    p031
spi_int_ena      dbit   0,spictl ; Interrupt Freigabe
talk             dbit   1,spictl ; Slave/Master Transmitter Enable
master_slave     dbit   2,spictl ; Slave/Master Select
spi_int_flag     dbit   6,spictl ; Interrupt Flag
receiver_overrun dbit   7,spictl ; Receiver Overflow

spibuf           equ    p037     ; Receive Buffer
spidat           equ    p039     ; Transmit Register

spipc1           equ    p03d
spiclk_data_dir  dbit   0,spipc1 ; SPICLK Pin Data Direction
spiclk_function  dbit   1,spipc1 ; SPICLK Pin Function Select
spiclk_data_out  dbit   2,spipc1 ; SPICLK Pin Data Output
spiclk_data_in   dbit   3,spipc1 ; SPICLK Pin Data Input

spipc2           equ    p03e
spisomi_data_dir dbit   0,spipc2 ; SPISOMI Pin Data Direction
spisomi_function dbit   1,spipc2 ; SPISOMI Pin Function Select
spisomi_data_out dbit   2,spipc2 ; SPISOMI Pin Data Output
spisomi_data_in  dbit   3,spipc2 ; SPISOMI Pin Data Input
spimosi_data_dir dbit   4,spipc2 ; SPIMOSI Pin Data Direction
spimosi_function dbit   5,spipc2 ; SPIMOSI Pin Function Select
spimosi_data_out dbit   6,spipc2 ; SPIMOSI Pin Data Output
spimosi_data_in  dbit   7,spipc2 ; SPIMOSI Pin Data Input

spipri           equ    p03f
spi_espen        dbit   5,spipri ; Suppress Emulator
spi_priority     dbit   6,spipri ; Interrupt Priority
spi_stest        dbit   7,spipri ; Self Test
                endif

;----------------------------------------------------------------------------
; A/D Converter

                if      MOMCPU>370C030h
adctl            equ    p070
ad_input_select0 dbit   0,adctl  ; Channel Selection
ad_input_select1 dbit   1,adctl
ad_input_select2 dbit   2,adctl
ref_volt_select0 dbit   3,adctl  ; Reference Voltage Selection
ref_volt_select1 dbit   4,adctl
ref_volt_select2 dbit   5,adctl
sample_start     dbit   6,adctl  ; Start Sample Phase (auto-clear)
convert_start    dbit   7,adctl  ; Start Conversion Phase

adstat           equ    p071
ad_int_ena       dbit   0,adstat ; Interrupt Enable
ad_int_flag      dbit   1,adstat ; Interrupt Flag
ad_ready         dbit   2,adstat ; AD Converter Ready?

addata           equ    p072     ; Conversion Result
adin             equ    p07d     ; Digital Input Data if Unused as Analog Inputs
adena            equ    p07e     ; Enable Analog Inputs

adpri            equ    p07f
ad_espen         dbit   5,adpri  ; Suppress Emulator
ad_priority      dbit   6,adpri  ; Interrupt Priority
ad_stest         dbit   7,adpri  ; Self Test
                endif

;----------------------------------------------------------------------------
; PACT

                if      MOMCPU=370C030h
pact_scr         equ    p040
pact_prescale_select0 dbit 0,pact_scr   ; Prescaler Selection
pact_prescale_select1 dbit 1,pact_scr
pact_prescale_select2 dbit 2,pact_scr
pact_prescale_select3 dbit 3,pact_scr
fast_mode_select dbit   4,pact_scr      ; Disable Prescaler-by-8
cmd_def_area_ena dbit   5,pact_scr      ; Enable Dual-Port-RAM Access
deftim_ovrfl_int_flag dbit 6,pact_scr   ; Timer Overflow Interrupt Flag
deftim_ovrfl_int_ena  dbit 7,pact_scr   ; Timer Overflow Interrupt Enable

cdstart          equ    p041
cmd_def_area2    dbit   2,cdstart       ; Command/Definition Area Start Address
cmd_def_area3    dbit   3,cdstart
cmd_def_area4    dbit   4,cdstart
cmd_def_area5    dbit   5,cdstart
cmd_def_area_int_ena dbit 7,cdstart     ; Enable Interrupts

cdend            equ    p042
cmd_def_area_end2 dbit  2,cdend         ; Command/Definition Area End Address
cmd_def_area_end3 dbit  3,cdend
cmd_def_area_end4 dbit  4,cdend
cmd_def_area_end5 dbit  5,cdend
cmd_def_area_end6 dbit  6,cdend

bufptr           equ    p043
buffer_pointer1  dbit   1,bufptr        ; Buffer Pointer Address
buffer_pointer2  dbit   2,bufptr
buffer_pointer3  dbit   3,bufptr
buffer_pointer4  dbit   4,bufptr
buffer_pointer5  dbit   5,bufptr

scictlp          equ    p045
sci_sw_reset      dbit  0,scictlp       ; Software Reset
sci_tx_int_ena    dbit  2,scictlp       ; Transmit Interrupt Enable
sci_rx_int_ena    dbit  3,scictlp       ; Receive Interrupt Enable
pact_fe           dbit  4,scictlp       ; Framing Error
pact_parity       dbit  5,scictlp       ; Parity Error
pact_txrdy        dbit  6,scictlp       ; Transmitter Empty
pact_rxrdy        dbit  7,scictlp       ; Receive Buffer Full

rxbufp           equ    p046            ; Mini-SCI Receive Buffer
txbufp           equ    p047            ; Mini-SCI Transmit Buffer

opstate          equ    p048            ; Output Pin State
cdflags          equ    p049            ; Command 0..7 Interrupt Flags

cpctl1           equ    p04a
cp1_capt_falling_edge dbit 0,cpctl1     ; CP1 Falling Edge Capture
cp1_capt_rising_edge dbit 1,cpctl1      ; CP1 Rising Edge Capture
cp1_int_flag     dbit   2,cpctl1        ; CP1 Edge Occured
cp1_int_ena      dbit   3,cpctl1        ; CP1 Interrupt Enable
cp2_capt_falling_edge dbit 4,cpctl1     ; CP2 Falling Edge Capture
cp2_capt_rising_edge dbit 5,cpctl1      ; CP2 Rising Edge Capture
cp2_int_flag     dbit   6,cpctl1        ; CP2 Edge Occured
cp2_int_ena      dbit   7,cpctl1        ; CP2 Interrupt Enable

cpctl2           equ    p04b
cp3_capt_falling_edge dbit 0,cpctl2     ; CP3 Falling Edge Capture
cp3_capt_rising_edge dbit 1,cpctl2      ; CP3 Rising Edge Capture
cp3_int_flag     dbit   2,cpctl2        ; CP3 Edge Occured
cp3_int_ena      dbit   3,cpctl2        ; CP3 Interrupt Enable
cp4_capt_falling_edge dbit 4,cpctl2     ; CP4 Falling Edge Capture
cp4_capt_rising_edge dbit 5,cpctl2      ; CP4 Rising Edge Capture
cp4_int_flag     dbit   6,cpctl2        ; CP4 Edge Occured
cp4_int_ena      dbit   7,cpctl2        ; CP4 Interrupt Enable

cpctl3           equ    p04c
cp5_capt_falling_edge dbit 0,cpctl3     ; CP5 Falling Edge Capture
cp5_capt_rising_edge dbit 1,cpctl3      ; CP5 Rising Edge Capture
cp5_int_flag     dbit   2,cpctl3        ; CP5 Edge Occured
cp5_int_ena      dbit   3,cpctl3        ; CP5 Interrupt Enable
cp6_capt_falling_edge dbit 4,cpctl3     ; CP6 Falling Edge Capture
cp6_capt_rising_edge dbit 5,cpctl3      ; CP6 Rising Edge Capture
cp6_int_flag     dbit   6,cpctl3        ; CP6 Edge Occured
cp6_int_ena      dbit   7,cpctl3        ; CP6 Interrupt Enable

cppre            equ    p04d
op_set_clr_select dbit  0,cppre         ; Set/Reset Output Pins in Software
event_counter_sw_reset dbit 1,cppre     ; Reset Event Counter
cp6_event_only   dbit   2,cppre         ; CP6 only for Event Counter
input_capt_prescale_select0 dbit 3,cppre; CP3..CP6 Prescaler
input_capt_prescale_select1 dbit 4,cppre
input_capt_prescale_select2 dbit 5,cppre
buffer_half_full_int_flag dbit 6,cppre  ; Buffer Half/Full Interrupt Flag
buffer_half_full_int_ena dbit 7,cppre   ; Buffer Half/Full Interrupt Enable

wdrst            equ    p04e

pactpri          equ    p04f
wd_prescale_select0 dbit 0,pactpri      ; Watchdog Prescaler
wd_prescale_select1 dbit 1,pactpri
pact_mode_select dbit 2,pactpri         ; PACT Mode A/B Select
pact_group_3_priority dbit 3,pactpri    ; Interrupt Groups Priority
pact_group_2_priority dbit 4,pactpri
pact_group_1_priority dbit 5,pactpri
pact_stest       dbit   7,pactpri       ; Self Test
                endif

;----------------------------------------------------------------------------

		restore                 ; re-allow listing

		endif			; stddef37inc
