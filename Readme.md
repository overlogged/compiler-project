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