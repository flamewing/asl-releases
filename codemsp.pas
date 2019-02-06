{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeMSP;

{ AS Codegeneratormodul MSP430 }

INTERFACE
        Uses NLS,
             Chunks,AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

CONST
   TwoOpCount=12;
   OneOpCount=6;
   JmpCount=10;

TYPE
   FixedOrder=RECORD
               Name:String[4];
               Code:Word;
              END;

   TwoOpArray=ARRAY[0..TwoOpCount-1] OF FixedOrder;
   JmpArray=ARRAY[0..JmpCount-1] OF FixedOrder;

   OneOpOrder=RECORD
               Name:String[4];
               MayByte:Boolean;
               Code:Word;
              END;

   OneOpArray=ARRAY[0..OneOpCount] OF OneOpOrder;


VAR
   CPUMSP430:CPUVar;

   TwoOpOrders:^TwoOpArray;
   OneOpOrders:^OneOpArray;
   JmpOrders:^JmpArray;

   AdrMode,AdrMode2,AdrPart,AdrPart2:Word;
   AdrCnt,AdrCnt2:Byte;
   AdrVal,AdrVal2:Word;
   OpSize:Byte;
   PCDist:Word;

{---------------------------------------------------------------------------}

        PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddTwoOp(NName:String; NCode:Word);
BEGIN
   IF z>=TwoOpCount THEN Halt(255);
   WITH TwoOpOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddOneOp(NName:String; NMay:Boolean; NCode:Word);
BEGIN
   IF z>=OneOpCount THEN Halt(255);
   WITH OneOpOrders^[z] DO
    BEGIN
     Name:=NName; MayByte:=NMay; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddJmp(NName:String; NCode:Word);
BEGIN
   IF z>=JmpCount THEN Halt(255);
   WITH JmpOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(TwoOpOrders); z:=0;
   AddTwoOp('MOV' ,$4000); AddTwoOp('ADD' ,$5000);
   AddTwoOp('ADDC',$6000); AddTwoOp('SUBC',$7000);
   AddTwoOp('SUB' ,$8000); AddTwoOp('CMP' ,$9000);
   AddTwoOp('DADD',$a000); AddTwoOp('BIT' ,$b000);
   AddTwoOp('BIC' ,$c000); AddTwoOp('BIS' ,$d000);
   AddTwoOp('XOR' ,$e000); AddTwoOp('AND' ,$f000);

   New(OneOpOrders); z:=0;
   AddOneOp('RRC' ,True ,$1000); AddOneOp('RRA' ,True ,$1100);
   AddOneOp('PUSH',True ,$1200); AddOneOp('SWPB',False,$1080);
   AddOneOp('CALL',False,$1280); AddOneOp('SXT' ,False,$1180);

   New(JmpOrders); z:=0;
   AddJmp('JNE' ,$2000); AddJmp('JNZ' ,$2000);
   AddJmp('JE'  ,$2400); AddJmp('JZ'  ,$2400);
   AddJmp('JNC' ,$2800); AddJmp('JC'  ,$2c00);
   AddJmp('JN'  ,$3000); AddJmp('JGE' ,$3400);
   AddJmp('JL'  ,$3800); AddJmp('JMP' ,$3C00);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(TwoOpOrders);
   Dispose(OneOpOrders);
   Dispose(JmpOrders);
END;

{---------------------------------------------------------------------------}

        PROCEDURE DecodeAdr(Asc:String; Mask:Byte; MayImm:Boolean);
LABEL
   Found;
VAR
   AdrWord:Word;
   OK:Boolean;
   p:Integer;

        FUNCTION DecodeReg(Asc:String; VAR Erg:Word):Boolean;
VAR
   IO:ValErgType;
BEGIN
   IF NLS_StrCaseCmp(Asc,'PC')=0 THEN
    BEGIN
     Erg:=0; DecodeReg:=True;
    END
   ELSE IF NLS_StrCaseCmp(Asc,'SP')=0 THEN
    BEGIN
     Erg:=1; DecodeReg:=True;
    END
   ELSE IF NLS_StrCaseCmp(Asc,'SR')=0 THEN
    BEGIN
     Erg:=2; DecodeReg:=True;
    END
   ELSE DecodeReg:=False;
   IF (UpCase(Asc[1])='R') AND (Length(Asc)>=2) AND (Length(Asc)<=3) THEN
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,IO);
     IF (OK) AND (Erg<16) THEN DecodeReg:=True;
    END;
END;

        PROCEDURE ResetAdr;
BEGIN
   AdrMode:=$ff; AdrCnt:=0;
END;

BEGIN
   ResetAdr;

   { immediate }

   IF Asc[1]='#' THEN
    BEGIN
     IF NOT MayImm THEN WrError(1350)
     ELSE
      BEGIN
       Delete(Asc,1,1);
       IF OpSize=1 THEN AdrWord:=EvalIntExpression(Asc,Int8,OK)
       ELSE AdrWord:=EvalIntExpression(Asc,Int16,OK);
       IF OK THEN
        BEGIN
         CASE AdrWord OF
         0:BEGIN
            AdrPart:=3; AdrMode:=0;
           END;
         1:BEGIN
            AdrPart:=3; AdrMode:=1;
           END;
         2:BEGIN
            AdrPart:=3; AdrMode:=2;
           END;
         4:BEGIN
            AdrPart:=2; AdrMode:=2;
           END;
         8:BEGIN
            AdrPart:=2; AdrMode:=3;
           END;
         $ffff:BEGIN
            AdrPart:=3; AdrMode:=3;
           END;
         ELSE
          BEGIN
           AdrVal:=AdrWord; AdrCnt:=1;
           AdrPart:=0; AdrMode:=3;
          END;
         END;
        END;
      END;
     Goto Found;
    END;

   { absolut }

   IF Asc[1]='&' THEN
    BEGIN
     Delete(Asc,1,1);
     AdrVal:=EvalIntExpression(Asc,UInt16,OK);
     IF OK THEN
      BEGIN
       AdrMode:=1; AdrPart:=2; AdrCnt:=1;
      END;
     Goto Found;
    END;

   { Register }

   IF DecodeReg(Asc,AdrPart) THEN
    BEGIN
     IF AdrPart=3 THEN WrXError(1445,Asc)
     ELSE AdrMode:=0;
     Goto Found;
    END;

   { Displacement }

   IF Asc[Length(Asc)]=')' THEN
    BEGIN
     p:=RQuotPos(Asc,'(');
     IF (p<>0) AND (DecodeReg(Copy(Asc,p+1,Length(Asc)-p-1),AdrPart)) THEN
      BEGIN
       AdrVal:=EvalIntExpression(Copy(Asc,1,p-1),Int16,OK);
       IF OK THEN
        IF (AdrPart=2) OR (AdrPart=3) THEN WrXError(1445,Asc)
        ELSE IF (AdrVal=0) AND (Mask AND 4<>0) THEN AdrMode:=2
        ELSE
         BEGIN
          AdrCnt:=1; AdrMode:=1;
         END;
       Goto Found;
      END;
    END;

   { indirekt mit/ohne Autoinkrement }

   IF (Asc[1]='@') OR (Asc[1]='*') THEN
    BEGIN
     Delete(Asc,1,1);
     IF Asc[Length(Asc)]='+' THEN
      BEGIN
       AdrWord:=1; Delete(Asc,Length(Asc),1);
      END
     ELSE AdrWord:=0;
     IF NOT DecodeReg(Asc,AdrPart) THEN WrXError(1445,Asc)
     ELSE IF (AdrPart=2) OR (AdrPart=3) THEN WrXError(1445,Asc)
     ELSE IF (AdrWord=0) AND (Mask AND 4=0) THEN
      BEGIN
       AdrVal:=0; AdrCnt:=1; AdrMode:=1;
      END
     ELSE AdrMode:=2+AdrWord;
     Goto Found;
    END;

   { bleibt PC-relativ }

   AdrWord:=EvalIntExpression(Asc,UInt16,OK)-EProgCounter-PCDist;
   IF OK THEN
    BEGIN
     AdrPart:=0; AdrMode:=1; AdrCnt:=1; AdrVal:=AdrWord;
    END;

Found:
   IF (AdrMode<>$ff) AND (Mask AND (1 SHL AdrMode)=0) THEN
   BEGIN
    ResetAdr; WrError(1350);
   END;
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF430Count=1;
   ONOFF430s:ARRAY[1..ONOFF430Count] OF ONOFFRec=
              ((Name:'PADDING'; Dest:@DoPadding ; FlagName:DoPaddingName ));
VAR
   t:TempResult;
   HVal16:Word;
   z:Integer;
   OK:Boolean;

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

   IF CodeONOFF(@OnOff430s,OnOff430Count) THEN Exit;

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

{   float exp (8bit bias 128) sign mant (impl. norm.)
   double exp (8bit bias 128) sign mant (impl. norm.)}

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_MSP;
	Far;
VAR
   z,AdrInt:Integer;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Attribut bearbeiten }

   IF AttrPart='' THEN OpSize:=0
   ELSE IF Length(AttrPart)>1 THEN WrError(1107)
   ELSE
    CASE UpCase(AttrPart[1]) OF
    'B': OpSize:=1;
    'W': OpSize:=0;
    ELSE
     BEGIN
      WrError(1107);
      Exit;
     END;
    END;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { zwei Operanden }

   FOR z:=0 TO TwoOpCount-1 DO
    WITH TwoOpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         PCDist:=2; DecodeAdr(ArgStr[1],15,True);
         IF AdrMode<>$ff THEN
          BEGIN
           AdrMode2:=AdrMode; AdrPart2:=AdrPart; AdrCnt2:=AdrCnt; AdrVal2:=AdrVal;
           Inc(PCDist,AdrCnt2 SHL 1); DecodeAdr(ArgStr[2],3,False);
           IF AdrMode<>$ff THEN
            BEGIN
             WAsmCode[0]:=Code+(AdrPart2 SHL 8)+(AdrMode SHL 7)
                          +(OpSize SHL 6)+(AdrMode2 SHL 4)+AdrPart;
             Move(AdrVal2,WAsmCode[1],AdrCnt2 SHL 1);
             Move(AdrVal,WAsmCode[1+AdrCnt2],AdrCnt SHL 1);
             CodeLen:=(1+AdrCnt+AdrCnt2) SHL 1;
            END;
          END;
        END;
       Exit;
      END;

   { ein Operand }

   FOR z:=0 TO OneOpCount-1 DO
    WITH OneOpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF (OpSize=1) AND (NOT MayByte) THEN WrError(1130)
       ELSE
        BEGIN
         PCDist:=2; DecodeAdr(ArgStr[1],15,False);
         IF AdrMode<>$ff THEN
          BEGIN
           WAsmCode[0]:=Code+(OpSize SHL 6)+(AdrMode SHL 4)+AdrPart;
           Move(AdrVal,WAsmCode[1],AdrCnt SHL 1);
           CodeLen:=(1+AdrCnt) SHL 1;
          END;
        END;
       Exit;
      END;

   { kein Operand }

   IF Memo('RETI') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF OpSize<>0 THEN WrError(1130)
     ELSE
      BEGIN
       WAsmCode[0]:=$1300; CodeLen:=2;
      END;
     Exit;
    END;

   { SprÅnge }

   FOR z:=0 TO JmpCount-1 DO
    WITH JmpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF OpSize<>0 THEN WrError(1130)
       ELSE
        BEGIN
         AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
         IF OK THEN
          IF Odd(AdrInt) THEN WrError(1375)
          ELSE IF (NOT SymbolQuestionable) AND ((AdrInt<-1024) OR (AdrInt>1022)) THEN WrError(1370)
          ELSE
           BEGIN
            WAsmCode[0]:=Code+((AdrInt SHR 1) AND $3ff);
            CodeLen:=2;
           END;
        END;
       Exit;
      END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_MSP:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<$10000;
   ELSE ok:=False;
   END;
   ChkPC_MSP:=(ok) AND (ProgCounter>=0);
END;


        FUNCTION IsDef_MSP:Boolean;
	Far;
BEGIN
   IsDef_MSP:=False;
END;

        PROCEDURE SwitchFrom_MSP;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_MSP;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$4a; NOPCode:=$4303; { = MOV #0,#0 }
   DivideChars:=','; HasAttrs:=True; AttrChars:='.';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_MSP; ChkPC:=ChkPC_MSP; IsDef:=IsDef_MSP;
   SwitchFrom:=SwitchFrom_MSP; InitFields;
END;

BEGIN
   CPUMSP430:=AddCPU('MSP430',SwitchTo_MSP);
END.
