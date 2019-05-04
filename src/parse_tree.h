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

// ============================================================

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
struct node_exp1s;
std::string to_string(const node_exp1s& node);
struct node_function_block
{
    std::string fun_name;
    std::vector<node_var_name_type> params;
    std::string ret_type;
    node_exp1s* exp1s;
};

inline std::string to_string(const node_function_block &node)
{
    return obj_to_string(
        vec_str{"fun_name", "params", "ret_type","exps"},
        vec_str{
            to_string(node.fun_name),
            to_string(node.params),
            to_string(node.ret_type),
            to_string(*node.exp1s)});
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
//node_exp
struct node_exp1;
std::string to_string(const node_exp1& node);
struct node_exp1alt;
std::string to_string(const node_exp1alt& node);
struct node_exp2;
std::string to_string(const node_exp2& node);
struct node_exp2alt;
std::string to_string(const node_exp2alt& node);
struct node_exp3;
std::string to_string(const node_exp3& node);
struct  node_exp4;
std::string to_string(const node_exp4& node);

struct node_exp1
{
    std::variant<node_exp1alt*,node_exp2*> val;
};
inline std::string to_string(const node_exp1& node)
{
    auto pval = std::get_if<node_exp2*>(&node.val);
    if(pval)
    {
        return to_string(**pval);
    }
    else
    {
        return to_string(**std::get_if<node_exp1alt*>(&node.val));
    }
}
struct node_exp2
{
    std::variant<node_exp2alt*,node_exp3*> val;
};
inline std::string to_string(const node_exp2& node)
{
    std::cout<<"node_exp2"<<std::endl;
    auto pval = std::get_if<node_exp3*>(&node.val);
    if(pval)
        return to_string(**pval);
    else
        return to_string(**std::get_if<node_exp2alt*>(&node.val));
}
struct node_exp3
{
    vec_str ops;
    std::vector<node_exp4> vars;
};
inline std::string to_string(const node_exp3& node)
{
    std::cout<<"node_exp3"<<std::endl;
    return obj_to_string(
        vec_str{"ops","vals"},
        vec_str{
            to_string(node.ops),
            to_string(node.vars)});
}
struct  node_exp4
{
    std::variant<std::string,node_exp1*> val;
};
inline std::string to_string(const node_exp4& node)
{
    std::cout<<"node_exp4"<<std::endl;
    auto pval = std::get_if<std::string>(&node.val);
    if(pval)
        return to_string(*pval);
    else
        return to_string(**std::get_if<node_exp1*>(&node.val));
}
struct node_exp1alt
{
    std::string op;
    node_exp1 val0;
    node_exp2 val1;
};
inline std::string to_string(const node_exp1alt& node)
{
    return obj_to_string(
        vec_str{"op","val0","val1"},
        vec_str{
            to_string(node.op),
            to_string(node.val0),
            to_string(node.val1)});
}
struct node_exp2alt
{
    node_exp2 val0;
    node_exp2 val1;
    node_exp3 val2;
};
inline std::string to_string(const  node_exp2alt& node)
{
    return obj_to_string(
        vec_str{"val0","val1","val2"},
        vec_str{
            to_string(node.val0),
            to_string(node.val1),
            to_string(node.val2)});
}
struct  node_exp1s
{
    std::vector<node_exp1> exp1s;
};
inline std::string to_string(const node_exp1s& node)
{
    std::cout<<"to_string node_expalt: "<<std::get_if<node_exp1alt*>(&(node.exp1s[0].val))<<std::endl;
    std::cout<<"to_string node_exp2: "<<std::get_if<node_exp2*>(&(node.exp1s[0].val))<<std::endl;
    std::cout<<"to_string node_addr: "<<&node.exp1s[0]<<std::endl;
    return to_string(node.exp1s);
}