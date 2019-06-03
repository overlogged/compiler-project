#include "codegen.h"

using namespace llvm;

// IR 层面，统统转换成浮点或整数处理
Type *codegen_llvm::type_primary(const primary_type &t)
{
    if (t.name == "f32")
    {
        return Type::getFloatTy(context);
    }
    else if (t.name == "f64")
    {
        return Type::getDoubleTy(context);
    }
    else if (t.name == "u7")
    {
        return IntegerType::getInt8Ty(context);
    }
    else if (t.name == "u15")
    {
        return IntegerType::getInt16Ty(context);
    }
    else if (t.name == "u31")
    {
        return IntegerType::getInt32Ty(context);
    }
    else if (t.name == "u63")
    {
        return IntegerType::getInt64Ty(context);
    }
    else if (t.name == "bool")
    {
        return IntegerType::getInt1Ty(context);
    }
    auto tt = Type::getInt128Ty(context);
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
        auto tf = llvm_type(*fields);
        members.push_back(tf);
    }
    return StructType::create(context, members, "product_type");
}

Type *codegen_llvm::type_sum(const sum_type &t)
{
    unsigned int max_size = 0;
    for (auto &alters : t.types)
    {
        auto tf = llvm_type(*alters);
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
    return StructType::create(context, members, "sum_type");
}

Type *codegen_llvm::llvm_type(const syntax_type &s)
{
    auto it = type_map.find(s.to_string());
    if (it != type_map.end())
    {
        return it->second;
    }

    Type *ret;
    if (auto p = std::get_if<primary_type>(&s.type))
    {
        ret = type_primary(*p);
    }
    else if (auto p = std::get_if<sum_type>(&s.type))
    {
        ret = type_sum(*p);
    }
    else if (auto p = std::get_if<product_type>(&s.type))
    {
        ret = type_product(*p);
    }
    else
    {
        throw std::string("bad type");
    }

    if (s.is_ref())
    {
        ret = PointerType::get(ret, 0);
    }
    type_map[s.to_string()] = ret;
    return ret;
}