{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code3201;

INTERFACE
        Uses NLS,Chunks,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	      END;

   AdrOrder=RECORD
	     Name:String[6];
	     Code:Word;
	     Must1:Boolean;
	    END;

   AdrShiftOrder=RECORD
		  Name:String;
		  Code:Word;
		  AllowShifts:Word;
		 END;

   ImmOrder=RECORD
	     Name:String;
	     Code:Word;
	     Min,Max:Integer;
	     Mask:Word;
	    END;

CONST
   FixedOrderCnt=14;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'ABS'   ; Code:$7f88),
		(Name:'APAC'  ; Code:$7f8f),
		(Name:'CALA'  ; Code:$7f8c),
		(Name:'DINT'  ; Code:$7f81),
		(Name:'EINT'  ; Code:$7f82),
		(Name:'NOP'   ; Code:$7f80),
		(Name:'PAC'   ; Code:$7f8e),
		(Name:'POP'   ; Code:$7f9d),
		(Name:'PUSH'  ; Code:$7f9c),
		(Name:'RET'   ; Code:$7f8d),
		(Name:'ROVM'  ; Code:$7f8a),
		(Name:'SOVM'  ; Code:$7f8b),
		(Name:'SPAC'  ; Code:$7f90),
		(Name:'ZAC'   ; Code:$7f89));


   JmpOrderCnt=11;
   JmpOrders:ARRAY[1..JmpOrderCnt] OF FixedOrder=
	       ((Name:'B'     ; Code:$f900),
		(Name:'BANZ'  ; Code:$f400),
		(Name:'BGEZ'  ; Code:$fd00),
		(Name:'BGZ'   ; Code:$fc00),
		(Name:'BIOZ'  ; Code:$f600),
		(Name:'BLEZ'  ; Code:$fb00),
		(Name:'BLZ'   ; Code:$fa00),
		(Name:'BNZ'   ; Code:$fe00),
		(Name:'BV'    ; Code:$f500),
		(Name:'BZ'    ; Code:$ff00),
		(Name:'CALL'  ; Code:$f800));

   AdrOrderCnt=21;
   AdrOrders:ARRAY[1..AdrOrderCnt] OF AdrOrder=
	       ((Name:'ADDH'  ; Code:$6000; Must1:False),
		(Name:'ADDS'  ; Code:$6100; Must1:False),
		(Name:'AND'   ; Code:$7900; Must1:False),
		(Name:'DMOV'  ; Code:$6900; Must1:False),
		(Name:'LDP'   ; Code:$6f00; Must1:False),
		(Name:'LST'   ; Code:$7b00; Must1:False),
		(Name:'LT'    ; Code:$6a00; Must1:False),
		(Name:'LTA'   ; Code:$6c00; Must1:False),
		(Name:'LTD'   ; Code:$6b00; Must1:False),
		(Name:'MAR'   ; Code:$6800; Must1:False),
		(Name:'MPY'   ; Code:$6d00; Must1:False),
		(Name:'OR'    ; Code:$7a00; Must1:False),
		(Name:'SST'   ; Code:$7c00; Must1:True ),
		(Name:'SUBC'  ; Code:$6400; Must1:False),
		(Name:'SUBH'  ; Code:$6200; Must1:False),
		(Name:'SUBS'  ; Code:$6300; Must1:False),
		(Name:'TBLR'  ; Code:$6700; Must1:False),
		(Name:'TBLW'  ; Code:$7d00; Must1:False),
		(Name:'XOR'   ; Code:$7800; Must1:False),
		(Name:'ZALH'  ; Code:$6500; Must1:False),
		(Name:'ZALS'  ; Code:$6600; Must1:False));

   AdrShiftOrderCnt=5;
   AdrShiftOrders:ARRAY[1..AdrShiftOrderCnt] OF AdrShiftOrder=
	       ((Name:'ADD'   ; Code:$0000; AllowShifts:$ffff),
		(Name:'LAC'   ; Code:$2000; AllowShifts:$ffff),
		(Name:'SACH'  ; Code:$5800; AllowShifts:$0013),
		(Name:'SACL'  ; Code:$5000; AllowShifts:$0001),
		(Name:'SUB'   ; Code:$1000; AllowShifts:$ffff));

   ImmOrderCnt=3;
   ImmOrders:ARRAY[1..ImmOrderCnt] OF ImmOrder=
	       ((Name:'LACK'  ; Code:$7e00; Min:    0; Max: 255; Mask:  $ff),
		(Name:'LDPK'  ; Code:$6e00; Min:    0; Max:   1; Mask:   $1),
		(Name:'MPYK'  ; Code:$8000; Min:-4096; Max:4095; Mask:$1fff));
VAR
   AdrMode:Word;
   AdrOK:Boolean;

   CPU32010,CPU32015:CPUVar;

	FUNCTION EvalARExpression(Asc:String; VAR OK:Boolean):Word;
BEGIN
   OK:=True;
   IF NLS_StrCaseCmp(Asc,'AR0')=0 THEN
    BEGIN
     EvalARExpression:=0; Exit;
    END;
   IF NLS_StrCaseCmp(Asc,'AR1')=0 THEN
    BEGIN
     EvalARExpression:=1; Exit;
    END;
   EvalARExpression:=EvalIntExpression(Asc,UInt1,OK);
END;

	PROCEDURE DecodeAdr(Arg:String; Aux:Integer; Must1:Boolean);
VAR
   h:Byte;
   z:Integer;
BEGIN
   AdrOK:=False;
   IF (Arg='*') OR (Arg='*-') OR (Arg='*+') THEN
    BEGIN
     AdrMode:=$88;
     IF Length(Arg)=2 THEN
      BEGIN
       IF Arg[2]='+' THEN Inc(AdrMode,$20) ELSE Inc(AdrMode,$10);
      END;
     IF Aux<=ArgCnt THEN
      BEGIN
       h:=EvalARExpression(ArgStr[Aux],AdrOK);
       IF AdrOK THEN
	BEGIN
	 AdrMode:=AdrMode AND $f7; Inc(AdrMode,h);
	END;
      END
     ELSE AdrOK:=True;
    END
   ELSE IF Aux<=ArgCnt THEN WrError(1110)
   ELSE
    BEGIN
     IF (Length(Arg)>3) AND (NLS_StrCaseCmp(Copy(Arg,1,3),'DAT')=0) THEN
      BEGIN
       AdrOK:=True;
       FOR z:=4 TO Length(Arg) DO
	IF (Arg[z]>'9') OR (Arg[z]<'0') THEN AdrOK:=False;
       IF AdrOK THEN h:=EvalIntExpression(Copy(Arg,4,Length(Arg)-3),Int8,AdrOK);
      END;
     IF NOT AdrOK THEN h:=EvalIntExpression(Arg,Int8,AdrOK);
     IF AdrOK THEN
      IF (Must1) AND (h<$80) AND (NOT FirstPassUnknown) THEN
       BEGIN
	WrError(1315); AdrOK:=False;
       END
      ELSE
       BEGIN
	AdrMode:=h AND $7f; ChkSpace(SegData);
       END;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   HLocHandle:LongInt;
   AdrByte:Byte;
   Size:Word;
   z,z2:Integer;
   t:TempResult;
   OK:Boolean;
BEGIN
   DecodePseudo:=True;

   IF Memo('PORT') THEN
    BEGIN
     CodeEquate(SegIO,0,7);
     Exit;
    END;

   IF Memo('RES') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Size:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (OK) AND (NOT FirstPassUnknown) THEN
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
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  EvalExpression(ArgStr[z],t);
	  WITH t DO
	  CASE Typ OF
	  TempInt:   IF (Int<-32768) OR (Int>$ffff) THEN
		      BEGIN
		       WrError(1320); OK:=False;
		      END
		     ELSE
		      BEGIN
		       WAsmCode[CodeLen]:=Int; Inc(CodeLen);
		      END;
	  TempFloat: BEGIN
		      WrError(1135); OK:=False;
		     END;
	  TempString:BEGIN
		      FOR z2:=1 TO Length(Ascii) DO
		       BEGIN
			IF Odd(z2) THEN
			 WAsmCode[CodeLen]:=Ord(CharTransTable[Ascii[z2]])
			ELSE
			 BEGIN
			  Inc(WAsmCode[CodeLen],Word(Ord(CharTransTable[Ascii[z2]])) SHL 8);
			  Inc(CodeLen);
			 END;
		       END;
		      IF Odd(Length(Ascii)) THEN Inc(CodeLen);
		     END;
	  ELSE OK:=False;
	  END;
	 END;
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_3201X;
	Far;
VAR
   OK,HasSh:Boolean;
   AdrWord:Word;
   AdrLong:LongInt;
   Cnt,z:Integer;
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

   { SprÅnge }

   FOR z:=1 TO JmpOrderCnt DO
    WITH JmpOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 WAsmCode[1]:=EvalIntExpression(ArgStr[1],Int16,OK);
	 IF OK THEN
	  IF WAsmCode[1]>$fff THEN WrError(1320)
	  ELSE
	   BEGIN
	    CodeLen:=2; WAsmCode[0]:=Code;
	   END;
	END;
       Exit;
      END;

   { nur Adresse }

   FOR z:=1 TO AdrOrderCnt DO
    WITH AdrOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],2,Must1);
	 IF AdrOK THEN
	  BEGIN
	   CodeLen:=1; WAsmCode[0]:=Code+AdrMode;
	  END;
	END;
       Exit;
      END;

   { Adresse & schieben }

   FOR z:=1 TO AdrShiftOrderCnt DO
    WITH AdrShiftOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>3) THEN WrError(1110)
       ELSE
	BEGIN
         IF ArgStr[1][1]='*' THEN
          IF ArgCnt=2 THEN
           IF NLS_StrCaseCmp(Copy(ArgStr[2],1,2),'AR')=0 THEN
            BEGIN
             HasSh:=False; Cnt:=2;
            END
           ELSE
            BEGIN
             HasSh:=True; Cnt:=3;
            END
          ELSE
           BEGIN
            HasSh:=True; Cnt:=3;
           END
         ELSE
          BEGIN
           Cnt:=3; HasSh:=(ArgCnt=2);
          END;
         DecodeAdr(ArgStr[1],Cnt,False);
	 IF AdrOK THEN
	  BEGIN
           IF NOT HasSh THEN
	    BEGIN
	     OK:=True; AdrWord:=0;
	    END
	   ELSE
	    BEGIN
	     AdrWord:=EvalIntExpression(ArgStr[2],Int4,OK);
	     IF (OK) AND (FirstPassUnknown) THEN AdrWord:=0;
	    END;
	   IF OK THEN
	    IF AllowShifts AND (1 SHL AdrWord)=0 THEN WrError(1380)
	    ELSE
	     BEGIN
	      CodeLen:=1; WAsmCode[0]:=Code+AdrMode+(AdrWord SHL 8);
	     END;
	  END;
	END;
       Exit;
      END;

   { Ein/Ausgabe }

   IF (Memo('IN')) OR (Memo('OUT')) THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],3,False);
       IF AdrOK THEN
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[2],UInt3,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegIO);
	   CodeLen:=1;
	   WAsmCode[0]:=$4000+AdrMode+(AdrWord SHL 8);
	   IF Memo('OUT') THEN Inc(WAsmCode[0],$800);
	  END;
	END;
      END;
     Exit;
    END;

   { konstantes Argument }

   FOR z:=1 TO ImmOrderCnt DO
    WITH ImmOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrLong:=EvalIntExpression(ArgStr[1],Int32,OK);
	 IF OK THEN
	  BEGIN
	   IF FirstPassUnknown THEN AdrLong:=AdrLong AND Mask;
	   IF AdrLong<Min THEN WrError(1315)
	   ELSE IF AdrLong>Max THEN WrError(1320)
	   ELSE
	    BEGIN
	     CodeLen:=1; WAsmCode[0]:=Code+(AdrLong AND Mask);
	    END;
	  END;
	END;
       Exit;
      END;

   { mit Hilfsregistern }

   IF Memo('LARP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalARExpression(ArgStr[1],OK);
       IF OK THEN
	BEGIN
         CodeLen:=1; WAsmCode[0]:=$6880+AdrWord;
	END;
      END;
     Exit;
    END;

   IF (Memo('LAR')) OR (Memo('SAR')) THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalARExpression(ArgStr[1],OK);
       IF OK THEN
	BEGIN
	 DecodeAdr(ArgStr[2],3,False);
	 IF AdrOK THEN
	  BEGIN
	   CodeLen:=1;
	   WAsmCode[0]:=$3000+AdrMode+(AdrWord SHL 8);
	   IF Memo('LAR') THEN Inc(WAsmCode[0],$800);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('LARK') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalARExpression(ArgStr[1],OK);
       IF OK THEN
	BEGIN
	 WAsmCode[0]:=EvalIntExpression(ArgStr[2],Int8,OK);
	 IF OK THEN
	  BEGIN
	   CodeLen:=1;
	   WAsmCode[0]:=Lo(WAsmCode[0])+$7000+(AdrWord SHL 8);
	  END;
	END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_3201X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=$fff;
   SegData  : IF MomCPU=CPU32010 THEN ok:=ProgCounter <=$8f
				 ELSE ok:=ProgCounter <=$ff;
   SegIO    : ok:=ProgCounter <=7;
   ELSE ok:=False;
   END;
   ChkPC_3201X:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_3201X:Boolean;
	Far;
BEGIN
   IsDef_3201X:=Memo('PORT');
END;

        PROCEDURE SwitchFrom_3201X;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_3201X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$74; NOPCode:=$7f80;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData,SegIO];
   Grans[SegCode]:=2; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;
   Grans[SegData]:=2; ListGrans[SegData]:=2; SegInits[SegData]:=0;
   Grans[SegIO  ]:=2; ListGrans[SegIO  ]:=2; SegInits[SegIO  ]:=0;

   MakeCode:=MakeCode_3201X; ChkPC:=ChkPC_3201X; IsDef:=IsDef_3201X;
   SwitchFrom:=SwitchFrom_3201X;
END;

BEGIN
   CPU32010:=AddCPU('32010',SwitchTo_3201X);
   CPU32015:=AddCPU('32015',SwitchTo_3201X);
END.
