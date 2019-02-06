{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

       Unit Code601;

Interface
       Uses AsmPars, AsmSub, AsmDef, CodePseu;


Implementation

TYPE
   BaseOrder=RECORD
	      Name:String[8];
	      Code:LongInt;
	      CPUMask:Byte;
	     END;

CONST
   FixedOrderCount     = 6;
   Reg1OrderCount      = 4;
   FReg1OrderCount     = 2;
   CReg1OrderCount     = 1;
   CBit1OrderCount     = 4;
   Reg2OrderCount      = 29;
   CReg2OrderCount     = 2;
   FReg2OrderCount     = 14;
   Reg2BOrderCount     = 2;
   Reg2SwapOrderCount  = 6;
   NoDestOrderCount    = 10;
   Reg3OrderCount      = 89;
   CReg3OrderCount     = 8;
   FReg3OrderCount     = 10;
   Reg3SwapOrderCount  = 49;
   MixedOrderCount     = 8;
   FReg4OrderCount     = 16;
   RegDispOrderCount   = 16;
   FRegDispOrderCount  = 8;
   Reg2ImmOrderCount   = 12;
   Imm16OrderCount     = 7;
   Imm16SwapOrderCount = 6;

TYPE
   FixedOrderArray     = ARRAY[1..FixedOrderCount    ] OF BaseOrder;
   Reg1OrderArray      = ARRAY[1..Reg1OrderCount     ] OF BaseOrder;
   CReg1OrderArray     = ARRAY[1..CReg1OrderCount    ] OF BaseOrder;
   CBit1OrderArray     = ARRAY[1..CBit1OrderCount    ] OF BaseOrder;
   FReg1OrderArray     = ARRAY[1..FReg1OrderCount    ] OF BaseOrder;
   Reg2OrderArray      = ARRAY[1..Reg2OrderCount     ] OF BaseOrder;
   CReg2OrderArray     = ARRAY[1..CReg2OrderCount    ] OF BaseOrder;
   FReg2OrderArray     = ARRAY[1..FReg2OrderCount    ] OF BaseOrder;
   Reg2BOrderArray     = ARRAY[1..Reg2BOrderCount    ] OF BaseOrder;
   Reg2SwapOrderArray  = ARRAY[1..Reg2SwapOrderCount ] OF BaseOrder;
   NoDestOrderArray    = ARRAY[1..NoDestOrderCount   ] OF BaseOrder;
   Reg3OrderArray      = ARRAY[1..Reg3OrderCount     ] OF BaseOrder;
   CReg3OrderArray     = ARRAY[1..CReg3OrderCount    ] OF BaseOrder;
   FReg3OrderArray     = ARRAY[1..FReg3OrderCount    ] OF BaseOrder;
   Reg3SwapOrderArray  = ARRAY[1..Reg3SwapOrderCount ] OF BaseOrder;
   MixedOrderArray     = ARRAY[1..MixedOrderCount    ] OF BaseOrder;
   FReg4OrderArray     = ARRAY[1..FReg4OrderCount    ] OF BaseOrder;
   RegDispOrderArray   = ARRAY[1..RegDispOrderCount  ] OF BaseOrder;
   FRegDispOrderArray  = ARRAY[1..FRegDispOrderCount ] OF BaseOrder;
   Reg2ImmOrderArray   = ARRAY[1..Reg2ImmOrderCount  ] OF BaseOrder;
   Imm16OrderArray     = ARRAY[1..Imm16OrderCount    ] OF BaseOrder;
   Imm16SwapOrderArray = ARRAY[1..Imm16SwapOrderCount] OF BaseOrder;

VAR
   FixedOrders     : ^FixedOrderArray;
   Reg1Orders      : ^Reg1OrderArray;
   CReg1Orders     : ^CReg1OrderArray;
   CBit1Orders     : ^CBit1OrderArray;
   FReg1Orders     : ^FReg1OrderArray;
   Reg2Orders      : ^Reg2OrderArray;
   CReg2Orders     : ^CReg2OrderArray;
   FReg2Orders     : ^FReg2OrderArray;
   Reg2BOrders     : ^Reg2BOrderArray;
   Reg2SwapOrders  : ^Reg2SwapOrderArray;
   NoDestOrders    : ^NoDestOrderArray;
   Reg3Orders      : ^Reg3OrderArray;
   CReg3Orders     : ^CReg3OrderArray;
   FReg3Orders     : ^FReg3OrderArray;
   Reg3SwapOrders  : ^Reg3SwapOrderArray;
   MixedOrders     : ^MixedOrderArray;
   FReg4Orders     : ^FReg4OrderArray;
   RegDispOrders   : ^RegDispOrderArray;
   FRegDispOrders  : ^FRegDispOrderArray;
   Reg2ImmOrders   : ^Reg2ImmOrderArray;
   Imm16Orders     : ^Imm16OrderArray;
   Imm16SwapOrders : ^Imm16SwapOrderArray;

   SaveInitProc:PROCEDURE;
   BigEndian:Boolean;

   CPU403,CPU505,CPU601,CPU6000:CPUVar;

{---------------------------------------------------------------------------}

{       PROCEDURE EnterByte(b:Byte);
BEGIN
   IF Odd(CodeLen) THEN
    BEGIN
     BAsmCode[CodeLen]:=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]:=b;
    END
   ELSE
    BEGIN
     BAsmCode[CodeLen]:=b;
    END;
   Inc(CodeLen);
END;}

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   IF z=FixedOrderCount THEN Halt;
   Inc(z);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName1; Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddReg1(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   IF z=Reg1OrderCount THEN Halt;
   Inc(z);
   WITH Reg1Orders^[z] DO
    BEGIN
     Name:=NName1; Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddCReg1(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   IF z=CReg1OrderCount THEN Halt;
   Inc(z);
   WITH CReg1Orders^[z] DO
    BEGIN
     Name:=NName1; Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddCBit1(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   IF z=CBit1OrderCount THEN Halt;
   Inc(z);
   WITH CBit1Orders^[z] DO
    BEGIN
     Name:=NName1; Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddFReg1(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   IF z=FReg1OrderCount THEN Halt;
   Inc(z);
   WITH FReg1Orders^[z] DO
    BEGIN
     Name:=NName1; Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddReg2(NName1,NName2:String; NCode:LongInt; NMask:Byte; WithOE,WithFl:Boolean);

	PROCEDURE AddSReg2(NName:String; NCode:LongInt);
BEGIN
   IF z=Reg2OrderCount THEN Halt;
   Inc(z);
   WITH Reg2Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   AddSReg2(NName1,NCode);
   IF WithOE THEN AddSReg2(NName1+'O',NCode OR $400);
   IF WithFL THEN
    BEGIN
     AddSReg2(NName1+'.',NCode OR $001);
     IF WithOE THEN AddSReg2(NName1+'O.',NCode OR $401);
    END;
END;

        PROCEDURE AddCReg2(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF z=CReg2OrderCount THEN Halt;
   Inc(z);
   WITH CReg2Orders^[z] DO
    BEGIN
     IF MomCPU=CPU6000 THEN Name:=NName2 ELSE Name:=NName1;
     Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddFReg2(NName1,NName2:String; NCode:LongInt; NMask:Byte; WithFl:Boolean);

	PROCEDURE AddSFReg2(NName:String; NCode:LongInt);
BEGIN
   IF z=FReg2OrderCount THEN Halt;
   Inc(z);
   WITH FReg2Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   AddSFReg2(NName1,NCode);
   IF WithFl THEN AddSFReg2(NName1+'.',NCode OR 1);
END;

        PROCEDURE AddReg2B(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF z=Reg2BOrderCount THEN Halt;
   Inc(z);
   WITH Reg2BOrders^[z] DO
    BEGIN
     IF MomCPU=CPU6000 THEN Name:=NName2 ELSE Name:=NName1;
     Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddReg2Swap(NName1,NName2:String; NCode:LongInt; NMask:Byte; WithOE,WithFl:Boolean);

	PROCEDURE AddSReg2Swap(NName:String; NCode:LongInt);
BEGIN
   IF z=Reg2SwapOrderCount THEN Halt;
   Inc(z);
   WITH Reg2SwapOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   AddSReg2Swap(NName1,NCode);
   IF WithOE THEN AddSReg2Swap(NName1+'O',NCode OR $400);
   IF WithFL THEN
    BEGIN
     AddSReg2Swap(NName1+'.',NCode OR $001);
     IF WithOE THEN AddSReg2Swap(NName1+'O.',NCode OR $401);
    END;
END;

        PROCEDURE AddNoDest(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF z=NoDestOrderCount THEN Halt;
   Inc(z);
   WITH NoDestOrders^[z] DO
    BEGIN
     IF MomCPU=CPU6000 THEN Name:=NName2 ELSE Name:=NName1;
     Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddReg3(NName1,NName2:String; NCode:LongInt; NMask:Byte; WithOE,WithFl:Boolean);

	PROCEDURE AddSReg3(NName:String; NCode:LongInt);
BEGIN
   IF z=Reg3OrderCount THEN Halt;
   Inc(z);
   WITH Reg3Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   AddSReg3(NName1,NCode);
   IF WithOE THEN AddSReg3(NName1+'O',NCode OR $400);
   IF WithFL THEN
    BEGIN
     AddSReg3(NName1+'.',NCode OR $001);
     IF WithOE THEN AddSReg3(NName1+'O.',NCode OR $401);
    END;
END;

        PROCEDURE AddCReg3(NName:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF z=CReg3OrderCount THEN Halt;
   Inc(z);
   WITH CReg3Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddFReg3(NName1,NName2:String; NCode:LongInt; NMask:Byte; WithFl:Boolean);

	PROCEDURE AddSFReg3(NName:String; NCode:LongInt);
BEGIN
   IF z=FReg3OrderCount THEN Halt;
   Inc(z);
   WITH FReg3Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   AddSFReg3(NName1,NCode);
   IF WithFl THEN AddSFReg3(NName1+'.',NCode OR 1);
END;

        PROCEDURE AddReg3Swap(NName1,NName2:String; NCode:LongInt; NMask:Byte; WithFl:Boolean);

	PROCEDURE AddSReg3Swap(NName:String; NCode:LongInt);
BEGIN
   IF z=Reg3SwapOrderCount THEN Halt;
   Inc(z);
   WITH Reg3SwapOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   AddSReg3Swap(NName1,NCode);
   IF WithFL THEN AddSReg3Swap(NName1+'.',NCode OR 1);
END;

        PROCEDURE AddMixed(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   IF z=MixedOrderCount THEN Halt;
   Inc(z);
   WITH MixedOrders^[z] DO
    BEGIN
     Name:=NName1; Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddFReg4(NName1,NName2:String; NCode:LongInt; NMask:Byte; WithFl:Boolean);

	PROCEDURE AddSFReg4(NName:String; NCode:LongInt);
BEGIN
   IF z=FReg4OrderCount THEN Halt;
   Inc(z);
   WITH FReg4Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   AddSFReg4(NName1,NCode);
   IF WithFl THEN AddSFReg4(NName1+'.',NCode OR 1);
END;

        PROCEDURE AddRegDisp(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF z=RegDispOrderCount THEN Halt;
   Inc(z);
   WITH RegDispOrders^[z] DO
    BEGIN
     IF MomCPU=CPU6000 THEN Name:=NName2 ELSE Name:=NName1;
     Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddFRegDisp(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF z=FRegDispOrderCount THEN Halt;
   Inc(z);
   WITH FRegDispOrders^[z] DO
    BEGIN
     IF MomCPU=CPU6000 THEN Name:=NName2 ELSE Name:=NName1;
     Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddReg2Imm(NName1,NName2:String; NCode:LongInt; NMask:Byte; WithFl:Boolean);

	PROCEDURE AddSReg2Imm(NName:String; NCode:LongInt);
BEGIN
   IF z=Reg2ImmOrderCount THEN Halt;
   Inc(z);
   WITH Reg2ImmOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   IF MomCPU=CPU6000 THEN NName1:=NName2;
   AddSReg2Imm(NName1,NCode);
   IF WithFl THEN AddSReg2Imm(NName1+'.',NCode OR 1);
END;

        PROCEDURE AddImm16(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF z=Imm16OrderCount THEN Halt;
   Inc(z);
   WITH Imm16Orders^[z] DO
    BEGIN
     IF MomCPU=CPU6000 THEN Name:=NName2 ELSE Name:=NName1;
     Code:=NCode; CPUMask:=NMask;
    END;
END;

        PROCEDURE AddImm16Swap(NName1,NName2:String; NCode:LongInt; NMask:Byte);
BEGIN
   IF z=Imm16SwapOrderCount THEN Halt;
   Inc(z);
   WITH Imm16SwapOrders^[z] DO
    BEGIN
     IF MomCPU=CPU6000 THEN Name:=NName2 ELSE Name:=NName1;
     Code:=NCode; CPUMask:=NMask;
    END;
END;

BEGIN
   { --> 0 0 0 }

   New(FixedOrders); z:=0;
   AddFixed('EIEIO'  ,'EIEIO'  ,(31 SHL 26)+(854 SHL 1),$0f);
   AddFixed('ISYNC'  ,'ICS'    ,(19 SHL 26)+(150 SHL 1),$0f);
   AddFixed('RFI'    ,'RFI'    ,(19 SHL 26)+( 50 SHL 1),$0f);
   AddFixed('SC'     ,'SVCA'   ,(17 SHL 26)+(  1 SHL 1),$0f);
   AddFixed('SYNC'   ,'DCS'    ,(31 SHL 26)+(598 SHL 1),$0f);
   AddFixed('RFCI'   ,'RFCI'   ,(19 SHL 26)+( 51 SHL 1),$01);

   { D --> D 0 0 }

   New(Reg1Orders); z:=0;
   AddReg1('MFCR'   ,'MFCR'    ,(31 SHL 26)+( 19 SHL 1),$0f);
   AddReg1('MFMSR'  ,'MFMSR'   ,(31 SHL 26)+( 83 SHL 1),$0f);
   AddReg1('MTMSR'  ,'MTMSR'   ,(31 SHL 26)+(146 SHL 1),$0f);
   AddReg1('WRTEE'  ,'WRTEE'   ,(31 SHL 26)+(131 SHL 1),$0f);

   { crD --> D 0 0 }

   New(CReg1Orders); z:=0;
   AddCReg1('MCRXR'  ,'MCRXR'  ,(31 SHL 26)+(512 SHL 1),$0f);

   { crbD --> D 0 0 }

   New(CBit1Orders); z:=0;
   AddCBit1('MTFSB0' ,'MTFSB0' ,(63 SHL 26)+( 70 SHL 1)  ,$0c);
   AddCBit1('MTFSB0.','MTFSB0.',(63 SHL 26)+( 70 SHL 1)+1,$0c);
   AddCBit1('MTFSB1' ,'MTFSB1' ,(63 SHL 26)+( 38 SHL 1)  ,$0c);
   AddCBit1('MTFSB1.','MTFSB1.',(63 SHL 26)+( 38 SHL 1)+1,$0c);

   { frD --> D 0 0 }

   New(FReg1Orders); z:=0;
   AddFReg1('MFFS'   ,'MFFS'  ,(63 SHL 26)+(583 SHL 1)  ,$0c);
   AddFReg1('MFFS.'  ,'MFFS.' ,(63 SHL 26)+(583 SHL 1)+1,$0c);

   { D,A --> D A 0 }

   New(Reg2Orders); z:=0;
   AddReg2('ABS'   ,'ABS'  ,(31 SHL 26)+(360 SHL 1),$08,True ,True );
   AddReg2('ADDME' ,'AME'  ,(31 SHL 26)+(234 SHL 1),$0f,True ,True );
   AddReg2('ADDZE' ,'AZE'  ,(31 SHL 26)+(202 SHL 1),$0f,True ,True );
   AddReg2('CLCS'  ,'CLCS' ,(31 SHL 26)+(531 SHL 1),$08,False,False);
   AddReg2('NABS'  ,'NABS' ,(31 SHL 26)+(488 SHL 1),$08,True ,True );
   AddReg2('NEG'   ,'NEG'  ,(31 SHL 26)+(104 SHL 1),$0f,True ,True );
   AddReg2('SUBFME','SFME' ,(31 SHL 26)+(232 SHL 1),$0f,True ,True );
   AddReg2('SUBFZE','SFZE' ,(31 SHL 26)+(200 SHL 1),$0f,True ,True );

   { cD,cS --> D S 0 }

   New(CReg2Orders); z:=0;
   AddCReg2('MCRF'  ,'MCRF'  ,(19 SHL 26)+(  0 SHL 1),$0f);
   AddCReg2('MCRFS' ,'MCRFS' ,(63 SHL 26)+( 64 SHL 1),$0c);

   { fD,fB --> D 0 B }

   New(FReg2Orders); z:=0;
   AddFReg2('FABS'  ,'FABS'  ,(63 SHL 26)+(264 SHL 1),$0c,True );
   AddFReg2('FCTIW' ,'FCTIW' ,(63 SHL 26)+( 14 SHL 1),$0c,True );
   AddFReg2('FCTIWZ','FCTIWZ',(63 SHL 26)+( 15 SHL 1),$0c,True );
   AddFReg2('FMR'   ,'FMR'   ,(63 SHL 26)+( 72 SHL 1),$0c,True );
   AddFReg2('FNABS' ,'FNABS' ,(63 SHL 26)+(136 SHL 1),$0c,True );
   AddFReg2('FNEG'  ,'FNEG'  ,(63 SHL 26)+( 40 SHL 1),$0c,True );
   AddFReg2('FRSP'  ,'FRSP'  ,(63 SHL 26)+( 12 SHL 1),$0c,True );

   { D,B --> D 0 B }

   New(Reg2BOrders); z:=0;
   AddReg2B('MFSRIN','MFSRIN',(31 SHL 26)+(659 SHL 1),$0c);
   AddReg2B('MTSRIN','MTSRI' ,(31 SHL 26)+(242 SHL 1),$0c);

   { A,S --> S A 0 }

   New(Reg2SwapOrders); z:=0;
   AddReg2Swap('CNTLZW','CNTLZ' ,(31 SHL 26)+( 26 SHL 1),$0f,False,True );
   AddReg2Swap('EXTSB ','EXTSB' ,(31 SHL 26)+(954 SHL 1),$0f,False,True );
   AddReg2Swap('EXTSH ','EXTS'  ,(31 SHL 26)+(922 SHL 1),$0f,False,True );

   { A,B --> 0 A B }

   New(NoDestOrders); z:=0;
   AddNoDest('DCBF'  ,'DCBF'  ,(31 SHL 26)+(  86 SHL 1),$0f);
   AddNoDest('DCBI'  ,'DCBI'  ,(31 SHL 26)+( 470 SHL 1),$0f);
   AddNoDest('DCBST' ,'DCBST' ,(31 SHL 26)+(  54 SHL 1),$0f);
   AddNoDest('DCBT'  ,'DCBT'  ,(31 SHL 26)+( 278 SHL 1),$0f);
   AddNoDest('DCBTST','DCBTST',(31 SHL 26)+( 246 SHL 1),$0f);
   AddNoDest('DCBZ'  ,'DCLZ'  ,(31 SHL 26)+(1014 SHL 1),$0f);
   AddNoDest('DCCCI' ,'DCCCI' ,(31 SHL 26)+( 454 SHL 1),$01);
   AddNoDest('ICBI'  ,'ICBI'  ,(31 SHL 26)+( 982 SHL 1),$0f);
   AddNoDest('ICBT'  ,'ICBT'  ,(31 SHL 26)+( 262 SHL 1),$01);
   AddNoDest('ICCCI' ,'ICCCI' ,(31 SHL 26)+( 966 SHL 1),$01);

   { D,A,B --> D A B }

   New(Reg3Orders); z:=0;
   AddReg3('ADD'   ,'CAX'   ,(31 SHL 26)+(266 SHL 1),$0f,True, True );
   AddReg3('ADDC'  ,'A'     ,(31 SHL 26)+( 10 SHL 1),$0f,True ,True );
   AddReg3('ADDE'  ,'AE'    ,(31 SHL 26)+(138 SHL 1),$0f,True ,True );
   AddReg3('DIV'   ,'DIV'   ,(31 SHL 26)+(331 SHL 1),$08,True ,True );
   AddReg3('DIVS'  ,'DIVS'  ,(31 SHL 26)+(363 SHL 1),$08,True ,True );
   AddReg3('DIVW'  ,'DIVW'  ,(31 SHL 26)+(491 SHL 1),$0f,True ,True );
   AddReg3('DIVWU' ,'DIVWU' ,(31 SHL 26)+(459 SHL 1),$0f,True ,True );
   AddReg3('DOZ'   ,'DOZ'   ,(31 SHL 26)+(264 SHL 1),$08,True ,True );
   AddReg3('ECIWX' ,'ECIWX' ,(31 SHL 26)+(310 SHL 1),$08,False,False);
   AddReg3('LBZUX' ,'LBZUX' ,(31 SHL 26)+(119 SHL 1),$0f,False,False);
   AddReg3('LBZX'  ,'LBZX'  ,(31 SHL 26)+( 87 SHL 1),$0f,False,False);
   AddReg3('LHAUX' ,'LHAUX' ,(31 SHL 26)+(375 SHL 1),$0f,False,False);
   AddReg3('LHAX'  ,'LHAX'  ,(31 SHL 26)+(343 SHL 1),$0f,False,False);
   AddReg3('LHBRX' ,'LHBRX' ,(31 SHL 26)+(790 SHL 1),$0f,False,False);
   AddReg3('LHZUX' ,'LHZUX' ,(31 SHL 26)+(311 SHL 1),$0f,False,False);
   AddReg3('LHZX'  ,'LHZX'  ,(31 SHL 26)+(279 SHL 1),$0f,False,False);
   AddReg3('LSCBX' ,'LSCBX' ,(31 SHL 26)+(277 SHL 1),$08,False,True );
   AddReg3('LSWX'  ,'LSX'   ,(31 SHL 26)+(533 SHL 1),$0f,False,False);
   AddReg3('LWARX' ,'LWARX' ,(31 SHL 26)+( 20 SHL 1),$0f,False,False);
   AddReg3('LWBRX' ,'LBRX'  ,(31 SHL 26)+(534 SHL 1),$0f,False,False);
   AddReg3('LWZUX' ,'LUX'   ,(31 SHL 26)+( 55 SHL 1),$0f,False,False);
   AddReg3('LWZX'  ,'LX'    ,(31 SHL 26)+( 23 SHL 1),$0f,False,False);
   AddReg3('MUL'   ,'MUL'   ,(31 SHL 26)+(107 SHL 1),$08,True ,True );
   AddReg3('MULHW' ,'MULHW' ,(31 SHL 26)+( 75 SHL 1),$0f,False,True );
   AddReg3('MULHWU','MULHWU',(31 SHL 26)+( 11 SHL 1),$0f,False,True );
   AddReg3('MULLW' ,'MULS'  ,(31 SHL 26)+(235 SHL 1),$0f,True ,True );
   AddReg3('STBUX' ,'STBUX' ,(31 SHL 26)+(247 SHL 1),$0f,False,False);
   AddReg3('STBX'  ,'STBX'  ,(31 SHL 26)+(215 SHL 1),$0f,False,False);
   AddReg3('STHBRX','STHBRX',(31 SHL 26)+(918 SHL 1),$0f,False,False);
   AddReg3('STHUX' ,'STHUX' ,(31 SHL 26)+(439 SHL 1),$0f,False,False);
   AddReg3('STHX'  ,'STHX'  ,(31 SHL 26)+(407 SHL 1),$0f,False,False);
   AddReg3('STSWX' ,'STSX'  ,(31 SHL 26)+(661 SHL 1),$0f,False,False);
   AddReg3('STWBRX','STBRX' ,(31 SHL 26)+(662 SHL 1),$0f,False,False);
   AddReg3('STWCX.','STWCX.',(31 SHL 26)+(150 SHL 1),$0f,False,False);
   AddReg3('STWUX' ,'STUX'  ,(31 SHL 26)+(183 SHL 1),$0f,False,False);
   AddReg3('STWX'  ,'STX'   ,(31 SHL 26)+(151 SHL 1),$0f,False,False);
   AddReg3('SUBF'  ,'SUBF'  ,(31 SHL 26)+( 40 SHL 1),$0f,True ,True );
   AddReg3('SUB'   ,'SUB'   ,(31 SHL 26)+( 40 SHL 1),$0f,True ,True );
   AddReg3('SUBFC' ,'SF'    ,(31 SHL 26)+(  8 SHL 1),$0f,True ,True );
   AddReg3('SUBC'  ,'SUBC'  ,(31 SHL 26)+(  8 SHL 1),$0f,True ,True );
   AddReg3('SUBFE' ,'SFE'   ,(31 SHL 26)+(136 SHL 1),$0f,True ,True );

   { cD,cA,cB --> D A B }

   New(CReg3Orders); z:=0;
   AddCReg3('CRAND'  ,(19 SHL 26)+(257 SHL 1),$0f);
   AddCReg3('CRANDC' ,(19 SHL 26)+(129 SHL 1),$0f);
   AddCReg3('CREQV'  ,(19 SHL 26)+(289 SHL 1),$0f);
   AddCReg3('CRNAND' ,(19 SHL 26)+(225 SHL 1),$0f);
   AddCReg3('CRNOR'  ,(19 SHL 26)+( 33 SHL 1),$0f);
   AddCReg3('CROR'   ,(19 SHL 26)+(449 SHL 1),$0f);
   AddCReg3('CRORC'  ,(19 SHL 26)+(417 SHL 1),$0f);
   AddCReg3('CRXOR'  ,(19 SHL 26)+(193 SHL 1),$0f);

   { fD,fA,fB --> D A B }

   New(FReg3Orders); z:=0;
   AddFReg3('FADD'  ,'FA'    ,(63 SHL 26)+(21 SHL 1),$0c,True );
   AddFReg3('FADDS' ,'FADDS' ,(59 SHL 26)+(21 SHL 1),$0c,True );
   AddFReg3('FDIV'  ,'FD'    ,(63 SHL 26)+(18 SHL 1),$0c,True );
   AddFReg3('FDIVS' ,'FDIVS' ,(59 SHL 26)+(18 SHL 1),$0c,True );
   AddFReg3('FSUB'  ,'FS'    ,(63 SHL 26)+(20 SHL 1),$0c,True );

   { A,S,B --> S A B }

   New(Reg3SwapOrders); z:=0;
   AddReg3Swap('AND'   ,'AND'   ,(31 SHL 26)+(  28 SHL 1),$0f,True );
   AddReg3Swap('ANDC'  ,'ANDC'  ,(31 SHL 26)+(  60 SHL 1),$0f,True );
   AddReg3Swap('ECOWX' ,'ECOWX' ,(31 SHL 26)+( 438 SHL 1),$0c,False);
   AddReg3Swap('EQV'   ,'EQV'   ,(31 SHL 26)+( 284 SHL 1),$0f,True );
   AddReg3Swap('MASKG' ,'MASKG' ,(31 SHL 26)+(  29 SHL 1),$08,True );
   AddReg3Swap('MASKIR','MASKIR',(31 SHL 26)+( 541 SHL 1),$08,True );
   AddReg3Swap('NAND'  ,'NAND'  ,(31 SHL 26)+( 476 SHL 1),$0f,True );
   AddReg3Swap('NOR'   ,'NOR'   ,(31 SHL 26)+( 124 SHL 1),$0f,True );
   AddReg3Swap('OR'    ,'OR'    ,(31 SHL 26)+( 444 SHL 1),$0f,True );
   AddReg3Swap('ORC'   ,'ORC'   ,(31 SHL 26)+( 412 SHL 1),$0f,True );
   AddReg3Swap('RRIB'  ,'RRIB'  ,(31 SHL 26)+( 537 SHL 1),$08,True );
   AddReg3Swap('SLE'   ,'SLE'   ,(31 SHL 26)+( 153 SHL 1),$08,True );
   AddReg3Swap('SLEQ'  ,'SLEQ'  ,(31 SHL 26)+( 217 SHL 1),$08,True );
   AddReg3Swap('SLLQ'  ,'SLLQ'  ,(31 SHL 26)+( 216 SHL 1),$08,True );
   AddReg3Swap('SLQ'   ,'SLQ'   ,(31 SHL 26)+( 152 SHL 1),$08,True );
   AddReg3Swap('SLW'   ,'SL'    ,(31 SHL 26)+(  24 SHL 1),$0f,True );
   AddReg3Swap('SRAQ'  ,'SRAQ'  ,(31 SHL 26)+( 920 SHL 1),$08,True );
   AddReg3Swap('SRAW'  ,'SRA'   ,(31 SHL 26)+( 792 SHL 1),$0f,True );
   AddReg3Swap('SRE'   ,'SRE'   ,(31 SHL 26)+( 665 SHL 1),$08,True );
   AddReg3Swap('SREA'  ,'SREA'  ,(31 SHL 26)+( 921 SHL 1),$08,True );
   AddReg3Swap('SREQ'  ,'SREQ'  ,(31 SHL 26)+( 729 SHL 1),$08,True );
   AddReg3Swap('SRLQ'  ,'SRLQ'  ,(31 SHL 26)+( 728 SHL 1),$08,True );
   AddReg3Swap('SRQ'   ,'SRQ'   ,(31 SHL 26)+( 664 SHL 1),$08,True );
   AddReg3Swap('SRW'   ,'SR'    ,(31 SHL 26)+( 536 SHL 1),$0f,True );
   AddReg3Swap('XOR'   ,'XOR'   ,(31 SHL 26)+( 316 SHL 1),$0f,True );

   { fD,A,B --> D A B }

   New(MixedOrders); z:=0;
   AddMixed('LFDUX' ,'LFDUX' ,(31 SHL 26)+(631 SHL 1),$0c);
   AddMixed('LFDX'  ,'LFDX'  ,(31 SHL 26)+(599 SHL 1),$0c);
   AddMixed('LFSUX' ,'LFSUX' ,(31 SHL 26)+(567 SHL 1),$0c);
   AddMixed('LFSX'  ,'LFSX'  ,(31 SHL 26)+(535 SHL 1),$0c);
   AddMixed('STFDUX','STFDUX',(31 SHL 26)+(759 SHL 1),$0c);
   AddMixed('STFDX' ,'STFDX' ,(31 SHL 26)+(727 SHL 1),$0c);
   AddMixed('STFSUX','STFSUX',(31 SHL 26)+(695 SHL 1),$0c);
   AddMixed('STFSX' ,'STFSX' ,(31 SHL 26)+(663 SHL 1),$0c);

   { fD,fA,fC,fB --> D A B C }

   New(FReg4Orders); z:=0;
   AddFReg4('FMADD'  ,'FMA'    ,(63 SHL 26)+(29 SHL 1),$0c,True );
   AddFReg4('FMADDS' ,'FMADDS' ,(59 SHL 26)+(29 SHL 1),$0c,True );
   AddFReg4('FMSUB'  ,'FMS'    ,(63 SHL 26)+(28 SHL 1),$0c,True );
   AddFReg4('FMSUBS' ,'FMSUBS' ,(59 SHL 26)+(28 SHL 1),$0c,True );
   AddFReg4('FNMADD' ,'FNMA'   ,(63 SHL 26)+(31 SHL 1),$0c,True );
   AddFReg4('FNMADDS','FNMADDS',(59 SHL 26)+(31 SHL 1),$0c,True );
   AddFReg4('FNMSUB' ,'FNMS'   ,(63 SHL 26)+(30 SHL 1),$0c,True );
   AddFReg4('FNMSUBS','FNMSUBS',(59 SHL 26)+(30 SHL 1),$0c,True );

   { D,d(A) --> D A d }

   New(RegDispOrders); z:=0;
   AddRegDisp('LBZ'   ,'LBZ'   ,(34 SHL 26),$0f);
   AddRegDisp('LBZU'  ,'LBZU'  ,(35 SHL 26),$0f);
   AddRegDisp('LHA'   ,'LHA'   ,(42 SHL 26),$0f);
   AddRegDisp('LHAU'  ,'LHAU'  ,(43 SHL 26),$0f);
   AddRegDisp('LHZ'   ,'LHZ'   ,(40 SHL 26),$0f);
   AddRegDisp('LHZU'  ,'LHZU'  ,(41 SHL 26),$0f);
   AddRegDisp('LMW'   ,'LM'    ,(46 SHL 26),$0f);
   AddRegDisp('LWZ'   ,'L'     ,(32 SHL 26),$0f);
   AddRegDisp('LWZU'  ,'LU'    ,(33 SHL 26),$0f);
   AddRegDisp('STB'   ,'STB'   ,(38 SHL 26),$0f);
   AddRegDisp('STBU'  ,'STBU'  ,(39 SHL 26),$0f);
   AddRegDisp('STH'   ,'STH'   ,(44 SHL 26),$0f);
   AddRegDisp('STHU'  ,'STHU'  ,(45 SHL 26),$0f);
   AddRegDisp('STMW'  ,'STM'   ,(47 SHL 26),$0f);
   AddRegDisp('STW'   ,'ST'    ,(36 SHL 26),$0f);
   AddRegDisp('STWU'  ,'STU'   ,(37 SHL 26),$0f);

   { fD,d(A) --> D A d }

   New(FRegDispOrders); z:=0;
   AddFRegDisp('LFD'   ,'LFD'   ,(50 SHL 26),$0c);
   AddFRegDisp('LFDU'  ,'LFDU'  ,(51 SHL 26),$0c);
   AddFRegDisp('LFS'   ,'LFS'   ,(48 SHL 26),$0c);
   AddFRegDisp('LFSU'  ,'LFSU'  ,(49 SHL 26),$0c);
   AddFRegDisp('STFD'  ,'STFD'  ,(54 SHL 26),$0c);
   AddFRegDisp('STFDU' ,'STFDU' ,(55 SHL 26),$0c);
   AddFRegDisp('STFS'  ,'STFS'  ,(52 SHL 26),$0c);
   AddFRegDisp('STFSU' ,'STFSU' ,(53 SHL 26),$0c);

   { A,S,Imm5 --> S A Imm }

   New(Reg2ImmOrders); z:=0;
   AddReg2Imm('SLIQ'  ,'SLIQ'  ,(31 SHL 26)+(184 SHL 1),$08,True);
   AddReg2Imm('SLLIQ' ,'SLLIQ' ,(31 SHL 26)+(248 SHL 1),$08,True);
   AddReg2Imm('SRAIQ' ,'SRAIQ' ,(31 SHL 26)+(952 SHL 1),$08,True);
   AddReg2Imm('SRAWI' ,'SRAI'  ,(31 SHL 26)+(824 SHL 1),$0f,True);
   AddReg2Imm('SRIQ'  ,'SRIQ'  ,(31 SHL 26)+(696 SHL 1),$08,True);
   AddReg2Imm('SRLIQ' ,'SRLIQ' ,(31 SHL 26)+(760 SHL 1),$08,True);

   { D,A,Imm --> D A Imm }

   New(Imm16Orders); z:=0;
   AddImm16('ADDI'   ,'CAL'    ,14 SHL 26,$0f);
   AddImm16('ADDIC'  ,'AI'     ,12 SHL 26,$0f);
   AddImm16('ADDIC.' ,'AI.'    ,13 SHL 26,$0f);
   AddImm16('ADDIS'  ,'CAU'    ,15 SHL 26,$0f);
   AddImm16('DOZI'   ,'DOZI'   , 9 SHL 26,$08);
   AddImm16('MULLI'  ,'MULI'   , 7 SHL 26,$0f);
   AddImm16('SUBFIC' ,'SFI'    , 8 SHL 26,$0c);

   { A,S,Imm --> S A Imm }

   New(Imm16SwapOrders); z:=0;
   AddImm16Swap('ANDI.'  ,'ANDIL.' ,28 SHL 26,$0f);
   AddImm16Swap('ANDIS.' ,'ANDIU.' ,29 SHL 26,$0f);
   AddImm16Swap('ORI'    ,'ORIL'   ,24 SHL 26,$0f);
   AddImm16Swap('ORIS'   ,'ORIU'   ,25 SHL 26,$0f);
   AddImm16Swap('XORI'   ,'XORIL'  ,26 SHL 26,$0f);
   AddImm16Swap('XORIS'  ,'XORIU'  ,27 SHL 26,$0f);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(Reg1Orders);
   Dispose(FReg1Orders);
   Dispose(CReg1Orders);
   Dispose(CBit1Orders);
   Dispose(Reg2Orders);
   Dispose(CReg2Orders);
   Dispose(FReg2Orders);
   Dispose(Reg2BOrders);
   Dispose(Reg2SwapOrders);
   Dispose(NoDestOrders);
   Dispose(Reg3Orders);
   Dispose(CReg3Orders);
   Dispose(FReg3Orders);
   Dispose(Reg3SwapOrders);
   Dispose(MixedOrders);
   Dispose(FReg4Orders);
   Dispose(RegDispOrders);
   Dispose(FRegDispOrders);
   Dispose(Reg2ImmOrders);
   Dispose(Imm16Orders);
   Dispose(Imm16SwapOrders);
END;

{---------------------------------------------------------------------------}

        PROCEDURE PutCode(Code:LongInt);
BEGIN
   Move(Code,BAsmCode[0],4);
   BAsmCode[4]:=BAsmCode[0]; BAsmCode[0]:=BAsmCode[3]; BAsmCode[3]:=BAsmCode[4];
   BAsmCode[4]:=BAsmCode[1]; BAsmCode[1]:=BAsmCode[2]; BAsmCode[2]:=BAsmCode[4];
END;

        PROCEDURE IncCode(Code:LongInt);
BEGIN
   Inc(BAsmCode[0],(Code SHR 24) AND $ff);
   Inc(BAsmCode[1],(Code SHR 16) AND $ff);
   Inc(BAsmCode[2],(Code SHR  8) AND $ff);
   Inc(BAsmCode[3],(Code       ) AND $ff);
END;

{---------------------------------------------------------------------------}

        FUNCTION DecodeGenReg(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   io:ValErgType;
BEGIN
   IF (Length(Asc)<2) OR (UpCase(Asc[1])<>'R') THEN DecodeGenReg:=False
   ELSE
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,io);
     DecodeGenReg:=(io=0) AND (Erg>=0) AND (Erg<=31);
    END;
END;

	FUNCTION DecodeFPReg(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   io:ValErgType;
BEGIN
   IF (Length(Asc)<3) OR (UpCase(Asc[1])<>'F') OR (UpCase(Asc[2])<>'R') THEN DecodeFPReg:=False
   ELSE
    BEGIN
     Val(Copy(Asc,3,Length(Asc)-2),Erg,io);
     DecodeFPReg:=(io=0) AND (Erg>=0) AND (Erg<=31);
    END;
END;

	FUNCTION DecodeCondReg(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   OK:Boolean;
BEGIN
   Erg:=EvalIntExpression(Asc,UInt3,OK) SHL 2;
   DecodeCondReg:=(OK) AND (Erg>=0) AND (Erg<=31);
END;

	FUNCTION DecodeCondBit(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   OK:Boolean;
BEGIN
   Erg:=EvalIntExpression(Asc,UInt5,OK);
   DecodeCondBit:=(OK) AND (Erg>=0) AND (Erg<=31);
END;

	FUNCTION DecodeRegDisp(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   p:Integer;
   OK:Boolean;
BEGIN
   DecodeRegDisp:=False;
   IF Asc[Length(Asc)]<>')' THEN Exit; Dec(Byte(Asc[0]));
   p:=Length(Asc); WHILE (p>0) AND (Asc[p]<>'(') DO Dec(p);
   IF p=0 THEN Exit;
   IF NOT DecodeGenReg(Copy(Asc,p+1,Length(Asc)-p),Erg) THEN Exit;
   Byte(Asc[0]):=p-1;
   p:=EvalIntExpression(Asc,Int16,OK); IF NOT OK THEN Exit;
   Erg:=(Erg SHL 16)+(LongInt(p) AND $ffff); DecodeRegDisp:=True;
END;

{---------------------------------------------------------------------------}

	FUNCTION Convert6000(Name1,Name2:String):Boolean;
BEGIN
   Convert6000:=True;
   IF Memo(Name1) THEN
    IF MomCPU=CPU6000 THEN OpPart:=Name2
    ELSE
     BEGIN
      Convert6000:=False; WrError(1200);
     END;
END;

	FUNCTION PMemo(Name:String):Boolean;
BEGIN
   PMemo:=Memo(Name) OR Memo(Name+'.');
END;

	PROCEDURE IncPoint;
BEGIN
   IF OpPart[Length(OpPart)]='.' THEN IncCode(1);
END;

	PROCEDURE ChkSup;
BEGIN
   IF NOT SupAllowed THEN WrError(50);
END;

	FUNCTION ChkCPU(Mask:Byte):Boolean;
BEGIN
   ChkCPU:=Odd(Mask SHR (Ord(MomCPU)-Ord(CPU403)));
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF601Count=2;
   ONOFF601s:ARRAY[1..ONOFF601Count] OF ONOFFRec=
             ((Name:'SUPMODE'; Dest:@SupAllowed; FlagName:SupAllowedName),
              (Name:'BIGENDIAN'; Dest:@BigEndian; FlagName:BigEndianName));
VAR
   OK:Boolean;
BEGIN
   DecodePseudo:=True;

   IF CodeONOFF(@ONOFF601s,ONOFF601Count) THEN Exit;

   DecodePseudo:=False;
END;

        PROCEDURE SwapCode(VAR Code:LongInt);
BEGIN
   Code:=((Code AND $1f) SHL 5) OR ((Code SHR 5) AND $1f)
END;

	PROCEDURE MakeCode_601;
	Far;
VAR
   z,Imm:Integer;
   Dest,Src1,Src2,Src3:LongInt;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { Nullanweisung }

   IF Memo('') AND (AttrPart='') AND (ArgCnt=0) THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(BigEndian) THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code);
	 IF Memo('RFI') THEN ChkSup;
	END;
       Exit;
      END;

   { ein Register }

   FOR z:=1 TO Reg1OrderCount DO
    WITH Reg1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21));
	 IF Memo('MTMSR') THEN ChkSup;
	END;
       Exit;
      END;

   { ein Steuerregister }

   FOR z:=1 TO CReg1OrderCount DO
    WITH CReg1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeCondReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF Dest AND 3<>0 THEN WrError(1351)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21));
	END;
       Exit;
      END;

   { ein Steuerregisterbit }

   FOR z:=1 TO CBit1OrderCount DO
    WITH CBit1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeCondBit(ArgStr[1],Dest) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21));
	END;
       Exit;
      END;

   { ein Gleitkommaregister }

   FOR z:=1 TO FReg1OrderCount DO
    WITH FReg1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeFPReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21));
	END;
       Exit;
      END;

   { 1/2 Integer-Register }

   FOR z:=1 TO Reg2OrderCount DO
    WITH Reg2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 ArgCnt:=2; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 16));
	END;
       Exit;
      END;

   { 2 Bedingungs-Bits }

   FOR z:=1 TO CReg2OrderCount DO
    WITH CReg2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeCondReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF Dest AND 3<>0 THEN WrError(1351)
       ELSE IF NOT DecodeCondReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE IF Src1 AND 3<>0 THEN WrError(1351)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 16));
	END;
       Exit;
      END;

   { 1/2 Float-Register }

   FOR z:=1 TO FReg2OrderCount DO
    WITH FReg2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 ArgCnt:=2; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeFPReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeFPReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 11));
	END;
       Exit;
      END;

   { 1/2 Integer-Register, Quelle in B }

   FOR z:=1 TO Reg2BOrderCount DO
    WITH Reg2BOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 ArgCnt:=2; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 11));
	 ChkSup;
	END;
       Exit;
      END;

   { 1/2 Integer-Register, getauscht }

   FOR z:=1 TO Reg2SwapOrderCount DO
    WITH Reg2SwapOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 ArgCnt:=2; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 16)+(Src1 SHL 21));
	END;
       Exit;
      END;

   { 2 Integer-Register, kein Ziel }

   FOR z:=1 TO NoDestOrderCount DO
    WITH NoDestOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Src1) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src2) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Src1 SHL 16)+(Src2 SHL 11));
	END;
       Exit;
      END;

   { 2/3 Integer-Register }

   FOR z:=1 TO Reg3OrderCount DO
    WITH Reg3Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=2 THEN
	BEGIN
	 ArgCnt:=3; ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[3],Src2) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 11));
	END;
       Exit;
      END;

   { 2/3 Bedingungs-Bits }

   FOR z:=1 TO CReg3OrderCount DO
    WITH CReg3Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=2 THEN
	BEGIN
	 ArgCnt:=3; ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeCondBit(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeCondBit(ArgStr[2],Src1) THEN WrError(1350)
       ELSE IF NOT DecodeCondBit(ArgStr[3],Src2) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 11));
	END;
       Exit;
      END;

   { 2/3 Float-Register }

   FOR z:=1 TO FReg3OrderCount DO
    WITH FReg3Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=2 THEN
	BEGIN
	 ArgCnt:=3; ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeFPReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeFPReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE IF NOT DecodeFPReg(ArgStr[3],Src2) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 11));
	END;
       Exit;
      END;

   { 2/3 Integer-Register, Ziel & Quelle 1 getauscht }

   FOR z:=1 TO Reg3SwapOrderCount DO
    WITH Reg3SwapOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=2 THEN
	BEGIN
	 ArgCnt:=3; ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[3],Src2) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 16)+(Src1 SHL 21)+(Src2 SHL 11));
	END;
       Exit;
      END;

   { 1 Float und 2 Integer-Register }

   FOR z:=1 TO MixedOrderCount DO
    WITH MixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeFPReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[3],Src2) THEN WrError(1350)
       ELSE
	BEGIN
         CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 11));
	END;
       Exit;
      END;

   { 3/4 Float-Register }

   FOR z:=1 TO FReg4OrderCount DO
    WITH FReg4Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=3 THEN
	BEGIN
	 ArgCnt:=4; ArgStr[4]:=ArgStr[3];
	 ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>4 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeFPReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeFPReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE IF NOT DecodeFPReg(ArgStr[3],Src3) THEN WrError(1350)
       ELSE IF NOT DecodeFPReg(ArgStr[4],Src2) THEN WrError(1350)
       ELSE
	BEGIN
	 CodeLen:=4;
         PutCode(Code+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 11)+(Src3 SHL 6));
	END;
       Exit;
      END;

   { Register mit indiziertem Speicheroperandem }

   FOR z:=1 TO RegDispOrderCount DO
    WITH RegDispOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeRegDisp(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
         PutCode(Code+(Dest SHL 21)+Src1); CodeLen:=4;
	END;
       Exit;
      END;

   { Gleitkommaregister mit indiziertem Speicheroperandem }

   FOR z:=1 TO FRegDispOrderCount DO
    WITH FRegDispOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT DecodeFPReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeRegDisp(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
         PutCode(Code+(Dest SHL 21)+Src1); CodeLen:=4;
	END;
       Exit;
      END;

   { 2 verdrehte Register mit immediate }

   FOR z:=1 TO Reg2ImmOrderCount DO
    WITH Reg2ImmOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
	 Src2:=EvalIntExpression(ArgStr[3],UInt5,OK);
	 IF OK THEN
	  BEGIN
           PutCode(Code+(Src1 SHL 21)+(Dest SHL 16)+(Src2 SHL 11));
	   CodeLen:=4;
	  END;
	END;
       Exit;
      END;

   { 2 Register+immediate }

   FOR z:=1 TO Imm16OrderCount DO
    WITH Imm16Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=2 THEN
	BEGIN
	 ArgCnt:=3; ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
	 Imm:=EvalIntExpression(ArgStr[3],Int16,OK);
	 IF OK THEN
	  BEGIN
           CodeLen:=4; PutCode(Code+(Dest SHL 21)+(Src1 SHL 16)+(Imm AND $ffff));
	  END;
	END;
       Exit;
      END;

   { 2 Register+immediate, Ziel & Quelle 1 getauscht }

   FOR z:=1 TO Imm16SwapOrderCount DO
    WITH Imm16SwapOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=2 THEN
	BEGIN
	 ArgCnt:=3; ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1];
	END;
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF NOT ChkCPU(CPUMask) THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
       ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
       ELSE
	BEGIN
	 Imm:=EvalIntExpression(ArgStr[3],Int16,OK);
	 IF OK THEN
	  BEGIN
           CodeLen:=4; PutCode(Code+(Dest SHL 16)+(Src1 SHL 21)+(Imm AND $ffff));
	  END;
	END;
       Exit;
      END;

   { Ausrei·er... }

   IF NOT Convert6000('FM','FMUL') THEN Exit;
   IF NOT Convert6000('FM.','FMUL.') THEN Exit;

   IF (PMemo('FMUL')) OR (PMemo('FMULS')) THEN
    BEGIN
     IF ArgCnt=2 THEN
      BEGIN
       ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1]; ArgCnt:=3;
      END;
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NOT DecodeFPReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE IF NOT DecodeFPReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE IF NOT DecodeFPReg(ArgStr[3],Src2) THEN WrError(1350)
     ELSE
      BEGIN
       PutCode((59 SHL 26)+(25 SHL 1)+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 6));
       IF PMemo('FMUL') THEN IncCode(4 SHL 26);
       IncPoint;
       CodeLen:=4;
      END;
     Exit;
    END;

   IF NOT Convert6000('LSI','LSWI') THEN Exit;
   IF NOT Convert6000('STSI','STSWI') THEN Exit;

   IF (Memo('LSWI')) OR (Memo('STSWI')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE
      BEGIN
       Src2:=EvalIntExpression(ArgStr[3],UInt5,OK);
       IF OK THEN
	BEGIN
         PutCode((31 SHL 26)+(597 SHL 1)+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 11));
         IF Memo('STSWI') THEN IncCode(128 SHL 1);
	 CodeLen:=4;
	END;
      END;
     Exit;
    END;

   IF (Memo('MFSPR')) OR (Memo('MTSPR')) THEN
    BEGIN
     IF Memo('MTSPR') THEN
      BEGIN
       ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE
      BEGIN
       Src1:=EvalIntExpression(ArgStr[2],UInt10,OK);
       IF OK THEN
        BEGIN
         SwapCode(Src1);
         PutCode((31 SHL 26)+(Dest SHL 21)+(Src1 SHL 11));
         IF Memo('MFSPR') THEN IncCode((339 SHL 1))
         ELSE IncCode((467 SHL 1));
         CodeLen:=4;
        END;
      END;
     Exit;
    END;

   IF (Memo('MFDCR')) OR (Memo('MTDCR')) THEN
    BEGIN
     IF Memo('MTDCR') THEN
      BEGIN
       ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<>CPU403 THEN WrXError(1500,OpPart)
     ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE
      BEGIN
       Src1:=EvalIntExpression(ArgStr[2],UInt10,OK);
       IF OK THEN
        BEGIN
         SwapCode(Src1);
         PutCode((31 SHL 26)+(Dest SHL 21)+(Src1 SHL 11));
         IF Memo('MFDCR') THEN IncCode((323 SHL 1))
         ELSE IncCode((451 SHL 1));
         CodeLen:=4;
        END;
      END;
     Exit;
    END;

   IF (Memo('MFSR')) OR (Memo('MTSR')) THEN
    BEGIN
     IF Memo('MTSR') THEN
      BEGIN
       ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE
      BEGIN
       Src1:=EvalIntExpression(ArgStr[2],UInt4,OK);
       IF OK THEN
	BEGIN
         PutCode((31 SHL 26)+(Dest SHL 21)+(Src1 SHL 16));
         IF Memo('MFSR') THEN IncCode(595 SHL 1)
         ELSE IncCode(210 SHL 1);
	 CodeLen:=4; ChkSup;
	END;
      END;
     Exit;
    END;

   IF Memo('MTCRF') THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[ArgCnt],Src1) THEN WrError(1350)
     ELSE
      BEGIN
       OK:=True;
       IF ArgCnt=1 THEN Dest:=$ff
       ELSE Dest:=EvalIntExpression(ArgStr[1],UInt8,OK);
       IF OK THEN
        BEGIN
         PutCode((31 SHL 26)+(Src1 SHL 26)+(Dest SHL 12)+(144 SHL 1));
         CodeLen:=4;
        END;
      END;
     Exit;
    END;

   IF PMemo('MTFSF') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeFPReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE
      BEGIN
       Dest:=EvalIntExpression(ArgStr[1],UInt8,OK);
       IF OK THEN
	BEGIN
         PutCode((63 SHL 26)+(Dest SHL 17)+(Src1 SHL 11)+(711 SHL 1));
	 IncPoint;
	 CodeLen:=4;
	END;
      END;
     Exit;
    END;

   IF PMemo('MTFSFI') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeCondReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE IF Dest AND 3<>0 THEN WrError(1351)
     ELSE
      BEGIN
       Src1:=EvalIntExpression(ArgStr[2],UInt4,OK);
       IF OK THEN
	BEGIN
         PutCode((63 SHL 26)+(Dest SHL 21)+(Src1 SHL 12)+(134 SHL 1));
	 IncPoint;
	 CodeLen:=4;
	END;
      END;
     Exit;
    END;

   IF PMemo('RLMI') THEN
    BEGIN
     IF ArgCnt<>5 THEN WrError(1110)
     ELSE IF MomCPU<CPU6000 THEN WrXError(1500,OpPart)
     ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE IF NOT DecodeGenReg(ArgStr[3],Src2) THEN WrError(1350)
     ELSE
      BEGIN
       Src3:=EvalIntExpression(ArgStr[4],UInt5,OK);
       IF OK THEN
	BEGIN
	 Imm:=EvalIntExpression(ArgStr[5],UInt5,OK);
	 IF OK THEN
	  BEGIN
           PutCode((22 SHL 26)+(Src1 SHL 21)+(Dest SHL 16)
                       +(Src2 SHL 11)+(Src3 SHL 6)+(Imm SHL 1));
	   IncPoint;
	   CodeLen:=4;
	  END;
	END;
      END;
     Exit;
    END;

   IF NOT Convert6000('RLNM','RLWNM') THEN Exit;
   IF NOT Convert6000('RLNM.','RLWNM.') THEN Exit;

   IF PMemo('RLWNM') THEN
    BEGIN
     IF ArgCnt<>5 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE IF NOT DecodeGenReg(ArgStr[3],Src2) THEN WrError(1350)
     ELSE
      BEGIN
       Src3:=EvalIntExpression(ArgStr[4],UInt5,OK);
       IF OK THEN
	BEGIN
	 Imm:=EvalIntExpression(ArgStr[5],UInt5,OK);
	 IF OK THEN
	  BEGIN
           PutCode((23 SHL 26)+(Src1 SHL 21)+(Dest SHL 16)
                       +(Src2 SHL 11)+(Src3 SHL 6)+(Imm SHL 1));
	   IncPoint;
	   CodeLen:=4;
	  END;
	END;
      END;
     Exit;
    END;

   IF NOT Convert6000('RLIMI','RLWIMI') THEN Exit;
   IF NOT Convert6000('RLIMI.','RLWIMI.') THEN Exit;
   IF NOT Convert6000('RLINM','RLWINM') THEN Exit;
   IF NOT Convert6000('RLINM.','RLWINM.') THEN Exit;

   IF (PMemo('RLWIMI')) OR (PMemo('RLWINM')) THEN
    BEGIN
     IF ArgCnt<>5 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE
      BEGIN
       Src2:=EvalIntExpression(ArgStr[3],UInt5,OK);
       IF OK THEN
	BEGIN
	 Src3:=EvalIntExpression(ArgStr[4],UInt5,OK);
	 IF OK THEN
	  BEGIN
	   Imm:=EvalIntExpression(ArgStr[5],UInt5,OK);
	   IF OK THEN
	    BEGIN
             PutCode((20 SHL 26)+(Dest SHL 16)+(Src1 SHL 21)
                         +(Src2 SHL 11)+(Src3 SHL 6)+(Imm SHL 1));
             IF PMemo('RLWINM') THEN IncCode(1 SHL 26);
	     IncPoint;
	     CodeLen:=4;
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF NOT Convert6000('TLBI','TLBIE') THEN Exit;

   IF Memo('TLBIE') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[1],Src1) THEN WrError(1350)
     ELSE
      BEGIN
       PutCode((31 SHL 26)+(Src1 SHL 11)+(306 SHL 1));
       CodeLen:=4; ChkSup;
      END;
     Exit;
    END;

   IF NOT Convert6000('T','TW') THEN Exit;

   IF Memo('TW') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE IF NOT DecodeGenReg(ArgStr[3],Src2) THEN WrError(1350)
     ELSE
      BEGIN
       Dest:=EvalIntExpression(ArgStr[1],UInt5,OK);
       IF OK THEN
	BEGIN
         PutCode((31 SHL 26)+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 11)+(4 SHL 1));
	 CodeLen:=4;
	END;
      END;
     Exit;
    END;

   IF NOT Convert6000('TI','TWI') THEN Exit;

   IF Memo('TWI') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE
      BEGIN
       Imm:=EvalIntExpression(ArgStr[3],Int16,OK);
       IF OK THEN
	BEGIN
	 Dest:=EvalIntExpression(ArgStr[1],UInt5,OK);
	 IF OK THEN
	  BEGIN
           PutCode((3 SHL 26)+(Dest SHL 21)+(Src1 SHL 16)+(Imm AND $ffff));
	   CodeLen:=4;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('WRTEEI') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<>CPU403 THEN WrXError(1500,OpPart)
     ELSE
      BEGIN
       Src1:=EvalIntExpression(ArgStr[1],UInt1,OK) SHL 15;
       IF OK THEN
        BEGIN
         PutCode((31 SHL 26)+Src1+(163 SHL 1));
         CodeLen:=4;
        END;
      END;
     Exit;
    END;

   { Vergleiche }

   IF (Memo('CMP')) OR (Memo('CMPL')) THEN
    BEGIN
     IF ArgCnt=3 THEN
      BEGIN
       ArgStr[4]:=ArgStr[3]; ArgStr[3]:=ArgStr[2]; ArgStr[2]:='0'; ArgCnt:=4;
      END;
     IF ArgCnt<>4 THEN WrError(1110)
     ELSE IF NOT DecodeGenReg(ArgStr[4],Src2) THEN WrError(1350)
     ELSE IF NOT DecodeGenReg(ArgStr[3],Src1) THEN WrError(1350)
     ELSE IF NOT DecodeCondReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE IF Dest AND 3<>0 THEN WrError(1351)
     ELSE
      BEGIN
       Src3:=EvalIntExpression(ArgStr[2],UInt1,OK);
       IF OK THEN
	BEGIN
         PutCode((31 SHL 26)+(Dest SHL 21)+(Src3 SHL 21)+(Src1 SHL 16)
                     +(Src2 SHL 11));
         IF Memo('CMPL') THEN IncCode(32 SHL 1);
	 CodeLen:=4;
	END;
      END;
     Exit;
    END;

   IF (Memo('FCMPO')) OR (Memo('FCMPU')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NOT DecodeFPReg(ArgStr[3],Src2) THEN WrError(1350)
     ELSE IF NOT DecodeFPReg(ArgStr[2],Src1) THEN WrError(1350)
     ELSE IF NOT DecodeCondReg(ArgStr[1],Dest) THEN WrError(1350)
     ELSE IF Dest AND 3<>0 THEN WrError(1351)
     ELSE
      BEGIN
       PutCode((63 SHL 26)+(Dest SHL 21)+(Src1 SHL 16)+(Src2 SHL 11));
       IF Memo('FCMPO') THEN IncCode(32 SHL 1);
       CodeLen:=4;
      END;
     Exit;
    END;

   IF (Memo('CMPI')) OR (Memo('CMPLI'))THEN
    BEGIN
     IF ArgCnt=3 THEN
      BEGIN
       ArgStr[4]:=ArgStr[3]; ArgStr[3]:=ArgStr[2]; ArgStr[2]:='0'; ArgCnt:=4;
      END;
     IF ArgCnt<>4 THEN WrError(1110)
     ELSE
      BEGIN
       Src2:=EvalIntExpression(ArgStr[4],Int16,OK);
       IF OK THEN
	IF NOT DecodeGenReg(ArgStr[3],Src1) THEN WrError(1350)
	ELSE IF NOT DecodeCondReg(ArgStr[1],Dest) THEN WrError(1350)
	ELSE IF Dest AND 3<>0 THEN WrError(1351)
	ELSE
	 BEGIN
	  Src3:=EvalIntExpression(ArgStr[2],UInt1,OK);
	  IF OK THEN
	   BEGIN
            PutCode((10 SHL 26)+(Dest SHL 21)+(Src3 SHL 21)
                        +(Src1 SHL 16)+(Src2 AND $ffff));
            IF Memo('CMPI') THEN IncCode(1 SHL 26);
	    CodeLen:=4;
	   END;
	 END;
      END;
     Exit;
    END;

   { SprÅnge }

   IF (Memo('B')) OR (Memo('BL')) OR (Memo('BA')) OR (Memo('BLA')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       Dest:=EvalIntExpression(ArgStr[1],Int32,OK);
       IF OK THEN
	BEGIN
	 IF (Memo('B')) OR (Memo('BL')) THEN Dec(Dest,EProgCounter);
         IF (NOT SymbolQuestionable) AND (Dest>$1ffffff) THEN WrError(1320)
         ELSE IF (NOT SymbolQuestionable) AND (Dest<-$2000000) THEN WrError(1315)
	 ELSE IF Dest AND 3<>0 THEN WrError(1375)
	 ELSE
	  BEGIN
           PutCode((18 SHL 26)+(Dest AND $03fffffc));
           IF (Memo('BA')) OR (Memo('BLA')) THEN IncCode(2);
           IF (Memo('BL')) OR (Memo('BLA')) THEN IncCode(1);
	   CodeLen:=4;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('BC')) OR (Memo('BCL')) OR (Memo('BCA')) OR (Memo('BCLA')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       Src1:=EvalIntExpression(ArgStr[1],UInt5,OK); { BO }
       IF OK THEN
	BEGIN
         Src2:=EvalIntExpression(ArgStr[2],UInt5,OK); { BI }
	 IF OK THEN
	  BEGIN
           Dest:=EvalIntExpression(ArgStr[3],Int32,OK); { ADR }
	   IF OK THEN
	    BEGIN
	     IF (Memo('BC')) OR (Memo('BCL')) THEN Dec(Dest,EProgCounter);
             IF (NOT SymbolQuestionable) AND (Dest>$7fff) THEN WrError(1320)
             ELSE IF (NOT SymbolQuestionable) AND (Dest<-$8000) THEN WrError(1315)
	     ELSE IF Dest AND 3<>0 THEN WrError(1375)
	     ELSE
	      BEGIN
               PutCode((16 SHL 26)+(Src1 SHL 21)+(Src2 SHL 16)+(Dest AND $fffc));
               IF (Memo('BCA')) OR (Memo('BCLA')) THEN IncCode(2);
               IF (Memo('BCL')) OR (Memo('BCLA')) THEN IncCode(1);
	       CodeLen:=4;
	      END;
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF NOT Convert6000('BCC','BCCTR') THEN Exit;
   IF NOT Convert6000('BCCL','BCCTRL') THEN Exit;
   IF NOT Convert6000('BCR','BCLR') THEN Exit;
   IF NOT Convert6000('BCRL','BCLRL') THEN Exit;

   IF (Memo('BCCTR')) OR (Memo('BCCTRL')) OR (Memo('BCLR')) OR (Memo('BCLRL')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       Src1:=EvalIntExpression(ArgStr[1],UInt5,OK);
       IF OK THEN
	BEGIN
	 Src2:=EvalIntExpression(ArgStr[2],UInt5,OK);
	 IF OK THEN
	  BEGIN
           PutCode((19 SHL 26)+(Src1 SHL 21)+(Src2 SHL 16));
	   IF (Memo('BCCTR')) OR (Memo('BCCTRL')) THEN
            IncCode(528 SHL 1)
	   ELSE
            IncCode(16 SHL 1);
           IF (Memo('BCCTRL')) OR (Memo('BCLRL')) THEN IncCode(1);
	   CodeLen:=4;
	  END;
	END;
      END;
     Exit;
    END;

   { unbekannter Befehl }

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_601:Boolean;
	Far;
BEGIN
   ChkPC_601:=ActPC=SegCode;
END;

	FUNCTION IsDef_601:Boolean;
	Far;
BEGIN
   IsDef_601:=False;
END;

        PROCEDURE InitPass_601;
        Far;
BEGIN
   SaveInitProc;
   SetFlag(BigEndian,BigEndianName,False);
END;

	PROCEDURE InternSymbol_601(VAR Asc:String; VAR Erg:TempResult);
        Far;
BEGIN
   Erg.Typ:=TempNone;
   IF (Length(Asc)=3) OR (Length(Asc)=4) THEN
    IF (UpCase(Asc[1])='C') AND (UpCase(Asc[2])='R') THEN
     IF (Asc[Length(Asc)]>='0') AND (Asc[Length(Asc)]<='7') THEN
      IF (Length(Asc)=3) XOR ((UpCase(Asc[3])='F') OR (UpCase(Asc[3])='B')) THEN
       BEGIN
        Erg.Typ:=TempInt; Erg.Int:=Ord(Asc[Length(Asc)])-AscOfs;
       END;
END;

	PROCEDURE SwitchFrom_601;
	Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_601;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeC; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$05; NOPCode:=$000000000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_601; ChkPC:=ChkPC_601; IsDef:=IsDef_601;
   SwitchFrom:=SwitchFrom_601; InternSymbol:=InternSymbol_601;

   InitFields;
END;

BEGIN
   CPU403 :=AddCPU('PPC403',SwitchTo_601);
   CPU505 :=AddCPU('MPC505',SwitchTo_601);
   CPU601 :=AddCPU('MPC601',SwitchTo_601);
   CPU6000:=AddCPU('RS6000',SwitchTo_601);

   SaveInitProc:=InitPassProc; InitPassProc:=InitPass_601;
END.
