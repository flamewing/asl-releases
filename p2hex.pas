{$g-,i-,s+,r-,v-,n+}
	PROGRAM ProgToHex;
        Uses Dos,
             StdHandl,Hex,NLS,Chunks,DecodeCm,
	     AsmUtils;

CONST
   HexSuffix='.HEX';
   MaxLineLen=254;

TYPE
   OutFormat=(Default,MotoS,IntHex,IntHex16,IntHex32,MOSHex,TekHex,TiDSK,Atmel);
   ProcessProc=PROCEDURE(FileName:String; Offset:LongInt);

VAR
   ParProcessed:CMDProcessed;
   z,z2:Integer;

   TargFile:Text;
   SrcName,TargName:String;

   StartAdr,StopAdr,LineLen:LongInt;
   StartData,StopData,EntryAdr:LongInt;
   StartAuto,StopAuto,EntryAdrPresent,AutoErase:Boolean;
   Seg,Ofs:Word;
   Dummy:LongInt;
   IntelMode:Byte;
   MultiMode:Byte;   { 0=8M, 1=16, 2=8L, 3=8H }
   Rec5:Boolean;
   SepMoto:Boolean;

   RelAdr,MotoOccured,IntelOccured,MOSOccured,DSKOccured:Boolean;
   MaxMoto,MaxIntel:Byte;

   DestFormat:OutFormat;

   UsedList:ChunkList;

{$i Tools.rsc}
{$i P2hex.rsc}

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
BEGIN
   Assign(TargFile,TargName); SetFileMode(2); Rewrite(TargFile);
   ChkIO(TargName);
END;

	PROCEDURE CloseTarget;
CONST
   EndHeader:Byte=0;
BEGIN
   Close(TargFile); ChkIO(TargName);
   IF Magic<>0 THEN Erase(TargFile);
END;

        PROCEDURE ProcessFile(FileName:String; Offset:LongInt);
	Far;
VAR
   SrcFile:File;
   TestID:Word;
   InpHeader,InpSegment,InpGran,BSwap:Byte;
   InpStart,SumLen:LongInt;
   InpLen,TransLen:Word;
   doit,FirstBank:Boolean;
   Buffer:ARRAY[0..MaxLineLen-1] OF Byte;
   WBuffer:ARRAY[0..(MaxLineLen DIV 2)-1] OF Word ABSOLUTE Buffer;
   ErgStart,ErgStop,NextPos,IntOffset,MaxAdr:LongInt;
   ErgLen,ChkSum,RecCnt,Gran,SwapBase,HSeg:Word;

   z:Integer;

   MotRecType:Byte;

   ActFormat:OutFormat;

BEGIN
   Assign(SrcFile,FileName); SetFileMode(0); Reset(SrcFile,1);
   ChkIO(FileName);

   BlockRead(SrcFile,TestID,2); ChkIO(FileName);
   IF TestID<>FileID THEN FormatError(FileName,FormatInvHeaderMsg);

   Write(FileName,'==>>',TargName); ChkIO(OutName);

   SumLen:=0;

   REPEAT
    ReadRecordHeader(InpHeader,InpSegment,InpGran,FileName,SrcFile);
    IF InpHeader=FileHeaderStartAdr THEN
     BEGIN
      BlockRead(SrcFile,ErgStart,SizeOf(ErgStart)); ChkIO(FileName);
      IF NOT EntryAdrPresent THEN
       BEGIN
        EntryAdr:=ErgStart; EntryAdrPresent:=True;
       END;
     END
    ELSE IF InpHeader<>0 THEN
     BEGIN
      Gran:=InpGran;
      ActFormat:=DestFormat;

      IF ActFormat=Default THEN
       CASE InpHeader OF
       $01,$05,$09,$52,$56,$61,
       $62,$63,$64,$65,$66,$68,
       $69,$6c:
        ActFormat:=MotoS;
       $12,$21,$31,$32,$33,$39,$3a,$41,
       $48,$49,$4a,$51,$53,$54,$55,$6e,$6f,
       $70,$71,$72,$73,$78,$79,$7a,
       $7b,$7c:
        ActFormat:=IntHex;
       $14,$42,$4c:
        ActFormat:=IntHex16;
       $13,$29,$3c,$76:
        ActFormat:=IntHex32;
       $11,$19:
        ActFormat:=MOSHex;
       $74,$75,$77:
        ActFormat:=TiDSK;
       $3b:
        ActFormat:=Atmel;
       ELSE FormatError(FileName,FormatInvRecordHeaderMsg);
       END;

      CASE ActFormat OF
      MotoS,IntHex32:MaxAdr:=$ffffffff;
      IntHex16:MaxAdr:=$ffff0+$ffff;
      Atmel:MaxAdr:=$ffffff;
      ELSE MaxAdr:=$ffff;
      END;

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

      IF IsUGreater(ErgStop,MaxAdr) THEN
       BEGIN
        WriteLn(' ',ErrMsgAdrOverflow); ChkIO(OutName);
       END;

      IF doit THEN
       BEGIN
	{ an Anfang interessierender Daten }

	Seek(SrcFile,FilePos(SrcFile)+(ErgStart-InpStart)*Gran);

	{ Statistik, Anzahl Datenzeilen ausrechnen }

        RecCnt:=ErgLen DIV LineLen;
        IF ErgLen MOD LineLen<>0 THEN Inc(RecCnt);

	{ relative Angaben ? }

	IF RelAdr THEN Dec(ErgStart,StartAdr);

	{ Kopf einer Datenzeilengruppe }

	CASE ActFormat OF
	MotoS:
	 BEGIN
	  IF (NOT MotoOccured) OR (SepMoto) THEN
	   BEGIN
	    WriteLn(TargFile,'S0030000FC'); ChkIO(TargName);
	   END;
	  IF ErgStop SHR 24<>0 THEN MotRecType:=2
	  ELSE IF ErgStop SHR 16<>0 THEN MotRecType:=1
	  ELSE MotRecType:=0;
          IF MaxMoto<MotRecType THEN MaxMoto:=MotRecType;
	  ChkSum:=Lo(RecCnt)+Hi(RecCnt)+3;
	  IF Rec5 THEN
	   BEGIN
	    WriteLn(TargFile,'S503',HexWord(RecCnt),HexByte(Lo(NOT ChkSum)));
	    ChkIO(TargName);
	   END;
	  MotoOccured:=True;
	 END;
	MOSHex:MOSOccured:=True;
	IntHex:
	 BEGIN
	  IntelOccured:=True;
	  IntOffset:=0;
	 END;
	IntHex16:
	 BEGIN
	  IntelOccured:=True;
	  IntOffset:=ErgStart AND $fffffff0;
	  HSeg:=IntOffset SHR 4; ChkSum:=4+Lo(HSeg)+Hi(HSeg);
	  WriteLn(TargFile,':02000002',HexWord(HSeg),HexByte($100-ChkSum));
          IF MaxIntel<1 THEN MaxIntel:=1;
	  ChkIO(TargName);
	 END;
        IntHex32:
	 BEGIN
	  IntelOccured:=True;
          IntOffset:=ErgStart AND $ffff0000;
          HSeg:=IntOffset SHR 16; ChkSum:=6+Lo(HSeg)+Hi(HSeg);
          WriteLn(TargFile,':02000004',HexWord(HSeg),HexByte($100-ChkSum));
          IF MaxIntel<2 THEN MaxIntel:=2;
	  ChkIO(TargName);
          FirstBank:=False;
         END;
        TekHex:BEGIN END;
        Atmel:BEGIN END;
	TiDSK:
	 IF NOT DSKOccured THEN
	  BEGIN
	   DSKOccured:=True;
	   WriteLn(TargFile,DSKHeaderLine,TargName); ChkIO(TargName);
	  END;
	END;

	{ Datenzeilen selber }

	WHILE ErgLen>0 DO
	 BEGIN
          { evtl. Folgebank fr Intel32 ausgeben }

          IF (ActFormat=IntHex32) AND (FirstBank) THEN
           BEGIN
            Inc(IntOffset,$10000);
            HSeg:=IntOffset SHR 16; ChkSum:=6+Lo(HSeg)+Hi(HSeg);
            WriteLn(TargFile,':02000004',HexWord(HSeg),HexByte($100-ChkSum));
            ChkIO(TargName);
            FirstBank:=False;
           END;

          { Recordl„nge ausrechnen, fr Intel32 auf 64K-Grenze begrenzen
            bei Atmel nur 2 Byte pro Zeile! }

          TransLen:=Min(LineLen,ErgLen);
          IF (ActFormat=IntHex32) AND ((ErgStart AND $ffff)+(TransLen DIV Gran)>=$10000) THEN
           BEGIN
            TransLen:=Gran*($10000-(ErgStart AND $ffff));
            FirstBank:=True;
           END
          ELSE IF ActFormat=Atmel THEN TransLen:=Min(2,TransLen);

	  { Start der Datenzeile }

	  CASE ActFormat OF
	  MotoS:
	   BEGIN
            Write(TargFile,'S',Chr(Ord('1')+MotRecType),HexByte(TransLen+3+MotRecType));
	    ChkIO(TargName);
	    ChkSum:=TransLen+3+MotRecType;
	    IF MotRecType>=2 THEN
	     BEGIN
	      Write(TargFile,HexByte((ErgStart SHR 24) AND $ff)); ChkIO(TargName);
	      Inc(ChkSum,(ErgStart SHR 24) AND $ff);
	     END;
	    IF MotRecType>=1 THEN
	     BEGIN
	      Write(TargFile,HexByte((ErgStart SHR 16) AND $ff)); ChkIO(TargName);
	      Inc(ChkSum,(ErgStart SHR 16) AND $ff);
	     END;
	    Write(TargFile,HexWord(ErgStart AND $ffff)); ChkIO(TargName);
	    Inc(ChkSum,Hi(ErgStart)); Inc(ChkSum,Lo(ErgStart));
	   END;
	  MOSHex:
	   BEGIN
	    Write(TargFile,';',HexByte(TransLen),HexWord(ErgStart AND $ffff));
            ChkIO(TargName);
	    ChkSum:=TransLen+Lo(ErgStart)+Hi(ErgStart);
	   END;
          IntHex,IntHex16,IntHex32:
	   BEGIN
	    Write(TargFile,':'); ChkIO(TargName); ChkSum:=0;
	    IF MultiMode=0 THEN
	     BEGIN
	      Write(TargFile,HexByte(TransLen)); ChkIO(TargName);
	      Write(TargFile,HexWord((ErgStart-IntOffset)*Gran)); ChkIO(TargName);
	      Inc(ChkSum,TransLen);
	      Inc(ChkSum,Lo((ErgStart-IntOffset)*Gran));
	      Inc(ChkSum,Hi((ErgStart-IntOffset)*Gran));
	     END
	    ELSE
	     BEGIN
	      Write(TargFile,HexByte(TransLen DIV Gran)); ChkIO(TargName);
	      Write(TargFile,HexWord(ErgStart-IntOffset)); ChkIO(TargName);
	      Inc(ChkSum,TransLen DIV Gran);
	      Inc(ChkSum,Lo(ErgStart-IntOffset));
	      Inc(ChkSum,Hi(ErgStart-IntOffset));
	     END;
	    Write(TargFile,'00'); ChkIO(TargName);
	   END;
	  TekHex:
	   BEGIN
	    Write(TargFile,'/',HexWord(ErgStart),HexByte(TransLen),
		  HexByte(Lo(ErgStart)+Hi(ErgStart)+TransLen));
	    ChkIO(TargName);
	    ChkSum:=0;
	   END;
	  TiDSK:
	   BEGIN
            Write(TargFile,'9',HexWord({Gran*}ErgStart));
	    ChkIO(TargName);
	    ChkSum:=0;
	   END;
          Atmel:
           Write(TargFile,HexByte(ErgStart SHR 16),HexWord(ErgStart AND $ffff),':');
	  END;

	  { Daten selber }

	  BlockRead(SrcFile,Buffer,TransLen); ChkIO(FileName);
	  IF (Gran<>1) AND (MultiMode=1) THEN
	   FOR z:=0 TO (TransLen DIV Gran)-1 DO
	    BEGIN
	     SwapBase:=z*Gran;
	     FOR z2:=0 TO (Gran DIV 2)-1 DO
	      BEGIN
	       BSwap:=Buffer[SwapBase+z2];
	       Buffer[SwapBase+z2]:=Buffer[SwapBase+Gran-1-z2];
	       Buffer[SwapBase+Gran-1-z2]:=BSwap;
	      END;
	    END;
	  IF ActFormat=TiDSK THEN
	   FOR z:=0 TO (TransLen DIV 2)-1 DO
	    BEGIN
	     IF (ErgStart+z >= StartData) AND (ErgStart+z <= StopData) THEN
	      Write(TargFile,'M',HexWord(WBuffer[z]))
	     ELSE
	      Write(TargFile,'B',HexWord(WBuffer[z]));
	     ChkIO(TargName);
	     Inc(ChkSum,WBuffer[z]);
	     Inc(SumLen,Gran);
	    END
          ELSE IF ActFormat=Atmel THEN
           BEGIN
            IF TransLen>=2 THEN
             BEGIN
              Write(TargFile,HexWord(WBuffer[0])); Inc(SumLen,2);
             END;
           END
	  ELSE
	   FOR z:=0 TO TransLen-1 DO
	   IF (MultiMode<2) OR (z MOD Gran=MultiMode-2) THEN
	    BEGIN
	     Write(TargFile,HexByte(Buffer[z])); ChkIO(TargName);
	     Inc(ChkSum,Buffer[z]);
	     Inc(SumLen);
	    END;

	  { Ende Datenzeile }

	  CASE ActFormat OF
	  MotoS:
	   BEGIN
	    WriteLn(TargFile,HexByte(Lo(NOT ChkSum)));
	    ChkIO(TargName);
	   END;
	  MOSHex:
	   BEGIN
	    WriteLn(TargFile,HexWord(ChkSum));
            ChkIO(TargName);
	   END;
          IntHex,IntHex16,IntHex32:
	   BEGIN
	    WriteLn(TargFile,HexByte(Lo(Succ(NOT ChkSum))));
	    ChkIO(TargName);
	   END;
	  TekHex:
	   BEGIN
	    WriteLn(TargFile,HexByte(Lo(ChkSum)));
	    ChkIO(TargName);
	   END;
	  TiDSK:
	   BEGIN
	    WriteLn(TargFile,'7',HexWord(ChkSum),'F');
	    ChkIO(TargName);
	   END;
          Atmel:
           BEGIN
            WriteLn(TargFile);
            ChkIO(TargName);
           END;
	  END;

	  { Z„hler rauf }

	  Dec(ErgLen,TransLen);
	  Inc(ErgStart,TransLen DIV Gran);
	 END;

	{ Ende der Datenzeilengruppe }

        CASE ActFormat OF
        MotoS:
         IF SepMoto THEN
          BEGIN
	   Write(TargFile,'S',Chr(Ord('9')-MotRecType),HexByte(3+MotRecType));
           ChkIO(TargName);
           FOR z:=1 TO 2+MotRecType DO
            BEGIN
	     Write(TargFile,HexByte(0)); ChkIO(TargName);
            END;
           WriteLn(TargFile,HexByte($ff-3-MotRecType)); ChkIO(TargName);
          END;
        MOSHex:BEGIN END;
        TekHex:BEGIN END;
        TiDSK:BEGIN END;
        IntHex,IntHex16,IntHex32:BEGIN END;
        Atmel:BEGIN END;
        END;
       END;
      Seek(SrcFile,NextPos); ChkIO(FileName);
     END;
   UNTIL InpHeader=0;

   WriteLn('  (',SumLen,' Byte)'); ChkIO(OutName);

   Close(SrcFile); ChkIO(FileName);
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
    WriteLn(ErrMsgNullMaskA,GroupName,ErrMsgNullMaskB)
   ELSE
    WHILE DosError=0 DO
     BEGIN
      Processor(Path+s.Name,Offset);
      FindNext(s);
     END;
END;

        PROCEDURE EraseFile(FileName:String; Offset:LongInt);
        Far;
VAR
   f:File;
BEGIN
   Assign(f,FileName); Erase(f); ChkIO(FileName);
END;

        PROCEDURE MeasureFile(FileName:String; Offset:LongInt);
        Far;
VAR
   f:File;
   Header,Gran,Segment:Byte;
   Length,TestID:Word;
   Adr,EndAdr,HPos,NextPos:LongInt;
BEGIN
   Assign(f,FileName); SetFileMode(0); Reset(f,1);
   ChkIO(FileName);

   BlockRead(f,TestID,2); IF TestID<>FileMagic THEN
    FormatError(FileName,FormatInvHeaderMsg);

   REPEAT
    ReadRecordHeader(Header,Segment,Gran,FileName,f);

    IF Header=FileHeaderStartAdr THEN
     BEGIN
      Seek(f,FilePos(f)+SizeOf(LongInt)); ChkIO(FileName);
     END
    ELSE IF (Header<>FileHeaderEnd) THEN
     BEGIN
      BlockRead(f,Adr,4); ChkIO(FileName);
      BlockRead(f,Length,2); ChkIO(FileName);
      NextPos:=FilePos(f)+Length;
      IF NextPos>FileSize(f) THEN
       FormatError(FileName,FormatInvRecordLenMsg);

      IF (Segment=SegCode) AND (FilterOK(Header)) THEN
       BEGIN
        Inc(Adr,Offset);
        EndAdr:=Adr+(Length DIV Gran)-1;
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


	FUNCTION CMD_RelAdr(Negate:Boolean; Arg:String):CMDResult;
	Far;
BEGIN
   RelAdr:=NOT Negate;
   CMD_RelAdr:=CMDOK;
END;

        FUNCTION CMD_AutoErase(Negate:Boolean; Arg:String):CMDResult;
	Far;
BEGIN
   AutoErase:=NOT Negate;
   CMD_AutoErase:=CMDOK;
END;

        FUNCTION CMD_Rec5(Negate:Boolean; Arg:String):CMDResult;
	Far;
BEGIN
   Rec5:=NOT Negate;
   CMD_Rec5:=CMDOK;
END;

        FUNCTION CMD_SepMoto(Negate:Boolean; Arg:String):CMDResult;
	Far;
BEGIN
   SepMoto:=NOT Negate;
   CMD_SepMoto:=CMDOK;
END;

        FUNCTION CMD_IntelMode(Negate:Boolean; Arg:String):CMDResult;
	Far;
VAR
   err,Mode:Integer;
BEGIN
   IF Arg='' THEN CMD_IntelMode:=CMDErr
   ELSE
    BEGIN
     Val(Arg,Mode,err);
     IF (err<>0) OR (Mode<0) OR (Mode>2) THEN CMD_IntelMode:=CMDErr
     ELSE
      BEGIN
       IF NOT Negate THEN IntelMode:=Mode
       ELSE IF IntelMode=Mode THEN IntelMode:=0;
       CMD_IntelMode:=CMDArg;
      END;
    END;
END;

	FUNCTION CMD_MultiMode(Negate:Boolean; Arg:String):CMDResult;
	Far;
VAR
   err,Mode:Integer;
BEGIN
   IF Arg='' THEN CMD_MultiMode:=CMDErr
   ELSE
    BEGIN
     Val(Arg,Mode,err);
     IF (err<>0) OR (Mode<0) OR (Mode>3) THEN CMD_MultiMode:=CMDErr
     ELSE
      BEGIN
       IF NOT Negate THEN MultiMode:=Mode
       ELSE IF MultiMode=Mode THEN MultiMode:=0;
       CMD_MultiMode:=CMDArg;
      END;
    END;
END;

        FUNCTION CMD_DestFormat(Negate:Boolean; Arg:String):CMDResult;
	Far;
CONST
   NameCnt=9;
   Names:ARRAY[1..NameCnt] OF String[7]=('DEFAULT','MOTO','INTEL','INTEL16','INTEL32','MOS','TEK','DSK','ATMEL');
   Format:ARRAY[1..NameCnt] OF OutFormat=(Default,MotoS,IntHex,IntHex16,IntHex32,MOSHex,TekHex,TiDSK,Atmel);
VAR
   err,z:Integer;
BEGIN
   FOR z:=1 TO Length(Arg) DO Arg[z]:=UpCase(Arg[z]);

   z:=1;
   WHILE (z<=NameCnt) AND (Arg<>Names[z]) DO Inc(z);
   IF z>NameCnt THEN
    BEGIN
     CMD_DestFormat:=CMDErr; Exit;
    END;

   IF NOT Negate THEN DestFormat:=Format[z]
   ELSE IF DestFormat=Format[z] THEN DestFormat:=Default;

   CMD_DestFormat:=CMDArg;
END;

	FUNCTION CMD_DataAdrRange(Negate:Boolean; Arg:String):CMDResult;
	Far;
VAR
   p,err:Integer;
BEGIN
   IF Negate THEN
    BEGIN
     StartData:=0; StopData:=$1fff;
     CMD_DataAdrRange:=CMDOK;
    END
   ELSE
    BEGIN
     CMD_DataAdrRange:=CMDErr;

     p:=Pos('-',Arg); IF p=0 THEN Exit;

     ULongVal(Copy(Arg,1,p-1),StartData,err);
     IF err<>0 THEN Exit;

     ULongVal(Copy(Arg,p+1,Length(Arg)-p),StopData,err);
     IF err<>0 THEN Exit;

     IF IsUGreater(StartData,StopData) THEN Exit;

     CMD_DataAdrRange:=CMDArg;
    END;
END;


	FUNCTION CMD_EntryAdr(Negate:Boolean; Arg:String):CMDResult;
	Far;
VAR
  err : Integer;
BEGIN
   IF Negate THEN
    BEGIN
     EntryAdrPresent:=False;
     CMD_EntryAdr:=CMDOK;
    END
   ELSE
    BEGIN
     CMD_EntryAdr:=CMDErr;
     Val(Arg,EntryAdr,err);
     IF (err<>0) THEN Exit;
     CMD_EntryAdr:=CMDArg;
    END;
END;

        FUNCTION CMD_LineLen(Negate:Boolean; Arg:String):CMDResult;
        Far;
VAR
   err:Integer;
BEGIN
   IF Negate THEN
    IF Arg<>'' THEN CMD_LineLen:=CMDErr
    ELSE
     BEGIN
      LineLen:=16; CMD_LineLen:=CMDOK;
     END
   ELSE
    IF Arg='' THEN CMD_LineLen:=CMDErr
    ELSE
     BEGIN
      Val(Arg,LineLen,err);
      IF (err<>0) OR (LineLen<1) OR (LineLen>MaxLineLen) THEN CMD_LineLen:=CMDErr
      ELSE
       BEGIN
        IF Odd(LineLen) THEN Inc(LineLen);
        CMD_LineLen:=CMDArg;
       END;
     END;
END;

CONST
   P2HEXParamCnt=12;
   P2HEXParams:ARRAY[1..P2HEXParamCnt] OF CMDRec=
	       ((Ident:'f'; Callback:CMD_FilterList),
		(Ident:'r'; Callback:CMD_AdrRange),
		(Ident:'a'; Callback:CMD_RelAdr),
		(Ident:'i'; Callback:CMD_IntelMode),
		(Ident:'m'; Callback:CMD_MultiMode),
		(Ident:'F'; Callback:CMD_DestFormat),
		(Ident:'5'; Callback:CMD_Rec5),
		(Ident:'s'; Callback:CMD_SepMoto),
		(Ident:'d'; Callback:CMD_DataAdrRange),
                (Ident:'e'; Callback:CMD_EntryAdr),
                (Ident:'l'; Callback:CMD_LineLen),
                (Ident:'k'; Callback:CMD_AutoErase));

VAR
   ChkSum:Word;

BEGIN
   NLS_Initialize; WrCopyRight('P2HEX/2 V1.41r7','P2HEX V1.41r7');

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

   ParProcessed:=[1..ParamCount];
   StartAdr:=0; StopAdr:=$7fff;
   StartAuto:=False; StopAuto:=False;
   StartData:=0; StopData:=$1fff;
   EntryAdr:=-1; EntryAdrPresent:=False;
   RelAdr:=False; Rec5:=True; LineLen:=16;
   IntelMode:=0; MultiMode:=0; DestFormat:=Default;
   TargName:=''; AutoErase:=False;
   ProcessCMD(@P2HEXParams,P2HEXParamCnt,ParProcessed,'P2HEXCMD',ParamError);

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
   AddSuffix(TargName,HexSuffix);

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
   MotoOccured:=False; IntelOccured:=False;
   MOSOccured:=False;  DSKOccured:=False;
   MaxMoto:=0; MaxIntel:=0;

   IF ParProcessed=[] THEN ProcessGroup(SrcName,ProcessFile)
   ELSE FOR z:=1 TO ParamCount DO
    IF z IN ParProcessed THEN ProcessGroup(ParamStr(z),ProcessFile);

   IF (MotoOccured) AND (NOT SepMoto) THEN
    BEGIN
     Write(TargFile,'S',Chr(Ord('9')-MaxMoto),HexByte(3+MaxMoto)); ChkIO(TargName);
     ChkSum:=3+MaxMoto;
     IF NOT EntryAdrPresent THEN EntryAdr:=0;
     IF MaxMoto>=2 THEN
      BEGIN
       Write(TargFile,HexByte((EntryAdr SHR 24) AND $ff)); ChkIO(TargName);
       Inc(ChkSum,(EntryAdr SHR 24) AND $ff);
      END;
     IF MaxMoto>=1 THEN
      BEGIN
       Write(TargFile,HexByte((EntryAdr SHR 16) AND $ff)); ChkIO(TargName);
       Inc(ChkSum,(EntryAdr SHR 16) AND $ff);
      END;
     Write(TargFile,HexWord(EntryAdr AND $ffff)); ChkIO(TargName);
     Inc(ChkSum,(EntryAdr SHR 8) AND $ff);
     Inc(ChkSum,EntryAdr AND $ff);
     WriteLn(TargFile,HexByte($ff-(ChkSum AND $ff))); ChkIO(TargName);
    END;

   IF IntelOccured THEN
    BEGIN
     IF EntryAdrPresent THEN
      BEGIN
       IF MaxIntel=2 THEN
        BEGIN
         Write(TargFile,':04000003'); ChkIO(TargName); ChkSum:=4+3;
         Write(TargFile,HexWord(EntryAdr SHR 16),HexWord(EntryAdr AND $ffff));
         ChkIO(TargName);
         Inc(ChkSum,((EntryAdr SHR 24) AND $ff)+
                    ((EntryAdr SHR 16) AND $ff)+
                    ((EntryAdr SHR  8) AND $ff)+
                    ( EntryAdr         AND $ff));
        END
       ELSE IF MaxIntel=1 THEN
        BEGIN
         Write(TargFile,':04000003'); ChkIO(TargName); ChkSum:=4+3;
         Seg:=(EntryAdr SHR 4) AND $ffff;
         Ofs:=EntryAdr AND $000f;
         Write(TargFile,HexWord(Seg),HexWord(Ofs));
         ChkIO(TargName);
         Inc(ChkSum,Lo(Seg)+Hi(Seg)+Ofs);
        END
       ELSE
        BEGIN
         Write(TargFile,':02000003',HexWord(EntryAdr AND $ffff));
         ChkIO(TargName); ChkSum:=2+3+Lo(EntryAdr)+Hi(EntryAdr);
        END;
       WriteLn(TargFile,HexByte($100-ChkSum)); ChkIO(TargName);
      END;
     CASE IntelMode OF
     0:WriteLn(TargFile,':00000001FF');
     1:WriteLn(TargFile,':00000001');
     2:WriteLn(TargFile,':0000000000');
     END;
     ChkIO(TargName);
    END;

   IF MOSOccured THEN
    BEGIN
     WriteLn(TargFile,';0000040004'); ChkIO(TargName);
    END;

   IF DSKOccured THEN
    BEGIN
     IF EntryAdrPresent THEN
      BEGIN
       WriteLn(TargFile,'1',HexWord(EntryAdr),'7',HexWord(EntryAdr),'F');
       ChkIO(TargName);
      END;
     WriteLn(TargFile,':'); ChkIO(TargName);
    END;

   CloseTarget;

   IF AutoErase THEN
    BEGIN
     IF ParProcessed=[] THEN ProcessGroup(SrcName,EraseFile)
     ELSE FOR z:=1 TO ParamCount DO
      IF z IN ParProcessed THEN ProcessGroup(ParamStr(z),EraseFile);
    END;
END.
