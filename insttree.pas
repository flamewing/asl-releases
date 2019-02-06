{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable PreLoad Discardable}
{$ENDIF}
	UNIT InstTree;

INTERFACE

	USES AsmDef,AsmSub;

TYPE
   InstProc=PROCEDURE(Index:Word);
   PInstTreeNode=^TInstTreeNode;
   TInstTreeNode=RECORD
                  Left,Right:PInstTreeNode;
                  Proc:InstProc;
                  Name:^String;
                  Index:Word;
                  Balance:ShortInt;
                 END;

        PROCEDURE AddInstTree(VAR Root:PInstTreeNode; NName:String; NProc:InstProc; NIndex:Word);

        PROCEDURE ClearInstTree(VAR Root:PInstTreeNode);

        FUNCTION SearchInstTree(Root:PInstTreeNode):Boolean;

	PROCEDURE PrintInstTree(Root:PInstTreeNode);

IMPLEMENTATION


        PROCEDURE AddInstTree(VAR Root:PInstTreeNode; NName:String; NProc:InstProc; NIndex:Word);

        FUNCTION AddSingle(VAR Node:PInstTreeNode):Boolean;
VAR
   p1,p2:PInstTreeNode;
BEGIN
   ChkStack; AddSingle:=False;

   IF Node=Nil THEN
    BEGIN
     New(Node);
     WITH Node^ DO
      BEGIN
       Left:=Nil; Right:=Nil; Proc:=NProc; Index:=NIndex;
       Balance:=0;
       GetMem(Name,Length(NName)+1); Name^:=NName;
      END;
     AddSingle:=True;
    END
   ELSE IF NName<Node^.Name^ THEN
    BEGIN
     IF AddSingle(Node^.Left) THEN
      CASE Node^.Balance OF
      1:Node^.Balance:=0;
      0:BEGIN
         Node^.Balance:=-1; AddSingle:=True;
        END;
      -1:BEGIN
          p1:=Node^.Left;
          IF p1^.Balance=-1 THEN
           BEGIN
            Node^.Left:=p1^.Right; p1^.Right:=Node;
            Node^.Balance:=0; Node:=p1;
           END
          ELSE
           BEGIN
            p2:=p1^.Right;
            p1^.Right:=p2^.Left; p2^.Left:=p1;
            Node^.Left:=p2^.Right; p2^.Right:=Node;
            IF p2^.Balance=-1 THEN Node^.Balance:= 1 ELSE Node^.Balance:=0;
            IF p2^.Balance= 1 THEN p1^  .Balance:=-1 ELSE p1  ^.Balance:=0;
            Node:=p2;
           END;
          Node^.Balance:=0;
         END;
      END;
    END
   ELSE
    BEGIN
     IF AddSingle(Node^.Right) THEN
      CASE Node^.Balance OF
      -1:Node^.Balance:=0;
      0:BEGIN
          Node^.Balance:=1; AddSingle:=True;
        END;
      1:BEGIN
         p1:=Node^.Right;
         IF p1^.Balance=1 THEN
          BEGIN
           Node^.Right:=p1^.Left; p1^.Left:=Node;
           Node^.Balance:=0; Node:=p1;
          END
         ELSE
          BEGIN
           p2:=p1^.Left;
           p1^.Left:=p2^.Right; p2^.Right:=p1;
           Node^.Right:=p2^.Left; p2^.Left:=Node;
           IF p2^.Balance= 1 THEN Node^.Balance:=-1 ELSE Node^.Balance:=0;
           IF p2^.Balance=-1 THEN p1^  .Balance:= 1 ELSE p1^  .Balance:=0;
           Node:=p2;
          END;
         Node^.Balance:=0;
        END;
      END;
    END;
END;

BEGIN
   IF AddSingle(Root) THEN;
END;

	PROCEDURE ClearInstTree(VAR Root:PInstTreeNode);

        PROCEDURE ClearSingle(VAR Node:PInstTreeNode);
BEGIN
   ChkStack;

   IF Node<>Nil THEN
    BEGIN
     FreeMem(Node^.Name,Length(Node^.Name^)+1);
     ClearSingle(Node^.Left);
     ClearSingle(Node^.Right);
     Dispose(Node); Node:=Nil;
    END;
END;

BEGIN
   ClearSingle(Root);
END;

        FUNCTION SearchInstTree(Root:PInstTreeNode):Boolean;
VAR
   z:Integer;
BEGIN
   z:=0;
   WHILE (Root<>Nil) AND (NOT Memo(Root^.Name^)) DO
    BEGIN
     IF OpPart<Root^.Name^ THEN Root:=Root^.Left
     ELSE Root:=Root^.Right;
     Inc(z);
    END;

   IF Root=Nil THEN SearchInstTree:=False
   ELSE
    BEGIN
     Root^.Proc(Root^.Index);
     SearchInstTree:=True;
    END;
END;

	PROCEDURE PrintInstTree(Root:PInstTreeNode);

	PROCEDURE PNode(Node:PInstTreeNode; Lev:Word);
BEGIN
   IF Node<>Nil THEN
    BEGIN
     PNode(Node^.Left,Lev+1);
     WriteLn('':5*Lev,Node^.Name^);
     PNode(Node^.Right,Lev+1);
    END;
END;

BEGIN
   PNode(Root,0);
END;

END.
