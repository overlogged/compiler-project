#pragma once

#include "syntax_tree.h"

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

class codegen_llvm
{
    const syntax_module &module;
    llvm::LLVMContext context;
    std::shared_ptr<llvm::Module> llvm_module;

    // 与当前状态相关
    llvm::Function *func;
    llvm::BasicBlock *block_return;
    llvm::Value *ret_value;
    llvm::IRBuilder<> *builder;

    llvm::Type *type_primary(const primary_type &t);
    llvm::Type *type_sum(const sum_type &t);
    llvm::Type *type_product(const product_type &t);
    llvm::Type *llvm_type(const syntax_type &s);

    llvm::Value *get_value(const std::shared_ptr<syntax_expr> &expr);

    llvm::Value *get_lit(const syntax_literal &lit);
    llvm::Value *get_call(const syntax_fun_call &call);
    llvm::Value *get_convert(const syntax_type_convert &conv);
    llvm::Value *get_dot(const syntax_dot &dot);

    void function(const std::string &fun_name, const std::vector<syntax_stmt> &stmts, const std::vector<std::shared_ptr<syntax_expr>> &args);
    void block(const std::vector<syntax_stmt> &stmts);
    void block_if(const syntax_if_block &syntax_if);
    void block_while(const syntax_while_block& syntax_while);

    llvm::Value *expression(std::shared_ptr<syntax_expr> expr);

public:
    codegen_llvm(const syntax_module &mod) : module(mod) {}

    void codegen();
};