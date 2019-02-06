{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

{ this code comes from Code3201x and was modified by Thomas Sailer, 27.2.94 }
{ enhancements to deal with case-sensitivity by Alfred Arnold, 18.1.97 }

        UNIT Code3202;

INTERFACE
        Uses Chunks,NLS,
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
   FixedOrderCnt=37;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'ABS'   ; Code:$ce1b),
		(Name:'CMPL'  ; Code:$ce27),
		(Name:'NEG'   ; Code:$ce23),
		(Name:'ROL'   ; Code:$ce34),
		(Name:'ROR'   ; Code:$ce35),
		(Name:'SFL'   ; Code:$ce18),
		(Name:'SFR'   ; Code:$ce19),
		(Name:'ZAC'   ; Code:$ca00),
		(Name:'APAC'  ; Code:$ce15),
		(Name:'PAC'   ; Code:$ce14),
		(Name:'SPAC'  ; Code:$ce16),
		(Name:'BACC'  ; Code:$ce25),
		(Name:'CALA'  ; Code:$ce24),
		(Name:'RET'   ; Code:$ce26),
		(Name:'RFSM'  ; Code:$ce36),
		(Name:'RTXM'  ; Code:$ce20),
		(Name:'RXF'   ; Code:$ce0c),
		(Name:'SFSM'  ; Code:$ce37),
		(Name:'STXM'  ; Code:$ce21),
		(Name:'SXF'   ; Code:$ce0d),
		(Name:'DINT'  ; Code:$ce01),
		(Name:'EINT'  ; Code:$ce00),
		(Name:'IDLE'  ; Code:$ce1f),
		(Name:'NOP'   ; Code:$5500),
		(Name:'POP'   ; Code:$ce1d),
		(Name:'PUSH'  ; Code:$ce1c),
		(Name:'RC'    ; Code:$ce30),
		(Name:'RHM'   ; Code:$ce38),
		(Name:'ROVM'  ; Code:$ce02),
		(Name:'RSXM'  ; Code:$ce06),
		(Name:'RTC'   ; Code:$ce32),
		(Name:'SC'    ; Code:$ce31),
		(Name:'SHM'   ; Code:$ce39),
		(Name:'SOVM'  ; Code:$ce03),
		(Name:'SSXM'  ; Code:$ce07),
		(Name:'STC'   ; Code:$ce33),
		(Name:'TRAP'  ; Code:$ce1e));


   JmpOrderCnt=16;
   JmpOrders:ARRAY[1..JmpOrderCnt] OF FixedOrder=
	       ((Name:'B'     ; Code:$ff80),
	        (Name:'BANZ'  ; Code:$fb80),
	        (Name:'BBNZ'  ; Code:$f980),
	        (Name:'BBZ'   ; Code:$f880),
	        (Name:'BC'    ; Code:$5e80),
	        (Name:'BGEZ'  ; Code:$f480),
	        (Name:'BGZ'   ; Code:$f180),
	        (Name:'BIOZ'  ; Code:$fa80),
	        (Name:'BLEZ'  ; Code:$f280),
	        (Name:'BLZ'   ; Code:$f380),
	        (Name:'BNC'   ; Code:$5f80),
	        (Name:'BNV'   ; Code:$f780),
	        (Name:'BNZ'   ; Code:$f580),
	        (Name:'BV'    ; Code:$f080),
	        (Name:'BZ'    ; Code:$f680),
	        (Name:'CALL'  ; Code:$fe80));



   AdrOrderCnt=43;
   AdrOrders:ARRAY[1..AdrOrderCnt] OF AdrOrder=
	       ((Name:'ADDC'  ; Code:$4300; Must1:False),
		(Name:'ADDH'  ; Code:$4800; Must1:False),
		(Name:'ADDS'  ; Code:$4900; Must1:False),
		(Name:'ADDT'  ; Code:$4a00; Must1:False),
		(Name:'AND'   ; Code:$4e00; Must1:False),
		(Name:'LACT'  ; Code:$4200; Must1:False),
		(Name:'OR'    ; Code:$4d00; Must1:False),
		(Name:'SUBB'  ; Code:$4f00; Must1:False),
		(Name:'SUBC'  ; Code:$4700; Must1:False),
		(Name:'SUBH'  ; Code:$4400; Must1:False),
		(Name:'SUBS'  ; Code:$4500; Must1:False),
		(Name:'SUBT'  ; Code:$4600; Must1:False),
		(Name:'XOR'   ; Code:$4c00; Must1:False),
		(Name:'ZALH'  ; Code:$4000; Must1:False),
		(Name:'ZALR'  ; Code:$7b00; Must1:False),
		(Name:'ZALS'  ; Code:$4100; Must1:False),
		(Name:'LDP'   ; Code:$5200; Must1:False),
		(Name:'MAR'   ; Code:$5500; Must1:False),
		(Name:'LPH'   ; Code:$5300; Must1:False),
		(Name:'LT'    ; Code:$3c00; Must1:False),
		(Name:'LTA'   ; Code:$3d00; Must1:False),
		(Name:'LTD'   ; Code:$3f00; Must1:False),
		(Name:'LTP'   ; Code:$3e00; Must1:False),
		(Name:'LTS'   ; Code:$5b00; Must1:False),
		(Name:'MPY'   ; Code:$3800; Must1:False),
		(Name:'MPYA'  ; Code:$3a00; Must1:False),
		(Name:'MPYS'  ; Code:$3b00; Must1:False),
		(Name:'MPYU'  ; Code:$cf00; Must1:False),
		(Name:'SPH'   ; Code:$7d00; Must1:False),
		(Name:'SPL'   ; Code:$7c00; Must1:False),
		(Name:'SQRA'  ; Code:$3900; Must1:False),
		(Name:'SQRS'  ; Code:$5a00; Must1:False),
		(Name:'DMOV'  ; Code:$5600; Must1:False),
		(Name:'TBLR'  ; Code:$5800; Must1:False),
		(Name:'TBLW'  ; Code:$5900; Must1:False),
		(Name:'BITT'  ; Code:$5700; Must1:False),
		(Name:'LST'   ; Code:$5000; Must1:False),
		(Name:'LST1'  ; Code:$5100; Must1:False),
		(Name:'POPD'  ; Code:$7a00; Must1:False),
		(Name:'PSHD'  ; Code:$5400; Must1:False),
		(Name:'RPT'   ; Code:$4b00; Must1:False),
		(Name:'SST'   ; Code:$7800; Must1:True),
		(Name:'SST1'  ; Code:$7900; Must1:True));

   AdrOrder2ndAdrCnt=4;
   AdrOrders2ndAdr:ARRAY[1..AdrOrder2ndAdrCnt] OF AdrOrder=
               ((Name:'BLKD'  ; Code:$fd00; Must1:False),
		(Name:'BLKP'  ; Code:$fc00; Must1:False),
		(Name:'MAC'   ; Code:$5d00; Must1:False),
		(Name:'MACD'  ; Code:$5c00; Must1:False));

   AdrShiftOrderCnt=6;
   AdrShiftOrders:ARRAY[1..AdrShiftOrderCnt] OF AdrShiftOrder=
	       ((Name:'ADD'   ; Code:$0000; AllowShifts:$f),
		(Name:'LAC'   ; Code:$2000; AllowShifts:$f),
		(Name:'SACH'  ; Code:$6800; AllowShifts:$7),
		(Name:'SACL'  ; Code:$6000; AllowShifts:$7),
		(Name:'SUB'   ; Code:$1000; AllowShifts:$f),
		(Name:'BIT'   ; Code:$9000; AllowShifts:$f));

   ImmOrderCnt=16;
   ImmOrders:ARRAY[1..ImmOrderCnt] OF ImmOrder=
	       ((Name:'ADDK'  ; Code:$cc00; Min:    0; Max:  255; Mask:  $ff),
		(Name:'LACK'  ; Code:$ca00; Min:    0; Max:  255; Mask:  $ff),
		(Name:'SUBK'  ; Code:$cd00; Min:    0; Max:  255; Mask:  $ff),
		(Name:'ADRK'  ; Code:$7e00; Min:    0; Max:  255; Mask:  $ff),
		(Name:'SBRK'  ; Code:$7f00; Min:    0; Max:  255; Mask:  $ff),
		(Name:'RPTK'  ; Code:$cb00; Min:    0; Max:  255; Mask:  $ff),
		(Name:'MPYK'  ; Code:$a000; Min:-4096; Max: 4095; Mask:$1fff),
		(Name:'SPM'   ; Code:$ce08; Min:    0; Max:    3; Mask:   $3),
		(Name:'CMPR'  ; Code:$ce50; Min:    0; Max:    3; Mask:   $3),
		(Name:'FORT'  ; Code:$ce0e; Min:    0; Max:    1; Mask:   $1),
		(Name:'ADLK'  ; Code:$d002; Min:    0; Max:$7fff; Mask:$ffff),
		(Name:'ANDK'  ; Code:$d004; Min:    0; Max:$7fff; Mask:$ffff),
		(Name:'LALK'  ; Code:$d001; Min:    0; Max:$7fff; Mask:$ffff),
		(Name:'ORK'   ; Code:$d005; Min:    0; Max:$7fff; Mask:$ffff),
		(Name:'SBLK'  ; Code:$d003; Min:    0; Max:$7fff; Mask:$ffff),
		(Name:'XORK'  ; Code:$d006; Min:    0; Max:$7fff; Mask:$ffff));

VAR
   AdrMode:Word;
   AdrOK:Boolean;

   CPU32025,CPU32026,CPU32028:CPUVar;

	FUNCTION EvalARExpression(Asc:String; VAR OK:Boolean):Word;
BEGIN
   OK:=True;

   IF (Length(Asc)=3)
   AND (UpCase(Asc[1])='A') AND (UpCase(Asc[2])='R') AND (Asc[3]>='0') AND (Asc[3]<='7') THEN
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
     CodeEquate(SegIO,0,15);
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

	PROCEDURE MakeCode_3202X;
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

   IF Memo('CNFD') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU=CPU32026 THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=1; WAsmCode[0]:=$ce04; Exit;
      END;
     Exit;
    END;

   IF Memo('CNFP') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE IF MomCPU=CPU32026 THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=1; WAsmCode[0]:=$ce05; Exit;
      END;
     Exit;
    END;



  IF Memo('CONF') THEN
   BEGIN
    IF ArgCnt<>1 THEN WrError(1110)
    ELSE IF MomCPU<>CPU32026 THEN WrError(1500)
    ELSE
     BEGIN
      WAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt2,OK);
      IF OK THEN
       BEGIN
	CodeLen:=1; Inc(WAsmCode[0],$ce3c);
       END;
     END;
    Exit;
   END;

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
       IF (ArgCnt<1) OR (ArgCnt>3) THEN WrError(1110)
       ELSE
	BEGIN
         AdrMode := 0;
         IF ArgCnt>1 THEN
          BEGIN
 	   DecodeAdr(ArgStr[2],3,False);
           IF AdrMode<$80 THEN WrError(1350);
          END;
	 WAsmCode[1]:=EvalIntExpression(ArgStr[1],Int16,OK);
	 IF OK THEN
          BEGIN
	   CodeLen:=2; WAsmCode[0]:=Code+(AdrMode AND $7f);
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

   { 2 Addressen }

   FOR z:=1 TO AdrOrder2ndAdrCnt DO
    WITH AdrOrders2ndAdr[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
       ELSE
	BEGIN
	 WAsmCode[1]:=EvalIntExpression(ArgStr[1],Int16,OK);
	 DecodeAdr(ArgStr[2],3,Must1);
	 IF OK AND AdrOK THEN
	  BEGIN
	   CodeLen:=2; WAsmCode[0]:=Code+AdrMode;
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
	 DecodeAdr(ArgStr[1],3,False);
	 IF AdrOK THEN
	  BEGIN
	   IF ArgCnt<2 THEN
	    BEGIN
	     OK:=True; AdrWord:=0;
	    END
	   ELSE
	    BEGIN
	     AdrWord:=EvalIntExpression(ArgStr[2],Int4,OK);
	     IF (OK) AND (FirstPassUnknown) THEN AdrWord:=0;
	    END;
	   IF OK THEN
	    IF AllowShifts < AdrWord THEN WrError(1380)
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
	 AdrWord:=EvalIntExpression(ArgStr[2],Int4,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegIO);
	   CodeLen:=1;
	   WAsmCode[0]:=$8000+AdrMode+(AdrWord SHL 8);
	   IF Memo('OUT') THEN Inc(WAsmCode[0],$6000);
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
       IF (ArgCnt<1) OR (ArgCnt>2) OR ((ArgCnt=2) AND (Mask <> $ffff)) THEN WrError(1110)
       ELSE
	BEGIN
	 AdrLong:=EvalIntExpression(ArgStr[1],Int32,OK);
	 IF OK THEN
	  BEGIN
	   IF FirstPassUnknown THEN AdrLong:=AdrLong AND Mask;
           IF Mask = $ffff THEN
            BEGIN
             IF AdrLong<-32768 THEN WrError(1315)
             ELSE IF AdrLong>65535 THEN WrError(1320)
             ELSE
              BEGIN
               AdrWord:=0;
               OK:=True;
               IF ArgCnt=2 THEN
                BEGIN
       	         AdrWord:=EvalIntExpression(ArgStr[2],Int4,OK);
	         IF (OK) AND (FirstPassUnknown) THEN AdrWord:=0;
                END;
               IF OK THEN
                BEGIN
                 CodeLen:=2; WAsmCode[0]:=Code+(AdrWord SHL 8); WAsmCode[1]:=AdrLong;
                END;
              END;
            END ELSE BEGIN
	     IF AdrLong<Min THEN WrError(1315)
	     ELSE IF AdrLong>Max THEN WrError(1320)
	     ELSE
	      BEGIN
  	       CodeLen:=1; WAsmCode[0]:=Code+(AdrLong AND Mask);
	      END;
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
	 CodeLen:=1; WAsmCode[0]:=$5588+AdrWord;
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
	   IF Memo('SAR') THEN Inc(WAsmCode[0],$4000);
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
	   WAsmCode[0]:=Lo(WAsmCode[0])+$c000+(AdrWord SHL 8);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('LRLK') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalARExpression(ArgStr[1],OK);
       IF OK THEN
	BEGIN
	 WAsmCode[1]:=EvalIntExpression(ArgStr[2],Int16,OK);
	 IF OK THEN
	  BEGIN
	   CodeLen:=2;
	   WAsmCode[0]:=$d000+(AdrWord SHL 8);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('LDPK') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=ConstIntVal(ArgStr[1],Int16,OK);
       IF OK AND (WAsmCode[0]<$200) AND (WAsmCode[0]>=0) THEN (* emulate Int9 *)
        BEGIN
          CodeLen:=1;
	  WAsmCode[0]:=(WAsmCode[0] AND $1ff)+$c800;
        END
       ELSE
        BEGIN
         WAsmCode[0]:=EvalIntExpression(ArgStr[1],Int16,OK);
         IF OK THEN
          BEGIN
           ChkSpace(SegData);
           CodeLen:=1;
 	   WAsmCode[0]:=((WAsmCode[0] SHR 7) AND $1ff)+$c800;
	  END;
        END;
      END;
     Exit;
    END;

   IF Memo('NORM') THEN
    BEGIN
     IF (ArgCnt<>1) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],2,False);
       IF AdrOK THEN
	BEGIN
         IF AdrMode < $80 THEN WrError(1350)
         ELSE
	  BEGIN
	   CodeLen:=1;
	   WAsmCode[0]:=$ce82+(AdrMode and $70);
	  END;
	END;
      END;
     Exit;
    END;


   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_3202X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=$ffff;
   SegData  : ok:=ProgCounter <=$ffff;
   SegIO    : ok:=ProgCounter <=$f;
   ELSE ok:=False;
   END;
   ChkPC_3202X:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_3202X:Boolean;
	Far;
BEGIN
   IsDef_3202X:=Memo('BSS') OR Memo('PORT') OR Memo('STRING') OR Memo('RSTRING') OR
                Memo('BYTE') OR Memo('WORD') OR Memo('LONG') OR Memo('FLOAT') OR
                Memo('DOUBLE') OR Memo('EFLOAT') OR Memo('BFLOAT') OR Memo('TFLOAT');
END;

        PROCEDURE SwitchFrom_3202X;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_3202X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$75; NOPCode:=$5500;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData,SegIO];
   Grans[SegCode]:=2; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;
   Grans[SegData]:=2; ListGrans[SegData]:=2; SegInits[SegData]:=0;
   Grans[SegIO  ]:=2; ListGrans[SegIO  ]:=2; SegInits[SegIO  ]:=0;

   MakeCode:=MakeCode_3202X; ChkPC:=ChkPC_3202X; IsDef:=IsDef_3202X;
   SwitchFrom:=SwitchFrom_3202X;
END;

BEGIN
   CPU32025:=AddCPU('320C25',SwitchTo_3202X);
   CPU32026:=AddCPU('320C26',SwitchTo_3202X);
   CPU32028:=AddCPU('320C28',SwitchTo_3202X);

   AddCopyright('TMS320C2x-Generator (C) 1994 Thomas Sailer');
END.
