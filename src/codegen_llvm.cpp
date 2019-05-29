#include "codegen.h"
#include <fstream>

#include "llvm/IR/Verifier.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FileSystem.h"

using namespace llvm;

void codegen_llvm::codegen()
{
    LLVMContext context;
    auto p_module = std::make_unique<Module>("main", context);

    InitializeNativeTarget();

    // 第一部分，生成复合类型的类型定义
    for (auto &type : module.env_type.user_def_type)
    {
    }
    // 一个 type pair = (u32,u32); 的例子
    if (debug_flag)
    {
        auto t_i32 = IntegerType::getInt32Ty(context);
        Type *args[] = {t_i32, t_i32};
        StructType::create(context, ArrayRef<Type *>(args, 2), "pair");
        auto t_pair = p_module->getTypeByName("pair");
        t_pair->print(outs());
    }

    // 第二部分，生成全局变量的声明

    // 第三部分，生成函数的内部

    // 输出
    std::error_code c;
    raw_fd_ostream fout(StringRef("samples/out.ll"), c, sys::fs::F_Text);
    fout << *p_module;
}