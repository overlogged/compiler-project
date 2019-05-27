#pragma once

#include "syntax_tree.h"

class codegen_llvm
{
    const syntax_module &module;

public:
    codegen_llvm(const syntax_module &mod) : module(mod) {}

    void codegen();
};