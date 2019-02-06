{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

	UNIT Code56K;

INTERFACE
        Uses NLS,
             Chunks,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[7];
	       Code:LongInt;
	      END;

   ParOrder=RECORD
	     Name:String[4];
	     Typ:(ParAB,ParXYAB,ParABXYnAB,ParABBA,ParXYnAB,ParMul);
	     Code:Byte;
	    END;

CONST
   FixedOrderCnt=9;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'NOP'    ; Code:$000000),
		(Name:'ENDDO'  ; Code:$00008c),
		(Name:'ILLEGAL'; Code:$000005),
		(Name:'RESET'  ; Code:$000084),
		(Name:'RTI'    ; Code:$000004),
		(Name:'RTS'    ; Code:$00000c),
		(Name:'STOP'   ; Code:$000087),
		(Name:'SWI'    ; Code:$000006),
		(Name:'WAIT'   ; Code:$000086));

   ParOrderCnt=29;
   ParOrders:ARRAY[1..ParOrderCnt] OF ParOrder=
	     ((Name:'ABS' ; Typ:ParAB;     Code:$26),
	      (Name:'ASL' ; Typ:ParAB;     Code:$32),
	      (Name:'ASR' ; Typ:ParAB;     Code:$22),
	      (Name:'CLR' ; Typ:ParAB;     Code:$13),
	      (Name:'LSL' ; Typ:ParAB;     Code:$33),
	      (Name:'LSR' ; Typ:ParAB;     Code:$23),
	      (Name:'NEG' ; Typ:ParAB;     Code:$36),
	      (Name:'NOT' ; Typ:ParAB;     Code:$17),
	      (Name:'RND' ; Typ:ParAB;     Code:$11),
	      (Name:'ROL' ; Typ:ParAB;     Code:$37),
	      (Name:'ROR' ; Typ:ParAB;     Code:$27),
	      (Name:'TST' ; Typ:ParAB;     Code:$03),
	      (Name:'ADC' ; Typ:ParXYAB;   Code:$21),
	      (Name:'SBC' ; Typ:ParXYAB;   Code:$25),
	      (Name:'ADD' ; Typ:ParABXYnAB;Code:$00),
	      (Name:'CMP' ; Typ:ParABXYnAB;Code:$05),
	      (Name:'CMPM'; Typ:ParABXYnAB;Code:$07),
	      (Name:'SUB' ; Typ:ParABXYnAB;Code:$04),
	      (Name:'ADDL'; Typ:ParABBA;   Code:$12),
	      (Name:'ADDR'; Typ:ParABBA;   Code:$02),
	      (Name:'SUBL'; Typ:ParABBA;   Code:$16),
	      (Name:'SUBR'; Typ:ParABBA;   Code:$06),
	      (Name:'AND' ; Typ:ParXYnAB;  Code:$46),
	      (Name:'EOR' ; Typ:ParXYnAB;  Code:$43),
	      (Name:'OR'  ; Typ:ParXYnAB;  Code:$42),
	      (Name:'MAC' ; Typ:ParMul;    Code:$82),
	      (Name:'MACR'; Typ:ParMul;    Code:$83),
	      (Name:'MPY' ; Typ:ParMul;    Code:$80),
	      (Name:'MPYR'; Typ:ParMul;    Code:$81));

   BitOrderCnt=4;
   BitOrders:ARRAY[0..BitOrderCnt-1] OF String[4]=('BCLR','BSET','BCHG','BTST');

   BitJmpOrderCnt=4;
   BitJmpOrders:ARRAY[0..BitOrderCnt-1] OF String[5]=('JCLR','JSET','JSCLR','JSSET');

   MacTable:ARRAY[4..7,4..7] OF Byte=((0,2,5,4),(2,$ff,6,7),(5,6,1,3),(4,7,3,$ff));

   ModNone=-1;
   ModImm=0;       MModImm=1 SHL ModImm;
   ModAbs=1;       MModAbs=1 SHL ModAbs;
   ModIReg=2;      MModIReg=1 SHL ModIReg;
   ModPreDec=3;    MModPreDec=1 SHL ModPreDec;
   ModPostDec=4;   MModPostDec=1 SHL ModPostDec;
   ModPostInc=5;   MModPostInc=1 SHL ModPostInc;
   ModIndex=6;     MModIndex=1 SHL ModIndex;
   ModModDec=7;    MModModDec=1 SHL ModModDec;
   ModModInc=8;    MModModInc=1 SHL ModModInc;

   MModNoExt=MModIReg+MModPreDec+MModPostDec+MModPostInc+MModIndex+
	     MModModDec+MModModInc;
   MModNoImm=MModAbs+MModNoExt;
   MModAll=MModNoImm+MModImm;

   SegLData=SegYData+1;

   MSegCode=1 SHL SegCode;
   MSegXData=1 SHL SegXData;
   MSegYData=1 SHL SegYData;
   MSegLData=1 SHL SegLData;

VAR
   CPU56000:CPUVar;

   TargSegment:Byte;
   AdrType:ShortInt;
   AdrMode:Word;
   AdrVal:LongInt;
   AdrCnt:Byte;
   AdrSeg:Byte;


	PROCEDURE SplitArg(VAR Orig,LDest,RDest:String);
VAR
   p:Integer;
BEGIN
   p:=QuotPos(Orig,',');
   RDest:=Copy(Orig,p+1,Length(Orig)-p);
   LDest:=Copy(Orig,1,p-1);
END;

	PROCEDURE DecodeAdr(Asc:String; Erl:Word; ErlSeg:Byte);
LABEL
   Found;
CONST
   ModMasks:ARRAY[ModIReg..ModModInc] OF String[7]=
	    ('(Rx)','-(Rx)','(Rx)-','(Rx)+','(Rx+Nx)','(Rx)-Nx','(Rx)+Nx');
   ModCodes:ARRAY[ModIReg..ModModInc] OF Byte=
	    (4,7,2,3,5,0,1);
   SegCount=4;
   SegNames:ARRAY[1..SegCount] OF Char='PXYL';
   SegVals:ARRAY[1..SegCount] OF Byte=(SegCode,SegXData,SegYData,SegLData);
VAR
   z,l:Integer;
   OK,BreakLoop:Boolean;
   OrdVal:Byte;
BEGIN
   AdrType:=ModNone; AdrCnt:=0;

   { Defaultsegment herausfinden }

   IF ErlSeg AND MSegXData<>0 THEN AdrSeg:=SegXData
   ELSE IF ErlSeg AND MSegYData<>0 THEN AdrSeg:=SegYData
   ELSE IF ErlSeg AND MSegCode<>0 THEN AdrSeg:=SegCode
   ELSE AdrSeg:=SegNone;

   { Zielsegment vorgegeben ? }

   FOR z:=1 TO SegCount DO
    IF (UpCase(Asc[1])=SegNames[z]) AND (Asc[2]=':') THEN
     BEGIN
      AdrSeg:=SegVals[z]; Delete(Asc,1,2);
     END;

   { Adre·ausdrÅcke abklopfen: dazu mit Referenzstring vergleichen }

   FOR z:=ModIReg TO ModModInc DO
    IF Length(Asc)=Length(ModMasks[z]) THEN
     BEGIN
      AdrMode:=$ffff; BreakLoop:=False;
      FOR l:=1 TO Length(Asc) DO
       IF NOT BreakLoop THEN
	IF ModMasks[z][l]='x' THEN
	 BEGIN
	  OrdVal:=Ord(Asc[l])-AscOfs;
	  IF OrdVal>7 THEN BreakLoop:=True
	  ELSE IF AdrMode=$ffff THEN AdrMode:=OrdVal
	  ELSE IF AdrMode<>OrdVal THEN
	   BEGIN
	    BreakLoop:=True; WrError(1760); Goto Found;
	   END;
	 END
        ELSE BreakLoop:=ModMasks[z][l]<>UpCase(Asc[l]);
      IF NOT BreakLoop THEN
       BEGIN
	AdrType:=z; Inc(AdrMode,ModCodes[z] SHL 3);
	Goto Found;
       END;
     END;

   { immediate ? }

   IF Asc[1]='#' THEN
    BEGIN
     AdrVal:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),Int24,OK);
     IF OK THEN
      BEGIN
       AdrType:=ModImm; AdrCnt:=1; AdrMode:=$34; Goto Found;
      END;
    END;

   { dann absolut }

   AdrVal:=EvalIntExpression(Asc,Int16,OK);
   IF OK THEN
    BEGIN
     AdrType:=ModAbs; AdrMode:=$30; AdrCnt:=1;
     IF (AdrSeg IN [SegCode,SegXData,SegYData]) THEN ChkSpace(AdrSeg);
     Goto Found;
    END;

Found:
   IF (AdrType<>ModNone) AND (Erl AND (1 SHL AdrType)=0) THEN
    BEGIN
     WrError(1350); AdrCnt:=0; AdrType:=ModNone;
    END;
   IF (AdrSeg<>SegNone) AND (ErlSeg AND (1 SHL AdrSeg)=0) THEN
    BEGIN
     WrError(1960); AdrCnt:=0; AdrType:=ModNone;
    END;
END;

	FUNCTION DecodeReg(Asc:String; VAR Erg:LongInt):Boolean;
CONST
   RegCount=12;
   RegNames:ARRAY[1..RegCount] OF String[2]=
	    ('X0','X1','Y0','Y1','A0','B0','A2','B2','A1','B1','A','B');
VAR
   z:Word;
BEGIN
   DecodeReg:=False;
   FOR z:=1 TO RegCount DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      Erg:=z+3; DecodeReg:=True; Exit;
     END;
   IF (Length(Asc)=2) AND (Asc[2]>='0') AND (Asc[2]<='7') THEN
    CASE UpCase(Asc[1]) OF
    'R':BEGIN
	 Erg:=16+Ord(Asc[2])-AscOfs; DecodeReg:=True;
	END;
    'N':BEGIN
	 Erg:=24+Ord(Asc[2])-AscOfs; DecodeReg:=True;
	END;
    END;
END;


	FUNCTION DecodeALUReg(Asc:String; VAR Erg:LongInt; MayX,MayY,MayAcc:Boolean):Boolean;
BEGIN
   DecodeALUReg:=False; IF NOT DecodeReg(Asc,Erg) THEN Exit;
   CASE Erg OF
   4,5:IF MayX THEN
	BEGIN
	 DecodeALUReg:=True; Dec(Erg,4);
	END;
   6,7:IF MayY THEN
	BEGIN
	 DecodeALUReg:=True; Dec(Erg,6);
	END;
   14,15:IF MayAcc THEN
	  BEGIN
	   DecodeALUReg:=True;
	   IF (MayX) OR (MayY) THEN Dec(Erg,12) ELSE Dec(Erg,14);
	  END;
   END;
END;

	FUNCTION DecodeLReg(Asc:String; VAR Erg:LongInt):Boolean;
CONST
   RegCount=8;
   RegNames:ARRAY[0..RegCount-1] OF String[3]=
	    ('A10','B10','X','Y','A','B','AB','BA');
VAR
   z:Word;
BEGIN
   DecodeLReg:=False;
   FOR z:=0 TO RegCount-1 DO
    IF NLS_StrCaseCmp(Asc,RegNames[z] )=0 THEN
     BEGIN
      Erg:=z; DecodeLReg:=True; Exit;
     END;
END;

	FUNCTION DecodeXYABReg(Asc:String; VAR Erg:LongInt):Boolean;
CONST
   RegCount=8;
   RegNames:ARRAY[0..RegCount-1] OF String[2]=
	    ('B','A','X','Y','X0','Y0','X1','Y1');
VAR
   z:Word;
BEGIN
   DecodeXYABReg:=False;
   FOR z:=0 TO RegCount-1 DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      Erg:=z; DecodeXYABReg:=True; Exit;
     END;
END;

	FUNCTION DecodePCReg(Asc:String; VAR Erg:LongInt):Boolean;
CONST
   RegCount=7;
   RegNames:ARRAY[1..RegCount] OF String[3]=
	    ('SR','OMR','SP','SSH','SSL','LA','LC');
VAR
   z:Word;
BEGIN
   DecodePCReg:=False;
   FOR z:=1 TO RegCount DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      Erg:=z; DecodePCReg:=True; Exit;
     END;
END;

	FUNCTION DecodeGeneralReg(Asc:String; VAR Erg:LongInt):Boolean;
BEGIN
   DecodeGeneralReg:=True;
   IF DecodeReg(Asc,Erg) THEN Exit;
   IF DecodePCReg(Asc,Erg) THEN
    BEGIN
     Inc(Erg,$38); Exit;
    END;
   IF (Length(Asc)=2) AND (UpCase(Asc[1])='M') AND (Asc[2]>='0') AND (Asc[2]<='7') THEN
    BEGIN
     Erg:=$20+Ord(Asc[2])-AscOfs; Exit;
    END;
   DecodeGeneralReg:=False;
END;

	FUNCTION DecodeControlReg(Asc:String; VAR Erg:LongInt):Boolean;
BEGIN
   DecodeControlReg:=True;
   IF NLS_StrCaseCmp(Asc,'MR')=0 THEN Erg:=0
   ELSE IF NLS_StrCaseCmp(Asc,'CCR')=0 THEN Erg:=1
   ELSE IF NLS_StrCaseCmp(Asc,'OMR')=0 THEN Erg:=2
   ELSE DecodeControlReg:=False;
END;

	FUNCTION DecodeOpPair(Left,Right:String; WorkSeg:Byte;
			      VAR Dir,Reg1,Reg2:LongInt;
			      VAR AType,AMode,ACnt,AVal:LongInt):Boolean;
BEGIN
   DecodeOpPair:=False;

   IF DecodeALUReg(Left,Reg1,WorkSeg=SegXData,WorkSeg=SegYData,True) THEN
    BEGIN
     IF DecodeALUReg(Right,Reg2,WorkSeg=SegXData,WorkSeg=SegYData,True) THEN
      BEGIN
       Dir:=2; DecodeOpPair:=True;
      END
     ELSE
      BEGIN
       Dir:=0; Reg2:=-1;
       DecodeAdr(Right,MModNoImm,1 SHL WorkSeg);
       IF AdrType<>ModNone THEN
	BEGIN
	 AType:=AdrType; AMode:=AdrMode; ACnt:=AdrCnt; AVal:=AdrVal;
	 DecodeOpPair:=True;
	END;
      END;
    END
   ELSE IF DecodeALUReg(Right,Reg1,WorkSeg=SegXData,WorkSeg=SegYData,True) THEN
    BEGIN
     Dir:=1; Reg2:=-1;
     DecodeAdr(Left,MModAll,1 SHL WorkSeg);
     IF AdrType<>ModNone THEN
      BEGIN
       AType:=AdrType; AMode:=AdrMode; ACnt:=AdrCnt; AVal:=AdrVal;
       DecodeOpPair:=True;
      END;
    END;
END;

	FUNCTION TurnXY(Inp:LongInt):LongInt;
BEGIN
   CASE Inp OF
   4,7:TurnXY:=Inp-4;
   5,6:TurnXY:=7-Inp;
   END;
END;

	FUNCTION DecodeTFR(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   Part1,Part2:LongInt;
   Left,Right:String;
BEGIN
   DecodeTFR:=False;
   SplitArg(Asc,Left,Right);
   IF NOT DecodeALUReg(Right,Part2,False,False,True) THEN Exit;
   IF NOT DecodeReg(Left,Part1) THEN Exit;
   IF NOT Part1 IN [4..7,14,15] THEN Exit;
   IF Part1>13 THEN
    IF (Part1 XOR Part2) AND 1=0 THEN Exit
    ELSE Part1:=0
   ELSE Part1:=TurnXY(Part1)+4;
   Erg:=(Part1 SHL 1)+Part2;
   DecodeTFR:=True;
END;

	FUNCTION DecodeMOVE(Start:Integer):Boolean;

	FUNCTION DecodeMOVE_0:Boolean;
BEGIN
   DecodeMOVE_0:=True; DAsmCode[0]:=$200000; CodeLen:=1;
END;

	FUNCTION DecodeMOVE_1:Boolean;
VAR
   Left,Right:String;
   RegErg,RegErg2,IsY,MixErg:LongInt;
BEGIN
   SplitArg(ArgStr[Start],Left,Right); DecodeMOVE_1:=False;

   { 1. Register-Update }

   IF Right='' THEN
    BEGIN
     DecodeAdr(Left,MModPostDec+MModPostInc+MModModDec+MModModInc,0);
     IF AdrType<>ModNone THEN
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$204000+(AdrMode SHL 8);
       CodeLen:=1;
      END;
     Exit;
    END;

   { 2. Ziel ist Register }

   IF DecodeReg(Right,RegErg) THEN
    BEGIN
     IF DecodeReg(Left,RegErg2) THEN
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$200000+(RegErg SHL 8)+(RegErg2 SHL 13);
       CodeLen:=1;
      END
     ELSE
      BEGIN
       DecodeAdr(Left,MModAll,MSegXData+MSegYData);
       IsY:=Ord(AdrSeg=SegYData);
       MixErg:=((RegErg AND $18) SHL 17)+(IsY SHL 19)+((RegErg AND 7) SHL 16);
       IF (AdrType=ModImm) AND (AdrVal AND $ffffff00=0) THEN
	BEGIN
	 DecodeMOVE_1:=True;
	 DAsmCode[0]:=$200000+(RegErg SHL 16)+((AdrVal AND $ff) SHL 8);
	 CodeLen:=1;
	END
       ELSE IF (AdrType=ModAbs) AND (AdrVal<=63) AND (AdrVal>=0) THEN
	BEGIN
	 DecodeMOVE_1:=True;
	 DAsmCode[0]:=$408000+MixErg+(AdrVal SHL 8);
	 CodeLen:=1;
	END
       ELSE IF AdrType<>ModNone THEN
	BEGIN
	 DecodeMOVE_1:=True;
	 DAsmCode[0]:=$40c000+MixErg+(AdrMode SHL 8); DAsmCode[1]:=AdrVal;
	 CodeLen:=1+AdrCnt;
	END;
      END;
     Exit;
    END;

   { 3. Quelle ist Register }

   IF DecodeReg(Left,RegErg) THEN
    BEGIN
     DecodeAdr(Right,MModNoImm,MSegXData+MSegYData);
     IsY:=Ord(AdrSeg=SegYData);
     MixErg:=((RegErg AND $18) SHL 17)+(IsY SHL 19)+((RegErg AND 7) SHL 16);
     IF (AdrType=ModAbs) AND (AdrVal<=63) AND (AdrVal>=0) THEN
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$400000+MixErg+(AdrVal SHL 8);
       CodeLen:=1;
      END
     ELSE IF AdrType<>ModNone THEN
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$404000+MixErg+(AdrMode SHL 8); DAsmCode[1]:=AdrVal;
       CodeLen:=1+AdrCnt;
      END;
     Exit;
    END;

   { 4. Ziel ist langes Register }

   IF DecodeLReg(Right,RegErg) THEN
    BEGIN
     DecodeAdr(Left,MModNoImm,MSegLData);
     MixErg:=((RegErg AND 4) SHL 17)+((RegErg AND 3) SHL 16);
     IF (AdrType=ModAbs) AND (AdrVal<=63) AND (AdrVal>=0) THEN
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$408000+MixErg+(AdrVal SHL 8);
       CodeLen:=1;
      END
     ELSE
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$40c000+MixErg+(AdrMode SHL 8); DAsmCode[1]:=AdrVal;
       CodeLen:=1+AdrCnt;
      END;
     Exit;
    END;

   { 5. Quelle ist langes Register }

   IF DecodeLReg(Left,RegErg) THEN
    BEGIN
     DecodeAdr(Right,MModNoImm,MSegLData);
     MixErg:=((RegErg AND 4) SHL 17)+((RegErg AND 3) SHL 16);
     IF (AdrType=ModAbs) AND (AdrVal<=63) AND (AdrVal>=0) THEN
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$400000+MixErg+(AdrVal SHL 8);
       CodeLen:=1;
      END
     ELSE
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$404000+MixErg+(AdrMode SHL 8); DAsmCode[1]:=AdrVal;
       CodeLen:=1+AdrCnt;
      END;
     Exit;
    END;

   WrError(1350);
END;

	FUNCTION DecodeMOVE_2:Boolean;
VAR
   Left1,Right1,Left2,Right2:String;
   RegErg,Reg1L,Reg1R,Reg2L,Reg2R:LongInt;
   Mode1,Mode2,Dir1,Dir2,Type1,Type2,Cnt1,Cnt2,Val1,Val2:LongInt;
BEGIN
   SplitArg(ArgStr[Start],Left1,Right1);
   SplitArg(ArgStr[Start+1],Left2,Right2);
   DecodeMOVE_2:=False;

   { 1. Spezialfall X auf rechter Seite ? }

   IF NlS_StrCaseCmp(Left2,'X0')=0 THEN
    BEGIN
     IF NOT DecodeALUReg(Right2,RegErg,False,False,True) THEN WrError(1350)
     ELSE IF Left1<>Right2 THEN WrError(1350)
     ELSE
      BEGIN
       DecodeAdr(Right1,MModNoImm,MSegXData);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=1+AdrCnt;
	 DAsmCode[0]:=$080000+(RegErg SHL 16)+(AdrMode SHL 8);
	 DAsmCode[1]:=AdrVal;
	 DecodeMOVE_2:=True;
	END;
      END;
     Exit;
    END;

   { 2. Spezialfall Y auf linker Seite ? }

   IF NLS_StrCaseCmp(Left1,'Y0')=0 THEN
    BEGIN
     IF NOT DecodeALUReg(Right1,RegErg,False,False,True) THEN WrError(1350)
     ELSE IF Left2<>Right1 THEN WrError(1350)
     ELSE
      BEGIN
       DecodeAdr(Right2,MModNoImm,MSegYData);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=1+AdrCnt;
	 DAsmCode[0]:=$088000+(RegErg SHL 16)+(AdrMode SHL 8);
	 DAsmCode[1]:=AdrVal;
	 DecodeMOVE_2:=True;
	END;
      END;
     Exit;
    END;

   { der Rest..... }

   IF (DecodeOpPair(Left1,Right1,SegXData,Dir1,Reg1L,Reg1R,Type1,Mode1,Cnt1,Val1)
   AND DecodeOpPair(Left2,Right2,SegYData,Dir2,Reg2L,Reg2R,Type2,Mode2,Cnt2,Val2)) THEN
    BEGIN
     IF (Reg1R=-1) AND (Reg2R=-1) THEN
      BEGIN
       IF (Mode1 SHR 3<1) OR (Mode1 SHR 3>4) OR (Mode2 SHR 3<1) OR (Mode2 SHR 3>4) THEN WrError(1350)
       ELSE IF ((Mode1 XOR Mode2) AND 4)=0 THEN WrError(1760)
       ELSE
	BEGIN
	 DAsmCode[0]:=$800000+(Dir2 SHL 22)+(Dir1 SHL 15)+
		      (Reg1L SHL 18)+(Reg2L SHL 16)+((Mode1 AND $1f) SHL 8)+
		      ((Mode2 AND 3) SHL 13)+((Mode2 AND 24) SHL 17);
	 CodeLen:=1; DecodeMOVE_2:=True;
	END;
      END
     ELSE IF Reg1R=-1 THEN
      BEGIN
       IF (Reg2L<2) OR (Reg2R>1) THEN WrError(1350)
       ELSE
	BEGIN
	 DAsmCode[0]:=$100000+(Reg1L SHL 18)+((Reg2L-2) SHL 17)+(Reg2R SHL 16)+
		      (Dir1 SHL 15)+(Mode1 SHL 8);
	 DAsmCode[1]:=Val1; CodeLen:=1+Cnt1; DecodeMOVE_2:=True;
	END;
      END
     ELSE IF Reg2R=-1 THEN
      BEGIN
       IF (Reg1L<2) OR (Reg1R>1) THEN WrError(1350)
       ELSE
	BEGIN
	 DAsmCode[0]:=$104000+(Reg2L SHL 16)+((Reg1L-2) SHL 19)+(Reg1R SHL 18)+
		      (Dir2 SHL 15)+(Mode2 SHL 8);
	 DAsmCode[1]:=Val2; CodeLen:=1+Cnt2; DecodeMOVE_2:=True;
	END;
      END
     ELSE WrError(1350);
     Exit;
    END;

   WrError(1350);
END;

BEGIN
   CASE ArgCnt-Start+1 OF
   0:DecodeMOVE:=DecodeMove_0;
   1:DecodeMOVE:=DecodeMove_1;
   2:DecodeMOVE:=DecodeMove_2;
   ELSE
    BEGIN
     WrError(1110); DecodeMOVE:=False;
    END;
   END;
END;

	FUNCTION DecodeCondition(Asc:String; VAR Erg:Word):Boolean;
CONST
   CondCount=18;
   CondNames:ARRAY[0..CondCount-1] OF String[2]=
	     ('CC','GE','NE','PL','NN','EC','LC','GT','CS','LT','EQ','MI',
	      'NR','ES','LS','LE','HS','LO');
BEGIN
   Erg:=0;
   WHILE (Erg<CondCount) AND (NLS_StrCaseCmp(CondNames[Erg],Asc)<>0) DO Inc(Erg);
   IF Erg=CondCount-1 THEN Erg:=8;
   DecodeCondition:=Erg<CondCount;
   Erg:=Erg AND 15;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   OK:Boolean;
   BCount:Integer;
   AdrWord,z,z2:Word;
   Segment:Byte;
   t:TempResult;
   HInt:LongInt;
BEGIN
   DecodePseudo:=True;

   IF Memo('XSFR') THEN
    BEGIN
     CodeEquate(SegXData,0,$ffff);
     Exit;
    END;

   IF Memo('YSFR') THEN
    BEGIN
     CodeEquate(SegYData,0,$ffff);
     Exit;
    END;

{   IF (Memo('XSFR')) OR (Memo('YSFR')) THEN
    BEGIN
     FirstPassUnknown:=False;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF (OK) AND (NOT FirstPassUnknown) THEN
	BEGIN
	 IF Memo('YSFR') THEN Segment:=SegYData ELSE Segment:=SegXData;
         PushLocHandle(-1);
	 EnterIntSymbol(LabPart,AdrWord,Segment,False);
         PopLocHandle;
	 IF MakeUseList THEN AddChunk(SegChunks[Segment],AdrWord,1,False);
	 ListLine:='='+'$'+HexString(AdrWord,4);
	END;
      END;
     Exit;
    END;}

   IF Memo('DS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (OK) AND (NOT FirstPassUnknown) THEN
	BEGIN
	 CodeLen:=AdrWord; DontPrint:=True;
         IF MakeUseList THEN
      	  IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
	END;
      END;
     Exit;
    END;

   IF Memo('DC') THEN
    BEGIN
     IF ArgCnt<1 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  FirstPassUnknown:=False;
	  EvalExpression(ArgStr[z],t);
	  CASE t.Typ OF
	  TempInt:
	   BEGIN
	    IF FirstPassUnknown THEN t.Int:=t.Int AND $ffffff;
	    IF NOT RangeCheck(t.Int,Int24) THEN
	     BEGIN
	      WrError(1320); OK:=False;
	     END
	    ELSE
	     BEGIN
              DAsmCode[CodeLen]:=t.Int AND $ffffff; Inc(CodeLen);
	     END
	   END;
	  TempString:
	   BEGIN
	    BCount:=2; DAsmCode[CodeLen]:=0;
	    FOR z2:=1 TO Length(t.Ascii) DO
	     BEGIN
              HInt:=Ord(CharTransTable[t.Ascii[z2]]);
              HInt:=HInt SHL (BCount*8);
              DAsmCode[CodeLen]:=DAsmCode[CodeLen] OR HInt;
	      Dec(BCount);
	      IF BCount<0 THEN
	       BEGIN
		BCount:=2; Inc(CodeLen); DAsmCode[CodeLen]:=0;
	       END;
	     END;
	    IF BCount<>2 THEN Inc(CodeLen);
	   END;
	  ELSE
	   BEGIN
	    WrError(1135);
	   END;
	  END;
	 END;
      END;
     IF NOT OK THEN CodeLen:=0;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_56K;
	Far;
VAR
   z:Integer;
   AddVal,h,Reg1,Reg2,Reg3:LongInt;
   HVal,HCnt,HMode,HType,HSeg:LongInt;
   Condition:Word;
   OK:Boolean;
   Left,Mid,Right:String;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
	BEGIN
	 CodeLen:=1; DAsmCode[0]:=Code;
	END;
       Exit;
      END;

   { ALU }

   FOR z:=1 TO ParOrderCnt DO
    WITH ParOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF DecodeMOVE(2) THEN
	BEGIN
	 OK:=True;
	 CASE Typ OF
	 ParAB:
	  BEGIN
	   IF DecodeALUReg(ArgStr[1],Reg1,False,False,True) THEN h:=Reg1 SHL 3
	   ELSE OK:=False;
	  END;
	 ParXYAB:
	  BEGIN
	   SplitArg(ArgStr[1],Left,Right);
	   IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN OK:=False
	   ELSE IF NOT DecodeLReg(Left,Reg1) THEN OK:=False
	   ELSE IF (Reg1<2) OR (Reg1>3) THEN OK:=False
	   ELSE h:=(Reg2 SHL 3)+((Reg1-2) SHL 4);
	  END;
	 ParABXYnAB:
	  BEGIN
	   SplitArg(ArgStr[1],Left,Right);
	   IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN OK:=False
	   ELSE IF NOT DecodeXYABReg(Left,Reg1) THEN OK:=False
	   ELSE IF Reg1 XOR Reg2=1 THEN OK:=False
	   ELSE
	    BEGIN
	     IF Reg1=0 THEN Reg1:=1; h:=(Reg2 SHL 3)+(Reg1 SHL 4);
	    END;
	  END;
	 ParABBA:
          IF NLS_StrCaseCmp(ArgStr[1],'B,A')=0 THEN h:=0
          ELSE IF NLS_StrCaseCmp(ArgStr[1],'A,B')=0 THEN h:=8
	  ELSE OK:=False;
	 ParXYnAB:
	  BEGIN
	   SplitArg(ArgStr[1],Left,Right);
	   IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN OK:=False
	   ELSE IF NOT DecodeReg(Left,Reg1) THEN OK:=False
	   ELSE IF (Reg1<4) OR (Reg1>7) THEN OK:=False
	   ELSE h:=(Reg2 SHL 3)+(TurnXY(Reg1) SHL 4);
	  END;
	 ParMul:
	  BEGIN
	   SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right); h:=0;
	   IF Left[1]='-' THEN
	    BEGIN
	     Delete(Left,1,1); Inc(h,4);
	    END
	   ELSE IF Left[1]='+' THEN Delete(Left,1,1);
	   IF NOT DecodeALUReg(Right,Reg3,False,False,True) THEN OK:=False
	   ELSE IF NOT DecodeReg(Left,Reg1) THEN OK:=False
	   ELSE IF (Reg1<4) OR (Reg1>7) THEN OK:=False
	   ELSE IF NOT DecodeReg(Mid,Reg2) THEN OK:=False
	   ELSE IF (Reg2<4) OR (Reg2>7) THEN OK:=False
	   ELSE IF MacTable[Reg1,Reg2]=$ff THEN OK:=False
	   ELSE Inc(h,(Reg3 SHL 3)+(MacTable[Reg1,Reg2] SHL 4));
	  END;
	 END;
	 IF OK THEN Inc(DAsmCode[0],Code+h)
	 ELSE
	  BEGIN
	   WrError(1350); CodeLen:=0;
	  END;
	END;
       Exit;
      END;

   IF Memo('DIV') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN WrError(1350)
       ELSE IF NOT DecodeReg(Left,Reg1) THEN WrError(1350)
       ELSE IF (Reg1<4) OR (Reg1>7) THEN WrError(1350)
       ELSE
	BEGIN
	 CodeLen:=1; DAsmCode[0]:=$018040+(Reg2 SHL 3)+(TurnXY(Reg1) SHL 4);
	END;
      END;
     Exit;
    END;

   IF (Memo('ANDI')) OR (Memo('ORI')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeControlReg(Right,Reg1) THEN WrError(1350)
       ELSE IF Left[1]<>'#' THEN WrError(1120)
       ELSE
	BEGIN
	 h:=EvalIntExpression(Copy(Left,2,Length(Left)-1),Int8,OK);
	 IF OK THEN
	  BEGIN
	   CodeLen:=1;
	   DAsmCode[0]:=$0000b8+((h AND $ff) SHL 8)+(Ord(Memo('ORI')) SHL 6)+Reg1;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('NORM') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN WrError(1350)
       ELSE IF NOT DecodeReg(Left,Reg1) THEN WrError(1350)
       ELSE IF (Reg1<16) OR (Reg1>23) THEN WrError(1350)
       ELSE
	BEGIN
	 CodeLen:=1; DAsmCode[0]:=$01d815+((Reg1 AND 7) SHL 8)+(Reg2 SHL 3);
	END;
      END;
     Exit;
    END;

   FOR z:=0 TO BitOrderCnt-1 DO
    IF Memo(BitOrders[z]) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE
       BEGIN
	Reg2:=((z AND 1) SHL 5)+(LongInt(z SHR 1) SHL 16);
	SplitArg(ArgStr[1],Left,Right);
	IF (Left='') OR (Right='') THEN WrError(1110)
	ELSE IF Left[1]<>'#' THEN WrError(1120)
	ELSE
	 BEGIN
	  h:=EvalIntExpression(Copy(Left,2,Length(Left)-1),Int8,OK);
	  IF FirstPassUnknown THEN h:=h AND 15;
	  IF OK THEN
	   IF (h<0) OR (h>23) THEN WrError(1320)
	   ELSE IF DecodeGeneralReg(Right,Reg1) THEN
	    BEGIN
	     CodeLen:=1;
	     DAsmCode[0]:=$0ac040+h+(Reg1 SHL 8)+Reg2;
	    END
	   ELSE
	    BEGIN
	     DecodeAdr(Right,MModNoImm,MSegXData+MSegYData);
	     Reg3:=Ord(AdrSeg=SegYData) SHL 6;
	     IF (AdrType=ModAbs) AND (AdrVal<=63) AND (AdrVal>=0) THEN
	      BEGIN
	       CodeLen:=1;
	       DAsmCode[0]:=$0a0000+h+(AdrVal SHL 8)+Reg3+Reg2;
	      END
	     ELSE IF (AdrType=ModAbs) AND (AdrVal>=$ffc0) AND (AdrVal<=$ffff) THEN
	      BEGIN
	       CodeLen:=1;
	       DAsmCode[0]:=$0a8000+h+((AdrVal AND $3f) SHL 8)+Reg3+Reg2;
	      END
	     ELSE IF AdrType<>ModNone THEN
	      BEGIN
	       CodeLen:=1+AdrCnt;
	       DAsmCode[0]:=$0a4000+h+(AdrMode SHL 8)+Reg3+Reg2;
	       DAsmCode[1]:=AdrVal;
	      END;
	    END;
	 END;
       END;
      Exit;
     END;

   { Datentransfer }

   IF Memo('MOVE') THEN
    BEGIN
     IF DecodeMOVE(1) THEN;
     Exit;
    END;

   IF Memo('MOVEC') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF DecodeGeneralReg(Left,Reg1) THEN
	IF DecodeGeneralReg(Right,Reg2) THEN
	 IF Reg1>=$20 THEN                            { S1,D2 }
	  BEGIN
	   CodeLen:=1; DAsmCode[0]:=$044080+(Reg2 SHL 8)+Reg1;
	  END
	 ELSE IF Reg2>=$20 THEN                       { S2,D1 }
	  BEGIN
	   CodeLen:=1; DAsmCode[0]:=$04c080+(Reg1 SHL 8)+Reg2;
	  END
	 ELSE WrError(1350)
	ELSE IF Reg1<$20 THEN WrError(1350)
	ELSE                                          { S1,ea/aa }
	 BEGIN
	  DecodeAdr(Right,MModNoImm,MSegXData+MSegYData);
	  IF (AdrType=ModAbs) AND (AdrVal>=0) AND (AdrVal<=63) THEN
	   BEGIN
	    CodeLen:=1;
	    DAsmCode[0]:=$050000+(AdrVal SHL 8)+(Ord(AdrSeg=SegYData) SHL 6)+Reg1;
	   END
	  ELSE IF AdrType<>ModNone THEN
	   BEGIN
	    CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
	    DAsmCode[0]:=$054000+(AdrMode SHL 8)+(Ord(AdrSeg=SegYData) SHL 6)+Reg1;
	   END;
	 END
       ELSE IF NOT DecodeGeneralReg(Right,Reg2) THEN WrError(1350)
       ELSE IF Reg2<$20 THEN WrError(1350)
       ELSE    					      { ea/aa,D1 }
	BEGIN
	 DecodeAdr(Left,MModAll,MSegXData+MSegYData);
	 IF (AdrType=ModImm) AND (AdrVal<=$ff) AND (AdrVal>=0) THEN
	  BEGIN
	   CodeLen:=1;
	   DAsmCode[0]:=$050080+(AdrVal SHL 8)+Reg2;
	  END
	 ELSE IF (AdrType=ModAbs) AND (AdrVal>=0) AND (AdrVal<=63) THEN
	  BEGIN
	   CodeLen:=1;
	   DAsmCode[0]:=$058000+(AdrVal SHL 8)+(Ord(AdrSeg=SegYData) SHL 6)+Reg2;
	  END
	 ELSE IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
	   DAsmCode[0]:=$05c000+(AdrMode SHL 8)+(Ord(AdrSeg=SegYData) SHL 6)+Reg2;
	  END;
	END
      END;
     Exit;
    END;

   IF Memo('MOVEM') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF DecodeGeneralReg(Left,Reg1) THEN
	BEGIN
	 DecodeAdr(Right,MModNoImm,MSegCode);
	 IF (AdrType=ModAbs) AND (AdrVal>=0) AND (AdrVal<=63) THEN
	  BEGIN
	   CodeLen:=1;
	   DAsmCode[0]:=$070000+Reg1+(AdrVal SHL 8);
	  END
	 ELSE IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
	   DAsmCode[0]:=$074080+Reg1+(AdrMode SHL 8);
	  END;
	END
       ELSE IF NOT DecodeGeneralReg(Right,Reg2) THEN WrError(1350)
       ELSE
	BEGIN
	 DecodeAdr(Left,MModNoImm,MSegCode);
	 IF (AdrType=ModAbs) AND (AdrVal>=0) AND (AdrVal<=63) THEN
	  BEGIN
	   CodeLen:=1;
	   DAsmCode[0]:=$078000+Reg2+(AdrVal SHL 8);
	  END
	 ELSE IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
	   DAsmCode[0]:=$07c080+Reg2+(AdrMode SHL 8);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('MOVEP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF DecodeGeneralReg(Left,Reg1) THEN
	BEGIN
	 DecodeAdr(Right,MModAbs,MSegXData+MSegYData);
	 IF AdrType<>ModNone THEN
	  IF (AdrVal>$ffff) OR (AdrVal<$ffc0) THEN WrError(1315)
	  ELSE
	   BEGIN
	    CodeLen:=1;
	    DAsmCode[0]:=$08c000+(LongInt(Ord(AdrSeg=SegYData)) SHL 16)+
			 (AdrVal AND $3f)+(Reg1 SHL 8);
	   END;
	END
       ELSE IF DecodeGeneralReg(Right,Reg2) THEN
	BEGIN
	 DecodeAdr(Left,MModAbs,MSegXData+MSegYData);
	 IF AdrType<>ModNone THEN
	  IF (AdrVal>$ffff) OR (AdrVal<$ffc0) THEN WrError(1315)
	  ELSE
	   BEGIN
	    CodeLen:=1;
	    DAsmCode[0]:=$084000+(LongInt(Ord(AdrSeg=SegYData)) SHL 16)+
			 (AdrVal AND $3f)+(Reg2 SHL 8);
	   END;
	END
       ELSE
	BEGIN
	 DecodeAdr(Left,MModAll,MSegXData+MSegYData+MSegCode);
	 IF (AdrType=ModAbs) AND (AdrSeg<>SegCode) AND (AdrVal>=$ffc0) AND (AdrVal<=$ffff) THEN
	  BEGIN
	   HVal:=AdrVal AND $3f; HSeg:=AdrSeg;
	   DecodeAdr(Right,MModNoImm,MSegXData+MSegYData+MSegCode);
	   IF AdrType<>ModNone THEN
	    IF AdrSeg=SegCode THEN
	     BEGIN
	      CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
	      DAsmCode[0]:=$084040+HVal+(AdrMode SHL 8)+
			   (LongInt(Ord(HSeg=SegYData)) SHL 16);
	     END
	    ELSE
	     BEGIN
	      CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
	      DAsmCode[0]:=$084080+HVal+(AdrMode SHL 8)+
			   (LongInt(Ord(HSeg=SegYData)) SHL 16)+
			   (Ord(AdrSeg=SegYData) SHL 6);
	     END;
	  END
	 ELSE IF AdrType<>ModNone THEN
	  BEGIN
	   HVal:=AdrVal; HCnt:=AdrCnt; HMode:=AdrMode; HType:=AdrType; HSeg:=AdrSeg;
	   DecodeAdr(Right,MModAbs,MSegXData+MSegYData);
	   IF AdrType<>ModNone THEN
	    IF (AdrVal<$ffc0) OR (AdrVal>$ffff) THEN WrError(1315)
	    ELSE IF HSeg=SegCode THEN
	     BEGIN
	      CodeLen:=1+HCnt; DAsmCode[1]:=HVal;
	      DAsmCode[0]:=$08c040+(AdrVal AND $3f)+(HMode SHL 8)+
			   (LongInt(Ord(AdrSeg=SegYData)) SHL 16);
	     END
	    ELSE
	     BEGIN
	      CodeLen:=1+HCnt; DAsmCode[1]:=HVal;
	      DAsmCode[0]:=$08c080+(Word(AdrVal) AND $3f)+(HMode SHL 8)+
			   (LongInt(Ord(AdrSeg=SegYData)) SHL 16)+
			   (Ord(HSeg=SegYData) SHL 6);
	     END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('TFR') THEN
    BEGIN
     IF ArgCnt<1 THEN WrError(1110)
     ELSE IF DecodeMOVE(2) THEN
      IF DecodeTFR(ArgStr[1],Reg1) THEN
       BEGIN
	Inc(DAsmCode[0],$01+(Reg1 SHL 3));
       END
      ELSE
       BEGIN
	WrError(1350); CodeLen:=0;
       END;
     Exit;
    END;

   IF (OpPart[1]='T') AND (DecodeCondition(Copy(OpPart,2,Length(OpPart)-1),Condition)) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE IF NOT DecodeTFR(ArgStr[1],Reg1) THEN WrError(1350)
     ELSE IF ArgCnt=1 THEN
      BEGIN
       CodeLen:=1;
       DAsmCode[0]:=$020000+(Condition SHL 12)+(Reg1 SHL 3);
      END
     ELSE
      BEGIN
       SplitArg(ArgStr[2],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeReg(Left,Reg2) THEN WrError(1350)
       ELSE IF (Reg2<16) OR (Reg2>23) THEN WrError(1350)
       ELSE IF NOT DecodeReg(Right,Reg3) THEN WrError(1350)
       ELSE IF (Reg2<16) OR (Reg2>23) THEN WrError(1350)
       ELSE
	BEGIN
	 Dec(Reg2,16); Dec(Reg3,16);
	 CodeLen:=1;
	 DAsmCode[0]:=$030000+(Condition SHL 12)+(Reg2 SHL 8)+(Reg1 SHL 3)+Reg3;
	END;
      END;
     Exit;
    END;

   IF Memo('LUA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(Left,MModModInc+MModModDec+MModPostInc+MModPostDec,MSegXData);
	 IF AdrType<>ModNone THEN
	  IF NOT DecodeReg(Right,Reg1) THEN WrError(1350)
	  ELSE IF (Reg1<16) OR (Reg1>31) THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=1;
	    DAsmCode[0]:=$044000+(AdrMode SHL 8)+Reg1;
	   END;
	END
      END;
     Exit;
    END;

   { SprÅnge }

   IF (Memo('JMP')) OR (Memo('JSR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AddVal:=LongInt(Ord(Memo('JSR'))) SHL 16;
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       IF AdrType=ModAbs THEN
	IF AdrVal AND $f000=0 THEN
	 BEGIN
	  CodeLen:=1; DAsmCode[0]:=$0c0000+AddVal+AdrVal;
	 END
	ELSE
	 BEGIN
	  CodeLen:=2; DAsmCode[0]:=$0af080+AddVal; DAsmCode[1]:=AdrVal;
	 END
       ELSE IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=1; DAsmCode[0]:=$0ac080+AddVal+(AdrMode SHL 8);
	END;
      END;
     Exit;
    END;

   IF (OpPart[1]='J') AND (DecodeCondition(Copy(OpPart,2,Length(OpPart)-1),Condition)) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       IF AdrType=ModAbs THEN
	IF AdrVal AND $f000=0 THEN
	 BEGIN
	  CodeLen:=1; DAsmCode[0]:=$0e0000+(Condition SHL 12)+AdrVal;
	 END
	ELSE
	 BEGIN
	  CodeLen:=2; DAsmCode[0]:=$0af0a0+Condition; DAsmCode[1]:=AdrVal;
	 END
       ELSE IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=1; DAsmCode[0]:=$0ac0a0+Condition+(AdrMode SHL 8);
	END;
      END;
     Exit;
    END;

   IF (Copy(OpPart,1,2)='JS') AND (DecodeCondition(Copy(OpPart,3,Length(OpPart)-2),Condition)) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       IF AdrType=ModAbs THEN
	IF AdrVal AND $f000=0 THEN
	 BEGIN
	  CodeLen:=1; DAsmCode[0]:=$0f0000+(Condition SHL 12)+AdrVal;
	 END
	ELSE
	 BEGIN
	  CodeLen:=2; DAsmCode[0]:=$0bf0a0+Condition; DAsmCode[1]:=AdrVal;
	 END
       ELSE IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=1; DAsmCode[0]:=$0bc0a0+Condition+(AdrMode SHL 8);
	END;
      END;
     Exit;
    END;

   FOR z:=0 TO BitJmpOrderCnt-1 DO
    IF Memo(BitJmpOrders[z]) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE
       BEGIN
	SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
	IF (Left='') OR (Mid='') OR (Right='') THEN WrError(1110)
	ELSE IF Left[1]<>'#' THEN WrError(1120)
	ELSE
	 BEGIN
	  DAsmCode[1]:=EvalIntExpression(Right,Int16,OK);
	  IF OK THEN
	   BEGIN
	    h:=EvalIntExpression(Copy(Left,2,Length(Left)-1),Int8,OK);
	    IF FirstPassUnknown THEN h:=h AND 15;
	    IF OK THEN
	    IF (h<0) OR (h>23) THEN WrError(1320)
	    ELSE
	     BEGIN
	      Reg2:=((z AND 1) SHL 5)+(LongInt(z SHR 1) SHL 16);
	      IF DecodeGeneralReg(Mid,Reg1) THEN
	       BEGIN
		CodeLen:=2;
		DAsmCode[0]:=$0ac080+h+Reg2+(Reg1 SHL 8);
	       END
	      ELSE
	       BEGIN
		DecodeAdr(Mid,MModNoImm,MSegXData+MSegYData);
		Reg3:=Ord(AdrSeg=SegYData) SHL 6;
		IF AdrType=ModAbs THEN
		 IF (AdrVal>=0) AND (AdrVal<=63) THEN
		  BEGIN
		   CodeLen:=2;
		   DAsmCode[0]:=$0a0080+h+Reg2+Reg3+(AdrVal SHL 8);
		  END
		 ELSE IF (AdrVal>=$ffc0) AND (AdrVal<=$ffff) THEN
		  BEGIN
		   CodeLen:=2;
		   DAsmCode[0]:=$0a8080+h+Reg2+Reg3+((AdrVal AND $3f) SHL 8);
		  END
		 ELSE WrError(1320)
		ELSE
		 BEGIN
		  CodeLen:=2;
		  DAsmCode[0]:=$0a4080+h+Reg2+Reg3+(AdrMode SHL 8);
		 END;
	       END;
	     END;
	   END;
	 END;
       END;
      Exit;
     END;

   IF Memo('DO') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE
	BEGIN
	 DAsmCode[1]:=EvalIntExpression(Right,Int16,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegCode);
	   IF DecodeGeneralReg(Left,Reg1) THEN
	    BEGIN
	     CodeLen:=2; DAsmCode[0]:=$06c000+(Reg1 SHL 8);
	    END
	   ELSE IF Left[1]='#' THEN
	    BEGIN
	     Reg1:=EvalIntExpression(Copy(Left,2,Length(Left)-1),Int12,OK);
	     IF OK THEN
	      BEGIN
	       CodeLen:=2;
	       DAsmCode[0]:=$060080+(Reg1 SHR 8)+((Reg1 AND $ff) SHL 8);
	      END;
	    END
	   ELSE
	    BEGIN
	     DecodeAdr(Left,MModNoImm,MSegXData+MSegYData);
	     IF (AdrType=ModAbs) THEN
	      IF (AdrVal<0) OR (AdrVal>63) THEN WrError(1320)
	      ELSE
	       BEGIN
		CodeLen:=2;
		DAsmCode[0]:=$060000+(AdrVal SHL 8)+(Ord(AdrSeg=SegYData) SHL 6);
	       END
	     ELSE
	      BEGIN
	       CodeLen:=2;
	       DAsmCode[0]:=$064000+(AdrMode SHL 8)+(Ord(AdrSeg=SegYData) SHL 6);
	      END;
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('REP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF DecodeGeneralReg(ArgStr[1],Reg1) THEN
      BEGIN
       CodeLen:=1;
       DAsmCode[0]:=$06c020+(Reg1 SHL 8);
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAll,MSegXData+MSegYData);
       IF AdrType=ModImm THEN
	IF (AdrVal<0) OR (AdrVal>$fff) THEN WrError(1320)
	ELSE
	 BEGIN
	  CodeLen:=1;
	  DAsmCode[0]:=$0600a0+(AdrVal SHR 8)+((AdrVal AND $ff) SHL 8);
	 END
       ELSE IF (AdrType=ModAbs) AND (AdrVal>=0) AND (AdrVal<=63) THEN
	BEGIN
	 CodeLen:=1;
	 DAsmCode[0]:=$060020+(AdrVal SHL 8)+(Ord(AdrSeg=SegYData) SHL 6);
	END
       ELSE
	BEGIN
	 CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
	 DAsmCode[0]:=$064020+(AdrMode SHL 8)+(Ord(AdrSeg=SegYData) SHL 6);
	END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_56K:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode,
   SegXData,
   SegYData : ok:=ProgCounter <=$ffff;
   ELSE ok:=False;
   END;
   ChkPC_56K:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_56K:Boolean;
	Far;
BEGIN
   IsDef_56K:=(Memo('XSFR')) OR (Memo('YSFR'));
END;

        PROCEDURE SwitchFrom_56K;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_56K;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$09; NOPCode:=$000000;
   DivideChars:=' '#9; HasAttrs:=False;

   ValidSegs:=[SegCode,SegXData,SegYData];
   Grans[SegCode ]:=4; ListGrans[SegCode ]:=4; SegInits[SegCode ]:=0;
   Grans[SegXData]:=4; ListGrans[SegXData]:=4; SegInits[SegXData]:=0;
   Grans[SegYData]:=4; ListGrans[SegYData]:=4; SegInits[SegYData]:=0;

   MakeCode:=MakeCode_56K; ChkPC:=ChkPC_56K; IsDef:=IsDef_56K;
   SwitchFrom:=SwitchFrom_56K;
END;

BEGIN
   CPU56000:=AddCPU('56000',SwitchTo_56K);
END.
