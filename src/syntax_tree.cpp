#include "syntax_tree.h"

static void fix_lookahead(type_table &env_type, top_graph &dependency_graph);

void syntax_module::syntax_analysis(node_module module)
{
    // 对于每一个可以定义函数的“环境”

    // 第一步：扫描所有类型定义，生成全局类型表（固定）
    // 需要封装类型表的功能，以支持 built-in 类型
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
        std::cout << "================type_table===============\n";
        env_type.print_type_table();
        std::cout << "===================end===================\n\n";
    }

    // 第二步：扫描所有函数定义，生成全局函数表（固定）
    // 需要封装函数表的功能，以支持 built-in 类型
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

            auto funname = pfun_block->fun_name.val;
            if (funname == "main")
            {
                funname = ".main";
            }
            auto fun = syntax_fun{
                .fun_name = funname,
                .ret_type = pfun_block->no_ret ? env_type.primary_unit : env_type.type_check(pfun_block->ret_type),
                .parameters = params,
                .origin_stmts = pfun_block->statement_list};

            env_fun.add_func(fun);
        }
    }

    // 第三步：扫描全局变量的声明，生成全局变量符号和类型定义，此处需要类型推导。生成入口函数 main
    syntax_fun main_fun;
    main_fun.fun_name = "main";
    main_fun.ret_type = env_type.get_type("i32");

    env_var.push();
    for (auto &block : module.blocks)
    {
        if (auto p_global_var_def = std::get_if<node_global_var_def_block>(&block))
        {
            for (auto &def : *p_global_var_def)
            {
                auto init_expr = expr_analysis(def.initial_exp, main_fun.stmts);
                std::shared_ptr<syntax_expr> rval;

                syntax_type t = env_type.type_check(def.var_type);
                if (t.is_auto())
                {
                    // 类型推导
                    t = init_expr->type;
                    rval = init_expr;
                }
                else
                {
                    // 隐式转换
                    auto impl_convert = syntax_type_convert{.source_expr = init_expr, .target_type = t};
                    rval->type = t;
                    rval->val = impl_convert;
                }

                // 分配全局变量
                for (auto &v : def.var_list)
                {
                    auto var = std::make_shared<syntax_expr>();
                    var->val = syntax_var{.alloc_type = syntax_var::STATIC};

                    // 声明
                    env_var.insert(v.val, var);

                    // 初始化
                    syntax_assign assign{.lval = var, .rval = rval};
                    main_fun.stmts.push_back(syntax_stmt{.stmt = assign});
                }
            }
        }
    }

    env_fun.add_func(main_fun);

    // 第四步：进入每个 block，完成语义分析
    // - 变量的分析（参考全局变量部分）
    // - 控制流 if, while 的分析
}

static void fix_lookahead(type_table &env_type, top_graph &dependency_graph)
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

// todo: 补全该函数
std::shared_ptr<syntax_expr> syntax_module::expr_analysis(const node_expression &node, std::vector<syntax_stmt> &stmts)
{
    return nullptr;
}