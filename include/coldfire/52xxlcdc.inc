		ifndef	__52xxlcdcinc		; avoid multiple inclusion
__52xxlcdcinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXLCDC.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx LCD Controller   *
;*                                                                          *
;****************************************************************************

LCD_SSAR	equ		MBAR_LCDC+$000	; Screen Start Address Register (32b)
SSA		cffield		LCD_SSAR2,30	;  Screen start address of the LCD panel (bits 2..31).
LCD_SR		equ		MBAR_LCDC+$004	; LCD Size Register (32b)
XMAX		cffield		LCD_SR,20,6	;  Screen width divided by 16.
YMAX		cffield		LCD_SR,0,10	;  Screen height.
LCD_VPW		equ		MBAR_LCDC+$008	; LCD Virtual Page Width Register (32b)
VPW		cffield		LCD_VPW,0,10	;  Virtual page width.
LCD_CPR		equ		MBAR_LCDC+$00C	; LCD Cursor Position Register (32b)
CC		cffield		LCD_CPR,30,2	;  Cursor control.
OP		cfbit		LCD_CPR,28	;  Arithmetic operation control.
CXP		cffield		LCD_CPR,16,10	;  Cursor X-position.
CYP		cffield		LCD_CPR,0,10	;  Cursor Y-position.
LCD_CWHB	equ		MBAR_LCDC+$010	; LCD Cursor Width Height and Blink Register (32b)
BK_EN		cfbit		LCD_CWHB,31	;  Blink enable.
CW		cffield		LCD_CWHB,24,5	;  Cursor width.
CH		cffield		LCD_CWHB,16,5	;  Cursor height.
BD		cffield		LCD_CWHB,0,8	;  Blink divisor.
LCD_CCMR	equ		MBAR_LCDC+$014	; LCD Color Cursor Mapping Register (32b)
CUR_COL_R	cffield		LCD_CCMR,12,6	;  Cursor red field.
CUR_COL_G	cffield		LCD_CCMR,6,6	;  Cursor green field.
CUR_COL_B	cffield		LCD_CCMR,0,6	;  Cursor blue field.
LCD_PCR		equ		MBAR_LCDC+$018	; LCD Panel Configuration Register (32b)
TFT		cfbit		LCD_PCR,31	;  TFT display enable.
COLOR		cfbit		LCD_PCR,30	;  Color display enable.
PBSIZ		cffield		LCD_PCR,28,2	;  Panel bus width.
BPIX		cffield		LCD_PCR,25,3	;  Indicates the number of bits per pixel in memory.
PIXPOL		cfbit		LCD_PCR,24	;  Pixel polarity.
FLMPOL		cfbit		LCD_PCR,23	;  First line marker polarity.
LPPOL		cfbit		LCD_PCR,22	;  Line pulse polarity.
CLKPOL		cfbit		LCD_PCR,21	;  LCD shift clock polarity.
OEPOL		cfbit		LCD_PCR,20	;  LCD output enable polarity.
SCLKIDLE	cfbit		LCD_PCR,19	;  LCD SCLK idle enable.
END_SEL		cfbit		LCD_PCR,18	;  Endian select.
SWAP_SEL	cfbit		LCD_PCR,17	;  Swap Select.
REV_VS		cfbit		LCD_PCR,16	;  Reverse vertical scan.
ACDSEL		cfbit		LCD_PCR,15	;  LCD_ACD clock source select.
ACD		cffield		LCD_PCR,8,7	;  Alternate crystal direction.
SCLKSEL		cfbit		LCD_PCR,7	;  LCD_LSCLK select.
SHARP		cfbit		LCD_PCR,6	;  Sharp panel enable.
PCD		cffield		LCD_PCR,0,6	;  Pixel clock divider.
LCD_HCR		equ		MBAR_LCDC+$01C	; LCD Horizontal Configuration Register (32b)
H_WIDTH		cffield		LCD_HCR,26,6	;  Horizontal sync pulse width.
H_WAIT_1	cffield		LCD_HCR,8,8	;  Wait between LCD_OE and LCD_HSYNC.
H_WAIT_2	cffield		LCD_HCR,0,8	;  Wait between LCD_HSYNC and start of next line.
LCD_VCR		equ		MBAR_LCDC+$020	; LCD Vertical Configuration Register (32b)
V_WIDTH		cffield		LCD_VCR,26,6	;  Vertical sync pulse width.
V_WAIT_1	cffield		LCD_VCR,8,8	;  Wait between frames 1.
V_WAIT_2	cffield		LCD_VCR,0,8	;  Wait between frames 2.
LCD_POR		equ		MBAR_LCDC+$024	; LCD Panning Offset Register (32b)
POS		cffield		LCD_POR,0,5	;  Panning offset.
LCD_SCR		equ		MBAR_LCDC+$028	; LCD Sharp Configuration Register (32b)
PS_RISE_DELAY	cffield		LCD_SCR,26,6	;  LCD_PS rise delay.
CLS_RISE_DELAY	cffield		LCD_SCR,16,8	;  LCD_CLS rise delay.
REV_TOG_GLE_DELAY	cffield	LCD_SCR,8,4	;  LCD_REV toggle delay.
GRAY2		cffield		LCD_SCR,4,4	;  Grayscale 2.
GRAY1		cffield		LCD_SCR,0,4	;  Grayscale 1.
LCD_PCCR	equ		MBAR_LCDC+$02C	; LCD PWM Contrast Control Register (32b)
CLS_HI_WIDTH	cffield		LCD_PCCR,16,9	;  LCD_CLS high pulse width.
LDMSK		cfbit		LCD_PCCR,15	;  LD mask.
SCR		cffield		LCD_PCCR,9,2	;  Source select.
CC_EN		cfbit		LCD_PCCR,8	;  Contrast control enable.
PW		cffield		LCD_PCCR,0,8	;  Pulse width.
LCD_DCR		equ		MBAR_LCDC+$030	; LCD DMA Control Register (32b)
BURST		cfbit		LCD_DCR,31	;  Burst length.
HM		cffield		LCD_DCR,16,5	;  DMA high mark.
TM		cffield		LCD_DCR,0,5	;  DMA trigger mark.
LCD_RMCR	equ		MBAR_LCDC+$034	; LCD Refresh Mode Control Register (32b)
SELF_REF	cfbit		LCD_RMCR,0	;  Self-refresh mode enable.
LCD_ICR		equ		MBAR_LCDC+$038	; LCD Interrupt Configuration Register (32b)
GW_INT_CON	cfbit		LCD_ICR,4	;  Graphic window interrupt condition.
INT_SYN		cfbit		LCD_ICR,2	;  Interrupt source.
INT_CON		cfbit		LCD_ICR,0	;  Interrupt condition.
LCD_IER		equ		MBAR_LCDC+$03C	; LCD Interrupt Enable Register (32b)
GWUDR		cfbit		LCD_IER,7	;  Graphic window underrun error interrupt enable.
GWERR		cfbit		LCD_IER,6	;  Graphic window error response interrupt enable.
GWEOF		cfbit		LCD_IER,5	;  Graphic window end-of-frame interrupt enable.
GWBOF		cfbit		LCD_IER,4	;  Graphic window beginning-of-frame interrupt enable.
UDR		cfbit		LCD_IER,3	;  Underrun error interrupt enable.
ERR		cfbit		LCD_IER,2	;  Error response interrupt enable.
EOF		cfbit		LCD_IER,1	;  End-of-frame interrupt enable.
BOF		cfbit		LCD_IER,0	;  Beginning-of-frame interrupt enable.
LCD_ISR		equ		MBAR_LCDC+$040	; LCD Interrupt Status Register (32b)
GWUDR		cfbit		LCD_ISR,7	;  Graphic window underrun error.
GWERR		cfbit		LCD_ISR,6	;  Graphic window error response interrupt.
GWEOF		cfbit		LCD_ISR,5	;  Graphic window end-of-frame interrupt.
GWBOF		cfbit		LCD_ISR,4	;  Graphic window beginning-of-frame interrupt.
UDR		cfbit		LCD_ISR,3	;  Underrun error interrupt.
ERR		cfbit		LCD_ISR,2	;  Error response interrupt.
EOF		cfbit		LCD_ISR,1	;  End-of-frame interrupt.
BOF		cfbit		LCD_ISR,0	;  Beginning-of-frame interrupt.
LCD_GWSAR	equ		MBAR_LCDC+$050	; LCD Graphic Window Start Address Register (32b)
GWSA		cffield		LCD_GWSAR,2,30	;  Graphic window start address on LCD screen (b2..31).
LCD_GWSR	equ		MBAR_LCDC+$054	; LCD Graphic Window Size Register (32b)
GW_XMAX		cffield		LCD_GWSR,20,6	;  Graphic window width divided by 16.
GW_YMAX		cffield		LCD_GWSR,0,10	;  Graphic window height.
LCD_GWVPW	equ		MBAR_LCDC+$058	; LCD Graphic Window Virtual Page Width Register (32b)
GWVPW		cffield		LCD_GWVPW,0,10	;  Graphic window virtual page width.
LCD_GWPOR	equ		MBAR_LCDC+$05C	; LCD Graphic Window Panning Offset Register (32b)
GWPO		cffield		LCD_GWPOR,0,5	;  Graphic window panning offset.
LCD_GWPR	equ		MBAR_LCDC+$060	; LCD Graphic Window Position Register (32b)
GWXP		cffield		LCD_GWPR,16,10	;  Graphic window X-position.
GWYP		cffield		LCD_GWPR,0,10	;  Graphic window Y-position.
LCD_GWCR	equ		MBAR_LCDC+$064	; LCD Graphic Window Control Register (32b)
GWAV		cffield		LCD_GWCR,24,8	;  Graphic window alpha value.
GWCKE		cfbit		LCD_GWCR,23	;  Graphic window color keying enable.
GWE		cfbit		LCD_GWCR,22	;  Graphic window enable.
GW_RVS		cfbit		LCD_GWCR,21	;  Graphic window reverse vertical scan.
GWCKR		cffield		LCD_GWCR,12,6	;  Graphic window color keying red component.
GWCKG		cffield		LCD_GWCR,6,6	;  Graphic window color keying green component.
GWCKB		cffield		LCD_GWCR,0,6	;  Graphic window color keying blue component.
LCD_GWDCR	equ		MBAR_LCDC+$068	; LCD Graphic Window DMA Control Register (32b)
GWBT		cfbit		LCD_GWDCR,31	;  Graphic window DMA burst type.
GWHM		cffield		LCD_GWDCR,16,5	;  Graphic window DMA high mark.
GWTM		cffield		LCD_GWDCR,0,5	;  Graphic window DMA trigger mark.
BGLUT		equ		MBAR_LCDC+$800	; Background Look-up Table (256*32b)
GPM		cffield		BGLUT,0,4	;  Gray palette map.
GWLUT		equ		MBAR_LCDC+$C00	; Graphic Window Look-up Table (256*32b)
R6		cffield		GWLUT,12,6	;  Red level (color display).
G6		cffield		GWLUT,6,6	;  Green level (color display).
B6		cffield		GWLUT,0,6	;  Blue level (color display).
R4		cffield		GWLUT,8,4	;  Red level (color display).
G4		cffield		GWLUT,4,4	;  Green level (color display).
B4		cffield		GWLUT,0,4	;  Blue level (color display).

		restore				; re-enable listing

                endif                           ; __52xxlcdcinc
