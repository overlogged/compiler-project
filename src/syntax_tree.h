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

struct syntax_expr
{
    syntax_type type;
    std::variant<std::shared_ptr<syntax_fun_call>> val; // todo: constant
};

struct syntax_if_block
{
};

struct syntax_environment
{
};

void syntax_analysis(node_module module);
