#include "codegen.h"
#include <fstream>
#include "type.h"

using namespace llvm;

llvm::Value *codegen_llvm::get_value(const std::shared_ptr<syntax_expr> &expr)
{
    if (expr->is_left_value())
    {
        return builder->CreateLoad((Value *)expr->reserved, "load");
    }
    else
    {
        return (Value *)expr->reserved;
    }
}

void codegen_llvm::codegen()
{
    llvm_module = std::make_shared<Module>("main", context);

    InitializeNativeTarget();
    builder = new IRBuilder<>(context);

    // 第一部分，生成全局变量的声明
    auto global_var = module.env_var.base();
    for (auto &pair : global_var)
    {
        auto name = pair.first;
        auto exp = pair.second;
        auto type = llvm_type(exp->type);

        auto global_var = new GlobalVariable(
            *llvm_module,
            type,
            false,
            GlobalVariable::CommonLinkage,
            Constant::getNullValue(type),
            name);

        exp->reserved = global_var;
        global_var->setAlignment(4);
    }

    // 第二部分，生成函数的声明
    ext_function_dec();
    for (auto i = 0; i < module.fun_name.size(); i++)
    {
        auto fun_name = module.fun_name[i];

        auto fun_decl = module.env_fun.get_user_fun(fun_name);

        // 声明
        std::vector<Type *> type_args;
        for (auto &p : fun_decl.parameters)
        {
            type_args.push_back(llvm_type(p.second));
        }
        auto fun_type = FunctionType::get(llvm_type(fun_decl.ret_type), type_args, false);
        Function::Create(fun_type, GlobalValue::ExternalLinkage, fun_name, llvm_module.get());
    }

    // 第三部分，生成函数的内部
    for (auto i = 0; i < module.fun_name.size(); i++)
    {
        function(module.fun_name[i], module.fun_impl[i], module.fun_args[i]);
    }

    // 输出
    std::error_code c;
    raw_fd_ostream fout(StringRef("a.ll"), c, sys::fs::F_Text);
    fout << *llvm_module;
}