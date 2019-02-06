{$i STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
        Unit AsmPars;

INTERFACE

        Uses NLS,StringUt,FileNums,Chunks,
             AsmDef,AsmSub;

TYPE
   IntType= (UInt1    ,UInt2    ,UInt3    ,SInt4    ,UInt4    , Int4    ,
             SInt5    ,UInt5    , Int5    ,UInt6    ,SInt7    ,UInt7    ,SInt8    ,UInt8    ,
              Int8    ,SInt9    ,UInt9    ,UInt10   , Int10   ,UInt11   ,UInt12   , Int12   ,
             UInt13   ,UInt15   ,SInt16   ,UInt16   , Int16   ,UInt18   ,SInt20   ,
             UInt20   , Int20   ,UInt22   ,SInt24   ,UInt24   , Int24   , Int32);

   FloatType=(Float32,Float64,Float80,FloatDec,FloatCo);

CONST
   IntMasks:ARRAY[IntType] OF LongInt=
            ($00000001,$00000002,$00000007,$00000007,$0000000f,$0000000f,
             $0000001f,$0000001f,$0000001f,$0000003f,$0000007f,$0000007f,$0000007f,$000000ff,
             $000000ff,$000001ff,$000001ff,$000003ff,$000003ff,$000007ff,$00000fff,$00000fff,
             $00001fff,$0000ffff,$00007fff,$0000ffff,$0000ffff,$0003ffff,$0007ffff,
             $000fffff,$000fffff,$003fffff,$007fffff,$00ffffff,$00ffffff,$ffffffff);
   IntMins :ARRAY[IntType] OF LongInt=
            (        0,        0,        0,       -8,        0,       -8,
                   -16,        0,      -16,        0,      -64,        0,     -128,        0,
                  -128,     -256,        0,        0,     -511,        0,        0,    -2047,
                     0,        0,   -32768,        0,   -32768,        0,  -524288,
                     0,  -524288,        0, -8388608,        0, -8388608,-$7fffffff);
   IntMaxs :ARRAY[IntType] OF LongInt=
            (        1,        3,        7,        7,       15,       15,
                    15,       31,       31,       63,       63,      127,      127,      255,
                   255,      255,      511,     1023,     1023,     2047,     4095,     4095,
                  8191,    32767,    32767,    65535,    65535,   262143,   524287,
               1048575,  1048575,  4194303,  8388607, 16777215, 16777215,$7fffffff);


        PROCEDURE AsmParsInit;


        FUNCTION  SingleBit(Inp:LongInt; VAR Erg:LongInt):Boolean;


        FUNCTION  ConstIntVal(Asc:String; Typ:IntType; VAR Ok:Boolean):LongInt;

        FUNCTION  ConstFloatVal(Asc:String; Typ:FloatType; VAR Ok:Boolean):Extended;

        FUNCTION  ConstStringVal(Asc:String; VAR OK:Boolean):String;


        PROCEDURE EvalExpression(Asc:String; VAR Erg:TempResult);

        FUNCTION  EvalIntExpression(Asc:String; Typ:IntType; VAR Ok:Boolean):LongInt;

        FUNCTION  EvalFloatExpression(Asc:String; Typ:FloatType; VAR Ok:Boolean):Extended;

        FUNCTION  EvalStringExpression(Asc:String; VAR Ok:Boolean):String;


        FUNCTION  RangeCheck(Wert:LongInt; Typ:IntType):Boolean;

        FUNCTION  FloatRangeCheck(Wert:Extended; Typ:FloatType):Boolean;


        FUNCTION  IdentifySection(Name:String; VAR Erg:LongInt):Boolean;

        FUNCTION  ExpandSymbol(VAR Name:String):Boolean;

        PROCEDURE EnterIntSymbol(Name:String; Wert:LongInt; Typ:Byte; MayChange:Boolean);

        PROCEDURE EnterFloatSymbol(Name:String; Wert:Extended; MayChange:Boolean);

        PROCEDURE EnterStringSymbol(Name:String; VAR Wert:String; MayChange:Boolean);

        FUNCTION  GetIntSymbol(Name:String; VAR Wert:LongInt):Boolean;

        FUNCTION  GetFloatSymbol(Name:String; VAR Wert:Extended):Boolean;

        FUNCTION  GetStringSymbol(Name:String; VAR Wert:String):Boolean;

        PROCEDURE PrintSymbolList;

        PROCEDURE PrintDebSymbols(VAR f:Text);

        PROCEDURE PrintSymbolTree;

        PROCEDURE ClearSymbolList;

        PROCEDURE ResetSymbolDefines;

        PROCEDURE PrintSymbolDepth;


        PROCEDURE SetSymbolSize(Name:String; Size:ShortInt);

        FUNCTION  GetSymbolSize(Name:String):ShortInt;


        FUNCTION  IsSymbolFloat(Name:String):Boolean;

        FUNCTION  IsSymbolString(Name:String):Boolean;

        FUNCTION  IsSymbolDefined(Name:String):Boolean;

        FUNCTION  IsSymbolUsed(Name:String):Boolean;

        FUNCTION  IsSymbolChangeable(Name:String):Boolean;

        FUNCTION  GetSymbolType(Name:String):Integer;


        FUNCTION  PushSymbol(SymName,StackName:String):Boolean;

        FUNCTION  PopSymbol(SymName,StackName:String):Boolean;

        PROCEDURE ClearStacks;


        PROCEDURE EnterFunction(FName,FDefinition:String; NewCnt:Byte);

        FUNCTION  FindFunction(Name:String):PFunction;

        PROCEDURE PrintFunctionList;

        PROCEDURE ClearFunctionList;


        PROCEDURE AddDefSymbol(Name:String; VAR Value:TempResult);

        PROCEDURE RemoveDefSymbol(Name:String);

        PROCEDURE CopyDefSymbols;


        PROCEDURE PrintCrossList;

        PROCEDURE ClearCrossList;


        FUNCTION  GetSectionHandle(SName:String; AddEmpt:Boolean; Parent:LongInt):LongInt;

        FUNCTION  GetSectionName(Handle:LongInt):String;

        PROCEDURE SetMomSection(Handle:LongInt);

        PROCEDURE AddSectionUsage(Start,Length:LongInt);

        PROCEDURE PrintSectionList;

        PROCEDURE PrintDebSections(VAR f:Text);

        PROCEDURE ClearSectionList;


        PROCEDURE SetFlag(VAR Flag:Boolean; Name:String; Wert:Boolean);


        FUNCTION  GetLocHandle:LongInt;

        PROCEDURE PushLocHandle(NewLoc:LongInt);

        PROCEDURE PopLocHandle;

        PROCEDURE ClearLocStack;


        PROCEDURE AddRegDef(Orig,Repl:String);

        FUNCTION  FindRegDef(Name:String; VAR Erg:StringPtr):Boolean;

        PROCEDURE TossRegDefs(Sect:LongInt);

        PROCEDURE CleanupRegDefs;

        PROCEDURE ClearRegDefs;

        PROCEDURE PrintRegDefs;


        PROCEDURE _Init_AsmPars;

VAR
   FirstPassUnknown:Boolean;    { Hinweisflag: evtl. im ersten Pass unbe-
                                  kanntes Symbol, Ausdruck nicht ausgewertet }
   SymbolQuestionable:Boolean;  { Hinweisflag:  Dadurch, da· Phasenfehler
                                  aufgetreten sind, ist dieser Symbolwert evtl.
                                  nicht mehr aktuell }
   UsesForwards:Boolean;        { Hinweisflag: benutzt VorwÑrtsdefinitionen }
   MomLocHandle:LongInt;        { Merker, den lokale Symbole erhalten }

   LocHandleCnt:LongInt;        { mom. verwendeter lokaler Handle }

   BalanceTree:Boolean;         { Symbolbaum ausbalancieren }

Implementation

{$i AS.RSC}

CONST
   DigitVals                      ='0123456789ABCDEF';
   BaseIds:    ARRAY[0..2] OF Char='%@$';
   BaseLetters:ARRAY[0..2] OF Char='BOH';
   BaseVals:   ARRAY[0..2] OF Byte=(2,8,16);

TYPE
   PCrossRef=^TCrossRef;
   TCrossRef=RECORD
              Next:PCrossRef;
              FileNum:Byte;
              LineNum:LongInt;
              OccNum:Integer;
             END;

   SymbolVal=RECORD CASE Typ:TempType OF
              TempInt   :(IWert:LongInt);
              TempFloat :(FWert:Extended);
              TempString:(SWert:StringPtr);
             END;

   SymbolPtr=^SymbolEntry;
   SymbolEntry=RECORD
                Left,Right:SymbolPtr;
                Balance:ShortInt;
                Attribute:LongInt;
                SymName:StringPtr;
                SymType:Byte;
                SymSize:ShortInt;
                Defined,Used,Changeable:Boolean;
                SymWert:SymbolVal;
                RefList:PCrossRef;
                FileNum:Byte;
                LineNum:LongInt;
               END;

   PSymbolStackEntry=^TSymbolStackEntry;
   TSymbolStackEntry=RECORD
                      Next:PSymbolStackEntry;
                      Contents:SymbolVal;
                     END;

   PSymbolStack=^TSymbolStack;
   TSymbolStack=RECORD
                 Next:PSymbolStack;
                 Name:^String;
                 Contents:PSymbolStackEntry;
                END;

   PDefSymbol=^TDefSymbol;
   TDefSymbol=RECORD
               Next:PDefSymbol;
               SymName:StringPtr;
               Wert:TempResult;
              END;

   PCToken=^TCToken;
   TCToken=RECORD
            Next:PCToken;
            Name:StringPtr;
            Parent:LongInt;
            Usage:ChunkList;
           END;

   Operator=RECORD
             Id:String[3];
             Dyadic:Boolean;
             Priority:Byte;
             MayInt:Boolean;
             MayFloat:Boolean;
             MayString:Boolean;
             Present:Boolean;
            END;

   PLocHandle=^TLocHeap;
   TLocHeap=RECORD
             Next:PLocHandle;
             Cont:LongInt;
            END;

   PRegDefList=^TRegDefList;
   TRegDefList=RECORD
                Next:PRegDefList;
                Section:LongInt;
                Value:StringPtr;
                Used:Boolean;
               END;
   PRegDef=^TRegDef;
   TRegDef=RECORD
            Left,Right:PRegDef;
            Orig:StringPtr;
            Defs,DoneDefs:PRegDefList;
           END;

VAR
   FirstSymbol,FirstLocSymbol:SymbolPtr;
   FirstDefSymbol:PDefSymbol;
   FirstSection:PCToken;
   FirstRegDef:PRegDef;
   DoRefs:Boolean;              { Querverweise protokollieren }
   FirstLocHandle:PLocHandle;
   FirstStack:PSymbolStack;
   MomSection:PCToken;

        PROCEDURE AsmParsInit;
BEGIN
   FirstSymbol:=Nil;
   FirstLocSymbol:=Nil; MomLocHandle:=-1; SetMomSection(-1);
   FirstSection:=Nil;
   FirstLocHandle:=Nil;
   FirstStack:=Nil;
   FirstRegDef:=Nil;
   DoRefs:=True;
END;


        FUNCTION RangeCheck(Wert:LongInt; Typ:IntType):Boolean;
BEGIN
   RangeCheck:=(Wert>=IntMins[Typ]) AND (Wert<=IntMaxs[Typ]);
END;

        FUNCTION FloatRangeCheck(Wert:Extended; Typ:FloatType):Boolean;
BEGIN
   CASE Typ OF
   Float32:FloatRangeCheck:=Abs(Wert)<=3.4e38;
   Float64:FloatRangeCheck:=Abs(Wert)<=1.7e308;
   FloatCo:FloatRangeCheck:=Abs(Wert)<=9.22e18;
   Float80:FloatRangeCheck:=True;
   FloatDec:FloatRangeCheck:=True;
   END;
   IF (Typ=FloatDec) AND (Abs(Wert)>1e1000) THEN WrError(40);
END;

        FUNCTION SingleBit(Inp:LongInt; VAR Erg:LongInt):Boolean;
BEGIN
   Erg:=0;
   REPEAT
    IF NOT Odd(Inp) THEN Inc(Erg);
    IF NOT Odd(Inp) THEN Inp:=Inp SHR 1;
   UNTIL (Erg=32) OR (Odd(Inp));
   SingleBit:=(Erg<>32) AND (Inp=1);
END;


        PROCEDURE ReplaceBkSlashes(VAR s:String);
VAR
   p,LastPos,cnt:Integer;
   err:ValErgType;
   ErgChar:Char;
BEGIN
   p:=Pos('\',s); LastPos:=0;
   WHILE (p>LastPos) AND (p<Length(s)) DO
    BEGIN
     ErgChar:='\'; cnt:=1;
     CASE UpCase(s[p+1]) OF
     '''','\',
     '"':BEGIN
          ErgChar:=s[p+1]; cnt:=2;
         END;
     'H':BEGIN
          ErgChar:=''''; cnt:=2;
         END;
     'I':BEGIN
          ErgChar:='"'; cnt:=2;
         END;
     'B':BEGIN
          ErgChar:=Char_BS; cnt:=2;
         END;
     'A':BEGIN
          ErgChar:=Char_BEL; cnt:=2;
         END;
     'E':BEGIN
          ErgChar:=Char_ESC; cnt:=2;
         END;
     'T':BEGIN
          ErgChar:=Char_HT; cnt:=2;
         END;
     'N':BEGIN
          ErgChar:=Char_LF; cnt:=2;
         END;
     'R':BEGIN
          ErgChar:=Char_CR; cnt:=2;
         END;
     '0'..'9':BEGIN
               cnt:=2;
               WHILE (Length(s)>p+cnt-1) AND (cnt<4) AND (s[p+cnt] IN ['0'..'9']) DO Inc(cnt);
               Val(Copy(s,p+1,cnt-1),Byte(ErgChar),err);
               IF err<>0 THEN WrError(1320);
              END;
     ELSE WrError(1135);
     END;
     s[p]:=ErgChar; Delete(s,p+1,cnt-1);
     LastPos:=p; p:=Pos('\',Copy(s,p+1,Length(s)-p));
     IF p<>0 THEN Inc(p,LastPos);
    END;
END;


        FUNCTION ExpandSymbol(VAR Name:String):Boolean;
VAR
   p1,p2:Integer;
   h:String;
   OK:Boolean;
BEGIN
   ExpandSymbol:=False;
   REPEAT
    p1:=Pos('{',Name);
    IF p1=0 THEN
     BEGIN
      ExpandSymbol:=True; Exit;
     END;
    h:=Copy(Name,p1+1,Length(Name)-p1); p2:=QuotPos(h,'}')+p1;
    IF p2>Length(Name) THEN
     BEGIN
      ExpandSymbol:=False; WrXError(1020,Name); Exit;
     END;
    h:=Copy(Name,p1+1,p2-p1-1); Delete(Name,p1,p2-p1+1);
    FirstPassUnknown:=False;
    h:=EvalStringExpression(h,OK);
    IF (NOT OK) OR (FirstPassUnknown) THEN
     BEGIN
      WrError(1820); ExpandSymbol:=False; Exit;
     END;
    IF NOT CaseSensitive THEN UpString(h);
    Insert(h,Name,p1);
   UNTIL False;
END;

        FUNCTION IdentifySection(Name:String; VAR Erg:LongInt):Boolean;
VAR
   SLauf:PSaveSection;
   Depth:Integer;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     IdentifySection:=False; Exit;
    END;
   IF NOT CaseSensitive THEN NLS_UpString(Name);
   IF Name='' THEN
    BEGIN
     IdentifySection:=True; Erg:=-1
    END
   ELSE IF ((Length(Name)=6) OR (Length(Name)=7))
       AND (NLS_LStrCaseCmp(Copy(Name,1,6),'PARENT')=0)
       AND ((Length(Name)=6) OR ((Name[7]>='0') AND (Name[7]<='9'))) THEN
    BEGIN
     IF Length(Name)=6 THEN Depth:=1 ELSE Depth:=Ord(Name[7])-AscOfs;
     SLauf:=SectionStack; Erg:=MomSectionHandle;
     WHILE (Depth>0) AND (Erg<>-2) DO
      BEGIN
       IF SLauf=Nil THEN Erg:=-2
       ELSE
        BEGIN
         Erg:=SLauf^.Handle;
         SLauf:=SLauf^.Next;
        END;
       Dec(Depth);
      END;
     IF Erg=-2 THEN
      BEGIN
       WrError(1484); IdentifySection:=False;
      END
     ELSE IdentifySection:=True;
    END
   ELSE IF Name=GetSectionName(MomSectionHandle) THEN
    BEGIN
     Erg:=MomSectionHandle; IdentifySection:=True;
    END
   ELSE
    BEGIN
     SLauf:=SectionStack;
     WHILE (SLauf<>Nil) AND (GetSectionName(SLauf^.Handle)<>Name) DO
      SLauf:=SLauf^.Next;
     IF SLauf=Nil THEN
      BEGIN
       WrError(1484); IdentifySection:=False;
      END
     ELSE
      BEGIN
       Erg:=SLauf^.Handle; IdentifySection:=True;
      END
    END;
END;

        FUNCTION GetSymSection(VAR Name:String; VAR Erg:LongInt):Boolean;
VAR
   Part:String;
   q:Integer;
BEGIN
   IF Name[Length(Name)]<>']' THEN
    BEGIN
     GetSymSection:=True; Erg:=-2; Exit;
    END;

   q:=RQuotPos(Name,'[');
   IF q<2 THEN
    BEGIN
     WrXError(1020,Name); GetSymSection:=False; Exit;
    END;

   Part:=Copy(Name,q+1,Length(Name)-q-1);
   Name:=Copy(Name,1,q-1);

   GetSymSection:=IdentifySection(Part,Erg);
END;

        FUNCTION ConstIntVal(Asc:String; Typ:IntType; VAR Ok:Boolean):LongInt;
VAR
   Stelle,Base:Byte;
   Wert:LongInt;
   NegFlag:Boolean;
   h:ShortInt;
   ActMode:TConstMode;
   Found:Boolean;
BEGIN
   Ok:=False; Wert:=0; ConstIntVal:=-1;
   IF Asc='' THEN
    BEGIN
     ConstIntVal:=0; Ok:=True; Exit;
    END

   { ASCII herausfiltern }

   ELSE IF Asc[1]='''' THEN
    BEGIN
     IF Asc[Length(Asc)]<>'''' THEN Exit;
     Asc:=Copy(Asc,2,Length(Asc)-2);  ReplaceBkSlashes(Asc);
     FOR Stelle:=1 TO Length(Asc) DO
      Wert:=Wert SHL 8+Ord(CharTransTable[Asc[Stelle]]);
     NegFlag:=False;
    END

   { Zahlenkonstante }

   ELSE
    BEGIN
     { Vorzeichen }

     IF Asc[1]='+' THEN Delete(Asc,1,1);
     NegFlag:=Asc[1]='-';
     IF NegFlag THEN Delete(Asc,1,1);

     IF RelaxedMode THEN
      BEGIN
       Found:=False;

       IF (Length(Asc)>=2) AND (Asc[1]='0') AND (UpCase(Asc[2])='X') THEN
        BEGIN
         ActMode:=ConstModeC; Found:=True;
        END;
       IF (NOT Found) AND (Length(Asc)>=2) THEN
        BEGIN
         Stelle:=0;
         REPEAT
          IF Asc[1]=BaseIDs[Stelle] THEN
           BEGIN
            ActMode:=ConstModeMoto; Found:=True;
           END;
          Inc(Stelle);
         UNTIL (Stelle=3) OR (Found);
        END;
       IF (NOT Found) AND (Length(Asc)>=2) AND (Asc[1]>='0') AND (Asc[1]<='9') THEN
        BEGIN
         Stelle:=0;
         REPEAT
          IF UpCase(Asc[Length(Asc)])=BaseLetters[Stelle] THEN
           BEGIN
            ActMode:=ConstModeIntel; Found:=True;
           END;
          Inc(Stelle);
         UNTIL (Stelle=3) OR (Found);
        END;
       IF NOT Found THEN ActMode:=ConstModeC;
      END
     ELSE ActMode:=ConstMode;

     { Zahlensystem ermitteln/prÅfen }

     Base:=10;
     Stelle:=0;
     CASE ActMode OF
     ConstModeIntel:
      REPEAT
       IF UpCase(Asc[Length(Asc)])=BaseLetters[Stelle] THEN
        BEGIN
         Base:=BaseVals[Stelle]; Dec(Byte(Asc[0]));
         Stelle:=2;
        END;
       Inc(Stelle)
      UNTIL Stelle=3;
     ConstModeMoto:
      REPEAT
       IF Asc[1]=BaseIds[Stelle] THEN
        BEGIN
         Base:=BaseVals[Stelle]; Delete(Asc,1,1);
         Stelle:=2;
        END;
       Inc(Stelle);
      UNTIL Stelle=3;
     ConstModeC:
      IF Asc='0' THEN
       BEGIN
        OK:=True; ConstIntVal:=0; Exit;
       END
      ELSE IF Asc[1]<>'0' THEN Base:=10
      ELSE IF Length(Asc)<2 THEN Exit
      ELSE
       BEGIN
        Delete(Asc,1,1);
        CASE UpCase(Asc[1]) OF
        'X':BEGIN
             Delete(Asc,1,1); Base:=16;
            END;
        'B':BEGIN
             Delete(Asc,1,1); Base:=2;
            END;
        ELSE Base:=8;
        END;
        IF Length(Asc)<1 THEN Exit;
       END;
     END;

     IF Asc='' THEN Exit;

     IF ActMode=ConstModeIntel THEN
      IF (Asc[1]<'0') OR (Asc[1]>'9') THEN Exit;

     FOR Stelle:=1 TO Length(Asc) DO
      BEGIN
       h:=Pos(UpCase(Asc[Stelle]),DigitVals)-1;
       IF (h=-1) OR (h>=Base) THEN Exit;
       Wert:=Wert*Base+h;
      END;
    END;

   IF NegFlag THEN Wert:=-Wert;

   OK:=RangeCheck(Wert,Typ);
   IF OK THEN ConstIntVal:=Wert
   ELSE IF HardRanges THEN WrError(1320)
   ELSE
    BEGIN
     OK:=True; ConstIntVal:=Wert AND IntMasks[Typ];
     WrError(260);
    END;
END;

        FUNCTION ConstFloatVal(Asc:String; Typ:FloatType; VAR Ok:Boolean):Extended;
VAR
   err:ValErgType;
   Erg:Extended;
BEGIN
   Val(Asc,Erg,err);
   Ok:=False; IF err<>0 THEN Exit;

   Ok:=FloatRangeCheck(Erg,Typ);
   IF Ok THEN ConstFloatVal:=Erg;
END;

        FUNCTION ConstStringVal(Asc:String; VAR OK:Boolean):String;
VAR
   tmp,Part:String;
   Num,z:Byte;
   err:ValErgType;
   t:TempResult;
BEGIN
   OK:=False;

   IF (Length(Asc)<2) OR (Asc[1]<>'"') OR (Asc[Length(Asc)]<>'"') THEN Exit;

   Asc:=Copy(Asc,2,Length(Asc)-2); tmp:='';

   WHILE Asc<>'' DO
    BEGIN
     z:=Pos('\',Asc); IF z=0 THEN z:=Length(Asc)+1;
     IF Pos('"',Copy(Asc,1,z-1))<>0 THEN Exit;
     tmp:=tmp+Copy(Asc,1,z-1); Delete(Asc,1,z-1);
     IF (Asc<>'') AND (Asc[1]='\') THEN
      BEGIN
       IF Length(Asc)<2 THEN Exit;
       CASE UpCase(Asc[2]) OF
       '''','\',
       '"':BEGIN
            tmp:=tmp+Asc[2]; z:=2;
           END;
       'H':BEGIN
            tmp:=tmp+''''; z:=2;
           END;
       'I':BEGIN
            tmp:=tmp+'"'; z:=2;
           END;
       'B':BEGIN
            tmp:=tmp+Char_BS; z:=2;
           END;
       'A':BEGIN
            tmp:=tmp+Char_BEL; z:=2;
           END;
       'E':BEGIN
            tmp:=tmp+Char_ESC; z:=2;
           END;
       'T':BEGIN
            tmp:=tmp+Char_HT; z:=2;
           END;
       'N':BEGIN
            tmp:=tmp+Char_LF; z:=2;
           END;
       'R':BEGIN
            tmp:=tmp+Char_CR; z:=2;
           END;
       '0'..'9':BEGIN
                 z:=2;
                 WHILE (Length(Asc)>z) AND (z<4) AND (Asc[z+1] IN ['0'..'9']) DO Inc(z);
                 Val(Copy(Asc,2,z-1),num,err);
                 IF err<>0 THEN Exit;
                 tmp:=tmp+Chr(Lo(num));
                 IF num=0 THEN WrError(240);
                END;
       '{':BEGIN
            z:=QuotPos(Asc,'}'); IF z>Length(Asc) THEN Exit;
            FirstPassUnknown:=False;
            Part:=Copy(Asc,3,z-3); KillBlanks(Part);
            EvalExpression(Part,t);
            IF FirstPassUnknown THEN WrError(1820)
            ELSE
             CASE t.Typ OF
             TempInt    : tmp:=tmp+HexString(t.Int,0);
             TempFloat  : tmp:=tmp+FloatString(t.Float);
             TempString : tmp:=tmp+t.Ascii;
             ELSE Exit;
             END;
           END;
       ELSE
        BEGIN
         WrError(1135); Exit;
        END;
       END;
       Delete(Asc,1,z);
      END;
    END;

   ConstStringVal:=tmp;
   OK:=True;
END;


        FUNCTION FindLocNode(Name:String; SearchType:TempType):SymbolPtr;
        FORWARD;

        FUNCTION FindNode(Name:String; SearchType:TempType):SymbolPtr;
        FORWARD;


        PROCEDURE EvalExpression(Asc:String; VAR Erg:TempResult);
LABEL
   FreeVars;
CONST
   OpCnt=23;
   Operators:ARRAY[0..OpCnt] OF Operator=
              { Dummynulloperator }
             ((Id:' ' ; Dyadic:False; Priority: 0; MayInt:False; MayFloat:False; MayString:False; Present:False),
              { Einerkomplement }
              (Id:'~' ; Dyadic:False; Priority: 1; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { Linksschieben }
              (Id:'<<'; Dyadic:True ; Priority: 3; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { Rechtsschieben }
              (Id:'>>'; Dyadic:True ; Priority: 3; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { Bitspiegelung }
              (Id:'><'; Dyadic:True ; Priority: 4; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { binÑres AND }
              (Id:'&' ; Dyadic:True ; Priority: 5; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { binÑres OR }
              (Id:'|' ; Dyadic:True ; Priority: 6; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { binÑres EXOR }
              (Id:'!' ; Dyadic:True ; Priority: 7; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { allg. Potenz }
              (Id:'^' ; Dyadic:True ; Priority: 8; MayInt:True ; MayFloat:True ; MayString:False; Present:False),
              { Produkt }
              (Id:'*' ; Dyadic:True ; Priority:11; MayInt:True ; MayFloat:True ; MayString:False; Present:False),
              { Quotient }
              (Id:'/' ; Dyadic:True ; Priority:11; MayInt:True ; MayFloat:True ; MayString:False; Present:False),
              { Modulodivision }
              (Id:'#' ; Dyadic:True ; Priority:11; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { Summe }
              (Id:'+' ; Dyadic:True ; Priority:13; MayInt:True ; MayFloat:True ; MayString:True ; Present:False),
              { Differenz }
              (Id:'-' ; Dyadic:True ; Priority:13; MayInt:True ; MayFloat:True ; MayString:False; Present:False),
              { logisches NOT }
              (Id:'~~'; Dyadic:False; Priority: 2; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { logisches AND }
              (Id:'&&'; Dyadic:True ; Priority:15; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { logisches OR }
              (Id:'||'; Dyadic:True ; Priority:16; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { logisches EXOR }
              (Id:'!!'; Dyadic:True ; Priority:17; MayInt:True ; MayFloat:False; MayString:False; Present:False),
              { Gleichheit }
              (Id:'=' ; Dyadic:True ; Priority:23; MayInt:True ; MayFloat:True ; MayString:True ; Present:False),
              { Grî·er als }
              (Id:'>' ; Dyadic:True ; Priority:23; MayInt:True ; MayFloat:True ; MayString:True ; Present:False),
              { Kleiner als }
              (Id:'<' ; Dyadic:True ; Priority:23; MayInt:True ; MayFloat:True ; MayString:True ; Present:False),
              { Kleiner oder gleich }
              (Id:'<='; Dyadic:True ; Priority:23; MayInt:True ; MayFloat:True ; MayString:True ; Present:False),
              { Grî·er oder gleich }
              (Id:'>='; Dyadic:True ; Priority:23; MayInt:True ; MayFloat:True ; MayString:True ; Present:False),
              { Ungleichheit }
              (Id:'<>'; Dyadic:True ; Priority:23; MayInt:True ; MayFloat:True ; MayString:True ; Present:False));
VAR
   Ok,FFound:Boolean;
   LVal,RVal:^TempResult;
   z1,z2:Integer;
   LKlamm,RKlamm,WKlamm:Integer;
   OpMax,LocOpMax,OpPos,OpLen:Integer;
   OpFnd,InHyp,InQuot:Boolean;
   HVal:LongInt;
   FVal:Extended;
   Ptr:SymbolPtr;
   ValFunc:PFunction;
   stemp,ftemp:^String;
   DummyPtr:StringPtr;

        PROCEDURE ChgFloat(VAR T:TempResult);
BEGIN
   IF T.Typ<>TempInt THEN Exit;
   T.Typ:=TempFloat; T.Float:=T.Int;
END;

{$IFDEF SPEEDUP}
        FUNCTION MemCmp(VAR Src,Dest; Cnt:Word):Boolean;
        Inline($8c/$da/                 { mov    dx,ds }
               $59/$5f/$07/$5e/$1f/     { pop    cx/di/es/si/ds }
               $fc/                     { cld }
               $28/$c0/                 { sub    al,al }
               $f3/$a6/                 { repe   cmpsb }
               $75/$02/                 { jne    $+4 }
               $fe/$c0/                 { inc    al }
               $8e/$da);                { mov    ds,dx }
{$ENDIF}

BEGIN
   ChkStack;

   New(LVal); New(RVal); New(ftemp); New(stemp);

   stemp^:=Asc; KillBlanks(Asc);
   IF MakeDebug THEN WriteLn(Debug,'Parse ',Asc);

   { Annahme Fehler }

   Erg.Typ:=TempNone;

   { ProgrammzÑhler ? }

   IF NLS_StrCaseCmp(Asc,PCSymbol)=0 THEN
    BEGIN
     Erg.Typ:=TempInt;
     Erg.Int:=EProgCounter;
     Goto FreeVars;
    END;

   { Konstanten ? }

   Erg.Int:=ConstIntVal(Asc,Int32,Ok);
   IF Ok THEN
    BEGIN
     Erg.Typ:=TempInt; Goto FreeVars;
    END;

   Erg.Float:=ConstFloatVal(Asc,Float80,Ok);
   IF Ok THEN
    BEGIN
     Erg.Typ:=TempFloat; Goto FreeVars;
    END;

   Erg.Ascii:=ConstStringVal(Asc,OK);
   IF OK THEN
    BEGIN
     Erg.Typ:=TempString; Goto FreeVars;
    END;

   InternSymbol(Asc,Erg); IF Erg.Typ<>TempNone THEN Goto FreeVars;

   { ZÑhler initialisieren }

   LocOpMax:=0; OpMax:=0; LKlamm:=0; RKlamm:=0; WKlamm:=0;
   InHyp:=False; InQuot:=False;
   FOR z1:=1 TO OpCnt DO
    WITH Operators[z1] DO
     Present:=Pos(Id,Asc)<>0;

   { nach Operator hîchster Rangstufe au·erhalb Klammern suchen }

   z1:=1;
   WHILE z1<=Length(Asc) DO
    BEGIN
     CASE Asc[z1] OF
     '(':IF NOT (InHyp OR InQuot) THEN Inc(LKlamm);
     ')':IF NOT (InHyp OR InQuot) THEN Inc(RKlamm);
     '{':IF NOT (InHyp OR InQuot) THEN Inc(WKlamm);
     '}':IF NOT (InHyp OR InQuot) THEN Dec(WKlamm);
     '"':IF NOT (InHyp) THEN InQuot:=NOT InQuot;
     '''':IF NOT (InQuot) THEN InHyp:=NOT InHyp;
     ELSE IF (LKlamm=RKlamm) AND (WKlamm=0) AND (NOT InHyp) AND (NOT InQuot) THEN
      BEGIN
       OpFnd:=False; OpLen:=0; LocOpMax:=0;
       FOR z2:=1 TO OpCnt DO
        WITH Operators[z2] DO
         IF Present THEN
{$IFDEF SPEEDUP}
         IF MemCmp(Asc[z1],Id[1],Length(Id)) THEN
{$ELSE}
         IF Copy(Asc,z1,Length(Id))=Id THEN
{$ENDIF}
         IF Length(Id)>=OpLen THEN
          BEGIN
           OpFnd:=True; OpLen:=Length(Id); LocOpMax:=z2;
           IF Operators[LocOpMax].Priority>=Operators[OpMax].Priority THEN
            BEGIN
             OpMax:=LocOpMax; OpPos:=z1;
            END;
          END;
       IF OpFnd THEN Inc(z1,Length(Operators[LocOpMax].Id)-1);
      END;
     END;
     Inc(z1);
    END;

   { Klammerfehler ? }

   IF LKlamm<>RKlamm THEN
    BEGIN
     WrXError(1300,Asc); Goto FreeVars;
    END;

   { Operator gefunden ? }

   IF OpMax<>0 THEN
   WITH Operators[OpMax] DO
    BEGIN
     { Minuszeichen sowohl mit einem als auch 2 Operanden }

     IF Id='-' THEN Dyadic:=OpPos>1;

     { Operandenzahl prÅfen }

     IF (Dyadic) XOR (OpPos<>1) THEN
      BEGIN
       WrError(1110); Goto FreeVars;
      END;
     IF OpPos=Length(Asc) THEN
      BEGIN
       WrError(1110); Goto FreeVars;
      END;

     { TeilausdrÅcke rekursiv auswerten }

     IF Dyadic THEN EvalExpression(Copy(Asc,1,OpPos-1),LVal^)
     ELSE
      BEGIN
       LVal^.Typ:=TempInt; LVal^.Int:=0;
      END;
     EvalExpression(Copy(Asc,OpPos+Length(Id),Length(Asc)-OpPos-Length(Id)+1),RVal^);

     { Abbruch, falls dabei Fehler }

     IF (LVal^.Typ=TempNone) OR (RVal^.Typ=TempNone) THEN Goto FreeVars;

     { TypÅberprÅfung }

     IF (dyadic) AND (LVal^.Typ<>RVal^.Typ) THEN
      BEGIN
       IF (LVal^.Typ=TempString) OR (RVal^.Typ=TempString) THEN
        BEGIN
         WrError(1135); Goto FreeVars;
        END;
       IF LVal^.Typ=TempInt THEN ChgFloat(LVal^);
       IF RVal^.Typ=TempInt THEN ChgFloat(RVal^);
      END;

     CASE RVal^.Typ OF
     TempInt:  IF NOT MayInt THEN
                IF NOT MayFloat THEN
                 BEGIN
                  WrError(1135); Goto FreeVars;
                 END
                ELSE
                 BEGIN
                  ChgFloat(RVal^); IF dyadic THEN ChgFloat(LVal^);
                 END;
     TempFloat: IF NOT MayFloat THEN
                 BEGIN
                  WrError(1135); Goto FreeVars;
                 END;
     TempString:IF NOT MayString THEN
                 BEGIN
                  WrError(1135); Goto FreeVars;
                 END;
     END;

     { Operanden abarbeiten }

     CASE OpMax OF
     1:BEGIN                                            { ~ }
        Erg.Typ:=TempInt;
        Erg.Int:=NOT RVal^.Int;
       END;
     2:BEGIN                                            { << }
        Erg.Typ:=TempInt;
        Erg.Int:=LVal^.Int SHL RVal^.Int;
       END;
     3:BEGIN                                            { >> }
        Erg.Typ:=TempInt;
        Erg.Int:=LVal^.Int SHR RVal^.Int;
       END;
     4:BEGIN                                            { >< }
        Erg.Typ:=TempInt;
        IF (RVal^.Int<1) OR (RVal^.Int>32) THEN WrError(1320)
        ELSE
         BEGIN
          Erg.Int:=(LVal^.Int SHR RVal^.Int) SHL RVal^.Int;
          Dec(RVal^.Int);
          FOR z1:=0 TO RVal^.Int DO
           BEGIN
            IF LVal^.Int AND (1 SHL (RVal^.Int-z1))<>0 THEN
            Inc(Erg.Int,1 SHL z1);
           END;
         END;
       END;
     5:BEGIN
        Erg.Typ:=TempInt;                               { & }
        Erg.Int:=LVal^.Int AND RVal^.Int;
       END;
     6:BEGIN
        Erg.Typ:=TempInt;                               { | }
        Erg.Int:=LVal^.Int OR RVal^.Int;
       END;
     7:BEGIN
        Erg.Typ:=TempInt;                               { ! }
        Erg.Int:=LVal^.Int XOR RVal^.Int;
       END;
     8:BEGIN                                            { ^ }
        Erg.Typ:=LVal^.Typ;
        CASE Erg.Typ OF
        TempInt:
        IF RVal^.Int<0 THEN Erg.Int:=0
        ELSE
         BEGIN
          Erg.Int:=1;
          WHILE RVal^.Int>0 DO
           BEGIN
            IF Odd(RVal^.Int) THEN Erg.Int:=Erg.Int*LVal^.Int;
            RVal^.Int:=RVal^.Int SHR 1;
            IF RVal^.Int<>0 THEN LVal^.Int:=Sqr(LVal^.Int);
           END;
         END;
        TempFloat:
         BEGIN
          IF RVal^.Float=0.0 THEN Erg.Float:=1.0
          ELSE IF LVal^.Float=0.0 THEN Erg.Float:=0.0
          ELSE IF LVal^.Float>0 THEN Erg.Float:=Exp(RVal^.Float*Ln(LVal^.Float))
          ELSE IF (Abs(RVal^.Float)<=MaxLongInt) AND (Round(RVal^.Float)=RVal^.Float) THEN
           BEGIN
            HVal:=Round(RVal^.Float);
            IF HVal<0 THEN
             BEGIN
              LVal^.Float:=1/LVal^.Float; HVal:=-HVal;
             END;
            Erg.Float:=1.0;
            WHILE HVal>0 DO
             BEGIN
              IF Odd(HVal) THEN Erg.Float:=Erg.Float*LVal^.Float;
              LVal^.Float:=Sqr(LVal^.Float); HVal:=HVal SHR 1;
             END
           END
          ELSE
           BEGIN
            WrError(1890); Erg.Typ:=TempNone;
           END;
         END;
        END;
       END;
     9:BEGIN                                            { * }
        Erg.Typ:=LVal^.Typ;
        CASE Erg.Typ OF
        TempInt:Erg.Int:=LVal^.Int*RVal^.Int;
        TempFloat:Erg.Float:=LVal^.Float*RVal^.Float;
        END;
       END;
     10:BEGIN                                           { / }
         CASE LVal^.Typ OF
         TempInt:
          IF RVal^.Int=0 THEN WrError(1310)
          ELSE
           BEGIN
            Erg.Typ:=TempInt;
            Erg.Int:=LVal^.Int DIV RVal^.Int;
           END;
         TempFloat:
          IF RVal^.Float=0.0 THEN WrError(1310)
          ELSE
           BEGIN
            Erg.Typ:=TempFloat;
            Erg.Float:=LVal^.Float/RVal^.Float;
           END;
         END;
        END;
     11:BEGIN                                           { # }
         IF RVal^.Int=0 THEN WrError(1310)
         ELSE
          BEGIN
           Erg.Typ:=TempInt;
           Erg.Int:=LVal^.Int MOD RVal^.Int;
          END;
        END;
     12:BEGIN                                           { + }
         Erg.Typ:=LVal^.Typ;
         CASE Erg.Typ OF
         TempInt   :Erg.Int:=LVal^.Int+RVal^.Int;
         TempFloat :Erg.Float:=LVal^.Float+RVal^.Float;
         TempString:Erg.Ascii:=LVal^.Ascii+RVal^.Ascii;
         END;
        END;
     13:IF Dyadic THEN                                  { - }
         BEGIN
          Erg.Typ:=LVal^.Typ;
          CASE Erg.Typ OF
          TempInt  :Erg.Int:=LVal^.Int-RVal^.Int;
          TempFloat:Erg.Float:=LVal^.Float-RVal^.Float;
          END;
         END
        ELSE
         BEGIN
          Erg.Typ:=RVal^.Typ;
          CASE Erg.Typ OF
          TempInt  :Erg.Int:=-RVal^.Int;
          TempFloat:Erg.Float:=-RVal^.Float;
          END;
         END;
     14:BEGIN                                           { ~~ }
         Erg.Typ:=TempInt;
         Erg.Int:=Ord(RVal^.Int=0);
        END;
     15:BEGIN                                           { && }
         Erg.Typ:=TempInt;
         Erg.Int:=Ord((LVal^.Int<>0) AND (RVal^.Int<>0))
        END;
     16:BEGIN                                           { || }
         Erg.Typ:=TempInt;
         Erg.Int:=Ord((LVal^.Int<>0) OR (RVal^.Int<>0))
        END;
     17:BEGIN                                           { !! }
         Erg.Typ:=TempInt;
         Erg.Int:=Ord((LVal^.Int<>0) XOR (RVal^.Int<>0))
        END;
     18:BEGIN                                           { = }
         Erg.Typ:=TempInt;
         CASE LVal^.Typ OF
         TempInt   :Erg.Int:=Ord(LVal^.Int  =RVal^.Int  );
         TempFloat :Erg.Int:=Ord(LVal^.Float=RVal^.Float);
         TempString:Erg.Int:=Ord(LVal^.Ascii=RVal^.Ascii);
         END;
        END;
     19:BEGIN                                           { > }
         Erg.Typ:=TempInt;
         CASE LVal^.Typ OF
         TempInt   :Erg.Int:=Ord(LVal^.Int  >RVal^.Int  );
         TempFloat :Erg.Int:=Ord(LVal^.Float>RVal^.Float);
         TempString:Erg.Int:=Ord(LVal^.Ascii>RVal^.Ascii);
         END;
        END;
     20:BEGIN                                           { < }
         Erg.Typ:=TempInt;
         CASE LVal^.Typ OF
         TempInt   :Erg.Int:=Ord(LVal^.Int  <RVal^.Int  );
         TempFloat :Erg.Int:=Ord(LVal^.Float<RVal^.Float);
         TempString:Erg.Int:=Ord(LVal^.Ascii<RVal^.Ascii);
         END;
        END;
     21:BEGIN                                           { <= }
         Erg.Typ:=TempInt;
         CASE LVal^.Typ OF
         TempInt   :Erg.Int:=Ord(LVal^.Int  <=RVal^.Int  );
         TempFloat :Erg.Int:=Ord(LVal^.Float<=RVal^.Float);
         TempString:Erg.Int:=Ord(LVal^.Ascii<=RVal^.Ascii);
         END;
        END;
     22:BEGIN                                           { >= }
         Erg.Typ:=TempInt;
         CASE LVal^.Typ OF
         TempInt   :Erg.Int:=Ord(LVal^.Int  >=RVal^.Int  );
         TempFloat :Erg.Int:=Ord(LVal^.Float>=RVal^.Float);
         TempString:Erg.Int:=Ord(LVal^.Ascii>=RVal^.Ascii);
         END;
        END;
     23:BEGIN                                           { <> }
         Erg.Typ:=TempInt;
         CASE LVal^.Typ OF
         TempInt   :Erg.Int:=Ord(LVal^.Int  <>RVal^.Int  );
         TempFloat :Erg.Int:=Ord(LVal^.Float<>RVal^.Float);
         TempString:Erg.Int:=Ord(LVal^.Ascii<>RVal^.Ascii);
         END;
        END;
     END;
     Goto FreeVars;
    END;

   { kein Operator gefunden: Klammerausdruck ? }

   IF LKlamm<>0 THEN
   WITH LVal^ DO
    BEGIN

     { erste Klammer suchen, Funktionsnamen abtrennen }

     OpPos:=Pos('(',Asc);

     { Funktionsnamen abschneiden }

     ftemp^:=Copy(Asc,1,OpPos-1);
     Asc:=Copy(Asc,OpPos+1,Length(Asc)-OpPos-1);

     { Nullfunktion: nur Argument }

     IF ftemp^='' THEN
      BEGIN
       EvalExpression(Asc,LVal^);
       Erg:=LVal^; Goto FreeVars;
      END;

     { selbstdefinierte Funktion ? }

     ValFunc:=FindFunction(ftemp^);
     IF ValFunc<>Nil THEN
      BEGIN
       ftemp^:=ValFunc^.Definition^;
       FOR z1:=1 TO ValFunc^.ArguCnt DO
        BEGIN
         IF Asc='' THEN
          BEGIN
           WrError(1490); Goto FreeVars;
          END;
         OpPos:=QuotPos(Asc,',');
         EvalExpression(Copy(Asc,1,OpPos-1),LVal^);
         Delete(Asc,1,OpPos);
         CASE LVal^.Typ OF
         TempInt:Str(LVal^.Int,stemp^);
         TempFloat:BEGIN
                    Str(LVal^.Float,stemp^); KillBlanks(stemp^);
                   END;
         TempString:stemp^:='"'+LVal^.Ascii+'"';
         TempNone:Goto FreeVars;
         END;
         stemp^:='('+stemp^+')';
         ExpandLine(stemp^,z1-1,ftemp^);
        END;
       IF Asc<>'' THEN
        BEGIN
         WrError(1490); Goto FreeVars;
        END;
       EvalExpression(ftemp^,Erg);
       Goto FreeVars;
      END;

     { hier einmal umwandeln ist effizienter...}

     NLS_UpString(ftemp^);

     { symbolbezogene Funktionen }

     IF ftemp^='SYMTYPE' THEN
      BEGIN
       Erg.Typ:=TempInt;
       IF FindRegDef(Asc,DummyPtr) THEN Erg.Int:=$80
       ELSE Erg.Int:=GetSymbolType(Asc);
       Goto FreeVars;
      END;

     { Unterausdruck auswerten (interne Funktionen nur mit einem Argument }

     EvalExpression(Asc,LVal^);

     { Abbruch bei Fehler }

     IF Typ=TempNone THEN Goto FreeVars;

     { Funktionen fÅr Stringargumente }

     IF Typ=TempString THEN
      BEGIN
       { in Gro·buchstaben wandeln ? }

       IF ftemp^='UPSTRING' THEN
        BEGIN
         Erg.Typ:=TempString; Erg.Ascii:=LVal^.Ascii;
         FOR z1:=1 TO Length(Erg.Ascii) DO
          Erg.Ascii[z1]:=UpCase(Erg.Ascii[z1]);
        END

       { in Kleinbuchstaben wandeln ? }

       ELSE IF ftemp^='LOWSTRING' THEN
        BEGIN
         Erg.Typ:=TempString; Erg.Ascii:=LVal^.Ascii;
         FOR z1:=1 TO Length(Erg.Ascii) DO
          Erg.Ascii[z1]:=LowCase(Erg.Ascii[z1]);
        END

       { LÑnge ? }

       ELSE IF ftemp^='STRLEN' THEN
        BEGIN
         Erg.Typ:=TempInt; Erg.Int:=Length(LVal^.Ascii);
        END

       { Parser aufrufen ? }

       ELSE IF ftemp^='VAL' THEN
        BEGIN
         EvalExpression(LVal^.Ascii,Erg);
        END

       { nix gefunden ? }

       ELSE
        BEGIN
         WrXError(1860,ftemp^); Erg.Typ:=TempNone;
        END;
      END

     { Funktionen fÅr Zahlenargumente }

     ELSE
      BEGIN
       FFound:=False; Erg.Typ:=TempNone;

       { reine Integerfunktionen }

       IF ftemp^='TOUPPER' THEN
        BEGIN
         IF LVal^.Typ<>TempInt THEN WrError(1135)
         ELSE IF (LVal^.Int<0) OR (LVal^.Int>255) THEN WrError(1320)
         ELSE
          BEGIN
           Erg.Typ:=TempInt;
           Erg.Int:=Ord(UpCase(Chr(LVal^.Int)));
          END;
         FFound:=True;
        END

       ELSE IF ftemp^='TOLOWER' THEN
        BEGIN
         IF LVal^.Typ<>TempInt THEN WrError(1135)
         ELSE IF (LVal^.Int<0) OR (LVal^.Int>255) THEN WrError(1320)
         ELSE
          BEGIN
           Erg.Typ:=TempInt;
           Erg.Int:=Ord(LowCase(Chr(LVal^.Int)));
          END;
         FFound:=True;
        END

       ELSE IF ftemp^='BITCNT' THEN
        BEGIN
         IF LVal^.Typ<>TempInt THEN WrError(1135)
         ELSE
          BEGIN
           Erg.Typ:=TempInt;
           Erg.Int:=0;
           FOR z1:=0 TO 31 DO
            BEGIN
             Erg.Int:=Erg.Int+(LVal^.Int AND 1);
             LVal^.Int:=LVal^.Int SHR 1;
            END;
          END;
         FFound:=True;
        END

       ELSE IF ftemp^='FIRSTBIT' THEN
        BEGIN
         IF LVal^.Typ<>TempInt THEN WrError(1135)
         ELSE
          BEGIN
           Erg.Typ:=TempInt;
           Erg.Int:=0;
           REPEAT
            IF NOT Odd(LVal^.Int) THEN Inc(Erg.Int);
            LVal^.Int:=LVal^.Int SHR 1;
           UNTIL (Erg.Int=32) OR (Odd(LVal^.Int));
           IF Erg.Int=32 THEN Erg.Int:=-1;
          END;
         FFound:=True;
        END

       ELSE IF ftemp^='LASTBIT' THEN
        BEGIN
         IF LVal^.Typ<>TempInt THEN WrError(1135)
         ELSE
          BEGIN
           Erg.Typ:=TempInt;
           Erg.Int:=-1;
           FOR z1:=0 TO 31 DO
            BEGIN
             IF Odd(LVal^.Int) THEN Erg.Int:=z1;
             LVal^.Int:=LVal^.Int SHR 1;
            END;
          END;
         FFound:=True;
        END

       ELSE IF ftemp^='BITPOS' THEN
        BEGIN
         IF LVal^.Typ<>TempInt THEN WrError(1135)
         ELSE
          BEGIN
           Erg.Typ:=TempInt;
           IF NOT SingleBit(LVal^.Int,Erg.Int) THEN
            BEGIN
             Erg.Int:=-1;
             WrError(1540);
            END;
          END;
         FFound:=True;
        END

       { variable Integer/Float-Fuktionen }

       ELSE IF ftemp^='ABS' THEN
        BEGIN
         Erg.Typ:=LVal^.Typ;
         CASE LVal^.Typ OF
         TempInt:Erg.Int:=Abs(LVal^.Int);
         TempFloat:Erg.Float:=Abs(LVal^.Float);
         END;
         FFound:=True;
        END

       ELSE IF ftemp^='SGN' THEN
        BEGIN
         Erg.Typ:=TempInt;
         CASE LVal^.Typ OF
         TempInt:IF LVal^.Int<0 THEN Erg.Int:=-1
                 ELSE IF LVal^.Int>0 THEN Erg.Int:=1
                 ELSE Erg.Int:=0;
         TempFloat:IF LVal^.Float<0 THEN Erg.Int:=-1
                   ELSE IF LVal^.Float>0 THEN Erg.Int:=1
                   ELSE Erg.Int:=0;
         END;
         FFound:=True;
        END;

       { Funktionen Float und damit auch Int }

       IF NOT FFound THEN
        BEGIN
         { Typkonvertierung }

         ChgFloat(LVal^); Erg.Typ:=TempFloat;

         { Integerwandlung }

         IF ftemp^='INT' THEN
          BEGIN
           IF Abs(Float)>MaxLongInt THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1320);
            END
           ELSE
            BEGIN
             Erg.Typ:=TempInt; Erg.Int:=Trunc(LVal^.Float);
            END;
          END

         { Quadratwurzel }

         ELSE IF ftemp^='SQRT' THEN
          BEGIN
           IF Float<0 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE Erg.Float:=Sqrt(Float);
          END

         { trigonometrische Funktionen }

         ELSE IF ftemp^='SIN' THEN Erg.Float:=sin(Float)
         ELSE IF ftemp^='COS' THEN Erg.Float:=cos(Float)
         ELSE IF ftemp^='TAN' THEN
          BEGIN
           FVal:=cos(Float);
           IF FVal=0.0 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE Erg.Float:=sin(Float)/FVal;
          END
         ELSE IF ftemp^='COT' THEN
          BEGIN
           FVal:=sin(Float);
           IF FVal=0.0 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE Erg.Float:=cos(Float)/FVal;
          END

         { inverse trigonometrische Funktionen }

         ELSE IF ftemp^='ASIN' THEN
          BEGIN
           IF Abs(Float)>1 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE IF Abs(Float)=1 THEN Erg.Float:=Float/Abs(Float)*Pi/2
           ELSE Erg.Float:=ArcTan(Float/(Sqrt(1-Sqr(Float))));
          END
         ELSE IF ftemp^='ACOS' THEN
          BEGIN
           IF Abs(Float)>1 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE IF Float=1 THEN Erg.Float:=0
           ELSE IF Float=-1 THEN Erg.Float:=Pi
           ELSE Erg.Float:=Pi/2-ArcTan(Float/(Sqrt(1-Sqr(Float))));
          END
         ELSE IF ftemp^='ATAN' THEN Erg.Float:=ArcTan(Float)
         ELSE IF ftemp^='ACOT' THEN Erg.Float:=Pi/2-ArcTan(Float)

         { exponentielle & hyperbolische Funktionen }

         ELSE IF ftemp^='EXP' THEN
          BEGIN
           IF Float>11354 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE Erg.Float:=Exp(Float);
          END
         ELSE IF ftemp^='ALOG' THEN
          BEGIN
           IF Float>4931 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE Erg.Float:=Exp(Float*Ln(10));
          END
         ELSE IF ftemp^='ALD' THEN
          BEGIN
           IF Float>16382 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE Erg.Float:=Exp(Float*Ln(2));
          END
         ELSE IF ftemp^='SINH' THEN
          BEGIN
           IF Float>11354 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE Erg.Float:=0.5*(Exp(Float)-Exp(-Float));
          END
         ELSE IF ftemp^='COSH' THEN
          BEGIN
           IF Float>11354 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE Erg.Float:=0.5*(Exp(Float)+Exp(-Float));
          END
         ELSE IF ftemp^='TANH' THEN
          BEGIN
           IF Float>11354 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE
            BEGIN
             FVal:=Exp(2*Float); Erg.Float:=(FVal-1)/(FVal+1);
            END
          END
         ELSE IF ftemp^='COTH' THEN
          BEGIN
           IF Float>11354 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE IF Float=0 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE
            BEGIN
             FVal:=Exp(2*Float); Erg.Float:=(FVal+1)/(FVal-1);
            END
          END

         { logarithmische & inverse hyperbolische Funktionen }

         ELSE IF ftemp^='LN' THEN
          BEGIN
           IF Float<=0 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE Erg.Float:=Ln(Float);
          END
         ELSE IF ftemp^='LOG' THEN
          BEGIN
           IF Float<=0 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE Erg.Float:=Ln(Float)/Ln(10);
          END
         ELSE IF ftemp^='LD' THEN
          BEGIN
           IF Float<=0 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1870);
            END
           ELSE Erg.Float:=Ln(Float)/Ln(2);
          END
         ELSE IF ftemp^='ASINH' THEN Erg.Float:=Ln(Float+Sqrt(Sqr(Float)+1))
         ELSE IF ftemp^='ACOSH' THEN
          BEGIN
           IF Float<1 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE Erg.Float:=Ln(Float+Sqrt(Sqr(Float)-1));
          END
         ELSE IF ftemp^='ATANH' THEN
          BEGIN
           IF Abs(Float)>=1 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE Erg.Float:=0.5*Ln((1+Float)/(1-Float));
          END
         ELSE IF ftemp^='ACOTH' THEN
          BEGIN
           IF Abs(Float)<=1 THEN
            BEGIN
             Erg.Typ:=TempNone; WrError(1880);
            END
           ELSE Erg.Float:=0.5*Ln((Float+1)/(Float-1));
          END

         { nix gefunden ? }

         ELSE
          BEGIN
           WrXError(1860,ftemp^); Erg.Typ:=TempNone;
          END;
        END;
      END;
     Goto FreeVars;
    END;

   { nichts dergleichen, dann einfaches Symbol: }

   { interne Symbole ? }

   Asc:=stemp^; KillPrefBlanks(Asc); KillPostBlanks(Asc);

   IF NLS_StrCaseCmp(Asc,'MOMFILE')=0 THEN
    BEGIN
     Erg.Typ:=TempString;
     Erg.Ascii:=CurrFileName;
     Goto FreeVars;
    END;

   IF NLS_StrCaseCmp(Asc,'MOMLINE')=0 THEN
    BEGIN
     Erg.Typ:=TempInt;
     Erg.Int:=CurrLine;
     Goto FreeVars;
    END;

   IF NLS_StrCaseCmp(Asc,'MOMPASS')=0 THEN
    BEGIN
     Erg.Typ:=TempInt;
     Erg.Int:=PassNo;
     Goto FreeVars;
    END;

   IF NLS_StrCaseCmp(Asc,'MOMSECTION')=0 THEN
    BEGIN
     Erg.Typ:=TempString;
     Erg.Ascii:=GetSectionName(MomSectionHandle);
     Goto FreeVars;
    END;

   IF NLS_StrCaseCmp(Asc,'MOMSEGMENT')=0 THEN
    BEGIN
     Erg.Typ:=TempString;
     Erg.Ascii:=SegNames[ActPC];
     Goto FreeVars;
    END;

   IF NOT ExpandSymbol(Asc) THEN Goto FreeVars;

   z1:=Pos('[',Asc);
   IF z1=0 THEN OK:=ChkSymbName(Asc)
   ELSE OK:=ChkSymbName(Copy(Asc,1,z1-1));
   IF NOT OK THEN
    BEGIN
     WrXError(1020,Asc); Goto FreeVars;
    END;

   Ptr:=FindLocNode(Asc,TempInt);
   IF Ptr=Nil THEN Ptr:=FindNode(Asc,TempInt);
   IF Ptr<>Nil THEN
    BEGIN
     Erg.Typ:=TempInt; Erg.Int:=Ptr^.SymWert.IWert;
     IF Ptr^.SymType<>0 THEN TypeFlag:=TypeFlag OR (1 SHL Ptr^.SymType);
     IF (Ptr^.SymSize<>-1) AND (SizeFlag=-1) THEN SizeFlag:=Ptr^.SymSize;
     IF (NOT Ptr^.Defined) THEN
      BEGIN
       IF (Repass) THEN SymbolQuestionable:=True;
       UsesForwards:=True;
      END;
     Ptr^.Used:=True;
     Goto FreeVars;
    END;
   Ptr:=FindLocNode(Asc,TempFloat);
   IF Ptr=Nil THEN Ptr:=FindNode(Asc,TempFloat);
   IF Ptr<>Nil THEN
    BEGIN
     Erg.Typ:=TempFloat; Erg.Float:=Ptr^.SymWert.FWert;
     IF Ptr^.SymType<>0 THEN TypeFlag:=TypeFlag OR (1 SHL Ptr^.SymType);
     IF (Ptr^.SymSize<>-1) AND (SizeFlag=-1) THEN SizeFlag:=Ptr^.SymSize;
     IF (NOT Ptr^.Defined) THEN
      BEGIN
       IF (Repass) THEN SymbolQuestionable:=True;
       UsesForwards:=True;
      END;
     Ptr^.Used:=True;
     Goto FreeVars;
    END;
   Ptr:=FindLocNode(Asc,TempString);
   IF Ptr=Nil THEN Ptr:=FindNode(Asc,TempString);
   IF Ptr<>Nil THEN
    BEGIN
     Erg.Typ:=TempString; Erg.Ascii:=Ptr^.SymWert.SWert^;
     IF Ptr^.SymType<>0 THEN TypeFlag:=TypeFlag OR (1 SHL Ptr^.SymType);
     IF (Ptr^.SymSize<>-1) AND (SizeFlag=-1) THEN SizeFlag:=Ptr^.SymSize;
     IF (NOT Ptr^.Defined) THEN
      BEGIN
       IF (Repass) THEN SymbolQuestionable:=True;
       UsesForwards:=True;
      END;
     Ptr^.Used:=True;
     Goto FreeVars;
    END;

   { Symbol evtl. im ersten Pass unbekannt }

   IF PassNo<=MaxSymPass THEN
    BEGIN
     Erg.Typ:=TempInt; Erg.Int:=EProgCounter;
     Repass:=True;
     IF (MsgIfRepass) AND (PassNo>=PassNoForMessage) THEN WrXError(170,Asc);
     FirstPassUnknown:=True;
    END

   { alles war nix, Fehler }

   ELSE WrXError(1010,Asc);

FreeVars:
   Dispose(LVal); Dispose(RVal); Dispose(ftemp); Dispose(stemp);
END;


        FUNCTION EvalIntExpression(Asc:String; Typ:IntType; VAR Ok:Boolean):LongInt;
VAR
   t:TempResult;
BEGIN
   EvalIntExpression:=-1; Ok:=False;
   TypeFlag:=0; SizeFlag:=-1;
   UsesForwards:=False;
   SymbolQuestionable:=False;
   FirstPassUnknown:=False;

   EvalExpression(Asc,t);
   IF t.Typ<>TempInt THEN
    BEGIN
     IF t.Typ<>TempNone THEN WrError(1135);
     Exit;
    END;

   IF FirstPassUnknown THEN t.Int:=t.Int AND IntMasks[Typ];

   IF Typ<>Int32 THEN
    IF (t.Int<IntMins[Typ]) OR (t.Int>IntMaxs[Typ]) THEN
     IF NOT HardRanges THEN
      BEGIN
       t.Int:=t.Int AND IntMasks[Typ];
       WrError(260);
      END
     ELSE
      BEGIN
       WrError(1320); Exit;
      END;

   EvalIntExpression:=t.Int; Ok:=True;
END;

        FUNCTION EvalFloatExpression(Asc:String; Typ:FloatType; VAR Ok:Boolean):Extended;
VAR
   t:TempResult;
BEGIN
   EvalFloatExpression:=-1.0; Ok:=False;
   TypeFlag:=0; SizeFlag:=-1;
   UsesForwards:=False;
   SymbolQuestionable:=False;
   FirstPassUnknown:=False;

   EvalExpression(Asc,t);
   CASE t.Typ OF
   TempNone:
    Exit;
   TempInt:
    t.Float:=t.Int;
   TempString:
    BEGIN
     WrError(1135); Exit;
    END;
   END;

   IF NOT FloatRangeCheck(t.Float,Typ) THEN
    BEGIN
     WrError(1320); Exit;
    END;

   EvalFloatExpression:=t.Float; Ok:=True;
END;

        FUNCTION EvalStringExpression(Asc:String; VAR Ok:Boolean):String;
VAR
   t:TempResult;
BEGIN
   EvalStringExpression:=''; Ok:=False;
   TypeFlag:=0; SizeFlag:=-1;
   UsesForwards:=False;
   SymbolQuestionable:=False;
   FirstPassUnknown:=False;

   EvalExpression(Asc,t);
   IF t.Typ<>TempString THEN
    BEGIN
     IF t.Typ<>TempNone THEN WrError(1135);
     Exit;
    END;

   EvalStringExpression:=t.Ascii; Ok:=True;
END;


        PROCEDURE FreeSymbol(VAR Node:SymbolPtr);
VAR
   Lauf:PCrossRef;
BEGIN
   FreeMem(Node^.SymName,Succ(Length(Node^.SymName^)));

   IF Node^.SymWert.Typ=TempString THEN
    FreeMem(Node^.SymWert.SWert,Succ(Length(Node^.SymWert.SWert^)));

   WHILE Node^.RefList<>Nil DO
    BEGIN
     Lauf:=Node^.RefList^.Next;
     Dispose(Node^.RefList);
     Node^.RefList:=Lauf;
    END;

   Dispose(Node); Node:=Nil;
END;

VAR
   serr:String;
   snum:String[10];

        FUNCTION EnterTreeNode(VAR Node:SymbolPtr; Neu:SymbolPtr; MayChange,DoCross:Boolean):Boolean;
VAR
   Hilf,p1,p2:SymbolPtr;
   Grown:Boolean;
   CompErg:ShortInt;
BEGIN
   { StapelÅberlauf prÅfen, noch nichts eingefÅgt }

   ChkStack; EnterTreeNode:=False;

   { an einem Blatt angelangt--> einfach anfÅgen }

   IF Node=Nil THEN
    BEGIN
     Node:=Neu;
     Node^.Balance:=0; Node^.Left:=Nil; Node^.Right:=Nil;
     Node^.Defined:=True; Node^.Used:=False; Node^.Changeable:=MayChange;
     Node^.RefList:=Nil;
     EnterTreeNode:=True;
     IF DoCross THEN
      BEGIN
       Node^.FileNum:=GetFileNum(CurrFileName);
       Node^.LineNum:=CurrLine;
      END;
     Exit;
    END;

   CompErg:=StrCmp(Neu^.SymName^,Node^.SymName^,Neu^.Attribute,Node^.Attribute);

   CASE CompErg OF
     1:BEGIN
        Grown:=EnterTreeNode(Node^.Right,Neu,MayChange,DoCross);
        IF (BalanceTree) AND (Grown) THEN
        CASE Node^.Balance OF
        -1:Node^.Balance:=0;
        0:BEGIN Node^.Balance:=1; EnterTreeNode:=True; END;
        1:BEGIN
           p1:=Node^.Right;
           IF p1^.Balance=1 THEN
            BEGIN
             Node^.Right:=p1^.Left; p1^.Left:=Node;
             Node^.Balance:=0; Node:=p1;
            END
           ELSE
            BEGIN
             p2:=p1^.Left;
             p1^.Left:=p2^.Right; p2^.Right:=p1;
             Node^.Right:=p2^.Left; p2^.Left:=Node;
             IF p2^.Balance= 1 THEN Node^.Balance:=-1 ELSE Node^.Balance:=0;
             IF p2^.Balance=-1 THEN p1^  .Balance:= 1 ELSE p1^  .Balance:=0;
             Node:=p2;
            END;
           Node^.Balance:=0;
          END;
        END;
       END;
    -1:BEGIN
        Grown:=EnterTreeNode(Node^.Left,Neu,MayChange,DoCross);
        IF (BalanceTree) AND (Grown) THEN
         CASE Node^.Balance OF
         1:Node^.Balance:=0;
         0:BEGIN Node^.Balance:=-1; EnterTreeNode:=True; END;
         -1:BEGIN
             p1:=Node^.Left;
             IF p1^.Balance=-1 THEN
              BEGIN
               Node^.Left:=p1^.Right; p1^.Right:=Node;
               Node^.Balance:=0; Node:=p1;
              END
             ELSE
              BEGIN
               p2:=p1^.Right;
               p1^.Right:=p2^.Left; p2^.Left:=p1;
               Node^.Left:=p2^.Right; p2^.Right:=Node;
               IF p2^.Balance=-1 THEN Node^.Balance:= 1 ELSE Node^.Balance:=0;
               IF p2^.Balance= 1 THEN p1^  .Balance:=-1 ELSE p1  ^.Balance:=0;
               Node:=p2;
              END;
             Node^.Balance:=0;
            END;
         END;
       END;
    0:BEGIN
       IF (Node^.Defined) AND (NOT MayChange) THEN
        BEGIN
         serr:=Node^.SymName^;
         IF DoCross THEN
          BEGIN
           Str(Node^.LineNum,snum);
           serr:=serr+', '+PrevDefMsg+' '+GetFileName(Node^.FileNum)+':'+snum;
          END;
         WrXError(1000,serr)
        END
       ELSE
        BEGIN
         IF NOT MayChange THEN
          BEGIN
           IF (Neu^.SymWert.Typ<>Node^.SymWert.Typ)
           OR ((Neu^.SymWert.Typ=TempString) AND (Neu^.SymWert.SWert^<>Node^.SymWert.SWert^))
           OR ((Neu^.SymWert.Typ=TempFloat ) AND (Neu^.SymWert.FWert <>Node^.SymWert.FWert ))
           OR ((Neu^.SymWert.Typ=TempInt   ) AND (Neu^.SymWert.IWert <>Node^.SymWert.IWert ))
           THEN
            BEGIN
             IF (NOT Repass) AND (JmpErrors>0) THEN
              BEGIN
               IF ThrowErrors THEN Dec(ErrorCount,JmpErrors);
               JmpErrors:=0;
              END;
             Repass:=True;
             IF (MsgIfRepass) AND (PassNo>=PassNoForMessage) THEN
              BEGIN
               serr:=Neu^.SymName^;
               IF Neu^.Attribute<>-1 THEN serr:=serr+'['+GetSectionName(Neu^.Attribute)+']';
               WrXError(80,serr);
              END;
            END;
          END;
         Neu^.Left:=Node^.Left; Neu^.Right:=Node^.Right; Neu^.Balance:=Node^.Balance;
         IF DoCross THEN
          BEGIN
           Neu^.LineNum:=Node^.LineNum; Neu^.FileNum:=Node^.FileNum;
          END;
         Neu^.RefList:=Node^.RefList; Node^.RefList:=Nil;
         Neu^.Defined:=True; Neu^.Used:=Node^.Used; Neu^.Changeable:=MayChange;
         Hilf:=Node; Node:=Neu;
         FreeSymbol(Hilf);
        END
      END;
    END;
END;

        PROCEDURE EnterLocSymbol(Neu:SymbolPtr);
BEGIN
   Neu^.Attribute:=MomLocHandle;
   IF NOT CaseSensitive THEN NLS_UpString(Neu^.SymName^);
   IF EnterTreeNode(FirstLocSymbol,Neu,False,False) THEN;
END;

        PROCEDURE EnterSymbol(Neu:SymbolPtr; MayChange:Boolean; ResHandle:LongInt);
VAR
   Lauf,Prev:PForwardSymbol;
   RRoot:^PForwardSymbol;
   SearchErg:Byte;
   CombName:String;
   RunSect:PSaveSection;
   MSect:LongInt;
   Copy:SymbolPtr;

        PROCEDURE Search(VAR Root:PForwardSymbol; ResCode:Byte);
BEGIN
   Lauf:=Root; Prev:=Nil; RRoot:=@Root;
   WHILE (Lauf<>Nil) AND (Lauf^.Name^<>Neu^.SymName^) DO
    BEGIN
     Prev:=Lauf; Lauf:=Lauf^.Next;
    END;
   IF Lauf<>Nil THEN SearchErg:=ResCode;
END;

BEGIN
{   Neu^.Attribute:=MomSectionHandle;
   IF SectionStack<>Nil THEN
    BEGIN
     Search(SectionStack^.GlobSyms);
     IF Lauf<>Nil THEN Neu^.Attribute:=Lauf^.DestSection
     ELSE Search(SectionStack^.LocSyms);
     IF Lauf<>Nil THEN
      BEGIN
       FreeMem(Lauf^.Name,Length(Lauf^.Name^)+1);
       IF Prev=Nil THEN RRoot^:=Lauf^.Next
       ELSE Prev^.Next:=Lauf^.Next;
       Dispose(Lauf);
      END;
    END;
   IF EnterTreeNode(FirstSymbol,Neu,MayChange,MakeCrossList) THEN;}

   IF NOT CaseSensitive THEN NLS_UpString(Neu^.SymName^);

   SearchErg:=0;
   IF ResHandle=-2 THEN Neu^.Attribute:=MomSectionHandle
   ELSE Neu^.Attribute:=ResHandle;
   IF (SectionStack<>Nil) AND (Neu^.Attribute=MomSectionHandle) THEN
    BEGIN
     Search(SectionStack^.LocSyms,1);
     IF Lauf=Nil THEN Search(SectionStack^.GlobSyms,2);
     IF Lauf=Nil THEN Search(SectionStack^.ExportSyms,3);
     IF SearchErg=2 THEN Neu^.Attribute:=Lauf^.DestSection;
     IF SearchErg=3 THEN
      BEGIN
       CombName:=Neu^.SymName^;
       RunSect:=SectionStack; MSect:=MomSectionHandle;
       WHILE (MSect<>Lauf^.DestSection) AND (RunSect<>Nil) DO
        BEGIN
         CombName:=GetSectionName(MSect)+'_'+CombName;
         MSect:=RunSect^.Handle; RunSect:=RunSect^.Next;
        END;
       New(Copy); Copy^:=Neu^;
       WITH Copy^ DO
        BEGIN
         GetMem(SymName,Length(CombName)+1); SymName^:=CombName;
         Attribute:=Lauf^.DestSection;
         IF SymWert.Typ=TempString THEN
          BEGIN
           GetMem(SymWert.SWert,Length(Neu^.SymWert.SWert^)+1);
           SymWert.SWert^:=Neu^.SymWert.SWert^;
          END;
        END;
       IF EnterTreeNode(FirstSymbol,Copy,MayChange,MakeCrossList) THEN;
      END;
     IF Lauf<>Nil THEN
      BEGIN
       FreeMem(Lauf^.Name,Length(Lauf^.Name^)+1);
       IF Prev=Nil THEN RRoot^:=Lauf^.Next
       ELSE Prev^.Next:=Lauf^.Next;
       Dispose(Lauf);
      END;
    END;
   IF EnterTreeNode(FirstSymbol,Neu,MayChange,MakeCrossList) THEN;
END;

        PROCEDURE PrintSymTree(Name:String);
BEGIN
   WriteLn(Debug,'---------------------');
   WriteLn(Debug,'Enter Symbol ',Name);
   WriteLn(Debug);
   PrintSymbolTree; PrintSymbolDepth;
END;

        PROCEDURE EnterIntSymbol(Name:String; Wert:LongInt; Typ:Byte; MayChange:Boolean);
VAR
   Neu:SymbolPtr;
   DestHandle:LongInt;
BEGIN
   IF NOT ExpandSymbol(Name) THEN Exit;
   IF NOT GetSymSection(Name,DestHandle) THEN Exit;
   IF NOT ChkSymbName(Name) THEN
    BEGIN
     WrXError(1020,Name); Exit;
    END;
   New(Neu);
   WITH Neu^ DO
    BEGIN
     GetMem(SymName,Length(Name)+1); SymName^:=Name;
     SymWert.Typ:=TempInt; SymWert.IWert:=Wert; SymType:=Typ; SymSize:=-1;
    END;

   IF (MomLocHandle=-1) OR (DestHandle<>-2) THEN
    BEGIN
     EnterSymbol(Neu,MayChange,DestHandle);
     IF MakeDebug THEN PrintSymTree(Name);
    END
   ELSE EnterLocSymbol(Neu);
END;

        PROCEDURE EnterFloatSymbol(Name:String; Wert:Extended; MayChange:Boolean);
VAR
   Neu:SymbolPtr;
   DestHandle:LongInt;
BEGIN
   IF NOT ExpandSymbol(Name) THEN Exit;
   IF NOT GetSymSection(Name,DestHandle) THEN Exit;
   IF NOT ChkSymbName(Name) THEN
    BEGIN
     WrXError(1020,Name); Exit;
    END;
   New(Neu);
   WITH Neu^ DO
    BEGIN
     GetMem(SymName,Length(Name)+1); SymName^:=Name;
     SymWert.Typ:=TempFloat; SymWert.FWert:=Wert; SymType:=0; SymSize:=-1;
    END;

   IF (MomLocHandle=-1) OR (DestHandle<>-2) THEN
    BEGIN
     EnterSymbol(Neu,MayChange,DestHandle);
     IF MakeDebug THEN PrintSymTree(Name);
    END
   ELSE EnterLocSymbol(Neu);
END;

        PROCEDURE EnterStringSymbol(Name:String; VAR Wert:String; MayChange:Boolean);
VAR
   Neu:SymbolPtr;
   DestHandle:LongInt;
BEGIN
   IF NOT ExpandSymbol(Name) THEN Exit;
   IF NOT GetSymSection(Name,DestHandle) THEN Exit;
   IF NOT ChkSymbName(Name) THEN
    BEGIN
     WrXError(1020,Name); Exit;
    END;
   New(Neu);
   WITH Neu^ DO
    BEGIN
     GetMem(SymName,Length(Name)+1); SymName^:=Name;
     GetMem(SymWert.SWert,Length(Wert)+1); SymWert.SWert^:=Wert; SymSize:=-1;
     SymWert.Typ:=TempString; SymType:=0;
    END;

   IF (MomLocHandle=-1) OR (DestHandle<>-2) THEN
    BEGIN
     EnterSymbol(Neu,MayChange,DestHandle);
     IF MakeDebug THEN PrintSymTree(Name);
    END
   ELSE EnterLocSymbol(Neu);
END;

        PROCEDURE AddReference(Node:SymbolPtr);
VAR
   Lauf,Neu:PCrossRef;
BEGIN
   { Speicher belegen }

   New(Neu); Neu^.LineNum:=CurrLine; Neu^.OccNum:=1; Neu^.Next:=Nil;

   { passende Datei heraussuchen }

   Neu^.FileNum:=GetFileNum(CurrFileName);

   { suchen, ob Eintrag schon existiert }

   Lauf:=Node^.RefList;
   WHILE (Lauf<>Nil)
     AND ((Lauf^.FileNum<>Neu^.FileNum) OR (Lauf^.LineNum<>Neu^.LineNum)) DO
    Lauf:=Lauf^.Next;

   { schon einmal in dieser Datei in dieser Zeile aufgetaucht: nur ZÑhler
     rauf: }

   IF Lauf<>Nil THEN
    BEGIN
     Inc(Lauf^.OccNum); Dispose(Neu);
    END

   { ansonsten an Kettenende anhÑngen }

   ELSE IF Node^.RefList=Nil THEN Node^.RefList:=Neu

   ELSE
    BEGIN
     Lauf:=Node^.RefList;
     WHILE (Lauf^.Next<>Nil) DO Lauf:=Lauf^.Next;
     Lauf^.Next:=Neu;
    END;
END;

        FUNCTION FindNode(Name:String; SearchType:TempType):SymbolPtr;
VAR
   Lauf:PSaveSection;
   DestSection:LongInt;

        FUNCTION FNode(Handle:LongInt):Boolean;
VAR
   Lauf:SymbolPtr;
   SErg:ShortInt;
BEGIN
   Lauf:=FirstSymbol; SErg:=-1; FNode:=False;
   WHILE (Lauf<>Nil) AND (SErg<>0) DO
    BEGIN
     SErg:=StrCmp(Name,Lauf^.SymName^,Handle,Lauf^.Attribute);
     IF SErg=-1 THEN Lauf:=Lauf^.Left
     ELSE IF SErg=1 THEN Lauf:=Lauf^.Right;
    END;
   IF Lauf<>Nil THEN
    IF Lauf^.SymWert.Typ=SearchType THEN
     BEGIN
      FindNode:=Lauf; FNode:=True;
      IF (MakeCrossList AND DoRefs) THEN AddReference(Lauf);
     END;
END;

        FUNCTION FSpec(Root:PForwardSymbol):Boolean;
BEGIN
   WHILE (Root<>Nil) AND (Root^.Name^<>Name) DO Root:=Root^.Next;
   FSpec:=Root<>Nil;
END;

BEGIN
   FindNode:=Nil;
   IF NOT GetSymSection(Name,DestSection) THEN Exit;
   IF NOT CaseSensitive THEN NLS_UpString(Name);
   IF SectionStack<>Nil THEN
    IF PassNo<=MaxSymPass THEN
     IF FSpec(SectionStack^.LocSyms) THEN DestSection:=MomSectionHandle;
{      IF FSpec(SectionStack^.GlobSyms) THEN Exit;}
   IF DestSection=-2 THEN
    BEGIN
     IF FNode(MomSectionHandle) THEN Exit;
     Lauf:=SectionStack;
     WHILE Lauf<>Nil DO
      BEGIN
       IF FNode(Lauf^.Handle) THEN Exit;
       Lauf:=Lauf^.Next;
      END;
    END
   ELSE IF FNode(DestSection) THEN;
END;

        FUNCTION FindLocNode(Name:String; SearchType:TempType):SymbolPtr;
VAR
   RunLocHandle:PLocHandle;

        FUNCTION FNode(Handle:LongInt):Boolean;
VAR
   Lauf:SymbolPtr;
   SErg:ShortInt;
BEGIN
   Lauf:=FirstLocSymbol; SErg:=-1; FNode:=False;
   WHILE (Lauf<>Nil) AND (SErg<>0) DO
    BEGIN
     SErg:=StrCmp(Name,Lauf^.SymName^,Handle,Lauf^.Attribute);
     IF SErg=-1 THEN Lauf:=Lauf^.Left
     ELSE IF SErg=1 THEN Lauf:=Lauf^.Right;
    END;

   IF (Lauf<>Nil) THEN
    IF Lauf^.SymWert.Typ=SearchType THEN
     BEGIN
      FindLocNode:=Lauf; FNode:=True;
     END;
END;

BEGIN
   FindLocNode:=Nil;

   IF NOT CaseSensitive THEN NLS_UpString(Name);

   IF MomLocHandle=-1 THEN Exit;

   IF FNode(MomLocHandle) THEN Exit;

   RunLocHandle:=FirstLocHandle;
   WHILE (RunLocHandle<>Nil) AND (RunLocHandle^.Cont<>-1) DO
    BEGIN
     IF FNode(RunLocHandle^.Cont) THEN Exit;
     RunLocHandle:=RunLocHandle^.Next;
    END;
END;

        PROCEDURE SetSymbolType(Name:String; NTyp:Byte);
VAR
   Lauf:SymbolPtr;
   HRef:Boolean;
BEGIN
   IF NOT ExpandSymbol(Name) THEN Exit;
   HRef:=DoRefs; DoRefs:=False;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf<>Nil THEN Lauf^.SymType:=NTyp;
   DoRefs:=HRef;
END;

        FUNCTION GetIntSymbol(Name:String; VAR Wert:LongInt):Boolean;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     GetIntSymbol:=False; Exit;
    END;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   GetIntSymbol:=Lauf<>Nil;
   IF Lauf<>Nil THEN
    BEGIN
     Wert:=Lauf^.SymWert.IWert;
     IF Lauf^.SymType<>0 THEN TypeFlag:=TypeFlag OR (1 SHL Lauf^.SymType);
     IF (Lauf^.SymSize<>-1) AND (SizeFlag<>-1) THEN SizeFlag:=Lauf^.SymSize;
     Lauf^.Used:=True;
    END
   ELSE
    BEGIN
     Wert:=EProgCounter;
     IF PassNo>MaxSymPass THEN WrXError(1010,Name);
    END;
END;

        FUNCTION GetFloatSymbol(Name:String; VAR Wert:Extended):Boolean;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     GetFloatSymbol:=False; Exit;
    END;
   Lauf:=FindLocNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempFloat);
   GetFloatSymbol:=Lauf<>Nil;
   IF Lauf<>Nil THEN
    BEGIN
     Wert:=Lauf^.SymWert.FWert;
     Lauf^.Used:=True;
    END
   ELSE
    BEGIN
     Wert:=0;
     IF PassNo>MaxSymPass THEN WrXError(1010,Name);
    END;
END;

        FUNCTION GetStringSymbol(Name:String; VAR Wert:String):Boolean;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     GetStringSymbol:=False; Exit;
    END;
   Lauf:=FindLocNode(Name,TempString);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempString);
   GetStringSymbol:=Lauf<>Nil;
   IF Lauf<>Nil THEN
    BEGIN
     Wert:=Lauf^.SymWert.SWert^;
     Lauf^.Used:=True;
    END
   ELSE
    BEGIN
     Wert:='';
     IF PassNo>MaxSymPass THEN WrXError(1010,Name);
    END;
END;

        PROCEDURE SetSymbolSize(Name:String; Size:ShortInt);
VAR
   Lauf:SymbolPtr;
   HRef:Boolean;
BEGIN
   IF NOT ExpandSymbol(Name) THEN Exit;
   HRef:=DoRefs; DoRefs:=False;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf<>Nil THEN Lauf^.SymSize:=Size;
   DoRefs:=HRef;
END;

        FUNCTION GetSymbolSize(Name:String):ShortInt;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     GetSymbolSize:=-1; Exit;
    END;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf<>Nil THEN GetSymbolSize:=Lauf^.SymSize ELSE GetSymbolSize:=-1;
END;

        FUNCTION IsSymbolFloat(Name:String):Boolean;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     IsSymbolFloat:=False; Exit;
    END;
   Lauf:=FindLocNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempFloat);
   IsSymbolFloat:=(Lauf<>Nil) AND (Lauf^.SymWert.Typ=TempFloat);
END;

        FUNCTION IsSymbolString(Name:String):Boolean;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     IsSymbolString:=False; Exit;
    END;
   Lauf:=FindLocNode(Name,TempString);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempString);
   IsSymbolString:=(Lauf<>Nil) AND (Lauf^.SymWert.Typ=TempString);
END;

        FUNCTION IsSymbolDefined(Name:String):Boolean;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     IsSymbolDefined:=False; Exit;
    END;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindLocNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindLocNode(Name,TempString);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempString);
   IsSymbolDefined:=(Lauf<>Nil) AND (Lauf^.Defined);
END;

        FUNCTION IsSymbolUsed(Name:String):Boolean;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     IsSymbolUsed:=False; Exit;
    END;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindLocNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindLocNode(Name,TempString);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempString);
   IsSymbolUsed:=(Lauf<>Nil) AND (Lauf^.Used);
END;

        FUNCTION IsSymbolChangeable(Name:String):Boolean;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     IsSymbolChangeable:=False; Exit;
    END;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindLocNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindLocNode(Name,TempString);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempString);
   IsSymbolChangeable:=(Lauf<>Nil) AND (Lauf^.Changeable);
END;

        FUNCTION GetSymbolType(Name:String):Integer;
VAR
   Lauf:SymbolPtr;
BEGIN
   IF NOT ExpandSymbol(Name) THEN
    BEGIN
     GetSymbolType:=-1; Exit;
    END;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindLocNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindLocNode(Name,TempString);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempFloat);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempString);
   IF Lauf=Nil THEN GetSymbolType:=-1
   ELSE GetSymbolType:=Lauf^.SymType;
END;

        PROCEDURE ConvertSymboLVal(VAR Inp:SymboLVal; VAR Outp:TempResult);
BEGIN
   Outp.Typ:=Inp.Typ;
   CASE Inp.Typ OF
   TempInt   :Outp.Int  :=Inp.IWert;
   TempFloat :Outp.Float:=Inp.FWert;
   TempString:Outp.Ascii:=Inp.SWert^;
   END;
END;

        PROCEDURE PrintSymbolList;
VAR
   Zeilenrest:String;
   Sum,USum:LongInt;
   ActPageWidth,cwidth:Integer;

        PROCEDURE AddOut(s:String);
BEGIN
   IF Length(s)+Length(Zeilenrest)>ActPageWidth THEN
    BEGIN
     Dec(Byte(Zeilenrest[0]),2);
     WrLstLine(Zeilenrest); Zeilenrest:=s;
    END
   ELSE Zeilenrest:=Zeilenrest+s
END;

        PROCEDURE PNode(Node:SymbolPtr);
VAR
   s1,sh:String;
   l1,l2:Byte;
   t:TempResult;
BEGIN
   ConvertSymboLVal(Node^.SymWert,t); s1:=StrSym(t,False);

   sh:=Node^.SymName^;
   IF Node^.Attribute<>-1 THEN sh:=sh+' ['+GetSectionName(Node^.Attribute)+']';
   IF Node^.Used THEN sh:=' '+sh ELSE sh:='*'+sh;
   l1:=(Length(s1)+Length(sh)+6) MOD cwidth;
   s1:=sh+' : '+Blanks(cwidth-2-l1)+s1+' '+SegShorts[Node^.SymType]+' | ';
   AddOut(s1); Inc(Sum);
   IF NOT Node^.Used THEN Inc(USum);
END;

        PROCEDURE PrintNode(Node:SymbolPtr);
BEGIN
   ChkStack;

   IF Node=Nil THEN Exit;

   PrintNode(Node^.Left);
   PNode(Node);
   PrintNode(Node^.Right);
END;

BEGIN
   NewPage(ChapDepth,True);
   WrLstLine(ListSymListHead1);
   WrLstLine(ListSymListHead2);
   WrLstLine('');

   Zeilenrest:=''; Sum:=0; USum:=0;
   ActPageWidth:=PageWidth; IF ActPageWidth=0 THEN ActPageWidth:=80;
   cwidth:=ActPageWidth SHR 1;
   PrintNode(FirstSymbol);
   IF ZeilenRest<>'' THEN
    BEGIN
     Dec(Byte(Zeilenrest[0]),2);
     WrLstLine(Zeilenrest);
    END;
   WrLstLine('');
   Str(Sum:7,Zeilenrest);
   IF Sum=1 THEN Zeilenrest:=Zeilenrest+ListSymSumMsg
   ELSE Zeilenrest:=Zeilenrest+ListSymSumsMsg;
   WrLstLine(Zeilenrest);
   Str(USum:7,Zeilenrest);
   IF USum=1 THEN Zeilenrest:=Zeilenrest+ListUSymSumMsg
   ELSE Zeilenrest:=Zeilenrest+ListUSymSumsMsg;
   WrLstLine(Zeilenrest);
   WrLstLine('');
END;

	PROCEDURE PrintDebSymbols(VAR F:Text);
VAR
   Space:Integer;
   HWritten:Boolean;

   	PROCEDURE PNode(Node:SymbolPtr);
VAR
   l1,z,z2:Integer;
   t:TempResult;
   is,s:String;
   ch:Char;
BEGIN
   IF NOT HWritten THEN
    BEGIN
     WriteLn(F); ChkIO(10004);
     WriteLn(F,'Symbols in Segment ',SegNames[Space]); ChkIO(10004);
     HWritten:=True;
    END;

   WITH Node^ DO
    BEGIN
     Write(F,SymName^); ChkIO(10004); l1:=Length(SymName^);
     IF Attribute<>-1 THEN
      BEGIN
       Str(Attribute,is);
       Write(F,'[',is,']'); ChkIO(10004);
       Inc(l1,Length(is)+2);
      END;
     Write(F,Blanks(37-l1),' '); ChkIO(10004);
     CASE SymWert.Typ OF
     TempInt:    Write(F,'Int    ');
     TempFloat:  Write(F,'Float  ');
     TempString: Write(F,'String ');
     END;
     ChkIO(10004);
     IF SymWert.Typ=TempString THEN
      BEGIN
       l1:=0; 
       FOR z:=1 TO Length(SymWert.SWert^) DO
        BEGIN
         ch:=SymWert.SWert^[z];
         IF (ch='\') OR (ch<=' ') THEN
          BEGIN
           Str(Ord(ch),s); Write(F,'\'); ChkIO(10004);
           FOR z2:=1 TO 3-Length(s) DO
            BEGIN
             Write(F,'0'); ChkIO(10004);
            END;
           Write(F,s); ChkIO(10004); Inc(l1,4);
          END
         ELSE
          BEGIN
           Write(F,ch); ChkIO(10004); Inc(l1);
          END;
        END;
      END
     ELSE
      BEGIN
       ConvertSymboLVal(SymWert,t); s:=StrSym(t,False);
       l1:=Length(s);
       Write(F,s); ChkIO(10004);
      END;
     Write(F,Blanks(25-l1),' '); ChkIO(10004);
     Str(SymSize,s); Write(F,s,Blanks(3-Length(s)),' '); ChkIO(10004);
     WriteLn(F,Used); ChkIO(10004);
    END;
END;

        PROCEDURE PrintNode(Node:SymbolPtr);
BEGIN
   ChkStack;

   IF Node=Nil THEN Exit;

   PrintNode(Node^.Left);

   IF Node^.SymType=Space THEN PNode(Node);

   PrintNode(Node^.Right);
END;

BEGIN
   FOR Space:=0 TO PCMax DO
    BEGIN
     HWritten:=False;
     PrintNode(FirstSymbol);
    END;
END;

        PROCEDURE PrintSymbolTree;

        PROCEDURE PrintNode(Node:SymbolPtr; Shift:Integer);
VAR
   z:Byte;
BEGIN
   IF Node=Nil THEN Exit;

   PrintNode(Node^.Left,Shift+1);

   FOR z:=1 TO Shift DO Write(Debug,'':6);
   WriteLn(Debug,Node^.SymName^);

   PrintNode(Node^.Right,Shift+1);
END;

BEGIN
   PrintNode(FirstSymbol,0);
END;

        PROCEDURE ClearSymbolList;
VAR
   Hilf:SymbolPtr;

        PROCEDURE ClearNode(VAR Node:SymbolPtr);
BEGIN
   WITH Node^ DO
    BEGIN
     IF Left<>Nil THEN ClearNode(Left);
     IF Right<>Nil THEN ClearNode(Right);
    END;
   FreeSymbol(Node);
END;

BEGIN
   IF FirstSymbol<>NIL THEN ClearNode(FirstSymbol);

   IF FirstLocSymbol<>NIL THEN ClearNode(FirstLocSymbol);
END;

{---------------------------------------------------------------------------}
{ Stack-Verwaltung }

        FUNCTION PushSymbol(SymName,StackName:String):Boolean;
VAR
   Src:SymbolPtr;
   LStack,NStack,PStack:PSymbolStack;
   Elem:PSymbolStackEntry;
BEGIN
   PushSymbol:=False;

   IF NOT ExpandSymbol(SymName) THEN Exit;

   Src:=FindNode(SymName,TempInt);
   IF Src=Nil THEN Src:=FindNode(SymName,TempFloat);
   IF Src=Nil THEN Src:=FindNode(SymName,TempString);
   IF Src=Nil THEN
    BEGIN
     WrXError(1010,SymName); Exit;
    END;

   IF StackName='' THEN StackName:=DefStackName;
   IF NOT ExpandSymbol(StackName) THEN Exit;
   IF NOT ChkSymbName(StackName) THEN
    BEGIN
     WrXError(1020,StackName); Exit;
    END;

   LStack:=FirstStack; PStack:=Nil;
   WHILE ((LStack<>Nil) AND (LStack^.Name^<StackName)) DO
    BEGIN
     PStack:=LStack;
     LStack:=LStack^.Next;
    END;

   IF ((LStack=Nil) OR (LStack^.Name^>StackName)) THEN
    BEGIN
     New(NStack);
     GetMem(NStack^.Name,Length(StackName)+1); NStack^.Name^:=StackName;
     NStack^.Contents:=Nil;
     NStack^.Next:=LStack;
     IF PStack=Nil THEN FirstStack:=NStack ELSE PStack^.Next:=NStack;
     LStack:=NStack;
    END;

   New(Elem); Elem^.Next:=LStack^.Contents;
   Elem^.Contents:=Src^.SymWert;
   LStack^.Contents:=Elem;

   PushSymbol:=True;
END;

        FUNCTION PopSymbol(SymName,StackName:String):Boolean;
VAR
   Dest:SymbolPtr;
   LStack,PStack:PSymbolStack;
   Elem:PSymbolStackEntry;
BEGIN
   PopSymbol:=False;

   IF NOT ExpandSymbol(SymName) THEN Exit;

   Dest:=FindNode(SymName,TempInt);
   IF Dest=Nil THEN Dest:=FindNode(SymName,TempFloat);
   IF Dest=Nil THEN Dest:=FindNode(SymName,TempString);
   IF Dest=Nil THEN
    BEGIN
     WrXError(1010,SymName); Exit;
    END;

   IF StackName='' THEN StackName:=DefStackName;
   IF NOT ExpandSymbol(StackName) THEN Exit;
   IF NOT ChkSymbName(StackName) THEN
    BEGIN
     WrXError(1020,StackName); Exit;
    END;

   LStack:=FirstStack; PStack:=Nil;
   WHILE ((LStack<>Nil) AND (LStack^.Name^<StackName)) DO
    BEGIN
     PStack:=LStack;
     LStack:=LStack^.Next;
    END;

   IF ((LStack=Nil) OR (LStack^.Name^>StackName)) THEN
    BEGIN
     WrXError(1530,StackName); Exit;
    END;

   Elem:=LStack^.Contents;
   Dest^.SymWert:=Elem^.Contents;
   LStack^.Contents:=Elem^.Next;
   IF LStack^.Contents=Nil THEN
    BEGIN
     IF PStack=Nil THEN FirstStack:=LStack^.Next ELSE PStack^.Next:=LStack^.Next;
     FreeMem(LStack^.Name,Length(LStack^.Name^)+1);
     Dispose(LStack);
    END;
   Dispose(Elem);

   PopSymbol:=True;
END;

        PROCEDURE ClearStacks;
VAR
   Act:PSymbolStack;
   ELem:PSymbolStackEntry;
   z:Integer;
   s:String;
BEGIN
   WHILE FirstStack<>Nil DO
    BEGIN
     z:=0; Act:=FirstStack;
     WHILE Act^.Contents<>Nil DO
      BEGIN
       Elem:=Act^.Contents; Act^.Contents:=Elem^.Next;
       Dispose(Elem); Inc(z);
      END;
     Str(z,s); WrXError(230,Act^.Name^+'('+s+')');
     FreeMem(Act^.Name,Length(Act^.Name^)+1);
     FirstStack:=Act^.Next; Dispose(Act);
    END;
END;

{---------------------------------------------------------------------------}
{ Funktionsverwaltung }

        PROCEDURE EnterFunction(FName,FDefinition:String; NewCnt:Byte);
VAR
   Neu:PFunction;
BEGIN
   IF NOT CaseSensitive THEN NLS_UpString(FName);

   IF NOT ChkSymbName(FName) THEN
    BEGIN
     WrXError(1020,FName); Exit;
    END;

   IF FindFunction(FName)<>Nil THEN
    BEGIN
     IF PassNo=1 THEN WrXError(1000,FName); Exit;
    END;

   New(Neu);
   WITH Neu^ DO
    BEGIN
     Next:=FirstFunction; ArguCnt:=NewCnt;
     GetMem(Name,Length(FName)+1); Name^:=FName;
     GetMem(Definition,Length(FDefinition)+1); Definition^:=FDefinition;
    END;
   FirstFunction:=Neu;
END;

        FUNCTION FindFunction(Name:String):PFunction;
VAR
   Lauf:PFunction;
BEGIN
   IF NOT CaseSensitive THEN NLS_UpString(Name);

   Lauf:=FirstFunction;
   WHILE (Lauf<>Nil) AND (Lauf^.Name^<>Name) DO Lauf:=Lauf^.Next;
   FindFunction:=Lauf;
END;

        PROCEDURE PrintFunctionList;
VAR
   Lauf:PFunction;
   OneS:String;
   cnt:Boolean;
BEGIN
   IF FirstFunction=Nil THEN Exit;

   NewPage(ChapDepth,True);
   WrLstLine(ListFuncListHead1);
   WrLstLine(ListFuncListHead2);
   WrLstLine('');

   OneS:=''; Lauf:=FirstFunction; cnt:=False;
   WHILE Lauf<>Nil DO
   WITH Lauf^ DO
    BEGIN
     OneS:=OneS+Name^;
     IF Length(Name^)<37 THEN OneS:=OneS+Blanks(37-Length(Name^));
     IF NOT cnt THEN OneS:=OneS+' | '
     ELSE
      BEGIN
       WrLstLine(OneS); OneS:='';
      END;
     cnt:=NOT cnt;
     Lauf:=Lauf^.Next;
    END;
   IF cnt THEN
    BEGIN
     Dec(Byte(OneS[0]),2);
     WrLstLine(OneS);
    END;
   WrLstLine('');
END;

        PROCEDURE ClearFunctionList;
VAR
   Lauf:PFunction;
BEGIN
   WHILE FirstFunction<>Nil DO
    WITH FirstFunction^ DO
     BEGIN
      Lauf:=Next;
      FreeMem(Name,Length(Name^)+1);
      FreeMem(Definition,Length(Definition^)+1);
      Dispose(FirstFunction);
      FirstFunction:=Lauf;
     END;
END;

{---------------------------------------------------------------------------}

        PROCEDURE ResetSymbolDefines;
VAR
   Lauf:SymbolPtr;

        PROCEDURE ResetNode(Node:SymbolPtr);
BEGIN
   IF Node^.Left <>Nil THEN ResetNode(Node^.Left);
   IF Node^.Right<>Nil THEN ResetNode(Node^.Right);
   Node^.Defined:=False; Node^.Used:=False;
END;

BEGIN
   IF FirstSymbol<>Nil THEN ResetNode(FirstSymbol);

   IF FirstLocSymbol<>Nil THEN ResetNode(FirstLocSymbol);
END;

        PROCEDURE SetFlag(VAR Flag:Boolean; Name:String; Wert:Boolean);
BEGIN
   Flag:=Wert; EnterIntSymbol(Name,Ord(Flag),0,True);
END;

        PROCEDURE AddDefSymbol(Name:String; VAR Value:TempResult);
VAR
   Neu:PDefSymbol;
BEGIN
   Neu:=FirstDefSymbol;
   WHILE Neu<>Nil DO
    BEGIN
     IF Neu^.SymName^=Name THEN Exit;
     Neu:=Neu^.Next;
    END;

   New(Neu);
   WITH Neu^ DO
    BEGIN
     Next:=FirstDefSymbol;
     GetMem(SymName,Length(Name)+1); SymName^:=Name;
     Wert:=Value;
    END;
   FirstDefSymbol:=Neu;
END;

        PROCEDURE RemoveDefSymbol(Name:String);
VAR
  Save,Lauf:PDefSymbol;
BEGIN
  IF FirstDefSymbol=Nil THEN Exit;

  IF FirstDefSymbol^.SymName^=Name THEN
   BEGIN
    Save:=FirstDefSymbol; FirstDefSymbol:=FirstDefSymbol^.Next;
   END
  ELSE
   BEGIN
    Lauf:=FirstDefSymbol;
    WHILE (Lauf^.Next<>Nil) AND (Lauf^.Next^.SymName^<>Name) DO Lauf:=Lauf^.Next;
    IF Lauf^.Next=Nil THEN Exit;
    Save:=Lauf^.Next; Lauf^.Next:=Lauf^.Next^.Next;
   END;
  FreeMem(Save^.SymName,Length(Save^.SymName^)+1); Dispose(Save);
END;

        PROCEDURE CopyDefSymbols;
VAR
   Lauf:PDefSymbol;
BEGIN
   Lauf:=FirstDefSymbol;
   WHILE Lauf<>Nil DO
   WITH Lauf^ DO
    BEGIN
     CASE Wert.Typ OF
     TempInt:EnterIntSymbol(SymName^,Wert.Int,0,True);
     TempFloat:EnterFloatSymbol(SymName^,Wert.Float,True);
     TempString:EnterStringSymbol(SymName^,Wert.Ascii,True);
     END;
     Lauf:=Lauf^.Next;
    END;
END;

        PROCEDURE PrintSymbolDepth;
VAR
   TreeMin,TreeMax:LongInt;

        PROCEDURE SearchTree(Lauf:SymbolPtr; SoFar:LongInt);
BEGIN
   IF Lauf=Nil THEN
    BEGIN
     IF SoFar>TreeMax THEN TreeMax:=SoFar;
     IF SoFar<TreeMin THEN TreeMin:=SoFar;
    END
   ELSE
    BEGIN
     SearchTree(Lauf^.Right,SoFar+1);
     SearchTree(Lauf^.Left,SoFar+1);
    END;
END;

BEGIN
   TreeMin:=MaxLongInt; TreeMax:=0;
   SearchTree(FirstSymbol,0);
   WriteLn(Debug,' MinTree ',TreeMin);
   WriteLn(Debug,' MaxTree ',TreeMax);
END;

        FUNCTION GetSectionHandle(SName:String; AddEmpt:Boolean; Parent:LongInt):LongInt;
VAR
   Lauf,Prev:PCToken;
   z:LongInt;
BEGIN
   if (NOT CaseSensitive) THEN NLS_UpString(SName);

   Lauf:=FirstSection; Prev:=Nil; z:=0;
   WHILE (Lauf<>Nil) AND ((Lauf^.Name^<>SName) OR (Lauf^.Parent<>Parent)) DO
    BEGIN
     Inc(z); Prev:=Lauf; Lauf:=Lauf^.Next;
    END;

   IF Lauf=Nil THEN
    IF AddEmpt THEN
     BEGIN
      New(Lauf);
      WITH Lauf^ DO
       BEGIN
        Parent:=MomSectionHandle;
        GetMem(Name,Length(SName)+1); Name^:=SName;
        Next:=Nil;
        InitChunk(Usage);
       END;
      IF Prev=Nil THEN FirstSection:=Lauf ELSE Prev^.Next:=Lauf;
     END
    ELSE z:=-2;
   GetSectionHandle:=z;
END;

        FUNCTION GetSectionName(Handle:LongInt):String;
VAR
   Lauf:PCToken;
BEGIN
   GetSectionName:=''; IF Handle=-1 THEN Exit;
   Lauf:=FirstSection;
   WHILE (Handle>0) AND (Lauf<>Nil) DO
    BEGIN
     Lauf:=Lauf^.Next; Dec(Handle);
    END;
   IF Lauf<>Nil THEN GetSectionName:=Lauf^.Name^;
END;

        PROCEDURE SetMomSection(Handle:LongInt);
VAR
   z:LongInt;
BEGIN
   MomSectionHandle:=Handle;
   IF Handle<0 THEN MomSection:=Nil
   ELSE
    BEGIN
     MomSection:=FirstSection;
     FOR z:=1 TO Handle DO
      IF MomSection<>Nil THEN MomSection:=MomSection^.Next;
    END;
END;

        PROCEDURE AddSectionUsage(Start,Length:LongInt);
BEGIN
   IF (ActPC<>SegCode) OR (MomSection=Nil) THEN Exit;
   IF AddChunk(MomSection^.Usage,Start,Length,False) THEN;
END;

        PROCEDURE PrintSectionList;

        PROCEDURE PSection(Handle:LongInt; Indent:Integer);
VAR
   Lauf:PCToken;
   Cnt:LongInt;
BEGIN
   ChkStack;
   IF Handle<>-1 THEN WrLstLine(Blanks(Indent SHL 1)+GetSectionName(Handle));
   Lauf:=FirstSection; Cnt:=0;
   WHILE Lauf<>Nil DO
    BEGIN
     IF Lauf^.Parent=Handle THEN PSection(Cnt,Indent+1);
     Lauf:=Lauf^.Next; Inc(Cnt);
    END;
END;

BEGIN
   IF FirstSection=Nil THEN Exit;

   NewPage(ChapDepth,True);
   WrLstLine(ListSectionListHead1);
   WrLstLine(ListSectionListHead2);
   WrLstLine('');
   PSection(-1,0);
END;

        PROCEDURE PrintDebSections(VAR f:Text);
VAR
   Lauf:PCToken;
   Cnt,z,l,s:LongInt;
BEGIN
   Lauf:=FirstSection; Cnt:=0;
   WHILE Lauf<>Nil DO
    BEGIN
     WriteLn(f); ChkIO(10004);
     WriteLn(f,'Info for Section ',Cnt,' ',GetSectionName(Cnt),' ',Lauf^.Parent); ChkIO(10004);
     FOR z:=1 TO Lauf^.Usage.RealLen DO
      BEGIN
       l:=Lauf^.Usage.Chunks^[z].Length;
       s:=Lauf^.Usage.Chunks^[z].Start;
       Write(f,HexString(s,0)); ChkIO(10004);
       IF l=1 THEN WriteLn(f) ELSE WriteLn(f,'-',HexString(s+l-1,0)); ChkIO(10004);
      END;
     Lauf:=Lauf^.Next;
     Inc(Cnt);
    END;
END;

        PROCEDURE ClearSectionList;
VAR
   Tmp:PCToken;
BEGIN
   WHILE FirstSection<>Nil DO
    BEGIN
     Tmp:=FirstSection;
     FreeMem(Tmp^.Name,Length(Tmp^.Name^)+1);
     ClearChunk(Tmp^.Usage);
     FirstSection:=Tmp^.Next; Dispose(Tmp);
    END;
END;


        PROCEDURE PrintCrossList;

        PROCEDURE PNode(Node:SymbolPtr);
VAR
   FileZ:Integer;
   Lauf:PCrossRef;
   LinePart,LineAcc:String;
   h:String;
   t:TempResult;
BEGIN
   IF Node^.RefList=Nil THEN Exit;

   ConvertSymboLVal(Node^.SymWert,t);

   Str(Node^.LineNum,h);
   h:=' (='+StrSym(t,False)+','+GetFileName(Node^.FileNum)+'/'+h+'):';
   IF Node^.Attribute<>-1 THEN h:=' ['+GetSectionName(Node^.Attribute)+'] '+h;

   WrLstLine(ListCrossSymName+Node^.SymName^+h);

   FOR FileZ:=0 TO GetFileCount-1 DO
    BEGIN
     Lauf:=Node^.RefList;

     WHILE (Lauf<>Nil) AND (Lauf^.FileNum<>FileZ) DO Lauf:=Lauf^.Next;

     IF Lauf<>Nil THEN
      BEGIN
       WrLstLine(' '+ListCrossFileName+GetFileName(FileZ)+' :');
       LineAcc:='   ';
       WHILE Lauf<>Nil DO
        BEGIN
         Str(Lauf^.LineNum:5,LinePart); LineAcc:=LineAcc+LinePart;
         IF Lauf^.OccNum<>1 THEN
          BEGIN
           Str(Lauf^.OccNum:2,LinePart); LineAcc:=LineAcc+'('+LinePart+')';
          END
         ELSE LineAcc:=LineAcc+'    ';
         IF Length(LineAcc)>=72 THEN
          BEGIN
           WrLstLine(LineAcc); LineAcc:='  ';
          END;
         Lauf:=Lauf^.Next;
        END;
       IF LineAcc<>'  ' THEN WrLstLine(LineAcc);
      END;
    END;
   WrLstLine('');
END;

        PROCEDURE PrintNode(Node:SymbolPtr);
BEGIN
   IF Node=Nil THEN Exit;

   PrintNode(Node^.Left);

   PNode(Node);

   PrintNode(Node^.Right);
END;

BEGIN
   WrLstLine('');
   WrLstLine(ListCrossListHead1);
   WrLstLine(ListCrossListHead2);
   WrLstLine('');
   PrintNode(FirstSymbol);
   WrLstLine('');
END;

        PROCEDURE ClearCrossList;
VAR
   Lauf:PCrossRef;

        PROCEDURE CNode(Node:SymbolPtr);
BEGIN
   IF Node^.Left<>Nil THEN CNode(Node^.Left);

   IF Node<>Nil THEN
    WHILE Node^.RefList<>Nil DO
     BEGIN
      Lauf:=Node^.RefList^.Next;
      Dispose(Node^.RefList);
      Node^.RefList:=Lauf;
     END;

   IF Node^.Right<>Nil THEN CNode(Node^.Right);
END;

BEGIN
   CNode(FirstSymbol);
END;

        FUNCTION GetLocHandle:LongInt;
BEGIN
   GetLocHandle:=LocHandleCnt; Inc(LocHandleCnt);
END;

        PROCEDURE PushLocHandle(NewLoc:LongInt);
VAR
   NewLocHandle:PLocHandle;
BEGIN
   New(NewLocHandle);
   WITH NewLocHandle^ DO
    BEGIN
     Cont:=MomLocHandle; Next:=FirstLocHandle;
    END;
   FirstLocHandle:=NewLocHandle; MomLocHandle:=NewLoc;
END;

        PROCEDURE PopLocHandle;
VAR
   OldLocHandle:PLocHandle;
BEGIN
   OldLocHandle:=FirstLocHandle;
   IF OldLocHandle=Nil THEN Exit;
   WITH OldLocHandle^ DO
    BEGIN
     MomLocHandle:=Cont; FirstLocHandle:=Next;
    END;
   Dispose(OldLocHandle);
END;

        PROCEDURE ClearLocStack;
BEGIN
   WHILE MomLocHandle<>-1 DO PopLocHandle;
END;

{----------------------------------------------------------------------------}

        FUNCTION LookupReg(VAR Name:String; CreateNew:Boolean):PRegDef;
VAR
   Run,Neu,Prev:PRegDef;
BEGIN
   Prev:=Nil; Run:=FirstRegDef;
   WHILE (Run<>Nil) AND (Run^.Orig^<>Name) DO
    BEGIN
     Prev:=Run;
     IF Run^.Orig^<Name THEN Run:=Run^.Left ELSE Run:=Run^.Right;
    END;
   IF (Run=Nil) AND (CreateNew) THEN
    BEGIN
     New(Neu);
     GetMem(Neu^.Orig,Length(Name)+1); Neu^.Orig^:=Name;
     Neu^.Left:=Nil; Neu^.Right:=Nil;
     Neu^.Defs:=Nil; Neu^.DoneDefs:=Nil;
     IF Prev=Nil THEN FirstRegDef:=Neu
     ELSE IF Prev^.Orig^<Name THEN Prev^.Left:=Neu ELSE Prev^.Right:=Neu;
     LookupReg:=Neu;
    END
   ELSE LookupReg:=Run;
END;

        PROCEDURE AddRegDef(Orig,Repl:String);
VAR
   Node:PRegDef;
   Neu:PRegDefList;
BEGIN
   IF NOT CaseSensitive THEN
    BEGIN
     NLS_UpString(Orig); NLS_UpString(Repl);
    END;
   IF NOT ChkSymbName(Orig) THEN
    BEGIN
     WrXError(1020,Orig); Exit;
    END;
   IF NOT ChkSymbName(Repl) THEN
    BEGIN
     WrXError(1020,Repl); Exit;
    END;
   Node:=LookupReg(Orig,True);
   IF (Node^.Defs<>Nil) AND (Node^.Defs^.Section=MomSectionHandle) THEN
    WrXError(1000,Orig)
   ELSE
    BEGIN
     New(Neu); Neu^.Next:=Node^.Defs; Neu^.Section:=MomSectionHandle;
     GetMem(Neu^.Value,Length(Repl)+1); Neu^.Value^:=Repl;
     Neu^.Used:=False;
     Node^.Defs:=Neu;
    END;
END;

        FUNCTION FindRegDef(Name:String; VAR Erg:StringPtr):Boolean;
VAR
   Sect:LongInt;
   Node:PRegDef;
   Def:PRegDefList;
BEGIN
   FindRegDef:=False;
   IF Name[1]='[' THEN Exit;
   IF NOT GetSymSection(Name,Sect) THEN Exit;
   IF NOT CaseSensitive THEN NLS_UpString(Name);
   Node:=LookupReg(Name,False);
   IF Node=Nil THEN Exit;
   Def:=Node^.Defs;
   IF Sect<>-2 THEN
    WHILE (Def<>Nil) AND (Def^.Section<>Sect) DO Def:=Def^.Next;
   IF Def=Nil THEN Exit
   ELSE
    BEGIN
     Erg:=Def^.Value; Def^.Used:=True; FindRegDef:=True;
    END;
END;


        PROCEDURE TossRegDefs(Sect:LongInt);

        PROCEDURE TossSingle(Node:PRegDef);
VAR
   Tmp:PRegDefList;
BEGIN
   IF Node=Nil THEN Exit; ChkStack;

   IF (Node^.Defs<>Nil) AND (Node^.Defs^.Section=Sect) THEN
    BEGIN
     Tmp:=Node^.Defs; Node^.Defs:=Node^.Defs^.Next;
     Tmp^.Next:=Node^.DoneDefs; Node^.DoneDefs:=Tmp;
    END;

   TossSingle(Node^.Left); TossSingle(Node^.Right);
END;

BEGIN
   TossSingle(FirstRegDef);
END;

        PROCEDURE ClearRegDefList(Start:PRegDefList);
VAR
   Tmp:PRegDefList;
BEGIN
   WHILE Start<>Nil DO
    BEGIN
     Tmp:=Start; Start:=Start^.Next;
     FreeMem(Tmp^.Value,Length(Tmp^.Value^)+1);
     Dispose(Tmp);
    END;
END;

        PROCEDURE CleanupRegDefs;

        PROCEDURE CleanupNode(Node:PRegDef);
BEGIN
   IF Node=Nil THEN Exit; ChkStack;
   ClearRegDefList(Node^.DoneDefs); Node^.DoneDefs:=Nil;
   CleanupNode(Node^.Left); CleanupNode(Node^.Right);
END;

BEGIN
   CleanupNode(FirstRegDef);
END;

        PROCEDURE ClearRegDefs;

        PROCEDURE ClearNode(Node:PRegDef);
BEGIN
   IF Node=Nil THEN Exit; ChkStack;
   ClearRegDefList(Node^.Defs); Node^.Defs:=Nil;
   ClearRegDefList(Node^.DoneDefs); Node^.DoneDefs:=Nil;
   ClearNode(Node^.Left); ClearNode(Node^.Right);
   FreeMem(Node^.Orig,Length(Node^.Orig^)+1);
   Dispose(Node);
END;

BEGIN
   ClearNode(FirstRegDef);
END;

        PROCEDURE PrintRegDefs;
VAR
   buf:String;
   ActPageWidth,cwidth:Integer;
   Sum,USum:Integer;

        PROCEDURE PNode(Node:PRegDef);
VAR
   Lauf:PRegDefList;
   tmp:String;
BEGIN
   Lauf:=Node^.DoneDefs;
   WHILE Lauf<>Nil DO
    BEGIN
     IF Lauf^.Used THEN tmp:=' ' ELSE tmp:='*';
     tmp:=tmp+Node^.Orig^;
     IF Lauf^.Section<>-1 THEN tmp:=tmp+'['+GetSectionName(Lauf^.Section)+']';
     tmp:=tmp+' --> '+Lauf^.Value^;
     IF Length(tmp)>cwidth-3 THEN
      BEGIN
       IF buf<>'' THEN WrLstLine(buf); buf:=''; WrLstLine(tmp);
      END
     ELSE
      BEGIN
       tmp:=tmp+Blanks(cwidth-3-Length(tmp));
       IF buf='' THEN buf:=tmp
       ELSE
        BEGIN
         buf:=buf+' | '+tmp; WrLstLine(buf); buf:='';
        END;
      END;
     Inc(Sum); IF NOT Lauf^.Used THEN Inc(USum);
     Lauf:=Lauf^.Next;
    END;
END;

        PROCEDURE PrintSingle(Node:PRegDef);
BEGIN
   IF Node=Nil THEN Exit; ChkStack;

   PrintSingle(Node^.Left);
   PNode(Node);
   PrintSingle(Node^.Right);
END;

BEGIN
   IF FirstRegDef=Nil THEN Exit;

   NewPage(ChapDepth,True);
   WrLstLine(ListRegDefListHead1);
   WrLstLine(ListRegDefListHead2);
   WrLstLine('');

   buf:=''; Sum:=0; USum:=0;
   ActPageWidth:=PageWidth; IF ActPageWidth=0 THEN ActPageWidth:=80;
   cwidth:=ActPageWidth SHR 1;
   PrintSingle(FirstRegDef);

   IF buf<>'' THEN WrLstLine(buf);
   WrLstLine('');
   Str(Sum:7,buf);
   IF Sum=1 THEN WrLstLine(buf+ListRegDefSumMsg) ELSE WrLstLine(buf+ListRegDefSumsMsg);
   Str(USum:7,buf);
   IF USum=1 THEN WrLstLine(buf+ListRegDefUSumMsg) ELSE WrLstLine(buf+ListRegDefUSumsMsg);
   WrLstLine('');
END;

{----------------------------------------------------------------------------}

        PROCEDURE _Init_AsmPars;
BEGIN
   FirstDefSymbol:=Nil;
   BalanceTree:=False;
END;

BEGIN
   Dec(IntMins[Int32]);
END.
