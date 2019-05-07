#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include "utils.h"

struct node_expression;
struct node_post_expr;

// node_identifier
struct node_identifier
{
    std::string val;
};

inline std::string to_string(const node_identifier &node)
{
    return to_string(node.val);
}

// node_arugments
using node_arguments = std::vector<std::shared_ptr<node_expression>>;

// node_constant
struct node_constant
{
    std::string val;
};

inline std::string to_string(const node_constant &node)
{
    return to_string(node.val);
}

// node_type
struct node_type
{
    std::string val;
};

inline std::string to_string(const node_type &node)
{
    return to_string(node.val);
}

// node_var_name_type
struct node_var_name_type
{
    node_identifier name;
    node_type type;
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

// node_primary_expr
using node_primary_expr = std::variant<node_identifier, node_constant, std::shared_ptr<node_expression>>;

// node_post_dot_expr
struct node_post_dot_expr
{
    std::shared_ptr<node_post_expr> obj;
    node_identifier attr;
};

inline std::string to_string(const node_post_dot_expr &node)
{
    return obj_to_string(
        vec_str{"obj", "attr"},
        vec_str{to_string(node.obj), to_string(node.attr)});
}

// node_post_call_expr
struct node_post_call_expr
{
    std::shared_ptr<node_post_expr> callable;
    node_arguments arguments;
};

inline std::string to_string(const node_post_call_expr &node)
{
    return obj_to_string(
        vec_str{"callable", "arugments"},
        vec_str{to_string(node.callable), to_string(node.arguments)});
}

// node_post_expr
struct node_post_expr
{
    std::variant<node_primary_expr, node_post_call_expr, node_post_dot_expr> expr;
};

inline std::string to_string(const node_post_expr &node)
{
    return obj_to_string(
        vec_str{"expr"},
        vec_str{to_string(node.expr)});
}

// node_unary_expr
struct node_unary_expr
{
    std::vector<std::string> ops;
    std::shared_ptr<node_post_expr> post_expr;
};

inline std::string to_string(const node_unary_expr &node)
{
    return obj_to_string(
        vec_str{"ops", "post_expr"},
        vec_str{to_string(node.ops), to_string(node.post_expr)});
}

// node_binary_expr
struct node_binary_expr
{
    std::vector<node_unary_expr> vars;
    std::vector<std::string> ops;
};

inline std::string to_string(const node_binary_expr &node)
{
    return obj_to_string(
        vec_str{"vars", "ops"},
        vec_str{to_string(node.vars), to_string(node.ops)});
}

// node_assign_expr
struct node_assign_expr
{
    node_unary_expr lval;
    std::string op;
    std::shared_ptr<node_expression> rval;
};

inline std::string to_string(const node_assign_expr &node)
{
    return obj_to_string(
        vec_str{"lval", "op", "rval"},
        vec_str{to_string(node.lval),
                to_string(node.op),
                to_string(node.rval)});
}

// node_expression
struct node_expression
{
    std::variant<node_binary_expr, node_assign_expr> expr;
};

inline std::string to_string(const node_expression &node)
{
    return obj_to_string(
        vec_str{"expr"},
        vec_str{to_string(node.expr)});
}

// node_statement
using node_statement = std::variant<std::shared_ptr<node_expression>>;

// node_function_block
struct node_function_block
{
    node_identifier fun_name;
    std::vector<node_var_name_type> params;
    node_type ret_type;
    std::vector<node_statement> statement_list;
};

inline std::string to_string(const node_function_block &node)
{
    return obj_to_string(
        vec_str{"fun_name", "params", "ret_type", "statement_list"},
        vec_str{
            to_string(node.fun_name),
            to_string(node.params),
            to_string(node.ret_type),
            to_string(node.statement_list)});
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
