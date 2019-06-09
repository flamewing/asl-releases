		ifndef	__stm8ladc1inc	; avoid multiple inclusion
__stm8ladc1inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADC1.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L ADC1                        *
;*                                                                          *
;****************************************************************************

__defadc1	macro	NAME,BUFBASE,Base
__NS		set	"\{NAME}_"
		if	"BUFBASE"<>""
{__NS}DB0R	label	BUFBASE		; ADC data buffer registers
{__NS}DB1R	label	BUFBASE+1
{__NS}DB2R	label	BUFBASE+2
{__NS}DB3R	label	BUFBASE+3
		endif
{__NS}CR1	label	Base+$00	; ADC configuration register 1
{__NS}OVERIE	bit	{__NS}CR1,7	;  Overrun interrupt enable
{__NS}RES	bfield	{__NS}CR1,5,2	;  Configurable resolution
{__NS}AWDIE	bit	{__NS}CR1,4	;  Analog watchdog interrupt enable
{__NS}EOCIE	bit	{__NS}CR1,3	;  Interrupt enable for EOC
{__NS}CONT	bit	{__NS}CR1,2	;  Continuous conversion
{__NS}START	bit	{__NS}CR1,1	;  Conversion start
{__NS}ADON	bit	{__NS}CR1,0	;  A/D Converter on/off
{__NS}CR2	label	Base+$01	; ADC configuration register 2
{__NS}PRESC	bit	{__NS}CR2,7	;  Clock prescaler
{__NS}TRIG_EDGE	bfield	{__NS}CR2,5,2	;  Active edge for external triggers
{__NS}EXTSEL	bfield	{__NS}CR2,3,2	;  External event selection
{__NS}SMTP	bfield	{__NS}CR2,0,3	;  Sampling time selection
{__NS}CR3	label	Base+$02	; ADC configuration register 3
{__NS}SMTP2	bfield	{__NS}CR3,5,3	;  Sampling time selection
{__NS}CHSEL	bfield	{__NS}CR3,0,5	;  Channel selection
{__NS}SR	label	Base+$03	; ADC status register
{__NS}OVER	bit	{__NS}SR,2	;  Overrun flag
{__NS}AWD	bit	{__NS}SR,1	;  Analog watchdog flag
{__NS}EOC	bit	{__NS}SR,0	;  End of conversion
{__NS}DRH	label	Base+$04	; ADC data register high
{__NS}DRL	label	Base+$05	; ADC data register low
{__NS}HTRH	label	Base+$06	; ADC high threshold register high
{__NS}HTRL	label	Base+$07	; ADC high threshold register low
{__NS}LTRH	label	Base+$08	; ADC low threshold register high
{__NS}LTRL	label	Base+$09	; ADC low threshold register low
{__NS}SQR1	label	Base+$0a	; ADC channel sequence 1 register
{__NS}DMAOFF	bit	{__NS}SQR1,7	;  DMA disable for a single conversion
{__NS}CHSEL_STS	bit	{__NS}SQR1,5	;  Selection of channel TS for scan
{__NS}CHSEL_SVREFINT bit {__NS}SQR1,4	;  Selection of channel VREFINT for scan
{__NS}SQR2	label	Base+$0b	; ADC channel sequence 2 register
{__NS}SQR3	label	Base+$0c	; ADC channel sequence 3 register
{__NS}SQR4	label	Base+$0d	; ADC channel sequence 4 register
{__NS}TRIGR1	label	Base+$0e	; ADC trigger disable 1
{__NS}TSON	bit	{__NS}TRIGR1,5	;  Temperature sensor internal reference voltage enable
{__NS}VREFINTON	bit	{__NS}TRIGR1,4	;  Internal reference voltage enable
{__NS}TRIGR2	label	Base+$0f	; ADC trigger disable 2
{__NS}TRIGR3	label	Base+$10	; ADC trigger disable 3
{__NS}TRIGR4	label	Base+$11	; ADC trigger disable 4
		endm

		restore
		endif			; __stm8ladc1inc
