var x:(x:i8, i8) = {x=8, 8};
var y:i32 = fun(4+3*fun(3));

fn fun(x:i8) i8 {
    if x == 5 {
        return 4;
    } else {
        return 5;
    }
}

fn main() i32 {
    x.x = 1;
    x = {x=3,5};
}