{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

{ AS - Codegeneratormodul TMS7000 }

        UNIT CodeTMS7;

INTERFACE

        Uses StringUt,NLS,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	      END;

CONST
   ModNone=-1;
   ModAccA=0;      MModAccA=1 SHL ModAccA;       { A }
   ModAccB=1;      MModAccB=1 SHL ModAccB;       { B }
   ModReg=2;       MModReg=1 SHL ModReg;         { Rn }
   ModPort=3;      MModPort=1 SHL ModPort;       { Pn }
   ModAbs=4;       MModAbs=1 SHL ModAbs;         { nnnn }
   ModBRel=5;      MModBRel=1 SHL ModBRel;       { nnnn(B) }
   ModIReg=6;      MModIReg=1 SHL ModIReg;       { *Rn }
   ModImm=7;       MModImm=1 SHL ModImm;         { #nn }
   ModImmBRel=8;   MModImmBRel=1 SHL ModImmBRel; { #nnnn(b) }


   FixedOrderCount=14;
   Rel8OrderCount=12;
   ALU1OrderCount=7;
   ALU2OrderCount=5;
   JmpOrderCount=2;
   ABRegOrderCount=14;

TYPE
   FixedOrderArray =ARRAY[1..FixedOrderCount] OF FixedOrder;
   Rel8OrderArray  =ARRAY[1..Rel8OrderCount ] OF FixedOrder;
   ALU1OrderArray  =ARRAY[1..ALU1OrderCount ] OF FixedOrder;
   ALU2OrderArray  =ARRAY[1..ALU2OrderCount ] OF FixedOrder;
   JmpOrderArray   =ARRAY[1..JmpOrderCount  ] OF FixedOrder;
   ABRegOrderArray =ARRAY[1..ABRegOrderCount] OF FixedOrder;

VAR
   CPU70C40,CPU70C20,CPU70C00,
   CPU70CT40,CPU70CT20,
   CPU70C82,CPU70C42,CPU70C02,
   CPU70C48,CPU70C08:CPUVar;

   OpSize:Byte;
   AdrType:ShortInt;
   AdrCnt:Byte;
   AdrVals:ARRAY[0..1] OF Byte;

   FixedOrders:^FixedOrderArray;
   Rel8Orders:^Rel8OrderArray;
   ALU1Orders:^ALU1OrderArray;
   ALU2Orders:^ALU2OrderArray;
   JmpOrders:^JmpOrderArray;
   ABRegOrders:^ABRegOrderArray;

	PROCEDURE InitFields;
VAR
   z:Integer;

	PROCEDURE InitFixed(NName:String; NCode:Word);
BEGIN
   IF z>FixedOrderCount THEN Exit;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitRel8(NName:String; NCode:Word);
BEGIN
   IF z>Rel8OrderCount THEN Exit;
   WITH Rel8Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitALU1(NName:String; NCode:Word);
BEGIN
   IF z>ALU1OrderCount THEN Exit;
   WITH ALU1Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitALU2(NName:String; NCode:Word);
BEGIN
   IF z>ALU2OrderCount THEN Exit;
   WITH ALU2Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitJmp(NName:String; NCode:Word);
BEGIN
   IF z>JmpOrderCount THEN Exit;
   WITH JmpOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitABReg(NName:String; NCode:Word);
BEGIN
   IF z>ABRegOrderCount THEN Exit;
   WITH ABRegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(FixedOrders); z:=1;
   InitFixed('CLRC' ,$00b0); InitFixed('DINT' ,$0060);
   InitFixed('EINT' ,$0005); InitFixed('IDLE' ,$0001);
   InitFixed('LDSP' ,$000d); InitFixed('NOP'  ,$0000);
   InitFixed('RETI' ,$000b); InitFixed('RTI'  ,$000b);
   InitFixed('RETS' ,$000a); InitFixed('RTS'  ,$000a);
   InitFixed('SETC' ,$0007); InitFixed('STSP' ,$0009);
   InitFixed('TSTA' ,$00b0); InitFixed('TSTB' ,$00c1);

   New(Rel8Orders); z:=1;
   InitRel8('JMP',$e0); InitRel8('JC' ,$e3); InitRel8('JEQ',$e2);
   InitRel8('JHS',$e3); InitRel8('JL' ,$e7); InitRel8('JN' ,$e1);
   InitRel8('JNC',$e7); InitRel8('JNE',$e6); InitRel8('JNZ',$e6);
   InitRel8('JP' ,$e4); InitRel8('JPZ',$e5); InitRel8('JZ' ,$e2);

   New(ALU1Orders); z:=1;
   InitALU1('ADC', 9); InitALU1('ADD', 8);
   InitALU1('DAC',14); InitALU1('DSB',15);
   InitALU1('SBB',11); InitALU1('SUB',10); InitALU1('MPY',12);

   New(ALU2Orders); z:=1;
   InitALU2('AND' , 3); InitALU2('BTJO', 6);
   InitALU2('BTJZ', 7); InitALU2('OR'  , 4); InitALU2('XOR', 5);

   New(JmpOrders); z:=1;
   InitJmp('BR'  ,12); InitJmp('CALL' ,14);

   New(ABRegOrders); z:=1;
   InitABReg('CLR'  , 5); InitABReg('DEC'  , 2); InitABReg('DECD' ,11);
   InitABReg('INC'  , 3); InitABReg('INV'  , 4); InitABReg('POP'  , 9);
   InitABReg('PUSH' , 8); InitABReg('RL'   ,14); InitABReg('RLC'  ,15);
   InitABReg('RR'   ,12); InitABReg('RRC'  ,13); InitABReg('SWAP' , 7);
   InitABReg('XCHB' , 6); InitABReg('DJNZ' ,10);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(Rel8Orders);
   Dispose(ALU1Orders);
   Dispose(ALU2Orders);
   Dispose(JmpOrders);
   Dispose(ABRegOrders);
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   AdrFound;
VAR
   HVal,z,Lev:Integer;
   OK:Boolean;
BEGIN
   AdrType:=ModNone; AdrCnt:=0;

   IF NLS_StrCaseCmp(Asc,'A')=0 THEN
    BEGIN
     IF (Mask AND MModAccA<>0) THEN AdrType:=ModAccA
     ELSE IF (Mask AND MModReg<>0) THEN
      BEGIN
       AdrCnt:=1; AdrVals[0]:=0; AdrType:=ModReg;
      END
     ELSE
      BEGIN
       AdrCnt:=2; AdrVals[0]:=0; AdrVals[1]:=0; AdrType:=ModAbs;
      END;
     Goto AdrFound;
    END;

   IF NLS_StrCaseCmp(Asc,'B')=0 THEN
    BEGIN
     IF (Mask AND MModAccB<>0) THEN AdrType:=ModAccB
     ELSE IF (Mask AND MModReg<>0) THEN
      BEGIN
       AdrCnt:=1; AdrVals[0]:=1; AdrType:=ModReg;
      END
     ELSE
      BEGIN
       AdrCnt:=2; AdrVals[0]:=0; AdrVals[1]:=1; AdrType:=ModAbs;
      END;
     Goto AdrFound;
    END;

   IF (Asc[1]='#') OR (Asc[1]='%') THEN
    BEGIN
     Delete(Asc,1,1);
     IF NLS_StrCaseCmp(Copy(Asc,Length(Asc)-2,3),'(B)')=0 THEN
      BEGIN
       HVal:=EvalIntExpression(Copy(Asc,1,Length(Asc)-3),Int16,OK);
       IF OK THEN
        BEGIN
         AdrVals[0]:=Hi(HVal); AdrVals[1]:=Lo(HVal);
         AdrType:=ModImmBRel; AdrCnt:=2;
        END;
      END
     ELSE
      BEGIN
       CASE OpSize OF
       0:AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
       1:BEGIN
          HVal:=EvalIntExpression(Asc,Int16,OK);
          AdrVals[0]:=Hi(HVal); AdrVals[1]:=Lo(HVal);
         END;
       END;
       IF OK THEN
        BEGIN
         AdrCnt:=1+OpSize; AdrType:=ModImm;
        END;
      END;
     Goto AdrFound;
    END;

   IF Asc[1]='*' THEN
    BEGIN
     Delete(Asc,1,1);
     AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
     IF OK THEN
      BEGIN
       AdrCnt:=1; AdrType:=ModIReg;
      END;
     Goto AdrFound;
    END;

   IF Asc[1]='@' THEN Delete(Asc,1,1);

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

   IF z=0 THEN
    BEGIN
     HVal:=EvalIntExpression(Asc,Int16,OK);
     IF OK THEN
      IF (Mask AND MModReg<>0) AND (Hi(HVal)=0) THEN
       BEGIN
	AdrVals[0]:=Lo(HVal); AdrCnt:=1; AdrType:=ModReg;
       END
      ELSE IF (Mask AND MModPort<>0) AND (Hi(HVal)=$01) THEN
       BEGIN
	AdrVals[0]:=Lo(HVal); AdrCnt:=1; AdrType:=ModPort;
       END
      ELSE
       BEGIN
        AdrVals[0]:=Hi(HVal); AdrVals[1]:=Lo(HVal); AdrCnt:=2;
	AdrType:=ModAbs;
       END;
     Goto AdrFound;
    END
   ELSE
    BEGIN
     FirstPassUnknown:=False;
     HVal:=EvalIntExpression(Copy(Asc,1,z-1),Int16,OK);
     IF OK THEN
      BEGIN
       Asc:=Copy(Asc,z+1,Length(Asc)-z-1);
       IF NLS_StrCaseCmp(Asc,'B')=0 THEN
	BEGIN
         AdrVals[0]:=Hi(HVal); AdrVals[1]:=Lo(HVal); AdrCnt:=2;
	 AdrType:=ModBRel;
	END
       ELSE WrXError(1445,Asc);
      END;
     Goto AdrFound;
    END;

AdrFound:
   IF (AdrType<>-1) AND (Mask AND (Word(1) SHL AdrType)=0) THEN
    BEGIN
     WrError(1350); AdrType:=ModNone; AdrCnt:=0;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_TMS7;
	Far;
VAR
   z,AdrInt:Integer;
   Bit:LongInt;
   OK,Rela:Boolean;

	PROCEDURE PutCode(Code:Word);
BEGIN
   IF Hi(Code)=0 THEN
    BEGIN
     CodeLen:=1; BAsmCode[0]:=Code;
    END
   ELSE
    BEGIN
     CodeLen:=2; BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
    END;
END;

BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=0;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(True) THEN Exit;

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE PutCode(Code);
       Exit;
      END;

   IF (Memo('MOV')) OR (Memo('MOVP')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('MOVP') THEN
        DecodeAdr(ArgStr[2],MModPort+MModAccA+MModAccB)
       ELSE
        DecodeAdr(ArgStr[2],MModAccB+MModReg+MModPort+MModAbs+MmodIReg+MModBRel
                           +MModAccA);
       CASE AdrType OF
       ModAccA:
	BEGIN
         IF Memo('MOVP') THEN
          DecodeAdr(ArgStr[1],MModPort)
         ELSE
          DecodeAdr(ArgStr[1],MModReg+MModAbs+MModIReg+MModBRel
                             +MModAccB+MModPort+MModImm);
         CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$12; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModAbs:
	  BEGIN
	   BAsmCode[0]:=$8a; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
	  END;
	 ModIReg:
	  BEGIN
	   BAsmCode[0]:=$9a; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModBRel:
	  BEGIN
	   BAsmCode[0]:=$aa; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$62; CodeLen:=1;
	  END;
	 ModPort:
	  BEGIN
	   BAsmCode[0]:=$80; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$22; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	END;
       ModAccB:
	BEGIN
         IF Memo('MOVP') THEN
          DecodeAdr(ArgStr[1],MModPort)
         ELSE
          DecodeAdr(ArgStr[1],MModAccA+MModReg+MModPort+MModImm);
         CASE AdrType OF
	 ModAccA:
	  BEGIN
	   BAsmCode[0]:=$c0; CodeLen:=1;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$32; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModPort:
	  BEGIN
	   BAsmCode[0]:=$91; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$52; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	END;
       ModReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModAccA+MModAccB+MModReg+MModPort+MModImm);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   BAsmCode[0]:=$d0; CodeLen:=2;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$d1; CodeLen:=2;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$42; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 ModPort:
	  BEGIN
	   BAsmCode[0]:=$a2; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$72; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 END;
	END;
       ModPort:
	BEGIN
	 BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=AdrVals[0];
         DecodeAdr(ArgStr[1],MModAccA+MModAccB+MModImm);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
           BAsmCode[0]:=$82; CodeLen:=2;
	  END;
	 ModAccB:
	  BEGIN
           BAsmCode[0]:=$92; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
           BAsmCode[0]:=$a2; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 END;
	END;
       ModAbs:
	BEGIN
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 DecodeAdr(ArgStr[1],MModAccA);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$8b; CodeLen:=3;
	  END;
	END;
       ModIReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModAccA);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$9b; CodeLen:=2;
	  END;
	END;
       ModBRel:
	BEGIN
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 DecodeAdr(ArgStr[1],MModAccA);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$ab; CodeLen:=3;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('LDA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAbs+MModBRel+MModIReg);
       CASE AdrType OF
       ModAbs:
        BEGIN
         BAsmCode[0]:=$8a; Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
        END;
       ModBRel:
        BEGIN
         BAsmCode[0]:=$aa; Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
        END;
       ModIReg:
        BEGIN
         BAsmCode[0]:=$9a; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('STA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAbs+MModBRel+MModIReg);
       CASE AdrType OF
       ModAbs:
        BEGIN
         BAsmCode[0]:=$8b; Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
        END;
       ModBRel:
        BEGIN
         BAsmCode[0]:=$ab; Move(AdrVals,BAsmCode[1],AdrCnt); CodeLen:=1+AdrCnt;
        END;
       ModIReg:
        BEGIN
         BAsmCode[0]:=$9b; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('MOVW')) OR (Memo('MOVD')) THEN
    BEGIN
     OpSize:=1;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrType<>ModNone THEN
	BEGIN
         z:=AdrVals[0];
         DecodeAdr(ArgStr[1],MModReg+MModImm+MModImmBRel);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$98; BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=z;
	   CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$88; Move(AdrVals,BAsmCode[1],2);
	   BAsmCode[3]:=z;
	   CodeLen:=4;
	  END;
	 ModImmBRel:
	  BEGIN
	   BAsmCode[0]:=$a8; Move(AdrVals,BAsmCode[1],2);
	   BAsmCode[3]:=z;
	   CodeLen:=4;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   FOR z:=1 TO Rel8OrderCount DO
    WITH Rel8Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
         AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
	 IF OK THEN
	  IF (AdrInt>127) OR (AdrInt<-128) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=2;
	    BAsmCode[0]:=Code; BAsmCode[1]:=AdrInt AND $ff;
	   END;
	END;
       Exit;
      END;

   IF (Memo('CMP')) OR (Memo('CMPA')) THEN
    BEGIN
     IF ((Memo('CMP')) AND (ArgCnt<>2)) OR ((Memo('CMPA')) AND (ArgCnt<>1)) THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('CMPA') THEN AdrType:=ModAccA
       ELSE DecodeAdr(ArgStr[2],MModAccA+MModAccB+MModReg);
       CASE AdrType OF
       ModAccA:
	BEGIN
         DecodeAdr(ArgStr[1],MModAbs+MModIReg+MModBRel+MModAccB+MModReg+MModImm);
	 CASE AdrType OF
	 ModAbs:
	  BEGIN
	   BAsmCode[0]:=$8d; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
	  END;
	 ModIReg:
	  BEGIN
	   BAsmCode[0]:=$9d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModBRel:
	  BEGIN
	   BAsmCode[0]:=$ad; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$6d; CodeLen:=1;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$1d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$2d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	END;
       ModAccB:
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg+MModImm);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$3d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$5d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	END;
       ModReg:
	BEGIN
	 BAsmCode[2]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModReg+MModImm);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$4d; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$7d; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 END;
	END;
       END;
      END;
     Exit;
    END;

   FOR z:=1 TO ALU1OrderCount DO
    WITH ALU1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],MModAccA+MModAccB+MModReg);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   DecodeAdr(ArgStr[1],MModAccB+MModReg+MModImm);
	   CASE AdrType OF
	   ModAccB:
	    BEGIN
	     CodeLen:=1; BAsmCode[0]:=$60+Code;
	    END;
	   ModReg:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$10+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   ModImm:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$20+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   END;
	  END;
	 ModAccB:
	  BEGIN
	   DecodeAdr(ArgStr[1],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$30+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   ModImm:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$50+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   END;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[2]:=AdrVals[0];
	   DecodeAdr(ArgStr[1],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     CodeLen:=3; BAsmCode[0]:=$40+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   ModImm:
	    BEGIN
	     CodeLen:=3; BAsmCode[0]:=$70+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   END;
	  END;
	 END;
	END;
       Exit;
      END;

   FOR z:=1 TO ALU2OrderCount DO
    WITH ALU2Orders^[z] DO
     IF (Memo(Name)) OR (Memo(Name+'P')) THEN
      BEGIN
       Rela:=Copy(OpPart,1,3)='BTJ';
       IF ((Rela) AND (ArgCnt<>3))
       OR ((NOT Rela) AND (ArgCnt<>2)) THEN WrError(1110)
       ELSE
	BEGIN
         IF OpPart[Length(OpPart)]='P' THEN
          DecodeAdr(ArgStr[2],MModPort)
         ELSE
          DecodeAdr(ArgStr[2],MModAccA+MModAccB+MModReg+MModPort);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   DecodeAdr(ArgStr[1],MModAccB+MModReg+MModImm);
	   CASE AdrType OF
	   ModAccB:
	    BEGIN
	     BAsmCode[0]:=$60+Code; CodeLen:=1;
	    END;
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=$10+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=$20+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	    END;
	   END;
	  END;
	 ModAccB:
	  BEGIN
	   DecodeAdr(ArgStr[1],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=$30+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=$50+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	    END;
	   END;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[2]:=AdrVals[0];
	   DecodeAdr(ArgStr[1],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=$40+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=$70+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	    END;
	   END;
	  END;
	 ModPort:
	  BEGIN
	   BAsmCode[1]:=AdrVals[0];
	   DecodeAdr(ArgStr[1],MModAccA+MModAccB+MModImm);
	   CASE AdrType OF
	   ModAccA:
	    BEGIN
	     BAsmCode[0]:=$80+Code; CodeLen:=2;
	    END;
	   ModAccB:
	    BEGIN
	     BAsmCode[0]:=$90+Code; CodeLen:=2;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=$a0+Code; BAsmCode[2]:=BAsmCode[1];
	     BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	    END;
	   END;
	  END;
	 END;
	 IF (CodeLen<>0) AND (Rela) THEN
	  BEGIN
	   AdrInt:=EvalIntExpression(ArgStr[3],Int16,OK)-(EProgCounter+CodeLen+1);
	   IF NOT OK THEN CodeLen:=0
	   ELSE IF (AdrInt>127) OR (AdrInt<-128) THEN
	    BEGIN
	     WrError(1370); CodeLen:=0;
	    END
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=AdrInt AND $ff; Inc(CodeLen);
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO JmpOrderCount DO
    WITH JmpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
         DecodeAdr(ArgStr[1],MModAbs+MModIReg+MModBRel);
	 CASE AdrType OF
	 ModAbs:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$80+Code; Move(AdrVals,BAsmCode[1],2);
	  END;
	 ModIReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$90+Code; BAsmCode[1]:=AdrVals[0];
	  END;
	 ModBRel:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$a0+Code; Move(AdrVals,BAsmCode[1],2);
	  END;
	 END;
	END;
       Exit;
      END;

   FOR z:=1 TO ABRegOrderCount DO
    WITH ABRegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ((NOT Memo('DJNZ')) AND (ArgCnt<>1))
       OR ((    Memo('DJNZ')) AND (ArgCnt<>2)) THEN WrError(1110)
       ELSE IF (NLS_StrCaseCmp(ArgStr[1],'ST')=0) THEN
	BEGIN
	 IF (Memo('PUSH')) OR (Memo('POP')) THEN
	  BEGIN
           BAsmCode[0]:=8+(Ord(Memo('PUSH'))*6); CodeLen:=1;
	  END
	 ELSE WrError(1350);
	END
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModAccA+MModAccB+MModReg);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   BAsmCode[0]:=$b0+Code; CodeLen:=1;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$c0+Code; CodeLen:=1;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$d0+Code; BAsmCode[CodeLen+1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	 IF (Memo('DJNZ')) AND (CodeLen<>0) THEN
	  BEGIN
	   AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK)-(EProgCounter+CodeLen+1);
	   IF NOT OK THEN CodeLen:=0
	   ELSE IF (AdrInt>127) OR (AdrInt<-128) THEN
	    BEGIN
	     WrError(1370); CodeLen:=0;
	    END
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=AdrInt AND $ff; Inc(CodeLen);
	    END;
	  END;
	END;
       Exit;
      END;

   IF Memo('TRAP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       BAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt5,OK);
       IF FirstPassUnknown THEN BAsmCode[0]:=BAsmCode[0] AND 15;
       IF OK THEN
        IF BAsmCode[0]>23 THEN WrError(1320)
        ELSE
         BEGIN
          BAsmCode[0]:=$ff-BAsmCode[0]; CodeLen:=1;
         END;
      END;
     Exit;
    END;

   IF Memo('TST') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAccA+MModAccB);
       CASE AdrType OF
       ModAccA:
	BEGIN
	 BAsmCode[0]:=$b0; CodeLen:=1;
	END;
       ModAccB:
	BEGIN
         BAsmCode[0]:=$c1; CodeLen:=1;
	END;
       END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_TMS7:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter < $10000;
   ELSE ok:=False;
   END;
   ChkPC_TMS7:=(ok) AND (ProgCounter>=0);
END;

        FUNCTION IsDef_TMS7:Boolean;
	Far;
BEGIN
   IsDef_TMS7:=False;
END;

        PROCEDURE InternSymbol_TMS7(VAR Asc:String; VAR Erg:TempResult);
        Far;
VAR
   h:String;
   err:ValErgType;
BEGIN
   Erg.Typ:=TempNone;
   IF (Length(Asc)<2) OR ((UpCase(Asc[1])<>'R') AND (UpCase(Asc[1])<>'P')) THEN Exit;

   h:=Copy(Asc,2,Length(Asc)-1);
   IF (h[1]='0') AND (Length(h)>1) THEN h[1]:='$';
   Val(h,Erg.Int,err);
   IF (Err<>0) OR (Erg.Int<0) OR (Erg.Int>255) THEN Exit;

   Erg.Typ:=TempInt; IF UpCase(Asc[1])='P' THEN Inc(Erg.Int,$100);
END;

        PROCEDURE SwitchFrom_TMS7;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_TMS7;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$73; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode ]:=1; ListGrans[SegCode ]:=1; SegInits[SegCode ]:=0;

   MakeCode:=MakeCode_TMS7; ChkPC:=ChkPC_TMS7; IsDef:=IsDef_TMS7;
   SwitchFrom:=SwitchFrom_TMS7; InternSymbol:=InternSymbol_TMS7;

   InitFields;
END;

BEGIN
   CPU70C00 :=AddCPU('TMS70C00', SwitchTo_TMS7);
   CPU70C20 :=AddCPU('TMS70C20', SwitchTo_TMS7);
   CPU70C40 :=AddCPU('TMS70C40', SwitchTo_TMS7);
   CPU70CT20:=AddCPU('TMS70CT20',SwitchTo_TMS7);
   CPU70CT40:=AddCPU('TMS70CT40',SwitchTo_TMS7);
   CPU70C02 :=AddCPU('TMS70C02', SwitchTo_TMS7);
   CPU70C42 :=AddCPU('TMS70C42', SwitchTo_TMS7);
   CPU70C82 :=AddCPU('TMS70C82', SwitchTo_TMS7);
   CPU70C08 :=AddCPU('TMS70C08', SwitchTo_TMS7);
   CPU70C48 :=AddCPU('TMS70C48', SwitchTo_TMS7);
END.
