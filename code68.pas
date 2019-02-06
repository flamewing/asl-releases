{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
	Unit Code68;

Interface

	Uses NLS,
	     AsmDef,AsmPars,AsmSub,CodePseu;


Implementation

TYPE
   FixedOrder=RECORD
	       Name:String[4];
	       MinCPU,MaxCPU:CPUVar;
	       Code:Word;
	      END;

   BaseOrder=RECORD
	      Name:String[3];
	      Code:Byte;
	     END;

   ALU8Order=RECORD
	      Name:String[3];
	      MayImm:Boolean;
	      Code:Byte;
	     END;

   ALU16Order=RECORD
	       Name:String[4];
	       MayImm:Boolean;
	       MinCPU:CPUVar;    { Shift  andere   ,Y   }
	       PageShift:Byte;   { 0 :     nix    Pg 2  }
	       Code:Byte;        { 1 :     Pg 3   Pg 4  }
	      END;               { 2 :     nix    Pg 4  }
				 { 3 :     Pg 2   Pg 3  }
CONST
   D_CPU6800=0;
   D_CPU6301=1;
   D_CPU6811=2;

   ModNone=-1;
   ModAcc=1;
   ModDir=2;
   ModExt=3;
   ModInd=4;
   ModImm=5;

   Page2Prefix=$18;
   Page3Prefix=$1a;
   Page4Prefix=$cd;

   FixedOrderCnt=46;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'ABA'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$001b),
		(Name:'ABX'  ; MinCPU:D_CPU6301; MaxCPU:D_CPU6811; Code:$003a),
		(Name:'ABY'  ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$183a),
		(Name:'ASLD' ; MinCPU:D_CPU6301; MaxCPU:D_CPU6811; Code:$0005),
		(Name:'LSLD' ; MinCPU:D_CPU6301; MaxCPU:D_CPU6811; Code:$0005),
		(Name:'CBA'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0011),
		(Name:'CLC'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$000c),
		(Name:'CLI'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$000e),
		(Name:'CLV'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$000a),
		(Name:'DAA'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0019),
		(Name:'DES'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0034),
		(Name:'DEX'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0009),
		(Name:'DEY'  ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$1809),
		(Name:'INS'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0031),
		(Name:'INX'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0008),
		(Name:'INY'  ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$1808),
		(Name:'LSRD' ; MinCPU:D_CPU6301; MaxCPU:D_CPU6811; Code:$0004),
		(Name:'MUL'  ; MinCPU:D_CPU6301; MaxCPU:D_CPU6811; Code:$003d),
		(Name:'NOP'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0001),
		(Name:'PSHX' ; MinCPU:D_CPU6301; MaxCPU:D_CPU6811; Code:$003c),
		(Name:'PSHY' ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$183c),
		(Name:'PULX' ; MinCPU:D_CPU6301; MaxCPU:D_CPU6811; Code:$0038),
		(Name:'PULY' ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$1838),
		(Name:'RTI'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$003b),
		(Name:'RTS'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0039),
		(Name:'SBA'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0010),
		(Name:'SEC'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$000d),
		(Name:'SEI'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$000f),
		(Name:'SEV'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$000b),
		(Name:'SLP'  ; MinCPU:D_CPU6301; MaxCPU:D_CPU6301; Code:$001a),
		(Name:'SWI'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$003f),
		(Name:'STOP' ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$00cf),
		(Name:'TAB'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0016),
		(Name:'TAP'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0006),
		(Name:'TBA'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0017),
		(Name:'TPA'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0007),
		(Name:'TSX'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0030),
		(Name:'TSY'  ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$1830),
		(Name:'TXS'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$0035),
		(Name:'TYS'  ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$1835),
		(Name:'WAI'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$003e),
		(Name:'XGDX' ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$008f),
		(Name:'SEI'  ; MinCPU:D_CPU6800; MaxCPU:D_CPU6811; Code:$000f),
		(Name:'XGDY' ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$188f),
		(Name:'IDIV' ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$0002),
		(Name:'FDIV' ; MinCPU:D_CPU6811; MaxCPU:D_CPU6811; Code:$0003));

   RelOrderCnt=19;
   RelOrders:ARRAY[1..RelOrderCnt] OF BaseOrder=
	     ((Name:'BRA' ; Code:$20),
	      (Name:'BRN' ; Code:$21),
	      (Name:'BHI' ; Code:$22),
	      (Name:'BLS' ; Code:$23),
	      (Name:'BHS' ; Code:$24),
	      (Name:'BCC' ; Code:$24),
	      (Name:'BLO' ; Code:$25),
	      (Name:'BCS' ; Code:$25),
	      (Name:'BNE' ; Code:$26),
	      (Name:'BEQ' ; Code:$27),
	      (Name:'BVC' ; Code:$28),
	      (Name:'BVS' ; Code:$29),
	      (Name:'BPL' ; Code:$2a),
	      (Name:'BMI' ; Code:$2b),
	      (Name:'BGE' ; Code:$2c),
	      (Name:'BLT' ; Code:$2d),
	      (Name:'BGT' ; Code:$2e),
	      (Name:'BLE' ; Code:$2f),
	      (Name:'BSR' ; Code:$8d));

   ALU8OrderCnt=11;
   ALU8Orders:ARRAY[1..ALU8OrderCnt] OF ALU8Order=
	      ((Name:'SUB' ; MayImm:TRUE ; Code:$80),
	       (Name:'CMP' ; MayImm:TRUE ; Code:$81),
	       (Name:'SBC' ; MayImm:TRUE ; Code:$82),
	       (Name:'AND' ; MayImm:TRUE ; Code:$84),
	       (Name:'BIT' ; MayImm:TRUE ; Code:$85),
	       (Name:'LDA' ; MayImm:TRUE ; Code:$86),
	       (Name:'STA' ; MayImm:FALSE; Code:$87),
	       (Name:'EOR' ; MayImm:TRUE ; Code:$88),
	       (Name:'ADC' ; MayImm:TRUE ; Code:$89),
	       (Name:'ORA' ; MayImm:TRUE ; Code:$8a),
	       (Name:'ADD' ; MayImm:TRUE ; Code:$8b));

   ALU16OrderCnt=13;
   ALU16Orders:ARRAY[1..ALU16OrderCnt] OF ALU16Order=
	       ((Name:'ADDD'; MayImm:True ; MinCPU:D_CPU6301; PageShift:0; Code:$c3),
		(Name:'SUBD'; MayImm:True ; MinCPU:D_CPU6301; PageShift:0; Code:$83),
		(Name:'LDD' ; MayImm:True ; MinCPU:D_CPU6301; PageShift:0; Code:$cc),
		(Name:'STD' ; MayImm:False; MinCPU:D_CPU6301; PageShift:0; Code:$cd),
		(Name:'CPD' ; MayImm:True ; MinCPU:D_CPU6811; PageShift:1; Code:$83),
		(Name:'CPX' ; MayImm:True ; MinCPU:D_CPU6800; PageShift:2; Code:$8c),
		(Name:'CPY' ; MayImm:True ; MinCPU:D_CPU6811; PageShift:3; Code:$8c),
		(Name:'LDS' ; MayImm:True ; MinCPU:D_CPU6800; PageShift:0; Code:$8e),
		(Name:'STS' ; MayImm:False; MinCPU:D_CPU6800; PageShift:0; Code:$8f),
		(Name:'LDX' ; MayImm:True ; MinCPU:D_CPU6800; PageShift:2; Code:$ce),
		(Name:'STX' ; MayImm:False; MinCPU:D_CPU6800; PageShift:2; Code:$cf),
		(Name:'LDY' ; MayImm:True ; MinCPU:D_CPU6811; PageShift:3; Code:$ce),
		(Name:'STY' ; MayImm:False; MinCPU:D_CPU6811; PageShift:3; Code:$cf));

   Bit63OrderCnt=4;
   Bit63Orders:ARRAY[1..Bit63OrderCnt] OF BaseOrder=
	       ((Name:'AIM'; Code:$61),
		(Name:'EIM'; Code:$65),
		(Name:'OIM'; Code:$62),
		(Name:'TIM'; Code:$6b));

   Sing8OrderCnt=12;
   Sing8Orders:ARRAY[1..Sing8OrderCnt] OF BaseOrder=
	       ((Name:'ASL'; Code:$48),
		(Name:'LSL'; Code:$48),
		(Name:'ASR'; Code:$47),
		(Name:'CLR'; Code:$4f),
		(Name:'COM'; Code:$43),
		(Name:'DEC'; Code:$4a),
		(Name:'INC'; Code:$4c),
		(Name:'LSR'; Code:$44),
		(Name:'NEG'; Code:$40),
		(Name:'ROL'; Code:$49),
		(Name:'ROR'; Code:$46),
		(Name:'TST'; Code:$4d));

VAR
   OpSize:ShortInt;         { TRUE = 16 Bit, FALSE = 8 Bit }
   PrefCnt:Byte;            { Anzahl BefehlsprÑfixe }
   AdrMode:ShortInt;        { Ergebnisadre·modus }
   AdrPart:Byte;            { Adressierungsmodusbits im Opcode }
   AdrCnt:Byte;             { Anzahl Bytes Adressargument }
   AdrVals:ARRAY[0..3] OF Byte; { Adre·argument }

   CPU6800,CPU6301,CPU6811:CPUVar;


	FUNCTION SplitAcc(Op:String):Boolean;
VAR
   Ch:Char;
   z:Integer;
BEGIN
   Ch:=OpPart[Length(OpPart)];
   IF (Length(Op)+1=Length(OpPart)) AND
      (Copy(OpPart,1,Length(Op))=Op) AND
      ((Ch='A') OR (Ch='B')) THEN
    BEGIN
     FOR z:=ArgCnt DOWNTO 1 DO ArgStr[z+1]:=ArgStr[z];
     ArgStr[1]:=Ch;
     Dec(Byte(OpPart[0])); Inc(ArgCnt);
    END;
   SplitAcc:=Memo(Op);
END;

	PROCEDURE DecodeAdr(StartInd,StopInd:Integer; Erl:Byte);
VAR
   Asc:String;
   OK,ErrOcc:Boolean;
   AdrWord:Word;
   Bit8:Byte;
BEGIN
   AdrMode:=ModNone; AdrPart:=0; Asc:=ArgStr[StartInd]; ErrOcc:=False;

   { eine Komponente ? }

   IF StartInd=StopInd THEN
    BEGIN

     { Akkumulatoren ? }

     IF NLS_StrCaseCmp(Asc,'A')=0 THEN
      BEGIN
       IF 1 AND erl<>0 THEN AdrMode:=ModAcc;
      END
     ELSE IF NLS_StrCaseCmp(Asc,'B')=0 THEN
      BEGIN
       IF 1 AND erl<>0 THEN
	BEGIN
	 AdrMode:=ModAcc; AdrPart:=1;
	END
      END

     { immediate ? }

     ELSE IF (Length(Asc)>1) AND (Asc[1]='#') THEN
      BEGIN
       IF 16 AND erl<>0 THEN
	BEGIN
	 Delete(Asc,1,1);
	 IF OpSize=1 THEN
	  BEGIN
	   AdrWord:=EvalIntExpression(Asc,Int16,OK);
	   IF OK THEN
	    BEGIN
	     AdrMode:=ModImm;
	     AdrVals[AdrCnt]:=Hi(AdrWord); AdrVals[AdrCnt+1]:=Lo(AdrWord);
	     Inc(AdrCnt,2);
	    END
	   ELSE ErrOcc:=True;
	  END
	 ELSE
	  BEGIN
	   AdrVals[AdrCnt]:=EvalIntExpression(Asc,Int8,OK);
	   IF OK THEN
	    BEGIN
	     AdrMode:=ModImm; Inc(AdrCnt);
	    END
	   ELSE ErrOcc:=True;
	  END;
	END;
      END

     { absolut ? }

     ELSE
      BEGIN
       Bit8:=0;
       IF Asc[1]='<' THEN
	BEGIN
	 Bit8:=2; Delete(Asc,1,1);
	END
       ELSE IF Asc[1]='>' THEN
	BEGIN
	 Bit8:=1; Delete(Asc,1,1);
	END;
       IF (Bit8=2) OR (Erl AND 4=0) THEN
	AdrWord:=EvalIntExpression(Asc,Int8,OK)
       ELSE
	AdrWord:=EvalIntExpression(Asc,Int16,OK);
       IF OK THEN
	BEGIN
	 IF (2 AND Erl<>0) AND (Bit8<>1) AND ((Bit8=2) OR (4 AND erl=0) OR (Hi(AdrWord)=0)) THEN
	  BEGIN
	   IF Hi(AdrWord)<>0 THEN
	    BEGIN
	     WrError(1340); ErrOcc:=True;
	    END
	   ELSE
	    BEGIN
	     AdrMode:=ModDir; AdrPart:=1;
	     AdrVals[AdrCnt]:=Lo(AdrWord);
	     Inc(AdrCnt);
	    END;
	  END
	 ELSE IF 4 AND erl<>0 THEN
	  BEGIN
	   AdrMode:=ModExt; AdrPart:=3;
	   AdrVals[AdrCnt]:=Hi(AdrWord); AdrVals[AdrCnt+1]:=Lo(AdrWord);
	   Inc(AdrCnt,2);
	  END;
	END
       ELSE ErrOcc:=True;
      END;
    END

   { zwei Komponenten ? }

   ELSE IF StartInd+1=StopInd THEN
    BEGIN

     { indiziert ? }

     IF ((NLS_StrCaseCmp(ArgStr[StopInd],'X')=0) OR (NLS_StrCaseCmp(ArgStr[StopInd],'Y')=0)) THEN
      BEGIN
       IF 8 AND erl<>0 THEN
	BEGIN
	 AdrWord:=EvalIntExpression(Asc,Int8,OK);
	 IF OK THEN
	  IF (MomCPU<CPU6811) AND (NLS_StrCaseCmp(ArgStr[StartInd+1],'Y')=0) THEN
	   BEGIN
	    WrError(1505); ErrOcc:=True;
	   END
	  ELSE
	   BEGIN
	    AdrVals[AdrCnt]:=Lo(AdrWord); Inc(AdrCnt);
	    AdrMode:=ModInd; AdrPart:=2;
	    IF NLS_StrCaseCmp(ArgStr[StartInd+1],'Y')=0 THEN
	     BEGIN
	      BAsmCode[PrefCnt]:=$18; Inc(PrefCnt);
	     END;
	   END
	  ELSE ErrOcc:=True;
	END;
      END
     ELSE
      BEGIN
       WrXError(1445,ArgStr[StopInd]); ErrOcc:=True;
      END

    END
   ELSE
    BEGIN
     WrError(1110); ErrOcc:=True;
    END;

   IF (NOT ErrOcc) AND (AdrMode=ModNone) THEN WrError(1350);
END;

	PROCEDURE AddPrefix(Prefix:Byte);
BEGIN
   BAsmCode[PrefCnt]:=Prefix; Inc(PrefCnt);
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   z,z2:Integer;
   OK:Boolean;
   HVal8:ShortInt;
   HVal16:Integer;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	PROCEDURE Try2Split(Src:Integer);
VAR
   p,z:Integer;
BEGIN
   KillPrefBlanks(ArgStr[Src]); KillPostBlanks(ArgStr[Src]);
   p:=Length(ArgStr[Src]);
   WHILE (p>=1) AND (NOT IsBlank(ArgStr[Src][p])) DO Dec(p);
   IF p>0 THEN
    BEGIN
     FOR z:=ArgCnt DOWNTO Src DO ArgStr[z+1]:=ArgStr[z]; Inc(ArgCnt);
     Byte(ArgStr[Src][0]):=p-1; Delete(ArgStr[Src+1],1,p);
     KillPostBlanks(ArgStr[Src]); KillPrefBlanks(ArgStr[Src+1]);
    END;
END;

	PROCEDURE MakeCode_68;
	Far;
VAR
   z,AdrInt:Integer;
   OK:Boolean;
   AdrByte,Mask:Byte;
BEGIN
   CodeLen:=0; DontPrint:=False; PrefCnt:=0; AdrCnt:=0; OpSize:=0;

   { Operandengrî·e festlegen }

   IF AttrPart<>'' THEN
   CASE UpCase(AttrPart[1]) OF
   'B':OpSize:=0;
   'W':OpSize:=1;
   'L':OpSize:=2;
   'Q':OpSize:=3;
   'S':OpSize:=4;
   'D':OpSize:=5;
   'X':OpSize:=6;
   'P':OpSize:=7;
   ELSE
    BEGIN
     WrError(1107); Exit;
    END;
   END;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeMotoPseudo(True) THEN Exit;
   IF DecodeMoto16Pseudo(OpSize,True) THEN Exit;

   { Anweisungen ohne Argument }

   { Sonderfall : XGDX hat anderen Code bei 6301 !!!! }

   IF (MomCPU=CPU6301) AND (Memo('XGDX')) THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$18;
      END;
     Exit;
    END;

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF (MomCPU-CPU6800<MinCPU) OR (MomCPU-CPU6800>MaxCPU) THEN WrError(1500)
       ELSE IF Hi(Code)<>0 THEN
	BEGIN
	 CodeLen:=2; WAsmCode[0]:=Swap(Code);
	END
       ELSE
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=Lo(Code);
	END;
       Exit;
      END;

   { rel. SprÅnge }

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK);
	 IF OK THEN
	  BEGIN
	   Dec(AdrInt,EProgCounter+2);
	   IF (AdrInt<-128) OR (AdrInt>127) THEN WrError(1370)
	   ELSE
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=Code; BAsmCode[1]:=Lo(AdrInt);
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO ALU8OrderCnt DO
    WITH ALU8Orders[z] DO
     IF SplitAcc(Name) THEN
      BEGIN
       IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
       ELSE
	BEGIN
	 IF MayImm THEN DecodeAdr(2,ArgCnt,16+8+4+2) ELSE DecodeAdr(2,ArgCnt,8+4+2);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   BAsmCode[PrefCnt]:=Code+(AdrPart SHL 4);
	   DecodeAdr(1,1,1);
	   IF AdrMode<>ModNone THEN
	    BEGIN
	     Inc(BAsmCode[PrefCnt],AdrPart SHL 6);
	     CodeLen:=PrefCnt+1+AdrCnt;
	     Move(AdrVals,BAsmCode[1+PrefCnt],AdrCnt);
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO ALU16OrderCnt DO
    WITH ALU16Orders[z] DO
     IF Memo(Name) THEN
      BEGIN
       OpSize:=1;
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE IF MomCPU-CPU6800<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 IF MayImm THEN DecodeAdr(1,ArgCnt,16+8+4+2) ELSE DecodeAdr(1,ArgCnt,8+4+2);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   CASE PageShift OF
	   1:IF PrefCnt=1 THEN BAsmCode[PrefCnt-1]:=Page4Prefix
			  ELSE AddPrefix(Page3Prefix);
	   2:IF PrefCnt=1 THEN BAsmCode[PrefCnt-1]:=Page4Prefix;
	   3:IF PrefCnt=0 THEN
	      IF AdrMode=ModInd THEN AddPrefix(Page3Prefix)
				ELSE AddPrefix(Page2Prefix);
{          3:IF PrefCnt=1 THEN BAsmCode[PrefCnt-1]:=Page3Prefix
			  ELSE AddPrefix(Page2Prefix);}
	   END;
	   BAsmCode[PrefCnt]:=Code+(AdrPart SHL 4);
	   CodeLen:=PrefCnt+1+AdrCnt;
	   Move(AdrVals,BAsmCode[1+PrefCnt],AdrCnt);
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO Bit63OrderCnt DO
    WITH Bit63Orders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
       ELSE IF MomCPU<>CPU6301 THEN WrError(1500)
       ELSE
	BEGIN
	 DecodeAdr(1,1,16);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   DecodeAdr(2,ArgCnt,2+8);
	   IF AdrMode<>ModNone THEN
	    BEGIN
	     BAsmCode[PrefCnt]:=Code;
	     IF AdrMode=ModDir THEN Inc(BAsmCode[PrefCnt],$10);
	     CodeLen:=PrefCnt+1+AdrCnt;
	     Move(AdrVals,BAsmCode[1+PrefCnt],AdrCnt);
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO Sing8OrderCnt DO
    WITH Sing8Orders[z] DO
     IF SplitAcc(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(1,ArgCnt,1+4+8);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   CodeLen:=PrefCnt+1+AdrCnt;
	   BAsmCode[PrefCnt]:=Code+(AdrPart SHL 4);
	   Move(AdrVals,BAsmCode[1+PrefCnt],AdrCnt);
	  END;
	END;
       Exit;
      END;

   IF Memo('JMP') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(1,ArgCnt,4+8);
       IF AdrMode<>ModImm THEN
	BEGIN
	 CodeLen:=PrefCnt+1+AdrCnt;
	 BAsmCode[PrefCnt]:=$4e+(AdrPart SHL 4);
	 Move(AdrVals,BAsmCode[1+PrefCnt],AdrCnt);
	END;
      END;
     Exit;
    END;

   IF Memo('JSR') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(1,ArgCnt,2+4+8);
       IF AdrMode<>ModImm THEN
	BEGIN
	 CodeLen:=PrefCnt+1+AdrCnt;
	 BAsmCode[PrefCnt]:=$8d+(AdrPart SHL 4);
	 Move(AdrVals,BAsmCode[1+PrefCnt],AdrCnt);
	END;
      END;
     Exit;
    END;

   IF (SplitAcc('PSH')) OR (SplitAcc('PUL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(1,1,1);
       IF AdrMode<>ModNone THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$32+AdrPart;
	 IF Memo('PSH') THEN Inc(BAsmCode[0],4);
	END;
      END;
     Exit;
    END;

   IF (Memo('BRSET')) OR (Memo('BRCLR')) THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       Try2Split(1); Try2Split(1);
      END
     ELSE IF ArgCnt=2 THEN
      BEGIN
       Try2Split(ArgCnt); Try2Split(2);
      END;
     IF (ArgCnt<3) OR (ArgCnt>4) THEN WrError(1110)
     ELSE IF MomCPU<CPU6811 THEN WrError(1500)
     ELSE
      BEGIN
       IF ArgStr[ArgCnt-1][1]='#' THEN Delete(ArgStr[ArgCnt-1],1,1);
       Mask:=EvalIntExpression(ArgStr[ArgCnt-1],Int8,OK);
       IF OK THEN
	BEGIN
	 DecodeAdr(1,ArgCnt-2,2+8);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   AdrInt:=EvalIntExpression(ArgStr[ArgCnt],Int16,OK);
	   IF OK THEN
	    BEGIN
	     Dec(AdrInt,EProgCounter+3+PrefCnt+AdrCnt);
	     IF (AdrInt<-128) OR (AdrInt>127) THEN WrError(1370)
	     ELSE
	      BEGIN
	       CodeLen:=PrefCnt+3+AdrCnt;
	       BAsmCode[PrefCnt]:=$12;
	       IF AdrMode=ModInd THEN Inc(BAsmCode[PrefCnt],12);
	       IF Memo('BRCLR') THEN Inc(BAsmCode[PrefCnt]);
	       Move(AdrVals,BAsmCode[PrefCnt+1],AdrCnt);
	       BAsmCode[PrefCnt+1+AdrCnt]:=Mask;
	       BAsmCode[PrefCnt+2+AdrCnt]:=Lo(AdrInt);
	      END;
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('BSET')) OR (Memo('BCLR')) THEN
    BEGIN
     IF MomCPU=CPU6301 THEN
      BEGIN
       ArgStr[ArgCnt+1]:=ArgStr[1];
       FOR z:=1 TO ArgCnt-1 DO ArgStr[z]:=ArgStr[z+1];
       ArgStr[ArgCnt]:=ArgStr[ArgCnt+1];
      END;
     IF (ArgCnt>=1) AND (ArgCnt<=2) THEN Try2Split(ArgCnt);
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE IF MomCPU<CPU6301 THEN WrError(1500)
     ELSE
      BEGIN
       IF ArgStr[ArgCnt][1]='#' THEN Delete(ArgStr[ArgCnt],1,1);
       Mask:=EvalIntExpression(ArgStr[ArgCnt],Int8,OK);
       IF (OK) AND (MomCPU=CPU6301) THEN
	IF (Mask>7) THEN
	 BEGIN
	  WrError(1320); OK:=False;
	 END
	ELSE
	 BEGIN
	  Mask:=1 SHL Mask;
	  IF Memo('BCLR') THEN Mask:=NOT Mask;
	 END;
       IF OK THEN
	BEGIN
	 DecodeAdr(1,ArgCnt-1,2+8);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   CodeLen:=PrefCnt+2+AdrCnt;
	   IF MomCPU=CPU6301 THEN
	    BEGIN
	     BAsmCode[PrefCnt]:=$61;
	     IF Memo('BSET') THEN Inc(BAsmCode[PrefCnt]);
	     IF AdrMode=ModDir THEN Inc(BAsmCode[PrefCnt],$10);
	     BAsmCode[1+PrefCnt]:=Mask;
	     Move(AdrVals,BAsmCode[2+PrefCnt],AdrCnt);
	    END
	   ELSE
	    BEGIN
	     BAsmCode[PrefCnt]:=$14;
	     IF Memo('BCLR') THEN Inc(BAsmCode[PrefCnt]);
	     IF AdrMode=ModInd THEN Inc(BAsmCode[PrefCnt],8);
	     Move(AdrVals,BAsmCode[1+PrefCnt],AdrCnt);
	     BAsmCode[1+PrefCnt+AdrCnt]:=Mask;
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('BTST')) OR (Memo('BTGL')) THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE IF MomCPU<>CPU6301 THEN WrError(1500)
     ELSE
      BEGIN
       AdrByte:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
	IF AdrByte>7 THEN WrError(1320)
	ELSE
	 BEGIN
	  DecodeAdr(2,ArgCnt,2+8);
	  IF AdrMode<>ModNone THEN
	   BEGIN
	    CodeLen:=PrefCnt+2+AdrCnt;
	    BAsmCode[1+PrefCnt]:=1 SHL AdrByte;
	    Move(AdrVals,BAsmCode[2+PrefCnt],AdrCnt);
	    BAsmCode[PrefCnt]:=$65;
	    IF Memo('BTST') THEN Inc(BAsmCode[PrefCnt],6);
	    IF AdrMode=ModDir THEN Inc(BAsmCode[PrefCnt],$10);
	   END;
	 END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_68:Boolean;
	Far;
BEGIN
   IF ActPC=SegCode THEN ChkPC_68:=(ProgCounter>=0) AND (ProgCounter<$10000)
   ELSE ChkPC_68:=False;
END;

	FUNCTION IsDef_68:Boolean;
	Far;
BEGIN
   IsDef_68:=False;
END;

	PROCEDURE SwitchFrom_68;
	Far;
BEGIN
END;

	PROCEDURE SwitchTo_68;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$61; NOPCode:=$01;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_68; ChkPC:=ChkPC_68; IsDef:=IsDef_68;
   SwitchFrom:=SwitchFrom_68;
END;

BEGIN
   CPU6800:=AddCPU('6800',SwitchTo_68);
   CPU6301:=AddCPU('6301',SwitchTo_68);
   CPU6811:=AddCPU('6811',SwitchTo_68);
END.
