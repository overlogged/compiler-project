#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <variant>
#include "utils.h"
#include "parse_tree.h"
#include "type.h"
#include "function.h"
#include "stack_map.h"

struct syntax_var_def
{
    std::string name;
    syntax_type type;
};

struct syntax_type_def
{
    std::string name;
    syntax_type type;
};

struct syntax_stmt;

struct syntax_literal
{
    syntax_type type;
    std::variant<unsigned long long, double, float, long double, char, std::string> val;
};

struct syntax_var
{
};

struct syntax_type_convert
{
    std::shared_ptr<syntax_expr> source_expr;
    syntax_type target_type;
};

struct syntax_dot
{
    std::string field;
    std::shared_ptr<syntax_expr> val;
};

struct syntax_construct
{
    std::vector<std::string> label;
    std::vector<std::shared_ptr<syntax_expr>> val;
};

struct syntax_expr
{
    bool is_immutale = false;
    syntax_type type;
    std::variant<syntax_fun_call, syntax_literal, syntax_var, syntax_type_convert, syntax_dot, syntax_construct> val;
    void *reserved = nullptr;
};

inline std::shared_ptr<syntax_expr> expr_convert_to(std::shared_ptr<syntax_expr> expr, const syntax_type &target)
{
    auto from_type = expr->type;
    auto to_type = target;
    if (from_type.subtyping(to_type))
    {
        auto ret = std::make_shared<syntax_expr>();
        ret->type = to_type;
        ret->val = syntax_type_convert{.source_expr = expr, .target_type = to_type};
        return ret;
    }
    throw inner_error{INNER_CANT_CAST};
}

struct syntax_assign
{
    std::shared_ptr<syntax_expr> lval, rval;
};

struct syntax_return
{
    std::shared_ptr<syntax_expr> val;
};

struct syntax_if_block
{
    std::vector<std::shared_ptr<syntax_expr>> condition;
    std::vector<std::vector<syntax_stmt>> condition_stmt;
    std::vector<std::vector<syntax_stmt>> branch;
    std::vector<syntax_stmt> defaul_branch;
};

struct syntax_while_block
{
    std::shared_ptr<syntax_expr> condition;
    std::vector<syntax_stmt> condition_stmt;
    std::vector<syntax_stmt> body;
};

struct syntax_stmt
{
    std::variant<std::shared_ptr<syntax_expr>, syntax_if_block, syntax_while_block, syntax_assign, syntax_return> stmt;
};

class syntax_module
{
    std::shared_ptr<syntax_expr> expr_analysis(const node_expression &node, std::vector<syntax_stmt> &stmts);
    std::shared_ptr<syntax_expr> binary_expr_analysis(const node_binary_expr &node, std::vector<syntax_stmt> &stmts);
    std::shared_ptr<syntax_expr> unary_expr_analysis(const node_unary_expr &node, std::vector<syntax_stmt> &stmts);
    std::shared_ptr<syntax_expr> post_expr_analysis(const node_post_expr &node, std::vector<syntax_stmt> &stmts);
    std::vector<syntax_fun> fundef_analysis(const node_module &module);
    void typedef_analysis(const node_module &module);
    void global_var_analysis(const node_module &module);
    void function_analysis(const syntax_fun &node);

    bool is_left_value(const syntax_expr &node);

    bool is_lvalue_immutale(const syntax_expr &node)
    {
        return node.is_immutale;
    }

    syntax_stmt if_analysis(const node_if_statement &node);
    syntax_stmt while_analysis(const node_while_statement &node);
    std::vector<syntax_stmt> statement_analysis(std::vector<node_statement> origin_stmts);

    void add_var(const node_var_def_statement &def, std::vector<syntax_stmt> &stmts);

public:
    type_table env_type;
    function_table env_fun;
    std::vector<std::pair<std::string, std::vector<syntax_stmt>>> fun_impl;
    stack_map<std::shared_ptr<syntax_expr>> env_var;

    void syntax_analysis(const node_module &module);
};