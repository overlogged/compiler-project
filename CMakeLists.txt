cmake_minimum_required (VERSION 3.0)

project (compiler)

add_compile_options(-std=c++17)
add_executable(compiler src/driver.cpp src/driver.h src/main.cpp src/parse_tree.h src/utils.cpp src/utils.h)