; Includedatei PcPar-System
; vor dem Programm mit include einbinden

;----------------------------------------------------------------------------
; offizieller Datenbereich:

                shared          S_RegSave,S_MemEnd,S_ParNo,S_CPUNo
                shared          _fadd_cnt,_fmul_cnt,_fdiv_cnt,_fsqrt_cnt

		org		$400
S_MemEnd:
                ds.l            1               ; Speicherende ( Default 64K )
S_SysStart:
		ds.l		1		; Anfang des Systemcodes
S_ParNo:        				; vom PC geschiebene Werte
                ds.w            1               ; Parallelrechneradresse
S_LibAdr:                                       ; Adresse der Library-
		ds.l            1               ; sprungtabelle
_fadd_cnt:                                      ; Anzahl ausgefuehrter Gleit-
                ds.l            1               ; kommaadditionen/subtraktion
_fmul_cnt:                                      ; dito Multiplikation
                ds.l            1
_fdiv_cnt:                                      ; dito Division
                ds.l            1
_fsqrt_cnt:                                     ; dito Quadratwurzel
                ds.l            1
S_FreeMemEnd:
                ds.l            1               ; Ende freien Speichers
S_CPUNo:
                ds.w            1               ; CPU-Typ: 0 = 68008
                                                ;          1 = 68000
                                                ;          2 = 68010
                                                ;          3 = 68020
                                                ;          4 = 68030
                                                ; Byte 1 : $01 = 68881
                                                ;          $02 = 68882
                                                ;          $10 = 68851
                org             $600

S_SSPEnd:
		ds.l		1		; Anfang des Systemstacks
S_ResVecSave:					; Sicherung Resetvektor
		ds.l		1
S_RegSave:                        		; Registersicherung
		ds.l		17		; wird vom PC veraendert
S_ExVec:                                        ; Exceptionvektor
                ds.w            1
S_LibStart:
                ds.l            1               ; Anfang Librarykette
                org             $800

;-----------------------------------------------------------------------------
; Libraryoffsets :

fadd            equ             $0000
fsub            equ             $0004
fmul            equ             $0008
fdiv            equ             $000c
fmul2           equ             $0010
fsqrt           equ             $0014
fabs		equ		$0018

fcmp            equ             $0020
fitof           equ             $0024
fftoi           equ             $0028

fexp            equ             $0040
fsinh           equ             $0044
fcosh           equ             $0048
ftanh           equ             $004c
fcoth           equ             $0050

fln             equ             $0060
flog            equ             $0064
fasinh          equ             $0068
facosh          equ             $006c
fatanh          equ             $0070
facoth          equ             $0074

fsin            equ             $0080
fcos            equ             $0084
ftan            equ             $0088
fcot            equ             $008c

fasin           equ             $00a0
facos           equ             $00a4
fatan           equ             $00a8
facot           equ             $00ac

;----------------------------------------------------------------------------
; Konstanten fuer Betriebssystemaufrufe:

TrapProgEnd     equ     15              ; Trap fuer Programmende

TrapTglSStep    equ     14              ; Trap Einzelschritt an/aus

TrapLibCtrl     equ     13              ; Trap Libraryverwaltung
LibCtrlInstall  equ     0
LibCtrlGetAdr   equ     1

;----------------------------------------------------------------------------
; andere Konstanten:

S_Latch         equ     $fffffffe       ; Adresse der I/O-Latches

is68008         equ     $00             ; Prozessorcode 68008
is68000         equ     $01             ;      "        68000
is68010         equ     $02             ;      "        68010
is68020         equ     $03             ;      "        68020
is68030         equ     $04             ;      "        68030
has68881        equ     $01             ;      "        68881
has68882        equ     $02             ;      "        68882
hasMMU          equ     $10             ;      "        68851
intMMU          equ     $20             ;      "        interne MMU

