cmake_minimum_required (VERSION 3.0)

project (compiler)

aux_source_directory(src SRC_FILES)

add_executable(compiler ${SRC_FILES})