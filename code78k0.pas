{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code78K0;

{ AS Codegeneratormodul uPD78K0-Familie }

INTERFACE
        Uses StringUt,NLS,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

CONST
   ModNone=-1;
   ModReg8=0;   MModReg8=1 SHL ModReg8;
   ModReg16=1;  MModReg16=1 SHL ModReg16;
   ModImm=2;    MModImm=1 SHL ModImm;
   ModShort=3;  MModShort=1 SHL ModShort;
   ModSFR=4;    MModSFR=1 SHL ModSFR;
   ModAbs=5;    MModAbs=1 SHL ModAbs;
   ModIReg=6;   MModIReg=1 SHL ModIReg;
   ModIndex=7;  MModIndex=1 SHL ModIndex;
   ModDisp=8;   MModDisp=1 SHL ModDisp;

   AccReg=1;    AccReg16=0;

   FixedOrderCount=11;
   AriOrderCount=8;
   Ari16OrderCount=3;
   ShiftOrderCount=4;
   Bit2OrderCount=3;
   RelOrderCount=4;
   BRelOrderCount=3;

TYPE
   FixedOrder=RECORD
               Name:String[5];
               Code:Word;
              END;

   FixedOrderArray = ARRAY[0..FixedOrderCount-1] OF FixedOrder;
   AriOrderArray   = ARRAY[0..AriOrderCount  -1] OF String[4];
   Ari16OrderArray = ARRAY[0..Ari16OrderCount-1] OF String[4];
   ShiftOrderArray = ARRAY[0..ShiftOrderCount-1] OF String[4];
   Bit2OrderArray  = ARRAY[0..Bit2OrderCount -1] OF String[4];
   RelOrderArray   = ARRAY[0..RelOrderCount  -1] OF String[3];
   BRelOrderArray  = ARRAY[0..BRelOrderCount -1] OF String[5];

VAR
   SaveInitProc:PROCEDURE;

   FixedOrders:^FixedOrderArray;
   AriOrders:^AriOrderArray;
   Ari16Orders:^Ari16OrderArray;
   ShiftOrders:^ShiftOrderArray;
   Bit2Orders:^Bit2OrderArray;
   RelOrders:^RelOrderArray;
   BRelOrders:^BRelOrderArray;

   OpSize,AdrCnt,AdrPart:Byte;
   AdrVals:ARRAY[0..1] OF Byte;
   AdrMode:ShortInt;

   CPU78070:CPUVar;


{---------------------------------------------------------------------------}
{ dynamische Codetabellenverwaltung }

	PROCEDURE InitFields;
VAR
   z:Integer;
   Err:ValErgType;

   	PROCEDURE AddFixed(NewName:String; NewCode:Word);
BEGIN
   IF z>=FixedOrderCount THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NewName; Code:=NewCode;
    END;
   Inc(z);
END;

   	PROCEDURE AddAri(NewName:String);
BEGIN
   IF z>=AriOrderCount THEN Halt;
   AriOrders^[z]:=NewName;
   Inc(z);
END;

   	PROCEDURE AddAri16(NewName:String);
BEGIN
   IF z>=Ari16OrderCount THEN Halt;
   Ari16Orders^[z]:=NewName;
   Inc(z);
END;

   	PROCEDURE AddShift(NewName:String);
BEGIN
   IF z>=ShiftOrderCount THEN Halt;
   ShiftOrders^[z]:=NewName;
   Inc(z);
END;

   	PROCEDURE AddBit2(NewName:String);
BEGIN
   IF z>=Bit2OrderCount THEN Halt;
   Bit2Orders^[z]:=NewName;
   Inc(z);
END;

   	PROCEDURE AddRel(NewName:String);
BEGIN
   IF z>=RelOrderCount THEN Halt;
   RelOrders^[z]:=NewName;
   Inc(z);
END;

   	PROCEDURE AddBRel(NewName:String);
BEGIN
   IF z>=BRelOrderCount THEN Halt;
   BRelOrders^[z]:=NewName;
   Inc(z);
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('BRK'  ,$00bf);
   AddFixed('RET'  ,$00af);
   AddFixed('RETB' ,$009f);
   AddFixed('RETI' ,$008f);
   AddFixed('HALT' ,$7110);
   AddFixed('STOP' ,$7100);
   AddFixed('NOP'  ,$0000);
   AddFixed('EI'   ,$7a1e);
   AddFixed('DI'   ,$7b1e);
   AddFixed('ADJBA',$6180);
   AddFixed('ADJBS',$6190);

   New(AriOrders); z:=0;
   AddAri('ADD' ); AddAri('SUB' ); AddAri('ADDC'); AddAri('SUBC');
   AddAri('CMP' ); AddAri('AND' ); AddAri('OR'  ); AddAri('XOR' );

   New(Ari16Orders); z:=0;
   AddAri16('ADDW'); AddAri16('SUBW'); AddAri16('CMPW');

   New(ShiftOrders); z:=0;
   AddShift('ROR'); AddShift('RORC'); AddShift('ROL'); AddShift('ROLC');

   New(Bit2Orders); z:=0;
   AddBit2('AND1'); AddBit2('OR1'); AddBit2('XOR1');

   New(RelOrders); z:=0;
   AddRel('BC'); AddRel('BNC'); AddRel('BZ'); AddRel('BNZ');

   New(BRelOrders); z:=0;
   AddBRel('BTCLR'); AddBRel('BT'); AddBRel('BF');
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(AriOrders);
   Dispose(Ari16Orders);
   Dispose(ShiftOrders);
   Dispose(Bit2Orders);
   Dispose(RelOrders);
   Dispose(BRelOrders);
END;

{---------------------------------------------------------------------------}
{ Adre·ausdruck parsen }

	PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   Found;
CONST
   RegNames:ARRAY[0..7] OF Char='XACBEDLH';
VAR
   AdrWord:Word;
   z:Integer;
   OK,LongFlag:Boolean;
BEGIN
   AdrMode:=ModNone; AdrCnt:=0;

   { Register }

   FOR z:=0 TO 7 DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      AdrMode:=ModReg8; AdrPart:=z; Goto Found;
     END;

   IF (UpCase(Asc[1])='R') THEN
    IF (Length(Asc)=2) AND (Asc[2]>='0') AND (Asc[2]<='7') THEN
     BEGIN
      AdrMode:=ModReg8; AdrPart:=Ord(Asc[2])-AscOfs; Goto Found;
     END
    ELSE IF (Length(Asc)=3) AND (UpCase(Asc[2])='P') AND (Asc[3]>='0') AND (Asc[3]<='3') THEN
     BEGIN
      AdrMode:=ModReg16; AdrPart:=Ord(Asc[3])-AscOfs; Goto Found;
     END;

   IF Length(Asc)=2 THEN
    FOR z:=0 TO 3 DO
     IF (UpCase(Asc[1])=RegNames[z SHL 1+1]) AND (UpCase(Asc[2])=RegNames[z SHL 1]) THEN
      BEGIN
       AdrMode:=ModReg16; AdrPart:=z; Goto Found;
      END;

   { immediate }

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize OF
     0:AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
     1:BEGIN
        AdrWord:=EvalIntExpression(Asc,Int16,OK);
        IF OK THEN
         BEGIN
          AdrVals[0]:=Lo(AdrWord); AdrVals[1]:=Hi(AdrWord);
	 END;
       END;
     END;
     IF OK THEN
      BEGIN
       AdrMode:=ModImm; AdrCnt:=OpSize+1;
      END;
     Goto Found;
    END;

   { indirekt }

   IF (Asc[1]='[') AND (Asc[Length(Asc)]=']') THEN
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);

     IF (NLS_StrCaseCmp(Asc,'DE')=0) OR (NLS_StrCaseCmp(Asc,'RP2')=0) THEN
      BEGIN
       AdrMode:=ModIReg; AdrPart:=0;
      END
     ELSE IF (NLS_StrCaseCmp(Copy(Asc,1,2),'HL')<>0) AND (NLS_StrCaseCmp(Copy(Asc,1,3),'RP3')<>0) THEN WrXError(1445,Asc)
     ELSE
      BEGIN
       Delete(Asc,1,2); IF Asc[1]='3' THEN Delete(Asc,1,1);
       IF (NLS_StrCaseCmp(Asc,'+B')=0) OR (NLS_StrCaseCmp(Asc,'+R3')=0) THEN
        BEGIN
         AdrMode:=ModIndex; AdrPart:=1;
        END
       ELSE IF (NLS_StrCaseCmp(Asc,'+C')=0) OR (NLS_StrCaseCmp(Asc,'+R2')=0) THEN
        BEGIN
         AdrMode:=ModIndex; AdrPart:=0;
        END
       ELSE
        BEGIN
         AdrVals[0]:=EvalIntExpression(Asc,UInt8,OK);
         IF OK THEN
          IF AdrVals[0]=0 THEN
           BEGIN
            AdrMode:=ModIReg; AdrPart:=1;
           END
          ELSE
           BEGIN
            AdrMode:=ModDisp; AdrCnt:=1;
           END;
        END;
      END;

     Goto Found;
    END;

   { erzwungen lang ? }

   IF Asc[1]='!' THEN
    BEGIN
     LongFlag:=True; Delete(Asc,1,1);
    END
   ELSE LongFlag:=False;

   { -->absolut }

   FirstPassUnknown:=True;
   AdrWord:=EvalIntExpression(Asc,UInt16,OK);
   IF (FirstPassUnknown) THEN
    BEGIN
     AdrWord:=AdrWord AND $ffffe;
     IF (Mask AND MModAbs=0) THEN
      AdrWord:=(AdrWord OR $ff00) AND $ff1f;
    END;
   IF OK THEN
    IF (NOT LongFlag) AND (Mask AND MModShort<>0) AND (AdrWord>=$fe20) AND (AdrWord<=$ff1f) THEN
     BEGIN
      AdrMode:=ModShort; AdrCnt:=1; AdrVals[0]:=Lo(AdrWord);
     END
    ELSE IF (NOT LongFlag) AND (Mask AND MModSFR<>0) AND (((AdrWord>=$ff00) AND (AdrWord<=$ffcf)) OR (AdrWord>=$ffe0)) THEN
     BEGIN
      AdrMode:=ModSFR; AdrCnt:=1; AdrVals[0]:=Lo(AdrWord);
     END
    ELSE
     BEGIN
      AdrMode:=ModAbs; AdrCnt:=2; AdrVals[0]:=Lo(AdrWord); AdrVals[1]:=Hi(AdrWord);
     END;

Found:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     WrError(1350);
     AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

	PROCEDURE ChkEven;
BEGIN
   IF (AdrMode=ModAbs) OR (AdrMode=ModShort) OR (AdrMode=ModSFR) THEN
    IF Odd(AdrVals[0]) THEN WrError(180);
END;

	FUNCTION DecodeBitAdr(Asc:String; VAR Erg:Byte):Boolean;
VAR
   p:Integer;
   OK:Boolean;
BEGIN
   DecodeBitAdr:=False;

   p:=RQuotPos(Asc,'.');
   IF p=0 THEN
    BEGIN
     WrError(1510); Exit;
    END;

   Erg:=EvalIntExpression(Copy(Asc,p+1,Length(Asc)-p),UInt3,OK) SHL 4;
   IF NOT OK THEN Exit;

   DecodeAdr(Copy(Asc,1,p-1),MModShort+MModSFR+MModIReg+MModReg8);
   CASE AdrMode OF
   ModReg8:
    IF AdrPart<>AccReg THEN WrError(1350)
    ELSE
     BEGIN
      Inc(Erg,$88); DecodeBitAdr:=True;
     END;
   ModShort:
    DecodeBitAdr:=True;
   ModSFR:
    BEGIN
     Inc(Erg,$08); DecodeBitAdr:=True;
    END;
   ModIReg:
    IF AdrPart=0 THEN WrError(1350)
    ELSE
     BEGIN
      Inc(Erg,$80); DecodeBitAdr:=True;
     END;
   END;
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_78K0;
	Far;
VAR
   z:Integer;
   HReg:Byte;
   AdrWord:Word;
   AdrInt:Integer;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=0;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { ohne Argument }

   FOR z:=0 TO FixedOrderCount-1 DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF Hi(Code)=0 THEN CodeLen:=1
       ELSE
        BEGIN
         BAsmCode[0]:=Hi(Code); CodeLen:=2;
        END;
       BAsmCode[CodeLen-1]:=Lo(Code);
       Exit;
      END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModShort+MModSFR+MModAbs+MModIReg+MModIndex+MModDisp);
       CASE AdrMode OF
       ModReg8:
        BEGIN
         HReg:=AdrPart;
         IF HReg=AccReg THEN DecodeAdr(ArgStr[2],MModImm+MModReg8+MModShort+MModSFR+MModAbs+MModIReg+MModIndex+MModDisp)
         ELSE DecodeAdr(ArgStr[2],MModReg8+MModImm);
         CASE AdrMode OF
         ModReg8:
          IF (HReg=AccReg)=(AdrPart=AccReg) THEN WrError(1350)
          ELSE IF HReg=AccReg THEN
           BEGIN
            CodeLen:=1; BAsmCode[0]:=$60+AdrPart;
           END
          ELSE
           BEGIN
            CodeLen:=1; BAsmCode[0]:=$70+HReg;
           END;
         ModImm:
          BEGIN
           CodeLen:=2; BAsmCode[0]:=$a0+HReg; BAsmCode[1]:=AdrVals[0];
          END;
         ModShort:
          BEGIN
           CodeLen:=2; BAsmCode[0]:=$f0; BAsmCode[1]:=AdrVals[0];
          END;
         ModSFR:
          BEGIN
           CodeLen:=2; BAsmCode[0]:=$f4; BAsmCode[1]:=AdrVals[0];
          END;
         ModAbs:
          BEGIN
           CodeLen:=3; BAsmCode[0]:=$fe; Move(AdrVals,BAsmCode[1],AdrCnt);
          END;
         ModIReg:
          BEGIN
           CodeLen:=1; BAsmCode[0]:=$85+(AdrPart SHL 1);
          END;
         ModIndex:
          BEGIN
           CodeLen:=1; BAsmCode[0]:=$aa+AdrPart;
          END;
         ModDisp:
          BEGIN
           CodeLen:=2; BAsmCode[0]:=$ae; BAsmCode[1]:=AdrVals[0];
          END;
         END;
        END;
       ModShort:
        BEGIN
         BAsmCode[1]:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModReg8+MModImm);
         CASE AdrMode OF
         ModReg8:
          IF AdrPart<>AccReg THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$f2; CodeLen:=2;
           END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$11; BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
          END;
         END;
        END;
       ModSFR:
        BEGIN
         BAsmCode[1]:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModReg8+MModImm);
         CASE AdrMode OF
         ModReg8:
          IF AdrPart<>AccReg THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$f6; CodeLen:=2;
           END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$13; BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
          END;
         END;
        END;
       ModAbs:
        BEGIN
         Move(AdrVals,BAsmCode[1],2);
         DecodeAdr(ArgStr[2],MModReg8);
         IF AdrMode=ModReg8 THEN
          IF AdrPart<>AccReg THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$9e; CodeLen:=3;
           END;
        END;
       ModIReg:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg8);
         IF AdrMode=ModReg8 THEN
          IF AdrPart<>AccReg THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$95+(AdrPart SHL 1); CodeLen:=1;
           END;
        END;
       ModIndex:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg8);
         IF AdrMode=ModReg8 THEN
          IF AdrPart<>AccReg THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$ba+HReg; CodeLen:=1;
           END;
        END;
       ModDisp:
        BEGIN
         BAsmCode[1]:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModReg8);
         IF AdrMode=ModReg8 THEN
          IF AdrPart<>AccReg THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$be; CodeLen:=2;
           END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('XCH') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF (NLS_StrCaseCmp(ArgStr[2],'A')=0) OR (NLS_StrCaseCmp(ArgStr[2],'RP1')=0) THEN
        BEGIN
         ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
        END;
       DecodeAdr(ArgStr[1],MModReg8);
       IF AdrMode<>ModNone THEN
        IF AdrPart<>AccReg THEN WrError(1350)
	ELSE
	 BEGIN
          DecodeAdr(ArgStr[2],MModReg8+MModShort+MModSFR+MModAbs+MModIReg+MModIndex+MModDisp);
          CASE AdrMode OF
          ModReg8:
           IF AdrPart=AccReg THEN WrError(1350)
           ELSE
            BEGIN
             BAsmCode[0]:=$30+AdrPart; CodeLen:=1;
            END;
          ModShort:
           BEGIN
            BAsmCode[0]:=$83; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           END;
          ModSFR:
           BEGIN
            BAsmCode[0]:=$93; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           END;
          ModAbs:
           BEGIN
            BAsmCode[0]:=$ce; Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=3;
           END;
          ModIReg:
           BEGIN
            BAsmCode[0]:=$05+(AdrPart SHL 1); CodeLen:=1;
           END;
          ModIndex:
           BEGIN
            BAsmCode[0]:=$31; BAsmCode[1]:=$8a+AdrPart; CodeLen:=2;
           END;
          ModDisp:
           BEGIN
            BAsmCode[0]:=$de; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           END;
          END;
	 END;
      END;
     Exit;
    END;

   IF Memo('MOVW') THEN
    BEGIN
     OpSize:=1;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16+MModShort+MModSFR+MModAbs);
       CASE AdrMode OF
       ModReg16:
        BEGIN
         HReg:=AdrPart;
         IF HReg=AccReg16 THEN DecodeAdr(ArgStr[2],MModReg16+MModImm+MModShort+MModSFR+MModAbs)
         ELSE DecodeAdr(ArgStr[2],MModReg16+MModImm);
         CASE AdrMode OF
         ModReg16:
          IF (HReg=AccReg16)=(AdrPart=AccReg16) THEN WrError(1350)
          ELSE IF HReg=AccReg16 THEN
           BEGIN
            BAsmCode[0]:=$c0+(AdrPart SHL 1); CodeLen:=1;
           END
          ELSE
           BEGIN
            BAsmCode[0]:=$d0+(HReg SHL 1); CodeLen:=1;
           END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$10+(HReg SHL 1); Move(AdrVals,BAsmCode[1],2);
           CodeLen:=3;
          END;
         ModShort:
          BEGIN
           BAsmCode[0]:=$89; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           ChkEven;
          END;
         ModSFR:
          BEGIN
           BAsmCode[0]:=$a9; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           ChkEven;
          END;
         ModAbs:
          BEGIN
           BAsmCode[0]:=$02; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
           ChkEven;
          END;
         END;
        END;
       ModShort:
        BEGIN
         ChkEven;
         BAsmCode[1]:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModReg16+MModImm);
         CASE AdrMode OF
         ModReg16:
          IF AdrPart<>AccReg16 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$99; CodeLen:=2;
           END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$ee; Move(AdrVals,BAsmCode[2],2); CodeLen:=4;
          END;
         END;
        END;
       ModSFR:
        BEGIN
         ChkEven;
         BAsmCode[1]:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModReg16+MModImm);
         CASE AdrMode OF
         ModReg16:
          IF AdrPart<>AccReg16 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$b9; CodeLen:=2;
           END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$fe; Move(AdrVals,BAsmCode[2],2); CodeLen:=4;
          END;
         END;
        END;
       ModAbs:
        BEGIN
         ChkEven;
         Move(AdrVals,BAsmCode[1],AdrCnt);
         DecodeAdr(ArgStr[2],MModReg16);
         IF AdrMode=ModReg16 THEN
          IF AdrPart<>AccReg16 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$03; CodeLen:=3;
           END;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('XCHW')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       IF AdrMode=ModReg16 THEN
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg16);
         IF AdrMode=ModReg16 THEN
          IF (HReg=AccReg16)=(AdrPart=AccReg16) THEN WrError(1350)
          ELSE
	   BEGIN
	    IF HReg=AccReg16 THEN BAsmCode[0]:=$e0+(AdrPart SHL 1)
            ELSE BAsmCode[0]:=$e0+(HReg SHL 1);
            CodeLen:=1;
           END;
        END;
      END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     z:=Ord(Memo('POP'));
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'PSW')=0 THEN
      BEGIN
       BAsmCode[0]:=$22+z; CodeLen:=1;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       IF AdrMode=ModReg16 THEN
        BEGIN
         BAsmCode[0]:=$b1-z+(AdrPart SHL 1); CodeLen:=1;
        END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=0 TO AriOrderCount-1 DO
    IF Memo(AriOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[1],MModReg8+MModShort);
        CASE AdrMode OF
        ModReg8:
         BEGIN
          HReg:=AdrPart;
          IF HReg=AccReg THEN DecodeAdr(ArgStr[2],MModImm+MModReg8+MModShort+MModAbs+MModIReg+MModIndex+MModDisp)
          ELSE DecodeAdr(ArgStr[2],MModReg8);
          CASE AdrMode OF
          ModReg8:
           IF AdrPart=AccReg THEN
	    BEGIN
             BAsmCode[0]:=$61; BAsmCode[1]:=(z SHL 4)+HReg; CodeLen:=2;
            END
           ELSE IF HReg=AccReg THEN
            BEGIN
             BAsmCode[0]:=$61; BAsmCode[1]:=$08+(z SHL 4)+AdrPart; CodeLen:=2;
            END
           ELSE WrError(1350);
          ModImm:
           BEGIN
            BAsmCode[0]:=(z SHL 4)+$0d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           END;
          ModShort:
           BEGIN
            BAsmCode[0]:=(z SHL 4)+$0e; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           END;
          ModAbs:
           BEGIN
            BAsmCode[0]:=(z SHL 4)+$08; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
           END;
          ModIReg:
           IF (AdrPart=0) THEN WrError(1350)
           ELSE
            BEGIN
             BAsmCode[0]:=(z SHL 4)+$0f; CodeLen:=2;
            END;
          ModIndex:
           BEGIN
            BAsmCode[0]:=$31; BAsmCode[1]:=(z SHL 4)+$0a+AdrPart; CodeLen:=2;
           END;
          ModDisp:
           BEGIN
            BAsmCode[0]:=(z SHL 4)+$09; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           END;
          END;
         END;
        ModShort:
         BEGIN
          BAsmCode[1]:=AdrVals[0];
          DecodeAdr(ArgStr[2],MModImm);
          IF AdrMode=ModImm THEN
           BEGIN
            BAsmCode[0]:=(z SHL 4)+$88; BAsmCOde[2]:=AdrVals[0]; CodeLen:=3;
           END;
         END;
        END;
       END;
      Exit;
     END;

   FOR z:=0 TO Ari16OrderCount-1 DO
    IF Memo(Ari16Orders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        OpSize:=1;
        DecodeAdr(ArgStr[1],MModReg16);
        IF AdrMode=ModReg16 THEN
         BEGIN
          DecodeAdr(ArgStr[2],MModImm);
          IF AdrMode=ModImm THEN
           BEGIN
            BAsmCode[0]:=$ca+(z SHL 4); Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
           END;
         END;
       END;
      Exit;
     END;

   IF Memo('MULU') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8);
       IF AdrMode=ModReg8 THEN
        IF AdrPart<>0 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$31; BAsmCode[1]:=$88; CodeLen:=2;
         END;
      END;
     Exit;
    END;

   IF Memo('DIVUW') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8);
       IF AdrMode=ModReg8 THEN
        IF AdrPart<>2 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$31; BAsmCode[1]:=$82; CodeLen:=2;
         END;
      END;
     Exit;
    END;

   IF (Memo('INC')) OR (Memo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       z:=Ord(Memo('DEC')) SHL 4;
       DecodeAdr(ArgStr[1],MModReg8+MModShort);
       CASE AdrMode OF
       ModReg8:
        BEGIN
         BAsmCode[0]:=$40+AdrPart+z; CodeLen:=1;
        END;
       ModShort:
        BEGIN
         BAsmCode[0]:=$81+z; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('INCW')) OR (Memo('DECW')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       IF AdrMode=ModReg16 THEN
        BEGIN
         BAsmCode[0]:=$80+(Ord(Memo('DECW')) SHL 4)+(AdrPart SHL 1);
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO ShiftOrderCount-1 DO
    IF Memo(ShiftOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[1],MModReg8);
        IF AdrMode=ModReg8 THEN
         IF AdrPart<>AccReg THEN WrError(1350)
         ELSE
          BEGIN
           HReg:=EvalIntExpression(ArgStr[2],UInt1,OK);
           IF OK THEN
            IF HReg<>1 THEN WrError(1315)
            ELSE
             BEGIN
              BAsmCode[0]:=$24+z; CodeLen:=1;
             END;
          END;
       END;
      Exit;
     END;

   IF (Memo('ROL4')) OR (Memo('ROR4')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       IF AdrMode=ModIReg THEN
        IF AdrPart=0 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$31; BAsmCode[1]:=$80+(Ord(Memo('ROR4')) SHL 4);
	  CodeLen:=2;
	 END;
      END;
     Exit;
    END;

   { Bitoperationen }

   IF Memo('MOV1') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[2],'CY')=0 THEN
        BEGIN
         ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
         z:=1;
        END
       ELSE z:=4;
       IF NLS_StrCaseCmp(ArgStr[1],'CY')<>0 THEN WrError(1110)
       ELSE IF DecodeBitAdr(ArgStr[2],HReg) THEN
        BEGIN
         BAsmCode[0]:=$61+(Ord(HReg AND $88<>$88) SHL 4);
         BAsmCode[1]:=z+HReg;
         Move(AdrVals,BAsmCode[2],AdrCnt);
         CodeLen:=2+AdrCnt;
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO Bit2OrderCount-1 DO
    IF Memo(Bit2Orders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF NLS_StrCaseCmp(ArgStr[1],'CY')<>0 THEN WrError(1110)
      ELSE IF DecodeBitAdr(ArgStr[2],HReg) THEN
       BEGIN
        BAsmCode[0]:=$61+(Ord(HReg AND $88<>$88) SHL 4);
        BAsmCode[1]:=z+5+HReg;
        Move(AdrVals,BAsmCode[2],AdrCnt);
        CodeLen:=2+AdrCnt;
       END;
      Exit;
     END;

   IF (Memo('SET1')) OR (Memo('CLR1')) THEN
    BEGIN
     z:=Ord(Memo('CLR1'));
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CY')=0 THEN
      BEGIN
       BAsmCode[0]:=$20+z; CodeLen:=1;
      END
     ELSE IF DecodeBitAdr(ArgStr[1],HReg) THEN
      IF HReg AND $88=0 THEN
       BEGIN
        BAsmCode[0]:=$0a+z+(HReg AND $70);
        BAsmCode[1]:=AdrVals[0];
        CodeLen:=2;
       END
      ELSE
       BEGIN
        BAsmCode[0]:=$61+(Ord(HReg AND $88<>$88) SHL 4);
        BAsmCode[1]:=HReg+2+z;
        Move(AdrVals,BAsmCode[2],AdrCnt);
        CodeLen:=2+AdrCnt;
       END;
     Exit;
    END;

   IF Memo('NOT1') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CY')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[0]:=$01;
       CodeLen:=1;
      END;
     Exit;
    END;

   { SprÅnge }

   IF Memo('CALL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAbs);
       IF AdrMode=ModAbs THEN
        BEGIN
         BAsmCode[0]:=$9a; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
        END;
      END;
     Exit;
    END;

   IF Memo('CALLF') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgStr[1][1]='!' THEN Delete(ArgStr[1],1,1);
       AdrWord:=EvalIntExpression(ArgStr[1],UInt11,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$0c+(Hi(AdrWord) SHL 4);
         BAsmCode[1]:=Lo(AdrWord);
         CodeLen:=2;
        END;
      END;
     Exit;
    END;

   IF Memo('CALLT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF (ArgStr[1][1]<>'[') OR (ArgStr[1][Length(ArgStr[1])]<>']') THEN WrError(1350)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrWord:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-2),UInt6,OK);
       IF FirstPassUnknown THEN AdrWord:=AdrWord AND $fffe;
       IF OK THEN
        IF Odd(AdrWord) THEN WrError(1325)
        ELSE
         BEGIN
          BAsmCode[0]:=$c1+(AdrWord AND $3e);
          CodeLen:=1;
         END;
      END;
     Exit;
    END;

   IF (Memo('BR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'AX')=0) OR (NLS_StrCaseCmp(ArgStr[1],'RP0')=0) THEN
      BEGIN
       BAsmCode[0]:=$31; BAsmCode[1]:=$98; CodeLen:=2;
      END
     ELSE
      BEGIN
       IF ArgStr[1][1]='!'  THEN
        BEGIN
         Delete(ArgStr[1],1,1); HReg:=1;
        END
       ELSE IF ArgStr[1][1]='$' THEN
        BEGIN
         Delete(ArgStr[1],1,1); HReg:=2;
        END
       ELSE HReg:=0;
       AdrWord:=EvalIntExpression(ArgStr[1],UInt16,OK);
       IF OK THEN
        BEGIN
         IF HReg=0 THEN
          BEGIN
           AdrInt:=AdrWord-(EProgCounter-2);
           IF (AdrInt>=-128) AND (AdrInt<127) THEN HReg:=2 ELSE HReg:=1;
          END;
         CASE HReg OF
         1:BEGIN
            BAsmCode[0]:=$9b; BAsmCode[1]:=Lo(AdrWord); BAsmCode[2]:=Hi(AdrWord);
            CodeLen:=3;
           END;
         2:BEGIN
            AdrInt:=AdrWord-(EProgCounter+2);
            IF ((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
            ELSE
             BEGIN
              BAsmCode[0]:=$fa; BAsmCode[1]:=AdrInt AND $ff; CodeLen:=2;
             END;
           END;
         END;
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO RelOrderCount-1 DO
    IF Memo(RelOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE
       BEGIN
        IF ArgStr[1][1]='$' THEN Delete(ArgStr[1],1,1);
        AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
        IF OK THEN
         IF ((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
         ELSE
          BEGIN
           BAsmCode[0]:=$8b+(z SHL 4); BAsmCode[1]:=AdrInt AND $ff;
           CodeLen:=2;
          END;
       END;
      Exit;
     END;

   FOR z:=0 TO BRelOrderCount-1 DO
    IF Memo(BRelOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF DecodeBitAdr(ArgStr[1],HReg) THEN
       BEGIN
        IF (z=1) AND (HReg AND $88=0) THEN
         BEGIN
          BAsmCode[0]:=$8c+HReg; BAsmCode[1]:=AdrVals[0]; HReg:=2;
         END
        ELSE
         BEGIN
          BAsmCode[0]:=$31;
          CASE HReg AND $88 OF
          $00:BAsmCode[1]:=$00;
          $08:BAsmCode[1]:=$04;
          $80:BAsmCode[1]:=$84;
          $88:BAsmCode[1]:=$0c;
          END;
          Inc(BAsmCode[1],(HReg AND $70)+z+1);
          BAsmCode[2]:=AdrVals[0];
          HReg:=2+AdrCnt;
         END;
        IF ArgStr[2][1]='$' THEN Delete(ArgStr[2],1,1);
        AdrInt:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+HReg+1);
        IF OK THEN
         IF ((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
         ELSE
          BEGIN
           BAsmCode[HReg]:=AdrInt AND $ff;
           CodeLen:=HReg+1;
          END;
       END;
      Exit;
     END;

   IF Memo('DBNZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModShort);
       IF (AdrMode=ModReg8) AND (AdrPart AND 6<>2) THEN WrError(1350)
       ELSE IF AdrMode<>ModNone THEN
        BEGIN
         IF AdrMode=ModReg8 THEN BAsmCode[0]:=$88+AdrPart ELSE BAsmCode[0]:=$04;
         BAsmCode[1]:=AdrVals[0];
         IF ArgStr[2][1]='$' THEN Delete(ArgStr[2],1,1);
         AdrInt:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+AdrCnt+2);
         IF OK THEN
          IF ((AdrInt<-128) OR (AdrInt>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
          ELSE
           BEGIN
            BAsmCode[AdrCnt+1]:=AdrInt AND $ff;
            CodeLen:=AdrCnt+2;
           END;
	END;
      END;
     Exit;
    END;

   { Steueranweisungen }

   IF Memo('SEL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1350)
     ELSE IF (Length(ArgStr[1])<>3) OR (NLS_StrCaseCmp(Copy(ArgStr[1],1,2),'RB')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       HReg:=Ord(ArgStr[1][3])-AscOfs;
       IF ChkRange(HReg,0,3) THEN
        BEGIN
         BAsmCode[0]:=$61;
         BAsmCode[1]:=$d0+((HReg AND 1) SHL 3)+((HReg AND 2) SHL 4);
         CodeLen:=2;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;


        FUNCTION ChkPC_78K0:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<$10000;
   ELSE ok:=False;
   END;
   ChkPC_78K0:=(ok) AND (ProgCounter>=0);
END;


        FUNCTION IsDef_78K0:Boolean;
	Far;
BEGIN
   IsDef_78K0:=False;
END;

        PROCEDURE SwitchFrom_78K0;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_78K0;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='PC'; HeaderID:=$7c; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_78K0; ChkPC:=ChkPC_78K0; IsDef:=IsDef_78K0;
   SwitchFrom:=SwitchFrom_78K0; InitFields;
END;

BEGIN
   CPU78070:=AddCPU('78070',SwitchTo_78K0);
END.



