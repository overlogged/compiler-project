# 语言设计

better c

todo: package，module

## 类型

- 基础类型
  - bool
  - u8,16,32,64
  - i8,16,32,64
  - f32,f64
  - char
- 复合类型
  - 结构体
  ```
  data A[T] = struct {
      x:bool,
      y:i8,
      z:T
  };
  ```
  - 联合
  ```
  data Maybe[T] = union {
      x:A,
      y:struct {
        z:bool,
        h:T
      }
  };
  ```

## 函数

```
fn fun[T](x:i8, f·: struct { g:T }):i8 = {
  //
  return 1;
}
```

```
fn fun[T](x,y):i8 = {
  
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

不要指针，不要强制类型转换
