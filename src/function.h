#pragma once

#include "syntax_tree.h"

struct syntax_fun
{
    std::string fun_name;
    syntax_type ret_type;
    std::vector<std::pair<std::string, syntax_type>> parameters;
    std::vector<syntax_stmt> stmts;
};

struct syntax_fun_call
{
    std::string fun_name;
};

class function_table
{
    std::map<std::string, syntax_fun> user_def_fun;

public:
    void add_func(const std::string &name, const syntax_fun &func)
    {
        // assert name is identifier
        // assert name is not used
        user_def_fun[name] = func;
    }

    void infer_ret_type(const std::string &func_name, const syntax_fun_call &call);
};