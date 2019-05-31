#include "syntax_tree.h"
#include "exception.h"

static void fix_lookahead(type_table &env_type, top_graph &dependency_graph);

void syntax_module::typedef_analysis(const node_module &module)
{
    // 需要封装类型表的功能，以支持 built-in 类型
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
    fix_lookahead(env_type, dependency_graph);

    if (verbose_flag)
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
    std::vector<syntax_stmt> stmts;
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
                auto init_expr = expr_analysis(def.initial_exp, stmts);
                // test remove!!
                // std::cout << stmts.size() << '\n';
                // for(auto it : stmts)
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
<<<<<<< HEAD
                std::shared_ptr<syntax_expr> rval = std::make_shared<syntax_expr>();
=======
                auto rval = std::make_shared<syntax_expr>();
>>>>>>> aea411cbb852345509770b565a1e6c73dd08cf80
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
                    rval->reserved = (void *)1;
                    stmts.emplace_back(syntax_stmt{rval});
                }
                // 分配全局变量
                for (auto &v : def.var_list)
                {
                    auto var = std::make_shared<syntax_expr>();
                    var->val = syntax_var{.alloc_type = syntax_var::STATIC};
                    var->type = t;

                    // 声明
                    env_var.insert(v.val, var);

                    // 初始化
                    syntax_assign assign{.lval = var, .rval = rval};
                    stmts.push_back(syntax_stmt{.stmt = assign});
                }
            }
        }
    }
    // todo: main 函数类型检查
    auto user_main_fun = env_fun.get_user_fun(".main");

    auto call_main = std::make_shared<syntax_expr>();
    auto call_struct = syntax_fun_call();
    call_struct.fun_name = ".main";
    call_main->type = user_main_fun.ret_type;
    call_main->val = call_struct;

    stmts.push_back(syntax_stmt{call_main});

    // return 0
    auto zero = std::make_shared<syntax_expr>();
    auto i32 = env_type.get_type("i32");
    zero->type = i32;
    zero->val = syntax_literal{.type = i32, .val = (unsigned long long)0};
    stmts.push_back(syntax_stmt{zero});
    stmts.push_back(syntax_stmt{syntax_return{zero}});

    fun_impl.emplace_back(std::make_pair("main", std::move(stmts)));
    env_fun.add_func(main_fun);
}

// todo:
syntax_stmt syntax_module::if_analysis(const node_if_statement &node)
{
    syntax_if_block block;
    env_var.push();
    block.condition_stmt.push_back(std::vector<syntax_stmt>());
    block.condition.push_back(expr_analysis(node.if_condition, block.condition_stmt[0]));
    block.branch.push_back(statement_analysis(node.if_statement));
    block.defaul_branch = statement_analysis(node.else_statement);
    auto it_condition = node.else_if_statement.else_if_condition.begin();
    auto it_statement = node.else_if_statement.else_if_statement.begin();
    for (int i = 1; it_condition != node.else_if_statement.else_if_condition.end(); it_condition++, it_statement++, i++)
    {
        block.condition_stmt.push_back(std::vector<syntax_stmt>());
        block.condition.push_back(expr_analysis(*it_condition, block.condition_stmt[i]));
        block.branch.push_back(statement_analysis(*it_statement));
    }
    env_var.pop();
    return {block};
}

syntax_stmt syntax_module::while_analysis(const node_while_statement &node)
{
    syntax_while_block block;
    env_var.push();
    block.condition = expr_analysis(node.while_condition, block.condition_stmt);
    //add while condition to the statement
    auto new_node = node;
    auto condition_stmt = node_statement{.loc = node.loc, .statement = node.while_condition};
    new_node.loop_statement.push_back(condition_stmt);
    //-------------------------------------
    block.body = statement_analysis(new_node.loop_statement);
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
    try
    {
        if (auto p = std::get_if<node_binary_expr>(&node.expr))
        {
            return binary_expr_analysis(*p, stmts);
        }
        else if (auto p = std::get_if<node_assign_expr>(&node.expr))
        {
            auto syntax_node_r = expr_analysis(*(p->rval), stmts);
            auto syntax_node_l = is_left_value(p->lval);
            if (p->op != "=")
            {
                auto syntax_fun_node = syntax_fun_call{.fun_name = p->op.substr(0, 1), .parameters = {syntax_node_l, syntax_node_r}};
                auto syntax_node = env_fun.infer_type(p->op.substr(0, 1), syntax_fun_node);
                stmts.push_back(syntax_stmt{.stmt = syntax_node});
                stmts.push_back(syntax_stmt{.stmt = syntax_assign{.lval = syntax_node_l, .rval = syntax_node}});
                return syntax_node_l;
            }
            else
            {
                stmts.push_back(syntax_stmt{.stmt = syntax_assign{.lval = syntax_node_l, .rval = syntax_node_r}});
                return syntax_node_l;
            }
        }
        else if (auto p = std::get_if<node_construct_expr>(&node.expr))
        {
            return nullptr;
        }
        else
            assert(false);
    }
    catch (inner_error &e)
    {
        if (e.number == INNER_NO_MATCH_FUN)
        {
            throw syntax_error(node.loc, "no such function called '" + e.info + "'");
        }
    }
}

std::shared_ptr<syntax_expr> syntax_module::binary_expr_analysis(const node_binary_expr &node, std::vector<syntax_stmt> &stmts)
{
    static std::map<std::string, int> precedence =
        {
            {"||", 0}, {"&&", 1}, {"^", 2}, {"==", 3}, {"!=", 3}, {">", 4}, {"<", 4}, {"<=", 4}, {">=", 4}, {"<<", 5}, {">>", 5}, {"+", 6}, {"-", 6}, {"*", 7}, {"/", 7}, {"%", 7}};
    if (node.ops.empty())
    {
        return unary_expr_analysis(node.vars[0], stmts);
    }
    std::vector<std::variant<node_unary_expr, std::shared_ptr<syntax_expr>>> expr_stack;
    std::vector<std::string> op_stack;
    op_stack.push_back(node.ops[0]);
    expr_stack.push_back(node.vars[0]);
    expr_stack.push_back(node.vars[1]);
    for (auto i = 1; i < node.ops.size(); i++)
    {
        if (precedence[op_stack[op_stack.size() - 1]] >= precedence[node.ops[i]])
        {
            std::shared_ptr<syntax_expr> syntax_node;
            auto fun_name = op_stack[op_stack.size() - 1];
            auto lexp = expr_stack[expr_stack.size() - 2];
            auto rexp = expr_stack[expr_stack.size() - 1];
            if (auto p = std::get_if<node_unary_expr>(&lexp))
            {
                if (auto q = std::get_if<node_unary_expr>(&rexp))
                {
                    auto syntax_node_l = unary_expr_analysis(*p, stmts);
                    auto syntax_node_r = unary_expr_analysis(*q, stmts);
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {syntax_node_l, syntax_node_r}};
                    syntax_node = env_fun.infer_type(fun_name, syntax_fun_node);
                }
                else if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
                {
                    auto syntax_node_l = unary_expr_analysis(*p, stmts);
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {syntax_node_l, *q}};
                    syntax_node = env_fun.infer_type(fun_name, syntax_fun_node);
                }
                else
                    assert(false);
            }
            else if (auto p = std::get_if<std::shared_ptr<syntax_expr>>(&lexp))
            {
                if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
                {
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, *q}};
                    syntax_node = env_fun.infer_type(fun_name, syntax_fun_node);
                }
                else if (auto q = std::get_if<node_unary_expr>(&rexp))
                {
                    auto syntax_node_r = unary_expr_analysis(*q, stmts);
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, syntax_node_r}};
                    syntax_node = env_fun.infer_type(fun_name, syntax_fun_node);
                }
                else
                    assert(false);
            }
            else
                assert(false);
            stmts.push_back(syntax_stmt{.stmt = syntax_node});
            expr_stack.pop_back();
            expr_stack.pop_back();
            op_stack.pop_back();
            op_stack.push_back(node.ops[i]);
            expr_stack.push_back(syntax_node);
            expr_stack.push_back(node.vars[i + 1]);
        }
        else
        {
            op_stack.push_back(node.ops[i]);
            expr_stack.push_back(node.vars[i + 1]);
        }
    }
    while (!op_stack.empty())
    {
        std::shared_ptr<syntax_expr> syntax_node;
        auto fun_name = op_stack[op_stack.size() - 1];
        auto lexp = expr_stack[expr_stack.size() - 2];
        auto rexp = expr_stack[expr_stack.size() - 1];
        if (auto p = std::get_if<node_unary_expr>(&lexp))
        {
            if (auto q = std::get_if<node_unary_expr>(&rexp))
            {
                auto syntax_node_l = unary_expr_analysis(*p, stmts);
                auto syntax_node_r = unary_expr_analysis(*q, stmts);
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {syntax_node_l, syntax_node_r}};
                syntax_node = env_fun.infer_type(fun_name, syntax_fun_node);
            }
            else if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
            {
                auto syntax_node_l = unary_expr_analysis(*p, stmts);
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {syntax_node_l, *q}};
                syntax_node = env_fun.infer_type(fun_name, syntax_fun_node);
            }
            else
                assert(false);
        }
        else if (auto p = std::get_if<std::shared_ptr<syntax_expr>>(&lexp))
        {
            if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
            {
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, *q}};
                syntax_node = env_fun.infer_type(fun_name, syntax_fun_node);
            }
            else if (auto q = std::get_if<node_unary_expr>(&rexp))
            {
                auto syntax_node_r = unary_expr_analysis(*q, stmts);
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, syntax_node_r}};
                syntax_node = env_fun.infer_type(fun_name, syntax_fun_node);
            }
            else
                assert(false);
        }
        else
            assert(false);
        stmts.push_back(syntax_stmt{.stmt = syntax_node});
        expr_stack.pop_back();
        expr_stack.pop_back();
        op_stack.pop_back();
        expr_stack.push_back(syntax_node);
    }
    if (auto ret = std::get_if<std::shared_ptr<syntax_expr>>(&expr_stack[0]))
    {
        return *ret;
    }
    throw std::string("binary_expr_analysis");
}

std::shared_ptr<syntax_expr> syntax_module::unary_expr_analysis(const node_unary_expr &node, std::vector<syntax_stmt> &stmts)
{
    if (node.ops.empty())
    {
        return post_expr_analysis(*node.post_expr.get(), stmts);
    }
    else
    {
        auto p = post_expr_analysis(*node.post_expr.get(), stmts);
        auto call = syntax_fun_call{.fun_name = node.ops[0], .parameters = {p}};
        auto syntax_unary = env_fun.infer_type(node.ops[0], call);
        stmts.push_back(syntax_stmt{.stmt = syntax_unary});
        return syntax_unary;
    }
}

std::shared_ptr<syntax_expr> syntax_module::post_expr_analysis(const node_post_expr &node, std::vector<syntax_stmt> &stmts)
{
    if (auto p = std::get_if<node_primary_expr>(&node.expr))
    {
        if (auto q = std::get_if<node_identifier>(&p->val))
        {
            try
            {
                // 这里不能加
                // 因为加了就会有一次分配行为
                // 而分配空间是在 val,var 语句里做的
                // stmts.push_back(syntax_stmt{.stmt = env_var.find(q->val)});
                // 这里应该是 load
                return env_var.find(q->val);
            }
            catch (const std::string &e)
            {
                throw e;
            }
        }
        else if (auto q = std::get_if<node_constant>(&p->val))
        {
            auto type_name = std::get_if<node_identifier>(&(q->type->type_val));
            auto type = env_type.get_type(type_name->val);
            auto literal = syntax_literal{.type = type, .val = q->val};
            auto syntax_node = std::make_shared<syntax_expr>();
            syntax_node->type = type;
            syntax_node->val = literal;
            stmts.push_back(syntax_stmt{.stmt = syntax_node});
            return syntax_node;
        }
        else if (auto q = std::get_if<std::shared_ptr<node_expression>>(&p->val))
        {
            return expr_analysis(*(q->get()), stmts);
        }
        else
            assert(false);
    }
    else if (auto p = std::get_if<node_post_call_expr>(&node.expr))
    {
        auto fun_name_ptr = std::get_if<node_primary_expr>(&p->callable->expr);
        auto fun_name = std::get_if<node_identifier>(&fun_name_ptr->val);
        auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name->val};
        for (auto para : p->exp_list.arr)
        {
            syntax_fun_node.parameters.push_back(expr_analysis(*para, stmts));
        }
        auto syntax_node = env_fun.infer_type(fun_name->val, syntax_fun_node);
        stmts.push_back(syntax_stmt{.stmt = syntax_node});
        return syntax_node;
    }
    else if (auto p = std::get_if<node_post_dot_expr>(&node.expr))
    {
        return nullptr;
    }
    else if (auto p = std::get_if<node_post_check_expr>(&node.expr))
    {
        return nullptr;
    }
    else
        assert(false);
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

    stmts = statement_analysis(node.origin_stmts);

    // 分析结束，保存函数实现
    fun_impl.emplace_back(std::make_pair(node.fun_name, std::move(stmts)));
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
            auto init_expr = expr_analysis(def->initial_exp, stmts);
            auto rval = std::make_shared<syntax_expr>();

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
                stmts.emplace_back(syntax_stmt{rval});
            }

            // 分配局部变量
            for (auto &v : def->var_list)
            {
                auto var = std::make_shared<syntax_expr>();
                var->val = syntax_var{.alloc_type = syntax_var::STACK};
                var->type = t;

                // 声明
                env_var.insert(v.val, var);
                stmts.push_back(syntax_stmt{var});

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

std::shared_ptr<syntax_expr> syntax_module::is_left_value(const node_unary_expr &node)
{
    if (auto p = std::get_if<node_primary_expr>(&node.post_expr->expr))
    {
        if (auto q = std::get_if<node_identifier>(&p->val))
        {
            return env_var.find(q->val);
        }
        else
        {
            throw std::string("not lval");
        }
    }
    else
    {
        throw std::string("not lval");
    }
}