; ****************************************************************************
; Definition der Parameter - Pattern
;
;NoParamSingle
;ParamSingle1
;ParamSingle2
;ParamSingle3
;ParamSingle4
;ParamSingle5
;ParamSingle6

;
; ****************************************************************************
; Definition der Messages
; Format:
;      DefMsg <Modul>, <MsgName>, <Anzahl Parameter>, <Parameter - Pattern>
;
    DefMsg Rob, MoveAB          , 1, NoParamSingle
    DefMsg Rob, MoveAS          , 1, NoParamSingle
    DefMsg Rob, MoveAE          , 1, NoParamSingle
    DefMsg Rob, MoveARW         , 1, NoParamSingle
    DefMsg Rob, MoveALW         , 1, NoParamSingle
    DefMsg Rob, MoveAH          , 1, NoParamSingle
    DefMsg Rob, MoveAPos1       , 3, NoParamSingle
    DefMsg Rob, MoveAPos2       , 3, NoParamSingle
    DefMsg Rob, MoveRB          , 1, NoParamSingle
    DefMsg Rob, MoveRS          , 1, NoParamSingle
    DefMsg Rob, MoveRE          , 1, NoParamSingle
    DefMsg Rob, MoveRRW         , 1, NoParamSingle
    DefMsg Rob, MoveRLW         , 1, NoParamSingle
    DefMsg Rob, MoveRH          , 1, NoParamSingle
    DefMsg Rob, MoveRPos1       , 3, NoParamSingle
    DefMsg Rob, MoveRPos2       , 3, NoParamSingle
    DefMsg Rob, MoveCBL         , 0, NoParamSingle
    DefMsg Rob, MoveCBR         , 0, NoParamSingle
    DefMsg Rob, StopCB          , 0, NoParamSingle
    DefMsg Rob, MoveCSU         , 0, NoParamSingle
    DefMsg Rob, MoveCSD         , 0, NoParamSingle
    DefMsg Rob, StopCS          , 0, NoParamSingle
    DefMsg Rob, MoveCEU         , 0, NoParamSingle
    DefMsg Rob, MoveCED         , 0, NoParamSingle
    DefMsg Rob, StopCE          , 0, NoParamSingle
    DefMsg Rob, MoveCHU         , 0, NoParamSingle
    DefMsg Rob, MoveCHD         , 0, NoParamSingle
    DefMsg Rob, StopCHP         , 0, NoParamSingle
    DefMsg Rob, MoveCHL         , 0, NoParamSingle
    DefMsg Rob, MoveCHR         , 0, NoParamSingle
    DefMsg Rob, StopCHT         , 0, NoParamSingle
    DefMsg Rob, MoveCHO         , 0, NoParamSingle
    DefMsg Rob, MoveCHC         , 0, NoParamSingle
    DefMsg Rob, StopCH          , 0, NoParamSingle
    DefMsg Rob, RobVelocity     , 1, ParamSingle1
    DefMsg Rob, RobGoHome       , 0, NoParamSingle
    DefMsg Rob, SetHome         , 0, NoParamSingle
    DefMsg Rob, GibPos1         , 2, ParamSingle1+ParamSingle2
    DefMsg Rob, GibPos2         , 2, ParamSingle1+ParamSingle2
    DefMsg Rob, RobGibReady     , 2, ParamSingle1+ParamSingle2
    DefMsg Rob, RobInit         , 0, NoParamSingle
    DefMsg Rob, RobDeInit       , 0, NoParamSingle

    DefMsg Frs, Init            , 0, NoParamSingle
    DefMsg Frs, DeInit          , 0, NoParamSingle
    DefMsg Frs, MoveCX          , 1, ParamSingle1
    DefMsg Frs, MoveCY          , 1, ParamSingle1
    DefMsg Frs, MoveCZ          , 1, ParamSingle1
    DefMsg Frs, MoveCV          , 1, ParamSingle1
    DefMsg Frs, StopCX          , 1, ParamSingle1
    DefMsg Frs, StopCY          , 1, ParamSingle1
    DefMsg Frs, StopCZ          , 1, ParamSingle1
    DefMsg Frs, StopCV          , 1, ParamSingle1
    DefMsg Frs, FrsVelocity     , 1, ParamSingle1
    DefMsg Frs, FrsVelocityDraw , 1, ParamSingle1
    DefMsg Frs, FrsGoHome       , 0, NoParamSingle
    DefMsg Frs, RefFahrt        , 0, NoParamSingle
    DefMsg Frs, SetDrillRef     , 0, NoParamSingle
    DefMsg Frs, SetPieceRef     , 3, NoParamSingle
    DefMsg Frs, MoveX           , 1, NoParamSingle
    DefMsg Frs, MoveY           , 1, NoParamSingle
    DefMsg Frs, MoveZ           , 1, NoParamSingle
    DefMsg Frs, MoveAPos        , 3, NoParamSingle
    DefMsg Frs, MoveRX          , 1, NoParamSingle
    DefMsg Frs, MoveRY          , 1, NoParamSingle
    DefMsg Frs, MoveRZ          , 1, NoParamSingle
    DefMsg Frs, MoveRV          , 1, NoParamSingle
    DefMsg Frs, MoveRPos        , 3, NoParamSingle
    DefMsg Frs, MoveVOpen       , 0, NoParamSingle
    DefMsg Frs, MoveVClose      , 0, NoParamSingle
    DefMsg Frs, MoveV           , 1, NoParamSingle
    DefMsg Frs, GoPieceRefPos   , 0, NoParamSingle
    DefMsg Frs, MovePR          , 3, NoParamSingle
    DefMsg Frs, Drill           , 1, ParamSingle1
    DefMsg Frs, SetMasterAdress , 3, ParamSingle1+ParamSingle2+ParamSingle3
    DefMsg Frs, GibFrsPos1      , 2, ParamSingle1+ParamSingle2
    DefMsg Frs, GibFrsPos2      , 2, ParamSingle1+ParamSingle2
    DefMsg Frs, GibPosP         , 2, ParamSingle1+ParamSingle2
    DefMsg Frs, GibStatus       , 2, ParamSingle1+ParamSingle2
    DefMsg Frs, GibReady        , 3, ParamSingle1+ParamSingle2+ParamSingle3

    DefMsg MemRob,LoadRob,0, NoParamSingle
    DefMsg MemRob,SaveRob,0, NoParamSingle
    DefMsg MemRob,StartRobRun,0, NoParamSingle
    DefMsg MemRob,StoreRobPos,0, NoParamSingle
    DefMsg MemRob,StoreRobSync,0, NoParamSingle
    DefMsg MemRob,StoreRobReady,0, NoParamSingle
    DefMsg MemRob,InitRobTeach,0, NoParamSingle
    DefMsg MemRob,DeinitRobTeach,0, NoParamSingle
    DefMsg MemRob,ClearRobTeach,0, NoParamSingle
    DefMsg MemRob,ResetRobTeach,0, NoParamSingle
    DefMsg MemRob,GetRobSync,1,ParamSingle1
    DefMsg MemRob,GetRobReady,0, NoParamSingle
    DefMsg MemRob,RobPos1,3, NoParamSingle
    DefMsg MemRob,RobPos2,3, NoParamSingle
    DefMsg MemRob,DebugRob,0, NoParamSingle

    DefMsg MemFrs,LoadFrs,0, NoParamSingle
    DefMsg MemFrs,SaveFrs,0, NoParamSingle
    DefMsg MemFrs,StartFrsRun,0, NoParamSingle
    DefMsg MemFrs,StoreFrsPos,0, NoParamSingle
    DefMsg MemFrs,StoreFrsSync,0, NoParamSingle
    DefMsg MemFrs,StoreFrsReady,0, NoParamSingle
    DefMsg MemFrs,StoreFrsPieceRef,0, NoParamSingle
    DefMsg MemFrs,StoreFrsTiefe,0, NoParamSingle
    DefMsg MemFrs,StoreFrsDrill,0, NoParamSingle
    DefMsg MemFrs,InitFrsTeach,0, NoParamSingle
    DefMsg MemFrs,DeinitFrsTeach,0, NoParamSingle
    DefMsg MemFrs,ClearFrsTeach,0, NoParamSingle
    DefMsg MemFrs,ResetFrsTeach,0, NoParamSingle
    DefMsg MemFrs,GetFrsSync,1,ParamSingle1
    DefMsg MemFrs,GetFrsReady,0, NoParamSingle
    DefMsg MemFrs,FrsPos1,3, NoParamSingle
    DefMsg MemFrs,FrsPos2,3, NoParamSingle
    DefMsg MemFrs,FrsPieceRef,3, NoParamSingle
    DefMsg MemFrs,FrsTiefe,3, NoParamSingle
    DefMsg MemFrs,GetFrsError,1, ParamSingle1
    DefMsg MemFrs,GetFrsWarning,1, ParamSingle1
    DefMsg MemFrs,DebugFrs,0, NoParamSingle

    DefMsg TeachFrs,Bug,0,NoParamSingle

;
; ****************************************************************************
