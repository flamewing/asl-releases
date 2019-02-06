{$g-,i-,v-,r-,n+}
	PROGRAM PList;
        Uses StdHandl,Hex,StringUt,AsmUtils,DecodeCm,NLS;

CONST
   HeaderCnt=54;
   HeaderBytes:ARRAY[1..HeaderCnt] OF Byte=(
    $01,$05,$09,$11,$12,$13,$14,
    $19,$21,$29,$31,$32,$33,$39,$3a,
    $3b,$3c,$41,
    $42,$47,$48,$49,$4a,$4c,$51,$52,$53,
    $54,$55,$56,$61,$62,$63,
    $64,$65,$66,$68,$69,$6c,$6e,$6f,$70,
    $71,$72,$73,$74,$75,$76,
    $77,$78,$79,$7a,$7b,$7c);
   HeaderNames:ARRAY[1..HeaderCnt] OF String[10]=(
    '680x0     ','MPC601    ','DSP56000  ','65xx      ','MELPS-4500','M16       ','M16C      ',
    'MELPS-7700','MCS-48    ','29xxx     ','MCS-(2)51 ','ST9       ','ST7       ','MCS-96    ','8X30x     ',
    'AVR       ','XA        ','8080/8085 ',
    '8086      ','TMS320C6x ','TMS9900   ','TMS370xx  ','MSP430    ','80C166/167','Zx80      ','TLCS-900  ','TLCS-90   ',
    'TLCS-870  ','TLCS-47xx ','TLCS-9000 ','68xx      ','6805/HC08 ','6809      ',
    '6804      ','68HC16    ','68HC12    ','H8/300(H) ','H8/500    ','SH7000    ','SC/MP     ','COP8      ','16C8x     ',
    '16C5x     ','17C4x     ','TMS7000   ','TMS3201x  ','TMS3202x  ','TMS320C3x ',
    'TMS320C5x ','ST62xx    ','Z8        ','78(C)1x   ','75K0      ','78K0      ');

   SegNames:ARRAY[0..PCMax] OF String[8]=('NONE','CODE','DATA','IDATA',
                                          'XDATA','YDATA','BDATA','IO',
                                          'REG');

VAR
   ProgFile:File;
   ProgName:String;
   Header,Segment,Gran:Byte;
   StartAdr:LongInt;
   Len,ID,z:Word;
   Ch:Char;HeadFnd:Boolean;
   Sums:ARRAY[0..PCMax] OF LongInt;

{$i Tools.rsc}
{$i PList.rsc}

BEGIN
   NLS_Initialize; WrCopyRight('PLIST/2 V1.41r7','PLIST V1.41r7');

   IF ParamCount=0 THEN
    BEGIN
     Write(MessFileRequest); ReadLn(ProgName);
     ChkIO(OutName);
    END
   ELSE IF ParamCount=1 THEN ProgName:=ParamStr(1)
   ELSE
    BEGIN
     WriteLn(InfoMessHead1,GetEXEName,InfoMessHead2); ChkIO(OutName);
     FOR z:=1 TO InfoMessHelpCnt DO
      BEGIN
       WriteLn(InfoMessHelp[z]); ChkIO(OutName);
      END;
     Halt(1);
    END;

   AddSuffix(ProgName,Suffix);

   Assign(ProgFile,ProgName); SetFileMode(0); Reset(ProgFile,1);
   ChkIO(ProgName);

   BlockRead(ProgFile,ID,2); ChkIO(ProgName);
   IF ID<>FileMagic THEN FormatError(ProgName,FormatInvHeaderMsg);

   WriteLn; ChkIO(OutName);
   WriteLn(MessHeaderLine1); ChkIO(OutName);
   WriteLn(MessHeaderLine2); ChkIO(OutName);

   FOR z:=0 TO PCMax DO Sums[z]:=0;

   REPEAT
    ReadRecordHeader(Header,Segment,Gran,ProgName,ProgFile);

    HeadFnd:=False;

    IF Header=FileHeaderEnd THEN
     BEGIN
      Write(MessGenerator); ChkIO(OutName);
      FOR z:=FilePos(ProgFile) TO FileSize(ProgFile)-1 DO
       BEGIN
	BlockRead(ProgFile,Ch,1); ChkIO(ProgName);
	Write(Ch); ChkIO(OutName);
       END;
      WriteLn; ChkIO(OutName); HeadFnd:=True;
     END

    ELSE IF Header=FileHeaderStartAdr THEN
     BEGIN
      BlockRead(ProgFile,StartAdr,4); ChkIO(ProgName);
      WriteLn(MessEntryPoint,HexWord(StartAdr SHR 16),HexWord(StartAdr AND $ffff));
      ChkIO(OutName);
     END

    ELSE
     BEGIN
      FOR z:=1 TO HeaderCnt DO
       IF (Magic=0) AND (Header=HeaderBytes[z]) THEN
	BEGIN
	 Write(HeaderNames[z],'':4); ChkIO(OutName);
	 HeadFnd:=True;
	END;

      IF NOT HeadFnd THEN Write('???=',HexByte(Header),'':8);

      Write(SegNames[Segment],'':(3+5-Length(SegNames[Segment])));
      ChkIO(OutName);

      BlockRead(ProgFile,StartAdr,4); ChkIO(ProgName);
      Write(HexWord(StartAdr SHR 16),HexWord(StartAdr AND $ffff),'':10);
      ChkIO(OutName);

      BlockRead(ProgFile,Len,2); ChkIO(ProgName);
      Write(HexWord(Len),'':6); ChkIO(OutName);

      IF Len<>0 THEN Inc(StartAdr,((Len DIV Gran)-1))
      ELSE Dec(StartAdr);
      WriteLn(HexWord(StartAdr SHR 16),HexWord(StartAdr AND $ffff));
      ChkIO(OutName);

      Inc(Sums[Segment],Len);

      IF FilePos(ProgFile)+Len>=FileSize(ProgFile) THEN
       FormatError(ProgName,FormatInvRecordLenMsg)
      ELSE Seek(ProgFile,FilePos(ProgFile)+Len); ChkIO(ProgName);
     END;

   UNTIL Header=0;

   WriteLn; ChkIO(OutName);
   Write(MessSum1); ChkIO(OutName);
   FOR z:=0 TO PCMax DO
    IF (z=SegCode) OR (Sums[z]<>0) THEN
     BEGIN
      Write(Sums[z]); ChkIO(OutName);
      IF Sums[z]=1 THEN WriteLn(MessSumSing,SegNames[z])
      ELSE WriteLn(MessSumPlur,SegNames[z]); ChkIO(OutName);
      Write(Blanks(Length(MessSum1))); ChkIO(OutName);
     END;
   WriteLn; ChkIO(OutName);
   WriteLn; ChkIO(OutName);

   Close(ProgFile); ChkIO(ProgName);
END.
