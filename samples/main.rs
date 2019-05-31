val x:i32 = 0;
var y = fun(3+3*6);

fn fun(x:i8) i8 {
    if x==0 {
        return 32;
    } else if x==1 {
        return 3;
    } else {
        return 5;
    }
}

fn main() {
    val y:i8 = 1;
    val z = fun(y);
}