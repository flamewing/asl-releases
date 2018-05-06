		ifndef	__s12z_ftmrz_inc
__s12z_ftmrz_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File FTMRZ.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Flash/EEPROM Controller   *
;*                                                                          *
;****************************************************************************

FCLKDIV		equ	$0380		; Flash Clock Divider Register
FDIVLD		s12zbit	FCLKDIV,7	;  Clock Divider Loaded
FDIVLCK		s12zbit	FCLKDIV,6	;  Clock Divider Locked
FDIV		s12zfld	FCLKDIV,6:0	;  Clock Divider Bits
FSEC		equ	$0381		; Flash Security Register
KEYEN		s12zfld	FSEC,2:6	;  Backdoor Key Security Enable Bits
RNV		s12zfld	FSEC,4:2	;  Reserved Nonvolatile Bits
SEC		s12zfld	FSEC,2:0	;  Flash Security Bits
FCCOBIX		equ	$0382		; Flash CCOB Index Register
CCOBIX		s12zfld	FCCOBIX,3:0	;  Common Command Register Index
FPSTAT		equ	$0383		; Flash Protection Status Register
FPOVRD		s12zbit	FPSTAT,7	;  Flash Protection Override Status
WSTATACK	s12zbit	FPSTAT,0	;  Wait-State Switch Acknowledge
FCNFG		equ	$0384		; Flash Configuration Register
CCIE		s12zbit	FCNFG,7		;  Command Complete Interrupt Enable
ERSAREQ		s12zbit	FCNFG,5		;  Erase All Request
IGNSF		s12zbit	FCNFG,4		;  Ignore Single Bit Fault
WSTAT		s12zfld	FCNFG,2:2	;  Wait State control bits
FDFD		s12zbit	FCNFG,1		;  Force Double Bit Fault Detect
FSFD		s12zbit	FCNFG,0		;  Force Single Bit Fault Detect
FERCNFG		equ	$0385		; Flash Error Configuration Register
SFDIE		s12zbit	FERCNFG,0	;  Single Bit Fault Detect Interrupt Enable
FSTAT		equ	$0386		; Flash Status Register
CCIF		s12zbit	FSTAT,7		;  Command Complete Interrupt Flag
ACCERR		s12zbit	FSTAT,5		;  Flash Access Error Flag
FPVIOL		s12zbit	FSTAT,6		;  Flash Protection Violation Flag
MGBUSY		s12zbit	FSTAT,3		;  Memory Controller Busy Flag
MGSTAT		s12zfld	FSTAT,2:0	;  Memory Controller Command Completion Status Flag
FERSTAT		equ	$0387		; Flash Error Status Register
DFDF		s12zbit	FERSTAT,1	;  Double Bit Fault Detect Flag
SFDIF		s12zbit	FERSTAT,0	;  Single Bit Fault Detect Interrupt Flag
FPROT		equ	$0388		; P-Flash Protection Register
FPOPEN		s12zbit	FPROT,7		;  Flash Protection Operation Enable
RNV6		s12zbit	FPROT,6		;  Reserved Nonvolatile Bit
FPHDIS		s12zbit	FPROT,5		;  Flash Protection Higher Address Range Disable
FPHS		s12zfld	FPROT,2:3	;  Flash Protection Higher Address Size
FPLDIS		s12zbit	FPROT,2		;  Flash Protection Lower Address Range Disable
FPLS		s12zfld	FPROT,2:0	;  Flash Protection Lower Address Size
DFPROT		equ	$0389		; EEPROM Protection Register
DPOPEN		s12zbit	DFPROT,7	;  EEPROM Protection Control
DPS		s12zfld	DFPROT,6:0	;  EEPROM Protection Size
FOPT		equ	$038A		; Flash Option Register
NV		s12zfld	FOPT,8:0	;  Nonvolatile Bits
FRSV1		equ	$038B		; Flash Reserved1 Register
FCCOB0		equ	$038C		; Flash Common Command Object Register 0 (16 bit)
FCCOB0HI	equ	$038C		; Flash Common Command Object Register 0 MSB
FCCOB0LO	equ	$038D           ; Flash Common Command Object Register 0 LSB
FCCOB1		equ	$038E		; Flash Common Command Object Register 1 (16 bit)
FCCOB1HI	equ	$038E           ; Flash Common Command Object Register 1 MSB
FCCOB1LO	equ	$038F           ; Flash Common Command Object Register 1 LSB
FCCOB2		equ	$0390		; Flash Common Command Object Register 2 (16 bit)
FCCOB2HI	equ	$0390           ; Flash Common Command Object Register 2 MSB
FCCOB2LO	equ	$0391           ; Flash Common Command Object Register 2 LSB
FCCOB3		equ	$0392		; Flash Common Command Object Register 3 (16 bit)
FCCOB3HI	equ	$0392           ; Flash Common Command Object Register 3 MSB
FCCOB3LO	equ	$0393           ; Flash Common Command Object Register 3 LSB
FCCOB4		equ	$0394		; Flash Common Command Object Register 4 (16 bit)
FCCOB4HI	equ	$0394           ; Flash Common Command Object Register 4 MSB
FCCOB4LO	equ	$0395           ; Flash Common Command Object Register 4 LSB
FCCOB5		equ	$0396		; Flash Common Command Object Register 5 (16 bit)
FCCOB5HI	equ	$0396           ; Flash Common Command Object Register 5 MSB
FCCOB5LO	equ	$0397           ; Flash Common Command Object Register 5 LSB

		restore			; re-enable listing

		endif			; __s12z_ftmrz_inc
