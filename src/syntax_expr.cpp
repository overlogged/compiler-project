#include "syntax_tree.h"

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
            auto syntax_node_l = unary_expr_analysis(p->lval, stmts);
            if(!is_left_value(*syntax_node_l))
                throw syntax_error(p->loc, "left oprand of assignment is not lvalue");
                //is immutable?
            if(is_lvalue_immutale(*syntax_node_l))
                throw syntax_error(p->loc, "left oprand of assignment is immutable");
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
        auto syntax_node = post_expr_analysis(*(p->obj), stmts);
        auto e = std::get_if<syntax_var>(&syntax_node->val);
        auto q = std::get_if<syntax_dot>(&syntax_node->val);
        if(e || q)
        {
            if(!syntax_node->type.get_primary().empty())
            {
                auto syntax_ret = std::make_shared<syntax_expr>();
                if(auto product_t = std::get_if<product_type>(&(syntax_node->type).type))
                {
                    bool flag = false;
                    int i = 0;
                    for(i = 0; i < product_t->fields.size(); i++)
                    {
                        if(product_t->fields[i] == (p->attr).val)
                        {
                            flag = true;
                            break;
                        }
                    }
                    if(flag)
                    {
                        syntax_ret->is_immutale = syntax_node->is_immutale;
                        syntax_ret->type = *(product_t->types[i]);
                        syntax_ret->val = syntax_dot{.field = (p->attr).val, .val = syntax_node};
                        return syntax_ret;
                    }
                    else
                        throw syntax_error(p->loc, "no such attribute");
                }
                else if(auto sum_t = std::get_if<sum_type>(&(syntax_node->type).type))
                {
                    auto syntax_ret = std::make_shared<syntax_expr>();
                    bool flag = false;
                    int i;
                    for(i = 0; i < sum_t->alters.size(); i++)
                    {
                        if(p->attr.val == sum_t->alters[i])
                        {
                            flag = true;
                            break;
                        }
                    }
                    if(flag)
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
    else if (auto p = std::get_if<node_post_check_expr>(&node.expr))
    {
        auto syntax_check_exp = post_expr_analysis(*(p->check_exp), stmts);
        auto syntax_fun_node = syntax_fun_call{.fun_name = "."+p->check_lable.val+"?", .parameters = {syntax_check_exp}};
        auto syntax_node = env_fun.infer_type(("."+p->check_lable.val+"?"), syntax_fun_node);
        stmts.push_back(syntax_stmt{.stmt = syntax_node});
        return syntax_node;
    }
    else
        assert(false);
}

bool syntax_module::is_left_value(const syntax_expr &node)
{
    if(auto p = std::get_if<syntax_dot>(&node.val))
        return true;
    else if(auto p = std::get_if<syntax_var>(&node.val))
        return true;
    else
        return false;
}