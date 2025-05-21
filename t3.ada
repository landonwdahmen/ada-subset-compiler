PROCEDURE one is
PROCEDURE two is
a,b,c:INTEGER;
BEGIN
a:= 5;
b:= 10;
c:= a + b;
put ("5 + 10 = ");
putln(c);
END two;
BEGIN
two();
END one;