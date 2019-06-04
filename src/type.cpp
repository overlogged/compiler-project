#include "type.h"

bool syntax_type::subtyping(const syntax_type &t) const
{
    // 指针类型
    if (is_ref())
    {
        if (t.is_ref())
        {
            auto a_de = de_ref();
            auto b_de = t.de_ref();

            return a_de.get_primary() == "unit" || a_de.type_equal(b_de);
        }
        else
        {
            return false;
        }
    }
    else
    {
        auto t1 = get_primary();
        auto t2 = t.get_primary();
        // 判断整形
        static std::map<std::string, int> int_table = {
            {"bool", 1}, {"u7", 3}, {"i8", 4}, {"u8", 5}, {"u15", 7}, {"i16", 8}, {"u16", 9}, {"u31", 11}, {"i32", 12}, {"u32", 13}, {"u63", 15}, {"i64", 16}, {"u64", 17}};

        // 基础类型
        if (!t1.empty() && !t2.empty())
        {
            if (t1 == t2)
            {
                return true;
            }

            if (t1 == "char" || t2 == "char")
            {
                return false;
            }

            if (t1 == "f32" || t1 == "f64" || t2 == "f32" || t2 == "f64")
            {
                if (t1 == "f32" && t2 == "f64")
                {
                    return true;
                }
                if (t2 == "f32" || t2 == "f64")
                {
                    auto it = int_table.find(t1);
                    return it != int_table.end();
                }
                return false;
            }

            if (int_table[t1] % 2 == int_table[t2] % 2)
            {
                if (int_table[t1] < int_table[t2])
                    return true;
                else
                    return false;
            }
            else if (int_table[t1] % 2 == 1 && int_table[t2] % 2 == 0)
            {
                if (int_table[t1] < int_table[t2])
                    return true;
                else
                    return false;
            }
            else
                return false;
        }
        // 积类型
        else if (auto p = std::get_if<product_type>(&type))
        {
            if (auto q = std::get_if<product_type>(&t.type))
            {
                if (q->types.size() > p->types.size())
                    return false;

                for (auto i = 0; i < q->types.size(); i++)
                {
                    auto idx = p->get_index(q->fields[i]);
                    if (idx != -1)
                    {
                        auto type_sub = p->types[idx];
                        if (!type_sub->subtyping(*(q->types[i])))
                            return false;
                    }
                    else
                    {
                        return false;
                    }
                }
                return true;
            }
            // 1 元素积类型 <= 和类型
            else if (auto q = std::get_if<sum_type>(&t.type))
            {
                if (p->types.size() == 1)
                {
                    auto p_type = p->types[0];
                    for (auto i = 0; i < q->alters.size(); i++)
                    {
                        if (p_type->subtyping(*(q->types[i])) && p->fields[0] == q->alters[i])
                        {
                            return true;
                        }
                    }
                }
                return false;
            }
            else
            {
                return false;
            }
        }
        // 和类型
        // 暂时不允许和类型到其他类型的 subtyping
        // else if (auto p = std::get_if<sum_type>(&type))
        // {
        //     if (auto q = std::get_if<sum_type>(&t.type))
        //     {
        //         if (p->types.size() > q->types.size())
        //             return false;
        //         for (auto i = 0; i < p->types.size(); i++)
        //         {
        //             if (p->alters[i] != q->alters[i])
        //                 return false;
        //             auto type_sub = q->types[i];
        //             if (!type_sub->type_equal(*(p->types[i])))
        //                 return false;
        //         }
        //         return true;
        //     }
        //     else
        //         return false;
        // }
    }

    return false;
}

bool syntax_type::type_equal(const syntax_type &t) const
{
    if (is_ref() != t.is_ref())
    {
        return false;
    }
    else if (is_ref() && t.is_ref())
    {
        return de_ref().type_equal(t.de_ref());
    }
    else if (!get_primary().empty() && !t.get_primary().empty())
    {
        return subtyping(t) && t.subtyping(*this);
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
    }
    return false;
}

syntax_type type_table::get_type(std::string name)
{
    if (name == ".string")
    {
        auto t = std::make_shared<syntax_type>();
        t->type = primary_type{
            .name = "char",
            .size = 1};
        return syntax_type{pointer_type{t}};
    }

    // 13 种基础类型
    static const std::string builtin_types[] =
        {"u7", "u15", "u31", "u63", "u8", "i8", "u16", "i16", "u32", "i32", "u64", "i64", "char", "unit", "bool", "f32", "f64", "auto"};
    static const size_t builtin_size[] = {1, 2, 4, 8, 1, 1, 2, 2, 4, 4, 8, 8, 1, 1, 1, 4, 8, 0};

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
        throw inner_error{INNER_NO_SUCH_TYPE, name};
    }
    return it->second;
}

syntax_type type_table::type_check(const node_type &node, top_graph *dependency_graph)
{
    if (auto p_id = std::get_if<node_identifier>(&node.type_val))
    {
        auto type_name = p_id->val;
        try
        {
            auto type = get_type(type_name);
            return type;
        }
        catch (inner_error &e)
        {
            if (dependency_graph == nullptr)
            {
                throw syntax_error(node.loc, "no such type '" + e.info + "'");
            }
            else
            {
                dependency_graph->changed = true;
                dependency_graph->add_node(type_name);
            }
        }
    }
    else if (auto p_prod = std::get_if<node_product_type>(&node.type_val))
    {
        product_type prod_t;
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
                catch (inner_error &e)
                {
                    if (dependency_graph == nullptr)
                    {
                        throw syntax_error(node.loc, "no such type '" + e.info + "'");
                    }
                    else
                    {
                        dependency_graph->changed = true;
                        dependency_graph->add_node(type_name);
                    }
                }
            }
            else if (auto type = std::get_if<std::shared_ptr<node_type>>(&(p_prod->element[i])))
            {
                auto type_ret = type_check(*(type->get()), dependency_graph);
                prod_t.types.push_back(std::make_shared<syntax_type>(type_ret));
            }
            else
                assert(false);
        }
        return syntax_type{prod_t};
    }
    else if (auto p_sum = std::get_if<node_sum_type>(&node.type_val))
    {
        sum_type sum_t;
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
                catch (inner_error &e)
                {
                    if (dependency_graph == nullptr)
                    {
                        throw syntax_error(node.loc, "no such type '" + e.info + "'");
                    }
                    else
                    {
                        dependency_graph->changed = true;
                        dependency_graph->add_node(type_name);
                    }
                }
            }
            else if (auto type = std::get_if<std::shared_ptr<node_type>>(&(p_sum->element[i])))
            {
                auto type_ret = type_check(*(type->get()), dependency_graph);
                sum_t.types.push_back(std::make_shared<syntax_type>(type_ret));
            }
            else
                assert(false);
        }
        // sum_t.size = size;
        return syntax_type{sum_t};
    }
    assert(false);
}

void product_type::fix()
{
    for (auto i = 0; i < types.size(); i++)
    {
        types[i]->fix();
    }
}

void sum_type::fix()
{
    for (auto i = 0; i < types.size(); i++)
    {
        types[i]->fix();
    }
}

void pointer_type::fix()
{
    type->fix();
}

std::string product_type::to_string() const
{
    std::string s = "(";
    for (auto i = 0; i < fields.size(); i++)
    {
        s += fields[i] + ":" + types[i]->to_string() + ",";
    }
    s += ")";
    return s;
}

std::string sum_type::to_string() const
{
    std::string s = "(";
    for (auto i = 0; i < alters.size(); i++)
    {
        s += alters[i] + ":" + types[i]->to_string() + "|";
    }
    s += ")";
    return s;
}

std::string pointer_type::to_string() const
{
    return type->to_string() + "*";
}