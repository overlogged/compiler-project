#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include "utils.h"
#include "location.hh"

struct node_expression;
struct node_post_expr;

// node_types
struct node_type;

// node_identifier
struct node_identifier
{
    yy::location loc;
    std::string val;
};

inline std::string to_string(const node_identifier &node)
{
    return to_string(node.val);
}

// node_arugments
struct node_expression_list
{
    yy::location loc;
    std::vector<std::shared_ptr<node_expression>> arr;
};

inline std::string to_string(const node_expression_list &node)
{
    return to_string(node.arr);
}

// node_constant
struct node_constant
{
    yy::location loc;
    bool is_const;
    std::shared_ptr<node_type> type;
    std::string ori;
    std::variant<unsigned long long, double, float, std::string> val;
};

inline std::string to_string(const node_constant &node)
{
    return obj_to_string(vec_str{"is_const", "const_type", "origin", "value"}, vec_str{to_string(node.is_const), to_string(node.type), to_string(node.ori), to_string(node.val)});
}

// node_primary_expr
struct node_primary_expr
{
    yy::location loc;
    std::variant<node_identifier, node_constant, std::shared_ptr<node_expression>> val;
};

inline std::string to_string(const node_primary_expr &node)
{
    return to_string(node.val);
}

// node_post_type_check_expr
struct node_post_check_expr
{
    yy::location loc;
    node_identifier check_lable;
    std::shared_ptr<node_post_expr> check_exp;
};

inline std::string to_string(const node_post_check_expr &node)
{
    return obj_to_string(
        vec_str{"check_lable", "check_exp"},
        vec_str{to_string(node.check_lable), to_string(node.check_exp)});
}

// node_post_dot_expr
struct node_post_dot_expr
{
    yy::location loc;
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
    yy::location loc;
    std::shared_ptr<node_post_expr> callable;
    node_expression_list exp_list;
};

inline std::string to_string(const node_post_call_expr &node)
{
    return obj_to_string(
        vec_str{"callable", "arugments"},
        vec_str{to_string(node.callable), to_string(node.exp_list.arr)});
}

//node_post_arr_expr
struct node_post_arr_expr
{
    yy::location loc;
    std::shared_ptr<node_post_expr> arr;
    std::shared_ptr<node_expression> arr_index;
};
inline std::string to_string(const node_post_arr_expr &node)
{
    return obj_to_string(vec_str{"arr", "arr_index"}, vec_str{to_string(node.arr), to_string(node.arr_index)});
}

// node_post_expr
struct node_post_expr
{
    yy::location loc;
    std::variant<node_primary_expr, node_post_call_expr, node_post_dot_expr, node_post_check_expr, node_post_arr_expr> expr;
};

inline std::string to_string(const node_post_expr &node)
{
    return to_string(node.expr);
}

// node_unary_expr
struct node_unary_expr
{
    yy::location loc;
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
    yy::location loc;
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
    yy::location loc;
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

// node_construct_expr
struct node_construct_expr
{
    yy::location loc;
    std::vector<std::string> lable;
    std::vector<std::shared_ptr<node_expression>> init_val;
};
inline std::string to_string(const node_construct_expr &node)
{
    return obj_to_string(vec_str{"lable", "init_val"}, vec_str{to_string(node.lable), to_string(node.init_val)});
}

struct node_sum_type
{
    yy::location loc;
    std::vector<std::string> lables;
    std::vector<std::variant<node_identifier, std::shared_ptr<node_type>>> element;
};
inline std::string to_string(const node_sum_type &node)
{
    return obj_to_string(vec_str{"type_lable", "sum_type"}, vec_str{to_string(node.lables), to_string(node.element)});
}

struct node_product_type
{
    yy::location loc;
    std::vector<std::variant<node_identifier, std::shared_ptr<node_type>>> element;
    std::vector<std::string> lables;
};
inline std::string to_string(const node_product_type &node)
{
    return obj_to_string(vec_str{"type_lable", "product_type"}, vec_str{to_string(node.lables), to_string(node.element)});
}

struct node_type
{
    yy::location loc;
    bool is_pointer;
    std::variant<node_identifier, node_sum_type, node_product_type, std::shared_ptr<node_type>> type_val;
};
inline std::string to_string(const node_type &node)
{
    std::string lable;
    if (node.is_pointer)
        lable = "pointer_type";
    else
        lable = "type";
    return obj_to_string(vec_str{lable}, vec_str{to_string(node.type_val)});
}
const node_type node_type_auto = node_type{
    .loc = yy::location(),
    .is_pointer = false,
    .type_val = node_identifier{
        .loc = yy::location(),
        .val = "auto"}};

// node_new_expr
struct node_new_expr
{
    yy::location loc;
    node_type new_type;
    bool single_flag;
    std::shared_ptr<node_expression> size;
};
inline std::string to_string(const node_new_expr &node)
{
    if (node.single_flag)
        return obj_to_string(vec_str{"new_type"}, vec_str{to_string(node.new_type)});
    else
        return obj_to_string(vec_str{"new_type", "size"}, vec_str{to_string(node.new_type), to_string(node.size)});
}

// node_statement
struct node_statement;

struct node_expression
{
    yy::location loc;
    std::variant<node_binary_expr, node_assign_expr, node_construct_expr, node_new_expr> expr;
};
inline std::string to_string(const node_expression &node)
{
    return to_string(node.expr);
}

struct node_return_statement
{
    yy::location loc;
    node_expression expr;
};
inline std::string to_string(const node_return_statement &node)
{
    return obj_to_string(vec_str{"ret_expr"}, vec_str{to_string(node.expr)});
}

struct node_else_if_statement
{
    yy::location loc;
    std::vector<node_expression> else_if_condition;
    std::vector<std::vector<node_statement>> else_if_statement;
};
inline std::string to_string(const node_else_if_statement &node)
{
    return obj_to_string(vec_str{"else_if_condition", "else_if_statement"}, vec_str{to_string(node.else_if_condition), to_string(node.else_if_statement)});
}

struct node_if_statement
{
    yy::location loc;
    node_expression if_condition;
    std::vector<node_statement> if_statement;
    std::vector<node_statement> else_statement;
    node_else_if_statement else_if_statement;
};
inline std::string to_string(const node_if_statement &node)
{
    return obj_to_string(vec_str{"if_codition", "if_stmt_list", "else_if", "else_stmt_list"},
                         vec_str{to_string(node.if_condition), to_string(node.if_statement), to_string(node.else_if_statement), to_string(node.else_statement)});
}

struct node_while_statement
{
    yy::location loc;
    node_expression while_condition;
    std::vector<node_statement> loop_statement;
};
inline std::string to_string(const node_while_statement &node)
{
    return obj_to_string(vec_str{"while_codition", "while_stmt_list"}, vec_str{to_string(node.while_condition), to_string(node.loop_statement)});
}

struct node_for_statement
{
    yy::location loc;
    node_expression init_expr;
    node_expression begin_test;
    node_expression end_process;
    std::vector<node_statement> for_statement;
};
inline std::string to_string(const node_for_statement &node)
{
    return obj_to_string(vec_str{"init_expr", "begin_test", "end_process", "for_statement"},
                         vec_str{to_string(node.init_expr), to_string(node.begin_test), to_string(node.end_process), to_string(node.for_statement)});
}

struct node_var_def_statement
{
    yy::location loc;
    bool is_immutable;
    std::vector<node_identifier> var_list;
    node_type var_type;
    node_expression initial_exp;
};
inline std::string to_string(const node_var_def_statement &node)
{
    return obj_to_string(vec_str{"is_immutable", "var_list", "var_type", "initial_exp"}, vec_str{to_string(node.is_immutable), to_string(node.var_list), to_string(node.var_type), to_string(node.initial_exp)});
}

struct node_delete_statement
{
    yy::location loc;
    node_expression delete_expr;
};
inline std::string to_string(const node_delete_statement &node)
{
    return obj_to_string(vec_str{"delete_expr"}, vec_str{to_string(node.delete_expr)});
}

struct node_global_var_def_block
{
    yy::location loc;
    std::vector<node_var_def_statement> arr;
};
inline std::string to_string(const node_global_var_def_block &node)
{
    return to_string(node.arr);
}

struct node_statement
{
    yy::location loc;
    std::variant<
        node_return_statement,
        node_if_statement,
        node_while_statement,
        node_for_statement,
        node_expression,
        node_var_def_statement,
        node_delete_statement>
        statement;
};
inline std::string to_string(const node_statement &node)
{
    return to_string(node.statement);
}

// type def statetment
struct node_type_def_statement
{
    yy::location loc;
    std::string type_name;
    node_type type;
};
inline std::string to_string(const node_type_def_statement &node)
{
    return obj_to_string(vec_str{"type_name", "type"}, vec_str{to_string(node.type_name), to_string(node.type)});
}

struct node_global_type_def_block
{
    yy::location loc;
    std::vector<node_type_def_statement> arr;
};
inline std::string to_string(const node_global_type_def_block &node)
{
    return to_string(node.arr);
}

// node fun_param
struct node_fun_param
{
    yy::location loc;
    bool empty_flag;
    node_product_type params;
};
inline std::string to_string(const node_fun_param &node)
{
    if (node.empty_flag)
    {
        return to_string("");
    }
    else
    {
        return to_string(node.params);
    }
}

// node_function_block
struct node_function_block
{
    yy::location loc;
    node_identifier fun_name;
    node_fun_param params;
    node_type ret_type;
    std::vector<node_statement> statement_list;
    bool no_ret;
};
inline std::string to_string(const node_function_block &node)
{
    if (node.no_ret)
        return obj_to_string(
            vec_str{"fun_name", "params", "statement_list"},
            vec_str{
                to_string(node.fun_name),
                to_string(node.params),
                to_string(node.statement_list)});
    else
        return obj_to_string(
            vec_str{"fun_name", "params", "ret_type", "statement_list"},
            vec_str{
                to_string(node.fun_name),
                to_string(node.params),
                to_string(node.ret_type),
                to_string(node.statement_list)});
}

// node_block
struct node_block
{
    yy::location loc;
    std::variant<node_function_block, node_global_var_def_block, node_global_type_def_block> val;
};
inline std::string to_string(const node_block &node)
{
    return to_string(node.val);
}

// node_module
struct node_module
{
    yy::location loc;
    std::vector<node_block> blocks;
};
inline std::string to_string(const node_module &node)
{
    return obj_to_string(vec_str{"blocks"}, vec_str{to_string(node.blocks)});
}