{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code85;

{ AS Codegeneratormodul 8080/8085 }

INTERFACE
        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
               Name:String[4];
               May80:Boolean;
               Code:Byte;
              END;

   BaseOrder=RECORD
              Name:String[4];
	      Code:Byte;
             END;

CONST
   FixedOrderCnt=27;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
               ((Name:'XCHG'; May80:True ; Code:$eb),
                (Name:'XTHL'; May80:True ; Code:$e3),
                (Name:'SPHL'; May80:True ; Code:$f9),
                (Name:'PCHL'; May80:True ; Code:$e9),
                (Name:'RET' ; May80:True ; Code:$c9),
                (Name:'RC'  ; May80:True ; Code:$d8),
                (Name:'RNC' ; May80:True ; Code:$d0),
		(Name:'RZ'  ; May80:True ; Code:$c8),
                (Name:'RNZ' ; May80:True ; Code:$c0),
                (Name:'RP'  ; May80:True ; Code:$f0),
                (Name:'RM'  ; May80:True ; Code:$f8),
                (Name:'RPE' ; May80:True ; Code:$e8),
                (Name:'RPO' ; May80:True ; Code:$e0),
                (Name:'RLC' ; May80:True ; Code:$07),
                (Name:'RRC' ; May80:True ; Code:$0f),
                (Name:'RAL' ; May80:True ; Code:$17),
                (Name:'RAR' ; May80:True ; Code:$1f),
                (Name:'CMA' ; May80:True ; Code:$2f),
                (Name:'STC' ; May80:True ; Code:$37),
                (Name:'CMC' ; May80:True ; Code:$3f),
		(Name:'DAA' ; May80:True ; Code:$27),
                (Name:'EI'  ; May80:True ; Code:$fb),
                (Name:'DI'  ; May80:True ; Code:$f3),
                (Name:'NOP' ; May80:True ; Code:$00),
                (Name:'HLT' ; May80:True ; Code:$76),
                (Name:'RIM' ; May80:False; Code:$20),
                (Name:'SIM' ; May80:False; Code:$30));

   Op16OrderCnt=22;
   Op16Orders:ARRAY[1..Op16OrderCnt] OF BaseOrder=
              ((Name:'STA' ; Code:$32),
               (Name:'LDA' ; Code:$3a),
               (Name:'SHLD'; Code:$22),
	       (Name:'LHLD'; Code:$2a),
               (Name:'JMP' ; Code:$c3),
               (Name:'JC'  ; Code:$da),
               (Name:'JNC' ; Code:$d2),
               (Name:'JZ'  ; Code:$ca),
               (Name:'JNZ' ; Code:$c2),
               (Name:'JP'  ; Code:$f2),
               (Name:'JM'  ; Code:$fa),
               (Name:'JPE' ; Code:$ea),
               (Name:'JPO' ; Code:$e2),
               (Name:'CALL'; Code:$cd),
               (Name:'CC'  ; Code:$dc),
               (Name:'CNC' ; Code:$d4),
	       (Name:'CZ'  ; Code:$cc),
               (Name:'CNZ' ; Code:$c4),
               (Name:'CP'  ; Code:$f4),
               (Name:'CM'  ; Code:$fc),
               (Name:'CPE' ; Code:$ec),
               (Name:'CPO' ; Code:$e4));

   Op8OrderCnt=10;
   Op8Orders:ARRAY[1..Op8OrderCnt] OF BaseOrder=
             ((Name:'IN'  ; Code:$db),
              (Name:'OUT' ; Code:$d3),
              (Name:'ADI' ; Code:$c6),
              (Name:'ACI' ; Code:$ce),
	      (Name:'SUI' ; Code:$d6),
              (Name:'SBI' ; Code:$de),
              (Name:'ANI' ; Code:$e6),
              (Name:'XRI' ; Code:$ee),
              (Name:'ORI' ; Code:$f6),
              (Name:'CPI' ; Code:$fe));

   ALUOrderCnt=8;
   ALUOrders:ARRAY[1..ALUOrderCnt] OF BaseOrder=
             ((Name:'ADD' ; Code:$80),
              (Name:'ADC' ; Code:$88),
              (Name:'SUB' ; Code:$90),
              (Name:'SBB' ; Code:$98),
	      (Name:'ANA' ; Code:$a0),
	      (Name:'XRA' ; Code:$a8),
	      (Name:'ORA' ; Code:$b0),
	      (Name:'CMP' ; Code:$b8));

VAR
   CPU8080,CPU8085:CPUVar;

        FUNCTION DecodeReg8(Asc:String; VAR Erg:Byte):Boolean;
CONST
   RegNames:String[8]='BCDEHLMA';
VAR
   p:Byte;
BEGIN
   IF Length(Asc)<>1 THEN DecodeReg8:=False
   ELSE
    BEGIN
     p:=Pos(UpCase(Asc[1]),RegNames);
     IF p=0 THEN DecodeReg8:=False
     ELSE
      BEGIN
       DecodeReg8:=True; Erg:=Pred(p);
      END;
    END;
END;

	FUNCTION DecodeReg16(Asc:String; Var Erg:Byte):Boolean;
CONST
   RegNames:ARRAY[0..3] OF String[2]=('B','D','H','SP');
VAR
   z:Byte;
BEGIN
   NLS_UpString(Asc);
   DecodeReg16:=False;
   FOR z:=0 TO 3 DO
    IF Asc=RegNames[z] THEN
     BEGIN
      DecodeReg16:=True;
      Erg:=z;
     END;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   HLocHandle:LongInt;
   OK:Boolean;
   AdrByte:Byte;
BEGIN
   DecodePseudo:=True;

   IF Memo('PORT') THEN
    BEGIN
     CodeEquate(SegIO,0,$ff);
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_85;
	Far;
VAR
   OK:Boolean;
   AdrWord:Word;
   AdrByte:Byte;
   z:Integer;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { Anweisungen ohne Operanden }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF (MomCPU<CPU8085) AND (NOT May80) THEN WrError(1500)
       ELSE
        BEGIN
         CodeLen:=1; BAsmCode[0]:=Code;
        END;
       Exit;
      END;

   { ein 16-Bit-Operand }

   FOR z:=1 TO Op16OrderCnt DO
    WITH Op16Orders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
         IF OK THEN
          BEGIN
           CodeLen:=3; BAsmCode[0]:=Code;
           BAsmCode[1]:=Lo(AdrWord); BAsmCode[2]:=Hi(AdrWord);
           ChkSpace(SegCode);
          END;
        END;
       Exit;
      END;

   FOR z:=1 TO Op8OrderCnt DO
    WITH Op8Orders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         AdrByte:=EvalIntExpression(ArgStr[1],Int8,OK);
         IF OK THEN
          BEGIN
           CodeLen:=2; BAsmCode[0]:=Code; BAsmCode[1]:=AdrByte;
           IF z<=2 THEN ChkSpace(SegIO);
          END;
        END;
       Exit;
      END;

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg8(ArgStr[1],AdrByte) THEN WrError(1980)
     ELSE IF NOT DecodeReg8(ArgStr[2],BAsmCode[0]) THEN WrError(1980)
     ELSE
      BEGIN
       Inc(BAsmCode[0],$40+(AdrByte SHL 3));
       IF BAsmCode[0]=$76 THEN WrError(1760) ELSE CodeLen:=1;
      END;
     Exit;
    END;

   IF Memo('MVI') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
        IF NOT DecodeReg8(ArgStr[1],AdrByte) THEN WrError(1980)
        ELSE
         BEGIN
          BAsmCode[0]:=$06+(AdrByte SHL 3); CodeLen:=2;
         END;
      END;
     Exit;
    END;

   IF Memo('LXI') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[2],Int16,OK);
       IF OK THEN
        IF NOT DecodeReg16(ArgStr[1],AdrByte) THEN WrError(1980)
        ELSE
         BEGIN
          BAsmCode[0]:=$01+(AdrByte SHL 4);
          BAsmCode[1]:=Lo(AdrWord); BAsmCode[2]:=Hi(AdrWord);
	  CodeLen:=3;
         END;
      END;
     Exit;
    END;

   IF (Memo('LDAX')) OR (Memo('STAX')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT DecodeReg16(ArgStr[1],AdrByte) THEN WrError(1980)
     ELSE CASE AdrByte OF
          3:WrError(1350);       { SP }
          2:BEGIN               { H --> MOV A,M oder M,A }
             CodeLen:=1;
             IF Memo('LDAX') THEN BAsmCode[0]:=$7e ELSE BAsmCode[0]:=$77;
            END;
          ELSE
           BEGIN
            CodeLen:=1;
            BAsmCode[0]:=$02+(AdrByte SHL 4);
            IF Memo('LDAX') THEN Inc(BasmCode[0],8);
           END;
          END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[1],'PSW')=0 THEN ArgStr[1]:='SP';
       IF NOT DecodeReg16(ArgStr[1],AdrByte) THEN WrError(1980)
       ELSE
        BEGIN
         CodeLen:=1; BAsmCode[0]:=$c1+(AdrByte SHL 4);
        END;
       IF (CodeLen<>0) AND (Memo('PUSH')) THEN Inc(BAsmCode[0],4);
      END;
     Exit;
    END;

   IF Memo('RST') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrByte:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF OK THEN
        BEGIN
         CodeLen:=1; BAsmCode[0]:=$c7+(AdrByte SHL 3);
        END;
      END;
     Exit;
    END;

   IF (Memo('INR')) OR (Memo('DCR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT DecodeReg8(ArgStr[1],AdrByte) THEN WrError(1980)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$04+(AdrByte SHL 3);
       IF Memo('DCR') THEN Inc(BAsmCode[0]);
      END;
     Exit;
    END;

   IF (Memo('INX')) OR (Memo('DCX')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT DecodeReg16(ArgStr[1],AdrByte) THEN WrError(1980)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$03+(AdrByte SHL 4);
       IF Memo('DCX') THEN Inc(BAsmCode[0],8);
      END;
     Exit;
    END;

   IF (Memo('DAD')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT DecodeReg16(ArgStr[1],AdrByte) THEN WrError(1980)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$09+(AdrByte SHL 4);
      END;
     Exit;
    END;

   FOR z:=1 TO ALUOrderCnt DO
    WITH ALUOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT DecodeReg8(ArgStr[1],AdrByte) THEN WrError(1980)
       ELSE
        BEGIN
         CodeLen:=1; BAsmCode[0]:=Code+AdrByte;
        END;
       Exit;
      END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_85:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter < $10000;
   SegIO    : ok:=ProgCounter < $100;
   ELSE ok:=False;
   END;
   ChkPC_85:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_85:Boolean;
	Far;
BEGIN
   IsDef_85:=OpPart='PORT';
END;

        PROCEDURE SwitchFrom_85;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_85;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$41; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegIO];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegIO  ]:=1; ListGrans[SegIO  ]:=1; SegInits[SegIO  ]:=0;

   MakeCode:=MakeCode_85; ChkPC:=ChkPC_85; IsDef:=IsDef_85;
   SwitchFrom:=SwitchFrom_85;
END;

BEGIN
   CPU8080:=AddCPU('8080',SwitchTo_85);
   CPU8085:=AddCPU('8085',SwitchTo_85);
END.
