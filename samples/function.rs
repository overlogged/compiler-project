val x:(i32) =0;
var y:(u16) =0;
type t1 = (x:i32,y:i32);
type t2 = (x:i32|y:i32);
fn fun()
{
	x=1;
}
fn main() (i8) {
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