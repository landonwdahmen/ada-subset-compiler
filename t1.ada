procedure two is 
    procedure x is 
        a, b, cc, d:integer;
    begin
        put ("Enter a number ");
        get(a);
        b:=10;
        d:=20;
        cc:=d+a*b;
        put("The answer is ");
        put(cc);
    end x;
begin
    x();
end two;

--output:

-- proc x                       done
-- wrs _S0                      done
-- rdi _bp-2                    done
-- _bp-10 = 10                  done
-- _bp-4 = _bp-10               done   
-- _bp-12 = 20                  done
-- _bp-8 = _bp-12               done
-- _bp-16 = _bp-2 * _bp-4       done
-- _bp-14 = _bp-8 + _bp-16      done
-- _bp-6 = _bp-14               done
-- wrs _S1                      done
-- wri _bp-6                    done
-- wrIn                         done
-- endp x                       done
-- proc two                     done
-- call x                       done
-- endp two                     done
-- start proc two               done