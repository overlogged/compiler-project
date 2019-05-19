# 编译原理大作业

## 编译环境

- 支持 c++17 的 c++ 编译期，如 g++ 7.0 以上，clang 5.0 以上，别名统一为 gcc 和 g++。
- flex 2.5 以上，bison 3.0 以上
- make

推荐 IDE：vscode 或 CLion，编译项目使用 Makefile，CMakeLists.txt 是为了让 CLion 进行代码提示。

## 参考资料

- [Bison A Simple C++ Example](https://www.gnu.org/software/bison/manual/html_node/A-Simple-C_002b_002b-Example.html)
- [A Complete C++ Example](https://www.gnu.org/software/bison/manual/html_node/A-Complete-C_002b_002b-Example.html#A-Complete-C_002b_002b-Example)

## 使用说明

`make all` 编译
`make test` 测试，测试样例在 samples 中，以 .rs 结尾，测试输出 *.json，为 parse tree

## 代码文档

整体程序设计思路是轻语法分析，重语义分析。语法分析部分基本只做框架性的分析，更多内容放到语义分析里进行理解。

- driver.h driver.cpp

编译环境，用于保存语法分析过程中的位置信息、最终的 parse tree 结果

- lexer.ll

只需在代码中间仿照样例定义 token

- main.cpp

目前只接受一个参数，即输入文件，以后要能处理多个参数

- parse_tree.h

语法树节点的数据结构，如果遇到列表，用 std::vector，遇到多种情况，用 std::variant，其余情况用 std::string。
每种节点都需要定义 to_string 函数，仿照例子即可。

- parser.yy

仿照例子，在代码中间定义新的产生式及其处理函数。

- utils.h utils.cpp

工具函数集合

## 路径

先不做范型 []
- 字面量（数字、字符）
- 变量的定义（var/val x:t;)
- 运算符（C 风格）
- 类型（初等类型集合）
- if, for, while

可以做语义分析和代码生成

后面
- 类型加入结构体和联合、范型

## 备忘/提醒
- 加入了f128对应于long double
- 变量是否可变标志改为is_immutable
- 对于整数, 目前的规则是根据数的范围判断其类型, 默认为有符号, 如果指定了类型u/U/L/l/LL/ll, 当数在其能表示的范围内时为该类型, 否则为更大范围的数据类型
- 对于浮点数, 根据是否加入相关后缀l/L/f/F, 判断其类型, 默认为f64
- 语法树结点中带有具体类型的值
    - 对于整数, 考虑到整数的表示仅为位数的不同, 目前为unsigned long long, 在后续语义分析中可以方便的转换
    - 对于浮点数, float-f32, double-f64, long double-f128
- 字面量显示在json中时可能有位数差异(整数比较大或者浮点精度问题), 在实际存储的value中并没有这个问题