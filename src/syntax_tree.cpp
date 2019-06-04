#include "syntax_tree.h"
#include "exception.h"

std::shared_ptr<syntax_if_block> syntax_merged_if_block::reduce(int index)
{
    syntax_if_block ret;
    ret.condition = condition[index];
    ret.cond_stmt = std::move(condition_stmt[index]);
    ret.then_stmt = std::move(branch[index]);

    // 最后一个 if
    if (index == condition.size() - 1)
    {
        ret.else_stmt = std::move(default_branch);
    }
    else
    {
        ret.else_stmt.push_back(syntax_stmt{reduce(index + 1)});
    }

    return std::make_shared<syntax_if_block>(ret);
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
                    env_type.add_type(dependency_graph.name_table[j], env_type.type_check(dependency_graph.arr[j]));
                    visit[j] = 1;
                    for (auto tmp : dependency_graph.adj_list[j])
                    {
                        in_degree[tmp]--;
                    }
                    break;
                }
                else
                {
                    env_type.get_type(dependency_graph.name_table[j]);
                    visit[j] = 1;
                    for (auto tmp : dependency_graph.adj_list[j])
                    {
                        in_degree[tmp]--;
                    }
                    break;
                }
            }
            else if (j == dependency_graph.seq_num - 1)
            {
                throw inner_error{INNER_LOOP};
            }
        }
    }
}

void syntax_module::add_var(const node_var_def_statement &def, std::vector<syntax_stmt> &stmts, bool is_global)
{
    auto init_expr = expr_analysis(def.initial_exp, stmts);
    auto rval = std::make_shared<syntax_expr>();
    syntax_type t = env_type.type_check(def.var_type);
    if (t.is_auto())
    {
        // 类型推导
        t = init_expr->type; // todo: t 应该被修正，u15 -> i16 这种
        rval = init_expr;
    }
    else
    {
        // 隐式转换
        try
        {
            rval = expr_convert_to(init_expr, t, stmts);
        }
        catch (inner_error &e)
        {
            throw syntax_error{def.loc, "can't do such type conversion"};
        }
    }

    // 分配变量
    for (auto &v : def.var_list)
    {
        auto var = std::make_shared<syntax_expr>();
        var->val = syntax_var();
        var->type = t;
        var->immutable = def.is_immutable;

        // 声明
        env_var.insert(v.val, var);
        if (!is_global)
            stmts.push_back(syntax_stmt{var});

        // 初始化
        syntax_assign assign{.lval = var, .rval = rval};
        stmts.push_back(syntax_stmt{.stmt = assign});
    }
}

void syntax_module::typedef_analysis(const node_module &module)
{
    top_graph dependency_graph;
    for (auto &block : module.blocks)
    {
        if (auto type_def_block = std::get_if<node_global_type_def_block>(&block.val))
        {
            for (auto type_def_statm : type_def_block->arr)
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
    catch (inner_error &e)
    {
        if (e.number == INNER_NO_SUCH_TYPE)
        {
            throw syntax_error(module.loc, "no such type '" + e.info + "'");
        }
        else if (e.number == INNER_LOOP)
        {
            throw syntax_error(module.loc, "exists loop in type defination");
        }
    }
}

std::vector<syntax_fun> syntax_module::fundef_analysis(const node_module &module)
{
    std::vector<syntax_fun> fun_lst;

    // 需要封装函数表的功能，以支持 built-in 类型
    for (auto &block : module.blocks)
    {
        if (auto pfun_block = std::get_if<node_function_block>(&block.val))
        {
            std::vector<std::pair<std::string, syntax_type>> params;
            if (!pfun_block->params.empty_flag)
            {
                auto type = env_type.type_check(node_type{block.loc, false, pfun_block->params.params});
                if (auto p_prod = std::get_if<product_type>(&type.type))
                {
                    for (auto i = 0; i < p_prod->fields.size(); i++)
                    {
                        params.emplace_back(std::make_pair(p_prod->fields[i], *(p_prod->types[i])));
                    }
                }
                else
                {
                    throw syntax_error(pfun_block->loc, "invliad argument list in function '" + pfun_block->fun_name.val + "'");
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
    std::vector<syntax_stmt> stmts{};
    syntax_fun main_fun;
    main_fun.fun_name = "main";
    main_fun.ret_type = env_type.get_type("i32");
    env_var.push();
    for (auto &block : module.blocks)
    {
        if (auto p_global_var_def = std::get_if<node_global_var_def_block>(&block.val))
        {
            for (auto &def : p_global_var_def->arr)
            {
                add_var(def, stmts, true);
            }
        }
    }

    syntax_fun user_main_fun;
    try
    {
        user_main_fun = env_fun.get_user_fun(".main");
        if (!user_main_fun.parameters.empty() || user_main_fun.ret_type.get_primary() != "i32")
        {
            throw syntax_error(module.loc, "function 'main' must be 'fn main() i32 {}'");
        }
    }
    catch (std::exception &)
    {
        throw syntax_error(module.loc, "function 'main' must be 'fn main() i32 {}'");
    }

    // 调用 main()
    auto call_main = std::make_shared<syntax_expr>();
    auto call_struct = syntax_fun_call();
    call_struct.fun_name = ".main";
    call_main->type = user_main_fun.ret_type;
    call_main->val = call_struct;

    stmts.push_back(syntax_stmt{call_main});

    // return main()
    auto i32 = env_type.get_type("i32");
    stmts.push_back(syntax_stmt{syntax_return{call_main}});

    fun_name.push_back("main");
    fun_impl.push_back(std::move(stmts));
    fun_args.push_back(std::vector<std::shared_ptr<syntax_expr>>());

    env_fun.add_func(main_fun);
}

syntax_stmt syntax_module::if_analysis(const node_if_statement &node)
{
    syntax_merged_if_block block;
    env_var.push();
    block.condition_stmt.push_back(std::vector<syntax_stmt>());
    block.condition.push_back(expr_analysis(node.if_condition, block.condition_stmt[0]));
    block.branch.push_back(statement_analysis(node.if_statement));
    block.default_branch = statement_analysis(node.else_statement);
    auto it_condition = node.else_if_statement.else_if_condition.begin();
    auto it_statement = node.else_if_statement.else_if_statement.begin();
    for (int i = 1; it_condition != node.else_if_statement.else_if_condition.end(); it_condition++, it_statement++, i++)
    {
        block.condition_stmt.push_back(std::vector<syntax_stmt>());
        block.condition.push_back(expr_analysis(*it_condition, block.condition_stmt[i]));
        block.branch.push_back(statement_analysis(*it_statement));
    }
    env_var.pop();

    return {block.reduce(0)};
}

syntax_stmt syntax_module::while_analysis(const node_while_statement &node)
{
    syntax_while_block block;
    env_var.push();
    block.condition = expr_analysis(node.while_condition, block.condition_stmt);
    block.body = statement_analysis(node.loop_statement);
    env_var.pop();
    return {std::make_shared<syntax_while_block>(block)};
}

syntax_stmt syntax_module::for_analysis(const node_for_statement &node)
{
    syntax_for_block block;
    env_var.push();
    block.begin_test = expr_analysis(node.begin_test, block.begin_test_stmt);
    expr_analysis(node.init_expr, block.init_stmt);
    expr_analysis(node.end_process, block.end_process_stmt);
    block.body = statement_analysis(node.for_statement);
    env_var.pop();
    return {std::make_shared<syntax_for_block>(block)};
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

void syntax_module::function_analysis(const syntax_fun &node)
{
    ret_type = node.ret_type;

    env_var.push();
    std::vector<std::shared_ptr<syntax_expr>> args;
    std::vector<syntax_stmt> stmts;

    // 参数加入局部变量
    for (auto &arg : node.parameters)
    {
        auto exp = std::make_shared<syntax_expr>();
        exp->type = arg.second;
        exp->val = syntax_var();
        args.push_back(exp);
        env_var.insert(arg.first, exp);
    }

    stmts = statement_analysis(node.origin_stmts);

    // 分析结束，保存函数实现
    fun_name.push_back(node.fun_name);
    fun_impl.emplace_back(std::move(stmts));
    fun_args.push_back(std::move(args));

    env_var.pop();
}

std::vector<syntax_stmt> syntax_module::statement_analysis(std::vector<node_statement> origin_stmts)
{
    std::vector<syntax_stmt> stmts;
    for (auto &node_stmt : origin_stmts)
    {
        auto ps = &node_stmt.statement;
        if (auto def = std::get_if<node_var_def_statement>(ps))
        {
            add_var(*def, stmts);
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
            stmts.emplace_back(for_analysis(*forb));
        }
        else if (auto expr = std::get_if<node_expression>(ps))
        {
            expr_analysis(*expr, stmts);
        }
        else if (auto ret = std::get_if<node_return_statement>(ps))
        {
            auto ret_e = expr_analysis(ret->expr, stmts);
            if (ret_e->type.subtyping(ret_type))
            {
                auto ret_v = expr_convert_to(ret_e, ret_type, stmts);
                stmts.emplace_back(syntax_stmt{syntax_return{ret_v}});
            }
            else
            {
                throw syntax_error(node_stmt.loc, "can't return value of this type");
            }
        }
        else if (auto d = std::get_if<node_delete_statement>(ps))
        {
            auto delete_e = expr_analysis(d->delete_expr, stmts);
            if (delete_e->type.is_ref())
            {
                auto expr = std::make_shared<syntax_expr>();
                expr->val = syntax_fun_call{
                    .fun_name = "delete",
                    .parameters = std::vector<std::shared_ptr<syntax_expr>>{delete_e}};
                expr->type = env_type.get_type("unit");
                stmts.emplace_back(syntax_stmt{expr});
            }
            else
            {
                throw syntax_error(node_stmt.loc, "can't delete this expression");
            }
        }
    }
    return stmts;
}
