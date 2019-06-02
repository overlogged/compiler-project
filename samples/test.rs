var x:(i8, i8) = {8, 8};
var y:i32 = fun(4+3*fun(3));

fn fun(x:i8) i8 {
    if x == 0 {
        return 4;
    } else {
        return 5;
    }
}

fn main() i32 {
    x._0 = 1;
}