                ifndef  __regavrinc     ; avoid multiple inclusion
__regavrinc	equ     1

                save
                listing off   		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - Datei REGAVROLD.INC                                          *
;*                                                                          *
;*   Contains SFR  and Bit Definitions for AVR Processors                   *
;*          OUTDATED VERSION - PLEASE SWITCH TO NEW FILE IF POSSIBLE        *
;*                                                                          *
;****************************************************************************

; Set internal CPU code and memory addresses in one step

		switch	MOMCPUNAME
		case	"AT90S1200"
__cpucode	  equ	  0x011200
eeend             equ     63		; End Address EEPROM
iramend           equ     95,data	; End Address SRAM
iromend           label   1023		; End Address EPROM
		case	"AT90S2313"
__cpucode	  equ	  0x012313
eeend             equ     127
iramend           equ     0xdf,data
iromend           label   2047
		case	"AT90S4414"
__cpucode	  equ	  0x014414
eeend             equ     255
iramend           equ     0x15f,data
iromend           label   4095
		case	"AT90S8515"
__cpucode	  equ	  0x018515
eeend             equ     511
iramend           equ     0x25f,data
iromend           label   8191
		case	"ATMEGA8"
__cpucode	  equ	  0x020008
eeend             equ     511
iramend           equ     0x45f,data
iromend           label   8191
		case	"ATMEGA16"
__cpucode	  equ	  0x020010
eeend             equ     511
iramend           equ     0x45f,data
iromend           label   16383
		elsecase
                  fatal  "wrong target selected: only AT90S1200, AT90S2313, AT90S4414, AT90S8515, ATMEGA8, or ATMEGA16 supported"
                endcase

ismega		function code,(code>>16)=2

                if      MOMPASS=1
                 message "Atmel AVR SFR Definitions (C) 1996,2002 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; Constant Memory Addresses

eestart         equ     0               ; Start Address Internal EEPROM
iram            equ     96,data         ; Start Address Internal SRAM
                                        ; (behind mapped I/O)
irom            label   0               ; Start Address Internal EPROM

;----------------------------------------------------------------------------
; Prozessorkern

sreg            port    0x3f            ; Status Register:
c               equ     0               ;  Carry
z               equ     1               ;  Zero Result
n               equ     2               ;  Negative Result
v               equ     3               ;  Twos Complement Overflow
s               equ     4               ;  Sign
h               equ     5               ;  Half Carry
t               equ     6               ;  Bit Storage
i               equ     7               ;  Globale Interrupt Enable

; Size of stack pointer depends on size of Internal data space
; (if present at all)

                if      __cpucode>=0x012313
spl              equ     0x3d           ; Stack Pointer (LSB)
                 if      iramend>=256
sph               equ     0x3e          ;               (MSB)
                 endif
                endif

;----------------------------------------------------------------------------
; Chip Configuration

mcucr           port    0x35            ; CPU Control:
isc00           equ     0               ;  INT0 Edge Selection
isc01           equ     1               ;  INT0 Edge/Level Trigger
                if      __cpucode>=0x012313
isc10             equ     2             ;  INT1 Edge Selection
isc11             equ     3             ;  INT1 Edge/Level Trigger
                endif
		if 	ismega(__cpucode)
; Note: for the Mega16, bits 6 & 7 are interchanged, I guess that's an
; error in the data sheet...
se		  equ     7		; Sleep Enable
sm2		  equ	  6		; Sleep Mode Select
sm1		  equ	  5
sm0		  equ	  4
		else ; !ismega
sm                equ     4             ;  Select Idle/PowerDdown Mode
se                equ     5             ;  Enable Sleep Mode
                  if      __cpucode>=0x014414
srw                 equ     6           ;  Wait State Selection External SRAM
sre                 equ     7           ;  External External SRAM
                  endif
		endif

		if	ismega(__cpucode)
osccal		  port	  0x31		; Oscillator Calibration

mcucsr		  port	  0x34
                  if	  __cpucode>=0x020010
jtd		    equ	    7		; JTAG Reset Flag
isc2		    equ	    6		; Interrupt Sense Control 2
		  endif
wdrf		  equ	  3		; Watchdog Reset Occured
borf		  equ	  2		; Brown Out Occured
extrf		  equ	  1		; External Reset Occured
porf		  equ	  0		; Power On Reset Occured

spmcr		  port	  0x37		; Store Program Memory Control Register
spmie		  equ	  7		; Interrupt Enable
rwwsb		  equ	  6		; Read-while-Write Section Busy
rwwsre		  equ	  4		; Read-while-Write Section Read Enable
blbset		  equ	  3		; Boot Lock Bit Set
pgwrt		  equ	  2		; Page Write
pgers		  equ	  1		; Page Erase
spmen		  equ	  0		; Store Program Memory Enable
		endif

;----------------------------------------------------------------------------
; Interrupt-Steuerung

		if 	ismega(__cpucode)
gicr		 port	 0x3b
ivce		 equ	 0		; Interrupt Vector Change Enable
ivsel		 equ	 1		; Interrupt Vector Select
int0		 equ	 6		; Enable External Interrupt 0
int1		 equ	 7		; Enable External Interrupt 1
                 if	 __cpucode>=0x020010
int2		  equ	  5		; Enable External Interrupt 2
		 endif
		elseif
gimsk            port    0x3b           ; Global Interrupt Mask:
int0             equ     6              ;  External Interrupt 0
                 if      __cpucode>=0x012313
int1              equ     7             ;  External Interrupt 1
                 endif
		endif

                if      __cpucode>=0x012313
gifr		 port	 0x3a           ; Global Interrupt Flags
intf0		 equ	 6		;  External Interrupt 0
intf1            equ     7              ;  External Interrupt 1
                 ifdef	 int2
intf2		  equ	  int2		; External Interrupt 2
		 endif
                endif

; who the heck decided to rearrange all bits for the Megas?

timsk           port    0x39            ; Timer Interrupt Mask:
                if      ismega(__cpucode)
toie0            equ     0
                elseif
toie0            equ     1              ;  Timer 0 Overflow
                endif
                if      __cpucode>=0x012313
                 if      __cpucode>=0x014414
                  if 	  ismega(__cpucode)
ocie1b             equ     3            ;  Timer 1 Vergleich B
                  elseif
ocie1b             equ     5            ;  Timer 1 Vergleich B
                  endif
                 endif
                 if	  ismega(__cpucode)
toie1		  equ	  2
ticie1		  equ	  5
toie2		  equ	  6
ocie2		  equ	  7
ocie1a            equ     4
                 elseif
toie1             equ     7             ;  Timer 1 Overflow
ticie1            equ     3             ;  Timer 1 Capture
ocie1a            equ     6             ;  Timer 1 Compare
		 endif
                endif

tifr            port    0x38            ; Timer Interrupt Flags
tov0            equ     toie0		;  Timer 0 Overflow
                if      __cpucode>=0x012313
ocf1a            equ     ocie1a         ;  Timer 1 Compare A
tov1             equ     toie1          ;  Timer 1 Overflow
icf1             equ     ticie1         ;  Timer 1 Capture
		endif
                if      __cpucode>=0x014414
ocf1b            equ     ocie1b         ;  Timer 1 Compare
                endif
		if 	ismega(__cpucode)
tov2		 equ	 toie2
ocf2		 equ	 ocie2
		endif


;----------------------------------------------------------------------------
; Parallel Ports

		if      ismega(__cpucode)
sfior		 equ	 0x30
psr10		 equ	 0		; T0/T1 Prescaler Reset
psr2		 equ	 1		; ditto T2
pud		 equ	 2		; Pullup Disable
acme		 equ	 3		; Analog Comparator Multiplexer Enable
adhsm		 equ	 4		; ADC High Speed Mode
		 if	 __cpucode>=0x020010
adts0		  equ	  5
adts1		  equ	  6
adts2		  equ	  7
		 endif
		endif

                if      (__cpucode=0x014414)||(__cpucode=0x018515)||(__cpucode=0x020010)
porta            port    0x1b           ; Port A Data Register
ddra             port    0x1a           ; Port A Data Direction Register
pina             port    0x19           ; Port A Read Register
                endif

portb           port    0x18            ; Port B Data Register
ddrb            port    0x17            ; Port B Data Direction Register
pinb            port    0x16            ; Port B Read Register

                if      __cpucode>=0x014414
portc            port    0x15           ; Port C Data Register
ddrc             port    0x14           ; Port C Data Direction Register
pinc             port    0x13           ; Port C Read Register
                endif

portd           port    0x12            ; Port D Data Register
ddrd            port    0x11            ; Port D Data Direction Register
pind            port    0x10            ; Port D Read Register

;----------------------------------------------------------------------------
; Timer

tccr0           port    0x33            ; Timer 0 Control Register:
cs00            equ     0               ;  Prescaler Setting
cs01            equ     1
cs02            equ     2
		if	__cpucode=0x020010
wgm01		 equ	 3		; Waveform Generation Mode
wgm00		 equ	 6
com00		 equ	 4		; Compare/Match Output Mode
com01		 equ	 5
foc0		 equ	 7		; Force Output Compare
		endif

tcnt0           port    0x32            ; Timer 0 Count Register

ocr0		port	0x3c

                if      __cpucode>=0x012313
tccr1a           port    0x2f           ; Timer 1 Steuerregister A:
pwm10            equ     0              ;  PWM Mode
wgm10		 equ	 pwm10
pwm11            equ     1
wgm11		 equ	 pwm11
com1a0           equ     6              ;  Compare Mode A
com1a1           equ     7
                 if      __cpucode>=0x014414
com1b0            equ     4             ;  Compare Mode B
com1b1            equ     5
                 endif
		 if 	 ismega(__cpucode)
foc1b		  equ	  2
foc1a		  equ	  3
		 endif

tccr1b           port    0x2e           ; Timer 1 Control Register B:
cs10             equ     0              ; Prescale setting
cs11             equ     1
cs12             equ     2
ctc1             equ     3              ; Reset after Equality ?
wgm12		 equ	 ctc1
		 if      ismega(__cpucode)
wgm13		  equ	  4
		 endif
ices1            equ     6              ; Capture Edge Selection
icnc1            equ     7              ; Capture Noise Filter

tcnt1l           port    0x2c           ; Timer 1 Count Register (LSB)
tcnt1h           port    0x2d           ;                        (MSB)

                 if      __cpucode>=0x014414
ocr1al            port    0x2a          ; Timer 1 Compare Register A (LSB)
ocr1ah            port    0x2b          ;                            (MSB)
ocr1bl            port    0x28          ; Timer 1 Compare Register B (LSB)
ocr1bh            port    0x29          ;                            (MSB)
                 elseif
ocr1l             port    0x2a          ; Timer 1 Compare Register (LSB)
ocr1h             port    0x2b          ;                          (MSB)
                 endif

		 if	 ismega(__cpucode)
icr1l             port    0x26
icr1h             port    0x27
		 elseif
icr1l             port    0x24          ; Timer 1 Capture Value (LSB)
icr1h             port    0x25          ;                       (MSB)
		 endif
                endif

		if 	ismega(__cpucode)
tccr2		 port	 0x26		; Timer 2 Control Register
cs20		 equ	 0		; Prescaler
cs21		 equ	 1
cs22		 equ	 2
wgm21		 equ	 3
com20		 equ	 4
com21		 equ	 5
wgm20		 equ	 6
foc2		 equ	 7

tcnt2		 port	 0x24		; Timer Value
ocr2		 port	 0x23		; Output Compare Value

assr		 port	 0x22		; Asynchronous Status Register
as2		 equ	 3		; Asynchronous Timer 2
tcn2ub		 equ	 2		; Timer/Counter 2 Update Busy
ocr2ub		 equ	 1		; Output Compare Register 2 Update Busy
tcr2ub		 equ	 0		; Timer/Counter Control Register 2 Update Busy
		endif

;----------------------------------------------------------------------------
; Watchdog

wdtcr           port    0x21            ; Watchdog-Control Register:
wdp0            equ     0               ;  Prescaler
wdp1            equ     1
wdp2            equ     2
wde             equ     3               ; Enable
		if	__cpucode=0x010008
wdce		  equ	  4		; Watchdog Change Enable
                elseif 	__cpucode>=0x012313
wdttoe		  equ	  4		; Needed for Disable
                endif

;----------------------------------------------------------------------------
; serielle Ports

                if      __cpucode>=0x012312
udr              port    0x0c           ; Data Register UART

usr              port    0x0b           ; Status Register UART:
ucsra		 port	 usr
		 if	 ismega(__cpucode)
mpcm		  equ	  0		; Multiprocessor Communication Mode
u2x		  equ	  1		; Double Speed
pe		  equ	  2		; Parity Error
		 endif
or               equ     3              ;  Receiver Overflow
dor		 equ	 or
fe               equ     4              ;  Framing Error
udre             equ     5              ;  Data Register Empty
txc              equ     6              ;  Transmission Complete
rxc              equ     7              ;  Reception Complete

ucr              port    0x0a           ; UART Control Register:
ucsrb		 port	 ucr
txb8             equ     0              ;  Transmit Bit 8
rxb8             equ     1              ;  Receive Bit 8
chr9             equ     2              ;  Enable 9 Bit Data Values
ucsz2		 equ	 chr9
txen             equ     3              ;  Transmitter Enable
rxen             equ     4              ;  Receiver Enable
udrie            equ     5              ;  Enable Free Data Register Interrupt
txcie            equ     6              ;  Enable Transmit Complete Interrupt
rxcie            equ     7              ;  Enable Receive Complete Interrupt

		 if      ismega(__cpucode)
ucsrc		  port	  0x20		; Control Register C
ursel		  equ	  7		; Register Select
umsel		  equ	  6		; Sync/Async Mode
upm1		  equ	  5		; Parity Mode
upm0		  equ	  4
usbs		  equ	  3		; Stop Bit Select
ucsz1		  equ	  2		; Character Size
ucsz0		  equ	  1
ucpol		  equ	  0		; Clock Polarity
		 endif

ubrr             port    0x09           ; Baud Rate Generator
		 if	 ismega(__cpucode)
ubrrl		  port	  ubrr
ubrrh		  port	  ucsrc
		 endif
                endif

                if      __cpucode>=0x014414
spcr             port    0x0d           ; SPI Control Register:
spr0             equ     0              ;  Clock Selection
spr1             equ     1
cpha             equ     2              ;  Clock Phase
cpol             equ     3              ;  Clock Polarity
mstr             equ     4              ;  Master/Slave Select
dord             equ     5              ;  Bit Order
spe              equ     6              ;  SPI Enable
spie             equ     7              ;  SPI Interrupt Enable

spsr             port    0x0e           ; SPI Status Register:
		 if	 ismega(__cpucode)
spi2x		  equ	  0		; Double Speed Mode
		 endif
wcol             equ     6              ;  Write Collision ?
spif             equ     7              ;  SPI Interrupt Flag

spdr             port    0x0f           ; SPI Data Register
                endif

		if	ismega(__cpucode)
twbr		 port	 0x00		; Bit Rate Register

twcr		 port	 0x36		; Control Register
twint		 equ	 7		; Interrupt Flag
twea		 equ	 6		; Enable Acknowledge Bit
twsta		 equ	 5		; Start Condition
twsto		 equ	 4		; Stop Condition
twwc		 equ	 3		; Write Collision Flag
twen		 equ	 2		; Enable Bit
twie		 equ	 0		; Interupt Enable

twsr		 port	 0x01		; Status Register
tws7		 equ	 7		; Status
tws6		 equ	 6
tws5		 equ	 5
tws4		 equ	 4
tws3		 equ	 3
twps1		 equ	 1		; Prescaler
twps0		 equ	 0

twdr		 port	 0x03		; Data Register

twar		 port	 0x02		; (Slave) Address Register
twgce		 equ	 0		; General Call recognition Bit
		endif

;----------------------------------------------------------------------------
; Analog Comparator

acsr            port    0x08            ; Comparator Control/Status Register:
acis0           equ     0               ;  Interrupt Mode
acis1           equ     1
		if	__cpucode>=0x012313
acic             equ     2              ;  Use Comparator as Capture Signal for Timer 1
		endif
acie            equ     3               ;  Interrupt Enable
aci             equ     4               ;  Interrupt Flag
aco             equ     5               ;  Comparator Output
		if      ismega(__cpucode)
acbg		 equ	 6		;  Bandgap Select
		endif
acd		equ	7		;  Power Off

;----------------------------------------------------------------------------
; A/D Converter

		if      ismega(__cpucode)
admux		 port	 0x07		; Multiplexer Selection
refs1		 equ	 7		; reference Selection Bits
refs0		 equ	 6
adlar		 equ	 5		; Left Adjust Right
mux3		 equ	 3		; Multiplexer
mux2		 equ	 2
mux1		 equ	 1
mux0		 equ	 0

		 if 	 __cpucode >=0x020010
adcsra		  port	  0x06		; Control/Status Register
adate		  equ	  5		; Auto Trigger Enable
		 elseif
adcsr		  port	  0x06		; Control/Status Register
adfr		  equ	  5		; free Running Select
		  endif
aden		 equ	 7		; Enable ADC
adsc		 equ	 6		; Start Conversion
adif		 equ	 4		; Interrupt Flag
adie		 equ	 3		; Interrupt Enable
adps2		 equ	 2		; Prescaler Select
adps1		 equ	 1
adps0		 equ	 0

adch		 port	 0x05		; Data Register
adcl		 port	 0x04
		endif

;----------------------------------------------------------------------------
; JTAG

		if	__cpucode >=0x020010
ocdr		 port	 osccal		; Debug Register
		endif

;----------------------------------------------------------------------------
; EEPROM

		if eeend>=256
eearl		 port	 0x1e		; Address Register
eearh		 port	 0x1f
		elseif 
eear             port    0x1e
		endif

eedr            port    0x1d            ; Data Register

eecr            port    0x1c            ; Control Register:
eere            equ     0               ;  Read Enable
eewe            equ     1               ;  Write Enable
		if	__cpucode>=0x012313
eemwe		 equ	 2
		endif
		if 	__cpucode=0x020008
eerie		 equ	 3		; Enable Ready Interrupt
		endif

;----------------------------------------------------------------------------
; Vectors
; Unfortunately, interrupt numbers change for biggger processors
; Why only, Atmel, why ?

vec_reset       label   0               ; Reset Entry
                switch  __cpucode
                case    0x011200
vec_int0         label   1              ; Entry External Interrupt 0
vec_tm0ovf       label   2              ; Entry Overflow Timer 0
vec_anacomp      label   3              ; Entry Analog Comparator
                case    0x012313
vec_int0         label   1              ; Entry External Interrupt 0
vec_int1         label   2              ; Entry External Interrupt 2
vec_tm1capt      label   3              ; Entry Capture Timer 1
vec_tm1comp      label   4              ; Entry Compare Timer 1
vec_tm1ovf       label   5              ; Entry Overflow Timer 1
vec_tm0ovf       label   6              ; Entry Overflow Timer 0
vec_uartrx       label   7              ; Entry UART Empfang komplett
vec_uartudre     label   8              ; Entry UART Data Register leer
vec_uarttx       label   9              ; Entry UART Sendung komplett
vec_anacomp      label   10             ; Entry Analog-Komparator
                case    0x014414,0x018515
vec_int0         label   1              ; Entry External Interrupt 0
vec_int1         label   2              ; Entry External Interrupt 2
vec_tm1capt      label   3              ; Entry Capture Timer 1
vec_tm1compa     label   4              ; Entry Compare A Timer 1
vec_tm1compb     label   5              ; Entry Compare A Timer 1
vec_tm1ovf       label   6              ; Entry Overflow Timer 1
vec_tm0ovf       label   7              ; Entry Overflow Timer 0
vec_spi          label   8              ; Entry SPI Interrupt
vec_uartrx       label   9              ; Entry UART Reception Complete
vec_uartudre     label   10             ; Entry UART Data Register Empty
vec_uarttx       label   11             ; Entry UART Sendung Complete
vec_anacomp      label   12             ; Entry Analog Comparator
		case	0x020008
vec_int0         label   1              ; Entry External Interrupt 0
vec_int1	 label	 2		; External Interrupt 1
vec_tm2comp	 label 	 3 		; Timer 2 Compare Match
vec_tm2ovf	 label	 4		; Timer 2 Overflow
vec_tm1capt	 label	 5		; Timer 1 Capture
vec_tm1compa	 label	 6		; Timer 1 Compare Match A
vec_tm1compb	 label	 7		; Timer 1 Compare Match B
vec_tm1ovf	 label	 8		; Timer 1 Overflow
vec_tm0ovf	 label	 9		; Timer 0 Overflow
vec_spi		 label   10		; SPI Transfer Complete
vec_uartrx	 label	 11		; UART Rx Complete
vec_uartudre	 label   12		; UART Data Register Empty
vec_uarttx	 label	 13		; UART Tx Complete
vec_adc		 label	 14		; ADC Conversion Complete
vec_eerdy	 label	 15		; EEPROM Ready
vec_anacomp	 label	 16		; analog Comparator
vec_twi		 label	 17		; Two-Wire Interface
vec_spm_rdy	 label	 18		; Store Program Memory Ready
		case	0x020010
vec_int0	 label	 2		; External Interrupt 0
vec_int1	 label	 4		; External Interrupt 1
vec_tm2comp	 label 	 6 		; Timer 2 Compare Match
vec_tm2ovf	 label	 8		; Timer 2 Overflow
vec_tm1capt	 label	 10		; Timer 1 Capture
vec_tm1compa	 label	 12		; Timer 1 Compare Match A
vec_tm1compb	 label	 14		; Timer 1 Compare Match B
vec_tm1ovf	 label	 16		; Timer 1 Overflow
vec_tm0ovf	 label	 18		; Timer 0 Overflow
vec_spi		 label   20		; SPI Transfer Complete
vec_uartrx	 label	 22		; UART Rx Complete
vec_uartudre	 label   24		; UART Data Register Empty
vec_uarttx	 label	 26		; UART Tx Complete
vec_adc		 label	 28		; ADC Conversion Complete
vec_eerdy	 label	 30		; EEPROM Ready
vec_anacomp	 label	 32		; analog Comparator
vec_twi		 label	 34		; Two-Wire Interface
vec_int2	 label	 36		; External Interrupt 2
vec_tm0comp	 label	 38		; Timer 0 Compare Match
vec_spm_rdy	 label	 40		; Store Program Memory Ready
                endcase

;----------------------------------------------------------------------------

                restore                 ; re-allow listing

                endif			; __regavrinc
