#include "function.h"
#include "syntax_tree.h"
#include <map>
#include <set>

using namespace std;

std::shared_ptr<syntax_expr> function_table::infer_type(const std::string &func_name, syntax_fun_call &call)
{
    auto p_ret = make_shared<syntax_expr>();
    p_ret->val = call;
    std::string t1 = call.parameters[0]->type.get_primary();
    std::string t2 = call.parameters[1]->type.get_primary();
    // .
    if(func_name[0] == '.' && func_name[func_name.size()-1] != '?')
    {
        bool match = false;
        std::string field_name = func_name.substr(1,func_name.size()-1);
        if(call.parameters.size() != 1)
            throw("parameters not match");
        auto ptr_product_val = call.parameters[0];
        auto ptr_product_type = std::get_if<product_type>(&ptr_product_val->type.type);
        if(!ptr_product_type)
            throw("parameters not match");
        auto it_fields = ptr_product_type->fields.begin();
        auto it_type = ptr_product_type->types.begin();
        for(;it_fields!=ptr_product_type->fields.end();it_fields++,it_type++)
        {
            if(field_name == *it_fields)
                {
                    match = true;
                    p_ret->type = **it_type;
                    break;
                }
        }
        if(match)
            return p_ret;
        else
            throw("parameters not match");
    }
    //.?
    else if(func_name[0] == '.' && func_name[func_name.size()-1] == '?')
    {
        bool match = false;
        std::string alt_name = func_name.substr(1,func_name.size()-2);
        if(call.parameters.size() != 1)
            throw("parameters not match");
        auto ptr_sum_val = call.parameters[0];
        auto ptr_sum_type = std::get_if<sum_type>(&ptr_sum_val->type.type);
        if(!ptr_sum_type)
            throw("parameters not match");
        auto it_alt = ptr_sum_type->alters.begin();
        auto it_type = ptr_sum_type->types.begin();
        for(;it_alt!=ptr_sum_type->alters.end();it_alt++,it_type++)
        {
            if(alt_name == *it_alt)
                {
                    match = true;
                    p_ret->type = **it_type;
                    break;
                }
        }
        if(match)
            return p_ret;
        else
            throw("parameters not match");
    }
    // == !=
    else if(func_name == "==" || func_name == "!=")
    {
        p_ret->type =  syntax_type{.type = primary_type{.name = "bool",.size = 1}};
        if(call.parameters.size() != 2)
            throw("parameters not match");
        if(call.parameters[0]->type.type_equal(call.parameters[1]->type))
        {
            return p_ret;
        }
        else if(t1 != "" && t2 != "")
        {
            implicit_conv(call.parameters[0],call.parameters[1]);
        }
        else 
            throw("parameters not match");
    }
    // inline function
    try
    {
        p_ret->type = infer_type_in_list(func_name, call, inline_fun);
        return p_ret;
    }
    catch (std::string exception)
    {
        if (exception == "parameters not match")
            throw(exception);
    }
    // normal function
    p_ret->type = infer_type_in_list(func_name, call, normal_fun);
    return p_ret;
}

syntax_type function_table::infer_type_in_list(const std::string &func_name,syntax_fun_call &call, const std::map<std::string, std::vector<syntax_fun>> func_list)
{
    syntax_type ret_type;
    auto it = func_list.find(func_name);
    if (it == func_list.end())
    {
        throw string("no such function " + func_name);
    }
    auto &fun = it->second;
    bool match = true;
    bool match_flag = false;
    for (auto i = 0; i < fun.size(); i++)
    {
        if (fun[i].parameters.size() == call.parameters.size())
        { 
            for (auto j = 0; j < fun[i].parameters.size(); j++)
            {
                auto t1 = fun[i].parameters[j].second;
                auto t2 = call.parameters[j]->type;
                auto t1p = fun[i].parameters[j].second.get_primary();
                auto t2p = call.parameters[j]->type.get_primary();
                if (t1p == "" || t2p == "")
                {
                    if (!t2.subtyping(t1))
                    {
                        match = false;
                        break;
                    }
                }
                else if (t1p != "" && t2p != "")
                {
                    if(t1p != t2p)
                        implicit_conv(call.parameters[j],fun[i].parameters[j].second);
                }
                else
                {
                    match = false;
                    break;
                }
            }
        }
        if (match)
        {
            ret_type = fun[i].ret_type;
            match_flag =true;
            break;
        }
        else
        {
            match = true;
        }
    }
    if(match_flag)
        return ret_type;
    else
        throw("parameters not match");
}

function_table::function_table()
{
    vec_str op = {"+", "-", "*", "/", "%", "&", "|", "^",">","<",">=","<="};
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
    op = {"<<", ">>", ">>=", "<<="};
    vec_str param_type = {"i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"};
    for (int i = 0; i < op.size(); i++)
    {
        for (int j = 0; j < param_type.size(); j++)
        {
            for (int k = 0; k < param_type.size(); k++)
            {
                create_bin_op_fun(op[i], param_type[j], (j / 2 + 1) * 4, param_type[j], (j / 2 + 1) * 4, param_type[k], (k / 2 + 1) * 4);
            }
        }
    }
}

void function_table::create_bin_op_fun(std::string op, std::string ret_type, size_t ret_size, std::string param1_type, size_t param1_size, std::string param2_type, size_t param2_size)
{
    inline_fun[op].push_back(syntax_fun{.fun_name = op,
                                        .ret_type = syntax_type{.type = primary_type{.name = ret_type, .size = ret_size}},
                                        .parameters = std::vector<std::pair<std::string, syntax_type>>{
                                            std::pair<std::string, syntax_type>("_0", syntax_type{.type = primary_type{.name = param1_type, .size = param1_size}}),
                                            std::pair<std::string, syntax_type>("_1", syntax_type{.type = primary_type{.name = param2_type, .size = param2_size}})}});
}

bool function_table::primary_match(std::string t1, std::string t2)
{
    vector<string> type = {"u8", "i8", "u16", "i16", "u32", "i32", "u64", "i64"};
    int i1 = -1;
    int i2 = -1;
    for (auto i = 0; i < type.size(); i++)
    {
        if (t1 == type[i])
            i1 = i;
        if (t2 == type[i])
            i2 = i;
    }
    if (i1 == -1 || i2 == -1)
        return t1 == t2;
    return (i1 == i2 || i1 - i2 >= 2) && (((i1 - i2) % 2 == 0) || ((i1 - i2) % 2 == 1 && i2 % 2 == 0));
}

void function_table::implicit_conv(std::shared_ptr<syntax_expr> param1,std::shared_ptr<syntax_expr> param2)
{
    std::string t1 = param1->type.get_primary();
    std::string t2 = param2->type.get_primary();
    if(primary_match(t1,t2))
    {
        auto conv = std::make_shared<syntax_expr>();
        conv->val = syntax_type_convert{.source_expr = param2,
                                        .target_type = param1->type};
        conv->type = param1->type;
        param2 = conv;
    }
    else if(primary_match(t2,t1))
    {
        auto conv = std::make_shared<syntax_expr>();
        conv->val = syntax_type_convert{.source_expr = param1,
                                        .target_type = param2->type};
        conv->type = param2->type;
        param1 = conv;
    }
    else
        throw("parameters not match");
}

void function_table::implicit_conv(std::shared_ptr<syntax_expr> param,const syntax_type fun_param_type)
{
    std::string t1 = param->type.get_primary();
    std::string t2 = fun_param_type.get_primary();
    if(primary_match(t2,t1))
    {
        auto conv = std::make_shared<syntax_expr>();
        conv->val = syntax_type_convert{.source_expr = param,
                                        .target_type = fun_param_type};
        conv->type = fun_param_type;
        param = conv;
    }
    else
        throw("parameters not match");
}