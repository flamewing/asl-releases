; *****************************************************************************
; Definition der Parameter
; Format:
;      DefParam <Parametername>, <Wert>, <String>
; soll der String gleichlautend wie der Parametername sein dann reicht ""
;

       DefParam On,      001H, ""
       DefParam Off,     000H, ""
       DefParam Plus,    001H, "+"
       DefParam Minus,   000H, "-"
       DefParam Fast,    001H, ""
       DefParam Slow,    000H, ""
       DefParam Superslow, 002H, ""

;      Hier kommen die Warning-Codes fÅr die FrÑse

       DefParam Queue_Warning,             081H, "Queue Warning"

;      Hier kommen die Error-Codes fÅr die FrÑse

       DefParam Queue_Full,                061H, "Queue Full"
       DefParam Undef_Par,                 062H, "Undefined Parameter"
       DefParam Undef_Piece_Ref,           063H, "Undefined Piece Reference"
       DefParam Undef_Drill_Ref,           064H, "Undefined Drill Reference"
       DefParam Undef_Koord_Ref,           065H, "Undefined Koord Reference"
       DefParam Internal_Error,            066H, "Internal Error"

       DefParam End_X_Plus,                067H, "Software End X+"
       DefParam End_Y_Plus,                068H, "Software End Y+"
       DefParam End_Z_Plus,                069H, "Software End Z+"
       DefParam End_V_Plus,                06AH, "Software End V+"
       DefParam End_X_Minus,               070H, "Software End X-"
       DefParam End_Y_Minus,               071H, "Software End Y-"
       DefParam End_Z_Minus,               072H, "Software End Z-"
       DefParam End_V_Minus,               073H, "Software End V-"

       DefParam Hard_End_X_Plus,           074H, "Hardware End X+"
       DefParam Hard_End_Y_Plus,           075H, "Hardware End Y+"
       DefParam Hard_End_Z_Plus,           076H, "Hardware End Z+"
       DefParam Hard_End_V_Plus,           077H, "Hardware End V+"
       DefParam Hard_End_X_Minus,          078H, "Hardware End X-"
       DefParam Hard_End_Y_Minus,          079H, "Hardware End Y-"
       DefParam Hard_End_Z_Minus,          07AH, "Hardware End Z-"
       DefParam Hard_End_V_Minus,          07BH, "Hardware End V-"
; *****************************************************************************
