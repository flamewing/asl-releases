{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code7700;

INTERFACE

        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;

IMPLEMENTATION

TYPE
   FixedOrder=RECORD
               Name:String[4];
	       Code:Word;
	       Allowed:Byte;
	      END;

   RelOrder=RECORD
             Name:String[4];
	     Code:Word;
	     Disp8,Disp16:ShortInt;
	    END;

   AccOrder=RECORD
	     Name:String[3];
	     Code:Byte;
	    END;

   RMWOrder=RECORD
	     Name:String[3];
	     ACode,MCode:Byte;
	    END;

   XYOrder=RECORD
	    Name:String[3];
	    CodeImm,CodeAbs8,CodeAbs16,CodeIdxX8,CodeIdxX16,
	    CodeIdxY8,CodeIdxY16:Byte;
	   END;

   MulDivOrder=RECORD
                Name:String[4];
                Code:Word;
                Allowed:Byte;
               END;

CONST
   ModNone=-1;
   ModImm    = 0;       MModImm    = 1 SHL ModImm;
   ModAbs8   = 1;       MModAbs8   = 1 SHL ModAbs8;
   ModAbs16  = 2;       MModAbs16  = 1 SHL ModAbs16;
   ModAbs24  = 3;       MModAbs24  = 1 SHL ModAbs24;
   ModIdxX8  = 4;       MModIdxX8  = 1 SHL ModIdxX8;
   ModIdxX16 = 5;       MModIdxX16 = 1 SHL ModIdxX16;
   ModIdxX24 = 6;       MModIdxX24 = 1 SHL ModIdxX24;
   ModIdxY8  = 7;       MModIdxY8  = 1 SHL ModIdxY8;
   ModIdxY16 = 8;       MModIdxY16 = 1 SHL ModIdxY16;
   ModIdxY24 = 9;       MModIdxY24 = 1 SHL ModIdxY24;
   ModInd8   =10;       MModInd8   = 1 SHL ModInd8;
   ModInd16  =11;       MModInd16  = 1 SHL ModInd16;
   ModInd24  =12;       MModInd24  = 1 SHL ModInd24;
   ModIndX8  =13;       MModIndX8  = 1 SHL ModIndX8;
   ModIndX16 =14;       MModIndX16 = 1 SHL ModIndX16;
   ModIndX24 =15;       MModIndX24 = 1 SHL ModIndX24;
   ModIndY8  =16;       MModIndY8  = 1 SHL ModIndY8;
   ModIndY16 =17;       MModIndY16 = 1 SHL ModIndY16;
   ModIndY24 =18;       MModIndY24 = 1 SHL ModIndY24;
   ModIdxS8  =19;       MModIdxS8  = 1 SHL ModIdxS8;
   ModIndS8  =20;       MModIndS8  = 1 SHL ModIndS8;

   FixedOrderCnt=64;

   RelOrderCnt=13;

   AccOrderCnt=9;

   RMWOrderCnt=6;

   Imm8OrderCnt=5;

   XYOrderCnt=6;

   MulDivOrderCnt=4;

   PushRegCnt=8;
   PushRegs:ARRAY[0..PushRegCnt-1] OF String[3]=('A','B','X','Y','DPR','DT','PG','PS');

   PrefAccB=$42;

TYPE
   FixedOrderArray   =ARRAY[1..FixedOrderCnt   ] OF FixedOrder;
   RelOrderArray     =ARRAY[1..RelOrderCnt     ] OF RelOrder;
   AccOrderArray     =ARRAY[1..AccOrderCnt     ] OF AccOrder;
   RMWOrderArray     =ARRAY[1..RMWOrderCnt     ] OF RMWOrder;
   Imm8OrderArray    =ARRAY[1..Imm8OrderCnt    ] OF FixedOrder;
   XYOrderArray      =ARRAY[1..XYOrderCnt      ] OF XYOrder;
   MulDivOrderArray  =ARRAY[1..MulDivOrderCnt  ] OF MulDivOrder;

VAR
   Reg_PG,Reg_DT,Reg_X,Reg_M,Reg_DPR,BankReg:LongInt;

   WordSize:Boolean;
   AdrVals:ARRAY[0..2] OF Byte;
   AdrCnt:Byte;
   AdrType:ShortInt;
   LFlag:Boolean;

   FixedOrders:^FixedOrderArray;
   RelOrders:^RelOrderArray;
   AccOrders:^AccOrderArray;
   RMWOrders:^RMWOrderArray;
   Imm8Orders:^Imm8OrderArray;
   XYOrders:^XYOrderArray;
   MulDivOrders:^MulDivOrderArray;

   SaveInitProc:PROCEDURE;

   CPU65816,CPUM7700,CPUM7750,CPUM7751:CPUVar;

{-----------------------------------------------------------------------------}

        PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName:String; NCode:Word; NAllowed:Byte);
BEGIN
   Inc(z); IF z>FixedOrderCnt THEN Halt(0);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Allowed:=NAllowed;
    END;
END;

        PROCEDURE AddRel(NName:String; NCode:Word; NDisp8,NDisp16:ShortInt);
BEGIN
   Inc(z); IF z>RelOrderCnt THEN Halt(0);
   WITH RelOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Disp8:=NDisp8; Disp16:=NDisp16;
    END;
END;

        PROCEDURE AddAcc(NName:String; NCode:Byte);
BEGIN
   Inc(z); IF z>AccOrderCnt THEN Halt(0);
   WITH AccOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddRMW(NName:String; NACode,NMCode:Byte);
BEGIN
   Inc(z); IF z>RMWOrderCnt THEN Halt(0);
   WITH RMWOrders^[z] DO
    BEGIN
     Name:=NName; MCode:=NMCode; ACode:=NACode;
    END;
END;

        PROCEDURE AddImm8(NName:String; NCode:Word; NAllowed:Byte);
BEGIN
   Inc(z); IF z>Imm8OrderCnt THEN Halt(0);
   WITH Imm8Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Allowed:=NAllowed;
    END;
END;

        PROCEDURE AddXY(NName:String; NCodeImm,NCodeAbs8,NCodeAbs16,NCodeIdxX8,
                        NCodeIdxX16,NCodeIdxY8,NCodeIdxY16:Byte);
BEGIN
   Inc(z); IF z>XYOrderCnt THEN Halt(0);
   WITH XYOrders^[z] DO
    BEGIN
     Name:=NName;
     CodeImm:=NCodeImm;
     CodeAbs8:=NCodeAbs8;
     CodeAbs16:=NCodeAbs16;
     CodeIdxX8:=NCodeIdxX8;
     CodeIdxX16:=NCodeIdxX16;
     CodeIdxY8:=NCodeIdxY8;
     CodeIdxY16:=NCodeIdxY16;
    END;
END;

        PROCEDURE AddMulDiv(NName:String; NCode:Word; NAllowed:Byte);
BEGIN
   Inc(z); IF z>MulDivOrderCnt THEN Halt(0);
   WITH MulDivOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Allowed:=NAllowed;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('CLC',$0018,15); AddFixed('CLI',$0058,15);
   AddFixed('CLM',$00d8,14); AddFixed('CLV',$00b8,15);
   AddFixed('DEX',$00ca,15); AddFixed('DEY',$0088,15);
   AddFixed('INX',$00e8,15); AddFixed('INY',$00c8,15);
   AddFixed('NOP',$00ea,15); AddFixed('PHA',$0048,15);
   AddFixed('PHD',$000b,15); AddFixed('PHG',$004b,14);
   AddFixed('PHP',$0008,15); AddFixed('PHT',$008b,14);
   AddFixed('PHX',$00da,15); AddFixed('PHY',$005a,15);
   AddFixed('PLA',$0068,15); AddFixed('PLD',$002b,15);
   AddFixed('PLP',$0028,15); AddFixed('PLT',$00ab,14);
   AddFixed('PLX',$00fa,15); AddFixed('PLY',$007a,15);
   AddFixed('RTI',$0040,15); AddFixed('RTL',$006b,15);
   AddFixed('RTS',$0060,15); AddFixed('SEC',$0038,15);
   AddFixed('SEI',$0078,15); AddFixed('SEM',$00f8,14);
   AddFixed('STP',$00db,15); AddFixed('TAD',$005b,15);
   AddFixed('TAS',$001b,15); AddFixed('TAX',$00aa,15);
   AddFixed('TAY',$00a8,15); AddFixed('TBD',$425b,14);
   AddFixed('TBS',$421b,14); AddFixed('TBX',$42aa,14);
   AddFixed('TBY',$42a8,14); AddFixed('TDA',$007b,15);
   AddFixed('TDB',$427b,14); AddFixed('TSA',$003b,15);
   AddFixed('TSX',$00ba,15); AddFixed('TXA',$008a,15);
   AddFixed('TXB',$428a,14); AddFixed('TXS',$009a,15);
   AddFixed('TXY',$009b,15); AddFixed('TYA',$0098,15);
   AddFixed('TYB',$4298,15); AddFixed('TYX',$00bb,15);
   AddFixed('WIT',$00cb,15); AddFixed('XAB',$8928,14);
   AddFixed('COP',$0002,1); AddFixed('CLD',$00d8, 1);
   AddFixed('SED',$00f8,1); AddFixed('TCS',$001b,15);
   AddFixed('TSC',$003b,15); AddFixed('TCD',$005b,15);
   AddFixed('TDC',$007b,15); AddFixed('PHK',$004b, 1);
   AddFixed('WAI',$00cb,1); AddFixed('XBA',$00eb, 1);
   AddFixed('SWA',$00eb,1); AddFixed('XCE',$00fb, 1);
   IF MomCPU>=CPUM7700 THEN AddFixed('DEA',$001a,15) ELSE AddFixed('DEA',$003a,15);
   IF MomCPU>=CPUM7700 THEN AddFixed('INA',$003a,15) ELSE AddFixed('DEA',$001a,15);

   New(RelOrders); z:=0;
   AddRel('BCC' ,$0090, 2,-1);
   AddRel('BLT' ,$0090, 2,-1);
   AddRel('BCS' ,$00b0, 2,-1);
   AddRel('BGE' ,$00b0, 2,-1);
   AddRel('BEQ' ,$00f0, 2,-1);
   AddRel('BMI' ,$0030, 2,-1);
   AddRel('BNE' ,$00d0, 2,-1);
   AddRel('BPL' ,$0010, 2,-1);
   AddRel('BRA' ,$8280, 2, 3);
   AddRel('BVC' ,$0050, 2,-1);
   AddRel('BVS' ,$0070, 2,-1);
   AddRel('BRL' ,$8200,-1, 3);
   AddRel('BRAL',$8200,-1, 3);

   New(AccOrders); z:=0;
   AddAcc('ADC',$60);
   AddAcc('AND',$20);
   AddAcc('CMP',$c0);
   AddAcc('CPA',$c0);
   AddAcc('EOR',$40);
   AddAcc('LDA',$a0);
   AddAcc('ORA',$00);
   AddAcc('SBC',$e0);
   AddAcc('STA',$80);

   New(RMWOrders); z:=0;
   AddRMW('ASL',$0a,$06);
   IF MomCPU>=CPUM7700 THEN AddRMW('DEC',$1a,$c6) ELSE AddRMW('DEC',$3a,$c6);
   AddRMW('ROL',$2a,$26);
   IF MomCPU>=CPUM7700 THEN AddRMW('INC',$3a,$e6) ELSE AddRMW('INC',$1a,$e6);
   AddRMW('LSR',$4a,$46);
   AddRMW('ROR',$6a,$66);

   New(Imm8Orders); z:=0;
   AddImm8('CLP',$00c2,15);
   AddImm8('REP',$00c2,15);
   AddImm8('LDT',$89c2,14);
   AddImm8('SEP',$00e2,15);
   AddImm8('RMPA',$89e2,8);

   New(XYOrders); z:=0;
   AddXY('CPX',$e0,$e4,$ec,$ff,$ff,$ff,$ff);
   AddXY('CPY',$c0,$c4,$cc,$ff,$ff,$ff,$ff);
   AddXY('LDX',$a2,$a6,$ae,$ff,$ff,$b6,$be);
   AddXY('LDY',$a0,$a4,$ac,$b4,$bc,$ff,$ff);
   AddXY('STX',$ff,$86,$8e,$ff,$ff,$96,$ff);
   AddXY('STY',$ff,$84,$8c,$94,$ff,$ff,$ff);

   New(MulDivOrders); z:=0;
   AddMulDiv('MPY',$0000,14); AddMulDiv('MPYS',$0080,12);
   AddMulDiv('DIV',$0020,14); AddMulDiv('DIVS',$00a0,12); (*???*)
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(RelOrders);
   Dispose(AccOrders);
   Dispose(RMWOrders);
   Dispose(Imm8Orders);
   Dispose(XYOrders);
   Dispose(MulDivOrders);
END;

{-----------------------------------------------------------------------------}

	PROCEDURE DecodeAdr(Start:Integer; Mask:LongInt);
LABEL
   AdrFnd;
VAR
   AdrWord:Word;
   OK:Boolean;
   HCnt:Integer;
   HStr:ARRAY[1..2] OF String;

	PROCEDURE CodeDisp(VAR Asc:String; Start:LongInt);
VAR
   OK:Boolean;
   Adr:LongInt;
   DType:ShortInt;
BEGIN
   IF (Length(Asc)>1) AND (Asc[1]='<') THEN
    BEGIN
     Delete(Asc,1,1); DType:=0;
    END
   ELSE IF (Length(Asc)>1) AND (Asc[1]='>') THEN
    IF (Length(Asc)>2) AND (Asc[2]='>') THEN
     BEGIN
      Delete(Asc,1,2); DType:=2;
     END
    ELSE
     BEGIN
      Delete(Asc,1,1); DType:=1;
     END
   ELSE DType:=-1;

   Adr:=EvalIntExpression(Asc,UInt24,OK);

   IF NOT OK THEN Exit;

   IF DType=-1 THEN
    BEGIN
     IF ((Mask AND (1 SHL Start))<>0) AND (Adr>=Reg_DPR) AND (Adr<Reg_DPR+$100) THEN DType:=0
     ELSE IF ((Mask AND (2 SHL Start))<>0) AND (Adr SHR 16=BankReg) THEN DType:=1
     ELSE DType:=2;
    END;

   IF (Mask AND (1 SHL (Start+DType))=0) THEN WrError(1350)
   ELSE CASE DType OF
   0:IF (FirstPassUnknown) OR (ChkRange(Adr,Reg_DPR,Reg_DPR+$ff)) THEN
      BEGIN
       AdrCnt:=1; AdrType:=Start;
       AdrVals[0]:=Lo(Adr-Reg_DPR);
      END;
   1:IF (NOT FirstPassUnknown) AND (Adr SHR 16<>BankReg) THEN WrError(1320)
     ELSE
      BEGIN
       AdrCnt:=2; AdrType:=Start+1;
       AdrVals[0]:=Lo(Adr); AdrVals[1]:=Hi(Adr);
      END;
   2:BEGIN
      AdrCnt:=3; AdrType:=Start+2;
      AdrVals[0]:=Lo(Adr); AdrVals[1]:=Hi(Adr); AdrVals[2]:=Adr SHR 16;
     END;
   END;
END;

	PROCEDURE SplitArg(Src:String);
VAR
   p:Integer;
BEGIN
   Src:=Copy(Src,2,Length(Src)-2);
   p:=QuotPos(Src,',');
   SplitString(Src,HStr[1],HStr[2],p);
   HCnt:=Ord(p<=Length(Src))+1;
END;

BEGIN
   AdrType:=ModNone; AdrCnt:=0; BankReg:=Reg_DT;

   { I. 1 Parameter }

   IF Start=ArgCnt THEN
    BEGIN
     { I.1. immediate }

     IF ArgStr[Start][1]='#' THEN
      BEGIN
       HStr[1]:=Copy(ArgStr[Start],2,Length(ArgStr[Start])-1);
       IF WordSize THEN
	BEGIN
	 AdrWord:=EvalIntExpression(HStr[1],Int16,OK);
	 AdrVals[0]:=Lo(AdrWord); AdrVals[1]:=Hi(AdrWord);
	END
       ELSE AdrVals[0]:=EvalIntExpression(HStr[1],Int8,OK);
       IF OK THEN
	BEGIN
	 AdrCnt:=1+Ord(WordSize); AdrType:=ModImm;
	END;
       Goto AdrFnd;
      END;

     { I.2. indirekt }

     IF IsIndirect(ArgStr[Start]) THEN
      BEGIN
       SplitArg(ArgStr[Start]);

       { I.2.i. einfach indirekt }

       IF HCnt=1 THEN
	BEGIN
	 CodeDisp(HStr[1],ModInd8); Goto AdrFnd;
	END

       { I.2.ii indirekt mit Vorindizierung }

       ELSE IF NLS_StrCaseCmp(HStr[2],'X')=0 THEN
	BEGIN
	 CodeDisp(HStr[1],ModIndX8); Goto AdrFnd;
	END

       ELSE
	BEGIN
	 WrError(1350); Goto AdrFnd;
	END;
      END

     { I.3. absolut }

     ELSE
      BEGIN
       CodeDisp(ArgStr[Start],ModAbs8); Goto AdrFnd;
      END;
    END

   { II. 2 Parameter }

   ELSE IF Start+1=ArgCnt THEN
    BEGIN
     { II.1 indirekt mit Nachindizierung }

     IF IsIndirect(ArgStr[Start]) THEN
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[Start+1],'Y')<>0 THEN WrError(1350)
       ELSE
	BEGIN
	 SplitArg(ArgStr[Start]);

	 { II.1.i. (d),Y }

	 IF HCnt=1 THEN
	  BEGIN
	   CodeDisp(HStr[1],ModIndY8); Goto AdrFnd;
	  END

	 { II.1.ii. (d,S),Y }

         ELSE IF NLS_StrCaseCmp(HStr[2],'S')=0 THEN
	  BEGIN
	   AdrVals[0]:=EvalIntExpression(HStr[1],Int8,OK);
	   IF OK THEN
	    BEGIN
	     AdrType:=ModIndS8; AdrCnt:=1;
	    END;
	   Goto AdrFnd;
	  END

	 ELSE WrError(1350);
	END;
       Goto AdrFnd;
      END

     { II.2. einfach indiziert }

     ELSE
      BEGIN
       { II.2.i. d,X }

       IF NLS_StrCaseCmp(ArgStr[Start+1],'X')=0 THEN
	BEGIN
	 CodeDisp(ArgStr[Start],ModIdxX8); Goto AdrFnd;
	END

       { II.2.ii. d,Y }

       ELSE IF NLS_StrCaseCmp(ArgStr[Start+1],'Y')=0 THEN
	BEGIN
	 CodeDisp(ArgStr[Start],ModIdxY8); Goto AdrFnd;
	END

       { II.2.iii. d,S }

       ELSE IF NLS_StrCaseCmp(ArgStr[Start+1],'S')=0 THEN
	BEGIN
	 AdrVals[0]:=EvalIntExpression(ArgStr[Start],Int8,OK);
	 IF OK THEN
	  BEGIN
	   AdrType:=ModIdxS8; AdrCnt:=1;
	  END;
	 Goto AdrFnd;
	END

       ELSE WrError(1350);
      END;
    END

   ELSE WrError(1110);

AdrFnd:
   IF AdrType<>ModNone THEN
    IF Mask AND (1 SHL LongInt(AdrType))=0 THEN
     BEGIN
      AdrType:=ModNone; AdrCnt:=0; WrError(1350);
     END;
END;

	FUNCTION DecodePseudo:Boolean;
TYPE
   Ass7700=RECORD
	    Name:String[3];
	    Size:IntType;
	    Ref:^Word;
	   END;
CONST
   ASSUME7700Count=5;
   ASSUME7700s:ARRAY[1..ASSUME7700Count] OF ASSUMERec=
	       ((Name:'PG' ; Dest:@Reg_PG ; Min:0; Max:  $ff; NothingVal:  $100),
		(Name:'DT' ; Dest:@Reg_DT ; Min:0; Max:  $ff; NothingVal:  $100),
		(Name:'X'  ; Dest:@Reg_X  ; Min:0; Max:    1; NothingVal:    -1),
		(Name:'M'  ; Dest:@Reg_M  ; Min:0; Max:    1; NothingVal:    -1),
		(Name:'DPR'; Dest:@Reg_DPR; Min:0; Max:$ffff; NothingVal:$10000));

VAR
   RegPart,ValPart:String;
   OK:Boolean;
   z,z2:Integer;
   erg:Word;
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME7700s,ASSUME7700Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

	FUNCTION LMemo(s:String):Boolean;
BEGIN
   IF Memo(s) THEN
    BEGIN
     LMemo:=True; LFlag:=False;
    END
   ELSE
    BEGIN
     s[Length(s)+1]:='L'; Inc(Byte(s[0]));
     IF Memo(s) THEN
      BEGIN
       LMemo:=True; LFlag:=True;
      END
     ELSE LMemo:=False;
    END;
END;

	PROCEDURE MakeCode_7700;
	Far;
VAR
   z,Start:Integer;
   AdrLong,Mask:LongInt;
   OK,Rel:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   IF Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeMotoPseudo(False) THEN Exit;
   IF DecodeIntelPseudo(False) THEN Exit;

   { ohne Argument }

   IF Memo('BRK') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$00; BAsmCode[1]:=NOPCode;
      END;
     Exit;
    END;

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF  NOT Odd(Allowed SHR (Ord(MomCPU)-Ord(CPU65816))) THEN WrError(1500)
       ELSE IF Hi(Code)=0 THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=Code;
	END
       ELSE
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
	END;
       Exit;
      END;

   IF (Memo('PHB')) OR (Memo('PLB')) THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       IF MomCPU>=CPUM7700 THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=PrefAccB; BAsmCode[1]:=$48;
	END
       ELSE
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$8b;
	END;
       IF Memo('PLB') THEN Inc(BAsmCode[CodeLen-1],$20);
      END;
     Exit;
    END;

   { relative Adressierung }

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 IF ArgStr[1][1]='#' THEN Delete(ArgStr[1],1,1);
         AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
	 IF Ok THEN
	  BEGIN
	   OK:=Disp8=-1;
	   IF OK THEN
	    Dec(AdrLong,EProgCounter+Disp16)
	   ELSE
	    BEGIN
	     Dec(AdrLong,EProgCounter+Disp8);
	     IF ((AdrLong>127) OR (AdrLong<-128)) AND (Disp16<>-1) THEN
	      BEGIN
	       OK:=True; Dec(AdrLong,Disp16-Disp8);
	      END;
	    END;
	   IF OK THEN         {d16}
            IF ((AdrLong<-32768) OR (AdrLong>32767)) AND (NOT SymbolQuestionable) THEN WrError(1330)
	    ELSE
	     BEGIN
	      CodeLen:=3; BAsmCode[0]:=Hi(Code);
	      BAsmCode[1]:=Lo(AdrLong); BAsmCode[2]:=Hi(AdrLong);
	     END
	   ELSE               {d8}
            IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
	    ELSE
	     BEGIN
	      CodeLen:=2; BAsmCode[0]:=Lo(Code);
	      BAsmCode[1]:=Lo(AdrLong);
	     END;
	  END;
	END;
       Exit;
      END;

   { mit Akku }

   FOR z:=1 TO AccOrderCnt DO
    WITH AccOrders^[z] DO
     IF LMemo(Name) THEN
      BEGIN
       IF (ArgCnt=0) OR (ArgCnt>3) THEN WrError(1110)
       ELSE
	BEGIN
	 WordSize:=Reg_M=0;
         IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN Start:=2
         ELSE IF NLS_StrCaseCmp(ArgStr[1],'B')=0 THEN
	  BEGIN
	   Start:=2; BAsmCode[0]:=PrefAccB; Inc(CodeLen);
	   IF MomCPU=CPU65816 THEN
	    BEGIN
	     WrError(1505); Exit;
	    END;
	  END
	 ELSE Start:=1;
	 Mask:=MModAbs8+MModAbs16+MModAbs24+
	       MModIdxX8+MModIdxX16+MModIdxX24+
	       MModIdxY16+
	       MModInd8+MModIndX8+MModIndY8+
	       MModIdxS8+MModIndS8;
	 IF LMemo('STA') THEN DecodeAdr(Start,Mask)
	 ELSE DecodeAdr(Start,Mask+MModImm);
	 IF AdrType<>ModNone THEN
	  IF (LFlag) AND (AdrType<>ModInd8) AND (AdrType<>ModIndY8) THEN WrError(1350)
	  ELSE
	   BEGIN
	    CASE AdrType OF
	    ModImm    : BAsmCode[CodeLen]:=Code+$09;
	    ModAbs8   : BAsmCode[CodeLen]:=Code+$05;
	    ModAbs16  : BAsmCode[CodeLen]:=Code+$0d;
	    ModAbs24  : BAsmCode[CodeLen]:=Code+$0f;
	    ModIdxX8  : BAsmCode[CodeLen]:=Code+$15;
	    ModIdxX16 : BAsmCode[CodeLen]:=Code+$1d;
	    ModIdxX24 : BAsmCode[CodeLen]:=Code+$1f;
	    ModIdxY16 : BAsmCode[CodeLen]:=Code+$19;
	    ModInd8   : IF LFlag THEN BAsmCode[CodeLen]:=Code+$07
				 ELSE BAsmCode[CodeLen]:=Code+$12;
	    ModIndX8  : BAsmCode[CodeLen]:=Code+$01;
	    ModIndY8  : IF LFlag THEN BAsmCode[CodeLen]:=Code+$17
				 ELSE BAsmCode[CodeLen]:=Code+$11;
	    ModIdxS8  : BAsmCode[CodeLen]:=Code+$03;
	    ModIndS8  : BAsmCode[CodeLen]:=Code+$13;
	    END;
	    Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt); Inc(CodeLen,1+AdrCnt);
	   END;
	END;
       Exit;
      END;

   IF (Memo('EXTS')) OR (Memo('EXTZ')) THEN
    BEGIN
     IF ArgCnt=0 THEN
      BEGIN
       ArgStr[1]:='A'; ArgCnt:=1;
      END;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPUM7750 THEN WrError(1500)
     ELSE
      BEGIN
       BAsmCode[1]:=$8b+(Ord(Memo('EXTZ')) SHL 5);
       BAsmCode[0]:=0;
       IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN BAsmCode[0]:=$89
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'B')=0 THEN BAsmCode[0]:=$42
       ELSE WrError(1350);
       IF BAsmCode[0]<>0 THEN CodeLen:=2;
      END;
     Exit;
    END;

   FOR z:=1 TO RMWOrderCnt DO
    WITH RMWOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt=0) OR ((ArgCnt=1) AND (NLS_StrCaseCmp(ArgStr[1],'A')=0)) THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=ACode;
	END
       ELSE IF (ArgCnt=1) AND (NLS_StrCaseCmp(ArgStr[1],'B')=0) THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=PrefAccB; BAsmCode[1]:=ACode;
	 IF MomCPU=CPU65816 THEN
	  BEGIN
	   WrError(1505); Exit;
	  END;
	END
       ELSE IF ArgCnt>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(1,MModAbs8+MModAbs16+MModIdxX8+MModIdxX16);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CASE AdrType OF
	   ModAbs8   : BAsmCode[0]:=MCode;
	   ModAbs16  : BAsmCode[0]:=MCode+8;
	   ModIdxX8  : BAsmCode[0]:=MCode+16;
	   ModIdxX16 : BAsmCode[0]:=MCode+24;
	   END;
	   Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
	  END;
	END;
       Exit;
      END;

   IF Memo('ASR') THEN
    BEGIN
     IF MomCPU<CPUM7750 THEN WrError(1500)
     ELSE IF (ArgCnt=0) OR ((ArgCnt=1) AND (NLS_StrCaseCmp(ArgStr[1],'A')=0)) THEN
      BEGIN
       BAsmCode[0]:=$89; BAsmCode[1]:=$08; CodeLen:=2;
      END
     ELSE IF (ArgCnt=1) AND (NLS_StrCaseCmp(ArgStr[1],'B')=0) THEN
      BEGIN
       BAsmCode[0]:=$42; BAsmCode[1]:=$08; CodeLen:=2;
      END
     ELSE IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(1,MModAbs8+MModIdxX8+MModAbs16+MModIdxX16);
       IF AdrType<>ModNone THEN
        BEGIN
         BAsmCode[0]:=$89;
         CASE AdrType OF
         ModAbs8:BAsmCode[1]:=$06;
         ModIdxX8:BAsmCode[1]:=$16;
         ModAbs16:BAsmCode[1]:=$0e;
         ModIdxX16:BAsmCode[1]:=$1e;
         END;
         Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
        END;
      END;
     Exit;
    END;

   IF (Memo('BBC')) OR (Memo('BBS')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF MomCPU<CPUM7700 THEN WrError(1500)
     ELSE
      BEGIN
       WordSize:=Reg_M=0;
       ArgCnt:=2; DecodeAdr(2,MModAbs8+MModAbs16);
       IF AdrType<>ModNone THEN
	BEGIN
	 BAsmCode[0]:=$24;
	 IF Memo('BBC') THEN Inc(BAsmCode[0],$10);
	 IF AdrType=ModAbs16 THEN Inc(BAsmCode[0],8);
	 Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
	 ArgCnt:=1; DecodeAdr(1,MModImm);
	 IF AdrType=ModNone THEN CodeLen:=0
	 ELSE
	  BEGIN
	   Move(AdrVals,BAsmCode[CodeLen],AdrCnt); Inc(CodeLen,AdrCnt);
           AdrLong:=EvalIntExpression(ArgStr[3],UInt24,OK)-(EProgCounter+CodeLen+1);
	   IF NOT OK THEN CodeLen:=0
	   ELSE IF (AdrLong<-128) OR (AdrLong>127) THEN
	    BEGIN
	     WrError(1370); CodeLen:=0;
	    END
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=Lo(AdrLong); Inc(CodeLen);
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('BIT') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE IF MomCPU<>CPU65816 THEN WrError(1500)
     ELSE
      BEGIN
       WordSize:=False;
       DecodeAdr(1,MModAbs8+MModAbs16+MModIdxX8+MModIdxX16+MModImm);
       IF AdrType<>ModNone THEN
	BEGIN
	 CASE AdrType OF
	 ModAbs8:BAsmCode[0]:=$24;
	 ModAbs16:BAsmCode[0]:=$2c;
	 ModIdxX8:BAsmCode[0]:=$34;
	 ModIdxX16:BAsmCode[0]:=$3c;
	 ModImm:BAsmCode[0]:=$89;
	 END;
	 Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
	END;
      END;
     Exit;
    END;

   IF (Memo('CLB')) OR (Memo('SEB')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPUM7700 THEN WrError(1500)
     ELSE
      BEGIN
       WordSize:=Reg_M=0;
       DecodeAdr(2,MModAbs8+MModAbs16);
       IF AdrType<>ModNone THEN
	BEGIN
	 BAsmCode[0]:=$04;
	 IF Memo('CLB') THEN Inc(BAsmCode[0],$10);
	 IF AdrType=ModAbs16 THEN Inc(BAsmCode[0],8);
	 Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
	 ArgCnt:=1; DecodeAdr(1,MModImm);
	 IF AdrType=ModNone THEN CodeLen:=0
	 ELSE
	  BEGIN
	   Move(AdrVals,BAsmCode[CodeLen],AdrCnt); Inc(CodeLen,AdrCnt);
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('TSB')) OR (Memo('TRB')) THEN
    BEGIN
     IF MomCPU=CPU65816 THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(1,MModAbs8+MModAbs16);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$04;
	   IF Memo('TRB') THEN Inc(BAsmCode[0],$10);
	   IF AdrType=ModAbs16 THEN Inc(BAsmCode[0],8);
	   Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
	  END;
	END;
      END
     ELSE IF Memo('TRB') THEN WrError(1500)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=02; BAsmCode[0]:=$42; BAsmCode[1]:=$3b;
      END;
     Exit;
    END;

   FOR z:=1 TO Imm8OrderCnt DO
    WITH Imm8Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF  NOT Odd(Allowed SHR (Ord(MomCPU)-Ord(CPU65816))) THEN WrError(1500)
       ELSE
	BEGIN
	 WordSize:=False;
	 DecodeAdr(1,MModImm);
	 IF AdrType=ModImm THEN
	  BEGIN
	   IF Hi(Code)=0 THEN
	    BEGIN
	     BAsmCode[0]:=Lo(Code); CodeLen:=1;
	    END
	   ELSE
	    BEGIN
	     BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code); CodeLen:=2;
	    END;
	   Move(AdrVals,BAsmCode[CodeLen],AdrCnt); Inc(CodeLen,AdrCnt);
	  END;
	END;
       Exit;
      END;

   IF Memo('RLA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       WordSize:=Reg_M=0;
       DecodeAdr(1,MModImm);
       IF AdrType<>ModNone THEN
        BEGIN
         CodeLen:=2+AdrCnt; BAsmCode[0]:=$89; BAsmCode[1]:=$49;
         Move(AdrVals,BAsmCode[2],AdrCnt);
        END;
      END;
     Exit;
    END;

   FOR z:=1 TO XYOrderCnt DO
    WITH XYOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE
	BEGIN
	 WordSize:=Reg_X=0; Mask:=0;
	 IF CodeImm   <>$ff THEN Inc(Mask,MModImm);
	 IF CodeAbs8  <>$ff THEN Inc(Mask,MModAbs8);
	 IF CodeAbs16 <>$ff THEN Inc(Mask,MModAbs16);
	 IF CodeIdxX8 <>$ff THEN Inc(Mask,MModIdxX8);
	 IF CodeIdxX16<>$ff THEN Inc(Mask,MModIdxX16);
	 IF CodeIdxY8 <>$ff THEN Inc(Mask,MModIdxY8);
	 IF CodeIdxY16<>$ff THEN Inc(Mask,MModIdxY16);
	 DecodeAdr(1,Mask);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CASE AdrType OF
	   ModImm   :BAsmCode[0]:=CodeImm;
	   ModAbs8  :BAsmCode[0]:=CodeAbs8;
	   ModAbs16 :BAsmCode[0]:=CodeAbs16;
	   ModIdxX8 :BAsmCode[0]:=CodeIdxX8;
	   ModIdxY8 :BAsmCode[0]:=CodeIdxY8;
	   ModIdxX16:BAsmCode[0]:=CodeIdxX16;
	   ModIdxY16:BAsmCode[0]:=CodeIdxY16;
	   END;
	   Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO MulDivOrderCnt DO
    WITH MulDivOrders^[z] DO
     IF LMemo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE IF NOT Odd(Allowed SHR (Ord(MomCPU)-Ord(CPU65816))) THEN WrError(1500)
       ELSE
        BEGIN
         WordSize:=Reg_M=0;
         DecodeAdr(1,MModImm+MModAbs8+MModAbs16+MModAbs24+MModIdxX8+MModIdxX16+
                     MModIdxX24+MModIdxY16+MModInd8+MModIndX8+MModIndY8+
                     MModIdxS8+MModIndS8);
         IF AdrType<>ModNone THEN
          IF (LFlag) AND (AdrType<>ModInd8) AND (AdrType<>ModIndY8) THEN WrError(1350)
         ELSE
          BEGIN
           BAsmCode[0]:=$89;
           CASE AdrType OF
           ModImm    : BAsmCode[1]:=$09;
           ModAbs8   : BAsmCode[1]:=$05;
           ModAbs16  : BAsmCode[1]:=$0d;
           ModAbs24  : BAsmCode[1]:=$0f;
           ModIdxX8  : BAsmCode[1]:=$15;
           ModIdxX16 : BAsmCode[1]:=$1d;
           ModIdxX24 : BAsmCode[1]:=$1f;
           ModIdxY16 : BAsmCode[1]:=$19;
           ModInd8   : IF LFlag THEN BAsmCode[1]:=$07
                                ELSE BAsmCode[1]:=$12;
           ModIndX8  : BAsmCode[1]:=$01;
           ModIndY8  : IF LFlag THEN BAsmCode[1]:=$17
                                ELSE BAsmCode[1]:=$11;
           ModIdxS8  : BAsmCode[1]:=$03;
           ModIndS8  : BAsmCode[1]:=$13;
           END;
           Inc(BAsmCode[1],Code);
           Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
        END;
       Exit;
      END;

   IF (Memo('JML')) OR (Memo('JSL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=4;
	 IF Memo('JSL') THEN BAsmCode[0]:=$22 ELSE BAsmCode[0]:=$5c;
	 BAsmCode[1]:=AdrLong SHR 16;
	 BAsmCode[2]:=Hi(AdrLong);
	 BAsmCode[3]:=Lo(AdrLong);
	END;
      END;
     Exit;
    END;

   IF (LMemo('JMP')) OR (LMemo('JSR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       BankReg:=Reg_PG;
       Mask:=MModAbs24+MModIndX16;
       IF NOT LFlag THEN Inc(Mask,MModAbs16);
       IF LMemo('JSR') THEN DecodeAdr(1,Mask)
       ELSE DecodeAdr(1,Mask+MModInd16);
       IF AdrType<>ModNone THEN
        BEGIN
         CASE AdrType OF
         ModAbs16:IF LMemo('JSR') THEN BAsmCode[0]:=$20
                                  ELSE BAsmCode[0]:=$4c;
         ModAbs24:IF LMemo('JSR') THEN BAsmCode[0]:=$22
                                  ELSE BAsmCode[0]:=$5c;
         ModIndX16:IF LMemo('JSR') THEN BAsmCode[0]:=$fc
                                   ELSE BAsmCode[0]:=$7c;
         ModInd16:IF LFlag THEN BAsmCode[0]:=$dc
                           ELSE BAsmCode[0]:=$6c;
         END;
         Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
        END;
      END;
     Exit;
    END;

   IF Memo('LDM') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE IF MomCPU<CPUM7700 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(2,MModAbs8+MModAbs16+MModIdxX8+MModIdxX16);
       IF AdrType<>ModNone THEN
	BEGIN
	 CASE AdrType OF
	 ModAbs8  : BAsmCode[0]:=$64;
	 ModAbs16 : BAsmCode[0]:=$9c;
	 ModIdxX8 : BAsmCode[0]:=$74;
	 ModIdxX16: BAsmCode[0]:=$9e;
	 END;
	 Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
	 WordSize:=Reg_M=0;
	 ArgCnt:=1; DecodeAdr(1,MModImm);
	 IF AdrType=ModNone THEN CodeLen:=0
	 ELSE
	  BEGIN
	   Move(AdrVals,BAsmCode[CodeLen],AdrCnt); Inc(CodeLen,AdrCnt);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('STZ') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE IF MomCPU<>CPU65816 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(1,MModAbs8+MModAbs16+MModIdxX8+MModIdxX16);
       IF AdrType<>ModNone THEN
	BEGIN
	 CASE AdrType OF
	 ModAbs8  : BAsmCode[0]:=$64;
	 ModAbs16 : BAsmCode[0]:=$9c;
	 ModIdxX8 : BAsmCode[0]:=$74;
	 ModIdxX16: BAsmCode[0]:=$9e;
	 END;
	 Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
	END;
      END;
     Exit;
    END;

   IF (Memo('MVN')) OR (Memo('MVP')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
	BEGIN
         Mask:=EvalIntExpression(ArgStr[2],UInt24,OK);
	 IF OK THEN
          BEGIN
           IF Memo('MVN') THEN BAsmCode[0]:=$54 ELSE BAsmCode[0]:=$44;
           BAsmCode[1]:=AdrLong SHR 16;
           BAsmCode[2]:=Mask SHR 16;
           CodeLen:=3;
          END;
	END;
      END;
     Exit;
    END;

   IF (Memo('PSH')) OR (Memo('PUL')) THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE IF MomCPU<CPUM7700 THEN WrError(1500)
     ELSE
      BEGIN
       BAsmCode[0]:=$eb+(Ord(Memo('PUL')) SHL 4);
       BAsmCode[1]:=0; OK:=True;
       z:=1;
       WHILE (z<=ArgCnt) AND (OK) DO
	BEGIN
	 IF ArgStr[z][1]='#' THEN
	  BAsmCode[1]:=BAsmCode[1] OR
	  EvalIntExpression(Copy(ArgStr[z],2,Length(ArgStr[z])-1),Int8,OK)
	 ELSE
	  BEGIN
	   Start:=0;
           WHILE (Start<PushRegCnt) AND (NLS_StrCaseCmp(PushRegs[Start],ArgStr[z])<>0) DO
	    Inc(Start);
	   OK:=Start<PushRegCnt;
           IF OK THEN BAsmCode[1]:=BAsmCode[1] OR (1 SHL Start)
	   ELSE WrXError(1980,ArgStr[z]);
	  END;
	 Inc(z);
	END;
       IF OK THEN CodeLen:=2;
      END;
     Exit;
    END;

   IF Memo('PEA') THEN
    BEGIN
     WordSize:=True;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(1,MModImm);
       IF AdrType<>ModNone THEN
        BEGIN
         CodeLen:=1+AdrCnt; BAsmCode[0]:=$f4;
         Move(AdrVals,BAsmCode[1],AdrCnt);
        END;
      END;
     Exit;
    END;

   IF Memo('PEI') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgStr[1][1]='#' THEN Delete(ArgStr[1],1,1);
       DecodeAdr(1,MModAbs8);
       IF AdrType<>ModNone THEN
        BEGIN
         CodeLen:=1+AdrCnt; BAsmCode[0]:=$d4;
         Move(AdrVals,BAsmCode[1],AdrCnt);
        END;
      END;
     Exit;
    END;

   IF Memo('PER') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       Rel:=True;
       IF ArgStr[1][1]='#' THEN
        BEGIN
         Delete(ArgStr[1],1,1); Rel:=False;
        END;
       BAsmCode[0]:=$62;
       IF Rel THEN
        BEGIN
         AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK)-(EProgCounter+2);
         IF OK THEN
          IF (AdrLong<-32768) OR (AdrLong>32767) THEN WrError(1370)
          ELSE
           BEGIN
            CodeLen:=3; BAsmCode[1]:=AdrLong AND $ff;
            BAsmCode[2]:=(AdrLong SHR 8) AND $ff;
           END;
        END
       ELSE
        BEGIN
         z:=EvalIntExpression(ArgStr[1],Int16,OK);
         IF OK THEN
          BEGIN
           CodeLen:=3; BAsmCode[1]:=Lo(z); BAsmCode[2]:=Hi(z);
          END;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_7700;
	Far;
BEGIN
   SaveInitProc;
   Reg_PG:=0;
   Reg_DT:=0;
   Reg_X:=0;
   Reg_M:=0;
   Reg_DPR:=0;
END;

	FUNCTION ChkPC_7700:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<=$ffffff
   ELSE ok:=False;
   END;
   ChkPC_7700:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_7700:Boolean;
	Far;
BEGIN
   IsDef_7700:=False;
END;

        PROCEDURE SwitchFrom_7700;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_7700;
	FAR;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$19; NOPCode:=$ea;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_7700; ChkPC:=ChkPC_7700; IsDef:=IsDef_7700;
   SwitchFrom:=SwitchFrom_7700;

   InitFields;
END;

BEGIN
   CPU65816:=AddCPU('65816'    ,SwitchTo_7700);
   CPUM7700:=AddCPU('MELPS7700',SwitchTo_7700);
   CPUM7750:=AddCPU('MELPS7750',SwitchTo_7700);
   CPUM7751:=AddCPU('MELPS7751',SwitchTo_7700);


   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_7700;
END.
