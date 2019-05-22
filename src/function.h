#pragma once
#include "type.h"
#include <map>

struct syntax_expr;

struct syntax_fun
{
    std::string fun_name;
    syntax_type ret_type;
    std::vector<std::pair<std::string, syntax_type>> parameters;
    // std::vector<syntax_stmt> stmts;
};

struct syntax_fun_call
{
    std::string fun_name;
    std::vector<std::shared_ptr<syntax_expr>> parameters;
};

class function_table
{
    std::map<std::string, std::vector<syntax_fun>> user_def_fun;

public:
    void add_func(const std::string &name, const syntax_fun &func)
    {
        // assert name is identifier
        // assert name is not used
        user_def_fun[name].push_back(func);
    }

    // 进行类型推导，并分析是否进行隐式类型转换
    std::shared_ptr<syntax_expr> infer_type(const std::string &func_name, const syntax_fun_call &call);
};