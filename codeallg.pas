{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
	UNIT CodeAllg;

INTERFACE

        USES Dos,NLS,StdHandl,StringUt,Chunks,
             AsmDef,AsmSub,AsmPars,AsmMac,AsmCode,CodePseu;


        PROCEDURE SetCPU(NewCPU:CPUVar; NoPrev:Boolean);

	FUNCTION  CodeGlobalPseudo:Boolean;


IMPLEMENTATION

CONST
   NullS:String[1]='';

	PROCEDURE SetCPU(NewCPU:CPUVar; NoPrev:Boolean);
VAR
   HCPU:LongInt;
   z,dest:Integer;
   ECPU:ValErgType;
   s:String[10];
   Lauf:PCPUDef;
BEGIN
   Lauf:=FirstCPUDef;
   WHILE (Lauf<>Nil) AND (Lauf^.Number<>NewCPU) DO Lauf:=Lauf^.Next;
   IF Lauf=Nil THEN Exit;

   MomCPUIdent:=Lauf^.Name;
   MomCPU:=Lauf^.Orig;
   MomVirtCPU:=Lauf^.Number;
   s:=MomCPUIdent; dest:=0;
   FOR z:=1 TO Length(s) DO
    IF s[z] IN ['0'..'9','A'..'F'] THEN
     BEGIN
      Inc(dest); s[dest]:=s[z];
     END;
   Byte(s[0]):=dest;
   z:=0; WHILE (z<Length(s)-1) AND (NOT (s[z+1] IN ['0'..'9'])) DO Inc(z);
   IF z>0 THEN Delete(s,1,z);
   Val('$'+s,HCPU,ECPU);
   IF ParamCount<>0 THEN
    BEGIN
     EnterIntSymbol(MomCPUName,HCPU,SegNone,True);
     EnterStringSymbol(MomCPUIdentName,MomCPUIdent,True);
    END;

   InternSymbol:=Default_InternSymbol;
   IF NOT NoPrev THEN SwitchFrom;
   Lauf^.SwitchProc;

   DontPrint:=True;
END;

	FUNCTION IntLine(Inp:LongInt):String;
VAR
   s:String;
BEGIN
   CASE ConstMode OF
   ConstModeIntel:
    BEGIN
     s:=HexString(Inp,0)+'H';
     IF s[1]>'9' THEN IntLine:='0'+s ELSE IntLine:=s;
    END;
   ConstModeMoto:IntLine:='$'+HexString(Inp,0);
   ConstModeC:IntLine:='0x'+HexString(Inp,0);
   END;
END;

	PROCEDURE CodeSECTION;
        Far;
VAR
   Neu:PSaveSection;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE IF ExpandSymbol(ArgStr[1]) THEN
    IF NOT ChkSymbName(ArgStr[1]) THEN WrXError(1020,ArgStr[1])
    ELSE IF (PassNo=1) AND (GetSectionHandle(ArgStr[1],False,MomSectionHandle)<>-2) THEN WrError(1483)
    ELSE
     BEGIN
      New(Neu);
      Neu^.Next:=SectionStack;
      Neu^.Handle:=MomSectionHandle;
      Neu^.LocSyms:=Nil; Neu^.GlobSyms:=Nil; Neu^.ExportSyms:=Nil;
      SetMomSection(GetSectionHandle(ArgStr[1],True,MomSectionHandle));
      SectionStack:=Neu;
     END;
END;

	PROCEDURE CodeENDSECTION;
        Far;

	PROCEDURE ChkEmptList(VAR Root:PForwardSymbol);
VAR
   Tmp:PForwardSymbol;
BEGIN
   WHILE Root<>Nil DO
    BEGIN
     WrXError(1488,Root^.Name^);
     FreeMem(Root^.Name,Length(Root^.Name^)+1);
     Tmp:=Root; Root:=Tmp^.Next; Dispose(Tmp);
    END;
END;

VAR
   Tmp:PSaveSection;
   S:String[10];
BEGIN
   IF ArgCnt>1 THEN WrError(1110)
   ELSE IF SectionStack=Nil THEN WrError(1487)
   ELSE IF (ArgCnt=0) OR (ExpandSymbol(ArgStr[1])) THEN
    IF (ArgCnt=1) AND (GetSectionHandle(ArgStr[1],False,SectionStack^.Handle)<>MomSectionHandle) THEN WrError(1486)
    ELSE
     BEGIN
      Tmp:=SectionStack; SectionStack:=Tmp^.Next;
      ChkEmptList(Tmp^.LocSyms);
      ChkEmptList(Tmp^.GlobSyms);
      ChkEmptList(Tmp^.ExportSyms);
      TossRegDefs(MomSectionHandle);
      IF ArgCnt=0 THEN
       ListLine:='['+GetSectionName(MomSectionHandle)+']';
      SetMomSection(Tmp^.Handle);
      Dispose(Tmp);
     END;
END;

	PROCEDURE CodeCPU;
	Far;
VAR
   Lauf:PCPUDef;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE IF AttrPart<>'' THEN WrError(1100)
   ELSE
    BEGIN
     NLS_UpString(ArgStr[1]);
     Lauf:=FirstCPUDef;
     WHILE (Lauf<>Nil) AND (ArgStr[1]<>Lauf^.Name) DO
      Lauf:=Lauf^.Next;
     IF Lauf=Nil THEN WrXError(1430,ArgStr[1])
     ELSE
      BEGIN
       SetCPU(Lauf^.Number,False); ActPC:=SegCode;
      END;
    END;
END;

	PROCEDURE CodeSETEQU;
	Far;
VAR
   t:TempResult;
   MayChange:Boolean;
   DestSeg:Integer;
BEGIN
   FirstPassUnknown:=False;
   MayChange:=(NOT Memo('EQU')) AND (NOT Memo('='));
   IF AttrPart<>'' THEN WrError(1100)
   ELSE IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
   ELSE
    BEGIN
     EvalExpression(ArgStr[1],t);
     IF NOT FirstPassUnknown THEN
      BEGIN
       IF ArgCnt=1 THEN DestSeg:=SegNone
       ELSE
        BEGIN
         NLS_UpString(ArgStr[2]);
         IF ArgStr[2]='MOMSEGMENT' THEN DestSeg:=ActPC
         ELSE IF ArgStr[2]='' THEN DestSeg:=SegNone
         ELSE
          BEGIN
           DestSeg:=0;
           WHILE (DestSeg<=PCMax) AND
                 (ArgStr[2]<>SegNames[DestSeg]) DO
            Inc(DestSeg);
          END;
        END;
       IF DestSeg>PCMax THEN WrXError(1961,ArgStr[2])
       ELSE
        BEGIN
         SetListLineVal(t);
         PushLocHandle(-1);
         CASE t.Typ OF
         TempInt   :EnterIntSymbol   (LabPart,t.Int,DestSeg,MayChange);
         TempFloat :EnterFloatSymbol (LabPart,t.Float,MayChange);
         TempString:EnterStringSymbol(LabPart,t.Ascii,MayChange);
         END;
         PopLocHandle;
        END;
      END;
    END;
END;

	PROCEDURE CodeORG;
	Far;
VAR
   HVal:LongInt;
   ValOK:Boolean;
BEGIN
   FirstPassUnknown:=False;
   IF AttrPart<>'' THEN WrError(1100)
   ELSE IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     HVal:=EvalIntExpression(ArgStr[1],Int32,ValOK);
     IF FirstPassUnknown THEN WrError(1820);
     IF (ValOK) AND (NOT FirstPassUnknown) THEN
      BEGIN
       PCs[ActPC]:=HVal; DontPrint:=True;
      END;
    END;
END;

	PROCEDURE CodeSHARED;
        Far;
VAR
   z:Integer;
   ValOK:Boolean;
   HVal:LongInt;
   FVal:Extended;
   s,c:String;

   	PROCEDURE BuildComment;
BEGIN
   CASE ShareMode OF
   1:c:='(* '+CommPart+' *)';
   2:c:='/* '+CommPart+' */';
   3:c:='; '+CommPart;
   END;
END;

BEGIN
   IF ShareMode=0 THEN WrError(30)
   ELSE IF (ArgCnt=0) AND (CommPart<>'') THEN
    BEGIN
     {$i-}
     BuildComment; WriteLn(ShareFile,c);
     ChkIO(10004);
     {$i+}
    END
   ELSE
    FOR z:=1 TO ArgCnt DO
     BEGIN
      IF IsSymbolString(ArgStr[z]) THEN
       BEGIN
	ValOK:=GetStringSymbol(ArgStr[z],s);
	IF ShareMode=1 THEN s:=''''+s+''''
	ELSE s:='"'+s+'"';
       END
      ELSE IF IsSymbolFloat(ArgStr[z]) THEN
       BEGIN
	ValOK:=GetFloatSymbol(ArgStr[z],FVal);
	Str(FVal,s);
       END
      ELSE
       BEGIN
	ValOK:=GetIntSymbol(ArgStr[z],HVal);
	CASE ShareMode OF
	1:s:='$'+HexString(HVal,0);
        2:s:='0x'+HexString(HVal,0);
	3:s:=IntLine(HVal);
	END;
       END;
      IF ValOK THEN
       BEGIN
        IF (z=1) AND (CommPart<>'') THEN
         BEGIN
          BuildComment; c:=' '+c;
         END
        ELSE c:='';
	{$i-}
	CASE ShareMode OF
        1:WriteLn(ShareFile,ArgStr[z],' = ',s,';',c);
        2:WriteLn(ShareFile,'#define ',ArgStr[z],' ',s,c);
	3:BEGIN
	   IF IsSymbolChangeable(ArgStr[z]) THEN s:='set '+s ELSE s:='equ '+s;
           WriteLn(ShareFile,ArgStr[z],' ',s,c);
          END;
	END;
        ChkIO(10004);
	{$i+}
       END
      ELSE IF PassNo=1 THEN
       BEGIN
        Repass:=True;
	IF (MsgIfRepass) AND (PassNo>=PassNoForMessage) THEN WrXError(170,ArgStr[z]);
       END;
     END;
END;

	PROCEDURE CodePAGE;
        Far;
VAR
   LVal,WVal:Integer;
   ValOK:Boolean;
BEGIN
   IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
   ELSE IF AttrPart<>'' THEN WrError(1100)
   ELSE
    BEGIN
     LVal:=EvalIntExpression(ArgStr[1],UInt8,ValOK);
     IF ValOK THEN
      BEGIN
       IF (LVal<5) AND (LVal<>0) THEN LVal:=5;
       IF ArgCnt=1 THEN
        BEGIN
         WVal:=0; ValOK:=True;
        END
       ELSE WVal:=EvalIntExpression(ArgStr[2],UInt8,ValOK);
       IF ValOK THEN
        BEGIN
         IF (WVal<5) AND (WVal<>0) THEN WVal:=5;
         PageLength:=LVal; PageWidth:=WVal;
        END;
      END;
    END;
END;

	PROCEDURE CodeNEWPAGE;
        Far;
VAR
   HVal8:ShortInt;
   ValOK:Boolean;
BEGIN
   IF ArgCnt>1 THEN WrError(1110)
   ELSE IF AttrPart<>'' THEN WrError(1100)
   ELSE
    BEGIN
     IF ArgCnt=0 THEN
      BEGIN
       HVal8:=0; ValOK:=True;
      END
     ELSE HVal8:=EvalIntExpression(ArgStr[1],Int8,ValOK);
     IF (ValOK) OR (ArgCnt=0) THEN
      BEGIN
       IF HVal8>ChapMax THEN HVal8:=ChapMax
       ELSE IF HVal8<0 THEN HVal8:=0;
       NewPage(HVal8,True);
      END;
    END;
END;

	PROCEDURE CodeString(VAR erg:String);
	Far;
VAR
   tmp:String;
   asc,z,err:Integer;
   OK:Boolean;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     tmp:=EvalStringExpression(ArgStr[1],OK);
     IF NOT OK THEN WrError(1970) ELSE erg:=tmp;
    END;
END;

	PROCEDURE CodePHASE;
        Far;
VAR
   OK:Boolean;
   HVal:LongInt;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE IF ActPC=StructSeg THEN WrError(1553)
   ELSE
    BEGIN
     HVal:=EvalIntExpression(ArgStr[1],Int32,OK);
     IF OK THEN Phases[ActPC]:=HVal-ProgCounter;
    END;
END;

	PROCEDURE CodeDEPHASE;
        Far;
BEGIN
   IF ArgCnt<>0 THEN WrError(1110)
   ELSE IF ActPC=StructSeg THEN WrError(1553)
   ELSE Phases[ActPC]:=0;
END;

	PROCEDURE CodeWARNING;
        Far;
VAR
   mess:String;
   OK:Boolean;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     mess:=EvalStringExpression(ArgStr[1],OK);
     IF NOT OK THEN WrError(1970)
     ELSE WrErrorString(mess,NullS,True,False);
    END;
END;

	PROCEDURE CodeMESSAGE;
	Far;
VAR
   mess:String;
   OK:Boolean;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     mess:=EvalStringExpression(ArgStr[1],OK);
     IF NOT OK THEN WrError(1970);
     WriteLn(mess,ClrEol);
     IF LstName<>'NUL' THEN WrLstLine(mess);
    END;
END;

	PROCEDURE CodeERROR;
	Far;
VAR
   mess:String;
   OK:Boolean;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     mess:=EvalStringExpression(ArgStr[1],OK);
     IF NOT OK THEN WrError(1970)
     ELSE WrErrorString(mess,NullS,False,False);
    END;
END;

	PROCEDURE CodeFATAL;
	Far;
VAR
   mess:String;
   OK:Boolean;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     mess:=ConstStringVal(ArgStr[1],OK);
     IF NOT OK THEN WrError(1970)
     ELSE WrErrorString(mess,NullS,False,True);
    END;
END;

	PROCEDURE CodeCHARSET;
        Far;
VAR
   w1,w2,w3:Byte;
   ch:Char;
   OK:Boolean;
BEGIN
   IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
   ELSE
    BEGIN
     w1:=EvalIntExpression(ArgStr[1],Int8,OK);
     IF OK THEN
      BEGIN
       w3:=EvalIntExpression(ArgStr[ArgCnt],Int8,OK);
       IF OK THEN
	BEGIN
         IF ArgCnt=2 THEN
          BEGIN
           w2:=w1; OK:=True;
          END
	 ELSE w2:=EvalIntExpression(ArgStr[2],Int8,OK);
         IF OK THEN
          BEGIN
	   IF w1>w2 THEN WrError(1320)
	   ELSE FOR ch:=Chr(w1) TO Chr(w2) DO
	    CharTransTable[ch]:=Chr(Ord(ch)-w1+w3);
	  END;
	END;
      END;
    END;
END;

	PROCEDURE CodeFUNCTION;
	Far;
VAR
   FName:String;
   OK:Boolean;
   z:Integer;
BEGIN
   IF ArgCnt<2 THEN WrError(1110)
   ELSE
    BEGIN
     OK:=True; z:=1;
     REPEAT
      OK:=OK AND ChkMacSymbName(ArgStr[z]);
      IF NOT OK THEN WrXError(1020,ArgStr[z]);
      Inc(z);
     UNTIL (z=ArgCnt) OR (NOT OK);
     IF OK THEN
      BEGIN
       FName:=ArgStr[ArgCnt];
       FOR z:=1 TO ArgCnt-1 DO
	CompressLine(ArgStr[z],z-1,FName);
       EnterFunction(LabPart,FName,ArgCnt-1);
      END;
    END;
END;

	PROCEDURE CodeSAVE;
	Far;
VAR
   Neu:PSaveState;
BEGIN
   IF ArgCnt<>0 THEN WrError(1110)
   ELSE
    BEGIN
     New(Neu);
     WITH Neu^ DO
      BEGIN
       Next:=FirstSaveState;
       SaveCPU:=MomCPU;
       SavePC:=ActPC;
       SaveListOn:=ListOn;
       SaveLstMacroEx:=LstMacroEx;
      END;
     FirstSaveState:=Neu;
    END;
END;

	PROCEDURE CodeRESTORE;
        Far;
VAR
   Old:PSaveState;
BEGIN
   IF ArgCnt<>0 THEN WrError(1110)
   ELSE IF FirstSaveState=Nil THEN WrError(1450)
   ELSE
    BEGIN
     Old:=FirstSaveState; FirstSaveState:=Old^.Next;
     WITH Old^ DO
      BEGIN
       IF SavePC<>ActPC THEN
	BEGIN
	 ActPC:=SavePC; DontPrint:=True;
	END;
       IF SaveCPU<>MomCPU THEN SetCPU(SaveCPU,False);
       ListOn:=SaveListOn; EnterIntSymbol(ListOnName,ListOn,0,True);
       SetFlag(LstMacroEx,LstMacroExName,SaveLstMacroEx);
      END;
     Dispose(Old);
    END;
END;

	PROCEDURE CodeSEGMENT;
        Far;
VAR
   SegZ:Byte;
   Found:Boolean;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     Found:=False; NLS_UpString(ArgStr[1]);
     FOR SegZ:=1 TO PCMax DO
     IF (SegZ IN ValidSegs) AND (ArgStr[1]=SegNames[SegZ]) THEN
      BEGIN
       Found:=True;
       IF ActPC<>SegZ THEN
	BEGIN
	 ActPC:=SegZ;
	 IF NOT PCsUsed[ActPC] THEN PCs[ActPC]:=SegInits[ActPC];
	 PCsUsed[ActPC]:=True;
	 DontPrint:=True;
	END;
      END;
     IF NOT Found THEN WrXError(1961,ArgStr[1]);
    END;
END;

	PROCEDURE CodeLABEL;
	Far;
VAR
   Erg:LongInt;
   OK:Boolean;
BEGIN
   FirstPassUnknown:=False;
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     Erg:=EvalIntExpression(ArgStr[1],Int32,OK);
     IF (OK) AND (NOT FirstPassUnknown) THEN
      BEGIN
       PushLocHandle(-1);
       EnterIntSymbol(LabPart,Erg,SegCode,False);
       ListLine:='='+IntLine(Erg);
       PopLocHandle;
       END;
    END;
END;

	PROCEDURE CodeREAD;
	Far;
VAR
   Exp:String;
   Erg:TempResult;
   OK:Boolean;
   SaveLocHandle:LongInt;
BEGIN
   IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
   ELSE
    BEGIN
     IF ArgCnt=2 THEN Exp:=EvalStringExpression(ArgStr[1],OK)
     ELSE
      BEGIN
       Exp:='READ '+ArgStr[1]+' ? '; OK:=True;
      END;
     IF Ok THEN
      BEGIN
       Write(Exp); ReadLn(Exp); UpString(Exp);
       FirstPassUnknown:=False;
       EvalExpression(Exp,Erg);
       IF OK THEN
	BEGIN
         SaveLocHandle:=MomLocHandle; MomLocHandle:=-1;
	 SetListLineVal(Erg);
	 IF FirstPassUnknown THEN WrError(1820)
	 ELSE CASE Erg.Typ OF
	 TempInt   : EnterIntSymbol(ArgStr[ArgCnt],Erg.Int,SegNone,True);
	 TempFloat : EnterFloatSymbol(ArgStr[ArgCnt],Erg.Float,True);
	 TempString: EnterStringSymbol(ArgStr[ArgCnt],Erg.Ascii,True);
	 END;
         MomLocHandle:=SaveLocHandle;
	END;
      END;
    END;
END;

	PROCEDURE CodeALIGN;
	Far;
VAR
   Dummy:Word;
   OK:Boolean;
   NewPC:LongInt;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE
    BEGIN
     FirstPassUnknown:=False;
     Dummy:=EvalIntExpression(ArgStr[1],Int16,OK);
     IF OK THEN
      IF FirstPassUnknown THEN WrError(1820)
      ELSE
       BEGIN
	NewPC:=ProgCounter+Dummy-1;
	NewPC:=NewPC-NewPC MOD Dummy;
	CodeLen:=NewPC-ProgCounter;
	DontPrint:=CodeLen<>0;
        IF (MakeUseList) AND (DontPrint) THEN
	 IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
       END;
    END;
END;

	PROCEDURE CodeENUM;
	Far;
VAR
   z,p:Integer;
   OK:Boolean;
   Counter,First,Last:LongInt;
   SymPart:String;
BEGIN
   Counter:=0;
   IF ArgCnt=0 THEN WrError(1110)
   ELSE
    FOR z:=1 TO ArgCnt DO
     BEGIN
      p:=QuotPos(ArgStr[z],'=');
      IF p<=Length(ArgStr[z]) THEN
       BEGIN
	SymPart:=Copy(ArgStr[z],p+1,Length(ArgStr[z])-p);
	FirstPassUnknown:=False;
	Counter:=EvalIntExpression(SymPart,Int32,OK);
	IF NOT OK THEN Exit;
	IF FirstPassUnknown THEN
	 BEGIN
	  WrXError(1820,SymPart); Exit;
	 END;
	Byte(ArgStr[z][0]):=p-1;
       END;
      EnterIntSymbol(ArgStr[z],Counter,SegNone,False);
      IF z=1 THEN First:=Counter
      ELSE IF z=ArgCnt THEN Last:=Counter;
      Inc(Counter);
     END;
   ListLine:='='+IntLine(First);
   IF ArgCnt<>1 THEN ListLine:=ListLine+'..'+IntLine(Last);
END;

	PROCEDURE CodeEND;
	Far;
VAR
   HVal:LongInt;
   OK:Boolean;
BEGIN
   IF ArgCnt>1 THEN WrError(1110)
   ELSE
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       FirstPassUnknown:=False;
       HVal:=EvalIntExpression(ArgStr[1],Int32,OK);
       IF OK AND (NOT FirstPassUnknown) THEN
        BEGIN
         ChkSpace(SegCode);
         StartAdr:=HVal;
         StartAdrPresent:=True;
        END;
      END;
     ENDOccured:=True;
    END;
END;

        PROCEDURE CodeLISTING;
        Far;
VAR
   Value:Byte;
   OK:Boolean;
BEGIN
   IF ArgCnt<>1 THEN WrError(1110)
   ELSE IF AttrPart<>'' THEN WrError(1100)
   ELSE
    BEGIN
     OK:=True; NLS_UpString(ArgStr[1]);
     IF ArgStr[1]='OFF' THEN Value:=0
     ELSE IF ArgStr[1]='ON' THEN Value:=1
     ELSE IF ArgStr[1]='NOSKIPPED' THEN Value:=2
     ELSE IF ArgStr[1]='PURECODE' THEN Value:=3
     ELSE OK:=False;
     IF NOT OK THEN WrError(1520)
     ELSE
      BEGIN
       ListOn:=Value;
       EnterIntSymbol(ListOnName,ListOn,0,True);
      END;
    END;
END;

        PROCEDURE CodeBINCLUDE;
        Far;
VAR
   F:File;
   Ofs,Len,Curr,Rest:LongInt;
   RLen:Word;
   OK,SaveTurnWords:Boolean;
   Name:String;
BEGIN
   IF (ArgCnt<1) OR (ArgCnt>3) THEN WrError(1110)
   ELSE IF ActPC<>SegCode THEN WrError(1940)
   ELSE
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       OK:=True; Ofs:=0; Len:=-1;
      END
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Ofs:=EvalIntExpression(ArgStr[2],Int32,OK);
       IF FirstPassUnknown THEN
        BEGIN
         WrError(1820); OK:=False;
        END;
        IF OK THEN
        IF ArgCnt=2 THEN Len:=-1
        ELSE
         BEGIN
          Len:=EvalIntExpression(ArgStr[3],Int32,OK);
          IF FirstPassUnknown THEN
           BEGIN
            WrError(1820); OK:=False;
           END;
         END;
      END;
     IF OK THEN
      BEGIN
       Name:=ArgStr[1];
       IF Name[1]='"' THEN Delete(Name,1,1);
       IF Name[Length(Name)]='"' THEN Dec(Byte(Name[0]));
       UpString(Name); ArgStr[1]:=Name;
       Name:=FExpand(FSearch(Name,IncludeList));
       IF Name[Length(Name)]='\' THEN Name:=Name+ArgStr[1];
       {$i-}
       SetFileMode(0); Assign(F,Name); Reset(F,1); ChkIO(10001);
       IF Len=-1 THEN
        BEGIN
         Len:=FileSize(F)-Ofs; ChkIO(10003);
         IF Len<0 THEN
          BEGIN
           WrError(1600); Close(F); Exit;
          END;
        END;
       Seek(F,Ofs); ChkIO(10003);
       Rest:=Len; SaveTurnWords:=TurnWords; TurnWords:=False;
       REPEAT
        IF Rest<MaxCodeLen THEN Curr:=Rest ELSE Curr:=MaxCodeLen;
        BlockRead(F,BAsmCode,Curr,RLen); ChkIO(10003);
        CodeLen:=RLen; WriteBytes;
        Dec(Rest,RLen);
       UNTIL (Rest=0) OR (RLen<>Curr);
       IF Rest<>0 THEN WrError(1600);
       TurnWords:=SaveTurnWords;
       DontPrint:=True; CodeLen:=Len-Rest;
       Close(F);
       {$i+}
      END;
    END;
END;

        PROCEDURE CodePUSHV;
        Far;
VAR
   z:Integer;
BEGIN
   IF ArgCnt<2 THEN WrError(1110)
   ELSE
    BEGIN
     IF NOT CaseSensitive THEN NLS_UpString(ArgStr[1]);
     FOR z:=2 TO ArgCnt DO
      IF PushSymbol(ArgStr[z],ArgStr[1]) THEN;
    END;
END;

        PROCEDURE CodePOPV;
        Far;
VAR
   z:Integer;
BEGIN
   IF ArgCnt<2 THEN WrError(1110)
   ELSE
    BEGIN
     IF NOT CaseSensitive THEN NLS_UpString(ArgStr[1]);
     FOR z:=2 TO ArgCnt DO
      IF PopSymbol(ArgStr[z],ArgStr[1]) THEN;
    END;
END;

        PROCEDURE CodeSTRUCT;
        Far;
VAR
   NStruct:PStructure;
   z:Integer;
   OK:Boolean;
BEGIN
   IF ArgCnt>1 THEN WrError(1110)
   ELSE IF NOT ChkSymbName(LabPart) THEN WrXError(1020,LabPart)
   ELSE
    BEGIN
     IF NOT CaseSensitive THEN NLS_UpString(LabPart);
     IF StructureStack<>Nil THEN StructureStack^.CurrPC:=ProgCounter;
     New(NStruct);
     WITH NStruct^ DO
      BEGIN
       GetMem(Name,Length(LabPart)+1); Name^:=LabPart;
       CurrPC:=0; DoExt:=True;
       Next:=StructureStack;
      END;
     OK:=True;
     FOR z:=1 TO ArgCnt DO
      IF OK THEN
       IF NLS_StrCasecmp(ArgStr[z],'EXTNAMES')=0 THEN NStruct^.DoExt:=True
       ELSE IF NLS_StrCaseCmp(ArgStr[z],'NOEXTNAMES')=0 THEN NStruct^.DoExt:=False
       ELSE
        BEGIN
         WrXError(1554,ArgStr[z]); OK:=False;
        END;
     IF OK THEN
      BEGIN
       StructureStack:=NStruct;
       IF ActPC<>StructSeg THEN StructSaveSeg:=ActPC; ActPC:=StructSeg;
       PCs[ActPC]:=0;
       Phases[ActPC]:=0;
       Grans[ActPC]:=Grans[SegCode]; ListGrans[ActPC]:=ListGrans[SegCode];
       ClearChunk(SegChunks[StructSeg]);
       CodeLen:=0; DontPrint:=True;
      END
     ELSE
      BEGIN
       FreeMem(NStruct^.Name,Length(NStruct^.Name^)+1); Dispose(NStruct);
      END;
    END;
END;

        PROCEDURE CodeENDSTRUCT;
        Far;
VAR
   OK:Boolean;
   OStruct:PStructure;
   t:TempResult;
   tmp:String;
BEGIN
   IF ArgCnt>1 THEN WrError(1110)
   ELSE IF StructureStack=Nil THEN WrError(1550)
   ELSE
    BEGIN
     IF LabPart='' THEN OK:=True
     ELSE
      BEGIN
       IF NOT CaseSensitive THEN NLS_UpString(LabPart);
       OK:=LabPart=StructureStack^.Name^;
       IF NOT OK THEN WrError(1552);
      END;
     IF OK THEN
      BEGIN
       OStruct:=StructureStack; StructureStack:=OStruct^.Next;
       IF ArgCnt=0 THEN tmp:=OStruct^.Name^+'_len'
       ELSE tmp:=ArgStr[1];
       EnterIntSymbol(tmp,ProgCounter,SegNone,False);
       t.Typ:=TempInt; t.Int:=ProgCounter; SetListLineVal(t);
       FreeMem(OStruct^.Name,Length(OStruct^.Name^)+1);
       Dispose(OStruct);
       IF StructureStack=Nil THEN ActPC:=StructSaveSeg
       ELSE Inc(PCs[ActPC],StructureStack^.CurrPC);
       ClearChunk(SegChunks[StructSeg]);
       CodeLen:=0; DontPrint:=True;
      END;
    END;
END;

        PROCEDURE CodePPSyms(VAR Orig,Alt1,Alt2:PForwardSymbol);
VAR
   Lauf:PForwardSymbol;
   SLauf:PSaveSection;
   z:Integer;
   Sym,Section:String;

	PROCEDURE SearchSym(Root:PForwardSymbol; VAR Comp:String);
BEGIN
   Lauf:=Root;
   WHILE (Lauf<>Nil) AND (Lauf^.Name^<>Comp) DO Lauf:=Lauf^.Next;
END;

BEGIN
   IF ArgCnt=0 THEN WrError(1110)
   ELSE
    FOR z:=1 TO ArgCnt DO
     BEGIN
      SplitString(ArgStr[z],Sym,Section,QuotPos(ArgStr[z],':'));
      IF NOT ExpandSymbol(Sym) THEN Exit;
      IF NOT ExpandSymbol(Section) THEN Exit;
      IF NOT CaseSensitive THEN NLS_UpString(Sym);
      SearchSym(Alt1,Sym);
      IF Lauf<>Nil THEN WrXError(1489,ArgStr[z])
      ELSE
       BEGIN
	SearchSym(Alt2,Sym);
	IF Lauf<>Nil THEN WrXError(1489,ArgStr[z])
	ELSE
	 BEGIN
	  SearchSym(Orig,Sym);
	  IF Lauf=Nil THEN
	   BEGIN
	    New(Lauf); Lauf^.Next:=Orig; Orig:=Lauf;
	    GetMem(Lauf^.Name,Length(Sym)+1); Lauf^.Name^:=Sym;
	   END;
	  IF NOT IdentifySection(Section,Lauf^.DestSection) THEN;
	 END;
       END;
     END;
END;

	FUNCTION CodeGlobalPseudo:Boolean;
TYPE
   PseudoOrder=RECORD
		Name:String[10];
		Proc:PROCEDURE;
	       END;
   PseudoStrOrder=RECORD
		   Name:String[10];
                   p:StringPtr;
		  END;
CONST
   PseudoCnt=29;
   Pseudos:ARRAY[1..PseudoCnt] OF PseudoOrder=
           ((Name:'ALIGN';      Proc:CodeALIGN     ),
            (Name:'BINCLUDE';   Proc:CodeBINCLUDE  ),
            (Name:'CHARSET';    Proc:CodeCHARSET   ),
            (Name:'CPU';        Proc:CodeCPU       ),
            (Name:'DEPHASE';    Proc:CodeDEPHASE   ),
            (Name:'END';        Proc:CodeEND       ),
            (Name:'ENDSECTION'; Proc:CodeENDSECTION),
            (Name:'ENDSTRUCT';  Proc:CodeENDSTRUCT ),
            (Name:'ENUM';       Proc:CodeENUM      ),
            (Name:'ERROR';      Proc:CodeERROR     ),
            (Name:'FATAL';      Proc:CodeFATAL     ),
            (Name:'FUNCTION';   Proc:CodeFUNCTION  ),
            (Name:'LABEL';      Proc:CodeLABEL     ),
            (Name:'LISTING';    Proc:CodeLISTING   ),
            (Name:'MESSAGE';    Proc:CodeMESSAGE   ),
            (Name:'NEWPAGE';    Proc:CodeNEWPAGE   ),
            (Name:'ORG';        Proc:CodeORG       ),
            (Name:'PAGE';       Proc:CodePAGE      ),
            (Name:'PHASE';      Proc:CodePHASE     ),
            (Name:'POPV';       Proc:CodePOPV      ),
            (Name:'PUSHV';      Proc:CodePUSHV     ),
            (Name:'READ';       Proc:CodeREAD      ),
            (Name:'RESTORE';    Proc:CodeRESTORE   ),
            (Name:'SAVE';       Proc:CodeSAVE      ),
            (Name:'SECTION';    Proc:CodeSECTION   ),
            (Name:'SEGMENT';    Proc:CodeSEGMENT   ),
            (Name:'SHARED';     Proc:CodeSHARED    ),
            (Name:'STRUCT';     Proc:CodeSTRUCT    ),
            (Name:'WARNING';    Proc:CodeWARNING   ));


   PseudoStrCnt=3;
   PseudoStrs:ARRAY[1..PseudoStrCnt] OF PseudoStrOrder=
	   ((Name:'PRTINIT';    p:@PrtInitString),
	    (Name:'PRTEXIT';    p:@PrtExitString),
	    (Name:'TITLE';      p:@PrtTitleString));

   ONOFFAllgCount=2;
   ONOFFAllgs:ARRAY[1..ONOFFAllgCount] OF ONOFFRec=
              ((Name:'MACEXP' ; Dest:@LstMacroEx;  FlagName:LstMacroExName),
               (Name:'RELAXED'; Dest:@RelaxedMode; FlagName:RelaxedName));

VAR
   z:Integer;
BEGIN
   CodeGlobalPseudo:=True;

   IF Memo('EQU')
   OR Memo('=')
   OR Memo(':=')
   OR ((NOT SETIsOccupied) AND (Memo('SET')))
   OR ((SetIsOccupied) AND (Memo('EVAL'))) THEN
    BEGIN
     CodeSETEQU; Exit;
    END;

   IF CodeONOFF(@ONOFFAllgs,ONOFFAllgCount) THEN Exit;

   z:=1;
   WHILE (z<=PseudoCnt) AND (Pseudos[z].Name<=OpPart) DO
    BEGIN
     IF Pseudos[z].Name=OpPart THEN
      BEGIN
       Pseudos[z].Proc; Exit;
      END;
     Inc(z);
    END;

   FOR z:=1 TO PseudoStrCnt DO
    WITH PseudoStrs[z] DO
     IF Memo(Name) THEN
      BEGIN
       CodeString(p^); Exit;
      END;

   IF SectionStack<>Nil THEN
    BEGIN
     IF Memo('FORWARD') THEN
      BEGIN
       IF PassNo<=MaxSymPass THEN
	CodePPSyms(SectionStack^.LocSyms,
		   SectionStack^.GlobSyms,
		   SectionStack^.ExportSyms);
       Exit;
      END;
     IF Memo('PUBLIC') THEN
      BEGIN
       CodePPSyms(SectionStack^.GlobSyms,
		  SectionStack^.LocSyms,
		  SectionStack^.ExportSyms);
       Exit;
      END;
     IF Memo('GLOBAL') THEN
      BEGIN
       CodePPSyms(SectionStack^.ExportSyms,
		  SectionStack^.LocSyms,
		  SectionStack^.GlobSyms);
       Exit;
      END;
    END;

   CodeGlobalPseudo:=False;
END;

END.
