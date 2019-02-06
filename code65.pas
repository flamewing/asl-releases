{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

	Unit Code65;

Interface

        Uses NLS,
             AsmDef,AsmPars,AsmSub,CodePseu;

Implementation

CONST
   ModZA=0;      { aa }
   ModA=1;       { aabb }
   ModZIX=2;     { aa,X }
   ModIX=3;      { aabb,X }
   ModZIY=4;     { aa,Y }
   ModIY=5;      { aabb,Y }
   ModIndX=6;    { (aa,X) }
   ModIndY=7;    { (aa),Y }
   ModInd16=8;   { (aabb) }
   ModImm=9;     { #aa }
   ModAcc=10;    { A }
   ModNone=11;   { }
   ModInd8=12;   { (aa) }
   ModSpec=13;   { \aabb }

TYPE
   FixedOrder=RECORD
	       Name:String[3];
	       CPUFlag:Byte;
	       Code:Byte;
	      END;
   NormOrder=RECORD
	      Name:String[3];
	      Codes:ARRAY[ModZA..ModSpec] OF Integer;
	     END;
   CondOrder=RECORD
	      Name:String[3];
	      CPUFlag:Byte;
	      Code:Byte;
	     END;

CONST
   FixedOrderCount=35;
   FixedOrders:ARRAY[1..FixedOrderCount] OF FixedOrder=
               ( (Name:'RTS'; CPUFlag:15; Code:$60),
                 (Name:'RTI'; CPUFlag:15; Code:$40),
                 (Name:'TAX'; CPUFlag:15; Code:$aa),
                 (Name:'TXA'; CPUFlag:15; Code:$8a),
                 (Name:'TAY'; CPUFlag:15; Code:$a8),
                 (Name:'TYA'; CPUFlag:15; Code:$98),
                 (Name:'TXS'; CPUFlag:15; Code:$9a),
                 (Name:'TSX'; CPUFlag:15; Code:$ba),
                 (Name:'DEX'; CPUFlag:15; Code:$ca),
                 (Name:'DEY'; CPUFlag:15; Code:$88),
                 (Name:'INX'; CPUFlag:15; Code:$e8),
                 (Name:'INY'; CPUFlag:15; Code:$c8),
                 (Name:'PHA'; CPUFlag:15; Code:$48),
                 (Name:'PLA'; CPUFlag:15; Code:$68),
                 (Name:'PHP'; CPUFlag:15; Code:$08),
                 (Name:'PLP'; CPUFlag:15; Code:$28),
                 (Name:'PHX'; CPUFlag: 6; Code:$da),
                 (Name:'PLX'; CPUFlag: 6; Code:$fa),
                 (Name:'PHY'; CPUFlag: 6; Code:$5a),
                 (Name:'PLY'; CPUFlag: 6; Code:$7a),
                 (Name:'BRK'; CPUFlag:15; Code:$00),
                 (Name:'STP'; CPUFlag: 8; Code:$42),
                 (Name:'SLW'; CPUFlag: 8; Code:$c2),
                 (Name:'FST'; CPUFlag: 8; Code:$e2),
                 (Name:'WIT'; CPUFlag: 8; Code:$c2),
                 (Name:'NOP'; CPUFlag:15; Code:$ea),
                 (Name:'CLI'; CPUFlag:15; Code:$58),
                 (Name:'SEI'; CPUFlag:15; Code:$78),
                 (Name:'CLC'; CPUFlag:15; Code:$18),
                 (Name:'SEC'; CPUFlag:15; Code:$38),
                 (Name:'CLD'; CPUFlag:15; Code:$d8),
                 (Name:'SED'; CPUFlag:15; Code:$f8),
                 (Name:'CLV'; CPUFlag:15; Code:$b8),
                 (Name:'CLT'; CPUFlag: 8; Code:$12),
                 (Name:'SET'; CPUFlag: 8; Code:$32));

   NormOrderCount=31;
   NormOrders:ARRAY[1..NormOrderCount] OF NormOrder=

		       {  ZA    A    ZIX    IX   ZIY   IY    @X    @Y   (n16)  imm   ACC   NON   (n8)  spec }

   ((Name:'LDA'; Codes:($0fa5,$0fad,$0fb5,$0fbd,   -1,$0fb9,$0fa1,$0fb1,   -1,$0fa9,   -1,   -1,$06b2,   -1)),
    (Name:'LDX'; Codes:($0fa6,$0fae,   -1,   -1,$0fb6,$0fbe,   -1,   -1,   -1,$0fa2,   -1,   -1,   -1,   -1)),
    (Name:'LDY'; Codes:($0fa4,$0fac,$0fb4,$0fbc,   -1,   -1,   -1,   -1,   -1,$0fa0,   -1,   -1,   -1,   -1)),
    (Name:'STA'; Codes:($0f85,$0f8d,$0f95,$0f9d,   -1,$0f99,$0f81,$0f91,   -1,   -1,   -1,   -1,$0692,   -1)),
    (Name:'STX'; Codes:($0f86,$0f8e,   -1,   -1,$0f96,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'STY'; Codes:($0f84,$0f8c,$0f94,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'STZ'; Codes:($0664,$069c,$0674,$069e,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'ADC'; Codes:($0f65,$0f6d,$0f75,$0f7d,   -1,$0f79,$0f61,$0f71,   -1,$0f69,   -1,   -1,$0672,   -1)),
    (Name:'SBC'; Codes:($0fe5,$0fed,$0ff5,$0ffd,   -1,$0ff9,$0fe1,$0ff1,   -1,$0fe9,   -1,   -1,$06f2,   -1)),
    (Name:'MUL'; Codes:(   -1,   -1,$0862,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'DIV'; Codes:(   -1,   -1,$08e2,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'AND'; Codes:($0f25,$0f2d,$0f35,$0f3d,   -1,$0f39,$0f21,$0f31,   -1,$0f29,   -1,   -1,$0632,   -1)),
    (Name:'ORA'; Codes:($0f05,$0f0d,$0f15,$0f1d,   -1,$0f19,$0f01,$0f11,   -1,$0f09,   -1,   -1,$0612,   -1)),
    (Name:'EOR'; Codes:($0f45,$0f4d,$0f55,$0f5d,   -1,$0f59,$0f41,$0f51,   -1,$0f49,   -1,   -1,$0652,   -1)),
    (Name:'COM'; Codes:($0844,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'BIT'; Codes:($0f24,$0f2c,$0634,$063c,   -1,   -1,   -1,   -1,   -1,$0689,   -1,   -1,   -1,   -1)),
    (Name:'TST'; Codes:($0864,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'ASL'; Codes:($0f06,$0f0e,$0f16,$0f1e,   -1,   -1,   -1,   -1,   -1,   -1,$0f0a,$0f0a,   -1,   -1)),
    (Name:'LSR'; Codes:($0f46,$0f4e,$0f56,$0f5e,   -1,   -1,   -1,   -1,   -1,   -1,$0f4a,$0f4a,   -1,   -1)),
    (Name:'ROL'; Codes:($0f26,$0f2e,$0f36,$0f3e,   -1,   -1,   -1,   -1,   -1,   -1,$0f2a,$0f2a,   -1,   -1)),
    (Name:'ROR'; Codes:($0f66,$0f6e,$0f76,$0f7e,   -1,   -1,   -1,   -1,   -1,   -1,$0f6a,$0f6a,   -1,   -1)),
    (Name:'RRF'; Codes:($0882,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'TSB'; Codes:($0604,$060c,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'TRB'; Codes:($0614,$061c,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1)),
    (Name:'INC'; Codes:($0fe6,$0fee,$0ff6,$0ffe,   -1,   -1,   -1,   -1,   -1,   -1,$0e1a,$0e1a,   -1,   -1)),
    (Name:'DEC'; Codes:($0fc6,$0fce,$0fd6,$0fde,   -1,   -1,   -1,   -1,   -1,   -1,$0e3a,$0e3a,   -1,   -1)),
    (Name:'CMP'; Codes:($0fc5,$0fcd,$0fd5,$0fdd,   -1,$0fd9,$0fc1,$0fd1,   -1,$0fc9,   -1,   -1,$06d2,   -1)),
    (Name:'CPX'; Codes:($0fe4,$0fec,   -1,   -1,   -1,   -1,   -1,   -1,   -1,$0fe0,   -1,   -1,   -1,   -1)),
    (Name:'CPY'; Codes:($0fc4,$0fcc,   -1,   -1,   -1,   -1,   -1,   -1,   -1,$0fc0,   -1,   -1,   -1,   -1)),
    (Name:'JMP'; Codes:(   -1,$0f4c,   -1,   -1,   -1,   -1,$067c,   -1,$0f6c,   -1,   -1,   -1,$08b2,   -1)),
    (Name:'JSR'; Codes:(   -1,$0f20,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,$0802,$0822)));

    CondOrderCount=9;
    CondOrders:ARRAY[1..CondOrderCount] OF CondOrder=
               ((Name:'BEQ'; CPUFlag:15; Code:$f0),
                (Name:'BNE'; CPUFlag:15; Code:$d0),
                (Name:'BPL'; CPUFlag:15; Code:$10),
                (Name:'BMI'; CPUFlag:15; Code:$30),
                (Name:'BCC'; CPUFlag:15; Code:$90),
                (Name:'BCS'; CPUFlag:15; Code:$b0),
                (Name:'BVC'; CPUFlag:15; Code:$50),
                (Name:'BVS'; CPUFlag:15; Code:$70),
                (Name:'BRA'; CPUFlag:14; Code:$80));

VAR
   CLI_SEI_Flag,ADC_SBC_Flag:Boolean;

   SaveInitProc:PROCEDURE;
   CPU6502,CPU65SC02,CPU65C02,CPUM740:CPUVar;
   SpecPage:LongInt;


	FUNCTION ChkZero(Asc:String; VAR erg:Byte):String;
BEGIN
   IF (Length(Asc)>1) AND ((Asc[1]='<') OR (Asc[1]='>')) THEN
    BEGIN
     erg:=Ord(Asc[1]='<')+1; ChkZero:=Copy(Asc,2,Length(Asc)-1);
    END
   ELSE
    BEGIN
     erg:=0; ChkZero:=Asc;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME740Count=1;
   ASSUME740s:ARRAY[1..ASSUME740Count] OF ASSUMERec=
	     ((Name:'SP'; Dest:@SpecPage; Min:0; Max:$ff; NothingVal:-1));
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     IF MomCPU<>CPUM740 THEN WrError(1500)
     ELSE CodeASSUME(@ASSUME740s,ASSUME740Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_65;
	Far;
LABEL
   Ende;
VAR
   OrderZ:Word;
   s1,s2:String;
   ErgMode:ShortInt;
   AdrVals:ARRAY[0..1] OF Byte;
   AdrWord:Word ABSOLUTE AdrVals;
   AdrCnt:Byte;
   ValOK,b:Boolean;
   ZeroMode:Byte;

	FUNCTION CPUAllowed(Flag:Byte):Boolean;
BEGIN
   CPUAllowed:=Odd(Flag SHR (Ord(MomCPU)-Ord(CPU6502)));
END;

	FUNCTION IsAllowed(Val:Word):Boolean;
BEGIN
   IsAllowed:=CPUAllowed(Hi(Val)) AND (Val<>-1);
END;

	PROCEDURE ChkZeroMode(Mode:ShortInt);
VAR
   OrderZ:Integer;
BEGIN
   FOR OrderZ:=1 TO NormOrderCount DO
   WITH NormOrders[OrderZ] DO
    IF Memo(Name) THEN
     BEGIN
      IF IsAllowed(Codes[Mode]) THEN
       BEGIN
	ErgMode:=Mode; Dec(AdrCnt);
       END;
      Exit;
     END;
END;

	PROCEDURE InsNOP;
BEGIN
   Move(BAsmCode[0],BAsmCode[1],CodeLen);
   Inc(CodeLen); BAsmCode[0]:=NOPCode;
END;

BEGIN
   CodeLen:=0; DontPrint:=False; AdrWord:=0;

   { zu ignorierendes }

   if Memo('') THEN Goto Ende;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Goto Ende;

   IF DecodeMotoPseudo(False) THEN Goto Ende;

   { Anweisungen ohne Argument }

   FOR OrderZ:=1 TO FixedOrderCount DO
   WITH FixedOrders[OrderZ] DO
   IF Memo(Name) THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF NOT CPUAllowed(CPUFlag) THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=1;
       BAsmCode[0]:=Code;
       IF Memo('BRK') THEN
	BEGIN
	 Inc(CodeLen); BAsmCode[CodeLen-1]:=NOPCode;
        END
       ELSE IF MomCPU=CPUM740 THEN
	BEGIN
         IF Memo('PLP') THEN
	  BEGIN
	   Inc(CodeLen); BAsmCode[CodeLen-1]:=NOPCode;
	  END;
	 IF (ADC_SBC_Flag) AND (Memo('SEC') OR Memo('CLC') OR Memo('CLD')) THEN InsNOP;
	END;
      END;
     Goto Ende;
    END;

    IF (Memo('SEB')) OR (Memo('CLB')) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF MomCPU<>CPUM740 THEN WrError(1500)
      ELSE
       BEGIN
	AdrWord:=EvalIntExpression(ArgStr[1],UInt3,ValOK);
	IF ValOK THEN
	 BEGIN
	  BAsmCode[0]:=$0b+(AdrWord SHL 5)+(Ord(Memo('CLB')) SHL 4);
          IF NLS_StrCaseCmp(ArgStr[2],'A')=0 THEN CodeLen:=1
	  ELSE
	   BEGIN
            BAsmCode[1]:=EvalIntExpression(ArgStr[2],UInt8,ValOK);
	    IF ValOK THEN
	     BEGIN
	      CodeLen:=2; Inc(BAsmCode[0],4);
	     END;
	   END;
	 END;
       END;
      Goto Ende;
     END;

    IF (Memo('BBC')) OR (Memo('BBS')) THEN
     BEGIN
      IF ArgCnt<>3 THEN WrError(1110)
      ELSE IF MomCPU<>CPUM740 THEN WrError(1500)
      ELSE
       BEGIN
	BAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt3,ValOK);
	IF ValOK THEN
	 BEGIN
	  BAsmCode[0]:=(BAsmCode[0] SHL 5)+(Ord(Memo('BBC')) SHL 4)+3;
          b:=NLS_StrCaseCmp(ArgStr[2],'A')<>0;
	  IF NOT b THEN ValOK:=True
	  ELSE
	   BEGIN
	    Inc(BAsmCode[0],4);
            BAsmCode[1]:=EvalIntExpression(ArgStr[2],UInt8,ValOK);
	   END;
	  IF ValOK THEN
	   BEGIN
	    AdrWord:=EvalIntExpression(ArgStr[3],Int16,ValOK)-(EProgCounter+2+Ord(b));
	    IF ValOK THEN
	     IF (AdrWord>$7f) AND (AdrWord<$ff80) THEN WrError(1370)
	     ELSE
	      BEGIN
	       CodeLen:=2+Ord(b);
	       BAsmCode[CodeLen-1]:=Lo(AdrWord);
	       IF CLI_SEI_Flag THEN InsNOP;
	      END;
	   END;
	 END;
       END;
      Goto Ende;
     END;

    IF ((Length(OpPart)=4)
    AND (OpPart[4]>='0') AND (OpPart[4]<='7')
    AND ((Copy(OpPart,1,3)='BBR') OR (Copy(OpPart,1,3)='BBS'))) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF MomCPU<>CPU65C02 THEN WrError(1500)
      ELSE
       BEGIN
	BAsmCode[1]:=EvalIntExpression(ArgStr[1],UInt8,ValOK);
	IF ValOK THEN
	 BEGIN
	  BAsmCode[0]:=((Ord(OpPart[4])-AscOfs) SHL 4)+(Ord(OpPart[3]='S') SHL 7)+15;
          AdrWord:=EvalIntExpression(ArgStr[2],UInt16,ValOK)-(EProgCounter+3);
	  IF ValOK THEN
	   IF (AdrWord>$7f) AND (AdrWord<$ff80) THEN WrError(1370)
	   ELSE
	    BEGIN
	     CodeLen:=3;
	     BAsmCode[2]:=Lo(AdrWord);
	    END;
	 END;
       END;
      Goto Ende;
     END;

    IF ((Length(OpPart)=4)
    AND (OpPart[4]>='0') AND (OpPart[4]<='7')
    AND ((Copy(OpPart,1,3)='RMB') OR (Copy(OpPart,1,3)='SMB'))) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE IF MomCPU<>CPU65C02 THEN WrError(1500)
      ELSE
       BEGIN
	BAsmCode[1]:=EvalIntExpression(ArgStr[1],UInt8,ValOK);
	IF ValOK THEN
	 BEGIN
	  BAsmCode[0]:=((Ord(OpPart[4])-AscOfs) SHL 4)+(Ord(OpPart[1]='S') SHL 7)+7;
          CodeLen:=2;
	 END;
       END;
      Goto Ende;
     END;

   IF Memo('LDM') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<>CPUM740 THEN WrError(1500)
     ELSE
      BEGIN
       BAsmCode[0]:=$3c;
       BAsmCode[2]:=EvalIntExpression(ArgStr[2],UInt8,ValOK);
       IF ValOK THEN
	IF ArgStr[1][1]<>'#' THEN WrError(1350)
	ELSE
	 BEGIN
          BAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),Int8,ValOK);
	  IF ValOK THEN CodeLen:=3;
	 END;
      END;
     Exit;
    END;

   { normale Anweisungen: Adreáausdruck parsen }

   ErgMode:=-1;

   IF ArgCnt=0 THEN
    BEGIN
     AdrCnt:=0; ErgMode:=ModNone;
    END

   ELSE IF ArgCnt=1 THEN
    BEGIN
     { 1. Akkuadressierung }

     IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN
      BEGIN
       AdrCnt:=0; ErgMode:=ModAcc;
      END

     { 2. immediate ? }

     ELSE IF ArgStr[1][1]='#' THEN
      BEGIN
       s1:=Copy(ArgStr[1],2,Length(ArgStr[1])-1);
       AdrVals[0]:=EvalIntExpression(s1,Int8,ValOK);
       IF ValOK THEN
	BEGIN
	 ErgMode:=ModImm; AdrCnt:=1;
	END;
      END

     { 3. Special Page ? }

     ELSE IF ArgStr[1][1]='\' THEN
      BEGIN
       s1:=Copy(ArgStr[1],2,Length(ArgStr[1])-1);
       AdrWord:=EvalIntExpression(s1,UInt16,ValOK);
       IF ValOK THEN
	IF Hi(AdrWord)<>SpecPage THEN WrError(1315)
	ELSE
	 BEGIN
	  ErgMode:=ModSpec; AdrVals[0]:=Lo(AdrWord); AdrCnt:=1;
	 END;
      END

     { 4. X-indirekt ? }

     ELSE IF NLS_StrCaseCmp(Copy(ArgStr[1],Length(ArgStr[1])-2,3),',X)')=0 THEN
      BEGIN
       IF ArgStr[1][1]<>'(' THEN WrError(1350)
       ELSE
	BEGIN
	 s1:=ChkZero(Copy(ArgStr[1],2,Length(ArgStr[1])-4),ZeroMode);
         IF Memo('JMP') THEN
          BEGIN
           AdrWord:=EvalIntExpression(s1,UInt16,ValOK);
           IF ValOK THEN
            BEGIN
             AdrVals[0]:=Lo(AdrWord); AdrVals[1]:=Hi(AdrWord);
             ErgMode:=ModIndX; AdrCnt:=2;
            END;
          END
         ELSE
          BEGIN
           AdrVals[0]:=EvalIntExpression(s1,UInt8,ValOK);
           IF ValOK THEN
            BEGIN
             ErgMode:=ModIndX; AdrCnt:=1;
            END;
          END;
	END;
      END

     ELSE
      BEGIN
       { 5. indirekt absolut ? }

       IF IsIndirect(ArgStr[1]) THEN
	BEGIN
	 s1:=ChkZero(Copy(ArgStr[1],2,Length(ArgStr[1])-2),ZeroMode);
	 IF ZeroMode=2 THEN
	  BEGIN
           AdrWord:=EvalIntExpression(s1,UInt8,ValOK);
	   IF ValOK THEN
	    BEGIN
	     ErgMode:=ModInd8; AdrCnt:=1;
	    END;
	  END
	 ELSE
	  BEGIN
	   AdrWord:=EvalIntExpression(s1,Int16,ValOK);
	   IF ValOK THEN
	    BEGIN
	     ErgMode:=ModInd16; AdrCnt:=2;
	     IF (ZeroMode=0) AND (Hi(AdrWord)=0) THEN
	      ChkZeroMode(ModInd8);
	    END
	  END
	END

       { 6. absolut }

       ELSE
	BEGIN
	 s1:=ChkZero(ArgStr[1],ZeroMode);
	 IF ZeroMode=2 THEN
	  BEGIN
           AdrWord:=EvalIntExpression(s1,UInt8,ValOK);
	   IF ValOK THEN
	    BEGIN
	     ErgMode:=ModZA; AdrCnt:=1;
	    END;
	  END
	 ELSE
	  BEGIN
	   AdrWord:=EvalIntExpression(s1,Int16,ValOK);
	   IF ValOK THEN
	    BEGIN
	     ErgMode:=ModA; AdrCnt:=2;
	     IF (ZeroMode=0) AND (Hi(AdrWord)=0) THEN
	      ChkZeroMode(ModZA);
	    END
	  END
	END
      END
    END

   ELSE IF ArgCnt=2 THEN
    BEGIN
     { 7. Y-indirekt ? }

     IF (IsIndirect(ArgStr[1])) AND (NLS_StrCaseCmp(ArgStr[2],'Y')=0) THEN
      BEGIN
       s1:=ChkZero(Copy(ArgStr[1],2,Length(ArgStr[1])-2),ZeroMode);
       AdrVals[0]:=EvalIntExpression(s1,UInt8,ValOK);
       IF ValOK THEN
	BEGIN
	 ErgMode:=ModIndY; AdrCnt:=1;
	END;
      END

     { 8. X,Y-indiziert ? }

     ELSE
      BEGIN
       s1:=ChkZero(ArgStr[1],ZeroMode);
       IF ZeroMode=2 THEN
	BEGIN
         AdrWord:=EvalIntExpression(s1,UInt8,ValOK);
	 IF ValOK THEN
	  BEGIN
	   AdrCnt:=1;
           IF NLS_StrCaseCmp(ArgStr[2],'X')=0 THEN ErgMode:=ModZIX ELSE ErgMode:=ModZIY;
	  END
	END
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(s1,Int16,ValOK);
	 IF ValOK THEN
	  BEGIN
	   AdrCnt:=2;
           IF NLS_StrCaseCmp(ArgStr[2],'X')=0 THEN ErgMode:=ModIX ELSE ErgMode:=ModIY;
	   IF (Hi(AdrWord)=0) AND (ZeroMode=0) THEN
            IF NLS_StrCaseCmp(ArgStr[2],'X')=0 THEN ChkZeroMode(ModZIX)
	    ELSE ChkZeroMode(ModZIY);
	  END;
	END;
      END;
    END
   ELSE
    BEGIN
     WrError(1110);
     Goto Ende;
    END;

   { in Tabelle nach Opcode suchen }

   FOR OrderZ:=1 TO NormOrderCount DO
   WITH NormOrders[OrderZ] DO
    IF Memo(Name) THEN
     BEGIN
      IF (ErgMode=-1) THEN WrError(1350)
      ELSE
       BEGIN
	IF (Codes[ErgMode]=-1) THEN
	 BEGIN
	  IF ErgMode=ModZA THEN ErgMode:=ModA;
	  IF ErgMode=ModZIX THEN ErgMode:=ModIX;
	  IF ErgMode=ModZIY THEN ErgMode:=ModIY;
	  IF ErgMode=ModInd8 THEN ErgMode:=ModInd16;
          AdrVals[AdrCnt]:=0; Inc(AdrCnt);
	 END;
	IF (Codes[ErgMode]=-1) THEN WrError(1350)
	ELSE IF NOT CPUAllowed(Hi(Codes[ErgMode])) THEN WrError(1500)
	ELSE
	 BEGIN
	  BAsmCode[0]:=Lo(Codes[ErgMode]); Move(AdrVals,BAsmCode[1],AdrCnt);
	  CodeLen:=Succ(AdrCnt);
	  IF (ErgMode=ModInd16) AND (MomCPU<>CPU65C02) AND (BAsmCode[1]=$ff) THEN
	   BEGIN
	    WrError(1900); CodeLen:=0;
	   END;
	 END;
       END;
      Goto Ende;
     END;

   { relativer Sprung ? }

   IF ErgMode=ModZA THEN
    BEGIN
     ErgMode:=ModA; AdrWord:=AdrVals[0];
    END;
   IF ErgMode=ModA THEN
   FOR OrderZ:=1 TO CondOrderCount DO
   WITH CondOrders[OrderZ] DO
    IF Memo(Name) THEN
     BEGIN
      Dec(AdrWord,EProgCounter+2);
      IF NOT CPUAllowed(CPUFlag) THEN WrError(1500)
      ELSE IF (Integer(AdrWord)>127) OR (Integer(AdrWord)<-128) THEN WrError(1370)
      ELSE
       BEGIN
        BAsmCode[0]:=Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
       END;
      Goto Ende;
     END;

   WrXError(1200,OpPart);

Ende:
   { Spezialflags ? }

   CLI_SEI_Flag:=Memo('CLI') OR Memo('SEI');
   ADC_SBC_Flag:=Memo('ADC') OR Memo('SBC');
END;

	PROCEDURE InitCode_65;
	Far;
BEGIN
   SaveInitProc;
   CLI_SEI_Flag:=False;
   ADC_SBC_Flag:=False;
END;

	FUNCTION ChkPC_65:Boolean;
	Far;
BEGIN
   IF ActPC=SegCode THEN ChkPC_65:=(ProgCounter>=0) AND (ProgCounter<$10000)
   ELSE ChkPC_65:=False;
END;

	FUNCTION IsDef_65:Boolean;
	Far;
BEGIN
   IsDef_65:=False;
END;

        PROCEDURE SwitchFrom_65;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_65;
	Far;
VAR
   z:Integer;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=MomCPU=CPUM740;

   PCSymbol:='*';  HeaderID:=$11; NOPCode:=$ea;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_65; ChkPC:=ChkPC_65; IsDef:=IsDef_65;
   SwitchFrom:=SwitchFrom_65;

   { Codes umpatchen }

   FOR z:=1 TO NormOrderCount DO
    WITH NormOrders[z] DO
     IF Name='INC' THEN
      BEGIN
       IF MomCPU=CPUM740 THEN Codes[ModAcc]:=$0e3a ELSE Codes[ModAcc]:=$0e1a;
       Codes[ModNone]:=Codes[ModAcc];
      END
     ELSE IF Name='DEC' THEN
      BEGIN
       IF MomCPU=CPUM740 THEN Codes[ModAcc]:=$0e1a ELSE Codes[ModAcc]:=$0e3a;
       Codes[ModNone]:=Codes[ModAcc];
      END;
END;

BEGIN
   CPU6502  :=AddCPU('6502'    ,SwitchTo_65);
   CPU65SC02:=AddCPU('65SC02'  ,SwitchTo_65);
   CPU65C02 :=AddCPU('65C02'   ,SwitchTo_65);
   CPUM740  :=AddCPU('MELPS740',SwitchTo_65);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_65;
END.
