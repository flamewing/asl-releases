{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
        Unit CodeH8_3;

{ AS - Codegenerator H8/300(H) }

Interface

        Uses NLS,
             AsmDef,AsmPars,AsmSub,CodePseu;


Implementation

CONST
   FixedOrderCount=4;
   ConditionCount=20;
   ShiftOrderCount=8;
   LogicOrderCount=3;
   MulOrderCount=4;
   Bit1OrderCount=10;
   Bit2OrderCount=4;

   ModNone=-1;
   ModReg=0;        MModReg=1 SHL ModReg;
   ModImm=1;        MModImm=1 SHL ModImm;
   ModAbs8=2;       MModAbs8=1 SHL ModAbs8;
   ModAbs16=3;      MModAbs16=1 SHL ModAbs16;
   ModAbs24=4;      MModAbs24=1 SHL ModAbs24;
   MModAbs=MModAbs8+MModAbs16+MModAbs24;
   ModIReg=5;       MModIReg=1 SHL ModIReg;
   ModPreDec=6;     MModPreDec=1 SHL ModPreDec;
   ModPostInc=7;    MModPostInc=1 SHL ModPostInc;
   ModInd16=8;      MModInd16=1 SHL ModInd16;
   ModInd24=9;      MModInd24=1 SHL ModInd24;
   ModIIAbs=10;     MModIIAbs=1 SHL ModIIAbs;
   MModInd=MModInd16+MModInd24;

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	      END;
   FixedOrderArray=ARRAY[1..FixedOrderCount] OF FixedOrder;
   ShiftOrderArray=ARRAY[1..ShiftOrderCount] OF FixedOrder;
   LogicOrderArray=ARRAY[1..LogicOrderCount] OF FixedOrder;
   MulOrderArray=ARRAY[1..MulOrderCount] OF FixedOrder;
   Bit1OrderArray=ARRAY[1..Bit1OrderCount] OF FixedOrder;
   Bit2OrderArray=ARRAY[1..Bit2OrderCount] OF FixedOrder;

   Condition=RECORD
	      Name:String[3];
	      Code:Byte;
	     END;
   ConditionArray=ARRAY[1..ConditionCount] OF Condition;

VAR
   OpSize:ShortInt;         { Grî·e=8*(2^OpSize) }
   AdrMode:ShortInt; 	    { Ergebnisadre·modus }
   AdrPart:Byte;            { Adressierungsmodusbits im Opcode }
   AdrCnt:Byte;		    { Anzahl Bytes Adressargument }
   AdrVals:ARRAY[0..5] OF Word; { Adre·argument }

   CPUH8_300L:CPUVar;
   CPU6413308,CPUH8_300:CPUVar;
   CPU6413309,CPUH8_300H:CPUVar;
   CPU16:Boolean;	    { keine 32-Bit-Register }

   Conditions:^ConditionArray;
   FixedOrders:^FixedOrderArray;
   ShiftOrders:^ShiftOrderArray;
   LogicOrders:^LogicOrderArray;
   MulOrders:^MulOrderArray;
   Bit1Orders:^Bit1OrderArray;
   Bit2Orders:^Bit2OrderArray;

{---------------------------------------------------------------------------}
{ dynamische Belegung/Freigabe Codetabellen }

	PROCEDURE InitFields;
VAR
   z:Word;

	PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   IF z=FixedOrderCount THEN Halt;
   Inc(z);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddCond(NName:String; NCode:Byte);
BEGIN
   IF z=ConditionCount THEN Halt;
   Inc(z);
   WITH Conditions^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddShift(NName:String; NCode:Word);
BEGIN
   IF z=ShiftOrderCount THEN Halt;
   Inc(z);
   WITH ShiftOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddLogic(NName:String; NCode:Word);
BEGIN
   IF z=LogicOrderCount THEN Halt;
   Inc(z);
   WITH LogicOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddMul(NName:String; NCode:Word);
BEGIN
   IF z=MulOrderCount THEN Halt;
   Inc(z);
   WITH MulOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddBit1(NName:String; NCode:Word);
BEGIN
   IF z=Bit1OrderCount THEN Halt;
   Inc(z);
   WITH Bit1Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddBit2(NName:String; NCode:Word);
BEGIN
   IF z=Bit2OrderCount THEN Halt;
   Inc(z);
   WITH Bit2Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('NOP',$0000); AddFixed('RTE',$5670);
   AddFixed('RTS',$5470); AddFixed('SLEEP',$0180);

   New(Conditions); z:=0;
   AddCond('BRA',$0); AddCond('BT' ,$0);
   AddCond('BRN',$1); AddCond('BF' ,$1);
   AddCond('BHI',$2); AddCond('BLS',$3);
   AddCond('BCC',$4); AddCond('BHS',$4);
   AddCond('BCS',$5); AddCond('BLO',$5);
   AddCond('BNE',$6); AddCond('BEQ',$7);
   AddCond('BVC',$8); AddCond('BVS',$9);
   AddCond('BPL',$a); AddCond('BMI',$b);
   AddCond('BGE',$c); AddCond('BLT',$d);
   AddCond('BGT',$e); AddCond('BLE',$f);

   New(ShiftOrders); z:=0;
   AddShift('ROTL' ,$1280); AddShift('ROTR' ,$1380);
   AddShift('ROTXL',$1200); AddShift('ROTXR',$1300);
   AddShift('SHAL' ,$1080); AddShift('SHAR' ,$1180);
   AddShift('SHLL' ,$1000); AddShift('SHLR' ,$1100);

   New(LogicOrders); z:=0;
   AddLogic('OR',0); AddLogic('XOR',1); AddLogic('AND',2);

   New(MulOrders); z:=0;
   AddMul('DIVXS',3); AddMul('DIVXU',1); AddMul('MULXS',2); AddMul('MULXU',0);

   New(Bit1Orders); z:=0;
   AddBit1('BAND',$16); AddBit1('BIAND',$96);
   AddBit1('BOR' ,$14); AddBit1('BIOR' ,$94);
   AddBit1('BXOR',$15); AddBit1('BIXOR',$95);
   AddBit1('BLD' ,$17); AddBit1('BILD' ,$97);
   AddBit1('BST' ,$07); AddBit1('BIST' ,$87);

   New(Bit2Orders); z:=0;
   AddBit2('BCLR',2); AddBit2('BNOT',1); AddBit2('BSET',0); AddBit2('BTST',3);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(Conditions);
   Dispose(ShiftOrders);
   Dispose(LogicOrders);
   Dispose(MulOrders);
   Dispose(Bit1Orders);
   Dispose(Bit2Orders);
END;

{---------------------------------------------------------------------------}
{ Adre·parsing }

	PROCEDURE SetOpSize(Size:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=Size
   ELSE IF Size<>OpSize THEN
    BEGIN
     WrError(1131); AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

	FUNCTION DecodeReg(Asc:String; VAR Erg:Byte; VAR Size:ShortInt):Boolean;

	FUNCTION IsNum(Inp:Char):Boolean;
BEGIN
   IF (Inp<'0') AND (Inp>'7') THEN IsNum:=False
   ELSE
    BEGIN
     Erg:=Ord(Inp)-AscOfs; IsNum:=True;
    END;
END;

BEGIN
   IF NLS_StrCaseCmp(Asc,'SP')=0 THEN
    BEGIN
     Erg:=7; IF Maximum THEN Size:=2 ELSE Size:=1; DecodeReg:=True;
    END

   ELSE IF (Length(Asc)=3) AND (UpCase(Asc[1])='R') AND (IsNum(Asc[2])) THEN
    IF UpCase(Asc[3])='L' THEN
     BEGIN
      Inc(Erg,8); Size:=0; DecodeReg:=True;
     END
    ELSE IF UpCase(Asc[3])='H' THEN
     BEGIN
      Size:=0; DecodeReg:=True;
     END
    ELSE DecodeReg:=False

   ELSE IF (Length(Asc)=2) AND (IsNum(Asc[2])) THEN
    IF UpCase(Asc[1])='R' THEN
     BEGIN
      Size:=1; DecodeReg:=True;
     END
    ELSE IF UpCase(Asc[1])='E' THEN
     BEGIN
      Inc(Erg,8); Size:=1; DecodeReg:=True;
     END
    ELSE DecodeReg:=False

   ELSE IF (Length(Asc)=3) AND (UpCase(Asc[1])='E') AND (UpCase(Asc[2])='R') AND (IsNum(Asc[3])) THEN
    BEGIN
     Size:=2; DecodeReg:=True;
    END
   ELSE DecodeReg:=False;
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   Found;
VAR
   HSize:ShortInt;
   HReg:Byte;
   HLong:LongInt;
   HWord:Word;
   OK:Boolean;
   p:Integer;
   DispAcc:LongInt;
   Part:String;
   MomSize:(SizeNone,Size8,Size16,Size24);

        PROCEDURE CutSize(VAR Asc:String);
BEGIN
   IF (Length(Asc)>=2)
   AND (Asc[Length(Asc)-1]=':')
   AND (Asc[Length(Asc)]='8') THEN
    BEGIN
     Asc:=Copy(Asc,1,Length(Asc)-2); MomSize:=Size8;
    END
   ELSE
   IF (Length(Asc)>=3)
   AND (Asc[Length(Asc)-2]=':') THEN
    BEGIN
     IF (Asc[Length(Asc)-1]='1')
     AND (Asc[Length(Asc)]='6') THEN
      BEGIN
       Asc:=Copy(Asc,1,Length(Asc)-3); MomSize:=Size16;
      END
     ELSE
     IF (Asc[Length(Asc)-1]='2')
     AND (Asc[Length(Asc)]='4') THEN
      BEGIN
       Asc:=Copy(Asc,1,Length(Asc)-3); MomSize:=Size24;
      END
    END;
END;

	FUNCTION DecodeBaseReg(Asc:String; VAR Erg:Byte):Byte;
VAR
   HSize:ShortInt;
BEGIN
   DecodeBaseReg:=0; IF NOT DecodeReg(Asc,Erg,HSize) THEN Exit;
   DecodeBaseReg:=1;
   IF (HSize=0) OR ((HSize=1) AND (Erg>7)) THEN
    BEGIN
     WrError(1350); Exit;
    END;
   IF (CPU16) XOR (HSize=1) THEN
    BEGIN
     WrError(1505); Exit;
    END;
   DecodeBaseReg:=2;
END;

	PROCEDURE DecideVAbsolute(Address:LongInt);

        FUNCTION Is8:Boolean;
BEGIN
   IF CPU16 THEN Is8:=(Address SHR 8=$ff)
   ELSE Is8:=(Address SHR 8=$ffff);
END;

        FUNCTION Is16:Boolean;
BEGIN
   IF CPU16 THEN Is16:=True
   ELSE Is16:=((Address>=0) AND (Address<=$7fff)) OR ((Address>=$ff8000) AND (Address<=$ffffff))
END;

BEGIN
   { bei Automatik Operandengrî·e festlegen }

   IF MomSize=SizeNone THEN
    BEGIN
     IF Is8 THEN MomSize:=Size8
     ELSE IF Is16 THEN MomSize:=Size16
     ELSE MomSize:=Size24;
    END;

   { wenn nicht vorhanden, eins rauf }

   IF (MomSize=Size8)  AND (Mask AND MModAbs8=0)  THEN MomSize:=Size16;
   IF (MomSize=Size16) AND (Mask AND MModAbs16=0) THEN MomSize:=Size24;

   { entsprechend Modus Bytes ablegen }

   CASE MomSize OF
   Size8:
    IF NOT Is8 THEN WrError(1925)
    ELSE
     BEGIN
      AdrCnt:=2; AdrVals[0]:=Address AND $ff; AdrMode:=ModAbs8;
     END;
   Size16:
    IF NOT Is16 THEN WrError(1925)
    ELSE
     BEGIN
      AdrCnt:=2; AdrVals[0]:=Address AND $ffff; AdrMode:=ModAbs16;
     END;
   Size24:
    BEGIN
     AdrCnt:=4; AdrVals[1]:=Address AND $ffff;
     AdrVals[0]:=Lo(Address SHR 16); AdrMode:=ModAbs24;
    END;
   END;
END;

	PROCEDURE DecideAbsolute(VAR Asc:String);
VAR
   OK:Boolean;
   Addr:LongInt;
BEGIN
   Addr:=EvalIntExpression(Asc,Int32,OK);
   IF OK THEN DecideVAbsolute(Addr);
END;

BEGIN
   AdrMode:=ModNone; AdrCnt:=0; MomSize:=SizeNone;

   { immediate ? }

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OPSize OF
     -1:WrError(1132);
     0:BEGIN
	HReg:=EvalIntExpression(Asc,Int8,OK);
	IF OK THEN
	 BEGIN
	  AdrCnt:=2; AdrVals[0]:=HReg; AdrMode:=ModImm;
	 END;
       END;
     1:BEGIN
	AdrVals[0]:=EvalIntExpression(Asc,Int16,OK);
	IF OK THEN
	 BEGIN
	  AdrCnt:=2; AdrMode:=ModImm;
	 END;
       END;
     2:BEGIN
	HLong:=EvalIntExpression(Asc,Int32,OK);
	IF OK THEN
	 BEGIN
	  AdrCnt:=4;
	  AdrVals[0]:=HLong SHR 16;
	  AdrVals[1]:=HLong AND $ffff;
	  AdrMode:=ModImm;
	 END;
       END;
     ELSE WrError(1130);
     END;
     Goto Found;
    END;

   { Register ? }

   IF DecodeReg(Asc,HReg,HSize) THEN
    BEGIN
     AdrMode:=ModReg; AdrPart:=HReg; SetOpSize(HSize); Goto Found;
    END;

   { indirekt ? }

   IF Asc[1]='@' THEN
    BEGIN
     Delete(Asc,1,1);

     IF Asc[1]='@' THEN
      BEGIN
       AdrVals[0]:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),UInt8,OK) AND $ff;
       IF OK THEN
	BEGIN
	 AdrCnt:=1; AdrMode:=ModIIAbs;
	END;
       GOTO Found;
      END;

     CASE DecodeBaseReg(Asc,AdrPart) OF
     1:GOTO Found;
     2:BEGIN
	AdrMode:=ModIReg; GOTO Found;
       END;
     END;

     IF Asc[1]='-' THEN
      CASE DecodeBaseReg(Copy(Asc,2,Length(Asc)-1),AdrPart) OF
      1:GOTO Found;
      2:BEGIN
	 AdrMode:=ModPreDec; Goto Found;
	END;
      END;

     IF Asc[Length(Asc)]='+' THEN
      CASE DecodeBaseReg(Copy(Asc,1,Length(Asc)-1),AdrPart) OF
      1:GOTO Found;
      2:BEGIN
	 AdrMode:=ModPostInc; Goto Found;
	END;
      END;

     IF IsIndirect(Asc) THEN
      BEGIN
       Asc:=Copy(Asc,2,Length(Asc)-2);
       AdrPart:=$ff; DispAcc:=0;
       REPEAT
	p:=QuotPos(Asc,',');
	SplitString(Asc,Part,Asc,p);
	CASE DecodeBaseReg(Part,HReg) OF
	2:IF AdrPart<>$ff THEN
	   BEGIN
	    WrError(1350); Goto Found;
	   END
	  ELSE AdrPart:=HReg;
	1:GOTO Found;
	0:BEGIN
           CutSize(Part);
	   Inc(DispAcc,EvalIntExpression(Part,Int32,OK));
	   IF NOT OK THEN Goto Found;
	  END;
	END;
       UNTIL Asc='';
       IF AdrPart=$ff THEN DecideVAbsolute(DispAcc)
       ELSE
        BEGIN
         IF (CPU16) AND (DispAcc AND $ffff8000=$8000) THEN Inc(DispAcc,$ffff0000);
         IF MomSize=SizeNone THEN
          IF (DispAcc>=-32768) AND (DispAcc<=32767) THEN MomSize:=Size16
          ELSE MomSize:=Size24;
         CASE MomSize OF
         Size8:WrError(1130);
         Size16:
	  IF (DispAcc<-32768) THEN WrError(1315)
	  ELSE IF (DispAcc>32767) THEN WrError(1320)
          ELSE
	   BEGIN
	    AdrCnt:=2; AdrVals[0]:=DispAcc AND $ffff; AdrMode:=ModInd16;
	   END;
         Size24:
	  BEGIN
	   AdrVals[1]:=DispAcc AND $ffff; AdrVals[0]:=Lo(DispAcc SHR 16);
	   AdrCnt:=4; AdrMode:=ModInd24;
	  END;
         END;
        END
      END
     ELSE
      BEGIN
       CutSize(Asc);
       DecideAbsolute(Asc);
      END;
     Goto Found;
    END;

   CutSize(Asc);
   DecideAbsolute(Asc);

Found:
   IF CPU16 THEN
    IF ((AdrMode=ModReg) AND (OpSize=2))
    OR ((AdrMode=ModReg) AND (OpSize=1) AND (AdrPart>7))
    OR (AdrMode=ModAbs24) OR (AdrMode=ModInd24) THEN
     BEGIN
      WrError(1505); AdrMode:=ModNone; AdrCnt:=0;
     END;
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     WrError(1350); AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

	FUNCTION ImmVal:LongInt;
BEGIN
   CASE OpSize OF
   0:ImmVal:=Lo(AdrVals[0]);
   1:ImmVal:=AdrVals[0];
   2:ImmVal:=(LongInt(AdrVals[0]) SHL 16)+AdrVals[1];
   END;
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFFH8_3Count=1;
   ONOFFH8_3s:ARRAY[1..ONOFFH8_3Count] OF ONOFFRec=
              ((Name:'MAXMODE'; Dest:@Maximum   ; FlagName:MaximumName   ));
BEGIN
   DecodePseudo:=True;

   IF CodeONOFF(@ONOFFH8_3s,ONOFFH8_3Count) THEN Exit;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_H8_3;
	Far;
VAR
   z:Integer;
   Mask:Word;
   HSize:ShortInt;
   AdrLong:LongInt;
   HReg,Opcode:Byte;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { Attribut verwursten }

   IF AttrPart<>'' THEN
    BEGIN
     IF Length(AttrPart)<>1 THEN
      BEGIN
       WrError(1105); Exit;
      END;
     CASE UpCase(AttrPart[1]) OF
     'B':SetOpSize(0);
     'W':SetOpSize(1);
     'L':SetOpSize(2);
     'Q':SetOpSize(3);
     'S':SetOpSize(4);
     'D':SetOpSize(5);
     'X':SetOpSize(6);
     'P':SetOpSize(7);
     ELSE
      BEGIN
       WrError(1107); Exit;
      END;
     END;
    END;

   IF DecodeMoto16Pseudo(OpSize,True) THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
	BEGIN
	 CodeLen:=2; WAsmCode[0]:=Code;
	END;
       Exit;
      END;

   IF Memo('EEPMOV') THEN
    BEGIN
     IF OpSize=-1 THEN OpSize:=Ord(NOT CPU16);
     IF OpSize>1 THEN WrError(1130)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF (OpSize=1) AND (CPU16) THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=4;
       IF OpSize=0 THEN WAsmCode[0]:=$7b5c ELSE WAsmCode[0]:=$7bd4;
       WAsmCode[1]:=$598f;
      END;
     Exit;
    END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPreDec+MModInd+MModAbs);
       CASE AdrMode OF
       ModReg:
	BEGIN
	 HReg:=AdrPart;
	 Mask:=MModReg+MModIReg+MModPostInc+MModInd+MModAbs+MModImm;
	 IF OpSize<>0 THEN Dec(Mask,MModAbs8);
	 DecodeAdr(ArgStr[1],Mask);
	 CASE AdrMode OF
	 ModReg:
	  BEGIN
	   z:=OpSize; IF z=2 THEN z:=3;
	   CodeLen:=2; WAsmCode[0]:=$0c00+(z SHL 8)+(AdrPart SHL 4)+HReg;
	   IF OpSize=2 THEN Inc(WAsmCode[0],$80);
	  END;
	 ModIReg:
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$6800+(AdrPart SHL 4)+HReg;
	    END;
	  1:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$6900+(AdrPart SHL 4)+HReg;
	    END;
	  2:BEGIN
             CodeLen:=4; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$6900+(AdrPart SHL 4)+HReg;
	    END;
	  END;
	 ModPostInc:
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$6c00+(AdrPart SHL 4)+HReg;
	    END;
	  1:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$6d00+(AdrPart SHL 4)+HReg;
	    END;
	  2:BEGIN
             CodeLen:=4; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$6d00+(AdrPart SHL 4)+HReg;
	    END;
	  END;
	 ModInd16:
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$6e00+(AdrPart SHL 4)+HReg;
	     WAsmCode[1]:=AdrVals[0];
	    END;
	  1:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$6f00+(AdrPart SHL 4)+HReg;
	     WAsmCode[1]:=AdrVals[0];
	    END;
	  2:BEGIN
             CodeLen:=6; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$6f00+(AdrPart SHL 4)+HReg;
	     WAsmCode[2]:=AdrVals[0];
	    END;
	  END;
	 ModInd24:
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=8;
	     WAsmCode[0]:=$7800+(AdrPart SHL 4);
	     WAsmCode[1]:=$6a20+HReg;
	     Move(AdrVals,WAsmCode[2],AdrCnt);
	    END;
	  1:BEGIN
	     CodeLen:=8;
	     WAsmCode[0]:=$7800+(AdrPart SHL 4);
	     WAsmCode[1]:=$6b20+HReg;
	     Move(AdrVals,WAsmCode[2],AdrCnt);
	    END;
	  2:BEGIN
             CodeLen:=10; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$7800+(AdrPart SHL 4);
	     WAsmCode[2]:=$6b20+HReg;
	     Move(AdrVals,WAsmCode[3],AdrCnt);
	    END;
	  END;
	 ModAbs8:
	  BEGIN
	   CodeLen:=2; WAsmCode[0]:=$2000+(Word(HReg) SHL 8)+Lo(AdrVals[0]);
	  END;
	 ModAbs16:
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$6a00+HReg;
	     WAsmCode[1]:=AdrVals[0];
	    END;
	  1:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$6b00+HReg;
	     WAsmCode[1]:=AdrVals[0];
	    END;
	  2:BEGIN
             CodeLen:=6; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$6b00+HReg;
	     WAsmCode[2]:=AdrVals[0];
	    END;
	  END;
	 ModAbs24:
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=6; WAsmCode[0]:=$6a20+HReg;
	     Move(AdrVals,WAsmCode[1],AdrCnt);
	    END;
	  1:BEGIN
	     CodeLen:=6; WAsmCode[0]:=$6b20+HReg;
	     Move(AdrVals,WAsmCode[1],AdrCnt);
	    END;
	  2:BEGIN
             CodeLen:=8; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$6b20+HReg;
	     Move(AdrVals,WAsmCode[2],AdrCnt);
	    END;
	  END;
	 ModImm:
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$f000+(Word(HReg) SHL 8)+Lo(AdrVals[0]);
	    END;
	  1:BEGIN
             CodeLen:=4; WAsmCode[0]:=$7900+HReg; WAsmCode[1]:=AdrVals[0];
	    END;
	  2:BEGIN
             CodeLen:=6; WAsmCode[0]:=$7a00+HReg;
             Move(AdrVals,WAsmCode[1],AdrCnt);
	    END;
	  END;
	 END;
	END;
       ModIReg:
	BEGIN
	 HReg:=AdrPart;
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrMode<>ModNone THEN
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$6880+(HReg SHL 4)+AdrPart;
	    END;
	  1:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$6980+(HReg SHL 4)+AdrPart;
	    END;
	  2:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$6980+(HReg SHL 4)+AdrPart;
	    END;
	  END;
	END;
       ModPreDec:
	BEGIN
	 HReg:=AdrPart;
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrMode<>ModNone THEN
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$6c80+(HReg SHL 4)+AdrPart;
	    END;
	  1:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$6d80+(HReg SHL 4)+AdrPart;
	    END;
	  2:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$6d80+(HReg SHL 4)+AdrPart;
	    END;
	  END;
	END;
       ModInd16:
	BEGIN
	 HReg:=AdrPart; WAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrMode<>ModNone THEN
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$6e80+(HReg SHL 4)+AdrPart;
	    END;
	  1:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$6f80+(HReg SHL 4)+AdrPart;
	    END;
	  2:BEGIN
	     CodeLen:=6; WAsmCode[0]:=$0100; WAsmCode[2]:=WAsmCode[1];
	     WAsmCode[1]:=$6f80+(HReg SHL 4)+AdrPart;
	    END;
	  END;
	END;
       ModInd24:
	BEGIN
	 HReg:=AdrPart; Move(AdrVals,WAsmCode[2],4);
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrMode<>ModNone THEN
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=8; WAsmCode[0]:=$7800+(HReg SHL 4);
	     WAsmCode[1]:=$6aa0+AdrPart;
	    END;
	  1:BEGIN
	     CodeLen:=8; WAsmCode[0]:=$7800+(HReg SHL 4);
	     WAsmCode[1]:=$6ba0+AdrPart;
	    END;
	  2:BEGIN
	     CodeLen:=10; WAsmCode[0]:=$0100;
	     WAsmCode[4]:=WAsmCode[3]; WAsmCode[3]:=WAsmCode[2];
	     WAsmCode[1]:=$7800+(HReg SHL 4);
	     WAsmCode[2]:=$6ba0+AdrPart;
	    END;
	  END;
	END;
       ModAbs8:
	BEGIN
	 HReg:=Lo(AdrVals[0]);
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrMode<>ModNone THEN
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=2; WAsmCode[0]:=$3000+(Word(AdrPart) SHL 8)+HReg;
	    END;
	  1:BEGIN
	     CodeLen:=4;
	     WAsmCode[0]:=$6b80+AdrPart; WAsmCode[1]:=$ff00+HReg;
	    END;
	  2:BEGIN
	     CodeLen:=6; WAsmCode[0]:=$0100;
	     WAsmCode[1]:=$6b80+AdrPart; WAsmCode[2]:=$ff00+HReg;
	    END;
	  END;
	END;
       ModAbs16:
	BEGIN
	 WAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrMode<>ModNone THEN
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$6a80+AdrPart;
	    END;
	  1:BEGIN
	     CodeLen:=4; WAsmCode[0]:=$6b80+AdrPart;
	    END;
	  2:BEGIN
	     CodeLen:=6; WAsmCode[0]:=$0100; WAsmCode[2]:=WAsmCode[1];
	     WAsmCode[1]:=$6b80+AdrPart;
	    END;
	  END;
	END;
       ModAbs24:
	BEGIN
	 Move(AdrVals,WAsmCode[1],4);
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrMode<>ModNone THEN
	  CASE OpSize OF
	  0:BEGIN
	     CodeLen:=6; WAsmCode[0]:=$6aa0+AdrPart;
	    END;
	  1:BEGIN
	     CodeLen:=6; WAsmCode[0]:=$6ba0+AdrPart;
	    END;
	  2:BEGIN
	     CodeLen:=8; WAsmCode[0]:=$0100;
	     WAsmCode[3]:=WAsmCode[2]; WAsmCode[2]:=WAsmCode[1];
	     WAsmCode[1]:=$6ba0+AdrPart;
	    END;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF (Memo('MOVTPE')) OR (Memo('MOVFPE')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<=CPUH8_300L THEN WrError(1500)
     ELSE
      BEGIN
       IF Memo('MOVTPE') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1]; ArgStr[1]:=ArgStr[3];
	END;
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrMode<>ModNone THEN
	IF OpSize<>0 THEN WrError(1130)
	ELSE
	 BEGIN
	  HReg:=AdrPart; DecodeAdr(ArgStr[1],MModAbs16);
	  IF AdrMode<>ModNone THEN
	   BEGIN
	    CodeLen:=4; WAsmCode[0]:=$6a40+HReg; WAsmCode[1]:=AdrVals[0];
	    IF Memo('MOVTPE') THEN Inc(WAsmCode[0],$80);
	   END;
	 END;
      END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     IF Argcnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       z:=Ord(Memo('PUSH'));
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode<>ModNone THEN
	IF OpSize=0 THEN WrError(1130)
	ELSE IF (CPU16) AND (OpSize=2) THEN WrError(1500)
	ELSE
	 BEGIN
	  IF OpSize=2 THEN WAsmCode[0]:=$0100;
	  CodeLen:=2*OpSize;
	  WAsmCode[(CodeLen-2) SHR 1]:=$6d70+(z SHL 7)+AdrPart;
	 END;
      END;
     Exit;
    END;

   IF (Memo('LDC')) OR (Memo('STC')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('STC') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	 z:=$80;
	END
       ELSE z:=0;
       IF NLS_StrCaseCmp(ArgStr[2],'CCR')<>0 THEN WrError(1350)
       ELSE
	BEGIN
	 SetOpSize(0);
	 Mask:=MModReg+MModIReg+MModInd+MModAbs16+MModAbs24;
	 IF Memo('LDC') THEN Inc(Mask,MModImm+MModPostInc)
	 ELSE Inc(Mask,MModPreDec);;
	 DecodeAdr(ArgStr[1],Mask);
	 CASE AdrMode OF
	 ModReg:
	  BEGIN
	   CodeLen:=2;
	   WAsmCode[0]:=$0300+AdrPart-(z SHL 1);
	  END;
	 ModIReg:
	  BEGIN
	   CodeLen:=4; WAsmCode[0]:=$0140;
	   WAsmCode[1]:=$6900+z+(AdrPart SHL 4);
	  END;
	 ModPostInc,ModPreDec:
	  BEGIN
	   CodeLen:=4; WAsmCode[0]:=$0140;
	   WAsmCode[1]:=$6d00+z+(AdrPart SHL 4);
	  END;
	 ModInd16:
	  BEGIN
	   CodeLen:=6; WAsmCode[0]:=$0140; WAsmCode[2]:=AdrVals[0];
	   WAsmCode[1]:=$6f00+z+(AdrPart SHL 4);
	  END;
	 ModInd24:
	  BEGIN
	   CodeLen:=10; WAsmCode[0]:=$0140; WAsmCode[1]:=$7800+(AdrPart SHL 4);
	   WAsmCode[2]:=$6b20+z; Move(AdrVals,WAsmCode[3],AdrCnt);
	  END;
	 ModAbs16:
	  BEGIN
	   CodeLen:=6; WAsmCode[0]:=$0140; WAsmCode[2]:=AdrVals[0];
	   WAsmCode[1]:=$6b00+z;
	  END;
	 ModAbs24:
	  BEGIN
	   CodeLen:=8; WAsmCode[0]:=$0140; 
	   WAsmCode[1]:=$6b20+z; Move(AdrVals,WAsmCode[2],AdrCnt);
	  END;
	 ModImm:
	  BEGIN
	   CodeLen:=2; WAsmCode[0]:=$0700+Lo(AdrVals[0]);
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   { Arithmetik mit 2 Operanden }

   IF (Memo('ADD')) OR (Memo('SUB')) THEN
    BEGIN
     z:=Ord(Memo('SUB'));
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrMode<>ModNone THEN
	BEGIN
	 HReg:=AdrPart;
	 DecodeAdr(ArgStr[1],MModReg+MModImm);
	 IF AdrMode<>ModNone THEN
	  IF (CPU16) AND ((OpSize>1) OR ((OpSize=1) AND (AdrMode=ModImm))) THEN WrError(1500)
	  ELSE CASE AdrMode OF
	  ModImm:
	   CASE OpSize OF
	   0:IF z=1 THEN WrError(1350)
	     ELSE
	      BEGIN
	       CodeLen:=2; WAsmCode[0]:=$8000+(Word(HReg) SHL 8)+Lo(AdrVals[0]);
	      END;
	   1:BEGIN
	      CodeLen:=4; WAsmCode[1]:=AdrVals[0];
	      WAsmCode[0]:=$7910+(z SHL 5)+HReg;
	     END;
	   2:BEGIN
	      CodeLen:=6; Move(AdrVals,WAsmCode[1],4);
	      WAsmCode[0]:=$7a10+(z SHL 5)+HReg;
	     END;
	   END;
	  ModReg:
	   CASE OpSize OF
	   0:BEGIN
	      CodeLen:=2; WAsmCode[0]:=$0800+(z SHL 12)+(AdrPart SHL 4)+HReg;
	     END;
	   1:BEGIN
	      CodeLen:=2; WAsmCode[0]:=$0900+(z SHL 12)+(AdrPart SHL 4)+HReg;
	     END;
	   2:BEGIN
              CodeLen:=2; WAsmCode[0]:=$0a00+(z SHL 12)+$80+(AdrPart SHL 4)+HReg;
	     END;
	   END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('CMP') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrMode<>ModNone THEN
	BEGIN
	 HReg:=AdrPart;
	 DecodeAdr(ArgStr[1],MModReg+MModImm);
	 IF AdrMode<>ModNone THEN
	  IF (CPU16) AND ((OpSize>1) OR ((OpSize=1) AND (AdrMode=ModImm))) THEN WrError(1500)
	  ELSE CASE AdrMode OF
	  ModImm:
	   CASE OpSize OF
	   0:BEGIN
	      CodeLen:=2; WAsmCode[0]:=$a000+(Word(HReg) SHL 8)+Lo(AdrVals[0]);
	     END;
	   1:BEGIN
	      CodeLen:=4; WAsmCode[1]:=AdrVals[0];
	      WAsmCode[0]:=$7920+HReg;
	     END;
	   2:BEGIN
	      CodeLen:=6; Move(AdrVals,WAsmCode[1],4);
	      WAsmCode[0]:=$7a20+HReg;
	     END;
	   END;
	  ModReg:
	   CASE OpSize OF
	   0:BEGIN
	      CodeLen:=2; WAsmCode[0]:=$1c00+(AdrPart SHL 4)+HReg;
	     END;
	   1:BEGIN
	      CodeLen:=2; WAsmCode[0]:=$1d00+(AdrPart SHL 4)+HReg;
	     END;
	   2:BEGIN
	      CodeLen:=2; WAsmCode[0]:=$1f80+(AdrPart SHL 4)+HReg;
	     END;
	   END;
	  END;
	END;
      END;
     Exit;
    END;

   FOR z:=1 TO LogicOrderCount DO
    WITH LogicOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
	 DecodeAdr(ArgStr[2],MModReg);
	 IF AdrMode<>ModNone THEN
          IF (CPU16) AND (OpSize>0) THEN WrError(1500)
          ELSE
           BEGIN
	    HReg:=AdrPart; DecodeAdr(ArgStr[1],MModImm+MModReg);
            CASE AdrMode OF
            ModImm:
             CASE OpSize OF
             0:BEGIN
                CodeLen:=2;
		WAsmCode[0]:=$c000+(Word(Code) SHL 12)+(Word(HReg) SHL 8)+Lo(AdrVals[0]);
               END;
             1:BEGIN
                CodeLen:=4; WAsmCode[1]:=AdrVals[0];
                WAsmCode[0]:=$7940+(Word(Code) SHL 4)+HReg;
               END;
             2:BEGIN
                CodeLen:=6; Move(AdrVals,WAsmCode[1],AdrCnt);
                WAsmCode[0]:=$7a40+(Word(Code) SHL 4)+HReg;
               END;
	     END;
	    ModReg:
             CASE OpSize OF
             0:BEGIN
	        CodeLen:=2; WAsmCode[0]:=$1400+(Word(Code) SHL 8)+AdrPart SHL 4+HReg;
	       END;
             1:BEGIN
	        CodeLen:=2; WAsmCode[0]:=$6400+(Word(Code) SHL 8)+AdrPart SHL 4+HReg;
               END;
             2:BEGIN
	        CodeLen:=4; WAsmCode[0]:=$01f0;
		WAsmCode[1]:=$6400+(Word(Code) SHL 8)+AdrPart SHL 4+HReg;
               END;
             END;
            END;
           END;
        END;
       Exit;
      END
     ELSE IF Memo(Name+'C') THEN
      BEGIN
       SetOpSize(0);
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NLS_StrCaseCmp(ArgStr[2],'CCR')<>0 THEN WrError(1350)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModImm);
	 IF AdrMode<>ModNone THEN
          BEGIN
           CodeLen:=2;
           WAsmCode[0]:=$0400+(Word(Code) SHL 8)+Lo(AdrVals[0]);
          END;
        END;
       Exit;
      END;

   IF (Memo('ADDX')) OR (Memo('SUBX')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrMode<>ModNone THEN
	IF OpSize<>0 THEN WrError(1130)
	ELSE
         BEGIN
          HReg:=AdrPart;
          DecodeAdr(ArgStr[1],MModImm+MModReg);
	  CASE AdrMode OF
          ModImm:
           BEGIN
            CodeLen:=2; WAsmCode[0]:=$9000+(Word(HReg) SHL 8)+Lo(AdrVals[0]);
            IF Memo('SUBX') THEN Inc(WAsmCode[0],$2000);
           END;
          ModReg:
           BEGIN
            CodeLen:=2; WAsmCode[0]:=$0e00+(AdrPart SHL 4)+HReg;
            IF Memo('SUBX') THEN Inc(WAsmCode[0],$1000);
           END;
          END;
         END;
      END;
     Exit;
    END;

   IF (Memo('ADDS')) OR (Memo('SUBS')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrMode<>ModNone THEN
        IF ((CPU16) AND (OpSize<>1)) OR ((NOT CPU16) AND (OpSize<>2)) THEN WrError(1130)
        ELSE
         BEGIN
          HReg:=AdrPart;
          DecodeAdr(ArgStr[1],MModImm);
          IF AdrMode<>ModNone THEN
           BEGIN
            AdrLong:=ImmVal;
            IF (AdrLong<>1) AND (AdrLong<>2) AND (AdrLong<>4) THEN WrError(1320)
            ELSE
             BEGIN
              CASE AdrLong OF
              1:WAsmCode[0]:=$0b00;
	      2:WAsmCode[0]:=$0b80;
	      4:WAsmCode[0]:=$0b90;
              END;
              CodeLen:=2; Inc(WAsmCode[0],HReg);
              IF Memo('SUBS') THEN Inc(WAsmCode[0],$1000);
	     END;
           END;
         END;
      END;
     Exit;
    END;

   FOR z:=1 TO MulOrderCount DO
    WITH MulOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         IF OpSize<>-1 THEN Inc(OpSize);
         DecodeAdr(ArgStr[2],MModReg);
	 IF AdrMode<>ModNone THEN
	  IF (OpSize=0) THEN WrError(1130)
	  ELSE IF (CPU16) AND (OpSize=2) THEN WrError(1500)
          ELSE
           BEGIN
	    HReg:=AdrPart; Dec(OpSize);
            DecodeAdr(ArgStr[1],MModReg);
            IF AdrMode<>ModNone THEN
             BEGIN
              IF Code AND 2=2 THEN
               BEGIN
	        CodeLen:=4; WAsmCode[0]:=$01c0;
		IF Code AND 1=1 THEN Inc(WAsmCode[0],$10);
	       END
	      ELSE CodeLen:=2;
              WAsmCode[CodeLen SHR 2]:=$5000
	                              +(Word(OpSize) SHL 9)
				      +(Word(Code AND 1) SHL 8)
				      +(AdrPart SHL 4)+HReg;
             END;
           END;
	END;
       Exit;
      END;

   { Bitoperationen }

   FOR z:=1 TO Bit1OrderCount DO
    WITH Bit1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       OpCode:=$60+(Code AND $7f);
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 IF ArgStr[1][1]<>'#' THEN WrError(1350)
	 ELSE
	  BEGIN
	   HReg:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt3,OK);
	   IF OK THEN
	    BEGIN
	     DecodeAdr(ArgStr[2],MModReg+MModIReg+MModAbs8);
	     IF AdrMode<>ModNone THEN
	      IF OpSize>0 THEN WrError(1130)
	      ELSE CASE AdrMode OF
	      ModReg:
	       BEGIN
		CodeLen:=2;
		WAsmCode[0]:=(Word(OpCode) SHL 8)+(Code AND $80)+(HReg SHL 4)+AdrPart;
	       END;
	      ModIReg:
	       BEGIN
		CodeLen:=4;
		WAsmCode[0]:=$7c00+(AdrPart SHL 4);
		WAsmCode[1]:=(Word(OpCode) SHL 8)+(Code AND $80)+(HReg SHL 4);
		IF OpCode<$70 THEN Inc(WAsmCode[0],$100);
	       END;
	      ModAbs8:
	       BEGIN
		CodeLen:=4;
		WAsmCode[0]:=$7e00+Lo(AdrVals[0]);
		WAsmCode[1]:=(Word(OpCode) SHL 8)+(Code AND $80)+(HReg SHL 4);
		IF OpCode<$70 THEN Inc(WAsmCode[0],$100);
	       END;
	      END;
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO Bit2OrderCount DO
    WITH Bit2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 IF ArgStr[1][1]='#' THEN
	  BEGIN
	   Opcode:=Code+$70;
	   HReg:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt3,OK);
	  END
	 ELSE
	  BEGIN
	   Opcode:=Code+$60;
	   OK:=DecodeReg(ArgStr[1],HReg,HSize);
	   IF NOT OK THEN WrError(1350);
	   IF (OK) AND (HSize<>0) THEN
	    BEGIN
	     WrError(1130); OK:=False;
	    END;
	  END;
	 IF OK THEN
	  BEGIN
	   DecodeAdr(ArgStr[2],MModReg+MModIReg+MModAbs8);
	   IF AdrMode<>ModNone THEN
	    IF OpSize>0 THEN WrError(1130)
	    ELSE CASE AdrMode OF
	    ModReg:
	     BEGIN
	      CodeLen:=2;
	      WAsmCode[0]:=(Word(OpCode) SHL 8)+(HReg SHL 4)+AdrPart;
	     END;
	    ModIReg:
	     BEGIN
	      CodeLen:=4;
	      WAsmCode[0]:=$7d00+(AdrPart SHL 4);
	      WAsmCode[1]:=(Word(OpCode) SHL 8)+(HReg SHL 4);
	      IF Code=3 THEN Dec(WAsmCode[0],$100);
	     END;
	    ModAbs8:
	     BEGIN
	      CodeLen:=4;
	      WAsmCode[0]:=$7f00+Lo(AdrVals[0]);
	      WAsmCode[1]:=(Word(OpCode) SHL 8)+(HReg SHL 4);
	      IF Code=3 THEN Dec(WAsmCode[0],$100);
	     END;
	    END;
	  END;
	END;
       Exit;
      END;

   { Read/Modify/Write-Operationen }

   IF (Memo('INC')) OR (Memo('DEC')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[ArgCnt],MModReg);
       IF AdrMode<>ModNone THEN
	IF (OpSize>0) AND (CPU16) THEN WrError(1500)
	ELSE
	 BEGIN
	  HReg:=AdrPart;
	  IF ArgCnt=1 THEN
	   BEGIN
	    OK:=True; z:=1;
	   END
	  ELSE
	   BEGIN
	    DecodeAdr(ArgStr[1],MModImm);
	    OK:=AdrMode=ModImm;
	    IF OK THEN
	     BEGIN
	      z:=ImmVal;
	      IF z<1 THEN
	       BEGIN
		WrError(1315); OK:=False;
	       END
	      ELSE IF ((OpSize=0) AND (z>1)) OR (z>2) THEN
	       BEGIN
		WrError(1320); OK:=False;
	       END;
	     END;
	   END;
	  IF OK THEN
	   BEGIN
	    CodeLen:=2; Dec(z);
	    CASE OpSize OF
	    0:WAsmCode[0]:=$0a00+HReg;
	    1:WAsmCode[0]:=$0b50+HReg+(z SHL 7);
	    2:WAsmCode[0]:=$0b70+Hreg+(z SHL 7);
	    END;
	    IF Memo('DEC') THEN Inc(WasmCode[0],$1000);
	   END;
	 END;
      END;
     Exit;
    END;

   FOR z:=1 TO ShiftOrderCount DO
    WITH ShiftOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrMode<>ModNone THEN
	  IF (OpSize>0) AND (CPU16) THEN WrError(1500)
	  ELSE
	   BEGIN
	    CodeLen:=2;
	    CASE OpSize OF
	    0:WAsmCode[0]:=Code+AdrPart;
	    1:WAsmCode[0]:=Code+AdrPart+$10;
	    2:WAsmCode[0]:=Code+AdrPart+$30;
	    END;
	   END;
	END;
       Exit;
      END;

   IF (Memo('NEG')) OR (Memo('NOT')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode<>ModNone THEN
	IF (OpSize>0) AND (CPU16) THEN WrError(1500)
	ELSE
	 BEGIN
	  CodeLen:=2;
	  CASE OpSize OF
	  0:WAsmCode[0]:=$1700+AdrPart;
	  1:WAsmCode[0]:=$1710+AdrPart;
	  2:WAsmCode[0]:=$1730+AdrPart;
	  END;
	  IF Memo('NEG') THEN Inc(WAsmCode[0],$80);
	 END;
      END;
     Exit;
    END;

   IF (Memo('EXTS')) OR (Memo('EXTU')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CPU16 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode<>ModNone THEN
	IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
	ELSE
	 BEGIN
	  CodeLen:=2;
	  CASE OpSize OF
	  1:IF Memo('EXTS') THEN WAsmCode[0]:=$17d0 ELSE WAsmCode[0]:=$1750;
	  2:IF Memo('EXTS') THEN WAsmCode[0]:=$17f0 ELSE WAsmCode[0]:=$1770;
	  END;
	  Inc(WAsmCode[0],AdrPart);
	 END;
      END;
     Exit;
    END;

   IF (Memo('DAA')) OR (Memo('DAS')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode<>ModNone THEN
        IF OpSize<>0 THEN WrError(1130)
        ELSE
         BEGIN
          CodeLen:=2;
          WAsmCode[0]:=$0f00+AdrPart;
          IF Memo('DAS') THEN Inc(WAsmCode[0],$1000);
         END;
      END;
     Exit;
    END;

   { SprÅnge }

   FOR z:=1 TO ConditionCount DO
    WITH Conditions^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF (OpSize<>-1) AND (OpSize<>4) AND (OpSize<>2) THEN WrError(1130)
       ELSE
	BEGIN
	 AdrLong:=EvalIntExpression(ArgStr[1],Int24,OK)-(EProgCounter+2);
	 IF OK THEN
	  BEGIN
	   IF OpSize=-1 THEN
	   IF (AdrLong>=-128) AND (AdrLong<=127) THEN OpSize:=4
	   ELSE
	    BEGIN
	     OpSize:=2; Dec(AdrLong,2);
	    END
	   ELSE IF OpSize=2 THEN Dec(AdrLong,2);
	   IF OpSize=2 THEN
	    BEGIN
	     IF (AdrLong<-32768) OR (AdrLong>32767) THEN WrError(1370)
	     ELSE IF CPU16 THEN WrError(1500)
	     ELSE
	      BEGIN
	       CodeLen:=4;
	       WAsmCode[0]:=$5800+Code SHL 4; WAsmCode[1]:=AdrLong AND $ffff;
	      END;
	    END
	   ELSE
	    BEGIN
	     IF (AdrLong<-128) OR (AdrLong>127) THEN WrError(1370)
	     ELSE
	      BEGIN
	       CodeLen:=2;
	       WAsmCode[0]:=$4000+(Word(Code) SHL 8)+(AdrLong AND $ff);
	      END;
	    END;
	  END;
	END;
       Exit;
      END;

   IF (Memo('JMP')) OR (Memo('JSR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       z:=Ord(Memo('JSR')) SHL 10;
       IF CPU16 THEN DecodeAdr(ArgStr[1],MModIReg+MModAbs16+MModIIAbs)
       ELSE DecodeAdr(ArgStr[1],MModIReg+MModAbs24+MModIIAbs);
       CASE AdrMode OF
       ModIReg:
	BEGIN
	 CodeLen:=2; WAsmCode[0]:=$5900+z+(AdrPart SHL 4);
	END;
       ModAbs16:
	BEGIN
	 CodeLen:=4; WAsmCode[0]:=$5a00+z; WAsmCode[1]:=AdrVals[0];
	END;
       ModAbs24:
	BEGIN
	 CodeLen:=4; WAsmCode[0]:=$5a00+z+Lo(AdrVals[0]);
	 WAsmCode[1]:=AdrVals[1];
	END;
       ModIIAbs:
	BEGIN
	 CodeLen:=2; WAsmCode[0]:=$5b00+z+Lo(AdrVals[0]);
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('BSR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF (OpSize<>-1) AND (OpSize<>4) AND (OpSize<>2) THEN WrError(1130)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],Int24,OK)-(EProgCounter+2);
       IF OK THEN
	BEGIN
	 IF OpSize=-1 THEN
	 IF (AdrLong>=-128) AND (AdrLong<=127) THEN OpSize:=4
	 ELSE
	  BEGIN
	   OpSize:=2; Dec(AdrLong,2);
	  END
	 ELSE IF OpSize=2 THEN Dec(AdrLong,2);
	 IF OpSize=2 THEN
	  BEGIN
	   IF (AdrLong<-32768) OR (AdrLong>32767) THEN WrError(1370)
	   ELSE IF CPU16 THEN WrError(1500)
	   ELSE
	    BEGIN
	     CodeLen:=4;
	     WAsmCode[0]:=$5c00; WAsmCode[1]:=AdrLong AND $ffff;
	    END;
	  END
	 ELSE
	  BEGIN
	   IF (AdrLong<-128) OR (AdrLong>127) THEN WrError(1370)
	   ELSE
	    BEGIN
	     CodeLen:=2;
	     WAsmCode[0]:=$5500+(AdrLong AND $ff);
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_H8_3:Boolean;
	Far;
BEGIN
   IF ActPC=SegCode THEN
    IF MomCPU=CPU6413308 THEN ChkPC_H8_3:=(ProgCounter>=0) AND (ProgCounter<$10000)
    ELSE ChkPC_H8_3:=(ProgCounter>=0) AND (ProgCounter<$1000000)
   ELSE ChkPC_H8_3:=False;
END;

	FUNCTION IsDef_H8_3:Boolean;
	Far;
BEGIN
   IsDef_H8_3:=False;
END;

	PROCEDURE SwitchFrom_H8_3;
	Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_H8_3;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$68; NOPCode:=$0000;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_H8_3; ChkPC:=ChkPC_H8_3; IsDef:=IsDef_H8_3;
   SwitchFrom:=SwitchFrom_H8_3;

   CPU16:=MomCPU<=CPUH8_300;

   InitFields;
END;

BEGIN
   CPUH8_300L:=AddCPU('H8/300L',SwitchTo_H8_3);
   CPU6413308:=AddCPU('HD6413308',SwitchTo_H8_3);
   CPUH8_300 :=AddCPU('H8/300',SwitchTo_H8_3);
   CPU6413309:=AddCPU('HD6413309',SwitchTo_H8_3);
   CPUH8_300H:=AddCPU('H8/300H',SwitchTo_H8_3);
END.
