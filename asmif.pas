{$I STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
	UNIT AsmIF;

INTERFACE

	Uses
             {$IFDEF WINDOWS}
             WinDOS,Strings,
             {$ELSE}
             DOS,
             {$ENDIF}
             DecodeCm,
	     AsmDef,AsmSub,AsmPars;

TYPE
   PIfSave=^TIfSave;
   TIfSave=RECORD
	    Next:PIfSave;
            NestLevel:Integer;
	    SaveIfAsm:Boolean;
	    SaveExpr:TempResult;
	    State:(IfState_IFIF,IfState_IFELSE,
		   IfState_CASESWITCH,IfState_CASECASE,IfState_CASEELSE);
	    CaseFound:Boolean;
	   END;

VAR
   FirstIfSave:PIfSave;
   IfAsm:Boolean;         { FALSE: in einer neg. IF-Sequenz-->kein Code }


	FUNCTION CodeIFs:Boolean;

        FUNCTION SaveIFs:Integer;

        PROCEDURE RestoreIFs(Level:Integer);

        FUNCTION IFListMask:Boolean;

	PROCEDURE AsmIFInit;

IMPLEMENTATION

VAR
   ActiveIF:Boolean;    { ausgewertetes IF-Statement }

       FUNCTION GetIfVal(Cond:String):LongInt;
VAR
   IfOK:Boolean;
BEGIN
   FirstPassUnknown:=False;
   GetIfVal:=EvalIntExpression(Cond,Int32,IfOK);
   IF (FirstPassUnknown) OR (NOT IfOK) THEN
    BEGIN
     GetIfVal:=1;
     IF FirstPassUnknown THEN WrError(1820)
     ELSE IF NOT IfOK THEN WrError(1135);
    END;
END;

	PROCEDURE AddBoolFlag(Flag:Boolean);
BEGIN
   IF Flag THEN ListLine:='=>TRUE' ELSE ListLine:='=>FALSE';
END;

        PROCEDURE PushIF(IfExpr:LongInt);
VAR
   NewSave:PIfSave;
BEGIN
   New(NewSave);
   WITH NewSave^ DO
    BEGIN
     Next:=FirstIfSave;
     NestLevel:=SaveIFs+1;
     SaveIfAsm:=IfAsm;
     State:=IfState_IFIF; CaseFound:=IfExpr<>0;
    END;
   FirstIfSave:=NewSave;
   IfAsm:=IfAsm AND (IfExpr<>0);
END;

	PROCEDURE CodeIF;
VAR
   IfExpr:LongInt;
BEGIN
   ActiveIF:=IfAsm;

   IF NOT IfAsm THEN IfExpr:=1
   ELSE IF ArgCnt<>1 THEN
    BEGIN
     WrError(1110); IfExpr:=1;
    END
   ELSE IfExpr:=GetIfVal(ArgStr[1]);
   IF IfAsm THEN AddBoolFlag(IfExpr<>0);
   PushIF(IfExpr);
END;

	PROCEDURE CodeIFDEF;
VAR
   IfExpr:LongInt;
   Defined:Boolean;
BEGIN
   ActiveIF:=IfAsm;

   IF NOT IfAsm THEN IfExpr:=1
   ELSE IF ArgCnt<>1 THEN
    BEGIN
     WrError(1110); IfExpr:=1;
    END
   ELSE
    BEGIN
     Defined:=IsSymbolDefined(ArgStr[1]);
     IF IfAsm THEN
      IF Defined THEN ListLine:='=>DEFINED' ELSE ListLine:='=>UNDEFINED';
     IfExpr:=Ord(Defined XOR Memo('IFNDEF'));
    END;
   PushIF(IfExpr);
END;

	PROCEDURE CodeIFUSED;
VAR
   IfExpr:LongInt;
   Used:Boolean;
BEGIN
   ActiveIF:=IfAsm;

   IF NOT IfAsm THEN IfExpr:=1
   ELSE IF ArgCnt<>1 THEN
    BEGIN
     WrError(1110); IfExpr:=1;
    END
   ELSE
    BEGIN
     Used:=IsSymbolUsed(ArgStr[1]);
     IF Used THEN ListLine:='=>USED' ELSE ListLine:='=>UNUSED';
     IfExpr:=Ord(Used XOR Memo('IFNUSED'));
    END;
   PushIF(IfExpr);
END;

	PROCEDURE CodeIFEXIST;
VAR
   IfExpr:LongInt;
   Found:Boolean;
BEGIN
   ActiveIF:=IfAsm;

   IF NOT IfAsm THEN IfExpr:=1
   ELSE IF ArgCnt<>1 THEN
    BEGIN
     WrError(1110); IfExpr:=1;
    END
   ELSE
    BEGIN
     ArgPart:=ArgStr[1];
     IF ArgPart[1]='"' THEN Delete(ArgPart,1,1);
     IF ArgPart[Length(ArgPart)]='"' THEN Dec(Byte(ArgPart[0]));
     AddSuffix(ArgPart,IncSuffix);
     Found:=FSearch(ArgPart,'.;'+IncludeList)<>'';
     IF Found THEN ListLine:='=>FOUND' ELSE ListLine:='=>NOT FOUND';
     IfExpr:=Ord(Found XOR Memo('IFNEXIST'));
    END;
   PushIF(IfExpr);
END;

        PROCEDURE CodeIFB;
VAR
   IfExpr:LongInt;
   z:Integer;
   Blank:Boolean;
BEGIN
   ActiveIF:=IfAsm;

   IF NOT IfAsm THEN IfExpr:=1
   ELSE
    BEGIN
     Blank:=True;
     FOR z:=1 TO ArgCnt DO
      IF ArgStr[z]<>'' THEN Blank:=False;
     IF Blank THEN ListLine:='=>BLANK' ELSE ListLine:='NOT BLANK';
     IfExpr:=Ord(Blank XOR Memo('IFNB'));
    END;
   PushIF(IfExpr);
END;

	PROCEDURE CodeELSEIF;
VAR
   IfExpr:LongInt;
BEGIN
   IF FirstIfSave=Nil THEN WrError(1840)
   ELSE WITH FirstIfSave^ DO
    IF ArgCnt=0 THEN
     BEGIN
      IF State<>IfState_IFIF THEN WrError(1480)
      ELSE IF SaveIfAsm THEN
       BEGIN
	IfAsm:=NOT CaseFound;
	AddBoolFlag(NOT CaseFound);
       END;
      State:=IfState_IFELSE;
     END
    ELSE IF ArgCnt=1 THEN
     BEGIN
      IF State<>IfState_IFIF THEN WrError(1480)
      ELSE
       BEGIN
	IF NOT SaveIfAsm THEN IfExpr:=1
	ELSE IF CaseFound THEN IfExpr:=0
	ELSE IfExpr:=GetIfVal(ArgStr[1]);
	IfAsm:=SaveIfAsm AND (IfExpr<>0) AND (NOT CaseFound);
	IF SaveIfAsm THEN AddBoolFlag(IfExpr<>0);
	IF IfExpr<>0 THEN CaseFound:=True;
       END;
     END
    ELSE WrError(1110);

   ActiveIF:=(FirstIFSave=Nil) OR (FirstIfSave^.SaveIfAsm);
END;

	PROCEDURE CodeENDIF;
VAR
   NewSave:PIfSave;
BEGIN
   IF ArgCnt<>0 THEN WrError(1110);
   IF FirstIfSave=Nil THEN WrError(1840)
   ELSE WITH FirstIfSave^ DO
    BEGIN
     IF (State<>IfState_IFIF) AND (State<>IfState_IFELSE) THEN WrError(1480)
     ELSE
      BEGIN
       IfAsm:=SaveIfAsm;
       NewSave:=FirstIfSave; FirstIfSave:=NewSave^.Next;
       Dispose(NewSave);
      END
    END;

   ActiveIF:=IfAsm;
END;

	PROCEDURE EvalIfExpression(Cond:String; VAR erg:TempResult);
BEGIN
   FirstPassUnknown:=False;
   EvalExpression(Cond,erg);
   IF (erg.Typ=TempNone) OR (FirstPassUnknown) THEN
    BEGIN
     erg.Typ:=TempInt; Erg.Int:=1;
     IF FirstPassUnknown THEN WrError(1820);
    END;
END;

	PROCEDURE CodeSWITCH;
VAR
   NewSave:PIfSave;
BEGIN
   ActiveIF:=IfAsm;

   New(NewSave);
   WITH NewSave^ DO
    BEGIN
     Next:=FirstIfSave;
     NestLevel:=SaveIFs+1;
     SaveIfAsm:=IfAsm;
     CaseFound:=False; State:=IfState_CASESWITCH;
     IF ArgCnt<>1 THEN
      BEGIN
       SaveExpr.Typ:=TempInt; SaveExpr.Int:=1; IF (IfAsm) THEN WrError(1110);
      END
     ELSE
      BEGIN
       EvalIfExpression(ArgStr[1],SaveExpr);
       SetListLineVal(SaveExpr);
      END;
     FirstIfSave:=NewSave;
    END;
END;

	PROCEDURE CodeCASE;
VAR
   eq:Boolean;
   z:Byte;
   t:TempResult;
BEGIN
   IF FirstIfSave=Nil THEN WrError(1840)
   ELSE IF ArgCnt=0 THEN WrError(1110)
   ELSE WITH FirstIfSave^ DO
    BEGIN
     IF (State<>IfState_CASESWITCH) AND (State<>IfState_CASECASE) THEN WrError(1480)
     ELSE
      BEGIN
       IF NOT SaveIfAsm THEN eq:=True
       ELSE IF CaseFound THEN eq:=False
       ELSE
	BEGIN
	 eq:=False; z:=1;
	 REPEAT
	  EvalIfExpression(ArgStr[z],t);
	  eq:=(SaveExpr.Typ=t.Typ);
	  IF eq THEN
	   CASE t.Typ OF
	   TempInt:eq:=t.Int=SaveExpr.Int;
	   TempFloat:eq:=t.Float=SaveExpr.Float;
	   TempString:eq:=t.Ascii=SaveExpr.Ascii;
	   END;
	  Inc(z);
	 UNTIL (eq) OR (z>ArgCnt);
	END;
       IfAsm:=SaveIfAsm AND eq AND (NOT CaseFound);
       IF SaveIfAsm THEN AddBoolFlag(eq AND (NOT CaseFound));
       IF eq THEN CaseFound:=True;
       State:=IfState_CASECASE;
      END;
    END;

   ActiveIF:=(FirstIFSave=Nil) OR (FirstIFSave^.SaveIfAsm);
END;

	PROCEDURE CodeELSECASE;
BEGIN
   IF ArgCnt<>0 THEN WrError(1110)
   ELSE WITH FirstIfSave^ DO
    BEGIN
     IF (State<>IfState_CASESWITCH) AND (State<>IfState_CASECASE) THEN WrError(1480)
     ELSE IfAsm:=SaveIfAsm AND (NOT CaseFound);
     IF SaveIfAsm THEN AddBoolFlag(NOT CaseFound);
     CaseFound:=True;
     State:=IfState_CASEELSE;
    END;

   ActiveIF:=(FirstIFSave=Nil) OR (FirstIFSave^.SaveIfAsm);
END;

	PROCEDURE CodeENDCASE;
VAR
   NewSave:PIfSave;
BEGIN
   IF ArgCnt<>0 THEN WrError(1110);
   IF FirstIfSave=Nil THEN WrError(1840)
   ELSE WITH FirstIfSave^ DO
    BEGIN
     IF (State<>IfState_CASESWITCH) AND (State<>IfState_CASECASE) AND
	(State<>IfState_CASEELSE) THEN WrError(1480)
     ELSE
      BEGIN
       IfAsm:=SaveIfAsm;
       IF NOT CaseFound THEN WrError(100);
       NewSave:=FirstIfSave; FirstIfSave:=NewSave^.Next;
       Dispose(NewSave);
      END;
    END;

   ActiveIF:=IfAsm;
END;

	FUNCTION CodeIFs:Boolean;
BEGIN
   CodeIFs:=True; ActiveIF:=False;
   IF Memo('IF') THEN CodeIF
   ELSE IF (Memo('IFDEF')) OR (Memo('IFNDEF')) THEN CodeIFDEF
   ELSE IF (Memo('IFUSED')) OR (Memo('IFNUSED')) THEN CodeIFUSED
   ELSE IF (Memo('IFEXIST')) OR (Memo('IFNEXIST')) THEN CodeIFEXIST
   ELSE IF (Memo('IFB')) OR (Memo('IFNB')) THEN CodeIFB
   ELSE IF Memo('ELSEIF') THEN CodeELSEIF
   ELSE IF Memo('ENDIF') THEN CodeENDIF
   ELSE IF Memo('SWITCH') THEN CodeSWITCH
   ELSE IF Memo('CASE') THEN CodeCASE
   ELSE IF Memo('ELSECASE') THEN CodeELSECASE
   ELSE IF Memo('ENDCASE') THEN CodeENDCASE
   ELSE CodeIFs:=False;
END;

        FUNCTION SaveIFs:Integer;
BEGIN
   IF FirstIfSave=Nil THEN SaveIFs:=0
   ELSE SaveIFs:=FirstIfSave^.NestLevel;
END;

        PROCEDURE RestoreIFs(Level:Integer);
VAR
   OldSave:PIfSave;
BEGIN
   WHILE (FirstIfSave<>Nil) AND (FirstIfSave^.NestLevel<>Level) DO
    BEGIN
     OldSave:=FirstIfSave; FirstIfSave:=OldSave^.Next;
     IfAsm:=OldSave^.SaveIfAsm;
     Dispose(OldSave);
    END;
END;

        FUNCTION IFListMask:Boolean;
BEGIN
   CASE ListOn OF
   0:IFListMask:=True;
   1:IFListMask:=False;
   2:IFListMask:=(NOT ActiveIF) AND (NOT IfAsm);
   3:IFListMask:=ActiveIF OR (NOT IfAsm);
   END;
END;

        PROCEDURE AsmIFInit;
BEGIN
   IfAsm:=True;
END;

END.
