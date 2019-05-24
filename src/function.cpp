#include "function.h"
#include "syntax_tree.h"
#include <map>
#include <set>

using namespace std;

std::shared_ptr<syntax_expr> function_table::infer_type(const std::string &func_name, const syntax_fun_call &call)
{
    auto p_ret = make_shared<syntax_expr>();
    auto p_call = make_shared<syntax_fun_call>();
    p_ret->val = p_call;
    *p_call = call;
    // inline function
    try{
        p_ret->type = infer_type_in_list(func_name, call, normal_inline);
        return p_ret;
    }
    catch(std::string exception){
        if(exception == "ambigious function definition")
            throw(exception);
    }
    // normal function
    p_ret->type = infer_type_in_list(func_name, call, normal_fun);
    return p_ret;
}

syntax_type function_table::infer_type_in_list(const std::string &func_name, const syntax_fun_call &call,const std::map<std::string, std::vector<syntax_fun>> func_list)
{
    syntax_type ret_type;
    auto it = func_list.find(func_name);
    if (it == func_list.end())
    {
        throw new string("no such function " + func_name);
    }

    bool already_match = false;

    for (auto &fun : it->second)
    {
        bool match = true;
        if (fun.parameters.size() == call.parameters.size())
        {
            for (auto i = 0; i < fun.parameters.size(); i++)
            {
                // todo: subtyping
                auto t1 = fun.parameters[i].second.get_primary();
                auto t2 = call.parameters[i]->type.get_primary();
                if (t1 == "" || t1 != t2)
                {
                    match = false;
                    break;
                }
            }
        }
        else
        {
            match = false;
        }

        if (match)
        {
            if (already_match)
            {
                throw new string("ambigious function definition");
            }
            already_match = true;
            ret_type = fun.ret_type;
        }
    }
    return ret_type;
}
function_table::function_table()
{
    vec_str op = {"+","-","*","/","%","&","|","^","+=","-=","*=","/=","%=","&=","|=","^="};
    for(int i =0;i<op.size();i++)
    {
        create_bin_op_fun(op[i],"i8",1,"i8",1,"i8",1);
        create_bin_op_fun(op[i],"u8",1,"i8",1,"u8",1);
        create_bin_op_fun(op[i],"u8",1,"u8",1,"i8",1);
        create_bin_op_fun(op[i],"u8",1,"u8",1,"u8",1);
        create_bin_op_fun(op[i],"i16",2,"i16",2,"i16",2);
        create_bin_op_fun(op[i],"u16",2,"i16",2,"u16",2);
        create_bin_op_fun(op[i],"u16",2,"u16",2,"i16",2);
        create_bin_op_fun(op[i],"u16",2,"u16",2,"u16",2);
        create_bin_op_fun(op[i],"i32",4,"i32",4,"i32",4);
        create_bin_op_fun(op[i],"u32",4,"i32",4,"u32",4);
        create_bin_op_fun(op[i],"u32",4,"u32",4,"i32",4);
        create_bin_op_fun(op[i],"u32",4,"u32",4,"u32",4);
        create_bin_op_fun(op[i],"i64",8,"i64",8,"i64",8);
        create_bin_op_fun(op[i],"u64",8,"i64",8,"u64",8);
        create_bin_op_fun(op[i],"u64",8,"u64",8,"i64",8);
        create_bin_op_fun(op[i],"u64",8,"u64",8,"u64",8);
    }
    op = {"<<",">>",">>=","<<="};
    vec_str param_type = {"i8","u8","i16","u16","i32","u32","i64","u64"}; 
    for(int i =0;i<op.size();i++)
    {
        for(int j=0;j<param_type.size();j++)
        {
            for(int k=0;k<param_type.size();k++)
            {
                create_bin_op_fun(op[i],param_type[j],(j/2+1)*4,param_type[j],(k/2+1)*4,param_type[j],(k/2+1)*4);
            }
        }
    }
}
void create_bin_op_fun(std::string op,std::string ret_type ,size_t ret_size,std::string param1_type ,size_t param1_size,std::string param2_type ,size_t param2_size)
{
    inline_fun.push_back(syntax_fun{.name = op,
                        .ret_type = syntax_type{.type =primary_type{.name = ret_type,.size = ret_size}},
                        .parameters = std::vector<std::pair<std::string, syntax_type>>{
                            std::pair<std::string, syntax_type>("_0",syntax_type{.type =primary_type{.name = param1_type,.size = param1_size}}),
                            std::pair<std::string, syntax_type>("_1",syntax_type{.type =primary_type{.name = param2_type,.size = param2_size}})}
                        });    
}