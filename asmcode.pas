{$I STDINC.PAS}
        UNIT ASMCode;

INTERFACE

        USES StdHandl,Chunks,
	     ASMDef,ASMSub;

VAR
   LenSoFar:Word;
   RecPos,LenPos:LongInt;

   	PROCEDURE DreheCodes;

        PROCEDURE NewRecord;

        PROCEDURE OpenFile;

        PROCEDURE CloseFile;

        PROCEDURE WriteBytes;

        PROCEDURE RetractWords(Cnt:Word);

IMPLEMENTATION

CONST
   CodeBufferSize=512;

VAR
   CodeBufferFill:Integer;
   CodeBuffer:ARRAY[0..CodeBufferSize] OF Byte;

        PROCEDURE FlushBuffer;
BEGIN
   IF CodeBufferFill>0 THEN
    BEGIN
     {$i-}
     BlockWrite(PrgFile,CodeBuffer,CodeBufferFill);
     ChkIO(10004);
     {$i+}
     CodeBufferFill:=0;
    END;
END;

	PROCEDURE DreheCodes;

{$IFDEF SPEEDUP}
	Assembler;
ASM
        call    [Granularity]
        cbw
        mul     word ptr [CodeLen]
        mov     cx,ax
	call	[ListGran]
	mov	dl,al
        cmp     dl,1
        je      @end
	lea     si,[BAsmCode]
	mov     di,si
	mov     ax,ds
	mov     es,ax
	shr     cx,1
	cld
	cmp	dl,2
	jne     @no2
@schl: 	lodsw
	xchg    ah,al
	stosw
	loop    @schl
	jmp	@end
@no2:   shr	cx,1
@schl4: lodsw
	mov	bx,ax
	lodsw
	xchg	al,ah
	xchg	bl,bh
	stosw
	mov	ax,bx
	stosw
        loop    @schl4
@end:
END;

{$ELSE}

VAR
   z:Integer;
BEGIN
    CASE ListGran OF
    2:FOR z:=0 TO ((CodeLen*Granularity) SHR 1)-1 DO WAsmCode[z]:=Swap(WAsmCode[z]);
    4:FOR z:=0 TO ((CodeLen*Granularity) SHR 2)-1 DO
       DAsmCode[z]:=(DAsmCode[z] SHR 24)+
		    ((DAsmCode[z] AND $00ff0000) SHR 8)+
		    ((DAsmCode[z] AND $0000ff00) SHL 8)+
		    ((DAsmCode[z] AND $000000ff) SHL 24);
    END;
END;

{$ENDIF}

{---- neuen Record in Codedatei anlegen.  War der bisherige leer, so wird ---
 ---- dieser Åberschrieben. -------------------------------------------------}

        PROCEDURE WrRecHeader;
VAR
   b:Byte;
BEGIN
   IF (ActPC=SegCode) THEN
    BEGIN
     BlockWrite(PrgFile,HeaderID,1); ChkIO(10004);
    END
   ELSE
    BEGIN
     b:=FileHeaderDataRec; BlockWrite(PrgFile,b,1); ChkIO(10004);
     BlockWrite(PrgFile,HeaderID,1); ChkIO(10004);
     b:=ActPC; BlockWrite(PrgFile,b,1); ChkIO(10004);
     b:=Granularity; BlockWrite(PrgFile,b,1); ChkIO(10004);
    END;
END;

        PROCEDURE NewRecord;
VAR
   h:LongInt;
   b:Byte;
BEGIN
   FlushBuffer;
   {$i-}
   IF LenSoFar=0 THEN
    BEGIN
     Seek(PrgFile,RecPos); ChkIO(10003);
     WrRecHeader;
     BlockWrite(PrgFile,PCs[ActPC],4); ChkIO(10004);
     LenPos:=FilePos(PrgFile);
     BlockWrite(PrgFile,LenSoFar,2); ChkIO(10004);
    END
   ELSE
    BEGIN
     h:=FilePos(PrgFile);
     Seek(PrgFile,LenPos); ChkIO(10003);
     BlockWrite(PrgFile,LenSoFar,2); ChkIO(10004);
     Seek(PrgFile,h); ChkIO(10003);

     RecPos:=h; LenSoFar:=0;
     WrRecHeader;
     BlockWrite(PrgFile,PCs[ActPC],4); ChkIO(10004);
     LenPos:=FilePos(PrgFile);
     BlockWrite(PrgFile,LenSoFar,2); ChkIO(10004);
    END;
   {$i+}
END;

{---- Codedatei erîffnen ----------------------------------------------------}

       PROCEDURE OpenFile;
VAR
   h:Word;
BEGIN
   {$i-} Assign(PrgFile,OutName); SetFileMode(2); Rewrite(PrgFile,1); {$i+}
   ChkIO(10001);

   {$i-} h:=FileMagic; BlockWrite(PrgFile,h,2); {$i+} ChkIO(10004);

   CodeBufferFill:=0;
   RecPos:=FilePos(PrgFile); LenSoFar:=0;
   NewRecord;
END;

{---- Codedatei schlie·en ---------------------------------------------------}

       PROCEDURE CloseFile;
VAR
   h:String;
   IO:Integer;
   Head:Byte;
BEGIN
   NewRecord; Seek(PrgFile,RecPos);

   IF StartAdrPresent THEN
    BEGIN
     Head:=FileHeaderStartAdr;
     {$i-} BlockWrite(PrgFile,Head,Sizeof(Head)); {$i+} ChkIO(10004);
     {$i-} BlockWrite(PrgFile,StartAdr,SizeOf(StartAdr));  {$i+} ChkIO(10004);
    END;

   {$i-} Head:=FileHeaderEnd; BlockWrite(PrgFile,Head,Sizeof(Head)); {$i-} ChkIO(10004);

   h:='AS '+Version+'/'+ArchVal;
   {$i-} BlockWrite(PrgFile,h[1],Length(h)); {$i+} ChkIO(10004);
   {$i-}
   Close(PrgFile); IF Magic<>0 THEN Erase(PrgFile); IO:=IOResult;
   {$i+}
END;

{---- erzeugten Code einer Zeile in Datei ablegen ---------------------------}

	PROCEDURE WriteBytes;
VAR
   ErgLen:Word;
BEGIN
   IF CodeLen=0 THEN Exit; ErgLen:=CodeLen*Granularity;
   IF TurnWords THEN DreheCodes;
   IF LongInt(LenSoFar)+LongInt(ErgLen)>$ffff THEN NewRecord;
   IF CodeBufferFill+ErgLen<CodeBufferSize THEN
    BEGIN
     Move(BAsmCode,CodeBuffer[CodeBufferFill],ErgLen);
     Inc(CodeBufferFill,ErgLen);
    END
   ELSE
    BEGIN
     FlushBuffer;
     IF ErgLen<CodeBufferSize THEN
      BEGIN
       Move(BAsmCode,CodeBuffer,ErgLen); CodeBufferFill:=ErgLen;
      END
     ELSE
      BEGIN
       {$i-}
       BlockWrite(PrgFile,BAsmCode,ErgLen);
       ChkIO(10004);
       {$i+}
      END;
    END;
   Inc(LenSoFar,ErgLen);
   IF TurnWords THEN DreheCodes;
END;

        PROCEDURE RetractWords(Cnt:Word);
VAR
   ErgLen:Word;
BEGIN
   ErgLen:=Cnt*Granularity;
   IF LenSoFar<ErgLen THEN
    BEGIN
     WrError(1950); Exit;
    END;

   IF MakeUseList THEN DeleteChunk(SegChunks[ActPC],ProgCounter-Cnt,Cnt);

   Dec(PCs[ActPC],Cnt);

   IF CodeBufferFill>=ErgLen THEN Dec(CodeBufferFill,ErgLen)
   ELSE
    BEGIN
     {$i-}
     Seek(PrgFile,FilePos(PrgFile)-(ErgLen-CodeBufferFill));
     {$i+}
     ChkIO(10004);
     CodeBufferFill:=0;
    END;

   Dec(LenSoFar,ErgLen);

   Retracted:=True;
END;

END.
