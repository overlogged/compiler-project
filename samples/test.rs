var x:(x:i8|y:i8) = {x=8};

fn main() i32 {
    val z = x.x;
    return z;
    if x.y? {
        return 2;
    } else {
        return 1;
    }
    x.y = 3;
}