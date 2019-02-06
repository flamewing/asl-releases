{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code86;

{ AS Codegeneratormodul 8086/80186/V30/V35 }

INTERFACE
        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION


TYPE FixedOrder=RECORD
		 Name:String[7];
		 MinCPU:CPUVar;
		 Code:Word;
		END;

     AddOrder=RECORD
	       Name:String[6];
	       MinCPU:CPUVar;
	       Code:Word;
	       Add:Byte;
	      END;


CONST
     D_CPU8086=0;
     D_CPU80186=1;
     D_CPUV30=2;
     D_CPUV35=3;

     FixedOrderCnt=41;
     FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
		 ((Name:'AAA';   MinCPU:D_CPU8086;  Code:$0037),
		  (Name:'AAS';   MinCPU:D_CPU8086;  Code:$003f),
		  (Name:'AAM';   MinCPU:D_CPU8086;  Code:$d40a),
		  (Name:'AAD';   MinCPU:D_CPU8086;  Code:$d50a),
		  (Name:'CBW';   MinCPU:D_CPU8086;  Code:$0098),
		  (Name:'CLC';   MinCPU:D_CPU8086;  Code:$00f8),
		  (Name:'CLD';   MinCPU:D_CPU8086;  Code:$00fc),
		  (Name:'CLI';   MinCPU:D_CPU8086;  Code:$00fa),
		  (Name:'CMC';   MinCPU:D_CPU8086;  Code:$00f5),
		  (Name:'CWD';   MinCPU:D_CPU8086;  Code:$0099),
		  (Name:'DAA';   MinCPU:D_CPU8086;  Code:$0027),
		  (Name:'DAS';   MinCPU:D_CPU8086;  Code:$002f),
		  (Name:'HLT';   MinCPU:D_CPU8086;  Code:$00f4),
		  (Name:'INTO';  MinCPU:D_CPU8086;  Code:$00ce),
		  (Name:'IRET';  MinCPU:D_CPU8086;  Code:$00cf),
		  (Name:'LAHF';  MinCPU:D_CPU8086;  Code:$009f),
		  (Name:'LOCK';  MinCPU:D_CPU8086;  Code:$00f0),
		  (Name:'NOP';   MinCPU:D_CPU8086;  Code:$0090),
		  (Name:'POPF';  MinCPU:D_CPU8086;  Code:$009d),
		  (Name:'PUSHF'; MinCPU:D_CPU8086;  Code:$009c),
		  (Name:'SAHF';  MinCPU:D_CPU8086;  Code:$009e),
		  (Name:'STC';   MinCPU:D_CPU8086;  Code:$00f9),
		  (Name:'STD';   MinCPU:D_CPU8086;  Code:$00fd),
		  (Name:'STI';   MinCPU:D_CPU8086;  Code:$00fb),
		  (Name:'WAIT';  MinCPU:D_CPU8086;  Code:$009b),
		  (Name:'XLAT';  MinCPU:D_CPU8086;  Code:$00d7),
		  (Name:'LEAVE'; MinCPU:D_CPU80186; Code:$00c9),
		  (Name:'PUSHA'; MinCPU:D_CPU80186; Code:$0060),
		  (Name:'POPA';  MinCPU:D_CPU80186; Code:$0061),
		  (Name:'ADD4S'; MinCPU:D_CPUV30;   Code:$0f20),
		  (Name:'SUB4S'; MinCPU:D_CPUV30;   Code:$0f22),
		  (Name:'CMP4S'; MinCPU:D_CPUV30;   Code:$0f26),
		  (Name:'STOP';  MinCPU:D_CPUV35;   Code:$0f9e),
		  (Name:'RETRBI';MinCPU:D_CPUV35;   Code:$0f91),
		  (Name:'FINT';  MinCPU:D_CPUV35;   Code:$0f92),
		  (Name:'MOVSPA';MinCPU:D_CPUV35;   Code:$0f25),
		  (Name:'SEGES'; MinCPU:D_CPU8086;  Code:$0026),
		  (Name:'SEGCS'; MinCPU:D_CPU8086;  Code:$002e),
		  (Name:'SEGSS'; MinCPU:D_CPU8086;  Code:$0036),
		  (Name:'SEGDS'; MinCPU:D_CPU8086;  Code:$003e),
		  (Name:'FWAIT'; MinCPU:D_CPU8086;  Code:$009b));
					
   FPUFixedOrderCnt=29;
   FPUFixedOrders:ARRAY[1..FPUFixedOrderCnt] OF FixedOrder=
		 ((Name:'FCOMPP'; MinCPU:D_CPU8086;  Code:$ded9),
		  (Name:'FTST';   MinCPU:D_CPU8086;  Code:$d9e4),
		  (Name:'FXAM';   MinCPU:D_CPU8086;  Code:$d9e5),
		  (Name:'FLDZ';   MinCPU:D_CPU8086;  Code:$d9ee),
		  (Name:'FLD1';   MinCPU:D_CPU8086;  Code:$d9e8),
		  (Name:'FLDPI';  MinCPU:D_CPU8086;  Code:$d9eb),
		  (Name:'FLDL2T'; MinCPU:D_CPU8086;  Code:$d9e9),
		  (Name:'FLDL2E'; MinCPU:D_CPU8086;  Code:$d9ea),
		  (Name:'FLDLG2'; MinCPU:D_CPU8086;  Code:$d9ec),
		  (Name:'FLDLN2'; MinCPU:D_CPU8086;  Code:$d9ed),
		  (Name:'FSQRT';  MinCPU:D_CPU8086;  Code:$d9fa),
		  (Name:'FSCALE'; MinCPU:D_CPU8086;  Code:$d9fd),
		  (Name:'FPREM';  MinCPU:D_CPU8086;  Code:$d9f8),
		  (Name:'FRNDINT';MinCPU:D_CPU8086;  Code:$d9fc),
		  (Name:'FXTRACT';MinCPU:D_CPU8086;  Code:$d9f4),
		  (Name:'FABS';   MinCPU:D_CPU8086;  Code:$d9e1),
		  (Name:'FCHS';   MinCPU:D_CPU8086;  Code:$d9e0),
		  (Name:'FPTAN';  MinCPU:D_CPU8086;  Code:$d9f2),
		  (Name:'FPATAN'; MinCPU:D_CPU8086;  Code:$d9f3),
		  (Name:'F2XM1';  MinCPU:D_CPU8086;  Code:$d9f0),
		  (Name:'FYL2X';  MinCPU:D_CPU8086;  Code:$d9f1),
		  (Name:'FYL2XP1';MinCPU:D_CPU8086;  Code:$d9f9),
		  (Name:'FINIT';  MinCPU:D_CPU8086;  Code:$dbe3),
		  (Name:'FENI';   MinCPU:D_CPU8086;  Code:$dbe0),
		  (Name:'FDISI';  MinCPU:D_CPU8086;  Code:$dbe1),
		  (Name:'FCLEX';  MinCPU:D_CPU8086;  Code:$dbe2),
		  (Name:'FINCSTP';MinCPU:D_CPU8086;  Code:$d9f7),
		  (Name:'FDECSTP';MinCPU:D_CPU8086;  Code:$d9f6),
		  (Name:'FNOP';   MinCPU:D_CPU8086;  Code:$d9d0));
					 
   StringOrderCnt=14;
   StringOrders:ARRAY[1..StringOrderCnt] OF FixedOrder=
		 ((Name:'CMPSB'; MinCPU:D_CPU8086;  Code:$00a6),
		  (Name:'CMPSW'; MinCPU:D_CPU8086;  Code:$00a7),
		  (Name:'LODSB'; MinCPU:D_CPU8086;  Code:$00ac),
		  (Name:'LODSW'; MinCPU:D_CPU8086;  Code:$00ad),
		  (Name:'MOVSB'; MinCPU:D_CPU8086;  Code:$00a4),
		  (Name:'MOVSW'; MinCPU:D_CPU8086;  Code:$00a5),
		  (Name:'SCASB'; MinCPU:D_CPU8086;  Code:$00ae),
		  (Name:'SCASW'; MinCPU:D_CPU8086;  Code:$00af),
		  (Name:'STOSB'; MinCPU:D_CPU8086;  Code:$00aa),
		  (Name:'STOSW'; MinCPU:D_CPU8086;  Code:$00ab),
		  (Name:'INSB';  MinCPU:D_CPU80186; Code:$006c),
		  (Name:'INSW';  MinCPU:D_CPU80186; Code:$006d),
		  (Name:'OUTSB'; MinCPU:D_CPU80186; Code:$006e),
		  (Name:'OUTSW'; MinCPU:D_CPU80186; Code:$006f));
					
   ReptOrderCnt=7;
   ReptOrders:ARRAY[1..ReptOrderCnt] OF FixedOrder=
		 ((Name:'REP';   MinCPU:D_CPU8086;  Code:$00f3),
		  (Name:'REPE';  MinCPU:D_CPU8086;  Code:$00f3),
		  (Name:'REPZ';  MinCPU:D_CPU8086;  Code:$00f3),
		  (Name:'REPNE'; MinCPU:D_CPU8086;  Code:$00f2),
		  (Name:'REPNZ'; MinCPU:D_CPU8086;  Code:$00f2),
		  (Name:'REPC';  MinCPU:D_CPUV30;   Code:$0065),
		  (Name:'REPNC'; MinCPU:D_CPUV30;   Code:$0064));

   RelOrderCnt=36;
   RelOrders:ARRAY[1..RelOrderCnt] OF FixedOrder=
		 ((Name:'JA';    MinCPU:D_CPU8086;  Code:$0077),
		  (Name:'JNBE';  MinCPU:D_CPU8086;  Code:$0077),
		  (Name:'JAE';   MinCPU:D_CPU8086;  Code:$0073),
		  (Name:'JNB';   MinCPU:D_CPU8086;  Code:$0073),
		  (Name:'JB';    MinCPU:D_CPU8086;  Code:$0072),
		  (Name:'JNAE';  MinCPU:D_CPU8086;  Code:$0072),
		  (Name:'JBE';   MinCPU:D_CPU8086;  Code:$0076),
		  (Name:'JNA';   MinCPU:D_CPU8086;  Code:$0076),
		  (Name:'JC';    MinCPU:D_CPU8086;  Code:$0072),
		  (Name:'JCXZ';  MinCPU:D_CPU8086;  Code:$00e3),
		  (Name:'JE';    MinCPU:D_CPU8086;  Code:$0074),
		  (Name:'JZ';    MinCPU:D_CPU8086;  Code:$0074),
		  (Name:'JG';    MinCPU:D_CPU8086;  Code:$007f),
		  (Name:'JNLE';  MinCPU:D_CPU8086;  Code:$007f),
		  (Name:'JGE';   MinCPU:D_CPU8086;  Code:$007d),
		  (Name:'JNL';   MinCPU:D_CPU8086;  Code:$007d),
		  (Name:'JL';    MinCPU:D_CPU8086;  Code:$007c),
		  (Name:'JNGE';  MinCPU:D_CPU8086;  Code:$007c),
		  (Name:'JLE';   MinCPU:D_CPU8086;  Code:$007e),
		  (Name:'JNG';   MinCPU:D_CPU8086;  Code:$007e),
		  (Name:'JNC';   MinCPU:D_CPU8086;  Code:$0073),
		  (Name:'JNE';   MinCPU:D_CPU8086;  Code:$0075),
		  (Name:'JNZ';   MinCPU:D_CPU8086;  Code:$0075),
		  (Name:'JNO';   MinCPU:D_CPU8086;  Code:$0071),
		  (Name:'JNS';   MinCPU:D_CPU8086;  Code:$0079),
		  (Name:'JNP';   MinCPU:D_CPU8086;  Code:$007b),
		  (Name:'JPO';   MinCPU:D_CPU8086;  Code:$007b),
		  (Name:'JO';    MinCPU:D_CPU8086;  Code:$0070),
		  (Name:'JP';    MinCPU:D_CPU8086;  Code:$007a),
		  (Name:'JPE';   MinCPU:D_CPU8086;  Code:$007a),
		  (Name:'JS';    MinCPU:D_CPU8086;  Code:$0078),
		  (Name:'LOOP';  MinCPU:D_CPU8086;  Code:$00e2),
		  (Name:'LOOPE'; MinCPU:D_CPU8086;  Code:$00e1),
		  (Name:'LOOPZ'; MinCPU:D_CPU8086;  Code:$00e1),
		  (Name:'LOOPNE';MinCPU:D_CPU8086;  Code:$00e0),
		  (Name:'LOOPNZ';MinCPU:D_CPU8086;  Code:$00e0));
					
   ModRegOrderCnt=4;
   ModRegOrders:ARRAY[1..ModRegOrderCnt] OF FixedOrder=
		 ((Name:'LDS';   MinCPU:D_CPU8086;  Code:$00c5),
		  (Name:'LEA';   MinCPU:D_CPU8086;  Code:$008d),
		  (Name:'LES';   MinCPU:D_CPU8086;  Code:$00c4),
		  (Name:'BOUND'; MinCPU:D_CPU80186; Code:$0062));
					
   ShiftOrderCnt=8;
   ShiftOrders:ARRAY[1..ShiftOrderCnt] OF FixedOrder=
		 ((Name:'SHL';   MinCPU:D_CPU8086;  Code:4),
		  (Name:'SAL';   MinCPU:D_CPU8086;  Code:4),
		  (Name:'SHR';   MinCPU:D_CPU8086;  Code:5),
		  (Name:'SAR';   MinCPU:D_CPU8086;  Code:7),
		  (Name:'ROL';   MinCPU:D_CPU8086;  Code:0),
		  (Name:'ROR';   MinCPU:D_CPU8086;  Code:1),
		  (Name:'RCL';   MinCPU:D_CPU8086;  Code:2),
		  (Name:'RCR';   MinCPU:D_CPU8086;  Code:3));
					
   Reg16OrderCnt=3;
   Reg16Orders:ARRAY[1..Reg16OrderCnt] OF AddOrder=
		 ((Name:'BRKCS'; MinCPU:D_CPUV35;   Code:$0f2d; Add:$c0),
		  (Name:'TSKSW'; MinCPU:D_CPUV35;   Code:$0f94; Add:$f8),
		  (Name:'MOVSPB';MinCPU:D_CPUV35;   Code:$0f95; Add:$f8));

   FPUSTOrderCnt=2;
   FPUSTOrders:ARRAY[1..FPUSTOrderCnt] OF FixedOrder=
		 ((Name:'FXCH';  MinCPU:D_CPU8086;  Code:$d9c8),
		  (Name:'FFREE'; MinCPU:D_CPU8086;  Code:$ddc0));
					
   FPU16OrderCnt=5;
   FPU16Orders:ARRAY[1..FPU16OrderCnt] OF FixedOrder=
		 ((Name:'FLDCW'; MinCPU:D_CPU8086;  Code:$d928),
		  (Name:'FSTCW'; MinCPU:D_CPU8086;  Code:$d938),
		  (Name:'FSTSW'; MinCPU:D_CPU8086;  Code:$dd38),
		  (Name:'FSTENV';MinCPU:D_CPU8086;  Code:$d930),
		  (Name:'FLDENV';MinCPU:D_CPU8086;  Code:$d920));

   ALU2OrderCnt=8;
   ALU2Orders:ARRAY[1..ALU2OrderCnt] OF String[3]=
		  ('ADD','OR','ADC','SBB','AND','SUB','XOR','CMP');

   MulOrderCnt=4;
   MulOrders:ARRAY[1..MulOrderCnt] OF String[4]=('MUL','IMUL','DIV','IDIV');

   Bit1OrderCnt=4;
   Bit1Orders:ARRAY[1..Bit1OrderCnt] OF String[5]=('TEST1','CLR1','SET1','NOT1');

   SegRegCnt=3;
   SegRegNames:ARRAY[0..SegRegCnt] OF String[2]=
	      ('ES','CS','SS','DS');
   SegRegPrefixes:ARRAY[0..SegRegCnt] OF Byte=($26,$2e,$36,$3e);

   TypeNone=-1;
   TypeReg8=0;
   TypeReg16=1;
   TypeRegSeg=2;
   TypeMem=3;
   TypeImm=4;
   TypeFReg=5;

VAR
   AdrType:ShortInt;
   AdrMode:Byte;
   AdrCnt:Byte;
   AdrVals:ARRAY[0..5] OF Byte;
   OpSize:ShortInt;
   UnknownFlag:Boolean;

   NoSegCheck:Boolean;

   Prefixes:ARRAY[0..5] OF Byte;
   PrefixLen:Byte;

   SegAssumes:ARRAY[0..SegRegCnt] OF Byte;

   SaveInitProc:PROCEDURE;

   CPU8086,CPU80186,CPUV30,CPUV35:CPUVar;

	FUNCTION Sgn(inp:Byte):Byte;
BEGIN
   IF inp>127 THEN Sgn:=$ff ELSE Sgn:=0;
END;

	PROCEDURE AddPrefix(Prefix:Byte);
BEGIN
   Prefixes[PrefixLen]:=Prefix;
   Inc(PrefixLen);
END;

	FUNCTION AbleToSign(Arg:Word):Boolean;
BEGIN
   AbleToSign:=(Arg<=$7f) OR (Arg>=$ff80);
END;

	FUNCTION MinOneIs0:Boolean;
BEGIN
   IF (UnknownFlag) AND (OpSize=-1) THEN
    BEGIN
     OpSize:=0; MinOneIs0:=True;
    END
   ELSE MinOneIs0:=False;
END;

	PROCEDURE DecodeAdr(Asc:String);
CONST
   RegCnt=7;
   Reg16Names:ARRAY[0..RegCnt] OF String[2]=
	      ('AX','CX','DX','BX','SP','BP','SI','DI');
   Reg8Names :ARRAY[0..RegCnt] OF String[2]=
	      ('AL','CL','DL','BL','AH','CH','DH','BH');
   RMCodes:ARRAY[0..7] OF Byte=(11,12,21,22,1,2,20,10);

VAR
   RegZ,z:Integer;
   IsImm:Boolean;
   IndexBuf,BaseBuf,SumBuf:ShortInt;
   DispAcc,DispSum:LongInt;
   p,p1,p2:Integer;
   HasAdr:Boolean;
   OK,OldNegFlag,NegFlag:Boolean;
   AdrPart,AddPart:String;
   SegBuffer:ShortInt;
   MomSegment:Byte;
   FoundSize:ShortInt;

	PROCEDURE ChkOpSize(NewSize:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=NewSize
   ELSE IF OpSize<>NewSize THEN
    BEGIN
     AdrType:=TypeNone; WrError(1131);
    END;
END;

	PROCEDURE ChkSpaces;
VAR
   EffSeg:Byte;

	PROCEDURE ChkSpace(Seg:Byte);
VAR
   z:Byte;
BEGIN
   { liegt Operand im zu prfenden Segment? nein-->vergessen }

   IF MomSegment AND (1 SHL Seg)=0 THEN Exit;

   { zeigt bish. benutztes Segmentregister auf dieses Segment? ja-->ok }

   IF EffSeg=Seg THEN Exit;

   { falls schon ein Override gesetzt wurde, nur warnen }

   IF PrefixLen>0 THEN WrError(70)

   { ansonsten ein passendes Segment suchen und warnen, falls keines da }

   ELSE
    BEGIN
     z:=0;
     WHILE (z<=SegRegCnt) AND (SegAssumes[z]<>Seg) DO Inc(z);
     IF z>SegRegCnt THEN WrXError(75,SegNames[Seg])
     ELSE AddPrefix(SegRegPrefixes[z]);
    END;
END;

BEGIN
   IF NoSegCheck THEN Exit;

   { in welches Segment geht das benutzte Segmentregister ? }

   EffSeg:=SegAssumes[SegBuffer];

   { Zieloperand in Code-/Datensegment ? }

   ChkSpace(SegCode); ChkSpace(SegXData); ChkSpace(SegData);
END;

BEGIN
   AdrType:=TypeNone; AdrCnt:=0;
   SegBuffer:=-1; MomSegment:=0;

   FOR RegZ:=0 TO RegCnt DO
    BEGIN
     IF NLS_StrCaseCmp(Asc,Reg16Names[RegZ])=0 THEN
      BEGIN
       AdrType:=TypeReg16; AdrMode:=RegZ;
       ChkOpSize(1);
       Exit;
      END;
     IF NLS_StrCaseCmp(Asc,Reg8Names[RegZ])=0 THEN
      BEGIN
       AdrType:=TypeReg8; AdrMode:=RegZ;
       ChkOpSize(0);
       Exit;
      END;
    END;

   FOR RegZ:=0 TO SegRegCnt DO
    IF NLS_StrCaseCmp(Asc,SegRegNames[RegZ])=0 THEN
     BEGIN
      AdrType:=TypeRegSeg; AdrMode:=RegZ;
      ChkOpSize(1);
      Exit;
     END;

   IF FPUAvail THEN
    BEGIN
     IF NLS_StrCaseCmp(Asc,'ST')=0 THEN
      BEGIN
       AdrType:=TypeFReg; AdrMode:=0;
       ChkOpSize(4);
       Exit;
      END;

     IF (Length(Asc)>4) AND (NLS_StrCaseCmp(Copy(Asc,1,3),'ST(')=0) AND (Asc[Length(Asc)]=')') THEN
      BEGIN
       AdrMode:=EvalIntExpression(Copy(Asc,4,Length(Asc)-4),UInt3,OK);
       IF OK THEN
        BEGIN
         AdrType:=TypeFReg;
         ChkOpSize(4);
        END;
       Exit;
      END;
    END;

   IsImm:=True;
   IndexBuf:=0; BaseBuf:=0;
   DispAcc:=0; FoundSize:=-1;

   IF NLS_StrCaseCmp(Copy(Asc,1,8),'WORD PTR')=0 THEN
    BEGIN
     Delete(Asc,1,8); FoundSize:=1; IsImm:=False;
     KillBlanks(Asc);
    END
   ELSE IF NLS_StrCaseCmp(Copy(Asc,1,8),'BYTE PTR')=0 THEN
    BEGIN
     Delete(Asc,1,8); FoundSize:=0; IsImm:=False;
     KillBlanks(Asc);
    END
   ELSE IF NLS_StrCaseCmp(Copy(Asc,1,9),'DWORD PTR')=0 THEN
    BEGIN
     Delete(Asc,1,9); FoundSize:=2; IsImm:=False;
     KillBlanks(Asc);
    END
   ELSE IF NLS_StrCaseCmp(Copy(Asc,1,9),'QWORD PTR')=0 THEN
    BEGIN
     Delete(Asc,1,9); FoundSize:=3; IsImm:=False;
     KillBlanks(Asc);
    END
   ELSE IF NLS_StrCaseCmp(Copy(Asc,1,9),'TBYTE PTR')=0 THEN
    BEGIN
     Delete(Asc,1,9); FoundSize:=4; IsImm:=False;
     KillBlanks(Asc);
    END;

   IF (Length(Asc)>2) AND (Asc[3]=':') THEN
    BEGIN
     AddPart:=Copy(Asc,1,2);
     FOR z:=0 TO SegRegCnt DO
      IF NLS_StrCaseCmp(AddPart,SegRegNames[z])=0 THEN
       BEGIN
	Delete(Asc,1,3); SegBuffer:=z;
	AddPrefix(SegRegPrefixes[SegBuffer]);
       END;
    END;

   REPEAT
    p:=QuotPos(Asc,'['); HasAdr:=p<=Length(Asc);

    IF p>1 THEN
     BEGIN
      FirstPassUnknown:=False;
      Inc(DispAcc,EvalIntExpression(Copy(Asc,1,p-1),Int16,OK));
      IF NOT OK THEN Exit;
      UnknownFlag:=UnknownFlag OR FirstPassUnknown;
      MomSegment:=MomSegment OR TypeFlag;
      IF FoundSize=-1 THEN FoundSize:=SizeFlag;
      Delete(Asc,1,p-1);
     END;

    IF HasAdr THEN
     BEGIN
      IsImm:=False;

      p:=QuotPos(Asc,']'); IF p>Length(Asc) THEN
       BEGIN
        WrError(1300); Exit;
       END;

      AdrPart:=Copy(Asc,2,p-2); Delete(Asc,1,p); OldNegFlag:=False;

      REPEAT
       NegFlag:=False;
       p1:=QuotPos(AdrPart,'+'); p2:=QuotPos(AdrPart,'-');
       IF p1>p2 THEN
	BEGIN
	 p:=p2; NegFlag:=True;
	END
       ELSE p:=p1;

       AddPart:=Copy(AdrPart,1,p-1);
       IF p>Length(AdrPart) THEN AdrPart:='' ELSE Delete(AdrPart,1,p);

       IF NLS_StrCaseCmp(AddPart,'BX')=0 THEN
	BEGIN
	 IF (OldNegFlag) OR (BaseBuf<>0) THEN Exit ELSE BaseBuf:=1;
	END
       ELSE IF NLS_StrCaseCmp(AddPart,'BP')=0 THEN
	BEGIN
	 IF (OldNegFlag) OR (BaseBuf<>0) THEN Exit ELSE BaseBuf:=2;
	END
       ELSE IF NLS_StrCaseCmp(AddPart,'SI')=0 THEN
	BEGIN
	 IF (OldNegFlag) OR (IndexBuf<>0) THEN Exit ELSE IndexBuf:=1;
	END
       ELSE IF NLS_StrCaseCmp(AddPart,'DI')=0 THEN
	BEGIN
	 IF (OldNegFlag) OR (IndexBuf<>0) THEN Exit ELSE IndexBuf:=2;
	END
       ELSE
	BEGIN
	 FirstPassUnknown:=False;
	 DispSum:=EvalIntExpression(AddPart,Int16,OK);
	 IF NOT OK THEN Exit;
	 UnknownFlag:=UnknownFlag OR FirstPassUnknown;
	 IF OldNegFlag THEN Dec(DispAcc,DispSum) ELSE Inc(DispAcc,DispSum);
	 MomSegment:=MomSegment OR TypeFlag;
	 IF FoundSize=-1 THEN FoundSize:=SizeFlag;
	END;
       OldNegFlag:=NegFlag;
      UNTIL AdrPart='';
     END;
   UNTIL Asc='';

   SumBuf:=BaseBuf*10+IndexBuf;

   { welches Segment effektiv benutzt ? }

   IF SegBuffer=-1 THEN
    IF BaseBuf=2 THEN SegBuffer:=2 ELSE SegBuffer:=3;

   { nur Displacement }

   IF (SumBuf=0) THEN

   { immediate }

    IF IsImm THEN
     BEGIN
      IF ((OpSize=0) AND UnknownFlag) OR (MinOneIs0) THEN DispAcc:=DispAcc AND $ff;
      CASE OpSize OF
      -1:WrError(1132);
       0:IF (DispAcc<-128) OR (DispAcc>255) THEN WrError(1320)
	 ELSE
	  BEGIN
	   AdrType:=TypeImm; AdrVals[0]:=DispAcc AND $ff; AdrCnt:=1;
	  END;
       1:BEGIN
	  AdrType:=TypeImm;
	  AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc); AdrCnt:=2;
	 END;
      END;
     END

    { absolut }

    ELSE
     BEGIN
      AdrType:=TypeMem; AdrMode:=$06;
      AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc); AdrCnt:=2;
      IF FoundSize<>-1 THEN ChkOpSize(FoundSize);
      ChkSpaces;
     END

   { kombiniert }

   ELSE
    BEGIN
     AdrType:=TypeMem;
     FOR z:=0 TO 7 DO
      IF SumBuf=RMCodes[z] THEN AdrMode:=z;
     IF DispAcc=0 THEN
      BEGIN
       IF SumBuf=20 THEN
	BEGIN
	 Inc(AdrMode,$40); AdrVals[0]:=0; AdrCnt:=1;
	END;
      END
     ELSE IF AbleToSign(DispAcc) THEN
      BEGIN
       Inc(AdrMode,$40);
       AdrVals[0]:=DispAcc AND $ff; AdrCnt:=1;
      END
     ELSE
      BEGIN
       Inc(AdrMode,$80);
       AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc); AdrCnt:=2;
      END;
     ChkSpaces;
     IF FoundSize<>-1 THEN ChkOpSize(FoundSize);
    END;
END;

	FUNCTION FMemo(Name:String):Boolean;
BEGIN
   IF Memo(Name) THEN
    BEGIN
     FMemo:=True; AddPrefix($9b);
    END
   ELSE
    BEGIN
     Insert('N',Name,2);
     FMemo:=Memo(Name);
    END;
END;

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF86Count=1;
   ONOFF86s:ARRAY[1..ONOFF86Count] OF ONOFFRec=
	    ((Name:'FPU'    ; Dest:@FPUAvail  ; FlagName:FPUAvailName  ));
VAR
   HLocHandle:LongInt;
   OK:Boolean;
   AdrWord:Word;
   z,z2,z3,p:Integer;
   SegPart,ValPart:String;
BEGIN
   DecodePseudo:=True;

   IF Memo('PORT') THEN
    BEGIN
     CodeEquate(SegIO,0,$ffff);
     Exit;
    END;

   IF CodeONOFF(@ONOFF86s,ONOFF86Count) THEN Exit;

   IF Memo('ASSUME') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; OK:=True;
       WHILE (z<=ArgCnt) AND (OK) DO
	BEGIN
	 OK:=False; p:=QuotPos(ArgStr[z],':');
	 SegPart:=Copy(ArgStr[z],1,p-1);
	 ValPart:=Copy(ArgStr[z],p+1,Length(ArgStr[z])-p);
	 z2:=0;
         NLS_UpString(SegPart); NLS_UpString(ValPart);
         WHILE (z2<=SegRegCnt) AND (SegPart<>SegRegNames[z2]) DO Inc(z2);
	 IF z2>SegRegCnt THEN WrXError(1962,SegPart)
	 ELSE
	  BEGIN
	   z3:=0;
	   WHILE (z3<=PCMax) AND (ValPart<>SegNames[z3]) DO Inc(z3);
	   IF z3>PCMax THEN WrXError(1961,ValPart)
	   ELSE IF (z3<>SegCode) AND (z3<>SegData) AND (z3<>SegXData) AND (z3<>SegNone) THEN WrError(1960)
	   ELSE
	    BEGIN
	     SegAssumes[z2]:=z3; OK:=True;
	    END;
	  END;
	 Inc(z);
	END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE PutCode(Code:Word);
BEGIN
   IF Hi(Code)=0 THEN
    BEGIN
     BAsmCode[CodeLen]:=Lo(Code); Inc(CodeLen);
    END
   ELSE
    BEGIN
     BAsmCode[CodeLen]:=Hi(Code); Inc(CodeLen);
     BAsmCode[CodeLen]:=Lo(Code); Inc(CodeLen);
    END;
END;

	PROCEDURE MoveAdr(Dest:Integer);
BEGIN
   Move(AdrVals,BAsmCode[CodeLen+Dest],AdrCnt);
END;

	FUNCTION DecodeFPU:Boolean;
VAR
   z:Integer;
   OpAdd:Byte;
BEGIN
   IF NOT FPUAvail THEN
    BEGIN
     DecodeFPU:=False; Exit;
    END;

   DecodeFPU:=True;

   FOR z:=1 TO FPUFixedOrderCnt DO
    WITH FPUFixedOrders[z] DO
     IF FMemo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU-CPU8086<MinCPU THEN WrError(1500)
       ELSE PutCode(Code);
       Exit;
      END;

   FOR z:=1 TO FPUStOrderCnt DO
    WITH FPUStOrders[z] DO
     IF FMemo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1]);
         IF AdrType=TypeFReg THEN
	  BEGIN
	   PutCode(Code); Inc(BAsmCode[CodeLen-1],AdrMode);
          END
         ELSE IF AdrType<>TypeNone THEN WrError(1350);
	END;
       Exit;
      END;

   IF FMemo('FLD') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeFReg:
	BEGIN
	 BAsmCode[CodeLen]:=$d9; BAsmCode[CodeLen+1]:=$c0+AdrMode;
	 Inc(CodeLen,2);
	END;
       TypeMem:
        BEGIN
         IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=2;
         IF OpSize=-1 THEN WrError(1132)
	 ELSE IF OpSize<2 THEN WrError(1130)
	 ELSE
	  BEGIN
	   MoveAdr(2);
	   BAsmCode[CodeLen+1]:=AdrMode;
	   CASE OpSize OF
	   2:BAsmCode[CodeLen]:=$d9;
	   3:BAsmCode[CodeLen]:=$dd;
	   4:BEGIN
	      BAsmCode[CodeLen]:=$db; Inc(BAsmCode[CodeLen+1],$28);
	     END;
	   END;
	   Inc(CodeLen,2+AdrCnt);
	  END;
        END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF FMemo('FILD') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeMem:
        BEGIN
         IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=1;
         IF OpSize=-1 THEN WrError(1132)
         ELSE IF (OpSize<1) OR (OpSize>3) THEN WrError(1130)
         ELSE
          BEGIN
           MoveAdr(2);
           BAsmCode[CodeLen+1]:=AdrMode;
           CASE OpSize OF
           1:BAsmCode[CodeLen]:=$df;
           2:BAsmCode[CodeLen]:=$db;
           3:BEGIN
              BAsmCode[CodeLen]:=$df; Inc(BAsmCode[CodeLen+1],$28);
             END;
           END;
           Inc(CodeLen,2+AdrCnt);
          END;
        END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF FMemo('FBLD') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeMem:
        BEGIN
         IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=4;
         IF OpSize=-1 THEN WrError(1132)
	 ELSE IF OpSize<>4 THEN WrError(1130)
         ELSE
          BEGIN
           BAsmCode[CodeLen]:=$df;
           MoveAdr(2);
           BAsmCode[CodeLen+1]:=AdrMode+$20;
           Inc(CodeLen,2+AdrCnt);
          END;
        END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350)
       END;
      END;
     Exit;
    END;

   IF (FMemo('FST')) OR (FMemo('FSTP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeFReg:
	BEGIN
	 BAsmCode[CodeLen]:=$dd; BAsmCode[CodeLen+1]:=$d0+AdrMode;
         IF FMemo('FSTP') THEN Inc(BAsmCode[CodeLen+1],8);
	 Inc(CodeLen,2);
	END;
       TypeMem:
        BEGIN
         IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=2;
 	 IF OpSize=-1 THEN WrError(1132)
         ELSE IF (OpSize<2) OR ((OpSize=4) AND (FMemo('FST'))) THEN WrError(1130)
	 ELSE
	  BEGIN
	   MoveAdr(2);
	   BAsmCode[CodeLen+1]:=AdrMode+$10;
           IF FMemo('FSTP') THEN Inc(BAsmCode[CodeLen+1],8);
	   CASE OpSize OF
	   2:BAsmCode[CodeLen]:=$d9;
	   3:BAsmCode[CodeLen]:=$dd;
	   4:BEGIN
	      BAsmCode[CodeLen]:=$db; Inc(BAsmCode[CodeLen+1],$20);
	     END;
	   END;
	   Inc(CodeLen,2+AdrCnt);
	  END;
        END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FIST')) OR (FMemo('FISTP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeMem:
        BEGIN
         IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=1;
         IF OpSize=-1 THEN WrError(1132)
         ELSE IF (OpSize<1) OR (OpSize=4) OR ((OpSize=3) AND (FMemo('FIST'))) THEN WrError(1130)
         ELSE
          BEGIN
           MoveAdr(2);
           BAsmCode[CodeLen+1]:=AdrMode+$10;
           IF FMemo('FISTP') THEN Inc(BAsmCode[CodeLen+1],8);
           CASE OpSize OF
           1:BAsmCode[CodeLen]:=$df;
           2:BAsmCode[CodeLen]:=$db;
           3:BEGIN
              BAsmCode[CodeLen]:=$df; Inc(BAsmCode[CodeLen+1],$20);
             END;
           END;
           Inc(CodeLen,2+AdrCnt);
          END;
        END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF FMemo('FBSTP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeMem:
        BEGIN
         IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=1;
	 IF OpSize=-1 THEN WrError(1132)
	 ELSE IF OpSize<>4 THEN WrError(1130)
 	 ELSE
	  BEGIN
	   BAsmCode[CodeLen]:=$df; BAsmCode[CodeLen+1]:=AdrMode+$30;
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
        END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FCOM')) OR (FMemo('FCOMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeFReg:
	BEGIN
	 BAsmCode[CodeLen]:=$d8; BAsmCode[CodeLen+1]:=$d0+AdrMode;
         IF FMemo('FCOMP') THEN Inc(BAsmCode[CodeLen+1],8);
	 Inc(CodeLen,2);
	END;
       TypeMem:
        BEGIN
         IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=1;
	 IF OpSize=-1 THEN WrError(1132)
	 ELSE IF (OpSize<>2) AND (OpSize<>3) THEN WrError(1130)
 	 ELSE
	  BEGIN
	   IF OpSize=2 THEN BAsmCode[CodeLen]:=$d8 ELSE BAsmCode[CodeLen]:=$dc;
	   BAsmCode[CodeLen+1]:=AdrMode+$10;
           IF FMemo('FCOMP') THEN Inc(BAsmCode[CodeLen+1],8);
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
        END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FICOM')) OR (FMemo('FICOMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeMem:
        BEGIN
         IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=1;
         IF OpSize=-1 THEN WrError(1132)
	 ELSE IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
	 ELSE
	  BEGIN
	   IF OpSize=1 THEN BAsmCode[CodeLen]:=$de ELSE BAsmCode[CodeLen]:=$da;
	   BAsmCode[CodeLen+1]:=AdrMode+$10;
           IF FMemo('FICOMP') THEN Inc(BAsmCode[CodeLen+1],8);
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
        END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FADD')) OR (FMemo('FMUL')) THEN
    BEGIN
     OpAdd:=0; IF FMemo('FMUL') THEN Inc(OpAdd,8);
     IF ArgCnt=0 THEN
      BEGIN
       BAsmCode[CodeLen]:=$de; BAsmCode[CodeLen+1]:=$c1+OpAdd;
       Inc(CodeLen,2);
       Exit;
      END;
     IF ArgCnt=1 THEN
      BEGIN
       ArgStr[2]:=ArgStr[1]; ArgStr[1]:='ST'; Inc(ArgCnt);
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]); OpSize:=-1;
       CASE AdrType OF
       TypeFReg:
	IF AdrMode<>0 THEN   { ST(i) ist Ziel }
 	 BEGIN
	  BAsmCode[CodeLen+1]:=AdrMode;
	  DecodeAdr(ArgStr[2]);
	  IF (AdrType<>TypeFReg) OR (AdrMode<>0) THEN WrError(1350)
	  ELSE
	   BEGIN
	    BAsmCode[CodeLen]:=$dc; Inc(BAsmCode[CodeLen+1],$c0+OpAdd);
	    Inc(CodeLen,2);
	   END;
	 END
	ELSE                      { ST ist Ziel }
	 BEGIN
	  DecodeAdr(ArgStr[2]);
	  CASE AdrType OF
	  TypeFReg:
	   BEGIN
	    BAsmCode[CodeLen]:=$d8; BAsmCode[CodeLen+1]:=$c0+AdrMode+OpAdd;
	    Inc(CodeLen,2);
	   END;
	  TypeMem:
           BEGIN
            IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=2;
	    IF OpSize=-1 THEN WrError(1132)
	    ELSE IF (OpSize<>2) AND (OpSize<>3) THEN WrError(1130)
	    ELSE
	     BEGIN
	      IF OpSize=2 THEN BAsmCode[CodeLen]:=$d8 ELSE BAsmCode[CodeLen]:=$dc;
	      BAsmCode[CodeLen+1]:=AdrMode+OpAdd;
	      MoveAdr(2);
	      Inc(CodeLen,AdrCnt+2);
	     END;
           END;
          ELSE IF AdrType<>TypeNone THEN WrError(1350);
	  END;
	 END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FIADD')) OR (FMemo('FIMUL')) THEN
    BEGIN
     OpAdd:=0; IF FMemo('FIMUL') THEN Inc(OpAdd,8);
     IF ArgCnt=1 THEN
      BEGIN
       ArgCnt:=2; ArgStr[2]:=ArgStr[1]; ArgStr[1]:='ST';
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeFReg:
	IF AdrMode<>0 THEN WrError(1350)
	ELSE
	 BEGIN
	  OpSize:=-1;
	  DecodeAdr(ArgStr[2]);
          IF (AdrType<>TypeMem) AND (AdrType<>TypeNone) THEN WrError(1350)
          ELSE IF AdrType<>TypeNone THEN
           BEGIN
            IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=1;
            IF OpSize=-1 THEN WrError(1132)
	    ELSE IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
	    ELSE
	     BEGIN
	      IF OpSize=1 THEN BAsmCode[CodeLen]:=$de ELSE BAsmCode[CodeLen]:=$da;
	      BAsmCode[CodeLen+1]:=AdrMode+OpAdd;
	      MoveAdr(2);
	      Inc(CodeLen,2+AdrCnt);
	     END;
           END;
	 END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FADDP')) OR (FMemo('FMULP')) THEN
    BEGIN
     OpAdd:=0; IF FMemo('FMULP') THEN Inc(OpAdd,8);
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2]);
       CASE AdrType OF
       TypeFReg:
	IF AdrMode<>0 THEN WrError(1350)
	ELSE
	 BEGIN
	  DecodeAdr(ArgStr[1]);
          IF (AdrType<>TypeFReg) AND (AdrType<>TypeNone) THEN WrError(1350)
          ELSE IF AdrType<>TypeNone THEN
	   BEGIN
	    BAsmCode[CodeLen]:=$de;
	    BAsmCode[CodeLen+1]:=$c0+AdrMode+OpAdd;
	    Inc(CodeLen,2);
	   END;
	 END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FSUB')) OR (FMemo('FSUBR')) OR (FMemo('FDIV')) OR (FMemo('FDIVR')) THEN
    BEGIN
     OpAdd:=0;
     IF (FMemo('FSUBR')) OR (FMemo('FDIVR')) THEN Inc(OpAdd,8);
     IF (FMemo('FDIV')) OR (FMemo('FDIVR')) THEN Inc(OpAdd,16);
     IF ArgCnt=0 THEN
      BEGIN
       BAsmCode[CodeLen]:=$de; BAsmCode[CodeLen+1]:=$e1+(OpAdd XOR 8);
       Inc(CodeLen,2);
       Exit;
      END;
     IF ArgCnt=1 THEN
      BEGIN
       ArgStr[2]:=ArgStr[1]; ArgStr[1]:='ST'; Inc(ArgCnt);
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]); OpSize:=-1;
       CASE AdrType OF
       TypeFReg:
	IF AdrMode<>0 THEN   { ST(i) ist Ziel }
	 BEGIN
	  BAsmCode[CodeLen+1]:=AdrMode;
	  DecodeAdr(ArgStr[2]);
	  CASE AdrType OF
	  TypeFReg:
	   IF AdrMode<>0 THEN WrError(1350)
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=$dc; Inc(BAsmCode[CodeLen+1],$e0+(OpAdd XOR 8));
	     Inc(CodeLen,2);
	    END;
          ELSE IF AdrType<>TypeNone THEN WrError(1350);
          END;
	 END
	ELSE                      { ST ist Ziel }
	 BEGIN
	  DecodeAdr(ArgStr[2]);
	  CASE AdrType OF
	  TypeFReg:
	   BEGIN
	    BAsmCode[CodeLen]:=$d8; BAsmCode[CodeLen+1]:=$e0+AdrMode+OpAdd;
	    Inc(CodeLen,2);
	   END;
	  TypeMem:
           BEGIN
            IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=2;
            IF OpSize=-1 THEN WrError(1132)
	    ELSE IF (OpSize<>2) AND (OpSize<>3) THEN WrError(1130)
	    ELSE
	     BEGIN
	      IF OpSize=2 THEN BAsmCode[CodeLen]:=$d8 ELSE BAsmCode[CodeLen]:=$dc;
	      BAsmCode[CodeLen+1]:=AdrMode+$20+OpAdd;
	      MoveAdr(2);
	      Inc(CodeLen,AdrCnt+2);
	     END;
           END;
	  ELSE IF AdrType<>TypeNone THEN WrError(1350);
	  END;
	 END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FISUB')) OR (FMemo('FISUBR')) OR (FMemo('FIDIV')) OR (FMemo('FIDIVR')) THEN
    BEGIN
     OpAdd:=0;
     IF (FMemo('FISUBR')) OR (FMemo('FIDIVR')) THEN Inc(OpAdd,8);
     IF (FMemo('FIDIV')) OR (FMemo('FIDIVR')) THEN Inc(OpAdd,16);
     IF ArgCnt=1 THEN
      BEGIN
       ArgCnt:=2; ArgStr[2]:=ArgStr[1]; ArgStr[1]:='ST';
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeFReg:
	IF AdrMode<>0 THEN WrError(1350)
	ELSE
	 BEGIN
	  OpSize:=-1;
	  DecodeAdr(ArgStr[2]);
	  CASE AdrType OF
	  TypeMem:
           BEGIN
            IF (OpSize=-1) AND (UnknownFlag) THEN OpSize:=1;
            IF OpSize=-1 THEN WrError(1132)
	    ELSE IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
	    ELSE
	     BEGIN
	      IF OpSize=1 THEN BAsmCode[CodeLen]:=$de ELSE BAsmCode[CodeLen]:=$da;
	      BAsmCode[CodeLen+1]:=AdrMode+$20+OpAdd;
	      MoveAdr(2);
	      Inc(CodeLen,2+AdrCnt);
	     END;
           END;
          ELSE IF AdrType<>TypeNone THEN WrError(1350);
          END;
         END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (FMemo('FSUBP')) OR (FMemo('FSUBRP')) OR (FMemo('FDIVP')) OR (FMemo('FDIVRP')) THEN
    BEGIN
     OpAdd:=0;
     IF (FMemo('FSUBRP')) OR (FMemo('FDIVRP')) THEN Inc(OpAdd,8);
     IF (FMemo('FDIVP')) OR (FMemo('FDIVRP')) THEN Inc(OpAdd,16);
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2]);
       CASE AdrType OF
       TypeFReg:
	IF AdrMode<>0 THEN WrError(1350)
	ELSE
	 BEGIN
	  DecodeAdr(ArgStr[1]);
	  CASE AdrType OF
	  TypeFReg:
	   BEGIN
	    BAsmCode[CodeLen]:=$de;
	    BAsmCode[CodeLen+1]:=$e0+AdrMode+(OpAdd XOR 8);
	    Inc(CodeLen,2);
	   END;
          ELSE IF AdrType<>TypeNone THEN WrError(1350);
          END;
	 END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   FOR z:=1 TO FPU16OrderCnt DO
    WITH FPU16Orders[z] DO
     IF FMemo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 OpSize:=1;
	 DecodeAdr(ArgStr[1]);
	 CASE AdrType OF
	 TypeMem:
	  BEGIN
	   PutCode(Code);
	   Inc(BAsmCode[CodeLen-1],AdrMode);
	   MoveAdr(0);
	   Inc(CodeLen,AdrCnt);
	  END;
         ELSE IF AdrType<>TypeNone THEN WrError(1350);
         END;
	END;
       Exit;
      END;

   IF (FMemo('FSAVE')) OR (FMemo('FRSTOR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeMem:
	BEGIN
	 BAsmCode[CodeLen]:=$dd; BAsmCode[CodeLen+1]:=AdrMode+$20;
         IF FMemo('FSAVE') THEN Inc(BasmCode[CodeLen+1],$10);
	 MoveAdr(2);
	 Inc(CodeLen,2+AdrCnt);
	END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   DecodeFPU:=False;
END;

	PROCEDURE MakeCode_86;
	Far;
LABEL
   AddPrefixes;
VAR
   OK:Boolean;
   AdrWord:Word;
   AdrByte:Byte;
   z,z2:Integer;
   OpAdd:Byte;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1; PrefixLen:=0;
   NoSegCheck:=False; UnknownFlag:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { ohne Operanden }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU-CPU8086<MinCPU THEN WrError(1500)
       ELSE PutCode(Code);
       Goto AddPrefixes;
      END;

   { Koprozessor }

   IF DecodeFPU THEN Goto AddPrefixes;

   { Stringoperationen }

   FOR z:=1 TO StringOrderCnt DO
    WITH StringOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU-CPU8086<MinCPU THEN WrError(1500)
       ELSE PutCode(Code);
       Goto AddPrefixes;
      END;

   { mit Wiederholung }

   FOR z:=1 TO ReptOrderCnt DO
    WITH ReptOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF MomCPU-CPU8086<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
         z2:=0; NLS_UpString(ArgStr[1]);
	 REPEAT
	  Inc(z2);
	 UNTIL (z2>StringOrderCnt) OR (StringOrders[z2].Name=ArgStr[1]);
	 IF z2>StringOrderCnt THEN WrError(1985)
	 ELSE IF MomCPU-CPU8086<StringOrders[z2].MinCPU THEN WrError(1500)
	 ELSE
	  BEGIN
	   PutCode(Code); PutCode(StringOrders[z2].Code);
	  END;
	END;
       Goto AddPrefixes;
      END;

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeReg8,TypeReg16:
	BEGIN
	 AdrByte:=AdrMode;
	 DecodeAdr(ArgStr[2]);
	 CASE AdrType OF
	 TypeReg8,TypeReg16:
	  BEGIN
	   BAsmCode[CodeLen]:=$8a+OpSize;
	   BAsmCode[CodeLen+1]:=$c0+(AdrByte SHL 3)+AdrMode;
	   Inc(CodeLen,2);
	  END;
	 TypeMem:
	  IF (AdrByte=0) AND (AdrMode=6) THEN
	   BEGIN
	    BAsmCode[CodeLen]:=$a0+OpSize;
	    MoveAdr(1);
	    Inc(CodeLen,1+AdrCnt);
	   END
	  ELSE
	   BEGIN
	    BAsmCode[CodeLen]:=$8a+OpSize;
	    BAsmCode[CodeLen+1]:=AdrMode+(AdrByte SHL 3);
	    MoveAdr(2);
	    Inc(CodeLen,2+AdrCnt);
	   END;
	 TypeRegSeg:
          IF OpSize=0 THEN WrError(1131)
          ELSE
	   BEGIN
	    BAsmCode[CodeLen]:=$8c;
	    BAsmCode[CodeLen+1]:=$c0+(AdrMode SHL 3)+AdrByte;
	    Inc(CodeLen,2);
	   END;
	 TypeImm:
	  BEGIN
	   BAsmCode[CodeLen]:=$b0+(OpSize SHL 3)+AdrByte;
	   MoveAdr(1);
	   Inc(CodeLen,1+AdrCnt);
	  END;
	 ELSE IF AdrType<>TypeNone THEN WrError(1350);
	 END;
	END;
       TypeMem:
	BEGIN
	 BAsmCode[CodeLen+1]:=AdrMode;
	 MoveAdr(2); AdrByte:=AdrCnt;
	 DecodeAdr(ArgStr[2]);
	 CASE AdrType OF
	 TypeReg8,TypeReg16:
	  IF (AdrMode=0) AND (BAsmCode[CodeLen+1]=6) THEN
	   BEGIN
	    BAsmCode[CodeLen]:=$a2+OpSize;
	    Move(BAsmCode[CodeLen+2],BAsmCode[CodeLen+1],AdrByte);
	    Inc(CodeLen,1+AdrByte);
	   END
	  ELSE
	   BEGIN
	    BAsmCode[CodeLen]:=$88+OpSize;
	    Inc(BAsmCode[CodeLen+1],AdrMode SHL 3);
	    Inc(CodeLen,2+AdrByte);
	   END;
	 TypeRegSeg:
	  BEGIN
	   BAsmCode[CodeLen]:=$8c;
	   Inc(BAsmCode[CodeLen+1],AdrMode SHL 3);
	   Inc(CodeLen,2+AdrByte);
	  END;
	 TypeImm:
	  BEGIN
	   BAsmCode[CodeLen]:=$c6+OpSize;
	   MoveAdr(2+AdrByte);
	   Inc(CodeLen,2+AdrByte+AdrCnt);
	  END;
	 ELSE IF AdrType<>TypeNone THEN WrError(1350);
	 END;
	END;
       TypeRegSeg:
	BEGIN
	 BAsmCode[CodeLen+1]:=AdrMode SHL 3;
	 DecodeAdr(ArgStr[2]);
	 CASE AdrType OF
	 TypeReg16:
	  BEGIN
	   BAsmCode[CodeLen]:=$8e;
	   Inc(BAsmCode[CodeLen+1],$c0+AdrMode);
	   Inc(CodeLen,2);
	  END;
	 TypeMem:
	  BEGIN
	   BAsmCode[CodeLen]:=$8e;
	   Inc(BAsmCode[CodeLen+1],AdrMode);
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
	 ELSE IF AdrType<>TypeNone THEN WrError(1350);
	 END;
	END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Goto AddPrefixes;
    END;

   FOR z:=0 TO ALU2OrderCnt-1 DO
    IF Memo(ALU2Orders[z+1]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1]);
	CASE AdrType OF
	TypeReg8,TypeReg16:
	 BEGIN
	  BAsmCode[CodeLen+1]:=AdrMode SHL 3;
	  DecodeAdr(ArgStr[2]);
	  CASE AdrType OF
	  TypeReg8,TypeReg16:
	   BEGIN
	    Inc(BAsmCode[CodeLen+1],$c0+AdrMode);
	    BAsmCode[CodeLen]:=(z SHL 3)+2+OpSize;
	    Inc(CodeLen,2);
	   END;
	  TypeMem:
	   BEGIN
	    Inc(BAsmCode[CodeLen+1],AdrMode);
	    BAsmCode[CodeLen]:=(z SHL 3)+2+OpSize;
	    MoveAdr(2);
	    Inc(CodeLen,2+AdrCnt);
	   END;
	  TypeImm:
	   IF (BAsmCode[CodeLen+1] SHR 3) AND 7=0 THEN
	    BEGIN
	     BAsmCode[CodeLen]:=z SHL 3+4+OpSize;
	     MoveAdr(1);
	     Inc(CodeLen,1+AdrCnt);
	    END
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=OpSize+$80;
	     IF (OpSize=1) AND (Sgn(AdrVals[0])=AdrVals[1]) THEN
	      BEGIN
	       AdrCnt:=1; Inc(BAsmCode[CodeLen],2);
	      END;
	     BAsmCode[CodeLen+1]:=BAsmCode[CodeLen+1] SHR 3+$c0+(z SHL 3);
	     MoveAdr(2);
	     Inc(CodeLen,2+AdrCnt);
	    END;
	  ELSE IF AdrType<>TypeNone THEN WrError(1350);
	  END;
	 END;
	TypeMem:
	 BEGIN
	  BAsmCode[CodeLen+1]:=AdrMode;
	  AdrByte:=AdrCnt; MoveAdr(2);
	  DecodeAdr(ArgStr[2]);
	  CASE AdrType OF
	  TypeReg8,TypeReg16:
	   BEGIN
	    BAsmCode[CodeLen]:=z SHL 3+OpSize;
	    Inc(BAsmCode[CodeLen+1],AdrMode SHL 3);
	    Inc(CodeLen,2+AdrByte);
	   END;
	  TypeImm:
	   BEGIN
	    BAsmCode[CodeLen]:=OpSize+$80;
	    IF (OpSize=1) AND (Sgn(AdrVals[0])=AdrVals[1]) THEN
	     BEGIN
	      AdrCnt:=1; Inc(BAsmCode[CodeLen],2);
	     END;
	    Inc(BAsmCode[CodeLen+1],z SHL 3);
	    MoveAdr(2+AdrByte);
	    Inc(CodeLen,2+AdrCnt+AdrByte);
	   END;
	  ELSE IF AdrType<>TypeNone THEN WrError(1350);
	  END;
	 END;
	ELSE IF AdrType<>TypeNone THEN WrError(1350);
	END;
       END;
      Goto AddPrefixes;
     END;

   IF (Memo('INC')) OR (Memo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeReg16:
	BEGIN
	 BAsmCode[CodeLen]:=$40+AdrMode;
	 IF Memo('DEC') THEN Inc(BAsmCode[CodeLen],8);
	 Inc(CodeLen);
	END;
       TypeReg8:
	BEGIN
	 BAsmCode[CodeLen]:=$fe;
	 BAsmCode[CodeLen+1]:=$c0+AdrMode;
	 IF Memo('DEC') THEN Inc(BAsmCode[CodeLen+1],8);
	 Inc(CodeLen,2);
	END;
       TypeMem:
	BEGIN
	 IF MinOneIs0 THEN;
	 IF OpSize=-1 THEN WrError(1132)
	 ELSE
	  BEGIN
	   BAsmCode[CodeLen]:=$fe+OpSize;
	   BAsmCode[CodeLen+1]:=AdrMode;
	   IF Memo('DEC') THEN Inc(BAsmCode[CodeLen+1],8);
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
	END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Goto AddPrefixes;
    END;

   FOR z:=0 TO MulOrderCnt-1 DO
    IF Memo(MulOrders[z+1]) THEN
     BEGIN
      CASE ArgCnt OF
      1:BEGIN
	 DecodeAdr(ArgStr[1]);
	 CASE AdrType OF
	 TypeReg8,TypeReg16:
	  BEGIN
	   BAsmCode[CodeLen]:=$f6+OpSize;
	   BAsmCode[CodeLen+1]:=$e0+(z SHL 3)+AdrMode;
	   Inc(CodeLen,2);
	  END;
	 TypeMem:
	  BEGIN
	   IF MinOneIs0 THEN;
	   IF OpSize=-1 THEN WrError(1132)
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=$f6+OpSize;
	     BAsmCode[CodeLen+1]:=$20+(z SHL 3)+AdrMode;
	     MoveAdr(2);
	     Inc(CodeLen,2+AdrCnt);
	    END;
	  END;
	 ELSE IF AdrType<>TypeNone THEN WrError(1350);
	 END;
	END;
      2,3:IF MomCPU<CPU80186 THEN WrError(1500)
	  ELSE IF NOT Memo('IMUL') THEN WrError(1110)
	  ELSE
	   BEGIN
	    IF ArgCnt=2 THEN
	     BEGIN
	      ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1]; Inc(ArgCnt);
	     END;
	    BAsmCode[CodeLen]:=$69;
	    DecodeAdr(ArgStr[1]);
	    CASE AdrType OF
	    TypeReg16:
	     BEGIN
	      BAsmCode[CodeLen+1]:=AdrMode SHL 3;
	      DecodeAdr(ArgStr[2]);
	      IF AdrType=TypeReg16 THEN
	       BEGIN
		AdrType:=TypeMem; Inc(AdrMode,$c0);
	       END;
	      CASE AdrType OF
	      TypeMem:
	       BEGIN
		Inc(BAsmCode[CodeLen+1],AdrMode);
		MoveAdr(2);
		AdrWord:=EvalIntExpression(ArgStr[3],Int16,OK);
		IF OK THEN
		 BEGIN
		  BAsmCode[CodeLen+2+AdrCnt]:=Lo(AdrWord);
		  BAsmCode[CodeLen+3+AdrCnt]:=Hi(AdrWord);
		  Inc(CodeLen,2+AdrCnt+SizeOf(Word));
		  IF (AdrWord>=$ff80) OR (AdrWord<$80) THEN
		   BEGIN
		    Dec(CodeLen);
		    Inc(BAsmCode[CodeLen-AdrCnt-Sizeof(Word)-1],2);
		   END;
		 END;
	       END;
              ELSE IF AdrType<>TypeNone THEN WrError(1350);
              END;
	     END;
            ELSE IF AdrType<>TypeNone THEN WrError(1350);
            END;
	   END;
      ELSE WrError(1110);
      END;
      Goto AddPrefixes;
     END;

   IF Memo('INT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[CodeLen+1]:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
	IF BAsmCode[1]=3 THEN
	 BEGIN
	  BAsmCode[CodeLen]:=$cc; Inc(CodeLen);
	 END
	ELSE
	 BEGIN
	  BAsmCode[CodeLen]:=$cd; Inc(CodeLen,2);
	 END;
      END;
     Goto AddPrefixes;
    END;

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF MomCPU-CPU8086<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
         IF OK THEN
	  BEGIN
	   ChkSpace(SegCode);
           Dec(AdrWord,EProgCounter+2);
           IF Hi(Code)<>0 THEN Dec(AdrWord);
           IF (AdrWord>=$80) AND (AdrWord<$ff80) AND (NOT SymbolQuestionable) THEN WrError(1370)
	   ELSE
	    BEGIN
	     PutCode(Code); BAsmCode[CodeLen]:=Lo(AdrWord); Inc(CodeLen);
	    END;
	  END;
	END;
       Exit;
      END;

   IF (Memo('IN')) OR (Memo('OUT')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('OUT') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeReg8,TypeReg16:
        IF AdrMode<>0 THEN WrError(1350)
        ELSE IF NLS_StrCaseCmp(ArgStr[2],'DX')=0 THEN
	 BEGIN
	  BAsmCode[CodeLen]:=$ec+OpSize;
	  IF Memo('OUT') THEN Inc(BAsmCode[CodeLen],2);
	  Inc(CodeLen);
	 END
	ELSE
	 BEGIN
	  BAsmCode[CodeLen+1]:=EvalIntExpression(ArgStr[2],Int8,OK);
	  IF OK THEN
	   BEGIN
	    ChkSpace(SegIO);
	    BAsmCode[CodeLen]:=$e4+OpSize;
	    IF Memo('OUT') THEN Inc(BAsmCode[CodeLen],2);
	    Inc(CodeLen,2);
	   END;
         END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Goto AddPrefixes;
    END;

   IF (Memo('CALL')) OR (Memo('JMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF Copy(ArgStr[1],1,6)='SHORT ' THEN
        BEGIN
         AdrByte:=2; Delete(ArgStr[1],1,6); KillPrefBlanks(ArgStr[1]);
        END
       ELSE IF (Copy(ArgStr[1],1,5)='LONG ') OR (Copy(ArgStr[1],1,5)='NEAR ') THEN
        BEGIN
         AdrByte:=1; Delete(ArgStr[1],1,5); KillPrefBlanks(ArgStr[1]);
        END
       ELSE AdrByte:=0;
       OK:=True;
       IF Memo('CALL') THEN
        IF AdrByte=2 THEN
         BEGIN
          WrError(1350); OK:=False;
         END
        ELSE AdrByte:=1;

       IF OK THEN
        BEGIN
         OpSize:=1; DecodeAdr(ArgStr[1]);
         CASE AdrType OF
         TypeReg16:
          BEGIN
           BAsmCode[0]:=$ff;
           BAsmCode[1]:=$c0+AdrMode;
           IF Memo('CALL') THEN Inc(BAsmCode[1],$10)
                           ELSE Inc(BAsmCode[1],$20);
           CodeLen:=2;
          END;
         TypeMem:
          BEGIN
           BAsmCode[0]:=$ff;
           BAsmCode[1]:=AdrMode;
           IF Memo('CALL') THEN Inc(BAsmCode[1],$10)
                           ELSE Inc(BAsmCode[1],$20);
           MoveAdr(2);
           CodeLen:=2+AdrCnt;
          END;
         TypeImm:
          BEGIN
           ChkSpace(SegCode);
           AdrWord:=(AdrVals[1] SHL 8)+AdrVals[0];
           IF (AdrByte=2) OR ((AdrByte=0) AND (AbleToSign(AdrWord-EProgCounter-2))) THEN
            BEGIN
             Dec(AdrWord,EProgCounter+2);
             IF NOT AbleToSign(AdrWord) THEN WrError(1330)
             ELSE
              BEGIN
               BAsmCode[0]:=$eb;
               BAsmCode[1]:=Lo(AdrWord);
               CodeLen:=2;
              END;
            END
           ELSE
            BEGIN
             Dec(AdrWord,EProgCounter+3);
             ChkSpace(SegCode);
             BAsmCode[0]:=$e8; IF Memo('JMP') THEN Inc(BAsmCode[CodeLen]);
             BAsmCode[1]:=Lo(AdrWord);
             BAsmCode[2]:=Hi(AdrWord);
             CodeLen:=3;
             Inc(AdrWord);
            END;
          END;
         ELSE IF AdrType<>TypeNone THEN WrError(1350);
         END;
        END;
      END;
     Goto AddPrefixes;
    END;

   FOR z:=1 TO ModRegOrderCnt DO
    WITH ModRegOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       NoSegCheck:=Memo('LEA');
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF MomCPU-CPU8086<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1]);
	 CASE AdrType OF
	 TypeReg16:
	  BEGIN
	   IF Memo('LEA') THEN OpSize:=-1 ELSE OpSize:=2;
	   AdrByte:=AdrMode SHL 3;
	   DecodeAdr(ArgStr[2]);
	   CASE AdrType OF
	   TypeMem:
	    BEGIN
	     PutCode(Code);
	     BAsmCode[CodeLen]:=AdrByte+AdrMode;
	     MoveAdr(1);
	     Inc(CodeLen,1+AdrCnt);
	    END;
           ELSE IF AdrType<>TypeNone THEN WrError(1350);
           END;
	  END;
         ELSE IF AdrType<>TypeNone THEN WrError(1350);
         END;
	END;
       Goto AddPrefixes;
      END;

   IF (Memo('NOT')) OR (Memo('NEG')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       IF MinOneIs0 THEN;
       BAsmCode[CodeLen]:=$f6+OpSize;
       BAsmCode[CodeLen+1]:=$10+Ord(Memo('NEG')) SHL 3;
       CASE AdrType OF
       TypeReg8,TypeReg16:
	BEGIN
	 Inc(BAsmCode[CodeLen+1],$c0+AdrMode);
	 Inc(CodeLen,2);
	END;
       TypeMem:
	BEGIN
	 IF OpSize=-1 THEN WrError(1132)
	 ELSE
	  BEGIN
	   Inc(BAsmCode[CodeLen+1],AdrMode);
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
	END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Goto AddPrefixes;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=1; DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeReg16:
	BEGIN
	 BAsmCode[CodeLen]:=$50+AdrMode;
	 IF Memo('POP') THEN Inc(BAsmCode[CodeLen],8);
	 Inc(CodeLen);
	END;
       TypeRegSeg:
	BEGIN
	 BAsmCode[CodeLen]:=$06+(AdrMode SHL 3);
	 IF Memo('POP') THEN Inc(BAsmCode[CodeLen]);
	 Inc(CodeLen);
	END;
       TypeMem:
	BEGIN
	 BAsmCode[CodeLen]:=$8f; BAsmCode[CodeLen+1]:=AdrMode;
	 IF Memo('PUSH') THEN
	  BEGIN
	   Inc(BAsmCode[CodeLen],$70);
	   Inc(BAsmCode[CodeLen+1],$30);
	  END;
	 MoveAdr(2);
	 Inc(CodeLen,2+AdrCnt);
	END;
       TypeImm:
	IF MomCPU<CPU80186 THEN WrError(1500)
	ELSE IF Memo('POP') THEN WrError(1350)
	ELSE
	 BEGIN
	  BAsmCode[CodeLen]:=$68;
	  BAsmCode[CodeLen+1]:=AdrVals[0];
	  IF Sgn(AdrVals[0])=AdrVals[1] THEN
	   BEGIN
	    Inc(BAsmCode[CodeLen],2); Inc(CodeLen,2);
	   END
	  ELSE
	   BEGIN
	    BAsmCode[CodeLen+2]:=AdrVals[1]; Inc(CodeLen,3);
	   END;
	 END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Goto AddPrefixes;
    END;

   FOR z:=1 TO ShiftOrderCnt DO
    WITH ShiftOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1]);
	 IF MinOneIs0 THEN;
	 IF OpSize=-1 THEN WrError(1132)
         ELSE CASE AdrType OF
         TypeReg8,TypeReg16,TypeMem:
	  BEGIN
	   BAsmCode[CodeLen]:=OpSize;
	   BAsmCode[CodeLen+1]:=AdrMode+(Code SHL 3);
	   IF AdrType<>TypeMem THEN Inc(BAsmCode[CodeLen+1],$c0);
	   MoveAdr(2);
           IF NLS_StrCaseCmp(ArgStr[2],'CL')=0 THEN
	    BEGIN
	     Inc(BAsmCode[CodeLen],$d2);
	     Inc(CodeLen,2+AdrCnt);
	    END
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen+2+AdrCnt]:=EvalIntExpression(ArgStr[2],Int8,OK);
	     IF OK THEN
	      IF BAsmCode[CodeLen+2+AdrCnt]=1 THEN
	       BEGIN
		Inc(BAsmCode[CodeLen],$d0);
		Inc(CodeLen,2+AdrCnt);
	       END
	      ELSE IF MomCPU<CPU80186 THEN WrError(1500)
	      ELSE
	       BEGIN
		Inc(BAsmCode[CodeLen],$c0);
		Inc(CodeLen,3+AdrCnt);
	       END;
	    END;
	  END;
          ELSE IF AdrType<>TypeNone THEN WrError(1350);
         END;
	END;
       Goto AddPrefixes;
      END;

   IF (Memo('RET')) OR (Memo('RETF')) THEN
    BEGIN
     IF ArgCnt>1 THEN WrError(1110)
     ELSE IF ArgCnt=0 THEN
      BEGIN
       BAsmCode[CodeLen]:=$c3;
       IF Memo('RETF') THEN Inc(BAsmCode[CodeLen],8);
       Inc(CodeLen);
      END
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[CodeLen]:=$c2;
	 IF Memo('RETF') THEN Inc(BAsmCode[CodeLen],8);
	 BAsmCode[CodeLen+1]:=Lo(AdrWord);
	 BAsmCode[CodeLen+2]:=Hi(AdrWord);
	 Inc(CodeLen,3);
	END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('TEST') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
       TypeReg8,TypeReg16:
	BEGIN
	 BAsmCode[CodeLen+1]:=AdrMode SHL 3;
	 DecodeAdr(ArgStr[2]);
	 CASE AdrType OF
	 TypeReg8,TypeReg16:
	  BEGIN
	   Inc(BAsmCode[CodeLen+1],$c0+AdrMode);
	   BAsmCode[CodeLen]:=$84+OpSize;
	   Inc(CodeLen,2);
	  END;
	 TypeMem:
	  BEGIN
	   Inc(BAsmCode[CodeLen+1],AdrMode);
	   BAsmCode[CodeLen]:=$84+OpSize;
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
	 TypeImm:
	 IF (BAsmCode[CodeLen+1] SHR 3) AND 7=0 THEN
	  BEGIN
	   BAsmCode[CodeLen]:=$a8+OpSize;
	   MoveAdr(1);
	   Inc(CodeLen,1+AdrCnt);
	  END
	 ELSE
	  BEGIN
	   BAsmCode[CodeLen]:=OpSize+$f6;
	   BAsmCode[CodeLen+1]:=BAsmCode[CodeLen+1] SHR 3+$c0;
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
	 ELSE IF AdrType<>TypeNone THEN WrError(1350);
	 END;
	END;
       TypeMem:
	BEGIN
	 BAsmCode[CodeLen+1]:=AdrMode;
	 AdrByte:=AdrCnt; MoveAdr(2);
	 DecodeAdr(ArgStr[2]);
	 CASE AdrType OF
	 TypeReg8,TypeReg16:
	  BEGIN
	   BAsmCode[CodeLen]:=$84+OpSize;
	   Inc(BAsmCode[CodeLen+1],AdrMode SHL 3);
	   Inc(CodeLen,2+AdrByte);
	  END;
	 TypeImm:
	  BEGIN
	   BAsmCode[CodeLen]:=OpSize+$f6;
           MoveAdr(2+AdrByte);
	   Inc(CodeLen,2+AdrCnt+AdrByte);
	  END;
	 ELSE IF AdrType<>TypeNone THEN WrError(1350);
	 END;
	END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('XCHG') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrType OF
	TypeReg8,TypeReg16:
	 BEGIN
	  AdrByte:=AdrMode;
	  DecodeAdr(ArgStr[2]);
	  CASE AdrType OF
	  TypeReg8,TypeReg16:
	   IF (OpSize=1) AND ((AdrMode=0) OR (AdrByte=0)) THEN
	    BEGIN
	     BAsmCode[CodeLen]:=$90+AdrMode+AdrByte;
	     Inc(CodeLen);
	    END
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=$86+OpSize;
	     BAsmCode[CodeLen+1]:=AdrMode+$c0+(AdrByte SHL 3);
	     Inc(CodeLen,2);
	    END;
	  TypeMem:
	   BEGIN
	    BAsmCode[CodeLen]:=$86+OpSize;
	    BAsmCode[CodeLen+1]:=AdrMode+(AdrByte SHL 3);
	    MoveAdr(2);
	    Inc(CodeLen,AdrCnt+2);
	   END;
	  ELSE IF AdrType<>TypeNone THEN WrError(1350);
	  END;
	 END;
	TypeMem:
	 BEGIN
	  BAsmCode[CodeLen+1]:=AdrMode;
	  MoveAdr(2); AdrByte:=AdrCnt;
	  DecodeAdr(ArgStr[2]);
	  CASE AdrType OF
	  TypeReg8,TypeReg16:
	   BEGIN
	    BAsmCode[CodeLen]:=$86+OpSize;
	    Inc(BAsmCode[CodeLen+1],AdrMode SHL 3);
	    Inc(CodeLen,AdrByte+2);
	   END;
	  ELSE IF AdrType<>TypeNone THEN WrError(1350);
	  END;
	 END;
	ELSE IF AdrType<>TypeNone THEN WrError(1350);
	END;
      END;
     Goto AddPrefixes;
    END;

   IF (Memo('CALLF')) OR (Memo('JMPF')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrByte:=QuotPos(ArgStr[1],':');
       IF AdrByte-1=Length(ArgStr[1]) THEN
	BEGIN
	 DecodeAdr(ArgStr[1]);
	 CASE AdrType OF
	 TypeMem:
	  BEGIN
	   BAsmCode[CodeLen]:=$ff;
	   BAsmCode[CodeLen+1]:=AdrMode+$18;
	   IF Memo('JMPF') THEN Inc(BAsmCode[CodeLen+1],16);
	   MoveAdr(2);
	   Inc(CodeLen,2+AdrCnt);
	  END;
         ELSE IF AdrType<>TypeNone THEN WrError(1350);
         END;
	END
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(Copy(ArgStr[1],1,AdrByte-1),Int16,OK);
	 IF OK THEN
	  BEGIN
	   BAsmCode[CodeLen+3]:=Lo(AdrWord);
	   BAsmCode[CodeLen+4]:=Hi(AdrWord);
	   AdrWord:=EvalIntExpression(Copy(ArgStr[1],AdrByte+1,Length(ArgStr[1])-AdrByte),Int16,OK);
	   IF OK THEN
	    BEGIN
	     BAsmCode[CodeLen+1]:=Lo(AdrWord);
	     BAsmCode[CodeLen+2]:=Hi(AdrWord);
	     IF Memo('CALLF') THEN BAsmCode[CodeLen]:=$9a
			      ELSE BAsmCode[CodeLen]:=$ea;
	     Inc(CodeLen,5);
	    END;
	  END;
	END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('ENTER') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80186 THEN WrError(1500)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[CodeLen+1]:=Lo(AdrWord);
	 BAsmCode[CodeLen+2]:=Hi(AdrWord);
	 BAsmCode[CodeLen+3]:=EvalIntExpression(ArgStr[2],Int8,OK);
	 IF OK THEN
	  BEGIN
	   BAsmCode[CodeLen]:=$c8; Inc(CodeLen,4);
	  END;
	END;
      END;
     Goto AddPrefixes;
    END;

   IF (Memo('ROL4')) OR (Memo('ROR4')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPUV30 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       BAsmCode[CodeLen  ]:=$0f;
       IF Memo('ROL4') THEN BAsmCode[CodeLen+1]:=$28
		       ELSE BAsmCode[CodeLen+1]:=$2a;
       CASE AdrType OF
       TypeReg8:
	BEGIN
	 BAsmCode[CodeLen+2]:=$c0+AdrMode;
	 Inc(CodeLen,3);
	END;
       TypeMem:
	BEGIN
	 BAsmCode[CodeLen+2]:=AdrMode;
	 MoveAdr(3);
	 Inc(CodeLen,3+AdrCnt);
	END;
       ELSE IF AdrType<>TypeNone THEN WrError(1350);
       END;
      END;
     Goto AddPrefixes;
    END;

   FOR z:=0 TO Bit1OrderCnt-1 DO
    IF Memo(Bit1Orders[z+1]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF MomCPU<CPUV30 THEN WrError(1500)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1]);
	IF (AdrType=TypeReg8) OR (AdrType=TypeReg16) THEN
	 BEGIN
	  Inc(AdrMode,$c0); AdrType:=TypeMem;
	 END;
	IF MinOneIs0 THEN;
	IF OpSize=-1 THEN WrError(1132)
	ELSE CASE AdrType OF
	TypeMem:
	 BEGIN
	  BAsmCode[CodeLen  ]:=$0f;
	  BAsmCode[CodeLen+1]:=$10+(z SHL 1)+OpSize;
	  BAsmCode[CodeLen+2]:=AdrMode;
	  MoveAdr(3);
          IF NLS_StrCaseCmp(ArgStr[2],'CL')=0 THEN Inc(CodeLen,3+AdrCnt)
	  ELSE
	   BEGIN
	    Inc(BAsmCode[CodeLen+1],8);
	    BAsmCode[CodeLen+3+AdrCnt]:=EvalIntExpression(ArgStr[2],Int4,OK);
	    IF OK THEN Inc(CodeLen,4+AdrCnt);
	   END;
	 END;
        ELSE IF AdrType<>TypeNone THEN WrError(1350);
        END;
       END;
      Goto AddPrefixes;
     END;

   IF (Memo('INS')) OR (Memo('EXT')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPUV30 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       IF (AdrType<>TypeReg8) AND (AdrType<>TypeNone) THEN WrError(1350)
       ELSE
	BEGIN
	 BAsmCode[CodeLen  ]:=$0f;
	 BAsmCode[CodeLen+1]:=$31;
	 IF Memo('EXT') THEN Inc(BAsmCode[CodeLen+1],2);
	 BAsmCode[CodeLen+2]:=$c0+AdrMode;
	 DecodeAdr(ArgStr[2]);
	 CASE AdrType OF
	 TypeReg8:
	  BEGIN
	   Inc(BasmCode[CodeLen+2],AdrMode SHL 3);
	   Inc(CodeLen,3);
	  END;
	 TypeImm:
	  IF AdrVals[0]>15 THEN WrError(1320)
	  ELSE
	   BEGIN
	    Inc(BAsmCode[CodeLen+1],8);
	    BAsmCode[CodeLen+3]:=AdrVals[1];
	    Inc(CodeLen,4);
	   END;
	 ELSE IF AdrType<>TypeNone THEN WrError(1350);
	 END;
	END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('FPO2') THEN
    BEGIN
     IF (ArgCnt=0) OR (ArgCnt>2) THEN WrError(1110)
     ELSE IF MomCPU<CPUV30 THEN WrError(1500)
     ELSE
      BEGIN
       AdrByte:=EvalIntExpression(ArgStr[1],Int4,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[CodeLen  ]:=$66+(AdrByte SHR 3);
	 BAsmCode[CodeLen+1]:=(AdrByte AND 7) SHL 3;
	 IF ArgCnt=1 THEN
	  BEGIN
	   Inc(BAsmCode[CodeLen+1],$c0);
	   Inc(CodeLen,2);
	  END
	 ELSE
	  BEGIN
	   DecodeAdr(ArgStr[2]);
	   CASE AdrType OF
	   TypeReg8:
	    BEGIN
	     Inc(BAsmCode[CodeLen+1],$c0+AdrMode);
	     Inc(CodeLen,2);
	    END;
	   TypeMem:
	    BEGIN
	     Inc(BAsmCode[CodeLen+1],AdrMode);
	     MoveAdr(2);
	     Inc(CodeLen,2+AdrCnt);
	    END;
	   ELSE IF AdrType<>TypeNone THEN WrError(1350);
	   END;
	  END;
	END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('BTCLR') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF MomCPU<CPUV35 THEN WrError(1500)
     ELSE
      BEGIN
       BAsmCode[CodeLen  ]:=$0f;
       BAsmCode[CodeLen+1]:=$9c;
       BAsmCode[CodeLen+2]:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
	BEGIN
         BAsmCode[CodeLen+3]:=EvalIntExpression(ArgStr[2],UInt3,OK);
         IF OK THEN
          BEGIN
           AdrWord:=EvalIntExpression(ArgStr[3],Int16,OK)-(EProgCounter+5);
           IF (OK) THEN
            IF (NOT SymbolQuestionable) AND ((AdrWord>$7f) AND (AdrWord<$ff80)) THEN WrError(1330)
            ELSE
             BEGIN
              BAsmCode[CodeLen+4]:=Lo(AdrWord);
              Inc(CodeLen,5);
             END;
          END;
	END;
      END;
     Goto AddPrefixes;
    END;

   FOR z:=1 TO Reg16OrderCnt DO
    WITH Reg16Orders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF MomCPU-CPU8086<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1]);
	 CASE AdrType OF
	 TypeReg16:
	  BEGIN
	   PutCode(Code);
	   BAsmCode[CodeLen]:=Add+AdrMode; Inc(CodeLen);
	  END;
         ELSE IF AdrType<>TypeNone THEN WrError(1350);
         END;
	END;
       Goto AddPrefixes;
      END;


   WrXError(1200,OpPart); Exit;

AddPrefixes:
   IF (CodeLen<>0) AND (PrefixLen<>0) THEN
    BEGIN
     Move(BAsmCode[0],BAsmCode[PrefixLen],CodeLen);
     Move(Prefixes,BAsmCode[0],PrefixLen);
     Inc(CodeLen,PrefixLen);
    END;
END;

	PROCEDURE InitCode_86;
	Far;
BEGIN
   SaveInitProc;
   SegAssumes[0]:=SegNone; { ASSUME ES:NOTHING }
   SegAssumes[1]:=SegCode; { ASSUME CS:CODE }
   SegAssumes[2]:=SegNone; { ASSUME SS:NOTHING }
   SegAssumes[3]:=SegData; { ASSUME DS:DATA }
END;

	FUNCTION ChkPC_86:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter < $10000;
   SegData  : ok:=ProgCounter < $10000;
   SegXData : ok:=ProgCounter < $10000;
   SegIO    : ok:=ProgCounter < $10000;
   ELSE ok:=False;
   END;
   ChkPC_86:=(ok) AND (ProgCounter>=0);
END;

	FUNCTION IsDef_86:Boolean;
	Far;
BEGIN
   IsDef_86:=OpPart='PORT';
END;

        PROCEDURE SwitchFrom_86;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_86;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$42; NOPCode:=$90;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData,SegXData,SegIO];
   Grans[SegCode ]:=1; ListGrans[SegCode ]:=1; SegInits[SegCode ]:=0;
   Grans[SegData ]:=1; ListGrans[SegData ]:=1; SegInits[SegData ]:=0;
   Grans[SegXData]:=1; ListGrans[SegXData]:=1; SegInits[SegXData]:=0;
   Grans[SegIO   ]:=1; ListGrans[SegIO   ]:=1; SegInits[SegIO   ]:=0;

   MakeCode:=MakeCode_86; ChkPC:=ChkPC_86; IsDef:=IsDef_86;
   SwitchFrom:=SwitchFrom_86;
END;

BEGIN
   CPU8086 :=AddCPU('8086' ,SwitchTo_86);
   CPU80186:=AddCPU('80186',SwitchTo_86);
   CPUV30  :=AddCPU('V30'  ,SwitchTo_86);
   CPUV35  :=AddCPU('V35'  ,SwitchTo_86);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_86;
END.
