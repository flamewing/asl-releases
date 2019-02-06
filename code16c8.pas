{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

	UNIT Code16C8;

INTERFACE
        Uses Chunks,NLS,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	      END;
   AriOrder=RECORD
	     Name:String[6];
	     Code:Word;
	     DefaultDir:Byte;
	    END;

CONST
   D_CPU16C64=0;
   D_CPU16C84=1;

   FixedOrderCnt=7;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'CLRW'  ; Code:$0100),
		(Name:'NOP'   ; Code:$0000),
		(Name:'CLRWDT'; Code:$0064),
		(Name:'OPTION'; Code:$0062),
		(Name:'SLEEP' ; Code:$0063),
		(Name:'RETFIE'; Code:$0009),
		(Name:'RETURN'; Code:$0008));

   LitOrderCnt=7;
   LitOrders:ARRAY[1..LitOrderCnt] OF FixedOrder=
	     ((Name:'ADDLW' ; Code:$3e00),
	      (Name:'ANDLW' ; Code:$3900),
	      (Name:'IORLW' ; Code:$3800),
	      (Name:'MOVLW' ; Code:$3000),
	      (Name:'RETLW' ; Code:$3400),
	      (Name:'SUBLW' ; Code:$3c00),
	      (Name:'XORLW' ; Code:$3a00));

   AriOrderCnt=14;
   AriOrders:ARRAY[1..AriOrderCnt] OF AriOrder=
	     ((Name:'ADDWF' ; Code:$0700 ; DefaultDir:0),
	      (Name:'ANDWF' ; Code:$0500 ; DefaultDir:0),
	      (Name:'COMF'  ; Code:$0900 ; DefaultDir:1),
	      (Name:'DECF'  ; Code:$0300 ; DefaultDir:1),
	      (Name:'DECFSZ'; Code:$0b00 ; DefaultDir:1),
	      (Name:'INCF'  ; Code:$0a00 ; DefaultDir:1),
	      (Name:'INCFSZ'; Code:$0f00 ; DefaultDir:1),
	      (Name:'IORWF' ; Code:$0400 ; DefaultDir:0),
	      (Name:'MOVF'  ; Code:$0800 ; DefaultDir:0),
	      (Name:'RLF'   ; Code:$0d00 ; DefaultDir:1),
	      (Name:'RRF'   ; Code:$0c00 ; DefaultDir:1),
	      (Name:'SUBWF' ; Code:$0200 ; DefaultDir:0),
	      (Name:'SWAPF' ; Code:$0e00 ; DefaultDir:1),
	      (Name:'XORWF' ; Code:$0600 ; DefaultDir:0));

   BitOrderCnt=4;
   BitOrders:ARRAY[1..BitOrderCnt] OF FixedOrder=
	     ((Name:'BCF'   ; Code:$1000),
	      (Name:'BSF'   ; Code:$1400),
	      (Name:'BTFSC' ; Code:$1800),
	      (Name:'BTFSS' ; Code:$1c00));

   FOrderCnt=2;
   FOrders:ARRAY[1..FOrderCnt] OF FixedOrder=
	   ((Name:'CLRF'  ; Code:$0180),
	    (Name:'MOVWF' ; Code:$0080));

VAR
   CPU16C64,CPU16C84:CPUVar;


	FUNCTION ROMEnd:Word;
BEGIN
   CASE MomCPU-CPU16C64 OF
   D_CPU16C64:ROMEnd:=$7ff;
   D_CPU16C84:ROMEnd:=$3ff;
   END;
END;

	FUNCTION EvalFExpression(VAR Asc:String; VAR OK:Boolean):Word;
VAR
   h:LongInt;
BEGIN
   h:=EvalIntExpression(Asc,UInt9,OK);
   IF OK THEN
    IF (h<0) OR (h>$1ff) THEN
     BEGIN
      WrError(1320); OK:=False;
     END
    ELSE
     BEGIN
      ChkSpace(SegData); EvalFExpression:=h AND $7f;
     END;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   Size:Word;
   AdrWord:Word;
   ValOK:Boolean;
   HLocHandle:LongInt;
   z,z2:Integer;
   t:TempResult;
   MinV,MaxV:LongInt;
BEGIN
   DecodePseudo:=True;

   IF Memo('SFR') THEN
    BEGIN
     CodeEquate(SegData,0,511);
     Exit;
    END;

   IF Memo('RES') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Size:=EvalIntExpression(ArgStr[1],Int16,ValOK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (ValOK) AND (NOT FirstPassUnknown) THEN
	BEGIN
	 DontPrint:=True;
	 CodeLen:=Size;
	 IF MakeUseList THEN
	  IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
	END;
      END;
     Exit;
    END;

   IF Memo('DATA') THEN
    BEGIN
     IF ActPC=SegCode THEN MaxV:=16383 ELSE MaxV:=255; MinV:=-((MaxV+1) SHR 1);
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       ValOK:=True;
       FOR z:=1 TO ArgCnt DO
	IF ValOK THEN
	 BEGIN
          FirstPassUnknown:=False;
	  EvalExpression(ArgStr[z],t);
          IF (FirstPassUnknown) AND (t.Typ=TempInt) THEN t.Int:=t.Int AND MaxV;
	  WITH t DO
	  CASE Typ OF
          TempInt:
           IF ChkRange(t.Int,MinV,MaxV) THEN
            IF ActPC=SegCode THEN
             BEGIN
              WAsmCode[CodeLen]:=Int AND MaxV; Inc(CodeLen);
             END
            ELSE
             BEGIN
              BAsmCode[CodeLen]:=Int AND MaxV; Inc(CodeLen);
             END;
          TempFloat:
           BEGIN
            WrError(1135); ValOK:=False;
           END;
          TempString:
           BEGIN
            FOR z2:=1 TO Length(Ascii) DO
             BEGIN
              IF ActPC=SegCode THEN
               WAsmCode[CodeLen]:=Ord(CharTransTable[Ascii[z2]])
              ELSE
               BAsmCode[CodeLen]:=Ord(CharTransTable[Ascii[z2]]);
              Inc(CodeLen);
             END;
           END;
	  ELSE ValOK:=False;
	  END;
	 END;
       IF NOT ValOK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('ZERO') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Size:=EvalIntExpression(ArgStr[1],Int16,ValOK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (ValOK) AND (NOT FirstPassUnknown) THEN
	IF Size SHL 1>MaxCodeLen THEN WrError(1920)
	ELSE
	 BEGIN
	  CodeLen:=Size;
	  FillChar(WAsmCode,2*Size,0);
	 END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_16C8X;
	Far;
VAR
   OK:Boolean;
   AdrWord:Word;
   z:Integer;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
	BEGIN
	 CodeLen:=1; WAsmCode[0]:=Code;
	 IF Memo('OPTION') THEN WrError(130);
	END;
       Exit;
      END;

   { nur ein Literal als Argument }

   FOR z:=1 TO LitOrderCnt DO
    WITH LitOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[1],Int8,OK);
	 IF OK THEN
	  BEGIN
	   CodeLen:=1; WAsmCode[0]:=Code+(AdrWord AND $ff);
	  END;
	END;
       Exit;
      END;

   { W-mit-f-Operationen }

   FOR z:=1 TO AriOrderCnt DO
    WITH AriOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt=0) OR (ArgCnt>2) THEN WrError(1110)
       ELSE
	BEGIN
	 AdrWord:=EvalFExpression(ArgStr[1],OK);
	 IF OK THEN
	  BEGIN
	   WAsmCode[0]:=Code+AdrWord;
	    IF ArgCnt=1 THEN
	     BEGIN
	      CodeLen:=1; Inc(WAsmCode[0],DefaultDir SHL 7);
	     END
            ELSE IF NLS_StrCaseCmp(ArgStr[2],'W')=0 THEN CodeLen:=1
            ELSE IF NLS_StrCaseCmp(ArgStr[2],'F')=0 THEN
	     BEGIN
	      CodeLen:=1; Inc(WAsmCode[0],$80);
	     END
	    ELSE
	     BEGIN
	      AdrWord:=EvalIntExpression(ArgStr[2],UInt1,OK);
	      IF OK THEN
	       BEGIN
		CodeLen:=1; Inc(WAsmCode[0],AdrWord SHL 7);
	       END;
	     END;
	   END;
	END;
       Exit;
      END;

   FOR z:=1 TO BitOrderCnt DO
    WITH BitOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[2],UInt3,OK);
	 IF OK THEN
	  BEGIN
	   WAsmCode[0]:=EvalFExpression(ArgStr[1],OK);
	   IF OK THEN
	    BEGIN
	     CodeLen:=1; Inc(WAsmCode[0],Code+(AdrWord SHL 7));
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO FOrderCnt DO
    WITH FOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrWord:=EvalFExpression(ArgStr[1],OK);
	 IF OK THEN
	  BEGIN
	   CodeLen:=1; WAsmCode[0]:=Code+AdrWord;
	  END;
	END;
       Exit;
      END;

   IF Memo('TRIS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrWord:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF FirstPAssUnknown THEN AdrWord:=5;
       IF OK THEN
	IF (AdrWord<5) OR (AdrWord>6) THEN WrError(1320)
	ELSE
	 BEGIN
	  CodeLen:=1; WAsmCode[0]:=$0060+AdrWord;
	  ChkSpace(SegData); WrError(130);
	 END;
      END;
     Exit;
    END;

   IF (Memo('CALL')) OR (Memo('GOTO')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF (AdrWord<0) OR (AdrWord>ROMEnd) THEN WrError(1320)
	ELSE
	 BEGIN
	  ChkSpace(SegCode);
	  IF ((ProgCounter XOR AdrWord) AND $800)<>0 THEN
	   BEGIN
	    WAsmCode[CodeLen]:=$118a+((AdrWord AND $800) SHR 1); { BCF/BSF 10,3 }
	    Inc(CodeLen);
	   END;
	  IF ((ProgCounter XOR AdrWord) AND $1000)<>0 THEN
	   BEGIN
	    WAsmCode[CodeLen]:=$120a+((AdrWord AND $400) SHR 2); { BCF/BSF 10,4 }
	    Inc(CodeLen);
	   END;
	  IF Memo('CALL') THEN WAsmCode[CodeLen]:=$2000+(AdrWord AND $7ff)
	  ELSE WAsmCode[CodeLen]:=$2800+(AdrWord AND $7ff);
	  Inc(CodeLen);
	 END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_16C8X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=RomEnd+$300;
   SegData  : ok:=ProgCounter <=$1ff;
   ELSE ok:=False;
   END;
   ChkPC_16C8X:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_16C8X:Boolean;
	Far;
BEGIN
   IsDef_16C8X:=Memo('SFR');
END;

        PROCEDURE SwitchFrom_16C8X;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_16C8X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$70; NOPCode:=$0000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=2; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;

   MakeCode:=MakeCode_16C8X; ChkPC:=ChkPC_16C8X; IsDef:=IsDef_16C8X;
   SwitchFrom:=SwitchFrom_16C8X;
END;

BEGIN
   CPU16C64:=AddCPU('16C64',SwitchTo_16C8X);
   CPU16C84:=AddCPU('16C84',SwitchTo_16C8X);
END.
