{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        Unit Code6805;

Interface

        Uses NLS,
             AsmDef,AsmPars,AsmSub,CodePseu;


Implementation

TYPE
   BaseOrder=RECORD
              Name:String[4];
              MinCPU:CPUVar;
              Code:Byte;
	     END;

   ALUOrder=RECORD
             Name:String[4];
             MinCPU:CPUVar;
             Code:Byte;
             Mask:Word;
             Size:ShortInt;
	    END;

   RMWOrder=RECORD
             Name:String[4];
             MinCPU:CPUVar;
             Code:Byte;
             Mask:Word;
	    END;

CONST
   FixedOrderCnt=52;
   RelOrderCnt=23;
   ALUOrderCnt=19;
   RMWOrderCnt=12;

   ModNone=-1;
   ModImm=0;      MModImm=1 SHL ModImm;
   ModDir=1;      MModDir=1 SHL ModDir;
   ModExt=2;      MModExt=1 SHL ModExt;
   ModIx2=3;      MModIx2=1 SHL ModIx2;
   ModIx1=4;      MModIx1=1 SHL ModIx1;
   ModIx=5;       MModIx =1 SHL ModIx ;
   ModSP2=6;      MModSP2=1 SHL ModSP2;
   ModSP1=7;      MModSP1=1 SHL ModSP1;
   ModIxP=8;      MModIxP=1 SHL ModIxP;
   MMod05=MModImm+MModDir+MModExt+MModIx2+MModIx1+MModIx;
   MMod08=MModSP2+MModSP1+MModIxP;

TYPE
   FixedOrderArray=ARRAY[1..FixedOrderCnt] OF BaseOrder;
   RelOrderArray=ARRAY[1..RelOrderCnt] OF BaseOrder;
   RMWOrderArray=ARRAY[1..RMWOrderCnt] OF RMWOrder;
   ALUOrderArray=ARRAY[1..ALUOrderCnt] OF ALUOrder;

VAR
   AdrMode,OpSize:ShortInt;
   AdrVals:ARRAY[0..1] OF Byte;
   AdrCnt:Byte;

   CPU6805,CPU6808:CPUVar;

   FixedOrders:^FixedOrderArray;
   RelOrders:^RelOrderArray;
   RMWOrders:^RMWOrderArray;
   ALUOrders:^ALUOrderArray;

{----------------------------------------------------------------------------}

        PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName:String; NMin:CPUVar; NCode:Byte);
BEGIN
   Inc(z); IF z>FixedOrderCnt THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName;
     MinCPU:=NMin;
     Code:=NCode;
    END;
END;

        PROCEDURE AddRel(NName:String; NMin:CPUVar; NCode:Byte);
BEGIN
   Inc(z); IF z>RelOrderCnt THEN Halt(255);
   WITH RelOrders^[z] DO
    BEGIN
     Name:=NName;
     MinCPU:=NMin;
     Code:=NCode;
    END;
END;

        PROCEDURE AddALU(NName:String; NMin:CPUVar; NCode:Byte; NMask:Word; NSize:ShortInt);
BEGIN
   Inc(z); IF z>ALUOrderCnt THEN Halt(255);
   WITH ALUOrders^[z] DO
    BEGIN
     Name:=NName;
     MinCPU:=NMin;
     Code:=NCode;
     Mask:=NMask;
     Size:=NSize;
    END;
END;

        PROCEDURE AddRMW(NName:String; NMin:CPUVar; NCode:Byte; NMask:Word);
BEGIN
   Inc(z); IF z>RMWOrderCnt THEN Halt(255);
   WITH RMWOrders^[z] DO
    BEGIN
     Name:=NName;
     MinCPU:=NMin;
     Code:=NCode;
     Mask:=NMask;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('RTI' ,CPU6805,$80); AddFixed('RTS' ,CPU6805,$81);
   AddFixed('SWI' ,CPU6805,$83); AddFixed('TAX' ,CPU6805,$97);
   AddFixed('CLC' ,CPU6805,$98); AddFixed('SEC' ,CPU6805,$99);
   AddFixed('CLI' ,CPU6805,$9a); AddFixed('SEI' ,CPU6805,$9b);
   AddFixed('RSP' ,CPU6805,$9c); AddFixed('NOP' ,CPU6805,$9d);
   AddFixed('TXA' ,CPU6805,$9f); AddFixed('NEGA',CPU6805,$40);
   AddFixed('NEGX',CPU6805,$50); AddFixed('COMA',CPU6805,$43);
   AddFixed('COMX',CPU6805,$53); AddFixed('LSRA',CPU6805,$44);
   AddFixed('LSRX',CPU6805,$54); AddFixed('RORA',CPU6805,$46);
   AddFixed('RORX',CPU6805,$56); AddFixed('ASRA',CPU6805,$47);
   AddFixed('ASRX',CPU6805,$57); AddFixed('ASLA',CPU6805,$48);
   AddFixed('ASLX',CPU6805,$58); AddFixed('LSLA',CPU6805,$48);
   AddFixed('LSLX',CPU6805,$58); AddFixed('ROLA',CPU6805,$49);
   AddFixed('ROLX',CPU6805,$59); AddFixed('DECA',CPU6805,$4a);
   AddFixed('DECX',CPU6805,$5a); AddFixed('INCA',CPU6805,$4c);
   AddFixed('INCX',CPU6805,$5c); AddFixed('TSTA',CPU6805,$4d);
   AddFixed('TSTX',CPU6805,$5d); AddFixed('CLRA',CPU6805,$4f);
   AddFixed('CLRX',CPU6805,$5f); AddFixed('CLRH',CPU6808,$8c);
   AddFixed('DAA' ,CPU6808,$72); AddFixed('DIV' ,CPU6808,$52);
   AddFixed('MUL' ,CPU6805,$42); AddFixed('NSA' ,CPU6808,$62);
   AddFixed('PSHA',CPU6808,$87); AddFixed('PSHH',CPU6808,$8b);
   AddFixed('PSHX',CPU6808,$89); AddFixed('PULA',CPU6808,$86);
   AddFixed('PULH',CPU6808,$8a); AddFixed('PULX',CPU6808,$88);
   AddFixed('STOP',CPU6805,$8e); AddFixed('TAP' ,CPU6808,$84);
   AddFixed('TPA' ,CPU6808,$85); AddFixed('TSX' ,CPU6808,$95);
   AddFixed('TXS' ,CPU6808,$94); AddFixed('WAIT',CPU6805,$8f);

   New(RelOrders); z:=0;
   AddRel('BRA' ,CPU6805,$20);   AddRel('BRN' ,CPU6805,$21);
   AddRel('BHI' ,CPU6805,$22);   AddRel('BLS' ,CPU6805,$23);
   AddRel('BCC' ,CPU6805,$24);   AddRel('BCS' ,CPU6805,$25);
   AddRel('BNE' ,CPU6805,$26);   AddRel('BEQ' ,CPU6805,$27);
   AddRel('BHCC',CPU6805,$28);   AddRel('BHCS',CPU6805,$29);
   AddRel('BPL' ,CPU6805,$2a);   AddRel('BMI' ,CPU6805,$2b);
   AddRel('BMC' ,CPU6805,$2c);   AddRel('BMS' ,CPU6805,$2d);
   AddRel('BIL' ,CPU6805,$2e);   AddRel('BIH' ,CPU6805,$2f);
   AddRel('BSR' ,CPU6805,$ad);   AddRel('BGE' ,CPU6808,$90);
   AddRel('BGT' ,CPU6808,$92);   AddRel('BHS' ,CPU6805,$24);
   AddRel('BLE' ,CPU6808,$93);   AddRel('BLO' ,CPU6805,$25);
   AddRel('BLT' ,CPU6808,$91);

   New(ALUOrders); z:=0;
   AddALU('SUB' ,CPU6805,$00,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('CMP' ,CPU6805,$01,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('SBC' ,CPU6805,$02,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('CPX' ,CPU6805,$03,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('AND' ,CPU6805,$04,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('BIT' ,CPU6805,$05,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('LDA' ,CPU6805,$06,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('STA' ,CPU6805,$07,        MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('EOR' ,CPU6805,$08,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('ADC' ,CPU6805,$09,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('ORA' ,CPU6805,$0a,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('ADD' ,CPU6805,$0b,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('JMP' ,CPU6805,$0c,        MModDir+MModExt+MModIx+MModIx1+MModIx2                ,-1);
   AddALU('JSR' ,CPU6805,$0d,        MModDir+MModExt+MModIx+MModIx1+MModIx2                ,-1);
   AddALU('LDX' ,CPU6805,$0e,MModImm+MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('STX' ,CPU6805,$0f,        MModDir+MModExt+MModIx+MModIx1+MModIx2+MModSP1+MModSP2,0);
   AddALU('CPHX',CPU6808,$c5,MModImm+MModDir                                               ,1);
   AddALU('LDHX',CPU6808,$a5,MModImm+MModDir                                               ,1);
   AddALU('STHX',CPU6808,$85,        MModDir                                               ,1);

   New(RMWOrders); z:=0;
   AddRMW('NEG',CPU6805,$00,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('COM',CPU6805,$03,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('LSR',CPU6805,$04,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('ROR',CPU6805,$06,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('ASR',CPU6805,$07,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('ASL',CPU6805,$08,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('LSL',CPU6805,$08,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('ROL',CPU6805,$09,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('DEC',CPU6805,$0a,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('INC',CPU6805,$0c,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('TST',CPU6805,$0d,MModDir+       MModIx+MModIx1+        MModSP1        );
   AddRMW('CLR',CPU6805,$0f,MModDir+       MModIx+MModIx1+        MModSP1        );
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(RelOrders);
   Dispose(ALUOrders);
   Dispose(RMWOrders);
END;

{----------------------------------------------------------------------------}

        PROCEDURE DecodeAdr(Start,Stop:Byte; Mask:Word);
LABEL
   Found;
VAR
   OK:Boolean;
   AdrWord,Mask08:Word;
   ZeroMode:Byte;
   s:String;
   tmode1,tmode2:ShortInt;

        FUNCTION ChkZero(VAR s:String; VAR Erg:Byte):String;
BEGIN
   IF (Length(s)>0) AND (s[1]='<') THEN
    BEGIN
     ChkZero:=Copy(s,2,Length(s)-1); Erg:=2;
    END
   ELSE IF (Length(s)>0) AND (s[1]='>') THEN
    BEGIN
     ChkZero:=Copy(s,2,Length(s)-1); Erg:=1;
    END
   ELSE
    BEGIN
     ChkZero:=s; Erg:=0;
    END;
END;

BEGIN
   AdrMode:=ModNone; AdrCnt:=0;

   Mask08:=Mask AND MMod08;
   IF MomCPU=CPU6805 THEN Mask:=Mask AND MMod05;

   IF (Stop-Start=1) THEN
    BEGIN
     IF (NLS_StrCaseCmp(ArgStr[Stop],'X')=0) THEN
      BEGIN
       tmode1:=ModIx1; tmode2:=ModIx2;
      END
     ELSE IF (NLS_StrCaseCmp(ArgStr[Stop],'SP')=0) THEN
      BEGIN
       tmode1:=ModSP1; tmode2:=ModSP2;
       IF MomCPU<CPU6808 THEN
        BEGIN
         WrXError(1445,ArgStr[Stop]); Goto Found;
        END;
      END
     ELSE
      BEGIN
       WrXError(1445,ArgStr[Stop]); Goto Found;
      END;

     s:=ChkZero(ArgStr[Start],ZeroMode);
     FirstPassUnknown:=False;
     IF ZeroMode=2 THEN
      AdrWord:=EvalIntExpression(s,Int8,OK)
     ELSE
      AdrWord:=EvalIntExpression(s,Int16,OK);

     IF OK THEN
      BEGIN
       IF (ZeroMode=0) AND (AdrWord=0) AND (Mask AND MModIx<>0) AND (ArgStr[Stop]='X') THEN AdrMode:=ModIx

       ELSE IF (Mask AND (1 SHL tmode2)=0) OR (ZeroMode=2) OR ((ZeroMode=0) AND (Hi(AdrWord)=0)) THEN
        BEGIN
         IF FirstPassUnknown THEN AdrWord:=AdrWord AND $ff;
         IF Hi(AdrWord)<>0 THEN WrError(1340)
         ELSE
          BEGIN
           AdrCnt:=1; AdrVals[0]:=Lo(AdrWord); AdrMode:=tmode1;
          END;
        END

       ELSE
        BEGIN
         AdrVals[0]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
         AdrCnt:=2; AdrMode:=tmode2;
        END;
      END;
    END

   ELSE IF (Stop=Start) THEN
    BEGIN
     { Postinkrement }

     IF NLS_StrCaseCmp(ArgStr[Start],'X+')=0 THEN
      BEGIN
       AdrMode:=ModIxP; Goto Found;
      END;

     { X-indirekt }

     IF NLS_StrCaseCmp(ArgStr[Start],'X')=0 THEN
      BEGIN
       AdrMode:=ModIx; Goto Found;
      END;

     { immediate }

     IF (ArgStr[Start]<>'') AND (ArgStr[Start][1]='#') THEN
      BEGIN
       Delete(ArgStr[Start],1,1);
       CASE OpSize OF
       -1:WrError(1132);
       0:BEGIN
          AdrVals[0]:=EvalIntExpression(ArgStr[Start],Int8,OK);
          IF OK THEN
           BEGIN
            AdrCnt:=1; AdrMode:=ModImm;
           END;
         END;
       1:BEGIN
          AdrWord:=EvalIntExpression(ArgStr[Start],Int16,OK);
          IF OK THEN
           BEGIN
            AdrVals[0]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
            AdrCnt:=2; AdrMode:=ModImm;
           END;
         END;
       END;
       Goto Found;
      END;

     { absolut }

     s:=ChkZero(ArgStr[Start],ZeroMode);
     FirstPassUnknown:=False;
     IF ZeroMode=2 THEN
      AdrWord:=EvalIntExpression(s,UInt8,OK)
     ELSE
      AdrWord:=EvalIntExpression(s,UInt16,OK);

     IF OK THEN
      BEGIN
       IF (Mask AND MModExt=0) OR (ZeroMode=2) OR ((ZeroMode=0) AND (Hi(AdrWord)=0)) THEN
        BEGIN
         IF FirstPassUnknown THEN AdrWord:=AdrWord AND $ff;
         IF Hi(AdrWord)<>0 THEN WrError(1340)
         ELSE
          BEGIN
           AdrCnt:=1; AdrVals[0]:=Lo(AdrWord); AdrMode:=ModDir;
          END;
        END
       ELSE
        BEGIN
         AdrVals[0]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
         AdrCnt:=2; AdrMode:=ModExt;
        END;
       Goto Found;
      END;
    END

   ELSE WrError(1110);


Found:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     IF (1 SHL AdrMode AND Mask08=0) THEN WrError(1350) ELSE WrError(1505);
     AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   z,z2:Integer;
   OK:Boolean;
   HVal8:ShortInt;
   HVal16:Integer;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_6805;
	Far;
VAR
   z,AdrInt:Integer;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeMotoPseudo(True) THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrXError(1500,OpPart)
       ELSE
        BEGIN
         CodeLen:=1; BAsmCode[0]:=Code;
        END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU6808 THEN WrXError(1500,OpPart)
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(1,1,MModImm+MModDir+MModIxP);
       CASE AdrMode OF
       ModImm:
        BEGIN
         BAsmCode[1]:=AdrVals[0]; DecodeAdr(2,2,MModDir);
         IF AdrMode=ModDir THEN
          BEGIN
           BAsmCode[0]:=$6e; BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
          END;
        END;
       ModDir:
        BEGIN
         BAsmCode[1]:=AdrVals[0]; DecodeAdr(2,2,MModDir+MModIxP);
         CASE AdrMode OF
         ModDir:
          BEGIN
           BAsmCode[0]:=$4e; BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
          END;
         ModIxP:
          BEGIN
           BAsmCode[0]:=$5e; CodeLen:=2;
          END;
         END;
        END;
       ModIxP:
        BEGIN
         DecodeAdr(2,2,MModDir);
         IF AdrMode=ModDir THEN
          BEGIN
           BAsmCode[0]:=$7e; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
          END;
        END;
       END;
      END;
     Exit;
    END;

   { relative Sprnge }

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrXError(1500,OpPart)
       ELSE
        BEGIN
         AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
          ELSE
           BEGIN
            CodeLen:=2; BAsmCode[0]:=Code; BAsmCode[1]:=Lo(AdrInt);
           END;
        END;
       Exit;
      END;

   IF (Memo('CBEQA')) OR (Memo('CBEQX')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU6808 THEN WrXError(1500,OpPart)
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(1,1,MModImm);
       IF AdrMode=ModImm THEN
        BEGIN
         BAsmCode[1]:=AdrVals[0];
         AdrInt:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+3);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
          ELSE
           BEGIN
            BAsmCode[0]:=$41+(Ord(Memo('CBEQX')) SHL 4);
            BAsmCode[2]:=AdrInt AND $ff;
            CodeLen:=3;
           END;
        END;
      END;
     Exit;
    END;

   IF Memo('CBEQ') THEN
    BEGIN
     IF MomCPU<CPU6808 THEN WrXError(1500,OpPart)
     ELSE IF ArgCnt=2 THEN
      BEGIN
       DecodeAdr(1,1,MModDir+MModIxP);
       CASE AdrMode OF
       ModDir:
        BEGIN
         BAsmCode[0]:=$31; BAsmCode[1]:=AdrVals[0]; z:=3;
        END;
       ModIxP:
        BEGIN
         BAsmCode[0]:=$71; z:=2;
        END;
       END;
       IF AdrMode<>ModNone THEN
        BEGIN
         AdrInt:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+z);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
          ELSE
           BEGIN
            BAsmCode[z-1]:=AdrInt AND $ff; CodeLen:=z;
           END;
        END;
      END
     ELSE IF ArgCnt=3 THEN
      BEGIN
       OK:=True;
       IF NLS_StrCaseCmp(ArgStr[2],'X+')=0 THEN z:=3
       ELSE IF NLS_StrCaseCmp(ArgStr[2],'SP')=0 THEN
        BEGIN
         BAsmCode[0]:=$9e; z:=4;
        END
       ELSE
        BEGIN
         WrXError(1445,ArgStr[2]); OK:=False;
        END;
       IF OK THEN
        BEGIN
         BAsmCode[z-3]:=$61;
         BAsmCode[z-2]:=EvalIntExpression(ArgStr[1],UInt8,OK);
         IF OK THEN
          BEGIN
           AdrInt:=EvalIntExpression(ArgStr[3],UInt16,OK)-(EProgCounter+z);
           IF OK THEN
            IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
            ELSE
             BEGIN
              BAsmCode[z-1]:=AdrInt AND $ff; CodeLen:=z;
             END;
          END;
        END;
      END
     ELSE WrError(1110);
     Exit;
    END;

   IF (Memo('DBNZA')) OR (Memo('DBNZX')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU6808 THEN WrXError(1500,OpPart)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
       IF OK THEN
        IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
        ELSE
         BEGIN
          BAsmCode[0]:=$4b+(Ord(Memo('DBNZX')) SHL 4);
          BAsmCode[1]:=AdrInt AND $ff;
          CodeLen:=2;
         END;
      END;
     Exit;
    END;

   IF Memo('DBNZ') THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE IF MomCPU<CPU6808 THEN WrXError(1500,OpPart)
     ELSE
      BEGIN
       DecodeAdr(1,ArgCnt-1,MModDir+MModIx+MModIx1+MModSP1);
       CASE AdrMode OF
       ModDir:
        BEGIN
         BAsmCode[0]:=$3b; BAsmCode[1]:=AdrVals[0]; z:=3;
        END;
       ModIx:
        BEGIN
         BAsmCode[0]:=$7b; z:=2;
        END;
       ModIx1:
        BEGIN
         BAsmCode[0]:=$6b; BAsmCode[1]:=AdrVals[0]; z:=3;
        END;
       ModSP1:
        BEGIN
         BAsmCode[0]:=$9e; BAsmCode[1]:=$6b; BAsmCode[2]:=AdrVals[0]; z:=4;
        END;
       END;
       IF AdrMode<>ModNone THEN
        BEGIN
         AdrInt:=EvalIntExpression(ArgStr[ArgCnt],UInt16,OK)-(EProgCounter+z);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
          ELSE
           BEGIN
            BAsmCode[z-1]:=AdrInt AND $ff; CodeLen:=z;
           END;
        END;
      END;
     Exit;
    END;

   { ALU-Operationen }

   FOR z:=1 TO ALUOrderCnt DO
    WITH ALUOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF MomCPU<MinCPU THEN WrXError(1500,OpPart)
       ELSE
        BEGIN
         OpSize:=Size;
         DecodeAdr(1,ArgCnt,Mask);
         IF AdrMode<>ModNone THEN
          BEGIN
           CASE AdrMode OF
           ModImm : BEGIN BAsmCode[0]:=$a0+Code; CodeLen:=1; END;
           ModDir : BEGIN BAsmCode[0]:=$b0+Code; CodeLen:=1; END;
           ModExt : BEGIN BAsmCode[0]:=$c0+Code; CodeLen:=1; END;
           ModIx  : BEGIN BAsmCode[0]:=$f0+Code; CodeLen:=1; END;
           ModIx1 : BEGIN BAsmCode[0]:=$e0+Code; CodeLen:=1; END;
           ModIx2 : BEGIN BAsmCode[0]:=$d0+Code; CodeLen:=1; END;
           ModSP1 : BEGIN BAsmCode[0]:=$9e; BAsmCode[1]:=$e0+Code; CodeLen:=2; END;
           ModSP2 : BEGIN BAsmCode[0]:=$9e; BAsmCode[1]:=$d0+Code; CodeLen:=2; END;
           END;
           Move(AdrVals,BAsmCode[CodeLen],AdrCnt); Inc(CodeLen,AdrCnt);
          END;
        END;
       Exit;
      END;

   IF (Memo('AIX')) OR (Memo('AIS')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU6808 THEN WrXError(1500,OpPart)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),SInt8,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$a7+(Ord(Memo('AIX')) SHL 3); CodeLen:=2;
        END;
      END;
     Exit;
    END;

   { Read/Modify/Write-Operationen }

   FOR z:=1 TO RMWOrderCnt DO
    WITH RMWOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF MomCPU<MinCPU THEN WrXError(1500,OpPart)
       ELSE
        BEGIN
         DecodeAdr(1,ArgCnt,Mask);
         IF AdrMode<>ModNone THEN
          BEGIN
           CASE AdrMode OF
           ModDir : BEGIN BAsmCode[0]:=$30+Code; CodeLen:=1; END;
           ModIx  : BEGIN BAsmCode[0]:=$70+Code; CodeLen:=1; END;
           ModIx1 : BEGIN BAsmCode[0]:=$60+Code; CodeLen:=1; END;
           ModSP1 : BEGIN BAsmCode[0]:=$9e; BAsmCode[1]:=$60+Code; CodeLen:=2; END;
           END;
           Move(AdrVals,BAsmCode[CodeLen],AdrCnt); Inc(CodeLen,AdrCnt);
          END;
        END;
       Exit;
      END;

   IF OpPart[Length(OpPart)] IN ['0'..'7'] THEN
    BEGIN
     FOR z:=ArgCnt DOWNTO 1 DO ArgStr[z+1]:=ArgStr[z];
     ArgStr[1]:=OpPart[Length(OpPart)]; Inc(ArgCnt);
     Dec(Byte(OpPart[0]));
    END;

   IF (Memo('BSET')) OR (Memo('BCLR')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt3,OK);
         IF OK THEN
          BEGIN
           CodeLen:=2; BAsmCode[0]:=$10+(BAsmCode[0] SHL 1)+Ord(Memo('BCLR'));
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('BRSET')) OR (Memo('BRCLR')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt3,OK);
         IF OK THEN
          BEGIN
           AdrInt:=EvalIntExpression(ArgStr[3],UInt16,OK)-(EProgCounter+3);
           IF OK THEN
            IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
            ELSE
             BEGIN
              CodeLen:=3; BAsmCode[0]:=(BAsmCode[0] SHL 1)+Ord(Memo('BRCLR'));
              BAsmCode[2]:=Lo(AdrInt);
             END;
          END;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_6805:Boolean;
	Far;
BEGIN
   IF ActPC=SegCode THEN ChkPC_6805:=(ProgCounter>=0) AND (ProgCounter<$2000)
   ELSE ChkPC_6805:=False;
END;

	FUNCTION IsDef_6805:Boolean;
	Far;
BEGIN
   IsDef_6805:=False;
END;

        PROCEDURE SwitchFrom_6805;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_6805;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$62; NOPCode:=$9d;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_6805; ChkPC:=ChkPC_6805; IsDef:=IsDef_6805;
   SwitchFrom:=SwitchFrom_6805; InitFields;
END;

BEGIN
   CPU6805:=AddCPU('6805',SwitchTo_6805);
   CPU6808:=AddCPU('68HC08',SwitchTo_6805);
END.
