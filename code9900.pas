{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
        UNIT Code9900;

{ AS - Codegenerator TMS9900 }

INTERFACE

        USES StringUt,NLS,Chunks,
	     AsmDef,AsmPars,AsmSub,CodePseu;


IMPLEMENTATION

CONST
   TwoOrderCount=6;
   OneOrderCount=6;
   SingOrderCount=14;
   SBitOrderCount=3;
   JmpOrderCount=13;
   ShiftOrderCount=4;
   ImmOrderCount=5;
   RegOrderCount=4;
   FixedOrderCount=6;

TYPE
   FixedOrder=RECORD
               Name:String[5];
               Code:Word;
              END;

   TwoOrderArray=ARRAY[0..TwoOrderCount-1] OF FixedOrder;
   OneOrderArray=ARRAY[0..TwoOrderCount-1] OF FixedOrder;
   SingOrderArray=ARRAY[0..SingOrderCount-1] OF FixedOrder;
   SBitOrderArray=ARRAY[0..SBitOrderCount-1] OF FixedOrder;
   JmpOrderArray=ARRAY[0..JmpOrderCount-1] OF FixedOrder;
   ShiftOrderArray=ARRAY[0..ShiftOrderCount-1] OF FixedOrder;
   ImmOrderArray=ARRAY[0..ImmOrderCount-1] OF FixedOrder;
   RegOrderArray=ARRAY[0..RegOrderCount-1] OF FixedOrder;
   FixedOrderArray=ARRAY[0..FixedOrderCount-1] OF FixedOrder;

VAR
   CPU9900:CPUVar;

   IsWord:Boolean;
   AdrVal,AdrPart:Word;
   AdrCnt:Integer;

   TwoOrders:^TwoOrderArray;
   OneOrders:^OneOrderArray;
   SingOrders:^SingOrderArray;
   SBitOrders:^SBitOrderArray;
   JmpOrders:^JmpOrderArray;
   ShiftOrders:^ShiftOrderArray;
   ImmOrders:^ImmOrderArray;
   RegOrders:^RegOrderArray;
   FixedOrders:^FixedOrderArray;

{---------------------------------------------------------------------------}
{ dynamische Belegung/Freigabe Codetabellen }

	PROCEDURE InitFields;
VAR
   z:Word;

        PROCEDURE AddTwo(NName:String; NCode:Word);
BEGIN
   IF z>=TwoOrderCount THEN Halt(255);
   WITH TwoOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddOne(NName:String; NCode:Word);
BEGIN
   IF z>=OneOrderCount THEN Halt(255);
   WITH OneOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddSing(NName:String; NCode:Word);
BEGIN
   IF z>=SingOrderCount THEN Halt(255);
   WITH SingOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddSBit(NName:String; NCode:Word);
BEGIN
   IF z>=SBitOrderCount THEN Halt(255);
   WITH SBitOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddJmp(NName:String; NCode:Word);
BEGIN
   IF z>=JmpOrderCount THEN Halt(255);
   WITH JmpOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddShift(NName:String; NCode:Word);
BEGIN
   IF z>=ShiftOrderCount THEN Halt(255);
   WITH ShiftOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddImm(NName:String; NCode:Word);
BEGIN
   IF z>=ImmOrderCount THEN Halt(255);
   WITH ImmOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddReg(NName:String; NCode:Word);
BEGIN
   IF z>=RegOrderCount THEN Halt(255);
   WITH RegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   IF z>=FixedOrderCount THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(TwoOrders); z:=0;
   AddTwo('A'   ,5); AddTwo('C'   ,4); AddTwo('S'   ,3);
   AddTwo('SOC' ,7); AddTwo('SZC' ,2); AddTwo('MOV' ,6);

   New(OneOrders); z:=0;
   AddOne('COC' ,$08); AddOne('CZC' ,$09); AddOne('XOR' ,$0a);
   AddOne('MPY' ,$0e); AddOne('DIV' ,$0f); AddOne('XOP' ,$0b);

   New(SingOrders); z:=0;
   AddSing('B'   ,$0440); AddSing('BL'  ,$0680); AddSing('BLWP',$0400);
   AddSing('CLR' ,$04c0); AddSing('SETO',$0700); AddSing('INV' ,$0540);
   AddSing('NEG' ,$0500); AddSing('ABS' ,$0740); AddSing('SWPB',$06c0);
   AddSing('INC' ,$0580); AddSing('INCT',$05c0); AddSing('DEC' ,$0600);
   AddSing('DECT',$0640); AddSing('X'   ,$0480);

   New(SBitOrders); z:=0;
   AddSBit('SBO' ,$1d); AddSBit('SBZ',$1e); AddSBit('TB' ,$1f);

   New(JmpOrders); z:=0;
   AddJmp('JEQ',$13); AddJmp('JGT',$15); AddJmp('JH' ,$1b);
   AddJmp('JHE',$14); AddJmp('JL' ,$1a); AddJmp('JLE',$12);
   AddJmp('JLT',$11); AddJmp('JMP',$10); AddJmp('JNC',$17);
   AddJmp('JNE',$16); AddJmp('JNO',$19); AddJmp('JOC',$18);
   AddJmp('JOP',$1c);

   New(ShiftOrders); z:=0;
   AddShift('SLA',$0a); AddShift('SRA',$08);
   AddShift('SRC',$0b); AddShift('SRL',$09);

   New(ImmOrders); z:=0;
   AddImm('AI'  ,$011); AddImm('ANDI',$012); AddImm('CI'  ,$014);
   AddImm('LI'  ,$010); AddImm('ORI' ,$013);

   New(RegOrders); z:=0;
   AddReg('STST',$02c); AddReg('LST',$008);
   AddReg('STWP',$02a); AddReg('LWP',$009);

   New(FixedOrders); z:=0;
   AddFixed('RTWP',$0380); AddFixed('IDLE',$0340);
   AddFixed('RSET',$0360); AddFixed('CKOF',$03c0);
   AddFixed('CKON',$03a0); AddFixed('LREX',$03e0);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(TwoOrders);
   Dispose(OneOrders);
   Dispose(SingOrders);
   Dispose(SBitOrders);
   Dispose(JmpOrders);
   Dispose(ShiftOrders);
   Dispose(ImmOrders);
   Dispose(RegOrders);
   Dispose(FixedOrders);
END;

{---------------------------------------------------------------------------}
{ Adressparser }

        FUNCTION DecodeReg(Asc:String; VAR Erg:Word):Boolean;
VAR
   Err:Integer;
BEGIN
   IF (Length(Asc)>=2) AND (UpCase(Asc[1])='R') THEN
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,Err);
     DecodeReg:=(Err=0) AND (Erg<=15);
    END
   ELSE IF (Length(Asc)>=3) AND (UpCase(Asc[1])='W') AND (UpCase(Asc[2])='R') THEN
    BEGIN
     Val(Copy(Asc,3,Length(Asc)-2),Erg,Err);
     DecodeReg:=(Err=0) AND (Erg<=15);
    END
   ELSE DecodeReg:=False;
END;

        FUNCTION HasDisp(VAR Asc:String):Integer;
VAR
   z,Lev:Integer;
BEGIN
   IF Asc[Length(Asc)]=')' THEN
    BEGIN
     z:=Length(Asc)-1; Lev:=0;
     WHILE (z>=1) AND (Lev<>-1) DO
      BEGIN
       CASE Asc[z] OF
       '(':Dec(Lev);
       ')':Inc(Lev);
       END;
       IF Lev<>-1 THEN Dec(z);
      END;
     IF Lev<>-1 THEN
      BEGIN
       WrError(1300); Exit;
      END;
    END
   ELSE z:=0;

   HasDisp:=z;
END;

        FUNCTION DecodeAdr(Asc:String):Boolean;
VAR
   IncFlag:Boolean;
   Reg:String;
   OK:Boolean;
   z:Integer;
BEGIN
   AdrCnt:=0; DecodeAdr:=False;

   IF DecodeReg(Asc,AdrPart) THEN
    BEGIN
     DecodeAdr:=True; Exit;
    END;

   IF Asc[1]='*' THEN
    BEGIN
     Delete(Asc,1,1);
     IF Asc[Length(Asc)]='+' THEN
      BEGIN
       IncFlag:=True; Delete(Asc,Length(Asc),1);
      END
     ELSE IncFlag:=False;
     IF NOT DecodeReg(Asc,AdrPart) THEN WrXError(1445,Asc)
     ELSE
      BEGIN
       Inc(AdrPart,$10+(Ord(IncFlag) SHL 5));
       DecodeAdr:=True;
      END;
     Exit;
    END;

   IF Asc[1]='@' THEN Delete(Asc,1,1);

   z:=HasDisp(Asc);
   IF z=0 THEN
    BEGIN
     FirstPassUnknown:=False;
     AdrVal:=EvalIntExpression(Asc,UInt16,OK);
     IF OK THEN
      BEGIN
       AdrPart:=$20; AdrCnt:=1; DecodeAdr:=True;
       IF (NOT FirstPassUnknown) AND (IsWord) AND (Odd(AdrVal)) THEN WrError(180);
      END;
    END
   ELSE
    BEGIN
     Reg:=Copy(Asc,z+1,Length(Asc)-z-1);
     IF NOT DecodeReg(Reg,AdrPart) THEN WrXError(1445,Reg)
     ELSE IF AdrPart=0 THEN WrXError(1445,Reg)
     ELSE
      BEGIN
       AdrVal:=EvalIntExpression(Copy(Asc,1,z-1),Int16,OK);
       IF OK THEN
        BEGIN
         Inc(AdrPart,$20); AdrCnt:=1; DecodeAdr:=True;
        END;
      END;
    END;
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF9900Count=1;
   ONOFF9900s:ARRAY[1..ONOFF9900Count] OF ONOFFRec=
              ((Name:'PADDING'; Dest:@DoPadding ; FlagName:DoPaddingName ));
VAR
   z:Integer;
   OK:Boolean;
   t:TempResult;
   HVal16:Word;

        PROCEDURE PutByte(Value:Byte);
BEGIN
   IF Odd(CodeLen) THEN
    BEGIN
     BAsmCode[CodeLen]:=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]:=Value;
    END
   ELSE BAsmCode[CodeLen]:=Value;
   Inc(CodeLen);
END;

BEGIN
   DecodePseudo:=True;

   IF CodeONOFF(@OnOff9900s,OnOff9900Count) THEN Exit;

   IF Memo('BYTE') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; OK:=True;
       REPEAT
        KillBlanks(ArgStr[z]);
        FirstPassUnknown:=False;
        EvalExpression(ArgStr[z],t);
        CASE t.Typ OF
        TempInt:
         BEGIN
          IF FirstPassUnknown THEN t.Int:=t.Int AND $ff;
          IF NOT RangeCheck(t.Int,Int8) THEN WrError(1320)
          ELSE IF CodeLen=MaxCodeLen THEN
           BEGIN
            WrError(1920); OK:=False;
           END
          ELSE PutByte(t.Int);
         END;
        TempFloat:
         BEGIN
          WrError(1135); OK:=False;
         END;
        TempString:
         IF Length(t.Ascii)+CodeLen>=MaxCodeLen THEN
          BEGIN
           WrError(1920); OK:=False;
          END
         ELSE
          BEGIN
           TranslateString(t.Ascii);
           FOR z:=1 TO Length(t.Ascii) DO PutByte(Ord(t.Ascii[z]));
          END;
        TempNone:
         OK:=False;
        END;
        Inc(z);
       UNTIL (z>ArgCnt) OR (NOT OK);
       IF NOT OK THEN CodeLen:=0
       ELSE IF (Odd(CodeLen)) AND (DoPadding) THEN PutByte(0);
      END;
     Exit;
    END;

   IF Memo('WORD')  THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; OK:=True;
       REPEAT
        HVal16:=EvalIntExpression(ArgStr[z],Int16,OK);
        IF OK THEN
	 BEGIN
	  WAsmCode[CodeLen SHR 1]:=HVal16;
	  Inc(CodeLen,2);
         END;
        Inc(z);
       UNTIL (z>ArgCnt) OR (NOT OK);
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('BSS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       HVal16:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF FirstPassUnknown THEN WrError(1820)
       ELSE IF (OK) THEN
        BEGIN
         IF (DoPadding) AND (Odd(HVal16)) THEN Inc(HVal16);
         DontPrint:=True; CodeLen:=HVal16;
         IF MakeUseList THEN
          IF AddChunk(SegChunks[ActPC],ProgCounter,HVal16,ActPC=SegCode) THEN WrError(90);
        END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_9900;
        Far;
VAR
   HPart:Word;
   AdrInt:Integer;
   z:Integer;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; IsWord:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { zwei Operanden }

   FOR z:=0 TO TwoOrderCount-1 DO
    WITH TwoOrders^[z] DO
     IF (Memo(Name)) OR (Memo(Name+'B')) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF DecodeAdr(ArgStr[1]) THEN
        BEGIN
         WAsmCode[0]:=AdrPart; WAsmCode[1]:=AdrVal;
         HPart:=AdrCnt;
         IF DecodeAdr(ArgStr[2]) THEN
          BEGIN
           Inc(WAsmCode[0],AdrPart SHL 6); WAsmCode[1+HPart]:=AdrVal;
           CodeLen:=(1+HPart+AdrCnt) SHL 1;
           IF OpPart[Length(OpPart)]='B' THEN Inc(WAsmCode[0],$1000);
           Inc(WAsmCode[0],Code SHL 13);
          END;
        END;
       Exit;
      END;

   FOR z:=0 TO OneOrderCount-1 DO
    WITH OneOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF DecodeAdr(ArgStr[1]) THEN
        BEGIN
         WAsmCode[0]:=AdrPart; WAsmCode[1]:=AdrVal;
         IF NOT DecodeReg(ArgStr[2],HPart) THEN WrXError(1445,ArgStr[2])
         ELSE
          BEGIN
           Inc(WAsmCode[0],(HPart SHL 6)+(Code SHL 10));
           CodeLen:=(1+AdrCnt) SHL 1;
          END;
        END;
       Exit;
      END;

   IF (Memo('LDCR')) OR (Memo('STCR')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF DecodeAdr(ArgStr[1]) THEN
      BEGIN
       WAsmCode[0]:=$3000+(Ord(Memo('STCR')) SHL 10)+AdrPart;
       WAsmCode[1]:=AdrVal;
       FirstPassUnknown:=False;
       HPart:=EvalIntExpression(ArgStr[2],UInt5,OK);
       IF FirstPassUnknown THEN HPart:=1;
       IF OK THEN
        IF ChkRange(HPart,1,16) THEN
         BEGIN
          Inc(WAsmCode[0],(HPart AND 15) SHL 6);
          CodeLen:=(1+AdrCnt) SHL 1;
	 END;
      END;
     Exit;
    END;

   FOR z:=0 TO ShiftOrderCount-1 DO
    WITH ShiftOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[2],WAsmCode[0]) THEN WrXError(1445,ArgStr[2])
       ELSE
        BEGIN
         IF DecodeReg(ArgStr[1],HPart) THEN
          BEGIN
           OK:=HPart=0;
           IF NOT OK THEN WrXError(1445,ArgStr[1]);
          END
         ELSE
	  BEGIN
           FirstPassUnknown:=False;
	   HPart:=EvalIntExpression(ArgStr[1],UInt4,OK);
           IF (OK) AND (NOT FirstPassUnknown) AND (HPArt=0) THEN
            BEGIN
             WrError(1315); OK:=False;
            END;
          END;
         IF OK THEN
          BEGIN
           Inc(WAsmCode[0],(HPart SHL 4)+(Code SHL 8));
           CodeLen:=2;
          END;
        END;
       Exit;
      END;

   FOR z:=0 TO ImmOrderCount-1 DO
    WITH ImmOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[2],WAsmCode[0]) THEN WrXError(1445,ArgStr[2])
       ELSE
        BEGIN
         WAsmCode[1]:=EvalIntExpression(ArgStr[1],Int16,OK);
         IF OK THEN
          BEGIN
           Inc(WAsmCode[0],(Code SHL 5)); CodeLen:=4;
          END;
        END;
       Exit;
      END;

   FOR z:=0 TO RegOrderCount-1 DO
    WITH RegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[1],WAsmCode[0]) THEN WrXError(1445,ArgStr[1])
       ELSE
        BEGIN
         Inc(WAsmCode[0],Code SHL 4); CodeLen:=2;
        END;
       Exit;
      END;

   { ein Operand }

   IF (Memo('MPYS')) OR (Memo('DIVS')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF DecodeAdr(ArgStr[1]) THEN
      BEGIN
       WAsmCode[0]:=$0180+(Ord(Memo('MPYS')) SHL 6)+AdrPart;
       WAsmCode[1]:=AdrVal;
       CodeLen:=(1+AdrCnt) SHL 1;
      END;
     Exit;
    END;

   FOR z:=0 TO SingOrderCount-1 DO
    WITH SingOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF  DecodeAdr(ArgStr[1]) THEN
        BEGIN
         WAsmCode[0]:=Code+AdrPart;
         WAsmCode[1]:=AdrVal;
         CodeLen:=(1+AdrCnt) SHL 1;
        END;
       Exit;
      END;

   FOR z:=0 TO SBitOrderCount-1 DO
    WITH SBitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         WAsmCode[0]:=EvalIntExpression(ArgStr[1],SInt8,OK);
         IF OK THEN
          BEGIN
           WAsmCode[0]:=(WAsmCode[0] AND $ff) OR (Code SHL 8);
           CodeLen:=2;
          END;
        END;
       Exit;
      END;

   FOR z:=0 TO JmpOrderCount-1 DO
    WITH JmpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
         IF OK THEN
          IF Odd(AdrInt) THEN WrError(1375)
          ELSE IF (NOT SymbolQuestionable) AND ((AdrInt<-256) OR (AdrInt>254)) THEN WrError(1370)
          ELSE
           BEGIN
            WAsmCode[0]:=((AdrInt SHR 1) AND $ff) OR (Code SHL 8);
            CodeLen:=2;
           END;
        END;
       Exit;
      END;

   IF (Memo('LWPI')) OR (Memo('LIMI')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[1]:=EvalIntExpression(ArgStr[1],UInt16,OK);
       IF OK THEN
        BEGIN
         WAsmCode[0]:=($017+Ord(Memo('LIMI'))) SHL 5;
         CodeLen:=4;
        END;
      END;
     Exit;
    END;

   { kein Operand }

   FOR z:=0 TO FixedOrderCount-1 DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
        BEGIN
         WASmCode[0]:=Code; CodeLen:=2;
        END;
       Exit;
      END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_9900:Boolean;
	Far;
BEGIN
   ChkPC_9900:=(ActPC=SegCode) AND (ProgCounter>=0) AND (ProgCounter<$10000);
END;

        FUNCTION IsDef_9900:Boolean;
	Far;
BEGIN
   IsDef_9900:=False;
END;

        PROCEDURE SwitchFrom_9900;
	Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_9900;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$48; NOPCode:=$0000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_9900; ChkPC:=ChkPC_9900; IsDef:=IsDef_9900;
   SwitchFrom:=SwitchFrom_9900;

   InitFields;
END;

BEGIN
   CPU9900:=AddCPU('TMS9900',SwitchTo_9900);
END.
