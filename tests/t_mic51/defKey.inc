; *****************************************************************************
; Definition des Tastenfeldes der Teach-In Einheit
; Format:
;      DefKey {On,Off}, {P4,P5}, <Teach-In Modul>, <String>
;
;      Es muss genau 8 Elemente pro Kategorie eingetragen werden
;      Wenn leerer String, wird keine Message durch das Netz gesendet


        DefKey On, P4, TeachRob, "Rob MoveCHL"     ; Hand links
        DefKey On, P4, TeachRob, "Rob MoveCHR"     ; Hand rechts
        DefKey On, P4, TeachRob, "Rob MoveCHO"     ; Hand oeffnen
        DefKey On, P4, TeachRob, "Rob MoveCHC"     ; Hand schliessen
        DefKey On, P4, TeachRob, "MemRob StoreRobPos" ; STORE
        DefKey On, P4, TeachRob, "MemRob StoreRobReady"     ; AUX1
        DefKey On, P4, TeachRob, "MemRob StoreRobSync"   ; AUX2
        DefKey On, P4, TeachRob, "Rob RobVelocity Fast" ; FAST

        DefKey Off, P4, TeachRob, "Rob StopCHT"
        DefKey Off, P4, TeachRob, "Rob StopCHT"
        DefKey Off, P4, TeachRob, "Rob StopCH"
        DefKey Off, P4, TeachRob, "Rob StopCH"
        DefKey Off, P4, TeachRob, ""
        DefKey Off, P4, TeachRob, ""
        DefKey Off, P4, TeachRob, ""
        DefKey Off, P4, TeachRob, "Rob RobVelocity Slow"

        DefKey On, P5, TeachRob, "Rob MoveCBL"     ; Rumpf links
        DefKey On, P5, TeachRob, "Rob MoveCBR"     ; Rumpf rechts
        DefKey On, P5, TeachRob, "Rob MoveCSU"     ; Oberarm heben
        DefKey On, P5, TeachRob, "Rob MoveCSD"     ; Oberarm senken
        DefKey On, P5, TeachRob, "Rob MoveCEU"     ; Unterarm heben
        DefKey On, P5, TeachRob, "Rob MoveCED"     ; Unterarm senken
        DefKey On, P5, TeachRob, "Rob MoveCHU"     ; Hand heben
        DefKey On, P5, TeachRob, "Rob MoveCHD"     ; Hand senken

        DefKey Off, P5, TeachRob, "Rob StopCB"
        DefKey Off, P5, TeachRob, "Rob StopCB"
        DefKey Off, P5, TeachRob, "Rob StopCS"
        DefKey Off, P5, TeachRob, "Rob StopCS"
        DefKey Off, P5, TeachRob, "Rob StopCE"
        DefKey Off, P5, TeachRob, "Rob StopCE"
        DefKey Off, P5, TeachRob, "Rob StopCHP"
        DefKey Off, P5, TeachRob, "Rob StopCHP"

        DefKey On, P4, TeachFrs, "Frs Drill on"       ; Fraesmotor ein
        DefKey On, P4, TeachFrs, "Frs Drill off"      ; Fraesmotor aus
        DefKey On, P4, TeachFrs, "MemFrs StoreFrsPos" ; STORE
        DefKey On, P4, TeachFrs, "Frs FrsVelocity fast"     ; FAST
        DefKey On, P4, TeachFrs, "MemFrs StoreFrsReady"     ; AUX1
        DefKey On, P4, TeachFrs, "MemFrs StoreFrsSync"      ; AUX2
        DefKey On, P4, TeachFrs, "MemFrs StoreFrsDrill"     ; AUX3
        DefKey On, P4, TeachFrs, ""     ; AUX4

        DefKey Off, P4, TeachFrs, ""
        DefKey Off, P4, TeachFrs, ""
        DefKey Off, P4, TeachFrs, ""
        DefKey Off, P4, TeachFrs, "Frs FrsVelocity slow"
        DefKey Off, P4, TeachFrs, ""
        DefKey Off, P4, TeachFrs, ""
        DefKey Off, P4, TeachFrs, ""
        DefKey Off, P4, TeachFrs, ""

        DefKey On, P5, TeachFrs, "Frs MoveCX +"     ; X+
        DefKey On, P5, TeachFrs, "Frs MoveCX -"     ; X-
        DefKey On, P5, TeachFrs, "Frs MoveCY +"     ; Y+
        DefKey On, P5, TeachFrs, "Frs MoveCY -"     ; Y-
        DefKey On, P5, TeachFrs, "Frs MoveCZ +"     ; Z+
        DefKey On, P5, TeachFrs, "Frs MoveCZ -"     ; Z-
        DefKey On, P5, TeachFrs, "Frs MoveCV +"     ; Schraubstock auf
        DefKey On, P5, TeachFrs, "Frs MoveCV -"     ; Schraubstock zu

        DefKey Off, P5, TeachFrs, "Frs StopCX +"
        DefKey Off, P5, TeachFrs, "Frs StopCX -"
        DefKey Off, P5, TeachFrs, "Frs StopCY +"
        DefKey Off, P5, TeachFrs, "Frs StopCY -"
        DefKey Off, P5, TeachFrs, "Frs StopCZ +"
        DefKey Off, P5, TeachFrs, "Frs StopCZ -"
        DefKey Off, P5, TeachFrs, "Frs StopCV +"
        DefKey Off, P5, TeachFrs, "Frs StopCV -"

;
; *****************************************************************************
