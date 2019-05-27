#include "syntax_tree.h"

void syntax_analysis(node_module module)
{
    // 对于每一个可以定义函数的“环境”

    // 第一步：扫描所有类型定义，生成全局类型表（固定）
    // 需要封装类型表的功能，以支持 built-in 类型
    type_table env_type;
    top_graph dependency_graph;
    for (auto &block : module.blocks)
    {
        if (auto type_def_block = std::get_if<node_global_type_def_block>(&block))
        {
            for (auto type_def_statm : *(type_def_block))
            {
                if (dependency_graph.contain(type_def_statm.type_name))
                    dependency_graph.set_internal_index(type_def_statm.type_name);
                syntax_type syntax_t = env_type.type_check(type_def_statm.type, &dependency_graph);
                if (!dependency_graph.changed)
                {
                    if (dependency_graph.contain(type_def_statm.type_name))
                        dependency_graph.reset_internal_index();
                    env_type.add_type(type_def_statm.type_name, syntax_t);
                }
                else
                {
                    dependency_graph.changed = false;
                    dependency_graph.add_node(type_def_statm.type_name, type_def_statm.type);
                }
            }
        }
    }
    try
    {
        fix_lookahead(env_type, dependency_graph);
    }
    catch (std::string &e)
    {
        std::cout << e << '\n';
    }

    if (debug_flag)
    {
        std::cout << "type_table:\n";
        env_type.print_type_table();
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

            env_fun.add_func(fun);
        }
    }

    // 第三步：扫描全局变量的声明，生成全局变量符号和类型定义，此处需要类型推导。生成初始化函数 __init

    // 第四步：进入每个 block，完成语义分析
}

void fix_lookahead(type_table &env_type, top_graph &dependency_graph)
{
    if (dependency_graph.seq_num == 0)
        return;

    std::vector<int> in_degree(dependency_graph.seq_num);
    std::vector<int> visit(dependency_graph.seq_num);

    for (auto it : dependency_graph.adj_list)
    {
        for (auto edge : it.second)
        {
            in_degree[edge]++;
        }
    }

    for (auto i = 0; i < dependency_graph.seq_num; i++)
    {
        for (auto j = 0; j < dependency_graph.seq_num; j++)
        {
            if (visit[j] == 0 && in_degree[j] == 0)
            {
                if (dependency_graph.arr.count(j) != 0)
                {
                    try
                    {
                        env_type.add_type(dependency_graph.name_table[j], env_type.type_check(dependency_graph.arr[j]));
                        visit[j] = 1;
                        for (auto tmp : dependency_graph.adj_list[j])
                        {
                            in_degree[tmp]--;
                        }
                        break;
                    }
                    catch (std::string &e)
                    {
                        throw e;
                    }
                }
                else
                {
                    try
                    {
                        env_type.get_type(dependency_graph.name_table[j]);
                        visit[j] = 1;
                        for (auto tmp : dependency_graph.adj_list[j])
                        {
                            in_degree[tmp]--;
                        }
                        break;
                    }
                    catch (std::string &e)
                    {
                        throw std::string("no such type " + dependency_graph.name_table[i]);
                    }
                }
            }
            else if (j == dependency_graph.seq_num - 1)
            {
                throw std::string("exists loop type define");
            }
        }
    }
}