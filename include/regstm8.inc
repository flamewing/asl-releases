                ifndef  __regstm8inc     ; avoid multiple inclusion
__regstm8inc	equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGSTM8.INC                                             *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8 Processors                   *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; General Macros

bfield		macro	{INTLABEL},reg,start,count
__LABEL__	equ	((1<<(count+start))-1)-((1<<start)-1)
		endm

__decstr	macro		DEST,SRC
		if		(SRC)<10
DEST		set		"\{SRC}"
		elseif
DEST		set		"\{(SRC)/10}\{(SRC)#10}"
		endif
		endm

__dec02str	macro		DEST,SRC
DEST		set		"\{(SRC)/10}\{(SRC)#10}"
		endm

;----------------------------------------------------------------------------
; Include proper CPU-specific register definitions

		switch	MOMCPUNAME
		case	"STM8S001J3"
		  include "stm8/stm8s/reg001j3.inc"
		case	"STM8S003F3","STM8S003K3"
		  include "stm8/stm8s/reg003f3.inc"
		case	"STM8S005C6","STM8S005K6"
                  include "stm8/stm8s/reg005c6.inc"
                case	"STM8S007C8"
                  include "stm8/stm8s/reg007c8.inc"
                case	"STM8S103F2","STM8S103F3","STM8S103K3"
		  include "stm8/stm8s/reg103f3.inc"
		case	"STM8S105C4","STM8S105C6","STM8S105K4","STM8S105K6","STM8S105S4","STM8S105S6"
		  include "stm8/stm8s/reg105c6.inc"
		case	"STM8S207MB","STM8S207M8","STM8S207RB","STM8S207R8","STM8S207R6","STM8S207CB","STM8S207C8","STM8S207C6","STM8S207SB","STM8S207S8","STM8S207S6","STM8S207K8","STM8S207K6"
		  include "stm8/stm8s/reg207mb.inc"
		case    "STM8S208MB","STM8S208RB","STM8S208R8","STM8S208R6","STM8S208CB","STM8S208C8","STM8S208C6","STM8S208SB","STM8S208S8","STM8S208S6"
		  include "stm8/stm8s/reg208mb.inc"
		case	"STM8S903K3","STM8S903F3"
		  include "stm8/stm8s/reg903f3.inc"
		case	"STM8L050J3"
		  include "stm8/stm8l/reg050j3.inc"
		case	"STM8L051F3"
		  include "stm8/stm8l/reg051f3.inc"
		case	"STM8L052C6"
		  include "stm8/stm8l/reg052c6.inc"
		case	"STM8L052R8"
		  include "stm8/stm8l/reg052r8.inc"
		case	"STM8L001J3"
		  include "stm8/stm8l/reg001j3.inc"
		case	"STM8L101F1","STM8L101F2","STM8L101G2","STM8L101F3","STM8L101G3","STM8L101K3"
		  include "stm8/stm8l/reg101xx.inc"
		case	"STM8L151C2","STM8L151K2","STM8L151G2","STM8L151F2","STM8L151C3","STM8L151K3","STM8L151G3","STM8L151F3"
		  include "stm8/stm8l/reg151xx.inc"
		case	"STM8L151C4","STM8L151C6","STM8L151K4","STM8L151K6","STM8L151G4","STM8L151G6"
		  include "stm8/stm8l/reg151x4.inc"
		case	"STM8L152C4","STM8L152C6","STM8L152K4","STM8L152K6"
		  include "stm8/stm8l/reg152x4.inc"
		case    "STM8L151R6","STM8L151C8","STM8L151M8","STM8L151R8"
		  include "stm8/stm8l/reg151x8.inc"
		case	"STM8L152R6","STM8L152C8","STM8L152K8","STM8L152M8","STM8L152R8"
		  include "stm8/stm8l/reg152x8.inc"
		case	"STM8L162M8","STM8L162R8"
		  include "stm8/stm8l/reg162x8.inc"
		case	"STM8AF6366"
		  include "stm8/stm8af/reg6366.inc"
		case	"STM8AF6388"
		  include "stm8/stm8af/reg6388.inc"
		case	"STM8AF6226","STM8AF6223","STM8AF6213"
		  include "stm8/stm8af/reg62x3.inc"
		case	"STM8AF6246","STM8AF6248","STM8AF6266","STM8AF6268"
		  include "stm8/stm8af/reg62x6.inc"
		case	"STM8AF6269","STM8AF6286","STM8AF6288","STM8AF6289","STM8AF628A","STM8AF62A6","STM8AF62A8","STM8AF62A9","STM8AF62AA"
		  include "stm8/stm8af/reg62x9.inc"
		case	"STM8AF5268","STM8AF5269","STM8AF5286","STM8AF5288","STM8AF5289","STM8AF528A","STM8AF52A6","STM8AF52A8","STM8AF52A9","STM8AF52AA"
		  include "stm8/stm8af/reg52x9.inc"
		case	"STM8AL3136","STM8AL3138","STM8AL3146","STM8AL3148","STM8AL3166","STM8AL3168"
		  include "stm8/stm8al/reg31x6.inc"
		case	"STM8AL3L46","STM8AL3L48","STM8AL3L66","STM8AL3L68"
		  include "stm8/stm8al/reg3lx6.inc"
		case	"STM8AL3188","STM8AL3189","STM8AL318A"
		  include "stm8/stm8al/reg31x8.inc"
		case	"STM8AL3L88","STM8AL3L89","STM8AL3L8A"
		  include "stm8/stm8al/reg3lx8.inc"
		case	"STM8TL52F4", "STM8TL52G4", "STM8TL53C4", "STM8TL53F4", "STM8TL53G4"
		  include "stm8/stm8t/reg05xx4.inc"
		elsecase
		  error	"wrong processor type set: only STM8S001J3, STM8S003F3, STM8S003K3, STM8S005C6, STM8S005K6, STM8S007C8, STM8S103F2, STM8S103F3,STM8S103K3,"
		  error	"STM8S105C4, STM8S105C6, STM8S105K4, STM8S105K6, STM8S105S4, STM8S105S6, STM8L050J3, STM8L051F3, STM8L052C6, STM8L052R8, STM8L001J3,"
		  error	"STM8L101F1, STM8L101F2, STM8L101G2, STM8L101F3, STM8L101G3, STM8L101K3, STM8L151C2, STM8L151K2, STM8L151G2, STM8L151F2, STM8L151C3,"
		  error	"STM8L151K3, STM8L151G3, STM8L151F3, STM8L162M8, STM8L162R8, STM8AF6366, STM8AF6388, STM8AF6213, STM8AF6223, STM8AF6226, STM8AF6246,"
		  error	"STM8AF6248, STM8AF6266, STM8AF6268, STM8AF6269, STM8AF6286, STM8AF6288, STM8AF6289, STM8AF628A, STM8AF62A6, STM8AF62A8, STM8AF62A9,"
		  error	"STM8AF62AA, STM8AF5268, STM8AF5269, STM8AF5286, STM8AF5288, STM8AF5289, STM8AF528A, STM8AF52A6, STM8AF52A8, STM8AF52A9, STM8AF52AA,"
		  fatal "STM8TL52F4, STM8TL52G4, STM8TL53C4, STM8TL53F4, STM8TL53G4 allowed!"
                endcase

                if      MOMPASS=1
                 message "STM8 Register Definitions (C) 2019 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; Post Processing

		; Flash Size can usually be deduced from the part number:
		
		ifndef	FLASHEND
                if      substr(MOMCPUNAME,4,1)=="A"     ; automotive
                 switch	substr(MOMCPUNAME,8,1)
		 case	"1"			; '1' -> 4K
FLASHEND	 label	FLASHSTART+$0fff
		 case	"2","3"			; '2/3' -> 8K
FLASHEND	 label	FLASHSTART+$1fff
		 case	"4"			; '4' -> 16K
FLASHEND	 label	FLASHSTART+$3fff
		 case	"6"			; '6' -> 32K
FLASHEND	 label	FLASHSTART+$7fff
		 case	"8"			; '8' -> 64K
FLASHEND	 label	FLASHSTART+$ffff
		 case	"A"			; 'A' -> 128K
FLASHEND	 label	FLASHSTART+$1ffff
		 elsecase
		 fatal	"Cannot deduce flash size from device name"
                 endcase
                elseif					; non-automotive
                 switch	substr(MOMCPUNAME,9,1)
		 case	"1"			; '1' -> 2K
FLASHEND	 label	FLASHSTART+$07ff
		 case	"2"			; '2' -> 4K
FLASHEND	 label	FLASHSTART+$0fff
		 case	"3"			; '3' -> 8K
FLASHEND	 label	FLASHSTART+$1fff
		 case	"4"			; '4' -> 16K
FLASHEND	 label	FLASHSTART+$3fff
		 case	"6"			; '6' -> 32K
FLASHEND	 label	FLASHSTART+$7fff
		 case	"8"			; '8' -> 64K
FLASHEND	 label	FLASHSTART+$ffff
		 case	"B"			; 'B' -> 128K
FLASHEND	 label	FLASHSTART+$1ffff
		 elsecase
		 fatal	"Cannot deduce flash size from device name"
		 endcase
		endif					; automotive
		endif

;----------------------------------------------------------------------------

                restore                 ; allow again

                endif			; __regstm8inc
