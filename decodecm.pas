{$i STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}

        Unit DecodeCm;

INTERFACE

	Uses {$IFDEF WINDOWS}
             WinDOS,Strings,
             {$ELSE}
             Dos,
             {$ENDIF}
             NLS;

TYPE
   CMDResult=(CMDErr,CMDFile,CMDOK,CMDArg);

   CMDCallback=FUNCTION(NegFlag:Boolean; Arg:String):CMDResult;

   CMDErrCallback=PROCEDURE(InEnv:Boolean; Arg:String);

   CMDRec=RECORD
	   Ident:String[10];
	   Callback:CMDCallback;
	  END;

   TCMDDef=ARRAY[1..1000] OF CMDRec;
   PCMDDef=^TCMDDef;

   CMDProcessed=SET OF 0..255;


	PROCEDURE ProcessCMD(Def:PCMDDef; Cnt:Integer; VAR Unprocessed:CMDProcessed; EnvName:String; ErrProc:CMDErrCallback);

        FUNCTION GetEXEName:String;

        {$IFDEF WINDOWS}
CONST
   AnyFile=faAnyFile;
   Hidden=faHidden;
   SysFile=faSysFile;
   VolumeID=faVolumeID;
   Directory=faDirectory;
TYPE
   SearchRec=TSearchRec;
   TextRec=TTextRec;

        FUNCTION GetEnv(Name:String):String;

        FUNCTION FSearch(Name,Path:String):String;

        FUNCTION FExpand(Name:String):String;

        PROCEDURE FindFirst(Path:String; Mask:Byte; VAR S:SearchRec);
        {$ENDIF}

Implementation

{$i DECODECM.RSC}

        PROCEDURE SetFileMode(NewMode:Word);
BEGIN
   {$IFDEF OS2}
   FileMode:=64+NewMode;
   {$ELSE}
   FileMode:=NewMode;
   {$ENDIF}
END;

        PROCEDURE ClrBlanks(VAR tmp:String);
VAR
   cnt:Integer;
BEGIN
   cnt:=0;
   WHILE (cnt<Length(tmp)) AND (tmp[cnt+1]=' ') DO Inc(cnt);
   IF cnt<>0 THEN Delete(tmp,1,cnt);
END;

	FUNCTION FirstBlank(VAR tmp:String):Integer;
VAR
   erg:Integer;
BEGIN
   erg:=Pos(' ',tmp);
   IF erg=0 THEN FirstBlank:=Length(tmp)+1 ELSE FirstBlank:=erg;
END;


	PROCEDURE ProcessCMD(Def:PCMDDef; Cnt:Integer; VAR Unprocessed:CMDProcessed; EnvName:String; ErrProc:CMDErrCallback);
VAR
   z:Integer;
   OneLine:String;
   KeyFile:Text;

	FUNCTION  EnvStr(No:Integer):String;
VAR
   tmp:String;
   z:Integer;
BEGIN
   EnvStr:='';
   tmp:=OneLine;
   FOR z:=1 TO No-1 DO
   IF tmp<>'' THEN
    BEGIN
     ClrBlanks(tmp);
     Delete(tmp,1,FirstBlank(tmp));
    END;
   IF tmp<>'' THEN
    BEGIN
     ClrBlanks(tmp);
     EnvStr:=Copy(tmp,1,FirstBlank(tmp)-1);
    END;
END;

	FUNCTION EnvCnt:Integer;
VAR
   z:Integer;
   tmp:String;
BEGIN
   z:=0; tmp:=OneLine;
   ClrBlanks(tmp);
   WHILE tmp<>'' DO
    BEGIN
     Inc(z);
     Delete(tmp,1,FirstBlank(tmp));
     ClrBlanks(tmp);
    END;
   EnvCnt:=z;
END;

	FUNCTION ProcessParam(Param,Next:String):CMDResult;
VAR
   Start:Integer;
   Negate:Boolean;
   z,Search:Integer;
   TempRes:CMDResult;
   s:String;
BEGIN
   IF (Next[1]='/') OR (Next[1]='-') OR (Next[1]='+') THEN Next:='';
   IF (Param[1]='/') OR (Param[1]='-') OR (Param[1]='+') THEN
    BEGIN
     Negate:=Param[1]='+'; Start:=2;

     IF Param[Start]='#' THEN
      BEGIN
       FOR z:=Start+1 TO Length(Param) DO Param[z]:=UpCase(Param[z]);
       Inc(Start);
      END
     ELSE IF Param[Start]='~' THEN
      BEGIN
       FOR z:=Start+1 TO Length(Param) DO
	IF (Param[z]>='A') AND (Param[z]<='Z') THEN
	 Param[z]:=Chr(Ord(Param[z])-Ord('A')+Ord('a'));
       Inc(Start);
      END;

     TempRes:=CMDOK;

     Search:=0;
     s:=Copy(Param,Start,Length(Param)-Start+1);
     FOR z:=1 TO Length(s) DO s[z]:=UpCase(s[z]);
     REPEAT
      Inc(Search);
     UNTIL (Search>Cnt) OR ((Length(Def^[Search].Ident)<>1) AND (s=Def^[Search].Ident));
     IF Search<=Cnt THEN
      TempRes:=Def^[Search].Callback(Negate,Next)

     ELSE
      FOR z:=Start TO Length(Param) DO
       IF TempRes<>CMDErr THEN
        BEGIN
         Search:=0;
         REPEAT
	  Inc(Search);
         UNTIL (Search>Cnt) OR (Def^[Search].Ident=Param[z]);
         IF Search>Cnt THEN
	  TempRes:=CMDErr
         ELSE
	  CASE Def^[Search].Callback(Negate,Next) OF
	  CMDErr:TempRes:=CMDErr;
	  CMDArg:TempRes:=CMDArg;
	  CMDOK:BEGIN END;
	  END;
        END;
     ProcessParam:=TempRes;
    END
   ELSE ProcessParam:=CMDFile;
END;

	PROCEDURE DecodeLine;
VAR
   z:Integer;
BEGIN
   ClrBlanks(OneLine);
   IF (OneLine<>'') AND (OneLine[1]<>';') THEN
    FOR z:=1 TO EnvCnt DO
     CASE ProcessParam(EnvStr(z),EnvStr(z+1)) OF
     CMDErr,CMDFile:ErrProc(True,EnvStr(z));
     CMDArg:Inc(z);
     END;
END;

BEGIN
   OneLine:=GetEnv(EnvName);

   IF OneLine[1]='@' THEN
    BEGIN
     {$i-}
     Delete(OneLine,1,1); ClrBlanks(OneLine);
     Assign(KeyFile,OneLine); SetFileMode(0); Reset(KeyFile);
     IF IoResult<>0 THEN ErrProc(True,ErrMsgKeyFileNotFound);
     WHILE NOT EOF(KeyFile) DO
      BEGIN
       ReadLn(KeyFile,OneLine);
       IF IoResult<>0 THEN ErrProc(True,ErrMsgKeyFileError);
       DecodeLine;
      END;
     Close(KeyFile);
     {$i+}
    END

   ELSE DecodeLine;

   Unprocessed:=[1..ParamCount];
   FOR z:=1 TO ParamCount DO
    IF z IN Unprocessed THEN
     CASE ProcessParam(ParamStr(z),ParamStr(z+1)) OF
     CMDErr:ErrProc(False,ParamStr(z));
     CMDOK:Unprocessed:=Unprocessed-[z];
     CMDArg:Unprocessed:=Unprocessed-[z,z+1];
     END;
END;

        FUNCTION GetEXEName:String;
VAR
   s:String;
BEGIN
   s:=ParamStr(0);
   WHILE Pos('\',s)<>0 DO Delete(s,1,Pos('\',s));
   IF Pos('.',s)<>0 THEN s:=Copy(s,1,Pos('.',s)-1);
   GetEXEName:=s;
END;

{$IFDEF WINDOWS}
        FUNCTION GetEnv(Name:String):String;
VAR
   Tmp:ARRAY[0..255] OF Char;
BEGIN
   StrPCopy(@Tmp,Name);
   GetEnv:=StrPas(GetEnvVar(Tmp));
END;

        FUNCTION FSearch(Name,Path:String):String;
VAR
   Inp,Path2,Tmp:ARRAY[0..255] OF Char;
BEGIN
   StrPCopy(@Inp,Name); StrPCopy(@Path2,Path);
   FileSearch(@Tmp,@Inp,@Path);
   FSearch:=StrPas(Tmp);
END;

        FUNCTION FExpand(Name:String):String;
VAR
   Inp,Erg:ARRAY[0..255] OF Char;
BEGIN
   StrPCopy(@Inp,Name);
   FileExpand(@Erg,@Inp);
   FExpand:=StrPas(Erg);
END;

        PROCEDURE FindFirst(Path:String; Mask:Byte; VAR S:SearchRec);
VAR
   Inp:ARRAY[0..255] OF Char;
BEGIN
   StrPCopy(@Inp,Path);
   WinDOS.FindFirst(@Inp,Mask,S);
END;
{$ENDIF}

END.
