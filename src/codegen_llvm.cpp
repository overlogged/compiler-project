#include "codegen.h"
#include <fstream>
#include "type.h"


using namespace llvm;

Type *codegen_llvm::type_primary(const primary_type &t)
{
    switch (t.size)
    {
    case 1:
        return IntegerType::getInt8Ty(context);
    case 2:
        return IntegerType::getInt16Ty(context);
    case 4:
        return IntegerType::getInt32Ty(context);
    case 8:
        return IntegerType::getInt64Ty(context);
    }
    throw std::string("type error");
}

Type *codegen_llvm::type_product(const product_type &t)
{
    std::vector<Type *> members;
    for (auto &fields : t.types)
    {
        auto tf = type(*fields);
        members.push_back(tf);
    }
    return StructType::create(context, members);
}

Type *codegen_llvm::type_sum(const sum_type &t)
{
    unsigned int max_size = 0;
    for (auto &alters : t.types)
    {
        auto tf = type(*alters);
        auto ts = tf->getPrimitiveSizeInBits();
        max_size = std::max(max_size, ts / 8);
        assert(ts % 8 == 0);
    }
    if (max_size == 0)
    {
        throw std::string("max_size == 0");
    }
    auto unions = ArrayType::get(IntegerType::getInt8Ty(context), max_size);
    std::vector<Type *> members{IntegerType::getInt32Ty(context), unions};
    return StructType::create(context, members);
}

Type *codegen_llvm::type(const syntax_type &s)
{
    if (auto p = std::get_if<primary_type>(&s.type))
    {
        return type_primary(*p);
    }
    else if (auto p = std::get_if<sum_type>(&s.type))
    {
        return type_sum(*p);
    }
    else if (auto p = std::get_if<product_type>(&s.type))
    {
        return type_product(*p);
    }
    else
    {
        assert(false);
    }
    return nullptr;
}

void codegen_llvm::codegen()
{
    auto p_module = std::make_unique<Module>("main", context);

    InitializeNativeTarget();

    // 第一部分，生成全局变量的声明

    // 第二部分，生成函数的内部

    // 输出
    std::error_code c;
    raw_fd_ostream fout(StringRef("samples/out.ll"), c, sys::fs::F_Text);
    fout << *p_module;
}