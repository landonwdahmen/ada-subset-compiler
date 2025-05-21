PROCEDURE main is
    a,b,x, sum:INTEGER;
PROCEDURE test1(d:INTEGER; e:INTEGER; OUT f:INTEGER) is
begin
    f := e * d + 30;
    putln(f);
    put (e);
    put (" * ");
    put(d);
    put(" + 30 = ");
    putln(f);
END test1;
BEGIN
    put("Enter value for a: ");
    get (a);
    put("Enter value for B: ");
    get (b);
    put("Enter value for x: ");
    get (x);
    put ("A is: ");
    putln(a);
    put("B is: ");
    putln(b);
    put("X is: ");
    putln(x);
    test1(a,b,x);
    sum := a + b + x;
    put (a);
    put(" + ");
    put(b);
    put ("+ ");
    put(x);
    put (" = ");
    putln(sum);
END main;