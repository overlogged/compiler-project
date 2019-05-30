#pragma once
#include "type.h"
#include "parse_tree.h"
#include <map>

struct syntax_expr;
struct syntax_stmt;

struct syntax_fun
{
    std::string fun_name;
    syntax_type ret_type;
    std::vector<std::pair<std::string, syntax_type>> parameters;
    std::vector<node_statement> origin_stmts;
};

struct syntax_fun_call
{
    std::string fun_name;
    std::vector<std::shared_ptr<syntax_expr>> parameters;
};

class function_table
{
    std::map<std::string, std::vector<syntax_fun>> normal_fun;
    std::map<std::string, std::vector<syntax_fun>> inline_fun;
    void create_bin_op_fun(std::string op, std::string ret_type, size_t ret_size, std::string param1_type, size_t param1_size, std::string param2_type, size_t param2_size);
    syntax_type infer_type_in_list(const std::string &func_name, syntax_fun_call &call, const std::map<std::string, std::vector<syntax_fun>> func_list, bool &find_flag);
    bool primary_match(std::string t1, std::string t2);
    void implicit_conv(std::shared_ptr<syntax_expr> param1, std::shared_ptr<syntax_expr> param2);
    void implicit_conv(std::shared_ptr<syntax_expr> param, const syntax_type fun_param_type);

public:
    function_table();

    void add_func(const syntax_fun &func)
    {
        // assert name is identifier
        // assert name is not used
        if (normal_fun[func.fun_name].size() > 0)
            throw "redifine funciton" + func.fun_name;
        normal_fun[func.fun_name].push_back(func);
    }

    // 进行类型推导，并分析是否进行隐式类型转换
    std::shared_ptr<syntax_expr> infer_type(const std::string &func_name, syntax_fun_call &call);

    syntax_fun get_user_fun(const std::string &name) const
    {
        return normal_fun.at(name)[0];
    }
};