# 编译原理大作业

## 编译环境

- 支持 c++17 的 c++ 编译期，如 g++ 7.0 以上，clang 5.0 以上，别名统一为 gcc 和 g++。
- flex 2.5 以上，bison 3.2 以上
- make

推荐 IDE：vscode 或 CLion，编译项目使用 Makefile，CMakeLists.txt 是为了让 CLion 进行代码提示。

## 参考资料

- [Bison A Simple C++ Example](https://www.gnu.org/software/bison/manual/html_node/A-Simple-C_002b_002b-Example.html)
- [A Complete C++ Example](https://www.gnu.org/software/bison/manual/html_node/A-Complete-C_002b_002b-Example.html#A-Complete-C_002b_002b-Example)

## 备忘

%option noyywrap nounput noinput batch debug