#include "codegen.h"

using namespace llvm;

// todo: 完成字面量的处理
// 包括整数，浮点，字符，字符串
Value *codegen_llvm::get_lit(const syntax_literal &lit)
{
    auto type = llvm_type(lit.type);
    auto value = std::get<unsigned long long>(lit.val);
    return ConstantInt::get(type, value);
}

// todo: 完成对函数调用的处理
// 各种 builtin 函数
// 注意判断有无符号
Value *codegen_llvm::get_call(const syntax_fun_call &call)
{
    if (call.fun_name == "+")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateAdd(arg1, arg2, "addtmp");
    }
    else if (call.fun_name == "-")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateSub(arg1, arg2, "subtmp");
    }
    else if (call.fun_name == "*")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateMul(arg1, arg2, "multmp");
    }
    else if (call.fun_name == "/")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateSDiv(arg1, arg2, "divtmp");
    }
    else if (call.fun_name == "==")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmpEQ(arg1, arg2, "eq");
    }
    else
    {
        // 普通函数
        auto fun_decl = llvm_module->getFunction(call.fun_name);
        std::vector<Value *> args;
        for (auto &p : call.parameters)
        {
            args.push_back(get_value(p));
        }
        return builder->CreateCall(fun_decl, args, call.fun_name + "_result");
    }
}

// todo: 完成对类型转换的处理
// sum type, product type 等等
Value *codegen_llvm::get_convert(const syntax_type_convert &conv)
{
    auto t_size = conv.target_type.get_primary_size();
    if (t_size != 0)
    {
        return builder->CreateCast(Instruction::CastOps::ZExt, get_value(conv.source_expr), llvm_type(conv.target_type), "zext");
    }
    throw std::string("convert failed");
}

Value *codegen_llvm::get_dot(const syntax_dot &dot)
{
    auto inner_val = dot.val;
    auto inner_type = inner_val->type;
    auto inner_llvm_val = (Value *)inner_val->reserved;
    uint64_t index = 0;
    if (inner_type.is_sum())
    {
        index = 1;
        auto llvm_i32 = IntegerType::getInt32PtrTy(context);
        auto llvm_idx = ConstantInt::get(llvm_i32, index);
        auto gep = builder->CreateGEP(inner_llvm_val, llvm_idx);

        auto sum = std::get<sum_type>(inner_type.type);
        auto idx = sum.get_index(dot.field);
        assert(idx != -1);
        auto target_type = llvm_type(*sum.types[idx]);

        return builder->CreateCast(Instruction::CastOps::BitCast, gep, target_type, sum.alters[idx]);
    }
    else if (inner_type.is_product())
    {
        auto prod = std::get<product_type>(inner_type.type);
        index = prod.get_index(dot.field);
        assert(index != -1);
        auto llvm_i32 = IntegerType::getInt32PtrTy(context);
        auto llvm_idx = ConstantInt::get(llvm_i32, index);
        return builder->CreateGEP(inner_llvm_val, llvm_idx, prod.fields[index]);
    }
    else
    {
        throw std::string("invalid dot");
    }
}

// 处理 expression
Value *codegen_llvm::expression(std::shared_ptr<syntax_expr> expr)
{
    auto p_val = &expr->val;

    if (auto p_lit = std::get_if<syntax_literal>(p_val))
    {
        expr->reserved = get_lit(*p_lit);
    }
    else if (auto p_fun = std::get_if<syntax_fun_call>(p_val))
    {
        expr->reserved = get_call(*p_fun);
    }
    else if (auto p_var = std::get_if<syntax_var>(p_val))
    {
        expr->reserved = builder->CreateAlloca(llvm_type(expr->type));
    }
    else if (auto p_convert = std::get_if<syntax_type_convert>(p_val))
    {
        expr->reserved = get_convert(*p_convert);
    }
    else if (auto p_dot = std::get_if<syntax_dot>(p_val))
    {
        expr->reserved = get_dot(*p_dot);
    }
    else
    {
        assert(false);
    }

    return (Value *)expr->reserved;
}

void codegen_llvm::block_if(const syntax_if_block &syntax_if)
{
    // 计算条件
    block(syntax_if.cond_stmt);
    auto condv = expression(syntax_if.condition);

    // 创建分支块
    auto block_then = BasicBlock::Create(context, "then", func);
    auto block_else = BasicBlock::Create(context, "else");
    auto block_merge = BasicBlock::Create(context, "ifcont");

    builder->CreateCondBr(condv, block_then, block_else);

    // then
    builder->SetInsertPoint(block_then);
    block(syntax_if.then_stmt);
    builder->CreateBr(block_merge);

    // else
    builder->SetInsertPoint(block_else);
    block(syntax_if.else_stmt);
    builder->CreateBr(block_merge);
    block_else->insertInto(func);

    // merge
    builder->SetInsertPoint(block_merge);
    block_merge->insertInto(func);
}

// 处理 block
void codegen_llvm::block(const std::vector<syntax_stmt> &stmts)
{
    for (auto &s : stmts)
    {
        auto p_stmt = &s.stmt;

        // 处理各种 statement
        if (auto p_expr = std::get_if<std::shared_ptr<syntax_expr>>(p_stmt))
        {
            expression(*p_expr);
        }
        else if (auto p_assign = std::get_if<syntax_assign>(p_stmt))
        {
            builder->CreateStore(get_value(p_assign->rval), (Value *)p_assign->lval->reserved);
        }
        else if (auto p_ret = std::get_if<syntax_return>(p_stmt))
        {
            builder->CreateStore(get_value(p_ret->val), ret_value);
            builder->CreateBr(block_return);
        }
        // todo: for, while, new, delete
        else if (auto p_if = std::get_if<std::shared_ptr<syntax_if_block>>(p_stmt))
        {
            block_if(*p_if->get());
        }
    }
}

// 处理函数
void codegen_llvm::function(const std::string &fun_name, const std::vector<syntax_stmt> &stmts, const std::vector<std::shared_ptr<syntax_expr>> &args)
{
    // 定义函数体
    func = llvm_module->getFunction(fun_name);
    auto return_type = func->getReturnType();

    // 入口函数
    auto block_entry = BasicBlock::Create(context, "entry", func);
    builder->SetInsertPoint(block_entry);
    ret_value = builder->CreateAlloca(return_type, nullptr, "ret_value");
    builder->CreateStore(Constant::getNullValue(return_type), ret_value);

    // 初始化参数
    for (auto &arg : func->args())
    {
        auto idx = arg.getArgNo();
        auto var = builder->CreateAlloca(arg.getType());
        args[idx]->reserved = var;
        builder->CreateStore(&arg, var);
    }

    block_return = BasicBlock::Create(context, "return");
    builder->SetInsertPoint(block_return);
    builder->CreateRet(ret_value);

    builder->SetInsertPoint(block_entry);
    block(stmts);

    block_return->insertInto(func);
}
