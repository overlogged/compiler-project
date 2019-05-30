; ModuleID = 'main'
source_filename = "main"

@x = common global i32 0, align 4
@y = common global i8 0, align 4

define i32 @main() {
entry:
  store i32 0, i32* @x
  %fun_result = call i8 @fun(i8 21)
  store i8 %fun_result, i8* @y
  %.main_result = call i8 @.main()
  ret i32 0
}

define i8 @fun(i8) {
entry:
  ret i8 5
}

define i8 @.main() {
entry:
  %0 = alloca i8
  store i8 1, i8* %0
  %1 = load i8, i8* %0
  %fun_result = call i8 @fun(i8 %1)
  %2 = alloca i8
  store i8 %fun_result, i8* %2
  ret i8 0
}
