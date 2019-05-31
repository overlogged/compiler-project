val x:i8 = 0;
var y = fun(4+3*fun(3));

fn fun(x:i8) i8 {
    return 5;
}

fn main() {
    val y:i8 = 1;
    val z = (y = fun(y+2));
    z = x = 3;
}