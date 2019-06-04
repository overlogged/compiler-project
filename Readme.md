# 编译原理大作业 Ice Cola 语言

## 编译环境

- 支持 c++17 的 c++ 编译期，如 g++ 7.0 以上，clang 5.0 以上，别名统一为 gcc 和 g++。
- flex 2.5 以上，bison 3.0 以上
- llvm 6.0 以上
- make

推荐 IDE：vscode 或 CLion，编译项目使用 Makefile，CMakeLists.txt 是为了让 CLion 进行代码提示。

## 使用说明

`make all` 编译(CMakeList.txt 仅作为 IDE 配置，不用做编译配置)

bin 目录下会出现编译器 icecola

编译命令:
`icecola xx.ic -o xx.out`

代码例子在 examples 目录下