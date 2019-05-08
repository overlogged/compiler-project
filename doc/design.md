# 基本语言设计

better c

todo: package，module

## 类型

- 基础类型
  - bool
  - u8,16,32,64
  - i8,16,32,64
  - f32,f64
  - char
  - null
- 复合类型
  - 结构体
  ```
  data A[T] = (x:bool, y:i32, z:T)
  data B = (bool,y:i32)
  ```
  - 联合
  ```
  data Maybe[T] = x:bool | a: (y:i8, h:i32) | z:T
  ```


## 函数

```
fn fun(x:&i8 | null) {
  if x.&i8? {

  } else {

  }
}

fn fun(x:&A[i8]):u8 {

}

fn fun[T](x:i8, f: (T g) ):i8 {
  //
  return 1;
  val x = (y:1,z:6)
}
```

## 函数调用

```
a.fun(b)
fun(a,b)
```
等价

## 运算符

和 C 语言一样

## 变量
```
val x: i8 = 0
var y: u32 = 1
```

## 控制语句
```
if xxx {

} else if xxx {

} else {

}


while xxx {

}

for i in range(1,10) {

}
```

## 其他

指针的目的：call by name；堆内存管理。不允许加减和随意赋值。