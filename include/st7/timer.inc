		ifndef	__st7timerinc	; avoid multiple inclusion
__st7timerinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File TIMER.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72xxx Timer                     *
;*                                                                          *
;****************************************************************************

__deftimer	macro	Name,Base
__NS		set	"\{NAME}"
{__NS}CR2	label	Base+$01	; Control Register 2
{__NS}OC1E	bit	{__NS}CR2,7	;  Output Compare 1 Pin Enable
{__NS}OC2E	bit	{__NS}CR2,6	;  Output Compare 2 Enable
{__NS}OPM	bit	{__NS}CR2,5	;  One Pulse Mode
{__NS}PWM	bit	{__NS}CR2,4	;  Pulse Width Modulation
{__NS}CC	bfield	{__NS}CR2,2,2	;  Clock Control
{__NS}IEDG2	bit	{__NS}CR2,1	;  Input Edge 2
{__NS}EXEDG	bit	{__NS}CR2,0	;  External Clock Edge
{__NS}CR1	label	Base+$02	; Control Register 1
{__NS}ICIE	bit	{__NS}CR1,7	;  Input Capture Interrupt Enable
{__NS}OCIE	bit	{__NS}CR1,6	;  Output Compare Interrupt Enable
{__NS}TOIE	bit	{__NS}CR1,5	;  Timer Overflow Interrupt Enable
{__NS}FOLV2	bit	{__NS}CR1,4	;  Forced Output Compare 2
{__NS}FOLV1	bit	{__NS}CR1,3	;  Forced Output Compare 1
{__NS}OLVL2	bit	{__NS}CR1,2	;  Output Level 2
{__NS}IEDG1	bit	{__NS}CR1,1	;  Input Edge 1
{__NS}OLVL1	bit	{__NS}CR1,0	;  Output Level 1
{__NS}SR	label	Base+$03	; Status Register
{__NS}ICF1	bit	{__NS}SR,7	;  Input Capture Flag 1
{__NS}OCF1	bit	{__NS}SR,6	;  Output Compare Flag 1
{__NS}TOF	bit	{__NS}SR,5	;  Timer Overflow
{__NS}ICF2	bit	{__NS}SR,4	;  Input Capture Flag 2
{__NS}OCF2	bit	{__NS}SR,3	;  Output Compare Flag 2
{__NS}IC1HR	label	Base+$04	; Input Capture 1 High Register
{__NS}IC1LR	label	Base+$05	; Input Capture 1 Low Register
{__NS}OC1HR	label	Base+$06	; Output Compare 1 High Register
{__NS}OC1LR	label	Base+$07	; Output Compare 1 Low Register
{__NS}CHR	label	Base+$08	; Counter High Register
{__NS}CLR	label	Base+$09	; Counter Low Register
{__NS}ACHR	label	Base+$0a	; Alternate Counter High Register
{__NS}ACLR	label	Base+$0b	; Alternate Counter Low Register
{__NS}IC2HR	label	Base+$0c	; Input Capture 2 High Register
{__NS}IC2LR	label	Base+$0d	; Input Capture 2 Low Register
{__NS}OC2HR	label	Base+$0e	; Output Compare 2 High Register
{__NS}OC2LR	label	Base+$0f	; Output Compare 2 Low Register
		endm

		restore
		endif			; __st7timerinc
