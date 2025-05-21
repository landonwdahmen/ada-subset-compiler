procedure five is
x, y: integer;
procedure something (a: integer; b: integer) is
sum: integer;
begin
    sum := a + b;
    put("Value is: ");
    putln(sum);
end something;
begin
    x:= 5;
    y:= 16;
    something(x, y);
end five;