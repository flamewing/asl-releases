{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code16C5;

INTERFACE
        Uses NLS,Chunks,
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
   FixedOrderCnt=5;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'CLRW'  ; Code:$040),
		(Name:'NOP'   ; Code:$000),
		(Name:'CLRWDT'; Code:$004),
		(Name:'OPTION'; Code:$002),
		(Name:'SLEEP' ; Code:$003));

   LitOrderCnt=5;
   LitOrders:ARRAY[1..LitOrderCnt] OF FixedOrder=
	     ((Name:'ANDLW' ; Code:$e00),
	      (Name:'IORLW' ; Code:$d00),
	      (Name:'MOVLW' ; Code:$c00),
	      (Name:'RETLW' ; Code:$800),
	      (Name:'XORLW' ; Code:$f00));

   AriOrderCnt=14;
   AriOrders:ARRAY[1..AriOrderCnt] OF AriOrder=
	     ((Name:'ADDWF' ; Code:$1c0 ; DefaultDir:0),
	      (Name:'ANDWF' ; Code:$140 ; DefaultDir:0),
	      (Name:'COMF'  ; Code:$240 ; DefaultDir:1),
	      (Name:'DECF'  ; Code:$0c0 ; DefaultDir:1),
	      (Name:'DECFSZ'; Code:$2c0 ; DefaultDir:1),
	      (Name:'INCF'  ; Code:$280 ; DefaultDir:1),
	      (Name:'INCFSZ'; Code:$3c0 ; DefaultDir:1),
	      (Name:'IORWF' ; Code:$100 ; DefaultDir:0),
	      (Name:'MOVF'  ; Code:$200 ; DefaultDir:0),
	      (Name:'RLF'   ; Code:$340 ; DefaultDir:1),
	      (Name:'RRF'   ; Code:$300 ; DefaultDir:1),
	      (Name:'SUBWF' ; Code:$080 ; DefaultDir:0),
	      (Name:'SWAPF' ; Code:$380 ; DefaultDir:1),
	      (Name:'XORWF' ; Code:$180 ; DefaultDir:0));

   BitOrderCnt=4;
   BitOrders:ARRAY[1..BitOrderCnt] OF FixedOrder=
	     ((Name:'BCF'   ; Code:$400),
	      (Name:'BSF'   ; Code:$500),
	      (Name:'BTFSC' ; Code:$600),
	      (Name:'BTFSS' ; Code:$700));

   FOrderCnt=2;
   FOrders:ARRAY[1..FOrderCnt] OF FixedOrder=
	   ((Name:'CLRF'  ; Code:$060),
	    (Name:'MOVWF' ; Code:$020));

   D_CPU16C54=0;
   D_CPU16C55=1;
   D_CPU16C56=2;
   D_CPU16C57=3;

VAR
   CPU16C54,CPU16C55,CPU16C56,CPU16C57:CPUVar;


	FUNCTION ROMEnd:Word;
BEGIN
   CASE MomCPU-CPU16C54 OF
   D_CPU16C54,D_CPU16C55:ROMEnd:=511;
   D_CPU16C56:ROMEnd:=1023;
   D_CPU16C57:ROMEnd:=2047;
   END;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   Size:Word;
   AdrByte:Byte;
   ValOK:Boolean;
   HLocHandle:LongInt;
   z,z2:Integer;
   t:TempResult;
   MinV,MaxV:LongInt;
BEGIN
   DecodePseudo:=True;

   IF Memo('SFR') THEN
    BEGIN
     CodeEquate(SegData,0,$1f);
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
     IF ActPC=SegCode THEN MaxV:=4095 ELSE MaxV:=255; MinV:=-((MaxV+1) SHR 1);
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

	PROCEDURE MakeCode_16C5X;
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
         AdrWord:=EvalIntExpression(ArgStr[1],UInt5,OK);
	 IF OK THEN
          BEGIN
           ChkSpace(SegData);
           WAsmCode[0]:=Code+(AdrWord AND $1f);
           IF ArgCnt=1 THEN
            BEGIN
             CodeLen:=1; Inc(WAsmCode[0],DefaultDir SHL 5);
            END
           ELSE IF NLS_StrCaseCmp(ArgStr[2],'W')=0 THEN CodeLen:=1
           ELSE IF NLS_StrCaseCmp(ArgStr[2],'F')=0 THEN
            BEGIN
             CodeLen:=1; Inc(WAsmCode[0],$20);
            END
           ELSE
            BEGIN
             AdrWord:=EvalIntExpression(ArgStr[2],UInt1,OK);
             IF OK THEN
              BEGIN
               CodeLen:=1; Inc(WAsmCode[0],AdrWord SHL 5);
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
	 AdrWord:=EvalIntExpression(ArgStr[2],Int8,OK);
	 IF OK THEN
	  IF (AdrWord<0) OR (AdrWord>7) THEN WrError(1320)
	  ELSE
	   BEGIN
            WAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt5,OK);
	    IF OK THEN
             BEGIN
              CodeLen:=1; Inc(WAsmCode[0],Code+(AdrWord SHL 5));
              ChkSpace(SegData);
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
         AdrWord:=EvalIntExpression(ArgStr[1],UInt5,OK);
	 IF OK THEN
          BEGIN
           CodeLen:=1; WAsmCode[0]:=Code+AdrWord;
           ChkSpace(SegData);
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
       IF FirstPassUnknown THEN AdrWord:=5;
       IF OK THEN
	IF (AdrWord<5) OR (AdrWord>7) THEN WrError(1320)
	ELSE
	 BEGIN
	  CodeLen:=1; WAsmCode[0]:=$000+AdrWord;
	  ChkSpace(SegData);
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
        IF (AdrWord>ROMEnd) THEN WrError(1320)
	ELSE IF (Memo('CALL')) AND (AdrWord AND $100<>0) THEN WrError(1905)
	ELSE
	 BEGIN
	  ChkSpace(SegCode);
	  IF ((ProgCounter XOR AdrWord) AND $200)<>0 THEN
	   BEGIN
	    WAsmCode[CodeLen]:=$4a3+((AdrWord AND $200) SHR 1); { BCF/BSF 3,5 }
	    Inc(CodeLen);
	   END;
	  IF ((ProgCounter XOR AdrWord) AND $400)<>0 THEN
	   BEGIN
	    WAsmCode[CodeLen]:=$4c3+((AdrWord AND $400) SHR 2); { BCF/BSF 3,6 }
	    Inc(CodeLen);
	   END;
	  IF Memo('CALL') THEN WAsmCode[CodeLen]:=$900+(AdrWord AND $ff)
	  ELSE WAsmCode[CodeLen]:=$a00+(AdrWord AND $1ff);
	  Inc(CodeLen);
	 END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_16C5X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=RomEnd;
   SegData  : ok:=ProgCounter <=$1f;
   ELSE ok:=False;
   END;
   ChkPC_16C5X:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_16C5X:Boolean;
	Far;
BEGIN
   IsDef_16C5X:=Memo('SFR');
END;

        PROCEDURE SwitchFrom_16C5X;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_16C5X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$71; NOPCode:=$000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=2; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_16C5X; ChkPC:=ChkPC_16C5X; IsDef:=IsDef_16C5X;
   SwitchFrom:=SwitchFrom_16C5X;
END;

BEGIN
   CPU16C54:=AddCPU('16C54',SwitchTo_16C5X);
   CPU16C55:=AddCPU('16C55',SwitchTo_16C5X);
   CPU16C56:=AddCPU('16C56',SwitchTo_16C5X);
   CPU16C57:=AddCPU('16C57',SwitchTo_16C5X);
END.
