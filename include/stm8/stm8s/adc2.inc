		ifndef	__stm8sadc2inc	; avoid multiple inclusion
__stm8sadc2inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File ADC2.INC                                                *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S ADC2                        *
;*                                                                          *
;****************************************************************************

__defadc2	macro	NAME,BUFBASE,Base
__NS		set	"\{NAME}_"
		if	"BUFBASE"<>""
{__NS}DB0R	label	BUFBASE		; ADC data buffer registers
{__NS}DB1R	label	BUFBASE+1
{__NS}DB2R	label	BUFBASE+2
{__NS}DB3R	label	BUFBASE+3
		endif
{__NS}CSR	label	Base+$00	; ADC control/status register
{__NS}EOC	bit	{__NS}CSR,7	;  End of conversion
{__NS}AWD	bit	{__NS}CSR,6	;  Analog Watchdog flag
{__NS}EOCIE	bit	{__NS}CSR,5	;  Interrupt enable for EOC
{__NS}AWDIE	bit	{__NS}CSR,4	;  Analog watchdog interrupt enable
{__NS}CH	bfield	{__NS}CSR,0,4	;  Channel selection bits
{__NS}CR1	label	Base+$01	; ADC configuration register 1
{__NS}SPSEL	bfield	{__NS}CR1,4,3	;  Prescaler selection
{__NS}CONT	bit	{__NS}CR1,1	;  Continuous conversion
{__NS}ADON	bit	{__NS}CR1,0	;  A/D Converter on/off
{__NS}CR2	label	Base+$02	; ADC configuration register 2
{__NS}EXTTRIG	bit	{__NS}CR2,6	;  External trigger enable
{__NS}EXTSEL	bfield	{__NS}CR2,4,2	;  External event selection
{__NS}ALIGN	bit	{__NS}CR2,3	;  Data alignment
{__NS}SCAN	bit	{__NS}CR2,1	;  Scan mode enable
{__NS}CR3	label	Base+$03	; ADC configuration register 3
{__NS}DBUF	bit	{__NS}CR3,7	;  Data buffer enable
{__NS}OVR	bit	{__NS}CR3,6	;  Overrun flag
{__NS}DRH	label	Base+$04	; ADC data register high
{__NS}DRL	label	Base+$05	; ADC data register low
{__NS}TDRH	label	Base+$06	; ADC Schmitt trigger disable register high
{__NS}TDRL	label	Base+$07	; ADC Schmitt trigger disable register low
		endm

		restore
		endif			; __stm8sadc2inc
