val x:i32 = 0;
val y:i32 = 3+3*6;

fn fun(x:i32) i32 {
    if x==0 {
        return 32;
    } else {
        if x==1 {
            return 3;
        } else {
            return 5;
        }
    }
}

fn main() unit {
    val y:i32 = 1;
    val z = fun(y);
}

fn __init() {
    
}