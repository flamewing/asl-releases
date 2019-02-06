{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
        UNIT CodePseu;

INTERFACE

        Uses Chunks,NLS,
	     AsmDef,AsmSub,AsmPars;

TYPE
   ASSUMERec=RECORD
	      Name:String[10];
	      Dest:^LongInt;
	      Min,Max:LongInt;
	      NothingVal:LongInt;
	     END;
   TASSUMEDef=ARRAY[1..1000] OF ASSUMERec;
   PASSUMEDef=^TASSUMEDef;

   ONOFFRec=RECORD
	     Name:String[20];
	     Dest:^Boolean;
	     FlagName:String[20];
	    END;
   TONOFFDef=ARRAY[1..1000] OF ONOFFRec;
   PONOFFDef=^TONOFFDef;


	FUNCTION  IsIndirect(Asc:String):Boolean;

	FUNCTION  DecodeIntelPseudo(Turn:Boolean):Boolean;

	FUNCTION  DecodeMotoPseudo(Turn:Boolean):Boolean;

	PROCEDURE ConvertDec(F:Extended; VAR w:WordField);

        FUNCTION  DecodeMoto16Pseudo(OpSize:ShortInt; Turn:Boolean):Boolean;

	PROCEDURE CodeEquate(DestSeg:ShortInt; Min,Max:LongInt);

	PROCEDURE CodeASSUME(Def:PASSUMEDef; Cnt:Integer);

	FUNCTION  CodeONOFF(Def:PONOFFDef; Cnt:Integer):Boolean;

IMPLEMENTATION

	FUNCTION IsIndirect(Asc:String):Boolean;
VAR
   z,Level:Integer;
BEGIN
   IF (Length(Asc)<=2) OR (Asc[1]<>'(') OR (Asc[Length(Asc)]<>')') THEN
    IsIndirect:=False
   ELSE
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);
     IsIndirect:=True; Level:=0;
     FOR z:=1 TO Length(Asc) DO
      BEGIN
       IF Asc[z]='(' THEN Inc(Level);
       IF Asc[z]=')' THEN Dec(Level);
       IF Level<0 THEN IsIndirect:=False;
      END;
    END
END;


VAR
   DSFlag:(DSNone,DSConstant,DSSpace);

	FUNCTION LayoutByte(Asc:String; VAR Cnt:Word; Turn:Boolean):Boolean;
	Far;
VAR
   OK:Boolean;
   z:Integer;
   t:TempResult;
BEGIN
   LayoutByte:=False;

   IF Asc='?' THEN
    BEGIN
     IF DSFlag=DSConstant THEN WrError(1930)
     ELSE
      BEGIN
       Cnt:=1; LayoutByte:=True; DSFlag:=DSSpace; Inc(CodeLen);
      END;
     Exit;
    END
   ELSE
    BEGIN
     IF DSFlag=DSSpace THEN
      BEGIN
       WrError(1930); Exit;
      END
     ELSE DSFlag:=DSConstant;
    END;

   FirstPassUnknown:=False; EvalExpression(Asc,t);
   CASE t.Typ OF
   TempInt   : BEGIN
		IF FirstPassUnknown THEN t.Int:=t.Int AND $ff;
		IF NOT RangeCheck(t.Int,Int8) THEN WrError(1320)
		ELSE
		 BEGIN
		  BAsmCode[CodeLen]:=t.Int; Inc(CodeLen); Cnt:=1;
		  LayoutByte:=True;
		 END;
	       END;
   TempFloat : WrError(1135);
   TempString: BEGIN
                TranslateString(t.Ascii);
                Move(t.Ascii[1],BAsmCode[CodeLen],Length(t.Ascii));
		Inc(CodeLen,Length(t.Ascii));
		Cnt:=Length(t.Ascii); LayoutByte:=True;
	       END;
   END;
END;

	FUNCTION LayoutWord(Asc:String; VAR Cnt:Word; Turn:Boolean):Boolean;
	Far;
VAR
   OK:Boolean;
   erg:Word;
BEGIN
   LayoutWord:=False; Cnt:=2;

   IF Asc='?' THEN
    BEGIN
     IF DSFlag=DSConstant THEN WrError(1930)
     ELSE
      BEGIN
       LayoutWord:=True; DSFlag:=DSSpace; Inc(CodeLen,2);
      END;
     Exit;
    END
   ELSE
    BEGIN
     IF DSFlag=DSSpace THEN
      BEGIN
       WrError(1930); Exit;
      END
     ELSE DSFlag:=DSConstant;
    END;

   IF CodeLen+2>MaxCodeLen THEN
    BEGIN
     WrError(1920); Exit;
    END;
   erg:=EvalIntExpression(Asc,Int16,OK);
   IF OK THEN
    BEGIN
     IF Turn THEN erg:=Swap(erg);
     BAsmCode[CodeLen]:=Lo(erg); BAsmCode[CodeLen+1]:=Hi(erg);
     Inc(CodeLen,2);
    END;
   LayoutWord:=OK;
END;

	FUNCTION LayoutDoubleWord(Asc:String; VAR Cnt:Word; Turn:Boolean):Boolean;
	Far;
VAR
   erg:TempResult;
   z:Integer;
   Exg:Byte;
   copy:Single;
BEGIN
   LayoutDoubleWord:=False; Cnt:=4;

   IF Asc='?' THEN
    BEGIN
     IF DSFlag=DSConstant THEN WrError(1930)
     ELSE
      BEGIN
       LayoutDoubleWord:=True; DSFlag:=DSSpace; Inc(CodeLen,4);
      END;
     Exit;
    END
   ELSE
    BEGIN
     IF DSFlag=DSSpace THEN
      BEGIN
       WrError(1930); Exit;
      END
     ELSE DSFlag:=DSConstant;
    END;

   IF CodeLen+4>MaxCodeLen THEN
    BEGIN
     WrError(1920); Exit;
    END;

   KillBlanks(Asc); EvalExpression(Asc,erg);
   CASE erg.Typ OF
   TempNone:Exit;
   TempInt:BEGIN
	    Move(erg.Int,BAsmCode[CodeLen],4);
	    Inc(CodeLen,4);
	   END;
   TempFloat:IF FloatRangeCheck(erg.Float,Float32) THEN
	      BEGIN
	       copy:=erg.Float; Move(copy,BAsmCode[CodeLen],4);
	       Inc(CodeLen,4);
	      END
	     ELSE WrError(1320);
   END;
   IF Turn THEN
    FOR z:=0 TO 1 DO
     BEGIN
      Exg:=BAsmCode[CodeLen-4+z];
      BAsmCode[CodeLen-4+z]:=BAsmCode[CodeLen-1-z];
      BAsmCode[CodeLen-1-z]:=Exg;
     END;
   LayoutDoubleWord:=True;
END;

	FUNCTION LayoutQuadWord(Asc:String; VAR Cnt:Word; Turn:Boolean):Boolean;
	Far;
VAR
   OK:Boolean;
   erg:TempResult;
   z:Integer;
   Exg:Byte;
   copy:Double;
BEGIN
   LayoutQuadWord:=False; Cnt:=8;

   IF Asc='?' THEN
    BEGIN
     IF DSFlag=DSConstant THEN WrError(1930)
     ELSE
      BEGIN
       LayoutQuadWord:=True; DSFlag:=DSSpace; Inc(CodeLen,8);
      END;
     Exit;
    END
   ELSE
    BEGIN
     IF DSFlag=DSSpace THEN
      BEGIN
       WrError(1930); Exit;
      END
     ELSE DSFlag:=DSConstant;
    END;

   IF CodeLen+8>MaxCodeLen THEN
    BEGIN
     WrError(1920); Exit;
    END;
   EvalExpression(Asc,erg);
   CASE erg.Typ OF
   TempNone:Exit;
   TempString:
    BEGIN
     WrError(1135); Exit;
    END;
   TempFloat:
    BEGIN
     copy:=erg.Float;
     Move(copy,BAsmCode[CodeLen],8);
     Inc(CodeLen,8);
    END;
   TempInt:
    BEGIN
     Move(Erg.Int,BAsmCode[CodeLen],4);
     FOR z:=4 TO 7 DO
      IF BAsmCode[CodeLen+3]>=$80 THEN BAsmCode[CodeLen+z]:=$ff
      ELSE BAsmCode[CodeLen+z]:=0;
     Inc(CodeLen,8);
    END;
   END;

   IF Turn THEN
    FOR z:=0 TO 3 DO
     BEGIN
      Exg:=BAsmCode[CodeLen-8+z];
      BAsmCode[CodeLen-8+z]:=BAsmCode[CodeLen-1-z];
      BAsmCode[CodeLen-1-z]:=Exg;
     END;

   LayoutQuadWord:=True;
END;

	FUNCTION LayoutTenBytes(Asc:String; VAR Cnt:Word; Turn:Boolean):Boolean;
	Far;
VAR
   OK:Boolean;
   erg:Extended;
   z:Integer;
   Exg:Byte;
BEGIN
   LayoutTenBytes:=False; Cnt:=10;

   IF Asc='?' THEN
    BEGIN
     IF DSFlag=DSConstant THEN WrError(1930)
     ELSE
      BEGIN
       LayoutTenBytes:=True; DSFlag:=DSSpace; Inc(CodeLen,10);
      END;
     Exit;
    END
   ELSE
    BEGIN
     IF DSFlag=DSSpace THEN
      BEGIN
       WrError(1930); Exit;
      END
     ELSE DSFlag:=DSConstant;
    END;

   IF CodeLen+10>MaxCodeLen THEN
    BEGIN
     WrError(1920); Exit;
    END;
   erg:=EvalFloatExpression(Asc,Float80,OK);
   IF OK THEN
    BEGIN
     Move(erg,BAsmCode[CodeLen],10);
     Inc(CodeLen,10);
     IF Turn THEN
      FOR z:=0 TO 4 DO
       BEGIN
	Exg:=BAsmCode[CodeLen-10+z];
	BAsmCode[CodeLen-10+z]:=BAsmCode[CodeLen-1-z];
	BAsmCode[CodeLen-1-z]:=Exg;
       END;
    END;
   LayoutTenBytes:=OK;
END;

	FUNCTION DecodeIntelPseudo(Turn:Boolean):Boolean;
VAR
   Dummy:Word;
   z:Integer;
   LayoutFunc:FUNCTION (Asc:String; VAR Cnt:Word; Turn:Boolean):Boolean;
   OK:Boolean;
   HVal8:Byte;
   NewPC:LongInt;

	FUNCTION LayoutMult(Asc:String; VAR Cnt:Word):Boolean;
VAR
   z,Depth,Fnd:Integer;
   Part:String;
   SumCnt,ECnt,SInd:Word;
   Rep:LongInt;
   OK,Hyp:Boolean;
BEGIN
   LayoutMult:=False;

   { nach DUP suchen }

   Depth:=0; Fnd:=0;
   FOR z:=1 TO Length(Asc)-2 DO
    BEGIN
     IF Asc[z]='(' THEN Inc(Depth)
     ELSE IF Asc[z]=')' THEN Dec(Depth)
     ELSE IF Depth=0 THEN
      IF ((z=1) OR (NOT (Asc[z-1] IN ValidSymChars)))
      AND (NOT (Asc[z+3] IN ValidSymChars))
      AND (NLS_StrCaseCmp(Copy(Asc,z,3),'DUP')=0) THEN Fnd:=z;
    END;


   { DUP gefunden: }

   IF Fnd<>0 THEN
    BEGIN
     { Anzahl ausrechnen }

     FirstPassUnknown:=False;
     Rep:=EvalIntExpression(Copy(Asc,1,Fnd-1),Int32,OK);
     IF FirstPassUnknown THEN
      BEGIN
       WrError(1820); Exit;
      END;
     IF NOT OK THEN Exit;

     { Nullargument vergessen, bei negativem warnen }

     IF Rep<0 THEN WrError(270);
     IF Rep<=0 THEN
      BEGIN
       LayoutMult:=True; Exit;
      END;

     { Einzelteile bilden & evaluieren }

     Delete(Asc,1,Fnd+2); KillPrefBlanks(Asc); SumCnt:=0; 
     IF (Length(Asc)>=2) AND (Asc[1]='(') AND (Asc[Length(Asc)]=')') THEN
      Asc:=Copy(Asc,2,Length(Asc)-2);
     REPEAT
      Fnd:=0; z:=1; Hyp:=False; Depth:=0;
      REPEAT
       IF Asc[z]='''' THEN Hyp:=NOT Hyp
       ELSE IF NOT Hyp THEN
        BEGIN
         IF Asc[z]='(' THEN Inc(Depth)
         ELSE IF Asc[z]=')' THEN Dec(Depth)
         ELSE IF (Depth=0) AND (Asc[z]=',') THEN Fnd:=z;
        END;
       Inc(z);
      UNTIL (z>Length(Asc)) OR (Fnd<>0);
      IF Fnd=0 THEN
       BEGIN
        Part:=Asc; Asc:='';
       END
      ELSE
       BEGIN
        Part:=Copy(Asc,1,Fnd-1); Delete(Asc,1,Fnd);
       END;
      IF NOT LayoutMult(Part,ECnt) THEN Exit; Inc(SumCnt,ECnt);
     UNTIL Asc='';

     { Ergebnis vervielfachen }

     IF DSFlag=DSConstant THEN
      BEGIN
       SInd:=CodeLen-SumCnt;
       IF CodeLen+SumCnt*(Rep-1)>MaxCodeLen THEN
        BEGIN
         WrError(1920); Exit;
        END;
       FOR z:=1 TO Rep-1 DO
        BEGIN
         IF CodeLen+SumCnt>MaxCodeLen THEN Exit;
         Move(BAsmCode[SInd],BAsmCode[CodeLen],SumCnt);
         Inc(CodeLen,SumCnt);
        END;
      END
     ELSE Inc(CodeLen,SumCnt*(Rep-1));
     Cnt:=SumCnt*Rep; LayoutMult:=True;
    END

   { kein DUP: einfacher Ausdruck }

   ELSE LayoutMult:=LayoutFunc(Asc,Cnt,Turn);
END;

BEGIN
   DecodeIntelPseudo:=TRUE;

   IF (Memo('DB')) OR (Memo('DW')) OR (Memo('DD')) OR (Memo('DQ')) OR (Memo('DT')) THEN
    BEGIN
     DSFlag:=DSNone;
     IF Memo('DB') THEN
      BEGIN
       LayoutFunc:=LayoutByte;
       IF LabPart<>'' THEN SetSymbolSize(LabPart,0);
      END
     ELSE IF Memo('DW') THEN
      BEGIN
       LayoutFunc:=LayoutWord;
       IF LabPart<>'' THEN SetSymbolSize(LabPart,1);
      END
     ELSE IF Memo('DD') THEN
      BEGIN
       LayoutFunc:=LayoutDoubleWord;
       IF LabPart<>'' THEN SetSymbolSize(LabPart,2);
      END
     ELSE IF Memo('DQ') THEN
      BEGIN
       LayoutFunc:=LayoutQuadWord;
       IF LabPart<>'' THEN SetSymbolSize(LabPart,3);
      END
     ELSE IF Memo('DT') THEN
      BEGIN
       LayoutFunc:=LayoutTenBytes;
       IF LabPart<>'' THEN SetSymbolSize(LabPart,4);
      END;
     z:=1;
     REPEAT
      OK:=LayoutMult(ArgStr[z],Dummy);
      IF NOT OK THEN CodeLen:=0;
      Inc(z);
     UNTIL (NOT OK) OR (z>ArgCnt);
     DontPrint:=DSFlag=DSSpace;
     IF (MakeUseList) AND (DontPrint) THEN
      IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
     Exit;
    END;

   IF Memo('DS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       NewPC:=EvalIntExpression(ArgStr[1],Int32,OK);
       IF FirstPassUnknown THEN WrError(1820)
       ELSE IF OK THEN
        BEGIN
         DontPrint:=True; CodeLen:=NewPC;
         IF MakeUseList THEN
          IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
        END;
      END;
     Exit;
    END;

   DecodeIntelPseudo:=FALSE;
END;

        FUNCTION CutRep(VAR Asc:String; VAR Erg:LongInt):Boolean;
VAR
   OK:Boolean;
   p:Integer;
BEGIN
   IF QuotPos(Asc,'[')<>1 THEN
    BEGIN
     Erg:=1; CutRep:=True;
    END
   ELSE
    BEGIN
     Delete(Asc,1,1); p:=QuotPos(Asc,']');
     IF p>Length(Asc) THEN
      BEGIN
       WrError(1300); CutRep:=False;
      END
     ELSE
      BEGIN
       Erg:=EvalIntExpression(Copy(Asc,1,p-1),Int32,OK);
       CutRep:=OK;
       Delete(Asc,1,p);
      END;
    END;
END;

        FUNCTION DecodeMotoPseudo(Turn:Boolean):Boolean;
VAR
   OK:Boolean;
   z,z2:Integer;
   HVal8:Byte;
   HVal16:Word;
   SVal:String;
   t:TempResult;
   Rep:LongInt;
BEGIN
   DecodeMotoPseudo:=True;

   IF (Memo('BYT')) OR (Memo('FCB')) THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; OK:=True;
       REPEAT
        KillBlanks(ArgStr[z]);
        OK:=CutRep(ArgStr[z],Rep);
        FirstPassUnknown:=False;
        EvalExpression(ArgStr[z],t);
        CASE t.Typ OF
        TempInt:
         BEGIN
          IF FirstPassUnknown THEN t.Int:=t.Int AND $ff;
          IF NOT RangeCheck(t.Int,Int8) THEN WrError(1320)
          ELSE IF CodeLen+Rep>MaxCodeLen THEN
           BEGIN
            WrError(1920); OK:=False;
           END
          ELSE
           BEGIN
            FOR z2:=0 TO Rep-1 DO
             BAsmCode[CodeLen+z2]:=t.Int;
            Inc(CodeLen,Rep);
           END;
         END;
        TempFloat:
         WrError(1135);
        TempString:
         IF (Length(t.Ascii)*Rep)+CodeLen>=MaxCodeLen THEN
          BEGIN
           WrError(1920); OK:=False;
          END
         ELSE
          BEGIN
           TranslateString(t.Ascii);
           FOR z2:=1 TO Rep DO
            BEGIN
             Move(t.Ascii[1],BAsmCode[CodeLen],Length(t.Ascii));
             Inc(CodeLen,Length(t.Ascii));
            END;
          END;
        END;
        Inc(z);
       UNTIL (z>ArgCnt) OR (NOT OK);
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF (Memo('ADR')) OR (Memo('FDB'))  THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; OK:=True;
       REPEAT
        OK:=CutRep(ArgStr[z],Rep);
        IF OK THEN
         IF CodeLen+Rep*2>MaxCodeLen THEN
          BEGIN
           WrError(1920); OK:=False;
          END
         ELSE
          BEGIN
           HVal16:=EvalIntExpression(ArgStr[z],Int16,OK);
           IF OK THEN
            BEGIN
             IF Turn THEN HVal16:=Swap(HVal16);
             FOR z2:=1 TO Rep DO
              BEGIN
               WAsmCode[CodeLen SHR 1]:=HVal16; Inc(CodeLen,2);
              END;
            END;
          END;
        Inc(z);
       UNTIL (z>ArgCnt) OR (NOT OK);
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('FCC') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; OK:=True;
       REPEAT
        OK:=CutRep(ArgStr[z],Rep);
        IF OK THEN
         BEGIN
          SVal:=EvalStringExpression(ArgStr[z],OK);
          IF OK THEN
          IF CodeLen+(Length(SVal)*Rep)>=MaxCodeLen THEN
           BEGIN
            WrError(1920); OK:=False;
           END
          ELSE
           BEGIN
            TranslateString(SVal);
            FOR z2:=1 TO Rep DO
             BEGIN
              Move(SVal[1],BAsmCode[CodeLen],Length(SVal));
              Inc(CodeLen,Length(SVal));
             END;
           END;
         END;
        Inc(z);
       UNTIL (z>ArgCnt) OR (NOT OK);
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF (Memo('DFS')) OR (Memo('RMB')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       HVal16:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF FirstPassUnknown THEN WrError(1820)
       ELSE IF (OK) THEN
        BEGIN
         DontPrint:=True; CodeLen:=HVal16;
         IF MakeUseList THEN
          IF AddChunk(SegChunks[ActPC],ProgCounter,HVal16,ActPC=SegCode) THEN WrError(90);
        END;
      END;
     Exit;
    END;

   DecodeMotoPseudo:=False;
END;


       PROCEDURE ConvertDec(F:Extended; VAR w:WordField);
VAR
   s,Man,Exp:String[30];
   h,epos:Byte;

       PROCEDURE DigIns(Ch:Char; Pos:Byte);
VAR
   wpos,bpos:Byte;
BEGIN
   wpos:=Pos SHR 2; bpos:=(Pos AND 3)*4;
   w[wpos]:=w[wpos] OR (Word(Ord(Ch)-48) SHL bpos);
END;

BEGIN
   Str(F,s); h:=Pos('E',s);
   Man:=Copy(s,1,h-1); Exp:=Copy(s,h+1,Length(s)-h);
   FillChar(w,12,0);
   IF Man[1]='-' THEN w[5]:=w[5] OR $8000; Delete(Man,1,1);
   IF Exp[1]='-' THEN w[5]:=w[5] OR $4000; Delete(Exp,1,1);
   DigIns(Man[1],16); Delete(Man,1,2);
   IF Length(Man)>16 THEN Man:=Copy(Man,1,16);
   FOR h:=1 TO Length(Man) DO DigIns(Man[h],16-h);
   IF Length(Exp)>4 THEN Exp:=Copy(Exp,Length(Exp)-3,4);
   FOR h:=Length(Exp) DOWNTO 1 DO
    BEGIN
     epos:=Length(Exp)-h;
     IF epos=3 THEN DigIns(Exp[h],19) ELSE DigIns(Exp[h],epos+20);
    END;
END;

        FUNCTION DecodeMoto16Pseudo(OpSize:ShortInt; Turn:Boolean):Boolean;
CONST
   ONOFFMoto16Count=1;
   ONOFFMoto16s:ARRAY[1..ONOFFMoto16Count] OF ONOFFRec=
             ((Name:'PADDING'; Dest:@DoPadding ; FlagName:DoPaddingName ));
VAR
   z,p:Byte;
   z2,z3:LongInt;
   WSize,Rep:LongInt;
   NewPC,HVal:LongInt;
   HVal16:Integer;
   FVal:Extended;
   t:TempResult;
   OK,ValOK:Boolean;

       PROCEDURE EnterByte(b:Byte);
BEGIN
   IF (Odd(CodeLen)) AND (ListGran<>1) THEN
    BEGIN
     BAsmCode[CodeLen]:=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]:=b;
    END
   ELSE
    BEGIN
     BAsmCode[CodeLen]:=b;
    END;
   Inc(CodeLen);
END;

BEGIN
   DecodeMoto16Pseudo:=True; IF OpSize<0 THEN OpSize:=1;

   IF CodeONOFF(@ONOFFMoto16s,ONOFFMoto16Count) THEN Exit;

   IF Memo('DC') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True; z:=1;
       REPEAT
	FirstPassUnknown:=False;
        OK:=CutRep(ArgStr[z],Rep);
	IF OK THEN
	 IF FirstPassUnknown THEN WrError(1820)
	 ELSE
	  BEGIN
	   CASE OpSize OF
	   0:BEGIN
              FirstPassUnknown:=False;
	      EvalExpression(ArgStr[z],t);
              IF (FirstPassUnknown) AND (t.Typ=TempInt) THEN t.Int:=t.Int AND $ff;
	      CASE t.Typ OF
	      TempInt   : IF NOT RangeCheck(t.Int,Int8) THEN
			   BEGIN
			    WrError(1320); OK:=False;
			   END
			  ELSE IF CodeLen+Rep>MaxCodeLen THEN
			   BEGIN
			    WrError(1920); OK:=False;
			   END
			  ELSE FOR z2:=1 TO Rep DO EnterByte(t.Int);
	      TempFloat : BEGIN
			   WrError(1135); OK:=False;
			  END;
	      TempString: IF CodeLen+Rep*Length(t.Ascii)>MaxCodeLen THEN
			   BEGIN
			    WrError(1920); Exit;
			   END
			  ELSE FOR z2:=1 TO Rep DO
			   FOR z3:=1 TO Length(t.Ascii) DO
			    EnterByte(Ord(CharTransTable[t.Ascii[z3]]));
	      ELSE OK:=False;
	      END;
	     END;
	   1:BEGIN
	      HVal16:=EvalIntExpression(ArgStr[z],Int16,OK);
	      IF OK THEN
	       IF CodeLen+(Rep*2)>MaxCodeLen THEN
		BEGIN
		 WrError(1920); OK:=False;
		END
               ELSE IF ListGran=1 THEN
                FOR z2:=1 TO Rep DO
                 BEGIN
                  BAsmCode[CodeLen  ]:=Hi(HVal16);
                  BAsmCode[CodeLen+1]:=Lo(HVal16);
                  Inc(CodeLen,2);
                 END
               ELSE
                FOR z2:=1 TO Rep DO
                 BEGIN
                  WAsmCode[CodeLen SHR 1]:=HVal16; Inc(CodeLen,2);
                 END;
	     END;
	   2:BEGIN
	      HVal:=EvalIntExpression(ArgStr[z],Int32,OK);
	      IF OK THEN
	       IF CodeLen+(Rep*4)>MaxCodeLen THEN
		BEGIN
		 WrError(1920); OK:=False;
		END
               ELSE IF ListGran=1 THEN
                FOR z2:=1 TO Rep DO
                 BEGIN
                  BAsmCode[CodeLen  ]:=(HVal SHR 24) AND $ff;
                  BAsmCode[CodeLen+1]:=(HVal SHR 16) AND $ff;
                  BAsmCode[CodeLen+2]:=(HVal SHR  8) AND $ff;
                  BAsmCode[CodeLen+3]:=(HVal       ) AND $ff;
                  Inc(CodeLen,4);
                 END
               ELSE
                FOR z2:=1 TO Rep DO
                 BEGIN
                  WAsmCode[(CodeLen SHR 1)  ]:=HVal SHR 16;
                  WAsmCode[(CodeLen SHR 1)+1]:=HVal AND $ffff;
                  Inc(CodeLen,4);
                 END;
	     END;
	   3:BEGIN
	      FVal:=EvalFloatExpression(ArgStr[z],FloatCo,OK);
	      IF OK THEN
	       IF CodeLen+(Rep*8)>MaxCodeLen THEN
		BEGIN
		 WrError(1920); OK:=False;
		END
	       ELSE
		BEGIN
		 MultiFace.ValCo:=FVal;
                 IF ListGran=1 THEN
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    BAsmCode[CodeLen  ]:=Hi(MultiFace.feld[3]);
                    BAsmCode[CodeLen+1]:=Lo(MultiFace.feld[3]);
                    BAsmCode[CodeLen+2]:=Hi(MultiFace.feld[2]);
                    BAsmCode[CodeLen+3]:=Lo(MultiFace.feld[2]);
                    BAsmCode[CodeLen+4]:=Hi(MultiFace.feld[1]);
                    BAsmCode[CodeLen+5]:=Lo(MultiFace.feld[1]);
                    BAsmCode[CodeLen+6]:=Hi(MultiFace.feld[0]);
                    BAsmCode[CodeLen+7]:=Lo(MultiFace.feld[0]);
                    Inc(CodeLen,8);
                   END
                 ELSE
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    WAsmCode[(CodeLen SHR 1)  ]:=MultiFace.feld[3];
                    WAsmCode[(CodeLen SHR 1)+1]:=MultiFace.feld[2];
                    WAsmCode[(CodeLen SHR 1)+2]:=MultiFace.feld[1];
                    WAsmCode[(CodeLen SHR 1)+3]:=MultiFace.feld[0];
                    Inc(CodeLen,8);
                   END;
		END;
	     END;
	   4:BEGIN
	      FVal:=EvalFloatExpression(ArgStr[z],Float32,OK);
	      IF OK THEN
	       IF CodeLen+(Rep*4)>MaxCodeLen THEN
		BEGIN
		 WrError(1920); OK:=False;
		END
	       ELSE
		BEGIN
		 MultiFace.Val32:=FVal;
                 IF ListGran=1 THEN
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    BAsmCode[CodeLen  ]:=Hi(MultiFace.feld[1]);
                    BAsmCode[CodeLen+1]:=Lo(MultiFace.feld[1]);
                    BAsmCode[CodeLen+2]:=Hi(MultiFace.feld[0]);
                    BAsmCode[CodeLen+3]:=Lo(MultiFace.feld[0]);
                    Inc(CodeLen,4);
                   END
                 ELSE
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    WAsmCode[(CodeLen SHR 1)  ]:=MultiFace.feld[1];
                    WAsmCode[(CodeLen SHR 1)+1]:=MultiFace.feld[0];
                    Inc(CodeLen,4);
                   END;
		END;
	     END;
	   5:BEGIN
	      FVal:=EvalFloatExpression(ArgStr[z],Float64,OK);
	      IF OK THEN
	       IF CodeLen+(Rep*8)>MaxCodeLen THEN
		BEGIN
		 WrError(1920); OK:=False;
		END
	       ELSE
		BEGIN
		 MultiFace.Val64:=FVal;
                 IF ListGran=1 THEN
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    BAsmCode[CodeLen  ]:=Hi(MultiFace.feld[3]);
                    BAsmCode[CodeLen+1]:=Lo(MultiFace.feld[3]);
                    BAsmCode[CodeLen+2]:=Hi(MultiFace.feld[2]);
                    BAsmCode[CodeLen+3]:=Lo(MultiFace.feld[2]);
                    BAsmCode[CodeLen+4]:=Hi(MultiFace.feld[1]);
                    BAsmCode[CodeLen+5]:=Lo(MultiFace.feld[1]);
                    BAsmCode[CodeLen+6]:=Hi(MultiFace.feld[0]);
                    BAsmCode[CodeLen+7]:=Lo(MultiFace.feld[0]);
                    Inc(CodeLen,8);
                   END
                 ELSE
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    WAsmCode[(CodeLen SHR 1)  ]:=MultiFace.feld[3];
                    WAsmCode[(CodeLen SHR 1)+1]:=MultiFace.feld[2];
                    WAsmCode[(CodeLen SHR 1)+2]:=MultiFace.feld[1];
                    WAsmCode[(CodeLen SHR 1)+3]:=MultiFace.feld[0];
                    Inc(CodeLen,8);
                   END;
		END;
	     END;
	   6:BEGIN
	      FVal:=EvalFloatExpression(ArgStr[z],Float80,OK);
	      IF OK THEN
	       IF CodeLen+(Rep*12)>MaxCodeLen THEN
		BEGIN
		 WrError(1920); OK:=False;
		END
	       ELSE
		BEGIN
		 MultiFace.Val80:=FVal;
                 IF ListGran=1 THEN
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    BAsmCode[CodeLen   ]:=Hi(MultiFace.feld[4]);
                    BAsmCode[CodeLen+ 1]:=Lo(MultiFace.feld[4]);
                    BAsmCode[CodeLen+ 2]:=0;
                    BAsmCode[CodeLen+ 3]:=0;
                    BAsmCode[CodeLen+ 4]:=Hi(MultiFace.feld[3]);
                    BAsmCode[CodeLen+ 5]:=Lo(MultiFace.feld[3]);
                    BAsmCode[CodeLen+ 6]:=Hi(MultiFace.feld[2]);
                    BAsmCode[CodeLen+ 7]:=Lo(MultiFace.feld[2]);
                    BAsmCode[CodeLen+ 8]:=Hi(MultiFace.feld[1]);
                    BAsmCode[CodeLen+ 9]:=Lo(MultiFace.feld[1]);
                    BAsmCode[CodeLen+10]:=Hi(MultiFace.feld[0]);
                    BAsmCode[CodeLen+11]:=Lo(MultiFace.feld[0]);
                    Inc(CodeLen,12);
                   END
                 ELSE
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    WAsmCode[(CodeLen SHR 1)  ]:=MultiFace.feld[4];
                    WAsmCode[(CodeLen SHR 1)+1]:=0;
                    WAsmCode[(CodeLen SHR 1)+2]:=MultiFace.feld[3];
                    WAsmCode[(CodeLen SHR 1)+3]:=MultiFace.feld[2];
                    WAsmCode[(CodeLen SHR 1)+4]:=MultiFace.feld[1];
                    WAsmCode[(CodeLen SHR 1)+5]:=MultiFace.feld[0];
                    Inc(CodeLen,12);
                   END;
		END;
	     END;
	   7:BEGIN
	      FVal:=EvalFloatExpression(ArgStr[z],FloatDec,OK);
	      IF OK THEN
	       IF CodeLen+(Rep*12)>MaxCodeLen THEN
		BEGIN
		 WrError(1920); OK:=False;
		END
	       ELSE
		BEGIN
		 ConvertDec(FVal,MultiFace.feld);
                 IF ListGran=1 THEN
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    BAsmCode[CodeLen   ]:=Hi(MultiFace.feld[5]);
                    BAsmCode[CodeLen+ 1]:=Lo(MultiFace.feld[5]);
                    BAsmCode[CodeLen+ 2]:=Hi(MultiFace.feld[4]);
                    BAsmCode[CodeLen+ 3]:=Lo(MultiFace.feld[4]);
                    BAsmCode[CodeLen+ 4]:=Hi(MultiFace.feld[3]);
                    BAsmCode[CodeLen+ 5]:=Lo(MultiFace.feld[3]);
                    BAsmCode[CodeLen+ 6]:=Hi(MultiFace.feld[2]);
                    BAsmCode[CodeLen+ 7]:=Lo(MultiFace.feld[2]);
                    BAsmCode[CodeLen+ 8]:=Hi(MultiFace.feld[1]);
                    BAsmCode[CodeLen+ 9]:=Lo(MultiFace.feld[1]);
                    BAsmCode[CodeLen+10]:=Hi(MultiFace.feld[0]);
                    BAsmCode[CodeLen+11]:=Lo(MultiFace.feld[0]);
                    Inc(CodeLen,12);
                   END
                 ELSE
                  FOR z2:=1 TO Rep DO
                   BEGIN
                    WAsmCode[(CodeLen SHR 1)  ]:=MultiFace.feld[5];
                    WAsmCode[(CodeLen SHR 1)+1]:=MultiFace.feld[4];
                    WAsmCode[(CodeLen SHR 1)+2]:=MultiFace.feld[3];
                    WAsmCode[(CodeLen SHR 1)+3]:=MultiFace.feld[2];
                    WAsmCode[(CodeLen SHR 1)+4]:=MultiFace.feld[1];
                    WAsmCode[(CodeLen SHR 1)+5]:=MultiFace.feld[0];
                    Inc(CodeLen,12);
                   END;
		END;
	     END;
	   END;
	  END;
	Inc(z);
       UNTIL (z>ArgCnt) OR (NOT OK);
       IF NOT OK THEN CodeLen:=0;
       IF (DoPadding) AND (Odd(CodeLen)) THEN EnterByte(0);
      END;
     Exit;
    END;

   IF Memo('DS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       HVal:=EvalIntExpression(ArgStr[1],Int32,ValOK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (ValOK) AND (NOT FirstPassUnknown) THEN
	BEGIN
	 DontPrint:=True;
	 CASE OpSize OF
	 0:BEGIN
	    WSize:=1; IF (Odd(HVal)) AND (DoPadding) THEN Inc(HVal);
	   END;
	 1:WSize:=2;
	 2,4:WSize:=4;
	 3,5:WSize:=8;
	 6,7:WSize:=12;
	 END;
	 IF HVal=0 THEN
	  BEGIN
	   NewPC:=ProgCounter+WSize-1;
	   NewPC:=NewPC-NewPC MOD WSize;
	   CodeLen:=NewPC-ProgCounter;
	   IF CodeLen=0 THEN DontPrint:=False;
	  END
	 ELSE CodeLen:=HVal*WSize;
         IF (MakeUseList) AND (DontPrint) THEN
	  IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
	END;
      END;
     Exit;
    END;

   DecodeMoto16Pseudo:=False;
END;

	PROCEDURE CodeEquate(DestSeg:ShortInt; Min,Max:LongInt);
VAR
   OK:Boolean;
   t:TempResult;
   Erg:LongInt;
BEGIN
   FirstPassUnknown:=False;
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     Erg:=EvalIntExpression(ArgStr[1],Int32,OK);
     IF (OK) AND (NOT FirstPassUnknown) THEN
      IF IsUGreater(Min,Erg) THEN WrError(1315)
      ELSE IF IsUGreater(Erg,Max) THEN WrError(1320)
      ELSE
       BEGIN
	PushLocHandle(-1);
	EnterIntSymbol(LabPart,Erg,DestSeg,False);
	PopLocHandle;
	IF MakeUseList THEN
	 IF AddChunk(SegChunks[DestSeg],Erg,1,False) THEN WrError(90);
	t.Typ:=TempInt; t.Int:=Erg; SetListLineVal(t);
       END;
    END;
END;


	PROCEDURE CodeASSUME(Def:PASSUMEDef; Cnt:Integer);
VAR
   z1,z2:Integer;
   OK:Boolean;
   HVal:LongInt;
   RegPart,ValPart:String;
BEGIN
   IF ArgCnt=0 THEN WrError(1110)
   ELSE
    BEGIN
     z1:=1; OK:=True;
     WHILE (z1<=ArgCnt) AND (OK) DO
      BEGIN
       SplitString(ArgStr[z1],RegPart,ValPart,QuotPos(ArgStr[z1],':'));
       z2:=1; NLS_UpString(RegPart);
       WHILE (z2<=Cnt) AND (Def^[z2].Name<>RegPart) DO Inc(z2);
       OK:=z2<=Cnt;
       IF NOT OK THEN WrError(1980)
       ELSE WITH Def^[z2] DO
	IF ValPart='NOTHING' THEN
	 IF NothingVal=-1 THEN WrError(1350)
	 ELSE Dest^:=NothingVal
	ELSE
	 BEGIN
	  FirstPassUnknown:=False;
	  HVal:=EvalIntExpression(ValPart,Int32,OK);
	  IF OK THEN
	   IF FirstPassUnknown THEN
	    BEGIN
	     WrError(1820); OK:=False;
	    END
	   ELSE IF HVal>Max THEN WrError(1320)
	   ELSE IF HVal<Min THEN WrError(1315)
	   ELSE Dest^:=HVal;
	 END;
       Inc(z1);
      END;
    END;
END;

	FUNCTION CodeONOFF(Def:PONOFFDef; Cnt:Integer):Boolean;
VAR
   z:Integer;
   OK:Boolean;
BEGIN
   FOR z:=1 TO Cnt DO
    WITH Def^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         NLS_UpString(ArgStr[1]);
         IF AttrPart<>'' THEN WrError(1100)
         ELSE IF (ArgStr[1]<>'ON') AND (ArgStr[1]<>'OFF') THEN WrError(1520)
         ELSE
          BEGIN
           OK:=ArgStr[1]='ON';
           SetFlag(Dest^,FlagName,OK);
          END;
        END;
       CodeOnOff:=True; Exit;
      END;
   CodeOnOff:=False;
END;

END.
