        PROGRAM Comp;
VAR
   a,b:LongInt;
BEGIN
   Write('A:'); ReadLn(A);
   Write('B:'); ReadLn(B);
   IF A>B THEN WriteLn('groesser')
   ELSE IF A<B THEN WriteLn('kleiner')
   ELSE WriteLn('gleich');
END.

