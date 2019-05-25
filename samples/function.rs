val x:(i32) =0;
var y:(u16) =0;
type t2 = ((x:i32, w:i32)|z:i8|a:u8);
type t1 = (x:i32|y:i32);
type t4 = (x:i32|y:i32|z:i8);
type t3 = (x:i32|w:u8|z:i8);
fn fun()
{
	x=1;
}
fn main() (i8,i32,u8) {
	if(x>1)
	{
		y=1;
	}
	else if(x>2)
	{
		y=2;
	}
	else if(x>3)
	{
		return y;
	}
	else
	{
		y=3;
	}
}