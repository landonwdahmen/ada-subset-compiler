procedure three is 
a, b: integer;
    procedure addNums(x, y:integer) is
        sum: integer;
    begin
        put ("Enter a number: ");
        get (x);
        put ("Enter another number: ");
        get(y);
        sum:= x + y;
        put ("The sum is: ");
        putln(sum);
    end addNums;

begin
    addNums (a, b);
end three;