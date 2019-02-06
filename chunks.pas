{$i STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
	UNIT Chunks;

INTERFACE

        USES StringUt;

TYPE
   OneChunk=RECORD
	     Start,Length:LongInt;
	    END;

   ChunkField=ARRAY[1..8100] OF OneChunk;

   ChunkList=RECORD
	      RealLen,AllocLen:Word;
	      Chunks:^ChunkField;
	     END;

        FUNCTION AddChunk(VAR NChunk:ChunkList; NewStart,NewLen:LongInt; Warn:Boolean):Boolean;

        PROCEDURE DeleteChunk(VAR NChunk:ChunkList; DelStart,DelLen:LongInt);

        PROCEDURE InitChunk(VAR NChunk:ChunkList);

        PROCEDURE ClearChunk(VAR NChunk:ChunkList);

        FUNCTION IsUGreater(x1,x2:LongInt):Boolean;

        FUNCTION IsUGreaterEq(x1,x2:LongInt):Boolean;

IMPLEMENTATION

{ eine Chunkliste initialisieren }

        PROCEDURE InitChunk(VAR NChunk:ChunkList);
BEGIN
   WITH NChunk DO
    BEGIN
     RealLen:=0; AllocLen:=0;
     Chunks:=Nil;
    END;
END;

{ lîschen }

        PROCEDURE ClearChunk(VAR NChunk:ChunkList);
BEGIN
   WITH NChunk DO
    IF AllocLen>0 THEN FreeMem(Chunks,AllocLen*SizeOf(OneChunk));
   InitChunk(NChunk);
END;


{----------------------------------------------------------------------------}
{ vorzeichenlose 32-Bit-Vergleiche }

        FUNCTION IsUGreater(x1,x2:LongInt):Boolean;

{$IFDEF SPEEDUP}

        Assembler;
ASM
        mov     al,1              { Annahme TRUE }
        mov     bx,word ptr[x1+2] { x1H>x2H ? }
        cmp     bx,word ptr[x2+2]
        ja      @isg
        jb      @false
        mov     bx,word ptr[x1]   { x1L>x2L ? }
        cmp     bx,word ptr[x2]
        ja      @isg
@false: dec     al                { FALSE }
@isg:
END;

{$ELSE}

BEGIN
   IF x1>=0 THEN
    IF x2>=0 THEN IsUGreater:=x1>x2
    ELSE IsUGreater:=False
   ELSE
    IF x2>=0 THEN IsUGreater:=True
    ELSE IsUGreater:=x1>x2;
END;

{$ENDIF}


        FUNCTION IsUGreaterEq(x1,x2:LongInt):Boolean;

{$IFDEF SPEEDUP }

        Assembler;
ASM
        mov     al,1              { Annahme TRUE }
        mov     bx,word ptr[x1+2] { x1H>x2H ? }
        cmp     bx,word ptr[x2+2]
        ja      @isg
        jb      @isb
        mov     bx,word ptr[x1]   { x1L>x2L ? }
        cmp     bx,word ptr[x2]
        jae     @isg
@isb:   dec     al                { FALSE }
@isg:
END;

{$ELSE}

BEGIN
   IF x1>=0 THEN
    IF x2>=0 THEN IsUGreaterEq:=x1>=x2
    ELSE IsUGreaterEq:=False
   ELSE
    IF x2>=0 THEN IsUGreaterEq:=True
    ELSE IsUGreaterEq:=x1>=x2;
END;

{$ENDIF}


{----------------------------------------------------------------------------}
{ eine Chunkliste um einen Eintrag erweitern }

        FUNCTION Min(n1,n2:LongInt):LongInt;
BEGIN
   IF IsUGreater(n1,n2) THEN Min:=n2 ELSE Min:=n1;
END;

        FUNCTION Max(n1,n2:LongInt):LongInt;
BEGIN
   IF IsUGreater(n2,n1) THEN Max:=n2 ELSE Max:=n1;
END;

        FUNCTION Overlap(Start1,Len1,Start2,Len2:LongInt):Boolean;
BEGIN
   Overlap:=  (Start1=Start2)
          OR (IsUGreater(Start2,Start1) AND IsUGreaterEq(Start1+Len1,Start2))
          OR (IsUGreater(Start1,Start2) AND IsUGreaterEq(Start2+Len2,Start1));
END;

        PROCEDURE SetChunk(VAR NChunk:OneChunk; Start1,Len1,Start2,Len2:LongInt);
BEGIN
   NChunk.Start :=Min(Start1,Start2);
   NChunk.Length:=Max(Start1+Len1-1,Start2+Len2-1)-NChunk.Start+1;
END;

        PROCEDURE IncChunk(VAR NChunk:ChunkList);
VAR
   PAlt:Pointer;
BEGIN
   WITH NChunk DO
    BEGIN
     IF RealLen+1>AllocLen THEN
      BEGIN
       PAlt:=Chunks;
       GetMem(Chunks,(RealLen+1)*SizeOf(OneChunk));
       Move(PAlt^,Chunks^,RealLen*SizeOf(OneChunk));
       IF AllocLen<>0 THEN FreeMem(PAlt,AllocLen*SizeOf(OneChunk));
       AllocLen:=RealLen+1;
      END;
    END;
END;

        FUNCTION AddChunk(VAR NChunk:ChunkList; NewStart,NewLen:LongInt; Warn:Boolean):Boolean;
VAR
   z,f1,f2:Word;
   Found:Boolean;
   PAlt:Pointer;
   PartSum:LongInt;

BEGIN
   AddChunk:=False;

   IF NewLen=0 THEN Exit;

   WITH NChunk DO
    BEGIN
     { herausfinden, ob sich das neue Teil irgendwo mitanhÑngen lÑ·t }

     Found:=False;
     FOR z:=1 TO RealLen DO
      IF Overlap(NewStart,NewLen,Chunks^[z].Start,Chunks^[z].Length) THEN
       BEGIN
        Found:=True; f1:=z; z:=RealLen;
       END;

     { Fall 1: etwas gefunden : }

     IF Found THEN
      BEGIN
       { gefundene Chunk erweitern }

       PartSum:=Chunks^[f1].Length+NewLen;
       SetChunk(Chunks^[f1],NewStart,NewLen,Chunks^[f1].Start,Chunks^[f1].Length);
       IF Warn THEN
        IF PartSum<>Chunks^[f1].Length THEN AddChunk:=True;

       { schauen, ob sukzessiv neue Chunks angebunden werden kînnen }

       REPEAT
        Found:=False;
        FOR z:=1 TO RealLen DO
        IF z<>f1 THEN
         IF Overlap(Chunks^[z].Start,Chunks^[z].Length,Chunks^[f1].Start,Chunks^[f1].Length) THEN
          BEGIN
           Found:=True; f2:=z; z:=RealLen;
          END;
        IF Found THEN
         BEGIN
          SetChunk(Chunks^[f1],Chunks^[f1].Start,Chunks^[f1].Length,Chunks^[f2].Start,Chunks^[f2].Length);
          Chunks^[f2]:=Chunks^[RealLen]; Dec(RealLen);
         END;
       UNTIL NOT Found;
      END

     { ansonsten Feld erweitern und einschreiben }

     ELSE
      BEGIN
       IncChunk(NChunk);
       Inc(RealLen);
       Chunks^[RealLen].Length:=NewLen; Chunks^[RealLen].Start:=NewStart;
      END;
    END;
END;

{----------------------------------------------------------------------------}
{ Ein StÅck wieder austragen }

        PROCEDURE DeleteChunk(VAR NChunk:ChunkList; DelStart,DelLen:LongInt);
VAR
   z:Word;
   OStart:LongInt;
BEGIN
   IF DelLen=0 THEN Exit;

   WITH NChunk DO
    BEGIN
     z:=1;
     WHILE z<=RealLen DO
      BEGIN
       IF Overlap(DelStart,DelLen,Chunks^[z].Start,Chunks^[z].Length) THEN
        BEGIN
         IF IsUGreater(Chunks^[z].Start,DelStart) THEN
          IF IsUGreaterEq(DelStart+DelLen,Chunks^[z].Start+Chunks^[z].Length) THEN
           BEGIN
            { ganz lîschen }
            Chunks^[z]:=Chunks^[RealLen]; Dec(RealLen);
           END
          ELSE
           BEGIN
            { unten abschneiden }
            OStart:=Chunks^[z].Start; Chunks^[z].Start:=DelStart+DelLen;
            Dec(Chunks^[z].Start,Chunks^[z].Start-OStart);
           END
         ELSE
          IF IsUGreaterEq(DelStart+DelLen,Chunks^[z].Start+Chunks^[z].Length) THEN
           BEGIN
            { oben abschneiden }
            Chunks^[z].Length:=DelStart-Chunks^[z].Start;
            { wenn LÑnge 0, ganz lîschen }
            IF Chunks^[z].Length=0 THEN
             BEGIN
              Chunks^[z]:=Chunks^[RealLen]; Dec(RealLen);
             END;
           END
          ELSE
           BEGIN
            { teilen }
            IncChunk(NChunk);
            Inc(RealLen);
            Chunks^[RealLen].Start:=DelStart+DelLen;
            Chunks^[RealLen].Length:=Chunks^[z].Start+Chunks^[z].Length-Chunks^[RealLen].Start;
            Chunks^[z].Length:=DelStart-Chunks^[z].Start;
           END;
        END;
       Inc(z);
      END;
    END;
END;

END.
