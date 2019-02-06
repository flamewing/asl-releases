{$i STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
	UNIT IncList;

INTERFACE
        USES StringUt,FileNums,AsmDef,AsmSub;

  	PROCEDURE PushInclude(VAR S:String);

	PROCEDURE PopInclude;

	PROCEDURE PrintIncludeList;

	PROCEDURE ClearIncludeList;


IMPLEMENTATION

{$I AS.RSC}

TYPE
   TFileArray=ARRAY[1..100] OF Pointer;
   PFileArray=^TFileArray;
   PFileNode=^TFileNode;
   TFileNode=RECORD
              Name:Integer;
              Len:Integer;
              Parent:PFileNode;
              Subs:PFileArray;
             END;

VAR
  Root,Curr:PFileNode;

  	PROCEDURE PushInclude(VAR S:String);
VAR
   Neu:PFileNode;
   Field:PFileArray;
BEGIN
   New(Neu);
   WITH Neu^ DO
    BEGIN
     Name:=GetFileNum(S);
     Len:=0; Subs:=Nil;
     Parent:=Curr;
    END;
   IF Root=Nil THEN Root:=Neu;
   IF Curr=Nil THEN Curr:=Neu
   ELSE
    BEGIN
     GetMem(Field,(Curr^.Len+1)*SizeOf(Pointer));
     Move(Curr^.Subs^,Field^,Curr^.Len*SizeOf(Pointer));
     Field^[Curr^.Len+1]:=Neu;
     FreeMem(Curr^.Subs,Curr^.Len*SizeOf(Pointer));
     Curr^.Subs:=Field; Inc(Curr^.Len);
     Curr:=Neu;
    END;
END;

	PROCEDURE PopInclude;
BEGIN
   IF Curr<>Nil THEN Curr:=Curr^.Parent;
END;

	PROCEDURE PrintIncludeList;

        PROCEDURE PrintNode(Node:PFileNode; Indent:Integer);
VAR
   z:Integer;
BEGIN
   ChkStack;

   IF Node<>Nil THEN
    WITH Node^ DO
     BEGIN
      WrLstLine(Blanks(Indent)+GetFileName(Node^.Name));
      FOR z:=1 TO Len DO PrintNode(Subs^[z],Indent+5);
     END;
END;

BEGIN
   NewPage(ChapDepth,True);
   WrLstLine(ListIncludeListHead1);
   WrLstLine(ListIncludeListHead2);
   WrLstLine('');
   PrintNode(Root,0);
END;

	PROCEDURE ClearIncludeList;

        PROCEDURE ClearNode(Node:PFileNode);
VAR
   z:Integer;
BEGIN
   ChkStack;

   IF Node<>Nil THEN
    BEGIN
     FOR z:=1 TO Node^.Len DO ClearNode(Node^.Subs^[z]);
     FreeMem(Node^.Subs,Node^.Len*SizeOf(Pointer));
     Dispose(Node);
    END;
END;

BEGIN
   ClearNode(Root);
   Curr:=Nil; Root:=Nil;
END;

BEGIN
  Root:=Nil; Curr:=Nil;
END.
