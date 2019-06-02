#include "function.h"
#include "syntax_tree.h"
#include "exception.h"
#include <map>
#include <set>

using namespace std;

std::shared_ptr<syntax_expr>
function_table::infer_type(const std::string &func_name, syntax_fun_call &call, std::vector<syntax_stmt> &stmts)
{
    auto p_ret = make_shared<syntax_expr>();

    //.?
    if (func_name[0] == '.' && func_name[func_name.size() - 1] == '?')
    {
        bool match = false;
        std::string alt_name = func_name.substr(1, func_name.size() - 2);

        if (call.parameters.size() != 1)
            throw inner_error(INNER_NO_MATCH_FUN, func_name);

        auto ptr_sum_val = call.parameters[0];
        auto ptr_sum_type = std::get_if<sum_type>(&ptr_sum_val->type.type);

        if (!ptr_sum_type)
            throw inner_error(INNER_NO_MATCH_FUN, func_name);

        auto it_alt = ptr_sum_type->alters.begin();
        auto it_type = ptr_sum_type->types.begin();
        for (; it_alt != ptr_sum_type->alters.end(); it_alt++, it_type++)
        {
            if (alt_name == *it_alt)
            {
                match = true;
                p_ret->type = syntax_type{.type = primary_type{.name = "bool", .size = 1}};
                break;
            }
        }
        if (match)
        {
            p_ret->val = call;
            return p_ret;
        }
        else
        {
            throw inner_error(INNER_NO_MATCH_FUN, func_name);
        }
    }
    // == !=
    else if (func_name == "==" || func_name == "!=")
    {
        p_ret->type = syntax_type{.type = primary_type{.name = "bool", .size = 1}};
        if (call.parameters.size() != 2)
            throw inner_error(INNER_NO_MATCH_FUN, func_name);

        auto t1 = call.parameters[0]->type;
        auto t2 = call.parameters[1]->type;
        std::string t1p = call.parameters[0]->type.get_primary();
        std::string t2p = call.parameters[1]->type.get_primary();

        if (call.parameters[0]->type.type_equal(call.parameters[1]->type))
        {
            p_ret->val = call;
            return p_ret;
        }
        else if (t1p != "" && t2p != "")
        {
            bool equal = false;
            if (t1.subtyping(t2))
            {
                call.parameters[0] = expr_convert_to(call.parameters[0], t2, stmts);
            }
            else if (t2.subtyping(t1))
            {
                call.parameters[1] = expr_convert_to(call.parameters[1], t1, stmts);
            }
            else
                throw inner_error(INNER_NO_MATCH_FUN, func_name);
        }
        else
            throw inner_error(INNER_NO_MATCH_FUN, func_name);
    }
    else
    {
        // inline function
        bool find_flag = false;
        p_ret->type = infer_type_in_list(func_name, call, inline_fun, find_flag, stmts);
        if (find_flag)
        {
            p_ret->val = call;
            return p_ret;
        }

        // normal function
        p_ret->type = infer_type_in_list(func_name, call, normal_fun, find_flag, stmts);
        if (find_flag)
        {
            p_ret->val = call;
            return p_ret;
        }
    }

    // not found
    throw inner_error(INNER_NO_MATCH_FUN, func_name);
}

syntax_type function_table::infer_type_in_list(const std::string &func_name, syntax_fun_call &call,
                                               const std::map<std::string, std::vector<syntax_fun>> func_list,
                                               bool &find_flag, std::vector<syntax_stmt> &stmts)
{
    syntax_type ret_type;

    auto it = func_list.find(func_name);
    bool match_flag = false;
    if (it != func_list.end())
    {
        auto &fun = it->second;
        bool match = true;
        for (auto i = 0; i < fun.size(); i++)
        {
            std::vector<int> to_convert_index;
            if (fun[i].parameters.size() == call.parameters.size())
            {
                for (auto j = 0; j < fun[i].parameters.size(); j++)
                {
                    auto t1 = fun[i].parameters[j].second;
                    auto t2 = call.parameters[j]->type;
                    if (t1.type_equal(t2))
                        continue;
                    else if (t2.subtyping(t1))
                        to_convert_index.push_back(j);
                    else
                    {
                        match = false;
                    }
                }
            }
            else
                match = false;

            if (match)
            {
                ret_type = fun[i].ret_type;
                match_flag = true;
                for (auto j = 0; j < to_convert_index.size(); j++)
                {
                    auto index = to_convert_index[j];
                    call.parameters[index] = expr_convert_to(call.parameters[index], fun[i].parameters[index].second,
                                                             stmts);
                }
                break;
            }
            else
                match = true;
        }
    }
    find_flag = match_flag;
    return ret_type;
}

function_table::function_table()
{
    vec_str op = {"+", "-", "*", "/", "%", "&", "|", "^"};
    for (int i = 0; i < op.size(); i++)
    {
        create_bin_op_fun(op[i], "i8", 1, "i8", 1, "i8", 1);
        create_bin_op_fun(op[i], "u8", 1, "u8", 1, "u8", 1);
        create_bin_op_fun(op[i], "i16", 2, "i16", 2, "i16", 2);
        create_bin_op_fun(op[i], "u16", 2, "u16", 2, "u16", 2);
        create_bin_op_fun(op[i], "i32", 4, "i32", 4, "i32", 4);
        create_bin_op_fun(op[i], "u32", 4, "u32", 4, "u32", 4);
        create_bin_op_fun(op[i], "i64", 8, "i64", 8, "i64", 8);
        create_bin_op_fun(op[i], "u64", 8, "u64", 8, "u64", 8);
    }
    op = {">", "<", ">=", "<=","==","!="};
    for (int i = 0; i < op.size(); i++)
    {
        create_bin_op_fun(op[i], "bool", 1, "i8", 1, "i8", 1);
        create_bin_op_fun(op[i], "bool", 1, "u8", 1, "u8", 1);
        create_bin_op_fun(op[i], "bool", 1, "i16", 2, "i16", 2);
        create_bin_op_fun(op[i], "bool", 1, "u16", 2, "u16", 2);
        create_bin_op_fun(op[i], "bool", 1, "i32", 4, "i32", 4);
        create_bin_op_fun(op[i], "bool", 1, "u32", 4, "u32", 4);
        create_bin_op_fun(op[i], "bool", 1, "i64", 8, "i64", 8);
        create_bin_op_fun(op[i], "bool", 1, "u64", 8, "u64", 8);
    }
    op = {"<<", ">>"};
    vec_str param_type = {"i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"};
    for (int i = 0; i < op.size(); i++)
    {
        for (int j = 0; j < param_type.size(); j++)
        {
            for (int k = 0; k < param_type.size(); k++)
            {
                create_bin_op_fun(op[i], param_type[j], (j / 2 + 1) * 4, param_type[j], (j / 2 + 1) * 4, param_type[k],
                                  (k / 2 + 1) * 4);
            }
        }
    }
    create_unary_op_fun("!", "bool", 1, "bool", 1);

    param_type = {"i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"};
    for (auto i = 0; i < param_type.size(); i++)
    {
        create_unary_op_fun("~", param_type[i], (i / 2 + 1) * 4, param_type[i], (i / 2 + 1) * 4);
    }
}

void function_table::create_bin_op_fun(std::string op, std::string ret_type, size_t ret_size, std::string param1_type,
                                       size_t param1_size, std::string param2_type, size_t param2_size)
{
    inline_fun[op].push_back(syntax_fun{.fun_name = op,
                                        .ret_type = syntax_type{.type = primary_type{.name = ret_type, .size = ret_size}},
                                        .parameters = std::vector<std::pair<std::string, syntax_type>>{
                                            std::pair<std::string, syntax_type>("_0",
                                                                                syntax_type{.type = primary_type{.name = param1_type, .size = param1_size}}),
                                            std::pair<std::string, syntax_type>("_1",
                                                                                syntax_type{.type = primary_type{.name = param2_type, .size = param2_size}})}});
}

void function_table::create_unary_op_fun(std::string op, std::string ret_type, size_t ret_size, std::string param_type,
                                         size_t param_size)
{
    inline_fun[op].push_back(syntax_fun{.fun_name = op,
                                        .ret_type = syntax_type{.type = primary_type{.name = ret_type, .size = ret_size}},
                                        .parameters = std::vector<std::pair<std::string, syntax_type>>{
                                            std::pair<std::string, syntax_type>("_0",
                                                                                syntax_type{.type = primary_type{.name = param_type, .size = param_size}})}});
}
