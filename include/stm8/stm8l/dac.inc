		ifndef	__stm8ldacinc	; avoid multiple inclusion
__stm8ldacinc	equ	1

		save
		listing	on		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File DAC.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L D/A Converters              *
;*                                                                          *
;****************************************************************************

__defdac	macro	Base,NumChan,DualMode
DAC_SWTRIGR	label	Base+$04	; DAC software trigger register
DAC_SR		label	Base+$05	; DAC status register
__CHAN		set	0
		rept	NumChan
		if	NumChan>1
__NS		set	"DAC_CH\{__CHAN+1}"
		elseif
__NS		set	"DAC_"
		endif
CHBase		set	Base+(__CHAN*2)
{__NS}CR1	label	CHBase+$00	; DAC control register 1
{__NS}WAVEN	bfield	{__NS}CR1,6,2	;  DAC channel x noise/triangle waveform generation enable
{__NS}TSEL	bfield	{__NS}CR1,3,3	;  DAC channel x trigger selection
{__NS}TEN	bit	{__NS}CR1,2	;  DAC channel trigger enable
{__NS}BOFF	bit	{__NS}CR1,1	;  DAC channel output buffer disable
{__NS}EN	bit	{__NS}CR1,0	;  DAC channel enable
{__NS}CR2	label	CHBase+$01	; DAC control register 2
{__NS}DMAUDRIE	bit	{__NS}CR2,5	;  DAC channel DMA underrun interrupt enable
{__NS}DMAEN	bit	{__NS}CR2,4	;  DAC DMA enable
{__NS}MAMP	bfield	{__NS}CR2,0,4	;  DAC channel x mask/amplitude selector
{__NS}SWTRIG{"\{__CHAN+1}"} bit DAC_SWTRIGR,__CHAN ; DAC channel n software trigger
{__NS}UDR{"\{__CHAN+1}"} bit DAC_SR,__CHAN ; DAC channel n DMA underrun flag
CHBase		set	Base+(__CHAN*12)
{__NS}RDHRH	label	CHBase+$08	; DAC right aligned data holding register high
{__NS}RDHRL	label	CHBase+$09	; DAC right aligned data holding register low
{__NS}LDHRH	label	CHBase+$0c	; DAC left aligned data holding register high
{__NS}LDHRL	label	CHBase+$0d	; DAC left aligned data holding register low
{__NS}DHR8	label	CHBase+$10	; DAC 8-bit data holding register
CHBase		set	Base+(__CHAN*4)
{__NS}DORH	label	CHBase+$2c	; DAC data output register high
{__NS}DORL	label	CHBase+$2d	; DAC data output register low
		if	DualMode	; Dual mode only makes sense with 2 channels!
__DNS		set	"DAC_DCH\{__CHAN+1}"
CHBase          set     Base+(__CHAN*2)
{__DNS}RDHRH	label	CHBase+$20	; DAC ddual mode right aligned data holding register high
{__DNS}RDHRL	label	CHBase+$21	; DAC ddual mode right aligned data holding register low
{__DNS}LDHRH	label	CHBase+$24	; DAC ddual mode left aligned data holding register high
{__DNS}LDHRL	label	CHBase+$25	; DAC ddual mode left aligned data holding register low
{__DNS}DHR8	label	Base+$28+__CHAN	; DAC ddual mode 8-bit data holding register
		endif
__CHAN		set	__CHAN+1
		endm
		endm

		restore
		endif			; __stm8ldacinc
