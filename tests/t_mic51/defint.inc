;****************************************************************************
; Definition der Interrupts und des Timers 2
;

Falling_Edge    EQU     True
Low_Level       EQU     False
Negative_Trans  EQU     False
Positive_Trans  EQU     True

Param_On        EQU     True
Param_Off       EQU     False

Nope            EQU     False

Tm2_Off         EQU     0               ;
Tm2_Timer       EQU     1               ; Input selection
Tm2_Counter     EQU     2               ;
Tm2_Gated       EQU     3               ;

Tm2_Reload_Off  EQU     0               ;
Tm2_Reload_Ov   EQU     2               ; Reload mode
Tm2_Reload_Ext  EQU     3               ;

Tm2_Comp_Mode0  EQU     False           ; Compare mode
Tm2_Comp_Mode1  EQU     True            ;

Tm2_Slow        EQU     True            ; Prescaler selection
Tm2_Normal      EQU     False           ;

Tm2_CC_Off      EQU     0
Tm2_CC_Capt_Ext EQU     1
Tm2_CC_Comp_On  EQU     2
Tm2_CC_Capt_CRCL EQU    3

;-----------------------------------------------------------------------------

Init_Interrupt  MACRO   PInterrupt, PTrig, PState
                IFNDEF PInterrupt
                  FATAL "unbekannter Interrupt"
                ELSEIF

                  SWITCH PInterrupt
                  CASE INT0_VEKTOR              ; INT0
                  IF PTrig
                    SETB        IT0
                  ELSEIF
                    CLR         IT0
                  ENDIF

                  CASE INT1_VEKTOR              ; INT1
                  IF PTrig
                    SETB        IT1
                  ELSEIF
                    CLR         IT1
                  ENDIF

                  CASE INT2_VEKTOR              ; INT2
                  IF PTrig
                    SETB        I2FR
                  ELSEIF
                    CLR         I2FR
                  ENDIF

                  CASE INT3_VEKTOR              ; INT3
                  IF PTrig
                    SETB        I3FR
                  ELSEIF
                    CLR         I3FR
                  ENDIF

                  ELSECASE
                  ENDCASE

                  IF PState
                    Int_On      PInterrupt
                  ELSEIF
                    Int_Off     PInterrupt
                  ENDIF

                ENDIF
                ENDM


;-----------------------------------------------------------------------------

Int_On          MACRO   PInterrupt
                IFNDEF PInterrupt
                  FATAL "unbekannter Interrupt"
                ELSEIF
                  SWITCH PInterrupt
                  CASE INT0_VEKTOR
                    CLR         IE0
                    SETB        EX0
                  CASE ICT0_VEKTOR
                    CLR         TF0
                    SETB        ET0
                  CASE INT1_VEKTOR
                    CLR         IE1
                    SETB        EX1
                  CASE ICT1_VEKTOR
                    CLR         TF1
                    SETB        ET1
                  CASE ICT2_VEKTOR
                    CLR         TF2
                    CLR         EXF2
                    SETB        ET2
                  CASE AD_I_VEKTOR
                    CLR         IADC
                    SETB        EADC
                  CASE INT2_VEKTOR
                    CLR         IEX2
                    SETB        EX2
                  CASE INT3_VEKTOR
                    CLR         IEX3
                    SETB        EX3
                  CASE INT4_VEKTOR
                    CLR         IEX4
                    SETB        EX4
                  CASE INT5_VEKTOR
                    CLR         IEX5
                    SETB        EX5
                  CASE INT6_VEKTOR
                    CLR         IEX6
                    SETB        EX6
                  ENDCASE
                ENDIF
                ENDM

;-----------------------------------------------------------------------------

Int_Off         MACRO   PInterrupt
                IFNDEF PInterrupt
                  FATAL "unbekannter Interrupt"
                ELSEIF
                  SWITCH PInterrupt
                  CASE INT0_VEKTOR
                    CLR         EX0
                    CLR         IE0
                  CASE ICT0_VEKTOR
                    CLR         ET0
                    CLR         TF0
                  CASE INT1_VEKTOR
                    CLR         EX1
                    CLR         IE1
                  CASE ICT1_VEKTOR
                    CLR         ET1
                    CLR         TF1
                  CASE ICT2_VEKTOR
                    CLR         ET2
                    CLR         TF2
                    CLR         EXF2
                  CASE AD_I_VEKTOR
                    CLR         EADC
                    CLR         IADC
                  CASE INT2_VEKTOR
                    CLR         EX2
                    CLR         IEX2
                  CASE INT3_VEKTOR
                    CLR         EX3
                    CLR         IEX3
                  CASE INT4_VEKTOR
                    CLR         EX4
                    CLR         IEX4
                  CASE INT5_VEKTOR
                    CLR         EX5
                    CLR         IEX5
                  CASE INT6_VEKTOR
                    CLR         EX6
                    CLR         IEX6
                  ENDCASE
                ENDIF
                ENDM

;-----------------------------------------------------------------------------

Init_Timer2     MACRO   PInput, PReloadMode, PCompareMode, PPrescaler, PReloadValue

                SWITCH  PInput
                CASE    Tm2_Off
                  CLR           T2I1
                  CLR           T2I0
                CASE    Tm2_Timer
                  CLR           T2I1
                  SETB          T2I0
                CASE    Tm2_Counter
                  SETB          T2I1
                  CLR           T2I0
                CASE    Tm2_Gated
                  SETB          T2I1
                  SETB          T2I0
                ENDCASE

                SWITCH  PReloadMode
                CASE    Tm2_Reload_Off
                  CLR           T2R1
                  CLR           T2R0
                CASE    Tm2_Reload_Ov
                  SETB          T2R1
                  CLR           T2R0
                CASE    Tm2_Reload_Ext
                  SETB          T2R1
                  SETB          T2R0
                ENDCASE

                IF      PCompareMode
                  SETB          T2CM
                ELSEIF
                  CLR           T2CM
                ENDIF

                IF      PPrescaler
                  SETB          T2PS
                ELSEIF
                  CLR           T2PS
                ENDIF

                MOV             CRCL, #(PReloadValue # 256)
                MOV             CRCH, #(PReloadValue / 256)

                ENDM



Init_Comp_Timer2  MACRO   PMode1, PMode2, PMode3, PValue1, PValue2, PValue3

                MOV     CCEN, #(PMode1*4+PMode2*16+PMode3*64)
                MOV     CCL1, #(PValue1 # 256)
                MOV     CCH1, #(PValue1 / 256)
                MOV     CCL2, #(PValue1 # 256)
                MOV     CCH2, #(PValue1 / 256)
                MOV     CCL3, #(PValue1 # 256)
                MOV     CCH3, #(PValue1 / 256)

                ENDM
