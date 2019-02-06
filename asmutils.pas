{$g-,i-,v-,r-,n+}
	UNIT AsmUtils;

INTERFACE

        USES DecodeCm,StdHandl;

	PROCEDURE WrCopyRight(OS2Msg,DOSMsg:String);

        PROCEDURE DelSuffix(VAR s:String);

        PROCEDURE AddSuffix(VAR s:String; Suff:String);

	PROCEDURE FormatError(VAR Name:String; Detail:String);

	PROCEDURE ChkIO(VAR Name:String);

	FUNCTION  Granularity(Header:Byte):Word;

        PROCEDURE ReadRecordHeader(VAR Header,Segment,Gran:Byte;
                                   VAR Name:String; VAR f:File);

        PROCEDURE WriteRecordHeader(VAR Header,Segment,Gran:Byte;
                                    VAR Name:String; VAR f:File);

        FUNCTION  CMD_FilterList(Negate:Boolean; Arg:String):CMDResult;

	FUNCTION  FilterOK(Header:Byte):Boolean;

	FUNCTION  IsUGreater(x1,x2:LongInt):Boolean;

	FUNCTION  IsUGreaterEq(x1,x2:LongInt):Boolean;

	FUNCTION  Min(x1,x2:LongInt):LongInt;

        FUNCTION  Max(x1,x2:LongInt):LongInt;

        PROCEDURE ULongVal(inp:String; VAR Erg:LongInt; VAR Err:Integer);

        FUNCTION RemoveOffset(VAR Name:String; VAR Offset:LongInt):Boolean;

CONST
   Magic:LongInt=$1b342b4d;

{$i FILEFORM.INC}

IMPLEMENTATION

{$i IoErrors.rsc}
{$i Tools.rsc}

VAR
   DoFilter:Boolean;
   FilterCnt:Integer;
   FilterBytes:ARRAY[1..100] OF Byte;


CONST
   InfoMessCopyright:String[50]='(C) 1992,1997 Alfred Arnold';

	PROCEDURE WrCopyRight(OS2Msg,DOSMsg:String);
BEGIN
   {$IFDEF OS2}
   WriteLn(OS2Msg);
   {$ELSE}
   WriteLn(DOSMsg);
   {$ENDIF}
   WriteLn(InfoMessCopyRight);
END;

        PROCEDURE DelSuffix(VAR s:String);
VAR
   z:Integer;
BEGIN
   z:=Length(s);
   WHILE (z>=1) AND (s[z]<>':') AND (s[z]<>'\') AND (s[z]<>'/') AND (s[z]<>'.') DO Dec(z);
   IF (z>=1) AND (s[z]='.') AND (z<>Length(s)) THEN s:=Copy(s,1,z-1);
END;

	PROCEDURE AddSuffix(VAR s:String; Suff:String);
VAR
   p:Byte;
   Part:String;
BEGIN
   p:=Pos('\',s);
   IF p<>0 THEN Part:=Copy(s,p+1,Length(s)-p) ELSE Part:=s;
   IF Pos('.',Part)=0 THEN s:=s+Suff;
END;

	PROCEDURE FormatError(VAR Name:String; Detail:String);
BEGIN
   WriteLn(StdErr,FormatErr1aMsg,Name,FormatErr1bMsg,' (',Detail,')');
   WriteLn(StdErr,FormatErr2Msg);
   Halt(3);
END;

	PROCEDURE ChkIO(VAR Name:String);
VAR
   io:Integer;
BEGIN
   io:=IOResult;

   IF io=0 THEN Exit;

   WriteLn(StdErr,IOErrAHeaderMsg,Name,IOErrBHeaderMsg);

   CASE io OF
     2 : Write(StdErr,IoErrFileNotFound);
     3 : Write(StdErr,IoErrPathNotFound);
     4 : Write(StdErr,IoErrTooManyOpenFiles);
     5 : Write(StdErr,IoErrAccessDenied);
     6 : Write(StdErr,IoErrInvHandle);
    12 : Write(StdErr,IoErrInvAccMode);
    15 : Write(StdErr,IoErrInvDriveLetter);
    16 : Write(StdErr,IoErrCannotRemoveActDir);
    17 : Write(StdErr,IoErrNoRenameAcrossDrives);

   100 : Write(StdErr,IoErrFileEnd);
   101 : Write(StdErr,IoErrDiskFull);
   102 : Write(StdErr,IoErrMissingAssign);
   103 : Write(StdErr,IoErrFileNotOpen);
   104 : Write(StdErr,IoErrNotOpenForRead);
   105 : Write(StdErr,IoErrNotOpenForWrite);
   106 : Write(StdErr,IoErrInvNumFormat);

   150 : Write(StdErr,IoErrWriteProtect);
   151 : Write(StdErr,IoErrUnknownDevice);
   152 : Write(StdErr,IoErrDrvNotReady);
   153 : Write(StdErr,IoErrUnknownDOSFunc);
   154 : Write(StdErr,IoErrCRCError);
   155 : Write(StdErr,IoErrInvDPB);
   156 : Write(StdErr,IoErrPositionErr);
   157 : Write(StdErr,IoErrInvSecFormat);
   158 : Write(StdErr,IoErrSectorNotFound);
   159 : Write(StdErr,IoErrPaperEnd);
   160 : Write(StdErr,IoErrDevReadError);
   161 : Write(StdErr,IoErrDevWriteError);
   162 : Write(StdErr,IoErrGenFailure);

   ELSE WriteLn(StdErr,IoErrUnknown,io);

   END;
   WriteLn(StdErr,'.');

   WriteLn(StdErr,ErrMsgTerminating);

   Halt(2);
END;

	FUNCTION Granularity(Header:Byte):Word;
BEGIN
   CASE Header OF
   $09,$76:Granularity:=4;
   $70,$71,$72,$74,$75,$77,$12,$3b:Granularity:=2
   ELSE Granularity:=1;
   END;
END;

        PROCEDURE ReadRecordHeader(VAR Header,Segment,Gran:Byte;
                                   VAR Name:String; VAR f:File);
BEGIN
    BlockRead(f,Header,1); ChkIO(Name);
    IF (Header<>FileHeaderEnd) AND (Header<>FileHeaderStartAdr) THEN
     IF Header=FileHeaderDataRec THEN
      BEGIN
       BlockRead(f,Header,1); ChkIO(Name);
       BlockRead(f,Segment,1); ChkIO(Name);
       BlockRead(f,Gran,1); ChkIO(Name);
      END
     ELSE
      BEGIN
       Segment:=SegCode;
       Gran:=Granularity(Header);
      END;
END;

        PROCEDURE WriteRecordHeader(VAR Header,Segment,Gran:Byte;
                                    VAR Name:String; VAR f:File);
VAR
   h:Byte;
BEGIN
   IF (Header=FileHeaderEnd) OR (Header=FileHeaderStartAdr) THEN
    BEGIN
     BlockWrite(f,Header,1); ChkIO(Name);
    END
   ELSE IF (Segment<>SegCode) OR (Gran<>Granularity(Header)) THEN
    BEGIN
     h:=FileHeaderDataRec;
     BlockWrite(f,h,1); ChkIO(Name);
     BlockWrite(f,Header,1); ChkIO(Name);
     BlockWrite(f,Segment,1); ChkIO(Name);
     BlockWrite(f,Gran,1); ChkIO(Name);
    END
   ELSE
    BEGIN
     BlockWrite(f,Header,1); ChkIO(Name);
    END;
END;

        FUNCTION CMD_FilterList(Negate:Boolean; Arg:String):CMDResult;
VAR
   FTemp:Byte;
   err,p:Integer;
   Search:Integer;
BEGIN
   CMD_FilterList:=CMDErr;

   IF Arg='' THEN Exit;

   REPEAT
    p:=Pos(',',Arg); IF p=0 THEN p:=1+Length(Arg);
    Val(Copy(Arg,1,p-1),FTemp,err);
    IF err<>0 THEN Exit;
    Search:=0;
    REPEAT
     Inc(Search);
    UNTIL (Search>FilterCnt) OR (FilterBytes[Search]=FTemp);
    IF (Negate) AND (Search<=FilterCnt) THEN
     BEGIN
      FilterBytes[Search]:=FilterBytes[FilterCnt];
      Dec(FilterCnt);
     END
    ELSE IF (NOT Negate) AND (Search>FilterCnt) THEN
     BEGIN
      Inc(FilterCnt);
      FilterBytes[FilterCnt]:=FTemp;
     END;
    Delete(Arg,1,p);
   UNTIL Arg='';

   CMD_FilterList:=CMDArg;

   DoFilter:=FilterCnt<>0;
END;

	FUNCTION FilterOK(Header:Byte):Boolean;
VAR
   z:Integer;
BEGIN
   IF DoFilter THEN
    BEGIN
     FilterOK:=False;
     FOR z:=1 TO FilterCnt DO
      IF Header=FilterBytes[z] THEN FilterOK:=True;
    END
   ELSE FilterOK:=True;
END;

	FUNCTION IsUGreater(x1,x2:LongInt):Boolean;
BEGIN
   IF x1>=0 THEN
    IF x2>=0 THEN IsUGreater:=x1>x2
    ELSE IsUGreater:=False
   ELSE
    IF x2>=0 THEN IsUGreater:=True
    ELSE IsUGreater:=x1>x2;
END;

	FUNCTION IsUGreaterEq(x1,x2:LongInt):Boolean;
BEGIN
   IsUGreaterEq:=(x1=x2) OR (IsUGreater(x1,x2));
END;

	FUNCTION Min(x1,x2:LongInt):LongInt;
BEGIN
   IF IsUGreater(x1,x2) THEN Min:=x2 ELSE Min:=x1;
END;

	FUNCTION Max(x1,x2:LongInt):LongInt;
BEGIN
   IF IsUGreater(x1,x2) THEN Max:=x1 ELSE Max:=x2;
END;

        PROCEDURE ULongVal(inp:String; VAR Erg:LongInt; VAR Err:Integer);
CONST
   HexVals='0123456789ABCDEF';
VAR
   Base:LongInt;
   Start,z,p:Integer;
BEGIN
   IF (Length(inp)>0) AND (inp[1]='$') THEN
    BEGIN
     Start:=2; Base:=16;
    END
   ELSE
    BEGIN
     Start:=1; Base:=10;
    END;
   Erg:=0;
   FOR z:=Start TO Length(inp) DO
    IF inp[z]<>' ' THEN
     BEGIN
      p:=Pos(UpCase(inp[z]),HexVals);
      IF (p=0) OR (p>Base) THEN
       BEGIN
        Err:=p; Exit;
       END;
      Erg:=Erg*Base+p-1;
     END;
    Err:=0;
END;

        FUNCTION RemoveOffset(VAR Name:String; VAR Offset:LongInt):Boolean;
VAR
   z,Nest,err:Integer;
BEGIN
   RemoveOffset:=True; Offset:=0;
   IF Name[Length(Name)]=')' THEN
    BEGIN
     z:=Length(Name)-1; Nest:=0;
     WHILE (z>0) AND (Nest>=0) DO
      BEGIN
       CASE Name[z] OF
       '(':Dec(Nest);
       ')':Inc(Nest);
       END;
       IF Nest<>-1 THEN Dec(z);
      END;
     IF Nest<>-1 THEN RemoveOffset:=False
     ELSE
      BEGIN
       Val(Copy(Name,z+1,Length(Name)-z-1),Offset,err);
       Delete(Name,z,Length(Name)-z+1);
      END;
    END;
END;

VAR
   z:Word;
   XORVal:LongInt;
BEGIN
   FilterCnt:=0; DoFilter:=False;
   FOR z:=1 TO Length(InfoMessCopyRight) DO
    BEGIN
     XORVal:=Ord(InfoMessCopyRight[z]);
     XORVal:=XORVal SHL ((z MOD 4)*8);
     Magic:=Magic XOR XORVal;
    END;
END.
