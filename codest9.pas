{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeST9;

{ AS Codegeneratormodul ST9 }

INTERFACE
        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
               Name:String[5];
	       Code:Word;
	      END;

CONST
   WorkOfs=$d0;

   ModNone=-1;
   ModReg=0;          MModReg=1 SHL ModReg;                  { Rn }
   ModWReg=1;         MModWReg=1 SHL ModWReg;                { rn }
   ModRReg=2;         MModRReg=1 SHL ModRReg;                { RRn }
   ModWRReg=3;        MModWRReg=1 SHL ModWRReg;              { rrn }
   ModIReg=4;         MModIReg=1 SHL ModIReg;                { (Rn) }
   ModIWReg=5;        MModIWReg=1 SHL ModIWReg;              { (rn) }
   ModIRReg=6;        MModIRReg=1 SHL ModIRReg;              { (RRn) }
   ModIWRReg=7;       MModIWRReg=1 SHL ModIWRReg;            { (rrn) }
   ModIncWReg=8;      MModIncWReg=1 SHL ModIncWReg;          { (rn)+ }
   ModIncWRReg=9;     MModIncWRReg=1 SHL ModIncWRReg;        { (rrn)+ }
   ModDecWRReg=10;    MModDecWRReg=1 SHL ModDecWRReg;        { -(rrn) }
   ModDisp8WReg=11;   MModDisp8WReg=1 SHL ModDisp8WReg;      { d8(rn) }
   ModDisp8WRReg=12;  MModDisp8WRReg=1 SHL ModDisp8WRReg;    { d8(rrn) }
   ModDisp16WRReg=13; MModDisp16WRReg=1 SHL ModDisp16WRReg;  { d16(rrn) }
   ModDispRWRReg=14;  MModDispRWRReg=1 SHL ModDispRWRReg;    { rrm(rrn }
   ModAbs=15;         MModAbs=1 SHL ModAbs;                  { NN }
   ModImm=16;         MModImm=1 SHL ModImm;                  { #N/#NN }
   ModDisp8RReg=17;   MModDisp8RReg=1 SHL ModDisp8RReg;      { d8(RRn) }
   ModDisp16RReg=18;  MModDisp16RReg=1 SHL ModDisp16RReg;    { d16(RRn) }

   FixedOrderCnt=12;
   ALUOrderCnt=10;
   RegOrderCnt=13;
   Reg16OrderCnt=8;
   Bit2OrderCnt=4;
   Bit1OrderCnt=4;
   ConditionCnt=20;
   LoadOrderCnt=4;

TYPE
   FixedOrderArray=ARRAY[0..FixedOrderCnt-1] OF FixedOrder;
   ALUOrderArray=ARRAY[0..ALUOrderCnt-1] OF FixedOrder;
   RegOrderArray=ARRAY[0..RegOrderCnt-1] OF FixedOrder;
   Reg16OrderArray=ARRAY[0..Reg16OrderCnt-1] OF FixedOrder;
   Bit2OrderArray=ARRAY[0..Bit2OrderCnt-1] OF FixedOrder;
   Bit1OrderArray=ARRAY[0..Bit1OrderCnt-1] OF FixedOrder;
   ConditionArray=ARRAY[0..ConditionCnt-1] OF FixedOrder;
   LoadOrderArray=ARRAY[0..LoadOrderCnt-1] OF FixedOrder;

VAR
   CPUST9020,CPUST9030,CPUST9040,CPUST9050:CPUVar;

   FixedOrders:^FixedOrderArray;
   ALUOrders:^ALUOrderArray;
   RegOrders:^RegOrderArray;
   Reg16Orders:^Reg16OrderArray;
   Bit2Orders:^Bit2OrderArray;
   Bit1Orders:^Bit1OrderArray;
   Conditions:^ConditionArray;
   LoadOrders:^LoadOrderArray;

   AdrMode,AbsSeg:ShortInt;
   AdrPart,AdrCnt,OpSize:Byte;
   AdrVals:ARRAY[0..2] OF Byte;

   SaveInitProc:PROCEDURE;
   DPAssume:LongInt;

{----------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

   	PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   IF z>=FixedOrderCnt THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

   	PROCEDURE AddALU(NName:String; NCode:Word);
BEGIN
   IF z>=ALUOrderCnt THEN Halt(255);
   WITH ALUOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddReg(NName:String; NCode:Word);
BEGIN
   IF z>=RegOrderCnt THEN Halt(255);
   WITH RegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddReg16(NName:String; NCode:Word);
BEGIN
   IF z>=Reg16OrderCnt THEN Halt(255);
   WITH Reg16Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddBit2(NName:String; NCode:Word);
BEGIN
   IF z>=Bit2OrderCnt THEN Halt(255);
   WITH Bit2Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddBit1(NName:String; NCode:Word);
BEGIN
   IF z>=Bit1OrderCnt THEN Halt(255);
   WITH Bit1Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddCondition(NName:String; NCode:Word);
BEGIN
   IF z>=ConditionCnt THEN Halt(255);
   WITH Conditions^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddLoad(NName:String; NCode:Word);
BEGIN
   IF z>=LoadOrderCnt THEN Halt(255);
   WITH LoadOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('CCF' ,$0061); AddFixed('DI'  ,$0010);
   AddFixed('EI'  ,$0000); AddFixed('HALT',$bf01);
   AddFixed('IRET',$00d3); AddFixed('NOP' ,$00ff);
   AddFixed('RCF' ,$0011); AddFixed('RET' ,$0046);
   AddFixed('SCF' ,$0001); AddFixed('SDM' ,$00fe);
   AddFixed('SPM' ,$00ee); AddFixed('WFI' ,$ef01);

   New(ALUOrders); z:=0;
   AddALU('ADC', 3); AddALU('ADD', 4); AddALU('AND', 1);
   AddALU('CP' , 9); AddALU('OR' , 0); AddALU('SBC', 2);
   AddALU('SUB', 5); AddALU('TCM', 8); AddALU('TM' ,10);
   AddALU('XOR', 6);

   New(RegOrders); z:=0;
   AddReg('CLR' ,$90); AddReg('CPL' ,$80); AddReg('DA'  ,$70);
   AddReg('DEC' ,$40); AddReg('INC' ,$50); AddReg('POP' ,$76);
   AddReg('POPU',$20); AddReg('RLC' ,$b0); AddReg('ROL' ,$a0);
   AddReg('ROR' ,$c0); AddReg('RRC' ,$d0); AddReg('SRA' ,$e0);
   AddReg('SWAP',$f0);

   New(Reg16Orders); z:=0;
   AddReg16('DECW' ,$cf); AddReg16('EXT'  ,$c6);
   AddReg16('INCW' ,$df); AddReg16('POPUW',$b7);
   AddReg16('POPW' ,$75); AddReg16('RLCW' ,$8f);
   AddReg16('RRCW' ,$36); AddReg16('SRAW' ,$2f);

   New(Bit2Orders); z:=0;
   AddBit2('BAND',$1f); AddBit2('BLD' ,$f2);
   AddBit2('BOR' ,$0f); AddBit2('BXOR',$6f);

   New(Bit1Orders); z:=0;
   AddBit1('BCPL',$6f); AddBit1('BRES' ,$1f);
   AddBit1('BSET',$0f); AddBit1('BTSET',$f2);

   New(Conditions); z:=0;
   AddCondition('F'   ,$0); AddCondition('T'   ,$8);
   AddCondition('C'   ,$7); AddCondition('NC'  ,$f);
   AddCondition('Z'   ,$6); AddCondition('NZ'  ,$e);
   AddCondition('PL'  ,$d); AddCondition('MI'  ,$5);
   AddCondition('OV'  ,$4); AddCondition('NOV' ,$c);
   AddCondition('EQ'  ,$6); AddCondition('NE'  ,$e);
   AddCondition('GE'  ,$9); AddCondition('LT'  ,$1);
   AddCondition('GT'  ,$a); AddCondition('LE'  ,$2);
   AddCondition('UGE' ,$f); AddCondition('UL'  ,$7);
   AddCondition('UGT' ,$b); AddCondition('ULE' ,$3);

   New(LoadOrders); z:=0;
   AddLoad('LDPP',$00); AddLoad('LDDP',$10);
   AddLoad('LDPD',$01); AddLoad('LDDD',$11);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(ALUOrders);
   Dispose(RegOrders);
   Dispose(Reg16Orders);
   Dispose(Bit2Orders);
   Dispose(Bit1Orders);
   Dispose(Conditions);
   Dispose(LoadOrders);
END;

{----------------------------------------------------------------------------}

	FUNCTION DecodeReg(Asc:String; VAR Erg,Size:Byte):Boolean;
VAR
   Res:ValErgType;
BEGIN
   DecodeReg:=False;

   IF Length(Asc)<2 THEN Exit;
   IF Asc[1]<>'r' THEN Exit; Delete(Asc,1,1);
   IF Asc[1]='r' THEN
    BEGIN
     IF Length(Asc)<2 THEN Exit;
     Size:=1; Delete(Asc,1,1);
    END
   ELSE Size:=0;

   Val(Asc,Erg,Res);
   IF (Res<>0) OR (Erg>15) THEN Exit;
   IF (Size=1) AND (Odd(Erg)) THEN Exit;

   DecodeReg:=True;
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:LongInt);
LABEL
   Found;
VAR
   AdrWord:Word;
   level,z:Integer;
   flg,Size:Byte;
   OK:Boolean;
   Reg:String;
BEGIN
   AdrMode:=ModNone; AdrCnt:=0;

   { immediate }

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize OF
     0:AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
     1:BEGIN
        AdrWord:=EvalIntExpression(Asc,Int16,OK);
        AdrVals[0]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
       END;
     END;
     IF OK THEN
      BEGIN
       AdrMode:=ModImm; AdrCnt:=OpSize+1;
      END;
     Goto Found;
    END;

   { Arbeitsregister }

   IF DecodeReg(Asc,AdrPart,Size) THEN
    BEGIN
     IF Size=0 THEN
      IF Mask AND MModWReg<>0 THEN AdrMode:=ModWReg
      ELSE
       BEGIN
        AdrVals[0]:=WorkOfs+AdrPart; AdrCnt:=1; AdrMode:=ModReg;
       END
     ELSE
      IF Mask AND MModWRReg<>0 THEN AdrMode:=ModWRReg
      ELSE
       BEGIN
        AdrVals[0]:=WorkOfs+AdrPart; AdrCnt:=1; AdrMode:=ModRReg;
       END;
     Goto Found;
    END;

   { Postinkrement }

   IF Asc[Length(Asc)]='+' THEN
    BEGIN
     IF (Asc[1]<>'(') OR (Asc[Length(Asc)-1]<>')') THEN WrError(1350)
     ELSE
      BEGIN
       Asc:=Copy(Asc,2,Length(Asc)-3);
       IF NOT DecodeReg(Asc,AdrPart,Size) THEN WrXError(1445,Asc);
       IF Size=0 THEN AdrMode:=ModIncWReg ELSE AdrMode:=ModIncWRReg;
      END;
     Goto Found;
    END;

   { Predekrement }

   IF (Asc[1]='-') AND (Asc[2]='(') AND (Asc[Length(Asc)]=')') THEN
    BEGIN
     IF DecodeReg(Copy(Asc,3,Length(Asc)-3),AdrPart,Size) THEN
      BEGIN
       IF Size=0 THEN WrError(1350) ELSE AdrMode:=ModDecWRReg;
       Goto Found;
      END;
    END;

   { indirekt<->direkt }

   IF (Asc[Length(Asc)]<>')') OR (Length(Asc)<3) THEN OK:=False
   ELSE
    BEGIN
     level:=0; z:=Length(Asc)-1; flg:=0;
     WHILE (z>0) AND (level>=0) DO
      BEGIN
       CASE Asc[z] OF
       '(':IF flg=0 THEN Dec(level);
       ')':IF flg=0 THEN Inc(level);
       '''':IF (flg AND 2=0) THEN flg:=flg XOR 1;
       '"':IF (flg AND 1=0) THEN flg:=flg XOR 2;
       END;
       Dec(z);
      END;
     OK:=(level=-1) AND ((z=0) OR (Asc[z] IN ['a'..'z','A'..'Z','0'..'9','.','_',#128..#255]));
    END;

   { indirekt }

   IF OK THEN
    BEGIN
     Reg:=Copy(Asc,z+2,Length(Asc)-z-2); Byte(Asc[0]):=z;
     IF DecodeReg(Reg,AdrPart,Size) THEN
      IF Size=0 THEN  { d(r) }
       BEGIN
        AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
        IF OK THEN
         BEGIN
          IF (Mask AND MModIWReg<>0) AND (AdrVals[0]=0) THEN AdrMode:=ModIWReg
          ELSE IF (Mask AND MModIReg<>0) AND (AdrVals[0]=0) THEN
           BEGIN
            AdrVals[0]:=WorkOfs+AdrPart; AdrMode:=ModIReg; AdrCnt:=1;
           END
          ELSE
           BEGIN
            AdrMode:=ModDisp8WReg; AdrCnt:=1;
           END;
         END;
       END
      ELSE            { ...(rr) }
       BEGIN
        IF DecodeReg(Asc,AdrVals[0],Size) THEN
         BEGIN        { rr(rr) }
          IF Size<>1 THEN WrError(1350)
          ELSE
           BEGIN
            AdrMode:=ModDispRWRReg; AdrCnt:=1;
           END;
         END
        ELSE
         BEGIN        { d(rr) }
          AdrWord:=EvalIntExpression(Asc,Int16,OK);
          IF (AdrWord=0) AND (Mask AND (MModIRReg+MModIWRReg)<>0) THEN
           BEGIN
            IF (Mask AND MModIWRReg<>0) THEN AdrMode:=ModIWRReg
            ELSE
             BEGIN
              AdrMode:=ModIRReg; AdrVals[0]:=AdrPart+WorkOfs; AdrCnt:=1;
             END;
           END
          ELSE IF (AdrWord<$100) AND (Mask AND (MModDisp8WRReg+MModDisp8RReg)<>0) THEN
           BEGIN
            IF (Mask AND MModDisp8WRReg<>0) THEN
             BEGIN
              AdrVals[0]:=Lo(AdrWord); AdrCnt:=1; AdrMode:=ModDisp8WRReg;
             END
            ELSE
             BEGIN
              AdrVals[0]:=AdrPart+WorkOfs; AdrVals[1]:=Lo(AdrWord);
              AdrCnt:=2; AdrMode:=ModDisp8RReg;
             END;
           END
          ELSE IF (Mask AND MModDisp16WRReg<>0) THEN
           BEGIN
            AdrVals[0]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
	    AdrCnt:=2; AdrMode:=ModDisp16WRReg;
           END
          ELSE
           BEGIN
            AdrVals[0]:=AdrPart+WorkOfs;
            AdrVals[2]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
            AdrCnt:=3; AdrMode:=ModDisp16RReg;
           END;
         END;
       END

     ELSE             { ...(RR) }
      BEGIN
       AdrWord:=EvalIntExpression(Reg,UInt9,OK);
       IF (TypeFlag AND (1 SHL SegReg)=0) THEN WrError(1350)
       ELSE IF AdrWord<$ff THEN
        BEGIN
         AdrVals[0]:=Lo(AdrWord);
         AdrWord:=EvalIntExpression(Asc,Int8,OK);
         IF AdrWord<>0 THEN WrError(1320)
         ELSE
          BEGIN
           AdrCnt:=1; AdrMode:=ModIReg;
          END;
        END
       ELSE IF (AdrWord>$1ff) OR (Odd(AdrWord)) THEN WrError(1350)
       ELSE
        BEGIN
         AdrVals[0]:=Lo(AdrWord);
         AdrWord:=EvalIntExpression(Asc,Int16,OK);
         IF (AdrWord=0) AND (Mask AND MModIRReg<>0) THEN
          BEGIN
           AdrCnt:=1; AdrMode:=ModIRReg;
          END
         ELSE IF (AdrWord<$100) AND (Mask AND MModDisp8RReg<>0) THEN
          BEGIN
           AdrVals[1]:=Lo(AdrWord); AdrCnt:=2; AdrMode:=ModDisp8RReg;
          END
         ELSE
          BEGIN
           AdrVals[2]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
           AdrCnt:=3; AdrMode:=ModDisp8RReg;
          END;
        END;
      END;
     Goto Found;
    END;

   { direkt }

   AdrWord:=EvalIntExpression(Asc,UInt16,OK);
   IF OK THEN
    IF (TypeFlag AND (1 SHL SegReg))=0 THEN
     BEGIN
      AdrMode:=ModAbs;
      AdrVals[0]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
      AdrCnt:=2; ChkSpace(AbsSeg);
     END
    ELSE IF AdrWord<$ff THEN
     BEGIN
      AdrMode:=ModReg; AdrVals[0]:=Lo(AdrWord); AdrCnt:=1;
     END
    ELSE IF (AdrWord>$1ff) OR (Odd(AdrWord)) THEN WrError(1350)
    ELSE
     BEGIN
      AdrMode:=ModRReg; AdrVals[0]:=Lo(AdrWord); AdrCnt:=1;
     END;

Found:
   IF (AdrMode<>ModNone) AND ((LongInt(1) SHL AdrMode) AND Mask=0) THEN
    BEGIN
     WrError(1350); AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

        FUNCTION SplitBit(VAR Asc:String; VAR Erg:Byte):Boolean;
VAR
   p:Integer;
   OK,Inv:Boolean;
BEGIN
   SplitBit:=False;

   p:=RQuotPos(Asc,'.');
   IF (p=0) OR (p=Length(Asc)) THEN
    BEGIN
     IF Asc[1]='!' THEN
      BEGIN
       Inv:=True; Delete(Asc,1,1);
      END
     ELSE Inv:=False;
     p:=EvalIntExpression(Asc,UInt8,OK);
     IF OK THEN
      BEGIN
       Erg:=p AND 15; IF Inv THEN Erg:=Erg XOR 1;
       Str(p SHR 4,Asc); Asc:='r'+Asc;
       SplitBit:=True;
      END;
     Exit;
    END;

   IF Asc[p+1]='!' THEN
    Erg:=1+(EvalIntExpression(Copy(Asc,p+2,Length(Asc)-p-1),UInt3,OK) SHL 1)
   ELSE
    Erg:=EvalIntExpression(Copy(Asc,p+1,Length(Asc)-p),UInt3,OK) SHL 1;
   Delete(Asc,p,Length(Asc)-p+1);
   SplitBit:=OK;
END;

{----------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUMEST9Count=1;
   ASSUMEST9s:ARRAY[1..ASSUMEST9Count] OF ASSUMERec=
               ((Name:'DP' ; Dest:@DPAssume ; Min:0; Max:  1; NothingVal:  $0));
VAR
   Bit:Byte;
   s:String[5];
BEGIN
   DecodePseudo:=True;

   IF Memo('REG') THEN
    BEGIN
     CodeEquate(SegReg,0,$1ff);
     Exit;
    END;

   IF Memo('BIT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF SplitBit(ArgStr[1],Bit) THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg);
       IF AdrMode=ModWReg THEN
        BEGIN
         PushLocHandle(-1);
         EnterIntSymbol(LabPart,(AdrPart SHL 4)+Bit,SegNone,False);
         PopLocHandle;
         Str(AdrPArt,s); ListLine:='=r'+s+'.';
         IF Odd(Bit) THEN ListLine:=ListLine+'!';
         ListLine:=ListLine+Chr((Bit SHR 1)+AscOfs);
        END;
      END;
     Exit;
    END;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUMEST9s,ASSUMEST9Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_ST9;
	Far;
VAR
   z,AdrInt:Integer;
   OK:Boolean;
   HReg,HPart:Byte;
   Mask1,Mask2,AdrWord:Word;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=0;
   IF DPAssume=1 THEN AbsSeg:=SegData ELSE AbsSeg:=SegCode;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(True) THEN Exit;

   { ohne Argument}

   FOR z:=0 TO FixedOrderCnt-1 DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
        BEGIN
         OK:=Hi(Code)<>0;
         IF OK THEN BAsmCode[0]:=Hi(Code);
         CodeLen:=Ord(OK);
         BAsmCode[CodeLen]:=Lo(Code); Inc(CodeLen);
        END;
       Exit;
      END;

   { Datentransfer }

   IF (Memo('LD')) OR (Memo('LDW')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF OpPart[Length(OpPart)]='W' THEN
        BEGIN
         OpSize:=1; Mask1:=MModWRReg; Mask2:=MModRReg;
        END
       ELSE
        BEGIN
         Mask1:=MModWReg; Mask2:=MModReg;
        END;
       DecodeAdr(ArgStr[1],Mask1+Mask2+MModIWReg+MModDisp8WReg+MModIncWReg+
                           MModIWRReg+MModIncWRReg+MModDecWRReg+MModDisp8WRReg+
                           MModDisp16WRReg+MModDispRWRReg+MModAbs+MModIRReg);
       CASE AdrMode OF
       ModWReg,ModWRReg:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],Mask1+Mask2+MModIWReg+MModDisp8WReg+MModIWRReg+
                             MModIncWRReg+MModDecWRReg+MModDispRWRReg+MModDisp8WRReg+
                             MModDisp16WRReg+MModAbs+MModImm);
         CASE AdrMode OF
         ModWReg:
          BEGIN
           BAsmCode[0]:=(HReg SHL 4)+8; BAsmCode[1]:=WorkOfs+AdrPart;
           CodeLen:=2;
          END;
         ModWRReg:
          BEGIN
           BAsmCode[0]:=$e3; BAsmCode[1]:=(HReg SHL 4)+AdrPart;
           CodeLen:=2;
          END;
         ModReg:
          BEGIN
           BAsmCode[0]:=(HReg SHL 4)+8; BAsmCode[1]:=AdrVals[0];
           CodeLen:=2;
          END;
         ModRReg:
          BEGIN
           BAsmCode[0]:=$ef; BAsmCode[1]:=AdrVals[0];
           BAsmCode[2]:=HReg+WorkOfs; CodeLen:=3;
          END;
         ModIWReg:
          IF OpSize=0 THEN
           BEGIN
            BAsmCode[0]:=$e4; BAsmCode[1]:=(HReg SHL 4)+AdrPart;
            CodeLen:=2;
           END
          ELSE
           BEGIN
            BAsmCode[0]:=$a6; BAsmCode[1]:=$f0+AdrPart;
            BAsmCode[2]:=WorkOfs+HReg; CodeLen:=3;
           END;
         ModDisp8WReg:
          BEGIN
           BAsmCode[0]:=$b3+(OpSize*$2b);
           BAsmCode[1]:=(HReg SHL 4)+AdrPart;
           BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
          END;
         ModIWRReg:
          BEGIN
           BAsmCode[0]:=$b5+(OpSize*$2e);
           BAsmCode[1]:=(HReg SHL 4)+AdrPart+OpSize;
           CodeLen:=2;
          END;
         ModIncWRReg:
          BEGIN
           BAsmCode[0]:=$b4+(OpSize*$21);
           BAsmCode[1]:=$f1+AdrPart;
           BAsmCode[2]:=WorkOfs+HReg; CodeLen:=3;
          END;
         ModDecWRReg:
          BEGIN
           BAsmCode[0]:=$c2+OpSize;
           BAsmCode[1]:=$f1+AdrPart;
           BAsmCode[2]:=WorkOfs+HReg; CodeLen:=3;
          END;
         ModDispRWRReg:
          BEGIN
           BAsmCode[0]:=$60;
           BAsmCode[1]:=($10*(1-OpSize))+(AdrVals[0] SHL 4)+AdrPart;
           BAsmCode[2]:=$f0+HReg; CodeLen:=3;
          END;
         ModDisp8WRReg:
          BEGIN
           BAsmCode[0]:=$7f+(OpSize*7);
           BAsmCode[1]:=$f1+AdrPart;
           BAsmCode[2]:=AdrVals[0]; BAsmCode[3]:=HReg+WorkOfs;
           CodeLen:=4;
          END;
         ModDisp16WRReg:
          BEGIN
           BAsmCode[0]:=$7f+(OpSize*7);
           BAsmCode[1]:=$f0+AdrPart;
           Move(AdrVals,BAsmCode[2],AdrCnt); BAsmCode[2+AdrCnt]:=HReg+WorkOfs;
           CodeLen:=3+AdrCnt;
          END;
         ModAbs:
          BEGIN
           BAsmCode[0]:=$c4+(OpSize*$1e);
           BAsmCode[1]:=$f0+AdrPart;
           Move(AdrVals,BAsmCode[2],AdrCnt);
           CodeLen:=2+AdrCnt;
          END;
         ModImm:
          IF OpSize=0 THEN
           BEGIN
            BAsmCode[0]:=(HReg SHL 4)+$0c;
            Move(AdrVals,BAsmCode[1],AdrCnt);
            CodeLen:=1+AdrCnt;
           END
          ELSE
           BEGIN
            BAsmCode[0]:=$bf; BAsmCode[1]:=WorkOfs+HReg;
            Move(AdrVals,BAsmCode[2],AdrCnt);
            CodeLen:=2+AdrCnt;
           END;
         END;
        END;
       ModReg,ModRReg:
        BEGIN
         HReg:=AdrVals[0];
         DecodeAdr(ArgStr[2],Mask1+Mask2+MModIWReg+MModIWRReg+MModIncWRReg+
                             MModDecWRReg+MModDispRWRReg+MModDisp8WRReg+MModDisp16WRReg+
                             MModImm);
         CASE AdrMode OF
         ModWReg:
          BEGIN
           BAsmCode[0]:=(AdrPart SHL 4)+$09; BAsmCode[1]:=HReg; CodeLen:=2;
          END;
         ModWRReg:
          BEGIN
           BAsmCode[0]:=$ef; BAsmCode[1]:=WorkOfs+AdrPart;
           BAsmCode[2]:=HReg; CodeLen:=3;
          END;
         ModReg,ModRReg:
          BEGIN
           BAsmCode[0]:=$f4-(OpSize*5);
           BAsmCode[1]:=AdrVals[0];
           BAsmCode[2]:=HReg; CodeLen:=3;
          END;
         ModIWReg:
          BEGIN
           BAsmCode[0]:=$e7-($41*OpSize); BAsmCode[1]:=$f0+AdrPart;
           BAsmCode[2]:=HReg; CodeLen:=3;
          END;
         ModIWRReg:
          BEGIN
           BAsmCode[0]:=$72+(OpSize*12);
           BAsmCode[1]:=$f1+AdrPart-OpSize; BAsmCode[2]:=HReg;
           CodeLen:=3;
          END;
         ModIncWRReg:
          BEGIN
           BAsmCode[0]:=$b4+($21*OpSize);
           BAsmCode[1]:=$f1+AdrPart; BAsmCode[2]:=HReg;
           CodeLen:=3;
          END;
         ModDecWRReg:
          BEGIN
           BAsmCode[0]:=$c2+OpSize;
           BAsmCode[1]:=$f1+AdrPart; BAsmCode[2]:=HReg;
           CodeLen:=3;
          END;
         ModDisp8WRReg:
          BEGIN
           BAsmCode[0]:=$7f+(OpSize*7);
           BAsmCode[1]:=$f1+AdrPart;
           BAsmCode[2]:=AdrVals[0];
           BAsmCode[3]:=HReg; CodeLen:=4;
          END;
         ModDisp16WRReg:
          BEGIN
           BAsmCode[0]:=$7f+(OpSize*7);
           BAsmCode[1]:=$f0+AdrPart;
           Move(AdrVals,BAsmCode[2],AdrCnt);
           BAsmCode[2+AdrCnt]:=HReg; CodeLen:=3+AdrCnt;
          END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$f5-(OpSize*$36);
           BAsmCode[1]:=HReg;
           Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
         END;
        END;
       ModIWReg:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],Mask2+Mask1);
         CASE AdrMode OF
         ModWReg:
          BEGIN
           BAsmCode[0]:=$e5; BAsmCode[1]:=(HReg SHL 4)+AdrPart; CodeLen:=2;
          END;
         ModWRReg:
          BEGIN
           BAsmCode[0]:=$96; BAsmCode[1]:=WorkOfs+AdrPart;
           BAsmCode[2]:=$f0+HReg; CodeLen:=3;
          END;
         ModReg,ModRReg:
          BEGIN
           BAsmCode[0]:=$e6-($50*OpSize); BAsmCode[1]:=AdrVals[0];
           BAsmCode[2]:=$f0+HReg; CodeLen:=3;
          END;
         END;
        END;
       ModDisp8WReg:
        BEGIN
         BAsmCode[2]:=AdrVals[0]; HReg:=AdrPart;
         DecodeAdr(ArgStr[2],Mask1);
         CASE AdrMode OF
         ModWReg,ModWRReg:
          BEGIN
           BAsmCode[0]:=$b2+(OpSize*$2c);
           BAsmCode[1]:=(AdrPart SHL 4)+(OpSize SHL 4)+HReg;
           CodeLen:=3;
          END;
         END;
        END;
       ModIncWReg:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModIncWRReg*(1-OpSize));
         CASE AdrMode OF
         ModIncWRReg:
          BEGIN
           BAsmCode[0]:=$d7; BAsmCode[1]:=(HReg SHL 4)+AdrPart+1; CodeLen:=2;
          END;
         END;
        END;
       ModIWRReg:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],(MModIWReg*(1-OpSize))+Mask1+Mask2+MModIWRReg+MModImm);
         CASE AdrMode OF
         ModIWReg:
          BEGIN
           BAsmCode[0]:=$b5; BAsmCode[1]:=(AdrPart SHL 4)+HReg+1; CodeLen:=2;
          END;
         ModWReg:
          BEGIN
           BAsmCode[0]:=$72; BAsmCode[1]:=$f0+HReg;
           BAsmCode[2]:=AdrPart+WorkOfs;
           CodeLen:=3;
          END;
         ModWRReg:
          BEGIN
           BAsmCode[0]:=$e3; BAsmCode[1]:=(HReg SHL 4)+$10+AdrPart;
           CodeLen:=2;
          END;
         ModReg,ModRReg:
          BEGIN
           BAsmCode[0]:=$72+(OpSize*$4c);
           BAsmCode[1]:=$f0+HReg+OpSize; BAsmCode[2]:=AdrVals[0];
           CodeLen:=3;
          END;
         ModIWRReg:
          IF OpSize=0 THEN
           BEGIN
            BAsmCode[0]:=$73;
            BAsmCode[1]:=$f0+AdrPart;
            BAsmCode[2]:=WorkOfs+HReg; CodeLen:=3;
           END
          ELSE
           BEGIN
            BAsmCode[0]:=$e3; BAsmCOde[1]:=$11+(HReg SHL 4)+AdrPart;
            CodeLen:=2;
           END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$f3-(OpSize*$35);
           BAsmCode[1]:=$f0+HReg;
           Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
         END;
        END;
       ModIncWRReg:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],Mask2+(MModIncWReg*(1-OpSize)));
         CASE AdrMode OF
         ModReg,ModRReg:
          BEGIN
           BAsmCode[0]:=$b4+(OpSize*$21);
           BAsmCode[1]:=$f0+HReg; BAsmCode[2]:=AdrVals[0];
           CodeLen:=3;
          END;
         ModIncWReg:
          BEGIN
           BAsmCode[0]:=$d7; BAsmCode[1]:=(AdrPart SHL 4)+HReg;
           CodeLen:=2;
          END;
         END;
        END;
       ModDecWRReg:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],Mask2);
         CASE AdrMode OF
         ModReg,ModRReg:
          BEGIN
           BAsmCode[0]:=$c2+OpSize;
           BAsmCode[1]:=$f0+HReg; BAsmCode[2]:=AdrVals[0];
           CodeLen:=3;
          END;
         END;
        END;
       ModDispRWRReg:
        BEGIN
         HReg:=AdrPart; HPart:=AdrVals[0];
         DecodeAdr(ArgStr[2],Mask1);
         CASE AdrMode OF
         ModWReg,ModWRReg:
          BEGIN
           BAsmCode[0]:=$60;
           BAsmCode[1]:=($10*(1-OpSize))+$01+(HPart SHL 4)+HReg;
           BAsmCode[2]:=$f0+AdrPart;
           CodeLen:=3;
          END;
         END;
        END;
       ModDisp8WRReg:
        BEGIN
         BAsmCode[2]:=AdrVals[0]; HReg:=AdrPart;
         DecodeAdr(ArgStr[2],Mask2+(OpSize*MModImm));
         CASE AdrMode OF
         ModReg,ModRReg:
          BEGIN
           BAsmCode[0]:=$26+(OpSize*$60);
           BAsmCode[1]:=$f1+HReg; BAsmCode[3]:=AdrVals[0];
           CodeLen:=4;
          END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$06; BAsmCode[1]:=$f1+HReg;
           Move(AdrVals,BAsmCode[3],AdrCnt); CodeLen:=3+AdrCnt;
          END;
         END;
        END;
       ModDisp16WRReg:
        BEGIN
         Move(AdrVals,BAsmCode[2],2); HReg:=AdrPart;
         DecodeAdr(ArgStr[2],Mask2+(OpSize*MModImm));
         CASE AdrMode OF
         ModReg,ModRReg:
          BEGIN
           BAsmCode[0]:=$26+(OpSize*$60);
           BAsmCode[1]:=$f0+HReg; BAsmCode[4]:=AdrVals[0];
           CodeLen:=5;
          END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$06; BAsmCode[1]:=$f0+HReg;
           Move(AdrVals,BAsmCode[4],AdrCnt); CodeLen:=4+AdrCnt;
          END;
         END;
        END;
       ModAbs:
        BEGIN
         Move(AdrVals,BAsmCode[2],2);
         DecodeAdr(ArgStr[2],Mask1+MModImm);
         CASE AdrMode OF
         ModWReg,ModWRReg:
          BEGIN
           BAsmCode[0]:=$c5+(OpSize*$1d);
           BAsmCode[1]:=$f0+AdrPart+OpSize;
           CodeLen:=4;
          END;
         ModImm:
          BEGIN
           Move(BAsmCode[2],BAsmCode[2+AdrCnt],2); BAsmCode[0]:=$2f+(OpSize*7);
           BAsmCode[1]:=$f1; Move(AdrVals,BAsmCode[2],AdrCnt);
           CodeLen:=4+AdrCnt;
          END;
         END;
        END;
       ModIRReg:
        BEGIN
         HReg:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModIWRReg*(1-OpSize));
         CASE AdrMode OF
         ModIWRReg:
          BEGIN
           BAsmCode[0]:=$73; BAsmCode[1]:=$f0+AdrPart; BAsmCode[2]:=HReg;
           CodeLen:=3;
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   FOR z:=0 TO LoadOrderCnt-1 DO
    WITH LoadOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModIncWRReg);
         IF AdrMode=ModIncWRReg THEN
          BEGIN
           HReg:=AdrPart SHL 4;
           DecodeAdr(ArgStr[2],MModIncWRReg);
           IF AdrMode=ModIncWRReg THEN
            BEGIN
             BAsmcode[0]:=$d6;
             BAsmCode[1]:=Code+HReg+AdrPart;
             CodeLen:=2;
            END;
          END;
        END;
       Exit;
      END;

   IF (Memo('PEA')) OR (Memo('PEAU')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModDisp8RReg+MModDisp16RReg);
       IF AdrMode<>ModNone THEN
        BEGIN
         BAsmCode[0]:=$8f;
         BAsmCode[1]:=$01+(2*Ord(Memo('PEAU')));
         Move(AdrVals,BAsmCode[2],AdrCnt);
         Inc(BAsmCode[2],AdrCnt-2);
         CodeLen:=2+AdrCnt;
        END;
      END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('PUSHU')) THEN
    BEGIN
     z:=Ord(Memo('PUSHU'));
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModIReg+MModImm);
       CASE AdrMode OF
       ModReg:
        BEGIN
         BAsmCode[0]:=$66-(z*$36); BAsmCode[1]:=AdrVals[0];
         CodeLen:=2;
        END;
       ModIReg:
        BEGIN
         BAsmCode[0]:=$f7-(z*$c6); BAsmCode[1]:=AdrVals[0];
         CodeLen:=2;
        END;
       ModImm:
        BEGIN
         BAsmCode[0]:=$8f; BAsmCode[1]:=$f1+(z*2);
         BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('PUSHW')) OR (Memo('PUSHUW')) THEN
    BEGIN
     z:=Ord(Memo('PUSHUW')); OpSize:=1;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModRReg+MModImm);
       CASE AdrMode OF
       ModRReg:
        BEGIN
         BAsmCode[0]:=$74+(z*$42); BAsmCode[1]:=AdrVals[0];
         CodeLen:=2;
        END;
       ModImm:
        BEGIN
         BAsmCode[0]:=$8f; BAsmCode[1]:=$c1+(z*2);
         Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
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
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode=ModReg THEN
        BEGIN
         BAsmCode[2]:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModReg);
         IF AdrMode=ModReg THEN
          BEGIN
           BAsmCode[1]:=AdrVals[0];
           BAsmCode[0]:=$16;
           CodeLen:=3;
          END;
        END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=0 TO ALUOrderCnt-1 DO
    WITH ALUOrders^[z] DO
     IF (Memo(Name)) OR (Memo(Name+'W')) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         IF OpPart[Length(OpPart)]='W' THEN
	  BEGIN
           OpSize:=1; Mask1:=MModWRReg; Mask2:=MModRReg;
	  END
	 ELSE 
	  BEGIN
           Mask1:=MModWReg; Mask2:=MModReg;
	  END;
         DecodeAdr(ArgStr[1],Mask1+Mask2+MModIWReg+MModIWRReg+MModIncWRReg+
                             MModDecWRReg+MModDispRWRReg+MModDisp8WRReg+MModDisp16WRReg+
                             MModAbs+MModIRReg);
         CASE AdrMode OF
         ModWReg,ModWRReg:
          BEGIN
           HReg:=AdrPart;
           DecodeAdr(ArgStr[2],Mask1+MModIWReg+Mask2+MModIWRReg+MModIncWRReg+
                               MModDecWRReg+MModDispRWRReg+MModDisp8WRReg+MModDisp16WRReg+
                               MModAbs+MModImm);
           CASE AdrMode OF
           ModWReg,ModWRReg:
            BEGIN
             BAsmCode[0]:=(Code SHL 4)+2+(OpSize*12);
	     BAsmCode[1]:=(HReg SHL 4)+AdrPart;
             CodeLen:=2;
            END;
           ModIWReg:
            IF OpSize=0 THEN
             BEGIN
              BAsmCode[0]:=(Code SHL 4)+3; BAsmCode[1]:=(HReg SHL 4)+AdrPart;
              CodeLen:=2;
             END
            ELSE
             BEGIN
              BAsmCode[0]:=$a6; BAsmCode[1]:=(Code SHL 4)+AdrPart;
              BAsmCode[2]:=WorkOfs+HReg; CodeLEn:=3;
             END;
           ModReg,ModRReg:
            BEGIN
             BAsmCode[0]:=(Code SHL 4)+4+(OpSize*3);
	     BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=HReg+WorkOfs;
             CodeLen:=3;
            END;
           ModIWRReg:
            IF OpSize=0 THEN
             BEGIN
              BAsmCode[0]:=$72; BAsmCode[1]:=(Code SHL 4)+AdrPart+1;
      	      BAsmCode[2]:=HReg+WorkOfs;
              CodeLen:=3;
             END
            ELSE
             BEGIN
              BAsmCode[0]:=(Code SHL 4)+$0e;
	      BAsmCode[1]:=(HReg SHL 4)+AdrPart+1;
              CodeLen:=2;
             END;
           ModIncWRReg:
            BEGIN
             BAsmCode[0]:=$b4+(OpSize*$21);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart+1;
	     BAsmCode[2]:=HReg+WorkOfs;
             CodeLen:=3;
            END;
           ModDecWRReg:
            BEGIN
             BAsmCode[0]:=$c2+OpSize;
	     BAsmCode[1]:=(Code SHL 4)+AdrPart+1;
	     BAsmCode[2]:=HReg+WorkOfs;
             CodeLen:=3;
            END;
           ModDispRWRReg:
            BEGIN
             BAsmCode[0]:=$60;
	     BAsmCode[1]:=$10*(1-OpSize)+(AdrVals[0] SHL 4)+AdrPart;
	     BAsmCode[2]:=(Code SHL 4)+HReg;
             CodeLen:=3;
            END;
           ModDisp8WRReg:
            BEGIN
             BAsmCode[0]:=$7f+(OpSize*7);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart+1;
	     BAsmCode[2]:=AdrVals[0]; BAsmCode[3]:=WorkOfs+HReg;
             CodeLen:=4;
            END;
           ModDisp16WRReg:
            BEGIN
             BAsmCode[0]:=$7f+(OpSize*7);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart;
	     Move(AdrVals,BAsmCode[2],AdrCnt);
	     BAsmCode[2+AdrCnt]:=WorkOfs+HReg;
             CodeLen:=3+AdrCnt;
            END;
           ModAbs:
            BEGIN
             BAsmCode[0]:=$c4+(OpSize*30);
	     BAsmCode[1]:=(Code SHL 4)+HReg;
	     Move(AdrVals,BAsmCode[2],AdrCnt);
             CodeLen:=2+AdrCnt;
            END;
           ModImm:
            BEGIN
             BAsmCode[0]:=(Code SHL 4)+5+(OpSize*2);
	     BAsmCode[1]:=WorkOfs+HReg+OpSize;
	     Move(AdrVals,BAsmCode[2],AdrCnt);
             CodeLen:=2+AdrCnt;
            END;
           END;
          END;
         ModReg,ModRReg:
          BEGIN
           HReg:=AdrVals[0];
           DecodeAdr(ArgStr[2],Mask2+MModIWReg+MModIWRReg+MModIncWRReg+MModDecWRReg+
                               MModDisp8WRReg+MModDisp16WRReg+MModImm);
           CASE AdrMode OF
           ModReg,ModRReg:
            BEGIN
             BAsmCode[0]:=(Code SHL 4)+4+(OpSize*3); CodeLen:=3;
	     BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=HReg;
            END;
           ModIWReg:
            BEGIN
             BAsmCode[0]:=$a6+65*(1-OpSize);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart;
             BAsmCode[2]:=HReg; CodeLen:=3;
            END;
           ModIWRReg:
            BEGIN
             BAsmCode[0]:=$72+(OpSize*12);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart+(1-OpSize);
             BAsmCode[2]:=HReg; CodeLen:=3;
            END;
           ModIncWRReg:
            BEGIN
             BAsmCode[0]:=$b4+(OpSize*$21);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart+1;
             BAsmCode[2]:=HReg; CodeLen:=3;
            END;
           ModDecWRReg:
            BEGIN
             BAsmCode[0]:=$c2+OpSize;
	     BAsmCode[1]:=(Code SHL 4)+AdrPart+1;
             BAsmCode[2]:=HReg; CodeLen:=3;
            END;
           ModDisp8WRReg:
            BEGIN
             BAsmCode[0]:=$7f+(OpSize*7);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart+1;
             BAsmCode[2]:=AdrVals[0]; BAsmCode[3]:=HReg; CodeLen:=4;
            END;
           ModDisp16WRReg:
            BEGIN
             BAsmCode[0]:=$7f+(OpSize*7);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart;
             Move(AdrVals,BAsmCode[2],AdrCnt); BAsmCode[2+AdrCnt]:=HReg;
	     CodeLen:=3+AdrCnt;
            END;
           ModImm:
            BEGIN
             BAsmCode[0]:=(Code SHL 4)+5+(OpSize*2);
             Move(AdrVals,BAsmCode[2],AdrCnt); BAsmCode[1]:=HReg+OpSize;
	     CodeLen:=2+AdrCnt;
            END;
           END;
          END;
         ModIWReg:
          BEGIN
           HReg:=AdrPart;
           DecodeAdr(ArgStr[2],Mask2);
           CASE AdrMode OF
           ModReg,ModRReg:
            BEGIN
             BAsmCode[0]:=$e6-(OpSize*$50); BAsmCode[1]:=AdrVals[0];
	     BAsmCode[2]:=(Code SHL 4)+HReg; CodeLen:=3;
            END;
           END;
          END;
         ModIWRReg:
          BEGIN
           HReg:=AdrPart;
           DecodeAdr(ArgStr[2],(OpSize*MModWRReg)+Mask2+MModIWRReg+MModImm);
           CASE AdrMode OF
           ModWRReg:
            BEGIN
             BAsmCode[0]:=(Code SHL 4)+$0e;
             BAsmCode[1]:=(HReg SHL 4)+$10+AdrPart; CodeLen:=2;
            END;
           ModReg,ModRReg:
            BEGIN
             BAsmCode[0]:=$72+(OPSize*76);
	     BAsmCode[1]:=(Code SHL 4)+HReg+OpSize;
             BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
            END;
           ModIWRReg:
            IF OpSize=0 THEN
             BEGIN
              BAsmCode[0]:=$73; BAsmCode[1]:=(Code SHL 4)+AdrPart;
              BAsmCode[2]:=HReg+WorkOfs; CodeLen:=3;
             END
            ELSE
             BEGIN
              BAsmCode[0]:=(Code SHL 4)+$0e;
              BAsmCode[1]:=$11+(HReg SHL 4)+AdrPart;
              CodeLen:=2;
             END;
           ModImm:
            BEGIN
             BAsmCode[0]:=$f3-(OpSize*$35);
	     BAsmCode[1]:=(Code SHL 4)+HReg;
             Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
            END;
           END;
          END;
         ModIncWRReg:
          BEGIN
           HReg:=AdrPart;
           DecodeAdr(ArgStr[2],Mask2);
           CASE AdrMode OF
           ModReg,ModRReg:
            BEGIN
             BAsmCode[0]:=$b4+(OpSize*$21);
	     BAsmCode[1]:=(Code SHL 4)+HReg;
             BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
            END;
           END;
          END;
         ModDecWRReg:
          BEGIN
           HReg:=AdrPart;
           DecodeAdr(ArgStr[2],Mask2);
           CASE AdrMode OF
           ModReg,ModRReg:
            BEGIN
             BAsmCode[0]:=$c2+OpSize;
	     BAsmCode[1]:=(Code SHL 4)+HReg;
             BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
            END;
           END;
          END;
         ModDispRWRReg:
          BEGIN
           HReg:=AdrPart; HPart:=AdrVals[0];
           DecodeAdr(ArgStr[2],Mask1);
           CASE AdrMode OF
           ModWReg,ModWRReg:
            BEGIN
             BAsmCode[0]:=$60;
	     BAsmCode[1]:=$11-(OpSize*$10)+(HPart SHL 4)+HReg;
	     BAsmCode[2]:=(Code SHL 4)+AdrPart; CodeLen:=3;
            END;
           END;
          END;
         ModDisp8WRReg:
          BEGIN
           HReg:=AdrPart; BAsmCode[2]:=AdrVals[0];
           DecodeAdr(ArgStr[2],Mask2+(OpSize*MModImm));
           CASE AdrMode OF
           ModReg,ModRReg:
            BEGIN
             BAsmCode[0]:=$26+(OpSize*$60);
	     BAsmCode[1]:=(Code SHL 4)+HReg+1;
             BAsmCode[3]:=AdrVals[0]+OpSize; CodeLen:=4;
            END;
           ModImm:
            BEGIN
             BAsmCode[0]:=$06; BAsmCode[1]:=(Code SHL 4)+HReg+1;
             Move(AdrVals,BAsmCode[3],AdrCnt); CodeLen:=3+AdrCnt;
            END;
           END;
          END;
         ModDisp16WRReg:
          BEGIN
           HReg:=AdrPart; Move(AdrVals,BAsmCode[2],2);
           DecodeAdr(ArgStr[2],Mask2+(OpSize*MModImm));
           CASE AdrMode OF
           ModReg,ModRReg:
            BEGIN
             BAsmCode[0]:=$26+(OpSize*$60);
	     BAsmCode[1]:=(Code SHL 4)+HReg;
             BAsmCode[4]:=AdrVals[0]+OpSize; CodeLen:=5;
            END;
           ModImm:
            BEGIN
             BAsmCode[0]:=$06; BAsmCode[1]:=(Code SHL 4)+HReg;
             Move(AdrVals,BAsmCode[4],AdrCnt); CodeLen:=4+AdrCnt;
            END;
           END;
          END;
         ModAbs:
          BEGIN
           Move(AdrVals,BAsmCode[2],2);
           DecodeAdr(ArgStr[2],Mask1+MModImm);
           CASE AdrMode OF
           ModWReg,ModWRReg:
            BEGIN
             BAsmCode[0]:=$c5+(OpSize*29);
	     BAsmCode[1]:=(Code SHL 4)+AdrPart+OpSize;
             CodeLen:=4;
            END;
           ModImm:
            BEGIN
             Move(BAsmCode[2],BAsmCode[2+AdrCnt],2); Move(AdrVals,BAsmCode[2],AdrCnt);
             BAsmCode[0]:=$2f+(OpSize*7);
	     BAsmCode[1]:=(Code SHL 4)+1;
             CodeLen:=4+AdrCnt;
            END;
           END;
          END;
         ModIRReg:
          BEGIN
           HReg:=AdrVals[0];
           DecodeAdr(Argstr[2],MModIWRReg*(1-OpSize));
           CASE AdrMode OF
           ModIWRReg:
            BEGIN
             BAsmCode[0]:=$73; BAsmCode[1]:=(Code SHL 4)+AdrPart;
             BAsmCode[2]:=HReg; CodeLen:=3;
            END;
           END;
          END;
         END;
        END;
       Exit;
      END;

   FOR z:=0 TO RegOrderCnt-1 DO
    WITH RegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModReg+MModIReg);
         CASE AdrMode OF
         ModReg:
          BEGIN
           BAsmCode[0]:=Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
          END;
         ModIReg:
          BEGIN
           BAsmCode[0]:=Code+1; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
          END;
         END;
        END;
       Exit;
      END;

   FOR z:=0 TO Reg16OrderCnt-1 DO
    WITH Reg16Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModRReg);
         CASE AdrMode OF
         ModRReg:
          BEGIN
           BAsmCode[0]:=Code; BAsmCode[1]:=AdrVals[0]+Ord(Memo('EXT'));
           CodeLen:=2;
          END;
         END;
        END;
       Exit;
      END;

   IF (Memo('DIV')) OR (Memo('MUL')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWRReg);
       IF AdrMode=ModWRReg THEN
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModWReg);
         IF AdrMode=ModWReg THEN
          BEGIN
           BAsmCode[0]:=$5f-($10*Ord(Memo('MUL')));
           BAsmCode[1]:=(HReg SHL 4)+AdrPart;
           CodeLen:=2;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('DIVWS') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWRReg);
       IF AdrMode=ModWRReg THEN
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModWRReg);
         IF AdrMode=ModWRReg THEN
          BEGIN
           BAsmCode[2]:=(HReg SHL 4)+AdrPart;
           DecodeAdr(ArgStr[3],MModRReg);
           IF AdrMode=ModRReg THEN
            BEGIN
             BAsmCode[0]:=$56; BAsmCode[1]:=AdrVals[0];
             CodeLen:=3;
            END;
          END;
        END;
      END;
     Exit;
    END;

   { Bitoperationen }

   FOR z:=0 TO Bit2OrderCnt-1 DO
    WITH Bit2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF SplitBit(ArgStr[1],HReg) THEN
        IF Odd(HReg) THEN WrError(1350)
        ELSE
         BEGIN
          DecodeAdr(ArgStr[1],MModWReg);
          IF AdrMode=ModWReg THEN
           BEGIN
            HReg:=(HReg SHL 4)+AdrPart;
            IF SplitBit(ArgStr[2],HPart) THEN
             BEGIN
              DecodeAdr(ArgStr[2],MModWReg);
              IF AdrMode=ModWReg THEN
               BEGIN
                HPart:=(HPart SHL 4)+AdrPart;
                BAsmCode[0]:=Code;
                IF Memo('BLD') THEN
                 BEGIN
                  BAsmCode[1]:=HPart OR $10; BAsmCode[2]:=HReg OR (HPart AND $10);
                 END
                ELSE IF Memo('BXOR') THEN
                 BEGIN
                  BAsmCode[1]:=$10+HReg; BasmCode[2]:=HPart;
                 END
                ELSE
                 BEGIN
                  BAsmCode[1]:=$10+HReg; BasmCode[2]:=HPart XOR $10;
                 END;
                CodeLen:=3;
               END;
             END;
           END;
         END;
       Exit;
      END;

   FOR z:=0 TO Bit1OrderCnt-1 DO
    WITH Bit1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF SplitBit(ArgStr[1],HReg) THEN
        IF Odd(HReg) THEN WrError(1350)
        ELSE
         BEGIN
          DecodeAdr(ArgStr[1],MModWReg+(Ord(Memo('BTSET'))*MModIWRReg));
          CASE AdrMode OF
          ModWReg:
           BEGIN
            BAsmCode[0]:=Code; BAsmCode[1]:=(HReg SHL 4)+AdrPart;
            CodeLen:=2;
           END;
          ModIWRReg:
           BEGIN
            BAsmCode[0]:=$f6; BAsmCode[1]:=(HReg SHL 4)+AdrPart;
            CodeLen:=2;
           END;
          END;
         END;
       Exit;
      END;

   { SprÅnge }

   IF (Memo('BTJF')) OR (Memo('BTJT')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF SplitBit(ArgStr[1],HReg) THEN
      IF Odd(HReg) THEN WrError(1350)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[1],MModWReg);
        IF AdrMode=ModWReg THEN
         BEGIN
          BAsmCode[1]:=(HReg SHL 4)+AdrPart+(Ord(Memo('BTJF')) SHL 4);
          AdrInt:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+3);
          IF OK THEN
           IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
           ELSE
            BEGIN
             BAsmCode[0]:=$af; BAsmCode[2]:=AdrInt AND $ff; CodeLen:=3;
             ChkSpace(SegCode);
            END;
         END;
       END;
     Exit;
    END;

   IF (Memo('JP')) OR (Memo('CALL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AbsSeg:=SegCode;
       DecodeAdr(ArgStr[1],MModIRReg+MModAbs);
       CASE AdrMode OF
       ModIRReg:
        BEGIN
         BAsmCode[0]:=$74+(Ord(Memo('JP'))*$60);
         BAsmCode[1]:=AdrVals[0]+Ord(Memo('CALL')); CodeLen:=2;
        END;
       ModAbs:
        BEGIN
         BAsmCode[0]:=$8d+(Ord(Memo('CALL'))*$45);
         Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('CPJFI')) OR (Memo('CPJTI')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg);
       IF AdrMode=ModWReg THEN
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModIWRReg);
         IF AdrMode=ModIWRReg THEN
          BEGIN
           BAsmCode[1]:=(AdrPart SHL 4)+(Ord(Memo('CPJTI')) SHL 4)+HReg;
           AdrInt:=EvalIntExpression(ArgStr[3],UInt16,OK)-(EProgCounter+3);
           IF OK THEN
            IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
            ELSE
             BEGIN
              ChkSpace(SegCode);
              BAsmCode[0]:=$9f; BAsmCode[2]:=AdrInt AND $ff;
              CodeLen:=3;
             END;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg);
       IF AdrMode=ModWReg THEN
        BEGIN
         BAsmCode[0]:=(AdrPart SHL 4)+$0a;
         AdrInt:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+2);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
          ELSE
           BEGIN
            ChkSpace(SegCode);
            BAsmCode[1]:=AdrInt AND $ff;
            CodeLen:=2;
           END;
        END;
      END;
     Exit;
    END;

   IF Memo('DWJNZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModRReg);
       IF AdrMode=ModRReg THEN
        BEGIN
         BAsmCode[1]:=AdrVals[0];
         AdrInt:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+3);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
          ELSE
           BEGIN
            ChkSpace(SegCode);
            BAsmCode[0]:=$c6; BAsmCode[2]:=AdrInt AND $ff;
            CodeLen:=3;
           END;
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO ConditionCnt-1 DO
    WITH Conditions^[z] DO
     IF Memo('JP'+Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         AdrWord:=EvalIntExpression(ArgStr[1],UInt16,OK);
         IF OK THEN
          BEGIN
           ChkSpace(SegCode);
           BAsmCode[0]:=$0d+(Code SHL 4);
           BAsmCode[1]:=Hi(AdrWord); BAsmCode[2]:=Lo(AdrWord);
           CodeLen:=3;
          END;
        END;
       Exit;
      END
     ELSE IF Memo('JR'+Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
          ELSE
           BEGIN
            ChkSpace(SegCode);
            BAsmCode[0]:=$0b+(Code SHL 4); BAsmCode[1]:=AdrInt AND $ff;
            CodeLen:=2;
           END;
        END;
       Exit;
      END;

   { Besonderheiten }

   IF Memo('SPP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=(EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt6,OK) SHL 2)+$02;
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$c7; CodeLen:=2;
        END;
      END;
     Exit;
    END;

   IF (Memo('SRP')) OR (Memo('SRP0')) OR (Memo('SRP1')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt5,OK) SHL 3;
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$c7; CodeLen:=2;
         IF Length(OpPart)=4 THEN Inc(BAsmCode[1],4);
         IF OpPart[Length(OpPart)]='1' THEN Inc(BAsmCode[1]);
        END;
      END;
     Exit;
    END;

   { Fakes... }

   IF Memo('SLA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg+MModReg+MModIWRReg);
       CASE AdrMode OF
       ModWReg:
        BEGIN
         BAsmCode[0]:=$42; BAsmCode[1]:=(AdrPart SHL 4)+AdrPart;
         CodeLen:=2;
        END;
       ModReg:
        BEGIN
         BAsmCode[0]:=$44;
         BAsmCode[1]:=AdrVals[0];
         BAsmCode[2]:=AdrVals[0];
         CodeLen:=3;
        END;
       ModIWRReg:
        BEGIN
         BAsmCode[0]:=$73; BAsmCode[1]:=$40+AdrPart;
         BAsmCode[2]:=WorkOfs+AdrPart; CodeLen:=3;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('SLAW') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWRReg+MModRReg+MModIWRReg);
       CASE AdrMode OF
       ModWRReg:
        BEGIN
         BAsmCode[0]:=$4e; BAsmCode[1]:=(AdrPart SHL 4)+AdrPart;
         CodeLen:=2;
        END;
       ModRReg:
        BEGIN
         BAsmCode[0]:=$47;
         BAsmCode[1]:=AdrVals[0];
         BAsmCode[2]:=AdrVals[0];
         CodeLen:=3;
        END;
       ModIWRReg:
        BEGIN
         BAsmCode[0]:=$4e; BAsmCode[1]:=$11+(AdrPart SHL 4)+AdrPart;
         CodeLen:=2;
        END;
       END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        PROCEDURE InitCode_ST9;
        Far;
BEGIN
   SaveInitProc;

   DPAssume:=0;
END;

        FUNCTION ChkPC_ST9:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode,SegData: ok:=ProgCounter<$10000;
   SegReg: ok:=ProgCounter < $100;
   ELSE ok:=False;
   END;
   ChkPC_ST9:=(ok) AND (ProgCounter>=0);
END;


        FUNCTION IsDef_ST9:Boolean;
	Far;
BEGIN
   IsDef_ST9:=Memo('REG') OR Memo('BIT');
END;

        PROCEDURE SwitchFrom_ST9;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE InternSymbol_ST9(VAR Asc:String; VAR Erg:TempResult);
        Far;
VAR
   h:String;
   err:ValErgType;
   Pair:Boolean;
BEGIN
   Erg.Typ:=TempNone;
   IF (Length(Asc)<2) OR (Asc[1]<>'R') THEN Exit;

   h:=Copy(Asc,2,Length(Asc)-1);
   IF h[1]='R' THEN
    BEGIN
     IF Length(h)<2 THEN Exit;
     Pair:=True; Delete(h,1,1);
    END
   ELSE Pair:=False;
   Val(h,Erg.Int,err);
   IF (Err<>0) OR (Erg.Int<0) OR (Erg.Int>255) THEN Exit;
   IF (Erg.Int AND $f0=$d0) THEN Exit;
   IF (Pair) AND (Odd(Erg.Int)) THEN Exit;

   IF Pair THEN Inc(Erg.Int,$100);
   Erg.Typ:=TempInt; TypeFlag:=TypeFlag OR (1 SHL SegReg);
END;

        PROCEDURE SwitchTo_ST9;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='PC'; HeaderID:=$32; NOPCode:=$ff;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData,SegReg];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;
   Grans[SegReg ]:=1; ListGrans[SegReg ]:=1; SegInits[SegReg ]:=0;

   MakeCode:=MakeCode_ST9; ChkPC:=ChkPC_ST9; IsDef:=IsDef_ST9;
   SwitchFrom:=SwitchFrom_ST9; InternSymbol:=InternSymbol_ST9;

   InitFields;
END;

BEGIN
   CPUST9020:=AddCPU('ST9020',SwitchTo_ST9);
   CPUST9030:=AddCPU('ST9030',SwitchTo_ST9);
   CPUST9040:=AddCPU('ST9040',SwitchTo_ST9);
   CPUST9050:=AddCPU('ST9050',SwitchTo_ST9);
   SaveInitProc:=InitPAssProc; InitPassProc:=InitCode_ST9;
END.


