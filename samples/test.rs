var x:(x:i8|y:i8) = {x=8};
var z = "asf";

fn t() char {
    z[0] = 'b';
    return z[1];
}

fn main() i32 {
    t();
    if z[0]=='a' {
        return 2;
    } else {
        return 1;
    }
    z = "dsfafs";
}