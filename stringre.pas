        UNIT StringRes;

INTERFACE
        USES StdHandl;

TYPE
   ResString=^String;

        PROCEDURE InitRes(AppName:String; Country:Integer);

        PROCEDURE DeinitRes;

        FUNCTION GetString(Number:LongInt):ResString;

IMPLEMENTATION

TYPE
   LIntArray=ARRAY[0..8000] OF LongInt;
VAR
   Initialized:Boolean;
   IndexCnt,StringSize:LongInt;
   Indices:^LIntArray;
   Strings:^Char;

        PROCEDURE InitRes(AppName:String; Country:Integer);
VAR
   f:File;
BEGIN
   {$i-}
   IF Initialized THEN DeinitRes;
   Assign(f,'AS.RES'); SetFileMode(0); Reset(f);
   {$i+}
   IF IoResult<>0 THEN Exit;
   BlockRead(f,IndexCnt,Sizeof(IndexCnt)); IndexCnt:=IndexCnt*2;
   BlockRead(f,StringSize,Sizeof(StringSize));
   GetMem(In
   Initialized:=True;
END;

        PROCEDURE DeinitRes;
BEGIN
   IF Initialized THEN
    BEGIN
     FreeMem(Indices,Sizeof(LongInt)*IndexCnt SHL 1);
     FreeMem(Strings,StringSize);
     Initialized:=False;
    END;
END;

        FUNCTION GetString(Number:LongInt):ResString;
BEGIN
   IF Initialized THEN
    BEGIN
    END
   ELSE GetString:=Nil;
END;

BEGIN
   Initialized:=False;
END.

