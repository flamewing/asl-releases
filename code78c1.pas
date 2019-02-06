{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code78C1;

{ AS Codegeneratormodul uPD8C10..uPD78C14 }

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
   FixedOrderCnt=23;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
               ((Name:'EXX'  ; Code:$0011),
		(Name:'EXA'  ; Code:$0010),
                (Name:'EXH'  ; Code:$0050),
                (Name:'BLOCK'; Code:$0031),
                (Name:'TABLE'; Code:$48a8),
                (Name:'DAA'  ; Code:$0061),
                (Name:'STC'  ; Code:$482b),
                (Name:'CLC'  ; Code:$482a),
                (Name:'NEGA' ; Code:$483a),
		(Name:'RLD'  ; Code:$4838),
                (Name:'RRD'  ; Code:$4839),
                (Name:'JB'   ; Code:$0021),
                (Name:'JEA'  ; Code:$4828),
                (Name:'CALB' ; Code:$4829),
		(Name:'SOFTI'; Code:$0072),
		(Name:'RET'  ; Code:$00b8),
		(Name:'RETS' ; Code:$00b9),
		(Name:'RETI' ; Code:$0062),
		(Name:'NOP'  ; Code:$0000),
		(Name:'EI'   ; Code:$00aa),
		(Name:'DI'   ; Code:$00ba),
		(Name:'HLT'  ; Code:$483b),
		(Name:'STOP' ; Code:$48bb));

   ALUOrderCnt=15;
   ALUOrderCodes:ARRAY[1..ALUOrderCnt] OF Byte=
		 (10, 8, 4, 1,15, 5, 7,13,11, 9, 3,14,12, 6, 2);
   ALUOrderImmOps:ARRAY[1..ALUOrderCnt] OF String[5]=
		  ('ACI'  ,'ADI'  ,'ADINC','ANI'  ,'EQI'  ,
		   'GTI'  ,'LTI'  ,'NEI'  ,'OFFI' ,'ONI'  ,
		   'ORI'  ,'SBI'  ,'SUI'  ,'SUINB','XRI'  );
   ALUOrderRegOps:ARRAY[1..ALUOrderCnt] OF String[5]=
		  ('ADC'  ,'ADD'  ,'ADDNC','ANA'  ,'EQA'  ,
		   'GTA'  ,'LTA'  ,'NEA'  ,'OFFA' ,'ONA'  ,
		   'ORA'  ,'SBB'  ,'SUB'  ,'SUBNB','XRA'  );
   ALUOrderEAOps:ARRAY[1..ALUOrderCnt] OF String[6]=
		  ('DADC'  ,'DADD'  ,'DADDNC','DAN'   ,'DEQ'   ,
		   'DGT'   ,'DLT'   ,'DNE'   ,'DOFF'  ,'DON'   ,
		   'DOR'   ,'DSBB'  ,'DSUB'  ,'DSUBNB','DXR'   );

   AbsOrderCnt=10;
   AbsOrders:ARRAY[1..AbsOrderCnt] OF FixedOrder=
	     ((Name:'CALL'; Code:$0040),(Name:'JMP' ; Code:$0054),
	      (Name:'LBCD'; Code:$701f),(Name:'LDED'; Code:$702f),
	      (Name:'LHLD'; Code:$703f),(Name:'LSPD'; Code:$700f),
	      (Name:'SBCD'; Code:$701e),(Name:'SDED'; Code:$702e),
	      (Name:'SHLD'; Code:$703e),(Name:'SSPD'; Code:$700e));

   Reg2OrderCnt=10;
   Reg2Orders:ARRAY[1..Reg2OrderCnt] OF FixedOrder=
	      ((Name:'DCR' ; Code:$0050),(Name:'DIV' ; Code:$483c),
	       (Name:'INR' ; Code:$0040),(Name:'MUL' ; Code:$482c),
	       (Name:'RLL' ; Code:$4834),(Name:'RLR' ; Code:$4830),
	       (Name:'SLL' ; Code:$4824),(Name:'SLR' ; Code:$4820),
	       (Name:'SLLC'; Code:$4804),(Name:'SLRC'; Code:$4800));

   WorkOrderCnt=4;
   WorkOrders:ARRAY[1..WorkOrderCnt] OF FixedOrder=
	      ((Name:'DCRW'; Code:$33),(Name:'INRW'; Code:$20),
	       (Name:'LDAW'; Code:$01),(Name:'STAW'; Code:$63));

   EAOrderCnt=4;
   EAOrders:ARRAY[1..EAOrderCnt] OF FixedOrder=
	    ((Name:'DRLL'; Code:$48b4),(Name:'DRLR'; Code:$48b0),
	     (Name:'DSLL'; Code:$48a4),(Name:'DSLR'; Code:$48a0));

VAR
   WorkArea:LongInt;

   SaveInitProc:PROCEDURE;

   CPU7810,CPU78C10:CPUVar;


	FUNCTION Decode_r(VAR Asc:String; VAR Erg:ShortInt):Boolean;
CONST
   Names:String[8]='VABCDEHL';
BEGIN
   IF Length(Asc)<>1 THEN Decode_r:=False
   ELSE
    BEGIN
     Erg:=Pos(UpCase(Asc[1]),Names)-1; Decode_r:=Erg<>-1;
    END;
END;

	FUNCTION Decode_r1(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_r1:=True;
   IF NLS_StrCaseCmp(Asc,'EAL')=0 THEN Erg:=1
   ELSE IF NLS_StrCaseCmp(Asc,'EAH')=0 THEN Erg:=0
   ELSE
    BEGIN
     Decode_r1:=False;
     IF NOT Decode_r(Asc,Erg) THEN Exit;
     Decode_r1:=Erg>1;
    END;
END;

	FUNCTION Decode_r2(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_r2:=False;
   IF NOT Decode_r(Asc,Erg) THEN Exit;
   Decode_r2:=(Erg>0) AND (Erg<4);
END;

	FUNCTION Decode_rp2(VAR Asc:String; VAR Erg:ShortInt):Boolean;
CONST
   RegCnt=5;
   Regs:ARRAY[0..RegCnt-1] OF String[2]=('SP','B','D','H','EA');
BEGIN
   Erg:=0;
   WHILE (Erg<RegCnt) AND (NLS_StrCaseCmp(Regs[Erg],Asc)<>0) Do Inc(Erg);
   Decode_rp2:=Erg<RegCnt;
END;

	FUNCTION Decode_rp(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_rp:=False;
   IF NOT Decode_rp2(Asc,Erg) THEN Exit;
   Decode_rp:=Erg<4;
END;

	FUNCTION Decode_rp1(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_rp1:=True;
   IF NLS_StrCaseCmp(Asc,'VA')=0 THEN Erg:=0
   ELSE
    BEGIN
     Decode_rp1:=False;
     IF NOT Decode_rp2(Asc,Erg) THEN Exit;
     Decode_rp1:=Erg<>0;
    END;
END;

	FUNCTION Decode_rp3(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_rp3:=False;
   IF NOT Decode_rp2(Asc,Erg) THEN Exit;
   Decode_rp3:=(Erg<4) AND (Erg>0);
END;

	FUNCTION Decode_rpa2(VAR Asc:String; VAR Erg,Disp:ShortInt):Boolean;
CONST
   OpCnt=13;
   OpNames:ARRAY[1..OpCnt] OF String[4]=('B','D','H','D+','H+','D-','H-',
					 'H+A','A+H','H+B','B+H','H+EA','EA+H');
   OpCodes:ARRAY[1..OpCnt] OF Byte=(1,2,3,4,5,6,7,12,12,13,13,14,14);
VAR
   z,p,pm:Integer;
   OK:Boolean;
BEGIN
   Decode_rpa2:=False;
   FOR z:=1 TO OpCnt DO
    IF NLS_StrCaseCmp(Asc,OpNames[z])=0 THEN
     BEGIN
      Decode_rpa2:=True; Erg:=OpCodes[z]; Exit;
     END;

   p:=QuotPos(Asc,'+'); pm:=QuotPos(Asc,'-'); IF pm<p THEN p:=pm;
   IF p>Length(Asc) THEN Exit;

   IF p=2 THEN
    CASE UpCase(Asc[1]) OF
    'H':Erg:=15;
    'D':Erg:=11;
    ELSE Exit;
    END
   ELSE Exit;
   Disp:=EvalIntExpression(Copy(Asc,p,Length(Asc)-p+1),SInt8,OK);
   Decode_rpa2:=OK;
END;

	FUNCTION Decode_rpa(VAR Asc:String; VAR Erg:ShortInt):Boolean;
VAR
   Dummy:ShortInt;
BEGIN
   Decode_rpa:=False;
   IF NOT Decode_rpa2(Asc,Erg,Dummy) THEN Exit;
   Decode_rpa:=Erg<=7;
END;

	FUNCTION Decode_rpa1(VAR Asc:String; VAR Erg:ShortInt):Boolean;
VAR
   Dummy:ShortInt;
BEGIN
   Decode_rpa1:=False;
   IF NOT Decode_rpa2(Asc,Erg,Dummy) THEN Exit;
   Decode_rpa1:=Erg<=3;
END;

	FUNCTION Decode_rpa3(VAR Asc:String; VAR Erg,Disp:ShortInt):Boolean;
BEGIN
   Decode_rpa3:=True;
   IF NLS_StrCaseCmp(Asc,'D++')=0 THEN Erg:=4
   ELSE IF NLS_StrCaseCmp(Asc,'H++')=0 THEN Erg:=5
   ELSE
    BEGIN
     Decode_rpa3:=False;
     IF NOT Decode_rpa2(Asc,Erg,Disp) THEN Exit;
     Decode_rpa3:=(Erg=2) OR (Erg=3) OR (Erg>=8);
    END;
END;

	FUNCTION Decode_f(VAR Asc:String; VAR Erg:ShortInt):Boolean;
CONST
   FlagCnt=3;
   Flags:ARRAY[1..FlagCnt] OF String[2]=('CY','HC','Z');
BEGIN
   Erg:=1;
   WHILE (Erg<=FlagCnt) AND (NLS_StrCaseCmp(Asc,Flags[Erg])<>0) DO Inc(Erg);
   Inc(Erg); Decode_f:=Erg<=4;
END;

	FUNCTION Decode_sr0(VAR Asc:String; VAR Erg:ShortInt):Boolean;
TYPE
   SReg=RECORD
	 Name:String[4];
	 Code:Byte;
	END;
CONST
   RegCnt=28;
   RegNames:ARRAY[1..RegCnt] OF SReg=
	    ((Name:'PA'  ; Code:$00),(Name:'PB'  ; Code:$01),
	     (Name:'PC'  ; Code:$02),(Name:'PD'  ; Code:$03),
	     (Name:'PF'  ; Code:$05),(Name:'MKH' ; Code:$06),
	     (Name:'MKL' ; Code:$07),(Name:'ANM' ; Code:$08),
	     (Name:'SMH' ; Code:$09),(Name:'SML' ; Code:$0a),
	     (Name:'EOM' ; Code:$0b),(Name:'ETNM'; Code:$0c),
	     (Name:'TMM' ; Code:$0d),(Name:'MM'  ; Code:$10),
	     (Name:'MCC' ; Code:$11),(Name:'MA'  ; Code:$12),
	     (Name:'MB'  ; Code:$13),(Name:'MC'  ; Code:$14),
	     (Name:'MF'  ; Code:$17),(Name:'TXB' ; Code:$18),
	     (Name:'RXB' ; Code:$19),(Name:'TM0' ; Code:$1a),
	     (Name:'TM1' ; Code:$1b),(Name:'CR0' ; Code:$20),
	     (Name:'CR1' ; Code:$21),(Name:'CR2' ; Code:$22),
	     (Name:'CR3' ; Code:$23),(Name:'ZCM' ; Code:$28));
VAR
   z:Integer;
BEGIN
   z:=1; WHILE (z<=RegCnt) AND (NLS_StrCaseCmp(Asc,RegNames[z].Name)<>0) DO Inc(z);
   Decode_sr0:=z<=RegCnt;
   IF (MomCPU=CPU7810) AND (z=RegCnt) THEN
    BEGIN
     WrError(1440); Decode_sr0:=False;
    END;
   IF z<=RegCnt THEN Erg:=RegNames[z].Code;
END;

	FUNCTION Decode_sr1(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_sr1:=False; IF NOT Decode_sr0(Asc,Erg) THEN Exit;
   Decode_sr1:=Erg IN [0..9,11,13,25,32..35];
END;

	FUNCTION Decode_sr(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_sr:=False; IF NOT Decode_sr0(Asc,Erg) THEN Exit;
   Decode_sr:=Erg IN [0..24,26,27,40];
END;

	FUNCTION Decode_sr2(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_sr2:=False; IF NOT Decode_sr0(Asc,Erg) THEN Exit;
   Decode_sr2:=((Erg>=0) AND (Erg<=9)) OR (Erg=11) OR (Erg=13);
END;

        FUNCTION Decode_sr3(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_sr3:=True;
   IF NLS_StrCaseCmp(Asc,'ETM0')=0 THEN Erg:=0
   ELSE IF NLS_StrCaseCmp(Asc,'ETM1')=0 THEN Erg:=1
   ELSE Decode_sr3:=False;
END;

        FUNCTION Decode_sr4(VAR Asc:String; VAR Erg:ShortInt):Boolean;
BEGIN
   Decode_sr4:=True;
   IF NLS_StrCaseCmp(Asc,'ECNT')=0 THEN Erg:=0
   ELSE IF NLS_StrCaseCmp(Asc,'ECPT')=0 THEN Erg:=1
   ELSE Decode_sr4:=False;
END;

	FUNCTION Decode_irf(VAR Asc:String; VAR Erg:ShortInt):Boolean;
CONST
   FlagCnt=18;
   FlagNames:ARRAY[1..FlagCnt] OF String[4]=
	     ('NMI' ,'FT0' ,'FT1' ,'F1'  ,'F2'  ,'FE0' ,
	      'FE1' ,'FEIN','FAD' ,'FSR' ,'FST' ,'ER'  ,
	      'OV'  ,'AN4' ,'AN5' ,'AN6' ,'AN7' ,'SB'  );
   FlagCodes:ARRAY[1..FlagCnt+1] OF ShortInt=
	     (0,1,2,3,4,5,6,7,8,9,10,11,12,16,17,18,19,20,-1);
BEGIN
   Erg:=1;
   WHILE (Erg<=FlagCnt) AND (NLS_StrCaseCmp(Asc,FlagNames[Erg])<>0) DO Inc(Erg);
   Decode_irf:=Erg<=FlagCnt;
   Erg:=FlagCodes[Erg];
END;

	FUNCTION Decode_wa(VAR Asc:String; VAR Erg:Byte):Boolean;
VAR
   Adr:Word;
   OK:Boolean;
BEGIN
   Decode_wa:=False;
   FirstPassUnknown:=False;
   Adr:=EvalIntExpression(Asc,Int16,OK); IF NOT OK THEN Exit;
   IF (FirstPassUnknown) AND (Hi(Adr)<>WorkArea) THEN WrError(110);
   Erg:=Lo(Adr);
   Decode_wa:=True;
END;

	FUNCTION HasDisp(Mode:ShortInt):Boolean;
BEGIN
   HasDisp:=Mode AND 11=11;
END;

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME78C10Count=1;
   ASSUME78C10s:ARRAY[1..ASSUME78C10Count] OF ASSUMERec=
		((Name:'V' ; Dest:@WorkArea; Min:0; Max:$ff; NothingVal:$100));
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME78C10s,ASSUME78C10Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_78C10;
	Far;
VAR
   z,AdrInt:Integer;
   HVal8,HReg:ShortInt;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF (Memo('STOP') AND (MomCPU=CPU7810)) THEN WrError(1500)
       ELSE IF Hi(Code)=0 THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=Lo(Code);
	END
       ELSE
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
	END;
       Exit;
      END;

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN
      IF Decode_sr1(ArgStr[2],HReg) THEN
       BEGIN
	CodeLen:=2; BAsmCode[0]:=$4c;
	BAsmCode[1]:=$c0+HReg;
       END
      ELSE IF Decode_r1(ArgStr[2],HReg) THEN
       BEGIN
	CodeLen:=1; BAsmCode[0]:=$08+HReg;
       END
      ELSE WrError(1350)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'A')=0 THEN
      IF Decode_sr(ArgStr[1],HReg) THEN
       BEGIN
	CodeLen:=2; BAsmCode[0]:=$4d;
	BAsmCode[1]:=$c0+HReg;
       END
      ELSE IF Decode_r1(ArgStr[1],HReg) THEN
       BEGIN
	CodeLen:=1; BAsmCode[0]:=$18+HReg;
       END
      ELSE WrError(1350)
     ELSE IF Decode_r(ArgStr[1],HReg) THEN
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=4; BAsmCode[0]:=$70; BAsmCode[1]:=$68+HReg;
	 BAsmCode[2]:=Lo(AdrInt); BAsmCode[3]:=Hi(AdrInt);
	END;
      END
     ELSE IF Decode_r(ArgStr[2],HReg) THEN
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=4; BAsmCode[0]:=$70; BAsmCode[1]:=$78+HReg;
	 BAsmCode[2]:=Lo(AdrInt); BAsmCode[3]:=Hi(AdrInt);
	END;
      END
     ELSE WrError(1350);
     Exit;
    END;

   IF Memo('MVI') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
	IF Decode_r(ArgStr[1],HReg) THEN
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$68+HReg;
	 END
	ELSE IF Decode_sr2(ArgStr[1],HReg) THEN
	 BEGIN
	  CodeLen:=3; BAsmCode[2]:=BAsmCode[1]; BAsmCode[0]:=$64;
	  BAsmCode[1]:=(HReg AND 7)+((HReg AND 8) SHL 4);
	 END
	ELSE WrError(1350);
      END;
     Exit;
    END;

   IF Memo('MVIW') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF Decode_wa(ArgStr[1],BAsmCode[1]) THEN
      BEGIN
       BAsmCode[2]:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$71;
	END;
      END;
     Exit;
    END;

   IF Memo('MVIX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT Decode_rpa1(ArgStr[1],HReg) THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=$48+HReg; CodeLen:=2;
	END;
      END;
     Exit;
    END;

   IF (Memo('LDAX')) OR (Memo('STAX')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT Decode_rpa2(ArgStr[1],HReg,ShortInt(BAsmCode[1])) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1+Ord(HasDisp(HReg));
       BAsmCode[0]:=$28+(Ord(Memo('STAX')) SHL 4)+((HReg AND 8) SHL 4)+(HReg AND 7);
      END;
     Exit;
    END;

   IF (Memo('LDEAX')) OR (Memo('STEAX')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT Decode_rpa3(ArgStr[1],HReg,ShortInt(BAsmCode[2])) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=2+Ord(HasDisp(HReg)); BAsmCode[0]:=$48;
       BAsmCode[1]:=$80+(Ord(Memo('STEAX')) SHL 4)+HReg;
      END;
     Exit;
    END;

   IF Memo('LXI') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT Decode_rp2(ArgStr[1],HReg) THEN WrError(1350)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$04+(HReg SHL 4);
	 BAsmCode[1]:=Lo(AdrInt); BAsmCode[2]:=Hi(AdrInt);
	END;
      END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT Decode_rp1(ArgStr[1],HReg) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1;
       BAsmCode[0]:=$a0+(Ord(Memo('PUSH')) SHL 4)+HReg;
      END;
     Exit;
    END;

   IF Memo('DMOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[1],'EA')<>0 THEN
        BEGIN
         ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
         OK:=True;
	END
       ELSE OK:=False;
       IF NLS_StrCaseCmp(ArgStr[1],'EA')<>0 THEN WrError(1350)
       ELSE IF Decode_rp3(ArgStr[2],HReg) THEN
        BEGIN
         CodeLen:=1; BAsmCode[0]:=$a4+HReg;
         IF OK THEN Inc(BAsmCode[0],$10);
        END
       ELSE IF ((OK) AND (Decode_sr3(ArgStr[2],HReg)))
            OR ((NOT OK) AND (Decode_sr4(ArgStr[2],HReg))) THEN
        BEGIN
         CodeLen:=2; BAsmCode[0]:=$48; BAsmCode[1]:=$c0+HReg;
         IF OK THEN Inc(BAsmCode[1],$12);
        END
       ELSE WrError(1350);
      END;
     Exit;
    END;

   FOR z:=1 TO AluOrderCnt DO
    IF Memo(AluOrderImmOps[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	HVal8:=EvalIntExpression(ArgStr[2],Int8,OK);
	IF OK THEN
         IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=$06+((AluOrderCodes[z] AND 14) SHL 3)+(AluOrderCodes[z] AND 1);
	   BAsmCode[1]:=HVal8;
	  END
	 ELSE IF Decode_r(ArgStr[1],HReg) THEN
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$74; BAsmCode[2]:=HVal8;
	   BAsmCode[1]:=HReg+(AluOrderCodes[z] SHL 3);
	  END
	 ELSE IF Decode_sr2(ArgStr[1],HReg) THEN
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$64; BAsmCode[2]:=HVal8;
	   BAsmCode[1]:=(HReg AND 7)+(AluOrderCodes[z] SHL 3)+((HReg AND 8) SHL 4);
	  END
	 ELSE WrError(1350);
       END;
      Exit;
     END
    ELSE IF Memo(AluOrderRegOps[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN
	 BEGIN
	  ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	  OK:=False;
	 END
	ELSE OK:=True;
        IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
	ELSE IF NOT Decode_r(ArgStr[2],HReg) THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$60;
	  BAsmCode[1]:=(ALUOrderCodes[z] SHL 3)+HReg;
	  IF (OK) OR (Memo('ONA')) OR (Memo('OFFA')) THEN
	   Inc(BAsmCode[1],$80);
	 END;
       END;
      Exit;
     END
    ELSE IF Memo(ALUOrderRegOps[z]+'W') THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE IF Decode_wa(ArgStr[1],BAsmCode[2]) THEN
       BEGIN
	CodeLen:=3; BAsmCode[0]:=$74;
	BAsmCode[1]:=$80+(ALUOrderCodes[z] SHL 3);
       END;
      Exit;
     END
    ELSE IF Memo(ALUOrderRegOps[z]+'X') THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE IF NOT Decode_rpa(ArgStr[1],HReg) THEN WrError(1350)
      ELSE
       BEGIN
	CodeLen:=2; BAsmCode[0]:=$70;
	BAsmCode[1]:=$80+(ALUOrderCodes[z] SHL 3)+HReg;
       END;
      Exit;
     END
    ELSE IF Memo(ALUOrderEAOps[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF NLS_StrCaseCmp(ArgStr[1],'EA')<>0 THEN WrError(1350)
      ELSE IF NOT Decode_rp3(ArgStr[2],HReg) THEN WrError(1350)
      ELSE
       BEGIN
	CodeLen:=2; BAsmCode[0]:=$74;
	BAsmCode[1]:=$84+(ALUOrderCodes[z] SHL 3)+HReg;
       END;
      Exit;
     END
    ELSE IF (Memo(ALUOrderImmOps[z]+'W')) AND (Odd(ALUOrderCodes[z])) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF Decode_wa(ArgStr[1],BAsmCode[1]) THEN
       BEGIN
	BAsmCode[2]:=EvalIntExpression(ArgStr[2],Int8,OK);
	IF OK THEN
	 BEGIN
	  CodeLen:=3;
	  BAsmCode[0]:=$05+((AluOrderCodes[z] SHR 1) SHL 4);
	 END;
       END;
      Exit;
     END;

   FOR z:=1 TO AbsOrderCnt DO
    WITH AbsOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK);
	 IF OK THEN
	  IF Hi(Code)=0 THEN
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=Code;
	    BAsmCode[1]:=Lo(AdrInt); BAsmCode[2]:=Hi(AdrInt);
	   END
	  ELSE
	   BEGIN
	    CodeLen:=4; BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
	    BAsmCode[2]:=Lo(AdrInt); BAsmCode[3]:=Hi(AdrInt);
	   END;
	END;
       Exit;
      END;

   FOR z:=1 TO Reg2OrderCnt DO
    WITH Reg2Orders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT Decode_r2(ArgStr[1],HReg) THEN WrError(1350)
       ELSE IF Hi(Code)=0 THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=Lo(Code)+HReg;
	END
       ELSE
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=Hi(Code);
	 BAsmCode[1]:=Lo(Code)+HReg;
	END;
       Exit;
      END;

   FOR z:=1 TO WorkOrderCnt DO
    WITH WorkOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF Decode_wa(ArgStr[1],BAsmCode[1]) THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=Code;
	END;
       Exit;
      END;

   IF (Memo('DCX')) OR (Memo('INX')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'EA')=0 THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$a8+Ord(Memo('DCX'));
      END
     ELSE IF Decode_rp(ArgStr[1],HReg) THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$02+Ord(Memo('DCX'))+(HReg SHL 4);
      END
     ELSE WrError(1350);
     Exit;
    END;

   IF (Memo('EADD')) OR (Memo('ESUB')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'EA')<>0 THEN WrError(1350)
     ELSE IF NOT Decode_r2(ArgStr[2],HReg) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$70;
       BAsmCode[1]:=$40+(Ord(Memo('ESUB')) SHL 5)+HReg;
      END;
     Exit;
    END;

   IF (Memo('JR')) OR (Memo('JRE')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK)-(EProgCounter+1+Ord(Memo('JRE')));
       IF OK THEN
	IF Memo('JR') THEN
	 IF (AdrInt<-32) OR (AdrInt>31) THEN WrError(1370)
	 ELSE
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$c0+(AdrInt AND $3f);
	  END
	ELSE
	 IF (AdrInt<-256) OR (AdrInt>255) THEN WrError(1370)
	 ELSE
	  BEGIN
	   IF (AdrInt>=-32) AND (AdrInt<=31) THEN WrError(20);
	   CodeLen:=2; BAsmCode[0]:=$4e+(Hi(AdrInt) AND 1);
	   BAsmCode[1]:=Lo(AdrInt);
	  END;
      END;
     Exit;
    END;

   IF Memo('CALF') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF (NOT FirstPassUnknown) AND (AdrInt SHR 11<>1) THEN WrError(1905)
	ELSE
	 BEGIN
	  CodeLen:=2;
	  BAsmCode[0]:=Hi(AdrInt)+$70; BAsmCode[1]:=Lo(AdrInt);
	 END;
      END;
     Exit;
    END;

   IF Memo('CALT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF (NOT FirstPassUnknown) AND (AdrInt AND $ffc1<>$80) THEN WrError(1905)
	ELSE
	 BEGIN
	  CodeLen:=1;
	  BAsmCode[0]:=$80+((AdrInt AND $3f) SHR 1);
	 END;
      END;
     Exit;
    END;

   IF Memo('BIT') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       HReg:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF OK THEN
	IF Decode_wa(ArgStr[2],BAsmCode[1]) THEN
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$58+HReg;
	 END;
      END;
     Exit;
    END;

   FOR z:=1 TO EAOrderCnt DO
    WITH EAOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'EA')<>0 THEN WrError(1350)
       ELSE
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
	END;
       Exit;
      END;

   IF (Memo('SK')) OR (Memo('SKN')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT Decode_f(ArgStr[1],HReg) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$48;
       BAsmCode[1]:=$08+(Ord(Memo('SKN')) SHL 4)+HReg;
      END;
     Exit;
    END;

   IF (Memo('SKIT')) OR (Memo('SKINT')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT Decode_irf(ArgStr[1],HReg) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$48;
       BAsmCode[1]:=$40+(Ord(Memo('SKINT')) SHL 5)+HReg;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_78C10;
	Far;
BEGIN
   SaveInitProc;
   WorkArea:=$100;
END;

	FUNCTION ChkPC_78C10:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<$10000;
   ELSE ok:=False;
   END;
   ChkPC_78C10:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_78C10:Boolean;
	Far;
BEGIN
   IsDef_78C10:=False;
END;

        PROCEDURE SwitchFrom_78C10;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_78C10;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$7a; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_78C10; ChkPC:=ChkPC_78C10; IsDef:=IsDef_78C10;
   SwitchFrom:=SwitchFrom_78C10;
END;

BEGIN
   CPU7810 :=AddCPU('7810' ,SwitchTo_78C10);
   CPU78C10:=AddCPU('78C10',SwitchTo_78C10);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_78C10;
END.


