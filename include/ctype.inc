		ifndef   ctypeinc       ; avoid multiple inclusion
ctypeinc        equ      1
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File CTYPE.INC                                               *
;*                                                                          *
;*   Contains Functions to Classify Characters                              *
;*                                                                          *
;* CAUTION!  isalpha() does not regard country-specific special characters! *
;*                                                                          *
;****************************************************************************

		save
		listing off   ; no listing over this file

		if	 mompass=1
		 message "Standard Character Funkcions (C) 1993 Alfred Arnold"
		endif

;----------------------------------------------------------------------------
; returns TRUE if the argument is a number:

isdigit         function ch,(ch>='0')&&(ch<='9')

;----------------------------------------------------------------------------
; returns TRUE if the argument is hexadecimal number:

isxdigit	function ch,(isdigit(ch))||((toupper(ch)>='A')&&(toupper(ch)<='F'))

;----------------------------------------------------------------------------
; returns TRUE if the argument is in the range of ASCII characters:

isascii         function ch,(ch>=0)&&(ch<128)

;----------------------------------------------------------------------------
; returns TRUE if the argument is an uppercase letter:

isupper		function ch,(ch>='A')&&(ch<='Z')

;----------------------------------------------------------------------------
; returns TRUE if the argument is a lowercase letter:

islower		function ch,(ch>='a')&&(ch<='z')

;----------------------------------------------------------------------------
; returns TRUE if the argument is a letter:

isalpha         function ch,(toupper(ch)>='A')&&(toupper(ch)<='Z')

;----------------------------------------------------------------------------
; returns TRUE if the argument is a number or letter:

isalnum         function ch,isdigit(ch)||isalpha(ch)

;----------------------------------------------------------------------------
; returns TRUE if the argument is some sor tof space:
; Hint: 11 = vertical tabulator

isspace		function ch,(ch=' ')||(ch=12)||(ch='\n')||(ch='\r')||(ch='\t')||(ch=11)

;----------------------------------------------------------------------------
; returns TRUE if the argument is a printable character:
; strictly spoken, DEL(127) should be excluded, but it is printable on a PC

isprint		function ch,(ch>31)&&(ch<255)

;----------------------------------------------------------------------------
; returns TRUE if the argument is a control character:

iscntrl		function ch,~~isprint(ch)

;----------------------------------------------------------------------------
; returns TRUE if the argument is ein printable and visible character:

isgraph		function ch,isprint(ch)&&(~~isspace(ch))

;----------------------------------------------------------------------------
; returns TRUE if the argument is a special character:

ispunct		function ch,isprint(ch)&&(~~isspace(ch))&&(~~isalnum(ch))

		restore                 ; re-enable listing

		endif			; ctypeinc
