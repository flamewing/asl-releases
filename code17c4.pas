{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code17C4;

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
	     DefaultDir:Byte;
	     Code:Word;
	    END;

CONST
   FixedOrderCnt=5;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'RETFIE'; Code:$0005),
		(Name:'RETURN'; Code:$0002),
		(Name:'CLRWDT'; Code:$0004),
		(Name:'NOP'   ; Code:$0000),
		(Name:'SLEEP' ; Code:$0003));

   LittOrderCnt=8;
   LittOrders:ARRAY[1..LittOrderCnt] OF FixedOrder=
	      ((Name:'MOVLB' ; Code:$b800),
	       (Name:'ADDLW' ; Code:$b100),
	       (Name:'ANDLW' ; Code:$b500),
	       (Name:'IORLW' ; Code:$b300),
	       (Name:'MOVLW' ; Code:$b000),
	       (Name:'SUBLW' ; Code:$b200),
	       (Name:'XORLW' ; Code:$b400),
	       (Name:'RETLW' ; Code:$b600));

   AriOrderCnt=23;
   AriOrders:ARRAY[1..AriOrderCnt] OF AriOrder=
	     ((Name:'ADDWF' ; DefaultDir:0; Code:$0e00),
	      (Name:'ADDWFC'; DefaultDir:0; Code:$1000),
	      (Name:'ANDWF' ; DefaultDir:0; Code:$0a00),
	      (Name:'CLRF'  ; DefaultDir:1; Code:$2800),
	      (Name:'COMF'  ; DefaultDir:1; Code:$1200),
	      (Name:'DAW'   ; DefaultDir:1; Code:$2e00),
	      (Name:'DECF'  ; DefaultDir:1; Code:$0600),
	      (Name:'INCF'  ; DefaultDir:1; Code:$1400),
	      (Name:'IORWF' ; DefaultDir:0; Code:$0800),
	      (Name:'NEGW'  ; DefaultDir:1; Code:$2c00),
	      (Name:'RLCF'  ; DefaultDir:1; Code:$1a00),
	      (Name:'RLNCF' ; DefaultDir:1; Code:$2200),
	      (Name:'RRCF'  ; DefaultDir:1; Code:$1800),
	      (Name:'RRNCF' ; DefaultDir:1; Code:$2000),
	      (Name:'SETF'  ; DefaultDir:1; Code:$2a00),
	      (Name:'SUBWF' ; DefaultDir:0; Code:$0400),
	      (Name:'SUBWFB'; DefaultDir:0; Code:$0200),
	      (Name:'SWAPF' ; DefaultDir:1; Code:$1c00),
	      (Name:'XORWF' ; DefaultDir:0; Code:$0c00),
	      (Name:'DECFSZ'; DefaultDir:1; Code:$1600),
	      (Name:'DCFSNZ'; DefaultDir:1; Code:$2600),
	      (Name:'INCFSZ'; DefaultDir:1; Code:$1e00),
	      (Name:'INFSNZ'; DefaultDir:1; Code:$2400));

   BitOrderCnt=5;
   BitOrders:ARRAY[1..BitOrderCnt] OF FixedOrder=
	     ((Name:'BCF'  ; Code:$8800),
	      (Name:'BSF'  ; Code:$8000),
	      (Name:'BTFSC'; Code:$9800),
	      (Name:'BTFSS'; Code:$9000),
	      (Name:'BTG'  ; Code:$3800));

   FOrderCnt=5;
   FOrders:ARRAY[1..FOrderCnt] OF FixedOrder=
	   ((Name:'MOVWF' ; Code:$0100),
	    (Name:'CPFSEQ'; Code:$3100),
	    (Name:'CPFSGT'; Code:$3200),
	    (Name:'CPFSLT'; Code:$3000),
	    (Name:'TSTFSZ'; Code:$3300));

VAR
   CPU17C42:CPUVar;

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
     CodeEquate(SegData,0,$ff);
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
     IF ActPC=SegCode THEN MaxV:=$ffff ELSE MaxV:=$ff; MinV:=-((MaxV+1) SHR 1);
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
           IF ChkRange(Int,MinV,MaxV) THEN
            BEGIN
             IF ActPC=SegCode THEN WAsmCode[CodeLen]:=Int
             ELSE BAsmCode[CodeLen]:=Int;
             Inc(CodeLen);
            END;
          TempFloat:
           BEGIN
            WrError(1135); ValOK:=False;
           END;
          TempString:
           BEGIN
            FOR z2:=1 TO Length(Ascii) DO
             BEGIN
              AdrByte:=Ord(CharTransTable[Ascii[z2]]);
              IF ActPC=SegData THEN
               BEGIN
                BAsmCode[CodeLen]:=AdrByte; Inc(CodeLen);
               END
              ELSE IF Odd(z2) THEN
               BEGIN
                WAsmCode[CodeLen]:=AdrByte; Inc(CodeLen);
               END
              ELSE Inc(WAsmCode[CodeLen-1],Word(AdrByte) SHL 8);
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

	PROCEDURE MakeCode_17C4X;
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

   { kein Argument }

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

   { konstantes Argument }

   FOR z:=1 TO LittOrderCnt DO
    WITH LittOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[1],Int8,OK);
	 IF OK THEN
	  BEGIN
	   WAsmCode[0]:=Code+(AdrWord AND $ff); CodeLen:=1;
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
	 AdrWord:=EvalIntExpression(ArgStr[1],Int8,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegData);
	   WAsmCode[0]:=Code+(AdrWord AND $ff);
	   IF ArgCnt=1 THEN
	    BEGIN
	     CodeLen:=1; Inc(WAsmCode[0],DefaultDir SHL 8);
	    END
           ELSE IF NLS_StrCaseCmp(ArgStr[2],'W')=0 THEN CodeLen:=1
           ELSE IF NLS_StrCaseCmp(ArgStr[2],'F')=0 THEN
	    BEGIN
	     CodeLen:=1; Inc(WAsmCode[0],$100);
	    END
	   ELSE
	    BEGIN
	     AdrWord:=EvalIntExpression(ArgStr[2],Int8,OK);
	     IF OK THEN
	      IF (AdrWord<>0) AND (AdrWord<>1) THEN WrError(1320)
	      ELSE
	       BEGIN
		CodeLen:=1; Inc(WAsmCode[0],AdrWord SHL 8);
	       END;
	    END;
	  END;
	END;
       Exit;
      END;

   { Bitoperationen }

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
           WAsmCode[0]:=EvalIntExpression(ArgStr[1],Int8,OK);
           IF OK THEN
            BEGIN
             CodeLen:=1; Inc(WAsmCode[0],Code+(AdrWord SHL 8));
             ChkSpace(SegData);
            END;
          END;
	END;
       Exit;
      END;

   { Register als Operand }

   FOR z:=1 TO FOrderCnt DO
    WITH FOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[1],Int8,OK);
	 IF OK THEN
	  BEGIN
	   CodeLen:=1; WAsmCode[0]:=Code+AdrWord;
	   ChkSpace(SegData);
	  END;
	END;
       Exit;
      END;

   IF (Memo('MOVFP')) OR (Memo('MOVPF')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('MOVFP') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       AdrWord:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
	IF (AdrWord<0) OR (AdrWord>31) THEN WrError(1320)
	ELSE
	 BEGIN
	  WAsmCode[0]:=EvalIntExpression(ArgStr[2],Int8,OK);
	  IF OK THEN
	   BEGIN
	    WAsmCode[0]:=Lo(WAsmCode[0])+(AdrWord SHL 8)+$4000;
	    IF Memo('MOVFP') THEN Inc(WAsmCode[0],$2000);
	    CodeLen:=1;
	   END;
	 END;
      END;
     Exit;
    END;

   IF (Memo('TABLRD')) OR (Memo('TABLWT')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=Lo(EvalIntExpression(ArgStr[3],Int8,OK));
       IF OK THEN
	BEGIN
         AdrWord:=EvalIntExpression(ArgStr[2],UInt1,OK);
	 IF OK THEN
          BEGIN
           Inc(WAsmCode[0],AdrWord SHL 8);
           AdrWord:=EvalIntExpression(ArgStr[1],UInt1,OK);
           IF OK THEN
            BEGIN
             Inc(WAsmCode[0],$a800+(AdrWord SHL 9));
             IF Memo('TABLWT') THEN Inc(WAsmCode[0],$400);
             CodeLen:=1;
            END;
          END;
	END;
      END;
     Exit;
    END;

   IF (Memo('TLRD')) OR (Memo('TLWT')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=Lo(EvalIntExpression(ArgStr[2],Int8,OK));
       IF OK THEN
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[1],UInt1,OK);
	 IF OK THEN
	  BEGIN
	   Inc(WasmCode[0],(AdrWord SHL 9)+$a000);
	   IF Memo('TLWT') THEN Inc(WasmCode[0],$400);
	   CodeLen:=1;
	  END;
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
	IF (ProgCounter XOR AdrWord) AND $e000<>0 THEN WrError(1910)
	ELSE
	 BEGIN
	  WAsmCode[0]:=$c000+(AdrWord AND $1fff);
	  IF Memo('CALL') THEN Inc(WAsmCode[0],$2000);
	  CodeLen:=1;
	 END;
      END;
     Exit;
    END;

   IF Memo('LCALL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=3;
	 WAsmCode[0]:=$b000+Hi(AdrWord);
	 WAsmCode[1]:=$0103;
	 WAsmCode[2]:=$b700+Lo(AdrWord);
	END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_17C4X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=$ffff;
   SegData  : ok:=ProgCounter <=$ff;
   ELSE ok:=False;
   END;
   ChkPC_17C4X:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_17C4X:Boolean;
	Far;
BEGIN
   IsDef_17C4X:=Memo('SFR');
END;

        PROCEDURE SwitchFrom_17C4X;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_17C4X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$72; NOPCode:=$0000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=2; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;

   MakeCode:=MakeCode_17C4X; ChkPC:=ChkPC_17C4X; IsDef:=IsDef_17C4X;
   SwitchFrom:=SwitchFrom_17C4X;
END;

BEGIN
   CPU17C42:=AddCPU('17C42',SwitchTo_17C4X);
END.
