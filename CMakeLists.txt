cmake_minimum_required (VERSION 3.0)

project (compiler)

execute_process(COMMAND llvm-config --includedir OUTPUT_VARIABLE LLVM_INC)
execute_process(COMMAND llvm-config --ldflags --system-libs --libs support core irreader executionengine interpreter mc mcjit bitwriter x86codegen target OUTPUT_VARIABLE LLVM_LIB)

add_compile_options(-std=c++17 -g -I${LLVM_INC})
add_link_options(-L/usr/local/Cellar/llvm@7/7.0.1/lib -lLLVMX86CodeGen -lLLVMGlobalISel -lLLVMX86Desc -lLLVMX86Info -lLLVMMCDisassembler -lLLVMX86AsmPrinter -lLLVMX86Utils -lLLVMSelectionDAG -lLLVMAsmPrinter -lLLVMMCJIT -lLLVMInterpreter -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMAggressiveInstCombine -lLLVMTransformUtils -lLLVMBitWriter -lLLVMExecutionEngine -lLLVMTarget -lLLVMAnalysis -lLLVMProfileData -lLLVMRuntimeDyld -lLLVMObject -lLLVMMCParser -lLLVMMC -lLLVMDebugInfoCodeView -lLLVMDebugInfoMSF -lLLVMIRReader -lLLVMBitReader -lLLVMAsmParser -lLLVMCore -lLLVMBinaryFormat -lLLVMSupport -lLLVMDemangle -lz -lcurses -lm -lxml2 -lffi)

add_executable(compiler src/lexer.cpp src/parser.cpp src/driver.cpp src/driver.h src/syntax_tree.cpp src/codegen_llvm_type.cpp src/syntax_tree.h src/syntax_expr.cpp src/main.cpp src/parse_tree.h src/type.h src/utils.cpp src/utils.h src/codegen.h src/type.cpp src/codegen_llvm.cpp src/exception.h src/function.cpp src/function.h)