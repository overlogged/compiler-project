var x:(x:i8, i8) = {x=8, 8};
var y:i32 = fun(4+3*fun(3));
val z = {a=3,3,x=4,y=5};

fn fun(x:i8) i8 {
    if x == 5 {
        return 4;
    } else {
        return 5;
    }
}

fn fun2(x:(x:i8,i8)) i8 {
    return 0;
}

fn main() i32 {
    val count:u32 = 16;
    val s = new i32[count];
    x.x = 1;
    val hh = 3;
    x = {x=3,5};
    fun2({x=3,5,6});
    x = z;
    scanf("%d",&y);
    s[0] = 1;
    var ss = "hello world %d y:%d";
    printf(ss,s[0],y);
}