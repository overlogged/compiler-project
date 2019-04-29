cmake_minimum_required (VERSION 3.0)

project (compiler)

aux_source_directory(src SRC_FILES)
aux_source_directory(include HEAD_FILES)

add_executable(compiler ${SRC_FILES} ${HEAD_FILES})