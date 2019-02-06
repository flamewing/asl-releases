{$g-,i-,s+,r-,v-}
	PROGRAM ProgToBin;
        Uses Dos,
             StdHandl,Hex,NLS,Chunks,DecodeCM,
             AsmUtils;

CONST
   BinSuffix='.BIN';

TYPE
   ProcessProc=PROCEDURE(FileName:String; Offset:LongInt);

VAR
   ParProcessed:CMDProcessed;
   z,z2:Integer;

   TargFile:File;
   SrcName,TargName:String;

   StartAdr,StopAdr,RealFileLen:LongInt;
   MaxGran,Dummy:LongInt;
   StartAuto,StopAuto:Boolean;

   FillVal:Byte;
   DoCheckSum:Boolean;

   SizeDiv:Byte;
   ANDMask,ANDEq:LongInt;

   UsedList:ChunkList;

{$i IoErrors.rsc}
{$i Tools.rsc}
{$i P2bin.rsc}


        PROCEDURE ParamError(InEnv:Boolean; Arg:String);
        Far;
BEGIN
   IF InEnv THEN Write(ErrMsgInvEnvParam)
            ELSE Write(ErrMsgInvParam);
   WriteLn(Arg);
   WriteLn(ErrMsgProgTerm);
   Halt(1);
END;

        PROCEDURE OpenTarget;
CONST
   BufferSize=4096;
VAR
   Rest,Trans:LongInt;
   Buffer:ARRAY[0..BufferSize-1] OF Byte;
BEGIN
   Assign(TargFile,TargName); SetFileMode(2); Rewrite(TargFile,1);
   ChkIO(TargName); RealFileLen:=((StopAdr-StartAdr+1)*MaxGran) DIV SizeDiv;

   FillChar(Buffer,BufferSize,FillVal);

   Rest:=RealFileLen;
   WHILE Rest<>0 DO
    BEGIN
     Trans:=Min(Rest,BufferSize);
     BlockWrite(TargFile,Buffer,Trans); ChkIO(TargName);
     Dec(Rest,Trans);
    END;
END;

	PROCEDURE CloseTarget;
CONST
   BufferSize=4096;
VAR
   Sum,Rest,Trans,z:LongInt;
   Buffer:ARRAY[0..BufferSize-1] OF Byte;
BEGIN
   IF DoCheckSum THEN
    BEGIN
     Rest:=FileSize(TargFile);
     Seek(TargFile,0); ChkIO(TargName);
     Sum:=0;
     WHILE Rest<>0 DO
      BEGIN
       Trans:=Min(Rest,BufferSize);
       Dec(Rest,Trans);
       BlockRead(TargFile,Buffer,Trans); ChkIO(TargName);
       FOR z:=0 TO Pred(Trans) DO Inc(Sum,Buffer[z]);
       IF Rest=0 THEN Dec(Sum,Buffer[Pred(Trans)]);
      END;
     WriteLn(InfoMessChecksum,HexWord(Sum SHR 16),HexWord(Sum AND $ffff)); ChkIO(OutName);
     Seek(TargFile,FileSize(TargFile)-1); ChkIO(TargName);
     Buffer[0]:=$100-Lo(Sum);
     BlockWrite(TargFile,Buffer,1); ChkIO(TargName);
    END;

   Close(TargFile); ChkIO(TargName);
   IF Magic<>0 THEN Erase(TargFile);
END;

        PROCEDURE ProcessFile(FileName:String; Offset:LongInt);
        Far;
CONST
   BufferSize=4096;
VAR
   SrcFile:File;
   TestID:Word;
   InpHeader,InpSegment:Byte;
   InpStart,SumLen:LongInt;
   InpLen,TransLen,ResLen:Word;
   doit:Boolean;
   Buffer:ARRAY[0..BufferSize-1] OF Byte;
   ErgStart,ErgStop,NextPos:LongInt;
   ErgLen:Word;
   z:Integer;
   Gran:Byte;
BEGIN
   Assign(SrcFile,FileName); SetFileMode(0); Reset(SrcFile,1);
   ChkIO(FileName);

   BlockRead(SrcFile,TestID,2); ChkIO(FileName);
   IF TestID<>FileID THEN FormatError(FileName,FormatInvHeaderMsg);

   Write(FileName,'==>>',TargName); ChkIO(OutName);

   SumLen:=0;

   REPEAT
    ReadRecordHeader(InpHeader,InpSegment,Gran,FileName,SrcFile);
    IF InpHeader=FileHeaderStartAdr THEN
     BEGIN
      BlockRead(SrcFile,ErgStart,SizeOf(ErgStart)); ChkIO(FileName);
     END
    ELSE IF InpHeader<>FileHeaderEnd THEN
     BEGIN
      BlockRead(SrcFile,InpStart,4); ChkIO(FileName);
      BlockRead(SrcFile,InpLen,2); ChkIO(FileName);

      NextPos:=FilePos(SrcFile)+InpLen;
      IF NextPos>=FileSize(SrcFile)-1 THEN
       FormatError(FileName,FormatInvRecordLenMsg);

      doit:=FilterOK(InpHeader) AND (InpSegment=SegCode);

      IF doit THEN
       BEGIN
        Inc(InpStart,Offset);
	ErgStart:=Max(StartAdr,InpStart);
	ErgStop:=Min(StopAdr,InpStart+(InpLen DIV Gran)-1);
	doit:=IsUGreaterEq(ErgStop,ErgStart);
        IF doit THEN
	 BEGIN
	  ErgLen:=(ErgStop+1-ErgStart)*Gran;
          IF AddChunk(UsedList,ErgStart,ErgStop-ErgStart+1,True) THEN
           BEGIN
            WriteLn(' ',ErrMsgOverlap); ChkIO(OutName);
           END;
         END;
       END;

      IF doit THEN
       BEGIN
	{ an Anfang interessierender Daten }

	Seek(SrcFile,FilePos(SrcFile)+(ErgStart-InpStart)*Gran);

	{ in Zieldatei an passende Stelle }

	Seek(TargFile,(ErgStart-StartAdr)*Gran DIV SizeDiv);

	{ umkopieren }

	WHILE ErgLen>0 DO
	 BEGIN
	  TransLen:=Min(BufferSize,ErgLen);
          BlockRead(SrcFile,Buffer,TransLen); ChkIO(FileName);
	  IF SizeDiv=1 THEN ResLen:=TransLen
	  ELSE
	   BEGIN
	    ResLen:=0;
	    FOR z:=0 TO Pred(TransLen) DO
	    IF (ErgStart*Gran+z) AND ANDMask = ANDEq THEN
	     BEGIN
	      Buffer[ResLen]:=Buffer[z]; Inc(ResLen);
	     END;
	   END;
          BlockWrite(TargFile,Buffer,ResLen); ChkIO(TargName);
	  Dec(ErgLen,TransLen); Inc(ErgStart,TransLen); Inc(SumLen,ResLen);
	 END;
       END;
      Seek(SrcFile,NextPos); ChkIO(FileName);
     END;
   UNTIL InpHeader=0;

   WriteLn('  (',SumLen,' Byte)'); ChkIO(OutName);

   Close(SrcFile); ChkIO(FileName);
END;

        PROCEDURE MeasureFile(FileName:String; Offset:LongInt);
	Far;
VAR
   f:File;
   Header,Segment,Gran:Byte;
   Length,TestID:Word;
   Adr,EndAdr,HPos,NextPos:LongInt;
BEGIN
   Assign(f,FileName); SetFileMode(0); Reset(f,1);
   ChkIO(FileName);

   BlockRead(f,TestID,2); ChkIO(FileName);
   IF TestID<>FileMagic THEN FormatError(FileName,FormatInvHeaderMsg);

   REPEAT
    ReadRecordHeader(Header,Segment,Gran,FileName,f);

    IF Header=FileHeaderStartAdr THEN
     BEGIN
      Seek(f,FilePos(f)+SizeOf(LongInt)); ChkIO(FileName);
     END
    ELSE IF Header<>FileHeaderEnd THEN
     BEGIN
      BlockRead(f,Adr,4); ChkIO(FileName);
      BlockRead(f,Length,2); ChkIO(FileName);
      NextPos:=FilePos(f)+Length;
      IF NextPos>FileSize(f) THEN
       FormatError(FileName,FormatInvRecordLenMsg);

      IF FilterOK(Header) AND (Segment=SegCode) THEN
       BEGIN
        Inc(Adr,Offset);
        EndAdr:=Adr+(Length DIV Gran)-1;
        IF Gran>MaxGran THEN MaxGran:=Gran;
	IF StartAuto THEN
	 IF IsUGreater(StartAdr,Adr) THEN StartAdr:=Adr;
	IF StopAuto THEN
	 IF IsUGreater(EndAdr,StopAdr) THEN StopAdr:=EndAdr;
       END;

      Seek(f,NextPos);
     END;

   UNTIL Header=0;

   Close(f);
END;

        PROCEDURE ProcessGroup(GroupName:String; Processor:ProcessProc);
VAR
   s:SearchRec;
   Path,Name,Ext:String;
   Offset:LongInt;
BEGIN
   Ext:=GroupName;
   IF NOT RemoveOffset(GroupName,Offset) THEN ParamError(False,Ext);

   AddSuffix(GroupName,Suffix);

   FSplit(GroupName,Path,Name,Ext);

   FindFirst(GroupName,Archive,s);
   IF DosError<>0 THEN
    BEGIN
     WriteLn(ErrMsgNullMaskA,GroupName,ErrMsgNullMaskB);
     ChkIO(OutName);
    END
   ELSE
    WHILE DosError=0 DO
     BEGIN
      Processor(Path+s.Name,Offset);
      FindNext(s);
     END;
END;

	FUNCTION CMD_AdrRange(Negate:Boolean; Arg:String):CMDResult;
	Far;
VAR
   p,err:Integer;
BEGIN
   IF Negate THEN
    BEGIN
     StartAdr:=0; StopAdr:=$7fff;
     CMD_AdrRange:=CMDOK;
    END
   ELSE
    BEGIN
     CMD_AdrRange:=CMDErr;

     p:=Pos('-',Arg); IF p=0 THEN Exit;

     IF Copy(Arg,1,p-1)='$' THEN
      BEGIN
       StartAuto:=True; err:=0;
      END
     ELSE
      BEGIN
       StartAuto:=False;
       ULongVal(Copy(Arg,1,p-1),StartAdr,err);
      END;
     IF err<>0 THEN Exit;

     IF Copy(Arg,p+1,Length(Arg)-p)='$' THEN
      BEGIN
       StopAuto:=True; err:=0;
      END
     ELSE
      BEGIN
       StopAuto:=False;
       ULongVal(Copy(Arg,p+1,Length(Arg)-p),StopAdr,err);
      END;
     IF err<>0 THEN Exit;

     IF (NOT StartAuto) AND (NOT StopAuto) AND IsUGreater(StartAdr,StopAdr)
     THEN Exit;

     CMD_AdrRange:=CMDArg;
    END;
END;

	FUNCTION CMD_ByteMode(Negate:Boolean; Arg:String):CMDResult;
	Far;
CONST
   ByteModeCnt=9;
   ByteModeStrings:ARRAY[1..ByteModeCnt] OF String=('ALL','EVEN','ODD','BYTE0','BYTE1','BYTE2','BYTE3','WORD0','WORD1');
   ByteModeDivs   :ARRAY[1..ByteModeCnt] OF Byte  =(1,2,2,4,4,4,4,2,2);
   ByteModeMasks  :ARRAY[1..ByteModeCnt] OF Byte  =(0,1,1,3,3,3,3,2,2);
   ByteModeEqs    :ARRAY[1..ByteModeCnt] OF Byte  =(0,0,1,0,1,2,3,0,2);
VAR
   z:Integer;
BEGIN
   IF Arg='' THEN
    BEGIN
     SizeDiv:=1; ANDEq:=0; ANDMask:=0;
     CMD_ByteMode:=CMDOK;
    END
   ELSE
    BEGIN
     FOR z:=1 TO Length(Arg) DO Arg[z]:=UpCase(Arg[z]);
     ANDEq:=$ff;
     FOR z:=1 TO ByteModeCnt DO
     IF Arg=ByteModeStrings[z] THEN
      BEGIN
       SizeDiv:=ByteModeDivs[z];
       ANDMask:=ByteModeMasks[z];
       ANDEq  :=ByteModeEqs[z];
      END;
     IF AndEq=$ff THEN CMD_ByteMode:=CMDErr ELSE CMD_ByteMode:=CMDArg;
    END;
END;

	FUNCTION CMD_FillVal(Negate:Boolean; Arg:String):CMDResult;
	Far;
VAR
   err:Integer;
BEGIN
   Val(Arg,FillVal,err);
   IF err<>0 THEN CMD_FillVal:=CMDErr ELSE CMD_FillVal:=CMDArg;
END;

	FUNCTION CMD_CheckSum(Negate:Boolean; Arg:String):CMDResult;
	Far;
BEGIN
   DoCheckSum:=NOT Negate;
   CMD_CheckSum:=CMDOK;
END;

CONST
   P2BINParamCnt=5;
   P2BINParams:ARRAY[1..P2BINParamCnt] OF CMDRec=
	       ((Ident:'f'; Callback:CMD_FilterList),
		(Ident:'r'; Callback:CMD_AdrRange),
		(Ident:'s'; Callback:CMD_CheckSum),
		(Ident:'m'; Callback:CMD_ByteMode),
		(Ident:'l'; Callback:CMD_FillVal));

BEGIN
   NLS_Initialize; WrCopyRight('P2BIN/2 V1.41r6','P2BIN V1.41r6');

   InitChunk(UsedList);

   IF ParamCount=0 THEN
    BEGIN
     WriteLn(InfoMessHead1,GetEXEName,InfoMessHead2); ChkIO(OutName);
     FOR z:=1 TO InfoMessHelpCnt DO
      BEGIN
       WriteLn(InfoMessHelp[z]); ChkIO(OutName);
      END;
     Halt(1);
    END;

   StartAdr:=0; StopAdr:=$7fff; StartAuto:=False; StopAuto:=False;
   FillVal:=$ff; DoCheckSum:=False; SizeDiv:=1; AndEq:=0;
   ProcessCMD(@P2BINParams,P2BINParamCnt,ParProcessed,'P2BINCMD',ParamError);

   IF ParProcessed=[] THEN
    BEGIN
     WriteLn(ErrMsgTargMissing);
     ChkIO(OutName);
     Halt(1);
    END;

   z:=ParamCount;
   WHILE (z>0) AND (NOT (z IN ParProcessed)) DO Dec(z);
   TargName:=ParamStr(z);
   IF NOT RemoveOffset(TargName,Dummy) THEN ParamError(False,ParamStr(z));
   ParProcessed:=ParProcessed-[z];
   IF ParProcessed=[] THEN
    BEGIN
     SrcName:=ParamStr(z); DelSuffix(TargName);
    END;
   AddSuffix(TargName,BinSuffix);

   MaxGran:=1;
   IF StartAuto OR StopAuto THEN
    BEGIN
     IF StartAuto THEN StartAdr:=-1;
     IF StopAuto THEN StopAdr:=0;
     IF ParProcessed=[] THEN ProcessGroup(SrcName,MeasureFile)
     ELSE FOR z:=1 TO ParamCount DO
      IF z IN ParProcessed THEN ProcessGroup(ParamStr(z),MeasureFile);
     IF IsUGreater(StartAdr,StopAdr) THEN
      BEGIN
       WriteLn(ErrMsgAutoFailed); ChkIO(OutName); Halt(1);
      END;
    END;

   OpenTarget;

   IF ParProcessed=[] THEN ProcessGroup(SrcName,ProcessFile)
   ELSE FOR z:=1 TO ParamCount DO
    IF z IN ParProcessed THEN ProcessGroup(ParamStr(z),ProcessFile);


   CloseTarget;
END.
