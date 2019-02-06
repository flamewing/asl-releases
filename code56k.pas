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
               Name:String[8];
	       Code:LongInt;
               MinCPU:CPUVar;
	      END;

   ParOrder=RECORD
	     Name:String[4];
             Typ:(ParAB,ParABShl1,ParABShl2,ParXYAB,ParABXYnAB,ParABBA,ParXYnAB,ParMul,ParFixAB);
	     Code:Byte;
	    END;

CONST
   D_CPU56000=0;
   D_CPU56002=1;
   D_CPU56300=2;

   FixedOrderCnt=14;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
               ((Name:'NOP'    ; Code:$000000; MinCPU:D_CPU56000),
                (Name:'ENDDO'  ; Code:$00008c; MinCPU:D_CPU56000),
                (Name:'ILLEGAL'; Code:$000005; MinCPU:D_CPU56000),
                (Name:'RESET'  ; Code:$000084; MinCPU:D_CPU56000),
                (Name:'RTI'    ; Code:$000004; MinCPU:D_CPU56000),
                (Name:'RTS'    ; Code:$00000c; MinCPU:D_CPU56000),
                (Name:'STOP'   ; Code:$000087; MinCPU:D_CPU56000),
                (Name:'SWI'    ; Code:$000006; MinCPU:D_CPU56000),
                (Name:'WAIT'   ; Code:$000086; MinCPU:D_CPU56000),
                (Name:'DEBUG'  ; Code:$000200; MinCPU:D_CPU56300),
                (Name:'PFLUSH' ; Code:$000003; MinCPU:D_CPU56300),
                (Name:'PFLUSHUN'; Code:$000001; MinCPU:D_CPU56300),
                (Name:'PFREE'  ; Code:$000002; MinCPU:D_CPU56300),
                (Name:'TRAP'   ; Code:$000006; MinCPU:D_CPU56300));

   ParOrderCnt=31;
   ParOrders:ARRAY[1..ParOrderCnt] OF ParOrder=
	     ((Name:'ABS' ; Typ:ParAB;     Code:$26),
              (Name:'ASL' ; Typ:ParABShl1; Code:$32),
              (Name:'ASR' ; Typ:ParABShl1; Code:$22),
	      (Name:'CLR' ; Typ:ParAB;     Code:$13),
              (Name:'LSL' ; Typ:ParABShl2; Code:$33),
              (Name:'LSR' ; Typ:ParABShl2; Code:$23),
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
              (Name:'MPYR'; Typ:ParMul;    Code:$81),
              (Name:'MAX' ; Typ:ParFixAB;  Code:$1d),
              (Name:'MAXM'; Typ:ParFixAB;  Code:$15));

   BitOrderCnt=4;
   BitOrders:ARRAY[0..BitOrderCnt-1] OF String[4]=('BCLR','BSET','BCHG','BTST');

   BitJmpOrderCnt=4;
   BitJmpOrders:ARRAY[0..BitOrderCnt-1] OF String[5]=('JCLR','JSET','JSCLR','JSSET');

   BitBrOrderCnt=4;
   BitBrOrders:ARRAY[0..BitOrderCnt-1] OF String[5]=('BRCLR','BRSET','BSCLR','BSSET');

   ImmMacOrderCnt=4;
   ImmMacOrders:ARRAY[0..ImmMacOrderCnt-1] OF String[5]=('MPYI','MPYRI','MACI','MACRI');

   MacTable:ARRAY[4..7,4..7] OF Byte=((0,2,5,4),(2,$ff,6,7),(5,6,1,3),(4,7,3,$ff));

   Mac4Table:ARRAY[4..7,4..7] OF Byte=((0,13,10,4),(5,1,14,11),(2,6,8,15),(12,3,7,9));

   Mac2Table:ARRAY[0..3] OF Byte=(1,3,2,0);

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
   ModDisp=9;      MModDisp=1 SHL ModDisp;

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
   CPU56000,CPU56002,CPU56300:CPUVar;
   AdrInt:IntType;
   MemLimit:LongInt;

   TargSegment:Byte;
   AdrType:ShortInt;
   AdrMode:LongInt;
   AdrVal:LongInt;
   AdrCnt:Byte;
   AdrSeg:Byte;


        PROCEDURE SplitArg(Orig:String; VAR LDest,RDest:String);
VAR
   p:Integer;
BEGIN
   p:=QuotPos(Orig,',');
   RDest:=Copy(Orig,p+1,Length(Orig)-p);
   LDest:=Copy(Orig,1,p-1);
END;

        PROCEDURE CutSize(VAR Asc:String; VAR ShortMode:Byte);
BEGIN
   IF Asc[1]='>' THEN
    BEGIN
     Delete(Asc,1,1); ShortMode:=2;
    END
   ELSE IF Asc[1]='<' THEN
    BEGIN
     Delete(Asc,1,1); ShortMode:=1;
    END
   ELSE ShortMode:=0;
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

        FUNCTION DecodeXYAB0Reg(Asc:String; VAR Erg:LongInt):Boolean;
CONST
   RegCount=6;
   RegNames:ARRAY[0..RegCount-1] OF String[2]=
            ('A0','B0','X0','Y0','X1','Y1');
VAR
   z:Word;
BEGIN
   DecodeXYAB0Reg:=False;
   FOR z:=0 TO RegCount-1 DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      Erg:=z+2; DecodeXYAB0Reg:=True; Exit;
     END;
END;

        FUNCTION DecodeXYAB1Reg(Asc:String; VAR Erg:LongInt):Boolean;
CONST
   RegCount=6;
   RegNames:ARRAY[0..RegCount-1] OF String[2]=
            ('A1','B1','X0','Y0','X1','Y1');
VAR
   z:Word;
BEGIN
   DecodeXYAB1Reg:=False;
   FOR z:=0 TO RegCount-1 DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      Erg:=z+2; DecodeXYAB1Reg:=True; Exit;
     END;
END;

        FUNCTION DecodePCReg(Asc:String; VAR Erg:LongInt):Boolean;
CONST
   RegCount=8;
   RegNames:ARRAY[1..RegCount] OF String[3]=
            ('SZ','SR','OMR','SP','SSH','SSL','LA','LC');
VAR
   z:Word;
BEGIN
   DecodePCReg:=False;
   FOR z:=1 TO RegCount DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      Erg:=z-1; DecodePCReg:=True; Exit;
     END;
END;

        FUNCTION DecodeAddReg(Asc:String; VAR Erg:LongInt):Boolean;
BEGIN
   DecodeAddReg:=True;
   IF (Length(Asc)=2) AND (UpCase(Asc[1])='M') AND (Asc[2]>='0') AND (Asc[2]<='7') THEN
    BEGIN
     Erg:=Ord(Asc[2])-AscOfs; Exit;
    END;
   IF NLS_StrCaseCmp(Asc,'EP')=0 THEN
    BEGIN
     Erg:=$0a; Exit;
    END;
   IF NLS_StrCaseCmp(Asc,'VBA')=0 THEN
    BEGIN
     Erg:=$10; Exit;
    END;
   IF NLS_StrCaseCmp(Asc,'SC')=0 THEN
    BEGIN
     Erg:=$11; Exit;
    END;
   DecodeAddReg:=False;
END;

        FUNCTION DecodeGeneralReg(Asc:String; VAR Erg:LongInt):Boolean;
BEGIN
   DecodeGeneralReg:=True;
   IF DecodeReg(Asc,Erg) THEN Exit;
   IF DecodePCReg(Asc,Erg) THEN
    BEGIN
     Inc(Erg,$38); Exit;
    END;
   IF DecodeAddReg(Asc,Erg) THEN
    BEGIN
     Inc(Erg,$20); Exit;
    END;
   DecodeGeneralReg:=False;
END;

        FUNCTION DecodeControlReg(Asc:String; VAR Erg:LongInt):Boolean;
BEGIN
   DecodeControlReg:=True;
   IF NLS_StrCaseCmp(Asc,'MR')=0 THEN Erg:=0
   ELSE IF NLS_StrCaseCmp(Asc,'CCR')=0 THEN Erg:=1
   ELSE IF (NLS_StrCaseCmp(Asc,'OMR')=0) OR (NLS_StrCaseCmp(Asc,'COM')=0) THEN Erg:=2
   ELSE IF (NLS_StrCaseCmp(Asc,'EOM')=0) AND (MomCPU>=CPU56300) THEN Erg:=3
   ELSE DecodeControlReg:=False;
END;

        FUNCTION DecodeCtrlReg(VAR Asc:String; VAR Erg:LongInt):Boolean;
BEGIN
   DecodeCtrlReg:=True;
   IF DecodeAddReg(Asc,Erg) THEN Exit;
   IF DecodePCReg(Asc,Erg) THEN
    BEGIN
     Inc(Erg,$18); Exit;
    END;
   DecodeCtrlReg:=False;
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
   z,l,pp,np:Integer;
   OK,BreakLoop:Boolean;
   OrdVal:Byte;
BEGIN
   AdrType:=ModNone; AdrCnt:=0;

   { Adressierungsmodi vom 56300 abschneiden }

   IF (MomCPU<CPU56300) AND (Erl AND MModDisp<>0) THEN Dec(Erl,MModDisp);

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

   { Register mit Displacement bei 56300 }

   IF IsIndirect(Asc) THEN
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);
     pp:=Pos('+',Asc); np:=Pos('-',Asc);
     IF (pp=0) OR ((np<>0) AND (np<pp)) THEN pp:=np;
     IF pp<>0 THEN
      IF (DecodeGeneralReg(Copy(Asc,1,pp-1),AdrMode)) AND (AdrMode>=16) AND (AdrMode<=23) THEN
       BEGIN
        Dec(AdrMode,16); FirstPassUnknown:=False;
        AdrVal:=EvalIntExpression(Copy(Asc,pp,Length(Asc)-pp+1),Int24,OK);
        IF OK THEN
         BEGIN
          IF FirstPassUnknown THEN AdrVal:=AdrVal AND 63;
          AdrType:=ModDisp;
         END;
        Goto Found;
       END;
    END;

   { dann absolut }

   AdrVal:=EvalIntExpression(Asc,AdrInt,OK);
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

        FUNCTION DecodeRR(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   Part1,Part2:LongInt;
   Left,Right:String;
BEGIN
   DecodeRR:=False;
   SplitArg(Asc,Left,Right);
   IF NOT DecodeGeneralReg(Right,Part2) THEN Exit;
   IF (Part2<16) OR (Part2>23) THEN Exit;
   IF NOT DecodeGeneralReg(Left,Part1) THEN Exit;
   IF (Part1<16) OR (Part1>23) THEN Exit;
   Erg:=(Part2 AND 7)+((Part1 AND 7) SHL 8);
   DecodeRR:=True;
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

        FUNCTION DecodeMOVE(Start:Integer):Boolean;

	FUNCTION DecodeMOVE_0:Boolean;
BEGIN
   DecodeMOVE_0:=True; DAsmCode[0]:=$200000; CodeLen:=1;
END;

	FUNCTION DecodeMOVE_1:Boolean;
VAR
   Left,Right:String;
   RegErg,RegErg2,IsY,MixErg,l:LongInt;
   Condition:Word;
   SegMask:Byte;
BEGIN
   { Bedingungen ab 56300 }

   IF  (NLS_StrCaseCmp(Copy(ArgStr[Start],1,2),'IF')=0) THEN
    BEGIN
     l:=Length(ArgStr[Start]);
     IF NLS_StrCaseCmp(Copy(ArgStr[Start],l-1,2),'.U')=0 THEN
      BEGIN
       RegErg:=$1000; Dec(l,2);
      END
     ELSE RegErg:=0;
     IF DecodeCondition(Copy(ArgStr[Start],3,l-2),Condition) THEN
      IF MomCPU<CPU56300 THEN WrError(1505)
      ELSE
       BEGIN
        DecodeMOVE_1:=True;
        DAsmCode[0]:=$202000+(Condition SHL 8)+RegErg;
        CodeLen:=1;
        Exit;
       END;
    END;

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
     AdrSeg:=SegNone;
     IF DecodeReg(Left,RegErg2) THEN
      BEGIN
       DecodeMOVE_1:=True;
       DAsmCode[0]:=$200000+(RegErg SHL 8)+(RegErg2 SHL 13);
       CodeLen:=1;
      END
     ELSE
      BEGIN
       SegMask:=MSegXData+MSegYData;
       IF (RegErg=14) OR (RegErg=15) THEN Inc(SegMask,MSegLData);
       DecodeAdr(Left,MModAll+MModDisp,SegMask);
       IF AdrSeg<>SegLData THEN
        BEGIN
         IsY:=Ord(AdrSeg=SegYData);
         MixErg:=((RegErg AND $18) SHL 17)+(IsY SHL 19)+((RegErg AND 7) SHL 16);
         IF AdrType=ModDisp THEN
          IF (AdrVal<63) AND (AdrVal>-64) AND (RegErg<=15) THEN
           BEGIN
            DAsmCode[0]:=$020090+((AdrVal AND 1) SHL 6)+((AdrVal AND $7e) SHL 10)
                        +(AdrMode SHL 8)+(IsY SHL 5)+RegErg;
            CodeLen:=1;
           END
          ELSE
           BEGIN
            DAsmCode[0]:=$0a70c0+(AdrMode SHL 8)+(IsY SHL 16)+RegErg;
            DAsmCode[1]:=AdrVal;
            CodeLen:=2;
           END
         ELSE IF (AdrType=ModImm) AND (AdrVal AND $ffffff00=0) THEN
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
      END;
     IF AdrSeg<>SegLData THEN Exit;
    END;

   { 3. Quelle ist Register }

   IF DecodeReg(Left,RegErg) THEN
    BEGIN
     SegMask:=MSegXData+MSegYData; AdrSeg:=SegNone;
     IF (RegErg=14) OR (RegErg=15) THEN Inc(SegMask,MSegLData);
     DecodeAdr(Right,MModNoImm+MModDisp,SegMask);
     IF AdrSeg<>SegLData THEN
      BEGIN
       IsY:=Ord(AdrSeg=SegYData);
       MixErg:=((RegErg AND $18) SHL 17)+(IsY SHL 19)+((RegErg AND 7) SHL 16);
       IF AdrType=ModDisp THEN
        IF (AdrVal<63) AND (AdrVal>-64) AND (RegErg<=15) THEN
         BEGIN
          DAsmCode[0]:=$020080+((AdrVal AND 1) SHL 6)+((AdrVal AND $7e) SHL 10)
                      +(AdrMode SHL 8)+(IsY SHL 5)+RegErg;
          CodeLen:=1;
         END
        ELSE
         BEGIN
          DAsmCode[0]:=$0a7080+(AdrMode SHL 8)+(IsY SHL 16)+RegErg;
          DAsmCode[1]:=AdrVal;
          CodeLen:=2;
         END
       ELSE IF (AdrType=ModAbs) AND (AdrVal<=63) AND (AdrVal>=0) THEN
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

   IF (NLS_StrCaseCmp(Left2,'X0')=0) AND (NLS_StrCaseCmp(Left1,Right2)=0) THEN
    BEGIN
     IF NOT DecodeALUReg(Right2,RegErg,False,False,True) THEN WrError(1350)
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

   IF (NLS_StrCaseCmp(Left1,'Y0')=0) AND (NLS_StrCaseCmp(Right1,Left2)=0) THEN
    BEGIN
     IF NOT DecodeALUReg(Right1,RegErg,False,False,True) THEN WrError(1350)
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

        FUNCTION DecodeMix(Asc:String; VAR Erg:Word):Boolean;
BEGIN
   DecodeMix:=True;
   IF Asc='SS' THEN Erg:=0
   ELSE IF Asc='SU' THEN Erg:=$100
   ELSE IF Asc='UU' THEN Erg:=$140
   ELSE DecodeMix:=False;
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
     CodeEquate(SegXData,0,MemLimit);
     Exit;
    END;

   IF Memo('YSFR') THEN
    BEGIN
     CodeEquate(SegYData,0,MemLimit);
     Exit;
    END;

{   IF (Memo('XSFR')) OR (Memo('YSFR')) THEN
    BEGIN
     FirstPassUnknown:=False;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],AdrInt,OK);
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
       AdrWord:=EvalIntExpression(ArgStr[1],AdrInt,OK);
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

	FUNCTION DecodeJump:Boolean;
VAR
   AddVal,h,Reg1,Reg2,Reg3:LongInt;
   Condition:Word;
   z:Integer;
   Left,Mid,Right:String;
   OK:Boolean;
BEGIN
   DecodeJump:=True;

   IF (Memo('JMP')) OR (Memo('JSR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AddVal:=LongInt(Ord(Memo('JSR'))) SHL 16;
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       IF AdrType=ModAbs THEN
        IF AdrVal AND $fff000=0 THEN
	 BEGIN
          CodeLen:=1; DAsmCode[0]:=$0c0000+AddVal+(AdrVal AND $fff);
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
        IF AdrVal AND $fff000=0 THEN
	 BEGIN
          CodeLen:=1; DAsmCode[0]:=$0e0000+(Condition SHL 12)+(AdrVal AND $fff);
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
        IF AdrVal AND $fff000=0 THEN
	 BEGIN
          CodeLen:=1; DAsmCode[0]:=$0f0000+(Condition SHL 12)+(AdrVal AND $fff);
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
        IF (Mid='') OR (Right='') THEN WrError(1110)
	ELSE IF Left[1]<>'#' THEN WrError(1120)
	ELSE
	 BEGIN
          DAsmCode[1]:=EvalIntExpression(Right,AdrInt,OK);
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
                DAsmCode[0]:=$0ac000+h+Reg2+(Reg1 SHL 8);
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
                 ELSE IF (AdrVal>=MemLimit-$3f) AND (AdrVal<=MemLimit) THEN
		  BEGIN
		   CodeLen:=2;
		   DAsmCode[0]:=$0a8080+h+Reg2+Reg3+((AdrVal AND $3f) SHL 8);
		  END
                 ELSE IF (MomCPU>=CPU56300) AND (AdrVal>=MemLimit-$7f) AND (AdrVal<=MemLimit-$40) THEN
		  BEGIN
		   CodeLen:=2;
                   Reg2:=((z AND 1) SHL 5)+(LongInt(z SHR 1) SHL 14);
                   DAsmCode[0]:=$018080+h+Reg2+Reg3+((AdrVal AND $3f) SHL 8);
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

   IF (Memo('DO')) OR (Memo('DOR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF (Memo('DOR')) AND (MomCPU<CPU56300) THEN WrError(1500)
     ELSE
      BEGIN
       z:=Ord(Memo('DOR'));
       SplitArg(ArgStr[1],Left,Right);
       IF (Right='') THEN WrError(1110)
       ELSE
	BEGIN
         DAsmCode[1]:=EvalIntExpression(Right,AdrInt,OK)-1;
         IF Memo('DOR') THEN DAsmCode[1]:=(DAsmCode[1]-(EProgCounter+2)) AND $ffffff;
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegCode);
           IF NLS_StrCaseCmp(Left,'FOREVER')=0 THEN
            BEGIN
             IF MomCPU<CPU56300 THEN WrError(1500)
             ELSE
              BEGIN
               DAsmCode[0]:=$000203-z; CodeLen:=2;
              END;
            END
           ELSE IF DecodeGeneralReg(Left,Reg1) THEN
	    BEGIN
             IF Reg1=$3c THEN WrXError(1445,Left) { kein SSH! }
             ELSE
              BEGIN
               CodeLen:=2; DAsmCode[0]:=$06c000+(Reg1 SHL 8)+(z SHL 4);
              END
	    END
	   ELSE IF Left[1]='#' THEN
	    BEGIN
             Reg1:=EvalIntExpression(Copy(Left,2,Length(Left)-1),UInt12,OK);
	     IF OK THEN
	      BEGIN
	       CodeLen:=2;
               DAsmCode[0]:=$060080+(Reg1 SHR 8)+((Reg1 AND $ff) SHL 8)+(z SHL 4);
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
                DAsmCode[0]:=$060000+(AdrVal SHL 8)+(Ord(AdrSeg=SegYData) SHL 6)+(z SHL 4);
	       END
	     ELSE
	      BEGIN
	       CodeLen:=2;
               DAsmCode[0]:=$064000+(AdrMode SHL 8)+(Ord(AdrSeg=SegYData) SHL 6)+(z SHL 4);
	      END;
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Copy(OpPart,1,3)='BRK') AND (DecodeCondition(Copy(OpPart,4,Length(OpPart)-3),Condition)) THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       DAsmCode[0]:=$00000210+Condition;
       CodeLen:=1;
      END;
     Exit;
    END;

   IF (Copy(OpPart,1,4)='TRAP') AND (DecodeCondition(Copy(OpPart,5,Length(OpPart)-4),Condition)) THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       DAsmCode[0]:=$000010+Condition;
       CodeLen:=1;
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
       ELSE IF (AdrType=ModAbs) THEN
        IF (AdrVal<0) OR (AdrVal>63) THEN WrError(1320)
        ELSE
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

   DecodeJump:=False;
END;

	PROCEDURE MakeCode_56K;
	Far;
VAR
   z,pp,np:Integer;
   AddVal,h,h2,Reg1,Reg2,Reg3,Dist:LongInt;
   HVal,HCnt,HMode,HType,HSeg:LongInt;
   Condition:Word;
   OK:Boolean;
   Left,Mid,Right:String;
   ErrCode:Integer;
   ErrString:String;
   DontAdd:Boolean;
   Size:Byte;

   	PROCEDURE SetError(Code:Integer);
BEGIN
   ErrCode:=Code; ErrString:='';
END;

	PROCEDURE SetXError(Code:Integer; VAR Ext:String);
BEGIN
   ErrCode:=Code; ErrString:=Ext;
END;

	PROCEDURE PrError;
BEGIN
   IF ErrString<>'' THEN WrXError(ErrCode,ErrString)
   ELSE IF ErrCode<>0 THEN WrError(ErrCode);
END;

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
       ELSE IF MomCPU-CPU56000<MinCPU THEN WrError(1500)
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
	 ErrCode:=0; ErrString:=''; DontAdd:=False;
	 CASE Typ OF
	 ParAB:
	  BEGIN
	   IF NOT DecodeALUReg(ArgStr[1],Reg1,False,False,True) THEN SetXError(1445,ArgStr[1])
	   ELSE h:=Reg1 SHL 3;
	  END;
         ParFixAB:
          BEGIN
           IF NLS_StrCaseCmp(ArgStr[1],'A,B')<>0 THEN SetError(1760)
           ELSE h:=0;
          END;
         ParABShl1:
          BEGIN
           IF Pos(',',ArgStr[1])=0 THEN
            BEGIN
             IF NOT DecodeALUReg(ArgStr[1],Reg1,False,False,True) THEN SetXError(1445,ArgStr[1])
             ELSE h:=Reg1 SHL 3;
            END
           ELSE IF ArgCnt<>1 THEN SetError(1950)
           ELSE IF MomCPU<CPU56300 THEN SetError(1500)
           ELSE
            BEGIN
             SplitArg(ArgStr[1],Left,Right);
             IF Pos(',',Right)=0 THEN Mid:=Right
             ELSE SplitArg(Right,Mid,Right);
             IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN SetXError(1445,Right)
             ELSE IF NOT DecodeALUReg(Mid,Reg2,False,False,True) THEN SetXError(1445,Mid)
             ELSE IF Left[1]='#' THEN
              BEGIN
               AddVal:=EvalIntExpression(Copy(Left,2,Length(Left)-1),UInt6,OK);
               IF OK THEN
                BEGIN
                 DAsmCode[0]:=$0c1c00+((Code AND $10) SHL 4)+(Reg2 SHL 7)+
                              (AddVal SHL 1)+Reg1;
                 CodeLen:=1; DontAdd:=True;
                END;
              END
             ELSE IF NOT DecodeXYAB1Reg(Left,Reg3) THEN SetXError(1445,Left)
             ELSE
              BEGIN
               DAsmCode[0]:=$0c1e60-((Code AND $10) SHL 2)+(Reg2 SHL 4)+
                            (Reg3 SHL 1)+Reg1;
               CodeLen:=1; DontAdd:=True;
              END;
            END;
          END;
         ParABShl2:
          BEGIN
           IF Pos(',',ArgStr[1])=0 THEN
            BEGIN
             IF NOT DecodeALUReg(ArgStr[1],Reg1,False,False,True) THEN SetXError(1445,ArgStr[1])
             ELSE h:=Reg1 SHL 3;
            END
           ELSE IF ArgCnt<>1 THEN SetError(1950)
           ELSE IF MomCPU<CPU56300 THEN SetError(1500)
           ELSE
            BEGIN
             SplitArg(ArgStr[1],Left,Right);
             IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN SetXError(1445,Right)
             ELSE IF Left[1]='#' THEN
              BEGIN
               AddVal:=EvalIntExpression(Copy(Left,2,Length(Left)-1),UInt5,OK);
               IF OK THEN
                BEGIN
                 DAsmCode[0]:=$0c1e80+(($33-Code) SHL 2)+
                              (AddVal SHL 1)+Reg1;
                 CodeLen:=1; DontAdd:=True;
                END;
              END
             ELSE IF NOT DecodeXYAB1Reg(Left,Reg3) THEN SetXError(1445,Left)
             ELSE
              BEGIN
               DAsmCode[0]:=$0c1e10+(($33-Code) SHL 1)+
                            (Reg3 SHL 1)+Reg1;
               CodeLen:=1; DontAdd:=True;
              END;
            END;
          END;
         ParXYAB:
	  BEGIN
	   SplitArg(ArgStr[1],Left,Right);
	   IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN SetXError(1445,Right)
	   ELSE IF NOT DecodeLReg(Left,Reg1) THEN SetXError(1445,Left)
	   ELSE IF (Reg1<2) OR (Reg1>3) THEN SetXError(1445,Left)
	   ELSE h:=(Reg2 SHL 3)+((Reg1-2) SHL 4);
	  END;
	 ParABXYnAB:
	  BEGIN
	   SplitArg(ArgStr[1],Left,Right);
	   IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN SetXError(1445,Right)
           ELSE IF Left[1]='#' THEN
            BEGIN
             IF Memo('CMPM') THEN SetError(1350)
             ELSE IF MomCPU<CPU56300 THEN SetError(1500)
             ELSE IF ArgCnt<>1 THEN SetError(1950)
             ELSE
              BEGIN
               AddVal:=EvalIntExpression(Copy(Left,2,Length(Left)-1),UInt24,OK);
               IF NOT OK THEN SetError(-1)
               ELSE IF (AddVal>=0) AND (AddVal<=63) THEN
                BEGIN
                 DAsmCode[0]:=$014000+(AddVal SHL 8); h:=$80+(Reg2 SHL 3);
                END
               ELSE
                BEGIN
                 DAsmCode[0]:=$014000; h:=$c0+(Reg2 SHL 3);
                 DAsmCode[1]:=AddVal AND $ffffff; CodeLen:=2;
                END;
              END
            END
	   ELSE
	    IF NOT DecodeXYABReg(Left,Reg1) THEN SetXError(1445,Left)
            ELSE IF Reg1 XOR Reg2=1 THEN SetError(1760)
            ELSE IF (Memo('CMPM')) AND (Reg1 AND 6=2) THEN SetXError(1445,Left)
            ELSE
	     BEGIN
              IF Reg1=0 THEN Reg1:=Ord(NOT Memo('CMPM'));
              h:=(Reg2 SHL 3)+(Reg1 SHL 4);
	     END;
	  END;
	 ParABBA:
          IF NLS_StrCaseCmp(ArgStr[1],'B,A')=0 THEN h:=0
          ELSE IF NLS_StrCaseCmp(ArgStr[1],'A,B')=0 THEN h:=8
	  ELSE SetXError(1760,ArgStr[1]);
	 ParXYnAB:
	  BEGIN
	   SplitArg(ArgStr[1],Left,Right);
	   IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN SetXError(1445,Right)
           ELSE IF Left[1]='#' THEN
            BEGIN
             IF MomCPU<CPU56300 THEN SetError(1500)
             ELSE IF ArgCnt<>1 THEN SetError(1950)
             ELSE
              BEGIN
               AddVal:=EvalIntExpression(Copy(Left,2,Length(Left)-1),UInt24,OK);
               IF NOT OK THEN SetError(-1)
               ELSE IF (AddVal>=0) AND (AddVal<=63) THEN
                BEGIN
                 DAsmCode[0]:=$014080+(AddVal SHL 8)+(Reg2 SHL 3)+(Code AND 7);
                 CodeLen:=1;
                 DontAdd:=True;
                END
               ELSE
                BEGIN
                 DAsmCode[0]:=$0140c0+(Reg2 SHL 3)+(Code AND 7);
                 DAsmCode[1]:=AddVal AND $ffffff; CodeLen:=2;
                 DontAdd:=True;
                END;
              END
            END
	   ELSE
	    IF NOT DecodeReg(Left,Reg1) THEN SetXError(1445,Left)
	    ELSE IF (Reg1<4) OR (Reg1>7) THEN SetXError(1445,Left)
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
	   IF NOT DecodeALUReg(Right,Reg3,False,False,True) THEN SetXError(1445,Right)
	   ELSE IF NOT DecodeReg(Left,Reg1) THEN SetXError(1445,Left)
	   ELSE IF (Reg1<4) OR (Reg1>7) THEN SetXError(1445,Left)
           ELSE IF Mid[1]='#' THEN
            BEGIN
             IF ArgCnt<>1 THEN WrError(1110)
             ELSE IF MomCPU<CPU56300 THEN WrError(1500)
             ELSE
              BEGIN
               FirstPassUnknown:=False;
               AddVal:=EvalIntExpression(Copy(Mid,2,Length(Mid)-1),UInt24,OK);
               IF FirstPassUnknown THEN AddVal:=1;
               IF NOT (SingleBit(AddVal,AddVal)) OR (AddVal>22) THEN WrError(1540)
               ELSE
                BEGIN
                 AddVal:=23-AddVal;
                 DAsmCode[0]:=$010040+(AddVal SHL 8)+Mac2Table[Reg1 AND 3] SHL 4
                             +(Reg3 SHL 3);
                 CodeLen:=1;
                END;
              END;
            END
	   ELSE IF NOT DecodeReg(Mid,Reg2) THEN SetXError(1445,Mid)
	   ELSE IF (Reg2<4) OR (Reg2>7) THEN SetXError(1445,Mid)
	   ELSE IF MacTable[Reg1,Reg2]=$ff THEN SetError(1760)
	   ELSE Inc(h,(Reg3 SHL 3)+(MacTable[Reg1,Reg2] SHL 4));
	  END;
	 END;
	 IF ErrCode=0 THEN
          BEGIN
           IF NOT DontAdd THEN Inc(DAsmCode[0],Code+h);
          END
	 ELSE
	  BEGIN
	   IF ErrCode>0 THEN PrError; CodeLen:=0;
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

   FOR z:=0 TO ImmMacOrderCnt-1 DO
    IF Memo(ImmMacOrders[z]) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE IF MomCPU<CPU56300 THEN WrError(1500)
      ELSE
       BEGIN
        SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
        h:=0;
        IF Left[1]='-' THEN
         BEGIN
          h:=4; Delete(Left,1,1);
         END
        ELSE IF Left[1]='+' THEN Delete(Left,1,1);
        IF (Mid='') OR (Right='') THEN WrError(1110)
        ELSE IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN WrXError(1445,Right)
        ELSE IF NOT DecodeXYABReg(Mid,Reg2) THEN WrXError(1445,Mid)
        ELSE IF (Reg2<4) OR (Reg2>7) THEN WrXError(1445,Mid)
        ELSE IF Left[1]<>'#' THEN WrError(1120)
        ELSE
         BEGIN
          DAsmCode[1]:=EvalIntExpression(Copy(Left,2,Length(Left)-1),Int24,OK);
          IF OK THEN
           BEGIN
            DAsmCode[0]:=$0141c0+z+h+(Reg1 SHL 3)+((Reg2 AND 3) SHL 4);
            CodeLen:=2;
           END;
         END;
       END;
      Exit;
     END;

   IF (Copy(OpPart,1,4)='DMAC') AND (DecodeMix(Copy(OpPart,5,Length(OpPart)-4),Condition)) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
       IF Left[1]='-' THEN
        BEGIN
         Delete(Left,1,1); Inc(Condition,16);
        END
       ELSE IF Left[1]='+' THEN Delete(Left,1,1);
       IF (Mid='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN WrXError(1445,Right)
       ELSE IF NOT DecodeXYAB1Reg(Mid,Reg2) THEN WrXError(1445,Mid)
       ELSE IF Reg2<4 THEN WrXError(1445,Mid)
       ELSE IF NOT DecodeXYAB1Reg(Left,Reg3) THEN WrXError(1445,Left)
       ELSE IF Reg3<4 THEN WrXError(1445,Left)
       ELSE
        BEGIN
         DAsmCode[0]:=$012480+Condition+(Reg1 SHL 5)+Mac4Table[Reg3,Reg2];
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   IF ((Copy(OpPart,1,3)='MAC') OR (Copy(OpPart,1,3)='MPY'))
   AND (DecodeMix(Copy(OpPart,4,Length(OpPart)-3),Condition)) THEN
    BEGIN
     IF Condition=0 THEN WrXError(1200,OpPart)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       IF OpPart[2]='A' THEN Dec(Condition,$100);
       SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
       IF Left[1]='-' THEN
        BEGIN
         Delete(Left,1,1); Inc(Condition,16);
        END
       ELSE IF Left[1]='+' THEN Delete(Left,1,1);
       IF (Mid='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN WrXError(1445,Right)
       ELSE IF NOT DecodeXYAB1Reg(Mid,Reg2) THEN WrXError(1445,Mid)
       ELSE IF Reg2<4 THEN WrXError(1445,Mid)
       ELSE IF NOT DecodeXYAB1Reg(Left,Reg3) THEN WrXError(1445,Left)
       ELSE IF Reg3<4 THEN WrXError(1445,Left)
       ELSE
        BEGIN
         DAsmCode[0]:=$012680+Condition+(Reg1 SHL 5)+Mac4Table[Reg3,Reg2];
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   IF (Memo('INC')) OR (Memo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56002 THEN WrError(1500)
     ELSE IF NOT DecodeALUReg(ArgStr[1],Reg1,False,False,True) THEN WrXError(1445,ArgStr[1])
     ELSE
      BEGIN
       DAsmCode[0]:=$000008+(Ord(Memo('DEC')) SHL 1)++Reg1;
       CodeLen:=1;
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
       ELSE IF NOT DecodeControlReg(Right,Reg1) THEN WrXError(1445,Right)
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

   IF Memo('NORMF') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN WrXError(1445,Right)
       ELSE IF NOT DecodeXYAB1Reg(Left,Reg1) THEN WrXError(1445,Left)
       ELSE
	BEGIN
         CodeLen:=1; DAsmCode[0]:=$0c1e20+Reg2+(Reg1 SHL 1);
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
             ELSE IF (AdrType=ModAbs) AND (AdrVal>=MemLimit-$3f) AND (AdrVal<=MemLimit) THEN
	      BEGIN
	       CodeLen:=1;
	       DAsmCode[0]:=$0a8000+h+((AdrVal AND $3f) SHL 8)+Reg3+Reg2;
	      END
             ELSE IF (AdrType=ModAbs) AND (MomCPU>=CPU56300) AND (AdrVal>=MemLimit-$7f) AND (AdrVal<=MemLimit-$40) THEN
	      BEGIN
	       CodeLen:=1;
               Reg2:=((z AND 1) SHL 5)+(LongInt(z SHR 1) SHL 14);
               DAsmCode[0]:=$010000+h+((AdrVal AND $3f) SHL 8)+Reg3+Reg2;
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

   IF (Memo('EXTRACT')) OR (Memo('EXTRACTU')) THEN
    BEGIN
     z:=Ord(Memo('EXTRACTU')) SHL 7;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
       IF (Mid='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN WrXError(1445,Right)
       ELSE IF NOT DecodeALUReg(Mid,Reg2,False,False,True) THEN WrXError(1445,Mid)
       ELSE IF Left[1]='#' THEN
        BEGIN
         DAsmCode[1]:=EvalIntExpression(Copy(Left,2,Length(Left)-1),Int24,OK);
         IF OK THEN
          BEGIN
           DAsmCode[0]:=$0c1800+z+Reg1+(Reg2 SHL 4); CodeLen:=2;
          END;
        END
       ELSE IF NOT DecodeXYAB1Reg(Left,Reg3) THEN WrXError(1445,Left)
       ELSE
        BEGIN
         DAsmCode[0]:=$0c1a00+z+Reg1+(Reg2 SHL 4)+(Reg3 SHL 1);
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   IF Memo('INSERT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Mid); SplitArg(Mid,Mid,Right);
       IF (Mid='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN WrXError(1445,Right)
       ELSE IF NOT DecodeXYAB0Reg(Mid,Reg2) THEN WrXError(1445,Mid)
       ELSE IF Left[1]='#' THEN
        BEGIN
         DAsmCode[1]:=EvalIntExpression(Copy(Left,2,Length(Left)-1),Int24,OK);
         IF OK THEN
          BEGIN
           DAsmCode[0]:=$0c1900+Reg1+(Reg2 SHL 4); CodeLen:=2;
          END;
        END
       ELSE IF NOT DecodeXYAB1Reg(Left,Reg3) THEN WrXError(1445,Left)
       ELSE
        BEGIN
         DAsmCode[0]:=$0c1b00+Reg1+(Reg2 SHL 4)+(Reg3 SHL 1);
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   IF Memo('MERGE') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       If Right='' THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN WrXError(1445,Right)
       ELSE IF NOT DecodeXYAB1Reg(Left,Reg2) THEN WrXError(1445,Left)
       ELSE
        BEGIN
         DAsmCode[0]:=$0c1b80+Reg1+(Reg2 SHL 1);
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   IF Memo('CLB') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF Right='' THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Left,Reg1,False,False,True) THEN WrXError(1445,Left)
       ELSE IF NOT DecodeALUReg(Right,Reg2,False,False,True) THEN WrXError(1445,Right)
       ELSE
        BEGIN
         DAsmCode[0]:=$0c1e00+Reg2+(Reg1 SHL 1);
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   IF Memo('CMPU') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeALUReg(Right,Reg1,False,False,True) THEN WrXError(1445,Right)
       ELSE IF NOT DecodeXYABReg(Left,Reg2) THEN WrXError(1445,Left)
       ELSE IF Reg1 XOR Reg2=1 THEN WrError(1760)
       ELSE IF (Reg2 AND 6=2) THEN WrXError(1445,Left)
       ELSE
        BEGIN
         IF Reg2<2 THEN Reg2:=0;
         DAsmCode[0]:=$0c1ff0+(Reg2 SHL 1)+Reg1;
         CodeLen:=1;
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
       IF Right='' THEN WrError(1110)
       ELSE IF DecodeCtrlReg(Left,Reg1) THEN
        IF DecodeGeneralReg(Right,Reg2) THEN
         BEGIN
          DAsmCode[0]:=$0440a0+(Reg2 SHL 8)+Reg1; CodeLen:=1;
         END
        ELSE
         BEGIN
          DecodeAdr(Right,MModNoImm,MSegXData+MSegYData);
          Reg3:=Ord(AdrSeg=SegYData) SHL 6;
          IF (AdrType=ModAbs) AND (AdrVal<=63) THEN
           BEGIN
            DAsmCode[0]:=$050020+(AdrVal SHL 8)+Reg3+Reg1; CodeLen:=1;
           END
          ELSE
           BEGIN
            DAsmCode[0]:=$054020+(AdrMode SHL 8)+Reg3+Reg1;
            DAsmCode[1]:=AdrVal; CodeLen:=1+AdrCnt;
           END;
         END
       ELSE IF NOT DecodeCtrlReg(Right,Reg1) THEN WrXError(1445,Right)
       ELSE
        IF DecodeGeneralReg(Left,Reg2) THEN
         BEGIN
          DAsmCode[0]:=$04c0a0+(Reg2 SHL 8)+Reg1; CodeLen:=1;
         END
        ELSE
         BEGIN
          DecodeAdr(Left,MModAll,MSegXData+MSegYData);
          Reg3:=Ord(AdrSeg=SegYData) SHL 6;
          IF (AdrType=ModAbs) AND (AdrVal<=63) THEN
           BEGIN
            DAsmCode[0]:=$058020+(AdrVal SHL 8)+Reg3+Reg1; CodeLen:=1;
           END
          ELSE IF (AdrType=ModImm) AND (AdrVal<=255) THEN
           BEGIN
            DAsmCode[0]:=$0500a0+(AdrVal SHL 8)+Reg1; CodeLen:=1;
           END
          ELSE
           BEGIN
            DAsmCode[0]:=$05c020+(AdrMode SHL 8)+Reg3+Reg1;
            DAsmCode[1]:=AdrVal; CodeLen:=1+AdrCnt;
           END;
         END;
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
       ELSE IF NOT DecodeGeneralReg(Right,Reg2) THEN WrXError(1445,Right)
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
          IF (AdrVal<=MemLimit) AND (AdrVal>=MemLimit-$3f) THEN
	   BEGIN
	    CodeLen:=1;
	    DAsmCode[0]:=$08c000+(LongInt(Ord(AdrSeg=SegYData)) SHL 16)+
			 (AdrVal AND $3f)+(Reg1 SHL 8);
           END
          ELSE IF (MomCPU>=CPU56300) AND (AdrVal<=MemLimit-$40) AND (AdrVal>=MemLimit-$7f) THEN
           BEGIN
            CodeLen:=1;
            DAsmCode[0]:=$04c000+(Ord(AdrSeg=SegYData) SHL 5)+
                         (Ord(AdrSeg=SegXData) SHL 7)+
                         (AdrVal AND $1f)+((AdrVal AND $20) SHL 1)+(Reg1 SHL 8);
           END
          ELSE WrError(1315);
	END
       ELSE IF DecodeGeneralReg(Right,Reg2) THEN
	BEGIN
	 DecodeAdr(Left,MModAbs,MSegXData+MSegYData);
	 IF AdrType<>ModNone THEN
          IF (AdrVal<=MemLimit) AND (AdrVal>=MemLimit-$3f) THEN
	   BEGIN
	    CodeLen:=1;
	    DAsmCode[0]:=$084000+(LongInt(Ord(AdrSeg=SegYData)) SHL 16)+
			 (AdrVal AND $3f)+(Reg2 SHL 8);
           END
          ELSE IF (MomCPU>=CPU56300) AND (AdrVal<=MemLimit-$40) AND (AdrVal>=MemLimit-$7f) THEN
           BEGIN
            CodeLen:=1;
            DAsmCode[0]:=$044000+(Ord(AdrSeg=SegYData) SHL 5)+
                         (Ord(AdrSeg=SegXData) SHL 7)+
                         (AdrVal AND $1f)+((AdrVal AND $20) SHL 1)+(Reg2 SHL 8);
           END
          ELSE WrError(1315);
        END
       ELSE
	BEGIN
	 DecodeAdr(Left,MModAll,MSegXData+MSegYData+MSegCode);
         IF (AdrType=ModAbs) AND (AdrSeg<>SegCode) AND (AdrVal>=MemLimit-$3f) AND (AdrVal<=MemLimit) THEN
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
         ELSE IF (AdrType=ModAbs) AND (MomCPU>=CPU56300) AND (AdrSeg<>SegCode) AND
                 (AdrVal>=MemLimit-$7f) AND (AdrVal<=MemLimit-$40) THEN
	  BEGIN
	   HVal:=AdrVal AND $3f; HSeg:=AdrSeg;
	   DecodeAdr(Right,MModNoImm,MSegXData+MSegYData+MSegCode);
	   IF AdrType<>ModNone THEN
	    IF AdrSeg=SegCode THEN
	     BEGIN
	      CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
              DAsmCode[0]:=$008000+HVal+(AdrMode SHL 8)+
                           (Ord(HSeg=SegYData) SHL 6);
	     END
	    ELSE
	     BEGIN
	      CodeLen:=1+AdrCnt; DAsmCode[1]:=AdrVal;
              DAsmCode[0]:=$070000+HVal+(AdrMode SHL 8)+
                           (Ord(HSeg=SegYData) SHL 7)+
                           (Ord(HSeg=SegXData) SHL 14)+
                           (Ord(AdrSeg=SegYData) SHL 6);
	     END;
          END
         ELSE IF AdrType<>ModNone THEN
	  BEGIN
	   HVal:=AdrVal; HCnt:=AdrCnt; HMode:=AdrMode; HType:=AdrType; HSeg:=AdrSeg;
	   DecodeAdr(Right,MModAbs,MSegXData+MSegYData);
	   IF AdrType<>ModNone THEN
            IF (AdrVal>=MemLimit-$3f) AND (AdrVal<=MemLimit) THEN
             BEGIN
              IF HSeg=SegCode THEN
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
             END
            ELSE IF (MomCPU>=CPU56300) AND (AdrVal>=MemLimit-$7f) AND (AdrVal<=MemLimit-$40) THEN
             BEGIN
              IF HSeg=SegCode THEN
               BEGIN
                CodeLen:=1+HCnt; DAsmCode[1]:=HVal;
                DAsmCode[0]:=$00c000+(AdrVal AND $3f)+(HMode SHL 8)+
                             (Ord(AdrSeg=SegYData) SHL 6);
               END
              ELSE
               BEGIN
                CodeLen:=1+HCnt; DAsmCode[1]:=HVal;
                DAsmCode[0]:=$078000+(Word(AdrVal) AND $3f)+(HMode SHL 8)+
                             (Ord(AdrSeg=SegYData) SHL 7)+
                             (Ord(AdrSeg=SegXData) SHL 14)+
                             (Ord(HSeg=SegYData) SHL 6);
               END;
             END
            ELSE WrError(1315);
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
     ELSE IF DecodeTFR(ArgStr[1],Reg1) THEN
      BEGIN
       IF ArgCnt=1 THEN
        BEGIN
         CodeLen:=1;
         DAsmCode[0]:=$020000+(Condition SHL 12)+(Reg1 SHL 3);
        END
       ELSE IF NOT DecodeRR(ArgStr[2],Reg2) THEN WrError(1350)
       ELSE
        BEGIN
         CodeLen:=1;
         DAsmCode[0]:=$030000+(Condition SHL 12)+(Reg1 SHL 3)+Reg2;
        END;
      END
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT DecodeRR(ArgStr[1],Reg1) THEN WrError(1350)
     ELSE
      BEGIN
       DAsmCode[0]:=$020800+(Condition SHL 12)+Reg1;
       CodeLen:=1;
      END;
     Exit;
    END;

   { SprÅnge }

   FOR z:=0 TO BitBrOrderCnt-1 DO
    IF (Memo(BitBrOrders[z])) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE IF MomCPU<CPU56300 THEN WrError(1500)
      ELSE
       BEGIN
        h:=(z AND 1) SHL 5;
        h2:=LongInt(z AND 2) SHL 15;
        SplitArg(ArgStr[1],Left,Right); SplitArg(Right,Mid,Right);
        IF (Left='') OR (Right='') Or (Mid='') THEN WrError(1110)
        ELSE IF Left[1]<>'#' THEN WrError(1120)
        ELSE
         BEGIN
          AddVal:=EvalIntExpression(Copy(Left,2,Length(Left)-1),Int8,OK);
          IF FirstPassUnknown THEN AddVal:=AddVal AND 15;
          IF OK THEN
           IF (AddVal<0) OR (AddVal>23) THEN WrError(1320)
           ELSE IF DecodeGeneralReg(Mid,Reg1) THEN
            BEGIN
             CodeLen:=1;
             DAsmCode[0]:=$0cc080+AddVal+(Reg1 SHL 8)+h+h2;
            END
           ELSE
            BEGIN
             FirstPassUnknown:=False;
             DecodeAdr(Mid,MModNoImm,MSegXData+MSegYData);
             Reg3:=Ord(AdrSeg=SegYData) SHL 6;
             IF (AdrType=ModAbs) AND (FirstPassUnknown) THEN AdrVal:=AdrVal AND $3f;
             IF (AdrType=ModAbs) AND (AdrVal<=63) AND (AdrVal>=0) THEN
              BEGIN
               CodeLen:=1;
               DAsmCode[0]:=$0c8080+AddVal+(AdrVal SHL 8)+Reg3+h+h2;
              END
             ELSE IF (AdrType=ModAbs) AND (AdrVal>=MemLimit-$3f) AND (AdrVal<=MemLimit) THEN
              BEGIN
               CodeLen:=1;
               DAsmCode[0]:=$0cc000+AddVal+((AdrVal AND $3f) SHL 8)+Reg3+h+h2;
              END
             ELSE IF (AdrType=ModAbs) AND (AdrVal>=MemLimit-$7f) AND (AdrVal<=MemLimit-$40) THEN
              BEGIN
               CodeLen:=1;
               DAsmCode[0]:=$048000+AddVal+((AdrVal AND $3f) SHL 8)+Reg3+h+(h2 SHR 9);
              END
             ELSE IF AdrType=ModAbs THEN WrError(1350)
             ELSE IF AdrType<>ModNone THEN
              BEGIN
               CodeLen:=1;
               DAsmCode[0]:=$0c8000+AddVal+(AdrMode SHL 8)+Reg3+h+h2;
              END;
            END;
         END;
        IF CodeLen=1 THEN
         BEGIN
          Dist:=EvalIntExpression(Right,AdrInt,OK)-(EProgCounter+2);
          IF OK THEN
           BEGIN
            DAsmCode[1]:=Dist AND $ffffff; CodeLen:=2;
           END
          ELSE CodeLen:=0;
         END;
       END;
      Exit;
     END;

   IF (Memo('BRA'))  OR (Memo('BSR')) THEN
    BEGIN
     IF Memo('BRA') THEN z:=$40 ELSE z:=0;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE IF DecodeReg(ArgStr[1],Reg1) THEN
      BEGIN
       IF (Reg1<16) OR (Reg1>23) THEN WrXError(1445,ArgStr[1])
       ELSE
        BEGIN
         Dec(Reg1,16);
         DAsmCode[0]:=$0d1880+(Reg1 SHL 8)+z;
         CodeLen:=1;
        END;
      END
     ELSE
      BEGIN
       CutSize(ArgStr[1],Size);
       Dist:=EvalIntExpression(ArgStr[1],AdrInt,OK)-(EProgCounter+1);
       IF Size=0 THEN
        IF (Dist>-256) AND (Dist<255) THEN Size:=1 ELSE Size:=2;
       CASE Size OF
       1:IF (NOT SymbolQuestionable) AND ((Dist<-256) OR (Dist>255)) THEN WrError(1370)
         ELSE
          BEGIN
           Dist:=Dist AND $1ff;
           DAsmCode[0]:=$050800+(z SHL 4)+((Dist AND $1e0) SHL 1)+(Dist AND $1f);
           CodeLen:=1;
          END;
       2:BEGIN
          Dec(Dist);
          DAsmCode[0]:=$0d1080+z;
          DAsmCode[1]:=Dist AND $ffffff;
          CodeLen:=2;
         END;
       END;
      END;
     Exit;
    END;

   IF (OpPart[1]='B') AND (DecodeCondition(Copy(OpPart,2,Length(OpPart)-1),Condition)) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE IF DecodeReg(ArgStr[1],Reg1) THEN
      BEGIN
       IF (Reg1<16) OR (Reg1>23) THEN WrXError(1445,ArgStr[1])
       ELSE
        BEGIN
         Dec(Reg1,16);
         DAsmCode[0]:=$0d1840+(Reg1 SHL 8)+Condition;
         CodeLen:=1;
        END;
      END
     ELSE
      BEGIN
       CutSize(ArgStr[1],Size);
       Dist:=EvalIntExpression(ArgStr[1],AdrInt,OK)-(EProgCounter+1);
       IF Size=0 THEN
        IF (Dist>-256) AND (Dist<255) THEN Size:=1 ELSE Size:=2;
       CASE Size OF
       1:IF (NOT SymbolQuestionable) AND ((Dist<-256) OR (Dist>255)) THEN WrError(1370)
         ELSE
          BEGIN
           Dist:=Dist AND $1ff;
           DAsmCode[0]:=$050400+(Condition SHL 12)+((Dist AND $1e0) SHL 1)+(Dist AND $1f);
           CodeLen:=1;
          END;
       2:BEGIN
          Dec(Dist);
          DAsmCode[0]:=$0d1040+Condition;
          DAsmCode[1]:=Dist AND $ffffff;
          CodeLen:=2;
         END;
       END;
      END;
     Exit;
    END;

   IF (Copy(OpPart,1,2)='BS') AND (DecodeCondition(Copy(OpPart,3,Length(OpPart)-2),Condition)) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE IF DecodeReg(ArgStr[1],Reg1) THEN
      BEGIN
       IF (Reg1<16) OR (Reg1>23) THEN WrXError(1445,ArgStr[1])
       ELSE
        BEGIN
         Dec(Reg1,16);
         DAsmCode[0]:=$0d1800+(Reg1 SHL 8)+Condition;
         CodeLen:=1;
        END;
      END
     ELSE
      BEGIN
       CutSize(ArgStr[1],Size);
       Dist:=EvalIntExpression(ArgStr[1],AdrInt,OK)-(EProgCounter+1);
       IF Size=0 THEN
        IF (Dist>-256) AND (Dist<255) THEN Size:=1 ELSE Size:=2;
       CASE Size OF
       1:IF (NOT SymbolQuestionable) AND ((Dist<-256) OR (Dist>255)) THEN WrError(1370)
         ELSE
          BEGIN
           Dist:=Dist AND $1ff;
           DAsmCode[0]:=$050000+(Condition SHL 12)+((Dist AND $1e0) SHL 1)+(Dist AND $1f);
           CodeLen:=1;
          END;
       2:BEGIN
          Dec(Dist);
          DAsmCode[0]:=$0d1000+Condition;
          DAsmCode[1]:=Dist AND $ffffff;
          CodeLen:=2;
         END;
       END;
      END;
     Exit;
    END;

   IF (Memo('LUA')) OR (Memo('LEA')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF (Left='') OR (Right='') THEN WrError(1110)
       ELSE IF NOT DecodeReg(Right,Reg1) THEN WrXError(1445,Right)
       ELSE IF Reg1>31 THEN WrXError(1445,Right)
       ELSE
	BEGIN
         DecodeAdr(Left,MModModInc+MModModDec+MModPostInc+MModPostDec+MModDisp,MSegXData);
         IF AdrType=ModDisp THEN
          BEGIN
           IF ChkRange(AdrVal,-64,63) THEN
            BEGIN
             AdrVal:=AdrVal AND $7f;
             DAsmCode[0]:=$040000+(Reg1-16)+(AdrMode SHL 8)+
                          ((AdrVal AND $0f) SHL 4)+
                          ((AdrVal AND $70) SHL 7);
             CodeLen:=1;
            END;
          END
         ELSE IF AdrType<>ModNone THEN
          BEGIN
           CodeLen:=1;
           DAsmCode[0]:=$044000+(AdrMode SHL 8)+Reg1;
          END;
        END
      END;
     Exit;
    END;

   IF Memo('LRA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       SplitArg(ArgStr[1],Left,Right);
       IF Right='' THEN WrError(1110)
       ELSE IF NOT DecodeGeneralReg(Right,Reg1) THEN WrXError(1445,Right)
       ELSE IF Reg1>$1f THEN WrXError(1445,Right)
       ELSE IF DecodeGeneralReg(Left,Reg2) THEN
        BEGIN
         IF (Reg2<16) OR (Reg2>23) THEN WrXError(1445,Left)
         ELSE
          BEGIN
           DasmCode[0]:=$04c000+((Reg2 AND 7) SHL 8)+Reg1;
           CodeLen:=1;
          END;
        END
       ELSE
        BEGIN
         DAsmCode[1]:=EvalIntExpression(Left,AdrInt,OK)-(EProgCounter+2);
         IF OK THEN
          BEGIN
           DAsmCode[0]:=$044040+Reg1;
           CodeLen:=2;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('PLOCK') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModNoImm,MSegCode);
       IF AdrType<>ModNone THEN
        BEGIN
         DAsmCode[0]:=$0ac081+(AdrMode SHL 8); DAsmCode[1]:=AdrVal;
         CodeLen:=2;
        END;
      END;
     Exit;
    END;

   IF (Memo('PLOCKR')) OR (Memo('PUNLOCKR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       DAsmCode[1]:=(EvalIntExpression(ArgStr[1],AdrInt,OK)-(EProgCounter+2)) AND $ffffff;
       IF OK THEN
        BEGIN
         DAsmCode[0]:=$00000e+Ord(Memo('PLOCKR')); CodeLen:=2;
        END;
      END;
     Exit;
    END;

   { SprÅnge }

   IF DecodeJump THEN Exit;

   IF (Copy(OpPart,1,5)='DEBUG') AND (DecodeCondition(Copy(OpPart,6,Length(OpPart)-5),Condition)) THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU<CPU56300 THEN WrError(1500)
     ELSE
      BEGIN
       DAsmCode[0]:=$00000300+Condition;
       CodeLen:=1;
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
   SegYData : ok:=ProgCounter <=MemLimit;
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

   IF MomCPU=CPU56300 THEN
    BEGIN
     MemLimit:=$ffffff; AdrInt:=UInt24;
    END
   ELSE
    BEGIN
     MemLimit:=$ffff; AdrInt:=UInt16;
    END;
END;

BEGIN
   CPU56000:=AddCPU('56000',SwitchTo_56K);
   CPU56002:=AddCPU('56002',SwitchTo_56K);
   CPU56300:=AddCPU('56300',SwitchTo_56K);
END.
