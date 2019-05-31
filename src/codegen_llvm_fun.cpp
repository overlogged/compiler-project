#include "codegen.h"

using namespace llvm;

void codegen_llvm::function(const std::string &fun_name, const std::vector<syntax_stmt> &stmts)
{
    // 定义函数体
    auto func = llvm_module->getFunction(fun_name);
    auto return_type = func->getReturnType();

    auto block_entry = BasicBlock::Create(context, "entry", func);
    builder->SetInsertPoint(block_entry);
    Value *ret_value = builder->CreateAlloca(return_type, nullptr, "ret_value");

    auto block_return = BasicBlock::Create(context, "return", func);
    builder->SetInsertPoint(block_return);
    builder->CreateRet(ret_value);

    for (auto &s : stmts)
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
                    auto fun_decl = llvm_module->getFunction(p_fun->fun_name);
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
        }
        else if (auto p_if = std::get_if<syntax_if_block>(p_stmt))
        {
        }
    }
}
