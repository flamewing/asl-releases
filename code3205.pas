{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

{ this code comes from Code3202x and was modified by Thomas Sailer, 3.11.95 }
{ adaptions for case-sensitivity by Alfred Arnold, 18.1.1997 }

        UNIT Code3205;

INTERFACE
        Uses NLS,Chunks,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	      END;

   JmpOrder=RECORD
	     Name:String[6];
	     Code:Word;
             Cond:Boolean;
	    END;

CONST
   FixedOrderCnt=45;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'ABS'   ; Code:$be00),
		(Name:'ADCB'  ; Code:$be11),
		(Name:'ADDB'  ; Code:$be10),
		(Name:'ANDB'  ; Code:$be12),
		(Name:'CMPL'  ; Code:$be01),
		(Name:'CRGT'  ; Code:$be1b),
		(Name:'CRLT'  ; Code:$be1c),
		(Name:'EXAR'  ; Code:$be1d),
		(Name:'LACB'  ; Code:$be1f),
		(Name:'NEG'   ; Code:$be02),
		(Name:'ORB'   ; Code:$be13),
		(Name:'ROL'   ; Code:$be0c),
		(Name:'ROLB'  ; Code:$be14),
		(Name:'ROR'   ; Code:$be0d),
		(Name:'RORB'  ; Code:$be15),
		(Name:'SACB'  ; Code:$be1e),
		(Name:'SATH'  ; Code:$be5a),
		(Name:'SATL'  ; Code:$be5b),
		(Name:'SBB'   ; Code:$be18),
		(Name:'SBBB'  ; Code:$be19),
		(Name:'SFL'   ; Code:$be09),
		(Name:'SFLB'  ; Code:$be16),
		(Name:'SFR'   ; Code:$be0a),
		(Name:'SFRB'  ; Code:$be17),
		(Name:'XORB'  ; Code:$be1a),
		(Name:'ZAP'   ; Code:$be59),
		(Name:'APAC'  ; Code:$be04),
		(Name:'PAC'   ; Code:$be03),
		(Name:'SPAC'  ; Code:$be05),
		(Name:'ZPR'   ; Code:$be58),
		(Name:'BACC'  ; Code:$be20),
		(Name:'BACCD' ; Code:$be21),
		(Name:'CALA'  ; Code:$be30),
		(Name:'CALAD' ; Code:$be3d),
		(Name:'NMI'   ; Code:$be52),
		(Name:'RET'   ; Code:$ef00),
		(Name:'RETD'  ; Code:$ff00),
		(Name:'RETE'  ; Code:$be3a),
		(Name:'RETI'  ; Code:$be38),
		(Name:'TRAP'  ; Code:$be51),
		(Name:'IDLE'  ; Code:$be22),
		(Name:'NOP'   ; Code:$8b00),
		(Name:'POP'   ; Code:$be32),
		(Name:'PUSH'  ; Code:$be3c),
		(Name:'IDLE2' ; Code:$be23));

   AdrOrderCnt=43;
   AdrOrders:ARRAY[1..AdrOrderCnt] OF FixedOrder=
	       ((Name:'ADDC'  ; Code:$6000),
	        (Name:'ADDS'  ; Code:$6200),
	        (Name:'ADDT'  ; Code:$6300),
	        (Name:'AND'   ; Code:$6e00),
	        (Name:'LACL'  ; Code:$6900),
	        (Name:'LACT'  ; Code:$6b00),
	        (Name:'OR'    ; Code:$6d00),
	        (Name:'SUBB'  ; Code:$6400),
	        (Name:'SUBC'  ; Code:$0a00),
	        (Name:'SUBS'  ; Code:$6600),
	        (Name:'SUBT'  ; Code:$6700),
	        (Name:'XOR'   ; Code:$6c00),
	        (Name:'ZALR'  ; Code:$6800),
	        (Name:'LDP'   ; Code:$0d00),
	        (Name:'APL'   ; Code:$5a00),
	        (Name:'CPL'   ; Code:$5b00),
	        (Name:'OPL'   ; Code:$5900),
	        (Name:'XPL'   ; Code:$5800),
	        (Name:'MAR'   ; Code:$8b00),
	        (Name:'LPH'   ; Code:$7500),
	        (Name:'LT'    ; Code:$7300),
	        (Name:'LTA'   ; Code:$7000),
	        (Name:'LTD'   ; Code:$7200),
	        (Name:'LTP'   ; Code:$7100),
	        (Name:'LTS'   ; Code:$7400),
	        (Name:'MADD'  ; Code:$ab00),
	        (Name:'MADS'  ; Code:$aa00),
	        (Name:'MPY'   ; Code:$5400),
	        (Name:'MPYA'  ; Code:$5000),
	        (Name:'MPYS'  ; Code:$5100),
	        (Name:'MPYU'  ; Code:$5500),
	        (Name:'SPH'   ; Code:$8d00),
	        (Name:'SPL'   ; Code:$8c00),
	        (Name:'SQRA'  ; Code:$5200),
	        (Name:'SQRS'  ; Code:$5300),
	        (Name:'BLDP'  ; Code:$5700),
	        (Name:'DMOV'  ; Code:$7700),
	        (Name:'TBLR'  ; Code:$a600),
	        (Name:'TBLW'  ; Code:$a700),
	        (Name:'BITT'  ; Code:$6f00),
	        (Name:'POPD'  ; Code:$8a00),
	        (Name:'PSHD'  ; Code:$7600),
	        (Name:'RPT'   ; Code:$0b00));


   JmpOrderCnt=10;
   JmpOrders:ARRAY[1..JmpOrderCnt] OF JmpOrder=
	       ((Name:'B'     ; Code:$7980 ; Cond:False),
	        (Name:'BD'    ; Code:$7d80 ; Cond:False),
	        (Name:'BANZ'  ; Code:$7b80 ; Cond:False),
	        (Name:'BANZD' ; Code:$7f80 ; Cond:False),
	        (Name:'BCND'  ; Code:$e000 ; Cond:True),
	        (Name:'BCNDD' ; Code:$f000 ; Cond:True),
	        (Name:'CALL'  ; Code:$7a80 ; Cond:False),
	        (Name:'CALLD' ; Code:$7e80 ; Cond:False),
	        (Name:'CC'    ; Code:$e800 ; Cond:True),
	        (Name:'CCD'   ; Code:$f800 ; Cond:True));

   PLUOrderCnt=5;
   PLUOrders:ARRAY[1..PLUOrderCnt] OF FixedOrder=
	       ((Name:'APL'  ; Code:$5e00),
	        (Name:'CPL'  ; Code:$5f00),
	        (Name:'OPL'  ; Code:$5d00),
	        (Name:'SPLK' ; Code:$ae00),
	        (Name:'XPL'  ; Code:$5c00));


VAR
   AdrMode:Word;
   AdrOK:Boolean;

   CPU32050:CPUVar;
   CPU32051:CPUVar;
   CPU32053:CPUVar;

	FUNCTION EvalARExpression(Asc:String; VAR OK:Boolean):Word;
BEGIN
   OK:=True;

   IF (Length(Asc)=3) AND (UpCase(Asc[1])='A') AND (UpCase(Asc[2])='R') AND (Asc[3]>='0') AND (Asc[3]<='7') THEN
    BEGIN
     EvalARExpression:=Ord(Asc[3])-AscOfs; Exit;
    END;
   EvalARExpression:=EvalIntExpression(Asc,UInt3,OK);
END;

	PROCEDURE DecodeAdr(Arg:String; Aux:Integer; Must1:Boolean);
VAR
   h:Byte;
   z:Integer;
   Copy:String;
BEGIN
   AdrOK:=False; Copy:=Arg; NLS_UpString(Copy);
   IF (Copy='*') OR (Copy='*-') OR (Copy='*+') OR (Copy='*BR0-') OR (Copy='*BR0+') OR
      (Copy='*AR0-') OR (Copy='*AR0+') OR (Copy='*0-') OR (Copy='*0+') THEN
    BEGIN
     IF Copy='*-' THEN AdrMode:=$90
     ELSE IF Copy='*+' THEN AdrMode:=$a0
     ELSE IF Copy='*BR0-' THEN AdrMode:=$c0
     ELSE IF (Copy='*0-') OR (Copy='*AR0-') THEN AdrMode:=$d0
     ELSE IF (Copy='*0+') OR (Copy='*AR0+') THEN AdrMode:=$e0
     ELSE IF Copy='*BR0+' THEN AdrMode:=$f0
     ELSE AdrMode:=$80;
     IF Aux<=ArgCnt THEN
      BEGIN
       h:=EvalARExpression(ArgStr[Aux],AdrOK);
       IF AdrOK THEN
	BEGIN
	 AdrMode:=AdrMode OR $8; Inc(AdrMode,h);
	END;
      END
     ELSE AdrOK:=True;
    END
   ELSE IF Aux<=ArgCnt THEN WrError(1110)
   ELSE
    BEGIN
     h:=EvalIntExpression(Arg,Int16,AdrOK);
     IF AdrOK THEN
      IF (Must1) AND (h>$80) AND (NOT FirstPassUnknown) THEN
       BEGIN
	WrError(1315); AdrOK:=False;
       END
      ELSE
       BEGIN
	AdrMode:=h AND $7f; ChkSpace(SegData);
       END;
    END;
END;

        FUNCTION DecodeCond(ArgP:Integer):Word;
VAR
   ret:Word;
   cnttp,cntzl,cntv,cntc:Byte;
BEGIN
   cnttp:=0;
   cntzl:=0;
   cntv:=0;
   cntc:=0;
   ret:=$300;
   NLS_UpString(ArgStr[ArgP]);
   WHILE ArgP<=ArgCnt DO
    BEGIN
     IF ArgStr[ArgP] = 'EQ' THEN
      BEGIN
       Inc(cntzl);
       ret:=ret or $088;
      END ELSE IF ArgStr[ArgP] = 'NEQ' THEN
      BEGIN
       Inc(cntzl);
       ret:=ret or $008;
      END ELSE IF ArgStr[ArgP] = 'LT' THEN
      BEGIN
       Inc(cntzl);
       ret:=ret or $044;
      END ELSE IF ArgStr[ArgP] = 'LEQ' THEN
      BEGIN
       Inc(cntzl);
       ret:=ret or $0cc;
      END ELSE IF ArgStr[ArgP] = 'GT' THEN
      BEGIN
       Inc(cntzl);
       ret:=ret or $004;
      END ELSE IF ArgStr[ArgP] = 'GEQ' THEN
      BEGIN
       Inc(cntzl);
       ret:=ret or $08c;
      END ELSE IF ArgStr[ArgP] = 'NC' THEN
      BEGIN
       Inc(cntc);
       ret:=ret or $001;
      END ELSE IF ArgStr[ArgP] = 'C' THEN
      BEGIN
       Inc(cntc);
       ret:=ret or $011;
      END ELSE IF ArgStr[ArgP] = 'NOV' THEN
      BEGIN
       Inc(cntv);
       ret:=ret or $002;
      END ELSE IF ArgStr[ArgP] = 'OV' THEN
      BEGIN
       Inc(cntv);
       ret:=ret or $022;
      END ELSE IF ArgStr[ArgP] = 'BIO' THEN
      BEGIN
       Inc(cnttp);
       ret:=ret and $0ff;
      END ELSE IF ArgStr[ArgP] = 'NTC' THEN
      BEGIN
       Inc(cnttp);
       ret:=ret and $2ff;
      END ELSE IF ArgStr[ArgP] = 'TC' THEN
      BEGIN
       Inc(cnttp);
       ret:=ret and $1ff;
      END ELSE IF ArgStr[ArgP] = 'UNC' THEN
      BEGIN
       Inc(cnttp);
       ret:=ret or $300;
      END ELSE WrError(1360);
     Inc(ArgP);
    END;
   IF (cnttp>1) OR (cntzl>1) OR (cntv>1) OR (cntc>1) THEN WrError(1); { invalid condition }
   DecodeCond:=ret;
END;


        PROCEDURE PseudoQXX(num:Integer);
TYPE
  TExtFmt=RECORD
            M0,M1,M2,M3:Word;
            Exp:Integer;
          END;
VAR
   z:Integer;
   OK:Boolean;
   res:Extended;
   mul:Extended;
   mul2:TExtFmt ABSOLUTE mul;
BEGIN
   IF ArgCnt=0 THEN
    BEGIN
     WrError(1110);
     Exit;
    END;
   mul:=1.0;
   Inc(mul2.Exp,num);
   FOR z:=1 TO ArgCnt DO
    BEGIN
     res:=EvalFloatExpression(ArgStr[z],Float80,OK)*mul;
     IF NOT OK THEN
      BEGIN
       CodeLen:=0;
       Exit;
      END;
     IF (res > 32767.49) OR (res < -32768.49) THEN
      BEGIN
       CodeLen:=0;
       WrError(1320);
       Exit;
      END;
     WAsmCode[CodeLen]:=Integer(Round(res));
     Inc(CodeLen);
    END;
END;

        PROCEDURE PseudoLQXX(num:Integer);
TYPE
  TExtFmt=RECORD
            M0,M1,M2,M3:Word;
            Exp:Integer;
          END;
VAR
   z:Integer;
   OK:Boolean;
   res:Extended;
   resli:LongInt;
   mul:Extended;
   mul2:TExtFmt ABSOLUTE mul;
BEGIN
   IF ArgCnt=0 THEN
    BEGIN
     WrError(1110);
     Exit;
    END;
   mul:=1.0;
   Inc(mul2.Exp,num);
   FOR z:=1 TO ArgCnt DO
    BEGIN
     res:=EvalFloatExpression(ArgStr[z],Float80,OK)*mul;
     IF NOT OK THEN
      BEGIN
       CodeLen:=0;
       Exit;
      END;
     IF (res > 2147483647.49) OR (res < -2147483647.49) THEN
      BEGIN
       CodeLen:=0;
       WrError(1320);
       Exit;
      END;
     resli:=Round(res);
     WAsmCode[CodeLen]:=(resli and $ffff);
     Inc(CodeLen);
     WAsmCode[CodeLen]:=(resli SHR 16);
     Inc(CodeLen);
    END;
END;

	PROCEDURE DefineUntypedLabel;
BEGIN
   IF LabPart <> '' THEN
    BEGIN
     PushLocHandle(-1);
     EnterIntSymbol(LabPart,EProgCounter,SegNone,False);
     PopLocHandle;
    END;
END;

TYPE
  TWrCode=PROCEDURE(VAR OK:Boolean;VAR Adr:Integer;Val:LongInt);

        PROCEDURE WrCodeByte(VAR OK:Boolean;VAR Adr:Integer;Val:LongInt);
        Far;
BEGIN
   IF (Val<-128) OR (Val>$ff) THEN
    BEGIN
     WrError(1320); OK:=False;
    END
   ELSE
    BEGIN
     WAsmCode[Adr]:=Val AND $ff;
     Inc(Adr);
     CodeLen:=Adr;
    END;
END;

        PROCEDURE WrCodeWord(VAR OK:Boolean;VAR Adr:Integer;Val:LongInt);
        Far;
BEGIN
   IF (Val<-32768) OR (Val>$ffff) THEN
    BEGIN
     WrError(1320); OK:=False;
    END
   ELSE
    BEGIN
     WAsmCode[Adr]:=Val;
     Inc(Adr);
     CodeLen:=Adr;
    END;
END;

        PROCEDURE WrCodeLong(VAR OK:Boolean;VAR Adr:Integer;Val:LongInt);
        Far;
BEGIN
   WAsmCode[Adr]:=Val AND $ffff;
   Inc(Adr);
   WAsmCode[Adr]:=Val SHR 16;
   Inc(Adr);
   CodeLen:=Adr;
END;

        PROCEDURE WrCodeByteHiLo(VAR OK:Boolean;VAR Adr:Integer;Val:LongInt);
        Far;
BEGIN
   IF (Val<-128) OR (Val>$ff) THEN
    BEGIN
     WrError(1320); OK:=False;
    END
   ELSE
    BEGIN
     IF ODD(Adr) THEN
      Inc(WAsmCode[Adr DIV 2],Val AND $ff)
     ELSE
      WAsmCode[Adr DIV 2]:=Val SHL 8;
     Inc(Adr);
     CodeLen:=(Adr+1) DIV 2;
    END;
END;

        PROCEDURE WrCodeByteLoHi(VAR OK:Boolean;VAR Adr:Integer;Val:LongInt);
Far;
BEGIN
   IF (Val<-128) OR (Val>$ff) THEN
    BEGIN
     WrError(1320); OK:=False;
    END
   ELSE
    BEGIN
     IF ODD(Adr) THEN
      Inc(WAsmCode[Adr DIV 2],Val SHL 8)
     ELSE
      WAsmCode[Adr DIV 2]:=Val AND $ff;
     Inc(Adr);
     CodeLen:=(Adr+1) DIV 2;
    END;
END;

        PROCEDURE PseudoStore(W_Callback:TWrCode);
VAR
   OK:Boolean;
   Adr:Integer;
   z,z2:Integer;
   t:TempResult;
BEGIN
   IF ArgCnt=0 THEN WrError(1110)
   ELSE
    BEGIN
     DefineUntypedLabel;
     OK:=True;
     Adr:=0;
     FOR z:=1 TO ArgCnt DO
      IF OK THEN
       BEGIN
	EvalExpression(ArgStr[z],t);
	WITH t DO
	 CASE Typ OF
	  TempInt:   W_Callback(OK,Adr,Int);
	  TempFloat: BEGIN
		      WrError(1135); OK:=False;
		     END;
	  TempString:BEGIN
		      FOR z2:=1 TO Length(Ascii) DO
		       BEGIN
                        W_Callback(OK,Adr,Ord(CharTransTable[Ascii[z2]]));
		       END;
		     END;
	  ELSE OK:=False;
         END;
       END;
    END;
END;


	FUNCTION DecodePseudo:Boolean;
TYPE
  TExtFmt=RECORD
            M:Comp;
            Exp:Integer;
          END;
VAR
   AdrByte:Byte;
   Size:Word;
   z,z2,z3:Integer;
   t:TempResult;
   OK:Boolean;
   ext:Extended;
   ext2:TExtFmt ABSOLUTE ext;
   dbl:Double;
   sngl:Single;
   li:LongInt;
   cmp:Comp;
   cmp2:Array[0..3] of Word ABSOLUTE cmp;
BEGIN
   DecodePseudo:=True;

   IF Memo('PORT') THEN
    BEGIN
     CodeEquate(SegIO,0,65535);
     Exit;
    END;

   IF Memo('RES') OR Memo('BSS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('BSS') THEN DefineUntypedLabel;
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

   IF Memo('STRING') THEN
    BEGIN
     PseudoStore(WrCodeByteHiLo); Exit;
    END;
   IF Memo('RSTRING') THEN
    BEGIN
     PseudoStore(WrCodeByteLoHi); Exit;
    END;
   IF Memo('BYTE') THEN
    BEGIN
     PseudoStore(WrCodeByte); Exit;
    END;
   IF Memo('WORD') THEN
    BEGIN
     PseudoStore(WrCodeWord); Exit;
    END;
   IF Memo('LONG') THEN
    BEGIN
     PseudoStore(WrCodeLong); Exit;
    END;

   { Qxx }

   IF (Length(OpPart)=3) AND (OpPart[1]='Q') AND (OpPart[2]>='0') AND
      (OpPart[2]<='9') AND (OpPart[3]>='0') AND (OpPart[3]<='9') THEN
    BEGIN
     PseudoQXX(10*(ORD(OpPart[2])-ORD('0'))+ORD(OpPart[3])-ORD('0'));
     EXIT;
    END;

   { LQxx }

   IF (Length(OpPart)=4) AND (OpPart[1]='L') AND (OpPart[2]='Q') AND (OpPart[3]>='0') AND
      (OpPart[3]<='9') AND (OpPart[4]>='0') AND (OpPart[4]<='9') THEN
    BEGIN
     PseudoLQXX(10*(ORD(OpPart[3])-ORD('0'))+ORD(OpPart[4])-ORD('0'));
     EXIT;
    END;

   { Floating point definitions }

   IF Memo('FLOAT') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       DefineUntypedLabel;
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  sngl:=EvalFloatExpression(ArgStr[z],Float32,OK);
	  Move(sngl,WAsmCode[CodeLen],4);
	  Inc(CodeLen,2);
	 END;
       IF NOT OK THEN CodeLen:=0;
      END;
     EXIT;
    END;

   IF Memo('DOUBLE') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       DefineUntypedLabel;
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  dbl:=EvalFloatExpression(ArgStr[z],Float64,OK);
	  Move(dbl,WAsmCode[CodeLen],8);
	  Inc(CodeLen,4);
	 END;
       OK:=False;
      END;
     EXIT;
    END;

   IF Memo('EFLOAT') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       DefineUntypedLabel;
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
          ext:=EvalFloatExpression(ArgStr[z],Float80,OK);
          cmp:=ext2.M/2;
          cmp2[3]:=cmp2[3] AND $7fff;
          IF (ext2.Exp AND $8000) <> 0 THEN cmp:=-cmp;
          WAsmCode[CodeLen]:=cmp2[3];
          Inc(CodeLen);
          WAsmCode[CodeLen]:=(ext2.Exp AND $7fff)-$3fff;
          Inc(CodeLen);
         END;
       OK:=False;
      END;
     EXIT;
    END;

   IF Memo('BFLOAT') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       DefineUntypedLabel;
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  ext:=EvalFloatExpression(ArgStr[z],Float80,OK);
	  cmp:=ext2.M/2;
	  cmp2[3]:=cmp2[3] AND $7fff;
	  IF (ext2.Exp AND $8000) <> 0 THEN cmp:=-cmp;
	  WAsmCode[CodeLen]:=cmp2[2];
	  Inc(CodeLen);
	  WAsmCode[CodeLen]:=cmp2[3];
	  Inc(CodeLen);
	  WAsmCode[CodeLen]:=(ext2.Exp AND $7fff)-$3fff;
	  Inc(CodeLen);
	 END;
       OK:=False;
      END;
     EXIT;
    END;

   IF Memo('TFLOAT') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       DefineUntypedLabel;
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
          ext:=EvalFloatExpression(ArgStr[z],Float80,OK);
          li:=(ext2.Exp AND $7fff)-$3fff;
          cmp:=ext2.M/2;
          cmp2[3]:=cmp2[3] AND $7fff;
          IF (ext2.Exp AND $8000) <> 0 THEN cmp:=-cmp;
          WAsmCode[CodeLen]:=cmp2[0];
          Inc(CodeLen);
          WAsmCode[CodeLen]:=cmp2[1];
          Inc(CodeLen);
          WAsmCode[CodeLen]:=cmp2[2];
          Inc(CodeLen);
          WAsmCode[CodeLen]:=cmp2[3];
          Inc(CodeLen);
          WAsmCode[CodeLen]:=li;
          Inc(CodeLen);
          WAsmCode[CodeLen]:=li SHR 16;
          Inc(CodeLen);
         END;
       IF NOT OK THEN CodeLen:=0;
      END;
     EXIT;
    END;


   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_3205X;
	Far;
VAR
   OK:Boolean;
   AdrWord:Word;
   AdrLong:LongInt;
   z:Integer;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { prozessorspezifische Befehle }

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

   { Immediate }

   IF Memo('MPY') AND (ArgCnt = 1) AND (Copy(ArgStr[1],1,1)='#') THEN
    BEGIN
     AdrLong:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK);
     IF OK THEN
      BEGIN
       IF FirstPassUnknown OR (AdrLong<-4096) OR (AdrLong>4095) THEN
        BEGIN
         CodeLen:=2;
         WAsmCode[0]:=$be80;
         WAsmCode[1]:=AdrLong;
        END ELSE BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$c000 or (AdrLong and $1fff);
        END;
      END;
     Exit;
    END;

   FOR z:=1 TO PLUOrderCnt DO
    WITH PLUOrders[z] DO
     IF Memo(Name) AND (ArgCnt>=2) AND (ArgCnt<=3) AND (Copy(ArgStr[1],1,1)='#') THEN
      BEGIN
       DecodeAdr(ArgStr[2],3,FALSE);
       WAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK);
       IF OK and AdrOK THEN
        BEGIN
	 CodeLen:=2;
         WAsmCode[0]:=Code+AdrMode;
        END;
       Exit;
      END;

   IF Memo('LDP') AND (ArgCnt = 1) AND (Copy(ArgStr[1],1,1)='#') THEN
    BEGIN
     AdrWord:=EvalIntExpression(Copy(ArgStr[1],2,255),UInt16,OK);
     IF (AdrWord >= $200) AND NOT FirstPassUnknown THEN WrError(1); { out of range }
     IF OK THEN
      BEGIN
       CodeLen:=1;
       WAsmCode[0]:=(AdrWord AND $1ff) or $bc00;
      END;
     Exit;
    END;

   IF (Memo('AND') OR Memo('OR') OR Memo('XOR')) AND (ArgCnt>=1) AND (ArgCnt<=2) AND (Copy(ArgStr[1],1,1)='#') THEN
    BEGIN
     WAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK);
     AdrWord:=0;
     AdrOK:=True;
     IF ArgCnt>=2 THEN AdrWord:=EvalIntExpression(ArgStr[2],UInt5,AdrOK);
     IF OK AND AdrOK THEN
      BEGIN
       IF (AdrWord>16) AND NOT FirstPassUnknown THEN WrError(1); { invalid shift }
       IF Memo('AND') THEN
        BEGIN
         IF AdrWord>=16 THEN WAsmCode[0]:=$be81 ELSE WAsmCode[0]:=$bfb0 or (AdrWord and $f);
        END ELSE IF Memo('OR') THEN BEGIN
         IF AdrWord>=16 THEN WAsmCode[0]:=$be82 ELSE WAsmCode[0]:=$bfc0 or (AdrWord and $f);
        END ELSE BEGIN
         IF AdrWord>=16 THEN WAsmCode[0]:=$be83 ELSE WAsmCode[0]:=$bfd0 or (AdrWord and $f);
        END;
       CodeLen:=2;
      END;
     Exit;
    END;

   IF Memo('LACL') AND (ArgCnt = 1) AND (Copy(ArgStr[1],1,1)='#') THEN
    BEGIN
     AdrWord:=EvalIntExpression(Copy(ArgStr[1],2,255),UInt8,OK);
     IF OK THEN
      BEGIN
       CodeLen:=1;
       WAsmCode[0]:=(AdrWord AND $ff) or $b900;
      END;
     Exit;
    END;

   IF Memo('RPT') AND (ArgCnt = 1) AND (Copy(ArgStr[1],1,1)='#') THEN
    BEGIN
     AdrLong:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK) and $ffff;
     IF OK THEN
      BEGIN
       IF FirstPassUnknown OR (AdrLong>255) THEN
        BEGIN
         CodeLen:=2;
         WAsmCode[0]:=$bec4;
         WAsmCode[1]:=AdrLong;
        END ELSE BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$bb00 or (AdrLong and $ff);
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
	 DecodeAdr(ArgStr[1],2,FALSE);
	 IF AdrOK THEN
	  BEGIN
	   CodeLen:=1; WAsmCode[0]:=Code+AdrMode;
	  END;
	END;
       Exit;
      END;

   { Adresse, spezial }

   IF Memo('LAMM') OR Memo('SAMM') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],2,TRUE);
       IF AdrOK THEN
        BEGIN
	 CodeLen:=1; WAsmCode[0]:=$0800+AdrMode;
         IF Memo('SAMM') THEN WAsmCode[0]:=WAsmCode[0] or $8000;
	END;
      END;
     Exit;
    END;

   IF Memo('LST') OR Memo('SST') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE IF Copy(ArgStr[1],1,1)<>'#' THEN WrError(1) { invalid instruction }
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(Copy(ArgStr[1],2,255),UInt1,OK);
       DecodeAdr(ArgStr[2],3,Memo('SST'));
       IF OK AND AdrOK THEN
        BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$0e00+((AdrWord and 1) shl 8)+AdrMode;
         IF Memo('SST') THEN WAsmCode[0]:=WAsmCode[0] or $8000;
        END;
      END;
     Exit;
    END;

   { Spezial: Set/Reset Mode Flags }
   IF Memo('CLRC') OR Memo('SETC') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=1; NLS_UpString(ArgStr[1]);
       WAsmCode[0]:=0;
       IF ArgStr[1] = 'OVM' THEN WAsmCode[0]:=$be42
       ELSE IF ArgStr[1] = 'SXM' THEN WAsmCode[0]:=$be46
       ELSE IF ArgStr[1] = 'HM' THEN WAsmCode[0]:=$be48
       ELSE IF ArgStr[1] = 'TC' THEN WAsmCode[0]:=$be4a
       ELSE IF ArgStr[1] = 'C' THEN WAsmCode[0]:=$be4e
       ELSE IF ArgStr[1] = 'XF' THEN WAsmCode[0]:=$be4c
       ELSE IF ArgStr[1] = 'CNF' THEN WAsmCode[0]:=$be44
       ELSE IF ArgStr[1] = 'INTM' THEN WAsmCode[0]:=$be40;
       IF WAsmCode[0] = 0 THEN WrError(1); { invalid instruction }
       IF Memo('SETC') THEN WAsmCode[0]:=WAsmCode[0] or 1;
      END;
     Exit;
    END;

   { Spruenge }

   FOR z:=1 TO JmpOrderCnt DO
    WITH JmpOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR ((ArgCnt>3) AND NOT Cond) THEN WrError(1110)
       ELSE
	BEGIN
         AdrMode := 0;
         IF Cond THEN
          BEGIN
           AdrMode := DecodeCond(2);
          END ELSE BEGIN
           IF ArgCnt>1 THEN
            BEGIN
 	     DecodeAdr(ArgStr[2],3,False);
             IF AdrMode<$80 THEN WrError(1350);
             AdrMode := AdrMode AND $7f;
            END;
          END;
	 WAsmCode[1]:=EvalIntExpression(ArgStr[1],Int16,OK);
	 IF OK THEN
          BEGIN
	   CodeLen:=2; WAsmCode[0]:=Code+AdrMode;
	  END;
	END;
       Exit;
      END;

   IF Memo('XC') THEN
    BEGIN
     IF ArgCnt<1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrMode:=EvalIntExpression(ArgStr[1],UInt2,OK);
       IF (AdrMode<>1) AND (AdrMode<>2) AND NOT FirstPassUnknown THEN WrError(1315);
       IF OK THEN
        BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$e400 or DecodeCond(2);
         IF AdrMode=2 THEN WAsmCode[0]:=WAsmCode[0] or $1000;
        END;
      END;
     Exit;
    END;

   IF Memo('RETC') THEN
    BEGIN
     CodeLen:=1;
     WAsmCode[0]:=$ec00 or DecodeCond(1);
     Exit;
    END;

   IF Memo('RETCD') THEN
    BEGIN
     CodeLen:=1;
     WAsmCode[0]:=$fc00 or DecodeCond(1);
     Exit;
    END;

   IF Memo('INTR') THEN
    BEGIN
     IF (ArgCnt<>1) THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt5,OK);
       IF OK THEN
        BEGIN
         WAsmCode[0]:=(WAsmCode[0] and $1f) or $be60;
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   { IO und Datenmemorybefehle }

   IF Memo('BLDD') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[1] , 'BMAR')=0 THEN
        BEGIN
         DecodeAdr(ArgStr[2],3,False);
         IF AdrOK THEN
          BEGIN
           CodeLen:=1;
           WAsmCode[0]:=$ac00+AdrMode;
          END;
         Exit;
        END;
       IF NLS_StrCaseCmp(ArgStr[2] , 'BMAR')=0 THEN
        BEGIN
         DecodeAdr(ArgStr[1],3,False);
         IF AdrOK THEN
          BEGIN
           CodeLen:=1;
           WAsmCode[0]:=$ad00+AdrMode;
          END;
         Exit;
        END;
       IF Copy(ArgStr[1],1,1)='#' THEN
        BEGIN
         WAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK);
         DecodeAdr(ArgStr[2],3,False);
         IF AdrOK AND OK THEN
          BEGIN
           CodeLen:=2;
           WAsmCode[0]:=$a800+AdrMode;
          END;
         Exit;
        END;
       IF Copy(ArgStr[2],1,1)='#' THEN
        BEGIN
         WAsmCode[1]:=EvalIntExpression(Copy(ArgStr[2],2,255),Int16,OK);
         DecodeAdr(ArgStr[1],3,False);
         IF AdrOK AND OK THEN
          BEGIN
           CodeLen:=2;
           WAsmCode[0]:=$a900+AdrMode;
          END;
         Exit;
        END;
       WrError(1); { invalid instruction }
      END;
     Exit;
    END;

   IF Memo('BLPD') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[1] , 'BMAR')=0 THEN
        BEGIN
         DecodeAdr(ArgStr[2],3,False);
         IF AdrOK THEN
          BEGIN
           CodeLen:=1;
           WAsmCode[0]:=$a400+AdrMode;
          END;
         Exit;
        END;
       IF Copy(ArgStr[1],1,1)='#' THEN
        BEGIN
         WAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK);
         DecodeAdr(ArgStr[2],3,False);
         IF AdrOK AND OK THEN
          BEGIN
           CodeLen:=2;
           WAsmCode[0]:=$a500+AdrMode;
          END;
         Exit;
        END;
       WrError(1); { invalid instruction }
      END;
     Exit;
    END;

   IF Memo('LMMR') OR Memo('SMMR') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE IF (Copy(ArgStr[2],1,1)<>'#') THEN WrError(1) {invalid parameter}
     ELSE
      BEGIN
       WAsmCode[1]:=EvalIntExpression(Copy(ArgStr[2],2,255),Int16,OK);
       DecodeAdr(ArgStr[1],3,True);
       IF AdrOK AND OK THEN
        BEGIN
         CodeLen:=2;
         WAsmCode[0]:=$0900+AdrMode;
         IF Memo('LMMR') THEN WAsmCode[0]:=WAsmCode[0] OR $8000;
        END;
      END;
     Exit;
    END;

   IF (Memo('IN')) OR (Memo('OUT')) THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],3,False);
       IF AdrOK THEN
	BEGIN
	 WAsmCode[1]:=EvalIntExpression(ArgStr[2],Int16,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegIO);
	   CodeLen:=2;
	   WAsmCode[0]:=$0c00+AdrMode;
	   IF Memo('IN') THEN WAsmCode[0]:=WAsmCode[0] or $a300;
	  END;
	END;
      END;
     Exit;
    END;

   { spezialbefehle }

   IF Memo('CMPR') OR Memo('SPM') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt2,OK);
       IF OK THEN
        BEGIN
         CodeLen:=1;
         WAsmCode[0]:=(WAsmCode[0] and 3) or $bf00;
         IF Memo('CMPR') THEN WAsmCode[0]:=WAsmCode[0] OR $0044;
        END;
      END;
     Exit;
    END;

   IF Memo('MAC') OR Memo('MACD') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[1]:=EvalIntExpression(ArgStr[1],Int16,OK);
       DecodeAdr(ArgStr[2],3,False);
       IF AdrOK AND OK THEN
        BEGIN
         CodeLen:=2;
         WAsmCode[0]:=$a200+AdrMode;
         IF Memo('MACD') THEN WAsmCode[0]:=WAsmCode[0] OR $0100;
        END;
      END;
     Exit;
    END;

   IF Memo('SACL') OR Memo('SACH') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True;
       AdrWord:=0;
       IF ArgCnt>=2 THEN AdrWord:=EvalIntExpression(ArgStr[2],UInt3,OK);
       DecodeAdr(ArgStr[1],3,False);
       IF AdrOK AND OK THEN
        BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$9000+AdrMode+((AdrWord and 7) shl 8);
         IF Memo('SACH') THEN WAsmCode[0]:=WAsmCode[0] OR $0800;
        END;
      END;
     Exit;
    END;

   { Auxregisterbefehle }

   IF Memo('ADRK') OR Memo('SBRK') THEN
    BEGIN
     IF (ArgCnt<>1) THEN WrError(1110)
     ELSE IF (Copy(ArgStr[1],1,1)<>'#') THEN WrError(1) {invalid parameter}
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(Copy(ArgStr[1],2,255),UInt8,OK);
       IF OK THEN
        BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$7800 or (AdrWord and $ff);
         IF Memo('SBRK') THEN WAsmCode[0]:=WAsmCode[0] OR $0400;
        END;
      END;
     Exit;
    END;

   IF Memo('SAR') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalARExpression(ArgStr[1],OK);
       DecodeAdr(ArgStr[2],3,False);
       IF OK AND AdrOK THEN
        BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$8000 or ((AdrWord and 7) shl 8) or AdrMode;
        END;
      END;
     Exit;
    END;

   IF Memo('LAR') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalARExpression(ArgStr[1],OK);
       IF Copy(ArgStr[2],1,1)='#' THEN
        BEGIN
         IF (ArgCnt>2) THEN WrError(1110);
         AdrLong:=EvalIntExpression(Copy(ArgStr[2],2,255),Int16,AdrOK) and $ffff;
         IF AdrOK AND OK THEN
          BEGIN
           IF FirstPassUnknown OR (AdrLong>255) THEN
            BEGIN
             CodeLen:=2;
             WAsmCode[0]:=$bf08 or (AdrWord and 7);
             WAsmCode[1]:=AdrLong;
            END ELSE BEGIN
             CodeLen:=1;
             WAsmCode[0]:=$b000 or ((AdrWord and 7) shl 8) or (AdrLong and $ff);
            END;
          END;
        END ELSE BEGIN
         DecodeAdr(ArgStr[2],3,False);
         IF OK AND AdrOK THEN
          BEGIN
           CodeLen:=1;
           WAsmCode[0]:=$0000 or ((AdrWord and 7) shl 8) or AdrMode;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('NORM') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],2,False);
       IF AdrOK THEN
	BEGIN
         IF AdrMode < $80 THEN WrError(1350)
         ELSE
	  BEGIN
	   CodeLen:=1;
	   WAsmCode[0]:=$a080+(AdrMode and $7f);
	  END;
	END;
      END;
     Exit;
    END;

   { add/sub }

   IF Memo('ADD') OR Memo('SUB') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       IF Copy(ArgStr[1],1,1)='#' THEN
        BEGIN
         IF ArgCnt>2 THEN WrError(1110);
         AdrLong:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK);
         AdrWord:=0;
         AdrOK:=True;
         IF (ArgCnt>1) OR FirstPassUnknown OR (AdrLong<0) OR (AdrLong>255) THEN
          BEGIN
           IF ArgCnt>1 THEN AdrWord:=EvalIntExpression(ArgStr[2],UInt4,AdrOK);
           IF AdrOK AND OK THEN
            BEGIN
             CodeLen:=2;
             WAsmCode[0]:=$bf90 or (AdrWord and $f);
             IF Memo('SUB') THEN Inc(WAsmCode[0],$10);
             WAsmCode[1]:=AdrLong;
            END;
          END ELSE BEGIN
           IF OK THEN
            BEGIN
             CodeLen:=1;
             WAsmCode[0]:=$b800 or (AdrLong and $ff);
             IF Memo('SUB') THEN Inc(WAsmCode[0],$0200);
            END;
          END;
        END ELSE BEGIN
         AdrWord:=0;
         OK:=True;
         DecodeAdr(ArgStr[1],3,False);
         IF ArgCnt>=2 THEN AdrWord:=EvalIntExpression(ArgStr[2],UInt5,OK);
         IF OK THEN
          BEGIN
           IF (AdrWord>16) AND NOT FirstPassUnknown THEN WrError(1); { shift out of range }
           CodeLen:=1;
           IF AdrWord>=16 THEN
            BEGIN
             WAsmCode[0]:=$6100 or AdrMode;
             IF Memo('SUB') THEN Inc(WAsmCode[0],$0400);
            END ELSE BEGIN
             WAsmCode[0]:=$2000 or ((AdrWord and $f) shl 8) or AdrMode;
             IF Memo('SUB') THEN Inc(WAsmCode[0],$1000);
            END;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('LACC') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       IF Copy(ArgStr[1],1,1)='#' THEN
        BEGIN
         IF ArgCnt>2 THEN WrError(1110);
         AdrLong:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK);
         AdrWord:=0;
         AdrOK:=True;
         IF ArgCnt>1 THEN AdrWord:=EvalIntExpression(ArgStr[2],UInt4,AdrOK);
         IF AdrOK AND OK THEN
          BEGIN
           CodeLen:=2;
           WAsmCode[0]:=$bf80 or (AdrWord and $f);
           WAsmCode[1]:=AdrLong;
          END;
        END ELSE BEGIN
         AdrWord:=0;
         OK:=True;
         DecodeAdr(ArgStr[1],3,False);
         IF ArgCnt>=2 THEN AdrWord:=EvalIntExpression(ArgStr[2],UInt5,OK);
         IF OK THEN
          BEGIN
           IF (AdrWord>16) AND NOT FirstPassUnknown THEN WrError(1); { shift out of range }
           CodeLen:=1;
           IF AdrWord>=16 THEN
             WAsmCode[0]:=$6a00 or AdrMode
            ELSE
             WAsmCode[0]:=$1000 or ((AdrWord and $f) shl 8) or AdrMode;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('BSAR') THEN
    BEGIN
     IF (ArgCnt<>1) THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],UInt5,OK);
       IF (AdrWord<1) THEN WrError(1315)
       ELSE IF (AdrWord>16) THEN WrError(1320)
       ELSE
        IF OK THEN
         BEGIN
          CodeLen:=1;
          WAsmCode[0]:=$bfe0 or ((AdrWord-1) and $f);
         END;
      END;
     Exit;
    END;

   IF Memo('BIT') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[2],UInt4,OK);
       DecodeAdr(ArgStr[1],3,False);
       IF AdrOK AND OK THEN
        BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$4000+AdrMode+((AdrWord and $f) shl 8);
        END;
      END;
     Exit;
    END;

   { repeat commands }

   IF Memo('RPTB') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[1]:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
        BEGIN
         CodeLen:=2;
         WAsmCode[0]:=$bec6;
        END;
      END;
     Exit;
    END;

   IF Memo('RPTZ') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF Copy(ArgStr[1],1,1)<>'#' THEN WrError(1) { not const }
     ELSE
      BEGIN
       WAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,255),Int16,OK);
       IF OK THEN
        BEGIN
         CodeLen:=2;
         WAsmCode[0]:=$bec5;
        END;
      END;
     Exit;
    END;


   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_3205X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=$ffff;
   SegData  : ok:=ProgCounter <=$ffff;
   SegIO    : ok:=ProgCounter <=$ffff;
   ELSE ok:=False;
   END;
   ChkPC_3205X:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_3205X:Boolean;
	Far;
BEGIN
   IsDef_3205X:=Memo('BSS') OR Memo('PORT') OR Memo('STRING') OR Memo('RSTRING') OR
                Memo('BYTE') OR Memo('WORD') OR Memo('LONG') OR Memo('FLOAT') OR
                Memo('DOUBLE') OR Memo('EFLOAT') OR Memo('BFLOAT') OR Memo('TFLOAT');
END;

        PROCEDURE SwitchFrom_3205X;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_3205X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$77; NOPCode:=$8b00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData,SegIO];
   Grans[SegCode]:=2; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;
   Grans[SegData]:=2; ListGrans[SegData]:=2; SegInits[SegData]:=0;
   Grans[SegIO  ]:=2; ListGrans[SegIO  ]:=2; SegInits[SegIO  ]:=0;

   MakeCode:=MakeCode_3205X; ChkPC:=ChkPC_3205X; IsDef:=IsDef_3205X;
   SwitchFrom:=SwitchFrom_3205X;
END;

BEGIN
   CPU32050:=AddCPU('320C50',SwitchTo_3205X);
   CPU32051:=AddCPU('320C51',SwitchTo_3205X);
   CPU32053:=AddCPU('320C53',SwitchTo_3205X);

   AddCopyright('TMS320C5x-Generator (C) 1995 Thomas Sailer');
END.


