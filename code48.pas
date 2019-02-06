{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code48;

INTERFACE
        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   CondOrder=RECORD
	      Name:String[5];
	      Code:Byte;
	      May2X:Byte;
	      UPIFlag:Byte;
	     END;

   AccOrder=RECORD
	     Name:String[4];
	     Code:Byte;
	    END;

   SelOrder=RECORD
	     Name:String[4];
	     Code:Byte;
	     Is22:Boolean;
	     IsNUPI:Boolean;
	    END;

CONST
   ModImm=0;
   ModReg=1;
   ModInd=2;
   ModAcc=3;
   ModNone=-1;

   ClrCplCnt=4;
   ClrCplVals:ARRAY[1..ClrCplCnt] OF String[2]=('A','C','F0','F1');
   ClrCplCodes:ARRAY[1..ClrCplCnt] OF Byte=($27,$97,$85,$a5);

   CondOrderCnt=22;
   CondOrders:ARRAY[1..CondOrderCnt] OF CondOrder=
	      ((Name:'JTF'  ;Code:$16; May2X:2; UPIFlag:3),
	       (Name:'JNI'  ;Code:$86; May2X:0; UPIFlag:2),
	       (Name:'JC'   ;Code:$f6; May2X:2; UPIFlag:3),
	       (Name:'JNC'  ;Code:$e6; May2X:2; UPIFlag:3),
	       (Name:'JZ'   ;Code:$c6; May2X:2; UPIFlag:3),
	       (Name:'JNZ'  ;Code:$96; May2X:2; UPIFlag:3),
	       (Name:'JT0'  ;Code:$36; May2X:1; UPIFlag:3),
	       (Name:'JNT0' ;Code:$26; May2X:1; UPIFlag:3),
	       (Name:'JT1'  ;Code:$56; May2X:2; UPIFlag:3),
	       (Name:'JNT1' ;Code:$46; May2X:2; UPIFlag:3),
	       (Name:'JF0'  ;Code:$b6; May2X:0; UPIFlag:3),
	       (Name:'JF1'  ;Code:$76; May2X:0; UPIFlag:3),
	       (Name:'JNIBF';Code:$d6; May2X:2; UPIFlag:1),
	       (Name:'JOBF' ;Code:$86; May2X:2; UPIFlag:1),
	       (Name:'JB0'  ;Code:$12; May2X:0; UPIFlag:3),
	       (Name:'JB1'  ;Code:$32; May2X:0; UPIFlag:3),
	       (Name:'JB2'  ;Code:$52; May2X:0; UPIFlag:3),
	       (Name:'JB3'  ;Code:$72; May2X:0; UPIFlag:3),
	       (Name:'JB4'  ;Code:$92; May2X:0; UPIFlag:3),
	       (Name:'JB5'  ;Code:$b2; May2X:0; UPIFlag:3),
	       (Name:'JB6'  ;Code:$d2; May2X:0; UPIFlag:3),
	       (Name:'JB7'  ;Code:$f2; May2X:0; UPIFlag:3));

   AccOrderCnt=6;
   AccOrders:ARRAY[1..AccOrderCnt] OF AccOrder=
	     ((Name:'DA'  ;Code:$57),
	      (Name:'RL'  ;Code:$e7),
	      (Name:'RLC' ;Code:$f7),
	      (Name:'RR'  ;Code:$77),
	      (Name:'RRC' ;Code:$67),
	      (Name:'SWAP';Code:$47));

   SelOrderCnt=6;
   SelOrders:ARRAY[1..SelOrderCnt] OF SelOrder=
	     ((Name:'MB0' ;Code:$e5; Is22:False; IsNUPI:True ),
	      (Name:'MB1' ;Code:$f5; Is22:False; IsNUPI:True ),
	      (Name:'RB0' ;Code:$c5; Is22:False; IsNUPI:False),
	      (Name:'RB1' ;Code:$d5; Is22:False; IsNUPI:False),
	      (Name:'AN0' ;Code:$95; Is22:True ; IsNUPI:False),
	      (Name:'AN1' ;Code:$85; Is22:True ; IsNUPI:False));

   D_CPU8021=0;
   D_CPU8022=1;
   D_CPU8039=2;
   D_CPU8048=3;
   D_CPU80C39=4;
   D_CPU80C48=5;
   D_CPU8041=6;
   D_CPU8042=7;

VAR
   AdrMode:ShortInt;
   AdrVal:Byte;

   CPU8021,CPU8022,CPU8039,CPU8048,CPU80C39,CPU80C48,CPU8041,CPU8042:CPUVar;

	PROCEDURE DecodeAdr(Asc:String);
VAR
   OK:Boolean;
BEGIN
   AdrMode:=ModNone;

   IF Asc='' THEN Exit;

   IF NLS_StrCaseCmp(Asc,'A')=0 THEN AdrMode:=ModAcc

   ELSE IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     AdrVal:=EvalIntExpression(Asc,Int8,OK);
     IF OK THEN
      BEGIN
       AdrMode:=ModImm; BAsmCode[1]:=AdrVal;
      END;
    END

   ELSE IF (Length(Asc)=2) AND (UpCase(Asc[1])='R') THEN
    BEGIN
     IF (Asc[2]>='0') AND (Asc[2]<='7') THEN
      BEGIN
       AdrMode:=ModReg; AdrVal:=Ord(Asc[2])-AscOfs;
      END;
    END

   ELSE IF (Length(Asc)=3) AND (Asc[1]='@') AND (UpCase(Asc[2])='R') THEN
    BEGIN
     IF (Asc[3]>='0') AND (Asc[3]<='1') THEN
      BEGIN
       AdrMode:=ModInd; AdrVal:=Ord(Asc[3])-AscOfs;
      END;
    END;
END;

	PROCEDURE ChkN802X;
BEGIN
   IF CodeLen=0 THEN Exit;
   IF (MomCPU=CPU8021) OR (MomCPU=CPU8022) THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    END;
END;

	PROCEDURE Chk802X;
BEGIN
   IF CodeLen=0 THEN Exit;
   IF (MomCPU<>CPU8021) AND (MomCPU<>CPU8022) THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    END;
END;

	PROCEDURE ChkNUPI;
BEGIN
   IF CodeLen=0 THEN Exit;
   IF (MomCPU=CPU8041) OR (MomCPU=CPU8042) THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    END;
END;

	PROCEDURE ChkUPI;
BEGIN
   IF CodeLen=0 THEN Exit;
   IF (MomCPU<>CPU8041) AND (MomCPU<>CPU8042) THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    END;
END;

	PROCEDURE ChkExt;
BEGIN
   IF CodeLen=0 THEN Exit;
   IF (MomCPU=CPU8039) OR (MomCPU=CPU80C39) THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_48;
	Far;
VAR
   OK:Boolean;
   AdrWord:Word;
   z:Integer;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   IF (Memo('ADD')) OR (Memo('ADDC')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1135)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2]);
       IF (AdrMode=-1) OR (AdrMode=ModAcc) THEN WrError(1350)
       ELSE
	BEGIN
         CASE AdrMode OF
         ModImm:BEGIN
                 CodeLen:=2; BAsmCode[0]:=$03;
                END;
	 ModReg:BEGIN
	         CodeLen:=1; BAsmCode[0]:=$68+AdrVal;
		END;
         ModInd:BEGIN
                 CodeLen:=1; BAsmCode[0]:=$60+AdrVal;
                END;
         END;
         IF Length(OpPart)=4 THEN Inc(BAsmCode[0],$10);
        END;
      END;
     Exit;
    END;

   IF (Memo('ANL')) OR (Memo('ORL')) OR (Memo('XRL')) THEN
    BEGIN
     NLS_UpString(ArgStr[1]);
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN
      BEGIN
       DecodeAdr(ArgStr[2]);
       IF (AdrMode=-1) OR (AdrMode=ModAcc) THEN WrError(1350)
       ELSE
	BEGIN
	 CASE AdrMode OF
	 ModImm:BEGIN
		 CodeLen:=2; BAsmCode[0]:=$43;
		END;
	 ModReg:BEGIN
		 CodeLen:=1; BAsmCode[0]:=$48+AdrVal;
		END;
	 ModInd:BEGIN
		 CodeLen:=1; BAsmCode[0]:=$40+AdrVal;
		END;
	 END;
	 IF Memo('ANL') THEN Inc(BAsmCode[0],$10)
	 ELSE IF Memo('XRL') THEN Inc(BAsmCode[0],$90);
	END;
      END
     ELSE IF (ArgStr[1]='BUS') OR (ArgStr[1]='P1') OR (ArgStr[1]='P2') THEN
      BEGIN
       IF Memo('XRL') THEN WrError(1135)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2]);
	 IF AdrMode<>ModImm THEN WrError(1350)
	 ELSE
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$88;
           IF UpCase(ArgStr[1][1])='P' THEN Inc(BAsmCode[0],Ord(ArgStr[1][2])-AscOfs);
	   IF Memo('ANL') THEN Inc(BAsmCode[0],$10);
           IF NLS_StrCaseCmp(ArgStr[1],'BUS')=0 THEN
	    BEGIN
	     ChkExt; ChkNUPI;
	    END;
	   ChkN802X;
	  END;
	END;
      END
     ELSE WrError(1135);
     Exit;
    END;

   IF (Memo('CALL')) OR (Memo('JMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF EProgCounter AND $7fe=$7fe THEN WrError(1900)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
        IF AdrWord>$fff THEN WrError(1320)
        ELSE
         BEGIN
          IF EProgCounter AND $800<>AdrWord AND $800 THEN
	   BEGIN
            BAsmCode[0]:=$e5+((AdrWord AND $800) SHR 7); CodeLen:=1;
           END;
          BAsmCode[CodeLen+1]:=Lo(AdrWord);
          BAsmCode[CodeLen]:=$04+(Hi(AdrWord AND $7ff) SHL 5);
          IF Memo('CALL') THEN Inc(BAsmCode[CodeLen],$10);
	  Inc(CodeLen,2); ChkSpace(SegCode);
         END;
      END;
     Exit;
    END;

   IF (Memo('CLR')) OR (Memo('CPL')) THEN
    BEGIN
     NLS_UpString(ArgStr[1]);
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; OK:=FALSE;
       REPEAT
        IF ClrCplVals[z]=ArgStr[1] THEN
	 BEGIN
          CodeLen:=1; BAsmCode[0]:=ClrCplCodes[z]; OK:=True;
          IF ArgStr[1][1]='F' THEN ChkN802X;
         END;
        Inc(z);
       UNTIL (z>ClrCplCnt) OR (CodeLen=1);
       IF NOT OK THEN WrError(1135)
       ELSE IF Memo('CPL') THEN Inc(BAsmCode[0],$10);
      END;
     Exit;
    END;

   FOR z:=1 TO AccOrderCnt DO
    WITH AccOrders[z] DO
     IF (Memo(Name)) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1135)
       ELSE
        BEGIN
	 CodeLen:=1; BAsmCode[0]:=Code;
        END;
       Exit;
      END;

   IF (Memo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModAcc:BEGIN
               CodeLen:=1; BAsmCode[0]:=$07;
              END;
       ModReg:BEGIN
               CodeLen:=1; BAsmCode[0]:=$c8+AdrVal;
	       ChkN802X;
              END;
       ELSE WrError(1135);
       END;
      END;
     Exit;
    END;

   IF (Memo('DIS')) OR (Memo('EN')) THEN
    BEGIN
     NLS_UpString(ArgStr[1]);
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1]='I' THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$05;
      END
     ELSE IF ArgStr[1]='TCNTI' THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$25;
      END
     ELSE IF (Memo('EN')) AND (ArgStr[1]='DMA') THEN
      BEGIN
       BAsmCode[0]:=$e5; CodeLen:=1; ChkUPI;
      END
     ELSE IF (Memo('EN')) AND (ArgStr[1]='FLAGS') THEN
      BEGIN
       BAsmCode[0]:=$f5; CodeLen:=1; ChkUPI;
      END
     ELSE WrError(1135);
     IF CodeLen<>0 THEN
      BEGIN
       IF Memo('DIS') THEN Inc(BAsmCode[0],$10);
       IF MomCPU=CPU8021 THEN
        BEGIN
         WrError(1500); CodeLen:=0;
        END;
      END;
     Exit;
    END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       IF AdrMode<>ModReg THEN WrError(1135)
       ELSE
        BEGIN
         AdrWord:=EvalIntExpression(ArgStr[2],Int16,OK);
         IF OK THEN
          BEGIN
           IF Succ(EProgCounter) SHR 8<>Hi(AdrWord) THEN WrError(1910)
           ELSE
	    BEGIN
             CodeLen:=2; BAsmCode[0]:=$e8+AdrVal; BAsmCode[1]:=Lo(AdrWord);
            END;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('ENT0') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CLK')<>0 THEN WrError(1135)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$75;
       ChkN802X; ChkNUPI;
      END;
     Exit;
    END;

   IF Memo('INC') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModAcc:BEGIN
               CodeLen:=1; BAsmCode[0]:=$17;
	      END;
       ModReg:BEGIN
               CodeLen:=1; BAsmCode[0]:=$18+AdrVal;
              END;
       ModInd:BEGIN
               CodeLen:=1; BAsmCode[0]:=$10+AdrVal;
              END;
       ELSE WrError(1135);
       END;
      END;
     Exit;
    END;

   IF Memo('IN') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1135)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'DBB')=0 THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$22; ChkUPI;
      END
     ELSE IF (Length(ArgStr[2])<>2) OR (UpCase(ArgStr[2][1])<>'P') THEN WrError(1135)
     ELSE CASE ArgStr[2][2] OF
     '0'..'2':BEGIN
               CodeLen:=1; BAsmCode[0]:=$08+(Ord(ArgStr[2][2])-AscOfs);
               IF ArgStr[2][2]='0' THEN Chk802X;
              END;
     ELSE WrError(1135);
     END;
     Exit;
    END;

   IF Memo('INS') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1135)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'BUS')<>0 THEN WrError(1135)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$08; ChkExt; ChkNUPI;
      END;
     Exit;
    END;

   IF Memo('JMPP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'@A')<>0 THEN WrError(1135)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$b3;
      END;
     Exit;
    END;

   FOR z:=1 TO CondOrderCnt DO
    WITH CondOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
	 IF NOT OK THEN
	 ELSE IF AdrWord>$fff THEN WrError(1320)
	 ELSE IF EProgCounter SHR 8<>Hi(AdrWord) THEN WrError(1910)
	 ELSE
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=Code; BAsmCode[1]:=Lo(AdrWord);
	   ChkSpace(SegCode);
	   IF May2X=0 THEN ChkN802X
	   ELSE IF (May2X=1) AND (MomCPU=CPU8021) THEN
	    BEGIN
	     WrError(1500); CodeLen:=0;
	    END;
	   IF UPIFlag=1 THEN ChkUPI
	   ELSE IF UPIFlag=2 THEN ChkNUPI;
          END;
        END;
       Exit;
      END;

   IF Memo('JB') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrVal:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
	IF (AdrVal<0) OR (AdrVal>7) THEN WrError(1320)
	ELSE
	 BEGIN
	  AdrWord:=EvalIntExpression(ArgStr[2],Int16,OK);
	  IF NOT OK THEN
	  ELSE IF AdrWord>$fff THEN WrError(1320)
	  ELSE IF (EProgCounter+2) SHR 8<>Hi(AdrWord) THEN WrError(1910)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$12+(AdrVal SHL 5);
	    BAsmCode[1]:=Lo(AdrWord);
	    ChkN802X;
	   END;
	 END;
      END;
     Exit;
    END;

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN
       BEGIN
        IF NLS_StrCaseCmp(ArgStr[2],'T')=0 THEN
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$42;
	 END
        ELSE IF NLS_StrCaseCmp(ArgStr[2],'PSW')=0 THEN
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$c7; ChkN802X;
	 END
	ELSE
	 BEGIN
	  DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModReg:BEGIN
		  CodeLen:=1; BAsmCode[0]:=$f8+AdrVal;
		 END;
	  ModInd:BEGIN
		  CodeLen:=1; BAsmCode[0]:=$f0+AdrVal;
		 END;
	  ModImm:BEGIN
		  CodeLen:=2; BAsmCode[0]:=$23;
		 END;
	  ELSE WrError(1135);
	  END;
	 END;
       END
      ELSE IF NLS_StrCaseCmp(ArgStr[2],'A')=0 THEN
       BEGIN
        IF NLS_StrCaseCmp(ArgStr[1],'STS')=0 THEN
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$90; ChkUPI;
	 END
        ELSE IF NLS_StrCaseCmp(ArgStr[1],'T')=0 THEN
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$62;
	 END
        ELSE IF NLS_StrCaseCmp(ArgStr[1],'PSW')=0 THEN
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$d7; ChkN802X;
	 END
	ELSE
	 BEGIN
	  DecodeAdr(ArgStr[1]);
	  CASE AdrMode OF
	  ModReg:BEGIN
		  CodeLen:=1; BAsmCode[0]:=$a8+AdrVal;
		 END;
	  ModInd:BEGIN
		  CodeLen:=1; BAsmCode[0]:=$a0+AdrVal;
		 END;
	  ELSE WrError(1135);
	  END;
	 END;
       END
      ELSE IF ArgStr[2][1]='#' THEN
       BEGIN
        Delete(ArgStr[2],1,1);
        AdrWord:=EvalIntExpression(ArgStr[2],Int8,OK);
        IF OK THEN
         BEGIN
          DecodeAdr(ArgStr[1]);
          CASE AdrMode OF
          ModReg:BEGIN
                  CodeLen:=2; BAsmCode[0]:=$b8+AdrVal; BAsmCode[1]:=AdrWord;
                 END;
	  ModInd:BEGIN
                  CodeLen:=2; BAsmCode[0]:=$b0+AdrVal; BAsmCode[1]:=AdrWord;
                 END;
	  ELSE WrError(1135);
	  END;
         END;
       END
      ELSE WrError(1135);
     Exit;
    END;

   IF (Memo('ANLD')) OR (Memo('ORLD')) OR (Memo('MOVD')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=False;
       IF (Memo('MOVD')) AND (NLS_StrCaseCmp(ArgStr[1],'A')=0) THEN
        BEGIN
         ArgStr[1]:=ArgStr[2]; ArgStr[2]:='A'; OK:=True;
        END;
       IF NLS_StrCaseCmp(ArgStr[2],'A')<>0 THEN WrError(1135)
       ELSE IF (Length(ArgStr[1])<>2) OR (UpCase(ArgStr[1][1])<>'P') THEN WrError(1135)
       ELSE IF (ArgStr[1][2]<'4') OR (ArgStr[1][2]>'7') THEN WrError(1320)
       ELSE
        BEGIN
         CodeLen:=1; BAsmCode[0]:=$0c+Ord(ArgStr[1][2])-Ord('4');
         IF Memo('ANLD') THEN Inc(BAsmCode[0],$90)
         ELSE IF Memo('ORLD') THEN Inc(BAsmCode[0],$80)
         ELSE IF NOT OK THEN Inc(BAsmCode[0],$30);
         ChkN802X;
        END;
      END;
     Exit;
    END;

   IF (Memo('MOVP')) OR (Memo('MOVP3')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) OR (NLS_StrCaseCmp(ArgStr[2],'@A')<>0) THEN WrError(1135)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$a3;
       IF Memo('MOVP3') THEN Inc(BAsmCode[0],$40);
       ChkN802X;
      END;
     Exit;
    END;

   IF Memo('MOVX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=False;
       IF NLS_StrCaseCmp(ArgStr[2],'A')=0 THEN
	BEGIN
	 ArgStr[2]:=ArgStr[1]; ArgStr[1]:='A'; OK:=True;
	END;
       IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1135)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2]);
	 IF AdrMode<>ModInd THEN WrError(1135)
	 ELSE
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$80+AdrVal;
	   IF OK THEN Inc(BAsmCode[0],$10);
	   ChkN802X; ChkNUPI;
	  END;
        END;
      END;
     Exit;
    END;

   IF Memo('NOP') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$00;
      END;
     Exit;
    END;

   IF Memo('OUT') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'DBB')<>0 THEN WrError(1135)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'A')<>0 THEN WrError(1135)
     ELSE
      BEGIN
       BAsmCode[0]:=$02; CodeLen:=1; ChkUPI;
      END;
     Exit;
    END;

   IF Memo('OUTL') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'A')<>0 THEN WrError(1135)
     ELSE
      BEGIN
       NLS_UpString(ArgStr[1]);
       IF ArgStr[1]='BUS' THEN
        BEGIN
         CodeLen:=1; BAsmCode[0]:=$02;
         ChkN802X; ChkExt; ChkNUPI;
        END
       ELSE IF ArgStr[1]='P0' THEN
        BEGIN
         CodeLen:=1; BAsmCode[0]:=$90;
        END
       ELSE IF (ArgStr[1]='P1') OR (ArgStr[1]='P2') THEN
        BEGIN
         CodeLen:=1; BAsmCode[0]:=$38+Ord(ArgStr[1][2])-AscOfs;
        END
       ELSE WrError(1135);
      END;
     Exit;
    END;

   IF (Memo('RET')) OR (Memo('RETR')) THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$83;
       IF Length(OpPart)=4 THEN
        BEGIN
	 Inc(BAsmCode[0],$10); ChkN802X;
        END;
      END;
     Exit;
    END;

   IF Memo('SEL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1135)
     ELSE IF MomCPU=CPU8021 THEN WrError(1500)
     ELSE
      BEGIN
       OK:=False;
       NLS_UpString(ArgStr[1]);
       FOR z:=1 TO SelOrderCnt DO
       WITH SelOrders[z] DO
       IF ArgStr[1]=Name THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=Code; OK:=True;
	 IF (Is22) AND (MomCPU<>CPU8022) THEN
	  BEGIN
	   CodeLen:=0; WrError(1500);
	  END;
	 IF (IsNUPI) THEN ChkNUPI;
        END;
       IF NOT OK THEN WrError(1135)
      END;
     Exit;
    END;

   IF Memo('STOP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'TCNT')<>0 THEN WrError(1135)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$65;
      END;
     Exit;
    END;

   IF Memo('STRT') THEN
    BEGIN
     NLS_UpString(ArgStr[1]);
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1]='CNT' THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$45;
      END
     ELSE IF ArgStr[1]='T' THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$55;
      END
     ELSE WrError(1135);
     Exit;
    END;

   IF Memo('XCH') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[2],'A')=0 THEN
        BEGIN
         ArgStr[2]:=ArgStr[1]; ArgStr[1]:='A';
        END;
       IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1135)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2]);
         CASE AdrMode OF
         ModReg:BEGIN
	         CodeLen:=1; BAsmCode[0]:=$28+AdrVal;
		END;
	 ModInd:BEGIN
	         CodeLen:=1; BAsmCode[0]:=$20+AdrVal;
		END;
	 ELSE WrError(1135);
	 END;
        END;
      END;
     Exit;
    END;

   IF Memo('XCHD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[2],'A')=0 THEN
        BEGIN
         ArgStr[2]:=ArgStr[1]; ArgStr[1]:='A';
        END;
       IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1135)
       ELSE
        BEGIN
	 DecodeAdr(ArgStr[2]);
         IF AdrMode<>ModInd THEN WrError(1135)
         ELSE
          BEGIN
           CodeLen:=1; BAsmCode[0]:=$30+AdrVal;
  	  END;
        END;
      END;
     Exit;
    END;

   IF Memo('RAD') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU<>CPU8022 THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$80;
      END;
     Exit;
    END;

   IF Memo('RETI') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU<>CPU8022 THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$93;
      END;
     Exit;
    END;

   IF Memo('IDL') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF (MomCPU<>CPU80C39) AND (MomCPU<>CPU80C48) THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$01;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_48:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : CASE MomCPU-CPU8021 OF
	      D_CPU8041:ok:=ProgCounter <  $400;
	      D_CPU8042:ok:=ProgCounter <  $800;
	      ELSE    ok:=ProgCounter < $1000;
	      END;
   SegXData,
   SegIData : ok:=ProgCounter <  $100;
   ELSE ok:=False;
   END;
   ChkPC_48:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_48:Boolean;
	Far;
BEGIN
   IsDef_48:=False;
END;

        PROCEDURE SwitchFrom_48;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_48;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$21; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegIData,SegXData];
   Grans[SegCode ]:=1; ListGrans[SegCode ]:=1; SegInits[SegCode ]:=0;
   Grans[SegIData]:=1; ListGrans[SegIData]:=1; SegInits[SegIData]:=$20;
   Grans[SegXData]:=1; ListGrans[SegXData]:=1; SegInits[SegXData]:=0;

   MakeCode:=MakeCode_48; ChkPC:=ChkPC_48; IsDef:=IsDef_48;
   SwitchFrom:=SwitchFrom_48;
END;

BEGIN
   CPU8021 :=AddCPU('8021' ,SwitchTo_48);
   CPU8022 :=AddCPU('8022' ,SwitchTo_48);
   CPU8039 :=AddCPU('8039' ,SwitchTo_48);
   CPU8048 :=AddCPU('8048' ,SwitchTo_48);
   CPU80C39:=AddCPU('80C39',SwitchTo_48);
   CPU80C48:=AddCPU('80C48',SwitchTo_48);
   CPU8041 :=AddCPU('8041' ,SwitchTo_48);
   CPU8042 :=AddCPU('8042' ,SwitchTo_48);
END.
