#include "syntax_tree.h"

static void fix_lookahead(type_table &env_type, top_graph &dependency_graph);

void syntax_module::typedef_analysis(const node_module &module)
{
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
    fix_lookahead(env_type, dependency_graph);

    if (debug_flag)
    {
        std::cout << "================type_table===============\n";
        env_type.print_type_table();
        std::cout << "===================end===================\n\n";
    }
}

std::vector<syntax_fun> syntax_module::fundef_analysis(const node_module &module)
{
    std::vector<syntax_fun> fun_lst;

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

            fun_lst.push_back(fun);
            env_fun.add_func(fun);
        }
    }
    return fun_lst;
}

void syntax_module::global_var_analysis(const node_module &module)
{
    std::vector<syntax_stmt> stmts;
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
                auto init_expr = expr_analysis(def.initial_exp, stmts);
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
                    stmts.push_back(syntax_stmt{.stmt = assign});
                }
            }
        }
    }
    // todo: fun 调用 .main
    fun_impl.emplace_back(std::make_pair("main", std::move(stmts)));
    env_fun.add_func(main_fun);
}

// todo:
syntax_stmt syntax_module::if_analysis(const node_if_statement &node)
{
    syntax_if_block block;
    env_var.push();
    block.condition.push_back(expr_analysis(node.if_condition));
    block.branch.push_back(statement_analysis(node.if_statement));
    block.defaul_branch.push_back(statement_analysis(node.else_statement));
    auto it_condition = node.else_if_statement.else_if_condition.begin();
    auto it_statement = node.else_if_statement.else_if_statement.begin();
    for(;it_condition!=node.else_if_statement.else_if_condition.end();it_condition++,it_statement++)
    {
        block.condition.push_back(expr_analysis(*it_condition));
        block.branch.push_back(statement_analysis(&it_statement));
    }
    env_var.pop();
    return {block};
}

syntax_stmt syntax_module::while_analysis(const node_while_statement &node)
{
    syntax_while_block block;
    env_var.push();
    block.while_condition = expr_analysis(node.while_condition);
    //add while condition to the statement
    node.loop_statement.push_back(node_statement{.statement = node.while_condition});
    //-------------------------------------
    block.body = statement_analysis(node.loop_statement);
    env_var.pop();
    return {block};
}

void syntax_module::syntax_analysis(const node_module &module)
{
    // 对于每一个可以定义函数的“环境”

    // 第一步：扫描所有类型定义，生成全局类型表（固定）
    typedef_analysis(module);

    // 第二步：扫描所有函数定义，生成全局函数表（固定）
    auto fun_lst = fundef_analysis(module);

    // 第三步：扫描全局变量的声明，生成全局变量符号和类型定义，此处需要类型推导。生成入口函数 main
    global_var_analysis(module);

    // 第四步：进入每个 block，完成语义分析
    // - 变量的分析（参考全局变量部分）
    // - 控制流 if, while 的分析
    for (auto &fun : fun_lst)
    {
        function_analysis(fun);
    }
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

void syntax_module::function_analysis(const syntax_fun &node)
{
    env_var.push();
    std::vector<syntax_stmt> stmts;

    // 参数加入局部变量
    for (auto &arg : node.parameters)
    {
        auto exp = std::make_shared<syntax_expr>();
        exp->type = arg.second;
        exp->val = syntax_var{syntax_var::PARAMETER};
        env_var.insert(arg.first, exp);
    }

    stmts = statement_analysis(origin_stmts);

    // 分析结束，保存函数实现
    fun_impl.emplace_back(std::make_pair(node.fun_name, std::move(stmts)));
    env_var.pop();
}
std::vector<syntax_stmt> syntax_module::statement_analysis(std::vector<node_statement> origin_stmts)
{
    std::vector<syntax_stmt> stmts;
    for (auto &node_stmt : node.origin_stmts)
    {
        auto ps = &node_stmt.statement;
        if (auto def = std::get_if<node_var_def_statement>(ps))
        {
            auto init_expr = expr_analysis(def->initial_exp, stmts);
            std::shared_ptr<syntax_expr> rval;

            syntax_type t = env_type.type_check(def->var_type);
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

            // 分配局部变量
            for (auto &v : def->var_list)
            {
                auto var = std::make_shared<syntax_expr>();
                var->val = syntax_var{.alloc_type = syntax_var::STACK};

                // 声明
                env_var.insert(v.val, var);

                // 初始化
                syntax_assign assign{.lval = var, .rval = rval};
                stmts.push_back(syntax_stmt{.stmt = assign});
            }
        }
        else if (auto ifb = std::get_if<node_if_statement>(ps))
        {
            stmts.emplace_back(if_analysis(*ifb));
        }
        else if (auto whileb = std::get_if<node_while_statement>(ps))
        {
            stmts.emplace_back(while_analysis(*whileb));
        }
        else if (auto forb = std::get_if<node_for_statement>(ps))
        {
            // todo:
            throw std::string("unsupported for statement");
        }
        else if (auto expr = std::get_if<node_expression>(ps))
        {
            auto e = expr_analysis(*expr, stmts);
            stmts.emplace_back(syntax_stmt{e});
        }
        else if (auto ret = std::get_if<node_return_statement>(ps))
        {
            auto ret_e = expr_analysis(ret->expr, stmts);
            stmts.emplace_back(syntax_stmt{syntax_return{ret_e}});
        }
    }
    return stmts;
}