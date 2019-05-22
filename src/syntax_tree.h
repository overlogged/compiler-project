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

struct syntax_func
{
    std::string fun_name;
    syntax_type ret_type;
    std::vector<std::pair<std::string, syntax_type>> parameters;
    std::vector<syntax_stmt> stmts;
};

struct syntax_expr
{
};

struct syntax_if_block
{
};

struct syntax_environment
{
};

void syntax_analysis(node_module module);
