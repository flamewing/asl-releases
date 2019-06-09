		ifndef	__stm8ldmainc	; avoid multiple inclusion
__stm8ldmainc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File DMA.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8L DMA                         *
;*                                                                          *
;****************************************************************************

__defdma	macro	NAME,Base
__NS		set	"\{NAME}_"
{__NS}GCSR	label	Base+$00	; global configuration & status register
{__NS}TO	bfield	{__NS}GCSR,2,6	;  Timeout
{__NS}GB	bit	{__NS}GCSR,1	;  Global busy
{__NS}GEN	bit	{__NS}GCSR,0	;  Global enable
{__NS}GIR1	label	Base+$01	; global interrupt register 1
{__NS}IFC3	bit	{__NS}GIR1,3	;  Interrupt flag channel 3
{__NS}IFC2	bit	{__NS}GIR1,2	;  Interrupt flag channel 2
{__NS}IFC1	bit	{__NS}GIR1,1	;  Interrupt flag channel 1
{__NS}IFC0	bit	{__NS}GIR1,0	;  Interrupt flag channel 0
__N		set	0
		rept	4
__NC		set	"\{NAME}_C\{__N}"
{__NC}CR	label	Base+__N*10+$05	; channel n configuration register
{__NC}MINCDEC	bit	{__NC}CR,5	;  Memory increment/decrement mode
{__NC}CIRC	bit	{__NC}CR,4	;  Circular buffer mode
{__NC}DIR	bit	{__NC}CR,3	;  Data transfer direction
{__NC}HTIE	bit	{__NC}CR,2	;  Half-transaction interrupt enable
{__NC}TCIE	bit	{__NC}CR,1	;  Transaction complete interrupt enable
{__NC}EN	bit	{__NC}CR,0	;  Channel enable
{__NC}SPR	label	Base+__N*10+$06	; channel n status & priority register
{__NC}BUSY	bit	{__NC}SPR,7	;  Channel busy
{__NC}PEND	bit	{__NC}SPR,6	;  Channel pending
{__NC}PL	bfield	{__NC}SPR,4,2	;  Channel priority level
{__NC}TSIZE	bit	{__NC}SPR,3	;  Transfer size
{__NC}HTIF	bit	{__NC}SPR,2	;  Half transaction interrupt flag
{__NC}TCIF	bit	{__NC}SPR,1	;  Transaction complete interrupt flag
{__NC}NDTR	label	Base+__N*10+$07	; channel n number of data to transfer register
{__NC}PARH	label	Base+__N*10+$08	; channel n peripheral address high register
{__NC}PARL	label	Base+__N*10+$09	; channel n peripheral address low register
{__NC}M0ARH	label	Base+__N*10+$0b	; channel n memory 0 address high register
{__NC}M0ARL	label	Base+__N*10+$0c	; channel n memory 0 address low register
__N		set	__N+1
		endm
{__NS}C3MEM	bit	{__NS}C3CR,6	;  Memory transfer enabled (ch 3 only)
		endm

		restore
		endif			; __stm8ldmainc
