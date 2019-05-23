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

    // built-in function

    // u8,i8,u16,i16,u32,i32,u64,i64,f32,f64
    if (func_name == "+" || func_name == "-" || func_name == "*" || func_name == "/")
    {
        static const string types[] = {"u8", "i8", "u16", "i16", "u32", "i32", "u64", "i64"};
        assert(call.parameters.size() == 2);
        auto t1 = call.parameters[0]->type.get_primary();
        auto t2 = call.parameters[1]->type.get_primary();
        assert(t1 == t2); // todo: 隐式转换

        bool match = false;
        for (auto &t : types)
        {
            if (t1 == t)
            {
                match = true;
                break;
            }
        }

        if (match)
        {
            p_ret->type = call.parameters[0]->type;
            return p_ret;
        }
        else
        {
            // todo
            throw new string("type mismatch");
        }
    }

    // user_def function
    auto it = user_def_fun.find(func_name);
    if (it == user_def_fun.end())
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
            p_ret->type = fun.ret_type;
        }
    }
    return p_ret;
}