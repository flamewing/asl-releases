{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

	UNIT Code96;

INTERFACE
        Uses AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[7];
	       Code:Byte;
	      END;

   BaseOrder=RECORD
	      Name:String[5];
	      Code:Byte;
              MinCPU,MaxCPU:CPUVar;
	     END;

   MacOrder=RECORD
             Name:String[6];
             Code:Byte;
             Reloc:Boolean;
            END;

CONST
   FixedOrderCnt=16;
   MacOrderCnt=8;
   RptOrderCnt=34;

TYPE
   FixedOrderArray=ARRAY[1..FixedOrderCnt] OF BaseOrder;
   MacOrderArray=ARRAY[0..MacOrderCnt-1] OF MacOrder;
   RptOrderArray=ARRAY[0..RptOrderCnt-1] OF FixedOrder;

CONST
   RelOrderCnt=16;
   RelOrders:ARRAY[1..RelOrderCnt] OF FixedOrder=
             ((Name:'JC'   ; Code:$db),
              (Name:'JE'   ; Code:$df),
	      (Name:'JGE'  ; Code:$d6),
              (Name:'JGT'  ; Code:$d2),
              (Name:'JH'   ; Code:$d9),
              (Name:'JLE'  ; Code:$da),
              (Name:'JLT'  ; Code:$de),
	      (Name:'JNC'  ; Code:$d3),
              (Name:'JNE'  ; Code:$d7),
              (Name:'JNH'  ; Code:$d1),
	      (Name:'JNST' ; Code:$d0),
              (Name:'JNV'  ; Code:$d5),
              (Name:'JNVT' ; Code:$d4),
              (Name:'JST'  ; Code:$d8),
              (Name:'JV'   ; Code:$dd),
              (Name:'JVT'  ; Code:$dc));

   ALU3OrderCnt=5;
   ALU3Orders:ARRAY[1..ALU3OrderCnt] OF FixedOrder=
	      ((Name:'ADD' ; Code:$01),
	       (Name:'AND' ; Code:$00),
	       (Name:'MUL' ; Code:$83),   (****)
	       (Name:'MULU'; Code:$03),
	       (Name:'SUB' ; Code:$02));

   ALU2OrderCnt=9;
   ALU2Orders:ARRAY[1..ALU2OrderCnt] OF FixedOrder=
	      ((Name:'ADDC'; Code:$a4),
	       (Name:'CMP' ; Code:$88),
	       (Name:'DIV' ; Code:$8c),   (****)
	       (Name:'DIVU'; Code:$8c),
	       (Name:'LD'  ; Code:$a0),
	       (Name:'OR'  ; Code:$80),
	       (Name:'ST'  ; Code:$c0),
	       (Name:'SUBC'; Code:$a8),
	       (Name:'XOR' ; Code:$84));

   ALU1OrderCnt=6;
   ALU1Orders:ARRAY[1..ALU1OrderCnt] OF FixedOrder=
	      ((Name:'CLR'; Code:$01),
	       (Name:'DEC'; Code:$05),
	       (Name:'EXT'; Code:$06),
	       (Name:'INC'; Code:$07),
	       (Name:'NEG'; Code:$03),
	       (Name:'NOT'; Code:$02));

   ShiftOrderCnt=3;
   ShiftOrders:ARRAY[0..ShiftOrderCnt-1] OF String[4]=('SHR','SHL','SHRA');

CONST
   ModNone=-1;
   ModDir=0;   MModDir=1 SHL ModDir;
   ModMem=1;   MModMem=1 SHL ModMem;
   ModImm=2;   MModImm=1 SHL ModImm;

   SFRStart=2; SFRStop=$17;

VAR
   FixedOrders:^FixedOrderArray;
   MacOrders:^MacOrderArray;
   RptOrders:^RptOrderArray;

   CPU8096,CPU80196,CPU80196N,CPU80296:CPUVar;
   SaveInitProc:PROCEDURE;

   AdrMode:Byte;
   AdrType:ShortInt;
   AdrVals:ARRAY[0..3] OF Byte;
   AdrCnt:Byte;
   OpSize:ShortInt;

   WSRVal,WSR1Val:LongInt;
   WinStart,WinStop,WinBegin,WinEnd:Word;
   Win1Start,Win1Stop,Win1Begin,Win1End:Word;

   MemInt:IntType;


   	PROCEDURE InitFields;
VAR
   z:Integer;

   	PROCEDURE AddFixed(NName:String; NCode:Byte; NMin,NMax:CPUVar);
BEGIN
   IF z>FixedOrderCnt THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NMin; MaxCPU:=NMax;
    END;
   Inc(z);
END;

        PROCEDURE AddMac(NName:String; NCode:Byte; NRel:Boolean);
BEGIN
   IF z>=MacOrderCnt THEN Halt(255);
   WITH MacOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Reloc:=NRel;
    END;
   Inc(z);
END;

        PROCEDURE AddRpt(NName:String; NCode:Byte);
BEGIN
   IF z>=RptOrderCnt THEN Halt(255);
   WITH RptOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(FixedOrders); z:=1;
   AddFixed('CLRC' ,$f8,CPU8096  ,CPU80296 );
   AddFixed('CLRVT',$fc,CPU8096  ,CPU80296 );
   AddFixed('DI'   ,$fa,CPU8096  ,CPU80296 );
   AddFixed('DPTS' ,$ea,CPU80196 ,CPU80196N);
   AddFixed('EI'   ,$fb,CPU8096  ,CPU80296 );
   AddFixed('EPTS' ,$eb,CPU80196 ,CPU80196N);
   AddFixed('NOP'  ,$fd,CPU8096  ,CPU80296 );
   AddFixed('POPA' ,$f5,CPU80196 ,CPU80296 );
   AddFixed('POPF' ,$f3,CPU8096  ,CPU80296 );
   AddFixed('PUSHA',$f4,CPU80196 ,CPU80296 );
   AddFixed('PUSHF',$f2,CPU8096  ,CPU80296 );
   AddFixed('RET'  ,$f0,CPU8096  ,CPU80296 );
   AddFixed('RSC'  ,$ff,CPU8096  ,CPU80296 );
   AddFixed('SETC' ,$f9,CPU8096  ,CPU80296 );
   AddFixed('TRAP' ,$f7,CPU8096  ,CPU80296 );
   AddFixed('RETI' ,$e5,CPU80296 ,CPU80296 );

   New(MacOrders); z:=0;
   AddMac('MAC'   ,$00,False); AddMac('SMAC'  ,$01,False);
   AddMac('MACR'  ,$04,True ); AddMac('SMACR' ,$05,True );
   AddMac('MACZ'  ,$08,False); AddMac('SMACZ' ,$09,False);
   AddMac('MACRZ' ,$0c,True ); AddMac('SMACRZ',$0d,True );

   New(RptOrders); z:=0;
   AddRpt('RPT'    ,$00); AddRpt('RPTNST' ,$10); AddRpt('RPTNH'  ,$11);
   AddRpt('RPTGT'  ,$12); AddRpt('RPTNC'  ,$13); AddRpt('RPTNVT' ,$14);
   AddRpt('RPTNV'  ,$15); AddRpt('RPTGE'  ,$16); AddRpt('RPTNE'  ,$17);
   AddRpt('RPTST'  ,$18); AddRpt('RPTH'   ,$19); AddRpt('RPTLE'  ,$1a);
   AddRpt('RPTC'   ,$1b); AddRpt('RPTVT'  ,$1c); AddRpt('RPTV'   ,$1d);
   AddRpt('RPTLT'  ,$1e); AddRpt('RPTE'   ,$1f); AddRpt('RPTI'   ,$20);
   AddRpt('RPTINST',$30); AddRpt('RPTINH' ,$31); AddRpt('RPTIGT' ,$32);
   AddRpt('RPTINC' ,$33); AddRpt('RPTINVT',$34); AddRpt('RPTINV' ,$35);
   AddRpt('RPTIGE' ,$36); AddRpt('RPTINE' ,$37); AddRpt('RPTIST' ,$38);
   AddRpt('RPTIH'  ,$39); AddRpt('RPTILE' ,$3a); AddRpt('RPTIC'  ,$3b);
   AddRpt('RPTIVT' ,$3c); AddRpt('RPTIV'  ,$3d); AddRpt('RPTILT' ,$3e);
   AddRpt('RPTIE'  ,$3f);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(MacOrders);
   Dispose(RptOrders);
END;

{---------------------------------------------------------------------------}

	PROCEDURE ChkSFR(Adr:Word);
BEGIN
   IF (Adr>=SFRStart) AND (Adr<=SFRStop) THEN WrError(190);
END;

        PROCEDURE Chk296(Adr:Word);
BEGIN
   IF (MomCPU=CPU80296) AND (Adr<=1) THEN WrError(190);
END;

	FUNCTION ChkWork(VAR Adr:Word):Boolean;
BEGIN
   { Registeradresse, die von Fenstern Åberdeckt wird ? }

   IF (Adr>=WinBegin) AND (Adr<=WinEnd) THEN ChkWork:=False

   ELSE IF (Adr>=Win1Begin) AND (Adr<=Win1End) THEN ChkWork:=False

   { Speicheradresse in Fenster ? }

   ELSE IF (Adr>=WinStart) AND (Adr<=WinStop) THEN
    BEGIN
     ChkWork:=True; Adr:=Adr-WinStart+WinBegin;
    END

   ELSE IF (Adr>=Win1Start) AND (Adr<=Win1Stop) THEN
    BEGIN
     ChkWork:=True; Adr:=Adr-Win1Start+Win1Begin;
    END

   { Default }

   ELSE ChkWork:=Adr<=$ff;
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:Byte; AddrWide:Boolean);
LABEL
   AdrFound;
VAR
   AdrInt:LongInt;
   AdrWord:LongInt;
   BReg:Word;
   OK:Boolean;
   p,p2:Integer;
   Reg:Byte;
   OMask:LongInt;
BEGIN
   AdrType:=ModNone; AdrCnt:=0;
   OMask:=(1 SHL OpSize)-1;

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize OF
     -1:WrError(1132);
     0:BEGIN
	AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
	IF OK THEN
	 BEGIN
	  AdrType:=ModImm; AdrCnt:=1; AdrMode:=1;
	 END;
       END;
     1:BEGIN
	AdrWord:=EvalIntExpression(Asc,Int16,OK);
	IF OK THEN
	 BEGIN
	  AdrType:=ModImm; AdrCnt:=2; AdrMode:=1;
	  AdrVals[0]:=Lo(AdrWord); AdrVals[1]:=Hi(AdrWord);
	 END;
       END;
     END;
     Goto AdrFound;
    END;

   p:=QuotPos(Asc,'[');
   IF p<=Length(Asc) THEN
    BEGIN
     p2:=RQuotPos(Asc,']');
     IF (p2>Length(Asc)) OR (p2<Length(Asc)-1) OR (p2<p) THEN WrError(1350)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       BReg:=EvalIntExpression(Copy(Asc,p+1,p2-p-1),Int16,OK);
       IF FirstPassUnknown THEN BReg:=0;
       IF OK THEN
        IF NOT ChkWork(BReg) THEN WrError(1320)
        ELSE
	 BEGIN
	  Reg:=Lo(BReg); ChkSFR(Reg);
	  IF Odd(Reg) THEN WrError(1351)
	  ELSE IF (p=1) AND (p2=Length(Asc)-1) AND (Asc[Length(Asc)]='+') THEN
	   BEGIN
	    AdrType:=ModMem; AdrMode:=2; AdrCnt:=1; AdrVals[0]:=Reg+1;
	   END
	  ELSE IF p2<>Length(Asc) THEN WrError(1350)
	  ELSE IF p=1 THEN
	   BEGIN
	    AdrType:=ModMem; AdrMode:=2; AdrCnt:=1; AdrVals[0]:=Reg;
	   END
	  ELSE
	   BEGIN
	    IF NOT AddrWide THEN AdrInt:=EvalIntExpression(Copy(Asc,1,p-1),Int16,OK)
            ELSE AdrInt:=EvalIntExpression(Copy(Asc,1,p-1),Int24,OK);
	    IF OK THEN
	     IF AdrInt=0 THEN
	      BEGIN
	       AdrType:=ModMem; AdrMode:=2; AdrCnt:=1; AdrVals[0]:=Reg;
	      END
             ELSE IF AddrWide THEN
              BEGIN
               AdrType:=ModMem; AdrMode:=3; AdrCnt:=4;
               AdrVals[0]:=Reg; AdrVals[1]:=AdrInt AND $ff;
               AdrVals[2]:=(AdrInt SHR 8) AND $ff;
               AdrVals[3]:=(AdrInt SHR 16) AND $ff;
              END
	     ELSE IF (AdrInt>=-128) AND (AdrInt<127) THEN
	      BEGIN
	       AdrType:=ModMem; AdrMode:=3; AdrCnt:=2;
	       AdrVals[0]:=Reg; AdrVals[1]:=Lo(AdrInt);
	      END
	     ELSE
	      BEGIN
	       AdrType:=ModMem; AdrMode:=3; AdrCnt:=3;
	       AdrVals[0]:=Reg+1; AdrVals[1]:=Lo(AdrInt); AdrVals[2]:=Hi(AdrInt);
	      END;
	   END;
	 END;
      END;
    END
   ELSE
    BEGIN
     FirstPassUnknown:=False;
     AdrWord:=EvalIntExpression(Asc,MemInt,OK);
     IF FirstPassUnknown THEN AdrWord:=AdrWord AND (NOT OMask);
     IF OK THEN
      IF AdrWord AND OMask<>0 THEN WrError(1325)
      ELSE
       BEGIN
        BReg:=AdrWord AND $ffff;
        IF (BReg AND $ffff0000=0) AND (ChkWork(BReg)) THEN
         BEGIN
          AdrType:=ModDir; AdrCnt:=1; AdrVals[0]:=Lo(BReg);
         END
        ELSE IF AddrWide THEN
         BEGIN
          AdrType:=ModMem; AdrMode:=3; AdrCnt:=4; AdrVals[0]:=0;
          AdrVals[1]:=AdrWord AND $ff;
          AdrVals[2]:=(AdrWord SHR 8) AND $ff;
          AdrVals[3]:=(AdrWord SHR 16) AND $ff;
         END
        ELSE IF AdrWord>=$ff80 THEN
         BEGIN
          AdrType:=ModMem; AdrMode:=3; AdrCnt:=2; AdrVals[0]:=0;
          AdrVals[1]:=Lo(AdrWord);
         END
        ELSE
         BEGIN
          AdrType:=ModMem; AdrMode:=3; AdrCnt:=3; AdrVals[0]:=1;
          AdrVals[1]:=Lo(AdrWord); AdrVals[2]:=Hi(AdrWord);
         END;
       END;
    END;

AdrFound:
   IF (AdrType=ModDir) AND (Mask AND MModDir=0) THEN
    BEGIN
     AdrType:=ModMem; AdrMode:=0;
    END;

   IF (AdrType<>ModNone) AND (Mask AND (1 SHL AdrType)=0) THEN
    BEGIN
     WrError(1350); AdrType:=ModNone; AdrCnt:=0;
    END;
END;

	PROCEDURE CalcWSRWindow;
BEGIN
   IF WSRVal<=$0f THEN
    BEGIN
     WinStart:=$ffff; WinStop:=0; WinBegin:=$ff; WinEnd:=0;
    END
   ELSE IF WSRVal<=$1f THEN
    BEGIN
     WinBegin:=$80; WinEnd:=$ff;
     IF WSRVal<$18 THEN WinStart:=(WSRVal-$10) SHL 7
     ELSE WinStart:=(WSRVal+$20) SHL 7;
     WinStop:=WinStart+$7f;
    END
   ELSE IF WSRVal<=$3f THEN
    BEGIN
     WinBegin:=$c0; WinEnd:=$ff;
     IF WSRVal<$30 THEN WinStart:=(WSRVal-$20) SHL 6
     ELSE WinStart:=(WSRVal+$40) SHL 6;
     WinStop:=WinStart+$3f;
    END
   ELSE IF WSRVal<=$7f THEN
    BEGIN
     WinBegin:=$e0; WinEnd:=$ff;
     IF WSRVal<$60 THEN WinStart:=(WSRVal-$40) SHL 5
     ELSE WinStart:=(WSRVal+$80) SHL 5;
     WinStop:=WinStart+$1f;
    END;
   IF (WinStop>$1fdf) AND (MomCPU<CPU80296) THEN WinStop:=$1fdf;
END;

	PROCEDURE CalcWSR1Window;
BEGIN
   IF WSR1Val<=$1f THEN
    BEGIN
     Win1Start:=$ffff; Win1Stop:=0; Win1Begin:=$ff; Win1End:=0;
    END
   ELSE IF WSR1Val<=$3f THEN
    BEGIN
     Win1Begin:=$40; Win1End:=$7f;
     IF WSR1Val<$30 THEN Win1Start:=(WSR1Val-$20) SHL 6
     ELSE Win1Start:=(WSR1Val+$40) SHL 6;
     Win1Stop:=Win1Start+$3f;
    END
   ELSE IF WSR1Val<=$7f THEN
    BEGIN
     Win1Begin:=$60; Win1End:=$7f;
     IF WSR1Val<$60 THEN Win1Start:=(WSR1Val-$40) SHL 5
     ELSE Win1Start:=(WSR1Val+$80) SHL 5;
     Win1Stop:=Win1Start+$1f;
    END
   ELSE
    BEGIN
     Win1Begin:=$40; Win1End:=$7f;
     Win1Start:=(WSR1Val+$340) SHL 6;
     Win1Stop:=Win1Start+$3f;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME96Count=2;
   ASSUME96s:ARRAY[1..ASSUME96Count] OF ASSUMERec=
	     ((Name:'WSR' ; Dest:@WSRVal ; Min:0; Max:$ff; NothingVal:$00),
	      (Name:'WSR1'; Dest:@WSR1Val; Min:0; Max:$bf; NothingVal:$00));
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     IF MomCPU<CPU80196 THEN WrError(1500)
     ELSE IF MomCPU<CPU80296 THEN CodeASSUME(@ASSUME96s,1)
     ELSE CodeASSUME(@ASSUME96s,ASSUME96Count);
     WSRVal:=WSRVal AND $7f;
     CalcWSRWindow; CalcWSR1Window;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE ChkAlign(Adr:Byte);
BEGIN
   IF ((OpSize=0) AND (Adr AND 1<>0))
   OR ((OpSize=1) AND (Adr AND 3<>0)) THEN WrError(180);
END;

	FUNCTION BMemo(Name:String):Boolean;
BEGIN
   IF Memo(Name) THEN
    BEGIN
     BMemo:=True; OpSize:=1;
    END
   ELSE IF Memo(Name+'B') THEN
    BEGIN
     BMemo:=True; OpSize:=0;
    END
   ELSE BMemo:=False;
END;

	FUNCTION LMemo(Name:String):Boolean;
BEGIN
   IF BMemo(Name) THEN LMemo:=True
   ELSE IF Memo(Name+'L') THEN
    BEGIN
     LMemo:=True; OpSize:=2;
    END
   ELSE LMemo:=False;
END;

	PROCEDURE MakeCode_96;
	Far;
VAR
   OK,Special,IsShort:Boolean;
   AdrWord:Word;
   z:Integer;
   AdrInt:LongInt;
   Start,HReg,Mask:Byte;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF (MomCPU<MinCPU) OR (MomCPU>MaxCPU) THEN WrError(1500)
       ELSE
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=Code;
	END;
       Exit;
      END;

   { Arithmetik }

   FOR z:=1 TO ALU3OrderCnt DO
    WITH ALU3Orders[z] DO
     IF BMemo(Name) THEN
      BEGIN
       IF (ArgCnt<>2) AND (ArgCnt<>3) THEN WrError(1110)
       ELSE
	BEGIN
	 Start:=0; Special:=(Name='MUL') OR (Name='MULU');
	 IF Code AND $80<>0 THEN
	  BEGIN
	   BAsmCode[Start]:=$fe; Inc(Start);
	  END;
	 BAsmCode[Start]:=$40+(Ord(ArgCnt=2) SHL 5)
			     +((1-OpSize) SHL 4)
			     +((Code AND $7f) SHL 2);
	 Inc(Start);
	 DecodeAdr(ArgStr[ArgCnt],MModImm+MModMem,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   Inc(BAsmCode[Start-1],AdrMode);
	   Move(AdrVals,BAsmCode[Start],AdrCnt); Inc(Start,AdrCnt);
	   IF (Special) AND (AdrMode=0) THEN ChkSFR(AdrVals[0]);
	   IF ArgCnt=3 THEN
	    BEGIN
	     DecodeAdr(ArgStr[2],MModDir,False);
	     OK:=AdrType<>ModNone;
	     IF OK THEN
	      BEGIN
	       BAsmCode[Start]:=AdrVals[0]; Inc(Start);
	       IF Special THEN ChkSFR(AdrVals[0]);
	      END;
	    END
	   ELSE OK:=True;
	   IF OK THEN
	    BEGIN
	     DecodeAdr(ArgStr[1],MModDir,False);
	     IF AdrType<>ModNone THEN
	      BEGIN
	       BAsmCode[Start]:=AdrVals[0]; CodeLen:=Start+1;
	       IF Special THEN
		BEGIN
                 ChkSFR(AdrVals[0]);
                 Chk296(AdrVals[0]);
                 ChkAlign(AdrVals[0]);
		END;
	      END;
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO ALU2OrderCnt DO
    WITH ALU2Orders[z] DO
     IF BMemo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 Start:=0; Special:=(Name='DIV') OR (Name='DIVU');
	 IF Name='DIV' THEN
	  BEGIN
	   BAsmCode[Start]:=$fe; Inc(Start);
	  END;
	 HReg:=(1+Ord(Name<>'ST')) SHL 1;
	 BAsmCode[Start]:=Code+((1-OpSize) SHL HReg); Inc(Start);
	 Mask:=MModMem; IF NOT BMemo('ST') THEN Inc(Mask,MModImm);
	 DecodeAdr(ArgStr[2],Mask,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   Inc(BAsmCode[Start-1],AdrMode);
	   Move(AdrVals,BAsmCode[Start],AdrCnt); Inc(Start,AdrCnt);
	   IF (Special) AND (AdrMode=0) THEN ChkSFR(AdrVals[0]);
	   DecodeAdr(ArgStr[1],MModDir,False);
	   IF AdrType<>ModNone THEN
	    BEGIN
	     BAsmCode[Start]:=AdrVals[0]; CodeLen:=1+Start;
	     IF Special THEN
	      BEGIN
	       ChkSFR(AdrVals[0]); ChkAlign(AdrVals[0]);
	      END;
	    END;
	  END;
	END;
       Exit;
      END;

   IF Memo('CMPL') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80196 THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=2;
       DecodeAdr(ArgStr[1],MModDir,False);
       IF AdrType<>ModNone THEN
        BEGIN
         BAsmCode[2]:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModDir,False);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[1]:=AdrVals[0]; BAsmCode[0]:=$c5; CodeLen:=3;
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     OpSize:=1;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       Mask:=MModMem; IF Memo('PUSH') THEN Inc(Mask,MModImm);
       DecodeAdr(ArgStr[1],Mask,False);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=1+AdrCnt;
	 BAsmCode[0]:=$c8+AdrMode+(Ord(Memo('POP')) SHL 2);
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	END;
      END;
     Exit;
    END;

   IF (Memo('BMOV')) OR (Memo('BMOVI') OR (Memo('EBMOVI'))) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80196 THEN WrError(1500)
     ELSE IF (MomCPU<CPU80196) AND (Memo('EBMOVI')) THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=2; DecodeAdr(ArgStr[1],MModDir,False);
       IF AdrType<>ModNone THEN
        BEGIN
         BAsmCode[2]:=AdrVals[0];
         OpSize:=1; DecodeAdr(ArgStr[2],MModDir,False);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[1]:=AdrVals[0];
           IF Memo('BMOVI') THEN BAsmCode[0]:=$ad
	   ELSE IF Memo('BMOV') THEN BAsmCode[0]:=$c1
           ELSE BAsmCode[0]:=$e4;
           CodeLen:=3;
          END;
        END;
      END;
     Exit;
    END;

   FOR z:=1 TO ALU1OrderCnt DO
    WITH ALU1Orders[z] DO
     IF BMemo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModDir,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=1+AdrCnt;
	   BAsmCode[0]:=Code+((1-OpSize) SHL 4);
	   Move(AdrVals,BAsmCode[1],AdrCnt);
	   IF BMemo('EXT') THEN ChkAlign(AdrVals[0]);
	  END;
	END;
       Exit;
      END;

   IF BMemo('XCH') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80196 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModMem+MModDir,False);
       CASE AdrType OF
       ModMem:
        IF AdrMode=1 THEN WrError(1350)
        ELSE
         BEGIN
          Move(AdrVals,BAsmCode[1],AdrCnt); HReg:=AdrCnt;
          BAsmCode[0]:=$04+((1-OpSize) SHL 4)+AdrMode;
          DecodeAdr(ArgStr[2],MModDir,False);
          IF AdrType<>ModNone THEN
           BEGIN
            BAsmCode[1+HReg]:=AdrVals[0]; CodeLen:=2+HReg;
           END
         END;
       ModDir:
        BEGIN
         HReg:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModMem,False);
         IF AdrType<>ModNone THEN
          IF AdrMode=1 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$04+((1-OpSize) SHL 4)+AdrMode;
            Move(AdrVals,BAsmCode[1],AdrCnt);
            BAsmCode[1+AdrCnt]:=HReg; CodeLen:=2+AdrCnt;
           END;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('LDBZE')) OR (Memo('LDBSE')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=0;
       DecodeAdr(ArgStr[2],MModMem+MModImm,False);
       IF AdrType<>ModNone THEN
	BEGIN
	 BAsmCode[0]:=$ac+(Ord(Memo('LDBSE')) SHL 4)+AdrMode;
	 Move(AdrVals,BAsmCode[1],AdrCnt); Start:=1+AdrCnt;
	 OpSize:=1; DecodeAdr(ArgStr[1],MModDir,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[Start]:=AdrVals[0]; CodeLen:=1+Start;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('NORML') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(ArgStr[2],MModDir,False);
       IF AdrType<>ModNone THEN
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 OpSize:=1; DecodeAdr(ArgStr[1],MModDir,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$0f; BAsmCode[2]:=AdrVals[0];
	   ChkAlign(AdrVals[0]);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('IDLPD') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU80196 THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(ArgStr[1],MModImm,False);
       IF AdrType<>ModNone THEN
        BEGIN
         CodeLen:=2; BAsmCode[0]:=$f6; BAsmCode[1]:=AdrVals[0];
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO ShiftOrderCnt-1 DO
    IF LMemo(ShiftOrders[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],MModDir,False);
	IF AdrType<>ModNone THEN
	 BEGIN
	  BAsmCode[0]:=$08+z+(Ord(OpSize=0) SHL 4)+(Ord(OpSize=2) SHL 2);
	  BAsmCode[2]:=AdrVals[0];
	  OpSize:=0; DecodeAdr(ArgStr[2],MModDir+MModImm,False);
	  IF AdrType<>ModNone THEN
	   IF (AdrType=ModImm) AND (AdrVals[0]>15) THEN WrError(1320)
	   ELSE IF (AdrType=ModDir) AND (AdrVals[0]<16) THEN WrError(1315)
	   ELSE
	    BEGIN
	     BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	    END;
	 END;
       END;
      Exit;
     END;

   IF Memo('SKIP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(ArgStr[1],MModDir,False);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=0; BAsmCode[1]:=AdrVals[0];
	END;
      END;
     Exit;
    END;

   IF (BMemo('ELD')) OR (BMemo('EST')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80196N THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModMem,True);
       IF AdrType=ModMem THEN
        IF (AdrMode=2) AND (Odd(AdrVals[0])) THEN WrError(1350) { kein Autoincr. }
        ELSE
         BEGIN
          BAsmCode[0]:=(AdrMode AND 1)+((1-OpSize) SHL 1);
          IF OpPart[2]='L' THEN Inc(BAsmCode[0],$e8)
	                   ELSE Inc(BAsmCode[0],$1c);
          Move(AdrVals,BAsmCode[1],AdrCnt); HReg:=1+AdrCnt;
          DecodeAdr(ArgStr[1],MModDir,False);
          IF AdrType=ModDir THEN
           BEGIN
            BAsmCode[HReg]:=AdrVals[0]; CodeLen:=HReg+1;
           END;
         END;
      END;
     Exit;
    END;

   FOR z:=0 TO MacOrderCnt-1 DO
    WITH MacOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
       ELSE IF MomCPU<CPU80296 THEN WrError(1500)
       ELSE
        BEGIN
         OpSize:=1; BAsmCode[0]:=$4c+(Ord(ArgCnt=1) SHL 5);
         IF Reloc THEN DecodeAdr(ArgStr[ArgCnt],MModMem,False)
         ELSE DecodeAdr(ArgStr[ArgCnt],MModMem+MModImm,False);
         IF AdrType<>ModNone THEN
          BEGIN
           Inc(BAsmCode[0],AdrMode);
           Move(AdrVals,BAsmCode[1],AdrCnt); HReg:=1+AdrCnt;
           IF ArgCnt=2 THEN
            BEGIN
             DecodeAdr(ArgStr[1],MModDir,False);
             IF AdrType=ModDir THEN
              BEGIN
               BAsmCode[HReg]:=AdrVals[0]; Inc(HReg);
              END;
            END;
           IF AdrType<>ModNone THEN
            BEGIN
             BAsmCode[HReg]:=Code; CodeLen:=1+HReg;
            END;
          END;
        END;
       Exit;
      END;

   IF (Memo('MVAC')) OR (Memo('MSAC')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80296 THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=2; DecodeAdr(ArgStr[1],MModDir,False);
       IF AdrType=ModDir THEN
        BEGIN
         BAsmCode[0]:=$0d; BAsmCode[2]:=AdrVals[0]+1+(Ord(Memo('MSAC')) SHL 1);
         OpSize:=0; DecodeAdr(ArgStr[2],MModImm+MModDir,False);
	 BAsmCode[1]:=AdrVals[0];
         CASE AdrType OF
         ModImm:
          IF AdrVals[0]>31 THEN WrError(1320) ELSE CodeLen:=3;
         ModDir:
          IF AdrVals[0]<32 THEN WrError(1315) ELSE CodeLen:=3;
         END;
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO RptOrderCnt-1 DO
    WITH RptOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF MomCPU<CPU80296 THEN WrError(1500)
       ELSE
        BEGIN
         OpSize:=1; DecodeAdr(ArgStr[1],MModImm+MModMem,False);
         IF AdrType<>ModNone THEN
          IF AdrMode=3 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$40+AdrMode;
            Move(AdrVals,BAsmCode[1],AdrCnt);
            BAsmCode[1+AdrCnt]:=Code;
            BAsmCode[2+AdrCnt]:=4;
            CodeLen:=3+AdrCnt;
           END;
        END;
       Exit;
      END;

   { SprÅnge }

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
         AdrInt:=EvalIntExpression(ArgStr[1],MemInt,OK)-(EProgCounter+2);
	 IF OK THEN
	  IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=Code; BAsmCode[1]:=AdrInt AND $ff;
	   END;
	END;
       Exit;
      END;

   IF (Memo('SCALL')) OR (Memo('LCALL')) OR (Memo('CALL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],MemInt,OK);
       IF OK THEN
	BEGIN
	 AdrInt:=AdrWord-(EProgCounter+2);
	 IF Memo('SCALL') THEN IsShort:=True
	 ELSE IF Memo('LCALL') THEN IsShort:=False
	 ELSE IsShort:=(AdrInt>=-1024) AND (AdrInt<1023);
	 IF IsShort THEN
	  BEGIN
	   IF (NOT SymbolQuestionable) AND ((AdrInt<-1024) OR (AdrInt>1023)) THEN WrError(1370)
	   ELSE
	    BEGIN
	     CodeLen:=2; BAsmCode[1]:=AdrInt AND $ff;
	     BAsmCode[0]:=$28+((AdrInt AND $700) SHR 8);
	    END;
	  END
	 ELSE
	  BEGIN
           CodeLen:=3; BAsmCode[0]:=$ef; Dec(AdrInt);
           BAsmCode[1]:=Lo(AdrInt); BAsmCode[2]:=Hi(AdrInt);
	   IF (AdrInt>=-1024) AND (AdrInt<=1023) THEN WrError(20);
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('BR')) OR (Memo('LJMP')) OR (Memo('SJMP')) THEN
    BEGIN
     OpSize:=1;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF (Memo('BR')) AND (QuotPos(ArgStr[1],'[')<=Length(ArgStr[1])) THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModMem,False);
       IF AdrType<>ModNone THEN
	IF (AdrMode<>2) OR (Odd(AdrVals[0])) THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$e3; BAsmCode[1]:=AdrVals[0];
	 END;
      END
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],MemInt,OK);
       IF OK THEN
	BEGIN
	 AdrInt:=AdrWord-(EProgCounter+2);
	 IF Memo('SJMP') THEN IsShort:=True
	 ELSE IF Memo('LJMP') THEN IsShort:=False
	 ELSE IsShort:=(AdrInt>=-1024) AND (AdrInt<1023);
	 IF IsShort THEN
	  BEGIN
	   IF (NOT SymbolQuestionable) AND ((AdrInt<-1024) OR (AdrInt>1023)) THEN WrError(1370)
	   ELSE
	    BEGIN
	     CodeLen:=2; BAsmCode[1]:=AdrInt AND $ff;
	     BAsmCode[0]:=$20+((AdrInt AND $700) SHR 8);
	    END;
	  END
	 ELSE
	  BEGIN
           CodeLen:=3; BAsmCode[0]:=$e7; Dec(AdrInt);
           BAsmCode[1]:=Lo(AdrInt); BAsmCode[2]:=Hi(AdrInt);
	   IF (AdrInt>=-1024) AND (AdrInt<=1023) THEN WrError(20);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('TIJMP') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF MomCPU<CPU80196 THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=1; DecodeAdr(ArgStr[1],MModDir,False);
       IF AdrType<>ModNone THEN
        BEGIN
         BAsmCode[3]:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModDir,False);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[1]:=AdrVals[0];
           OpSize:=0; DecodeAdr(ArgStr[3],MModImm,False);
           IF AdrType<>ModNone THEN
            BEGIN
             BAsmCode[2]:=AdrVals[0]; BAsmCode[0]:=$e2; CodeLen:=4;
            END;
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('DJNZ')) OR (Memo('DJNZW')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (Memo('DJNZW')) AND (MomCPU<CPU80196) THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=Ord(Memo('DJNZW'));
       DecodeAdr(ArgStr[1],MModDir,False);
       IF AdrType<>ModNone THEN
	BEGIN
	 BAsmCode[0]:=$e0+OpSize; BAsmCode[1]:=AdrVals[0];
	 AdrInt:=EvalIntExpression(ArgStr[2],MemInt,OK)-(EProgCounter+3);
	 IF OK THEN
	  IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=3; BAsmCode[2]:=AdrInt AND $ff;
	   END;
	END;
      END;
     Exit;
    END;

   IF (Memo('JBC')) OR (Memo('JBS')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[0]:=EvalIntExpression(ArgStr[2],UInt3,OK);
       IF OK THEN
	BEGIN
	 Inc(BAsmCode[0],$30+(Ord(Memo('JBS')) SHL 3));
	 OpSize:=0; DecodeAdr(ArgStr[1],MModDir,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[1]:=AdrVals[0];
	   AdrInt:=EvalIntExpression(ArgStr[3],MemInt,OK)-(EProgCounter+3);
	   IF OK THEN
	    IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
	    ELSE
	     BEGIN
	      CodeLen:=3; BAsmCode[2]:=AdrInt AND $ff;
	     END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('ECALL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU80196N THEN WrError(1500)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],MemInt,OK)-(EProgCounter+4);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$f1;
         BAsmCode[1]:=AdrInt AND $ff;
         BAsmCode[2]:=(AdrInt SHR 8) AND $ff;
         BAsmCode[3]:=(AdrInt SHR 16) AND $ff;
         CodeLen:=4;
        END;
      END;
     Exit;
    END;

   IF (Memo('EJMP')) OR (Memo('EBR')) THEN
    BEGIN
     OpSize:=1;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU80196N THEN WrError(1500)
     ELSE IF ArgStr[1][1]='[' THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModMem,False);
       IF AdrType=ModMem THEN
        IF AdrMode<>2 THEN WrError(1350)
        ELSE IF Odd(AdrVals[0]) THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$e3; BAsmCode[1]:=AdrVals[0]+1;
          CodeLen:=2;
         END;
      END
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],MemInt,OK)-(EProgCounter+4);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$e6;
         BAsmCode[1]:=AdrInt AND $ff;
         BAsmCode[2]:=(AdrInt SHR 8) AND $ff;
         BAsmCode[3]:=(AdrInt SHR 16) AND $ff;
         CodeLen:=4;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_96;
	Far;
BEGIN
   SaveInitProc;
   WSRVal:=0; CalcWSRWindow;
   WSR1Val:=0; CalcWSR1Window;
END;

	FUNCTION ChkPC_96:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : IF MomCPU>=CPU80196N THEN OK:=ProgCounter<$1000000
              ELSE OK:=ProgCounter<$10000;
   ELSE ok:=False;
   END;
   ChkPC_96:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_96:Boolean;
	Far;
BEGIN
   IsDef_96:=False;
END;

        PROCEDURE SwitchFrom_96;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_96;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$39; NOPCode:=$fd;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode ]:=1; ListGrans[SegCode ]:=1; SegInits[SegCode ]:=0;

   MakeCode:=MakeCode_96; ChkPC:=ChkPC_96; IsDef:=IsDef_96;
   SwitchFrom:=SwitchFrom_96;

   InitFields;

   IF MomCPU>=CPU80196N THEN MemInt:=UInt24
   ELSE MemInt:=UInt16;
END;

BEGIN
   CPU8096  :=AddCPU('8096'  ,SwitchTo_96);
   CPU80196 :=AddCPU('80196' ,SwitchTo_96);
   CPU80196N:=AddCPU('80196N',SwitchTo_96);
   CPU80296 :=AddCPU('80296' ,SwitchTo_96);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_96;
END.
