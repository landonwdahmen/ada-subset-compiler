procedure six is
x, y, som: integer;
procedure something (a: integer; b: integer; out sum:integer) is
begin
    sum := a + b;
    put("Value is: ");
    putln(sum);
end something;
begin
    x:= 1;
    y:= 2;
    something(x, y, som);
    put("Value is still: ");
    putln(som);
end six;