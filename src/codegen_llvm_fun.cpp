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
    else if (call.fun_name == "s/")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateSDiv(arg1, arg2, "divtmp");
    }
    else if (call.fun_name == "u/")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateUDiv(arg1, arg2, "divtmp");
    }
    else if (call.fun_name == "==")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmpEQ(arg1, arg2, "eq");
    }
    else if (call.fun_name == "!=")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_UGT, arg1, arg2, "ne");
    }
    else if (call.fun_name == "&")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateAnd(arg1, arg2, "andtmp");
    }
    else if (call.fun_name == "|")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateOr(arg1, arg2, "ortmp");
    }
    else if (call.fun_name == "s%")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateSRem(arg1, arg2, "modtmp");
    }
    else if (call.fun_name == "u%")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateURem(arg1, arg2, "modtmp");
    }
    else if (call.fun_name == "l>>")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateLShr(arg1, arg2, "shifttmp");
    }
    else if (call.fun_name == "a>>")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateAShr(arg1, arg2, "shifttmp");
    }
    else if (call.fun_name == "<<")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateShl(arg1, arg2, "shifttmp");
    }
    else if (call.fun_name == "s>")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_SGT, arg1, arg2, "cmptmp");
    }
    else if (call.fun_name == "u>")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_UGT, arg1, arg2, "cmptmp");
    }
    else if (call.fun_name == "s<")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_SLT, arg1, arg2, "cmptmp");
    }
    else if (call.fun_name == "u<")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_ULT, arg1, arg2, "cmptmp");
    }
    else if (call.fun_name == "s>=")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_SGE, arg1, arg2, "cmptmp");
    }
    else if (call.fun_name == "u>=")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_UGE, arg1, arg2, "cmptmp");
    }
    else if (call.fun_name == "s<=")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_SLE, arg1, arg2, "cmptmp");
    }
    else if (call.fun_name == "u<=")
    {
        auto arg1 = get_value(call.parameters[0]);
        auto arg2 = get_value(call.parameters[1]);
        return builder->CreateICmp(CmpInst::ICMP_ULE, arg1, arg2, "cmptmp");
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

Value *codegen_llvm::get_convert(const syntax_type_convert &conv)
{
    auto t_size = conv.target_type.get_primary_size();
    if (t_size != 0)
    {
        if (conv.source_expr->type.get_primary()[0] == 'u' || conv.source_expr->type.get_primary() == "bool")
            return builder->CreateCast(Instruction::CastOps::ZExt, get_value(conv.source_expr), llvm_type(conv.target_type), "zext");
        else
            return builder->CreateCast(Instruction::CastOps::SExt, get_value(conv.source_expr), llvm_type(conv.target_type), "zext");
    }
    throw std::string("convert failed");
}

Value *codegen_llvm::get_dot(const syntax_dot &dot)
{
    auto inner_val = dot.val;
    auto inner_type = inner_val->type;
    auto inner_llvm_val = (Value *)inner_val->reserved;
    auto llvm_i32 = IntegerType::getInt32PtrTy(context);
    auto llvm_zero = ConstantInt::get(llvm_i32, 0);

    if (inner_type.is_sum())
    {
        if (dot.field == ".tag")
        {
            return builder->CreateStructGEP(inner_llvm_val, 0, "tag");
        }
        else
        {
            auto sum = std::get<sum_type>(inner_type.type);
            auto idx = sum.get_index(dot.field);
            assert(idx != -1);
            auto target_type = llvm_type(*sum.types[idx]);

            auto gep = builder->CreateStructGEP(inner_llvm_val, 1);
            return builder->CreateCast(Instruction::CastOps::BitCast, gep, target_type, sum.alters[idx]);
        }
    }
    else if (inner_type.is_product())
    {
        auto prod = std::get<product_type>(inner_type.type);
        uint64_t index = prod.get_index(dot.field);
        auto llvm_idx = ConstantInt::get(llvm_i32, index);
        return builder->CreateStructGEP(inner_llvm_val, index, prod.fields[index]);
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
    auto condv = get_value(syntax_if.condition);

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
void codegen_llvm::block_while(const syntax_while_block &syntax_while)
{
    // branch
    auto block_begin_test = BasicBlock::Create(context, "begin_test");
    auto block_loop = BasicBlock::Create(context, "loop");
    auto block_loop_end = BasicBlock::Create(context, "loop_end");

    builder->CreateBr(block_begin_test);
    // begin test
    builder->SetInsertPoint(block_begin_test);
    block(syntax_while.condition_stmt);
    builder->CreateCondBr(get_value(syntax_while.condition), block_loop, block_loop_end);
    block_begin_test->insertInto(func);

    // loop
    builder->SetInsertPoint(block_loop);
    block(syntax_while.body);
    builder->CreateBr(block_begin_test);
    block_loop->insertInto(func);
    // loop end
    builder->SetInsertPoint(block_loop_end);
    block_loop_end->insertInto(func);
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
            if (auto dot = std::get_if<syntax_dot>(&p_assign->lval->val))
            {
                builder->CreateStore(get_value(p_assign->rval), get_value(p_assign->lval));
            }
            else
            {
                builder->CreateStore(get_value(p_assign->rval), (Value *)p_assign->lval->reserved);
            }
        }
        else if (auto p_ret = std::get_if<syntax_return>(p_stmt))
        {
            builder->CreateStore(get_value(p_ret->val), ret_value);
            builder->CreateBr(block_return);
        }
        // todo: new, delete
        else if (auto p_if = std::get_if<std::shared_ptr<syntax_if_block>>(p_stmt))
        {
            block_if(*p_if->get());
        }
        else if (auto p_if = std::get_if<std::shared_ptr<syntax_while_block>>(p_stmt))
        {
            block_while(*p_if->get());
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
        var->setName("arg" + std::to_string(idx));
        args[idx]->reserved = var;
        builder->CreateStore(&arg, var);
    }

    // 返回值
    block_return = BasicBlock::Create(context, "return");
    builder->SetInsertPoint(block_return);
    auto ret_v = builder->CreateLoad(ret_value, ".retv");
    builder->CreateRet(ret_v);

    builder->SetInsertPoint(block_entry);
    block(stmts);
    builder->CreateBr(block_return);

    block_return->insertInto(func);
}

// 外部函数声明
void codegen_llvm::ext_function_dec()
{
    std::vector<llvm::Type *> printf_arg_types;
    printf_arg_types.push_back(llvm::Type::getInt8PtrTy(llvm_module->getContext()));

    llvm::FunctionType *printf_type =
        llvm::FunctionType::get(
            llvm::Type::getInt32Ty(llvm_module->getContext()), printf_arg_types, true);
    llvm::Function *func = llvm::Function::Create(
        printf_type, llvm::Function::ExternalLinkage,
        llvm::Twine("printf"),
        &*llvm_module);
    func->setCallingConv(llvm::CallingConv::C);
}