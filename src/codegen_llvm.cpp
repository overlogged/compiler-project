#include "codegen.h"
#include <fstream>
#include "type.h"

using namespace llvm;

llvm::Value *codegen_llvm::get_value(const std::shared_ptr<syntax_expr> &expr)
{
    if (auto pvar = std::get_if<syntax_var>(&expr->val))
    {
        return builder->CreateLoad((Value *)expr->reserved);
    }
    else
    {
        return (Value *)expr->reserved;
    }
}

void codegen_llvm::codegen()
{
    auto p_module = std::make_shared<Module>("main", context);

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
            *p_module,
            type,
            false,
            GlobalVariable::CommonLinkage,
            Constant::getNullValue(type),
            name);

        exp->reserved = global_var;
        global_var->setAlignment(4);
    }

    // 第二部分，生成函数的声明
    for (auto &fun : module.fun_impl)
    {
        auto fun_name = fun.first;
        auto fun_decl = module.env_fun.get_user_fun(fun_name);

        // 声明
        std::vector<Type *> type_args;
        for (auto &p : fun_decl.parameters)
        {
            type_args.push_back(llvm_type(p.second));
        }
        auto fun_type = FunctionType::get(llvm_type(fun_decl.ret_type), type_args, false);
        Function::Create(fun_type, GlobalValue::ExternalLinkage, fun_name, p_module.get());
    }

    // 第三部分，生成函数的内部
    for (auto &fun : module.fun_impl)
    {
        // 定义函数体
        bool has_ret = false;

        auto func = p_module->getFunction(fun.first);
        auto block_entry = BasicBlock::Create(context, "entry", func);
        builder->SetInsertPoint(block_entry);
        for (auto &s : fun.second)
        {
            auto p_stmt = &s.stmt;

            // 处理各种 statement
            if (auto p_expr = std::get_if<std::shared_ptr<syntax_expr>>(p_stmt))
            {
                // 各种 expr
                auto expr = *p_expr;
                auto p_val = &expr->val;

                // todo: 处理各种字面量
                if (auto p_lit = std::get_if<syntax_literal>(p_val))
                {
                    assert(p_lit->type.get_primary() != "");
                    auto type = llvm_type(p_lit->type);
                    auto value = std::get<unsigned long long>(p_lit->val);
                    expr->reserved = ConstantInt::get(type, value);
                }
                else if (auto p_fun = std::get_if<syntax_fun_call>(p_val))
                {
                    // todo: 在赋值和函数调用时，需要对 var 类型解引用
                    // !important

                    if (p_fun->fun_name == "+")
                    {
                        auto arg1 = get_value(p_fun->parameters[0]);
                        auto arg2 = get_value(p_fun->parameters[1]);
                        expr->reserved = builder->CreateAdd(arg1, arg2, "addtmp");
                    }
                    else if (p_fun->fun_name == "-")
                    {
                        auto arg1 = get_value(p_fun->parameters[0]);
                        auto arg2 = get_value(p_fun->parameters[1]);
                        expr->reserved = builder->CreateSub(arg1, arg2, "subtmp");
                    }
                    else if (p_fun->fun_name == "*")
                    {
                        auto arg1 = get_value(p_fun->parameters[0]);
                        auto arg2 = get_value(p_fun->parameters[1]);
                        expr->reserved = builder->CreateMul(arg1, arg2, "multmp");
                    }
                    else if (p_fun->fun_name == "/")
                    {
                        auto arg1 = get_value(p_fun->parameters[0]);
                        auto arg2 = get_value(p_fun->parameters[1]);
                        expr->reserved = builder->CreateSDiv(arg1, arg2, "divtmp");
                    }
                    else
                    {
                        // 普通函数
                        auto fun_decl = p_module->getFunction(p_fun->fun_name);
                        std::vector<Value *> args;
                        for (auto &p : p_fun->parameters)
                        {
                            args.push_back(get_value(p));
                        }
                        expr->reserved = builder->CreateCall(fun_decl, args, p_fun->fun_name + "_result");
                    }
                }
                else if (auto p_var = std::get_if<syntax_var>(p_val))
                {
                    expr->reserved = builder->CreateAlloca(llvm_type(expr->type));
                }
                else if (auto p_convert = std::get_if<syntax_type_convert>(p_val))
                {
                    auto t_size = p_convert->target_type.get_primary_size();
                    if (t_size != 0)
                    {
                        expr->reserved = builder->CreateCast(Instruction::CastOps::ZExt, get_value(p_convert->source_expr), llvm_type(p_convert->target_type), "zext");
                    }
                }
            }
            else if (auto p_assign = std::get_if<syntax_assign>(p_stmt))
            {
                builder->CreateStore(get_value(p_assign->rval), (Value *)p_assign->lval->reserved);
            }
            else if (auto p_ret = std::get_if<syntax_return>(p_stmt))
            {
                builder->CreateRet(get_value(p_ret->val));
                has_ret = true;
            }
            else if (auto p_if = std::get_if<syntax_if_block>(p_stmt))
            {
            }
        }
        if (!has_ret)
        {
            builder->CreateRet(Constant::getNullValue(func->getReturnType()));
        }
    }
    // 输出
    std::error_code c;
    raw_fd_ostream fout(StringRef("samples/out.ll"), c, sys::fs::F_Text);
    fout << *p_module;
}