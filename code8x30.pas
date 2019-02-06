{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code8x30;

INTERFACE
        Uses NLS,Chunks,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

CONST
   AriOrderCnt=4;

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	      END;

   AriOrderArray=ARRAY[0..AriOrderCnt-1] OF FixedOrder;

VAR
   CPU8x300,CPU8x305:CPUVar;
   AriOrders:^AriOrderArray;

{---------------------------------------------------------------------------}


	PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddAri(NName:String; NCode:Word);
BEGIN
   IF z>=AriOrderCnt THEN Halt(255);
   WITH AriOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(AriOrders); z:=0;
   AddAri('MOVE',0); AddAri('ADD',1); AddAri('AND',2); AddAri('XOR',3);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(AriOrders);
END;

{---------------------------------------------------------------------------}

        FUNCTION DecodeReg(Asc:String; VAR Erg:Word; VAR ErgLen:ShortInt):Boolean;
VAR
   OK:Boolean;
   Acc:Word;
   Adr:LongInt;
   z:Integer;
BEGIN
   DecodeReg:=True; ErgLen:=-1;

   IF NLS_StrCaseCmp(Asc,'AUX')=0 THEN
    BEGIN
     Erg:=0; Exit;
    END;

   IF NLS_StrCaseCmp(Asc,'OVF')=0 THEN
    BEGIN
     Erg:=8; Exit;
    END;

   IF NLS_StrCaseCmp(Asc,'IVL')=0 THEN
    BEGIN
     Erg:=7; Exit;
    END;

   IF NLS_StrCaseCmp(Asc,'IVR')=0 THEN
    BEGIN
     Erg:=15; Exit;
    END;

   IF (UpCase(Asc[1])='R') AND (Length(Asc)>1) AND (Length(Asc)<4) THEN
    BEGIN
     Acc:=0; OK:=True;
     FOR z:=2 TO Length(Asc) DO
      IF OK THEN
       IF (Asc[z]<'0') OR (Asc[z]>'7') THEN OK:=False
       ELSE Acc:=(Acc SHL 3)+(Ord(Asc[z])-AscOfs);
     IF (OK) AND (Acc<32) THEN
      BEGIN
       IF (MomCPU=CPU8x300) AND (Acc>9) AND (Acc<15) THEN
        BEGIN
         WrXError(1445,Asc); DecodeReg:=False;
        END
       ELSE Erg:=Acc;
       Exit;
      END;
    END;

   IF (Length(Asc)=4) AND (NLS_StrCaseCmp(Copy(Asc,2,2),'IV')=0) AND (Asc[4]>='0') AND (Asc[4]<='7') THEN
    IF UpCase(Asc[1])='L' THEN
     BEGIN
      Erg:=Ord(Asc[4])-AscOfs+$10; Exit;
     END
    ELSE IF UpCase(Asc[1])='R' THEN
     BEGIN
      Erg:=Ord(Asc[4])-AscOfs+$18; Exit;
     END;

   { IV - Objekte }

   Adr:=EvalIntExpression(Asc,UInt24,OK);
   IF OK THEN
    BEGIN
     ErgLen:=Adr AND 7;
     Erg:=$10+((Adr AND $10) SHR 1)+((Adr AND $700) SHR 8);
    END
   ELSE DecodeReg:=False;
END;

        FUNCTION HasDisp(VAR Asc:String):Integer;
VAR
   z,Lev:Integer;
BEGIN
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

   HasDisp:=z;
END;

        FUNCTION GetLen(VAR Asc:String; VAR Erg:Word):Boolean;
VAR
   OK:Boolean;
BEGIN
   FirstPassUnknown:=False; GetLen:=False;
   Erg:=EvalIntExpression(Asc,UInt4,OK); IF NOT OK THEN Exit;
   IF FirstPassUnknown THEN Erg:=8;
   IF NOT ChkRange(Erg,1,8) THEN Exit;
   Erg:=Erg AND 7; GetLen:=True;
END;

{---------------------------------------------------------------------------}

{ Symbol: 00AA0ORL }

	FUNCTION DecodePseudo:Boolean;
VAR
   Adr,Ofs,Erg:LongInt;
   Len:Word;
   OK:Boolean;
BEGIN
   DecodePseudo:=True;

   IF (Memo('LIV')) OR (Memo('RIV')) THEN
    BEGIN
     Erg:=$10*Ord(Memo('RIV'));
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       Adr:=EvalIntExpression(ArgStr[1],UInt8,OK);
       IF OK THEN
        BEGIN
         Ofs:=EvalIntExpression(ArgStr[2],UInt3,OK);
         IF OK THEN
          IF GetLen(ArgStr[3],Len) THEN
           BEGIN
            PushLocHandle(-1);
            EnterIntSymbol(LabPart,Erg+(Adr SHL 16)+(Ofs SHL 8)+(Len AND 7),SegNone,False);
            PopLocHandle;
           END;
        END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_8x30X;
	Far;
VAR
   OK:Boolean;
   AdrWord,SrcReg,DestReg:Word;
   SrcLen,DestLen:ShortInt;
   Op:LongInt;
   Rot,Adr:Word;
   z,p:Integer;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { eingebaute Makros }

   IF Memo('NOP') THEN   { NOP = MOVE AUX,AUX }
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=$0000; CodeLen:=1;
      END;
     Exit;
    END;

   IF Memo('HALT') THEN { HALT = JMP * }
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=$e000+(EProgCounter AND $1fff); CodeLen:=1;
      END;
     Exit;
    END;

   IF (Memo('XML')) OR (Memo('XMR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU8x305 THEN WrError(1500)
     ELSE
      BEGIN
       Adr:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
        BEGIN
         WAsmCode[0]:=$ca00+(Ord(Memo('XER')) SHL 8)+(Adr AND $ff);
         CodeLen:=1;
        END;
      END;
     Exit
    END;

   { Datentransfer }

   IF Memo('SEL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       Op:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
        BEGIN
         WAsmCode[0]:=$c700+((Op AND $10) SHL 7)+((Op SHR 16) AND $ff);
         CodeLen:=1;
        END;
      END;
     Exit;
    END;

   IF Memo('XMIT') THEN
    BEGIN
     IF (ArgCnt<>2) AND (ArgCnt<>3) THEN WrError(1110)
     ELSE IF DecodeReg(ArgStr[2],SrcReg,SrcLen) THEN
      IF SrcReg<16 THEN
       BEGIN
        IF ArgCnt<>2 THEN WrError(1110)
        ELSE
         BEGIN
          Adr:=EvalIntExpression(ArgStr[1],Int8,OK);
          IF OK THEN
           BEGIN
            WAsmCode[0]:=$c000+(SrcReg SHL 8)+(Adr AND $ff);
            CodeLen:=1;
           END;
         END;
       END
      ELSE
       BEGIN
        IF ArgCnt=2 THEN
	 BEGIN
	  Rot:=$ffff; OK:=True;
         END
        ELSE OK:=GetLen(ArgStr[3],Rot);
        IF OK THEN
         BEGIN
          IF Rot=$ffff THEN
           IF SrcLen=-1 THEN Rot:=0 ELSE Rot:=SrcLen;
          IF (SrcLen<>-1) AND (Rot<>SrcLen) THEN WrError(1131)
          ELSE
           BEGIN
            Adr:=EvalIntExpression(ArgStr[1],Int5,OK);
            IF OK THEN
             BEGIN
              WAsmCode[0]:=$c000+(SrcReg SHL 8)+(Rot SHL 5)+(Adr AND $1f);
              CodeLen:=1;
             END;
           END;
         END;
       END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=0 TO AriOrderCnt-1 DO
    WITH AriOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<>2) AND (ArgCnt<>3) THEN WrError(1110)
       ELSE IF DecodeReg(ArgStr[ArgCnt],DestReg,DestLen) THEN
        IF DestReg<16 THEN         { Ziel Register }
         BEGIN
          IF ArgCnt=2 THEN         { wenn nur zwei Operanden und Ziel Register... }
           BEGIN
            p:=HasDisp(ArgStr[1]); { kann eine Rotation dabei sein }
            IF p<>0 THEN
             BEGIN                 { jau! }
              Rot:=EvalIntExpression(Copy(ArgStr[1],p+1,Length(ArgStr[1])-p-1),UInt3,OK);
              IF OK THEN
               BEGIN
                Delete(ArgStr[1],p,Length(ArgStr[1])+1-p);
                IF DecodeReg(ArgStr[1],SrcReg,SrcLen) THEN
                 IF SrcReg>=16 THEN WrXError(1445,ArgStr[1])
                 ELSE
                  BEGIN
                   WAsmCode[0]:=(Code SHL 13)+(SrcReg SHL 8)+(Rot SHL 5)+DestReg;
                   CodeLen:=1;
                  END;
               END;
             END
            ELSE                   { noi! }
             BEGIN
              IF DecodeReg(ArgStr[1],SrcReg,SrcLen) THEN
               BEGIN
                WAsmCode[0]:=(Code SHL 13)+(SrcReg SHL 8)+DestReg;
                IF (SrcReg>=16) AND (SrcLen<>-1) THEN Inc(WAsmCode[0],SrcLen SHL 5);
                CodeLen:=1;
               END;
             END;
           END
          ELSE                     { 3 Operanden --> Quelle ist I/O }
           BEGIN
            IF GetLen(ArgStr[2],Rot) THEN
             IF DecodeReg(ArgStr[1],SrcReg,SrcLen) THEN
              IF SrcReg<16 THEN WrXError(1445,ArgStr[1])
              ELSE IF (SrcLen<>-1) AND (SrcLen<>Rot) THEN WrError(1131)
              ELSE
               BEGIN
                WAsmCode[0]:=(Code SHL 13)+(SrcReg SHL 8)+(Rot SHL 5)+DestReg;
                CodeLen:=1;
               END;
           END;
         END
        ELSE                       { Ziel I/O }
         BEGIN
          IF ArgCnt=2 THEN         { 2 Argumente: LÑnge=LÑnge Ziel }
           BEGIN
            Rot:=DestLen; OK:=True;
           END
          ELSE                     { 3 Argumente: LÑnge=LÑnge Ziel+Angabe }
           BEGIN
            OK:=GetLen(ArgStr[2],Rot);
            IF OK THEN
             BEGIN
              IF FirstPassUnknown THEN Rot:=DestLen;
              IF DestLen=-1 THEN DestLen:=Rot;
              OK:=Rot=DestLen;
              IF NOT OK THEN WrError(1131);
             END;
           END;
          IF OK THEN
           IF DecodeReg(ArgStr[1],SrcReg,SrcLen) THEN
            BEGIN
             IF (Rot=$ffff) THEN
	      IF (SrcLen=-1) THEN Rot:=0 ELSE Rot:=SrcLen;
             IF (DestReg>=16) AND (SrcLen<>-1) AND (SrcLen<>Rot) THEN WrError(1131)
             ELSE
              BEGIN
               WAsmCode[0]:=(Code SHL 13)+(SrcReg SHL 8)+(Rot SHL 5)+DestReg;
               CodeLen:=1;
              END;
            END;
         END;
       Exit;
      END;

   IF Memo('XEC') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       p:=HasDisp(ArgStr[1]);
       IF p=-1 THEN WrError(1350)
       ELSE IF DecodeReg(Copy(ArgStr[1],p+1,Length(ArgStr[1])-p-1),SrcReg,SrcLen) THEN
        BEGIN
         Delete(ArgStr[1],p,Length(ArgStr[1])-p+1);
         IF SrcReg<16 THEN
          BEGIN
           IF ArgCnt<>1 THEN WrError(1110)
           ELSE
            BEGIN
             WAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt8,OK);
             IF OK THEN
              BEGIN
               Inc(WAsmCode[0],$8000+(SrcReg SHL 8));
               CodeLen:=1;
              END;
            END;
          END
         ELSE
          BEGIN
           IF ArgCnt=1 THEN
	    BEGIN
	     Rot:=$ffff; OK:=True;
            END
           ELSE OK:=GetLen(ArgStr[2],Rot);
           IF OK THEN
            BEGIN
             IF Rot=$ffff THEN
              IF SrcLen=-1 THEN Rot:=0 ELSE Rot:=SrcLen;
             IF (SrcLen<>-1) AND (Rot<>SrcLen) THEN WrError(1131)
             ELSE
              BEGIN
               WAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt5,OK);
               IF OK THEN
                BEGIN
                 Inc(WAsmCode[0],$8000+(SrcReg SHL 8)+(Rot SHL 5));
                 CodeLen:=1;
                END;
              END;
            END;
          END;
        END;
      END;
     Exit;
    END;

   { SprÅnge }

   IF Memo('JMP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt13,OK);
       IF OK THEN
        BEGIN
         Inc(WAsmCode[0],$e000); CodeLen:=1;
        END;
      END;
     Exit;
    END;

   IF Memo('NZT') THEN
    BEGIN
     IF (ArgCnt<>2) AND (ArgCnt<>3) THEN WrError(1110)
     ELSE IF DecodeReg(ArgStr[1],SrcReg,SrcLen) THEN
      IF SrcReg<16 THEN
       BEGIN
        IF ArgCnt<>2 THEN WrError(1110)
        ELSE
         BEGIN
          Adr:=EvalIntExpression(ArgStr[2],UInt13,OK);
          IF OK THEN
           IF (NOT SymbolQuestionable) AND ((Adr SHR 8)<>(EProgCounter SHR 8)) THEN WrError(1910)
           ELSE
            BEGIN
             WAsmCode[0]:=$a000+(SrcReg SHL 8)+(Adr AND $ff);
             CodeLen:=1;
            END;
         END;
       END
      ELSE
       BEGIN
        IF ArgCnt=2 THEN
	 BEGIN
	  Rot:=$ffff; OK:=True;
         END
        ELSE OK:=GetLen(ArgStr[2],Rot);
        IF OK THEN
         BEGIN
          IF Rot=$ffff THEN
           IF SrcLen=-1 THEN Rot:=0 ELSE Rot:=SrcLen;
          IF (SrcLen<>-1) AND (Rot<>SrcLen) THEN WrError(1131)
          ELSE
           BEGIN
            Adr:=EvalIntExpression(ArgStr[ArgCnt],UInt13,OK);
            IF OK THEN
             IF (NOT SymbolQuestionable) AND ((Adr SHR 5)<>(EProgCounter SHR 5)) THEN WrError(1910)
             ELSE
              BEGIN
               WAsmCode[0]:=$a000+(SrcReg SHL 8)+(Rot SHL 5)+(Adr AND $1f);
               CodeLen:=1;
              END;
           END;
         END;
       END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_8x30X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=$1fff;
   ELSE ok:=False;
   END;
   ChkPC_8x30X:=(ok) AND (ProgCounter>=0);
END;


        FUNCTION IsDef_8x30X:Boolean;
	Far;
BEGIN
   IsDef_8x30X:=Memo('LIV') OR Memo('RIV');
END;

        PROCEDURE SwitchFrom_8x30X;
        Far;
BEGIN
   DeInitFields;
END;

        PROCEDURE SwitchTo_8x30X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$3a; NOPCode:=$0000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=2; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_8x30X; ChkPC:=ChkPC_8x30X; IsDef:=IsDef_8x30X;
   SwitchFrom:=SwitchFrom_8x30X; InitFields;
END;

BEGIN
   CPU8x300:=AddCPU('8x300',SwitchTo_8x30X);
   CPU8x305:=AddCPU('8x305',SwitchTo_8x30X);
END.

