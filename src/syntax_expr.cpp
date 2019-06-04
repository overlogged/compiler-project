#include "syntax_tree.h"

std::shared_ptr<syntax_expr> syntax_module::assign_analysis(const node_assign_expr &node, std::vector<syntax_stmt> &stmts)
{
    // 准备左值
    auto syntax_node_l = unary_expr_analysis(node.lval, stmts);

    if (!syntax_node_l->is_left_value())
        throw syntax_error(node.loc, "left oprand of assignment is not lvalue");

    // is immutable?
    if (syntax_node_l->is_immutable())
        throw syntax_error(node.loc, "left oprand of assignment is immutable");

    // 准备右值
    auto syntax_node_r = expr_analysis(*(node.rval), stmts);

    // 类型转换
    try
    {
        if (node.op != "=")
        {
            auto syntax_fun_node = syntax_fun_call{.fun_name = node.op.substr(0, 1), .parameters = {syntax_node_l, syntax_node_r}};
            syntax_node_r = env_fun.infer_type(syntax_fun_node, stmts);
            stmts.push_back(syntax_stmt{syntax_node_r});
        }
        syntax_node_r = expr_convert_to(syntax_node_r, syntax_node_l->type, stmts);
    }
    catch (inner_error &e)
    {
        throw syntax_error(node.loc, "assignment oprands not match");
    }

    // 最终赋值

    // 判断是不是 sum type 的左值，如果是，则加入对 tag 的赋值
    try_sum_tag_assign(syntax_node_l, stmts);
    stmts.push_back(syntax_stmt{.stmt = syntax_assign{.lval = syntax_node_l, .rval = syntax_node_r}});
    return syntax_node_l;
}

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
            return assign_analysis(*p, stmts);
        }

        // 分析 construct_expr，得到一个 product type 的值
        else if (auto p = std::get_if<node_construct_expr>(&node.expr))
        {
            // 构造 syntax_construct 表达式
            syntax_construct syntax_construct_node;
            syntax_type type_node;
            product_type product_node;
            for (auto i = 0; i < p->lable.size(); i++)
            {
                auto syntax_ret = expr_analysis(*(p->init_val[i]), stmts);
                product_node.fields.push_back(p->lable[i]);
                auto syntax_type_node = std::make_shared<syntax_type>();
                syntax_type_node->type = syntax_ret->type.type;
                product_node.types.push_back(syntax_type_node);
                syntax_construct_node.label.push_back(p->lable[i]);
                syntax_construct_node.val.push_back(syntax_ret);
            }
            type_node.type = product_node;

            // 为临时的 product type 值分配空间
            auto syntax_node = std::make_shared<syntax_expr>(syntax_var(), type_node);
            stmts.push_back(syntax_stmt{.stmt = syntax_node});

            // 利用 syntax_construct_node 赋值构造一个临时的 product_type
            for (auto i = 0; i < p->lable.size(); i++)
            {
                auto lval_dot = syntax_dot{
                    .field = syntax_construct_node.label[i],
                    .val = syntax_node};

                auto lval = std::make_shared<syntax_expr>(lval_dot, syntax_construct_node.val[i]->type);
                stmts.push_back(syntax_stmt{.stmt = lval});

                syntax_assign assign{.lval = lval, .rval = syntax_construct_node.val[i]};
                stmts.push_back(syntax_stmt{.stmt = assign});
            }
            return syntax_node;
        }

        // new
        else if (auto p = std::get_if<node_new_expr>(&node.expr))
        {
            auto count = expr_analysis(*p->size, stmts);
            auto type_u64 = syntax_type{primary_type{.name = "u64", .size = 8}};
            if (count->type.subtyping(type_u64))
            {
                auto count_expr = expr_convert_to(count, type_u64, stmts);
                auto member_type = env_type.type_check(p->new_type);
                auto ptr = syntax_type{
                    .type = pointer_type{
                        std::make_shared<syntax_type>(member_type)}};
                auto expr = std::make_shared<syntax_expr>(syntax_new_expr{.type = member_type, .count = count_expr}, ptr);
                stmts.push_back(syntax_stmt{expr});
                return expr;
            }
            else
            {
                throw syntax_error(p->loc, "index must be integers");
            }
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
        else if (e.number == INNER_CANT_CAST)
        {
            throw syntax_error(node.loc, "can't do such type conversion");
        }
        else
        {
            throw e;
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
                    syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
                }
                else if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
                {
                    auto syntax_node_l = unary_expr_analysis(*p, stmts);
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {syntax_node_l, *q}};
                    syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
                }
                else
                    assert(false);
            }
            else if (auto p = std::get_if<std::shared_ptr<syntax_expr>>(&lexp))
            {
                if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
                {
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, *q}};
                    syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
                }
                else if (auto q = std::get_if<node_unary_expr>(&rexp))
                {
                    auto syntax_node_r = unary_expr_analysis(*q, stmts);
                    auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, syntax_node_r}};
                    syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
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
                syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
            }
            else if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
            {
                auto syntax_node_l = unary_expr_analysis(*p, stmts);
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {syntax_node_l, *q}};
                syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
            }
            else
                assert(false);
        }
        else if (auto p = std::get_if<std::shared_ptr<syntax_expr>>(&lexp))
        {
            if (auto q = std::get_if<std::shared_ptr<syntax_expr>>(&rexp))
            {
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, *q}};
                syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
            }
            else if (auto q = std::get_if<node_unary_expr>(&rexp))
            {
                auto syntax_node_r = unary_expr_analysis(*q, stmts);
                auto syntax_fun_node = syntax_fun_call{.fun_name = fun_name, .parameters = {*p, syntax_node_r}};
                syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
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
    // 单目运算符
    std::shared_ptr<syntax_expr> expr = post_expr_analysis(*node.post_expr.get(), stmts);
    for (auto op : node.ops)
    {
        if (op == "*")
        {
            try
            {
                auto arr = expr;
                auto idx = std::make_shared<syntax_expr>();
                idx->type = syntax_type{primary_type{.name = "u64", .size = 8}};
                idx->val = syntax_literal{.type = idx->type, .val = (unsigned long long)0};
                stmts.push_back(syntax_stmt{idx});

                auto member = std::make_shared<syntax_expr>();
                member->type = arr->type.de_ref();
                member->val = syntax_arr_member{
                    .base = arr,
                    .idx = idx};
                stmts.push_back(syntax_stmt{member});
                expr = member;
            }
            catch (inner_error &)
            {
                throw syntax_error(node.loc, "* operator misused");
            }
        }
        else
        {
            auto call = syntax_fun_call{.fun_name = op, .parameters = {expr}};
            auto syntax_unary = env_fun.infer_type(call, stmts);
            stmts.push_back(syntax_stmt{.stmt = syntax_unary});
            expr = syntax_unary;
        }
    }
    return expr;
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
            std::variant<unsigned long long, double, float, std::string> val = q->val;
            if (type_name->val == "char")
            {
                char ch;
                if (trans_char(std::get<std::string>(q->val), ch))
                {
                    val = (unsigned long long)ch;
                }
                else
                {
                    throw syntax_error{p->loc, "invalid char"};
                }
            }
            else if (type_name->val == ".string")
            {
                std::string str;
                if (trans_string(std::get<std::string>(q->val), str))
                {
                    val = str;
                }
                else
                {
                    throw syntax_error{p->loc, "invalid string"};
                }
            }
            auto literal = syntax_literal{.type = type, .val = val};
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
        auto syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
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
            if (syntax_node->type.get_primary().empty())
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
                        syntax_ret->immutable = syntax_node->immutable;
                        syntax_ret->type = *(product_t->types[i]);
                        syntax_ret->val = syntax_dot{.field = (p->attr).val, .val = syntax_node};
                        stmts.push_back(syntax_stmt{.stmt = syntax_ret});
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
                        syntax_ret->immutable = syntax_node->immutable;
                        syntax_ret->type = *(sum_t->types[i]);
                        syntax_ret->val = syntax_dot{.field = p->attr.val, .val = syntax_node};
                        stmts.push_back(syntax_stmt{.stmt = syntax_ret});
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
        if (auto q = std::get_if<sum_type>(&syntax_check_exp->type.type))
        {
            auto syntax_fun_node = syntax_fun_call{.fun_name = "." + p->check_lable.val + "?", .parameters = {syntax_check_exp}};
            auto syntax_node = env_fun.infer_type(syntax_fun_node, stmts);
            stmts.push_back(syntax_stmt{.stmt = syntax_node});
            return syntax_node;
        }
        else
        {
            throw syntax_error(p->loc, "? must be used with sum_type");
        }
    }
    else if (auto p = std::get_if<node_post_arr_expr>(&node.expr))
    {
        try
        {
            auto arr = post_expr_analysis(*p->arr, stmts);
            auto idx = expr_analysis(*p->arr_index, stmts);
            auto member = std::make_shared<syntax_expr>();
            member->type = arr->type.de_ref();
            member->val = syntax_arr_member{
                .base = arr,
                .idx = idx};
            stmts.push_back(syntax_stmt{member});
            return member;
        }
        catch (inner_error &)
        {
            throw syntax_error(p->loc, "[] operator misused");
        }
    }
    else
        assert(false);
}

std::shared_ptr<syntax_expr> expr_convert_to(std::shared_ptr<syntax_expr> expr, const syntax_type &target, std::vector<syntax_stmt> &stmts)
{
    auto from_type = expr->type;
    auto to_type = target;
    if (from_type.subtyping(to_type))
    {
        if (from_type.type_equal(to_type))
        {
            return expr;
        }

        auto ret = std::make_shared<syntax_expr>();

        if (target.is_sum())
        {
            // 转换到 sum type
            // 必然来自 product type
            ret->type = target;
            ret->val = syntax_var();
            stmts.push_back(syntax_stmt{.stmt = ret});

            auto source = std::get<product_type>(expr->type.type);
            auto sum = std::get<sum_type>(target.type);

            auto field = source.fields[0];
            auto sum_idx = sum.get_index(field);
            assert(sum_idx != -1);

            auto source_field_type = *source.types[0];
            auto target_field_type = *sum.types[sum_idx];

            // 右值
            auto rval = std::make_shared<syntax_expr>(syntax_dot{.field = field, .val = expr}, source_field_type);
            stmts.push_back(syntax_stmt{.stmt = rval});
            rval = expr_convert_to(rval, target_field_type, stmts);

            // 左值
            auto lval = std::make_shared<syntax_expr>(syntax_dot{.field = field, .val = ret}, target_field_type);
            stmts.push_back(syntax_stmt{.stmt = lval});

            auto assign = syntax_assign{.lval = lval, .rval = rval};
            stmts.push_back(syntax_stmt{.stmt = assign});

            try_sum_tag_assign(lval, stmts);
        }
        else if (target.is_product())
        {
            // 转到 product type
            ret->type = target;
            ret->val = syntax_var();
            stmts.push_back(syntax_stmt{.stmt = ret});

            auto target_type = std::get<product_type>(target.type);
            auto source_type = std::get<product_type>(expr->type.type);
            for (auto i = 0; i < target_type.fields.size(); i++)
            {
                auto field = target_type.fields[i];
                auto target_field_type = *target_type.types[i];

                auto idx = source_type.get_index(field);
                if (idx == -1)
                {
                    throw inner_error{INNER_CANT_CAST};
                }

                auto source_field_type = *source_type.types[idx];

                // 右值
                auto rval = std::make_shared<syntax_expr>(syntax_dot{.field = field, .val = expr}, source_field_type);
                stmts.push_back(syntax_stmt{.stmt = rval});
                rval = expr_convert_to(rval, target_field_type, stmts);

                // 左值
                auto lval = std::make_shared<syntax_expr>(syntax_dot{.field = field, .val = ret}, target_field_type);
                stmts.push_back(syntax_stmt{.stmt = lval});

                auto assign = syntax_assign{.lval = lval, .rval = rval};
                stmts.push_back(syntax_stmt{.stmt = assign});
            }
        }
        else
        {
            // 普通类型转换
            ret->type = to_type;
            ret->val = syntax_type_convert{.source_expr = expr, .target_type = to_type};
            stmts.push_back(syntax_stmt{ret});
        }
        return ret;
    }
    throw inner_error{INNER_CANT_CAST};
}

void try_sum_tag_assign(const std::shared_ptr<syntax_expr> expr, std::vector<syntax_stmt> &stmts)
{
    if (auto idx = expr->is_sum_dot())
    {
        auto type_i32 = syntax_type{.type = primary_type{.name = "i32", .size = 4}};
        auto lval_father = std::get<syntax_dot>(expr->val).val;
        auto lval = std::make_shared<syntax_expr>(syntax_dot{.field = ".tag", .val = lval_father}, type_i32);
        auto rval = std::make_shared<syntax_expr>(syntax_literal{.type = type_i32, .val = (unsigned long long)idx}, type_i32);
        stmts.push_back(syntax_stmt{.stmt = lval});
        stmts.push_back(syntax_stmt{.stmt = rval});
        stmts.push_back(syntax_stmt{.stmt = syntax_assign{
                                        .lval = lval,
                                        .rval = rval}});
    }
}
