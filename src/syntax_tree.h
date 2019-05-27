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
    int alloc_type; // 0 for stack, 1 for heap, 2 for static
    static const int STACK = 0;
    static const int HEAP = 1;
    static const int STATIC = 2;
};

struct syntax_type_convert
{
    std::shared_ptr<syntax_expr> source_expr;
    syntax_type target_type;
};

struct syntax_expr
{
    syntax_type type;
    std::variant<syntax_fun_call, syntax_literal, syntax_var, syntax_type_convert> val;
    void *reserved = nullptr;
};

struct syntax_assign
{
    std::shared_ptr<syntax_expr> lval, rval;
};

struct syntax_if_block
{
    std::shared_ptr<syntax_expr> condition;
    std::vector<syntax_stmt> true_branch;
    std::vector<syntax_stmt> false_branch;
};

struct syntax_while_block
{
    std::shared_ptr<syntax_expr> condition;
    std::vector<syntax_stmt> body;
};

struct syntax_stmt
{
    std::variant<std::shared_ptr<syntax_expr>, syntax_if_block, syntax_while_block, syntax_assign> stmt;
};

class syntax_module
{
    std::shared_ptr<syntax_expr> expr_analysis(const node_expression &node, std::vector<syntax_stmt> &stmts);

public:
    type_table env_type;
    function_table env_fun;
    stack_map<std::shared_ptr<syntax_expr>> env_var;

    void syntax_analysis(node_module module);
};