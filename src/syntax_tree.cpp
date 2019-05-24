#include "syntax_tree.h"

void syntax_analysis(node_module module)
{
    // 对于每一个可以定义函数的“环境”

    // 第一步：扫描所有类型定义，生成全局类型表（固定）
    // 需要封装类型表的功能，以支持 built-in 类型
    type_table env_type;

    for(auto &block : module.blocks) {
        if(auto type_def_block = std::get_if<node_global_type_def_block>(&block)) {
            for(auto type_def_statm : *(type_def_block)) {
                env_type.add_type(type_def_statm.type_name, env_type.type_check(type_def_statm.type));
            }
        }
    }

    // 第二步：扫描所有函数定义，生成全局函数表（固定）
    // 需要封装函数表的功能，以支持 built-in 类型
    function_table env_fun;

    for (auto &block : module.blocks)
    {
        if (auto pfun_block = std::get_if<node_function_block>(&block))
        {
            std::vector<std::pair<std::string, syntax_type>> params;
            if (!pfun_block->params.empty_flag)
            {
                auto type = env_type.type_check(node_type{false, pfun_block->params.params});
                if (auto p_prod = std::get_if<product_type>(&type.type))
                {
                    for (auto i = 0; i < p_prod->fields.size(); i++)
                    {
                        params.emplace_back(std::make_pair(p_prod->fields[i], *(p_prod->types[i])));
                    }
                }
                else
                {
                    throw new std::string("invliad argument list in fun " + pfun_block->fun_name.val);
                }
            }

            auto fun = syntax_fun{
                .fun_name = pfun_block->fun_name.val,
                .ret_type = pfun_block->no_ret ? env_type.primary_unit : env_type.type_check(pfun_block->ret_type),
                .parameters = params,
                .origin_stmts = pfun_block->statement_list};

            env_fun.add_func(pfun_block->fun_name.val, fun);
        }
    }

    // 第三步：扫描全局变量的声明，生成全局变量符号和类型定义，此处需要类型推导。生成初始化函数 __init
    
    // 第四步：进入每个 block，完成语义分析
}