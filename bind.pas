{$g-,i-,v-}
	PROGRAM Bind;
        Uses Dos,StdHandl,DecodeCm,AsmUtils,NLS;

CONST
   {$IFDEF OS2}
   Creator:ARRAY[1..14] OF Char='BIND/2 1.41r7';
   {$ELSE}
   Creator:ARRAY[1..12] OF Char='BIND 1.41r7';
   {$ENDIF}

VAR
   ParProcessed:CMDProcessed;
   z,z2:Integer;

   TargFile:File;
   TargName:String;

{$i Tools.rsc}
{$i Bind.rsc}

	PROCEDURE OpenTarget;
BEGIN
   Assign(TargFile,TargName); SetFileMode(2); Rewrite(TargFile,1);
   ChkIO(TargName);
   BlockWrite(TargFile,FileID,2);
   ChkIO(TargName);
END;

	PROCEDURE CloseTarget;
CONST
   EndHeader:Byte=FileHeaderEnd;
BEGIN
   BlockWrite(TargFile,EndHeader,1); ChkIO(TargName);
   BlockWrite(TargFile,Creator,Sizeof(Creator)); ChkIO(TargName);
   Close(TargFile); ChkIO(TargName);
   IF Magic<>0 THEN Erase(TargFile);
END;

	PROCEDURE ProcessFile(FileName:String);
CONST
   BufferSize=8192;
VAR
   SrcFile:File;
   TestID:Word;
   InpHeader,InpSegment,InpGran:Byte;
   InpStart,SumLen:LongInt;
   InpLen,TransLen:Word;
   doit:Boolean;
   Buffer:ARRAY[1..BufferSize] OF Byte;
   z:Integer;
BEGIN
   Assign(SrcFile,FileName); SetFileMode(0); Reset(SrcFile,1);
   ChkIO(FileName);

   BlockRead(SrcFile,TestID,2);
   IF TestID<>FileMagic THEN FormatError(FileName,FormatInvHeaderMsg);

   Write(FileName,'==>>',TargName); ChkIO(OutName);

   SumLen:=0;

   REPEAT
    ReadRecordHeader(InpHeader,InpSegment,InpGran,FileName,SrcFile);
    IF InpHeader=FileHeaderStartAdr THEN
     BEGIN
      BlockRead(SrcFile,InpStart,4); ChkIO(FileName);
      WriteRecordHeader(InpHeader,InpSegment,InpGran,TargName,TargFile);
      BlockWrite(TargFile,InpStart,4); ChkIO(TargName);
     END
    ELSE IF InpHeader<>FileHeaderEnd THEN
     BEGIN
      BlockRead(SrcFile,InpStart,4); ChkIO(FileName);
      BlockRead(SrcFile,InpLen,2); ChkIO(FileName);

      IF FilePos(SrcFile)+InpLen>=FileSize(SrcFile)-1 THEN
       FormatError(FileName,FormatInvRecordLenMsg);

      doit:=FilterOK(InpHeader);

      IF doit THEN
       BEGIN
	Inc(SumLen,InpLen);
        WriteRecordHeader(InpHeader,InpSegment,InpGran,TargName,TargFile);
	BlockWrite(TargFile,InpStart,4); ChkIO(TargName);
	BlockWrite(TargFile,InpLen,2); ChkIO(TargName);
	WHILE InpLen>0 DO
	 BEGIN
	  IF InpLen>BufferSize THEN TransLen:=BufferSize ELSE TransLen:=InpLen;
	  BlockRead(SrcFile,Buffer,TransLen); ChkIO(FileName);
	  BlockWrite(TargFile,Buffer,TransLen); ChkIO(TargName);
	  Dec(InpLen,TransLen);
	 END;
       END
      ELSE
       BEGIN
        Seek(SrcFile,FilePos(SrcFile)+InpLen); ChkIO(FileName);
       END;
     END;
   UNTIL InpHeader=0;

   WriteLn('  (',SumLen,' Byte)'); ChkIO(OutName);

   Close(SrcFile); ChkIO(FileName);
END;

	PROCEDURE ProcessGroup(GroupName:String);
VAR
   s:SearchRec;
   Path,Name,Ext:String;
BEGIN
   AddSuffix(GroupName,Suffix);

   FSplit(GroupName,Path,Name,Ext);

   FindFirst(GroupName,Archive,s);
   WHILE DosError=0 DO
    BEGIN
     ProcessFile(Path+s.Name);
     FindNext(s);
    END;
END;

	PROCEDURE ParamError(InEnv:Boolean; Arg:String);
	Far;
BEGIN
   IF InEnv THEN Write(ErrMsgInvEnvParam)
	    ELSE Write(ErrMsgInvParam);
   WriteLn(Arg);
   WriteLn(ErrMsgProgTerm);
   Halt(1);
END;

CONST
   BINDParamCnt=1;
   BINDParams:ARRAY[1..BINDParamCnt] OF CMDRec=
	      ((Ident:'f'; Callback:CMD_FilterList));

BEGIN
   NLS_Initialize; WrCopyRight('BIND/2 V1.41r7','BIND V1.41r7');

   IF ParamCount=0 THEN
    BEGIN
     WriteLn(InfoMessHead1,GetEXEName,InfoMessHead2); ChkIO(OutName);
     FOR z:=1 TO InfoMessHelpCnt DO
      BEGIN
       WriteLn(InfoMessHelp[z]); ChkIO(OutName);
      END;
     Halt(1);
    END;

   ProcessCMD(@BINDParams,BINDParamCnt,ParProcessed,'BINDCMD',ParamError);

   z:=ParamCount;
   WHILE (z>0) AND (NOT (z IN ParProcessed)) DO Dec(z);
   IF z=0 THEN
    BEGIN
     WriteLn(ErrMsgTargetMissing);
     ChkIO(OutName);
     Halt(1);
    END
   ELSE
    BEGIN
     TargName:=ParamStr(z); ParProcessed:=ParProcessed-[z];
     AddSuffix(TargName,Suffix);
    END;

   OpenTarget;

   FOR z:=1 TO ParamCount DO
    IF z IN ParProcessed THEN ProcessGroup(ParamStr(z));

   CloseTarget;
END.
