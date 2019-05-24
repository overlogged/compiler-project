#pragma once
#include <vector>
#include <string>
#include <memory>
#include <variant>
#include "parse_tree.h"

struct syntax_type;

struct primary_type
{
    std::string name;
    size_t size;
};

struct product_type
{
    std::vector<std::string> fields;
    std::vector<std::shared_ptr<syntax_type>> types;
    std::vector<size_t> offsets;
    size_t size;
};

struct sum_type
{
    std::vector<std::string> alters;
    std::vector<std::shared_ptr<syntax_type>> types;
    size_t size; // include 4 bytes tag
};

struct syntax_type
{
    std::variant<primary_type, product_type, sum_type> type;

    syntax_type() = default;

    // 失败返回 ""
    std::string get_primary() const
    {
        auto p = std::get_if<primary_type>(&type);
        return p->name;
    }

    // todo: subtyping 判定
    bool subtyping(const syntax_type &t) const;
};

class type_table
{
    std::map<std::string, syntax_type> user_def_type;

public:
    const syntax_type primary_unit{primary_type{"unit", 1}};

    void add_type(std::string type_name, const syntax_type &t)
    {
        user_def_type[type_name] = t;
    }

    syntax_type get_type(std::string name)
    {
        static const std::string builtin_types[] = {"u8", "i8", "u16", "i16", "u32", "i32", "u64", "i64", "char", "unit"};
        static const size_t builtin_size[] = {1, 1, 2, 2, 4, 4, 8, 8, 1, 1};
        static const size_t builtin_n = sizeof(builtin_size) / sizeof(size_t);
        for (auto i = 0; i < builtin_n; i++)
        {
            auto t = builtin_types[i];
            if (name == t)
            {
                auto primary_t = primary_type{.name = t, .size = builtin_size[i]};
                return syntax_type{primary_t};
            }
        }
        auto it = user_def_type.find(name);
        if (it == user_def_type.end())
        {
            throw new std::string("no such type");
        }
        return it->second;
    }

    
    syntax_type type_check(const node_type &node)
    {
        if (auto p_id = std::get_if<node_identifier>(&node.type_val))
        {
            auto type_name = p_id->val;
            auto type = get_type(type_name);
            return type;
        }
        else if (auto p_prod = std::get_if<node_product_type>(&node.type_val))
        {
        }
    }
};