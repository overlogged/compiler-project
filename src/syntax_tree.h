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
};

struct syntax_expr
{
    syntax_type type;
    std::variant<std::shared_ptr<syntax_fun_call>, syntax_literal, syntax_var> val;
};

struct syntax_if_block
{
};

struct syntax_environment
{
};

void syntax_analysis(node_module module);
