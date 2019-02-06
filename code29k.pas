{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable}
{$ENDIF}

{ AS - Codegeneratormodul AMD 29K }

       Unit Code29K;

Interface
       Uses StringLi, NLS,
            AsmPars, AsmSub, AsmDef, CodePseu;


Implementation

TYPE
    StdOrder=RECORD
              Name:String[8];
              MustSup:Boolean;
              MinCPU:CPUVar;
              Code:Byte;
             END;

    JmpOrder=RECORD
              Name:String[7];
              HasReg,HasInd:Boolean;
              MinCPU:CPUVar;
              Code:Byte;
             END;

CONST
   StdOrderCount=51;
   NoImmOrderCount=22;
   VecOrderCount=10;
   JmpOrderCount=5;
   FixedOrderCount=2;
   MemOrderCount=7;

TYPE
   StdOrderArray=ARRAY[1..StdOrderCount] OF StdOrder;
   NoImmOrderArray=ARRAY[1..NoImmOrderCount] OF StdOrder;
   VecOrderArray=ARRAY[1..VecOrderCount] OF StdOrder;
   JmpOrderArray=ARRAY[1..JmpOrderCount] OF JmpOrder;
   FixedOrderArray=ARRAY[1..FixedOrderCount] OF StdOrder;
   MemOrderArray=ARRAY[1..MemOrderCount] OF StdOrder;

VAR
   StdOrders:^StdOrderArray;
   NoImmOrders:^NoImmOrderArray;
   VecOrders:^VecOrderArray;
   JmpOrders:^JmpOrderArray;
   FixedOrders:^FixedOrderArray;
   MemOrders:^MemOrderArray;
   CPU29000,CPU29240,CPU29243,CPU29245:CPUVar;
   Reg_RBP:LongInt;
   Emulations:StringList;
   SaveInitProc:PROCEDURE;

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddStd(NName:String; NMin:CPUVar; NSup:Boolean; NCode:Byte);
BEGIN
   Inc(z); IF z>StdOrderCount THEN Halt;
   WITH StdOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MustSup:=NSup; MinCPU:=NMin;
    END;
END;

        PROCEDURE AddNoImm(NName:String; NMin:CPUVar; NSup:Boolean; NCode:Byte);
BEGIN
   Inc(z); IF z>NoImmOrderCount THEN Halt;
   WITH NoImmOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MustSup:=NSup; MinCPU:=NMin;
    END;
END;

        PROCEDURE AddVec(NName:String; NMin:CPUVar; NSup:Boolean; NCode:Byte);
BEGIN
   Inc(z); IF z>VecOrderCount THEN Halt;
   WITH VecOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MustSup:=NSup; MinCPU:=NMin;
    END;
END;

        PROCEDURE AddJmp(NName:String; NMin:CPUVar; NHas,NInd:Boolean; NCode:Byte);
BEGIN
   Inc(z); IF z>JmpOrderCount THEN Halt;
   WITH JmpOrders^[z] DO
    BEGIN
     Name:=NName; HasReg:=NHas; HasInd:=NInd; Code:=NCode; MinCPU:=NMin;
    END;
END;

        PROCEDURE AddFixed(NName:String; NMin:CPUVar; NSup:Boolean; NCode:Byte);
BEGIN
   Inc(z); IF z>FixedOrderCount THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MustSup:=NSup; MinCPU:=NMin;
    END;
END;

        PROCEDURE AddMem(NName:String; NMin:CPUVar; NSup:Boolean; NCode:Byte);
BEGIN
   Inc(z); IF z>MemOrderCount THEN Halt;
   WITH MemOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MustSup:=NSup; MinCPU:=NMin;
    END;
END;

BEGIN
   New(StdOrders); z:=0;
   AddStd('ADD'    ,CPU29245,False,$14); AddStd('ADDC'   ,CPU29245,False,$1c);
   AddStd('ADDCS'  ,CPU29245,False,$18); AddStd('ADDCU'  ,CPU29245,False,$1a);
   AddStd('ADDS'   ,CPU29245,False,$10); AddStd('ADDU'   ,CPU29245,False,$12);
   AddStd('AND'    ,CPU29245,False,$90); AddStd('ANDN'   ,CPU29245,False,$9c);
   AddStd('CPBYTE' ,CPU29245,False,$2e); AddStd('CPEQ'   ,CPU29245,False,$60);
   AddStd('CPGE'   ,CPU29245,False,$4c); AddStd('CPGEU'  ,CPU29245,False,$4e);
   AddStd('CPGT'   ,CPU29245,False,$48); AddStd('CPGTU'  ,CPU29245,False,$4a);
   AddStd('CPLE'   ,CPU29245,False,$44); AddStd('CPLEU'  ,CPU29245,False,$46);
   AddStd('CPLT'   ,CPU29245,False,$40); AddStd('CPLTU'  ,CPU29245,False,$42);
   AddStd('CPNEQ'  ,CPU29245,False,$62); AddStd('DIV'    ,CPU29245,False,$6a);
   AddStd('DIV0'   ,CPU29245,False,$68); AddStd('DIVL'   ,CPU29245,False,$6c);
   AddStd('DIVREM' ,CPU29245,False,$6e); AddStd('EXBYTE' ,CPU29245,False,$0a);
   AddStd('EXHW'   ,CPU29245,False,$7c); AddStd('EXTRACT',CPU29245,False,$7a);
   AddStd('INBYTE' ,CPU29245,False,$0c); AddStd('INHW'   ,CPU29245,False,$78);
   AddStd('MUL'    ,CPU29245,False,$64); AddStd('MULL'   ,CPU29245,False,$66);
   AddStd('MULU'   ,CPU29245,False,$74); AddStd('NAND'   ,CPU29245,False,$9a);
   AddStd('NOR'    ,CPU29245,False,$98); AddStd('OR'     ,CPU29245,False,$92);
   AddStd('SLL'    ,CPU29245,False,$80); AddStd('SRA'    ,CPU29245,False,$86);
   AddStd('SRL'    ,CPU29245,False,$82); AddStd('SUB'    ,CPU29245,False,$24);
   AddStd('SUBC'   ,CPU29245,False,$2c); AddStd('SUBCS'  ,CPU29245,False,$28);
   AddStd('SUBCU'  ,CPU29245,False,$2a); AddStd('SUBR'   ,CPU29245,False,$34);
   AddStd('SUBRC'  ,CPU29245,False,$3c); AddStd('SUBRCS' ,CPU29245,False,$38);
   AddStd('SUBRCU' ,CPU29245,False,$3a); AddStd('SUBRS'  ,CPU29245,False,$30);
   AddStd('SUBRU'  ,CPU29245,False,$32); AddStd('SUBS'   ,CPU29245,False,$20);
   AddStd('SUBU'   ,CPU29245,False,$22); AddStd('XNOR'   ,CPU29245,False,$96);
   AddStd('XOR'    ,CPU29245,False,$94);

   New(NoImmOrders); z:=0;
   AddNoImm('DADD'    ,CPU29000,False,$f1); AddNoImm('DDIV'    ,CPU29000,False,$f7);
   AddNoImm('DEQ'     ,CPU29000,False,$eb); AddNoImm('DGE'     ,CPU29000,False,$ef);
   AddNoImm('DGT'     ,CPU29000,False,$ed); AddNoImm('DIVIDE'  ,CPU29000,False,$e1);
   AddNoImm('DIVIDU'  ,CPU29000,False,$e3); AddNoImm('DMUL'    ,CPU29000,False,$f5);
   AddNoImm('DSUB'    ,CPU29000,False,$f3); AddNoImm('FADD'    ,CPU29000,False,$f0);
   AddNoImm('FDIV'    ,CPU29000,False,$f6); AddNoImm('FDMUL'   ,CPU29000,False,$f9);
   AddNoImm('FEQ'     ,CPU29000,False,$ea); AddNoImm('FGE'     ,CPU29000,False,$ee);
   AddNoImm('FGT'     ,CPU29000,False,$ec); AddNoImm('FMUL'    ,CPU29000,False,$f4);
   AddNoImm('FSUB'    ,CPU29000,False,$f2); AddNoImm('MULTIPLU',CPU29243,False,$e2);
   AddNoImm('MULTIPLY',CPU29243,False,$e0); AddNoImm('MULTM'   ,CPU29243,False,$de);
   AddNoImm('MULTMU'  ,CPU29243,False,$df); AddNoImm('SETIP'   ,CPU29245,False,$9e);

   New(VecOrders); z:=0;
   AddVec('ASEQ'   ,CPU29245,False,$70); AddVec('ASGE'   ,CPU29245,False,$5c);
   AddVec('ASGEU'  ,CPU29245,False,$5e); AddVec('ASGT'   ,CPU29245,False,$58);
   AddVec('ASGTU'  ,CPU29245,False,$5a); AddVec('ASLE'   ,CPU29245,False,$54);
   AddVec('ASLEU'  ,CPU29245,False,$56); AddVec('ASLT'   ,CPU29245,False,$50);
   AddVec('ASLTU'  ,CPU29245,False,$52); AddVec('ASNEQ'  ,CPU29245,False,$72);

   New(JmpOrders); z:=0;
   AddJmp('CALL'   ,CPU29245,True ,True ,$a8); AddJmp('JMP'    ,CPU29245,False,True ,$a0);
   AddJmp('JMPF'   ,CPU29245,True ,True ,$a4); AddJmp('JMPFDEC',CPU29245,True ,False,$b4);
   AddJmp('JMPT'   ,CPU29245,True ,True ,$ac);

   New(FixedOrders); z:=0;
   AddFixed('HALT'   ,CPU29245,True,$89); AddFixed('IRET'   ,CPU29245,True,$88);

   New(MemOrders); z:=0;
   AddMem('LOAD'   ,CPU29245,False,$16); AddMem('LOADL'  ,CPU29245,False,$06);
   AddMem('LOADM'  ,CPU29245,False,$36); AddMem('LOADSET',CPU29245,False,$26);
   AddMem('STORE'  ,CPU29245,False,$1e); AddMem('STOREL' ,CPU29245,False,$0e);
   AddMem('STOREM' ,CPU29245,False,$3e);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(StdOrders);
   Dispose(NoImmOrders);
   Dispose(VecOrders);
   Dispose(JmpOrders);
   Dispose(FixedOrders);
   Dispose(MemOrders);
END;

{---------------------------------------------------------------------------}

	PROCEDURE ChkSup;
BEGIN
   IF NOT SupAllowed THEN WrError(50);
END;

	FUNCTION IsSup(RegNo:LongInt):Boolean;
BEGIN
   IsSup:=(RegNo<$80) OR (RegNo>=$a0);
END;

	FUNCTION ChkCPU(Min:CPUVar):Boolean;
BEGIN
   IF MomCPU>=Min THEN ChkCPU:=True
   ELSE ChkCPU:=StringListPresent(Emulations,OpPart);
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodeReg(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   io:ValErgType;
   OK:Boolean;
BEGIN
   IF (Length(Asc)>=2) AND (UpCase(Asc[1])='R') THEN
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,io);
     OK:=(io=0) AND (Erg>=0) AND (Erg<=255);
    END
   ELSE IF (Length(Asc)>=3) AND (UpCase(Asc[1])='G') AND (UpCase(Asc[2])='R') THEN
    BEGIN
     Val(Copy(Asc,3,Length(Asc)-2),Erg,io);
     OK:=(io=0) AND (Erg>=0) AND (Erg<=127);
    END
   ELSE IF (Length(Asc)>=3) AND (UpCase(Asc[1])='L') AND (UpCase(Asc[2])='R') THEN
    BEGIN
     Val(Copy(Asc,3,Length(Asc)-2),Erg,io);
     OK:=(io=0) AND (Erg>=0) AND (Erg<=127);
     Inc(Erg,128);
    END
   ELSE OK:=False;
   IF OK THEN
    IF (Erg<127) AND (Odd(Reg_RBP SHR (Erg SHR 4))) THEN ChkSup;
   DecodeReg:=OK;
END;

	FUNCTION DecodeSpReg(Asc:String; VAR Erg:LongInt):Boolean;
BEGIN
   DecodeSPReg:=True; NLS_UPString(Asc);
   IF Asc='VAB' THEN Erg:=0
   ELSE IF Asc='OPS'  THEN Erg:=1
   ELSE IF Asc='CPS'  THEN Erg:=2
   ELSE IF Asc='CFG'  THEN Erg:=3
   ELSE IF Asc='CHA'  THEN Erg:=4
   ELSE IF Asc='CHD'  THEN Erg:=5
   ELSE IF Asc='CHC'  THEN Erg:=6
   ELSE IF Asc='RBP'  THEN Erg:=7
   ELSE IF Asc='TMC'  THEN Erg:=8
   ELSE IF Asc='TMR'  THEN Erg:=9
   ELSE IF Asc='PC0'  THEN Erg:=10
   ELSE IF Asc='PC1'  THEN Erg:=11
   ELSE IF Asc='PC2'  THEN Erg:=12
   ELSE IF Asc='MMU'  THEN Erg:=13
   ELSE IF Asc='LRU'  THEN Erg:=14
   ELSE IF Asc='CIR'  THEN Erg:=29
   ELSE IF Asc='CDR'  THEN Erg:=30
   ELSE IF Asc='IPC'  THEN Erg:=128
   ELSE IF Asc='IPA'  THEN Erg:=129
   ELSE IF Asc='IPB'  THEN Erg:=130
   ELSE IF Asc='Q'    THEN Erg:=131
   ELSE IF Asc='ALU'  THEN Erg:=132
   ELSE IF Asc='BP'   THEN Erg:=133
   ELSE IF Asc='FC'   THEN Erg:=134
   ELSE IF Asc='CR'   THEN Erg:=135
   ELSE IF Asc='FPE'  THEN Erg:=160
   ELSE IF Asc='INTE' THEN Erg:=161
   ELSE IF Asc='FPS'  THEN Erg:=162
   ELSE DecodeSpReg:=False;
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF29KCount=1;
   ONOFF29Ks:ARRAY[1..ONOFF29KCount] OF ONOFFRec=
             ((Name:'SUPMODE'; Dest:@SupAllowed; FlagName:SupAllowedName));
   ASSUME29KCount=1;
   ASSUME29Ks:ARRAY[1..ASSUME29KCount] OF ASSUMERec=
              ((Name:'RBP'; Dest:@Reg_RBP; Min:0; Max:$ff; NothingVal:$00000000));
VAR
   OK:Boolean;
   z:Integer;
BEGIN
   DecodePseudo:=True;

   IF CodeONOFF(@ONOFF29Ks,ONOFF29KCount) THEN Exit;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME29Ks,ASSUME29KCount);
     Exit;
    END;

   IF Memo('EMULATED') THEN
    BEGIN
     IF ArgCnt<1 THEN WrError(1110)
     ELSE
      FOR z:=1 TO ArgCnt DO
       BEGIN
        NLS_UpString(ArgStr[z]);
        IF NOT StringListPresent(Emulations,ArgStr[z]) THEN
         AddStringListLast(Emulations,ArgStr[z]);
       END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_29K;
	Far;
LABEL
   IsReg;
VAR
   z,Imm:Integer;
   Dest,Src1,Src2,Src3,AdrLong:LongInt;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { Nullanweisung }

   IF Memo('') AND (AttrPart='') AND (ArgCnt=0) THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(True) THEN Exit;

   { Variante 1: Register <-- Register op Register/uimm8 }

   FOR z:=1 TO StdOrderCount DO
    WITH StdOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt>3) OR (ArgCnt<2) THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
       ELSE
        BEGIN
         OK:=True;
         IF ArgCnt=2 THEN Src1:=Dest
         ELSE OK:=DecodeReg(ArgStr[2],Src1);
         IF NOT OK THEN WrError(1445)
         ELSE
          BEGIN
           IF DecodeReg(ArgStr[ArgCnt],Src2) THEN
            BEGIN
             OK:=True; Src3:=0;
	    END
	   ELSE
	    BEGIN
	     Src2:=EvalIntExpression(ArgStr[ArgCnt],UInt8,OK);
             Src3:=$1000000;
            END;
           IF OK THEN
            BEGIN
             CodeLen:=4;
	     DAsmCode[0]:=(LongInt(Code) SHL 24)+Src3+(Dest SHL 16)+(Src1 SHL 8)+Src2;
             IF MustSup THEN ChkSup;
            END;
          END;
        END;
       Exit;
      END;

   { Variante 2: Register <-- Register op Register }

   FOR z:=1 TO NoImmOrderCount DO
    WITH NoImmOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt>3) OR (ArgCnt<2) THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
       ELSE
        BEGIN
         OK:=True;
         IF ArgCnt=2 THEN Src1:=Dest
         ELSE OK:=DecodeReg(ArgStr[2],Src1);
         IF NOT OK THEN WrError(1445)
         ELSE IF NOT DecodeReg(ArgStr[ArgCnt],Src2) THEN WrError(1445)
         ELSE
          BEGIN
           CodeLen:=4;
	   DAsmCode[0]:=(LongInt(Code) SHL 24)+(Dest SHL 16)+(Src1 SHL 8)+Src2;
           IF MustSup THEN ChkSup;
          END;
        END;
       Exit;
      END;

   { Variante 3: Vektor <-- Register op Register/uimm8 }

   FOR z:=1 TO VecOrderCount DO
    WITH VecOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE
        BEGIN
         FirstPassUnknown:=False;
	 Dest:=EvalIntExpression(ArgStr[1],UInt8,OK);
         IF FirstPassUnknown THEN Dest:=64;
         IF OK THEN
          IF NOT DecodeReg(ArgStr[2],Src1) THEN WrError(1445)
          ELSE
           BEGIN
            IF DecodeReg(ArgStr[ArgCnt],Src2) THEN
             BEGIN
              OK:=True; Src3:=0;
	     END
	    ELSE
	     BEGIN
	      Src2:=EvalIntExpression(ArgStr[ArgCnt],UInt8,OK);
              Src3:=$1000000;
             END;
            IF OK THEN
             BEGIN
              CodeLen:=4;
	      DAsmCode[0]:=(LongInt(Code) SHL 24)+Src3+(Dest SHL 16)+(Src1 SHL 8)+Src2;
              IF (MustSup) OR (Dest<=63) THEN ChkSup;
             END;
           END;
        END;
       Exit;
      END;

   { Variante 4: ohne Operanden }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
        BEGIN
         CodeLen:=4; DAsmCode[0]:=LongInt(Code) SHL 24;
         IF MustSup THEN ChkSup;
        END;
       Exit;
      END;

   { Variante 5 : [0], Speichersteuerwort, Register, Register/uimm8 }

   FOR z:=1 TO MemOrderCount DO
    WITH MemOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<>3) AND (ArgCnt<>4) THEN WrError(1110)
       ELSE
        BEGIN
         IF ArgCnt=3 THEN OK:=True
         ELSE
          BEGIN
           AdrLong:=EvalIntExpression(ArgStr[1],Int32,OK);
           IF OK THEN OK:=ChkRange(AdrLong,0,0);
          END;
         IF OK THEN
          BEGIN
           Dest:=EvalIntExpression(ArgStr[ArgCnt-2],UInt7,OK);
           IF OK THEN
            IF DecodeReg(ArgStr[ArgCnt-1],Src1) THEN
             BEGIN
              IF DecodeReg(ArgStr[ArgCnt],Src2) THEN
               BEGIN
                OK:=True; Src3:=0;
	       END
	      ELSE
	       BEGIN
	        Src2:=EvalIntExpression(ArgStr[ArgCnt],UInt8,OK);
                Src3:=$1000000;
               END;
              IF OK THEN
               BEGIN
                CodeLen:=4;
	        DAsmCode[0]:=(LongInt(Code) SHL 24)+Src3+(Dest SHL 16)+(Src1 SHL 8)+Src2;
                IF MustSup THEN ChkSup;
               END;
             END;
          END;
        END;
       Exit;
      END;

   { Sprungbefehle }

   FOR z:=1 TO JmpOrderCount DO
    WITH JmpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1+Ord(HasReg) THEN WrError(1110)
       ELSE IF DecodeReg(ArgStr[ArgCnt],Src1) THEN Goto IsReg
       ELSE
        BEGIN
         IF NOT HasReg THEN
          BEGIN
           Dest:=0; OK:=True;
          END
         ELSE OK:=DecodeReg(ArgStr[1],Dest);
         IF OK THEN
          BEGIN
           AdrLong:=EvalIntExpression(ArgStr[ArgCnt],Int32,OK);
           IF OK THEN
            IF AdrLong AND 3<>0 THEN WrError(1325)
	    ELSE IF (AdrLong-EProgCounter<=$1ffff) AND (AdrLong-EProgCounter>=-$20000) THEN
	     BEGIN
              CodeLen:=4;
              Dec(AdrLong,EProgCounter);
              DAsmCode[0]:=(LongInt(Code) SHL 24)
	                  +((AdrLong AND $3fc00) SHL 6)
                          +(Dest SHL 8)+((AdrLong AND $3fc) SHR 2);
	     END
            ELSE IF (AdrLong<0) OR (AdrLong>$3fffff) THEN WrError(1370)
            ELSE
             BEGIN
              CodeLen:=4;
              DAsmCode[0]:=(LongInt(Code+1) SHL 24)
	                  +((AdrLong AND $3fc00) SHL 6)
                          +(Dest SHL 8)+((AdrLong AND $3fc) SHR 2);
             END;
          END;
        END;
       Exit;
      END
     ELSE IF (HasInd) AND (Memo(Name+'I')) THEN
      BEGIN
       IF ArgCnt<>1+Ord(HasReg) THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[ArgCnt],Src1) THEN WrError(1445)
       ELSE
        BEGIN
IsReg:   IF NOT HasReg THEN
          BEGIN
           Dest:=0; OK:=True;
          END
         ELSE OK:=DecodeReg(ArgStr[1],Dest);
         IF OK THEN
          BEGIN
           CodeLen:=4;
           DAsmCode[0]:=(LongInt(Code+$20) SHL 24)+(Dest SHL 8)+Src1;
          END;
        END;
       Exit;
      END;

   { Sonderf„lle }

   IF Memo('CLASS') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NOT ChkCPU(CPU29000) THEN WrError(1500)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE IF NOT DecodeReg(ArgStr[2],Src1) THEN WrError(1445)
     ELSE
      BEGIN
       Src2:=EvalIntExpression(ArgStr[3],UInt2,OK);
       IF OK THEN
        BEGIN
         CodeLen:=4;
         DAsmCode[0]:=$e6000000+(Dest SHL 16)+(Src1 SHL 8)+Src2;
        END;
      END;
     Exit;
    END;

   IF Memo('EMULATE') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Dest:=EvalIntExpression(ArgStr[1],UInt8,OK);
       IF FirstPassUnknown THEN Dest:=64;
       IF OK THEN
        IF NOT DecodeReg(ArgStr[2],Src1) THEN WrError(1445)
        ELSE IF NOT DecodeReg(ArgStr[ArgCnt],Src2) THEN WrError(1445)
        ELSE
         BEGIN
          CodeLen:=4;
	  DAsmCode[0]:=$d7000000+(Dest SHL 16)+(Src1 SHL 8)+Src2;
          IF Dest<=63 THEN ChkSup;
         END;
      END;
     Exit;
    END;

   IF Memo('SQRT') THEN
    BEGIN
     IF (ArgCnt<>3) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE IF NOT ChkCPU(CPU29000) THEN WrError(1500)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE
      BEGIN
       IF ArgCnt=2 THEN
        BEGIN
         OK:=True; Src1:=Dest;
        END
       ELSE OK:=DecodeReg(ArgStr[2],Src1);
       IF NOT OK THEN WrError(1445)
       ELSE
        BEGIN
         Src2:=EvalIntExpression(ArgStr[ArgCnt],UInt2,OK);
         IF OK THEN
          BEGIN
           CodeLen:=4;
           DAsmCode[0]:=$e5000000+(Dest SHL 16)+(Src1 SHL 8)+Src2;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('CLZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE
      BEGIN
       IF DecodeReg(ArgStr[2],Src1) THEN
        BEGIN
         OK:=True; Src3:=0;
	END
       ELSE
	BEGIN
	 Src1:=EvalIntExpression(ArgStr[2],UInt8,OK);
         Src3:=$1000000;
        END;
       IF OK THEN
        BEGIN
         CodeLen:=4;
	 DAsmCode[0]:=$08000000+Src3+(Dest SHL 16)+Src1;
        END;
      END;
     Exit;
    END;

   IF Memo('CONST') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[2],Int32,OK);
       IF OK THEN
        BEGIN
         CodeLen:=4;
	 DAsmCode[0]:=((AdrLong AND $ff00) SHL 8)+(Dest SHL 8)+(AdrLong AND $ff);
         AdrLong:=AdrLong SHR 16;
         IF AdrLong=$ffff THEN Inc(DAsmCode[0],$01000000)
         ELSE
          BEGIN
           Inc(DAsmCode[0],$03000000);
           IF AdrLong<>0 THEN
            BEGIN
             CodeLen:=8;
             DAsmCode[1]:=$02000000+((AdrLong AND $ff00) SHL 16)+(Dest SHL 8)+(AdrLong AND $ff);
            END;
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('CONSTH')) OR (Memo('CONSTN')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrLong:=EvalIntExpression(ArgStr[2],Int32,OK);
       IF FirstPassUnknown THEN AdrLong:=AdrLong AND $ffff;
       IF (Memo('CONSTN')) AND (AdrLong SHR 16=$ffff) THEN AdrLong:=AdrLong AND $ffff;
       IF ChkRange(AdrLong,0,$ffff) THEN
        BEGIN
         CodeLen:=4;
         DAsmCode[0]:=$1000000+((AdrLong AND $ff00) SHL 8)+(Dest SHL 8)+(AdrLong AND $ff);
         IF Memo('CONSTH') THEN Inc(DAsmCode[0],$1000000);
	END;
      END;
     Exit;
    END;

   IF Memo('CONVERT') THEN
    BEGIN
     IF ArgCnt<>6 THEN WrError(1110)
     ELSE IF NOT ChkCPU(CPU29000) THEN WrError(1500)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE IF NOT DecodeReg(ArgStr[2],Src1) THEN WrError(1445)
     ELSE
      BEGIN
       Src2:=0;
       Inc(Src2,EvalIntExpression(ArgStr[3],UInt1,OK) SHL 7);
       IF OK THEN
        BEGIN
         Inc(Src2,EvalIntExpression(ArgStr[4],UInt3,OK) SHL 4);
         IF OK THEN
          BEGIN
           Inc(Src2,EvalIntExpression(ArgStr[5],UInt2,OK) SHL 2);
           IF OK THEN
            BEGIN
             Inc(Src2,EvalIntExpression(ArgStr[6],UInt2,OK));
             IF OK THEN
              BEGIN
               CodeLen:=4;
               DAsmCode[0]:=$e4000000+(Dest SHL 16)+(Src1 SHL 8)+Src2;
              END;
            END;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('EXHWS') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE IF NOT DecodeReg(ArgStr[2],Src1) THEN WrError(1445)
     ELSE
      BEGIN
       CodeLen:=4;
       DAsmCode[0]:=$7e000000+(Dest SHL 16)+(Src1 SHL 8);
      END;
     Exit;
    END;

   IF (Memo('INV')) OR (Memo('IRETINV')) THEN
    BEGIN
     IF ArgCnt>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=0 THEN
        BEGIN
         Src1:=0; OK:=True;
        END
       ELSE Src1:=EvalIntExpression(ArgStr[1],UInt2,OK);
       IF OK THEN
        BEGIN
         CodeLen:=4;
         DAsmCode[0]:=Src1 SHL 16;
         IF Memo('INV') THEN Inc(DAsmCode[0],$9f000000)
         ELSE Inc(DAsmCode[0],$8c000000);
         ChkSup;
        END;
      END;
     Exit;
    END;

   IF Memo('MFSR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE IF NOT DecodeSpReg(ArgStr[2],Src1) THEN WrError(1440)
     ELSE
      BEGIN
       DAsmCode[0]:=$c6000000+(Dest SHL 16)+(Src1 SHL 8);
       CodeLen:=4; IF IsSup(Src1) THEN ChkSup;
      END;
     Exit;
    END;

   IF Memo('MTSR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeSpReg(ArgStr[1],Dest) THEN WrError(1440)
     ELSE IF NOT DecodeReg(ArgStr[2],Src1) THEN WrError(1445)
     ELSE
      BEGIN
       DAsmCode[0]:=$ce000000+(Dest SHL 8)+Src1;
       CodeLen:=4; IF IsSup(Dest) THEN ChkSup;
      END;
     Exit;
    END;

   IF Memo('MTSRIM') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeSpReg(ArgStr[1],Dest) THEN WrError(1440)
     ELSE
      BEGIN
       Src1:=EvalIntExpression(ArgStr[2],UInt16,OK);
       IF OK THEN
        BEGIN
         DAsmCode[0]:=$04000000+((Src1 AND $ff00) SHL 8)+(Dest SHL 8)+Lo(Src1);
         CodeLen:=4; IF IsSup(Dest) THEN ChkSup;
        END;
      END;
     Exit;
    END;

   IF Memo('MFTLB') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE IF NOT DecodeReg(ArgStr[2],Src1) THEN WrError(1445)
     ELSE
      BEGIN
       DAsmCode[0]:=$b6000000+(Dest SHL 16)+(Src1 SHL 8);
       CodeLen:=4; ChkSup;
      END;
     Exit;
    END;

   IF Memo('MTTLB') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Dest) THEN WrError(1445)
     ELSE IF NOT DecodeReg(ArgStr[2],Src1) THEN WrError(1445)
     ELSE
      BEGIN
       DAsmCode[0]:=$be000000+(Dest SHL 8)+Src1;
       CodeLen:=4; ChkSup;
      END;
     Exit;
    END;

   { unbekannter Befehl }

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_29K;
        Far;
BEGIN
   SaveInitProc;
   Reg_RBP:=0; ClearStringList(Emulations);
END;

        FUNCTION ChkPC_29K:Boolean;
	Far;
BEGIN
   ChkPC_29K:=ActPC=SegCode;
END;

        FUNCTION IsDef_29K:Boolean;
	Far;
BEGIN
   IsDef_29K:=False;
END;

        PROCEDURE SwitchFrom_29K;
	Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_29K;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeC; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$29; NOPCode:=$000000000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=4; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_29K; ChkPC:=ChkPC_29K; IsDef:=IsDef_29K;

   SwitchFrom:=SwitchFrom_29K; InitFields;
END;

BEGIN
   CPU29245:=AddCPU('AM29245',SwitchTo_29k);
   CPU29243:=AddCPU('AM29243',SwitchTo_29k);
   CPU29240:=AddCPU('AM29240',SwitchTo_29K);
   CPU29000:=AddCPU('AM29000',SwitchTo_29K);

   Emulations:=Nil;

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_29K;
END.

