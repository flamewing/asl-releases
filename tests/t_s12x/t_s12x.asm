		CPU	68HC12X

;###########################################
;# SYMBOLS #
;###########################################

SCI_REGS 	EQU 	$00C8 		;SCI register space
SCIBDH		EQU	SCI_REGS+$00	;SCI Baud Rate Register
SCICR2		EQU	SCI_REGS+$03	;SCI Control Register 2
SCISR1		EQU	SCI_REGS+$04	;SCI Status Register 1
SCIDRL		EQU	SCI_REGS+$07	;SCI Control Register 2
TIE		EQU	$80		;TIE bit mask
TE		EQU	$08		;TE bit mask
RE		EQU	$04		;RE bit mask
SCI_VEC		EQU	$D6		;SCI vector number
INT_REGS	EQU	$0120		;S12X_INT register space
INT_CFADDR	EQU	INT_REGS+$07	;Interrupt Configuration Address Register
INT_CFDATA	EQU	INT_REGS+$08	;Interrupt Configuration Data Registers
RQST		EQU	$80		;RQST bit mask
XGATE_REGS	EQU	$0380		;XGATE register space
XGMCTL		EQU	XGATE_REGS+$00	;XGATE Module Control Register
XGMCTL_CLEAR	EQU	$F902
XGEM		EQU	$8000
XGDBGM		EQU	$2000
XGE		EQU	$0080
XGDBG		EQU	$0020
XGSWEIF		EQU	$0002
XGCHID		EQU	XGATE_REGS+$02	;XGATE Channel ID Register
XGVBR		EQU	XGATE_REGS+$06	;XGATE Vector Base Register
XGIF		EQU	XGATE_REGS+$08	;XGATE Interrupt Flag Vector
XGSWT		EQU	XGATE_REGS+$18	;XGATE Software Trigger Register
XGSEM		EQU	XGATE_REGS+$1A	;XGATE Semaphore Register
RPAGE		EQU	$0016
RAM_SIZE	EQU	20*$400		;20k RAM
RAM_START_GLOBAL EQU	$100000-RAM_SIZE
RAM_START_XGATE	EQU	$10000-RAM_SIZE
RAM_START_S12	EQU	$1000
RPAGE_VALUE	EQU	RAM_START_GLOBAL>>12
XGATE_VECTORS	EQU	RAM_START_S12
XGATE_DATA	EQU	RAM_START_S12+(4*128)
XGATE_OFFSET	EQU	(RAM_START_XGATE+(4*128))-XGATE_DATA_BEGIN
BUS_FREQ_HZ	EQU	40000000

;###########################################
;# RESET VECTOR #
;###########################################

		ORG	$FFFE
		ADR	START_OF_CODE
		ORG	$FF00
START_OF_CODE

;###########################################
;# INITIALIZE SCI #
;###########################################

INIT_SCI	MOVW	#(BUS_FREQ_HZ/(16*9600)), SCIBDH ;set baud rate
		MOVB	#(TIE|TE|RE), SCICR2 ;enable tx buffer empty interrupt

;###########################################
;# INITIALIZE S12X_INT #
;###########################################

INIT_INT	SEI			;disable interrupts
		MOVB	#(SCI_VEC&$F0), INT_CFADDR ;switch SCI interrupts to XGATE
		MOVB	#RQST|$01, INT_CFDATA+((SCI_VEC&$0F)>>1)

;###########################################
;# INITIALIZE XGATE #
;###########################################

INIT_XGATE	MOVW	#XGMCTL_CLEAR, XGMCTL ;clear all XGMCTL bits
		BRSET	XGCHID, $FF, INIT_XGATE ;wait until current thread is done
		MOVW	#$10000-RAM_SIZE, XGVBR ;set vector base register
		LDX	#XGIF 		;clear all channel interrupt flags
		LDD	#$FFFF
		STD	2,X+
		STD	2,X+
		STD	2,X+
		STD	2,X+
		STD	2,X+
		STD	2,X+
		STD	2,X+
		STD	2,X+
		MOVW	#$FF00, XGSWT	;clear all software triggers

;###########################################
;# INITIALIZE XGATE VECTOR SPACE #
;###########################################

INIT_XGATE_VECTOR_SPACE
		MOVB 	#(RAM_START_GLOBAL>>12), RPAGE ;set all vectors to dummy service routine
		LDX	#128
		LDY	#RAM_START_S12
		LDD	#XGATE_DUMMY+XGATE_OFFSET
INIT_XGATE_VECTOR_SPACE_LOOP
		STD	4,Y+
		DBNE	X,INIT_XGATE_VECTOR_SPACE_LOOP
;set SCI INTERRUPT VECTOR
		MOVW	#XGATE_CODE_BEGIN+XGATE_OFFSET, RAM_START_S12+(2*SCI_VEC)
		MOVW	#XGATE_DATA_BEGIN+XGATE_OFFSET, RAM_START_S12+(2*SCI_VEC)+2

;###########################################
;# COPY XGATE CODE #
;###########################################

COPY_XGATE_CODE
		LDX	#XGATE_DATA_BEGIN
COPY_XGATE_CODE_LOOP
		MOVW	2,X+, 2,Y+
		MOVW	2,X+, 2,Y+
		MOVW	2,X+, 2,Y+
		MOVW	2,X+, 2,Y+
		CPX	#XGATE_CODE_END
		BLS	COPY_XGATE_CODE_LOOP

;###########################################
;# START XGATE #
;###########################################

START_XGATE	MOVW	#(XGE|XGDBGM|XGSWEIF), XGMCTL ;enable XGATE
		BRA	*

		CPU XGATE

;###########################################
;# XGATE DATA #
;###########################################

		ALIGN 2
XGATE_DATA_BEGIN
XGATE_DATA_SCI_PTR ADR	SCI_REGS	;pointer to SCI register space
XGATE_DATA_MSG_IDX BYT	XGATE_DATA_MSG-XGATE_DATA_BEGIN ;string pointer
XGATE_DATA_MSG 	FCC "Hello World!"	;ASCII string
XGATE_DATA_END	BYT $0D			;CR

;###########################################
;# XGATE CODE #
;###########################################

		ALIGN 2
XGATE_CODE_BEGIN
		LDW 	R2,(R1,#(XGATE_DATA_SCI_PTR-XGATE_DATA_BEGIN)) ;SCI -> R2
		LDB 	R3,(R1,#(XGATE_DATA_MSG_IDX-XGATE_DATA_BEGIN));msg -> R3
		LDB	R4,(R1,R3+)	;curr. char -> R4
		STB	R3,(R1,#(XGATE_DATA_MSG_IDX-XGATE_DATA_BEGIN));R3 -> idx
		LDB	R0,(R2,#(SCISR1-SCI_REGS)) ;initiate SCI transmit
		STB	R4,(R2,#(SCIDRL-SCI_REGS)) ;initiate SCI transmit
		CMPL	R4,#$0D
		BEQ	XGATE_CODE_DONE
		RTS
XGATE_CODE_DONE LDL	R4,#$00		;disable SCI interrupts
		STB	R4,(R2,#(SCICR2-SCI_REGS))
		LDL	R3,#(XGATE_DATA_MSG-XGATE_DATA_BEGIN);reset R3
		STB	R3,(R1,#(XGATE_DATA_MSG_IDX-XGATE_DATA_BEGIN))
XGATE_CODE_END	RTS
XGATE_DUMMY	EQU XGATE_CODE_END
