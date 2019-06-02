#include "syntax_tree.h"

// todo: 补全该函数
std::shared_ptr<syntax_expr> syntax_module::expr_analysis(const node_expression &node, std::vector<syntax_stmt> &stmts)
{
    try
    {
        // 二元表达式
        // 1+2-3*4
        // fun(2,3)
        // x.y
        if (auto p = std::get_if<node_binary_expr>(&node.expr))
        {
            return binary_expr_analysis(*p, stmts);
        }
        
        // x = y = z
        // x.y = 1
        // x.y = {1, 2}
        else if (auto p = std::get_if<node_assign_expr>(&node.expr))
        {
            auto syntax_node_l = unary_expr_analysis(p->lval, stmts);
            if (!is_left_value(*syntax_node_l))
                throw syntax_error(p->loc, "left oprand of assignment is not lvalue");
            
            // is immutable?
            if(is_lvalue_immutale(*syntax_node_l))
                throw syntax_error(p->loc, "left oprand of assignment is immutable");
            if(auto construct = std::get_if<node_construct_expr>(&p->rval->expr))
            {
                auto type_node = syntax_node_l->type;
                if(auto product_t = std::get_if<product_type>(&type_node.type))
                {
                    auto i = 0;
                    auto j = 0;
                    for(i = 0; i < construct->lable.size(); i++)
                    {
                        for(j = 0; j < product_t->fields.size(); j++)
                        {
                            if(construct->lable[i] == product_t->fields[j])
                            {
                                break;
                            }
                        }
                        if(j >= product_t->fields.size())
                        {
                            throw syntax_error(p->loc, "no corresponding attribute in the assignment");
                        }
                        else
                        {
                            auto node_construct = node_expression{.loc = p->loc, .expr = node_assign_expr{.loc = p->loc, .lval = node_unary_expr{.loc = p->loc}, .op = "=", .rval = construct->init_val[j]}};
                            auto tmp = std::get_if<node_assign_expr>(&node_construct.expr);
                            tmp->lval.post_expr = std::make_shared<node_post_expr>();
                            tmp->lval.post_expr->loc = p->loc;
                            node_post_dot_expr node_dot;
                            node_dot.loc = p->loc;
                            node_dot.obj = p->lval.post_expr;
                            node_dot.attr.loc = p->loc;
                            node_dot.attr.val = construct->lable[j];
                            tmp->lval.post_expr->expr = node_dot;
                            expr_analysis(node_construct, stmts);
                        }
                    }
                }
                else if(auto sum_t = std::get_if<sum_type>(&type_node.type))
                {
                    auto i = 0;
                    if(construct->lable.size() != 1)
                        throw syntax_error(p->loc, "sum type assignment error, not match");
                    for(i = 0; i < sum_t->alters.size(); i++)
                    {
                        if(sum_t->alters[i] == construct->lable[0])
                        {
                            break;
                        }
                    }
                    if(i >= sum_t->alters.size())
                    {
                        throw syntax_error(p->loc, "sum type assignment error, not match any");
                    }
                    else
                    {
                        auto node_construct = node_expression{.loc = p->loc, .expr = node_assign_expr{.loc = p->loc, .lval = node_unary_expr{.loc = p->loc}, .op = "=", .rval = construct->init_val[i]}};
                        auto tmp = std::get_if<node_assign_expr>(&node_construct.expr);
                        tmp->lval.post_expr = std::make_shared<node_post_expr>();
                        tmp->lval.post_expr->loc = p->loc;
                        node_post_dot_expr node_dot;
                        node_dot.loc = p->loc;
                        node_dot.obj = p->lval.post_expr;
                        node_dot.attr.loc = p->loc;
                        node_dot.attr.val = construct->lable[i];
                        tmp->lval.post_expr->expr = node_dot;
                        expr_analysis(node_construct, stmts);
                    }
                }
                else
                {
                    throw syntax_error(p->loc, "lval is not sum type or product type");
                }
                return syntax_node_l;
            }
            else 
            {

                auto syntax_node_r = expr_analysis(*(p->rval), stmts);
                if (p->op != "=")
                {
                    auto syntax_fun_node = syntax_fun_call{.fun_name = p->op.substr(0, 1), .parameters = {syntax_node_l, syntax_node_r}};
                    auto syntax_node = env_fun.infer_type(p->op.substr(0, 1), syntax_fun_node, stmts);
                    // if(!syntax_node->type.subtyping(syntax_node_l->type))
                    // {
                    //     throw syntax_error(p->loc, "assignment oprands not match");
                    // }
                    try
                    {
                        syntax_node = expr_convert_to(syntax_node, syntax_node_l->type, stmts);
                    }
                    catch(inner_error &e)
                    {
                        throw syntax_error(p->loc, "assignment oprands not match");
                    }
                    stmts.push_back(syntax_stmt{.stmt = syntax_node});
                    stmts.push_back(syntax_stmt{.stmt = syntax_assign{.lval = syntax_node_l, .rval = syntax_node}});
                    return syntax_node_l;
                    }
                else
                {
                    if(syntax_node_l->type.is_product())
                    {
                        if(syntax_node_r->type.subtyping(syntax_node_l->type))
                        {
                            construct_assign(syntax_node_l, syntax_node_r, stmts);
                            return syntax_node_l;
                        }   
                        else
                        {
                            throw syntax_error(p->loc, "product type assignment no match");
                        }
                    }
                    else if(syntax_node_l->type.is_sum())
                    {
                        if(syntax_node_r->type.subtyping(syntax_node_l->type))
                        {
                            construct_assign(syntax_node_l, syntax_node_r, stmts);
                            return syntax_node_l;
                        }
                        else
                        {
                            throw syntax_error(p->loc, "sum type assignment not match");
                        }
                    }
                    primary_assign(syntax_node_l, syntax_node_r, stmts);
                    return syntax_node_l;
                }
            }
        }

        //
        else if (auto p = std::get_if<node_construct_expr>(&node.expr))
        {
            auto syntax_node = std::make_shared<syntax_expr>();
            syntax_construct syntax_construct_node;
            syntax_type type_node;
            product_type product_node;
            if(p->lable.size() != 1)
            {
                for(auto i = 0; i < p->lable.size(); i++)
                {
                    auto syntax_ret = expr_analysis(*(p->init_val[i]), stmts);
                    product_node.fields.push_back(p->lable[i]);
                    auto syntax_type_node = std::make_shared<syntax_type>();
                    syntax_type_node->type = syntax_ret->type.type;
                    product_node.types.push_back(syntax_type_node);
                    syntax_construct_node.label.push_back(p->lable[i]);
                    syntax_construct_node.val.push_back(syntax_ret);
                }
                syntax_type type_node;
                type_node.type = product_node;
            }
            else
            {
                sum_type sum_node;
                auto syntax_ret = expr_analysis(*(p->init_val[0]), stmts);
                sum_node.alters.push_back(p->lable[0]);
                auto syntax_type_node = std::make_shared<syntax_type>();
                syntax_type_node->type = syntax_ret->type.type;
                sum_node.types.push_back(syntax_type_node);
                syntax_type type_node;
                type_node.type = product_node;
            }
            syntax_node->type = type_node;
            syntax_node->val = syntax_construct_node;
            return syntax_node;
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
    assert(false);
}

std::shared_ptr<syntax_expr> syntax_module::binary_expr_analysis(const node_binary_expr &node, std::vector<syntax_stmt> &stmts)
{
    static std::map<std::string, int> precedence =
        {
            {"||", 0}, {"&&", 1}, {"^", 2}, {"==", 3}, {"!=", 3}, {">", 4}, {"<", 4}, {"<=", 4}, {">=", 4}, {"<<", 5}, {">>", 5}, {"+", 6}, {"-", 6}, {"*", 7}, {"/", 7}, {"%", 7}};

    // fun(2, 3)
    // 单个字面量
    if (node.ops.empty())
    {
        return unary_expr_analysis(node.vars[0], stmts);
    }
    // 处理binary expression 优先级
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
                    syntax_node = env_fun.infer_type(fun_name, syntax_fun_node, stmts);
                }
                else if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
                {
                    auto syntax_node_l = unary_expr_analysis(*p, stmts);
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {syntax_node_l, *q}};
                    syntax_node = env_fun.infer_type(fun_name, syntax_fun_node, stmts);
                }
                else
                    assert(false);
            }
            else if (auto p = std::get_if<std::shared_ptr<syntax_expr>>(&lexp))
            {
                if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
                {
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, *q}};
                    syntax_node = env_fun.infer_type(fun_name, syntax_fun_node, stmts);
                }
                else if (auto q = std::get_if<node_unary_expr>(&rexp))
                {
                    auto syntax_node_r = unary_expr_analysis(*q, stmts);
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, syntax_node_r}};
                    syntax_node = env_fun.infer_type(fun_name, syntax_fun_node, stmts);
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
                syntax_node = env_fun.infer_type(fun_name, syntax_fun_node, stmts);
            }
            else if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
            {
                auto syntax_node_l = unary_expr_analysis(*p, stmts);
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {syntax_node_l, *q}};
                syntax_node = env_fun.infer_type(fun_name, syntax_fun_node, stmts);
            }
            else
                assert(false);
        }
        else if (auto p = std::get_if<std::shared_ptr<syntax_expr>>(&lexp))
        {
            if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
            {
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, *q}};
                syntax_node = env_fun.infer_type(fun_name, syntax_fun_node, stmts);
            }
            else if (auto q = std::get_if<node_unary_expr>(&rexp))
            {
                auto syntax_node_r = unary_expr_analysis(*q, stmts);
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, syntax_node_r}};
                syntax_node = env_fun.infer_type(fun_name, syntax_fun_node, stmts);
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
        // 单目运算符
        auto p = post_expr_analysis(*node.post_expr.get(), stmts);
        auto call = syntax_fun_call{.fun_name = node.ops[0], .parameters = {p}};
        auto syntax_unary = env_fun.infer_type(node.ops[0], call, stmts);
        stmts.push_back(syntax_stmt{.stmt = syntax_unary});
        return syntax_unary;
    }
}

std::shared_ptr<syntax_expr> syntax_module::post_expr_analysis(const node_post_expr &node, std::vector<syntax_stmt> &stmts)
{
    if (auto p = std::get_if<node_primary_expr>(&node.expr))
    {
        // 变量
        if (auto q = std::get_if<node_identifier>(&p->val))
        {
            try
            {
                return env_var.find(q->val);
            }
            catch (const std::string &e)
            {
                throw e;
            }
        }
        // 字面量
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
        // 嵌套的其他表达式
        else if (auto q = std::get_if<std::shared_ptr<node_expression>>(&p->val))
        {
            return expr_analysis(*(q->get()), stmts);
        }
        else
            assert(false);
    }

    // 函数表达式
    else if (auto p = std::get_if<node_post_call_expr>(&node.expr))
    {
        auto fun_name_ptr = std::get_if<node_primary_expr>(&p->callable->expr);
        auto fun_name = std::get_if<node_identifier>(&fun_name_ptr->val);
        auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name->val};
        for (auto para : p->exp_list.arr)
        {
            // 分析函数的参数
            syntax_fun_node.parameters.push_back(expr_analysis(*para, stmts));
        }
        auto syntax_node = env_fun.infer_type(fun_name->val, syntax_fun_node, stmts);
        stmts.push_back(syntax_stmt{.stmt = syntax_node});
        return syntax_node;
    }

    // x.y
    else if (auto p = std::get_if<node_post_dot_expr>(&node.expr))
    {
        auto syntax_node = post_expr_analysis(*(p->obj), stmts);
        auto e = std::get_if<syntax_var>(&syntax_node->val);
        auto q = std::get_if<syntax_dot>(&syntax_node->val);
        if (e || q)
        {
            if(syntax_node->type.get_primary().empty())
            {
                auto syntax_ret = std::make_shared<syntax_expr>();
                if (auto product_t = std::get_if<product_type>(&(syntax_node->type).type))
                {
                    bool flag = false;
                    int i = 0;
                    for (i = 0; i < product_t->fields.size(); i++)
                    {
                        if (product_t->fields[i] == (p->attr).val)
                        {
                            flag = true;
                            break;
                        }
                    }
                    if (flag)
                    {
                        syntax_ret->is_immutale = syntax_node->is_immutale;
                        syntax_ret->type = *(product_t->types[i]);
                        syntax_ret->val = syntax_dot{.field = (p->attr).val, .val = syntax_node};
                        return syntax_ret;
                    }
                    else
                        throw syntax_error(p->loc, "no such attribute");
                }
                else if (auto sum_t = std::get_if<sum_type>(&(syntax_node->type).type))
                {
                    auto syntax_ret = std::make_shared<syntax_expr>();
                    bool flag = false;
                    int i;
                    for (i = 0; i < sum_t->alters.size(); i++)
                    {
                        if (p->attr.val == sum_t->alters[i])
                        {
                            flag = true;
                            break;
                        }
                    }
                    if (flag)
                    {
                        syntax_ret->is_immutale = syntax_node->is_immutale;
                        syntax_ret->type = *(sum_t->types[i]);
                        syntax_ret->val = syntax_dot{.field = p->attr.val, .val = syntax_node};
                        return syntax_ret;
                    }
                    else
                        throw syntax_error(p->loc, "no such attribute");
                }
                else
                    assert(false);
            }
            else
                throw syntax_error(p->loc, "lvalue is not product type or sum type");
        }
        else
            throw syntax_error(p->loc, "not lvalue");
    }

    // . ?
    // x.y.z?
    // x.y?
    else if (auto p = std::get_if<node_post_check_expr>(&node.expr))
    {
        auto syntax_check_exp = post_expr_analysis(*(p->check_exp), stmts);
        if(auto q = std::get_if<sum_type>(&syntax_check_exp->type.type))
        {
            auto syntax_fun_node = syntax_fun_call{.fun_name = "." + p->check_lable.val + "?", .parameters = {syntax_check_exp}};
            auto syntax_node = env_fun.infer_type(("." + p->check_lable.val + "?"), syntax_fun_node, stmts);
            stmts.push_back(syntax_stmt{.stmt = syntax_node});
            return syntax_node;
        }
        else
        {
            throw syntax_error(p->loc, "? must be used with sum_type");
        }
    }
    else
        assert(false);
}

bool syntax_module::is_left_value(const syntax_expr &node)
{
    if (auto p = std::get_if<syntax_dot>(&node.val))
        return true;
    else if (auto p = std::get_if<syntax_var>(&node.val))
        return true;
    else
        return false;
}

void syntax_module::primary_assign(std::shared_ptr<syntax_expr> &lval, std::shared_ptr<syntax_expr> &rval, std::vector<syntax_stmt> &stmts)
{
    rval = expr_convert_to(rval, lval->type, stmts);
    stmts.push_back(syntax_stmt{.stmt = syntax_assign{.lval = lval, .rval = rval}});
}

void syntax_module::construct_assign(std::shared_ptr<syntax_expr> &lval, std::shared_ptr<syntax_expr> &rval, std::vector<syntax_stmt> &stmts)
{
    if(auto construct = std::get_if<syntax_construct>(&rval->val))
    {
        if(lval->type.is_product())
        {
            auto p = std::get_if<product_type>(&lval->type.type);
            if(construct->label.size() < p->fields.size())
            {
                throw inner_error();
            }
            for(auto i = 0; i < p->fields.size(); i++)
            {
                if(p->fields[i] != construct->label[i])
                {
                    throw inner_error();
                }
                auto lval_node = std::make_shared<syntax_expr>();
                auto lval_dot = syntax_dot{.field = p->fields[i], .val = lval};
                lval_node->type.type = p->types[i]->type;
                lval_node->val = lval_dot;
                construct_assign(lval_node, construct->val[i], stmts);
            }
        }
        else if(lval->type.is_sum())
        {
            auto p = std::get_if<sum_type>(&lval->type.type);
            if(construct->label.size() != 1)
            {
                throw inner_error();
            }
            auto i = 0;
            for(i = 0; i < p->alters.size(); i++)
            {
                if(p->alters[i] == construct->label[0])
                {
                    break;
                }
            }
            if(i >= p->alters.size())
            {
                throw inner_error();
            }
            auto lval_node = std::make_shared<syntax_expr>();
            auto lval_dot = syntax_dot{.field = p->alters[i], .val = lval};
            lval_node->type.type = p->types[i]->type;
            lval_node->val = lval_dot;
            construct_assign(lval_node, construct->val[0], stmts);
        }
        else
            assert(false);
    }
    else 
    {
        if(lval->type.get_primary() != "")
        {
            primary_assign(lval, rval, stmts);
        }
        else
        {
            if(lval->type.is_product())
            {
                auto p = std::get_if<product_type>(&lval->type.type);
                auto q = std::get_if<product_type>(&rval->type.type);
                for(auto i = 0; i < p->fields.size(); i++)
                {
                    auto lval_node = std::make_shared<syntax_expr>();
                    auto lval_dot = syntax_dot{.field = p->fields[i], .val = lval};
                    auto rval_dot = syntax_dot{.field = q->fields[i], .val = rval};
                    auto rval_node = std::make_shared<syntax_expr>();
                    lval_node->type.type = p->types[i]->type;
                    lval_node->val = lval_dot;
                    rval_node->type.type = q->types[i]->type;
                    rval_node->val = rval_dot;
                    construct_assign(lval_node, rval_node, stmts);
                }
            }
            else if(lval->type.is_sum())
            {
                assert(false);
            }
            else
            {
                assert(false);
            } 
        }
    }
}
