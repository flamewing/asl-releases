
; ***************************************************************************
; Definition einer Tabelle aus den MSG - Includedateien

Create_IncOffsTab macro TabName, IncDatei
TabName_Tab:  label   $
FStr:         db      TableAnf
              db      FStrEnd - FStr, StringEnde, FStrEnd - FStr
FStrEnd:
              include IncDatei
TabName_Tend:  label   $
              db      TableEnd
              endm


Create_IncTab macro TabName, IncDatei
TabName_Tab:  label   $
              include IncDatei
TabName_Tend:  label   $
              endm


Create_IncKeyTab macro State, Port, Modul, IncDatei
Key_Counter   set 0
State_Port_Modul_Tab:  label   $
             include IncDatei
State_Port_Modul_Tend:  label   $
             if Key_Counter <> 8
                fatal "Inkorrekte Anzahl von Elementen in Key-Tabelle"
             endif
           endm


Create_MsgJmpTab macro Modul, IncDatei
Modul_MsgCall_Tab:      label   $
                 include IncDatei
Modul_MsgCall_Tend:     label   $
                 db      TableEnd
                 endm



; ***************************************************************************
;

DefModul macro Modul, NetAdr

   ifndef Modul
Modul     equ  NextModul
Net_Modul equ  NetAdr
NextModul set  NextModul + 1

     SHARED Modul,NetAdr

   endif

   ifdef ModulNetAdr_Tab
     ifndef ModulNetAdr_TEND
       if (ModulNetAdr_Tab + Modul - $) < 0
           Fatal "NetAdr Tabelle inkonsistent"
         elseif
           org ModulNetAdr_Tab + Modul
         endif

        db NetAdr
      endif
    endif

; -------------------------

   ifdef ModulStr_Tab
     ifndef ModulStr_TEnd

strdef:   db Modul, strdefnext - strdef, "MODUL", StringEnde, strdefnext - strdef
strdefnext:

       endif
     endif

   endm

;
; ***************************************************************************
;

DefParam macro Param, Wert, String

   ifndef Param
Param     equ Wert

     SHARED Param

   endif

   ifdef ParamStr_Tab
     ifndef ParamStr_TEnd
strdef:   db Wert
          db strdefnext - strdef
          if String = ""
              db "PARAM", StringEnde
            elseif
              db String,  StringEnde
            endif
strdefnext:
       endif
     endif

   endm

;
; ***************************************************************************
;

DefKey macro Status, Port, Modul, String

   ifdef Status_Port_Modul_Tab
     ifndef Status_Port_Modul_TEnd
strdef:   db strdefnext - strdef
          db String,StringEnde
strdefnext:
Key_Counter  set  Key_Counter + 1
     endif
   endif

   endm

;
; ***************************************************************************
;

DefMsg  macro Modul, MsgName, ParamCount, ParamPattern

; ---------------------------------------------------------------------------
; Definition der Message-Nummern

      ifndef MsgName
MsgName        equ  Modul_NextMsg
Modul_NextMsg  set  Modul_NextMsg + 1

        SHARED  MsgName

      endif
; ---------------------------------------------------------------------------
; Aufbau der Tabelle fuer den Message-Handler

  ifdef Modul_MsgCall_Tab
    ifndef Modul_MsgCall_Tend
      if (Modul_MsgCall_Tab + (2 * MsgName) - $) < 0
          Fatal "Msg-Call Tabelle inkonsistent"
        elseif
          org Modul_MsgCall_Tab + (2 * MsgName)
        endif

        db Adr_MsgName # 256
        db Adr_MsgName / 256
      endif
    endif

; ---------------------------------------------------------------------------
; Aufbau der Stringtabelle

  ifdef MsgStr_Tab
    ifndef MsgStr_TEnd

strdef: db Modul, strdefnext - strdef, MsgName, "MSGNAME", StringEnde, ParamCount, ParamPattern
        db strdefnext - strdef
strdefnext:
      endif
    endif

        endm
;
; ***************************************************************************


