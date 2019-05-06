#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include "utils.h"

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
};

inline std::string to_string(const node_function_block &node)
{
    return obj_to_string(
        vec_str{"fun_name", "params", "ret_type"},
        vec_str{
            to_string(node.fun_name),
            to_string(node.params),
            to_string(node.ret_type)});
}

// node_block
using node_block = std::variant<node_function_block>;

// node_module
struct node_module
{
    std::vector<node_block> blocks;
};

inline std::string to_string(const node_module &node)
{
    return obj_to_string(vec_str{"blocks"}, vec_str{to_string(node.blocks)});
}
