#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include "utils.h"

inline std::string to_string(const std::string &s)
{
    return "\"" + s + "\"";
}

// to_string for vector
template <typename T>
std::string to_string(const std::vector<T> &arr)
{
    if (arr.empty())
        return "[]";
    std::string ret = "[";
    for (auto i = 0; i < arr.size(); i++)
    {
        if (i)
        {
            ret.append(",");
        }
        auto s = to_string(arr[i]);
        ret.append(s);
    }
    ret.append("]");
    return ret;
}
//to_string for 2 types variant
template <typename T1,typename T2>
std::string to_string(const std::variant<T1,T2>& node)
{
    auto pval1 = std::get_if<T1>(&node);
    auto pval2 = std::get_if<T2>(&node);
    if(pval1)
        return to_string(*pval1);
    else if(pval2)
        return to_string(*pval2);
    else
        assert(false);
}
//to_string for sharad_ptr<>
template<typename T>
std::string to_string(const std::shared_ptr<T>& node)
{
    assert(node.get());
    return to_string(*node.get());
}

// ============================================================
//node_exp
struct node_exp1alt;
struct node_exp2alt;
struct node_exp3alt;

using node_exp3 = node_exp3alt;
using node_exp2 = std::variant<std::shared_ptr<node_exp2alt>,std::shared_ptr<node_exp3>>;
using node_exp1 = std::variant<std::shared_ptr<node_exp1alt>,std::shared_ptr<node_exp2>>;
using node_exp_list = std::vector<std::shared_ptr<node_exp1>>;
using node_exp4 = std::variant<std::shared_ptr<node_exp1>,std::string>;
struct node_exp1alt
{
    std::string op;
    std::shared_ptr<node_exp1> left_val;
    std::shared_ptr<node_exp2> right_val;
};
struct node_exp2alt
{
    std::shared_ptr<node_exp2> condition;
    std::shared_ptr<node_exp2> true_val;
    std::shared_ptr<node_exp3> false_val;
};
struct node_exp3alt
{
    std::shared_ptr<vec_str> ops;
    std::shared_ptr<std::vector<node_exp4>> vars;
};

inline std::string to_string(const node_exp1alt& node)
{
    return obj_to_string(
        vec_str{"op","left_val","right_val"},
        vec_str{to_string(node.op),
                to_string(node.left_val),
                to_string(node.right_val)});
}
inline std::string to_string(const node_exp2alt& node)
{
    return obj_to_string(
    vec_str{"condition","true_val","false_val"},
    vec_str{to_string(node.condition),
            to_string(node.true_val),
            to_string(node.false_val)});
}
inline std::string to_string(const node_exp3alt& node)
{
    return obj_to_string(
    vec_str{"ops","vars"},
    vec_str{to_string(node.ops),
            to_string(node.vars)});
}
// node_var_name_type
struct node_var_name_type
{
    std::string name, type;
};

inline std::string to_string(const node_var_name_type &node)
{
    return obj_to_string(
        vec_str{"name", "type"},
        vec_str{
            to_string(node.name),
            to_string(node.type)});
}
// node_parameters
struct node_parameters
{
    std::vector<node_var_name_type> params;
};

inline std::string to_string(const node_parameters &node)
{
    return obj_to_string(
        vec_str{"params"},
        vec_str{to_string(node.params)});
}


// node_function_block
struct node_function_block
{
    std::string fun_name;
    std::vector<node_var_name_type> params;
    std::string ret_type;
    std::shared_ptr<node_exp_list> exp_list;
};

inline std::string to_string(const node_function_block &node)
{
    return obj_to_string(
        vec_str{"fun_name", "params", "ret_type","exp_list"},
        vec_str{
            to_string(node.fun_name),
            to_string(node.params),
            to_string(node.ret_type),
            to_string(node.exp_list)});
}

// node_block
using node_block = std::variant<node_function_block>;
inline std::string to_string(const node_block &node)
{
    if (auto x = std::get_if<node_function_block>(&node))
    {
        return to_string(*x);
    }
    else
    {
        assert(false);
    }
}

// node_module
struct node_module
{
    std::vector<node_block> blocks;
};

inline std::string to_string(const node_module &node)
{
    return obj_to_string(vec_str{"blocks"}, vec_str{to_string(node.blocks)});
}

