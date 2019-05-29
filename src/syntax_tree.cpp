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
                // std::vector<syntax_stmt> s;
                auto init_expr = expr_analysis(def.initial_exp);
                // std::cout << s.size() << '\n';
                // for(auto it : s)
                // {
                //     auto tmp = std::get_if<std::shared_ptr<syntax_expr>>(&it.stmt);
                //     if(auto t_1 = std::get_if<syntax_literal>(&tmp->get()->val))
                //     {
                //         if(auto t_2 = std::get_if<unsigned long long>(&t_1->val))
                //         {
                //             std::cout << *t_2 << '\n';
                //         }
                //     }
                //     else if(auto t_3 = std::get_if<syntax_fun_call>(&tmp->get()->val))
                //     {
                //         std::cout << t_3->fun_name << '\n';
                //     }
                // }


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
    static std::map<std::string, int> precedence = {
        {"+", 0}, {"-", 0}, {"*", 1}, {"/", 1}
    };
    if(auto p = std::get_if<node_binary_expr>(&node.expr)) 
    {
        std::cout << "bin" << '\n';
        auto syntax_exp = unary_expr_analysis(p->vars[0], stmts);
        // auto syntax_exp = std::make_shared<syntax_expr>();
        // auto oprand1 = std::get_if<node_primary_expr>(&(p->vars[0].post_expr)->expr);
        // auto constant = std::get_if<node_constant>(oprand1);
        // syntax_literal t;
        // t.type = syntax_type{.type = primary_type{.name = std::get_if<node_identifier>(&constant->type->type_val)->val}};
        // t.val = constant->val;
        // syntax_exp->type = t.type;
        // syntax_exp->val = t;
        // stmts.push_back(syntax_stmt{.stmt = syntax_exp});
        for(auto i = 1; i < p->vars.size(); i++)
        {
            auto syntax_tmp = unary_expr_analysis(p->vars[i], stmts);
            // auto syntax_tmp = std::make_shared<syntax_expr>();
            // auto oprand_tmp = std::get_if<node_primary_expr>(&(p->vars[i].post_expr)->expr);
            // auto constant_tmp = std::get_if<node_constant>(oprand_tmp);
            // syntax_literal tmp;
            // tmp.type = syntax_type{.type = primary_type{.name = std::get_if<node_identifier>(&constant_tmp->type->type_val)->val}};
            // tmp.val = constant_tmp->val;
            // syntax_tmp->type = tmp.type;
            // syntax_tmp->val = tmp;
            // stmts.push_back(syntax_stmt{.stmt = syntax_tmp});
            syntax_fun_call call_tmp;
            call_tmp.fun_name = p->ops[i-1];
            call_tmp.parameters.push_back(syntax_exp);
            call_tmp.parameters.push_back(syntax_tmp);
            syntax_exp = env_fun.infer_type(p->ops[i-1], call_tmp);
            stmts.push_back(syntax_stmt{.stmt = syntax_exp});
        }
        return syntax_exp;
    }
    else if(auto p = std::get_if<node_assign_expr>(&node.expr))
    {
        // std::cout << "assign" << '\n';
        return nullptr;
    }
    else if(auto p = std::get_if<node_construct_expr>(&node.expr))
    {
        // std::cout << "construct" << '\n';
        return nullptr;
    }
    else
        assert(false);

}

std::shared_ptr<syntax_expr> syntax_module::binary_expr_analysis(const node_binary_expr &node, std::vector<syntax_stmt> &stmts)
{
    return nullptr;
}

std::shared_ptr<syntax_expr> syntax_module::unary_expr_analysis(const node_unary_expr &node, std::vector<syntax_stmt> &stmts)
{
    if(node.ops.empty())
    {
        return post_expr_analysis(*node.post_expr.get(), stmts);
    }
    else
    {
        auto p = post_expr_analysis(*node.post_expr.get(), stmts);
        auto call = syntax_fun_call{.fun_name = node.ops[0], .parameters={p}};
        auto syntax_unary = env_fun.infer_type(node.ops[0], call);
        stmts.push_back(syntax_stmt{.stmt = syntax_unary});
        return syntax_unary;
    }
}

std::shared_ptr<syntax_expr> syntax_module::post_expr_analysis(const node_post_expr &node, std::vector<syntax_stmt> &stmts)
{
    if(auto p = std::get_if<node_primary_expr>(&node.expr))
    {
        if(auto q = std::get_if<node_identifier>(p))
        {
            return nullptr;
        }
        else if(auto q = std::get_if<node_constant>(p))
        {
            auto type_name = std::get_if<node_identifier>(&(q->type->type_val));
            auto type = syntax_type{.type = primary_type{.name = type_name->val}};
            auto literal = syntax_literal{.type = type, .val = q->val};
            auto syntax_node = std::make_shared<syntax_expr>();
            syntax_node->type = type;
            syntax_node->val = literal;
            stmts.push_back(syntax_stmt{.stmt = syntax_node});
            return syntax_node;
        }
        else if(auto q = std::get_if<std::shared_ptr<node_expression>>(p))
        {
            return expr_analysis(*(q->get()), stmts);
        }
        else 
            assert(false);
    }
    else if(auto p = std::get_if<node_post_call_expr>(&node.expr))
    {
        return nullptr;
    }
    else if(auto p = std::get_if<node_post_dot_expr>(&node.expr))
    {
        return nullptr;
    }
    else if(auto p = std::get_if<node_post_check_expr>(&node.expr))
    {
        return nullptr;
    }
    else 
        assert(false);
}