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
        if(auto p = std::get_if<primary_type>(&type))
            return p->name;
        else
            return "";
    }

    // size_t get_size() const {
    //     if(auto p = std::get_if<primary_type>(&type)) {
    //         return p->size;
    //     } else if(auto p = std::get_if<product_type>(&type)) {
    //         return p->size;
    //     } else if(auto p = std::get_if<sum_type>(&type)) {
    //         return p->size;
    //     } else
    //         assert(false);
    // }

    // todo: subtyping 判定
    // type is subtype of t or not
    bool subtyping(const syntax_type &t) const
    {
        if(!get_primary().empty() && !t.get_primary().empty) {
            if(get_primary() == t.get_primary())
                return true;
            else
                return false;
        } else if(auto p = std::get_if<product_type>(&type)) {
            if(auto q = std::get_if<product_type>(&t.type)) {
                if(q->types.size() > p->types.size())
                    return false;
                for(auto i = 0; i < q->types.size();i++) {
                    if(p->fields[i] != q->fields[i])
                        return false;
                    auto type_sub = p->types[i];
                    if(!type_sub->subtyping(*(q->types[i])))
                        return false;
                }
                return true;
            } else
                return false;
        } else if(auto p = std::get_if<sum_type>(&type)) {
            if(auto q = std::get_if<sum_type>(&type)) {
                if(p->types.size() > q->types.size())
                    return false;
                for(auto i = 0; i < p->types.size();i++) {
                    if(p->alters[i] != q->alters[i])
                        return false;
                    auto type_sub = q->types[i];
                    if(!type_sub->subtyping(*(p->types[i])))
                        return false;
                }
                return true;
            } else
                return false;
        } else {
            assert(false);
        }
    }
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

    // size和offset暂时先放一下, 等想出比较好的方式的时候就加上
    // 大致需要先得到出现过的最大built-in类型的内存长度
    // 然后对各个类型进行布局/内存对齐
    syntax_type type_check(const node_type &node)
    {
        if (auto p_id = std::get_if<node_identifier>(&node.type_val)) {
            auto type_name = p_id->val;
            auto type = get_type(type_name);
            return type;
        }
        else if (auto p_prod = std::get_if<node_product_type>(&node.type_val)) {
            product_type prod_t;
            // size_t size = 0;
            for(auto i = 0; i < p_prod->element.size(); i++) {
                prod_t.fields.push_back(p_prod->lables[i]);
                if(auto type = std::get_if<node_identifier>(&(p_prod->element[i]))) {
                    auto type_name = type->val;
                    auto type_ret = get_type(type_name);
                    // size += type_ret.get_size();
                    prod_t.types.push_back(std::make_shared<syntax_type>(type_ret));
                } else if(auto type = std::get_if<std::shared_ptr<node_type>>(&(p_prod->element[i]))) {
                    auto type_ret = type_check(*(type->get()));
                    // size += type_ret.get_size();
                    prod_t.types.push_back(std::make_shared<syntax_type>(type_ret));
                } else
                    assert(false);
                // prod_t.offsets.push_back(size);
            }
            // prod_t.size = size;
            return syntax_type{prod_t};
        }
        else if (auto p_sum = std::get_if<node_sum_type>(&node.type_val)) {
            sum_type sum_t;
            // size_t size = 0;
            for(auto i = 0; i < p_sum->element.size(); i++) {
                sum_t.alters.push_back(p_sum->lables[i]);
                if(auto type = std::get_if<node_identifier>(&(p_sum->element[i]))) {
                    auto type_name = type->val;
                    auto type_ret = get_type(type_name);
                    // auto tmp_size = type_ret.get_size();
                    // if(size < tmp_size)
                    //     size = tmp_size;
                    sum_t.types.push_back(std::make_shared<syntax_type>(type_ret));
                } else if(auto type = std::get_if<std::shared_ptr<node_type>>(&(p_sum->element[i]))) {
                    auto type_ret = type_check(*(type->get()));
                    // auto tmp_size = type_ret.get_size();
                    // if(size < tmp_size)
                    //     size = tmp_size;
                    sum_t.types.push_back(std::make_shared<syntax_type>(type_ret));
                } else
                    assert(false);
            }
            // sum_t.size = size;
            return syntax_type{sum_t};
        }
        else {
            assert(false);
        }
    }
};