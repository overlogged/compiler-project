#pragma once
#include <vector>
#include <string>
#include <memory>
#include <variant>
#include <exception>
#include "parse_tree.h"
#include "exception.h"
#include <iostream>

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

struct top_graph
{
    int in_node_num;
    int seq_num;
    std::map<int, std::string> name_table;
    std::map<std::string, int> type_reg_table;
    std::map<int, node_type> arr;
    std::map<int, std::vector<int>> adj_list;
    bool changed;
    top_graph() : seq_num(0), in_node_num(0), changed(false) {}
    void add_node(std::string type_name)
    {
        if (type_reg_table.count(type_name) == 0)
        {
            ++seq_num;
            type_reg_table[type_name] = seq_num;
            name_table[seq_num] = type_name;
            adj_list[seq_num].push_back(in_node_num);
        }
        else
        {
            adj_list[type_reg_table[type_name]].push_back(in_node_num);
        }
    }
    void add_node(std::string type_name, const node_type &node)
    {
        if (type_reg_table.count(type_name) == 0)
        {
            type_reg_table[type_name] = in_node_num;
            name_table[in_node_num] = type_name;
            arr[in_node_num] = node;
        }
        else
        {
            arr[type_reg_table[type_name]] = node;
        }
        ++seq_num;
        in_node_num = seq_num;
    }
    bool contain(std::string type_name)
    {
        return type_reg_table.count(type_name) != 0;
    }

    void set_internal_index(std::string type_name)
    {
        seq_num--;
        in_node_num = type_reg_table[type_name];
    }
    void reset_internal_index()
    {
        seq_num++;
        in_node_num = seq_num;
    }
};

struct syntax_type
{
    std::variant<primary_type, product_type, sum_type> type;

    syntax_type() = default;

    bool is_auto()
    {
        return get_primary() == "auto";
    }

    void print()
    {
        if (auto p = std::get_if<primary_type>(&type))
        {
            std::cout << p->name;
        }
        else if (auto p = std::get_if<product_type>(&type))
        {
            std::cout << "(";
            for (auto i = 0; i < p->types.size(); i++)
            {
                if (i != 0)
                {
                    std::cout << ',';
                }
                std::cout << p->fields[i] << ":";
                p->types[i].get()->print();
            }
            std::cout << ")";
        }
        else if (auto p = std::get_if<sum_type>(&type))
        {
            std::cout << "(";
            for (auto i = 0; i < p->types.size(); i++)
            {
                if (i != 0)
                {
                    std::cout << '|';
                }
                std::cout << p->alters[i] << ":";
                p->types[i].get()->print();
            }
            std::cout << ")";
        }
        else
            assert(false);
    }

    // 失败返回 ""
    std::string get_primary() const
    {
        if (auto p = std::get_if<primary_type>(&type))
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
        static std::map<std::string, int> table = {
            {"i8", 0}, {"u8", 1}, {"i16", 2}, {"u16", 3}, {"i32", 4}, {"u32", 5}, {"i64", 6}, {"u64", 7}};
        if (!get_primary().empty() && !t.get_primary().empty())
        {
            if (get_primary() == t.get_primary())
                return true;
            else if (table[get_primary()] % 2 == table[t.get_primary()] % 2)
            {
                if (table[get_primary()] < table[t.get_primary()])
                    return true;
                else
                    return false;
            }
            else if (table[get_primary()] % 2 == 1 && table[t.get_primary()] % 2 == 0)
            {
                if (table[get_primary()] < table[t.get_primary()])
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
        else if (auto p = std::get_if<product_type>(&type))
        {
            if (auto q = std::get_if<product_type>(&t.type))
            {
                if (q->types.size() > p->types.size())
                    return false;
                for (auto i = 0; i < q->types.size(); i++)
                {
                    if (p->fields[i] != q->fields[i])
                        return false;
                    auto type_sub = p->types[i];
                    if (!type_sub->type_equal(*(q->types[i])))
                        return false;
                }
                return true;
            }
            else
                return false;
        }
        else if (auto p = std::get_if<sum_type>(&type))
        {
            if (auto q = std::get_if<sum_type>(&t.type))
            {
                if (p->types.size() > q->types.size())
                    return false;
                for (auto i = 0; i < p->types.size(); i++)
                {
                    if (p->alters[i] != q->alters[i])
                        return false;
                    auto type_sub = q->types[i];
                    if (!type_sub->type_equal(*(p->types[i])))
                        return false;
                }
                return true;
            }
            else
                return false;
        }
        else
        {
            assert(false);
        }
    }

    bool type_equal(const syntax_type &t) const
    {
        if (!get_primary().empty() && !t.get_primary().empty())
        {
            if (get_primary() == t.get_primary())
                return true;
            else
                return false;
        }
        else if (auto p = std::get_if<product_type>(&type))
        {
            if (auto q = std::get_if<product_type>(&t.type))
            {
                if (p->types.size() != q->types.size())
                    return false;
                for (auto i = 0; i < p->types.size(); i++)
                {
                    if (p->fields[i] != q->fields[i])
                        return false;
                    auto type = p->types[i];
                    if (!type->type_equal(*(q->types[i])))
                        return false;
                }
                return true;
            }
            else
                return false;
        }
        else if (auto p = std::get_if<sum_type>(&type))
        {
            if (auto q = std::get_if<sum_type>(&t.type))
            {
                if (p->types.size() != q->types.size())
                    return false;
                for (auto i = 0; i < p->types.size(); i++)
                {
                    if (p->alters[i] != q->alters[i])
                        return false;
                    auto type = p->types[i];
                    if (!type->type_equal(*(q->types[i])))
                        return false;
                }
                return true;
            }
            else
                return false;
        }
        else
            assert(false);
    }
};

class type_table
{
public:
    std::map<std::string, syntax_type> user_def_type;

    const syntax_type primary_unit{primary_type{"unit", 1}};

    void print_type_table()
    {
        for (auto it : user_def_type)
        {
            std::cout << it.first << " : ";
            it.second.print();
            std::cout << '\n';
        }
    }

    void add_type(std::string type_name, const syntax_type &t)
    {
        user_def_type[type_name] = t;
    }

    syntax_type get_type(std::string name)
    {
        static const std::string builtin_types[] = {"u8", "i8", "u16", "i16", "u32", "i32", "u64", "i64", "char", "unit", "auto", "bool"};
        static const size_t builtin_size[] = {1, 1, 2, 2, 4, 4, 8, 8, 1, 1, 0, 1};
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
            throw std::string("no such type");
        }
        return it->second;
    }

    // size和offset暂时先放一下, 等想出比较好的方式的时候就加上
    // 大致需要先得到出现过的最大built-in类型的内存长度
    // 然后对各个类型进行布局/内存对齐
    syntax_type type_check(const node_type &node, top_graph *dependency_graph = nullptr)
    {
        if (auto p_id = std::get_if<node_identifier>(&node.type_val))
        {
            auto type_name = p_id->val;
            try
            {
                auto type = get_type(type_name);
                return type;
            }
            catch (std::string &e)
            {
                if (dependency_graph == nullptr)
                {
                    throw e;
                }
                else
                {
                    dependency_graph->changed = true;
                    dependency_graph->add_node(type_name);
                    throw inner_error(INNER_NOT_INFER_TYPE);
                }
            }
        }
        else if (auto p_prod = std::get_if<node_product_type>(&node.type_val))
        {
            product_type prod_t;
            // size_t size = 0;
            for (auto i = 0; i < p_prod->element.size(); i++)
            {
                prod_t.fields.push_back(p_prod->lables[i]);
                if (auto type = std::get_if<node_identifier>(&(p_prod->element[i])))
                {
                    auto type_name = type->val;
                    try
                    {
                        auto type_ret = get_type(type_name);
                        prod_t.types.push_back(std::make_shared<syntax_type>(type_ret));
                    }
                    catch (std::string &e)
                    {
                        if (dependency_graph == nullptr)
                        {
                            throw e;
                        }
                        else
                        {
                            dependency_graph->changed = true;
                            dependency_graph->add_node(type_name);
                        }
                    }
                    // size += type_ret.get_size();
                }
                else if (auto type = std::get_if<std::shared_ptr<node_type>>(&(p_prod->element[i])))
                {
                    auto type_ret = type_check(*(type->get()), dependency_graph);
                    // size += type_ret.get_size();
                    prod_t.types.push_back(std::make_shared<syntax_type>(type_ret));
                }
                else
                    assert(false);
                // prod_t.offsets.push_back(size);
            }
            // prod_t.size = size;
            return syntax_type{prod_t};
        }
        else if (auto p_sum = std::get_if<node_sum_type>(&node.type_val))
        {
            sum_type sum_t;
            // size_t size = 0;
            for (auto i = 0; i < p_sum->element.size(); i++)
            {
                sum_t.alters.push_back(p_sum->lables[i]);
                if (auto type = std::get_if<node_identifier>(&(p_sum->element[i])))
                {
                    auto type_name = type->val;
                    try
                    {
                        auto type_ret = get_type(type_name);
                        sum_t.types.push_back(std::make_shared<syntax_type>(type_ret));
                    }
                    catch (std::string &e)
                    {
                        if (dependency_graph == nullptr)
                        {
                            throw e;
                        }
                        else
                        {
                            dependency_graph->changed = true;
                            dependency_graph->add_node(type_name);
                        }
                    }
                    // auto tmp_size = type_ret.get_size();
                    // if(size < tmp_size)
                    //     size = tmp_size;
                }
                else if (auto type = std::get_if<std::shared_ptr<node_type>>(&(p_sum->element[i])))
                {
                    auto type_ret = type_check(*(type->get()), dependency_graph);
                    // auto tmp_size = type_ret.get_size();
                    // if(size < tmp_size)
                    //     size = tmp_size;
                    sum_t.types.push_back(std::make_shared<syntax_type>(type_ret));
                }
                else
                    assert(false);
            }
            // sum_t.size = size;
            return syntax_type{sum_t};
        }
        else
        {
            assert(false);
        }
    }
};